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
#ifndef sLib_sIon_hpp
#define sLib_sIon_hpp


#include <slib/core.hpp>
#include <slib/std.hpp>

namespace slib {

    class sTbl;
    class sTxtTbl;
    class sIon
    {
        friend class sIonWander;
        friend class sIonBirel;
        friend class sIonTransaction;

        static const idx sizeofIdx;

        public:
        enum eCType {
            eCTypeIdx=0,
            eCTypeReal,
            eCTypeString,
            eCTypeBool,
            eCTypeOther,
            eCTypeIndexOnly,
            eCTypeIdxRange,
            eCTypeMax
        };


        private:

        static const char * ctypelist;;

        public:
        enum eHashType {
            eHashTypeNone=0,
            eHashTypeIdx,
            eHashTypeOrdered,
            eHashTypeString,
            eHashTypeOther,
            eHashTypeMax
        };

        private:
        static const char * hashtypelist;

        struct Record{
            idx size;
            idx ofs;
        };


        struct RecordType
        {
            idx typeIndex;
            idx typeIndexContainer;
            eCType cType;
            eHashType hashType;
            idx nameOfs, nameOfsContainer;
            idx flags;
            idx autoIncrement;
            idx typeIndexAlias;
            idx typeValueSynonyms00;
            idx reserve[28];
        };

        struct RelationType
        {
            idx typeIndex;
            idx typeIndexContainer;
            idx nameOfs , nameOfsContainer;
            idx ofsRelationTargets;
            idx cntRelationTargets;
            idx cntRelationHashTypes;
            idx cntRelationsAll;
            enum eRelationType {
                bRelation64Bit=0,
                bRelationReverseLookup
            };
            idx flags;
            idx reserve[32];
        };

        struct Header {
            char reserved[1024-0*sizeof(idx)];
        };

        struct Containers {

            sVec < Record > recordsArr;
            sMex relationsArr;

            sMex recordBodyContainer;
            sMex relationBucketContainer;

            sHash2  relationHashContainer;

            sHash2 recordHashContainer;
            sIndex< sIndexStruc> recordIndexContainer;

            sDic < sMex > associatedContainers;
        };
        struct Configuration{
                idx sizeExpectation;
        };

        public:
            struct RecordResult {

                const void * body;
                idx size;
                idx typeIndex;
                idx cType;
                idx index;

                eCType ccType(void){return (eCType)(cType&(0xFFFFFFFF)); }
                idx flags(void){return cType>>32; }
                idx recordTypeIndex(void){return typeIndex&(0xFFFFFFFF); }
                idx relationIndex(void){return typeIndex>>32; }

            };


            idx openMode,openHashMode;
        private:
            sStr baseName;

            const char * extension;
            sMex baseContainer;
            sVec <RecordType> recordTypesArr;
            sVec < RelationType > relationTypesArr;
            sVec < idx > relationTargetsArr;
            enum eRARHash{eRARHash_record=0,eRARHash_relation=1};
            sHash2 recordAndRelationTypesHash;
            sVec < Containers > typeContainers;
            enum eRelationTypeFlags {
                fRelationTypeIndex=0x40000000
            };
            sDic < Configuration > configuration;
            sVec < sIon::RecordResult > resultSetForEngines;


        private:
            idx getRecordByHash(RecordType * pRecordType, const void * recordBody, idx recordSize , idx * pHash=0);
            idx setRecordHash(RecordType * pRecordType, const void * recordBody, idx recordSize, idx recordIndex);
            static void * getRecordTypeKeyFunction(void * param, sHash2::hashtypeIdx typeIndexAs0, idx recordTypeIndex,idx * pRecordTypeNameSize, sMex * body);
            static void * getRecordKeyFunction(void * param, sHash2::hashtypeIdx recordTypeIndex, idx recordIndex,idx * pRecordSize, sMex * body);
            idx getHashBodyForRelation(sMex * toHash, RelationType * pRelationType, idx relationOfsInInt, sMex * relationsArr, idx relStart);
            static void * getRelationKeyFunction(void * param, sHash2::hashtypeIdx typeRelationIndex, idx relationIndex,idx * pRelationHashBodySize, sMex * body);

