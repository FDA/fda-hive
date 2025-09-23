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
#ifndef sLib_sVax_hpp
#define sLib_sVax_hpp


#include <slib/core/str.hpp>
#include <slib/std/file.hpp>
#include <fcntl.h>

namespace slib {
    struct WordList{
            const char * word;
            idx len;
    };

    class sFlax
    {
        public:

            void * filePointer;
            int fileHandle;
            sFil mapFile;
            const char * srcStart,*srcEnd;
            idx flags;
            enum eFlags {
                fDestroySource=0x01,
                fUseMMap=0x02,
                fUseFStream=0x04,
                fUseIOStream=0x08,
            } ;
            sMex recBuf;
            sStr varBuf;
            const char * recStart,* recNext;
            idx recLen,reMapChunk,mapOfs;
            sFilePath flName;
            void * callbackParam;
            idx flNmPos;

            sIO * gLog; idx prgLast;
            typedef idx (*callbackProgressReportType)(sFlax* vax, void * param, idx prg, idx prgPerc, sIO * glog);
            callbackProgressReportType callbackProgress;
            static idx progressReport(sFlax* vax, void * param, idx prg, idx prgTot, sIO * genLog)
            {
                idx prgCur=prgTot ? prg*100/prgTot :0;
                if(genLog) {
                    if(vax->prgLast>=prgCur)return vax->prgLast;
                    if(prgCur>100)prgCur=100;
                    vax->prgLast=prgCur;
                    genLog->printf("progress: %" DEC "/%" DEC " %" DEC "%%                                        \r",prg,prgTot,vax->prgLast);
                    if(prgCur>=100)
                        genLog->printf("\n");
                }
                return prgCur;
            }

            sFlax(void){init(0,0,0,0);}
            sFlax(idx flagSet, const char * src, idx len ){init(flagSet,src,len,0);}
            sFlax(idx flagSet, void * fileP){init(flagSet,0,0,fileP);}
            sFlax(idx flagSet, const char * fileName){init(flagSet,fileName);}

            ~sFlax() {
                destroy();
            }

            void destroy(void)
            {
                resetData();
            }

            sFlax * init(idx flagSet, const char * fileName )
            {
                resetData();

                prgLast=-1;
                gLog=0;
                if( fileName ) {
                    flName.addString(fileName,sLen(fileName));

                    if( fileName[0]=='\\' ){
                        init(flagSet,fileName, sLen(fileName),0);
                    }

                    if(flagSet&fUseMMap) {
                        mapFile.init(fileName, sMex::fReadonly);
                        init(flagSet,mapFile.ptr(), mapFile.length(),0);
                    }if(flagSet&fUseFStream) {
                        FILE * fp= (fileName[0]=='.' && fileName[1]==0) ? stdin : fopen(fileName,"r");
                        init(flagSet|fDestroySource,0,0,fp,0);
                    }if(flagSet&fUseIOStream) {
                        int fh=open(fileName,O_RDONLY,0);
                        init(flagSet|fDestroySource,0,0,0,fh);
                    }
                }

                return this;
            }

            sFlax * init(idx flagSet, const char * src, idx len , void * fileP=0, int fileH=0)
            {
                flags=flagSet;
                srcStart=src;
                srcEnd=srcStart+len;
                recNext=srcStart;
                reMapChunk=0;
                mapOfs=0;

                filePointer=fileP;
                fileHandle=fileH;
                flNmPos=0;
                callbackProgress=0;
                return this;
            }

            void resetData(void )
            {
                if(flags&fDestroySource) {
                    if( filePointer) {
                        if(filePointer!=stdin)
                            fclose((FILE*)filePointer);
                        filePointer=0;
                    }
                    if( fileHandle) {
                        close(fileHandle);
                        fileHandle=0;
                    }
                    if( (!(flags&fUseMMap)) && srcStart) {
                        sDel(srcStart);
                        srcStart=0;
                    }
                    flNmPos=0;
                }
                mapFile.destroy();
            }


            idx ensureRecordBuf(const char * seprec=0);
            static void spaceAndQuoteCleanup (const void ** recordBody, idx * recordSize, sMex * clnEscapeBuf=0);

    };




    class sJax: public sFlax {
            sMex valBuf;
            sDic < sMex::Pos > vars;
        public:
            struct JsonFrame {
                    const char * varOfs, * valOfs;
                    idx varSize,valSize;
                    char type, op ;
                    idx userData,varNum,depth,rowNum,lineNumber;
            };
            idx varCounter, lineCount, errPos, lineStart,nextLine;const char * errCode;
            bool parseJson;
        private:
            sVec < JsonFrame > Scope;
            sVec < JsonFrame > * scope;

