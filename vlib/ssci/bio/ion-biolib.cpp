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
#include <slib/utils/tbl.hpp>
#include <ssci/bio/ion-bio.hpp>


// #################################################################
// Guessing the type of file: in vitro or omics based on the Header
// in vitro Header:
//          Donor   Passage     Time:day(optional)      rep1_treatment      rep2_treatment
// omics Header:
//          Accession   GeneName    Description(optional)   metadata1(optional)   metadata2(optional)   PCBM1632_P3_R1  PCBM1632_P5_R1
// Note:
//          donor and sample are the same throughout the comments and code
//          PCBM1632 <==> one of the donor's identifier
//          P3 <==> passage number 3
//          treatment <==> could be replace by treatment type, for example: confluency, adipogenesis
// ##################################################

idx sIonExpression::checkFileType(){
    // return 0 ==> file error or unrecognizable file type
    //        1 ==> in vitro data
    //        2 ==> Omics data
    // Getting the first column header
    // const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=sIdxMax, const char * defValue=0, idx flags=0) const;
    // const char * hdr=tbl.cell(-1,ic,&sizeHdr); // sTxtTbl use row -1 as header
    idx sizeHdr;
    // checking the first column from the file
    const char * hdrValue = myTable->cell(-1,0,&sizeHdr);
    if (strncasecmp(hdrValue,"Donor",4)==0) {
        const char * hdrValue_1 = myTable->cell(-1,1,&sizeHdr);
        if (strncasecmp(hdrValue_1,"Passage",7)==0) {
            return 1; // in vitro data format
        }
        else return 0;
    }
    if (strncasecmp(hdrValue,"Accession",9)==0) {
        const char * hdrValue_1 = myTable->cell(-1,1,&sizeHdr);
        if (strncasecmp(hdrValue_1,"GeneName",8)==0) {
            return 2; // Omics data format
        }
        else return 0;
    }

    return 0;
}
idx sIonExpression::parseConventionalExpression(const char * expressionFile, const char * experiment){
// Header: sample(mandatory),passage(mandatory),idType_1(optional),idType_2(optional),..,rep1_[treatment],rep2_[treatment],...
// example: sample,passage,time:day,treatment,rep1_adipogenesis,rep2_adipogenesis,...
//          8F3560,3,1,adipogenesis,8.123,7.589,...

    sFil exprFl(expressionFile,sMex::fReadonly);
    if(!exprFl.ok())
        return 0;
    // ##############
    // ignoring line starts with # at the beginning of the file
    // #################
    const char * fileBuf = exprFl.ptr(0);
    idx fileLength = exprFl.length();
    while (true){
        if (fileBuf[0]!='#') break;
        while (fileBuf < (fileBuf + fileLength) && *fileBuf != '\n'){ // scan until the end of the line
            fileBuf++; fileLength--;
        }
        if (fileBuf[0]=='\n') {
            fileBuf++; fileLength--;
        }
    }

    // Reading the rest as table
    sTxtTbl tbl;
    myTable = &tbl;
    myTable->setBuf(fileBuf,fileLength);
    myTable->parse();

    if (!myTable->cols() || myTable->cols()<2) {
        errorMsg.printf(0,"file's not in conventional format. There are not enough columns");
        return 0; // not conventional format
    }

    idx type = checkFileType();
    switch(type) {
        case 1: // in vitro data format
            parseInVitro(experiment);
            break;
        case 2: // Omics data format
            parseOmics(experiment);
            break;
        default:
            errorMsg.printf(0,"file's not in conventional format");
            break;
    }

    return 1;
}

