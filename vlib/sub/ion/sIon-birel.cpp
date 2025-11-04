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
#include <slib/core/word.hpp>

using namespace slib;

const char * sIonBirel::Link_INDEX = sNotPtr - 1;
const idx sIonBirel::Link_AUTOVAL = sIdxMax;
const idx sIonBirel::Link_AUTOARR = sIdxMax - 1;
const char * sIonBirel::Link_IONBIREL_ROOT = "$root";
const char * sIonBirel::Link_IONBIREL_ROOT_BODY = "$root";
const char * sIonBirel::ionScanOutFlagsFormat =
        "%b=0|json|naked|totals|stotals|space|frame|tbl|hdr|row|sub;";


void sIonBirel::construct(void) {

    if (root)
        return;

    if (!(openMode & sMex::fReadonly)) {
        typeSub = addRecordType("sub", eCTypeString, eHashTypeString);
        typeAtr = addRecordType("atr", eCTypeString, eHashTypeString);
        typeVal = addRecordType("val", eCTypeString, eHashTypeString);
        aliasRecordTypes(typeSub, typeVal);

        typeRel = addRelationType(0, "rel", "sub" _ "atr" _ "val" __, "sub" __,
                "sub" _ "atr" __, "sub" _ "atr" _ "val" __, "sub" _ "val" __,
                "atr" __, "atr" _ "val" __, "val" __, (const char *) 0);

        if (!getRecordCount(typeSub))
            root = addRecord(typeSub, sLen(Link_IONBIREL_ROOT_BODY),
                    Link_IONBIREL_ROOT_BODY);
    } else {
        typeSub = recordAndRelationTypesHash.find(eRARHash_record, "sub", 4);
        typeAtr = recordAndRelationTypesHash.find(eRARHash_record, "atr", 4);
        typeVal = recordAndRelationTypesHash.find(eRARHash_record, "val", 4);
        if (!typeSub || !typeAtr || !typeVal)
            return;
        --typeSub;
        --typeAtr;
        --typeVal;
        RecordType * pRecordType = recordTypesArr.ptr(typeSub);
        root = getRecordByHash(pRecordType, Link_IONBIREL_ROOT_BODY,
                sLen(Link_IONBIREL_ROOT_BODY));
    }
    *dicVar.set(Link_IONBIREL_ROOT, sLen(Link_IONBIREL_ROOT)) = root;

    *dicFun.set("ionql", 5) = func_ionql;
    *dicFun.set("timenow", 7) = func_timenow;
    *dicFun.set("newid", 5) = func_newid;
    *dicFun.set("randint", 7) = func_randint;
    *dicFun.set("randreal", 8) = func_randreal;
    *dicFun.set("randstr", 7) = func_randstr;

}

void sIonBirel::setRoot(const char * rootnode, idx rootlen) {
    root = resolve(typeSub, (const char*) rootnode,
            rootlen ? rootlen : sLen(rootnode),
            fConstructPath | fDictionarize | fFuncEval);
}

idx sIonBirel::parse(sJax * jax) {
    jax->callbackFuncValue = analyzeValueStatic;
    jax->callbackParam = this;
    return jax->parse();
}

#define scanTillChar(_v_p, _v_ch )  { \
    while( strchr((_v_ch),*(_v_p)) && (_v_p)<recNext) { \
        if(*(_v_p)=='\n'){ ++lineCount; lineStart=nextLine;nextLine=_v_p+1-srcStart;} \
        ++(_v_p);} \
    } \
    if( (_v_p)>=recNext) break;

#define scanTillNotChar(_v_p, _v_ch , _v_len)  for (idx inquote=0, ip=0; (_v_p)[ip]<(_v_len); ++ip ) { \
        if((_v_p)[ip]=='\\' || (_v_p)[ip]=='\"'){ if(!inquote) {inquote=(_v_p)[ip];continue;} else if(inquote==(_v_p)[ip]){inquote=0;continue;} } \
        if( !inquote && strchr((_v_ch),(_v_p)[ip]) )break; \
    }

