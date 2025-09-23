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
#include <ssci/math.hpp>
#include <ssci/math/algebra/algebra.hpp>

using namespace slib;

#define v(val1,val2)    (*( (real *)evecs+n*(val1)-n+(val2)-1 ))
#define d(val)  (*( (real *)evals+(val)-1 ))

void sAlgebra::matrix::diagSort(idx n,real * evals,real   * evecs,idx  issort, bool keepParity)
{
    idx imin,i,j,p;
    real tau;

    idx parity=0;
    if(issort){
        for(i=1;i<=n;i++){
            for(imin=i,j=i+1;j<=n;j++){
                if(issort==1){if(fabs(d(j))<fabs(d(imin)))imin=j;}
                else if (issort==-1){if(fabs(d(j))>fabs(d(imin)))imin=j;}
            }
            if(imin==i) continue;
            ++parity;

            tau=d(i);
            d(i)=d(imin);
            d(imin)=tau;
            if(evecs) { 
                for(p=1;p<=n;p++){
                    tau=v(p,i);
                    v(p,i)=v(p,imin);
                    v(p,imin)=tau;
                }
            }
        }
        if(keepParity && (parity%2) ){
            for(p=1;p<=n;++p){
                v(p,1)*=-1;
            }
        }
    }
   
}

#undef  v
#undef  d


