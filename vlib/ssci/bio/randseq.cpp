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
#include <slib/core/def.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/mex.hpp>
#include <slib/core/str.hpp>
#include <slib/core/var.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>
#include <ssci/bio/randseq.hpp>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define PROGRESS(items,cur,max,perc) (m_callback && !m_callback(m_callbackParam, items, ((cur) * 1.0 / (max)) * perc, 100))

using namespace slib;

idx sRandomSeq::validate(idx num, idx low, idx high)
{
    if( num >= low && (high < 0 ? true : num <= high) ) {
        return true;
    }
    return false;
}
bool sRandomSeq::validate(const char * str)
{
    if( sLen(str) != 0 ) {
        return true;
    }
    return false;
}

void sRandomSeq::translateRevComp(idx orientation, bool *isrev, bool *iscomp)
{
    switch(orientation) {
        case 0:
            *isrev = false;
            *iscomp = false;
            break;
        case 1:
            *isrev = true;
            *iscomp = false;
            break;
        case 2:
            *isrev = false;
            *iscomp = true;
            break;
        case 3:
            *isrev = true;
            *iscomp = true;
            break;
    }

}

idx sRandomSeq::preparePrimers()
{
    sStr temp;
    temp.add(primersString.ptr(0), primersString.length());
    primersString.cut(0);
    sString::searchAndReplaceSymbols(&primersString, temp, 0, ";" sString_symbolsEndline, 0, 0, true, true, false, false);
    primersCnt = sString::cnt00(primersString);
    return primersCnt;
}

idx sRandomSeq::prepareLowComplexity()
{
    sStr temp;
    temp.add(lowComplexityString.ptr(0), lowComplexityString.length());
    lowComplexityString.cut(0);
    sString::searchAndReplaceSymbols(&lowComplexityString, temp, 0, ";" sString_symbolsEndline, 0, 0, true, true, false, false);
    lowComplexityCnt = sString::cnt00(lowComplexityString);
    return lowComplexityCnt;
}

void sRandomSeq::applyNoise(sStr &buf, idx buflen)
{
    char *let = buf.ptr(0);
    real randNum;
    idx j;
    for(idx i = 0; i < buflen; ++i) {
        if( flipCoin(noisePercentage, 100) ) {
            randNum = randDist(0, 100);
            idx row = sBioseq::mapATGC[(idx) let[i]];
            for(j = 0; j < 4; j++) {
                if( (randNum < noiseAccumTable[row][j]) ) {
                    break;
                }
            }
            if( (j < 4) && (row != j) ) {
                let[i] = sBioseq::mapRevATGC[j];
            } else if( j == 4 ) {
            } else if( j == 5 ) {
            }
        }
    }
}

bool sRandomSeq::addPrimers(sStr &buf, idx buflen)
{
    if( flipCoin(primersFreq, 100) ) {
        idx getPrimerNum = randDist(0, primersCnt);
        const char *primerS = sString::next00(primersString.ptr(), getPrimerNum);
        if( !primerS ) {
            return false;
        }
        idx prLen = sLen(primerS);
        idx primDist = randDist(primersMin, prLen);
        if( primDist > buflen ) {
            primDist = buflen;
        }
        idx primStart = randDist(0, prLen - primDist);
        if( (primDist + primStart) > buflen ) {
            primDist = buflen - primStart;
        }
        strncpy(buf.ptr(0), primerS + primStart, primDist);
        return true;
    }
    return false;
}


int sRandomSeq::applySortMutations(sStr &buf, sStr &qua, idx buflen, idx irow, idx seqStart, sStr *id, idx seqlenDump)
{
    idx ploidy = 2;
    if (!seqlenDump){
        seqlenDump = buflen;
    }
    if( irow < refSeqs.dim() ) {
        ploidy = refSeqs[irow].diploidicity;
    }

    idx randAllele = randDist(0, ploidy) + 1;
    idx posMut = binarySearchMutation(seqStart);
    idx mlen;
    idx refposoffset = 0;
    idx blen, restlen;
    RefMutation * mutation;
    mutation = mutContainer.ptr(mutSortList[posMut]);
    idx prevgroupMut = mutation->groupid;
    bool coinResult = flipCoin(mutation->frequency, 100);

    while( (mutation->refNumSeq == irow) && (seqStart < (mutation->position + refposoffset)) && ((mutation->position + refposoffset) < (seqStart + buflen)) ) {
        idx posinbuf = refposoffset + mutation->position - seqStart;
        coinResult = (prevgroupMut == mutation->groupid) ? coinResult : flipCoin(mutation->frequency, 100);
        if( coinResult && ((mutation->allele == 0) || (randAllele == mutation->allele)) ) {
            blen = buf.length();
            if( id && (posinbuf < seqlenDump)) {
                id->printf(" mut=%" DEC " %c>%s", mutation->position + 1, buf.ptr(posinbuf)[0], mutationStringContainer.ptr(mutation->mutationOffset));
            }
            mutation->count++;
            mlen = sLen(mutationStringContainer.ptr(mutation->mutationOffset));
            if( mlen > 1 && ( mutationStringContainer.ptr(mutation->mutationOffset)[0] != '-' )) {
                restlen = blen - (posinbuf + mlen - 1) - 1;
                const char *src = buf.ptr(posinbuf);
                char *dest = buf.ptr(posinbuf+mlen-1);
                for (idx imove = restlen-1; 0 <= imove && src[imove] != '\0'; --imove){
                    dest[imove] = src[imove];
                }
                refposoffset += (mlen - 1);
            }
            if( mutationStringContainer.ptr(mutation->mutationOffset)[0] == '-' ) {

                restlen = blen - (posinbuf + mlen - 1) - 1;
                const char *src = buf.ptr(posinbuf+mlen);
                char *dest = buf.ptr(posinbuf);
                for (idx imove = 0; imove < restlen && src[imove] != '\0'; ++imove){
                    dest[imove] = src[imove];
                }
                buf.cut(blen - 1);
                refposoffset -= (mlen);
            } else {
                strncpy(buf.ptr(posinbuf), mutationStringContainer.ptr(mutation->mutationOffset), mlen);
                if ((mutation->quality > 0) && (qua.length()>0)){
                    for(idx i = 0; i < mlen; ++i) {
                        qua[posinbuf + i] = mutation->quality + 33;
                    }
                }
            }
        } else {
            mutation->miss++;
        }
        ++posMut;
        if( (posMut == mutContainer.dim()) ) {
            break;
        }
        prevgroupMut = mutation->groupid;
        mutation = mutContainer.ptr(mutSortList[posMut]);

    }
    return 1;
}

