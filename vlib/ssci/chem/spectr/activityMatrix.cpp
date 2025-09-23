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
#include <ctype.h>

#include <ssci/chem/spectr/spectraFile.hpp>

void actMatOut(sStr * out, outPar * op, real * pVal, idx row, idx col)
{

    if(row==-1 && col==-1) out->printf("peaks");
    else if(col==-1){out->printf("%s",op->flnms->id(op->roset ? (*op->roset)[row] : row));}
    else if(row==-1){
        if(op->colset) col=(*(op->colset))[col];
        sSpctr::rMassPrf * rmp=(sSpctr::rMassPrf*)op->peaks->id(col);
        out->printf("%s%lf-%s",  (char*)(&(rmp->rPrfx)),rmp->rMass, op->peaks->mex()->ptr((*(op->peaks))[col]) );
    }
    return ;
}


idx sSpctr::prepareAMat(const char * path, const char * fls)
{
    gLog->printf("\n");
    gLog->printf("_/_/_/_/_/_/_/_/_/_/_/_/_/\n");
    gLog->printf("_/\n");
    gLog->printf("_/ Collecting activity matrix \n");
    gLog->printf("_/\n");
    gLog->printf("_/_/_/_/_/_/_/_/_/_/_/_/_/\n");
    gLog->printf("\n");
    sStr actWork("%s/act_work",path);
    sDir::makeDir(actWork);

    sDic<idx> peaks;

    sStr fpeakset("%s/peakset",actWork.ptr());
    peaks.init(fpeakset.ptr(0));
    peaks.empty();

    sDic < clsFile > flnms;
    sStr fflnmset("%s/flnmset",actWork.ptr());
    flnms.init(fflnmset.ptr());
    flnms.empty();


    sDir dir;
    sVec < sVec <clsPeak> > cpListList;
    real dummy;
    char name[1024],fmt[128],namshft[1024];
    sStr fomt;
    clsPeak cp;
    char * p; idx ifile;
    sStr ptl;

    sprintf(fmt,"*%s",fls);
    if(gSet.pathList ) {
        gLog->printf("multiple directories are to be analysed\n");
        sString::searchAndReplaceSymbols(&ptl,gSet.pathList,0,"="sString_symbolsBlank,0,0,true,true,true);
    }
    else ptl.add0(4);

    sVec <real> excl;
    if(gSet.collect.exclpeaks.length()){
        real ex=0;
        gLog->printf("reading peak exclusion list\n");
        for(const char * p=gSet.collect.exclpeaks.ptr(); p && *p; p=sString::skipWords(p,0,1,",;"sString_symbolsBlank) ) {
            if(sscanf(p,"%lf", &ex)){
                excl.vadd(1,ex);
                gLog->printf("exclude peak %lf\n",ex);
            }
        }
        gLog->printf("\n");
    }

    sVec <real> incl;
    if(gSet.collect.inclpeaks.length()){
        gLog->printf("reading peak inclusion-only list\n");
        real inc=0;
        for(const char * p=gSet.collect.inclpeaks.ptr(); p && *p; p=sString::skipWords(p,0,1,",;"sString_symbolsBlank) ) {
            if(sscanf(p,"%lf", &inc))incl.vadd(1,inc);
            gLog->printf("include peak %lf\n",inc);
        }
        gLog->printf("\n");
    }
    sStr exclf;
    if(gSet.collect.exclfiles) {
        gLog->printf("reading sample file exclusion list\n%s\n",gSet.collect.exclfiles.ptr());
        sString::searchAndReplaceSymbols(&exclf,gSet.collect.exclfiles.ptr(),0,",;"sString_symbolsBlank,0,0,true,true,true);
    }

    sFilePath curd; curd.curDir();

    for(const char * path=ptl.ptr(); path ; path=sString::next00(path,2)){
        const char * prfx=path;const char * dirini=sString::next00(path);
        sDir::chDir(curd);
        if(*path)sDir::chDir(dirini);
        dir.cut(0);
        dir.list(sFlag(sDir::bitFiles)|sFlag(sDir::bitNamesOnly), ".", fmt);


        for(p=dir.ptr(), ifile=0; p ; p=sString::next00(p) , ++ifile ) {
            gLog->printf("\n\nReading file %d named %s",  ifile+1, p);
            if(*prfx)gLog->printf("from a dir %s prefixed %s",dirini,prfx);
            gLog->printf(":");

            const char * flexl;
            for(flexl=exclf.ptr(); flexl && *flexl; flexl=sString::next00(flexl,1)){
                if(strstr(p,flexl))break;
            }
            if(flexl) {
                gLog->printf("excluded\n");
                continue;
            }

            sFilePath fp(p, SRC_FILES_PATH"%%flnmx_%s.csv", gSet.collect.pmajfiles);

            sFil fl(fp.ptr(),sFil::fReadonly);
            if(fl.length()==0){
                gLog->printf("truncated, skipping.\n");
                continue;
            }
            gLog->printf("\n");


            char flnmbuf[4096];
            strcpy(flnmbuf,p);
            if(gSet.collect.join) {
                char * last_=strrchr(p,'_'), * dig;
                if(last_ && isdigit(last_[1]) ){
                    for( dig=last_+1; *dig!=gSet.files[0];++dig);
                    if(*dig==gSet.files[0]){
                        memcpy(flnmbuf,p,(idx)(last_-p));
                        strcpy(flnmbuf+(idx)(last_-p),dig);
                    }
                }
            }

            clsFile * f=0;
            sVec < clsPeak  >  * cpList=0;
            if((f=flnms.get(flnmbuf))!=0) {
                cpList=cpListList.ptr(f->cplistIdx);
                ++f->Rpt;
            }else {
                f=flnms.set(flnmbuf);
                f->cplistIdx=cpListList.dim();
                f->Rpt=1;
                cpList=cpListList.add(1);
            }

            real intst[19];idx ie,ii;
            real massMax=0,ampMax=0, diff=0;

            for(const char * s=fl.ptr(); s && s<fl.last(); ){
                idx rest=(idx)(fl.last()-s);
                fomt.cut(0);sString::copyUntil(&fomt,s,rest,sString_symbolsEndline);
                namshft[0]=0;
                if( sscanf(fomt.ptr(),"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s %s",&dummy, &intst[0], &intst[1], &intst[2], &intst[3], &intst[4], &intst[5], &intst[6], &intst[7], &intst[8], &intst[9], &intst[10], &intst[11], &intst[12], &intst[13], &intst[14], &intst[15], &intst[16], &intst[17], &intst[18], name,namshft) >= 14) {
                    cp.rmp.rMass=intst[4];cp.rmp.rPrfx=0;
                    if(*path)strncpy((char*)(&cp.rmp.rPrfx),prfx,sizeof(real)-1);

                    gLog->printf("  Trying peak %s with mass %s%lf  ... ",   name, (char *)(&cp.rmp.rPrfx),cp.rmp.rMass);
                    if(namshft[0] ){
                        if(namshft[0]=='+' && namshft[1]=='-')strcat(name,namshft+1);
                        else strcat(name,namshft);
                    }

                    idx iFound,iClosest=0;
                    double diffClosest=1.e+13;
                    rMassPrf * rmp=0;
                    for(iFound=0; iFound<peaks.dim() ; ++iFound) {
                        rmp=(rMassPrf*)peaks.id(iFound);
                        if( cp.rmp.rPrfx!=rmp->rPrfx)
                            continue;
                        diff=sAbs(cp.rmp.rMass-rmp->rMass);
                        if(diff<diffClosest){iClosest=iFound;diffClosest=diff;}
                        if(diff<=gSet.collect.wobble) break;
                    }

                    real massToCompare= (iFound>=peaks.dim() ) ? cp.rmp.rMass :  rmp->rMass;

                    for(ie=0; ie<excl.dim() ; ++ie) {
                        diff=sAbs(massToCompare-excl[ie]);
                        if(diff<=gSet.collect.wobbleExcl) break;
                    }
                    for(ii=0; ii<incl.dim() ; ++ii) {
                        diff=sAbs(massToCompare-incl[ii]);
                        if(diff<=gSet.collect.wobbleExcl) break;
                    }

                    if( ie==excl.dim()  ) {
                        if( incl.dim()==0 || ii!=incl.dim()  ) {
                            cp.intensity=intst[gSet.collect.column-1];

                            if(iFound>=peaks.dim()){
                                *peaks.set((const void *)&cp.rmp, sizeof(cp.rmp), &cp.iNum)=peaks.mex()->add((const char *)name,sLen(name)+1);
                                gLog->printf("added as a new peak number %d ",   cp.iNum+1 );
                                rmp=(rMassPrf *)peaks.id(iClosest);
                                if(ifile>0)gLog->printf(" - the closest was peak #%d with mass %s%lf diff=%lf", iClosest+1, (char*)(&(rmp->rPrfx)),rmp->rMass,fabs(diffClosest));
                                gLog->printf("\n");
                            }
                            else {
                                cp.iNum=iFound;
                                if(diff==0) gLog->printf("exactly "); else gLog->printf("with %lf wobble ",diff);
                                rmp=(rMassPrf *)peaks.id(cp.iNum);
                                gLog->printf("matched with previously detected peak[%d]=%s with mass %s%lf\n",   cp.iNum+1, peaks.mex()->ptr(peaks[cp.iNum]), (char*)(&(rmp->rPrfx)),rmp->rMass);
                            }
                            if(ampMax<cp.intensity){ampMax=cp.intensity;massMax=cp.rmp.rMass;}
                            if(gSet.collect.join) {
                                idx rpt;
                                for(rpt=0; rpt<cpList->dim() && (cpList->ptr(rpt)->rmp.rMass!=cp.rmp.rMass || cpList->ptr(rpt)->rmp.rPrfx!=cp.rmp.rPrfx); ++rpt);
                                if( rpt<cpList->dim() ) {
                                    cpList->ptr(rpt)->intensity+=cp.intensity;
                                }
                                else cpList->vadd(1,cp);
                            }
                            else cpList->vadd(1,cp);

                        }else {
                            gLog->printf("skipped since it is not in inclusion list wihtin specified wobble %ld\n ",   gSet.collect.wobble);
                        }
                    }else {
                        gLog->printf("skipped since it is in exclusion list wihtin specified wobble %lf from #%d %lf\n ",   gSet.collect.wobble, ie,excl[ie]);
                    }
                }
                s=sString::skipWords(s,rest, 1,sString_symbolsEndline);
            }
            gLog->printf("%d peaks detected in %s (maxes at %lf to %lf) \n",  cpList->dim(), p,massMax, ampMax);
        }
    }

    if(!flnms.dim()) {
        gLog->printf("ERROR: no appropriate peak files to collect in this directory.\n");
        return -1;
    }

    sDir::chDir(curd);
    sMatrix ori;
    sMatrix trn;

    ori.init(SRC_FILES_PATH""ACT_WORKDIR"/amat.bin");
    trn.init(SRC_FILES_PATH""ACT_WORKDIR"/atrn.bin");
    ori.resize(cpListList.dim(),peaks.dim());ori.set(0);
    trn.resize(peaks.dim(),cpListList.dim());trn.set(0);

    idx cntRep=flnms.ptr(0)->Rpt;
    gSet.allMax=0;
    for(idx ir=0; ir<ori.rows(); ++ir) {
        sVec < clsPeak>  * cpList=cpListList.ptr(ir);
        clsFile * f=flnms.ptr(ir);
        if(f->Rpt!=cntRep)
            gLog->printf("WARNING: different number %d of repeats for this sample compared to the first %d\n",  f->Rpt, cntRep);
        int nonzero=0;
        for( idx ic=0; ic<cpList->dim(); ++ic) {
            clsPeak * cp=cpList->ptr(ic);
            real val=cp->intensity*1000/f->Rpt;
            ori(ir,cp->iNum)=val;
            if(gSet.allMax<val)gSet.allMax=val;
            if(val>0)++nonzero;
        }
        if(nonzero==0)
            ++nonzero;
    }
    ori.copy(trn, true);

    if(gSet.collect.renormalize){
        for(idx ir=0; ir<ori.rows(); ++ir) {
            real sum=0;
            for(idx ic=0; ic<ori.cols(); ++ic) {
                sum+=ori(ir,ic);
            }
            if(!sum)continue;
            for(idx ic=0; ic<ori.cols(); ++ic) {
                ori(ir,ic)*=100./sum;
            }
        }
    }
    sFil out(SRC_FILES_PATH""ACT_WORKDIR"/amat.csv");out.cut(0);
    sFil ouT(SRC_FILES_PATH""ACT_WORKDIR"/atrn.csv");ouT.cut(0);

    outPar op(this);op.peaks=&peaks;op.flnms=&flnms;
    idx colstart=1,colend=0x7FFFFFFF;
    idx rowstart=1,rowend=0x7FFFFFFF;
    sscanf(gSet.collect.colout,"%lld-%lld",&colstart,&colend);
    sscanf(gSet.collect.rowout,"%lld-%lld",&rowstart,&rowend);
    ori.out(&out,(void*)&op, false, true, "%lf",(sMatrix::MatrixOutput)actMatOut, colstart-1, colend, rowstart-1, rowend);
    ori.out(&ouT,(void*)&op, true, true, "%lf",(sMatrix::MatrixOutput)actMatOut, colstart-1, colend, rowstart-1, rowend);

    sFil mpks(SRC_FILES_PATH""ACT_WORKDIR"/peaks.bin");mpks.cut(0);
    for (idx i=0; i<peaks.dim(); ++i) {
        rMassPrf * rmp=(rMassPrf *)peaks.id(i);
        const char *name=(const char *)peaks.mex()->ptr(peaks[i]);
        mpks.printf("%s_%s%lf",   name, (char *)(&rmp->rPrfx),rmp->rMass);mpks.add0();
    }

    sFil mfls(SRC_FILES_PATH""ACT_WORKDIR"/files.bin");mfls.cut(0);
    for (idx i=0; i<flnms.dim(); ++i) {
        const char *name=(const char  *)flnms.id(i);
        mfls.printf("%s", name);mfls.add0();
    }

    return peaks.dim();
}


