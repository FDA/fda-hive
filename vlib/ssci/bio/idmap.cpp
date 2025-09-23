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
#include <ssci/bio/idmap.hpp>

#define ID_TYPE 1
#define IDTYPE_TYPE 2

const char * sIdMap::idMapVioDBFileFlag ="idTest";
const char * sIdMap::excludeList= "abc" __;

bool sIdMap::getNeighbourByLevel(const char * startId, const char * idTypeFrom,sStr &buf, idx level, idx maxCnt, idx start,idx maxExpand){
    sVec<QC>  idWithType;
    if(!findIdTypeAuto(startId, idWithType,idTypeFrom))
        return false;
    idx cnt=0;
    for(idx i=0;i<idWithType.dim();i++){
        for(idx j=0;j<dbList.dim();j++){
            sDic<idx> table;
            getNeighbour(&idWithType[i],&dbList[j],level,buf,table,maxCnt,start,&cnt,maxExpand);
        }
    }
    return cnt ? true : false;
}

void sIdMap::getNeighbour(const QC * root,sVioDB *db,idx level, sStr &list, sDic<idx> &table, idx maxCnt, idx start, idx *cnt,idx maxExpand){
    sStr whatToHash;
    whatToHash.printf(0,"%s%s",root->type.ptr(0),root->id.ptr(0));
    if(level>0 && *cnt < maxCnt+start ){
        idx idFind = table.find(whatToHash.ptr(0));
        if(!idFind){
            table.set(whatToHash.ptr(0));
            sStr realBodyBuf;
            realBodyBuf.printf(0,"%s",root->id.ptr(0));
            if(setBodyWithType(findTypeIndex(root->type.ptr(0), *db, 0), realBodyBuf)){
                idx bodyIndex =db->GetRecordIndexByBody(realBodyBuf.ptr(), ID_TYPE,realBodyBuf.length());
                if(bodyIndex){
                    idx cntRel=0;
                    idx * relPtr = db->GetRelationPtr(ID_TYPE,bodyIndex, 1, &cntRel, 0);
                    if(*relPtr>0){
                        for(idx i=0;i<cntRel && i < maxExpand;i++,(relPtr++)){
                             if(*cnt >= maxCnt+start)  return;
                             idx idChildSize = 0;
                             const char * idChild = (const char*)db->Getbody(ID_TYPE,*relPtr,&idChildSize);
                             sStr realBody;
                             getIdRealBody(idChild,*relPtr, realBody, *db,idChildSize);
                             if(*cnt >= start){
                                 if(*cnt)list.printf(",");
                                 list.printf("%s",realBody.ptr(0));
                             }
                             (*cnt)++;

                             QC child;
                             child.id.printf(0,"%s",realBody.ptr(0));
                             findIdType(*relPtr,child.type,*db);
                             getNeighbour(&child,db,--level, list, table,maxCnt,start, cnt,maxExpand);
                        }
                    }
                }
            }
        }
    }
    return ;
}
idx sIdMap::findIdByInput(const char * idInput, const char * idTo, sStr &buf, const char * idtypeFrom, idx cnt){
    idx cntResult = 0;
      sStr lowIdTo;
      if(!idTo || !(*idTo) ||!idInput || !(*idInput))
          return cntResult;
      idx idToLen = sLen(idTo);
      for(idx i=0;i<idToLen;i++){
          lowIdTo.printf("%c",tolower(*(idTo+i)));
      }
      if(*idInput=='>'){
          sStr line;
          sString::searchAndReplaceSymbols(&line, idInput+1 ,0, "|=\n\r\t", 0, 0, true , true, false , true);
          sDic<sStr> table;
          const char * ptr1=line.ptr();
          sStr * dicPtr=0;
          for(idx i=0; ptr1 ;i++)
          {
              sStr buf1;
              buf1.printf(0,"%s",ptr1);
              sString::searchAndReplaceSymbols(buf1.ptr(0), 0, sString_symbolsBlank, " ", 0, true , true, false , true);
              sString::cleanEnds(buf1.ptr(0),0,sString_symbolsBlank,true);
              if(!(i%2)){
                  for(idx i=0;i<buf1.length();i++){
                      buf1[i]=tolower(buf1[i]);
                  }
                  dicPtr = table.set(buf1.ptr(0));
              }
              else{
                  if(dicPtr)
                      dicPtr->printf(0,"%s",buf1.ptr(0));
                  dicPtr=0;
              }
              ptr1=sString::next00(ptr1);
          }

          for(idx i=0;i<table.dim();i++){
              const char * type = (const char*)table.id(i);
              const char * val = table[i].ptr(0);
              if(lowIdTo.ptr() && *lowIdTo.ptr() && (strncmp(type,lowIdTo.ptr(),lowIdTo.length())==0)){
                if(cntResult) buf.printf(",");
                buf.printf("%s",val);
                ++cntResult;
              }
              if(cntResult>=cnt){
                  return cntResult;
              }
          }
      }
      return cntResult;
}