bool sRandomSeq::addLowComplexity(sStr &buf, idx buflen)
{
    if( flipCoin(lowComplexityFreq, 100) ) {
        idx getlowCNum = randDist(0, lowComplexityCnt);
        const char *lowCstring = sString::next00(lowComplexityString.ptr(), getlowCNum);

        if( !lowCstring ) {
            return false;
        }
        idx lCLen = sLen(lowCstring);
        if( lowComplexityMin == -1 ) {
            lowComplexityMin = lCLen;
        }
        if( lowComplexityMax == -1 ) {
            lowComplexityMax = lCLen;
        }
        if( lCLen < lowComplexityMin ) {
            return false;
        }
        idx randlowCLen = randDist(lowComplexityMin, lowComplexityMax > lCLen ? lCLen : lowComplexityMax);

        if( randlowCLen > buflen ) {
            randlowCLen = buflen;
        }
        idx seqStart = randDist(0, buflen - randlowCLen);

        strncpy(buf.ptr(seqStart), lowCstring, randlowCLen);
        return true;
    }
    return false;

}

bool sRandomSeq::printFastXData(sFil * outFile, idx seqlen, const char *seqid, const char *seq, const char *seqqua, idx subrpt)
{
    char initChar = seqqua ? '@' : '>';
    outFile->printf("%c%s", initChar, seqid);
    if( subrpt > 1 ) {
        outFile->printf(" H#=%" DEC, subrpt);
    }
    outFile->printf("\n%.*s\n", (int) seqlen, seq);
    if( seqqua ) {
        outFile->printf("+\n%.*s\n", (int) seqlen, seqqua);
    }
    return true;
}

real sRandomSeq::randDist(real rmin, real rmax, void *dist)
{
    if( !dist ) {
        return rmin + sRand::ran0(&idum) * (rmax - rmin);
    }
    return 0;
}

bool sRandomSeq::flipCoin(real pr, idx maxValue)
{
    bool valid = randDist(0, maxValue) < pr ? true : false;
    return valid;
}

const char * sRandomSeq::generateRandString(sStr &auxbuf, idx randlength, idx tabu)
{
    auxbuf.cut(0);
    if( (tabu != -1) && (randlength == 1) ) {
        auxbuf.printf("%c", sBioseq::mapRevATGC[(tabu + (idx) randDist(1, 4)) % 4]);
    } else {
        for(idx i = 0; i < randlength; ++i) {
            auxbuf.printf("%c", sBioseq::mapRevATGC[(idx) randDist(0, 4)]);
        }
    }
    return auxbuf.ptr();
}

sTxtTbl * sRandomSeq::tableParser(const char * tblDataPath, const char * colsep, bool header, idx parseCnt)
{
    sFil tblDataFil(tblDataPath);
    idx offset = 0;
    while( offset < tblDataFil.length() && tblDataFil[offset] == '#' ) {
        while( offset < tblDataFil.length() && tblDataFil[offset] != '\r' && tblDataFil[offset] != '\n' )
            offset++;
        while( offset < tblDataFil.length() && (tblDataFil[offset] == '\r' || tblDataFil[offset] == '\n') )
            offset++;
    }
    sTxtTbl *tbl = new sTxtTbl();
    tbl->borrowFile(&tblDataFil, sFile::time(tblDataPath));
    idx flags = sTblIndex::fColsepCanRepeat | sTblIndex::fLeftHeader;
    flags |= header ? sTblIndex::fTopHeader : 0;
    tbl->parseOptions().flags = flags;
    tbl->parseOptions().colsep = colsep ? colsep : " ";
    tbl->parseOptions().comment = "#";
    tbl->parseOptions().initialOffset = offset;
    tbl->parse();
    tbl->remapReadonly();
    return tbl;
}

void sRandomSeq::addMutation(RefMutation *dst, RefMutation *src, idx pos)
{
    *dst = *src;
    dst->count = src->count;
    dst->miss = src->miss;
    dst->position = pos;
}