            idx curBufSize, arrayMode;

            sJax * init0(void) {
                errPos=0;
                lineCount=0;
                arrayMode=0;
                callbackFuncVariable=0;
                callbackFuncValue=0;
                callbackParam=0;
                stopOnErrors=1;
                errCode=0;
                scope=&Scope;
                expected=0;
                fr=0;prv=0;
                parseJson=true;
                return this;
            }
        public:
            void empty() {
                Scope.empty();
                vars.empty();
                valBuf.cut(0);
                errIO.cut(0);
                jsonNodes.empty();
            }
            JsonFrame * fr, *prv;
            idx expected;
        public:
            sIO errIO;
            idx stopOnErrors;

            sJax(void){init0();sFlax::init(0,0,0,0);}
            sJax(idx flagSet, const char * src, idx len ){init0();sFlax::init(flagSet,src,len,0);}
            sJax(idx flagSet, void * fileP){init0();sFlax::init(flagSet,0,0,fileP);}
            sJax(idx flagSet, const char * fileName){init0();sFlax::init(flagSet,fileName);}

            enum eParseMode { eModeVar, eModeVal, eModeObj, eModeArr };


            typedef idx (*callbackAnalyzeConstruct)(sJax * jax, void * param, JsonFrame * fr,JsonFrame * prv, sIO * err);
            callbackAnalyzeConstruct callbackFuncVariable,callbackFuncValue;


            idx analyzeVariable(JsonFrame * fr,JsonFrame * prv)
            {
                if(callbackFuncVariable)
                    return callbackFuncVariable(this,callbackParam,fr,prv,&errIO);
                return 1;
            }
            idx analyzeValue(JsonFrame * fr,JsonFrame * prv)
            {
                if(parseJson)
                    this->analyzeJsonValue(fr,prv);
                if(callbackFuncValue)
                    return callbackFuncValue(this,callbackParam,fr,prv,&errIO);
                return 1;
            }

            class JsonNode {
                public:
                sJax * json;
                JsonFrame * fr;
                const char * val(idx * psz=0){
                    const void * body=fr->valOfs; idx sz=fr->valSize;
                    sFlax::spaceAndQuoteCleanup (&body, &sz );
                    if(psz)*psz=sz;
                    return (const char*)body;
                };
            };
            sDic < JsonNode > jsonNodes;

            idx analyzeJsonValue(JsonFrame * fr,JsonFrame * prv);
            const char * jsonValue(const char * key, idx len=0, idx * psz=0){
                if(!len)len=sLen(key);
                JsonNode* js=jsonNodes.get(key,len);
                if(!js){
                        if(psz)*psz=0;
                        return 0;}
                return js->val(psz);
            }
            const char * jsonValue(sStr * buf, const char * key, idx len=0){
                idx sz;
                const char * p=jsonValue(key,len,&sz);
                if(p && sz) { 
                    idx l=buf->length();
                    buf->add(p,sz);
                    buf->add0(1);
                    return buf->ptr(l);
                }
                return 0;
            }
            idx jsonIValue(const char * key, idx len=0){
                idx sz,r;const char * p=jsonValue(key,len,&sz);
                if(!sz || !p)
                    return sNotIdx;
                sIScanf(r,p,sz,10);
                return r;
            }


            idx ensureRecordBuf(void);
            idx parse(void);
            idx include(const char * p);
            void errorReport(sIO * err, idx pos=0){
                if(!pos)pos=flNmPos;
                err->printf("ERROR in %s: %s\n",flName.ptr(pos),errCode);
                err->printf("%" DEC " %.*s\n",lineCount+1,(int)(nextLine-lineStart),lineStart+srcStart);
                err->printf("%" DEC " %.*s\n",lineCount+2,128,nextLine+srcStart);
                if(err!=&errIO && errIO.length())
                    err->printf("%s", errIO.ptr());
            }
    };





    class sVax: public sFlax
    {

        public:

            enum eFlags {
                fSupportQuotes = 0x0100000000,
                fDoNotSupportVaxHeader=0x0200000000,
                fSupportMultipleVaxHeader=0x0400000000,
                fDoNotSupportTableHeader=0x0800000000,
                fDoNotPreparseCellOffsets=0x1000000000,
                fParseImmediate=0x10000000000,
                fStopAfterHeaders=0x20000000000,
                fDoNotUseQuoteProtection=0x40000000000
            } ;

