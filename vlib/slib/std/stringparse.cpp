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

#include <slib/std/string.hpp>

#include <ctype.h>

using namespace slib;


idx sString::readChoiceList(idx * visCols, idx * sizCols, const char * cols00, const char * defline, const char * separ, bool isCaseInsensitive)
{
    idx i=0,cnt=0,num=0;
    const char * p;
    for(const char * ptr=defline; ptr && *ptr; ptr=sString::skipWords(ptr,0,1,separ), ++i ) {
        if( sString::compareChoice(ptr, cols00, &num, isCaseInsensitive,0) != sNotIdx ) {
            if(visCols)visCols[num]=1;
            if(sizCols && (p=strchr(ptr,':'))!=0 )
                sizCols[num]= atoi(p+1);

            ++cnt;
        }
    }
    return cnt;
}

idx sString::composeChoiceList(sStr * dfl,const char * cols00, idx * visCols, idx * sizCols, const char * separ)
{
    idx i=0,cnt=0;
    for(const char * ptr=cols00; ptr && *ptr; ptr=sString::next00(ptr,1), ++i ) {
        if(visCols[i]) {
            if(cnt) dfl->addSeparator(separ);
            dfl->printf("%s",ptr);
            if(sizCols)dfl->printf(":%" DEC,sizCols[i]);
            ++cnt;
        }
    }
    return cnt;
}


idx sString::scanRangeSet(const char * src, idx len, sVec<idx> * range, idx shift, idx cidfirst, idx cidlast)
{
    if( !*src ) {
        return 0;
    }
    if( !len ) {
        len = sLen(src);
    }
    idx icid = 0, prvcid = 0;
    const char * e = src + len;
    char mbyf[128];

    for(const char * ptr = src; ptr && ptr < e;) {
        idx mbyfLen = sMin<idx>(sizeof(mbyf) - 1, e - ptr);
        strncpy(mbyf, ptr, mbyfLen);
        mbyf[mbyfLen] = 0;
        if( (mbyf[0] == '0' && (mbyf[1] == 'x' || mbyf[1] == 'X')) || (mbyf[0] == '-' && mbyf[1] == '0' && (mbyf[2] == 'x' || mbyf[2] == 'X')) ) {
            if( !sscanf((mbyf[0] == '-') ? mbyf + 3 : mbyf + 2, "%" HEX, &icid) ) {
                break;
            }
            if( mbyf[0] == '-' ) {
                icid = -icid;
            }
        } else if( !sscanf(mbyf, "%" DEC, &icid) ) {
            break;
        }
        if( icid < 0 && ptr != src ) {
            for(int ii = prvcid + 1; ii <= (-icid); ++ii) {
                range->vadd(1, ii + shift);
            }
        } else {
            if( cidfirst == cidlast || (icid >= (long) cidfirst && icid <= (long) cidlast) ) {
                range->vadd(1, icid + shift);
            }
        }
        prvcid = icid;
        ptr = strpbrk(ptr + 1, " \t\n,;-/");
        if( ptr ) {
            ptr = strpbrk(ptr + (*ptr == '-' ? 0 : 1), "0123456789-xX");
        }
    }
    return range->dim();
}

idx sString::scanRangeSetSet(const char * src00,  sVec < sVec < idx >  > * colset )
{
    sVec < idx > * cur=0;

    for(const char * p=src00; p ; p=sString::next00(p,1)) {
        if(!cur || cur->dim()) cur=colset->add();
        sString::scanRangeSet(p,0, cur,0,0,0);
    }
    if(cur && !cur->dim())
        colset->del(colset->dim()-1,1);
    return colset->dim();
}




idx sString::splitRange(real sVal, real fVal, idx cntTicks, sVec < real > * vec, bool filterBoundaries )
{
    if(cntTicks<2)cntTicks=2;
    real vd=fVal-sVal;
    if(vd==0)return 0;
    real vstp=vd/cntTicks;
    cntTicks+=2;
    sVec < idx > VV; VV.resize(cntTicks);

    real acc=1., bestacc=-1;
    idx iAcc=0,iBestAcc=0;
    idx il,i;

    if(vec)vec->resize(cntTicks+1);

    while( acc != bestacc) {

        for(i=0; i<cntTicks; ++i) {
            VV[i]=(idx)((sVal+vstp*i)*acc);

            for(il=0; il<i && VV[i]!=VV[il]; ++il );
            if(il<i)break;
        }
        if(i<cntTicks){
            acc*=10;
            ++iAcc;
        }
        else {
            bestacc=acc;
            iBestAcc=iAcc;
            acc/=10;
            --iAcc;
        }

    }
    if(vec){
        for(i=0;i<cntTicks;++i)VV[i]=(idx)((sVal+vstp*i)*bestacc);
        for(i=0,il=0;i<cntTicks;++i){
            real val=VV[i]/bestacc;
            if(filterBoundaries && (val<sVal || val>fVal))continue;
            (*vec)[il++]=val;
        }
        (*vec).cut(il);
    }
    return iBestAcc;
}

