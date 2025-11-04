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
$(function (){
    var oThis;
    $.widget("layout.explorerview", $.layout.layoutmanager, {
        options:{
            data:'dsVoid',
            idtype: 'id',
            container: "dvExplorer"+Math.random(),
            currentFolder: "",
            folderIcons: {folder:"folder-open.gif",all:"all_folder.png",Inbox:"mail_box.png",Trash:"trash_folder.png"},
            vjDS: vjDS,
            vjDV: vjDV,
            drag: true,
            autoexpand: 1
        },        
        _onBeforeInit: function () {
            oThis = this;
            
            if(oThis.options.preselectedFolder === undefined) {
                oThis.options.preselectedFolder = docLocValue("folder");
            }if(!oThis.options.preselectedFolder) {
                if(oThis.options.useCookies) {
                    oThis.options.preselectedFolder=cookieGet("explorer_curdir_open",undefined);
                }
            }if(!oThis.options.preselectedFolder) {
                oThis.options.preselectedFolder=user_getvar("curdir_open",undefined);
            }
            if(!oThis.options.visitedFolders) {
                oThis.options.visitedFolders = cookieGet("explorer_visited_folders",undefined);
                if(oThis.options.visitedFolders)oThis.options.visitedFolders = oThis.options.visitedFolders.split(',');
            }
            oThis.options.objCls=oThis.options.container.replace(/\W/g, "_");
            vjObj.register(oThis.options.objCls,this);
            if(!oThis.options.folderURL){
                if(oThis.options.qryLang){
                    oThis.options.folderURL =  "http:
                    var objQrytxt="alloftype('sysfolder|folder')";
                    if(oThis.options.partialFolderLoading ){
                        objQrytxt="[alloftype('sysfolder'),(alloftype('sysfolder')";
                        if(oThis.options.visitedFolders && oThis.options.visitedFolders.length)
                            objQrytxt+= '.append(["' + oThis.options.visitedFolders.join('","') + '"] as objlist) ';
                        objQrytxt+=") .map({.child = (.child as objlist).filter({._type=='folder'})}).reduce(function(x,y){x.append(y)})]";
                        objQrytxt+=".reduce(function(x,y){x.append(y)})" ;
                    }
                    objQrytxt+=".csv(['name','child'])";
                    oThis.options.folderURL += vjDS.escapeQueryLanguage(objQrytxt);
                }
                else
                    oThis.options.folderURL =  "http:
            }
            if(!oThis.options.container_Folders)oThis.options.container_Folders=oThis.options.container+"_Folders";
            if(!oThis.options.container_Tables)oThis.options.container_Tables=oThis.options.container+"_Tables";
            if(!oThis.options.container_Previews)oThis.options.container_Previews=oThis.options.container+"_Previews";
            if(!oThis.options.container_Submit)oThis.options.container_Submit=oThis.options.container+"_Submit";
            
            if(!oThis.options.formFolders)oThis.options.formFolders="form-"+oThis.options.container_Folders;
            if(!oThis.options.formTables)oThis.options.formTables="form-"+oThis.options.container_Tables;
            if(!oThis.options.formPreviews)oThis.options.formPreviews="form-"+oThis.options.container_Previews;
            if( oThis.options.isSubmitMode ) {
                oThis.options.isDisplay_Submit = true;
            }
            this.options.flV={
                    name: oThis.options.container,
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
            
            oThis.element.append($(document.createElement("div")).attr("id", oThis.options.container));
            
            oThis.options.data = verarr(oThis.options.data);
            $(oThis.options.data).each(function(i, dsName){
                vjDS[dsName].register_callback(oThis.allReady);
                vjDS[dsName].load();
            });            
            
            


            oThis.init_DS();

            if(oThis.folders_DV)
                oThis.init_Folders_viewers();
            if(oThis.tables_DV)
                oThis.init_Tables_viewers();
            if(oThis.previews_DV)
                oThis.init_Previews_viewers();
            if(oThis.submit_DV)
                oThis.init_Submit_viewers();
            
            this.options.config = {
                layout: {
                    items: [
                        {
                            id: 'folders',
                            top: '0',
                            left: '0',
                            right: '25%',
                            bottom: '100%',
                            toggler: "east",
                                    view: {
                                        name: 'directorywidget',
                                        options: {
                                            data: "ds"+oThis.options.container+"Folders",
                                            onSelectCapture: oThis.selectFolderCB
                                        }
                                    }
                        }
                    ]
                }
            };
            
            if (oThis.options.isDisplay_Submit){
                this.options.config.layout.items.push({
                    id: 'table',
                    top: '0',
                    left: '25%',
                    right: '100%',
                    bottom: '80%',
                });
                this.options.config.layout.items.push({
                    id: 'submit',
                    top: '80%',
                    left: '25%',
                    right: '100%',
                    bottom: '100%',
                    view: oThis.init_Submit()                   
                });
            }
            else{
                this.options.config.layout.items.push({
                    id: 'table',
                    top: '0',
                    left: '25%',
                    right: '100%',
                    bottom: '60%',
                });
                this.options.config.layout.items.push({
                    id: 'preview',
                    top: '60%',
                    left: '25%',
                    right: '100%',
                    bottom: '100%',
                });
            }
        },
        init_DS: function()
        {
            oThis.options.dsFolders = oThis.options.vjDS.add("infrastructure: Folders", "ds"+oThis.options.container+"Folders", oThis.options.folderURL);
            oThis.options.dsFoldersToolbar = oThis.options.vjDS.add("infrastructure: Folders Toolbar", "ds"+oThis.options.container+"FoldersToolbar","static:
            oThis.options.dsFoldersHelp = oThis.options.vjDS.add("infrastructure: Folders Help", "ds"+oThis.options.container+"FoldersHelp","http:
            oThis.options.dsTablesHelp = oThis.options.vjDS.add("infrastructure: Files and sequences help documentation", "ds"+oThis.options.container+"TablesHelp","http:
            oThis.options.dsPreviewsHelp = oThis.options.vjDS.add("infrastructure: Preview help documentation", "ds"+oThis.options.container+"PreviewsHelp","http:
            oThis.options.dsPreviewsRecord = oThis.options.vjDS.add("infrastructure: Loading Objects Metadata Information", "ds"+oThis.options.container+"PreviewsRecord" , "static:
            oThis.options.dsPreviewsSpec = oThis.options.vjDS.add("infrastructure: Obect Specifications","ds"+oThis.options.container+"PreviewsSpec" , "static:
            oThis.options.dsPreviewsUserTree = oThis.options.vjDS.add("infrastructure: Retrieving User/Group Hierarchy", "ds"+oThis.options.container+"UserTree", "http:
            oThis.options.dsPreviewsUserList = oThis.options.vjDS.add("infrastructure: Retrieving User List", "ds"+oThis.options.container+"UserList", "http:
        },
        destroyDS: function()
        {
            delete oThis.options.vjDS[oThis.options.dsFolders.name];
            delete oThis.options.vjDS[oThis.options.dsFoldersToolbar.name];
            delete oThis.options.vjDS[oThis.options.dsFoldersHelp.name];
            delete oThis.options.vjDS[oThis.options.dsTablesHelp.name];
            delete oThis.options.vjDS[oThis.options.dsPreviewsHelp.name];
            delete oThis.options.vjDS[oThis.options.dsPreviewsRecord.name];
            delete oThis.options.vjDS[oThis.options.dsPreviewsSpec.name];

            delete oThis.options.vjDS[oThis.options.dsPreviewsUserTree.name];
            delete oThis.options.vjDS[oThis.options.dsPreviewsUserList.name];

            for (var i=0 ; i < oThis.options.viewerFSTable.length ; ++i ) {
                delete oThis.options.vjDS[oThis.options.viewerFSTable[i].data];
            } 
        },
        allReady: function(viewer, content){
            if (++oThis.options.dataCounter < oThis.options.data.length)
                return;                
           
            oThis.refresh(content);
        },
        refresh: function (content){
            
        },
        init_Folders_viewers: function (){
            var toReturn=[];
            
            toReturn.push ({
                name : 'hierarchy',
                icon : 'tree',
                drag : oThis.options.drag,
                qryLang:oThis.options.qryLang,
                partialFolderLoading:oThis.options.partialFolderLoading,
                visitedFolders:oThis.options.visitedFolders,
                data : oThis.options.dsFolders.name,
                hierarchyColumn : 'path',
                highlightRow : true,
                expandSize : 12,
                folderSize : 24,
                showRoot : 0,
                showLeaf : oThis.options,
                hideEmpty : oThis.options,
                refreshDelayCallback:"vjObjEvent('onRefreshDelayTables','"+oThis.options.objCls+"',$(id))",
                preselectedFolder : oThis.options.preselectedFolder,
                setDragElementsOperation:"if(node.path && node._type!='sysfolder'){var o=gObject(sanitizeElementId(params.that.container+node.path));" +
                        "if(o)o=o.parentNode;if(o){params.list.push(o);}}",
                setDropElementsOperation:"if(node.path && !node.isVirtual ){var o=gObject(sanitizeElementId(params.that.container+node.path));" +
                        "if(o)o=o.parentNode;if(o){params.list.push(o);}}",
                showChildrenCount : oThis.options.showChildrenCount,
                icons       :{leaf:null},
                urls: {
                     input_folder : "function:vjObjFunc('input_folder','"+oThis.options.objCls+"')",
                       undo :"function:vjObjFunc('undo','"+oThis.options.objCls+"')",
                    redo :"function:vjObjFunc('redo','"+oThis.options.objCls+"')",
                       refresh :"function:vjObjFunc('refresh','"+oThis.options.objCls+"')"
                       },
                autoexpand : oThis.options.autoexpand,
                maxTxtLen : 64,
                onSelectFolder: "function:vjObjFunc('onSelectFolder','"+oThis.options.objCls+"')",
                isok : true
            });
            
            toReturn.push({data : this.dsFoldersHelp.name});
            
            return toReturn;
        },
        init_Previews_viewers: function()
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
        },
        init_Submit_viewers: function()
        {
            var rows=[{ name : 'display', type : 'text', title : 'Select object(s) to add', value : 'Select object(s) to add', align:'right', readonly:true, size:80, prefix:'Selected object(s):  ', order : 1},
                    { name : 'submit', type : 'button', value:'Submit', align: 'right' , order : 2, url : "javascript:vjObjEvent(\"onSubmitObjs\",\"" + this.objCls + "\")"},
                    { name : 'clear', type : 'button', value:'Clear', align: 'right' , order : 3, url : "javascript:vjObjEvent(\"onClearSubmitObjs\",\"" + this.objCls + "\")"}
                    ];

           return { name: 'dataview', 
                       options:{ dataViewer: 'vjPanelView', dataViewerOptions: {
                        data:["dsVoid"],
                        rows: rows,
                        formObject: document.forms[oThis.options.formName],
                        isok: true }}
                   };
        },
        init_Tables_viewers: function()
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

            vjPAGE.initStandardDownloader(this.subTableDefault.dvname,
                    "Downloader", this.subTableDefault.formname,
                    "dmDownloader.cgi?cmd=-qpProcSubmit&svc=dmDownloader");


            var tabname = "help", tabico = "help";
            var dsname = 'ds-' + this.subTableDefault.dvname + '-' + tabname;
            var dsurl = "http:
            if (this[dsname] !== undefined)
                dsurl = this[dsname];

            this.dsFoldersHelp = this.vjDS.add("infrastructure: Folders Help", dsname, dsurl);
            this.tables_DV.add(tabname, tabico, "tab", [ new vjHelpView({ data : dsname }) ]);
        },
        selectFolderCB: function (a,b,c,d){
            console.log("selectFolderCB");
        },
        onSubmitObjs: function(){
            
        },
        onClearSubmitObjs: function(){
            
        }
    });
    
}(jQuery));
