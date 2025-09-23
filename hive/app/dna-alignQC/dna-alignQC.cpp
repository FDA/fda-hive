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
#include "dna-alignQC.hpp"
#include <slib/utils/sRangeTree.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>

#define isSameBase(baseS,baseQ) (baseS!='N'&&baseQ!='N'&&baseS==baseQ)

bool alignQC::init()
{
    sStr tmp_buf;

    sHiveId objID = hp.objs[0].Id();
    sUsrObj obj(*hp.user, objID);

    if( !obj.Id().valid() ) {
        err_buf.printf("Object %s not found or access denied", objID.print());
        return false;
    }

    sHiveId alignerID(hp.formValue("alignQC_objid"));

    sUsrFile aligner(alignerID, hp.user);
    aligner.getFilePathname00(tmp_buf, "alignment.hiveal" _ "alignment.vioal" __);

    alignments = new sHiveal(hp.user, tmp_buf, sBioseq::eBioModeLong);

    if(alignments->dimAl() == 0)
    {
        err_buf.printf("Alignments '%s' are missing or corrupted\n", alignerID.print() );
        return false;
    }

    tmp_buf.cut0cut();
    QPSvcDnaHexagon::getSubject00(aligner,tmp_buf);
    Sub = new sHiveseq(hp.user, tmp_buf.ptr(), alignments->getSubMode());

    if(Sub->dim() == 0)
    {
        err_buf.printf("Reference sequence(s) '%s' are missing or corrupted\n", tmp_buf.length() ? tmp_buf.ptr() : "unspecified");
        hp.logOut(sHiveProc::eQPLogType_Error,"%s",err_buf.ptr());
        hp.reqSetInfo(hp.reqId, sHiveProc::eQPInfoLevel_Error, "%s",err_buf.ptr());
        hp.reqSetStatus(hp.reqId, sHiveProc::eQPReqStatus_ProgError);
        return false;
    }

    tmp_buf.cut0cut();
    QPSvcDnaHexagon::getQuery00(aligner,tmp_buf);
    Qry = new sHiveseq(hp.user, tmp_buf.ptr(), alignments->getQryMode());

    if(Qry->dim() == 0)
    {
        err_buf.printf("Short read sequences '%s' are missing or corrupted\n", tmp_buf.length() ? tmp_buf.ptr() : "unspecified");
        return false;
    }

    alignments->Sub = Sub;
    alignments->Qry = Qry;

    return true;
}

bool rnaSeqQC::init()
{

    if ( !TParent::init() )
        return false;

    sStr tmp_buf;
    sHiveId heptagonID(hp.formValue("heptagon_objid"));
    sUsrFile heptagon(heptagonID, hp.user);
    if ( !heptagon.Id().valid() ) {
        sRC rtc;
        err_buf.printf("Profiler computation '%s' is not accessible\n", heptagon.Id().print() );
        return false;
    }

    heptagon.getFilePathname(tmp_buf, "%s", "SNPprofile.csv");
    profile  = new sFil(tmp_buf,sMex::fReadonly);
    if ( !profile->ok() ) {
        err_buf.printf("Profile files for  computation '%s' are missing or corrupted\n", heptagon.Id().print() );
        return false;
    }


    short_transcript_threshold = hp.formIValue("max_length_of_short",500);
    long_transcript_threshold = hp.formIValue("min_length_of_long",5000);

    return true;
}

bool targetedSeqQC::init()
{
    if ( !TParent::init() )
        return false;

    const char * referenceAnnot = hp.formValue("referenceAnnotations");
    if (referenceAnnot) {
        wander = new sIonWander;
        sVec<sHiveId> annotIds;
        sHiveId::parseRangeSet(annotIds, referenceAnnot);
        sStr path;
        char * p;
        for(idx ia = 0; ia < annotIds.dim(); ++ia) {
            sUsrObj uo(*hp.user, annotIds[ia]);
            uo.getFilePathname(path, "ion.ion");
            if( path.length() && (p = strrchr(path.ptr(0), '.')) ) {
                *p = 0;
                wander->attachIons(path);
            }
            path.cut(0);
        }
        target_mode = eTargetingOn;
        hp.logOut(sHiveProc::eQPLogType_Info, "Setting targetQC mode to targeting on\n");
    } else {
        target_mode = eTargetingOff;
        hp.logOut(sHiveProc::eQPLogType_Info, "Setting targetQC mode to targeting off\n");

    }

    return true;
}

