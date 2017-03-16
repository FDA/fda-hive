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
#ifndef sLib_vioDB_hpp
#define sLib_vioDB_hpp


#include <slib/core.hpp>
#include <slib/std.hpp>


class sVioDB
{
    public:
        sVec <idx> arr;
        sStr filename;
        sHash hashTable;
        

        struct LST{
            idx cnt;
            idx ofs;
        };
        struct RLST{
            idx cnt;
            idx ofs;
            idx typeIndex;
        };


        struct COLUMN{
            idx ifHash;
            idx typeIndex;
            idx relCnt;
            idx ctype;
            sVec <idx> colIndLIST;
            sVec <idx> relIndLIST;
        };

        struct TYPE{
            idx ifHash;
            idx ctype;
            idx relCnt;
            sVec <idx> typeIndLIST;
        };

        sVec < sVec <idx> > recordContainerArray;
        sVec < sVec <idx> > recordBodyContainerArray;
        sVec < sVec <idx> > recordRelationContainerArray;
        sVec < sVec <idx> > tmapContainerArray;
        sDic <idx> Rank;

        struct TypeList{
            idx ctype;           //type
            idx cnt, ofs, relCnt, bodyofs,bodyCnt,tnameOfs, tnameSizeOrHash,index;//relCnt=relation for each type; cnt=each type has how many records; ofs=type begin
            RLST rels[1];        
        };
        struct RecordType{
            idx size;
            idx ofs;
            LST rels[1];
        };
        enum ctype{ 
            eInt=   1,
            eDouble=2,
            eString=3,
            eBool=  4,
            eOther= 5,
        };

        enum hashType{
            hNotSpecified=0,
            hReg = 9,
            hNone = 10,
            hInt = 11,
            hFloat= 12,
            hString=13,
            hOther= 14,
        };



        TypeList * GetTypePointer(idx typeIndexOneBased);
        RecordType * GetRecordPointerByIndex(idx typeIndexOneBased, idx recordIndexOneBased);

        static idx spaceForFlags, pointerForCntType, pointerForMaxRel, spaceForConstructInfo,pointerForMapSize,pointerForMapOfs;
        static const char * vioDBFileMarker;
        idx fileOpenMode;
        
    public: 
        virtual ~sVioDB();

        sVioDB(const char * Filename = 0, idx defaultFlag = sMex::fReadonly, const char * FileFlag = 0)
            : arr(defaultFlag)
        {
            fileOpenMode = defaultFlag;
            if( Filename ) {
                init(Filename, FileFlag, 0, 0, fileOpenMode);
            }
        }
        sVioDB(const char * Filename, const char * FileFlag, idx cntTypeObj, idx maxRel)
            : arr(sMex::fSetZero | sMex::fBlockDoubling)
        {
            fileOpenMode = sMex::fSetZero | sMex::fBlockDoubling;
            if( Filename ) {
                init(Filename, FileFlag, cntTypeObj, maxRel, fileOpenMode);
            }
        }
        sVioDB * init(const char * Filename, const char * FileFlag, idx cntTypeObj, idx maxRel, idx lfilemode = sMex::fSetZero | sMex::fBlockDoubling)
        {
            fileOpenMode=lfilemode;

            sStr filenamebuf;
            filename.cut(0);
            filename.printf("%s",Filename);
            arr.setflag(fileOpenMode);
            arr.init(filename.ptr());
            
            filenamebuf.printf(0,"%s_hash",filename.ptr());
            if(cntTypeObj && maxRel){
                arr.cut(0);
                //idx len=sLen(FileFlag);
                arr.add(spaceForFlags);
                char * space=(char *)(arr.ptr(1));
                strcpy(space,vioDBFileMarker);
                if(FileFlag)strncpy(space+sLen(vioDBFileMarker),FileFlag,(spaceForFlags-1)*sizeof(idx)-sLen(vioDBFileMarker));
                //if(0 < len && len < (idx)(spaceForFlags*sizeof(idx)) )
                //    strcat((char *)(arr.add(spaceForFlags)+1),FileFlag);
                //else strcpy((char *)(arr.add(spaceForFlags)+1),"UnknownFileFlag");
                arr.add(12);
                *arr.ptr(pointerForCntType)=cntTypeObj;//Here we store how many data type we have in our file format
                *arr.ptr(pointerForMaxRel)=maxRel;//Here we store how many data type we have in our file format

                sFile::remove(filenamebuf);
                hashTable.init(filenamebuf,sMex::fSetZero|sMex::fBlockDoubling);
            }
            else {
                if(!arr.dim())
                    return 0;

                if(!arr[0]) // we will be lazy here and not map hashtable until needed
                    ;// hashTable.init(filenamebuf,fileOpenMode);
                else{
                    if(arr.ptr(pointerForMapSize)&&arr.ptr(pointerForMapOfs)&&arr[pointerForMapSize]&&arr[pointerForMapOfs]){
                        hashTable.hashPtr=(sHash::HashRecord *)arr.ptr(*arr.ptr(pointerForMapOfs));
                    }
                } 

            }
            hashTable.keyfunc=(sHash::keyFuncType)getRecordFunction;
            return this;
        }


        char * userSpace8(void)
        {
            return (char * ) arr.ptr(3);
        }


