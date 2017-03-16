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
        for(idx is=0; is<isocnt; ++is) { // first we bin the isospectrum into our bins
                idx ibin=(idx)((isomol->spctr[is].rMass+sSpectrMS::isoDistribution::absShiftAll-(minx))/stp);
                if(ibin>=cnt || ibin<0) continue;  // out of spectrum
                //aaa=bins[ibin];
                bins[ibin]+=isomol->spctr[is].abundance;
                if(ibin<leftbin)leftbin=ibin;
                if(ibin>rightbin)rightbin=ibin;
        }

        sVec <idx> isoi;sVec <real>isox; sVec <real> isoy; // then we prepare non-=zero x,y sets for splining
        topBin=leftbin;
        firstMaxBin=-1;
        for( idx ib=leftbin; ib<=rightbin; ++ib ) {
                if(ib>0 && bins[ib]<=bins[ib-1])continue;
                if(ib<cnt-1 && bins[ib]<bins[ib+1]) continue;
                if(ib>0){bins[ib]+=bins[ib-1];bins[ib-1]=0;}
                if(ib<cnt-1){bins[ib]+=bins[ib+1];bins[ib+1]=0;}
                //if(bins[ib]==0)continue;
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
                    real lenToRight= dist*peaks.widerRightPeak;//(0.7*dist) ;
                    real ACoef=-log(0.001)/(lenToRight*lenToRight);
                    for(idx ib=1; ib*stp<lenToRight; ++ib ) {
                            if(isoi[is]+ib >=cnt )break;
                            real dlt=(ib-1)*stp;
                            //rval[is]=exp(-(dlt*dlt)/( -0.64*gSet.peaks.widerPeaksRight/log(0.03)  )) * isoy[is];
                            rval[is]=exp(-ACoef*dlt*dlt) * isoy[is];
                            bins[isoi[is]+ib]+=rval[is];
                            if(isoi[is]+ib>rightbin)rightbin=isoi[is]+ib;
                    }
            }
            for(idx is=0; is<isoi.dim(); ++is) {
                    lval.add(1);
                    real dist= (is==0) ? 1: isox[is]-isox[is-1] ;
                    idx ib=1;
                    real lenToLeft= dist*peaks.widerLeftPeak*0.5;// left peaks are naturally narrower
                    real ACoef=-log(0.01)/(lenToLeft*lenToLeft);
                    for(ib=1; ib*stp<lenToLeft; ++ib ) {
                            if(isoi[is]-ib < 0 )break;
                            real dlt=(ib-1)*stp;
                            //lval[is]=exp(-(dlt*dlt)/( -0.09*gSet.peaks.widerPeaksLeft/log(0.005)  )) * isoy[is];
                            lval[is]=exp(-ACoef*dlt*dlt) * isoy[is];
                            if(is>0 && lval[is]<rval[is-1])lval[is]=rval[is-1];
                            bins[isoi[is]-ib]+=lval[is];
                            if(isoi[is]-ib<leftbin)leftbin=isoi[is]-ib;
                    }
                    /*
                    for(;is>0 && bins[isoi[is]-ib]==0;++ib){
                            if(isoi[is]-ib < 0 )break;
                            bins[isoi[is]-ib]+=lval[is];
                    }*/
            }
        }

        return 1;
}

