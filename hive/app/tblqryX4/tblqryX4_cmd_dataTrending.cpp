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

#include <slib/utils.hpp>
#include "tblqryX4_cmd.hpp"
#include "utils.hpp"
#include "sort.hpp"
#include <regex.h>


#define PRFX "dataTrending-"
#define OUTFILE "dataTrending.csv"

using namespace slib;
using namespace slib::tblqryx4;

const char * defaultMissingValue = "null/novalue";

namespace slib {
    namespace tblqryx4 {
        class DataTrendingCommand : public Command
        {
            private:
                idx colLabel, colY, colPeriod;
                idx colGroup;
                sVec <idx> colPeriodVec;
                sVec <idx> colLabelVec;
                sVec <idx> colYVec;
                sVec <idx> colGroupVec;
                bool isDate;
                bool useKeywords;
                bool misMatch;
                bool count;
                bool misValue;
                sStr groupKeywords;
                sStr periodKeywords;
                sStr periodFrom, periodTo;
                sStr colPeriodHdr;
                sStr graphType;
                RowSorter sorter;

            public:
                DataTrendingCommand(ExecContext & ctx) : Command(ctx)
                {
                    colLabel = colY = colPeriod = colGroup = -2;
                    isDate = useKeywords = misMatch = count = misValue =false;
                }

