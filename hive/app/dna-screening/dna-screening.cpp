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
#include <violin/violin.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>
#include <qpsvc/qpsvc-dna-alignx.hpp>
#include <qpsvc/screening.hpp>
#include <ssci/math/stat/stat.hpp>
#include <slib/std/file.hpp>
#include <dmlib/dmlib.hpp>


class censuScopeProc: public sQPrideProc
{
    public:
        censuScopeProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv), samplesize(1000), useAlignments(false)
        {
            sBioseq::initModule(sBioseq::eACGT);
        }
        ~censuScopeProc()
        {
        }
        virtual idx OnExecute(idx);

//        void launchHexagon(idx req, const char * qry, const char * sub);
        idx launchAlignment(idx isBlast, const char * hqrylist, const char * sublist, const char * alignSelector, idx Sample, idx priority = 0);
        idx CurateResult(sDic<idx> *taxCnt,sDic<idx> *accCnt, const char * sublist, idx isBlast, sHivealtax *idAlTax, const char * alignObjId);
        idx curateTextBasedFile (sDic<idx> *taxCnt,sDic<idx> *accCnt, sHivealtax *idAlTax, const char *objId);
        idx filterByRank(sStr *dstTax, const char * srcTax, const char * requiredRank);

        bool moveAlignments(const char * sublist, const char * svcName);

        const char * fixDataPaths(sStr *pathAl, const char * path, const char *separator = 0){
            for (const char * ptr = path; ptr && *ptr; ptr = sString::next00(ptr)) {
                if (sFile::exists(ptr)){
                    pathAl->printf("%s", ptr);
                    if(separator)pathAl->printf("%s",separator);
                    else pathAl->add0();
                }
                else {
                    sFilePath flnm (ptr, "%%flnm");
                    sStr dst;
                    objs[0].getFilePathname(dst, flnm.ptr(0));
                    if (sFile::exists(dst.ptr())){
                        pathAl->printf("%s", dst.ptr());
                        if(separator)pathAl->printf("%s",separator);
                        else pathAl->add0();
                    }
                }
            }
            pathAl->add0(2);
            return pathAl->ptr(0);
        }
    private:
        idx samplesize;
        bool useAlignments;

        struct RanksAndWeights
        {
                const char * rankName;
                real weight;
                idx totalPopulation;
        };
        // i have to count the whole population of each node that we have in the tax tree
        // TODO implement sliding dynamic weights
        RanksAndWeights rankList[NUM_RANKS];
};

bool censuScopeProc::moveAlignments(const char * sublist, const char * svcName)
{
    sStr pathAl;
    grpDataPaths(reqId, "req-alignment.hiveal", &pathAl, svcName, 0);
    //grpDataPaths(reqId, "alignx-qry-", &pathAl, svcName, ";");

    idx chunk = 0;
    //we have some files which is result from running blast against a random samples
    //ok, we want to parse them and accumulate the result
    //it seems that we want to have some statistical value so with this for loop we can
    sStr dst, prefile, filesToZip;
    sFilePath nsrc, ndir, flnm;
    sStr extension ("-alignment.hiveal");
    sStr hivealPath;
    reqAddFile(hivealPath, "alignment.hiveal");

    sFil hivealText (hivealPath);
    for(const char *src = pathAl.ptr(); src; src = sString::next00(src), ++chunk)
    {
        // We access the file and get the container
        sFil filecontent (src, sFil::fReadonly);
        hivealText.printf("%s", filecontent.ptr());
        nsrc.cut(0);
        ndir.cut(0);
        prefile.cut(0);
        ndir.makeName(src, "%%dir/");
        nsrc.makeName(src, "%%flnm");
        prefile.add(nsrc.ptr(0), nsrc.length() - extension.length());
        prefile.printf("*");
        sDir results;
        results.list(sFlag(sDir::bitFiles), ndir.ptr(), prefile.ptr(), 0, 0);
        for(const char * ptr = results; ptr && *ptr; ptr = sString::next00(ptr)) {
            flnm.cut(0);
            flnm.makeName(ptr, "%%flnm");
            dst.cut(0);

            reqAddFile(dst, flnm);
            sFile::rename(ptr, dst.ptr());
//                    sFile::copy(ptr, dst.ptr());
            logOut(eQPLogType_Info, "moving %s to %s\n", ptr, dst.ptr());
        }
    }

    if (true){  // we can put a flag to indicate if we want them zip or not
        sStr tempDir;
        dst.cut(0);
        reqAddFile(dst, "unaligned.zip");
        ndir.cut(0);
        ndir.makeName(dst, "%%dir/");
        sDir results;
        results.list(sFlag(sDir::bitFiles), ndir.ptr(), "*unaligned.fa", 0, 0);

        // We move all the unaligned reads to a temporary directory
        tempDir.printf(0,"%stemp/", ndir.ptr());
        sDir::removeDir(tempDir.ptr(), true);
        if( !sDir::makeDir(tempDir.ptr()) ) {
            logOut(eQPLogType_Error, "Could not create temporary directory");
            return false;
        }
        // Move the files into the directory
        for(const char *src = results.ptr(); src; src = sString::next00(src)){
            nsrc.cut(0);
            nsrc.makeName(src, "%%flnm");
            prefile.printf(0,"%s%s", tempDir.ptr(), nsrc.ptr());
            sFile::rename(src, prefile.ptr());
        }

        // We pack the whole temporary directory
        if( !dmLib::pack(tempDir, dst, dmLib::eZip)){//ndir, dst, dmLib::eZip, &log, &log, &zipFile) ) {
            logOut(eQPLogType_Info, "Compression of unaligned reads failed");
        }
        // Remove the directory
        sDir::removeDir(tempDir, true);
    }
    return true;
}

