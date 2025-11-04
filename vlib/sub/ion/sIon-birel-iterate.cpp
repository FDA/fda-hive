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
#include <ion/sIon-birel.hpp>
using namespace slib;

#define indentLevel(_v_len_diff) { if((outFlags&sFlag(fOutSpace))){for(idx ip=0; ip<iterCurDepth+(_v_len_diff);++ip)cliWander->pTraverseBuf->add("    ",4);} }

#define outParValRev(_v_par, _v_par_len, _v_val, _v_val_len ) { \
    if(!avoidcomma) { \
        cliWander->pTraverseBuf->add(",",1); \
        if((outFlags&sFlag(fOutSpace)))cliWander->pTraverseBuf->add("\n",1); \
    } else avoidcomma=false; \
    indentLevel(1); \
    cliWander->pTraverseBuf->add((_v_par),(_v_par_len)); \
    if((outFlags&sFlag(fOutSpace)))cliWander->pTraverseBuf->add(" ",1); \
    cliWander->pTraverseBuf->add(":" ,1); \
    if((outFlags&sFlag(fOutSpace)))cliWander->pTraverseBuf->add(" ",1); \
    if((_v_val_len)>0)cliWander->pTraverseBuf->add((_v_val),(_v_val_len)); \
}
#define outParValQuotedRev(_v_par, _v_par_len, _v_val, _v_val_len ) { \
    if(!avoidcomma) { \
        cliWander->pTraverseBuf->add(",",1); \
        if((outFlags&sFlag(fOutSpace)))cliWander->pTraverseBuf->add("\n",1); \
    } else avoidcomma=false; \
    indentLevel(1); \
    cliWander->pTraverseBuf->add((_v_par),(_v_par_len)); \
    if((outFlags&sFlag(fOutSpace)))cliWander->pTraverseBuf->add(" ",1); \
    cliWander->pTraverseBuf->add(":" ,1); \
    if((outFlags&sFlag(fOutSpace)))cliWander->pTraverseBuf->add(" ",1); \
    cliWander->pTraverseBuf->add("\"",1); \
    if((_v_val_len)>0)cliWander->pTraverseBuf->add((_v_val),(_v_val_len)); \
    cliWander->pTraverseBuf->add("\"",1); \
}

#define outFrameParams(_v_li, _v_statement ) { \
    char ibuf[32]; idx ilen; \
    outParValQuotedRev("\"__sub\"",7, (const char*)((sIon::RecordResult*)((_v_li)->reslist))[1].body,((sIon::RecordResult*)((_v_li)->reslist))[1].size ); \
    if((outFlags&sFlag(fOutTotals))) {sIPrintf(ibuf,ilen,(li->levelPass),10); outParValRev("\"__dim\"",7, ibuf,ilen ); } \
    else outParValRev("\"__dim\"",7, (const char*)((sIon::RecordResult*)((_v_li)->reslist))[0].body,((sIon::RecordResult*)((_v_li)->reslist))[0].size ); \
    idx val=iterCurDepth==0 ? iterStartRange  : 0; sIPrintf(ibuf,ilen,val,10);  \
    outParValRev("\"__start\"",9, ibuf,ilen ); \
    sIPrintf(ibuf,ilen,(li->totMatch),10); \
    outParValRev("\"__tot\"",7, ibuf,ilen ); \
    sIPrintf(ibuf,ilen,(li->levelMatch),10); \
    outParValRev("\"__cnt\"",7, ibuf,ilen ); \
    sIPrintf(ibuf,ilen,(iterCurDepth),10); \
    outParValRev("\"__depth\"",9, ibuf,ilen ); \
}