            idx separField, separRecord, separAttribs;
            const char * vaxMarker, * commentMarker, *inclusionNames,*exclusionValues,*exclusionNames;
            sDic < char > * nameInclusionDic;
            idx maxNumRecords;
            idx isInHeader;
            idx recordStart, recordCnt, recordCur;




            typedef idx (* callbackFunc)(void * vaxObj, void * param, const char * buf, idx len );
            struct callbackHeaderStruct{
                void * param;
                callbackFunc func;
            };

            sDic < callbackHeaderStruct > callbackList;
            void addHeaderCallback(const char * key, callbackFunc cb, void * param=0)
            {
                callbackHeaderStruct * cs=callbackList.set(key);
                cs->func=cb;
                cs->param=param;
            }

            struct FieldInfo {
                idx iCol;
                idx isValid;
                enum eDefValue {
                    eDefValueNone=0,
                    eDefValueDirect,
                    eDefValueIncrement,
                    eDefValuePrevious,
                    eDefValueReference
                };
                enum eValueType {
                    eValueTypeString=0,
                    eValueTypeInteger,
                    eValueTypeReal,
                    eValueTypeBoolean
                };

                idx defValuePos;
                idx defValueType, defValueSize;
                idx valueType, valueUnique;
                idx keyValSeparator;
                idx internalAttrStart, internalAttrCnt;


                const char * ofs;
                idx size;

                FieldInfo(void)
                {
                    iCol=sNotIdx;
                    valueType=-1;
                    valueUnique=0;
                    defValuePos=sNotIdx;
                    defValueType=eDefValueNone;
                    defValueSize=0;
                    isValid=1;
                    keyValSeparator=0;
                    internalAttrStart=0;
                    internalAttrCnt=0;

                }
            };
            sDic <FieldInfo> hdrTable;
            idx * internalColumnMap;
            idx internalFieldIterator;
            sDic < const char * > * nameSubstitutionDictionary;


            idx colCnt(void) {
                return hdrTable.dim();
            }

            const char * value(idx icol, idx * psize)
            {
                if(psize)*psize=hdrTable[icol].size;
                return hdrTable[icol].ofs;
            }

            idx iValue(idx icol)
            {

                idx res=0;
                for(const char * pp=hdrTable[icol].ofs, * pe=(hdrTable[icol+1].ofs);*pp>='0' && *pp<='9' && pp<pe; ++pp)
                    res=res*10+(*pp-'0');
                return res;
            }

            idx rValue(idx icol)
            {

                real res=0.;
                idx div=0;
                for(const char * pp=hdrTable[icol].ofs, * pe=(hdrTable[icol+1].ofs);((*pp>='0' && *pp<='9') || *pp=='.') && pp<pe; ++pp) {
                    res=res*10+(*pp-'0');
                    if(*pp=='.')div=1;
                    if(div)div*=10;
                }

                if(div)
                    res/=div;
                return res;
            }

            const char * compositeValue(idx * pidx, idx cnt, sStr * buf, idx * plen, const char * sep=" ")
            {
                const char * p=0;
                idx len,pos=buf->length();
                if( cnt >1 ) {
                    for ( idx icol=0; icol<cnt; ++icol ){
                        if(icol>0)
                            buf->add(sep,1);
                        p=this->value(pidx[icol], &len);
                        buf->add(p,len);
                    }
                    p=buf->ptr(pos);
                    if(*plen)*plen=buf->length();
                } else {
                    p=this->value(pidx[0], plen);
                }
                return p;
            }

            idx nextInternal(idx iter, bool isName, const void * * recordBody, idx * recordSize);
            idx iteratorCol(idx internalIterator){return internalIterator&0xFFFFFFFF;}


            sVax(void){init0();sFlax::init(0,0,0,0);}
            sVax(idx flagSet, const char * src, idx len ){init0();sFlax::init(flagSet,src,len,0);}
            sVax(idx flagSet, void * fileP){init0();sFlax::init(flagSet,0,0,fileP);}
            sVax(idx flagSet, const char * fileName){init0();sFlax::init(flagSet,fileName);}