                const char * getName() { return "data-trending"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdDataTrendingFactory(ExecContext & ctx) { return new DataTrendingCommand(ctx); }
    };
};

struct elementInfo {
        sVec < idx > rowIndex;
        sDic < idx > group;
        idx numberOfOccurences;
        elementInfo(){
            numberOfOccurences = 0;
        }
};

static bool periodTextTreatment(sStr & dst, const char * src){
    sStr splitTerm;
    sString::searchAndReplaceStrings(&splitTerm,src,0,"/" _ "-" __,0,0,0);
    if (sString::cnt00(splitTerm)==3){
        sStr cleanQuote;
        const char * firstField = sString::next00(splitTerm,0);
        if (firstField[0] == '"') firstField = firstField +1;
        const char * lastField = sString::next00(splitTerm,2);
        if (lastField[sLen(lastField)-1] == '"') {
            cleanQuote.add(lastField,sLen(lastField)-1);
            cleanQuote.add0();
            lastField = cleanQuote.ptr(0);
        }
        if (strlen(firstField)==4){
            dst.printf("%s",firstField);
            const char * m = sString::next00(splitTerm,1);
            sStr month;
            if (sLen(m)<2) month.printf("0%s",m);
            else month.printf("%s",m);
            dst.printf("-%s",month.ptr());
        }
        else {
            dst.printf("%s",lastField);
            sStr month;
            if (sLen(firstField)<2) month.printf("0%s",firstField);
            else month.printf("%s",firstField);
            dst.printf("-%s",month.ptr(0));
        }
        return 1;
    }

    dst.printf("%s",src);

    return 0;
}

static bool labelTreatment(sStr & bufLabel,const char * bufLabelToClean, idx bufLen) {
    bufLabel.cut(0);
    char * buf = (char *)bufLabelToClean;
    if (buf[0]=='"' && buf[bufLen-1]=='"') {
        buf++; bufLen = bufLen -2;
    }

    for (idx i=0, pos=0; i< bufLen; ++i){
        while (buf[pos] != ',' && buf && i <bufLen){
            pos++; i++;
        }
        if (!pos) { buf++; continue; }
        bufLabel.addString(buf,pos);
        buf = buf +pos+1 ;
        pos =0;
    }

    return 1;
}



static bool mapToKeywords(const char * sentence,const char * keywordsList, bool useKeywords, bool misMatch, sStr & mapTerm){

    sStr splitTerm;
    sString::searchAndReplaceStrings(&splitTerm,keywordsList,0,",",0,0,0);

    bool isNotFound = true;
    for (char * p=splitTerm.ptr(0); p ; p=sString::next00(p)){
        regex_t re;
        idx regerr = regcomp(&re, p, REG_EXTENDED|REG_ICASE) ;

        if (!regerr){
            bool match = (regexec(&re,sentence, 0, NULL, 0)==0) ? true : false;
            if (match){
                if (useKeywords)  {
                    *p = toupper(p[0]);
                    mapTerm.printf(0,"%s",p);
                }
                else {
                    mapTerm.printf(0,"%s",sentence);
                }
                isNotFound = false;
                regfree(&re);
                break;
            }
        }
        regfree(&re);
    }
    if (isNotFound && misMatch){
        mapTerm.printf(0,"Others");
        return 1;
    }
    else if (isNotFound && misMatch==false){
        mapTerm.printf(0,"_Nothing");
        return 0;
    }
    return 1;
}


static idx convertToIdx (const char * src){
    idx a = sIdxMax;
    sStr toSplit, dst;
    sString::searchAndReplaceStrings(&toSplit,src,0,",",0,0,0);
    if (sString::cnt00(toSplit)>0){
        for (const char * p = toSplit.ptr(0); p ; p=sString::next00(p)){
            dst.printf("%s",p);
        }
    }
    else dst.printf("%s",src);

    sscanf(dst.ptr(),"%" DEC "", &a);
    return a;
}

bool DataTrendingCommand::init(const char * op_name, sVariant * arg)
{

    if ( arg->getDicElt("periodAsDate") !=0 )
    {
        isDate = arg->getDicElt("periodAsDate")->asBool();
    }

    if (sVariant * colSetImgVal = arg->getDicElt("period"))
    {
        if (colSetImgVal->isList())
        {
            for (idx i = 0; i < colSetImgVal->dim(); i++){
                colPeriodVec.vadd(1, colSetImgVal->getListElt(i)->asInt());
            }
            if (isDate)
                sorter.init(arg, &_ctx, getName(), true);
        }
        else
            return false;
    }
    if (sVariant * colSetImgVal = arg->getDicElt("labelColumns"))
    {
        if (colSetImgVal->isList())
        {
            for (idx i = 0; i < colSetImgVal->dim(); i++){
                colLabelVec.vadd(1, colSetImgVal->getListElt(i)->asInt());
            }
        }
        else
            return false;
    }
    if (sVariant * colSetImgVal = arg->getDicElt("yColumns"))
       {
           if (colSetImgVal->isList())
           {
               for (idx i = 0; i < colSetImgVal->dim(); i++){
                   colYVec.vadd(1, colSetImgVal->getListElt(i)->asInt());
               }
           }
           else
               return false;
       }


    if (sVariant * colSetImgVal = arg->getDicElt("groupBy"))
          {
              if (colSetImgVal->isList())
              {
                  for (idx i = 0; i < colSetImgVal->dim(); i++){
                      colGroupVec.vadd(1, colSetImgVal->getListElt(i)->asInt());
                  }
              }
              else
                  return false;
     }


    if (arg->getDicElt("periodKeywords") != 0){
        periodKeywords.printf (0,"%s", arg->getDicElt("periodKeywords")->asString());
    }

    if (arg->getDicElt("groupKeywords") != 0){
        groupKeywords.printf (0,"%s", arg->getDicElt("groupKeywords")->asString());
    }

    if (arg->getDicElt("periodFrom") != 0){
     }
    if (arg->getDicElt("periodTo") != 0){
    }

    if (arg->getDicElt("mismatch") != 0){
            misMatch = arg->getDicElt("mismatch")->asBool();
    }

    if (arg->getDicElt("missingValue") != 0){
            misValue = arg->getDicElt("missingValue")->asBool();
    }

    if (arg->getDicElt("searchOpt") != 0){
        useKeywords =  arg->getDicElt("searchOpt")->asBool();
    }

    if (arg->getDicElt("graphType") != 0){
        graphType.printf("%s", arg->getDicElt("graphType")->asString());
        if (strcmp(graphType.ptr(),"pie")==0) misMatch = true;
    }


    return true;
}


bool DataTrendingCommand::compute(sTabular * tbl)
{
    if (colPeriodVec.dim()){
        colPeriod = colPeriodVec[colPeriodVec.dim()-1];
        tbl->printCell(colPeriodHdr,-1,colPeriod,0);
    }

    if (colLabelVec.dim())
        colLabel = colLabelVec[colLabelVec.dim()-1];

    if (colYVec.dim())
        colY = colYVec[colYVec.dim()-1];

    if (colGroupVec.dim())
        colGroup = colGroupVec[colGroupVec.dim()-1];



    sDic < elementInfo > dicOfElements;

    if (colPeriod !=-2){
        sReorderedTabular reordered (tbl, false);
        if (isDate){
            sorter.sort(&reordered);
        }


        sStr period, dst;
        for (idx irow=0; irow< tbl->rows(); irow++){
            period.cut(0); dst.cut(0);
            reordered.printCell(period,irow,colPeriod,0);

            if (!period.length()){
                if (isDate && misValue){
                    period.printf(0,"0000-00-00");
                    periodTextTreatment(dst,period.ptr());
                }
                else if (misValue){
                    dst.addString(defaultMissingValue, sLen(defaultMissingValue) );
                }
                else continue;
            }
            else {
                if (isDate) periodTextTreatment(dst,period.ptr());
                else dst.printf("%s",period.ptr(0));
            }
            if (isDate==false &&periodKeywords.length() && strcmp(dst.ptr(),defaultMissingValue)!=0){
                sStr periodResult; bool isMapped = false;
                if (useKeywords) {
                    isMapped = mapToKeywords(dst.ptr(0),periodKeywords.ptr(), true, misMatch, periodResult);
                }
                else {
                    isMapped = mapToKeywords(dst.ptr(0),periodKeywords.ptr(),false,misMatch,periodResult);
                }
                if (isMapped) {
                    dst.printf(0,"%s",periodResult.ptr(0));
                }
                else continue;

            }
            dicOfElements.set(dst.ptr())->rowIndex.vadd(1,irow);
        }

        sStr bufLabel, bufLabelToClean;
        sStr bufY;
        for (idx iE=0; iE < dicOfElements.dim(); ++iE){
            const char * key = (const char *)dicOfElements.id(iE);
            elementInfo * rowElement = dicOfElements.get(key);

            for (idx ir=0; ir<rowElement->rowIndex.dim();++ir){
                bufLabel.cut(0); bufLabelToClean.cut(0); bufY.cut(0);
                idx valY=sIdxMax;
                idx realRow = rowElement->rowIndex.ptr(ir)[0];
                idx columnToRead = -2;
                if (colGroup!=-2){
                    columnToRead = colGroup;
                }
                else columnToRead = colPeriod;
                reordered.printCell(bufLabelToClean,realRow,columnToRead,0);
                if (!bufLabelToClean.length() && misValue){
                    bufLabelToClean.cut(0);
                    bufLabelToClean.addString(defaultMissingValue,sLen(defaultMissingValue));
                }
                else if (!bufLabelToClean.length()) continue;

                labelTreatment(bufLabel,bufLabelToClean.ptr(0), bufLabelToClean.length());

                if (colY!=-2){
                    reordered.printCell(bufY,realRow,colY,0,"");
                    valY = convertToIdx(bufY.ptr(0));

                }
                if (colGroup!=-2 && groupKeywords.length() && strcmp(bufLabel.ptr(),defaultMissingValue)!=0){
                    sStr mapResult; bool isMapped = false;
                    if (useKeywords) {
                        isMapped = mapToKeywords(bufLabel.ptr(0),groupKeywords.ptr(), true, misMatch, mapResult);
                    }
                    else {
                        isMapped = mapToKeywords(bufLabel.ptr(0),groupKeywords.ptr(),false,misMatch,mapResult);
                    }
                    if (isMapped) {
                        bufLabel.printf(0,"%s",mapResult.ptr(0));
                    }
                    else continue;

                }
                dicOfElements.ptr(iE)->numberOfOccurences +=1;
                if (colGroup ==-2) {
                    bufLabel.cut(0);
                    bufLabel.addString(colPeriodHdr.ptr(),colPeriodHdr.length());
                }
                if (dicOfElements.ptr(iE)->group.find(bufLabel.ptr(0),bufLabel.length())){
                    idx * value = dicOfElements.ptr(iE)->group.get(bufLabel.ptr(0),bufLabel.length());
                    if (valY == sIdxMax) {
                        if (colGroup != -2){
                            *value = *value +1;
                        } else {
                            *value = dicOfElements.ptr(iE)->numberOfOccurences;
                        }
                    }
                    else *value += valY;
                }
                else {
                    if (valY == sIdxMax) {
                        if (colGroup != -2){
                            *dicOfElements.ptr(iE)->group.set(bufLabel.ptr(0),bufLabel.length()) = 1;
                        } else {
                            *dicOfElements.ptr(iE)->group.set(bufLabel.ptr(0),bufLabel.length()) = dicOfElements.ptr(iE)->numberOfOccurences;
                        }
                    }
                    else *dicOfElements.ptr(iE)->group.set(bufLabel.ptr(0),bufLabel.length()) = valY;
                }
            }
        }



    }


    sStr outPutPath;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" OUTFILE,0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(),OUTFILE,&outPutPath);
    sFile::remove(outPutPath);

    sFil dataTrendingCSV(outPutPath);


    sDic < idx > headerDic;
    idx len = 0;
    for (idx iElement=0; iElement < dicOfElements.dim(); ++iElement){
        for (idx iHdr=0; iHdr < dicOfElements.ptr(iElement)->group.dim(); ++iHdr){
            const char * header = (const char *)(dicOfElements.ptr(iElement))->group.id(iHdr,&len);
            if (!headerDic.find(header, len)) {
                 *headerDic.set(header,len) = 1;
             }
        }
    }

    dataTrendingCSV.addString(colPeriodHdr.ptr(),colPeriodHdr.length());


    for (idx iHdr=0; iHdr < headerDic.dim(); ++iHdr){
        const char * header = (const char *)headerDic.id(iHdr,&len);
        if (headerDic.dim()==1 && strncmp(header,colPeriodHdr.ptr(),len)==0){
            dataTrendingCSV.addString(",",1);
            dataTrendingCSV.printf("Number of Events");
        }
        else {
            dataTrendingCSV.addString(",",1);
            dataTrendingCSV.addString(header,len);
        }
    }
    if (!headerDic.dim()) {
        dataTrendingCSV.addString(",",1);
        if (groupKeywords.length()){
            dataTrendingCSV.addString(groupKeywords.ptr(0),groupKeywords.length());
        }
        else if (colGroup!=-2){
            sStr headerReplacement;
            tbl->printCell(headerReplacement,-1,colGroup,0);
            dataTrendingCSV.addString(headerReplacement.ptr(0),headerReplacement.length());
        }
        else {
            dataTrendingCSV.addString("Number of Events",16);
        }

    }
    dataTrendingCSV.addString("\n",1);

    for (idx iElement=0; iElement < dicOfElements.dim(); ++iElement){
        const char * rowElement = (const char *)dicOfElements.id(iElement,&len);
        dataTrendingCSV.addString(rowElement,len);
        dataTrendingCSV.addString(",",1);
        for (idx iHdr=0; iHdr < headerDic.dim(); ++iHdr){
            if (iHdr) dataTrendingCSV.addString(",",1);
            const char * header = (const char *)headerDic.id(iHdr,&len);
            idx * value;
            if (dicOfElements.ptr(iElement)->group.find(header,len)){
             value= dicOfElements.ptr(iElement)->group.get(header,len);
             dataTrendingCSV.printf("%" DEC "",*value);
            }
            else {
                dataTrendingCSV.printf("0");
            }

        }
        if (!headerDic.dim()){
            dataTrendingCSV.printf("0");
        }
        dataTrendingCSV.addString("\n",1);
    }
    return true;
}

