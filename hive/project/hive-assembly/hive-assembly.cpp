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
#include <violin/hiveseq.hpp>
#include "hive-assembly-utils.hpp"




using namespace slib;
using namespace ha_utils;

CFLOW_START(HIVEASSEMBLY,"hive-assembly")
{
    sBioseq::initModule(sBioseq::eACGT);
    
    initHandler("hive-assembly",this,objs);
    sStrT name;
    formValue("name",&name);

    sStr hoxID;
    cfgStr(&hoxID, 0, "HIVE.hoxID", "-");
    hoxID.printf("_%s", reportO->IdStr());
    reportO->propSet("accession", hoxID.ptr());

    if (!name.length()) {
        name.printf("hive-assembly_%s",objs[0].IdStr());
        pForm->inp("name",name.ptr());
        varset.inp("name",name.ptr());
    }

    const char * fld=formValue("wflow-folder",0,name.ptr());

    if( !cntObjset("folder/") ) {
        setFolder("folder/",  fld, "Inbox");
        moveObject(0);
    }
    reportO->propSet("status","3");

    setEpoch("Data loading");
    varset.inp("v_slice","200000");
    sStr buf, buf2;
    sStr reads, genome;
    formValue("reads", &reads);
    formValue("reference_genome", &genome);

    if( !cntObjset("o_sample_reads") ) {
        setObjset("o_sample_reads", reads);
    }
    if( !cntObjset("o_reference_genome") ) {
        setObjset("o_reference_genome",genome);
    }
    sStr objIds; sString::searchAndReplaceSymbols(&objIds,listObjset("o_reference_genome"),0,",",0,0,true,true,true,true,0);

    if (!listObjset("p_alignment-sample")) {
        sStr hexagonName, maff_objID, maffName;
        idx alCounter=0, alTotal=0;
        idx maffCounter=0, maffTotal=0;

        for (const char * ref=objIds.ptr(0); ref; ref=sString::next00(ref)) {
            sHiveId refId(ref);
            sUsrObj hqry(*user, refId);
            if ( !hqry.Id()){
                logOut(eQPLogType_Info, "Object %s not found or access denied", hqry.IdStr());
                reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", hqry.IdStr());
                continue;
            }
            hexagonName.printf(0,"p_alignment-sample.%s",ref);
            buf.printf(0,"%s_Reads sample vs %s_%s",name.ptr(),hqry.propGet("name"),ref);
            varset.inp("hexName",buf.ptr());
            varset.inp("_o_reference_genome",ref);
            ++alTotal;
            if(!ensureProcess(hexagonName.ptr()))
                continue;
            ++alCounter;

            maff_objID.cut(0);
            objQry(maff_objID,"alloftype('svc-align-mafft',{'subject':'%s'})[0]",ref);
            maffName.printf(0,"p_alignment-maff.%s",ref);
            buf.printf(0,"%s_%s",hqry.propGet("name"),ref);
            varset.inp("maffName",buf.ptr());
            ++maffTotal;
            if (maff_objID.length() && !cntObjset((maffName.ptr(0)))) {                
                setObjset(maffName.ptr(0),maff_objID);
            }

            if (!ensureProcess(maffName.ptr()))
                continue;

            ++maffCounter;
        }

        if (alCounter < alTotal) {
            return 0;
        }

        if (maffCounter < maffTotal) {
            return 0;
        }
        const char * hexagons=listObjRegex(buf.printf(0,"p_alignment-sample[.]*"));
        setObjset("p_alignment-sample", hexagons );

        const char * maff=listObjRegex(buf.printf(0,"p_alignment-maff[.]*"));
        setObjset("p_alignment-maff", maff );        
    }


    idx clone_cov = formIValue("clone_cov",1);
    idx clone_len = formIValue("clone_length",1);
    idx clone_support = formIValue("clone_support",1);

    sStr clonalName, procName,cloneDeflineFastaTmplt;
    idx procCounter=0, procTotal=0;
    for (const char * ref=objIds.ptr(0); ref; ref=sString::next00(ref)) {
        sHiveId refId(ref);
        sUsrObj hqry(*user, refId);
        if ( !hqry.Id()){
            logOut(eQPLogType_Info, "Object %s not found or access denied", hqry.IdStr());
            reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", hqry.IdStr());
            continue;
        }
        const char * refName = hqry.propGet("name");
        idx refLen = sLen(refName);
        const char * p = strstr(refName,".");
        if (p) {
            refLen = (p - refName);
        }
        cloneDeflineFastaTmplt.printf(0,"$_(v) reference:%.*s clone_cov:%" DEC " source:hive-assembly", (int)refLen, refName, clone_cov);
        clonalName.printf(0,"p_clonal-analysis.%s",ref);
        buf.printf(0,"%s_%s_%s",name.ptr(),hqry.propGet("name"),ref);
        varset.inp("clonalName",buf.ptr());
        
        buf.printf(0,"p_alignment-sample.%s",ref);
        idx alId = procId(buf.ptr());
        buf.printf(0,"%" DEC "",alId);
        varset.inp("_p_alignment-sample",buf.ptr());

        buf.printf(0,"p_alignment-maff.%s",ref);
        idx maffId = procId(buf.ptr());
        buf.printf(0,"%" DEC "",maffId);
        varset.inp("_p_alignment-maff",buf.ptr());
        
        ++procTotal;
        if(!ensureProcess(clonalName.ptr()))
            continue;

        idx cloneCount = getCloneCount(user, procId(clonalName.ptr()));

        if( cloneCount == sNotIdx || cloneCount == 0 ) {
            logOut(
                cloneCount == 0 ? eQPLogType_Info : eQPLogType_Warning,
                cloneCount == 0
                    ? "No clones found for reference %s; skipping remaining steps"
                    : "Could not read clones for reference %s; skipping remaining steps",
                ref
            );
            ++procCounter;
            continue;
        }
        buf.printf(0,"p_archiver_clones.%s",ref);
        buf2.printf(0,"%s_%s_clones.fasta",name.ptr(),hqry.propGet("name"));
        idx res_ClonalContigs = archiveCGI(buf.ptr(),clonalName.ptr(), buf2.ptr(),"cmd=popExtended&fasta_tmplt=%.*s&noGapsFrame=0&contig_print=seq&minFrequency=0.5&minDiversity=1&simThrs=50&maskLowDiversity=1&covThrs=%" DEC "&minCloneLen=%" DEC "&mergeHidden=1&minCloneCov=%" DEC "",(int)cloneDeflineFastaTmplt.length(),cloneDeflineFastaTmplt.ptr(),clone_cov,clone_len,clone_support);
        if(res_ClonalContigs) {
            buf2.printf(0,"o_clones.%s",ref);
            res_ClonalContigs=searchArchived(buf2.ptr(),buf.ptr(),"nuc-read");
        }
        if(!res_ClonalContigs)
            continue;
        
        procName.printf(0,"p_alignment-sample-clonal.%s",ref);
        buf.printf(0,"%s_%s_%s-vs_clones",name.ptr(),hqry.propGet("name"),ref);
        varset.inp("hexClonalName",buf.ptr());

        buf.printf(0,"o_clones.%s",ref);
        idx clId = procId(buf.ptr());
        buf.printf(0,"%" DEC "",clId);
        varset.inp("_o_clones",buf.ptr());

        if(!ensureProcess(procName.ptr()))
            continue;

        procName.printf(0,"p_hepta-profiler.%s",ref);
        buf.printf(0,"%s_%s_%s-vs_clones",name.ptr(),hqry.propGet("name"),ref);
        varset.inp("heptClonalName",buf.ptr());

        buf.printf(0,"p_alignment-sample-clonal.%s",ref);
        idx clAlId = procId(buf.ptr());
        buf.printf(0,"%" DEC "",clAlId);
        varset.inp("_p_alignment-sample-clonal",buf.ptr());

        if(!ensureProcess(procName.ptr()))
            continue;

        buf.printf(0,"p_archiver_consensus.%s",ref);
        buf2.printf(0,"%s_%.*s_clones_consensus.fasta",name.ptr(),(int)refLen,refName);
        idx res_ClonalConsensus = archiveCGI(buf.ptr(),procName.ptr(), buf2.ptr(),"cmd=profConsensus&idSub=0&gaps=skip&arch=1&backend=1&check=1&down=0&raw=0");
        if(res_ClonalConsensus) {
            buf2.printf(0,"o_clones_consensus.%s",ref);
            res_ClonalConsensus=searchArchived(buf2.ptr(),buf.ptr(),"nuc-read");
            if (res_ClonalConsensus) {
                buf.printf(0,"%" DEC "",procId(buf2.ptr()));
                buf2.printf(0,"%s/%s_%.*s_clones_consensus-dedup.fasta",procFolder.ptr(),name.ptr(),(int)refLen,refName);

                if( !sFile::exists(buf2.ptr())){ 
                    sviolin::sHiveseq sf(user, buf.ptr(),sviolin::sHiveseq::eBioModeShort);
                    sFil myFile(buf2.ptr(), sMex::fMapRemoveFile);
                    sf.printFastX(&myFile, false,0,sf.dim(),0,true);
                    myFile.destroy();
                }
                
                buf.printf(0,"file://%s",buf2.ptr());        
                buf2.printf(0,"%s_%.*s_clones_consensus-dedup.fasta",name.ptr(),(int)refLen,refName);
                procName.printf(0, "p_uploader_dedup.%s", ref);
                res_ClonalConsensus=ensureDownload(procName.ptr(),buf2.ptr(),"0",buf.ptr());
                if (res_ClonalConsensus) {
                    buf.printf(0,"o_clones_dedup.%s",ref);
                    res_ClonalConsensus=searchArchived(buf.ptr(),procName.ptr(),"nuc-read");
                }
            }    
            
        }
        if(!res_ClonalConsensus)
            continue;

        procName.printf(0,"p_alignment-clonal-ref-id.%s",ref);
        buf.printf(0,"%s_%s_%s-vs_clones_consensus_dedup",name.ptr(),hqry.propGet("name"),ref);
        varset.inp("hexClonalRefIDName",buf.ptr());

        buf.printf(0,"o_clones_dedup.%s",ref);
        idx cloneId = procId(buf.ptr());
        buf.printf(0,"%" DEC "",cloneId);
        varset.inp("_o_clones_dedup",buf.ptr());
        varset.inp("_o_reference_genome_dedup",ref);

        if(!ensureProcess(procName.ptr()))
            continue;

        buf.printf(0,"p_archiver_consensus_id_table.%s",ref);
        buf2.printf(0,"%s_%.*s_clones_consensus_id_hitTable.csv",name.ptr(),(int)refLen,refName);
        idx res_ClonalConsensusID = archiveCGI(buf.ptr(),procName.ptr(), buf2.ptr(),"cmd=alMatch&info=1&found=1&qty=-1");

        if (res_ClonalConsensusID) {
            buf2.printf(0,"o_clones_consensus_id_table.%s",ref);
            res_ClonalConsensusID=searchArchived(buf2.ptr(),buf.ptr(),"csv-table");
            if (res_ClonalConsensusID) {
                sStr hitTablePath;
                getFilePath(buf2.ptr(),"_.csv",0,&hitTablePath);
                buf2.printf(0,"%s_%.*s_clones_consensus_dedup_id_final.fasta",name.ptr(),(int)refLen,refName);
                CloneInfo curClones;
                curClones.objId = cloneId; curClones.procId = reportObjId;
                curClones.procName = name.ptr();
                curClones.procFolder = procFolder.ptr();
                curClones.hoxID = hoxID.ptr();
                fixIdLineClones(hitTablePath.ptr(), curClones, user, buf2.ptr(),buf);
                buf2.printf(0,"file://%s",buf.ptr());
                buf.printf(0,"%s_%.*s_clones_consensus_dedup_id_final.fasta",name.ptr(),(int)refLen,refName);
                procName.printf(0, "p_uploader_final.%s", ref);
                res_ClonalConsensusID=ensureDownload(procName.ptr(),buf.ptr(),"0",buf2.ptr());
                if (res_ClonalConsensusID) {
                    buf.printf(0,"o_clones_final.%s",ref);
                    res_ClonalConsensusID=searchArchived(buf.ptr(),procName.ptr(),"nuc-read");
                    if (!res_ClonalConsensusID) {
                        res_ClonalConsensusID=searchArchived(buf.ptr(),procName.ptr(),"genome");
                    }
                }

            }

        }


        if(!res_ClonalConsensusID)
            continue;

        ++procCounter;
    }

    if (procCounter < procTotal) {
        return 0;
    }

    setEpoch("Final");

    sStr consensusObjIds; 
    sString::searchAndReplaceSymbols(&consensusObjIds,listObjRegex(buf.printf(0,"o_clones_final[.]*")),0,",",0,0,true,true,true,true,0);

    for (const char * r=consensusObjIds.ptr(0); r; r=sString::next00(r)) {
        const char * cDone=varset.value(r);

        if (!cDone) {
            sHiveId consensusId(r);
            sUsrObj consObj(*user, consensusId);
            buf.printf(0,"hive-assembly/%s",reportO->IdStr());
            const char * p="1.5", *v=buf.ptr(0);
            consObj.propSet("base_tag", &p,&v,1,true  );
            consObj.cast("genome");
            
            varset.inp(r,"done");
        }

        
    }

    reqProgress(100,100, 100);
    reqSetStatus(req, eQPReqStatus_Done );

    reportO->propSet("status","5");

    return 0;

    

}
CFLOW_STOP()