bool sRandomSeq::readTableMutations(const char * tblDataPath, sBioseq *sub, sStr *errmsg)
{
    bool isHeader = true;

    sFilePath flnm3 (tblDataPath, "%%ext");
    char default_colsep[] = ",";
    if (flnm3.cmp("vcf", 3) == 0){
        default_colsep[0] = '\t';
        isHeader = false;
    }
    sTxtTbl * table = tableParser(tblDataPath, default_colsep, isHeader, 0);

    enum
    {

    };
    idx dimTable = table->rows();
    if( dimTable <= 0 ) {
        errmsg->printf("Can't open Mutation table file");
        return false;
    }
    mutationStringContainer.cut(0);
    idx mutCount = 0;
    sStr cbuf;
    sStr auxString;
    idx irow, icol;
    idx endrange;
    idx refoffset, mutoffset, zygoffset;
    RefMutation mutAux;
    for(idx j = 0; j < dimTable; ++j) {
        cbuf.cut(0);
        table->printCell(cbuf, j, -1);

        irow = sFilterseq::getrownum(idlist, cbuf.ptr());

        if( irow >= 0 && irow < sub->dim() ) {
            icol = 0;
            RefMutation *mutInfo = &mutAux;

            mutInfo->position = table->ival(j, icol++) - 1;
            endrange = table->ival(j, icol++) - 1;
            cbuf.add(0);
            refoffset = cbuf.length();
            table->printCell(cbuf, j, icol++);
            if (endrange <= 0){
                endrange = (mutInfo->position - 1) + (cbuf.length() - refoffset);
            }
            cbuf.add(0);
            mutoffset = cbuf.length();
            table->printCell(cbuf, j, icol++);
            mutInfo->refNumSeq = irow;
            mutInfo->refLength = 1;
            mutInfo->frequency = table->ival(j, icol);
            if (mutInfo->frequency > 100){
                mutInfo->frequency = -1;
            }
            icol++;
            idx qual = table->ival(j, icol);
            mutInfo->quality = (qual == -1) ? (idx) randDist(quaMinValue, quaMaxValue) : 0;
            icol++;
            cbuf.add(0);
            zygoffset = cbuf.length();
            table->printCell(cbuf, j, icol++);
            const char *zygo = cbuf.ptr(zygoffset);
            idx heterohomozygous = 0;

            if ((zygo && *zygo) && isdigit(zygo[0]) ) {
                mutInfo->allele = atoidx(zygo);
                if (mutInfo->allele > 10){
                    mutInfo->allele = 0;
                }
            }
            else {
                mutInfo->allele = 0;
            }
            if( (zygo && *zygo) && !isdigit(zygo[0]) ) {
                auxString.cut(0);
                sString::changeCase(&auxString, zygo, sMin(sLen(zygo), (idx)4), sString::eCaseLo);
                if( strncmp(auxString.ptr(0), "hete", 4) == 0 ) {
                    heterohomozygous = -1;
                } else if( strncmp(auxString.ptr(0), "homo", 4) == 0 ) {
                    heterohomozygous = 1;
                }
            }
            if (heterohomozygous == 0){
                idx lastColumn = table->cols() - 1;
                const char *cell = table->printCell(cbuf, j, lastColumn);
                if (cell && (sLen(cell) > 3) && cell[1] == '/'){
                    if (cell[0] != cell[2]){
                        heterohomozygous = -1;
                    }
                    else {
                        heterohomozygous = 1;
                    }
                }
            }
            if (mutInfo->frequency < 0 ){
                if( heterohomozygous == -1 ) {
                    mutInfo->frequency = (idx) randDist(50, 100);
                    if (mutInfo->allele == 0){
                        mutInfo->allele = (idx) randDist(0, 2) + 1;
                    }
                } else {
                    mutInfo->frequency = (idx) randDist(75, 100);
                    mutInfo->allele = 0;
                }
            }
            mutInfo->mutBiasStart = 0;
            mutInfo->mutBiasEnd = 0;
            mutInfo->groupid = mutCount;
            char *charmut = cbuf.ptr(mutoffset);
            idx pos, ipos = 0;
            for ( idx is=0; charmut[is] != 0;  ++is) {
                unsigned char let = sBioseq::mapATGC[(idx)(charmut[is])];
                if(let == 0xFF) {
                    charmut[is] = 0;
                    break;
                }
            }
            for(pos = mutInfo->position; pos <= (endrange); ++pos, ++ipos) {
                if( !*charmut ) {
                    charmut--;
                    *charmut = '-';
                }
                RefMutation *mut = mutContainer.add(1);
                *mut = *mutInfo;
                mut->position = pos;
                mut->refBase = 0;
                mut->mutationOffset = mutationStringContainer.length();
                mut->count = 0;
                mut->miss = 0;
                if( pos == endrange ) {
                    mutationStringContainer.printf("%s", charmut);
                } else {
                    mutationStringContainer.printf("%c", *charmut);
                }
                mutationStringContainer.add0();
                ++charmut;
                ++mutCount;
            }
        }
    }
    if( mutCount != 0 ) {
        inSilicoFlags |= eSeqMutations;
    }
    delete table;

    return true;
}

idx sRandomSeq::mutationComparator(sRandomSeq * myThis, void * arr, udx i1, udx i2)
{
    idx fit1 = myThis->mutContainer[i1].position;
    idx fit2 = myThis->mutContainer[i2].position;

    if( fit1 > fit2 ) {
        return 1;
    } else if( fit1 < fit2 ) {
        return -1;
    }
    return 0;
}

