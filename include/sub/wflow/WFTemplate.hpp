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
#ifndef sLib_wftemplate_h
#define sLib_wftemplate_h

#include <slib/std.hpp>
#include <ulib/ulib.hpp>

namespace slib {

    class WFelement : public sVar
    {
        public:
            idx indexInTemplate;
            idx status;
    };



    class WFtemplate
    {
        idx autoName;

        public:
            ~WFtemplate(){autoName=1;}
            sStr title;

            sDic < sDic < WFelement > > elements;
            sDic < sDic < sDic < idx > > > groups;


            WFelement * add(const char * elementType, const char * role, idx size=0, idx * pNum=0, const char * command=0, idx len=0)
            {
                if(!role){
                    role=(const char *)&autoName;
                    size=sizeof(autoName);
                    ++autoName;
                }
                WFelement * el=elements[elementType].set(role,size,pNum);

                if(command) {
                    sStr dst;
                    sString::searchAndReplaceSymbols(&dst , command, len , "="sString_symbolsEndline , 0, 0, true, true, true, true);
                    for( const char* p=dst.ptr(); p ; p=sString::next00(p) ){
                        const char * val=sString::next00(p); if(!val)break;
                        (*el)[p]=val;
                    }
                }
                return el;
            }
            WFelement & add(const char * elementType, const char * role, const char * command, idx * pNum=0)
            {
                return *add(elementType, role, 0, pNum, command);
            }

            WFelement * get(const char * elementType, const char * role, idx size=0, idx * pNum=0)
            {
                return elements[elementType].get(role,size,pNum);
            }

            idx getIndex(const char * elementType,const char * role)
            {
                idx num;
                if( elements[elementType].find(role,&num) )  {
                    return num;
                }
                return -1;
            }

            void printSankey(sStr * out)
            {
                out->printf("{\n");
                sDic < WFelement > * els=elements["node"];
                if(els && els->dim() ) {
                    out->printf("\"nodes\":[\n");
                    for(idx iel=0 ; iel<els->dim() ; ++iel){
                        const char * elName=(const char * )els->id(iel);
                        out->printf();
                    }

                }


                out->printf("]\n");
                out->printf(",\n");
                out->printf("\"links\":[\n");
                out->printf("]\n");
                out->printf("}\n");


            }

            }


    };


    class WFinstance: public WFtemplate
    {

            enum eStatus {
                eStatus_Waiting,
                eStatus_Processing,
                eStatus_Running,
                eStatus_Suspended,
                eStatus_Done,
                eStatus_Killed,
                eStatus_ProgramError,
                eStatus_SystemError,
                eStatus_Last,
                eStatus_ErrorNotInSystemButInProgrammersGeneticCode

            };
            WFtemplate * tmplt, myTmplt;

        public:
            WFinstance(WFtemplate *  ltmplt=0){
                tmplt=ltmplt ? ltmplt :  &myTmplt;
            }

       public:
            idx evaluate(WFelement & el, const char * evaluator, const char * parameters );
            idx instanteniateReadyElements(const char * elementType);
            idx executeReadyNodes(const char * elementType);

    };

#endif 


bool finalNode;