bool DnaAlignQC::isRNASeq()
{
    return objs[0].isTypeOf("svc-rna-alignQC");
}

bool DnaAlignQC::isTargeted()
{
    return objs[0].isTypeOf("svc-dna-targetQC");
}

void alignQC::printTailHist(sStr &left_out, sStr &right_out, const sVec<tailLen> &tail_hist)
{
    left_out.addString("tail_len, count\n");
    right_out.addString("tail_len, count\n");

    for (idx i = 0; i < tail_hist.dim(); ++i) {
        if (tail_hist[i].left > 0)
            left_out.printf("%lld, %lld\n", i, tail_hist[i].left);
        if (tail_hist[i].right > 0)
            right_out.printf("%lld, %lld\n", i, tail_hist[i].right);
    }
}

void alignQC::printPosStats(sStr &out, const sVec<posStats> &pos_stats)
{
    out.printf("qry_pos, match, mismatch, insertion, deletion1, deletion2, deletion3+\n");
    for (idx i = 0; i < pos_stats.dim(); ++i) {
        out.printf("%lld, %lld, %lld, %lld, %lld, %lld, %lld\n",
                    i, pos_stats[i].match, pos_stats[i].mismatch, pos_stats[i].insertion,
                    pos_stats[i].deletion[0], pos_stats[i].deletion[1], pos_stats[i].deletion[2]);
    }
}

void alignQC::countTailLens(sVec<tailLen> &tail_len_freq, sBioseqAlignment::Al * hdr, idx * match, idx repeats)
{
    idx qry_len = Qry->len(hdr->idQry());

    idx left_tail_len = hdr->getQueryStart(match);
    tail_len_freq.resize(left_tail_len + 1);
    tail_len_freq[left_tail_len].left += repeats;

    idx right_tail_len = (qry_len - 1) - hdr->getQueryEnd_uncompressed(match);
    tail_len_freq.resize(right_tail_len + 1);
    tail_len_freq[right_tail_len].right += repeats;
}

void alignQC::countDels(sVec<posStats> &pos_stats, idx qry_pos, idx consecutive_dels, idx repeats)
{
    if (consecutive_dels == 1) {
        pos_stats[qry_pos].deletion[0] += repeats;
    } else if (consecutive_dels == 2) {
        pos_stats[qry_pos].deletion[0] -= repeats;
        pos_stats[qry_pos].deletion[1] += repeats;
    } else if (consecutive_dels == 3) {
        pos_stats[qry_pos].deletion[1] -= repeats;
        pos_stats[qry_pos].deletion[2] += repeats;
    }
}


void alignQC::countPosStats(sVec<posStats> &pos_stats, sBioseqAlignment::Al * hdr, idx * match, idx repeats, sStr &uncomp_sub)
{
    idx qry_id = hdr->idQry();
    idx qry_len = Qry->len(qry_id);
    const char * qry = Qry->seq(qry_id);
    sStr uncomp_qry;
    if (hdr->isReverseComplement())
        sBioseq::uncompressATGC(&uncomp_qry, qry, 0, qry_len, true, 0, 0, 1);
    else
        sBioseq::uncompressATGC(&uncomp_qry, qry, 0, qry_len);

    idx sub_pos = 0;
    idx qry_pos = 0;
    idx consecutive_dels = 0;
    idx last_valid_qry_pos = 0;
    pos_stats.resize(qry_len);
    for (idx j = 0; j < hdr->lenAlign(); j++) {
        sub_pos = hdr->getSubjectPosition(match,j);
        qry_pos = hdr->getQueryPosition(match, j, Qry->len(hdr->idQry()));
        if (qry_pos >= 0) {
            last_valid_qry_pos = qry_pos;
            consecutive_dels = 0;
        }
        if ((sub_pos > 0) && (qry_pos > 0)) {
            char sub_let = *uncomp_sub.ptr(sub_pos);
            char qry_let = *uncomp_qry.ptr(qry_pos);
            if ( (sub_let == qry_let) && (sub_let != 'N') )
                pos_stats[qry_pos].match += repeats;
            else
                pos_stats[qry_pos].mismatch += repeats;

        } else if (sub_pos < 0) {
            pos_stats[qry_pos].insertion += repeats;
        } else if (qry_pos < 0) {
            ++consecutive_dels;
            countDels(pos_stats, last_valid_qry_pos, consecutive_dels, repeats);
        }
    }
}

