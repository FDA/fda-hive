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
#include <ssci/math.hpp>
#include "tblqryX4_cmd.hpp"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class EmbHepSleepScratch : public Command
        {
            private:
                idx flank_sleep;

            public:
                EmbHepSleepScratch(ExecContext & ctx) : Command(ctx)
                {
                    flank_sleep=0;
                }

                const char * getName() { return "embHepSleepScratch"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }
                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdEmbHepSleepScratch(ExecContext & ctx) { return new EmbHepSleepScratch(ctx); }

    };
};

bool EmbHepSleepScratch::init(const char * op_name, sVariant * arg)
{

    flank_sleep=0;
    if (sVariant * val = arg->getDicElt("flank_sleep")){
        flank_sleep=atoidx(val->asString());
    }

    return true;
}


struct SScratch {
    idx start, end;
    idx intensity;
} ;

struct SSleep {
    idx start, end;
    idx qual,QOLA;
    sVec < SScratch > scr;
    idx totalScratchDuration,totalScratchIntensity;
} ;

struct SQolie {
    idx tot_score;
    idx tot_day;
    idx tot_scratch;
    SQolie(){
        tot_score=0;tot_scratch=0;tot_day=0;
    }
};

bool EmbHepSleepScratch::compute(sTabular * tbl)
{
    idx cSubid=tbl->colId( "entry__resource__subject__identifier");
    idx cStart=tbl->colId( "entry__resource__effectivePeriod__start");
    idx cEnd=tbl->colId( "entry__resource__effectivePeriod__end");

    idx cResourceType=tbl->colId( "entry__resource__resourceType");
    idx cCodeText=tbl->colId( "entry__resource__code__text");
    idx cHasMemberType=tbl->colId( "entry__resource__hasMember__type");
    idx cCategoryText=tbl->colId( "entry__resource__category__text");

    idx cLinkID=tbl->colId( "entry__resource__item__linkId");
    idx cAnswerValueDecimal=tbl->colId( "entry__resource__item__answer__valueDecimal");
    idx cAnswerValueString=tbl->colId( "entry__resource__item__answer__valueString");
    idx cAuthored = tbl->colId( "entry__resource__authored");

    idx cQuestionnaire= tbl->colId( "entry__resource__questionnaire");
    idx cValueInteger = tbl->colId( "entry__resource__component__valueInteger");
    idx cComponentCodeText=tbl->colId( "entry__resource__component__code__text");


    sVariantTbl * outTbl = new sVariantTbl (8,0);
    sStr out, pat;
    struct tm tmS;
    const char * p; idx lenp;

    const char * timFmt="%Y-%m-%d %H:%M:%S";
    const char * timFmtTZ="%Y-%m-%dT%H:%M:%SZ";
    #define isCell(_v_ir,_v_ic, _v_val) {out.cut(0);p=tbl->printCell(out,_v_ir, _v_ic);lenp=out.length();}if(p && lenp==sLen(_v_val) && memcmp(p,_v_val,lenp)==0)
    #define pCell(_v_ir,_v_ic) {out.cut(0);p=tbl->printCell(out,_v_ir, _v_ic);}p=out.length() ? out.ptr() :"";
    #define parseTime(_v_tim, _v_timstr, _v_fmt) {strptime( _v_timstr, _v_fmt, sSet(&tmS,0,sizeof(tm)) ); _v_tim=mktime(&tmS);}
    #define getSleep(_v_tim) day=((_v_tim)/daysec)*daysec; slp=sleepDic[pat.ptr()].set((const void *)&(day),sizeof((day)));

    idx start,end,authored,day,daysec=3600*24;
    idx iQ;


    SSleep * slp;
    SScratch * scr;
    sDic < sDic < SSleep > > sleepDic(0,sMex::fSetZero);
    sDic < sVec < SScratch > > scratchDic(0,sMex::fSetZero);


    for (idx ir = 0; ir < tbl->rows(); ir++){
        pat.cut(0);p=tbl->printCell(pat,ir,cSubid);
        if(!pat.length())continue;


        isCell(ir,cResourceType,"Observation"){
            isCell(ir,cCodeText,"Sleep Itching"){

                pCell(ir,cStart);parseTime(start,p,timFmt);
                pCell(ir,cEnd);parseTime(end,p,timFmt);

                isCell(ir,cHasMemberType,"Observation"){
                    isCell(ir,cCategoryText,"Activity"){
                        getSleep(end);
                        slp->start=start;
                        slp->end=end;
                    }
                }else isCell(ir,cComponentCodeText,"Intensity"){
                    scr=scratchDic[pat.ptr()].add(1);
                    scr->start=start;
                    scr->end=end;
                    pCell(ir,cValueInteger)scr->intensity=p ? atoidx(p) : 0;
                }
            }
        } else isCell(ir,cResourceType,"QuestionnaireResponse") {

            pCell(ir,cAuthored);parseTime(authored,p,timFmtTZ);


            isCell(ir,cQuestionnaire,"DevItchTracker/questDLQI/1.7/1"){
                pCell(ir,cLinkID);
                    pCell(ir,cAnswerValueString);
                    iQ=0;
                    if(p){
                        if( strcmp(p,"A lot")==0 ) iQ=2;
                        else if( strcmp(p,"A little")==0) iQ=1;
                        else if( strcmp(p,"Not at all")==0) iQ=0;
                        else if( strcmp(p,"No")==0) iQ=0;
                        else if( strcmp(p,"Not relevant")==0) iQ=0;
                        else if( strcmp(p,"Very much")==0) iQ=3;
                    }
                    day=((authored)/daysec)*daysec;
                    slp=sleepDic[pat.ptr()].set((const void *)&(day),sizeof((day)));
                    slp->QOLA+=iQ;
            } else isCell(ir,cLinkID,"q1") {
                pCell(ir,cAnswerValueDecimal);iQ=p ? atoidx(p) : 0;

                getSleep(authored);
                if(iQ)slp->qual=iQ;

            }

        }


    }

    idx idlen;
    for (idx is = 0; is < scratchDic.dim(); is++){
        const char * id=(const char * )scratchDic.id(is,&idlen);
        sDic < SSleep > * vslp=sleepDic.get(id,idlen);
        if(!vslp)continue;

        sVec < SScratch> * vscr=scratchDic.ptr(is);
        for( idx iss=0; iss<vscr->dim(); ++iss){
            SScratch * scr=vscr->ptr(iss);

            for( idx ip=0; ip<vslp->dim(); ++ip){
                SSleep * slp=vslp->ptr(ip);

                if(    scr->start >=slp->start+flank_sleep && scr->start<=slp->end-flank_sleep &&
                    scr->end >=slp->start+flank_sleep && scr->end<=slp->end-flank_sleep ) {
                    slp->scr.vadd(1,*scr);
                    slp->totalScratchIntensity+=scr->intensity;
                    slp->totalScratchDuration+=scr->end-scr->start+1;
                }
            }
        }
    }


    outTbl->setVal(-1, 0, "patient_id");
    outTbl->setVal(-1, 1, "sleep_start");
    outTbl->setVal(-1, 2, "sleep_end");
    outTbl->setVal(-1, 3, "sleep_duration (minutes)");
    outTbl->setVal(-1, 4, "scratch_duration (seconds)");
    outTbl->setVal(-1, 5, "sleep_quality");
    outTbl->setVal(-1, 6, "sleep_scratch_intensity");
    outTbl->setVal(-1, 7, "itch_tracking");

    idx irow=0;
    sDic < SQolie > qolie_dict;
    for (idx ip = 0; ip < sleepDic.dim(); ip++){
        const char * patient_id=(const char * )sleepDic.id(ip,&idlen);
        if (! qolie_dict.get(patient_id,idlen)) {
            qolie_dict.set((const void *)patient_id,idlen);
        }
        SQolie * qolie = qolie_dict.get(patient_id,idlen);

        sDic < SSleep > * vslp=sleepDic.ptr(ip);

        for (idx is = 0; is < vslp->dim(); is++){
            SSleep * slp=vslp->ptr(is);


            if(slp->start==0 && slp->end==0) {
                qolie->tot_scratch+=slp->totalScratchDuration;
                qolie->tot_score+=slp->QOLA;
                continue;
            }
            qolie->tot_day+=1;
            qolie->tot_scratch+=slp->totalScratchDuration;
            qolie->tot_score+=slp->QOLA;

            if (slp->totalScratchDuration < 4) continue;

            char bufS[128]; strftime(bufS,sizeof(bufS),"%Y-%m-%d %H:%M:%S",localtime((time_t *)(&slp->start)));
            char bufE[128];strftime(bufE,sizeof(bufE),"%Y-%m-%d %H:%M:%S",localtime((time_t *)(&slp->end)));
            sStr dur;

            idx sleep_duration = (slp->end - slp->start) / (60);
            outTbl->setVal(irow, 0, patient_id);
            outTbl->setVal(irow, 1, bufS);
            outTbl->setVal(irow, 2, bufE);
            outTbl->setVal(irow, 3, sleep_duration);
            outTbl->setVal(irow, 4, slp->totalScratchDuration);
            outTbl->setVal(irow, 5, slp->qual);
            outTbl->setVal(irow, 6, slp->totalScratchIntensity/(slp->scr.dim() ? slp->scr.dim() : 1));
            dur.printf(0,"%.4f",(slp->totalScratchDuration*1.0/(slp->end - slp->start)));
            outTbl->setVal(irow, 7, dur.ptr());


            ++irow;
        }

    }


    sStr outPutPath;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://summary_qolie.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(),"summary_qolie.csv",&outPutPath);
    sFile::remove(outPutPath);

    sFil qolie_csv(outPutPath);
    qolie_csv.printf(0,"patient_id,number_day_of_scratching,total_number_of_seconds_scratching,average_all_nights,QOLIE\n");
    for (idx ip=0; ip<qolie_dict.dim(); ++ip) {
        const char * patient_id=(const char * )sleepDic.id(ip,&idlen);
        SQolie * qolie=qolie_dict.ptr(ip);
        if (!qolie->tot_scratch) continue;
        qolie_csv.printf("%.*s,%" DEC ",%" DEC ",%.2lf,%" DEC "\n",(int)idlen,patient_id,qolie->tot_day,qolie->tot_scratch,(qolie->tot_scratch*1.0/(qolie->tot_day ? qolie->tot_day : 1)),qolie->tot_score);
    }

    setOutTable(outTbl);
    return true;
}

