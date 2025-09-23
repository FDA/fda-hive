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
#include <ssci/bio/sVioAnnot.hpp>


struct GBFloorDescription;

    struct GBFloorDescription{
        const char * floorTag;
        GBFloorDescription * children;
        bool specificTreatment;
    };

    GBFloorDescription sCDS[]={
    {"/locus_tag=",0,true},
    {"/product=",0,true},
    {"/protein_id=",0,true},
        {"/",0,false},
        {0,0}
    };

    GBFloorDescription sFeatures[]={
        {"CDS",sCDS,true},
        {"mat_peptide",sCDS,true},
        {0,0}
    };
  GBFloorDescription searchSystem[]={
        {"LOCUS",0,true},
        {"DEFINITION",0,true},
        {"VERSION",0,true},
        {"FEATURES",sFeatures,false},
        {"//",0,true},
        {0,0}
    };
idx sVioAnnot::progressReport(idx progress, idx percent)
{
    idx rc = 1;
    if( myCallbackFunction ) {
        time_t t0 = time(0);
        if( percent > m_progressUpdate || (t0 - m_progressLast) > 60 ) {
            m_progressUpdate = percent;
            m_progressLast = t0;
            rc = myCallbackFunction(myCallbackParam, progress, percent, 100);
        }
    }
    return rc;
}

const char * getonelinefromBuffer(char * buf, const char * fileContent, idx len)
{
    int i;
    if(!fileContent[0])
    {
        return fileContent;
    }
    for (i=0;fileContent[i] && fileContent[i]!='\n' && fileContent[i]!='\r' && i<len ; ++ i)
        buf[i]=fileContent[i];
    buf[i]='\0';
    if(fileContent[i]=='\r' && fileContent[i+1]=='\n')
        ++i;

    return fileContent+i+1;
}


void gbdDumper(const char * locus, const char * data, const char * tag, const char * path, const char *value, sStr & lineEx)
{
    sStr buf; sString::searchAndReplaceSymbols(&buf,tag,0,"/=","",0,true,true,true,true);
    const char * clTag =buf.ptr();

    sStr vbuf; vbuf.printf("%s",value);
    if(vbuf.length())sString::cleanEnds(vbuf.ptr(), 0,"\"",true);
    const char * clValue =vbuf.ptr();

    sStr sLocus, sData, sTag, sPath, sValue;
    sString::escapeForCSV(sLocus,locus);
    sString::escapeForCSV(sData,data);
    sString::escapeForCSV(sTag,clTag);
    sString::escapeForCSV(sPath,path);
    sString::escapeForCSV(sValue,clValue);

    lineEx.printf("%s,%s,%s,%s,%s\n",sLocus.ptr(),sData.ptr(), sTag.ptr(),sPath.ptr(),sValue.ptr());
}

void sVioAnnot::specificTreatmentFunction ( const char * tagName, const char * value, bool isContinued, sStr & lineExp )
{
    sStr buf;
    sStr path(" ");

    sString::searchAndReplaceSymbols(&buf,value,0,sString_symbolsBlank," ",0,true,true,true,true);
    if(buf.length())sString::cleanEnds(buf.ptr(), 0,sString_symbolsBlank,true);
    const char * text =buf.ptr();

    if(strcmp(tagName,"LOCUS")==0 ){
        ++locusNum;
        gbd.locus.printf(0,"%s",text);
        char * p=strchr(gbd.locus,' ');
        if(p)*p=0;
        cdsNum=0; tagNum=0;
        matPepNum = 0, matPepTagNum = 0;
    }
    else {
        if(isContinued){
            if(gbd.curValue.length())gbd.curValue.printf(" ");
            gbd.curValue.printf("%s",text);
            if (gbd.curLocus.length()==0){
                gbd.curLocus.printf("%s",gbd.locus.ptr());
            }
            return;
        }
    }

    if(strcmp(tagName,"CDS")==0 || strcmp(tagName,"mat_peptide")==0){
        if (strcmp(tagName,"CDS")==0) {++cdsNum; tagNum=1;}
        if (strcmp(tagName,"mat_peptide")==0) {++matPepNum; matPepTagNum=1;}
        dataFlag.cut(0);
        dataFlag.printf(0,"%s",tagName);

            if(strncmp(text+0,"complement",10)==0 ){ complement = true; }
            if(strncmp(text+0,"join",4)==0){ join = true; }
        if (complement) {
            compText.printf("%s", text);
        }
        if (join){
            joinText.printf("%s", text);
        }
        buf.cut(0);
        sString::searchAndReplaceSymbols(&buf,text,0,"..",0,0,true,true,true,true);

        gbd.start.cut(0);
        sStr startRw; startRw.printf(0,"%s",sString::next00(buf,0));
        if (strncmp("<",startRw,1)==0) { gbd.start.printf(0,"%s",startRw.ptr(1));}
        else { gbd.start.printf(0,"%s",startRw.ptr(0));}
        gbd.start.add0();
        gbd.end.cut(0);
        sStr endRw; endRw.printf(0,"%s",sString::next00(buf,1));
        if (strncmp(">",endRw,1)==0) { gbd.end.printf(0,"%s",endRw.ptr(1));}
        else { gbd.end.printf(0,"%s",endRw.ptr(0));}
        gbd.end.add0();
    }
    else if (strcmp(dataFlag.ptr(0),"CDS")==0) ++tagNum;
    else if (strcmp(dataFlag.ptr(0),"mat_peptide")==0) ++matPepTagNum;

    if(gbd.curValue.length()!=0) {
        path.cut(0);
        if (strcmp(dataFlag.ptr(0),"CDS")==0) path.printf("%" DEC ".1.%" DEC ".%" DEC,locusNum,cdsNum,tagNum);
        if (strcmp(dataFlag.ptr(0),"mat_peptide")==0) path.printf("%" DEC ".2.%" DEC ".%" DEC,locusNum,matPepNum,matPepTagNum);
        if (strcmp(gbd.curTag.ptr(),"CDS")==0 || strcmp(gbd.curTag.ptr(),"mat_peptide")==0) {
            if (join==false && complement==false) {
                lineExp.printf("%s,%s,%s,%s,%s,%s\n",gbd.locus.ptr(),dataFlag.ptr(),"range", gbd.curPath.ptr(), gbd.start.ptr(),gbd.end.ptr());
            }
            if (join) {
                lineExp.printf("%s,%s,%s,%s,%s\n",gbd.locus.ptr(),dataFlag.ptr(),"range", gbd.curPath.ptr(), joinText.ptr());
                joinText.cut(0);
                join = false;
            }
            if (complement) {
                lineExp.printf("%s,%s,%s,%s,%s\n",gbd.locus.ptr(),dataFlag.ptr(),"range", gbd.curPath.ptr(), compText.ptr());
                compText.cut(0);
                complement = false;
            }
            gbd.curPath.cut(0);
        }

        else if (gbd.curPath.length()>1) {
            gbdDumper(gbd.curLocus.ptr(),gbd.curData.ptr(),gbd.curTag.ptr(), gbd.curPath.ptr(), gbd.curValue.ptr(),lineExp);
             }
        else if (strcmp(gbd.curTag.ptr(),"CDS")!=0 && strcmp(gbd.curTag.ptr(),"mat_peptide")!=0) {
            header = header + 1 ;
            gbd.curPath.cut(0);
            gbd.curPath.printf("%" UDEC ".%d.%d.%" DEC,locusNum,0,0,header);
            gbdDumper(gbd.locus.ptr(), gbd.curData.ptr(), gbd.curTag.ptr(), gbd.curPath.ptr(), gbd.curValue.ptr(),lineExp);
        }
    }

    if (strcmp(tagName,"//")==0 && buf.length()==0) {
           gbd.curLocus.cut(0);
           gbd.curData.cut(0);
           gbd.curPath.cut(0);
           gbd.curTag.cut(0);
           gbd.curValue.cut(0);
           dataFlag.cut(0);
           header = 0;
           return;
    }
    gbd.curLocus.printf(0,"%s",gbd.locus.ptr());
    gbd.curPath.printf(0,"%s",path.ptr());
    gbd.curData.printf(0,"%s",dataFlag.ptr());
    gbd.curTag.printf(0,"%s", tagName);
    gbd.curValue.printf(0,"%s",text);

}