idx censuScopeProc::launchAlignment(idx isBlast, const char * hqrylist, const char * sublist, const char * alignSelector, idx Sample, idx priority)
{
    idx filterNs = formIValue("filterNs", 1);
    bool deepScanning = formBoolValue("deepScanning", 0);
    idx wordSize = formIValue("blastWordSize", 28);
    if (wordSize < 11){
        wordSize = 11;
    }
    idx reqsubmitAlgorithm = 0;

    sQPSvc * svc = 0;
    if( isBlast ) {
        svc = new QPSvcDnaAlignx(*this);
        svc->setVar("cmdLine", "-task megablast -num_threads 1 -best_hit_score_edge 0.1 -best_hit_overhang 0.1 -num_alignments 1 -num_descriptions 1 -outfmt 0");
        svc->setVar("blastProgram", "blastn");
        svc->setVar("resultFileTemplate", "req-");
        svc->setVar("produceRandomReadsForNT", "1");
        svc->setVar("maxMissQueryPercent", "10");
        svc->setVar("evalueFilter", "1e-6");
        svc->setVar("seedSize", "%" DEC, wordSize);
        svc->setVar("filterNs", "%" DEC, filterNs);
        svc->setVar("keepAllMatches","1");//i want to keep best first match
        svc->setVar("keepRefNs", "1");
        svc->setVar("qrybiomode", "1");
    } else {
        svc = new QPSvcDnaHexagon(*this);
        svc->setVar("resultFileTemplate", "req-");
        svc->setVar("produceRandomReadsForNT", "1");
        svc->setVar("maxMissQueryPercent", "10");
        svc->setVar("seedSize", "14");
    }

    if( svc ) {
        //will take whole file as one slice
//        svc->setVar("slice", "%" UDEC, sIdxMax);

        idx slice = formIValue("chunk_size", 4000);

        svc->setVar("slice", "%" DEC, slice);
        svc->setVar("query", "%s", hqrylist);
        svc->setVar("subject", "%s", sublist);
        svc->setVar("alignSelector", alignSelector);

        bool storeAlignments = formBoolValue("storeAlignments", 0);
        if (storeAlignments){
            svc->setVar("keepUnalignedReads", "1");
        }
        svc->setVar("minMatchLen", "15");

        if (deepScanning){
            Sample = sIdxMax;
        }
        svc->setVar("maxNumberQuery", "%" DEC, Sample);

        reqsubmitAlgorithm = svc->launch(*user, grpId, 0, priority);
        logOut(eQPLogType_Info, "Submitted %s request %" DEC "\n", svc->getSvcName(), reqsubmitAlgorithm);
        if( reqsubmitAlgorithm ) {
            grpAssignReqID(reqsubmitAlgorithm, reqId, 0);
            logOut(eQPLogType_Info, "BLAST %" DEC " PROCESS HAS BEEN LAUNCHED Submitted %s request %" DEC "\n", reqsubmitAlgorithm, svc->getSvcName(), reqId);
        }

    }

    return reqsubmitAlgorithm;
}

