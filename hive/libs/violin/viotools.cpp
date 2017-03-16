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
#include <math.h>
#include <slib/std.hpp>
#include <ssci/math.hpp>
#include <ssci/bio.hpp>
#include <slib/utils.hpp>
#include <violin/violin.hpp>

#define limits(_v_nam) idx _v_nam##Min=gFilter.ivalue( #_v_nam "Min",0); idx _v_nam##Max=gFilter.ivalue( #_v_nam "Max",sIdxMax)
#define compare(_v_nam,_v_var) if(_v_nam##Min!=0 || _v_nam##Max!=sIdxMax) {idx _v_nam=(_v_var); if ( _v_nam < _v_nam##Min  || _v_nam > _v_nam##Max )continue;}
#define compareS(_v_nam,_v_var) if(_v_nam && _v_nam[0]!=0){if( strstr( _v_var, _v_nam  )==0 ) continue;}

static idx __on_help(sVioTools * QP, const char * cmd, const char * , const char * ,sVar * pForm);

idx start=0,cnt=sIdxMax, wrap=0,lenMinRand=100,lenMaxRand=100,rangeStartRand=0,rangeEndRand=0;
idx endline=1, negFilter=0,negSelect=0,isNrevCompRand=0,rmfile=0,kfile=0,nId=0,noLim=0,isLengthNormalized=0;
idx isComplement=0, isReverse=0;
static idx verbose=1000;
idx gSilent=0,gBiomode=1;
sVar gFilter;
sVar gPrint;
sVar gInsilico;
sVec < udx > gHitlist;
idx gHitListDim=0;
idx complexityWindow=0; // by default we are not filtering by complexity
real complexityEntropy=1.,mutfreqRand=0.4,noiseRand=0;
sStr gPrimers,qualitySymbol;
sStr taxpathfile;
sVioAnnot annot;

char prefix[32], suffix[32];
char tmppath[200];

#define WSIZE     (sizeof(idx)*8)