bool alignQC::collectStats()
{
    const char * tail_len_filename = "tail_len";
    const char * pos_stats_filename = "pos_stats";

    sStr filename_buf;

    filename_buf.addString("file://");
    filename_buf.addString(tail_len_filename);
    hp.reqSetData(hp.reqId, filename_buf.ptr(), 0, 0);
    filename_buf.cut(0);
    if (!hp.reqDataPath(hp.reqId, tail_len_filename, &filename_buf)) {
        err_buf.printf("Cannot write %s file", tail_len_filename);
        return false;
    }
    sVec<tailLen> tail_len_freq(sMex::fSetZero|sMex::fExactSize, filename_buf.ptr());
    filename_buf.cut(0);

    filename_buf.addString("file://");
    filename_buf.addString(pos_stats_filename);
    hp.reqSetData(hp.reqId, filename_buf.ptr(), 0, 0);
    filename_buf.cut(0);
    if (!hp.reqDataPath(hp.reqId, pos_stats_filename, &filename_buf)) {
        err_buf.printf("Cannot write %s file", pos_stats_filename);
        return false;
    }
    sVec<posStats> pos_stats(sMex::fSetZero|sMex::fExactSize, filename_buf.ptr());
    filename_buf.cut(0);

    hp.logOut(sHiveProc::eQPLogType_Info, "Collecting alignQC statistics\n");

    idx sub_id = -1;
    const char * sub = 0;
    sStr uncomp_sub;

    idx total_al = alignments->dimAl();
    idx al_num = total_al / hp.reqSliceCnt;
    idx al_start = hp.reqSliceId * al_num;
    idx al_end = al_start + al_num;
    if ((hp.reqSliceCnt > 1) && (hp.reqSliceId == hp.reqSliceCnt - 1))
        al_end += total_al % hp.reqSliceCnt;

    for (idx i = al_start, al_dim = al_end; i < al_dim; ++i) {
        if (!((i-al_start) % 5000) && !hp.reqProgress(i-al_start, i-al_start, al_end-al_start)) {
            err_buf.printf("Program killed");
            return false;
        }
        sBioseqAlignment::Al * hdr = alignments->getAl(i);
        idx * match = alignments->getMatch(i);
        if (hdr->isCompressed()) {
            match = sBioseqAlignment::uncompressAlignment(hdr, match);
        }
        idx repeats = alignments->getRpt(i);

        countTailLens(tail_len_freq, hdr, match, repeats);

        if ( (sub_id != hdr->idSub()) || (sub == 0) ) {
            sub_id = hdr->idSub();
            sub = Sub->seq(sub_id);
            uncomp_sub.cut(0);
            sBioseq::uncompressATGC(&uncomp_sub, sub, 0, Sub->len(sub_id));
        }
        countPosStats(pos_stats, hdr, match, repeats, uncomp_sub);
    }

    const char * svc_name = hp.vars.value("serviceName");
    if( hp.isLastInMasterGroup(svc_name) ) {
        sVec<tailLen> merged_tail_len_freq(sMex::fSetZero);
        sVec<posStats> merged_pos_stats(sMex::fSetZero);

        sVec<idx> req_list;
        hp.grp2Req(hp.masterId, &req_list, svc_name);
        for (idx i = 0; i < req_list.dim(); ++i) {
            if (!hp.reqDataPath(req_list[i], tail_len_filename, &filename_buf)) {
                err_buf.printf("Unable to open %s for request id %lld", tail_len_filename, req_list[i]);
                return false;
            }
            sVec<tailLen> part_tail_len_freq(sMex::fReadonly|sMex::fExactSize, filename_buf.ptr());
            filename_buf.cut(0);

            if (!hp.reqDataPath(req_list[i], pos_stats_filename, &filename_buf)) {
                err_buf.printf("Unable to open %s for request id %lld", pos_stats_filename, req_list[i]);
                return false;
            }
            sVec<posStats> part_pos_stats(sMex::fReadonly|sMex::fExactSize, filename_buf.ptr());
            filename_buf.cut(0);

            idx tail_len_dim = part_tail_len_freq.dim();
            merged_tail_len_freq.resize(tail_len_dim);
            for (idx j = 0; j < tail_len_dim; ++j) {
                merged_tail_len_freq[j].left += part_tail_len_freq[j].left;
                merged_tail_len_freq[j].right += part_tail_len_freq[j].right;
            }

            idx pos_stats_dim = part_pos_stats.dim();
            merged_pos_stats.resize(pos_stats_dim);
            for (idx j = 0; j < pos_stats_dim; ++j) {
                merged_pos_stats[j].match += part_pos_stats[j].match;
                merged_pos_stats[j].mismatch += part_pos_stats[j].mismatch;
                merged_pos_stats[j].insertion += part_pos_stats[j].insertion;
                for (idx k = 0; k < 3; ++k)
                    merged_pos_stats[j].deletion[k] += part_pos_stats[j].deletion[k];
            }
        }

        hp.logOut(sHiveProc::eQPLogType_Info, "Printing statistics\n");

        const char * left_tail_csv = "alignqc.left_tail.csv";
        if (!hp.reqAddFile(filename_buf, left_tail_csv)) {
            err_buf.printf("Unable to write to %s", left_tail_csv);
            return false;
        }
        sFil left_tail_file(filename_buf);
        filename_buf.cut(0);

        const char * right_tail_csv = "alignqc.right_tail.csv";
        if (!hp.reqAddFile(filename_buf, right_tail_csv)) {
            err_buf.printf("Unable to write to %s", right_tail_csv);
            return false;
        }
        sFil right_tail_file(filename_buf);
        filename_buf.cut(0);

        printTailHist(left_tail_file, right_tail_file, merged_tail_len_freq);

        const char * pos_stats_csv = "alignqc.pos_stats.csv";
        if (!hp.reqAddFile(filename_buf, pos_stats_csv)) {
            err_buf.printf("Unable to write to %s", pos_stats_csv);
            return false;
        }
        sFil pos_file(filename_buf);
        filename_buf.cut(0);
        printPosStats(pos_file, merged_pos_stats);
    }

    hp.reqProgress(al_end-al_start, 100, 100);

    return true;
}