            sVax * init(idx flagSet, const char * fileName, const char * hdrFile=0)
            {
                if(hdrFile) {
                    sFil f(hdrFile,sMex::fReadonly);
                    if(f.length())
                        tableHeaderParse(f.ptr(),f.length());
                }
                sFlax::init(flagSet, fileName);
                if(flags&fParseImmediate)
                    parse();
                return this;
            }
            idx parse( idx count=1 );

            static idx ionProviderCallback(idx record, void * param, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize){
                return ((sVax*)param)->ionProviderCallback(record, iRecord, fieldType, fieldTypeName, recordBody, recordSize);
            }
            virtual idx ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize);

            idx ionProviderCallbackTable(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize);
            static idx ionProviderCallbackTable(idx record, void * param, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize) {
                return ((sVax*)param)->ionProviderCallbackTable(record, iRecord, fieldType, fieldTypeName, recordBody, recordSize) ;
            }

            sVax * vaxHeaderParse(const char * ptr,idx len=0);
            sVax * tableHeaderParse(const char * ptr,idx len=0);

            void setSepar(const char * sepField, const char * sepRec, const char * sepAttribs, idx colNum=sNotIdx)
            {
                if(colNum==sNotIdx) {
                    if(sepField) {
                        separField=varBuf.length();varBuf.add(sepField,sLen(sepField)+1);
                    }
                    if(sepRec) {
                        separRecord=varBuf.length();varBuf.add(sepRec,sLen(sepRec)+1);
                    }
                    if(sepAttribs) {
                        separAttribs=varBuf.length();varBuf.add(sepAttribs,sLen(sepAttribs)+1);
                    }
                }
                else {
                    hdrTable.resize(colNum+1);
                    hdrTable[colNum].keyValSeparator=varBuf.length();varBuf.add(sepAttribs,sLen(sepAttribs)+1);
                }
            }

        protected:
            sVax * init0(void)
            {
                separField=varBuf.length();varBuf.add(",");
                separRecord=varBuf.length();varBuf.add("\n");
                separAttribs=0;
                maxNumRecords=0;
                vaxMarker="#";
                commentMarker=0;
                inclusionNames=0;
                exclusionValues=0;
                exclusionNames=0;
                nameInclusionDic=0;
                internalColumnMap=0;
                internalFieldIterator=0;
                nameSubstitutionDictionary=0;
                recordStart=0;
                recordCnt=0;
                recordCur=0;

                isInHeader=0;
                if( !(flags&fDoNotSupportTableHeader) )
                    ++isInHeader;
                if( !(flags&fDoNotSupportVaxHeader) )
                    ++isInHeader;
                if( flags&fSupportMultipleVaxHeader)
                    isInHeader=sIdxMax;

                return this;
            }


            sVax * init(idx flagSet, const char * src, idx len , void * fileP=0)
            {
                sFlax::init(flagSet,src,len,fileP);
                if(flags&fParseImmediate)
                    parse();
                return this;
            }


            friend class sVaxSet;

            const char * cellScan(const char * ptr, const char * end, idx cellSkip=0);
        public:
            idx ensureRecordBuf(void);


            enum eKeywords {eNumRecords=0,eFieldSeparator,eRecordSeparator,eFieldList};
            enum eAttributes {eAttrDefault=0,eAttrType,eAttrFieldNum,eAttrUnique,eAttrKeyValSeparator};
            static const char * headerKeywords, * headerAttributes, * valueTypes;
    };

    class sVaxSet {
        public:
            sVaxSet(void) {
                activeVax=0;
            }
            static idx ionProviderCallback(idx record, void * param, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
            {
                sVaxSet * vs=(sVaxSet *)param;
                idx res=0;
                while( vs->activeVax<vs->collection.dim() && (res=sVax::ionProviderCallback(record, vs->collection.ptr(vs->activeVax), iRecord, fieldType, fieldTypeName, recordBody, recordSize))<0  )
                    ++vs->activeVax;
                return res;
            }
            static idx ionProviderCallbackTable(idx record, void * param, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
            {
                sVaxSet * vs=(sVaxSet *)param;
                idx res=0;
                while( vs->activeVax<vs->collection.dim() && (res=sVax::ionProviderCallbackTable(record, vs->collection.ptr(vs->activeVax), iRecord, fieldType, fieldTypeName, recordBody, recordSize))<0  )
                    ++vs->activeVax;
                return res;
            }
        protected:
            sVec < sVax > collection;
            idx activeVax;
        public:
            sVax * newVax(void){
                return collection.add();
            }

    };
};

#endif