bool vParseSingle=false;

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Variables
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
static idx __on_vars(sVioTools * QP, const char * cmd, const char * , const char * ,sVar * pForm)
{
    if(sIs(cmd,"-biomode")){
        gBiomode=pForm->ivalue("mode",0);
        sBioseq::initModule(gBiomode);
    }else if(sIs(cmd,"-noise")){
        noiseRand=pForm->rvalue("value",0);
    }else if(sIs(cmd,"-qualitySymbol")){
        qualitySymbol.printf("%s",pForm->value("value",0));
    }else if(sIs(cmd,"-range")){
        rangeStartRand=pForm->ivalue("start",0);
        rangeEndRand=pForm->ivalue("end",0);
    }else if(sIs(cmd,"-readLen")){
        lenMinRand=pForm->ivalue("min",100);
        lenMaxRand=pForm->ivalue("max",lenMinRand);
    }else if(sIs(cmd,"-mutFreq")){
        mutfreqRand=pForm->rvalue("value",0.4);
    }else if(sIs(cmd,"-NoRevComp")){
        isNrevCompRand=1;
    }else if(sIs(cmd,"-taxPath")){
        taxpathfile.printf(0,"%s", pForm->value("taxpathfile",0));
    }else if(sIs(cmd,"-lengthNormalized")){
        isLengthNormalized=1;
    }else if(sIs(cmd,"-rmfile")){
        rmfile=1;
    }else if(sIs(cmd,"-keepfile")){
        kfile=1;
    }else if(sIs(cmd,"-noID")){
        nId=1;
    }else if(sIs(cmd,"-iscomp")){
        isComplement=1;
    }else if(sIs(cmd,"-isrev")){
        isReverse=1;
    }else if(sIs(cmd,"-user")){
        if (pForm->ivalue("userID",0) == (idx)7)
            noLim=1;
    }else if(sIs(cmd,"-start")){
        start=pForm->ivalue("value",0);
    }else if(sIs(cmd,"-cnt")){
        cnt=pForm->ivalue("value",sIdxMax);
        if(!cnt)cnt=sIdxMax;
    }else if(sIs(cmd,"-verbose")){
        verbose=pForm->ivalue("level",0);
    }else if(sIs(cmd,"-silent")){
        gSilent=pForm->ivalue("level",0);
    } else if(sIs(cmd,"-version")){
        QP->printf("Hive (ver. 0.3)\n");
        QP->printf("%s\n",__TIMESTAMP__);
    } else if(sIs(cmd,"-filter")){
        const char * argv[2]; argv[0]=0;
        argv[1]=pForm->value("conditions");
        if(argv[1])sHtml::inputCGI(0, 2, argv, &gFilter,0,false,"ARGS"); // read the filters
    } else if(sIs(cmd,"-print")){
        const char * argv[2]; argv[0]=0;
        argv[1]=pForm->value("values");
        if(argv[1])sHtml::inputCGI(0, 2, argv, &gPrint,0,false,"ARGS"); // read the print
    } else if (sIs(cmd,"-insilico")){
        const char * argv[2]; argv[0]=0;
        argv[1]=pForm->value("options");
        if(argv[1])sHtml::inputCGI(0, 2, argv, &gInsilico,0,false,"ARGS"); // dna-insilico parameters
    } else if(sIs(cmd,"-endl")){
        endline=1-endline;
    } else if(sIs(cmd,"-negFilter")){
        negFilter=1-negFilter;
    } else if(sIs(cmd,"-negSelect")){
        negSelect=1-negSelect;
    }else if(sIs(cmd,"-prefix")){
        strcpy(prefix,pForm->value("prefix",""));
    }else if(sIs(cmd,"-suffix")){
        strcpy(suffix,pForm->value("suffix",""));
    }else if(sIs(cmd,"-tmp")){
        strcpy(tmppath,pForm->value("tmp",""));
    }else if(sIs(cmd,"-wrap")){
        wrap=pForm->ivalue("columns",0);
    } else if(sIs(cmd,"-usemask")){
        const char * letters=pForm->value("letters","atgc");
        sBioseq::usemask(letters);
    } else if(sIs(cmd,"-complexity")){
        complexityWindow=pForm->ivalue("window",16);
        complexityEntropy=pForm->rvalue("entropy",1.);
    } else if(sIs(cmd,"-debug")){
        sPerf::gDebugStart=pForm->ivalue("start",-1);
        sPerf::gDebugEnd=pForm->ivalue("end",sPerf::gDebugStart);
    }
    return 0;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Variables
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
static idx __on_sel(sVioTools * , const char * cmd, const char * , const char * ,sVar * pForm)
{
    gHitlist.mex()->flags|=sMex::fSetZero;

    if(sIs(cmd,"-selectI")){
        FILE * fp=fopen(pForm->value("hitlist"),"r");
        if(!fp)return 0;
        char buf[1024];idx inum;
        gHitListDim=0;

        while( !feof(fp) ){
            inum=-1;
            fscanf(fp, "%"DEC,&inum);
            fgets(buf,sizeof(buf),fp);
            if(inum==-1)continue;

            gHitlist.resize((inum+1)/WSIZE+1); // for a bit array
            idx ibyte=inum/WSIZE;
            idx ibit=inum%WSIZE;
            gHitlist[ibyte]|=((udx)1)<<ibit;

            if(inum>gHitListDim)gHitListDim=inum;
        }
        fclose(fp);
    }

    return 0;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Data
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
static idx __on_fasta(sVioTools * QP, const char * cmd, const char * arg , const char * equCmd, sVar * pForm)
{
    if(sIs(cmd,"-vRedund")){

        const char * out=pForm->value("outfile");
        gFilter.inp("concat", "1");
        vParseSingle = true;
        __on_fasta(QP, "-vParse",arg, equCmd, pForm);
        gPrint.inp("qual", "1");
        gPrint.inp("hiverpt", "1");
        if (!kfile){ rmfile = 1;}
        if (nId){ gPrint.inp("noid", "1");}
        if(out && (strcmp(out, "stdout") != 0)){
            gPrint.inp("outfile", out);
        }
        __on_fasta(QP, "-vSeq",arg, equCmd, pForm);

    }
    if(sIs(cmd,"-vParse")){

        idx isID = gFilter.ivalue("noid");
        idx isQual = gFilter.ivalue("qual2bit");
        idx isConcat = gFilter.ivalue("concat");
        idx isVioDBMulti = gFilter.ivalue("single");
        idx maxChunk = gFilter.ivalue("maxChunk");
        idx isVioseqlist = gFilter.ivalue("vlist");
        idx filetype = gFilter.ivalue("filetype");

        idx flags=0;
        if(isID ) flags|=sVioseq2::eParseNoId;
        if(isQual ) flags|=sVioseq2::eParseQuaBit;
        if(isVioDBMulti ) flags|=sVioseq2::eParseMultiVioDB;
        if (isVioseqlist) flags|=sVioseq2::eCreateVioseqlist;
        if (filetype){
            if (filetype == 1){   //  Treat as a fasta or fa file
                flags |= sVioseq2::eTreatAsFastA;
            }
            if (filetype == 2){   //  Treat as a fastq or fq file
                flags |= sVioseq2::eTreatAsFastQ;
            }
            else if (filetype == 3){   // Is a sam file
                flags |= sVioseq2::eTreatAsSAM;
            }
            else if (filetype == 4){ // Annotation File
                flags |= sVioseq2::eTreatAsAnnotation;
            }
            else {  // by default use it as FastA file
                flags |= sVioseq2::eTreatAsFastA;
            }
        }


        idx cntTot=0;

        for (const char * infile=arg; infile; infile=sString::next00(infile)) {
            bool isStdIn=strcmp(infile,"stdin")==0 ? true : false ;

            sVioseq2 sub(0);
            //sub.complexityWindow=complexityWindow;
            //sub.complexityEntropy=complexityEntropy;
            if (!noLim)
                sub.setLimit(10000000);
//                sub.setLimit(1);

            sStr ext;

            if (isVioseqlist){
                ext.printf("vioseqlist");
            }
            else{
                ext.printf("vioseq2");
            }
            sFilePath o(infile,"%%pathx.%s",ext.ptr(0));
            // sFilePath in(infile,"%%pathx.sviodb");

            if (isConcat){
                sub.setSingleFile(true);
                pForm->inp("vioseq", o.ptr());
            }

            if(verbose)fprintf(stderr,"%s -> %s ",infile, isStdIn ? "stdout" : o.ptr() );
            idx subdim = sub.parseSequenceFile(isStdIn ? 0 : o.ptr(), infile, flags, maxChunk ? maxChunk : ((idx)(8)*1024*1024*1024), gPrimers.length() ? gPrimers.ptr() : 0  , complexityWindow , complexityEntropy);

            if( verbose && subdim > 0 ) {
                fprintf(stderr, "%"DEC"\n", sub.dim());
            }
            if( subdim > 0 ) {
                cntTot += subdim;
            } else if( subdim == -2 ) {
                QP->printf("\n ERROR:  You have surpassed the 10 million sequences");
                QP->printf("\n Please Contact: HIVE help desk in order to get the full version");
                QP->printf("\n ");
                break;
            }
            if (vParseSingle)
                break;
        }
        if (!isConcat){  // I don't want to print total sequences
            QP->printf("%"DEC"\n",cntTot);
        }
    }
    else if(sIs(cmd,"-vPrimers")){
        const char * primers=pForm->value("primers");
        sFil fil(primers);
        if(fil.length())sString::searchAndReplaceSymbols(&gPrimers, fil.ptr(),0, ",\n\r;",0,0,true,true,false,false);
    }
    else if(sIs(cmd,"-vCnt")){

        const sBioseq::EBioMode biomode = gPrint.ivalue("long")? sBioseq::eBioModeLong: sBioseq::eBioModeShort;
        sHiveseq sub(0, pForm->value("vioseq"), biomode);

        QP->printf("%"DEC,sub.dim());
        if(endline)QP->printf("\n");
    }
    else if(sIs(cmd,"-vAnnot")){
            // read static annotation file
            const char * path = pForm->value("vioannot_file");
            annot.init( path,sMex::fReadonly);
    }
    else if(sIs(cmd,"-vIDs")){
        sHiveseq sub(0,pForm->value("vioseq"), sBioseq::eBioModeShort);
        for( idx i=0; i<sub.dim(); ++i ){
            const char * id=sub.id(i);
            const char * s=strpbrk(id," \t");
            if(s)::printf("%.*s\n",(int)(s-id),id);
            else ::printf("%s\n",id);
        }
    }
    else if(sIs(cmd,"-vSeq")){
PERF_START("Init");
        idx predefinedSeqStart=pForm->ivalue("seqStart",0);
        idx seqLen=pForm->ivalue("seqLen",0);
        idx isId=gPrint.ivalue("id");
        idx isNoID=gPrint.ivalue("noid");
        idx isLen=gPrint.ivalue("len");
        idx isNum=gPrint.ivalue("num");
        idx isQual=gPrint.ivalue("qual");
        idx isQuabit=gPrint.ivalue("quabit", 1);
        idx isPos=gPrint.ivalue("pos");
        const sBioseq::EBioMode biomode = gPrint.ivalue("long")? sBioseq::eBioModeLong: sBioseq::eBioModeShort;
        idx noseq=gPrint.ivalue("noseq");
        idx hiverpt=gPrint.ivalue("hiverpt")?1:0;
        idx cutspace=gPrint.ivalue("cutspace") ? 1 : 0 ;
        idx annotMode=gPrint.ivalue("annotMode") ? 1 : 0 ;
        const char * annotType=gPrint.value("annotType") ;

//        idx isHiveseq=gPrint.ivalue("hiveseq")?1:0;
        idx totLen=0;
        const char * out=pForm->value("outfile");


        sFil ofil;
        if(out && (strcmp(out, "stdout") != 0)){
            sFile::remove(out);
            ofil.init(out);
        }

        sHiveseq sub(0,pForm->value("vioseq"), biomode);//sub.reindex();


        limits(Len);
        //const char * idcont=gFilter.value("id");
        //idx hitDim=gHitlist.dim()*WSIZE;
        udx * hitlist=gHitListDim ? gHitlist.ptr() : 0;

        idx maxnum= (cnt!=sIdxMax) ?  start+cnt : sub.dim();
        if(maxnum>sub.dim())maxnum=sub.dim();



PERF_END();
        sStr t, oQP;
        for( idx i=start; i<maxnum ; ++i ){
            endline=0;
            idx seqStart =0;
            if (predefinedSeqStart) seqStart = predefinedSeqStart;
PERF_START("SEQUENCE");
            idx sublen=sub.len(i);
            bool quaBit = false;
            if (isQuabit){
                quaBit = sub.getQuaBit(i);
            }

            if (isQual && sub.qua(i) == 0)
                isQual = 0;

            if( hitlist ) {
                if( i>=gHitListDim ) break;
                idx ibyte=i/WSIZE;
                udx ibit=(((udx)1)<<(i%WSIZE));
                if( ( hitlist[ibyte]&ibit)!=0 && negSelect==1 ) continue;
                if( ( hitlist[ibyte]&ibit)==0 && negSelect==0 ) continue;
            }

            compare(Len,sublen);
            //compareS(idcont,id);

            const char * id=sub.id(i);
            idx slen=seqLen ? seqLen : sublen;

            idx irangeStart=0,cntRanges=1;
            idx irangeJointStart=0,cntRangeJoints=1;

            idx * indexRangePtr =0;
            if(annot.isok()) {
                indexRangePtr = annot.getNumberOfRangesByIdTypeAndId("seqID",id, cntRanges);
            }

            for( idx irange=irangeStart; irange<cntRanges; ++irange) {

                sVioAnnot::startEnd * rangePtr=0;
                const char * annotID=0, * annotIDType=0;
                if(annot.isok() && indexRangePtr) {

                    if(annotType){ // check if we are interested only in a particular type of annotations : exome, mrna, etc
                        // if(not the type )
                        //     continue;
                        // annotID, annotIDType
                        idx cntIDsForRange=annot.getNberOfIdsByRangeIndex(indexRangePtr[irange]);
                        for( idx iid=0; iid<cntIDsForRange; ++iid)  {  /* loop for id list */
                            const char * idPtr,*idTypePtr;
                            annot.getIdTypeByRangeIndexAndIdIndex(indexRangePtr[irange], iid, &idPtr, 0, &idTypePtr, 0);
                            if (strcasecmp(idTypePtr,annotType)==0){
                                annotID = idPtr;
                                annotIDType = idTypePtr;
                                break;
                            }
                            else continue;
                        }

                    }
                    idx cntRangeJoints=0;
                    rangePtr = annot.getRangeJointsByRangeIndex(indexRangePtr[irange],&cntRangeJoints);
                    cntRangeJoints/=sizeof(sVioAnnot::startEnd);
                    if(annotMode!=1){
                        seqStart = rangePtr[0].start;
                        slen = rangePtr[cntRangeJoints-1].end-seqStart;
                        cntRangeJoints=1;
                    }

                }

                bool supressIDline=false;
                for( idx irangeJoint=irangeJointStart; irangeJoint<cntRangeJoints; ++irangeJoint) {


                    if(annot.isok() && annotMode==1 && rangePtr){
                        seqStart = rangePtr[irangeJoint].start;
                        slen = rangePtr[irangeJoint].end-seqStart;
                        if(annotMode==2 || irangeJoint!=0)
                            supressIDline=true;
                    }

                    if(!isId){
                        if (isQual)
                            oQP.printf("@");
                        else
                            oQP.printf(">");
                    }
                    if(isNum)oQP.printf("%"DEC"-",i);
                    //const char * id=sub.id(i);
                    if(!isNoID){
                        if(id[0]=='>' || id[0] == '@')++id;
                        oQP.printf("%s",id);
                        if(annot.isok() ) {
                            if(!supressIDline ) {
                                // annot.get irange find id/idtype pairs from annotation into -> annotID, annotIDType
                                if(annotID) {
                                    oQP.printf(" %s:%s:%"DEC"-%"DEC,annotIDType, annotID,seqStart, slen+seqStart);
                                }else  {
                                    oQP.printf(" %"DEC"-%"DEC,seqStart, slen+seqStart);
                                }
                            }
                        }
                        if(cutspace){
                            char * p=strpbrk(oQP.ptr(),sString_symbolsSpace);
                            if(p){*p=0;
                                oQP.cut(p-oQP.ptr());
                            }
                        }

                    }
                    if(!supressIDline ) {
                        idx subrpt = sub.rpt(i);
                        if(isLen){oQP.printf(" len=%"DEC" rpt=%"DEC" sim=%"DEC" ",sublen,subrpt, sub.sim(i)); totLen+=sublen;}
                        if(isPos){oQP.printf(" range=%"DEC"-%"DEC" ",seqStart,seqStart+seqLen); }
            //            if(isReverse){oQP.printf(" rev"); }
                        if(hiverpt && (subrpt != 1)){
                            oQP.printf(" H#=%"DEC"", sub.rpt(i));
                        }

                        //else oQP->printf("\n");

                    }
                    if(isId ){if(!endline)oQP.printf("\n");continue;}
                    const char * seq=sub.seq(i);



                    t.cut(0);
                    PERF_END();
                    PERF_START("SEQUENCE-OUTPUT");
                    if(!noseq){
                        if (!isComplement){
                            sBioseq::uncompressATGC(&t,seq, seqStart, slen ,true,wrap, isReverse);
                        }
                        else {
                            sBioseq::uncompressATGC(&t,seq, seqStart, slen ,true, wrap, isReverse, isComplement);
                        }

                        if (isQual){
                            const char * qua=sub.qua(i);
                            for (idx i=0; i<slen; ++i ){
                                if (qua[i+seqStart] == 0)
                                    t[i]='N';
                            }
                        }
                        if (quaBit){
                            const char * qua=sub.qua(i);
                            if (qua){
                                for (idx i=0; i<slen; ++i ){
                                   if (sub.Qua( qua, i+seqStart, true) == 0){
                                       t[i]='N';
                                   }
                                }
                            }
                        }
                        oQP.printf("\n%s",t.ptr(0));
                        if(slen<sublen)
                            oQP.printf("...");
                    }
        PERF_END();
                    if(isQual){
        PERF_START("QUALITIES");

                        const char * qua=sub.qua(i);
                        if(qua){
                            oQP.printf("\n+\n");
                            idx qlen=seqLen ? seqLen : sublen;
                            for (idx iq=0; iq<qlen; ++iq ){
                                oQP.printf("%c",qua[iq]+33);
                            }
                            if(qlen<sublen)
                                oQP.printf("...");
                        }
        PERF_END();
                    }

                    if(!noseq || !endline)oQP.printf("\n");

                    oQP.add0();
                    if(out && (strcmp(out, "stdout") != 0)){
                        ofil.printf("%s", oQP.ptr(0));
                    }
                    else{    // stdout
                        QP->printf("%s", oQP.ptr(0));
                    }
                    oQP.cut(0);
                }
            }
        }
        if(isLen && cnt>1)
            oQP.printf("%"DEC" total\n",totLen);
        oQP.add0();
        if(out && (strcmp(out, "stdout") != 0)){
            ofil.printf("%s\n", oQP.ptr(0));
        }
        else{    // stdout
            QP->printf("%s\n", oQP.ptr(0));
        }
        if (rmfile){
            sVioDB dbrm(pForm->value("vioseq"));
            sFile::remove(pForm->value("vioseq"));
            dbrm.deleteAllJobs();
        }

PERF_PRINT();
    }
    else if(sIs(cmd,"-fSeq")){
        sBioseq::initModule(sBioseq::eATGC);

        sVioseq sub(pForm->value("vioseq"));
        idx seqStart=pForm->ivalue("seqStart",0);
        idx seqLen=pForm->ivalue("seqLen",0);
        idx isId=gPrint.ivalue("id");
        idx isNoID=gPrint.ivalue("noid");
        idx isLen=gPrint.ivalue("len");
        idx isNum=gPrint.ivalue("num");
        idx isQual=gPrint.ivalue("qual");
        idx isQualBit=gPrint.ivalue("quabit");
        idx isPos=gPrint.ivalue("pos");

        //idx isSort = gFilter.ivalue("sort");

        idx totLen=0;
        //sort()

        limits(Len);
        const char * idcont=gFilter.value("id");
        //idx hitDim=gHitlist.dim()*WSIZE;
        udx * hitlist=gHitListDim ? gHitlist.ptr() : 0;

        for( idx i=start; i<start+cnt && i<sub.dim() ; ++i ){
            idx len=sub.len(i);
            const char * id=sub.id(i);if(id[0]=='>')++id;


            if( hitlist ) {
                if( i>=gHitListDim ) break;
                idx ibyte=i/WSIZE;
                udx ibit=(((udx)1)<<(i%WSIZE));
                if( ( hitlist[ibyte]&ibit)!=0 && negSelect==1 ) continue;
                if( ( hitlist[ibyte]&ibit)==0 && negSelect==0 ) continue;
            }

            compare(Len,len);
            compareS(idcont,id);

            if(!isId)QP->printf(">");
            if(isNum)QP->printf("%"DEC" ",i);
            if(!isNoID)QP->printf("%s",id);
            if(isLen){QP->printf(" len=%"DEC" ",len); totLen+=len;}
            if(isPos){QP->printf(" range=%"DEC"-%"DEC" ",seqStart,seqStart+seqLen); }

            //else QP->printf("\n");
            if(isId ){if(endline)QP->printf("\n");continue;}
            const char * seq=sub.seq(i);
            idx sublen=sub.len(i), slen=seqLen ? seqLen : sublen;
            sStr t; sBioseq::uncompressATGC(&t,seq, seqStart, slen ,true,wrap);
            QP->printf("\n%s",t.ptr(0));
            if(slen<sublen)
                QP->printf("...");
            if(isQual){
                const char * qua=sub.qua(i);
                if(qua){
                    QP->printf("\n+\n");
                    idx qlen=seqLen ? seqLen : sublen;
                    if(isQualBit || (sub.hdrR.flags&sVioseq::eParseQuaBit)) {
                        for( idx iq=seqStart; iq<qlen; ++iq ){
                            //QP->printf("%"DEC " ",(idx)qua[iq/8]&((idx)1)<<(iq%8) ? 40 : 10);
                            //if(wrap && iq && (iq%wrap)==0)QP->printf("\n");
                            idx qqq=(idx)qua[iq/8]&((idx)1)<<(iq%8) ? 40 : 10;
                            QP->printf("%c",(idx)(qqq+33));

                        }
                    } else {
                        for( idx iq=0; iq<len; ++iq ){
                            //QP->printf("%"DEC " ",(idx)qua[iq]);
                            QP->printf("%c",(idx)(qua[iq]+33));
                            //if(iq && (iq%wrap)==0)QP->printf("\n");
                        }
                    }
                    if(qlen<sublen)
                        QP->printf("...");
                }
            }

            if(endline)QP->printf("\n");
        }
        if(isLen && cnt>1)
            QP->printf("%"DEC" total\n",totLen);

        sBioseq::initModule(sBioseq::eATGC);

    }

    else if(sIs(cmd,"-vRand")){
        //sRand trand;

        //Grab vioseq file----------------------------------------------------------------------//
        sHiveseq sub(0, pForm->value("vioseq"));//sub.reindex();

        if(!sub.dim()){QP->printf("File missing or corrupted\n");return 0;}
        //--------------------------------------------------------------------------------------//

        //=------
        const char * out=pForm->value("outfile");

        sFil ofil;
        if(out && (strcmp(out, "stdout") != 0)){
            sFile::remove(out);
            ofil.init(out);
        }

        sFilterseq::randomizer(ofil,sub,cnt,noiseRand,lenMinRand,lenMaxRand,isNrevCompRand,qualitySymbol?true:false,isLengthNormalized,pForm->value("subset",0),pForm->value("mutations"));

        if(!out || (strcmp(out, "stdout") == 0))
            QP->printf("%s",ofil.ptr());

    } else if(sIs(cmd,"-vRecomb")){

        idx sStart=0,seqLen=0;
        sHiveseq sub(0,pForm->value("vioseq"));sub.reindex();
        const char * onm=pForm->value("outfile");
        //sStr ovioseq,ofasta;
        sFilePath o;
        if(!onm)onm=o.makeName(pForm->value("vioseq"),"%%pathxComb.fasta");
        sFil ofil;
        ofil.init(onm);
        if(ofil.length())
        {
            sFile::remove(onm);
            ofil.init(onm);
        }
        //ovioseq.printf(0,"%s.vioseq",onm);
        //ofasta.printf(0,"%s.fasta",onm);
        sStr fastaTitle;fastaTitle.printf(">RECOMBINANT (");
        const char * recomb=pForm->value("recomb","false");
        sStr rst, t,out;
        sString::searchAndReplaceSymbols(&rst, recomb, 0, ",",0,0,true , true, false, true);
        QP->printf(" recomb=%s\n",recomb);
        if(strstr(recomb,"false"))
            QP->printf(" ->must provide recombination events \n");
        else{
            for(const char * nPtr=rst.ptr(); *nPtr ; nPtr+=strlen(nPtr)+1 ){
                sStr subid;
                sString::searchAndReplaceSymbols(&subid, nPtr, 0, ":",0,0,true , true, false, true);
                idx i=atol(subid.ptr());
//                QP->printf(" id=%"DEC,i);
                if(i<sub.dim())
                {
                    sStr subID;sString::searchAndReplaceSymbols(&subID, sub.id(i), 0, ","," ",0,true,true,false,false);
                    fastaTitle.printf(" %s",subID.ptr());
                    const char * rng=subid.ptr()+strlen(subid.ptr())+1;
                    if(*rng && strcmp(rng,"")!=0)
                    {
                        sStr subrstr;
                        sString::searchAndReplaceSymbols(&subrstr, rng , 0, "-",0,0,true , true, false, true);
                        sStart=atol(subrstr.ptr());

                        const char * lenrng=subrstr.ptr()+strlen(subrstr.ptr())+1;
                        if(*lenrng && strcmp(lenrng,"")!=0){
                            seqLen=atol(lenrng);
                            if(!seqLen)
                                seqLen=sub.len(i)-sStart;
                        }
                        else
                            seqLen=sub.len(i)-sStart;
                        fastaTitle.printf(":%"DEC"-%"DEC",",sStart,seqLen);
                        //QP->printf(" start=%8"DEC" , length=%8"DEC"\n",sStart,seqLen);
                    }
                    else
                        QP->printf(" whole sequence used \n");
                    const char * seq=sub.seq(i);
                    sBioseq::uncompressATGC(&t,seq, sStart, seqLen ,true,wrap);
                    t.add(0);
                }
                else
                    QP->printf("%4"DEC" NOT FOUND",i);
            }
            t.add0(2);
            fastaTitle.printf(fastaTitle.length()-1,")");

            QP->printf("\nName of the genome:\n  %s\n",fastaTitle.ptr());
            sStr rcmbStr;rcmbStr.printf(",%s",recomb);
            recomb=rcmbStr.ptr();
            sStr tcmb;
            /*while(*recomb){
                sStr sid;
                sString::extractSubstring(&sid,recomb,0,1,","_":"__,0,0);
                i=atol(sid.ptr());
                sid.printf(0,"%s"_"%s",sub.id(i)+1,sub.id(i)+1);
                recomb=sString::cleanMarkup(recomb,0,","_","__,","_":"__,sid.ptr(),1,false,false);
            }*/

            out.printf(0,"%s\n",fastaTitle.ptr());
            for(const char *nPtr=t.ptr();nPtr;nPtr=sString::next00(nPtr))
                out.printf("%s",nPtr);
            out.printf("\n");
            //QP->printf("%s" ,out.ptr());
        }
        ofil.printf("%s",out.ptr(0));

    }   else if (sIs(cmd, "-vInsilico")){

        // Load output file
        const char * onm=pForm->value("outfile");
        sFilePath o1, o2;
        idx numReads = 0;

        // Load input files
        bool useMutFile = false;
        bool useAnnotFile = false;
        // 1. sub
        sHiveseq sub(0,pForm->value("infile"), sBioseq::eBioModeLong);
        if(!sub.dim()){
            sub.parse(pForm->value("vioseq"), sBioseq::eBioModeLong, false);
            if (!sub.dim()){
                QP->printf("\nERROR: File missing or corrupted\n");return 0;
            }
        }
        // 2. mutation
        const char * mutFile = gInsilico.value("mutFile");
        if (mutFile){
            useMutFile = true;
        }
        // 3. annotation
        const char * annotFile = gInsilico.value("annotFile");
        sIon ionAnnot (annotFile);
        sIonWander wander(&ionAnnot);
//        wand.addIon()(annotFile);
        if (annotFile && ionAnnot.ok()){
            useAnnotFile = true;
        }

        sStr err;
        sRandomSeq randseq;
        randseq.init();
        randseq.setPrefixID("HIVE-vioapp ");

        if( !randseq.loadParticularAttributes(gInsilico, &err, sub, useMutFile, useAnnotFile) ) {
            QP->printf("%s\n", err.ptr());
            return 0;
        }
        bool parseDicValidation = sFilterseq::parseDicBioseq(randseq.idlist, sub, &err);
        if( !parseDicValidation ) {
            QP->printf("%s\n", err.ptr());
            return 0;
        }
        if( useMutFile ) {
            // load the csv Table and put it in RefSeqs
//            idx filetype=0;
//            sTxtTbl * tbl = randseq.tableParser(mutFile, 0, 0, filetype);
            if( !randseq.readTableMutations(mutFile, &sub, &err) ) {
                QP->printf("%s", err.ptr());
                return 0;
            }
        }
        idx randseqFlags = randseq.getFlags ();
        if( randseqFlags & sRandomSeq::eSeqParseAnnotFile ) {
            // Important: sRandomSeq::ionWanderCallback assumes the result is going to be in
            // the variable a=find.annot()
            wander.traverseCompile("b=foreach.seqID(\"\");a=find.annot(seqID=b.1,type='strand');");
            randseq.Sub = &sub;
            wander.callbackFunc = sRandomSeq::ionWanderCallback;
            wander.callbackFuncParam = (void *) &randseq;
            wander.traverse();
        }
        randseq.mutationFillandFix(sub);
        if( (randseqFlags & sRandomSeq::eSeqParseMutationFile) && (randseqFlags & sRandomSeq::eSeqGenerateRandomMut) ) {
            bool success = randseq.generateRandomMutations(sub, err);
            if( !success ) {
                QP->printf("%s", err.ptr());
                return 0;
            }
        }
//        sDic <idx> cntRowProb;
        if( (randseqFlags & sRandomSeq::eSeqPrintRandomReads) && (randseqFlags & sRandomSeq::eSeqPrintPairedEnd) ) {
            // Generate two paired end reads
            const char *outfile1;
            const char *outfile2;
            if (!onm){
                outfile1 = o1.makeName(pForm->value("vioseq"),"%%pathx_1.%s",(randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
                outfile2 = o2.makeName(pForm->value("vioseq"),"%%pathx_2.%s",(randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
            }
            else {
                outfile1 = o1.makeName(pForm->value("outfile"),"%%pathx_1.%s",(randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
                outfile2 = o2.makeName(pForm->value("outfile"),"%%pathx_2.%s",(randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
            }
            sFil readsFile1(outfile1);
            sFil readsFile2(outfile2);
            if( !readsFile1.ok() || !readsFile2.ok()) {
                QP->printf("failed to create destination file");
                return 0;
            }
            readsFile1.empty();
            readsFile2.empty();
            numReads = randseq.randomize(&readsFile1, sub, err, &readsFile2, 0);

            if( !numReads || (readsFile1.length() == 0) || (readsFile2.length() == 0) ) {
                QP->printf("%s", err.ptr());
                return 0;
            }
        } else {
            // Generate only one read
            sStr outReadsPath;
            const char *outfile;
            if (!onm){
                outfile = o1.makeName(pForm->value("vioseq"),"%%pathx.%s",(randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
            }
            else {
                outfile = o1.makeName(pForm->value("outfile"),"%%pathx.%s",(randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
            }
            sFil readsFile(outfile);
            if( !readsFile.ok() ) {
                QP->printf("failed to create report destination file");
                return 0;
            }
            readsFile.empty();
            numReads = randseq.randomize(&readsFile, sub, err, 0, 0);

            if( !numReads || readsFile.length() == 0 ) {
                QP->printf("%s", err.ptr());
                return 0;
            }
        }
//        if (cntRowProb.dim()){
//            const char *outfile;
//            if( !onm ) {
//                outfile = o1.makeName(pForm->value("vioseq"), "%%pathx_report.csv");
//            } else {
//                outfile = o1.makeName(pForm->value("outfile"), "%%pathx_report.csv");
//            }
//            sFil readsFile(outfile);
//            if( !readsFile.ok() ) {
//                QP->printf("failed to create destination file");
//                return 0;
//            }
//            readsFile.empty();
//            randseq.outrowProbInfo(&readsFile, &cntRowProb);
//        }
        if( randseqFlags & sRandomSeq::eSeqMutations ) {
            const char *csvTableOutput;
            const char *vcfTableOutput;
            sFilePath oaux1, oaux2;
            if (!onm){
                csvTableOutput = oaux1.makeName(pForm->value("vioseq"),"%%pathx_mutationTable.csv");
                vcfTableOutput = oaux2.makeName(pForm->value("vioseq"),"%%pathx_mutationTable.vcf");
            }
            else {
                csvTableOutput = oaux1.makeName(pForm->value("outfile"),"%%pathx_mutationTable.csv");
                vcfTableOutput = oaux2.makeName(pForm->value("outfile"),"%%pathx_mutationTable.vcf");
            }
            sFil csvFile(csvTableOutput);
            if( !csvFile.ok() ) {
                QP->printf("failed to create destination CSV file");
                return 0;
            }
            csvFile.empty();
            bool success = randseq.mutationCSVoutput(&csvFile, sub, err);
            if( !success ) {                //IF WE WERE UNABLE TO LAUNCH ALIGNMENT
                QP->printf("%s", err.ptr());
                return 0;
            }
            // Create an output for the mutation parameters as a VCF file
            sFil vcfFile(vcfTableOutput);
            if( !vcfFile.ok() ) {
                QP->printf("failed to create destination VCF file");
                return 0;
            }
            vcfFile.empty();
            success = randseq.mutationVCFoutput(&vcfFile, "hive_vioapp", sub, err);
            if( !success ) {
                QP->printf("%s", err.ptr());
                return 0;
            }
        }

        if (useMutFile){
            QP->printf("Insilico - Successfully parsed mutation File: %s\n", mutFile);
        }
        if (useAnnotFile){
            QP->printf("Insilico - Successfully parsed annotation File: %s\n", annotFile);
        }
        QP->printf("Insilico - Successfully generated %"DEC" short reads in: %s", numReads, o1.ptr());
        if (randseqFlags & sRandomSeq::eSeqPrintPairedEnd){
            QP->printf(" and %s\n", o2.ptr());
        }
        else {
            QP->printf("\n");
        }
        QP->printf("Done !!!\n");

    } else if( sIs(cmd, "-tScreening") ) {

        // Load output file
        const char * startDirectory = pForm->value("directory", ".");
        bool deepSearch = pForm->boolvalue("deepSearch", true);
        idx limitFiles = pForm->ivalue("limit", sIdxMax);
        bool overwrite = pForm->boolvalue("overwrite", false);

        if (limitFiles <= 0 ){
            limitFiles = sIdxMax;
        }

        if( !startDirectory ) {
            QP->printf("Can't recognize the path: %s\n", startDirectory);
            return 0;
        }
        if (!taxpathfile.length()){
            QP->printf("a path for the ion database must be specify with: -taxPath filepath\n");
            return 0;
        }
        // Load the taxonomy database ver
        sTaxIon taxIon(taxpathfile.ptr(), false);
        taxIon.precompileGItoAcc();
        if (!taxIon.isok()){
            QP->printf("There was an error at loading the database: %s\n", taxpathfile.ptr());
            return 0;
        }
        sDir dirList;
        const char *srcExtension = "dna-alignx_gicenteric.csv";
        const char *dstExtension = "dna-alignx_acclist.csv";
        idx findFlags = sFlag(sDir::bitFiles);
        findFlags |= (deepSearch) ? sFlag(sDir::bitRecursive) : 0;
        idx cntFiles = dirList.find(findFlags, startDirectory, srcExtension, 0, 0, 0);
        if (!cntFiles){
            QP->printf("It can't find any file to modify\n");
            return 0;
        }
        sFilePath dstflnm;
        idx cntRows =0;
        idx skippedRows=0;
        idx ie = 0;
        for(ie = 0; ie < dirList.dimEntries(); ie++) {
            const char * srcfilename = dirList.getEntryAbsPath(ie);
            // Check that source file exists
            if (!sFile::exists(srcfilename)){
                ++skippedRows;
                continue;
            }

            dstflnm.cut(0);
            dstflnm.makeName(srcfilename, "%%dir/%s",dstExtension);

            if (!overwrite && sFile::exists(dstflnm.ptr())){
                ++skippedRows;
                continue;
            }
            sFil out(dstflnm);
            out.empty();
            idx rowsFound = taxIon.dnaScreeningGItoAcc(out, srcfilename);
            cntRows += (rowsFound > 0) ? 1 : 0;
            if (cntRows >= limitFiles){
                break;
            }
        }
        QP->printf("Files found with extension \"%s\": %"DEC"\n", srcExtension, cntFiles);
        QP->printf("Files skipped: %"DEC"\n", skippedRows);
        QP->printf("Files created with extension \"%s\": %"DEC"\n", srcExtension, cntRows);
    } else if( sIs(cmd, "-about") ) {
        QP->printf("Hive (ver. 0.5) Credits !!!!\n");
    }

    return 0;
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/  Initialization
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

void sVioTools::printf(const char * formatDescription , ...)
{
    if(gSilent)return;
    sStr str;sCallVarg(str.vprintf,formatDescription);
    if(str.length())fwrite(str.ptr(), str.length(),1,stdout) ;
}

idx sVioTools::CmdForm(const char * cmd , sVar * pForm)
{
    for ( idx i=0; cmdExes[i].param!=sNotPtr; ++i) {  // see if this is a sQPrideClient command
        if ( cmdExes[i].cmd ==0 ) continue;
        if( strcmp(cmdExes[i].cmd , cmd )==0 ) {
            sCmdLine cmdline;char equCmd[128];equCmd[0]=0;
            cmdline.exeFunCaller(&cmdExes[i], cmd , 0, equCmd, pForm);
            return 1;
        }
    }
    return 0;
}


sCmdLine::exeCommand sVioTools::cmdExes[]={
     {0,0,0,"","List of available commands"},

    {0,0,0,      "\t","\n\nSettings variables\n"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-biomode"," mode                   // set the biomode "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-verbose"," level                  // define the level of debug outputs "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-silent"," level                   // defines if there are outputs "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-version","                        // show the version of the program "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-start"," value                    // set the start elements for prints "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-cnt"," value                      // limit the number of elements printed "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-wrap"," columns                   // wrap printed fasta "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-usemask"," letters                // do not use the following letters "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-complexity"," window entropy      // set complexity filtering parameters "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,                  "-endl","                           // toggle to print newline at the end of output "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-filter"," conditions              // filters to use "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-prefix"," prefix                  // to attach to the front of sequence ids "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-suffix"," make cl                 // to attach to the tail of sequence ids "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-tmp"," tmpfile                    // to create the file in a new directory "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-negFilter","                      //  "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-negSelect","                      //  "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-print"," values                   // what to print "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-insilico"," options               // insilico parameters "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-debug"," start end                // range to debug "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-noise"," value                    // Randomizer: generate noise in the reads (default:0) "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-qualitySymbol"," value            // Randomizer: generate fastq format with constant quality pattern"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-readLen"," min max                // Randomizer: generate reads of length [min-max] (default: [100-100])"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-range"," start end                // Randomizer: generate reads randomly starting from position 'start' until position 'end' (default: 0, length of genome)"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-user"," userID                    // Identifies user"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-mutFreq"," value                  // Randomizer: create mutation of specific frequency (default:0.4) "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-keepfile","                       // Keep the non-redundant vioseq2 file "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-rmfile","                         // Delete the vioseq2 file after -vSeq "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-noID","                           // Remove ID's"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-iscomp","                         // show complement in sequences"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-isrev","                         // show reverse in sequences"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-NoRevComp","                      // Randomizer: generate Reads from reverse complement too (default: 0) "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-taxPath"," taxpathfile               // Screening: taxonomy database ION path"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-lengthNormalized","               // Randomizer: Normalize the ratio of generated reads based on the length of the references "},

    {0,0,0,      "\t","\n\nSelection commands\n"},
    {0,(sCmdLine::exeFunType)&__on_sel,sCmdLine::argOneByOne,           "-selectI"," hitlist                // select elements by their hitlist file "},


    {0,0,0,      "\t","\n\nFasta parsing\n"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllZeroList,      "-vPrimers"," primers               // define primers for parsing"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllZeroList,      "-vParse"," infile                  // parse a fasta/fastq without repetitions"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllZeroList,      "-vRedund"," infile outfile         // parse a fasta/fastq and delivers a fasta/fastq file without repetitions"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllZeroList,      "-vCnt"," vioseq                    // count sequences "},


    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-fSeq"," vioseq seqStart seqLen    // output sequences "},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-vAnnot"," vioannot_file           // initialize annotation file  "},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-vIDs"," vioseq                    // output IDs "},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-vSeq"," vioseq outfile seqStart seqLen    // output sequences for vioseq2 "},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-vRand"," vioseq outfile subset mutations  // output random subsequences. subset format: refID1,ratio1:refID2,ratio2:refID3-refID8:ratio3. eg 0,0.1:1,0.3:2-4:0.2"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-vRecomb"," vioseq recomb outfile    // create recombinative genomes "},

    {0,0,0,      "\t","\n\nInsilico tool\n"},
{0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-vInsilico"," infile outfile           // create insilico data"},

    {0,0,0,      "\t","\n\ndna-screening tool\n"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argAllSpacedList,    "-tScreening"," directory limit overwrite   // translate gi based tables into accession based files"},


    {0,0,0,      "\t","\n\nCredits ...\n"},
    {0,(sCmdLine::exeFunType)&__on_fasta,sCmdLine::argNone,           "-about","                           // Show credits "},

    {0,0,0,      "\t","\n\nHelp commands\n"},
    {0,(sCmdLine::exeFunType)&__on_help,sCmdLine::argAllZeroList,    "-help"," command // help"},

    {sNotPtr,0}
};


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Help
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
static idx __on_help(sVioTools * QP, const char * cmd, const char * , const char * ,sVar * pForm)
{
    if(sIs(cmd,"-help")){

        const char * which=pForm->value("command");
        for ( idx i=0; sVioTools::cmdExes[i].param!=sNotPtr ; ++i ) {
            if( which && strcmp(which,sVioTools::cmdExes[i].cmd+1)!=0)continue;
            QP->printf("\t%s%s\n", sVioTools::cmdExes[i].cmd,sVioTools::cmdExes[i].descr);
        }
    }
    return 0;
}

// vioapp -vParse dodo.fa dodo.vioseq -insilico "input=.vioseq2&..."