idx sRandomSeq::binarySearch(real * array, idx num, real target, bool returnIndexbeforeTarget)
{
    idx lo = 0, mid = 0, hi = num - 1;
    real aux;
    while( lo <= hi ) {
        mid = lo + (hi - lo) / 2;
        aux = array[mid];
        if( aux == target ) {
            break;
        } else if( aux < target ) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }

    }
    while( returnIndexbeforeTarget && (mid > 0) && (array[mid] >= target) ) {
        mid -= 1;
    }
    return mid;
}

idx sRandomSeq::binarySearchMutation(idx target)
{
    idx lo = 0, mid = 0, hi = mutSortList.dim() - 1;
    idx aux;
    while( lo <= hi ) {
        mid = lo + (hi - lo) / 2;
        aux = mutContainer[mutSortList[mid]].position;
        if( aux == target ) {
            break;
        } else if( aux < target ) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }

    }
    if( (mid < mutSortList.dim() - 1) && (mutContainer[mutSortList[mid]].position < target) ) {
        mid += 1;
    }
    while( (mid > 0) && (mutContainer[mutSortList[mid-1]].position >= target) ) {
        mid -= 1;
    }
    return mid;
}

bool sRandomSeq::mutationCSVoutput(sFil *out, sBioseq &sub, sStr &err)
{
    out->printf("id, position start, position end, reference, variation, frequency, quality, allele, bias_start, bias_end, count, miss\n");
    idx irow;
    for(idx imut = 0; imut < mutContainer.dim(); ++imut) {
        RefMutation * mutation = mutContainer.ptr(imut);
        if( !mutation ) {
            err.printf("error while reading mutation vector");
            return false;
        }
        irow = mutation->refNumSeq;
        out->printf("\"%s\",%" DEC ",%" DEC ",%c,%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC "\n", sub.id(irow), mutation->position + 1, mutation->position + 1, mutation->refBase, mutationStringContainer.ptr(mutation->mutationOffset), mutation->frequency,
            mutation->quality, mutation->allele, mutation->mutBiasStart, mutation->mutBiasEnd, mutation->count, mutation->miss);
    }

    return true;
}

bool sRandomSeq::mutationVCFoutput(sFil *out, const char *refID, sBioseq &sub, sStr &err)
{
    time_t t = time(0);
    tm * now = localtime(&t);
    idx threshold = 10;
    out->printf("##fileformat=VCFv4.2\n"
        "##fileDate=%d%02d%02d\n"
        "##source=HIVE\n", (now->tm_year + 1900), (now->tm_mon + 1), now->tm_mday);
    out->printf("##reference=%s\n", refID);
    out->printf(
        "##INFO=<ID=AF,Number=.,Type=Float,Description=\"Allele Frequency\">\n"
            "##FILTER=<ID=PASS,Description=\"Coverage Threshold level of at least %" DEC "\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\n", threshold);
    sStr info;
    idx irow;
    for(idx imut = 0; imut < mutContainer.dim(); ++imut) {
            RefMutation * mutation = mutContainer.ptr(imut);

            if( !mutation ) {
                err.printf("error while reading mutation vector");
                return false;
            }
            irow = mutation->refNumSeq;
            info.printf(0, "AF=%" DEC, mutation->frequency);
            out->printf("%s\t%" DEC "\t%s\t%c\t%s\t%" DEC "\tPASS\t%s\n", sub.id(irow), mutation->position + 1, sub.id(irow), mutation->refBase, mutationStringContainer.ptr(mutation->mutationOffset), mutation->quality, info.ptr());
    }

    return true;
}

bool sRandomSeq::normalizeTable()
{
    idx irow = 5, jcol = 6;
    real sum;
    for(idx i = 0; i < irow; ++i) {
        sum = 0;
        for(idx j = 0; j < jcol; ++j) {
            sum += noiseOriginalTable[i][j];
        }
        if( sum == 0 ) {
            return false;
        }
        noiseAccumTable[i][0] = (noiseOriginalTable[i][0] * 100) / sum;
        for(idx j = 1; j < jcol; ++j) {
            noiseAccumTable[i][j] = noiseAccumTable[i][j - 1] + (noiseOriginalTable[i][j] * 100) / sum;
        }
    }
    return true;
}

bool sRandomSeq::noiseDefaults ()
{
    noiseOriginalTable[0][0] = 1;
    noiseOriginalTable[0][1] = 1;
    noiseOriginalTable[0][2] = 1;
    noiseOriginalTable[0][3] = 1;
    noiseOriginalTable[0][4] = 0;
    noiseOriginalTable[0][5] = 0;
    noiseOriginalTable[1][0] = 1;
    noiseOriginalTable[1][1] = 1;
    noiseOriginalTable[1][2] = 1;
    noiseOriginalTable[1][3] = 1;
    noiseOriginalTable[1][4] = 0;
    noiseOriginalTable[1][5] = 0;
    noiseOriginalTable[2][0] = 1;
    noiseOriginalTable[2][1] = 1;
    noiseOriginalTable[2][2] = 1;
    noiseOriginalTable[2][3] = 1;
    noiseOriginalTable[2][4] = 0;
    noiseOriginalTable[2][5] = 0;
    noiseOriginalTable[3][0] = 1;
    noiseOriginalTable[3][1] = 1;
    noiseOriginalTable[3][2] = 1;
    noiseOriginalTable[3][3] = 1;
    noiseOriginalTable[3][4] = 1;
    noiseOriginalTable[3][5] = 1;
    noiseOriginalTable[4][0] = 1;
    noiseOriginalTable[4][1] = 1;
    noiseOriginalTable[4][2] = 1;
    noiseOriginalTable[4][3] = 1;
    noiseOriginalTable[4][4] = 0;
    noiseOriginalTable[4][5] = 0;

    return true;
}


