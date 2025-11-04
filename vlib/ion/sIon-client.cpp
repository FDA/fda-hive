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
#include <ion/sIon-client.hpp>
#include <slib/utils/tbl.hpp>

const char * listCommands=
    "wander" _
    "wantbl" _
    "brquery" _
    "brgetjson" _
    "brsetjson" _
    "hcRoot" _
    "hcQry" _
    "hcType" _
    "hcType+" _
    "hcObj" _
    "hcUser" _
    "hcGroup" _
    "hcRule" __;

enum enumCommands{
    eWander,
    eWanderTbl,
    eBrQuery,
    eBrGetJson,
    eBrSetJson,
    eHCRoot,
    eHCQry,
    eHCType,
    eHCTypePlus,
    eHCObj,
    eHCUser,
    eHCGroup,
    eHCRule,
    eLast
};


idx sIonClient::Cmd(sIO * io, const char * cmd, sVar * pForm)
{
    sString::compareChoice( cmd, listCommands,&cmdNum,false, 0,true);
    if(cmdNum==-1){return 0;}

    idx res=0;
    res=CmdBiRel(io, cmd, pForm);
    if(res)return res;

    if( pForm) {
        idx ionlen;const char * ion=pForm->value("ion",0,&ionlen);
        wander.attachIons(ion,sMex::fReadonly);
        idx qrylen;const char * query=pForm->value("query",0,&qrylen);
        wander.traverseCompile(query, qrylen, errIO, true);
        wander.maxNumberResults=pForm->ivalue("cnt",200);
        wander.setSepar(pForm->value("sepField"),pForm->value("sepRecord"));
        wander.debug=pForm->ivalue("debug");

    }


    wander.pTraverseBuf=io;

    switch(cmdNum) {

        case eWander:{
            wander.retrieveParametricWander(pForm,0);
            wander.traverse();
        } break;
        case eWanderTbl:{
            idx tbllen;const char * tbl=pForm->value("tbl",&tbllen);
            #ifdef USEOLDTBL
                sTbl tbl;tbl.parse(tbl, tbllen);
                wander.traverseTable(&tbl,0);
            #else
                sTxtTbl txttbl;txttbl.setBuf(tbl, tbllen);txttbl.parse();
                wander.traverseTable(0,&txttbl);
            #endif
        } break;
        default:
            break;
    };

    return res;
}



void sIonClient::initMaster(sIO * io, const char * cmd, sVar * pForm)
{

    master.errIO=io;
    if( pForm ) {
        idx flags=sMex::fReadonly;
        if(cmdNum>=eBrSetJson )flags&=~(sMex::fReadonly);
        const char * ion_master=pForm->ptrvalue("ion_master");
        if(ion_master){
            master.init(ion_master,flags);

        }
        master.debug=pForm->ivalue("debug");
        master.iterCntRangeRoot=pForm->ivalue("brCnt",0);
        master.iterCntRangeDeep=pForm->ivalue("brCntD",0);
        master.iterStartRange=pForm->ivalue("brStart",0);
        master.iterMaxDepth=pForm->ivalue("brDepth",sIdxMax);

        const char * brOut=pForm->value("brOut","json|totals|stotals|space|frame");
        if(brOut)sString::xscanf(brOut,sIonBirel::ionScanOutFlagsFormat,&master.outFlags);

        master.setSearch(sIonBirel::eSearch,pForm->value("brSearch"));
        master.setSearch(sIonBirel::eInto,pForm->value("brInto"));
        master.setSearch(sIonBirel::eFields,pForm->value("brFields"));


    }
}

idx sIonClient::CmdBiRel(sIO * io, const char * cmd, sVar * pForm)
{


    initMaster(io, cmd, pForm );

    idx res=1;
    sIonWander * w=master.wander("");
    w->pTraverseBuf=io;


    sIonBirel * ionBirel=(sIonBirel * )&master;


    switch(cmdNum) {

        case eBrQuery: {
            const char * qry=pForm->value("qry");
            w->traverseCompile(qry, 0, errIO, true);
            w->retrieveParametricWander(pForm,0);
            return w->traverse();
        } break;
        case eBrSetJson: {
            idx jsonlen;const char * json=pForm->value("json",0,&jsonlen);
            sJax js(0,json,jsonlen);
            ionBirel->parse(&js);
            if( js.errIO.length() ) {
                js.errorReport(io);
            }
        } break;
        case eBrGetJson: {
            master.iterateNodes(pForm->value("sub"));
        } break;

        case eHCRoot: {
            master.setRoot(pForm->value("sub","$root"));
        } break;

        case eHCQry: {
            master.iterateNodes(pForm->value("sub","$root"));
        } break;

        case eHCType: {
            master.attachTypeIterators(io);
            master.addDomainIterator("_field", 0, 0, 0, pForm->ivalue("startDepth",2),pForm->ivalue("endDepth",10));
            master.iterateObjects( pForm->value("types") , 0,0,0, "?type");
        } break;

        case eHCTypePlus: {
            master.getInheritedTypes( pForm->value("types"), 0, io) ;
        } break;

        case eHCObj: {
            master.attachObjectListTraverseQuery(io);
            master.iterateObjects( pForm->value("types") , 0, pForm->value("ids") , 0 );
        } break;

        case eHCUser: {
            master.attachObjectListTraverseQuery(io);
            master.iterateObjects( "+hc_group" , 0, pForm->value("ids") , 0 );
        } break;

        case eHCGroup: {
            master.attachGroupListListTraverseQuery (io);
            master.iterateObjects( "+hc_group" , 0, pForm->value("ids"), 0, "?group" );
        } break;

        case eHCRule: {
            master.addRule( pForm->value("act"), pForm->value("party"), pForm->value("obj"), pForm->value("infparty"), pForm->value("infobj") );
        } break;

        default:
            master.freeSearch();
            return 0;
    };


    master.freeSearch();
    return res;
}


