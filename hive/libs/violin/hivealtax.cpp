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
#include <ulib/ulib.hpp>
#include <violin/violin.hpp>
#include "common.hpp"

using namespace sviolin;

real sHivealtax::ShannonFunction(sDic<idx> * taxCnt, idx totalPopulation)
{
    if( taxCnt->dim() <= 1 )
        return 0;
    real totCnt = 0;
    for(idx i = 0; i < taxCnt->dim(); ++i) {
        totCnt += (*taxCnt)[i];
    }
    if( totCnt == 0 )
        return 0.0;

    real plogPSum = 0;
    for(idx i = 0; i < taxCnt->dim(); ++i) {
        real p = (*taxCnt)[i] / totCnt;
        if (p){
            //this log is LN log by neper value so we divide logx,2= ln x/ln 2
            plogPSum += -p * log(p);
        }
    }

    //plogPSum /= log(taxCnt->dim()); // TODO scale correctly

    //plogPSum=plogPSum/log(taxCnt->dim);
    real a = log(totalPopulation);
    plogPSum = plogPSum / a;
    return plogPSum;
}

real sHivealtax::calculateShannonEntropy(sDic<idx> *taxCnt, idx rankdim, idx totRankWeight, sStr * outReport)
{
    real shEntropy = 0.0;
    sVec<idx> previoustaxCnt;
    sVec<idx> countRank;
    sStr outString;
    countRank.cut(0);
    countRank.vadd(rankdim, 0);
    /// ttt is showing either to the new taxCnt or to the previous one
    sDic<idx> * ttt = taxCnt;
    if( totRankWeight == 0 ) {
        totRankWeight = 1;
    }

    // from here beyond in these loops we are agnostic to what dictionary is used for our computations
    // all we care that ttt has taxCnt

    sDic<idx> newTaxCntForThisRank;
    newTaxCntForThisRank.flagOn(sMex::fSetZero);

    outString.printf("%"DEC, iteration);
    // this loop is going over all ranks from species to kingdom
    for(idx ir = 0; ir < rankdim; ++ir) {

        if( rankList[ir].weight == 0 ) // for zeroed levels
            continue;

        sDic<idx> * whichTaxCountToUse = ir == 0 ? ttt : &newTaxCntForThisRank;

        // this computes shannon entropy for particular rank level
        real currentShannonForRank = ShannonFunction(whichTaxCountToUse, rankList[ir].totalPopulation);
        outString.printf(",%.3lf", currentShannonForRank);
        //and accumulate weighted value
        shEntropy += currentShannonForRank * (rankList[ir].weight / totRankWeight);

        newTaxCntForThisRank.empty();
        if( ir >= sDim(rankList) - 1 )
            break;
        RankedTaxCountFunction(newTaxCntForThisRank, ttt, rankList[ir + 1].rankName);
    }
    if( outReport ) {
        outReport->printf("%s,%.4lf\n", outString.ptr(0), (real) shEntropy * totRankWeight);
    }
    return shEntropy;
}

idx sHivealtax::RankedTaxCountFunction(sDic<idx> & dstTaxCnt, sDic<idx> * srcTaxCnt, const char * requiredRank)
{
    sStr taxCurrent;

    for(idx i = 0; i < srcTaxCnt->dim(); ++i) {
        taxCurrent.cut(0);
        idx taxId = filterByRank (&taxCurrent, (const char *)srcTaxCnt->id(i), requiredRank);
        if (taxId){
            idx * pCnt = dstTaxCnt.set(taxCurrent.ptr());
            *pCnt += (*srcTaxCnt)[i];
        }
    }
    return dstTaxCnt.dim();
}

idx sHivealtax::filterByRank(sStr *dstTax, const char * srcTax, const char * requiredRank)
{
    if(strcmp(requiredRank, "leaf") == 0){
        dstTax->addString(srcTax);
        return atoidx((const char *)srcTax);;
    }
    sStr taxParent, taxRank, taxCurrent;
    idx taxId;
    taxCurrent.addString(srcTax);
    do{
        taxParent.cut(0);
        taxRank.cut(0);
        taxion->getParentTaxIds(taxCurrent, &taxParent, &taxRank);
        taxId = atoidx((const char *)taxCurrent.ptr());
        if (taxParent.length() && (strcmp (taxRank.ptr(), requiredRank) == 0)){
            dstTax->addString(taxCurrent);
            return taxId;
        }
        taxCurrent.printf(0,"%s", taxParent.ptr());
    }while (taxId > 1);

    dstTax->printf(0,"NO_INFO");
    return 0;
}