void spectraPeaks::peakKnownOut(const char * nam, sVec < knownPeak > & kpl, const char * flnm, real smin, real * d, real * r, real * s, real * p , real * u, idx cnt, idx istart, idx * sortord, real allStp)
{
        sStr pksnam("%s-%slist.csv",  nam, flnm);
        sFile::remove(pksnam.ptr());
        sFil pea(pksnam.ptr());
        //printf("molecule,mass,intensity,name\r\n");
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
                //km->itgrl+=kp->itgrl; // if the parent was already generated - we shouldn't detect it second time
                if(kp->iGrp==0) {
                    real itst=km->itgrl;
                    *km=*kp;
                    km->itgrl=itst;
                }
            }else {
                *majors.add(1)=(*kp);
            }


            //pea.printf("%lf,   %lf,   %lf,   %lf,  %.3lf", smin+allStp*kp->iMatch, s[kp->iMatch], kp->coef, kp->itgrl, kp->rmass);
            pea.printf("%lf,%lf,%lf,%lf,%.3lf", smin+allStp*kp->iMatch, s[kp->iMatch], kp->coef, kp->itgrl, kp->rmass);
            if(peaks.satellite[kp->iGrp].shift!=0)pea.printf("%+" DEC "", peaks.satellite[kp->iGrp].shift );
            //pea.printf(",   %.3lf,   %s\r\n" , kp->rmass + peakGroup[kp->iGrp], IsoD.isoDistr.id(kp->iMol));
            pea.printf(",   %.3lf,%s\r\n" , kp->rmass + peaks.satellite[kp->iGrp].shift, (const char *)IsoD.isoDistr.id(kp->iMol));
        }

        sStr majnam("%s-%smajor.csv",nam,flnm);
        sFile::remove(majnam.ptr());
        sFil maj(majnam.ptr());

        //if(istart==0)maj.printf("daltons,      intensity,       coef,    content,   content0, exactmass,     M-42,     M-28,     M-14,     M-0,     M+16,     M+30,     M+44,     I-42,     I-28,     I-14,     I-0,     I+16,     I+30,     I+44,               name\r\n");
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
                    //kp->itgrl+=km->itgrl;
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
            //knownPeak * kp=majors(majind[i]);
            if(kp->sats[0]==0 && kp->iGrp!=0){
                //gLog->printf("The intensity %lf of the peak %lf +0 has been absorbed by other peaks during the second run\n",kp->itgrl,kp->rmass); // , peakGroup[kp->iGrp]
                continue; // after second run some of the major peak intensities may diisappear
            }

            //maj.printf("%lf,   %lf,   %lf,   %lf,   %lf,  %.3lf", smin+allStp*kp->iMatch, kp->firstIntensity/sclfactorFirst, kp->coef, kp->itgrl/sclfactor, kp->itgrl0/sclfactor0, kp->rmass);
            maj.printf("%lf,%lf,%lf,%lf,%lf,%.3lf", smin+allStp*kp->iMatch, kp->firstIntensity/sclfactorFirst, kp->coef, kp->itgrl/sclfactor, kp->itgrl0/sclfactor0, kp->rmass);
            //maj.printf(",  %.5lf,  %.5lf,  %.5lf,  %.5lf,  %.5lf,  %.5lf,  %.5lf",kp->sats[5]/sclfactor,kp->sats[3]/sclfactor,kp->sats[1]/sclfactor,kp->sats[0]/sclfactor,kp->sats[2]/sclfactor,kp->sats[4]/sclfactor,kp->sats[6]/sclfactor);
            maj.printf(",%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf",kp->sats[5]/sclfactor,kp->sats[3]/sclfactor,kp->sats[1]/sclfactor,kp->sats[0]/sclfactor,kp->sats[2]/sclfactor,kp->sats[4]/sclfactor,kp->sats[6]/sclfactor);
            //maj.printf(",  %.5lf,  %.5lf,  %.5lf,  %.5lf,  %.5lf,  %.5lf,  %.5lf",kp->fint[5],kp->fint[3],kp->fint[1],kp->fint[0],kp->fint[2],kp->fint[4],kp->fint[6]);
            maj.printf(",%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf,%.5lf",kp->fint[5],kp->fint[3],kp->fint[1],kp->fint[0],kp->fint[2],kp->fint[4],kp->fint[6]);
            //maj.printf(",   %s\r\n" , IsoD.isoDistr.id(kp->iMol));
            maj.printf(",%s\r\n" , (const char *)IsoD.isoDistr.id(kp->iMol));
        }

    sStr txtnam("%ss.csv",nam);
    if(peaks.outputPeakfile) {
        //gLog->printf("Outputing peaklist file for %s\n",txtnam.ptr());

        sFile::remove(txtnam);
        sFil txt(txtnam);
        txt.printf("mass,normalized,cleanall,cleanknown,knownpeaks,unknownpeaks\n");
        for (idx i=0; i<cnt; ++i) {
            //txt.printf("%lf,    %lf,    %lf,    %lf,   %lf,    %lf\r\n", smin+allStp*i, s[i],d[i],r[i],p[i],u[i]);
            txt.printf("%lf,%lf,%lf,%lf,%lf,%lf\r\n", smin+allStp*i, s[i],d[i],r[i],p[i],u[i]);
        }
        txt.printf("\n");
    }else{
        //gLog->printf("Skipping peaklist file output for %s. Not requested\n",txtnam.ptr());
    }

}


