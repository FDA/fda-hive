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
#ifndef sLib_IDMAP_hpp
#define sLib_IDMAP_hpp

#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>

class sIdMap
{
    public:
        sIdMap(const char * InputFilename=0)
        {
            if(InputFilename)
                init(InputFilename);
        }

        ~sIdMap()
        {
            for(idx i=0;i<dbList.dim();i++){
                dbList[i].deleteAllJobs(true);
                dbList[i].arr.cut(0);
                dbList[i].arr.destroy();
            }
        }

        sIdMap * init (const char * InputFilename=0)
        {
            sStr nameListbuf;
            sString::searchAndReplaceSymbols(&nameListbuf,InputFilename,0,",",0, 0,true , true, false , true);
             const char * srcName=nameListbuf.ptr();
             for(;srcName;){
                sVioDB *db = dbList.add(1);
                db->init(srcName,0,0,0,sMex::fReadonly);
                srcName = sString::next00(srcName);
             }
             return this;
        }
        bool isok(void)
        {
            for(idx i=0;i<dbList.dim();i++){
                if(!dbList[i].isok())   return false;
            }
            return true;
        }

        struct QC{
                sStr type;
                sStr id;
        };

        bool findIdTypeAuto(const char * id, sVec<QC> & idIndexWithDB, const char * idTypeFrom =0 );
        idx getIdMap(const char * idFrom, const char * idtypeFrom, const char * idTo, sStr &buf,  idx MaxLevelAllowed, idx maxCnt=sIdxMax, idx start=0,idx maxExpand=50);
        bool getNeighbourByLevel(const char * startId,const char * idTypeFrom, sStr &buf, idx level=1, idx maxCnt=sIdxMax, idx start = 0,idx maxExpand=50);
        idx findId(const char * idInput, const char * idTo, sStr &buf, const char * idtypeFrom=0, idx cnt=1, idx start=0,sStr *giNumber=0);
        static idx findIdByInput(const char * idInput, const char * idTo, sStr &buf, const char * idtypeFrom=0, idx cnt=1);
        static void Parsefile(const char* src,  const char* outfilename,bool combinedFile=true, const char * keyType=0);


        static idx setBodyWithType(idx typeRecordIndex, sStr & body);
        static idx getBodyWithType(idx typeRecordIndex, sStr & body);

    private:
        idx getQCFromAllVioDB(const char * id, sVec<QC> & qcList,idx idSize = 0);
        bool findIdType(idx bodyIndex, sStr & idtype, sVioDB &db);

        idx findTypeIndex(const char * type, sVioDB & db, idx typeSize = 0);
        void getTypeBodyByIndex(sStr & buf,idx indexOneBase, sVioDB & db);
        void getNeighbour(const QC * root,sVioDB * db,idx level, sStr &list, sDic<idx> &table, idx maxCnt, idx start, idx * cnt, idx maxExpand);
        void getIdRealBody(const char * idWithType,idx recordIndex, sStr &realBody, sVioDB & db, idx idSize );


        static const char * getonelinefromBuffer(char * buf, const char * fileContent);
        static void ParseOneline(const char * buf, sStr & key, sStr &otherType, sStr &other);
        static void ParseOnelineNCBI(const char * buf, sStr & key, sStr &otherType, sStr &other);
        sVec <sVioDB> dbList;
        static const char * idMapVioDBFileFlag;
        static const char * excludeList;
};

#endif
