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
//var debug=0;//'flow';

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Datasource
// _/
// _/_/_/_/_/_/_/_/_/_/_/

/** Data source class - synchronously or asynchronously loads data, preprocesses it,
 * and calls callbacks when the data has been retrieved.
 * @constructor
 * @param {{}} sourcebase Data source's prototype */
function vjDataSource( sourcebase )
{
    /** Human-readable title
      * @member {string} vjDataSource#title */
    /** Unique identifier by viewers to refer to the datasource
      * @member {string} vjDataSource#name */
    /** HIVE-format extended url for the data; can be http:// or qpbg_tblqryx4:// etc.
      * @member {string} vjDataSource#url */
    /** Header prepended to retrieved data
      * @member {string} vjDataSource#header */
    /** Callback for parsing retrieved data; not used if empty
      * @method
      * @param {vjDataSource} ds Data source
      * @param {string} text Retrieved data
      * @returns {string|null} parsed text, or null on failure */
    this.parser='';
    /** The data itself
      * @member {string} */
    this.data='';
    /** Complete response from the ajax layer
      * @member {XMLHttpRequest} */
    this.page_request='';
    this.callbacks={'refreshed': new Array()}; // {refreshed: this.refresh_callbacks}; // where refresh_callbacks is an array of {obj,func,param}  triples
    /** Whether automatic parsing of variables is turned on
      * @member {boolean} */
    this.evaluator=false;
    this.progressor='QP_bgProgress'; // the progress bar container during background data downloads
    this.dataurl=''; // the url for which we got the data already: check this in lazy mode
    this.state='';  // can be '', 'load' , 'done'
    this.div='';  // the div to set the content after loading
    this.timeLoad=null;
    this.timeDone=null;


    this.backendCGI = function(url)
    {
        return docLocValue("backend", 0, url);
    };

    this.clone=function (sourcebase)
    {
        if(!sourcebase)sourcebase=this;
        var i=0;
        // copy all the features from base objejct
        for (i in sourcebase ) {
            //if( typeof(viewbase[i]) == "Object" )
                //this[i] = viewbase[i];
            this[i] = sourcebase[i];
        }

    };
    this.clone(sourcebase);
    this.vjType="vjDataSource";
    this.objCls=this.vjType+"-"+this.name;
    //vjObj.register(this.vjType+"-"+this.name,this);
    vjObj.register(this.objCls,this);

    // loadmode: null/undefined (normal mode), "download", or "retrieveURL" (retrieve url of generated file)
    this.load=function( params )
    {
        if( typeof(params) === "undefined" ) {
            params = new Object();
        }
        else if( typeof(params) !== "object" ) {
            params = {'loadmode':params};
        }

        if(params.loadmode!='download') {

            if(debug=='flow')alert( "INQUIRY: "+this.name +"\n"+"url="+this.url +"\n"+"dataurl="+this.dataurl+ "\n"+this.state);

            // be lazy about loading the data
            // check that data exists and it is from the same source
            if(this.hasDataForUrl()) {
                if(debug=='flow')alert( "LAZY: "+this.name +"\n"+"url="+this.url + "\n"+this.state);
                return true;
            }
            if(this.state=='load' &&  this.dataurl==this.url ) // currently loading : do not attempt to get it twice
                return true;

            if(debug=='flow')alert( "LOAD: "+this.name +"\n"+"url="+this.url + "\n"+this.state);
            this.state='load';
            this.timeLoad=new Date();
            this.timeDone=null;
            this.dataurl=this.url; // associate the data with the url

            this.call_callbacks("fetching");
        }

        // split the source and the url
        var spl=this.url.split("://"), locreq=typeof req =="undefined"?"":req;
        var src=spl[0];

        var url=spl.slice(1).join("://");

        // now make the replacements
        url=evalVars(url,"$(",")"); // evaluate variables

        if(src=="file"){
            url = "-qpFile&raw=1&file=" + url; // for example after trying to download this after qapp ziping ";
        }

        // now the operation itself
        if( params.loadmode=='download' && src.indexOf("qpbg_")!=0 )  { // downlaods of direct links
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
                
                this.ajaxObj = ajaxDynaRequestPage(url+rawFmt, params, function(param, text, page_request){that.load_callback(param, text, page_request);}, this.isPost); // +"&raw="+vDS_raw
                return true;
            }
        } else if(src=="innerHTML")  { // if the data is associated with an innerHTML of some other layer
            delete this.ajaxObj;
            var o=document.getElementById(url);
            var bod=o ? ""+o.innerHTML : "DEVELOPER alert innerHTML of "+url+" not found";
//            bod=bod.replace(/\$\(DV\)/g,this.name).replace("<pre>","").replace("<PRE>","").replace("</pre>","").replace("</PRE>","");
            this.load_callback(this ,bod);
            return true;
        }  else if(src=="innerText")  { // if the data is associated with an innerHTML of some other layer
            delete this.ajaxObj;
            var o=document.getElementById(url);
            var bod=o ? ""+o.innerHTML : "DEVELOPER alert innerHTML of "+url+" not found";
            bod = textContent(bod);
            this.load_callback(this ,bod);
            return true;
        } else if(src=="elementValue")  { // if the data is associated with an innerHTML of some other layer
            delete this.ajaxObj;
            var o=document.getElementsByName(url)[0];
            var bod=o ? ""+o.value : "DEVELOPER alert elementValue of "+url+" not found";
            //bod=bod.replace(/\$\(DV\)/g,this.name).replace("<pre>","").replace("<PRE>","").replace("</pre>","").replace("</PRE>","");
            this.load_callback(this ,bod);
            return true;
        } else if(src=='static') {
            delete this.ajaxObj;
            this.load_callback( this,url);
            return true;
        } else if(src=="qp_data") { // if the data is assoociated with a datablob
            this.showDataLoadingProgress(true);
            if( this.ajaxObj && !this.alowMultipleAjax && this.ajaxObj.readyState<4 ) {
                this.ajaxObj.abort();
            }
            this.ajaxObj =  ajaxDynaRequestPage("?cmd=-qpData&req="+locreq+"&raw=1&grp=1&dname="+url, this, function(param, text, page_request){that.load_callback(param, text, page_request);} , this.isPost );
            return true;
        } else if(src.indexOf("qpbg_")==0 ){
            var dataorigin="", dataname="", datasaveas="";
            var isArchive = docLocValue("arch", 0, url);
        // for background retrievals there is a dataspec in front of the job
        // format: "qpbg_<svcname>://dataorig:req:dataname:datasaveas//<field>=<value>"
            spl=url.split("//");
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
                url=spl.slice(1).join("//");
            }
            svcname=src.substring("qpbg_".length);
            if( svcname===undefined || !svcname) {
                console.log("No svc specified for qpbg_svc submission");
                return false;
            }
            switch(svcname){
            case "http":
                /** format: "qpbg_http://dataorig:req:dataname:datasaveas//<field>=<value>"
                * if dataname is provided then is used as cgi_dstname. If cgi_dstname is provided then is used
                * as dataname. If arch=1 then dataname is overwritten by the hardcoded "arch_<cgi_dstname>.txt"
                * that has the reqID of the archiver and the cgi_dstname is the name of the file passed to
                * the archiver.*/
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
            params.title = this.title;
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
        if (this.dataurl.indexOf("static://") == 0 || this.dataurl.indexOf("innerText://") == 0 || this.dataurl.indexOf("innerHTML://") == 0 || this.dataurl.indexOf("http://help/") == 0 ||
            this.dataurl.indexOf("http://js/") == 0 || this.dataurl.search(/http:\/\/js-obj[^/]*\//) == 0 || this.dataurl.indexOf("http://js-graph/") == 0 || this.dataurl.indexOf("http://d3js/") == 0 ||
            this.dataurl.indexOf("http://?cmd=propspec") == 0)
        {
            known_safe_url = true;
        }
        if (!known_safe_url) {
            // remove potentially unsafe html tags
            text = text.replace(/<([^>]*)>/gm, function(match, tag) {
                if (tag.match(/^(?:\/?(?:b|code|definition|description|i|id|em|p|s|small|strong|sub|sup|title)|!--.*--)$/)) {
                    return match;
                } else {
                    return "&lt;" + tag + "&gt;";
                }
            });
        }
        //if(this.constructor.toString().indexOf("vjDataSource")==-1 )// in case if we are called from ajax callbacks where this is a Window object
        if (this.vjType != "vjDataSource")// || this.vjType!="vjSVG_Series")
            return param.load_callback(param, text, page_request);

        this.call_callbacks("arrived");

        //  remember the data
        if (isok(this.header)) this.data = this.header + "\n" + text;
        else this.data = text;
        this.page_request = page_request;
        this.state = 'done';
        this.timeDone=new Date();

        if (debug == 'flow') alert("FiNISHED: " + this.name + "\n" + "url=" + this.url + "\n" + this.state);
        if (!isok(this.data) && !this.refreshOnEmpty) { this.state = 'err'; return; }

        if (known_safe_url && this.evaluator) { // evaluate variables which are included as comments
            this.data = evalVars(this.data, "<!--", "//-->");
            //alert(this.data);
        }
        if (isok(this.parser)) { // if parser is specified ... do parse
            var parsed;
            if (typeof (this.parser) == 'function') parsed = this.parser(this, this.data);
            else parsed = eval(this.parser)(this, this.data);
//            if (!parsed) return;
            this.data = parsed;
        }

        // refresh callbacks for this

//        this.showDataLoadingProgress(true);


        //objCont(this);
        this.call_refresh_callbacks(param);

    };

    this.call_refresh_callbacks = function(params)
    {
        return this.call_callbacks("refreshed",params);
    };

    this.call_callbacks = function(type,params)
    {

        if (!type) type = "refreshed";
        if(this.div){
            var o=gObject(this.div);if(o)o.innerHTML=this.data;
        }
        var call_length=this.callbacks[type]?this.callbacks[type].length:0;
        if(debug=='flow')alert( "CALLBACING: "+this.name +"\n"+"url="+this.url + "\n"+this.state+"\n\n\n"+jjj(this.callbacks[type]));
        for ( var i=0 ; i< call_length ; ++i) {
            var func=this.callbacks[type][i].func;
            if( isok(func) || typeof(func) == "function" ) { // if a callback function is specieid call it
                //if( !(func && instanceof function) )
                if( typeof(func) != "function" )
                    func=eval(func);
                func.call( this.callbacks[type][i].obj, this.callbacks[type][i].param , this.data, this.page_request );
            }
            else if( isok(this.callbacks[type][i].obj) ){ // call the objects function

                var whatToCall=this.callbacks[type][i].obj.dataLoaded;
                if(!whatToCall)whatToCall='render';

                if(this.callbacks[type][i].obj[whatToCall])
                    this.callbacks[type][i].obj[whatToCall]( this.callbacks[type][i].param , this.data, this.page_request);
            }
            // if no object is specified the whole callback is a function
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
        //var ds=this.dataSourceEngine[this.data];
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

    this.reload=function(url, force, loadmode)
    {
        this.state='';
        if(url) this.url=url;
        if(force) this.load(loadmode);
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
                // if the state is done, then call_callbacks was called, and it
                // would have prepended header + "\n" to data
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
            vjObj.register(this.objCls, this); // because maybe old vjDS[name].objCls == this.objCls
        }

        vjDS[name] = this;
        return this;
    };

    this.escapeQueryLanguage = function(div_or_string)
    {
        if (!div_or_string)
            return "";

        var text = textContent(div_or_string);
        text = text.replace(/%/mg, "%25");
        text = text.replace(/\r?\n/mg, "%0A");
        text = text.replace(/ /mg, "%20");
        text = text.replace(/[\r\n]/mg, "");
        text = text.replace(/#/mg, "%23");
        text = text.replace(/\+/mg, "%2B");
        text = text.replace(/=/mg, "%3D");
        text = text.replace(/&/mg, "%26");
        text = text.replace(/#/mg, "%23");
        return text;
    };

    return this;
}







// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ DataSourceEngine
// _/
// _/_/_/_/_/_/_/_/_/_/_/


function vjDataSourceEngine()
{
    this.forceload=function (dsprefix)
    {
        for (var i=0 ; i< this.DataSources.length; ++i) {
            if( !isok(dsprefix) || this[i].name.indexOf(dsprefix)==0) {
                //this.DataSources[i].state='';
                this[i].state='';
            }
        }
    };

    this.add=function( title, name, url, refresh_callbacks,  header )
    {
        if(this[name])  { // if exists
            //this[name].title=title;
            this[name].url=url;
            if(refresh_callbacks)this[name].callbacks.refreshed=this[name].callbacks.refreshed.concat(refresh_callbacks);
            this[name].header=header;
            return this[name];
        }
        this[name]=new vjDataSource( { title:title, name:name,url:url,header:header} ); // this makes sure element is refferencable by its name in array
        this[name].callbacks.refreshed = verarr(refresh_callbacks);
        return this[name];
    };

    this.reload=function (dsname, url)
    {
        this[dsname].reload(url);
    };

    this.add( "infrastructure: Void" ,"dsVoid" , "static:// "); // Initialize datasource

    this.escapeQueryLanguage = this["dsVoid"].escapeQueryLanguage;
}

var vjDS=new vjDataSourceEngine();

//# sourceURL = getBaseUrl() + "/js/vjDataSource.js"