bool sVioAnnot::extractedInfo(char * mystring, int FloorNum[],int & currentFloor ,int & FoundLineNonBlankPos, int & quoteCount, bool & continueationMode, int & line_num, int & find_result,sStr & lineLine){
    struct GBFloorDescription * searchingLayers = 0;
    bool flag=true;
    int nonBlankPos=0;
          for(nonBlankPos=0; strchr(" \t\r\n",mystring[nonBlankPos])!=NULL; ++nonBlankPos)
              {}

          if(quoteCount%2==0){
              quoteCount=0;
              continueationMode=false;
              if( nonBlankPos==0){
                  searchingLayers=searchSystem;
                  currentFloor=0;
              }
              else if( nonBlankPos==12 && FloorNum[0]!=-1){
                 searchingLayers=searchSystem;
                 searchingLayers->specificTreatment=true;
                 continueationMode=true;
                 flag=false;
              }
              else if( nonBlankPos <20 && FloorNum[0]!=-1 ) {
                  searchingLayers=searchSystem[FloorNum[0]].children;
                  currentFloor=1;
              }
              else if( nonBlankPos >20 && FloorNum[1]!=-1 && FloorNum[0]!=-1) {

                  currentFloor=2;
                  searchingLayers=searchSystem[FloorNum[0]].children[FloorNum[1]].children;
              }
              else {
                  return false;
              }
              if (flag){
                  FloorNum[currentFloor]=-1;
                  int i;
                  for( i=0 ; searchingLayers[i].floorTag!=0 ; ++i ) {
                      if ((strstr(mystring+nonBlankPos,searchingLayers[i].floorTag))==mystring+nonBlankPos)
                      {
                          FloorNum[currentFloor]=i;
                          find_result++;
                          FoundLineNonBlankPos=nonBlankPos;
                          break;
                      }
                      line_num++;
                  }

                  if(FloorNum[currentFloor]==-1 || (searchingLayers[i].floorTag==0 && searchingLayers[FloorNum[currentFloor]].floorTag==NULL) )
                      return false;

                  if(!searchingLayers[FloorNum[currentFloor]].specificTreatment)
                      return false;
              }
          }else{
              continueationMode=true;
              if ( nonBlankPos >20 && FloorNum[1]!=-1){
                  searchingLayers=searchSystem[FloorNum[0]].children[FloorNum[1]].children;
              }
          }
          for ( int i=0; mystring[i]!=0; ++i) if(mystring[i]=='\"')++quoteCount;
          const char * tag=searchingLayers[FloorNum[currentFloor]].floorTag;

          specificTreatmentFunction(tag, mystring+nonBlankPos+( continueationMode==false ? strlen(tag) : 0 ) , continueationMode,lineLine);
          return true;
}
void RangeExtraction(char * value,sVec <idx> * startArray,sVec <idx> * endArray){
    sStr buf;
    if (strncmp("join",value,4)==0) { buf.printf(0,"%s",value+5);}
    if (strncmp("complement",value,10)==0) { buf.printf(0,"%s",value+11);}
    else buf.printf(0,"%s",value);
    value = buf.ptr(0);
    buf.cut(0);
    sString::searchAndReplaceSymbols(&buf,value,0,",",0,0,true,true,false,true);
    idx eleNum = sString::cnt00(buf);

    for (idx ele =0; ele<eleNum;ele++){
        const char * textRaw = sString::next00(buf,ele);

        sStr text;
        sStr startEnd;
        bool checkComplement = (strncmp("complement",textRaw,10)==0) ? true : false;
        bool checkJoin = (strncmp("join",textRaw,4)==0) ? true : false;
        if (checkComplement==true){
            sString::cleanMarkup(&text,textRaw,0,"complement(" _,")" _,"" _,0,true,false, false);
            sString::searchAndReplaceSymbols(&startEnd,text.ptr(1),0,"..",0,0,true,true,false,true);
        }
        if (checkJoin==true){
            sString::cleanMarkup(&text,textRaw,0,"join(" _,")" _,"" _,0,true,false, false);
            sString::searchAndReplaceSymbols(&startEnd,text.ptr(1),0,"..",0,0,true,true,false,true);
        }
        else if (checkComplement==false && checkJoin==false) {
            text.printf("%s",textRaw) ;
            sString::searchAndReplaceSymbols(&startEnd,text.ptr(0),0,"..",0,0,true,true,false,true);
        }
        idx eS = sString::cnt00(startEnd);
        if (eS==1){
            sStr startClean; const char * startRaw = sString::next00(startEnd,0);
            if (strncmp("<",startRaw,1)==0) { startClean.printf(0,"%s",startRaw+1);}
            else if (strncmp("<",startRaw,1)!=0) { startClean.printf(0,"%s",startRaw);}
            startArray->add();
            sscanf(startClean.ptr(0),"%" DEC "",startArray->ptr(ele));

            endArray->add();
            sscanf("0","%" DEC "",endArray->ptr(ele)) ;

        }
        else {
            sStr startClean; char * startRaw = sString::next00(startEnd,0);
            sString::cleanEnds(startRaw,0,")" _,true,0);
            sStr endClean; char * endRaw =sString::next00(startEnd,1);
            sString::cleanEnds(endRaw,0,")" _,true,0);

            if (strncmp("<",startRaw,1)==0) { startClean.printf(0,"%s",startRaw+1);}
            else if (strncmp("<",startRaw,1)!=0) { startClean.printf(0,"%s",startRaw);}
            if (strncmp(">",endRaw,1)==0) { endClean.printf(0,"%s",endRaw+1);}
            else if (strncmp(">",endRaw,1)!=0) { endClean.printf(0,"%s",endRaw);}
            startArray->add();
            sscanf(startClean.ptr(0),"%" DEC "",startArray->ptr(ele));

            endArray->add();
            sscanf(endClean.ptr(0),"%" DEC "",endArray->ptr(ele));

        }
    }
}



void sVioAnnot::composeIdAndIdType(const char * myId,const char * myIdType, sStr & ididType){
    ididType.printf(0,"%s",myId);
    ididType.add0();
    ididType.printf("%s",myIdType);
}
void sVioAnnot::addRecordRelationShipCounterForSeqID(sVioDB * myDB, sStr & myLineSeparByZero, sStr & myGi){
    sVec < sStr > myIdList;


    sStr * seqid = myIdList.add();
    composeIdAndIdType(sString::next00(myLineSeparByZero,0),"seqID",*seqid);
    sStr * feature = myIdList.add();
    composeIdAndIdType(sString::next00(myLineSeparByZero,1),"feature",*feature);
    sStr * ggi = myIdList.add();
    composeIdAndIdType(myGi.ptr(0),"gi",*ggi);

    for (idx iid=0; iid<myIdList.dim(); ++iid){
        idx recordNum = 0;
        if(myDB->SetRecordIndexByBody((const void *)myIdList[iid].ptr(0), idxType_id, &recordNum, myIdList[iid].length()+1 )){
            recordNum=myDB->AddRecord(idxType_id,(const void *)myIdList[iid].ptr(0), myIdList[iid].length()+1);
        }
        myDB->AddRecordRelationshipCounter(idxType_id, recordNum, 1);
        myDB->AddRecordRelationshipCounter(idxType_id, recordNum, 2);

        idx idTypeRecord =0;
        sStr idt; idt.printf("%s",sString::next00(myIdList[iid].ptr(0),1));
        if(myDB->SetRecordIndexByBody((const void *)idt.ptr(0), idxType_idType, &idTypeRecord, idt.length()+1 )){
            idTypeRecord=myDB->AddRecord(idxType_idType,(const void *)idt.ptr(0), idt.length()+1);
        }
        myDB->AddRecordRelationshipCounter(idxType_idType, idTypeRecord, 1);
    }
}

