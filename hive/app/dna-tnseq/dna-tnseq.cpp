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

#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>

class DnaTNSeq: public sQPrideProc
{
    public:
        DnaTNSeq(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);

        struct PosDef {
                idx cnt,cntRpt;
                idx len,dir;
                idx alid;
        };
        struct GeneSpecInfo {
                idx inserts,coverage;
                idx forward, reverse;
                idx histogOffset,histogLen, histogOffsetStart;
        };

        idx generateHeatMap(sStr & pathToTable, const char * baseName);
};

idx DnaTNSeq::generateHeatMap(sStr & pathToTable, const char * baseName) {
        sTxtTbl * tbl = new sTxtTbl();
        tbl->setFile(pathToTable);
        tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fTopHeader|sTblIndex::fLeftHeader|sTblIndex::fColsep00;
        if( !tbl->parse() ) {
           ::printf("Failed to parse the table");
           return 1;
        }

        sVec <idx> colSetTree; colSetTree.add(tbl->cols());
        for (idx i=0; i < tbl->cols();++i) { colSetTree[i]=i;}

        sVec <idx> rowSetTree; rowSetTree.add(tbl->rows());
        for (idx i=0; i < tbl->rows();++i) { rowSetTree[i]=i;}

        sTree::DistanceMethods method = sTree::EUCLIDEAN;
        sTree::neighborJoiningMethods njM = sTree::FAST;


        sStr hTreePath; reqAddFile(hTreePath,"%s_horizontal.tre",baseName);
        sFil horizontalTree(hTreePath);

        sVec <idx> actualRowOrder; actualRowOrder.add(tbl->rows());

        sTree::generateTree(horizontalTree, &colSetTree, &rowSetTree,tbl,&actualRowOrder,1,0,method,njM);

        sStr vTreePath; reqAddFile(vTreePath,"%s_vertical.tre",baseName);
        sFil verticalTree(vTreePath);

        sVec <idx> actualColOrder; actualColOrder.add(tbl->cols());
        sTree::generateTree(verticalTree, &colSetTree, &rowSetTree,tbl,&actualColOrder,0,0,method,njM);

        sVec < sVec< real > > vals;

        sHeatmap::generateRangeMap( &vals, tbl, &actualColOrder,  &actualRowOrder, 0);


        sStr covHeatMap_toDraw; reqAddFile(covHeatMap_toDraw,"%s_heatMap.csv",baseName);
        sFile::remove(covHeatMap_toDraw);
        sFil heatmapCSV(covHeatMap_toDraw);

        heatmapCSV.printf("%s","rows");
        for( idx ic=0; ic<actualColOrder.dim(); ++ic) {
           heatmapCSV.printf(",");
           tbl->printTopHeader(heatmapCSV,actualColOrder[ic]);
        }
        heatmapCSV.printf("\n");

        for( idx ir=0; ir<actualRowOrder.dim(); ++ir) {
            tbl->printCell(heatmapCSV,ir,-1);
           for( idx ic=0; ic<actualColOrder.dim(); ++ic)
           {
               if (vals[ir][ic]==-1)
                   heatmapCSV.printf(",");
               else
                   heatmapCSV.printf(",%g",vals[ir][ic]);
           }
           heatmapCSV.printf("\n");
        }

        return 0;
}

idx * tryAlternativeWay (sIonWander * myWander, const char * orignal_id, idx & recDim, idx resSet=6) {

    const char * nxt;
    nxt = orignal_id+sLen(orignal_id);
    idx sizeSeqId=nxt-orignal_id;
    for( const char * p=orignal_id; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
               nxt=strpbrk(p,"| ");
               if(!nxt || *nxt==' ')
                   break;

               const char * curId=nxt+1;
               nxt=strpbrk(nxt+1," |");
               if(!nxt)
                   nxt=orignal_id+sizeSeqId;
               if(*nxt==' ')
                   break;

               myWander->setSearchTemplateVariable("$seqID1",7,curId, nxt-curId);
               myWander->setSearchTemplateVariable("$seqID2",7,curId, nxt-curId);
               myWander->resetResultBuf();
               myWander->traverse();

               if (myWander->traverseBuf.length()){
                   recDim=myWander->traverseBuf.length()/sizeof(idx)/resSet;
                   return (idx * )myWander->traverseBuf.ptr(0);
               }
               const char * dot = strpbrk(curId,".");
               if (dot){
                   myWander->setSearchTemplateVariable("$seqID1",7, curId, dot-curId);
                   myWander->setSearchTemplateVariable("$seqID2",7, curId, dot-curId);

                   myWander->resetResultBuf();
                   myWander->traverse();
                   if (myWander->traverseBuf.length()){
                       return (idx * )myWander->traverseBuf.ptr(0);
                  }
              }
           }

    return 0 ;
}

