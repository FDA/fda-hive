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
#include <slib/core/index.hpp>
#include <ssci/math/clust/clust.hpp>
#include <ssci/math/rand/rand.hpp>
#include <slib/std/file.hpp>
#include <slib/utils/txt.hpp>
#include <slib/utils/sort.hpp>

#include <math.h>
using namespace slib;

#define DONOTCOMPILE

idx sRlda::compute(sMatrix & ori , sDic < sVec < idx >  > & grpset, sVec< real> * ldaTransformVals, sMatrix * ldaTransformVecs, real regulAlpha, idx scaleType)
{
    idx cols=ori.cols();
    sFilePath nm;
    sMatrix Stats;Stats.resize(grpset.dim(),cols);
    sVec < sMatrix > Scat;Scat.add(grpset.dim());
    sVec < real > TotStat;




    if(scaleType) {
        TotStat.add(ori.cols()*2);
        ori.computeRowStat(TotStat.ptr(0),TotStat.ptr(cols),0);
    }

    for ( idx il=0; il<grpset.dim(); ++il) {
        sMatrix grpmat;ori.extractRowset(grpmat, grpset[il]);

        if(scaleType && TotStat.dim()) {
            for(idx ir=0;ir<grpmat.rows(); ++ir ) {
                for(idx ic=0;ic<cols; ++ic ) {
                    grpmat.val(ir,ic)=(grpmat.val(ir,ic)-TotStat[ic] );
                    if(TotStat[ic+cols]==0)continue;
                    grpmat.val(ir,ic)/=TotStat[ic+cols];
                }
            }
        }

        grpmat.computeRowStat(Stats.ptr(il,0),0,0);
        grpmat.shiftRows(Stats.ptr(il,0));
        grpmat.scatter(Scat[il]);
    }


    sMatrix ScatterW; ScatterW.resize(cols,cols);ScatterW.set(0);
    for ( idx il=0; il<grpset.dim(); ++il) {
        for( idx ic1=0; ic1<cols; ++ic1) {
            for( idx ic2=0; ic2<cols; ++ic2) {
                ScatterW(ic1,ic2)+=Scat[il](ic1,ic2);
            }
        }
    }
    #ifdef DEBUG_WORKDIR
        ScatterW.out(DEBUG_WORKDIR"/ScatterW.csv",0,false,false,"%lg");
    #endif
    if(regulAlpha!=0.){
        for ( idx il=0; il<grpset.dim(); ++il) {
            for( idx ic1=0; ic1<cols; ++ic1) {
                ScatterW(ic1,ic1)+=regulAlpha;
            }
        }
        #ifdef DEBUG_WORKDIR
            ScatterW.out(DEBUG_WORKDIR"/ScatterWRegul.csv",0,false,false,"%lg");
        #endif
    }

    sVec < real > meanMean;meanMean.resize(ori.cols());Stats.computeRowStat(meanMean,0);
    Stats.shiftRows(meanMean);
    sMatrix ScatterB;
    Stats.scatter(ScatterB,1./cols);
    #ifdef DEBUG_WORKDIR
        ScatterB.out(DEBUG_WORKDIR"/ScatterB.csv");
    #endif

    sMatrix copy; ScatterW.copy(copy);
    ScatterW.inverse();
    sMatrix unit;unit.multiplyMatrixes(ScatterW,copy);

    #ifdef DEBUG_WORKDIR
        ScatterW.out(DEBUG_WORKDIR"/ScatterWInverse.csv");
        unit.out(DEBUG_WORKDIR"/unit.csv");
    #endif

    for( idx ic1=0; ic1<cols; ++ic1) {
        for( idx ic2=0; ic2<cols; ++ic2) {
            real kronecker=(ic1==ic2) ? 1. : 0.;
            if( (unit(ic1,ic2)-kronecker)>1.e-6 )
                return 1;
        }
    }

    sMatrix SBSW; SBSW.multiplyMatrixes(ScatterW,ScatterB);

    #ifdef DEBUG_WORKDIR
        SBSW.out(DEBUG_WORKDIR"/SBSW.csv");
    #endif
    ldaTransformVecs->resize(cols*2,cols);
    ldaTransformVals->resize(SBSW.cols()*2);

    SBSW.transpose();

    SBSW.diagNonSym(ldaTransformVals->ptr(0), ldaTransformVecs,0);
    #ifdef DEBUG_WORKDIR
        ldaTransformVecs->out(DEBUG_WORKDIR"/ldaTransformVecs.csv");
    #endif

    ldaTransformVals->cut(cols);
    ldaTransformVecs->head()->rows = cols;

    sAlgebra::matrix::diagSort(cols,ldaTransformVals->ptr(),ldaTransformVecs->ptr(0,0),-1);
    ldaTransformVecs->normalilzeCols(true);




    #ifdef DEBUG_WORKDIR
        SBSW.out(DEBUG_WORKDIR"/diagSBSW.csv");
    #endif

    return 0;

}




