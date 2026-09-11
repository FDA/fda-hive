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
#include "ncbi-meta.hpp"
#include <ulib/ufile.hpp>
#include <slib/utils/tbl.hpp>
#include <slib/std/file.hpp>
#include <ulib/ufolder.hpp>
#include <violin/violin.hpp>



#define NM(_v_prefix) N.printf(0,_v_prefix "-%s",name.ptr())

CFLOW_START(ARGOS,"argos")
{

    initHandler("argos-cflow",this,objs);

    sHIVENCBI hN;
    sStrT N,name,fldName;formValue("name",&name);



    const char * fld=formValue("wflow-folder",&fldName,name.ptr());
    if( !cntObjset("folder/") ) {
        setFolder("folder/",  fld, "Inbox");
        moveObject(0);
    }


    setEpoch("Data loading");
    varset.inp("conflictResolution","0");
    varset.inp("keepAllMatches","4");
    varset.inp("v_slice", "200000");
    varset.inp("min_match_len", formValue("min_match_len"));
    varset.inp("max_miss_pct", formValue("max_miss_pct"));

    sStr dst,tmpBioSample,tmpAssm, cflowName;
    
    const char * biosample = formValue("wflow-biosampleAcc",&tmpBioSample);
    const char * assm = formValue("wflow-asm",&tmpAssm);
    sStr splitAssm, tmpSrr, dstSrr, refLength;
    const char * srr = formValue("wflow-srr", &tmpSrr);

    if(assm && tmpAssm.length()) {
        sString::searchAndReplaceSymbols(&splitAssm,assm,0,".",0,0,true,true,true,true,0);
        const char * unversionedAssm = splitAssm.ptr(0);
        biosample = hN.assm2biosample(unversionedAssm, &dst, 0, &refLength);
        if (!srr && !cntObjset("o_reads")) {
            srr = hN.assembly2SRR(unversionedAssm, &dstSrr);
        }
        if (biosample) {
            pForm->inp("wflow-biosampleAcc",biosample);
            varset.inp("wflow-biosampleAcc", biosample);
            objs[0].propSet("name", biosample);
        }
    }

    sStr biosampleBuf;
    biosampleBuf.cut(0);

    const char * srrAcc=varset.value("srrAcc");

    if(!assm && !biosample && srr){

        sStr srrIDs, tmp; sString::searchAndReplaceSymbols(&srrIDs,srr,0,";",0,0,true,true,true,true,0);
        for (const char * srrTemp=srrIDs.ptr(0); srrTemp && *srrTemp; srrTemp=sString::next00(srrTemp)) {
            biosample = hN.srr2biosampleAcc(srrTemp, &biosampleBuf);
            
            if(!biosample) {
                biosample = hN.srr2biosampleAccXML(srrTemp, &biosampleBuf);
            }
        }
        if(biosample) {
            pForm->inp("wflow-biosampleAcc",biosample);
            varset.inp("wflow-biosampleAcc", biosample);
            objs[0].propSet("name", biosample);
        }
    }
    
    const char *refAcc = varset.value("refAcc");
    sStr refBuff, modifiedRef, refseqAssmAcc;
    idx refFlag = formIValue("references");

    if (!cntObjset("o_references") && !refAcc) {
        switch (refFlag) {
            case 1:
                refAcc = formValue("wflow-ref",  &refBuff);
                break;

            case 2: {
                bool isRefseq = false;
                refAcc = hN.getReferences(assm, &refBuff, &isRefseq);

                if (isRefseq) {
                    hN.getRefSeqAssemblyAcc(assm, &refseqAssmAcc);
                    if (refseqAssmAcc.length() && strncmp(refseqAssmAcc.ptr(), "GCA_", 4) == 0) {
                        logOut(eQPLogType_Warning,
                            "RefSeq references pulled, no RefSeq assembly accession (GCF_XXXXXX) available"
                            "Falling back to GenBank accession: %s",
                            refseqAssmAcc.ptr());
                        
                        reqSetInfo(req, eQPInfoLevel_Warning,
                               "RefSeq references pulled but no GCF_ accession available. Falling back to GenBank accession: %s",
                               refseqAssmAcc.ptr());
                    }
                }

                break;
            }

            case 3:
                setObjset("o_references", formValue("wflow-genome"));
                break;
            }
    }
    if(refAcc && !refBuff.length()) {
        refBuff.addString(refAcc);
    }
    
    if(refFlag == 1 && refAcc) {
        hN.getRefLen(refAcc, &refLength);
    }

    if (!cntObjset("o_references") && refAcc) {



        sString::searchAndReplaceSymbols(&modifiedRef, refAcc, 0, ",", ";", 0, true, true, false, false);
        varset.inp("refAcc", modifiedRef);

        if (refseqAssmAcc.length()) {
            varset.inp("RefSeqAcc", refseqAssmAcc);
            pForm->inp("RefSeqAcc", refseqAssmAcc);
        }

        sStr refIDs; sString::searchAndReplaceSymbols(&refIDs,modifiedRef.ptr(),0,";",0,0,true,true,true,true,0);
        sStr refObjID, refSrr, splitRef;

        for (const char * refs = refIDs.ptr(0); refs && *refs; refs=sString::next00(refs)) {
            splitRef.cut(0);
            sString::searchAndReplaceSymbols(&splitRef, refs,0,".",0,0,true,true,true,true,0);
            const char * unversionedRef = splitRef.ptr(0);
            if(!biosample || !*biosample) {
                biosample = hN.nucseq2BiosampleAcc(unversionedRef, &biosampleBuf);
                if(biosample && *biosample) {
                    pForm->inp("wflow-biosampleAcc",biosample);
                    varset.inp("wflow-biosampleAcc", biosample);
                    objs[0].propSet("name", biosample);
                }
            }

            if(!cntObjset("o_reads") && !srr) {
                srr = hN.nucseq2SRR(unversionedRef, &dstSrr);
                if(srr && *srr) {
                    pForm->inp("wflow-srr",srr);
                    varset.inp("wflow-srr", srr);
                }
            }

            if((biosample && *biosample) || (srr && *srr)) break;
        }

        for (const char * refs = refIDs.ptr(0); refs && *refs; refs=sString::next00(refs)) {
            refObjID.cut(0);
            objQry(refObjID, "alloftype('genome',{'orig_name':'%s.gb'})[0]", refs);
            sHiveseq Refs(user, refObjID.ptr());

            if(!Refs.dim()) {
                logOut(eQPLogType_Warning, "Genome object %s is empty, re-download from NCBI", refObjID.ptr());
                continue;
            }else if(refObjID && Refs.dim()) {
                setObjset("o_references", refObjID);
            }
        }
        
    }

    if (assm && !cntObjset("o_references") && !refseqAssmAcc.length()) {
        hN.getRefSeqAssemblyAcc(assm, &refseqAssmAcc);
    }



    sStr srrBuff, sraBuff;
    
    if (!cntObjset("o_reads")&& biosample && !srr) {
        srr = hN.biosample2SRA(biosample, &srrBuff, 0);
        if(srr && *srr){
            pForm->inp("wflow-srr",srr);
            varset.inp("wflow-srr", srr);
        } else {
            const char * sra = hN.assm2sra(assm, &sraBuff, 0);
            if (sra){
                srr = hN.srs2srr(sra, &srrBuff);
            }
            if (srr){
                pForm->inp("wflow-srr",srr);
                varset.inp("wflow-srr", srr);
            } else {
                setObjset("wflow-srr", "0");
                logOut(eQPLogType_Warning, "No SRR found for BioSample: %s\n", biosample);
                reqSetInfo(req, eQPInfoLevel_Warning, "No SRR found for BioSample: %s\n", biosample);
            }
        }    
    }

    if(!cntObjset("o_reads") && !srrAcc ) {
        switch(formIValue("ngs")) {
            case 1:srrAcc=formValue("wflow-srr");break;
            case 2:setObjset("o_reads",formValue("wflow-nucreads"));break;
        }

        if(!srrAcc) {
            srrAcc = srr;
        }
        
    } 

    sStr modifiedSrr;
    if(srrAcc){
        
        sString::searchAndReplaceSymbols(&modifiedSrr, srrAcc, 0, ",", ";", 0, true, true, false, false);

        varset.inp("srrAcc", modifiedSrr.ptr());
        varset.inp("wflow-srr", modifiedSrr.ptr());
        pForm->inp("wflow-srr", modifiedSrr.ptr());
        srrAcc = modifiedSrr;
    } else {
        setObjset("wflow-srr", "0");
    }


    idx resRef=-1;
    if(!cntObjset("o_references") && refBuff.length()) {
        resRef=ensureDownload("p_ref_downloader",NM("References"),"genbank", refBuff);
    } 
    if (resRef && !formValue("hive_asm_ref")) {
        if(!cntObjset("o_references"))  {
            logOut(eQPLogType_Error, "Cannot download references.");
            reqSetInfo(req, eQPInfoLevel_Error, "Cannot download references.");
            return 0;
        }
    }        
    
    idx resSrr=-1;
    sStr read_objIDs, readObj1, currReadObj;
    sStr srrIDs; sString::searchAndReplaceSymbols(&srrIDs,srrAcc,0,";",0,0,true,true,true,true,0);
    sStr id1, id2, currentId;

    if(!cntObjset("o_reads") && srrIDs){
        for (const char * readAcc=srrIDs.ptr(0); readAcc && *readAcc; readAcc=sString::next00(readAcc)) {
            objQry(read_objIDs, "[alloftype('nuc-read', {'source':'sra://%s'})[0]].csv(['base_tag'])", readAcc);
            sTbl ttbl;ttbl.parse(read_objIDs.ptr(0), read_objIDs.length());
            if (ttbl.dim() >1) {
                sStr cellBuf;
                ttbl.get(&cellBuf,1,1);
                if (cellBuf.length()) {
                    char * archPrefix = strstr(cellBuf.ptr(), "archiver/");
                    if (archPrefix) {
                        char * archPrefixStart = archPrefix;
                        idx iPos = archPrefix - cellBuf.ptr();
                        while (iPos < cellBuf.length() && archPrefix[0] != '"' && archPrefix[0] != ',' && archPrefix[0] != '\n' && archPrefix[0] != '\r' && archPrefix[0] != '\0') {
                            archPrefix++;iPos++;
                        }
                        cellBuf.cutAddString(0,archPrefixStart, archPrefix-archPrefixStart);
                    }
                }
                
                sStr numReadsStr;
                objQry(numReadsStr, "a=alloftype('nuc-read', {'base_tag':'%s'}).filter({this.source=='sra://%s'}); return a as int;", cellBuf.ptr(), readAcc);
                idx numReads = strtoidx(numReadsStr, 0, 10);
                for (idx iR=0; iR<numReads; iR++) {
                    read_objIDs.cut(0);
                    objQry(read_objIDs, "alloftype('nuc-read', {'source':'sra://%s','base_tag':'%s'})[%" DEC "]", readAcc, cellBuf.ptr(), iR);
                    if (read_objIDs.length()) {
                        sHiveseq readHS(user, read_objIDs);
                        if(!readHS.dim()) {
                            logOut(eQPLogType_Warning, "Read object %s is empty, re-download from NCBI", read_objIDs.ptr());
                            continue;
                        }
                        setObjset("o_reads", read_objIDs);
                    }
                
                }
            }
             
        }
    }

    if(cntObjset("o_references") && refLength) {    
        idx ncbiCnt = strtoidx(refLength.ptr(), 0, 10);
        sStr objIds; sString::searchAndReplaceSymbols(&objIds, listObjset("o_references"),0,",",0,0,true,true,true,true,0);
        idx bpCnt = 0;
        for (const char * ref=objIds.ptr(0); ref; ref=sString::next00(ref)) {
            sHiveId refId(ref);
            sUsrObj refObj(*user, refId);
            bpCnt += refObj.propGetI("bases-count");
        }
        if(ncbiCnt > 0 && ncbiCnt != bpCnt) {
            logOut(eQPLogType_Warning, "reference download is incomplete");
        }
    }

    bool readsAvailable = cntObjset("o_reads");
    bool readsExpected = srrAcc && *srrAcc && strcmp(srrAcc, "0") != 0;
    bool readsUnavailable = !readsAvailable && !readsExpected;

    if(!readsAvailable && readsExpected) {
        resSrr=ensureDownload("p_ngs_downloader",NM("reads"),"sra",srrAcc);
        if(!resSrr) {
            return 0;
        }

        readsAvailable = cntObjset("o_reads");
        if(!readsAvailable) {
            readsUnavailable = true;
            logOut(eQPLogType_Warning, "Cannot download reads. Skipping alignment and variant calling");
            reqSetInfo(req, eQPInfoLevel_Warning, "Cannot download reads. Skipping alignment and variant calling");
        }
    }

    if (resRef==-1 && !cntObjset("o_references") && formValue("hive_asm_ref")) {
        sStr genome; formValue("hive_asm_ref", &genome);

        tmpBioSample.cut(0);formValue("wflow-biosampleAcc",&tmpBioSample);
        varset.inp("hive-asm-name", tmpBioSample.length() ? tmpBioSample.ptr() : name.ptr());
        varset.inp("hive-asm-wflow-folder", fld);

        if( !cntObjset("o_hive_asm_ref") ) {
            setObjset("o_hive_asm_ref",genome);
        }
        varset.inp("clone_params_clone_cov", formValue("clone_params_clone_cov",0,"10"));
        varset.inp("clone_params_clone_length",formValue("clone_params_clone_length",0,"6"));
        varset.inp("clone_params_clone_support",formValue("clone_params_clone_support",0,"4"));

        if (!ensureProcess("p_hive-asm")) 
            return 0;

        sVec <idx> * hive_asm_procs = objsets.get("p_hive-asm");
        if (!hive_asm_procs || !hive_asm_procs->dim()) {
            logOut(eQPLogType_Error, "Hive assembly process not found.");
            reqSetInfo(req, eQPInfoLevel_Error, "Hive assembly process not found.");
            return 0;
        }
            
        genome.printf(0,"hive-assembly/%" DEC "",*(hive_asm_procs->ptr(0)));
        searchObjects("o_references", "genome", "base_tag",genome.ptr(0));
        resRef=1;
        varset.inp("conflictResolution","2");
        varset.inp("keepAllMatches","3");
    }
    
    if (!cntObjset("o_references")) {
        return 0;
    }


    setEpoch("Analysis");
    tmpBioSample.cut(0);
    formValue("wflow-biosampleAcc",&tmpBioSample);
    if (!tmpBioSample.length()) {
        tmpBioSample.printf(0,"%s", name.ptr());
        varset.inp("wflow-biosampleAcc", tmpBioSample);
    } else {
        objs[0].propSet("name", tmpBioSample.ptr());
    }

    sStr hexName, heptName, alqcName, assmValue, taxGroup;
    formValue("tax_group", &taxGroup);
    hexName.printf("%s %s Hexagon Aligner", tmpBioSample.ptr(), taxGroup.ptr());
    heptName.printf("%s Heptagon Variant Caller", tmpBioSample.ptr());
    alqcName.printf("%s ARGOS QC", tmpBioSample.ptr());

    varset.inp("hexagonName", hexName.ptr());
    varset.inp("heptagonName", heptName.ptr());
    varset.inp("alqcName", alqcName.ptr());

    if(formValue("wflow-asm")){
        assmValue.printf("%s", formValue("wflow-asm"));
    } else {
        assmValue.printf("Template Guided Assembly");
    }
    varset.inp("assmVal", assmValue.ptr());

    readsAvailable = cntObjset("o_reads");
    if (readsAvailable) {
            const char *hexagonProcess = 0;

        if (strcmp(taxGroup.ptr(), "Bacteria") == 0) {
            hexagonProcess = "p_alignment-bacteria";
        } else if (strcmp(taxGroup.ptr(), "Protozoa") == 0) {
            hexagonProcess = "p_alignment-protozoa";
        } else if (strcmp(taxGroup.ptr(), "Eukaryote") == 0) {
            hexagonProcess = "p_alignment-eukaryote";
        } else if (strcmp(taxGroup.ptr(), "Fungi") == 0) {
            hexagonProcess = "p_alignment-fungi";
        } else {
            hexagonProcess = "p_alignment-viruses";
        }

        if (!ensureProcess(hexagonProcess)) {
            return 0;
        }

        sVec<idx> *hexagonObjects = objsets.get(hexagonProcess);
        if (!hexagonObjects || !hexagonObjects->dim()) {
            logOut(eQPLogType_Error,
                   "Selected Hexagon process has no object ID: %s",
                   hexagonProcess);
            reqSetInfo(req, eQPInfoLevel_Error,
                       "Selected Hexagon process was not created.");
            return 0;
        }

        sStr hexagonParentID;
        hexagonParentID.printf("%" DEC, *(hexagonObjects->ptr(0)));
        varset.inp("hexagon-parent-id", hexagonParentID.ptr());

        if (!ensureProcess("p_hepta-profiler")) {
            return 0;
        }
    } else if (readsUnavailable) {
        setObjset("p_hepta-profiler", "0");
    } else {
        return 0;
    }

    varset.inp("argos-wflow-id", objs[0].IdStr());
    if (!ensureProcess("p_dna-alqc")) {
        return 0;
    }



    setEpoch("Final");

    sStr finalBioSampleBuf;
    formValue("wflow-biosampleAcc", &finalBioSampleBuf);
    const char * effectiveBioSample = finalBioSampleBuf.length() ? finalBioSampleBuf.ptr() : biosample;

    sStr prefix;
    prefix.printf(0, "%s", effectiveBioSample ? effectiveBioSample : "");
    prefix.add(0, 128);
    copyFiles("p_dna-alqc","*.json", 0, 0, false, prefix.ptr());
    

    sTaxIon *taxIon = 0;
    sStr ionP, error_log;
    if( sviolin::SpecialObj::findTaxDbIonPath(ionP, *user, 0, 0, &error_log) ) {
        taxIon = new sTaxIon(ionP.ptr());
        
    }

    sStr tempAssm;
    if(cntObjset("p_hive-asm")){
        sVec <idx> * hive_asm_procs = objsets.get("p_hive-asm");
        if(hive_asm_procs->dim()) {
            idx hiveAsmID = *hive_asm_procs->ptr(0);
            sStr hiveID;
            hiveID.printf("%lld", hiveAsmID);
            sHiveId tmpID(hiveID.ptr());
            sUsrObj obj(*user, tmpID);
            tempAssm.printf("%s", obj.propGet("accession"));
        }
    }

    NCBI_Meta bioMeta(effectiveBioSample, objs[0].IdStr(), user);
    if(taxIon) {
        bioMeta.taxTree = taxIon;
    }
    bioMeta.fetchBioSampleMeta(hN);

    if(assm) { 
        NCBI_Meta assmMeta(effectiveBioSample, assm, objs[0].IdStr(), user, false);
        if(taxIon) {
            assmMeta.taxTree = taxIon;
        }
        assmMeta.fetchAssemblyMeta(hN);

        if(assmMeta.getAssemblyJson()) {

            sStr fileName;
            fileName.printf("%s-NCBI_assemblyMeta.json", effectiveBioSample ? effectiveBioSample : "0");
            sStr filePath;
            ProcFile(fileName, true &filePath);
            assmMeta.assembly2HiveStore(filePath.ptr());
            assmMeta.parseAssemblyMeta();
            sJson * outputObj = assmMeta.getHiveObjJson();
            idx wflow_fld_id = procId("folder/");
            sHiveId dst_id(wflow_fld_id, 0);
            if(dst_id){
                sUsrFolder dst(*user, dst_id);
                assmMeta.populateHiveObj(outputObj, dst);
            }

        } else {
            logOut(eQPLogType_Warning, "Saving response json for Datasets NCBI API call failed");
        }

    } else if (refAcc) {
        NCBI_Meta genbankMeta(effectiveBioSample, refBuff.ptr(), objs[0].IdStr(), user, true);
        if(taxIon) {
            genbankMeta.taxTree = taxIon;
        }
        genbankMeta.fetchGenbankMeta(hN);
        genbankMeta.fetchBioSampleMeta(hN);

        if(genbankMeta.getGenbankJson()) {
            bioMeta.getBiosampleJson();
            sStr fileName, logName;
            fileName.printf("%s-NCBI_genbankMeta.json", effectiveBioSample ? effectiveBioSample : "0");
            logName.printf("%s-Metadata_log.txt", effectiveBioSample ? effectiveBioSample : "0");
            sStr gbFile;
            ProcFile(fileName, true, &gbFile);
            genbankMeta.genbank2HiveStore(gbFile.ptr());
            genbankMeta.parseGenbankMeta();
            sFil metaLog(ProcFile(logName, false), sMex::fMapRemoveFile);

            genbankMeta.log2HiveStore(metaLog);
            sJson * outputObj = genbankMeta.getHiveObjJson();

            idx wflow_fld_id = procId("folder/");
            sHiveId dst_id(wflow_fld_id, 0);
            if(dst_id){
                sUsrFolder dst(*user, dst_id);
                genbankMeta.populateHiveObj(outputObj, dst);
            }

        } else {
            logOut(eQPLogType_Warning, "Saving response json for Datasets NCBI API call failed");
        }

    } else if(bioMeta.getBiosampleJson()) {
        sStr fileName;
        fileName.printf("%s-NCBI_biosampleMeta.json", effectiveBioSample ? effectiveBioSample : "0");
        sStr bioFile;
        ProcFile(fileName, true, &bioFile);
        bioMeta.biosample2HiveStore(bioFile.ptr());
        sStr assemblyName; sString::searchAndReplaceSymbols(&assemblyName, tempAssm, 0, "_", 0,0,true,true,true,true,0);
        bioMeta.tempAssmMeta(tempAssm, assemblyName.ptr(0));


        sJson * outputObj = bioMeta.getHiveObjJson();
        idx wflow_fld_id = procId("folder/");
        sHiveId dst_id(wflow_fld_id, 0);
        if(dst_id){
            sUsrFolder dst(*user, dst_id);
            bioMeta.populateHiveObj(outputObj, dst);
        }
    } else {
        logOut(eQPLogType_Warning, "Datasets NCBI API call failed, no return json");
    }

    if(cntObjset("o_reads") && srrIDs){
        sJson readMeta;
        sUsrObj srrMeta(*user, "argos_srr_metadata");
        sHiveId srrMetaId = srrMeta.Id();
        user->objJson(srrMetaId.objId(), &readMeta);

        JSNode root = JSNode(&readMeta);
        JSNode arrObj = root.linkarr("readMeta");
        sStr instrumentModel, libStrategy, libSource, libSelection, libConstProtocol;
        for (const char * readAcc=srrIDs.ptr(0); readAcc && *readAcc; readAcc=sString::next00(readAcc)) {
            instrumentModel.cut(0);
            libStrategy.cut(0);
            libSource.cut(0);
            libSelection.cut(0);
            libConstProtocol.cut(0);

            hN.getLibaryInfo(readAcc, &instrumentModel, &libStrategy, &libSource, &libSelection, &libConstProtocol);

            JSNode srrObj = arrObj.linkobj("#");
            JSNode record = srrObj.linkobj("read_record");

            record.link("srr_id", readAcc);
            record.link("instrument_model", instrumentModel.ptr(0));
            record.link("library_strategy", libStrategy.ptr(0));
            record.link("library_source", libSource.ptr(0));
            record.link("library_selection", libSelection.ptr(0));
        }

        root.link("argos_objID", objs[0].IdStr());
        if(tmpBioSample.length()) {
            root.link("biosample", tmpBioSample.ptr());
        } else {
            root.link("biosample", "0");
        }
        readMeta.serialize();
        sStr jsonStr;
        idx jsLen = 0;
        const char *jsonCont = readMeta.ret(&jsLen);
        jsonStr.addString(jsonCont, jsLen);
        user->propSetJson(0, jsonStr.ptr());
        idx wflow_fld_id = procId("folder/");
        sHiveId dst_id(wflow_fld_id, 0);
        if(dst_id){
            sUsrFolder fld(*user, dst_id);
            fld.attach(srrMeta);
        }
    }

    reqProgress(100,100, 100);
    reqSetStatus(req, eQPReqStatus_Done );

    return 0;
}
CFLOW_STOP()
