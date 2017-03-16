/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once
#ifndef sLib_variant_h
#define sLib_variant_h

#include <slib/core/dic.hpp>
#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/id.hpp>

namespace slib {
    //! A variant type; can hold integer, real, or string scalars, or a list or dictionary of variants
    // TODO: make final when we switch to c++11
    class sVariant {
    public:
        // keep in sync with sVariantNames in variant.cpp
        enum eType {
            value_NULL, //!< nothing
            value_INT, //!< idx value
            value_UINT, //!< uidx value
            value_REAL, //!< real value
            value_STRING, //!< string value
            value_LIST, //!< list of sVariants
            value_DIC, //!< associative array of sVariant
            value_HIVE_ID, //!< sHiveId
            value_DATE_TIME, //!< timestamp
            value_DATE, //!< timestamp with only date component visible
            value_TIME, //!< seconds since midnight
            value_DATA //!< other data (sVariantData)
        };

        // keep in sync with default_whitespace in variant.cpp
        enum ePrintMode {
            eDefault = 0, //!< null as 0; strings, dates, dic keys quoted but not escaped
            eUnquoted, //!< null as 0; strings, dates, and dic keys not quoted
            eCSV, //!< for use in a CSV table cell
            eJSON //!< as a JSON document
        };

        struct Whitespace {
            const char * space; //!< generic whitespace separator, must not be null
            const char * indent; //!< indentation prefix, must not be null
            const char * newline; //!< newline, must not be null
        };

        static const Whitespace * getDefaultWhitespace(ePrintMode default_for=sVariant::eJSON);

        //! Wrapper interface for extending sVariant to support additional data types
        class sVariantData {
        public:
            virtual sVariantData* clone() = 0; //< If data is refcounted, clone() is assumed to call incrementRefCount()
            virtual sVariantData* cloneInto(sVariantData *rhs)
            {
                if (rhs && !operator==(*rhs)) {
                    rhs->decrementRefCount();
                    if (!rhs->hasRefs())
                        delete rhs;
                }
                return clone();
            }
            virtual ~sVariantData() {}
            virtual void incrementRefCount() {}
            virtual void decrementRefCount() {}
            virtual bool hasRefs() const { return false; }
            virtual bool asBool() const = 0;
            virtual idx asInt() const = 0;
            virtual real asReal() const = 0;
            virtual void getNumericValue(sVariant &numeric) const = 0;
            virtual void print(sStr &s, ePrintMode mode=eDefault, const Whitespace * whitespace=0, idx indent_level=0) const = 0;
            virtual const char* getTypeName() const = 0;
            virtual bool operator==(const sVariantData &rhs) const = 0;
            virtual bool operator<(const sVariantData &rhs) const = 0;
            virtual bool operator>(const sVariantData &rhs) const = 0;
        };

    protected:
        class sVariantList {
        protected:
            mutable idx _refCount;
            sVec<sVariant> _vec;
        public:
            sVariantList(): _refCount(1) {}
            virtual ~sVariantList() {}
            void incrementRefCount() { _refCount++; }
            void decrementRefCount() { _refCount--; }
            bool hasRefs() const { return _refCount > 0; }
            idx dim() const { return _vec.dim(); }
            void resize(idx size) { _vec.resize(size); }
            bool has(idx i) const { return i >= 0 && i < _vec.dim(); }
            sVariant& get(idx i) { return _vec[i]; }
            sVariant& set(idx i, sVariant &rhs) { return _vec[i] = rhs; }
            sVariant& add(sVariant &rhs) { return *(_vec.add()) = rhs; }
            sVariant& add() { return *(_vec.add()); }
        };

        class sVariantDic {
        protected:
            mutable idx _refCount;
            sDic<sVariant> _dic;
        public:
            sVariantDic(): _refCount(1) {}
            virtual ~sVariantDic() {}
            void incrementRefCount() { _refCount++; }
            void decrementRefCount() { _refCount--; }
            bool hasRefs() const { return _refCount > 0; }
            idx dim() const { return _dic.dim(); }
            bool has(const char *key) { return _dic.get(key); }
            sVariant& get(const char *key) { return *(_dic.get(key)); }
            sVariant* getPtr(const char *key) { return _dic.get(key); }
            sVariant* getPtr(idx i) { return _dic.ptr(i); }
            sVariant* getPtr(int i) { return _dic.ptr((idx)i); }
            sVariant& set(const char *key, idx key_len, sVariant &rhs) { return *(_dic.setString(key, key_len)) = rhs; }
            const char * key(idx i) { return static_cast<const char*>(_dic.id(i)); }
            void keys(sStr &out00) { for (idx i=0; i<_dic.dim(); i++) out00.add(key(i)); out00.add0(2); }
        };

