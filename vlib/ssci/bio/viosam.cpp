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
#include <ctype.h>
#include <ctime>

#include <ssci/bio/viosam.hpp>
#include <ssci/bio/bioseqtree.hpp>
#include <ssci/bio/filterseq.hpp>
using namespace slib;


char * sViosam::scanNumUntilEOL(const char * ptr, idx * pVal, const char * lastpos)
{
    idx p = 0;
    while ( (ptr[p]<'0' || ptr[p]>'9') && ptr[p]!='\n' && ptr[p]!='*' && ptr+p<lastpos )
        ++p;
    *pVal=sNotIdx;
    if( ptr+p==lastpos || ptr[p]=='\n' || ptr[p]=='*' )return (char *)(ptr+p+1);

    *pVal=0;
    while ( ptr[p]>='0' && ptr[p]<='9'&& ptr+p<lastpos )
    {
        *pVal=*pVal*10+(*(ptr+p)-'0');
        ++p;
    }
    return (char *) (ptr+p);
}

char * sViosam::skipBlanks(const char * ptr, const char * lastpos)
{
    idx p = 0;
    while ( (ptr[p]==' ' || ptr[p]=='\t') && ptr[p]!='\n' && ptr+p<lastpos )
        ++p;

    return (char *)(ptr+p);
}

char * sViosam::skipUntilEOL(const char * ptr, const char * lastpos)
{
    idx p = 0;
    while ( ptr[p]!='\n' && ptr+p<lastpos )
        ++p;

    if (ptr+p<lastpos) ++p;
    return (char *)(ptr +p);
}

char * sViosam::scanUntilLetterOrSpace(const char * ptr, idx * pVal, const char * lastpos)
{
    idx p = 0;
    while ( (ptr[p]<'0' || ptr[p]>'9') && ptr[p]!='\n' && ptr[p]!='*' && ptr+p<lastpos )
        ++p;
    *pVal=sNotIdx;
    if( ptr+p==lastpos || ptr[p]=='\n' || ptr[p]=='*' || ptr[p]==' ' || ptr[p]=='\t')return (char *)(ptr+p+1);

    *pVal=0;
    while ( ptr[p]>='0' && ptr[p]<='9'&& ptr+p<lastpos )
    {
        *pVal=*pVal*10+(*(ptr+p)-'0');
        ++p;
    }
    return (char *) (ptr+p);
}

char * sViosam::scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos)
{
    idx p = 0;
    while ( ptr[p]!='\n' && ptr+p<lastpos && ((ptr[p]=='\t')||(ptr[p]==' ')) )
        ++p; //shift while name

    const char * start = ptr + p; //remember start of the name

    while (ptr+p<lastpos && ptr[p]!='\n' && ptr[p]!=' ' && ptr[p]!='\t')
        ++p;

    ptr = ptr + p;
    if( ptr > start && strVal) {
        strVal->cut(0);
        strVal->add(start, ptr-start);
        strVal->add0();
    }
    return (char *) ptr;

}

char * sViosam::cigar_parser(const char * ptr, const char * lastpos, sVec<idx> * alignOut, idx * lenalign, idx * qryStart)
{
    sStr full_str;
    *lenalign=0;
    //*qry_start=0;
    //idx del_counter = 0;
    //idx ins_counter = 0;
    idx shift=0,firstTime=true;
    idx current_qry = 0;
    //idx qry_end = 0;
    idx current_sub = 0; //sub_start-1;
    //idx sub_end = sub_start-1;

    for( ; strchr("0123456789MIDNSHPX=*+",*ptr) && ptr<lastpos ; ++ptr)
    {
        idx number;
        ptr=scanUntilLetterOrSpace(ptr, &number, lastpos);
        if (*ptr==' ' || *ptr=='\t') {
            break;//return (char *)ptr;
        }

        if(*ptr=='N' || *ptr=='H' ) {
            current_sub+=number;
            continue;
        }
        if(*ptr=='S' || *ptr=='H') {
            current_qry+=number;
            continue;
        }

        if( !strchr("MXDI=",*ptr) )
            continue;


        if(firstTime) {
            shift=current_qry;
            firstTime=false;
        }

        idx * addPtr=alignOut->add(2*number);
        for (idx k=0; k<number; ++k)
        {
             addPtr[2*k]=(*ptr=='I') ? -1: current_sub+k-shift ;
             addPtr[2*k+1]=(*ptr=='D') ? -1 : current_qry+k-shift ;
             ++(*lenalign);
        }
        if(*ptr!='I')current_qry += number;
        if(*ptr!='D')current_sub += number;

        }

    if(qryStart)*qryStart=shift;
    return (char *)ptr;
}

idx sViosam::ParseSamFile(char * fileContent, idx filesize, sVec < idx > * alignOut, const char * vioseqFilename, sDic <idx> *rgm )
{
    sBioseq::initModule(sBioseq::eACGT);
    sFilePath baseFileName(vioseqFilename,"%%pathx.baseFile.tmp");

    sVioDB db(vioseqFilename, "vioseq2", (idx)4, (idx)3);
    idx relationlistIDS[1]={2};
    idx relationlistREC[3]={1, 3, 4}; // REC is linked to Id's and Sequences
    idx relationlistSEQ[1]={3};
    idx relationlistQUA[1]={4}; // QUA is linked to Infile

    db.AddType(sVioDB::eOther,1, relationlistIDS,"ids", 1);
    db.AddType(sVioDB::eOther,3, relationlistREC,"rec", 2);
    db.AddType(sVioDB::eOther,1, relationlistSEQ,"seq", 3);
    db.AddType(sVioDB::eOther,1,relationlistQUA,"qua", 4);

    sFil baseFile(baseFileName);
    baseFile.cut(0);

    idx res= ParseAlignment(fileContent, filesize, db, baseFile, alignOut, rgm );

    if (res == -1)
    {
        db.deleteAllJobs();
        sFile::remove(baseFileName);
        return 0;
    }
    // Should remove later
//    db.deleteAllJobs();

    vioDB.init(vioseqFilename,0,0,0);
    sFile::remove(baseFileName);
    return res;
}