idx sHivealtax::CurateResult(sDic<idx> *taxCnt, sHiveseq *Sub, sHiveal *hiveal, sDic<idx> *giCnt, const char *taxDepth, sTxtTbl *tbl, idx colNumber)
{
    sStr dstTax, tid, gi, acclist;
    idx taxCount = 0;
    idx cntResult = 1;

    bool useTblSource = false;

    if ( (!Sub && !hiveal) && tbl){
        useTblSource = true;
    }
    idx numRows = useTblSource ? tbl->rows() : hiveal->dimAl();

    for(idx i = 0; i < numRows; i++, ++taxCount) {

        if (PROGRESS(i, i, numRows)){
            return -1;
        }

        const char *seqId = 0;
        idx seqlen = 0;
        if (!useTblSource){
            sBioseqAlignment::Al * hdr = hiveal->getAl(i);
            seqId = Sub->id(hdr->idSub());
        }
        else {
            seqId = tbl->cell(i, colNumber, &seqlen);

        }
        tid.cut(0);
        gi.cut(0);
        acclist.cut(0);
        const char * taxid = taxion->extractTaxIDfromSeqID(&tid, &gi, &acclist, seqId, seqlen, "-1");

        if( taxid && *taxid) {
            dstTax.cut(0);

            filterByRank(&dstTax, taxid, taxDepth);

            //new or old taxid after filtration
            const char *currTax = (dstTax) ? dstTax.ptr() : taxid;

            idx * GIbuf = taxCnt->get(currTax);
            if( !GIbuf ) {
                GIbuf = taxCnt->set(currTax);
                *GIbuf = 0;
            };
            (*GIbuf) = (*GIbuf) + cntResult;

            // Add to the statistics
            stats * auxcensusStat = censusStats.set(currTax);
            auxcensusStat->localnum += cntResult;
            if( giCnt ) {
                idx * GIbuf2 = giCnt->get(acclist.ptr(),acclist.length());
                if( !GIbuf2 ) {
                    GIbuf2 = giCnt->set(acclist.ptr(),acclist.length());
                    *GIbuf2 = 0;
                };
                (*GIbuf2) = (*GIbuf2) + cntResult;

                stats * centeric = accCentericStats.set(acclist.ptr(),acclist.length());
                centeric->localnum += cntResult;
                centeric->family_taxid_pos = setTaxidContainer(currTax);
                centeric->leaf_taxid_pos = setTaxidContainer(taxid);
            }
        }
        else if (giCnt){
            // We have an accession Number that we couldn't recognize
            idx * GIbuf2 = giCnt->get(acclist.ptr(),acclist.length());
            if( !GIbuf2 ) {
                GIbuf2 = giCnt->set(acclist.ptr(),acclist.length());
                *GIbuf2 = 0;
                stats * centeric = accCentericStats.set(acclist.ptr(),acclist.length());
                centeric->localnum = 0;
                centeric->family_taxid_pos = setTaxidContainer("-1",2);
                centeric->leaf_taxid_pos = setTaxidContainer("-1",2);
            };
            (*GIbuf2) = (*GIbuf2) + cntResult;
        }
//        else {
//            ::printf("I'm losing this one: %s !!!", gi.ptr());
//        }
    }
    return taxCount;
}

real sHivealtax::analyzeResults(sFil * outTaxCnt, sDic<idx> *taxCnt=0, sFil * outSha)
{
    sStr shaFile;
    idx firstround;
    sDic<idx> previousTaxCnt;

    if( outTaxCnt->length() != 0 ) {
        firstround = 0;
        sTbl t;
        t.parse(outTaxCnt->ptr(), outTaxCnt->length(), 0);
        for(idx ir = 1; ir < t.rows(); ++ir) {
            sStr taxid, scnt;
            t.get(&taxid, ir, 0);
            t.get(&scnt, ir, 1);
            *previousTaxCnt.set(taxid) = atoidx(scnt);
        }
    } else if( outTaxCnt->length() == 0 ) {
        firstround = 1;
        shaFile.cut(0);
        shaFile.printf("iter,");
        for(idx i = 0; i < sDim(rankList); ++i) {
            shaFile.printf("%s,", rankList[i].rankName);
        }
        shaFile.printf("weighted_sum\n");
    }

    real howsignificantismychange = 0;
    idx ir, totRankWeight = 0;                //totalWeightedShannon=0;
    for(ir = 0; ir < NUM_RANKS; ++ir) {
        totRankWeight += rankList[ir].weight;
        rankList[ir].totalPopulation = 0;
    }

    rankList[0].totalPopulation = taxion->getRecordCount(0);
    for (idx ir = 1; ir < NUM_RANKS; ++ir) {
        rankList[ir].totalPopulation = taxion->getTaxCount(rankList[ir].rankName);
    }

    real compareShannons[2];
    if( firstround == 1 ) {
        compareShannons[0] = calculateShannonEntropy(taxCnt, sDim(rankList), totRankWeight, &shaFile);
        howsignificantismychange = 100;
    } else {
        compareShannons[0] = calculateShannonEntropy(taxCnt, sDim(rankList), totRankWeight, &shaFile);
        compareShannons[1] = calculateShannonEntropy(&previousTaxCnt, sDim(rankList), totRankWeight, 0);
        howsignificantismychange = compareShannons[1] - compareShannons[0];
    }

    if( howsignificantismychange < 0 ) {
        howsignificantismychange *= -1;
    }

    if( outSha ) {
        outSha->add(shaFile.ptr(), shaFile.length());
    }
    return howsignificantismychange;
}

