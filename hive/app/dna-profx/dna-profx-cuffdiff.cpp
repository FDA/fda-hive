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
#include "dna-profx.hpp"

idx DnaProfXcuffdiff::PrepareData ( sUsr& user, const char * parentIDs, const char * workDir, sStr &errMsg) {

    if (!parentIDs) return 0;
    sVec<sHiveId> parids;
    sHiveId::parseRangeSet(parids, parentIDs);
    sStr dataDir("%s/datain", workDir);
    sDir::makeDir(dataDir.ptr());
    if (!sDir::exists(dataDir.ptr())) {
        errMsg.printf("Could not write into workDir");
        return 0;
    }
    sStr ListGFTfileName("%s/gtfFileList.txt", dataDir.ptr());
    sStr ListBAMfileName("%s/bamFileList.txt", dataDir.ptr());
    sStr subjectFastaFile("%s/subject.fa", dataDir.ptr());
    sStr referenceGTFfileLocal("%s/referenceGTFfileLocal.gtf", dataDir.ptr());
    {
    sFil ListGFTfile(ListGFTfileName.ptr()); ListGFTfile.cut(0);
    sFil ListBAMfile(ListBAMfileName.ptr()); ListBAMfile.cut(0);
    for(idx ip=0; ip<parids.dim(); ip++) {

        sUsrObj profile(user, parids[ip]);
        if ( !profile.Id() ) {
            errMsg.printf("can not get parent aligner process %s", parids[ip].print());
            return 0;
        }

        //put .gtf filename from each parent aligner process into a list file (for cuffmerge)
        sStr spath;
        profile.getFilePathname00(spath, "transcripts.gtf"__);
        if (spath && sFile::size(spath.ptr())>0) {
            ListGFTfile.printf("%s\n",spath.ptr()); //one .gtf filename per line is required by cuffmerge
        }
        else {
            errMsg.printf("Could not get .gtf file from parent aligner process %s", parids[ip].print());
            return 0;
        }

        //put .bam filenames from each parent aligner process into a list file (for cuffdiff)
        spath.cut(0);
        profile.getFilePathname00(spath, "accepted_hits.bam"__);
        if (spath && sFile::size(spath.ptr())>0) {
            if (ListBAMfile.length()>0)
                ListBAMfile.printf(", "); //comma and space between filenames is required by cuffdiff
            ListBAMfile.printf("%s",spath.ptr());
        }
        else {
            errMsg.printf("Could not get .bam file from parent aligner process %s", parids[ip].print());
            return 0;
        }

        //copy reference .gtf file used in alignments
        if (!sFile::size(referenceGTFfileLocal.ptr())) {
            sHiveId referenceGTFfileId(profile.propGet("GTFfile"));
            sStr referenceGTFfile;
            DnaProfXcuffdiff::getGTFFilePath( referenceGTFfile, referenceGTFfileId);
            if (sFile::exists(referenceGTFfile.ptr()))
                sFile::copy(referenceGTFfile,referenceGTFfileLocal.ptr(),false);
            else {
                errMsg.printf("Could not get parent aligner process .gtf file from %s", parids[ip].print());
                return 0;
            }
        }

        // load the subject (only once)
        if (!sFile::exists(subjectFastaFile.ptr())) {
            const char * subject=profile.propGet00("subject", 0, ";");
            sHiveseq Sub(&user, subject);Sub.reindex();
            if(Sub.dim()==0) {
                errMsg.printf("Could not load reference from parent aligner process");
                return 0;
            }
            // dump subject into fasta file with original reference names
            sFil subjectFastaFil(subjectFastaFile.ptr());
            Sub.printFastX(&subjectFastaFil, false, 0, Sub.dim(), 0, true, true);

            if (!subjectFastaFile || sFile::size(subjectFastaFile.ptr())==0) {
                errMsg.printf("Could not get .fasta file with reads");
                return 0;
            }
        }
    }
    }//sFils closed

    //final checks
    if (!sFile::exists(subjectFastaFile.ptr())) {
        errMsg.printf("Could not get subject .fasta file from parent aligner process");
        return 0;
    }
    if (!sFile::exists(referenceGTFfileLocal.ptr())) {
        errMsg.printf("Could not get reference .gtf file from parent aligner process");
        return 0;
    }
    return 1;
}


idx DnaProfXcuffdiff::Profile (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters/*=0*/)
{
    //form .bam files string
    sStr bamFilesListName("%s/datain/bamFileList.txt", workDir);
    sFil bamFiles(bamFilesListName.ptr(), sMex::fReadonly);

    sStr refGTFfile("%s/datain/referenceGTFfileLocal.gtf", workDir);
    sStr subjectFastaFile("%s/datain/subject.fa", workDir);
    sStr GTFfileList("%s/datain/gtfFileList.txt", workDir);

    sStr cmdLine("cuffdiff.os"SLIB_PLATFORM" \'%s\' \'%s\' \'%s\' \'%s\' \'%s\'",
        workDir, refGTFfile.ptr(), subjectFastaFile.ptr(), GTFfileList.ptr(), bamFiles.ptr());
    if(log)log->printf("RUNNING: %s\n",cmdLine.ptr());
    sPS::execute(cmdLine);

    outFile->printf("%s/cuffdiff/gene_exp.diff", workDir);

    sStr vpath;
    qp->destPath( &vpath, "gene_exp.diff" );
    if (vpath && sFile::exists( outFile->ptr() ) &&( sFile::size( outFile->ptr() ) > 0 ) ) {
        sFile::copy( outFile->ptr() , vpath.ptr() , false);
    }
    else {
        qp->reqSetInfo(qp->reqId, qp->eQPInfoLevel_Error, "request %"DEC" failed to produce gene_exp.diff file.", qp->reqId);
        qp->reqSetStatus(qp->reqId, qp->eQPReqStatus_ProgError);
        return 0;
    }

    return 1;
}