idx sVioAnnot::ParseGBfile(const char * inputGBFile, const char* vioFileName, bool combineFile)
{
   idx maxRelationShip=3;
   idx numberOfType=32;


   sVioDB db(vioFileName,"vioAnnot",numberOfType,maxRelationShip);

    idx relationlistId[3]={2,3,4};
    idx relationlistRange[1]={1};
    idx relationlistIdType[1]={1};
    idx relationlistRefSource[1]={1};



      db.AddType(sVioDB::eString,sDim(relationlistId),relationlistId,"id",1);
      db.AddType(sVioDB::eOther,sDim(relationlistRange),relationlistRange,"range", 2);
      db.AddType(sVioDB::eString,sDim(relationlistIdType),relationlistIdType,"idType",3);
      db.AddType(sVioDB::eString,sDim(relationlistRefSource),relationlistRefSource,"refSrce", 4);

      for (idx iDummy=5; iDummy<33; ++iDummy){
          sStr dummyName; dummyName.printf(0,"dmmy_%" DEC "",iDummy);
          db.AddType(sVioDB::eString,sDim(relationlistRefSource),relationlistRefSource, dummyName.ptr(0),iDummy);
      }

   int FloorNum[10] ,FoundLineNonBlankPos=0,quoteCount=0;
   int currentFloor=0;
   bool continueationMode=false;
   for ( int i=0; i< sDim(FloorNum); ++i)FloorNum[i]=-1;

   char mystring[SIZEBUF];
   int line_num=1;
   int find_result=0;

   sFil srcFile(inputGBFile,sMex::fReadonly);
   if( !srcFile.ok() ) {
       return -3;
   }

   if (strncmp("LOCUS",srcFile.ptr(0),5)!=0){
       return -6;
   }

   const char * src=srcFile.ptr();
   idx length=srcFile.length();
   const char * endpos=srcFile.ptr(length);
   bool success;
   idx rangeN=1;
   sStr currentGi;
#ifdef _DEBUG
   ::printf("Start preparing containers \n\n");
#endif
       for (idx l=0;l<length && *src;l++)
       {
           src=getonelinefromBuffer(mystring,src,endpos-src);
           if (sLen(mystring)==0) continue;
           sStr extractedLine;

           success = extractedInfo(mystring,FloorNum, currentFloor ,FoundLineNonBlankPos, quoteCount,continueationMode,line_num,find_result, extractedLine);

           if (!success){
               success = true;
               continue;
           }
           idx recNumTag=rangeN-1;
           if (extractedLine.length()>6){
                 sStr lineSeparByZero;
                 sString::searchAndReplaceSymbols(&lineSeparByZero,extractedLine,0,",",0,0,true,true,false,true);
                 idx elementNum = sString::cnt00(lineSeparByZero);

                 sStr tagName;
                 tagName.printf("%s",sString::next00(lineSeparByZero,2));

                 sStr indexPath;
                 indexPath.printf("%s",sString::next00(lineSeparByZero,3));

                 sStr Nodes;
                 sString::searchAndReplaceSymbols(&Nodes,indexPath,0,".",0,0,true,true,false,true);

                 sStr valueUnclean;
                 valueUnclean.printf(0,"%s",sString::next00(lineSeparByZero,4));
                 if (elementNum >5){
                     for (idx i=5;i<elementNum;i++){
                         valueUnclean.printf(",%s",sString::next00(lineSeparByZero,i));
                     }
                 }

                 sStr value;
                 sString::cleanEnds(&value,valueUnclean.ptr(),valueUnclean.length(),sString_symbolsEndline,true);

                 if (sIs(tagName,"LOCUS")){ continue; }
                 if (sIs(tagName,"DEFINITION")){ continue; }

                 if (sIs(tagName,"VERSION")){
                     sStr giNumLine;
                     sString::searchAndReplaceSymbols(&giNumLine, value, 0, ":", 0, 0, true, true, false, true);
                     currentGi.printf(0,"%s", sString::next00(giNumLine, 1));
                 }

                 if (sIs(tagName,"range")){
                       sVec <startEnd> rangeVec;
                       if (strncmp("join",value.ptr(0),4)==0 || strncmp("complement",value.ptr(0),10)==0){
                           sVec <idx> startArray; sVec <idx> endArray;
                           RangeExtraction(value.ptr(0),&startArray,&endArray);
                           for (idx i=0;i<startArray.dim();i++){
                               rangeVec.add();
                               rangeVec[i].start = startArray[i];
                               rangeVec[i].end = endArray[i];
                           }
                           if(startArray.dim())rangeVec[0].max=rangeVec[startArray.dim()-1].end;
                       }
                       else {
                           rangeVec.add();
                           sscanf(sString::next00(lineSeparByZero.ptr(),4),"%" DEC "",&rangeVec[0].start);
                           sscanf(sString::next00(lineSeparByZero.ptr(),5),"%" DEC "",&rangeVec[0].end);
                           rangeVec[0].max=rangeVec[0].end;
                       }
                       db.AddRecord(idxType_range,rangeVec.ptr(), sizeof(struct startEnd)*rangeVec.dim());

                       db.AddRecordRelationshipCounter(idxType_range, rangeN, 1);
                       db.AddRecordRelationshipCounter(idxType_range, rangeN, 1);
                       db.AddRecordRelationshipCounter(idxType_range, rangeN, 1);

                       addRecordRelationShipCounterForSeqID(&db,lineSeparByZero,currentGi);
                       rangeN++;
                   }

                 if (strcmp(tagName,"locus_tag")==0 || strcmp(tagName,"protein_id")==0 || strcmp(tagName,"product")==0){
                         idx recNumKind=0,recNumID=0;

                         if(db.SetRecordIndexByBody((const void *)tagName.ptr(0), idxType_idType, &recNumKind, tagName.length()+1 )){
                             recNumKind=db.AddRecord(kindOfId_TYPE,(const void *)tagName.ptr(0), tagName.length()+1);
                         }
                         db.AddRecordRelationshipCounter(idxType_idType, recNumKind, 1);

                         sStr myididType;
                         composeIdAndIdType(value.ptr(0),tagName.ptr(0),myididType);

                         if(db.SetRecordIndexByBody((const void *)myididType.ptr(0), idxType_id, &recNumID, myididType.length()+1 )) {
                             recNumID=db.AddRecord(idxType_id,(const void *)myididType.ptr(0), myididType.length()+1);
                         }

                         db.AddRecordRelationshipCounter(idxType_id, recNumID, 1);
                         db.AddRecordRelationshipCounter(idxType_id, recNumID, 2);
                         db.AddRecordRelationshipCounter(idxType_range, recNumTag, 1);
                 }
               }
           }
       sStr tt;
       specificTreatmentFunction ( "TERMINATION_IMPOSSIBLE", "",false,tt);
      db.AllocRelation();

#ifdef _DEBUG
      ::printf("Start adding the relationships \n\n");
#endif
      sStr flnmInput1;
      flnmInput1.printf("%s",  inputGBFile);

      locusNum=0;
      cdsNum=0; tagNum=0;
      matPepNum = 0; matPepTagNum = 0;
      header = 0;
      dataFlag.printf(0," ");
      int FloorNumBis[10] ,FoundLineNonBlankPosBis=0,quoteCountBis=0;
      int currentFloorBis=0;
      bool continueationModeBis=false;
      for ( int i=0; i< 10; ++i)FloorNumBis[i]=-1;

      char mystringBis[SIZEBUF];
      int line_numBis=1;
      int find_resultBis=0;

      sFil srcFileBis(flnmInput1.ptr(),sMex::fReadonly);
      const char *srcBis=srcFileBis.ptr();
      idx lengthBis=srcFileBis.length();
      const char * endposBis=srcBis+lengthBis;
      bool successBis;

      idx rangeNBis=1;
      sStr currentGiBis;

       for (idx l=0;l<lengthBis && *srcBis;l++)
       {
                  srcBis=getonelinefromBuffer(mystringBis,srcBis,endposBis-srcBis);
                  if (sLen(mystringBis)==0) continue;
                  sStr extractedLineBis;
                  successBis = extractedInfo(mystringBis,FloorNumBis, currentFloorBis, FoundLineNonBlankPosBis, quoteCountBis,continueationModeBis,line_numBis,find_resultBis, extractedLineBis);

                  if (!successBis){
                      successBis = true;
                      continue;
                  }
                  idx recNumRange=rangeNBis-1;
                  if (extractedLineBis.length()>6){
                        sStr lineSeparByZeroBis;
                        sString::searchAndReplaceSymbols(&lineSeparByZeroBis,extractedLineBis,0,",",0,0,true,true,false,true);
                        idx elementNumBis = sString::cnt00(lineSeparByZeroBis);

                        sStr tagNameBis;
                        tagNameBis.printf("%s",sString::next00(lineSeparByZeroBis,2));

                        sStr indexPathBis;
                        indexPathBis.printf("%s",sString::next00(lineSeparByZeroBis,3));

                        sStr NodesBis;
                        sString::searchAndReplaceSymbols(&NodesBis,indexPathBis,0,".",0,0,true,true,false,true);

                        sStr valueUncleanBis;
                        valueUncleanBis.printf("%s",sString::next00(lineSeparByZeroBis,4));
                        if (elementNumBis >5){
                            for (idx i=5;i<elementNumBis;i++){
                                valueUncleanBis.printf(",%s",sString::next00(lineSeparByZeroBis,i));
                            }
                        }

                        sStr valueBis;
                        sString::cleanEnds(&valueBis,valueUncleanBis.ptr(),valueUncleanBis.length(),sString_symbolsEndline,true);

                        if (strcmp(tagNameBis,"LOCUS")==0){continue;}

                        if (strcmp(tagNameBis,"DEFINITION")==0){continue;}

                        if (strcmp(tagNameBis,"VERSION")==0){
                            sStr giNumLine;
                            sString::searchAndReplaceSymbols(&giNumLine, valueBis, 0, ":", 0, 0, true, true, false, true);
                            currentGiBis.printf(0,"%s",sString::next00(giNumLine,1));
                            currentGiBis.add0();
                            currentGiBis.printf("gi");
                        }
                        if (strcmp(tagNameBis,"range")==0 ){
                                sStr mySeqid; sStr myFeature;

                                composeIdAndIdType(sString::next00(lineSeparByZeroBis,0),"seqID",mySeqid);
                                composeIdAndIdType(sString::next00(lineSeparByZeroBis,1),"feature",myFeature);

                                idx recNumIdBis = 0;
                                idx recTypeBis = 0;
                                recNumIdBis = db.GetRecordIndexByBody((const void *)mySeqid.ptr(0), idxType_id, mySeqid.length()+1 );
                                db.AddRelation(idxType_range, 1, rangeNBis, recNumIdBis );
                                db.AddRelation(idxType_id, 1, recNumIdBis,rangeNBis );

                                recTypeBis = db.GetRecordIndexByBody((const void *)"seqID", idxType_idType, 5+1 );
                                db.AddRelation(idxType_id, 2, recNumIdBis, recTypeBis);
                                db.AddRelation(idxType_idType, 1, recTypeBis,recNumIdBis);

                                recNumIdBis = db.GetRecordIndexByBody((const void *)myFeature.ptr(0), idxType_id, myFeature.length()+1 );
                                db.AddRelation(idxType_range, 1, rangeNBis, recNumIdBis );
                                db.AddRelation(idxType_id, 1, recNumIdBis, rangeNBis );

                                recTypeBis = db.GetRecordIndexByBody((const void *)"feature", idxType_idType, 7+1 );
                                db.AddRelation(idxType_id, 2, recNumIdBis, recTypeBis);
                                db.AddRelation(idxType_idType, 1, recTypeBis,recNumIdBis);

                                recNumIdBis = db.GetRecordIndexByBody((const void *)currentGiBis.ptr(0), idxType_id, currentGiBis.length()+1 );
                                db.AddRelation(idxType_range, 1, rangeNBis, recNumIdBis );
                                db.AddRelation(idxType_id, 1, recNumIdBis, rangeNBis );

                                recTypeBis = db.GetRecordIndexByBody((const void *)"gi", idxType_idType, 2+1 );
                                db.AddRelation(idxType_id, 2, recNumIdBis, recTypeBis);
                                db.AddRelation(idxType_idType, 1, recTypeBis,recNumIdBis);


                                rangeNBis++;
                           }

                        if (strcmp(tagNameBis,"locus_tag")==0 || strcmp(tagNameBis,"protein_id")==0 || strcmp(tagNameBis,"product")==0){

                                idx matchNumKind=db.GetRecordIndexByBody((const void *)tagNameBis.ptr(0), idxType_idType, tagNameBis.length()+1 );
                                sStr myididTypeBis;
                                composeIdAndIdType(valueBis.ptr(0),tagNameBis.ptr(0),myididTypeBis);

                                idx matchNumID=db.GetRecordIndexByBody((const void *)myididTypeBis.ptr(0), idxType_id, myididTypeBis.length()+1 );

                                db.AddRelation(idxType_id, 2, matchNumID, matchNumKind);
                                db.AddRelation(idxType_idType, 1, matchNumKind, matchNumID );

                                db.AddRelation(idxType_range, 1, recNumRange, matchNumID);
                                db.AddRelation(idxType_id, 1, matchNumID, recNumRange);
                        }
                      }
                  }

       db.Finalize(combineFile);

       sVioAnnot myAnnot;
       myAnnot.init(vioFileName);
       idx cntId = sortVioAnnotFileRangesBasedOnSeqId(&myAnnot);

       return cntId;

}