bool sRandomSeq::loadParticularAttributes (sVar &options, sStr *err, sBioseq &qry, bool useMutFile, bool useIonAnnot)
{
    inSilicoFlags |= eSeqPrintRandomReads;
    idx format=1;
    sString::xscanf(options.value("outformat", 0),"%n=0^0^fasta^1^fq^fastq", &format);
    inSilicoFlags |= (format < 2) ? eSeqFastA : eSeqFastQ;

    sString::xscanf(options.value("showId", 0),"%n=0^0^false^off^no^1^true^on^yes", &format);
    inSilicoFlags |= (format < 4) ? eSeqNoId : 0;
    numReads = options.ivalue("numReads", 1000);
    if( !validate(numReads, 1, -1) ) {
        err->printf("Invalid number of Reads");
        return false;
    }
    minLength = options.ivalue("minLen", 100);
    maxLength = options.ivalue("maxLen", 100);
    lengthDistributionType = 0;

    if( minLength > maxLength ) {
        err->printf("Minimum length must be greater than Maximum Length");
        return false;
    }
    if( !validate(minLength, 0, -1) && !validate(maxLength, 0, -1) ) {
        err->printf("Invalid length");
        return false;
    }
    lowComplexityMin = 0;
    lowComplexityMax = 0;
    lowComplexityFreq = 0;
    primersMin = 0;
    primersFreq = 0;

    strandedness = options.ivalue("strand", 2);
    idx pairedEnd = options.boolvalue("pairedEnd", 0);
    if( pairedEnd ) {
        minPEread = options.ivalue("pEndmin", 100);
        maxPEread = options.ivalue("pEndmax", 500);
        if( minPEread < minLength ) {
            minPEread = minLength;
        }
        if( (minPEread < maxPEread) && validate(minPEread, 0, -1) && validate(maxPEread, 0, -1) ) {
            inSilicoFlags |= eSeqPrintPairedEnd;
        } else {
            err->printf("Please check the Paired end inputs, they are invalid");
            return false;
        }
    } else {
        minPEread = 0;
        maxPEread = 0;
    }
    quaType = 0;
    quaMinValue = options.ivalue("quaMin", 25);
    quaMaxValue = options.ivalue("quaMax", 40);
    complexityEntropy = options.rvalue("lowCompEntropy", 0);
    complexityWindow = options.rvalue("lowCompWindow", 0);
    if( complexityWindow && complexityEntropy ) {
        inSilicoFlags |= eSeqComplexityFilter;
    }
    filterNperc = options.ivalue("filterN", -1);
    if( validate(filterNperc, 0, 101) ) {
        inSilicoFlags |= eSeqFilterNs;
    }

    inSilicoFlags |= eSeqPreloadedGenomeFile;
    if( useMutFile && (qry.dim() > 0) ) {
        qryFileCnt = qry.dim();
        inSilicoFlags |= eSeqParseMutationFile;
    }
    idx randseed = options.ivalue("randSeed");
    if( randseed == 0 ) {
    } else {
    }

    noiseDefaults ();

    noisePercentage = options.rvalue("noisePerc", 0);
    if( noisePercentage > 0 ) {
        if( !normalizeTable() ) {
            err->printf("error at normalizing noise table");
            return false;
        }
        inSilicoFlags |= eSeqNoise;
    }

    randMutNumber = options.ivalue("randMutations", 0);
    if( validate(randMutNumber, 1, -1) ) {
        inSilicoFlags |= eSeqGenerateRandomMut;
    }
    if( useIonAnnot ) {
        useAllAnnotRanges = true;
        inSilicoFlags |= eSeqParseAnnotFile;
    }
    randMutStringLen = 1;
    randMutFreq = -1;
    return true;
}

idx sRandomSeq::ionWanderCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults)
{
    sRandomSeq *th = (sRandomSeq *) ts->callbackFuncParam;
    idx seqlen;
    const char *seqid = 0;
    idx pos;
    idx irow;
    sIO *aux = &(th->auxbuf);
    aux->cut(0);
    idx ifCut = th->rangeContainer.dim();
    if (th->Sub->dim() == 0){
        return 0;
    }
    if( traverserStatement->label[0] == 'a' ) {
        RangeSeq *range = th->rangeContainer.add();
        sIon::RecordResult * p = curResults + 1;
        if( p ) {
            pos = aux->length();
            sIon_outTextBody(aux, p->cType, p->body, p->size,',');
            aux->add0();
            seqid = aux->ptr(pos);
        }
        p = curResults + 2;
        if( p ) {
            range->sourceStartRange = (idx) (((int *) (p->body))[1]) - 1;
            range->sourceEndRange = (idx) (((int *) (p->body))[0]) - 1;
        }
        p = curResults + 5;
        if( p ) {
            pos = aux->length();
            sIon_outTextBody(aux, p->cType, p->body, p->size,',');
            aux->add0();
        }
        p = curResults + 4;
        if( p ) {
            pos = aux->length();
            sIon_outTextBody(aux, p->cType, p->body, p->size,',');
            aux->add0();
        }

        irow = sFilterseq::getrownum(th->idlist, seqid);
        if( (irow >= 0) && (irow < th->Sub->dim()) ) {
            seqlen = th->Sub->len(irow);
            if( (range->sourceEndRange > 0) && (range->sourceStartRange > range->sourceEndRange) ) {
                th->rangeContainer.cut(ifCut);
                return 0;
            }
            if( range->sourceStartRange > seqlen ) {
                th->rangeContainer.cut(ifCut);
                return 0;
            }
            if( range->sourceStartRange < 0 ) {
                range->sourceStartRange = 0;
            }
            if( (range->sourceEndRange == -1) || (range->sourceEndRange > seqlen) ) {
                range->sourceEndRange = seqlen;
            }
            range->sourceSeqnum = irow;
            range->destSeqnum = -1;
            range->hiveseqListOffset = 0;
            range->orientation = 0;
            range->coverage = 1;
        }
        else {
            th->rangeContainer.cut(ifCut);
        }

    }

    return 1;
}

