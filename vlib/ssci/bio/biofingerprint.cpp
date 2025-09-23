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
#include <slib/core/perf.hpp>
#include <slib/utils/sort.hpp>
#include <ssci/bio/biofingerprint.hpp>
#include <math.h>

#define RAND1 (real)rand()/RAND_MAX
#define RANDR(a1, a2) ((((real)rand()/RAND_MAX) * (a2 - a1)) + a1)

using namespace slib;

sBioFingerPrint * sBioFingerPrint::init(sBioseq * s, idx start, idx cnt)
{
    hdr->version = 1;
    hdr->chromCnt = cnt;

    return this;
}

idx sBioFingerPrint::compileChromosome(idx chrID, idx generationNum)
{
    idx seqlen = Sub->len(chrID);

    ChromInfo *chr1 = addChromosome(chrID, seqlen);

    idx seed0 = time(NULL);
    srand(seed0);
    initPopulation(chr1, 1);
    sStr buf;
    ::printf("seed0 is %" DEC, seed0);
    ::printf("\nAnalyzing patterns \n");

    sVec<idx> sortList;
    sortList.resize(hdr->numPatterns * 2);
    for(idx i = 0; i < hdr->numPatterns * 2; ++i) {
        sortList[i] = i;
    }
    for(idx i = 0; i < hdr->numPatterns; ++i) {
        bitC[i] = populateTable(chr1, i);
        buf.cut(0);
        getPatRepresentation(&buf, chr1, i);
        ::printf("%" DEC " - %" DEC ", %" DEC ", %s\n", i, bitC[i], countBits(chr1, i), buf.ptr(0));
    }

    bitSetCount64(7);
    generateNewSolution(chr1, true, &sortList);
    calcPopFitness(chr1, &sortList);
    ::printf("\nSorted List of n= %" DEC " \n", sortList.dim());
    sortPopulation(chr1, &sortList);

    printPopulation(0, chr1, &sortList, 0, hdr->numPatterns * 2);

    for(idx gen = 0; gen < generationNum; ++gen) {
        ::printf("\nGeneration No. %" DEC "\n", gen + 1);

        generateNewPopulation(chr1, sortList.ptr(0));

        calcPopFitness(chr1, &sortList);
        sortPopulation(chr1, &sortList);

        printPopulation(0, chr1, &sortList, 0, hdr->numPatterns * 2);

    }

    selectBest(chr1, sortList.ptr(), hdr->numPatterns, hdr->numPatterns * 2);
    PERF_PRINT();

    return 0;
}

sBioFingerPrint::ChromInfo * sBioFingerPrint::addChromosome(idx chrID, idx seqlen)
{
    ChromInfo *c0 = chromList.add();
    idx chr1offset = tFl.length();
    Chromosome *chr1 = (Chromosome *) (tFl.add(0, sizeof(Chromosome)));
    chr1->chromosomeID = chrID;
    chr1->_tableOffset = container.dim();
    chr1->startCompile = 0;
    chr1->cntCompile = seqlen;
    numOfSegs = segsCount(chr1);
    container.add(2 * hdr->numPatterns * ((numOfSegs - 1) / 64 + 1));
    contPointer = container.ptr();


    udx *i1 = (udx *) tFl.add(0, 2 * sizeof(udx) * hdr->numPatterns * 2);

    c0->chr0 = (Chromosome *) (chr1offset + tFl.ptr(0));
    c0->pattern = i1;

    idx n = 2 * hdr->numPatterns;
    distMatrix.resize((n * (n - 1)) / 2);
    bitC.resize(n);
    c0->bitsCountOffset = bitC.ptr();
    return c0;
}

idx sBioFingerPrint::printPopulation(sStr *buf, ChromInfo *chr, sVec<idx> *listPop, idx start, idx cnt)
{
    if (!buf){::printf("\n");}
    idx toPrint;
    if (!cnt){
        cnt = hdr->numPatterns;
    }
    for(idx i = start; i < start + cnt; ++i) {
        toPrint = i;
        if (listPop){
            toPrint = *listPop->ptr(i);
        }
        printIndividual(buf, chr, toPrint );
    }
    return 0;
}

