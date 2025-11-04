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
#include <slib/std.hpp>

#define  SIZEBUF (1024*4)
struct GBFloorDescription;

    struct GBFloorDescription{
        const char * floorTag;
        GBFloorDescription * children;
        bool specificTreatment;
    };

    GBFloorDescription sCDS[]={
        {"/gene=",0,true},
      {"/product=",0,true},
      {"/translation=",0,true},
        {"/",0,false},
        {0,0}
    };


    GBFloorDescription sFeatures[]={
        {"CDS",sCDS,true},
        {0,0}
    };
  GBFloorDescription searchSystem[]={
        {"LOCUS",0,true},
        {"DEFINITION",0,true},
        {"FEATURES",sFeatures,false},
        {0,0}
    };

    struct GBdesc{
        sStr locus,start,end;
        sStr curTag, curValue, curPath, curLocus;
    };
    GBdesc gbd;
    udx locusNum=0,cdsNum=0,tagNum=0;

void printUsage() { ::printf("\nError: no input file specified\n Usage: genbankParser [inputFileName]\n e.g. genbankParser duplicategenbank.txt\n\n"); }


const char * getonelinefromBuffer(char * buf, const char * fileContent)
{
    int i;
    if(!fileContent[0])
    {
        return fileContent;
    }
    for (i=0;fileContent[i]!='\n' ; ++ i)
        buf[i]=fileContent[i];
    buf[i]='\0';
    return fileContent+i+1;
}



void gbdDumper(const char * locus, const char * tag, const char * path, const char *value, sStr & lineEx)
{
    sStr buf; sString::searchAndReplaceSymbols(&buf,tag,0,"/=","",0,true,true,true,true);
    const char * clTag =buf.ptr();

    sStr vbuf; vbuf.printf("%s",value);
    if(vbuf.length())sString::cleanEnds(vbuf.ptr(), 0,"\"",true);
    const char * clValue =vbuf.ptr();

    sStr sLocus,sTag,sPath,sValue;
    sString::escapeForCSV(sLocus,locus);
    sString::escapeForCSV(sTag,clTag);
    sString::escapeForCSV(sPath,path);
    sString::escapeForCSV(sValue,clValue);

    lineEx.printf("%s,%s,%s,%s\n",sLocus.ptr(),sTag.ptr(),sPath.ptr(),sValue.ptr());
}

void specificTreatmentFunction ( const char * tagName, const char * value, bool isContinued, sStr & lineExp )
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

    if(strcmp(tagName,"CDS")==0 ){
        ++cdsNum; tagNum=1;
        bool complement = false;
        for (udx i=0; i<strlen(text);++i) {
            if(strncmp(text+i,"complement",10)==0 ){ complement = true; }
        }
        if (complement) {
            buf.cut(0);
            sString::cleanMarkup(&buf,text,0,"complement(" __,")" __,"" __,0,true,false,false);
            text = buf.ptr(1);
        }
        idx i=0,delim=0;

        while (text[i] != '.') {i++;} delim=i;
        gbd.start.cut(0); gbd.start.resize(delim);
        strncpy(gbd.start.ptr(),text,delim);
        gbd.start.add0();

        gbd.end.cut(0);
        gbd.end.resize(sizeof(text+gbd.start.length()+1));
        strcpy(gbd.end.ptr(),text+gbd.start.length()+1);
        gbd.end.add0();
    }
    else ++tagNum;

    if(gbd.curValue.length()!=0) {
        path.cut(0); path.printf("%" DEC ".%" DEC ".%" DEC,locusNum,cdsNum,tagNum);
        if (strcmp(gbd.curTag.ptr(),"CDS")==0) {
            lineExp.printf("%s,%s,%s,%s,%s\n",gbd.locus.ptr(),"range", gbd.curPath.ptr(), gbd.start.ptr(),gbd.end.ptr());
        }
        else if (gbd.curPath.length()>1) {
            gbdDumper(gbd.curLocus.ptr(),gbd.curTag.ptr(), gbd.curPath.ptr(), gbd.curValue.ptr(),lineExp);
             }
             else {
                 gbd.curPath.cut(0); gbd.curPath.printf("%" DEC ".%d.%" DEC,locusNum,0,tagNum-1);
                 gbdDumper(gbd.locus.ptr(),gbd.curTag.ptr(), gbd.curPath.ptr(), gbd.curValue.ptr(),lineExp);
             }
    }

    gbd.curLocus.printf(0,"%s",gbd.locus.ptr());
    gbd.curPath.printf(0,"%s",path.ptr());
    gbd.curTag.printf(0,"%s", tagName);
    gbd.curValue.printf(0,"%s",text);

}

bool extractedInfo(struct GBFloorDescription * searchingLayers, char * mystring, int FloorNum[],int & currentFloor ,int & FoundLineNonBlankPos, int & quoteCount, bool & continueationMode, int & line_num, int & find_result,sStr & lineLine){

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
           else if( nonBlankPos <20 && FloorNum[0]!=-1 ) {
               searchingLayers=searchSystem[FloorNum[0]].children;
               currentFloor=1;
           }
           else if( nonBlankPos >20 && FloorNum[1]!=-1) {

               currentFloor=2;
               searchingLayers=searchSystem[FloorNum[0]].children[FloorNum[1]].children;
           }
           else {
               return false;
           }

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

       }else
           {continueationMode=true;
               if( nonBlankPos >20 && FloorNum[1]!=-1){
                   searchingLayers=searchSystem[FloorNum[0]].children[FloorNum[1]].children;
               }
           }
       for ( int i=0; mystring[i]!=0; ++i) if(mystring[i]=='\"')++quoteCount;
       const char * tag=searchingLayers[FloorNum[currentFloor]].floorTag;

       specificTreatmentFunction(tag, mystring+nonBlankPos+( continueationMode==false ? strlen(tag) : 0 ) , continueationMode,lineLine);
       return true;
}

int main(int argc, char *argv[])
{
    GBFloorDescription searchingLayers;
    int FloorNum[10] ,FoundLineNonBlankPos=0,quoteCount=0;
    int currentFloor=0;
    bool continueationMode=false;
    for ( int i=0; i< 10; ++i)FloorNum[i]=-1;


    char mystring[SIZEBUF];
    int line_num=1;
    int find_result=0;
    bool success;

    sFil srcFile(argv[1],sMex::fReadonly);
    const char * src=srcFile.ptr();
    idx length=srcFile.length();

        for (idx l=0;l<length && *src;l++)

        {
            src=getonelinefromBuffer(mystring,src);

            sStr extractedLine;

            success = extractedInfo(&searchingLayers, mystring,FloorNum, currentFloor ,FoundLineNonBlankPos, quoteCount,continueationMode,line_num,find_result, extractedLine);

           if (!success){
               success = true;
               continue;
           }
           if (extractedLine.length()>6){
                sStr lineSeparByZero;
                sString::searchAndReplaceSymbols(&lineSeparByZero,extractedLine,0,",",0,0,true,true,false,true);
                ::printf("%s\n",extractedLine.ptr());
            }
        }

    return (0);
}
