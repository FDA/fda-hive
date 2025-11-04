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

#include <xlib/s_curl.hpp>
#include <ulib/ucgi.hpp>
#include <ulib/uobj.hpp>
#include "uperm.hpp"
#include <ion/sJson.hpp>

using namespace slib;


idx xternDbg=1;
static sCurl xternCurl;
sIO out,err;
sJson xternJson(0,0,&out,&err);
const char * xternResponse=0;
sStr xternURLBuf;

sStr xtern_CHIOS_BASE;

static sJson * xtern_Submit(const char * xtern_base, const char * url, sUsr * tokenUsr, const char * postbody, ... )
{
    sStr h,b;
    xternCurl.io->cut(0);

    const char * token=tokenUsr ? tokenUsr->getSingletonVar("user-settings","xternAPI_blockchain_token1") : 0 ;
    if(token)xternCurl.setHeader("Content-Type","application/json","Authorization",h.printf(0,"Bearer %s",token),0);
    else xternCurl.setHeader("Content-Type","application/json",0);

    if(postbody){
        sCallVarg(b.vprintf,postbody);
        xternCurl.setPost( b.ptr(), 0,0,b.length(),0);
        xternCurl.setCipher("ECDHE-RSA-AES256-GCM-SHA384");
        xternCurl.Post(xternURLBuf.printf(0,"%s%s",xtern_base,url),0);
    } else {
        xternCurl.Get(xternURLBuf.printf(0,"%s%s",xtern_base,url));
    }

    xternResponse=xternCurl.io->ptr(0);
    if(xternResponse) {
        xternJson.destroy();
        xternJson.initMem(xternResponse,sLen(xternResponse));
    }



    if(xternDbg) {
        sFil llog("xtern.log",sMex::fMapRemoveFile);
        llog.printf("------- URL --------\n"
            "%s/%s\n"
            "-------- TOKEN ------------\n"
            "%s\n"
            "-------- INPUT ------------\n"
            "%s\n"
            "------- RESPONSE --------\n"
            "%s\n"
            "--------------------\n"
            ,xtern_base,url,token ? token : "" ,b.ptr(),xternResponse ? xternResponse : "" );
    }

    return &xternJson;
}


static idx xtern_CHIOS_Cmd( sUsrCGI * ucgi, sVar * pForm, const char * cmd, va_list ap)
{
    sVA(const char * , command , ap);

    sStr b;

    if(sIs(command,"xternList")){

        sVec < sJson::TrippleSet > tbl;
        xtern_Submit( xtern_CHIOS_BASE , b.printf("consent/%" DEC,ucgi->m_User.Id()) ,&(ucgi->m_User),0)
            ->tabulateChildren("arr","$root", &tbl);


        for(idx i=0; i<tbl.dim(); ++i) {
            sJson::TrippleSet & ts=tbl[i];
            xternJson.link(ts.ival(), "consentingPartyEmail", ucgi->m_User.Email() );

            udx requestorId=atoudx(xternJson.value(ts.ival(),"record.requestorId"));
            sUsr usrReq((idx)ucgi->m_User.getGroupUser(requestorId));
            xternJson.link(ts.ival(), "requestingPartyEmail", usrReq.Email() );
        }
        xternJson.print("$root",&(ucgi->dataForm));

        return 1;
    }

    if(sIs(command,"xternRevoke")){
        sVec < idx > idList;
        sString::scanRangeSet(pForm->value("ids",0),  0, &idList, 0, 0,0);
        for(idx i=0; i<idList.dim() ; ++i) {
            sUsrObj * obj = ucgi->m_User.objFactory(sHiveId(idList[i],0));
            const char * contractId=obj->propGet("contract_id");
            xtern_Submit(xtern_CHIOS_BASE , "consent/revoke",&(ucgi->m_User),
                "{"
                    "\"contractId\": \"%s\""
                "}"
                ,contractId
                );
        }

        return 1;
    }

    return 0;
}

static idx xtern_CHIOS_userSet( sUsrCGI * ucgi, sVar * pForm, const char * cmd , va_list ap)
{

    sVA(const char * ,email, ap);
    sVA(idx, id, ap);

    const char * token=xtern_Submit(xtern_CHIOS_BASE , "patient/register",0,
        "{"
            "\"email\":\"%s\","
            "\"username\":\"%" DEC "\""
        "}"
        ,email,id )
        ->value("$root.token");
    if(!token) {
        return 1;
    }
    ucgi->m_User.setSingletonVar("user-settings","xternAPI_blockchain_token1", token,0);
    return 1;
}

