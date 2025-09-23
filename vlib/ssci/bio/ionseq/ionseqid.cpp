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
#include <ssci/bio/ionseq/ionseqid.hpp>

using namespace slib;

idx IonSeqID::lcommon(const char *s, idx strlen1, const char *t, idx strlen2) {
    if (!strlen1){
        strlen1 = sLen (s);
    }
    if (!strlen2){
        strlen2 = sLen (t);
    }

    sVec <idx> aux;
    idx *curr = aux.add(strlen1);
    idx *prev = aux.add(strlen2);
    idx *swap = 0;
    idx maxSubstr = 0;

    for (idx i = 0; i < strlen1; ++i) {
        for (idx j = 0; j < strlen2; ++j)
        {
            if (s[i] != t[j]) {
                curr[j] = 0;
            }
            else {
                if (i == 0 || j == 0) {
                    curr[j] = 1;
                } else {
                    curr[j] = 1 + prev[j-1];
                }
                if (maxSubstr < curr[j]) {
                    maxSubstr = curr[j];
                }
            }
        }
        swap = curr;
        curr = prev;
        prev = swap;
    }
    return maxSubstr;
}

idx IonSeqID::endsWith(const char *s, idx slen, const char *t, idx tlen)
{
    if (!s || !t)
        return 0;

    const char *s1 = s + slen - 1;
    const char *t1 = t + tlen - 1;
    idx minlen = slen > tlen ? tlen : slen;
    idx counter = 0;
    while ( (counter < minlen) && (*s1 == *t1)){
        ++counter;
        s1 -= 1;
        t1 -= 1;
    }

    return counter;
}

idx IonSeqID::searchCompressed(const char *s, idx len, bool tail, idx *lenmatch)
{
    idx maxlen = 0;
    idx bestword = -1;
    for (idx iword = 0; iword < libraryIndex.dim(); ++iword){
        idx retlen;
        const char *retid = getStringfromCompressedLibrary(iword, 0, &retlen);
        idx commonlength = 0;
        idx minlen = len > retlen ? retlen : len;
        if( !tail ) {
            while( commonlength < minlen && s[commonlength] == retid[commonlength] ) {
                commonlength++;
            }
        } else {
            while( commonlength < minlen && s[len - commonlength - 1] == retid[retlen - commonlength - 1] ) {
                commonlength++;
            }
        }
        if (commonlength == retlen && commonlength > maxlen){
            maxlen = commonlength;
            bestword = iword;
        }
    }
    if (lenmatch){
        *lenmatch = maxlen;
    }
    return bestword;
}

idx IonSeqID::searchBigContainer (const char *s, idx lenString, idx *lenmatch)
{
    idx maxlen = 0;
    idx bestword = -1;
    for (idx iword = 0; iword < indexBigC.dim(); ++iword){
        idx idlen;
        const char *ret = getLetterFromBigContainer(iword, 0, &idlen);
        idx minlen = idlen < lenString ? idlen : lenString;
        idx match = 0;
        for(idx i = 0; (s[i] == ret[i]) && (i < minlen); i++) {
            match++;
        }
        if (match > maxlen){
            maxlen = match;
            bestword = iword;
        }
    }
    if (lenmatch){
        *lenmatch = maxlen;

    }
    return bestword;
}

