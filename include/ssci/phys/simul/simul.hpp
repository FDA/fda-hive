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
#ifndef sPhys_simul_h
#define sPhys_simul_h

#include <slib/core/def.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/sIO.hpp>


namespace slib
{
    class sPhysicalEnsemble  {
        public:
            sMex ensemble;
            idx particleMemory;
            sIO * gLog, * gDbg;

            typedef idx (* SimulOperationActorFunction)(sPhysicalEnsemble * ensemble, void * params);
            struct SimulOperation {
                SimulOperationActorFunction func,state;
                real weight;
                void * params;
            };
            struct OperationCategory {
                sDic < SimulOperation > operations;
                idx loops;
            };
            sDic < OperationCategory > operationCategories;

            OperationCategory * addSimulCategory(const char * categoryName, idx loops=1);
            void addSimulOperation(OperationCategory * categ,const char * operationName, real weight, SimulOperationActorFunction func, SimulOperationActorFunction state);
            void finalizeCategory(OperationCategory * categ);
            SimulOperation * selectWeightedOperationRandomly(OperationCategory * categ);


            idx computeSystemState(void);
            idx adjustSystemState(void);

            idx runSimul(idx cntIters);

    };
}

#endif // sPhys_simul_h