idx DnaTNSeq::OnExecute(idx req)
{

    sStr cbuf;
    bool doBreak = false;

    sVec < idx > allHistograms(sMex::fSetZero) ;allHistograms.add(1);


    sVec<sHiveId> alIds;
    sHiveId::parseRangeSet(alIds, objs[0].propGet00("aligners",0,","));
    if(!alIds.dim()) {
        reqSetInfo(req, eQPInfoLevel_Error, "No alignment information available\n" );
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    idx window=formIValue("window",0);
    idx coverage=formIValue("coverage_threshold",1);
    idx cntFound = 0, totWork = alIds.dim(), curWork = 0;

    const char * refAnnot=formValue("referenceAnnot");
    sHiveIon hionAnnot(user,refAnnot,0,"ion");

    const char * refType = formValue("referenceType",0);
    sStr refFeature; formValue("referenceFeature",&refFeature,0);




    sDic < sDic < PosDef > > subList;

    for(idx iAli=0; iAli<alIds.dim(); ++iAli) {

        sUsrObj alo(*user, alIds[iAli]);if(!alo.Id())continue;
        QPSvcDnaHexagon::getSubject00(alo,cbuf);
        sHiveseq sub(user,cbuf.ptr()); cbuf.cut(0);

        QPSvcDnaHexagon::getQuery00(alo,cbuf);
        sHiveseq qry(user,cbuf.ptr()); cbuf.cut(0);

        alIds[iAli].print(cbuf);
        sHiveal als(user,cbuf.ptr());cbuf.cut(0);


        for ( idx isub=0, cntAl, cntSub=sub.dim(); isub<cntSub; ++isub ){

            als.listSubAlIndex(isub, &cntAl);
            for ( idx ia=0; ia<cntAl; ++ia ){
                sBioseqAlignment::Al * hdr=als.getAl(ia);
                idx * m=als.getMatch(ia), len=hdr->lenAlign(), refPos, dir;

                sDic < PosDef > * pSubject=subList.set(sub.id(hdr->idSub()));
                pSubject->flagOn(sMex::fSetZero);

                if((hdr->flags()&(sBioseqAlignment::fAlignBackward))) {
                    dir=-1;
                    refPos=hdr->getSubjectEnd(m);
                } else {
                    refPos = hdr->getSubjectStart(m)+1;
                    dir=1;
                }

                if(hdr->idSub()==1)
                    ::printf("DEBUG\n");

                refPos|=(iAli<<32);

                if (dir==-1) {
                    refPos|=(sIdxHighBit);
                }
                PosDef * p=pSubject->set(&refPos,sizeof(refPos));
                p->cnt+=1;
                p->cntRpt+=als.getRpt(ia);
                p->len+=len;
                p->dir=dir;
                p->alid=alIds[iAli].objId();

                if( (curWork % 10000) == 0 ) {
                    if( !reqProgress(cntFound, curWork, totWork) ) {
                        doBreak = true;
                        break;
                    }
                }

                ++cntFound;
                ++curWork;
            }
            if(doBreak)
                break;
        }
        if(doBreak)
            break;
    }


    logOut(eQPLogType_Debug, "Analyzing results\n");

    sDic < sDic < GeneSpecInfo > > gsi;

    sDic < idx > geneList;

    sStr cov_heatMap_path;
    sFil cov_heatMap_out(reqAddFile(cov_heatMap_path, "cov_heatMapData.csv"));
    cov_heatMap_out.printf("AlignerID");

    sStr ins_heatMap_path;
    sFil ins_heatMap_out(reqAddFile(ins_heatMap_path, "ins_heatMapData.csv"));
    ins_heatMap_out.printf("AlignerID");

    if (alIds.dim() < 2){
        sFile::remove(cov_heatMap_path);
        sFile::remove(ins_heatMap_path);
    }

    {

        sStr genebuf;
        sIonWander * wander=0;
        if(refAnnot) {
            if(refType){
                if (refFeature && *refFeature.ptr(0)) {
                    wander=hionAnnot.addIonWander("myion","r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);\
                                                                        b=find.annot(seqID=r.seqID,record=r.record,type=FEATURES);\
                                                                        c=find.annot(seqID=b.seqID,record=b.record,id=\"%s\");\
                                                                        d=find.annot(seqID=c.seqID,record=c.record,type=\"%s\");unique.1(d.pos);\
                                                                        check_off;itraverse(\"this\",\"this\");\
                                                                        e=find.annot(seqID=d.seqID,record=d.record,type=strand);\
                                                                        blob(d.pos,d.type,d.id,e.id);",refFeature.ptr(),refType);
                }
                else {
                    wander=hionAnnot.addIonWander("myion","r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);\
                                                           d=find.annot(seqID=r.seqID,record=r.record,type=\"%s\");unique.1(d.pos);\
                                                           check_off;itraverse(\"this\",\"this\");\
                                                           e=find.annot(seqID=d.seqID,record=d.record,type=strand);\
                                                           blob(d.pos,d.type,d.id,e.id);",refType);
                }
            }
            else{
                wander=hionAnnot.addIonWander("myion","r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);unique.1(r.pos);check_off;itraverse(\"this\",\"this\");e=find.annot(seqID=r.seqID,record=r.record,type=strand);blob(r.pos,r.type,r.id,e.id);");
            }
        }
        char szStart[128],szEnd[128];szEnd[0]='0';szEnd[1]=':';
        idx lenStart,lenEnd;


        sStr path;
        sFil out(destPath(&path, "tnseq.csv"),sMex::fMapRemoveFile);
        out.printf("AlignerID,Reference,Start-Position,Direction,Coverage,Length");
        #ifdef _DEBUG
            ::printf("AlignerID,ref,pos,dir,coverage,len");
        #endif

        if(wander) {
            if (refType) {
                out.printf(",%s-pos,%s,range,strand",refType,refType);
            }
            else out.printf(",type,range,strand");
            #ifdef _DEBUG
                ::printf(",type,id,range");
            #endif
        }

        out.printf("\n");
        #ifdef _DEBUG
            ::printf("\n");
        #endif


        const char * listOfTypesToSkip[] ={"FEATURES","accession","gi","organism"};

        for(idx iSub=0, lenId; iSub<subList.dim(); ++iSub) {
            sDic < PosDef > * pSubject=subList.ptr(iSub);
            const char * id=(const char *)subList.id(iSub);
            for(lenId=0;(!strchr(sString_symbolsSpace,id[lenId]));++lenId);

            if(wander) {
                wander->setSearchTemplateVariable("$seqID1",7,id,lenId);
                wander->setSearchTemplateVariable("$seqID2",7,id,lenId);
            }

            for( idx ii=0; ii<pSubject->dim() ; ++ii ){
                PosDef * pd=pSubject->ptr(ii);
                idx * pRefPos=(idx*)pSubject->id(ii);
                idx refPos=((*pRefPos)&0xFFFFFFFFll);
                if(window && pd->len<window*pd->cnt)
                    continue;
                if(coverage && pd->cntRpt<coverage)
                    continue;


                out.printf("%" DEC ",\"%.*s\",%" DEC ",%" DEC ",%" DEC ",%" DEC "",pd->alid,(int)lenId,id,refPos,pd->dir,pd->cntRpt,pd->len/pd->cnt);
                #ifdef _DEBUG
                #endif


                if(wander) {
                    sDic < idx > printedGene;
                    idx start=refPos;sIPrintf(szStart,lenStart,start,10);memcpy(szStart+lenStart,":0",3);lenStart+=2;
                    idx end=refPos;sIPrintf(szEnd+2,lenEnd,end,10);lenEnd+=2;
                    wander->setSearchTemplateVariable("$start",6,szStart,lenStart);
                    wander->setSearchTemplateVariable("$end",4,szEnd,lenEnd);
                    wander->resetResultBuf();
                    wander->traverse();
                    idx resSet=6;
                    idx recDim=wander->traverseBuf.length()/sizeof(idx)/resSet;
                    idx *p=recDim ? (idx * )wander->traverseBuf.ptr(0) : 0 ;

                    if (!p) {
                        p = tryAlternativeWay(wander,id, recDim,resSet);
                    }

                    for( idx iRec=0; iRec<recDim; ++iRec ){
                        idx pos=*sConvInt2Ptr(p[1+iRec*resSet],idx);
                        idx lenType=p[2+iRec*resSet];
                        const char * type=sConvInt2Ptr(p[3+iRec*resSet],const char );
                        idx lenId=p[4+iRec*resSet];
                        const char * id=sConvInt2Ptr(p[5+iRec*resSet],const char );

                        bool isContinue = false;
                        for ( idx it=0; it<sDim(listOfTypesToSkip) ; ++it ) {
                            if (memcmp(type,listOfTypesToSkip[it],lenType)==0){
                                isContinue = true;
                                break;
                            }
                        }
                        if (isContinue){
                            continue;
                        }
                        if (start < ((pos)>>32) || start > ((pos)&0xFFFFFFFF)) {
                        #ifdef _DEBUG
                        #endif
                            continue;
                        }
                        if ( ( (pos) & 0xFFFFFFFF) == 0xFFFFFFFF){
                            continue;
                        }

                        idx posWithRelationToGene=refPos-((pos)>>32)+1;

                        idx lenStrand=p[6+iRec*resSet];
                        const char * strand=sConvInt2Ptr(p[7+iRec*resSet],const char );
                        if (strand && *strand && strncmp(strand,"-",lenStrand)==0) {
                            posWithRelationToGene=((pos)&0xFFFFFFFF) - refPos +1;
                        }


                        genebuf.printf(0,"\"%.*s\"",(int)lenId,id);
                        genebuf.printf(",\"%" DEC ":%" DEC "\"",(pos)>>32,((pos)&0xFFFFFFFF));
                        genebuf.add0(2);

                        if ( alIds.dim() > 1 && !geneList.find(genebuf.ptr(),genebuf.length())) {
                            *geneList.set(genebuf.ptr(),genebuf.length())=1;
                            cov_heatMap_out.printf(",\"%.*s %" DEC ":%" DEC "\"",(int)lenId,id,(pos)>>32,((pos)&0xFFFFFFFF));
                            ins_heatMap_out.printf(",\"%.*s %" DEC ":%" DEC "\"",(int)lenId,id,(pos)>>32,((pos)&0xFFFFFFFF));
                        }

                        if (printedGene.find(genebuf.ptr(0),genebuf.length())) {
                            continue;
                        }
                        *printedGene.set(genebuf.ptr(0),genebuf.length())=1;
                        out.printf(",%" DEC"",posWithRelationToGene);
                        out.printf(",%s",genebuf.ptr());
                        out.printf(",%.*s",(int)lenStrand,strand);

                        #ifdef _DEBUG
                        #endif
                        sDic < GeneSpecInfo > * pGSI=gsi.set(&(sMex::_zero),sizeof(sMex::_zero));pGSI->flagOn(sMex::fSetZero);
                            pGSI->set(genebuf.ptr(),genebuf.length());
                        pGSI=gsi.set(&pd->alid,sizeof(pd->alid));pGSI->flagOn(sMex::fSetZero);
                        GeneSpecInfo * g=pGSI->set(genebuf.ptr(),genebuf.length());
                        g->inserts++;
                        g->coverage += pd->cntRpt;
                        if (pd->dir >0) g->forward++;
                        else g->reverse++;

                        if(!g->histogOffset) {
                            g->histogOffset=allHistograms.dim();
                            g->histogLen= ((pos)&0xFFFFFFFF) - ((pos)>>32)+1;
                            g->histogOffsetStart=((pos)>>32);
                            allHistograms.add(g->histogLen);
                        }
                        idx * histog = allHistograms.ptr(g->histogOffset);
                        *(histog+(start-g->histogOffsetStart)) += pd->cntRpt;

                    }

                    wander->resetResultBuf();
                }

                out.printf("\n");
                #ifdef _DEBUG
                #endif

            }
        }
        out.destroy();
    }

    if ( alIds.dim() > 1 ) {
        cov_heatMap_out.printf("\n");
        ins_heatMap_out.printf("\n");
    }

    logOut(eQPLogType_Debug, "Generating Gene Specific Information \n");

        sStr gsi_path;
        sFil gsi_out(reqAddFile(gsi_path, "geneSpecInfo.csv"));
        gsi_out.printf("AlignerID,%s,Range,Coverage,Insert,Forward,Reverse,Entropy,Center of Balance\n",refType);


        for ( idx iA=1; iA < gsi.dim() ; ++iA )
        {
            sDic < GeneSpecInfo > * pGSI = gsi.ptr(iA);
            for (idx iGene=0; iGene < pGSI->dim(); ++iGene) {
                GeneSpecInfo * g = pGSI->ptr(iGene);

                idx * histog=allHistograms.ptr(g->histogOffset);
                real sumRelPosToHalf=0; idx totPos=0;
                real sumPlogP=0;
                for( idx l=0; l< g->histogLen; ++l) {
                    if( *(histog+l)==0 ) continue;
                    real p= *(histog+l)/((real)g->coverage);
                    sumPlogP+=p*log(p);
                    sumRelPosToHalf+=((l+1)-(g->histogLen/2));
                    totPos+=1;
                }
                real shannon = -sumPlogP/log(g->histogLen);
                real momentum = sumRelPosToHalf/(totPos*(g->histogLen/2));
                if (shannon <= 0) {
                    shannon = 0 ;
                }
                gsi_out.printf("%" DEC ",%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",%.3lf,%.3lf\n",*((idx *)gsi.id(iA)),(const char *)pGSI->id(iGene),g->coverage,g->inserts,g->forward,g->reverse,shannon,momentum);
            }
            if (alIds.dim()>1) {
                idx len=0;
                cov_heatMap_out.printf("%" DEC "",*((idx *)gsi.id(iA)));
                ins_heatMap_out.printf("%" DEC "",*((idx *)gsi.id(iA)));
                for (idx iG=0; iG < geneList.dim(); ++iG) {
                    const char * gname = (const char *)geneList.id(iG,&len);
                    GeneSpecInfo * g = pGSI->get(gname,len);
                    if (!g) {
                        cov_heatMap_out.printf(",0");
                        ins_heatMap_out.printf(",0");
                        continue;
                    }
                    cov_heatMap_out.printf(",%" DEC "",g->coverage);
                    ins_heatMap_out.printf(",%" DEC "",g->inserts);
                }
                cov_heatMap_out.printf("\n");
                ins_heatMap_out.printf("\n");
            }
        }

        if (alIds.dim()>1) {
            cov_heatMap_out.destroy();
            ins_heatMap_out.destroy();
        }



        {
            logOut(eQPLogType_Debug, "Generating Coverage Heatmap tables \n");
            if (sFile::exists(cov_heatMap_path)){
                generateHeatMap(cov_heatMap_path, "cov");
            }
            logOut(eQPLogType_Debug, "Generating Insert Heatmap tables \n");
            if (sFile::exists(ins_heatMap_path)){
                generateHeatMap(ins_heatMap_path, "ins");
            }
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

    DnaTNSeq backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-tnseq", argv[0]));
    return (int) backend.run(argc, argv);
}



