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
#ifndef sTaxTreeMap_hpp
#define sTaxTreeMap_hpp

#include <slib/std.hpp>
#include <ssci/bio.hpp>
#include <ulib/ulib.hpp>

namespace sviolin {
    class sTaxTreeMap
    {
        public:
            sTaxTreeMap(sUsr & usr)
                : m_usr(usr)
            {
            }
            bool getLatest(sHiveId & out_latestId)
            {
                init();
                //Although objlist will not change for one obj, still do it everytime this function was called
                out_latestId.reset();
                udx maxV = 0;
                for(sUsrObjRes::IdIter it = objIdList.first(); objIdList.has(it); objIdList.next(it)) {
                    const sUsrObjRes::TObjProp * obj =  objIdList.get(it);
                    const sUsrObjRes::TPropTbl * tbl = objIdList.get(*obj, "version");
                    while( tbl ) {
                        udx versionU = 0;
                        const char * v = objIdList.getValue(tbl);
                        if( v ) {
                            sscanf(v, "%" UDEC, &versionU);
                        }
                        if( versionU && maxV < versionU ) {
                            maxV = versionU;
                            out_latestId = *objIdList.id(it);
                        }
                        tbl = objIdList.getNext(tbl);
                    }
                }
                return out_latestId;
            }
            idx getFileName(sStr & buf)
            {
                init();
                //Although objlist will not change for one obj, still do it everytime this function was called
                buf.cut(0);
                sHiveId latestId;
                getLatest(latestId);
                if( !latestId ) {
#ifdef _DEBUG
                    fprintf(stderr, "%s:%u: Cannot find latest ID\n", __FILE__, __LINE__);
#endif
                    return 0;
                }
                idx cnt = 0;
                std::auto_ptr<sUsrObj> obj(m_usr.objFactory(latestId));
                if( !obj.get() ) {
#ifdef _DEBUG
                    fprintf(stderr, "%s:%u: Cannot get object with ID %s\n", __FILE__, __LINE__, latestId.print());
#endif
                    return 0;
                }
                if( obj->Id() ) {
                    sStr fileNames;
                    obj->propGet00("file", &fileNames);
                    cnt = sString::cnt00(fileNames);
                    if(cnt==1)
                        obj->getFilePathname(buf,fileNames.ptr());
#ifdef _DEBUG
                    else
                        fprintf(stderr, "%s:%u: file cnt can not be %" DEC ", can only be 1 ", __FILE__, __LINE__, cnt);

#endif
                }
                return cnt;
            }
        private:

            void init()
            {
                if( !objIdList.dim() ) {
                    m_usr.objs2("special", objIdList, 0, "meaning", "^taxonomy_tree$", "version,file");
                }
            }

            sUsr & m_usr;
            sUsrObjRes objIdList;
    };
}
#endif // sTaxTreeMap_hpp
