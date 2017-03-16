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

//#include <ssci/bio.hpp>
#include <violin/svc-batcher.hpp>
#include <violin/violin.hpp>

class DnaHexagonBatcher: public SvcBatcher
{
    public:
        DnaHexagonBatcher(const char * defline00,const char * srv) : SvcBatcher (defline00,srv)
        {

        }
        virtual idx OnExecute(idx);
};

idx cleanOverlappingAlignments(sBioal & ha,idx thresholdOverlap, sVec < sVec < idx > > * qryAl, bool setSelection=false)
{
    // Sort by id Qry
    sVec < idx > subScores(sMex::fSetZero);
    for (idx ial = 0; ial < ha.dimAl(); ial++) {
        sBioseqAlignment::Al * hdr = ha.getAl(ial);
        qryAl->resize(hdr->idQry()+1);
        qryAl->ptr(hdr->idQry())->vadd(1, ial);

        subScores.resize(hdr->idSub()+1);
        subScores[hdr->idSub()]+=hdr->score();
    }

    for (idx iQ = 0; iQ < qryAl->dim(); ++iQ) {
        if (qryAl->ptr(iQ)->dim() == 0) continue;
        idx cntQryAls=qryAl->ptr(iQ)->dim();
        if(!cntQryAls)continue;

        //idx jjQry;
        //idx iiQry;

        sBioseqAlignment::Al * iihdr,* jjhdr;
        for (idx ii=0 ; ii < cntQryAls ; ++ii) {
            if(!*qryAl->ptr(iQ)->ptr(ii) || *qryAl->ptr(iQ)->ptr(ii) < 0 ) continue;
            idx iiAlIndex=*qryAl->ptr(iQ)->ptr(ii);
            iihdr = ha.getAl(iiAlIndex);
            idx * iim=ha.getMatch(iiAlIndex);
            idx iistart=iihdr->qryStart()+iim[1];
            idx iiend=iistart+iihdr->lenAlign();

            idx overlap=0;
            for( idx jj=ii+1; jj<cntQryAls; ++jj) {
                if(*qryAl->ptr(iQ)->ptr(jj) < 0) continue;
                idx jjAlIndex=*qryAl->ptr(iQ)->ptr(jj);
                jjhdr = ha.getAl(jjAlIndex);

                if (jjhdr->idSub() == iihdr->idSub()) {
                    overlap=thresholdOverlap;
                } else {
                    idx * jjm=ha.getMatch(jjAlIndex);
                    idx jjstart=jjhdr->qryStart()+jjm[1];
                    idx jjend=jjstart+jjhdr->lenAlign();

                    idx ostart=sMax(iistart,jjstart);
                    idx oend=sMin(iiend,jjend);

                    overlap=oend-ostart;
                    if(overlap<0)
                        overlap=0;

                    if(overlap) {
                        overlap=overlap*100/(jjend-jjstart);
                        if (overlap < thresholdOverlap) {
                            overlap = overlap*100/(iiend-iistart);
                        }
                    }
                }
                if(overlap>=thresholdOverlap) {
                    //if(iihdr->score()>=jjhdr->score() ) {
                    if(subScores[iihdr->idSub()]>=subScores[jjhdr->idSub()]) {
                        *qryAl->ptr(iQ)->ptr(jj)=(ii * (-1)) -1;
                        if( setSelection ) {
                            jjhdr->setFlagsOff(sBioseqAlignment::fSelectedAlignment);
                            iihdr->setFlagsOn(sBioseqAlignment::fSelectedAlignment);
                        }
                    }else {
                        *qryAl->ptr(iQ)->ptr(ii)= (jj * (-1)) - 1;
                        if( setSelection ) {
                            iihdr->setFlagsOff(sBioseqAlignment::fSelectedAlignment);
                            jjhdr->setFlagsOn(sBioseqAlignment::fSelectedAlignment);
                        }
                        break;
                    }
                }


            }
        }
    }
    return 0;
}

