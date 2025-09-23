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
#include <ulib/ulib.hpp>
#include <violin/hiveion.hpp>



using namespace sviolin;


idx collapseByRangeAndtype(const char * contentInSpecificFormat, idx contentLength,sStr & output, idx cntStart=0, idx cnt=100, idx endMax=0,sBioal * mutAl=0);


sHiveIon * sHiveIon::init(sUsr * user, const char * objList, const char * iontype, const char * filenameWithoutExtension00)
{
    if( user )
        myUser = user;
    sVec<sHiveId> ionList;
    if( objList )
        sHiveId::parseRangeSet(ionList, objList);
    if( !ionList.dim() && iontype ) {
        sUsrObjRes res;
        user->objs2(iontype, res);
        for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
            *(ionList.add(1)) = *res.id(it);
        }
    }
    ionCnt=0;
    pathList00.cut(0);
    wanderList.empty();
    sStr tmpPath;
    for(idx iV = 0; iV < ionList.dim(); ++iV) {

        sUsrObj obj(*user, ionList[iV]);
        if( !obj.Id() )
            continue;


        tmpPath.cut(0);

        obj.getFilePathname00(tmpPath, filenameWithoutExtension00);

        if (tmpPath.length() && sFile::exists(tmpPath.ptr(0))){
            pathList00.add(tmpPath.ptr(0),tmpPath.length() + 1);
            ++ionCnt;
            continue;
        }
        sStr addExtensionByDefaut("%s.%s",filenameWithoutExtension00,"ion");
        addExtensionByDefaut.add0cut(2);
        obj.getFilePathname00(tmpPath, addExtensionByDefaut);

        if (tmpPath.length() && sFile::exists(tmpPath.ptr(0))){
            sFilePath fullPathWithoutExtension(tmpPath.ptr(),"%%dir/%s",filenameWithoutExtension00);

            pathList00.add(fullPathWithoutExtension.ptr());
            ++ionCnt;
        }
    }
    pathList00.add0(2);

    return this;
}

sIonWander * sHiveIon::addIonWander(const char * wandername, const char * iql, idx iqllen)
{

    sIonWander * ionWander=wanderList.set(wandername);


    ionWander->attachIons(pathList00.ptr(0),sMex::fReadonly,ionCnt);
    ionWander->ionList.dim();
    if( ionWander->traverseCompile(iql,iqllen ? iqllen : sLen(iql)) )
        return 0;


    return ionWander;
}




