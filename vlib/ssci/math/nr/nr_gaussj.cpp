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
#include <slib/core/str.hpp>
#include <ssci/math/nr/nrutil.h>
#include <math.h>

#define NRANSI
#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
#define a(_v_1, _v_2)  (A[((_v_1)-1)*n+(_v_2)-1])
#define b(_v_1, _v_2)  (B[((_v_1)-1)*m+(_v_2)-1])

idx sMathNR::gaussj(real * A, idx n, real *B, idx m)
{
    idx *indxc,*indxr,*ipiv;
    idx i,icol=0,irow=0,j,k,l,ll;
    real big,dum,pivinv,temp;

    indxc=sMathNRUtil::ivector(1,n);
    indxr=sMathNRUtil::ivector(1,n);
    ipiv=sMathNRUtil::ivector(1,n);
    for (j=1;j<=n;j++) ipiv[j]=0;
    for (i=1;i<=n;i++) {
        big=0.0;
        for (j=1;j<=n;j++)
            if (ipiv[j] != 1) {
                for (k=1;k<=n;k++) {
                    if (ipiv[k] == 0) {
                        if (fabs(a(j,k)) >= big) {
                            big=fabs(a(j,k));
                            irow=j;
                            icol=k;
                        }
                    }
                }
            }
            ++(ipiv[icol]);

            if (irow != icol) {
                for (l=1;l<=n;l++) SWAP(a(irow,l),a(icol,l))
                for (l=1;l<=m;l++) SWAP(b(irow,l),b(icol,l))
            }
            indxr[i]=irow;
            indxc[i]=icol;
            if (sAbs(a(icol,icol)) <= 1e-8)  {
                sMathNRUtil::nrerror("gaussj: Singular Matrix");
                return i;
            }
            
            pivinv=1.0/a(icol,icol);
            a(icol,icol)=1.0;
            for (l=1;l<=n;l++) a(icol,l) *= pivinv;
            for (l=1;l<=m;l++) b(icol,l) *= pivinv;

            for (ll=1;ll<=n;ll++)
                    if (ll != icol) {
                            dum=a(ll,icol);
                            a(ll,icol)=0.0;
                            for (l=1;l<=n;l++) 
                                a(ll,l) -= a(icol,l)*dum;
                            for (l=1;l<=m;l++) 
                                b(ll,l) -= b(icol,l)*dum;
                    }
    }
    for (l=n;l>=1;l--) {
        if (indxr[l] != indxc[l])
            for (k=1;k<=n;k++)
                SWAP(a(k,indxr[l]),a(k,indxc[l]));
    }
    sMathNRUtil::free_ivector(ipiv,1,n);
    sMathNRUtil::free_ivector(indxr,1,n);
    sMathNRUtil::free_ivector(indxc,1,n);
    return 0;
}



idx sMathNR_gaussjMi(real * A, idx n, real *B, idx m)
{
    for( idx row=1; row<=n; ++row) {
        idx maxbig=row;
        for (idx ibig=maxbig+1; ibig<=n; ++ibig) {
            if( sAbs( a(ibig,row)) > sAbs( a(maxbig,row)) ) maxbig=ibig;
        }
        if(maxbig!=row) {  // switch rows 
            real t;
            for ( idx c=1; c<=n; ++c) {
                t=a(row,c);
                a(row,c)=a(maxbig,c);
                a(maxbig,c)=t;
            }
            for ( idx c=1; c<=m; ++c) {
                t=b(row,c);
                b(row,c)=b(maxbig,c);
                b(maxbig,c)=t;
            }
        }


//        real coef=1./a(row,row);
        if(sAbs(a(row,row))<=1e-8){
            //a(row,row)=1e-8;
            sMathNRUtil::nrerror("gaussj: Singular Matrix");
            return row;
        }
        real coef=1./a(row,row);
        
        for( idx c=1; c<=n;++c)a(row,c)*=coef;
        for( idx c=1; c<=m;++c)b(row,c)*=coef;


        for( idx i=1; i<=n;++i){
            if(i==row )continue;
            coef=a(i,row);
            for( idx c=1; c<=n;++c)
                a(i,c)= (c==row) ? 0 : (a(i,c)-a(row,c)*coef);
                //a(i,c)= (c==row) ? 0 : (a(i,c)-a(row,c)*coef);
            for( idx c=1; c<=m;++c)
                b(i,c)=b(i,c)-b(row,c)*coef;
        }
        
        
    }
    
    for ( idx i=1; i<=n; ++i ) {
        for ( idx j=1; j<=n; ++j ) {
            a(i,j)= (real)b(i,j);
        }
    }

    return n  ;
}

#define aa(_v_1, _v_2)  (AA[((_v_1)-1)*n+(_v_2)-1])
#define bb(_v_1, _v_2)  (BB[((_v_1)-1)*m+(_v_2)-1])

idx sMathNR::inverse_matrix(real *A, idx n)
{
    idx m=n+1;
    real* B=sMathNRUtil::vector(1,(n+1)*(m+1)+1);

//long double * AA=new long double [n*n];
//long double * BB=new long double [n*m];
            
    for ( idx i=1; i<=n; ++i ) {
        for ( idx j=1; j<=n; ++j ) {
//bb(i,j)= (i==j ? 1. : 0.) ;
            b(i,j)= (i==j ? 1. : 0.) ;
//aa(i,j)=a(i,j);
        }
    }
    for ( idx j=1; j<=n; ++j ) {
        b(j,m)=1;
    }

    idx res=sMathNR_gaussjMi(A, n, B, m);
    //idx res=gaussj(A, n, B, m);
    
    //delete AA;
    //delete BB;
    
    sMathNRUtil::free_vector(B,1,(n+1)*(m+1)+1);
    return res;
}

void sMathNR::inverse_matrix_lud(real *A, idx n)
{
    idx * ind=sMathNRUtil::ivector(1,n);
    real * cols=sMathNRUtil::vector(1,n);
    real ** amat =sMathNRUtil::matrix(1,n,1,n);
    
    // copy yhe matrix 
    for(idx i=1; i<=n; ++i) {for(idx j=1; j<=n; ++j) {amat[i][j]=a(i,j);}}

    real d;
    ludcmp(amat, n, ind, &d);

    for(idx i=0; i<n; ++i) {
        memset(cols,0,sizeof(real)*(n+1));
        cols[i+1]=1;
        lubksb(amat,n,ind,cols);
        for(idx j=0; j<n; ++j) A[j*n+i]=cols[j+1];        
    }

    sMathNRUtil::free_matrix(amat,1,n,1,n);
    sMathNRUtil::free_vector(cols,1,n);
    sMathNRUtil::free_ivector(ind,1,n);
}

#undef SWAP
#undef NRANSI
