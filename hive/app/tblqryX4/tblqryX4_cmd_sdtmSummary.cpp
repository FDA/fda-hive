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

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class SdtmSummaryCommand : public Command
        {
            private:
                sStr measurements00;
                sStr keyWords;
                sVec < sHiveId > sdtmObjs;

                struct paramList {
                        char tbl[16];                      // "lb"
                        char groupBy[32];                  // "STUDYID"
                        char patientField[32];             // "USUBJID"
                        char paramField[32];              //  "LBTESTCD"
                        char paramVal[32];                //  "BILI"
                } cur_params;

                sIonWander * iWander;

            public:

                enum efieldName {
                    eTbl=0, eGroupBy, ePatientField,  eParamField, eParamVal
                };

                SdtmSummaryCommand(ExecContext & ctx) : Command(ctx)
                {
                    init();
                    baseStat[0]="Normal";
                    baseStat[1]=">1xULN";
                    baseStat[2]=">2xULN";
                    baseStat[3]=">3xULN";

                }

                void init() {
                    sSet(&cur_params);
                    iWander =0;
                    bufValues.cut(0);
                }

                struct measurement {
                        sDic <  sDic < sMex::Pos> > m_value;

                        sStr * cur_buf;
                };

                sStr bufValues;
                //const char * baseStat[4]={"Normal",">1xULN",">2xULN",">3xULN"};
                const char * baseStat[4];

                struct studyStat {
                        sDic < sDic < real > > baseStat;
                        sDic < idx > totN;
                        sDic < idx > totBase;
                        idx totParam;
                };

                void updateCurrentParams(const char* tbl, const char * groupBy, const char * patientField, const char * paramField=0, const char * paramVal=0){
                    updateSingleFieldParam(eTbl, tbl);
                    updateSingleFieldParam(eGroupBy, groupBy);
                    updateSingleFieldParam(ePatientField,patientField);
                    if (paramField) {
                        updateSingleFieldParam(eParamField,paramField);
                    }
                    if (paramVal) {
                        updateSingleFieldParam(eParamVal,paramVal);
                    }

                }

                void updateSingleFieldParam (efieldName fn, const char * value);

                idx performSummary(sDic <real> * summary_dic);

                idx performLiverSummary(sStr & measurements00, sDic < studyStat > & summary_dic);

                idx listOfPatientFromOneGroup(const char * group_name, sDic <sStr> * patientDic);


                bool retrieveMeasurementsValues(sDic < studyStat > & summary_dic);

                idx collectionMeasurement(sDic <sStr> * resultDic, sDic < sDic < sDic<real> > > * totP=0);

                static idx measurementCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist );

                void cleanMyWander();

                void launchMyQry(const char * qry, sStr * errMsg, sDic <sStr> * resultDict);

                idx listOfGroups(sDic <sStr> * resultDic);

            public:

                const char * getName() { return "sdtm_summary"; }

                //bool getSdTmDatabase(sUsr * usr,sIonWander & iWander,sVec < sHiveId > & sdtmObj);
                //bool retrieveMeasurementsValues(sIonWander & iWander, const char * p, sDic < sDic <real> > & summary_dic);

                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);

                bool compute(sTabular * tbl);
        };
        Command * cmdSdtmSummaryFactory(ExecContext & ctx) { return new SdtmSummaryCommand(ctx); }
    };
};

// =====================================
//         tqs=[{"op":"sdtmSummary","arg":{"objID":"3101158","measurements":["ALP","ALT","AST","BILI"],"keywords":"liver"}}]";
//
// ======================================

bool SdtmSummaryCommand::init(const char * op_name, sVariant * arg)
{
    // "measurements": is an array
    if (sVariant * measurements = arg->getDicElt("measurements"))
    {
        const char * p=measurements->asString(); idx len = sLen(p);
        idx iskip=0;
        if(*p=='['){++p; len-=1; }; if(*p=='"'){++p; len-=1;};
        if (p[len-2]=='"') iskip+=1; if (p[len-1]==']') iskip+=1;
        len = len -iskip;
        sString::searchAndReplaceSymbols(&measurements00,p,len,",", 0,0,true,false,true, true);
    }

    if (sVariant * objToUse = arg->getDicElt("objID"))
    {
        objToUse->asHiveIds(sdtmObjs);
    }

    if (sVariant * kwVal = arg->getDicElt("keywords"))
    {
        keyWords.printf(0,"%s",kwVal->asString());
    }
    else {
        keyWords.printf(0,"liver");
    }

    return true;
}