idx sHiveIonSeq::annotMap(sIO * io, sBioseq * sub, sDic < sStr > * dic, const char * seqidFrom00, idx countResultMax, idx startResult, idx contSequencesMax , idx outPutWithHeader, sStr * header )
{
    bool complexIDLookUp = false;
    const char * wanderId;
    sDic <sStr >   tmpDic;
     if(!dic)dic=&tmpDic;

    if (wanderList.find("profilerIDLookup")){
        wanderId = "profilerIDLookup";
    }
    else {
        wanderId = "simpleIDLookup";
    }
    sIonWander * iWander1 = &wanderList[wanderId];
    iWander1->resultCumulator=dic;


    sIonWander * iWander2=0;
    if (wanderList.find("complexIDLookup")){
        iWander2 = &wanderList["complexIDLookup"];
        iWander2->resultCumulator=dic;
        complexIDLookUp = true;
    }

    idx countSequenceIds=sub ? sub->dim() : contSequencesMax ;
    idx success=0;
    const char * nxt, *seqid=seqidFrom00;

    for (idx i=0; i< countSequenceIds && success<countResultMax ; ++i ){
        idx found=0;

        if(sub)seqid= sub->id(i);
        else if(i && seqidFrom00 ) { seqid=sString::next00(seqid); if(!seqid) break;}

        nxt=seqid+sLen(seqid);
        idx sizeSeqId=nxt-seqid;
        iWander1->setSearchTemplateVariable("$id", 0, seqid, sizeSeqId);


        iWander1->traverseBuf.cut(0);
        iWander1->traverse();

        if( iWander1->traverseBuf.length()!=0 ) {
            ++success;
            if(success<=startResult)
                return success;

            found=1;
        }

        if (complexIDLookUp){
          if(strcmp("profilerIDLookup",wanderId)==0 || (!found  && strcmp("profilerIDLookup",wanderId)!=0) ) {
                for( const char * p=seqid; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
                    const char * curType=p;
                    nxt=strpbrk(p,"| ");
                    if(!nxt || *nxt==' ')
                        break;

                    const char * curId=nxt+1;
                    nxt=strpbrk(nxt+1," |");
                    if(!nxt)
                        nxt=seqid+sizeSeqId;
                    if(*nxt==' ')
                        break;

                    iWander2->setSearchTemplateVariable("$id", 3, curId, nxt-curId);
                    iWander2->setSearchTemplateVariable("$type", 5, curType, curId-1-p);

                    iWander2->traverseBuf.cut(0);
                    iWander2->traverse();

                    if( iWander2->traverseBuf.length()!=0 ) {
                        ++success;
                        if(success<=startResult)
                            continue;

                        found=1;
                        break;
                    }

                }
            }
        }
        if(found){
            if(outPutWithHeader){
                io->add("sequence");
            }
            sVec <idx> toSkip; toSkip.resize(dic->dim());
            idx icoma=0, elementAdded=0;
            for ( idx il=0 ; il < dic->dim(); ++il){
                const char * id=(char*)dic->id(il);
                if (strncmp(id,"FEATURES",8)==0 || strncmp(id,"strand",6)==0) {
                    toSkip[elementAdded++]=il;
                    continue;
                }
                if (header) {
                    if (icoma) header->add(",",1);
                    header->add(id,sLen(id));
                }
                if (outPutWithHeader){
                    io->add(",",1);
                    io->add(id,sLen(id));
                }
                ++icoma;
            }
            if(outPutWithHeader){
                io->add("\n",1);
            }
            outPutWithHeader=0;
            toSkip.cut(elementAdded);
            icoma=0;
            for(idx il = 0; il < dic->dim(); ++il) {
                const char * pptr = dic->ptr(il)->ptr(0);
                bool skip = false;
                for ( idx isk=0; isk < toSkip.dim(); ++isk ) {
                    if (il == toSkip[isk]) {
                        skip=true;
                        break;
                    }
                }
                if (skip) continue;
                if( icoma )
                    io->add(",", 1);
                if( *pptr ) {
                    io->add("\"", 1);
                    io->add( pptr, dic->ptr(il)->length() > sLen(pptr) ? sLen(pptr) : dic->ptr(il)->length() );
                    io->add("\"", 1);
                }
                dic->ptr(il)->cut(0);
                ++icoma;
            }
            io->add("\n", 1);
        }
     }
   return success;
}


