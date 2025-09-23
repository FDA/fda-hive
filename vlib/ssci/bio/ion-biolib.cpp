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



idx sIonExpression::checkFileType(){
    idx sizeHdr;
    const char * hdrValue = myTable->cell(-1,0,&sizeHdr);
    if (strncasecmp(hdrValue,"Donor",4)==0) {
        const char * hdrValue_1 = myTable->cell(-1,1,&sizeHdr);
        if (strncasecmp(hdrValue_1,"Passage",7)==0) {
            return 1;
        }
        else return 0;
    }
    if (strncasecmp(hdrValue,"Accession",9)==0) {
        const char * hdrValue_1 = myTable->cell(-1,1,&sizeHdr);
        if (strncasecmp(hdrValue_1,"GeneName",8)==0) {
            return 2;
        }
        else return 0;
    }

    return 0;
}
idx sIonExpression::parseConventionalExpression(const char * expressionFile, const char * experiment){

    sFil exprFl(expressionFile,sMex::fReadonly);
    if(!exprFl.ok())
        return 0;
    const char * fileBuf = exprFl.ptr(0);
    idx fileLength = exprFl.length();
    while (true){
        if (fileBuf[0]!='#') break;
        while (fileBuf < (fileBuf + fileLength) && *fileBuf != '\n'){
            fileBuf++; fileLength--;
        }
        if (fileBuf[0]=='\n') {
            fileBuf++; fileLength--;
        }
    }

    sTxtTbl tbl;
    myTable = &tbl;
    myTable->setBuf(fileBuf,fileLength);
    myTable->parse();

    if (!myTable->cols() || myTable->cols()<2) {
        errorMsg.printf(0,"file's not in conventional format. There are not enough columns");
        return 0;
    }

    idx type = checkFileType();
    switch(type) {
        case 1:
            parseInVitro(experiment);
            break;
        case 2:
            parseOmics(experiment);
            break;
        default:
            errorMsg.printf(0,"file's not in conventional format");
            break;
    }

    return 1;
}

idx sIonExpression::parseInVitro(const char * experiment){
       idx sizeHdr=0,sizeVal = 0, arr[100];
          idx repColumn = -2;
          for (idx ic=2; ic<myTable->cols(); ++ic){
              const char * hdr=myTable->cell(-1,ic,&sizeHdr);
              if (strncmp(hdr,"rep",3)==0 && strchr(hdr,'_')!=0){
                  repColumn = ic;
                  break;
              }
          }
          if (repColumn==-2) {
              errorMsg.printf("no replication, wrong file format");
              return 0;
          }

          sDic <idx> sampleDic;
          idx iVal = 0;
          char ibuf[128]; idx ilen=0;
          for( idx ir = 0; ir < myTable->rows(); ++ir ) {
              const char * sample = 0;
              const char * passage = 0;
              idx sizeSample = 0, sizePassage = 0;
              for( idx ic = 0; ic < myTable->cols(); ++ic ) {
                  const char * hdr = myTable->cell(-1, ic, &sizeHdr);
                  const char * val = myTable->cell(ir, ic, &sizeVal);
                  if( ic == 0 ) {
                      iVal = sampleDic.find(val, sizeVal);
                      if( !iVal ) {
                          *sampleDic.set(val, sizeVal) = 1;
                          iVal = sampleDic.dim();
                      }
                      if( experiment ) {
                          arr[0] = addRecord(sIonExpression::eRow, sizeof(iVal), &iVal);
                          arr[1] = addRecord(sIonExpression::eExperiment, sLen(experiment), experiment);
                          addRelationVarg(sIonExpression::eRelExperiment, sNotIdx, arr, 0);
                      }
                      sample = val;
                      sizeSample = sizeVal;
                  } else if( ic == 1 ) {
                      passage = val;
                      sizePassage = sizeVal;
                  } else if( ic > 1 && ic < repColumn ) {
                      arr[0] = addRecord(sIonExpression::eID, sizeVal, val);
                      arr[1] = addRecord(sIonExpression::eType, sizeHdr, hdr);
                      arr[2] = addRecord(sIonExpression::eRow, sizeof(iVal), &iVal);
                      addRelationVarg(sIonExpression::eRelID, sNotIdx, arr, 0);
                  } else {
                      ilen = 0;
                      char * separator = sString::searchSubstring(hdr, sizeHdr, "_" __, 1, sString_symbolsEndline, 0);
                      if( separator ) {
                          ilen = (hdr + sizeHdr - 1) - separator;
                          memcpy(ibuf, separator + 1, ilen);
                          memcpy(ibuf + ilen, "@P", 2);
                          ilen += 2;
                      }
                      memcpy(ibuf + ilen, passage, sizePassage);
                      ilen += sizePassage;
                      arr[0] = addRecord(sIonExpression::eRow, sizeof(iVal), &iVal);
                      arr[1] = addRecord(sIonExpression::eSample, sizeSample, sample);
                      arr[2] = addRecord(sIonExpression::ePassage, ilen, ibuf);
                      arr[3] = addRecord(sIonExpression::eValue, sizeVal, val);
                      arr[4] = addRecord(sIonExpression::ePvalue, 1, &sMex::_zero);
                      addRelationVarg(sIonExpression::eRelVal, sNotIdx, arr, 0);
                  }
              }
          }
          return 1;
}

idx sIonExpression::parseOmics(const char * experiment){
        idx sizeHdr=0,sizeVal = 0, arr[100];
         idx repColumn = -2, underscore=0;
         sStr hdrCell;
         for (idx ic=0; ic<myTable->cols(); ++ic){
             hdrCell.cut(0);
             myTable->printCell(hdrCell,-1,ic);
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
             return 0;
         }
         idx ilenSample=0, ilenPassage=0;
         for(idx ir=0; ir<myTable->rows(); ++ir) {
             bool ingestExperiment = false;
             for (idx ic=0; ic<myTable->cols(); ++ic){
                 const char * hdr=myTable->cell(-1,ic,&sizeHdr);
                 const char * val=myTable->cell(ir,ic,&sizeVal);
                 if (ic < repColumn) {
                     arr[0]=addRecord(sIonExpression::eID,sizeVal,val);
                     arr[1]=addRecord(sIonExpression::eType,sizeHdr,hdr);
                     arr[2]=addRecord(sIonExpression::eRow,sizeof(ir),&ir);
                     addRelationVarg(sIonExpression::eRelID,sNotIdx,arr,0);

                     if (experiment && !ingestExperiment){
                          arr[0]=addRecord(sIonExpression::eRow,sizeof(ir),&ir);
                          arr[1]=addRecord(sIonExpression::eExperiment,sLen(experiment),experiment);
                          addRelationVarg(sIonExpression::eRelExperiment,sNotIdx,arr,0);
                          ingestExperiment = true;
                     }
                 }
                else {
                    ilenSample=0, ilenPassage=0;
                    arr[0]=addRecord(sIonExpression::eRow,sizeof(ir),&ir);
                    const char * ibuf = strchr(hdr,'_');
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
