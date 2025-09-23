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
#include <ssci/bio/ncbitax.hpp>
#include <regex.h>

#define TAXID_TYPE 1
#define RANK_TYPE 2
#define NAME_TYPE 3 
#define Project_TYPE 4 

const char *sNCBITaxTree::columnNames =
    "taxid" _
    "parentid" _
    "name" _
    "rank" _
    "pathname" _
    "idpath" _
    "childrenCnt" _
    "matchname" _
    "allname" _
    "bioprojectID" _
    "path" _
    "matchCnt" _
    "num" _
    "min" _
    "max" _
    "mean" _
    "stddev" _
    "intval" __;

const char * sNCBITaxTree::printColumnNames(sStr & out, idx whatToPrint)
{
    idx startLen = out.length();
    idx flag = 1;
    for(const char * colName = columnNames; colName; colName=sString::next00(colName)) {
        if(whatToPrint & flag) {
            if(out.length() > startLen)
                out.add(",", 1);
            out.printf("%s", colName);
        }
        flag <<= 1;
    }
    return out.ptr(startLen);
}

idx sNCBITaxTree::getNodeIndex(idx taxid){
    if( taxid <= 0) return 0;
    else return DB.GetRecordIndexByBody((void *)&taxid,TAXID_TYPE);
}

idx sNCBITaxTree::getTaxIdCnt(void){
    sVioDB::TypeList * typelist_type=DB.GetTypePointer(TAXID_TYPE);
    return typelist_type->cnt;
}

idx sNCBITaxTree::getNodeIndexByProID(idx projectID){
    if( projectID <= 0) return 0;
    idx index= DB.GetRecordIndexByBody((void *)&projectID,Project_TYPE);
    if(index>0 && *DB.GetRelationPtr(Project_TYPE,index, 1, 0, 0)>0)
    return  *(idx *)DB.Getbody(TAXID_TYPE,*DB.GetRelationPtr(Project_TYPE,index, 1, 0, 0),0);
    else return 0;
}

bool sNCBITaxTree::getNameTaxid(sVec <idx> * TaxidList, const char * srch){
    bool IfFind=false;
    idx bodysize;
    idx relationCnt;
    idx relationTypeIndex;
    regex_t re;
    idx index;
    idx regerr = srch ? regcomp(&re, srch, REG_EXTENDED|REG_ICASE) : true;
    if(regerr || !srch || !*srch) return 0;
    else{
        for(idx i=1;i <= DB.GetRecordCnt(NAME_TYPE);i++){
            const char *body = (const char *) DB.Getbody(NAME_TYPE,i,&bodysize);
            bool match= (regexec(&re, body, 0, NULL, 0)==0) ? true : false;
            if(match){
                index= *DB.GetRelationPtr(NAME_TYPE, i, 1,&relationCnt,&relationTypeIndex);
                TaxidList->vadd(1,*(idx *) DB.Getbody(relationTypeIndex,index,&bodysize));
                IfFind=true;
            }
        }
    }
    return IfFind;
}

idx sNCBITaxTree::getChildCnt(idx taxid){
    idx whichOne=5;
    idx nodeIndex = sNCBITaxTree::getNodeIndex(taxid);
    if(nodeIndex)
    return DB.GetRelationCnt(TAXID_TYPE, nodeIndex, whichOne);
    return -1;
}

idx sNCBITaxTree::getParentCnt(idx taxid){
    idx nodeIndex = sNCBITaxTree::getNodeIndex(taxid);
    if(nodeIndex)
    return DB.GetRelationCnt(TAXID_TYPE, nodeIndex, 1);
    return -1;
}

idx sNCBITaxTree::getParentIndex(idx taxid){
    idx nodeIndex = sNCBITaxTree::getNodeIndex(taxid);
    if(!nodeIndex) return 0;
    idx relationCnt=0;
    idx relationType=0;
    return *DB.GetRelationPtr(TAXID_TYPE,nodeIndex, 1, &relationCnt, &relationType);
}    