idx sRlda::bootstrap(sMatrix & mat, idx maxIter, idx squeezeSize, real importantLDA, idx randSeed, sDic < sVec < idx > > & catSet, sDic < sVec < idx > > & checkSet, sIO * gLog, sStr * flda, sStr * cat , sStr * rslfCSV, sDic <idx > * rids, sDic < idx > * cids)
{

    sFilePath fflnm;
    sMatrix::MatrixDicHeaders hd; hd.cols=cids;hd.rows=rids;

    sVec < real > importance,impLocal;importance.add(mat.cols());impLocal.add(mat.cols());
    sVec < real > classifier,clsLocal;classifier.add(mat.cols());clsLocal.add(mat.cols());
    sVec < sVec < idx >  > belong;  belong.add(mat.rows()) ;
    for( idx ir=0; ir<belong.dim(); ++ir)   {belong[ir].add(catSet.dim());belong[ir].set();}
    idx strongestCol=sNotIdx;
    importance.set();classifier.set();

    for ( idx iter=0; iter<maxIter; ++iter) {

        gLog->printf("\n\nITERATION %" DEC "\n",iter+1);

        sDic < sVec < idx > > grpset;
        for ( idx il=0 ; il<catSet.dim();  ++il)
            grpset.set(catSet.id(il));

        for ( idx il=0 ; il<catSet.dim();  ++il) {
            sVec < idx > & gs=grpset[il];
            for ( idx ic=0 ; ic<catSet[il].dim();  ++ic) { *gs.add()=catSet[il][ic];}
            idx realSqueeze = (squeezeSize>0) ? squeezeSize : (gs.dim()+squeezeSize);

            if(realSqueeze*2<gs.dim()){
                gLog->printf("With current bootstrap space of %" DEC " - it is not possible to RLDA analyse %s\n",squeezeSize,catSet.id(il));
                return 0;
            }
            while ( gs.dim()>realSqueeze) {
                idx todel=(idx)(sRand::ran0(&randSeed)*gs.dim());
                if(todel>=gs.dim())todel=0;
                gs.del(todel);
            }
            gLog->printf("Current randomly chosen learning set\n");
            for( idx is=0; is<gs.dim() ; ++is)
                gLog->printf("%s[%" DEC "]  ", rids->id(gs[is]),gs[is]);
            gLog->printf("\n");
        }

        for(idx ir=0;ir<rids->dim(); ++ir) (*rids)[ir]=sNotIdx;
        sText::categoryListToDic( &grpset, rids);


        idx prvCheckNegit=mat.rows();
        real bestRegul=regulAlphaStart;
        for( real regulAlpha=regulAlphaStart; regulAlpha<=regulAlphaEnd ; regulAlpha+=(regulAlphaEnd - regulAlphaStart )*0.1) {

            sVec< real> ldaTransformVals;
            sMatrix ldaTransformVecs;
            idx errcode=compute(mat, grpset, &ldaTransformVals,&ldaTransformVecs,regulAlpha);
            if(errcode) {
                gLog->printf("\n\nFATAL ERROR: cannot continue the computation, SW singularity!\n");
                return 1;
            }

            for(idx ie=0; ie<ldaTransformVals.dim(); ++ie) ldaTransformVals[ie]=sqrt(sAbs(ldaTransformVals[ie])/(ldaTransformVals.dim()-1));
            sMatrix trsMat;trsMat.resize(mat.rows(),mat.cols());
            sAlgebra::matrix::pcaReMap(trsMat.ptr(0,0),mat.ptr(0,0), mat.cols(), mat.rows(), ldaTransformVecs.ptr(0,0));


            sMatrix Stats;Stats.resize(grpset.dim()*2+2,mat.cols());
            real * stat, * stdev, *statG, * stdevG;
            {
                flda->printf("\n\nIteration %" DEC "\n\n",iter+1);

                flda->printf("Activity matrix after rotation to LDA optimized space\n");
                trsMat.out(flda,&hd, false, true);

                flda->printf("\n\nStatistics after rotation to LDA discriminant\n");
                for ( idx il=0; il<grpset.dim(); ++il) {
                    sMatrix grpmat;
                    trsMat.extractRowset(grpmat, grpset[il]);
                    stat=Stats.ptr(il,0); stdev=Stats.ptr((grpset.dim()+il),0);
                    grpmat.computeRowStat(stat, stdev);
                    flda->printf("mean_g%" DEC "",il+1);for(idx ic=0; ic<Stats.cols(); ++ic) flda->printf(",%lf",stat[ic]);flda->printf("\n");
                    flda->printf("sd_g%" DEC "",il+1);for(idx ic=0; ic<Stats.cols(); ++ic) flda->printf(",%lf",stdev[ic]);flda->printf("\n");
                }

                statG=Stats.ptr(grpset.dim()*2,0); stdevG=Stats.ptr(grpset.dim()*2+1,0);
                trsMat.computeRowStat(statG,stdevG);
                flda->printf("mean_all");for(idx ic=0; ic<Stats.cols(); ++ic) flda->printf(",%lf",statG[ic]);flda->printf("\n");
                flda->printf("sd_all");for(idx ic=0; ic<Stats.cols(); ++ic) flda->printf(",%lf",stdevG[ic]);flda->printf("\n");

                flda->printf("\n\nPeak contributions in order of importance\n");
                flda->printf("#LDA, StdDev, StdDev%%, Contributions (%%weight, peak, ... )\n");
                for(idx ic=0; ic<ldaTransformVecs.rows(); ++ic) {
                    ldaTransformVecs.outSingleEvecSrt(flda, ldaTransformVals.ptr(0), ic, cids);
                }

                flda->printf("\n\nMost important few contributors\n");
                for( idx ic=0; ic<ldaTransformVecs.rows(); ++ic){
                    real val=ldaTransformVecs.val(ic,0);
                    if(sAbs(val)<importantLDA)continue;
                    flda->printf("%s %lf\n",(const char*)cids->id(ic), val);
                }

                flda->printf("\n\nEigenvalues and eigenvectors of scattering discriminant matrix\n");
                hd.colset=0;hd.rowset=0;hd.rows=cids;
                ldaTransformVecs.out(flda,&hd, false, true);
                hd.rows=rids;
            }

            cat->printf("\n\nIteration %" DEC "\n\n",iter+1);
            idx ldaNum=0;
            idx posit=0,negit=0;
            idx checkposit=0,checknegit=0;
            for(idx ir=0; ir<trsMat.rows(); ++ir){
                real discrMin=1e+13;
                idx iMin=0;
                for ( idx il=0; il<grpset.dim(); ++il) {
                    stat=Stats.ptr(il,0); stdev=Stats.ptr((grpset.dim()+il),0);
                    real val=trsMat(ir,ldaNum);
                    real fal= (val-stat[ldaNum])/stdevG[ldaNum];
                    val=fal*fal;
                    if(discrMin>val) {discrMin=val; iMin=il;}
                }
                ++belong[ir][iMin];

                idx categ=(*rids)[ir];
                cat->printf("%s %30s %30s",(char*)rids->id(ir),categ==sNotIdx ? "TBD" : (const char*)grpset.id(categ),(const char*)grpset.id(iMin));

                if(categ!=sNotIdx){
                    if(categ==iMin) {++posit;cat->printf(" +");}
                    else {++negit;cat->printf(" -");}
                }else cat->printf("  ");


                for(idx il=0,ii; il<checkSet.dim(); ++il) {
                    for(ii=0; ii<checkSet[il].dim(); ++ii) {
                        if(ir==checkSet[il][ii])break;
                    }
                    if(ii>=checkSet[il].dim())continue;

                    cat->printf(" %30s",(const char * )checkSet.id(il));
                    if(!strcmp((const char *)checkSet.id(il),(const char *)grpset.id(iMin))){
                        ++checkposit;
                        if(categ!=iMin)cat->printf(" +");
                    }
                    else {++checknegit;cat->printf(" -");}
                    break;
                }
                cat->printf("\n");
            }


            if( checknegit < prvCheckNegit ){
                for( idx ir=0; ir<ldaTransformVecs.rows(); ++ir) {
                    real v=ldaTransformVecs.val(ir,ldaNum);
                    impLocal[ir]=v*v;
                    clsLocal[ir]=v;
                }
                bestRegul=regulAlpha;
                prvCheckNegit=checknegit ;
                cat->printf("regulAlpha %lf has been chosen as best up to this point\n",regulAlpha);
                gLog->printf("regulAlpha %lf has been chosen as best up to this point\n",regulAlpha);
            }

            ldaTransformVecs.outSingleEvecSrt(cat, ldaTransformVals.ptr(0),0, cids);
            cat->printf("selfcheck positives %" DEC "  negatives %" DEC " \n",posit, negit);
            cat->printf("realcheck positives %" DEC "  negatives %" DEC " \n",checkposit, checknegit);
            gLog->printf("selfcheck positives %" DEC "  negatives %" DEC " \n",posit, negit);
            gLog->printf("realcheck positives %" DEC "  negatives %" DEC " \n",checkposit, checknegit);
            if( regulAlphaEnd == regulAlphaStart )break;
        }

        idx dir=1;
        if(strongestCol!=sNotIdx) {
            if(classifier[strongestCol] *clsLocal[strongestCol] < 0) dir=-1;
        }
        strongestCol=0;

        for( idx ir=0; ir<impLocal.dim(); ++ir) {
            importance[ir]+=impLocal[ir];
            classifier[ir] +=clsLocal[ir]*dir;
            if( sAbs( classifier[strongestCol] )  < sAbs ( classifier[ir] ) )
                strongestCol=ir;
        }

        gLog->printf("best regulAlpha = %lf\n",bestRegul);
    }


    sStr rslt;
    rslt.printf("\n\nBootstrapped column importance\n");
    sVec < idx > ind;ind.resize(importance.dim());
    sSort::sort(importance.dim(), &importance[0],  ind.ptr(0) );

    rslt.printf("index   ,");
    for( idx ic=0; ic<importance.dim() ; ++ic)
            rslt.printf("%10" DEC ",",ic+1);
    rslt.printf("\n");

    rslt.printf("column  ,");
    for( idx ic=0; ic<importance.dim() ; ++ic) {
        const char * ll=(const char * )cids->id(ind[importance.dim()-1-ic]);
        const char * rl=strchr(ll,'_');
        rslt.printf("%10s,",rl ? rl+1 : ll );
    }
    rslt.printf("\n");

    rslt.printf("coeff   ,");
    for( idx ic=0; ic<classifier.dim() ; ++ic)
        rslt.printf("%+10.5lf,",classifier[ind[classifier.dim()-1-ic]]/maxIter);
    rslt.printf("\n");

    rslt.printf("contrib ,");
    for( idx ic=0; ic<importance.dim() ; ++ic)
        rslt.printf("%10.5lf,",100*importance[ind[importance.dim()-1-ic]]/maxIter);
    rslt.printf("\n");

    rslt.printf("total   ,");
    real tot=0;
    for( idx ic=0; ic<importance.dim() ; ++ic)   {
        tot+=100*importance[ind[importance.dim()-1-ic]]/maxIter;
        rslt.printf("%10.5lf,",tot);
    }
    rslt.printf("\n");


    rslt.printf("\n\nBootstrapped sample belongness\n");
    rslt.printf("    sample");for( idx ic=0; ic<catSet.dim(); ++ic) rslt.printf(",%5s",(const char * )catSet.id(ic));
    rslt.printf(", confidence,  prediction\n");
    for( idx ir=0; ir<mat.rows(); ++ir) {
        rslt.printf("%10s  ",(const char * )rids->id(ir));
        idx imax=0;
        for( idx ic=0; ic<belong[ir].dim(); ++ic) {
            rslt.printf(",%05" DEC,belong[ir][ic]);
            if(belong[ir][ic]>belong[ir][imax])imax=ic;
        }
        rslt.printf(",%4" DEC "%%,%10s",(idx)(belong[ir][imax]*100/maxIter),(const char * )catSet.id(imax));

        if(checkSet.dim()) {
            idx il,ii;
            for(il=0; il<checkSet.dim(); ++il) {
                for(ii=0; ii<checkSet[il].dim(); ++ii) {
                     if(ir==checkSet[il][ii])break;
                }
                if(ii>=checkSet[il].dim())continue;

                rslt.printf(" %10s",(const char * )checkSet.id(il));
                if(strcmp((const char *)checkSet.id(il),(const char *)catSet.id(imax))==0)
                    rslt.printf(" ++");
                else rslt.printf(" --");
                break;
            }
        }
        rslt.printf("\n");

    }

    if(rslfCSV)rslfCSV->printf("%s",rslt.ptr());
    gLog->printf("%s",rslt.ptr());


    return 1;
}








