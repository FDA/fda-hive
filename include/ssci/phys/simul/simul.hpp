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

            enum eSignal {eContinue,eGlobalError};

        public:

            class OperationCategory {
                friend class sPhysicalEnsemble;

                class Operation {
                    SimulOperationActorFunction func,state;
                    real weight;
                    void * params;
                    eSignal signal;
                    eSignal runIteration(void) {
                        signal=eContinue;
                        if(state)state( 0, params );
                        if(func && signal==eContinue)func( 0, params );
                        return signal;
                    }
                    friend class OperationCategory;
                };
                sDic < Operation > operations;
                idx loops;
                real weight;

                void addOperation(const char * operationName, real weight, SimulOperationActorFunction func, SimulOperationActorFunction state)
                {
                    Operation * oper=operations.set(operationName);
                    oper->func=func;
                    oper->state=state;
                    oper->weight=weight;
                }
                void frameOperations(void);
                Operation * selectOperation(real randNum);

            };
            sDic < OperationCategory > operationCategories;

            OperationCategory * addCategory(const char * categoryName, idx loops=1)
            {
                OperationCategory * ocat=operationCategories.set(categoryName);
                ocat->loops=loops;
                return ocat;
            }
            void frameCategories(void);






            idx computeSystemState(void);
            idx adjustSystemState(void);

            idx runSimul(idx cntIters);

    };
}

#endif 