idx censuScopeProc::curateTextBasedFile (sDic<idx> *taxCnt,sDic<idx> *accCnt, sHivealtax *idAlTax, const char *objId)
{
    sUsrFile obj(objId, user);
    sStr t3_buf;
    const char * taxDepth = formValue("taxDepth",&t3_buf,"family");
    const char *separator = formIValue("textBasedFileSeparator") == 1 ? "\t" : "," ;
    idx columnNumber = formIValue ("textBasedColumn", 0) - 1;
    // Extract taxonomy information based on the file
    sStr pathFile;
    obj.getFile(pathFile);

    sTxtTbl tbl;
    tbl.setFile(pathFile.ptr());
    tbl.parseOptions().colsep = separator;
    tbl.parse(); //(flags, " ", "\r\n", "\"", 0, 0, sIdxMax, sIdxMax, 1, 1, offset);
    const char *unTax = "-1" __;
    idx *unCnt = taxCnt->set(unTax);
    *unCnt = 0;
    if (accCnt){
        idx *gCnt = accCnt->set(unTax);
        *gCnt = 0;
        sHivealtax::stats *st = idAlTax->setStats(1,unTax);

        st->family_taxid_pos = idAlTax->setTaxidContainer("-1", 2);
        st->leaf_taxid_pos = idAlTax->setTaxidContainer("-1", 2);
    }
    progress100Start = 21;
    progress100End = 70;

    idAlTax->setProgressReport(reqProgressStatic, this);

    idx taxCount = idAlTax->CurateResult(taxCnt, 0, 0, accCnt, taxDepth, &tbl, columnNumber);

    progress100End = 100;

    idx idlen;
    for(idx i = 0; i < taxCnt->dim(); ++i) {
//            cnt = *taxCnt->ptr(i);
        const char * key = (const char *) (taxCnt->id(i, &idlen));
        idAlTax->addStats(0, key, idlen, 0);
    }
    for(idx i = 0; i < accCnt->dim(); ++i) {
//            cnt = *accCnt->ptr(i);
        const char * key = (const char *) (accCnt->id(i, &idlen));
        idAlTax->addStats(1, key, idlen, 0);
    }

    return taxCount;

}

