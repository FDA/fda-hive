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
#include <cflow.hpp>
#include <slib/utils/tbl.hpp>
#include <ssci/bio/viopop.hpp>
#include <violin/hiveseq.hpp>
#include "hive-assembly-utils.hpp"
#include <slib/std/string.hpp>
using namespace sviolin;

void ha_utils::mapAcc2Tier2DB(const char* fileName, sVar * pAccToDBName)
{
    return ;
}

idx ha_utils::getCloneCount(sUsr * user, idx clonalAnalysisObjId)
{
    if( !user || clonalAnalysisObjId <= 0 ) {
        return sNotIdx;
    }

    sUsrObj clonalAnalysis(*user, sHiveId(clonalAnalysisObjId, 0));
    if( !clonalAnalysis.Id() ) {
        return sNotIdx;
    }

    sStr clonesPath;
    if( !clonalAnalysis.getFilePathname00(clonesPath, "clones.viopop" __) ) {
        return sNotIdx;
    }

    sViopop population(clonesPath.ptr());
    return population.isok() ? population.dimCl() : sNotIdx;
}

void ha_utils::composeIDLineClones(sStr & outbuf, const char * readId, const char * refId, idx refIdLen, CloneInfo & curClones){
    sStr bbuf; bbuf.cut(0);
    outbuf.printf(0,">");

    const char * pR=readId, *endR=readId+sLen(readId);
    while(pR<endR && !strchr(sString_symbolsBlank,*pR))++pR;
    const char * cloneNum = readId;
    idx cloneNumLen = pR - readId;
    if (cloneNumLen > 6 && strncmp(cloneNum, "clone_", 6) == 0) { cloneNum += 6; cloneNumLen -= 6; }
    outbuf.printf("%s_%.*s", curClones.hoxID, (int)cloneNumLen, cloneNum);

    if (refId && refIdLen>0) {
        const char * p=refId, *end=refId+refIdLen;
        while(p<end && strchr(sString_symbolsBlank,*p))++p;
        const char * refId=p;

        while(p<end && !strchr(sString_symbolsBlank,*p))++p;
        refIdLen=p-refId;

        outbuf.printf(" Derived from %.*s", (int)refIdLen, refId);
        if (end-p>0){
            bbuf.printf(0,"%.*s", (int)(end-p), p);
        }
    } else {
        outbuf.printf(" Derived from NO_REF");
    }

    if (bbuf.length()>0) {
        const char * desc = bbuf.ptr();
        while(*desc && (strchr(sString_symbolsBlank,*desc) || *desc=='|')) ++desc;
        if (*desc) outbuf.printf(" %s", desc);
    }

}

void ha_utils::fixIdLineClones(const char * hitTableFilePath, CloneInfo & curClones, sUsr * user, const char * outname, sStr & outpath){
    outpath.printf(0,"%s/%s",curClones.procFolder,outname);
    if (sFile::exists(outpath.ptr()) && sFile::size(outpath.ptr()) > 0) {
        return;
    }

    sTbl hitTable;
    if (!hitTable.parseFile(hitTableFilePath)) {
        return;
    }

    sDic <sStr> readId2Ref;
    for (idx irow = 1; irow < hitTable.rows(); ++irow) {
        idx readNum = hitTable.ival(irow,3);  
        idx refIdLen; const char * refId=hitTable.cell(irow,2,&refIdLen);
        if (readNum >0) readNum--;

        sStr * pVal=readId2Ref.set(&readNum, sizeof(idx));
        pVal->printf(0," %.*s", (int)refIdLen, refId);
    }

    sStr tmp("%" DEC"",curClones.objId);
    sviolin::sHiveseq sf(user, tmp.ptr(),sviolin::sHiveseq::eBioModeLong);

    sFil myFile(outpath.ptr(), sMex::fMapRemoveFile);
    for (idx iR=0; iR<sf.dim(); ++iR) {
        sStr * refId = readId2Ref.get((const void *)(&iR), sizeof(idx));
        sStr tmpseq, tmpIdLine;
        sf.seqstr_2Bit(&tmpseq,iR);
        idx seqlen = sf.len(iR);  
        if( refId ) {
            composeIDLineClones(tmpIdLine, sf.id(iR), refId->ptr(), refId->length(), curClones);
        } else {
            composeIDLineClones(tmpIdLine, sf.id(iR), 0, 0, curClones);
        }
        sf.printFastXData(&myFile, seqlen,tmpIdLine.ptr(),tmpseq.ptr(),0,0);
    }
    myFile.destroy();
}
