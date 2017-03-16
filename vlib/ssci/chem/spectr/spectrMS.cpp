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
#include <slib/std/string.hpp>
#include <slib/utils/sort.hpp>
#include <ssci/chem/spectr/spectr.hpp>

using namespace slib;

real sSpectrMS::isoDistribution::absShiftAll=0.;


idx sSpectrMS::isoDistributionList::makeMolList (sStr * molList, idx istart, idx iend) 
{
    if (iend==-1) iend=isoDistr.dim();

    for( idx il=istart; il<iend; ++il) {
        molList->printf("%s",(const char *)isoDistr.id(il));
        //continue;
        isoDistribution * ssaa=isoDistr.ptr(il);
        
        //for( idx ir=0; ir<isoDistr[il].spctr.dim(); ++ir) { molList->printf(" %lf %lf",isoDistr[il].spctr[ir].rMass,isoDistr[il].spctr[ir].abundance);} 
        for( idx ir=0; ir<isoDistr[il].spctr.dim(); ++ir) { 
            sChem::isotop * sotop=ssaa->spctr.ptr(ir);
            molList->printf(" %lf %lf",sotop->rMass,sotop->abundance);
            
            //real oo =sotop->rMass;
            ///real pp=sotop->abundance;
            //continue;
            //printf("%" DEC " %" DEC " @@@@ %" DEC " %" DEC " %" DEC " @@@@@@@@@ %p %p %p\n", il, ir, sizeof(sChem::isotop) , sizeof(int) , sizeof(real), sotop, &sotop->rMass, &sotop->abundance);
            //molList->printf(" %lf ",12.);//sotop->rMass
        } 
        molList->printf("\n");
    }
    //printf("term\n");
    return isoDistr.dim();
}


//idx sSpectrMS::isoDistributionList::readIsoDistributions(const char * molListFile, real tol1, real tol2)
idx sSpectrMS::isoDistributionList::readIsoDistributions(const char * src, idx len, real tol1, real tol2, real shift)
{
    //DAVID's edit - empty isoDistribution before reloading so it isni twice as much big ;)
    empty();

    isoTol1=tol1;
    isoTol2=tol2;
    
    aveElem.add(sChem::element::cntElements);// for periodic elements 
    aveElem.set(0);
    idx totElem=0;
    isoDistr.mex()->flags|=sMex::fSetZero;
    bool computed=false;
    {
        sChem::isotop ms;
        //sStr tmp;sString::searchAndReplaceSymbols(&tmp,molList.ptr(),molList.length(),sString_symbolsEndline,0,0,true,true,true);
        sStr tmpA;sString::cleanEnds(&tmpA,src,len,sString_symbolsBlank,true);
        sStr tmp;sString::searchAndReplaceSymbols(&tmp,tmpA.ptr(),tmpA.length(),sString_symbolsEndline,0,0,true,true,true);
        idx cnt=0;
        real daltons;

        for (char * p = tmp.ptr(),* nxt, *s; p ; p=nxt, ++cnt){ // go line by line 

            nxt=sString::next00(p);
            sString::cleanEnds(p,0,sString_symbolsBlank,true);
            s=sString::skipWords(p,0,1,0);
            sString::searchAndReplaceSymbols(p,0,sString_symbolsBlank,0,0,true,true,true);
            
            //void * a=sNew(1024,0);
            sSpectrMS::isoDistribution * isos = isoDistr.set(p,sLen(p)+1); // add one more isotopic spectra 
            //isos->isoMolNameOfs=isoMolNames.add(p,sLen(p)+1);
            isos->frml.parse(p);
            for(idx i=0; i<isos->frml.dim(); ++i) { 
                aveElem[isos->frml[i].element]+=isos->frml[i].count;
                totElem+=isos->frml[i].count;
            }
            
            for(; s ; s=sString::next00(s)) { // first we try reading from file 
                if(!sscanf(s,"%lf",&ms.rMass))break;
                if(shift)ms.rMass+=shift;
                s=sString::next00(s);
                if(!s || !sscanf(s,"%lf",&ms.abundance))break;
                *isos->spctr.add(1)=ms;
            }
            if(!isos->spctr.dim()) { // if file doesn't have that information - we try composing them 
                printf("Building isotopic distribution for %s\n",p);
                if(isos->frml.dim() ) { 
                    isos->frml.isotopicDistr(&isos->isoFormulas, tol1,tol2);// formula was present ? 
                    for ( idx i=0; i< isos->isoFormulas.dim(); ++i) {
                        ms.rMass=isos->isoFormulas[i].mass( sChem::formula::eIsotopicExact, false) ;
                        if(shift)ms.rMass+=shift;
                        ms.abundance=isos->isoFormulas[i].prob;
                        sStr out; isos->isoFormulas[i].printfAtomList(&out, true);
                        printf("%lf/%lf -- %s",ms.rMass, ms.abundance, out.ptr());
                        *isos->spctr.add(1)=ms;
                    }
                } else if( sscanf(p,"%lf", &daltons)==1) {  // a mass is directly specified
                    makeCloseIsoDistribution(daltons,isos);
                }
                computed=true;
            }
            if(!isos->spctr.dim()) {
                printf("Could not read or generate isotopicSpectra for %s",p);
                continue;
            }

            sSort::sort(isos->spctr.dim(),isos->spctr.ptr());
            
        }

    }

    if(!totElem) {
        aveElem[1]=0.53118072570295183;
        aveElem[6]=0.30116089137790225;
        aveElem[7]=0.013388169408470424;
        aveElem[8]=0.15184925912962316;
        aveElem[11]=0.0024209543810523862;
    }else { 
        for(idx i=0; i<aveElem.dim(); ++i) { aveElem[i]/=totElem;}
    }
    
//    cntPermaDistr=isoDistr.dim();
    return isoDistr.dim();
}





