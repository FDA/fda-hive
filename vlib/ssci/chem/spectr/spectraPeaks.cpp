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
#include <ssci/math.hpp>
#include <ssci/chem.hpp>
#include <ssci/chem/spectr/spectraFile.hpp>
#include <ssci/chem/spectr/spectraPeaks.hpp>

idx spectraPeaks::prepareIsopeakBins(idx imol, real * bins, idx cnt, real stp, real minx, real maxx, idx & leftbin, idx & rightbin, idx & topBin, idx & firstMaxBin)
{
        sSpectrMS::isoDistribution * isomol=IsoD.isoDistr(imol);

        idx isocnt=isomol->spctr.dim();
        if(isomol->spctr[0].rMass+sSpectrMS::isoDistribution::absShiftAll>=maxx || isomol->spctr[isocnt-1].rMass+sSpectrMS::isoDistribution::absShiftAll<=minx)return 0;

        memset(bins,0,cnt*sizeof(real));
        leftbin=0x7FFFFFFF, rightbin=0;
        for(idx is=0; is<isocnt; ++is) {
                idx ibin=(idx)((isomol->spctr[is].rMass+sSpectrMS::isoDistribution::absShiftAll-(minx))/stp);
                if(ibin>=cnt || ibin<0) continue;
                bins[ibin]+=isomol->spctr[is].abundance;
                if(ibin<leftbin)leftbin=ibin;
                if(ibin>rightbin)rightbin=ibin;
        }

        sVec <idx> isoi;sVec <real>isox; sVec <real> isoy;
        topBin=leftbin;
        firstMaxBin=-1;
        for( idx ib=leftbin; ib<=rightbin; ++ib ) {
                if(ib>0 && bins[ib]<=bins[ib-1])continue;
                if(ib<cnt-1 && bins[ib]<bins[ib+1]) continue;
                if(ib>0){bins[ib]+=bins[ib-1];bins[ib-1]=0;}
                if(ib<cnt-1){bins[ib]+=bins[ib+1];bins[ib+1]=0;}
                if(bins[ib]>=bins[topBin])topBin=ib;
                if(firstMaxBin==-1)firstMaxBin=ib;
                *isoi.add(1)=ib;
                *isox.add(1)=minx+ib*stp;
                *isoy.add(1)=bins[ib];
        }

        {
            sVec <real> rval,lval;
            for(idx is=0; is<isoi.dim(); ++is) {
                    rval.add(1);
                    real dist= (is+1<isoi.dim()) ? isox[is+1]-isox[is] : 1 ;
                    real lenToRight= dist*peaks.widerRightPeak;
                    real ACoef=-log(0.001)/(lenToRight*lenToRight);
                    for(idx ib=1; ib*stp<lenToRight; ++ib ) {
                            if(isoi[is]+ib >=cnt )break;
                            real dlt=(ib-1)*stp;
                            rval[is]=exp(-ACoef*dlt*dlt) * isoy[is];
                            bins[isoi[is]+ib]+=rval[is];
                            if(isoi[is]+ib>rightbin)rightbin=isoi[is]+ib;
                    }
            }
            for(idx is=0; is<isoi.dim(); ++is) {
                    lval.add(1);
                    real dist= (is==0) ? 1: isox[is]-isox[is-1] ;
                    idx ib=1;
                    real lenToLeft= dist*peaks.widerLeftPeak*0.5;
                    real ACoef=-log(0.01)/(lenToLeft*lenToLeft);
                    for(ib=1; ib*stp<lenToLeft; ++ib ) {
                            if(isoi[is]-ib < 0 )break;
                            real dlt=(ib-1)*stp;
                            lval[is]=exp(-ACoef*dlt*dlt) * isoy[is];
                            if(is>0 && lval[is]<rval[is-1])lval[is]=rval[is-1];
                            bins[isoi[is]-ib]+=lval[is];
                            if(isoi[is]-ib<leftbin)leftbin=isoi[is]-ib;
                    }
            }
        }

        return 1;
}

