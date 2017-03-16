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
#include <slib/utils/txt.hpp>
#include <slib/std/string.hpp>

using namespace slib;

void sText::fmtParseLeftTag(sVar * flist, const char * src, idx len,const char * subsects00, idx caselo, bool constid, const char * separsvar,const char * separsval)
{
    if(!separsvar) separsvar=sString_symbolsBlank;
    if(!separsval) separsval=sString_symbolsBlank;
    sStr tmp; 
    sStr id;
    idx idlen=0;
    sDic < sStr  > fldist;
    sString::searchAndReplaceSymbols(&tmp,src,len, sString_symbolsEndline, 0,0,true,true,false);
    
    for ( const char * p = tmp.ptr(); p ; p=sString::next00(p) ) { 
        //const char * cont=sString::extractSubstring(&id,p,0, 1,sString_symbolsBlank, caseSensitive,true);
        if(!(*p))continue;
        const char * cont=sString::skipWords( p, 0, 1,separsvar);
        if(!constid || !idlen)idlen=(idx)(cont-p);
        id.cut(0);sString::cleanEnds(&id, p,cont ? idlen : 0 ,separsval,true);
        if(caselo!=sString::eCaseNone)sString::changeCase(id.ptr(),id.length(),(sString::eCase)caselo);
        if(idlen)cont=p+idlen;

        sStr * pstr=fldist.set(id.ptr());
        if(!pstr)continue;
        if(pstr->length())pstr->printf("\n");
        if(cont)pstr->printf("%s",cont);
    }

    for( idx i=0 ; i< fldist.dim(); ++i) { 
        sStr * pstr=fldist.ptr(i);pstr->add0(1);
//        const char * nm=(const char * )fldist.id(i);
//        const char * p=pstr->ptr(0);
        if( pstr->length() ) flist->inp((const char * )fldist.id(i), pstr->ptr(0), pstr->length()) ;
        else flist->inp((const char * )fldist.id(i), __, 2) ;
    }

    for ( const char *ss=subsects00; ss ; ss=sString::next00(ss) ){
        const char * ssval=flist->value(ss);
        if(ssval && *ssval) 
            sText::fmtParseLeftTag(flist, ssval,sLen(ssval),0,caselo,false, separsvar,separsval);
    }

}

idx sText::compileSearchStrings(const char * cmp00, sVec < SearchStruc > * ssv)
{

    for (const char * cmp = cmp00; cmp; cmp=sString::next00(cmp) )
    {

        SearchStruc * ss = ssv->add(1);
        ss->srch=cmp;
        ss->len=sLen(cmp);

        if (ss->len > 6 && strncmp(ss->srch, "regex:", 6) == 0) {
            regcomp(&ss->rex, ss->srch+6, 0);
            ss->srch += 6;
        }
        else ss->rex.allocated=0;
    }
}

idx sText::matchSearchToString(const char * str, idx len0, sText::SearchStruc * ss, idx cnt, idx maxmatch)
{
    sStr regBuf;
    idx ismatch=0;

    for ( idx i=0; i<cnt; ++i ) {


        if(ss->rex.allocated) {
            regBuf.add(str, len0);
            regBuf.add0(1);
            ismatch += regexec(&ss->rex, (const char*) regBuf.ptr(0), 0, 0, 0) ? 0 : 1;
            regBuf.cut(0);
        } else {
            idx is=0, ip;
            idx pl=ss->len;
            const char * pp=ss->srch;
            idx len=len0, l0=len;

            if (*pp  == '^') {
                ++pp;
                --pl;
                len=pl;
            }
            if (pp[pl- 1] == '$') {
                --pl;
                if (ss->srch[0] == '^') {
                    if (l0 > pl)
                        len = 0;
                    else
                        --len;
                } // do not compare
                else {
                    is = len - pl;
                }
            }
            for (; is <= len - pl; ++is) {
                for (ip = 0; ip < pl && str[is + ip] == pp[ip]; ++ip)
                    ;
                if (ip == pl) {
                    ++ismatch;
                    break;
                }
            }
        }
        if(ismatch >= maxmatch)
            return ismatch;
        ++ss;
    }
    return 0;
}