#define PRINTING_THRESHOLD 9/10

idx sRlda::prepareComputeExtraLarge(sMatrix & OriMat, idx rowsToUseDicDim, idx cntNonZerosMinMax)
{
    samplingSetSize=OriMat.cols();


    FDAvariablePool.init(sMex::fSetZero);
    cntFDAList=sIdxMax;
    sVec <short> FDAvarCls;


    if(!pFDAfuncList) {
        pFDAfuncList=FDAfuncList.ptr() ;
        cntFDAList=FDAfuncList.dim();
    }
    idx totFDAspace=0;
    if(pFDAfuncList && cntFDAList) {
        idx iCls;
        for( iCls=0; iCls<cntFDAList; ++iCls){

            idx myUniverse =
                (pFDAfuncList[iCls].universeScaling > 0) ? pFDAfuncList[iCls].universeScaling :
                (pFDAfuncList[iCls].universeScaling==-2.5) ? (samplingSetSize*(samplingSetSize+1)/2) :
                pow(samplingSetSize,-pFDAfuncList[iCls].universeScaling);
            myUniverse *= pFDAfuncList[iCls].universe;

            if( !myUniverse )
                break;

            if( pFDAfuncList[iCls].varSize ) {
                pFDAfuncList[iCls].classVarOfs=FDAvariablePool.add(0,sizeof(idx)*myUniverse*pFDAfuncList[iCls].varSize);
            } else pFDAfuncList[iCls].classVarOfs=sNotIdx;
            pFDAfuncList[iCls].universeBase=totFDAspace;

            if( pFDAfuncList[iCls].initCall ){
                for( idx i=0; i<myUniverse; ++i ) {
                    pFDAfuncList[iCls].initCall( pFDAfuncList+iCls, i,FDAvariablePool.ptr(pFDAfuncList[iCls].classVarOfs));
                }

            }
            totFDAspace+=myUniverse;
        }
        cntFDAList=iCls;
        samplingSetSize=totFDAspace;
    }


    sMex measureNonZeros;
    char * nonZ=0;

    if(cntNonZerosMinMax!=0) {
        idx cntNonZerosMax=cntNonZerosMinMax>>32;
        idx cntNonZerosMin=cntNonZerosMinMax&0xFFFFFFFF;


        measureNonZeros.add(0,OriMat.cols()/8+1);
        measureNonZeros.set(0);
        nonZ=(char*)measureNonZeros.ptr(0);


        for(idx icol=0; icol<OriMat.cols()  ; ++icol ) {
            idx num=0;
            for(idx irow=0; irow<OriMat.rows()  ; ++irow) {
                    if(OriMat.val(irow,icol)!=0)
                        ++num;
            }
            if(num>=cntNonZerosMin && num<=cntNonZerosMax) {
                nonZ[icol/8]|=1<<(icol%8);
            }

        }
    }


    samplingSet.resize(samplingSetSize);

    samplingSetDimAvailable=samplingSetSize;
    for( idx irnd=0; irnd<samplingSet.dim() ; ++irnd ) {
        samplingSet[irnd]=irnd;
        if( nonZ && !(nonZ[irnd/8]&(1<<(irnd%8))) ) {
            samplingSet[irnd]*=-1;
            --samplingSetDimAvailable;
        }

    }
    samplingLdaTransforVecCumulator.resize(samplingSetSize,rowsToUseDicDim-1);samplingLdaTransforVecCumulator.set(0);
    samplingLdaTransforValCumulator.resize(samplingSetSize);samplingLdaTransforValCumulator.set(0);
    samplingLdaTransforValCumulatorOrder.resize(samplingSetSize);samplingLdaTransforValCumulatorOrder.set(0);
    samplingLdaTransforOccurence.resize(samplingSetSize+2);samplingLdaTransforOccurence.set(0);


    totalVals.resize(rowsToUseDicDim-1);
    totalItersRun=0;

    return cntFDAList;
}

