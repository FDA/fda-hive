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

function vjExplorerBaseView(viewer)
{
    this.clone=function (viewer)
    {
        if(!viewer)viewer=this;
        var i=0;
        for (i in viewer ) {
            this[i] = viewer[i];
        }

    };
    this.clone(viewer);
    if(this.preselectedFolder === undefined) {
        this.preselectedFolder = docLocValue("folder");
    }if(!this.preselectedFolder) {
        if(this.useCookies) {
            this.preselectedFolder=cookieGet("explorer_curdir_open",undefined);
        }
    }if(!this.preselectedFolder) {
        this.preselectedFolder=user_getvar("curdir_open",undefined);
    }
    if(!this.visitedFolders) {
        this.visitedFolders = cookieGet("explorer_visited_folders",undefined);
        if(this.visitedFolders)this.visitedFolders = this.visitedFolders.split(',');
    }

    if(!this.idtype)
        this.idtype='id';
        //Register the viewer
    if(!this.container)
        this.container="dvExplorer"+Math.random();
    this.objCls=this.container.replace(/\W/g, "_");
    vjObj.register(this.objCls,this);
    //Generate DV names out to container or use the names provided
    if(!this.folderURL){
        if(this.qryLang){
            this.folderURL =  "http://?cmd=objQry&qry=";
            var objQrytxt="alloftype('sysfolder|folder')";
            if(this.partialFolderLoading ){
                objQrytxt="[alloftype('sysfolder'),(alloftype('sysfolder')";
                if(this.visitedFolders && this.visitedFolders.length)
                    objQrytxt+= ".append(["+this.visitedFolders.join(',')+"] as objlist) ";
                objQrytxt+=") .map({.child = (.child as objlist).filter({._type=='folder'})}).reduce(function(x,y){x.append(y)})]";
                objQrytxt+=".reduce(function(x,y){x.append(y)})" ;
            }
            objQrytxt+=".csv(['name','child'])";
            this.folderURL += vjDS.escapeQueryLanguage(objQrytxt);
        }
        else
            this.folderURL =  "http://?cmd=objList&type=sysfolder,folder&actions=1&cnt=1000000&prop=_type,name,child&mode=csv&showtrash=true";
    }
    if(!this.tablesUrlTmplt) {
        this.tablesUrlTmplt = undefined;
    }

    this.currentFolder="";
    if(this.folderIcon)this.folderIcons={folder:"folder-open.gif",all:"all_folder.png",Inbox:"mail_box.png",Trash:"trash_folder.png"};
    if(!this.container_Folders)this.container_Folders=this.container+"_Folders";
    if(!this.container_Tables)this.container_Tables=this.container+"_Tables";
    if(!this.container_Previews)this.container_Previews=this.container+"_Previews";
    if(!this.container_Submit)this.container_Submit=this.container+"_Submit";
    
    if(!this.formFolders)this.formFolders="form-"+this.container_Folders;
    if(!this.formTables)this.formTables="form-"+this.container_Tables;
    if(!this.formPreviews)this.formPreviews="form-"+this.container_Previews;
    
    if(!this.vjDS) this.vjDS = vjDS;
    if(!this.vjDV) this.vjDV = vjDV;
    if(this.drag === undefined ) this.drag = true;
    if(this.autoexpand===undefined)this.autoexpand=1;

    if( this.isSubmitMode ) {
        this.isDisplay_Submit = true;
    }

    this.flV={
            name: this.container,
            body:"<table border='0' width='100'% class='HIVE_bar'>" +
                    "<tr><td class='HIVE_sect1' width='1%' valign=top rowspan='2'>"+
                    "%%Folders%%"+
                "</td><td class='HIVE_sect1' valign=top>"+
                    "%%Tables%%"+
                "</td></tr><tr><td  class='HIVE_sect1' valign=top colspan=2>"+
                    "%%Previews%%"+
                "</td></tr></table>",
            onlyPopup:true, role: 'output',  align: 'left'
    };

    // Will return the vjTableView of the right section from the (tabI)th tab

    this.init=function ( )
    {

        // if the DIVs are not defined ... generate them on the fly
        var cnt=0;
        var cont_Div = gObject(this.container);
        var cont_Folders_Div = gObject(this.container_Folders);
        var cont_Tables_Div = gObject(this.container_Tables);
        var cont_Previews_Div = gObject(this.container_Previews);
        var cont_Submit_Div = gObject(this.container_Submit);
        if( cont_Div && !cont_Folders_Div && !cont_Tables_Div && !cont_Previews_Div){
            var html = "<table border='0' width='100%' class='HIVE_bar'>";
            html +="<tr>";
            if( !this.isNdisplay_Info ) {
                var hrf="";//"javascript:var a=gObject('"+this.objCls+"Info');alerJ('aa',a);if(a)a.style.display='none';";
                html = "<table border='0' width='100%' class='HIVE_bar'>";
                /*
                html +="<tr id='"+this.objCls+"Info' class='Explorer_info' onclick='javascript:this.style.display=\"none\";'>";
                html += "<td class='HIVE_sect1' width='1%' style='vertical-align: top;' colspan='2'>" +
                                "<img src='img/delete.gif' height='10' border='0' style='float:left;cursor:pointer;'>" +
                        "Press and hold Ctrl or Shift button to select multiple rows" +
                                "</td></tr><tr>";*/
                html+="<tr>";
            }
            html += "<td class='HIVE_sect1' style='vertical-align: top;' "+(this.isNdisplay_Previews?"":"rowspan='2'")+"'>";
            var formObj = document.forms[this.formFolders];
            if( !formObj )html += "<form name='"+this.formFolders+"' >" ;
            html+="<span id='"+this.container_Folders+"'></span>" ;
            if( !formObj )html += "</form>";
            html += "</td>";
            if( !this.isNdisplay_Tables ) {
                html += "<td class='HIVE_sect1' width='1%' style='vertical-align: top;'>" ;
                var formObj = document.forms[this.formTables];
                if( !formObj )html += "<form name='"+this.formTables+"' >" ;
                html+="<span id='"+this.container_Tables+"'></span>" ;
                if( !formObj )html += "</form>";
                html += "</td>";
            }
            html +="</tr>";
            if( !this.isNdisplay_Previews ) {
                html += "<tr><td class='HIVE_sect1' width='1%' >";
                var formObj = document.forms[this.formPreviews];
                if( !formObj )html += "<form name='"+this.formPreviews+"' >" ;
                html+="<span id='"+this.container_Previews+"'></span>" ;
                if( !formObj )html += "</form>";
                html += "</td></tr>";
            }
            if( this.isDisplay_Submit ) {
                html +="<tr><td class='HIVE_sect1' style='vertical-align:bottom;' colspan='2'>";
                var formObj = document.forms[this.formSubmit];
                if( !formObj )html += "<form name='"+this.formSubmit+"' >" ;
                html+="<span id='"+this.container_Submit+"' style='display:block;'></span>" ;
                if( !formObj )html += "</form>";
                html += "</td></tr>";
            }
            html += "</table>";
            cont_Div.innerHTML = html;
        }

        cont_Div = gObject(this.container);
        cont_Folders_Div = gObject(this.container_Folders);
        cont_Tables_Div = gObject(this.container_Tables);
        cont_Previews_Div = gObject(this.container_Previews);
        cont_Submit_Div = gObject(this.container_Submit);
        if( cont_Folders_Div ){
            if(this.isNdisplay_Folders) {
                cont_Folders_Div.className = 'sectHid';
            }
            this.folders_DV=this.vjDV.add(this.container_Folders, this.folders_DV_attributes.width, this.folders_DV_attributes.height);
            for(var attr in this.folders_DV_attributes){
                if(attr=='width' || attr=='height') continue;
                this.folders_DV[attr]=this.folders_DV_attributes[attr];
            }
        }
        if(gObject(this.container_Tables) && !this.isNdisplay_Tables){
            this.tables_DV=this.vjDV.add(this.container_Tables, this.tables_DV_attributes.width, this.tables_DV_attributes.height);
            this.tables_DV.callbackTabSelect="function:vjObjFunc('tablesTabSelectedCallback','"+this.objCls+"')";
            for(var attr in this.tables_DV_attributes){
                if(attr=='width' || attr=='height') continue;
                this.tables_DV[attr]=this.tables_DV_attributes[attr];
                if( attr=="frame" && this.tables_DV_attributes[attr]=="notab" ) {
                    this.isSingleTab = true;
                }
            }
            this.tables_DV.callbackDVrendered = "function:vjObjFunc('tablesTabRendered','"+this.objCls+"')";
            if( this.useCookies ) {
                var tab_selected = cookieGet('explorer_curtab_open', 0);
                if( tab_selected && cookieGet("explorer_curdir_open",undefined)) {
                    this.tables_DV.selected = tab_selected;
                }
            }
        }
        if(gObject(this.container_Previews) && !this.isNdisplay_Previews){
            this.previews_DV=this.vjDV.add(this.container_Previews, this.previews_DV_attributes.width, this.previews_DV_attributes.height);
            for(var attr in this.previews_DV_attributes){
                if(attr=='width' || attr=='height') continue;
                this.previews_DV[attr]=this.previews_DV_attributes[attr];
            }
        }
        
        if(gObject(this.container_Submit) && this.isDisplay_Submit){
            this.submit_DV=this.vjDV.add(this.container_Submit, this.submit_DV_attributes.width, this.submit_DV_attributes.height);
            this.submit_DV.frame="notab";
            for(var attr in this.submit_DV_attributes){
                if(attr=='width' || attr=='height') continue;
                this.submit_DV[attr]=this.submit_DV_attributes[attr];
            }
        }

        // init Data Sources
        this.init_DS();

        if(this.folders_DV)
            this.init_Folders_viewers();
        if(this.tables_DV)
            this.init_Tables_viewers();
        if(this.previews_DV)
            this.init_Previews_viewers();
        if(this.submit_DV)
            this.init_Submit_viewers();
    };
    this.init_DS=function()
    {
        this.dsFolders=this.vjDS.add("infrastructure: Folders", "ds"+this.container+"Folders", this.folderURL);
        this.dsFoldersToolbar=this.vjDS.add("infrastructure: Folders Toolbar", "ds"+this.container+"FoldersToolbar","static:// ");
        this.dsFoldersHelp=this.vjDS.add("infrastructure: Folders Help", "ds"+this.container+"FoldersHelp","http://help/hlp.page.newhome.html");
        this.dsTablesHelp=this.vjDS.add("infrastructure: Files and sequences help documentation", "ds"+this.container+"TablesHelp","http://help/hlp.page.home.filesAndSequences.html");
        this.dsPreviewsHelp=this.vjDS.add("infrastructure: Preview help documentation", "ds"+this.container+"PreviewsHelp","http://help/hlp.page.home.filesAndSequences.html");
        this.dsPreviewsRecord=this.vjDS.add("infrastructure: Loading Objects Metadata Information", "ds"+this.container+"PreviewsRecord" , "static://preview:Select objects to see detail metadata" );
        this.dsPreviewsJson=this.vjDS.add("infrastructure: Loading Objects Metadata Information (JSON)", "ds"+this.container+"PreviewsJson" , "static://preview:Select objects to see detail JSON metadata");
        this.dsPreviewsSpec=this.vjDS.add("infrastructure: Obect Specifications","ds"+this.container+"PreviewsSpec" , "static:// " );
        this.dsPreviewsUserTree = this.vjDS.add("infrastructure: Retrieving User/Group Hierarchy", "ds"+this.container+"UserTree", "http://?cmd=grpList&raw=1&usr=1&grp=1");
        this.dsPreviewsUserList = this.vjDS.add("infrastructure: Retrieving User List", "ds"+this.container+"UserList", "http://?cmd=usrList&grp=0raw=1");
    };

    this.destroyDS=function()
    {
        delete this.vjDS[this.dsFolders.name];delete this.vjDS[this.dsFoldersToolbar.name];delete this.vjDS[this.dsFoldersHelp.name];delete this.vjDS[this.dsTablesHelp.name];
        delete this.vjDS[this.dsPreviewsHelp.name];delete this.vjDS[this.dsPreviewsRecord.name];delete this.vjDS[this.dsPreviewsJson.name];delete this.vjDS[this.dsPreviewsSpec.name];

        delete this.vjDS[this.dsPreviewsUserTree.name];
        delete this.vjDS[this.dsPreviewsUserList.name];

        for (var i=0 ; i < this.viewerFSTable.length ; ++i ) {
            delete this.vjDS[this.viewerFSTable[i].data];
        } 
    };

    var that = this;
    this.init_Folders_viewers=function()
    {

        var dirViewers = new vjDirectoryControl ({
            name : 'hierarchy',
            icon : 'tree',
            drag : this.drag,
            qryLang:this.qryLang,
            partialFolderLoading:this.partialFolderLoading,
            visitedFolders:this.visitedFolders,
            data : this.dsFolders.name,
            hierarchyColumn : 'path',
            highlightRow : true,
            expandSize : 12,
            folderSize : 24,
            showRoot : 0,
            showLeaf : true,
            hideEmpty : false,
            refreshDelayCallback:"vjObjEvent('onRefreshDelayTables','"+this.objCls+"',$(id))",
            preselectedFolder : this.preselectedFolder,
            setDragElementsOperation:"if(node.path && node._type!='sysfolder'){var o=gObject(sanitizeElementId(params.that.container+node.path));" +
                    "if(o)o=o.parentNode;if(o){params.list.push(o);}}",
            setDropElementsOperation:"if(node.path && !node.isVirtual ){var o=gObject(sanitizeElementId(params.that.container+node.path));" +
                    "if(o)o=o.parentNode;if(o){params.list.push(o);}}",
            showChildrenCount : this.showChildrenCount,
            icons       :{leaf:null},
            urls: {
                 input_folder : "function:vjObjFunc('input_folder','"+this.objCls+"')",
                   undo :"function:vjObjFunc('undo','"+this.objCls+"')",
                redo :"function:vjObjFunc('redo','"+this.objCls+"')",
                   refresh :"function:vjObjFunc('refresh','"+this.objCls+"')"
                   },

            formObject : document.forms[this.formFolders],
            autoexpand : this.autoexpand,
            maxTxtLen : 64,
            onSelectFolder: "function:vjObjFunc('onSelectFolder','"+this.objCls+"')",
            isok : true
        });
        this.viewerFSTree = dirViewers[1];
        this.viewerFSTreePanel = dirViewers[0];
        this.viewerFSTreePanel.addActionResponseCallback(this.actionsHandler, this);
        this.viewerFSTree.onDoneChildren = function(unused_param, text, page_request) { that.actionControler(unused_param,text, page_request); };
        if (this.isSingleTab) {
            this.viewerFSTree.onSelectCaptureCallback = function(t_view,t_node){
                if (!t_node.selected) {
                    t_view.currentFolder = "";
                    return false;
                }
                t_view.currentFolder = t_node.path ? t_node.path : "";
                t_view.onSelectedFolder({'node':t_node},"");
            };
        }

        this.folders_DV.add("home", "img/home.png", "tab", [ dirViewers[0], dirViewers[1]]);
        this.folders_DV.add("help", "help", "tab", [ new vjHelpView({data : this.dsFoldersHelp.name}) ]);

    };

    this.actionControler = function (params, content, page_request) {
        this.viewerFSTree.updateHistory(params, content, page_request);
        if ( !this.viewerFSTreePanel.onActionResponse(content,page_request) ) {
            this.viewerFSTree.refreshOnArrive();
        }
    };

    this.init_Previews_viewers=function()
    {
        if(!this.notAddHelpAtPrewView)
            this.previews_DV.add( "help", "help", "tab", [ new vjHelpView ({ data:this.dsPreviewsHelp.name})  ], undefined, 20000);

        var this_ = this;
        this.previews_DV.add("sharing", "share", "tab", [
            new vjPanelView({
                data: [ "dsVoid", this.dsPreviewsRecord.name ],
                rows: [
                    { name : 'refresh', icon : 'img/48/refresh.png', title : 'Reload sharing permissions', url : "javascript:vjObjEvent(\"onRefreshSharing\",\"" + this.objCls + "\")"},
                    { name : 'edit', icon : 'edit.png', title : 'Edit sharing permissions' }
                ],
                precompute: function(viewer, tbl, ir) {
                    var node = tbl.rows[ir];
                    if (node.name == "edit") {
                        var record_tbl = new vjTable(viewer.getData(1).data, 0, vjTable_propCSV);
                        if (record_tbl.rows.length && record_tbl.rows[0]._action && record_tbl.rows[0]._action.match("share")) {
                            node.hidden = node.treenode.hidden = false;
                            node.url = node.treenode.url = "javascript:vjObjEvent(\"onEditSharing\",\"" + this_.objCls + "\",\"" + sanitizeStringJS(record_tbl.rows[0].id) + "\")";
                        } else {
                            node.hidden = node.treenode.hidden = true;
                        }
                    }
                },
                iconSize: 24,
                formObject: document.forms[this.formPreviews]
            }),
            new vjUserShareTreeView({
                data: [this.dsPreviewsUserTree.name, this.dsPreviewsUserList.name, this.dsPreviewsRecord.name, "dsVoid"],
                notSelectable: true,
                permsInTitle: true,
                hideNonPermitted: true
            }),
            new vjUserShareTreeColorView()
        ], undefined, 11000);

        this.previews_DV.add( "details", "rec", "tab",
            [
                new vjRecordView({
                    data:[this.dsPreviewsSpec.name,this.dsPreviewsRecord.name],
                    showRoot: 0,
                    autoStatus:3,
                    autoDescription:false,
                    objType:"",
                    readonlyMode: true,
                    editExistingOnly:  false,
                    showReadonlyInNonReadonlyMode: true,
                    RVtag: "RVPreviews",
                    formObject: document.forms[this.formPreviews],
                    implementSetStatusButton: true,
                    isok: true } )
            ], undefined, 10000);

        this.previews_DV.add("JSON", "rec", "tab",
            [
                new vjTextView({
                    data: this.dsPreviewsJson.name,
                    isok: true
                })
            ], undefined, 10001);
    };

    this.init_Submit_viewers=function()
    {
        var rows=[{ name : 'display', type : 'text', title : 'Select object(s) to add', value : 'Select object(s) to add', align:'right', readonly:true, size:80, prefix:'Selected object(s):  ', order : 1},
                { name : 'submit', type : 'button', value:'Submit', align: 'right' , order : 2, url : "javascript:vjObjEvent(\"onSubmitObjs\",\"" + this.objCls + "\")"},
                { name : 'clear', type : 'button', value:'Clear', align: 'right' , order : 3, url : "javascript:vjObjEvent(\"onClearSubmitObjs\",\"" + this.objCls + "\")"}
                ];

        this.submit_DV.add( "submit", "rec", "tab",
            [
                new vjPanelView({
                    data:["dsVoid"],
                    rows: rows,
                    formObject: document.forms[this.formSubmit],
                    isok: true } )
            ]);
    };


    this.init_Tables_viewers=function()
    {

        for ( var iS = 0; iS < this.subTablesAttrs.length; ++iS)
        {
            var curAttrs = this.subTablesAttrs[iS];
            var clDAttr = cpyObj(this.subTableDefault);
            var specifiedUrl = false;
            for ( var a in curAttrs) {
                if (a == "url") {
                    specifiedUrl = true;
                } else {
                    clDAttr[a] = curAttrs[a];
                }
            }
            var dsname = 'ds' + clDAttr.dvname + clDAttr.tabname;
            if (this[dsname] !== undefined)
                clDAttr.url = this[dsname];
            if (specifiedUrl) {
                var exchUrlType = curAttrs["url"];
                for ( var t in exchUrlType) {
                    clDAttr.url_tmplt = urlExchangeParameter(clDAttr.url_tmplt, t, exchUrlType[t]);
                }
            }

            var tviewers = this.initFSTable(clDAttr.url_tmplt, clDAttr.url, clDAttr.dvname,
                    clDAttr.tabname, clDAttr.tabico, clDAttr.recordviewer,
                    clDAttr.formname, clDAttr.fullpanel, clDAttr.active,
                    clDAttr.addCmd, clDAttr.dbClickCallback, clDAttr.selectCallback, clDAttr.bgClrMap,
                    clDAttr.CloneDragCallback, clDAttr.DropHandler,
                    clDAttr.DragStartCallback, clDAttr.DragStopCallback,
                    clDAttr.callbackRendered, clDAttr.panelCallbackRendered,
                    clDAttr.DragCancelCallback, clDAttr.mangle, clDAttr.editCmd,
                    clDAttr.precompute,clDAttr.hideListCols,clDAttr.multiSelect);
            if(clDAttr.objType) {
                tviewers[0].objType=clDAttr.objType;
                tviewers[1].objType=clDAttr.objType;
            }
            this.viewerFSTable.push(tviewers[1]);
            this.viewerFSPanel.push(tviewers[0]);
        }
        
        if( this.tables_DV.frame!='notab' ) {
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if( !this.isNoUpload ) {
                vjPAGE.initStandardDownloader(this.subTableDefault.dvname,this.objCls+"Downloader", this.subTableDefault.formname,"dmDownloader.cgi?cmd=-qpProcSubmit&svc=dmDownloader");
                vjPAGE.uploaderSubmitCallback = function (from, tabWord, qty){
                    if (qty){
                        //for now, not sure how to handle switching tabs just yet
                        //that.selectDataLoading();
                    }
                };
            }

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            var tabname = "help", tabico = "help";
            var dsname = 'ds-' + this.subTableDefault.dvname + '-' + tabname;
            var dsurl = "http://help/hlp.page.home.cloudProcesses.html";
            if (this[dsname] !== undefined)
                dsurl = this[dsname];

            this.dsFoldersHelp = this.vjDS.add("infrastructure: Folders Help", dsname, dsurl);
            this.tables_DV.add(tabname, tabico, "tab", [ new vjHelpView({ data : dsname }) ]);
        }
    };
    
    this.selectDataLoading = function(param) {
        $("#"+that.tables_DV.tabs["data-loading"].container+"_tabname").click();
        
        setTimeout(function(){
            var ds1 = that.tables_DV.tabs["data-loading"].viewers[0].data[0];
            var ds2 = that.tables_DV.tabs["data-loading"].viewers[0].data[1];
            vjDS[ds1].reload(vjDS[ds1].url, true);
            vjDS[ds2].reload(vjDS[ds2].url, true);
            that.tables_DV.tabs["data-loading"].refresh();
        }, 500);
    };
    
    //Initialize containers of viewers
    this.viewerFSTree=new Object();
    this.viewerFSTable=new Array();
    this.viewerFSPanel = new Array();

    //Initialize default attributes of table viewers
    if(!this.subTableDefault)this.subTableDefault={
        dvname      : this.container_Tables,
        tabname     : "All",
        tabico      : "file",
        drag        : this.drag,
        recordviewer: this.container_Previews,
        formname    : this.formTables,
        active      : this.active,
        fullpanel   : true,
        bgClrMap    : this.bgClrMap,
        addCmd      : this.addCmd,
        category    : null,
        objId        : 'parIds',
        url_tmplt   : "http://?cmd=objList&mode=csv&actions=1&parIds=id&parP=child&prop=_type,_summary,created,category&info=1&cnt=50",
        objQry_tmplt   : '($(explorer_folder) as objlist).filter({._type=="svc-align-multiple"}).csv(["_type","_brief","created"],{start:$(explorer_start),cnt:$(explorer_cnt),info:true})',
        url         : "static://preview:Select a folder to see its content",
        dbClickCallback     : "function:vjObjFunc('onDoubleClickFile','" + this.objCls + "')",
        selectCallback      : "function:vjObjFunc('onSelectFile','" + this.objCls + "')",
        CloneDragCallback   : "function:vjObjFunc('setCloneDrag','" + this.objCls + "')",
        DropHandler         : "function:vjObjFunc('DropHandler','" + this.objCls + "')",
        DragStartCallback   : "function:vjObjFunc('onDragStart','" + this.objCls + "')",
        DragStopCallback   : "function:vjObjFunc('onDragStop','" + this.objCls + "')",
        DragCancelCallback  : "function:vjObjFunc('onDragCancel','" + this.objCls + "')",
        callbackRendered    : "function:vjObjFunc('onRenderTableViewer','" + this.objCls + "')",
        panelCallbackRendered    : "function:vjObjFunc('onRenderTablePanelViewer','" + this.objCls + "')",
        precompute:"node=funcLink(\"function:vjObjFunc('precomputeTable','"+this.objCls+"')\",node,this);",
        isok:true
    };


    if(!this.subTablesAttrs)this.subTablesAttrs = [];



    this.render=function()
    {

        if(this.folders_DV)
            this.folders_DV.render();
        if(this.tables_DV)
            this.tables_DV.render();
        if(this.previews_DV)
            this.previews_DV.render();
        if(this.submit_DV)
            this.submit_DV.render();
    };

    this.load=function(loadmode)
    {
        if(this.folders_DV){
            this.folders_DV.load(loadmode);
            this.viewerFSTree=this.vjDV.locate(this.container_Folders+".0.1");
        }

        if(this.tables_DV){
            this.tables_DV.load(loadmode);
        }

        if(this.previews_DV){
            this.previews_DV.load(loadmode);
        }
        if(this.submit_DV){
            this.submit_DV.load(loadmode);
            this.viewerFSSubmit=this.vjDV.locate(this.container_Submit+".0.0");
//            
//            if( this.viewerFSSubmit && this.viewerFSSubmit.tree ) {
//                var dispnode = this.viewerFSSubmit.tree.findByName('display');
//                if(dispnode){
//                    var tt = this.getSelectedObjsStats();
//                    dispnode.value=tt;
//                    this.viewerFSSubmit.refresh(dispnode);
//                }
//            }
        }
    };
    
    this.refresh = function() {
        this.onClearSubmitObjs();
        this.viewerFSTree=this.vjDV.locate(this.container_Folders+".0.1");
        if(this.viewerFSTree)this.viewerFSTree.refresh();
        this.viewerFSActiveTable = this.getActiveViewer();
        if(this.viewerFSActiveTable) this.viewerFSActiveTable.refresh();
    };
    
    this.onClearSubmitObjs = function() {
        delete this.selectedObjs;
        delete this.id2iselectedObj;
        
        this.viewerFSActiveTable = this.getActiveViewer();
        this.viewerFSActiveTable.enumerate("node.selected=0;");
        this.viewerFSActiveTable.refresh();
        var dispnode = this.viewerFSSubmit.tree.findByName('display');
        if(dispnode) {
            dispnode.value="Select object(s) to add";
            this.viewerFSSubmit.refresh(dispnode);
        }
    };

    this.onSubmitObjs = function(view) {
//        if( !this.selectedObjs ){
//            return;
//        }
        var func = this.onSubmitObjsCallback;
        if(func) {
            funcLink(func, view, this.selectedObjs);
        }
    };
    
    this.onSelectFolder = function(view, node, content )
    {
        if ( (!node.selected || this.currentFolder==this.viewerFSTree.currentFolder) && !this.viewerFSTree.selectedNodeContentChanged)
            return false;
        user_setvar("curdir_open",node.id);
        if( this.useCookies ) {
            cookieSet('explorer_curdir_open', node.id, 1);
        }

        this.viewerFSTree.selectedNodeContentChanged=false;

        if(view.currentFolder)
            this.currentFolder = view.currentFolder;
        var added_prop=undefined;
//        if(0)if(this.currentFolder==this.viewerFSTree.systemFolder.all.path || (this.viewerFSTree.systemFolder.Trash && this.currentFolder==this.viewerFSTree.systemFolder.Trash.path) )
    //if(this.currentFolder==this.viewerFSTree.systemFolder.All.path || this.currentFolder==this.viewerFSTree.systemFolder.Trash.path)
//            added_prop="_parent";
        if(this.qryLang)
            this.queryArrivedCallback_QryLang(node,content,added_prop);
        else
            this.queryArrivedCallback(node,content,added_prop);

        var src_arr = this.currentFolder.split("/");
        var currentFolderID=src_arr.pop();
        var current_parentFolderID = src_arr.pop();
        if( !current_parentFolderID ) {
            current_parentFolderID = "null";
        }
        for(var i = 0 ; i < this.viewerFSPanel.length ; ++i ) {
            if(!this.viewerFSPanel[i].evalVariables) {
                this.viewerFSPanel[i].evalVariables = new Object();
            }
            this.viewerFSPanel[i].evalVariables.srcFolder = currentFolderID;// == this.viewerFSTree.systemFolder.all.id  ? this.viewerFSTree.systemFolder.all.id :currentFolderID;
            this.viewerFSPanel[i].evalVariables.dcls = this.objCls;
        }
        if(!this.viewerFSTreePanel.evalVariables) {
            this.viewerFSTreePanel.evalVariables = new Object();
        }

        this.viewerFSTreePanel.evalVariables.srcFolder = current_parentFolderID;
    };



    ///////////////////////////////////////////////////////////////////////////
    //                          File viewers Dependencies                    //
    ///////////////////////////////////////////////////////////////////////////
    this.precomputeTable = function(node,tbl)
    {
        var icon=vjHO.get(node._type,'icon',node.category);
        icon=evalVars(icon, "$(", ")",node);

        if(icon)
            node.icon=icon;
        else
            node.icon='rec';

        if(node._parent){
            var c=0;
            for(; c<tbl.hdr.length ; ++c){
                if(tbl.hdr[c].name=="_parent")
                    break;
            }
            if(c>=tbl.hdr.length)
                return node;

            var parent=[];
            node._parent=verarr(node._parent);
            for(var p=0 ; p<node._parent.length ; ++p){
                var nodeParent=node._parent[p];
                var treeN=this.viewerFSTree.tree.findById(nodeParent);
                if(treeN){
                    var pathStack=[];
                    while(treeN.parent && treeN.depth){
                        pathStack.push(treeN.title+"/");
                        treeN=treeN.parent;
                    }
                    var path="";
                    while(pathStack.length)
                        path+=pathStack.pop();
                    parent.push(path.substring(0,path.length-1));
                }
            }
            node._parentIDs=node._parent;
            node._parent=parent;
            node.cols[c]=parent;
        }
        
        if(this.isSubmitMode && this.selectedObjs) {
            if( this.id2iselectedObj[node.id]) node.selected=1;
        }
        
        return node;
    };

    this.onRefreshDelayTables = function (viewer, nodeId) {
        this.viewerFSTree.selectedNodeContentChanged=true;
        this.viewerFSTree.refreshOnArrive();
    };

    this.onSelectFile = function(viewer, node, ir, ic , column ) {

        if(node && node.submitter ){

            if(column==-1 || (column && (column.name=="reqID" || column.name=="status")) ) {

                linkURL("?cmd="+node.submitter+"&id="+node.id,"process"+node.id);
            }
        }

        if(this.previews_DV){
            if(!node || !node.selected)node=null;

            var comp={obj:this.previews_DV};
            var rmtabs=[];
            for(var it=0 ; it<this.previews_DV.tabs.length ; ++it){
                var cTab=this.previews_DV.tabs[it];
                var found=false;
                var stickytabs=['help','details','JSON','sharing'];
                for(var is=0; is< stickytabs.length ; ++is){
                    var sTab=stickytabs[is];
                    if(cTab.name==sTab){
                        found=true;
                        break;
                    }
                }
                if(!found){
                    rmtabs.push(cTab.name);
                }
            }
            this.previews_DV_selected=this.previews_DV.selected;
            if(isNaN(this.previews_DV_selected) || this.previews_DV.tabs.length <= this.previews_DV_selected) {
                this.previews_DV_selected=1;
            }
            var selectedTab=parseInt(this.previews_DV_selected);
            
            this.selectedTabName = this.previews_DV.tabs[selectedTab]?this.previews_DV.tabs[selectedTab].name:undefined;
            
            if(node) {
                this.dsPreviewsSpec.reload("http://?cmd=propspec&type="+node._type);

                if(this.appendUserPerspective){
                    this.dsPreviewsRecord.reload("http://?cmd=propget&mode=csv&perm=1&actions=1&ids="+node.id+"&userPerspective="+viewer.userId);
                    this.dsPreviewsJson.reload("http://?cmd=propget&mode=json&ids="+node.id+"&userPerspective="+viewer.userId);
                    node.userId = viewer.userId;
                } else {
                    this.dsPreviewsRecord.reload("http://?cmd=propget&mode=csv&perm=1&actions=1&ids="+node.id);
                    this.dsPreviewsJson.reload("http://?cmd=propget&mode=json&ids="+node.id);
                }

                var rv=this.previews_DV.tabs[selectedTab].viewers[0];
                rv.objType=node._type;
                rv.hiveId=node.id;

                if(vjHO[node._type] && vjHO[node._type].resetViewers)vjHO[node._type].resetViewers();
            }

            this.previews_DV.remove(rmtabs);


            this.previews_DV.render();
            this.previews_DV.load('rerender');
            
            if(node){
                var hiveObjType = vjHO.preview(node._type,node,comp);
                if (hiveObjType) {
                    var that = this;
                    hiveObjType.onPreviewRenderCallback = function(){
                        if(that.previews_DV.tabs[that.selectedTabName])
                            that.previews_DV.select(that.selectedTabName,true);
                        else
                            that.previews_DV.select(0,true);
                    };
                } else {
                    console.log("Developer warning: obj " + node.id + " is of type '" + node._type + "' which is not registered in vjHO. Type-specific preview will not be loaded.");
                }
            }

//            this.previews_DV.select(0);

        }
        if(this.isSubmitMode) {
            
            this.viewerFSActiveTable = this.getActiveViewer();
            if(!gKeyCtrl) {
                delete this.selectedObjs;
                delete this.id2iselectedObj;
            }
            if(!this.selectedObjs) {
                this.selectedObjs= [];
                this.id2iselectedObj = {};
            }
            this.viewerFSActiveTable.enumerate("if(params.id2iselectedObj[node.id] && !node.selected) { params.selectedObjs.splice(params.id2iselectedObj[node.id], 1); delete params.id2iselectedObj[node.id]; }" +
                                                "else if( node.selected && params.id2iselectedObj[node.id] == undefined) { params.id2iselectedObj[node.id] = params.selectedObjs.length; params.selectedObjs.push(node); }", {selectedObjs: this.selectedObjs, id2iselectedObj: this.id2iselectedObj});
            
            var tt = this.getSelectedObjsStats();
            var dispnode = this.viewerFSSubmit.tree.findByName('display');
            if(dispnode){
                dispnode.title=tt;
                dispnode.value=tt;
                this.viewerFSSubmit.refresh(dispnode);
            }
        }


        if(this.onSelectFileCallback){
            funcLink(this.onSelectFileCallback, viewer, node);
        }
    };
    
    
    this.getSelectedObjsStats=function(){
        var types=new Object(),ct='';
        if (this.selectedObjs) {
            this.selectedObjs.forEach(function(obj) {
                ct = obj._type;
                if (types[ct])
                    ++types[ct];
                else
                    types[ct] = 1;
            });
        }
        var res='';
        for(var tt in types) {
            res+=","+types[tt]+" "+tt+ (types[tt]>1?"s":"");
        }
        return res.substring(1);
    };
    
    this.onDoubleClickFile = function(viewer, tnode, ir, ic, column ) {
        node=this.viewerFSTree.tree.findById(tnode.id);
        var isDblSubmit = false;
        if( this.isSubmitMode ) {
            if( node===undefined || ( gKeyCtrl && this.selectedObjs && this.selectedObjs.length))
                this.onSelectFile(viewer, tnode, ir, ic , column );
            isDblSubmit = true;
        }
//        
//        if(node===undefined && !isDblSubmit){
//            if(this.isSubmitMode && this.selectedObjs && this.selectedObjs.length){
//                isDblSubmit = true;
//            }
//            else
//                return ;
//        }
        
        if( node && !isDblSubmit ){
            node.selected=1;
            var parent = node.parent, p_parent=0;
            while( parent && !parent.expanded ) {
                parent.expanded=1;
                parent.selected=0;
                p_parent=parent;
                parent = parent.parent;
            }
            if( p_parent ) {
                this.viewerFSTree.refresh(p_parent);
            }
            this.viewerFSTree.onClickNode.call(this.viewerFSTree,viewer,node.path);
        }
        else if( this.isSubmitMode ){
            if(this.selectedObjs && !this.id2iselectedObj[tnode.id])
                viewer.mimicClickCell.call(viewer,ir, ic);
//            this.onSelectFile(viewer,node,ir,ic,column);
            this.onSubmitObjs();
        }
    };

    this.onRenderTableViewer = function(viewer) {
        if(this.renderTableCallback){
            funcLink(this.onRenderTableCallback, viewer, node);
        }
    };
    
    this.onRenderTablePanelViewer = function (viewer) {
        var dsurl = vjDS[viewer.data[1]].url;
        var argsToRetain = ["cnt","start","search"];
        
        if( !this.cachedViewerArgs )
            this.cachedViewerArgs = new Object();
        
        var folderID = this.currentFolder.split("/").pop();
        if ( !this.cachedViewerArgs[folderID] )
            this.cachedViewerArgs[folderID] = new Object();
        var folderArgs = this.cachedViewerArgs[folderID]; 
        
        var tabName = this.tables_DV.tabs[this.vjDV[this.container_Tables].selected].name;
        if ( !folderArgs[tabName] )
            folderArgs[tabName] = new Object();
        var folderTabArgs = folderArgs[tabName];
        
        for ( var ii in argsToRetain ) {
            var argValue = docLocValue(argsToRetain[ii],"",dsurl);
            if( folderTabArgs[argsToRetain[ii]] || argValue ) {
                folderTabArgs[argsToRetain[ii]] = argValue;
            }
        }

    };

    this.tablesTabRendered = function( dv ) {
        if( this.useCookies ) {
            cookieSet('explorer_curtab_open', dv.selected, 1);
        }
    };

    this.tablesTabSelectedCallback=function(dv, dvname,itab, prv_itab) {
        if(!this.previews_DV)return ;
        var view=this.getTableViewerofTab(dv.tabs[itab]);

        if(this.currentFolder) {
            var t_node = this.viewerFSTree.tree.findByPath(this.currentFolder);
            t_node.selectedTab = itab;
            this.currentTab = itab;
        }

        var actView=this.getTableViewerofTab(dv.tabs[prv_itab]);
        if(actView!==undefined){
            var oldPreviewData=new Array();
            for(var i=2 ; i < this.previews_DV.tabs.length ; ++i){
                var prvw_t=this.previews_DV.tabs [i];
                for(var v=0 ; v< prvw_t.viewers.length ; ++v){
                    var prvw_v=prvw_t.viewers[v];
                    var dataArr=verarr(prvw_v.data);
                    for(var d=0 ; d<dataArr.length ; ++d){
                        var prvw_v_d=this.vjDS[dataArr[d]];
                        var t_obj=new Object();
                        t_obj['name']=prvw_v_d.name;
                        t_obj['data']=prvw_v_d.data;
                        oldPreviewData.push (t_obj );
                    }
                }
            }
            actView['oldPreviewData']=oldPreviewData;
        }

        if(!view)return;

            if(!view.selectedCnt || !view.selectedNodes.length){
                this.onSelectFile(viewer,0);
            }
            else if(view.selectedCnt>=1){
                this.onSelectFile(viewer,view.selectedNodes[0]);
            }
//        }
        if(this.onTablesTabSelectedCallback){
            funcLink(this.onTablesTabSelectedCallback, dv, dvname,itab, prv_itab);
        }
    };

//    this.onDeleteObject=function(){
//        if(!this.currentFolder.length)return ;
//        if(this.currentFolder==this.viewerFSTree.systemFolder.Trash.path){
//            var res=0;
//            if(this.currentFolder==this.viewerFSTree.systemFolder.Trash.path){
//                 res = confirm("Are you sure you want to permanently delete the selected objects");
//            }
//            else
//                res = confirm("This will permanently delete the selected objects from all locations.\n Do you want to proceed?");
//            if (!res|| !this.viewerFSActiveTable || !this.viewerFSActiveTable.selectedNodes || !this.viewerFSActiveTable.selectedNodes.length) return;
//            var ids=[];
//            for(var i=0 ; i < this.viewerFSActiveTable.selectedNodes.length ; ++i ){
//                ids.push(this.viewerFSActiveTable.selectedNodes[i].id);
//            }
//            var url="?cmd=objDel&ids="+ids.join(",");
//            var selNodes=this.viewerFSActiveTable.selectedNodes;
//            var objs=[],srcs=[];
//            for(var i = 0 ; i < selNodes.length ; ++i) {
//                var node=selNodes[i];
//                objIDs.push(node[this.idtype]);
//                node._parentIDs=verarr(node._parentIDs);
//                for(var i=0 ; i < node._parentIDs.length ; ++i) {
//                    srcIDs=objIDs.concat(node.parentIDs[i].split(","));
//                }
//            }
//            ajaxDynaRequestPage(url, {objCls: this.viewerFSTree.objCls, callback:'cleanChildLinks',params:{objIDs:objs,srcIDs:srcs}}, vjObjAjaxCallback);
//            return ;
//        }
//        var src=this.viewerFSTree.tree.findByPath(this.currentFolder);
//        var dst=this.viewerFSTree.systemFolder.Trash;
//
//        this.moveChildren(src, dst, this.viewerFSActiveTable.selectedNodes);
//    };
//    this.onDeleteFolder=function(vv){
//        if(!this.currentFolder.length)return ;
//        if(this.currentFolder==this.viewerFSTree.systemFolder.Trash.path
//                || this.currentFolder==this.viewerFSTree.systemFolder.Inbox.path
//                || this.currentFolder==this.viewerFSTree.systemFolder.Home.path) {
//            alert("You cannot delete \""+this.currentFolder.substr(1)+"\" folder");
//            return ;
//        }
//
//        var src=this.viewerFSTree.tree.findByPath(this.currentFolder);
//        var dst=this.viewerFSTree.systemFolder.Trash;
//
//        this.moveChildren(src.parent,dst,src);
//    };


    ///////////////////////////////////////////////////////////////////////////
    //                          Manipulate folders                           //
    ///////////////////////////////////////////////////////////////////////////


//    this.moveChildren=function(src,dst,children,copy){
//        var parent_folders= new Array();
//        if(!this.viewerFSTree)return ;
//        else{
//            if(children._parentIDs){
//                for(var i=0 ; i < children._parentIDs ; ++i){
//                    var t_folder=this.viewerFSTree.tree.findById(children._parentIDs[i]);
//                    if(t_folder)
//                        parent_folders.push(t_folder);
//                }
//            }
//            if(!parent_folders.length)
//                this.viewerFSTree.moveChildren(src,dst,children,copy);
//            else
//                this.viewerFSTree.moveChildren(src,dst,children,copy,parent_folders);
//        }
//    };

    ///////////////////////////////////////////////////////////////////////////
    //                          DnD between viewers                          //
    ///////////////////////////////////////////////////////////////////////////

    this.setCloneDrag = function(viewer, dragE) {
        var icon = "";
        var obj = new Object();
        for ( var is = 0; is < viewer.drag.sourceObj.length; ++is) {
            obj = viewer.findByDOM(viewer.drag.sourceObj[is]);
            var node = viewer.tblArr.rows[obj.ir];
            if (icon == "" && node.icon)
                icon = node.icon;
            if (node.icon != icon) {
                icon = "";
                break;
            }
        }
        if (!icon.length)
            icon = "folderEmpty";
        if(icon.indexOf("/")==-1)icon="img/"+icon;
        if(icon.indexOf(".")==-1)icon+=".gif";
        var clElem = gObject(viewer.container + "_" + obj.ir + "_icon");
        if (clElem)
            clElem = clElem.cloneNode(true);
        else
            return;
        clElem.innerHTML = "<img src='"+icon+"' height='24px' border='0' >";
        if (viewer.drag.sourceObj.length > 1)
            clElem.innerHTML += "<span style='position:absolute;text-align:center;left:0;top:0;opacity:0.7;height:100%;width:100%;vertical-align:middle'><span style='background-color:#0033FF;color:white'>"
                    + viewer.drag.sourceObj.length + "</span></span>";

        clElem.childNodes[0].height = "24";
        return clElem;
    };

    this.findInSelectedNodes=function(nodelist,node,param){
        nodelist=verarr(nodelist);
        if(!param)param='id';
        for(var i=0 ; i < nodelist.length ; ++i){
            var curNode=nodelist[i];
            if(curNode[param]==node[param]){
                return i;
            }
        }
        return -1;
    };
    this.setDropables = function(srcNodes,modeStart) {
        var viewerFSTree = this.viewerFSTree;
        if (!viewerFSTree || !srcNodes)
            return false;
        for ( var is = 0; is < srcNodes.length; ++is) {
            var srcNode = this.viewerFSActiveTable.tblArr.rows[srcNodes[is].ir];
            if (!srcNode._type)
                continue;
            if (srcNode._type != "folder")
                continue;
            var name = srcNode[this.idtype];
            var dragFolder = this.currentFolder;
            if (name)
                dragFolder += "/" + name;
            viewerFSTree.toggleNonDropableSubTree(dragFolder,modeStart);
        }
        viewerFSTree.toggleNonDropableElement(this.currentFolder,modeStart);
        this.viewerFSTree.excludeNonDroppables();
    };
    this.onDragStart = function(srcNodes) {
        this.autoSelectedOnDrag=false;
        this.viewerFSActiveTable = this.viewerFSTable[this.vjDV[this.container_Tables].selected];
        if (!this.viewerFSActiveTable)
            return false;
        this.viewerFSActiveTable.drag.sourceObj = [];
        var srcTrigElem = this.viewerFSActiveTable.drag.sourceTriggerObj;
        var node = this.viewerFSActiveTable.findByDOM(srcTrigElem);
        var nodeRow = Int(node.ir), nodeCol = Int(node.ic);
        srcNode = this.viewerFSActiveTable.tblArr.rows[node.ir];
        if (!this.viewerFSActiveTable.selectedNodes.length
                || this.findInSelectedNodes(this.viewerFSActiveTable.selectedNodes,srcNode) == -1){
            this.viewerFSActiveTable.mimicClickCell(nodeRow, nodeCol);
            this.autoSelectedOnDrag=true;
        }

        if (this.viewerFSActiveTable.selectedCnt <= 0)
            return false;

        for ( var is = 0; is < this.viewerFSActiveTable.selectedNodes.length; ++is) {
            var indS = this.viewerFSActiveTable.tblArr.rows
                    .indexOf(this.viewerFSActiveTable.selectedNodes[is]);
            if (indS == -1)
                continue;
            var sourceEl = gObject(this.viewerFSActiveTable.container + "_"
                    + indS + "_" + nodeCol);
            if (sourceEl)
                this.viewerFSActiveTable.drag.sourceObj.push(sourceEl);
        }
        if (!this.viewerFSActiveTable.drag.sourceObj.length)
            this.viewerFSActiveTable.drag.sourceObj
                    .push(gObject(this.viewerFSActiveTable.container + "_"
                            + nodeRow + "_" + nodeCol));

        this.setDropables(srcNodes,true);
        return true;
    };

    this.onDragStop = function(srcNodes) {
        this.viewerFSActiveTable = this.viewerFSTable[this.vjDV[this.container_Tables].selected];
        if (!this.viewerFSActiveTable)
            return false;
        this.viewerFSActiveTable.drag.sourceObj = [];

        this.setDropables(srcNodes,false);
        return true;
    };

    this.onDragCancel = function(srcNodes) {// srcNode,nodeValue,
                                                // nodeRow,nodeCol,d){
        this.viewerFSActiveTable = this.viewerFSTable[this.vjDV[this.container_Tables].selected];
        if (!this.viewerFSActiveTable)
            return false;
        this.viewerFSActiveTable.drag.sourceObj = [];
        if(this.autoSelectedOnDrag){
            gDragEvent=true;
            this.autoSelectedOnDrag=false;
        }

        if (!this.currentFolder.length)
            return false;

        this.setDropables(srcNodes,false);
        return true;
    };

    this.DropHandler = function(dragNodes, dropE,copy) {
        this.viewerFSActiveTable = this.viewerFSTable[this.vjDV[this.container_Tables].selected];
        if (!this.viewerFSActiveTable)
            return false;
        var viewerFSTree = this.viewerFSTree;
        if (!viewerFSTree)
            return false;

        var dropO = viewerFSTree.findByDOM(dropE);

        this.setDropables(dragNodes,false);

        if (!dragNodes || !dropO || !dragNodes.length)
            return false;

        var dragOs = new Array();
        var dragParent = viewerFSTree.tree.findByPath(this.currentFolder);

        if (!dragParent)
            dragParent = viewerFSTree.tree.root;
            for ( var iO = 0; iO < dragNodes.length; ++iO) {
                var dragO = this.viewerFSActiveTable.tblArr.rows[Int(dragNodes[iO].ir)];
                dragOs.push(dragO.id);
            }
        this.paste(dragParent,dropO,dragOs,copy);
    };

    this.paste = function (src, dst, movingChildren, isCopy) {
        if(isCopy) {
            this.viewerFSTree.copyChildren(src.id,dst.id,movingChildren);
        }
        else{
            this.viewerFSTree.cutChildren(src.id,dst.id,movingChildren);
        }
    };
    this._delete = function (src, ids) {
        this.viewerFSTree._delete(src,ids);
    };

    ///////////////////////////////////////////////////////////////////////////
    //                                  AUX                                  //
    //        (Directory view return children IDs of selected folder.        //
    //               Here we send new request for all these IDs)             //
    ///////////////////////////////////////////////////////////////////////////

    this.queryArrivedCallback=function(node,content,addProperties)
    {
        var ids="-";
//        var params=ajaxParams.params;
        var url = undefined;
        var tbl=new vjTable(content,0,vjTable_propCSV|vjTable_inheritPathPropFormat);
        if( (content.length || this.isSingleTab) && this.currentFolder!="/all"){
            ids =node[this.idtype];
        }
        if(!content.length && this.currentFolder!="/all"){
            if(!this.currentFolder || !this.currentFolder.length){
                url  = "static://preview:Select a folder to see its content";
            }
//            else if(this.currentFolder.length){
//                url  = "static://preview:Folder is empty";
//            }

        }

        var urlArray = new Array();
        for( var it=0 ; it < this.viewerFSTable.length ; ++it) {
            var vv=this.viewerFSTable[it];

            if(this.appendUserIdToFSTable)    vv.userId = node.id;
            var ds=vv.getData(0);
            if(url)
                ds.reload(url,true);
            else if (this.tablesUrlTmplt) {
                urlArray.push( constructObjQryUrl(this.tablesUrlTmplt , {"folder":ids}));
            }
            else{
                var dsurl=ds.url.indexOf("static://")==0?ds.url_tmplt: ds.url;
                var m_index=0;
                var curAttrs=0;
                for( ; m_index < this.subTablesAttrs.length ; ++m_index){
                    curAttrs=this.subTablesAttrs[m_index];
                    if(curAttrs.tabname==vv.tab.name)
                        break;
                    curAttrs=0;
                }
                if(!curAttrs)return ;

                var typeCount=undefined;
                var displayedTypes= docLocValue("type","",dsurl);
                if( !displayedTypes.length || curAttrs.types ) {
                    if(curAttrs.types)
                        displayedTypes = curAttrs.types;
                    else if (curAttrs.url && curAttrs.url.type)
                        displayedTypes = curAttrs.url.type;
                }
                //TODO Do not filter for counts in javascript. We should ask cgi for type_cnt of specific types instead.
                if( this.isSingleTab ) {
                    displayedTypes="";
                }

                typeCount=this.findTypeCount(displayedTypes,tbl);
                if( !this.isSingleTab && ( (tbl && ( isok(displayedTypes) && curAttrs.url.type!='-') &&  !typeCount) || !tbl.rows.length) ){
                    vv.tab.invisible=true;
                    vv.tab.hidden=true;
                    ds.reload("static://preview:Selected folder doesn't have any objects relevant for this tab", true);
                    continue;
                }
                else{
                    vv.tab.invisible=false;
                    vv.tab.hidden=false;
                    var extCnt=typeCount?"<sup>["+typeCount+"]</sup>":"";
                    vv.tab.title=vv.tab.name+extCnt;
                }

                var t_ids=ids;
                var t_prop="";
                if(curAttrs.url && curAttrs.url.prop) {
                    t_prop=curAttrs.url.prop;
                }
                else {
                    t_prop=docLocValue("prop","",this.subTableDefault.url_tmplt);
                }

                if(addProperties){
                    if(t_prop.length)
                        t_prop+=",";
                    t_prop += addProperties;
                }

                if(t_prop.indexOf(",_type") == -1)
                    t_prop += ",_type";
                
                if( this.viewerFSTree.inTrash(this.currentFolder+"/") ) {
                    dsurl=urlExchangeParameter(dsurl, "showTrashed","1");
                }
                else {
                    dsurl=urlExchangeParameter(dsurl, "showTrashed","-");
                }
                
                dsurl=urlExchangeParameter(dsurl, "prop",t_prop);
                dsurl=urlExchangeParameter(dsurl, "start",0);
                dsurl=urlExchangeParameter(dsurl, "search","-");
                
                var folderID = this.currentFolder.split("/").pop();
                if( this.cachedViewerArgs &&  this.cachedViewerArgs[folderID] && this.cachedViewerArgs[folderID][vv.tab.name] ) {
                    var cachedArgs = this.cachedViewerArgs[folderID][vv.tab.name];
                    for ( var ia in cachedArgs) {
                        dsurl=urlExchangeParameter(dsurl, ia,cachedArgs[ia]);
                    }
                }
                
                dsurl=urlExchangeParameter(dsurl, this.subTableDefault.objId,ids);
                if(this.isNShowactions){
                    dsurl=urlExchangeParameter(dsurl, "actions","-");
                }
                urlArray.push(dsurl);
                ids=t_ids;
            }
        }
        var tabNum = node.selectedTab;
        if( tabNum===undefined ) {
            tabNum = this.currentTab;
        }
        if( tabNum===undefined ) {
            if( this.useCookies ) {
                var tab_selected = cookieGet('explorer_curtab_open', 0);
                if( tab_selected && cookieGet("explorer_curdir_open",undefined)) {
                    tabNum = Int(tab_selected);
                }
            }
        }
        if( tabNum===undefined ) {
            tabNum = 0;
        }
        if(this.tables_DV.tabs[tabNum].invisible)
        {
            tabNum = 0;
        }
        node.selectedTab = tabNum ;
        this.currentTab = tabNum ;
        this.tables_DV.select(tabNum);
        this.vjDV[this.container_Tables].render(true);
        var ii = 0;
        for( var it=0 ; it < this.viewerFSTable.length ; ++it) {
            var vv=this.viewerFSTable[it];
            if(vv.tab.invisible) {
                continue;
            }
            var ds=vv.getData(0);
            ds.reload(urlArray[ii++],vv.tabvisible);
        }


    };

    this.queryArrivedCallback_QryLang=function(node,content,addProperties){
        var ids="-";
      var url = undefined;
      var tbl=new vjTable(content,0,vjTable_propCSV|vjTable_inheritPathPropFormat);
      if(content.length && this.currentFolder!="/all"){
          ids =node[this.idtype];
      }
      if(!content.length && this.currentFolder!="/all"){
          if(!this.currentFolder || !this.currentFolder.length){
              url  = "static://preview:Select a folder to see its content";
          }
      }
      
      if(ids=="-"){
          ids ="alloftype(\"*\")";
      }
      else {
          ids ="((obj)"+ids+").child ";
      }
      var urlArray = new Array();
      for( var it=0 ; it < this.viewerFSTable.length ; ++it) {
          var vv=this.viewerFSTable[it];

          if(this.appendUserIdToFSTable)    vv.userId = node.id;
          var ds=vv.getData(0);
          if(url)
              ds.reload(url,true);
          else if (this.tablesUrlTmplt) {
              urlArray.push( constructObjQryUrl(this.tablesUrlTmplt , {"folder":ids}));
          }
          else{
              var dsurl=ds.url_tmplt;
              var m_index=0;
              var curAttrs=0;
              for( ; m_index < this.subTablesAttrs.length ; ++m_index){
                  curAttrs=this.subTablesAttrs[m_index];
                  if(curAttrs.tabname==vv.tab.name)
                      break;
                  curAttrs=0;
              }
              if(!curAttrs)return ;

              if(tbl && !tbl.rows.length){
                  vv.tab.invisible=true;
                  vv.tab.hidden=true;

                  continue;
              }
              else{
                  vv.tab.invisible=false;
                  vv.tab.hidden=false;
              }

              var urlPrefix="http://?cmd=objQry&qry=";
              dsurl=evalVars(dsurl,"$(",")", {explorer_folder: ids, explorer_start: 0, explorer_cnt: 1000});

              urlArray.push(urlPrefix+vjDS.escapeQueryLanguage(dsurl));
          }
      }
      var tabNum = node.selectedTab;
      if( tabNum===undefined ) {
          tabNum = this.currentTab;
      }
      if( tabNum===undefined ) {
          if( this.useCookies ) {
              var tab_selected = cookieGet('explorer_curtab_open', 0);
              if( tab_selected && cookieGet("explorer_curdir_open",undefined)) {
                  tabNum = Int(tab_selected);
              }
          }
      }
      if( tabNum===undefined ) {
          tabNum = 0;
      }
      if(this.tables_DV.tabs[tabNum].invisible)
      {
          tabNum = 0;
      }
      node.selectedTab = tabNum ;
      this.currentTab = tabNum ;
      this.tables_DV.select(tabNum);
      this.vjDV[this.container_Tables].render(true);
      var ii = 0;
      for( var it=0 ; it < this.viewerFSTable.length ; ++it) {
          var vv=this.viewerFSTable[it];
          if(vv.tab.invisible) {
              continue;
          }
          var ds=vv.getData(0);
          ds.reload(urlArray[ii++],vv.tabvisible);
      }

    };

    this.findTypeCount=function(urlType,resultsTbl){

        var tot_cnt=undefined;
        if(!resultsTbl.rows.length)return tot_cnt;
        var types=verarr(urlType.split(","));
        if(resultsTbl.rows[0].type && resultsTbl.rows[0].count){
            var cpRows=cpyObj(resultsTbl,{cpyObjParams:{deep:true}});
            for(var t=0 ; t<types.length ; ++t){

                var type=(types[t]==''?'.*':types[t]);
                var patt=new RegExp(type);
                for(var tr=0 ; tr < cpRows.rows.length ; ++tr){
                    if(patt.test(cpRows.rows[tr].type)){
                        if(tot_cnt===undefined)
                            tot_cnt=0;
                        tot_cnt+=parseInt(cpRows.rows[tr].count);
                        cpRows.rows[tr].count=0;
                    }
                }
            }
        }
        else{
            var cpRows=cpyObj(resultsTbl.rows[0],{cpyObjParams:{deep:true}});
            for(var t=0 ; t<types.length ; ++t){
                var type=(types[t]==''?'.*':types[t]);
                var patt=new RegExp(type);
                for(var attr in cpRows){
                    if(attr.indexOf("type_count.")!=0)continue;
                    if( patt.test(attr.replace("type_count.","") ) ){
                        if(tot_cnt===undefined)
                            tot_cnt=0;
                        tot_cnt+=parseInt(cpRows[attr]);
                        cpRows[attr]=0;
                    }
                }
            }
        }

        return tot_cnt;
    };


    this.getTableViewerofTab=function(tab){
        var view=0;
        for(var iv=0 ; iv < this.viewerFSTable.length ; ++iv){
            var vv=this.viewerFSTable[iv];
            if(vv.tab.num==tab.num){
                view=vv;
                break;
            }
        }
        return view;
    };

    this.getActiveViewer = function (tabI)
    {
        // ensure selected tab is among this.viewerFSTable
        this.vjDV[this.container_Tables].updateTabVisibility(false, 0, this.viewerFSTable.length - 1);

        this.viewerFSActiveTable = this.viewerFSTable[this.vjDV[this.container_Tables].selected];
        return this.viewerFSActiveTable;
    };


    this.signalHandler = {
        "cast" : function( thisObj, id, data, params ) {
            thisObj.viewerFSTree.selectedNodeContentChanged = true;
            if (data && data.background) {
                $("<div title='Information'><p>Object conversion has been successfully requested; the conversion process will take some time.</p></div>").dialog({
                    buttons: { "OK": function() { $(this).dialog("close"); } },
                    modal: true,
                    close: function(event, ui) {
                        // if the file is small, by the time the user clicks "OK", conversion might be done
                        thisObj.viewerFSTree.refreshOnArrive();
                    }
                });
            } else {
                thisObj.viewerFSTree.refreshOnArrive();
            }
        },
        "copy" : function( thisObj, id, data, params ){
            thisObj.viewerFSTree.refreshOnArrive();
        },
        "move" : function( thisObj, id, data, params ){
            thisObj.viewerFSTree.selectedNodeContentChanged = true;
            thisObj.viewerFSTree.refreshOnArrive();
        },
        "trash" : function( thisObj, id, data, params ){
            thisObj.viewerFSTree.selectedNodeContentChanged = true;
            thisObj.viewerFSTree.refreshOnArrive();
        },
        "delete" : function( thisObj, id, data, params ){
            thisObj.viewerFSTree.selectedNodeContentChanged = true;
            thisObj.viewerFSTree.refreshOnArrive();
        },
        "default" : function( thisObj, id, data, params ){
            thisObj.viewerFSTree.selectedNodeContentChanged = true;
            thisObj.viewerFSTree.refreshOnArrive();
        },
        "folder" : function(thisObj, id, data, params) {
            thisObj.viewerFSTree.refreshOnArrive();
        },
        "clone" : function(thisObj, id, data, params) {
            if (data && data.to) {
                $("<div title='Information'><p>Process " + id + " was successfully resubmitted under ID " + data.to + "</p></div>").dialog({
                    buttons: { "OK": function() { $(this).dialog("close"); } },
                    modal: true,
                    close: function(event, ui) {
                        thisObj.viewerFSTree.selectedNodeContentChanged = true;
                        thisObj.viewerFSTree.refreshOnArrive();
                    }
                });
            } else if (data && data.error) {
                $("<div title='Error'><p>Failed to resubmit process " + id + ": " + data.error + "</p></div>").dialog({
                    buttons: { "OK": function() { $(this).dialog("close"); } },
                    modal: true
                });
            }
        }
    };

    this.actionsHandler = function ( thisObj, response, params ) {
        var ret = undefined;
        for (var id in response ) {
            if( this.signalHandler [ response[id].signal ] ) {
                ret = this.signalHandler [ response[id].signal ] (this,id,response[id].data, params);
            }
        }
        return ret;
    };

    // turn an objList url into a query language expression
    // not : search not supported for now
    this.objList2ObjQry = function(container, url, options)
    {
        function urlParamValue(param) {
            var ret = docLocValue(param, "-", url);
            if (ret == "" || ret == "-" ) {
                return null;
            } else {
                return ret;
            }
        }
        // options: { "start": true, "cnt": true, "info": true } ; set to false to ignore start/cnt/info parameters
        var start = (!options || options.start) ? urlParamValue("start") : null;
        var cnt = (!options || options.cnt) ? urlParamValue("cnt") : null;
        var info = (!options || options.start) ? urlParamValue("info") : null;

        var parIds = urlParamValue("parIds");
        if (urlParamValue("parP") != "child") {
            parIds = null;
        }
        var types = urlParamValue("type");

        var qry = "";
        if (parIds) {
            qry = "(((objlist)\"" + parIds + "\").map({.child})).apply(append)";
            if (types) {
                qry += ".filter({.objoftype(\"" + types + "\")})";
            }
        } else {
            qry = "alloftype(\"" + (types ? types : "*") + "\")";
        }

        var props_list = ["_type", "_brief", "created"];
        var pushed_props = { };
        props_list.forEach(function(p) {pushed_props[p] = true});
        var props = urlParamValue("prop");
        if (props) {
            props.split(",").forEach(function(p) {
                if (!(p in pushed_props)) {
                    props_list.push(p);
                    pushed_props[p] = true;
                }
            });
        }

        qry += ".csv(" + JSON.stringify(props_list);
        if (start || cnt || info) {
            qry += "," + JSON.stringify({start: start ? start : 0, cnt : cnt ? cnt : 0, info : info ? info : 0});
        }
        qry += ")";
        return qry;
    };

    this.initFSTable = function (dsurl_tmplt, dsurl, dvname, tabname, tabico, recordviewer,
                                formname, fullpanel, active,  addCmd,dbClickCallback, selectCallback,
                                bgColorMap,setCloneDrag,DropHandler,onDragStart,onDragStop,callbackRendered,panelCallbackRendered,
                                onDragCancel,exclusionObjRegexArr, editCmd,precompute,hideListCols,lmultiSelect){


        //--------------------------------------------------------------------------------------//
        //-------------------------------------vjPANEL------------------------------------------//
        //--------------------------------------------------------------------------------------//
        var dsname='ds-'+dvname+'-'+tabname;
        if(!this.dsTables) {
            this.dsTables = new Array();
        }
        var ds = this.vjDS.add("infrastructure: Retrieving List of Objects", dsname , dsurl );
        ds.url_tmplt = dsurl_tmplt;
        this.dsTables.push(ds);


        var hideCmd = [ {
            name : new RegExp(/.*/),
            hidden : false
        }, {
            name : '_type',
            align : 'left',
            hidden : true
        }, {
            name : 'category',
            align : 'left',
            hidden : true
        }, {
            name : '_action',
            align : 'left',
            hidden : true
        }, {
            name : 'total',
            align : 'left',
            hidden : true
        }, {
            name : 'start',
            align : 'left',
            hidden : true
        } ];

        if(exclusionObjRegexArr){
            exclusionObjRegexArr.push({id:/^info*/});
            exclusionObjRegexArr.push({info : /^info*/});

        }

        if(!editCmd) {
            editCmd = [ {
                //Not  yet implemented, so hidden=true
                    name : 'create',
                    hidden : true,
                    prohibit_new : true
                },
                {
                    name : new RegExp(/convert.*/),
                    hidden : false,
//                    refreshDelay : 1000,
//                    refreshDelayCallback:"vjObjEvent('onRefreshDelayTables','"+this.objCls+"',$(id))",
                    prohibit_new : true
                },
                {
                    name : 'upload',
                    hidden : this.isNoUpload?true:false,
                    url: "javascript:vjDV.select('" + dvname + ".add.0',true);",
                    prohibit_new : true,
                    isok:true
                },
                {
                    name : 'delete',
                    hidden : false,
//                    refreshDelay : 1000,
//                    refreshDelayCallback:"vjObjEvent('onRefreshDelayTables','"+this.objCls+"',$(id))",
                    prohibit_new : true
                },
                {
                    name : 'admin',
                    hidden : true,
                    prohibit_new : true
                },
                {
                    name : 'edit',
                    hidden : false,
                    showTitle:true,
                    prohibit_new : true
                },
                {
                    name : 'rename',
                    hidden : false,
                    showTitle:true,
                    prohibit_new : true,
                    refreshDelay: 1000,
                    prompt: "New name of object"
                },
                {
                    name : 'detail',
                    hidden : false,

                    showTitle:true,
                    prohibit_new : true
                },
//                {
//                    name : 'clipboard',
//                    hidden : false,
//                    inactive:"javascript:var params={'show':false};args[1].tree.enumerate('if(!node.hidden) params.show=true',params,null,null,thisObj);params.show?false:true",
//                    nonAutoNode : false,
//                    icon : 'img/clipboard.png',
//                    prohibit_new : true
//                },
                {
                    name : 'copy',
                    hidden : false,
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
                    refreshDelay : 1000,
                    refreshDelayCallback:"vjObjEvent('onRefreshDelayTables','"+this.objCls+"',$(id))",
                    inactive:'javascript:gClip.content.length? false :true',
                    prohibit_new : true
                },
                {
                    name : 'download',
                    hidden : false,
                    prohibit_new : true
                },
                {
                    name : 'share',
                    hidden : false,
                    prohibit_new : true
                } ];
        }
        var viewCmd = [
                {
                    name : 'refresh',
                    align : 'left',
                    order : -100,
                    hidden : false,
                    align : 'left',
                    title : 'Refresh',
                    icon : 'img/48/refresh.png',
                    //icon_srcset : [24, 48],
                    description : 'refresh the content of the control to retrieve up to date information',
                    url : "javascript:vjDS['" + dsname
                            + "'].reload(null,true);"
                },
                {
                    name : 'pager',
                    align : 'right',
                    order : 99,
                    hidden : false,
                    icon : 'page',
                    title : 'per page',
                    description : 'page up/down or show selected number of objects in the control',
                    type : 'pager',
                    counters : [ 10, 20, 50, 100, 1000, 'all' ]
                }
                , {
                    name : 'search',
                    align : 'right',
                    order : 100,
                    hidden : false,
                    isSubmitable : true
                }, {
                    name : 'tblQryObjs',
                    title : 'Table of objects',
                    description : 'Display table of objects using Table Query',
                    align : 'right',
                    order : 110,
                    hidden : false,
                    icon : 'rec',
                    isSubmitable : false,
                    // tblqry objQry is generated from dsnname url at time of click, with no "info", "start", or "cnt" params
                    url : "javascript:linkURL('?cmd=tblqry-new&objQry='+vjDS.escapeQueryLanguage(vjObjEvent('objList2ObjQry','" + this.objCls + "',vjDS['" + dsname + "'].url,{start:0,cnt:0,info:0})),\"tblQryObjs_" + dsname + "\")"
                }];

        var rowlist = (fullpanel ? hideCmd.concat(editCmd, viewCmd) : hideCmd.concat(viewCmd) );
        if (addCmd)
            rowlist = rowlist.concat(addCmd);

        this.vjDS.add("infrastructure: Loading Actions", "dsActions" , "http://?cmd=objList&type=action&mode=csv" );
        var viewerPanel = new vjPanelView({
            data : [ "dsActions", dsname ],
            formObject : document.forms[formname],
            iconSize : 24,
            paste:this.paste,
            isSparse: true,
            exclusionObjRegex:exclusionObjRegexArr,
            callbackRendered: panelCallbackRendered,
            hideViewerToggle : true,
            showTitles : false,
            rows : rowlist,
            isok : true
        });
        viewerPanel.addActionResponseCallback(this.actionsHandler, this);


        //--------------------------------------------------------------------------------------//
        //-------------------------------------vjTABLE------------------------------------------//
        //--------------------------------------------------------------------------------------//

        var showListCols= [
         {
            name : 'id',
            hidden : false,
            title : 'ID',
            order : 1
        }, {
            name : 'name',
            hidden : false,
            title : 'Name',
            maxTxtLen:256,
            wrap : true,
            order : 2
        }, {
            name : '_brief',
            hidden : false,
            title : "Summary",
            maxTxtLen:256,
            wrap:true,
            order : 3
        }, {
            name : 'hierarchy',
            hidden : false,
            title : "Path",
            order : 3
        }, {
            name : 'created',
            hidden : false,
            title : "Created",
            align : "right",
            type : "datetime",
            order : 30
        },
        {
            name : 'uri',
            title : 'Source',
            hidden:false,
            order : 5,
            maxTxtLen: 80
        },
        {
            name : '_parent',
            title : 'Folders',
            hidden:false,
            maxTxtLen:16,
            order : 7
        }, {
            name : 'size',
            title : 'Size',
            type:'bytes',
            hidden:false,
            order : 7
        },
        {
            name:'svc',
            hidden:true
        },
        {
            name : 'submitter',
            hidden : true
        },
        {
            name : 'progress100',
            hidden : false,
            title : "Progress%",
            align : "right",
            type : 'percent',
            order : 4
        },
        {
            name : 'rec-count',
            hidden : false,
            title : "Records",
            align : "right",
            type : 'largenumber',
            order : 4
        },
//        {
//            name : 'rec-dim',
//            hidden : false,
//            title : "Unique Records",
//            align : "right",
//            type : 'largenumber',
//            order : 4
//        },
        {
            name : 'TITLE',
            hidden : false,
            title : "Title",
            order : 3
        },
        {
            name : 'Desc_Info_Summary',
            hidden : false,
            title : "Description",
            wrap : true,
            order : 3.1
        },
        {
            name : 'svcTitle',
            hidden : false,
            title : "Service",
            align : "right",
            order : 4.5
        },
        {
            name : 'status',
            order : 4.1,
            hidden : false,
            title : "Status",
            align : "left",
            type : 'choice',
            choice : [ 'unknown', 'waiting', 'processing', 'running',
                    'suspended', 'done', 'killed', 'failure', 'error',
                    'unknown' ]
        } ];
        showListCols=hideCmd.concat(showListCols);
        if(hideListCols) {
            showListCols = showListCols.concat(hideListCols);
        }

        var cntViewersIn = (this.vjDV[dvname].tabs && this.vjDV[dvname].tabs[tabname]) ? this.vjDV[dvname].tabs[tabname].viewers.length:0;
        if (!active)
            active = "list";

        if (!isok(bgColorMap))
            bgColorMap = [ 'white', '#f2f2f2' ];

        var viewerList = new vjTableView(
            {
                //debug:1,
                name : 'list',
                icon : 'list',
                data : dsname,
                formObject : document.forms[formname],
                bgColors : bgColorMap,
                precompute : precompute?precompute:"if (node.icon=='self') node.icon= '?cmd=objFile&propname=thumb&prefix=0&ids='+node.id;else {var icon=vjHO.get(node._type,'icon',node.category);if(icon)node.icon=icon;else node.icon='rec';}",
                cols :showListCols,
                selectCallback : selectCallback ? selectCallback : 0,
                dbClickCallback: dbClickCallback ? dbClickCallback : 0,
                checkable : false,
                multiSelect:(lmultiSelect==undefined?true:lmultiSelect),
                objectsDependOnMe : [ dvname + '.' + tabname + '.' + cntViewersIn ],
                selectInsteadOfCheck:true,
                defaultIcon : 'rec',
                iconSize : 24,
                iconWidth:24,
                notOpenUrl:this.notOpenUrlForTableView ? true:false,
                maxTxtLen : this.maxTxtLen,
                paste:this.paste,
                defaultEmptyText : 'no accessible information to show',
                geometry : {
                    width : '100%'
                },
                waitOnLoad:true,
                drag : this.drag,
                exclusionObjRegex:exclusionObjRegexArr,
                dragCancelCallback:onDragCancel,
                dragStartCallback : onDragStart,
                dragStopCallback : onDragStop,
                dragDropCallback : DropHandler,
                dropOnAttrs : null,
                dragRow : true,
                callbackRendered:callbackRendered,
                dragSetCloneCallback: setCloneDrag,
                isok : true
            });


        gClip.objectsDependOnMe.push(viewerPanel);
        this.vjDV[dvname].add( tabname, tabico, "tab", [ viewerPanel,viewerList ]);
        var viewer = this.vjDV.locate(dvname + "." + tabname + "." + (cntViewersIn + 1));

        return [viewerPanel,viewer];
    };

    this.onRefreshSharing = function() {
        this.dsPreviewsRecord.reload(undefined, true);
        this.dsPreviewsJson.reload(undefined, true);
    };

    this.onEditSharing = function(dvname, id) {
        linkURL("?cmd=sharing&ids=" + id, "sharing" + id);
    };
};


//# sourceURL = getBaseUrl() + "/js/vjExplorerBaseView.js"