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
#include "ion-tools.hpp"
#include <ion/vax.hpp>
#include <slib/utils/tbl.hpp>

sIon IONReal, * ION=&IONReal;
sVec< sIon > IONS;
sIonWander IWANDER;
sIonTransaction ITR;
sStr vaxNameInclusionList;
sStr vaxNameExclusionList;
sStr vaxValueExclusionList;
sDic < char >  nameInclDic;
sIO dbg(sMex::fBlockDoubling, (sIO::callbackFun)::printf );

sStr ionFiles;
sStr constBuf;

idx __on_ion(sIonTools * iap, const char * cmd, const char * args, const char * ,sVar * pForm)
{
    if(iap->gLazy)
        return 0;

    if(sIs(cmd,"-ionDebug")){
        iap->debug=atoidx(args);
        return 0;
    }
    else if(sIs(cmd,"-ionProtect")) {
        iap->recordProtectQuote=*args;
        return 0;
    }
    else if(sIs(cmd,"-vaxNoQuote")) {
        iap->vaxQuoteDoNotProtect=(char)pForm->ivalue("ignoreQuote",0);
        return 0;
    }
    else if(sIs(cmd,"-ionStream")){
        iap->stream=atoidx(args);
        return 0;
    } else if(sIs(cmd,"-ionLevel")){
        iap->ion_LevelMax=atoidx(args);
        return 0;
    } if(sIs(cmd,"-ionAlias")){
        const char * ptr=constBuf.addString(pForm->value("value"));
        ION->dicLoadAdd(pForm->value("alias"),0,ptr,sLen(ptr));
        return 0;
    }else if(sIs(cmd,"-nameInclude")) {
        idx len;const char * nlist=pForm->value("inclusionListForNames",0,&len);
        sFil f;
        if(strncmp(nlist,"file://",7)==0) {
            f.init(nlist+7,sMex::fReadonly);
            nlist=f.ptr();
            len=f.length();
        }
        if(vaxNameInclusionList.length())vaxNameInclusionList.shrink00(0,1);
        sString::searchAndReplaceSymbols(&vaxNameInclusionList,nlist,len,",;\n",0,0,true,true,true,true);
        if(f.length()) {
            for(const char * ptr=vaxNameInclusionList.ptr(0); ptr; ptr=sString::next00(ptr) ) {
                *nameInclDic.set(ptr)=(char)1;
            }
            nlist=0;
            vaxNameInclusionList.cut(0);
        }
    }else if(sIs(cmd,"-nameExclude")) {
        vaxNameExclusionList.cut(0);
        sString::searchAndReplaceSymbols(&vaxNameExclusionList,pForm->value("exclusionListForNames"),0,",;",0,0,true,true,true,true);
    }
    else if(sIs(cmd,"-valueExclude")) {
        vaxValueExclusionList.cut(0);
        sString::searchAndReplaceSymbols(&vaxValueExclusionList,pForm->value("valueExclusionList"),0,",;",0,0,true,true,true,true);
    }
    else if(sIs(cmd,"-ionFile")){
        ionFiles.add(args);ionFiles.add0(2);
    }else if(sIs(cmd,"-ionRead")){
        ION->destroy();
        for( const char * iof=args; iof; iof=sString::next00(iof) ) {

            IONS.add()->init(iof,sMex::fReadonly);
        }
        ION=IONS.ptr(0);
        return 0;
    }else  if(sIs(cmd,"-ionWrite")){
        ION->destroy();
        ION->init(pForm->value("ionname","sion"),iap->hashmode);
        return 0;
    }else  if(sIs(cmd,"-ionCreate") || sIs(cmd,"-ionCreateLazy") ){
        ION->destroy();
        const char * ionName=pForm->value("ionname","sion");
        sStr tmpI("%s.ion",ionName);
        if(sIs(cmd,"-ionCreateLazy")) {
            const char * dep=pForm->value("dependencyFile");
            if(dep && sFile::exists(tmpI) && sFile::time(dep)<sFile::time(tmpI) ) {
                iap->gLazy=1;
                ION->init(ionName,iap->hashmode);
                return 0;
            }
        }
        ION->openHashMode=iap->hashmode;
        ION->init(ionName,sMex::fMapRemoveFile);
        return 0;

    } else  if(sIs(cmd,"-ionExpect")){
        ION->expect(pForm->value("record"),pForm->ivalue("size"));
        return 0;

    }  else if(sIs(cmd,"-ionParseVAX")  || sIs(cmd,"-ionParseTable") ) {

        bool basicIndex=false;
        if(sIs(cmd,"-ionParseTable") ) {
            basicIndex=true;
            const char * controlFile=
                "record,#R,#,ordered\n"
                "record,name,string,string\n"
                "record,value,string,string\n"
                "record,tbl,string,string\n"
                "relation,row,#R|name|value|tbl,#R,#R|name,#R|value,name|value,name,value,tbl|name,tbl|#R,tbl|#R|name,tbl|name|value,#R|name|value\n";
            ION->constructRecordAndRelationTypes(controlFile, sLen(controlFile));
        }


        sVaxSet vs;
        for( const char * p=args; p; p=sString::next00(p))
        {
            sVax * vax=vs.newVax();
            vax->recordStart=iap->gStart;
            vax->recordCnt=iap->gCnt;
            vax->commentMarker= (iap->gComment != sNotIdx) ? iap->buf.ptr(iap->gComment) : 0;

            if(vaxNameInclusionList.length())vax->inclusionNames=vaxNameInclusionList.ptr();
            if(vaxValueExclusionList.length())vax->exclusionValues=vaxValueExclusionList.ptr();

            vax->setSepar(
                iap->gSeparField!=sNotIdx ? iap->buf.ptr(iap->gSeparField) : 0,
                iap->gSeparRec!=sNotIdx ? iap->buf.ptr(iap->gSeparRec) : 0 ,
                iap->gSeparAttribs !=sNotIdx ? iap->buf.ptr(iap->gSeparAttribs) : 0);
            vax->addHeaderCallback("$ion", sIon::constructRecordAndRelationTypes,(void*)ION);

            if( !(p[0]=='.' && p[1]==0 )) {
                vax->init(sVax::fUseMMap|sVax::fParseImmediate, p);
            }
            p=sString::next00(p);
            vax=vax->init(iap->stream ? sVax::fUseFStream : sVax::fUseMMap ,p);
            if(iap->vaxQuoteDoNotProtect  ){vax->flags|=sVax::fDoNotUseQuoteProtection;}
            vax->reMapChunk=512*1024*1024;
            if(!vax->colCnt()) {
                vax->flags|=sVax::fStopAfterHeaders;
                vax->parse(1);

            }
        }



        if(basicIndex)
            ION->providerLoad(sVaxSet::ionProviderCallbackTable ,(void*)&vs,0,iap->gStart, sFlag(sIon::fDoNotLoadEmpty) ) ;
        else
            ION->providerLoad(sVaxSet::ionProviderCallback,(void*)&vs,0,iap->gStart,sFlag(sIon::fDoNotLoadEmpty));
    }
    else if(sIs(cmd,"-ionMerge")){
        sIon srcION(pForm->value("source_ion"),sMex::fReadonly);
        ION->mergeIons(&srcION);
    }
    else if(sIs(cmd,"-ionMode")){
        iap->hashmode=pForm->ivalue("hashmode",0) ? sMex::fMapMemoryLazyFile : 0 ;
    }

    else if(sIs(cmd,"-ionInfo") || sIs(cmd,"-ionInfoAll") ){
        sIO buf( 0 , (sIO::callbackFun)::printf);
        idx flags=sFlag(sIon::fInfoRecordTypes)|sFlag(sIon::fInfoRelationTypes)|sFlag(sIon::fInfoRecordSummary);
        if( sIs("-ionInfoAll", cmd) )
            flags|=sFlag(sIon::fInfoRecords)|sFlag(sIon::fInfoRelations);
        ION->info(flags,&buf, pForm->value("record-type") , pForm->value("sortFile") , iap->gStart, iap->gCnt  );
    }
    else if(sIs(cmd,"-ionExport") ){
        sIO buf( 0 , (sIO::callbackFun)::printf);
        idx flags=sFlag(sIon::fInfoRecordTypes)|sFlag(sIon::fInfoRelationTypes)|sFlag(sIon::fInfoRelations);

        ION->info(flags,&buf, pForm->value("record-type") , pForm->value("sortFile") , iap->gStart, iap->gCnt , &sIon::vaxExporter );
    }
    else if(sIs(cmd,"-ionSort")){
        const char * sortRelation=pForm->value("relation","0");
        const char * useRecords=pForm->value("useRecords",0);
        const char * sortName=pForm->value("sortName",0);


        sVec <idx> recordTypesUsed;
        ION->sortRelations(sortRelation, useRecords, sortName ,0, 0, &recordTypesUsed, 0 );

        sVec < sIon::RecordResult > rr;rr.resize(2*recordTypesUsed.dim());
        idx cnt=recordTypesUsed.dim() ;
        for( idx ir=0; ir<cnt; ++ir) {
            rr[ir].typeIndex=recordTypesUsed[ir];
            rr[ir+cnt].typeIndex=recordTypesUsed[ir];
        }

        const char * vtreeName=pForm->value("vtreeName",0);
        if(vtreeName){
            ION->buildVTree(sortRelation, sortName, vtreeName, rr.ptr(0), rr.ptr(cnt ),cnt );
        }
    }
    else if(sIs(cmd,"-ionSearch")){
        sVec < idx > searchParams;

        idx relationTypeIndex=pForm->ivalue("relation"), len;
        idx relationHasherIndex=pForm->ivalue("hasher");
        for(const char * ptr=sString::next00(args,2); ptr; ptr=sString::next00(ptr)) {
            searchParams.vadd(1,sConvPtr2Int(ptr));
            len=-sLen(ptr);
            searchParams.vadd(1,sConvInt2Ptr(len,void));

        }

        idx cntList=0;
        sMex toHashBuf;
        sIon::Bucket bucket;bucket.toHash=&toHashBuf;
        ION->getRelationBucketByHashVarg(&bucket, relationTypeIndex, relationHasherIndex,&cntList, searchParams.ptr(0));

        idx cntBlock=1,cntRelationTargets=ION->getRelationTargetsCount(relationTypeIndex);
        sVec < sIon::RecordResult > recordResults; recordResults.resize(cntRelationTargets*cntBlock);

        sStr buf;
        while( bucket.found() ) {
              ION->getRelationsByBucketAndIndex(&bucket, relationTypeIndex, recordResults, &cntBlock) ;

              for ( idx ic=0; ic<cntBlock; ++ic ) {for ( idx i=0 ; i<cntRelationTargets; ++i) {
                  ION->getRecordBody(recordResults+i,&buf,0);
                  ::printf(" %s",buf.ptr(0));
              }
              ::printf("\n");
              }
        }
        ::printf("%" DEC " elements\n",cntList);
    }

    else if( sIs(cmd,"-ionTraverseFile") || sIs(cmd,"-ionTraverse") || sIs(cmd,"-ionTraversePrecompileFile") || sIs(cmd,"-ionTraversePrecompile") ){

        const char* flname=0;

        sFil f;
        if(strncmp(args,"file://",7)==0) {
            flname=args+7;
        }else if( strstr(cmd,"File") ) {
            flname=args;
        }

        if(flname){
            f.init(flname,sMex::fReadonly);
        }

        if(strncmp(args,"iql://",6)==0) {
            sIon_QLibrary::iqLibElement * el=sIon_QLibrary::find(args+6);
            if(!args)
                return 0;
            args=el->iql;
        }

        sIonWander & ts= IWANDER;
        if(ionFiles.ptr()) {
            ts.attachIons(ionFiles.ptr(),sMex::fReadonly,sIdxMax,(sIon *)0);
        }
        else if(IONS.dim()) {
            for ( idx i=0; i< IONS.dim() ; ++i)
                ts.addIon(IONS.ptr(i));
        }else {
            ts.addIon(ION);
        }

        ts.debug=iap->debug;
        ts.recordProtectQuote=iap->recordProtectQuote;
        ts.traverseBuf._funcCallback=(sIO::callbackFun)::printf;
        ts.maxLevelOperations=iap->ion_LevelMax;
        if(iap->gCnt)ts.maxNumberResults=iap->gCnt;
        if (iap->gSeparField != sNotIdx){
            ts.traverseFieldSeparator = iap->buf.ptr(iap->gSeparField);
            if (sIs (ts.traverseFieldSeparator, "null"))
                ts.traverseFieldSeparator= 0;
        }
        if (iap->gSeparRec != sNotIdx){
            ts.traverseRecordSeparator = iap->buf.ptr(iap->gSeparRec);
            if (sIs (ts.traverseRecordSeparator, "null"))
                ts.traverseRecordSeparator= 0;
        }

        sIO err;
        sStr buf;
        ts.traverseCompile(f ? f.ptr() : args ,f.length(),&err,true);

        if( strstr(cmd,"Precompile")!=0 ) {
            if(err)
                ::printf("ERROR:\n%s\n",err.ptr(0));

            ts.printPrecompiled(&buf);
            ::printf("%s\n",buf.ptr(0));
            return 0;
        }

        sDic < sStr > dic;
        ts.resultCumulator=&dic;
        sDic < sMex::Pos  > bigD;
        ts.bigDicCumulator=&bigD;

        const char * table=pForm->value("table");
        if(table) {
            sFil tblFile(table,sMex::fReadonly);
            if(tblFile.ok()) {
                sTbl t;
                t.parse(tblFile.ptr(), tblFile.length());
                ts.traverseTable(&t);
            }
        }else {
            ts.traverse();
        }


        if(iap->gVerbose>0) {
            ts.traverseBuf.printf("--------- Dict Encounters ----------\n");
            ts.traverseView();
            ts.traverseBuf.printf("----------Dict Values ---------\n");
            ts.traverseViewVal();
            ts.traverseBuf.printf("----------Dict Values Tbl---------\n");
            ts.traverseViewValTbl();
            ts.traverseBuf.printf("----------DictTable Values ---------\n");
            ts.traverseViewBigDic2D();
        }

    }




    else if(sIs(cmd,"-ionOpenTransaction")){
        const char * trans=pForm->value("transaction_number_or_filepath");
        idx trid=atoidx(trans);

        if(ION->ok()) ITR.init(ION,0 ); else ITR.init(0,&IWANDER);
        ITR.openTransaction (trid, (ION->ok()) ? 0 : trans, (ION->ok()) ? 0 : ".ion");
    }
    else if(sIs(cmd,"-ionCommitTransaction")){
        ITR.commitTransaction();
    }
    else if( sIs(cmd,"-ionLoadTransactions") || sIs(cmd,"-ionExecuteTransactions") ) {
        if(sIs(cmd,"-ionExecuteTransactions")) {
            ITR.init(0,&IWANDER );
            if(IONS.dim()) {
                for ( idx i=0; i< IONS.dim() ; ++i)
                    IWANDER.addIon(IONS.ptr(i));
            }else {
                IWANDER.addIon(ION);
            }
        } else {
            ITR.init(ION,0);
        }

        sIO io;
        io._funcCallback=(sIO::callbackFun)::printf;
        idx monitoringPeriod=pForm->ivalue("monitoring_period",1000);
        const char * trans=pForm->value("transaction_number_or_folder");
        idx trid=atoidx(trans);
        if(trid) {
            ITR.loadTransactions(&io,trid);
        } else {
            ITR.loadTransactions(&io,trans,"*.iontr",100, monitoringPeriod==-1 ? sNotIdx :  monitoringPeriod*10 );
        }
    }

    return 0;
}
