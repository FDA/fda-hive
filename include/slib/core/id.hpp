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
#ifndef sLib_core_id_hpp
#define sLib_core_id_hpp

#include <slib/core/def.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>

namespace slib {
    // 2048 bytes for url, 20 each for obj_id and ion_id, 2 separators, 1 null
#define S_HIVE_ID_MAX_BUFLEN 2091
    // 8 bytes for domain, 20 each for obj_id and ion_id, 2 separators, 1 null
#define S_HIVE_ID_SHORT_BUFLEN 51
    //! 3-part identifier for objects in HIVE's sql database or ION
    class sHiveId
    {
        private:
            udx _domain_id;
            udx _obj_id;
            udx _ion_id;

            static sDic<udx> _urlToDomainId;
            static sDic<sStr> _domainIdToUrl;

        public:
            static const sHiveId zero; //!< all-0 sHiveId for use as a default value

            sHiveId()
            {
                reset();
            }
            sHiveId(const sHiveId & rhs)
            {
                memcpy(this, &rhs, sizeof(sHiveId));
            }
            sHiveId(const char * dom, udx obj, udx ion)
            {
                set(dom, obj, ion);
            }
            sHiveId(udx dom, udx obj, udx ion)
            {
                set((const char *)0, obj, ion);
                setDomainId(dom);
            }
            sHiveId(udx obj, udx ion)
            {
                set((const char *)0, obj, ion);
            }
            sHiveId(const char * s)
            {
                parse(s);
            }
            sHiveId& operator=(const sHiveId & rhs)
            {
                if( this != &rhs ) {
                    memcpy(this, &rhs, sizeof(sHiveId));
                }
                return *this;
            }

            udx domainId() const
            {
                return _domain_id;
            }
            //! set domain ID using HIVE name or URL
            /* \param dom HIVE name case insensitive (8 char maximum), or URL (must have been previously
             registered using mapDomainIdUrl()), or 0 for a local ID. An invalid name
             or unregistered URL will be interpreted as 0
             \returns true if set successfully */
            bool setDomainId(const char * dom);
            bool setDomainId(udx dom_id);
            //! encode an 8-character case insensitive HIVE name into a numeric domain id
            static udx encodeDomainId(const char * dom, idx len = 0) __attribute__((pure));
            //! decode a numeric domain id into a readable HIVE name
            /*! \param outbuf buffer of at least 9 bytes (8 for name + terminal 0) */
            static bool decodeDomainId(char * outbuf, udx dom);
            //! verify that a readable HIVE name is alphanumeric, and starts with a letter or '_'
            static bool validDomainId(const char * dom, idx len) __attribute__((pure));
            //! Return the URL that maps to the domain ID, assuming that such a map had been defined
            const char * domainIdAsUrl() const
            {
                const sStr * purl = static_cast<const sStr *>(_domainIdToUrl.get(&_domain_id, sizeof(_domain_id)));
                return purl ? purl->ptr() : 0;
            }

            udx objId() const
            {
                return _obj_id;
            }
            void setObjId(udx obj)
            {
                _obj_id = obj;
            }
            udx ionId() const
            {
                return _ion_id;
            }
            void setIonId(udx ion)
            {
                _ion_id = ion;
            }
            /*! \param dom HIVE name (lowercase, 8 char maximum), or URL (must have been previously
             registered using mapDomainIdUrl()), or 0 for local ids. Invalid names are
             interpeted as 0.
             \param obj ID in sql database, or 0 for an invalid object
             \param ion ID in ION */
            void set(const char * dom, udx obj, udx ion)
            {
                reset();
                setDomainId(dom);
                _obj_id = obj;
                _ion_id = ion;
            }

            void set(udx dom_id, udx obj, udx ion)
            {
                reset();
                setDomainId(dom_id);
                _obj_id = obj;
                _ion_id = ion;
            }

            void reset()
            {
                // set entire object to 0 so it's usable as hash key
                sSet(this, 0);
            }

