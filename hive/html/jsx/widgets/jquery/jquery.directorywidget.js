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
$(function () {
    var oThis;
    $.widget("view.directorywidget", {
        options:{
            idtype: "id",
            selectedNode: [],
            drag: true,
            dropableAttrs: ['dragDropable'],
            currentFolder: "",
            isNshowIconEmpty: true,
            alwaysSelected: true,
            HistoryStack: [],
            DoStack: [],
            cacheDirectionyExpansionStatus: {},
            expandNodeCallback: "javascript:thisObj.cacheDirectionyExpansionStatus[args[1].id]=args[1].expanded;",
            systemFolder: {},
            systemFolderExclude: {
                addIn : [],
                removeFrom : [],
                delObj : [],
                moveObj : [],
                isNodeIn: function(node, list) {if (!this[list])return false;for ( var i = 0; i < this[list].length; ++i) {if (this[list][i].path == node.path)return this[list][i];}return false;},
                isPathIn: function(path, list) {if (!this[list])return false;for ( var i = 0; i < this[list].length; ++i) {if (this[list][i].path == path)return this[list][i];}return false;}
            },
            systemFolderList: [ {
                name : "HIVE Space",
                title : "HIVE Space",
                icon : "img/hiveicon.jpg",
                order : 1,
                _actionCancel : {"delete":true,"edit":true},
                block : {addIn : false,removeFrom : false,delObj : true,moveObj: true}
            }, {
                id : "all",
                name : "all",
                title : "All objects",
                icon : "img/folder-apps.gif",
                order : 2,
                _actionCancel : {"delete":true,"edit":true},
                block : {addIn : true,removeFrom : true,delObj : true,moveObj: true},
                isVirtual : true
            }, {
                name : "Inbox",
                title : "Inbox",
                icon : "img/folder-inbox.gif",
                order : 3,
                _actionCancel : {"delete":true,"edit":true},
                block : {addIn : false,removeFrom : false,delObj : true,moveObj: true}
            }, {
                name : "Trash",
                title : "Trash",
                icon : "img/trash.gif",
                order : 4,
                _actionCancel : {"delete":true,"edit":true},
                block : {addIn : false,removeFrom : false,delObj : true,moveObj: true}
            } ],
            exapndSize: 8
        },        
        _create: function () {
            oThis = this;
            
            oThis.options.container = "directoryWidgetDiv" + parseInt(Math.random() * 100000);
            oThis.element.append($(document.createElement("div")).attr("id", oThis.options.container)); // add random number generation to the end of this 
            
            oThis.options.data = verarr(oThis.options.data);
            $(oThis.options.data).each(function(i, dsName){
                vjDS[dsName].register_callback(oThis.allReady);
                vjDS[dsName].load();
            });            
            
            if (this.options.qryLang) this.options.idtype = '_id';
            if (this.options.dropOnAttrs === undefined) this.options.dropOnAttrs = this.options.dropableAttrs;
            if (this.options.currentFolder === undefined) this.options.currentFolder = "";
            if (this.options.preselectedFolder === undefined) this.options.preselectedFolder = docLocValue("folder");
            if (this.options.preselectedFolder === "") this.options.preselectedFolder = undefined;
            
            this.options.expandSize = 8;
            this.options.className = "directory";
            
            vjDS.add("","ds_"+oThis.options.container+"_signals","static://",function(param,text,page_request){oThis.doneChildren(param, text,page_request);});
            
            //need to change this, since callbacks/events will be captured differently
            if (this.options.qreyLang) this.options.selectCaptureCallback = oThis.onSelectCaptureCallback_QryLang;
            else this.options.selectCaptureCallback = oThis.onSelectCaptureCallback;
        },
        allReady: function(viewer, content){
            if (++oThis.options.dataCounter < oThis.options.data.length)
                return;                
           
            oThis.refresh(content);
        },
        refresh: function (content){
            if (oThis.topPanel) oThis.topPanel.refresh();
            else {
                oThis.topPanel = new vjPanelView({
                    data : ["dsVoid"].concat (oThis.options.data),
                    formObject : $("[name='"+oThis.options.formName+"']"),
                    iconSize : 24,
                    hideViewerToggle : true,
                    showTitles : false,
                    rows : [
                            { name : new RegExp(/.*/), hidden : true },
                            { name : 'folderActions', align: "right", order:30, hidden : false, title : "Edit", showTitle:true, icon:"folder-process", align : 'right', prohibit_new : true },
                            { name : 'subfolder', path :'/folderActions/subfolder', hidden : false, showTitle:true, prompt : "Name of the folder", prohibit_new : true },
                            { name : 'edit', align: "right", order:30, path :'/folderActions/edit', hidden : false, showTitle:true, prohibit_new : true },
                            { name : 'sep', path : '/folderActions/sep', order: 5, type : 'separator' },
                            { name : 'share', path :'/folderActions/share', hidden : false, showTitle:true, prohibit_new : true },
                            { name : 'delete', path :'/folderActions/delete', showTitle:true, hidden : false, prohibit_new : true },
                            { name : 'copy', hidden:false, showTitle:true, inactive:true, prohibit_new : true, url: oThis.copy },
                            { name : 'cut', showTitle:true, hidden : false, inactive:true, prohibit_new : true, url: oThis.cut },
                            { name : 'paste', hidden : false, refreshDelay:1000, inactive:'javascript:gClip.content.length? false :true', prohibit_new : true, url: oThis.paste},
                            { name : 'undo', align : 'left', order : -2, title : 'Undo', description : 'Undo', url : oThis.undo, icon : 'back', prohibit_new : true },
                            { name : 'redo', align : 'left', order : -1, title : 'Redo', description : 'Redo', showTitles:true, url: oThis.redo, icon : 'img/forward.png', prohibit_new : true },
                            { name : 'refresh', align : 'left', order : 0, title : 'Refresh', description : 'refresh the content of the control to retrieve up to date information', url: oThis.refreshAllDS, icon : 'refresh.png', icon_srcset : [24, 48], prohibit_new : true } 
                            ],
                    isok : true
                });
                $("#"+oThis.options.container).dataview({instance: oThis.topPanel});
            }
            
            if (oThis.options.treeDiv){
                oThis.options.treeDiv.treewidget("allReady", content);
            }else{
                var optionsForTree = oThis.options;
                optionsForTree.contextMenuOptions = [
                    { name: "cut", title: "Cut", callback: oThis.cut },
                    { name: "copy", title: "Copy", callback: oThis.copy },
                    { name: "paste", title: "Paste", callback: oThis.paste }
                ];
                optionsForTree.preTreatNodesBeforeParsing = oThis.preTreatDirTree;
                optionsForTree.dragCallonStart = oThis.cut;
                optionsForTree.dragCallonStop = oThis.paste;
                oThis.options.treeDiv = $("#"+oThis.options.container).treewidget(optionsForTree);
            }
            oThis.dirTree = oThis.options.treeDiv.treewidget ("getTree");
        },
        undo: function (viewer, node){
            if (!oThis.options.HistoryStack.length) return;
            
            var params = oThis.options.HistoryStack.pop();
            params.rollback = true;

            if (isok(params.urls.rollback)){
                vjDS["ds_"+oThis.options.container+"_signals"].reload(params.urls.rollback,true,params);
            }
            else
                oThis.doneChildren({
                    objCls : oThis.options.objCls,
                    callback : 'doneChildren',
                    params : params
                }, "");
            oThis.updateHistory (params);
        },
        redo: function (viewer, node){
            alert("redo was pressed");            
        },
        preTreatDirTree: function(tbl, content)
        {
            for ( var is = 0; is < oThis.options.systemFolderList.length; ++is) {
                var sys = oThis.options.systemFolderList[is];

                var node;
                if (!sys.isVirtual) {
                    node = tbl.accumulate("(node.name=='" + sys.name + "' && node._type=='sysfolder') ", "node");
                    if((node instanceof Array) && node.length==1)
                        node=node[0];
                    if (node && (node instanceof Array)){
                        var t_node=[];
                        for(var i = 0 ; i < node.length ; ++i ){
                            var tt_nodes=tbl.accumulate("verarr(node.child).indexOf('"+node[i].id+"')!=-1",node[i].id);
                            if(!tt_nodes || ((tt_nodes instanceof Array) && !tt_nodes.length) )
                                t_node.push(node[i]);
                        }
                        node = t_node[0];
                    }
                    if (!node) continue;
                    if (!sys.isVirtual) {
                        node.icon = sys.icon;
                        node.expanded = 0;
                        node.order = sys.order;
                        if (sys.title) node.title = sys.title;
                        if (sys.leafnode) node.leafnode = true;
                        if(sys._actionCancel) node._actionCancel = sys._actionCancel;
                        if (sys.isNexpandable!==undefined) node.isNexpandable = sys.isNexpandable;
                    }
                    if (node.type_count) node.type_count = JSON.parse(node.type_count);
                } else {
                    tbl.rows.push(sys);
                    node = sys;
                }
                
                if (node.child && !(node.child instanceof Array)) node.child = node.child.split(",");
                oThis.options.systemFolder[sys.name] = node;
                if(sys.block.addIn) oThis.options.systemFolderExclude.addIn.push(node);
                if(sys.block.removeFrom) oThis.options.systemFolderExclude.removeFrom.push(node);
                if(sys.block.delObj) oThis.options.systemFolderExclude.delObj.push(node);
                if(sys.block.moveObj) oThis.options.systemFolderExclude.moveObj.push(node);
            }

            oThis.options.precompute = "if(!node.isVirtual){node.title=node.name;node.name=node['" + oThis.options.idtype + "'];}"+
                "if(params.cacheDirectionyExpansionStatus[node.id])node.expanded=params.cacheDirectionyExpansionStatus[node.id];";

            oThis.options.postcompute = "node.iconSize=" + oThis.options.folderSize
                    + ";if(!node.icon)node.icon='img/folder-hive.gif';"
                    + "params.childSorter(node); if(node.path=='"+oThis.options.currentFolder+"'){var tt=node;while(tt.parent){tt=tt.parent;tt.expanded=1;}}";
        },
        
        
        
        // Hierarchy Alterations
        copy: function (eventNode)
        {
            /*
             * eventNode has the following structure: 
             * {
             *         element:{}, // the html element (the drop down menu itself) 
             *         item:{}, //the item from the drop down that was selected
             *         position:{}, //the x&y coordinates of either the click or the menu itself. not sure 
             *         reference:{} //this has information about what element was selected (right clicked on), and its parents. the actually useful thing!
             * }
             */
            var curNodeId = eventNode.reference.prevObject[0].id; // the id of the node is its path
            var lastIndex = curNodeId.lastIndexOf("/");
            
            oThis.options.operation = "copy";
            oThis.options.selectedNode.push(oThis.dirTree.findByPath(curNodeId));
            oThis.options.parentNode = oThis.dirTree.findByPath(curNodeId.substring(0, lastIndex > -1? lastIndex : curNodeId.length));
        },
        cut: function (eventNode, fromDND)
        {
            //if the even was triggered from drag and drop callback
            if (fromDND){
                var curNodeId = fromDND.data.nodes[0]; // the id of the node is its path
                var lastIndex = curNodeId.lastIndexOf("/");
                
                oThis.options.operation = "cut";
                oThis.options.selectedNode.push(oThis.dirTree.findByPath(curNodeId));
                oThis.options.parentNode = oThis.dirTree.findByPath(curNodeId.substring(0, lastIndex > -1? lastIndex : curNodeId.length));
                
                return;
            }
            
            /*
             * eventNode has the following structure: 
             * {
             *         element:{}, // the html element (the drop down menu itself) 
             *         item:{}, //the item from the drop down that was selected
             *         position:{}, //the x&y coordinates of either the click or the menu itself. not sure 
             *         reference:{} //this has information about what element was selected (right clicked on), and its parents. the actually useful thing!
             * }
             */
            var curNodeId = eventNode.reference.prevObject[0].id; // the id of the node is its path
            var lastIndex = curNodeId.lastIndexOf("/");
            
            oThis.options.operation = "cut";
            oThis.options.selectedNode.push(oThis.dirTree.findByPath(curNodeId));
            oThis.options.parentNode = oThis.dirTree.findByPath(curNodeId.substring(0, lastIndex > -1? lastIndex : curNodeId.length));
        },
        paste: function (eventNode, fromDND){
            if (fromDND) oThis.options.destinationNode = oThis.dirTree.findByPath(fromDND.event.target.parentElement.id);
            else oThis.options.destinationNode = oThis.dirTree.findByPath(eventNode.reference.prevObject[0].id);
            
            if(oThis.options.operation == "copy") return oThis.copyChildren(oThis.options.parentNode,oThis.options.destinationNode,oThis.options.selectedNode);
            else if (oThis.options.operation == "cut") return oThis.cutChildren(oThis.options.parentNode,oThis.options.destinationNode,oThis.options.selectedNode);
        },
        onContinue: function (params){
            var movingChildren = params.movingChildren;
            var sourceFolder = params.sourceFolder;
            var destinationFolder = params.destinationFolder;
            var action = params.action;
            
            movingChildren = verarr(movingChildren);
            var next_currentFolder = oThis.options.currentFolder;
            oThis.options.selectedNodeContentChanged = false;

            var url_do = "http://?cmd=obj"+action+"&raw=1";
            var url_undo = url_do;
            if (action == "Copy") url_undo = "http://?cmd=objCut&raw=1";

            var srcFolderName;

            var sourceFolderPath = "";
            var historyReset = false;
            if (sourceFolder) sourceFolderPath = sourceFolder.path;
            if( !sourceFolder  ) srcFolderName = "root"; else srcFolderName = sourceFolder.id;

            url_do += "&src=" + srcFolderName;
            url_undo += "&src=" + destinationFolder.id;

            url_do += "&dest=" + destinationFolder.id;
            url_undo += "&dest=" + srcFolderName;

            if (sourceFolderPath == oThis.options.currentFolder || destinationFolder.path == oThis.options.currentFolder)
                oThis.options.selectedNodeContentChanged = true;

            var movingChlId = [];
            for (var i = 0; i < movingChildren.length; ++i) {
                var obj = movingChildren[i];
                if (obj && obj.path && oThis.options.currentFolder.indexOf(obj.path) == 0)
                    next_currentFolder = next_currentFolder.replace(sourceFolderPath, dstFolder_obj.path);
                
                if (movingChlId.indexOf(obj.id) < 0) movingChlId.push(obj.id);
            }

            url_do += "&ids=" + movingChlId.join(",");
            url_undo += "&ids=" + movingChlId.join(",");

            var url = {
                exec : url_do,
                rollback : url_undo
            };

            var params = {
                sourceFolder : srcFolderName,
                destinationFolder : destinationFolder.id,
                movedIDs : movingChlId,
                urls : url,
                next_currentFolder : {
                    exec : next_currentFolder,
                    rollback : oThis.options.currentFolder
                },
                resetHistory : historyReset,
                rollback : false
            };

            oThis.options.DoStack = [];

            if (isok(url.exec)) {
                vjDS["ds_"+oThis.options.container+"_signals"].reload(url.exec,true,params);
            } /*else {
                oThis.doneChildren({
                    callback : 'doneChildren',
                    params : params
                }, "");
            }*/
            
            oThis.updateHistory (params);
        },
        cutChildren: function (sourceFolder, destinationFolder, movingChildren)
        {
            var param = {action: "Cut", sourceFolder: sourceFolder, destinationFolder: destinationFolder, movingChildren: movingChildren};
            if (sourceFolder == oThis.options.systemFolder.all.id || sourceFolder===undefined) {
                oThis.contructDialog(param);
                return;
            }
            oThis.onContinue (param);
        },
        copyChildren: function (sourceFolder, destinationFolder, movingChildren)
        {
            var param = {action: "Copy", sourceFolder: sourceFolder, destinationFolder: destinationFolder, movingChildren: movingChildren};
            if (sourceFolder == oThis.options.systemFolder.all.id || sourceFolder===undefined) {
                oThis.contructDialog(param);
                return;
            }
            oThis.onContinue (param);
        },
        doneChildren: function (params, content, page_request) {
            if( oThis.options.onDoneChildren ) {
                funcLink(oThis.options.onDoneChildren, params, content,page_request);
            }
            else {
                oThis.updateHistory(params,content,page_request);
                if (oThis.options.actionsHandler) {
                    oThis.options.actionsHandler(content, page_request);
                }
                //function to refresh all DS needs to be written and called here
                oThis.refreshAllDS();
            }
        },
        _checkObjForObj (target, object){
            for (var key in object){
                var returned = true;
                
                if (object[key] instanceof Array && item[key]) returned = oThis._checkArrayForObjects (item[key], object[key]);
                if (!returned) return false;
                
                if (object[key] instanceof Object && item[key]) returned = oThis._checkObjForObj (item[key], object[key]);
                if (!returned) return false;
                
                if (!target[key] || target[key] != object[key]) return false;
            }
            return true;
        },
        _checkArrayForObject(array, object){
            return $(array).each(function (i, item){
                for (var key in object){
                    var returned = true;
                    
                    if (object[key] instanceof Array && item[key]){
                        for (var ii = 0; ii < object[key].length; ii++){
                            returned = oThis._checkArrayForObject (item[key], object[key][ii]);
                            if (!returned) return false;
                        }
                        
                    } 
                    if (!returned) return false;
                    
                    if (object[key] instanceof Object && item[key]) returned = oThis._checkObjForObj (item[key], object[key]);
                    if (!returned) return false;
                    
                    if (!item[key] || item[key] != object[key]) return false;
                }
                return true;
            });
        },
        updateHistory: function (params, content, page_request){
            if(!params.resetHistory){
                //neeed to check if the params object has already been in the stack
                var ret1 = oThis._checkArrayForObject(oThis.options.HistoryStack, params);
                var ret2 = oThis._checkArrayForObject(oThis.options.DoStack, params);
                
                if(params.rollback && ret2.length == 0){
                    params.rollback=false;
                    oThis.options.DoStack.push(params);
                    oThis.options.currentFolder=params.next_currentFolder.rollback;
                }
                else if ( ret1.length == 0){
                    params.rollback=true;
                    oThis.options.HistoryStack.push(params);
                    oThis.options.currentFolder=params.next_currentFolder.exec;
                }
            }
            else {
                oThis.options.HistoryStack=[];
                oThis.options.DoStack=[];
            }
        },
        refreshAllDS: function(){
            $(oThis.options.data).each(function (i, curDS){
                vjDS[curDS].reload(vjDS[curDS].url, true);
            });
        },
        contructDialog: function (params){            
            var text = "You are trying to "+params.action+" from a "+ ((params.sourceFolder===undefined)?"root":"virtual")+ " folder. You cannot undo this action. \n\n Do you want to proceed?";
            
            if ($("#dialog") && $("#dialog").length == 1){
                $("#dialog").text(text);
                $("#dialog").dialog("open");
                return;
            }
            $("body").append(
                    $(document.createElement("div"))
                        .attr("id", "dialog")
                        .attr("title", "Continue Dialog")
                        .append (
                                $(document.createElement("p")).text(text)
                        )
                    );
                
                $("#dialog").dialog({
                    modal: true,
                    width: 500,
                    buttons: {
                        Yes: function() {
                            $(this).dialog("close");
                            oThis.onContinue(params);
                        },
                        No: function() {
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
        },
        //functions that might be needed later
        onSelectCaptureCallback: function(view, node) {
            //whatever will need to happen in the select capture
            
            if (oThis.options.onSelectCapture){
                oThis.options.onSelectCapture(view, node);
            }
        },
        onSelectedFolder: function(ajaxParams, content) {
            
        },
        onSelectCaptureCallback_QryLang: function(view, node) {
        },
        onSelectedFolder_QryLang: function(ajaxParams, content) {
        },
        addFolder: function(viewer, ttt) {
        },
        create_folder: function(folderName,folderDestination){
        },
        onCreated_folder: function (ajaxParams,content){
        }
    });
}(jQuery));