idx sIdMap::findId(const char * idInput, const char * idTo, sStr &buf, const char * idtypeFrom, idx cnt, idx start,sStr * giNumber){
    idx cntResult = 0;
    sStr lowIdTo;
    for(idx i=0;idTo && *idTo && i<sLen(idTo);i++){
        lowIdTo.printf("%c",tolower(*(idTo+i)));
    }
    char cset[]="|,=\n\r\t";

    if(idInput && *idInput && (idx)strcspn (idInput,cset)!=sLen(idInput)){
        if(*idInput=='>')    idInput++;
        sStr line;
        sString::searchAndReplaceSymbols(&line, idInput ,0, cset, 0, 0, true , true, false , true);
        sDic<sStr> table;
        const char * ptr1=line.ptr();
        sStr * dicPtr=0;
        for(idx i=0; ptr1 ;i++)
        {
            sStr buf1;
            buf1.printf(0,"%s",ptr1);
            sString::searchAndReplaceSymbols(buf1.ptr(0), 0, sString_symbolsBlank, " ", 0, true , true, false , true);
            sString::cleanEnds(buf1.ptr(0),0,sString_symbolsBlank,true);
            if(!(i%2)){
                for(idx i=0;i<buf1.length();i++){
                    buf1[i]=tolower(buf1[i]);
                }
                dicPtr = table.set(buf1.ptr(0));
            }
            else{
                if(dicPtr)
                    dicPtr->printf(0,"%s",buf1.ptr(0));
                dicPtr=0;
            }
            ptr1=sString::next00(ptr1);
        }

        for(idx i=0;i<table.dim();i++){
            const char * type = (const char*)table.id(i);
            const char * val = table[i].ptr(0);
            giNumber->printf("%s",val);

            if(idTo){
                if(lowIdTo.ptr() && *lowIdTo.ptr() && (strncmp(type,lowIdTo.ptr(),lowIdTo.length())==0)){
                    if(cntResult >=start){
                        if(cntResult) buf.printf(",");
                        buf.printf("%s",val);
                        ++cntResult;
                    }
                }
                else{
                    if(idtypeFrom && *idtypeFrom){
                        sStr lowIdFrom;
                        for(idx i=0;i<sLen(idtypeFrom);i++){
                            lowIdFrom.printf("%c",tolower(*(idtypeFrom+i)));
                        }
                        if(strncmp(type,lowIdFrom.ptr(0),lowIdFrom.length())==0){
                            cntResult += getIdMap(val, lowIdFrom.ptr(0), lowIdTo.ptr(),buf, 3, cnt-cntResult, start,50);
                        }
                        else continue;
                    }
                    else cntResult += getIdMap(val, type, lowIdTo.ptr(),buf, 3, cnt-cntResult, start,50);

                }
            }
            else{
                getNeighbourByLevel(val, type,buf, 1, cnt-cntResult, start,50);
                return cnt;
            }

            if(cntResult>=cnt){
                return cntResult;
            }
        }

    }
    else if(idInput && *idInput){
        if(idTo)cntResult=getIdMap(idInput, idtypeFrom, idTo,buf, 3, cnt, start,50);
        else{
            getNeighbourByLevel(idInput,idtypeFrom,buf, 1, cnt-cntResult, start,50);
            cntResult = cnt;
        }
    }

    return cntResult;
}

