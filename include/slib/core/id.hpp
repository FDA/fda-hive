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
#define S_HIVE_ID_MAX_BUFLEN 2091
#define S_HIVE_ID_SHORT_BUFLEN 51
    class sHiveId
    {
        private:
            udx _domain_id;
            udx _obj_id;
            udx _ion_id;

            static sDic<udx> _urlToDomainId;
            static sDic<sStr> _domainIdToUrl;
            static sDic<sHiveId> _domainIdToId;
            static bool _printDomainAsUrl;

        public:
            static const sHiveId zero;

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
            bool setDomainId(const char * dom);
            bool setDomainId(udx dom_id);
            static udx encodeDomainId(const char * dom, idx len = 0) __attribute__((pure));
            static bool decodeDomainId(char * outbuf, udx dom);
            static bool validDomainId(const char * dom, idx len) __attribute__((pure));
            const char * domainIdAsUrl() const
            {
                return domainIdAsUrl(_domain_id);
            }
            static const char * domainIdAsUrl(udx domain_id)
            {
                const sStr * purl = static_cast<const sStr *>(_domainIdToUrl.get(&domain_id, sizeof(domain_id)));
                return purl ? purl->ptr() : 0;
            }
            static const sHiveId * domainObjId(udx domain_id)
            {
                const sHiveId * pid = static_cast<const sHiveId *>(_domainIdToId.get(&domain_id, sizeof(domain_id)));
                return pid;
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
            idx parse(const char * s, idx len = 0);

            static bool setDomainUrlPrintMode(const bool always_print_full_url)
            {
                bool x = _printDomainAsUrl;
                _printDomainAsUrl = always_print_full_url;
                return x;
            }
            const char * print(sStr & out, bool domain_id_as_url = false) const;
            idx print(char * out, bool domain_id_as_url = false) const;
            const char * print(bool domain_id_as_url = false) const
            {
                static char tmpbuf[S_HIVE_ID_MAX_BUFLEN];
                print(tmpbuf, domain_id_as_url);
                return tmpbuf;
            }
            const char * printSQL(sStr & out, const char * prefix, bool no_ion = false) const;
            static idx parseRangeSet(sVec<sHiveId>& out, const char * src, idx len = 0, idx * pLenParsed = 0);
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

            static bool mapDomainIdUrl(const udx domain_id, const char * url, const sHiveId & id);
            static bool mapDomainIdUrl(const char * domain_id, const char * url, const sHiveId & id);
            static void mapDomainReset();

        private:
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
};

#endif