bool sRandomSeq::addAnnotRange (AnnotQuery * annot, sStr &stringContainer, const char *seqid, const char *id, const char *type, idx tandem, idx before, idx after, void *ptr){
    const char *notavailable = "na";
    bool isValid = false;
    if ((seqid == 0) || ( strncmp(seqid, notavailable, 2) == 0)){
        annot->seqoffset = -1;
    }
    else {
        annot->seqoffset = stringContainer.length();
        stringContainer.add(seqid);
        stringContainer.add0();
        isValid = true;
    }


    if ((id == 0) || ( strncmp(id, notavailable, 2) == 0)){
        annot->idoffset = -1;
    }
    else {
        annot->idoffset = stringContainer.length();
        stringContainer.add(id);
        stringContainer.add0();
        isValid = true;
    }


    if ((type == 0) || ( strncmp(type, notavailable, 2) == 0)){
        annot->typeoffset = -1;
    }
    else {
        annot->typeoffset = stringContainer.length();
        stringContainer.add(type);
        stringContainer.add0();
        isValid = true;
    }
    annot->tandem = tandem;
    annot->before = before;
    annot->after = after;
    annot->extraInfo = ptr;
    if (ptr){
    }

    return isValid;
}

bool sRandomSeq::generateQualities(const char *seq, sStr &qua, idx seqlen)
{
    char ran;

    char *quabuf = qua.add(0, seqlen);
    if( quaType == 1 ) {
        real a, b, c;

        if( inSilicoFlags & eSeqPrintPairedEnd ) {
            a = -(real) (quaMaxValue - quaMinValue) / (seqlen * seqlen);
            b = 0;
            c = quaMaxValue;
        } else {
            a = -(real) (quaMaxValue - quaMinValue) / (maxLength * maxLength);
            b = 0;
            c = quaMaxValue;
        }
        for(idx x = 0; x < seqlen; ++x) {
            ran = (a * (x * x) + b * x + c) + randDist(-2, 2);
            if( ran < 0 ) {
                ran = 0;
            }
            quabuf[x] = ran + 33;
        }
    } else if( quaType == 2 ) {
        idx rows = qualityTable->rows();
        idx irow;
        real minQ, maxQ;
        char let;
        for(idx i = 0; i < seqlen; ++i) {
            irow = (i * rows) / seqlen;
            let = sBioseq::mapATGC[(idx) seq[i]];
            minQ = qualityTable->rval(irow, 4 * let + 0);
            maxQ = qualityTable->rval(irow, 4 * let + 2);
            ran = randDist(minQ, maxQ);
            if( ran < 0 ) {
                ran = 0;
            }
            quabuf[i] = ran + 33;
        }
    } else {
        for(idx ll = 0; ll < seqlen; ++ll) {
            ran = randDist(quaMinValue, quaMaxValue);
            if( ran < 0 ) {
                ran = 0;
            }
            quabuf[ll] = ran + 33;
        }
    }
    return true;
}

void sRandomSeq::mutationFillandFix(sBioseq &sub)
{
    bool fixMutation;
    sStr t;
    for(idx imut = 0; imut < mutContainer.dim(); ++imut) {
        RefMutation *mutInfo = mutContainer.ptr(imut);
        fixMutation = (mutInfo->refBase == -1) ? true : false;
        if( mutInfo->refBase <= 0 ) {
            t.cut(0);
            sBioseq::uncompressATGC(&t, sub.seq(mutInfo->refNumSeq), mutInfo->position, mutInfo->refLength, true, 0, 0, 0);
            mutInfo->refBase = t[0];
        }
        if( fixMutation ) {
            idx mutlen = sLen(mutationStringContainer.ptr(mutInfo->mutationOffset));
            char mutChar = mutationStringContainer.ptr(mutInfo->mutationOffset)[0];
            if( (mutlen == 1) && (mutChar != '-') && (mutInfo->refBase == mutChar) ) {
                t.cut(0);
                generateRandString(t, mutlen, sBioseq::mapATGC[(idx) mutInfo->refBase]);
                mutationStringContainer.ptr(mutInfo->mutationOffset)[0] = t[0];
            }
        }
    }
}