void spectraPeaks::peakKnownOut(const char * nam, sVec < knownPeak > & kpl, const char * flnm, real smin, real * d, real * r, real * s, real * p , real * u, idx cnt, idx istart, idx * sortord, real allStp)
{
        sStr pksnam("%s-%slist.csv",  nam, flnm);
        sFile::remove(pksnam.ptr());
        sFil pea(pksnam.ptr());
        sVec < knownPeak > majors;

        if(istart==0)pea.printf("daltons,intensity, coef, content, exactmass, realmass, name\r\n");
        for(idx i=istart; i<kpl.dim(); ++i) {
            knownPeak * kp=sortord ? kpl(sortord[i]) : kpl(i);

            idx iMaj;
            knownPeak * km=0;
            for(iMaj=0; iMaj<majors.dim(); ++iMaj) {
                km=majors.ptr(iMaj);
                if(kp->rmass==km->rmass) break;
            }
            if(iMaj<majors.dim()) {
                if(kp->iGrp==0) {
                    real itst=km->itgrl;
                    *km=*kp;
                    km->itgrl=itst;
                }
            }else {
                *majors.add(1)=(*kp);
            }


            pea.printf("%lf,%lf,%lf,%lf,%.3lf", smin+allStp*kp->iMatch, s[kp->iMatch], kp->coef, kp->itgrl, kp->rmass);
            if(peaks.satellite[kp->iGrp].shift!=0)pea.printf("%+" DEC "", peaks.satellite[kp->iGrp].shift );
            pea.printf(",   %.3lf,%s\r\n" , kp->rmass + peaks.satellite[kp->iGrp].shift, (const char *)IsoD.isoDistr.id(kp->iMol));
        }

        sStr majnam("%s-%smajor.csv",nam,flnm);
        sFile::remove(majnam.ptr());
        sFil maj(majnam.ptr());

        if(istart==0)maj.printf("daltons,intensity,coef,content,content0, exactmass,M-42,M-28,M-14,M-0,M+16,M+30,M+44,I-42,I-28,I-14,I-0,I+16,I+30,I+44,name\r\n");

        for(idx i=0; i<majors.dim(); ++i) {
            knownPeak * kp=majors(i);
            for( idx ks=0; ks<sDim(kp->sats) ; ++ks) {kp->sats[ks]=0; kp->fint[ks]=0;}
            real tot=0;
            for(idx ii=istart; ii<kpl.dim(); ++ii) {
                knownPeak * km=kpl(ii);
                if(kp->rmass==km->rmass) {
                    kp->sats[km->iGrp]=km->itgrl;
                    kp->fint[km->iGrp]=km->firstIntensity;
                    tot+=km->itgrl;
                }
            }
            kp->itgrl=tot;
        }


        real totmajItgrl=0, totalFirstInt=0, totalItgrl0=0;
        for(idx i=0; i<majors.dim(); ++i) {
            knownPeak * kp=majors(i);
            totmajItgrl+=kp->itgrl;
            totalItgrl0+=kp->itgrl0;
            totalFirstInt+=kp->firstIntensity;
        }


        sVec < idx > majind;
        majind.resize(majors.dim());
        sSort::sort(majors.dim(), majors.ptr());
        real sclfactor=peaks.scaleOut==false  ? totmajItgrl : 1. ;
        real sclfactorFirst=peaks.scaleOut==false  ? totalFirstInt : 1. ;
        real sclfactor0=peaks.scaleOut==false  ? totalItgrl0 : 1. ;

        for(idx i=0; i<majors.dim(); ++i) {
            knownPeak * kp=majors(i);
            if(kp->sats[0]==0 && kp->iGrp!=0){
                continue;
            }

            maj.printf("%lf,%lf,%lf,%lf,%lf,%.3lf", smin+allStp*kp->iMatch, kp->firstIntensity/sclfactorFirst, kp->coef, kp->itgrl/sclfactor, kp->itgrl0/sclfactor0, kp->rmass);
            maj.printf(",%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf",kp->sats[5]/sclfactor,kp->sats[3]/sclfactor,kp->sats[1]/sclfactor,kp->sats[0]/sclfactor,kp->sats[2]/sclfactor,kp->sats[4]/sclfactor,kp->sats[6]/sclfactor);
            maj.printf(",%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf",kp->fint[5],kp->fint[3],kp->fint[1],kp->fint[0],kp->fint[2],kp->fint[4],kp->fint[6]);
            maj.printf(",%s\r\n" , (const char *)IsoD.isoDistr.id(kp->iMol));
        }

    sStr txtnam("%ss.csv",nam);
    if(peaks.outputPeakfile) {

        sFile::remove(txtnam);
        sFil txt(txtnam);
        txt.printf("mass,normalized,cleanall,cleanknown,knownpeaks,unknownpeaks\n");
        for (idx i=0; i<cnt; ++i) {
            txt.printf("%lf,%lf,%lf,%lf,%lf,%lf\r\n", smin+allStp*i, s[i],d[i],r[i],p[i],u[i]);
        }
        txt.printf("\n");
    }else{
    }

}


