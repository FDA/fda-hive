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
#include "slib/std.hpp"
#include <unistd.h>
#include <ion/sJson.hpp>

class sHIVENCBI {
    public:
    sStr log;
    idx debug;
    static const char * NCBI_BaseURL;
    static const char * Dataset_Base;
    sStr m_curlBuf;
    idx sleepSec,sleepStep,sleepGranules;
    sHIVENCBI () {
        debug=0;
        sleepSec=3;
        sleepGranules=2;
        sleepStep=1;
    }
    const char * eSearch(const char * db, const char * term , sStr * dst=0);
    const char * eFetch(const char * db, const char * id, const char * mode=0, sStr * dst=0);
    const char * eLink(const char * dbFrom, const char * id, const char * dbto, const char * linkname, sStr * dst=0);
    const char * id2Acc(const char * db, const char * ids, sStr * dst=0);

    const char * assembly2genome(const char * assembly, sStr * dst=0);
    
    const char * biosample(const char * acc, sStr * dst=0, sJson * json=0);

    const char * assm2biosample(const char * assmACC, sStr * dst, const char * json, sStr * seqLen);
    const char * getRefLen(const char * ref, sStr * dst);
    const char * biosample2SRA(const char * biosample, sStr * dst, const char * json);
    const char * biosample2assm(const char * bsAcc, sStr * dst, const char * json);
    const char * assm2genome(const char * assembly, sStr * dst, sStr * outRefseqAssembly);
    const char * getReferences(const char * assembly, sStr * dst, bool *isRefseq);
    const char * getRefSeqAssemblyAcc(const char * assembly, sStr * dst);

    const char * saveBiosampleData(const char * assmACC, sStr * dst=0);
    const char * getBiosampleData(const char * assmACC, sStr * dst=0, sJson * json=0);
    const char * parseJson(const char * json, sStr * dist=0);
    const char * srs2srr(const char * srsId, sStr * dst=0);
    bool getLineage(const char *taxID, sStr *strLineage=0, sStr *dst=0);
    bool flattenNcbiJson(const char * ncbiRawJson, sJson * strucJson, sStr * outJsonStr = 0, sStr * log = 0, const char * argos_ID=0);
    bool getUniProtData(const char *speciesName, sJson *strucJson);
    const char * searchUniProt(const char *speciesName, sStr * dst=0);
    const char * getUniProtProteomeID(const char *uniprotJson, sStr *buf=0, const char *speciesName=0);
    const char * mapUniProtID(const char *proteomeID, sStr * dst=0);
    const char * proteinInfoResource(const char *speciesName, sStr *dst=0);
    const char * assm2sra(const char * assmACC, sStr * dst=0, const char * json=0);
    const char * srr2biosampleAcc(const char * srr, sStr * dst=0);
    const char * eSummary(const char * db, const char * id, sStr * dst=0);
    const char * biosampleUid2Acc(const char * biosample_uid, sStr * dst=0);
    idx extractBioSampleAcc(const char * txt, sStr & out);
    const char * eSummaryXML(const char * db, const char * id, sStr * dst=0);
    const char * srr2biosampleAccXML(const char * srr, sStr * dst=0);
    const char * getBiosampleMeta(const char * bsAcc, sStr * dst=0, const char * json=0);
    const char * fetchGenbankData(const char * gbAcc, sStr * dst=0);
    const char * eFetchRunInfo(const char * sra_uids, sStr * dst=0);
    const char * sraUid2SRR(const char * sra_uids, sStr * dst=0);
    const char * biosampleUid2SRR(const char * biosample_uids, sStr * dst=0);
    const char * assembly2SRR(const char * assembly, sStr * dst=0);
    const char * nucUid2SRRFromDBLink(const char * nuc_uid, sStr * dst=0);
    const char * nucUid2SRRThroughBioSample(const char * nuc_uid, sStr * dst=0);
    const char * nucAcc2CurrentAcc(const char * nuc_acc, sStr * dst=0);
    const char * nucseq2BiosampleAcc(const char * nuc_acc, sStr * dst=0);
    const char * nucseq2SRR(const char * nuc_acc, sStr * dst=0);
    const char * nucseq2BiosampleAccOnce(const char * nuc_acc, sStr * dst=0);
    const char * nucseq2SRROnce(const char * nuc_acc, sStr * dst=0);
    const char * getLibaryInfo(const char *srr, sStr *instrumentModel, sStr *libStrategy, sStr *libSource, sStr *libSelection, sStr *libConstProtocol);
};