idx sVioAnnot::getTotalRecord(){
    return DB.GetRecordCnt(generalLocus_TYPE);
}

const char * sVioAnnot::getDataNameByRangeIndex(idx rangeIndex){
    idx relationCntRange=0, relationTypeIndexRange=0;
    idx bodysizeData;
    idx * indexPtrData = DB.GetRelationPtr(range_TYPE, rangeIndex, 3, &relationCntRange, &relationTypeIndexRange);
    return (const char *) DB.Getbody(relationTypeIndexRange, *indexPtrData, &bodysizeData);
}


idx sVioAnnot::GetAnnotRangeList( const char * idToUse, const char * idtype00, idx position, sVec < sVioAnnot::AnnotStruct  > * results,  sVec < sVioAnnot > * annotList, const char * dataName)
{

    idx resultQuerySize=0,accumulatedRecords=0;
    ++position;
    for (idx al=0; al<annotList->dim(); al++) {
        sVioAnnot *Annot = annotList->ptr(al);
        if (!Annot) continue;
        if( Annot->isGBstructure()) {

        }
        else {
            idx cntRanges=0;
            idx * indexRangePtr =0;
            indexRangePtr = Annot->getNumberOfRangesByIdTypeAndId("seqID",idToUse, cntRanges);

            if (!indexRangePtr){
                indexRangePtr = Annot->getNumberOfRangesByIdTypeAndId("gi",idToUse, cntRanges);
                if (!indexRangePtr) continue;
            }

            accumulatedRecords++;
            sVec<startEndNode> locusResults;

            idx resultSize=Annot->searchInVirtualTree(indexRangePtr,cntRanges,locusResults,position,position);
            resultQuerySize+=resultSize;
            for (idx rs=0; rs<resultSize; rs++){
                idx subRangeHit=0;
                if(locusResults[rs].subRangesCnt>1){
                    idx foundinSubR=0;
                    for(idx sR=0;sR<locusResults[rs].subRangesCnt;++sR){
                        if(sOverlap(position,position,locusResults[rs].ranges[sR].start,locusResults[rs].ranges[sR].end)){
                            foundinSubR=1;
                            subRangeHit=sR;
                            break;
                        }
                    }
                    if(!foundinSubR)continue;
                }
                sVioAnnot::AnnotStruct * as=results->add();


                idx cntIDsForRange=Annot->getNberOfIdsByRangeIndex(locusResults[rs].index);
                sStr idTypeToCompare("protein");
                if (dataName && strcmp(dataName,"CDS") != 0) idTypeToCompare.printf(0,"%s",dataName);
                if(cntIDsForRange){
                    for( idx iid=0; iid<cntIDsForRange; ++iid)  {
                        const char * idPtr,*idTypePtr;
                        Annot->getIdTypeByRangeIndexAndIdIndex(locusResults[rs].index, iid, &idPtr, 0, &idTypePtr, 0);

                        if ( strncasecmp(idTypeToCompare.ptr(),idTypePtr,idTypeToCompare.length())==0 ) {
                            as->rangeID.printf(0,"%s",idPtr);
                            break;
                        }
                        else if (iid == cntIDsForRange - 1) {
                            as->rangeID.printf(0,"-");
                        }
                    }
                }
                as->subRangeHit=subRangeHit;
                as->annotFileIndex=al;
                for(idx sR=0;sR<locusResults[rs].subRangesCnt; ++sR){
                    as->rangeStart.vadd(1, locusResults[rs].ranges[sR].start-1);
                    as->rangeEnd.vadd(1,locusResults[rs].ranges[sR].end-1);
                }
            }

        }
    }
    if(!accumulatedRecords)return -1;
    return resultQuerySize;
}