        struct DateTime {
            idx timestamp;
            idx utc_offset;
        };
        eType _type;
        union {
            idx i;
            udx u;
            real r;
            sVariantList *list;
            sVariantDic *dic;
            sVariantData *data;

            // replace this hack with "sHiveId id" when we switch to c++11
            unsigned char id_buf[sizeof(sHiveId)];
            sHiveId * id() { return reinterpret_cast<sHiveId*>(id_buf); }
            const sHiveId * id() const { return reinterpret_cast<const sHiveId*>(id_buf); }
            DateTime date;
        } _val;
        sStr _str;

    public:
        //! Construct a null-valued variant
        sVariant(): _str(sMex::fExactSize) { _type = value_NULL; _val.i = 0; }
        //! Construct an integer-valued variant
        sVariant(bool b): _str(sMex::fExactSize) { _type = value_INT; _val.i = b; }
        //! Construct an integer-valued variant
        sVariant(int i): _str(sMex::fExactSize) { _type = value_INT; _val.i = i; }
        //! Construct an integer-valued variant
        sVariant(idx i): _str(sMex::fExactSize) { _type = value_INT; _val.i = i; }
        //! Construct an integer-valued variant
        sVariant(udx u): _str(sMex::fExactSize) { _type = value_UINT; _val.u = u; }
        //! Construct a real-valued variant 
        sVariant(real r): _str(sMex::fExactSize) { _type = value_REAL; _val.r = r; }
        //! Construct a string-valued variant
        sVariant(const char *s, idx len=0): _str(sMex::fExactSize) { _type = value_NULL; setString(s, len); }
        //! Construct a list variant
        /* \param list Note that list elements will have their ref count incremented */
        sVariant(sVariant *list, idx listdim): _str(sMex::fExactSize) { _type = value_NULL; setList(list, listdim); }
        //! Copy constructor
        /* \param rhs Note that if rhs is a list variant, we will construct a shallow copy, and rhs._list elements will have their ref count incremented */
        sVariant(const sVariant &rhs);

        // not virtual - sVariant should not be inherited from, and no vptr saves memory due to alignment issues
        ~sVariant() { empty(); }

        //! Set _val.list/_val.dic to NULL, decrement its reference count, and delete it if necessary
        void empty();

        //! Human-readable name of the variant's type
        const char* getTypeName() const __attribute__((pure));
        static const char* getTypeName(idx type) __attribute__((const));
        //! Variant's type as an eType value
        eType getType() const { return _type; }
        static eType parseTypeName(const char * type_name) __attribute__((const));

        //! \returns true if the variant is holding a null
        inline bool isNull() const { return _type == value_NULL; }
        //! \returns true if the variant is holding an idx
        inline bool isInt() const { return _type == value_INT; }
        //! \returns true if the variant is holding a uidx
        inline bool isUInt() const { return _type == value_UINT; }
        //! \returns true if the variant is holding a real value
        inline bool isReal() const { return _type == value_REAL; }
        //! \returns true if the variant is holding a numeric scalar
        inline bool isNumeric() const { return isNull() || isInt() || isUInt() || isReal(); }
        //! \returns true if the variant is holding a string scalar
        inline bool isString() const { return _type == value_STRING; }
        //! \returns true if the variant is holding a scalar value
        inline bool isScalar() const { return isNumeric() || isString() || isHiveId() || isDateOrTime() || isData(); }
        //! \returns true if the variant is a list of variants
        inline bool isList() const { return _type == value_LIST; }
        //! \returns true if the variant is a dic of variants
        inline bool isDic() const { return _type == value_DIC; }
        //! \returns true if the variant is an object id
        inline bool isHiveId() const { return _type == value_HIVE_ID; }
        //! \returns true if the variant is date and time
        inline bool isDateTime() const { return _type == value_DATE_TIME; }
        //! \returns true if the variant is date without time
        inline bool isDate() const { return _type == value_DATE; }
        //! \returns true if the variant is time without date
        inline bool isTime() const { return _type == value_TIME; }
        //! \returns true if the variant is date and/or time
        inline bool isDateOrTime() const { return isDateTime() || isDate() || isTime(); }
        //! \returns true if the variant is sVariantData
        inline bool isData() const { return _type == value_DATA; }

