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
#include <qlib/QPrideProc.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>

//#define SINGLE_PROFILE_FILE

struct AARange{
    sBioseqSNP::AnnotAlRange annot;
    idx iReq;
    idx ofsInFile;
    bool isOverlap(AARange * r2) {
        return annot.iSub == r2->annot.iSub && sOverlap(annot.getStart(), annot.getEnd(), r2->annot.getStart(), r2->annot.getEnd());
    }
};

class DnaHeptagonAnnotate : public sQPrideProc
{
    public:
        DnaHeptagonAnnotate(const char * defline00,const char * srv) : sQPrideProc(defline00,srv)
        {
        }
        bool getChunkRanges(idx &annotStart, idx &annotEnd, idx * t_reqSliceId);
        static idx AARangesorter(void * param, void * arr, idx i1, idx i2);
        virtual idx OnExecute(idx);
};

idx DnaHeptagonAnnotate::AARangesorter(void * param, void * arr, idx i1, idx i2)
{
    AARange * aaArray = (AARange *)arr;
    AARange * r1 = aaArray + i1;
    AARange * r2 = aaArray + i2;
    if( r1->annot.iSub != r2->annot.iSub )
        return r1->annot.iSub - r2->annot.iSub;
    else if(r2->annot.getStart() != r1->annot.getStart())
        return r1->annot.getStart() - r2->annot.getStart();
    else if(r2->iReq != r1->iReq)
        return r1->iReq - r2->iReq;
    else
        return r1->ofsInFile - r2->ofsInFile;
}

bool DnaHeptagonAnnotate::getChunkRanges(idx &annotStart, idx &annotEnd, idx * t_reqSliceId = 0)
{
    sVec<idx> annot_Chunks;
    reqGetData(grpId,"annotChunkPos",annot_Chunks.mex());
    if( annot_Chunks.dim() < 1 )
        return 0;
    if(!t_reqSliceId)t_reqSliceId = &reqSliceId;
    idx l_t_reqSliceId = *t_reqSliceId;
    annotStart = annotEnd ;

    annotStart = annot_Chunks[l_t_reqSliceId];
    if( l_t_reqSliceId < annot_Chunks.dim()-1 ) {
        annotEnd = annot_Chunks[l_t_reqSliceId+1];
    } else {
        annotEnd = 0;
    }
    return annotStart + annotEnd + annot_Chunks.dim();
}