bool sVioAnnot::isGBstructure(void) {
    sVioDB::TypeList * typelist_type = DB.GetTypePointer(1);
    const char * isLocus = (const char *)&typelist_type->tnameOfs;
    if (strcmp(isLocus,"Locus")!=0) return false;
    return true;
}

idx sVioAnnot::getRangeTypeIdx(void) {
    if(isGBstructure()) return range_TYPE;
    return idxType_range;
}

idx sVioAnnot::getIdTypeIdx(void) {
    if(isGBstructure()) return 4;
    return 1;
}

idx sVioAnnot::getRltTypeIdx_Id2Rng(void) {
    if(isGBstructure()) return 2;
    return 1;
}



idx sVioAnnot::getIdIndex(const char * idToUse, const char * idTypeToUse) {
    sStr hashLookUpBuf("%s",idToUse);
    hashLookUpBuf.add0(1);
    hashLookUpBuf.printf("%s",idTypeToUse);

   return DB.GetRecordIndexByBody((const void *)hashLookUpBuf.ptr(),idxType_id,hashLookUpBuf.length()+1);

}

idx sVioAnnot::getIdTypeIndex(const char * idToUse) {
    return DB.GetRecordIndexByBody((const void *)idToUse,idxType_idType,sLen(idToUse)+1);
}

void sVioAnnot::getIdByIdIndex(sStr & myId, idx idIndex){
    idx bdsize;
    const char * id = (const char *)DB.Getbody(idxType_id,idIndex,&bdsize);
    if (id){
        myId.printf(0,"%s",id);
    }
}

idx sVioAnnot::getNberOfIdsByRangeIndex(idx rangeIndex){
    idx relationCnt;
    idx relationTypeIndex;

    DB.GetRelationPtr(idxType_range,rangeIndex,1,&relationCnt,&relationTypeIndex);
    return relationCnt;
}

void sVioAnnot::getIdTypeByRangeIndexAndIdIndex(idx rangeIndex, idx idIndex, const char ** idPtr, idx * pIdSize, const char ** idTypePtr, idx * pIdTypeSize )
{
    idx relationTypeIndex, relationCnt;
    idx * indexPtrToId = DB.GetRelationPtr(idxType_range,rangeIndex,1,&relationCnt,&relationTypeIndex);
    const char * id = (const char *) DB.Getbody(relationTypeIndex,indexPtrToId[idIndex],pIdSize);
    idx cnt = sString::cnt00(id);
    const char * idtype;
    if (cnt<2){
        idx * indexPtrToIdType = DB.GetRelationPtr(idxType_id,indexPtrToId[idIndex],2,&relationCnt,&relationTypeIndex);
        idtype = (const char * ) DB.Getbody(idxType_idType,*indexPtrToIdType,pIdTypeSize);
    }
    else idtype = sString::next00(id,1);
    if(idPtr)*idPtr=id;
    if(idTypePtr)*idTypePtr=idtype;
}

sVioAnnot::startEnd * sVioAnnot::getRangeJointsByRangeIndex(idx rangeIndex,idx * cntRangeJoints){
    return (startEnd *)DB.Getbody(idxType_range,rangeIndex,cntRangeJoints);
}





idx * sVioAnnot::getIdByIdType(const char * idTypeToUse, idx * cntId){
    idx index = DB.GetRecordIndexByBody((const void *)idTypeToUse,idxType_idType,sLen(idTypeToUse)+1);
    if (index){
        idx relationTypeIndex =0;
        idx *indexPtrFromIdTypeToId = DB.GetRelationPtr(idxType_idType,index,1,cntId,&relationTypeIndex);
        return indexPtrFromIdTypeToId;
    }
    cntId = 0;
    return 0;
}



idx * sVioAnnot::getNumberOfRangesByIdTypeAndId(const char * idTypeToUse, const char * idToUse, idx & cntRanges){

    sStr hashLookUpBuf("%s",idToUse);
    hashLookUpBuf.add0(1);
    hashLookUpBuf.printf("%s",idTypeToUse);

    idx indexId = DB.GetRecordIndexByBody((const void *)hashLookUpBuf.ptr(),idxType_id,hashLookUpBuf.length()+1);

    idx * indexPtrFromIdToRange = DB.GetRelationPtr(idxType_id,indexId,1,&cntRanges,0);
    if (indexPtrFromIdToRange) {
        return indexPtrFromIdToRange;
    }
    return 0;
}



idx * sVioAnnot::getNumberOfRangesByIdType(const char * idTypeToUse, idx &cntRanges){

    idx indexId = DB.GetRecordIndexByBody((const void *)idTypeToUse,idxType_idType,sLen(idTypeToUse)+1);

    idx * indexPtrFromIdToRange = DB.GetRelationPtr(idxType_idType,indexId,1,&cntRanges,0);
    if (indexPtrFromIdToRange) {
        return indexPtrFromIdToRange;
    }
    return 0;
}


idx sVioAnnot::getAllIdTypes(sStr & buf, sVec<sMex::Pos> * bufposes) {
    idx rcrdcnt = DB.GetRecordCnt(idxType_idType);
    idx bufposesstart = bufposes ? bufposes->dim() : 0;
    if (bufposes) {
        bufposes->add(rcrdcnt);
    }
    for (idx ii=1; ii<rcrdcnt+1; ++ii) {
        idx bdystart = buf.length();
        idx bdysize = 0;
        const char * body = (char *) DB.Getbody(idxType_idType, ii, &bdysize);
        if (bdysize) {
            buf.add(body, bdysize);
        }
        buf.add0();
        if (bufposes) {
            bufposes->ptr(bufposesstart + ii - 1)->pos = bdystart;
            bufposes->ptr(bufposesstart + ii - 1)->size = bdysize;
        }
    }
    return rcrdcnt;
}





void sVioAnnot::cleanIdFromProfiler(const char * idToClean, sStr & idOut)
{
    sStr locusB; locusB.cut(0);

    if (strncmp(idToClean,">gi|",4)==0){
        sString::extractSubstring(&idOut,idToClean,0, 1, ">gi|" _ "|" __, true , true);
    }
    else if (strncmp(idToClean,"gi|",3)==0){
            sString::extractSubstring(&idOut,idToClean,0, 1, "gi|" _ "|" __, true , true);
        }
    else if (strncmp(idToClean,">",1)==0) {
        sString::extractSubstring(&idOut,idToClean,0, 1, ">" _ " " __, true , true);
    }
    else {
        sString::searchAndReplaceSymbols(&locusB, idToClean, 0, " ", 0, 0, true, true, false, true);
        sStr loc;loc.cut(0);
        sString::searchAndReplaceSymbols(&loc, locusB.ptr(0), 0, ".", 0, 0, true, true, false, true);
        idOut.printf(0,"%s",loc.ptr(0));
    }
}

