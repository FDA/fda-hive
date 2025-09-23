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
#include <ssci/math/nr/nrutil.h>

using namespace slib;

#define NR_END 1
#define FREE_ARG char*

char sMathNRUtil::lastErr[4096];

void sMathNRUtil::nrerror(const char * error_text)
{
    strncpy(lastErr,error_text,sizeof(lastErr)-1);
}


real* sMathNRUtil::vector(idx nl, idx nh)
{
        real*v;

        v=(real*)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(real)));
        if (!v) sMathNRUtil::nrerror("allocation failure in vector()");
        return v-nl+NR_END;
}

void sMathNRUtil::free_vector(real *v, idx nl, idx )
{
        free((FREE_ARG) (v+nl-NR_END));
}


idx *sMathNRUtil::ivector(idx nl, idx nh)
{
        idx *v;

        v=(idx *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(idx)));
        if (!v) sMathNRUtil::nrerror("allocation failure in ivector()");
        return v-nl+NR_END;
}
void sMathNRUtil::free_ivector(idx *v, idx nl, idx )
{
        free((FREE_ARG) (v+nl-NR_END));
}




real ** sMathNRUtil::matrix(idx nrl, idx nrh, idx ncl, idx nch)
{
        idx i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
        real **m;

        m=(real **) malloc((size_t)((nrow+NR_END)*sizeof(real*)));
        if (!m) sMathNRUtil::nrerror("allocation failure 1 in matrix()");
        m += NR_END;
        m -= nrl;

        m[nrl]=(real *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(real)));
        if (!m[nrl]) sMathNRUtil::nrerror("allocation failure 2 in matrix()");
        m[nrl] += NR_END;
        m[nrl] -= ncl;

        for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

        return m;
}

void sMathNRUtil::free_matrix(real **m, idx nrl, idx , idx ncl, idx )
{
        free((FREE_ARG) (m[nrl]+ncl-NR_END));
        free((FREE_ARG) (m+nrl-NR_END));
}
