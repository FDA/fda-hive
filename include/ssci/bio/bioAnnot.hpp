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
#ifndef sLib_bioAnnot_hpp
#define sLib_bioAnnot_hpp

#include <slib/core/def.hpp>

class sBioAnnot
{

    public:
        sBioAnnot ()
        {
        }

        virtual ~sBioAnnot ()
        {
        }


        bool isok(void){return true;}
        virtual idx dimAnnot(void){return 0;}


    protected:
        class iterator {
            idx iBioAnnotNum;
            idx iIndexInBioAnnot;
            void reset (){
                iBioAnnotNum = 0;
                iIndexInBioAnnot=0;
            }

            iterator (){reset();}
        };
        idx getIDByAnotherID(iterator & it, const char * IdtosearchFor , const char * idtype)
        {
            return 0;
        }



};

class sBioAnnotSet : public sBioAnnot
{
        // each record defines a multiple or a single sequences in the container
        struct RefAnnot {
            idx bioNum; // the serial number of sBioal in the biosR this alignment originates from
        };

        sVec < sBioAnnot * > biosR; // managed sBioal objects;
        sVec < RefAnnot > refs; // list of references from this bioseq

        sVec < idx > alInd;
        idx totDim; // total dimension

    public:

        sBioAnnotSet()         {
            totDim=0;

        }

        void attach(sBioAnnot * bioannot,
                idx alNum=0,
                idx alCnt=sIdxMax);

        sBioAnnot * ref(idx * inum, idx iSub=sNotIdx); // returns the sBioal * object and its offset

   public:
        virtual idx dimAnnot(void){return totDim;}


};

#endif
