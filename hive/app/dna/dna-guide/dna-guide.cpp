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
#include <violin/violin.hpp>
#include <slib/std/file.hpp>

#if _DEBUG
    idx debug=1;
#else
    idx debug=0;
#endif

class DnaGuideProc: public sQPrideProc
{
    public:
        DnaGuideProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
            totSubjectSize=0;
            totRowsParsed=0;
            chunkParallel=50000000;
            realParsed=0;
        }
        virtual idx OnExecute(idx);

        virtual sRC OnSplit(idx req, idx &cnt) {
            sUsrObj subjObj(*user,sHiveId(formValue("subject")));
            sStr subPath;subjObj.getFilePathnameX(subPath,"_.iupac.nucl.fa",false);
            sBioseq::IUPAC subjs(subPath);
            if(sFile::size(subPath)==0) {
                prepareIUPACsFromObj("subject", &subjs);
                subjs.finalize();
            }
            subjs.readIUPACbuf(subPath);

            totSubjectSize=0,totRowsParsed=0;
            for(idx is=0; is<subjs.dim(); ++is) totSubjectSize+=subjs.len(is);
            cnt=1+totSubjectSize/chunkParallel;

            return sRC::zero;
        }


        void readScoreMatrix(sBioseqAlignment::CostTable * ct);
        idx prepareIUPACsFromObj(const char * variable, sBioseq::IUPAC * iupacSeqs);

        idx totSubjectSize,totRowsParsed,chunkParallel,realParsed;
        static idx onReqProgress(DnaGuideProc * qp, idx progress , idx progress100)
        {
            qp->reqProgress(progress, progress100+qp->realParsed, qp->chunkParallel);
            return 0;
        }
};


idx DnaGuideProc::prepareIUPACsFromObj(const char * variable, sBioseq::IUPAC * iupacSeqs)
{
    sVec < sHiveId > objIDs;formHiveIdValues(variable, &objIDs);
    for(idx ig=0; ig<objIDs.dim(); ++ig){
        sUsrObj guide(*user,objIDs[ig]);

        const char * ext=guide.propGet("ext");
        sStr path;
        guide.getFilePathname(path,"_.%s",ext);

        if(path.length()) {
            if(strcmp(path.ptr(path.length()-3),".gz")==0) {
                sStr cmd("/bin/gunzip -c %s > ",path.ptr(0));

                sPS ps;
                path.cut(0);reqAddFile(path,"req-%" DEC, objIDs[ig].objId());
                cmd.printf("%s",path.ptr(0));
                ps.execute(cmd);
            }
        }
        iupacSeqs->readIUPACfile(path,ext);

    }
    return 1;
}



void DnaGuideProc::readScoreMatrix(sBioseqAlignment::CostTable * ct)
{

    sSet(ct,0);

    ct->match=formIValue("score_match");
    ct->misMatch=formIValue("score_misMatch");
    ct->bulgeDNA=formIValue("score_bulgeDNA");
    ct->bulgeGuide=formIValue("score_bulgeGuide");
    ct->maxMisMatches=formIValue("maxMisMatches");
    ct->maxGapsTotal=formIValue("maxGapsTotal");
    ct->maxGapsDNA=formIValue("maxGapsDNA");
    ct->maxGapsGuide=formIValue("maxGapsGuide");
    ct->maxNsOnRef=formIValue("maxNsOnRef");

    const char * scoreMat=formValue("iupac-match-matrix");
    sUsrObj mat(*user,sHiveId(scoreMat));
    sStr matPath;mat.getFilePathname(matPath,"_.csv");
    sFil mcsv(matPath);
    sTbl tbl;tbl.parse(mcsv.ptr(),mcsv.length());
    sDic <idx> scores; sStrT t;


    for ( idx ir=1; ir<tbl.cols(); ++ir) {
        char row=*tbl.cell(ir,(idx)0);
        for ( idx ic=1; ic<tbl.cols(); ++ic) {
            char col=*tbl.cell(0,ic);
            ct->matchTbl[256*((int)row)+(int)col]=tbl.ival(ir,ic);
        }
    }

    const char * scoreSeverity=formValue("score-severity");
    sString::scanRangeSet(scoreSeverity,0,&ct->scoreSeverity,0,0,0,0);

}