idx sHivealtax::reportResults(sFil * outTaxCnt, sDic<idx> *taxCnt, sFil * outGiCnt, sDic<idx> *accCnt)
{
    sStr taxFile, accFile;

    if( taxCnt->dim() ) {
        idx cntTaxid = taxCnt->dim();
        //! filling an array to sort taxCnt
        sVec<idx> ind, array;

        ind.add(taxCnt->dim());
        array.add(taxCnt->dim());
        for(idx i = 0; i < taxCnt->dim(); ++i)
            array[i] = *taxCnt->ptr(i);

        sSort::sort<idx>(taxCnt->dim(), array, ind.ptr(0));

        taxFile.printf(0, "taxid,cnt,pct,min,max,mean,std,intval\n");
        //enough is a parameter to show thAT THE ITERATION IS ENOUGH
        idx countTotal = 0;
        for(idx i = 0; i < censusStats.dim(); ++i) {
            countTotal += censusStats.ptr(i)->sum;
        }
        for(idx i = cntTaxid - 1; i >= 0; --i) {

            stats * auxcensusStat = censusStats.set(taxCnt->id(ind[i]));
            sStr state_confidence_interval;
            real stderror;
            real confInterval;
            if (auxcensusStat->num != iteration * iterationNumber){
                auxcensusStat->min = 0;
                auxcensusStat->num = iteration * iterationNumber;
            }
            real stdev = Stats_stddev(auxcensusStat);
            real mean = Stats_mean(auxcensusStat);

            if( auxcensusStat->num > 1 ) { //lower than 30 sample it is not a distribution
                stderror = stdev / sqrt(auxcensusStat->num);
                confInterval = 1.96 * stderror; //confidence interval of 95%
                state_confidence_interval.printf("%0.3lf", confInterval);
            } else {
                confInterval = 0.0;
                state_confidence_interval.printf("%0.1lf", confInterval);
            }

            taxFile.printf("%s,%"DEC",%0.2lf,%"DEC",%"DEC",%0.2lf,%0.3lf,%s\n", (const char *) (taxCnt->id(ind[i])), (idx) auxcensusStat->sum, (real) (auxcensusStat->sum * 100) / countTotal, auxcensusStat->min,
                auxcensusStat->max, mean, stdev, state_confidence_interval.ptr());

        }
    }
    if( accCnt->dim() ) {
        idx cntaccid = accCnt->dim();

        sVec<idx> ind2, array2;

        ind2.add(accCnt->dim());
        array2.add(accCnt->dim());
        for(idx i = 0; i < accCnt->dim(); ++i)
            array2[i] = *accCnt->ptr(i);

        // sSort::sortCallback(sort_myCompare, 0, cntTaxid, &taxCnt, ind);
        sSort::sort<idx>(accCnt->dim(), array2, ind2.ptr(0));

        accFile.printf(0, "acclist,leaf_taxid,taxid,cnt,pct,min,max,mean,std,intval\n");
        //enough is a parameter to show thAT THE ITERATION IS ENOUGH
        idx countTotal = 0;
        for(idx i = 0; i < accCentericStats.dim(); ++i) {
            countTotal += accCentericStats.ptr(i)->sum;
        }

        sStr state_confidence_interval;
        idx acclen;
        for(idx i = cntaccid - 1; i >= 0; --i) {

            const char * accnum = (const char *) (accCnt->id(ind2[i], &acclen));

            stats * centeric = accCentericStats.get(accnum, acclen);
            if (!centeric){
                continue;
            }
            real stdev = Stats_stddev(centeric);
            real mean = Stats_mean(centeric);
            real stderror;
            real confInterval;
            if( centeric->num > 1 ) { //lower than 30 sample it is not a distribution
                stderror = stdev / sqrt(centeric->num);
                confInterval = 1.96 * stderror; //confidence interval of 95%

                state_confidence_interval.printf(0,"%0.3lf", confInterval);
            } else {
                confInterval = 0.0;
                state_confidence_interval.printf(0,"%0.1lf", confInterval);
            }
            const char *family_taxid = getTaxidContainer(centeric->family_taxid_pos);
            const char *leaf_taxid = getTaxidContainer(centeric->leaf_taxid_pos);
            accFile.printf("%.*s,%s,%s,%"DEC",%0.2lf,%"DEC",%"DEC",%0.2lf,%0.3lf,%s\n", (int)acclen,accnum, leaf_taxid, family_taxid, *accCnt->ptr(ind2[i]), (real) (centeric->sum * 100) / countTotal, centeric->min, centeric->max, mean, stdev, state_confidence_interval.ptr());
        }
    }
    if( outTaxCnt && taxFile) {
        outTaxCnt->cut(0);
        outTaxCnt->add(taxFile.ptr(), taxFile.length());
    }

    if( outGiCnt && accFile ) {
        outGiCnt->cut(0);
        outGiCnt->add(accFile.ptr(), accFile.length());
    }

    return 0;
}