idx sSpectrMS::isoDistributionList::makeAverageIsoDistribution(real daltons)
{
    idx ik;

    isoDistr.mex()->flags|=sMex::fSetZero;
    sSpectrMS::isoDistribution * isos = isoDistr.add(); // add one more isotopic spectra 
            
    real aveUnitWeight=0;
    sChem::atomcount ac;

    for(ik=0; ik<aveElem.dim(); ++ik) {
        if(aveElem[ik]==0) continue;
        aveUnitWeight+=aveElem[ik]*sChem::element::ptr(ik)->atomicMass;
    }

    real units=daltons/aveUnitWeight;

    // create and average formula 
    idx iH=0, iC=0;
    for(ik=0; ik<aveElem.dim(); ++ik) {
        if(aveElem[ik]==0) continue;
        ac.element=ik;
        ac.count=(idx)(aveElem[ik]*units+0.5);
        if(ac.element==1)iH=isos->frml.dim();
        else if(ac.element==6)iC=isos->frml.dim();
        *isos->frml.add(1)=ac;
    }
    real lastMass;
    while( (lastMass=isos->frml.mass( sChem::formula::eIsotopicAbundant, false) ) <daltons )++(isos->frml[iC].count); //add carbons until close to the formula 
    while( (lastMass=isos->frml.mass( sChem::formula::eIsotopicAbundant, false) ) >daltons )--(isos->frml[iC].count); //add carbons until close to the formula 
    while( (lastMass=isos->frml.mass( sChem::formula::eIsotopicAbundant, false) ) <daltons )++(isos->frml[iH].count); //add hydrogens until close to the formula 
    while( (lastMass=isos->frml.mass( sChem::formula::eIsotopicAbundant, false) ) >daltons )--(isos->frml[iH].count); //add hydrogens until close to the formula 
    
    //printf("Building isotopic distribution for %s\n",buf);
    isos->frml.isotopicDistr(&isos->isoFormulas, isoTol1,isoTol2);
            
    sChem::isotop ms;ms.iMass=0; ms.dummy=0;
    idx iMax=0;
    for ( idx i=0; i< isos->isoFormulas.dim(); ++i) {
        ms.rMass=isos->isoFormulas[i].mass( sChem::formula::eIsotopicExact, false) ;
        ms.abundance=isos->isoFormulas[i].prob;
        *isos->spctr.add(1)=ms;
        if(ms.abundance>isos->spctr[iMax].abundance)iMax=i;
    }

    real shift=daltons-isos->spctr[iMax].rMass;
    shiftIsoDistribution(isoDistr.dim()-1, shift);
    
    sSort::sort(isos->spctr.dim(),isos->spctr.ptr());

    sStr tmp;isos->frml.printfAtomList(&tmp,false);tmp.printf("+%lf",shift);
    isoDistr.dict(isoDistr.dim()-1,tmp.ptr(),sLen(tmp.ptr())+1);
    //isos->isoMolNameOfs=isoMolNames.add(tmp.ptr(),sLen(tmp.ptr())+1);
    
    return isoDistr.dim();

}