// =====================================
//
//
// =====================================


void SdtmSummaryCommand::updateSingleFieldParam (efieldName fn, const char * value){
    switch(fn) {
        case eTbl :{
            strcpy(cur_params.tbl,value);
        } break;
        case eGroupBy: {
            strcpy(cur_params.groupBy,value);
        } break;
        case ePatientField: {
            strcpy(cur_params.patientField,value);
        } break;
        case eParamField: {
            strcpy(cur_params.paramField,value);
        } break;
        case eParamVal: {
            strcpy(cur_params.paramVal,value);
        } break;
        default:
            break;
    }
}


bool getSdTmDatabase(sUsr * usr,sIonWander & iWander,sVec < sHiveId > & objs) {
    //if (!sHiveIon::loadIonFile(usr,objs,iWander,"ion.ion")) {
    if (!sHiveIon::loadIonFile(usr,objs,iWander,"sdtmIon.ion")) {
        return false;
    }

    return true;
}

void SdtmSummaryCommand::cleanMyWander() {

    if (iWander){
        iWander->resetCompileBuf();
        iWander->resetResultBuf();
        iWander->resultCumulator=0;
    }

}

void SdtmSummaryCommand::launchMyQry(const char * qry, sStr * errMsg, sDic <sStr> * resultDict) {

    cleanMyWander();

    if (iWander->traverseCompile(qry, sLen(qry))){
        if (errMsg)
            errMsg->printf(0,"something went wrong");
    }

    if (resultDict){
        iWander->resultCumulator = resultDict;
    }
    iWander->traverse();
}

// for one specific Param
idx SdtmSummaryCommand::listOfPatientFromOneGroup(const char * group_name, sDic <sStr> * patientDic){
    sStr query;
    query.printf(0,"a=find.row(tbl=%s,name=%s,value=%s);",cur_params.tbl,cur_params.groupBy,group_name); // STUDY
    query.printf("b=find.row(tbl=a.tbl,#R=a.#R,name=%s);",cur_params.paramField);            // Measurement rows
    query.printf("c=find.row(tbl=b.tbl,#R=b.#R,value=%s);",cur_params.paramVal);            // Measurement Name
    query.printf("d=find.row(tbl=c.tbl,#R=c.#R,name=%s);",cur_params.patientField);         // patient List
    query.printf("dict(d.value,1);");

    launchMyQry(query.ptr(),0,patientDic);

    return  patientDic->dim();

}

/*


 static idx traverserCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist );

iWander->callbackFunc =  sBioseqSNP::traverserCallback;
iWander->callbackFuncParam = rangeVec;

*/

idx SdtmSummaryCommand::listOfGroups(sDic <sStr> * resultDic) {

    sStr query;
    query.printf(0,"a=find.row(tbl=%s,name=%s);dict(a.value,1);",cur_params.tbl,cur_params.groupBy);
    launchMyQry(query.ptr(),0,resultDic);

    return resultDic->dim();
}

idx SdtmSummaryCommand::measurementCallback (sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist){

    if(!statement->label)
       return 1;

    measurement * mm = (measurement *) wander->callbackFuncParam;

    // 1: #R => row number | 2: name => header name |3: value | 4: tbl => table name

    if( memcmp(statement->label,"b",1)==0) {
                //seqID|pos|record|type|id
            idx rowNum = *((idx*) (reslist[1].body));
            sStr hdr; hdr.addString( (const char *)(reslist[2].body), reslist[2].size);

            const char * listHdr[] = { "STUDYID","USUBJID","LBSTNRHI","LBSTRESN","LBDY"};

            idx ls = sDim(listHdr);

            for (idx ih=0; ih<ls; ++ih) {
                if ( strcmp(hdr.ptr(),listHdr[ih])==0) {
                    sDic < sMex::Pos > * tmpDic = mm->m_value.set(reslist[1].body,reslist[1].size);
                    (*tmpDic->set(hdr.ptr(),hdr.length())).pos= mm->cur_buf->pos();
                    (*tmpDic->set(hdr.ptr(),hdr.length())).size= reslist[3].size;
                    mm->cur_buf->addString((const char *)(reslist[3].body), reslist[3].size);
                    break;
                }
            }
    }

    return 1;
}