idx sIdMap::getIdMap(const char * idFrom, const char * idtypeFrom, const char * idTo,sStr &buf, idx MaxLevelAllowed, idx maxCnt, idx start,idx maxExpand){
    sDic <idx > table;
    sDic <idx> typeTable;
    sVec<QC> myQueue;
    idx cntLevel=0;
    idx peakPos = 0;
    idx cntQueueItem = 0;
    idx cntResult = 0;
    sStr lowIdTo;
    for(idx i=0;i<sLen(idTo);i++){
        lowIdTo.printf("%c",tolower(*(idTo+i)));
    }
    if(findIdTypeAuto(idFrom, myQueue,idtypeFrom)){
        if(lowIdTo.ptr(0))    typeTable.set(lowIdTo.ptr(0));
        else return cntResult;
        cntQueueItem=myQueue.dim();
        while(peakPos < myQueue.dim()){
            idx ifWithinExcludeList=-1;
            QC * top = &myQueue[peakPos];
            peakPos++;
            cntQueueItem--;
            const char * body = top->id.ptr(0);
            const char * type = top->type.ptr(0);
            if(type && body){
                sStr whatToHash;
                whatToHash.printf(0,"%s%s",type,body);
                table.set(whatToHash.ptr(0));
                sString::compareChoice(type, excludeList, &ifWithinExcludeList, false, 0, true);
                if(strcmp(type,lowIdTo.ptr(0))==0){
                    cntResult++;
                    if(cntResult <= maxCnt+start && cntResult>=start){
                        if(cntResult>1) buf.printf(",");
                        buf.printf("%s",body);
                    }
                    else if(cntResult > maxCnt+start)
                        return cntResult;
                }
                else if(!typeTable.find(type)){

                    if(!cntLevel)typeTable.set(type);
                    for(idx i=0;ifWithinExcludeList==-1&&i<dbList.dim();i++){
                        sVioDB & db = dbList[i];
                        sStr wholeBodyBuf;
                        wholeBodyBuf.printf(0,"%s",body);
                        setBodyWithType(findTypeIndex(type,db,0), wholeBodyBuf);
                        idx recordIndex =  db.GetRecordIndexByBody(wholeBodyBuf.ptr(0),ID_TYPE,wholeBodyBuf.length());
                        if(recordIndex){
                            idx cntRel=0;
                            idx * relPtr = db.GetRelationPtr(ID_TYPE,recordIndex, 1, &cntRel, 0);
                            if(*relPtr>0){
                                for(idx i=0;i<cntRel && i < maxExpand;i++,(relPtr++)){
                                    idx bodySize = 0;
                                    sStr typeStr;
                                    findIdType(*relPtr, typeStr, db);
                                    const char * bodyPtr = (const char*)db.Getbody(ID_TYPE,*relPtr,&bodySize);
                                    if(bodyPtr && typeStr){
                                        sStr realBody;
                                        getIdRealBody(bodyPtr,*relPtr, realBody, db,bodySize);
                                        if(realBody.ptr(0)){
                                            sStr whatToFind;
                                            whatToFind.printf(0,"%s%s",typeStr.ptr(0),realBody.ptr(0));
                                            if(!table.find(bodyPtr)){
                                                QC * ptr = myQueue.add(1);
                                                ptr->id.printf(0,"%s",realBody.ptr(0));
                                                ptr->type.printf(0,"%s",typeStr.ptr(0));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }


            if(cntQueueItem==0){
                cntQueueItem = myQueue.dim()-peakPos;
                cntLevel++;
                if(MaxLevelAllowed){
                    if(MaxLevelAllowed < cntLevel){
                        return cntResult;
                    }
                }
            }
        }
    }
    return cntResult;
}




idx sIdMap::findTypeIndex(const char * type, sVioDB &db, idx typeSize){
    typeSize = typeSize ? typeSize : sLen(type);
    sStr buf;
    for(idx i=0;i<sLen(type);i++){
        buf.printf("%c",tolower(*(type+i)));
    }
    return db.GetRecordIndexByBody(buf.ptr(0),IDTYPE_TYPE,typeSize);
}

void sIdMap::getTypeBodyByIndex(sStr & buf,idx indexOneBase, sVioDB &db){
    buf.printf(0,"%s",(const char*)db.Getbody(IDTYPE_TYPE,indexOneBase,0));
}


bool sIdMap::findIdTypeAuto(const char * id, sVec<QC> & idIndexWithDB,const char * idTypeFrom){
    for(idx i=0;i<dbList.dim();i++){
        sVioDB  * db = &dbList[i];
        if(idTypeFrom && *idTypeFrom){
            idx typeIndex=findTypeIndex(idTypeFrom, *db, 0);
            if(typeIndex){
                sStr idWithType;
                idWithType.printf(0,"%s",id);
                if(setBodyWithType(typeIndex,idWithType)){
                    idx bodyIndex = db->GetRecordIndexByBody(idWithType.ptr(0),ID_TYPE,idWithType.length());
                    if(bodyIndex){
                        QC * temp = idIndexWithDB.add(1);
                        temp->id.printf(0,"%s",id);
                        temp->type.printf(0,"%s",idTypeFrom);
                        return true;
                    }
                }
            }
        }
        else{
            idx cntType = db->GetRecordCnt(IDTYPE_TYPE);
            for(idx ti=0;ti<cntType;ti++){
                sStr idWithType;
                idWithType.printf(0,"%s",id);
                setBodyWithType(ti+1, idWithType);
                idx bodyIndex = db->GetRecordIndexByBody(idWithType.ptr(0),ID_TYPE,idWithType.length());
                if(bodyIndex){
                   sStr typeBuf;
                   getTypeBodyByIndex(typeBuf,ti+1,*db);
                   idx cntRel=0;
                   idx * relPtr = db->GetRelationPtr(ID_TYPE,bodyIndex, 2 , &cntRel, 0);
                   if(cntRel==1 && (*relPtr)>0){
                       idx typeBodySize = 0;
                       const char * typeBody=(const char*)db->Getbody(IDTYPE_TYPE,*relPtr,&typeBodySize);
                       if(typeBuf.length()==typeBodySize && strncmp(typeBuf.ptr(0),typeBody,typeBodySize)==0){
                           QC * temp = idIndexWithDB.add(1);
                           temp->id.printf(0,"%s",id);
                           temp->type.printf(0,"%s",typeBuf.ptr(0));
                       }
                   }
                }
            }
        }
    }
    return idIndexWithDB.dim() ? true : false;
}

bool sIdMap::findIdType(idx bodyIndex, sStr & idtype, sVioDB &db){
    if(bodyIndex){
       idx cntRel=0;
       idx relationTypeIndex=0;
       idx * relPtr = db.GetRelationPtr(ID_TYPE,bodyIndex, 2 , &cntRel, &relationTypeIndex);
       if(cntRel==1 && (*relPtr)>0){
           idtype.printf(0,"%s",(const char*)db.Getbody(2,*relPtr,0));
           return true;
       }
    }
    return false;
}



void sIdMap::getIdRealBody(const char * idWithType,idx recordIndex, sStr &realBody, sVioDB & db, idx idWithTypeSize){
    idx * typeIndexPtr = db.GetRelationPtr(ID_TYPE,recordIndex, 2, 0, 0);
    idx byteTaken = ((*typeIndexPtr)/255)+1;
    realBody.add(idWithType,idWithTypeSize-byteTaken);
    realBody.add0();
}

idx sIdMap::setBodyWithType(idx typeRecordIndex, sStr & body){
    idx i=0;
    while(typeRecordIndex){
        body.printf("%c",(char)(typeRecordIndex%255));
        typeRecordIndex/=255;
        i++;
    }
    return i;
}

idx sIdMap::getBodyWithType(idx typeRecordIndex, sStr & body){
    idx len = body.length();
    while(typeRecordIndex){
        body.cut(--len);
        typeRecordIndex/=255;
    }
    body.add0();
    return len;
}

void sIdMap::Parsefile(const char* inputFilename, const char* outfilename, bool combinedFile, const char * keyTypeOrigin)
{


    sVioDB db(outfilename,idMapVioDBFileFlag,2,2);
    sFil inputFile(inputFilename,sMex::fReadonly);
    const char * src=inputFile.ptr();

    idx length=inputFile.length();

    if(!src){
        printf("input file %s is unreadable\n",inputFilename);
        return;
    }

    sStr line;
    char buf[sSizePage];
    sStr key;
    sStr other;
    sStr otherType;
    sStr keyType;

    if(keyTypeOrigin && *keyTypeOrigin){
        for(idx i=0;i<sLen(keyTypeOrigin);i++){
            keyType.printf("%c",tolower(*(keyTypeOrigin+i)));
        }
    }

    idx relationlistID[2]={1,2};
    idx relationlistType[1]={1};


    db.AddType(sVioDB::eString,2,relationlistID,"id", 1);
    db.AddType(sVioDB::eString,1,relationlistType,"type",2);


    for ( idx i=0 ; i<length && *src; ++i)
    {
        src=getonelinefromBuffer(buf,src);
        ParseOnelineNCBI(buf,key,otherType,other);
        if(otherType.ptr(0)){
            idx len = otherType.length();
            for(idx i=0;i<len;i++){
                *otherType.ptr(i)=tolower(*(otherType.ptr(i)));
            }
        }



        idx keyTypeIndex=0;
        idx otherTypeIndex=0;
        if( db.SetRecordIndexByBody(otherType.ptr(0),IDTYPE_TYPE,&otherTypeIndex,otherType.length()))
            db.AddRecord(IDTYPE_TYPE,otherType.ptr(0), otherType.length());
        setBodyWithType(otherTypeIndex,other);

        if( db.SetRecordIndexByBody(keyType,IDTYPE_TYPE,&keyTypeIndex,sLen(keyType)))
            db.AddRecord(IDTYPE_TYPE,keyType, sLen(keyType));
        setBodyWithType(keyTypeIndex,key);

        idx myRecordID = 0;
        if(db.SetRecordIndexByBody(key.ptr(0), ID_TYPE, &myRecordID,key.length())){
            db.AddRecord(ID_TYPE,key.ptr(0), key.length());
            db.AddRecordRelationshipCounter(ID_TYPE, myRecordID, 2);
        }

        db.AddRecordRelationshipCounter(ID_TYPE, myRecordID, 1);

        myRecordID=0;
        if( db.SetRecordIndexByBody(other.ptr(0),ID_TYPE, &myRecordID,other.length())){
            db.AddRecord(ID_TYPE,other.ptr(0), other.length());
            db.AddRecordRelationshipCounter(ID_TYPE, myRecordID, 2);
        }
        db.AddRecordRelationshipCounter(ID_TYPE, myRecordID, 1);
    }

    db.AllocRelation();

    src=inputFile.ptr();

    for ( idx k=0 ; k< length  && *src ; ++k) {

        src=getonelinefromBuffer(buf,src);
        ParseOnelineNCBI(buf,key,otherType,other);
        if(otherType.ptr(0)){
            idx len = otherType.length();
             for(idx i=0;i<len;i++){
                *otherType.ptr(i)=tolower(*(otherType.ptr(i)));
            }
        }
        idx otherTypeIndex=db.GetRecordIndexByBody(otherType.ptr(0), IDTYPE_TYPE,otherType.length());
        idx keyTypeIndex=db.GetRecordIndexByBody(keyType, IDTYPE_TYPE,sLen(keyType));

        setBodyWithType(otherTypeIndex,other);
        setBodyWithType(keyTypeIndex,key);

        idx keyIndex=db.GetRecordIndexByBody(key.ptr(0), ID_TYPE,key.length());
        idx otherIndex=db.GetRecordIndexByBody(other.ptr(0), ID_TYPE,other.length());

        db.AddRelation(ID_TYPE, 1, keyIndex, otherIndex );
        db.AddRelation(ID_TYPE, 1, otherIndex ,keyIndex );
        idx relCnt=0;
        idx * ptr1 = db.GetRelationPtr(ID_TYPE,keyIndex,2,&relCnt,0);
        if(!(*ptr1))
            db.AddRelation(ID_TYPE, 2 , keyIndex ,keyTypeIndex );
        ptr1 = db.GetRelationPtr(ID_TYPE,otherIndex,2,&relCnt,0);
        if(!(*ptr1))
            db.AddRelation(ID_TYPE, 2 , otherIndex ,otherTypeIndex );
    }
    db.Finalize(combinedFile);
}


void sIdMap::ParseOneline(const char * buf, sStr & key, sStr &otherType, sStr &other)
{
    sStr line;
    sString::searchAndReplaceSymbols(&line, buf ,0, "\t" __, 0, 0, true , true, false , true);
    const char * ptr1=line.ptr();
    if(ptr1)    key.printf(0,"%s",ptr1);
    ptr1=sString::next00(ptr1);
    if(ptr1)    otherType.printf(0,"%s",ptr1);
    ptr1=sString::next00(ptr1);
    if(ptr1)    other.printf(0,"%s",ptr1);
}

void sIdMap::ParseOnelineNCBI(const char * buf, sStr & key, sStr &otherType, sStr &other)
{
    sStr line;
    sString::searchAndReplaceSymbols(&line, buf ,0, "\t" __, 0, 0, true , true, false , true);
    const char * ptr1=line.ptr();
    if(ptr1)    key.printf(0,"%s",ptr1);
    ptr1=sString::next00(ptr1);
    otherType.printf(0,"ncbi_taxid");
    if(ptr1)    other.printf(0,"%s",ptr1);
}



const char * sIdMap::getonelinefromBuffer(char * buf, const char * fileContent)
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