bool sNCBITaxTree::getChildrenIndexList(sVec <idx> * lontosa, idx taxid, const char * rankFilters, idx depth)
{
    idx nodeIndex = sNCBITaxTree::getNodeIndex(taxid);
    idx ChildCnt = sNCBITaxTree::getChildCnt(taxid);
    if(ChildCnt && ChildCnt!=-1){
        idx cntChildren, relationType;
        idx * ChildrenIndexListPtr=DB.GetRelationPtr(TAXID_TYPE ,nodeIndex, 5, &cntChildren, &relationType);
        for(idx i=0; ChildrenIndexListPtr && i< ChildCnt; i++, ChildrenIndexListPtr++){
            idx IfAmongRanks=0;
            if(rankFilters && *rankFilters)
                IfAmongRanks=sString::compareChoice(getRankByIndex(*ChildrenIndexListPtr),rankFilters ,0,false,0,true);
            if(IfAmongRanks!=-1){
                lontosa->vadd(1,*ChildrenIndexListPtr);
            }
            if(depth-1>=1){
                idx bodysize;
                idx id=*(idx *)DB.Getbody(TAXID_TYPE,*ChildrenIndexListPtr,&bodysize);
                sNCBITaxTree::getChildrenIndexList(lontosa, id, rankFilters, depth-1);
            }
        }
        return true;
    }
    return false;
}

bool sNCBITaxTree::getParentIndexList(sVec <idx> * lontosa, idx taxid,  idx depth)
{
    idx index = sNCBITaxTree::getNodeIndex(taxid);
    idx parentid=0;
    if(!depth) depth = DB.GetRecordCnt(1);
    if(index && taxid!=1){
        while(depth-- > 0 && taxid!=1){
            parentid=sNCBITaxTree::getParentByIndex(index);
            index = sNCBITaxTree::getNodeIndex(parentid);
            taxid = parentid;
            lontosa->vadd(1,index);
        }
    }
    else return false;
    return true;
}

bool sNCBITaxTree::getPathByIndex(sStr * buf, idx index, idx WhatToPrintFlags,const char * RankFilters,bool ifPrintRoot){
    idx parentid=0;
    sVec <idx> TaxidList;
    idx taxid = getTaxIdByIndex(index);
    if(taxid==1){
        buf->printf("/all/");
        return true;
    }
    if(index){
        parentid=sNCBITaxTree::getParentByIndex(index);
        if(!parentid) return false;
        while(taxid!=1){
            idx IfAmongRanks=0;
            if(RankFilters && *RankFilters)
                IfAmongRanks=sString::compareChoice(getRankByIndex(index),RankFilters ,0,false,0,true);
            if(IfAmongRanks!=-1){
                TaxidList.vadd(1,index);
            }
            parentid=sNCBITaxTree::getParentByIndex(index);
            taxid = parentid;
            index = sNCBITaxTree::getNodeIndex(taxid);
        }
        if(ifPrintRoot)  TaxidList.vadd(1,index);

        buf->printf("/");
        for (idx i=TaxidList.dim()-1; i>=0;i--){
            idx childCnt = sNCBITaxTree::getChildCnt(sNCBITaxTree::getTaxIdByIndex(TaxidList[i]));

            if(WhatToPrintFlags & ePath){
                if(!sNCBITaxTree::getNameByIndex(buf, TaxidList[i],1))
                    return false;
            }
            else if(WhatToPrintFlags & ePathID){
                if(!sNCBITaxTree::getTaxIdByIndex(TaxidList[i])){
                    buf->printf(" ");
                    return false;
                }
                else buf->printf("%" DEC "",sNCBITaxTree::getTaxIdByIndex(TaxidList[i]));
            }
            else if(WhatToPrintFlags & eDefPath){
                if(sNCBITaxTree::getNameByIndex(buf, TaxidList[i],1)){
                    idx taxid = sNCBITaxTree::getTaxIdByIndex(TaxidList[i]);
                    if(!taxid){
                        buf->printf(" ");
                        return false;
                    }
                    else buf->printf(":%" DEC ":%" DEC "",taxid,childCnt);
                }

            }
            if(i ||childCnt ){
               buf->printf("/"); 
            }
        }
    }
    return true;
}