idx sVioAnnot::sortVioAnnotFileRangesBasedOnSeqId(sVioAnnot * myAnnot){
    if( !myAnnot->isok() ) {
      return 0;
    }
    ParamsRangeSorter Param;
    Param.vioannot=myAnnot;

    idx cntId=0;
    idx * idPtr = myAnnot->getIdByIdType("seqID",&cntId);
    sDic < idx > doneSeqIDs;

    for (idx iid=0; iid< cntId; ++iid){
      idx cntRanges= 0;
      sStr myId;
      myAnnot->getIdByIdIndex(myId, idPtr[iid]);

      if(doneSeqIDs.find(myId))
          continue;

      idx * indexRelationPtrToRange = myAnnot->getNumberOfRangesByIdTypeAndId("seqID",myId.ptr(), cntRanges);
     if (!indexRelationPtrToRange) {
          continue;
      }
      myAnnot->DB.vioDBRelationSorter(indexRelationPtrToRange, VioAnnotRangeComparator , &Param, idxType_id, idxType_range , idPtr[iid], cntRanges );
      myAnnot->FixUpMaxVirtualTree(indexRelationPtrToRange,cntRanges,&myAnnot->DB);
      doneSeqIDs[myId.ptr(0)]=1;

    }
    return cntId;
}


idx sVioAnnot::printInformationBasedOnIdAndIdType(const char * idTypeToSearch, const char * idToSearch, sVec < sStr > & whatToPrint, sStr & outPut, idx & nbOfLinePrinted, idx start, idx end, idx cnt, bool combineIdIdType){
    idx * indexRangePtr = 0;
    idx cntRanges = 0;
    if (idTypeToSearch && idToSearch && *idToSearch!=0){
        indexRangePtr = getNumberOfRangesByIdTypeAndId(idTypeToSearch, idToSearch, cntRanges);
    }
    else if (idTypeToSearch && (!idToSearch || *idToSearch ==0)){
        indexRangePtr = getNumberOfRangesByIdType(idTypeToSearch, cntRanges);
    }
    if( !indexRangePtr || cntRanges == 0 )
        return 0;
    sDic<sStr> idTypeAndId;
    sDic <idx> alreadyPrinted;
    for(idx irange = 0; irange < cntRanges; ++irange) {
        idTypeAndId.empty();
        idx cntIDsForRange = getNberOfIdsByRangeIndex(indexRangePtr[irange]);
        bool startPrinted = false;
        bool print = true;
        bool endPrinted = false;
        bool chromosomePrinted = false;
        idx cntRangeJoints;
        startEnd * rangePtr = getRangeJointsByRangeIndex(indexRangePtr[irange], &cntRangeJoints);

        for(idx iid = 0; iid < cntIDsForRange; ++iid) {
            const char * idPtr, *idTypePtr;
            getIdTypeByRangeIndexAndIdIndex(indexRangePtr[irange], iid, &idPtr, 0, &idTypePtr, 0);

            for(idx ii = 0; ii < whatToPrint.dim(); ++ii) {
                if( strcasecmp(whatToPrint[ii].ptr(0), idTypePtr) == 0 ) {
                    sStr * myId = idTypeAndId.set(whatToPrint[ii].ptr(0));
                    if (combineIdIdType && strcasecmp(idTypePtr, "seqID") !=0){
                        myId->printf(0, "\"%s\" %s",idTypePtr, idPtr);
                    }else{
                        myId->printf(0, "%s", idPtr);
                    }
                    break;
                } else if( strcasecmp(whatToPrint[ii].ptr(0), "chromosome") == 0 && strcasecmp(idTypePtr, "seqID") == 0 && chromosomePrinted == false ) {
                    sStr * myId = idTypeAndId.set(whatToPrint[ii].ptr(0));
                    myId->printf(0, "%s", idPtr);
                    chromosomePrinted = true;
                    break;
                } else if( strcasecmp(whatToPrint[ii].ptr(0), "rangestart") == 0 && startPrinted == false ) {
                    if (rangePtr[0].start < start) {
                        print = false;
                        break;
                    }
                    sStr * myId = idTypeAndId.set(whatToPrint[ii].ptr(0));
                    myId->printf(0, "%" DEC "", rangePtr[0].start);
                    startPrinted = true;
                } else if( strcasecmp(whatToPrint[ii].ptr(0), "rangeend") == 0 && endPrinted == false ) {
                    sStr * myId = idTypeAndId.set(whatToPrint[ii].ptr(0));
                    myId->printf(0, "%" DEC "", rangePtr[0].end);
                    if (rangePtr[0].end > end) {
                       print = false;
                       break;
                    }
                    endPrinted = true;
                }
            }
            if (print == false) break;
        }
        if (print == false) continue;
        sStr lineToPrint, cellToPrint;
        bool notSpecial = false;
        for(idx ii = 0; ii < whatToPrint.dim(); ++ii) {
            if (strcasecmp(whatToPrint[ii].ptr(0), "rangeend")!=0 && strcasecmp(whatToPrint[ii].ptr(0), "rangestart") != 0 && strcasecmp(whatToPrint[ii].ptr(0), "chromosome") != 0 && strcasecmp(whatToPrint[ii].ptr(0), "seqID") != 0){
                notSpecial = true;
            }
            if( idTypeAndId.find(whatToPrint[ii].ptr()) ) {
                sStr * idValue = idTypeAndId.get(whatToPrint[ii].ptr());
                cellToPrint.printf("%s", idValue->ptr());
            } else {
                cellToPrint.printf("-");
            }
            if( ii < whatToPrint.dim()-1){
                if (combineIdIdType && notSpecial){
                    cellToPrint.printf(";");
                } else {
                    if (cellToPrint.length()) {
                        sString::escapeForCSV(lineToPrint, cellToPrint.ptr(), cellToPrint.length());
                    }
                    cellToPrint.cut(0);
                    lineToPrint.printf(",");
                }
            }
            notSpecial = false;
        }
        if (cellToPrint.length()) {
            sString::escapeForCSV(lineToPrint, cellToPrint.ptr(), cellToPrint.length());
        }
        if (alreadyPrinted.find(lineToPrint.ptr(0))) continue;
        outPut.printf("%s\n", lineToPrint.ptr());
        alreadyPrinted[lineToPrint.ptr(0)] = 1;
        nbOfLinePrinted += 1;
        if (nbOfLinePrinted >cnt) break;
    }
    return 1;
}

