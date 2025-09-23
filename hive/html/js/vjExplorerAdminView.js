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

function vjExplorerAdminView(viewer)
{

    vjExplorerBaseView.call(this,viewer);


    this.init_DS=function()
    {
        this.dsFolders=vjDS.add("infrastructure: Folders", "ds"+this.container+"Folders", this.folderURL,null,"id,path,name,email\n");
        this.dsFoldersToolbar=vjDS.add("infrastructure: Folders Toolbar", "ds"+this.container+"FoldersToolbar","static:
        this.dsPreviewsRecord=vjDS.add("infrastructure: Loading Objects Metadata Information", "ds"+this.container+"PreviewsRecord" , "static:
        this.dsPreviewsSpec=vjDS.add("infrastructure: Obect Specifications","ds"+this.container+"PreviewsSpec" , "static:
        this.dsFoldersHelp=vjDS.add("infrastructure: Folders Help", "ds"+this.container+"FoldersHelp","http:
    };
    this.addCmd =     [{
        name : 'Create11',
        align : 'left',
        order : 1,
        hidden : false,
        url : "function:vjObjFunc('onAction','" + this.objCls + "')",
        icon : 'plus',
        prohibit_new : true
    },
    {
        name : 'upload',
        hidden : true,
        prohibit_new : true,
        isok:false
    },
    {
        name : 'delete',
        hidden : false,
        refreshDelay : 1000,
        prohibit_new : true
    },
    {
        name : 'edit',
        hidden : false,
        icon:'eye',
        prohibit_new : true
    },
    {
        name : 'detail',
        hidden : false,
        icon:'rec',
        prohibit_new : true
    },


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
        hidden : true,
        inactive:true,
        prohibit_new : true
    },
    {
        name : 'paste',
        hidden : true,
        refreshDelay : 1000,
        inactive:'javascript:gClip.content.length? false :true',
        prohibit_new : true
    },
    {
        name : 'download',
        hidden : true,
        prohibit_new : true
    },
    {
        name : 'share',
        hidden : false,
        prohibit_new : true
    }]

    this.subTableDefault={
            dvname      : this.container_Tables,
            tabname     : "All Kinds Objs",
            tabico      : "file",
            recordviewer: this.container_Previews,
            formname    : this.formTables,
            active      : this.active,
            fullpanel   : true,
            bgClrMap    : this.bgClrMap,
            addCmd      : this.addCmd,
            category    : null,
            objId        : 'userPerspective',
            url_tmplt   : "http:
            url         : "static:
            selectCallback      : "function:vjObjFunc('onSelectFile','" + this.objCls + "')",
            callbackRendered    : "function:vjObjFunc('onRenderTableViewer','" + this.objCls + "')",
            precompute:"node=funcLink(\"function:vjObjFunc('precomputeTable','"+this.objCls+"')\",node,this);",
            isok:true
        };



    this.subTableDefault.editCmd = [

                                    {
                                        name : new RegExp(/.*/),
                                        hidden : true,
                                        refreshDelay : 1000,
                                        prohibit_new : true
                                    },
                                    ];


    this.init_Folders_viewers=function()
    {
        var dirViewers = new vjDirectoryAdminControl ({
            name : 'hierarchy',
            icon : 'tree',
            qryLang:this.qryLang,
            data : this.dsFolders.name,
            hierarchyColumn : 'path',
            highlightRow : true,
            expandSize : 12,
            folderSize : 24,
            showRoot : 1,
            showLeaf : true,
            hideEmpty : false,
            userId : this.userId,
            setDragElementsOperation:"if(node.path && node.depth>1){var o=gObject(sanitizeElementId(params.that.container+node.path));" +
                    "if(o)o=o.parentNode;if(o){params.list.push(o);}}",
            setDropElementsOperation:"if(node.path && (!(node.depth==1 && node.title=='All')) ){var o=gObject(sanitizeElementId(params.that.container+node.path));" +
                    "if(o)o=o.parentNode;if(o){params.list.push(o);}}",
            showChildrenCount : true,
            icons       :{leaf:null},
            urls: {
                 input_folder : "function:vjObjFunc('input_folder','"+this.objCls+"')",
                 delete_folder: "function:vjObjFunc('onDeleteFolder','" + this.objCls + "')",
                   undo :"function:vjObjFunc('undo','"+this.objCls+"')",
                 redo :"function:vjObjFunc('redo','"+this.objCls+"')",
                   refresh :"function:vjObjFunc('refresh','"+this.objCls+"')"
                   },

            formObject : document.forms[this.formFolders],
            autoexpand : 2,
            maxTxtLen : 200,
            drag : true,
            onSelectFolder: "function:vjObjFunc('onSelectFolder','"+this.objCls+"')",
            isok : true
        });
        this.viewerFSTree = dirViewers[1];
        this.viewerFSTreePanel = dirViewers[0];

        this.folders_DV.add("home", "tree", "tab", [ dirViewers[0], dirViewers[1]]);
        this.folders_DV.add("help", "help", "tab", [ new vjHelpView({data : this.dsFoldersHelp.name}) ]);
    };

    this.notOpenUrlForTableView=true;
    this.notAddHelpAtPrewView = true;
    this.appendUserIdToFSTable = true;


    this.previews_DV_selected = 0;
    this.NotShowFullPanel =  true;
    this.appendUserPerspective = true;



    this.onAction=function(viewer,node){
        if(node.name=='Create11'){

                if(viewer.tab.name!='All'){
                        var url = "?cmd=record&types="+viewer.tab.name;
                        if(this.currentUserId && this.currentUserId!='all') url +="&userId="+this.currentUserId;
                        window.open(url);
                }
                else{
                        if(viewer.objectsIDependOn[0].accumulate("node.checked>0 && node.id","node.id",0).length){
                                node.url = "?cmd=record&types=$(types)";
                                viewer.onClickMenuNode(viewer.container,node.path);
                        }
                        else {
                                alert('select an object to create that obj type');
                        }
                }
        }else if(node.name=='edit11' || node.name=='detail11'){
                var url = "?cmd=record&ids=$(ids)&types=$(types)" ;
                if(node.name=='detail11')
                viewer.onClickMenuNode(viewer.container,node.path);
        }

    };



    this.subTablesAttrs = [
                           {
                               tabname : "All",
                               tabico : "rec",
                               url : { type : "menuitem,u-usage,action,special,view,webform,user,group,type,webform-Setting,algorithm,qpsvc,qphost,user-info", prop:"id,_brief,created" }

                           }, {
                               tabname : "menuitem",
                               tabico : "rec",
                               url : { type : "menuitem",prop: "id,created,path,title,url"}
                           }, {
                               tabname : "u-usage",
                               tabico : "ico-file",
                               url : { type : "u-usage",prop:"user,records_count,id,created" }
                           }, {
                               tabname : "action",
                               tabico : "ico-image",
                               url : { type : "action" ,prop:"id,created,name,path,title,type_name,url"}
                           },
                           {
                               tabname : "view",
                               tabico : "ico-image",
                               url : { type : "view" ,prop:"id,created,name,path,title,type_name,url"}
                           },{
                               tabname : "qpsvc",
                               tabico : "rec",
                               url : { type : "qpsvc",prop: "id,name,title"}
                           },{
                               tabname : "special",
                               tabico : "svc-process",
                               url : { type : "special",prop:"meanning,id,created,version,title"}
                           },{
                               tabname : "qphost",
                               tabico : "rec",
                               url : { type : "qphost",prop: "id,name,title"}
                           },{
                               tabname : "algorithm",
                               tabico : "rec",
                               url : { type : "algorithm",prop: "id,name,title"}
                           },{
                               tabname : "type",
                               tabico : "rec",
                               url : { type : "type",prop: "id,name,title"}
                           },{
                               tabname : "webform-Setting",
                               tabico : "rec",
                               url : { type : "webform-Setting",prop: "id,name,title"}
                           },{
                               tabname : "user",
                               tabico : "rec",
                               url : { type : "user",prop: "id,name,title"}
                           },{
                               tabname : "grouup",
                               tabico : "rec",
                               url : { type : "grouup",prop: "id,name,title"}
                           },{
                               tabname : "user",
                               tabico : "rec",
                               url : { type : "user",prop: "id,name,title"}
                           },{
                               tabname : "user-info",
                               tabico : "help",
                               url : { type : "user-info",prop: "id,name,title"}
                           },{
                               tabname : "webform",
                               tabico : "rec",
                               url : { type : "webform",prop: "id,name,title"}
                           }];
    this.onSelectFolder = function(view, node, content )
    {

        this.currentUserId = node.id;
        if(this.onSelectFileCallback){
            funcLink(this.onSelectFileCallback, viewer, null);
        }

        if ( (!node.selected || this.currentFolder==this.viewerFSTree.currentFolder) && !this.viewerFSTree.selectedNodeContentChanged)
            return false;

        if(view.currentFolder)
            this.currentFolder = view.currentFolder;
        var added_prop=undefined;

        if(this.qryLang)
            this.queryArrivedCallback_QryLang(node,content,added_prop);
        else
            this.queryArrivedCallback(node,content,added_prop);

        for(var i = 0 ; i < this.viewerFSPanel.length ; ++i ) {
            if(!this.viewerFSPanel[i].evalVariables) {
                this.viewerFSPanel[i].evalVariables = new Object();
            }
            this.viewerFSPanel[i].evalVariables.srcFolder = this.currentFolder.split("/").pop();
        }
        if(this.viewerFSTreePanel && !this.viewerFSTreePanel.evalVariables) {
            this.viewerFSTreePanel.evalVariables = new Object();
        }
        var src_arr = this.currentFolder.split("/");
        src_arr.pop();
        if(this.viewerFSTreePanel)this.viewerFSTreePanel.evalVariables.srcFolder = src_arr.pop();


    };





};


