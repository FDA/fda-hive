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
#ifndef sLib_sIonBirel_hpp
#define sLib_sIonBirel_hpp

#include <regex.h>
#include <ion/sIon.hpp>
#include <ion/vax.hpp>

namespace slib {
    class sVax;
    class sIonBirel: public sIon
    {
        private:
            idx typeSub, typeAtr, typeVal;
            idx typeRel;

        public:
            union LinkType {
                idx arr[5];
                struct {
                    idx sub;
                    idx atr;
                    idx val;
                    idx rel;
                    idx flags;
                }ixes;

            };
            idx root;
            static const char * Link_INDEX;
            static const char * Link_IONBIREL_ROOT;
            static const char * Link_IONBIREL_ROOT_BODY;
            static const idx Link_AUTOVAL;
            static const idx Link_AUTOARR;
            static const char * ionScanOutFlagsFormat;
            bool supportVariableResolution;
        private:
            sVec < LinkType > ltStack;
            sDic < idx > dicVar;
            typedef const void * (*callbackDicFunc)(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf);
            sDic < callbackDicFunc > dicFun;
            sDic < sIonWander  * > wanderList;
            sMex toHushBuf;
            sStr brBuf;
            idx activeWander;
            sStr pathIter;


            idx resolve( idx type, const char * ref, idx sizeRef, idx flags, RecordResult * pResults=0, idx startFrom=sNotIdx);
        public:

            sIonBirel(const char * baseName=0, idx lopenMode=0, sIO * out=0, sIO * lerrIO=0) {
                init0(out, lerrIO);
                init(baseName,lopenMode);
                return;

            }
            ~sIonBirel(){destroy();}
            void destroy()
            {
                for( idx i=0; i<wanderList.dim() ; ++i) {
                    delete *wanderList.ptr(i);
                }
            }
            sIonBirel * init0(sIO * out, sIO * lerrIO)
            {
                root=0;
                debug=0;
                activeWander=sNotIdx;
                errIO=lerrIO;

                totalMatch=0;

                iterCurDepth=0;
                iterMaxDepth=sIdxMax;
                iterElementsTraversed=0;
                iterStartRange=0;
                iterCntRangeRoot=0;
                iterCntRangeList=0;
                iterCntRangeArr=0;
                skip0frame=false;
                searchShowsSiblingUp=0;
                supportVariableResolution=true;

                levelInfo.add(1);levelInfo.set(0);

                iteratorParam=0;
                iteratorCallbackIn=0;
                iteratorCallbackOut=0;
                iteratorCallbackSearch=0;
                pathCallback=0;
                pathCallbackParam=0;
                pathIter.addString("$root");

                subQueryCallback=0;
                subQueryParam=0;


                doRaction=eRactionContinue;

                for( idx is=0; is<sDim(searchStruc); ++is ) {
                    searchStruc[is].el.flagOn(sMex::fSetZero|sMex::fBlockCompact);
                }


                domainIterators.flagOn(sMex::fBlockCompact|sMex::fSetZero);
                nodePathSeparator=0;



                traverseIterateQuery="hc=find.rel(sub=\"$val\");";
                firstTraverseQueryID="??";
                addWander("",out);


                outFlags=sFlag(fOutJson)|sFlag(fOutSpace);
                tableElements=&TableElements;
                avoidFrame=false;
                iterateLevelCallbackFunc=0;
                return this;
            }
            sIonBirel * init(const char * baseName, idx lopenMode)
            {
                sIon::init(baseName,lopenMode);
                construct();
                return this;
            }
            void construct(void);

            sIonWander * addWander(const char * id, sIO * out=0 ) {
                sIonWander * w=new sIonWander();
                *wanderList.set(id,sLen(id),&activeWander)=w;
                w->addIon(this );
                if(out)w->pTraverseBuf=out; else w->pTraverseBuf=wander("")->pTraverseBuf;
                return w;
            }
            sIonWander * wander(idx iWand=sNotIdx) {return *wanderList.ptr(iWand!=sNotIdx  ? iWand : activeWander); }
            sIonWander * wander(const char * id, bool autoset=false) {sIonWander * * res=wanderList.get(id,sLen(id));return res ? *res : (autoset ? addWander(id,wander("")->pTraverseBuf) : 0); }


