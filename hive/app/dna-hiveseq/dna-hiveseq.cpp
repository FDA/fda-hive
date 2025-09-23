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
#include <qlib/QPrideProc.hpp>
#include <ulib/ulib.hpp>
#include <math.h>
#include <violin/violin.hpp>
#include <qlib/QPrideClient.hpp>
#include <dna-denovo/dna-denovoextension.hpp>
#include <qpsvc/archiver.hpp>
#include <violin/alignparse.hpp>


#define posMatrix( _v_i, _v_j, _v_len, _v_mat) _v_mat[(((_v_i)+1) * ((_v_len)+1)) + ((_v_j)+1)]

class DnaHiveseqProc: public sQPrideProc
{
        enum eSeqFlags
        {
            eSeqNone = 0x00000000,
            eSeqNoId = 0x00000001,
            eSeqRev = 0x00000002,
            eSeqComp = 0x00000004,
            eSeqKeepEnd = 0x00000008,
            eSeqKeepMid = 0x00000010,
            eSeqLowComplexity = 0x00000020,
            eSeqAdapters = 0x00000040,
            eSeqPrimers = 0x00000080,
            eSeqFastQ = 0x00000100,
            eSeqTrimming = 0x00000200,
            eSeqLengthFilter = 0x00000400,
            eAdapReverse = 0x00000800,
            eAdapComplement = 0x00001000,
            ePrimReverse = 0x00002000,
            ePrimComplement = 0x00004000,
            eSkipArchiver = 0x00008000,
            eSeqQuaFilter = 0x00010000,
            eHiveseq = 0x00020000,
            eRemoveFilter = 0x00040000,
            eAppendLength = 0x00080000,
            eSeqNegateFiltersOutput = 0x00100000,
            eSeqFilterNs = 0x00200000
        };
        idx seqFlags;
    public:
        DnaHiveseqProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv), seqFlags(0)
        {
        }

        virtual idx OnExecute(idx);

        bool getPrimerSequence(sStr * buf, const char * seq, idx start, idx len, idx iteration = 0, idx isrev = 0, idx iscomp = 0)
        {
            idx option = (2 * iscomp) + isrev;
            idx rev = 0, comp = 0;
            switch(option) {
                case 0: {
                    rev = 0, comp = 0;
                    break;
                }
                case 1: {
                    rev = iteration, comp = 0;
                    break;
                }
                case 2: {
                    rev = 0, comp = iteration;
                    break;
                }
                case 3: {
                    rev = iteration % 2, comp = iteration / 2;
                    break;
                }
            }
            buf->cut(0);
            sBioseq::uncompressATGC(buf, seq, start, len, true, 0, rev, comp);
            return rev;
        }

        bool parseDicFile(sStr *idlistContainer, const char * idlistObjId, sStr *errmsg)
        {
            sStr flnm, idtemp;
            sUsrFile obj(idlistObjId, user);
            if( !obj.Id() ) {
                errmsg->printf("Can't access ObjId: %s", idlistObjId);
                return false;
            }
            obj.getFile(flnm);
            if( !sFile::exists(flnm) ) {
                errmsg->printf("Can't access File or it doesn't exist: %s", flnm.ptr());
                return false;
            }
            sFil idlistFile(flnm, sMex::fReadonly);
            if( !idlistFile ) {
                errmsg->printf("No data for sequence file %s; terminating\n", idlistObjId);
            }
            sString::searchAndReplaceSymbols(idlistContainer, idlistFile, 0, ";" sString_symbolsEndline, 0, 0, true, true, false, false);
            return true;
        }

        bool printSequenceformat(sBioseq &Sub, sFil * outFile, idx row, idx start, idx length, bool keepOriginalId, sVec<idx> * minLength, sVec<idx> * maxLength)
        {
            if( row < 0 || row > Sub.dim() ) {
                return 0;
            }
            idx seqlen = Sub.len(row);
            if( start < 0 ) {
                start = seqlen + start;
            }
            if( start > seqlen ) {
                return 0;
            }
            idx end = length ? start + length : seqlen;
            if( end > seqlen ) {
                end = seqlen;
            }
            sStr idstr((seqFlags & eSeqFastQ) ? "@" : ">");
            bool quaBit = Sub.getQuaBit(row);

            const char * seqid = Sub.id(row);
            if( seqid[0] == '>' || seqid[0] == '@' ) {
                ++seqid;
            }
            idx subrpt = 1;
            if( Sub.getmode() == 0 ) {
                subrpt = Sub.rpt(row);
            }
            if( keepOriginalId == false ) {
                idstr.printf("%" DEC, row);
            } else {
                idstr.printf("%s", seqid);
            }

            sStr seqstr, finalseq;
            seqstr.cut(0);
            finalseq.cut(0);
            const char * seqs = Sub.seq(row);
            sBioseq::uncompressATGC(&seqstr, seqs, 0, seqlen, true, 0, (seqFlags & eSeqRev), (seqFlags & eSeqComp));
            const char * seqqua = Sub.qua(row);

            if( seqqua) {
                if( quaBit ) {
                    for(idx i = start; i < end; ++i) {
                        if( Sub.Qua(seqqua, i, true) == 0 ) {
                            seqstr[i] = 'N';
                        }
                    }
                } else {
                    for(idx i = start; i < end; ++i) {
                        if( seqqua[i] == 0 )
                            seqstr[i] = 'N';
                    }
                }
            }
            sStr quastr, finalqua;
            quastr.cut(0);
            finalqua.cut(0);
            if( seqFlags & eSeqFastQ ) {
                if( seqqua) {
                    for(idx i = start; i < end; ++i) {
                        quastr.printf("%c", seqqua[i] + 33);
                    }
                } else {
                    for(idx i = start; i < end; ++i) {
                        quastr.printf("%c", 30 + 33);
                    }
                }
            }

            idx newseqlen = 0;
            if (minLength && maxLength){
                idx iCount = sMin(minLength->dim(), maxLength->dim());
                idx elim = 0;
                for(idx i = start; i < end; ++i) {
                    bool isInRange = true;
                    for(idx j = 0; j < iCount; ++j) {
                        if( (idx) *minLength->ptr(j) - 1 <= i && i <= (idx) *maxLength->ptr(j) - 1 ) {
                            isInRange = false;
                            ++elim;
                            break;
                        }
                    }
                    if( isInRange ) {
                        finalseq.printf("%c", seqstr[i]);
                        if( seqFlags & eSeqFastQ ) {
                            finalqua.printf("%c", seqqua[i]);
                        }
                        ++newseqlen;
                    }
                }
            }
            else {
                newseqlen = end - start;
            }

            if( seqFlags & eAppendLength ) {
                idstr.printf(" len=%" DEC, newseqlen);
            }

            Sub.printFastXData(outFile, newseqlen, idstr.ptr(), finalseq.ptr(start), finalqua.ptr(0), subrpt);
            return true;
        }

        void registerParameters(DnaDenovoAssembly &dnv)
        {
            idx sizem = formIValue("denovoExtSizemer", 16);
            idx sizef = formIValue("denovoExtSizeFilter", 0);
            idx rptf = formIValue("denovoExtRptFilter", 0);
            bool firstStage = formBoolValue("denovoExtFirstStage", 0) ? true : false;
            idx limit = formIValue("denovoOutFilterLength", 0);
            idx missperc = formIValue("denovoMissmatchesPercentage", 2);
            dnv.setInitialParameters(sizem, sizef, rptf, firstStage, limit);
            dnv.setMissmatchesPercent(100 - missperc);
        }

        const char * getQuery00(sUsrObj & obj, sStr & query, const char * alSeparator = 0){
            if(!alSeparator)alSeparator=";";
            idx qpos = query.length();
            const char * hqry = obj.propGet("hiveseqQry", &query);
            query.add0();
            const char * objqry = obj.propGet00("objQry", &query, alSeparator);
            query.add0();
            if (hqry && objqry){
                return 0;
            }
            if (hqry){
                return hqry;
            }
            else if (objqry){
                return objqry;
            }
            query.cut(qpos);
            return 0;
        }

};