idx sRlda::computeExtraLarge(sMatrix & OriMat,  sDic < sVec < idx > > & rowsToUseDic, idx cntSample, idx topChoiceForFinal, idx iterMax, idx iterMin,real shannonThreshold, idx goodShannonMaxThreshold, bool useEValScaling,real KTemperature, real radioactiveDecay, idx maxMissFire)
{



    iterMax=iterMax*samplingSetSize/OriMat.cols();




    idx goodShannonStableCount=0;
    goodShannonStableCount=0;

    curShannon=0;
    if(!maxMissFire)
        maxMissFire=cntSample/2;




    idx iter, iboot;
    for( iter=0, iboot=0; iter<iterMax; ++iter, ++totalItersRun ) {


        if(callbackProgress) {
            callbackProgress(callbackParam,iter,iter,iterMax);
        }


        for( idx irnd=0,randexp, missfire=0; irnd<cntSample; ++irnd ) {

            bool canreplace=true;
            if(KTemperature) {
                randexp=sRand::rand_xorshf96()&0xFF;
                real e=0;
                for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar){
                    e+=samplingLdaTransforVecCumulator(samplingSet[irnd],ivar);
                }
                if( KTemperature*(5.5412635451584261462455391880218-::log(randexp))<e)
                    canreplace=false;
            }
            if(radioactiveDecay){
                randexp=sRand::rand_xorshf96()&0xFF;
                idx lifetime=samplingLdaTransforOccurence[samplingSet[irnd]];
                if( radioactiveDecay*(8-1.4426950408889634073599246810019*::log(randexp))>lifetime)
                    canreplace=false;
            }

            if(canreplace==false && missfire<maxMissFire) {
                ++missfire;
                continue;
            }

            idx choice;
            idx t=-1;
            while (t<0) {
                choice=sRand::rand_xorshf96()%samplingSetSize;
                if(choice==irnd) {++choice;}
                t=samplingSet[choice];
            }
            samplingSet[choice]=samplingSet[irnd];
            samplingSet[irnd]=t;
        }

        for( idx irnd=0; irnd<cntSample; ++irnd ) {
            ++samplingLdaTransforOccurence[samplingSet[irnd]];
        }

        if(iter && (iter%50)==0) {
            real sumPLogP=0, totSamp=((totalItersRun+1)*cntSample);
            for( idx irnd=0; irnd<samplingSet.dim() ; ++irnd ) {
                idx occ=samplingLdaTransforOccurence[irnd];
                if(!occ)
                    continue;
                real p=occ/totSamp;
                sumPLogP+=-p*(::log(p));
            }
            curShannon=sumPLogP/::log(samplingSetDimAvailable );
            samplingLdaTransforOccurence[samplingSetSize]=totalItersRun+1;
            samplingLdaTransforOccurence[samplingSetSize+1]=totSamp;

            if(curShannon>1-shannonThreshold)
                ++goodShannonStableCount;

        }

        samplingSet.cut(cntSample);
        SubMatrix.empty();



        if(pFDAfuncList) {
            SubMatrix.resize(OriMat.rows(),samplingSet.dim());
            for (idx is=0; is<cntSample ; ++is) {
                idx icol=samplingSet[is];
                idx icls;for( icls=1; icls<cntFDAList && icol>=pFDAfuncList[icls].universeBase ; ++icls ) {}--icls;
                FDAStruc * pF=pFDAfuncList+icls;
                void * variables=pF->varSize ? FDAvariablePool.ptr(icol-pF->universeBase) :0;
                for( idx ir=0; ir<OriMat.rows() ; ++ir ) {
                    real val;
                    if(pF->funcCall)val = pF->funcCall(&OriMat,ir,icol-pF->universeBase, pF,variables, 0);
                    else val=OriMat.val(ir,icol-pF->universeBase);
                    SubMatrix[ir][is]=val;
                }
            }
        }
        else OriMat.extractColset(SubMatrix, samplingSet) ;


        samplingSet.resize(samplingSetSize);



        if( bootStrapCounter && iter && (iter%bootStrapCounter)==0 ) {

            for(idx it=0;it<rowsToUseDic.dim(); ++it) {
                sVec < idx > * cs=rowsToUseDic.ptr(it);
                idx frac=(udx)(cs->dim()*bootStrapFraction);
                for(idx ir=0;ir<cs->dim(); ++ir) {
                    idx * p=cs->ptr(ir);
                    bool isExcl=(sRand::rand_xorshf96()%cs->dim())>(udx)frac ? true : false ;
                    if(*p>=0 && isExcl)*p=-(*p)-1;
                    if(*p<0 && !isExcl)*p=-(*p)-1 ;
                }
            }
            ++iboot;
        }

        if(PCAMode)SubMatrix.pca(ldaTransformVals, ldaTransformVecs);
        else compute(SubMatrix, rowsToUseDic);




        for( idx icol=0; icol<cntSample; ++icol ){

            idx iRealCol=samplingSet[icol];
            for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar){

                real ev=ldaTransformVals[ivar];ev=ev*ev;
                if(icol==0){
                    samplingLdaTransforValCumulator[ivar]+=ev;
                }

                real v=ldaTransformVecs.val(icol,ivar);
                if(useEValScaling) {
                    v=v*v;
                    v*=ev;
                }else {
                    idx cntSmallerThanMe=0;
                    for( idx isor=0; isor< cntSample; ++isor) {
                        if(sAbs(v)>=sAbs(ldaTransformVecs.val(isor,ivar)) )
                            ++cntSmallerThanMe;
                    }
                    v=cntSmallerThanMe;
                }
                samplingLdaTransforVecCumulator.val(iRealCol,ivar)+=v;
            }
        }






    #ifdef DONOTCOMPILE
        if(iter%10000==0  && iter)
        {
            ::printf("iteration %" DEC "/%" DEC "/%" DEC " bootstrap %" DEC "/%" DEC " convergence shannon=%lf sampling set %" DEC "/%" DEC " \n",iter, iterMax, totalItersRun,iboot,bootStrapCounter, curShannon, cntSample,samplingSetSize);


        }

    #endif

        if(iter>=iterMin && goodShannonStableCount>=goodShannonMaxThreshold)
            break;


    }


    totalVals.set(0);
    samplingLdaTransforValCumulatorOrder.set(0);

    #ifdef DONOTCOMPILE
    for(idx iii=0; iii<samplingSetSize; ++iii){
        idx occ=samplingLdaTransforOccurence[iii];
        if(!occ){continue;}

        for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar){
            real ave=samplingLdaTransforVecCumulator.val(iii,ivar) / occ;
            totalVals[ivar]+=ave;
        }
    }
    #endif

    for(idx iii=0; iii<samplingSetSize; ++iii){
        idx occ=samplingLdaTransforOccurence[iii];
        if(!occ){continue;}

        for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar){
            real ave=samplingLdaTransforVecCumulator.val(iii,ivar) / occ;
            if(ave>(-samplingLdaTransforValCumulatorOrder[iii])){
                samplingLdaTransforValCumulatorOrder[iii]=-ave;
            }
        }
    }

    for(idx it=0;it<rowsToUseDic.dim(); ++it) {
        sVec < idx > * cs=rowsToUseDic.ptr(it);
        for(idx ir=0;ir<cs->dim(); ++ir) {
            idx * p=cs->ptr(ir);
            if(*p<0)*p=-(*p)-1 ;
        }
    }









    if(topChoiceForFinal) {
        finalCompute( OriMat,  rowsToUseDic, cntSample, topChoiceForFinal);
    }


    return (goodShannonStableCount>=goodShannonMaxThreshold) ? 1 : 0;

}