idx sIonBirel::resolve(idx type, const char * ref, idx sizeRef, idx flags, RecordResult * pResults, idx startFrom) {
    idx ipos = 0, ithis = 0;
    idx ibase = startFrom;
    RecordResult resBuf[3];
    RecordResult * results=pResults ? pResults : resBuf;
    char outbuf[1024];

    for (ithis = 0; ipos < sizeRef; ithis = ipos + 1) {
        idx inparentesis = 0, inquote = 0;
        for (ipos = ithis; ipos < sizeRef; ++ipos) {
            if (ref[ipos] == '\"' || ref[ipos] == '\'') {
                if (!inquote)
                    inquote = ref[ipos];
                else if (ref[ipos] == inquote)
                    inquote = 0;
                continue;
            }
            if (ref[ipos] == '(') {
                ++inparentesis;
                continue;
            }
            if (inparentesis > 0 && ref[ipos] == ')') {
                --inparentesis;
                continue;
            }
            if (ref[ipos] == '.' || ref[ipos] == '[' || ref[ipos] == ']')
                break;
        }
        if (ithis == sizeRef)
            return ibase;
        idx sz = ipos - ithis;
        if ( ipos+1<sizeRef && ( ref[ipos + 1] == '.' || ref[ipos + 1] == ']' || ref[ipos + 1] == '[') )
            ++ipos;
        idx * pdic = 0;
        if ( supportVariableResolution && ref[ithis] == '$' && ithis == 0) {
            if ((flags & fFuncEval)) {
                idx ifunpos, iparentesis;
                for (ifunpos = ithis; ifunpos < ipos; ++ifunpos)
                    if (ref[ifunpos] == '(')
                        break;
                ipos = ifunpos;
                if ( ifunpos<sizeRef && ref[ifunpos] == '(') {
                    idx frsize, ipar = 1;
                    for (iparentesis = ifunpos + 1; ifunpos < sizeRef && ipar;
                            ++iparentesis) {
                        if (ref[iparentesis] == '(')
                            ++ipar;
                        else if (ref[iparentesis] == ')')
                            --ipar;
                    }
                    callbackDicFunc * func =
                            ifunpos - ithis - 1 ?
                                    dicFun.get(ref + ithis + 1,
                                            ifunpos - ithis - 1) :
                                    0;
                    if (func) {

                        if (ipar) {
                            errIO->add("incomplete function call");
                            return sNotIdx;
                        }
                        ipos = iparentesis;

                        const void * bd = (*func)(this, ibase,
                                ref + ifunpos + 1, iparentesis - ifunpos - 2,
                                &frsize,outbuf);
                        if (errIO->length())
                            return sNotIdx;
                        if (bd == Link_INDEX)
                            ibase = frsize;
                        else {
                            if (!bd)
                                return sNotIdx;
                            ibase = addRecord(typeVal, frsize, bd);

                        }
                        if (ibase != sNotIdx)
                            continue;
                    }
                }
            }
            pdic = dicVar.get(ref + ithis, sz);
        }
        if (pdic) {
            ibase = *pdic;
            continue;
        }

        if (ibase == sNotIdx) {

            if (flags & fConstructPath) {
                ibase = addRecord(typeVal, sIdxMax, ref + ithis);
                if (supportVariableResolution && flags | fDictionarize && ref[ithis] == '$')
                    *dicVar.set((const char*) ref + ithis, sz) = ibase;
                continue;
            } else
                return sNotIdx;
        }

        Bucket searchBucket;
        searchBucket.toHash = &toHushBuf;
        const char * lst;
        idx num=0;
            idx base=1;
            for(lst=ref+ithis+sz-1; lst>ref+ithis && (*lst>='0' && *lst<='9'); --lst){
                num+=base*(*lst-'0');
                base*=10;
            }
            if(*lst!='#')num=0;
            else sz=lst-(ref+ithis);

        idx pp;for (pp=0; pp<sz && ref[ithis+pp]!='='; ++pp);
        if(pp<sz ) {
            getRelationBucketByHash(&searchBucket, typeRel, 0, 0, sNotPtr - 1, ibase);
            Bucket dodo;dodo.toHash=&toHushBuf;
            while( searchBucket.found()) {
                getRelationsByBucketAndIndex(&searchBucket, typeRel, results,0,false);
                getRelationBucketByHash(&dodo, typeRel, 2, 0, sNotPtr - 1, results[2].index, ref + ithis, pp, ref+ithis+pp+1, sz-(pp+1), false);
                if(dodo.found())
                    break;
                getNextRelationInBucket(&searchBucket, typeRel, 1);
            }
        }
        else {
            getRelationBucketByHash(&searchBucket, typeRel, 1, 0, sNotPtr - 1, ibase, ref + ithis, sz);
        }
        if(num>0) {
            getNextRelationInBucket(&searchBucket, typeRel, num);
        }

        if (searchBucket.found()) {
            getRelationsByBucketAndIndex(&searchBucket, typeRel, results, 0,false);
            ibase = results[2].index;
        } else {
            errIO->addString("unresolvable reference ");
            errIO->add(ref, sizeRef);
            return sNotIdx;
        }
    }
    if(ibase==0) {
        results[0].index=0;
        results[1].index=0;
        results[2].index=0;
    }

    return ibase;
}

