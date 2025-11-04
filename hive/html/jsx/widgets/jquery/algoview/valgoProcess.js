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

var currentCompletionState = "preSubmit";
var algoFormName="algoForm";
var valgoToolbarWaitingList="type,align,order,name,title,icon,path,url,description\n"+
                            ",left,1,files,Modify and Resubmit,img/recRevert.gif,/resubmit,"+
                            urlExchangeParameter(document.location,"id","-"+docLocValue("id"))+
                            ",Modify parameters and resubmit this computation using the same template\n";
var valgoToolbarDoneList="";
var toolBar="processToolbar";
var originalStat = -1;


function valgoProcess(loadedID,  qpSvc, svcProcType, svcRecViewer)
{
    this.recViewerName = 'DV_Parameter_view';
    this.recViewer = vjDV[this.recViewerName];
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
        this.formName=algoFormName;


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

    vjDS.add("infrastructure: Constructing Toolbar", "ds"+toolBar, "static:
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
        $("."+currentCompletionState).children("ul").toggle();
    }

    this.onRecordLoaded=function(viewer,text)
    {
        var name = viewer.getElement("name") ? viewer.getElement("name").value : "";
        if ($("[data-name='objNameDiv']").children().length < 1){
            $("[data-name='objNameDiv']").append($(document.createElement("h6"))
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
        algoWidgetObj = $.getAlgoViewManager();
        
        if(that.doneComputing){
            callbackFullview = that.doneComputing(viewer,reqid,stat);
        }

        if (stat >= 6 && (originalStat > -1 && originalStat < 5)) {
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
        else if (stat >= 1 && stat < 5) {
            algoWidgetObj.hideButton();
            currentCompletionState = "whileRunning";
            $("."+currentCompletionState).children("ul").toggle();
            algoWidgetObj._iterateAlgoJson (algoWidgetObj.options.jsonForPage.subTabs, "progress");
            if (that.callbackProgressComputing)
                that.callbackProgressComputing (viewer,reqid,stat);
        }
        else if (!that.isBatch()){
            algoWidgetObj.hideButton();
            onContinueToResults();
        }
        
        if (originalStat < 0)
            originalStat = stat;
    };
    
    function onContinueToResults ()
    {        
        currentCompletionState = "computed";
        $("."+currentCompletionState).children("ul").toggle();
        $(".computed").removeAttr("style");

        vjDS["ds"+toolBar].reload("static:


        if (callbackFullview)
            return;

        node = {
            _type : algoProcess.svcProcType,
            id : docLocValue("id")
        };

        vjHO.fullview(node._type, node, $.getAlgoViewManager().options.jsonForPage.subTabs.results);
        
        if (algoProcess.noAllDownloadsTab) return;
        if (!vjDS.dsAllDownloads)
            vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
        
        algoWidgetObj.addTabs([{
            tabId: 'downloadAllFiles',
            tabTitle: "Available Files to Download",
            position: { x:0, y: 14, width:4, height:6 },
            inactive: true,
            autoOpen:["computed"],
            viewerConstructor: {
                dataViewer: "vjTableView",
                dataViewerOptions:{
                    parsemode: vjTable_hasHeader,
                    data: "dsAllDownloads" ,
                    formName: algoFormName,
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
                                vjDS.dsVoid.reload(ing_url,true); \
                                alert ('Your selected item is being ingested. You can monitor the progress from within data loading tab');"
                        },
                        { name: "down", hidden: false, url: algoProcess.allDownUrlModification ? algoProcess.allDownUrlModification() : "javascript: var ing_url = 'http:
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


    this.onSubmitRequest=function(a,b,c  )
    {
        if (algoProcess.submitCallback)
            algoProcess.submitCallback;
        
        return !this.submit();
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

            if (elem.fld.is_optional_fg == 0 && elem.value == "" && elem.name.indexOf(".system.") < 0)
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

        this.viewer.saveValues(null, true, this.onRedirectProcSubmitted);

        if (!this.validateSubmit(vjDV[this.parametersDiv].nodeTree.root.children))
            return false;
        else
            $('.toSubmitBtn').attr("disabled", "disabled");
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
            document.location=makeCmdSafe(docLocValue("cmd"))+"&id="+this.loadedID+"&cmdMode="+docLocValue("cmdMode");

        }
    };


};