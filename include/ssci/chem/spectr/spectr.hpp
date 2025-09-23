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
#ifndef sChem_spectr_hpp
#define sChem_spectr_hpp

#include <slib/core/dic.hpp>
#include <ssci/chem/elements/elements.hpp>

namespace slib {


struct sSpectrMS {
    struct isoDistribution {
        static real absShiftAll;
        sChem::formula frml;
        real frmlShift,rMass;
        idx shiftedFrom;
        sVec < sChem::isotop > spctr;
        sVec < sChem::formula > isoFormulas;
        isoDistribution (void) {frmlShift=0;rMass=0;shiftedFrom=sNotIdx;}
        real getMass(void){
            if(rMass==0)rMass=frml.mass(sChem::formula::eIsotopicAbundant, false);
            return rMass+frmlShift+absShiftAll;
        }
    };


    struct isoDistributionList
    {
        public:

            isoDistributionList()
        : isoTol1(0), isoTol2(0)
        {
        }
        ~isoDistributionList(){
            empty();
        }
        sDic < isoDistribution > isoDistr;
        sVec < real > aveElem;
        real isoTol1;
        real isoTol2;

        idx makeMolList (sStr * moList, idx istart=0, idx iend=-1) ;
        idx readIsoDistributions(const char * src, idx len, real tol1, real tol2, real shift=0);
        idx makeAverageIsoDistribution(real daltons);
        idx makeCloseIsoDistribution(real daltons, isoDistribution * isos=0, idx maxCntToUse=0);
        void shiftIsoDistribution(idx imol,real shift);
        void empty(void){isoDistr.empty(); aveElem.empty();}
    };


} ;




#endif 


}