idx censuScopeProc::CurateResult(sDic<idx> *taxCnt,sDic<idx> *accCnt, const char * sublist, idx isBlast, sHivealtax *idAlTax, const char * alignObjId)
{
    sStr t3_buf;
    const char * taxDepth = formValue("taxDepth",&t3_buf,"species");
    const char * svcName = isBlast ? "dna-alignx" : "dna-hexagon";
    sStr dstTax;
    sStr pathAl, primPathAl;
    sStr subject;
    bool unalignedReads = true;

    if (!useAlignments){
        grpDataPaths(reqId, "req-alignment.hiveal", &primPathAl, svcName, 0);
    }
    else {
        sUsrFile aligner(alignObjId, user);
        aligner.getFilePathname00(primPathAl, "alignment.hiveal" _ "alignment.vioal" __);
        aligner.propGet00("subject", &subject, ";");
        sublist = subject.ptr(0);
        samplesize = -1;
    }

    // Check if the files exist either in the reqData directory or our own objID directory
    fixDataPaths(&pathAl, primPathAl.ptr());

    sHiveseq Sub(user, sublist);

    idx chunk = 0;
    //we have some files which is result from running blast against a random samples
    //ok, we want to parse them and accumulate the result
    const char *unTax = "-1" __;

    idx *unCnt = taxCnt->set(unTax);
    *unCnt = 0;
    if (accCnt){
        idx *gCnt = accCnt->set(unTax);
        *gCnt = 0;
        sHivealtax::stats *st = idAlTax->setStats(1,unTax);

        st->family_taxid_pos = idAlTax->setTaxidContainer("-1",2);
        st->leaf_taxid_pos = idAlTax->setTaxidContainer("-1",2);
    }
    for(const char *iter = pathAl.ptr(); iter; iter = sString::next00(iter), ++chunk) {
        reqProgress(chunk, 20 + chunk, 100);
        sHiveal hiveal(user);
        hiveal.parse(iter);

        idx taxCount = idAlTax->CurateResult(taxCnt, &Sub, &hiveal, accCnt, taxDepth);
        if (samplesize == -1){
            samplesize = taxCount; // Or unaligned reads from hiveal
            sBioal::Stat * stat = hiveal.getStat(-1, 0);
            if (stat){
                samplesize += stat->foundRpt;
            }
            else {
                samplesize = 0;
            }

        }
        if (unalignedReads){
            // Add the number of unaligned reads to TaxCnt
            unCnt = taxCnt->get(unTax);
            (*unCnt) = (*unCnt) + samplesize - taxCount;
            // Add to the statistics
            idAlTax->addStats(0,unTax, 0, samplesize - taxCount);

            if (accCnt){
                idx *gCnt = accCnt->get(unTax);
                (*gCnt) = (*gCnt) + samplesize - taxCount;
                // Add to the statistics
                idAlTax->addStats(1,unTax, 0, samplesize - taxCount);
            }
        }

        // Reset the statistics to get another chunk
//        idx cnt;

        idx keylen;
        for(idx i = 0; i < taxCnt->dim(); ++i) {
//            cnt = *taxCnt->ptr(i);
            const char * key = (const char *) (taxCnt->id(i, &keylen));
            idAlTax->addStats(0, key, keylen, 0);
        }
        for(idx i = 0; i < accCnt->dim(); ++i) {
//            cnt = *accCnt->ptr(i);
            const char * key = (const char *) (accCnt->id(i, &keylen));
            idAlTax->addStats(1, key, keylen, 0);
        }
    }
    return taxCnt->dim();
}

static bool getNTid (sHiveId & out_id, const sUsr & user){

    sUsrObjRes subIDList;
    user.objs2("genome", subIDList, 0, "taxonomy", "^NT$", "taxonomy");

    if( !subIDList.dim() ) {
        out_id = sHiveId::zero;
        return false;
    }
    out_id = *subIDList.firstId();
    return out_id;
}