        protected:

            idx addRecordType(const char * typeName,eCType cType,eHashType hashType, const char * tvs=0);
            void aliasRecordTypes(idx dstRecordType,idx srcRecordType);
        public:
            idx addRecord(idx recordTypeIndex, idx recordSize , const void * recordBody);
            idx getRecordTypeIndex(const char * typeName){return recordAndRelationTypesHash.find(eRARHash_record,typeName,sLen(typeName)+1);}
            idx autoNumber(idx recordTypeIndex, idx range=1);
            const void * getRecordBody(idx recordTypeIndex, idx recordIndex , idx * * pRecordSize=0 , sStr * bodyText=0);
        protected:


            idx addRelationType(idx relationTypeFlags, const char * relationName, ... )
            {
                va_list marker;va_start(marker, relationName);
                idx ret=addRelationTypeVarg(relationTypeFlags,relationName, (const char **)0, marker ,(idx)0);
                va_end( marker );
                return ret;
            }

        public:

            idx addRelationTypeVarg(idx relationTypeFlags, const char * relationName, const char  * * markerArr , va_list marker, idx directCnt=0);
            idx addRelationVarg(idx relationTypeIndex, idx insertIndex, const idx * markerArr, va_list marker);
            idx addRelation(idx relationTypeIndex, idx insertIndex,... ){
                va_list marker;va_start(marker, insertIndex);
                idx ret=addRelationVarg(relationTypeIndex, insertIndex, 0, marker );
                va_end( marker );
                return ret;
            }
        protected:
            idx getResultSetFromSearchTrajectory(sVec < RecordResult > * resultFinal, const idx * searchTraj, va_list marker, sVec < RecordResult > * resultSet);

        public:

            sIon(const char * baseName=0, idx lopenMode=0) {
                openMode=lopenMode;
                openHashMode=0;
                if(baseName){
                    if(openMode&sMex::fMapRemoveFile)deleteIonContainers(baseName);
                    init(baseName,openMode);return;
                }

            }
            ~sIon(){destroy();}

            sIon * init(const char * baseName, idx fileOpenMode);
            idx deleteIonContainers(const char * baseName);
            void destroyDependencies(void);

            void expect(const char * record_or_relationName, idx size);
            void destroy(){
                baseContainer.destroy();
                baseName.destroy();

                recordTypesArr.destroy();
                relationTypesArr.destroy();
                relationTargetsArr.destroy();

                if(openMode&sMex::fReadonly){
                    for( idx ir=0; ir<typeContainers.dim(); ++ir) {
                        typeContainers[ir].recordsArr.cutM(0);
                    }
                }
                typeContainers.empty();
                typeContainers.destroy();
                configuration.destroy();
                resultSetForEngines.destroy();
                recordAndRelationTypesHash.destroy();

                destroyDependencies();
            }
            bool ok(){return baseContainer.ok();}

            idx getRecordCount (idx recordTypeIndex);
            idx dimRecordTypes() const { return recordTypesArr.dim(); }

            idx constructRecordAndRelationTypes(const char * defTypes, idx len );
            static idx constructRecordAndRelationTypes(void * vaxObj, void * ionObj, const char * defTypes, idx len ){return ((sIon*)ionObj)->constructRecordAndRelationTypes(defTypes, len );};
            enum eProviderRecordIndexCommand{eProviderSkip=-3,eProviderDestroy=-2,eProviderInit=-1,eProviderStart=0};

            struct DicRecord {const void * recBody; idx recSize; };
            sDic < DicRecord  > dicLoadConst;
            DicRecord * dicLoadAdd(const char * name, idx nameLen, const void * body, idx siz) { struct DicRecord * dr=dicLoadConst.set(name,nameLen);dr->recBody=body;dr->recSize=siz ? siz: sLen(body); return dr;};
            enum eLoadFlags{fDoNotLoadIncompleteRelation=0,fDoNotLoadEmpty,fValueFormatting};
            typedef idx (*providerCallback)(idx record, void * param, idx iRecord, idx iRecordType, const char * recordTypeName, const void * * recordBody, idx * recordSize );
            idx providerLoad(providerCallback providerFunc, void * param=0, idx maxcnt=0, idx start=0, idx flags=0);