idx sBioFingerPrint::printIndividual(sStr *dest, ChromInfo *chr, idx ind)
{
    sStr buf;
    buf.cut(0);
    getPatRepresentation(&buf, chr, ind);
    udx *ipat = getPattern(chr, ind);
    idx fitness = getFitness (chr, ind);
    if (dest){
        dest->printf("fitness: %" DEC ", %" DEC " - %" DEC ", %" DEC ", %s (%" DEC "-mer)\n", fitness, ind, chr->bitsCountOffset[ind], chr->bitsCountOffset[ind] * 100 / segsCount(chr->chr0), buf.ptr(0), bitSetCount64(ipat[1]) / 2);
    }
    else {
        ::printf("fitness: %" DEC ", %" DEC " - %" DEC ", %" DEC ", %s (%" DEC "-mer)\n", fitness, ind, bitC[ind], countBits(chr, ind) * 100 / segsCount(chr->chr0), buf.ptr(0), bitSetCount64(ipat[1]) / 2);
    }
    return 0;

}
idx sBioFingerPrint::generateNewPopulation(ChromInfo *chr, idx * listPop)
{
    idx popNum = hdr->numPatterns;
    ::printf("Analyzing selection \n");
    for(idx i = 0; i < popNum; i += 2) {
        idx p1 = selection(popNum, listPop);
        idx p2 = p1;

        while( p1 == p2 ) {
            p2 = selection(popNum, listPop);
        }

        udx *parent1 = getPattern(chr, listPop[p1]);
        udx *parent2 = getPattern(chr, listPop[p2]);
        idx ch1 = listPop[popNum + i];
        idx ch2 = listPop[popNum + i + 1];

        udx *child1 = getPattern(chr, ch1);
        udx *child2 = getPattern(chr, ch2);

        ::printf("\n\nParents: \n");
        printIndividual(0, chr, listPop[p1]);
        printIndividual(0, chr, listPop[p2]);

        recombination(parent1, parent2, child1, child2);

        mutate(child1, 0.9);
        mutate(child2, 0.9);
        bitC[ch1] = populateTable(chr, ch1);
        bitC[ch2] = populateTable(chr, ch2);

        ::printf("\nAfter recombination \n");
        printIndividual(0, chr, ch1);
        printIndividual(0, chr, ch2);
    }
    return 0;
}

idx sBioFingerPrint::selection(idx popNum, idx *listPop)
{
    idx p1 = RANDR(0, popNum);
    idx p2 = p1;
    while( p1 == p2 ) {
        p2 = RANDR(0, popNum);
    }

    idx comp = bioFingerComparator(this, 0, listPop[p1], listPop[p2]);
    if( comp < 0 ) {
        return p1;
    }
    return p2;
}

idx sBioFingerPrint::recombination(udx *parent1, udx *parent2, udx *child1, udx *child2)
{

    child1[0] = parent1[0];
    child1[1] = parent1[1];
    child2[0] = parent2[0];
    child2[1] = parent2[1];
    for(idx pos = 0; pos < hdr->lenPatterns; ++pos) {
        if( RAND1 < 0.5 ) {
            udx bmask = ((udx)0x3 << pos * 2);
            child1[0] = (child1[0] & ~bmask) | (parent2[0] & bmask);
            child1[1] = (child1[1] & ~bmask) | (parent2[1] & bmask);
            child2[0] = (child2[0] & ~bmask) | (parent1[0] & bmask);
            child2[1] = (child2[1] & ~bmask) | (parent1[1] & bmask);
        }
    }
    return 0;
}

idx sBioFingerPrint::mutate(udx *individual, real pmut)
{
    udx mutmask = 0;
    if( RAND1 < pmut ) {
        idx pos = RANDR(0, hdr->lenPatterns);
        mutmask = (udx)0x3 << pos * 2;
        udx option = ((idx) RANDR(0, 4) ) << pos * 2;
        if (mutmask & individual[1]){
            if ((individual[0] & mutmask) == option ){
                individual[0] &= ~mutmask;
                individual[1] &= ~mutmask;
            }
            else {
                individual[0] = (individual[0] & ~mutmask) | (individual[0] & option);
            }
        }
        else {
            individual[0] |= option;
            individual[1] |= mutmask;
        }
        return 1;
    }
    return 0;
}

idx sBioFingerPrint::generateNewSolution(ChromInfo *chr, bool randomgenerated, sVec<idx> * listPop)
{
    sStr buf;
    for(idx i = hdr->numPatterns; i < 2 * hdr->numPatterns; ++i) {
        udx *ipat = getPattern(chr, i);
        if( i == hdr->numPatterns / 2 ) {
            ::printf("Random sequences\n");
        }
        if( randomgenerated == true ) {
            initIndividual(chr, ipat, true);
            ipat[1] = initMask(hdr->cntbitmask);
            for(idx j = 0; j < hdr->sizeofPattern; ++j) {
                ipat[j] &= ipat[j + hdr->sizeofPattern];
            }
        } else {
        }
        bitC[i] = populateTable(chr, i);

        buf.cut(0);
        getPatRepresentation(&buf, chr, i);
        ::printf("%" DEC ", %" DEC ", %s\n", bitC[i], countBits(chr, i), buf.ptr(0));

    }
    return 0;
}

