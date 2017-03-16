

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
    var txt="<form name='algoForm'>" +
                "<div class='content'>" +
                    "<div class='main-content' id='mainAlgoArea'></div>" +
                "</div>" +
            "</form>";

    return txt;
}


//all of the functions that deal with the algorithm (including all of its callbacks, etc)
function valgoProcess(loadedID,  qpSvc, svcProcType, svcRecViewer)
{
    this.recViewerName = 'DV_Parameter_view';
    this.cmdModeLst = {  batch_svc: ["bool:single","batch"]};
    this.ID=docLocValue("id");
    algoProcessID = docLocValue("id"); //I think that this.ID should be used in all of those URL's
    this.cmdMode=docLocValue("cmdMode");
    this.help = []; //if any extra helps need to be passed in
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
        vjObj.register(this.objCls,this); // needed to find this object in events by object container
    };

    this.isMode = function(mode)
    {
        return ((!this.cmdMode || (this.cmdMode.indexOf("-"+mode)==-1 && this.cmdMode.indexOf(mode)!=0))) ? false : true;
    };
    
    this.isResultsMode = function(mode)
    {
        return (!this.cmdMode || (this.cmdMode.indexOf("-"+mode)==-1 && this.cmdMode.indexOf(mode)!=0)) ? false : true;
    };

    // this function should not be called before algoProcess record viewer was initialized
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

    vjDS.add("infrastructure: Constructing Toolbar", "ds"+toolBar, "static://");
    vjDS["ds"+toolBar].register_callback(this.toolbarCallback);
    vjDS["ds"+toolBar].reload("static://"+valgoToolbarWaitingList, true);

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
        if(this.initialPresets && currentCompletionState == "preSubmit"){
            viewer.changeValueList(this.initialPresets);
        }

        var batch_param_constraint_text = "";
        viewer.fldTree.enumerate(function(param, node) {
            if (node.is_batch_fg) {
                if (batch_param_constraint_text.length) {
                    batch_param_constraint_text += "|";
                }
                batch_param_constraint_text += node.name + "///" + node.title;
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
        }
        if(this.inputChanged)
            this.inputChanged (viewer,elem);
    };

    this.updateBatchFldTree=function(viewer)
    {
        if (this._in_updateFldPresets) {
            // avoid potentially recursive callbacks
            return;
        }
        this._in_updateFldPresets = true;

        // in batch mode, batch_param and batch_value fields must be non-optional
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
                    // batchable fields are (sometimes) allowed to be ';'-separated lists
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

    this.callbackDoneComputing = function (viewer, reqid, stat)
    {
        if(that.doneComputing){
            callbackFullview = that.doneComputing(viewer,reqid,stat);
        }

        if (stat >= 6 && (originalStat > -1 && originalStat < 5)) 
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
        else if (!that.isBatch())
            onContinueToResults();
        
        if (originalStat < 0)
            originalStat = stat;
    };
    
    function onContinueToResults ()
    {

        $(".whileRunning").children("ul").toggle();
        currentCompletionState = "computed";
        $("."+currentCompletionState).children("ul").toggle();
        $(".computed").removeAttr("style");

        //here we will add a toolbar to a tab with options
        vjDS["ds"+toolBar].reload("static://"+valgoToolbarWaitingList+valgoToolbarDoneList, true);

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
            vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http://?cmd=propget&files="+vjDS.escapeQueryLanguage("*.{csv,json,png,tab,tsv,txt,fasta,fastq,fa}")+"&mode=csv&prop=none&ids="+algoProcessID, 0, "id,name,path,value");
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
                                var ing_url = 'qpbg_http://?cmd=objFile&ids='+node.id+'&filename='+node.value+'&arch=1&backend=1&ext='+extension+'&arch_dstname=o'+docLocValue('id')+'-'+node.value; \
                                vjDS.dsVoid.reload(ing_url,true); \
                                alert ('Your selected item is being ingested. You can monitor the progress from within data loading tab');"
                        },
                        { name: "down", hidden: false, url: algoProcess.allDownUrlModification ? algoProcess.allDownUrlModification() : "javascript: var ing_url = 'http://?cmd=objFile&ids='+node.id+'&filename='+node.value;\
                            vjDS.add('', 'dsDownSrc', 'static://');\
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

            //since all of the algorithm's parent is the process type, and almost all of the fields in
            //that type are only defined after submition, we are skipping the check of the ".system." children
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

        this.viewer.saveValues(null, "auto", "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");

        if (!this.validateSubmit(vjDV[parametersDiv].nodeTree.root.children))
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

    //below are all of the functions that deal with the widget and how the different tabs are opened or closed
    this.findInJson = function (options, field, value)
    {
        if (options.length < 1)
            options = this.optionsForPage.subTabs;

        for ( tab in options ) {
              var curNode = options[tab];

              //for (subView in curNode.pageTabChildren) {
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

              //for (subView in curNode.pageTabChildren) {
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
    
                //$.extend(viewerOptions, {instance: preObj[curTab.viewerConstructor.preObjPos]});
            }
            else if (curTab.viewerConstructor && curTab.viewerConstructor.instance)
                $.extend(viewerOptions, {instance: curTab.viewerConstructor.instance});
            else if (curTab.viewerConstructor.viewName)
                viewerOptions = curTab.viewerConstructor.dataViewerOptions
        }
/*        
        var viewOptionsToUse = viewerOptions;
        if (curTab.multiView){ //in future may want to use this for extra options
            if (viewerOptions.views){
                viewOptionsToUse.views = new Array (viewerOptions.views.length)
                for (var j = 0; j < viewerOptions.views.length; j++){
                    viewOptionsToUse.views[j] = {allowClose: viewerOptions.allowClose, view: viewerOptions.views[j]};
                }
            }
        }*/
        
        var manager = $.getAlgoViewManager();
        if (/*!manager.show(curTab.position.posId) && */!manager.show(curTab.tabId))
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
                        //allowClose: curTab.allowClose ? curTab.allowClose : true,
                        allowMaximize: true,
                        tabs:{
                            items: [{
                                overflow: /*curTab.overflow ? curTab.overflow :*/ 'auto',
                                allowHide: true,
                                active: curTab.inactive ? false : true,
                                name: curTab.tabId,
                                //allowClose: curTab.allowClose ? curTab.allowClose : false,
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

        /*if ($("[tab-id='"+curTab.tabId+"']").length)
            $("[tab-id='"+curTab.tabId+"']").attr("style", activeStyleClick);*/

        if (curTab.showSubmitButton && currentCompletionState == "preSubmit" && !curTab.submitCreated)
        {
            var buttons = verarr(curTab.showSubmitButton);
            
            for (var i = 0; i < buttons.length; i++)
                $("#"+curTab.tabId+"-tab").append (buttons[i]);
            
            curTab.submitCreated=true;
        }

        //$(this).trigger('tab-open', { index: p.index, name: $('a', p.tab).attr('data-name'), area: $(p.tab).closest('div.layout-area').attr('data-id') });
        //if (!curTab.inactive)
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
        //$(someVar).attr("style", activeStyleClick);
        return false;
    };


    //must come in with subTabs being in the options
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
                                        //.click(function(){ that.clickFunc($(this)); })
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

              //for (subView in curNode.pageTabChildren) {
              for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                  var curView = curNode.pageTabChildren[ii];
                  var toTriggerTabOpen = false;

                  if (!curView.visible && ((curView.autoOpen && curView.autoOpen.indexOf(currentCompletionState) > -1) || curView.openUp))
                      this.openTab (curView, -1, true);
                  else if (!curView.visible || !(curView.autoOpen && curView.autoOpen.indexOf(currentCompletionState) > -1))
                      this.closeTab (curView);
                  toTriggerTabOpen = curView.inactive ? !curView.inactive : true;

                  if (curView.visible)
                  {
                      $("#"+tab).append(
                          $(document.createElement("li"))
                            .append($(document.createElement("a"))
                                    //.addClass("activeSelection")
                                    .attr({"tab-id": curView.tabId/*, "style": activeStyleClick*/})
                                    .click(function(){ return that.clickFunc($(this)); })
                                    .text(curView.tabName)
                            )
                            .append($(document.createElement("i"))
                                    .addClass("fa fa-angle-right")
                            )
                      );
                  }
                  else if (!curView.constructedAlready)
                  {
                      $("#"+tab).append(
                          $(document.createElement("li"))
                                .append($(document.createElement("a"))
                                        .click(function(){ return that.clickFunc($(this)); })
                                        .attr({"tab-id": curView.tabId})
                                        .text(curView.tabName)
                                )
                                .append($(document.createElement("i"))
                                        .addClass("fa fa-angle-right")
                                )
                      );
                  }

                  if (toTriggerTabOpen)
                      $(document).trigger("tab-active", {index: "", name: curView.tabId, area: ""});

                  if (curView.subTabs)
                      toReturn += this.iterateAlgoJSON (curView.subTabs, tab);
              }
        }

        return toReturn;
    };

    //this will return the structure that was added if the addition happened successfully,
    //otherwise, false will be returned
    //once the structure is received, the tab can be opened if needed
    //will be added to pageTabChildren
    this.addTabs = function (whatToAdd, whereToAdd)
    {
        var options = $.getAlgoViewManager().options;

        var that = this;
        var actualObjLoc = options.jsonForPage;
        
        if (whereToAdd instanceof Array){ // the path will be passed in
            actualObjLoc = options.jsonForPage.subTabs;
            for (var i = 0; i < whereToAdd.length-2; i++){
                if (!actualObjLoc[whereToAdd[i]]){
                    var key = whereToAdd[i];
                    actualObjLoc[key]={};
                }
                if (!actualObjLoc[whereToAdd[i]].pageTabChildren){
                    var key = whereToAdd[i];
                    actualObjLoc[key].subTabs = {};
                    actualObjLoc =  actualObjLoc[key].subTabs;
                    break;
                }
                
                actualObjLoc = actualObjLoc[whereToAdd[i]].pageTabChildren;
            }
            
            actualObjLoc[whereToAdd[whereToAdd.length-2]] = {};
            actualObjLoc = actualObjLoc[whereToAdd[whereToAdd.length-2]];
            actualObjLoc.pageTabName = whereToAdd[whereToAdd.length-1];
            actualObjLoc.pageTabChildren = [];
        }
        else if (!options.jsonForPage.subTabs[whereToAdd])
            return false;
        else
            actualObjLoc = options.jsonForPage.subTabs[whereToAdd];

        $(verarr(whatToAdd)).each(function (index, nObj){
            //here we will check that the whatToAdd stucture is appropriate
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

    //whereToRemove must contain what cattegory to remove from (this means parameters, progress, results, any others?)
    //whatToRemove had the tabId list
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

    //this will check if a tab exists. even if the tab is not visible
    //will return the object that has the tab information in it (From jsonForPage)
    this.existsTab = function (lookForTabId, where)
    {
        if (!where)
            where = $.getAlgoViewManager().options.jsonForPage.subTabs;

        for ( var tab in where )
        {
            var curNode = where[tab];

            //for (subView in curNode.pageTabChildren) {
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
            $('body').append(this.buildModalWindow());
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

    //this will move the tab and the are where it is located to a new location.
    //will happen over the span of time passed in
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
                "<h2>" + this.optionsForPage.algorithmTitle + "</h2><ul id='algoMenu'>" +
                "</ul>" +
            "</div>");

    var listToAppend = this.iterateAlgoJSON (this.optionsForPage.subTabs, "algoMenu");
    parametersDiv = this.optionsForPage.parametersDiv;
    var that = this;
    this.buildModal = false;
};


/*
 * if want to pass in extra viewers into parameters or progress do so in the following format:
 * algoObj.tabsToAdd = [
 *         {
 *             whereToAdd: "parameters",
 *             whatToAdd: {
                    tabId: 'general',
                    tabName: "General",
                    //showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                    showSubmitButton: "<button id='submitterInput' type=button style='visibility:hidden;' class='myButton' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20>"+(algoProcess.submitButtonName ? algoProcess.submitButtonName : "SUBMIT")+"</button>",
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
                }
        },{
            whereToAdd: "progress",
            whatToAdd: {other stuff here}
           }
 * ]
 */
function setUpJsonForAlgo (algoTitle, algoObj, paramsPos)
{
    var batchParams = ["batch", "batch_ignore_errors"];
    // if process has been submitted, force batch_children_proc_ids to be visible with other batch params
    // otherwise, let record viewer do the default (which is to hide batch_children_proc_ids)
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
    
   // var typeToUse = algoProcess.svcProcType ? algoProcess.svcProcType :  algoProcess.svcRecViewer;
    var dsArray=[
        {name: "dsAlgoSpec", url: "http://?cmd=propspec&type="+(algoProcess.svcRecViewer ? algoProcess.svcRecViewer : algoProcess.svcProcType)},
        {name: "dsAlgoVals", url: "http://?cmd=propget&mode=csv&ids=$id&files=*"},
        {name: "dsProgress", url: "http://?cmd=-qpRawCheck&showreqs=0&reqObjID=$id"},
        {name: "dsProgress_download", url: "http://?cmd=-qpRawCheck&showreqs=0&reqObjID=$id" },
        {name: "dsProgress_info", url: "http://?cmd=-qpReqInfo&reqObjID=$id"},
        {name: "dsProgress_inputs", url: "http://?cmd=objQry&qry=allusedby($id).csv(['_type','_brief','created','svc','submitter'],{info:true,cnt:20})"},
        {name: "dsProgress_outputs", url: "http://?cmd=objQry&qry=allthatuse($id).filter({._type!~/folder/}).csv(['_type','_brief','created','svc','submitter'],{info:true,cnt:20})"},
        {name: "dsHelp", url: "http://help/hlp." + (algoProcess.svcRecViewer ? algoProcess.svcRecViewer :algoProcess.svcProcType) + ".html"}
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
        dsArray.push( {name: moveParams[i].ds, url: "static://"+t});
    }
    if (!batchInGeneral){
        for (var i = 0; i < dsArray.length; i++)
            if (dsArray[i].name == "dsBatchParams")
                dsArray[i].url += "<span id='RV-batch_svc'></span>";
    }

    var batchFldPreset={batch_svc:{constraint:'choice+', constraint_data:'single///Single Computation Mode|'+algoObj.qpSvc+'///Batch Mode'}};
    var submitButtons = algoProcess.submitButtons ? algoProcess.submitButtons :  ("<button id='submitterInput' style='visibility:hidden;' type=button class='myButton submitterInputButton' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20>"+(algoProcess.submitButtonName ? algoProcess.submitButtonName : "SUBMIT")+"</button>");
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
                    //showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+ (algoProcess.submitButtonName ? algoProcess.submitButtonName : "SUBMIT") +"' />",
                    showSubmitButton: submitButtons,
                    tabDependents: ["general","system", "batch"],
                    inactive: true,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjRecordView',
                        dataViewerOptions: {
                            divName: 'DV_Parameter_view',
                            kind:"valgoProcess",
                            data: algoProcess.loadedID ? [ "dsAlgoSpec", "dsAlgoVals" ] : ["dsAlgoSpec"],
                            hiveId: propHiveId,
                            objType:  algoProcess.svcRecViewer ? algoProcess.svcRecViewer : algoProcess.svcProcType  ,
                            cmdPropSet:"?cmd="+submitCmd+"&svc="+algoProcess.qpSvc,
                            readonlyMode: algoProcess.modeActive ? false : true,
                            callbackRendered : "function:vjObjFunc('onRecordLoaded','"+algoProcess.objCls+"')",
                            onChangeCallback : "function:vjObjFunc('onRecordChanged','"+algoProcess.objCls+"')",
                            onAddElementCallback : "function:vjObjFunc('onRecordAddedElement','"+algoProcess.objCls+"')",
                            accumulateWithNonModified:true,
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
                    /*subTabs : {
                        basicStuff: {
                            pageTabName: "Basic Stuff",
                            pageTabChildren: [
                                {
                                    tabId: 'otherStuff',
                                    tabName: "Some Other Stuff",
                                    visible: true, //this is also modified by the algoview. just lets know if the tab is visible or not
                                    position: {posId: 'blah', top:'50%', bottom:'100%', left:'0', right:'75%'}
                                }
                            ]
                        }
                    },*/
                },
                {
                    tabId: 'general',
                    tabName: "General",
                    //showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
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
                    //showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                    showSubmitButton: submitButtons,
                    inactive: true,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    //some div name here or the data source
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
                    //showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                    showSubmitButton: submitButtons,
                    inactive: true,
                    position: paramsPos ? paramsPos : {posId: 'layout_inputs', top:'0', bottom:'100%', left:'20%', right:'75%'},
                    //some div name here or the data source
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

            setUpForPage.subTabs[whereToAdd].pageTabChildren.push (whatToAdd);
        }
    }

    return setUpForPage;
};

$(function ()
{
    $.widget("layout.algoview", $.layout.layoutmanager, {


        //type flex is not working?
        //yes it is. for flex layout do not specify anything for the type
        //need to figure out a way to access this options thing
        options: {
            algoObj: {},
            jsonForPage:{},
            svcType: "",
            algoTitle:"",
            showSubmitByDefault: true,
            paramsPos: undefined,
            closeHelp: true
        },

        //this is called from layoutmanager's _create function called 1st
        _onBeforeInit: function() {

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

                //if any url replacing needs to happen, do it here
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

            //this._minimizeToolbar();
        },

        _minimizeToolbar: function() {
            var minimizeFunction = function() {
                    $("#entireTopBar").animate({top: '-85px'});
                     $(".mainAlgocontent").animate({ top: '40px' }, { done: function() { $(window).trigger('resize'); } });
                     $("#dvMenu").animate({"left":"5%", "width":"95%"});

                     $("#outterMenu").append(
                             $(document.createElement("a")).append(
                                     $(document.createElement("img"))
                                         .css({"height": "25px", "left": "10px", "top": "2px", "width": "3%", "position": "fixed"})
                                         .attr({"src": "img/HIVELogoDec0213_Transparent.png"})
                             )
                     );

                    $(document).unbind('mousemove');
            };

            var timer = setTimeout(minimizeFunction, 2000);
            $(document).on('mousemove', function(e) {
                clearInterval(timer);
                timer = setTimeout(minimizeFunction, 2000);
            });
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
    //if a tab is selected from the area, then we just deselect the ones that could be selected in that area
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
    //when tab gets closed, need to remove selection from the side menu
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
    //when tab gets closed, need to remove selection from the side menu
    $("[tab-id='"+areaInfo+"']").removeClass("activeSelection");  
    
    var tabObj = algoWidgetObj.findInJson(jsonOptions.subTabs, "tabId", areaInfo);    
    if (tabObj.allowClose){
        var manager = $.getAlgoViewManager();
        manager.remove(areaInfo);
        $("[tab-id='"+areaInfo+"']").remove();
        algoWidgetObj.deleteFromJson(jsonOptions.subTabs, "tabId", areaInfo);
    }
});
