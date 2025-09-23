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
#ifndef sLib_ion_bio_hpp
#define sLib_ion_bio_hpp

#include <ion/vax.hpp>
#include <ion/sIon.hpp>



class sIonAnnot : public sIon
{
    public:
        sIonAnnot(const char * baseName=0, idx lopenMode=0) : sIon(baseName,lopenMode){
            if(!(lopenMode&sMex::fReadonly))
                constructSchema();
        }
        enum eRecordTypes{ eSeqID=0, ePos,eType,eID,eRecord,eMax };
        void constructSchema(void){
             const char * controlFile=
                    "record,seqID,string,string\n"                    "record,pos,irange,idx\n"
                    "record,type,string,string\n"
                    "record,id,string,string\n"
                    "record,record,idx,idx\n"
                    "relation,annot,seqID|pos|record|type|id,record,seqID|record,seqID|record|type,seqID|record|id,seqID,seqID|pos,seqID|type,seqID|type|id,id|type,id,type\n";
                constructRecordAndRelationTypes(controlFile, sLen(controlFile));

        }

        idx indexArr[eMax];
        union sIonPos {
            struct {int end,start;} s32;
            idx s64;
        };

};

class sIonExpression: public sIon
{
    public:
        sIonExpression(const char * baseName=0, idx lopenMode=0) : sIon(baseName,lopenMode){
            if(!(lopenMode&sMex::fReadonly)){
                constructSchema();
            }

        }
        enum eRecordTypes{ eRow=0, eID,eType,eValue,ePvalue,eSample,ePassage,eExperiment,eMax };
        enum eRelationTypes{ eRelID=0, eRelVal,eRelExperiment,eRelMax };
        sTxtTbl * myTable;
        sStr errorMsg;
        void constructSchema(void){
             const char * controlFile=
                    "record,record,idx,idx\n"
                    "record,id,string,string\n"
                    "record,type,string,string\n"
                    "record,value,string,string\n"
                    "record,pvalue,string,string\n"
                    "record,sample,string,string\n"
                    "record,passage,string,string\n"
                    "record,experiment,string,string\n"
                    "relation,relid,id|type|record,id,id|type,record|id|type,record\n"
                    "relation,relsample,record|sample|passage|value|pvalue,record,record|sample,sample,sample|passage,passage,sample\n"
                    "relation,relexperiment,record|experiment,record,record|experiment,experiment\n";
                constructRecordAndRelationTypes(controlFile, sLen(controlFile));

        }
        idx indexArr[eMax];
        idx parseConventionalExpression(const char * expressionFile,const char * experiment);
        idx checkFileType();
        idx parseInVitro(const char * experiment);
        idx parseOmics(const char * experiment);

};


#endif