static idx xtern_CHIOS_propSet( sUsrCGI * ucgi, sVar * pForm, const char * cmd, va_list ap)
{

    idx lid;
    sStr fields,tok;
    fields.printf("{\"consentingPartyId\":\"%" DEC "\",",ucgi->m_User.Id());
    for(idx i=0,io=0; i<pForm->dim(); ++i) {
        const char * id=(const char*)pForm->id(i,&lid);

        if(id[0]=='.' || memcmp(id,"prop.",5)!=0)continue;

        sString::searchAndReplaceSymbols(&tok,id,lid,".",0,0,true,false,false,false,false);
        const char * type=sString::next00(tok.ptr());
        const char * name=sString::next00(type);

        if(io==0){
            fields.printf("\"dataType\":\"%s\",\"fieldNames\":[",type);
        }else
            fields.add(",",1);
        fields.printf("\"%s\"",name);
        ++io;
        tok.cut(0);
    }
    fields.printf("]}");

    const char * provId= xtern_Submit(xtern_CHIOS_BASE ,  "patient/provenancelog",&(ucgi->m_User),
        "%s"
        ,fields.ptr())
        ->value("$root.identifier");

    sVA(sVec<sHiveId> *,obj_ids,ap);
    for(idx io=0; io<obj_ids->dim(); ++io) {
        sUsrObj * obj = ucgi->m_User.objFactory((*obj_ids)[io]);
        obj->propSet("provenance_id", provId);
    }

    return 1;
}


static idx xtern_CHIOS_permSet( sUsrCGI * ucgi, sVar * pForm, const char * cmd , va_list ap)
{

    sVA(udx, datatypeId, ap);
    sVA(udx, requestorId, ap);

    const char * contractId=xtern_Submit(xtern_CHIOS_BASE , "consent/grant",&(ucgi->m_User),
        "{"
            "\"consentingPartyId\": \"%" DEC "\","
            "\"requestorId\": \"%" DEC "\","
            "\"scope\": {"
                "\"description\": \"%s\","
                "\"documentUri\": \"%s\","
                "\"timeLimits\": {"
                    "\"startTime\": \"%s\","
                    "\"endTime\": \"%s\""
                "},"
                "\"typeLimits\": {"
                    "\"datatypeId\":\"%" UDEC "\","
                    "\"fieldNames\": [\"%s\"]"
                "},"
                "\"useLimits\": {"
                    "\"algorithmIds\": [\"%s\"]"
                "}"
            "},"
            "\"additionalTerms\": {"
                "\"terms\": \"%s\""
            "}"
        "}"
        ,ucgi->m_User.Id()
        ,requestorId
        ,pForm->value("xternDocumentUri","granting general access")
        ,pForm->value("xternDescription","http://xlhive.com/consent.pdf")
        ,pForm->value("xternStartTime","2020-01-12T17:20:19.165Z")
        ,pForm->value("xternEndTime","2028-01-12T17:20:19.165Z")
        ,datatypeId
        ,pForm->value("xternFieldNames","all")
        ,pForm->value("xternAlgorithmIds","download")
        ,pForm->value("xternAdditionalTerms","-")
        )
        ->value("$root.contractId");

    sUsrObj * obj = ucgi->m_User.objFactory(sHiveId(datatypeId,0));
    if(obj)
        obj->propSet("contract_id", contractId);

    return 1;
}


static idx xtern_CHIOS_propGet( sUsrCGI * ucgi, sVar * pForm, const char * cmd , va_list ap)
{
    sVA( udx , datatypeId, ap);

    udx user,group=ucgi->m_User.getObjOwner(sHiveId(datatypeId,0),&user);
    sUsr usr((idx)user);

    const char * tokenRequestor=ucgi->m_User.getSingletonVar("user-settings","xternAPI_blockchain_token1") ;
    const char * tokenOwner=usr.getSingletonVar("user-settings","xternAPI_blockchain_token1") ;
    if(!tokenRequestor || ! tokenOwner)
        return 1;


    idx dataOwner=user;
    udx requestor=ucgi->m_User.groupId();
    if(group==requestor)
        return 1;
    const char * consentId=pForm->value("consentId",0);
    if(!consentId){
        sUsrObj * obj = ucgi->m_User.objFactory(sHiveId(datatypeId,0));
        consentId=obj->propGet("contract_id");
    }

    const char * isTransactionValid=xtern_Submit(xtern_CHIOS_BASE , "consent/validate",&(usr),
        "{"
            "\"dataOwner\": \"%" UDEC  "\","
            "\"requestor\": \"%" UDEC "\","
            "\"consentId\": \"%s\","
            "\"datatypeId\": \"%" UDEC "\","
            "\"contractRequest\": {"
                "\"terms\": \"%s\""
            "}"
        "}"
        ,dataOwner
        ,requestor
        ,consentId
        ,datatypeId
        ,pForm->value("xternAdditionalTerms","-")
        )
        ->value("$root.isTransactionValid");

    bool valid=(isTransactionValid && strcmp(isTransactionValid,"true")==0) ? true : false;

    return valid ? 1 : 0 ;
}






