void sRlda::finalCompute(sMatrix & OriMat,  sDic < sVec < idx > > & rowsToUseDic, idx cntSample, idx topChoiceForFinal, bool onlyExtract)
{
        sSort::sort(samplingLdaTransforValCumulatorOrder.dim(),samplingLdaTransforValCumulatorOrder.ptr(), samplingSet.ptr() );

        if(topChoiceForFinal>samplingSet.dim())
            topChoiceForFinal=samplingSet.dim();
        samplingSet.cut(topChoiceForFinal);

        SubMatrix.empty();
        if(pFDAfuncList) {
            cntSample=samplingSet.dim();
            SubMatrix.resize(OriMat.rows(),cntSample);
            for (idx is=0; is<cntSample ; ++is) {
                idx icol=samplingSet[is];
                idx icls;for( icls=1; icls<cntFDAList && icol>=pFDAfuncList[icls].universeBase ; ++icls ) {}--icls;
                FDAStruc * pF=pFDAfuncList+icls;
                void * variables=FDAvariablePool.ptr(icol-pF->universe);
                for( idx ir=0; ir<OriMat.rows() ; ++ir ) {
                    real val ;
                    if(pF->funcCall) val= pF->funcCall(&OriMat,ir,icol-pF->universeBase, pF,variables, 0);
                    else val=OriMat.val(ir,icol-pF->universeBase);
                    SubMatrix[ir][is]=val;
                }
            }
        }
        else OriMat.extractColset(SubMatrix, samplingSet) ;

        if(!onlyExtract)
            compute(SubMatrix, rowsToUseDic);

}










