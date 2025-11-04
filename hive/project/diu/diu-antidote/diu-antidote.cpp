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
#include "diu-antidote-utils.hpp"




using namespace slib;
using namespace diu_utils;

#define useDB(_v_db, _v_tax) \
    idx resAl_##_v_db= ensureProcess("p_alignment-" #_v_db);  \
    idx resArcTaxCount_##_v_db=0,  resArcAlFastq_##_v_db=0, resArcAlMatch_##_v_db=0; \
    if(resAl_##_v_db) { \
        resArcTaxCount_##_v_db = archiveCGI("p_archiver_" #_v_db "_table","p_alignment-" #_v_db , #_v_db "-TaxCount.csv",_v_tax ? "cmd=taxCount&ranks=superkingdom&prefix=-" : "cmd=alCount&prefix=-") ; \
        if(resArcTaxCount_##_v_db ) searchArchived("o_" #_v_db "_table","p_archiver_" #_v_db "_table","csv-table");  \
        resArcAlFastq_##_v_db = archiveCGI("p_archiver_" #_v_db "_fastq","p_alignment-" #_v_db , #_v_db "-Aligned.fq","cmd=alFastq&found=1&qty=-1&collapseRpts=1") ; \
        if(resArcAlFastq_##_v_db)searchArchived("o_" #_v_db "_fastq", "p_archiver_" #_v_db "_fastq", "nuc-read"); \
        resArcAlMatch_##_v_db = archiveCGI("p_archiver_" #_v_db "_alMatch","p_alignment-" #_v_db , #_v_db "-AlMatch.csv","cmd=alMatch&extendTaxonomy=2&start=0&cnt=0&info=1&found=1&qty=-1") ; \
        if(resArcAlMatch_##_v_db)searchArchived("o_matches-" #_v_db , "p_archiver_" #_v_db "_alMatch", "csv-table"); \
    }
#define searchDB(_v_db, _v_title) \
    if( err( searchObjects("o_DIU_" #_v_db, "genome", "taxonomy","DIU_" #_v_db), \
        _v_title " references are missing.")) \
        return 0;

#define eventDB(_v_db,_v_tax) \
    if(resAl_##_v_db){ \
        foreachFilePath(fle, "o_" #_v_db "_table", "_.csv") { \
            createEventJson (fle,resinf,b,cntEvents, highestSeverity,_v_tax,&acc2DB); \
        } \
    }


CFLOW_START(DIUANTIDOTE,"diu-antidote")
{
    sBioseq::initModule(sBioseq::eACGT);
    
    if(!initHandler("diu-antidote",this,objs,"prop:sampleObj")) 
        return 0;
    reportO->propSet("all_status","processing");
    if(err(reportObjId,"Missing Sample Object"))
        return 0;

    if( !cntObjset("folder/") ) {
        setFolder("folder/",  formValue("name",0,0), "Inbox");
        moveObject(0);
        moveObject(0, reportObjId);

    }
    
    idx support_2_tier=1;

    reportO->propSet("all_status","running");

     searchDB(viruses,"Viruses");
    searchDB(bacteria,"Bacteria");
    searchDB(toxins,"Toxins");
    searchDB(protozoa,"Protozoa");
    searchDB(fungi,"Fungi");
    searchDB(bioengineered,"Bioengineered");


    if( err(searchObjects("o_tier1_to_tier2_table", "csv-table", "tag", "DIU_Antidote_Tier1ToTier2Table"), 
        "Tier1 to Tier2 table is missing."))
        return 0;

    sVar acc2DB;
    foreachFilePath(flt, "o_tier1_to_tier2_table", "_.csv") {
        mapAcc2Tier2DB(flt, &acc2DB);
    }
    mapGenomesToAccDB( user,&acc2DB);
    




    setEpoch("Data loading");
    const char * storeId=reportO->propGet("storeID");
    idx isPaired=1; for(const char * p=storeId;*p;++p){if(*p==';')++isPaired;}
    if(isPaired==1) 
        varset.inp("v_slice","10000");
    else 
        varset.inp("v_slice","200000");

    
    if(!ensureDownload("p_downloader",formValue("sampleID"),"0",storeId)) {
        return 0;
    }
    if( err( searchArchived("o_sample_reads","p_downloader","nuc-read"),
        "Sequence files cannot be retrieved."))
        return 0;




    setEpoch("Quality Control");
    bool qcPassed=true;
    if(qcPassed)setStageStatus("f_QC",sQPrideBase::eQPReqStatus_Done , 1, 100 , 0 );
    else setStageStatus("f_QC",sQPrideBase::eQPReqStatus_ProgError , 1, 100 , "Low Quality Sequences");
    if(!qcPassed)
        return 0;



    setEpoch("Analysis");


     useDB(viruses,true);
    useDB(bacteria,true);
    useDB(toxins,true);
    useDB(protozoa,true);
    useDB(fungi,true);
    useDB(bioengineered,false);
    
    if (!resArcAlMatch_viruses || !resArcAlMatch_bioengineered || !resArcAlMatch_toxins || !resArcAlMatch_bacteria || !resArcAlMatch_protozoa || !resArcAlMatch_fungi)
      return 0;    
    

     
    sStr objIds; sString::searchAndReplaceSymbols(&objIds,listObjset("o_sample_reads"),0,",",";",0,true,true,true,true,0);
    sviolin::sHiveseq sf(user, objIds,sviolin::sHiveseq::eBioModeLong);

    const char * llnlDone=varset.value("llnlDone");
    if(!llnlDone) { 

        sStr llnl_outputPath;qpObjs->addFilePathname(llnl_outputPath, true, "%s.tsv",formValue("name",0,0));
        sFile::remove(llnl_outputPath);sFil tsv(llnl_outputPath);
        tsv.printf("Read label\tRead length (nt)\tResult code\tAssignment NCBI TaxId\tAssignment confidence\tResult details\n");
        
    
        sVec < sTbl > tblSet;
        sVec < sDic< LLNL_BULL > > dicReadsSet;
        for(const char * m="o_matches-viruses" _ "o_matches-bioengineered" _ "o_matches-bacteria" _ "o_matches-toxins" _ "o_matches-protozoa" _ "o_matches-fungi" __; m; m=sString::next00(m) ) {
            sTbl * tbl=tblSet.add();
            sDic< LLNL_BULL > * dicReads=dicReadsSet.add();dicReads->init(0,sMex::fSetZero);
            foreachFilePath(fln, m, "_.csv") { 
                if(!tbl->parseFile(fln)) continue;
                llnl_tempDic(*tbl,*dicReads,&acc2DB,m+10);
                
            }
        }
        llnl_tempOut(tsv,dicReadsSet, sf, isPaired);
        varset.inp("llnlDone","finita-la-comedia");
    }



    idx resAl_tier2=0;
    if( resAl_viruses && support_2_tier ) {
        sStr fqlist;const char * FQs=varset.value("2ndStageFQ");
        if( !FQs )  { 
            sDic<Tier2DB> dbNameToFileDic;
            sStr objIds; sString::searchAndReplaceSymbols(&objIds,listObjset("o_sample_reads"),0,",",";",0,true,true,true,true,0);
            sviolin::sHiveseq sf(user, objIds);

            foreachFilePath(fls, "o_matches-viruses", "_.csv") {
                fillSequenceFiles(fls, procFolder, acc2DB, dbNameToFileDic, sf);
            }
            FQs=closeDbFiles(fqlist,dbNameToFileDic);
            varset.inp("2ndStageFQ",FQs);
        }

        if(!listObjset("p_alignment-tier2")){
            sStr dbList,role,hexName,tier2DBList,tier2FQList; sFilePath FQPath;
            sFilePath dbFlnm;
            sString::searchAndReplaceSymbols(&dbList,FQs,0,",",0,0,true,true,true,true,0);
            idx hexCounter=0,cntHexTotal=0;
            for( const char * db=dbList.ptr(0); db; db=sString::next00(db)) {
                dbFlnm.makeNameAt(0,db,"%%pathx_prio.fasta");
                if(err(searchObjects("_o_tier2_references","genome", "name", dbFlnm.ptr()), "Could not find tier2 DB %s", db))
                    continue;

                ++cntHexTotal;
                FQPath.makeNameAt(0,role.printf(0,"file://%s%s",procFolder.ptr(),db),"%%pathx.fq");
                role.printf(0, "p_uploader.%s", db);
                if(!ensureDownload(role,db,"0",FQPath.ptr(0)))
                    continue;

                if(err(searchArchived("_o_tier2_reads", role.ptr(),"nuc-read"), "Could not upload tier2 fastq"))
                    return 0;
                
                hexName.printf(0, "p_alignment-tier2.%s", db);
                varset.inp("tier2Name", db);
                if(!ensureProcess(hexName.ptr()))
                    continue;
                ++hexCounter;        
                tier2DBList.printf("%s%s",tier2DBList.length() ? "," : "", listObjset("_o_tier2_references"));
                tier2FQList.printf("%s%s",tier2FQList.length() ? "," : "", listObjset("_o_tier2_reads"));
            }
            if(hexCounter < cntHexTotal ) {
                return 0;
            }

            if(!listObjset("o_tier2_DBs"))
                setObjset("o_tier2_DBs", tier2DBList );
            if(!listObjset("o_tier2_FQs"))
                setObjset("o_tier2_FQs",  tier2FQList );
            
            const char * tier2Hexagons=listObjRegex(dbList.printf(0,"p_alignment-tier2[.]*"));
            setObjset("p_alignment-tier2", tier2Hexagons );
                    
        }
        resAl_tier2=archiveCGI("p_archiver_tier2_table","p_alignment-tier2", "tier2-TaxCount.csv","cmd=taxCount&ranks=superkingdom&prefix=-");
        if(resAl_tier2) {
            resAl_tier2=searchArchived("o_tier2_table","p_archiver_tier2_table","csv-table");
        }
        if(!resAl_tier2)
            return 0;
    }

    if (!ensureProcess("p_hex-alignment-all"))
        return 0;

    idx resDIProfiler = ensureProcess("p_di_profiler");
    if (!resDIProfiler)
        return 0;


    sJson * oj=user->objJson(reportObjId);
    
    if(oj) {
        sStr b;
        JSNode root(oj,"$root");
        root.del("result_info");
        JSNode & resinf=root.linkarr("result_info");
        idx cntEvents=0, highestSeverity=0;
        if(support_2_tier) {
            eventDB(tier2,true); 
        } else {
            eventDB(viruses,true); 
        }
        eventDB(bacteria,true); 
        eventDB(toxins,true); 
        eventDB(protozoa,true); 
        eventDB(fungi,true); 
        eventDB(bioengineered,false);

        if (resDIProfiler) {
            sStr diProfFilePath;
            getFilePath("p_di_profiler","di-profile.csv",0,&diProfFilePath);
            createDIEventJson (diProfFilePath.ptr(),resinf, b, cntEvents, &acc2DB);
        }

        switch(highestSeverity){
            case 0:root.link("highest_severity","unknown");break;
            case 1:root.link("highest_severity","low");break;
            case 2:root.link("highest_severity","medium");break;
            case 3:root.link("highest_severity","high");break;
        }


        if(cntEvents ) {
            if(err( user->propSetJson(oj), "Events registering failure."))
                return 0;
        }
    }

#ifdef DONOTCOMPILE    
    

    idx resAlTier2=ensureProcess("p_hex-alignment-all");
    if(resAlTier2) {
        resAlTier2=archiveCGI("p_acc_and_tax-all", "p_hex-alignment-all", "Read2Accn-all.csv", "cmd=alMatch&extendTaxonomy=2&start=0&cnt=0&info=1&found=1&qty=-1");
        if(resAlTier2) {
            resAlTier2 = searchArchived("o_acc_and_tax-all", "p_acc_and_tax-all", "csv-table");
        }
    }
    if(!resAlTier2)
        return 0;





#endif


    reqProgress(100,100, 100);
    reqSetStatus(req, eQPReqStatus_Done );
    reportO->propSet("all_status","done");

    sStr qpProcInfo;
    qpObjs->propGet("completed",&qpProcInfo);
    if (qpProcInfo.length()) {
        reportO->propSet("all_completed", qpProcInfo.ptr());
    }
    qpProcInfo.cut(0);
    qpObjs->propGet("started",&qpProcInfo);
    if (qpProcInfo.length()) {
        reportO->propSet("all_started", qpProcInfo.ptr());
    }

    return 0;

}
CFLOW_STOP()









    
    