idx sIonBirel::find(Bucket * bucket, const char * sub, idx sizeSub,
        const char * atr, idx sizeAtr, const char * val, idx sizeVal) {
    bucket->toHash = &toHushBuf;
    idx bodyArr[6];
    idx cnt ;
    idx relationHasherIndexType =0;

    if(sizeSub) {
        cnt = 2;
        bodyArr[0] = sConvPtr2Int(sub);
        bodyArr[1] = sizeSub;
        idx posForVal=2;
        if(sizeAtr) {
            bodyArr[2] = sConvPtr2Int(atr);
            bodyArr[3] = sizeAtr;
            posForVal+=2;
            relationHasherIndexType = 1;
        }
        if (sizeVal) {
            bodyArr[posForVal] = sConvPtr2Int(val);
            bodyArr[posForVal+1] = sizeVal;
            cnt = 3;
            if(!sizeAtr) {
                relationHasherIndexType = 3;
            }
            else relationHasherIndexType = 2;
        }
    }
    else {
        cnt=1;
        idx posForVal=0;
        if(sizeAtr) {
            bodyArr[0] = sConvPtr2Int(atr);
            bodyArr[1] = sizeAtr;
            posForVal+=2;
            relationHasherIndexType = 4;
        }
        if (sizeVal) {
            bodyArr[posForVal] = sConvPtr2Int(val);
            bodyArr[posForVal+1] = sizeVal;
            cnt = 2;
            if(!sizeAtr) {
                relationHasherIndexType = 6;
            }
            else relationHasherIndexType = 5;
        }
    }

    return getRelationBucketByHashVarg(bucket, typeRel, relationHasherIndexType,
            &cnt, bodyArr);

}

sIonBirel::LinkType * sIonBirel::look(LinkType * lnk, const char * sub,
        idx sizeSub, const char * atr, idx sizeAtr, const char * val,
        idx sizeVal, Bucket * bucket, idx * pcnt) {
    Bucket Bucket;
    if(!bucket)bucket=&Bucket;

    lnk->ixes.rel = find(bucket, sub, sizeSub, atr, sizeAtr, val, sizeVal);

    if (!bucket->found())
        return 0;

    idx relCnt = 1;
    sIon::RecordResult recordResults[4];

    getRelationsByBucketAndIndex(bucket, typeRel, recordResults, &relCnt,false);

    lnk->ixes.sub = recordResults[0].index;
    lnk->ixes.atr = recordResults[1].index;
    lnk->ixes.val = recordResults[2].index;
    if(pcnt)*pcnt=relCnt;
    if(!relCnt)return 0;
    return lnk;

}

sIonBirel::LinkType * sIonBirel::next(LinkType * lnk, Bucket * bucket ) {

    bool isdel=true;
    
        getNextRelationInBucket(bucket, typeRel, 1, &isdel);

        if (!bucket->found())
            return 0;

    idx relCnt = 1;
    sIon::RecordResult recordResults[4];

    getRelationsByBucketAndIndex(bucket, typeRel, recordResults, &relCnt,false);

    lnk->ixes.sub = recordResults[0].index;
    lnk->ixes.atr = recordResults[1].index;
    lnk->ixes.val = recordResults[2].index;
    if(!relCnt)return 0;
    return lnk;

}