            static const void * func_ionbr(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );
            static const void * func_ionql(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );
            static const void * func_timenow(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );
            static const void * func_newid(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );
            static const void * func_randreal(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );
            static const void * func_randint(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );
            static const void * func_randstr(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize, char * outbuf );


            sIO * errIO;
            idx debug;
            idx find(Bucket * bucket, const char * sub, idx sizeSub, const char * atr, idx sizeAtr,  const char * val, idx sizeVal);

        public:
            enum eResolveFlags{
                fConstructPath=0x01,
                fDictionarize=0x02,
                fFuncEval=0x04
            };



            LinkType * link(LinkType * lnk, const char * sub, idx sizeSub, const char * atr, idx sizeAtr,  const char * val, idx sizeVal);
            LinkType * look(LinkType * lnk, const char * sub, idx sizeSub, const char * atr=0, idx sizeAtr=0,  const char * val=0, idx sizeVal=0, Bucket * bucket=0, idx * pCnt=0);
            LinkType * next(LinkType * lnk, Bucket * bucket );
            idx del(const char * sub, idx sizeSub, const char * atr, idx sizeAtr,  const char * val, idx sizeVal, idx istart, idx iend);
            idx set(idx sub, const char * path, const char * value00 , idx valSize, idx valCnt=1, sDic < idx > * sublist=0);
            idx resolve( const char * ref, idx sizeRef=0, idx flags=(fConstructPath|fDictionarize|fFuncEval),RecordResult * pResults=0,idx startFrom=sNotIdx){return resolve(typeSub, ref, sizeRef ? sizeRef : sLen(ref), flags, pResults, startFrom);}



            static idx analyzeValueStatic(sJax * jax, void * param, sJax::JsonFrame * fr,sJax::JsonFrame * prv,sIO * err) {
                sIO * t;
                if(err){
                    t=((sIonBirel*)param)->errIO;
                    ((sIonBirel*)param)->errIO=err;
                }
                idx ret=((sIonBirel*)param)->analyzeValue(jax, fr,prv);
                if(err) {
                    ((sIonBirel*)param)->errIO=t;
                }
                return ret;
            }

            idx analyzeValue(sJax * jax, sJax::JsonFrame * fr,sJax::JsonFrame * prv);
            void setRoot(const char * rootnode, idx rootlen=0);
            idx parse(sJax * jax) ;


        public:
            struct BirelResult {

                const char * cnt;
                idx cntLen;
                idx cntTypeIndex;
                idx cntCType;
                idx cntIndex;

                const char * sub;
                idx subLen;
                idx subTypeIndex;
                idx subCType;
                idx subIndex;

                const char * atr;
                idx atrLen;
                idx atrTypeIndex;
                idx atrCType;
                idx atrIndex;

                const char * val;
                idx valLen;
                idx valTypeIndex;
                idx valCType;
                idx valIndex;



            };

        public:


            idx iterMaxDepth
               ,iterCurDepth
               ,iterElementsTraversed
               ,iterStartRange
               ,iterCntRangeRoot
               ,iterCntRangeList
               ,iterCntRangeArr
               ,totalMatch
               ,searchShowsSiblingUp;
               ;
            bool skip0frame;
            struct SearchStruc {
                struct Element {
                    const char * par;
                    idx len;
                    regex_t rex;
                };
                struct AtrVal {
                    Element atrval[3];
                    char logic;
                };
                sVec < AtrVal > el;
            };
            enum eSearchCriteria {
                eSearch=0,
                eInto,
                eFields,
                eSearchLast
            };
            SearchStruc searchStruc[eSearchLast];


            sStr regBuf, nodePath;
            const char * nodePathSeparator;