bool isReversePrimerID(const char *idseq)
{
    sStr id;
    id.printf("%s", idseq);
    const char *revString = "rev" __;
    char *pos;
    pos = sString::searchSubstring(id.ptr(), id.length(), revString, 1, __, true);
    if( pos ) {
        return true;
    } else
        return false;
}


idx DnaHiveseqProc::OnExecute(idx req)
{
    sStr form_buf;
    idx algorithms = formIValue("AlgorithmsName", 0);
    sBioseq::EBioMode mode = sBioseq::eBioModeLong;
    const idx modearg = formIValue("inputMode", mode);
    if( sBioseq::isBioModeShort( modearg ) ) {
        mode = sBioseq::eBioModeShort;
    } else if ( !sBioseq::isBioModeLong( modearg ) ) {
        logOut(eQPLogType_Error, "Unknown biomode: %" DEC, modearg);
        return false;
    }
    if( algorithms == 2 ) {
        mode = sBioseq::eBioModeShort;
    }
    if( algorithms && (algorithms == 7) ) {
        mode = sBioseq::eBioModeLong;
    }
    if( algorithms && (algorithms == 9) ) {
        mode = sBioseq::eBioModeLong;
    }
    const char * hqry = getQuery00(objs[0], form_buf);
    if( !hqry ) {
        logOut(eQPLogType_Error, "Hiveseq query is empty or there is a conflict with the input");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sHiveseq hs(user, hqry, mode);
    if( hs.dim() == 0 ) {
        logOut(eQPLogType_Error, "Hiveseq query is empty or not accessible: %s", hqry);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    logOut(eQPLogType_Debug, "hiveseq query requested for execution: %s", hqry);
    sBioseq &Sub = hs;
    bool isQ = false;
    if( Sub.getQuaBit(0) == false ) {
        isQ = true;
    }

    seqFlags |= (formBoolValue("primersactive", 0) ? eSeqPrimers : eSeqNone);
    seqFlags |= (formBoolValue("lowcomplexityactive", 0) ? eSeqLowComplexity : eSeqNone);
    seqFlags |= (formBoolValue("qualityactive", 0) ? eSeqQuaFilter : eSeqNone);
    seqFlags |= (formBoolValue("adaptersactive", 0) ? eSeqAdapters : eSeqNone);
    seqFlags |= (formBoolValue("primersReverse", 0) ? ePrimReverse : eSeqNone);
    seqFlags |= (formBoolValue("primersComplement", 0) ? ePrimComplement : eSeqNone);
    seqFlags |= (formBoolValue("adaptersReverse", 0) ? eAdapReverse : eSeqNone);
    seqFlags |= (formBoolValue("adaptersComplement", 0) ? eAdapComplement : eSeqNone);
    bool keepOriginalID = formBoolValue("keepOriginalID", 0) ? true : false;
    seqFlags |= (formBoolValue("isHiveseq", 0) ? eHiveseq : eSeqNone);
    seqFlags |= ((formBoolValue("isFastQFile", 0) && isQ && (algorithms != 2)) ? eSeqFastQ : eSeqNone);
    seqFlags |= (formBoolValue("appendLength", 0) ? eAppendLength : eSeqNone);
    seqFlags |= (formBoolValue("negateOutput", 0) ? eSeqNegateFiltersOutput : eSeqNone);
    seqFlags |= (formBoolValue("filternsactive", 0) ? eSeqFilterNs : eSeqNone);
    sBioseq::ESeqDirection revCompFlag = static_cast<sBioseq::ESeqDirection>(formIValue("isRevComp", (idx)sBioseq::eSeqForward));
    idx filterNsperc = formIValue("filterseqns", 0);


    switch(revCompFlag) {
        case sBioseq::eSeqReverse:
            seqFlags |= eSeqRev;
            break;
        case sBioseq::eSeqForwardComplement:
            seqFlags |= eSeqComp;
            break;
        case sBioseq::eSeqReverseComplement:
            seqFlags |= (eSeqRev | eSeqComp);
            break;
        case sBioseq::eSeqForward:
        default:
            break;
    };
    {
        idx keepvar = formIValue("primersKeep", 0);
        if( keepvar == 1 ) {
            seqFlags |= (eSeqKeepMid);
        } else if( keepvar == 2 ) {
            seqFlags |= (eSeqKeepEnd);
        } else if( keepvar == 3 ) {
            seqFlags |= (eSeqKeepMid | eSeqKeepEnd);
        }
    }

    idx threshold = formIValue("qualityThreshold", 50);
    idx percentage = formIValue("qualityPercentage", 100);
    idx complexityWindow = formIValue("lowcomplexityWindow", 16);
    real complexityEntropy = formRValue("lowcomplexityEntropy", 1);
    idx cfOption = formIValue("lowcomplexityOption", 0);

    udx minSeqLen = formUValue("minimumSeqLength", 0);
    ;
    if( minSeqLen > 0 ) {
        seqFlags |= eSeqLengthFilter;
    }

    idx trimMin = formIValue("trimMinimum", -1);
    idx trimMax = formIValue("trimMaximum", -1);
    if( trimMin > 0 || trimMax > 0 ) {
        if( (trimMax != -1) && (trimMin > trimMax) ) {
            idx temp = trimMax;
            trimMax = trimMin;
            trimMin = temp;
        }
        seqFlags |= eSeqTrimming;
    }

    sVec<idx> removeMinVec, removeMaxVec;
    formIValues("removeMin", &removeMinVec);
    formIValues("removeMax", &removeMaxVec);
    if( removeMinVec.dim() != 0 && removeMinVec[0] != -1 ) {
        seqFlags |= eRemoveFilter;
    }

    idx randcounter = formIValue("randomizerNumValue", 0);
    idx randminValue = formIValue("randomizerMinLength", 100);
    idx randmaxValue = formIValue("randomizerMaxLength", 100);
    real randNoise = formRValue("randomizerNoise", 0);
    bool randLengthNorm = formBoolValue("randLengthNorm", false);
    const char * randmutations = formValue("randomizerMutations", &form_buf);
    const char * randsettings = formValue("randomizerSettings", &form_buf);

    idx adaptersmismatches = formIValue("adaptersMaxmissmatches", 1);
    idx adaptersminLength = formIValue("adaptersMinLength", 100);
    sBioseq *adapters = 0;
    sVec<idx> isReverseAdapter;
    if( seqFlags & eSeqAdapters ) {
        const char * adaptersObjId = formValue("adaptersObjId", &form_buf);
        adapters = new sHiveseq(user, adaptersObjId);
        if( adapters->dim() == 0 ) {
            logOut(eQPLogType_Error, "Incorrect Adapters object Id:\n%s", adaptersObjId);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        isReverseAdapter.cut(0);
        for(idx i = 0; i < adapters->dim(); i++) {
            const char *idseq = adapters->id(i);
            idx isRev = isReversePrimerID(idseq);
            isReverseAdapter.vadd(1, isRev);
        }

    }

    idx primersmismatches = formIValue("primersMaxmissmatches", 2);
    idx primersminLength = formIValue("primersMinLength", 10);
    sBioseq *primers = 0;
    sVec<idx> isReversePrimer;

    if( seqFlags & eSeqPrimers ) {
        const char * primersObjId = formValue("primersObjId", &form_buf);
        primers = new sHiveseq(user, primersObjId);
        if( primers->dim() == 0 ) {
            logOut(eQPLogType_Error, "Incorrect Primers object Id:\n%s", primersObjId);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        isReversePrimer.cut(0);
        for(idx i = 0; i < primers->dim(); i++) {
            const char *idseq = primers->id(i);
            idx isRev = isReversePrimerID(idseq);
            isReversePrimer.vadd(1, isRev);
        }

    }

    logOut(eQPLogType_Debug, "\n Hiveseq is running fine and we have: %" DEC " sequences\n", Sub.dim());
    sStr errmsg;
    sStr warningmsg;

    sStr hsLocation, extension, hsOutfile;
    cfgStr(&hsLocation, 0, "user.download");
    hsLocation.printf("%" DEC "/", req);
    if( !sDir::makeDir(hsLocation) ) {
        reqSetInfo(req, eQPInfoLevel_Error, "internal error %" UDEC, (udx) __LINE__);
        logOut(eQPLogType_Error, "Failed to mkdir '%s'\n", hsLocation.ptr());
        return 0;
    }
    if( seqFlags & eHiveseq ) {
        extension.printf("hiveseq");
    } else if( seqFlags & eSeqFastQ ) {
        extension.printf("fastq");
    } else {
        extension.printf("fasta");
    }

    hsOutfile.printf("%shs%" DEC ".%s", hsLocation.ptr(), req, extension.ptr());

    sStr namefile;
    const char *nmfile = formValue("name", &form_buf);
    if( !nmfile && *nmfile ) {
        namefile.printf("hs-%" DEC, req);

        if( algorithms == 0 ) {
            if( seqFlags & eSeqTrimming ) {
                namefile.printf("-trim");
            }
            if( seqFlags & eSeqQuaFilter ) {
                namefile.printf("-qua");
            }
            if( seqFlags & eSeqLowComplexity ) {
                namefile.printf("-lowcomp");
            }
            if( seqFlags & eSeqPrimers ) {
                namefile.printf("-primers");
            }
            if( seqFlags & eSeqAdapters ) {
                namefile.printf("-adapters");
            }
            if( seqFlags |= eRemoveFilter ) {
                namefile.printf("-removing");
            }

        } else if( algorithms == 1 ) {
            namefile.printf("-rand");
        } else if( algorithms == 2 ) {
            namefile.printf("-contigext");
        } else if( algorithms == 7 ) {
            namefile.printf("-idFilter");
        } else if( algorithms == 9 ) {
            namefile.printf("-dupidFilter");
        }
    } else {
        namefile.printf("%s", nmfile);
    }
    namefile.printf(".%s", extension.ptr());

    sFil fp(hsOutfile);
    progress100Start = 0;
    progress100End = 100;

    idx seqCount = 0;
    if( fp.ok() ) {
        fp.cut(0);
        if( seqFlags & eHiveseq ) {
            fp.printf("%s", hqry);
            seqCount = 1;
        }
        else if( algorithms == 0 ) {
            for(idx iq = 0; iq < Sub.dim(); ++iq) {
                idx len = Sub.len(iq);
                idx startlen = 0;
                idx endlen = len;
                bool validSeq = true;
                if( len == 0 ) {
                    continue;
                }
                const char * seq = Sub.seq(iq);
                const char * qua = Sub.qua(iq);
                bool isQual = qua ? true : false;


                if( (validSeq) && (seqFlags & eSeqFilterNs) ) {
                        idx nCount = Sub.countNs(iq);
                        if (!filterNsperc && nCount){
                            validSeq = false;
                        }
                        else if ( filterNsperc && (nCount >= (idx)((len * filterNsperc * 10) / 1000)) ) {
                            validSeq = false;
                        }
                    }

                if( (validSeq) && (seqFlags & eSeqTrimming) ) {
                    if( trimMin > 0 ) {
                        startlen = trimMin;
                    }
                    if( (trimMax > 0) && (trimMax < endlen) ) {
                        endlen = trimMax;
                    }
                }
                if( (validSeq) && (seqFlags & eSeqQuaFilter) && isQual ) {
                    validSeq = sFilterseq::qualityFilter(qua, len, threshold, percentage);
                }
                if( (validSeq) && (seqFlags & eSeqLowComplexity) ) {

                    if (cfOption == 0){
                        idx res = sFilterseq::complexityFilter(seq, len, complexityWindow, complexityEntropy);
                        if (res == -1){
                            validSeq = false;
                        }
                    }
                    if (cfOption == 1 || cfOption == 3){
                        startlen = sFilterseq::complexityFilter(seq, len, complexityWindow, complexityEntropy, true, 1);
                    }
                    if (cfOption == 2 || cfOption == 3){
                        endlen = sFilterseq::complexityFilter(seq, len, complexityWindow, complexityEntropy, true, 2);
                    }
                }
                if( (validSeq) && adapters && (seqFlags & eSeqAdapters) ) {
                    bool keepIfMid = true;
                    bool keepIfEnd = true;

                    for(idx iadapt = 0; iadapt < adapters->dim(); ++iadapt) {
                        const char * adap = adapters->seq(iadapt);
                        bool reverse = isReverseAdapter[iadapt];
                        idx adlen = adapters->len(iadapt);
                        idx iprim = 0;
                        idx isReverse = (seqFlags & eAdapReverse) ? 1 : 0;
                        idx isComplement = (seqFlags & eAdapComplement) ? 1 : 0;
                        idx numChecks = (isReverse + 1) * (isComplement + 1);

                        sStr t;
                        t.cut(0);
                        sBioseq::uncompressATGC(&t, seq, 0, len, true, 0);
                        sStr adapter;
                        do {
                            adapter.cut(0);
                            sBioseq::uncompressATGC(&adapter, adap, 0, adlen, true, 0);
getPrimerSequence(&adapter, adap, 0, adlen, iprim, isReverse, isComplement);

                            idx res = sFilterseq::primersFilter(t, len, adapter, adlen, (adaptersminLength > adlen) ? adlen : adaptersminLength, adaptersmismatches, reverse, keepIfMid, keepIfEnd, adlen + 1);
                            if( res == 0 ) {
                                validSeq = false;
                                break;
                            } else if( res > 0 ) {
                                if( !reverse ) {
                                    startlen = res;
                                } else {
                                    endlen = res;
                                }
                                break;
                            }
                            ++iprim;
                        } while( iprim < numChecks );
                    }
                }
                if( (validSeq) && primers && (seqFlags & eSeqPrimers) ) {
                    bool keepIfMid = (seqFlags & eSeqKeepMid) ? true : false;
                    bool keepIfEnd = (seqFlags & eSeqKeepEnd) ? true : false;
                    idx isReverse = (seqFlags & ePrimReverse) ? 1 : 0;
                    idx isComplement = (seqFlags & ePrimComplement) ? 1 : 0;
                    idx numChecks = (isReverse + 1) * (isComplement + 1);

                    sStr t;
                    t.cut(0);
                    sBioseq::uncompressATGC(&t, seq, 0, len, true, 0);

                    for(idx iter = 0; (iter < primers->dim()) && validSeq; ++iter) {
                        const char * pr = primers->seq(iter);
                        bool reverse = isReversePrimer[iter];
                        idx prlen = primers->len(iter);

                        idx iprim = 0;

                        sStr primer;
                        do {
                            primer.cut(0);
                            bool inv = getPrimerSequence(&primer, pr, 0, prlen, iprim, isReverse, isComplement);

                            reverse = (reverse ^ inv);

                            idx res = sFilterseq::primersFilter(t, len, primer, prlen, (primersminLength > prlen) ? prlen : primersminLength, primersmismatches, reverse, keepIfMid, keepIfEnd, 0);
                            if( res == 0 ) {
                                validSeq = false;
                                break;
                            } else if( res > 0 ) {
                                if( !reverse ) {
                                    startlen = res;
                                } else {
                                    endlen = res;
                                }
                                break;
                            }
                            ++iprim;
                        } while( iprim < numChecks );
                    }
                }
                if( (validSeq) && (seqFlags & eSeqLengthFilter) ) {
                    if( (endlen - startlen) < (idx) minSeqLen ) {
                        validSeq = false;
                    }
                }
                if( endlen <= startlen ) {
                    validSeq = false;
                }
                if (seqFlags & eSeqNegateFiltersOutput){
                    validSeq = !validSeq;
                }

                if( validSeq ) {
                    if (iq % 5000){
                        reqProgress(iq, iq, Sub.dim());
                    }
                    PERF_START("printfastXRow");
                    if( seqFlags & eRemoveFilter ) {
                        printSequenceformat(Sub, &fp, iq, startlen, endlen - startlen, keepOriginalID, &removeMinVec, &removeMaxVec);
                    } else {
                        Sub.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, iq, startlen, endlen - startlen, 0, keepOriginalID, (seqFlags & eAppendLength), 0, 0, revCompFlag);
                    }
                    PERF_END();

                    ++seqCount;
                }
            }
        } else if( algorithms == 1 ) {
            if( randminValue <= 0 ) {
                randminValue = 1;
            }
            complexityWindow = formBoolValue("lowcomplexityactive", 0) ? complexityWindow : 0;
            bool randNrevcomp = formBoolValue("randomizerRevComp", 0) ? false : true;

            sFilterseq::randomizer(fp, Sub, randcounter, ((real) randNoise / 100.), randminValue, randmaxValue, randNrevcomp, seqFlags & eSeqFastQ, randLengthNorm, randsettings, randmutations, complexityWindow, complexityEntropy, this,
                reqProgressStatic);
            seqCount = randcounter;
        } else if( algorithms == 2 ) {

            BioseqTree tree(&Sub, 0);
            DnaDenovoAssembly dnv(this);
            registerParameters(dnv);
            idx err = dnv.dnaDenovoExtension(Sub, tree);

            dnv.printResult(&fp, &Sub, 0, false, nmfile);
            seqCount = dnv.getfinalCount();

            if( err ) {
                errmsg.printf("contig extension error: %" DEC, err);
            }
        } else if( algorithms == 3 ) {

            idx minmatchlen = formIValue("pecMinMatchLen", 15);
            bool keepAlignments = formBoolValue("pecKeepAlignments", 0);
            idx qualityOption = formIValue("pecQualities", 0);
            const char * secondObjId = formValue("pecPairEndFile", &form_buf);
            idx filterNsOption = formIValue("pairEndColFilterNs", 0);
            bool concatenateReads = formBoolValue("pairEndColExtendReads", 0);


            sHiveseq Sub2(user, secondObjId, sBioseq::eBioModeLong);
            if( Sub2.dim() == 0 ) {
                logOut(eQPLogType_Error, "Paired end file is invalid:%s\n", secondObjId);
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            idx subdim = Sub.dim();
            if( Sub2.dim() < subdim ) {
                subdim = Sub2.dim();
            }

            sVec< idx > alignmentMap;
            idx curAlignmentMapOfs=0;
            idx * matchTrain = 0;
            sBioseqAlignment::Al * hdr=0;


            sVec <idx> costMatrix;
            sVec <idx> dirMatrix;
            idx res[4];
            sStr seqstr, quastr;

            sStr idstr;
            seqCount = 0;
            for(idx iq = 0; iq < subdim; ++iq) {
                idx seqlen1 = Sub.len(iq);
                idx seqlen2 = Sub2.len(iq);

                if( seqlen1 == 0 || seqlen2 == 0 ) {
                    continue;
                }
                const char * seq1 = Sub.seq(iq);
                const char * seq2 = Sub2.seq(iq);



                idx diagonalFilter = 0;
                if (filterNsOption >= 0){
                    idx nCount = Sub.countNs(iq);
                    if (((filterNsOption == 0) && nCount) || ( nCount >= ((seqlen1 * filterNsOption * 10) / 1000)) ) {
                        continue;
                    }
                    nCount = Sub2.countNs(iq);
                    if (((filterNsOption == 0) && nCount) || ( nCount >= ((seqlen2 * filterNsOption * 10) / 1000)) ) {
                        continue;
                    }
                }


                costMatrix.resize((seqlen1+1)*(seqlen2+1));
                dirMatrix.resize((seqlen1+1)*(seqlen2+1));


                idx lenalign = sFilterseq::adaptiveAlignment (seq1, seqlen1, seq2, seqlen2, 5, -4, -12, res, costMatrix, dirMatrix, diagonalFilter);


                if (lenalign > minmatchlen){

                    if (keepAlignments){
                        curAlignmentMapOfs=alignmentMap.dim();
                        alignmentMap.addM(sizeof(sBioseqAlignment::Al)/sizeof(idx)+lenalign*2);
                        hdr =(sBioseqAlignment::Al*)alignmentMap.ptr(curAlignmentMapOfs);
                        matchTrain=alignmentMap.ptr(curAlignmentMapOfs+sizeof(sBioseqAlignment::Al)/sizeof(idx));
                        sSet(hdr);
                        hdr->setLenAlign(lenalign);
                        hdr->setIdSub(iq);hdr->setIdQry(iq);
                        hdr->setFlags(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignBackward);
                        idx score= posMatrix (res[3],res[1] , seqlen1, costMatrix.ptr(0));
                        hdr->setScore(score);
                        hdr->setDimAlign(2*lenalign);
                    }

                    seqstr.cut(0);
                    quastr.cut(0);
                    idstr.cut(0);
                    const char * id = Sub.id(iq);
                    const char * qua1 = Sub.qua(iq);
                    bool quaBit1 = Sub.getQuaBit(iq);
                    const char * qua2 = Sub2.qua(iq);
                    bool quaBit2 = Sub2.getQuaBit(iq);
                    idstr.printf("%s%s", (seqFlags & eSeqFastQ) ? "@" : ">", id);

                    if (res[0] != 0){
                        idx seqposition = seqstr.length();
                        sBioseq::uncompressATGC(&seqstr, seq1, 0, res[0], true, 0);
                        if(qua1) {
                            char * seqpos = seqstr.ptr(seqposition);
                            for(idx i = 0; i < res[0]; ++i) {
                                if( Sub.Qua(qua1, i, quaBit1) == 0 ) {
                                    seqpos[i] = 'N';
                                }
                            }
                         }
                        if( seqFlags & eSeqFastQ ) {
                            if( qua1 && !quaBit1) {
                                for(idx i = 0; i < res[0]; ++i) {
                                    quastr.printf("%c", qua1[i] + 33);
                                }
                            } else {
                                for(idx i = 0; i < res[0]; ++i) {
                                    quastr.printf("%c", 30 + 33);
                                }
                            }
                        }
                    }

                    idx direction;

                    idx si1 = res[1];
                    idx si2 = res[3];
                    char qbase = 0;
                    char readbase = 0;

                    seqstr.add(0, lenalign);
                    quastr.add(0, lenalign);
                    idx index = seqstr.length()-1;
                    idx ii1=0;
                    while ( posMatrix (si2, si1, seqlen1, costMatrix.ptr(0)) > 0){

                        if (keepAlignments && matchTrain){
                            matchTrain[2*lenalign-1-(ii1*2)-1]=si1;
                            matchTrain[2*lenalign-1-(ii1*2)]=si2;
                            ++ii1;
                        }
                        direction = posMatrix (si2, si1, seqlen1, dirMatrix.ptr(0));
                        if (direction == 0){
                            if( seqFlags & eSeqFastQ) {
                                idx diff = qua1[si1] - qua2[seqlen2-1 - si2];
                                readbase = (diff > 0) ? sBioseqAlignment::_seqBits(seq1, si1) : sBioseqAlignment::_seqBits(seq2, seqlen2-1-si2, sBioseqAlignment::fAlignForwardComplement | sBioseqAlignment::fAlignForward);
                                if (qualityOption == 0) {
                                    qbase = (qua1[si1] + qua2[seqlen2-1 - si2])/2;
                                }
                                else if (qualityOption == 1) {
                                    qbase = (diff > 0) ? qua1[si1] : qua2[seqlen2-1 -si2];
                                }
                                else {
                                    qbase = (diff < 0) ? qua1[si1] : qua2[seqlen2-1 -si2];
                                }
                            }
                            else {
                                readbase = sBioseqAlignment::_seqBits(seq1, si1);
                            }
                            --si1; --si2;
                        }
                        else if (direction == 1){
                            readbase = sBioseqAlignment::_seqBits(seq2, seqlen2-1-si2, sBioseqAlignment::fAlignForwardComplement | sBioseqAlignment::fAlignForward);
                            if( seqFlags & eSeqFastQ ) {
                                qbase = qua2[seqlen2-1 - si2];
                            }
                            --si2;
                        }
                        else {
                            readbase = sBioseqAlignment::_seqBits(seq1, si1);
                            if( seqFlags & eSeqFastQ ) {
                                qbase = qua1[si1];
                            }
                            --si1;
                        }
                        if( seqFlags & eSeqFastQ ) {
                            quastr[index] = qbase + 33;
                        }
                        seqstr[index] = ( ( seqFlags & eSeqFastQ ) && !qbase) ? 'N' : sBioseq::mapRevATGC[(idx)readbase];
                        index--;
                    }
                    if ( (seqlen2-1 - res[3]) > 0){
                        idx ilen = seqstr.length();
                        idx seqposition = ilen;
                        seqstr.add(0, (seqlen2-1 - res[3]));
                        idx is;
                        for(is = seqlen2-1-res[3]-1; is >= 0; --is) {
                            seqstr[seqposition++] = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)((seq2[is/4]>>((is%4)*2)&0x3))]];
                        }
                        if(qua2) {
                            seqposition = ilen;
                            for(idx i = seqlen2-1-res[3]-1; i >= 0; --i, ++seqposition) {
                                if( Sub.Qua(qua2, i, quaBit2) == 0 ) {
                                    seqstr[seqposition] = 'N';
                                }
                            }
                         }
                        if( seqFlags & eSeqFastQ ) {
                            if( qua2 && !quaBit2) {
                                for(idx i = seqlen2-1-res[3]-1; i >= 0; --i) {
                                    quastr.printf("%c", qua2[i] + 33);
                                }
                            } else {
                                for(idx i = 0; i < seqlen2-1-res[3]; ++i) {
                                    quastr.printf("%c", 30 + 33);
                                }
                            }
                        }
                    }
                    idstr.printf(" r1[%" DEC "-%" DEC "]:align=%" DEC ":r2[%" DEC "-%" DEC "]", (idx)1, (res[0]+1), lenalign, seqlen2, (res[3] + 1));

                    if ( ( seqFlags & eSeqFastQ ) && (seqstr.length() != quastr.length()) ){
                        logOut(eQPLogType_Error,"failed to generate correct qualities for read number: % " DEC , iq);
                        reqSetInfo(req, eQPLogType_Error,"failed to generate correct qualities for read number: % " DEC , iq);
                        return 0;
                    }
                    Sub.printFastXData(&fp, seqstr.length(), idstr, seqstr, (seqFlags & eSeqFastQ) ? quastr.ptr(0) : 0, 1);
                    seqCount++;

                    if (keepAlignments){
                        idx dimalign=sBioseqAlignment::compressAlignment(hdr,matchTrain,matchTrain);
                        hdr->setDimAlign(dimalign);
                        alignmentMap.cut(curAlignmentMapOfs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+dimalign);
                    }
                }
                else if (concatenateReads){
                    seqstr.cut(0);
                    quastr.cut(0);
                    idstr.cut(0);
                    const char * id = Sub.id(iq);
                    const char * qua1 = Sub.qua(iq);
                    bool quaBit1 = Sub.getQuaBit(iq);
                    const char * qua2 = Sub2.qua(iq);
                    bool quaBit2 = Sub2.getQuaBit(iq);
                    idstr.printf("%s%s", (seqFlags & eSeqFastQ) ? "@" : ">", id);

                    idstr.printf(" r1[%" DEC "-%" DEC "]:align=%" DEC ":r2[%" DEC "-%" DEC "]", (idx)1, seqlen1, (idx)0, seqlen2, (idx)1);
                    idx seqposition = seqstr.length();
                    sBioseq::uncompressATGC(&seqstr, seq1, 0, seqlen1, true, 0);
                    if(qua1) {
                        char * seqpos = seqstr.ptr(seqposition);
                        for(idx i = 0; i < seqlen1; ++i) {
                            if( Sub.Qua(qua1, i, quaBit1) == 0 ) {
                                seqpos[i] = 'N';
                            }
                        }
                     }
                    if( seqFlags & eSeqFastQ ) {
                        if( qua1 && !quaBit1) {
                            for(idx i = 0; i < seqlen1; ++i) {
                                quastr.printf("%c", qua1[i] + 33);
                            }
                        } else {
                            for(idx i = 0; i < seqlen1; ++i) {
                                quastr.printf("%c", 30 + 33);
                            }
                        }
                    }
                    seqposition = seqstr.length();
                    sBioseq::uncompressATGC(&seqstr, seq2, 0, seqlen2, true, 0, true, true);
                    if(qua2) {
                        char * seqpos = seqstr.ptr(seqposition);
                        for(idx i = seqlen2-1; i >= 0; --i) {
                            if( Sub.Qua(qua2, i, quaBit2) == 0 ) {
                                seqpos[i] = 'N';
                            }
                        }
                     }
                    if( seqFlags & eSeqFastQ ) {
                        if( qua2 && !quaBit2) {
                            for(idx i = 0; i < seqlen2; ++i) {
                                quastr.printf("%c", qua2[seqlen2-1-i] + 33);
                            }
                        } else {
                            for(idx i = 0; i < seqlen2; ++i) {
                                quastr.printf("%c", 30 + 33);
                            }
                        }
                    }
                    if ( ( seqFlags & eSeqFastQ ) && (seqstr.length() != quastr.length()) ){
                        logOut(eQPLogType_Error,"failed to generate concatenated qualities for read number: % " DEC , iq);
                        reqSetInfo(req, eQPLogType_Error,"failed to generate concatenated qualities for read number: % " DEC , iq);
                        return 0;
                    }

                    Sub.printFastXData(&fp, seqstr.length(), idstr, seqstr, (seqFlags & eSeqFastQ) ? quastr.ptr(0) : 0, 1);
                    seqCount++;
                }
                if (!reqProgressStatic(this, iq, iq, subdim)){
                    errmsg.printf("process needs to be killed; terminating...");
                    break;
                }
            }
            if(seqCount && keepAlignments) {
                sUsrProc al_obj(*user, "svc-align-pairwise");
                al_obj.propSetHiveId("subject", hqry);
                al_obj.propSetHiveId("query", secondObjId);
                sUsrObj *alObj = &al_obj;
                alObj->propSet("submitter","dna-hexagon");
                alObj->propSet("name",namefile.ptr());

                AlignMapParser alignParser(*this, hs, Sub2);
                idx countAls = 0;
                sStr errmsg;
                if (sRC rc = alignParser.writeAls(alignmentMap, 0, 0, countAls, errmsg)) {
                    reqSetInfo(req, eQPLogType_Error, "Error writing alignment map: (Line %d): %s", __LINE__, rc.print());
                    return 0;
                }

                sVioal::digestParams params;
                params.flags= 0;
                params.countHiveAlPieces = 1000000;
                params.combineFiles = false;
                if (sRC rc = alignParser.joinAls(params, alObj)) {
                    reqSetInfo(req, eQPLogType_Error,"Failed to create hiveal from alignment map: (Line %d): %s", __LINE__, rc.print());
                    return 0;
                }

                reqSetInfo(req, eQPInfoLevel_Info,"An alignment objId = %s was created", alObj->IdStr(0));
            }
        } else if( algorithms == 7 ) {

            sDic<idx> idlist;
            sStr tempSubDir;
            bool listExclusion = false;

            if( Sub.dim() > 1000000 ) {
                if (cfgStr(&tempSubDir, 0, "qm.tempDirectory")) {
                    tempSubDir.printf("%" DEC "-%s", reqId, "subids.dic");
                    idlist.init(tempSubDir);
                    if ( !idlist.mex()->ok() ) {
                        errmsg.printf("Unable to create subject id dictionary");
                    }
                } else {
                    errmsg.printf("Unable to receive temporary directory");
                }
            }

            if ( !errmsg ) {
                const char * idlistObjId = formValue("idlistObjId", &form_buf);

                sStr idlistContainer;
                bool parseDicValidation = parseDicFile(&idlistContainer, idlistObjId, &errmsg);
                if ( parseDicValidation && idlistContainer.length() && !errmsg ) {
                    idx end = idlistContainer.length();
                    while( idlistContainer[end - 1] == 0 ) {
                        end--;
                    }
                    idlist.parse00(idlistContainer, end);
                    listExclusion = (formIValue("listExclusion", 0) == 0) ? false : true;
                    if( parseDicValidation && idlist.dim() == 0 ) {
                        errmsg.printf("Dictionary is empty");
                    }
                }
            }

            if( !errmsg ) {

                sStr nonId;
                seqCount = 0;
                idx subdim = Sub.dim();
                idx idlistdim = idlist.dim();
                for(idx irow = 0; irow < subdim && ((seqCount < idlistdim) || (listExclusion)); ++irow) {
                    const char *id = Sub.id(irow);
                    idx pNum = 0;
                    idx row = sFilterseq::getrownum(idlist, id, 0, &pNum);
                    bool isValid = (row <= -1) ? false : true;

                    PERF_START("is Valid");
                    if( isValid ) {
                        *idlist.ptr(pNum) = -1;
                        if( !listExclusion ) {
                            Sub.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, irow, 0, 0, 0, keepOriginalID, (seqFlags & eAppendLength));
                            ++seqCount;
                        }
                    } else if( listExclusion ) {
                        Sub.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, irow, 0, 0, 0, keepOriginalID, (seqFlags & eAppendLength));
                        ++seqCount;
                    } PERF_END();PERF_START("REPORT");
                    if( irow % 100000 == 0 ) {
                        reqProgress(irow, irow, (subdim * 2));
                    }PERF_END();
                }
                bool rmIdfile = false;
                if( seqCount < idlist.dim() || listExclusion ) {
                    if( !listExclusion ) {
                        reqSetInfo(req, eQPInfoLevel_Info, "There are : %" DEC " Id's missing in the file\n", idlist.dim() - seqCount);
                    }
                    nonId.printf(0, "%shs-nonId%" DEC ".txt", hsLocation.ptr(), req);
                    sFil nId(nonId);
                    if( nId.ok() ) {
                        idx idmatches = 0;
                        nId.cut(0);
                        hsOutfile.cut(0);
                        hsOutfile.printf("%s", hsLocation.ptr());
                        nId.printf("# List of Id's that were not found in Process: %" DEC " \n", req);

                        for(idx i = 0; i < idlist.dim(); ++i) {
                            if( idlist[i] >= 0 ) {
                                idx sizeID;
                                const char * idl = (const char *) idlist.id(i, &sizeID);
                                nId.printf("%.*s\n", (int) sizeID, idl);
                                ++idmatches;
                            }
                        }
                        if (!idmatches){
                            rmIdfile = true;
                            reqSetInfo(req, eQPInfoLevel_Info, "All the Id's were found in the file: %" DEC "\n", idlist.dim());
                        }
                        else if( listExclusion ) {
                            reqSetInfo(req, eQPInfoLevel_Info, "%" DEC " Id's were not found in the file\n", idmatches);
                        }
                    } else {
                        errmsg.printf("dna-hiveseq can not create Id destination file");
                    }
                }
                if (rmIdfile){
                    sFile::remove(nonId);
                }
            }

            if ( tempSubDir.length() ) {
                idlist.destroy();
                if (!sFile::remove(tempSubDir)) {
                    errmsg.printf("Unable to delete temporary subject id dictionary");
                }
            }

        } else if ( algorithms == 8){
            bool considerRevComp = (formBoolValue("seqExclusionRevComp", false) == false) ? false : true;
            bool seqExclusion = (formIValue("seqExclusionOption", 0) == 0) ? false : true;
            const char * seqObjId = formValue("seqExcObjId", &form_buf);

            sHiveseq Qry(user, seqObjId, sBioseq::eBioModeShort);
            if( Qry.dim() == 0 ) {
                logOut(eQPLogType_Error, "Query is not accessible:%s\n", seqObjId);
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            BioseqTree bioseqtree(&Qry, 0);

            idx unique;
            idx uniqueCount = 0;
            progress100Start = 1;
            progress100End = 25;
            for(idx pos = 0; pos < Qry.dim(); ++pos) {

                if (pos % 5000){
                    reqProgress(pos, pos, Qry.dim());
                }

                idx len = Qry.len(pos);
                idx rpt = Qry.rpt(pos);

                unique = bioseqtree.addSequence(pos, len, rpt, 0);
                if( unique == -1 ) {
                    uniqueCount++;
                }
                if (considerRevComp){
                    unique = bioseqtree.addSequence(pos, len, rpt, 1);
                }
            }
            progress100Start = 25;
            progress100End = 50;
            for(idx pos = 0; pos < Sub.dim(); ++pos) {
                const char *seq = Sub.seq(pos);
                idx seqlen = Sub.len(pos);
                unique = bioseqtree.getSubNode(seq, seqlen, true, true);
                if( (unique == -1 && seqExclusion) || (unique != -1 && !seqExclusion)) {
                    if (pos % 5000){
                        reqProgress(pos, pos, Sub.dim());
                    }
                    Sub.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, pos, 0, seqlen, 0, keepOriginalID, (seqFlags & eAppendLength), 0, 0, revCompFlag);
                    ++seqCount;
                }
            }
            progress100Start = 50;
            progress100End = 100;

        } else if ( algorithms == 9){
            idx seqExclusion = formIValue("deduplicationlistExclusion", 0);
            const char * seqObjId = formValue("iddeduplicationlistObjId", &form_buf);

            sDic<idx> idlist;


            if (seqExclusion == 2){
                sStr mergeString;
                mergeString.add(hqry);
                mergeString.shrink00();
                if(*mergeString.ptr(mergeString.length()-1) == '\n'){
                    mergeString.cut(-1);
                }
                mergeString.addString(";");
                mergeString.addString(seqObjId);
                sHiveseq Sub_and_Qry(user, mergeString, sBioseq::eBioModeLong);
                if( Sub_and_Qry.dim() == 0 ) {
                    logOut(eQPLogType_Error, "Query is not accessible:%s\n", mergeString.ptr(0));
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }

                sStr tempidseq;
                for(idx iseq = 0; iseq < Sub_and_Qry.dim(); ++iseq) {
                    const char *idseq = Sub_and_Qry.id(iseq);
                    tempidseq.cut0cut(0);
                    idx tempidseqlen = sString::copyUntil(&tempidseq, idseq, sLen(idseq), sString_symbolsBlank);

                    *idlist.set(tempidseq.ptr(), tempidseqlen) = iseq;
                }

                for(idx i = 0; i < idlist.dim(); ++i) {
                    if( idlist[i] >= 0 ) {
                        idx irow = idlist[i];
                        Sub_and_Qry.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, irow, 0, 0, 0, keepOriginalID, (seqFlags & eAppendLength));
                        seqCount += 1;
                    }
                }
            }
            else{
                sHiveseq Qry(user, seqObjId, sBioseq::eBioModeLong);
                if( Qry.dim() == 0 ) {
                    logOut(eQPLogType_Error, "Query is not accessible:%s\n", seqObjId);
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }
                if( Qry.dim() > 1000000 ) {
                    sStr tempSubDir;
                    if (cfgStr(&tempSubDir, 0, "qm.tempDirectory")) {
                        tempSubDir.printf("%" DEC "-%s", reqId, "qryids.dic");
                        idlist.init(tempSubDir);
                        if ( !idlist.mex()->ok() ) {
                            errmsg.printf("Unable to create query id dictionary");
                        }
                    } else {
                        errmsg.printf("Unable to receive temporary directory");
                    }
                }

                sStr tempidseq;
                if( !errmsg ) {
                    for(idx iseq = 0; iseq < Qry.dim(); ++iseq) {
                        const char *idseq = Qry.id(iseq);
                        tempidseq.cut0cut(0);
                        idx tempidseqlen = sString::copyUntil(&tempidseq, idseq, sLen(idseq), sString_symbolsBlank);

                        *idlist.set(tempidseq.ptr(), tempidseqlen) = iseq;
                    }
                }

                idx subdim = Sub.dim();
                idx idlistdim = idlist.dim();

                for(idx irow = 0; irow < subdim && ((seqCount < idlistdim) || (seqExclusion == 1)); ++irow) {
                    const char *id = Sub.id(irow);
                    tempidseq.cut0cut(0);
                    idx tempidseqlen = sString::copyUntil(&tempidseq, id, sLen(id), sString_symbolsBlank);
                    idx pNum = 0;
                    idx row = sFilterseq::getrownum(idlist, tempidseq.ptr(), tempidseqlen, &pNum);
                    bool isValid = (row == -1) ? false : true;

                    PERF_START("is Valid");
                    if( isValid ) {
                        *idlist.ptr(pNum) = -2;
                        if( seqExclusion == 0) {
                            Sub.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, irow, 0, 0, 0, keepOriginalID, (seqFlags & eAppendLength));
                            ++seqCount;
                        }
                    } else if( seqExclusion == 1 ) {
                        Sub.printFastXRow(&fp, (seqFlags & eSeqFastQ) ? true : false, irow, 0, 0, 0, keepOriginalID, (seqFlags & eAppendLength));
                        ++seqCount;
                    } PERF_END();PERF_START("REPORT");
                    if( irow % 100000 == 0 ) {
                        reqProgress(irow, irow, (subdim * 2));
                    }PERF_END();
                }
            }
            progress100Start = 50;
            progress100End = 100;
        }
    } else {
        errmsg.printf("dna-hiveseq can not create destination file");
    }PERF_PRINT();
    if( (algorithms == 0 || algorithms == 2 || algorithms == 3 || algorithms == 7 || algorithms == 8 || algorithms == 9) && seqCount == 0 ) {
        warningmsg.printf("output sequence set is empty");
    }

    if( !errmsg && !warningmsg) {
        if( !(seqFlags & eSkipArchiver) && fp.ok() ) {
            sStr src("hiveseq://%s", namefile.ptr());
            dmArchiver archHS(*this, hsOutfile.ptr(), src, 0, namefile.ptr());
            archHS.setScreenFlag(formIValue("launchScreening",1));
            archHS.setQCFlag(formIValue("launchQC",1));
            idx archReqId = archHS.launch(*user, grpId);
            logOut(eQPLogType_Info, "Launching dmArchiver request %" DEC " with %" DEC " sequences\n", archReqId, seqCount);
        }
        if( reqProgress(-1, 100, 100) ) {
            reqSetStatus(req, eQPReqStatus_Done);
        }
    } else if( warningmsg ) {
        reqSetInfo(req, eQPInfoLevel_Warning, "%s\n", warningmsg.ptr());
        if( reqProgress(-1, 100, 100) ) {
            reqSetStatus(req, eQPReqStatus_Done);
        }
    } else {
        reqSetInfo(req, eQPInfoLevel_Error, "%s\n", errmsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }
    return 1;
}

int main(int argc, const char *argv[], const char *envp[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaHiveseqProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-hiveseq", argv[0]));
    return (int) backend.run(argc, argv);

}