idx sSpctr::filterAMat(idx iCat, sDic < sDic < sVec < idx > > > * catSet ,sStr * rexcl,sStr * rincl)
{
    gLog->printf("\n");
    gLog->printf("_/_/_/_/_/_/_/_/_/_/_/_/_/\n");
    gLog->printf("_/\n");
    gLog->printf("_/ Filtering activity matrix \n");
    gLog->printf("_/\n");
    gLog->printf("_/_/_/_/_/_/_/_/_/_/_/_/_/\n");
    gLog->printf("\n");

    sVec < real > alwaysDetect;
    if(gSet.collect.alwaysPeaks.length())
        sString::sscanfRVec(&alwaysDetect, gSet.collect.alwaysPeaks.ptr(), gSet.collect.alwaysPeaks.length());

    sDic<idx> peaks;
    peaks.init(SRC_FILES_PATH""ACT_WORKDIR"/peakset");

    sDic < clsFile > flnms;
    flnms.init(SRC_FILES_PATH""ACT_WORKDIR"/flnmset");

    if(!peaks.dim() || !flnms.dim() ){gLog->printf("Please, run the collection stage before filtering!\n"); return 0;}

    sMatrix ori; ori.init(SRC_FILES_PATH""ACT_WORKDIR"/amat.bin"); if(!ori.rows())return 0;

    sDic < sVec < idx > > * colset=0;
    sMatrix Stats, * stat=0;

    if(catSet && catSet->dim()!=0) {
        colset=&(*catSet)[iCat];
        Stats.resize(colset->dim()*2,ori.cols());

        for(idx il=0; il<colset->dim(); ++il) {
            sMatrix grpmat;ori.extractRowset(grpmat, (*colset)[il]);
            grpmat.computeRowStat(Stats.ptr(il,0),Stats.ptr(colset->dim()+il,0),0);
        }
        stat=&Stats;
    }else {
        gLog->printf("No categories specified! Will not use statistics for filtering.\n");
    }


    sFil flt(SRC_FILES_PATH""ACT_WORKDIR"/amat_filters.csv");flt.cut(0);
    real * da=ori.ptr(0,0); idx cols=ori.cols(), rows=ori.rows();
    flt.printf("peaks, formula,, frequency, non-zero, out-of, , all-min, all-average, all-max, all-sigma, , exist-min, exist-average, exist-max, exist-sigma, ,expression, intensity");
    if(colset){
        if(stat){
            flt.printf(",");
            for ( idx il=0; il<colset->dim() ; ++il) flt.printf(",SD grp_%d",il+1);
        }
        flt.printf(",");
        for ( idx il1=0; il1<colset->dim()-1; ++il1) {
            for ( idx il2=il1+1; il2<colset->dim(); ++il2) {
                flt.printf(",T %d<>%d",il1+1,il2+1);
            }
        }
    }
    flt.printf(",,decision\n");
    flt.printf("\n");

    sMatrix trn;ori.copy(trn,true);
    sStr excl, incl;
    sVec < idx> colsin;
    sVec < idx> colflt(SRC_FILES_PATH""ACT_WORKDIR"/fltr.bin");colflt.resize(cols-1);

    for( idx ic=0; ic<cols; ++ic) {

        idx cntnonzero=0,cntall=0;
        real rmax=-REAL_MAX, rmin=REAL_MAX, rave=0, zmax=-REAL_MAX, zmin=REAL_MAX, zave=0, rsigma=0, zsigma=0;
        for(idx ir=0;ir<rows; ++ir) {
            real val=da[ir*cols+ic];
            if(val!=0){
                ++cntnonzero;
                if(zmax<val)zmax=val;
                if(zmin>val)zmin=val;
                zave+=val;
            }
            ++cntall;
            if(rmax<val)rmax=val;
            if(rmin>val)rmin=val;
            rave+=val;
        }
        if(cntall)rave/=cntall;
        else {rmin=0;rmax=0;}
        if(cntnonzero)zave/=cntnonzero;
        else {zmin=0;zmax=0;}

        int recommended=0;
        for(idx ir=0;ir<rows; ++ir) {
            real val=da[ir*cols+ic], dif;
            if(val!=0){
                dif=val-zave;
                zsigma+=dif*dif;
            }
            dif=val-rave;
            rsigma+=dif*dif;
        }
        rsigma=cntall>1 ? sqrt(rsigma/(cntall-1)) : -1 ;
        zsigma=cntnonzero>1 ? sqrt(zsigma/(cntnonzero-1)) : -1 ;


        rMassPrf * rmp=(rMassPrf*)peaks.id(ic);
        flt.printf("%s%lf, %s ,, %.1lf, %d, %d, ",  (char*)(&(rmp->rPrfx)),rmp->rMass, peaks.mex()->ptr(peaks[ic]) , cntnonzero*100./cntall, cntnonzero, cntall);
        flt.printf(",");

        flt.printf("%lf, %lf, %lf, ",rmin, rave, rmax);
        if(rsigma!=-1)flt.printf("%lf, ",rsigma);else flt.printf("!!!, ");
        flt.printf(",");

        flt.printf("%lf, %lf, %lf, ",zmin, zave, zmax);
        if(zsigma!=-1)flt.printf("%lf, ",zsigma);else flt.printf("!!!, ");
        flt.printf(",");

        if(zsigma!=-1){
            if(cntnonzero*100./cntall>=gSet.filter.filterFrequency) recommended|=1;
            if(zmax>=gSet.filter.filterIntensity) recommended|=2;
        }

        idx  excluded=0;
        sVec<real> probvals, tvals;

        if(colset){
            if(stat) {
                for ( idx il=0; il<colset->dim() ; ++il) {
                    if( stat->val(colset->dim()+il,ic)==0) excluded|=(((idx)1)<<il);
                }
            }

            for ( idx il1=0,ik=0; il1<colset->dim()-1; ++il1) {
                for ( idx il2=il1+1; il2<colset->dim(); ++il2) {

                    sVec < sVec < idx > > pairset;pairset.add(2);
                    for(idx ii=0; ii<(*colset)[il1].dim(); ++ii)  pairset[0].vadd(1,(*colset)[il1][ii]);
                    for(idx ii=0; ii<(*colset)[il2].dim(); ++ii)  pairset[1].vadd(1,(*colset)[il2][ii]);

                    real tval, probval;
                    sStat::statTest(trn.ptr(ic,0), 1,trn.cols(), &pairset, &probval, &tval);
                    if(probval<gSet.filter.filterTProb)
                        excluded|=(((idx)1)<<(16+ik));
                    probvals.vadd(1,probval);
                    tvals.vadd(1,tval);
                }
            }
        }

        if(recommended&1)flt.printf("ok,");else flt.printf("-,");
        if(recommended&2)flt.printf("ok,");else flt.printf("-,");
        if(colset) {
            if(stat){
                for ( idx il=0; il<colset->dim() ; ++il) {
                    if(!(excluded&(((idx)1)<<il)))flt.printf(",var");
                    else flt.printf(",-");
                }
            }
            flt.printf(",");
            for ( idx il1=0,ik=0; il1<colset->dim()-1; ++il1) {
                for ( idx il2=il1+1; il2<colset->dim(); ++il2) {
                    if(!(excluded&(((idx)1)<<(16+ik))))flt.printf(",T%.1lf",100.*probvals[ik]);
                    else flt.printf(",%.1lf",100.*probvals[ik]);
                    ++ik;
                }
            }
        }

        if(gSet.filter.andor1==0 && recommended!=3 )recommended=0;
        if(gSet.filter.andor2==1 && recommended )excluded=0;
        if(excluded)recommended=0;


        idx fnd=0;
        for ( fnd=0; fnd<alwaysDetect.dim(); ++fnd) {
            real dif=(alwaysDetect)[fnd]-rmp->rMass; if(dif<0)dif=-dif;
            if(dif<gSet.collect.wobbleAlwaysDetect)
                break;
        }
        if(fnd!=alwaysDetect.dim())
            recommended=1;



        if(recommended){
            flt.printf(",,in");
            *colsin.add()=ic;
            colflt[ic]=1;
        }else {
            flt.printf(",,-");
            colflt[ic]=-1;
        }

        if(!recommended){
            excl.printf("    %lf ,,%s\n", rmp->rMass,(char*)(&(rmp->rPrfx)));
            gLog->printf("    %lf %s declined\n", rmp->rMass,(char*)(&(rmp->rPrfx)));
        }
        else {
            incl.printf("    %lf ,, %s\n", rmp->rMass,(char*)(&(rmp->rPrfx)));
            gLog->printf("    %lf %s accepted\n", rmp->rMass,(char*)(&(rmp->rPrfx)));
        }
        flt.printf("\n");
    }

    if(excl.length())
        flt.printf("\n\n[ExcludePeaks]\n%s\n\n",excl.ptr());
    if(incl.length())
        flt.printf("\n\n[IncludePeaks]\n%s\n\n",incl.ptr());

    sMatrix oflt(SRC_FILES_PATH""ACT_WORKDIR"/amatflt.bin");oflt.empty();
    ori.extractColset(oflt, colsin);

    sFil out(SRC_FILES_PATH""ACT_WORKDIR"/amatflt.csv");out.cut(0);
    sFil ouT(SRC_FILES_PATH""ACT_WORKDIR"/atrnflt.csv");ouT.cut(0);
    outPar op(this);op.peaks=&peaks;op.flnms=&flnms;op.colset=&colsin;
    oflt.out(&out,(void*)&op, false, true, "%lf",(sMatrix::MatrixOutput)actMatOut);
    oflt.out(&ouT,(void*)&op, true, true, "%lf",(sMatrix::MatrixOutput)actMatOut);

    sFil mpks(SRC_FILES_PATH""ACT_WORKDIR"/fltpeaks.bin");mpks.cut(0);
    for (idx i=0; i<colsin.dim(); ++i) {
        rMassPrf * rmp=(rMassPrf *)peaks.id(colsin[i]);
        const char *name=(const char *)peaks.mex()->ptr(peaks[colsin[i]]);
        mpks.printf("%s_%s%lf",  name,(char *)(&rmp->rPrfx),rmp->rMass);mpks.add0();
    }

    if(rexcl)rexcl->printf("%s",excl.ptr());
    if(rincl)rincl->printf("%s",incl.ptr());

    return peaks.dim();
}




