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
#include <ctype.h>
#include <slib/utils/tbl.hpp>
#include "../argos/hive-ncbi.hpp"


CFLOW_START(CFLOWLAUNCHER,"cflow-launcher")
{
    idx TOTAL_TASKS = 0;
    idx LAUNCHED_TASKS = 0;

    printf("cflow-launcher start\n");


    initHandler("cflow-launcher", this, objs);

    sQPrideBase::Service argosSvc;
    if (!serviceGet(&argosSvc, "argos", 0)) {
        printf("failed to retrieve QPSvc service argos\n");
        return 0;
    }

    const idx SLOT_LIMIT = argosSvc.maxJobs;
    printf("argos maxJobs=%" DEC "\n", SLOT_LIMIT);

    sStrT name;
    formValue("name", &name);
    if (!name.length()) {
        name.printf("cflow-launcher_%s", objs[0].IdStr());
        pForm->inp("name", name.ptr());
        varset.inp("name", name.ptr());
    }

    const char * fld = formValue("wflow-folder", 0, name.ptr());
    if (!cntObjset("folder/")) {
        setFolder("folder/", fld, "Inbox");
        moveObject(0);
    }

    reportO->propSet("status", "3");
    sHIVENCBI ncbi;

    setEpoch("Scheduler");
    printf("Entered Scheduler epoch\n");

    const idx PROGRESS_PREFREE = 90;

    sStr cflowID;
    objQry(cflowID, "alloftype('workflow',{'name':'argos-cflow'})[0]");
    printf("argos-cflow workflow id=%s\n", cflowID.ptr());
    
    sStr taskName;
    sStr filename;
    sUsrObj csvObj(*user, sHiveId(formValue("accessions_csv")));
    if(!csvObj.Id()) {
        printf("csv object not found\n");
        
        return 0;
    }
    if(!csvObj.getFilePathname(filename, "_.csv")) {
        printf("csv object has no _.csv pathname\n");
        
        return 0;
    }
    printf("csv pathname=%s\n", filename.ptr());
    
    idx doneTasks = 0, runningTasks = 0;
    sFil argosCSV(filename, sMex::fReadonly);


    if(!argosCSV.ok()) {
        printf("csv file is not readable\n");
    }
    sTbl table; table.parse(argosCSV, argosCSV.length(), sTbl::fPreserveQuotes | sTbl::fAllowEmptyCells, ",");
    sStr refVal, refTypeVal, codeTblVal, refType, refBuff, taxGroup;
    sFil accErrorLog(ProcFile("Accession_Error_Log.txt", false), sMex::fMapRemoveFile);
    
    TOTAL_TASKS = table.rows();
    LAUNCHED_TASKS = table.rows();
    printf("csv rows=%" DEC "\n", TOTAL_TASKS);
    
    for (idx iTask=0; iTask <= TOTAL_TASKS; ++iTask) {
        taskName.printf(0, "p_computation.%" DEC, iTask);
        printf("task=%s begin\n", taskName.ptr());
        

        if (hasObjects(taskName.ptr(), 0)) {
            printf("task=%s already present in procmap\n", taskName.ptr());
            
            error = 0;
            log.cut(0);
            idx ensureResult = ensureProcess(taskName.ptr());
            if (!ensureResult) {
                printf("task=%s ensureProcess failed error=%" DEC " log=%s\n",
                       taskName.ptr(), error, log.ptr());
                idx prg, prg100;
                statusProcess(taskName.ptr(), &prg, &prg100);
                printf("task=%s status=%" DEC " progress100=%" DEC "\n", taskName.ptr(), prg, prg100);
                
                if (prg100 < PROGRESS_PREFREE) {
                    ++runningTasks;
                    printf("task=%s counts as running runningTasks=%" DEC "\n", taskName.ptr(), runningTasks);
                    
                }
            } else {
                ++doneTasks;
                printf("task=%s already done doneTasks=%" DEC "\n", taskName.ptr(), doneTasks);
                
            }
            continue;
        }

        if (runningTasks >= SLOT_LIMIT) {
            printf("slot limit reached runningTasks=%" DEC " limit=%" DEC "\n", runningTasks, SLOT_LIMIT);
            
            return 0;
        }

        printf("task=%s fetching csv row\n", taskName.ptr());
        
            
            refType.cut0cut(0);
            refBuff.cut0cut(0);

            table.get0(&refTypeVal, iTask, 3);
            table.get0(&refVal, iTask, 2);
            table.get0(&codeTblVal, iTask, 7);
            table.get0(&taxGroup, iTask, 6);
            printf("task=%s extracted codingTable value=%s\n", taskName.ptr(), codeTblVal.ptr());
            printf("task=%s refType=%s accession=%s codingTable=%s\n", taskName.ptr(), refTypeVal.ptr(), refVal.ptr(), codeTblVal.ptr());
            

            
            if(strcmp(refTypeVal.ptr(),"wflow-asm") == 0){
                refType.addString("2");
                if(refVal){
                    printf("task=%s biosample lookup for %s\n", taskName.ptr(), refVal.ptr());
                    
                    ncbi.saveBiosampleData(refVal.ptr(), &refBuff);
                }

            } else if(strcmp(refTypeVal.ptr(),"wflow-ref") == 0){
                refType.addString("1");
                if(refVal){
                    printf("task=%s genbank lookup for %s\n", taskName.ptr(), refVal.ptr());
                    
                    ncbi.fetchGenbankData(refVal.ptr(), &refBuff);
                }
            } else {
                printf("task=%s skipped unsupported refType=%s\n", taskName.ptr(), refTypeVal.ptr());
                
                continue;
            }

            printf("task=%s lookup returned length=%" DEC "\n", taskName.ptr(), refBuff.length());
            
            if(refBuff.length() <= 2){
                LAUNCHED_TASKS -=1;
                accErrorLog.printf("%s,%s,%s\n",refVal.ptr(),refTypeVal.ptr(),codeTblVal.ptr());
                printf("task=%s no NCBI result, recorded in error log\n", taskName.ptr());
                
                continue;
            }
            
            varset.empty();
            varset.inp(refTypeVal, refVal);
            varset.inp("references", refType);
            varset.inp("wflow-codingTable", codeTblVal);
            varset.inp("cflowID", cflowID);
            varset.inp("computationName", taskName);
            varset.inp("computationFolder", "cflow-launcher");
            
            if(strcmp(taxGroup,"bacteria")==0) {
                varset.inp("tax_group", "Bacteria");
                varset.inp("max_miss_pct", "5");
                varset.inp("min_match_len", "38");
            } else if(strcmp(taxGroup,"fungi")==0 || strcmp(taxGroup,"yeast")==0) {
                varset.inp("tax_group", "Fungi");
                varset.inp("max_miss_pct", "3");
                varset.inp("min_match_len", "30");
            } else if(strcmp(taxGroup,"Eukaryota")==0) {
                varset.inp("tax_group", "Eukaryote");
                varset.inp("max_miss_pct", "3");
                varset.inp("min_match_len", "30");
            } else if(strcmp(taxGroup,"Protozoa")==0) {
                varset.inp("tax_group", "Protozoa");
                varset.inp("max_miss_pct", "3");
                varset.inp("min_match_len", "30");
            } else {
                varset.inp("tax_group", "Virus");
                varset.inp("max_miss_pct", "15");
                varset.inp("min_match_len", "75");
            } 
            printf("task=%s launching with accession=%s\n", taskName.ptr(), refVal.ptr());
            

            error = 0;
            log.cut(0);
            idx ensureResult = ensureProcess(taskName.ptr());
            if (!ensureResult) {
                printf("task=%s ensureProcess failed error=%" DEC " log=%s\n",
                       taskName.ptr(), error, log.ptr());
              
                idx prg, prg100;
                statusProcess(taskName.ptr(), &prg, &prg100);
                printf("task=%s launch pending status=%" DEC " progress100=%" DEC "\n", taskName.ptr(), prg, prg100);
                
                if (prg100 < PROGRESS_PREFREE) {
                    ++runningTasks;
                    printf("task=%s counts as running runningTasks=%" DEC "\n", taskName.ptr(), runningTasks);
                    
                }
              
            } else {
                ++doneTasks;
                printf("task=%s already complete doneTasks=%" DEC "\n", taskName.ptr(), doneTasks);
                
            }
    }

    if (doneTasks == LAUNCHED_TASKS) {
        printf("all launched tasks complete doneTasks=%" DEC " launched=%" DEC "\n", doneTasks, LAUNCHED_TASKS);
        
        reqProgress(100, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        reportO->propSet("status", "5");
    }
   

    return 0;
}
CFLOW_STOP()
