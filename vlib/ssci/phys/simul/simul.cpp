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
#include <math.h>
#include <ssci/math/rand/rand.hpp>
#include <ssci/phys/simul/simul.hpp>
using namespace slib;


/*_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ Adding new type of operations
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/*/

sPhysicalEnsemble::OperationCategory * sPhysicalEnsemble::addSimulCategory(const char * categoryName, idx loops)
{
    OperationCategory * ocat=operationCategories.set(categoryName);
    ocat->loops=loops;
    return ocat;
}


void sPhysicalEnsemble::addSimulOperation(sPhysicalEnsemble::OperationCategory * categ,const char * operationName, real weight, SimulOperationActorFunction func, SimulOperationActorFunction state)
{
    SimulOperation * mol=categ->operations.set(operationName);
    mol->func=func;
    mol->state=state;
    mol->weight=weight;
}

void sPhysicalEnsemble::finalizeCategory(sPhysicalEnsemble::OperationCategory * categ)
{
    SimulOperation * mol;
    real totweight=0;
    idx im, cntM=categ->operations.dim();
    for( im=0; im<cntM ; ++im ) {
        mol=categ->operations.ptr(im);
        totweight+=mol->weight;
    }
    if(totweight) {
        for( im=0; im<cntM ; ++im ) {
            mol=categ->operations.ptr(im);
            mol->weight/=totweight;
        }
    }
}

sPhysicalEnsemble::SimulOperation * sPhysicalEnsemble::selectWeightedOperationRandomly(OperationCategory * categ)
{
    idx cnt=categ->operations.dim();if(!cnt)return 0;
    idx i=0;
    real r=sRand::random1() , totweight=0;
    SimulOperation * mol;
    while(i<cnt) {
        mol=categ->operations.ptr(i);
        totweight+=mol->weight;
        if(r<totweight)
            return mol;
        ++i;
    }
    return 0;
}



/*_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ Adding new type of operations
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/*/


idx sPhysicalEnsemble::computeSystemState(void)
{

    return 0;
}

idx sPhysicalEnsemble::adjustSystemState(void)
{

    return 0;
}


idx sPhysicalEnsemble::runSimul(idx cntIters)
{
    idx cntMD, cntMC;

    SimulOperation * oper;
    /*_/_/_/_/_/_/_/_/_/_/_/_/
    _/
    _/ Iterations
    _/
    _/_/_/_/_/_/_/_/_/_/_/_/*/

    for( idx iter=0 ; iter<cntIters; ++iter ) {


        /*_/_/_/_/_/_/_/_/_/_/_/_/
        _/
        _/ Temperature selection
        _/
        _/_/_/_/_/_/_/_/_/_/_/_/*/


        /*_/_/_/_/_/_/_/_/_/_/_/_/
        _/
        _/ System State
        _/
        _/_/_/_/_/_/_/_/_/_/_/_/*/

        //adjustSystemState(void);

        /*_/_/_/_/_/_/_/_/_/_/_/_/
        _/
        _/ Operation steps
        _/
        _/_/_/_/_/_/_/_/_/_/_/_/*/

        for( idx icat=0  ; icat<operationCategories.dim(); ++icat  ) {

            OperationCategory * ocat=operationCategories.ptr();
            if( !ocat->loops || ocat->operations.dim() ) continue; // no operations in this category or no loops for this operation category ?

            for ( idx iop=0 ; iop<cntMD ; ++iop ) {

                oper=selectWeightedOperationRandomly(ocat);
                if(!oper)continue;

                /*_/_/_/_/_/_/_/_/_/_/_/_/
                _/
                _/ particular operation selection in this category
                _/
                _/_/_/_/_/_/_/_/_/_/_/_/*/

                if(oper->state)
                    oper->state( this, oper->params );
                if(oper->func)
                    oper->func( this, oper->params );
            }
        }

    }

    return cntIters;
}