bool sRandomSeq::generateRandomMutations(sBioseq &sub, sStr &err)
{
    idx mutSize = mutContainer.dim();
    idx refcount = 0;
    sStr auxbuf, t;
    for(idx imut = 0; imut < randMutNumber; ++imut) {
        mutContainer.add();
        RefMutation *mutInfo = mutContainer.ptr(mutSize + refcount);
        mutInfo->refNumSeq = (idx) randDist(0, sub.dim());

        mutInfo->position = (idx) randDist(0, sub.len(mutInfo->refNumSeq));
        t.cut(0);
        sBioseq::uncompressATGC(&t, sub.seq(mutInfo->refNumSeq), mutInfo->position, 1, true, 0, 0, 0);
        mutInfo->refBase = t[0];

        generateRandString(auxbuf, randMutStringLen, sBioseq::mapATGC[(idx) mutInfo->refBase]);

        mutInfo->mutationOffset = mutationStringContainer.length();
        mutationStringContainer.addString(auxbuf.ptr(), randMutStringLen);
        mutationStringContainer.add0();

        mutInfo->frequency = (randMutFreq == -1) ? (idx) randDist(0, 100) : randMutFreq;
        mutInfo->quality = (idx) randDist(quaMinValue, quaMaxValue);
        mutInfo->allele = 0;
        mutInfo->mutBiasStart = 0;
        mutInfo->mutBiasEnd = 0;
        mutInfo->count = 0;
        mutInfo->miss = 0;
        ++refcount;
    }
    return true;
}

bool sRandomSeq::launchParser(const char *outFile, const char *srcFile, sVioseq2 &v, sStr &errmsg)
{

    v.m_callback = m_callback;
    v.m_callbackParam = m_callbackParam;
    idx parseflags = sVioseq2::eTreatAsFastA | sVioseq2::eParseQuaBit;

    idx ires = v.parseSequenceFile(outFile, srcFile, parseflags, 0, 0, 0, 0, 0, 0);

    if( ires < 0 ) {
        for(idx j = 0; v.listErrors[0].errN != 0; ++j) {
            if( v.listErrors[j].errN == ires ) {
                errmsg.printf("%s", v.listErrors[j].msg);
                return false;
            }
        }
    }

    return true;
}

idx sRandomSeq::selectRandRow(real *prob, idx num, real ra)
{
    idx ps = 0;
    if( !ra ) {
        ra = randDist();
    }
    ps = binarySearch(prob, num, ra, true);
    return ps;
}