idx sNCBITaxTree::getParentByIndex(idx index){
    if(index==1) return 0;
    idx bodysize, relationCnt=0;
    idx relationType=0;
    idx * relationPtr = DB.GetRelationPtr(TAXID_TYPE ,index, 1, &relationCnt, &relationType);
    if(relationPtr)
        return *(idx *)DB.Getbody(relationType,*relationPtr,&bodysize);
    else return 0;
}

idx sNCBITaxTree::getTaxIdByIndex(idx index){
    idx bodysize;
    if(index)
    return *(idx *)DB.Getbody(TAXID_TYPE,index,&bodysize);
    else return 0;
}

bool sNCBITaxTree::IfNameMatch(idx index, const char * srch, idx * matchNameIndex){
    bool IfMatch=false;
    idx cntName = DB.GetRelationCnt(TAXID_TYPE,index,3);
    regex_t re;
    idx regerr = srch ? regcomp(&re, srch, REG_EXTENDED|REG_ICASE) : true;
    if(regerr) return false;
    for(idx i=1;i <= cntName && !IfMatch;i++){
        sStr t;
        sNCBITaxTree::getNameByIndex(&t, index,i);
        IfMatch= (regexec(&re, t.ptr(0), 0, NULL, 0)==0) ? true : false;
        *matchNameIndex = i;
    }
    return IfMatch;
}


idx sNCBITaxTree::getProjIDByIndex(idx index){
    idx bodysize, relationCnt=0;
    idx relationType=0;
    idx * relationPtr = DB.GetRelationPtr(TAXID_TYPE ,index, 4, &relationCnt, &relationType);
    if(relationPtr)
    return *(idx *)DB.Getbody(relationType,*relationPtr,&bodysize);
    else return 0;
}


idx sNCBITaxTree::getRankCount(const char * rank )
{
    idx relationCnt=0,relationType=0;
    idx indexOfRank=DB.GetRecordIndexByBody(rank,RANK_TYPE,sLen(rank)+1);
    DB.GetRelationPtr(RANK_TYPE,indexOfRank, 1, &relationCnt, &relationType);
    return relationCnt;
}

const char * sNCBITaxTree::getRankByIndex(idx index){
    idx bodysize, relationCnt=0;
    idx relationType=0;
    idx * relationPtr = DB.GetRelationPtr(TAXID_TYPE ,index, 2, &relationCnt, &relationType);
    if(relationPtr)
    return (char *)DB.Getbody(relationType,*relationPtr,&bodysize);
    else return 0;
} 

bool sNCBITaxTree::getNameByIndex(sStr * buf, idx index, idx whichName){
    idx bodysize, realCntNames=0;
    idx relationType=0;
    idx * relationPtr = DB.GetRelationPtr(TAXID_TYPE ,index, 3, &realCntNames, &relationType);
    whichName = whichName > realCntNames ? realCntNames: whichName;
    if(!relationPtr) return false;
    for(idx i=1; relationPtr && i <= whichName; i++){
        if (i==whichName){
            sStr t;
            sString::searchAndReplaceSymbols(&t, (const char *)DB.Getbody(relationType,*relationPtr++,&bodysize),0, "/\"\'", "   " , 0, true , true, false , true);
            buf->printf("%s",t.ptr(0));
        }
        else relationPtr++;
        
    }
    return true;
}

