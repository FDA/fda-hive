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
var optionsForPage={};
var currentCompletionState = "preSubmit";
var formName="algoForm";
var valgoToolbarWaitingList="type,align,order,name,title,icon,path,url,description\n"+
                            ",left,1,files,Modify and Resubmit,img/recRevert.gif,/resubmit,"+
                            urlExchangeParameter(document.location,"id","-"+docLocValue("id"))+
                            ",Modify parameters and resubmit this computation using the same template\n";
var valgoToolbarDoneList="";
var toolBar="processToolbar";
var parametersDiv = "";
var algoWidgetObj;
var originalStat = -1;


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
        var tbl = new vjTableView();
        tbl.initTblArr(content);

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

        var tblArr = tbl.tblArr;
        for (var i = 1; tblArr.rows && i < tblArr.rows.length; i++)
        {
            var url = tblArr.rows[i].cols[7];
            if(url == "") continue;
            $("#"+tab).append(
                    $(document.createElement("li"))
                    .append($(document.createElement("a"))
                            .attr({"cur-index":i})
                            .click(function(){
                                var ind = $(this).attr("cur-index");
                                funcLink(tblArr.rows[ind].cols[7]);
                            })
                            .text(tblArr.rows[i].cols[4])
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
        if ($("#menuDiv").children("div").children("p").length < 1){
            $("#menuDiv").children("div").append($(document.createElement("p"))
                    .text(name));
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
        var cmd = docLocValue("cmd");
        var cmdMode = docLocValue("cmdMode");
        var batchMode = docLocValue("batchMode");
        if (isok(cmdMode)) cmd += "&cmdMode=" + cmdMode;
        if (parseBool(batchMode) && batch_svc_elt) {
            cmd += "&batchMode=true";
            viewer.changeElementValue("batch_svc", this.qpSvc);
        }
        viewer.changeElementValue("submitter", cmd);
        this.updateBatchFldTree(viewer);

        if(this.inputLoaded)
            this.inputLoaded (viewer,this);
        if(this.callbackLoaded)
            this.callbackLoaded (viewer,this);
    };
    
    this.readFromDocLoc=function(namearr)
    {
        if(namearr=="*")namearr=vjDV[this.recViewerName].fldTree.accumulate(1,"node.name");
        for( var i=0; i<namearr.length ; ++ i) {
            par = docLocValue(namearr[i]);
            if( isok(par) ) {
                var o=new Object();
                o[namearr[i]]=par.split(",");
                this.setValueList(o);
            }
        }
    };

    this.onRecordChanged=function(viewer, elem)
    {
        if (elem.fld.name == "batch_svc") {
            var qpSvc = parseBool(elem.value) && elem.value != "single" ? "svc-batcher" : this.qpSvc;
            viewer.cmdPropSet = "?cmd=-qpProcSubmit&svc=" + qpSvc;
            this.updateBatchFldTree(viewer);
        } else if (this.cmdModeLst) {
            var value = verarr(elem.value)[0];
            for (f in this.cmdModeLst) {
                if (elem.fld.name == f) {
                    var svcs = this.cmdModeLst[f];
                    var new_cmdMode = null;
                    var that = this;
                    var old_svc = null;
                    var old_cmdMode_prefix = '';
                    var old_cmdMode_suffix = '';
                    svcs.forEach(function(svc) {
                        var re = new RegExp('(^(?:.*\\W)?)' + svc + '((?:\\W.*)?$)');
                        if (value.match(re)) {
                            if (!new_cmdMode || svc.length > new_cmdMode.length) {
                                new_cmdMode = svc;
                            }
                        }
                        var m = 0;
                        if (m = that.cmdMode.match(re)) {
                            if (!old_svc || svc.length > old_cmdMode.length) {
                                old_svc = svc;
                                old_cmdMode_prefix = m[1];
                                old_cmdMode_suffix = m[2];
                            }
                        }
                    });
                    if (new_cmdMode && this.cmdMode != new_cmdMode) {
                        var new_url = urlExchangeParameter("" + document.location, "cmdMode", old_cmdMode_prefix + new_cmdMode + old_cmdMode_suffix);
                        document.location = new_url;
                    }
                    break;
                }
            }
        }
        if(this.inputChanged)
            this.inputChanged (viewer,elem);
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
        if(that.doneComputing && stat <= 5 && !that.isBatch()){
            callbackFullview = that.doneComputing(viewer,reqid,stat);
        }
        stat = parseInt(stat);

        if (stat == 5 && (originalStat > -1 && originalStat < 5)) 
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
            algoWidgetObj.openTab("info");
        }
        else if (stat >= 1 && stat < 5)
        {
            currentCompletionState = "whileRunning";
            algoWidgetObj.iterateAlgoJSON (algoWidgetObj.optionsForPage.subTabs, "algoMenu");
            if (that.callbackProgressComputing)
                that.callbackProgressComputing (viewer,reqid,stat);
        }
        else if(stat > 5){
            vjDS["dsProgress"].reload(vjDS["dsProgress"].url, true);
            vjDS["dsProgress_download"].reload(vjDS["dsProgress_download"].url, true);
            vjDS["dsProgress_info"].reload(vjDS["dsProgress_info"].url, true);
            vjDS["dsProgress_inputs"].reload(vjDS["dsProgress_inputs"].url, true);
            vjDS["dsProgress_outputs"].reload(vjDS["dsProgress_outputs"].url, true);
            
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
        }
        else if (!that.isBatch() && stat >= 5)
            onContinueToResults();
        
        if (originalStat < 0)
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

        if (callbackFullview)
            return;

        node = {
            _type : algoProcess.svcProcType,
            id : docLocValue("id")
        };
        algoWidgetObj.iterateAlgoJSON (algoWidgetObj.optionsForPage.subTabs, "algoMenu")
        vjHO.fullview(node._type, node, $.getAlgoViewManager().options.jsonForPage.subTabs.results);
        
        if (algoWidgetObj.noAllDownloadsTab)return;

        if (!vjDS.dsAllDownloads)
            vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
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
                            document.location = "?cmd=objFile&ids="+node.id+"&filename="+node.value;
                        }
                    },
                    iconSize:24,
                    cols:[
                        { name: new RegExp(/.*/), hidden:true },
                        { name: "name", hidden:true },
                        { name: "pretty_name", title: "name", hidden: false },
                        { name: "description", hidden: false },
                        { name: "icon", hidden: true },
                        { name: "archive", hidden: false, url: "javascript: var extension = node.value.substr(node.value.lastIndexOf('.')+1); \
                                var ing_url = 'qpbg_http:
                                var ing_url = 'qpbg_http:
                                vjDS.dsVoid.reload(ing_url,true); \
                                alert ('Your selected item is being ingested. You can monitor the progress from within data loading tab');"
                        },
                        { name: "down", hidden: false, url: algoProcess.allDownUrlModification ? algoProcess.allDownUrlModification() : "javascript: \
                           var ing_url = 'http:
                            vjDS.add('', 'dsDownSrc', 'static:
                            vjDS['dsDownSrc'].reload(ing_url,true,'download');    "
                        }
                    ],
                    appendCols : [{header: {name: "down", title: "download"}, cell: "<img src='img/download.gif' height=24 width=24/>" }, {header: {name: "archive"}, cell: "<img src='img/upload.gif' height=24 width=24/>"}, {header: {name: "pretty_name", title: "name"}}, {header: {name: "description"}}],
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
                        if (!node.prety_name) {
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
                    }
                }
            }
        }], "results");

        if (that.noResultViewers)
        {
            algoWidgetObj.openTab("downloadAllFiles");
        }
    }


    this.onSubmitRequest=function()
    {
        if (algoProcess.submitCallback)
            algoProcess.submitCallback();
        
        var tThis = this;
        
        vjDS.add("", "dsProjectList", "static:
        vjDS.add("", "dsComputationProjectList", "static:
        
        var bothBack = false;
        var callbackFunc = function(){
            if(!bothBack){
                bothBack = true;
                return;
            }
            
            var projectJson = JSON.parse(vjDS.dsProjectList.data);
            
            if(!projectJson.objs || projectJson.objs.length < 1) return !tThis.submit();
            
            var computationData = vjDS.dsComputationProjectList.data;
            var compRows = new vjTable(computationData, undefined, vjTable_propCSV);
            
            var allIds=[];
            for(var i=0; i < compRows.rows.length; i++){
                var row = compRows.rows[i];                
                var id = parseInt(row.submission_project);
                
                if(allIds.indexOf(id) > -1) continue;
                else allIds.push(id);
            }
            
            projectJson.objs.sort(function(a,b){
                if(allIds.indexOf(a._id) >= 0 || allIds.indexOf(b._id) >= 0) return -1;
                
                return a._brief.localeCompare(b._brief);
            });
            
            var selectOptions = $(document.createElement("select"))
                    .text("Slect a Project")
                    .append($(document.createElement("option"))
                        .attr("value", -1)
                        .text(" ")
                    );
            for(var i = 0; i < projectJson.objs.length; i++){
                selectOptions.append($(document.createElement("option"))
                        .attr("value", projectJson.objs[i]._id)
                        .text(projectJson.objs[i]._brief)
                );
            }
            
            $("body").append(
                $(document.createElement("div"))
                    .attr("id", "dialog-project")
                    .attr("title", "Select Project")
                    .append (
                            $(document.createElement("p")).text("Please select a project for this computation")
                    )
                    .append($(document.createElement("form"))
                            .append($(document.createElement("fieldset"))
                                    .append($(document.createElement("label"))
                                            .text("Select a Project")
                                    )
                                    .append(selectOptions)
                            )
                    )
                );
            
            $("#dialog-project").dialog({
                modal: true,
                width: 500,
                buttons: {
                    OK: function() {
                        $(this).dialog("close");
                        var value = parseInt($(this).find("select").val());
                        if(value > -1) 
                            vjDV[tThis.recViewerName].changeElementValue("submission_project",value);
                        tThis.submit();
                    },
                    Cancel: function() {
                       $(this).dialog("close");
                       tThis.submit();
                    }
                },
                open: function() {
                    $(this).closest(".ui-dialog")
                    .find(".ui-dialog-titlebar-close")
                    .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                    .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                }
            });
        };
        

        vjDS.dsProjectList.register_callback(callbackFunc);
        vjDS.dsComputationProjectList.register_callback(callbackFunc);

        vjDS.dsProjectList.reload("http:
        vjDS.dsComputationProjectList.reload("http:
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

            if (((elem.fld.is_optional_fg == 0 && !elem.fld.is_readonly_fg && !elem.fld.is_hidden_fg) && (!elem.children || elem.children.length < 1)) && elem.value == "" && elem.name.indexOf(".system.") < 0)
                return false;
        }

        return true;
    };

    this.submit=function(cbFunc,cnts)
    {
        if(this.submittedAlready)return;
        
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
                                    $(document.createElement("p")).text("You have not entered values for all of the requiered fields")
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
        this.submittedAlready=true;
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
            document.location="?cmd="+docLocValue("cmd")+"&id="+this.loadedID+"&cmdMode="+docLocValue("cmdMode");

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
        else if (!options.jsonForPage.subTabs[whereToAdd])
            return false;
        else
            actualObjLoc = options.jsonForPage.subTabs[whereToAdd];

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
        if (!this.buildModal)
        {
            var modalForm = $(document.createElement("form")).attr("id", modalFormName);
            modalForm.append(this.buildModalWindow());
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
        { ds:"dsSystemParams", params: ["comment","system","random_seed","split"]},
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
        {name: "dsProgress_download", url: "http:
        {name: "dsProgress_info", url: "http:
        {name: "dsProgress_inputs", url: "http:
        {name: "dsProgress_outputs", url: "http:
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
                            cmdPropSet:"?cmd="+submitCmd+"&svc="+algoProcess.qpSvc,
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
                            autoDescription:0,
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
                        tabDependents: ["inputObj", "objUsing", "info", "downloads"],
                        position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'20%', right:'75%'},
                        preConstructor: {
                            dataViewer: 'vjProgressControl',
                            dataViewerOptions: {
                                data : {
                                    progress: "dsProgress",
                                    progress_download:  "dsProgress_download",
                                    inputs:  "dsProgress_inputs",
                                    outputs:  "dsProgress_outputs",
                                    progress_info: "dsProgress_info"
                                },
                                width: "100%",
                                newViewer: true,
                                dontRefreshAll: true,
                                formName: formName,
                                doneCallback: algoProcess.callbackDoneComputing
                            }
                        },
                        viewerConstructor: {
                            preObjPos: 0,
                        },
                        autoOpen: ["whileRunning"]
                    },
                    {
                        tabId: 'inputObj',
                        tabName: "Input Objects",
                        position: {posId: 'progress_info', top:'50%%', bottom:'100%', left:'20%', right:'75%'},
                        viewerConstructor: {
                            preObjPos: 1,
                            inactive:true
                        },
                        autoOpen: ["whileRunning"]
                    },
                    {
                        tabId: 'objUsing',
                        tabName: "Objects Using This as Input",
                        position: {posId: 'progress_info', top:'50%%', bottom:'100%', left:'20%', right:'75%'},
                        viewerConstructor: {
                            preObjPos: 2,
                            inactive:true
                        },
                        autoOpen: ["whileRunning"]
                    },
                    {
                        tabId: 'info',
                        tabName: "Notifications",
                        position: {posId: 'progress_info', top:'50%%', bottom:'100%', left:'20%', right:'75%'},
                        viewerConstructor: {
                            preObjPos: 3,
                            inactive:true
                        },
                        autoOpen: ["whileRunning"]
                    },
                    {
                        tabId: 'downloads',
                        tabName: "Download",
                        position: {posId: 'progress_info', top:'50%%', bottom:'100%', left:'20%', right:'75%'},
                        viewerConstructor: {
                            preObjPos: 4,
                            inactive:true
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