idx sSpectrMS::isoDistributionList::makeCloseIsoDistribution(real daltons, isoDistribution * lisos, idx maxCntToUse)
{
    sSpectrMS::isoDistribution * isoClos, * isos = lisos ? lisos : isoDistr.add(); // add one more isotopic spectra 
    
    daltons-=sSpectrMS::isoDistribution::absShiftAll;
    idx iClosest=0;
    real closeMasDiff=1e+13,rmass, masDiff;
    if(maxCntToUse==0)maxCntToUse=isoDistr.dim();
    for( idx il=0; il<maxCntToUse; ++il) {
        isoClos=isoDistr.ptr(il);if(isoClos==isos)continue; // we do not consider self 
        ///rmass=isoClos->frml.mass( sChem::formula::eIsotopicAbundant, false);
        rmass=isoClos->getMass();
        masDiff=sAbs(rmass-daltons);
        if(masDiff<closeMasDiff){iClosest=il;closeMasDiff=masDiff;}
    }
    isoClos=isoDistr.ptr(iClosest);
    isos->shiftedFrom=iClosest;

    for(idx ie=0; ie<isoClos->frml.dim(); ++ie)
        *isos->frml.add(1)=isoClos->frml[ie];
    isos->rMass=isoClos->rMass;
    
    for ( idx i=0; i< isoClos->isoFormulas.dim(); ++i) {
        sChem::formula *cf=isos->isoFormulas.add();
        for(idx ie=0; ie<isoClos->isoFormulas[i].dim(); ++ie)
            *cf->add()=isoClos->isoFormulas[i][ie];
    }
    idx iMax=0;
    real maxAbundance=0;
    for ( idx i=0; i< isoClos->spctr.dim(); ++i) {
        *isos->spctr.add()=isoClos->spctr[i];
        if(isos->spctr[i].abundance>=maxAbundance){
            iMax=i;maxAbundance=isos->spctr[i].abundance;
        }
   }

    //real oo=isos->spctr[iMax].rMass;
    real shift=daltons-isos->spctr[iMax].rMass;
    shiftIsoDistribution(isoDistr.dim()-1, shift);
    
    sSort::sort(isos->spctr.dim(),isos->spctr.ptr());

    if(!lisos){
        sStr tmp;tmp.printf("<->");isos->frml.printfAtomList(&tmp,false);tmp.printf("+%lf",shift);
        isoDistr.dict(isoDistr.dim()-1,tmp.ptr(),sLen(tmp.ptr())+1);
        //isos->isoMolNameOfs=isoMolNames.add(tmp.ptr(),sLen(tmp.ptr())+1);
    }
    return isoDistr.dim();

}



void sSpectrMS::isoDistributionList::shiftIsoDistribution(idx imol,real shift)
{
    sSpectrMS::isoDistribution * isos = isoDistr.ptr(imol); // add one more isotopic spectra 

    isos->frmlShift=shift;
    for ( idx i=0; i< isos->spctr.dim(); ++i) {
        isos->spctr[i].rMass+=shift;
    }
}