idx spectraPeaks::peakeDetectSingle(sVec < knownPeak > & kpl, idx imol, idx cntBin, real smin, real smax,real * d, real * bins, real * p, idx grpStart, idx cntGrps, real threshold, bool takeorphans, knownPeak * kParent, int runnumber, sVec < real> * alwaysDetect, real allStp)
{
        idx gMaxBin, gFirstNonZero=0,gLastNonZero=0,gFirsMaxBin;
        idx ik;

        idx cntDetected=0;

        sSpectrMS::isoDistribution * isomol=IsoD.isoDistr(imol);
        //gLog->printf("  searching #%d(%s) at %lf ",imol+1,IsoD.isoDistr.id(imol),isomol->getMass());

        prepareIsopeakBins(imol, bins, cntBin, allStp, smin, smax, gFirstNonZero, gLastNonZero, gMaxBin,gFirsMaxBin);
        if(gFirstNonZero>=gLastNonZero) {
            //gLog->printf("... peak is beyond the spectrum boundaries, skipping\n");
            return 0; // peak is off spectra
        }

        for( idx iAss=grpStart; peaks.satellite[iAss].shift!=-1 && iAss<cntGrps; ++iAss ){
            if(cntGrps>grpStart+1){
                //gLog->printf("\t");
            }
            if(iAss!=0) {
                //gLog->printf("%+d",peakGroup[iAss]);
            }
            else {
                //gLog->printf(" major peak");
            }
            real secondaryfarction=0;
            if(iAss!=0)secondaryfarction=peaks.satellite[iAss].intensity;

            knownPeak * kp=0;
            if(kParent)
                kp=kParent; // if outside parent is defined ... use it otherwise find it
            else {
                if(iAss!=0 && secondaryfarction!=0) {  // for secondary peaks - find the parent
                    idx iParent;
                    for(iParent=0; iParent<kpl.dim(); ++iParent) {
                        kp=kpl.ptr(iParent);
                        if(kp->iMol==imol && kp->iGrp==0) break;
                    }
                    if( (takeorphans==false) && iParent>=kpl.dim()) {
                        //gLog->printf(".... abandoned because +0 has not been found\n");
                        continue; // if the parent peak was not found - we shouldn't find the derivative peak
                    }
                }
            }

            real rSteps=1./allStp;
            idx dSteps=(idx)rSteps;//(1./gSet.allStp);

            idx grpShift=peaks.satellite[iAss].shift*dSteps;

            if(gLastNonZero+grpShift<0 || gFirstNonZero+grpShift>cntBin-1) continue;

            // find maximum in this region as well
            idx iMaxRegion=-1;real maxRegion=0;
            for( ik=gMaxBin+grpShift-dSteps/2, iMaxRegion=ik; ik<=gMaxBin+grpShift+dSteps/2 && ik<cntBin; ++ik) {
                    if(ik<0 || ik>cntBin-1)continue;
                    if(d[ik]>maxRegion) {iMaxRegion=ik;maxRegion=d[iMaxRegion];}
            }
            if(iMaxRegion<0 || iMaxRegion>=cntBin)continue;

            // now we estimate the apparent shift and make sure it is not more than allowed 1 dalton
            idx iShift=iMaxRegion-gMaxBin;

            // compute the total integral to see if there is anything to catch in this region
            real itgrl=0;
            for( ik=gFirstNonZero; ik<=gLastNonZero; ++ik) {
                if(ik+iShift<0 || ik+iShift>=cntBin) continue;
                itgrl+=d[ik+iShift];
            }

            // now we start substracting the molecule intensities
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
                //gLog->printf(" ... no spectral intensity in this region, skipping ...\n");
                break; // we intensionally leave this here since overlapping peaks interpretation depends on order
            }
            if(kp && secondaryfarction!=0 && coef>kp->maxAmplitude*secondaryfarction/bins[gMaxBin])
                coef=kp->maxAmplitude*secondaryfarction/bins[gMaxBin];

            real itgrlSubs=0;
            for( ik=gFirstNonZero; ik<=gLastNonZero; ++ik) {
                if(ik+iShift < 0 || ik+iShift>=cntBin)continue;
                real toSubs=bins[ik]*coef;//*(1+secondaryfarction);
                if(toSubs>d[ik+iShift]) toSubs=d[ik+iShift];
                d[ik+iShift]-=toSubs;                           //if(d[ik]<0)d[ik]=0;
                if(ik+iShift==matchPos)matchPosY=toSubs;
                p[ik+iShift]+=toSubs;
                itgrlSubs+=allStp*toSubs;
            }

            real rmass=isomol->getMass();///isomol->frml.mass(sChem::formula::eIsotopicAbundant, false)+isomol->frmlShift;


            if(itgrl<threshold) {  // too small a peak - we do not even consider this
               // gLog->printf(" ... too small intensity %lf, beyound treshold %lf, skipping ...\n",itgrl,threshold);
                idx fnd=0;
                for ( fnd=0; fnd<alwaysDetect->dim(); ++fnd) {
                    //DAVID edit unused variable m
                    //real m=(*alwaysDetect)[fnd];
                    real dif=(*alwaysDetect)[fnd]-rmass; if(dif<0)dif=-dif;
                    if(dif<collect.wobbleAlwaysDetect)
                        break; // match
                }
                if(fnd==alwaysDetect->dim()) // this is not one of always detect peaks
                    break; // we do not need to look further if we cannot find more major peak
            }


            // see if this was alredy in the list
            idx iSelf;
            for(iSelf=0; iSelf<kpl.dim(); ++iSelf) {
                kp=kpl.ptr(iSelf);
                if(kp->rmass==rmass && kp->iGrp==iAss) break;
            }
            if(iSelf<kpl.dim()) {
                kp->itgrl+=itgrlSubs; // if the prent was already generated - we shouldn't detect it second time
                continue;
            }
            knownPeak * kk=kpl.add(1);
            *kk=knownPeak(imol,iAss,iMaxRegion, coef, itgrlSubs,rmass,maxAlt,firstIntensity);
            kk->iMolOri=isomol->shiftedFrom==sNotIdx ? imol : isomol->shiftedFrom;
            kk->shiftOri=isomol->frmlShift;
            kk->matchPosX=smin+matchPos*allStp;
            kk->matchPosY=matchPosY;
            //gLog->printf(" ... found intensity %lf in the range of %lf - %lf\n", itgrlSubs, smin+gFirstNonZero*allStp+grpShift*allStp, smin+gLastNonZero*allStp+grpShift*allStp);

            ++cntDetected;

        }
        return cntDetected;
}
/*idx spectraPeaks::peakGroup[]={0,-14,16,-28,30,-42,44,-1};*/ //Rado
//idx spectraPeaks::peakGroup[]={0,-14,16,-28,30,-42,44,-1};

