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

function vjQPride ()
{
    this.req=0;

    this.objCls='vjQPride-'+Math.random();
    this.delayCheckBG = 500;
    this.delayCheckDB_step =20;
    this.delayCheckBG_max = 5000;
    this.cachedReqIDs=new Object();

    vjObj.register(this.objCls,this);

//    this.ds_load = vjDS.add("waiting for request","dsQPrideDatasource","static:// ");

    this.killRequest=function(lreq)
    {
        if(!lreq)lreq=this.req;
        linkSelf("-qpKillReq&req="+lreq);
    };

    this.setAction=function(lreq, act, isgrp)
    {
        if(!lreq)lreq=this.req;
        if (isgrp === undefined) isgrp = this.reqIsGrp;
        // this is broken since: 1. action is not read from from; 2. grpSetAction cmd do not exists in CGI
        linkCmd("req="+lreq+"&cmd=-"+(isgrp ? "grp" : "req")+"SetAction&action="+act);
    };
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Request Submission on Background
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    this.backgroundCheckFail = function(params, txt)
    {
        var tbl=new vjTable(txt, 0, vjTable_hasHeader);
        var errTxt;
        if (tbl.rows.length) {
            errTxt = tbl.rows[0].infoTxt;
        } else {
            errTxt = "request " + this.req + " failed";
        }

        if (params.loadmode=="download" ) {
            alert("Error: " + errTxt);
        } else {
            console.error(errTxt);
            vjObjAjaxCallback(params.callbackParams, "");
        }
    };

    this.backgroundCheckRequest=function(params, txt) // we use container here as a function name
    {
        var tbl=new vjTable(txt, 0, vjTable_hasHeader);
        var RQ=tbl.rows[0];

        this.req = parseInt(RQ.reqID);
        if (this.req && !isNaN(this.req)) {
            this.reqIsGrp = false;
            RQ.rootID = RQ.reqID;
        } else {
            this.req = parseInt(RQ.grpID);
            if (isNaN(this.req)) {
                this.req = 0;
            }
            this.reqIsGrp = true;
            RQ.rootID = RQ.grpID;
        }

        RQ.dataID = params.dataReq ? params.dataReq : RQ.rootID;

        RQ.stat=parseInt(RQ.stat);
        if (isNaN(RQ.stat))
            RQ.stat = 0;

        var nm_ds = 'vjQP_req' + RQ.rootID;
        var cur_ds = vjDS[nm_ds];

        if(!txt || txt.length==0){callback(0,0); return ;}

        var qp_params = params.QP_params ? params.QP_params : params;

        if (qp_params.callbackParams) {
            qp_params.callbackParams.RQ = RQ;
            if (qp_params.callbackParams.callback_submitted && !qp_params.called_callback_submitted) {
                var key, params_callback_submitted = {};
                for (key in qp_params.callbackParams) {
                    params_callback_submitted[key] = qp_params.callbackParams[key];
                }
                params_callback_submitted.callback = qp_params.callbackParams.callback_submitted;
                qp_params.called_callback_submitted = true;
                vjObjAjaxCallback(params_callback_submitted, RQ);
            }
        }

        if(RQ.stat>=5) {
            if(cur_ds) {
                delete vjDS[nm_ds];
            }
            if(RQ.stat>5 && qp_params.loadmode!="retrieveUrl") {
                //level Error: 500
                var url="?cmd=-qpReqInfo&req="+RQ.dataID+"&raw=1&level=500";
                ajaxDynaRequestPage(url, this, function(this_, responseText){ this_.backgroundCheckFail(qp_params, responseText); });
            } else {
                //alerJ(RQ.reqID,qp_params)
                if(!isok(qp_params.blob) && (qp_params.loadmode!="download" && qp_params.loadmode!="retrieveURL") )
                    vjObjAjaxCallback(qp_params.callbackParams,RQ);
                else {
                    var url="?cmd=-qpData&req="+RQ.dataID+"&raw=1&grp=1&dname="+qp_params.blob+"&default=error:%20"+RQ.dataID+"%20"+qp_params.blob+"%20not%20found";
                    //var url="?cmd=-qpData&req="+RQ.dataID+"&raw=1&grp=1&dname="+blobname+"&default=error:%20"+RQ.dataID+"%20"+blobname+"%20not%20found";
                    if (isok(qp_params.saveas)) {
                        url += "&dsaveas=" + qp_params.saveas;
                    }

                    if(qp_params.loadmode=="retrieveURL") {
                        vjObjAjaxCallback(qp_params.callbackParams, url);
                    } else if (qp_params.loadmode=="download" ) {
                        document.location=url;
                    } else {
                        ajaxDynaRequestPage(url, qp_params.callbackParams, vjObjAjaxCallback);
                    }
                }
            }
        }
        else if(this.delayCheckBG) {
            if(RQ.rootID){
                if(!cur_ds) {
                    var ds_title = params.callbackParams.title?params.callbackParams.title:'preparing requested data ' + RQ.rootID;
                    cur_ds = vjDS.add(ds_title,nm_ds,"static://",vjObjAjaxCallback);
                }
                cur_ds.RQ = RQ;
                if(!params.QP_params) {
                    var t = params;
                    params.QP_params = cpyObj(t);
                    params.loadmode = undefined;
                }
                setTimeout(function(){cur_ds.reload("http://?cmd=-qpRawCheck&raw=1&req="+RQ.rootID,true,params);}, (params.delay ? params.delay : this.delayCheckBG));

//                setTimeout("linkCmd('-qpRawCheck&raw=1&req="+RQ.rootID +"',"+JSON.stringify(params)+",vjObjAjaxCallback)", (params.delay ? params.delay : this.delayCheckBG) );
                if( this.delayCheckBG < this.delayCheckBG_max ) {
                    this.delayCheckBG += this.delayCheckDB_step;
                }
            }
        }
    };

    this.backgroundExecution=function(cmd, callbackAjax )
    {
        var params = { objCls: this.objCls , callback: "backgroundCheckRequest", callbackParams: callbackAjax};
        linkCmd(cmd+"&raw=1", params ,vjObjAjaxCallback);
        //linkCmd(cmd+"&raw=1",JSON.stringify(params),QPride_backgroundCheckRequest);
    };

    this.backgroundRetrieveBlob=function(cmd, callbackAjax, namefetch, dataReq, delay, loadmode, saveas)
    {
        var params = { objCls: this.objCls , callback: "backgroundCheckRequest", blob : namefetch, callbackParams: callbackAjax , delay: delay, dataReq: dataReq, loadmode: loadmode, saveas: saveas };
        if(cmd.indexOf("?cmd") == 0 ){
            cmd = cmd.substr(5);
        }
        linkCmd(cmd+"&raw=1", params ,vjObjAjaxCallback);

    };


    /*

    this.retrieveEmptyReqID=function(param, content)
    {
        this.cachedReqIDs[param.nametag]={svc:param.svc, req:parseInt(content)};
        if(param.callbackFunc){
            funcLink(param.callbackFunc, param, content , this.cachedReqIDs[param.nametag] );
        }

    }

    this.generateEmptyReqID=function(svc, nametag, callbackFunc)
    {
        var params = { objCls: this.objCls , callback: "retrieveEmptyReqID", svc: svc, nametag: nametag , callbackFunc : callbackFunc };
        linkCmd("-qpProcSubmit&svc="+svc+"&raw=1",params,vjObjAjaxCallback);
    }*/


}

var vjQP = new vjQPride();
var req=0;

//# sourceURL = getBaseUrl() + "/js/vjQPride.js"