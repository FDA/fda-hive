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

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Dataview Viewer
// _/
// _/_/_/_/_/_/_/_/_/_/_/
var debug=0;

function vjDataViewViewer( viewbase )
{
    // allow distinguishing constructed vjDataViewViewers from parameter objects
    this.isDataViewViewer = true;
    //this.objectsIDependOn=verarr(this.objectsIDependOn);
    //this.objectsDependOnMe=verarr(this.objectsDependOnMe);
    if( typeof(this.formObject) == "string" )
        this.formObject=document.forms[this.formObject];
        
    this.clone=function (viewbase)
    {
        if(!viewbase)viewbase=this;
        var i=0;
        // copy all the features from base objejct
        for ( i in viewbase ) {
            //if( typeof(viewbase[i]) == "Object" )
                //this[i] = viewbase[i];
            this[i] = viewbase[i];
        }

    };
    this.clone(viewbase);
    this.uniqueId=Math.random();
    //this.objCls="cls_"+this.container+"_"+parseInt(1000*Math.random());
    // replace non-alphanumerics with underscore to ensure objCls can be used as a variable name in evals
    if(this.container)this.objCls=this.container.replace(/\W/g, "_");
    else this.objCls=("cls"+this.uniqueId).replace(".","");
    vjObj.register(this.objCls,this); // needed to find this object in events by object container
    // _/_/_/_/_/_/_/_/_/_/_/
    // Renderer

    this.unlink=function(){
        this.unregister_callback();
        vjObj.unregister(this.objCls);
    };

//  this.remove=function(){
//      this.unlink();
//  };

    this.render=function( param, text, page_request)
    {
        //alerJ(text,this)
        var dataDone=0;
        this.data=verarr(this.data);
        // count how many datasource are in by now
        for(var i=0; i<this.data.length; i++) {
            var ds=this.dataSourceEngine[this.data[i]];
            if(ds && ds.state=='done')++dataDone;
        }
        if(!this.allowPartialLoad && dataDone<this.data.length) // not all data sources are loaded yet
            return;
        if(this.isVisible() || this.prohibitReRender )
            this._rendered=true;
        else{
            this._rendered=false;
        }
        if(!text && this.data[0])
            text=this.dataSourceEngine[this.data[0]].data;
        if (!text)
            text = "";

        if(!this.composerFunction)
            this.composerFunction=this.dataViewEngine.registeredRenderers[this.composer];

        if(debug=='flow')alert(" RENDERING " + this.container );

        var o=document.getElementById(this.container+"_cell");
        if(o) {
            if(this.hidden) o.className="cellHid";
            else o.className="cellVis";
        }

        if(this.callbackDataViewerParser){
            text=funcLink(this.callbackDataViewerParser,this,text,page_request);
        }

        this.div=document.getElementById(this.container);
        if(this.callbackDataReady) {
            //alert(text)
            var rescontinue=funcLink(this.callbackDataReady,this,text,page_request);
            if(rescontinue)return ;
        }

        if(!this.nodiv ){
            if(!this.div) {
                console.error("Div does not exist !!!");
                return ;
            }
        }

        // now deal with registered viewers
        if(this.hideOnEmptyData) {
            if(!text || !text.length)
                this.div.className="sectHid";
            else
                this.div.className="sectVis";
        }
        if(this.composerFunction && text.indexOf("preview:")!=0 ) {
            this.composerFunction.call(this,this,text,page_request);
            //return;
        }
        else if( this.composer=='preview' || text.indexOf("preview")==0 ) { // unformtatted blobs
            var t="";
            if(this.prefixHTML)t+=this.prefixHTML;
            if(text.indexOf("preview")==0)t+="<pre>"+text.substring(8)+"</pre>";
            else t+="<pre>"+text+"</pre>";
            if(this.appendHTML)t+=this.appendHTML;
            this.div.innerHTML=t;
            //return ;
        }
        else {
            this.div.innerHTML=text;
            //return ;
        }

        if(this.hideTabOnEmpty){
            if((!text || !text.length || this.noContent) && this.tab && this.tab.container){
                var otab=gObject(this.tab.container);
                this.tab.invisible=true;
                this.tab.hidden=true;
            }
            else{
                var otab=gObject(this.tab.container);
                this.tab.invisible=false;
                this.tab.hidden=false;
            }
            this.tab.parent.render(true);
        }

        if(this.div && this.div.style.overflow=='hidden')this.div.style.overflow='auto';
//        this.emptySources=true;
//        for(var iD=0; iD < this.getData.length ; ++iD){
//            if(this.getData(iD).data.length > 50)
//                this.emptySources=false;
//        }
//
//        if(this.emptySources){
//            this.div.innerHTML="";
//        }

        if(this.operationInit)
            eval(this.operationInit);

        if(this.callbackRendered) {
            funcLink(this.callbackRendered,this,text,page_request);
        }

        this.updateDependents();

        if(this.isVisible() || this.prohibitReRender )
            this._rendered=true;
        else{
            this._rendered=false;
        }
    };

    this.updateDependents=function( )
    {
        if(!this.objectsDependOnMe)return ;

        for( var im=0; im < this.objectsDependOnMe.length ; ++im) {
            if(typeof ( this.objectsDependOnMe[im] ) =='string' )
                this.objectsDependOnMe[im]=this.dataViewEngine.locate(this.objectsDependOnMe[im]);
            var depObj=this.objectsDependOnMe[im];
            //alerJ("ddddd",depObj);

            // make sure I am on the list of dependent objects
            depObj.objectsIDependOn=verarr(depObj.objectsIDependOn);
            for( var id=0; id < depObj.objectsIDependOn.length && depObj.objectsIDependOn[id].uniqueId!=this.uniqueId ; ++id)
                ;
            depObj.objectsIDependOn[id]=this;

            // update dependent objects
            if(depObj.composerFunction)
                depObj.composerFunction(depObj,depObj.getData(0).data);
            //alerJ('here',depObj)
        }
    };

    // _/_/_/_/_/_/_/_/_/_/_/
    // Loader

    this.load=function( loadmode )
    {
        //if(!isok(this.data)) return ; // no associated data source: can do nothing
        this.data=verarr(this.data);
        if(!this.data.length)return;
        var dataHere=0, ds ;

        for(var id=0; id<this.data.length; ++id) {

            ds=this.dataSourceEngine[this.data[id]];
            if( !ds) {
                alert("DEVELOPER ALERT: data source '"+this.data[id]+"' does not exist");
                return ;
            }

            if( loadmode=='download'){
                ds.load(loadmode ); // refresher will be called from data handler
                //return ;
                continue;
            }
            if(debug=='flow')alert("VIEWLOAD ANALIZING "+vDS[this.data[id]].url +"\n\n"+ vDS[this.data[id]].dataurl +"\n\n"+ vDS[this.data[id]].state) ;
            if( (ds.url && ds.url!=ds.dataurl ) || (ds.state!='done' && ds.state!='err') ) { // datasource is associated but data is not here
                if(debug=='flow')alert("VIEWLOAD attempts to get the data " +  ds.data ) ;

                ds.load(loadmode ); // refresher will be called from data handler
                //return ;
                continue;
            }
            else if( (ds.state=='done' || ds.state=='err') && ( (loadmode=='rerender' && !this.preventRerender) || (!this._rendered && this.tabvisible)) && ds.url)
            {
                if(this.selfReload !==undefined)this.selfReload=true;
                if(!this.prohibitReRender || !this._rendered)
                    this.render(this,ds.data);
            }
            if(debug=='flow')alert("VIEWLOAD had the data from " + this.data[id] ) ;
            ++dataHere;
        }
      //  if(dataHere!=this.data.length)return; // not all data are here , should wait until data are here

      //  this.render(this.param, ds.data ) ;
    };

    this.viewer_refresh=function(datasource_names){
        var dsNames=verarr(datasource_names);
        var all_ds_here=true;
        for(var d=0; d<dsNames.length ; ++d){
            if(!this.getData(dsNames[d])){
                all_ds_here=false;
                this.register_callback(dsNames[d]);
                this.load();
                break;
            }
        }
        if(all_ds_here){
            for(var d=0; d<dsNames.length ; ++d){
                this.render(this,this.dataSourceEngine[dsNames[d]].data);
            }
        }
    };

    this.getData=function(whichData)
    {
        var iData=parseInt(whichData);
        for(var i=0; i<this.data.length; i++) {
            if( isNaN(iData) ) {if( this.data[i]!=whichData) continue;}
            else { if( i!=iData) continue;}
            var ds=vjDS[this.data[i]];
            //if(ds && ds.state=='done')return ds;//.data;
            return ds;
        }
        return ;
    };

    this.getDataSessionID = function(whichData) {
        var iData = whichData ? parseInt(whichData) : 0;
        this.data = verarr(this.data);
        if( !this.data[iData] )
            return ;
        if( !this.dataSourceEngine )
            return;
        if( !this.dataSourceEngine[this.data[iData]] ){
            return;
        }
        if( !this.dataSourceEngine[this.data[iData]].ajaxObj )
            return;
        return this.dataSourceEngine[this.data[iData]].ajaxObj.sessionID;
    };

    this.waitOnLoadCallback=function(a,b,c,d)
    {
        for(var i=0; i<this.data.length; i++)
            this.dataSourceEngine[this.data[i]].refreshOnEmpty=true;
        this.div=this.container?gObject(this.container):null;
        if(!this.div)return ;
        var top=this.div.scrollTop;
        if( !this.doNotShowRefreshIcon) {
            if(this.div.style.overflow=='auto')this.div.style.overflow='hidden';
            this.div.innerHTML ="<div id='"+this.container+"_loading' style='position:static;width:100%;height:100%'><div style='top:"+top+"px' class='progressing'>&nbsp;</div></div>"+this.div.innerHTML;//styles='background-image:url(\"img/progress.gif\");' width='16'></img>";
        }
//        this.div_loading=gObject(this.container+"_loading");
    };
    // _/_/_/_/_/_/_/_/_/_/_/
    // Hooks

    this.register_callback=function(dsname, param, current_only) //current_only is a flag in order to register only the datasources provided from dsname argument
    {
        this.data=verarr(this.data);
        if(this.data) {
            var dsnamearr=verarr(dsname);
            for (var is=0 ,i ; is<dsnamearr.length; ++is ) {
                for(i=0; i<this.data.length; i++)
                    {if (this.data[i] == dsnamearr[is]) break;}
                if(i==this.data.length)
                    this.data.push(dsnamearr[is]);
            }
        }
        var dataArr=this.data;
        if(current_only)
            dataArr=verarr(dsname);
        for ( var is=0; is<dataArr.length ; ++is){
            if(!this.dataSourceEngine[dataArr[is]])
                alert("DEVELOPER ALERT: datasource "+dataArr[is]+ " is not found");
            this.dataSourceEngine[dataArr[is]].register_callback( { 'obj' : this, 'param' : param } ); // register ourselves a callback for a data updater
            if(!this.isNrefreshOnLoad)
                this.dataSourceEngine[dataArr[is]].register_callback( { 'obj' : this, 'param' : param, 'func' : this.waitOnLoadCallback} , "fetching");
        }
    };

    this.unregister_callback=function( )
    {
        this.data=verarr(this.data);

        for(var i=0; i<this.data.length; i++) {
            this.dataSourceEngine[this.data[i]].unregister_callback( 0, this.uniqueId);
            if(!this.isNrefreshOnLoad)
                this.dataSourceEngine[this.data[i]].unregister_callback(0,this.uniqueId,"fetching");
        }
        return this.dataSourceEngine[this.data[i]];
    };


    // _/_/_/_/_/_/_/_/_/_/_/
    // data types

    this.formDataValue = function (realText, type, node, col, options) {
        // options: { ellipsize: true, textonly: false }

        if(!realText)
            return "";
        realText=""+realText;
        if (realText.indexOf("eval:") == 0)
            realText = eval(realText.substring(5));
        else if (this.allowVariableReplacement)
            realText = evalVars(realText, "$(", ")", node);
        //        alert("realText OUT= " + realText);

        var ellipsize = !options || options.ellipsize;
        var textonly = options && options.textonly;

        var maxlen = node ? node.maxTxtLen : 0;
        if (!maxlen && col) maxlen = col.maxTxtLen;
        if (!maxlen) maxlen = this.maxTxtLen;
        if (maxlen && realText.length > maxlen && ellipsize) { //  && realText.indexOf(">") == -1 && realText.indexOf("<") == -1
            var lastIndexToKeep = (this.maxTextLenCutExtensionAfterSymbols) ? realText.lastIndexOf(this.maxTextLenCutExtensionAfterSymbols) : realText.length;
            if(lastIndexToKeep==-1)lastIndexToKeep=realText.length;
            realText=realText.substr(0,lastIndexToKeep);
            if( realText.length > maxlen+3 ) {
                realText = realText.substr( 0, maxlen/2) + "..." + realText.substr(realText.length-maxlen/2);
            }
        }

        if (type == "treenode" && node.treenode && !textonly) {
            realText = this.formatTreenode (realText, node, col);
        }
        if(type=="reqstatus") {
            var stat=parseInt(realText);
            realText = this.formatRequestStatus(stat, node, col, textonly);
        }
        if (type == "id-specific") {
            //var tmplt=this.template;
            realText=evalVars(col.template, "$(", ")",node);
            //realText= "<img src='?cmd=objFile&ids="+realText+"&raw=1' ";
            //if(this.iconWidth){
            //    realText = " width='" + this.iconWidth + "' ";
            //}
            //realText+="/>";
        }
        if (type == "icon" && !textonly) {
            icon = realText;
            var srcset = options && options.srcset ? options.srcset : null;
            if (col && !isok(icon)) icon = col.icon;
            if (!isok(icon)) icon = this.defaultIcon;
            var iconSize=this.iconSize;
            if(isok(node)){if(isok(node.iconSize))iconSize=node.iconSize;}
            if(isok(col)){if(isok(col.iconSize))iconSize=col.iconSize;}
            var widthTxt = "";
            if(this.iconWidth){
                widthTxt = " width='" + this.iconWidth + "' ";
            }
            if (srcset && srcset.length) {
                realText = "<img src='" + makeImgSrc(icon, srcset[0]) + "'";
                var srcsetText = "";
                for (var is=1; is<srcset.length; is++) {
                    if (srcsetText.length) {
                        srcsetText += ", ";
                    }
                    srcsetText += makeImgSrc(icon, srcset[is]) + " " + srcset[is]/srcset[0] + "x";
                }
                if (srcsetText.length) {
                    realText += " srcset='" + srcsetText + "'";
                }
            } else {
                realText = "<img src='" + makeImgSrc(icon) + "'";
            }
            realText += " height=" + iconSize + widthTxt+ " border=0 />";
        }
        else if (type == "percent" || type == "percent*100") {
            var prc = (type == "percent*100") ? parseInt(parseFloat(realText) * 100) : parseInt(realText);

            realText = this.formatPercent(prc, node, col, textonly);
        }
        else if (type == "graphEigen" || type == "graphEigen*100") {
             var prc = parseFloat(realText);
            if (type=="graphEigen*100")
                realText = this.formatGraphEigenVect(prc, node, col, textonly, true);
            else
                realText = this.formatGraphEigenVect(prc, node, col, textonly, false);
        }
        else if (type == "timespan") {
            realText = this.formatTimespan( parseInt(realText), node, col);
        }
        else if (type == "largenumber") {
            realText += '';
            var v = realText.split('.');
            var v1 = v[0];
            var v2 = v.length > 1 ? '.' + v[1] : '';
            var rgx = /(\d+)(\d{3})/;
            while (rgx.test(v1)) {
                v1 = v1.replace(rgx, '$1' + ',' + '$2');
            }
            realText = v1 + v2;
        }
        else if (type == "largesci") {
            var vsize = parseInt(realText);

            realText = this.formatLargeSci(vsize, node, col, textonly);
        }
        else if (type == "bytes") {
            var vsize = parseInt(realText);
            var sizeUnit = new Array("", "KB", "MB", "GB", "TB", "PB", "EB"), leftover = 0;
            for (var unit = 0; vsize > 1024; ++unit) {
                leftover = vsize % 1024;
                vsize = parseInt(vsize / 1024);
            }
            realText = parseInt(vsize);
            if (leftover) leftover = Math.round(100 * ((1024 + leftover) / 1024));
            if (leftover) realText += "." + ("" + leftover).substring(1);

            realText += "&nbsp;" + sizeUnit[unit];
        }
        else if (type == "datetime") {
            var dt_abs,dt_fcDate,dt_fcTime;
            if(options && options.datetime) {
                dt_abs = options.datetime.absoluteOnly ? options.datetime.absoluteOnly : null;
                dt_fcDate = options.datetime.forceDate ? options.datetime.forceDate : null;
                dt_fcTime = options.datetime.forceTime ? options.datetime.forceTime : null;
            }
            realText = formaDatetime(realText, dt_abs, dt_fcDate, dt_fcTime);
        }
        else if (type == "pre" && !textonly) {
            realText = "<span class='code'>" + realText + "</span>";
        }
        else if (type == "alignment" && !col.isNmatchCol && !textonly) {
            realText = this.formatAlignment(realText, node, col);
        }
        else if (type == "choice") {
            var vvv = parseInt(realText);
            if (!isNaN(vvv) && col && vvv < col.choice.length) {
                realText = col.choice[vvv];
            } else realText = "";
        }
        else if (type == "qp_progress" || type == "qp_progress*100") {
            realText = realText.split("|");
            realText[0] = (type == "qp_progress*100") ? parseInt(parseFloat(realText[0]) * 100) : parseInt(realText[0]);
            realText = this.formatRequestQPProgress(realText, node, col, textonly);

        }
        return realText;
    };

    this.formatPercent = function( prc, node, col, textonly) {
        if (textonly) {
            if (!prc) {
                prc = 0;
            }
            return "" + prc + "%";
        }
        var txt = "<table width='"+ ( (col && col.width) ? col.width : "100%")+"' class='HIVE_table' ><tr>";
        if (prc > 0) txt += "<td class='ProgressBgColor'  width='" + prc + "%' >" + (prc >= 50 ? prc + "&nbsp;%" : "") + "</td>";
        if (prc < 100 && prc > 0 ) txt += "<td bgcolor='white' width='" + (100 - prc) + "%'>" + (prc < 50 ? prc + "&nbsp;%" : "") + "</td>";
        if (prc < 0) {//   {[rot_bar]||||||prc[barsize]||||}
            var barsize = 40;
            var t = new Date();
            prc = Int((Int(t.getTime()/10))%100);
            var rot_bar = (barsize - (100 - prc) );
            if(rot_bar<0)rot_bar = 0;
            if( rot_bar > 0 ) {
                txt += "<td class='ProgressBgColor' width='" + rot_bar + "%'>&nbsp;</td>";
            }
            if(prc > 0) {
                txt += "<td bgcolor='white' width='" + (prc-rot_bar) + "%'>&nbsp;</td>";
            }
            if(prc + barsize > 100 ) {
                txt += "<td class='ProgressBgColor' >&nbsp;</td>";
            }
            else {
                txt += "<td class='ProgressBgColor' width='" + ( barsize ) + "%'>&nbsp;</td>";
                txt += "<td bgcolor='white' >&nbsp;</td>";
            }
        }
        txt += "</tr></table>";
        return txt;
    };
    
    this.formatGraphEigenVect = function( prc, node, col, textonly, multiply) {
        if (textonly) {
            if (!prc) {
                prc = 0;
            }
            
            if (multiply)
                return "" + prc*100 + "%";
            
            return prc+"%";
        }
        var toUse=prc;
        if (!multiply)
            toUse=prc*100;
        
        if (multiply)
            prc=prc*100;
        
        var txt = "<table width='"+ ( (col && col.width) ? col.width : "100%")+"'><tr align='"+(prc>0 ? 'right' : 'left')+"'>";
        if (prc > 0)
        {    
            txt += "<td bgcolor='white' width='50%' ></td>" +
                "<td class='posClassGradient' width='" + prc/2 + "%' align='right'></td>";
        }
        if (prc < 100 && prc > 0 ) 
        {
            txt += "<td width='10%' style=' color: #980035; '>&#x25BA;</td><td bgcoor='white' width='" + (50-prc/2-10) + "%'><small><small>" + ( toUse ) + "</small></small></td>";
        }
        if (prc < 0) 
        {
            var barsize = 40;
            var rot_bar = Math.abs(prc);
            txt += "<td bgcoor='white' width='" + (50 - rot_bar/2-10) + "%' align='right'><small><small>" + (toUse)+ "</small></small></td><td width='10%' style='color: #07434E;'>&#x25C4;</td>";
            txt += "<td class='negClassGradient' width='" + rot_bar/2 + "%' align='center'></td>";
            txt += "<td bgcolor='white' width='50%'></td>";
        }
        txt += "</tr></table>";
        return txt;
    };

    this.formatTimespan = function( tt, node, col) {
        var realText = "";
        if (tt > 24 * 3600) { 
            realText += parseInt(tt / (24 * 3600)) + "d "; 
            tt = parseInt(tt % (24 * 3600)); 
        }
        if( tt > 3600 ){ realText += (parseInt(tt / (3600)) + "") + "h "; tt = parseInt(tt % (3600)); }
        if( tt > 60 ) { realText += (parseInt(tt / (60)) + "") + "m "; tt = parseInt(tt % (60)); }
        if(!realText || tt>0){ realText += (parseInt(tt) + "s") + ""; }
        return realText;
    };

    this.formatRequestStatus = function( stat, node, col, textonly ) {
        var statList = new Array("Any", "Queued", "Processing", "Running",
                "Suspended", "Done", "Killed", "Program Error", "System Error",
                "Unknown");
        var realText;
        if (textonly) {
            realText = statList[stat];
        } else {
            var stat_icon = stat?("img/reqstatus"+stat+".gif"):"img/reqQuestion.gif";
            realText="<img src='"+stat_icon+"' width=16 border=0 />&nbsp;<small>"+statList[stat]+"</small>";
        }
        if(node.orderExecute && node.stat<3) {
            if (textonly) {
                realText += "&nbsp;#" + node.orderExecute;
            } else {
                realText+="&nbsp;<small>#"+node.orderExecute+"</small>";
            }
        }
        return realText;
    };

    this.formatRequestQPProgress = function( txt, node, col, textonly ) {

        var statList = new Array("Any", "Queued", "Processing", "Running",
                "Suspended", "Done", "Killed", "Program Error", "System Error",
                "Unknown");
        var stat = parseInt(txt[1]);
        var prc = parseInt(txt[0]);
        if( stat < 3 && prc==0 ) {
            realText = this.formatRequestStatus(stat, node, col, textonly);
        } else if (textonly) {
            realText = statList[stat];
        } else {
            var col_bar;
            var tooltip_txt = statList[stat];
            switch (stat) {
                case 3:
                    col_bar = '#90caf9';
                    break;
                case 4:
                    col_bar = '#90caf9';
                    break;
                case 5:
                    col_bar = '#90caf9';
                    break;
                case 6:
                case 7:
                case 8:
                    col_bar = '#90caf9';
                    break;
                default:
                    col_bar = '#90caf9';
                    break;
            }
            realText = "<table width='"+ ( (col && col.width) ? col.width : "100%")+"' class='HIVE_table' title='"+sanitizeElementAttr(tooltip_txt)+"'><tr>";
            if (prc >= 0) {
                realText += "<td bgcolor='"+col_bar+"' width='" + prc + "%' >" + (prc >= 50 ? tooltip_txt + " "+ prc + "&nbsp;%" : "") + "</td>";
            }
            if (prc < 100 && prc >= 0 ) {
                realText += "<td bgcoor='white' width='" + (100 - prc) + "%'>" + (prc < 50 ? prc + "&nbsp;% "+tooltip_txt : "") + "</td>";
            }
            if (prc < 0 ) {//   {[rot_bar]||||||prc[barsize]||||}
                var barsize = 40;
                var t = new Date();
                prc = Int((Int(t.getTime()/10))%100);
                var rot_bar = (barsize - (100 - prc) );
                if(rot_bar<0)rot_bar = 0;
                if( rot_bar > 0 ) {
                    realText += "<td bgcolor='"+col_bar+"' width='" + rot_bar + "%'>&nbsp;</td>";
                }
                if(prc > 0) {
                    realText += "<td bgcolor='white' width='" + (prc-rot_bar) + "%'>&nbsp;</td>";
                }
                if(prc + barsize > 100 ) {
                    realText += "<td bgcolor='"+col_bar+"' >&nbsp;</td>";
                }
                else {
                    realText += "<td bgcolor='"+col_bar+"' width='" + ( barsize ) + "%'>&nbsp;</td>";
                    realText += "<td bgcolor='white' >&nbsp;</td>";
                }
            }
            realText += "</tr></table>";
        }
        return realText;
    };

    this.formatAlignment = function (realText, node, col){
        if(this.idd=='aa')
            alert("node.PosHigh="+node.PosHigh);
        var ip=parseInt(node.PosHigh);//,is=parseInt(node.Start);
        var extS="<span style='background-color:" + (this.matchColor ? this.matchColor : 'red') + ";'>";
        var extE="</span>";
        var realVarTextIndex=realText.search(/\S/);
        var blancText="";
        if (node.matchSubstring){
            node.matchSubstring.end=parseInt(node.matchSubstring.end)-(blancText.length);
            node.matchSubstring.start=parseInt(node.matchSubstring.start)-(blancText.length);
            if(node.matchSubstring.end >= node.matchSubstring.start ){
                realText =
                    realText.substring(0, node.matchSubstring.start - 1) +extS +
                    realText.substring(node.matchSubstring.start - 1, node.matchSubstring.end) + extE +
                    realText.substring(node.matchSubstring.end);
            }
        }
        if(node && !isNaN(ip) && !isNaN(ip) ){
            ip-=1;
            var ext2S="<span style='color:navy;font-weight:900;background-color:rgb(255,255,0);'>";
            var ext2E="</span>";
            if(node.matchSubstring && node.matchSubstring.end >= node.matchSubstring.start){
                if(node.matchSubstring.start-1<=ip){
                        ip+=extS.length;//+extE.length;//+node.matchSubstring.end-node.matchSubstring.start+1;
                }
                if(node.matchSubstring.end-1<parseInt(node.PosHigh)-1)
                    ip+=extE.length;
            }
            realText=realText.substring(0,ip)+ext2S+realText.substring(ip,ip+1)+ext2E+realText.substring(ip+1);
        }

        realText = "<span class='code'>" +blancText+ realText + "</span>";
        return realText;
    };

    this.formatLargeSci = function (vsize, node, col, textonly) {
        var sizeUnit = new Array("", "K", "&middot;10<sup>6</sup>", "&middot;10<sup>9</sup>", "&middot;10<sup>12</sup>", "&middot;10<sup>15</sup>", "&middot;10<sup>18</sup>"), leftover = 0;
        for (var unit = 0; vsize > 1000; ++unit) {
            leftover = vsize % 1000;
            vsize = parseInt(vsize / 1000);
        }
        var realText = "" + parseInt(vsize);
        if (leftover) leftover = Math.floor(100 * ((1000 + leftover) / 1000));
        if (leftover) realText += "." + ("" + leftover).substring(1);
        realText += "" + sizeUnit[unit];
        if (textonly) {
            realText = realText.replace(new RegExp("<sup>(.*)</sup>"), "^$1");
        }
        return realText;
    };

    this.formatTreenode = function ( realText, node, col) {
        var tt="";
        var tn=node.treenode;
        var icoSize=16;
        var white="<img src='img/tree-white.gif' width='"+icoSize+"' border=0 />";

        for ( var it=0; it<node.treenode.depth ; ++it ) {
            var connector="";
            if(it==0) {
                if(tn.childId==tn.parent.children.length-1) connector="northeast"; // "&#9492;&#9472;";
                else connector="northsoutheast";//"&#9500;&#9472;";
            }else {
                if(tn.childId==tn.parent.children.length-1) connector="white"; // null;
                else connector="northsouth";//"&#9474;";
            }
            if(tn.depth==1)break;
            tt="<td width="+icoSize+"><img src='img/tree-white.gif' width='"+icoSize+"' border=0 /></td>"+tt;

            tn=tn.parent;

        }
        tt="<table><tr>"+tt+"<td>";
        tn=node.treenode;

        var imgnam=""; // "bio-run" 
        if(tn && tn.children && tn.children.length){
            if(!tn.expanded)imgnam='recExpand';
            else imgnam='recCollapse';
        }

        if(this.onClickExpandNode)tt+="<span onclick='vjObjEvent(\"onClickExpandNode\",\""+this.objCls+"\", \""+sanitizeElementAttrJS(tn.path)+"\","+ tn.expanded+")' style='cursor:hand'>";
        if(imgnam)tt+="<img src='img/"+imgnam+".gif' width="+icoSize+" />";
        if(this.onClickExpandNode)tt+="</span>";
        tt+="</td><td>"+(node.title?node.title:node.name)+"</td></tr></table>";
        realText=tt;
        return realText;
    };

    this.isVisible=function(){
        if(!this.div){
            return false;
        }
        return __isVisible(this.div);
    };

    this.getIconSrc=function(icontxt) {
    };

    this.resize = function(diffW, diffH) {
        var el = gObject(this.container);
        var ret = {};
        if (!el) return ret;

        if (!isNaN(parseInt(diffW))) {
            ret.diffW = parseInt(diffW);
            ret.oldW = parseInt(el.style.width);
            if (!ret.oldW || isNaN(ret.oldW))
                ret.oldW = el.clientWidth;

            ret.newW = ret.oldW + ret.diffW;
            el.style.width = ret.newW + "px";

            var oldMaxW = parseInt(this.tab.parent.width);
            if (!isNaN(oldMaxW)) {
                for (var iv=0; iv<this.tab.viewers.length; iv++) {
                    var tab_el = gObject(this.tab.viewers[iv].container);
                    if (ret.newW > oldMaxW) {
                        ret.maxW = ret.newW;
                    } else {
                        ret.maxW = oldMaxW;
                    }
                    tab_el.style.maxWidth = ret.maxW + "px";
                }
            }
        }

        if (!isNaN(parseInt(diffH))) {
            ret.diffH = parseInt(diffH);
            ret.oldH = parseInt(el.style.height);
            if (!ret.oldH || isNaN(ret.oldH))
                ret.oldH = el.clientHeight;

            ret.newH = ret.oldH + ret.diffH;
            el.style.height = ret.newH + "px";

            var oldMaxH = parseInt(this.tab.parent.height);
            if (!isNaN(oldMaxH)) {
                for (var iv=0; iv<this.tab.viewers.length; iv++) {
                    var tab_el = gObject(this.tab.viewers[iv].container);
                    if (ret.newH > oldMaxH) {
                        ret.maxH = ret.newH;
                    } else {
                        ret.maxH = oldMaxH;
                    }
                    tab_el.style.maxHeight = ret.maxH + "px";
                }
            }
        }

        return ret;
    };

    this.getTitle = function() {
        if (this.title) {
            return this.title;
        }
        var ds = this.data ? vjDS[verarr(this.data)[0]] : null;
        if (ds) {
            var title = ds.title ? ds.title : ds.name;
            return title.replace(/.*(composing|computing|constructing|finding|getting|loading|preparing|retrieving) /i, "");
        }
        return "viewer";
    };
}


// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Dataview Tab
// _/
// _/_/_/_/_/_/_/_/_/_/_/

function vjDataViewTab(name, icon, type, viewers, doClone, order)
{
    this.name=name;
    this.icon=(icon ? icon : name );
    this.viewers=new Array(); // one of the known types of viewers; currently "ggraph', seltable
    this.type=type; // tab, link, download, redir
    if(this.columns===undefined)this.columns=1;
    this.width='100%';
    this.height='1%';
    this.valign='top';
    this.align='left';
    this.viewtoggles=0;
    this.is_composed=false;

    // _/_/_/_/_/_/_/_/_/_/_/
    // Constructor

    this.add=function(viewers, doClone)
    {
        var vie=verarr(viewers);

        for ( var iv=0;  iv< vie.length ; ++ iv) {
            //vie[iv].uniqueId=Math.random();
            // remember this viewer in our list
            var vv = (!doClone && vie[iv].isDataViewViewer) ? vie[iv] : new vjDataViewViewer(vie[iv]);
            vv.dataSourceEngine=this.dataSourceEngine;
            vv.dataViewEngine=this.dataViewEngine;
            vv.tab=this;
            if(!vv.hidden)this.activeView=iv;
            vv.iview=this.viewers.length;
            this.viewers[this.viewers.length]=vv;

            if( isok(vie[iv].data) ) {
                vv.register_callback(0,this);
            }
        }
    };

    this.remove=function(viewers)
    {
        if (!viewers) {
            viewers = this.viewers;
        }
        var remove_ids = {};
        (Array.isArray(viewers) ? viewers : verarr(viewers)).forEach(function(v) {
            if (v.uniqueId !== undefined) {
                remove_ids[v.uniqueId] = true;
            }
        });
        for(var it=0; it<this.viewers.length; it++){
            if (remove_ids[this.viewers[it].uniqueId] === true) {
                this.viewers[it].unlink();
                this.viewers.splice(it, 1);
                it--; // to compensate for loop increment and spliced out member
            }
        }
    };

    // _/_/_/_/_/_/_/_/_/_/_/
    // Loader

    this.load=function( loadmode  ) // load all the viewers
    {
        var vie=verarr(this.viewers);
        for ( var iv=0;  iv< vie.length ; ++ iv) {
            if(!vie[iv].forceLoad && vie[iv].hidden)continue;
//            if(vie[iv].loadIssued)continue;
            // If the tab html was regenerated, composeText() would have set vie[iv]._rendered = false;
            // if the viwer was never rendered, vie[iv]._rendered would not exist.
            // In both of these cases, the viewer must be force-rendered, no matter what its datasource thinks.
            vie[iv].load(vie[iv]._rendered ? loadmode : "rerender");
//            vie[iv].loadIssued=true;
        }
    };

    this.refresh=function( mode  ) // load all the viewers
    {
        var vie=verarr(this.viewers);
        for ( var iv=0;  iv< vie.length ; ++ iv) {
            var dlist=verarr( vie[iv].data );
            var donecnt=0;
            for( var j=0; j< dlist.length; ++j) {
                if(vie[iv].getData(j).state=='done')
                    donecnt++;
            }

            if(donecnt==dlist.length){
                if( vie[iv].refresh)
                    vie[iv].refresh(mode);
            }else
                vie[iv].load(mode);
        }

    };

    this.add(viewers, doClone);
}


// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Dataview
// _/
// _/_/_/_/_/_/_/_/_/_/_/



function vjDataView( name, width, height , maxtabs )
{
    this.name=name;
    this.container=sanitizeElementId(this.name);  // the default container is the same as the name
    this.tabs=new Object ({length:0});
    //this.tabsN=new Array (); // keeps the tabs by name
    this.width=(width ? width  : 0 );
    this.height=(height ? height : 0 );
    this.maxtabs=(maxtabs ? maxtabs : 0 );
    this.tabstart=0; // index in this.sortTabs() of first tab to show
    this.selected=0; // tab.num of current selected tab
    //this.active=''; // the list of available tabs
    this.iconSize=24;
    this.iconToggleSize=18;
    this.tabSeparation=10;
    this.tabHeight=16;

    // _/_/_/_/_/_/_/_/_/_/_/
    // Constructors

    this.add=function(name, icon, type, viewers, doClone, order)
    {
        var tb=this.tabs[name];
        if(!tb){
            tb=new vjDataViewTab(name, icon, type);
            tb.num=this.tabs.length;
            tb.parent=this;
            order = parseFloat(order);
            tb.order = isNaN(order) ? this.tabs.length : order;
            tb.dataViewEngine=this.dataViewEngine;
            tb.dataSourceEngine=this.dataSourceEngine;
            this.tabs[this.tabs.length]=tb;
            this.tabs[name]=this.tabs[this.tabs.length];
            ++this.tabs.length;
        }
        tb.add(viewers, doClone);
        //this.tabsN[name]=tb;

        return tb;
    };
    this.objCls="cls"+Math.random();//this.name;
    vjObj.register(this.objCls,this); // needed to find this object in events by object container

    this.addTab=function(name, icon, viewers, doClone, order) { return this.add(name, icon, "tab", viewers, doClone, order);};

    this.remove=function(t_names,removeviewers){
        if(!t_names){
            this.tabs={};
            this.tabs.length=0;
            return ;
            }
        var names=verarr(t_names);
        for(var n=0; n<names.length ; ++n){
            var tb = this.tabs[names[n]];
            if(tb){
                if(!removeviewers)tb.remove();
                delete this.tabs[tb.num];
                delete this.tabs[tb.name];
                --this.tabs.length;
            }
        }

    };

    // _/_/_/_/_/_/_/_/_/_/_/
    // Renderer

    this.updateTabVisibility = function(force_recompose, min_selectable, max_selectable) {
        if (min_selectable === undefined) {
            min_selectable = 0;
        }
        if (max_selectable === undefined) {
            max_selectable = this.tabs.length - 1;
        }
        var recompose = force_recompose;

        if (this.selected < min_selectable || this.selected > max_selectable) {
            this.selected = 0;
            recompose = true;
        }
        for (var it = 0; it < this.tabs.length; ++it) {
            o = gObject(this.tabs[it].container);
            if (o) o.className = (it == this.selected) ? "sectVis" : "sectHid";
            var viewer = verarr(this.tabs[it].viewers);
            for (var iv = 0; iv < viewer.length; ++iv) {
                viewer[iv].tabvisible = (this.selected == it) ? true : false;
            }
        }

        if (recompose) {
            var nm = "DV_" + this.container + "_tablist"; // the tablist is here
            o = gObject(nm);
            if (o) o.innerHTML = this.composeText(2);
            nm = "DV_" + this.container + "_viewtoggles"; // the tablist is here
            o = gObject(nm);
            //this.tabs[this.selected]
            if (o) o.innerHTML = this.composeText(4);
        }
    };

    this.render = function (reuse) {
        if(this.maxtabs){
            this.invisibleCnt=0;
            var visibleTabs=0;
            for (var it = this.tabstart ; visibleTabs < this.maxtabs && it<this.tabs.length; ++it) {
                if(this.tabs[it].invisible)
                    ++this.invisibleCnt;
                else
                    ++visibleTabs;
            }
        }

        var o = gObject(this.container); if (!o) return;
        //{o.style.resize="both";o.style.overflow="auto";}

        if (!reuse) {
            o.innerHTML = this.composeText();
        } else {
            var eov = gObject("DV_" + this.container + "_end_of_viewers");
            if (eov) {
                // If we added some new tabs, we want to add divs for their new viewers
                // without making the browser redraw existing tabs' viewers
                eov.insertAdjacentHTML("beforebegin", this.composeText(1));
            }

            // Update viewer and tab visibility
            this.updateTabVisibility(true);
        }
        //this.fixResizeables();
        if(this.callbackDVrendered)
            funcLink(this.callbackDVrendered,this);
        return;
    };

    this.sortTabs=function()
    {
        var tabs_sorted = [];
        for(var i=0; i<this.tabs.length; i++) {
            tabs_sorted.push(this.tabs[i]);
        }
        return tabs_sorted.sort(function(t1,t2) { return t1.order - t2.order; });
    };

    this.tabNum2SortIndex=function(tabnum)
    {
        var tabs_sorted = this.sortTabs();
        for(var i=0; i<tabs_sorted.length; i++) {
            var tb = tabs_sorted[i];
            if(tb.num == tabnum) {
                return i;
            }
        }
        return undefined;
    };

    this.composeText=function( whattocompose)
    {
        var tabs_sorted = this.sortTabs();

        if(this.frame=="none"){
            var t1="";
            for (var i=0 ; i<tabs_sorted.length; ++i ) {
                var tb=tabs_sorted[i];
                var viewer=verarr(tb.viewers);
                var cntviewer=viewer.length;

                for ( var iv=0; iv<cntviewer; ++iv) {
                    viewer[iv].container=sanitizeElementId("DV_"+this.container+"_"+tb.name+"_"+iv);
                    viewer[iv].tabvisible=(this.selected==tb.num) ? true : false;

                    t1+="<div id='"+viewer[iv].container+"'  >";
                    t1+="</div>";
                    if(iv<cntviewer-1 && ((iv+1)%tb.columns)==0)t1+="<br/>";
                }
            }
            return t1;
        }

        if(!whattocompose){
            whattocompose=15;
            tabs_sorted.forEach(function(tab) { tab.is_composed=false; });
        }

        var sp=" cellspacing=0 cellpadding=0 border=0 ";
        var t1="";
        if(whattocompose&1) { // data viewers
            var num_tabs_composed = 0;
            if (whattocompose != 1) {
                t1+="<table "+sp+" width='100%' height='100%'>";
                t1+="<tr><td valign=top width=100%>";
            }

                    for (var i=0 ; i<tabs_sorted.length; ++i ) {
                        var tb=tabs_sorted[i];
                        if (tb.is_composed) {
                            continue;
                        }
                        tb.is_composed=true;
                        num_tabs_composed++;
                        tb.container=sanitizeElementId("DV_"+this.container+"_"+tb.name);
                        t1+="<div id='"+tb.container+"' class='"+( this.selected==tb.num ? "sectVis" : "sectHid" ) +"' style='overflow:auto;" ;
                        if(__isIE)t1+="min-height:0%;";
                        if(this.height && this.frame!="notab")
                            t1+="height:"+this.height+";";
                        t1+="' >";
                        var viewer=verarr(tb.viewers);
                        var cntviewer=viewer.length;

                            t1+="<table "+sp+" width='100%' height=" + (tb.height ? tb.height : '100%') +">";
                                t1+="<tr>";
                                var colspans=0, startRowSpan = new Array(); rowspans=0;
                                var curTr=0;
                                for ( var iv=0, curTd=0; iv<cntviewer; ++iv,++curTd) {
                                    viewer[iv].container=sanitizeElementId("DV_"+this.container+"_"+tb.name+"_"+iv);
                                    //viewer[iv].container=this.name+"."+tb.name+"."+iv;
                                    viewer[iv].tabvisible=(this.selected==tb.num) ? true : false;
                                    viewer[iv]._rendered = false; // since viewer[iv].div will be pointing out of dom after tab's html is regenerated

                                    t1+="<td ";
                                    if(viewer[iv].colspan){
                                        t1+=" colspan='"+viewer[iv].colspan+"' ";
                                        colspans+=viewer[iv].colspan-1;
                                    }
                                    if(viewer[iv].rowspan){
                                        t1+=" rowspan='"+viewer[iv].rowspan+"' ";
                                        startRowSpan[curTr]= viewer[iv].rowspan-1;
                                        
                                    }
                                    t1+=" class='"+(this.selected==tb.num ? "cellVis" : "cellHid" )+ "' id='"+viewer[iv].container+"_cell' valign='"+(tb.valign)+"' align='"+(tb.align)+"' " + (tb.width ? ("width="+tb.width) : "");
                                    t1+=" >";
                                        var szWd=gConstructCSSMetrics(this.width),szVWd=gConstructCSSMetrics(viewer[iv].width);
                                        var szHi=gConstructCSSMetrics(this.height),szVHi=gConstructCSSMetrics(viewer[iv].height);
                                        var szVMWd=gConstructCSSMetrics(viewer[iv].maxWidth);
                                        var szVMHi=gConstructCSSMetrics(viewer[iv].maxHeight);

                                        t1+="<div ";
                                        t1+=" onMouseEnter='vjObjEvent(\"onShowResizer\",\""+this.objCls+"\",this,"+tb.num+","+iv+",\"rb\")' ";
                                        t1+="id='"+viewer[iv].container+"' style='position:relative;overflow:auto;margin:0px;padding: 0px;";
                                        //if(this.frame!="notab")
                                        if(tb.columns>1)t1+="display:inline-block;vertical-align:top;";
                                        t1+="max-height:"+(szVMHi||szHi)+";";
                                        t1+="max-width:"+(szVMWd||szWd)+";";
                                        //if(szVHi)t1+="height:"+szVHi+";";
                                        //if(szVWd)t1+="width:"+szVWd+";";
                                        if (szVHi != undefined) {
                                            t1+="height:"+szVHi+";";
                                        }
                                        if (szVWd != undefined) {
                                            t1+="width:"+szVWd+";";
                                        }
                                        t1+="'>";

                                           // t1+=viewer[iv].container;
                                        t1+="</div>";
                                    t1+="</td>";
                                    //alerJ("iv="+iv + " cntviewer=" + cntviewer + " " + tb.columns + " iv<cntviewer+1 && ((iv+1)%tb.columns)==0="+(iv<cntviewer+1 && ((iv+1)%tb.columns)==0),tb)
                                    if(iv<cntviewer+1 && ((rowspans+colspans+curTd+1)%tb.columns)==0){
                                        t1+="</tr><tr>"; // insert a new row  if new columns has to be started
                                        curTr++;
                                        curTd=0;
                                        rowspans=0;colspan=0;
                                        for(var irsp = 0 ; irsp < startRowSpan.length ; irsp++ ) {
                                            if(!startRowSpan[irsp]){
                                                rowspans++;
                                                startRowSpan[irsp]--;
                                            }
                                        }
                                    }
                                }
                                t1+="</tr>";
                            t1+="</table>";
                        t1+="</div>";
                    }

            if (whattocompose == 1) {
                return num_tabs_composed ? t1 : "";
            } else {
                t1+="<div id='DV_"+this.container+"_end_of_viewers' class='sectVis' style='overflow:auto;";
                if(this.height && this.frame!="notab")
                    t1+="height:"+this.height+";";
                t1+="' >";
                t1+="</td></tr>";
                t1+="</table>";
            }
        }

        var t2="";
        if(whattocompose&2 && this.frame!="none"){ // tab names
            //t2+="<tr><td colspan="+this.tabs.length +" >"+t3+"</td></tr>";
            //t2+=t3;
            var tableTabSize="";
            if(this.width){
                var t_width=parseInt(this.width);
                if(this.maxtabs && t_width){
                    t_width-=4*this.tabSeparation;
                }
                tableTabSize="width="+t_width;
            }
            t2+="<table  "+sp+" " + tableTabSize +" " + (this.tabHeight ? ("height="+this.tabHeight) : "") + " >";
                t2+="<tr>";
//                var curHiddenTabs=0;
                for (var i=0; i<tabs_sorted.length ; ++i ) {
//                    if(i<this.tabstart)continue;
//                    if(this.maxtabs && i-this.tabstart>=(this.maxtabs-curHiddenTabs))continue;
//                    if( tabs_sorted[i].hidden ){++curHiddenTabs;continue;}
                    var tb = tabs_sorted[i];
                    if(i<this.tabstart || (this.maxtabs && i-this.tabstart>=(this.maxtabs+this.invisibleCnt)))
                        tb.hidden=true;
                    else if (!tb.invisible){
                        tb.hidden=false;
                    }
                    var cls=( this.selected==tb.num ? "DV_tab_selected" : "DV_tab" );
                    t2+="<td "+sp+" id='"+sanitizeElementId("DV_"+this.container+"_"+tb.name+"_tab"+tb.num)+"' valign=middle";
                    if(tb.hidden)t2+=" class='sectHid' ";
                    else if(this.frame!="none")t2+=" class='"+ cls +"' ";
                    t2+=" height="+ this.tabHeight+" width='1%' >";
                        t2+="<span style='white-space: nowrap;'>";
                        // don't make the currentselected tab clickable, but put
                        // it in a clickable-styled span for visual consistency
                        if (this.selected == tb.num) {
                            t2 += "<span class='clickablish'>";
                        } else {
                            t2+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickTabSelect\",\""+this.objCls+"\","+tb.num+",\""+sanitizeElementAttrJS(tb.name)+"\"); stopDefault(event);'>";
                        }
                        //t2+="<img border=0 src='img/white.gif' width="+this.tabSeparation+" height=1 />&nbsp;"

                        var icontxt = tb.icon;
                        if( tb.icon.indexOf("/") == -1 )
                            icontxt="img/"+icontxt;
                        if( tb.icon.indexOf(".") == -1 )
                            icontxt=icontxt+".gif";
                        t2+="<img class='DV_tab_icon' src='"+icontxt+"' width="+this.iconSize+" height="+this.iconSize+" />";
                        var tabname_id = sanitizeElementId("DV_" + this.container + "_" + tb.name + "_tabname");
                        t2+="<span class='DV_tab_label' id='" + tabname_id + "'>";
                        t2+=tb.title?tb.title:tb.name;
                        t2+="</span>";
                        if (this.selected == tb.num) {
                            t2 += "</span>";
                        } else {
                            t2 += "</button>";
                        }
                        //t2+="<img border=0 src='img/white.gif' width="+this.tabSeparation+" height=1 />";
                        if(tb.canclose){
                            t2+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickTabClose\",\""+this.objCls+"\","+tb.num+",\""+sanitizeElementAttrJS(tb.name)+"\"); stopDefault(event);'>";
                            t2+="<img class='DV_tab_icon' src='img/close.gif' width="+this.iconSize+" height="+this.iconSize+" />";
                            t2+="</button>";
                        }
                        t2+="</span>";

                    t2+="</td>";
                }
                t2+="<td ";
                t2+=" class='DV_tab' width='"+(100-tabs_sorted.length)+"%' ";
                t2+=">&nbsp;</td>";
                t2+="</tr>";

            t2+="</table>";
            if(whattocompose==2)return t2;
        }

        var t3="";
        if(whattocompose&4 && this.selected<this.tabs.length){ // view toggles
            var tb=this.tabs[this.selected];
            var viewer=verarr(tb.viewers);
            var cntviewer=viewer.length;
            //alerJ(tb.viewtoggles,tb);
            //alerJ(tb.viewtoggles,this);
            if(cntviewer>1 && tb.viewtoggles ){
                var seenNames = {};
                for ( var iv=0; iv<cntviewer; ++iv) {
                    if(viewer[iv].hideViewerToggle) continue;
                    if(!viewer[iv].hidden && tb.viewtoggles==-1)continue;
                    var viewerName = isok(viewer[iv] && viewer[iv].name ) ? viewer[iv].name : iv;
                    // treat all viewers with the same name as a group that has one toggle
                    if(seenNames[viewerName]) continue;
                    seenNames[viewerName] = true;
                    //t3+="<tr>";

                    t3+="<span class='DV_tab_label'><small>&raquo;</small></span>";
                        //t3+="<img border=0 src='img/white.gif' width="+this.tabSeparation+" height=1 />&nbsp;"
                    t3+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickViewToggle\",\""+this.objCls+"\","+this.selected+","+iv+"); stopDefault(event);'>";
                    t3+="<span class='DV_tab_icon'>";
                    var icothis=(this.iconToggleSize && viewer[iv].icon) ? "<img src='img/"+ viewer[iv].icon  +".gif' width="+this.iconToggleSize+" height="+this.iconToggleSize +" border=0 />" : "+";
                    if(tb.viewtoggles==-1){
                        t3+=icothis;"<img src='img/"+ viewer[iv].icon  +".gif' width="+this.iconToggleSize+" height="+this.iconToggleSize +" border=0 />";
                    }
                    else t3+=viewer[iv].hidden ? icothis : "&times;";
                    t3+="</span>";

                    t3+="<small class='DV_tab_label'>" + viewerName + "</small>";

                    t3+="</button>";
                    //t3+="</tr>";
                }

            }
            var o=gObject("DV_"+this.container+"_viewtoggles");
            if(o)o.className=t3.length ? "DV_content" : "sectHid";
            if(whattocompose==4)return t3;
        }


        var t="";
        if(whattocompose&8) { // the framing of the whole dv
            t+="<table border=0 "+sp+" " + (this.width ? ("width="+this.width) : "") +" ";
            if(this.height )
                t+="height="+this.height ;
            t+= " >";


                var t22="";
                if(this.frame!="notab") {
                    t22+="<tr>";
                        t22+="<td height="+this.tabHeight;
                        if(this.frame!="none")t22+=" class='DV_tab' ";
                        t22+=" width="+(this.tabSeparation*2)+" valign='center'";
                        if(!this.maxtabs)t22+= " style='display:none'";
                        t22+=">";
                            //if(this.tabstart>0)
                            if(this.maxtabs){
                                t22+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickTabScroll\",\""+this.objCls+"\",-1); stopDefault(event);'>";
                                t22+="<img src='img/left.gif' class='DV_tab_icon' height="+this.tabHeight+" width="+(this.tabSeparation*2)+" />";
                                t22+="</button>";
                             }
                        t22+="</td>";
                        t22+="<td height="+this.tabHeight+" id='DV_"+this.container+"_tablist'  valign='top' >";
                            t22+=t2;
                        t22+="</td>";
                        t22+="<td height="+this.tabHeight;
                        if(this.frame!="none")t22+=" class='DV_tab' ";
                        t22+=" width="+(this.tabSeparation*2)+" valign='center' ";
                        if(!this.maxtabs)t22+= " style='display:none'";
                        t22+=">";
                            //if(this.maxtabs && this.tabstart+this.maxtabs<this.tabs.length)
                            if(this.maxtabs){
                                t22+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickTabScroll\",\""+this.objCls+"\",1); stopDefault(event);'>";
                                t22+="<img src='img/right.gif' class='DV_tab_icon' height="+this.tabHeight+" width="+(this.tabSeparation*2)+" />";
                                t22+="</button>";
                            }
                        t22+="</td>";
                    t22+="</tr>";
                } else {
                    t22+="<tr height='0px' >";
                        t22+="<td class='DV_tab' width='100%' height='0px' >";
                        t22+="</td>";
                    t22+="</tr>";
                }


                if(!this.tabsOnBottom)
                    t+=t22;

                t+="<tr>";
                t+="<td colspan=3  id='DV_"+this.container+"_tabcontainer' valign=top ";
                if(this.frame!="none")t+=" class='DV_content' ";
                t+=">";
                    t+=t1;
                t+="</td>";
                t+="</tr>";

           //if(t3.length){
                t+="<tr  height=0 >";
                    t+="<td colspan=3 id='DV_"+this.container+"_viewtoggles' class='"+(t3.length ? "DV_content" : "sectHid")+"' >";
                        t+=t3;
                    t+="</td>";
                t+="</tr>";
            //}

            if(this.tabsOnBottom)
                t+=t22;
            //if(this.resizeable)

            t+="</table>";

            if(this.hideDV){
                var tmp;

                tmp="<div id='DV_"+this.container+"_switcher' valign='top' align='right' height='100%'>";
                tmp+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickToggleDV\",\""+this.objCls+"\",0); stopDefault(event);'>";
                tmp+="<img src='img/minus.gif' height='15' />" ;
                tmp+="</button></div>";

                tmp+="<div id='DV_"+this.container+"_frame'>"+t+"</div>";


                t=tmp;
            }
            //return t;
        }
        //alert(t1);

        return t;
    };


    // _/_/_/_/_/_/_/_/_/_/_/
    // Loader

    this.load=function( loadmode )
    {
        //var tbs=verarr(this.tabs);
        for ( var it=0;  it< this.tabs.length ; ++ it) {

            //alerJ("loading Viewer " + (this.tabs[it].type=='tab' || this.tabs[it].type=='menu') && ( it==this.selected || this.frame=='none' || this.forceLoadAll || this.tabs[it].forceLoadAll ) , this.tabs[it] ) ;
            if( (this.tabs[it].type=='tab' || this.tabs[it].type=='menu') && ( it==this.selected || this.frame=='none' || this.forceLoadAll || this.tabs[it].forceLoadAll ) ){
                //alerJ(it+" loading  "+this.selected, this.tabs[it]);
                this.tabs[it].load(loadmode);
            }
        }
    };

    this.refresh=function( mode )
    {
        for ( var it=0;  it< this.tabs.length ; ++ it) {
            this.tabs[it].refresh(mode);
        }
    };
    // _/_/_/_/_/_/_/_/_/_/_/
    // Utility

    this.find=function( tbname, iview )
    {
        //var tb=this.tabsN[tbname];if(!tb)return ;
        var tb;

        if(tbname=="_active")tb=this.tabs[this.selected];
        else tb=this.tabs[tbname];
        if(!tb)return ;
        if(iview=="")return tb;

        if(iview=="_active")return tb.viewers[tb.activeView];
        var num=parseInt(iview);
        if(isNaN(num)){
            for ( var ik=0; ik<tb.viewers.length; ++ik )
                if(tb.viewers[ik].name & tb.viewers[ik].name==iview)return tb.viewers[ik];
            return ;
        }
        return tb.viewers[num];
    };

    this.select=function ( tbname , dorender )
    {
        if(!this.tabs[tbname])return ;
        var prv_selected=this.selected;
        this.selected=this.tabs[tbname].num;
        this.tabs[tbname].hidden=false;

        if(dorender){
            if(this.maxtabs) {
                //alert(this.selected + " == " + this.tabstart + " " + this.maxtabs+"\n"+(this.selected<this.tabstart)+"\n"+(this.selected>=this.tabstart+this.maxtabs));
                var selected_sort_index = this.tabNum2SortIndex(this.selected);
                if (selected_sort_index < this.tabstart) {
                    this.tabstart = selected_sort_index;
                }
                if (selected_sort_index >= this.tabstart + this.maxtabs + (this.invisibleCnt?this.invisibleCnt:0)) {
                    this.tabstart = selected_sort_index - this.maxtabs + (this.invisibleCnt?this.invisibleCnt:0) + 1;
                }
                //alert(this.tabstart)
            }
            if(this.callbackTabSelect)
                funcLink(this.callbackTabSelect,this,this.tabs[tbname].name,this.tabs[tbname].num,prv_selected);
            this.render(true);
            this.load();
        }
    };

    this.hide=function ( tbname , dorender )
    {
        if(!this.tabs[tbname])return ;
        var it;
        for( it=this.tabs[tbname].num-1; it>=0 && this.tabs[it].hidden==true ; --it) ;
        if(it<0){for( it=this.tabs[tbname].num+1; it<this.tabs.length && this.tabs[it].hidden==true ; ++it) ;}
        if( it==this.tabs.length )return ;

        this.tabs[tbname].hidden=true;
        this.select( it, dorender ) ;
    };

    // _/_/_/_/_/_/_/_/_/_/_/
    // Events

    this.onShowResizer=function(cls, e, it, iv ,type )
    {
        var vObjCls;
        var container;
        var callback;
        if(it!=-1){ // viewer resizing
            var v = verarr(this.tabs[it].viewers)[iv];//var o=gObject(v.container+"_resizeable");if(!o)return;
            //callback=(v.refreshOnResize) ? "javascript:vjObjEvent(\"onResizeViewer\",\""+this.objCls+"\","+it+","+iv+")" : "" ;
            //callback="javascript:vjObjEvent(\"onResizeViewer\",\""+this.objCls+"\","+it+","+iv+")"  ;
            callback="javascript:vjObjEvent(\"onResizeViewer\",\""+this.objCls+"\","+it+","+iv+")" ;
            vObjCls = v.objCls;
            container = v.container;
        } else {
            vObjCls = null;
            container = iv;
        }
        gShowResizer (container, vObjCls, type, callback);
        /*var t=gResizerText(container, vObjCls, type, callback);
        var o = gObject(container) ;
        var pos = gObjPos(o, false);
        //alerJ(5, gObjPos(gObject(v.container) ));
        gObjectSet('basic-resizer',pos.x+o.clientWidth,pos.y+o.clientHeight,t,'show','-','-');
        */

    }
    this.onResizeViewer=function(cls,it, iv )
    {
        //alert("resize");
        var v=verarr(this.tabs[it].viewers)[iv];//var o=gObject(v.container+"_resizeable");if(!o)return;
        //if(v.refreshOnResize && v.refresh)
        if (v.refresh)
            v.refresh();
    }

    this.onClickViewToggle=function(name, itb , ivi , donotrender )
    {

        function setHidden (v, hidden) {
            v.hidden = hidden;
            var o = document.getElementById(v.container+"_cell");
            if (o) o.className = hidden ? "cellHid" : "cellVis";
        }

        var tb=this.tabs[itb];

        var viewer;
        var o;
        if(tb.viewtoggles<0){
            for ( var ik=0; ik<tb.viewers.length; ++ik){
                if(ivi==ik)continue;
                viewer=tb.viewers[ik];
                if(viewer.hideViewerToggle)continue;
                setHidden(viewer, true);
            }
        }

        if(ivi>=0) {
            viewer=tb.viewers[ivi];
            setHidden(viewer, !viewer.hidden);
            var viewerName = viewer.name ? viewer.name : ivi;

            for (var ik = 0; ik<tb.viewers.length; ik++) {
                var viewerk = tb.viewers[ik];
                if (ik == ivi || viewerk.uniqueId == viewer.uniqueId) continue;

                if (viewerk.name == viewerName || (!viewerk.name && ik == viewerName))
                    setHidden(viewerk, viewer.hidden);
                else if (viewer.uniGroup && viewerk.uniGroup == viewer.uniGroup)
                    setHidden(viewerk, !viewer.hidden);
            }
        }
        tb.activeView=ivi;

        /*
        var cols=tb.columns ? tb.columns : 1 , rows=0;
        for ( var iv=0; iv<tb.viewers.length; ++iv)if( !tb.viewers[iv].hidden ) rows++;if(!rows)rows=1;
        for ( var iv=0; iv<tb.viewers.length; ++iv) {
            viewer=tb.viewers[iv];
            viewer.widthAvail=parseInt(parseInt(this.width)/cols);
            viewer.heightAvail=parseInt(parseInt(this.height)/rows);
            //viewer.reload();
            //this.loadview(viewer);
        }*/

        if(donotrender)
            return;

        this.render(true);
        this.load();
    };

    this.onClickTabScroll=function( name,what )
    {
        this.tabstart+=what;
        if(this.maxtabs && this.tabstart>this.tabs.length-(this.maxtabs+(this.invisibleCnt?this.invisibleCnt:0)))
            this.tabstart=this.tabs.length-(this.maxtabs+(this.invisibleCnt?this.invisibleCnt:0));
        if(this.tabstart<0)this.tabstart=0;
        this.render(true);
    };

    this.onClickTabClose=function( name,itb)
    {
        //this.tabs[itb].hidden=true;
        //this.render(true);
        this.hide( itb, true) ;
    };

    this.onClickTabSelect=function( name,itb)
    {
        var tb=this.tabs[itb];
        var prv_selected=this.selected;
        if( tb.type=='tab' ) {
            this.selected=itb;
            if (this.maxtabs) {
                var selected_sort_index = this.tabNum2SortIndex(this.selected);
                if (selected_sort_index <= this.tabstart) {
                    this.tabstart=selected_sort_index-1;
                }
                if (selected_sort_index >= this.tabstart+this.maxtabs +(this.invisibleCnt?this.invisibleCnt:0)-1) {
                    this.tabstart = this.selected + 1 - (this.maxtabs+(this.invisibleCnt?this.invisibleCnt:0)) + 1;
                }
                if(this.tabstart<0)this.tabstart=0;
                if(this.tabstart>=this.tabs.length)this.tabstart=this.tabs.length-1;
            }
            this.render(true);
            tb.load();
            if(this.callbackTabSelect)
                funcLink(this.callbackTabSelect,this,this.tabs[itb].name, itb, prv_selected);
        }
        else if( tb.type=='download' ) {
            tb.load("download");
        }
        else if(tb.type=='link'){
            document.location=tb.src;
        }else if(tb.type=='function'){
            if(tb.viewers)
                eval(tb.viewers+"()");
        }
    };

    this.onClickToggleDV=function(dvname,action){
        var o=gObject("DV_"+this.container+"_frame");
        if(o){
            if(!action)
                o.style.display='none';
            else
                o.style.display='inline';
        }

        var o=gObject("DV_"+this.container+"_switcher");
        if(o){
            if(!action){
                var tmp="";
                tmp+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickToggleDV\",\""+this.objCls+"\",1); stopDefault(event);'>";
                tmp+="<img src='img/plus.gif' height='15' />" ;
                tmp+="</button>";
                o.innerHTML=tmp;
            }
            else{
                var tmp="";
                tmp+="<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onClickToggleDV\",\""+this.objCls+"\",0); stopDefault(event);'>";
                tmp+="<img src='img/minus.gif' height='15' />" ;
                tmp+="</button>";
                o.innerHTML=tmp;
            }
        }
    };
}

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ DataViewEngine
// _/
// _/_/_/_/_/_/_/_/_/_/_/