#define rememberTplt( _v_s, _v_i ) \
            const void  * v ## _v_s = 0;  \
            idx l ## _v_s = 0 ;  \
            sIonWander::TraverseParamReference * t ## _v_s = subWander->getSearchDictionaryPointer("$" #_v_s, 4 , 0); \
            if( t ## _v_s ) { \
                v ## _v_s =(const char*)t ## _v_s->body; \
                l ## _v_s=t ## _v_s->size; \
                subWander->setSearchTemplateVariable("$" # _v_s ,4,reslist[ (_v_i) ].body,reslist[ (_v_i) ].size); \
            }

#define recoverTplt( _v_s ) \
            if( t ## _v_s ) { \
                subWander->setSearchTemplateVariable("$" # _v_s ,4, v ## _v_s , l ## _v_s ); \
            }

idx sIonBirel::iterateNodesCallback(sIonWander * cliWander,
        sIonWander::StatementHeader * statement, sIon::RecordResult * reslist, sIon::Bucket * cbBucket) {

    idx prvIterElementsTraversed = (++iterElementsTraversed);
    idx avoidcomma = false;
    idx nodePathLen = 0;

    if ((!statement->label || statement->label[0] == '_')) {
        if (iterCurDepth == 0 && (!skip0frame) ) {
            if ((outFlags & sFlag(fOutJson)) && (outFlags & sFlag(fOutFrame))) {
                LevelInfo * li = levelInfo.ptr(0);

                sIO * old = cliWander->pTraverseBuf;
                cliWander->pTraverseBuf = &firstFrame;
                cliWander->pTraverseBuf->cut(0);

                if (li->levelMatch) {
                    outFrameParams(li, statement);
                } else {
                    cliWander->pTraverseBuf->add("\"__empty\"", 9);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add(" ", 1);
                    cliWander->pTraverseBuf->add(":", 1);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add(" ", 1);
                    cliWander->pTraverseBuf->add("true", 4);
                }
                cliWander->pTraverseBuf = old;
            }
        }
        return 1;
    }
    if (statement->label[0] != 'h' || statement->label[1] != 'c') {
        return 1;
    }

    if (domainIterators.dim()) {
        nodePathLen = nodePath.length();
        nodePath.add(nodePathSeparator ? nodePathSeparator : "\0", 1);
        nodePath.addString((const char*) reslist[2].body, reslist[2].size);
    }


    levelInfo.resize(iterCurDepth + 2);
    LevelInfo * li = levelInfo.ptr(iterCurDepth + 1);
    sSet(li, 0);
    LevelInfo * par = li - 1;
    par->reslist = (sIonBirel::BirelResult*) reslist;
    par->statement = statement;

    searchOutcomes.resize(1 + searchStruc[eSearch].el.dim() * levelInfo.dim() / (8 * sizeof(searchOutcomes)));
    idx * so = searchOutcomes.ptr(0);
    for (idx id = 0; id < searchStruc[eSearch].el.dim(); ++id) {
        idx num = searchStruc[eSearch].el.dim() * (iterCurDepth + 1) + id;
        so[num / (sizeof(idx) * 8)] &= ~(((idx) 1) << (num % (sizeof(idx) * 8)));
    }
    idx pathIterLength=pathIter.length();
    if(par->reslist)pathIter.printf(".%.*s",(int)(par->reslist->atrLen),par->reslist->atr);
    if(pathCallback) { 
        pathCallback(pathCallbackParam, pathIter.ptr(0), par->reslist->val,par->reslist->valLen, par, this);

    }
    if (iteratorCallbackIn) {

        doRaction = iteratorCallbackIn(iteratorParam, par, (sIonBirel *) this,
                cliWander, 1);
        if (domainIterators.dim() && doRaction != eRactionContinue)
            nodePath.cut(nodePathLen);

        if (doRaction == eRactionSkip) {
            pathIter.cut(pathIterLength);return 1;
        } else if (doRaction == eRactionHalt) {
            pathIter.cut(pathIterLength);return 0;
        } else if (doRaction == eRactionReturn) {
            pathIter.cut(pathIterLength);return 0;
        } else if (doRaction == eRactionContinue)
            ;
    }
    bool isArray = ( ( ( (const char*) reslist[3].body)[0] == '_') && ( ( (const char*) reslist[3].body)[1] == '#') ) ? true : false;

    ++iterCurDepth;
    idx start = (iterCurDepth == 1) ? iterStartRange : 0;
    li->isInRange = par->levelPass < start ? 0 : 1;
    idx iterCntRange = iterCurDepth <=2 ? iterCntRangeRoot : (par->isArray ? iterCntRangeArr : iterCntRangeList) ;
    if ((outFlags & sFlag(fOutSearchTotals)) && iterCntRange && li->isInRange
            && par->levelPass >= (start + iterCntRange))
        li->isInRange = 0;
    if (iterCurDepth == 1)
        par->isInRange = li->isInRange;
    else
        li->isInRange *= par->isInRange;
    idx isInRange = li->isInRange;
    idx prevTotalMatch = par->totMatch;

    bool outFrame = true;
    idx priorIOPosition = 0, priorValPosition = 0;

    if (!isInRange)
        outFrame = false;
    if (!(outFlags & sFlag(fOutJson)))
        outFrame = false;
    if ((outFlags & sFlag(fOutJson))) {
        priorIOPosition = cliWander->pTraverseBuf->length();
        priorValPosition = priorIOPosition;
    }
    if(avoidFrame)
        outFrame=false;

    idx lastSearchResult = 0;
    bool isok = true, isout = searchStruc[eFields].el.dim() ? false : true;
    const char * atr = (const char*) reslist[2].body;
    idx sza = reslist[2].size;


    sIonWander * * pw=wanderList.get(atr, sza);
    li->subWander = pw ? (*pw) : 0;
    if (par->isArray && !li->subWander)
        li->subWander = par->subWander;

    li->isArray = isArray;

    DomainIterator * di =
            domainIterators.dim() ? domainIterators.get(atr, sza) : 0;
    if (di) {
        DomainIterator::Level *dl = di->stk.add(1);
        dl->level = iterCurDepth;
        dl->pathlen = nodePath.length() + 1;
    }

    if (outFrame) {

        if (par->levelMatch != 0) {
            cliWander->pTraverseBuf->add(",", 1);
            if ((outFlags & sFlag(fOutSpace)))
                cliWander->pTraverseBuf->add("\n", 1);
        }
        indentLevel(0);
        bool isArrayElement = isdigit(*atr) ? true : false;

        if (!isArrayElement) {
            cliWander->pTraverseBuf->add("\"", 1);
            cliWander->pTraverseBuf->add(atr, sza);
            cliWander->pTraverseBuf->add("\"", 1);
            if ((outFlags & sFlag(fOutSpace)))
                cliWander->pTraverseBuf->add(" ", 1);
            cliWander->pTraverseBuf->add(":", 1);
            if ((outFlags & sFlag(fOutSpace)))
                cliWander->pTraverseBuf->add(" ", 1);
        }
        priorValPosition = cliWander->pTraverseBuf->length();
        cliWander->pTraverseBuf->add(isArray ? "[" : "{", 1);

        if ((outFlags & sFlag(fOutSpace)))
            cliWander->pTraverseBuf->add("\n", 1);
    }

    bool forceNoMatch = false;
    const char * body = (const char*) reslist[3].body;
    idx sz = reslist[3].size;
    bool isBoolValue=false,isNumberValue=false;
    if( sz==5 && strcmp(body,"false")==0)isBoolValue=true;
    else if( sz==4 && strcmp(body,"true")==0)isBoolValue=true;
    else {
        idx isz;
        for(isz=0;isz<sz;++isz) {
            if( !strchr( "0123456789.+-",body[isz]) )
                break;
            if((body[isz]=='+' || body[isz]=='-') && isz!=0)
                break;
        }
        if(isz<sz)isNumberValue=false;
        else isNumberValue=true;
    }

    if (isok) {
        if( !par->forcedMatchSiblings ) {
            if (searchStruc[eSearch].el.dim()) {
                lastSearchResult = doSearch(eSearch, atr, sza, body, sz, par, 0);
                isok = lastSearchResult ? true : false;

                if(lastSearchResult) {
                    if(searchShowsSiblingUp) {
                        par->forcedMatchSiblings=searchShowsSiblingUp;
                        par->redoSelf=1;
                    }
               }

                if (lastSearchResult) {
                    --lastSearchResult;
                    idx num = searchStruc[eSearch].el.dim() * (iterCurDepth - 1)
                            + lastSearchResult;
                    so[num / (sizeof(idx) * 8)] |= ((idx) 1)
                            << (num % (sizeof(idx) * 8));
                }

            }
        }
    }
    if (iterCurDepth < iterMaxDepth) {
        idx resCnt=0;
        Bucket searchBucket0;
        searchBucket0.toHash = &toHushBuf;
        getRelationBucketByHash(&searchBucket0, typeRel, 0, &resCnt, sNotPtr - 1, reslist[3].index );
        if(resCnt) {
            bool stepIn = (iterCurDepth == 1 && searchStruc[eInto].el.dim()) ? doSearch(eInto, atr, sza, 0, 0, li) : true;

            if (stepIn) {

                sIonWander * subWander = li->isArray ? 0 : li->subWander;
                if (subQueryCallback) {
                    subWander = subQueryCallback(subQueryParam, par, this,
                            cliWander);
                }
                if (!subWander)
                    subWander = (sIonWander *) wander("");

                rememberTplt(sub, 1);
                rememberTplt(atr, 2);
                rememberTplt(val, 3);
                li = levelInfo.ptr(iterCurDepth );
                par = li - 1;
                for ( idx l=0; l< 1+li->redoSelf ; ++l ) {
                    idx prvOfs = subWander->ofsTrajectory;
                    subWander->ofsTrajectory = 0;
                    subWander->traverse();
                    subWander->ofsTrajectory = prvOfs;
                    li = levelInfo.ptr(iterCurDepth );
                    par = li - 1;
                }
                li->redoSelf=0;
                li->forcedMatchSiblings=0;

                recoverTplt(val);
                recoverTplt(atr);
                recoverTplt(sub);

                subWander = cliWander;

                if (doRaction == eRactionReturn || doRaction == eRactionHalt) {
                    isok = false;
                    priorValPosition = priorIOPosition;
                }
                if (doRaction == eRactionReturn) {
                    forceNoMatch = true;
                    doRaction = eRactionContinue;
                }
            }
        }
    }

    if (prvIterElementsTraversed == iterElementsTraversed) {
        bool isValArray = ( ( ( body)[0] == '_') && ( ( body)[1] == '#') ) ? true : false;
        bool isValObject = ( ( ( body)[0] == '_') && ( ( body)[1] == '@') ) ? true : false;

        if (isInRange) {
            if ((outFlags & sFlag(fOutJson))) {
                cliWander->pTraverseBuf->cut(priorValPosition);
                if(!avoidFrame && !isBoolValue && !isNumberValue && !isValArray && !isValObject)cliWander->pTraverseBuf->add("\"", 1);
            }
        }

        if (!isout) {
            isout = doSearch(eFields, atr, sza, 0, 0, li);
        }

        if (isok) {
            if (iteratorCallbackSearch && isout)
                doRaction = iteratorCallbackSearch(iteratorSearchParam, par,
                        (sIonBirel *) this, cliWander, 2 + lastSearchResult);
        }

        if (isok) {
            bool isAnyout = false;
            for (idx idd = 0; idd < domainIterators.dim(); ++idd) {
                DomainIterator * ddi = domainIterators.ptr(idd);
                if (ddi->stk.dim() == 0)
                    continue;
                idx domainLevel = ddi->stk[ddi->stk.dim() - 1].level;
                if (iterCurDepth >= domainLevel + ddi->startDepth
                        && iterCurDepth <= domainLevel + ddi->endDepth) {
                    if (ddi->callback)
                        ddi->callback(ddi->param, par, (sIonBirel *) this,
                                cliWander, idd);
                    isAnyout = true;
                }
            }
            if (isout && domainIterators.dim())
                isout = isAnyout;

        }

        if (isok) {
            if (isout) {
                ++par->levelPass;
                ++par->totPass;
            }
            ++par->searchHits;

            if (isInRange) {
                if (isout) {
                    ++par->levelMatch;
                    ++par->totMatch;
                }
                const char * remBody=body;
                idx remSize=sz;
                if(isValArray){body="[]";sz=2;}
                else if(isValObject){body="{}";sz=2;}
                if ((outFlags & sFlag(fOutJson))) {
                    if (isout) {

                        const char * escapableCharacters = "\\\"\n\r\b\t";
                        char chch[3];
                        chch[0] = '\\';
                        for (idx ik = 0, j; ik < sz;) {
                            for (j = ik; j < sz; ++j) {
                                if( body[j]=='\\' && body[j+1]=='u' )
                                    continue;
                                if (strchr(escapableCharacters, body[j]) != 0)
                                    break;
                            }
                            if (j > ik)
                                cliWander->pTraverseBuf->add(body + ik, j - ik);
                            if (j < sz) {
                                if(body[j]=='\n')chch[1]='n';
                                else if(body[j]=='\r')chch[1]='r';
                                else if(body[j]=='\b')chch[1]='b';
                                else if(body[j]=='\t')chch[1]='t';
                                else chch[1] = body[j];
                                if(body[0]!=0)
                                    cliWander->pTraverseBuf->add(chch, 2);
                            }
                            ik = j + 1;
                        }
                        if(!avoidFrame){if(!isBoolValue && !isNumberValue && !isValArray && !isValObject) cliWander->pTraverseBuf->add("\"", 1);}
                        else cliWander->pTraverseBuf->add("\n", 1);
                    } else
                        cliWander->pTraverseBuf->cut(priorIOPosition);
                }
                body=remBody;
                sz=remSize;
                ++totalMatch;
            }
        } else {
            if (isInRange) {
                if ((outFlags & sFlag(fOutJson))) {
                    cliWander->pTraverseBuf->cut(priorIOPosition);
                }
            }
        }
    } else {

            if (!li->match && !forceNoMatch) {
                if (searchStruc[eSearch].el.dim()) {

                    idx kkk = searchStruc[eSearch].el.dim() * iterCurDepth;
                    for (idx ie = 0; ie < searchStruc[eSearch].el.dim(); ++ie) {
                        idx num = kkk + ie;
                        char logic = searchStruc[eSearch].el[ie].logic;
                        idx bit = so[num / (sizeof(idx) * 8)]
                                & ((idx) 1) << (num % (sizeof(idx) * 8));

                        if (logic == '|') {
                            li->match = li->match | (bit ? true : false);
                        } else if (logic == '&') {
                            li->match = li->match & (bit ? true : false);
                        } else if (logic == '^') {
                            li->match = li->match ^ (bit ? true : false);
                        }

                    }

                } else
                    li->match = true;
            }
        if (li->match) {
            if (iterCurDepth > 0)
                par->match = 1;
        } else {
            li->levelMatch = 0;
            li->totMatch = 0;
            li->levelPass = 0;
            li->totPass = 0;
        }

        if (li->totPass) {
            ++par->levelPass;
            ++par->totPass;
        }

        if (isInRange) {
            if (li->totMatch) {
                ++par->levelMatch;
                ++par->totMatch;
            } else {
                if ((outFlags & sFlag(fOutJson))) {
                    cliWander->pTraverseBuf->cut(priorIOPosition);
                }
            }
        }
    }



    if (isInRange)
        par->totMatch += li->totMatch;
    par->totPass += li->totPass;


    if (outFrame) {
        if (prvIterElementsTraversed != iterElementsTraversed) {

            if (li->totMatch && isInRange) {

                if (!isArray && (outFlags & sFlag(fOutFrame))) {
                    outFrameParams(li, statement);
                }

                if ((outFlags & sFlag(fOutSpace)))
                    cliWander->pTraverseBuf->add("\n", 1);
                indentLevel(0);
                cliWander->pTraverseBuf->add(isArray ? "]" : "}", 1);
                if (isArray && (outFlags & sFlag(fOutFrame))) {
                    cliWander->pTraverseBuf->add(",", 1);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add("\n", 1);
                    indentLevel(0);
                    cliWander->pTraverseBuf->add("\"__", 3);
                    cliWander->pTraverseBuf->add("[]", 2);
                    cliWander->pTraverseBuf->add(atr, sza);
                    cliWander->pTraverseBuf->add("\"", 1);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add(" ", 1);
                    cliWander->pTraverseBuf->add(":", 1);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add(" ", 1);
                    cliWander->pTraverseBuf->add("{", 1);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add("\n", 1);
                    avoidcomma = true;
                    outFrameParams(li, statement);
                    if ((outFlags & sFlag(fOutSpace)))
                        cliWander->pTraverseBuf->add("\n", 1);
                    indentLevel(0);
                    cliWander->pTraverseBuf->add("}", 1);
                }

            }
        }
    }
    --iterCurDepth;

    if (iteratorCallbackOut) {
        doRaction = iteratorCallbackOut(iteratorParam, par, (sIonBirel *) this,
                cliWander, 0);
    }

    if (di)
        di->stk.cut(-1);

    if (domainIterators.dim())
        nodePath.cut(nodePathLen);

    if ((outFlags & sFlag(fOutJson))) {
        if (prvIterElementsTraversed != iterElementsTraversed) {
            if (prevTotalMatch != par->totMatch) {
                if (!isInRange || !li->match)
                    cliWander->pTraverseBuf->cut(priorIOPosition);

                else if (cliWander->pTraverseBuf->length())
                    cliWander->pTraverseBuf->callback(
                            cliWander->pTraverseBuf->ptr(0));
            }

        }
    }


    if (doRaction == eRactionSkip) {
        pathIter.cut(pathIterLength);return 1;
    } else if (doRaction == eRactionHalt) {
        pathIter.cut(pathIterLength);return 0;
    } else if (doRaction == eRactionReturn) {
        pathIter.cut(pathIterLength);return 0;
    } else if (doRaction == eRactionContinue)
        ;


    if (iterCntRange && par->levelMatch >= iterCntRange
            && (!(outFlags & sFlag(fOutSearchTotals)))){
        pathIter.cut(pathIterLength);return 0;
    }

    if (doRaction == eRactionHalt){
        pathIter.cut(pathIterLength);return 0;
    }

    pathIter.cut(pathIterLength);
    return 1;
}


sIon::RecordResult * sIonBirel::getRelationVals(sIon::Bucket * bucket, sIon::RecordResult * rr, idx * pRelCnt, bool moveToNext)
{
    getRelationsByBucketAndIndex(bucket, typeRel, rr, pRelCnt, moveToNext);
    return rr;
}

sIon::RecordResult * sIonBirel::getRelationVals(sIon::Bucket * bucket, sVec < sIon::RecordResult > * rvec, idx * pRelCnt, bool moveToNext)
{
    sIon::RecordResult * rr=rvec->add(4*(pRelCnt ? *pRelCnt : 1 ));
    return getRelationVals(bucket, rr, pRelCnt, moveToNext);
}

idx sIonBirel::iterateLevelCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults, sIon::Bucket * cbBucket)
{
    sIonBirel * br=(sIonBirel * )ts->callbackFuncParam;
    if(!br->iterateLevelCallbackFunc)return 1;
    if(traverserStatement->label[0]!='h' || traverserStatement->label[1]!='c')return 1;
    return br->iterateLevelCallbackFunc(br->iteratorParam,br,curResults+1,cbBucket);
}

idx sIonBirel::iterateLevel(const char * cursor, const void * sub, idx sublen, sIonBirel::callbackSimpleFunc func, void * param)
{
    char Cursor[128];
    if(!cursor){sprintf(Cursor,"%li",random());cursor=Cursor;}
    sIonWander * w;
    if(!strcmp(cursor,"_new")){
        w=new sIonWander(this) ;
        w->pTraverseBuf=wander("")->pTraverseBuf;
    }else
        w = wander(cursor,true);


    idx res=sIonBirel::iterateLevel(w, sub, sublen, func, param);
    if(!strcmp(cursor,"_new")){
        delete w;
    }
    return res;
}

struct CopyStruc {
    sIonBirel * dst;
    idx dstNode;
    const char * exclList00;
};

idx sIonBirel::copyIterator(void * param,sIonBirel * thisptr, sIon::RecordResult * curResults, sIon::Bucket * bucket)
{
    CopyStruc * cs=(CopyStruc *)param;
    LinkType lnk;

    if(cs->exclList00 && sString::compareChoice((const char*)curResults[1].body, cs->exclList00, 0, false, 0, true, curResults[1].size)!=-1)
        return 1;
    const char * b=(const char*)curResults[2].body;
    if(b[0]=='_' && (b[1]=='@' || b[1]=='#')) {
        cs->dst->link(&lnk, sIonBirel_INDEX(cs->dstNode), (const char*)curResults[1].body,curResults[1].size, 0, b[1]=='@' ? Link_AUTOVAL : Link_AUTOARR );
        thisptr->copy(curResults[0].index, (const char*)curResults[1].body,curResults[1].size,cs->dst,cs->dstNode,(const char*)curResults[1].body,curResults[1].size);
    }
    else
        cs->dst->link(&lnk, sIonBirel_INDEX(cs->dstNode), (const char*)curResults[1].body,curResults[1].size, (const char*)curResults[2].body,curResults[2].size);
    return 1;
}

idx sIonBirel::copy (idx from, sIonBirel * dstIon, idx to, const char * exclList00 )
{
    CopyStruc cs;
    cs.dst=dstIon;
    cs.exclList00=exclList00;
    cs.dstNode=to;
    idx ret=iterateLevel("_new", sIonBirel_INDEX(from),copyIterator, (void * )&cs);
    return ret ? to : 0 ;
}

idx sIonBirel::copy (idx from, const char * atrFrom, idx atrFromLen,sIonBirel * dstIon, idx to, const char * atrTo, idx atrToLen, idx createDestNode, const char * exclList00 )
{
    LinkType lnk,srclnk;
    if(!look(&srclnk,sIonBirel_INDEX(from),atrFrom,atrFromLen))return 0;

    CopyStruc cs;
    cs.dst=dstIon;
    cs.exclList00=exclList00;


    LinkType * res;
    if(createDestNode)
        res=cs.dst->link(&lnk, sIonBirel_INDEX(to), atrTo, atrToLen ? atrToLen : sLen(atrTo), 0, createDestNode );
    else
        res=cs.dst->look(&lnk, sIonBirel_INDEX(to), atrTo, atrToLen ? atrToLen : sLen(atrTo) );

    if(!res)return 0;

    cs.dstNode=lnk.ixes.val;
    look(&srclnk,sIonBirel_INDEX(from),atrFrom,atrFromLen);
    idx ret=iterateLevel("_new", sIonBirel_INDEX(srclnk.ixes.val),copyIterator, (void * )&cs);
    return ret ? lnk.ixes.val : 0;
}

idx sIonBirel::iterateLevel(sIonWander * w, const void * sub, idx sublen, sIonBirel::callbackSimpleFunc func, void * param)
{
    callbackSimpleFunc oldFunc=iterateLevelCallbackFunc;
    void * oldParam=iteratorParam;
    iterateLevelCallbackFunc=func;
    if(sub!=Link_BodyByIndex && ((const char*)sub)[0]=='$'){
        bool s=supportVariableResolution;supportVariableResolution=true;
        idx start=resolve((const char*)sub,sublen,sIonBirel::fFuncEval|sIonBirel::fDictionarize,0);
        supportVariableResolution=s;
        if(start>=0){
            sub=sIon::Link_BodyByIndex;
            sublen=start;
        }
    }

    if (!w->traverseIsCompiled())
        w->traverseCompile(traverseIterateQuery, 0, errIO, true);
    w->setSearchTemplateVariable("$val", 4, sub, (sub == Link_BodyByIndex || sublen) ? sublen : sLen(sub));

    w->callbackFunc = iterateLevelCallback;
    w->callbackFuncParam = (void*) this;
    for (idx i = 0; i < wanderList.dim(); ++i) {
        sIonWander * w = *wanderList.ptr(i);
        w->callbackFunc = iterateLevelCallback;
        w->callbackFuncParam = (void*) this;
    }
    if (param)
        iteratorParam = param;

    idx r = w->traverse();
    iterateLevelCallbackFunc=oldFunc;
    iteratorParam=oldParam;

    return r;
}

idx sIonBirel::iterateNodesL(const char * cursor, const void * sub, idx sublen, callbackIterator func, void * param)
{
    sIonWander * w = wander(cursor,true);
    return iterateNodesL(w , sub, sublen, func, param);
}
idx sIonBirel::iterateNodesL(sIonWander * w , const void * sub, idx sublen, callbackIterator func, void * param)
{

    if (!w->traverseIsCompiled())
        w->traverseCompile(traverseIterateQuery, 0, errIO, true);
    sStr fBuf;

    sIonWander * wf = wander(firstTraverseQueryID);
    if (wf) {
        w = wf;
    }
    else {
        w->setSearchTemplateVariable("$val", 4, sub, (sub == Link_BodyByIndex || sublen) ? sublen : sLen(sub));
    }




    ValCollect cache;
    if (outFlags & sFlag(fOutTbl)) {
        cache.dic = tableElements;
        cache.sub = 0;
        cache.subLen = 0;
        cache.irow = 0;
        cache.flags = outFlags;
        iteratorCallbackSearch = tableElementIterator;
        iteratorSearchParam = (void *) &cache;

    }

    for (idx i = 0; i < wanderList.dim(); ++i) {
        sIonWander * w = *wanderList.ptr(i);
        w->callbackFunc = iterateNodesCallback;
        w->callbackFuncParam = (void*) this;
    }

    if (func) {
        iteratorCallbackIn = func;
        iteratorCallbackOut = func;
        if (!(outFlags & sFlag(fOutTbl)))
            iteratorCallbackSearch = func;
    }
    if (param)
        iteratorParam = param;

    nodePath.cut(0);

    idx pos=w->pTraverseBuf->length();
    if ((outFlags & sFlag(fOutJson))) {
        w->pTraverseBuf->flags |= ((sIO::fNoAutoCallback));
        if ((!(outFlags & sFlag(fOutJsonNaked)))) {
            w->pTraverseBuf->add("{\n", 2);
        }
    }

    idx r = w->traverse();

    if(pos+2==w->pTraverseBuf->length())
        w->pTraverseBuf->cut(pos);
    else {
        if ((outFlags & sFlag(fOutJson))) {
            if (firstFrame.length())
                w->pTraverseBuf->add(firstFrame.ptr(), firstFrame.length());
            if ((!(outFlags & sFlag(fOutJsonNaked)))) {
                w->pTraverseBuf->add("\n}", 2);
            }
            w->pTraverseBuf->callback(w->pTraverseBuf->ptr(0));
        }
    }

    if (outFlags & sFlag(fOutTbl)) {
        tableElementIterator(&cache, 0, 0, w, sNotIdx);
        if (cache.flags & sFlag(fOutTblHdrBottom))
            tableHeaderOut(w->pTraverseBuf, tableElements, cache.flags);
    }

    return r;
}

void sIonBirel::setSearch(idx what, const char * cmp, idx len)
{
    SearchStruc * ss = searchStruc + what;

    idx lenP = 0, ic = 0;
    const char * chset = "|&^";
    char logic = '|';
    sStr t;
    if (!len)
        len = sLen(cmp);
    for (const char * cmpP = cmp; cmpP < cmp + len; cmpP += lenP + 2) {
        idx equ = 0;
        for (lenP = 0; cmpP + lenP < cmp + len; ++lenP) {
            if (cmpP[lenP] == '~' || cmpP[lenP] == '=')
                equ = lenP;
            for (ic = 0; ic < 3 && (cmpP[lenP + 0] != chset[ic] || cmpP[lenP + 1] != chset[ic]); ++ic)
                ;
            if (ic < 3)
                break;
        }
        if (cmpP[equ] != '~' && cmpP[equ] != '=')
            equ = lenP;

        SearchStruc::AtrVal * se = ss->el.add();
        idx point;
        for (point = equ; point > 0 && cmpP[point] != '.'; --point) {
        }
        if (point) {
            se->atrval[2].par = cmpP;
            se->atrval[2].len = point;
            ++point;
        }
        se->atrval[0].par = cmpP + point;
        se->atrval[0].len = equ - point;
        if (equ != lenP) {
            se->atrval[1].par = cmpP + equ + 1;
            se->atrval[1].len = lenP - equ - 1;
        }
        se->logic = logic;
        logic = chset[ic];

        for (idx k = 0; k < sDim(se->atrval); ++k) {
            if (se->atrval[k].len > 6
                    && strncmp(se->atrval[k].par, "regex:", 6) == 0) {
                se->atrval[k].par += 6;
                t.cut(0);
                t.addString(se->atrval[k].par, se->atrval[k].len);
                regcomp(&se->atrval[k].rex, t.ptr(), 0);
            }
        }
    }
}

idx sIonBirel::doSearch(idx what, const char * atr, idx szatr, const char * val,
        idx szval, LevelInfo * li, idx * pCntSubMatch) {

    SearchStruc * ss = searchStruc + what;

    idx match = 0;
    if (pCntSubMatch)
        *pCntSubMatch = 0;

    for (idx ie = 0; ie < ss->el.dim(); ++ie) {
        SearchStruc::AtrVal * se = ss->el.ptr(ie);

        bool localmatch = true;

        for (idx k = 0; k < sDim(se->atrval); ++k) {

            SearchStruc::Element * pv = se->atrval + k;

            if (localmatch && pv->len) {
                const char * cmp;
                idx len;
                if (k == 0) {
                    cmp = atr;
                    len = szatr;
                } else if (k == 1) {
                    cmp = val;
                    len = szval;
                } else {
                    sIonBirel::BirelResult * pres = (li - 1)->reslist;
                    if (!pres)
                        continue;
                    cmp = (char*) (pres->atr);
                    len = pres->atrLen;
                }
                idx l0 = len;

                if (pv->rex.allocated) {
                    regBuf.cut(0);
                    regBuf.add(cmp, len);
                    regBuf.add0(1);
                    int ismatch =
                            regexec(&pv->rex, (const char*) regBuf.ptr(0), 0, 0,
                                    0) ? 0 : 1;
                    if (!ismatch)
                        localmatch = false;
                } else {
                    localmatch = false;
                    const char * pp = pv->par;
                    idx pl = pv->len, is = 0, ip;
                    if (*pp == '^') {
                        ++pp;
                        --pl;
                        len = pl;
                    }
                    if (pp[pl - 1] == '$') {
                        --pl;
                        if (pv->par[0] == '^') {
                            if (l0 > pl)
                                len = 0;
                            else
                                --len;
                        }
                        else {
                            is = len - pl;
                        }
                    }
                    for (; is <= len - pl; ++is) {
                        for (ip = 0; ip < pl && cmp[is + ip] == pp[ip]; ++ip)
                            ;
                        if (ip == pl) {
                            localmatch = true;
                            break;
                        }
                    }
                }
            } else
                localmatch = 1;
            if (!localmatch)
                break;
        }
        if (localmatch && pCntSubMatch)
            ++(*pCntSubMatch);

        if (localmatch) {
            match = ie + 1;
            break;
        }
    }

    return match;
}

void sIonBirel::tableHeaderOut(sIO * out, sDic<ValCollect::Val> * elements,
        idx flags) {
    const char * comma = 0;
    if (flags & sFlag(fOutTblRow)) {
        comma = ",";
        out->add("irow", 4);
    }
    if (flags & sFlag(fOutTblSub)) {
        if (comma)
            out->add(comma, 1);
        comma = ",";
        out->add("sub", 3);
    }
    for (idx i = 0; i < elements->dim(); ++i) {
        idx idlen;
        const char * id = (const char*) elements->id(i, &idlen);
        if (comma)
            out->add(comma, 1);
        comma = ",";
        out->printf("%.*s", (int) idlen, id);
    }
    out->add("\n", 1);
    out->callback(out->ptr());
}

idx sIonBirel::tableElementIterator(void * param, sIonBirel::LevelInfo * level,
        sIonBirel * birel, sIonWander * wander, idx inorout) {
    ValCollect * sc = (ValCollect *) param;
    sDic<ValCollect::Val> * elements = sc->dic;

    bool newsub = false;
    if (!sc->subLen) {
        sc->sub = level->reslist->sub;
        sc->subLen = level->reslist->subLen;
    } else {
        if (inorout < 0 || sc->subLen != level->reslist->subLen
                || (sc->sub != level->reslist->sub
                        && memcmp(sc->sub, level->reslist->sub, sc->subLen))) {
            newsub = true;
        }
        if (newsub) {
            const char * comma = 0;
            if (sc->flags & sFlag(fOutTblRow)) {
                comma = ",";
                wander->pTraverseBuf->printf("%" DEC, sc->irow);
            }
            if (sc->flags & sFlag(fOutTblSub)) {
                if (comma)
                    wander->pTraverseBuf->add(comma, 1);
                comma = ",";
                wander->pTraverseBuf->printf("%.*s", (int) sc->subLen,
                        (const char*) sc->sub);
            }

            for (idx i = 0; i < elements->dim(); ++i) {
                if (comma)
                    wander->pTraverseBuf->add(comma, 1);
                comma = ",";
                ValCollect::Val * v = elements->ptr(i);
                if (v->irow != sc->irow) {
                    continue;
                }
                wander->pTraverseBuf->printf("\"%.*s\"", (int) v->valLen,
                        (const char*) v->val);
            }
            wander->pTraverseBuf->add("\n", 1);

            ++sc->irow;

            if (inorout < 0)
                return sIonBirel::eRactionContinue;

            sc->sub = level->reslist->sub;
            sc->subLen = level->reslist->subLen;
        }
    }

    ValCollect::Val * v = elements->set(level->reslist->atr,
            level->reslist->atrLen);
    v->val = level->reslist->val;
    v->valLen = level->reslist->valLen;
    v->irow = sc->irow;

    return sIonBirel::eRactionContinue;
}

idx sIonBirel::domainElementIterator(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx iDomain)
{
    DomainCollect * sc = (DomainCollect *) param;
    sDic<BirelResult> * attributes = sc->dic;

    idx pathlen = 0;
    const char * path = birel->pathRelativeToDomain(iDomain, &pathlen);
    if (attributes)
        *attributes->set(path, pathlen) = *level->reslist;

    return sIonBirel::eRactionContinue;
}