            idx recordTypeCnt(void){return recordTypesArr.dim();}
            const char * getRecordBody (RecordResult * pRecordResult, sStr * bodyText, idx cut=sNotIdx){
                if(cut!=sNotIdx)bodyText->cut(cut);
                if(pRecordResult->cType==eCTypeReal ) {bodyText->printf("%lf",*(real*)pRecordResult->body);}
                else if(pRecordResult->cType==eCTypeIdx ) { bodyText->printf("%" DEC,*(idx *)pRecordResult->body);}
                else {bodyText->printf("%.*s",(int)pRecordResult->size,(const char * )pRecordResult->body);}
                return bodyText->ptr();
            }
            idx getRelationTargetsCount(idx relationTypeIndex){return relationTypesArr.ptr(relationTypeIndex)->cntRelationTargets;}

            struct Bucket {
                    idx relationBucketPos; idx ionDef;
                    sVec < sMex > requiredContainers;
                    sMex * toHash;
                    idx xhash;
                    Bucket(void) {ionDef=0; relationBucketPos=sNotIdx;}
                    bool found(void){return relationBucketPos==sNotIdx ? false: true; }
            };
            static const void * Link_BodyByIndex;
            idx getRelationBucketByHashVarg(Bucket * bucket, idx relationTypeIndex, idx relationHasherIndexType, idx * pCnt, const idx * markerArr, va_list marker=0 , sVec < RecordResult > * stackRecords=0);
            idx getRelationBucketByHash(Bucket * bucket, idx relationTypeIndex, idx relationHasherIndexType, idx * pCnt, ... )
            {
                va_list marker;va_start(marker, pCnt);
                idx res=getRelationBucketByHashVarg(bucket,relationTypeIndex, relationHasherIndexType, pCnt, 0, marker );
                va_end(marker);

                return res;
            }
            idx getNextRelationInBucket(Bucket * bucket, idx relationTypeIndex, idx num);
            idx deleteRelationByHashVarg(Bucket * bucket, idx relationTypeIndex, idx relationHasherIndexType, idx * pCnt, const idx * markerArr, va_list marker=0 , sVec < RecordResult > * stackRecords=0);

            idx getRelationsByBucketAndIndex(Bucket * bucket, idx relationTypeIndex, RecordResult * pRecordResults, idx * pRelCnt=0, bool moveToNext=true );
            idx getRelationResultsByRelationTargets(idx relationTypeIndex, idx * pRelationTargets, RecordResult * pRecordResults );

            void deleteRelation(idx relationTypeIndex,idx relationOffset );
            void deleteRelationBucket(idx relationTypeIndex, Bucket * bucket );
            idx deleteRelationsByBucketAndIndex(Bucket * bucket, idx relationTypeIndex, idx iStart, idx * pRelCnt, RecordResult * pRecordResults);
            void deleteRelationsFromBucket(Bucket * bucket, idx relationTypeIndex, idx iStart, idx relCnt)
            {
                RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
                sHash2 * relationHashContainer=&(typeContainers[pRelationType->typeIndex].relationHashContainer);

                relationHashContainer->bucketDeleteElements(&bucket->relationBucketPos,iStart,relCnt);

            }

        public:

            public:
            enum eInfoTypes{ fInfoRecordTypes=0,fInfoRelationTypes, fInfoRecordSummary, fInfoRelationSummary, fInfoRecords, fInfoRelations};

