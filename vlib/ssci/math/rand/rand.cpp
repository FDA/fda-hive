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
#include <ssci/math/rand/rand.hpp>
#include <math.h>

using namespace slib;

idx sRand::randomFirstCall=1;
real sRand::random1(void)
{
    
    
    static  idx   IJ=18734,KL=23869;
    static  real   randomU[98];
    static  real   randomC,randomCD,randomCM;
    static  idx   randomI97,randomJ97;
    idx   i,j,k,l,ii,jj,m;
    real   uni,s,t;    

    if(randomFirstCall==1){
        i = ((IJ/177)%177) + 2;
        j = (IJ%177) + 2;
        k = ((KL/169)%178) + 1;
        l = (KL%169);

        for(ii = 1;ii<=97;ii++){
            s = 0.0;
            t = 0.5;
            for(jj=1;jj<24;jj++){
                m = ( (((i*j)%179)*k) % 179);
                i = j;
                j = k;
                k = m;
                l = ((53*l+1)%169);
                if(((l*m)%64)>=32){
                   s = s + t;
                }
                t = 0.5 * t;
            }
            randomU[ii] = s;
        }
        randomC = 362436.0 / 16777216.0;
        randomCD = 7654321.0 / 16777216.0;
        randomCM = 16777213.0 /16777216.0;
        randomI97 = 97;
        randomJ97 = 33;

        randomFirstCall=0;
    }

    uni = randomU[randomI97] - randomU[randomJ97];
    if( uni < 0.0 ) uni = uni + 1.0;
    randomU[randomI97] = uni;
    randomI97 = randomI97 - 1;if(randomI97 == 0) randomI97 = 97;
    randomJ97 = randomJ97 - 1;if(randomJ97 == 0) randomJ97 = 97;
    randomC = randomC - randomCD;if( randomC < 0.0 ) randomC = randomC + randomCM;
    uni = uni - randomC;
    if( uni < 0.0 ) uni = uni + 1.0;
    if( uni >= 1.0 ) uni = uni - 1.0;

    return uni;
}



real sRand::randomGauss(real mean,real sig,real (* randomFunc)(void))
{
    real   v1=0,v2,r;

    r=2.;
    while(r>1.){
        v1=2*randomFunc()-1.;
        v2=2*randomFunc()-1;
        r=v1*v1+v2*v2;
    }
    return mean+v1*sig*sqrt(-2.*log(r)/r);
}


















#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876
real sRand::ran0(sRand::idxlong *idum) 
{
    sRand::idxlong k;
    real ans;
    *idum ^= MASK;
    k=(*idum)/IQ;
    *idum=IA*(*idum-k*IQ)-IR*k;
    if(*idum < 0) *idum += IM;
    ans=AM*(*idum);
    *idum ^= MASK;
    return ans;
}


#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)
real sRand::ran1(sRand::idxlong *idum)
{
    idx j;
    sRand::idxlong k;
    static sRand::idxlong iy=0;
    static sRand::idxlong iv[NTAB];
    real temp;

    if (*idum <= 0 || !iy) {
        if (-(*idum) < 1) *idum=1;
        else *idum = -(*idum);
        for (j=NTAB+7;j>=0;j--) {
            k=(*idum)/IQ;
            *idum=IA*(*idum-k*IQ)-IR*k;
            if (*idum < 0) *idum += IM;
            if (j < NTAB) iv[j] = *idum;
        }
        iy=iv[0];
    }
    
    k=(*idum)/IQ;
    *idum=IA*(*idum-k*IQ)-IR*k;
    if(*idum < 0) *idum += IM;
    j=iy/NDIV;
    iy=iv[j];
    iv[j] = *idum;
    if ((temp=AM*iy) > RNMX) return RNMX;
    else return temp;
}




#define IM1 2147483563
#define IM2 2147483399
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#undef NDIV
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