bool rnaSeqQC::collectStats()
{
    sBioseqSNP::SNPRecord Line;
    Line.position = 0;

    const char * startPos = profile->ptr();
    const char  * endSNP = startPos + profile->length();

    const char * SNPline = sBioseqSNP::SNPConcatenatedRecordNext(startPos, &Line, endSNP);

    idx currSub = -1;
    idx prevPos = (idx)(Line.position-1);
    idx prevBin = sNotIdx;

    idx iSub = (unsigned int)-1;
    idx iSubLen = 0;
    idx iPos = 0;
    idx iCoverage = 0;

    idx iScaleRange1 = short_transcript_threshold;
    idx iScaleRange2 = long_transcript_threshold;


    idx scLen = 1000;
    idx lengthSoftLimit = 100000;
    idx lengthHardLimit = 1000000;
    real scaleF = 1;


    idx iBinPerPos5, iBinPerPos3, iScaleBin;

    sVec<MeanCov> vecPerPos5(sMex::fExactSize|sMex::fSetZero);
    sVec<MeanCov> vecPerPos3(sMex::fExactSize|sMex::fSetZero);

    sVec<MeanCov> vecScale(sMex::fExactSize|sMex::fSetZero);
    vecScale.add(scLen);

    sVec<MeanCov> vecScaleG1(sMex::fExactSize|sMex::fSetZero);
    vecScaleG1.add(scLen);
    sVec<MeanCov> vecScaleG2(sMex::fExactSize|sMex::fSetZero);
    vecScaleG2.add(scLen);
    sVec<MeanCov> vecScaleG3(sMex::fExactSize|sMex::fSetZero);
    vecScaleG3.add(scLen);

    sVec<MeanCov> * pCurrScaleGroup = 0;
    idx subsLongerThanSoftLimit = 0, maxReportedWarnings = 10;

    for (idx iTemp = 0; SNPline && iTemp < 2; SNPline = sBioseqSNP::SNPConcatenatedRecordNext(SNPline, &Line,endSNP ) )
    {

        iSub = Line.iSub;
        iPos = Line.position - 1;
        iCoverage = Line.coverage();

        if(iSub != currSub)
        {
            if(!hp.reqProgress( iSub, iSub, alignments->Sub->dim() ) ) return 0;
            currSub = iSub;
            prevPos = iPos;
            prevBin = sNotIdx;

            iSubLen = alignments->Sub->len(currSub-1);
            if (iSubLen > lengthHardLimit) {
                err_buf.printf("Subject '%s' is %" DEC" bp long. Longer than the %" DEC"bp limit\n", alignments->Sub->id(currSub-1),iSubLen, lengthHardLimit);
                return false;
            } else if(iSubLen > lengthSoftLimit) {
                ++subsLongerThanSoftLimit;
                if( subsLongerThanSoftLimit <= maxReportedWarnings ) {
                    sStr t_buf("Subject '%s' is %" DEC" bp long. Longer than the %" DEC"bp limit\n", alignments->Sub->id(currSub-1),iSubLen, lengthSoftLimit);
                    hp.logOut(sHiveProc::eQPLogType_Warning,"%s",t_buf.ptr());
                    hp.reqSetInfo(hp.reqId, sHiveProc::eQPInfoLevel_Warning, "%s",t_buf.ptr());
                }

            }

            vecPerPos5.resize(iSubLen);
            vecPerPos3.resize(iSubLen);

            scaleF = (real)scLen/iSubLen;

            if(iSubLen > iScaleRange2)
            {
                pCurrScaleGroup = &vecScaleG3;
            }
            else if(iSubLen > iScaleRange1)
            {
                pCurrScaleGroup = &vecScaleG2;
            }
            else
            {
                pCurrScaleGroup = &vecScaleG1;
            }
        } else {
            while ( iPos - prevPos > 1 )
            {
                vecPerPos5[prevPos].cnt += 1;
                vecPerPos3[iSubLen - prevPos - 1].cnt += 1;
                prevPos++;
            }

            prevPos = iPos;
        }

        if (!pCurrScaleGroup)
        {
            err_buf.printf("Scale Groups are not defined.");
            hp.logOut(sHiveProc::eQPLogType_Error,"%s",err_buf.ptr());
            hp.reqSetInfo(hp.reqId, sHiveProc::eQPInfoLevel_Error, "%s",err_buf.ptr());
            hp.reqSetStatus(hp.reqId, sHiveProc::eQPReqStatus_ProgError);
            return 0;
        }

        iBinPerPos5 = iPos;
        vecPerPos5[iBinPerPos5].sumCov += iCoverage;
        vecPerPos5[iBinPerPos5].cnt += 1;

        iBinPerPos3 = iSubLen - iBinPerPos5 - 1;
        vecPerPos3[iBinPerPos3].sumCov += iCoverage;
        vecPerPos3[iBinPerPos3].cnt += 1;


        if(scaleF == 1)
        {
            iScaleBin = iPos;
            vecScale[iScaleBin].sumCov += iCoverage;
            vecScale[iScaleBin].cnt += 1;

            pCurrScaleGroup->ptr(iScaleBin)->sumCov += iCoverage;
            pCurrScaleGroup->ptr(iScaleBin)->cnt += 1;
         }
        else if(scaleF < 1)
        {
            iScaleBin = floor(iPos*scaleF);
            idx iScalePrevBin = floor(iPos*scaleF - scaleF);
            if( iScalePrevBin < 0 )
                iScalePrevBin = iScaleBin;

            vecScale[iScaleBin].cnt += 1;
            pCurrScaleGroup->ptr(iScaleBin)->cnt += 1;

            if(iScaleBin == iScalePrevBin)
            {
                vecScale[iScaleBin].sumCov += iCoverage;
                pCurrScaleGroup->ptr(iScaleBin)->sumCov += iCoverage;
            }
            else
            {
                real cov = (iPos*scaleF - iScaleBin) * iCoverage / scaleF;

                vecScale[iScaleBin].sumCov += cov;
                vecScale[iScalePrevBin].sumCov += (iCoverage - cov);

                pCurrScaleGroup->ptr(iScaleBin)->sumCov += cov;
                pCurrScaleGroup->ptr(iScalePrevBin)->sumCov += (iCoverage - cov);

                if(prevBin == sNotIdx)
                    prevBin = iScalePrevBin;
            }
        }
        else
        {
            real transPos = iPos*scaleF;
            real transPrevPos = transPos - scaleF;
            real cov = iCoverage/scaleF;

            if( transPrevPos < 0 )
                transPrevPos = 0;

            iScaleBin = floor(transPos);
            idx sBin = floor(transPrevPos);
            if(prevBin == sNotIdx)
                prevBin = sBin;

            if(sBin>0) {
                vecScale[sBin].sumCov += cov*((sBin+1) - transPrevPos);
                pCurrScaleGroup->ptr(sBin)->sumCov += cov*((sBin+1) - transPrevPos);
            }

            vecScale[iScaleBin].sumCov += cov*(transPos - iScaleBin);
            vecScale[iScaleBin].cnt += 1;

            pCurrScaleGroup->ptr(iScaleBin)->sumCov += cov*(transPos - iScaleBin);
            pCurrScaleGroup->ptr(iScaleBin)->cnt += 1;


            for(idx i = sBin + 1; i < iScaleBin; ++i)
            {
                vecScale[i].sumCov += cov;
                pCurrScaleGroup->ptr(i)->sumCov += cov;
            }
        }


        if(prevBin != sNotIdx)
        {
            while ( iScaleBin - prevBin > 1 )
            {
                vecScale[prevBin].cnt += 1;
                pCurrScaleGroup->ptr(prevBin)->cnt += 1;
                prevBin++;
            }
            prevBin = iScaleBin;
        }

    }

    if( subsLongerThanSoftLimit > maxReportedWarnings ) {
        sStr t_buf("%" DEC" more subjects are longer than the %" DEC"bp limit\n", subsLongerThanSoftLimit - maxReportedWarnings, lengthSoftLimit);
        hp.logOut(sHiveProc::eQPLogType_Warning,"%s",t_buf.ptr());
        hp.reqSetInfo(hp.reqId, sHiveProc::eQPInfoLevel_Warning, "%s",t_buf.ptr());
    }
    sStr dstFilePath;
    dstFilePath.cut(0);hp.reqAddFile(dstFilePath, "perPos_5to3_coverage.csv");sFile::remove(dstFilePath);
    {
        sFil out53File(dstFilePath);
        out53File.printf("Position,Coverage\n");

        dstFilePath.cut(0);hp.reqAddFile(dstFilePath, "perPos_3to5_coverage.csv");sFile::remove(dstFilePath);
        sFil out35File(dstFilePath);
        out35File.printf("Position,Coverage\n");

        for ( idx irow=0; irow < vecPerPos3.dim(); ++irow)
        {
            if(vecPerPos5[irow].cnt)
                out53File.printf("%" DEC",%f\n", irow+1, vecPerPos5[irow].sumCov/vecPerPos5[irow].cnt);
            if(vecPerPos3[irow].cnt)
                out35File.printf("%" DEC",%f\n", irow+1, vecPerPos3[irow].sumCov/vecPerPos3[irow].cnt);
        }
    }

    dstFilePath.cut(0);hp.reqAddFile(dstFilePath, "scale_coverage.csv");sFile::remove(dstFilePath);
    {
        sFil outScaleFile(dstFilePath);
        outScaleFile.printf("Bin,Coverage\n");

        for ( idx irow=0; irow < vecScale.dim(); ++irow)
        {
            if(vecScale[irow].cnt)
                outScaleFile.printf("%.2f,%f\n", (real)(100*(irow+1))/vecScale.dim(), vecScale[irow].sumCov/vecScale[irow].cnt);
        }
    }

    dstFilePath.cut(0);hp.reqAddFile(dstFilePath, "scale_gr1_coverage.csv");sFile::remove(dstFilePath);
    {
        sFil outScaleFile(dstFilePath);
        outScaleFile.printf("Bin,Coverage\n");

        for ( idx irow=0; irow < vecScaleG1.dim(); ++irow)
        {
            if(vecScaleG1[irow].cnt)
                outScaleFile.printf("%.2f,%f\n", (real)(100*(irow+1))/vecScaleG1.dim(), vecScaleG1[irow].sumCov/vecScaleG1[irow].cnt);
        }
    }

    dstFilePath.cut(0);hp.reqAddFile(dstFilePath, "scale_gr2_coverage.csv");sFile::remove(dstFilePath);
    {
        sFil outScaleFile(dstFilePath);
        outScaleFile.printf("Bin,Coverage\n");

        for ( idx irow=0; irow < vecScaleG2.dim(); ++irow)
        {
            if(vecScaleG2[irow].cnt)
                outScaleFile.printf("%.2f,%f\n", (real)(100*(irow+1))/vecScaleG2.dim(), vecScaleG2[irow].sumCov/vecScaleG2[irow].cnt);
        }
    }

    dstFilePath.cut(0);hp.reqAddFile(dstFilePath, "scale_gr3_coverage.csv");sFile::remove(dstFilePath);
    {
        sFil outScaleFile(dstFilePath);
        outScaleFile.printf("Bin,Coverage\n");

        for ( idx irow=0; irow < vecScaleG3.dim(); ++irow)
        {
            if(vecScaleG3[irow].cnt)
                outScaleFile.printf("%.2f,%f\n", (real)(100*(irow+1))/vecScaleG3.dim(), vecScaleG3[irow].sumCov/vecScaleG3[irow].cnt);
        }
    }

    return true;
}