idx sViosam::convertVarScan2OutputintoCSV(const char * varscanFile, const char * csvFileTmplt, sBioseq * Sub, bool skipHeader) {
    sFil varscanFileContent(varscanFile, sMex::fReadonly);
    if( !varscanFileContent )
        return 0;

    const char * varscanFileLastPos = varscanFileContent.ptr() + varscanFileContent.length();

    //for all references in Sub
    // Need to do all references in a single VCF now

    sStr csvFileRef;
    csvFileRef.printf("%s.csv", csvFileTmplt);

    sFil csvFileContent(csvFileRef);
    csvFileContent.cut(0);
    if (!skipHeader) {
        csvFileContent.printf("Chromosome,Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,"
            "Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T\n");
    }

    // Only looking at a single reference (in profx it is split by reference so this is called for each one)
    // No need to look through all references

    // Create a pointer to the TSV file
    const char * buf = varscanFileContent.ptr();

    // Skip to end of first line (header line) in varscan output TSV file
    buf = skipUntilEOL(buf, varscanFileLastPos);

    //check all entries in varscan file
    for(idx iAl = 0; buf < varscanFileLastPos; ++iAl) {

        // Varscan tab-deliminated output fields
        // Col Num  Column Name     Column Definition
        // -------  -----------     -----------------
        // 1        Chrom           chromosome name
        // 2        Position        position (1-based)
        // 3        Ref             reference allele at this position
        // 4        Cons            Consensus genotype of sample in IUPAC format.
        // 5        Reads1          reads supporting reference allele
        // 6        Reads2          reads supporting variant allele
        // 7        VarFreq         frequency of variant allele by read count
        // 8        Strands1        strands on which reference allele was observed
        // 9        Strands2        strands on which variant allele was observed
        // 10       Qual1           average base quality of reference-supporting read bases
        // 11       Qual2           average base quality of variant-supporting read bases
        // 12       Pvalue          Significance of variant read count vs. expected baseline error
        // 13       MapQual1        Average map quality of ref reads (only useful if in pileup)
        // 14       MapQual2        Average map quality of var reads (only useful if in pileup)
        // 15       Reads1Plus      Number of reference-supporting reads on + strand
        // 16       Reads1Minus     Number of reference-supporting reads on - strand
        // 17       Reads2Plus      Number of variant-supporting reads on + strand
        // 18       Reads2Minus     Number of variant-supporting reads on - strand
        // 19       VarAllele       Most frequent non-reference allele observed


        //-----------------------------------------
        // First column is the reference ID
        //-----------------------------------------
        sStr _refIdStr;
        buf = scanAllUntilSpace(buf, &_refIdStr, varscanFileLastPos);
        // Fail if no data read (?)
        if (_refIdStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        // Need to convert to HIVE ID #
        sDic<idx> subjectIdDictionary;
        sFilterseq::parseDicBioseq(subjectIdDictionary, *Sub);
        idx refId = sFilterseq::getrownum (subjectIdDictionary, _refIdStr.ptr(), _refIdStr.length());


        //-----------------------------------------
        // Second column is the reference position (genomic coordinate)
        //-----------------------------------------
        sStr refPosStr;
        buf = scanAllUntilSpace(buf, &refPosStr, varscanFileLastPos);
        // Fail if no data read (?)
        if (refPosStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        //-----------------------------------------
        // Third column is the reference nucleotide
        //-----------------------------------------
        sStr refNucleotideStr;
        buf = scanAllUntilSpace(buf, &refNucleotideStr, varscanFileLastPos);
        // Fail if no data read (?)
        if (refNucleotideStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        //-----------------------------------------
        // Fourth column is the consensus nucleotide
        //-----------------------------------------
        sStr consensusNucleotideStr;
        buf = scanAllUntilSpace(buf, &consensusNucleotideStr, varscanFileLastPos);
        // Fail if no data read (?)
        if (consensusNucleotideStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

    //
    // Calculate Count-A, Count-C, Count-G, and Count-T.  Only two will have data depending on what the reference and consensus nucleotides are
    //

        //
        // Read in the reads supporting both the reference and consensus allele and then match them to the appropriate Count-A, Count-C, etc.
        //

        //-----------------------------------------
        // Fifth column is the Reads1 -> Number of reads supporting the reference allele
        //-----------------------------------------
        //sStr reads1Str;
        //char *buf1 = scanAllUntilSpace(buf, &reads1Str, varscanFileLastPos);
        idx reads1Count = -1;
        buf = scanNumUntilEOL(buf, &reads1Count, varscanFileLastPos);
        // Fail if no data read (?)
        if (reads1Count < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }


        //-----------------------------------------
        // Sixth column is the Reads2 -> Number of reads supporting the consensus allele
        //-----------------------------------------
        //sStr reads2Str;
        //buf = scanAllUntilSpace(buf, &reads2Str, varscanFileLastPos);
        idx reads2Count = -1;
        buf = scanNumUntilEOL(buf, &reads2Count, varscanFileLastPos);
        // Fail if no data read (?)
        if (reads2Count < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        // Set up variables that will be written into output CSV
        //idx countA = 0, countC = 0, countG = 0, countT = 0;
        idx countACGT[4] = {0, 0, 0, 0};

        // Map reference and consensus nucleotide to 0, 1, 2, 3
        idx referenceMap = sBioseq::mapATGC[(idx)refNucleotideStr[0]];
        idx consensusMap = sBioseq::mapATGC[(idx)consensusNucleotideStr[0]];

        // Get the total reads count (will need for Count Total column and potentially for next statement)
        idx totalCount = reads1Count + reads2Count;

        if (referenceMap == consensusMap) {
            // Reference and consensus are the same, so why is this called as a SNP?
            // But handle just in case
            countACGT[referenceMap] = totalCount;
        } else {
            // Record the Reference and the Consensus counts under the proper nucleotide count
            countACGT[referenceMap] = reads1Count;
            countACGT[consensusMap] = reads2Count;
        }


        //-----------------------------------------
        // Read in columns 7, 8, and 9 (not needed for our purposes)
        //-----------------------------------------
        sStr _dummy;
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);


        //-----------------------------------------
        // Read in Qualities - Columns 10 and 11
        //-----------------------------------------
        idx quality1 = -1;
        buf = scanNumUntilEOL(buf, &quality1, varscanFileLastPos);
        // Fail if no data read (?)
        if (quality1 < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx quality2 = -1;
        buf = scanNumUntilEOL(buf, &quality2, varscanFileLastPos);
        // Fail if no data read (?)
        if (quality2 < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        // Calculate quality score
        idx quality = (reads1Count/totalCount * quality1) + (reads2Count/totalCount * quality2);

        //-----------------------------------------
        // Read in columns 12, 13, and 14 (not needed for our purposes)
        //-----------------------------------------
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);


        //-----------------------------------------
        // Read in columns 15, 16, 17, and 18
        //      ReadsPlus and ReadsMinus variables
        //-----------------------------------------

        idx reads1Plus = -1;
        buf = scanNumUntilEOL(buf, &reads1Plus, varscanFileLastPos);
        // Fail if no data read (?)
        if (reads1Plus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx reads1Minus = -1;
        buf = scanNumUntilEOL(buf, &reads1Minus, varscanFileLastPos);
        // Fail if no data read (?)
        if (reads1Minus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx reads2Plus = -1;
        buf = scanNumUntilEOL(buf, &reads2Plus, varscanFileLastPos);
        // Fail if no data read (?)
        if (reads2Plus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx reads2Minus = -1;
        buf = scanNumUntilEOL(buf, &reads2Minus, varscanFileLastPos);
        // Fail if no data read (?)
        if (reads2Minus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        // Calculate Count Forward and Count Reverse
        idx countForward = reads1Plus + reads2Plus;
        idx countReverse = reads1Minus + reads2Minus;

        // Calculate frequencies
        float freqACGT[4];
        freqACGT[0] = countACGT[0] / totalCount;
        freqACGT[1] = countACGT[1] / totalCount;
        freqACGT[2] = countACGT[2] / totalCount;
        freqACGT[3] = countACGT[3] / totalCount;

        // Keep 3 decimal points

        //-------------------------------------
        // Print row in HIVE file
        //-------------------------------------
        csvFileContent.printf("%" DEC ",%s,%s,%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",0,0,%" DEC ",%" DEC ",%" DEC ",%" DEC ",0,0,%.2lf,%.2lf,%.2lf,%.2lf\n",
            refId,refPosStr.ptr(), refNucleotideStr.ptr(), consensusNucleotideStr.ptr(), countACGT[0], countACGT[1], countACGT[2], countACGT[4], totalCount,
            countForward, countReverse, quality, freqACGT[0], freqACGT[1],freqACGT[2],freqACGT[3]); //A,C,G,T
    }
}




idx sViosam::convertVCFintoCSV(const char * vcfFile, const char * csvFileTmplt, sBioseq * Sub, bool skipHeader)
{
    sFil vcfFileContent(vcfFile, sMex::fReadonly);
    if( !vcfFileContent )
        return 0;

    const char * lastPos = vcfFileContent.ptr() + vcfFileContent.length();

    //for all references in Sub
    // Need to do all references in a single VCF now

    sStr csvFileRef;
    csvFileRef.printf("%s.csv", csvFileTmplt);

    sFil csvFileContent(csvFileRef);
    csvFileContent.cut(0);
    if (!skipHeader) {
        csvFileContent.printf("Chromosome,Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,"
            "Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T\n");
    }

    for(idx refNum = 0; refNum < Sub->dim(); ++refNum) {
        const char * buf = vcfFileContent.ptr();


        {

            //check all entries in .vcf file
            for(idx iAl = 0; buf < lastPos; ++iAl) {

                if( *buf == '#' ) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr refIdStr;
                buf = scanAllUntilSpace(buf, &refIdStr, lastPos);
                if (refIdStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                idx idSubptr;
                sscanf(refIdStr.ptr(), "%" DEC, &idSubptr);
                if (idSubptr==sNotIdx || idSubptr<0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                if (idSubptr != refNum) continue;

                sStr refPosStr;
                buf = scanAllUntilSpace(buf, &refPosStr, lastPos);
                if (refPosStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr skipStr;
                buf = scanAllUntilSpace(buf, &skipStr, lastPos);
                if (skipStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr refLetterStr;
                buf = scanAllUntilSpace(buf, &refLetterStr, lastPos);
                if (refLetterStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr consensusLettersStr;
                buf = scanAllUntilSpace(buf, &consensusLettersStr, lastPos);
                if (consensusLettersStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                idx totAltLetterCount = 1; //count quantity of alternative letters
                for (idx is = 0; is < consensusLettersStr.length(); is++) {
                    if (consensusLettersStr[is] == ',') totAltLetterCount++;
                }

                sStr quaStr;
                buf = scanAllUntilSpace(buf, &quaStr, lastPos);
                if (quaStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                skipStr.cut(0);
                buf = scanAllUntilSpace(buf, &skipStr, lastPos);
                if (skipStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr dpStr;
                buf = scanAllUntilSpace(buf, &dpStr, lastPos);
                if (dpStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                //parse for DP value
                char *dps = strstr(dpStr.ptr(), "DP=");
                if (!dps) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                char *dpe = strstr(dps, ";");
                if (!dpe) dpe = strstr(dps, "\n");
                if (!dpe || (dpe-dps-3<1)) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr dpvalStr; dpvalStr.resize(dpe-dps-3);
                strncpy(dpvalStr.ptr(), dps+3, dpe-dps-3);
                dpvalStr.add0(dpe-dps-3+1);

                idx dpval; sscanf(dpvalStr.ptr(), "%" DEC, &dpval);
                if (dpval==sNotIdx || dpval<1) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr otherStr;
                buf = scanAllUntilSpace(buf, &otherStr, lastPos);
                if (otherStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                buf = skipUntilEOL(buf, lastPos);

                //parse for DP4 string
                char *dp4s = strstr(dpStr.ptr(), "DP4=");
                if( !dp4s ) {
                    //buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                char *dp4e = strstr(dp4s, ";");
                if( !dp4e )
                    dp4e = strstr(dp4s, "\n");
                if( !dp4e || (dp4e - dp4s - 4 < 1) ) {
                    //buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr dp4valStr;
                dp4valStr.resize(dp4e - dp4s - 4);
                strncpy(dp4valStr.ptr(), dp4s + 4, dp4e - dp4s - 4);
                dp4valStr.add0(dp4e - dp4s - 4 + 1);
                if (dp4valStr.length()<1) {
                    //buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                // parse DP4 string for values
                sStr dp4valStrZ;
                sString::searchAndReplaceSymbols(&dp4valStrZ, dp4valStr.ptr(), 0, ",", 0, 0, true, true, false, true);
                sVec <idx> dpIdx(sMex::fSetZero); dpIdx.add(4);
                idx dpCounter = 0;
                for( const char * onedp=dp4valStrZ.ptr(); onedp; onedp=sString::next00(onedp)) {
                    sscanf(onedp, "%" DEC, &(dpIdx[dpCounter]));
                    ++dpCounter;
                }
                idx refCount = dpIdx[0] + dpIdx[1];
                idx altCount = dpIdx[2] + dpIdx[3];
                idx forwardCount = dpIdx[0] + dpIdx[2];
                idx reverseCount = dpIdx[1] + dpIdx[3];
                idx totDP = (forwardCount+reverseCount>0)?(forwardCount+reverseCount):dpval;

                //calculate frequencies:
                sVec <idx> dpLetCnt(sMex::fSetZero); dpLetCnt.add(4);
                sVec <real> freq(sMex::fSetZero); freq.add(4);

                for (idx letN=0; letN<4; ++letN) { //consensusLetterStr has format A,T,G
                    for (idx i=0; i<consensusLettersStr.length(); i=i+2) {
                        if( consensusLettersStr[i] == sBioseq::mapRevATGC[letN]) {
                            dpLetCnt[letN] = altCount / (real)totAltLetterCount;
                            freq[letN] = (real)altCount / (real)totDP / (real)totAltLetterCount;
                        }
                    }
                    if( refLetterStr[0] == sBioseq::mapRevATGC[letN] ) {
                        dpLetCnt[letN] = refCount;
                        freq[letN] = (real)refCount / (real)totDP;
                    }
                }

                if (consensusLettersStr.length()>1) { // CSV format lists only one ALT letter
                    consensusLettersStr.cut(1);
                    consensusLettersStr.add0(2);
                }

                if (idSubptr == refNum) {
                    //sStr seqIDtoPrint;
                    //sString::escapeForCSV(seqIDtoPrint, Sub->id(refNum));

                    csvFileContent.printf("%" DEC ",%s,%s,%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",0,0,%" DEC ",%" DEC ",%" DEC ",%s,0,0,%.2lf,%.2lf,%.2lf,%.2lf\n",
                        refNum,refPosStr.ptr(), refLetterStr.ptr(), consensusLettersStr.ptr(), dpLetCnt[0], dpLetCnt[1], dpLetCnt[2], dpLetCnt[3],
                        totDP, forwardCount, reverseCount, quaStr.ptr(), freq[0],freq[1],freq[2],freq[3]); //A,C,G,T
                }
            }

        }
    }


    return 1;
}


idx sViosam::ParseAlignment(const char * fileContent, idx filesize, sVioDB &db , sFil & baseFile, sVec < idx > * alignOut, sDic <idx> * rgm)
{
    // VioDB Variables
    RecSam rec;
    sVec < RecSam > vofs (sMex::fBlockDoubling);
    idx unique = 0;
    idx uniqueCount = 0;
    idx removeCount = 0;
    sVec < idx > ids, ods;
    ids.cut(0);
    ods.cut(0);
    BioseqTree tree (&baseFile, 0);
    sStr SEQ;
    sStr QUA;
    sStr qty;
    sStr tmp;
    sStr idQry;

    const char * buf=fileContent;
    const char * lastPos=fileContent+filesize;

    idx flag, idSub,subStart, score, cntFound=0, lenalign, qryStart;
    sStr idSubStr;

    bool alignflag;

    for (idx iAl=0; buf<lastPos ; ++iAl ){

        PERF_START ("PARSE LINES");
        //buf=sString::next00(buf); if(!buf)break;

        if( *buf=='@') {
            buf=skipUntilEOL(buf, lastPos);
            continue;
        }

        buf=scanAllUntilSpace(buf, &idQry , lastPos);
        const char * id = idQry.ptr(0);
        idx idlen = idQry.length();
//        if(idQry==sNotIdx)
//            continue;

        buf=scanNumUntilEOL(buf, &flag, lastPos);
        if(flag==sNotIdx)
            continue;
        idx dir_flag = 0;
        if (flag&0x4)  { alignflag = false; }
        else {           alignflag = true;  }

        if (flag&0x10) dir_flag = sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignBackward;
        else           dir_flag = sBioseqAlignment::fAlignForward;

        if (flag&0x100){    // Skip the sequence
            buf=skipUntilEOL(buf, lastPos);
            continue;
        }

        sStr idSubStr2;
        buf=scanAllUntilSpace(buf, &idSubStr2, lastPos);
        idSubStr.cut(0);
        idSubStr.printf(">%s",idSubStr2.ptr());

        if (idSubStr.length()<1)
            continue;

        // I
        idx * idSubptr = 0;
        if (rgm != 0){   // There is a dictionary to look for
            idSubptr = rgm->get(idSubStr.ptr());  // So go and get something
        }
        if (!idSubptr){  // There is nothing in the dictionary, or return value is 0
            char *p = strstr(idSubStr.ptr(), "HIVESEQID=");
            if (p){  // If there is a HIVESEQID=, get it in idSubptr
                sscanf(p+10,"%" DEC "", idSubptr);
            }
            else // There is no HIVESEQ
                idSubptr = &iAl;
        }
        else
            idSubptr = &iAl;

        idSub = *idSubptr;
        if(idSub==sNotIdx)
            continue;

        buf=scanNumUntilEOL(buf, &subStart, lastPos);
        if(subStart) subStart--; //sam subStart is 1-based, vioalt is 0-based
        if(idSub==sNotIdx)
            continue;

        buf=scanNumUntilEOL(buf, &score, lastPos);
        if(score==sNotIdx)
            continue;

        buf=skipBlanks(buf, lastPos);
        if( *buf=='\n' || buf>=lastPos)
            continue;

        idx ofsThisAl = 0, headerSize;
        if ((alignOut != 0) && (alignflag == true)){
            ofsThisAl=alignOut->dim();
            headerSize = sizeof(sBioseqAlignment::Al)/sizeof(idx);
            alignOut->add(headerSize);// add space for header
            buf = cigar_parser(buf, lastPos, alignOut, &lenalign,&qryStart);
        }
        else {
            buf=scanAllUntilSpace(buf, 0, lastPos);
        }
        idx len;

        PERF_START("MOVE ONLY");
        buf=scanAllUntilSpace(buf, 0, lastPos);
        buf=scanAllUntilSpace(buf, 0, lastPos);
        buf=scanAllUntilSpace(buf, 0, lastPos);
        PERF_END();
        buf=scanAllUntilSpace(buf, &SEQ, lastPos);
        len = SEQ.length()-1;
        if (strstr (SEQ, "*") || (len == 0)){
            buf=skipUntilEOL(buf, lastPos);
            continue;
        }
        buf=scanAllUntilSpace(buf, &QUA, lastPos);
//        rec.ofsSeq = buf - (fileContent + rec.lenSeq);
//        idx ofsQua = buf - (fileContent + len);
        PERF_END();

        PERF_START("COMPRESSION");

        const char * seq= SEQ;
        const char * qua= QUA;

        // now squeeze the sequence
        rec.ofsSeq=baseFile.length();
        //while ( mapATGC[(idx)(*seq)]==0xFF && seq<nxt )++seq; // new-code
        char * cpy=baseFile.add(0, ( len )/4+1 ); // for two bit representation
        qty.cut(0);
        qty.resize(len/8 + 1);
        rec.lenSeq=sBioseq::compressATGC(cpy,seq,len, qty.ptr());

//        rec.lenSeq=sBioseq::compressATGC(cpy,seq,len); // TODO : cut the bufflen extension length to req.lenseq
        baseFile.cut(rec.ofsSeq+(rec.lenSeq-1)/4+1); // new-code

//        // now squeeze the qualities
        char *pqu;
        if( rec.lenSeq ) {
            pqu=baseFile.add(0,rec.lenSeq);
            for ( idx iq=0; iq<rec.lenSeq; ++iq){
                pqu[iq]=qua[iq]-33;
            }
        }

        PERF_END();
        PERF_START("TREE INSERT");
        // Add record to sVioDBp
        db.AddRecord(eRecID_TYPE,(void *)id, idlen);
        db.AddRecordRelationshipCounter(eRecID_TYPE, 0, 1);

        char *currQua = baseFile.ptr(rec.ofsSeq) + ((rec.lenSeq-1)/4 +1);
        // If there are 'N's in the sequence, reduce the quality to 'min'
        char * Nb=(char *)qty.ptr() ;
        for (idx is = 0; is < rec.lenSeq; is++){
            if (Nb[is/8] != 0){
                if (Nb[is/8] & (0x01<<(is%8))){
                    currQua[is] = 0;
                }
            }
            else is += 7;
        }

        // Add it to the tree
        unique = tree.addSequence(rec.ofsSeq, rec.lenSeq);
        if (unique == -1){
            rec.countSeq = 1;
            *vofs.add()=rec;
            uniqueCount ++;
            ids.vadd(1, uniqueCount-1);
            ods.vadd(1, cntFound);
        }
        else {
            // check and update qualities if necessary
            RecSam *origRec = vofs.ptr(unique);
            char *origQua = baseFile.ptr( origRec->ofsSeq) + ((origRec->lenSeq-1)/4 +1);
            for (idx iqua = 0; iqua < origRec->lenSeq; iqua++){
                origQua[iqua] = ((origQua[iqua] * origRec->countSeq) + currQua[iqua]) / (origRec->countSeq + 1);
            }

            origRec->countSeq++;
            baseFile.cut(rec.ofsSeq);
            removeCount ++;

            ids.vadd(1, unique);
        }


//        ::printf("> %lld len = %lld, start pos = %lld \n", cntFound, rec.lenSeq, rec.ofsSeq);
//        ::printf("Id: %s\n", id);
//        ::printf("We got the sequence: \n");
//        sStr outbuf;
//        const char * seq2 = rec.ofsSeq + baseFile.ptr(0);
//        sBioseq::uncompressATGC (&outbuf, seq2, 0, rec.lenSeq );
//        printf("%s\n",outbuf.ptr());
//        ::printf("And the Qualities: \n");
////          char *q = rec.ofsSeq+(rec.lenSeq-1)/4+1;
//        char *q = ofsQua + fileContent;
//        for (idx ii = 0; ii < rec.lenSeq; ii++)
//        {
//            ::printf("%c", q[ii]);
//        }
//        ::printf("\n");
        PERF_END();

        buf=skipUntilEOL(buf, lastPos);
        //if cigar_parser added no elements to alignOut, remove the useless header:
        if ((alignOut != 0) && (alignflag == true)){
            idx dimAlign=lenalign*2;//alignOut->dim()-ofsThisAl+headerSize;
            if( dimAlign==0) {
                alignOut->cut(ofsThisAl);
                continue;
            }

            sBioseqAlignment::Al * hdr=(sBioseqAlignment::Al *)alignOut->ptr(ofsThisAl);
            hdr->setIdSub(idSub);
            hdr->setIdQry(atoidx(idQry.ptr()));
            hdr->setScore(score);
            hdr->setFlags(dir_flag);
            hdr->setLenAlign(lenalign);
            hdr->setDimAlign(dimAlign);

            idx * m=hdr->match();

            subStart+=m[0];
            qryStart+=m[1];
            hdr->setSubStart(subStart);
            hdr->setQryStart(qryStart);


            hdr->setDimAlign(sBioseqAlignment::compressAlignment(hdr, m, m));
            alignOut->cut(ofsThisAl+headerSize+hdr->dimAlign());
        }
        ++cntFound;
        }

    PERF_START("WRITING");
    // I must sort them, modifying vofs
    sVec < RecSam > vofsSort (sMex::fBlockDoubling);
    sVec < idx > inSort;
    inSort.cut(0);
    sVec < idx > simSort;
    simSort.cut(0);
    sVec < idx > outSort;
    outSort.resize(uniqueCount);

    //tree.inOrderTree2 (0, &inSort);
    tree.inOrderTree3(0, &inSort, &simSort);

    for (idx inx = 0; inx < inSort.dim(); inx++){
        rec = vofs[inSort[inx]];
        {   // I must merge countSeq and simSeq in 1 variable
            idx sim = simSort[inx];
            idx rpt = rec.countSeq;
            if (sim == -1) {sim = 0;}
            rec.countSeq = (sim << 32) | (rpt & 0xFFFF);
        }
        *vofsSort.add()=rec;
        outSort[inSort[inx]] = inx;
        // Add Rec
        db.AddRecord(eRecREC_TYPE,(void *)&rec, sizeof(RecSam));
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 1);
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 2);
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 3);
        // Add Sequences to sVioDB
        char *cpy=baseFile.ptr(rec.ofsSeq);
        char *pqu=cpy+(rec.lenSeq-1)/4+1;
        db.AddRecord(eRecSEQ_TYPE,(void *)cpy, (rec.lenSeq-1)/4+1);
        db.AddRecord(eRecQUA_TYPE,(void *)pqu, rec.lenSeq);
    }

    db.AllocRelation();
    for (idx i = 0; i < ids.dim(); i++){
        db.AddRelation(eRecID_TYPE, 1, i + 1, outSort[ids[i]]+1 ); // eRecREC_TYPE
    }

    for (idx i = 0; i < uniqueCount; i++){
        db.AddRelation(eRecREC_TYPE, 1, i+1, ods[inSort[i]]+1 ); // eRecID_TYPE
        db.AddRelation(eRecREC_TYPE, 2 , i+1, i+1 ); // eRecSEQ_TYPE
        db.AddRelation(eRecREC_TYPE, 3, i+1, i+1 ); // eRecQUA_TYPE
    }
    db.Finalize();
    PERF_END();
    PERF_PRINT();
    return vofsSort.dim();
//    return cntFound;
}

idx sViosam::convertVioaltIntoSam(sBioal *bioal, idx subId, sBioseq *Qry, sBioseq *Sub, bool originalrefIds/*=true*/, const char * outputFilename, FILE * fstream, void * myCallbackParam, callbackType myCallbackFunction)
{

    // idx start
    // idx chunk

    // initialize biological library so all codons, ACGT letter assignments can work
    sBioseq::initModule(sBioseq::eACGT);
    //reqSetInfo(reqId, eQPInfoLevel_Info, "Preparing third party tool.");
    //reqProgress()

    //we need this structure to customize how the alignment iterator works : not really used now ... just needs to be there
    sBioal::ParamsAlignmentIterator PA;
    sStr SAMFile;

    if( outputFilename ) {
        // Construct the sFil here and delete the contents.
        sFile::remove(outputFilename);
        SAMFile.init(outputFilename);
        PA.str = &SAMFile;
    } else {
        PA.str = &SAMFile;
        PA.outF = fstream;
    }

    if( Sub )
        bioal->Sub = Sub; // make sure alignment points to right subject
    else
        Sub = bioal->Sub;
    if( Qry )
        bioal->Qry = Qry; // make sure alignment points to right query
    else
        Qry = bioal->Qry;

    // Add header line for Sam file
    SAMFile.printf("@HD\tVN:1.0\tSO:unsorted\n");

    // If -1 then use all subject sequences
    //sStr subIDs00;
    sVec<idx> subjectAligns;

    if( subId == -1 ) {
        for(idx i = 0; i < Sub->dim(); ++i) {
            idx len = Sub->len(i);

            // Print the @SQ header lines
            if( originalrefIds ) {
                // Print with the original reference ID line (could be long)
                // Spaces are not allowed in SAM format for reference ID

                sStr subIDStrNoSpace;
                sString::searchAndReplaceSymbols(&subIDStrNoSpace, Sub->id(i), 0, " ", "", 0, true, true, false, true);
                SAMFile.printf("@SQ\tSN:%s\tLN:%" DEC "\n", subIDStrNoSpace.ptr(), len);
            } else {
                // Print using just the HIVE seq ID numbers
                SAMFile.printf("@SQ\tSN:%" DEC "\tLN:%" DEC "\n", i + 1, len);
            }

            // Maintain track of the reads that have alignments for the next loop
            subjectAligns.vadd(1, i);


            /*
            bool matched = false;
            for(idx ii = 0; ii < bioal->dimAl(); ii++) {
                sBioseqAlignment::Al * _tmpAl = bioal->getAl(ii);
                idx referenceID = _tmpAl->idSub();
                //idx referenceID = (bioal->getAl(ii))->idSub();
                if( i == referenceID ) {
                    matched = true;
                    break;
                }
            }

            if( !matched ) {
                continue;
            }

            // Maintain track of the reads that have alignments for the next loop
            subjectAligns.vadd(1, i);

            sStr subIDStr("%s", Sub->id(i));
            idx idstart = 0;
            if( *subIDStr.ptr(0) == '>' ) {
                ++idstart;
            }
            idx len = Sub->len(i);

            // Print the @SQ header lines
            if( originalrefIds ) {
                // Print with the original reference ID line (could be long)
                // Spaces are not allowed in SAM format for reference ID

                sStr subIDStrNoSpace;
                sString::searchAndReplaceSymbols(&subIDStrNoSpace, subIDStr.ptr(idstart), 0, " ", "", 0, true, true, false, true);
                SAMFile.printf("@SQ\tSN:%s\tLN:%" DEC "\n", subIDStrNoSpace.ptr(), len);
            } else {
                // Print using just the HIVE seq ID numbers
                SAMFile.printf("@SQ\tSN:%" DEC "\tLN:%" DEC "\n", i + 1, len);
            }
            */
        }

        PA.reqProgressFunction = myCallbackFunction;
        PA.reqProgressParam = myCallbackParam;
        PA.navigatorFlags = sBioal::alPrintCollapseRpt;


        // for every subject (genome) sequence (chromosomes)
        sStr subIDStr;

        for(idx iter = 0; iter < subjectAligns.dim(); ++iter) {
            idx igenome = subjectAligns[iter];
            subIDStr.printf(0, "%s", Sub->id(igenome));
            idx idstart = 0;
            if( *subIDStr.ptr(0) == '>' ) {
                ++idstart;
            }

            // get the start and the total count of alignments
            idx start = 0;

            // iterate those alignment for the current subject igenome
            if( originalrefIds ) {
                // Print the original reference IDs
                // Spaces are not allowed in SAM format for reference ID
                sStr subIDStrNoSpace;
                sString::searchAndReplaceSymbols(&subIDStrNoSpace, subIDStr.ptr(idstart), 0, " ", 0, 0, true, true, false, true);
                subIDStr.printf(0, "%s", subIDStrNoSpace.ptr());
            } else {
                // Print the subject IDs as HIVE seq numbers
                subIDStr.printf(0, "%" DEC, igenome + 1);
            }

            if (originalrefIds){
                PA.userPointer = 0;
            }
            else{
                PA.userPointer = &subIDStr;
            }
            bioal->iterateAlignments(0, start, 0, igenome, (sBioal::typeCallbackIteratorFunction) &vioaltIteratorFunction, (sBioal::ParamsAlignmentIterator *) &PA);

            // Report progress if the callback function is specified to prevent the system from killing a long sam export
            if( myCallbackFunction ) {
                myCallbackFunction(myCallbackParam, 1, 12, 100);
            }
        }

    } else {
        // Otherwise only use the subject sequence passed in from the user
        // user start and count variables to determine how many to dump each iteration

        sStr subIDStr("%s", Sub->id(subId));
        idx idstart = 0;
        if( *subIDStr.ptr(0) == '>' ) {
            ++idstart;
        }
        idx len = Sub->len(subId);

        // Print the @SQ header line
        if( originalrefIds ) {
            // Print the original reference IDs
            // Spaces are not allowed in SAM format for reference ID
            sStr subIDStrNoSpace;
            sString::searchAndReplaceSymbols(&subIDStrNoSpace, subIDStr.ptr(idstart), 0, " ", 0, 0, true, true, false, true);
            SAMFile.printf("@SQ\tSN:%s\tLN:%" DEC "\n", subIDStrNoSpace.ptr(), len);
            subIDStr.printf(0, "%s", subIDStrNoSpace.ptr());
        } else {
            // Print the subject IDs as HIVE seq numbers
            SAMFile.printf("@SQ\tSN:%" DEC "\tLN:%" DEC "\n", subId + 1, len);
            subIDStr.printf(0, "%" DEC, subId + 1);
        }

        // get the start and the total count of alignments
        idx start = 0;
        if (originalrefIds){
            PA.userPointer = 0;
        }
        else {
            PA.userPointer = &subIDStr;
        }

        // iterate those alignment for the current subject igenome
        bioal->iterateAlignments(0, start, 0, subId, (sBioal::typeCallbackIteratorFunction) &vioaltIteratorFunction, (sBioal::ParamsAlignmentIterator *) &PA);

        // Report progress if the callback function is specified to prevent the system from killing a long sam export
        if( myCallbackFunction ) {
            myCallbackFunction(myCallbackParam, 1, 12, 100);
        }
    }
    return 1;
}

idx sViosam::vioaltIteratorFunction(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum)
{
    // Declare variables:
    // queryATGCsequence - the sequence in nucleotide form of the read
    //CIGAR - the string in CIGAR format which includes data on deletions
    //insertions, and matches/missmatches.
    // SAMFile - The output sam file, currently set at SAMOutput.sam.

    sVec<idx> uncompressMM;
    static sStr queryATGCsequence;
    static sStr qualityATGCsequence;
    static sStr subIDStrNoSpace;
    static sStr qryIDStrNoSpace;
    queryATGCsequence.cut(0);
    qualityATGCsequence.cut(0);
    //sStr AllqueryATGCsequence;
    sStr CIGAR;
    CIGAR.cut(0);

    //
    // Uncompress the alignment/read from more efficient two bit into a nucleotide
    // sequence.

    uncompressMM.resize(hdr->lenAlign() * 2); // Resize the sVec to the size of double the length of the alignment
    sBioseqAlignment::uncompressAlignment(hdr, m, uncompressMM.ptr()); // Send the information necessary to uncompress

    // Generate the Flags data
    // To do this, create a samflags variable with a default value of 0.
    // Next, check to see if the vioalt file flags (hdr->flags()) from the
    // header parameter matches the fAlignBackwardComplement flag.  If so,
    // set to 0x10.
    //
    // Possible flags set:
    //      0x00 = Forward alignment
    //      0x10 = Backwards alignment, backwards complement
    //
    // Additional flags can be set based on what vioalt supports in the future.
    //
    idx samflags = 0; // Set the default to forward alignment with no other flags

    // If it is backwards, add hex 0x10
    if( hdr->isBackwardComplement()){
        samflags |= eSamRevComp;
    }

    //  Loop through the alignment keeping track of the match/mismatches, deletions,
    //  and insertions.  The loop defaults to mismatch/matches, increasing a count (mcounter)
    //each time no (-1) is found (which would signify an insertion or deletion).
    //
    // When (-1) is found, the loop write the previously recorded information (on the last
    // set of matches/mismatches, insertions or deletions, to a sStr.  A deletion or insertion
    // counter is increased accordingly and the loop continues.  When something different is
    // found, the loop writes the previously recorded information to the stream and resets the
    // counters.
    //
    // For loop increases by 2 since the data is supplied in pairs.

    idx mcounter = 0; // Counter for match/mismatch data.
    idx dcounter = 0; // Counter for deletions.
    idx icounter = 0; // Counter for insertions.

    idx subAlign; // The subject/reference data in the uncompressed data.  Part of a pair of data with queryAlign.
    idx queryAlign; // The query data in the uncompressed data.  Part of a pair of data with the subAlign.

    // Together both subAlign and queryAlign represent the information at a specific position of the alignment on
    // whether or not there is an insertion, deletion, or match/missmatch.
        for(idx i = 0; i < uncompressMM.dim(); i += 2) {

            subAlign = uncompressMM[i]; // Get the Sub
            queryAlign = uncompressMM[i + 1]; // Get the query

            // Check if there is an insertion
            if( subAlign == -1 ) {
                // There is an insertion, write previous CIGAR information if non insertion
                if( dcounter ) {
                    // There were previous deletions recorded.  Since insertion now found:
                    // Write deletions to stream
                    CIGAR.printf("%" DEC "D", dcounter);
                    // Reset deletion counter since deletions have now been recorded
                    dcounter = 0;
                }
                if( mcounter ) {
                    // There were previous match/mismatch recorded.  Since insertion now found:
                    // Write match/mismatches to stream.
                    CIGAR.printf("%" DEC "M", mcounter);
                    // Reset match/mismatch counter since they have now been recorded
                    mcounter = 0;
                }

                // Increase insertion counter since insertion was detected.
                icounter++;
            }
            // Check if there is a deletion
            else if( queryAlign == -1 ) {
                // There is an deletion, write previous CIGAR information if non deletion
                if( icounter ) {
                    // There were previous insertions recorded.  Since deletion now found:
                    // Write insertions to stream.
                    CIGAR.printf("%" DEC "I", icounter);
                    // Reset insertion counter since insertions have been recorded
                    icounter = 0;
                }
                if( mcounter ) {
                    // There were previous match/mismatch recorded.  Since deletion now found:
                    // Write match/mismatches to stream.
                    CIGAR.printf("%" DEC "M", mcounter);
                    // Reset match/mismatch counter since they have been recorded
                    mcounter = 0;
                }

                // Since a deletion was detected, increase deletion counter
                dcounter++;
            }
            // Default is that there were previous mismatch/matches
            // If the data is anything except for -1, it is a match/mismatch.
            else {
                // Write previous deletion/insertion block if previous read was one of the two.
                if( icounter ) { // Check for previous unrecorded insertions.
                    // There were previous insertion(s).
                    // Write insertions since match/mismatch now found.
                    CIGAR.printf("%" DEC "I", icounter);
                    // Reset insertions counter since they were recorded
                    icounter = 0;
                }
                if( dcounter ) {
                    // There were previous deletion(s).
                    // Write deletions since match/mismatch now found
                    CIGAR.printf("%" DEC "D", dcounter);
                    // Reset deletion counter since they were recorded
                    dcounter = 0;
                }

                // Increase mismatch/match counter
                // This increases since by default anything not a -1 is a match/mismatch.
                mcounter++;
            }
        }

        // Need to add the last group recorded in the loop above.
    // The for loop will go through the information, but will not record the last
    // batch of mismatch/matches, deletions, or insertions it found.
    //
    // Check each counter to see if it has any unwritten records and write.
    // Only one of these should be true (i.e. have recorded unrecorded).

    if( icounter )
        CIGAR.printf("%" DEC "I", icounter);
    if( dcounter )
        CIGAR.printf("%" DEC "D", dcounter);
    if( mcounter )
        CIGAR.printf("%" DEC "M", mcounter);

    // Generate the read sequence from the 2 bit data.
    const char * qry = bioal->Qry->seq(hdr->idQry()); // Generate qry variable
    idx flags=hdr->flags();
    idx nonCompFlags=(flags)&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement));
    idx qrybuflen=bioal->Qry->len(hdr->idQry());

    if( (flags & sBioseqAlignment::fAlignBackward) && ((flags) & sBioseqAlignment::fAlignReverseEngine) == 0 ) {
        for(idx i = 0; i < uncompressMM.dim(); i += 2) {
            idx trainvalue = uncompressMM[i + 1];
            if( trainvalue >= 0 ) {
                idx posQry = hdr->qryStart() + trainvalue;
                char chQ = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx) sBioseqAlignment::_seqBits(qry, (qrybuflen - 1 - (posQry)), nonCompFlags)]];
                queryATGCsequence.add(&chQ, 1);
            }

        }
    } else {
        for(idx i = 0; i < uncompressMM.dim(); i += 2) {
            idx trainvalue = uncompressMM[i + 1];
            if( trainvalue >= 0 ) {
                char chQ = sBioseq::mapRevATGC[(idx) sBioseqAlignment::_seqBits(qry, hdr->qryStart() + trainvalue, nonCompFlags)];
                queryATGCsequence.add(&chQ, 1);

            }

        }
    }
    const char * seqqua = bioal->Qry->qua(hdr->idQry());
    bool quaBit = bioal->Qry->getQuaBit(hdr->idQry());//! check the quality of FASTQ sequence , it will return zero if given is FASTA

    if (seqqua && !quaBit) {
        for(idx i = 0; i < uncompressMM.dim(); i += 2) {
            idx trainvalue = uncompressMM[i + 1];
            if( trainvalue >= 0 ) {
                idx posQry = hdr->qryStart() + trainvalue;

                idx iqx = ((flags & sBioseqAlignment::fAlignBackward) && ((flags) & sBioseqAlignment::fAlignReverseEngine) == 0) ? (qrybuflen - 1 - (posQry)) : (posQry);
                char chQual = seqqua[iqx] + 33;
                qualityATGCsequence.add(&chQual, 1);
            }
        }

    }
    else {
        qualityATGCsequence.addString("*",1);
    }

    // Save the data to the file.
    //
    // The data is recorded in tab separated columns for human readability.
    // See SAM standard for column headings.

    idx rptCnt = bioal->Qry->rpt(hdr->idQry());

    if( param->userPointer ){ //if param->originalrefIds==false, print numeric ID instead of original string id
        for(idx ii=0; ii<rptCnt; ii++){
//            param->str->printf("%" DEC "_%" DEC "\t%" DEC "\t", hdr->idQry(), ii, samflags);
            param->str->addNum(hdr->idQry());
            param->str->add("_",1);
            param->str->addNum(ii);
            param->str->add("\t",1);
            param->str->addNum(samflags);
            param->str->add("\t",1);
            param->str->add(((sStr*) param->userPointer)->ptr(0), ((sStr*) param->userPointer)->length());
            param->str->add("\t",1);
            param->str->addNum(hdr->subStart() + uncompressMM[0] + 1);
            param->str->add("\t",1);
            param->str->addNum(hdr->score());
            param->str->add("\t",1);
//            param->str->printf("\t%" DEC "\t%" DEC "\t", hdr->subStart() + uncompressMM[0] + 1, hdr->score());
            param->str->add(CIGAR.ptr(), CIGAR.length());
            param->str->add("\t*\t0\t0\t", 7);
            param->str->add(queryATGCsequence.ptr(), queryATGCsequence.length());
            param->str->add("\t",1);
            param->str->add(qualityATGCsequence.ptr(), qualityATGCsequence.length());
            param->str->add("\n",1);

        }
    } else {
        const char *subid = bioal->Sub->id(hdr->idSub());
        if (*subid == '>'){
            ++subid;
        }
        subIDStrNoSpace.cut(0);
        //
        // Expensive function
        //
        idx subidlen = sString::copyUntil(&subIDStrNoSpace, subid, 0, " ");
        const char *qryid = bioal->Qry->id(hdr->idQry());
        if (*qryid == '>'){
            ++qryid;
        }
        qryIDStrNoSpace.cut(0);
        //
        // Expensive function
        //
        idx qryidlen = sString::copyUntil(&qryIDStrNoSpace, qryid, 0, " ");
        for(idx ii=0; ii<rptCnt; ii++){
            //param->str->printf("%s_%" DEC "\t%" DEC "\t%s\t%" DEC "\t%" DEC "\t%s\t*\t0\t0\t%s\t*\n", qryIDStrNoSpace.ptr(), ii, samflags, subIDStrNoSpace.ptr(), hdr->subStart() + uncompressMM[0] + 1, hdr->score(), CIGAR.ptr(), queryATGCsequence.ptr());
            param->str->add(qryIDStrNoSpace.ptr(), qryidlen);
            param->str->add("_",1);
            param->str->addNum(ii);
            param->str->add("\t",1);
            param->str->addNum(samflags);
            param->str->add("\t",1);
//            param->str->printf("_%" DEC "\t%" DEC "\t", ii, samflags);
            param->str->add(subIDStrNoSpace.ptr(),subidlen);
            param->str->add("\t",1);
            param->str->addNum(hdr->subStart() + uncompressMM[0] + 1);
            param->str->add("\t",1);
            param->str->addNum(hdr->score());
            param->str->add("\t",1);
//            param->str->printf("\t%" DEC "\t%" DEC "\t", hdr->subStart() + uncompressMM[0] + 1, hdr->score());
            param->str->add(CIGAR.ptr(), CIGAR.length());
            param->str->add("\t*\t0\t0\t", 7);
            param->str->add(queryATGCsequence.ptr(), queryATGCsequence.length());
            param->str->add("\t",1);
            param->str->add(qualityATGCsequence.ptr(), qualityATGCsequence.length());
            param->str->add("\n",1);

        }
    }

    param->str->add0cut();

    return 1;
}

idx sViosam::createVCFheader(FILE * stream, const char * refname,  real threshold) {

    time_t t = time(0); // Current time
    tm * now = localtime( &t); // Use the simple tm structure

    /*
     * Generate the header for the VCF file
     * This involves generating a date.
     * First generate the time and date using the standard
     * library.
     */
    // Construct header

    fprintf(stream,
        "##fileformat=VCFv4.0\n"
        "##fileDate=%d%d%d\n"
        "##source=HIVE\n", (now->tm_year + 1900), (now->tm_mon +1), now->tm_mday);
    if (refname) {
        fprintf(stream, "##reference=%s\n", refname);
    }
    fprintf(stream,
        "##INFO=<ID=AC,Number=.,Type=Integer,Description=\"Allele count in genotypes, for each ALT allele, in the same order as listed\">\n"
        "##INFO=<ID=AF,Number=.,Type=Float,Description=\"Allele Frequency\">\n"
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##INFO=<ID=CG,Number=1,Type=Integer,Description=\"Consensus Genotype\">\n"
        "##FILTER=<ID=PASS,Description=\"Coverage Threshold level of at least %lf\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\n", threshold);

    return 1;
}

idx sViosam::convertSNPintoVCF(sBioseqSNP::SNPRecord * snpRecord, sBioseqSNP::ParamsProfileIterator * params,idx iNum)
{
    //sStr chrName;
    sStr alleleFrequency;
    sStr alleleFreqPercentage;
    float FreqPercent[7];
    //float refFreq = 0;

    idx coverageCount = snpRecord->coverage() + snpRecord->indel[0] + snpRecord->indel[1]; //Gets the total coverage plus the inserts and deletes (snpRecord->coverage() only includes the nucleotide based snps)

    sStr CG;
    idx highest2[1];


    //
    // Set User ID
    //
//    idx mySubID;
//    if( params->userIndex != sNotIdx)
//        mySubID = params->userIndex;
//    else
//        return -1;

    //
    // Create sBioseq object with User ID
    //
// sBioseq object passed in with chrName
//    sBioseq * Sub;
//    if( params->userPointer ) {
//        Sub = (sBioseq*) params->userPointer;
//        chrName.printf("%s", (const char *) (Sub->id(mySubID)));
//    } else
//        return -1;

    //
    // Locate the position of the SNP
    //
    idx position;
    if ((idx)(snpRecord->position) == sNotIdx)
        return -1;
    else position = snpRecord->position;

    // Get the sequence
    // Passed in
//    const char * seq = Sub->seq(mySubID);

    //
    // Generate previous position information for Deletes
    //
    idx prevPosition = (position == 0) ? (-1) : (position-1);

    //
    // Generate the next position information for Inserts
    //
    idx nextPosition = position; //Taking into account the difference between starting at 0 and starting at one, this actually is the 'next' position starting at 0

    //
    // If there are deletes, decrease the previous position in order to get position before delete
    //
    if (snpRecord->indel[1] > 0 && prevPosition > 0) --prevPosition;
    else if (snpRecord->indel[1] > 0 && prevPosition<=0) return -1; //deletes in the first position are not allowed

    //
    // Check to make sure the snp is ACGT or N
    //
    if( snpRecord->letter != 'A' && snpRecord->letter != 'C' && snpRecord->letter != 'G' && snpRecord->letter != 'T' && snpRecord->letter != 'N' )
        return -1;


    sStr sPrevLet;

    //
    // Set up the previous position information for use later...
    //
    if( prevPosition >= 0 && ( snpRecord->indel[1] > 0 || snpRecord->indel[0] > 0) )
        sBioseq::uncompressATGC(&sPrevLet, params->seq, prevPosition, 2); // This should output into sPrevLet the previous letter and the current letter

    //else if( prevPosition < 0 && snpRecord->atgc[5] > 0 )
    //    return -1;
    // ^^^ This is redundant from line 804 ^^^^

    sStr sNextLet;

    //
    // Set up the next position information for use later...
    //
    if( (nextPosition) >= 0 && (snpRecord->indel[0] > 0))
        sBioseq::uncompressATGC(&sNextLet, params->seq, (nextPosition - 1), 2); // This should output into sPrevLet the previous letter and the current letter
    // Need to subtract one here so we additionally get the allele from the reference genome in this position (the nucleotide before the insert)

    //
    // Determine the Reference nucleotide
    //

    sStr ref;
    // snpRecord is a valid letter and there are deletes
    if ((snpRecord->letter=='A'||snpRecord->letter=='C'||snpRecord->letter=='G'||snpRecord->letter=='T'||snpRecord->letter=='N') && snpRecord->indel[1] > 0 )
        ref.printf("%s", sPrevLet.ptr());
    // snpRecord is a valid letter and there are no deletes (else would be caught in the above if)
    else if (snpRecord->letter=='A'||snpRecord->letter=='C'||snpRecord->letter=='G'||snpRecord->letter=='T'||snpRecord->letter=='N')
        ref.printf("%c", snpRecord->letter);
    else return -1;


    sStr alt;

    //
    // Determine quality score
    //
    idx quality = 0;
    if (snpRecord->qua != sNotIdx)
        quality = snpRecord->qua;


    //
    // Now, record the alleles, consensus nucleotides, and calculate their percentage frequencies
    //
    real percentFrequencyOfAlleles = 0.0;
    for(idx i = 0; i < sDim(snpRecord->atgc); ++i) {
        char let = sBioseq::mapRevATGC[i];
        if( snpRecord->atgc[i] > 0 && snpRecord->letter != let ) {
            // letters[i] are present
            // Record the number of occurrences for allele [i]
            if(alleleFrequency.length() > 0)
                alleleFrequency.printf(",");
            alleleFrequency.printf("%" DEC "", snpRecord->atgc[i]);

            // snpRecord->atgc[i] is allele frequency
            // calculate allele frequency percentage
            if(alleleFreqPercentage.length() > 0)
                 alleleFreqPercentage.printf(",");
            //FreqPercent[i] = (float)snpRecord->atgc[i]/(float)snpRecord->coverage();
            alleleFreqPercentage.printf("%.02f", (float)snpRecord->atgc[i]/(float)coverageCount);
            percentFrequencyOfAlleles += ((float)snpRecord->atgc[i]/(float)coverageCount);


            if (i < 4) {
            // Record the nucleotide for allege [i]
                if( alt.length() > 0 )
                    alt.printf(",");
                alt.printf("%c", let);
            }
        } else if (snpRecord->letter == let) {
            FreqPercent[6] = (float)snpRecord->atgc[i]/(float)coverageCount;
        }
    }

    // If the total frequencies of alleles does not meet the threshold cutoff, return without printing
    if (percentFrequencyOfAlleles < params->threshold) {
        return 1;
    }

    if (FreqPercent[6] >= 0.9) {
        // Output is REF|REF
        CG.printf("%s%s", ref.ptr(), ref.ptr()); // Print reference letter for both
    } else if (FreqPercent[6] < 0.9){
        // Need to get all of the freqvalues
        for (idx i = 0; i < sDim(snpRecord->atgc); ++i)
            FreqPercent[i] = (float)snpRecord->atgc[i]/(float)coverageCount;

        // Need to order all of the frequencies
        // Now, need to decide the two highest ones to assign.
        highest2[0] = -1; // Index of the highest FR value
        highest2[1] = -1; // Index of the second highest FR value

        // Use initial two values as seeds
        if (FreqPercent[0] > FreqPercent[1]) {
            //if (FreqPercent[1] != 0)
                highest2[1] = 1;
            //if (FreqPercent[0] != 0)
                highest2[0] = 0;
        } else {
            //if (FreqPercent[1] != 0)
                highest2[0] = 1;
            //if (FreqPercent[0] != 0)
                highest2[1] = 0;
        }
        // Now, given a float array of values, pick the highest two for the consensus genotype

        for(idx i = 2; i < 4; ++i) {
            if (FreqPercent[i] > FreqPercent[highest2[0]]) {
                highest2[1] = highest2[0];
                highest2[0] = i;
            } else if (FreqPercent[i] > FreqPercent[highest2[1]]) {
                highest2[1] = i;
            }
        }

        // Print the values
        if (FreqPercent[highest2[1]] >= .1) { // second highest value is above .1
            CG.printf("%c%c", sBioseq::mapRevATGC[highest2[0]], sBioseq::mapRevATGC[highest2[1]]);
            /*
            if (highest2[0] != 6 && highest2[1] != 6)
                CG.printf("%c%c", sBioseq::mapRevATGC[highest2[0]], sBioseq::mapRevATGC[highest2[1]]);
            else if (highest2[0] == 6 && highest2[1] == 6)
                CG.printf("%c%c", snpRecord->letter, snpRecord->letter);
            else if (highest2[0] == 6)
                CG.printf("%c%c", snpRecord->letter, sBioseq::mapRevATGC[highest2[1]]);
            else if (highest2[1] == 6)
                CG.printf("%c%c", sBioseq::mapRevATGC[highest2[0]], snpRecord->letter);
                */
        } else { // second highest is lower than 0.1
            CG.printf("%c%c", sBioseq::mapRevATGC[highest2[0]], sBioseq::mapRevATGC[highest2[0]]);
            /*
            if (highest2[0] != 6)
                CG.printf("%c%c", sBioseq::mapRevATGC[highest2[0]], sBioseq::mapRevATGC[highest2[0]]);
            else if (highest2[0] == 6)
                CG.printf("%c%c", snpRecord->letter, snpRecord->letter);
                */
        }
    }


        //
        // Determine deletes and inserts
        //

        if( snpRecord->indel[0] > 0 ) {
            // inserts are present
            // Need to alter the ALT

            // Don't need to alter the ref since the recorded ref is the correct position
            //ref.cut(0);
            //ref.printf("%s", sPrevLet.ptr());
            //ref.cut(ref.length()-1);
            //ref.add0();

            // Need to get the next letter for the ALT.  This will be the projected next letter (there could be additional inserts), but we don't have that information until next iteration
            if( alt.length() > 0 )
                alt.printf(",");
            //alt.printf("%cN", snpRecord->letter);

            // Someone added N here for indels; but I can't find N being added into the alternative nucleotides in the VCF standard - not sure if it was a typo or not but it shouldn't be here
            // If it is an insert, we have both letters and should print both.
            // i.e. T with an A insert would be written TA according to the standard
            // Sample line from VCF standard:
            // 20 1234567 microsat1 GTC G,GTCT 50 PASS NS=3;DP=9;AA=G GT:GQ:DP 0/1:35:4 0/2:17:2 1/1:40:3
            // This is a GTC with two alternative alleles, a G (two deletes) and a GTCT (single insert at the end)

            // We are only looking at single inserts, so we can assume the next letter is the insert.  I can't be the previous letter because the insert would have then been recorded at the letter before that
            alt.printf("%c%s", snpRecord->letter, sNextLet.ptr());

        }

        if( snpRecord->indel[1] > 0 ) {
            // deletes are present
            // Find the delete snp
            // handled above...
            //ref.cut(0);
            //ref.printf("%s", sPrevLet.ptr());

            //
            // Need to add the deletion alternative
            // For a deletion, the Reference column contains the nuclotide prior to where the deletion is located
            // So for example, if the reference sequence was:
            //      ATGCCTGC
            //      AT-CCTGC
            // And this is the read sequence, the output would be:
            // ...     REF  ALT     ....
            // ...     TG   T       ....
            // So under the alternate, the same previous nucleotide is used.
            //
            // Have to make sure that the alternative T hasn't already been used though! (i.e.
            // there could be a SNP of T at this position

            // Create a char string to hold the previous letter (index [0])

            //const char *tmpChr = sPrevLet.ptr();
            //const char *tmpAlleles = alt.ptr();
            bool alleleFlag = false;
            // Loop through alleles and check to see if previous nucleotide is already represented

            for (int i = 0; i < alt.length(); ++i) {
                if (strncmp(sPrevLet.ptr(), alt.ptr(i), 1) == 0) {
                    alleleFlag = true; // already there
                    break;
                }
            }

            //sStr tmpAlt;
            //tmpAlt.printf("%s", sPrevLet.ptr());
            //tmpAlt.cut(tmpAlt.length() - 1);
            //tmpAlt.add0();

            if (!alleleFlag) { // Flag is set to false, so no matches found, add the allele
               if( alt.length() > 0 )
                   alt.printf(",");
               alt.printf("%s", sPrevLet.ptr());
               alt.cut(alt.length()-1);
               alt.add0();
            }

        }


    if( alt.length() > 0 ){
        // Arbitrary cutoff of 10% threshold for delete and insert coverage.
        if (CG.length() == 2)
            params->str->printf("%s\t%" UDEC "\t.\t%s\t%s\t%" DEC "\tPASS\tDP=%" DEC ";AC=%s;AF=%s;CG=%s\n", params->chrName.ptr(), position, ref.ptr(), alt.ptr(), quality, coverageCount, alleleFrequency.ptr(), alleleFreqPercentage.ptr(), CG.ptr());
        else params->str->printf("%s\t%" UDEC "\t.\t%s\t%s\t%" DEC "\tPASS\tDP=%" DEC ";AC=%s;AF=%s\n", params->chrName.ptr(), position, ref.ptr(), alt.ptr(), quality, coverageCount, alleleFrequency.ptr(), alleleFreqPercentage.ptr());
    }
    return 1;
}