function vjDataViewEngine( dataSourceEngine )
{
    //this.DataViews= new Array ();
    this.registeredRenderers=new Array();
    this.dataSourceEngine =dataSourceEngine;

    this.add=function(name, width, height, maxtabs)
    {
        var dv=new vjDataView(name,width,height,maxtabs);
        dv.dataSourceEngine=this.dataSourceEngine;
        dv.dataViewEngine=this;
        this[name]=dv;
        return dv;
    };

    this.registerViewer=function( viewerTypeName, viewerFunction)
    {
        this.registeredRenderers[viewerTypeName]=viewerFunction;
    };

    this.find=function( dvname, tbname, iview )
    {
        var dv=this[dvname]; if(!dv)return ;
        return dv.find(tbname, iview);
    };

    this.locate=function( objorname )
    {
        if( typeof( objorname )!='string')
            return objorname;

        var nm=objorname.split('.');
        return this.find(nm[0],nm[1],nm[2]);
    };

    this.select=function( objorname , redraw)
    {
        var viewer=this.locate( objorname );
        viewer.tab.parent.select(viewer.tab.num,redraw);
        //viewer.tab.parent.selected=viewer.tab.num;
        //if(redraw)viewer.tab.parent.render();
        return viewer;
    };
/*
    this.viewerToDivs=function ( dvorTabList , tabs , associationList  )
    {
         var t_viewersArr=new Array();
         for ( var io=0 ; io<associationList.length; ++io ){
             var as=associationList[io];

             t_viewerArr=t_viewersArr.concat(dvorTabList[as.dvnum].obj.addTab(tabs.names[as.tabnum],
                     tabs.icons[as.tabnum],verarr(as.viewerList)).viewers;
         }

    }
*/
}



// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Static Even Handlers : statics
// _/
// _/_/_/_/_/_/_/_/_/_/_/
var vjDV=new vjDataViewEngine(vjDS);

//# sourceURL = getBaseUrl() + "/js/vjDataView.js"