        sVariant& operator=(const sVariant &rhs);
        //! Set to null value
        inline void setNull() { empty(); }
        //! Overwrite the variant's contents with an idx value
        void setInt(int i) { setInt((idx)i); }
        //! Overwrite the variant's contents with an idx value
        void setInt(idx i = 0);
        //! Overwrite the variant's contents with a udx value
        void setUInt(udx u = 0);
        //! Overwrite the variant's contents with a real value
        void setReal(real r = 0.);
        //! Overwrite the variant's contents with a string
        void setString(const char *s = NULL, idx len=0);
        //! Overwrite the variant's contents with a string, vprintf-style
        void setVSprintf(const char *fmt, va_list marker);
        //! Overwrite the variant's contents with a string, printf-style
        void setSprintf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
        //! Overwrite the variant's contents with a list value
        void setList(sVariant *list, idx listdim);
        //! Overwrite the variant's contents with an empty list
        void setList();
        //! Overwrite the variant's contents with an empty dic
        void setDic();
        //! Overwrite the variant's contents with a HIVE id
        void setHiveId(const sHiveId & id);
        void setHiveId(const char * domain_id, udx obj_id, udx ion_id=0);
        void setHiveId(udx obj_id, udx ion_id=0) { setHiveId(0, obj_id, ion_id); }
        void setHiveId(const sVariant & rhs);
        void setHiveId(const char *s);
        //! Overwrite the variant's contents with a date-time
        void setDateTime(idx unix_time);
        void setDateTime(const struct tm * tm);
        void setDateTime(const sVariant & rhs);
        //! Overwrite the variant's contents with a date-time
        void setDate(idx unix_time);
        void setDate(const struct tm * tm);
        void setDate(const sVariant & rhs);
        //! Overwrite the variant's contents with a time of day
        /*! \param seconds seconds since midnight
            \param utc_offset timezone in seconds relative to UTC, or -sIdxMax for system timezone */
        void setTime(idx seconds, idx utc_offset=-sIdxMax);
        void setTime(const struct tm * tm);
        void setTime(const sVariant & rhs);
        //! Overwrite the variant's contents with other data (will be cloned)
        void setData(sVariantData &data);

        //! Parse a string as an integer, and set variant's value to it
        void parseInt(const char *s);
        //! Parse a string as an unsigned integer, and set variant's value to it
        void parseUInt(const char *s);
        //! Parse a string as boolean, and set variant's value to it
        void parseBool(const char *s);
        //! Parse a string as a list of integers, and set variant's value to it
        void parseIntList(const char *s);
        //! Parse a string as a real, and set variant's value to it
        void parseReal(const char *s);
        //! Parse a string as a HIVE id, and set variant's value to it
        void parseHiveId(const char *s);
        //! Parse a string as a list of HIVE IDs, and set variant's value to it
        void parseHiveIdList(const char *s);
        //! Parse a string as a date-time, and set the variant's value to it
        void parseDateTime(const char *s);
        //! Parse a string as a date, and set the variant's value to it
        void parseDate(const char *s);
        //! Parse a string as a time of day, and set the variant's value to it
        void parseTime(const char *s);
        //! Parse a string as a specified type; value_NULL means guess the type, but cast false result to null
        void parseScalarType(const char * s, eType type);
        //! Guess appropriate scalar eType for a buffer
        /*! \param len must be provided - 0 is interpreted as 0-length string */
        static eType guessScalarType(const char * buf, idx len, eType prevGuess = value_NULL);
        //! Guess appropriate scalar eType for a string
        static eType guessScalarType(const char * s, eType prevGuess = value_NULL);

