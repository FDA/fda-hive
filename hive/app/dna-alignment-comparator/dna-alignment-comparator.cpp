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
#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

#include <slib/utils.hpp>

#include <qpsvc/qpsvc-dna-hexagon.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>

class DnaAlignmentComparator: public sQPrideProc
{
    public:
        DnaAlignmentComparator(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
        struct Accum {
            idx hitsRpt;
            real RPKM;
            real FPKM;
            real num, min, max;
            Accum(){
                sSet(this,0);
            }
        };
};


idx DnaAlignmentComparator::OnExecute(idx req)
{

    sStr cbuf;
    bool doBreak = false;

    sVec<sHiveId> alIds;
    sHiveId::parseRangeSet(alIds, objs[0].propGet00("aligners",0,","));
    if(!alIds.dim()) {
        reqSetInfo(req, eQPInfoLevel_Error, "No alignment information available\n" );
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    real minRPKM=formIValue("minRPKM",0);
    real minFPKM=formIValue("minFPKM",0);
    real minHits=formIValue("minHits",0);
    idx valueToUse=formIValue("valueToUse");

    idx seqClassfy=formIValue("seqClassify",0);



    const char * annotationToUse=formValue("annotationToUse");
    sHiveIon hionAnnot(user,annotationToUse,0,"ion");
    const char * collapseBy=formValue("collapseBy",&cbuf);
    const char * collapseId=formValue("collapseId");

    sIonWander * wander=0;
    if(collapseId && collapseBy)
        wander=hionAnnot.addIonWander("replacer","a=find.annot(id=\"$id\",type=\"%s\");unique.1(a.record);b=find.annot(seqID=a.seqID,record=a.record,type=\"%s\");unique.1(b.type);blob(b.id);",collapseBy,collapseId);

    cbuf.cut(0);
    const char * mapregBy=formValue("mapRegBy",&cbuf);
    const char * mapregId=formValue("mapRegId");
    sHiveIon hionMapRegAnnot(user,annotationToUse,0,"ion");
    if(mapregBy && mapregId)
        hionMapRegAnnot.addIonWander("mapreg","a=find.annot(id=\"$id\",type=\"%s\");unique.1(a.record);b=find.annot(seqID=a.seqID,record=a.record,type=\"%s\");unique.1(b.type);blob(b.id);",collapseBy,collapseId);

    cbuf.cut(0);



    idx cntFound = 0, totWork = alIds.dim(), curWork = 0;


    sDic < sDic < Accum > > accum;
    sDic < idx > subList;subList.flagOn(sMex::fSetZero);
    sVec < sBioal::Stat > statistics;

    
    sDic < idx > qryHitRefList;
    sVec < idx > idQryOfs(sMex::fSetZero);idQryOfs.add(1);
    sStr subListHit;
    sVec < idx > combKey; combKey.add(alIds.dim());
    sDic < idx > combKeyCount;combKeyCount.flagOn(sMex::fSetZero);
    sDic < sStr > combKeySeqs;
    idx singles[2];

    bool isAllPairedEnd = true;

    for(idx iAli=0, cntAls=alIds.dim(); iAli< cntAls; ++iAli) {
        alIds[iAli].print(cbuf);
        sHiveal als(user,cbuf.ptr());cbuf.cut(0);

        sUsrObj alo(*user, alIds[iAli]);if(!alo.Id())continue;

        QPSvcDnaHexagon::getSubject00(alo,cbuf);
        sHiveseq sub(user,cbuf.ptr(), als.getSubMode()); cbuf.cut(0);

        QPSvcDnaHexagon::getQuery00(alo,cbuf);
        sHiveseq qry(user,cbuf.ptr(), als.getQryMode()); cbuf.cut(0);

        alIds[iAli].print(cbuf);

        cbuf.add(" ",1);alo.propGet00("name",&cbuf," ");cbuf.shrink00();
        sDic < Accum > * pacc=accum.set(cbuf.ptr()); cbuf.cut(0);
        pacc->flagOn(sMex::fSetZero);


        als.Sub=&sub;
        statistics.cut(0);
        als.countAlignmentSummaryBySubject(statistics);
        bool isPairedEnd = als.isPairedEnd();
        isAllPairedEnd &= isPairedEnd;

        idx totFound = 0, totFoundRpt = 0, totFrag = 0, totFragRpt = 0;
        for ( idx isub=0, cntSub=als.Sub->dim(); isub<cntSub; ++isub ){
            sBioal::Stat * ps=statistics.ptr(isub+1);
            totFound+=ps->found;
            totFoundRpt+=ps->foundRpt;
            if( isPairedEnd ) {
                sBioal::Stat * ps=statistics.ptr(als.Sub->dim()+isub+1);
                totFrag+=ps->found;
                totFragRpt+=ps->foundRpt;
            }
        }
        if(!totFoundRpt)continue;

        real RPKMcoef=1000000.*1000./totFoundRpt, rpkm = 0;
        real FPKMcoef=1000000.*1000./totFragRpt, fpkm = 0;

        for ( idx isub=0, cntSub=als.Sub->dim(); isub<cntSub; ++isub ){
            sBioal::Stat * ps=statistics.ptr(isub+1);

            if( ps->foundRpt ==0)
                continue;
            if( ps->foundRpt < minHits  || (rpkm=ps->foundRpt*RPKMcoef/sub.len(isub)) < minRPKM )
                continue;
            if( isPairedEnd ) {
                sBioal::Stat * ps=statistics.ptr( als.Sub->dim() + isub + 1);
                if( ps->foundRpt ==0)
                    continue;
                if( ps->foundRpt < minHits  || (fpkm=ps->foundRpt*FPKMcoef/sub.len(isub)) < minFPKM )
                    continue;
            }
            const char * seqid=sub.id(isub);
            idx lenId=sLen(seqid)+1;

            if(wander) {
                const char * pSpace = strchr(seqid,' ');
                lenId = (pSpace) ? (pSpace - seqid) : lenId;
                wander->setSearchTemplateVariable("$id",3,seqid,lenId);
                wander->traverse();
                idx *p=wander->traverseBuf.length() ? (idx * )wander->traverseBuf.ptr(0)  : 0 ;
                if (!p) {
                    for(lenId=0;(!strchr("." sString_symbolsSpace,seqid[lenId]));++lenId);
                    wander->setSearchTemplateVariable("$id",3,seqid,lenId);
                    wander->traverse();
                    p=wander->traverseBuf.length() ? (idx * )wander->traverseBuf.ptr(0)  : 0 ;
                }
                if(p) {
                    lenId=p[0];
                    seqid=sConvInt2Ptr(p[1],const char );
                }
            }

            Accum * pAlSub=pacc->set(seqid,lenId);
            pAlSub->hitsRpt+=ps->foundRpt;
            if( pAlSub->num == 0 ) {
                pAlSub->min = rpkm;
                pAlSub->max = rpkm;
            } else {
                if( pAlSub->min > rpkm ){
                    pAlSub->min = rpkm;
                }
                if( pAlSub->max < rpkm ){
                    pAlSub->max = rpkm;
                }
            }
            pAlSub->num += 1;
            pAlSub->RPKM+=rpkm;
            pAlSub->FPKM+=isPairedEnd?fpkm:0;


            (*subList.set(seqid,lenId))++;

            ++cntFound;

            if(wander)
                wander->resetResultBuf();
            if(isub%10000==0) {if( !reqProgress(cntFound, curWork, totWork) ) {doBreak = true;break;}}
        }


        if( !doBreak && seqClassfy) {
            
            for ( idx ial=0, cntAl=als.dimAl(); ial<cntAl; ++ial ){
                    
                sBioseqAlignment::Al * hdr = als.getAl(ial);
                    
                idx * pofs=qryHitRefList.set( qry.id(hdr->idQry()) ); 
                if(*pofs==0){*pofs=idQryOfs.dim();idQryOfs.add(cntAls);}
                pofs=idQryOfs.ptr(*pofs);
                const char * seqid=sub.id(hdr->idSub());
                subList.find((const void*)seqid,sLen(seqid)+1,&(pofs[iAli]));
                ++pofs[iAli];

                if(ial%10000==0) {if( !reqProgress(cntFound, curWork, totWork) ) {doBreak = true;break;}}
            }
            
        }


            if( doBreak || !reqProgress(cntFound, curWork, totWork) ) {
                doBreak = true;
                break;
            }
        ++curWork;
    }



    #define outCell() { \
        const char * seqid=(const char*)subList.id(isub); \
        Accum * pAlSub=pacc->get(seqid,sLen(seqid)+1); \
        if( !pAlSub ){ \
            if(vU&0x01)out.add(",0",2); \
            if(vU&0x02)out.add(",0",2); \
        } else { \
            if(vU&0x01)out.printf(",%" DEC,pAlSub->hitsRpt); \
            if(vU&0x02)out.printf(",%.5lg",(real)pAlSub->RPKM); \
        } \
    }
    const char * what[3]={"Hits","RPKM","FPKM"};
    sStr bufPath, failedPath00;
    bool printStatsRPKM = ((valueToUse & 0x02) && wander) ? true : false;
    sIO outStats, outMean;
    if (printStatsRPKM){
        const char * dstPath = reqAddFile(bufPath, "activity-statsRPKM.csv");
        if( !dstPath ) {
            failedPath00.printf("activity-statsRPKM.csv");
            failedPath00.add0();
        }
        else {
            outStats.init(dstPath, sMex::fMapRemoveFile);
        }
        bufPath.cut(0);
        const char * meanPath = reqAddFile(bufPath, "activity-meanRPKM.csv");
        bufPath.add0cut();
        if( !meanPath ) {
            failedPath00.printf("activity-statsRPKM.csv");
            failedPath00.add0();
        }
        else {
            outMean.init(meanPath, sMex::fMapRemoveFile);
        }
    }

    logOut(eQPLogType_Debug, "Analyzing results\n");
      {
        idx maxloops = isAllPairedEnd?3:2;

        for( idx vTU=0; vTU<maxloops; ++vTU) {
            idx vU=(vTU+1)&valueToUse;
            if(!vU)continue;
            const char * dstPath = reqAddFile( bufPath,"activity-%s.csv",what[vTU]);
            if( !dstPath ) {
                failedPath00.printf("activity-%s.csv",what[vTU]);failedPath00.add0();
                continue;
            }
            sIO out;out.init(dstPath,sMex::fMapRemoveFile);
            out.printf("reference");
            printStatsRPKM = ((vU & 0x02) && wander);
            if (printStatsRPKM){
                outStats.addString("reference");
                outMean.addString("reference");
            }
            for(idx iac=0, cntAcc=accum.dim() ; iac<cntAcc; ++iac ) {
                out.printf(",\"%s\"",(const char*)accum.id(iac));
                if (printStatsRPKM){
                    const char *printName = (const char*)accum.id(iac);
                    outStats.printf(",\"%s-num\"",printName);
                    outStats.printf(",\"%s-mean\"",printName);
                    outStats.printf(",\"%s-min\"",printName);
                    outStats.printf(",\"%s-max\"",printName);
                    outMean.printf(",\"%s\"",printName);
                }
            }
            out.printf("\n");
            if( printStatsRPKM ) {
                outStats.addString("\n");
                outMean.addString("\n");
            }

            for(idx isub=0, cntSub=subList.dim() ; isub<cntSub; ++isub ) {
                if(!subList[isub])
                    continue;

                idx lenId;
                const char * seqid=(const char*)subList.id(isub,&lenId);
                out.printf("\"%.*s\"",(int)lenId,seqid);
                if( printStatsRPKM ) {
                    outStats.printf("\"%.*s\"",(int)lenId,seqid);
                    outMean.printf("\"%.*s\"",(int)lenId,seqid);
                }

                for(idx iac=0, cntAcc=accum.dim() ; iac<cntAcc; ++iac ) {
                    sDic < Accum > * pacc=accum.ptr(iac);

                    Accum * pAlSub=pacc->get(seqid,lenId);
                    if( !pAlSub ){
                        out.add(",0",2);
                        if (printStatsRPKM){
                            outStats.add(",0,0,0,0",8);
                            outMean.add(",0",2);
                        }
                    } else {
                        if( vU == 1 ){
                            out.printf(",%" DEC, pAlSub->hitsRpt);
                        }
                        if( vU & 0x02 ) {
                            if( !isAllPairedEnd || vTU == 1 ){
                                out.printf(",%.5lg", (real) pAlSub->RPKM);
                                if (printStatsRPKM){
                                    if (pAlSub->RPKM == 0){
                                        outStats.add(",0,0,0,0",8);
                                        outMean.add(",0",2);
                                    }
                                    else {
                                        outStats.printf(",%" DEC, (idx)pAlSub->num);
                                        outStats.printf(",%.5lg", (real) pAlSub->RPKM / pAlSub->num);
                                        outStats.printf(",%.5lg", (real) pAlSub->min);
                                        outStats.printf(",%.5lg", (real) pAlSub->max);
                                        outMean.printf(",%.5lg", (real) pAlSub->RPKM / pAlSub->num);
                                    }
                                }
                            }
                            else{
                                out.printf(",%.5lg", (real) pAlSub->FPKM);
                            }
                        }
                    }
                }
                out.printf("\n");
                if (printStatsRPKM){
                    outStats.printf("\n");
                    outMean.printf("\n");
                }
            }
        }

    }
      
      
    if(seqClassfy) {
        bufPath.add0();
        const char * dstPath = reqAddFile( bufPath,"refmap.csv");
        if( dstPath ) {
            sIO out;out.init(dstPath,sMex::fMapRemoveFile);
            out.printf("sequence");
            for(idx iAli=0, cntAls=alIds.dim(); iAli< cntAls; ++iAli) {
                out.printf(",%" DEC,alIds[iAli].objId());
            }
            out.printf("\n");

            for ( idx iqry=0, cntQry=qryHitRefList.dim(); iqry<cntQry; ++iqry ){
                idx idlen;
                const char * qid=(const char*) qryHitRefList.id(iqry,&idlen);
                out.printf("\"%s\"",qid);
                idx * pofs = qryHitRefList.ptr(iqry);
                pofs=idQryOfs.ptr(*pofs);

                for(idx iAli=0, cntAls=alIds.dim(); iAli< cntAls; ++iAli) {
                    if(pofs[iAli])
                        out.printf(",\"%s\"",subList.id(pofs[iAli]-1));
                    else
                        out.add(",",1);
                    combKey[iAli]=pofs[iAli]-1;
                }

                out.add("\n",1);

                idx rpt=1;
                const char * rpts=strstr(qid,"rpt=");
                if(rpts)sIScanf(rpt,rpts+4,idlen-4,10);

                idx * pcnt=combKeyCount.set(combKey.ptr(0),combKey.dim()*sizeof(idx));
                ++(*pcnt);

                sStr * sTmp = combKeySeqs.set(combKey.ptr(0),combKey.dim()*sizeof(idx));
                sTmp->printf("%s;", qid);

                for( idx ic=0; ic<combKey.dim(); ++ic ) {
                    singles[0]=ic;
                    singles[1]=combKey[ic];
                    pcnt=combKeyCount.set(singles,2*sizeof(idx));
                    ++(*pcnt);
                    sTmp = combKeySeqs.set(singles,2*sizeof(idx));
                    sTmp->printf("%s;", qid);
                }

            }
        } else {
            failedPath00.printf("refmap.csv");
            failedPath00.add0();
        }


        bufPath.add0();
        dstPath = reqAddFile( bufPath,"combmap.csv");
        if( dstPath ) {
            sIO out;out.init(dstPath,sMex::fMapRemoveFile);

            for(idx iAli=0, cntAls=alIds.dim(); iAli< cntAls; ++iAli) {
                out.printf("%" DEC ",",alIds[iAli].objId());
            }
            out.printf("count,libs");
            out.printf("\n");
            const char * seqid;

            for(idx itMode=0; itMode<2; ++itMode ) {

                for(idx iComb=0, cntComb=combKeySeqs.dim(); iComb< cntComb; ++iComb) {
                    idx seqlen, idlen, * combKey=(idx * )combKeyCount.id(iComb,&idlen);

                    if( (idlen==(idx)sizeof(idx)*alIds.dim() && itMode==1) ||
                        (idlen!=(idx)sizeof(idx)*alIds.dim() && itMode==0) )
                        continue;

                    for( idx iAli=0; iAli<alIds.dim(); ++iAli ) {
                        if(idlen==(idx)sizeof(idx)*alIds.dim()){
                            seqid=(const char*)subList.id(combKey[iAli],&seqlen);
                            if(seqlen)out.printf("\"%s\",",seqid);
                            else out.add(",",1);
                        } else {

                            if(iAli==combKey[0] ) {
                                seqid=(const char*)subList.id(combKey[1],&seqlen);
                                if(seqlen)out.printf("\"%s\",",seqid);
                                else out.add(",",1);

                            }
                            else
                                out.printf("-any-,");
                        }
                    }
                    out.printf("%" DEC ",\"%s\"\n",combKeyCount[iComb], combKeySeqs[iComb].ptr());
                }
            }
        } else {
            failedPath00.printf("combmap.csv");
            failedPath00.add0();
        }
    }
    
    if( failedPath00.ptr() ){
        bufPath.cut(0);
        failedPath00.add0();
        for( const char * fn=failedPath00.ptr(); fn; fn=sString::next00(fn) ) {
            bufPath.printf(",%s",fn);
        }
        reqSetInfo(req, eQPInfoLevel_Error, "Failed to add %s", bufPath.ptr(1) );
        logOut(eQPLogType_Error,"Failed to add %s", bufPath.ptr(1));
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    reqSetProgress(req, cntFound, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    PERF_PRINT();

    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaAlignmentComparator backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-alignment-comparator", argv[0]));
    return (int) backend.run(argc, argv);
}