idx censuScopeProc::OnExecute(idx req)
{
    sStr t_buf;
    t_buf.cut(0);
    reqProgress(0, 1, 100);

    // the algorithm is there BLAST or HEXAGON
    const char * alignSelector = formValue("alignSelector", &t_buf, "svc-align-blast"); // reading from mysql blast or hexagon
    idx CensuslimitIterations = formIValue("CensuslimitIterations", 5);

    samplesize = formIValue("Sample", 1000);
    real cutOffvalue = formRValue("cutOffvalue", 0.05);

    idx CensusIteration = formIValue("CensusIterations", 1);
    bool deepScanning = formBoolValue("deepScanning", 0) ? true : false;
    if (CensusIteration <= 0 || deepScanning){
        CensusIteration = 1;
    }
    idx autolauncher = formIValue("automanual", 1);
    idx isBlast = 1;

    sStr sublist, qrylist;
    sublist.cut(0);
    qrylist.cut(0);
    const char * hqrylist=0;
    sVec<idx> reqList;
    reqList.cut(0);
    const char * svcName = isBlast ? "dna-alignx" : "dna-hexagon";

    const char * alignmentObjId = formValue("alignmentInput", 0);
    const char * otherObjId = formValue ("textBasedInput", 0);
    useAlignments = false;  // Use alignment input instead of running dna-alignx
    bool analyzeAlignments = false;
    bool useShannonEntropy = formBoolValue ("selfStopping", 0);
    bool useTextBasedFile = false;
    idx currentIterationNumber = 0;
    if (alignmentObjId || otherObjId){
        currentIterationNumber = 1;
        CensusIteration = 1;
        useAlignments = true;
        analyzeAlignments = true;
        reqList.vadd(1,reqId);
        if (otherObjId && !alignmentObjId){
            useTextBasedFile = true;
        }
    }
    else {
        formValue("subject",&sublist);

        if( autolauncher == 1 ) {
            // dna-parser launched dna-screening and we need to use NT as subject
            hqrylist = formValue("query", &qrylist);

            sHiveId objId(hqrylist);
            if( objId ){
                sStr lockBuf;
                DnaScreening dnascreening(*this, objId);
                const char *lockStringkey = dnascreening.getLockString(lockBuf, objId);
                idx reqLockedby;
                reqLock(lockStringkey, &reqLockedby);
                if (req != reqLockedby){
                    // Report waiting for another request and exit as DONE
                    logOut(eQPLogType_Info, "Waiting for %s request: %" DEC " \n", dnascreening.getSvcName(), reqLockedby);
                    reqProgress(0, 100, 100);
                    reqSetStatus(req, eQPReqStatus_Done);
                    return 0;
                }
            }
            else {
                // error
                reqSetInfo(req, eQPInfoLevel_Error, "Query not valid: %s", hqrylist);
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 1;
            }

            sHiveId ntID;
            if( !getNTid(ntID, *user) ) {
                reqSetInfo(req, eQPInfoLevel_Error, "NT is missing");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            sublist.printf(0, "%s", ntID.print());
        } else {
            hqrylist = formValues00("query", &qrylist, ";"); // is your query files
        }

        sHiveseq qry(user, hqrylist, sBioseq::eBioModeLong);

        if( !hqrylist || (qry.dim() == 0)) {
            reqSetInfo(req, eQPInfoLevel_Error, "Query not found");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 1;
        }

        sVec<idx> statList;

        if (qry.dim() < samplesize){
            CensusIteration = 1; // run only 1 aligner
            CensuslimitIterations = 1;  // run only 1 iteration
            samplesize = qry.dim();
        }

        //
        // GET STATUSES OF RUNNING ALIGNERS
        //

        grp2Req(reqId, &reqList, svcName);
        if( reqList.dim() == 1 && reqList[0] == reqId )
            reqList.cut(0);    //! we erase the vector
        // pop the first one from vector
        //status of a request
        if( reqList.dim() ) {
            //there are more than one request
            grpGetStatus(reqId, &statList, svcName);
        }
        currentIterationNumber = reqList.dim() / CensusIteration;
        // if more than on query has been listed
        idx cntDone = 0, cntNotDone = 0, cntErr = 0;
        for(idx i = 0; i < statList.dim(); ++i) {
            if( statList[i] <= eQPReqStatus_Suspended )
                ++cntNotDone;
            else if( statList[i] == eQPReqStatus_Done )
                ++cntDone;
            else if( statList[i] > eQPReqStatus_Done )
                ++cntErr;
        }
        if( cntErr != 0 ) {
            // stop progress, report errors , quick exit
            reqSetInfo(req, eQPInfoLevel_Error, "Alignment produced errors");
            reqSetStatus(&reqList, eQPReqStatus_ProgError);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        } else if( cntNotDone > 0 ) {
            // WE ALREADY LAUNCH ALIGNMENT AND WE ARE WAITING FOR THE FINISH
            logOut(eQPLogType_Info, "Waiting for %s %" DEC " requests \n", svcName, cntNotDone);
            //sleepSeconds(10);
            reqProgress(0, 10, 100);
            reqReSubmit(req, 60);
            return 0;
        }
        else if ( statList.dim() && cntDone == statList.dim()){
            analyzeAlignments = true;
        }
    }
    sStr errmsg;
    idx needMoreSubmissions = 0;

    if( analyzeAlignments) {
        // ALIGNMENT IS OVER AND WE WANT TO PARSE THE RESULT INTO A FILE
        reqProgress(0, 20, 100);
        //
        // parse the alignment and produce tax dictionary
        //
        sDic<idx> taxCnt;
        taxCnt.flagOn(sMex::fSetZero);
        sDic<idx> accCnt;
        accCnt.flagOn(sMex::fSetZero);// how to zero a vector

        sHiveId objID;                // = 0;
        if( autolauncher ) {
            objID.parse(hqrylist);
        } else if( objs.dim() ) {
            objID = objs[0].Id();
        }
        //// REPLACING TAX WITH IONTAX

        sHiveId taxDatabaseID;
        if( !sviolin::SpecialObj::findTaxDb(taxDatabaseID, *user) ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Taxonomy Database is missing");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        sUsrFile of(taxDatabaseID, user);
        sStr ionPath; of.getFilePathname(ionPath,"ncbiTaxonomy.ion");
        sFilePath ionP(ionPath.ptr(), "%%dir/%s", "ncbiTaxonomy");

        sTaxIon TaxIon(ionP);
        sHivealtax * idAlTax = new sHivealtax(&TaxIon, CensusIteration, currentIterationNumber);

        if (useTextBasedFile){
            curateTextBasedFile (&taxCnt, &accCnt, idAlTax, otherObjId);
        }
        else {
            CurateResult (&taxCnt, &accCnt, sublist, isBlast, idAlTax, alignmentObjId);
        }

        // analyze the taxid lists and write to file ... meanwhile return how many new ones have been found
        reqProgress(0, 70, 100);
        real newSpeciesFound = 0;
        if( taxCnt.dim() == 0 /* nothing detected*/ && !autolauncher/* it is not from parser*/ && reqList.dim() < CensuslimitIterations/* we can launch more*/ ) {
            // WE DID NOT FIND ANY THING SO WE SET THE EMPTY FILE AND WE EXIT
            needMoreSubmissions = 1;
        } else {
            bool ok = false;
            sUsrFile sf(objID, user);
            if( sf.Id() ) {
                sStr pathR, shannPathname, gipathR;
                sf.makeFilePathname(pathR, "%s_screenResult.csv", svcName);
                sf.makeFilePathname(shannPathname, "%s_screenShannon.csv", svcName);
                sf.makeFilePathname(gipathR, "%s_acclist.csv", svcName);
                if( pathR && shannPathname && gipathR ) {
                    sFil fp(pathR), shaFile(shannPathname), fp_gi(gipathR);
                    if( fp.ok() && shaFile.ok() && fp_gi.ok() ) {
                        newSpeciesFound = idAlTax->analyzeResults(&fp, &taxCnt, &shaFile);
                        idAlTax->reportResults(&fp, &taxCnt, &fp_gi, &accCnt);
                        ok = true;
                    }
                }
            }
            if( !ok ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Failed to save screen result for %s", objID.print());
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
        }

        if( autolauncher == 1 || currentIterationNumber >= CensuslimitIterations || useAlignments) {
            needMoreSubmissions = 0;
        }
        else if( (useShannonEntropy == false  && currentIterationNumber < CensuslimitIterations)|| newSpeciesFound > cutOffvalue ) {
            // we will try again if result is not satisfying
            needMoreSubmissions = 1;
        }
        reqProgress(0, 90, 100);
    }

    if( reqList.dim() == 0 || needMoreSubmissions  ) {
        // IF QUERY IS EMPTY AND OUR grpid IS THE NEXT ONE LUNCH
        idx reqsubmitAlgorithm = 0;

        if (useShannonEntropy == false){
            CensusIteration = CensuslimitIterations * CensusIteration;
        }
        // Set the same priority as dna-screening
        Request r;
        requestGet(req, &r);

        for(idx iblast = 0; iblast < CensusIteration; ++iblast) {
            reqsubmitAlgorithm = launchAlignment(isBlast, hqrylist, sublist, alignSelector, samplesize, r.priority);
            // LAUNCH ALIGNMENT

            if( !reqsubmitAlgorithm ) {
                //IF WE WERE UNABLE TO LAUNCH ALIGNMENT
                reqSetInfo(req, eQPInfoLevel_Error, "Cannot launch ALIGNING process");
                reqSetStatus(req, eQPReqStatus_ProgError);
                break;
            }
        }
        if( reqsubmitAlgorithm ) {
            reqProgress(0, 30, 100);
            reqReSubmit(req, 40);
        }

    } else {
        // We will store the alignments
        bool storeAlignments = formBoolValue("storeAlignments", 0);
        if (storeAlignments && !useTextBasedFile){
            moveAlignments(hqrylist, svcName);
        }
        reqSetStatus(req, eQPReqStatus_Done);
        reqProgress(0, 100, 100);
    }

    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    censuScopeProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-screening", argv[0]));
    return (int) backend.run(argc, argv);
}
