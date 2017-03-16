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
// _/ constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/

function vjDirectoryView(viewer) {
    if(this.drag ===undefined)this.drag = true;
    this.idtype = 'id';
    if (this.qryLang)
        this.idtype = '_id';
    if (this.dropableAttrs === undefined)
        this.dropableAttrs = [ 'dragDropable' ];
    if (this.dropOnAttrs === undefined)
        this.dropOnAttrs = this.dropableAttrs;

    if (this.currentFolder === undefined)
        this.currentFolder = "";
    if(viewer.preselectedFolder===undefined)viewer.preselectedFolder = docLocValue("folder");
    if(viewer.preselectedFolder === "")viewer.preselectedFolder=undefined;

    if (this.isNshowIconEmpty === undefined)
        this.isNshowIconEmpty = true;
    if (this.alwaysSelected === undefined)
        this.alwaysSelected = true;
    this.HistoryStack = new Array();
    this.DoStack = new Array();
    if(!this.cacheDirectionyExpansionStatus)
        this.cacheDirectionyExpansionStatus=new Object ();
    this.expandNodeCallback="javascript:thisObj.cacheDirectionyExpansionStatus[args[1].id]=args[1].expanded;";

    this.systemFolder = {};

    this.systemFolderExclude = {
        addIn : [],
        removeFrom : [],
        delObj : [],
        moveObj : [],
        isNodeIn: function(node, list) {if (!this[list])return false;for ( var i = 0; i < this[list].length; ++i) {if (this[list][i].path == node.path)return this[list][i];}return false;},
        isPathIn: function(path, list) {if (!this[list])return false;for ( var i = 0; i < this[list].length; ++i) {if (this[list][i].path == path)                    return this[list][i];}return false;}
    };
    this.systemFolderList = [ {
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
    } ];


    //------------------------------------------------------------------------------------//
    //-------------------------------Tree Browsing Functions------------------------------//
    //------------------------------------------------------------------------------------//

    this.findByTitle = function (){

    };

    //------------------------------------------------------------------------------------//
    //-----------------------------------Drag Operations----------------------------------//
    //------------------------------------------------------------------------------------//
    this.excludeNonDroppables=function(){
        for(var i = 0 ; i < this.systemFolderExclude.addIn.length ; ++i) {
            this.toggleNonDropableElement(this.systemFolderExclude.addIn[i],true);
        }
    };
    this.toggleNonDropableSubTree=function(srcNode,mode,parent){
        if(typeof(srcNode)=='string')
            srcNode=this.tree.findByPath(srcNode);
        if(parent){
            srcNode=srcNode.parent;
        }
        if (srcNode) {
            this.tree.enumerate("if(node.path){var o=gObject(sanitizeElementId('"
                                    + this.container
                                    + "'+node.path));if(o)o=o.parentNode;if(o){o.setAttribute('dragged',"+mode+");}}",
                            null, null, null, srcNode);
        }
    };
    this.toggleNonDropableElement=function(srcNode,mode,parent){
        if(typeof(srcNode)=='string')
            srcNode=this.tree.findByPath(srcNode);
        var o=gObject(sanitizeElementId(this.container+srcNode.path));
        if(o)o=o.parentNode;
        if(o)o.setAttribute('dragged',mode);
        if(parent){
            srcNode=srcNode.parent;
            o=gObject(sanitizeElementId(this.container+srcNode.path));
            if(o)o=o.parentNode;
            if(o)o.setAttribute('dragged',mode);
        }
    };
    this.dragCallonStart = function(dragE) {
        var dragNode = this.findByDOM(dragE[0]);
        this.toggleNonDropableSubTree(dragNode,true);
        this.toggleNonDropableElement(dragNode,true,true);
        this.excludeNonDroppables();

        if (typeof (this.dragStartCallback) == "function")
            return this.dragStartCallback(dragE[0]);
        return true;
    };
    this.dragCallonMove = function(dragE) {

        if (typeof (this.dragMoveCallback) == "function")
            return this.dragMoveCallback(dragE[0]);
        return true;
    };
    this.dragCallonTarget = function(dragE, dropE) {

        if (typeof (this.dragTargetCallback) == "function")
            return this.dragTargetCallback(dragE[0], dropE);
        return true;
    };

    this.setClonedObject = function(dragE) {
        var clElem = null;
        if (typeof (this.dragSetCloneCallback) == "function")
            clElem = this.dragSetCloneCallback(this, dragE[0]);
        return clElem;
    };

    this.dragCallonDrop = function(dragE, dropE, copy) {
        var drag = this.findByDOM(dragE[0]);
        var drop = this.findByDOM(dropE);

        if (drag && drop) {
            var updateRemovedList = new Array();
            var dragParent = drag.parent;

            if (this.currentFolder == drop.path
                    || this.currentFolder == drag.parent.path)
                this.forceReSelect = true;
            if (dragParent.depth) {
                for ( var ic = 0; ic < dragParent.children.length; ++ic) {
                    if (dragParent.children[ic][this.idtype] != drag[this.idtype]) {
                        updateRemovedList
                                .push(dragParent.children[ic][this.idtype]);
                    }
                }
            }

            this.paste(dragParent[this.idtype],drop[this.idtype],drag[this.idtype],copy);
        }

        if (typeof (this.dragDropCallback) == "function")
            return this.dragDropCallback(dragE[0], dropE, copy);
        return true;
    };
    this.dragCallonStop = function(dragE) {
        var dragNode = this.findByDOM(dragE[0]);

        this.toggleNonDropableSubTree(dragNode,false);
        this.toggleNonDropableElement(dragNode,false,true);
        this.excludeNonDroppables();

        if (typeof (this.dragStopCallback) == "function")
            return this.dragStopCallback(dragE[0]);
        return true;
    };
    //------------------------------------------------------------------------------------//
    ////////////////////////////////////////////////////////////////////////////////////////


    //------------------------------------------------------------------------------------//
    //-----------------------------------Initialization-----------------------------------//
    //------------------------------------------------------------------------------------//

    this.expandSize = 8;
    this.className = "directory";
    vjTreeView.call(this, viewer);
    this.tree.findById = function( value , node) {
        return this.findByAttribute("id",value,node);
    };

    this.iconSize = this.expandSize;

    this.drag.dropOnAttrs = this.dropOnAttrs;
    this.drag.dropableAttrs = this.dropableAttrs;
    this.drag.setClonedObject = {
        func : this.setClonedObject,
        obj : this
    };



    //Overwrite icons handling functions

    this.scaleDist = 32;

    this.icons.leaf_nse ="img/tree-northsoutheast_slim_dotted.png" ;
    this.icons.leaf_ne = "img/tree-northeast_slim_dotted.png";
    this.icons.leaf_ns = "img/tree-northsouth_slim_dotted.png";
    this.icons.branch_nse_open = "img/tree-northsoutheast_plus_slim_dotted.png";
    this.icons.branch_nse_close = "img/tree-northsoutheast_minus_slim_dotted.png";
    this.icons.branch_ne_open = "img/tree-northeast_plus_slim_dotted.png";
    this.icons.branch_ne_close = "img/tree-northeast_minus_slim_dotted.png";
    this.icons.white_open = "img/tree-white_plus_slim_dotted.png";
    this.icons.white_close = "img/tree-white_minus_slim_dotted.png";


    this.setIcons.background_td = function(node){
        var t = " ";
        var depth_threshold = this.self.showRoot ? 1 : 0;
        if (node.depth > depth_threshold && !node.isLastSibling()) {
            t += "background-image:url(\""+this.self.icons.leaf_ns+"\");background-repeat:repeat-y;height:100%;";
        }
        return t;
    };
    this.setIcons.leaf = function(node) {
        t="";
        var depth_threshold = this.self.showRoot ? 1 : 0;
        if( node.depth>depth_threshold ){
            if( !node.isLastSibling() ) {
                t= "<img class='branchIcon' src='"+this.self.icons.leaf_nse+"' />";
            }
            else {
                t= "<img class='branchIcon' src='"+this.self.icons.leaf_ne+"' />";
            }
        }
        return t;
    };
    this.setIcons.branch = function(node,cntCh) {
        var t = "";
        var depth_threshold = this.self.showRoot ? 1 : 0;
        if(node.depth > depth_threshold){
            var is_expanded = (node.expanded>=this.self.expansion && node.children.length>0 && cntCh <= node.children.length);
            t+="<img class='branchIcon' ";
            if( !node.isLastSibling() ){
                t += "src='"+( is_expanded ? this.self.icons.branch_nse_close : this.self.icons.branch_nse_open)+"' />";
            }
            else {
                t += "src='"+( is_expanded ? this.self.icons.branch_ne_close : this.self.icons.branch_ne_open)+"' />";
            }
        }
        else {
            var is_expanded = (node.expanded>=this.self.expansion && node.children.length>0 && cntCh <= node.children.length);
            t+="<img class='branchIcon' " +
                    "src='"+( is_expanded ? this.self.icons.white_close : this.self.icons.white_open)+"' />";
        }
        return t;
    };


    //Overwrite CSS handling functions
    this.setClass.tr = function(state) {
        var t = " ";
//                if (!state) {
//                    if (this.self.className) {
//                        t = this.tag + "='" + this.self.className + "'";
//                    }
//                } else if (state == 1) {    //highlighted
//                    //we don't give default class for highlighted trs
//                } else if (state == 2) {    //selected
//                  //we don't give default class for selected trs
//                } else {
//                    return alert("DEV alert: not tristate flag");
//                }
            return t;
    };
    this.setClass.td = function(state) {
        var t = " ";
        if (!state) {
            t = " state=0 ";
        } else if (state == 1) {    //highlighted
            t = " state='highlighted' ";
        } else if (state == 2) {    //selected
            t = " state='selected' ";
        } else {
            return alert("Dev alert: not tristate flag");
        }
        return t;
    };
    this.setClass.nodeSelected = function(node,html_el) {

        var val = node.selected ? 'selected' : 0;
        html_el.setAttribute('state',val);
    };
    this.setClass.nodeHighlighted = function(node, html_el) {
        html_el.setAttribute('state','highlighted');
    };
    this.setClass.branchButton = function() {
        return "class='linker branchIcon'";
    };

    if (this.qreyLang)
        this.selectCaptureCallback = "function:vjObjFunc('onSelectCaptureCallback_QryLang','"
                + this.objCls + "')";
    else
        this.selectCaptureCallback = "function:vjObjFunc('onSelectCaptureCallback','"
                + this.objCls + "')";

    this.callbackRendered= "function:vjObjFunc('onRender','"+this.objCls+"')";

    this.preTreatNodesBeforeParsing = function(tbl, content)
    {
        for ( var is = 0; is < this.systemFolderList.length; ++is) {
            var sys = this.systemFolderList[is];

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
                if (!node)
                    continue;
                if (!sys.isVirtual) {
                    node.icon = sys.icon;
                    node.expanded = 0;
                    node.order = sys.order;
                    if (sys.title)
                        node.title = sys.title;
                    if (sys.leafnode)
                        node.leafnode = true;
                    if(sys._actionCancel) {
                        node._actionCancel = sys._actionCancel;
                    }
                    if (sys.isNexpandable!==undefined) {
                        node.isNexpandable = sys.isNexpandable;
                    }
                }
                if (node.type_count) {
                    node.type_count = JSON.parse(node.type_count);
                }
            } else {
                tbl.rows.push(sys);
                node = sys;
            }



            if (node.child && !(node.child instanceof Array))
                node.child = node.child.split(",");
//            var nameSysFolder = sys.name == "0" ? "All" :sys.name;
            this.systemFolder[sys.name] = node;
            if(sys.block.addIn)
                this.systemFolderExclude.addIn.push(node);
            if(sys.block.removeFrom)
                this.systemFolderExclude.removeFrom.push(node);
            if(sys.block.delObj)
                this.systemFolderExclude.delObj.push(node);
            if(sys.block.moveObj)
                this.systemFolderExclude.moveObj.push(node);
        }

        this.precompute = "if(!node.isVirtual){node.title=node.name;node.name=node['" + this.idtype + "'];}"+
            "if(params.cacheDirectionyExpansionStatus[node.id])node.expanded=params.cacheDirectionyExpansionStatus[node.id];";

        this.postcompute = "node.iconSize=" + this.folderSize
                + ";if(!node.icon)node.icon='img/folder-hive.gif';"
                + "params.childSorter(node); if(node.path=='"+this.currentFolder+"'){var tt=node;while(tt.parent){tt=tt.parent;tt.expanded=1;}}";



    };

    this.childSorter = function(node) {
        if (!node.children)
            return;
        node.children.sort(function(a, b) {
            var ao = (a.order === undefined) ? Number.POSITIVE_INFINITY : a.order;
            var bo = (b.order === undefined) ? Number.POSITIVE_INFINITY : b.order;
            var _a = (a.title === undefined) ? "" : a.title.toLowerCase();
            var _b = (b.title === undefined) ? "" : b.title.toLowerCase();
            if (ao > bo)
                return 1;
            else if (ao < bo)
                return -1;
            else
                return cmpNatural(_a, _b);
        });
    };
    //------------------------------------------------------------------------------------//
    ////////////////////////////////////////////////////////////////////////////////////////





    //------------------------------------------------------------------------------------//
    //--------------------------------Hierarchy Alterations-------------------------------//
    //------------------------------------------------------------------------------------//
    var that = this;
    this.signals_DS = vjDS.add("","ds_"+this.objCls+"_signals","static://",function(param,text,page_request){that.doneChildren(param, text,page_request);});
    this.signals_DS.register_callback( { 'obj' : this, 'param' : '', 'func' : this.waitOnLoadCallback} , "fetching");


    this.paste = function (src, dst , movingChildren, isCopy){
        if(isCopy) {
            return this.copyChildren(src,dst,movingChildren);
        }
        else{
            return this.cutChildren(src,dst,movingChildren);
        }
    };
    this._delete = function(src,movingChildren) {
        return this.deleteChildren(src,movingChildren);
    };

    this.deleteChildren = function(sourceFolder,movingChildren){

        movingChildren = verarr(movingChildren);
        var next_currentFolder = this.currentFolder;
        this.selectedNodeContentChanged = false;

        var sourceFolderPath = "";

        var srcFolder_obj = this.tree.findById(sourceFolder);
        var historyReset = false;
        if (srcFolder_obj) {
            sourceFolderPath = srcFolder_obj.path;
        }

        var url_do = "qpbg_http://?cmd=objRemove&raw=1";
        var url_undo = "http://?cmd=objCopy&raw=1";

        if (sourceFolder == this.systemFolder.all.id || !srcFolder_obj) {
            var res = confirm("You are trying to cut from a " + ((!srcFolder_obj)?"root":"virtual") +" folder. Object will be removed from all actual folders.\n "
                    + "    You cannot undo this action. \n\n Do you want to proceed?");
            if (!res) {
                return;
            } else {
                historyReset = true;
            }

            if( !srcFolder_obj  ) {
                sourceFolder = "root";
                historyReset = true;
                this.selectedNodeContentChanged = true;
            }
        }

        if( srcFolder_obj && srcFolder_obj.path.indexOf(this.systemFolder.Trash.path)==0 ){
            var res = confirm("Are you sure you want to permanently delete the selected objects? \nYou cannot undo this action.");
            if( !res ) {
                return ;
            } else {
                historyReset = true;
            }
        }

        url_do += "&src=" + sourceFolder;
        url_undo += "&src=" + this.systemFolder.Trash.id+"&dst="+sourceFolder;

        if (sourceFolderPath == this.currentFolder
                || this.systemFolder.Trash.path == this.currentFolder)
            this.selectedNodeContentChanged = true;

        for (var i = 0; i < movingChildren.length; ++i) {
            var obj = this.tree.findById(movingChildren[i]);

            if (obj && obj.path && this.currentFolder.indexOf(obj.path) == 0) {
                next_currentFolder = next_currentFolder.replace(
                        sourceFolderPath, this.systemFolder.Trash.path);
            }
        }

        url_do += "&ids=" + movingChildren.join(",");
        url_undo += "&ids=" + movingChildren.join(",");

        var url = {
            exec : url_do,
            rollback : url_undo
        };

        var params = {
            sourceFolder : sourceFolder,
            destinationFolder : this.systemFolder.Trash.id,
            movedIDs : movingChildren,
            urls : url,
            next_currentFolder : {
                exec : next_currentFolder,
                rollback : this.currentFolder
            },
            resetHistory : historyReset,
            rollback : false
        };

        this.DoStack = [];

        if (isok(url.exec)) {
//            ajaxDynaRequestPage(url.exec, {
//                objCls : this.objCls,
//                callback : 'doneChildren',
//                params : params
//            }, vjObjAjaxCallback);
            this.signals_DS.reload(url.exec,true,params);
        } else {
            this.doneChildren({
                objCls : this.objCls,
                callback : 'doneChildren',
                params : params
            }, "");
        }
    };

    this.cutChildren = function (sourceFolder,destinationFolder,movingChildren)
     {
        movingChildren = verarr(movingChildren);
        var next_currentFolder = this.currentFolder;
        this.selectedNodeContentChanged = false;

        var url_do = "http://?cmd=objCut&raw=1";
        var url_undo = url_do;

        var srcFolder_obj = this.tree.findById(sourceFolder);
        var dstFolder_obj = this.tree.findById(destinationFolder);

        var sourceFolderPath = "";
        var historyReset = false;
        if (srcFolder_obj) {
            sourceFolderPath = srcFolder_obj.path;
        }

        if (sourceFolder == this.systemFolder.all.id || !srcFolder_obj) {
            var res = confirm("You are trying to cut from a " + ((!srcFolder_obj)?"root":"virtual") +" folder. Object will be removed from all actual folders.\n "
                    + "    You cannot undo this action. \n\n Do you want to proceed?");
            if (!res) {
                return;
            } else {
                historyReset = true;
            }

            if( !srcFolder_obj  ) {
                sourceFolder = "root";
            }
        }


        url_do += "&src=" + sourceFolder;
        url_undo += "&src=" + destinationFolder;

        url_do += "&dest=" + destinationFolder;
        url_undo += "&dest=" + sourceFolder;

        if (sourceFolderPath == this.currentFolder
                || dstFolder_obj.path == this.currentFolder)
            this.selectedNodeContentChanged = true;

        for (var i = 0; i < movingChildren.length; ++i) {
            var obj = this.tree.findById(movingChildren[i]);

            if (obj && obj.path && this.currentFolder.indexOf(obj.path) == 0) {
                next_currentFolder = next_currentFolder.replace(
                        sourceFolderPath, dstFolder_obj.path);
            }
        }

        url_do += "&ids=" + movingChildren.join(",");
        url_undo += "&ids=" + movingChildren.join(",");

        var url = {
            exec : url_do,
            rollback : url_undo
        };

        var params = {
            sourceFolder : sourceFolder,
            destinationFolder : destinationFolder,
            movedIDs : movingChildren,
            urls : url,
            next_currentFolder : {
                exec : next_currentFolder,
                rollback : this.currentFolder
            },
            resetHistory : historyReset,
            rollback : false
        };

        this.DoStack = [];

        if (isok(url.exec)) {
//            ajaxDynaRequestPage(url.exec, {
//                objCls : this.objCls,
//                callback : 'doneChildren',
//                params : params
//            }, vjObjAjaxCallback);

            this.signals_DS.reload(url.exec,true,params);
        } else {
            this.doneChildren({
                objCls : this.objCls,
                callback : 'doneChildren',
                params : params
            }, "");
        }

    };

    this.copyChildren = function (sourceFolder,destinationFolder,movingChildren)
     {
        movingChildren = verarr(movingChildren);
        this.selectedNodeContentChanged = false;

        var url_do = "http://?cmd=objCopy&raw=1";
        var url_undo = "http://?cmd=objCut&raw=1";

        var historyReset = false;

        if (sourceFolder == this.systemFolder.all.id || sourceFolder===undefined) {
            var res = confirm("You are trying to copy from a "+ ((sourceFolder===undefined)?"root":"virtual")+ " folder. "
                    + "    You cannot undo this action. \n\n Do you want to proceed?");
            if (!res) {
                return;
            } else {
                historyReset = true;
            }
            if(sourceFolder===undefined) {
                sourceFolder = "root";
            }
        }

        url_do += "&src=" + sourceFolder;
        url_undo += "&src=" + destinationFolder;

        url_do += "&dest=" + destinationFolder;
        url_undo += "&dest=" + this.systemFolder.Trash.id;

        url_do += "&ids=" + movingChildren.join(",");
        url_undo += "&ids=" + movingChildren.join(",");

        var dstFolder_obj = this.tree.findById(destinationFolder);

        if (dstFolder_obj.path == this.currentFolder)
            this.selectedNodeContentChanged = true;

        var url = {
            exec : url_do,
            rollback : url_undo
        };

        var params = {
            sourceFolder : sourceFolder,
            destinationFolder : destinationFolder,
            movedIDs : movingChildren,
            urls : url,
            next_currentFolder : {
                exec : this.currentFolder,
                rollback : this.currentFolder
            },
            // folderToEdit:folderToEdit,
            resetHistory : historyReset,
            rollback : false
        };

        this.DoStack = [];

        if (isok(url.exec)) {
//            ajaxDynaRequestPage(url.exec, {
//                objCls : this.objCls,
//                callback : 'doneChildren',
//                params : params
//            }, vjObjAjaxCallback);
            this.signals_DS.reload(url.exec,true,params);
        } else {
            this.doneChildren({
                objCls : this.objCls,
                callback : 'doneChildren',
                params : params
            }, "");
        }
    };

    this.doneChildren = function (params,content,page_request) {
        if( this.onDoneChildren ) {
            funcLink(this.onDoneChildren, params, content,page_request);
        }
        else {
            this.updateHistory(params,content,page_request);
            if (this.actionsHandler) {
                this.actionsHandler(content, page_request);
            }
            this.refreshOnArrive();
        }
    };

    this.updateHistory = function (params,content, page_request)
    {
//        var params=ajaxParams.params;


        if(!params.resetHistory){
            if(params.rollback){
                params.rollback=false;
                this.DoStack.push(params);
                this.currentFolder=params.next_currentFolder.rollback;
            }
            else{
                params.rollback=true;
                this.HistoryStack.push(params);
                this.currentFolder=params.next_currentFolder.exec;
            }
        }
        else {
            this.HistoryStack=[];
            this.DoStack=[];
        }
    };

    //------------------------------------------------------------------------------------//
    ////////////////////////////////////////////////////////////////////////////////////////


    this.inTrash = function (curfolder) {
        if(!curfolder)curfolder = this.currentFolder;
        if( this.systemFolder.Trash && this.systemFolder.Trash.path && (curfolder+"/").indexOf(this.systemFolder.Trash.path+"/")==0 ){
            return true;
        }
        return false;
    };

    //------------------------------------------------------------------------------------//
    //----------------------------------Viewer Callbacks----------------------------------//
    //------------------------------------------------------------------------------------//
    
    
    this.refreshOnArrive = function() {
        var ds = this.dataSourceEngine[this.data[0]];
        if(!this.forceUrl)
            this.forceUrl = ds.url;
        ds.reload(this.forceUrl, true);
    };


    this.onRender = function() {
        var setFolder = undefined;
        var changeCur = false;
        setFolder = this.tree.findByPath(this.currentFolder);
        if (!setFolder) {
            changeCur = true;
        }
        if (!setFolder && this.preselectedFolder) {
            setFolder = this.tree.findById(this.preselectedFolder);
            if (!setFolder) {
                setFolder = this.tree.findByPath(this.preselectedFolder);
            }
            if (!setFolder) {
                setFolder = this.tree.findByPath(this.preselectedFolder,
                        this.tree.root, "title");
            }
        }
        if (setFolder && setFolder.path) {
            var autoexpand = this.autoexpand + 1;
            if (setFolder.depth > autoexpand && !setFolder.expanded) {
                var node = setFolder;
                while (node.depth) {
                    node.expanded = true;
                    node = node.parent;
                }
                this.refresh();
            }
            setFolder = setFolder.path;
        } else {
            setFolder = undefined;
        }
        if (!setFolder && this.systemFolder.Inbox) {
            setFolder = this.systemFolder.Inbox.path;
        }
        if (!setFolder) {
            setFolder = this.systemFolder.all.path;
        }
        if (changeCur) {
            this.currentFolder = changeCur;
        }
        this.onClickNode(this.container, setFolder);

    };

    this.onSelectCaptureCallback = function(view, node) {
        if (!node.selected) {
            this.currentFolder = "";
            return false;
        }
        this.currentFolder = node.path ? node.path : "";


        var url;
        if (node[this.idtype] == this.systemFolder.all.id)
            url = "?cmd=allStat&raw=1&mode=csv";
        else
            url = "?cmd=propget&ids=" + node[this.idtype]
                    + "&prop=name,type_count&raw=1&mode=csv";
        
        if( this.inTrash(this.currentFolder) ) {
            url=urlExchangeParameter(url, "showTrashed","1");
        }
        else {
            url=urlExchangeParameter(url, "showTrashed","-");
        }
        ajaxDynaRequestPage(url, {
            objCls : this.objCls,
            node : node,
            callback : 'onSelectedFolder'
        }, vjObjAjaxCallback);
    };

    this.onSelectedFolder = function(ajaxParams, content) {
        var tbl = new vjTable(content, 0, vjTable_hasHeader);

        // if(node[this.idtype]="all")

        ajaxParams.node.type_count = {};
        for ( var it = 0; it < tbl.rows.length; ++it) {
            if (tbl.rows[it].name != "type_count")
                continue;
            ajaxParams.node.type_count[tbl.rows[it].path] = tbl.rows[it].value;
        }
        if (this.onSelectFolder) {
            funcLink(this.onSelectFolder, this, ajaxParams.node, content);
        }
    };

    //------------(For query language)-------------//
    this.onSelectCaptureCallback_QryLang = function(view, node) {
        if (!node.selected) {
            this.currentFolder = "";
            return false;
        }
        this.currentFolder = node.path ? node.path : "";

        if (this.type_count_onFolder) {
            var url = '';
            if (node[this.idtype] == this.systemFolder.all.id)
                url = "?cmd=allStat&raw=1&mode=csv";
            else
                url = "?cmd=propget&ids=" + node[this.idtype]
                        + "&prop=name,type_count&raw=1&mode=csv";

            ajaxDynaRequestPage(url, {
                objCls : this.objCls,
                node : node,
                callback : 'onSelectedFolder_QryLang'
            }, vjObjAjaxCallback);
        } else {
            if (this.onSelectFolder) {
                funcLink(this.onSelectFolder, this, node, '');
            }
        }
    };
    this.onSelectedFolder_QryLang = function(ajaxParams, content) {
        var tbl = new vjTable(content, 0, vjTable_propCSV);
        ajaxParams.node.type_count = JSON.parse(tbl.rows[0].type_count);

        if (this.onSelectFolder) {
            funcLink(this.onSelectFolder, this, ajaxParams.node, content);
        }
    };
    //------------------------------------------------------------------------------------//
    ////////////////////////////////////////////////////////////////////////////////////////

    // ----------------------------------------------------------------------------------//
    // -------------------------------Panel Related operations---------------------------//
    // ----------------------------------------------------------------------------------//
    this.addFolder = function(viewer, ttt) {
        var folderName = prompt("Name the folder:", "New Folder");
        if (!isok(folderName))
            return false;

        var sysFolder=this.systemFolderExclude.isPathIn(this.currentFolder, 'addIn');
        if(sysFolder){
            alertI("You cannot add to \"" + sysFolder.title + "\" folder");
            return;
        }

        var selNode = isok(this.currentFolder) ? this.tree.findByPath(this.currentFolder) : undefined;
        var selName = undefined;
        if (selNode && isok(selNode.id))
            selName = selNode.id;

        if(!selName){
            alertI("Destination must be selected.");
            return ;
        }

        this.create_folder(folderName, selName);
    };

    this.create_folder = function(folderName,folderDestination){
        if(!folderDestination){
            alertI("Please select a destination for the new folder");
            return ;
        }
        var url_folderCreate="?cmd=propset&prop.newdir._type=folder&prop.newdir.name="+ folderName
        + "&prop." + folderDestination + ".child%2B=${newdir}";


        var ds = this.dataSourceEngine[this.data[0]];
        this.forceUrl = ds.url;
        ajaxDynaRequestPage(url_folderCreate, {
            objCls : this.objCls,
            callback : 'onCreated_folder'
        }, vjObjAjaxCallback);

    };
    this.onCreated_folder = function (ajaxParams,content){
//        var params=ajaxParams.params;
        if(content.indexOf("err")!=-1){
            var errcode=content.split(".");
            alertI("Cannot create object: "+errcode[1]+".\n");
            return;
        }
        this.HistoryStack=[];
        this.DoStack=[];

        this.selectedNodeContentChanged=true;
        this.refreshOnArrive();
    };