idx sRandomSeq::randomize(sFil *out, sBioseq &sub, sStr &err, sFil *out2, const char * lazyLenPath, sDic<idx> * cntProbabilityInfo)
{
    idx maxNumberInfiniteLoop = 1000;
    sStr id, t, revt;
    sStr qua, revqua;

    sVec<real> rowProb(lazyLenPath);

    idx subdim = 0;
    if( inSilicoFlags & eSeqParseAnnotFile ) {
        rowProb.resizeM(1+rangeContainer.dim());
        subdim = rangeContainer.dim();

        rowProb[0] = 0;
        for(idx i = 0; i < subdim; ++i) {
            rowProb[i+1] = rowProb[i] + ((rangeContainer.ptr(i)->sourceEndRange - rangeContainer.ptr(i)->sourceStartRange) * rangeContainer.ptr(i)->coverage);
        }
    } else {
        rowProb.resizeM(1+sub.dim());
        idx refDim = refSeqs.dim();
        subdim = sub.dim();
        idx coverage = (0 < refDim) ? refSeqs[0].coverage : 1;
        rowProb[0] = 0;
        for(idx i = 0; i < subdim; ++i) {
            coverage = (i < refDim) ? refSeqs[i].coverage : 1;
            rowProb[i+1] = rowProb[i] + (sub.len(i) * coverage);
        }
    }
    real invAccumLength = 0;
    invAccumLength = 1.0 / rowProb[subdim];
    for(idx i = 0; i <= subdim; ++i) {
        rowProb[i] *= invAccumLength;
    }

    if( inSilicoFlags & eSeqMutations ) {
        mutSortList.resize(mutContainer.dim());
        for(idx i = 0; i < mutContainer.dim(); ++i) {
            mutSortList[i] = i;
        }
        sSort::sortSimpleCallback((sSort::sCallbackSorterSimple) mutationComparator, (void *) this, mutContainer.dim(), mutContainer.ptr(0), mutSortList.ptr(0));

    }

    idx seqlenDump = 0;
    idx seqLen = 0;
    idx totLen = 0;
    idx iter = 0;
    idx countInfiniteLoop = 0;
    idx NCount;
    idx temppos;
    real ra = 0;
    while( iter < numReads ) {
        if( iter % 10000 == 0 ) {
            if( PROGRESS(iter, iter, numReads, 100) ) {
                err.printf("Process needs to be killed; terminating");
                return 0;
            }
        }

        ra  = randDist();
        idx irowProb = selectRandRow(rowProb.ptr(), rowProb.dim(), ra);
        if( irowProb >= subdim || irowProb < 0 ) {
            err.printf("random number is out of bounds to select row ID");
            return -1;
        }

        seqLen = seqlenDump = randDist(minLength, maxLength);
        if( inSilicoFlags & eSeqPrintPairedEnd ) {
            seqLen = randDist(minPEread, maxPEread);
            if( seqLen < seqlenDump ) {
                seqLen = seqlenDump;
            }
        }
        totLen = seqLen * 2;

        idx irow = irowProb;
        idx seqStart = 0;

        if (inSilicoFlags & eSeqParseAnnotFile){
            RangeSeq *range = rangeContainer.ptr(irowProb);
            irow = range->sourceSeqnum;
            if( seqLen > (range->sourceEndRange - range->sourceStartRange) ) {
                seqLen = range->sourceEndRange - range->sourceStartRange;
            }
            seqStart = (idx) randDist(range->sourceStartRange, range->sourceEndRange - seqLen);
            if( totLen > (range->sourceEndRange - seqStart) ) {
                totLen = range->sourceEndRange - seqStart;
            }
        }
        else {
            idx subLen = sub.len(irow);
            if( seqLen > subLen ) {
                seqLen = subLen;
            }
            seqStart = (idx) randDist(0, subLen - seqLen);
            if( totLen > (seqStart + seqLen) ) {
                totLen = seqStart + seqLen;
            }
        }

        if( totLen < minLength ) {
            if( countInfiniteLoop > maxNumberInfiniteLoop ) {
                err.printf("we couldn't find reads with the specified length");
                return 0;
            }
            ++countInfiniteLoop;
            continue;
        }


        const char * seq = sub.seq(irow);
        qua.cut(0);
        t.cut(0);
        sBioseq::uncompressATGC(&t, seq, seqStart, totLen, true, 0);

        const char * seqqua = sub.qua(irow);
        NCount = 0;
        if( seqqua ) {
            bool quabit = sub.getQuaBit(irow);
            for(idx i = seqStart, pos = 0; i < seqStart + seqLen; ++i, ++pos) {
                if( sub.Qua(seqqua, i, quabit) == 0 ) {
                    t[pos] = 'N';
                    NCount++;
                }
            }
        }
        if( (inSilicoFlags & eSeqFilterNs) && (NCount > (seqLen * filterNperc / 100)) ) {
            if( countInfiniteLoop > maxNumberInfiniteLoop ) {
                err.printf("it didn't pass the filter N's stage to generate reads");
                return 0;
            }
            ++countInfiniteLoop;
            continue;
        }

        id.printf(0, "%s%s pos=%" DEC " len=%" DEC, prefixID, sub.id(irow), seqStart + 1, seqlenDump);

        if( inSilicoFlags & eSeqPrimers ) {
            addPrimers(t, seqLen);
        }
        if( inSilicoFlags & eSeqLowComplexity ) {
            addLowComplexity(t, seqLen);
        }

        if( inSilicoFlags & eSeqNoise ) {
            applyNoise(t, seqLen);
        }

        if( inSilicoFlags & eSeqFastQ ) {
            generateQualities(t, qua, seqLen);
        }

        if( inSilicoFlags & eSeqMutations ) {
            applySortMutations(t, qua, seqLen, irow, seqStart, &id);
        }

        if( (inSilicoFlags & eSeqComplexityFilter) && ((temppos = sFilterseq::complexityFilter(t.ptr(), seqLen, complexityWindow, complexityEntropy, false)) != 0) ) {
            if( countInfiniteLoop > maxNumberInfiniteLoop ) {
                err.printf("it didn't pass the complexity test to generate reads");
                return 0;
            }
            ++countInfiniteLoop;
            continue;
        }
        countInfiniteLoop = 0;


        if( inSilicoFlags & eSeqPrintPairedEnd ) {
            sFil *file1, *file2;
            if( flipCoin(0.5) ) {
                file1 = out;
                file2 = out2;
            } else {
                file1 = out2;
                file2 = out;

            }
            if( inSilicoFlags & eSeqNoId ) {
                id.printf(0, "%" DEC, iter + 1);
            } else {
                id.printf(" FragLength=%" DEC " Strand=fwd", seqLen);
            }
            printFastXData(file1, seqlenDump, id, t.ptr(), qua.length() ? qua.ptr() : 0, 1);
            idx revll = seqLen - 1;
            revt.cut(0);
            revt.add(t.ptr(), seqlenDump);
            for(idx ll = 0; ll < seqlenDump; ++ll, --revll) {
                revt[ll] = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[sBioseq::mapATGC[(idx) t[revll]]]];
            }
            if (qua.length()){
                revqua.cut(0);
                revqua.add(qua.ptr(), seqlenDump);
                idx revqlen = qua.length() - 1;
                for(idx ll = 0; ll < seqlenDump; ++ll, --revqlen) {
                    revqua[ll] = qua[revqlen];
                }
            }
            if( inSilicoFlags & eSeqNoId ) {
                id.printf(0, "%" DEC, iter + 1);
            } else {
                id.cut(id.length() - 3);
                id.addString("rev");
            }
            printFastXData(file2, seqlenDump, id, revt.ptr(), qua.length() ? revqua.ptr() : 0, 1);
        } else {
            if( strandedness == 1 || (strandedness == 2 && (randDist() < 0.5)) ) {
                id.printf(" Strand=rev");
                idx revll = seqLen - 1;
                revt.printf(0, "%s", t.ptr());
                char * rev = revt.ptr(0);
                char * tt = t.ptr(0);
                for(idx ll = 0; ll < seqLen; ++ll, --revll) {
                    tt[ll] = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[sBioseq::mapATGC[(idx) rev[revll]]]];
                }
            } else {
                id.printf(" Strand=fwd");
            }
            if( inSilicoFlags & eSeqNoId ) {
                id.printf(0, "%" DEC, iter + 1);
            }
            printFastXData(out, seqLen, id, t.ptr(), qua.length() ? qua.ptr() : 0, 1);
        }
        if( cntProbabilityInfo ) {
            idx * unCnt = cntProbabilityInfo->get(&irowProb, sizeof(idx));
            if( !unCnt ) {
                unCnt = cntProbabilityInfo->set(&irowProb, sizeof(idx));
                *unCnt = 0;
            };
            (*unCnt) = (*unCnt) + 1;
        }
        ++iter;
    }

    return iter;
}