            struct LevelInfo {
                    idx levelPass;
                    idx totPass;
                    idx levelMatch;
                    idx totMatch;
                    idx isInRange;
                    idx searchHits;
                    idx forcedMatchSiblings;
                    bool match;
                    bool isArray;
                    idx redoSelf;

                    sIonBirel::BirelResult * reslist;
                    sIonWander::StatementHeader * statement;
                    sIonWander * subWander;

                    LevelInfo () {
                        sSet(this,0,sizeof(*this));
                    }
               };


            void setSearch(idx what, const char * cmp, idx len=0);
            idx doSearch (idx what, const char * atr, idx szatr, const char * val, idx szval, LevelInfo * li, idx * pCntSubMatch=0);
            void freeSearch(void){
                for( idx is=0; is<sDim(searchStruc); ++is ) {
                    for( idx i=0; i<searchStruc[is].el.dim(); ++i ) {
                        for( idx k=0; k<sDim(searchStruc[is].el[i].atrval); ++k ) {
                            if(searchStruc[is].el[i].atrval[k].rex.allocated)regfree(&searchStruc[is].el[i].atrval[k].rex);
                        }
                    }
                }
            }


                       sVec < LevelInfo > levelInfo;
           sVec < idx > searchOutcomes;
           sIO firstFrame;
           void * iteratorParam,* iteratorSearchParam, * pathCallbackParam;
           typedef idx (*callbackIterator)(void * param, LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout);
           callbackIterator iteratorCallbackIn,iteratorCallbackOut,iteratorCallbackSearch;
           typedef idx (*pathCallbackIterator)(void * param, const char * path, const char * val, idx vallen, LevelInfo * level, sIonBirel * birel);
           pathCallbackIterator pathCallback;

           void * subQueryParam;
           typedef sIonWander * (*callbackSubQuery)(void * param, LevelInfo * level, sIonBirel * birel, sIonWander * wander);
           callbackSubQuery subQueryCallback;

           enum OutFlags {
               fOutJson=0,
               fOutJsonNaked,
               fOutTotals,
               fOutSearchTotals,
               fOutSpace,
               fOutFrame,
               fOutTbl,
               fOutTblHdrBottom,
               fOutTblRow,
               fOutTblSub,
               fOutFlagLast
           };
           idx outFlags;
           bool avoidFrame;

           idx iterateNodesCallback(sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist , sIon::Bucket * cbBucket);
           static idx iterateNodesCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist , sIon::Bucket * cbBucket)
           {
               return ((sIonBirel*)ion)->iterateNodesCallback(wander, statement, reslist , cbBucket);
           }


           struct DomainIterator {
               typedef idx (*callbackDomainIterator)(void * param, LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx iDomain);
               callbackDomainIterator callback;
               void * param;
               idx startDepth, endDepth;
               struct Level {
                 idx level, pathlen;
               };
               sVec < Level > stk;
           };
           sDic < DomainIterator > domainIterators;
           void addDomainIterator(const char * id, idx idlen, DomainIterator::callbackDomainIterator callback, void * param, idx startDepth, idx endDepth){

               DomainIterator * di=domainIterators.set(id,idlen);
               di->callback=callback;
               di->param=param;
               di->startDepth=startDepth;
               di->endDepth=endDepth;
               di->stk.flagOn(sMex::fBlockCompact|sMex::fSetZero);
           }
           const char * pathRelativeToDomain(idx iDomain, idx * plen=0) {
               if( !nodePath.length()) return "";
               DomainIterator::Level * l=domainIterators.ptr(iDomain)->stk.last();
               if(plen)
                   *plen=nodePath.length()-l->pathlen;
               return nodePath.ptr( l->pathlen ) ;
           }


           struct DomainCollect{
               sDic <  BirelResult > * dic;
           } ;

           static idx domainElementIterator(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx iDomain );



           struct ValCollect{
               struct Val {
                   const void * val;
                   idx valLen;
                   idx irow;
               };
               const void * sub;
               idx subLen;
               idx irow;