idx SdtmSummaryCommand::collectionMeasurement(sDic <sStr> * resultDic, sDic < sDic < sDic<real> > > * totP) {

    sStr query;
    query.printf(0,"a=find.row(tbl=%s,name=%s,value=%s);b=find.row(tbl=%s,#R=a.#R);",cur_params.tbl,cur_params.paramField,cur_params.paramVal,cur_params.tbl);

    iWander->resetCompileBuf();
    iWander->resultCumulator=0;

    if (iWander->traverseCompile(query.ptr(), query.length())){
        // something is wrong
        return 0;
    }


    measurement abc;
    bufValues.cut(0);
    abc.cur_buf = &bufValues;
    iWander->callbackFunc =  SdtmSummaryCommand::measurementCallback;
    iWander->callbackFuncParam = &abc;

    iWander->resetResultBuf();
    iWander->traverse();


    launchMyQry(query.ptr(),0,resultDic);

    if (abc.m_value.dim()) {   // {RowIndex: {"USUBJID": {pos, size}, "STUDYID":{pos,size}, "LBDY":{pos,size},}}

        sDic < sDic < sDic<real> > > tmptotP; // {STUDYID:{USUBJID: {day: value, day: value}}
        if (!totP) {
            totP = &tmptotP;
        }

        char tmpBuf[128];
        sStr delMe;
        // Loop through row index
        for (idx ir=0; ir<abc.m_value.dim(); ++ir) {

            sDic < sMex::Pos > * tmp=abc.m_value.ptr(ir);

            // STUDYID
            sDic < sDic <real> > * vv = totP->set(bufValues.ptr(tmp->get("STUDYID")->pos),tmp->get("STUDYID")->size);

            // USUBJID: {day: value, day1: value1}
            delMe.cut(0);
            delMe.addString(bufValues.ptr(tmp->get("USUBJID")->pos),tmp->get("USUBJID")->size);
            //::printf("%s \n", delMe.ptr());
            sDic <real> * v = vv->set(bufValues.ptr(tmp->get("USUBJID")->pos),tmp->get("USUBJID")->size);

            // USUBJID: {day: value, day1: value1}
            if (tmp->find("LBDY") && tmp->find("LBSTNRHI") && tmp->find("LBSTRESN")) {
                      // UPPER limit of normal  and  get BASE limit of normal
                const char * keyList[2] = {"LBSTNRHI","LBSTRESN"};

                for (idx ik=0; ik < sDim(keyList); ++ik) {
                    const char * curk = keyList[ik];

                    idx sL = tmp->get(curk)->size;
                    memcpy(tmpBuf,bufValues.ptr(tmp->get(curk)->pos),sL); // get the value in form of string of the current key
                    if (tmpBuf[0] && (tmpBuf[0]!='N')) {
                        if (sL +4 <128) {
                            for (idx ic=sL; ic < sL+5; ++ic) {tmpBuf[ic]=0;}
                        }
                        real r; sscanf(tmpBuf,"%lg",&r);  // turn it to a real number

                        sL = tmp->get("LBDY")->size;
                        memcpy(tmpBuf,bufValues.ptr(tmp->get("LBDY")->pos),sL); // get the value in form of string of the current key

                        tmpBuf[sL] = '_'; sL+=1;
                        memcpy(tmpBuf+sL,curk,sLen(curk)); // get the day number in form of string
                        sL+=8;
                        // FORMAT: {1_LBSTRESN: 12.5, 5_LBSTRESN: 15.02, 1_LBSTNRHI: 14.5, 5_LBSTNRHI: 20.02}
                        *(v->set(tmpBuf,sL))=r;
                    }

                }

            }
        }

    }

    return resultDic->dim();
}

