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
                            objQrytxt+= '.append(["' + oThis.options.visitedFolders.join('","')+ '"] as objlist) ';
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
        }
    });
}(jQuery));
