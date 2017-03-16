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

#include <slib/utils.hpp>
#include <slib/core.hpp>
#include <ssci/bio/vax-bio.hpp>
#include <ssci/bio/bioseq.hpp>




class DirectVirMut: public sQPrideProc
{
    public:
        DirectVirMut(const char * defline00,const char * srv) : sQPrideProc(defline00,srv)
        {
        }
        virtual idx OnExecute(idx);
        idx accumulateTable();

};

#define findCOL(_v_col) { \
    if(i ## _v_col ==-2 )  \
        vax.hdrTable.find(#_v_col,&i ## _v_col); \
    if( i ## _v_col>=0 ) { \
        body ## _v_col =(const char * ) vax.hdrTable[i ## _v_col].ofs; \
        size ## _v_col =vax.hdrTable[i ## _v_col].size ; \
    } \
}

#define findCOL_S(_v_b,_v_col) findCOL(_v_col) \
    if(body ## _v_col) sString::changeCase(&_v_b,body ## _v_col,size ## _v_col , sString::eCaseHi );_v_b.add0();

#define findCOL_I(_v_col)  findCOL(_v_col) \
    if(body ## _v_col) sIScanf(val ## _v_col, body ## _v_col , size ## _v_col, 10 );

#define findCOL_R(_v_col)  findCOL(_v_col) \
    if(body ## _v_col) sRScanf(val ## _v_col, body ## _v_col , size ## _v_col, 10 );


struct MEASURE{
        //idx row;
        idx AAPOS,TCOV,VCOV;
        real AAFREQ;
        char AAREF, AASUB;
        char visit[32],ngspl[32];
        bool confirmed;
};


idx DirectVirMut::OnExecute(idx req)
{

    real frequencyThreshold=formRValue("frequencyThreshold",0.1);

    idx rowCnt=0;
    sDic <sDic <sDic < sVec <MEASURE> > > > originalMapping; // by patient, by position, by company -> substitution types

    sStr bufPath,failedPath00;
    const char * dstPath = reqAddFile(bufPath,"input.csv"); bufPath.add0();
    if(!dstPath) {
        logOut(eQPLogType_Error, "Failed to add input.csv");
        reqSetInfo(req, eQPInfoLevel_Error, "Failed to add input.csv" );
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sFil finp(dstPath);
    finp.printf("USUBJID,NGSPL,VISIT,AAPOS,TCOV,AAREF,AASUB,VCOV,AAFREQ,VARDECT\n");
    #ifdef _DEBUG
        ::printf("USUBJID,NGSPL,VISIT,AAPOS,TCOV,AAREF,AASUB,VCOV,AAFREQ,VARDECT\n");
    #endif


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/  Reading objects and tables into one big container
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sVec<sHiveId> altIDs;
    sHiveId::parseRangeSet(altIDs, formValue("formatAlternatives"));
    sHiveId::parseRangeSet(altIDs, formValue("formatHIVE"));
    bool skipSynonymous = formBoolIValue("skipSynonymous", true);
    bool collapseIdenticalAAMutations = formBoolIValue("collapseIdenticalAAMutations", false);
    sDir fileList00;
    sStr defBuf,nameBuf;
    idx enrichment=formIValue("enrichment",1);

    for(idx ia=0; ia<altIDs.dim(); ia++) {
        sUsrObj obj( *user, altIDs[ia]);

        bool isHIVE=false;
        const char * wildcard= "*.csv", *defVarDect = "gil", * defVisit= "baseline",* defSubjID="subject",* defNgsPl="na";

        if( strcmp( obj.getTypeName(), "svc-profiler-heptagon")==0 ) {
            wildcard= "AAprofile.csv";
            defVarDect="hive";
            isHIVE=true;

            sHiveId aID(obj.propGet00("parent_proc_ids", &defBuf ));sUsrObj aObj( *user, aID);if(!aObj.Id())continue;defBuf.cut(0);
            sHiveId qID(aObj.propGet00("query", &defBuf ));sUsrObj qObj( *user, qID);if(!qObj.Id())continue;defBuf.cut(0);
            if(!qObj.propGet00("name", &defBuf, ";"))continue;
            defSubjID=defBuf.ptr(0);
            char * s=strchr(defBuf.ptr(),'.');if(s){*s=0;defVisit=s+1;s=strchr(s+1,'.');if(s)*s=0;}

        }

        fileList00.cut();obj.files(fileList00, sFlag(sDir::bitFiles),wildcard,"");
        if(!fileList00.dimEntries()){
            reqSetInfo(req, eQPInfoLevel_Warning, "Failed to find amino acid mutations in computation %s.", altIDs[ia].print() );
            logOut(eQPLogType_Warning, "Failed to find AAprofile.csv of %s.", altIDs[ia].print() );
        }
        sStr b1, b2;

        for( const char * fl=fileList00.ptr(); fl; fl=sString::next00(fl)) {
            #ifdef _DEBUG
                ::printf("*** start FILE=%s\n",fl);
            #endif

            nameBuf.cut(0);

            sVax vax(sVax::fUseFStream, fl );
            //sVax vax(sVax::fUseMMap, fl );

            idx iAAPOS=-2,iTCOV=-2,iVCOV=-2,iAAFREQ=-2,iAAREF=-2,iAASUB=-2,iUSUBJID=-2,iNGSPL=-2,iVISIT=-2,iVARDECT=-2;
            const char * bodyAAPOS=0,*bodyTCOV=0,*bodyAAREF=0,*bodyAASUB=0,*bodyVCOV=0,*bodyAAFREQ=0,*bodyUSUBJID=0,*bodyNGSPL=0,*bodyVISIT=0,*bodyVARDECT=0;
            idx valAAPOS=0,valTCOV=0,valVCOV=0;
            real valAAFREQ=0;
            idx sizeAAPOS=0,sizeTCOV=0,sizeAAREF=0,sizeAASUB=0,sizeVCOV=0,sizeAAFREQ=0,sizeUSUBJID=0,sizeNGSPL=0,sizeVISIT=0,sizeVARDECT=0;
            //idx valAAPOS=0,valTCOV=0,valVCOV=0;


            idx len=0, buflen = 0; const char * id=0;
            idx isCLC=-1;
            MEASURE * m = 0, * prev_m = 0;
            while ( vax.ensureRecordBuf() ) {

                b1.cut(0); b2.cut(0);
                if(!isHIVE && isCLC==-1){
                    id=(const char * )vax.hdrTable.id(0,&len);

                    if(len!=7 && strncasecmp(id,"USUBJID",len) ) {
                        isCLC=true;
                        defBuf.cut(0);defBuf.addString(sFilePath::nextToSlash(fl));
                        defSubjID=defBuf.ptr(0);
                        char * s=strchr(defBuf.ptr(),'.');if(s){*s=0;defVisit=s+1;s=strchr(s+1,'.');if(s)*s=0;}
                        obj.propGet("name", &nameBuf);
                        if(nameBuf.length()) {
                            defVarDect=nameBuf.ptr(0);
                            char * p=strchr(nameBuf.ptr(0),'.');
                            if(p)*p=0;
                        }
                        else defVarDect="clc";

                    }
                    else
                        isCLC=0;
                }

                if(isCLC==1) {
                    //findCOL_S(b1,USUBJID);//if(!bodyUSUBJID) {bodyUSUBJID = defSubjID;sizeUSUBJID= sLen(defSubjID);}
                    idx iAAChange=-2,iCount=-2,iCoverage=-2,iFrequency=-2;
                    real valFrequency=0;
                    idx valCount=0,valCoverage=0,sizeAAChange=0,sizeCount=0,sizeCoverage=0,sizeFrequency=0;
                    const char * bodyAAChange=0, * bodyCount=0, *bodyCoverage=0, * bodyFrequency=0;
                    findCOL_I(Count);
                    findCOL_I(Coverage);
                    findCOL_R(Frequency);
                    valAAFREQ=valFrequency/100.;
                    valTCOV=valCoverage;
                    valVCOV=valCount;
                    if(iAAChange==-2 ){
                        vax.hdrTable.find("Amino acid change",&iAAChange);
                    }
                    sizeAASUB=0;
                    if( iAAChange>=0 ) {
                        bodyAAChange=(const char * ) vax.hdrTable[iAAChange].ofs;
                        sizeAAChange=vax.hdrTable[iAAChange].size ;
                        if(sizeAAChange>3) {
                            const char * prot=sString::searchSubstring(bodyAAChange,sizeAAChange,":p." __,1,0,true);
                            if(prot) {
                                prot+=3;
                                sBioseq::ProtAA *  p1=sBioseq::AAFindByLet3(prot);
                                idx ilen;sIScanf_Mv(valAAPOS,prot+3,(sizeAAChange-6),10,ilen);
                                sBioseq::ProtAA *  p2=sBioseq::AAFindByLet3(prot+3+ilen);
                                if(p1)bodyAAREF=p1->let;
                                else bodyAAREF=prot;
                                if(p2) bodyAASUB=p2->let;
                                else bodyAASUB=prot+3+ilen;
                                sizeAAREF=1;
                                sizeAASUB=1;
                            }
                        }
                    }

                    if(!sizeAASUB)
                        continue;
                } else {
                    findCOL_S(b1,USUBJID);//if(!bodyUSUBJID) {bodyUSUBJID = defSubjID;sizeUSUBJID= sLen(defSubjID);}
                    findCOL(NGSPL);
                    findCOL(VISIT);
                    findCOL_I(AAPOS);
                    findCOL_I(TCOV);
                    findCOL(AAREF);
                    findCOL(AASUB);
                    findCOL_I(VCOV);
                    findCOL_R(AAFREQ);
                    findCOL_S(b2,VARDECT);
                }
                if(skipSynonymous && toupper(*bodyAAREF)==toupper(*bodyAASUB) ) continue;
                if(!bodyNGSPL) {bodyNGSPL = defNgsPl;sizeNGSPL = sLen(defNgsPl);}
                if(!bodyVISIT) {bodyVISIT = defVisit;sizeVISIT = sLen(defVisit);}

                const char * pat=bodyUSUBJID ? b1.ptr(0) : defSubjID ;
                const char * comp=bodyVARDECT ?  b2.ptr() : defVarDect ;

                if(collapseIdenticalAAMutations && prev_m && prev_m->AAPOS==valAAPOS && toupper(prev_m->AASUB)==toupper(*bodyAASUB) && (sLen(prev_m->visit)==sizeVISIT && strncmp(prev_m->visit,bodyVISIT,sizeVISIT)==0) ) {
                    prev_m->VCOV+=valVCOV;
                    prev_m->AAFREQ+=valAAFREQ;
                    finp.cut(buflen);
                } else {
                    m=originalMapping[pat][&valAAPOS][comp].add(1);
                    m->confirmed=false;
                    m->AAPOS=valAAPOS;
                    m->TCOV=valTCOV;
                    m->VCOV=valVCOV;
                    m->AAFREQ=valAAFREQ;
                    m->AAREF=*bodyAAREF;
                    m->AASUB=*bodyAASUB;
                    if(sizeNGSPL>=(idx)(sizeof(m->ngspl)-1))sizeNGSPL=(idx)(sizeof(m->ngspl)-1);
                    strncpy(m->ngspl,bodyNGSPL , sizeNGSPL); m->ngspl[sizeNGSPL]=0;
                    if(sizeVISIT>=(idx)(sizeof(m->visit)-1))sizeVISIT=(idx)(sizeof(m->visit)-1);
                    strncpy(m->visit,bodyVISIT , sizeVISIT); m->visit[sizeVISIT]=0;
                }
                buflen = finp.length();
                prev_m = m;

                ++rowCnt;

                finp.printf("%s,%s,%s,%" DEC ",%" DEC ",%c,%c,%" DEC ",%.3lf,%s\n",pat,m->ngspl,m->visit,m->AAPOS,m->TCOV,m->AAREF,m->AASUB,m->VCOV,m->AAFREQ,comp);
                #ifdef _DEBUG
                ::printf("%s,%s,%s,%" DEC ",%" DEC ",%c,%c,%" DEC ",%.3lf,%s\n",pat,m->ngspl,m->visit,m->AAPOS,m->TCOV,m->AAREF,m->AASUB,m->VCOV,m->AAFREQ,comp);
                #endif

            }
            #ifdef _DEBUG
                ::printf("*** end FILE=%s\n",fl);
            #endif

        }

    }
    finp.destroy();

    if(!originalMapping.dim()){
        reqSetInfo(req, eQPInfoLevel_Warning, "No valid input" );
        logOut(eQPLogType_Warning, "No valid input" );
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/  filtration and baseline collapse stage
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sDic <idx > compList;
    idx len;
    sDic < sDic < sDic < sDic < idx > > > > finalMap ; // by position, by substitution , by patient , by the company type the counts
    sDic < idx > finTotals;
    sStr sbaseline_keywords00;
    const char * baseline_keywords00 = formValues00("baseline_keywords", &sbaseline_keywords00);

    if( !baseline_keywords00 )
        baseline_keywords00 = "baseline" _ "screening" __;
    else {
        sString::searchAndReplaceSymbols(sbaseline_keywords00.ptr(0),0,",;",0,0,true,false,false,true,0);
    }

    dstPath = reqAddFile(bufPath,"f_input.csv"); bufPath.add0();
    sFil it_inp(dstPath);
    it_inp.printf(0,"USUBJID,NGSPL,VISIT,AAPOS,TCOV,AAREF,AASUB,VCOV,AAFREQ,VARDECT\n");


    //  iterating over samples
    for(idx p = 0; p < originalMapping.dim(); p++) {
        const char * pat = (const char *)(originalMapping.id(p));
        sDic <sDic <sVec <MEASURE> > > * oriPat=originalMapping.ptr(p);

        //iterating over positions
        for(idx po = 0; po < oriPat->dim(); po++) {
            idx * pos = (idx*)oriPat->id(po);
            sDic <sVec <MEASURE> > * oriPo=oriPat->ptr(po);

            //iterating over companies
            for(idx c = 0; c < oriPo->dim(); c++) {
                const char * comp = (const char *)(oriPo->id(c,&len));
                sVec <MEASURE> * oriCo=oriPo->ptr(c);

                MEASURE * m=0;

                real baselineFreq = 0;
                for(idx i = 0; i < oriCo->dim(); i++) {
                    m =oriCo->ptr(i);
                    if( sString::compareChoice(m->visit, baseline_keywords00, 0, true, 0, true) != sNotIdx ){
                        baselineFreq += m->AAFREQ;
                    }
                }

                idx howManyNonBaseline=0, howmanyprinted=0;
                for(idx i = 0; i < oriCo->dim(); i++) {
                    m =oriCo->ptr(i);
                    if( sString::compareChoice(m->visit, baseline_keywords00, 0, true, 0, true) != sNotIdx )
                        continue;

                    ++howManyNonBaseline;

                    if(baselineFreq*100.>enrichment)
                        continue;

                    real tempDelta = sAbs(m->AAFREQ - baselineFreq);
                    if( tempDelta < frequencyThreshold )
                        continue;

                    m->AAFREQ=m->AAFREQ - baselineFreq;

                    char sub[4];sub[0]=m->AAREF;sub[1]=m->AASUB;sub[2]=0;sub[3]=0;
                    ++finalMap[pos][sub][pat][comp];
                    ++finTotals[pos];

                    *compList.set(comp,len)=1;
                    it_inp.printf("%s,%s,%s,%" DEC ",%" DEC ",%c,%c,%" DEC ",%.3lf,%s\n",pat,m->ngspl,m->visit,m->AAPOS,m->TCOV,m->AAREF,m->AASUB,m->VCOV,m->AAFREQ,comp);
                    howmanyprinted++;
                }

                //if(howManyNonBaseline==0 && m && m->AAFREQ >= frequencyThreshold ) {
                if(howManyNonBaseline==0 && m && baselineFreq >= frequencyThreshold ) {
                    char sub[4];sub[0]=m->AAREF;sub[1]=m->AASUB;sub[2]=0;sub[3]=0;
                    ++finalMap[pos][sub][pat][comp];
                    ++finTotals[pos];
                    *compList.set(comp,len)=1;
                    howmanyprinted++;

                }

                if( howmanyprinted ) {
                    for(idx i = 0; i < oriCo->dim(); i++) {
                        m = oriCo->ptr(i);
                        if( sString::compareChoice(m->visit, baseline_keywords00, 0, true, 0, true) != sNotIdx ) {
                            it_inp.printf("%s,%s,%s,%" DEC ",%" DEC ",%c,%c,%" DEC ",%.3lf,%s\n", pat, m->ngspl, m->visit, m->AAPOS, m->TCOV, m->AAREF, m->AASUB, m->VCOV, m->AAFREQ, comp);
                        }
                    }
                }

                m->confirmed=true;

            }


        }
    }



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/  composition stage
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    dstPath = reqAddFile(bufPath,"filtered.csv");
    bufPath.add0();

    if( dstPath ) {
        sFil fflt(dstPath );

        fflt.printf("AAPOS,SUBS10+,SUBJID");
        for(idx i=0; i<compList.dim() ; ++i ) {
            const char * comp=(const char *) compList.id(i,&len);
            fflt.printf(",%.*s",(int)len,comp);
        }
        fflt.printf("\n");

        //  iterating over positions
        for(idx po = 0; po < finalMap.dim(); po++) {
            idx * pos = (idx *)(finalMap.id(po));
            sDic <sDic < sDic <idx> > > * finPos=finalMap.ptr(po);

            //iterating over substitutions
            for(idx su = 0; su< finPos->dim(); su++) {
                const char * sub= (const char *)finPos->id(su);
                sDic < sDic <idx> > * finSub=finPos->ptr(su);

                //iterating over patients
                for(idx p = 0; p < finSub->dim(); p++) {
                    const char * pat = (const char *)(finSub->id(p));
                    sDic <idx> * finPat=finSub->ptr(p);

                    fflt.printf("%" DEC ",%c%" DEC "%c,%s",*pos,sub[0],*pos,sub[1],pat);

                    for(idx i=0; i<compList.dim() ; ++i ) { // for(idx c = 0; c < finPat->dim(); c++) {
                        const void * comp=compList.id(i,&len);
                        idx * res =finPat->get( comp,len);
                        fflt.printf(",%" DEC,res ? *res : 0 );
                    }
                    fflt.printf("\n");
                }
            }
        }
        #ifdef _DEBUG
            ::printf("%.*s",(int)(fflt.length()),fflt.ptr());
        #endif
        fflt.destroy();
    } else {
        failedPath00.printf("filtered.csv");
        failedPath00.add0();
    }



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/  final stage
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    dstPath = reqAddFile(bufPath,"results.csv");
    bufPath.add0();

    if( dstPath ) {
        sFil fres(dstPath);
        fres.printf("Position,Number OF Subjects,SUB10+-,Ratio\n");

        sDic < idx > indTotals;
        idx * fin=indTotals.add(finTotals.dim()*2), * ind=fin+finTotals.dim();
        for(idx po = 0; po < finalMap.dim(); po++) fin[po]=-finTotals[po];
        sSort::sort(finTotals.dim(),fin,ind);
        for(idx po = 0; po < finalMap.dim(); po++) {
            idx ipo=ind[po];
            idx * pos=(idx *)finalMap.id(ipo);
            fres.printf("%" DEC ",%" DEC ",",*pos,finTotals[ipo]);

            sDic < sDic < sDic < idx > > > * finPos=finalMap.ptr(ipo); // by position, by substitution , by patient , by the company type the counts
            for( idx su=0; su<finPos->dim(); ++su) {

                const char * sub= (const char *)finPos->id(su);
                //sDic < sDic <idx> > * finSub=finPos->ptr(su);
                fres.printf("%s%c%" DEC "%c",su ? "/":"" ,sub[0],*pos,sub[1]);
            }

            fres.printf(",");

            for( idx su=0; su<finPos->dim(); ++su) {
                sDic < sDic <idx> > * finSub=finPos->ptr(su);

                idx tot=0;
                for(idx p = 0; p < finSub->dim(); p++) {
                    sDic <idx> * finPat=finSub->ptr(p);
                    //tot+=finPat->dim();
                    for(idx c = 0; c < finPat->dim(); c++) {
                        tot+=*finPat->ptr(c);
                    }
                }

                fres.printf("%s%" DEC "",su ? "-":"" ,tot);
            }

            fres.printf("\n");
        }
        #ifdef _DEBUG
            ::printf("%.*s",(int)(fres.length()),fres.ptr());
        #endif

        fres.destroy();
    } else {
        failedPath00.printf("results.csv");
        failedPath00.add0();
    }
    if( failedPath00.ptr() ){
        bufPath.cut(0);
        failedPath00.add0();
        for( const char * fn=failedPath00.ptr(); fn; fn=sString::next00(fn) ) {
            bufPath.printf(",%s",fn);
        }
        reqSetInfo(req, eQPInfoLevel_Error, "Failed to add %s", bufPath.ptr(1) );
        logOut(eQPLogType_Error, "Failed to add %s", bufPath.ptr(1) );
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    reqSetProgress(req,rowCnt,100);
    reqSetStatus(req, eQPReqStatus_Done);

    return 0;
}

int main(int argc, const char * argv[])
{
    //sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    DirectVirMut backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "viral-mutation-comp", argv[0]));
    return (int) backend.run(argc, argv);
}