sStr xtern_EMBHEP3_BASE;

static idx xtern_EMBHEP3_propSet( sUsrCGI * ucgi, sVar * pForm, const char * cmd, va_list ap)
{

    const char * pars[]={
        "_id",
        "_type",
        "is_draft",
        "euid",
        "created",
        "modified",
        "is_draft",
        "date_completed",
        "date_completed_system",
        "participantID",
        "Set2PatientTaskID",
        "latest_editor",
        "authorID",
        "Set2TaskID",
        "SiteName",
        "Set2TaskSet2ID",
        "Set2EventID",
        "Set2EventSet2ID",
        "Set2PatientEventID"
    };

    sStr b,fields;
    idx cntFields=0, cntObj=0;
    sVA(sVec<sHiveId> *,obj_ids,ap);
    fields.printf("{");
    for(idx io=0; io<obj_ids->dim(); ++io) {
        if( !(*obj_ids)[io] ) continue;
        sUsrObj * obj = ucgi->m_User.objFactory((*obj_ids)[io]);
        if(cntObj)fields.printf(",");
        if(obj_ids->dim()>1)fields.printf("[");
        cntFields=0;
        for (idx ip=0; ip<sDim(pars); ++ip) {
            obj->propGet(pars[ip], &b);
            if(b.length()==0)continue;
            if(cntFields)fields.printf(",");
            sUsrTypeField::EType tp=obj->propGetValueType(pars[ip]);
            if(tp==sUsrTypeField::eTime || tp==sUsrTypeField::eDateTime) {
                time_t now=obj->propGetDTM(pars[ip]);
                struct tm ts; ts=*localtime(&now);
                char buf[80];

                ts = *localtime(&now);
                strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &ts);
                fields.printf("\"%s\":\"%s\"",pars[ip],buf);
            }
            else if(tp==sUsrTypeField::eBool) {
                bool istrue=obj->propGetBool(pars[ip]);
                fields.printf("\"%s\":%s",pars[ip],istrue ? "true" : "false");
            }
            else
                fields.printf("\"%s\":\"%s\"",pars[ip],b.ptr(0));
            b.cut(0);
            cntFields++;
        }
        if(cntFields)fields.printf(",\"_id\": %s",obj->IdStr());
        if(obj_ids->dim()>1)fields.printf("]");
        if(cntFields)cntObj++;
    }

    fields.printf("}");
    if(!cntObj)return 0;
    xtern_Submit(xtern_EMBHEP3_BASE,"partner/hive/webhook",&(ucgi->m_User),
            "%s"
            ,fields.ptr());
    if(xternResponse)
        ucgi->dataForm.printf("\n\n%s",xternResponse);
    return 1;
}




void sUsrCGI::xternInitModules(void)
{

    xternAddAPI("CHIOS", "Cmd", (sCallbackUniversal)xtern_CHIOS_Cmd);
    xternAddAPI("CHIOS", "login", (sCallbackUniversal)xtern_CHIOS_userSet);
    xternAddAPI("CHIOS", "userSet", (sCallbackUniversal)xtern_CHIOS_userSet);
    xternAddAPI("CHIOS", "propset", (sCallbackUniversal)xtern_CHIOS_propSet);
    xternAddAPI("CHIOS", "permset", (sCallbackUniversal)xtern_CHIOS_permSet);
    xternAddAPI("CHIOS", "propget", (sCallbackUniversal)xtern_CHIOS_propGet);

    xternAddAPI("EMBHEP3", "propset", (sCallbackUniversal)xtern_EMBHEP3_propSet);

    sString::SectVar genVarsCentral[]={
        {0, 0, 0, 0, 0 }
        };

    sFil fl("xtern.cfg",sMex::fReadonly);
    if(fl) {
        sString::xscanSect(fl.ptr(0), fl.length(),genVarsCentral,0);
    }

}