idx SdtmSummaryCommand::performSummary(sDic <real> * summary_dic) {

    idx tot=0;
    sDic <sStr> result;
    sStr query;
    // list of group in one table
    idx totGroup = listOfGroups(&result);
   // list of patient by one group
    idx len=0;

    sDic <sStr> patient_dic;

    for (idx ig=0; ig< totGroup; ++ig) {
        const char * grp_name = (const char *)result.id(ig,&len);

        idx patientTot = listOfPatientFromOneGroup(grp_name,&patient_dic);
        if (patientTot) {

        }

    }



    return tot;
}
//  retrieve values for a specific measurement, and do some statistics

bool SdtmSummaryCommand::retrieveMeasurementsValues(sDic < studyStat > & summary_dic){

    sDic < sDic < sDic<real> > > tot_study_patient;
    sDic <sStr> result;
    collectionMeasurement(&result, &tot_study_patient);

    // at this point, we get tot_study_patient consisting of {STUDYID1 : {USUBJID1: {day1: value1, day2: value2}, USUBJID2: {day1: value1}}}

    sStr out;

    // loop through STUDY

    idx tot_hasDayOne = 0, idLen =0;
    real valBase = -1, ratio = -1;

    real normal=0, oneUln=0, twoUln=0, threeUln=0;

    for (idx is=0; is < tot_study_patient.dim(); ++is) {

        sDic < sDic <real> > * cur_study = tot_study_patient.ptr(is);

        tot_hasDayOne = 0;
        // loop through USUBJIDs, i.e, over patients
        for (idx ip=0; ip < cur_study->dim(); ++ip) {

            const char * pat_id = (const char *)cur_study->id(ip, &idLen);
            sDic <real> * days_dic = cur_study->ptr(ip);

            // FORMAT: {1_LBSTRESN: 12.5, 5_LBSTRESN: 15.02, 1_LBSTNRHI: 14.5, 5_LBSTNRHI: 20.02}
            valBase = ratio = -1;

            if (days_dic->find("1_LBSTRESN",10)) {

                tot_hasDayOne +=1;
                real * a = days_dic->get("1_LBSTRESN",10);

                valBase = *a;
            }
            if (valBase!=-1 && days_dic->find("1_LBSTNRHI",10)) {

                real * valUpper = days_dic->get("1_LBSTNRHI",10);
                ratio = *valUpper / valBase;
//                out.printf("%s,%.3lf\n",pat_id,ratio);
                if (ratio >3) {
                    threeUln+=1;
                } else if (ratio > 2) {
                    twoUln+=1;
                } else if (ratio >1) {
                    oneUln+=1;
                } else {
                    normal+=1;
                }
            }

        }

        /*out.printf("===============================================\n");
        out.printf("Current parameter %s\n", cur_params.paramVal);

        out.printf("Normal     : %.@lf \n", (normal/tot_hasDayOne) *100);
        out.printf("1 < x < 2  : %.2lf \n", (oneUln/tot_hasDayOne) *100);
        out.printf("2 < x < 3  : %.2lf \n", (twoUln/tot_hasDayOne) *100);
        out.printf("x > 3      : %.2lf \n", (threeUln/tot_hasDayOne) *100);

        out.printf("==> TOTAL PATIENT: %" DEC " and baseLine: %" DEC " \n", cur_study->dim(), tot_hasDayOne);
*/
        const char * cur_study_id = (const char *)tot_study_patient.id(is,&idLen);
        if (!summary_dic.find(cur_study_id,idLen)) {
            summary_dic.set(cur_study_id,idLen);
        }
        studyStat * curParam = summary_dic.get(cur_study_id,idLen);
        *curParam->totN.set(cur_params.paramVal) = cur_study->dim();
        *curParam->totBase.set(cur_params.paramVal) = tot_hasDayOne;

        sDic <real> * stat =curParam->baseStat.set(cur_params.paramVal);
        for (idx ibs=0; ibs < sDim(baseStat); ++ibs) {
            const char * cur_baseStat = baseStat[ibs];
            if (strncmp(cur_baseStat,"Normal",sLen("Normal"))==0) {
                *stat->set("Normal",6)= (normal/tot_hasDayOne) *100;
            } else if (strncmp(cur_baseStat,">1xULN",sLen(">1xULN"))==0) {
                *stat->set(">1xULN",6)= (oneUln/tot_hasDayOne) *100;
            } else if (strncmp(cur_baseStat,">2xULN",sLen(">2xULN"))==0) {
                *stat->set(">2xULN",6)= (twoUln/tot_hasDayOne) *100;
            } else if (strncmp(cur_baseStat,">3xULN",sLen(">3xULN"))==0) {
                *stat->set(">3xULN",6)= (threeUln/tot_hasDayOne) *100;
            }
        }





    }
    //::printf("%s",out.ptr());


    return true;
}