sIonBirel::LinkType * sIonBirel::link(LinkType * lnk, const char * sub,
        idx sizeSub, const char * atr, idx sizeAtr, const char * val,
        idx sizeVal) {

    static LinkType lLnk;
    if (!lnk)
        lnk = &lLnk;
    lnk->ixes.flags = 0;

    if (sub == Link_INDEX)
        lnk->ixes.sub = sizeSub;
    else {
        lnk->ixes.sub = resolve(typeSub, (const char*) sub, sizeSub,
                fConstructPath | fDictionarize | fFuncEval);
        if (lnk->ixes.sub < 0)
            return 0;
        if (errIO->length())
            return 0;

    }

    idx sz = sizeAtr;
    idx iS = sNotIdx;
    char autoAtrBuf[64];autoAtrBuf[0]=0;
    if (atr == Link_INDEX)
        lnk->ixes.atr = sizeAtr;
    else {
        if(atr) {
            if ((atr[0] == '\'' || atr[0] == '\"')
                    && atr[sizeAtr - 1] == atr[0]) {
                ++atr;
                sizeAtr -= 2;
            }

            if (atr[0] == '+' || atr[0] == '-'
                    || (atr[0] >= '0' && atr[0] <= '9')) {



            }
            if(sizeAtr==1 && atr[0]=='#' ) {
                idx len,iN;
                Bucket bucket;
                for(iN=0; iN<sIdxMax; ++iN) {
                    sIPrintf(autoAtrBuf,len,iN,10);
                    find(&bucket, sub, sizeSub, autoAtrBuf, len, 0, 0);
                    if(!bucket.found())break;
                }
                if(iN==sIdxMax)return 0;
                atr=autoAtrBuf;
                sizeAtr=len;
            }
            if ( supportVariableResolution && ((const char *) atr)[0] == '$') {
                if (((const char *) atr)[1] == '$') {
                    ++atr;
                    --sizeAtr;
                } else {
                    const char * presolve = atr + sizeAtr - 1;
                    while (presolve > atr && *presolve != '.')
                        --presolve;
                    if (presolve == atr)
                        sizeAtr = sIdxMax;
                    else {
                        lnk->ixes.sub = resolve(typeSub, (const char*) atr,
                                presolve - atr, fDictionarize | fFuncEval);
                        if (lnk->ixes.sub != sNotIdx) {
                            lnk->ixes.val = resolve(typeVal, (const char*) atr,
                                    sizeAtr, fDictionarize | fFuncEval);
                            if (lnk->ixes.val != sNotIdx) {
                                return lnk;
                            }
                        }
                    }
                    lnk->ixes.atr = resolve(typeAtr, (const char*) atr, sizeAtr,
                            fDictionarize | fFuncEval);
                    sizeAtr = sIdxMax;
                }
            }
        }
        lnk->ixes.atr = addRecord(typeAtr, sizeAtr, atr);
    }

    if (val == Link_INDEX)
        lnk->ixes.val = sizeVal;
    else {
        if(sizeVal==1){
            if(val[0]=='#') {val=0;sizeVal=sIonBirel::Link_AUTOARR;}
            else if( val[0]=='@') {val=0;sizeVal=sIonBirel::Link_AUTOVAL;}
        }
        else if(supportVariableResolution && sizeVal && val && strncmp(val,"$del()",sMin(sizeVal,(idx)6))==0)return 0;

        idx flags = fFuncEval;
        if (sizeVal < sIdxMax - 1) {
            if (val[0] == '\'')
                flags &= ~fFuncEval;
            if ((val[0] == '\"') && val[sizeVal - 1] == val[0]) {
                ++val;
                sizeVal -= 2;
            }
            if (supportVariableResolution && sizeVal == 4 && val[0] == '$' && val[1] == 'a' && val[2] == 't'
                    && val[3] == 'r') {
                val = atr;
                sizeVal = sizeAtr;
            }
            lnk->ixes.val = resolve(typeVal, (const char*) val, sizeVal, flags);
        }
        if ((sizeVal == sIdxMax - 1 || sizeVal == sIdxMax)
                || lnk->ixes.val == sNotIdx)
            lnk->ixes.val = addRecord(typeVal, sizeVal, val);

        if (supportVariableResolution && atr && ((const char *) atr)[0] == '$') {
            *dicVar.set((const char*) atr, sz) = lnk->ixes.val;
        }
    }
    if (lnk->ixes.sub < 0)
        return 0;
    lnk->ixes.rel = addRelationVarg(typeRel, iS, lnk->arr, 0);

    return lnk;
}

