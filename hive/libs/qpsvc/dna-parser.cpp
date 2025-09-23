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
#include <qpsvc/dna-parser.hpp>
#include <ssci/bio/vioseq2.hpp>

DnaParser::DnaParser(sQPride& qp, const char * file, const sHiveId & objId, const char * dstTypeName, const bool isVioseqlist, const char * dataType, const char * userFilename)
    : TParent(qp)
{
    setFile("%s", file);
    setObjId(objId);
    if( isVioseqlist ) {
        setVioseqlist(isVioseqlist);
    }
    setTypeName(dstTypeName);
    setTreatAsTypeFile(dataType);
    setUserFilename("%s", userFilename);
}

DnaParser::~DnaParser()
{
}

void DnaParser::setFile(const char * file, ...)
{
    sStr s;
    if( file && file[0] ) {
        sCallVarg(s.vprintf, file);
    }
    setVar("sourceSequenceFilePath", "%s", s.ptr());
}

void DnaParser::setUserFilename(const char * file, ...)
{
    if( file && file[0] ) {
        sStr s;
        sCallVarg(s.vprintf, file);
        setVar("userFileName", "%s", s.ptr());
    }
}

void DnaParser::setObjId(const sHiveId & objId)
{
    setVar("obj", "%s", objId.print());
}

void DnaParser::setMerge(bool isMerged)
{
    setVar("merge", "%" UDEC, (udx)isMerged ? (udx)1 : (udx)0);
}

void DnaParser::setSingleFile(bool isSingle)
{
    setVar("single", "%" UDEC, (udx)isSingle ? (udx)1 : (udx)0);
}

void DnaParser::setVioseqlist(bool isHiveseq)
{
    setVar("hiveseq", "%" UDEC, (udx)isHiveseq ? (udx)1 : (udx)0);
}

void DnaParser::setTreatAsTypeFile(const char * dataType)
{
    udx typeFile = 0;

    if( !dataType){
        typeFile = 0;
    } else if (strcasecmp(dataType, "fasta") == 0 ) {
        typeFile = sVioseq2::eTreatAsFastA | sVioseq2::eParseQuaBit;
    } else if( strcasecmp(dataType, "fastq") == 0 ) {
        typeFile = sVioseq2::eTreatAsFastQ;
    } else if( strcasecmp(dataType, "sam") == 0 ) {
        typeFile = sVioseq2::eTreatAsSAM;
    } else if( strcasecmp(dataType, "annotation") == 0 ) {
        typeFile = sVioseq2::eTreatAsAnnotation;
    } else if( strcasecmp(dataType, "ma") == 0 ) {
        typeFile = sVioseq2::eTreatAsMA;
    } else {
        typeFile = 0;
    }
    setVar("parseAsType", "%" UDEC, typeFile);
}

void DnaParser::setTypeName(const char * typeName)
{
    setVar("dstType", "%s", typeName);
}