        bool deleteAllJobs(bool ifJustUnmap = false);
        bool renameAllFiles(const char *newfilename);
        bool readonlyAllRecord(void);

        bool isok(const char * FileFlag=0)
        {
            if(!arr.dim())return false;
            const char * space = (const char *)arr.ptr(1);
            if(strncmp(space,vioDBFileMarker,sLen(vioDBFileMarker))==0)
                if(FileFlag && *FileFlag){
                    return strcmp(space + sLen(vioDBFileMarker),FileFlag)==0 ? true : false;
                }
                else {return true;}
            else return false;
        }
        void AddType(sVioDB::ctype type,idx relCnt,idx * relationlist,const char * tayename, idx indexOneBased,sVioDB::hashType hashtype=hNotSpecified);
       
        idx AddRecord(idx typeIndexOneBased, const void * record, const idx recordsize);

        idx AddRecordRelationshipCounter(idx typeIndexOneBased,idx recordIndexOneBased, idx relationIndexOneBased);

        bool SetRecordIndexByBody(const void * body,idx typeIndexOneBased, idx * recordIndexOneBased, idx bodysize=0);
        idx GetRecordIndexByBody(const void * body, idx typeIndexOneBased, idx bodysize=0);
        
          bool AllocRelation();
          bool AddRelation(idx typeIndexOneBased_from, idx typeIndexOneBased_to, idx reocrdIndexOneBased_from, idx recordIndexOneBased_to);//,idx relationIndexOneBased_to);
          bool Finalize(bool ifGlueTheFile=false);
       // static bool JoinVioDBFiles(const char * fileFrom=0, const char * fileTo=0, bool ifGlueTheFile=false);
        // how many types of data are in this vioDB: example: taxids, names, ranks 
        idx GetTypeCnt();
        // how many records does this type have 
        idx GetRecordCnt(idx typeIndexOneBased);
        // how many relations of which kind does this particular record GetRelationCnt(taxid , recrodnumer, 1); 1 being theparent child relation for example
        idx GetRelationCnt(idx typeIndexOneBased, idx recordIndexOneBased, idx relationIndexOneBased);
          
        // retrieve the body of a record of a sepcified type 
        void * Getbody(idx typeIndexOneBased, idx recordIndexOneBased, idx * bodysize=0);

        // return the list of relationships of a particular record 
        idx * GetRelationPtr(idx typeIndexOneBased, idx recordIndexOneBased, idx relationIndexOneBased, idx * relationCnt=0, idx * relationTypeIndex=0);

        static void * getRecordFunction(sVioDB * db, idx typeindex, idx recordindex,idx * sizeKey)
        {
            return db->Getbody((idx)typeindex, (idx)recordindex,(idx* )sizeKey);
        }



        idx concatTypes(sVioDB * src );
        idx concatRecordsAndRelationshipCounts(sVioDB * src , idx * countsForFileTypes=0);
        idx concatRelationships(sVioDB * src, idx * countsForFileTypes  );
        idx concatFiles( const char * dstfile, const char * filenames , const char * fileMarker, bool ifGlueTheFile=false , bool removeOriginals=false);
        
        typedef idx (* sVioDBSorterFunction)(void * param, void * ob1, void * obj2 , void * objSrc,idx i1,idx i2);

        static idx ComparatorWrapper(void * param, void * arr, idx i1,idx i2 );
        void viodDBSorter( idx typeIndexOneBased, idx relationIndexOneBased, sVioDBSorterFunction myCallback , void * param );
        void viodDBSorter( idx typeIndexOneBased, idx * indexes, sVioDBSorterFunction myCallback , void * myParam );
        void vioDBRelationSorter (idx * relPtr, sVioDBSorterFunction myCallback , void * myParam, idx fromType, idx typeTo, idx recordFrom, idx relationCnt );


        typedef idx (* genericParserForVioDB)(void * usrObj, sVec<sVar> & buf, idx usrIndex,  sDic< sVec<sStr> > &columnListDic,  idx runNumber, void * param);
        typedef void * (* genericFileInitForVioDB)(const char * fileBody, idx fileLength, const char * separtor, const char * filename, void * param);
        typedef void (* genericFileDestoryForVioDB)(void* usrObj);
        static void parsefileToVioDBFormat(const char* controlFileName, const char* sVioDBType, const char* inputfilename, bool combinedFile, sVec <genericParserForVioDB> * ParserFunctionList, sVec <genericFileInitForVioDB> * fileInitList, sVec <genericFileDestoryForVioDB> * fileDestroyList, const char * srcFileNameListCommaSep, const char * tableSeprator = 0, void * param=0);


        static idx genericTableParserForVioDB(void * usrObj, sVec<sVar> &buf, idx usrIndex, sDic< sVec<sStr> > &columnListDic, void * param=0);
        static void * genericTableFileInitForVioDB(const char * fileBody, idx fileLength, const char * separtor, const char * filename );
        static void genericTableFileDestoryForVioDB(void* usrObj);


        struct MyParamStructure {
            sVioDB * viodb;
            idx fromType;
            idx typeTo;
            sVioDBSorterFunction externalCallback;
            void * externalParam;
            idx recordFrom;
        };
        


};  
#endif