idx spectraPeaks::peakeDetectSingle(sVec < knownPeak > & kpl, idx imol, idx cntBin, real smin, real smax,real * d, real * bins, real * p, idx grpStart, idx cntGrps, real threshold, bool takeorphans, knownPeak * kParent, int runnumber, sVec < real> * alwaysDetect, real allStp)
{
        idx gMaxBin, gFirstNonZero=0,gLastNonZero=0,gFirsMaxBin;
        idx ik;

        idx cntDetected=0;

        sSpectrMS::isoDistribution * isomol=IsoD.isoDistr(imol);

        prepareIsopeakBins(imol, bins, cntBin, allStp, smin, smax, gFirstNonZero, gLastNonZero, gMaxBin,gFirsMaxBin);
        if(gFirstNonZero>=gLastNonZero) {
            return 0;
        }

        for( idx iAss=grpStart; peaks.satellite[iAss].shift!=-1 && iAss<cntGrps; ++iAss ){
            if(cntGrps>grpStart+1){
            }
            if(iAss!=0) {
            }
            else {
            }
            real secondaryfarction=0;
            if(iAss!=0)secondaryfarction=peaks.satellite[iAss].intensity;

            knownPeak * kp=0;
            if(kParent)
                kp=kParent;
            else {
                if(iAss!=0 && secondaryfarction!=0) {
                    idx iParent;
                    for(iParent=0; iParent<kpl.dim(); ++iParent) {
                        kp=kpl.ptr(iParent);
                        if(kp->iMol==imol && kp->iGrp==0) break;
                    }
                    if( (takeorphans==false) && iParent>=kpl.dim()) {
                        continue;
                    }
                }
            }

            real rSteps=1./allStp;
            idx dSteps=(idx)rSteps;

            idx grpShift=peaks.satellite[iAss].shift*dSteps;

            if(gLastNonZero+grpShift<0 || gFirstNonZero+grpShift>cntBin-1) continue;

            idx iMaxRegion=-1;real maxRegion=0;
            for( ik=gMaxBin+grpShift-dSteps/2, iMaxRegion=ik; ik<=gMaxBin+grpShift+dSteps/2 && ik<cntBin; ++ik) {
                    if(ik<0 || ik>cntBin-1)continue;
                    if(d[ik]>maxRegion) {iMaxRegion=ik;maxRegion=d[iMaxRegion];}
            }
            if(iMaxRegion<0 || iMaxRegion>=cntBin)continue;

            idx iShift=iMaxRegion-gMaxBin;

            real itgrl=0;
            for( ik=gFirstNonZero; ik<=gLastNonZero; ++ik) {
                if(ik+iShift<0 || ik+iShift>=cntBin) continue;
                itgrl+=d[ik+iShift];
            }

            real coef=d[iMaxRegion]/bins[gMaxBin];
            idx shiftFirstMax=gMaxBin-gFirsMaxBin;
            idx matchPos=iMaxRegion;
            real matchPosY=d[iMaxRegion];
            if(shiftFirstMax>0 && iMaxRegion>shiftFirstMax ){
                real coefF=d[iMaxRegion-shiftFirstMax]/bins[gMaxBin-shiftFirstMax];
                if(runnumber>1 || coefF>coef ){
                    coef=coefF;
                    matchPos=iMaxRegion-shiftFirstMax;
                }
            }
            real maxAlt=d[iMaxRegion];
            real firstIntensity=(iMaxRegion-shiftFirstMax>0) ? d[iMaxRegion-shiftFirstMax] : maxAlt;
            if(coef==0) {
                break;
            }
            if(kp && secondaryfarction!=0 && coef>kp->maxAmplitude*secondaryfarction/bins[gMaxBin])
                coef=kp->maxAmplitude*secondaryfarction/bins[gMaxBin];

            real itgrlSubs=0;
            for( ik=gFirstNonZero; ik<=gLastNonZero; ++ik) {
                if(ik+iShift < 0 || ik+iShift>=cntBin)continue;
                real toSubs=bins[ik]*coef;
                if(toSubs>d[ik+iShift]) toSubs=d[ik+iShift];
                d[ik+iShift]-=toSubs;
                if(ik+iShift==matchPos)matchPosY=toSubs;
                p[ik+iShift]+=toSubs;
                itgrlSubs+=allStp*toSubs;
            }

            real rmass=isomol->getMass();


            if(itgrl<threshold) {
                idx fnd=0;
                for ( fnd=0; fnd<alwaysDetect->dim(); ++fnd) {
                    real dif=(*alwaysDetect)[fnd]-rmass; if(dif<0)dif=-dif;
                    if(dif<collect.wobbleAlwaysDetect)
                        break;
                }
                if(fnd==alwaysDetect->dim())
                    break;
            }


            idx iSelf;
            for(iSelf=0; iSelf<kpl.dim(); ++iSelf) {
                kp=kpl.ptr(iSelf);
                if(kp->rmass==rmass && kp->iGrp==iAss) break;
            }
            if(iSelf<kpl.dim()) {
                kp->itgrl+=itgrlSubs;
                continue;
            }
            knownPeak * kk=kpl.add(1);
            *kk=knownPeak(imol,iAss,iMaxRegion, coef, itgrlSubs,rmass,maxAlt,firstIntensity);
            kk->iMolOri=isomol->shiftedFrom==sNotIdx ? imol : isomol->shiftedFrom;
            kk->shiftOri=isomol->frmlShift;
            kk->matchPosX=smin+matchPos*allStp;
            kk->matchPosY=matchPosY;

            ++cntDetected;

        }
        return cntDetected;
}

