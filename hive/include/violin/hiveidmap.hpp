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
#ifndef sHiveIdMap_hpp
#define sHiveIdMap_hpp

#include <slib/std.hpp>
#include <ssci/bio.hpp>
#include <ulib/ulib.hpp>

namespace sviolin
{
    class sHiveIdMap
    {
        public:
            sHiveIdMap (sUsr & usr,const char * flag=0)
                :m_usr(usr)
            {
                getMapInit(flag);
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
            idx getMapInit(const char * flag)
            {
                init();
                //Although objlist will not change for one obj, still do it everytime this function was called
                fileNameList.cut(0);
                sHiveId latestId;
                getLatest(latestId);
                idx cnt = 0;
                std::auto_ptr<sUsrObj> obj(m_usr.objFactory(latestId));
                regex_t re;
                idx regerr = flag ? regcomp(&re, flag, REG_EXTENDED|REG_ICASE) : true;

                if( obj->Id() ) {
                    sStr fileNames;
                    obj->propGet00("file", &fileNames);
                    for(const char * ptr = fileNames.ptr(0); ptr && *ptr; ptr = sString::next00(ptr)) {
                        if(!regerr){
                            bool match= (regexec(&re, ptr, 0, NULL, 0)==0) ? true : false;
                            if(!match)continue;
                        }
                        if(cnt)   fileNameList.printf(",");
                        sStr buf;
                        obj->getFilePathname(buf,ptr);
                        fileNameList.printf("%s",buf.ptr(0));
                        cnt++;
                    }
                }
                if( cnt ) {
                    idMap.init(fileNameList);
                }
                return cnt;
            }

            idx getIdMap(const char * idFrom, const char * idtypeFrom, const char * idTo, sStr & buf, idx MaxLevelAllowed = 0, idx maxCnt = sIdxMax, idx start = 0);
            bool getNeibourByDepth(const char * idRoot, const char * rootType, sStr &buf, idx depth = 1, idx maxCnt = sIdxMax, idx start = 0);
            idx findId(const char * idFrom, const char * idTo, sStr & buf, const char * idtypeFrom = 0, idx maxCnt = 1, idx start = 0, sStr * giNumber = 0)
            {
                return idMap.findId(idFrom, idTo, buf, idtypeFrom, maxCnt, start, giNumber);
            }

        private:
            void init()
            {
                if( !objIdList.dim() ) {
                    m_usr.objs2("special", objIdList, 0, "meaning", "id_mapping", "version,file");
                }
            }

            sUsr & m_usr;
            sUsrObjRes objIdList;
            sStr fileNameList;

            sIdMap idMap;
    };
}
#endif // sHiveIdMap_hpp