idx sVioAnnot::printInformationBasedOnIdTypeList(const char * sourceFileName,const char * refSeqID,sVec < sStr > & idTypeFilterList, sStr & outPut, idx & nbOfLinePrinted, idx pos_start, idx pos_end, idx page_start,idx page_end,idx cnt){

    idx ligneNumber = 0;
    idx cntId = 0;
    idx * idPtr = getIdByIdType("seqID",&cntId);
    sStr seqID;
    sDic <idx> seqIDAlreadyParsed;

    for( idx is=0; is< cntId; ++is) {
       seqID.cut(0);
       getIdByIdIndex(seqID,idPtr[is]);
       if (refSeqID && sLen(refSeqID) >0){
           if (strcasecmp(refSeqID,seqID.ptr(0)) !=0 ) continue;
       }
       if (seqIDAlreadyParsed.find(seqID.ptr())) continue;
       seqIDAlreadyParsed[seqID.ptr()] = 1;
       idx * indexRangePtr = 0;
       idx cntRanges = 0;

       indexRangePtr = getNumberOfRangesByIdTypeAndId("seqID",seqID, cntRanges);
       sDic <idx> idTypeFilterDic;
       for (idx i=0; i< idTypeFilterList.dim(); ++i){
           idTypeFilterDic[idTypeFilterList[i].ptr()] = 1;
       }

       sDic<sStr> idTypeAndId;
       sDic <idx> alreadyPrinted;
       for(idx irange = 0; irange < cntRanges; ++irange) {
           idTypeAndId.empty();
           idx cntIDsForRange = getNberOfIdsByRangeIndex(indexRangePtr[irange]);

           bool print = true;
           idx found = 0;

           idx cntRangeJoints;
           startEnd * rangePtr = getRangeJointsByRangeIndex(indexRangePtr[irange], &cntRangeJoints);

           for(idx iid = 0; iid < cntIDsForRange; ++iid) {
               const char * idPtr, *idTypePtr;
               getIdTypeByRangeIndexAndIdIndex(indexRangePtr[irange], iid, &idPtr, 0, &idTypePtr, 0);
               if (!idPtr && !idTypePtr) continue;
               if(idTypeFilterDic.find(idTypePtr) && sLen(idPtr) ==0 ){
                   print = false;
               }
               if (print==false) break;
               if (idTypeFilterDic.find(idTypePtr)){
                   found += 1;
               }
               idTypeAndId.set(idTypePtr)->printf("%s",idPtr);

           }
           if (found < idTypeFilterDic.dim()) print = false;
           if (rangePtr[0].start < pos_start || rangePtr[0].end > pos_end){
               print = false;
           }
           if (print == false) continue;

           sStr lineToPrint;


           if (sourceFileName && strcasecmp(sourceFileName,"gwatch")==0){
              for(idx ii = 0; ii < idTypeAndId.dim(); ++ii) {

                  const char * key = (const char *)idTypeAndId.id(ii);
                  sStr * idValue = idTypeAndId.get(key);
                  lineToPrint.cut(0);
                  lineToPrint.printf("%" DEC ",%" DEC ",%s,1,%s,%s",rangePtr[0].start, rangePtr[0].end,key,idValue->ptr(0),seqID.ptr(0));
                  if (alreadyPrinted.find(lineToPrint.ptr(0))) continue;
                  outPut.printf("%s\n", lineToPrint.ptr());
                  alreadyPrinted[lineToPrint.ptr(0)] = 1;
                  nbOfLinePrinted += 1;
              }
           }
           else {
               lineToPrint.cut(0);
               lineToPrint.printf("%s,%" DEC ",%" DEC ",%s,",seqID.ptr(),rangePtr[0].start, rangePtr[0].end,sourceFileName);
               sStr cellToPrint;
               cellToPrint.cut(0);
               for(idx ii = 0; ii < idTypeAndId.dim(); ++ii) {
                   if (ii>0) cellToPrint.printf(";");
                   const char * key = (const char *)idTypeAndId.id(ii);
                   sStr * idValue = idTypeAndId.get(key);
                   cellToPrint.printf("\"%s\" %s", key, idValue->ptr());
               }
               if (cellToPrint.length()) {
                     sString::escapeForCSV(lineToPrint, cellToPrint.ptr(), cellToPrint.length());
               }

               if (alreadyPrinted.find(lineToPrint.ptr(0))) continue;
               ligneNumber += 1;
               if (ligneNumber < page_start) continue;
               outPut.printf("%s\n", lineToPrint.ptr());
               alreadyPrinted[lineToPrint.ptr(0)] = 1;
               nbOfLinePrinted += 1;
               if (nbOfLinePrinted >cnt || nbOfLinePrinted > (page_end - page_start)) break;
           }
       }
    }
    return 1;
}

idx sVioAnnot::runBumperEngine(const char * contentInCSVFormat,idx sourceLength,sStr & tableOut, idx _referenceStart, idx _referenceEnd, idx _width, idx _resolution, idx _annotationDensity, idx _maxLayers){

    sTxtTbl * tbl = new sTxtTbl();
    tbl->setBuf(contentInCSVFormat, sourceLength, 0);
    tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fColsep00;
    tbl->parseOptions().colsep = "," __;
    tbl->parseOptions().comment = 0;
    tbl->parse();

    Bumper myBumper(_referenceStart,_referenceEnd,_width, _resolution, _annotationDensity, _maxLayers);
    sStr mySeqID, mySrc,myStart,myEnd, myIdTypeId;

    idx error = 0;
    for (idx ir=0; ir< tbl->rows(); ++ir){

        mySeqID.cut(0); mySrc.cut(0); myStart.cut(0); myEnd.cut(0), myIdTypeId.cut(0);

        tbl->printCell(mySeqID, ir, 0, 0, "");
        tbl->printCell(myStart, ir, 1, 0, "");
        tbl->printCell(myEnd, ir, 2, 0, "");
        tbl->printCell(mySrc, ir, 3, 0, "");
        tbl->printCell(myIdTypeId, ir, 4, 0,"", sTxtTbl::fForCSV);

        idx start =0, end=0;
        sscanf(myStart,"%" DEC,&start);
        sscanf(myEnd,"%" DEC,&end);
        if ( end>_referenceEnd || start<_referenceStart) continue;
        idx _error = myBumper.add(start,end,mySrc,mySeqID, myIdTypeId);
        if (_error > error) error = _error;
    }
    myBumper.print(tableOut);
    return error;
}
idx sVioAnnot::VioAnnotRangeComparator(void * parameters, void * A, void * B,void * objSrc,idx i1,idx i2 )
{
    startEnd * rangeA= (startEnd * )A, * rangeB=(startEnd * )B;

    return rangeA->start-rangeB->start;

}



struct ParamsRangeSorter{
    idx flags;
    sVioAnnot * vioannot;
    ParamsRangeSorter(){
        flags=0;
        vioannot=0;
    }
};

idx sVioAnnot::deBrLUT[64]={
    0,  1,  2, 53,  3,  7, 54, 27,
    4, 38, 41,  8, 34, 55, 48, 28,
   62,  5, 39, 46, 44, 42, 22,  9,
   24, 35, 59, 56, 49, 18, 29, 11,
   63, 52,  6, 26, 37, 40, 33, 47,
   61, 45, 43, 21, 23, 58, 17, 10,
   51, 25, 36, 32, 60, 20, 57, 16,
   50, 31, 19, 15, 30, 14, 13, 12,
};

idx sVioAnnot::getLSBindex1(idx & a){
    return deBrLUT[ (a&-a)*deBrSeq ];
}

idx sVioAnnot::getRoot(idx depth){
    idx root=0;
    return root|(1<<depth);
}

idx sVioAnnot::getParent(idx x, idx * level){
    if(!level){
        *level=getLSBindex1(x);
    }
    x&=~(3<<*level);
    x|=2<<*level;
    return x;
}

idx sVioAnnot::getLeft(idx x, idx * level){
    if(!level){
        *level=getLSBindex1(x);
    }
    if(*level<=1)return 0;
    x&=~(1<<(*level-1));
    x|=(1<<(*level-2));
    return x;
}

idx sVioAnnot::getRight(idx x,idx * level){
    if(!level){
        *level=getLSBindex1(x);
    }
    if(*level<=1)return 0;
    x|=(1<<(*level-2));
    return x;
}

void sVioAnnot::FixUpMaxVirtualTree(idx * relPtr,idx bodysize,sVioDB * DB)
{
    idx depth=floor(log10(bodysize)/log10(2));
    idx virtualSize=pow(2,depth+1)-1;

    idx rgType = getRangeTypeIdx();
    idx current = 0, bodysizeC=0, parent = 0, bodysizeP=0;
    startEnd * rangeC = 0, * rangeP = 0;
    for(idx i=1; i<virtualSize; i+=2){
        idx x=i, level=0;
        current = relPtr[x-1];bodysizeC=0;
        rangeC = ( startEnd*)DB->Getbody(rgType,current,&bodysizeC);
        while( level<depth ) {
            x=getParent(x,&level);
            if(x>bodysize){ ++level;continue;}
            parent = relPtr[x-1];bodysizeP=0;
            rangeP = ( startEnd*)DB->Getbody(rgType,parent,&bodysizeP);
            if(!rangeC || !rangeP){::printf("ERROR\n");break;}
            if(!rangeC->max) rangeC->max = rangeC->end;
            if(!rangeP->max) rangeP->max = rangeP->end;
            if(rangeC[0].max>rangeP[0].max)
                rangeP[0].max=rangeC[0].max;
            ++level;
            current = parent; bodysizeC=0;
            rangeC = rangeP;
        }
    }
}