idx sBioFingerPrint::calcDistance(udx *i1, udx *i2, idx len)
{
    idx dist = 0;
    for(idx i = 0; i < len; ++i) {
        dist += bitSetCount64(i1[i] & i2[i]);
    }
    return dist;
}

idx sBioFingerPrint::calcPopFitness(ChromInfo *chr, sVec<idx> * listPop)
{
    udx *i1, *i2;
    idx count = listPop->dim();
    if( count == 0 ) {
        return 0;
    }
    idx len = ((numOfSegs - 1) / 64 + 1);
    idx n = 2 * hdr->numPatterns;
    idx index;
    ::printf("\nNumber of segments that each Pattern hit\n");
    for(idx i = 0; i < count; ++i) {
        ::printf(" %" DEC, bitC[i]);
    }
    for(idx i = 0; i < count; ++i) {
        i1 = getRowTable(chr, i);
        for(idx j = i + 1; j < count; ++j) {
            i2 = getRowTable(chr, j);
            index = (j * (2 * n - j - 1)) / 2 + (i - j - 1);
            distMatrix[index] = calcDistance(i1, i2, len);
        }
    }
    return 0;
}

real sBioFingerPrint::getFitness(ChromInfo *chr, idx ind)
{
    real a, b, c;

    real x = (real) chr->bitsCountOffset[ind] / segsCount(chr->chr0);
    if (x < 0.87){
        a = -132.11;
        b = 229;
        c = 0;
    }
    else {
        a = -5917.15;
        b = 0.4394;
        c = 1.2506;
    }
    x = x - 0.87;
    b = 0;
    c = 100;
    real fitness = a * (x * x) + b * x + c;

    return fitness;
}

idx sBioFingerPrint::bioFingerComparator(sBioFingerPrint * myThis, void * arr, udx i1, udx i2)
{
    ChromInfo *c0 = myThis->chromList.ptr(myThis->chrToSort);
    real fit1 = myThis->getFitness(c0, i1);
    real fit2 = myThis->getFitness(c0, i2);

    if( fit1 > fit2 ) {
        return -1;
    } else if( fit1 < fit2 ) {
        return 1;
    }
    return 1;
}

idx sBioFingerPrint::sortPopulation(ChromInfo *chr, sVec<idx> * sortList)
{

    idx len = ((numOfSegs - 1) / 64 + 1);

    chrToSort = chr - chromList.ptr(0);
    lenOfSegs = len;
    sSort::sortSimpleCallback((sSort::sCallbackSorterSimple) bioFingerComparator, (void *) this, bitC.dim(), bitC.ptr(0), sortList->ptr(0));
    return 0;
}

idx sBioFingerPrint::getPatRepresentation(sStr *buf, ChromInfo *chr, idx num)
{
    udx *ipat = getPattern(chr, num);
    sBioseq::uncompressATGC_2Bit(buf, (char *) ipat, 0, hdr->lenPatterns);

    for(idx pos = 0; pos < hdr->lenPatterns; ++pos) {
        if( (ipat[1] & ((udx) 0x1 << (pos * 2))) == 0 ) {
            *buf->ptr(pos) = '-';
        }
    }
    return 0;
}

idx sBioFingerPrint::initIndividual(ChromInfo *chr, udx * ind, bool useBioseq)
{
    idx pos = 0;
    if( useBioseq && Sub->len(chr->chr0->chromosomeID) ) {
        pos = chr->chr0->startCompile + RANDR(0, chr->chr0->cntCompile - 32);
        idx *binseq = (idx *) Sub->seq(chr->chr0->chromosomeID);
        idx lenseq = Sub->len(chr->chr0->chromosomeID);
        ind[0] = read32letters(pos, binseq, lenseq);
    } else {
        ind[0] = (udx) rand() * RAND_MAX + rand();
    }
    return pos;
}

udx sBioFingerPrint::initMask(idx num)
{
    idx array[hdr->lenPatterns];
    idx i = hdr->lenPatterns;
    idx j;
    udx x = 0;

    for(idx is = 0; is < i; ++is) {
        array[is] = is;
    }
    while( i > 1 ) {
        i--;
        j = RANDR(0, i);
        idx t = array[j];
        array[j] = array[i];
        array[i] = t;
    }

    for(idx is = 0; is < num; ++is) {
        x |= (udx) (0x3) << (array[is] * 2);
    }
    return x;
}