            bool valid() const
            {
                char dombuf[9];
                decodeDomainId(dombuf, _domain_id);
                return validDomainId(dombuf, sizeof(dombuf) - 1) && _obj_id > 0;
            }
            operator bool() const
            {
                return _obj_id > 0;
            }
            //! parse a stringified ID
            /*! \param s string starting with a HIVE ID like "foo.1.2" or "1.2" or "1" or "http://example.com/hive/1.2";
             *           in the latter format the URL must have been registered using mapDomainIdUrl(). Parsing will stop
             *           when a valid HIVE ID has been scanned.
             *  \param len length of s; if 0, s is assumed to be 0-terminated
             *  \returns number of bytes of s that were parsed successfully */
            idx parse(const char * s, idx len = 0);
            //! print ID as a string into the specified string
            /*! \param domain_id_as_url print non-zero domain id as a URL instead of 8-character HIVE name.
             *                          The URL must have been previously registered using mapDomainIdUrl().
             *  \returns pointer to start of printed id inside \a out*/
            const char * print(sStr & out, bool domain_id_as_url = false) const;
            //! print ID as a string into a pre-allocated buffer (at least 51 bytes when domainIdAsUrl is false; or 2091 bytes for urls)
            /*! \param domain_id_as_url print non-zero domain ids as URL instead of 8-character HIVE names.
             *                          The URL must have been previously registered using mapDomainIdUrl().
             *  \returns number of bytes printed */
            idx print(char * out, bool domain_id_as_url = false) const;
            //! print ID as a string into a static buffer
            /*! \param domain_id_as_url print non-zero domain id as a URL instead of 8-character HIVE name.
             *                          Note that the URL must hae been previously restered using mapDomainIdUrl().
             *  \returns pointer to start of printed id
             *  \warning Uses a static buffer, so make sure to use or save the result before calling print() again */
            const char * print(bool domain_id_as_url = false) const
            {
                static char tmpbuf[S_HIVE_ID_MAX_BUFLEN];
                print(tmpbuf, domain_id_as_url);
                return tmpbuf;
            }
            //! print ID as a string into the specified string
            /*! \param domain_id_as_url print non-zero domain id as a URL instead of 8-character HIVE name.
             *                          The URL must have been previously registered using mapDomainIdUrl().
             *  \returns pointer to start of printed id inside \a out*/
            const char * printSQL(sStr & out, const char * prefix /* = 0 */, bool no_ion = false) const;
            //! parse a list of IDs, possibly specified as ranges, e.g. "foo.1.0-foo.1.5,2.0-5.0"
            /*! \returns number of ids parsed */
            static idx parseRangeSet(sVec<sHiveId>& out, const char * src, idx len = 0, idx * pLenParsed = 0);
            //! print a list of IDs, delimited by specified separator
            /*! \param domain_id_as_url print non-zero domain ids as URLs instead of 8-character HIVE names.
             *                          The URL must hae been previously restered using mapDomainIdUrl().
             *  \returns pointer to start of printed list in out */
            static const char * printVec(sStr & out, const sVec<sHiveId> & vec, const char * separ = ",", bool domain_id_as_url = false);

            idx cmp(const sHiveId & rhs) const
            {
                if( this == &rhs ) {
                    return 0;
                }
                idx ret = _domain_id - rhs._domain_id;
                if( !ret ) {
                    ret = _obj_id - rhs._obj_id;
                }
                if( !ret ) {
                    ret = _ion_id - rhs._ion_id;
                }
                return ret;
            }
            bool operator==(const sHiveId & rhs) const
            {
                return cmp(rhs) == 0;
            }
            bool operator!=(const sHiveId & rhs) const
            {
                return cmp(rhs) != 0;
            }
            bool operator<(const sHiveId & rhs) const
            {
                return cmp(rhs) < 0;
            }
            bool operator>(const sHiveId & rhs) const
            {
                return cmp(rhs) > 0;
            }
            bool operator<=(const sHiveId & rhs) const
            {
                return cmp(rhs) <= 0;
            }
            bool operator>=(const sHiveId & rhs) const
            {
                return cmp(rhs) >= 0;
            }

            //! Associate a URL with a numeric domain ID
            static bool mapDomainIdUrl(const udx domain_id, const char * url);

        private:
            // for catching mixups between sHiveId and integer types
            operator idx() const
            {
                return 0;
            }
            operator udx() const
            {
                return 0;
            }
            operator int() const
            {
                return 0;
            }
    };
}
;

#endif