bool sNCBITaxTree::getAllNameByIndex(sStr * buf, idx index){
    idx bodysize, realCntNames=0;
    idx relationType=0;
    idx * relationPtr = DB.GetRelationPtr(TAXID_TYPE ,index, 3, &realCntNames, &relationType);
    if(!relationPtr) return false;
    for(idx i=1; relationPtr && i <= realCntNames; i++,relationPtr++){
        sStr t;
        sString::searchAndReplaceSymbols(&t, (const char *)DB.Getbody(relationType,*relationPtr++,&bodysize),0, "/\"\'", "   " , 0, true , true, false , true);
        if(i>1) buf->printf("|");
        buf->printf("%s",t.ptr(0));
    }
    return true;
}


idx  sNCBITaxTree::getNodeCnt(){
    return DB.GetRecordCnt(TAXID_TYPE);
}

void sNCBITaxTree::printByFlags(sStr * buf, idx index, idx WhatToPrintflags, idx cntNames, idx matchNameIndex,const char *rankFilters, bool ifPrintRoot, const ExtraData * extra)
{ 

    if(WhatToPrintflags & eTaxId){ 
        if(buf->length())
            buf->printf(",");    
        buf->printf("%" DEC,sNCBITaxTree::getTaxIdByIndex(index));
    }
    if(WhatToPrintflags & eParent){
        if(buf->length())
            buf->printf(",");
         buf->printf("%" DEC,sNCBITaxTree::getParentByIndex(index));
    }
    if(WhatToPrintflags & eName){
        if(buf->length())
            buf->printf(",");
        sStr t;
        bool ifFind = sNCBITaxTree::getNameByIndex(&t, index, cntNames);
        if(ifFind)
            sString::escapeForCSV(*buf, t.ptr(0), t.length());
        else buf->printf(" ");
    }
    if(WhatToPrintflags & eRank) {
        if(buf->length())
            buf->printf(",");
        if(sNCBITaxTree::getRankByIndex(index))
            buf->printf("%s",sNCBITaxTree::getRankByIndex(index));
        else buf->printf(" ");
    }
    if(WhatToPrintflags & ePath) {
        if(buf->length())
            buf->printf(",");
        sStr t;
        bool ifFind = sNCBITaxTree::getPathByIndex(&t,index,ePath,rankFilters,ifPrintRoot);
        if(ifFind)
            sString::escapeForCSV(*buf, t.ptr(0), t.length());
        else buf->printf(" ");
    }

    if(WhatToPrintflags & ePathID) {
        if(buf->length())
            buf->printf(",");
        sNCBITaxTree::getPathByIndex(buf,index, ePathID,rankFilters);

    }
    if(WhatToPrintflags & eCntChildren) {
        if(buf->length())
            buf->printf(",");
        buf->printf("%" DEC "",sNCBITaxTree::getChildCnt(sNCBITaxTree::getTaxIdByIndex(index)));
    }
    if(WhatToPrintflags & eMatchName){
        if(buf->length())
            buf->printf(",");
        sStr t;
        bool ifFind = sNCBITaxTree::getNameByIndex(&t, index, matchNameIndex);
        if(ifFind)
            sString::escapeForCSV(*buf, t.ptr(0), t.length());
        else buf->printf(" ");
    }
    if(WhatToPrintflags & eAllName){
        if(buf->length())
            buf->printf(",");
        sStr t;
        bool ifFind = sNCBITaxTree::getAllNameByIndex(&t, index);
        if(ifFind)
            sString::escapeForCSV(*buf, t.ptr(0), t.length());
        else buf->printf(" ");
    }

    if(WhatToPrintflags & eProjectID){
        if(buf->length())
            buf->printf(",");
        buf->printf("%" DEC,sNCBITaxTree::getProjIDByIndex(index));
    }
    if(WhatToPrintflags & eDefPath) {
        if(buf->length())
            buf->printf(",");
        sStr t;
        bool ifFind = sNCBITaxTree::getPathByIndex(&t,index,eDefPath,rankFilters,ifPrintRoot);
        if(ifFind)
            sString::escapeForCSV(*buf, t.ptr(0), t.length());
        else buf->printf(" ");
    }
    if(WhatToPrintflags & eMatchCnt){
        if(buf->length())
            buf->printf(",");
        if (extra)
            buf->printf("%" DEC, extra->matchCnt);
    }
    if(WhatToPrintflags & eNum){
        if(buf->length())
            buf->printf(",");
        if (extra)
            buf->printf("%0.1f", extra->num);
    }
    if(WhatToPrintflags & eMin){
        if(buf->length())
            buf->printf(",");
        if (extra)
            buf->printf("%0.1f", extra->min);
    }
    if(WhatToPrintflags & eMax){
        if(buf->length())
            buf->printf(",");
        if (extra)
            buf->printf("%0.1f", extra->max);
    }
    if(WhatToPrintflags & eMean){
        if(buf->length())
            buf->printf(",");
        if (extra)
            buf->printf("%0.1f", extra->mean);
    }
    if(WhatToPrintflags & eStdDev){
        if(buf->length())
            buf->printf(",");
        if (extra)
            buf->printf("%0.2f", extra->stddev);
    }
    if(WhatToPrintflags & eConfIntval){
        if(buf->length())
            buf->printf(",");
        if (extra) {
            if (WhatToPrintflags & eOutFile){
                buf->printf("%0.1f+/-%0.2f", extra->mean, extra->confIntval);
            }
            else {
                buf->printf("%0.1f\302\261%0.2f", extra->mean, extra->confIntval);
            }
        }
    }
}