idx sSpctr::rlda(idx iCat, sDic < sDic < sVec < idx > > > * catSet, sDic < sDic < sVec < idx > > > * checkSet )
{
    gLog->printf("\n");
    gLog->printf("_/_/_/_/_/_/_/_/_/_/_/_/_/\n");
    gLog->printf("_/\n");
    gLog->printf("_/ Regularized Linear Discriminant Analysis \n");
    gLog->printf("_/\n");
    gLog->printf("_/_/_/_/_/_/_/_/_/_/_/_/_/\n");
    gLog->printf("\n");

    sDic<idx> rids; rids.parseFile00(SRC_FILES_PATH""ACT_WORKDIR"/files.bin");
    sDic<idx> cids;

    sMatrix ori(SRC_FILES_PATH""ACT_WORKDIR"/amatflt.bin");
    if(!ori.rows()){
        ori.destroy();
        ori.init(SRC_FILES_PATH""ACT_WORKDIR"/amat.bin");
        cids.parseFile00(SRC_FILES_PATH""ACT_WORKDIR"/peaks.bin");
    }
    else {cids.parseFile00(SRC_FILES_PATH""ACT_WORKDIR"/fltpeaks.bin");}
    if(!ori.rows()){ gLog->printf("Run collection and/or filtering steps before trying to Cluster.\n");return -1;}

    sDic < sVec < idx > > & grpset=(*catSet)[iCat];

    sRlda rlda;
    rlda.compute(ori,grpset);

    sMatrix trsMat(SRC_FILES_PATH""ACT_WORKDIR"/ldamat.bin");
    rlda.remap(trsMat, ori);
    trsMat.out(SRC_FILES_PATH""ACT_WORKDIR"/ldamat.csv");


    sFile::remove(SRC_FILES_PATH""ACT_WORKDIR"/rlda.csv");sFil rldacsv(SRC_FILES_PATH""ACT_WORKDIR"/rlda.csv");

    idx randSeed=12345;
    sRlda rl;
    rl.bootstrap(ori, gSet.lda.maxIter, gSet.lda.bootSpace, 0.1, randSeed, (*catSet)[iCat], checkSet ? (*checkSet)[iCat] : (*catSet)[iCat], gLog, (sStr *)gLog, (sStr *)gLog,&rldacsv,&rids, &cids);



    return trsMat.rows();
}