bool targetedSeqQC::collectStats()
{
    hp.progress100Start=0;
    hp.progress100End=90;
    hp.logOut(sHiveProc::eQPLogType_Info, "Collecting targetedQC statistics\n");
    hp.logOut(sHiveProc::eQPLogType_Info, "Generating coverage histogram\n");
    std::unique_ptr<CoverageHist> hist;
    switch (target_mode) {
        case eTargetingOff:
            hist.reset(new CoverageHist(Qry->getlongCount()));
            break;
        case eTargetingOn:
            hist.reset(new TargetedCoverageHist(wander, Qry->getlongCount()));
            break;
        default:
            hist.reset(new TargetedCoverageHist(wander, Qry->getlongCount()));
            break;
    }
    idx total_sub_count = Sub->dim();
    for (idx sub_num = 0; sub_num < total_sub_count; ++sub_num) {
        hist->addSubject(*alignments, sub_num);
        hp.reqProgress(total_sub_count, sub_num, total_sub_count);
    }
    hist->sort();
    hp.progress100Start=90;
    hp.progress100End=100;
    hp.logOut(sHiveProc::eQPLogType_Info, "Normalizing coverage histogram\n");
    NormCoverageHist norm_hist(*hist);
    hp.logOut(sHiveProc::eQPLogType_Info, "Generating cumulative normalized coverage table\n");
    real norm_cov_values[11] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
                                 1.0, 1.5, 2.0, 2.5, 3.0 };
    sVec<real> norm_cov_input;
    norm_cov_input.resize(11);
    for (idx i = 0; i < 11; ++i)
        norm_cov_input[i] = norm_cov_values[i];
    CumulNormCoverageTable norm_table(norm_hist, norm_cov_input);
    hp.logOut(sHiveProc::eQPLogType_Info, "Generating mean sequencing coverage required table\n");
    sVec<real> base_frac_input;
    base_frac_input.resize(2);
    base_frac_input[0] = 0.9;
    base_frac_input[1] = 0.8;
    CoverageReqTable cov_req_table(norm_table, base_frac_input);
    hp.logOut(sHiveProc::eQPLogType_Info, "Printing coverage histogram\n");
    {
        sStr filename;
        hp.reqAddFile(filename, "targetedqc.cov_hist.csv");
        sFil file(filename);
        hist->printCSV(file);
    }
    hp.logOut(sHiveProc::eQPLogType_Info, "Printing coverage statistics\n");
    {
        sStr filename;
        hp.reqAddFile(filename, "targetedqc.cov_stats.csv");
        sFil file(filename);
        hist->printCoverageStats(file);
    }
    hp.logOut(sHiveProc::eQPLogType_Info, "Printing read hit statistics\n");
    {
        sStr filename;
        hp.reqAddFile(filename, "targetedqc.hit_stats.csv");
        sFil file(filename);
        hist->printHitStats(file);
    }
    hp.logOut(sHiveProc::eQPLogType_Info, "Printing cumulative normalized coverage table\n");
    {
        sStr filename;
        hp.reqAddFile(filename, "targetedqc.norm_cov_table.csv");
        sFil file(filename);
        norm_table.printCSV(file);
    }
    hp.logOut(sHiveProc::eQPLogType_Info, "Printing mean sequencing coverage required table\n");
    {
        sStr filename;
        hp.reqAddFile(filename, "targetedqc.cov_req_table.csv");
        sFil file(filename);
        cov_req_table.printCSV(file);
    }
    hp.reqProgress(0, 100, 100);
    return true;
}