//
//    this.cleanChildLink=function(ajaxParams,content){
//        var parentIDs=verarr(ajaxParams.params.srcIDs);
//        var childIDs=verarr(ajaxParams.params.objIDs);
//        if(content.indexOf("err")!=-1){
//            alertI("WARNING: Cannot delete object(s).");
//            return;
//        }
//        var urlRemove="?cmd=propDel&raw=1";
//        var ids="ids=";
//        var prop="prop=child";
//        var val="val=";
//        for(var n=0; n < childIDs.length; ++n){
//            if(n)val+=",";
//            val+=childIDs[n];
//        }
//
//        for(var i=0 ; i<parentIDs.length ; ++i){
//            if(i) ids+=",";
//            ids+=parentIDs[i];
//        }
//        urlRemove+="&"+ids+"&"+prop+"&"+val;
//        ajaxDynaRequestPage(urlRemove, {objCls: this.objCls, callback:'onCleanedChildLinks'}, vjObjAjaxCallback);
//    };
//    this.onCleanedChildLinks=function(ajaxParams,content){
//        if(content.indexOf("err")!=-1){
//            alertI("WARNING: Cannot clean folder links).");
//            return;
//        }
//    };

    this.undo = function() {
        if (!this.HistoryStack.length) {
            alertI("Nothing to Undo");
            return;
        }
        var params = this.HistoryStack.pop();
        params.rollback = true;

        if (isok(params.urls.rollback)){
//            ajaxDynaRequestPage(params.urls.rollback, {
//                objCls : this.objCls,
//                callback : 'doneChildren',
//                params : params
//            }, vjObjAjaxCallback);
            this.signals_DS.reload(params.urls.rollback,true,params);
        }
        else
            this.doneChildren({
                objCls : this.objCls,
                callback : 'doneChildren',
                params : params
            }, "");
    };

    this.redo = function() {
        if (!this.DoStack.length) {
            alertI("Nothing to Redo");
            return;
        }

        var params = this.DoStack.pop();
        params.rollback = false;

        if (isok(params.urls.exec)){
//            ajaxDynaRequestPage(params.urls.exec, {
//                objCls : this.objCls,
//                callback : 'doneChildren',
//                params : params
//            }, vjObjAjaxCallback);
            this.signals_DS.reload(params.urls.exec,true,params);
        }
        else
            this.doneChildren({
                objCls : this.objCls,
                callback : 'doneChildren',
                params : params
            }, "");
    };

    //------------------------------------------------------------------------------------//
    ////////////////////////////////////////////////////////////////////////////////////////

}