real sRand::ran2(sRand::idxlong *idum)
{
    idx j;
    sRand::idxlong k;
    static sRand::idxlong idum2=123456789;
    static sRand::idxlong iy=0;
    static sRand::idxlong iv[NTAB];
    real temp;

    if (*idum <= 0) {
        if (-(*idum) < 1) *idum=1;
        else *idum = -(*idum);
        idum2=(*idum);
        for (j=NTAB+7;j>=0;j--) {
            k=(*idum)/IQ1;
            *idum=IA1*(*idum-k*IQ1)-k*IR1;
            if (*idum < 0) *idum += IM1;
            if (j < NTAB) iv[j] = *idum;
        }
        iy=iv[0];
    }
    
    k=(*idum)/IQ1;
    *idum=IA1*(*idum-k*IQ1)-k*IR1;
    if (*idum < 0) *idum += IM1;
    k=idum2/IQ2;
    idum2=IA2*(idum2-k*IQ2)-k*IR2;
    if (idum2 < 0) idum2 += IM2;
    j=iy/NDIV;
    iy=iv[j]-idum2;
    iv[j] = *idum;
    if (iy < 1) iy += IMM1;
    if ((temp=AM*iy) > RNMX) return RNMX;
    else return temp;
}



#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)
real sRand::ran3(sRand::idxlong *idum)
{
    static idx inext,inextp;
    static sRand::idxlong ma[56];
    static idx iff=0;
    sRand::idxlong mj,mk;
    idx i,ii,k;
    if (*idum < 0 || iff == 0) {
    iff=1;
    mj=MSEED-(*idum < 0 ? -*idum : *idum);
    mj %= MBIG;
    ma[55]=mj;
    mk=1;
    
    for (i=1;i<=54;i++) {
        ii=(21*i) % 55;
        ma[ii]=mk;
        mk=mj-mk;
        if (mk < MZ) mk += MBIG;
        mj=ma[ii];
    }

    for (k=1;k<=4;k++)
        for(i=1;i<=55;i++) { 
            ma[i] -= ma[1+(i+30) % 55];
            if (ma[i] < MZ) ma[i] += MBIG;
        }
    inext=0;
    inextp=31;
    *idum=1;
    }

    if (++inext == 56) inext=1;
    if (++inextp == 56) inextp=1;
    mj=ma[inext]-ma[inextp];
    if (mj < MZ) mj += MBIG;
    ma[inext]=mj;
    return mj*FAC;
}


        void sRand::normalize(real * v, idx dim) {
            real sum = 0.0;
            for (idx i=0; i<dim; i++) {
                sum += ((real) v[i]);
            }

            if (sum > 1e-12) {
                for (idx i=0; i<dim; i++) {
                    v[i] = v[i] / sum;
                }
            } else {
                for (idx i=0; i<dim; i++) {
                    v[i] = 1.0 / ((double) dim);
                }
            }
        }

        void sRand::cumulative(real * v, real * w, idx dim) {
            v[0] = w[0];
            for (idx i=1; i<dim; i++) {
                v[i] = w[i]+v[i-1];
            }
        }

        idx sRand::lubIndex(real * v, real q, idx len) {

            idx index;

            if (q <= v[0]) {
                index = 0;
            } else if (q > v[len-1]) {
                index = len-1;
            } else {
                idx L = 0;
                idx U = len-1;
                while (U-L > 1) {
                    idx M = (L+U) >> 1;
                    if (q <= v[M]) U = M;
                    else L = M;
                }
                index = U;
            }

            return index;
        }

        real sRand::safeLog(real x) {
            if (x > 1e-12) {
                return log(x);
            } else {
                return log(1e-12);
            }
        }



real sRand::expRand(real lambda) {
    return -log(1-random1())/lambda;
}

void sRand::dirichletRand(real * p, real * rates, idx dim) {
    for (int i=0; i<dim; i++) {
        p[i] = expRand(rates[i]);
    }
    normalize(p,dim);
}

void sRand::dirichletRand(real * p, idx dim) {
    real * r = new real[dim];
    for (int i=0; i<dim; i++) {
        r[i] = 1.0;
    }
    dirichletRand(p,r,dim);
}


idx sRand::categoricalRand(real * cdf, idx dim) {
    return lubIndex(cdf,random1(),dim);
}