//      BILI,
//
//


idx SdtmSummaryCommand::performLiverSummary(sStr & measurements00, sDic < studyStat > & summary_dic) {

    updateCurrentParams("lb","STUDYID","USUBJID");

    // loooping over each params value like:
    //              BILI, ASP, ALT
    for (const char * p = measurements00.ptr(0); p ; p = sString::next00(p)) {

        updateSingleFieldParam(eParamField,"LBTESTCD");
        updateSingleFieldParam(eParamVal,p);

        retrieveMeasurementsValues(summary_dic);
    }

    return 1;
}


bool SdtmSummaryCommand::compute(sTabular * tbl)
{
    if (!measurements00.length())
    {
        _ctx.logError("%s operation: bad measurements argument", getName());
        return false;
    }

    // ==================
    // Setting up ionDB

    sIonWander curWander;

    if (!getSdTmDatabase(_ctx.qproc().user,curWander,sdtmObjs)){
        _ctx.logError("%s operation: can not find sdtm database", getName());
        return false;
    }

    iWander = &curWander;

    /* ********************************************
     *          Statistics
     *  *******************************************
     */
    // dict = {"BILI":{"N": 535,""}}





    if (strncasecmp(keyWords,"liver",5)==0) {
        sStr outputPath;
        _ctx.qproc().reqSetData(_ctx.outReqID(), "file://sdtmSummary.csv",0,0);
        _ctx.qproc().reqDataPath(_ctx.outReqID(), "sdtmSummary.csv",&outputPath); // getting the path
        sFile::remove(outputPath); // if for some reasons the file existed, remove it

        sDic < studyStat > summary_dic;

        performLiverSummary(measurements00, summary_dic);
        idx idLen=0;

        sFil delMe(outputPath); // open the file to write
        //sStr delMe;
        delMe.printf("Study Id,Lab Test,total Patients,Missing Base Value (Pct),Normal (Pct)");
        // preparing Header
        for (idx ibs=0; ibs < sDim(baseStat); ++ibs) {
            delMe.printf(",%s",baseStat[ibs]);
        }
        delMe.printf("\n");
        // studyid,param,totN,notBase
        for (idx ist=0; ist < summary_dic.dim(); ++ist) {
            const char * study_id = (const char *) summary_dic.id(ist,&idLen);
            delMe.printf("%.*s",(int)idLen,study_id);
            studyStat * cur_st = summary_dic.ptr(ist);
            // looping through each parameter
            for (idx ipar=0; ipar < cur_st->totN.dim(); ++ipar) {
                const char * cur_par = (const char *) cur_st->totN.id(ipar,&idLen);

                idx * totBase = cur_st->totBase.get(cur_par,idLen);
                real normalPct = ((real)(*totBase)*100/(*cur_st->totN.ptr(ipar)));
                delMe.printf(",%.*s,%" DEC ",%.2lf,%.2lf",(int)idLen,cur_par,*cur_st->totN.ptr(ipar),100-normalPct,normalPct);

                sDic < real > * cur_analysis = cur_st->baseStat.get(cur_par,idLen);

                for (idx ibs=0; ibs < sDim(baseStat); ++ibs) {
                    const char * cur_a = baseStat[ibs];
                    real * val = cur_analysis->get(cur_a,sLen(cur_a));
                    if (!val) {
                        delMe.printf(",0");
                    } else {
                        delMe.printf(",%.2lf",*val);
                    }

                }
                delMe.printf("\n");
            }
        }
        ::printf("%s",delMe.ptr());
    }


    // {patient: rowNum: }




    // ===========================================================
    //  ========================// END // =====================
    // ===========================================================

    return true;

}