        //! Casts to bool
        /*! \returns true if the variant's value is a non-zero, non-NAN number; a non-empty string; or a non-empty list */
        bool asBool() const;
        //! Casts to idx
        /*! \returns the variant's numeric value (rounded if real); or its string value parsed as an idx; or its list length */
        idx asInt() const;
        //! Casts to udx
        udx asUInt() const;
        //! Casts to real
        /*! \returns the variant's numeric value; or its string value parsed as a real; or its list length */
        real asReal() const;
        //! Casts to numeric variant
        /*! \param[out] numeric a variant, will be set to this one's integer or real value, whichever is most appropriate */
        void getNumericValue(sVariant& numeric) const;
        //! Print the variant's value
        /*! \param[out] s string to print to
         *  \param mode type of quoting needed, e.g. if the value needs to be a JSON string or a CSV cell
            \param whitespace how to format non-atomic values
            \param indent_level number of prefixes to insert
            \returns pointer to start of printed string */
        const char * print(sStr &s, ePrintMode mode=eDefault, const Whitespace * whitespace=0, idx indent_level=0) const;
        //! Returns the variant's value printed as a string
        /*! \note non-const because the string buffer is saved in _str */
        const char* asString();
        //! Casts to HIVE ID
        /*! \returns variant's HIVE ID if out is 0 and variant is of type HIVE ID;
                     or 0 if out is 0 and variant is not of type HIVE ID;
                     or pointer to out (where the variant will be cast) otherwise */
        const sHiveId * asHiveId(sHiveId * out=0) const;
        //! Casts to a list of HIVE IDs
        idx asHiveIds(sVec<sHiveId> & out) const;
        //! Casts to date-time
        idx asDateTime(struct tm * out=0) const;
        //! Get the data pointer, if defined
        const sVariantData* asData() const { return isData() ? _val.data : NULL; }
        //! Get the data pointer, if defined
        sVariantData* asData() { return isData() ? _val.data : NULL; }