idx DnaGuideProc::OnExecute(idx req)
{
    sBioseq::IUPAC guides;
    prepareIUPACsFromObj("guide_sequence", &guides);
    guides.readIUPACbuf();

    sUsrObj subjObj(*user,sHiveId(formValue("subject")));
    sStr subPath;subjObj.getFilePathnameX(subPath,"_.iupac.nucl.fa",false);
    sBioseq::IUPAC subjs(subPath);
    if(sFile::size(subPath)==0) {
        prepareIUPACsFromObj("subject", &subjs);
        subjs.finalize();
    }
    subjs.readIUPACbuf(subPath);
    totSubjectSize=0,totRowsParsed=0;
    for(idx is=0; is<subjs.dim(); ++is) totSubjectSize+=subjs.len(is);
    idx thisThreadStart=reqSliceId*chunkParallel;
    idx thisThreadEnd=thisThreadStart+chunkParallel;

    sBioseqAlignment::CostTable ct;
    readScoreMatrix(&ct);
    ct.strictPAM=1;

    sStr Pam;sString::searchAndReplaceSymbols(&Pam,formValue("pam"),0,";,\n",0,0,true,true,false,true,0);
    sStr guidePAM;
    bool bothStrands=formBoolValue("both_strands");

    sDic < sBioseqAlignment::GSMatch > Gs; Gs.init(0,sMex::fSetZero);
    sStr Buf;
    realParsed=0;
    for(idx is=0; is<subjs.dim();  totRowsParsed+=subjs.len(is), ++is) {
        const char * ref=subjs.seq(is);
        idx reflen=subjs.len(is);

        if(totRowsParsed+reflen<thisThreadStart)continue;
        if(totRowsParsed>=thisThreadEnd)break;

        idx refStartPos=thisThreadStart-totRowsParsed;if(refStartPos<0)refStartPos=0;
        idx refEndPos=refStartPos+chunkParallel-realParsed;if(refEndPos>reflen)refEndPos=reflen;


        for(idx ig=0; ig<guides.dim(); ++ig) {

            for( const char * pam=Pam.ptr(); pam; pam=sString::next00(pam)) {
                guidePAM.cut(0);guidePAM.add(guides.seq(ig), guides.len(ig));guidePAM.add(pam);guidePAM.cut(-1);

                idx rshift=refStartPos;
                idx rlen=refEndPos-refStartPos+guidePAM.length();
                if(rshift+rlen>reflen)rlen=reflen-rshift;

#if _DEBUG
if(rlen>10000000)rlen=10000000;
#endif

                ct.lenPAM=sLen(pam);
                sBioseqAlignment::guideSearch(is,+1,rshift,guidePAM.ptr(), guidePAM.length(), ref+rshift,rlen,&ct,&Gs,&Buf, (sCallbackUniversal)onReqProgress,(void*)this,debug);
                if(bothStrands) {
                    sStr rc;sBioseq::iupacRevComplement(&rc,guidePAM.ptr(),guidePAM.length()); rc.cut(-1);
                    sBioseqAlignment::guideSearch(is,-1,rshift,rc.ptr(), rc.length(), ref+rshift,rlen,&ct,&Gs,&Buf, (sCallbackUniversal)onReqProgress,(void*)this,debug);
                }
            }
        }


        realParsed=refEndPos-refStartPos;
    }






    sVec < sHiveId > infoSrcList;formHiveIdValues("info_csv",&infoSrcList);
    sStr srcNames;sDic < idx > hdr;
    sHiveannot hAnnot(user,formValue("annotation_source"),formValue("annotation_enrichment"),&infoSrcList,&srcNames,&hdr);
    idx flanking=formIValue("flanking",500);
    sStrT t,aInf;sString::searchAndReplaceSymbols(&aInf,formValue("annotaton_terms",0,"rsID,ref,alt,CLNVC,CLNDN,AF_ESP,AF_EXAC,AF_TGP"),0,",;",0,0,true,true,false,true,0);
    for (const char * p=aInf.ptr(); p; p=sString::next00(p)) {
        hdr.set(t.printf(0,"%s",p));
        hdr.set(t.printf(0,"%s-POS",p));
    }



    sFil out;reqAddFile(&out,sMex::fForceRemapTruncate,"req-guide-results.csv");
    sStrT buf;

    if(reqSliceId==0){
        out.printf("#HIVE-BCO,SCORE,CHROMOSOME,STRAND,GENE,EXON,MISMATCHES,BULGES,CUT-POS,GUIDE,ALIGNMENT");
        for(idx i=0; i<hdr.dim(); ++i){
            idx lid; const char * id=(const char * )hdr.id(i,&lid);
            out.printf(",%.*s",(int)lid,id);
        }
        out.printf(",FLANK-CUTPOS,FLANK-SEQUENCE\n");
    }


    for( idx i=0; i<Gs.dim(); ++i) {
        sBioseqAlignment::GSMatch * gs=Gs.ptr(i);
        idx idlen;const char * ref=subjs.id(gs->iSub,&idlen);

        sVar info;
        hAnnot.mapPosToGeneInfo(&info,ref,idlen,gs->refStart-flanking,gs->refEnd+flanking, gs->refStart, gs->refEnd);

        out.printf("#%s-%" DEC "-%" DEC "-%" DEC "-%" DEC ,objs[0].IdStr(),reqId,reqSliceId,i+1,gs->iSub+1);
        out.printf(",%" DEC ,gs->score);
        out.printf(",%s" ,hAnnot.chromosome());
        out.printf(",%c",gs->strand==-1 ? '-' : '+');
        out.printf(",%s" ,hAnnot.gene());
        out.printf(",%s" ,hAnnot.value("exon_id"));
        out.printf(",%" DEC ,gs->missMatch);
        out.printf(",%" DEC ,gs->gapDNA+gs->gapGuide);
        out.printf(",%" DEC ,gs->refCut);
        out.printf(",%s",Buf.ptr(gs->guideSeqPos));
        const char * alignRef=Buf.ptr(gs->matchTrainPos),* alignGuide=sString::next00(alignRef),* alignCygar=sString::next00(alignGuide);
        out.printf(",\"%12" DEC ": %s :%-12" DEC "\n",gs->refStart,alignRef,gs->refEnd);
        out.printf("              %s\n",alignCygar);
        out.printf("              %s\"",alignGuide);

        for(idx i=0; i<hdr.dim(); ++i){
            idx lid; const char * id=(const char * )hdr.id(i,&lid);
            const char * value = info.value(id,"",0,lid);
            out.printf(",\"%s\"",value);
        }

        idx rs=sMax( gs->refStart-flanking, (idx)0);
        idx re=sMin(gs->refEnd+flanking, subjs.len(gs->iSub)) ;
        out.printf(",%" DEC ,gs->refCut-rs);
        out.printf(",\"%.*s\"",(int)(re-rs),subjs.seq(gs->iSub)+rs );



        out.printf("\n");
    }

    if( !isLastInGroup() ) {
        reqSetProgress(req, Gs.dim(), 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }
    sStr path;objs[0].getFilePathnameX(path,"guide-matches.csv",false);
    sFile::remove(path);

    sVec < idx > reqList; grp2Req(masterId, &reqList, vars.value("serviceName"));
    sStr locpath;
    for( idx ir=0; ir<reqList.dim(); ++ir){
        locpath.cut(0);
        const char * flnm=reqDataPath(reqList[ir], "req-guide-results.csv",&locpath);
        if(flnm && sFile::size(flnm))
            sFile::copy(flnm,path,true);
    }

    reqProgress(Gs.dim(), 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc,argv);
    DnaGuideProc backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-guide",argv[0]));
    return (int)backend.run(argc,argv);
}