idx DnaHexagonBatcher::OnExecute(idx req)
{
    doCreateProcesses=false;
    svcToWaitFor="dna-hexagon-collector";
    svcToSubmit="dna-hexagon";


    idx thresholdOverlap=80; // percent

    SvcBatcher::OnExecute(req);
    if(alreadyDone == 0 && (killed || stillRunning)) {
        idx percent = ((alreadyDone + killed)/(alreadyDone + killed + stillRunning) * 90);
        reqProgress(alreadyDone + killed, percent, 100);
        return 0;
    }

    if (stillRunning == 0 && killed == 0) {
        reqProgress(alreadyDone + killed, 90, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }

/*
    if(killed) {
        reqProgress(valueSets.dim(), 0, 100);
        reqSetStatus(req, eQPReqStatus_ProgError); // change the status
        return 0;
    }
    else if(stillRunning){
        reqReSubmit(req,60);
        return 0;
    }
*/


    // at this point all subjobs are done - perform the rest of the operations
    //idx bitsInIdx=(sizeof(idx)*8);



    for ( idx ig=0; ig<waitedReqs.dim() ; ++ig) {
        idx grpID=waitedReqs[ig];
        //reqDataPath(grpID,"alignment.hiveal",&path);

        sStr pathHiveAl;
        reqDataPath(grpID,"reqself-alignment.hiveal",&pathHiveAl);
        sHiveal ha(user,pathHiveAl);

        //idx qryNum,subNum;
        //sStr tmp;

        /*{
            reqGetData(grpID,"subject",&tmp);
            sHiveseq sub(tmp);
            subNum=sub.dim();
        }
        */
        /*
        {
            reqGetData(grpID,"query",&tmp,true);
            sHiveseq qry(tmp);
            qryNum=qry.dim();
        }
        */

        sVec < sVec < idx > > qryAl;
        cleanOverlappingAlignments(ha,thresholdOverlap, &qryAl, false);
        sVec <idx> coverages;

        for (idx iQ = 0; iQ < qryAl.dim(); ++iQ) {
            if (qryAl[iQ].dim() == 0) continue;
            idx cntQryAls=qryAl[iQ].dim();
            if(!cntQryAls)continue;
            //idx * qryal=qryAl[iQ].ptr();


            for (idx ii=0 ; ii < cntQryAls ; ++ii) {
                if(*qryAl.ptr(iQ)->ptr(ii)==-1) continue;
                //iihdr = ha.getAl(qryal[ii]);
                //idx _tmp = ha.getAl(qryal[ii]);
                idx alignNumber = *qryAl.ptr(iQ)->ptr(ii);
                if (alignNumber < 0) {
                    alignNumber = (alignNumber * (-1)) - 1;
                }
                if (*qryAl.ptr(iQ)->ptr(ii) > 0) {
                    // Get subject ID
                    idx idSub = ha.getAl(alignNumber)->idSub();
                    // Resize if needed
                    coverages.resize(idSub + 1);
                    idx len = ha.getAl(alignNumber)->lenAlign();
                    // Maintain list of culmative alignment lengths per subject
                    coverages[idSub] += len;

                    /*
                    idx score = ha.getAl(alignNumber)->score();
                    idx start = ha.getAl(alignNumber)->subStart();
                    idx qryStart = ha.getAl(alignNumber)->qryStart();
                    ::printf("Qry: %"DEC"\tAlign: %"DEC"\tScore %"DEC"\tSubID:%"DEC"\tSubStart:%"DEC"\tQryStart:%"DEC"\tLen:%"DEC"\tSubEnd:%"DEC"\tQryEnd:%"DEC"\n", iQ, *qryAl.ptr(iQ)->ptr(ii), score, idSub,start,qryStart,len, len+start, len+qryStart);
                    */
                }
            }

        }

        // Loop through and calculate which meets the 70% threshold of 5x coverage
        // get subject
        //idx mySubID = formIValue("mySubID",0);
        sHiveseq Sub(user, "3019630", sBioseq::eBioModeLong);
        sVec <idx> subjects;

        for (idx i = 0; i < Sub.dim(); i++) {
            idx subLength = Sub.len(i);
            // set at ~75% length required
            subLength *= .75;
            idx _totCov = coverages[i];
            _totCov /= 10;
            if (_totCov > subLength) {
                // Met requirement, mark subID
                subjects.vadd(1, i);
            }
        }

        for (idx i = 0; i < subjects.dim(); i++) {
            idx s = subjects[i];
            ::printf("grpID: %"DEC"  Bacteria %"DEC": %s\n", grpID, s, Sub.id(subjects[i]));
        }
    }





//    reqProgress(cntFound, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);// change the status

    return 0;

}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaHexagonBatcher backend("config=qapp.cfg"__,sQPrideProc::QPrideSrvName(&tmp,"dna-hexagon-batcher",argv[0]));
    return (int)backend.run(argc,argv);
}



//                if(hdr->score()>Scores[hdr->idSub])
  //                  Scores[hdr->idSub]=hdr->score();

    //            idx firstQryPos = hdr->qryStart() + m[1];
        //        idx firstArrPos = hdr->idSub() * maxQryLen + firstQryPos ;
      //          for (idx c = 0; c < hdr->lenAlign(); c++ ) {
          //          QF[ (firstArrPos + c)/bitsInIdx ] |= ((idx)1) << ((firstArrPos + c)%bitsInIdx);
            //    }
            //}