idx sIonBirel::del(const char * sub, idx sizeSub, const char * atr, idx sizeAtr,
        const char * val, idx sizeVal, idx istart, idx icnt) {
    if (icnt <= 0)
        return 0;

    Bucket bucket;
    find(&bucket, sub, sizeSub, atr, sizeAtr, val, sizeVal);
    if (!bucket.found())
        return 0;


    return deleteRelationsByBucketAndIndex(&bucket, typeRel, istart, &icnt, 0);
    return 1;

}

idx sIonBirel::set(idx sub, const char * path, const char * value00, idx valSize, idx valCnt, sDic < idx> * sublist) {
    LinkType mid,lnk;
    idx curSub;
    char ibuf[64];
    idx ii = 0, ibuflen, vlen;

    curSub = sub;

    idx cnt = 0, tp=sIonBirel::Link_AUTOVAL, * psub;
    const char * cur=path,*nxt;

    for (nxt = cur; *cur; cur = nxt) {

        for (nxt = cur + 1; *nxt && *nxt != '.'; ++nxt) {
        }
        if(!*nxt)break;
        if (*cur == '#') {
            tp = sIonBirel::Link_AUTOARR;
            ++cur;
        } else
            tp = sIonBirel::Link_AUTOVAL;
        psub=0;
        if(sublist && *cur=='{' ) {
            psub=sublist->get(cur+1,nxt-cur-1);
            if(!psub) {
                sIPrintf(ibuf, ibuflen, ii, 10);++ii;
                link(&mid, sIonBirel_INDEX(curSub), ibuf,ibuflen, sIonBirel_AUTOVAL);
                psub=sublist->set(cur+1,nxt-cur-1);
                *psub=mid.ixes.val;
            }
            curSub=*psub;

        }
        else {
            if (!look(&mid, sIonBirel_INDEX(curSub), cur, nxt - cur, 0, 0))
                link(&mid, sIonBirel_INDEX(curSub), cur, nxt - cur, 0, tp);
            curSub = mid.ixes.val;
        }


        if (*nxt)
            ++nxt;
    }



    const char * val = value00;
    ii=0;
    while (val) {

        bool doAdd=true;
        vlen=(valSize ? valSize : sLen(val));


        if(cur[0]=='-' || cur[0]=='+' ) {
            if(tp == sIonBirel::Link_AUTOVAL)look(&lnk, sIonBirel_INDEX(curSub), sIonBirel_STRING((cur+1)),  val,vlen);
            else look(&lnk, sIonBirel_INDEX(curSub), 0,0,  val,vlen);
            if(lnk.ixes.rel!=sNotIdx)
                doAdd=false;
        }

        if(doAdd){
            if(tp == sIonBirel::Link_AUTOVAL) {
                if(cur[0]=='-' || cur[0]=='+' ) link(&lnk, sIonBirel_INDEX(curSub), sIonBirel_STRING((cur+1)),  val,vlen);
                else link(&lnk, sIonBirel_INDEX(curSub), sIonBirel_STRING(cur),  val,vlen );
            }else {
                sIPrintf(ibuf, ibuflen, ii, 10);++ii;
                link(&lnk, sIonBirel_INDEX(curSub), ibuf,ibuflen,  val,vlen );
            }
        }
        ++cnt;

        if (valCnt && cnt >= valCnt)
            break;
        if(valSize)
            valSize-=(vlen+1);

        val = sString::next00(val);
        if (valSize && val >= value00 + valSize)
            break;


    }
    return cnt;
}

