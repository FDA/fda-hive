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
#ifndef sChem_elements_hpp
#define sChem_elements_hpp

#include <slib/core/def.hpp>
#include <slib/core/vec.hpp>

namespace slib { 

class sStr;

struct sChem{
    public:

        
    struct isotop {
        real rMass;
        real abundance;
        idx iMass, dummy;
        isotop(){}
        ~isotop (){}
        
        bool operator > (isotop &cP)  { return (rMass> cP.rMass);  }   
        bool operator < (isotop &cP)  { return (rMass< cP.rMass);  }   
        bool operator >= (isotop &cP)  { return (rMass>= cP.rMass);  }   
        bool operator <= (isotop &cP)  { return (rMass<= cP.rMass);  }   
    };

    struct element {
        
        idx number;
        char name[32];
        char symbol[4];
        real clrR, clrG, clrB; 
        real radCov;
        real radVDW;
        real atomicMass;
        real meltingT, boilingT;
        
        idx    ofsIsotops;
        idx cntIsotops;

    public:
        const static idx cntElements=120;
    private:
        static char _elementSymbolNum[1<<(2<<3)];
        static element chemTable[cntElements];
    public:
        static idx index(const char * symbol){return _elementSymbolNum[ ( (((int)(symbol[0]))<<8)+((int)symbol[1]) ) ];}
        static element * ptr(idx elem){return chemTable+elem;}
        static idx makeIsotopicElement( idx element, idx isotop){return (element)|(isotop<<16);}

    private:
        static const char * isotopTableRaw;
        
    public:
        static sVec <isotop > isoTable;
        static void initTable( void );
        static void freeTable( void ){isoTable.empty();}

    };



    struct atomcount {
        idx element;
        idx count;
        atomcount(idx lelement=0, idx lcount=0){element=lelement; count=lcount;} 
        ~atomcount(){}
    };


    struct formula : public sVec <atomcount> 
    {
        real prob;
        idx parse(const char * frml);
        idx countAtoms(void) ;
        void printfAtomList(sStr * dst, bool isotopic=false);

        enum eIsotopicMass { eIsotopicExact=1000, eIsotopicAbundant, eIsotopicRare, eIsotopicLightest, eIsotopicHeaviest } ;
        real mass(idx type=eIsotopicAbundant, bool integralonly=false);

        idx isotopicList(formula * dst, idx totIsoStart, idx totIsoEnd);
        idx isotopicDistr(sVec < formula > * dst, real cutoff,real finalcutoff);

        static void printfIsotopicDistribution(sStr * dst,sVec < formula > * peaks, idx start=0, idx finish=-1);

    };


};



#endif 


}