spectraPeaks::satellite_peaks * spectraPeaks::peakGroup = 0;

idx spectraPeaks::peaksDetect(const char * nam, const char * srcsfx,sVec < knownPeak >  * kpl,sVec < knownPeak >  * kpl2,real allStp)
{


    sVec < real > alwaysDetect;
    if(collect.alwaysPeaks.length())
        sString::sscanfRVec(&alwaysDetect, collect.alwaysPeaks.ptr(), collect.alwaysPeaks.length());

    sSpectrMS::isoDistribution::absShiftAll=peaks.shiftIsoPeaks;

    idx originalIsoCount=IsoD.isoDistr.dim();

    sStr nm("%s.x",nam);

    sVec <real> xx(nm.ptr());
    idx cnt=xx.dim(); if(!cnt)return -1;

    nm.printf(0,"%s.%s",nam, srcsfx);
    sVec <real> src(nm.ptr());



    peaks.satellite.resize(peaks.minMax_satellite.dim());
    for(int i=0; i< peaks.minMax_satellite.dim(); ++i) {
        peaks.satellite[i].intensity = peaks.minMax_satellite[i].max_intensity;
        peaks.satellite[i].shift = peaks.minMax_satellite[i].shift;

    }
    peakGroup = peaks.satellite.ptr(0);


    real totItgrl=0;
    for ( idx jit=0;jit<cnt; ++jit){
        totItgrl+=src[jit];
    }

    idx cntDetected=0, imol=0;
    real firstMax=0;

    sVec <real > Bins; real * bins=Bins.add(cnt+10)+1;
    sVec < idx > ind;

    {
        sStr * flnmKP = m_tmpFiles.add(1);
        flnmKP->printf(0,"%s.1knownpeaks",nam);
        sVec <real> knownpeaks1(flnmKP->ptr());
        knownpeaks1.cut(0);knownpeaks1.add(cnt);knownpeaks1.set();

        sStr * flnmKPO = m_tmpFiles.add(1);
        flnmKPO->printf(0,"%s.1knownpeaksoff",nam);
        sVec <real> knownpeaksoff1(flnmKPO->ptr());
        knownpeaksoff1.cut(0);knownpeaksoff1.add(cnt);

        {

            memcpy(knownpeaksoff1.ptr(),src.ptr(),cnt*sizeof(real));

            if (peaks.satellite.dim()) {
                for( idx iAss=0; peaks.satellite[iAss].shift !=-1 && iAss<peaks.satellite.dim(); ++iAss ){
                    for( imol=0; imol<IsoD.isoDistr.dim() &&  imol<peaks.maxPeaks; ++imol) {

                        cntDetected+=peakeDetectSingle(*kpl,imol, cnt, xx[0], xx[cnt-1],knownpeaksoff1, bins, knownpeaks1, iAss,iAss+1, peaks.threshold*0.01*totItgrl, false, 0, 1 , &alwaysDetect, allStp);
                    }
                }
            }
        }

        sVec <real> unknownpeaks1(nm.printf(0,"%s.%s",nam,"1unknownpeaks")); unknownpeaks1.cut(0);unknownpeaks1.add(cnt);unknownpeaks1.set(0);
        sVec <real> allpeaksoff1(nm.printf(0,"%s.%s",nam, "1allpeaksoff")); allpeaksoff1.cut(0);allpeaksoff1.add(cnt);
        memcpy(allpeaksoff1,knownpeaksoff1,sizeof(real)*cnt);


        if(peaks.dogenerate) {
            for( ; imol<peaks.maxPeaks; ++imol)  {
                idx iTop, ik ;

                for ( iTop=0, ik=1; ik<cnt; ++ik ) {if(allpeaksoff1[iTop]<allpeaksoff1[ik] ) iTop=ik;}
                if(!firstMax) firstMax=allpeaksoff1[iTop];
                if(allpeaksoff1[iTop]<firstMax*peaks.threshold*0.01) break;

                IsoD.makeCloseIsoDistribution(xx[0]+allStp*iTop,0,originalIsoCount);

                idx newDetected=peakeDetectSingle(*kpl,imol, cnt,xx[0],xx[cnt-1],allpeaksoff1, bins, unknownpeaks1, 0,peaks.satellite.dim(),peaks.threshold*0.01*totItgrl, false, 0, 1  , &alwaysDetect,  allStp);
                if(!newDetected)break;
                cntDetected+=newDetected;
            }
        }


        ind.add(kpl->dim());
        sSort::sort(kpl->dim(), kpl->ptr(),ind.ptr());
        peakKnownOut(nam,*kpl,"peak",xx[0],allpeaksoff1.ptr(), knownpeaksoff1.ptr(), src.ptr(), knownpeaks1.ptr(), unknownpeaks1.ptr(), cnt,0,ind, allStp);

    }

    real satelliteControl=IsoD.isoDistr[peaks.satelliteControlI-1].getMass();
    idx imax;
    for( imax=0; imax<kpl->dim() && fabs(kpl->ptr(imax)->rmass-satelliteControl)>0.5; ++imax) ;
    if( imax<kpl->dim() && kpl->ptr(imax)->rmass==satelliteControl) {
        knownPeak * kp=kpl->ptr(imax);
        for( idx ia=0; ia<kpl->dim() ; ++ia) {
            knownPeak * ka=kpl->ptr(ind[ia]);
            if( ka->iGrp==0 || ka->rmass!=kp->rmass)continue;

            peaks.satellite[ka->iGrp].intensity = ka->maxAmplitude/kp->maxAmplitude;
                if(peaks.satellite[ka->iGrp].intensity<peaks.minMax_satellite[ka->iGrp].min_intensity){
                    peaks.satellite[ka->iGrp].intensity=peaks.minMax_satellite[ka->iGrp].min_intensity;
                }
                if(peaks.satellite[ka->iGrp].intensity>peaks.minMax_satellite[ka->iGrp].max_intensity){
                    peaks.satellite[ka->iGrp].intensity=peaks.minMax_satellite[ka->iGrp].max_intensity;
                }
        }
        }else {
    }




    {
        sVec <real> peaks2(nm.printf(0,"%s.%s",nam, "2peaks")); peaks2.cut(0);peaks2.add(cnt);peaks2.set();
        sVec <real> peaksoff2(nm.printf(0,"%s.%s", nam,"2peaksoff")); peaksoff2.cut(0);peaksoff2.add(cnt);

        memcpy(peaksoff2.ptr(),src.ptr(),cnt*sizeof(real));


        cntDetected=0;
        idx prvMol=-1, prvGrp=-1;
        for( imol=0; imol<kpl->dim() ; ++imol) {
            knownPeak * kp=kpl->ptr(ind[imol]);
            if(prvMol==kp->iMol && prvGrp==kp->iGrp) continue;

            knownPeak * kParent=0;
            for( idx ip=0; ip<kpl->dim() ; ++ip) {if( kpl->ptr(ip)->rmass==kp->rmass && kpl->ptr(ip)->iGrp==0 ){kParent=kpl->ptr(ip);break;}}
            cntDetected+=peakeDetectSingle(*kpl2,kp->iMol, cnt,xx[0],xx[cnt-1],peaksoff2,bins, peaks2, kp->iGrp,kp->iGrp+1, peaks.threshold*0.01*totItgrl,true, kParent, 2 , &alwaysDetect, allStp);

            prvMol=kp->iMol;
            prvGrp=kp->iGrp;
        }


        sVec < idx > ind2; ind2.add(kpl2->dim());
        sSort::sort(kpl2->dim(), kpl2->ptr(),ind2.ptr());
        peakKnownOut(nam,*kpl2,"peak2",xx[0],peaksoff2.ptr(),peaksoff2.ptr(),src.ptr(),peaks2.ptr(), peaks2.ptr(), cnt,0,ind2, allStp);

    }
    IsoD.isoDistr.cut(originalIsoCount);


    return cntDetected;
}