namespace {
    struct NaturalSeg {
        enum {
            eString,
            eInt,
            eSep
        } type;
        idx len;
    };
};

static inline bool isDotDash(char c) {
    return c == '.' || c == '-' || c == '_';
}

static idx readNaturalSeg(NaturalSeg & seg, const char * s)
{
    sSet(&seg);
    if( !s || !s[0] ) {
        return 0;
    }

    if( isdigit(s[0]) ) {
        seg.type = NaturalSeg::eInt;
        for(; isdigit(s[seg.len]); seg.len++);
    } else if( isDotDash(s[0]) ) {
        seg.type = NaturalSeg::eSep;
        seg.len = 1;
    } else if( isspace(s[0]) ) {
        seg.type = NaturalSeg::eSep;
        for(; isspace(s[seg.len]); seg.len++);
    } else {
        seg.type = NaturalSeg::eString;
        for(; s[seg.len] && !isdigit(s[seg.len]) && !isDotDash(s[seg.len]) && !isspace(s[seg.len]); seg.len++);
    }

    return seg.len;
}

int sString::cmpNatural(const char * a, const char * b, bool caseSensitive)
{
    if( a == b ) {
        return 0;
    } else if( !a ) {
        return -1;
    } else if( !b ) {
        return 1;
    }

    while( 1 ) {
        NaturalSeg sega, segb;
        idx lena = readNaturalSeg(sega, a);
        idx lenb = readNaturalSeg(segb, b);
        if( !lena ) {
            return lenb ? -1 : 0;
        } else if( !lenb ) {
            return 1;
        }

        if( sega.type == NaturalSeg::eInt ) {
            if( segb.type == NaturalSeg::eInt ) {
                if( idx diff = sega.len - segb.len ) {
                    return diff;
                }
            } else {
                return -1;
            }
        } else if( segb.type == NaturalSeg::eInt ) {
            return 1;
        }
        int diff = caseSensitive ? strncmp(a, b, sMin<idx>(sega.len, segb.len)) : strncasecmp(a, b, sMin<idx>(sega.len, segb.len));
        if( !diff ) {
            diff = sega.len - segb.len;
        }
        if( diff ) {
            return diff;
        }

        a += sega.len;
        b += segb.len;
    }

    return 0;
}

bool sString::extractNCBIInfoSeqID(sStr *ginum, sStr *accnum, const char *seqid, idx seqlen)
{
    sStr buf, idseq;
    if (!seqlen){
        seqlen = sLen (seqid);
    }
    sString::copyUntil(&idseq, seqid, seqlen, " ");
    idseq.shrink00();
    buf.cut(0);
    searchAndReplaceSymbols(&buf, idseq.ptr(), idseq.length(), "|", 0, 0, true, true, false, false);
    if (cnt00(buf) > 1){
        idx gipos = 2;
        idx accpos = 4;
        if (cnt00(buf) == 5){
            gipos = 3;
            accpos = 5;
        }
        if (strncmp(next00(buf, accpos-2), "gi", 2) == 0){
            idx t = gipos;
            gipos = accpos;
            accpos = t;
        }
        if (ginum){
            const char *p = next00(buf, gipos-1);
            copyUntil(ginum, p, 0, ".");
        }
        if (accnum){
            const char *p = next00(buf, accpos-1);
            accnum->addString(p);
        }
    }
    else if (accnum){
        accnum->addString(buf.ptr(), buf.length());
    }
    return true;
}

bool sString::stringEndsWith (const char *str, idx str_len, const char *suffix, idx suffix_len)
{
    if (str_len <= 0){
        str_len = sLen (str);
    }
    if (suffix_len <= 0){
        suffix_len = sLen (suffix);
    }
    return ((str_len >= suffix_len) && (strncmp(str + (str_len-suffix_len), suffix, suffix_len) == 0));

}
