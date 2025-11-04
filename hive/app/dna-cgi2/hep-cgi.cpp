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
#include <violin/violin.hpp>
#include <regex.h>
#include <slib/utils/json/parser.hpp>
#include <slib/utils/json/parser.hpp>
#include <ion/sJson.hpp>
using namespace slib;

enum enumHepCommands
{
    eGetPatientJson,
    ePatSitesCount,
    eRedCapCsv
};



idx DnaCGI::CmdHep(idx cmd)
{

    sStr blobName("result.json");
    sStr outPutContent;
    
    const char * quest_opt = formValue("questlist", 0);
    sStr quest_list;
    sString::searchAndReplaceSymbols(&quest_list, quest_opt, 0, ";", 0, 0, true, true, true, true);

    const char * provider_opt = formValue("providerlist", 0);
    sStr provider_list;
    sString::searchAndReplaceSymbols(&provider_list, provider_opt, 0, ";", 0, 0, true, true, true, true);
   


    switch(cmd) {
        case eGetPatientJson: {
            const char * mode = formValue("mode", 0,"json");

            const char * patients_opt = formValue("patientlist", 0);
            sStr patient_list;
            sString::searchAndReplaceSymbols(&patient_list, patients_opt, 0, ";", 0, 0, true, true, true, true);
            idx patient_cnt = sString::cnt00(patient_list.ptr(0));

            if (!patient_cnt) {
                sStr errOut("{\"error\":\"no patient selected\"}");
                outBin(errOut.ptr(), errOut.length(), 0, true, blobName.ptr(0));
                return 1;
            }
            sUsrObjRes obj_res;
            idx cnt;
            m_User.objs2("^fhir_pro$", obj_res,(udx*)&cnt);

            sJson newJson;
            JSNode root(&newJson);

            JSNode patientArr = root.linkarr("patients");


            sStr myBuff;
            for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
                sUsrFile ufile(*obj_res.id(it), &m_User);
                const char * patientID = ufile.propGet("patient-id");

                idx foundPat = -1;
                sString::compareChoice(patientID, patient_list.ptr(), &foundPat, false, 0, true);
                if (foundPat==-1) {
                    continue;
                }

                JSNode pat_node = patientArr.linkobj("#");
                pat_node.link("ID",patientID);
                pat_node.link("resourceType","Bundle");
                pat_node.link("type","collection");
                JSNode entry_node = pat_node.linkarr("entry");

                sStr path;ufile.makeFilePathname(path, "_.json");
                sJsonFile pip(path);

                JSNode entryList=pip.node("$root.entry");
                
                
                for( idx ie=0; ie<entryList.dim(); ++ie ) {
                    JSNode rs_node = entryList[ie]["resource"]["resourceType"];
                    idx a=0;
                    if ( rs_node.ok()){
                        const char * rs_type_val = rs_node.val(&a);
                        
                        if ( strncmp(rs_type_val,"QuestionnaireResponse",a)==0 ) {
                            JSNode quest_node = entryList[ie]["resource"]["questionnaire"];                        
                            if (quest_node.ok()) {                            
                                const char * quest_val = quest_node.val(&a);
                                myBuff.printf(0,"\"%.*s\"",(int)a,quest_val);
                                const char * questChoice = quest_list.ptr();
                                for (idx i=0; questChoice; i++) {
                                    if (strstr(myBuff.ptr(),questChoice)) {
                                        entry_node.linkobj("#").copy(entryList[ie]);
                                        break;
                                    }
                                    questChoice = sString::next00(questChoice);
                                }
                            }
                        }                     
                    }
                    JSNode provider_node = entryList[ie]["resource"]["providerName"];
                    if (provider_node.ok()){
                        const char * provider_val = provider_node.val(&a);
                        myBuff.printf(0,"\"%.*s\"",(int)a,provider_val);
                        const char * providerChoice = provider_list.ptr();
                        for (idx i=0; providerChoice; i++) {
                            if (strstr(myBuff.ptr(),providerChoice)) {
                                entry_node.linkobj("#").copy(entryList[ie]);
                                break;
                            }
                            providerChoice = sString::next00(providerChoice);

                        }
                    }
                    
                }

            }
            blobName.printf(0,"o-%" DEC "-hep-patients.json",reqId ? reqId : getpid() + rand());
            myBuff.printf(0,"/tmp/%s",blobName.ptr());
            newJson.save(myBuff.ptr());
            if (strcmp(mode,".csv")==0) {
                sJsonFile pip(myBuff.ptr(),0,0);
                JSNode rootNode=pip.node("$root");if (!rootNode.ok()) return 1 ;

                sJson::CSVFlattener cf;
                cf.streamJsonIn(&rootNode);
                cf.printCSV(&dataForm);
                blobName.printf(0,"o-%" DEC "-hep-patients.csv",reqId ? reqId : getpid() + rand());
                outBin(dataForm.ptr(), dataForm.length(), 0, true, blobName.ptr(0) );
            } else {
                sFil oo(myBuff.ptr(),sMex::fReadonly);
                outBin(oo.ptr(), oo.length(), 0, true, blobName.ptr(0));
            }
            return 1;
        }break;
        case ePatSitesCount:{
            const char * site_opt = formValue("sitelist", 0);
            sStr site_list;
            sString::searchAndReplaceSymbols(&site_list, site_opt, 0, ";", 0, 0, true, true, true, true);
           
            const char * site[11]={"NYU","MMC","UPENN","NWH","MAYO","MES","NWH","STB","UMIA","UTHS","WUSTL"};
            sDic <idx> site_dict;
            *site_dict.set("NYU Langone")=0;
            *site_dict.set("Maine Medical Center")=1;
            *site_dict.set("University of Pennsylvania")=2;
            *site_dict.set("MN Epilepsy Group, P.A.")=3;
            *site_dict.set("Mayo Clinic")=4;
            *site_dict.set("Mid-Atlantic Epilepsy and Sleep Center")=5; 
            *site_dict.set("Northwell Health, Division of Pediatric Neurology")=6;
            *site_dict.set("Saint Barnabas")=7;
            *site_dict.set("University of Miami")=8;
            *site_dict.set("UT Health San Antonio")=9;
            *site_dict.set("Washington University in St. Louis")=10;

            
            const char * siteChoice = site_list.ptr();
  
            sDic <idx> patCount;
            for (idx i=0; siteChoice; i++) {
                if (site_dict.find(siteChoice,sLen(siteChoice))) {
                   *patCount.set(site[*site_dict.get(siteChoice)])=0;
                }
                siteChoice = sString::next00(siteChoice);
            }

            if (!patCount.dim()) {
                sStr errOut("{\"error\":\"no sites found in our database\"}");
                outBin(errOut.ptr(), errOut.length(), 0, true, blobName.ptr(0));
                return 1;
            }
            sUsrObjRes obj_res; idx cnt;
            m_User.objs2("^fhir_pro$", obj_res,(udx*)&cnt);

           sStr myBuff;
            for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
                sUsrFile ufile(*obj_res.id(it), &m_User);
                const char * patientID = ufile.propGet("patient-id");

                for (idx ik=0; ik < patCount.dim(); ik++) {
                    idx keyLen=0;
                    const char * key = (const char *)patCount.id(ik,&keyLen);
                    if (strncmp(key,patientID,keyLen)==0) {
                        patCount[key]+=1;
                    }
                }
            }
            myBuff.printf(0,"{");             
            for (idx ik=0; ik < patCount.dim(); ik++) {
                idx keyLen=0;
                const char * key = (const char *)patCount.id(ik,&keyLen);
                if (ik>0) {
                    myBuff.printf(",");
                }
                myBuff.printf("\"%.*s\":%" DEC "",(int)keyLen, key,patCount[key]);
            }
            myBuff.printf("}");
            blobName.printf(0,"o-hep-patSitesCount-%" DEC ".json",reqId ? reqId : getpid() + rand());
            outBin(myBuff.ptr(), myBuff.length(), 0, true,blobName.ptr() );
            return 1;
        }break;

        case eRedCapCsv: {
            const char * obj_id = pForm->value("objId",0);

             if (!obj_id) {
                outPutContent.printf(0,"{'status':'Error','msg':'Please provide an object Id'}");
                outBin(outPutContent.ptr(), outPutContent.length(), 0, true,blobName.ptr() );
                return 1;
            }
            sJsonFile pip(obj_id,0,0);
            JSNode rootNode=pip.node("$root");if (!rootNode.ok()) return 1 ;

            sJson::CSVFlattener cf;
            cf.streamJsonIn(&rootNode);

            cf.printCSV(&dataForm);
            outBin(dataForm.ptr(), dataForm.length(), 0, true, "json2CSV" );

            
        } break;
        default:
            break;
    }

    return 1;
}