function vjDirectoryControl(viewer) {
    if(!this.container)
        this.container="dvDirectory"+Math.random();
    
    this.objCls=this.container.replace(/\W/g, "_");
    vjObj.register(this.objCls,this);
    
    var arr = new Array("dsActions");
    arr = arr.concat(viewer.data);
    if(viewer.selectInsteadOfCheck===undefined)viewer.selectInsteadOfCheck=true;

    var viewerTree = new vjDirectoryView(viewer);

    var viewerPanel = new vjPanelView(
            {
                data : arr,
                formObject : viewer.formObject,
                iconSize : 24,
                hideViewerToggle : true,
//                actionCallback: viewer.actionCallback,
                evalVariables: {dcls:viewerTree.objCls},
                showTitles : false,
                rows : [{
                            name : new RegExp(/.*/),
                            hidden : true
                        },
                        {
                            name : 'folderActions',
                            hidden : false,
                            title : "Edit",
                            showTitle:true,
                            icon:"folder-process",
                            align : 'right',
                            prohibit_new : true
                        },
                        {
                            name : 'subfolder',
                            path :'/folderActions/subfolder',
                            hidden : false,
                            showTitle:true,
                            prompt : "Name of the folder",
//                            refreshDelay : 1000,
//                            refreshDelayCallback:viewer.refreshDelayCallback,
                            prohibit_new : true
                        },
                        {
                            name : 'edit',
                            path :'/folderActions/edit',
                            hidden : false,
                            showTitle:true,
                            prohibit_new : true
                        },
                        {
                            name : 'sep',
                            path : '/folderActions/sep',
                            order: 5,
                            type : 'separator'
                        },
                        {
                            name : 'share',
                            path :'/folderActions/share',
                            hidden : false,
                            showTitle:true,
                            prohibit_new : true
                        },
                        {
                            name : 'delete',
                            path :'/folderActions/delete',
                            showTitle:true,
                            hidden : false,
//                            refreshDelay:1000,
                            prohibit_new : true
                        },
                        {
                            name : 'copy',
                            hidden:false,
                            showTitle:true,
                            inactive:true,
                            prohibit_new : true
                        },
                        {
                            name : 'cut',
                            showTitle:true,
                            hidden : false,
                            inactive:true,
                            prohibit_new : true
                        },
                        {
                            name : 'paste',
                            hidden : false,
                            refreshDelay:1000,
                            inactive:'javascript:gClip.content.length? false :true',
                            prohibit_new : true
                        },
                        {
                            name : 'undo',
                            align : 'right',
                            order : 200,
                            title : 'Undo',
                            path: '/undo',
                            description : 'Undo',
                            url : "function:vjObjFunc('undo','"
                                    + viewerTree.objCls + "')",
                            icon : 'back',
                            prohibit_new : true
                        },
                        {
                            name : 'redo',
                            align : 'right',
                            order : 3,
                            path: '/undo/redo',
                            title : 'Redo',
                            description : 'Redo',
                            showTitles:true,
                            url : "function:vjObjFunc('redo','"
                                    + viewerTree.objCls + "')",
                            icon : 'img/forward.png',
                            prohibit_new : true
                        },
                        {
                            name : 'refresh',
                            align : 'left',
                            order : 0,
                            title : 'Refresh',
                            description : 'refresh the content of the control to retrieve up to date information',
                            url : "javascript:vjDS['" + viewer.data + "'].state=\"\";vjDS['" + viewer.data + "'].load();",
                            icon : 'refresh.png',
                            icon_srcset : [24, 48],
                            prohibit_new : true
                        } ],
                isok : true
            });

    if(viewerTree.selectInsteadOfCheck){
        if(viewerTree.objectsDependOnMe===undefined)
            viewerTree.objectsDependOnMe=[viewerPanel];
        if(viewerTree.actionsOnLeaves===undefined)viewerTree.actionsOnLeaves=true;
    }
    gClip.objectsDependOnMe.push(viewerPanel);
    return [ viewerPanel, viewerTree ];

}

//# sourceURL = getBaseUrl() + "/js/vjDirectoryView.js"