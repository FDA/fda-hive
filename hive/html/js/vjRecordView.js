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
    vjDataViewViewer.call(this,viewer);

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
    if(!this.recordEditingCommand)this.recordEditingCommand="record";
    this.td_width = 30
    if (!this.getValidateSeparatorCb) this.getValidateSeparatorCb = function(fld_name) { return null; };

    this._evalKeys = ["$(","$_("];

    if(!this.vjDS)this.vjDS=vjDS;

    if(!this.icons)this.icons={
        collapse: "<img border=0 width=16 height=16 src='img/chevronBottomBlack.svg' title='collapse the hierarchy of sub-elements' />" ,
        expand: "<img border=0 width=16 height=16 src='img/chevronRightBlack.svg' title='expand the hierarchy of sub-elements' />" ,
        help: "<img border=0 width=12 height=12 src='img/recQuestion.gif' title='help' />" ,
        lock: "<img border=0 width=16 height=16 src='img/recLock.gif' title='the field cannot be modified' />" ,
        search: "<img border=0 width=16 height=16 src='img/recSearch.gif' title='lookup possible values' />" ,
        link: "<img border=0 width=16 height=16 src='img/recLink.gif' title='jump to appropriate internet location' />" ,
        delRow: "<img border=0 width=14 height=14 src='img/closeRed.svg' title='delete the specified element' />",
        clearRow: "<img border=0 width=16 height=16 src='img/delete.gif' title='clear the content' />",
        addRow: "<img border=0 width=16 height=16 src='img/recAdd.gif' title='create new sub-element' />" ,
        addRowMore: "<img border=0 width=12 height=12 src='img/addBlue.svg' />",
        revertRow: "<img border=0 width=14 height=14 src='img/redoLightBlue.svg' title='discard modifications' />" ,
        setRow: "<img border=0 width=16 height=16 src='img/recSet.gif' title='save the modifications' />" ,
        itemRow: "<img border=0 width=16 height=16 src='img/chevronRightGrey.svg' title='input element value' />" ,
        errorRow: "<img border=0 width=14 height=14 src='img/exclamationRed.svg'/>",
        notEmpty:"<img border=0 width=16 height=16 src='img/recNotEmpty.gif'  title='field cannot be empty'/>" ,
        white:"<img border=0 width=16 height=16 src='img/white.gif'  title='white img taken one img place'/>" ,
        urlJump: "<img border=0 width=16 height=16 src='img/new.gif' title='jump to the external website' />"
        };

    this.hasNodeDataSource = function() {
        return this.data.length > 1 && this.data[1] != "dsVoid";
    };


    this.readFromDocLoc=function(namearr)
    {
        for( var i=0; i<namearr.length ; ++ i) {
            par = docLocValue(namearr[i]);
            if( isok(par) ) {
                var o=new Object();
                o[namearr[i]]=par.split(",");
                this.changeValueList(o);
            }
        }
    };
    
    this.defaultPropSpecFilter = function(tbl) {
        for (var ir=0; ir<tbl.rows.length; ir++) {
            while (ir<tbl.rows.length && tbl.rows[ir].name[0] == "_") {
                tbl.rows.splice(ir, 1);
            }
        }
    };
    if (!this.propSpecFilter) {
        this.propSpecFilter = this.defaultPropSpecFilter;
    }

    this.composerFunction=function( viewer, content )
    {
        if(!this.formObject && this.formName)
            this.formObject = document.forms[this.formName];
        
        var tMyFloater=gObject("dvFloatingDiv").outerHTML;
        if(!tMyFloater)tMyFloater="";
        this.myFloaterName="floater-"+this.objCls;
        
        tMyFloater=tMyFloater.replace(/dvFloating/g,this.myFloaterName);
        tMyFloater=tMyFloater.replace(/href=\"#\" onclick=\".*\return false;\">/g, "href=\"#\" onclick=\"vjObjEvent('onClosepop','"+this.objCls+"'\)return false;\">");
        gCreateFloatingDiv(tMyFloater);

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

            this.propSpecFilter(tbl);

            this.fldTree=new vjTree(tbl );
            this.fldTree.root.title = objTypeTitle;
            this.fldTree.root.type="list";
            this.fldTree.t_sessionID = this.getDataSessionID(0);
        }

        if (want_new_nodeTree) {
            if (this.hasNodeDataSource() && this.reloadObjIDFromData && !(parseHiveId(this.hiveId).objId > 0)) {
                var elemTxt = this.getData(1).original_data || this.getData(1).data;
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

            this.nodeTree=new vjTree( );
            this.nodeTree.root=cpyObj(this.nodeTree.root, {value: '' , value0: '', fld: this.fldTree.root, row: 0, subRow: 0, obj: this.hiveId, expanded:this.expansion , children: new Array (), depth: 0, path:'/', name:'root'} );
            this.nodeTree.t_sessionID = this.getDataSessionID(1);
        }

        if (want_new_fldTree) {
            this.fldTree.root.children.splice(0,0,{ type_id:-1, default_value: this.objType, name: this.spcTypeObj, title: 'ObjectType', type: '', hidden: true, parent:this.nodeTree.root , is_key_fg:1, is_readonly:1, is_readonly_fg:-1, is_optional:0 , is_multi_fg: 0, is_hidden_fg: 0,  description: 'Determines the type of the object' , children: new Array()});
            if(this.fldPresets)
                this.setFields(this.fldPresets);
            this.fldTree.enumerate(
                function(params, node) {
                    node.is_optional_fg = node.is_optional_fg ? parseIBool(node.is_optional_fg) : 0;
                    node.elname = params.obj.RVtag+"-"+node.name;
                    node.div = params.document.getElementById( node.elname );
                    node.dsSource = params.obj.dataSourceEngine["ds"+params.obj.RVtag+"-"+node.name];

                    var showNode=true;

                    if(node.role === "output"){
                        node.is_readonly_fg = 1
                        node.is_hidden_fg = 1
                    }

                    if (node.is_readonly_fg) {
                        var int_readonly_fg = parseIBool(node.is_readonly_fg);
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
                        if (!node.is_optional_fg && !node.is_readonly_fg) {
                            node.constraint = "choice";
                            node.constraint_data = "1
                        }
                    }

                    node.is_hidden_fg=node.is_hidden_fg ? parseIBool(node.is_hidden_fg) : 0 ;
                    node.is_batch_fg=node.is_batch_fg ? parseIBool(node.is_batch_fg) : 0 ;
                    if(node.is_hidden_fg) node.hidden = true;
                    node.is_multi_fg = node.is_multi_fg ? parseIBool(node.is_multi_fg) : 0;
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
                        node.order = node.order ? node.order : 10000; 
                        newNode.order = parseFloat(node.order) + .1;
                        newNode.is_disabled_fg = true;
                        
                        
                        var confirmNode = parent.children[parent.children.length-1];
                        confirmNode.isCopy=true;
                        confirmNode.title = 'Confirm New '+node.title;
                        confirmNode.subpath = "confirm";
                        confirmNode.name += '.' + confirmNode.subpath;
                        confirmNode.path += '.' + confirmNode.subpath;
                        confirmNode.order = newNode.order + .1;
                        confirmNode.is_disabled_fg = true;
                        confirmNode.passwordSiblingNode = newNode;
                        confirmNode.passwordVerifiableNode = newNode;
                        confirmNode.passwordOldNode = node;
                        
                        newNode.passwordSiblingNode = confirmNode;
                        newNode.passwordConfirmingNode = confirmNode;
                        
                        node.passwordSiblingNodes = [newNode,confirmNode];
                        
                        node.subpath = "old";
                        node.title = 'Old '+ node.title;
                   }
                },
                { document: this.formProviderDoc, obj: this }
            );

            if(this.fldEnumerate)
                this.fldTree.enumerate(this.fldEnumerate, { document: this.formProviderDoc, obj: this });
        }

        if (this.hasNodeDataSource() && want_new_nodeTree) {
            var elemTxt = this.getData(1).original_data || this.getData(1).data;
            if( elemTxt.indexOf("preview:")==0 ) {
                this.div.innerHTML=elemTxt.substring(8);
                return ;
            }

            elemTxt += this.hiveId + "," + this.spcTypeObj + ",0," + this.objType + "\n";
            this.elemArr = new vjTable(elemTxt, 0, vjTable_hasHeader);

            if (!(parseHiveId(this.hiveId).objId>0)) {
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

            if (this.elemArr.rows.length) {
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

        if((!this.hasNodeDataSource() || this.constructionPropagateDown) && (!this.readonlyMode|| this.showReadonlyInNonReadonlyMode) && !this.editExistingOnly ){
            this.constructInfrastructure ( this.nodeTree.root , 0,  true , this.constructionPropagateDown);
        }

        if (this.showReadonlyWhenValueThere && want_new_fldTree) {
            this.nodeTree.enumerate("if(node.fld.is_readonly_fg && node.fld.type!='list' && node.fld.type!='array' && !node.value){node.fld.hidden = true;}");
        }
        this.nodeTree.enumerate("node.cntChildren0=node.children ? node.children.length :0;");
        this.nodeTree.enumerate("node.warnings=node.warnings ? parseInt(node.warnings) : 0 ;");

        this.nodeTree.enumerate(function(params, node) {
            if(!node.fld.order || parseFloat(node.fld.order).toString()=='NaN') {
                node.fld.order = +1000000;
            }
        });
        this.nodeTree.enumerate(function(params, node)
        {
            function sorter(a, b, a2, b2, a3, b3) {
                a = parseFloat(a) || 0;
                b = parseFloat(b) || 0;
                a2 = a2 || "";
                b2 = b2 || "";
                a3 = parseFloat(a3) || 0;
                b3 = parseFloat(b3) || 0;
                return a > b ? 1 : a < b ? -1 : a2 > b2 ? 1 : a2 < b2 ? -1 : a3 > b3 ? 1 : a3 < b3 ? -1 : 0;
            }

            if (node.fld.children && node.fld.children.length)
                node.fld.children.sort(function (a, b) {
                    return sorter(a.order, b.order, a.title?a.title:a.name, b.title?b.title:b.name, 0, 0);
                });
            if (node.children && node.children.length)
                node.children.sort(function (a, b) {
                    return sorter(a.fld.order, b.fld.order, a.fld.title?a.fld.title:a.fld.name, b.fld.title?b.fld.title:b.fld.name, a.row, b.row);
                });
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
                             c_node.fld.is_disabled_fg = false;
                             c_node.fld.passwordVerifiableNode = node.fld;
                             node.fld.passwordConfirmingNode = c_node.fld;
                             c_node.fld.title = c_node.fld.title.replace(/New /,"");
                         }
                         node.fld.subpath = '';
                         node.fld.title = node.fld.title.replace(/^Old /,"");
                     }
                 }
                if(node.fld.subpath === "old" && node.value.length >= 0){
                   node.value = '';
                }
            }
        },{ document: this.formProviderDoc, obj: this });

        this.nodeTree.enumerate( "if(node.depth==0 || (params.autoexpand && ( params.autoexpand=='all' || node.depth<=params.autoexpand) && node.children && node.children.length>0))node.expanded=params.expansion;else node.expanded=0;" , {autoexpand:this.autoexpand, expansion: this.expansion} ) ;
        this.vjDS.add("Retrieving Objects Metadata Information","ds"+this.container,"static:
        
        if(this.onConstructCallback)this.onConstructCallback();
        
        this.redraw();
        if(this.fromDocLoc)
            this.readFromDocLoc(this.fromDocLoc);        
    };

    this.popUpQuery = `
        cnt = $(cnt);
        start = $(start);
        folder = "$(folder)";
        type = "$(type)";
        if(!type || type == "-"){
            type = "*";
        }
        all = false;
        if(folder && type != "*"){
            all =  allusedby(folder,{recurse:true}).filter({.objoftype(type)}); 
        }else if(folder){
            all =  allusedby(folder,{recurse:true}); 
        }else {
            all = alloftype(type);
        }
        result = [];
        for (i=0; i<(int)all; i++) {
            if( !all[i].status){
                result.push(all[i]);
            } else if ( (all[i] as obj).objoftype("^process$+") &&  all[i].status == 0 | 1 | 2 | 3 | 4 | 6 | 7 ){
                continue;
            } else {
                result.push(all[i]);
            }
        }
        total = result as int;
        if(cnt && total){
            end = min(start + cnt, total) - 1;
            result = result[start : end];
        }
        return  cat(result.csv(["_brief","created","_type"]) , "info,total,," , total ,"\ninfo,start,," , start);
    `

    this.popUpQuerySearch = `
        folder = "$(folder)";
        type = "$(type)";
        if(!type || type == "-"){
            type = "*";
        }
        all = false;
        if(folder && type != "*"){
            all =  allusedby(folder,{recurse:true}).filter({.objoftype(type)}); 
        }else if(folder){
            all =  allusedby(folder,{recurse:true}); 
        }else {
            all = alloftype(type);
        }
        all = all.filter({ ._brief=~ /$(search)/i});
        result = [];
        for (i=0; i<(int)all; i++) {
        if( !all[i].status){
            result.push(all[i]);
        } else if ( (all[i] as obj).objoftype("^process$+") &&  all[i].status == 0 | 1 | 2 | 3 | 4 | 6 | 7 ){
            continue;
        } else {
            result.push(all[i]);
            }
        }
        total = result as int;
        return  cat(result.csv(["_brief","created","_type"]) , "info,total,," , total ,"\ninfo,start,,0");
    `

    this.constructPopUpViewer = function(tabname, popupType){
        if(!popupType)
            popupType = "basic";

        if(popupType == "basic"){
            this.popObjectPanel = new vjPanelView({
                data:[ "dsVoid", "ds"+this.container],
                formObject: document.forms["form-floatingDiv"],
                iconSize:24,
                rows: [
                    {name:'pager', icon:'page' , title:'per page', description: 'PAge up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                    {name: 'search', align: 'right', type: ' search', isSubmitable: true, title: 'Search', description: 'search sequences by ID', url: "?cmd=objFile&ids=$(ids)" }
                ],
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
                    selectCallback: "function:vjObjFunc('onSelectPopupList','" + this.objCls + "')",
                    checkCallback: "function:vjObjFunc('onCheckPopupList','" + this.objCls + "')",
                    defaultIcon:'rec',
                    geometry:{ width:'96%',height:'100%'},
                    iconSize:0,
                    isok:true });

            this.popupViewer = this.myFloaterName+"Viewer";
             this.vjDV.add(this.popupViewer, (this.popUpViewerWidth && !isNaN(this.popUpViewerWidth))? parseInt(this.popUpViewerWidth):600,300);
             this.vjDV[this.popupViewer].add("select","table","tab",[ this.popObjectPanel, this.popObjectViewer ]);
             this.vjDV[this.popupViewer].render();
             this.vjDV[this.popupViewer].load();
        }
        if(popupType == "explorer"){
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
                 formPreviews:"form-floatingDiv",
                 isNShowactions:true,
                 isNoUpload: true,
                 subTablesAttrs : [
                                    {    
                                       tabname: tabname,
                                       tabico: "folder-apps",
                                       url: { },
                                       addCmd: [
                                        { 
                                            name: 'search', 
                                            align: 'right', 
                                            type: 'search', 
                                            isSubmitable: true, 
                                            title: 'Search', 
                                            description: 'Search sequences by name', 
                                            evalVarCallback: this.evalVarCallbackAll,
                                            url_tmplt: 'http:
                                            qry_tmplt: `${this.popUpQuerySearch}`, 
                                        }
                                        ],
                                       evalVarCallback: this.evalVarCallbackAll,
                                       url_tmplt: 'http:
                                       qry_tmplt: `${this.popUpQuery}`,
                                       urlValues: { start: 0, cnt:20, search: ""}
                                    },{    
                                        tabname: 'Flat List',
                                        tabico: "elements_tree.png",
                                        url: { },
                                        isHiddenCallback: this.getIfFlatListHidden,
                                        addCmd: [
                                            {name:'pager',hidden: true , type:'pager',prohibit_new: false},
                                            { 
                                                name: 'search', 
                                                align: 'right', 
                                                type: 'search', 
                                                isSubmitable: true, 
                                                title: 'Search', 
                                                description: 'Search sequences by name', 
                                                evalVarCallback: this.evalVarCallbackFlatList,
                                                urlWithoutTypeFilter: '?cmdr=objQry&qry=allusedby($(folder),{recurse:true,with_topic:false}).filter({!(.objoftype("sysfolder,folder"))}).filter({._brief=~/$(search)/i}).csv(["_brief","created","_type"])',
                                                url: '?cmdr=objQry&qry=allusedby($(folder),{recurse:true,with_topic:false}).filter({.objoftype("$(type)")}).filter({!(.objoftype("sysfolder,folder"))}).filter({._brief=~/$(search)/i}).csv(["_brief","created","_type"])', 
                                                url_tmplt: 'http:
                                                qry_tmplt: 'allusedby($(folder),{recurse:true,with_topic:false}).filter({.objoftype("$(type)")}).filter({!(.objoftype("sysfolder,folder"))}).filter({._brief=~/$(search)/i}).csv(["_brief","created","_type"])' 
                                            }
                                        ],
                                        evalVarCallback: this.evalVarCallbackFlatList,
                                        urlWithoutTypeFilter: 'allusedby($(folder),{recurse:true,with_topic:false}).filter({!(.objoftype("sysfolder,folder"))}).csv(["_brief","created","_type"])',
                                        url_tmplt: 'http:
                                        qry_tmplt: 'allusedby($(folder),{recurse:true,with_topic:false}).filter({.objoftype("$(type)")}).filter({!(.objoftype("sysfolder,folder"))}).csv(["_brief","created","_type"])'

                                    }
                                ],
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

    this.evalVarCallbackAll = (curAttrs,dsurl, url_obj, start, fin) => {
        url_obj.folder = url_obj.folder === '-' ? '': url_obj.folder;
        url_obj.type = (url_obj.type === " " || url_obj.type === undefined || !url_obj.type ) ? "" : url_obj.type;
        return evalVars( dsurl, start, fin, url_obj );
    }
    this.evalVarCallbackFlatList = (curAttrs,dsurl, url_obj,  start, fin) =>{

        if(url_obj.folder=="-"){
            url_obj.folder ="alloftype(\"*\")";
        }else{
            url_obj.folder ="((obj)"+url_obj.folder+").child ";
        }

        if(url_obj.type === " " || url_obj.type === undefined || !url_obj.type ){
            return evalVars( `${curAttrs.urlWithoutTypeFilter}`, start, fin, url_obj )
        }
        return evalVars( dsurl, start, fin, url_obj )
    }
    this.getIfFlatListHidden = (folderName) => {
        if(folderName === "/all"){
            return true
        }
        return false
    }




    this.redraw=function( elements ,ifFromChange)
    {
        if (!elements) elements = this.nodeTree.root;
        if(!ifFromChange)    ifFromChange = false;
        this.validate(elements);
        this.fldTree.enumerate( "node.innerTxt='';" ) ;
        this.nodeTree.enumerate("node.onlyOneTitleForMultiArray='';");


        this.allHiddenControlsText = "";
        var t="";
        if (this.prefixHTML) t += this.prefixHTML;
        if (this.implementTreeButton) {
            t += "<table align=topleft class='REC_table'>";

            t += "<tr><td><img onClick='vjObjEvent(\"showAllFunction\",\"" + this.objCls + "\")' src=" + (this.showReadonlyInNonReadonlyMode ? 'img/off_icon.gif' : 'img/on_icon.gif') + " border=0 width=20 height=20 title='show/hide readonly field'/></td>";
            t += "<td><img onClick='vjObjEvent(\"expandAllFunction\",\"" + this.objCls + "\",\"\")' src=" + (!this.nodeTree.root.expanded ? 'img/recExpand.gif' : 'img/recCollapse.gif') + " border=0 width=20 height=20 title='expand/collapse all field'/></td>";
            t += " <td><img onClick='vjObjEvent(\"constructAllFunction\",\"" + this.objCls + "\")'" + (this.constructAllField ? 'class=sectHid' : 'src="img/recAdd.gif" title="show all editable Field show"') + "  border=0 width=16 height=16 /></td></tr>";
            t += "<tr><td>" + (this.showReadonlyInNonReadonlyMode ? 'Hide' : 'Show') + "</td><td>" + (!this.nodeTree.root.expanded ? 'Expand' : 'Collapse') + "</td><td border=0>" + (this.constructAllField ? '' : 'Construct') + "</td></tr></table>";
        }

        t += this.generateText(elements, this.showRoot ? false : true);
        
        if (this.implementSaveButton) {
            let value = this.saveButtonText ?  this.saveButtonText :  (parseHiveId(this.hiveId).objId>0 ) ? 'SAVE' : 'CREATE';
            let btn_class = this.saveButtonClass ? this.saveButtonClass : '';
            t += `<input class="${btn_class}" type='button' name='save_record+${this.container}' value="${value}" onclick='vjObjEvent(\"onSetVerification\",\"${this.objCls}\",\"/\",\"save\")' />`;
        }

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
            fld.dsSource.reload("static:
        }


        for ( var ic=0; ic<fld.children.length ; ++ic) {
            this.setLayoutFields(fld.children[ic]);
        }
    };


    this.constructElementViewers=function ( element ,ifFromChange)
    {


        if(element.fld.type =='file'){
            var o=gObject(this.RVtag+"-"+element.name+"-input");
            if(o && element.inputNode){
                o.parentNode.replaceChild(element.inputNode,o);
            }
        }

        if (element.viewerAssociated && element.fld.constraint!='search+' && element.fld.constraint != "choice+")  {
            var newInlineList = new Array();
            var hideRowList=null, showRowList=null;
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
                url = haveChildrenIdVals ? "http:
            }
            else if (element.fld.constraint == 'search') {
                if (childrenVals.length) {
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
                            url = constraint_data.inline_url ? constraint_data.inline_url : "http:
                            url = urlExchangeParameter(url, "ids", encodeURIComponent(childrenVals));
                            url = urlExchangeParameter(url, "prop", encodeURIComponent(constraint_data.fetch + "," + inlineProps));
                        }
                    }
                    if(!url){
                        url = "" + constraint_data.url;
                        hideRowList = new Object();
                        hideRowList[constraint_data.fetch] = new RegExp("^("+sanitizeStringRE(childrenVals).split(",").join("|")+")$");
                    }
                    if (constraint_data.inline) {
                        newInlineList.push({ name: new RegExp(/.*/), hidden: true });
                        if (constraint_data.inline instanceof Array)
                            newInlineList = newInlineList.concat(constraint_data.inline);
                        else {
                            var inline = constraint_data.inline.split(',');
                            for (var i = 0; i < inline.length; i++)
                                newInlineList.push({ name: inline[i], hidden: false, title: inline[i] });
                        }
                    }

                }
                else url = "static:

            }
            else if (element.fld.constraint == "choice" ) {
                var url;
                if (element.defaltValueShow && !ifFromChange) {
                    url = "static:
                } else {
                    url = "static:
                    var choiceOptionFound = (element.choiceOption || []).some(function(c) {
                        if (c.value == element.value) {
                            url += "\n" + quoteForCSV(c.description) + "," + quoteForCSV(c.value);
                            return true;
                        }
                        return false;
                    }, this);
                    if (!choiceOptionFound) {
                        url += "\n" + quoteForCSV(element.value) + "," + quoteForCSV(element.value);
                    }
                }

            }
            if (["choice", "search", "type"].includes(element.fld.constraint) && !url.startsWith("static:
                && !!element.fld.default_value && element.fld.default_value == element.value) {
                showRowList = JSON.parse(element.fld.default_value);
                var that = this;
                element.fld.callbackRendered = function(myviewer){
                    element.value = myviewer.accumulate("true", that.elementValueAccumulator(element))[0];
                }
                hideRowList = null;
            }

            this.dataSourceEngine.add("infrastructure: Constructing Objecyt Lists in RecrodViewer", 'ds' + element.name, url);
            this.dataViewEngine.add(element.viewerAssociated, 500, 500).frame = 'none';
            this.defaultInlineShow = [{ name: new RegExp(/./), hidden: true },{ name: '_brief', hidden: false, title: 'Description' },{name:'icon', hidden: true,type: 'icon'},{ name: 'description', hidden: false, title: 'Description' },{ name: 'brief', hidden: false, title: 'Summary' }, { name: new RegExp(/Title.*/i), hidden: false, title: 'Title' }];
            if(element.fld.constraint == 'type'){
                this.defaultInlineShow.push( {name: 'id' , hidden: false })
            }
            var myListViewer=new vjTableView( {
                data: "ds"+element.name,
                formObject: this.formObject,
                bgColors:['#F0F3F9','#F0F3F9'] ,
                isNheader: true,
                defaultEmptyText:" ",
                cols: newInlineList.length ? newInlineList : this.defaultInlineShow,
                inclusionObjRegex: hideRowList ,
                inclusionByIndex: showRowList ,
                selectCallback: element.fld.selectCallback,
                checkCallback: element.fld.checkCallback,
                callbackRendered: element.fld.callbackRendered,
                maxTxtLen:64,
                multiSelect:true,
                geometry:{ width:'100%',height:'100%'},
                doNotShowRefreshIcon: true,
                iconSize: 0,
                defaultIcon:'rec',
                field_constraint: element.fld.constraint,
                isok: true
            });

            this.dataViewEngine[element.viewerAssociated].add("details", "table", "tab", [myListViewer]);
            this.dataViewEngine[element.viewerAssociated].render();

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
                }
            }
        }
        for (var ie=0;  ie<element.children.length; ++ie)
        {
            this.constructElementViewers(element.children[ie],ifFromChange);
        }

    };



    this.createElement=function( fld, element)
    {
        var newel=new Object( {value: '' , value0:'', fld: fld, row: element.subRow, subRow: 0, obj: element.obj, expanded:this.expansion, children: new Array (), depth: element.depth+1, distance: 1 } );
        newel.parent=element;
        newel.name=this.elementName(newel);
        newel.path=element.path+newel.name+"/"; 

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
        if (element == this.nodeTree.root) {
            if (!epxandOrClose)
                epxandOrClose = element.expanded ? 0 : 1;
        }
        element.expanded = epxandOrClose;

        for (var ic = 0; ic < element.children.length; ic++) this.expandAllFunction(container, element.children[ic], epxandOrClose);
        if (element == this.nodeTree.root) this.redraw();
    };

    this.constructInfrastructure=function(element, kind , onlyifnochildren, propagateDown)
    {

        var fld=element.fld;
        var newel;
        var is_newly_created_newel = false;
        var this_ = this;

        var array_rows = {};
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

                if(kind && fls.name!=kind  ) continue;



                if(docreate){
                    newel = undefined;
                    is_newly_created_newel = false;
                    if (onlyifnochildren) {
                        for (var ie=0; ie<row_ichildren.length; ++ie) {
                            var child = element.children[row_ichildren[ie]];
                            if (child.fld.name == fls.name) {
                                newel = child;
                                break;
                            }
                        }
                    }
                    if (!newel) {
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

                        if (!fls_is_global_multi && this_.accumulate("node.fld.name=='" + (fls.name) + "'", "node").length) {
                            continue;
                        }

                        newel=this_.createElement( fls, element) ;
                        if(!newel)continue;
                        newel.cntChildren0 = 0;
                        is_newly_created_newel = true;
                        element.children.push(newel);
                    }

                    if(propagateDown>1 || fls.type=="array") {
                        this_.constructInfrastructure ( newel , 0, onlyifnochildren, fld.type=="array" ? propagateDown : propagateDown-1);
                        if (is_newly_created_newel && newel.children) {
                            newel.cntChildren0 = newel.children.length;
                        }
                    }
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
            var fld=this.fldTree.findByName( row.name );

            if(!fld){
                if(row.name.charAt(0)=="_") {
                    if(!this[row.name.substring(1)])this[row.name.substring(1)]=new Object();
                    this[row.name.substring(1)][row.value]=1;
                }
                continue;
            }

            var el=this.nodeTree.findByName(row.name );
            if(!el) {

                var parList=new Array();
                for ( var curT=fld; curT.parent && (curT!=element.fld); curT=curT.parent ){
                    parList.push(curT);
                }
                var parentelem=element;
                var rowlist="";

                for (var ip = parList.length - 1; ip >= 0; --ip) {
                    if (row.path[row.path.length - ip - 1] == "NaN") row.path[row.path.length - ip - 1] = '0';
                    var ir=(ip<row.path.length)? row.path[row.path.length-ip-1] : "0";
                    parentelem.subRow=parseInt(ir);
                    rowlist = rowlist + "." + ir;

                    var elname = "" + this.objCls + ".prop."+this.idForProp(obj)+"."+parList[ip].name+rowlist;
                    el=this.nodeTree.findByName(elname,parentelem);
                    if(el){
                        parentelem=el;
                        continue;
                    }

                    if( parentelem.fld.type=="array") {
                        for( var il=0; il<parentelem.fld.children.length; ++il) {
                            var thisel=this.createElement( parentelem.fld.children[il], parentelem) ;
                            if(!thisel)continue;
                            if(ip>0) {thisel.value0='';thisel.value=thisel.value0;}
                            parentelem.children.push(thisel);
                        }
                        el=this.nodeTree.findByName(elname,parentelem);
                    }
                    else {
                        el=this.createElement( parList[ip], parentelem) ;
                        if(!el)break;
                            if(ip>0) {el.value0='';el.value=el.value0;}
                            parentelem.children.push(el);
                    }

                    parentelem=el;
                }

            }
            if(!el){
                continue;
            }
                el.value0=valarr.rows[iv].value;
                el.value=el.value0;
            if( el.parent ) {
                if(el.parent && row.path.length && el.parent.subRow<=row.path[row.path.length-1])
                    el.parent.subRow=parseInt(row.path[row.path.length-1])+1;
            }

        }

        return element;
    };




    this.idForProp=function(id)
    {
        return parseHiveId(id).objId ? id : this.propFormatId;
    };

    this.elementName=function(element, forAccumulateValues)
    {
        var prefix = forAccumulateValues ? "" : ("" + this.objCls + ".");
        if(element.fld.name==this.spcTypeObj)
            return prefix + "prop."+this.idForProp(element.hiveId)+"._type";

        var t=new Array();
        for (var cur=element; cur && cur.parent!=cur && cur.parent; cur=cur.parent)  {
            t.push(cur.row);
        }

        return prefix + "prop."+this.idForProp(element.obj)+"."+element.fld.name+"."+t.reverse().join(".");
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

            if (!element.errors && this.implementSetStatusButton) {
                if (element.children.length) {
                    t += "<td>";
                    t += "<a href='javascript:vjObjEvent(\"onSet\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\")' >" + this.icons.setRow + "</a>";
                    t += "</td>";
                }
            }

            t+="<td>";

            t+="<a href='javascript:vjObjEvent(\"onRevert\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")' >"+this.icons.revertRow+"</a>";
            t+="</td>";



        }

        if (!this.hideErrors && element.errors) {
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

    this.validate=function ( element, visualize, skip_optionality_validation, errorText)
    {
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
            
            this.validate(element.children[ie], visualize, skip_optionality_validation, errorText);

            if (element.children[ie].errors > 0 && element.children[ie].fld.type == "string") {
                element.errorTooltip = element.children[ie].errorTooltip;
                element.errorText = element.children[ie].errorText;
            }
            element.errors += element.children[ie].errors;
            element.warnings += element.children[ie].warnings;
            element.modifications+=element.children[ie].modifications;
        }

        if (!element.fld.is_hidden_fg&&!element.fld.is_readonly_fg && (!isok(element.value) || element.value == 0) && !element.fld.is_optional_fg && (element.fld.type != "list") && (element.fld.type != "array")) {
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
             else if(!isok(element.value) && (element.value.toString() != "0")){
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
                var verifyNode = element.fld.hasOwnProperty('passwordVerifiableNode') ? this.accumulate("(node.fld.name=='" + (element.fld.passwordVerifiableNode.name) + "') ", "node", null,element.parent)[0] : null;
                
                var confirmNode = element.fld.hasOwnProperty('passwordConfirmingNode') ? this.accumulate("(node.fld.name=='" + (element.fld.passwordConfirmingNode.name) + "') ", "node", null,element.parent)[0] : null;
                
                if(confirmNode && (confirmNode.value || element.value) && confirmNode.value!= element.value){
                    element.errors++;
                    element.errorText = "Passwords do not match.";
                } else if(verifyNode && (verifyNode.value || element.value) && verifyNode.value!=element.value){
                    element.errors++;
                    element.errorText = "Passwords do not match.";
                } else if (verifyNode && verifyNode.value && element.value && verifyNode.value==element.value && verifyNode.value.length >= 0){
                    var oldNode = this.accumulate("(node.fld.name=='" + (element.fld.passwordOldNode.name) + "') && !node.isCopy", "node");
                    if(oldNode.length==1){
                        if(oldNode[0].hidden ||oldNode[0].overWrite ){
                            oldNode[0].value = element.value;
                            oldNode[0].modifications=1;
                        }
                    }
                }
            }
            else if (!element.hidden){
                if(element.fld.subpath === 'old'){
                    
                    if (element.value != element.value0 && !element.overWrite){
                        element.errors++;
                        element.errorText = "Does not match your old password.";
                    } else {
                        element.overWrite = true;
                        
                        let siblings = element.fld.passwordSiblingNodes;
                        for (let i = 0 ; i < siblings.length ; i++ ) {
                            let sibNode = this.accumulate("(node.fld.name=='" + (siblings[i].name) + "') ", "node", null,element.parent)[0];
                            let sibling = gObject(this.RVtag+"-"+sibNode.name+"-input");
                            sibling.disabled = false;
                        }
                        
                        var o=gObject(this.RVtag+"-"+element.name+"-input");
                        if( o ){
                            o.readOnly = true;
                        }
                    }
                }
            }
        }
        if(element.fld.type=="datetime"){
            var val = element.value;
            var date = formatToDate(element.value);

            if(val != "" && isNaN(date.getDate())){
                element.errors++;
                element.errorText = "invalid date format"; 
                element.fld.errors=element.errors;
            }
        }



        if (isok(element.value) && element.modifications && element.fld.constraint == "regexp") {
            var constraint_data = element.fld.constraint_data;
            if(constraint_data.indexOf("/") == 0) constraint_data = constraint_data.substring(1);
            if(constraint_data.lastIndexOf("/") == constraint_data.length-1) constraint_data = constraint_data.substring(0, constraint_data.length-1);
            var myRegExp = new RegExp(constraint_data, 'u');
            if (element.value.match(myRegExp) == null) {
                element.errors++;
                element.errorText = "please give something within the constraint: " + (element.fld.constraint_description? element.fld.constraint_description:element.fld.constraint_data);
            }
        }
        
        element.fld.modifications=element.modifications;
        element.fld.errors=element.errors;

        if(visualize) {
            var elname=this.elementName(element);

            var o=gObject(elname+"-status");
            var v=gObject(this.RVtag+"-"+element.fld.name+"-status");
            var g=gObject(this.RVtag+"-status");
            if (o || v || g) {
                t = this.elementStatusText(element);
                if(o)o.innerHTML=t;
                if(v)v.innerHTML=t;
                if(g)g.innerHTML=t;
            }

        }
    };

    this.accumulateValues=function(element, propagatedown, separator, togetherWithNonModified, withoutHidden, cmdLineStyle, forSubmission, formData)
    {
        var t="";

        if(element) {
            var doshow = true;
            var dofiles = null;

            if( !togetherWithNonModified && element.fld.type != "file" && element.value==element.value0 )
                doshow=false;
            if( withoutHidden && ( element.fld.is_hidden_fg==true || element.fld.name.charAt(0) == "_") && !element.fld.force_unhidden )
                doshow=false;

            if (element.fld.type == "file") {
                if (element.inputNode.files && element.inputNode.files.length) {
                    dofiles = element.inputNode.files;
                }
            } else {
                if (!element.value || element.value.length == 0)
                    doshow = false;
                if (!element.value && element.modifications>0 && element.fld.type!='list' && element.fld.type!='array')
                    doshow = true;
            }

            if (forSubmission && !element.fld.is_submittable) {
                doshow = false;
                dofiles = null;
            }

            if (doshow) {
                var name;
                var value = element.value;
                if (dofiles && !formData) {
                    value = Array.prototype.map.call(dofiles, function(file) { return file.name }).join(";");
                }

                var equals = "=";
                var encode = true;

                if (cmdLineStyle=="CGI") {
                    name = element.fld.name;
                } else if (cmdLineStyle=="no_prop") {
                    name = this.elementName(element, true).replace(/prop\.[0-9]\./,"").replace(/\./g,"_");
                } else if (cmdLineStyle) {
                    name = "-" + element.fld.name;
                    equals = " ";
                    encode = false;
                    value = '"' + value + '"';
                } else {
                    name = this.elementName(element, true);
                }
                
                if(element.fld.type == "datetime"){
                    let tmp = (new Date(value)).getTime();
                    tmp /= 1000;

                    if(isNaN(tmp)) value = (new Date(parseInt(value))).getTime(); 
                    else value = tmp;
                }

                if (formData) {
                    if (dofiles) {
                        Array.prototype.forEach.call(dofiles, function(file) {
                            formData.append(name, file);
                        });
                    } else {
                        formData.append(name, value);
                    }
                } else {
                    if (encode) {
                        value = encodeURIComponent(value);
                    }
                    t += name + equals + value;
                }
            }
        }
        else element=this.nodeTree.root;
        if(!propagatedown)return ;
        for( var ie=0; ie<element.children.length; ++ie) {
            var r=this.accumulateValues(element.children[ie], propagatedown, separator, togetherWithNonModified, withoutHidden, cmdLineStyle, forSubmission, formData);
            if(r.length && t.length) t+=separator;
            t+=r;
        }

        return t;
    };

    this.changeElementValue=function (fldName , eleVal , rownum , dovalidate , doTriggerOnChange, forceConstruct)
    {
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
    };

    this.changeElementValueByPath=function (fldPath , eleVal , rownum , dovalidate, doTriggerOnChange, forceConstruct, donotredraw, node)
    {

        if(!rownum)rownum=0;
        var arr = this.accumulate("node.path=='" + (fldPath) + "'", "node");

        var element=this.nodeTree.findByPath(fldPath);
        if(element.elementValueArray)    element.elementValueArray = eleVal;
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


        }


        this.constructElementViewers(element, true);
        if(this.autoSaveOnChange)
            this.saveValues(element,true);

        if (dovalidate) {
            this.validate(par, true);
        }

        if(doTriggerOnChange && this.onChangeCallback)
            return funcLink(this.onChangeCallback, this, arr[rownum], this.formObject.elements[arr[rownum].name], node );

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
                                        elem.choiceOption = txtVal.split("\n").map(function(a) {
                                            return { description: a, value: a };
                                        });
                                        txtVal=txtVal.replace(/\n/g,"|");
                                    }
                                    elem.fld.constraint_data=txtVal;
                                                                        
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

    this.getElement=function( fldName, which, node)
    {
        var arr = this.accumulate("node.fld.name=='" + (fldName) + "'", "node",null,node);
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

    this.computeConstraintData = function(element) {
        var constraint_data = "";
        var fld = element.fld;
        if (fld.constraint == "search" || fld.constraint == "search+") {
            var this_ = this;
            var constraint_data_raw = eval("new Object(" + fld.constraint_data + ")");
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
    this.getElementValue=function( fldName, which)
    {

        var vals=this.accumulate( "node.fld.name=='"+fldName+"' && isok(node.value)","node.value" );


        if(!vals)return null;
        if(!which)which=this.whichDefined;

        if(!which )return vals[0];
        else if(which=="join")return vals.join(",");
        else if(which=="array")return vals;
        else return vals[parseInt(which)];
        return vals[0];
    };


    this.attached=function(kind, name )
    {
        if(!name)return this[kind];
        else if(!this[kind])return null;
        return this[kind][name];

    };

    this.saveValues=function( element, useajax , callback,whattodo )
    {
        var separator=this.separator?this.separator:"&";
        this.setUrl = this.cmdPropSet + separator + "raw=1" + separator;

        this.enumerate("if(node.fld.type=='list') node.value='';if(node.isCopy)  node.value='';");
        var arr = this.accumulate("(node.fld.type=='password') && node.value && node.value.length", "node");
        if(arr.length && arr[0].fld.type == 'password') {
            this.submitByPost = true;
        }
        var formData;
        if (useajax == "FormData") {
            formData = new FormData();
        }
        this.setUrl += this.accumulateValues(element, true, separator, this.accumulateWithNonModified, this.accumulateWithoutHidden, this.submitFormat, true, formData);

        var oldcb;
        if(callback){
            oldcb=this.setDoneCallback ;
            this.setDoneCallback = callback;
        }
        if( useajax && useajax != "later") {
            var submitByPost = this.submitByPost || (useajax == "FormData");
            ajaxDynaRequestPage(this.setUrl, {objCls: this.objCls, callback:'onSetCallback', dowhat:whattodo}, vjObjAjaxCallback, submitByPost, formData);
        } else {
                if(useajax == "later") return;
            this.formObject.submit();
        }
        if(oldcb) {
            this.setDoneCallback =oldcb;
        }
    };
    
    this.submitAfterSave = function (element, useajax, callback, whattodo ){
            var submitByPost = this.submitByPost || (useajax == "FormData");
        ajaxDynaRequestPage(this.setUrl, {objCls: this.objCls, callback:'onSetCallback', dowhat:whattodo}, vjObjAjaxCallback, submitByPost, undefined);
    }

    this.accumulate=function( checker, collector , params, node )
    {
        return this.nodeTree?this.nodeTree.accumulate( checker, collector , params, false,0, node ):0;
    };
    this.enumerate=function( operation, params, node )
    {
        if(!this.nodeTree)return ;
        return this.nodeTree.enumerate( operation, params , false, 0, node);
    };




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

    this.onSetCallback=function(param, text , page_request)
    {

        if(text.indexOf('error:')!=-1 || text.indexOf('err.') != -1){
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
            if(this.ErrorOnSave){
               this.ErrorOnSave(text);
            }
        }
 
        if (this.implementSaveButton || this.implementCopyButton) {
            var hiveId = text.slice(text.indexOf("_id=") + 4);
            if(parseHiveId(hiveId).objId > 0) this.ForSaveReload(hiveId);
            else if(param.dowhat && param.dowhat=='copy')
                this.ForSaveReload('-'+this.hiveId);
        } 


        if(this.setDoneCallback)
            return funcLink(this.setDoneCallback, this, text , page_request);          
        for(var iv=0; iv<this.data.length; ++iv) {
            this.getData(iv).reload(null,true);
        }
    };

    if (!this.ForSaveReload){
        this.ForSaveReload = function (hiveId) {
            newurl = makeCmdSafe(this.recordEditingCommand)+"&ids=" + hiveId + "&types=" + this.objType;
            if(hiveId.charAt(0)!='-')alert("Record Created Successfully");
            else alert("Record Copied Successfully")
            window.location.href = newurl;
        };
    }

    this.onSet=function( container, path, whattodo)
    {
        var element=this.nodeTree.findByPath(path);

        var docontinue=true;
        if(this.setCallback)
            docontinue = this.setCallback(container, path, whattodo);
        if(!docontinue)
            return;
        if (!this.noAutoSubmit) {
            this.saveValues(element, true,null,whattodo);
        }else if (this.noAutoSubmit) {
           this.saveValues(element, true ,this.myCallback,whattodo);
         }
    };

    this.onSetVerification = function (container, path, whattodo) {
        if (((this.nodeTree.root.warnings == this.nodeTree.root.errors) && this.hiveId == this.objType) || ((this.hiveId != this.objType) && !this.nodeTree.root.errors)) this.onSet(container, path, whattodo);
        else if (this.nodeTree.root.errors) alert("Please give the value within constraint");
    };

    this.setFields=function(obj)
    {
        if(typeof obj == 'string')
            obj=eval(obj);
        for ( nm in obj ) {
            var el = this.fldTree.findByName(nm);
            if ( !el )continue;

            for ( attr in obj[nm] ) {
                var val= obj[nm][attr];
                el[attr]=obj[nm][attr];
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
        if (element.expanded) {
            o.className=this.tblClass+"_table";
            plsmin=this.icons.collapse;
        } else {
            o.className="sectHid";
            plsmin=this.icons.expand;
        }
        o=gObject(elname+"-collapser");if(!o)return;

        o.innerHTML=plsmin;

    };

    this.onClickNode=function(container, path, event)
    {

        if(this.currentPopupElement){
            this.onClosepop();
        }

        var element = this.nodeTree.findByPath(path);

        var fld=element.fld;

        var url = "http:

        var popupType = "basic";
        var that=this;

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
            this.vjDS["ds" + this.container].reload("static:
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

        var clickCount = fld.is_multi_fg  ? "-" : undefined;

        var tableDS = null;
        if (popupType == "explorer") {
            clickCount = 0;
            gModalCallback = "true;";

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
            if(!this.popObjectViewer['urlValues']){  this.popObjectViewer['urlValues'] = {} }
            this.popObjectViewer['urlValues']['type'] = constraint_data
        } else if (fld.constraint == "search" || fld.constraint == "search+") {
            var url=constraint_data.url;
            if (constraint_data.qryLang && popupType == "explorer") {
                this.popObjectViewer.qryLang = true;
            }
            if (!url) {
                url = "static:
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
            if(cur_panel)cur_panel.hidden = false;

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
            url=evalVars(url, "$(", ")", this );
            this.vjDS["ds" + this.container].reload(url, true);
        }
        else if (fld.constraint == 'choice' || fld.constraint == 'choice+') {
            cur_table.iconSize = 0;
            url = "static:
                return s + "\n" + quoteForCSV(c.description) + "," + quoteForCSV(c.value); 
            }, "");
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

        this.parsePopupList(list, node);
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
    
    this.parsePopupList = function (list, node) {
        var element=this.nodeTree.findByPath(this.currentPopupElement.path);
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

         this.changeElementValueByPath(this.currentPopupElement.path, whatToPass, 0, true, true, undefined, undefined, node);

         var res=0;
         if(this.currentPopupElement.fld.selectCallback){
             res=funcLink(this.currentPopupElement.fld.selectCallback, viewer, undefined , this);
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
                if (element.fld.children.length!=0){
                    this.constructInfrastructure(element, element.fld.children[0].name, false, 1);
                    element.children[element.children.length - 1].value = node[nodeProp];
                }
            }
            else {
                var deleteRow;
                for (var i = 0; i < element.children.length; i++) {
                    if (element.children[i].value == node[nodeProp]) deleteRow = element.children[i].row;
                }
                this.onDelElement(this.objCls, element.path, deleteRow, 1, true, true);
            }
        }

        var list = viewer.accumulate("node.checked", whatToAccumulate);

        var elementValue;
        elementValue = list;
        var allList = viewer.accumulate("1", whatToAccumulate).join(";");
        element.idList = allList;
        this.changeElementValueByPath(element.path, elementValue, 0, true, true, false, donotredraw);
        if(element.fld.checkCallback)
            return funcLink(element.fld.checkCallback, viewer, node, this);
    };

    this.onClosepop=function(force)
    {
        if (gKeyCtrl == 0 || force) {
            gModalClose(this.myFloaterName+"Div");
            this.currentPopupElement = null;
        }
        return 1;
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
            var preserveValues = viewer.accumulate("node.selected!=1", this.elementValueAccumulator(element));
            if (preserveValues && preserveValues.length == viewer.dim()) {
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
        element.children = newchld;
        if (!Notredraw) this.redraw();
    };

    this.onClearContent = function (container, path) {
        this.changeElementValueByPath(path,"", 0, true);
    };

    this.onChangeSelectValue=function (container, path , selbox)
    {
        var element=this.nodeTree.findByPath(path);
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

        if( element.fld.type=="bool")element.value=textbox.checked ? 1 : 0;
        else if(element.fld.type=='file'){
            var o=gObject(this.RVtag+"-"+element.name+'-input');
            if(o){
                element.inputNode = o;
            }
        }else if(element.fld.type=="datetime"){
            var val = textbox.value;
            var date = new Date(val);
            if(!isNaN(date.getDate())){
                var fDate = date.getFullYear() + "/" + (date.getMonth()+1 < 10 ? ("0"+(date.getMonth()+1)) : (date.getMonth()+1)) + "/" + 
                        (date.getDate() < 10 ? ("0"+date.getDate()):date.getDate()) + " " + (date.getHours()<10?("0"+date.getHours()):date.getHours()) + 
                        ":" + (date.getMinutes()<10?("0"+date.getMinutes()):date.getMinutes()) + ":" + 
                        (date.getSeconds()<10?("0"+date.getSeconds()):date.getSeconds());
                textbox.value = fDate;
                element.value = fDate;
            }
            else{
                element.value = textbox.value;
            }
        }else{
            element.value=textbox.value;
        }
        
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
                this.previousHelp=o.id;
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

            var o=gObject(elname+'-'+this.RVtag+"-controls");
            if(!o){
                elname=this.elementName(element.parent)+"."+element.row;
                o=gObject(elname+'-'+this.RVtag+"-controls");
            }

            return this.showControlsByElname(container, elname, showorhide);
        }

    };

    this.showControlsByElname = function(container, elname, showorhide) {
        var o = gObject(elname+'-'+this.RVtag+"-controls");

        if(isok(this.previousControl) && this.previousControl != elname+'-'+this.RVtag+"-controls" ){
            var v = gObject(this.previousControl);
            if (v) v.className = "sectHid";
            this.previousControl = "";
        }

        if (o) {
            o.className = showorhide ? "sectVis" : "sectHid";
            this.previousControl = elname+'-'+this.RVtag+"-controls";
        }
    };


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
        var fld = element.fld;
        var elname=sanitizeElementId(this.elementName(element));

        var tTit="";

        if (this.wantCollapseElement(element))
            hidetitle = true;

        if(fld.hidden) hidetitle=true;


        if(fld.icon)  {
            tTit+="<td><img src='img/"+icon+"' /></td>";
        }

        if (!hidetitle) {
            if(element.children.length>0 )  {
                tTit += "<td id='" + elname + "-group'><small>";
                tTit+= "<a href='javascript:vjObjEvent(\"onClickExpand\",\""+ this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")'>";
                tTit+="<span id='"+elname+"-collapser'>"+( element.expanded>=this.expansion ? this.icons.collapse  : this.icons.expand )+"</span>";
                tTit+="</a>";
                tTit+="</small></td>";
            } else
               tTit+="<td>"+this.icons.itemRow+"</td>";

            tTit += "<td id='" + elname + "-title' >";
                 var usetitle = element.title ? element.title : fld.title;
                tTit+=usetitle;
                tTit+="</td>";
        }



        if ( (fld.type == "list" && !fld.constraint) || fld.type=="array") {

            
        } else {

            val=element.value;

            var eventFuncs="onmouseover='vjObjEvent(\"onMouseOver\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" , 1);vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
            eventFuncs+="onfocus='vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
            eventFuncs+="onmouseout='vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", \" \"); stopDefault(event);' ";

            tTit += "<td align=left " + (fld.hidden ? "class='sectHid'" : "");

            var eventFuncsWithoutClickNode = eventFuncs;
            if (fld.constraint == 'search'  || (fld.constraint == "type") || (fld.constraint == 'choice' || fld.constraint == 'choice+' && !fld.is_readonly_fg && !fld.is_hidden_fg)) {
                eventFuncs+="onClick='vjObjEvent(\"onClickNode\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", \" \"); stopDefault(event);' ";
            }
            tTit+=">";


            if(fld.is_readonly_fg || this.readonlyMode){
                eventFuncs = '';
                eventFuncsWithoutClickNode = '';
            }

            if ((fld.constraint == 'choice' ) && (fld.is_readonly_fg || fld.is_hidden_fg)) {
                var chc = fld.constraint_data.split("|");
                tTit+="<select" + this.fieldDescriptionTitle(fld) + " ";
                tTit+=eventFuncs;
                tTit+="onchange='vjObjEvent(\"onChangeSelectValue\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", this )' ";
                if(fld.is_readonly_fg)tTit+=" readonly='readonly' disabled='disabled' ";
                tTit+="class='"+this.inputClass+(fld.is_readonly_fg?"ReadOnly":"")+"' type='select' name='"+elname+"' >";
                for( var ic=0;ic<chc.length; ++ic)  {
                    var farr=chc[ic].split("
                    tTit+="<option value='"+sanitizeElementAttr(farr[0])+"' "+(farr[0]==val ? "selected" : "")+" >"+(farr.length>1 ? farr[1] : farr[0]) +"</option>";
                }
                if(!chc.length || !chc[0].length){
                    tTit+="<option value='"+sanitizeElementAttr(val)+"' selected >"+sanitizeInnerHTML(val) +"</option>";
                }
                tTit+="</select>";

            } else {

                if (fld.type == "datetime") {
                    if(val != ""){
                        var tmp = formatToDate(val);
                  
                        if(!isNaN(tmp.getDate()))
                            val = tmp.getFullYear() + "/" + (tmp.getMonth()+1 < 10 ? ("0"+(tmp.getMonth()+1)) : (tmp.getMonth()+1)) + "/" + 
                                (tmp.getDate() < 10 ? ("0"+tmp.getDate()):tmp.getDate()) + " " + (tmp.getHours()<10?("0"+tmp.getHours()):tmp.getHours()) + 
                                ":" + (tmp.getMinutes()<10?("0"+tmp.getMinutes()):tmp.getMinutes()) + ":" + 
                                (tmp.getSeconds()<10?("0"+tmp.getSeconds()):tmp.getSeconds());
                    }
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
                if(fld.is_disabled_fg){
                    myType = this.inputClass;
                    myTags += " disabled='true' ";
                }
                else myType = this.inputClass;

                myTags+="class='"+myType+"' "+ (this.textsize ? ("size="+this.textsize+"") : "") +" ";
                if (fld.type == "bool") myTags += "type='checkbox' ";
                else if (fld.type == 'password') {
                    myTags += "type='password' ";
                }
                else if(fld.type=="file"){
                    myTags+=" type='file'";
                    if( fld.is_multi_fg ) {
                        myTags += " multiple ";
                    }
                }
                else if(fld.type=="date"){
                    myTags+=" type='date'";
                }
                else if(fld.type=="datetime"){
                    myTags+=" type='string'";
                }
                else myTags += "type='text' ";
                myTags+="name='"+elname+"'" + this.fieldDescriptionTitle(fld);
                var oo = gObject(this.RVtag + "-" + fld.name + "-TEMPLATE");

                if(oo && oo.innerHTML){
                    tTit+=oo.innerHTML.replace("%"+this.RVtag+"-INTERFACE-TAGS%",myTags).replace("%"+this.RVtag.toLowerCase()+"-interface-tags%",myTags).replace("%"+this.RVtag+"-INTERFACE-VALUE%",sanitizeInnerHTML(val));
                }
                else if (fld.constraint == 'type' || fld.constraint == 'search' ) {
                    var myRecordVewer = elname + "Viewer"+'-'+this.RVtag;

                    if(fld.doNotCollapseMultiValueViewers || !element.parent.childrenOfTypeViewerAssociatedWithElement)
                        element.parent.childrenOfTypeViewerAssociatedWithElement=new Object();

                    if( !element.parent.childrenOfTypeViewerAssociatedWithElement[fld.name] ){
                        element.parent.childrenOfTypeViewerAssociatedWithElement[fld.name]=element;
                        element.viewerAssociated = myRecordVewer;

                    }

                    if(element.viewerAssociated ){
                        tTit += "<table " + (this.classChoiceStyle ? "class='" + this.classChoiceStyle + "'" : "") + this.fieldDescriptionTitle(fld) + "><tr><td><input " + (fld.constraint == "search+" ? "" : "class='sectHid'");
                        tTit+=myTags;
                        tTit += " value='" + sanitizeElementAttr(val) + "' ";
                        tTit += "/></td><td><span  " + (element.fld.is_multi_fg ? eventFuncsWithoutClickNode : eventFuncs) + "  id= \"" + myRecordVewer + "\"></span></td>";
                        if(!this.readonlyMode && !fld.is_readonly_fg )

                            tTit+="<td style='vertical-align:top;'><button class='linker' type='button' " + eventFuncs + "><img src='img/combobox.gif'></button></td>";

                        tTit +=    "</tr></table>";
                        if(fld.constraint_data=='image' || fld.constraint_data=='system-image'){
                                 tTit += "<table " + (this.classSearchStyle ? "class='" + this.classSearchStyle + "'" : "") + "><tr><td>" ;
                                 tTit += "<div id='"+this.RVtag+"-"+elname+"-imageControl'>" ;
                                 element.myTags = myTags;
                                 tTit +=    "</div></td></tr></table>";
                        }
                    }

                }else if((fld.constraint == 'choice' ||fld.constraint == 'choice+') && !fld.is_readonly_fg && !fld.is_hidden_fg){
                    var chc = fld.constraint_data.split("|");
                    tTit += "<table " + (fld.constraint == "choice+" ? "class='" + this.classSearchStyle + "'" : "class='" + this.classChoiceStyle + "'") + this.fieldDescriptionTitle(fld) + "><tr><td><input " + (fld.constraint == "choice+" ? "" : "class='sectHid'");
                    tTit += myTags;
                    tTit += " value='" + sanitizeElementAttr(val) + "' ";
                    
                    var myRecordVewer = elname + "Viewer"+'-'+this.RVtag;
                    element.viewerAssociated = myRecordVewer;
                    element.choiceOption = [];
                    var tagForSpan = ((fld.constraint == 'choice+') ? 'class=sectHid' : '');
                    
                    for (var ic = 0; ic < chc.length; ++ic, ir++) {
                        var farr = chc[ic].split("
                        if (farr.length == 1) {
                            farr.push(farr[0]);
                        }
                        element.choiceOption.push({ description: farr[1], value: farr[0] });
                        if (farr[0] == val) { element.defaltValueShow = "description,value\n" + quoteForCSV(farr[1]) + "," + quoteForCSV(farr[0])+"\n"; }
                    }
                    tTit += "/></td><td><span "+tagForSpan+" id=\"" + myRecordVewer + "\" ></span></td>";
                    if(!this.readonlyMode)tTit +="<td><button class='linker' type='button' " + eventFuncs + "><img src='img/combobox.gif' ></button></td>";
                    tTit +="</tr></table>";
                } else if (fld.constraint == 'search+') {
                    tTit += "<table " + (this.classSearchStyle ? "class='" + this.classSearchStyle + "'" : "") + this.fieldDescriptionTitle(fld) + "><tr><td>";
                    if (fld.type == "text" || ("" + val).length > 60) {
                        tTit += "<textarea cols='60' rows='6' "+myTags +">"+sanitizeInnerHTML(val)+"</textarea>";
                    } else {
                        tTit += "<input "+myTags+ " value=\""+sanitizeElementAttr(val)+"\" />";
                    }
                    tTit += "</td>";
                    if(!this.readonlyMode)tTit +="<td ><button class='linker' type='button' " + "onClick='vjObjEvent(\"onClickNode\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\", \" \")' " + eventFuncs + "><img src='img/combobox.gif' align='to: 'top'></button></td>";
                    tTit +="</tr></table>";

                }




                else {
                    if(fld.type=="text") {
                        tTit+="<textarea cols='60' rows='6' ";
                        tTit+=myTags;
                        tTit+=" >";
                        tTit+=sanitizeInnerHTML(val);
                        tTit+="</textarea>";
                    }
                    else {

                        tTit+="<input ";
                            tTit+=myTags;
                            if( fld.type=="bool"){
                                tTit+=(parseBool(element.value) ? " checked " : "") ;
                            }
                            else{
                                tTit+=" value=\""+sanitizeElementAttr(val)+"\" ";
                            }

                        tTit+=" />";
                    }

                }

            }
            if(fld.link_url){

                tTit+="<td><a href=javascript:vjObjEvent(\"onClickUrlLink\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")>"+this.icons.urlJump+"</a></td>";
                
            }
            tTit+="</td>";

        }



        if (!hidetitle ) {

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



        var t="";
        if(tTit) {
            t += "<table id='"+elname+"-wrapper' class='"+(fld.hidden ? this.tblClass+"_table" : this.tblClass+"_table")+"'";
            if (!fld.is_readonly_fg && !this.readonlyMode) {
                t += "onmouseover='vjObjEvent(\"onMouseOver\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" , 1)'";
            }
            t += "><tr>";
            t += tTit;
            t += "</tr>";
            t += "</table>";
        }




        var tGrp = "";
        if( fld.type=="array" ) {
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

            let titles;
            if(!hidetitle){
                let list = '';

                for( var i=0; i< fld.children.length; ++i) {
                    list += `<th ${fld.children[i].hidden ?  "class='sectHid'" : ""}>`;

                    list += subRows.length > 0 && element.children[subRows[0][ fld.children[i].name] ].title 
                            ? element.children[subRows[0][ fld.children[i].name] ].title
                            : fld.children[i].title;

                    list +="</th>";
                }

                titles = `<td width=${this.td_width}></td> ${list} <td>${this.icons.white}</td>`;
            }

            let tableClass = element.expanded >= this.expansion  ?  `${this.tblClass}_table ${this.tblClass}_array` : "sectHid"
            tGrp += `   <table 
                            class='${ tableClass }' 
                            id='${elname}-children' 
                            border=1
                        >
                          <tr> 
                            ${titles ? titles : ''}
                          </tr>
                    `;

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
                if(!hidetitle ||fld.is_multi_fg ) tGrp += `<td width=${this.td_width}></td>`;
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
                tGrp+="</td>";
                tGrp+="</tr>";
            }

            if((fld.children[0] && (element.children.length<fld.children[0].maxEl*fld.children.length)) && !this.readonlyMode && !this.editExistingOnly){
                tGrp += `<tr class='${this.tblClas}_interface'>
                            <td width=${this.td_width}></td>
                            <td><button class='linker' type='button' onclick='javascript:vjObjEvent("onAddArrayRow","${this.objCls}","${sanitizeElementAttrJS(element.path)}"); stopDefault(event);' ${this.fieldAddTitleTitle(fld)}> ${this.icons.addRowMore}&nbsp;${fld.title}</button></td>
                        </tr>
                        `;
            }

            tGrp+="</table>";

        } else if (fld.type=="list" || fld.type == "array" || ((fld.type == "string" && ((fld.constraint=="search") || (fld.constraint == "type")) ))) {
            tGrp += "<table class='" + this.tblClass + "_table" + "' id='" + elname + "-children' border=0>";

            for (var il = 0; il < fld.children.length; ++il) {

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
                    if(!hidetitle && !fld.children[il].div) tList+=`<td width=${this.td_width}></td>`;
                    if(element.children[ie].hidden) tList+="<td class='sectHid'>";

                    else tList+="<td>";
                    cntC++;

                    tList += this.generateText(element.children[ie], false, true);
                    tList += "</td>";
                    rowCtrl = element.children[ie].row;
                    tList += "</tr>";
                    ++thiskind;

                }

                var doPlusSign = ( element.childrenOfTypeViewerAssociatedWithElement && element.childrenOfTypeViewerAssociatedWithElement[fld.children[il].name] ) ? false : true ;

                if(thiskind<fld.children[il].maxEl && !this.readonlyMode && !this.editExistingOnly && doPlusSign ) {
                    if(!hidetitle && !fld.children[il].div) {
                        tList+=`<tr><td width=${this.td_width}></td><td>`;
                    }
                    if (thiskind) {
                        tList += `<table class='${this.tblClass}_interface'><tr><td width=${this.td_width}></td><td>`;
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
                    
                    var showOrNot=true;
                    if (fld.children[il].is_readonly_fg) {
                        if (this.showReadonlyInNonReadonlyMode) {
                            if (!thiskind) showOrNot = false;
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





        t+=tGrp;

        fld.innerTxt=t;
        if(!prohibitLayering && fld.div)return '';
        return fld.innerTxt;
    };

}