idx sSpctr::computGeneralPCA(idx iCat, sDic < sDic < sVec < idx > > > * catSet )
{
    sDic<idx> cids;
    sMatrix ori(SRC_FILES_PATH""ACT_WORKDIR"/amatflt.bin");
    if(!ori.rows()){
        ori.destroy();
        ori.init(SRC_FILES_PATH"""/amat.bin");
        cids.parseFile00(SRC_FILES_PATH"""/peaks.bin");
    }
    else {cids.parseFile00(SRC_FILES_PATH""ACT_WORKDIR"/fltpeaks.bin");}
    if(!ori.rows()){ gLog->printf("Run collection and/or filtering steps before trying to Cluster.\n");return -1;}
    sDic < sVec < idx > > & grpset=(*catSet)[iCat];
    sDic<idx> rids; rids.parseFile00(SRC_FILES_PATH""ACT_WORKDIR"/files.bin");


    sFilePath nm;
    sMatrix::MatrixDicHeaders hdrs;

    sMatrix Stats;Stats.resize(2,ori.cols());
    real * stat=Stats.ptr(0,0);
    real * stdev=Stats.ptr(1,0);

    idx temp = grpset.dim();
    for ( idx il=0; il<temp; ++il) {
        sFil fp(nm.makeName("",SRC_FILES_PATH""ACT_WORKDIR"/grp_%d.csv",il+1));fp.cut(0);

        sMatrix grpmat;ori.extractRowset(grpmat, grpset[il]);
        grpmat.computeRowStat(stat,stdev);

        hdrs.rows=&rids;
        hdrs.cols=&cids;
        grpmat.out(&fp,&hdrs,false, true);
        fp.printf("average");for(idx ic=0; ic<grpmat.cols(); ++ic) fp.printf(",%lf",stat[ic]);fp.printf("\n");
        fp.printf("stddev");for(idx ic=0; ic<grpmat.cols(); ++ic) fp.printf(",%lf",stdev[ic]);fp.printf("\n");


        grpmat.shiftRows(Stats.ptr(il,0));
        sMatrix covar,evc; grpmat.covariance(covar);

        sVec <real> evl;evl.resize(covar.cols());
        covar.diagJacoby(evl,&evc);

        for(idx ie=0; ie<evl.dim(); ++ie) evl[ie]=sqrt(sAbs(evl[ie])/(evl.dim()-1));
        fp.printf("\n\nPeak contributions in order of importance\n");
        fp.printf("#PCA, StdDev, StdDev%%, Contributions (%%weight, peak, ... )\n");
        for(idx ic=0; ic<evc.cols(); ++ic) evc.outSingleEvecSrt(&fp, evl,ic, hdrs.cols);
        fp.printf("\n\nEigenvalues and eigenvectors of covariance matrix\n");
        hdrs.rows=&cids;
        hdrs.cols=&cids;
        evc.out(&fp,&hdrs,false, true);
    }

    sFil fp(SRC_FILES_PATH""ACT_WORKDIR"/grp_all.csv");fp.cut(0);

    hdrs.rows=&rids;
    hdrs.cols=&cids;
    ori.out(&fp,&hdrs,false,true);
    ori.computeRowStat(stat,stdev);
    fp.printf("average");for(idx ic=0; ic<Stats.cols(); ++ic) fp.printf(",%lf",stat[ic]);fp.printf("\n");
    fp.printf("stddev");for(idx ic=0; ic<Stats.cols(); ++ic) fp.printf(",%lf",stdev[ic]);fp.printf("\n");


    sMatrix mat,evecs; ori.copy(mat);
    sVec <real > evals;
    mat.pca(evals,evecs);

    for(idx ie=0; ie<evals.dim(); ++ie) evals[ie]=sqrt(sAbs(evals[ie])/(evals.dim()-1));
    fp.printf("#PCA, StdDev, StdDev%%, Contributions (%%weight, peak, ... )\n");
    for(idx ic=0; ic<evecs.cols(); ++ic) {evecs.outSingleEvecSrt(&fp, evals,ic,hdrs.cols);}

    fp.printf("\n\nEigenvalues and eigenvectors of covariance matrix\n");
    hdrs.rows=&cids;
    hdrs.cols=&cids;
    evecs.out(&fp,&hdrs,false, true);

    fp.printf("\n\nMatrix after rotation to PCA directions \n");
    sMatrix trs;trs.resize(ori.rows(),ori.cols());
    sAlgebra::matrix::pcaReMap(trs.ptr(0,0),ori.ptr(0,0), ori.cols(), ori.rows(), evecs.ptr(0,0));

    hdrs.rows=&rids;
    hdrs.cols=&cids;
    trs.out(&fp,&hdrs, false, true);
return 1;
}