            struct sIon_InfoCallbacks {
                    typedef idx (* sCallbackRecordType) (sIon * ion, sIO * io, void * param, RecordType * pRecordType );
                    typedef idx (* sCallbackRelationType) (sIon * ion, sIO * io, void * param, RelationType * pRelationType );
                    typedef idx (* sCallbackRecord) (sIon * ion, sIO * io, void * param, RecordType * pRecordType, const void * pRecordBody, idx recordSize , idx iLine, idx iRecOrderInRelation);
                    typedef idx (* sCallbackRelation) (sIon * ion, sIO * io, void * param, RelationType * pRelationType , idx * pRelationTargets,idx * pRecordTargetIndexes, idx iLine );
                    sCallbackRecordType callbackRecordType;
                    sCallbackRelationType callbackRelationType;
                    sCallbackRecord callbackRecord;
                    sCallbackRelation callbackRelation;
                    void * params;
                    sIon_InfoCallbacks () { sSet(this);}
                    sIon_InfoCallbacks(sCallbackRecordType lRCT, sCallbackRelationType lRLT, sCallbackRecord lRC, sCallbackRelation lRL, void * lparams)
                    {
                        callbackRecordType=lRCT;
                        callbackRelationType=lRLT;
                        callbackRecord=lRC;
                        callbackRelation=lRL;
                        params=lparams;
                    }

            };

            static idx exportRecordTypeVax(sIon * ion, sIO * io, void * param, RecordType * pRecordType );
            static idx exportRelationTypeVax(sIon * ion, sIO * io, void * param, RelationType * pRelationType );
            static idx exportRecordVax(sIon * ion, sIO * io, void * param, RecordType * pRecordType, const void * pRecordBody, idx recordSize , idx iLine, idx iRecOrderInRelation);
            static idx exportRelationVax(sIon * ion, sIO * io, void * param, RelationType * pRelationType , idx * pRelationTargets,idx * pRecordTargetIndexes , idx iLine);
            static sIon_InfoCallbacks vaxExporter;
            const char  * info(idx whatToPrint, sIO * buf, const char * type, const char * sortIndex=0, idx start=0, idx cnt=0 , sIon_InfoCallbacks * infoCallbacks=0);



        public:
            idx mergeIons(sIon * fromdb);

        public:
            sVec <idx > transactionFile;
            idx loadTransactions(const idx * fl, idx fldim, sIO * out=0, sIO * errb=0);

            const char * getIonAtachmentName(sStr * flnm,const char * relationName, const char * suffix);


            struct SortRelationsHeader{
                    idx cntSorted, shiftRelationOffsets, countSortedRelationOffsets;
                    idx reserved[16];
                    RecordResult usedRecords[1];
                    static idx * relationOffetsArray( sMex * mex ) {
                        if(!mex || !mex->ok())return 0;
                        SortRelationsHeader * sh=(SortRelationsHeader * )mex->ptr(0);
                        return (idx*)mex->ptr(sh->shiftRelationOffsets);
                    }

            };

            typedef idx (* sCallbackRelationSorter)(sIon * ion, void * param, RecordResult * targets1, RecordResult * targets2, idx ind1, idx ind2 );
            struct RelationIterator{
                RelationType * pRelationType;
                idx cntRelations,serno, irPos;
                idx is64Bit;
                sMex * relationsArr;
                void * relationsOffsets;
                idx * pRelationTargets;
                sMex localMex,* mex;
                void reset( void ){
                    serno=0;
                    irPos=0;
                }
                RelationIterator(){reset();mex=&localMex;}
            };

            enum eSortRelationFlags{ bSortRelationReverse=0, bSortRelationNumeric};
            idx sortRelations(const char * relationName, const char * useRecords, const char * sortName, sCallbackRelationSorter sorterFunc, void * sorterParam, sVec<idx> * typeIndices =0 , idx extraMem=0, idx sortCache=64);
            idx analyzeUseRecordSortLine(RelationType * pRelationType, const char * useRecords, sVec < idx > * recordTypeIndexes);
            SortRelationsHeader * getRelationSorterIterator(const char * relationName, const char * sortName, RelationIterator * iter);
            SortRelationsHeader * getRelationSorterIterator(idx relationTypeIndex, RelationIterator * iter);
            idx getRelationOffsetsBySortIterator(sIon::RelationIterator* iter, idx serno);
            idx getRelationBodyBySortIterator(RelationIterator* iter, RecordResult * resultSet , idx cntRelationBodies=0, idx countToProduce=1, idx serno=sNotIdx);


            struct SearchElement {
                const void * val;
                idx valSize;
                idx iRecordTypeTocompare;
            };
            sMex regexpList;
            sMex regBuf;
            idx prepareSearchValset(idx relationTypeIndex, SearchElement * seList, idx maxSeList, const idx * markerArr, va_list marker,sVec < RecordResult > * stackRecords) ;
            idx getNextRelationBySearchValset(idx relationTypeIndex, SearchElement * seList, idx iVarCnt, idx * pirPos=0, idx * iStartFrom=0);

