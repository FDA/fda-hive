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
                idx colGroup;       // In case users want to group elements with another criteria
                sVec <idx> colPeriodVec;
                sVec <idx> colLabelVec;
                sVec <idx> colYVec;
                sVec <idx> colGroupVec;
                bool isDate;
                bool useKeywords;   // when users search for terms, flag for displaying just the keywords or the whole word
                bool misMatch;      // flag for interpretation of mis match for search cases
                bool count;      // flag for just count number of elements
                bool misValue;      // flag for just count number of elements
                sStr groupKeywords;     // keywords list in comma seperated format
                sStr periodKeywords;
                sStr periodFrom, periodTo;
                sStr colPeriodHdr;
                sStr graphType;     // line, pie or  column
                RowSorter sorter;   // used for sorting date

            public:
                DataTrendingCommand(ExecContext & ctx) : Command(ctx)
                {
                    colLabel = colY = colPeriod = colGroup = -2; // initialize at -2 because -1 is used for attribute column in TxtTbl class
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
    // sString::searchAndReplaceSymbols(&splitTerm,src,0,"/"_"-"__,0,0,true,false,false,true);
    sString::searchAndReplaceStrings(&splitTerm,src,0,"/"_"-"__,0,0,0);
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
        return 1; // "Month-Year"
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


static bool mapToKeywords(sDic <idx> * dict, const char * sentence,const char * keywordsList, idx valueY, bool useKeywords, bool misMatch, sStr & mapTerm){

    sStr splitTerm;
    sString::searchAndReplaceStrings(&splitTerm,keywordsList,0,",",0,0,0);

    idx valY = 1;
    if (valueY!=sIdxMax){
        valY = valueY;
    }
    bool isNotFound = true;
    for (const char * p=splitTerm.ptr(0); p ; p=sString::next00(p)){
        regex_t re;
        idx regerr = regcomp(&re, p, REG_EXTENDED|REG_ICASE) ; // set up key for regex

        if (!regerr){
            bool match = (regexec(&re,sentence, 0, NULL, 0)==0) ? true : false; // matching process
            if (match){
                idx  * value;
                if (useKeywords)  {
                    value= dict->set(p);
                    mapTerm.printf(0,"%s",p);
                }
                else {
                    value = dict->set(sentence);
                    mapTerm.printf(0,"%s",sentence);
                }
                *value += valY;
                isNotFound = false;
                regfree(&re);
                break; // once found, break out the loop through keywords list
            }
        }
        regfree(&re);
    }
    if (isNotFound && misMatch){
        idx  * value;
        value= dict->set("Others");
        *value += valY;
        mapTerm.printf(0,"Others");
        return 1;
    }
    else if (isNotFound && misMatch==false){
        mapTerm.printf(0,"_Nothing");
        return 0;
    }
    return 1;
}

//bool mapToKeywords(const char * sentence,const char * keywordsList, idx valueY, bool useKeywords, bool misMatch, sStr & mapTerm){
static bool mapToKeywords(const char * sentence,const char * keywordsList, bool useKeywords, bool misMatch, sStr & mapTerm){

    sStr splitTerm;
    sString::searchAndReplaceStrings(&splitTerm,keywordsList,0,",",0,0,0);

    /*idx valY = 1;
    if (valueY!=sIdxMax){
        valY = valueY;
    }*/
    bool isNotFound = true;
    for (char * p=splitTerm.ptr(0); p ; p=sString::next00(p)){
        regex_t re;
        idx regerr = regcomp(&re, p, REG_EXTENDED|REG_ICASE) ; // set up key for regex

        if (!regerr){
            bool match = (regexec(&re,sentence, 0, NULL, 0)==0) ? true : false; // matching process
            if (match){
     //           idx  * value;
                if (useKeywords)  {
                   // value= dict->set(p);
                    *p = toupper(p[0]);
                    mapTerm.printf(0,"%s",p);
                }
                else {
                    //value = dict->set(sentence);
                    mapTerm.printf(0,"%s",sentence);
                }
       //         *value += valY;
                isNotFound = false;
                regfree(&re);
                break; // once found, break out the loop through keywords list
            }
        }
        regfree(&re);
    }
    if (isNotFound && misMatch){
        //idx  * value;
       // value= dict->set("Others");
       // *value += valY;
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

    sscanf(dst.ptr(),"%"DEC"", &a); // sscanf(c.ptr(0), "%"DEC"", &myRangeNum.start);
    return a;
}

bool DataTrendingCommand::init(const char * op_name, sVariant * arg)
{
    // Getting Params from front-end

   /* if (arg->getDicElt("xColumns") != 0) {// what if the first column was chosen ?
        toAdd->colX = arg->getDicElt("xColumns")->asInt();
    }*/
    if ( arg->getDicElt("periodAsDate") !=0 ) // like x axis
    {
        isDate = arg->getDicElt("periodAsDate")->asBool();
    }

    if (sVariant * colSetImgVal = arg->getDicElt("period")) // like x axis
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
    if (sVariant * colSetImgVal = arg->getDicElt("labelColumns")) // like group of x
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
    if (sVariant * colSetImgVal = arg->getDicElt("yColumns")) // number of elements, like y axis
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

  /*  if (arg->getDicElt("groupBy") != 0) {// In case when users want to group X elements by another criteria
           toAdd->colGroup = arg->getDicElt("groupBy")->asInt();
    }*/

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
         //toAdd->xKeywords.printf (0,"%s", arg->getDicElt("periodFrom")->asString());
     }
    if (arg->getDicElt("periodTo") != 0){
        // toAdd->xKeywords.printf (0,"%s", arg->getDicElt("periodTo")->asString());
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

   // toAdd->sorter.init(arg, qlang_parser, log, "data_trending", true);

    return true;
}


bool DataTrendingCommand::compute(sTabular * tbl)
{
    // use the last chosen column from the vector of column
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

    // check if required to group X by another criteria
 //   sDic <sDic <idx> > subGroup;   // [x][group] = y
//    sDic <sDic <idx> > dateOccurence; // [date][label] = y

 //   sDic <idx> xFreq;// dictionary of X by Date

    sDic < elementInfo > dicOfElements;

    //---------------------------------
    // Sorting the period column
    //---------------------------------
    if (colPeriod !=-2){
        sReorderedTabular reordered (tbl, false);
        if (isDate){
            sorter.sort(&reordered);
        }

        //--------------------------------
        // Preparing output table
        // Dictionarize the period column

        // Group rows by Sorted Date
        sStr period, dst;
        for (idx irow=0; irow< tbl->rows(); irow++){
            period.cut(0); dst.cut(0);
            //tbl->printCell(period,irow,colPeriod,0,""); // ToFix: empty cell case ??
            reordered.printCell(period,irow,colPeriod,0);

            if (!period.length()){ // in case the cell has no value
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
                    //mapToKeywords(const char * sentence,const char * keywordsList, idx valueY, bool useKeywords, bool misMatch, sStr & mapTerm)
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

        //  test with dictionary of elements
        sStr bufLabel, bufLabelToClean;
        sStr bufY;
        for (idx iE=0; iE < dicOfElements.dim(); ++iE){
            const char * key = (const char *)dicOfElements.id(iE); // Date or key
            elementInfo * rowElement = dicOfElements.get(key);

            for (idx ir=0; ir<rowElement->rowIndex.dim();++ir){
                bufLabel.cut(0); bufLabelToClean.cut(0); bufY.cut(0);
                idx valY=sIdxMax;
                idx realRow = rowElement->rowIndex.ptr(ir)[0];
                idx columnToRead = -2;
                if (colGroup!=-2){ // if sub group column exist
                    columnToRead = colGroup;
                }
                else columnToRead = colPeriod;
                reordered.printCell(bufLabelToClean,realRow,columnToRead,0);
                if (!bufLabelToClean.length() && misValue){ // in case missing value
                    //bufLabelToClean.printf(0,defaultMissingValue);
                    bufLabelToClean.cut(0);
                    bufLabelToClean.addString(defaultMissingValue,sLen(defaultMissingValue));
                }
                else if (!bufLabelToClean.length()) continue; // dont take the missing value

                labelTreatment(bufLabel,bufLabelToClean.ptr(0), bufLabelToClean.length());

                if (colY!=-2){
                    reordered.printCell(bufY,realRow,colY,0,"");
                    valY = convertToIdx(bufY.ptr(0));

                }
                if (colGroup!=-2 && groupKeywords.length() && strcmp(bufLabel.ptr(),defaultMissingValue)!=0){
                    sStr mapResult; bool isMapped = false;
                    if (useKeywords) {
                        //mapToKeywords(const char * sentence,const char * keywordsList, idx valueY, bool useKeywords, bool misMatch, sStr & mapTerm)
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
                // at this point, we have bufX and valY
                dicOfElements.ptr(iE)->numberOfOccurences +=1;
                if (colGroup ==-2) { //i.e, no Group
                    bufLabel.cut(0);
                    bufLabel.addString(colPeriodHdr.ptr(),colPeriodHdr.length());
                }
                if (dicOfElements.ptr(iE)->group.find(bufLabel.ptr(0),bufLabel.length())){
                    idx * value = dicOfElements.ptr(iE)->group.get(bufLabel.ptr(0),bufLabel.length());
                    if (valY == sIdxMax) {
                        if (colGroup != -2){ // in case did not choose Count, increase the element
                            *value = *value +1;
                        } else { // not choose group and count, take period occurrences
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


    // -------------------
    //  Outputting
    // -------------------
    sStr outPutPath;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://"OUTFILE,0,0); // set the file name to the request
    _ctx.qproc().reqDataPath(_ctx.outReqID(),OUTFILE,&outPutPath); // find the path of the file name
    sFile::remove(outPutPath);                // in case the file already existed

    sFil dataTrendingCSV(outPutPath);

    // preparingheader

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
        //if (iElement) dataTrendingCSV.addString(",",1);
        dataTrendingCSV.addString(rowElement,len);
        dataTrendingCSV.addString(",",1);
        for (idx iHdr=0; iHdr < headerDic.dim(); ++iHdr){
            if (iHdr) dataTrendingCSV.addString(",",1);
            const char * header = (const char *)headerDic.id(iHdr,&len);
            idx * value;
            if (dicOfElements.ptr(iElement)->group.find(header,len)){
             value= dicOfElements.ptr(iElement)->group.get(header,len);
             dataTrendingCSV.printf("%"DEC"",*value);
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