spectraPeaks::satellite_peaks * spectraPeaks::peakGroup = 0;

idx spectraPeaks::peaksDetect(const char * nam, const char * srcsfx,sVec < knownPeak >  * kpl,sVec < knownPeak >  * kpl2,real allStp)
{


    //if(alwaysPeaks.dim()==0 && gSet.collect.alwaysPeaks.length())
    sVec < real > alwaysDetect;
    if(collect.alwaysPeaks.length())
        sString::sscanfRVec(&alwaysDetect, collect.alwaysPeaks.ptr(), collect.alwaysPeaks.length());

    sSpectrMS::isoDistribution::absShiftAll=peaks.shiftIsoPeaks;

    //remember originally known isotopic distributions count
    idx originalIsoCount=IsoD.isoDistr.dim();

    // prepare the source file
    sStr nm("%s.x",nam);

    sVec <real> xx(nm.ptr());
    idx cnt=xx.dim(); if(!cnt)return -1;

    nm.printf(0,"%s.%s",nam, srcsfx);
    sVec <real> src(nm.ptr());


/*
    // initial sattelite intensities
    real satMinIntensity[]={0, 0.05, 0.01, 0.025,  0.01,  0.01,  0.01,  -1};
    real satMaxIntensity[]={0, 0.30, 0.10, 0.150,  0.10,  0.07,  0.10,  -1};
    real satRealIntensity[sDim(peaks.satellite)];
    //gLog->printf("Looking for %d peaks within a threshold of %lf %% \n", peaks->maxPeaks, peaks->threshold);
    //gLog->printf("Satellite intensity limits are imposed:\n");
    for(int i=0; i<sDim(satRealIntensity); ++i) {
        satRealIntensity[i] = (peaks.satellite[i]!=0) ? peaks.satellite[i] : satMaxIntensity[i] ;
        if(peakGroup[i]!=0 && peakGroup[i]!=-1){
            //gLog->printf("Peak %+d relative intensity %lf (should be >%lf and <%lf) \n",peakGroup[i], satRealIntensity[i], satMinIntensity[i] ,satMaxIntensity[i]);
        }
    }*/

    peaks.satellite.resize(peaks.minMax_satellite.dim());
    //peakGroup.resize(peaks.minMax_satellite.dim());
    for(int i=0; i< peaks.minMax_satellite.dim(); ++i) {
        peaks.satellite[i].intensity = peaks.minMax_satellite[i].max_intensity;
        peaks.satellite[i].shift = peaks.minMax_satellite[i].shift;
        /*peakGroup[i].shift =  peaks.satellite[i].shift;
        peakGroup[i].intensity =  peaks.satellite[i].intensity;*/

    }
    peakGroup = peaks.satellite.ptr(0);
    //setPeakGroup(peaks.satellite.ptr(0));

     // peakGroup[i] -> peaks.satellite[i].shift
    // satRealIntensity[i] -> peaks.satellite[i].intensity

    real totItgrl=0;//1/gSet.allStp;
    for ( idx jit=0;jit<cnt; ++jit){
        totItgrl+=src[jit];
    }

    idx cntDetected=0, imol=0;
    real firstMax=0;

    sVec <real > Bins; real * bins=Bins.add(cnt+10)+1; // bins for mapping the isotopic distributions
    sVec < idx > ind;

    // ========================================================
    /*  FIRST STAGE */
    {
        sStr * flnmKP = m_tmpFiles.add(1);
        flnmKP->printf(0,"%s.1knownpeaks",nam);
        sVec <real> knownpeaks1(flnmKP->ptr());
        knownpeaks1.cut(0);knownpeaks1.add(cnt);knownpeaks1.set();

        sStr * flnmKPO = m_tmpFiles.add(1);
        flnmKPO->printf(0,"%s.1knownpeaksoff",nam);
        sVec <real> knownpeaksoff1(flnmKPO->ptr());
        knownpeaksoff1.cut(0);knownpeaksoff1.add(cnt);

        { // first stage
            // prepare the list of destination files

            memcpy(knownpeaksoff1.ptr(),src.ptr(),cnt*sizeof(real)); // we copy the source to this list ... we are subtracting found peaks from this

            //gLog->printf("\nDetecting known peaks ... \n");
            // first we decect and subtruct the known peaks
            if (peaks.satellite.dim()) {
                for( idx iAss=0; peaks.satellite[iAss].shift !=-1 && iAss<peaks.satellite.dim(); ++iAss ){
                    for( imol=0; imol<IsoD.isoDistr.dim() &&  imol<peaks.maxPeaks; ++imol) { // one by one we go over all of the isomolecules
        //                sVec <real> bins(nm.makeName("",PEAK_WORKDIR"/iso_%d",imol+1)); bins.cut(0);bins.add(cnt);

                        cntDetected+=peakeDetectSingle(*kpl,imol, cnt, xx[0], xx[cnt-1],knownpeaksoff1, bins, knownpeaks1, iAss,iAss+1, peaks.threshold*0.01*totItgrl, false, 0, 1 , &alwaysDetect, allStp); // secondaryfarction   // idx allStp, Collect * collect, Peaks * peaks
                    }
                }
            }
        }

        sVec <real> unknownpeaks1(nm.printf(0,"%s.%s",nam,"1unknownpeaks")); unknownpeaks1.cut(0);unknownpeaks1.add(cnt);unknownpeaks1.set(0);
        sVec <real> allpeaksoff1(nm.printf(0,"%s.%s",nam, "1allpeaksoff")); allpeaksoff1.cut(0);allpeaksoff1.add(cnt);
        memcpy(allpeaksoff1,knownpeaksoff1,sizeof(real)*cnt); // we continue subtracting here


        if(peaks.dogenerate) {// then we detect  and subtruct unknown peaks
            //gLog->printf("\n\nDetecting unknown peaks ... \n");
            for( ; imol<peaks.maxPeaks; ++imol)  {
                idx iTop, ik ;

                for ( iTop=0, ik=1; ik<cnt; ++ik ) {if(allpeaksoff1[iTop]<allpeaksoff1[ik] ) iTop=ik;}// determine the maximum of the spectrum to identify it as a peak
                if(!firstMax) firstMax=allpeaksoff1[iTop];
                if(allpeaksoff1[iTop]<firstMax*peaks.threshold*0.01) break;

                IsoD.makeCloseIsoDistribution(xx[0]+allStp*iTop,0,originalIsoCount);// we generate a close isodistribution from one of the original ones;

                idx newDetected=peakeDetectSingle(*kpl,imol, cnt,xx[0],xx[cnt-1],allpeaksoff1, bins, unknownpeaks1, 0,peaks.satellite.dim(),peaks.threshold*0.01*totItgrl, false, 0, 1  , &alwaysDetect,  allStp);
                if(!newDetected)break;
                cntDetected+=newDetected;
            }
        }


        //gLog->printf("\n\nPerforming output for peakmajor files (first stage)... \n");
        ind.add(kpl->dim());
        sSort::sort(kpl->dim(), kpl->ptr(),ind.ptr());
        peakKnownOut(nam,*kpl,"peak",xx[0],allpeaksoff1.ptr(), knownpeaksoff1.ptr(), src.ptr(), knownpeaks1.ptr(), unknownpeaks1.ptr(), cnt,0,ind, allStp);

    }

    // ========================================================
    // autodetermine the left and right sattelite intensities from the top peak
    real satelliteControl=IsoD.isoDistr[peaks.satelliteControlI-1].getMass();
    //gLog->printf("\n\nProceeding to autodetermination of satellite intensities from a major peak %lf ... \n",satelliteControl);
    idx imax;
    for( imax=0; imax<kpl->dim() && fabs(kpl->ptr(imax)->rmass-satelliteControl)>0.5; ++imax) ;
    //gLog->printf("\n");
        // now set the satellite intensities to the minimummms in case some of the major peaks satellites are not there.
        //for(int i=0; i<sDim(peaks.satellite.dim()); ++i) satRealIntensity[i] = (peaks.satellite[i]!=0) ? peaks.satellite[i] : satMinIntensity[i] ;
        if( imax<kpl->dim() && kpl->ptr(imax)->rmass==satelliteControl) { // the top peak has been found
        knownPeak * kp=kpl->ptr(imax);
        ///idx ia, iaLeft=-1, iaRight=-1, satLeft=-10000, satRight=+10000;
        for( idx ia=0; ia<kpl->dim() ; ++ia) {
            knownPeak * ka=kpl->ptr(ind[ia]);
            if( ka->iGrp==0 || ka->rmass!=kp->rmass)continue;

            //if(peaks.satellite[ka->iGrp]==0){
            peaks.satellite[ka->iGrp].intensity = ka->maxAmplitude/kp->maxAmplitude;
                //gLog->printf("Rel. intensity for P%+d computed as %lf",peakGroup[ka->iGrp],satRealIntensity[ka->iGrp]);
                if(peaks.satellite[ka->iGrp].intensity<peaks.minMax_satellite[ka->iGrp].min_intensity){
                    peaks.satellite[ka->iGrp].intensity=peaks.minMax_satellite[ka->iGrp].min_intensity;
                    //gLog->printf("... too low, must be at least %lf",satMinIntensity[ka->iGrp]);
                }
                if(peaks.satellite[ka->iGrp].intensity>peaks.minMax_satellite[ka->iGrp].max_intensity){
                    peaks.satellite[ka->iGrp].intensity=peaks.minMax_satellite[ka->iGrp].max_intensity;
                  //  gLog->printf("... too high, must be at less than %lf",satMaxIntensity[ka->iGrp]);
                }
                //gLog->printf("\n");
            //}
        }
        }else {
        //gLog->printf("WARNING: Top peak %lf has not been found\n", satelliteControl);
        //gLog->printf("WARNING: Satellite peak intensities will not be adjusted but taken as defaults \n");
    }

    //gLog->printf("Final satellite intensity profile after adjustments follow:\n");
    //for(int i=1; i<sDim(satRealIntensity); ++i) {
        //gLog->printf("  P%+d=%lf\n",peakGroup[i],satRealIntensity[i]) ;
    //}
    //gLog->printf("\n");



    //gLog->printf("\n\nReinitializing for the second run : left to right \n");
    {
        sVec <real> peaks2(nm.printf(0,"%s.%s",nam, "2peaks")); peaks2.cut(0);peaks2.add(cnt);peaks2.set();
        sVec <real> peaksoff2(nm.printf(0,"%s.%s", nam,"2peaksoff")); peaksoff2.cut(0);peaksoff2.add(cnt);

        // reset the arrays for the second run from left to right
        memcpy(peaksoff2.ptr(),src.ptr(),cnt*sizeof(real)); // we copy the source to this list ... we are subtracting found peaks from this

        //gLog->printf("\n\nDetecting peaks from left to right\n");

        cntDetected=0;
        //sVec < knownPeak > kpl2;
        idx prvMol=-1, prvGrp=-1;
        for( imol=0; imol<kpl->dim() ; ++imol) { // one by one we go over all of the isomolecules
            knownPeak * kp=kpl->ptr(ind[imol]);
            if(prvMol==kp->iMol && prvGrp==kp->iGrp) continue; // two peaks matched at a single point

            knownPeak * kParent=0;
            for( idx ip=0; ip<kpl->dim() ; ++ip) {if( kpl->ptr(ip)->rmass==kp->rmass && kpl->ptr(ip)->iGrp==0 ){kParent=kpl->ptr(ip);break;}}
            cntDetected+=peakeDetectSingle(*kpl2,kp->iMol, cnt,xx[0],xx[cnt-1],peaksoff2,bins, peaks2, kp->iGrp,kp->iGrp+1, peaks.threshold*0.01*totItgrl,true, kParent, 2 , &alwaysDetect, allStp);

            prvMol=kp->iMol;
            prvGrp=kp->iGrp;
        }


        //gLog->printf("\nPerforming output for peak2major files (second stage)... \n");
        sVec < idx > ind2; ind2.add(kpl2->dim());
        sSort::sort(kpl2->dim(), kpl2->ptr(),ind2.ptr());
        peakKnownOut(nam,*kpl2,"peak2",xx[0],peaksoff2.ptr(),peaksoff2.ptr(),src.ptr(),peaks2.ptr(), peaks2.ptr(), cnt,0,ind2, allStp);

    }
    //gLog->printf("\nFinished peak detection stage\n");
    //gLog->printf("\nRestoring isotopic tables\n");
    IsoD.isoDistr.cut(originalIsoCount);


    return cntDetected;
}



idx spectraPeaks::peaksGenerateKnown(const char * path, real allStp, real smin, real smax, bool doredo )
{
    idx cntBin=(idx)((smax-smin)/allStp)+1;
    sVec <real > memBins; real * membins=memBins.add(cntBin); // bins for mapping the isotopic distributions

    idx gFirstNonZero, gLastNonZero, gMaxBin,gFirsMaxBin;
    for( idx imol=0; imol<IsoD.isoDistr.dim() &&  imol<peaks.maxPeaks; ++imol) { // one by one we go over all of the isomolecules
        sStr * isoFlnm=m_tmpFiles.add(1);
        isoFlnm->printf("%siso_%" DEC "",path, imol+1);
        //m_tmpFiles.add();


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
