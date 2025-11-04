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
#include <ion/sJson.hpp>
#include <qlib/QPrideCGI.hpp>
using namespace slib;

enum enumDIUCommands
{
    eDIUAntidote_SampleList,
    eDIUAntidote_Launch,
    eDIUAntidote_List,
    eDIUAntidote_MessageList,
    eDIUAntidote_MessageReceived,
    eDIUAntidote_SendSignal,
    eDIUAntidote_MarkSignal,
    eDIUAntidote_Remove
};





idx DnaCGI::CmdDIU(idx cmd)
{
    
    switch(cmd) {
        case eDIUAntidote_Launch: {
            const char * diu_sampleID=pForm->value("sampleID", 0);
            const char * dropBoxID=pForm->value("dropboxID", 0);
            const char * geoLocation = pForm->value("geoLocation","19.61369,-155.34761");
            idx isNanopore=pForm->ivalue("isNanopore",0);

            const char * techID=pForm->value("techID", "admin@gmail.com");
            const char * hoxID=pForm->value("hoxID", "ip-10-0-1-132.ec2.internal");

            if (diu_sampleID && dropBoxID) {
                sStr jt, ppath;
                ppath.printf(0,"dropbox://%s/illumina/%s_R1.fastq.gz;dropbox://%s/illumina/%s_R2.fastq.gz",dropBoxID,diu_sampleID,dropBoxID,diu_sampleID);
                if (isNanopore) {
                    ppath.printf(0,"dropbox://%s/nanopore/%s.fastq.gz", dropBoxID, diu_sampleID);
                }
                jt.printf(0, "[{\"_id\" :\"$newid\", \"_type\" : \"da_sample\", "
                        "\"sampleID\" : \"%s\", "
                        "\"storeID\" : \"%s\", "
                        "\"hive_in_a_box\" : { "
                            "\"techID\" : \"%s\", "
                            "\"hoxID\" : \"%s\" "
                        "}, " 
                        "\"sample_collection\" : {"
                            "\"geoLocation\" : \"%s\""
                        "}, "
                        "\"process_status\" : { "
                            "\"status\" : \"parked\" "
                        "}}]",diu_sampleID,ppath.ptr(),techID, hoxID,geoLocation);

                pForm->inp("_json", jt.ptr());
            }

            overCall=true;
            pForm->inp("raw","1");
            pForm->inp("cmd","propset");
            DnaCGI::Cmd("propset");
            udx objID=0;
            if(dataForm.length() && dataForm.ptr()) {
                const char * p=strstr(dataForm.ptr(),"prop.da_sample._id=");
                if(p)sscanf(p+19,"%" DEC, & objID);
            }
            if(!objID) return 1;
            sUsrObj obj(m_User,sHiveId(objID,0));
            const char * name=obj.propGet("name");
            const char * sampleID=obj.propGet("sampleID");

            pForm->empty();
            pForm->inp("svc","diu-antidote");
            pForm->inp("prop.svc-diu-antidote-cflow.name.1",name ? name : sampleID );
            pForm->inp("prop.svc-diu-antidote-cflow.submitter.20","diu-antidote");
            pForm->inp("prop.svc-diu-antidote-cflow._type","svc-diu-antidote-cflow");
            pForm->inp("prop.svc-diu-antidote-cflow.sampleID.12",sampleID);
            pForm->inpv("prop.svc-diu-antidote-cflow.sampleObj.13","%" UDEC ,objID);
            pForm->inp("prop.svc-diu-antidote-cflow.folder.18.2","Inbox");


            pForm->inp("cmd","-qpProcSubmit");
            dataForm.cut(0);
            DnaCGI::Cmd("-qpProcSubmit");
            overCall=false;

            idx processObj=0,req;
            if(dataForm.length() && dataForm.ptr()){
                sscanf(dataForm.ptr(),"%" DEC ",%" DEC,&req,&processObj);
            }
            dataForm.cut(0);
            obj.propSetI("processObj",(idx)processObj);
            obj.propSet("status","processing");
            pForm->inp("mode","json");
            pForm->inp("cmd","propget");
            pForm->inp("ids",obj.IdStr());
            DnaCGI::Cmd("propget");

            break;
        }
        case eDIUAntidote_MessageList:
        case eDIUAntidote_SampleList: {
            
            idx tim=pForm->ivalue("tim",0);
            if(!tim)tim=sTime::gmtNow()-pForm->ivalue("gap",120);
            idx maxAttempts=pForm->ivalue("max",100);
            sStrT tatr;
            

            idx info=pForm->ivalue("info");
            idx cnt=pForm->ivalue("cnt");
            idx start=pForm->ivalue("start");
            idx startSignal=pForm->ivalue("startSignal");
            idx cntSignal=pForm->ivalue("cntSignal");
            idx hideSubmitted= pForm->ivalue("hideSubmitted",(cmd==eDIUAntidote_MessageList) ? 1 : 0);
            idx hideMarked=pForm->ivalue("hideMarked",(cmd==eDIUAntidote_MessageList) ? 0 : 1 );
            idx hideLineage=pForm->ivalue("hideLineage",(cmd==eDIUAntidote_MessageList) ? 1 : 0 );
            const char* severity=pForm->value("severity");
            sStr sever;if(severity){sString::searchAndReplaceSymbols(&sever,severity,0,",",0,0,true,true,true,true,0);severity=sever;}
            const char* signal=pForm->value("signal");
            sStr signl;if(signal){sString::searchAndReplaceSymbols(&signl,signal,0,",",0,0,true,true,true,true,0);signal=signl;}
            const char* classN=pForm->value("class");
            sStr clsn;if(classN){sString::searchAndReplaceSymbols(&clsn,classN,0,",",0,0,true,true,true,true,0);classN=clsn;}
            bool doProgressReport=(cmd==eDIUAntidote_MessageList) ? false : pForm->boolvalue("progress",true);

            idx hideResultInfo = pForm->ivalue("hideResultInfo",0);
            idx hideComputed = pForm->ivalue("hideComputed",0);
            


            sStr hoxID;sQPride::cfgStr(&hoxID,pForm,"HIVE.hoxID","-");
            sStr techID;techID.printf("%s",pForm->value("email","-"));

            const char * fileTypes=pForm->value("fileTypes",".fa,.fq,.fasta,.fastq,.fa.gz,.fq.gz,.fasta.gz,.fastq.gz");
            sStr extensions;sString::searchAndReplaceSymbols(&extensions,fileTypes,0,",;",0,0,true,true,false,true,0);

            const char * boxes=pForm->value("box","diu_antidote");
            sStr boxlist;sString::searchAndReplaceSymbols(&boxlist,boxes,0,",;",0,0,true,true,false,true,0);
            const char *  dropbox= (cmd==eDIUAntidote_MessageList) ? 0: pForm->value("dropbox","diu_dropbox") ;
            const char * pars=pForm->value("prop_name");
            const char * vals=pForm->value("prop_val");
            const char * ids=pForm->value("_id");
            if(ids) { 
                pars="_id";
                vals=ids;
            }
            if(pars || vals) dropbox=0;

            sDic < sStr > dicDropbox;
            if( dropbox && *dropbox )  { 
                
                overCall=true;
                DnaCGI::Cmd("dropboxlist");
                overCall=false;
                sTbl tbl;
                tbl.parse(dataForm.ptr(0),dataForm.length());
                dataForm.cut(0);
                
                for ( idx it=1; it<tbl.rows(); ++it) {
                    idx pathLen,idLen;
                    const char * path=tbl.cell(it,2,&pathLen), * ext=0;
                    for ( ext=extensions.ptr(0); ext; ext=sString::next00(ext)) {
                        idx elen=sLen(ext);
                        if( pathLen>elen && strncmp(path+pathLen-elen,ext,elen)==0)break;
                    }
                    if(ext==0)continue;
                    const char * id = sString::searchSubstring(path, pathLen, "/" _ "\\" __, sNotIdx, (const char *)0, true, (idx*)0 );
                    if(!id) continue;
                    else ++id;
                    idLen = pathLen - (id - path);
                    if(idLen < 2) continue;

                    const char* runIdStart = id + idLen - 2, *pEnd = runIdStart, *pSeek;
                    while(runIdStart > id) {
                        if((*runIdStart == '_') && (runIdStart < pEnd)) {
                            pSeek = runIdStart + sizeof(char);
                            if((toupper(*pSeek++) == 'R') && isdigit(*pSeek))
                                break;
                        }
                        --runIdStart;
                    }
                    if(runIdStart > id) 
                        idLen = runIdStart - id;

                    sStr* pStore=dicDropbox.set(id,idLen);
                    if(pStore->length()) pStore->add(";", 1);
                    pStore->printf("dropbox:/%.*s",(int)pathLen,path);
                }
            }

            sUsrObjRes obj_res;
            
            sDic <idx > dicSample;
            
            idx cntMatch=0,cntReported=0; 
            if(cmd==eDIUAntidote_MessageList)
                dataForm.printf("[");
            dataForm.printf("{\n    \"objs\" : [\n");

            sStr propsFilter;
            if (hideComputed) {
                hideResultInfo=1;
                propsFilter.printf(0,"sampleID,storeID");
            }
            udx objsCnt =0;
            if (propsFilter.length()) {
                objsCnt = user->objs2("da_sample", obj_res,(udx*)0,pars,vals,propsFilter.ptr());
            } else {
                objsCnt = user->objs2("da_sample", obj_res,(udx*)0,pars,vals);
            }
            if( objsCnt ) {


                sStr sbuf;
                for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {

                    sbuf.printf(0,"{\"objs\":");
                    sJSONPrinter json_printer;json_printer.respectArrays=true;json_printer.init(&sbuf);
                    json_printer.startArray();json_printer.startObject();
                    obj_res.json(m_User, it, json_printer, true, false);
                    json_printer.endObject();json_printer.endArray();sbuf.printf("}");
                    sJson pip;pip.initMem(sbuf.ptr());
                    JSNode objs=pip.node("$root.objs");
                    sbuf.cut(0);
                    
                    
                    
                    idx cnt_SignalsMatch=0,cnt_SignalsOut=0;
                    idx sub_confirmed=0,sub_attempts=0,sub_time=0;
                    for( idx io=0; io<objs.dim(); ++io ) {
                        JSNode obj=objs[io];
                        bool isSubmitted=false;
                        if(hideSubmitted){
                            JSNode & sub=obj["submission"];

                            sub_confirmed=0;sub_attempts=0;sub_time=0;
                            if(sub.ok()){
                                sub_confirmed=sub["submission_confirmed"];
                                if(sub_confirmed>0)
                                    isSubmitted=true;
                                if(!isSubmitted) { 
                                    sub_attempts=sub["submission_attempts"];
                                    if(sub_attempts>=maxAttempts)
                                        isSubmitted=true;
                                    
                                    if(!isSubmitted) {
                                        sub_time=sub["submission_time"];
                                        if(sub_time && sub_time>=tim)
                                            isSubmitted=true;
                                    }
                                }
                                if(isSubmitted) {
                                    cnt_SignalsOut=-1;
                                    break;
                                }
                            }

                        }
                        const char * sampleID=obj["sampleID"];
                        if(sampleID)*dicSample.set(sampleID)=1;
                        if(hideComputed) continue;

                        JSNode & ov=obj.child();
                        for(; ov.ok(); ov=ov.next()) {
                            
                            idx alen;const char * atr=ov.atr(&alen);
                            
                            if(memcmp(atr,"detail_progress",15)==0 ) { 
                                if(!doProgressReport)
                                    obj.del("detail_progress");
                            } 
                            if(memcmp(atr,"result_info",11)==0) { 
                                idx ov_dim = ov.dim();
                                if (hideResultInfo) {
                                    obj.del("result_info");
                                    ov_dim = 0;
                                }
                                for( idx ir=0; ir<ov_dim; ++ir ) {
                                    JSNode op=ov[ir];
                                    const char * ind=op.atr(0);

                                    idx confidence=((real)op["confidence"])*100;
                                    if(cmd==eDIUAntidote_MessageList)op.link("confidence",(idx)confidence);
                                    const char * thisSeverity=op["severity"];
                                    const char * thisClassN=op["class"];
                                    const char * thisSignal=op["signal"];
                                    idx thisMarked=op["markedSeen"];
                                    const char* thisSubmitted=op["isaSubmitted"];
                                    idx isMatch=true; 
                                    if(isMatch && hideSubmitted && (!thisSubmitted || strcmp(thisSubmitted,"0")))isMatch=false;
                                    if(isMatch && hideMarked && thisMarked)isMatch=false;
                                    if(severity && isMatch && (!thisSeverity || sString::compareChoice(thisSeverity, severity,0,0, 0, true, 0)==-1))isMatch=false;
                                    if(classN && isMatch && (!thisClassN || sString::compareChoice(thisClassN, classN,0,0, 0, true, 0)==-1))isMatch=false;
                                    if(signal && isMatch && (!thisSignal || sString::compareChoice(thisSignal, signal,0,0, 0, true, 0)==-1))isMatch=false;
                                    

                                    if(!isMatch) {
                                        ov.del(ind);
                                        continue;
                                    }                                    

                                    cnt_SignalsMatch++;
                                    
                                    if(startSignal  && cnt_SignalsMatch<startSignal) {
                                        ov.del(ind);
                                        continue;
                                    }

                                    if(cntSignal && cnt_SignalsMatch>cntSignal){
                                        ov.del(ind);
                                        continue;
                                    }
                                    if(hideLineage)
                                        op.del("lineage");
                                        
                                    cnt_SignalsOut++;
                                }
                            }
                        }
                        if(info)
                            obj.link("result_dim",cnt_SignalsMatch);
                    }
                    if (hideComputed) continue;
                    if(cnt_SignalsOut==-1 || (cnt_SignalsOut==0 && cmd==eDIUAntidote_MessageList) )
                        continue;
                    if(cnt_SignalsOut)
                        ++cntMatch;
                    if(start && cntMatch<start)
                        continue ;
                    
                    if(cmd==eDIUAntidote_MessageList){
                        idx oSubId=objs["0"]["_id"];
                        if(oSubId) {
                            sUsrObj o(m_User,sHiveId(oSubId,0));
                            o.propSetI("submission_time",sTime::gmtNow());
                            o.propSetI("submission_attempts",sub_attempts+1);
                        }
                    }
                    sbuf.cut(0);pip.print("$root",&sbuf);
                    if(!cnt || cntReported<cnt) {
                        if(cntReported)dataForm.printf(",\n    ");
                        dataForm.printf("%.*s",(int)sbuf.length()-30,sbuf.ptr(21));
                    }
                    if(cnt && cntReported>=cnt && info<2)
                        break;
                    cntReported++;
                }
                
            }

            if(dropbox && *dropbox) {
                sDic < sStr > dropBoxPath;
                sStr fn;
                sStr sbuf;
                for ( idx i=0; i< dicDropbox.dim(); ++i) {
                    sbuf.cut(0);
                    idx len;
                    const char* id = static_cast<const char*>(dicDropbox.id(i,&len));

                    if( dicSample.find(id,len) ) 
                        continue;

                    if(start && cntMatch<start)
                        continue ;

                    idx dropboxID=0; sscanf(dicDropbox[i].ptr(),"dropbox://%" DEC,&dropboxID);
                    sStr * dbP=0;
                    if(dropboxID){
                         dbP=dropBoxPath.set(&dropboxID);
                         if(dbP && !dbP->length()) {
                             sUsrObj dob(m_User,sHiveId(dropboxID,0));
                             if(dob.Id())
                                 dob.propGet("dropbox_path",dbP);
                         }
                    }
                    if(dbP && dbP->length()){
                        const char * flnm=strchr(dicDropbox[i].ptr(11),'/');
                        if(flnm) {
                            fn.printf(0,"%s%s",dbP->ptr(),flnm+1);
                            char *cut=strchr(fn.ptr(0),';');if(cut)*cut=0;
                        }
                    }

                    if(cnt && cntReported>=cnt && info<2)
                        break;
                     
                    time_t now=sFile::time(fn.ptr(0));
                    char buf[256];
                    strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
                    sbuf.printf(0,
                        "    {\n"
                        "            \"_id\":\"$newid\",\n"
                        "            \"_type\":\"da_sample\",\n"
                        "            \"sampleID\":\"%.*s\",\n"
                        "            \"storeID\":\"%.*s\",\n"
                        "            \"hive_in_a_box\" : {\n"
                        "                \"techID\":\"%s\",\n"
                        "                \"hoxID\":\"%s\"\n"
                        "            },\n"
                        "            \"process_status\" : {\n"
                        "                \"status\":\"parked\"\n"
                        "            },\n"
                        "            \"sample_collection\" : {\n"
                        "                \"sequencingTime\" : \"%s\"\n"
                        "            }\n"
                        "        }",
                        (int)len,id,
                        (int)dicDropbox[i].length(),dicDropbox[i].ptr(),
                        techID.ptr(),
                        hoxID.ptr(),
                        buf
                        );

                    if(!cnt || cntReported<cnt) {
                        if(cntReported)                        
                            dataForm.printf(",\n    ");
                        dataForm.printf("%.*s",(int)sbuf.length(),sbuf.ptr());
                    }

                    ++cntReported;
                    ++cntMatch;
                }
            }

            if(info==3){
                dataForm.printf(",\n        {\n            \"total_cnt\" : %" DEC "\n        }\n",cntMatch);
            }

            dataForm.printf("\n    ]\n}\n");
            if(cmd==eDIUAntidote_MessageList)
                dataForm.printf("]");

            overCall=false;
            outHtml();
            break;
        }
        case eDIUAntidote_List: {
            pForm->inp("cmd","objList");
            pForm->inp("raw","1");
            if(!pForm->value("_type"))
                pForm->inp("_type","da_sample");
            if(!pForm->value("mode"))
                pForm->inp("mode","json");
            DnaCGI::Cmd("objList");
        break;
        }

        case eDIUAntidote_MarkSignal:
        case eDIUAntidote_SendSignal:
        case eDIUAntidote_MessageReceived: {
            sVec < idx > ids, times,subids;
            const char * idlist=pForm->value("ids","");
            const char * subidlist=pForm->value("subids","");
            sString::scanRangeSet(idlist,0,&ids,0,0,0,false);
            sString::scanRangeSet(subidlist,0,&subids,0,0,0,false);
            sString::scanRangeSet(pForm->value("times"),0,&times,0,0,0,false);
            idx prvTime=sTime::gmtNow();
            for ( idx i=0; i<ids.dim(); ++i) {

                idx time=prvTime;
                if(i<times.dim()) {
                    time=prvTime=times[i];
                }

                sJson oj;m_User.objJson(ids[i],&oj);

                JSNode rJ(&oj,"$root");
                JSNode & events=rJ["result_info"];if(!events.ok())continue;
                idx cntDone=0;
                for( idx ie=0; ie<events.dim(); ++ie){
                    JSNode & ev=events[ie];

                    idx serno=ev["serial_no"], is;
                    for( is=0; is<subids.dim(); ++is) {
                        if(serno==subids[is])break;
                    }
                    
                    if(cmd==eDIUAntidote_SendSignal) { 
                        if(is<subids.dim())
                            ev.link("isaSubmitted",time);
                    } 
                    else if(cmd==eDIUAntidote_MarkSignal) { 
                        if(is<subids.dim())
                            ev.link("markedSeen",time);
                    } else {
                        if(is<subids.dim()) {
                            ev.link("isaSubmitted",time);
                            ++cntDone;
                        } else if((idx)(ev["isaSubmitted"])>(idx)0) {
                            ++cntDone;
                        }
                    }
                }
                if(cmd==eDIUAntidote_MessageReceived && cntDone==events.dim()) {
                    rJ.link("submission_confirmed",time);
                }

                if(cntDone>0 || cmd!=eDIUAntidote_MessageReceived )
                    user->propSetJson(&oj);
            }

            dataForm.printf("%s",idlist);
            outHtml();

        break;
        }
        case eDIUAntidote_Remove: {
            pForm->inp("cmd","objDel");
            pForm->inp("raw","1");
            DnaCGI::Cmd("objDel");

        break;
        }
        default:
            break;
    }

    return 1;
}












        

        


        







            