idx DnaHeptagonAnnotate::OnExecute(idx req)
{
    sStr errS;

    sBioseqSNP::SNPParams SP;
    SP.lenPercCutoff=formIValue("lenPercCutoff",0);
    SP.maxRptIns=formIValue("maxRptIns",3);
    SP.cutEnds=formIValue("cutEnds",0);
    SP.disbalanceFR=formIValue("disbalanceFR",3);
    SP.minCover=formIValue("minCover",50);
    SP.minFreqPercent=formRValue("minFreqPercent",0);
    SP.entrCutoff=formRValue("entrCutoff",1);
    SP.snpCompare=formIValue("snpCompare",0); // 0 is the consensus
    SP.noiseProfileResolution=formRValue("noiseProfileResolution",sBioseqSNP::noiseProfileResolution);
    SP.freqProfileResolution=formRValue("freqProfileResolution",sBioseqSNP::freqProfileResolution);
    SP.histProfileResolution=formRValue("histProfileResolution",sBioseqSNP::histCoverResolution);
    SP.noiseProfileMax=formRValue("noiseProfileMax",0.01);
    SP.maxLowQua=formIValue("maxLowQua",0);
    SP.useQuaFilter=formIValue("useQuaFilter",20);
    SP.minImportantEntropy=formRValue("minImportantEntropy",0.7);
    SP.filterZeros=formIValue("filterZeros",1);
    SP.minFreqIgnoreSNP=formIValue("minFreqIgnoreSNP",0);
    SP.directionalityInfo=formBoolValue("directionalityInfo",false);
    SP.supportedDeletions=formBoolValue("supportedDeletions",true);

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare the Alignment file
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sHiveId alignerID(formValue("parent_proc_ids"));
    sUsrFile aligner(alignerID,sQPride::user);
    // if profiling stage is not yet done
    sStr path;
    aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
    sHiveal hiveal(user, path);
    sBioal * bioal = &hiveal;
    bioal=&hiveal;
    if( !bioal->isok() || bioal->dimAl()==0){
        errS.printf("No alignments are detected or the alignment file %s is missing or corrupted\n", alignerID.print());
        //logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ load the subject and query sequences
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sStr subject;
    aligner.propGet00("subject", &subject, ";");
    sHiveseq Sub(user, subject.ptr(), hiveal.getSubMode(), false, false, &errS);//Sub.reindex();
    if(Sub.dim()==0) {
        //logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "Reference '%s' sequences are missing or corrupted%s%s", subject.length() ? subject.ptr() : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);

        return 0; // error
    }
    errS.cut0cut();

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Determine the number of references needed to be profiled
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sStr str;

    // load the subject and query sequences
    sStr query;
    aligner.propGet00("query",&query,";");
    sHiveseq Qry(user, query, hiveal.getQryMode(), false, false, &errS);//Qry.reindex();
    if(Qry.dim()==0) {
        reqSetInfo(req, eQPInfoLevel_Error, "Query '%s' sequences are missing or corrupted%s%s", query.length() ? query.ptr() : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);

        return 0; // error
    }
    errS.cut0cut();

    bioal->Sub=&Sub;
    bioal->Qry=&Qry;

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Determine the information about preious stages of computations
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    // profileRegions contain iformation about every segment of every which has been mapped
    sVec< sBioseqSNP::AnnotAlRange > annotRegionsAll;
    reqGetData(grpId,"annotRegions",annotRegionsAll.mex());

    idx annotStart, annotEnd;

    // output chunks has information on the number of chunks to be analyzed
    if( !getChunkRanges(annotStart, annotEnd) ) {
        logOut(eQPLogType_Warning,"Empty chunk");
        reqSetInfo(req, eQPInfoLevel_Warning, "%s","Empty chunk");
        reqSetProgress(req, 0, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }
    if( !annotEnd ) {
        annotEnd = annotRegionsAll.dim();
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Prepare a name for the resulting file profile.vioprof
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sStr profName, reqAAPathX;
    profName.printf(0,"file://aa.vioprofX");
    reqSetData(req,profName,0,0); // create and empty file for this request
    reqDataPath(req,profName.ptr(7),&reqAAPathX);
    sFile::remove(reqAAPathX);

    bool doBreak=false;


    const char * refAnnot=formValue("referenceAnnot");
    sHiveIon hionAnnot(user,refAnnot,0,"ion");
    sIonWander * wander=0;
    if(refAnnot) {
        sStr ionqry("a=find.annot(record= $myrec );unique.1(a.record);");
        sBioseqSNP::createAnnotationIonRangeQuery(ionqry,"a.seqID","a.record");
        wander = hionAnnot.addIonWander("rangesLookUp",ionqry);
    } else {
        reqSetInfo(req, eQPInfoLevel_Error, "Annotations are missing or corrupted");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    SP.computeAAs = true;
    SP.countAAs = true;

    idx totFound = 0;
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ iterate through annotations
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sVec <idx> uncompressMM;

    sVec<AARange> aaRanges;
    sVec<sBioseqSNP::ionRange> rangeVec;
    wander->callbackFunc =  sBioseqSNP::traverserCallback;
    wander->callbackFuncParam = &rangeVec;
    FILE * fpA = 0;
    idx fSize = 0, chunkAl = 0, cumAl = 0, reqAl = 0;
    for(idx is = annotStart; is < annotEnd; ++is)
        reqAl += annotRegionsAll.ptr(is)->iAlDim();

    for(idx is = annotStart; is < annotEnd; ++is) {
        cumAl = 0;
        sBioseqSNP::AnnotAlRange * rng = annotRegionsAll.ptr(is);
        rangeVec.cut();
        wander->setSearchTemplateVariable("$myrec",6,&rng->iRec, sizeof(rng->iRec));
        wander->resetResultBuf();
        wander->traverse();
        idx sublen = Sub.len(rng->iSub);
        const char * subseq = Sub.seq(rng->iSub);
        if( rng->iAlEnd < rng->iAlStart && !rangeVec.dim() ) {
            continue;
        }
        if( rangeVec[0].forward != rng->getStrand() )
            rng->reverse();
        sVec < sBioseqSNP::ProfileAAInfo > Ainf(sMex::fSetZero|sMex::fAlignPage|sMex::fMaintainAlignment);
        Ainf.add(rng->getLength());
        sBioseqSNP::ProfileAAInfo * painf = Ainf.ptr();
        idx cntFound=0;
        for (idx ia=rng->iAlStart; ia<=rng->iAlEnd && rangeVec.dim(); ++ia, ++cumAl) {
            // retrieve a particular alignment
            sBioseqAlignment::Al *  hdr=bioal->getAl(ia);
            idx * m=bioal->getMatch(ia);
            uncompressMM.resize(hdr->lenAlign()*2);
            sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
            // compute its start and end coordinates

            if( sBioseqSNP::snpCountSingleSeq(0, 0, painf, rng->getStart()-1, rng->getLength(),subseq,sublen,
                bioal->Qry->seq(hdr->idQry()),bioal->Qry->len(hdr->idQry()),bioal->Qry->qua(hdr->idQry()), false, hdr,
                uncompressMM.ptr(), &SP, bioal->getRpt(ia),0, 0, rangeVec.ptr()) ) {
                ++totFound;
                ++cntFound;
            }

            if( (cumAl%1000)==0 && !reqProgress(totFound, chunkAl + cumAl, reqAl) ) {
                doBreak = true;
                break;
            }
        }
        if(cntFound) {
            AARange * farng = aaRanges.add();
            farng->annot = *rng;
            farng->ofsInFile = fSize;
            farng->iReq = reqId;
            if(!fpA)fpA=fopen(reqAAPathX,"a");
            if(fpA){
                // we do this through mex because these are page aligned object arrays and dim() might not be actual allocated size
                idx written=fwrite((char*)Ainf.mex()->ptr(),1,Ainf.mex()->pos(),fpA);
                fSize+=written;
            }
        }
        chunkAl += rng->iAlDim();
        if( !reqProgress(totFound, chunkAl, reqAl) ) {
            doBreak = true;
            break;
        }
    }

    if(fpA)fclose(fpA);

    if( doBreak ) {
        return 0;
    }
    reqSetData(req,"aaRanges",aaRanges.mex());
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Determine if this request is the last one in its group and if not - finish
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    if( !isLastInMasterGroup() ) {
        reqSetProgress(req, totFound, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }
    aaRanges.cut(0);
    sVec < idx > reqList; grp2Req(masterId, &reqList, vars.value("serviceName"));

    for( idx ir=0; ir<reqList.dim() ; ++ir ){
        reqGetData(reqList[ir],"aaRanges",aaRanges.mex());
    }
    sSort::sortSimpleCallback(DnaHeptagonAnnotate::AARangesorter, 0, aaRanges.dim(),aaRanges.ptr());
    idx rstart = 0, rend = 0;
    sStr subname,fN;sQPrideProc::reqAddFile( fN, "AAprofile.csv");
    sFil ffl(fN.ptr());
    ffl.addString("REFERENCE,NUCPOS,AAPOS,TCOV,AAREF,AASUB,VCOV,AAFREQ,START,END\n");
    sBioseqSNP::ProfileAAInfo * ainf = 0, * aine = 0;
    sDic< sVec<sBioseqSNP::ProfileAAInfo > > scannedRanges;
    sVec< AARange * > overlapRanges;
    idx aatot = 0, lastSub = -1, lastPos = -1;
    for(idx i = 0 ; i < aaRanges.dim() ; ++i ) {
        overlapRanges.cut(0);
        bool existed = false;
        AARange * mrng = aaRanges.ptr(i);
        if( lastPos >= 0 ) {
            if(mrng->annot.iSub == lastSub && sOverlap(lastPos, lastPos, mrng->annot.getStart(),mrng->annot.getEnd())) {
                //continue
            } else if(mrng->annot.iSub != lastSub || mrng->annot.getStart() > lastPos ) {
                lastPos = -1;
                lastSub = -1;
                scannedRanges.empty();
            } else { //same subID but range scanned already (curPos > mrng->annot.oEnd)
                sVec<sBioseqSNP::ProfileAAInfo> * pAinf =  scannedRanges.get((const void*)mrng, sizeof(AARange));
                if(pAinf && pAinf->dim()) {
                    pAinf->destroy();
                }
                continue;
            }
        }
        sString::copyUntil(&subname,Sub.id(mrng->annot.iSub),0," ");
        if( scannedRanges.get((const void*)mrng,sizeof(AARange)) ) {
            existed = true;
        }
        sVec<sBioseqSNP::ProfileAAInfo> * pAinf =  scannedRanges.set((const void*)mrng, sizeof(AARange));
        if(!existed){
            fN.cut0cut();reqDataPath(mrng->iReq,"aa.vioprofX",&fN);
            pAinf->init(sMex::fSetZero|sMex::fAlignPage|sMex::fMaintainAlignment, fN.ptr(), mrng->ofsInFile, mrng->annot.getLength());
        }
        *overlapRanges.add() = mrng;
        for(idx j = i + 1 ; j < aaRanges.dim() ; ++j) {
            AARange * crng = aaRanges.ptr(j);
            existed = false;
            if( crng->annot.iSub == mrng->annot.iSub && crng->annot.getStart() < mrng->annot.getEnd() )  {
                if( scannedRanges.get((const void*)crng,sizeof(AARange)) )
                    existed = true;
                sVec<sBioseqSNP::ProfileAAInfo> * bAinf =  scannedRanges.set((const void*)crng, sizeof(AARange));
                if(!existed){
                    fN.cut0cut();reqDataPath(crng->iReq,"aa.vioprofX",&fN);
                    bAinf->init(sMex::fSetZero|sMex::fAlignPage|sMex::fMaintainAlignment, fN.ptr(), crng->ofsInFile, crng->annot.getLength());
                }
                *overlapRanges.add() = crng;
            } else {
                break;
            }
        }
        rstart = lastPos < 0 ? mrng->annot.getStart():lastPos;
        rend = mrng->annot.getEnd();
        idx pos = rstart, p=0;

        for( ; pos <= rend ; ++pos) {
            for( idx s = 0 ; s < overlapRanges.dim() ; ++s) {
                mrng = overlapRanges[s];
                if( s && !sOverlap(pos, pos,mrng->annot.getStart(),mrng->annot.getEnd()))
                    continue;
                pAinf = scannedRanges.get((const void*)mrng,sizeof(AARange));
                ainf = pAinf->ptr(0);
                p = pos - mrng->annot.getStart();

                aine = ainf + p;
                aatot = aine->coverage();
                if(aatot) {
                    for(idx ico=0 ; ico< sDim(ainf->aa); ++ico ) {
                        if(aine->aa[ico]<aatot*0.01)
                            continue;
                        if((idx) aine->ref==ico)
                            continue;
                        ffl.printf("%s,%" DEC ",%d,%" DEC ",%s,%s,%d,%.2lf,%" DEC ",%" DEC "\n", subname.ptr(), pos+1, aine->pos+1, aatot,sBioseq::mappedCodon2AA(aine->ref)->let,sBioseq::mappedCodon2AA(ico)->let,aine->aa[ico], aine->aa[ico]*1./aatot, mrng->annot.Start, mrng->annot.End);
                        ico++;
                        ico = ico-1;
                    }
                }
            }
        }
        lastPos = p;
        lastSub = mrng->annot.iSub;
        if( !reqProgress(i, aaRanges.dim(), reqAl) ) {
            doBreak = true;
            break;
        }
    }
    if( doBreak ) {
        return 0;
    }
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ collapse the regions from different request if they belong to the same subject and same region
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    logOut(eQPLogType_Info,"\n\nCoalescing results\n");

    progress100Start=50;
    progress100End=100;
    reqSetProgress(req, totFound, progress100Start );



    if( !doBreak ) {
        reqSetProgress(req, totFound, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }
    return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaHeptagonAnnotate backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-heptagon-annotate",argv[0]));
    return (int)backend.run(argc,argv);
}