               idx flags;
               sDic <  Val > * dic;
           } ;
           static idx tableElementIterator(void * param, LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout);
           void tableHeaderOut(sIO * out, sDic < ValCollect::Val > * elements, idx  flags );



           const char * traverseIterateQuery;
           const char * traverseIterateQueryBranch;
           const char * traverseIterateQueryLeaf;
           const char * firstTraverseQueryID;
           typedef idx (*callbackSimpleFunc)(void * param,sIonBirel * thisptr, sIon::RecordResult * curResults, sIon::Bucket * bucket);
           callbackSimpleFunc iterateLevelCallbackFunc;
           sStr iql;
           sDic < ValCollect::Val > TableElements, * tableElements;
           sIon::RecordResult * getRelationVals(sIon::Bucket * bucket, sIon::RecordResult * rr, idx * pRelCnt=0, bool moveToNext=false);
           sIon::RecordResult * getRelationVals(sIon::Bucket * bucket, sVec < sIon::RecordResult > * rvec, idx * pRelCnt=0, bool moveToNext=false);

          static idx copyIterator(void * param,sIonBirel * thisptr, sIon::RecordResult * curResults, sIon::Bucket * bucket);
            idx copy (idx from, sIonBirel * dstIon, idx to, const char * exclList00=0);
            idx copy (idx from, const char * atrFrom, idx atrFromLen,sIonBirel * dstIon, idx to, const char * atrTo, idx atrToLen , idx createDestNode=0, const char * exclList00=0);
            void save(const char * filename, const char * path=0, bool append=false) {
                if(!append)sFile::remove(filename);
                sIO Out; Out.init(filename);
                wander("")->pTraverseBuf=&Out;
                iterateNodes("",path ? path : "$root");
            }
            idx iterateLevel(const char * cursor, const void * sub, idx sublen, callbackSimpleFunc func, void * param);
            idx iterateLevel(sIonWander * w, const void * sub, idx sublen, sIonBirel::callbackSimpleFunc func, void * param);
           static idx iterateLevelCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults, sIon::Bucket * cbBucket);
           idx iterateNodesL(const char * cursor, const void * sub, idx sublen=0, callbackIterator func=0, void * param=0);
           idx iterateNodesL(sIonWander * w , const void * sub, idx sublen, callbackIterator func, void * param) ;
           idx iterateNodes(const char * cursor,  const char * subfmt=0, ... )
           {

               idx start;
               if(subfmt) {
                   iql.cut(0);
                   sCallVarg(iql.vprintf,subfmt);
                   RecordResult res[4];
                   start=resolve((const char *)iql.ptr(0),iql.length(),fFuncEval|fDictionarize,res+1);



                    idx resCnt=0;
                    Bucket searchBucket0;
                    searchBucket0.toHash = &toHushBuf;
                    getRelationBucketByHash(&searchBucket0, typeRel, 0, &resCnt, sNotPtr - 1, start);
                    if(resCnt<1) {
                        return 0;
                    }

                   if(start<0)
                       iterateNodesL(cursor,(const void*)iql.ptr(0),iql.length());
               } else start=sConvPtr2Int(Link_IONBIREL_ROOT);
               return iterateNodesL(cursor, Link_BodyByIndex, start);
           }


           idx doRaction;
           enum eReturnAction {
               eRactionContinue=0,
               eRactionHalt,
               eRactionReturn,
               eRactionSkip,
               eRactionLast

           };



    };
    };

    #define  sIonBirel_ROOT  sIonBirel::Link_IONBIREL_ROOT,5
    #define  sIonBirel_INDEX(_v_i)  sIonBirel::Link_INDEX,(_v_i)
    #define  sIonBirel_STRING(_v_s)  (_v_s), sLen(_v_s)
    #define  sIonBirel_AUTOVAL  0, sIonBirel::Link_AUTOVAL
    #define  sIonBirel_AUTOARR  0, sIonBirel::Link_AUTOARR
    #define  sIonBirel_NULL 0,0


#endif