void DnaAlignQC::cleanUp()
{
    if(qc->err_buf.length()) {
        logOut(eQPLogType_Error,"%s",qc->err_buf.ptr());
        reqSetInfo(reqId, eQPInfoLevel_Error, "%s",qc->err_buf.ptr());
        reqSetStatus(reqId, eQPReqStatus_ProgError);
    }
}

idx DnaAlignQC::OnExecute(idx req)
{
    qc.reset(new alignQC(*this));
    if ( isRNASeq() ) {
        qc.reset(new rnaSeqQC(*this));
    } else if( isTargeted() ) {
        qc.reset(new targetedSeqQC(*this));
    }

    if( !qc->init() ) {
        cleanUp();
        return 0;
    }

    if ( !qc->collectStats() ) {
        cleanUp();
        return 0;
    }

    cleanUp();
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;

}

sRC DnaAlignQC::OnSplit(idx req, idx &cnt)
{
    if (isRNASeq() || isTargeted()) {
        cnt = 1;
        return sRC::zero;
    }

    idx maxAlignments = formIValue("chunkSize", 500000);

    sStr tmp_buf;
    sHiveId alignerID(formValue("alignQC_objid"));
    sUsrFile aligner(alignerID, user);
    aligner.getFilePathname00(tmp_buf, "alignment.hiveal" _ "alignment.vioal" __);
    sHiveal * alignments = new sHiveal(user, tmp_buf, sBioseq::eBioModeLong);
    if(!alignments || alignments->dimAl() == 0) {
        reqSetInfo(req, eQPInfoLevel_Error, "Cannot load alignment file to determine chunk size");
        reqSetStatus(req, eQPReqStatus_ProgError);
        delete alignments;
        return RC(sRC::eSplitting,sRC::eRequest,sRC::eBlob, sRC::eUndefined);
    }

    cnt = (alignments->dimAl() / maxAlignments) + 1;

    delete alignments;
    return sRC::zero;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    const char * srv_name = sQPrideProc::QPrideSrvName(&tmp, "dna-alignQC", argv[0]);

    if( strstr(argv[0], "dna-rnaSeqQC") ) {
        tmp.cut(0);
        srv_name = sQPrideProc::QPrideSrvName(&tmp, "dna-rnaSeqQC", argv[0]);
    } else if( strstr(argv[0], "dna-targetedSeqQC") ) {
        tmp.cut(0);
        srv_name = sQPrideProc::QPrideSrvName(&tmp, "dna-targetedSeqQC", argv[0]);
    }

    DnaAlignQC backend("config=qapp.cfg" __, srv_name );
    return (int) backend.run(argc, argv);
}