            struct ElementIterator {

            };

            idx buildVTree(const char * relation, const char * sortName, const char * vtreeName, RecordResult * rStart, RecordResult * rEnd,idx cnt );

            static bool getIonVTreeNode( void * rngPtr, void * maxPtr, idx treeInd, void * vnode, void * params );
            static idx ionVTreeComparator( void * i1, void * i2, void * params, idx compType );
            static void setIonVTreeMax( void * rngPtr, void * maxPtr, idx treeInd, idx maxVal, void * params );


            idx getRelationBucketByRange(Bucket * bucket, idx relationTypeIndex, const char * engineID00, idx * pCnt, const idx * markerArr, va_list marker=0 , sVec < RecordResult > * resultSet=0,sDic < sMex > * ac=0);
            idx getRangeNextBucket(Bucket * bucket);

        private:

            struct RelationSortStruct {
                sIon * ion;
                RelationType * pRelationType;
                idx is64Bit;
                const void * relationsArr;

                idx cntSorted;
                idx i1Prev,i2Prev;
                sVec< sVec <Record>  * > recordMexs;
                sVec < sMex * > recordBodyMexs;
                sVec < RecordResult  > pairRelBuf;
                idx sortBufCacheSize;
                sVec < idx > cacheIndexes;

                sCallbackRelationSorter sorterFunc;
                void * sorterParam;
            };

            static idx sortRelationsCallbackFunction(RelationSortStruct * param, void * relationsArr, idx i1, idx i2 );
            static idx sortRecordComparator(RecordResult * r1, RecordResult * r2, idx cnt, idx i2 = 0 , idx i1 = 0 );
    };


    class sIonWander
    {
        public:
        static const char * traverseCommands;
        const char * oriRules;

        enum eTraverseCommand {
            eTraverseFind=0,
            eTraverseCount,
            eTraverseSearch,
            eTraverseDelete,
            eTraverseAdd,
            eTraversePrint ,
            eTraversePrintCSV ,
            eTraverseBody ,
            eTraverseBlob,
            eTraverseSerial,
            eTraverseReturn,
            eTraverseSet,
            eTraverseForeach,
            eTraverseJumpNot,
            eTraverseJump,
            eTraverseDictNot,
            eTraverseDict,
            eTraverseDDictNot,
            eTraverseDDict,
            eTraverseSumNot,
            eTraverseSum,
            eTraverseUniqueNot,
            eTraverseUnique,
            eTraverseCheckOn,
            eTraverseCheckOff,
            eTraverseIonLimit,
            eTraverseSkipOperation,
            eTraverseParseStop,
            eTraversePrematureTermination=1000,
            eTraverseOperation,
            eTraverseRelationBased=eTraverseAdd,
            eTraverseRelationHashBased=eTraverseDelete
        } ;



        public:
            struct StatementHeader {
                idx relationTypeIndex;
                idx traverseCommand(){return relationTypeIndex>>32;}
                idx relationHasherIndexType;
                idx limitStart,limitEnd;
                idx scope;
                const char * label;
            };



            typedef idx (*traverserCallback)(sIon * ion, sIonWander *ts, StatementHeader * traverserStatement, sIon::RecordResult * curResults);
            void * userParam;
            const char * errDetail;
            bool debug;
            sDic < idx > parametricArguments;
            bool breakMode,continueScope;
            traverserCallback callbackFunc;
            idx callbackFuncIter;
            void * callbackFuncParam;
            bool checkOn;
            char internalSeparator;

        private:
            idx * pCnt;

            sDic < sDic < idx > > resultIndexes;
            sDic<sVec< idx > > forwardLabels;
            sDic < idx > uniqResultCounters;

            idx resultIndexesIncrement;


            sStr compileBuf,dictBuf, uniqBuf;
            bool traverseOutputting, someInRecord;


        public:
            idx ofsTrajectory;
            idx levelOperation,cntResults;
            char recordProtectQuote;
            idx listStart,listEnd,listCallingIon;