bool IonSeqID::addID(const char *id, idx lenid)
{
    if (!lenid){
        lenid = sLen(id);
    }
    if (idIndex.dim() <= 1){
        idx newIDpos = getNewId();
        idx indexBigContainer = addBigContainer(newIDpos, id, lenid);
        return setIndex(newIDpos, indexBigContainer, BigContainer);
    }
    else if (idsCompressedTrain.length() > 0){
        idx startmatch;
        idx posStartLibrary = searchCompressed (id, lenid, false, &startmatch);

        idx endmatch;
        idx posEndLibrary = searchCompressed (id, lenid, true, &endmatch);


        if ((posStartLibrary != -1 && startmatch > matchFilterHead) && (posEndLibrary != -1 && endmatch > matchFilterTail)){

            idx v1 = compressString (posStartLibrary, id, lenid, startmatch, endmatch, posEndLibrary);
            idx newIDpos = getNewId();
            return setIndex(newIDpos, v1, CompressedContainer);
        }
    }

    {
        idx startmatch;
        idx posbigC = searchBigContainer (id, lenid, &startmatch);

        if (posbigC != -1 && startmatch > matchFilterHead){
            idx lenauxstr;
            const char *auxstr = getLetterFromBigContainer(posbigC, 0, &lenauxstr);

            idx endmatch = endsWith (id, lenid, auxstr, lenauxstr);


            idx posStartLibrary = addStringCompressedLibrary(id, startmatch);
            idx posEndLibrary = addStringCompressedLibrary(id+lenid-endmatch, endmatch);

            idx v1 = compressString (posStartLibrary, id, lenid, startmatch, endmatch, posEndLibrary);
            idx newIDpos = getNewId();
            setIndex(newIDpos, v1, CompressedContainer);

            idx v2 = compressString (posStartLibrary, auxstr, lenauxstr, startmatch, endmatch, posEndLibrary);
            setIndex(indexBigC[posbigC].idref, v2, CompressedContainer);
        }
        else {
            idx newIDpos = getNewId();
            idx indexBigContainer = addBigContainer(newIDpos, id, lenid);
            setIndex(newIDpos, indexBigContainer, BigContainer);
        }
    }


    return true;
}

idx IonSeqID::addBigContainer(idx idNum, const char *word, idx lenword)
{
    if (!lenword){
        lenword = sLen (word);
    }
    stringBigContainer *bigC = indexBigC.add(1);
    bigC->idref = idNum;
    bigC->stringpos = bigContainer.length();

    bigContainer.add(word, lenword);
    bigContainer.add0(1);
    return indexBigC.dim() - 1;
}

idx IonSeqID::compressString (udx startLibrary, const char *s, idx slen, idx start, idx end, udx endLibrary)
{
    sStr *buf = &idsCompressedTrain;
    unsigned char stlib = (unsigned char) startLibrary;
    unsigned char endlib = (unsigned char) endLibrary;
    idx pos = buf->length();
    idx is = slen - (end + start) + 2;
    idx ix = (4 - (is&0x3)) & 0x3;
    idx ib = is + ix;
    buf->resize(pos+ib);
    char *b=buf->ptr(pos);
    idx ipos = 0;

    b[ipos++] = (LastBitChar | (stlib & ~LastBitChar));

    idx icount = 0;
    while (icount < (is - 2)){
        b[ipos++] = s[start+icount++];
    }

    while (ix){
        b[ipos++] = 0;
        --ix;
    }

    b[ipos] = (LastBitChar | (endlib & ~LastBitChar));

    return pos/4;
}

const char * IonSeqID::uncompressString (sStr *buf, const char *s, idx *len, bool wStats)
{
    if ((LastBitChar & s[0]) != LastBitChar){
        return 0;
    }
    idx initpos = buf->length();

    char first = s[0] & ~(LastBitChar);
    idx templen1, templen2;
    const char *firstString = getStringfromCompressedLibrary(first, 0, &templen1);
    if (firstString && *firstString){
        buf->add(firstString, templen1);
    }

    unsigned char bitOn = false;
    idx is = 1;
    while (!bitOn){
        if (s[is]){
            buf->add(&s[is], 1);
        }
        ++is;
        bitOn = s[is] & LastBitChar;
    }
    char end = s[is] & ~(LastBitChar);
    const char *lastString = getStringfromCompressedLibrary(end, 0, &templen2);
    if (lastString && *lastString){
        buf->add(lastString, templen2);
    }
    buf->add0cut();

    if (len){
        *len = buf->length() - initpos;
    }
    if (wStats){
        stats.count += 1;
        stats.headLength += templen1;
        stats.middleLength += is - 1 ;
        stats.tailLength += templen2;
        *stats.compBin.ptr(first) += 1;
        *stats.compBin.ptr(end) += 1;
    }

    return buf->ptr(initpos);
}