idx sIonExpression::parseInVitro(const char * experiment){
    //
       idx sizeHdr=0,sizeVal = 0, arr[100];
          // LOOK FOR what column the replication starts from
          // go over the column header
          idx repColumn = -2;
          for (idx ic=2; ic<myTable->cols(); ++ic){ // the first 2 column always the Sample, Passage
              const char * hdr=myTable->cell(-1,ic,&sizeHdr); // sTxtTbl use row -1 as header
              if (strncmp(hdr,"rep",3)==0 && strchr(hdr,'_')!=0){
                  repColumn = ic;
                  break;
              }
          }
          if (repColumn==-2) {
              errorMsg.printf("no replication, wrong file format");
              return 0; // no replication => wrong file format
          }

          sDic <idx> sampleDic;
          idx iVal = 0;
         // sStr compositePassage;
          char ibuf[128]; idx ilen=0;
          for(idx ir=0; ir<myTable->rows(); ++ir) {
              const char * sample; const char *passage;
              idx sizeSample, sizePassage;
              for (idx ic=0; ic<myTable->cols(); ++ic){
                  const char * hdr=myTable->cell(-1,ic,&sizeHdr); // sTxtTbl use row -1 as header
                  const char * val=myTable->cell(ir,ic,&sizeVal);
                  if (ic==0) { // column for sample or donor
                      iVal = sampleDic.find(val,sizeVal); // check if the sample exists in the Dictionary
                      if (!iVal){
                          *sampleDic.set(val,sizeVal) = 1; //
                          iVal = sampleDic.dim();
                      }
                      if (experiment){
                          arr[0]=addRecord(sIonExpression::eRow,sizeof(iVal),&iVal);
                          arr[1]=addRecord(sIonExpression::eExperiment,sLen(experiment),experiment);
                          addRelationVarg(sIonExpression::eRelExperiment,sNotIdx,arr,0);
                      }
                      sample = val;
                      sizeSample = sizeVal;
                  }
                  else if (ic == 1){
                      passage = val;
                      sizePassage = sizeVal;
                  }
                 else if (ic >1 && ic <repColumn){
                     arr[0]=addRecord(sIonExpression::eID,sizeVal,val);
                     arr[1]=addRecord(sIonExpression::eType,sizeHdr,hdr);
                     arr[2]=addRecord(sIonExpression::eRow,sizeof(iVal),&iVal);
                     addRelationVarg(sIonExpression::eRelID,sNotIdx,arr,0);
                  }
                 else {
                     ilen=0;
                     // ( const char *  src, const char * choice00,idx * numfnd,bool isCaseInSensitive, idx startNum, bool exactMatch)
                     //const char * separator = strchr(hdr,'_');
                     //idx foundSepar = sString::compareChoice(hdr,"_" __,0,0,0,1);
                     char * separator = sString::searchSubstring( hdr, sizeHdr, "_" __,1, sString_symbolsEndline,0);
                     if (separator){
                         ilen = (hdr+sizeHdr-1)-separator;
                         memcpy(ibuf,separator+1,ilen);
                        // compositePassage.addString(separator+1,(hdr+sizeHdr-1)-separator);
                       //  compositePassage.addString("@P",2);
                         memcpy(ibuf+ilen,"@P",2);
                         ilen+=2;
                     }
                     memcpy(ibuf+ilen,passage,sizePassage);
                     ilen+=sizePassage;
                    // compositePassage.addString(passage,sizePassage);
                     arr[0]=addRecord(sIonExpression::eRow,sizeof(iVal),&iVal);
                     arr[1]=addRecord(sIonExpression::eSample,sizeSample,sample);
                     arr[2]=addRecord(sIonExpression::ePassage,ilen,ibuf);
                     arr[3]=addRecord(sIonExpression::eValue,sizeVal,val);
                     arr[4]=addRecord(sIonExpression::ePvalue,1,&sMex::_zero);
                     addRelationVarg(sIonExpression::eRelVal,sNotIdx,arr,0);
                     //compositePassage.cut(0);
                 }
              }


          }
          return 1;
}

idx sIonExpression::parseOmics(const char * experiment){
        idx sizeHdr=0,sizeVal = 0, arr[100];
         // LOOK FOR what column the sample_passage_replicate column starts
         // go over the column header
         idx repColumn = -2, underscore=0;
         sStr hdrCell;
         for (idx ic=0; ic<myTable->cols(); ++ic){
             hdrCell.cut(0);
             myTable->printCell(hdrCell,-1,ic); // sTxtTbl use row -1 as header
             underscore=0;
             for (const char * p=strchr(hdrCell.ptr(0),'_'); p!=NULL; p=strchr(p+1,'_')){
                 underscore++;
             }
             if (underscore==2){
                 repColumn = ic;
                 break;
             }
         }
         if (repColumn==-2) {
             errorMsg.printf("no donor_passage_replication column header");
             return 0; // no replication => wrong file format
         }
         // Start Parsing table
         idx ilenSample=0, ilenPassage=0;
         for(idx ir=0; ir<myTable->rows(); ++ir) {  // Loop over ROWS
             bool ingestExperiment = false;
             for (idx ic=0; ic<myTable->cols(); ++ic){     // Loop over COLUMNS
                 const char * hdr=myTable->cell(-1,ic,&sizeHdr); // sTxtTbl use row -1 as header
                 const char * val=myTable->cell(ir,ic,&sizeVal);
                 if (ic < repColumn) { // id idtype pairs
                     arr[0]=addRecord(sIonExpression::eID,sizeVal,val);
                     arr[1]=addRecord(sIonExpression::eType,sizeHdr,hdr);
                     arr[2]=addRecord(sIonExpression::eRow,sizeof(ir),&ir);
                     addRelationVarg(sIonExpression::eRelID,sNotIdx,arr,0);

                     if (experiment && !ingestExperiment){  // relexperiment
                          arr[0]=addRecord(sIonExpression::eRow,sizeof(ir),&ir);
                          arr[1]=addRecord(sIonExpression::eExperiment,sLen(experiment),experiment);
                          addRelationVarg(sIonExpression::eRelExperiment,sNotIdx,arr,0);
                          ingestExperiment = true;
                     }
                 }
                else {
                    ilenSample=0, ilenPassage=0;
                    arr[0]=addRecord(sIonExpression::eRow,sizeof(ir),&ir);
                    const char * ibuf = strchr(hdr,'_');   // sample_passage_replication <=> PCBM1632_P3_R1
                    ilenSample = ibuf-hdr;
                    arr[1]=addRecord(sIonExpression::eSample,ilenSample,hdr);
                    ibuf = strchr(hdr+ilenSample+1,'_');
                    ilenPassage = ibuf - hdr - ilenSample -1;
                    arr[2]=addRecord(sIonExpression::ePassage,ilenPassage,hdr+ilenSample+1);
                    arr[3]=addRecord(sIonExpression::eValue,sizeVal,val);
                    arr[4]=addRecord(sIonExpression::ePvalue,1,&sMex::_zero);
                    addRelationVarg(sIonExpression::eRelVal,sNotIdx,arr,0);
                }
             }
         }
    return 1;
}
