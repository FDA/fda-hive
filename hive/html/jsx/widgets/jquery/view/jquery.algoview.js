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

javaScriptEngine.include("js/vjProgress2View.js");
$.loadCSS('css/dialog.css');
$.loadScript('jsx/widgets/jquery/components/project_list_dialog.js');
loadJS('jsx/widgets/vanilla/button.template.js')
loadJS('jsx/widgets/vanilla/htmlpreview.template.js')
loadJS('jsx/widgets/vanilla/textpreview.template.js')
loadJS('jsx/widgets/vanilla/pdbpreview.template.js')

var optionsForPage={};
var currentCompletionState = "preSubmit";
var formName="algoForm";

var valgoToolbarWaitingList = [
    {
        type: '',
        align: 'left',
        order: '1',
        name: 'modresubmit',
        title: 'Modify and Resubmit',
        icon: 'img/recRevert.gif',
        path: '/resubmit',
        url: urlExchangeParameter(document.location,"id","-"+docLocValue("id")),
        description: 'Modify parameters and resubmit this computation using the same template'
    }
]

var valgoToolbarDoneList=[];
var toolBar="processToolbar";
var parametersDiv = "";
var algoWidgetObj;
var originalStat;
var vjRefFileExp=new vjPageLayout();

function algoViewHTMLSetUp(arg1, movedown)
{
    var txt="<form name='algoForm' style='height:100%;'>" +
                "<div class='content'>" +
                    "<div class='main-content' id='mainAlgoArea'></div>" +
                "</div>" +
            "</form>";

    return txt;
}