idx spectraPeaks::peaksGenerateKnown(const char * path, real allStp, real smin, real smax, bool doredo )
{
    idx cntBin=(idx)((smax-smin)/allStp)+1;
    sVec <real > memBins; real * membins=memBins.add(cntBin);

    idx gFirstNonZero, gLastNonZero, gMaxBin,gFirsMaxBin;
    for( idx imol=0; imol<IsoD.isoDistr.dim() &&  imol<peaks.maxPeaks; ++imol) {
        sStr * isoFlnm=m_tmpFiles.add(1);
        isoFlnm->printf("%siso_%" DEC "",path, imol+1);


        sVec <real> Bins(isoFlnm->ptr());
        if(doredo==false  && Bins.dim()) continue;
        Bins.cut(0);

        prepareIsopeakBins(imol, membins, cntBin, allStp, 0, 10000, gFirstNonZero, gLastNonZero, gMaxBin,gFirsMaxBin);

        --gFirstNonZero;++gLastNonZero;
        idx cnt=gLastNonZero-gFirstNonZero+1;
        real * bins=Bins.add(cnt*2);
        for(idx ii=0; ii<cnt; ++ii) {
            bins[ii]=smin+(gFirstNonZero+ii)*allStp;
            bins[cnt+ii]=memBins[gFirstNonZero+ii];
        }

    }
    return IsoD.isoDistr.dim() ;
}
