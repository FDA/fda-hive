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
#ifndef sLib_vax_bio_hpp
#define sLib_vax_bio_hpp

#include <ion/vax.hpp>
#include <ssci/bio/ion-bio.hpp>

class sVaxSeq: public sVax
{
        sVaxSeq(sIon * ion){
            ((sIonAnnot*)ion)->constructSchema();
        }
        idx ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize);
};



class sVaxAnnot : public sVax
{
    public:
    idx seqIDCol, startCol, endCol;
    bool skip0,needsCleanup;
    bool _isVCF;
    bool _isGTF;
    bool cleanHeaderForVCF;
    protected:
        sIonAnnot::sIonPos pos;

    public:

        sVaxAnnot(sIon * ion, idx seqID=0, idx start=0, idx end=0, idx * columnMap=0 ){
            seqIDCol=seqID;
            startCol=start;
            endCol=end;
            internalColumnMap=columnMap;
            skip0=true;
            needsCleanup=false;
            _isVCF=false;
            _isGTF=false;
            cleanHeaderForVCF=true;

            ((sIonAnnot*)ion)->constructSchema();
        }
        idx ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize);

        static idx rangeSorter(sIon * ion, void * param, sIon::RecordResult * targets1, sIon::RecordResult * targets2);


};


class sVaxAnnotGB : public sVaxAnnot
{
    idx ofsetsOfFieldTypes[4];
    idx pointersOfFieldTypes[4];
    bool fromFeature;
    bool fromHeader;
    bool skipToNextLocus;
    bool pleaseAddFeature;
    sStr reserveBuf;
    sStr curFeatureToAdd;
    const char * strandWord;
    idx headerMatchNumber;
    int defaultLength;

    public:
        idx recordNum; // record number incrementing based on relation number
        idx previousRecordNum;
    protected:
        struct startEnd {
                idx start, end, group, max;
                bool exactStart, exactEnd, oneBaseBetween, oneSiteBetween, forward;
                sStr buf;
                startEnd (){
                    start = end = group = max = 0;
                    buf.cut(0);
                    forward = exactStart= exactEnd = true;
                    oneBaseBetween = false; // 102.110
                    oneSiteBetween = false;  // 123^124
                }
        };
    public:
         bool continuousRanges;
         bool forward, pleaseAddStrand;
         sStr rangeListForJoinAndComplement;

    public:
    sVaxAnnotGB ( sIon * ion ) : sVaxAnnot ( ion )
    {
        fromFeature = fromHeader = false;
        recordNum =0; // use as the ligne number
        previousRecordNum =0;
        needsCleanup=true;
        skipToNextLocus=false;
        pleaseAddFeature=pleaseAddStrand=false;
        continuousRanges =false;
        forward=true;
        strandWord="strand";
        headerMatchNumber=0;
        defaultLength = 0xFFFFFFFF;
        for(idx i=0; i<sDim(ofsetsOfFieldTypes);++i)
            ofsetsOfFieldTypes[i]=sNotIdx;
    }

    idx ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize);

    private:
        void scanUntilNextLine (sStr & dest, const char * body, idx * recordSize, const char * srcEnd);
        idx strchrFreq (const char * input, const char * srch, idx & frq);
        char * extractContent (const char * input, sStr & dst, const char * startMarkup, const char * endMarkup);
        void extractLocation(const char * location, startEnd & locationExtracted, bool forward);
        void parseJoinOrOrder(const char * textRaw, sVec < startEnd > & startEndOut, const char * toCompare="join("); // toCompare either: "join(" or "order("
        void parseComplement(const char * textRaw, sVec < startEnd > & startEndOut);
};

#endif

