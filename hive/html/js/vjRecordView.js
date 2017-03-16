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

function vjRecordView ( viewer )
{
    vjDataViewViewer.call(this,viewer); // inherit default behaviours of the DataViewer

    this.spcTypeObj="spcTypeObj";

    if(!this.tblClass) this.tblClass="REC";
    if(!this.formProviderDoc)this.formProviderDoc=document;
    if(!this.inputClass)this.inputClass=this.tblClass+"_input";
    if(!this.expansion)this.expansion=1;
    if(!this.autoexpand)this.autoexpand=1;
    if(!this.textsize)this.textsize=18;
    if(!this.autoStatus)this.autoStatus=0;
    if(!this.objType)this.objType="0";
    if(!this.hiveId)this.hiveId="0";
    if(!this.propFormatId) this.propFormatId = parseHiveId(this.hiveId).objId ? this.hiveId : this.objType;
    if(!this.RVtag)this.RVtag="RV";
    if(!this.cmdPropSet)this.cmdPropSet="?cmd=propset";
    if (this.accumulateWithNonModified === undefined) this.accumulateWithNonModified=true;
    if(!this.accumulateWithoutHidden)this.accumulateWithoutHidden=false;
    if(!this.showReadonlyInNonReadonlyMode)this.showReadonlyInNonReadonlyMode=false;
    if(!this.readonlyMode)this.readonlyMode=false;
    if (!this.vjDV) this.vjDV = vjDV;
    if (!this.constructAllNonOptionField) this.constructAllNonOptionField = false;
    if (!this.constructAllField) this.constructAllField = false;
    if (!this.classChoiceStyle) this.classChoiceStyle = "REC_input";
    if (!this.classSearchStyle) this.classSearchStyle = "REC_inputSearch";
    if (!this.implementTreeButton) this.implementTreeButton = false;
    if (!this.implementSaveButton) this.implementSaveButton = false;
    if (!this.implementCopyButton) this.implementCopyButton = false;
    if (!this.implementSetStatusButton) this.implementSetStatusButton = false;
    if (!this.showReadonlyWhenValueThere) this.showReadonlyWhenValueThere = false;
    // callback to check if element value needs to be split by a separator before validation, e.g.
    // this.getValidateSeparatorCb(fld_name) { return name == 'foo' ? ',' : null; }
    if (!this.getValidateSeparatorCb) this.getValidateSeparatorCb = function(fld_name) { return null; };

    this._evalKeys = ["$(","$_("];

  //  if (!this.accumulateWithNonModified) this.accumulateWithNonModified = true;
    if(!this.vjDS)this.vjDS=vjDS;

    if(!this.icons)this.icons={
        collapse: "<img border=0 width=16 height=16 src='img/recCollapse.gif' title='collapse the hierarchy of sub-elements' />" ,
        expand: "<img border=0 width=16 height=16 src='img/recExpand.gif' title='expand the hierarchy of sub-elements' />" ,
        help: "<img border=0 width=12 height=12 src='img/recQuestion.gif' title='help' />" ,
        lock: "<img border=0 width=16 height=16 src='img/recLock.gif' title='the field cannot be modified' />" ,
        search: "<img border=0 width=16 height=16 src='img/recSearch.gif' title='lookup possible values' />" ,
        link: "<img border=0 width=16 height=16 src='img/recLink.gif' title='jump to appropriate internet location' />" ,
        delRow: "<img border=0 width=16 height=16 src='img/recDelete.gif' title='delete the specified element' />",
        clearRow: "<img border=0 width=16 height=16 src='img/delete.gif' title='clear the content' />",
        addRow: "<img border=0 width=16 height=16 src='img/recAdd.gif' title='create new sub-element' />" ,
        addRowMore: "<img border=0 width=16 height=16 src='img/recAddMore.gif' />", // no title - that is set in parent button
        revertRow: "<img border=0 width=16 height=16 src='img/recRevert.gif' title='discard modifications' />" ,
        setRow: "<img border=0 width=16 height=16 src='img/recSet.gif' title='save the modifications' />" ,
        itemRow: "<img border=0 width=16 height=16 src='img/recItem.gif' title='input element value' />" ,
        errorRow: "<img border=0 width=16 height=16 src='img/recError.gif'/>", // no title - that is set in parent span
        notEmpty:"<img border=0 width=16 height=16 src='img/recNotEmpty.gif'  title='field cannot be empty'/>" ,
        white:"<img border=0 width=16 height=16 src='img/white.gif'  title='white img taken one img place'/>" ,
        urlJump: "<img border=0 width=16 height=16 src='img/new.gif' title='jump to the external website' />"
        };


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ main composer function is called when the viewer control needs to be drawn upon arrival of the data
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    this.composerFunction=function( viewer, content )
    {

        var tMyFloater=gObject("dvFloatingDiv").outerHTML;
        if(!tMyFloater)tMyFloater="";
        this.myFloaterName="floater-"+this.objCls;
        //this.debug=1;
        tMyFloater=tMyFloater.replace(/dvFloating/g,this.myFloaterName);
        tMyFloater=tMyFloater.replace(/href=\"#\" onclick=\".*\return false;\">/g, "href=\"#\" onclick=\"vjObjEvent('onClosepop','"+this.objCls+"'\)return false;\">");
        gCreateFloatingDiv(tMyFloater);
        //alert(content);
        //this.debug=1;

        var want_new_fldTree = this.force_fld_load || !this.fldTree || this.fldTree.t_sessionID !== this.getDataSessionID(0);
        var want_new_nodeTree = this.force_node_load || !this.nodeTree || this.nodeTree.t_sessionID !== this.getDataSessionID(1);

        if (want_new_fldTree) {
            var tbl = new vjTable(this.getData(0).data, 0, vjTable_propCSV);
            if(this.rows)
                tbl.customizeRows(this.rows);

            var objTypeTitle = tbl.accumulate(function(node) { return node.name == "_type"; }, function(node) { return node.title; });
            if (objTypeTitle.length) {
                objTypeTitle = objTypeTitle[0];
            } else {
                objTypeTitle = this.objType;
            }

            // We don't want system properties (e.g. _type, _action) in the record
            for (var ir=0; ir<tbl.rows.length; ir++) {
                while (ir<tbl.rows.length && tbl.rows[ir].name[0] == "_") {
                    tbl.rows.splice(ir, 1);
                }
            }

            this.fldTree=new vjTree(tbl );
            this.fldTree.root.title = objTypeTitle;
            this.fldTree.root.type="list";
            this.fldTree.t_sessionID = this.getDataSessionID(0);
        }

        if (want_new_nodeTree) {
            if (this.data.length > 1 && this.reloadObjIDFromData && !(parseHiveId(this.hiveId).objId > 0)) {
                var elemTxt = this.getData(1).data;
                if (elemTxt.indexOf("preview:") !== 0) {
                    var prelimElemArr = new vjTable(elemTxt, 0, vjTable_hasHeader);
                    for (var ir=0; ir<prelimElemArr.rows.length; ir++) {
                        var id = prelimElemArr.rows[ir].id;
                        if (id > 0) {
                            this.hiveId = id;
                            break;
                        }
                    }
                }
            }

            this.nodeTree=new vjTree( ); // parsing propspec
            this.nodeTree.root=cpyObj(this.nodeTree.root, {value: '' , value0: '', fld: this.fldTree.root, row: 0, subRow: 0, obj: this.hiveId, expanded:this.expansion , children: new Array (), depth: 0, path:'/', name:'root'} );
            this.nodeTree.t_sessionID = this.getDataSessionID(1);
        }

        if (want_new_fldTree) {
            this.fldTree.root.children.splice(0,0,{ type_id:-1, default_value: this.objType, name: this.spcTypeObj, title: 'ObjectType', type: '', hidden: true, parent:this.nodeTree.root , is_key_fg:1, is_readonly:1, is_readonly_fg:-1, is_optional:0 , is_multi_fg: 0, is_hidden_fg: 0,  description: 'Determines the type of the object' , children: new Array()});
            if(this.fldPresets)
                this.setFields(this.fldPresets);
            this.fldTree.enumerate(
                function(params, node) {
                    node.is_optional_fg = node.is_optional_fg ? parseInt(node.is_optional_fg) : 0 ;
                    node.elname = params.obj.RVtag+"-"+node.name;
                    node.div = params.document.getElementById( node.elname );
                    node.dsSource = params.obj.dataSourceEngine["ds"+params.obj.RVtag+"-"+node.name];
                    var showNode=true;
                    if (node.is_readonly_fg) {
                        var int_readonly_fg = parseInt(node.is_readonly_fg);
                        if(parseHiveId(params.obj.hiveId).objId>0) {
                            node.is_readonly_fg = (int_readonly_fg != 0);
                            showNode = !node.is_readonly_fg;
                            node.is_submittable = int_readonly_fg == 0;
                        } else if( params.obj.cloneMode ) {
                            node.is_readonly_fg = ( int_readonly_fg > 0 || int_readonly_fg ==-2 );
                            node.is_submittable = (int_readonly_fg != 1 );
                        } else {
                            node.is_readonly_fg = (int_readonly_fg > 0);
                            showNode = !node.is_readonly_fg;
                            node.is_submittable = int_readonly_fg != 1;
                        }
                        node.reset_to_default = (int_readonly_fg > 1);
                    } else {
                        node.is_readonly_fg = false;
                        node.reset_to_default = false;
                        node.is_submittable = true;
                    }

                    if(params.obj.readonlyMode) {
                        node.is_readonly_fg=true;
                    }
                    else if( !params.obj.showReadonlyInNonReadonlyMode && !showNode ) {
                        node.hidden = true;
                    }

                    if (node.type == "bool") {
                        if (node.default_value && node.default_value.length) {
                            node.default_value = parseBool(node.default_value) ? '1' : '0';
                        } else {
                            node.default_value = '';
                        }
                        // render non-optional bools as a dialog, forcing the user to choose yes or no
                        if (!node.is_optional_fg && !node.is_readonly_fg) {
                            node.constraint = "choice";
                            node.constraint_data = "1///yes|0///no";
                        }
                    }

                    node.is_hidden_fg=node.is_hidden_fg ? parseInt(node.is_hidden_fg) : 0 ;
                    node.is_batch_fg=node.is_batch_fg ? parseInt(node.is_batch_fg) : 0 ;
                    if(node.is_hidden_fg) node.hidden = true;
                    node.is_multi_fg = node.is_multi_fg ? parseInt(node.is_multi_fg) : 0;
                    //alerJ(params.RVtag+'ds-'+'node.name',node);
                    node.maxEl = node.is_multi_fg ? 0x7FFFFFFF : 1;
                    if(node.parent && node.parent.children.length>1 && node.parent.type=="array") {
                        node.doNotCollapseMultiValueViewers=true;
                        node.is_multi_fg=false;
                    }
                },
                { document: this.formProviderDoc, obj: this }
            );
            this.fldTree.enumerate(
                    function(params, node) {
                    if(node.type =='password' && !node.isCopy && !node.isCopied) {
                        var parent = node.parent;
                        parent.children.push(new vjTreeNode(node));
                        parent.children.push(new vjTreeNode(node));
                        node.isCopied = true;
                        
                        var newNode = parent.children[parent.children.length-2];
                        newNode.isCopy=true;
                        newNode.title = 'New '+ node.title;
                        newNode.subpath = "new";
                        newNode.name += '.' + newNode.subpath;
                        newNode.path += '.' + newNode.subpath;
                        node.order = node.order?node.order:10000
                        newNode.order = node.order+.0001;
                        var confirmNode = parent.children[parent.children.length-1];
                        confirmNode.isCopy=true;
                        confirmNode.title = 'Confirm '+node.title;
                        confirmNode.subpath = "confirm";
                        confirmNode.name += '.' + confirmNode.subpath;
                        confirmNode.path += '.' + confirmNode.subpath;
                        confirmNode.order = newNode.order+.0001;
                        
                        confirmNode.passwordSiblingNode = newNode;
                        confirmNode.passwordVerifiableNode = newNode;
                        newNode.passwordSiblingNode = confirmNode;
                        newNode.passwordConfirmingNode = confirmNode;
                        node.passwordSiblingNodes = [newNode,confirmNode];
                        
                        node.subpath = "old";
//                        node.name += '.'+ node.subpath;
//                        node.path += '.'+node.subpath;
                        node.title = 'Old '+ node.title;
                   }
                },
                { document: this.formProviderDoc, obj: this }
            );

            if(this.fldEnumerate)
                this.fldTree.enumerate(this.fldEnumerate, { document: this.formProviderDoc, obj: this });


//          if(this.readonlyMode){
//              this.fldTree.enumerate( "node.is_readonly_fg=true;") ;
//          }else {
//              if(!this.showReadonlyInNonReadonlyMode)
//                  this.fldTree.enumerate("if(node.is_readonly_fg){node.hidden = true;}");
//
//          }
            //this.fldTree.enumerate("if(node.hidden)node.type='string';");
        }

        if (this.data.length > 1 && want_new_nodeTree) { // new values are specified
            var elemTxt = this.getData(1).data;
            if( elemTxt.indexOf("preview:")==0 ) {
                this.div.innerHTML=elemTxt.substring(8);
                return ;
            }

            //elemTxt="id,name,path,value\n";
            elemTxt += this.hiveId + "," + this.spcTypeObj + ",0," + this.objType + "\n";
           // alert(elemTxt)
            //alert(this.hiveId+ "\n " + elemTxt);
            this.elemArr = new vjTable(elemTxt, 0, vjTable_hasHeader);

            if (!(parseHiveId(this.hiveId).objId>0)) {
                // For fields that need to be reset to default, change element value
                // to default if the default value is specified, and remove element
                // if the default value is empty
                var newElemArrRows = [];
                for (var ir=0; ir<this.elemArr.rows.length; ir++) {
                    var elemRow = this.elemArr.rows[ir];
                    var fld = this.fldTree.findByName(elemRow.name);
                    if (fld && fld.reset_to_default && (elemRow.value === undefined || !elemRow.value.length)) {
                        if (!fld.default_value || !fld.default_value.length)
                            continue;

                        elemRow.value = elemRow[3] = fld.default_value;
                    }
                    newElemArrRows.push(elemRow);
                }
                this.elemArr.rows = newElemArrRows;
            }

            this.elemArr.enumerate(function(this_, tbl, ir) {
                var node = tbl.rows[ir];
                if (node.path) {
                    node.path=node.path.split('.');
                } else {
                    node.path='0';
                }
            }, this);

            if (this.elemArr.rows.length) { // this.nodeTree.root
                // sort fields for correct insertion order: by path, then by field order, then by name
                var this_ = this;
                this.elemArr.rows.sort(function(a, b) {
                    for (var i = 0; i < a.path.length && i < b.path.length; i++) {
                        var diff = parseInt(a.path[i]) - parseInt(b.path[i]);
                        if (diff < 0 || diff > 0) {
                            return diff;
                        }
                    }
                    var afld = this_.fldTree.findByName(a.name);
                    var bfld = this_.fldTree.findByName(b.name);
                    if (afld && bfld) {
                        var diff = parseFloat(afld.order) - parseFloat(bfld.order);
                        if (diff < 0 || diff > 0) {
                            return diff;
                        }
                    }
                    if (a.name < b.name) {
                        return -1;
                    } else if (a.name > b.name) {
                        return 1;
                    }
                    return 0;
                });
                for (var i=0; i<this.elemArr.rows.length; i++) {
                    this.elemArr.rows[i].irow = i;
                }
                this.nodeTree.root=this.populateInfrastructure ( this.nodeTree.root  , this.elemArr, this.hiveId );
            }

        }




        if (this.constructAllNonOptionField) this.constructionPropagateDown = 300;

        if((this.data.length<1 || this.constructionPropagateDown) && (!this.readonlyMode|| this.showReadonlyInNonReadonlyMode) && !this.editExistingOnly ){
            this.constructInfrastructure ( this.nodeTree.root , 0,  true , this.constructionPropagateDown);
        }

        if (this.showReadonlyWhenValueThere && want_new_fldTree) {
            this.nodeTree.enumerate("if(node.fld.is_readonly_fg && node.fld.type!='list' && node.fld.type!='array' && !node.value){node.fld.hidden = true;}");
        }
        this.nodeTree.enumerate("node.cntChildren0=node.children ? node.children.length :0;");
        this.nodeTree.enumerate("node.warnings=node.warnings ? parseInt(node.warnings) : 0 ;");

        this.nodeTree.enumerate(function(params, node) {
            if(!node.fld.order || parseFloat(node.fld.order).toString()=='NaN') {
                // sort undefined orders at the beginning
                node.fld.order = +1000000;
                //alert('find a NaN in order');
            }
            //else alerJ('node.fld',node.fld);
        });
        this.nodeTree.enumerate(function(params, node)
        {
            function sorter(a, b, a2, b2, a3, b3) {
                // compare a and b, falling back to comparing a2 and b2 if a and b equal
                return a > b ? 1 : a < b ? -1 : a2 > b2 ? 1 : a2 < b2 ? -1 : a3 > b3 ? 1 : a3 < b3 ? -1 : 0;
            }

            if (node.fld.children && node.fld.children.length)
                node.fld.children = node.fld.children.sort(function (a, b) {
                    return sorter(parseFloat(a.order), parseFloat(b.order), a.title?a.title:a.name, b.title?b.title:b.name, 0, 0);
                });
            if (node.children && node.children.length)
                node.children = node.children.sort(function (a, b) {
                    return sorter(parseFloat(a.fld.order), parseFloat(b.fld.order), a.fld.title?a.fld.title:a.fld.name, b.fld.title?b.fld.title:b.fld.name, a.row, b.row);
                });
            node;
        });


        this.nodeTree.enumerate(function(params, node) {
            if(node.fld.type =='password') {
                 if((!node.value0 || node.value0.length==0) && node.fld.isCopied){
                     for (var ic = 0 ; ic < node.parent.children.length ; ++ic ) {
                         var c_node = node.parent.children[ic];
                         if(c_node.fld.subpath == "new") {
                             c_node.fld.is_hidden_fg = 1;
                             c_node.fld.hidden = 1;
                             c_node.fld.is_optional_fg = 1;
                         }
                         if(c_node.fld.subpath == "confirm" ) {
                             c_node.fld.passwordVerifiableNode = node.fld;
                             node.fld.passwordConfirmingNode = c_node.fld;
                         }
                         node.fld.title = node.fld.title.replace(/^Old /,"");
                     }
                 }
            }
        },{ document: this.formProviderDoc, obj: this });

        this.nodeTree.enumerate( "if(node.depth==0 || (params.autoexpand && ( params.autoexpand=='all' || node.depth<=params.autoexpand) && node.children && node.children.length>0))node.expanded=params.expansion;else node.expanded=0;" , {autoexpand:this.autoexpand, expansion: this.expansion} ) ;
        this.vjDS.add("Retrieving Objects Metadata Information","ds"+this.container,"static://");
        this.redraw();
    };


    this.constructPopUpViewer = function(tabname, popupType){
        if(!popupType)
            popupType = "basic";

        if(popupType == "basic"){
            this.popObjectPanel = new vjPanelView({
                data:[ "dsVoid", "ds"+this.container],
                formObject: document.forms["form-floatingDiv"],
                //callbackRendered: ala,// "javascript:alerJ('a',this)",
                iconSize:24,
                //geometry:{width:'30%' },
                rows: [
                    //{name:'refresh', title: 'Refresh' , icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS.ds"+tagWord+".state=\"\";vjDS.ds"+tagWord+".load();"},
                    {name:'pager', icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                    {name: 'search', align: 'right', type: ' search', isSubmitable: true, title: 'Search', description: 'search sequences by ID', url: "?cmd=objFile&ids=$(ids)" }
                ],
                //hidden:true,
                isok:true });
            this.defaultOutlineShow = [
                   {name:new RegExp(/./), hidden:true },
                   {name: '_brief', hidden: false, title: 'Summary' },
                   {name: 'description', hidden: false, title: 'Description' },
                   {name: 'created', hidden: false, title: 'Created', type: 'datetime'},
                   { name: '^id$', hidden: false, title: 'Identifier' },
                   {name: 'name', hidden: false, title: 'Name' },
                   {name: 'icon', hidden: false, type:'icon', title: 'Icon' },
                   {name: new RegExp(/Title.*/i), hidden: false, title: 'Title' }];

            this.popObjectViewer=new vjTableView( {
                    data: "ds"+this.container,
                    formObject: document.forms["form-floatingDiv"],
                    bgColors:['#F0F3F9','#ffffff'] ,
                    cols: this.defaultOutlineShow,
                    checkable:false,
                    maxTxtLen:this.popUpTableText? this.popUpTableText : 64,
                    //objectsDependOnMe:[ dv+'.select.0' ],
                    selectCallback: "function:vjObjFunc('onSelectPopupList','" + this.objCls + "')",
                    checkCallback: "function:vjObjFunc('onCheckPopupList','" + this.objCls + "')",
                    defaultIcon:'rec',
                    geometry:{ width:'96%',height:'100%'},
                    iconSize:0,
                    isok:true });

            this.popupViewer = this.myFloaterName+"Viewer";
            // this.vjDV.add(this.popupViewer, this.popObjectViewer.geometry.height, this.popObjectViewer.geometry.width);
             this.vjDV.add(this.popupViewer, (this.popUpViewerWidth && !isNaN(this.popUpViewerWidth))? parseInt(this.popUpViewerWidth):600,300);
             this.vjDV[this.popupViewer].add("select","table","tab",[ this.popObjectPanel, this.popObjectViewer ]);//.viewtoggles=1;
             this.vjDV[this.popupViewer].render();
             this.vjDV[this.popupViewer].load();
        }
        if(popupType == "explorer" && !this.popNExplorer){
            if(!tabname) tabname = "All";
            if(this.popObjectExplorerViewer){
                this.popObjectExplorerViewer.destroyDS();
                delete this.popObjectExplorerViewer; 
            }
            if(this.popObjectViewer)delete this.popObjectViewer; 
             this.popObjectExplorerViewer=new vjExplorerBaseView({
                 container:this.myFloaterName+"Viewer",
                 formFolders:"form-floatingDiv",
                 formTables:"form-floatingDiv",
                 formSubmit:"form-floatingDiv",
                 preselectedFolder:"/All",
                 formPreviews:"form-floatingDiv",
                 isNShowactions:true,
                 subTablesAttrs : [{    tabname : tabname,
                                       tabico : "folder-apps",
                                       url : { type : "-" , prop:"id,_brief,created" }
                                   }],
                 folders_DV_attributes : {
                     width : 200,
                     height : 400,
                     hideDV:true
                 },
                 tables_DV_attributes : {
                     width : 700,
                     height : 400,
                     maxtabs : 8,
                     isok:true
                 },
                 submit_DV_attributes : {
                     width : "100%",
                     height : 60,
                     frame : "notab",
                     isok:true
                 },
                 preselectedFolder:"/all",
                 isSubmitMode:true,
                 autoexpand:0,
                 isNdisplay_Previews:true,

                 onSubmitObjsCallback: "function:vjObjFunc('onGetPopupList','" + this.objCls + "')",
                 drag:false,
                 isok:true
             });
             this.popObjectViewer=this.popObjectExplorerViewer;
             var explorer=this.popObjectViewer;
             explorer.init();
             explorer.render();
             explorer.load();
        }
    };






    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ renderer function, can be called when needs to be redrawn
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    this.redraw=function( elements ,ifFromChange)
    {
        if (!elements) elements = this.nodeTree.root;
        if(!ifFromChange)    ifFromChange = false;
        this.validate(elements);
        //this.constructElementViewers(elements, false);
        this.fldTree.enumerate( "node.innerTxt='';" ) ;  //this.enumerateNodesTreeView( this.fldTree.root, "node.innerTxt=''");
        this.nodeTree.enumerate("node.onlyOneTitleForMultiArray='';");


        this.allHiddenControlsText = "";
        var t="";
        if (this.prefixHTML) t += this.prefixHTML;
        if (this.implementTreeButton) {
            t += "<table align=topleft class='REC_table'>";

            t += "<tr><td><img onClick='vjObjEvent(\"showAllFunction\",\"" + this.objCls + "\")' src=" + (this.showReadonlyInNonReadonlyMode ? 'img/off_icon.gif' : 'img/on_icon.gif') + " border=0 width=20 height=20 title='show/hide readonly field'/></td>";
            t += "<td><img onClick='vjObjEvent(\"expandAllFunction\",\"" + this.objCls + "\",\"\")' src=" + (!this.nodeTree.root.expanded ? 'img/recExpand.gif' : 'img/recCollapse.gif') + " border=0 width=20 height=20 title='expand/collapse all field'/></td>";
            t += " <td><img onClick='vjObjEvent(\"constructAllFunction\",\"" + this.objCls + "\")'" + (this.constructAllField ? 'class=sectHid' : 'src="img/recAdd.gif" title="show all editable Field show"') + "  border=0 width=16 height=16 /></td></tr>";
            // t += "<tr><td>" + (this.constructAllField ? '' : 'show all editable Field show') + "</td><td>" + (this.showReadonlyInNonReadonlyMode ? 'Hide Read Only Field' : 'Show Read Only Field') + "</td><td>" + (!this.nodeTree.root.expanded ? 'Expand' : 'Collapse') + " all</td></tr></table>"
            t += "<tr><td>" + (this.showReadonlyInNonReadonlyMode ? 'Hide' : 'Show') + "</td><td>" + (!this.nodeTree.root.expanded ? 'Expand' : 'Collapse') + "</td><td border=0>" + (this.constructAllField ? '' : 'Construct') + "</td></tr></table>";
        }

        t += this.generateText(elements, this.showRoot ? false : true);
        //if (this.implementSaveButton) t += "<input type='button' name='save_record+" + this.container + "' value=" + ((this.hiveId == this.objType)? "CREATE_RECORD":"SAVE_RECORD") + " onclick='vjObjEvent(\"onSetVerification\",\"" + this.objCls + "\",\"/\")' />";
        if (this.implementSaveButton) t += "<input type='button' name='save_record+" + this.container + "' value=" + ((parseHiveId(this.hiveId).objId>0 )? "SAVE" : "CREATE") + " onclick='vjObjEvent(\"onSetVerification\",\"" + this.objCls + "\",\"/\",\"save\")' />";

        if (this.implementCopyButton && parseHiveId(this.hiveId).objId>0 ) t += "<input type='button' name='copy_record+" + this.container + "' value='COPY' onclick='vjObjEvent(\"onSetVerification\",\"" + this.objCls + "\",\"/\",\"copy\")' />";
        if (this.noFileErrorText) t += "<span style='color:red' id="+this.RVtag+"_noFileErrorText></span>";
        t += "<span class='sectHid'>" + this.allHiddenControlsText + "</span>";
        if(this.appendHTML)t+=this.appendHTML;
        this.div.innerHTML=t;

        this.setLayoutFields(elements.fld);
        this.constructElementViewers(elements, ifFromChange);

    };

    this.setLayoutFields=function ( fld )
    {
        if(fld.div){
            fld.div.innerHTML=fld.innerTxt;
        }
        if(fld.dsSource){
            fld.dsSource.reload("static://<span id='"+this.RVtag+"-"+fld.name+"' >"+fld.innerTxt+"</span>",true);
        }


        for ( var ic=0; ic<fld.children.length ; ++ic) {
            this.setLayoutFields(fld.children[ic]);
        }
    };


    this.constructElementViewers=function ( element ,ifFromChange)
    {


       // if (element.viewerAssociated && element.fld.constraint!='search+' && element.fld.constraint != "choice+")  {
        if(element.fld.type =='file'){
            var o=gObject(this.RVtag+"-"+element.name+"-input");
            if(o && element.inputNode){
                o.parentNode.replaceChild(element.inputNode,o);
            }
        }

        if (element.viewerAssociated && element.fld.constraint!='search+' && element.fld.constraint != "choice+")  {
            var newInlineList = new Array();
            var hideRowList=null;
            var childrenVals= new Array();
            var par=element.parent;
            for ( var ie=0; ie<par.children.length; ++ie) {
                if (par.children[ie].fld.name==element.fld.name) {
                    if (par.children[ie].value instanceof Array) {
                        childrenVals = childrenVals.concat(par.children[ie].value);
                    } else {
                        childrenVals.push(par.children[ie].value);
                    }
                }
            }
            if(element.fld.doNotCollapseMultiValueViewers || !isok(childrenVals)) {
                childrenVals=verarr(element.value);
            }

            var haveChildrenIdVals = false;
            for (var ic=0; ic<childrenVals.length; ic++) {
                var hiveId = parseHiveId(childrenVals[ic]);
                if (hiveId.domainId || hiveId.objId || hiveId.ionId) {
                    haveChildrenIdVals = true;
                    break;
                }
            }

            var url;
            if (element.fld.constraint == 'type') {

                //url = element.value ? "http://?cmd=propget&prop=id,brief,name,description,title&mode=csv&ids=" + childrenVals : "static://";
                url = haveChildrenIdVals ? "http://?cmd=propget&prop=id,_brief,&mode=csv&ids=" + childrenVals : "static://";
            }
            else if (element.fld.constraint == 'search') {
                if (childrenVals.length) {
                    // constraint_data may have substitutable substrings and need jsonification
                    var constraint_data = this.computeConstraintData(element);
                    if ( (constraint_data.qryLang || (constraint_data.url.indexOf('cmd=') != -1) ) && (constraint_data.url.indexOf('taxTree') == -1) && (constraint_data.url.indexOf('cmd=usrList') == -1)) {
                        var inlineProps = "_brief,name,description,title";
                        if (constraint_data.inline instanceof Array) {
                            var inlinePropArray = [];
                            for (var c in constraint_data.inline)
                                inlinePropArray.push(constraint_data.inline[c].name);
                            inlineProps = inlinePropArray.join(",");
                        } else if (constraint_data.inline)
                            inlineProps = constraint_data.inline;
                        if(constraint_data.fetch=="id" && haveChildrenIdVals) {
                            url = constraint_data.inline_url ? constraint_data.inline_url : "http://?cmd=propget&mode=csv";
                            url = urlExchangeParameter(url, "ids", encodeURIComponent(childrenVals));
                            url = urlExchangeParameter(url, "prop", encodeURIComponent(constraint_data.fetch + "," + inlineProps));
                        }
                        //else url = "http://?cmd=propget&mode=csv&"+constraint_data.fetch+"=" + childrenVals+"&prop=" + constraint_data.fetch + "," + inlineProps;
                        //else if(element.fld.compleURLUsed) {
                        //        url = urlExchangeParameter(element.fld.compleURLUsed,"prop");//;"http://?cmd=objList&mode=csv&prop_val=" + childrenVals+"&prop_name=" + constraint_data.fetch + "&prop=" + inlineProps;
                        //        alert("sshe " + url);
                        //}
                        //else
                    }
                    if(!url){
                    //else {
                        url = "" + constraint_data.url;
                        hideRowList = new Object();
                        hideRowList[constraint_data.fetch] = new RegExp("^("+sanitizeStringRE(childrenVals).split(",").join("|")+")$");
                        //alerJ("we are going to hide", hideRowList);
                    }
                    if (constraint_data.inline) {
                        newInlineList.push({ name: new RegExp(/.*/), hidden: true });
                        //newInlineList.push({ name: 'icon', hidden: false, type:'icon' });
                        if (constraint_data.inline instanceof Array)
                            newInlineList = newInlineList.concat(constraint_data.inline);
                        else {
                            var inline = constraint_data.inline.split(',');
                            for (var i = 0; i < inline.length; i++)
                                newInlineList.push({ name: inline[i], hidden: false, title: inline[i] });
                        }
                        //alert(newInlineList)
                    }

                }
                else url = "static://";

            }
            else if (element.fld.constraint == "choice" ) {
                var url;
                var whatToShow = element.choiceOption.split("\n");
                var ic = 0;
                for (; ic < whatToShow.length; ic++) {
                    if (whatToShow[ic].split(",")[1] == element.value) break;
                }

                //if(ic>=whatToShow.length)
                url = "static://" + whatToShow[0].concat("\n", (ic>=whatToShow.length)?element.value :whatToShow[ic]);
                if (element.defaltValueShow && !ifFromChange) url = "static://" + element.defaltValueShow;

            }

            this.dataSourceEngine.add("infrastructure: Constructing Object Lists in RecrodViewer", 'ds' + element.name, url);
            this.dataViewEngine.add(element.viewerAssociated, 500, 500).frame = 'none';
            // alert(whatTheSpanShow.title)
            this.defaultInlineShow = [{ name: new RegExp(/./), hidden: true }, { name: '_brief', hidden: false, title: 'Description' },{name:'icon', type: 'icon'},{ name: 'description', hidden: false, title: 'Description' },{ name: 'brief', hidden: false, title: 'Summary' }, { name: new RegExp(/Title.*/i), hidden: false, title: 'Title' }];
           // this.defaultInlineShow = [{ name: '_brief', hidden: false, title: 'Description' },{name:'icon', type: 'icon'}];
            var myListViewer=new vjTableView( {
                data: "ds"+element.name,
                formObject: this.formObject,
                bgColors:['#F0F3F9','#F0F3F9'] ,
               // cols: this.defaultOutlineShow,
                isNheader: true,
                defaultEmptyText:" ",
                cols: newInlineList.length?newInlineList:this.defaultInlineShow,
                inclusionObjRegex: hideRowList ,
                selectCallback: element.fld.selectCallback,//"function:vjObjFunc('onSelectPopupList','" + this.objCls + "')",
                checkCallback: element.fld.checkCallback,// "function:vjObjFunc('onCheckPopupList','" + this.objCls + "')",
                maxTxtLen:64,
                multiSelect:true,
                geometry:{ width:'100%',height:'100%'},
                doNotShowRefreshIcon: true,
                //iconSize: (element.fld.constraint == 'search')?20:0,
                iconSize: 0,
                defaultIcon:'rec',

                isok: true
            });

            this.dataViewEngine[element.viewerAssociated].add("details", "table", "tab", [myListViewer]);
            this.dataViewEngine[element.viewerAssociated].render();

            // On common events like tab switch, we do not want to reload viewer data sources if we already have the data
            // (to avoid unnecessary ajax requests), but we do want to refresh the element's table view (because the view's
            // div may have been invalidated).
            if (this.dataSourceEngine['ds' + element.name].hasDataForUrl()) {
                this.dataSourceEngine['ds' + element.name].call_refresh_callbacks();
            } else {
                this.dataSourceEngine['ds' + element.name].load();
            }

            if(element.fld.constraint == 'type' && (element.fld.constraint_data=='image' || element.fld.constraint_data=='system-image') && haveChildrenIdVals){
                var o=gObject(this.RVtag+"-"+element.name+"-imageControl");
                if (o ) {

                    var t = " "
                    for(var vi = 0;vi<childrenVals.length;vi++){
                        t+="<img src = '?cmd=objFile&ids="+ childrenVals[vi] + "&filename=_.png"+ "' "+element.myTags+" >" ;
                    }
                    o.innerHTML=t;
                    //alerJ('a',o.innerHTML);
                }
            }
        }
        for (var ie=0;  ie<element.children.length; ++ie)
        {
            this.constructElementViewers(element.children[ie],ifFromChange);
        }

    };


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Record Viewers Data Model construction functions
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    this.createElement=function( fld, element)
    {

        var newel=new Object( {value: '' , value0:'', fld: fld, row: element.subRow, subRow: 0, obj: element.obj, expanded:this.expansion, children: new Array (), depth: element.depth+1, distance: 1 } );
        newel.parent=element;
        newel.name=this.elementName(newel);
        newel.path=element.path+newel.name+"/"; 

        //fld.default_value="eval:timeNow();";

        if(fld.default_value && typeof(fld.default_value) === "string" && fld.default_value.indexOf("eval:")==0){
            fld.default_value=eval(fld.default_value.substring(5));
        }
        newel.value0=fld["default_value"];
        newel.value=newel.value0;
        if(fld.name==this.spcTypeObj){
            newel.value=this.objType;
            newel.value0=newel.value;
            newel.hiveId=this.hiveId;
        }

        return newel;

    };
    this.constructAllFunction = function ( ) {
        this.constructionPropagateDown = 200;
            this.constructInfrastructure(this.nodeTree.root, 0, true, this.constructionPropagateDown);
            this.constructAllField = true;
        this.redraw();
    };
    this.showAllFunction = function () {
        if (this.showReadonlyInNonReadonlyMode) {
            this.showReadonlyInNonReadonlyMode = false;
            this.fldTree.enumerate("if(node.is_readonly_fg){node.hidden = true}");
            this.redraw();
        }
        else {
            this.showReadonlyInNonReadonlyMode = true;
            this.fldTree.enumerate("if(node.is_readonly_fg){node.type=''}");
            this.redraw();
        }
    };

    this.expandAllFunction = function (container,element,epxandOrClose) {
        if (!element) element = this.nodeTree.root;
        //alerJ("element", element)
        if (element == this.nodeTree.root) {
            if (!epxandOrClose)
                epxandOrClose = element.expanded ? 0 : 1;
        }
//        var elname = element.fld.name;
        element.expanded = epxandOrClose;//element.expanded ? 0 : this.expansion;
        //element.expanded = element.expanded ? 0 : this.expansion;

        for (var ic = 0; ic < element.children.length; ic++) this.expandAllFunction(container, element.children[ic], epxandOrClose);
        if (element == this.nodeTree.root) this.redraw();
    };

    this.constructInfrastructure=function(element, kind , onlyifnochildren, propagateDown)
    {
        //element is parent node
        //fld is parent fld

        var fld=element.fld;
//        var rowToAdd=element.row;
        var newel;
        var this_ = this;

        // create the cells
        var array_rows = {}; // map array row -> indices of element.children for that row
        if (fld.type == "array" && element.children.length) {
            element.children.forEach(function(c, ic) {
                var row = +c.row;
                if (array_rows[row]) {
                    array_rows[row].push(ic);
                } else {
                    array_rows[row] = [ic];
                }
            });
        } else {
            array_rows[1] = element.children.map(function(c, ic) { return ic; });
        }

        Object.keys(array_rows).forEach(function(row) {
            var row_ichildren = array_rows[row];

            for (var il=0; il<fld.children.length; ++il ) {

                var fls=fld.children[il],docreate=true;

                if(kind && fls.name!=kind  ) continue;//docreate=false;



                if(docreate){
                    newel = undefined;
                    if (onlyifnochildren) { // find a child of this kind
                        for (var ie=0; ie<row_ichildren.length; ++ie) {
                            var child = element.children[row_ichildren[ie]];
                            if (child.fld.name == fls.name) {
                                newel = child;
                                break;
                            }
                        }
                    }
                    if (!newel) {
                        // check if it's legal for fls to have multiple elements
                        var fls_is_global_multi = ({
                            recurse: function(f) {
                                if (f) {
                                    if (f.is_multi_fg || (f.parent && f.parent.type == "array")) {
                                        return true;
                                    } else {
                                        return this.recurse(f.parent);
                                    }
                                } else {
                                    return false;
                                }
                            }
                        }).recurse(fls);

                        // for globally single-valued fields that already have an element with the wrong
                        // or different parent, don't create any more elements (this issue can arise with
                        // a corrupt tree structure when some nodes with invalid paths were added to an
                        // object). And don't construct into them either (that will be taken care of by the
                        // parent of the already created element).
                        if (!fls_is_global_multi && this_.accumulate("node.fld.name=='" + (fls.name) + "'", "node").length) {
                            continue;
                        }

                        newel=this_.createElement( fls, element) ;
                        if(!newel)continue;
                        element.children.push(newel);
                    }

                    if(propagateDown>1 || fls.type=="array")
                        this_.constructInfrastructure ( newel , 0, onlyifnochildren, fld.type=="array" ? propagateDown : propagateDown-1);
                }
                if(fld.type!="array")
                    ++element.subRow;
            }
        });
        if(fld.type=="array")
            ++element.subRow;

        return newel;
    };

    this.populateInfrastructure=function( element, valarr, obj )
    {

        for( var iv=0; iv< valarr.rows.length ; ++iv) {
            var row=valarr.rows[iv];
           // alerJ("rw",row)
            var fld=this.fldTree.findByName( row.name );

            if(!fld){
                if(row.name.charAt(0)=="_") { // accumulate special objects
                    if(!this[row.name.substring(1)])this[row.name.substring(1)]=new Object();
                    this[row.name.substring(1)][row.value]=1;
                }
                //alerJ('r',this[row.name.substring(1)]);
                continue; // field with this name must exist
            }

            var el=this.nodeTree.findByName(row.name );
            if(!el) { // element does not exist .. need to be created

                // remember the path to the top parent
                var parList=new Array();
                for ( var curT=fld; curT.parent && (curT!=element.fld); curT=curT.parent ){
                    parList.push(curT);
                }
                var parentelem=element;
                var rowlist="";

                // now scan and create all elements from the root if those do not exist
                for (var ip = parList.length - 1; ip >= 0; --ip) {
                    if (row.path[row.path.length - ip - 1] == "NaN") row.path[row.path.length - ip - 1] = '0';
                    var ir=(ip<row.path.length)? row.path[row.path.length-ip-1] : "0";
                    parentelem.subRow=parseInt(ir);
                    rowlist = rowlist + "." + ir;

                    var elname="prop."+this.idForProp(obj)+"."+parList[ip].name+rowlist;
                    //alert(elname+"   rowlist"+rowlist)
                    el=this.nodeTree.findByName(elname,parentelem);
                    if(el){
                        parentelem=el;
                        continue;
                    }

                    if( parentelem.fld.type=="array") {// for arrays we always add all elements at once
                        for( var il=0; il<parentelem.fld.children.length; ++il) {
                            var thisel=this.createElement( parentelem.fld.children[il], parentelem) ;
                            if(!thisel)continue;
                            if(ip>0) {thisel.value0='';thisel.value=thisel.value0;}
                            parentelem.children.push(thisel);
                        }
                        el=this.nodeTree.findByName(elname,parentelem);
                    }
                    else { // other elements
                        el=this.createElement( parList[ip], parentelem) ;
                        if(!el)break;
                        //if(el){
                            if(ip>0) {el.value0='';el.value=el.value0;}
                            parentelem.children.push(el);
                        //}
                    }

                    parentelem=el;
                }

            }
            if(!el){
                continue;
            }
            //f(el){
                el.value0=valarr.rows[iv].value;
                el.value=el.value0;
            //}
            if( el.parent ) {
                if(el.parent && row.path.length && el.parent.subRow<=row.path[row.path.length-1])
                    el.parent.subRow=parseInt(row.path[row.path.length-1])+1;
            }

        }

        return element;
    };



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Miscelaneous data information and mainuplation functions
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    // When printing prop format identifiers for a new object, we want to use
    // the object type instead of "0" (the object ID) - this is significant for
    // the backend's prop format parser
    this.idForProp=function(id)
    {
        return parseHiveId(id).objId ? id : this.propFormatId;
    };

    this.elementName=function(element)
    {
        if(element.fld.name==this.spcTypeObj)
            return "prop."+this.idForProp(element.hiveId)+"._type";
            //return "type."+this.idForProp(element.hiveId);

        var t=new Array();
        for (var cur=element; cur && cur.parent!=cur && cur.parent; cur=cur.parent)  {
            t.push(cur.row);
        }

        return "prop."+this.idForProp(element.obj)+"."+element.fld.name+"."+t.reverse().join(".");
    };

    this.fieldIsReadonly = function(fld)
    {
        if (fld.is_readonly_fg) {
            return true;
        }
        for (var ifld=0; ifld<fld.children.length; ifld++) {
            if (this.fieldIsReadonly(fld.children[ifld])) {
                return true;
            }
        }
        return false;
    };

    this.elementIsRemovable = function(element)
    {
        // an element is considered removable if:
        // 1. its values get deleted using multiSelectDelete; or
        // 2. it is a multi-value element which has siblings with the same field
        // But readonly elements are never removable

        if (this.readonlyMode || this.fieldIsReadonly(element.fld)) {
            return false;
        }
        if (element.fld.constraint == "type" || element.fld.constraint == "search") {
            return true;
        }
        if (element.fld.is_multi_fg && element.parent) {
            for (var i=0; i<element.parent.children.length; i++) {
                if (element.parent.children[i] != element && element.parent.children[i].fld.name == element.fld.name) {
                    return true;
                }
            }
        }

        return false;
    };

    this.elementValueAccumulator = function(element, nodeProp)
    {
        if (nodeProp) {
            return "node[" + JSON.stringify(nodeProp) + "]";
        } else if (element.fld.constraint == "choice" || element.fld.constraint == "choice+") {
            return "node.value";
        } else if (element.fld.constraint == "search" || element.fld.constraint == "search+") {
            var constraint_data = this.computeConstraintData(element);
            return "node[" + JSON.stringify(constraint_data.fetch) + "]";
        }
        return "node.id";
    };

    this.elementsErrorText=function(elements)
    {
        if (!elements || (Array.isArray(elements) && !elements.length) ) {
            return "";
        }
        elements = verarr(elements);
        var t = "";
        t+="<span class='"+this.tblClass+"_error' title='";
        var errorTooltip;
        var errorText;
        var errors = 0;
        elements.forEach(function(element) {
            if (element.errors > 1) {
                errors += element.errors;
            }
            if (!errorTooltip && element.errorTooltip) {
                errorTooltip = element.errorTooltip;
            }
            if (!errorText && element.errorText) {
                errorText = element.errorText;
            }
        });

        t += sanitizeElementAttr(errorTooltip ? errorTooltip : "There is a mistake in the value of this element");
        t += "'>";
        t += this.icons.errorRow;
        if (errorText) t += errorText;
        if (errors>1) t+= "&nbsp;&times;&nbsp;" + errors;
        t+="</span>";
        return t;
    }

    this.elementStatusText=function(element)
    {

        var t="";

        if (element.modifications ) {

          //  alerJ("element",element.fld)
            if (!element.errors && this.implementSetStatusButton) {
               // alert(this.implementSetStatusButton)
                if (element.children.length) {
                    t += "<td>";
                    //alerJ("el",element)
                    t += "<a href='javascript:vjObjEvent(\"onSet\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\")' >" + this.icons.setRow + "</a>";
                    t += "</td>";
                }
            }

            t+="<td>";

            t+="<a href='javascript:vjObjEvent(\"onRevert\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")' >"+this.icons.revertRow+"</a>";
            t+="</td>";



        }

        if (!this.hideErrors && element.errors) {
            //alert(element.fld.title)
            t+="<td valign=middle border=1>";
            t += this.elementsErrorText([element]);
            t+="</td>";
        }

        if (t) {
            t = "<table><tr>" + t + "</tr></table>";
        }

        return t;
    };

    this.elementDescriptonText=function( element)
    {
        var fld=element.fld;
        var description="";
        if( isok(fld.description) )description=fld.description;

        if(!description.length )
            return "";

//        var elname=this.elementName(element);
        var t="";
        t+="<span class='"+this.tblClass+"_description'>";
        t+=this.icons.help;
        t+=description;
        t+="</span>";


        return t;
    };

    this.fieldDescriptionTitle=function(fld)
    {
        var ret = "";
        if(isok(fld.description)) {
            ret = " title='" + sanitizeElementAttr(fld.description) + "'";
        }
        return ret;
    };

    this.fieldAddTitleTitle=function(fld)
    {
        var ret = " title='add more rows'";
        if(isok(fld.title)) {
            ret = " title='Add another row for " + sanitizeElementAttr(fld.title) + "'";
        }
        return ret;
    }

    this.validate=function ( element, visualize, skip_optionality_validation)
    {
       // alerJ("call me one time",element);
        if (!element) element = this.nodeTree.root;

        if(!element.modifications ) element.modifications=0;
        if(!element.errors ) element.errors=0;

        if( element.value!=element.value0) element.modifications+=1;
        else element.modifications=0;

        if(element.modifications==0){
            var newchld=element.children ? element.children.length :0;
            if( element.cntChildren0!=newchld )element.modifications=1;
            else element.modifications=0;
        }


        element.errors=0;
        element.warnings = 0;
        for  ( var ie=0; ie<element.children.length; ++ie) {
            
            //if(skip_optionality_validation) { 
                this.validate(element.children[ie], visualize);
            //}

            if (element.children[ie].errors > 0 && element.children[ie].fld.type == "string") {
                element.errorTooltip = element.children[ie].errorTooltip;
                element.errorText = element.children[ie].errorText;
            }
            element.errors += element.children[ie].errors;
            element.warnings += element.children[ie].warnings;
            element.modifications+=element.children[ie].modifications;
        }

        if (!element.fld.is_hidden_fg&&!element.fld.is_readonly_fg && !isok(element.value) && !element.fld.is_optional_fg && (element.fld.type != "list") && (element.fld.type != "array")) {
             if (element.fld.is_multi_fg && element.fld.constraint=='type') {
                 var arrcheck=new Array();
                 var par=element.parent;
                 if(par){
                     for ( var ie=0; ie<par.children.length; ++ie) {

                         if(par.children[ie].fld.name==element.fld.name){
                             if(isok(par.children[ie].value))
                                 arrcheck=arrcheck.concat(par.children[ie].value.split(";"));
                         }
                     }
                 }
                 if(arrcheck.length==0){
                     ++element.errors;
                    element.errorTooltip = "Required field is empty";
                    element.warnings++;

                 }
             }
             else{
                ++element.errors;
                element.errorTooltip = "Required field is empty";
                element.warnings++;
             }
        }
        if (isok(element.value) && element.modifications && element.fld.constraint == "eval") {
            var values = [];
            var sep = this.getValidateSeparatorCb ? this.getValidateSeparatorCb(element.fld.name) : null;
            (sep ? element.value.split(sep) : [element.value]).forEach(function(value) {
                if (isok(value) && !eval(element.fld.constraint_data.replace(/\$_?\(val\)/g, value))) {
                    element.errors++;

                    element.errorText = (element.fld.constraint_description? element.fld.constraint_description:element.fld.constraint_data.replace(/\$_?\(val\)/g, element.fld.title));
                }
            });
        }

        if (isok(element.value)  && element.fld.type == "password") {
            if(element.fld.isCopy && element.modifications){
                var verifyNode = this.accumulate("(node.fld.name=='" + (element.fld.passwordVerifiableNode.name) + "') ", "node", null,element.parent)[0];
                if((verifyNode.value && element.value) && verifyNode.value!=element.value){
                    element.errors++;
                    element.errorText = "Passwords do not match";
                }
                else if (verifyNode.value && element.value && verifyNode.value==element.value){
                    //if they are the same, we allow to change the real password field
                    var oldNode = this.accumulate("(node.fld.name=='" + (element.fld.name) + "') && !node.isCopy", "node");
                    if(oldNode.length==1){
                        //if old one is hidden, which means we do not have value, what ever comes will be set
                        //if not, we need to verify if the old field has already been put right value
                        if(oldNode[0].hidden ||oldNode[0].overWrite ){
                            oldNode[0].value = element.value;
                            oldNode[0].modifications=1;
                        }
                    }
                }
            }
            else if (!element.hidden){
                //we are changing the old one
                if(element.value!=element.value0 && !element.overWrite){
                    element.errors++;
                    element.errorText = "Not match your old password.";
                }else{
                    element.overWrite = true;
                    //after we see there is a correct one, we do not let it change
                    var o=gObject(this.RVtag+"-"+element.name+"-input");
                    if(o ){
                        //alerJ('a',o);
                        o.readOnly = true;
                    }
                }
            }

        }



        if (isok(element.value) && element.modifications && element.fld.constraint == "regexp") {
            var myRegExp = new RegExp(element.fld.constraint_data);
            if (!eval(myRegExp.test(element.value))) {
                element.errors++;
                element.errorText = "plase give something within the constraint: " + (element.fld.constraint_description? element.fld.constraint_description:element.fld.constraint_data);

            }
        }
        

    /*    if (element.fld.constraint!="choice+"&&!element.fld.is_hidden_fg && !element.fld.is_readonly_fg &&  element.idList && element.idList.indexOf(element.value) == -1) {
            //alert(element.idList);
            element.errors++;
            element.errorText = "plase give valid id";
       }*/
        element.fld.modifications=element.modifications;
        element.fld.errors=element.errors;

        if(visualize) {
            var elname=this.elementName(element);

            var o=gObject(elname+"-status");
            var v=gObject(this.RVtag+"-"+element.fld.name+"-status");
            var g=gObject(this.RVtag+"-status");
            if (o || v || g) {
                //alert("I will call function")
                t = this.elementStatusText(element);
                if(o)o.innerHTML=t;
                if(v)v.innerHTML=t;
                if(g)g.innerHTML=t;
            }

        }
        //this.redraw(element)
    };

    this.accumulateValues=function(element, propagatedown, separator, togetherWithNonModified, withoutHidden, cmdLineStyle, forSubmission)
    {
        var t="";

        if(element) {
            var doshow=true;

            if( !togetherWithNonModified && element.value==element.value0 )
                doshow=false;
            if( withoutHidden && ( element.fld.is_hidden_fg==true || element.fld.name.charAt(0) == "_") )
                doshow=false;
            if( !element.value || element.value.length==0)
                doshow = false;
            if (!element.value && element.modifications>0 && element.fld.type!='list' && element.fld.type!='array')
                doshow = true;
            if (forSubmission && !element.fld.is_submittable)
                doshow = false;

            //if(!doshow)alerJ(element.fld.name +" " + doshow ,element)
            if( doshow) {

                if(cmdLineStyle=="CGI") {
                    //t+="-"+element.fld.name+"="+element.value+"\"";
                    t+=element.fld.name+"="+encodeURIComponent(element.value);
                } else if(cmdLineStyle=="no_prop") {
                    t+=this.elementName(element).replace(/prop\.[0-9]\./,"").replace(/\./g,"_")+"="+encodeURIComponent(element.value);
                }
                else if(cmdLineStyle) {
                    t+="-"+element.fld.name+" \""+element.value+"\"";
                }
                else {
                    t+=this.elementName(element)+"="+encodeURIComponent(element.value);
                }
            }
        }
        else element=this.nodeTree.root;
        if(!propagatedown)return ;
        for( var ie=0; ie<element.children.length; ++ie) {
            var r=this.accumulateValues(element.children[ie], propagatedown,separator,togetherWithNonModified,withoutHidden ,cmdLineStyle,forSubmission);
            if(r.length && t.length) t+=separator;
            t+=r;
        }
        //alert(t)
        return t;
    };

    this.changeElementValue=function (fldName , eleVal , rownum , dovalidate , doTriggerOnChange, forceConstruct)
    {
        //if want to set up multiple levels in the viewer
        if ((eleVal instanceof Object) && !(eleVal instanceof Array)){
            if (eleVal.name){
                var arr = this.accumulate( "node.fld.name=='" + (eleVal.name) + "'", "node");
                if(isok(arr))
                    this.changeElementValueByPath(arr[0].path, eleVal.value , rownum , dovalidate, doTriggerOnChange , forceConstruct );
                return ;
            }
            else if (eleVal.path){
                var arr = this.accumulate("node.name.indexOf('" + (eleVal.path) + "') >= 0", "node");
                if(isok(arr))
                    this.changeElementValueByPath(arr[0].path, eleVal.value , rownum , dovalidate, doTriggerOnChange , forceConstruct );
                return ;
            }
        }
        else{
            var arr = this.accumulate("node.fld.name=='" + (fldName) + "'", "node");
            if(isok(arr))
                this.changeElementValueByPath(arr[0].path, eleVal , rownum , dovalidate, doTriggerOnChange , forceConstruct );
            return ;
        }
        /*
        if(!rownum)rownum=0;
        var arr = this.accumulate("node.fld.name=='" + (fldName) + "'", "node");
        if(!arr || arr.length<rownum+1)return ;
        arr[rownum].value = eleVal;

        if ((arr[rownum].fld.type.indexOf("bool") == 0) && ((eleVal==1)||(eleVal == 'true')) ){
            this.formObject.elements[arr[rownum].name].checked = true;
        }
        else{
            this.formObject.elements[arr[rownum].name].value = eleVal;
        }
        //alerJ(fldName+"="+eleVal + " \n"+arr[rownum].fld.constraint ,arr[rownum]);
        if (forceConstruct || arr[rownum].fld.constraint.indexOf("choice")!=-1 ||(arr[rownum].fld.constraint == "type" || arr[rownum].fld.constraint.indexOf("search")!=-1)){
            //alerJ("OK "+fldName+"="+eleVal,arr[rownum]);
            this.constructElementViewers(arr[rownum],true);
            if (arr[rownum].length > 1) {
                this.constructInfrastructure(element, fld.children[0].name, false, 1);
            }
        }
        this.redraw(this.nodeTree.root,true);

        if(doTriggerOnChange && this.onChangeCallback)
            return funcLink(this.onChangeCallback, this, arr[rownum], this.formObject.elements[arr[rownum].name] );

        if (dovalidate) {
            this.validate(arr[rownum], true);
        }
        */
    };

    this.changeElementValueByPath=function (fldPath , eleVal , rownum , dovalidate, doTriggerOnChange, forceConstruct, donotredraw)
    {

        if(!rownum)rownum=0;
        var arr = this.accumulate("node.path=='" + (fldPath) + "'", "node");

        var element=this.nodeTree.findByPath(fldPath);
        if(element.elementValueArray)    element.elementValueArray = eleVal;
        //var element = this.accumulate("node.fld.path=='" + (fldPath) + "'", "node")[0];
        var par=element.parent;
        if( (eleVal instanceof Array) && (element.fld.is_multi_fg && ((element.fld.constraint=='search') || (element.fld.constraint=='type'))) ) {

            var nm=element.fld.name;
            var newarr= new Array();
            for ( var ie=0; ie<par.children.length; ++ie) {
                if(par.children[ie].fld.name!=nm || par.children[ie]==element)
                    newarr.push(par.children[ie]);
            }
            par.children=newarr;
            element.value='';

            for( var ie=0; ie<eleVal.length; ++ie) {
                if(ie)this.constructInfrastructure(par, nm, false, 0);
                var newchild= ie==0 ? element : par.children[par.children.length-1];
                newchild.value=eleVal[ie];
                newchild.hidden= (ie==0) ? false: true;
                //alerJ(ie,newchild)
            }


        }
        else  {
            if(!arr || arr.length<rownum+1)return ;
            if(arr[rownum].value == eleVal)return;

            if (arr[rownum].fld.type == "bool") {
                var wantCheck = false;
                if (eleVal instanceof Array) {
                    for (var ival=0; ival<eleVal.length; ival++) {
                        if (parseBool(eleVal[ival])) {
                            wantCheck = true;
                            eleVal[ival] = '1';
                        } else {
                            eleVal[ival] = '0';
                        }
                    }
                } else {
                    wantCheck = parseBool(eleVal);
                }
                arr[rownum].value = eleVal;
                this.formObject.elements[arr[rownum].name].checked = wantCheck;
            } else {
                arr[rownum].value = eleVal;
                this.formObject.elements[arr[rownum].name].value = eleVal;
            }

           // if (forceConstruct || arr[rownum].fld.constraint.indexOf("choice") != -1 || (arr[rownum].fld.constraint == "type" || arr[rownum].fld.constraint == "search")) {
                //this.constructElementViewers(arr[rownum], true);
               /* if (arr[rownum].length > 1) {
                    this.constructInfrastructure(element, fld.children[0].name, false, 1);
                }*/

            //}
        }

       // if (!donotredraw)
        //    this.redraw(this.nodeTree.root,true);

        this.constructElementViewers(element, true);
        if(this.autoSaveOnChange)
            this.saveValues(element,true);

        if (dovalidate) {
            this.validate(par, true);
            //this.validate(arr[rownum], true);
        }

        if(doTriggerOnChange && this.onChangeCallback)
            return funcLink(this.onChangeCallback, this, arr[rownum], this.formObject.elements[arr[rownum].name] );

        /*if (dovalidate) {
            if(arr[rownum].fld.is_multi_fg==1)
                this.validate(arr[rownum].parent, true);
            this.validate(arr[rownum], true);
        }*/
    };

    this.changeValueList=function(obj)
    {
        for ( fld in obj )  {
            var qryStr=obj[fld];

            if( typeof(qryStr)==="string" && qryStr.indexOf("eval:")==0)
                qryStr=this.computeExpression(qryStr.substr(5),"join");
            if(typeof(qryStr)==="string" &&  ( qryStr.indexOf("query:")==0 || qryStr.indexOf("ajax:")==0 ) ) {
                var url= (qryStr.indexOf("query:")==0) ? "objQry&raw=1&qry="+vjDS.escapeQueryLanguage(qryStr.substr(6)) : qryStr.substr(5)+"&raw=1" ; 
                linkCmd(url,{obj:this, fldName: fld, change: obj["change"] },
                    function (par, txt) {
                        var txtVal = txt;
                        if (txt && txt.length && ((txt[0] == '[' && txt[txt.length - 1] == ']') || (txt[0] == '{' && txt[txt.length - 1] == '}'))) {
                            // txt looks like a query language structure
                            txtVal = null;
                            try {
                                txtVal = JSON.parse(txt);
                            } catch(err) {}
                        }
                        if (txtVal != null) {
                            if(par.change=='constraint') { 
                                var elem= par.obj.accumulate("node.fld.name=='" + (par.fldName) + "'", "node")[0];
                                if(isok(elem)) {
                                    
                                    if(elem.fld.constraint.indexOf('choice')==0) {
                                        var a=txtVal.split("\n");
                                        var opt="";
                                        for( var i=0; i<a.length; ++i ) { 
                                            opt+=a[i]+","+a[i]+"\n";
                                        }
                                        txtVal=txtVal.replace(/\n/g,"|");
                                        elem.choiceOption="description,value\n"+opt;
                                    }
                                    elem.fld.constraint_data=txtVal;
                                    //par.obj.constructElementViewers( elem,true);
                                                                        
                                }
                            }
                            else 
                                par.obj.changeElementValue(par.fldName, txtVal, 0, true, true, true);
                        }
                    }
                );
            }
            else  {
                this.changeElementValue(fld,qryStr,0,true);
            }
        }
    };


    this.getElementRealName=function( fldName, which)
    {
        var arr = this.accumulate("node.fld.name=='" + (fldName) + "'", "node.name");
        if(!which)which=this.whichDefined;
        if(!which)return arr[0];
        else if(which=="join")return arr.join(",");
        else if(which=="array")return arr;
        else return arr[parseInt(which)];
    };

    this.getElement=function( fldName, which)
    {
        var arr = this.accumulate("node.fld.name=='" + (fldName) + "'", "node");
        if(!which)which=this.whichDefined;
        if(!which)return arr[0];
        else if(which=="join")return arr.join(",");
        else if(which=="array")return arr;
        else return arr[parseInt(which)];
    };

    this.computeEvalFields=function ( element , depth  )
    {
        if (!element) element = this.nodeTree.root;
        if(!depth)depth=0;

        if(isok(element.fld.eval)) {
            var this_ = this;
            var t = evalVars(element.fld.eval, this._evalKeys, ")", null, function(key, orig_key_expr) {
                return this_.getElementValue(key, "array");
            });

            if(t.indexOf("eval:")==0)t=eval(t);

            if( !isok(eval.value) || element.valueAuto ) {
                element.value=t;
                element.valueAuto=true;
            }
        }
        for  ( var ie=0; ie<element.children.length; ++ie) {
            this.computeEvalFields(element.children[ie],depth+1);
        }

        if(depth==0)
            this.redraw(element,true);
    };

    this.computeExpression=function(expr ,which )
    {
        var this_ = this;
        var t = evalVars(expr, this._evalKeys, ")", null, function(key, orig_key_expr) {
            return this_.getElementValue(key, which);
        });
        return t;
    };

    // substitute $(key) expressions which are located closest in the node tree to the specified element;
    // keys present in field tree but not in the node tree are replaced with empty string "";
    // keys that are not present in the field tree are left unchanged
    this.computeExpressionAtElement = function(expr, element)
    {
        var this_ = this;
        return evalVars(expr, this._evalKeys, ")", null, function(key, orig_key_expr) {
            var value = this_.fldTree.findByName(key) ? "" : orig_key_expr;
            var best_common_len = 0;

            this_.nodeTree.accumulate("node.fld.name=='"+key+"'", "node").forEach(function(value_element) {
                var len = Math.max(element.path.length, value_element.path.length);
                var common_len = 0;
                for ( ;common_len < len && element.path[common_len] == value_element.path[common_len]; common_len++);
                if (common_len > best_common_len) {
                    value = value_element.value;
                    best_common_len = common_len;
                }
            });

            return value;
        });
    };

    // retrieve constraint_data as object (for search/search+) or string (other constraints)
    // and then perform all appropriate $(key) expression substitutions
    this.computeConstraintData = function(element) {
        var constraint_data = "";
        var fld = element.fld;
        if (fld.constraint == "search" || fld.constraint == "search+") {
            var this_ = this;
            var constraint_data_raw = eval("new Object(" + fld.constraint_data + ")"); // TODO: switch to JSON.parse
            // recusively substitute object/array values
            constraint_data = function computeExpressionRecursive(o) {
                if (o instanceof Array) {
                    o.forEach(function(e, i, o) {
                        o[i] = computeExpressionRecursive(e);
                    });
                    return o;
                } else if (typeof(o) === "object") {
                    for (var k in o) {
                        if (o.hasOwnProperty(k)) {
                            o[k] = computeExpressionRecursive(o[k]);
                        }
                    }
                    return o;
                } else if (typeof(o) === "string") {
                    return this_.computeExpressionAtElement(o, element);
                } else {
                    return o;
                }
            }(constraint_data_raw);
        } else {
            constraint_data = this.computeExpressionAtElement(fld.constraint_data, element);
        }
        return constraint_data;
    };
/*
    this.retrieveFieldValue=function (fldName, url)
    {
        ajaxDynaRequestPage(url, {objCls: this.objCls, callback:'onRetrieveValueFromURLCallback', fldName: fldName  }, vjObjAjaxCallback, false );
    }
    
    this.onRetrieveValueFromURLCallback = function(container, text )
    {
        //alert(container +"\n"+text);
        this.changeElementValue(container.fldName,text,0,true);
    }
*/
    this.getElementValue=function( fldName, which)
    {

        var vals=this.accumulate( "node.fld.name=='"+fldName+"' && isok(node.value)","node.value" );

         //var el=this.accumulate( "node.fld.name=='"+fldName+"'","node" );
        //if(el.length && el[0].jointVal && vals.length==1 )
        //    vals=vals[0].split(";");

        if(!vals)return null;
        if(!which)which=this.whichDefined;

        //alerJ(fldName+"+"+vals+"=="+which,el[0])

        if(!which )return vals[0];
        else if(which=="join")return vals.join(",");
        else if(which=="array")return vals;
        else return vals[parseInt(which)];
        return vals[0];
    };


    this.attached=function(kind, name )
    {
        //alert(kind+"/."+name,"",this[kind])
        if(!name)return this[kind];
        else if(!this[kind])return null;
        return this[kind][name];

    };

    this.saveValues=function( element, useajax , callback,whattodo )
    {
        var separator=this.separator?this.separator:"&";
        this.setUrl = this.cmdPropSet + separator + "raw=1" + separator;

        ///this.setUrl+="type."+this.idForProp(this.hiveId)+"="+this.objType+separator;
        //this.setUrl+="prop."+this.idForProp(this.hiveId)+"._type="+this.objType+separator;
        this.enumerate("if(node.fld.type=='list') node.value='';if(node.isCopy)  node.value='';");
        var arr = this.accumulate("(node.fld.type=='password') && node.value && node.value.length", "node");
        if(arr.length && arr[0].fld.type=='password'){
            //not sure if the forObject.submit() submit varies by post
            //so set useajax true
            useajax = true;
            this.submitByPost= true;
        }
        this.setUrl += this.accumulateValues(element, true, separator, this.accumulateWithNonModified, this.accumulateWithoutHidden, this.submitFormat, true);

        var oldcb;
        if(callback){
            oldcb=this.setDoneCallback ;
            this.setDoneCallback = callback;
        }
        if( useajax ) {
            ajaxDynaRequestPage(this.setUrl, {objCls: this.objCls, callback:'onSetCallback',dowhat:whattodo}, vjObjAjaxCallback, this.submitByPost  ? true : false );
        }
        else this.formObject.submit();
        if(oldcb) {
            this.setDoneCallback =oldcb;
        }
        //alert(this.setUrl);
    };

    this.accumulate=function( checker, collector , params, node )
    {
        return this.nodeTree?this.nodeTree.accumulate( checker, collector , params, false,0, node ):0;
    };
    this.enumerate=function( operation, params, node )
    {
        if(!this.nodeTree)return ;
        return this.nodeTree.enumerate( operation, params , false, 0, node);
    };


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Event Handlers
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


    this.onRevert=function( container, path, donotredraw)
    {
        var element=this.nodeTree.findByPath(path);

        element.value=element.value0;
        for( var ie=0; ie<element.children.length; ++ie) {
            this.onRevert( container, element.children[ie].path, true);
        }
        if(!donotredraw)
            this.redraw();
    };

    this.onSetCallback=function(param, text)
    {
        //it's so confused.. actually it's for save and create button
        //and this part works for create logic....

        if(text.indexOf('error:')!=-1){
            var textMessage="";
            var firstN = text.indexOf('\n');
            if(firstN!=-1){
                textMessage = text.slice(0,firstN+1);
            }
            var tbl = new vjTable(text.slice(firstN+1), 0, 0,0,0,'=',0);
            for(var i=0;i<tbl.rows.length;i++){
                var typeName = tbl.rows[i].cols[0];
                var fldName = typeName.slice(typeName.lastIndexOf(".")+1);

                var arr = this.accumulate("node.fld.name=='" + (fldName) + "'", "node");

                if(arr && arr.length){
                    arr[0].errors++;
                    arr[0].errorText = tbl.rows[i].cols[1];
                    var elname=this.elementName(arr[0]);
                    var o=gObject(elname+"-status");
                    var v=gObject(this.RVtag+"-"+arr[0].fld.name+"-status");
                    var g=gObject(this.RVtag+"-status");
                    if (o || v || g) {
                        t = this.elementStatusText(arr[0]);
                        if(o)o.innerHTML=t;
                        if(v)v.innerHTML=t;
                        if(g)g.innerHTML=t;
                    }
                }
                else{
                     var o=gObject(this.RVtag+"_noFileErrorText");
                     if(o) o.innerHTML+=tbl.rows[i].cols[1]+" ";
                }
            }
        }

        //alert(text.indexOf('\n'))
        if (this.implementSaveButton || this.implementCopyButton) {
            var hiveId = text.slice(text.indexOf("_id=") + 4);
            if(parseHiveId(hiveId).objId > 0) this.ForSaveReload(hiveId);
            else if(param.dowhat && param.dowhat=='copy')
                this.ForSaveReload('-'+this.hiveId);
        }
        //alerJ(this.setDoneCallback + '====' + text,this);
        if(this.setDoneCallback)
            return funcLink(this.setDoneCallback, this, text );
        for(var iv=0; iv<this.data.length; ++iv) {
            this.getData(iv).reload(null,true);
        }
    };

    this.ForSaveReload = function (hiveId) {
        //if (!hiveId) return;
        //var newurl = urlExchangeParameter(this.vjDS[this.data].url, "ids", hiveId);
        //alert("asdf")
        newurl = "?cmd=record&ids=" + hiveId + "&types=" + this.objType;
        if(hiveId.charAt(0)!='-')alert("Record Created Successfully");
        else alert("Record Copied Successfully")
        window.location.href = newurl;
        //this.vjDS[this.data].reload(newurl,true);
    };

    this.onSet=function( container, path, whattodo)
    {
        var element=this.nodeTree.findByPath(path);

        //gObject(this.RVtag+"-SET").innerHTML=url;
        var docontinue=true;
        if(this.setCallback)
            docontinue = funcLink(this.setCallback);
        if(!docontinue)
            return;
        if (!this.noAutoSubmit) {
            //alerJ("auto", element)
            this.saveValues(element, true,null,whattodo);
        }
    };

    this.onSetVerification = function (container, path, whattodo) {
        //alerJ("d", this.nodeTree.root)
        if (((this.nodeTree.root.warnings == this.nodeTree.root.errors) && this.hiveId == this.objType) || ((this.hiveId != this.objType) && !this.nodeTree.root.errors)) this.onSet(container, path, whattodo);
        else if (this.nodeTree.root.errors) alert("Please give the value within constraint");
    };

    this.setFields=function(obj)
    {
        if(typeof obj == 'string')
            obj=eval(obj);
        for ( nm in obj ) {
            // var el=this.getElement( nm );
            var el = this.fldTree.findByName(nm);
            if ( !el )continue;

            for ( attr in obj[nm] ) {
                //if(!obj[nm][attr])continue;
                var val= obj[nm][attr];
                //if(!val)val="";
                el[attr]=obj[nm][attr];
                //alerJ(111+ "--"+nm +"."+attr+"="+val);
            }
        }
    };



    this.onClickExpand=function(container, path, event)
    {
        var element=this.nodeTree.findByPath(path);
        var elname=this.elementName(element);

        element.expanded=element.expanded ? 0 : this.expansion;
        var o=gObject(elname+"-children");if(!o)return;

        var  plsmin;
        if(o.className=="sectHid") {o.className=this.tblClass+"_table";plsmin=this.icons.collapse;}
        else {o.className="sectHid" ;plsmin=this.icons.expand;}
        o=gObject(elname+"-collapser");if(!o)return;

        o.innerHTML=plsmin;//"<img widht=24 height=24 src='img/rec"+plsmin+".gif'>";

    };

    this.onClickNode=function(container, path, event)
    {

        if(this.currentPopupElement){
            this.onClosepop();
        }

        var element = this.nodeTree.findByPath(path);

        var fld=element.fld;

        var url = "http://";

        var popupType = "basic";
        var that=this;

        // constraint_data may have substitutable substrings and need jsonification
        var constraint_data = this.computeConstraintData(element);

        if(fld.constraint == "type") {
            popupType = "explorer";
            if (this.eltOfPopObjectViewer != element ){
                this.eltOfPopObjectViewer = element;
                this.constructPopUpViewer(undefined, popupType);
            }
            else if(this.popObjectViewer && this.popObjectViewer.refresh)
            setTimeout(function(){that.popObjectViewer.refresh();},150);
        } else if (fld.type == "obj" && (fld.constraint == "search" || fld.constraint == "search+")) {
            popupType = constraint_data.explorer ? "explorer" : "basic";
            if (this.eltOfPopObjectViewer != element ) {
                this.eltOfPopObjectViewer = element;
                this.constructPopUpViewer(undefined, popupType);
            }
            else  if(this.popObjectViewer && this.popObjectViewer.refresh)
                setTimeout(function(){that.popObjectViewer.refresh();},150);
        } else {
            this.vjDS["ds" + this.container].reload("static:// ", true);
            if (this.eltOfPopObjectViewer != element ) {
                this.eltOfPopObjectViewer = element;
                this.constructPopUpViewer(undefined, popupType);
            }
            else  if(this.popObjectViewer && this.popObjectViewer.refresh)
                setTimeout(function(){that.popObjectViewer.refresh();},150);
        }


        var cur_panel = this.vjDV.locate(this.myFloaterName+"Viewer._active.0");
        var cur_table = this.vjDV.locate(this.myFloaterName+"Viewer._active.1");

        var gModalCallback = "vjObjEvent('onClosepop','" + this.objCls + "')";

        //undefined clickCount prevents closing of popUp
        var clickCount = fld.is_multi_fg  ? "-" : undefined;

        var tableDS = null;
        if (popupType == "explorer") {
            clickCount = 0;
            gModalCallback = "true;";//function _t(){gOnDocumentClickCallbacksStack[iv].clickCounter;return true;};_t(); ";

            cur_table = this.popObjectViewer.getActiveViewer();
            var selectedTAB = this.popObjectViewer.tables_DV.selected;
            if (fld.is_multi_fg) {
                cur_table.multiSelect = fld.doNotCollapseMultiValueViewers ? false : true;
            } else {
                cur_table.multiSelect = false;
            }
            tableDS = this.popObjectViewer.dsTables[selectedTAB];
        }

        if (fld.constraint == "type") {
            var turl = urlExchangeParameter(tableDS.url_tmplt, "type", encodeURIComponent(constraint_data));
            tableDS.url_tmplt= turl;
        } else if (fld.constraint == "search" || fld.constraint == "search+") {
            var url=constraint_data.url;
            if (constraint_data.qryLang && popupType == "explorer") {
                this.popObjectViewer.qryLang = true;
            }
            if (!url) {
                url = "static://";
            }
            if (tableDS) {
                tableDS.url_tmplt = url;
            }

            if (constraint_data.url.indexOf('taxTree') != -1) {
                cur_table.iconSize = 0;
            } else if (constraint_data.url.match(/cmd=(propspec|propget|propDel)/)) {
                var propsList = constraint_data.fetch ? [ constraint_data.fetch ] : [];
                if (constraint_data.outline instanceof Array) {
                    propsList = propsList.concat(constraint_data.outline);
                } else if (constraint_data.outline) {
                    propsList = propsList.concat(constraint_data.outline.split(","));
                } else {
                    propsList.push("_brief", "name", "description", "title");
                }
                url += (propsList.length ? "&prop=" + propsList.join(",") : "");
                cur_table.iconSize = 0;
            } else {
                cur_table.iconSize = 20;
            }

            if (constraint_data.outline) {
                newOutlineList = [{ name: new RegExp(/.*/), hidden: true }];
                if (constraint_data.outline instanceof Array)
                    newOutlineList = newOutlineList.concat(constraint_data.outline);
                else {
                    var outline = constraint_data.outline.split(',');
                    for (var i = 0; i < outline.length; i++)
                        newOutlineList.push({ name: outline[i], hidden: false, title: outline[i] });
                }
                cur_table.cols = newOutlineList;
            } else if (popupType == "basic" && this.defaultOutlineShow) {
                cur_table.cols = this.defaultOutlineShow;
            }
            //alert(url)
            if(cur_panel)cur_panel.hidden = false;
            //alert("befire"+element.value)

            var checkedValues = null;

            if (fld.is_multi_fg) {
                var par = element.parent;
                checkedValues = {};
                for (var ie=0; ie<par.children.length; ie++) {
                    if (par.children[ie].fld.name == element.fld.name) {
                        var checkedChildList = par.children[ie].value;
                        if (typeof(checkedChildList) == "string") {
                            checkedChildList = checkedChildList.split(";");
                        }
                        for (var ic=0; ic<checkedChildList.length; ic++) {
                            var val = checkedChildList[ic];
                            if (val != "") {
                                checkedValues[val] = 1;
                            }
                        }
                    }
                }
                cur_table.multiSelect = true;
            } else if (fld.constraint == "search+") {
                checkedValues = {};
                checkedValues[element.value] = 1;
                cur_table.multiSelect = true;
            } else {
                cur_table.multiSelect = false;
            }

            if (checkedValues) {
                var fetchColName = constraint_data.fetch ? constraint_data.fetch : "id";
                cur_table.precompute = function(myTableViewer, tbl, ir) {
                    var node = tbl.rows[ir];
                    if (checkedValues[node[fetchColName]]) {
                        node.selected = 1;
                    }
                };
            }

            this.vjDS["ds" + this.container].reload(url, true);
        }
        else if (fld.constraint == 'choice' || fld.constraint == 'choice+') {
//            if(this.whichToConstruct == 2){
//                this.whichToConstruct = 1;
//                this.constructPopUpViewer();
//            }
            cur_table.iconSize = 0;
            url = "static://" + element.choiceOption;
            if(cur_panel)cur_panel.hidden = true;
            cur_table.checkable = false;
            cur_table.multiSelect = false;
            cur_table.cols = this.defaultOutlineShow;
            this.vjDS["ds" + this.container].reload(url, true);
        }

        fld.compleURLUsed=url;

        this.currentPopupElement=element;

        var xx=(gMoX+1000>gPgW) ? gPgW-1000 : gMoX;
        gModalOpen(this.myFloaterName+"Div", gModalCallback , xx, gMoY, clickCount);
    };

    this.onSelectPopupList = function (viewer, node) {
        var whatToAccumulate = this.elementValueAccumulator(this.currentPopupElement);
        var list = viewer.accumulate("node.selected", whatToAccumulate);
        
        var allList = viewer.accumulate("1", whatToAccumulate).join(";");
        this.currentPopupElement.idList = allList;

        this.parsePopupList(list);
    };

    this.onGetPopupList = function (viewer,nodelist) {
        var whatToAccumulate = this.elementValueAccumulator(this.currentPopupElement);

        var element=this.nodeTree.findByPath(this.currentPopupElement.path);
        if(!element.nodelist)element.nodelist=new Object();
        var list=new Array();
        for (var i =0 ; i < nodelist.length ; ++i){
            element.nodelist[i]=nodelist[i];
            var node=nodelist[i];
            list.push(eval(whatToAccumulate));
        }
        this.currentPopupElement.idList = list.join(";");
        this.parsePopupList(list);
        this.onClosepop(true);
    };
    
    this.parsePopupList = function (list) {
        var element=this.nodeTree.findByPath(this.currentPopupElement.path);
        // var elementValue;
         if(element.fld.is_multi_fg){
             if(!element.elementValueArray){
                 element.elementValueArray = new Array();
                 var par=element.parent;
                 for ( var ie=0; ie<par.children.length; ++ie) {

                     if(par.children[ie].fld.name==element.fld.name && par.children[ie].value!=="" )
                         element.elementValueArray.push(par.children[ie].value);
                 }

             }
             for(var i=0;i<list.length;i++){
                 var j=0;
                 for(;j<element.elementValueArray.length;j++){
                     if(list[i]==element.elementValueArray[j])    break;
                 }
                 if(j==element.elementValueArray.length) element.elementValueArray.push(list[i]);
             }
         }

         var whatToPass = list;
         if (this.currentPopupElement.fld.is_multi_fg && this.currentPopupElement.fld.constraint == "type") {
             whatToPass = element.elementValueArray;
         } else if (!element.fld.is_multi_fg) {
             whatToPass = list.length ? list[0] : "";
         }

         this.changeElementValueByPath(this.currentPopupElement.path, whatToPass, 0, true, true);

         var res=0;
         if(this.currentPopupElement.fld.selectCallback){
             res=funcLink(this.currentPopupElement.fld.selectCallback, viewer,node , this);
         }
         if( !this.currentPopupElement.fld.is_multi_fg )
             {this.onClosepop(true);}
         return res;

    }

    this.onCheckPopupList=function(viewer,node)
    {

        return this.onCheckOutsideList(viewer, node, this.currentPopupElement);
    };


    this.onCheckOutsideList=function(viewer, node, element, nodeProp, donotredraw)
    {
        // alerJ("a", node);

        if (!element) element = this.currentPopupElement;

        var whatToAccumulate = this.elementValueAccumulator(element, nodeProp);

        if (!node) {
            viewer.enumerate(function(recordViewer, tbl, ir) {
                var node = tbl.rows[ir];
                if (!node)
                    return;
                recordViewer.onCheckOutsideList(viewer, node, element, nodeProp, true);
            }, this);
        }
        else {
            if (node.checked) {
                //this.onAddElement(this.objCls, element.path, element.children[0].fld.name);
                //alerJ("aa",element.fld)
                if (element.fld.children.length!=0){
                    this.constructInfrastructure(element, element.fld.children[0].name, false, 1);
                    element.children[element.children.length - 1].value = node[nodeProp];
                }//this.redraw();
            }
            else {
                var deleteRow;
                for (var i = 0; i < element.children.length; i++) {
                    if (element.children[i].value == node[nodeProp]) deleteRow = element.children[i].row;
                }
                this.onDelElement(this.objCls, element.path, deleteRow, 1, true, true);
                //this.redraw();
            }
        }

        var list = viewer.accumulate("node.checked", whatToAccumulate);

        var elementValue;
        elementValue = list;
        var allList = viewer.accumulate("1", whatToAccumulate).join(";");
        element.idList = allList;
        this.changeElementValueByPath(element.path, elementValue, 0, true, true, false, donotredraw);
        // use element, because this.currentPopupElement may be undefined when using outside viewers
        if(element.fld.checkCallback)
            return funcLink(element.fld.checkCallback, viewer, node, this);
    };

    this.onClosepop=function(force)
    {
        //var viewer=vjDV.locate(this.container+"_popViewer.select.0");
        if (gKeyCtrl == 0 || force) {
            gModalClose(this.myFloaterName+"Div");
            this.currentPopupElement = null;
        }
        return 1; //onClosepop is called from href and firefox will jump unless return is false
    };

    this.onAddElement=function(container,path, kind)
    {
        var element=this.nodeTree.findByPath(path);
        var newel = this.constructInfrastructure(element, kind, false, this.constructionPropagateDown);
        this.redraw();

        if (this.onAddElementCallback)
            return funcLink(this.onAddElementCallback, this, newel);
    };

    this.onAddArrayRow=function(container, path)
    {
        var element=this.nodeTree.findByPath(path);
        var fld = element.fld;
        if (fld.type != "array") {
            return;
        }

        element.subRow++;
        var newels = [];
        for(var il = 0; il < fld.children.length; il++) {
            var fls = fld.children[il];
            var newel = this.createElement(fls, element);
            if (!newel) {
                continue;
            }
            newels.push(newel);
            element.children.push(newel);
            this.constructInfrastructure(newel, 0, false, this.constructionPropagateDown);
        }
        this.redraw();

        if (this.onAddElementCallback) {
            var ret;
            for(var ie = 0; ie < newels.length; ie++) {
                ret = funcLink(this.onAddElementCallback, this, newels[ie]);
            }
            return ret;
        }
    };

    this.multiSelectDelete=function(container, path){

        var element=this.nodeTree.findByPath(path);

        if(((element.fld.constraint=='search') || (element.fld.constraint=='type') )  && element.viewerAssociated)
        {
            var viewer = this.dataViewEngine[element.viewerAssociated].tabs[0].viewers[0];
            // delete selected values, preserve non-selected
            var preserveValues = viewer.accumulate("node.selected!=1", this.elementValueAccumulator(element));
            if (preserveValues && preserveValues.length == viewer.dim()) {
                // if the user didn't select any values for deletion - assume he wants to delete all values
                preserveValues = [];
            }
            this.changeElementValueByPath(path, preserveValues, 0, true, true);
            this.onMouseOver(this.objCls, element.path);
        }

    };

    this.onClickUrlLink=function (container, path , textbox)
    {
        var element=this.nodeTree.findByPath(path);
        var realUrl = element.fld.link_url.replace(/\$_?\(value\)/g, element.value);
        window.open(realUrl);
    };



    this.onDelElement=function (container, path, row, cntdel, notrecurse,Notredraw)
    {
        //   alert("path"+path)
        if (!Notredraw) Notredraw = false;
        var element=this.nodeTree.findByPath(path);

        if(!cntdel)cntdel=1;

        var newchld=new Array();
        for( var ir=0; ir<element.children.length; ++ir ){

            if (element.children[ir].row >= row && element.children[ir].row < row + cntdel) {
                element.children[ir].value = "";
                if (element.children.length == 1) {
                    newchld[newchld.length] = element.children[ir];
                    break;
                }
                else continue;
            }
            newchld[newchld.length]=element.children[ir];
        }

        if (newchld.length == 0 && element.parent && !notrecurse && element.fld.type != "array") {
            return this.onDelElement(container, element.parent.path, element.row , 1 , true) ;
        }
 //       alert(element.path)
        element.children = newchld;
        if (!Notredraw) this.redraw();
    };

    this.onClearContent = function (container, path) {
        this.changeElementValueByPath(path,"", 0, true);
    };

    this.onChangeSelectValue=function (container, path , selbox)
    {
        var element=this.nodeTree.findByPath(path);
        //alert("before " + element.value);
        element.value=selbox.value;
        if (element.fld.is_multi_fg) element.parent.modifications = 1;
        element.modifications = 1;
        this.validate(this.nodeTree.root, true);
        if(this.autoSaveOnChange)
            this.saveValues(element,true);
        if(this.onChangeCallback)
            return funcLink(this.onChangeCallback, this, element );

    };

    this.onChangeTextValue=function (container, path , textbox)
    {
        var element=this.nodeTree.findByPath(path);
//        if((typeof(element)==="array") && element.length==2)

        if( element.fld.type=="bool")element.value=textbox.checked ? 1 : 0;
        else if(element.fld.type=='file'){
            var o=gObject(this.RVtag+"-"+element.name+'-input');
            if(o){
                element.inputNode = o;
            }
            //alerJ('a',o);
        }else{
            element.value=textbox.value;
        }
       // if (element.fld.constraint.indexOf("choice")!=-1 || element.fld.type == "obj" && (element.fld.constraint == "type" || element.fld.constraint == "search"))
            //this.constructElementViewers(this.nodeTree.roo);
        this.constructElementViewers(element, true);
        this.validate(this.nodeTree.root, true);
        if(this.autoSaveOnChange)
            this.saveValues(element,true);
        if(this.onChangeCallback)
            return funcLink(this.onChangeCallback, this, element );

    };

    this.onElementFocus=function (container, path , fieldtext )
    {
        var element=this.nodeTree.findByPath(path);
        if(element){
            if( !fieldtext ) fieldtext=this.elementDescriptonText(element);
            var o=gObject(this.RVtag+"-"+element.fld.name+"_descriptionDiv");
            if(!o)o=gObject(element.fld.name+"_descriptionDiv");
            if(!o)o=gObject(this.RVtag+"_descriptionDiv");

            if(isok(this.previousHelp) && this.previousHelp!=element.fld.name+"_descriptionDiv" ){
                var v=gObject(this.previousHelp);
                if(v)v.innerHTML="" ;
                this.previousHelp="";
            }

            if(!o && element.parent)return this.onElementFocus(container, element.parent.path, fieldtext );

            if(o) {
                o.innerHTML=fieldtext;
                this.previousHelp=o.id;//element.fld.name+"_descriptionDiv";
            }
        }
    };

    this.onMouseOver=function (container, path , showorhide )
    {
        var element=this.nodeTree.findByPath(path);
        if(element){
            var elname=this.elementName(element);
            if(element){
                if(element.fld.is_multi_fg && element.fld.constraint=="type" && !element.value){
                    showorhide=false;
                }

            }
            // alert("searchig for " + elname+"-controls");
            var o=gObject(elname+'-'+this.RVtag+"-controls");
            if(!o){
                //alert("searchig for parent of " + elname+"-controls which is "+this.elementName(element.parent)+"."+element.row+"-controls");
                elname=this.elementName(element.parent)+"."+element.row;
                o=gObject(elname+'-'+this.RVtag+"-controls");
              //  alert(o);
            }

            return this.showControlsByElname(container, elname, showorhide);
        }
        //setTimeout("this.onMouseOver('"+container+"','"+path+"',"+ (1-showorhide)+" )",5000);

    };

    this.showControlsByElname = function(container, elname, showorhide) {
        var o = gObject(elname+'-'+this.RVtag+"-controls");

        if(isok(this.previousControl) && this.previousControl != elname+'-'+this.RVtag+"-controls" ){
            var v = gObject(this.previousControl);
            if (v) v.className = "sectHid";
            this.previousControl = "";
        }

        //alert("path="+path + "  showorhide="+showorhide + " "+elname+"-controls" );
        if (o) {
            o.className = showorhide ? "sectVis" : "sectHid";
            this.previousControl = elname+'-'+this.RVtag+"-controls";
        }
    };

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ HTML construction function
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    this.wantCollapseElement = function(element)
    {
        var fld = element.fld;
        var elname = this.elementName(element);

        if (((fld.constraint=='search') || (fld.constraint=='type') ) && element.parent.childrenOfTypeViewerAssociatedWithElement && element.parent.childrenOfTypeViewerAssociatedWithElement[fld.name]) {
            if (!fld.doNotCollapseMultiValueViewers && element.name!=element.parent.childrenOfTypeViewerAssociatedWithElement[fld.name].name) {
                return true;
            }
        }

        if (fld.type=="array" && fld.is_multi_fg && this.oneTitleForMultiValueArray) {
            if (!element.parent.onlyOneTitleForMultiArray) {
                element.parent.onlyOneTitleForMultiArray = new Object();
            }

            if (!element.parent.onlyOneTitleForMultiArray[fld.name]) {
                element.parent.onlyOneTitleForMultiArray[fld.name]=element.name;
            } else if (element.parent.onlyOneTitleForMultiArray[fld.name]!=element.name){
                return true;
            }
        }

        return false;
    };

    this.generateText=function ( element, hidetitle, prohibitLayering)
    {
        //var hidetitle = false;
        var fld = element.fld;
        var elname=sanitizeElementId(this.elementName(element));

        var tTit="";

        /*if(fld.hidden || fld.is_hidden_fg)return "";
        if(fld.is_readonly_fg && !this.readonlyMode && !this.showReadonlyInNonReadonlyMode) {
            return "";
        }*/

        //alert(fld.constraint)
        if (this.wantCollapseElement(element))
            hidetitle = true;

        if(fld.hidden) hidetitle=true;

        // _/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Here we construct the title of the element
        // _/

        if(fld.icon)  {
            tTit+="<td><img src='img/"+icon+"' /></td>";
        }

        if (!hidetitle) {  // && fld.hidden
            if(element.children.length>0 )  { // && fld.maxEl>1
                tTit += "<td id='" + elname + "-group'><small>";
                tTit+= "<a href='javascript:vjObjEvent(\"onClickExpand\",\""+ this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")'>";
                tTit+="<span id='"+elname+"-collapser'>"+( element.expanded>=this.expansion ? this.icons.collapse  : this.icons.expand )+"</span>";
                tTit+="</a>";
                tTit+="</small></td>";
            } else
               tTit+="<td>"+this.icons.itemRow+"</td>";

            //if(!fld.div) {
            tTit += "<td id='" + elname + "-title' >";
                 var usetitle = element.title ? element.title : fld.title;
                tTit+=usetitle;
                if(this.debug){
                    tTit+="<small><small><br/>";
                    tTit+= (fld.is_optional_fg?"O":"-")+(fld.is_multi_fg?"M":"-");
                    //if(fld.type=="list" || fld.type=="array")
                        tTit+=":"+elname+":"+element.path;
                    tTit+="</small></small>";
                }
                tTit+="</td>";
            //}
        }


        // _/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Here we construct the body of the element (without children)
        // _/

        if (fld.type == "list" && !fld.constraint ) {

        } else if( fld.type=="array"  ) {
            if( element.children.length<fld.maxEl*fld.children.length ) {
                if(this.debug)tTit+="<td><small><small>["+element.children.length+"]"+fld.type+"</small></small></td>";
            }
        } else { // other types

            val=element.value;

            var eventFuncs="onmouseover='vjObjEvent(\"onMouseOver\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" , 1);vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
            eventFuncs+="onfocus='vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
            eventFuncs+="onmouseout='vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", \" \"); stopDefault(event);' ";

            tTit += "<td align=left " + (fld.hidden ? "class='sectHid'" : "");

            var eventFuncsWithoutClickNode = eventFuncs;
            if (fld.constraint == 'search'  || (fld.constraint == "type") || (fld.constraint == 'choice' || fld.constraint == 'choice+' && !fld.is_readonly_fg && !fld.is_hidden_fg)) { // Searchable types : dialogs are open when clicking on the table cell
                eventFuncs+="onClick='vjObjEvent(\"onClickNode\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", \" \"); stopDefault(event);' ";
            }
            tTit+=">";


            if(fld.is_readonly_fg || this.readonlyMode){
                eventFuncs = '';
                eventFuncsWithoutClickNode = '';
            }

            if ((fld.constraint == 'choice' ) && (fld.is_readonly_fg || fld.is_hidden_fg)) { // CHOICE
                var chc = fld.constraint_data.split("|");
                tTit+="<select" + this.fieldDescriptionTitle(fld) + " ";
                tTit+=eventFuncs;
                tTit+="onchange='vjObjEvent(\"onChangeSelectValue\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", this )' ";
                if(fld.is_readonly_fg)tTit+=" readonly='readonly' disabled='disabled' ";
                tTit+="class='"+this.inputClass+(fld.is_readonly_fg?"ReadOnly":"")+"' type='select' name='"+elname+"' >";
                for( var ic=0;ic<chc.length; ++ic)  {
                    var farr=chc[ic].split("///");
                    //tTit+="<option value='"+farr[0]+"' "+(chc[ic]==val ? "selected" : "")+" />"+farr[1]+(farr.length>1 ? (" - " + farr[1]) : "");
                    tTit+="<option value='"+sanitizeElementAttr(farr[0])+"' "+(farr[0]==val ? "selected" : "")+" >"+(farr.length>1 ? farr[1] : farr[0]) +"</option>";
                }
                if(!chc.length || !chc[0].length){
                    tTit+="<option value='"+sanitizeElementAttr(val)+"' selected >"+val +"</option>";
                }
                tTit+="</select>";

            } else { // every other non specifically composable type

                if (fld.type == "datetime") {
                    //alert("a")
                    val=formaDatetime(val);
                }

                var myTags=" ";
                if(fld.type=="file" || fld.type=='password')    myTags +=  " id='" +this.RVtag+"-"+ element.name + "-input' ";
                if(fld.constraint == 'choice+') myTags += eventFuncsWithoutClickNode;
                else myTags += eventFuncs;
                myTags += "onchange='vjObjEvent(\"onChangeTextValue\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\", this );' ";
                var myType;
                if (fld.is_readonly_fg) {
                    myType = this.inputClass + "ReadOnly";
                    myTags += " readonly='readonly' ";
                }
                else myType = this.inputClass;

                myTags+="class='"+myType+"' "+ (this.textsize ? ("size="+this.textsize+"") : "") +" ";
               // if(fld.is_readonly_fg)myTags+=" readonly ";
                //if( fld.hidden) myTags+="type='hidden' ";
                if (fld.type == "bool") myTags += "type='checkbox' ";
                else if (fld.type == 'password') {
                    myTags += "type='password' ";
                }
                    //else if (fld.type == 'obj' && (fld.constraint == 'type' || fld.constraint == 'search')) myTags += "type='text' ";
                else if(fld.type=="file"){
                    myTags+=" type='file'";
                }
                else if(fld.type=="date"){
                    myTags+=" type='date'";
                }
                else if(fld.type=="datetime"){
                    myTags+=" type='datetime'";
                }
                else myTags += "type='text' ";
                myTags+="name='"+elname+"'" + this.fieldDescriptionTitle(fld);
                var oo = gObject(this.RVtag + "-" + fld.name + "-TEMPLATE");

                if(oo && oo.innerHTML){ // here we do template manipulations, if specified
                    tTit+=oo.innerHTML.replace("%"+this.RVtag+"-INTERFACE-TAGS%",myTags).replace("%"+this.RVtag.toLowerCase()+"-interface-tags%",myTags).replace("%"+this.RVtag+"-INTERFACE-VALUE%",val);
                }
                else if (fld.constraint == 'type' || fld.constraint == 'search' ) {
                    //alert(elname)
                    var myRecordVewer = elname + "Viewer"+'-'+this.RVtag;

                    if(fld.doNotCollapseMultiValueViewers || !element.parent.childrenOfTypeViewerAssociatedWithElement)
                        element.parent.childrenOfTypeViewerAssociatedWithElement=new Object();

                    if( !element.parent.childrenOfTypeViewerAssociatedWithElement[fld.name] ){
                        element.parent.childrenOfTypeViewerAssociatedWithElement[fld.name]=element;
                        element.viewerAssociated = myRecordVewer;

                    }

                    if(element.viewerAssociated ){
                        tTit += "<table " + (this.classChoiceStyle ? "class='" + this.classChoiceStyle + "'" : "") + this.fieldDescriptionTitle(fld) + "><tr><td><input " + (fld.constraint == "search+" ? "" : "class='sectHid'");
                    //tTit += "<table " + (this.classChoiceStyle ? "class='" + this.classChoiceStyle + "'" : "") + "><tr><td><input " + (fld.constraint == "search+" ? "" : "");
                        tTit+=myTags;
                        tTit += " value='" + sanitizeElementAttr(val) + "' ";
                        tTit += "/></td><td><span  " + (element.fld.is_multi_fg ? eventFuncsWithoutClickNode : eventFuncs) + "  id= \"" + myRecordVewer + "\"></span></td>";
                        if(!this.readonlyMode && !fld.is_readonly_fg )

                            tTit+="<td style='vertical-align:top;'><button class='linker' type='button' " + eventFuncs + "><img src='img/combobox.gif'></button></td>";

                        tTit +=    "</tr></table>"; // <button type=button " +  eventFuncs + "></button>
                        // tTit += "<span id= \"" + myRecordVewer + "\"></span>";
                        if(fld.constraint_data=='image' || fld.constraint_data=='system-image'){
                            // for(var i=0;i<val.length;i++){
                                 tTit += "<table " + (this.classSearchStyle ? "class='" + this.classSearchStyle + "'" : "") + "><tr><td>" ;
                                 tTit += "<div id='"+this.RVtag+"-"+elname+"-imageControl'>" ;
                                 element.myTags = myTags;
                                // if(val!=0) tTit+="<img src = '?cmd=objFile&ids="+ val + "' "+myTags+" >" ;
                                 tTit +=    "</div></td></tr></table>";
                            // }
                        }
                    }

                }else if((fld.constraint == 'choice' ||fld.constraint == 'choice+') && !fld.is_readonly_fg && !fld.is_hidden_fg){
                    var chc = fld.constraint_data.split("|");
                    tTit += "<table " + (fld.constraint == "choice+" ? "class='" + this.classSearchStyle + "'" : "class='" + this.classChoiceStyle + "'") + this.fieldDescriptionTitle(fld) + "><tr><td><input " + (fld.constraint == "choice+" ? "" : "class='sectHid'");
                    tTit += myTags;
                    //These thing is the input box, we send these thing to the server
                    tTit += " value='" + sanitizeElementAttr(val) + "' ";
                    //alert(val)
                    var myRecordVewer = elname + "Viewer"+'-'+this.RVtag;
                    element.viewerAssociated = myRecordVewer;
                    element.choiceOption = "description,value\n";
                    var option = new Array();
                    var tagForSpan = ((fld.constraint == 'choice+') ? 'class=sectHid' : '');
                    //alert("value:"+val+"elname is : "+elname)
                    for (var ic = 0; ic < chc.length; ++ic, ir++) {
                        var farr = chc[ic].split("///");
                        // If farr contains embedded double-quotes, escape them for CSV to allow use in vjTable
                        for (var ifarr=0; ifarr < farr.length; ifarr++)
                            farr[ifarr] = quoteForCSV(farr[ifarr]);

                        //if (farr[0] == val) tTit += "value= '" + (farr.length > 1 ? farr[1] : farr[0]) + "'";
                        option.push((farr.length > 1 ? farr[1] : farr[0]) + "," + farr[0]);
                        //following thing goes to the table between span, show to the user
                        if (farr[0] == val) { element.defaltValueShow = "description,value\n" + (farr.length > 1 ? farr[1] : farr[0]) + "," + farr[0]+"\n"; }
                    }
                    element.choiceOption += option.join("\n");
                    //if the choice+, span do not need to been show, instead show the input box
                    tTit += "/></td><td><span "+tagForSpan+" id=\"" + myRecordVewer + "\" ></span></td>";
                    if(!this.readonlyMode)tTit +="<td><button class='linker' type='button' " + eventFuncs + "><img src='img/combobox.gif' ></button></td>";
                    tTit +="</tr></table>";
                   // tTit += "<span id= \"" + myRecordVewer + "\"></span>";
                } else if (fld.constraint == 'search+') {
                    //alert(elname)
                 //   var myRecordVewer = elname + "Viewer";
                  //  element.viewerAssociated = myRecordVewer;
                    //
                    tTit += "<table " + (this.classSearchStyle ? "class='" + this.classSearchStyle + "'" : "") + this.fieldDescriptionTitle(fld) + "><tr><td>";
                    if (fld.type == "text" || ("" + val).length > 60) {
                        tTit += "<textarea cols='60' rows='6' "+myTags +">"+val+"</textarea>";
                    } else {
                        tTit += "<input "+myTags+ " value=\""+sanitizeElementAttr(val)+"\" />";
                    }
                    tTit += "</td>";
                    if(!this.readonlyMode)tTit +="<td ><button class='linker' type='button' " + "onClick='vjObjEvent(\"onClickNode\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\", \" \")' " + eventFuncs + "><img src='img/combobox.gif' align='to: 'top'></button></td>";
                    tTit +="</tr></table>"; // <button type=button " +  eventFuncs + "></button>
                    // tTit += "<span id= \"" + myRecordVewer + "\"></span>";

                }




                else {
                    if(fld.type=="text") {
                        tTit+="<textarea cols='60' rows='6' ";
                        tTit+=myTags;
                        tTit+=" >";
                        tTit+=val;
                        tTit+="</textarea>";
                    }
                    else {

                        tTit+="<input ";
                            tTit+=myTags;
                            if( fld.type=="bool"){
                                tTit+=(parseBool(element.value) ? " checked " : "") ;
                            }
            /*                else if(fld.type=="file"){
                                tTit +=

                            }*/
                            else{
                                //alert(" value=\""+val+"\" ")
                                tTit+=" value=\""+sanitizeElementAttr(val)+"\" ";
                            }
                            //else tTit+=" value=\""+val+"\" ";

                        tTit+=" />";
                    }

                }
                if(this.debug)tTit+="<br/><small><small>"+elname+"</small></small>";

            }
            if(fld.link_url){

                tTit+="<td><a href=javascript:vjObjEvent(\"onClickUrlLink\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")>"+this.icons.urlJump+"</a></td>";
                //alert(tTit);
            }
            //here we add
            tTit+="</td>";

        }


        // _/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ construction of the control section: status, update, etc
        // _/

        if (!hidetitle ) { // && fld.hidden

            if (((this.autoStatus & 0x01) && (fld.type != "list" && fld.type != "array")) ||
                ((this.autoStatus & 0x02) && (fld.type == "list" || fld.type == "array"))) {  tTit += "<td style='vertical-align: top;' id='" + elname + "-status'>" + this.elementStatusText(element) + "</td>"; }

            if( !this.hideControls && !element.hideYourControls ){
                tTit+="<td align=left id='"+elname+'-'+this.RVtag+"-controls' class='sectHid' style='display:table-cell; ' >";
                if (!fld.is_optional_fg && fld.type != "list" && fld.type != "array") {
                    tTit+=this.icons.notEmpty;
                }
                if (this.elementIsRemovable(element) && element.parent && !this.doNotDrawDelete) {
                    if (fld.constraint == "type" || fld.constraint == "search") {
                        tTit += "<a href='javascript:vjObjEvent(\"multiSelectDelete\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\")'>";
                    } else {
                        tTit += "<a href='javascript:vjObjEvent(\"onDelElement\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.parent.path)+"\", "+element.row+")'>";
                    }
                    tTit += this.icons.delRow;
                    tTit += "</a>";
                }
                tTit+="</td>";
            }
            if(this.autoDescription)tTit+="<td id='"+sanitizeElementId(fld.name)+"_descriptionDiv'></td>";


        }


        // _/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Summarize all what is about the current element (not its children )
        // _/

        var t="";
        if(tTit) {
            //t+="<table id='"+elname+"-wrapper' class='"+(fld.hidden ? "sectHid" : this.tblClass)+"' >";
            t += "<table id='"+elname+"-wrapper' class='"+(fld.hidden ? this.tblClass+"_table" : this.tblClass+"_table")+"'";
            if (!fld.is_readonly_fg && !this.readonlyMode) {
                t += "onmouseover='vjObjEvent(\"onMouseOver\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" , 1)'";
            }
            t += "><tr>";
            t += tTit;
            t += "</tr>";
            t += "</table>";
        }



        // _/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Here we construct the children of elements (arrays and lists)
        // _/

        var tGrp = "";
        var ifDrawtGrap;
        if( fld.type=="array" ) {
            tGrp+="<table class='"+( (element.expanded>=this.expansion ) ? (this.tblClass+"_table" + " " + this.tblClass + "_array") : "sectHid")+"' id='"+elname+"-children' border=1>";
                tGrp+="<tr>";
                if(!hidetitle){
                    tGrp+="<td width=30></td>";

                    for( var i=0; i< fld.children.length; ++i) {
                        tGrp+="<th" + (fld.children[i].hidden ? " class='sectHid'" : "") + ">";
                        if(this.debug)tGrp+= (fld.children[i].is_optional_fg?"O":"-")+(fld.children[i].is_multi_fg?"M":"-")+"["+fld.children[i].children.length+"]";
                        tGrp += fld.children[i].title;
                        tGrp+="</th>";
                    }
                //always draw one additional td for control
                    tGrp+="<td >"+this.icons.white+"</td>";
                    tGrp+="</tr>";
                }

            // create the cells
            var subRows = [];
            var subRowsSeen = {};
            for (var ichild=0; ichild<element.children.length; ichild++) {
                var child = element.children[ichild];
                var isub = subRowsSeen[child.row];
                if (isub == undefined) {
                    isub = subRows.length;
                    subRowsSeen[child.row] = isub;
                    subRows[isub] = {};
                }

                subRows[isub][child.fld.name] = ichild;
            }

            // array rows are removable if non-empty and either there are other rows in the array or the array as a whole is optional
            var subRowsRemovable = !this.readonlyMode && !this.doNotDrawDelete && (subRows.length > 1 || fld.is_optional_fg) && !this.fieldIsReadonly(element.fld);

            for (var isub=0; isub<subRows.length; isub++) {
                var ir = 0;
                for (var ifld=0; ifld<fld.children.length; ifld++) {
                    var ichild = subRows[isub][fld.children[ifld].name];
                    if (ichild != undefined) {
                        ir = element.children[ichild].row;
                    }
                }
                var row_elname = this.elementName(element)+"."+ir;
                tGrp+="<tr";
                if (subRowsRemovable) {
                    tGrp += " onmouseover='vjObjEvent(\"showControlsByElname\",\""+ this.objCls+"\",\""+ sanitizeStringJS(row_elname)+"\", 1); stopDefault(event);'"
                }
                tGrp += ">";
                if(!hidetitle ||fld.is_multi_fg )tGrp+="<td width=30></td>";
                for (var ifld=0; ifld<fld.children.length; ifld++) {
                    tGrp += "<td" + (fld.children[ifld].hidden ? " class='sectHid'" : "") + ">";
                    var ichild = subRows[isub][fld.children[ifld].name];
                    if (ichild != undefined) {
                        element.children[ichild].hideYourControls = true;
                        tGrp += "<span class='HIVE_oneliner'>";
                        tGrp += "<span class='HIVE_onelined_vcenter'>" + this.generateText(element.children[ichild], true) + "</span>";
                        if ((this.autoStatus & 0x01) && element.children[ichild].fld.type != "list" && element.children[ichild].fld.type != "array") {
                            tGrp += "<span class='HIVE_onelined_vcenter' id='" + sanitizeElementId(this.elementName(element.children[ichild])) + "-status'>" + this.elementStatusText(element.children[ichild]) + "</span>";
                        }
                        tGrp += "</span>";
                    }
                    tGrp += "</td>";
                }
                tGrp += "<td align=left id='" + elname + "." + ir + '-'+this.RVtag+"-controls' class='sectHid' style='display:table-cell' >";

                if (subRowsRemovable) {
                    tGrp+="<a href='javascript:vjObjEvent(\"onDelElement\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", "+ir+")'>";
                    tGrp += this.icons.delRow;
                    tGrp += "</a>";
                }
                //if(!fld.is_optional_fg)tGrp+=this.icons.notEmpty;
                tGrp+="</td>";
                tGrp+="</tr>";
            }

            if((fld.children[0] && (element.children.length<fld.children[0].maxEl*fld.children.length)) && !this.readonlyMode && !this.editExistingOnly){
                tGrp += "<tr class='" + this.tblClass + "_interface'><td width=30></td>";
                //alert("vjObjEvent(\"onAddElement\",\""+ this.objCls+"\",\""+ element.path+"\"")
                tGrp += "<td><button class='linker' type='button' onclick='javascript:vjObjEvent(\"onAddArrayRow\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\"); stopDefault(event);'" + this.fieldAddTitleTitle(fld) + ">" + this.icons.addRowMore + "&nbsp;" + fld.title + "</button></td>";
                tGrp += "</tr>";
            }

            tGrp+="</table>";

        } else if (fld.type=="list" || fld.type == "array" || ((fld.type == "string" && ((fld.constraint=="search") || (fld.constraint == "type")) ))) {
            //tGrp="";
            // TODO: when there is a UI control for folding/unfolding sub-lists/sub-arrays in an array cell,
            // change from this.tvlClass + "_table" to
            // (element.expanded >= this.expansion ? this.tblClass + "_table" : "sectHid")
            tGrp += "<table class='" + this.tblClass + "_table" + "' id='" + elname + "-children' border=0>";

            for (var il = 0; il < fld.children.length; ++il) {  // to group each kind of its daughters

                var tList="<table border='0'>";
                var thiskind = 0;
                var cntC = 0,rowCtrl=0;
                var ie = 0;
                var unhidden_children = 0;
                for (; ie < element.children.length; ++ie) {
                    if (fld.children[il].name != element.children[ie].fld.name) continue;
                    element.children[ie].hideYourControls = !this.elementIsRemovable(element.children[ie]);
                    if(this.wantCollapseElement(element.children[ie]) || element.children[ie].hidden) {
                        tList += "<tr class='sectHid'>";
                    } else {
                        tList+="<tr>";
                        unhidden_children++;
                    }
                    if(!hidetitle && !fld.children[il].div)tList+="<td width=30></td>";
                    if(element.children[ie].hidden)tList+="<td class='sectHid'>";

                    else tList+="<td>";
                    cntC++;

                    tList += this.generateText(element.children[ie], false, true);
                    tList += "</td>";
                    rowCtrl = element.children[ie].row;
                    tList += "</tr>";
                    ++thiskind;

                }

                var doPlusSign = ( element.childrenOfTypeViewerAssociatedWithElement && element.childrenOfTypeViewerAssociatedWithElement[fld.children[il].name] ) ? false : true ;
                //alerJ(fld.name,element.childrenOfTypeViewerAssociatedWithElement )


                if(thiskind<fld.children[il].maxEl && !this.readonlyMode && !this.editExistingOnly && doPlusSign ) {
                    if(!hidetitle && !fld.children[il].div) {
                        tList+="<tr><td width=30></td><td>";
                    }
                    //tList+="<td>";
                    if (thiskind) {
                        tList += "<table class='" + this.tblClass + "_interface'><tr><td width=30></td><td>";
                    }
                    if ((fld.children[il].constraint !='search+')&& fld.children[il].is_multi_fg &&  !fld.children[il].is_readonly_fg) {
                        tList += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onAddElement\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\",\"" + sanitizeElementAttrJS(fld.children[il].name) + "\"); stopDefault(event);'" + this.fieldAddTitleTitle(fld.children[il]) + ">";
                        tList += thiskind ? this.icons.addRowMore : this.icons.addRow;
                        tList += "&nbsp;" + fld.children[il].title;
                        tList += "</button>";
                    }
                    if (thiskind) {
                        tList += "</td></tr></table>";
                    }
                    if (!hidetitle && !fld.children[il].div) {
                        tList+="</td></tr>";
                    }
                }
                tList+="</table>";


                if (fld.children[il].div) {
                    fld.children[il].innerTxt = tList;
                    tList = "";
                } else {
                    //alert(fld.children[il].title + "   " + fld.children[il].type)
                    var showOrNot=true;
                    if (fld.children[il].is_readonly_fg) {
                        if (this.showReadonlyInNonReadonlyMode) {
                            if (!thiskind) showOrNot = false;
                            //else if (element.children[ie-1].value) showOrNot = false;
                        }
                        else if (this.readonlyMode) showOrNot = true;
                        else showOrNot = false;
                    }
                    if (fld.children[il].title == "ObjectType" || !showOrNot || fld.children[il].hidden || (!cntC && !(thiskind < fld.maxEl && !this.readonlyMode && !this.editExistingOnly))) {
                        this.allHiddenControlsText += "<tr><td>" + tList + "</td></tr>";
                    } else {
                        tGrp += "<tr" + (unhidden_children ? "" : " class='sectHid'")+ "><td>" + tList + "</td></tr>";
                    }
                }
            }
            tGrp+="</table>";
        }





        // _/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Finalize
        // _/
        t+=tGrp;

        fld.innerTxt=t;
        if(!prohibitLayering && fld.div)return ''; // if this one has a div it does not contribute to its parents
        return fld.innerTxt;
    };

}


//# sourceURL = getBaseUrl() + "/js/vjRecordView.js"