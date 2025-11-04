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
    \
}

#define rememberTplt( _v_s, _v_i ) \
            const void  * v ## _v_s ;  \
            idx l ## _v_s ;  \
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
        sIonWander::StatementHeader * statement, sIon::RecordResult * reslist) {

    idx prvIterElementsTraversed = (++iterElementsTraversed);
    idx avoidcomma = false;

    if ((!statement->label || statement->label[0] == '_')) {
        if (iterCurDepth == 0) {
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

    idx nodePathLen = 0;
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

    searchOutcomes.resize(
            1
                    + searchStruc[eSearch].el.dim() * levelInfo.dim()
                            / (8 * sizeof(searchOutcomes)));
    idx * so = searchOutcomes.ptr(0);
    for (idx id = 0; id < searchStruc[eSearch].el.dim(); ++id) {
        idx num = searchStruc[eSearch].el.dim() * (iterCurDepth + 1) + id;
        so[num / (sizeof(idx) * 8)] &=
                ~(((idx) 1) << (num % (sizeof(idx) * 8)));
    }

    if (iteratorCallbackIn) {
        doRaction = iteratorCallbackIn(iteratorParam, par, (sIonBirel *) this,
                cliWander, 1);
        if (domainIterators.dim() && doRaction != eRactionContinue)
            nodePath.cut(nodePathLen);

        if (doRaction == eRactionSkip) {
            return 1;
        } else if (doRaction == eRactionHalt) {
            return 0;
        } else if (doRaction == eRactionReturn) {
            return 0;
        } else if (doRaction == eRactionContinue)
            ;
    }

    ++iterCurDepth;
    idx start = (iterCurDepth == 1) ? iterStartRange : 0;
    li->isInRange = par->levelPass < start ? 0 : 1;
    idx iterCntRange = iterCurDepth == 1 ? iterCntRangeRoot : iterCntRangeDeep;
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

    idx lastSearchResult = 0;
    bool isok = true, isout = searchStruc[eFields].el.dim() ? false : true;
    const char * atr = (const char*) reslist[2].body;
    idx sza = reslist[2].size;

    bool isArray = *(const char*) reslist[3].body == '#' ? true : false;
    li->subWander = wanderList.get(atr, sza);
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

    if (isok) {
        if (searchStruc[eSearch].el.dim()) {
            lastSearchResult = doSearch(eSearch, atr, sza, body, sz, par, 0);
            isok = lastSearchResult ? true : false;

            if (lastSearchResult) {
                --lastSearchResult;
                idx num = searchStruc[eSearch].el.dim() * (iterCurDepth - 1)
                        + lastSearchResult;
                so[num / (sizeof(idx) * 8)] |= ((idx) 1)
                        << (num % (sizeof(idx) * 8));
            }

        }
    }
    if (iterCurDepth < iterMaxDepth) {
        bool stepIn =
                (iterCurDepth == 1 && searchStruc[eInto].el.dim()) ?
                        doSearch(eInto, atr, sza, 0, 0, li) : true;
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

            idx prvOfs = subWander->ofsTrajectory;
            subWander->ofsTrajectory = 0;
            subWander->traverse();
            subWander->ofsTrajectory = prvOfs;
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

    if (prvIterElementsTraversed == iterElementsTraversed) {

        if (isInRange) {
            if ((outFlags & sFlag(fOutJson))) {
                cliWander->pTraverseBuf->cut(priorValPosition);
                cliWander->pTraverseBuf->add("\"", 1);
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
                if ((outFlags & sFlag(fOutJson))) {
                    if (isout) {

                        const char * escapableCharacters = "\\\'\"";
                        char chch[3];
                        chch[0] = '\\';
                        chch[1] = '\\';
                        for (idx ik = 0, j; ik < sz;) {
                            for (j = ik; j < sz; ++j) {
                                if (strchr(escapableCharacters, body[j]) != 0)
                                    break;
                            }
                            if (j > ik)
                                cliWander->pTraverseBuf->add(body + ik, j - ik);
                            if (j < sz) {
                                chch[2] = body[j];
                                if(body[0]!=0)
                                    cliWander->pTraverseBuf->add(chch, 3);
                            }
                            ik = j + 1;
                        }

                        cliWander->pTraverseBuf->add("\"", 1);
                    } else
                        cliWander->pTraverseBuf->cut(priorIOPosition);
                }
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
        return 1;
    } else if (doRaction == eRactionHalt) {
        return 0;
    } else if (doRaction == eRactionReturn) {
        return 0;
    } else if (doRaction == eRactionContinue)
        ;


    if (iterCntRange && par->levelMatch >= iterCntRange
            && (!(outFlags & sFlag(fOutSearchTotals))))
        return 0;

    if (doRaction == eRactionHalt)
        return 0;

    return 1;
}

idx sIonBirel::iterateNodes(const void * sub, idx sublen, callbackIterator func,
        void * param) {
    sIonWander * w = wander("");
    if (!w->traverseIsCompiled())
        w->traverseCompile(traverseIterateQuery, 0, errIO, true);
    sIonWander * wf = wander(firstTraverseQueryID);
    if (wf)
        w = wf;
    else
        w->setSearchTemplateVariable("$val", 4, sub,
                (sub == Link_BodyByIndex || sublen) ? sublen : sLen(sub));


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
        sIonWander * w = wanderList.ptr(i);
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

    if ((outFlags & sFlag(fOutJson))) {
        w->pTraverseBuf->flags |= ((sIO::fNoAutoCallback));
        if ((!(outFlags & sFlag(fOutJsonNaked)))) {
            w->pTraverseBuf->add("{\n", 2);
        }
    }

    idx r = w->traverse();
    if ((outFlags & sFlag(fOutJson))) {
        if (firstFrame.length())
            w->pTraverseBuf->add(firstFrame.ptr(), firstFrame.length());
        if ((!(outFlags & sFlag(fOutJsonNaked))))
            w->pTraverseBuf->add("\n}", 2);
        w->pTraverseBuf->callback(w->pTraverseBuf->ptr(0));
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
            for (ic = 0;
                    ic < 3
                            && (cmpP[lenP + 0] != chset[ic]
                                    || cmpP[lenP + 1] != chset[ic]); ++ic)
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