idx sHiveIonSeq::annotMapPosition(sStr * output, sDic < sStr > * dic, const char * seqidFrom00, idx pos_start, idx pos_end, idx countResultMax, idx startResult, idx header, sBioal * seqMultipleAlign ){
    idx success =0;

    sDic <sStr > tmpDic;
     if(!dic)dic=&tmpDic;

     char b[128];strcat(b,"chr");
     sStr buf_out;
    if (wanderList.find("seqPositionLookUp")){
        sIonWander * iWander1 = &wanderList["seqPositionLookUp"];
        iWander1->resultCumulator=dic;

        sIonWander * iWander2 = &wanderList["seqPositionLookUp_1"];
        iWander2->resultCumulator=dic;

        const char *seqidRaw=seqidFrom00;


        for(idx ikind=0; ikind<2; ++ikind){
            const char * seqid=seqidRaw;

            if(ikind==1) {
                if( strncasecmp(seqid,"chr",3)==0 ) {
                    seqid=seqid+3;
                }
                else if(isdigit(seqid[0])) {
                    strcat(b,seqid);
                    seqid=b;
                }
                else break;
            }

            const char * nxt=seqid+sLen(seqid);
            idx sizeSeqId=nxt-seqid;

            iWander1->setSearchTemplateVariable("$id", 0, seqid, sizeSeqId);
            iWander1->traverse();

            iWander2->setSearchTemplateVariable("$id", 0, seqid, sizeSeqId);
            iWander2->traverse();

            bool needToTryDot = true;
            if (iWander1->traverseBuf.length()){
                buf_out.addString(iWander1->traverseBuf.ptr(0),iWander1->traverseBuf.length());
            }
            if (iWander2->traverseBuf.length()){
                buf_out.addString(iWander2->traverseBuf.ptr(0),iWander2->traverseBuf.length());
            }
            if (buf_out.length()){
                collapseByRangeAndtype(buf_out.ptr(0),buf_out.length(),*output, startResult, countResultMax, pos_end, seqMultipleAlign);
                return success;
            }

             const char * dot = strpbrk(seqid,".");
             if (dot && needToTryDot){
                 iWander1->setSearchTemplateVariable("$id", 0, seqid, dot-seqid);
                 iWander1->resetResultBuf();
                 iWander1->traverse();
                 if (iWander1->traverseBuf.length()){
                     collapseByRangeAndtype(iWander1->traverseBuf.ptr(0),iWander1->traverseBuf.length(),*output, startResult, countResultMax, pos_end,seqMultipleAlign);
                     return success;
                }
            }
            for( const char * p=seqid; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
                nxt=strpbrk(p,"| ");
                if(!nxt || *nxt==' ')
                    break;

                const char * curId=nxt+1;
                nxt=strpbrk(nxt+1," |");
                if(!nxt)
                    nxt=seqid+sizeSeqId;
                if(*nxt==' ')
                    break;

                iWander1->setSearchTemplateVariable("$id", 0, curId, nxt-curId);
                iWander1->resetResultBuf();
                iWander1->traverse();

                iWander2->setSearchTemplateVariable("$id", 0, curId, nxt-curId);
                iWander2->resetResultBuf();
                iWander2->traverse();

                if (iWander1->traverseBuf.length()){
                    buf_out.addString(iWander1->traverseBuf.ptr(0),iWander1->traverseBuf.length());
                }
                if (iWander2->traverseBuf.length()){
                    buf_out.addString(iWander2->traverseBuf.ptr(0),iWander2->traverseBuf.length());
                }
                if (buf_out.length()){
                    collapseByRangeAndtype(buf_out.ptr(0),buf_out.length(),*output,startResult, countResultMax, pos_end,seqMultipleAlign);
                    return success;
                }
                const char * dot = strpbrk(curId,".");
                if (dot){
                    iWander1->setSearchTemplateVariable("$id", 0, curId, dot-curId);
                    iWander1->resetResultBuf();
                    iWander1->traverse();
                    if (iWander1->traverseBuf.length()){
                        collapseByRangeAndtype(iWander1->traverseBuf.ptr(0),iWander1->traverseBuf.length(),*output, startResult, countResultMax, pos_end,seqMultipleAlign);
                        return success;
                   }
               }
            }
            sIonWander * iWander3 = &wanderList["idTypePositionLookUp"];
            sDic <sStr > tmpDicSeq;
            iWander3->resultCumulator = &tmpDicSeq;
            for( const char * p=seqid; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
                const char * curType=p;
                nxt=strpbrk(p,"| ");
                if(!nxt || *nxt==' ')
                    break;

                const char * curId=nxt+1;
                nxt=strpbrk(nxt+1," |");
                if(!nxt)
                    nxt=seqid+sizeSeqId;
                if(*nxt==' ')
                    break;

                iWander3->setSearchTemplateVariable("$id", 3, curId, nxt-curId);
                iWander3->setSearchTemplateVariable("$type", 5, curType, curId-1-p);

                iWander3->resetResultBuf();
                iWander3->traverse();

                if(tmpDicSeq.dim()){
                    sStr mySeqToReplace;
                    for (idx iS=0; iS < tmpDicSeq.dim(); ++iS){
                        if (iS) mySeqToReplace.printf(",");
                        const char * myseq = (const char *) tmpDicSeq.id(iS);
                        mySeqToReplace.printf("%s",myseq);
                    }
                    iWander1->setSearchTemplateVariable("$id", 3, mySeqToReplace.ptr(0), mySeqToReplace.length());
                    iWander1->resetResultBuf();
                    iWander1->traverse();

                    iWander2->setSearchTemplateVariable("$id", 3, mySeqToReplace.ptr(0), mySeqToReplace.length());
                    iWander2->resetResultBuf();
                    iWander2->traverse();

                    if (iWander1->traverseBuf.length()){
                        buf_out.addString(iWander1->traverseBuf.ptr(0),iWander1->traverseBuf.length());
                    }
                    if (iWander2->traverseBuf.length()){
                        buf_out.addString(iWander2->traverseBuf.ptr(0),iWander2->traverseBuf.length());
                    }
                    if (buf_out.length()){
                        collapseByRangeAndtype(buf_out.ptr(0),buf_out.length(),*output,startResult, countResultMax, pos_end,seqMultipleAlign);
                        return success;
                    }


                }
            }
        }
    }
    return success;
}
idx sHiveIonSeq::traverserCallback (sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist){
    if (!statement->label || sLen(statement->label)==0){
        return 1;
    }

    infoParams * myParams = (infoParams *) wander->callbackFuncParam;
    if (strncmp(statement->label,"relation",8)==0 || strncmp(statement->label,"printLimit",10)==0){
        ++myParams->curIndex;
        if ( myParams->curIndex >= (myParams->cntStart + myParams->cnt)){
            wander->breakMode=true;
            idx bufLen=wander->pTraverseBuf->length();
            if (bufLen && *wander->pTraverseBuf->ptr(bufLen-1)!='\n') wander->pTraverseBuf->addString("\n");
            if (myParams->printDots) wander->pTraverseBuf->addString("......\n");
            return 0;
        }
        if (myParams->curIndex < myParams->cntStart) {
            wander->pTraverseBuf->cut(0);
        }
    }


    return 1;
}
idx sHiveIonSeq::standardTraverse(sStr & output, sDic < sStr > * dic, idx countResultMax, idx startResult, bool printDots, idx wanderIndex){
    idx success=0;

    sDic <sStr > tmpDic;
    if(!dic)dic=&tmpDic;

    infoParams myParams;
    myParams.cntStart = (startResult ? startResult : 0);
    myParams.cnt = (countResultMax ? countResultMax : 20);
    myParams.printDots = printDots;
    idx iW=0;
    if (wanderIndex>-1)
        iW=wanderIndex;


    for (; iW<wanderList.dim(); ++iW){
        sIonWander * iWander1 = wanderList.ptr(iW);
        iWander1->resultCumulator=dic;
        iWander1->callbackFunc =  sHiveIonSeq::traverserCallback;
        iWander1->callbackFuncParam = &myParams;

        iWander1->resetResultBuf();
        iWander1->traverse();
        if ( myParams.curIndex > startResult && iWander1->traverseBuf.length()) {
            output.addString(iWander1->traverseBuf.ptr(0),iWander1->traverseBuf.length());
        }
        myParams.curIndex += iWander1->cntResults;
    }

    return success;

}