        private:
            struct sIonRef {sIon * ion; bool self; sVec < idx > SearchTrajectory; idx ionSerial; sVec< idx > fixBodyPos; const char * precompBuf; idx precompBufLen;};

            sMex toHushBuf;
            sStr debugStatementBuf;
            void updateCompileBufPointers(void);


        public:
            sVec < sIonRef > ionList;

            sIonWander(sIon * lion=0, void * param=0, idx * searchTraj=0, traverserCallback func=0) {
                userParam=param;
                ofsTrajectory=0;
                pCnt=0;
                callbackFunc=func;
                callbackFuncIter=0;
                traverseFieldSeparator=",";
                traverseRecordSeparator="\n";
                traverseOutputting=false;
                someInRecord=false;
                resultCounter.flagOn(sMex::fSetZero);
                resultSummator.flagOn(sMex::fSetZero);
                resultCumulator=0;
                bigDicCumulator=0;
                cntResults=0;
                maxLevelOperations=sIdxMax;
                maxNumberResults=sIdxMax;
                levelOperation=0;
                recordProtectQuote=0;
                listStart=0;
                listEnd=sIdxMax;
                debug=0;
                breakMode=false;
                continueScope=false;
                pTraverseBuf=&traverseBuf;
                checkOn=true;
                internalSeparator=',';
                if(lion)addIon(lion,0);
            }
            ~sIonWander(){destroy();}
            void destroy(void){ for ( idx i=0 ; i< ionList.dim() ; ++i ) {if(ionList[i].self)delete ionList[i].ion;}}


            sVec < sIon::RecordResult > stackResults;

            sDic < idx > resultCounter;
        sDic < real > resultSummator;
            sDic < sStr > * resultCumulator;
            sMex bigDicBuffer;
            sVec < sDic < idx > > keysForDic;
            sDic < sMex::Pos > * bigDicCumulator;
            sIO traverseBuf, * pTraverseBuf;
            const char * traverseFieldSeparator, * traverseRecordSeparator;
            idx maxLevelOperations,maxNumberResults;


            struct TraverseParamReference{void * body; idx size;};
            idx setSearchTemplateVariable(const char * templateName, idx templateLen, const void * value, idx valSize);
            idx setSearchTemplateVariable(const char * templateName, const void * value) {return setSearchTemplateVariable(templateName, 0, value, 0);}
            TraverseParamReference * getSearchArgumentPointer(idx serial, idx ionNum=0);
            TraverseParamReference * getSearchDictionaryPointer(const char *serno, idx serlen=0, idx ionNum=0);
            TraverseParamReference * getSearchDictionaryPointer(idx serial, idx ionNum=0);
            idx retrieveParametricWander(sVar * pForm, idx ionNum);
            const char * traverseCompileReal(sIonRef * ionRef, const char * rules, idx len, sIO *  err=0, bool acceptInvalid=false);
            bool traverseIsCompiled(void ){ return ionList.ptr(0)->SearchTrajectory.dim() ? true : false ;}
            const char * traverseView(void);
            const char * traverseViewVal(void);
            const char * traverseViewValTbl(void);
            const char * traverseViewBigDic2D(bool quotes=true);
            const char * printPrecompiledReal(sIonRef * ionRef, sStr * buf);
            const char * printPrecompiled(sStr * buf);
            idx traverse(idx start=0, idx iListStart=0, idx iListEnd=sIdxMax);
            const char * traverseCompile(const char * rules, idx len=0, sIO * errb=0, bool acceptInvalid=false, bool lazyCompile=false);