idx sVioAnnot::searchInVirtualTree(idx * relPtr,idx bodysize,sVec<startEndNode> &results,idx start,idx end )
{
    idx rgType = getRangeTypeIdx();
    if(start<0)start=0;
    if(!end || end<start)end=start;
    idx depth=floor(log10(bodysize)/log10(2)),rangeBodySize=0;
    vtreeNode curr;curr.ind=getRoot(depth);curr.level=depth+1;curr.range=0;

    sVec<vtreeNode> stack;
    stack.vadd(1,curr);
PERF_START("Loop inside Tree");
    while(stack.dim()){
        curr=*stack.ptr(stack.dim()-1);
        stack.cut(stack.dim()-1);

        if(curr.ind>bodysize){
            if(curr.level>0){
                vtreeNode next;
                next.ind=getLeft(curr.ind,&curr.level);
                next.level=curr.level-1;
                stack.vadd(1,next);
            }
            continue;
        }
curr.range= ( startEnd*)DB.Getbody(rgType,relPtr[curr.ind-1]?relPtr[curr.ind-1]:1,&rangeBodySize);

        idx subRangesCnt= (rangeBodySize/ sizeof(startEnd));

        if(sOverlap(start,end,curr.range[0].start,curr.range[subRangesCnt?subRangesCnt-1:subRangesCnt].end)){
            startEndNode node;
            node.ranges=curr.range;
            node.index=relPtr[curr.ind-1];
            node.subRangesCnt= subRangesCnt;
            results.vadd(1,node);

        }

        if(curr.level>1){
            vtreeNode nextL;
            nextL.ind=getLeft(curr.ind,&curr.level);
            nextL.level=curr.level-1;
            if(nextL.ind<bodysize){
                nextL.range = ( startEnd*)DB.Getbody(rgType,relPtr[nextL.ind-1],0);
                if(start<=nextL.range->max){
                    stack.vadd(1,nextL);
                }
            }
            else
                stack.vadd(1,nextL);

            vtreeNode nextR;
            nextR.ind=getRight(curr.ind,&curr.level);
            nextR.level=curr.level-1;
            if(nextR.ind<bodysize){
                nextR.range = ( startEnd*)DB.Getbody(rgType,relPtr[nextR.ind-1],0);
                if(end >= curr.range->start && start<=nextR.range->max){
                    stack.vadd(1,nextR);
                }
            }
            else if(end >= curr.range->start)
                stack.vadd(1,nextR);
        }

    }
PERF_END();
    return results.dim();
}


bool sVioAnnot::printRangeSetSearch( sVec<idx> &startV, sVec<idx> &endV, idx recordIndex, searchOutputParams & params)
{
    if( endV.dim()!=startV.dim() || !startV.dim() ) {
        return false;
    }
    sStr * out = params.outBuf;
    if( !out ) {
        return false;
    }
    idx typeIdIdx = getIdTypeIdx();
    idx start = 0 , end = 0, relationCnt= 0, relationTypeIndex = 0, resultSize = 0;
    sVec<startEndNode> resStruct;

    idx * indexPtrRange= DB.GetRelationPtr(typeIdIdx, recordIndex, 1,&relationCnt,&relationTypeIndex);
    sStr id_pos,myId;

    getIdByIdIndex(myId, recordIndex);

    for( idx i = 0 ; i < startV.dim() ; ++i ) {
        start = startV[i]; end = endV[i];
        resultSize = searchInVirtualTree(indexPtrRange,relationCnt,resStruct,start,end);
        id_pos.printf(0,"%" DEC "-%" DEC,start,end);

        if(resultSize) {
            if( (params.rowParams&ePrintSingleHitRow) ) {
                if( params.rowParams&ePrintIDpos ) {
                    params.outBuf->printf("%s",id_pos.ptr());
                    params.outBuf->printf("%s",params.column_delim);
                }
                if( params.rowParams&ePrintSeqID ) {
                    params.outBuf->printf("\"%s\"", myId.ptr(0));
                    params.outBuf->printf("%s",params.column_delim);
                }
            }
            else{
                params.miscParams = (void *)id_pos.ptr();
            }
        }


        for (idx rs=0; rs<resultSize; rs++){
            if( !printSingleRangeSearch(resStruct[rs], params,myId.ptr() ) ) {
                return false;
            }
            if( (params.rowParams&ePrintSingleHitRow) ) {
                out->printf("%s", params.column_delim);
            }
            else {
                out->printf("%s", params.row_delim);
            }
        }
        if(resultSize) {
            out->cut(out->length()-1);
        }
        out->printf("%s", params.row_delim);
    }
    out->cut(out->length()-1);
    return true;
}

bool sVioAnnot::printSingleRangeSearch( startEndNode &range,searchOutputParams & params, const char * seqID)
{
    idx cntIDsForRange=getNberOfIdsByRangeIndex(range.index);
    bool not_empty = false;
    sStr * out = params.outBuf;
    if( !out ) {
        return false;
    }
    idx output_format = params.rowParams;

    if( cntIDsForRange && ( (params.rowParams&ePrintSingleAnnotRangeRow) ) ) {
        if( output_format&ePrintAnnotRange) {
            out->printf("%" DEC, range.ranges->start);
            if( output_format&ePrintAnnotRangeInOneColumn ) {
                out->printf(" - ");
            }
            else {
                out->printf("%s",params.column_delim);
            }
            out->printf("%" DEC, range.ranges->end);
            out->printf("%s",params.column_delim);
        }
    }

    for( idx i = 0; i < cntIDsForRange; ++i)  {
        const char * idPtr,*idTypePtr;
        getIdTypeByRangeIndexAndIdIndex(range.index, i, &idPtr, 0, &idTypePtr, 0);
        if( seqID && !strcmp(idPtr,seqID) ) {
            continue;
        }

        if( !printSingleAnnotation ( idPtr, idTypePtr, params, range.ranges->start, range.ranges->end, seqID) ) {
            return false;
        }
        if( (params.rowParams&ePrintSingleAnnotRangeRow) || (params.rowParams&ePrintSingleHitRow) ) {
            out->printf("%s", params.column_delim);
        }
        else {
            out->printf("%s", params.row_delim);
        }

        not_empty = true;
    }
    if( not_empty ) {
        out->cut(out->length()-1);
    }
    return not_empty;
}

bool sVioAnnot::printSingleAnnotation ( const char * id, const char * id_type, searchOutputParams & params, idx start, idx end, const char * seqID )
{
    sStr * out = params.outBuf;
    if( !out ) {
        return false;
    }
    idx output_format = params.rowParams;
    if( (output_format&ePrintIDpos) && !((params.rowParams&ePrintSingleHitRow) || (params.rowParams&ePrintSingleAnnotRangeRow)) ) {
        const char * id_pos = (const char *)params.miscParams;
        if( !id_pos ) {
            return false;
        }
        out->printf("%s", id_pos);
        out->printf("%s",params.column_delim);
    }
    if( (output_format&ePrintSeqID) && !((params.rowParams&ePrintSingleHitRow) || (params.rowParams&ePrintSingleAnnotRangeRow)) ) {
        out->printf("\"%s\"", seqID?seqID:"-");
        out->printf("%s",params.column_delim);
    }
    if( (output_format&ePrintAnnotRange) && !(params.rowParams&ePrintSingleAnnotRangeRow)) {
        out->printf("%" DEC, start);
        if( output_format&ePrintAnnotRangeInOneColumn ) {
            out->printf(" - ");
        }
        else {
            out->printf("%s",params.column_delim);
        }
        out->printf("%" DEC, end);
        out->printf("%s",params.column_delim);
    }
    out->printf("\"%s\"",id);
    out->printf("%s",params.column_delim);
    out->printf("%s",id_type);

    return true;
}



idx * sVioAnnot::getRangesForGivenIDAndIDType(const char * srchID, const char * idtype, idx * pRelationCnt)
{
    idx indexLocus = DB.GetRecordIndexByBody((const void *)srchID, generalLocus_TYPE, sLen(srchID)+1 );

    idx relationTypeIndex;
    idx * indexPtr =  DB.GetRelationPtr(generalLocus_TYPE, indexLocus, 2,pRelationCnt,&relationTypeIndex);
    return indexPtr;
}

sVioAnnot::startEnd * sVioAnnot::getRangesByIndex(idx rangeIndex, idx * pDimension)
{
    startEnd * seList = (startEnd *) DB.Getbody(range_TYPE, rangeIndex, pDimension);
    if(pDimension && seList)
        *pDimension/=sizeof(startEnd );
    return seList;
}


