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
#pragma once
#ifndef sMath_rand_hpp
#define sMath_rand_hpp

#include <slib/core/vec.hpp>

namespace slib
{
    class sRand {
        typedef idx idxlong;
    private:
        static idx randomFirstCall;
    public:
        static real random1(void);
        static real randomGauss(real mean,real sig,real (* randomFunc)(void));

        static real ran0(idx *idum);
        static real ran1(idx *idum);
        static real ran2(idx *idum);
        static real ran3(idx *idum);

        static void normalize(real * v, idx dim);
        static void cumulative(real * v, real * w, idx dim);
        static idx lubIndex(real * v, real q, idx len);
        static real safeLog(real x);

        static real expRand(real lambda);
        static void dirichletRand(real * p, real * rates, idx dim);
        static void dirichletRand(real * p, idx dim);
        static idx categoricalRand(real * cdf, idx dim);

        static inline udx rand_xorshf96(void)
        {
            static udx x=123456789, y=362436069, z=521288629;
            udx long t;
            x ^= x << 16;
            x ^= x >> 5;
            x ^= x << 1;

           t = x;
           x = y;
           y = z;
           z = t ^ x ^ y;

          return z;
        }
    };

    class sRandom {
        private:
            idx idum;
            idx randseed0;
        public:
            sRandom(idx seed = 0){
                setRandSeed(seed);
            }
            void setRandSeed (idx seed0 = 0)
            {
                if (seed0 <= 0){
                    seed0 = time(0);
                }
                idum = randseed0 = seed0;
            }

            idx getRandSeed()
            {
                return randseed0;
            }

            real rand()
            {
                return sRand::ran0(&idum);
            }

            real randRange(real rmin = 0, real rmax = 1.0)
            {
                return rmin + sRand::ran0(&idum) * (rmax - rmin);
            }

            bool flipCoin(real pr = 0.5, idx maxValue = 1.0)
            {
                return randRange(0, maxValue) < pr ? true : false;
            }

    };
}

#endif 





