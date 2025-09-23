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

#include <violin/svc-batcher1.hpp>
#include <violin/hiveproc.hpp>

idx SvcBatcher1::OnExecute(idx req)
{
#ifdef _DEBUG
    fprintf(stderr, "qpride form for req %" DEC ":\n", req);
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

    sStr bSvc;
    if(!svcToSubmit)
        svcToSubmit = formValue("batch_svc",&bSvc);
    if(!svcToSubmit) {
        return 0;
    }
    idx reqPriority = formIValue("reqPriority", 0);

    selfService = ( strcmp(vars.value("serviceName"),"svc-batcher")==0 ) ? true : false;

    stillRunning=0;
    alreadyDone=0;
    killed=0;

    sStr nameTemplate;

    if(!reqGetData(req,"submittedGrpIDs",submittedGrpIDs.mex())){
        if( !doCreateProcesses )
            reqSetData(req,"resultFileTemplate","reqself-");

        const char * svc=svcToSubmit ? svcToSubmit : pForm->value("svc",0);
        sQPride::Service Svc;
        serviceGet( &Svc, svc, 0) ;



        if(batchingDescriptor.dim()==0) {
            const sUsrObjPropsTree * objPropsTree=objs[0].propsTree();
            const sUsrObjPropsNode * batchArr = objPropsTree ? objPropsTree->find("batch_list") : 0;
            for(const sUsrObjPropsNode * batchRow = batchArr ? batchArr->firstChild() : 0; batchRow; batchRow= batchRow->nextSibling()) {
                const sUsrObjPropsNode * batchOn= batchRow ->find("batch_param");if(!batchOn)break;
                const sUsrObjPropsNode * batchVal= batchRow ->find("batch_value");if(!batchVal)break;

                const char * param=batchOn->value();if(!param || !(*param))break;
                idx value=atoidx(batchVal->value());
                batchingDescriptor[param]=value;

            }
        }

        if(batchingDescriptor.dim()==0) {
            logOut(eQPLogType_Error, "Incorrect or missing 'batch_param' and 'batch_value' parameters\n");
            reqSetInfo(req, eQPInfoLevel_Error, "Incorrect or missing batch parameters.");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

        recursiveBatchCollect( );
        sStr log,strObjList;
        submittedGrpIDs.add(valueSets.dim());
        if(doCreateProcesses)
            submittedProcessIDs.add(valueSets.dim());

        idx howManySubmitted=0;
        for ( idx iv=0; iv<valueSets.dim() ; ++iv) {
            sVec< sUsrProc > procObjs;
            idx reqSub;

            idx err=0;
            strObjList.cut(0);
            if(doCreateProcesses) {
                err=sUsrProc::createProcesForsubmission(this , pForm , user, procObjs, &Svc, &strObjList, &log);
                if( err ) {
                    logOut(eQPLogType_Error, "Failed to create process for value set %" DEC ": %s\n", iv, log.ptr());
                    reqSetInfo(req, eQPInfoLevel_Error, "Failed to create process.");
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }
            }


            sDic <idx > * curCombination=valueSets.ptr(iv);
            nameTemplate.printf(0," %s #%" DEC, svcToSubmit, iv+1);

            for( idx i=0 ; i<curCombination->dim() ; ++i ){
                const char * param=(const char * ) curCombination->id(i);
                const char * value=(const char * )valueBuf.ptr(*curCombination->ptr(i));
                sHiveId valueId(value);
                sUsrObj _tmp (*user, valueId);
                sStr b;

                _tmp.propGet("name", &b);
                if (b.length() == 0) {
                    b.printf("%s %s", value, param);
                }

                nameTemplate.printf(" %s",b.ptr());

                if( doCreateProcesses ) {
                    for ( idx ip=0; ip<procObjs.dim() ; ++ip) {
                        procObjs[ip].propSet(param,(const char**)0,&value,1);


                    }
                }else {
                    pForm->inp(param,value,sLen(value)+1);

                }
            }
            if( doCreateProcesses ) {
                for(idx ip = 0; ip < procObjs.dim(); ++ip) {
                    procObjs[ip].propDel("batch_param", 0, 0);
                    const char * a = nameTemplate.ptr();
                    procObjs[ip].propSet("name", 0, &a, 1);
                    procObjs[ip].propSetI("reqPriority", reqPriority < 0 ? reqPriority : -reqPriority);
                }
            } else {
                pForm->inp("batch_param", "", 1);
                pForm->inp("name", nameTemplate.ptr(), nameTemplate.length() + 1);
            }

            idx cntParallel=sHiveProc::customizeSubmission(pForm , user, procObjs.dim() ? procObjs.ptr(0) : 0, &Svc, &log);
            if( !cntParallel ) {
                logOut(eQPLogType_Error, "Failed to customize submission for value set %" DEC ": %s\n", iv, log.ptr());
                reqSetInfo(req, eQPInfoLevel_Error, "Failed to customize submission.");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }

            err=sUsrProc::standardizedSubmission(this , pForm , user, procObjs , cntParallel, &reqSub, &Svc, 0, &strObjList, &log );
            if( err ) {
                logOut(eQPLogType_Error, "Failed to submit process for value set %" DEC ": %s\n", iv, log.ptr());
                reqSetInfo(req, eQPInfoLevel_Error, "Failed to submit process.");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            howManySubmitted+=cntParallel;

            if( !doCreateProcesses ) {
                for( idx i=0 ; i<curCombination->dim() ; ++i ){
                    const char * param=(const char * ) curCombination->id(i);
                    const char * value=(const char * )valueBuf.ptr(*curCombination->ptr(i));
                    reqSetData(reqSub,param,value,sLen(value)+1);
                }
            }


            sVec < idx > reqIds;
            grp2Req(reqSub, &reqIds);
            if( !doCreateProcesses ) {
                for ( idx ir=0; ir<reqIds.dim(); ++ir ) {
                    grpAssignReqID(reqIds[ir],req,ir);
                }
            }
            reqSetAction(&reqIds,eQPReqAction_Run);

            if( !reqProgress(howManySubmitted, iv, valueSets.dim()) ) {
                break;
            }
            submittedGrpIDs[iv]=reqSub;
            if(doCreateProcesses)
                submittedProcessIDs[iv]=procObjs[0].Id();
            ++stillRunning;

        }


        if( selfService && doCreateProcesses ){
            reqProgress(valueSets.dim(), 100, 100);
            reqSetStatus(req, eQPReqStatus_Done);
        }
        else
            reqSetData(req,"submittedGrpIDs",submittedGrpIDs.dim()*sizeof(idx),submittedGrpIDs.ptr());

    }
    else {


        for ( idx ig=0; ig<submittedGrpIDs.dim() ; ++ig) {
            sVec < idx > stat;
            grp2Req(req, &waitedReqs, svcToWaitFor, 0);
            if(waitedReqs.dim())reqGetStatus(&waitedReqs,&stat);

            for ( idx is=0; is<stat.dim() ; ++is ) {
                if(stat[is]<eQPReqStatus_Done)
                    ++stillRunning;
                else if(stat[is]==eQPReqStatus_Done)
                    ++alreadyDone;
                else
                    ++killed;
            }
        }


    }

    if(!selfService) {
        if(alreadyDone+stillRunning+killed)
            reqSetProgress(req,alreadyDone, alreadyDone/(alreadyDone+stillRunning+killed));

        if(killed) {
            reqSetProgress(req,valueSets.dim(), 0);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
    }

    return 0;
}

void SvcBatcher1::recursiveBatchCollect( idx level , sDic < idx > * curCombination )
{
    sDic < idx > local;
    if(!curCombination)curCombination=&local;

    if(level==batchingDescriptor.dim())
    {
        sDic < idx > *d=valueSets.add(1);
        for( idx i=0 ; i<curCombination->dim() ; ++i ){
            const char * param = (const char * ) curCombination->id(i);
            idx valueOfs=*curCombination->ptr(i);
            *d->set( param ) = valueOfs ;
        }
        return ;
    }



    sStr valB,extraBuf00;
    const char * param=(const char* )batchingDescriptor.id(level);
    const char * value00=formValues00(param,&valB);
    idx numberOfElements=sString::cnt00(value00);

     if(numberOfElements==1) {
        sString::searchAndReplaceSymbols(&extraBuf00,value00,0, ";",0,0, true, true, false, true);
        value00=extraBuf00.ptr();
        numberOfElements=sString::cnt00(value00);
    }

    idx piece=batchingDescriptor[level];
    idx chunkCnt=numberOfElements/piece;


    const char * v=value00;
    for ( idx i=0; i<chunkCnt ; ++i) {


        (*curCombination)[param]=valueBuf.pos();
        for (idx is=0;is<piece; ++is, v=sString::next00(v)){
            if(is>0)valueBuf.add(";",1);
            valueBuf.add(v,sLen(v));
        }
        valueBuf.add("\0",1);


        if(level<=batchingDescriptor.dim()-1)
            recursiveBatchCollect( level+1, curCombination );
    }

}
