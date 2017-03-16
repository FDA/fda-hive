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
#ifndef sLib_core_olst_h
#define sLib_core_olst_h

#include <slib/core/dic.hpp>

namespace slib
{
    class sObjList
    {
        public:
            sObjList()
                {}
            ~sObjList()
                {}
            sObjList* init(const char * flnm = 0)
            {
                m_dic.init(flnm);
                return this;
            }
            /**
             * Add an object to list
             * @return index != sNotIdx if success
             */
            template<class TObj> idx set(TObj& obj)
                {
                    idx ret = sNotIdx;
                    const void* key = 0;
                    idx key_len = 0;
                    sMex::Pos* p;

                    obj.GetKey(&key, &key_len);
                    if( key != 0 && key_len != 0 && (p = m_dic.set(key, key_len, &ret)) != 0) {
                        p->size = sizeof(TObj);
                        p->pos = m_dic.mex()->add(&obj, sizeof(TObj));
                    }
                    return ret;
                }
            /**
             * Add a key to an object already in the list
             */
            idx dict(idx index, const void* key, idx key_len = 0)
                {
                    return m_dic.dict(index, key, key_len);
                }
            /**
             * Remove item from list (make it not searchable)
             */
            void undict(idx index)
                {
                    m_dic.undict(index, 1);
                }
            /**
             * Retrieve number of objs in list
             */
            idx dim(void)
                {
                    return m_dic.dim();
                }
            /**
             * Drop list content
             */
            void empty(void)
                {
                    m_dic.empty();
                }
            /**
             * Retrieve obj by its index
             */
            template<class TObj> TObj* ptr(idx index) const
                {
                    TObj* o = 0;
                    sMex::Pos* p = m_dic.ptr(index);
                    if( p != 0 ) {
                        o = (TObj*)(m_dic.mex()->ptr(p->pos));
                    }
                    return o;
                }
            /**
             * Retrieve obj by its key
             */
            template<class TObj> TObj* get(const void* key, idx key_len = 0, idx* pNum = 0) const
                {
                    TObj* o = 0;
                    sMex::Pos* p = m_dic.get(key, key_len, pNum);
                    if( p != 0 ) {
                        o = (TObj*)(m_dic.mex()->ptr(p->pos));
                    }
                    return o;
                }
        protected:
            sDic<sMex::Pos> m_dic;
    };

}

#endif // sLib_core_olst_h
