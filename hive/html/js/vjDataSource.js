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


function vjDataSource( sourcebase )
{
    this.parser='';
    this.data='';
    this.page_request='';
    this.callbacks={'refreshed': new Array()};
    this.evaluator=false;
    this.progressor='QP_bgProgress';
    this.dataurl='';
    this.state='';
    this.div='';
    this.timeLoad=null;
    this.timeDone=null;
    this.urlParams=null;


    this.backendCGI = function(url)
    {
        return docLocValue("backend", 0, url);
    };

    this.clone=function (sourcebase)
    {
        if(!sourcebase)sourcebase=this;
        var i=0;
        for (i in sourcebase ) {
            this[i] = sourcebase[i];
        }

    };
    this.clone(sourcebase);
    this.vjType="vjDataSource";
    this.objCls=this.vjType+"-"+this.name;
    vjObj.register(this.objCls,this);

    this.load=function( params , fileName)
    {
        if( typeof(params) === "undefined" ) {
            params = new Object();
        }
        else if( typeof(params) !== "object" ) {
            params = {'loadmode':params};
        }

        if(params.loadmode!='download') {
            if(this.hasDataForUrl()) {
                return true;
            }
            if(this.state=='load' &&  this.dataurl==this.url )
                return true;

            this.state='load';
            this.timeLoad=new Date();
            this.timeDone=null;
            this.dataurl=this.url;

            this.call_callbacks("fetching");
        }

        var spl=this.url.split(":
        var src=spl[0];

        var url=spl.slice(1).join(":

        if(this.urlParams){
            let urlParamKeys = Object.keys(this.urlParams)
            let urlParams = this.urlParams
            urlParamKeys.forEach(function( key ) {
                let value = urlParams[key] !== '' 
                            ? urlParams[key] 
                            : '-'
                url = urlExchangeParameter(url , key , value)
            })
        }

        url=evalVars(url,"$(",")");

        if(src=="file"){
            url = "-qpFile&raw=1&file=" + url;
        } if( src.indexOf("cors_http")==0 ) {
            src = src.substring("cors_".length);
            params.isCORS = true;
            url = this.url.substring("cors_".length);
        }
        
        if(params.loadmode=='download' && src.indexOf("static") == 0){
                if(!fileName) fileName = "file";
                
                var url = this.url;
                var index = url.indexOf(":
                if (index < 0 ) return;
                var text = url.substring(index+3);

                var tmpA = document.createElement('a');
                tmpA.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
                tmpA.setAttribute('download', fileName);
              document.body.appendChild(tmpA);
              tmpA.click();
              document.body.removeChild(tmpA);
                
                return;
        }

        if( params.loadmode=='download' && src.indexOf("qpbg_")!=0 )  {
            if(src=='qp_data')url="?cmd=-qpData&req="+locreq+"&grp=1&dname="+url;
            url+="&raw=1";
            document.location=url;
            return ;
        }

        var that = this;
        if(src =="http" ){
            this.showDataLoadingProgress(true);
            if( !this.backendCGI(url) ) {
                var rawFmt=( (url.indexOf("?")==-1) ? "": "&raw=1" );
                if( this.ajaxObj && !this.alowMultipleAjax && this.ajaxObj.readyState<4 ) {
                    this.ajaxObj.abort();
                }
                
                this.ajaxObj = ajaxDynaRequestPage(url+rawFmt, params, function(param, text, page_request){that.load_callback(param, text, page_request);}, this.isPost, null);
                return true;
            }
        } else if(src=="innerHTML")  {
            delete this.ajaxObj;
            var o=document.getElementById(url);
            var bod=o ? ""+o.innerHTML : "DEVELOPER alert innerHTML of "+url+" not found";
            this.load_callback(this ,bod);
            return true;
        }  else if(src=="innerText")  {
            delete this.ajaxObj;
            var o=document.getElementById(url);
            var bod=o ? ""+o.innerHTML : "DEVELOPER alert innerHTML of "+url+" not found";
            bod = textContent(bod);
            this.load_callback(this ,bod);
            return true;
        } else if(src=="elementValue")  {
            delete this.ajaxObj;
            var o=document.getElementsByName(url)[0];
            var bod=o ? ""+o.value : "DEVELOPER alert elementValue of "+url+" not found";
            this.load_callback(this ,bod);
            return true;
        } else if(src=='static') {
            delete this.ajaxObj;
            this.load_callback( this,url);
            return true;
        } else if(src=="qp_data") {
            this.showDataLoadingProgress(true);
            if( this.ajaxObj && !this.alowMultipleAjax && this.ajaxObj.readyState<4 ) {
                this.ajaxObj.abort();
            }
            this.ajaxObj =  ajaxDynaRequestPage("?cmd=-qpData&req="+locreq+"&raw=1&grp=1&dname="+url, this, function(param, text, page_request){that.load_callback(param, text, page_request);} , this.isPost );
            return true;
        } else if(src.indexOf("qpbg_")==0 ){
            var dataorigin="", dataname="", datasaveas="";
            var isArchive = docLocValue("arch", 0, url);
            spl=url.split("
            if(spl.length>1){
                var dataspec=spl[0].split(":");
                dataorigin=dataspec[0];
                if (dataspec.length > 1 && isok(dataspec[1])) {
                    locreq = dataspec[1];
                }
                if (dataspec.length > 2 && isok(dataspec[2])) {
                    dataname = dataspec[2];
                }
                if (dataspec.length > 3 && isok(dataspec[3])) {
                    datasaveas = dataspec[3];
                }
                url=spl.slice(-1);
            }
            svcname=src.substring("qpbg_".length);
            if( svcname===undefined || !svcname) {
                console.log("No svc specified for qpbg_svc submission");
                return false;
            }
            switch(svcname){
            case "http":
                if(params) {
                    if(params.dataname)dataname=params.dataname;
                    if(params.saveas)datasaveas=params.saveas;
                }
                var cgi_output = docLocValue("cgi_dstname", 0, url);

                if( isok(dataname ) && !isok(cgi_output) )
                    url = urlExchangeParameter(url, "cgi_dstname", dataname);
                else if( !isok(dataname ) )
                    dataname = docLocValue("cgi_dstname", "cgi_output", url);

                if (isArchive) {
                    dataname = "arch_" + dataname;
                }
                url = urlExchangeParameter(url, "backend", '1');
                url = urlExchangeParameter(url, "check", '1');
                break;
            case "tblqryx4":
                url="-qpRawSubmit&check=1&svc="+src.substring(5)+"&oper=list&"+url;
                if (isok(dataorigin))
                    url = urlExchangeParameter(url, "tbl", dataorigin);
                if (isok(locreq))
                    url = urlExchangeParameter(url, "dataGrpID", locreq);
                if (!isok(dataname))
                    dataname = "_.csv";
                if (!isok(datasaveas))
                    datasaveas = isok(dataorigin) ? "Tbl-" + dataorigin : "Tbl.csv";
                break;
            default :
                url="-qpRawSubmit&check=1&svc="+svcname+"&oper=list&tbl="+dataorigin+"&dataGrpID="+locreq+"&"+url;
            break;
            }
            
            this.showDataLoadingProgress(true);

            if (!isok(dataname))
                dataname = dataorigin;
            var callbackAjax = {objCls: this.objCls, callback: "load_callback", callback_submitted: "qpbg_submit_callback"};
            if( this.title && !params.title ) {
                params.title = this.title;
            }
            for (var i in params) {
                callbackAjax[i] = params[i];
            }
            this.qpbg_params = callbackAjax;
            if (params.loadmode == "download" || params.loadmode == "retrieveURL") {
                vjQP.backgroundRetrieveBlob(url, callbackAjax, dataname, locreq, 0, params.loadmode, datasaveas);
            } else {
                vjQP.backgroundRetrieveBlob(url, callbackAjax, dataname, locreq);
            }
            return true;
        }
        return false;
    };

    this.showDataLoadingProgress = function(showorhide)
    {
        if(showorhide) {
            vjDatasourceViewerDiv_show('auto','auto');
        }
        else
            vjDatasourceDiv_hide();
    };

    this.qpbg_submit_callback = function(unused, RQ)
    {
        if (!this.qpbg_params) {
            return;
        }
        this.qpbg_params.RQ = RQ;
        this.call_callbacks("qpbg_submitted", RQ);
    };

    this.load_callback = function (param, text, page_request)
    {
        if(page_request && page_request.status>300) {
            this.state='err';
            return;
        }
        var known_safe_url = false;
        if (this.dataurl.indexOf("static:
            this.dataurl.indexOf("http:
            this.dataurl.search(/http:\/\/js[^/]*\//) == 0 || this.dataurl.indexOf("http:
            this.dataurl.indexOf("http:
            this.known_safe_url
           )
        {
            known_safe_url = true;
        }
        this.original_data = null;
        if (!known_safe_url) {
            if (text.match(/<([^>]*)>/m)) {
                this.original_data = text;
                text = text.replace(/<([^>]*)>/gm, function(match, tag) {
                    if (tag.match(/^(?:\/?(?:b|code|definition|description|i|id|em|p|s|small|strong|sub|sup|title)|!--.*--)$/)) {
                        return match;
                    } else {
                        return "&lt;" + tag + "&gt;";
                    }
                });
            }
        }

        if (this.vjType != "vjDataSource") {
            return param.load_callback(param, text, page_request);
        }

        this.call_callbacks("arrived");

        this.data = isok(this.header) ? this.header + "\n" + text : text;
        this.page_request = page_request;
        this.state = 'done';
        this.timeDone = new Date();

        if (!isok(this.data) && !this.refreshOnEmpty) { this.state = 'err'; return; }

        if (known_safe_url && this.evaluator) {
            this.data = evalVars(this.data, "<!--", "
        }
        if (isok(this.parser)) {
            var parsed = typeof (this.parser) == 'function' 
                            ? this.parser(this, this.data)
                            : eval(this.parser)(this, this.data);
            this.data = parsed;
        }
        this.call_refresh_callbacks(param);
    };

    this.call_refresh_callbacks = function(params)
    {
        return this.call_callbacks("refreshed",params);
    };

    this.call_callbacks = function(type,params)
    {

        if (!type) { type = "refreshed" }
        if (this.div) {
            var o=gObject(this.div);if(o)o.innerHTML=this.data;
        }
        var call_length=this.callbacks[type]?this.callbacks[type].length:0;

        for ( var i=0 ; i< call_length ; ++i) {
            var func=this.callbacks[type][i].func;
            if( isok(func) || typeof(func) == "function" ) {
                if( typeof(func) != "function" )
                    func=eval(func);
                func.call( this.callbacks[type][i].obj, this.callbacks[type][i].param , this.data, this.page_request );
            }
            else if( isok(this.callbacks[type][i].obj) ){

                var whatToCall=this.callbacks[type][i].obj.dataLoaded;
                if(!whatToCall)whatToCall='render';

                if(this.callbacks[type][i].obj[whatToCall])
                    this.callbacks[type][i].obj[whatToCall]( this.callbacks[type][i].param , this.data, this.page_request);
            }
            else if (typeof(this.callbacks[type][i]) == "function") {
                this.callbacks[type][i].call(this, params, this.data, this.page_request);
            } else {
                eval(this.callbacks[type][i])( params, this.data, this.page_request );
            }
        }
        if(type=="arrived")delete this.callbacks[type];
    };

    this.setdata=function( text , autorefresh )
    {
        this.data=text;
        if(autorefresh ) this.call_refresh_callbacks();
    };

    this.register_callback=function (cbfun, type)
    {
        if (!type) type = "refreshed";
        if(!this.callbacks[type])this.callbacks[type]=new Array();
        for(var i=0; i<this.callbacks[type].length; i++)
            {if (this.callbacks[type][i] == cbfun) return ;}
        this.callbacks[type].push(cbfun);
    };

    this.unregister_callback=function (cbfun, uniqueId, type)
    {
        if (!type) type = "refreshed";
        if(this.callbacks[type]){
            for ( var ic=this.callbacks[type].length-1; ic>=0; --ic  ) {
                if(uniqueId && this.callbacks[type][ic].obj && this.callbacks[type][ic].obj.uniqueId==uniqueId ) {
                    this.callbacks[type].splice(ic,1);
                }else if( this.callbacks[type][ic] == cbfun){
                    this.callbacks[type].splice(ic,1);
                }
            }
        }
    };

    this.clear_callbacks = function (type)
    {
        if (!type) type = "refreshed";
        if (type === "*") {
            for (var i in this.callbacks)
                this.callbacks[i] = new Array();
            return;
        }
        this.callbacks[type] = new Array();
    };

    this.reload=function(url, force, loadmode, fileName)
    {
        this.state='';
        if(url) this.url=url;
        if(force) this.load(loadmode, fileName);
    };

    this.loadHTML=function(div)
    {
        this.div=div;
        this.load();
    };

    this.has_nontrivial_data=function()
    {
        if (this.state === "done") {
            if (isok(this.header)) {
                return this.data.length > this.header.length + 1;
            }
            return this.data && this.data.length > 0;
        }
        return false;
    };

    this.hasDataForUrl = function(url)
    {
        if (!url) {
            url = this.url;
        }
        return isok(this.data) && isok(this.dataurl) && this.dataurl==url && (this.state=='done' || this.state=='err');
    };

    this.registerDS = function(name)
    {
        if (!name) name = this.name;
        if (vjDS[name] == this)
            return;

        if (vjDS[name]) {
            vjObj.unregister(vjDS[name].objCls);
            vjObj.register(this.objCls, this);
        }

        vjDS[name] = this;
        return this;
    };

    this.escapeQueryLanguage = function(div_or_string)
    {
        if (!div_or_string){
            return "";
        }
            
        var text = textContent(div_or_string);
        const escapedText = getEscapedObjQry( text )
        return escapedText ? escapedText : "";
    };

    return this;
}









function vjDataSourceEngine()
{
    this.forceload=function (dsprefix)
    {
        for (var i=0 ; i< this.DataSources.length; ++i) {
            if( !isok(dsprefix) || this[i].name.indexOf(dsprefix)==0) {
                this[i].state='';
            }
        }
    };

    this.add=function( title, name, url, refresh_callbacks,  header )
    {
        if(this[name])  {
            this[name].url=url;
            if(refresh_callbacks)this[name].callbacks.refreshed=this[name].callbacks.refreshed.concat(refresh_callbacks);
            this[name].header=header;
            return this[name];
        }
        this[name]=new vjDataSource( { title:title, name:name,url:url,header:header} );
        this[name].callbacks.refreshed = verarr(refresh_callbacks);
        return this[name];
    };

    this.reload=function (dsname, url)
    {
        this[dsname].reload(url);
    };

    this.add( "infrastructure: Void" ,"dsVoid" , "static:

    this.escapeQueryLanguage = this["dsVoid"].escapeQueryLanguage;
}

var vjDS=new vjDataSourceEngine();