        //! Size of a list or dic, or length of a string
        /*! \returns string or list length; or number of keys in a dic; or 0 for numerics */
        idx dim() const __attribute__((pure));
        //! Retrieve a string's character or a list's element
        /*! \param i index into string or list
         *  \param[out] out the return value
         *  \returns false if the variant is not a string or a list, or if i was out of range */
        bool getElt(idx i, sVariant &out) const;
        //! Retrieve a list element
        /*! \param i index into list
         *  \returns pointer to the element, or NULL if the value is not a list or i was out of range */
        const sVariant* getListElt(idx i) const;
        //! Retrieve a list element
        /*! \param i index into list
         *  \returns pointer to the element, or NULL if the value is not a list or i was out of range */
        sVariant* getListElt(idx i);
        //! Retrieve a dic element
        /*! \param key dic key
         *  \param[out] out the return value
         *  \returns false if the variant is not a dic, or if key is not present in it */
        bool getElt(const char *key, sVariant &out) const;
        //! Retrieve a dic element
        /*! \param key dic key
         *  \returns pointer to the element, or NULL if the value is not a dic or if key is not present in it */
        const sVariant* getDicElt(const char *key) const;
        //! Retrieve a dic element
        /*! \param key dic key
         *  \returns pointer to the element, or NULL if the value is not a dic or if key is not present in it */
        sVariant* getDicElt(const char *key);
        //! Retrieve a dic element
        /*! \param i index of key
         *  \returns pointer to the element, or NULL if the value is not a dic or if key is not present in it */
        sVariant* getDicElt(idx i);
        inline const sVariant* getDicElt(idx i) const { return const_cast<const sVariant&>(*this).getDicElt((idx)i); }
        inline sVariant* getDicElt(int i) { return getDicElt((idx)i); }
        inline const sVariant* getDicElt(int i) const { return getDicElt((idx)i); }
        //! Retrieves a dic's key/value pair
        /*\ \param i index of key
         *  \param[out] outValue value for key with index i
         *  \returns key with index i, or NULL if no such key exists */
        const char * getDicKeyVal(idx i, sVariant &outValue) const;
        //! Set a list element
        /*! \param i index into string or list; if it's higher than the current max index, intermediate list elements will be created and set to integer value 0
         *  \param elt value to set
         *  \returns pointer to modified/created element on success;
         *           NULL if the variant is not a list, or if i was negative */
        sVariant* setElt(idx i, sVariant &elt);
        sVariant* setElt(idx i, bool b)
        {
            sVariant elt(b);
            return setElt(i, elt);
        }
        sVariant* setElt(idx i, int j)
        {
            sVariant elt(j);
            return setElt(i, elt);
        }
        sVariant* setElt(idx i, idx j)
        {
            sVariant elt(j);
            return setElt(i, elt);
        }
        sVariant* setElt(idx i, udx u)
        {
            sVariant elt(u);
            return setElt(i, elt);
        }
        sVariant* setElt(idx i, real r)
        {
            sVariant elt(r);
            return setElt(i, elt);
        }
        sVariant* setElt(idx i, const char *s, idx len=0)
        {
            sVariant elt(s, len);
            return setElt(i, elt);
        }
        //! Set a dic element
        /*! \param key dic key
         *  \param elt value to set
         *  \returns pointer to modified/created element on success;
         *           NULL if the variant is not a dic */
        sVariant* setElt(const char *key, sVariant &elt);
        sVariant* setElt(const char *key, idx key_len, sVariant &elt);
        sVariant* setElt(const char *key, bool b)
        {
            sVariant elt(b);
            return setElt(key, elt);
        }
        sVariant* setElt(const char *key, int j)
        {
            sVariant elt(j);
            return setElt(key, elt);
        }
        sVariant* setElt(const char *key, idx j)
        {
            sVariant elt(j);
            return setElt(key, elt);
        }
        sVariant* setElt(const char *key, udx u)
        {
            sVariant elt(u);
            return setElt(key, elt);
        }
        sVariant* setElt(const char *key, real r)
        {
            sVariant elt(r);
            return setElt(key, elt);
        }
        sVariant* setElt(const char *key, const char *s, idx len=0)
        {
            sVariant elt(s, len);
            return setElt(key, elt);
        }
        //! Adds a new element to a list variant
        /*! \note no-op for non-list variants
         *  \returns pointer to added element on success;
         *           NULL if we are not a list */
        sVariant* push(sVariant &elt);
        //! Adds a boolean integer (0 or 1) value to a list variant
        sVariant* push(bool b)
        {
            sVariant elt(b);
            return push(elt);
        }
        //! Adds an integer value to a list variant
        sVariant* push(int i)
        {
            sVariant elt(i);
            return push(elt);
        }
        sVariant* push(idx i)
        {
            sVariant elt(i);
            return push(elt);
        }
        sVariant* push(udx u)
        {
            sVariant elt(u);
            return push(elt);
        }
        //! Adds a real value to a list variant
        sVariant* push(real r)
        {
            sVariant elt(r);
            return push(elt);
        }
        //! Adds a string value to a list variant
        sVariant* push(const char *s, idx len=0)
        {
            sVariant elt(s, len);
            return push(elt);
        }
        //! Concatenates strings or appends lists
        /*! \note no-op unless the variant is a string or a list
         * \returns false if this was a no-op */
        bool append(sVariant &rhs);
        bool appendVSprintf(const char *fmt, va_list marker);
        bool append(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
        //! Retrieves a dic's key
        /*! \param i index of key
            \returns key with index i */
        const char * getDicKey(idx i) const;
        //! Retrieves a dic's keys
        /*! \param[out] out the dic's keys as a list variant
         * \returns false if the value is not a dic */
        bool getDicKeys(sVariant &out) const;
        //! Retrieves a dic's keys
        /*! \param[out] out00 the dic's keys as 00-terminated list of concatenated, 0-terminated strings
         * \returns false if the value is not a dic */
        bool getDicKeys(sStr &out00) const;

        //!@{
        //! In-place math for numeric variants. No-op for non-numerics (in which case these operations will return false)
        bool operator+=(idx i);
        bool operator-=(idx i);
        bool operator*=(idx i);
        bool operator/=(idx i);
        bool operator%=(idx i);
        bool operator+=(real r);
        bool operator-=(real r);
        bool operator*=(real r);
        bool operator/=(real r);
        bool operator%=(real r);
        bool operator+=(const sVariant &rhs);
        bool operator-=(const sVariant &rhs);
        bool operator*=(const sVariant &rhs);
        bool operator/=(const sVariant &rhs);
        bool operator%=(const sVariant &rhs);
        //!@}

        //!@{
        //! Deep comparisons
        bool operator==(const sVariant &rhs) const;
        bool operator!=(const sVariant &rhs) const { return !operator==(rhs); }
        bool operator<(const sVariant &rhs) const;
        bool operator>(const sVariant &rhs) const;
        inline bool operator<=(const sVariant &rhs) const { return !operator>(rhs); }
        inline bool operator>=(const sVariant &rhs) const { return !operator<(rhs); }
        idx cmp(const sVariant &rhs) const;
        //!@}

        //! intern a string value to make future variants holding it use less memory
        static void internString(const char * s, idx len=0);

    protected:
        void parseNumList(const char *s, idx type);
    };
};

#endif