idx collapseByRangeAndtype(const char * contentInSpecificFormat, idx contentLength,sStr & output, idx cntStart, idx cnt, idx endMax,sBioal * mutAl){
    sTxtTbl tbl;
    tbl.setBuf(contentInSpecificFormat,contentLength);
    tbl.parseOptions().flags = sTblIndex::fSaveRowEnds;
    tbl.parse();


    if (tbl.cols()!=5) {
#ifdef _DEBUG
        ::printf("\n!!! table has %" DEC " columns more than expected number (5 columns) !!!\n",tbl.cols());
        sStr buf;
        ::printf("%s\n", tbl.printCSV(buf));
#endif
        return 1;
    }


    sDic < sDic < sVec <sStr> > > dictBySeq_PosAndType_id;

    idx typeLen,idLen, seqLen;
    sStr compositeKey;

    idx iCnt=0, iNum=0;
    idx prvStart=0;
    idx curStart=0;
    sVariant startVar;

    sVariant endVar;
    char * prev_seq = 0;
    sVec<idx> uncompM(sMex::fSetZero);
    idx * matchTrain = 0, iMutS = 0;
    sBioseqAlignment::Al * hdr = 0;

    for (idx ir=0; ir < tbl.rows(); ++ir){
        if (iCnt > cnt)
            break;

        const char * seq = tbl.cell(ir,0,&seqLen);
        tbl.val(startVar,ir,1);
        tbl.val(endVar,ir,2);
        if (mutAl && mutAl->Qry){
            if( seq!=prev_seq ) {
                for( iMutS = 0 ; iMutS < mutAl->dimAl(); ++iMutS ) {
                    hdr = mutAl->getAl(iMutS);
                    if( strncmp(mutAl->Qry->id( hdr->idQry() ), seq,seqLen )==0) {
                        matchTrain = mutAl->getMatch(iMutS);
                        uncompM.resize(hdr->lenAlign()*2);
                        sBioseqAlignment::uncompressAlignment(hdr,matchTrain,uncompM);
                        break;
                    }
                }
            }
            if( iMutS < mutAl->dimAl() ) {
                startVar.setInt( sBioseqAlignment::remapQueryPosition( hdr,uncompM.ptr(),startVar.asInt() ) );
                endVar.setInt( sBioseqAlignment::remapQueryPosition(hdr,uncompM.ptr(),endVar.asInt() ) );
            }
        }
        const char * type = tbl.cell(ir,3,&typeLen);
        const char * id = tbl.cell(ir,4,&idLen);
        prev_seq = (char *)seq;
        compositeKey.cut(0);
        compositeKey.addString(seq,seqLen);
        compositeKey.addString(",",1);
        compositeKey.printf("%" DEC "",startVar.asInt());
        compositeKey.addString(",",1);
        if (endMax!=0 && (endVar.asInt() > endMax || (endVar.asInt()==-1 || endVar.asInt()==0))) {
            compositeKey.printf("%" DEC "",endMax);
        }
        else {
            compositeKey.printf("%" DEC "",endVar.asInt());
        }
        curStart = startVar.asInt();
        if (curStart != prvStart){
             prvStart = curStart;
             ++iNum;
        }
        if (iNum < cntStart) continue;

        sStr * aa =dictBySeq_PosAndType_id.set(compositeKey.ptr(0),compositeKey.length())->set(type,typeLen)->add(1);
        aa->addString(id,idLen);
        ++iCnt;

    }

    if (!dictBySeq_PosAndType_id.dim()) return 1;
    idx keyLen=0;
    idx keykeyLen=0;
    output.printf(0,"seqID,start,end,idType-id\n");
    sStr idType_id;
    for (idx ik=0; ik < dictBySeq_PosAndType_id.dim(); ++ik){
            if (ik) output.addString("\n",1);
            const char * key = (const char *)dictBySeq_PosAndType_id.id(ik,&keyLen);
            output.addString(key,keyLen);
            output.addString(",",1);

            if (dictBySeq_PosAndType_id.get(key,keyLen)->dim()){
                idType_id.cut(0);
                sDic < sVec <sStr> > * dicdic = dictBySeq_PosAndType_id.get(key,keyLen);
                for (idx iik=0; iik < dicdic->dim(); ++iik) {
                    if (iik) idType_id.addString(";",1);
                    const char * secondKey = (const char *)dicdic->id(iik, &keykeyLen);
                    if (strncmp("listOfConnectedRanges",secondKey,15)==0){
                        idType_id.addString("Join");
                    } else {
                        idType_id.addString(secondKey,keykeyLen);
                    }
                    idType_id.addString(":",1);
                    sVec <sStr> * valueVec= dicdic->get(secondKey,keykeyLen);
                    sDic <idx> checkDuplicate;
                    for (idx iv=0; iv<valueVec->dim(); ++iv){
                        if (checkDuplicate.find(valueVec->ptr(iv)->ptr(0),valueVec->ptr(iv)->length())){
                            continue;
                        }
                        if (iv) idType_id.addString("|",1);
                        idType_id.addString(valueVec->ptr(iv)->ptr(0));
                        *checkDuplicate.set(valueVec->ptr(iv)->ptr(0),valueVec->ptr(iv)->length()) = 1;
                    }
                }
                sString::escapeForCSV(output,idType_id.ptr(0),idType_id.length());
            }
    }


    return 0;
}