void sNCBITaxTree::Parsefile(const char* nodesName, const char* namesName, const char* inputfilename, bool combinedFile)
{

    
    sVioDB db(inputfilename,"ncbiTaxTree",3,4);
    sFil NodesFile(nodesName,sMex::fReadonly);
    sFil NamesFile(namesName,sMex::fReadonly);
    const char * src=NodesFile.ptr();
    const char * nsrc=NamesFile.ptr();
    idx length=NodesFile.length();
    idx nlength=NamesFile.length();
    
    if(!src)::printf("Taxonomy file %s is unreadable\n",nodesName);
    if(!nsrc)::printf("Taxonomy Name file %s is unreadable\n",namesName);
    if(!nsrc || !src)return;

    sStr line;
    char buf[sSizePage];
    idx taxid=0;
    idx parentid=0;
    sStr string;


    idx relationlistID[4]={1,1,2,3};
    idx relationlistRank[1]={1};
    idx relationlistName[1]={1};

    db.AddType(sVioDB::eInt,4,relationlistID,"taxID", 1);
    db.AddType(sVioDB::eString,1,relationlistRank,"rank",2);
    db.AddType(sVioDB::eString,1,relationlistName,"name",3);


    for ( idx i=0 ; i<length && *src; ++i)
    {
        src=getonelinefromBuffer(buf,src);
        ParseOneline(2,buf,&string,&taxid,&parentid);
        char * rank=string.ptr(0);

        idx IDrecordsize=sizeof(taxid);
        idx Rankrecordsize=strlen(rank)+1;

        idx myRecordID=0;

        if(db.SetRecordIndexByBody((const void *)&taxid, TAXID_TYPE, &myRecordID))
            db.AddRecord(TAXID_TYPE,(const void *)&taxid, IDrecordsize);
        db.AddRecordRelationshipCounter(TAXID_TYPE, myRecordID, 2);
        db.AddRecordRelationshipCounter(TAXID_TYPE, myRecordID, 3);


        myRecordID=0;
        if( db.SetRecordIndexByBody((const void *)&parentid,TAXID_TYPE, &myRecordID) ) 
            db.AddRecord(TAXID_TYPE,(const void *)&parentid, IDrecordsize);
        if(parentid!=taxid) 
            db.AddRecordRelationshipCounter(TAXID_TYPE, myRecordID, 1);
        myRecordID=0;
        if( db.SetRecordIndexByBody((const void*)rank,RANK_TYPE,&myRecordID) ) 
            db.AddRecord(RANK_TYPE,(const void *)rank, Rankrecordsize);
        db.AddRecordRelationshipCounter(RANK_TYPE, myRecordID, 1);
    }

    for ( idx k=0; k<nlength && *nsrc; ++k)
    {
        nsrc=getonelinefromBuffer(buf,nsrc);    
        ParseOneline(1,buf,&string,&taxid);
        char * names= string.ptr(0);    
        idx IDrecordsize=sizeof(taxid);
        idx Namerecordsize=strlen(names)+1;

        idx myRecordID=0;
        if(db.SetRecordIndexByBody((const void *)&taxid,TAXID_TYPE,&myRecordID)) 
            db.AddRecord(TAXID_TYPE,(const void *)&taxid, IDrecordsize);
        db.AddRecordRelationshipCounter(TAXID_TYPE, myRecordID, 4);
        
        db.AddRecord(NAME_TYPE ,(const void *)names, Namerecordsize);
        db.AddRecordRelationshipCounter(NAME_TYPE, 0, 1);
    } 
    
    db.AllocRelation();
    


    src=NodesFile.ptr();
    nsrc=NamesFile.ptr();

    for ( idx k=0 ; k< length  && *src ; ++k) {
 
        src=getonelinefromBuffer(buf,src);    
        ParseOneline(2,buf,&string,&taxid,&parentid);
        char * rank=string.ptr(0);

        idx myRecordID=db.GetRecordIndexByBody((const void *)&taxid, TAXID_TYPE);
        idx myParentRecordID=db.GetRecordIndexByBody((const void *)&parentid, TAXID_TYPE);
        idx myRankRecordID=db.GetRecordIndexByBody((const void*)rank, RANK_TYPE, strlen(rank)+1);

        if(parentid!=taxid) db.AddRelation(TAXID_TYPE, 1, myParentRecordID, myRecordID );
        db.AddRelation(TAXID_TYPE, 2, myRecordID , myParentRecordID );
        db.AddRelation(TAXID_TYPE, 3 , myRecordID ,myRankRecordID );

        db.AddRelation(RANK_TYPE, 1 , myRankRecordID ,myRecordID );

    }
    
    for ( idx k=0 ; k< nlength  && *nsrc ; ++k) {
        nsrc=getonelinefromBuffer(buf,nsrc); 
        ParseOneline(1,buf,&string,&taxid);

        idx myRecordID=db.GetRecordIndexByBody((const void *)&taxid, TAXID_TYPE);
        db.AddRelation(TAXID_TYPE, 4, myRecordID , k+1 );
        db.AddRelation(NAME_TYPE, 1, k+1 , myRecordID );
        
    }

    db.Finalize(combinedFile);
    
}    

void sNCBITaxTree::ParseOneline(idx idCnt, const char * buf, sStr * string, idx * id, ...)
{
    string->cut(0);

    va_list marker;
    va_start(marker, id );
    sStr line;
    sString::searchAndReplaceSymbols(&line, buf ,0, "|", 0, 0, true , true, false , true);
    const char * ptr1=line.ptr(); 
    for(idx i=0; i<idCnt ;i++)
    {
        sscanf(ptr1,"%" DEC,id);
        id = va_arg( marker, idx *);
        ptr1=sString::next00(ptr1);
    }
    va_end(marker);

    string->printf(0,"%s",ptr1);
    sString::searchAndReplaceSymbols(string->ptr(0), 0, sString_symbolsBlank, " ", 0, true , true, false , true);
    sString::cleanEnds(string->ptr(0),0,sString_symbolsBlank,true);
}

const char * sNCBITaxTree::getonelinefromBuffer(char * buf, const char * fileContent)
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