idx sIonBirel::analyzeValue(sJax * jax, sJax::JsonFrame * fr,
        sJax::JsonFrame * prv) {
    if (!prv)
        return 1;

    char buf[1024];
    idx sizeSub, sizeAtr, sizeVal;
    const char * sub, *atr, *val = 0;
    if (!prv) {
        sub = (const char *) Link_INDEX;
        sizeSub = root;
    } else {

        if (prv->valSize) {
            sub = prv->valOfs;
            sizeSub = prv->valSize;
        }
        else {
            sub = (const char *) Link_INDEX;
            sizeSub = prv->userData != -1 ? ltStack.ptr(prv->userData)->ixes.val : root;
        }
    }

    sizeAtr = fr->varSize;
    atr = fr->varOfs;
    if (atr[0] == '[' && sizeAtr == 1) {
        idx l = prv ? prv->rowNum - 1 : 0;
        sIPrintf(buf, sizeAtr, l, 10);
        atr = buf;
    }

    if (fr->valSize) {
        val = fr->valOfs;
        sizeVal = fr->valSize;
    } else {
        if (fr->type == '[')
            sizeVal = sIdxMax - 1;
        else
            sizeVal = sIdxMax;
    }

    ltStack.resize(fr->depth);
    fr->userData = fr->depth - 1;
    LinkType * lnk = ltStack.ptr(fr->userData);
    link(lnk, sub, sizeSub, atr, sizeAtr, val, sizeVal);
    if (errIO->length())
        return 0;

    if (debug) {
        idx * sz;
        const char * bd;
        ::printf("%" DEC " ", fr->lineNumber);
        for (idx i = 0; i < fr->depth - 2; ++i)
            ::printf("  ");

        bd = (const char*) getRecordBody(0, lnk->ixes.sub, &sz);
        if (bd[0] != 0)
            ::printf("%.*s", (int) (*sz), (const char*) bd);
        else
            ::printf("%llx", *(idx *) bd);

        bd = (const char*) getRecordBody(1, lnk->ixes.atr, &sz);
        printf(".");
        if (bd[0] != 0)
            ::printf("%.*s", (int) (*sz), (const char*) bd);
        else
            ::printf("$%" DEC, lnk->ixes.atr);
        printf("=");

        bd = (const char*) getRecordBody(2, lnk->ixes.val, &sz);
        if (bd[0] != 0)
            ::printf("%.*s", (int) (*sz), (const char*) bd);
        else
            ::printf("%llx", *(idx *) bd);

        ::printf("\t\t\t $%" DEC ".$%" DEC "=$%" DEC, lnk->ixes.sub, lnk->ixes.atr,
                lnk->ixes.val);
        ::printf("\n");
    }

    return 1;
}
const void * sIonBirel::func_ionql(sIonBirel * ibr, idx isub,
        const char * arglist, idx len, idx * psize, char * outbuf) {
    sIonWander * w = ibr->wander("_resolve", true);
    if (!w->ionList.dim())
        w->addIon(ibr, 0);
    w->resetCompileBuf();
    w->traverseFieldSeparator = 0;
    w->traverseRecordSeparator = 0;

    const void * ret;
    w->traverseCompile(arglist, len, ibr->errIO, false);
    if (ibr->errIO->length()) {
        ret = ibr->errIO->ptr();
        *psize = ibr->errIO->length();
    } else {
        w->traverse();
        ret = w->pTraverseBuf->ptr();
        if (!ret) {
            ibr->errIO->add("no ionql results");
        } else {
            *psize = w->pTraverseBuf->length();
        }
    }
    return ret;
}
const void * sIonBirel::func_timenow(sIonBirel * ibt, idx isub, const char * arglist, idx len, idx * psize, char * outbuf ) {
    time_t t;
    time(&t);
    sIPrintf(outbuf, (*psize), t, 10);
    return outbuf;
}
const void * sIonBirel::func_newid(sIonBirel * ibt, idx isub, const char * arglist, idx len, idx * psize, char * outbuf ) {
    idx res = ibt->autoNumber(ibt->typeSub);
    sIPrintf(outbuf, (*psize), res, 10);
    return outbuf;
}
const void * sIonBirel::func_randint(sIonBirel * ibt, idx isub, const char * arglist, idx len, idx * psize, char * outbuf ) {
    idx res = rand(), max;
    if(arglist){
        sIScanf(max, arglist, len, 10 );
        if(max)res%=(max+1);
    }
    sIPrintf(outbuf, (*psize), res, 10);
    return outbuf;
}

const void * sIonBirel::func_randreal(sIonBirel * ibt, idx isub, const char * arglist, idx len, idx * psize, char * outbuf )
{
    idx res = rand(),max;
    if(arglist){
        sRScanf(max, arglist, len, 10 );
        max*=1000;
        if(max)res%=((int)max+1);
    }
    real rres=res/1000.;

    sFlPrintf(outbuf, (*psize), rres, 10, 3);
    return outbuf;
}


const void * sIonBirel::func_randstr(sIonBirel * ibt, idx isub, const char * arglist, idx len, idx * psize, char * outbuf ) {
    strcpy(outbuf,sWord::getWord(rand()));
    strcat(outbuf,"-");
    strcat(outbuf,sWord::getWord(rand()));
    if(psize)*psize=sLen(outbuf);
    return outbuf;
}


