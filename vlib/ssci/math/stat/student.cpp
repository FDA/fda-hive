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
#include <ssci/math/stat/stat.hpp>
#include <ssci/math/func/func.hpp>
#include <math.h>

using namespace slib;

#define arr(_v_row, _v_col)     (actmat[(_v_row)*cols+(_v_col)])

void sStat::statTest(real * actmat, idx rows, idx cols, sVec < sVec < idx > >  * colset, real * probvals, real * tstatvals)
{
    struct tagSta{real ss,ave ; idx n;} sta[2];
    idx ir, ic, is , grps, isT=0;

    grps=colset->dim();
    if(grps==2)isT=1;

    for ( ir=0; ir < rows; ++ir ) {
        real ssWAll=0, aveAll=0, ssBAll=0, prob, tval;
        idx nAll=0;

        for( is=0; is<grps ; ++is ) {
            sVec < idx > & cur=*(colset->ptr(is));
            idx cc=cur.dim();

            real ave=0 , ss, s ;
            for ( ic=0; ic<cc; ++ic) {
                ave+=arr(ir, cur[ic] );
            }
            aveAll+=ave;
            ave/=cur.dim();
            ss=0;
            for ( ic=0; ic<cc; ++ic) {
                s=arr(ir, cur[ic] )-ave;
                ss+=s*s;
            }
            ssWAll+=ss;

            if(isT) {
                sta[is].ss=ss;
                sta[is].ave=ave;
                sta[is].n=cc;
            }
            nAll+=cc;

            ssBAll+=cc*ave*ave;
        }
        aveAll/=nAll;
        ssBAll-=nAll*aveAll*aveAll;

        if(isT) {
            idx df=sta[0].n+sta[1].n-2;
            real t=(sta[0].ave-sta[1].ave)/sqrt((sta[0].ss/((sta[0].n-1)*sta[0].n)+sta[1].ss/((sta[1].n-1)*sta[1].n)));
            t=sAbs(t);
            prob=sStat::studentsT(t, (real)df );
            tval=t;
        }else {
            idx dfb=(grps-1);
            idx dfw=(nAll-grps);
            real MSB=ssBAll/dfb;
            real MSW=ssWAll/dfw;
            real f=MSB/MSW;
            prob=sStat::studentsF(f, (real)dfb, (real)dfw );
            tval=f;
        }
        probvals[ir]=prob;
        tstatvals[ir]=tval;
    }
}


void sStat::statTestCols(real * actmat, idx rows, idx cols, sDic < sVec < idx > >  * rowset, real * probvals, real * tstatvals, bool isPariwiseTTest, tagSta * stats)
{
    tagSta sta[1024];
    idx ir, ic, is , grps, isT=0;
    sVec <idx> paired;

    grps=rowset->dim();
    if(grps==2 || isPariwiseTTest)isT=1;

    idx cntGrp=grps, ipend, ipstart, ik=0;
    if( isPariwiseTTest) {
        cntGrp=2;
        ipstart=0;
        ipend=grps*(grps-1)/2;
    }else {
        cntGrp=grps;
        ipstart=0;
        ipend=1;
    }

    if (stats){
        for ( ic=0; ic < cols; ++ic ) {
            for (is=0; is<grps; ++is ) {
                sVec < idx > & cur=*(rowset->ptr(is));
                idx cc=cur.dim();

                real ave=0 , ss, s ;
                for ( ir=0; ir<cc; ++ir) {

                    ave+=arr(cur[ir], ic );
                }
                ave/=cur.dim();
                ss=0;
                for ( ir=0; ir<cc; ++ir) {
                    s=arr(cur[ir] , ic )-ave;
                    ss+=s*s;
                }
                stats[ic*grps+is].ave=ave;
                stats[ic*grps+is].n=cc;
                stats[ic*grps+is].ss = cc > 1 ? sqrt(ss / (cc - 1)) : 0;
            }
        }
    }

    for ( ic=0; ic < cols; ++ic ) {
        idx rr2=0,cc2=1;

        for ( idx ip=ipstart ; ip<ipend; ++ip , ++cc2)  {
            real ssWAll=0, aveAll=0, ssBAll=0, prob, tval;
            idx nAll=0;


            for( is=0; is<cntGrp; ++is ) {
                idx dex=is;
                if( isPariwiseTTest )  {
                    dex=is==0 ? rr2 : cc2;
                }
                sVec < idx > & cur=*(rowset->ptr(dex));
                idx cc=cur.dim();

                real ave=0 , ss, s ;
                for ( ir=0; ir<cc; ++ir) {

                    ave+=arr(cur[ir], ic );
                }
                aveAll+=ave;
                ave/=cur.dim();
                ss=0;
                for ( ir=0; ir<cc; ++ir) {
                    s=arr(cur[ir] , ic )-ave;
                    ss+=s*s;
                }
                ssWAll+=ss;

                if(isT) {
                    sta[is].ss=ss;
                    sta[is].ave=ave;
                    sta[is].n=cc;
                }
                nAll+=cc;

                ssBAll+=cc*ave*ave;
            }


            if(isT) {
                idx df=sta[0].n+sta[1].n-2;
                real sJoint=ssWAll/df;
                real t=sJoint ? (sta[0].ave-sta[1].ave)/sqrt(sJoint*(1./sta[0].n+1./sta[1].n)) : 0 ;
                t=sAbs(t);
                prob=sStat::studentsT(t, (real)df );
                tval=t;
            }else {
                aveAll/=nAll;
                ssBAll-=nAll*aveAll*aveAll;

                idx dfb=(grps-1);
                idx dfw=(nAll-grps);
                real MSB=ssBAll/dfb;
                real MSW=ssWAll/dfw;
                real f=MSB/MSW;
                prob=sStat::studentsF(f, (real)dfb, (real)dfw );
                tval=f;
            }

            probvals[ik]=prob;
            tstatvals[ik]=tval;
            ++ik;

            if( isPariwiseTTest  && cc2==grps-1) {
                rr2++;cc2=rr2;
            }


        }

    }
}



real sStat::studentsT(real t, real df)
{
    return 1.-sFunc::special::betaIncomplete(0.5*df,0.5,df/(df+t*t));
}

real sStat::studentsF(real f, real df1, real df2)
{
    real prob=sFunc::special::betaIncomplete(0.5*df2,0.5*df1,df2/(df2+df1*f));
    if (prob > 1.0) prob=2.0-prob;
    return prob;
}


