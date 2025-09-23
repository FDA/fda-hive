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
    class sVariant {
    public:
        enum eType {
            value_NULL,
            value_INT,
            value_UINT,
            value_REAL,
            value_STRING,
            value_LIST,
            value_DIC,
            value_HIVE_ID,
            value_DATE_TIME,
            value_DATE,
            value_TIME,
            value_DATA,
            value_BOOL
        };

        enum ePrintMode {
            eDefault = 0,
            eUnquoted,
            eCSV,
            eJSON
        };

        struct Whitespace {
            const char * space;
            const char * indent;
            const char * newline;
        };

        static const Whitespace * getDefaultWhitespace(ePrintMode default_for=sVariant::eJSON);

        class sVariantData {
        public:
            virtual sVariantData* clone() = 0;
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
            virtual bool isNullish() const = 0;
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
            void clone(const sVariantList & rhs) {
                _vec.resize(rhs._vec.dim());
                for(idx i = 0; i < rhs._vec.dim(); ++i) {
                    _vec[i].clone(rhs._vec[i]);
                }
            }
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
            void clone(const sVariantDic & rhs) {
                _dic.empty();
                for(idx i = 0; i < rhs.dim(); ++i) {
                    idx len;
                    const char * k = (const char * )(rhs._dic.id(i, &len));
                    sVariant * o = _dic.setString(k, len);
                    if( o ) {
                        o->clone(rhs._dic[i]);
                    }
                }
            }
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

            unsigned char id_buf[sizeof(sHiveId)];
            sHiveId * id() { return reinterpret_cast<sHiveId*>(id_buf); }
            const sHiveId * id() const { return reinterpret_cast<const sHiveId*>(id_buf); }
            DateTime date;
        } _val;
        sStr _str;

    public:
        sVariant(): _str(sMex::fExactSize) { _type = value_NULL; _val.i = 0; }
        sVariant(bool b): _str(sMex::fExactSize) { _type = value_BOOL; _val.i = b; }
        sVariant(int i): _str(sMex::fExactSize) { _type = value_INT; _val.i = i; }
        sVariant(idx i): _str(sMex::fExactSize) { _type = value_INT; _val.i = i; }
        sVariant(udx u): _str(sMex::fExactSize) { _type = value_UINT; _val.u = u; }
        sVariant(real r): _str(sMex::fExactSize) { _type = value_REAL; _val.r = r; }
        sVariant(const char *s, idx len=0): _str(sMex::fExactSize) { _type = value_NULL; setString(s, len); }
        sVariant(sVariant *list, idx listdim): _str(sMex::fExactSize) { _type = value_NULL; setList(list, listdim); }
        sVariant(const sVariant &rhs);

        ~sVariant() { empty(); }

        void empty();

        const char* getTypeName() const __attribute__((pure));
        static const char* getTypeName(idx type) __attribute__((const));
        eType getType() const { return _type; }
        static eType parseTypeName(const char * type_name) __attribute__((const));

        inline bool isNull() const { return _type == value_NULL; }
        inline bool isBool() const { return _type == value_BOOL; }
        inline bool isInt() const { return _type == value_INT; }
        inline bool isIntLike() const { return isNull() || isBool() || isInt(); }
        inline bool isUInt() const { return _type == value_UINT; }
        inline bool isUIntLike() const { return isNull() || isBool() || isUInt(); }
        inline bool isReal() const { return _type == value_REAL; }
        inline bool isNumeric() const { return isNull() || isBool() || isInt() || isUInt() || isReal(); }
        inline bool isString() const { return _type == value_STRING; }
        inline bool isScalar() const { return isNumeric() || isString() || isHiveId() || isDateOrTime() || isData(); }
        inline bool isList() const { return _type == value_LIST; }
        inline bool isDic() const { return _type == value_DIC; }
        inline bool isHiveId() const { return _type == value_HIVE_ID; }
        inline bool isDateTime() const { return _type == value_DATE_TIME; }
        inline bool isDate() const { return _type == value_DATE; }
        inline bool isTime() const { return _type == value_TIME; }
        inline bool isDateOrTime() const { return isDateTime() || isDate() || isTime(); }
        inline bool isData() const { return _type == value_DATA; }

        sVariant& operator=(const sVariant &rhs);
        sVariant& clone(const sVariant & rhs);
        inline void setNull() { empty(); }
        void setBool(bool b = false);
        void setInt(int i) { setInt((idx)i); }
        void setInt(idx i = 0);
        void setUInt(udx u = 0);
        void setReal(real r = 0.);
        void setString(const char *s = NULL, idx len=0);
        void setVSprintf(const char *fmt, va_list marker);
        void setSprintf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
        void setList(sVariant *list, idx listdim);
        void setList();
        void setDic();
        void setHiveId(const sHiveId & id);
        void setHiveId(const char * domain_id, udx obj_id, udx ion_id=0);
        void setHiveId(udx obj_id, udx ion_id=0) { setHiveId(0, obj_id, ion_id); }
        void setHiveId(const sVariant & rhs);
        void setHiveId(const char *s);
        void setDateTime(idx unix_time);
        void setDateTime(const struct tm * tm);
        void setDateTime(const sVariant & rhs);
        void setDate(idx unix_time);
        void setDate(const struct tm * tm);
        void setDate(const sVariant & rhs);
        void setTime(idx seconds, idx utc_offset=-sIdxMax);
        void setTime(const struct tm * tm);
        void setTime(const sVariant & rhs);
        void setData(sVariantData &data);

        void parseInt(const char *s);
        void parseUInt(const char *s);
        void parseBool(const char *s);
        void parseIntList(const char *s);
        void parseReal(const char *s);
        void parseHiveId(const char *s);
        void parseHiveIdList(const char *s);
        void parseDateTime(const char *s);
        void parseDate(const char *s);
        void parseTime(const char *s);
        void parseScalarType(const char * s, eType type);
        static eType guessScalarType(const char * buf, idx len, eType prevGuess = value_NULL);
        static eType guessScalarType(const char * s, eType prevGuess = value_NULL);

        bool asBool() const;
        bool isNullish() const;
        idx asInt() const;
        udx asUInt() const;
        real asReal() const;
        void getNumericValue(sVariant& numeric) const;
        const char * print(sStr &s, ePrintMode mode=eDefault, const Whitespace * whitespace=0, idx indent_level=0) const;
        const char* asString();
        const sHiveId * asHiveId(sHiveId * out=0) const;
        idx asHiveIds(sVec<sHiveId> & out) const;
        idx asDateTime(struct tm * out=0) const;
        const sVariantData* asData() const { return isData() ? _val.data : NULL; }
        sVariantData* asData() { return isData() ? _val.data : NULL; }

        idx dim() const __attribute__((pure));
        bool getElt(idx i, sVariant &out) const;
        const sVariant* getListElt(idx i) const;
        sVariant* getListElt(idx i);
        bool getElt(const char *key, sVariant &out) const;
        const sVariant* getDicElt(const char *key) const;
        sVariant* getDicElt(const char *key);
        sVariant* getDicElt(idx i);
        inline const sVariant* getDicElt(idx i) const { return const_cast<const sVariant&>(*this).getDicElt((idx)i); }
        inline sVariant* getDicElt(int i) { return getDicElt((idx)i); }
        inline const sVariant* getDicElt(int i) const { return getDicElt((idx)i); }
        const char * getDicKeyVal(idx i, sVariant &outValue) const;
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
        sVariant* push(sVariant &elt);
        sVariant* push(bool b)
        {
            sVariant elt(b);
            return push(elt);
        }
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
        sVariant* push(real r)
        {
            sVariant elt(r);
            return push(elt);
        }
        sVariant* push(const char *s, idx len=0)
        {
            sVariant elt(s, len);
            return push(elt);
        }
        bool append(sVariant &rhs);
        bool appendVSprintf(const char *fmt, va_list marker);
        bool append(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
        const char * getDicKey(idx i) const;
        bool getDicKeys(sVariant &out) const;
        bool getDicKeys(sStr &out00) const;

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

        bool operator==(const sVariant &rhs) const;
        bool operator!=(const sVariant &rhs) const { return !operator==(rhs); }
        bool operator<(const sVariant &rhs) const;
        bool operator>(const sVariant &rhs) const;
        inline bool operator<=(const sVariant &rhs) const { return !operator>(rhs); }
        inline bool operator>=(const sVariant &rhs) const { return !operator<(rhs); }
        idx cmp(const sVariant &rhs) const;

        static void internString(const char * s, idx len=0);

    protected:
        void parseNumList(const char *s, idx type);
    };
};

#endif
