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
        private:
            sVec < LinkType > ltStack;
            sDic < idx > dicVar;
            typedef const void * (*callbackDicFunc)(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize);
            sDic < callbackDicFunc > dicFun;
            sDic < sIonWander > wanderList;
            sMex toHushBuf;
            sStr brBuf;
            idx activeWander;

            idx resolve( idx type, const char * ref, idx sizeRef, idx flags);
        public:

            sIonBirel(const char * baseName=0, idx lopenMode=0, sIO * out=0, sIO * lerrIO=0) {
                init0(out, lerrIO);
                if(baseName)init(baseName,lopenMode);
                return;

            }
            ~sIonBirel(){}
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
                iterCntRangeDeep=0;

                levelInfo.add(1);levelInfo.set(0);

                iteratorParam=0;
                iteratorCallbackIn=0;
                iteratorCallbackOut=0;

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
                wanderList.set(id,sLen(id),&activeWander);
                sIonWander * w=wanderList.ptr(activeWander);
                w->addIon(this );
                if(out)w->pTraverseBuf=out; else w->pTraverseBuf=wander("")->pTraverseBuf;
                return w;
            }
            sIonWander * wander(idx iWand=sNotIdx) {return wanderList.ptr(iWand!=sNotIdx  ? iWand : activeWander); }
            sIonWander * wander(const char * id, bool autoset=false) {return autoset ? wanderList.set(id,sLen(id)) : wanderList.get(id,sLen(id)); }


            static const void * func_ionbr(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize);
            static const void * func_ionql(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize);
            static const void * func_timenow(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize);
            static const void * func_newid(sIonBirel * thisptr, idx isub, const char * arglist, idx len, idx * psize);


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
            LinkType * look(LinkType * lnk, const char * sub, idx sizeSub, const char * atr, idx sizeAtr,  const char * val, idx sizeVal);
            idx del(const char * sub, idx sizeSub, const char * atr, idx sizeAtr,  const char * val, idx sizeVal, idx istart, idx iend);
            idx set(idx sub, const char * path, const char * value00 , idx valSize, idx valCnt=1, sDic < idx > * sublist=0);
            idx resolve( const char * ref, idx sizeRef=0, idx flags=(fConstructPath|fDictionarize|fFuncEval)){return resolve(typeSub, ref, sizeRef ? sizeRef : sLen(ref), flags);}
            idx resolve( const char * atr, idx sizeAtr, ...);



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
               ,iterCntRangeDeep
               ,totalMatch
               ;
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
                    bool match;
                    bool isArray;

                    sIonBirel::BirelResult * reslist;
                    sIonWander::StatementHeader * statement;
                    sIonWander * subWander;
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
           void * iteratorParam,* iteratorSearchParam;
           typedef idx (*callbackIterator)(void * param, LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout);
           callbackIterator iteratorCallbackIn,iteratorCallbackOut,iteratorCallbackSearch;

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

           idx iterateNodesCallback(sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist );
           static idx iterateNodesCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist )
           {
               return ((sIonBirel*)ion)->iterateNodesCallback(wander, statement, reslist );
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
           const char * firstTraverseQueryID;
           sStr iql;
           sDic < ValCollect::Val > TableElements, * tableElements;
           idx iterateNodes(const void * sub, idx sublen=0, callbackIterator func=0, void * param=0);
           idx iterateNodes(const char * subfmt=0, ... )
           {

               idx start;
               if(subfmt) {
                   iql.cut(0);
                   sCallVarg(iql.vprintf,subfmt);
                   start=resolve(iql.ptr(0),iql.length(),fFuncEval|fDictionarize);
                   if(start<0)
                       iterateNodes((const void*)iql.ptr(0),iql.length());
               } else start=sConvPtr2Int(Link_IONBIREL_ROOT);
               return iterateNodes(Link_BodyByIndex, start);
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

    #define  sIonBirel_INDEX(_v_i)  sIonBirel::Link_INDEX,(_v_i)
    #define  sIonBirel_STRING(_v_s)  (_v_s), sLen(_v_s)
    #define  sIonBirel_AUTOVAL  0, sIonBirel::Link_AUTOVAL
    #define  sIonBirel_AUTOARR  0, sIonBirel::Link_AUTOARR

#endif