idx sBioFingerPrint::initPopulation(ChromInfo *chr, idx num)
{
    sStr buf;
    bool norandom = true;
    for(idx i = 0; i < hdr->numPatterns; ++i) {
        udx *ipat = getPattern(chr, i);
        if( i == hdr->numPatterns / 2 ) {
            ::printf("Random sequences\n");
            norandom = false;
        }
        idx pos = initIndividual(chr, ipat, norandom);
        ipat[1] = initMask(hdr->cntbitmask);
        for(idx j = 0; j < hdr->sizeofPattern; ++j) {
            ipat[j] &= ipat[j + hdr->sizeofPattern];
        }
        buf.cut(0);
        getPatRepresentation(&buf, chr, i);
        ::printf("%" DEC "  :\t  %s\n", pos, buf.ptr(0));
    }
    return 0;
}

idx sBioFingerPrint::bitSetCount64(idx i)
{
    i = i - ((i >> 1) & 0x5555555555555555);
    i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
    i = ((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F);
    return (i * (0x0101010101010101)) >> 56;
}

idx sBioFingerPrint::countBits(ChromInfo *chr, idx num)
{
    Chromosome *chr1 = chr->chr0;
    idx sgsCount = segsCount(chr1);
    udx *res0 = getRowTable(chr, num);
    idx resLen = ((sgsCount - 1) / 64 + 1);
    idx count = 0;
    for(idx isegidx = 0; isegidx < resLen; ++isegidx) {
        count += bitSetCount64(res0[isegidx]);
    }

    return count;
}

bool sBioFingerPrint::findStringPattern(udx *pat, const char * seq, idx seqlen)
{
    udx p = read32letters(0, (idx *) seq, seqlen);
    bool foundPat = false;
    idx is = 32;
    do {
        if( pat[0] == (p & pat[1]) ) {
            foundPat = true;
        }
        p >>= 2;
        idx ibyte = is / 4;
        idx ishift = (is % 4) * 2;
        idx val = (idx) ((seq[ibyte] >> ishift) & 0x3);

        p |= (val << 62);
        is++;

    }while (!foundPat && is < seqlen);

    return foundPat;
}

idx sBioFingerPrint::populateTable(ChromInfo *chr0, idx num)
{
    Chromosome *chr1 = chr0->chr0;

    udx *pat = getPattern(chr0, num);
    udx *res0 = getRowTable(chr0, num);
    idx startcompile = chr1->startCompile;
    const char *seq = Sub->seq(chr1->chromosomeID);
    idx bitCount = 0;

    for(idx iseg = 0; iseg < segsCount(chr1); ++iseg) {
        idx is = startcompile + (iseg * hdr->segmentSize);
        bool foundPat = false;
        udx p = read32letters(is, (idx *) seq, chr1->cntCompile);
        is += 32;

        do {

            if( pat[0] == (p & pat[1]) ) {
                foundPat = true;
            }

            p >>= 2;
            idx ibyte = is / 4;
            idx ishift = (is % 4) * 2;
            idx val = (idx) ((seq[ibyte] >> ishift) & 0x3);

            p |= (val << 62);
            is++;

        } while( !foundPat && is < startcompile + ((iseg + 1) * hdr->segmentSize) );
        if( foundPat ) {
            idx ibyte = iseg / 64;
            idx ishift = iseg % 64;
            res0[ibyte] |= (udx) (0x1) << ishift;
            ++(bitCount);
        }
        else {
            idx ibyte = iseg / 64;
            idx ishift = iseg % 64;
            res0[ibyte] &= ~((udx)(0x1) << ishift);
        }
    }
    return bitCount;
}

idx sBioFingerPrint::moveIndividual (ChromInfo *chr, idx dst, idx src)
{
    udx *ipatdst = getPattern(chr, dst);
    udx *ipatsrc = getPattern(chr, src);
    ipatdst[0] = ipatsrc[0];
    ipatdst[1] = ipatsrc[1];

    udx *resdst = getRowTable(chr, dst);
    udx *ressrc = getRowTable(chr, src);
    idx resLen = ((segsCount(chr->chr0) - 1) / 64 + 1);
    for (idx i = 0; i < resLen; ++i){
        resdst[i] = ressrc[i];
    }
    chr->bitsCountOffset[dst] = chr->bitsCountOffset[src];
    return 0;
}

void sBioFingerPrint::selectBest(ChromInfo *chr, idx *sortList, idx best, idx total)
{
    bool isPresent;
    idx j, k=0;
    for (idx i = 0; i < best; ++i){
        isPresent = false;
        j = 0;
        while (!isPresent && (j < best)){
            if (sortList[j++] == i){
                isPresent = true;
            }
        }
        if (!isPresent){
            for(; sortList[k] < best; ++k);
            moveIndividual (chr, i, sortList[k++]);
        }
    }
}

void sBioFingerPrint::transpose32(udx *A)
{
    idx j, k;
    udx m, t;
    m = 0x0000FFFF;
    for(j = 16; j != 0; j = j >> 1, m = m ^ (m << j)) {
        for(k = 0; k < 32; k = (k + j + 1) & ~j) {
            t = (A[k] ^ (A[k + j] >> j)) & m;
            A[k] = A[k] ^ t;
            A[k + j] = A[k + j] ^ (t << j);
        }
    }
}

void sBioFingerPrint::transpose64(udx *A)
{
    udx j, k;
    udx m, t;
    m = 0xFFFFFFFF;
    for(j = 32; j != 0; j = j >> 1, m = m ^ (m << j)) {
        for(k = 0; k < 64; k = (k + j + 1) & ~j) {
            t = (A[k] ^ (A[k + j] >> j)) & m;
            A[k] = A[k] ^ t;
            A[k + j] = A[k + j] ^ (t << j);
        }
    }
}

idx sBioFingerPrint::translateTable (ChromInfo *chr, udx *pat_table, idx numSegs, idx lenPat)
{
    udx m64[64];
    idx i, j, k;
    idx np = ((hdr->numPatterns - 1) / 64 + 1);
    udx * paPointers[hdr->numPatterns];
    for (i = 0; i < hdr->numPatterns; ++i){
        paPointers[i] = getRowTable(chr, i);
    }

    for (i = 0; i < numSegs; ++i){
        for (k = 0; k < np; ++k){
            for (j = 0; j < 64 && j < hdr->numPatterns; ++j) {
                m64[j] = (paPointers[j+k*64])[i];
            }
            while (j < 64){
                m64[j++] = 0;
            }
            transpose64 (m64);
            for (j = 0; j < 64; ++j) {
                pat_table[i*np+j+(k)*64] = m64[j];
            }
        }
    }
    return 0;
}

idx sBioFingerPrint::finalize()
{

    hdr->chromCnt += 1;

    ChromInfo *chr = chromList(chromList.dim()-1);
    Chromosome *chsrc = chr->chr0;

    char *chra = (char *) (cFl->add(0, sizeof(Chromosome)));
    idx chroffset = chra - cFl->ptr(0);
    udx *patterns = (udx *) cFl->add(0, 2 * sizeof(udx) * hdr->numPatterns);

    Chromosome *chr1 = (Chromosome *)(cFl->ptr() + chroffset);
    chr1->chromosomeID = chsrc->chromosomeID;
    chr1->_tableOffset = c2Fl->length();
    chr1->startCompile = chsrc->startCompile;
    chr1->cntCompile = chsrc->cntCompile;

    idx * bitCountPointer = (idx *) cFl->add(0, sizeof(idx) * hdr->numPatterns);
    udx *pat_table = (udx *) c2Fl->add(0, numOfSegs * sizeof(udx) * ((hdr->numPatterns - 1) / 64 + 1));

    for (idx i = 0; i < hdr->numPatterns; ++i){
        udx *s = getPattern(chr, i);
        udx *d = patterns + (2 * i);
        d[0] = s[0];
        d[1] = s[1];
        bitCountPointer[i] = bitC[i];
    }
    translateTable (chr, pat_table, (numOfSegs - 1) / 64 + 1, 0);

    return 0;

}

idx sBioFingerPrint::readChromosomes()
{
    chromList.resize(hdr->chromCnt);
    idx offset = sizeof(Hdr);
    for (idx i = 0; i < hdr->chromCnt; ++i){
        chromList.ptr(i)->chr0 = (Chromosome *)(cFl->ptr(offset));
        chromList.ptr(i)->pattern = (udx *)(cFl->ptr(offset + sizeof(Chromosome)));
        idx numP = getNumPatterns(i);
        chromList.ptr(i)->bitsCountOffset = (idx *)(cFl->ptr(offset + sizeof(Chromosome) + 2 * sizeof(udx)*numP));
        offset += sizeof(Chromosome) + (3 * sizeof(udx)*numP);
    }

    return hdr->chromCnt;
}