            idx traverseReal(sIonRef * ionRef, idx start=0, idx myListIndex=0);
            void resetCompileBuf(void);
            void resetResultBuf(void);
            template < class Tobj> sIonRef * addIon(Tobj * curIon, bool self=false, idx serial=sNotIdx)
            {
                sIonRef ref;

                if( curIon ){
                    ref.ion=(sIon*)curIon ;
                    ref.self=self;
                }else {
                    ref.ion=(sIon*)(new Tobj());
                    ref.self=true;
                }


                sIonRef * r=ionList.add(1);
                *r=ref;
                r->ionSerial= (serial!=sNotIdx ) ? serial : ionList.dim();

                return r;
            }
            sIonRef * addIon(sIon * curIon)
            {
                return addIon(curIon,false,sNotIdx);
            }
            idx callCalbackFunc(sIon * ion, sIonWander *ts, StatementHeader * traverserStatement, sIon::RecordResult * curResults)
            {
                idx res;
                for( callbackFuncIter=0; (res=callbackFunc(ion, ts, traverserStatement, curResults)) ==-1; )
                    {++callbackFuncIter;}
                return res;
            }

            template < class Tobj > idx attachIons( const char * fileList00, idx flags=sMex::fReadonly, idx maxCount=1, Tobj * t=0 )
            {
                idx ser=ionList.dim()+1, iin=0;
                sStr d; sFilePath p;
                for( const char * iof=fileList00; iof; iof=sString::next00(iof) ) {
                    sFil iofl(iof,sMex::fReadonly);
                    if( iofl.length()>5 && memcmp(iofl.ptr(0),"list\n",5)==0){
                        d.cut(0);sString::searchAndReplaceSymbols(&d,iofl.ptr(5),iofl.length(),"\n",0,0,true,true,true,true);
                        for( const char * lop=d.ptr(); lop; lop=sString::next00(lop) ) {
                            p.cut(0);p.makeName(iof,"%%dir/%s",lop);
                            addIon(t,false,ser)->ion->init(p.ptr(0),sMex::fReadonly);
                        }
                        ++ser;
                        ++iin;
                        if(iin>=maxCount)break;
                        continue;
                    }

                    addIon(t,false,ser)->ion->init(iof,sMex::fReadonly);
                    ++ser;
                    ++iin;
                    if(iin>=maxCount)break;
                }
                return ionList.dim();
            }
            idx attachIons( const char * fileList00, idx flags=sMex::fReadonly, idx maxCount=1)
            {
                return attachIons( fileList00, flags, maxCount, (sIon*)0 );
            }

        private:
            struct ColMapStruct {
                    idx * ptrToChange;
                    idx columnInTbl;
            };

        public:

            idx traverseTable(sTbl * tbl, sTxtTbl * txttbl=0, idx countResultMax=sIdxMax, idx startResult=0, idx contRowsMax=sIdxMax );
            void setSepar(const char * sepField, const char * sepRecord ) {
                if(sepField)traverseFieldSeparator=(*sepField=='~') ? 0 : sepField;
                if(sepRecord) traverseRecordSeparator=(*sepRecord=='~') ? 0 : sepRecord;
            }

            sVec <idx > transactionFile;
            idx loadTransactions(const idx * fl, idx fldim, sIO * out=0, sIO * errb=0);



    };

    class sIonTransaction {
            sIon * _ion;
            sIonWander * _wander;

        public:
            sIonTransaction (sIon * ion=0) {init(ion,0);}
            sIonTransaction (sIonWander * wander) {init(0, wander);}
            sIonTransaction * init(sIon * ion=0, sIonWander * wander=0){_ion=ion; _wander=wander; return this;}

            struct Header {
                    idx id;
                    idx flags;
                    idx checksum;
                    idx reserve;
            };

            enum fTransactionStates{
                fTransactionLockedForWriting=0x01,
                fTransactionLockedForReading=0x02
            };
            enum eTransactionCommands {
                eTransactionAddRecord=0,
                eTransactionAddRelation,
                eTransactionDeleteRelation,
                eTransactionTraverseCompile,
                eTransactionTraverse
            };


            idx openTransaction(idx lTransactionId, const char * basename=0, const char * extension=0);
            idx commitTransaction(void);
            idx waitTransaction(idx transactionId, const char * baseName, const char * extension, idx waitInMiliSec=100, idx maxIters=10000 );
            idx loadTransactions(sIO * log, const char * dirini, const char * wildcard, idx waitInMiliSec=100, idx maxIters=1 );
            idx loadTransactions(sIO * log, idx lTransactionId, const char * baseName=0, const char * extension=0);
            idx loadTransactions(sIO * log, const char * flnm);


    };
};

#endif

