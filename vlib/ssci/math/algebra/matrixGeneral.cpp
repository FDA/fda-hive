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
#include <ssci/math/algebra/algebra.hpp>
#include <math.h>

using namespace slib;

#define a(val1,val2)    (*( (real  *)A+acols*(val1)+(val2) ))
#define b(val1,val2)    (*( (real  *)B+bcols*(val1)+(val2) ))
#define c(val1,val2)    (*( (real  *)C+dim2*(val1)+(val2) ))
#define p(val)          (*( (real  *)P+(val) ))
#define q(val)          (*( (real  *)Q+(val) ))



void sAlgebra::matrix::multiplyToMatrix(const real   * A,idx arows, idx acols, const real   * B, idx brows, idx bcols, real   * C, bool transposesecond )
{
    if(!transposesecond && acols!=brows)return;
    if(transposesecond && acols!=bcols)return;

    idx dim1=arows;
    idx dim2=transposesecond ? brows : bcols ;

    for(idx i=0;i<dim1;i++){
        for(idx j=0;j<dim2;j++){
            real coef=0;
            if(transposesecond){
                for(idx k=0;k<acols && k<bcols;k++){
                    coef+=a(i,k)*b(j,k);
                }
            } else {
                for(idx k=0;k<acols && k<brows;k++){
                    coef+=a(i,k)*b(k,j);
                }
            }
            c(i,j)=coef;

        }
    }
    return ;
}


void sAlgebra::matrix::multiplyToVector(const real   * A,idx arows, idx acols, const real   * P,real   * Q)
{
    idx      i,k;
    for(i=0;i<arows;i++){
        q(i)=0;
        for(k=0;k<acols;k++){
            q(i)+=a(i,k)*p(k);
        }
    }
    return;
}

#undef  a
#undef  b
#undef  c
#undef  p
#undef  q

#define val(val1,val2)    (*( (real  *)src+cols*(val1)+(val2) ))


void sAlgebra::matrix::transpose(real * src, idx rows, idx cols)
{
    for( idx ir=0; ir<rows; ++ir){
        for( idx ic=ir+1; ic<cols; ++ic){
            real t=val(ir,ic);
            val(ir,ic)=val(ic,ir);
            val(ic,ir)=t;
        }
    }
}

void sAlgebra::matrix::normalizeCols(real * src, idx rows, idx cols, bool transpose, bool sqr, real scale)
{

    if(transpose) {
        for( idx ir=0; ir<rows; ++ir) {
            real sum=0;
            if(sqr){
                for( idx ic=0; ic<cols; ++ic)
                    sum+=val(ir,ic)*val(ir,ic);
                sum=sqrt(sum);
            }else {
                for( idx ic=0; ic<cols; ++ic)
                    sum+=val(ir,ic);
            }
            sum/=scale;
            for( idx ic=0; ic<cols; ++ic) val(ir,ic)/=sum;
        }
    }else {
        for( idx ic=0; ic<cols; ++ic) {
            real sum=0;
            if(sqr){
                for( idx ir=0; ir<rows; ++ir)
                    sum+=val(ir,ic)*val(ir,ic);
                sum=sqrt(sum);
            }else {
                for( idx ir=0; ir<rows; ++ir)
                    sum+=val(ir,ic);
            }
            sum/=scale;
            for( idx ir=0; ir<rows; ++ir) val(ir,ic)/=sum;
        }
    }
}

void sAlgebra::matrix::shiftRows(real * src, idx rows, idx cols, const real * shift, real scale)
{
    idx ic,ir;
    for(ir=0; ir<rows; ++ir){
        for( ic=0;ic<cols; ++ic) {
            val(ir,ic)-=shift[ic]*scale; // shift the data in current column
        }
    }
}

void sAlgebra::matrix::cutoffMinVal(real * src, idx rows, idx cols, real mintol)
{
    for(idx ir=0; ir<rows; ++ir) {
        for(idx ic=0; ic<cols; ++ic) {
            if(sAbs(val(ir,ic))<mintol)
                val(ir,ic)=mintol+(cols-ic)*mintol/cols;
        }
    }
}



#undef val