function valgoProcess(loadedID,  qpSvc, svcProcType, svcRecViewer)
{
    this.recViewerName = 'DV_Parameter_view';
    this.cmdModeLst = {  batch_svc: ["bool:single","batch"]};
    this.ID=docLocValue("id");
    algoProcessID = docLocValue("id");
    this.cmdMode=docLocValue("cmdMode");
    this.help = [];
    var that = this;
    this.cloneMode = this.ID && (this.ID[0] == "-" || this.ID < 0) ? true : false 

    this.valgoCore = function(cls, loadedID )
    {
        this.vjDS=vjDS;
        this.vjDV=vjDV;
        this.vjVIS=vjVIS;

        this.modeActive=true;
        if(loadedID.length && loadedID[0]!='-')this.modeActive=false;
        if(loadedID.length && loadedID[0]=='-')loadedID=loadedID.substring(1);
        this.loadedID=loadedID;
        this.formName=formName;


        this.objCls="algo-"+cls+"-"+Math.random();
        vjObj.register(this.objCls,this);
    };

    this.isMode = function(mode)
    {
        return ((!this.cmdMode || (this.cmdMode.indexOf("-"+mode)==-1 && this.cmdMode.indexOf(mode)!=0))) ? false : true;
    };
    
    this.isResultsMode = function(mode)
    {
        return (!this.cmdMode || (this.cmdMode.indexOf("-"+mode)==-1 && this.cmdMode.indexOf(mode)!=0)) ? false : true;
    };

    this.isBatch = function (){
        if (this.isMode("batch")) {
            return true;
        }

        var svc = this.getValue("svc");
        if (svc && svc.indexOf("batch") != -1) {
            return true;
        }

        var batch_svc = this.getValue("batch_svc");
        if (batch_svc) {
            return parseBool(batch_svc) && batch_svc != "single";
        }

        return false;
    };
    
    this.activateSubmitButton=function(cond)
    {
        if ( !this.submitButon ) {
            this.submitButton = $(".submitterInputButton");
            if (!this.submitButton)
                return;
        }

        if(cond)
            this.submitButton.each(function(){
                $(this).attr("style", "");
            });
        else
            this.submitButton.each(function(){
                $(this).attr("style", "display:none;");
            });
    };

    this.toolbarCallback = function (viewer, content)
    {
        var menuUl = $("#algoMenu");
        var whatNext = $(".whatNext");

        var tab="whatNext";

        if (whatNext.length == 0)
        {
            menuUl.append($(document.createElement("li")).addClass("whatNext")
                .append($(document.createElement("span"))
                            .append($(document.createElement("a"))
                                    .text("What's Next?")
                            )
                            .click(function(){
                                $(this).parent().children('ul').toggle();
                            })
                            .append($(document.createElement("i"))
                                    .addClass("fa fa-angle-right")
                            )
                    )
                    .append($(document.createElement("ul"))
                            .attr({"id": tab})
                    ));
        }
        $("#"+tab).empty();
        let tblArr;
        try{
            tblArr = typeof content === 'string' ? JSON.parse(content) : null
        } catch {
            tblArr = null
        }

        for (var i = 0; tblArr && i < tblArr.length; i++)
        {
            let url = tblArr[i].url;
            if(!url || url === "") continue;
            $("#"+tab).append(
                    $(document.createElement("li"))
                    .append($(document.createElement("a"))
                            .click(function(){
                                funcLink(url);
                            })
                            .text(tblArr[i].title)
                    )
                    .append($(document.createElement("i"))
                            .addClass("fa fa-angle-right")
                    )
            );
        }
    };

    vjDS.add("infrastructure: Constructing Toolbar", "ds"+toolBar, "static:
    vjDS["ds"+toolBar].register_callback(this.toolbarCallback);
    vjDS["ds"+toolBar].reload("static:

    this.valgoCore("process", loadedID );

    var ver="";
    var vl=""+document.location;
    var ps1=vl.indexOf("version~");
    if(ps1!=-1){
        var ps2= vl.indexOf("/",ps1+1) ;
        if(ps2==-1)ps2=vl.indexOf("=",ps1+1) ;
        if(ps2==-1)ver=vl.substring(ps1+8);
        else ver=vl.substring(ps1+7,ps2);
    }
    this.qpSvc=qpSvc+ver;
    this.svcProcType=svcProcType;
    this.svcRecViewer=svcRecViewer;

    if (docLocValue("id") && docLocValue("id").indexOf("-")!=0)
    {
        currentCompletionState = "whileRunning";
    }

    this.onRecordLoaded=function(viewer,text)
    {
        var name = viewer.getElement("name") ? viewer.getElement("name").value : "";
        let slideoutMenuDescription = $("#menuDiv").children("div").children(".slideout-menu-description");
        if (slideoutMenuDescription.length < 1){
            $("#menuDiv").children("div").append(
                $(document.createElement("div"))
                .addClass("slideout-menu-description")
            );
            slideoutMenuDescription =  $(".slideout-menu-description")
            if(Boolean(name)){
                slideoutMenuDescription.append(
                    $(document.createElement("p"))
                        .text(name)
                );
            }
        }
        if (projectHandler.getProjectID()) {
            slideoutMenuDescription.append(
                $(document.createElement("div"))
                .addClass("slideout-menu-projectInfo")
            );
            let slideoutMenuProjectInfo = $(".slideout-menu-projectInfo")
            slideoutMenuProjectInfo.append(
                $(document.createElement("p"))
                    .text(`Project ID: ${projectHandler.getProjectID()}`)
            );

            function fn () {
                if( !projectHandler.getProjectNickname() && !projectHandler.getProjectName() ){ return; }
                if( !projectHandler.getProjectName() ){
                    slideoutMenuProjectInfo.append(
                        $(document.createElement("p"))
                            .text(`Project Nickname: ${projectHandler.getProjectNickname()}`)
                    );
                    return;
                }
                slideoutMenuProjectInfo
                .empty()
                .append(
                    $(document.createElement("p"))
                        .text(`Project Name: ${projectHandler.getProjectName()} (  ${projectHandler.getProjectNickname()} | ID-${projectHandler.getProjectID()} )`)
                );
            }

            if (!projectHandler.getProjectNickname() && !projectHandler.getProjectName()) { getProjectInfo(fn.bind(this)) }
            else { fn() }
        }
        
        if(this.initialPresets && currentCompletionState == "preSubmit"){
            viewer.changeValueList(this.initialPresets);
        }

        var batch_param_constraint_text = "";
        viewer.fldTree.enumerate(function(param, node) {
            if (node.is_batch_fg) {
                if (batch_param_constraint_text.length) {
                    batch_param_constraint_text += "|";
                }
                batch_param_constraint_text += node.name + "
            }
        });

        var batch_param_elt = viewer.getElement("batch_param");
        if (batch_param_elt) {
            var batch_param_fld = batch_param_elt.fld;
            batch_param_fld.constraint_data = batch_param_constraint_text;
            viewer.redraw();
        }

        var batch_svc_elt = viewer.getElement("batch_svc");

        if(this.cloneMode){
            this.changeCmdPropSet(viewer,batch_svc_elt)
        }
        
        var url = document.location.href;
        if(url.indexOf("?cmd=")!=-1){
            url = url.substring(url.indexOf("?cmd=") + 5);
        } else {
            url = url.substring(url.indexOf("?"));
        }
        url = urlExchangeParameter(url, "id", "-");
        var cmd = docLocValue("cmd");
        var cmdMode = docLocValue("cmdMode");
        var batchMode = docLocValue("batchMode");
        if (isok(cmdMode)) cmd += "&cmdMode=" + cmdMode;
        if (parseBool(batchMode) && batch_svc_elt) {
            cmd += "&batchMode=true";
            viewer.changeElementValue("batch_svc", this.qpSvc);
        }
        viewer.changeElementValue("submitter", url);
        this.updateBatchFldTree(viewer);

        if(this.inputLoaded)
            this.inputLoaded (viewer,this);
        if(this.callbackLoaded)
            this.callbackLoaded (viewer,this);
    };

    this.changeCmdPropSet = function(viewer, elem) {
        var qpSvc = parseBool(elem.value) && elem.value != "single" ? "svc-batcher" : this.qpSvc;
        viewer.cmdPropSet = "?cmd=-qpProcSubmit&svc=" + qpSvc + (algoProcess.forceGroup ? "&forceGroup=1" : "");
        this.updateBatchFldTree(viewer);
    }
    
    this.readFromDocLoc=function(namearr)
    {
        for( var i=0; i<namearr.length ; ++ i) {
            par = docLocValue(namearr[i]);
            if( isok(par) ) {
                var o=new Object();
                o[namearr[i]]=par.split(",");
                this.setValueList(o);
            }
        }
    };

    this.onRecordChanged = function(viewer, elem, domElem, node) {
        if (elem.fld.name == "batch_svc") {
            this.changeCmdPropSet(viewer,elem)
        }
        if( this.inputChanged ) {
            this.inputChanged(viewer, elem, node);
        }
    };

    this.updateBatchFldTree=function(viewer)
    {
        if (this._in_updateFldPresets) {
            return;
        }
        this._in_updateFldPresets = true;

        var is_batch = this.isBatch();
        var need_redraw = false;
        ["batch_param", "batch_value"].forEach(function(fldname) {
            var is_optional_fg = this.isBatch() ? 0 : 1;

            if (viewer.fldPresets[fldname]) {
                var preset = viewer.fldPresets[fldname];
                var prev_is_optional_fg = preset.is_optional_fg;
                if (is_optional_fg !== prev_is_optional_fg) {
                    preset.is_optional_fg = is_optional_fg;
                    need_redraw = true;
                }
            } else {
                viewer.fldPresets[fldname] = { is_optional_fg: is_optional_fg };
                need_redraw = true;
            }

            var flds = viewer.fldTree.accumulate("node.name=='" + fldname + "'", "node");
            if (flds && flds.length) {
                var prev_is_optional_fg = flds[0].is_optional_fg;
                if (prev_is_optional_fg !== is_optional_fg) {
                    flds[0].is_optional_fg = is_optional_fg;
                    need_redraw = true;
                }
            }
        }, this);

        if (is_batch) {
            viewer.getValidateSeparatorCb = function(fld_name) {
                if(fld_name && verarr(algoProcess.getValue("batch_param", "array")).indexOf(fld_name) >= 0) {
                    return ';';
                } else {
                    return null;
                }
            };
        } else {
            viewer.getValidateSeparatorCb = function(fld_name) { return null; };
        }

        if (need_redraw) {
            viewer.redraw(undefined, true);
        }

        delete this._in_updateFldPresets;
    }

    this.getValue=function(name,which)
    {
        return vjDV[this.recViewerName].getElementValue(name,which);
    };

    this.setValueList=function(obj)
    {
        vjDV[this.recViewerName].changeValueList(obj);
    };

    this.setValue=function(name,value)
    {
        vjDV[this.recViewerName].changeElementValue(name,value);
    };

    this.onRecordAddedElement=function(viewer,elem)
    {
        if(this.callbackAddedElement)
            funcLink(this.callbackAddedElement,viewer,elem);
    };
    var callbackFullview = false;

    this.callbackDoneComputing = function (viewer, reqid, stat) {
        if(originalStat == "none" || originalStat == stat) return;

        if(that.doneComputing && (stat != "Killed" || stat != "ProgError" || stat != "SysError") && !that.isBatch()){
            callbackFullview = that.doneComputing(viewer,reqid,stat);
        }

        if (stat == "Done" && (originalStat == "Any" || originalStat == "Waiting" || 
            originalStat == "Processing" || originalStat == "Running" || originalStat == "Suspended")) 
        {
            if (that.isMode("batch") || (vjDV[that.recViewerName].getElement("batch_svc") && vjDV[that.recViewerName].getElement("batch_svc").value == that.qpSvc))
                return;
            if ($("#dialog").length) return;
            
            $("body").append(
                $(document.createElement("div"))
                    .attr("id", "dialog")
                    .attr("title", "Continue Dialog")
                    .append (
                            $(document.createElement("p")).text("The computation has completed. You can click 'Continue' to see the results or 'Cancel' to stay here.")
                    )
                );
            
            $("#dialog").dialog({
                modal: true,
                width: 500,
                buttons: {
                    Continue: function() {
                        $(this).dialog("close");
                        onContinueToResults();
                    },
                    Cancel: function() {
                       $(this).dialog("close");
                    }
                },
                open: function() {
                    $(this).closest(".ui-dialog")
                    .find(".ui-dialog-titlebar-close")
                    .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                    .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                }
            });
            
            var menuUl = $("#algoMenu");
            var whatNext = $(".whatNext");

            var tab="whatNext";

                $("#"+tab).append(
                        $(document.createElement("li"))
                        .append($(document.createElement("a"))
                                .attr({"cur-index":0})
                                .click(function(){
                                    onContinueToResults();
                                })
                                .text("Continue to Results")
                        )
                        .append($(document.createElement("i"))
                                .addClass("fa fa-angle-right")
                        )
                );
                
            whatNext.children("ul").toggle();
        }
        else if (stat == "Any" || stat == "Waiting" || 
        stat == "Processing" || stat == "Running" || stat == "Suspended")
        {
            currentCompletionState = "whileRunning";
            algoWidgetObj.iterateAlgoJSON (algoWidgetObj.optionsForPage.subTabs, "algoMenu");
            if (that.callbackProgressComputing)
                that.callbackProgressComputing (viewer,reqid,stat);
        }
        else if((stat == "Killed" || stat == "ProgError" || stat == "SysError")){
            vjDS["dsProgress"].reload(vjDS["dsProgress"].url, true);
            
            $("body").append(
                    $(document.createElement("div"))
                        .attr("id", "dialog")
                        .attr("title", "Error Dialog")
                        .append (
                                $(document.createElement("p")).text("There was an error in the computation. Please check the Progress tabs")
                        )
                    );
                
                $("#dialog").dialog({
                    modal: true,
                    width: 500,
                    buttons: {
                        OK: function() {
                           $(this).dialog("close");
                        }
                    },
                    open: function() {
                        $(this).closest(".ui-dialog")
                            .find(".ui-dialog-titlebar-close")
                            .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                            .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                        $(this).closest(".ui-dialog")
                            .find(".ui-dialog-titlebar")
                            .addClass("error-header");
                    }
                });
            originalStat = "none";
            return;
        }
        else if (!that.isBatch() && (stat == "Done" || stat == "Killed" || stat == "ProgError" || stat == "SysError"))
            onContinueToResults();
        
        if (!originalStat)
            originalStat = stat;
    };
    
    function onContinueToResults (){
        $(".whileRunning").children("ul").toggle();
        currentCompletionState = "computed";
        $("."+currentCompletionState).children("ul").toggle();
        $(".computed").removeAttr("style");
        
        vjDS["ds"+toolBar].reload("static:

        if ($.getAlgoViewManager().options.closeHelp){
            setTimeout(function() {
                $("[data-id='right']").children("[title='Close']").click();
            }, 5000);
        }

        if (callbackFullview) {
            return;
        } else if(algoProcess.callbackFullview){
            algoProcess.callbackFullview();
            return;
        }

        function archiveCallback(viewer, node, row) {
            var extension = node.value.substr(node.value.lastIndexOf('.') + 1);
            var url = 'qpbg_http:
                     + '&filename=' + encodeURIComponent(node.value) + '&cgi_dstname=' + encodeURIComponent(node.value)
                     + '&ext=' + encodeURIComponent(extension) + '&projectID=' + projectHandler.getProjectID();
            
            if( node.all_files ) {
                alert('Function will be supported in a future release.\nYou can download [All files] and then upload the downloaded zip file on Home page.');
            } else if(extension === 'sam' || extension === 'bam' || extension === 'blast_out'){
                vjRefArchive(url,node.value);
                alert ('Your selected item is being ingested. You can monitor the progress from within data loading tab');
            }else {
                var url = 'qpbg_http:
                     + '&filename=' + encodeURIComponent(node.value) + '&cgi_dstname=' + encodeURIComponent(node.value) + '&arch_dstname=' + encodeURIComponent(node.value)
                     + '&ext=' + encodeURIComponent(node.value.substr(node.value.lastIndexOf('.') + 1)) + '&projectID=' + projectHandler.getProjectID();
                vjDS.dsVoid.reload(url, true);
                alert ('Your selected item is being ingested. You can monitor the progress from within data loading tab');
            }
        };

        function downloadCallback(viewer, node) {
            var url;
            if( node.all_files ) {
                url = "qpbg_http:
                    + node.id + "-results&function=objFiles2&files2mask=*&objs=" + node.id + '&projectID=' + projectHandler.getProjectID();
            } else {
                if( algoProcess.allDownUrlModification ) {
                    url = algoProcess.allDownUrlModification(viewer, node);
                } else {
                    url = 'http:
                }
            }
            vjDS.add('', 'dsDownSrc', 'static:
            vjDS['dsDownSrc'].reload(url, true, {loadmode: 'download', saveas : '-o' + node.id + '-all.zip', title: '"preparing [All Files] for download, please wait"'});
        };

        vjDS.add("Retrieving HTML file" , "dsHTML" , "");
        vjDS.dsHTML.known_safe_url = true
        vjDS.dsHTML.register_callback(function( _ , data ) {
            let regex = /\.([^\.]+)$/;
            let found = vjDS.dsHTML.filename.match(regex);
            if( found &&  found.length > 1 ) {
                let preview = null;
                if( found[1] == "html" ) {
                    preview = new HTMLpreview({ data: data, filename: vjDS.dsHTML.filename });
                } else if( found[1] == "txt" || found[1] == "log" ) {
                    preview = new TextPreview({ data: data, filename: vjDS.dsHTML.filename });
                } else if( found[1] == "pdb" ) {
                    preview = new PDBpreview({ data: vjDS.dsHTML.dataurl, filename: vjDS.dsHTML.filename });
                }
                if( preview ) {
                    preview.init();
                }
            }
        })

        function htmlPreviewCallback( viewer , node , _ , _ , col , event ) {
            if(col.name === 'preview' && ( !node.preview || !node.preview === "" ) ) return
            vjDS.dsHTML.filename = node.value
            vjDS.dsHTML.reload(`http:
        }

        node = {
            _type : algoProcess.svcProcType,
            id : docLocValue("id")
        };
        algoWidgetObj.iterateAlgoJSON (algoWidgetObj.optionsForPage.subTabs, "algoMenu")
        vjHO.fullview(node._type, node, $.getAlgoViewManager().options.jsonForPage.subTabs.results);
        
        if (algoWidgetObj.noAllDownloadsTab) {
            return;
        }

        if (!vjDS.dsAllDownloads) {
            vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
        }
       
        algoWidgetObj.addTabs([{
            tabId: 'downloadAllFiles',
            tabName: "Available Files to Download",
            position: {posId: 'maxCenter', top:'0', bottom:'100%', left:'20%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjTableView",
                dataViewerOptions:{
                    parsemode: vjTable_hasHeader,
                    data: "dsAllDownloads" ,
                    formName: formName,
                    selectCallback: function (viewer,node,ir,col){
                        if (col>1)
                            return;
                        if (node.url) {
                            document.location = node.url;
                        } else {
                            document.location = "?cmd=objFile&ids=" + node.id + "&filename=" + encodeURIComponent(node.value) + '&projectID=' + projectHandler.getProjectID();
                        }
                    },
                    iconSize:24,
                    cols:[
                        { name: new RegExp(/.*/), hidden:true },
                        { name: "name", hidden:true, order:0},
                        { name: "pretty_name", title: "name", hidden: false , order:3},
                        { name: "description", hidden: true },
                        { name: "icon", hidden: true },
                        { name: "size", hidden: false, type: "bytes" , order:5},
                        { name: "preview", order:2.5, hidden: true, url: htmlPreviewCallback },
                        { name: "archive", order:2, hidden: false, url: archiveCallback },
                        { name: "down", order:1, width: 150 , hidden: false, url: downloadCallback },
                    ],
                    appendCols : [{header: {name: "down", title: "download"}, cell: "<img src='img/download.gif' height='24' width='24' />", width: '150',},
                                  {header: {name: "archive"}, cell: "<img src='img/upload.gif' height='24' width='24' />"},
                                  {header: {name: "pretty_name", title: "name"}}, 
                                  {header: {name: "description"}},
                                  {header: {name: "preview", title:"preview"}, cell: ""}
                                ],
                    bgColors:['#f2f2f2','#ffffff'],
                    getPrettyName: algoProcess.prettyFileName ? algoProcess.prettyFileName : function(filename) { return filename; },
                    getDescription: algoProcess.allDownloadsGetDescription ? algoProcess.allDownloadsGetDescription : function (filename){
                            return filename.replace(/[^.]*\./, "").toUpperCase() + " file";
                    },
                    precompute: function(viewer, tbl, ir) {
                        var node = tbl.rows[ir];
                        if (!node.value || node.name != "_file") {
                            node.hidden = true;
                            return;
                        }
                        if (node.value.indexOf("_.") == 0 ) {
                            node.hidden = true;
                            return;
                        }
                        if (!node.pretty_name) {
                            node.pretty_name = viewer.getPrettyName(node.value);
                        }
                        node.cols[node.cols.length-2] = node.pretty_name;
                        if (!node.description) {
                            node.description = viewer.getDescription(node.value);
                        }
                        node.cols[node.cols.length-1] = node.description;

                    },
                    preEditTable: function(viewer) {
                        viewer.tblArr.sortTbl(0, 0, function(a, b) {
                            return cmpCaseNatural("" + a.name + a.value, "" + b.name + b.value);
                        });
                        if( viewer.tblArr.rows.length > 1 ) {
                            var tot_size = 0;
                            let tot_html = 0;
                            let q = 0;
                            for(var i = 0; i < viewer.tblArr.rows.length; ++i) {
                                let row =  viewer.tblArr.rows[i]
                                if ( !row.hidden ) {
                                    var sz = row.size;
                                    if( !isNaN(sz) ) {
                                        tot_size += Number(sz);
                                        q += 1;
                                    }
                                    let regex = /\.(html|txt|log|pdb)$/;
                                    let found = row.value.match(regex);
                                    if( found ) {
                                        tot_html++ 
                                        let button = { 
                                            name: `preview`, 
                                            classes:'hv-btn-link hv-btn-link-tbl', 
                                        }
                                        row.preview = createFakeButton(button)
                                    }
                                }
                            }
                            if(tot_size > 0 && q > 1 ) {
                                var v = Object.assign({}, viewer.tblArr.rows[1]);
                                v.cols =[...viewer.tblArr.rows[1].cols];
                                v.pretty_name = '[All files]';
                                v.value = v.pretty_name;
                                v.hidden = false
                                v.preview = null;
                                v.cols[3] = v.pretty_name;
                                v.cols[5] = v.pretty_name;
                                v.size = tot_size;
                                v.cols[4] = undefined;
                                v.all_files = true;
                                viewer.tblArr.rows.splice(0, 0, v);
                            }
                            if(tot_html > 0) {
                                viewer.cols.forEach(function(col, i ){
                                    if( col.name === 'preview') { 
                                        col['hidden'] = false;
                                    }
                                })
                            }
                        }
                    }
                }
            }
        }], "results");

        if(that.tabs_to_move && that.tabs_to_move.hasOwnProperty('downloadAllFiles')){
            algoWidgetObj.moveTab("downloadAllFiles", that.tabs_to_move.downloadAllFiles);
        }
        
        if (that.noResultViewers)
        {
            algoWidgetObj.openTab("downloadAllFiles");
        }
    }


    this.onSubmitRequest=function()
    {
        if (algoProcess.submitCallback)
            algoProcess.submitCallback();
        
        tThis = this
        
        const onSubmit = function (subid) {
            if(subid > -1) {
               vjDV[tThis.recViewerName].changeElementValue("submission_project",subid); 
            }else {
              vjDV[tThis.recViewerName].changeElementValue("submission_project",'');
            }
            tThis.submit()
        }
                
        let submission_project_dialog = new ProjectListDialog({
            submit: onSubmit, 
            cur_submission_project: vjDV[tThis.recViewerName].getElementValue('submission_project')
        })

    };

    this.validateSubmit = function(allElements)
    {
        for (var i = 0; i < allElements.length; i++)
        {
            var elem = allElements[i];

            if (elem.children && elem.children.length > 0 && elem.name.indexOf(".system.") < 0)
            {
                var returned = this.validateSubmit(elem.children);

                if (!returned)
                    return false;
            }

            if (((elem.fld.is_optional_fg == 0 && !elem.fld.is_readonly_fg && !elem.fld.is_hidden_fg) && (!elem.children || elem.children.length < 1)) && (elem.value == "" && parseFloat(elem.value) != 0) && elem.name.indexOf(".system.") < 0)
                return false;
        }

        return true;
    };

    this.submit=function(cbFunc,cnts)
    {
        var proceed=true;
        if(cbFunc)this.callbackSubmited=cbFunc;
        if (!this.viewer && this.recViewerName)
            this.viewer = vjDV[this.recViewerName];

        this.viewer.saveValues(null, "later", "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");

        if (!this.validateSubmit(vjDV[parametersDiv].nodeTree.root.children)){
                if ($("#dialog").length > 0){
                    $("#dialog").empty();
                }
                else{
                    $("body").append(
                            $(document.createElement("div"))
                                .attr("id", "dialog")
                                .attr("title", "Error Dialog")
                        );
                }
                $("#dialog").append (
                                    $(document.createElement("p")).text("You have not entered values for all of the required fields")
                            );
                
            $("#dialog").dialog({
                modal: true,
                width: 500,
                buttons: {
                    OK: function() {
                       $(this).dialog("close");
                    }
                },
                open: function() {
                    $(this).closest(".ui-dialog")
                        .find(".ui-dialog-titlebar-close")
                        .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                        .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                    $(this).closest(".ui-dialog")
                        .find(".ui-dialog-titlebar")
                        .addClass("error-header");
                }
            });
                
            return false;
        }
        else{
            $('.toSubmitBtn').attr("disabled", "disabled");
            this.viewer.submitAfterSave(null, "later", "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");
            
            if ($("#dialog").length > 0){
                    $("#dialog").empty();
            }
            else{
                $("body").append(
                        $(document.createElement("div"))
                            .attr("id", "dialog")
                            .attr("title", "All Set")
                    );
            }
        
            $("#dialog").append (
                $(document.createElement("p")).text("Your computation has been submitted, please wait for the page to refresh")
            );
                
            $("#dialog").dialog({
                modal: true,
                width: 500,
                buttons: {
                    OK: function() {
                       $(this).dialog("close");
                    }
                },
                open: function() {
                    $(this).closest(".ui-dialog")
                        .find(".ui-dialog-titlebar-close")
                        .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                        .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                    $(this).closest(".ui-dialog")
                        .find(".ui-dialog-titlebar");
                }
            });
        }
    };

    this.onRedirectProcSubmitted=function (param, text )
    {

        var reqID=0, objID=0;
        var nums=isok(text) ? text.split(",") : new Array();
        if(nums.length>=2) {
               reqID=parseInt(nums[0]);
               objID=parseInt(nums[1]);
        }
        if(!reqID || !objID) {
            alert("Error: could not submit the computation request!\n"+text);
            var a = gObject( this.dvname+"SubmitterInput");
            if( a) { a.disabled = false; }
            return ;
        }
        this.loadedID=objID;

        if(this.callbackSubmited) {
            funcLink(this.callbackSubmited,this,this.viewer);
        }
        else {
            document.location=urlExchangeParameter(document.location.href, "id", this.loadedID);
        }
    };


};



function AlgoWidgetHelp (optionsForPage)
{
    this.optionsForPage = optionsForPage;

    this.render = function()
    {
        return $("#menuDiv").html();
    }

    this.findInJson = function (options, field, value)
    {
        if (options.length < 1)
            options = this.optionsForPage.subTabs;

        for ( tab in options ) {
              var curNode = options[tab];

              for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                  var curView = curNode.pageTabChildren[ii];

                  for (smt in curView){
                      if (smt == field && curView[smt] == value)
                          return curView;
                  }

                  if (curView.subTabs){
                      var toReturn = this.findInJson (curView.subTabs, field, value);
                      if (toReturn)
                          return toReturn;
                  }
              }
        }
        return false;
    };
    
    this.deleteFromJson = function (options, field, value)
    {
        if (options.length < 1)
            options = this.optionsForPage.subTabs;

        for ( tab in options ) {
              var curNode = options[tab];

              for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                  var curView = curNode.pageTabChildren[ii];

                  for (smt in curView){
                      if (smt == field && curView[smt] == value){
                          if (curNode.pageTabChildren instanceof Array)
                              curNode.pageTabChildren.splice(subView, 1);
                          else
                              delete curNode.pageTabChildren[ii];
                          return true;
                      }
                  }

                  if (curView.subTabs){
                      var toReturn = this.findInJson (curView.subTabs, field, value);
                      if (toReturn) return toReturn;
                  }
              }
        }
        return false;
    };

    this.openTab = function (curTab, preObj, fromIterateJSON)
    {
        var oThis = this;
        
        if (typeof curTab == "string")
            curTab = this.findInJson(this.optionsForPage.subTabs, "tabId", curTab);

        if (curTab.preConstructor)
        {
            var objToCreate = eval(curTab.preConstructor.dataViewer);
            preObj = new objToCreate (curTab.preConstructor.dataViewerOptions);
        }

        if(curTab.tabDependents) {
            for (var i=0; i<curTab.tabDependents.length; ++i ){
                var fnd=this.findInJson(optionsForPage.subTabs, "tabId", curTab.tabDependents[i]);
                if(!fnd || fnd.constructedAlready) continue;
                this.openTab(fnd, preObj);
                fnd.preconstructed = true;
            }
        }

        var viewerOptions = {
            preload: curTab.preload
        };
        
        if (curTab.multiView){
            viewerOptions = curTab.viewerConstructor;
        }
        else{
            var preObjPos = verarr(curTab.viewerConstructor.preObjPos);
            if (curTab.viewerConstructor.preObjPos == 0)
                preObjPos = [0];
    
            if (curTab.viewerConstructor && curTab.viewerConstructor.dataViewer)
            {
                $.extend(viewerOptions, {
                        dataViewer: curTab.viewerConstructor ? curTab.viewerConstructor.dataViewer : null,
                        dataViewerOptions: curTab.viewerConstructor ? curTab.viewerConstructor.dataViewerOptions : null
                });
            }
            else if (curTab.viewerConstructor &&
                    (typeof curTab.viewerConstructor.preObjPos == "object" || curTab.viewerConstructor.preObjPos > -1 || typeof curTab.viewerConstructor.preObjPos == "string")
                    && preObj)
            {
                var toPut = [];
    
                for (var i = 0; i < preObjPos.length; i++)
                {
                    var item = preObjPos[i];
                    if (typeof item == "number" && item < 0)
                        continue;
                    toPut.push(preObj[item]);
                }
                $.extend(viewerOptions, {instance: toPut});
            }
            else if (curTab.viewerConstructor && curTab.viewerConstructor.instance)
                $.extend(viewerOptions, {instance: curTab.viewerConstructor.instance});
            else if (curTab.viewerConstructor.viewName)
                viewerOptions = curTab.viewerConstructor.dataViewerOptions
        }
        
        var manager = $.getAlgoViewManager();
        if (!manager.show(curTab.tabId))
        {
            manager.append({
                layout: {
                    items:[
                    {
                        id: curTab.position.posId,
                        top: curTab.position.top,
                        left: curTab.position.left,
                        right: curTab.position.right,
                        bottom: curTab.position.bottom,
                        toggler: curTab.toggler,
                        allowMaximize: true,
                        tabs:{
                            items: [{
                                overflow: 'auto',
                                allowHide: true,
                                active: curTab.inactive ? false : true,
                                name: curTab.tabId,
                                title: curTab.tabName,
                                view: {
                                    name: curTab.viewerConstructor.viewName ? curTab.viewerConstructor.viewName : (curTab.multiView ? 'dataviews' : 'dataview'),
                                    options: viewerOptions
                                }
                            }]
                        }
                    }]
                }
            });
        }

        curTab.constructedAlready=true;
        curTab.visible=true;

        if (curTab.showSubmitButton && currentCompletionState == "preSubmit" && !curTab.submitCreated)
        {
                var buttons = (curTab.showSubmitButton);
                
                $("#"+curTab.tabId+"-tab").append (curTab.showSubmitButton[curTab.tabId]);
            
            curTab.submitCreated=true;
        }

        if (!fromIterateJSON)
            $(document).trigger("tab-active", {index: "", name: curTab.tabId, area: ""});
    };


    this.closeTab = function(curTab)
    {
        var tmp=true;
        if (typeof curTab == "string")
            tmp = this.findInJson(optionsForPage.subTabs, "tabId", curTab);
        else
            tmp = curTab;
        var manager = $.getAlgoViewManager();

        if (!tmp)
            manager.hide(curTab);

        manager.hide(tmp.tabId);
        tmp.constructedAlready = false;
        tmp.visible=false;
        tmp.submitCreated=false;
        tmp.inactive = true;

        $("[tab-id='"+tmp.tabId+"']").removeClass("activeSelection");
    };

    this.clickFunc = function (someVar)
    {
        var tabId = $(someVar).attr('tab-id');
        var manager = $.getAlgoViewManager();

        var lookedUp = that.findInJson(jsonOptions.subTabs, "tabId", tabId);

        if (!lookedUp)
            return;

        that.openTab(lookedUp);
        if (lookedUp.callback) lookedUp.callback(lookedUp);
        return false;
    };


    this.iterateAlgoJSON = function (options, appendTo)
    {
        if (options.length < 1)
            return "";

        var toReturn = "";
        var whereToAppend = $("#"+appendTo);

        for ( var tab in options )
        {
              var curNode = options[tab];
              var visibleDuring = "";

              for (var i = 0; curNode.visibleDuring && i < curNode.visibleDuring.length; i++)
                  visibleDuring += " " + curNode.visibleDuring[i];

              if (curNode.visibleDuring && (curNode.visibleDuring.indexOf(currentCompletionState) >= 0 || curNode.openUp) < 0 && $("#"+tab).length == 0)
              {
                  whereToAppend.append(
                      $(document.createElement("li"))
                        .addClass(visibleDuring)
                        .attr({'style': 'display:none;'})
                        .append($(document.createElement("span"))
                                .click(function(){
                                    $(this).parent().children('ul').toggle();
                                })
                                .append($(document.createElement("a"))
                                        .text(curNode.pageTabName)
                                )
                                .append($(document.createElement("i"))
                                        .addClass("fa fa-angle-right")
                                )
                        )
                        .append($(document.createElement("ul"))
                                .attr({"id": tab})
                        )
                  );
              }
              else if ($("#"+tab).length == 0)
              {
                  whereToAppend.append(
                      $(document.createElement("li"))
                        .addClass(visibleDuring)
                        .append($(document.createElement("span"))
                                .append($(document.createElement("a"))
                                        .text(curNode.pageTabName)
                                )
                                .click(function(){
                                    $(this).parent().children('ul').toggle();
                                })
                                .append($(document.createElement("i"))
                                        .addClass("fa fa-angle-right")
                                )
                        )
                        .append($(document.createElement("ul"))
                                .attr({"id": tab})
                        )
                  );
              }

              $("#"+tab).empty();
              
              if (curNode.visibleDuring && (curNode.visibleDuring.indexOf(currentCompletionState) < 0 ) &&
                      currentCompletionState != "computed"){
                  $("#"+tab).parent().addClass("slideout-menu-inactive");
                  continue;
              }

              for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                  var curView = curNode.pageTabChildren[ii];
                  var toTriggerTabOpen = false;

                  if (!curView.visible && ((curView.autoOpen && curView.autoOpen.indexOf(currentCompletionState) > -1) || curView.openUp))
                      this.openTab (curView, -1, true);
                  else if (!curView.visible || !(curView.autoOpen && curView.autoOpen.indexOf(currentCompletionState) > -1))
                      this.closeTab (curView);
                  toTriggerTabOpen = curView.inactive ? !curView.inactive : true;

                  if (curView.visible && !curView.preconstructed || (!curView.constructedAlready))
                  {
                      if(curView.tabDependents) {
                          for (var i = 0; i < curView.tabDependents.length; ++i ){
                            var fnd = this.findInJson(optionsForPage.subTabs, "tabId", curView.tabDependents[i]);
                            $("#"+tab).append(
                                    $(document.createElement("li"))
                                      .append($(document.createElement("a"))
                                              .attr({"tab-id": fnd.tabId})
                                              .click(function(){ return that.clickFunc($(this)); })
                                              .text(fnd.tabName)
                                      )
                                      .append($(document.createElement("i"))
                                              .addClass("fa fa-angle-right")
                                      )
                                );
                        }
                      }
                      
                      if($("[tab-id='" + curView.tabId +"']").length == 0){
                          $("#"+tab).append(
                              $(document.createElement("li"))
                                .append($(document.createElement("a"))
                                        .attr({"tab-id": curView.tabId})
                                        .click(function(){ return that.clickFunc($(this)); })
                                        .text(curView.tabName)
                                )
                                .append($(document.createElement("i"))
                                        .addClass("fa fa-angle-right")
                                )
                          );
                      }
                  }

                  if (toTriggerTabOpen)
                      $(document).trigger("tab-active", {index: "", name: curView.tabId, area: ""});

                  if (curView.subTabs)
                      toReturn += this.iterateAlgoJSON (curView.subTabs, tab);
              }
        }

        return toReturn;
    };

    this.addTabs = function (whatToAdd, whereToAdd)
    {
        var options = $.getAlgoViewManager().options;

        var that = this;
        var actualObjLoc = options.jsonForPage;
        
        if (whereToAdd instanceof Array){
            actualObjLoc = options.jsonForPage.subTabs;
            for (var i = 0; i < whereToAdd.length-2; i++){
                if (!actualObjLoc[whereToAdd[i]]){
                    var key = whereToAdd[i];
                    actualObjLoc[key]={};
                }
                if (!actualObjLoc[whereToAdd[i]].pageTabChildren){
                    var key = whereToAdd[i];
                    if (!actualObjLoc[key].subTabs) actualObjLoc[key].subTabs = {};
                    actualObjLoc =  actualObjLoc[key].subTabs;
                    break;
                }
                
                actualObjLoc = actualObjLoc[whereToAdd[i]].pageTabChildren;
            }
            
            if (!actualObjLoc[whereToAdd[whereToAdd.length-2]]) actualObjLoc[whereToAdd[whereToAdd.length-2]] = {};
            actualObjLoc = actualObjLoc[whereToAdd[whereToAdd.length-2]];
            actualObjLoc.pageTabName = whereToAdd[whereToAdd.length-1];
            if (!actualObjLoc.pageTabChildren) actualObjLoc.pageTabChildren = [];
        }
        else if (!options.jsonForPage.subTabs[whereToAdd]){
            return false;
        }
        else{
            actualObjLoc = options.jsonForPage.subTabs[whereToAdd];
        }

        $(verarr(whatToAdd)).each(function (index, nObj){
            if ((!nObj.tabId && !nObj.tabName && !nObj.position &&
                    !nObj.viewerConstructor) || that.existsTab(nObj.tabId))
                return false;

            if (nObj.tabOrder != null && nObj.tabOrder <= actualObjLoc.pageTabChildren.length){
                actualObjLoc.pageTabChildren.splice(nObj.tabOrder, 0, nObj);
            }
            else {
                actualObjLoc.pageTabChildren.push(nObj);
            }
            
        });

        if (whereToAdd instanceof Array)
            $("#"+whereToAdd[0]).empty();
        else
            $("#"+whereToAdd).empty();
        var appendToMenu = this.iterateAlgoJSON (options.jsonForPage.subTabs, "algoMenu");
        if (appendToMenu == "")
            return false;
    };

    this.removeTabs = function (whatToRemove, whereToRemove)
    {
        if (!this.optionsForPage.subTabs[whereToRemove])
            return false;

        $(verarr(whatToRemove)).each(function(index, rem){
            for (var i = 0; i < that.optionsForPage.subTabs[whereToRemove].pageTabChildren.length; i++)
            {
                if (that.optionsForPage.subTabs[whereToRemove].pageTabChildren[i].tabId == rem)
                {
                    $("[tab-id='"+rem+"']").remove();
                    that.closeTab(that.optionsForPage.subTabs[whereToRemove].pageTabChildren[i]);
                    that.optionsForPage.subTabs[whereToRemove].pageTabChildren.splice(i,1);
                }
            }
        });

        return true;
    };
    
    this.createJustSideElem = function(descrJson){
        
        $("#"+descrJson.appendTo).append(
                $(document.createElement("li"))
                  .append($(document.createElement("a"))
                          .attr({
                              "tab-id": descrJson.name,
                              "href": descrJson.url,
                              "target": "_blank",
                              "rel": "noopener noreferrer"
                              })
                          .text(descrJson.title)
                  )
                  .append($(document.createElement("i"))
                          .addClass("fa fa-angle-right")
                  )
            );
    }

    this.existsTab = function (lookForTabId, where)
    {
        if (!where)
            where = $.getAlgoViewManager().options.jsonForPage.subTabs;

        for ( var tab in where )
        {
            var curNode = where[tab];

            for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                var curView = curNode.pageTabChildren[ii];
                if (curView.tabId == lookForTabId)
                    return curView;

                if (curView.subTabs)
                {
                    var ret = this.existsTab (lookForTabId, curView.subTabs);
                    if (ret != false)
                        return ret;
                }
            }
        }

        return false;
    };

    this.createPopup = function(config, afterInit)
    {
        let hasForm = document.forms[modalFormName];
        if(!this.buildModal && hasForm !== undefined){
            let modalForm = document.getElementById(modalFormName);
            $(modalForm).append(this.buildModalWindow());
            this.buildModal = true;
        }
        
        if (!this.buildModal)
        {
            var modalForm = document.createElement("form").setAttribute("id", modalFormName);
            $(modalForm).append(this.buildModalWindow());
            $("body").append(modalForm);
            this.buildModal = true;
        }

         $('#testModalWindow').on('shown.bs.modal', function (e) {
             $('#testModalLayout').layoutmanager({
                 type: 'horizontal',
                 saveState: false,
                 config: config,
                 _onAfterInit: afterInit
             });
         });
         
         $('#testModalWindow').modal('show');
    }

    this.buildModalWindow = function()
    {
        var wnd = $(document.createElement('div'))
                        .addClass('modal fade')
                        .attr({
                            id: 'testModalWindow',
                            tabindex: -1,
                            role: "dialog",
                            'aria-labelledby': "testModalLabel"
                        })
                        .append(
                            $(document.createElement('div'))
                                .addClass('modal-dialog modal-lg')
                                .attr({ role: "document" })
                                .append(
                                    $(document.createElement('div'))
                                        .addClass('modal-content')
                                        .append(
                                            $(document.createElement('div'))
                                                .addClass('modal-header')
                                                .append(
                                                    $(document.createElement('button'))
                                                        .addClass('close')
                                                        .attr({
                                                            type: "button",
                                                            'data-dismiss': "modal",
                                                            'aria-label': "Close"
                                                        })
                                                        .append(
                                                            $(document.createElement('span'))
                                                                .attr({ 'aria-hidden': "true" })
                                                                .html('&times;')
                                                        )
                                                )
                                                .append(
                                                    $(document.createElement('h4'))
                                                        .addClass('modal-title')
                                                        .attr({ id: 'testModalLabel' })
                                                        .text('Details')
                                                )

                                        )
                                        .append(
                                            $(document.createElement('div'))
                                                .addClass('modal-body')
                                                .append(
                                                    $(document.createElement('div'))
                                                    .attr({ id: 'testModalLayout' })
                                                )
                                        )
                                )
                        )
        return wnd;
    }

    this.moveTab = function (tabName, tabPlace, tabTime)
    {
         var curTab = this.findInJson(this.optionsForPage.subTabs, "tabId", tabName);
         if (!curTab) return;
         curTab.position.top = tabPlace.top;
         curTab.position.left = tabPlace.left;
         curTab.position.right = tabPlace.right;
         curTab.position.bottom = tabPlace.bottom;

         var manager = $.getAlgoViewManager();
         manager.moveArea(curTab.position.posId, tabPlace, tabTime);
    }
    
    this.setAlgoTitle = function(nTitle){
        $("#titleName").empty();
        $("#titleName").append($(document.createElement("h2")).text(nTitle));
    }

    var manager = $.getAlgoViewManager();

    $("body").append(
            "<div class='slideout-menu' id='menuDiv'>" +
                "<div id='titleName'><h2>" + this.optionsForPage.algorithmTitle + "</h2></div><ul id='algoMenu'>" +
                "</ul>" +
            "</div>");

    var listToAppend = this.iterateAlgoJSON (this.optionsForPage.subTabs, "algoMenu");
    parametersDiv = this.optionsForPage.parametersDiv;
    var that = this;
    this.buildModal = false;
};


function setUpJsonForAlgo (algoTitle, algoObj, paramsPos)
{
    gUserLoginAccess();
    
    var batchParams = ["batch", "batch_ignore_errors"];
    if (currentCompletionState == "whileRunning" || currentCompletionState == "computed") {
        batchParams.push("batch_children_proc_ids");
    }
    var moveParams=[
        { ds:"dsSystemParams", params: ["comment","system"]},
        { ds:"dsBatchParams", params: batchParams },
        { ds:"dsInputParams", params: algoProcess.visibleParameters }
    ];
    if (algoProcess.moveParams){
        moveParams = moveParams.concat(algoProcess.moveParams);
    }
    
    var dsArray=[
        {name: "dsAlgoSpec", url: "http:
        {name: "dsAlgoVals", url: "http:
        {name: "dsProgress", url: "http:
        {name: "dsHelp", url: "http:
    ];


    var batchInGeneral = false;
    for(i=0; i<moveParams.length; ++i)
    {
        var t="";
        for( var ir=0; moveParams[i].params && ir < moveParams[i].params.length; ++ir)
        {
            t+="<span id='RV-"+moveParams[i].params[ir]+"'></span>";
            if (moveParams[i].params[ir] == "batch-svc")
                batchInGeneral = true;
        }
        dsArray.push( {name: moveParams[i].ds, url: "static:
    }
    if (!batchInGeneral){
        for (var i = 0; i < dsArray.length; i++)
            if (dsArray[i].name == "dsBatchParams")
                dsArray[i].url += "<span id='RV-batch_svc'></span>";
    }

    var batchFldPreset={batch_svc:{constraint:'choice+', constraint_data:'single
    var submitButtons = algoProcess.submitButtons ? algoProcess.submitButtons : {};
    if (!algoProcess.submitButtons){
            var arr = ["general", "system", "batch", 'advanced'];
            
            for (var ii = 0; ii < arr.length; ii++){
                var tmp = $(document.createElement("button"))
                    .attr({
                        style: "visibility:hidden;",
                        type: "button",
                        name: "BUTTON-submitter",
                        size: 20
                    })
                    .data("tab", arr[ii])
                    .addClass("myButton submitterInputButton")
                    .text(algoProcess.submitButtonName ? algoProcess.submitButtonName : "SUBMIT")
                    .on("click", function(){
                            if (!algoProcess.onSubmitRequest())
                                event.stopImmediatePropagation();
                    });
                submitButtons[arr[ii]] = tmp;
            }                            
    }
    
    var propHiveId = (algoProcess.svcRecViewer ?algoProcess.svcRecViewer: algoProcess.svcProcType);
    var submitCmd = "-qpProcSubmit";
    if (algoProcess.overwriteID && algoProcess.loadedID) {
        propHiveId = algoProcess.loadedID;
        submitCmd = "-qpProcReSubmit";
    }
    var setUpForPage = {
        algorithmTitle: algoTitle,
        parametersDiv: 'DV_Parameter_view',
        dataSourceArray: dsArray,
        subTabs: {
            parameters: {
                visibleDuring: ["preSubmit", "whileRunning"],
                pageTabName: "Parameters",
                pageTabChildren: [
                {
                    tabId: 'advanced',
                    tabName: "Advanced",
                    showSubmitButton: submitButtons,
                    tabDependents: ["general","system", "batch"],
                    inactive: true,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjRecordView',
                        dataViewerOptions: {
                            divName: 'DV_Parameter_view',
                            kind:"valgoProcess",
                            data: algoProcess.loadedID ? [ "dsAlgoSpec", "dsAlgoVals", "dsCurrentUserSpecLoaded" ] : ["dsAlgoSpec", "dsVoid", "dsCurrentUserSpecLoaded" ],
                            hiveId: propHiveId,
                            objType:  algoProcess.svcRecViewer ? algoProcess.svcRecViewer : algoProcess.svcProcType  ,
                            cmdPropSet:makeCmdSafe(submitCmd)+"&svc="+algoProcess.qpSvc + (algoProcess.forceGroup ? "&forceGroup=1" : ""),
                            readonlyMode: algoProcess.modeActive ? false : true,
                            callbackRendered : "function:vjObjFunc('onRecordLoaded','"+algoProcess.objCls+"')",
                            onChangeCallback : "function:vjObjFunc('onRecordChanged','"+algoProcess.objCls+"')",
                            onAddElementCallback : "function:vjObjFunc('onRecordAddedElement','"+algoProcess.objCls+"')",
                            accumulateWithNonModified:true,
                            cloneMode:  algoProcess.ID && (algoProcess.ID[0] == "-" || algoProcess.ID < 0) ? true : false,
                            showReadonlyInNonReadonlyMode: algoProcess.showReadonlyInNonReadonlyMode,
                            constructionPropagateDown:10,
                            autoexpand: algoProcess.autoexpand ?  algoProcess.autoexpand : 1,
                            showRoot:false,
                            autoDescription:0,
                            autoStatus:3,
                            fldPresets: algoProcess.fldPresets ? cpyObj(batchFldPreset,algoProcess.fldPresets) : batchFldPreset,
                            RVtag: "RV",
                            formObject:document.forms[formName],
                            isok:true
                        }
                    },
                    autoOpen: ["preSubmit", "whileRunning"],
                    preload: true
                },
                {
                    tabId: 'general',
                    tabName: "General",
                    showSubmitButton: submitButtons,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjHTMLView',
                        dataViewerOptions: {
                            data: "dsInputParams"
                        }
                    },
                    preload: true,
                    inactive: false,
                    autoOpen: ["preSubmit", "whileRunning"]
                },
                {
                    tabId: 'system',
                    tabName: "System",
                    showSubmitButton: submitButtons,
                    inactive: true,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjHTMLView',
                        dataViewerOptions: {
                            data: "dsSystemParams"
                        }
                    },
                    preload: true,
                    autoOpen: ["preSubmit", "whileRunning"]
                },
                {
                    tabId: 'batch',
                    tabName: "Batch",
                    showSubmitButton: submitButtons,
                    inactive: true,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjHTMLView',
                        dataViewerOptions: {
                            data: "dsBatchParams"
                        }
                    },
                    preload: true,
                    autoOpen: ["preSubmit", "whileRunning"]
                }]
            },
            progress:
            {
                visibleDuring: ["whileRunning"],
                pageTabName: "Progress",
                pageTabChildren: [
                    {
                        tabId: 'main',
                        tabName: "Main Progress",
                        position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'20%', right:'75%'},

                        viewerConstructor: {
                            dataViewer: 'vjProgress2Control',
                            dataViewerOptions: {
                                data: "dsProgress",
                                width: "100%",
                                newViewer: true,
                                dontRefreshAll: true,
                                formName: formName,
                                id: algoObj.ID,
                                doneCallback: algoProcess.callbackDoneComputing
                            }
                        },
                        autoOpen: ["whileRunning"]
                    }
                ]
            },
            results: {
                visibleDuring: ["computed"],
                pageTabName: "Results",
                pageTabChildren: []
            }
        }
    };

    if (algoObj.tabsToAdd && algoObj.tabsToAdd.length > 0)
    {
        for (var i = 0; i < algoObj.tabsToAdd.length; i++)
        {
            var whereToAdd = algoObj.tabsToAdd[i].whereToAdd;
            var whatToAdd = algoObj.tabsToAdd[i].whatToAdd;

            if (!setUpForPage.subTabs[whereToAdd]){
                setUpForPage.subTabs[whereToAdd] = {};
                setUpForPage.subTabs[whereToAdd].visibleDuring = algoObj.tabsToAdd[i].visibleDuring
                setUpForPage.subTabs[whereToAdd].pageTabName = algoObj.tabsToAdd[i].pageTabName;
                setUpForPage.subTabs[whereToAdd].pageTabChildren = [];
            }
            setUpForPage.subTabs[whereToAdd].pageTabChildren.push (whatToAdd);
        }
    }

    return setUpForPage;
};

$(function ()
{
    $.widget("layout.algoview", $.layout.layoutmanager, {


        options: {
            algoObj: {},
            jsonForPage:{},
            svcType: "",
            algoTitle:"",
            showSubmitByDefault: true,
            paramsPos: undefined,
            closeHelp: true
        },

        _onBeforeInit: function() {
            modalFormName = "algoModalForm";
            
            var node = {
                _type : this.options.svcType,
                id : docLocValue("id")
            };
            this.options.jsonForPage = setUpJsonForAlgo(this.options.algoTitle, this.options.algoObj, this.options.paramsPos);

            jsonOptions = this.options.jsonForPage;

            for (var i = 0; i < jsonOptions.dataSourceArray.length; i++)
            {
                var curDS = jsonOptions.dataSourceArray[i];
                var url=curDS.url;

                if( algoProcess.docLocsToBorrow ) {
                    for( var ir=0; ir<algoProcess.docLocsToBorrow.length; ++ir ) {
                        url=urlExchangeParameter(url, algoProcess.docLocsToBorrow[ir], docLocValue(algoProcess.docLocsToBorrow[ir] , "-"), true);
                    }
                }
                else
                    algoProcess.docLocsToBorrow=[];
                
                algoProcess.docLocsToBorrow.push("id");
                
                var idVal = docLocValue("id" , "-");
                if (idVal.length > 1 && idVal.indexOf("-") == 0)
                    idVal = idVal.substring(1);
                url=urlExchangeParameter(url, "ids", idVal, true);
                url=urlExchangeParameter(url, "reqObjID", idVal, true);
                url=urlExchangeParameter(url, "submitter", docLocValue("cmdMode" , "-"), true);
                ir = algoProcess.docLocsToBorrow.indexOf("id");
                if (url.indexOf("$id") > -1 && algoProcess.docLocsToBorrow && docLocValue(algoProcess.docLocsToBorrow[ir] , "-") != '-')
                    url = url.substring(0, url.indexOf('$id')) + docLocValue(algoProcess.docLocsToBorrow[ir] , "-") + url.substring(url.indexOf('$id')+3);

                vjDS.add('', curDS.name, url);
                if (curDS.parser)
                    vjDS[curDS.name].parser = curDS.parser;
            }

            this.options.config = {
                layout: {
                    items: [
                        {
                            id: 'right',
                            top: '0',
                            left: '75%',
                            right: '100%',
                            bottom: '100%',
                            toggler: "west",
                            allowMaximize: true,
                            tabs:{
                                items:[{
                                    active: true,
                                    overflow:'auto',
                                    title: this.options.helpName ? this.options.helpName : "Service Help",
                                    view: {
                                        name: 'dataview',
                                        options: {
                                            dataViewer: 'vjHelpView',
                                            dataViewerOptions: {
                                                data: "dsHelp"
                                            }
                                        }
                                    }
                                }]
                            }
                        }
                    ]
                }
            };
        },
        
        _onAfterInit : function() {            
            algoWidgetObj = new AlgoWidgetHelp(jsonOptions);

            if (vjDS["ds"+toolBar].data && currentCompletionState != "preSubmit")
                algoProcess.toolbarCallback ("", vjDS["ds"+toolBar].data);

            var manager = $.getAlgoViewManager();
            manager.append({
                layout: {
                    items:[
                    {
                        id: "left",
                        top: '0',
                        left: '0%',
                        right: '20%',
                        bottom: '100%',
                        toggler: 'east',
                        overflow:"auto",
                        view: {
                            name: 'dataview',
                            overflow:"auto",
                            options: {
                                instance: algoWidgetObj
                            }
                        }
                    }]
                }
            });

            $("#left-panel").append($("#menuDiv"));

            if (this.options.algoObj.help.length > 0)
            {
                for (var i = 0; i < this.options.algoObj.help.length; i++)
                {
                    var curHlp = this.options.algoObj.help[i];
                    var dsName = "ds" + parseInt(Math.random()*10000);
                    vjDS.add ("", dsName, curHlp.url);

                    manager.append({
                        layout: {
                            items:[
                            {
                                id: "right",
                                top: "0",
                                left: "75%",
                                right: "100%",
                                bottom: "100%",
                                toggler: "west",
                                allowMaximize: true,
                                tabs:{
                                    items: [{
                                        overflow: 'auto',
                                        active: false,
                                        name: curHlp.name.replace(/ /g,'')+"tab",
                                        title: curHlp.name,
                                        view: {
                                            name: 'dataview',
                                            options: {
                                                dataViewer: 'vjHelpView',
                                                dataViewerOptions: {
                                                    data: dsName
                                                }
                                            }
                                        }
                                    }]
                                }
                            }]
                        }
                    });
                }
            }
            $("."+currentCompletionState).children("ul").toggle();
            if (currentCompletionState != "preSubmit")
                algoWidgetObj.moveTab((this.options.paramsPos ? this.options.paramsPos.posId : "general"), {top:"0%", left: "20%", right: "75%", bottom: "50%"}, 0);

            if(this.options.showSubmitByDefault) $("[name='BUTTON-submitter']").attr("style", "");
        },

    });

}(jQuery));


jQuery.getAlgoViewManager = function() {
    if($('.layout-manager').length == 0)
        console.log('ERROR: cannot find Layout Manager!');
    else
        return $('.layout-manager').first().algoview('instance');
}

$(document).on("tab-active", function (event, areaInfo)
{
    if (areaInfo.tabs)
    {
        for (var i = 0; i < areaInfo.tabs.length; i++)
        {
            if (areaInfo.tabs[i].active){ 
                $("[tab-id='"+areaInfo.tabs[i].name+"']").addClass("activeSelection");
                if (algoWidgetObj){
                    var info = algoWidgetObj.findInJson(jsonOptions.subTabs, "tabId", areaInfo.tabs[i].name);
                    if (info.callback)
                        info.callback(info);
                }
            }
            else $("[tab-id='"+areaInfo.tabs[i].name+"']").removeClass("activeSelection");
        }
    }
    else
    {
        $("[tab-id='"+areaInfo.name+"']").addClass("activeSelection");
        var allOtherTabsInCurArea = $("[data-name='"+areaInfo.name+"']").parent().siblings().children();

        for (var i = 0; i < allOtherTabsInCurArea.length; i++)
        {
            var cur = allOtherTabsInCurArea[i];
            $("[tab-id='"+$(cur).attr("data-name")+"']").removeClass("activeSelection");
        }
    }
});

$(document).on("tab-inactive", function (event, areaInfo)
{
    $("[tab-id='"+areaInfo+"']").removeClass("activeSelection");
    
    var tabObj = algoWidgetObj.findInJson(jsonOptions.subTabs, "tabId", areaInfo);    
    if (tabObj.allowClose){
        var manager = $.getAlgoViewManager();
        manager.remove(areaInfo);
        $("[tab-id='"+areaInfo+"']").remove();
        algoWidgetObj.deleteFromJson(jsonOptions.subTabs, "tabId", areaInfo);
    }
});

$(document).on("area-hide", function (event, areaInfo)
{
    $("[tab-id='"+areaInfo+"']").removeClass("activeSelection");  
    
    var tabObj = algoWidgetObj.findInJson(jsonOptions.subTabs, "tabId", areaInfo);    
    if (tabObj.allowClose){
        var manager = $.getAlgoViewManager();
        manager.remove(areaInfo);
        $("[tab-id='"+areaInfo+"']").remove();
        algoWidgetObj.deleteFromJson(jsonOptions.subTabs, "tabId", areaInfo);
    }
});
