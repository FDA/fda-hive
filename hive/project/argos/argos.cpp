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
#include "hive-ncbi.hpp"

#include </home/skeeney/hive.code/include/ulib/ufile.hpp>



#define NM(_v_prefix) N.printf(0,_v_prefix "-%s",name.ptr())

CFLOW_START(ARGOS,"argos")
{

    initHandler("argos-cflow",this,objs);

    sHIVENCBI hN;
    sStrT N,name;formValue("name",&name);



    const char * fld=formValue("wflow-folder",0,name.ptr());
    if( !cntObjset("folder/") ) {
        setFolder("folder/",  fld, "Inbox");
        moveObject(0);
    }


    setEpoch("Data loading");
    varset.inp("conflictResolution","0");
    varset.inp("keepAllMatches","4");

    sStr dst,tmpBioSample,tmpAssm;  
    const char * biosample = formValue("wflow-biosampleAcc",&tmpBioSample);
    const char * assm = formValue("wflow-asm",&tmpAssm);

    if (!biosample && assm) {
        biosample = hN.assm2biosample(assm, &dst, 0);
        if (biosample) {
            pForm->inp("wflow-biosampleAcc",biosample);
            varset.inp("wflow-biosampleAcc", biosample);
        } else {
            setObjset("wflow-biosampleAcc", "0");
            logOut(eQPLogType_Warning, "No BioSample found for assembly: %s", assm);
            reqSetInfo(req, eQPInfoLevel_Warning, "No BioSample found for assembly: %s", assm);
        }
    }
    
    const char *refAcc = varset.value("refAcc");
sStr refBuff, modifiedRef, refseqAssmAcc;
idx refFlag = formIValue("references");
sStr referenceAcc;

if (!cntObjset("o_references") && !refAcc) {
    switch (refFlag) {
        case 1:
            refAcc = formValue("wflow-ref");
            break;

        case 2: {
            bool isRefseq = false;
            refAcc = hN.getReferences(assm, &refBuff, &isRefseq);
            if (refAcc) {
                referenceAcc.printf("%s", refAcc);
                refAcc = referenceAcc.ptr();
            }            

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

        sString::searchAndReplaceSymbols(&modifiedRef, refAcc, 0, ",", ";", 0, true, true, false, false);
        varset.inp("refAcc", modifiedRef);

        if (refseqAssmAcc.length()) {
            varset.inp("RefSeqAcc", refseqAssmAcc);
            pForm->inp("RefSeqAcc", refseqAssmAcc);
        }
    }

    if (!refseqAssmAcc.length()) {
        hN.getRefSeqAssemblyAcc(assm, &refseqAssmAcc);
    }



    sStr srrBuff, tmpSrr;
    const char * srr = formValue("wflow-srr", &tmpSrr);
    
    if (!srr && biosample) {
        srr = hN.biosample2SRA(biosample, &srrBuff, 0);
        if(srr && *srr){
            pForm->inp("wflow-srr",srr);
            varset.inp("wflow-srr", srr);
        } else {
            setObjset("wflow-srr", "0");
            logOut(eQPLogType_Warning, "No SRR found for BioSample: %s\n", biosample);
            reqSetInfo(req, eQPInfoLevel_Warning, "No SRR found for BioSample: %s\n", biosample);
        }

            
    }

    const char * srrAcc=varset.value("srrAcc");
    if(!cntObjset("o_reads") && !srrAcc ) {
        switch(formIValue("ngs")) {
            case 1:srrAcc=formValue("wflow-srr");break;
            case 2:setObjset("o_reads",formValue("wflow-nucreads"));break;
        }
        
    } 

    if(srrAcc){
        sStr modifiedSrr;
        sString::searchAndReplaceSymbols(&modifiedSrr, srrAcc, 0, "\n", ";", 0, true, true, false, false);

        varset.inp("srrAcc", modifiedSrr.ptr());
        varset.inp("wflow-srr", modifiedSrr.ptr());
        pForm->inp("wflow-srr", modifiedSrr.ptr());
    } else {
        setObjset("wflow-srr", "0");
    }


    idx resRef=-1;
    if(refAcc ) {
        resRef=ensureDownload("p_ref_downloader",NM("References"),"genbank", refAcc);
    }
    idx resSrr=1;
    if(srrAcc ) {
        resSrr=ensureDownload("p_ngs_downloader",NM("reads"),"sra",srrAcc);
    }
    if (!resSrr) {
        return 0;
    }
    if (!srrAcc) {
        logOut(eQPLogType_Warning, "No SRR specified. Skipping alignment and variant calling.");
    } else if (!resSrr) {
        return 0;
    }
    if (srrAcc && err(searchArchived("o_reads", "p_ngs_downloader", "nuc-read"), "Cannot download reads.")) {
        logOut(eQPLogType_Error, "Failed to retrieve archived reads.");
        return 0;
    }

    if (resRef==-1) {
        sStr genome;

        formValue("hive_asm_ref", &genome);
        varset.inp("hive-asm-name", fld);
        if( !cntObjset("o_hive_asm_ref") ) {
            setObjset("o_hive_asm_ref",genome);
        }
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
    if (!resRef) {
        
        return 0;
    }
    
    


    if (!cntObjset("o_references") && err(searchArchived("o_references", "p_ref_downloader", "genome"), "Cannot download references.")) {
        return 0;
    }

    


    setEpoch("Analysis");

    if (cntObjset("o_reads")) {
        if (!ensureProcess("p_hex-alignment")) {
            return 0;
        }

        if (!ensureProcess("p_hepta-profiler")) {
            return 0;
        }
    } else {
        setObjset("p_hepta-profiler", "0");
    }

    if (!ensureProcess("p_dna-alqc")) {
        return 0;
    }



    setEpoch("Final");

    sStr prefix;
    prefix.printf(0, "%s", formValue("wflow-biosampleAcc") ? formValue("wflow-biosampleAcc") : "");
    prefix.add(0, 128);
    copyFiles("p_dna-alqc","*.json", 0, 0, false, prefix.ptr());
    

    sJson jsonObj;

    {
        const char* biosampleResult = hN.getBiosampleData(formValue("wflow-asm"), &dst, &jsonObj);  
        printf("Biosample Data:\n%s\n", biosampleResult ? biosampleResult : "No data");
    
        const char *finalAssemblyAcc = refseqAssmAcc.length() ? refseqAssmAcc.ptr() : formValue("wflow-asm");
    
        sStr bf0, tt0;
        qpObjs->addFilePathname(bf0, true, tt0.printf("%s-Biosample-Meta.json", formValue("wflow-biosampleAcc")));
        sFil f(bf0, sMex::fMapRemoveFile);
    
        f.printf("Reference Assembly Accession: %s\n", finalAssemblyAcc);
    
        if (biosampleResult && *biosampleResult) {
            f.printf("%s\n", biosampleResult);
        } else {
            f.printf("No biosample metadata found.\n");
        }
    }
    


    {
        const char * bs=hN.biosample(formValue("wflow-biosampleAcc"));
        sStr bf,tt;qpObjs->addFilePathname(bf,true,tt.printf("%s-biosample.json",formValue("wflow-biosampleAcc")));
        sFil f(bf,sMex::fMapRemoveFile);f.printf("%s",bs);
    }

    sJson reportStruct, uniprotJson;    

    sStr jsonStr, plog, propLog, orgName;
    const char * ncbiJson = hN.saveBiosampleData(formValue("wflow-asm"), &jsonStr);
    sStr cleanNCBIJson;
    
    
    sUsrObj ncbiReport(*user, "argos_ncbi_Metadata");

    reportStruct.cln();
    user->objJson(strtoidx(ncbiReport.IdStr(), nullptr, 10), &reportStruct);

    hN.flattenNcbiJson(ncbiJson, &reportStruct, &cleanNCBIJson, 0);

    orgName.printf("%s", reportStruct.value("organism_name"));
    bool ncbiObjProp = user->propSetJson(&reportStruct, 0, &propLog);

    sJson datasetReport;
    datasetReport.initMem(ncbiJson, jsonStr.length());
    if(ncbiJson && jsonStr.length()) {
      datasetReport.file(ProcFile("datasetReport.json"));
      datasetReport.serialize();
    } else {
      logOut(eQPLogType_Warning, "Saving response json for Datasets NCBI API call failed");
    }


    sUsrObj uniProt(*user, "argos_uniprot");
    user->objJson(strtoidx(uniProt.IdStr(), nullptr, 10), &uniprotJson);
    sUsrObjRes obj_res;

    bool found = hN.getUniProtData(orgName, &uniprotJson);

    if(!found){
        sStr rebuf;
        udx cnt = 0;
        rebuf.printf(0, "uniprot_rpg-75");

        user->objs2("^special$", obj_res, &cnt, "title", rebuf.ptr(), "title");
        idx inum=0;

        idx max1 = 0;
        idx max2 = 0;
        sStr newVersion;
        const sHiveId *fileID = 0;

        for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it),++inum) {

            const char * idStr = obj_res.id(it)->print();
            
            sUsrObj obj(*user, *obj_res.id(it)); 
            sStr currentVersion;
            obj.propGet("version", &currentVersion);

            sStr vPart1, vPart2;
            char splitToken = '.';
            bool beforePoint = true;
            for (idx i = 0; i < currentVersion.length(); i++){
    
                if(currentVersion[i] == splitToken){
                    beforePoint = false;
                    continue;
                }

                if(beforePoint) {
                    vPart1.printf("%c", currentVersion[i]);
                } else {
                    vPart2.printf("%c", currentVersion[i]);
                }
            }
            idx versionIdx1, versionIdx2;

            versionIdx1 = strtoidx(vPart1.ptr(), nullptr, 10);
            versionIdx2 = strtoidx(vPart2.ptr(), nullptr, 10);

            if (versionIdx1 > max1){
                max1 = versionIdx1;
                max2 = versionIdx2;
                fileID = obj_res.id(it);
            } 
            else if (versionIdx1 == max1 && versionIdx2 > max2) {
                max1 = versionIdx1;
                max2 = versionIdx2;
                fileID = obj_res.id(it);
            }

            




            idx temp =0;
        }

        sStr path, fileBuf;
        sUsrFile ufile(*fileID, user);
        
        ufile.makeFilePathname(path, "uniprot.txt");


        


    }

    

    user->propSetJson(&uniprotJson, 0, &plog);


    reqProgress(100,100, 100);
    reqSetStatus(req, eQPReqStatus_Done );

    return 0;
}
CFLOW_STOP()





