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
    if(!this.myFloaterName)this.myFloaterName="floater-"+this.objCls;
    
this.escapeCaptured=false;

    if(this.autoResizeFields===undefined)
        this.autoResizeFields=true;
    if(!this.formObject && this.formName)
        this.formObject=document.forms[this.formName];    
    if(!this.maxInputWidth)this.maxInputWidth=400;
    if(!this.divResizer)this.divResizer=this.objCls+"-basic-resizer";
    if(!this.minInputWidth)this.minInputWidth=32;
    if(!this.tblClass) this.tblClass="REC";
    if(!this.formProviderDoc)this.formProviderDoc=document;
    if(!this.inputClass)this.inputClass=this.tblClass+"_input";
    if(!this.expansion)this.expansion=1;
    if(typeof(this.autoexpand)==="undefined")this.autoexpand=1;
    if(!this.textsize)this.textsize=128;
    if(!this.autoStatus)this.autoStatus=0;
    if(!this.objType)this.objType="0";
    if(!this.hiveId)this.hiveId="0";
    if(!this.propFormatId) this.propFormatId = parseHiveId(this.hiveId).objId ? this.hiveId : this.objType;
    if(!this.RVtag)this.RVtag="RV";
    if(!this.cmdPropSet)this.cmdPropSet="?cmd=propset&ppo=*";
    if (this.accumulateWithNonModified === undefined) this.accumulateWithNonModified=true;
    if(!this.accumulateWithoutHidden)this.accumulateWithoutHidden=false;
    if(!this.showReadonlyInNonReadonlyMode)this.showReadonlyInNonReadonlyMode=false;
    if(!this.readonlyMode)this.readonlyMode=false;
    if (!this.vjDV) this.vjDV = vjDV;
    if (!this.constructAllNonOptionField) this.constructAllNonOptionField = false;
    if (!this.constructAllField) this.constructAllField = false;
    if (!this.implementTreeButton) this.implementTreeButton = false;
    if (!this.implementSaveButton) this.implementSaveButton = false;
    if (!this.implementCopyButton) this.implementCopyButton = false;
    if (!this.implementSetStatusButton) this.implementSetStatusButton = false;
    if (!this.maximizeModal) this.maximizeModal = true;
    if (!this.showReadonlyWhenValueThere) this.showReadonlyWhenValueThere = false;
    if(!this.recordEditingCommand)this.recordEditingCommand="record";
    if(!this.gModalOpen)this.gModalOpen=gModalOpen;
    if(!this.gModalClose)this.gModalClose=gModalClose;
    if(!this.gCreateFloatingDiv)this.gCreateFloatingDiv=gCreateFloatingDiv;
    if(!this.specialFloater)this.specialFloater="";
    if(!this.pictureWidth)this.pictureWidth=300;
    if(!this.ChemDoodleSize)this.ChemDoodleSize={x:300,y:300};
    if(!this.model3DSize)this.model3DSize={x:300,y:300};
    if(!this.rectPopup) {
        this.rectPopup={    x:"0.23%",y:"0.23%",cx:"0.54%",cy:"0.50%" };
    }
    if(!this.rectPopupExpl) {
        this.rectPopupExpl={    x:"0.03%",y:"0.03%",cx:"0.94%",cy:"0.90%" };
    }
    if(this.rectPopup){
        if((""+this.rectPopup.x.indexOf("%"))!=-1)this.rectPopup.x=parseFloat(this.rectPopup.x)*gPgW;
        if((""+this.rectPopup.y.indexOf("%"))!=-1)this.rectPopup.y=parseFloat(this.rectPopup.y)*gPgW;
        if((""+this.rectPopup.cx.indexOf("%"))!=-1)this.rectPopup.cx=parseFloat(this.rectPopup.cx)*gPgW;
        if((""+this.rectPopup.cy.indexOf("%"))!=-1)this.rectPopup.cy=parseFloat(this.rectPopup.cy)*gPgH;
    }
    if(this.rectPopupExpl){
        if((""+this.rectPopupExpl.x.indexOf("%"))!=-1)this.rectPopupExpl.x=parseFloat(this.rectPopupExpl.x)*gPgW;
        if((""+this.rectPopupExpl.y.indexOf("%"))!=-1)this.rectPopupExpl.y=parseFloat(this.rectPopupExpl.y)*gPgW;
        if((""+this.rectPopupExpl.cx.indexOf("%"))!=-1)this.rectPopupExpl.cx=parseFloat(this.rectPopupExpl.cx)*gPgW;
        if((""+this.rectPopupExpl.cy.indexOf("%"))!=-1)this.rectPopupExpl.cy=parseFloat(this.rectPopupExpl.cy)*gPgH;
    }
            
    if (!this.getValidateSeparatorCb) this.getValidateSeparatorCb = function(fld_name) { return null; };

    this._evalKeys = ["$(","$_("];

    if(!this.vjDS)this.vjDS=vjDS;

    if(!this.icons)this.icons={
        collapse: "<img style='width:18px;padding:2px;transform:rotate(90deg);-web-kit-transofrm(90deg);opacity:0.5' src='img/recExpand.gif' title='collapse' />" ,
        expand: "<img style='width:18px;padding:2px;' src='img/recExpand.gif' title='collapse' />" ,
        help: "<img border=0 width=16 height=16 src='img/recQuestion.gif' title='help' />" ,
        setNow: "<img border=0 width=16 height=16 src='img/clock-icon.gif' title='help' />" ,
        requiredField: "<font color='lightgreen'>&#x2731;</font>" ,
        lock: "<img border=0 width=16 height=16 src='img/recLock.gif' title='the field cannot be modified' />" ,
        search: "<img border=0 width=16 height=16 src='img/recSearch.gif' title='lookup possible values' />" ,
        link: "<img border=0 width=16 height=16 src='img/recLink.gif' title='jump to appropriate internet location' />" ,
        delRow: "<img border=0 width=16 height=16 src='img/closeRed.svg' title='delete the specified element' />",
        clearRow: "<img border=0 width=16 height=16 src='img/delete.gif' title='clear the content' />",
        addRow: "<big>&#x271A;&nbsp;</big>",
        addRowMore: "<big>&#x271A;&nbsp;</big>",
        revertRow: "<img border=0 width=16 height=16 src='img/redoLightBlue.svg' title='discard modifications' />" ,
        setRow: "<img border=0 width=16 height=16 src='img/recSet.gif' title='save the modifications' />" ,
        itemRow: "",
        errorRow: "<font color='red'>&#x2731;</font>" ,
        notEmpty:"<img border=0 width=16 height=16 src='img/recNotEmpty.gif'  title='field cannot be empty'/>" ,
        white:"<img border=0 width=16 height=16 src='img/white.gif'  title='white img taken one img place'/>" ,
        urlJump: "<img border=0 width=16 height=16 src='img/recLink.gif' title='jump to the external website' />",
        barcode: "<img border=0 width=16 height=16 src='img/barcodeGenerate.gif' title='click to scan barcode' />",
        photo: "<img border=0 width=32 height=32 src='img/photo.jpg' title='click to take a picture' />",
        comboDown: "&#x25BC;",
        checkOn: "<img border=0 width=20 height=20 src='img/checkmark.gif' class='"+this.inputClass+"_checkbox' />",
        checkOff: "<img border=0 width=20 height=20 src='img/white.gif' class='"+this.inputClass+"_checkbox' />"
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
    
    
    this.setNow=function(c,path) 
    {
        var val="";
        var today = new Date();
        var element=this.nodeTree.findByPath(path);

        if (element.fld.type == "date") {
            val= today.getFullYear() + '-' + ('0' + (today.getMonth() + 1)).slice(-2) + '-' + ('0' + today.getDate()).slice(-2);
        }
        else if (element.fld.type == "datetime") {
            val= today.toISOString().slice(0,16);
        }
        this.changeElementValueByPath(element.path, val, 0, true, false, false, false);

    }
    this.defaultPropSpecFilter = function(tbl) {
        var remove_rows_index = []
        for (var ir=0; ir<tbl.rows.length; ir++) {
            if( tbl.rows[ir].name[0] == "_" ) {
                remove_rows_index.push(ir);
            }
        }
        while( remove_rows_index.length) {
            tbl.deleteRows(remove_rows_index.pop());
        }
    };
    if (!this.propSpecFilter) {
        this.propSpecFilter = this.defaultPropSpecFilter;
    }

    this.composerFunction=function( viewer, content )
    {
        if(!this.formObject && this.formName)
            this.formObject = document.forms[this.formName];
    
        
        this.div=document.getElementById(this.div.id);
        
        var want_new_fldTree = this.force_fld_load || !this.fldTree || this.fldTree.t_sessionID !== this.getDataSessionID(0);
        var want_new_nodeTree = this.force_node_load || !this.nodeTree || this.nodeTree.t_sessionID !== this.getDataSessionID(1);

        if (want_new_fldTree) {
            var tbl = new vjTable(this.getData(0).data, 0, vjTable_propCSV);
            if(this.rows)
                tbl.customizeRows(this.rows);
            this._id_type=0;
            for ( var ir=0; ir<tbl.rows.length; ++ir) {
                if(tbl.rows[ir].name=="_id") {
                    this._id_type=tbl.rows[ir].title;
                    break;
                }
            }

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
            this.fldTree.root.children.splice(0,0,{ type_id:-1, default_value: this.objType, value: this.objType, name: this.spcTypeObj, title: 'ObjectType', type: '', hidden: true, parent:this.nodeTree.root , is_key_fg:1, is_readonly:1, is_readonly_fg:-1, is_optional:0 , is_multi_fg: 0, is_hidden_fg: 0,  description: 'Determines the type of the object' , children: new Array()});
            if(this.fldPresets)
                this.setFields(this.fldPresets);
            
            if(this.listSections) {
                if(this.listSections.length==0){
                    this.listSections.push({tag:"root",title:"Summary", arr: new Array ("root") , cnt:0 });
                }
                     
                this.fldTree.enumerate(
                    function(params,node){
                        var thiSS=params.obj; 
                        if(!node.order )
                            return;
                        var pos=node.order.lastIndexOf(".");
                        if( pos ==-1 )
                            return ;
                        
                        var s=node.order.substring(pos+1);
                        if( isk(parseInt(s)) ) 
                            return ;
                        
                        var i;
                        for( i=0;i<thiSS.listSections.length; ++i) {
                            if(thiSS.listSections[i].tag==s)break;
                        }
                        if(i==thiSS.listSections.length) { 
                            node.sectionIn=thiSS.listSections.length+1;
                            thiSS.listSections.push({tag:s,arr:new Array(),cntd:0});
                        }                        
                        thiSS.listSections[i].arr.push(node.name);
                        
                    },
                    {obj:this}
                )
                
                if(this.sectionAutomatically) {
                    var se="";
                    this.firstSectionToSelect=0 ;
                    for ( var v=0;v<this.listSections.length; ++v) {
                        if( this.listSections[v].cnt==0 ) {
                            this.listSections[v].noButton=true;
                            continue;
                        }
                        if(!this.firstSectionToSelect)
                            this.firstSectionToSelect=v+1;
                        var te="";
                        var o=document.getElementById(this.RVtag+"-"+this.listSections[v].tag);
                        
                        for ( var ii=0;ii<this.listSections[v].arr.length; ++ii) {
                            var id=this.RVtag+"-"+this.listSections[v].arr[ii];
                            te+="<span id='"+id+"' class='"+( (v && !o) ? 'sectHid' : 'sectVis')+"' ></span>";
                        }
                        
                        if(o){ o.innerHTML=te; }
                        if(v && o){ this.listSections[v].noButton=true;}
                        else se+=te;
                    }


                    this.div.innerHTML=se;
                }

            }
            
            this.fldTree.enumerate(
                function(params, node) {
                    node.is_optional_fg = node.is_optional_fg ? parseIBool(node.is_optional_fg) : 0;
                    node.elname = params.obj.RVtag+"-"+node.name;
                    node.div = params.document.getElementById( node.elname );
                    node.dsSource = params.obj.dataSourceEngine["ds"+params.obj.RVtag+"-"+node.name];
                    var showNode=true;
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

                    if (node.default_value && node.default_value.indexOf("eval:")==0) {
                        node.default_value= eval(node.default_value.substr(5));
                    }
                    
                    if (node.type == "bool") {
                        if (node.default_value && node.default_value.length) {
                            node.default_value = parseBool(node.default_value) ? '1' : '0';
                        } else {
                            node.default_value = '';
                        }
                    }
                    else if (node.type == "date") {
                        if (node.default_value=="$now") {
                            var today = new Date();
                            node.default_value= today.getFullYear() + '-' + ('0' + (today.getMonth() + 1)).slice(-2) + '-' + ('0' + today.getDate()).slice(-2);
                        }
                    }
                    else if (node.type == "datetime") {
                        if (node.default_value=="$now") {
                            var today = new Date();
                            node.default_value= today.toISOString().slice(0,16);
                        }
                    }
                    node.is_hidden_fg=node.is_hidden_fg ? parseIBool(node.is_hidden_fg) : 0 ;
                    node.is_batch_fg=node.is_batch_fg ? parseIBool(node.is_batch_fg) : 0 ;
                    if(node.name!="batch_param" && node.constraint && (!node.constraint_data || node.constraint_data.length==0))
                        {delete node.constraint; delete node.sontraint_data;}
                    if(node.is_hidden_fg) node.hidden = true;
                    node.is_multi_fg = node.is_multi_fg ? parseIBool(node.is_multi_fg) : 0;
                    node.maxEl = node.is_multi_fg ? 0x7FFFFFFF : 1;
                    if(node.parent && node.parent.children && node.parent.children.length>1 && node.parent.type=="array") {
                        node.doNotCollapseMultiValueViewers=true;
                        node.is_multi_fg=false;
                    }
                    var carr=(node.constraint_data && node.constraint_data.length) ? node.constraint_data.split("\n>") : [];
                    if(carr.length)node.constraint_data="";
                    for(var i=0; i<carr.length;++i){
                        if(carr[i].indexOf("is_hidden_fg=")==0)node.is_hidden_fg="eval:"+carr[i].substr(13);
                        else if(carr[i].indexOf("=")==0)node.set_auto="eval:"+carr[i].substr(1);
                        else {
                            if(node.constraint_data.length)node.constraint_data+="\n";
                            node.constraint_data+=carr[i];
                        }
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
            this.enumerate(function(params,node){if(node.fld.is_readonly_fg && node.fld.type!='list' && node.fld.type!='array' && !node.value){node.fld.hidden = true;}});
        }
        this.enumerate(function(paramns,node){node.cntChildren0=node.children ? node.children.length :0;});
        this.enumerate(function(paramns,node){node.warnings=node.warnings ? parseInt(node.warnings) : 0 ;});

        this.enumerate(function(params, node) {
            if(!node.fld.order || parseFloat(node.fld.order).toString()=='NaN') {
                node.fld.order = +1000000;
            }
        });
        this.enumerate(function(params, node)
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


        this.enumerate(function(params, node) {
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
        });

        this.enumerate( function(params,node){
            params.fixExpansionLevel.call(params,node);
        }, this );
        
        this.vjDS.add("Retrieving Objects Metadata Information","ds"+this.container,"static:
        
        if(this.callbackBeforeConstruction)
            funcLink(this.callbackBeforeConstruction, this);
        
        this.fldTree.enumerate( function(params, node) {
            node.div = document.getElementById( node.elname );
        });
        
        this.redraw();
        if(this.fromDocLoc)
            this.readFromDocLoc(this.fromDocLoc);
        if(!gObject(this.myFloaterName+"Div")) { 
            var tMyFloater=gObject("dvFloatingDiv").outerHTML;
            if(!tMyFloater)tMyFloater="";
            tMyFloater=tMyFloater.replace(/dvFloating/g,this.myFloaterName);
            tMyFloater=tMyFloater.replace(/href=\"#\" onclick=\".*\return false;\">/g, "href=\"#\" onclick=\"vjObjEvent('onClosepop','"+this.objCls+"'\)return false;\">");
            this.gCreateFloatingDiv(tMyFloater);
        }        
        
        if(this.postPresets){
            this.changeValueList(this.postPresets);
        }
        if(this.firstSectionToSelect)
            this.onSelectSection(1,this.firstSectionToSelect-1);
    };

    this.fixExpansionLevel = function(node) {
        if(node.depth==0 || (this.autoexpand && ( this.autoexpand=='all' || node.depth<=this.autoexpand)))
            node.expanded=this.expansion;else node.expanded=0;
    }

    this.constructPopUpViewer = function(tabname, popupType, hlprFld){
        
        if(!popupType)
            popupType = "basic";

        if(popupType == "basic"){
            this.popObjectPanel = new vjPanelView({
                data:[ "dsVoid", "ds"+this.container],
                formObject: this.formObject,
                iconSize:24,
                rows: [
                    {name:'pager', icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
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
                    formObject: this.formObject,
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
             this.vjDV.add(this.popupViewer, (this.popUpViewerWidth && !isNaN(this.popUpViewerWidth))? parseInt(this.popUpViewerWidth):this.rectPopup.cx,this.rectPopup.cy).frame="notab";
             this.vjDV[this.popupViewer].add("select","table","tab",[ this.popObjectPanel, this.popObjectViewer ]);
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
                 isNShowactions:true,
                 subTablesAttrs : [{    tabname : tabname,
                                       tabico : "folder-apps",
                                       url : { type : "-" , prop:"id,_brief,created" }
                                   }],
                 folders_DV_attributes : {
                     width : (this.rectPopupExpl.cx*0.25),
                     height : (this.rectPopupExpl.cy*0.9-60),
                     hideDV:true
                 },
                 tables_DV_attributes : {
                     width : (this.rectPopupExpl.cx*0.70),
                     height : (this.rectPopupExpl.cy*0.9-60),
                     maxtabs : 20,
                     isok:true
                 },
                 submit_DV_attributes : {
                     width : "100%",
                     height : 60,
                     frame : "notab",
                     isok:true
                 },
                 isSubmitMode:true,
                 autoexpand:0,
                 isNdisplay_Previews:true,
                 defaultActiveTab: (hlprFld.type=="obj" && hlprFld.constraint_data=="image") ? 3 : 0 ,
                 onSubmitObjsCallback: "function:vjObjFunc('onGetPopupList','" + this.objCls + "')",
                 drag:false,
                 isok:true
             });
             this.popObjectViewer=this.popObjectExplorerViewer;
             var explorer=this.popObjectViewer;
             explorer.useCookies=true;
             vjPAGE.silentDownload=true;
             explorer.init();
             explorer.render();
             explorer.load();
        }
    };







    this.redraw=function( elements ,ifFromChange)
    {
        if (!elements) elements = this.nodeTree.root;
        if(!ifFromChange)    ifFromChange = false;
        this.validate(elements);
        this.fldTree.enumerate( function(params,node){node.innerTxt='';} ) ;
        this.enumerate(function(params,node){node.onlyOneTitleForMultiArray='';});


        this.allHiddenControlsText = "";
        var t="";
        if(!gObject(this.divResizer)) { 
            var bb="<span style='visibility:hidden'><span class='"+this.inputClass+"' id='"+this.divResizer+"'></span></span>";
            var rs=document.createElement("SPAN");
            this.formObject.appendChild(rs);
            rs.innerHTML=bb;
        }
        if (this.prefixHTML) t += this.prefixHTML;
        if (this.implementTreeButton) {
            t += "<table border=0 align=topleft class='REC_table'>";

            t += "<tr><td><img onClick='vjObjEvent(\"showAllFunction\",\"" + this.objCls + "\")' src=" + (this.showReadonlyInNonReadonlyMode ? 'img/off_icon.gif' : 'img/on_icon.gif') + " border=0 width=20 height=20 title='show/hide readonly field'/></td>";
            t += "<td><img onClick='vjObjEvent(\"expandAllFunction\",\"" + this.objCls + "\",\"\")' src=" + (!this.nodeTree.root.expanded ? 'img/recExpand.gif' : 'img/recCollapse.gif') + " border=0 width=20 height=20 title='expand/collapse all field'/></td>";
            t += " <td><img onClick='vjObjEvent(\"constructAllFunction\",\"" + this.objCls + "\")'" + (this.constructAllField ? 'class=sectHid' : 'src="img/recAdd.gif" title="show all editable Field show"') + "  border=0 width=16 height=16 /></td></tr>";
            t += "<tr><td>" + (this.showReadonlyInNonReadonlyMode ? 'Hide' : 'Show') + "</td><td>" + (!this.nodeTree.root.expanded ? 'Expand' : 'Collapse') + "</td><td border=0>" + (this.constructAllField ? '' : 'Construct') + "</td></tr></table>";
        }
        
        var tbuttons="";
        if(this.listSections && this.listSections.length>1) {
            for ( var v=0;v<this.listSections.length; ++v) {
                if(this.listSections[v].noButton)continue;
                var nm=(this.listSections[v].title ? this.listSections[v].title : this.listSections[v].tag);
                var id=this.RVtag+"-"+this.listSections[v].tag;
                tbuttons += "<input class='REC_input_container' type='button' id='"+id+"-button' name='section_+" + this.container + "' value='"+nm+"' onclick='vjObjEvent(\"onSelectSection\",\"" + this.objCls + "\","+v+")' />";
            }
            
            var o=gObject(this.RVtag+"_SECTIONS_BUTTONS");
            if(o){o.innerHTML=tbuttons; tbuttons="";}
        }

        
        this.chemDoodleArr=[];
        this.model3DArr=[];
        this.barcodeArr=[];        
        t += this.generateText(elements, this.showRoot ? false : true);

        var hid=parseHiveId(this.hiveId);
        if(tbuttons.length)tbuttons+="<hr/>";
        if (this.implementSaveButton ) tbuttons += "<input class='REC_input_container' type='button' name='save_record+" + this.container + "' value=" + ((hid.modify || hid.objId<=0 )? "CREATE" : "SAVE") + " onclick='vjObjEvent(\"onSetVerification\",\"" + this.objCls + "\",\"/\",\"save\")' />";
        if (this.implementCopyButton && !(hid.modify) && hid.objId>0 ) tbuttons += "<input class='REC_input' type='button' name='copy_record+" + this.container + "' value='COPY' onclick='vjObjEvent(\"onCopySelf\",\"" + this.objCls + "\",\"/\")' />";

        if (this.implementExtraButtons){
            for ( var v=0;v<this.implementExtraButtons.length; ++v) {
                var vv=this.implementExtraButtons[v];
                tbuttons += "<input class='REC_input_container' type='button' value='"+vv.name+"' onclick='vjObjEvent(\"onClickExtraButton\",\""+this.objCls+"\","+v+")' />";
            }
        }
        
        var o=gObject(this.RVtag+"_SPECIAL_BUTTONS");
        if(o)o.innerHTML=tbuttons; else t+=tbuttons;
        
        if (this.noFileErrorText) t += "<span style='color:red' id="+this.RVtag+"_noFileErrorText></span>";
        t += "<span class='sectHid'>" + this.allHiddenControlsText + "</span>";
        if(this.appendHTML)t+=this.appendHTML;
        if(!this.sectionAutomatically || !this.listSections)
            this.div.innerHTML=t;
        else {
            var id=this.RVtag+"-root";
            var v=document.getElementById(id);
            if(!v){
                v=document.createElement("SPAN");
                v.id=id;
            }
            v.innerHTML=t;
            this.div.appendChild(v);
        }
         

        this.setLayoutFields(elements.fld);
        this.constructElementViewers(elements, ifFromChange);
        for(var ic=0; ic<this.chemDoodleArr.length; ++ic) { 
            var canvas=ChemDoodle_constructCanvas(this.chemDoodleArr[ic].id,this.ChemDoodleSize.x,this.ChemDoodleSize.y);
            ChemDoodle_load(canvas, this.chemDoodleArr[ic].value);
        }
        for(var ic=0; ic<this.barcodeArr.length; ++ic) {
            var element=this.nodeTree.findByPath(this.barcodeArr[ic].path);
            if(element && element.value)barcodeGenerate(document.getElementById(this.barcodeArr[ic].id), "code128", element.value,2,30,5);
        }
        
        for(var ic=0; ic<this.model3DArr.length; ++ic) {
            var element=this.nodeTree.findByPath(this.model3DArr[ic].path);
            if(element && element.value)createBabylonScene(this.model3DArr[ic].id, element.value );
        }
        
        this.validate();
        if(this.autoResizeFields) 
            this.enumerate(function ( params, node ) {
                if(node.fld.type!='datetime' && node.fld.type!='date')  
                    params.thiSS.fixElementSize(node);
            },{thiSS:this});        
    };
    
    this.onClickExtraButton=function (obj,index)
    {
        var btn=this.implementExtraButtons[index];
        funcLink(btn.callback,this);
    }
    
    this.onCopySelf=function ()
    {
        document.location=urlExchangeParameter(document.location,"ids","-"+this.hiveId);
    }
    
        
    this.onClickNextButton=function()
    {
        funcLink(this.callbackNextButton, this);
    }
    
    this.onSelectSection=function(c,iv)
    {
        for ( var v=0;v<this.listSections.length; ++v) {
            if(this.listSections[v].noButton)continue;
            for ( var ii=0;ii<this.listSections[v].arr.length; ++ii) {
                var id=this.RVtag+"-"+this.listSections[v].arr[ii];
                document.getElementById(id).className= (v==iv ?'sectVis' : 'sectHid' ) ;
            }
        }
    
    }
    
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
            var haveDirectDataVals = false;
            var hiveIDArr=[];
            var hiveIDFileArr=[];
            for (var ic=0; ic<childrenVals.length; ic++) {
                if((""+childrenVals[ic]).indexOf("data:")==0) { 
                    haveDirectDataVals = true;
                    continue;
                }
                var hiveId = parseHiveId(childrenVals[ic]);
                if (hiveId.domainId || hiveId.objId || hiveId.ionId) {
                    haveChildrenIdVals = true;
                    hiveIDFileArr.push(hiveId.file);
                    hiveIDArr.push(hiveId.objId);
                    continue;
                }
            }

            var url = "";
            
            var current_DS ; 
            if(element.fld.name=='field_include_type' || element.fld.name=='parent' )
                current_DS=this.dataSourceEngine.add("infrastructure: Constructing Object Lists in RecrodViewer", 'ds' + element.fld.name);
            else current_DS = this.dataSourceEngine.add("infrastructure: Constructing Object Lists in RecrodViewer", 'ds' + element.name);
            
            if (element.fld.constraint == 'type') {

                if(haveChildrenIdVals) {
                    var hiveId = parseHiveId(childrenVals[ic]);
                    url =  "http:
                }
                else if(haveDirectDataVals)  { url = "static:
                    for(var ii=0; ii<childrenVals.length ;  ++ii){ if(ii) url+="," ; url+="Inline image "+(ii+1) ; } 
                }
                else url="static:
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
                        }else if(hiveIDArr.length && (""+childrenVals).length ) {
                            url = "" + constraint_data.url
                            url = urlExchangeParameter(url, "val", encodeURIComponent(childrenVals));
                            url = urlExchangeParameter(url, "var", constraint_data.fetch );
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

            current_DS.url = url;
            this.dataViewEngine.add(element.viewerAssociated, 500, 500).frame = 'none';
            this.defaultInlineShow = [{ name: new RegExp(/./), hidden: true }, { name: '_brief', hidden: false, title: 'Description' },{name:'icon', type: 'icon'},{ name: 'description', hidden: false, title: 'Description' },{ name: 'brief', hidden: false, title: 'Summary' }, { name: new RegExp(/Title.*/i), hidden: false, title: 'Title' }];
            var myListViewer=new vjTableView( {
                data: (element.fld.name=='field_include_type' || element.fld.name=='parent') ? "ds"+ element.fld.name : "ds"+element.name,
                formObject: this.formObject,
                bgColors:['#FFFFFF','#F0F3F9'] ,
                isNheader: true,
                defaultEmptyText:" ",
                cols: newInlineList.length?newInlineList:this.defaultInlineShow,
                inclusionObjRegex: hideRowList ,
                selectCallback: element.fld.selectCallback,
                checkCallback: element.fld.checkCallback,
                maxTxtLen:64,
                multiSelect:true,
                appendCols : hiveIDFileArr.length ? [{header: {name: "title"}, cell: 'eval:node.title=(((node.title && node.id.indexOf("/")==-1) ? node.title+node.id : "") +( ( node.id && (p=node.id.indexOf("/"))!=-1)  ? ":"+node.id.substring(p+1) : "" )).substring(0,12) ' }] : null,                
                idFileArr: hiveIDFileArr.length ? hiveIDFileArr : 0,
                inlineStyle: this.readonlyMode ? "REC_inputReadOnly" : "REC_input" , 
                geometry:{ width:'100%',height:'100%'},
                doNotShowRefreshIcon: true,
                iconSize: 0,
                defaultIcon:'rec',

                isok: true
            });

            this.dataViewEngine[element.viewerAssociated].add("details", "table", "tab", [myListViewer]);
            this.dataViewEngine[element.viewerAssociated].render();

            var ddsname=(element.fld.name=='field_include_type' || element.fld.name=='parent') ? "ds"+ element.fld.name : "ds"+element.name;
            if (this.dataSourceEngine[ddsname].hasDataForUrl()) {
                this.dataSourceEngine[ddsname].call_refresh_callbacks();
            } else {
                this.dataSourceEngine[ddsname].load();
            }
            
            if(element.fld.constraint == 'geolocation' ){
                
            }
            if(element.fld.constraint == 'type' && (element.fld.constraint_data=='image' || element.fld.constraint_data=='system-image') && (haveChildrenIdVals || haveDirectDataVals) ){
                var o=gObject(this.RVtag+"-"+element.name+"-imageControl");
                if (o ) {

                    var t = " "
                        
                    for(var vi = 0;vi<childrenVals.length;vi++){
                        
                        if(childrenVals[vi].indexOf("data:")==0) {
                            t+="<img src='"+childrenVals[vi]+"' width='"+this.pictureWidth+"'  />";
                        }else {
                            var hiveId = parseHiveId(childrenVals[vi])
                            if (hiveId.domainId || hiveId.objId || hiveId.ionId) {
                                t+="<img src='?cmdr=objFile&ids="+ hiveId.objId + (hiveId.file ? "&filename="+protectFields(hiveId.file) : "" )+"' "+element.myTags+" width='"+this.pictureWidth+"' />" ;
                            }
                        }
                        t += "<a href='javascript:vjObjEvent(\"multiSelectDelete\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\")'>";
                        t += this.icons.delRow;
                        t += "</a>";
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

        var newdepth = element.depth + 1;
        if (element && element.fld && element.fld.type == "array") {
            newdepth -= 1;
        }
        var newel=new Object( {value: '' , value0:'', fld: fld, row: element.subRow, subRow: 0, obj: element.obj, children: new Array (), depth: newdepth, distance: 1 } );
        
        this.fixExpansionLevel(newel);
        
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
            this.fldTree.enumerate(function(params,node){if(node.is_readonly_fg){node.hidden = true;}});
            this.redraw();
        }
        else {
            this.showReadonlyInNonReadonlyMode = true;
            this.fldTree.enumerate(function(params,node){if(node.is_readonly_fg){node.type='';}});
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
                            if(el.fld.name=='field_include_type' )element.children.push(el);
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
            var fa=constraint_data.fetch.split(",");
            var t="";
            for (var it=0;it<fa.length;++it){
                if(it>0)t+="+' '+";
                t+="node['" + fa[it] + "']";
            }
            return t;
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
                if (element.children.length && element.fld.type!="bool" ) {
                    t += "<td border=0>";
                    t += "<a href='javascript:vjObjEvent(\"onSet\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\")' >" + this.icons.setRow + "</a>";
                    t += "</td>";
                }
            }

            if(element.fld.type!="bool") {
                t+="<td border=0 valign=center >";
                t+="<a href='javascript:vjObjEvent(\"onRevert\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")' >"+this.icons.revertRow+"</a>";
                t+="</td>";
            }



        }

        if (!this.hideErrors && element.errors) {
            t+="<td valign=middle border=0>";
            t += this.elementsErrorText([element]);
            t+="</td>";
        }else if(!element.fld.is_optional_fg && element.fld.type != "list" && element.fld.type != "array" && element.fld.type != "bool"){
            t+="<td valign=middle>"+this.icons.requiredField+"</td>";
        }

        if (t) {
            t = "<table border=0 ><tr>" + t + "</tr></table>";
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
    
    this.findVariableNode=function(node,varname)
    {
        if(node.fld.name==varname)return node;
        var node=node.parent;if(!node)return null;
        var found=this.nodeTree.findByAttribute( "fld", varname , node , "name" )
        if(found)return found;
        return this.findVariableNode(node,varname) 
    }
    
    this.evalRecVars=function(varname,orig,par)
    {
        var node=this.nodeTree.findByPath(par.element.path);
        
        node=this.findVariableNode(node,varname);
        if(!node)return undefined;
        val=node.value;
        if(!val && node.fld.type=="date" || node.fld.type=="datetime")
            return "-";
        if(!val && node.fld.type=="bool")
            return "0";
        if(!val && node.fld.type=="string")
            return "''";
        if(!val && (node.fld.type=="int" || node.fld.type=="integer" || node.fld.type=="real") )
            return 0;
        return val;
        
    }
    
    this.evalStatement=function(element,statement)
    {
        statement=statement.replace(/\$_?\(val\)/g, element.value);
        var res=evalVars(statement, "$(", ")", this, "evalRecVars",{element: element})
        var node=element;
        res=eval(res);
        return res;
    }
    
    this.validate=function ( element, visualize, skip_optionality_validation)
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
            
                this.validate(element.children[ie], visualize);

            if (element.children[ie].errors > 0 && element.children[ie].fld.type == "string") {
                element.errorTooltip = element.children[ie].errorTooltip;
                element.errorText = element.children[ie].errorText;
            }
            element.errors += element.children[ie].errors;
            element.warnings += element.children[ie].warnings;
            element.modifications+=element.children[ie].modifications;
        }
        if (element.fld.is_hidden_fg && (""+element.fld.is_hidden_fg).indexOf("eval:")==0){
            var res=this.evalStatement(element,element.fld.is_hidden_fg.substr(5));
            
            var o=gObject(element.name+"-children");
            if(o){
                if(res)o.className="sectHid";
                else o.className="sectVis";
            }
            o=gObject(element.name+"-wrapper");
            if(o){
                if(res)o.className="sectHid";
                else o.className="sectVis";
            }
            o=gObject(element.name+"-title");
            if(o){
                if(res)o.className="sectHid";
                else o.className="REC_title";
            }
                        
            if(element.fld.sectionIn) {
                var id=this.RVtag+"-"+this.listSections[element.fld.sectionIn-1].tag+"-button";
                var o=gObject(id);
                if(o){
                    if(res)o.className="sectHid";
                    else o.className="REC_input_container";
                }    
            }
            
        }
        if (element.fld.set_auto && (""+element.fld.set_auto).indexOf("eval:")==0){
            var res=this.evalStatement(element,element.fld.set_auto.substr(5));
            this.changeElementValueByPath(element.path, res, 0, false, false, false, false);
            this.fixElementSize(element);
        }
        
        if (!element.fld.is_hidden_fg&&!element.fld.is_readonly_fg && !isok(element.value) && !element.fld.is_optional_fg && (element.fld.type != "list") && (element.fld.type != "array") && (element.fld.type != "bool")) {
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
        if (isok(element.value)  && element.fld.constraint == "eval" && element.fld.constraint_data && element.fld.constraint_data.length) {
            var values = [];
            var sep = this.getValidateSeparatorCb ? this.getValidateSeparatorCb(element.fld.name) : null;
            var thiSS=this;
            (sep ? element.value.split(sep) : [element.value]).forEach(function(value) {
            
                var res=thiSS.evalStatement(element,element.fld.constraint_data);
                if (isok(value) && !res) {
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
                    var oldNode = this.accumulate("(node.fld.name=='" + (element.fld.name) + "') && !node.isCopy", "node");
                    if(oldNode.length==1){
                        if(oldNode[0].hidden ||oldNode[0].overWrite ){
                            oldNode[0].value = element.value;
                            oldNode[0].modifications=1;
                        }
                    }
                }
            }
            else if (!element.hidden){
                if(element.value!=element.value0 && !element.overWrite){
                    element.errors++;
                    element.errorText = "Does not match your old password.";
                }else{
                    element.overWrite = true;
                    var o=gObject(this.RVtag+"-"+element.name+"-input");
                    if(o ){
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
                if (element.fld.name=="root" )
                    doshow = false;
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
                
                if(element.fld.type=='datetime') {
                    var parsedUnixTime=new Date(value).getTime()/1000|0;
                    value=parsedUnixTime;
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

    this.changeElementValueByPath=function (fldPath , eleVal , rownum , dovalidate, doTriggerOnChange, forceConstruct, donotredraw)
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
                if(this.formObject.elements[arr[rownum].name])
                    this.formObject.elements[arr[rownum].name].value = eleVal;
            }


            
            this.onInputKeyUp(this.objCls, fldPath );
        }


        this.constructElementViewers(element, true);
        if(this.autoSaveOnChange)
            this.saveValues(element,true);

        if (dovalidate) {
            this.validate(par, true);
        }

        if(doTriggerOnChange && this.onChangeCallback)
            return funcLink(this.onChangeCallback, this, arr[rownum], this.formObject.elements[arr[rownum].name] );

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

        this.enumerate(function(params, node) {
            if (node.fld.type == 'list')
                node.value = '';
            if (node.isCopy)
                node.value = '';
        });
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

    this.onSetCallback=function(param, text)
    {

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

        if (this.implementSaveButton || this.implementCopyButton) {
            var hiveId = text.slice(text.indexOf("_id=") + 4);
            if(parseHiveId(hiveId).objId > 0) { if(this.reloadAfterSave)funcLink(this.reloadAfterSave,this,hiveId);  else this.defaultSaveReload(hiveId );}
            else if(param.dowhat && param.dowhat=='copy')
                {if(this.reloadAfterSave)funcLink(this.reloadAfterSave,this,"-"+hiveId);  else this.defaultSaveReload("-"+hiveId );}
        }
        if(this.setDoneCallback)
            return funcLink(this.setDoneCallback, this, text );
        for(var iv=0; iv<this.data.length; ++iv) {
            this.getData(iv).reload(null,true);
        }
    };


    this.defaultSaveReload = function (hiveId) {
       newurl = "?cmd="+this.recordEditingCommand+"&ids=" + hiveId + "&types=" + this.objType;
        if(hiveId.charAt(0)!='-')alert("Record Created Successfully");
        else alert("Record Copied Successfully")
        window.location.href = newurl;
    };

    this.onSet=function( container, path, whattodo)
    {
        var element=this.nodeTree.findByPath(path);

        var docontinue=true;
        if(this.setCallback)
            docontinue = funcLink(this.setCallback);
        if(!docontinue)
            return;
        if (!this.noAutoSubmit) {
            this.saveValues(element, true,null,whattodo);
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

    this.onClickNode=function(container, path)
    {

        if(this.currentPopupElement){
            this.onClosepop();
            vjPAGE.silentDownload=false;
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
                this.constructPopUpViewer(undefined, popupType,fld);
            }
            else if(this.popObjectViewer && this.popObjectViewer.refresh)
            setTimeout(function(){that.popObjectViewer.refresh();},150);
        } else if (fld.type == "obj" && (fld.constraint == "search" || fld.constraint == "search+")) {
            popupType = constraint_data.explorer ? "explorer" : "basic";
            if (this.eltOfPopObjectViewer != element ) {
                this.eltOfPopObjectViewer = element;
                this.constructPopUpViewer(undefined, popupType,fld);
            }
            else  if(this.popObjectViewer && this.popObjectViewer.refresh)
                setTimeout(function(){that.popObjectViewer.refresh();},150);
        } else {
            this.vjDS["ds" + this.container].reload("static:
            if (this.eltOfPopObjectViewer != element ) {
                this.eltOfPopObjectViewer = element;
                this.constructPopUpViewer(undefined, popupType,fld);
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
            var el = element;
            var that = this;
            function reload_popup_ds(ds) {
                url = "static:
                    return s + "\n" + quoteForCSV(c.description) + "," + quoteForCSV(c.value); 
                }, "");
                if(cur_panel)cur_panel.hidden = true;
                cur_table.checkable = false;
                cur_table.multiSelect = false;
                cur_table.cols = that.defaultOutlineShow;
                ds.reload(url, true);    
            }
            
            if( element.choiceUrl ) {
                function send_data_to_popup_ds(v,data){
                    el.choiceOption = [];
                    var tbl = new vjTable(data, 0, vjTable_propCSV);
                    tbl.rows.forEach(function(r) {
                        el.choiceOption.push({
                            "value" : r.cols[0],
                            "description" : ((r.cols.length > 1) ? r.cols[1] : r.cols[0])
                        });
                    });
                    return reload_popup_ds(that.vjDS["ds" + that.container]);
                }
                
                if(!this.choice_data_DS) {
                    this.choice_data_DS = this.vjDS.add("Retrieving options","ds" + this.container + "choice",url,send_data_to_popup_ds);
                }

                url = this.computeExpressionAtElement(element.choiceUrl, element);
                this.choice_data_DS.reload(url, true);
                
            } else {
                reload_popup_ds(this.vjDS["ds" + this.container])
            }

        }

        fld.compleURLUsed=url;

        this.currentPopupElement=element;
        
        var rect=document.getElementById(element.name+"-wrapper").getClientRects()[0];
        if ( popupType != "explorer" ) {var r={x:rect.left,y:rect.bottom,cx:rect.right-rect.left,cy:this.rectPopup.cy};}
        else var r= this.rectPopupExpl;
        this.gModalOpen(this.myFloaterName+"Div", gModalCallback , r.x, r.y, clickCount,r.cx,r.cy,200,200);
        
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
        vjPAGE.onImmediateDownloadCallback=null;
    };
    
    this.parsePopupList = function (list) {
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

         this.changeElementValueByPath(this.currentPopupElement.path, whatToPass, 0, true, true);

         var res=0;
         if(this.currentPopupElement.fld.selectCallback){
             res=funcLink(this.currentPopupElement.fld.selectCallback, viewer,node , this);
         }
         this.validate(this.nodeTree.root, true);
         if( !this.currentPopupElement.fld.is_multi_fg )
             {this.onClosepop(true);}
         this.fixElementSize(element);
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
            this.gModalClose(this.myFloaterName+"Div");
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
        this.redraw();

    };

    this.onClickUrlLink=function (container, path , textbox)
    {
        var element=this.nodeTree.findByPath(path);
        var realUrl = element.fld.link_url ? element.fld.link_url.replace(/\$_?\(value\)/g, element.value) : element.value;
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
    
    this.onInputKeyUp=function (container, path )
    {
        var element=this.nodeTree.findByPath(path);
        if(element.fld.constraint=="barcode"){
            var domel=this.formObject.elements[element.name];
            if(domel && domel.value ){
                gObject(element.barid).className="sectVis";
                barcodeGenerate(document.getElementById(element.barid), "code128",domel.value,2,40,5);
            }
            else 
                gObject(element.barid).className="sectHid";
        }
        {
            if(this.autoResizeFields) { 
                this.fixElementSize(element);
            }
        }
    
    }
    
    this.fixElementSize=function(element)
    {
        var domel=this.formObject.elements[element.name];
        if(!domel)return ;
        if(domel.type=="checkbox" || domel.type=="datetime" || domel.type=="date")return;
        var val=domel.value;

        newsize=this.sizeOfInput(val);
        o=document.getElementById(this.RVtag+"-"+ element.name + "-input");
        if(o)o.style.width=newsize+"px";        
    }
    this.sizeOfInput=function (val)
    {
        
        var o = document.getElementById(this.divResizer);
        if(!o)return ;
        if(!val)return this.minInputWidth;
        o.innerHTML=val;
        var valRect=o.getBoundingClientRect();
        var newsize=Math.min( this.maxInputWidth, (valRect.width+40));
        return newsize;
    }
    
    
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
    
    this.onCheckboxClick=function(c,path)
    {
        var element=this.nodeTree.findByPath(path);
        var val = (element.value==1) ? 0 : 1;
        
        this.changeElementValueByPath(element.path, val, 0, true, false, false, false);
        this.redraw();
    }

    this.generateText=function ( element, hidetitle, prohibitLayering)
    {
        var tident = this.ident;
        var fld = element.fld;
        var elname=sanitizeElementId(this.elementName(element));

        var tTit="";


        if (this.wantCollapseElement(element))
            hidetitle = true;

        if(fld.hidden) hidetitle=true;


        if(fld.icon)  {
            tTit+="<td><img src='img/"+icon+"' /></td>";
        }
        
        var descFuncs=" onmouseover='vjObjEvent(\"onMouseOver\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" , 1);vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
        descFuncs+="onfocus='vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
        
        
        
        if (!hidetitle ) {
            var valig= ( element.fld.constraint=="3dmol" || element.fld.constraint=="3dmol" || element.fld.constraint=="barcode" || element.fld.constraint_data=='image' || element.fld.constraint_data=='system-image') ? " style='vertical-align:top;' " : "";
            if(element.children.length>0 && (fld.type=="array" || fld.type=="list"))  {
                tTit += "<td id='" + elname + "-group'><small>";
                tTit+= "<a href='javascript:vjObjEvent(\"onClickExpand\",\""+ this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")'>";
                tTit+="<span id='"+elname+"-collapser' >"+( element.expanded>=this.expansion ? this.icons.collapse  : this.icons.expand );
                tTit+="</span>";
                tTit+="</a>";
                tTit+="</small></td>";
            } else
               tTit+="<td "+ valig +"  >"+this.icons.itemRow+"</td>";

            tTit += "<td class='"+(element.children.length>1 ? "REC_title_section" : "REC_title")+"' id='" + elname + "-title' " + valig + " ";
            tTit+=descFuncs + "onTouchStart='vjObjEvent(\"onElementFocus\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" ); stopDefault(event);' ";
            tTit+=" >";
                var usetitle =element.title;
                if( !usetitle && fld.title==this.objType )
                    usetitle=(fld.title + " " + ( (parseInt(this.hiveId) && this.hiveId!=fld.title) ? this.hiveId : "<b>NEW-OBJECT</b>" ) );
                else usetitle=fld.title ;
                if(usetitle=="
                else usetitle+=":";
                tTit+=usetitle+(tident ? "&nbsp;" : "")
                if(this.debug){
                    tTit+="<small><small><br/>";
                    tTit+= (fld.is_optional_fg?"O":"-")+(fld.is_multi_fg?"M":"-");
                        tTit+=":"+elname+":"+element.path;
                    tTit+="</small></small>";
                }
                tTit+="</td>";

        }



        if (fld.type == "list" && !fld.constraint ) {

        } else if( fld.type=="array"  ) {
            if( element.children.length<fld.maxEl*fld.children.length ) {
                if(this.debug)tTit+="<td><small><small>["+element.children.length+"]"+fld.type+"</small></small></td>";
            }
        } else {

            val=element.value;
            if(fld.type=='datetime') {
                if(val && (""+val).indexOf(":")==-1) {
                    var ival=parseInt(val);
                    var dd=new Date();
                    if(!isNaN(ival))
                        dd.setUTCMilliseconds(ival);
                    val=dd.toISOString().slice(0,-1);
                }
            }

            var eventFuncs=descFuncs;
            eventFuncs+= fld.type=='datetime' ?  " " : " onkeyup='vjObjEvent(\"onInputKeyUp\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" )'" ;

            tTit += "<td align=right " + (fld.hidden ? "class='sectHid'" : "");
            var eventFuncsWithoutClickNode = eventFuncs;
            if (fld.constraint == 'search'  || (fld.constraint == "type") || (fld.constraint == 'choice' || fld.constraint == 'choice+' && !fld.is_readonly_fg && !fld.is_hidden_fg)) {
                eventFuncs+="href='javascript:vjObjEvent(\"onClickNode\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", \" \"); stopDefault(event);' ";
            }
            tTit+=">";


            if(fld.is_readonly_fg || this.readonlyMode){
                eventFuncs = '';
                eventFuncsWithoutClickNode = '';
            }

            if (fld.type == "datetime") {
            }

            var myTags=" ";
            if(fld.type!="text") myTags +=  " id='" +this.RVtag+"-"+ element.name + "-input' ";
            if(fld.constraint == 'choice+') myTags += eventFuncsWithoutClickNode;
            else myTags +=  eventFuncs;
            myTags += "onchange='vjObjEvent(\"onChangeTextValue\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\", this );' ";
            var myType;
            if (fld.is_readonly_fg) {
                myType = this.inputClass + "ReadOnly";
                myTags += " readonly='readonly' ";
            }
            else myType = fld.type=="bool" ? this.inputClass+"_checkbox" : this.inputClass ;

            var myCls=" class='"+myType+"' "+ ((this.textsize && fld.type!="bool") ? ("size="+this.textsize+"") : "") +" ";
            
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
                myTags+=" type='datetime-local'";
            }
            else if(fld.type=="integer"){
                myTags+=" type='string'";
            }
            else if(fld.type=="real"){
                myTags+=" type='string' step='0.1' ";
            }
            else myTags += "type='text' ";
            myTags+="name='"+elname+"'" + this.fieldDescriptionTitle(fld);
            var oo = gObject(this.RVtag + "-" + fld.name + "-TEMPLATE");

            let fld_cdt;
            if(fld.constraint_data ) {
                if(fld.constraint_data.indexOf("eval:")==0) {
                    fld_cdt=this.evalStatement(element,fld.constraint_data.substring(5))
                }else {
                    fld_cdt=fld.constraint_data
                }
            }
            if(!fld_cdt)fld_cdt="";
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
                    
                    tTit += "<table class='"+this.inputClass+"_container'  border=0 "  + this.fieldDescriptionTitle(fld) + "><tr><td>";
                    tTit+="<input " + (fld.constraint == "search+" ? "" : "class='sectHid'");
                    tTit+=myTags;
                    tTit+=myCls;
                    tTit += " value='" + sanitizeElementAttr(val) + "' ";
                    tTit += "/>";

                    tTit+="</td><td><span  " + (element.fld.is_multi_fg ? eventFuncsWithoutClickNode : eventFuncs) + "  id= \"" + myRecordVewer + "\"></span></td>";
                    if(!this.readonlyMode && !fld.is_readonly_fg ) {
                        tTit+="<td style='vertical-align:top;'><a " + eventFuncs + ">";
                        tTit+= this.icons.comboDown;
                        tTit+="</a>";
                        if(fld_cdt=='image' || fld_cdt=='system-image'){
                            tTit+="<a href=javascript:vjObjEvent(\"startPhotoSession\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")>"+this.icons.photo+"</a>";
                        }
                        tTit+="</td>";
                  }
                        

                    tTit +=    "</tr></table>";
                    if(fld_cdt=='image' || fld_cdt=='system-image'){
                             tTit += "<table  border=0 " + (this.classSearchStyle ? "class='" + this.classSearchStyle + "'" : "") + "><tr><td>" ;
                             tTit += "<div id='"+this.RVtag+"-"+elname+"-imageControl'>" ;
                             element.myTags = myTags;
                             tTit +=    "</div></td></tr></table>";
                    }
                    
                }

            }else if( fld.constraint == 'choice' || fld.constraint == 'choice+' ){
                var chc=[], is_url = true;
                try {
                    chc = JSON.parse(fld_cdt);                    
                    if("url" in chc) {
                        chc = chc["url"];
                    }
                } catch(e) {
                    is_url = false
                }
                
                
                if( fld.constraint == 'choice' ) {
                    
                    if(is_url)
                        chc = [];
                    else if (fld_cdt && fld_cdt.length) {
                        if(fld.constraint == 'choice' ) chc=fld_cdt.split("|")  ;
                        if(fld.constraint == '3dmodel' ) chc=fld_cdt.split("|")  ;
                    }
                    
                    if(!this.noAutoSelectValue) { 
                        if( (!element.value || element.value.length==0) &&  (!element.fld.default_value || element.fld.default_value.length==0) && chc.length>0){
                            element.value=element.value0=val=chc[0].split("
                        }
                    }
                    
                    tTit+="<select class='REC_input_container' " + this.fieldDescriptionTitle(fld) + " ";
                    tTit+=eventFuncs;
                    tTit+="onchange='vjObjEvent(\"onChangeSelectValue\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\", this )' ";
                    if(fld.is_readonly_fg)tTit+=" readonly='readonly' disabled='disabled' ";
                    tTit+="class='"+this.inputClass+(fld.is_readonly_fg?"ReadOnly":"")+"' type='select' name='"+elname+"' >";
                    for( var ic=0;ic<chc.length; ++ic)  {
                        var farr=chc[ic].split("
                        tTit+="<option value='"+sanitizeElementAttr(farr[0])+"' "+(farr[0]==val ? "selected" : "")+" >"+(farr.length>1 ? farr[1] : farr[0]) +"</option>";
                    }
                    if(!chc.length ){
                        tTit+="<option value='"+sanitizeElementAttr(val)+"' selected >"+sanitizeInnerHTML(val) +"</option>";
                    }
                    tTit+="</select>";
                } else if( !fld.is_readonly_fg && !fld.is_hidden_fg ) {
                    tTit += "<table border=0 class='"+this.inputClass+"_container' " +  this.fieldDescriptionTitle(fld) + "><tr><td>";
                    tTit+="<input " + (fld.constraint == "choice+" ? "" : "class='sectHid'");
                    tTit += myTags;
                    tTit += myCls;
                    tTit+=" onkeyup='vjObjEvent(\"onInputKeyUp\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" )'";
                    tTit += " value='" + sanitizeElementAttr(val) + "' ";
                    var myRecordVewer = elname + "Viewer"+'-'+this.RVtag;
                    element.viewerAssociated = myRecordVewer;
                    element.choiceOption = [];
                    var tagForSpan = ((fld.constraint == 'choice+') ? 'class=sectHid' : '');
                    

                    if (is_url) {
                        element.choiceUrl = chc;
                    } else {
                        chc=fld_cdt.split("|") 
                        for (var ic = 0; ic < chc.length; ++ic, ir++) {
                            var farr = chc[ic].split("
                            if (farr.length == 1) {
                                farr.push(farr[0]);
                            }
                            element.choiceOption.push({ description: farr[1], value: farr[0] });
                            if (farr[0] == val) { element.defaltValueShow = "description,value\n" + quoteForCSV(farr[1]) + "," + quoteForCSV(farr[0])+"\n"; }
                        }
                    }
                    tTit += "/></td><td><span "+tagForSpan+" id=\"" + myRecordVewer + "\" ></span></td>";
                    if(!this.readonlyMode){
                    tTit +="<td><a " + eventFuncs + ">"
                    tTit+=this.icons.comboDown;
                    tTit+="</a></td>";
                    }
                    tTit +="</tr></table>";    
                }
            } else if (fld.constraint == 'search+') {
                tTit += "<table  border=0 class='"+this.inputClass+"_container'" + this.fieldDescriptionTitle(fld) + "><tr><td>";
                if (fld.type == "text" || ("" + val).length > 60) {
                    tTit += "<textarea cols='60' rows='6' "+myTags;
                    tTit+=">"+sanitizeInnerHTML(val)+"</textarea>";
                } else {
                    tTit += "<input "+myTags+ myCls + " value=\""+sanitizeElementAttr(val);
                    tTit +="\" />";
                }
                tTit += "</td>";
                if(!this.readonlyMode){
                    tTit +="<td ><a " + "href='javascript:vjObjEvent(\"onClickNode\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.path) + "\")' " + eventFuncs + ">";
                    tTit+=this.icons.comboDown;
                    tTit+="</a></td>";
                }
                tTit +="</tr></table>";

            }  

            else {
                if(fld.type=="text") {
                    tTit+="<textarea cols='60' rows='6' ";
                    tTit+=" class='"+this.inputClass+"_container' ";
                    tTit+=myTags;
                    tTit+=" >";
                    tTit+=sanitizeInnerHTML(val);
                    tTit+="</textarea>";
                }
                
                else {
                    if( fld.type=="bool") {
                        tTit+="<a";
                        tTit+=" href=javascript:vjObjEvent(\"onCheckboxClick\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\") ";
                        tTit+=eventFuncs;
                        tTit+=">";
                        tTit+=element.value==1 ? this.icons.checkOn : this.icons.checkOff;
                        tTit+="</a>";
                    }
                    
                    tTit+="<span ";
                    if(fld.type!="bool")
                        tTit+="class='"+this.inputClass+"_container' ";
                    else tTit+="class='sectHid'";
                    tTit+=" style='white-space:nowrap;'>";
                    tTit+="<input ";
                        tTit+=myTags;
                        if( fld.type=="bool"){
                            tTit+=(parseBool(element.value) ? " checked " : "") ;
                            tTit+=" class='"+this.inputClass+"_checkbox' ";
                        }
                        else{
                            tTit+=" class='"+this.inputClass+"' ";
                            tTit+=" value=\""+sanitizeElementAttr(val)+"\" ";
                        }

                    tTit+=" />";
                    if(fld.type=="date" || fld.type=="datetime") {
                        tTit+="<a href=javascript:vjObjEvent(\"setNow\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")>"+this.icons.setNow+"</a>";
                    }
                    tTit+="</span>"
                    
                }
                if(fld.constraint=="barcode"){
                    element.barid=sanitizeElementAttrJS(element.path);
                    tTit+="<br/><a href=javascript:vjObjEvent(\"startBarcodeScanner\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")>";
                    tTit+="<canvas class='" + (element.value ? "sectVis" : "sectHid")+"' id='"+element.barid+"'></canvas>";
                    if(!element.value)tTit+=this.icons.barcode; 
                    tTit+="</a>";
                    this.barcodeArr.push({path:element.path,id:element.barid});
                }
                if(fld.constraint=="3dmol"){
                    var cd=this.objCls+"-chemdoodle";
                    tTit+="<br/><canvas class='ChemDoodleWebComponent' id='"+cd+"' width='"+this.ChemDoodleSize.x+"' height='"+this.ChemDoodleSize.y+"' alt='ChemDoodle Web Component' style='width: "+this.ChemDoodleSize.x+"px; height: "+this.ChemDoodleSize.y+"px; background-color: rgb(255, 255, 255);'>This browser does not support HTML5/Canvas.</canvas>" ;
                    this.chemDoodleArr.push({id:cd,value: element.value});
                             
                }
                if(fld.constraint=="3dmodel"){
                    var cd=this.objCls+"-3dmodel";
                    tTit+="<br/><canvas id='"+cd+"' width='"+this.model3DSize.x+"' height='"+this.model3DSize.y+"' alt='3D Modeling Babylon Web Component' style='width: "+this.model3DSize.x+"px; height: "+this.model3DSize.y+"px; background-color: rgb(255, 255, 255);'>This browser does not support HTML5/Canvas.</canvas>" ;
                    this.model3DArr.push({id:cd,path:element.path});
                             
                }
                if(fld.constraint=="barcode"){
                }

            }
            if(this.debug)tTit+="<br/><small><small>"+elname+"</small></small>";

            
            if(fld.link_url || fld.type=="url"){

                tTit+="<td width=32 align=right><a href=javascript:vjObjEvent(\"onClickUrlLink\",\""+this.objCls+"\",\""+sanitizeElementAttrJS(element.path)+"\")>"+this.icons.urlJump+"</a></td>";
            }
            tTit+="</td>";

        }



        if (!hidetitle ) {
            if (((this.autoStatus & 0x01) && (fld.type != "list" && fld.type != "array")) ||
                ((this.autoStatus & 0x02) && (fld.type == "list" || fld.type == "array"))) {  tTit += "<td something_crazu=1 style='vertical-align: middle;' border=0 id='" + elname + "-status'>" + this.elementStatusText(element) + "</td>"; }

            if( !this.hideControls && !element.hideYourControls ){
                tTit+="<td border=0 align=left id='"+elname+'-'+this.RVtag+"-controls' class='sectHid' style='display:table-cell; ' >";
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
        
        
        if ((fld.constraint !='search+')&& fld.is_multi_fg &&  !fld.is_readonly_fg) {
            tTit += "<td><button class='linker' style='padding-left:50px' onclick='javascript:vjObjEvent(\"onAddElement\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(element.parent.path) + "\",\"" + sanitizeElementAttrJS(fld.name) + "\"); stopDefault(event);'" + this.fieldAddTitleTitle(fld) + ">";
            tTit += this.icons.addRowMore;
            tTit += (tident ? "&nbsp;" : "");
            tTit += "</button></td>";
        }



        var t="";
        var arrNoArr=false;
        if(element.fld.title && element.fld.title.indexOf(":")==0){
            tTit=null;
            tident=0;
            element.expanded=true;
            arrNoArr=true;
        }
        if(tTit) {
            t += "<table  border=0 id='"+elname+"-wrapper' class='"+(fld.hidden ? this.tblClass+"_table" : this.tblClass+"_table")+"'";
            if (!fld.is_readonly_fg && !this.readonlyMode) {
                t += "onmouseover='vjObjEvent(\"onMouseOver\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\" , 1)'";
            }
            t += "><tr>";
            
            t += tTit;
            
            t += "</tr>";
            t += "</table>";
        }




        var tGrp = "";
        var ifDrawtGrap;
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
            
            tGrp+="<table border=0 class='"+( (element.expanded>=this.expansion ) ? (this.tblClass+"_table" + (arrNoArr ? "" : " " + this.tblClass + "_array" )) : "sectHid")+"' id='"+elname+"-children' border=1>";
                tGrp+="<tr>";
                if(!hidetitle && !(arrNoArr)){
                    tGrp+="<td width="+tident+"></td>";

                    for( var i=0; i< fld.children.length; ++i) {
                        tGrp+="<th" + (fld.children[i].hidden ? " class='sectHid'" : "") + ">";
                        if(this.debug)tGrp+= (fld.children[i].is_optional_fg?"O":"-")+(fld.children[i].is_multi_fg?"M":"-")+"["+fld.children[i].children.length+"]";
                        tGrp += (element.children[subRows[0][ fld.children[i].name] ].title)?element.children[subRows[0][ fld.children[i].name] ].title: fld.children[i].title;
                        tGrp+="</th>";
                    }
                    tGrp+="<td >"+this.icons.white+"</td>";
                    tGrp+="</tr>";
                }


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
                if(!arrNoArr)if(!hidetitle ||fld.is_multi_fg )tGrp+="<td width="+tident+"></td>";
                for (var ifld=0; ifld<fld.children.length; ifld++) {
                    if(arrNoArr)  {
                        if(ifld==0)tGrp+="<td>"+this.icons.itemRow+(tident ? "&nbsp;" : "")+"<td>";
                        var ttt= (element.children[subRows[0][ fld.children[ifld].name] ].title)?element.children[subRows[0][ fld.children[ifld].name] ].title: fld.children[ifld].title;
                        if(ttt!=","){
                            tGrp+="<td align=right  >";
                            tGrp+="<span id='"+element.children[subRows[0][ fld.children[ifld].name] ].name+"-title' >";
                            tGrp+=ttt+":"+(!ifld ? (tident ? "&nbsp;" : "") :"");
                            tGrp+="</span>";
                            tGrp+="</td>";
                        }
                    }
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
                tGrp += "<tr class='" + this.tblClass + "_interface'><td width="+tident+"></td>";
                tGrp += "<td><button class='linker' type='button' onclick='javascript:vjObjEvent(\"onAddArrayRow\",\""+ this.objCls+"\",\""+ sanitizeElementAttrJS(element.path)+"\"); stopDefault(event);'" + this.fieldAddTitleTitle(fld) + ">" + this.icons.addRowMore + "&nbsp;" + fld.title + "</button></td>";
                tGrp += "</tr>";
            }

            tGrp+="</table>";

        } else if ((fld.children && fld.children.length) || ((fld.type == "string" && ((fld.constraint=="search") || (fld.constraint == "type")) ))) {
            tGrp += "<div></div><table border=0 class='" + (this.tblClass + "_table" ) + "' id='" + elname + "-children' border=0>";

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
                    if(!hidetitle && !fld.children[il].div)tList+="<td width="+tident+"></td>";
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


                if(thiskind<fld.children[il].maxEl && !this.readonlyMode && !this.editExistingOnly && doPlusSign ) {
                    if(!hidetitle && !fld.children[il].div) {
                        tList+="<tr><td width="+tident+"></td><td>";
                    }
                    if (thiskind) {
                        tList += "<table  border=0 class='" + this.tblClass + "_interface'><tr><td width="+tident+"></td><td>";
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

    this.selectedCamera=function(ev,path)
    {
        if(this._scannerIsRunning) 
            Quagga.stop();
        this.startBarcodeScanner(null, path )    
        
    }
    
    this.generateBarcode=function(container,path,barcodeText, closePopup)
    {
        
        var cnvid=this.objCls+"-barcodecanvas";
        gObject(cnvid).className="sectVis";
        gObject(this.objCls+"-barcodePrintButton").className="REC_input";
        gObject(this.objCls+"-barcodeGenerateNew").className="sectHid";
        
        
        var element=this.nodeTree.findByPath(path);
        if(!barcodeText || !barcodeText.length) {
            barcodeText="42"+this._id_type+"0"+Math.floor((Math.random()*1000000))+"0"+(this.hiveId ? this.hiveId : Math.floor((Math.random()*100)));
            this.fixElementSize(element);
        }
        barcodeGenerate(document.getElementById(cnvid), "code128", barcodeText,3,80,10);
        
        element.value = barcodeText;
        this.formObject.elements[element.name].value = barcodeText;
        if(this.autoResizeFields) 
            this.fixElementSize(element);
        if(closePopup)
            this.gModalClose(this.myFloaterName+this.specialFloater+"Div");


    }
    
    this.onModalClose=function(container,path)
    {
        this.gModalClose(this.myFloaterName+this.specialFloater+"Div");
    }
    
    this.printBarcode=function(container,path)
    {
        printCanvas(this.objCls+"-barcodecanvas",800,600);
    }
    
    this.startBarcodeScanner=function(container, path , textbox)
    {
        if(this._scannerIsRunning) {
            Quagga.stop();
            this._scannerIsRunning=false;
        }
        var element=this.nodeTree.findByPath(path);

        
        var dv=this.myFloaterName+this.specialFloater+"Viewer";
        var o=gObject(dv);
        var t="";
        t+="<table  border=0 width='100%'>";
        t+="<tr onclick='vjObjEvent(\"generateBarcode\",\""+this.objCls+"\",\""+path+"\",0,true)' ><td align='center'><canvas  class='"+(element.value ? 'sectVis' : 'sectHid')+"' height='100' id='"+this.objCls+"-barcodecanvas'></canvas><img  id='"+this.objCls+"-barcodeGenerateNew' class='"+(element.value ? 'sectHid' : 'sectVis')+"' width='150' src='img/barcodeGenerate.gif' /><br/><span style='cursor:pointer;' class='REC_input' >GENERATE NEW</span></td></tr>";
        t+="<tr class='"+((element.value && element.value.length) ? 'sectVis' : 'sectHid')+"' onclick='vjObjEvent(\"printBarcode\",\""+this.objCls+"\",\""+path+"\")' id='"+this.objCls+"-barcodePrintButton' ><td align='center'><img height='200' src='img/barcodePrint.jpg' /><br/><span style='cursor:pointer;' class='REC_input' >PRINT</span></td></tr>";
        t+="<tr onclick='gObject(\"\")' ><td align='center'>";
            t+="<scan class='REC_input' style='cursor:pointer;' >SCAN</scan>";
            t+=mediaCameraListText("vjObjEvent(\"selectedCamera\",\"" + this.objCls + "\",\""+path+"\");")+"<br/>";
            t+="<div id='"+this.myFloaterName+this.specialFloater+"Image"+"'></div><br/><scan class='REC_input' onclick='vjObjEvent(\"onModalClose\",\""+this.objCls+"\",\""+path+"\")' style='cursor:pointer;' >CLOSE</scan>";
        t+="</td></tr>";
        t+="</table>";

        
        o.innerHTML=t;
        if(element.value)this.generateBarcode(null,path,element.value);
        
        dv=this.myFloaterName+this.specialFloater+"Image";
        
        
        
        
        this.gModalOpen(this.myFloaterName+this.specialFloater+"Div", "vjObjEvent('stopBarcodeScanner','" + this.objCls + "')" , this.rectPopup.x, this.rectPopup.y, 0,this.rectPopup.cx,this.rectPopup.cy);
        
        Quagga.init({
            inputStream: {
                name: "Live",
                type: "LiveStream",
                target: document.querySelector("#"+dv),
                constraints: {
                    width: 480,
                    height: 320,
                    deviceId: mediaActiveCam ? mediaCameraList[mediaActiveCam].deviceId : null,
                    facingMode: mediaActiveCam ? undefined:  "environment"
                }
            },
            locate: true,
            locator: { 
                halfSample: true,
                  patchSize: "medium",
                  debug: {
                    showCanvas: false,
                    showPatches: false,
                    showFoundPatches: false,
                    showSkeleton: false,
                    showLabels: false,
                    showPatchLabels: false,
                    showRemainingPatchLabels: false,
                    boxFromPatches: {
                      showTransformed: false,
                      showTransformedBox: false,
                      showBB: false
                    }
                  }
            },
            decoder: {
                readers: [
                    "code_128_reader",
                    "ean_reader",
                    "ean_8_reader",
                    "code_39_reader",
                    "code_39_vin_reader",
                    "codabar_reader",
                    "upc_reader",
                    "upc_e_reader",
                    "i2of5_reader"
                ],
                debug: {
                    showCanvas: true,
                    showPatches: true,
                    showFoundPatches: true,
                    showSkeleton: true,
                    showLabels: true,
                    showPatchLabels: true,
                    showRemainingPatchLabels: true,
                    boxFromPatches: {
                        showTransformed: true,
                        showTransformedBox: true,
                        showBB: true
                    }
                }
            }

        }, function (err) {
            if (err) {
                console.log(err);
                return
            }

            Quagga.start()

            
        });
        this._scannerIsRunning = true;
        
        Quagga.onProcessed(function (result) {
            var drawingCtx = Quagga.canvas.ctx.overlay,
            drawingCanvas = Quagga.canvas.dom.overlay;

            if (result) {
                if (result.boxes) {
                    drawingCtx.clearRect(0, 0, parseInt(drawingCanvas.getAttribute("width")), parseInt(drawingCanvas.getAttribute("height")));
                    result.boxes.filter(function (box) {
                        return box !== result.box;
                    }).forEach(function (box) {
                        Quagga.ImageDebug.drawPath(box, { x: 0, y: 1 }, drawingCtx, { color: "green", lineWidth: 2 });
                    });
                }

                if (result.box) {
                    Quagga.ImageDebug.drawPath(result.box, { x: 0, y: 1 }, drawingCtx, { color: "#00F", lineWidth: 2 });
                }

                if (result.codeResult && result.codeResult.code) {
                    Quagga.ImageDebug.drawPath(result.line, { x: 'x', y: 'y' }, drawingCtx, { color: 'red', lineWidth: 3 });
                }
            }
        });
        var thiSS=this;
        Quagga.onDetected(function (result) {
            element.value = result.codeResult.code;
            thiSS.formObject.elements[element.name].value = result.codeResult.code;
            fixElementSize(element);
            thiSS.stopBarcodeScanner();
            thiSS.gModalClose(thiSS.myFloaterName+thiSS.specialFloater+"Div");
        });
    }
    this.stopBarcodeScanner=function()
    {
        if (this._scannerIsRunning) {
            Quagga.stop();
            this._scannerIsRunning=false;
        }
    }
    
    this.startPhotoSession=function(container, path , textbox)
    {
        
        var element=this.nodeTree.findByPath(path);
        this.currentPopupElement=element;
        
        var tagWord="recView"+this.objCls;
        gObject(this.myFloaterName+this.specialFloater+"Viewer").innerHTML=mediaConstructBody(tagWord);
             
        this.gModalOpen(this.myFloaterName+this.specialFloater+"Div", 0 , this.rectPopup.x, this.rectPopup.y, 0,this.rectPopup.cx,this.rectPopup.cy);

        var thiSS=this;
        mediaInitiate(tagWord, function (){
                thiSS.saveImage(mediaGetImageDataURL(tagWord));
            }, function () {
                thiSS.saveImage(mediaGetClipboardDataURL(tagWord));
                
            });
    }
    
    this.saveImage=function(vr)
    {
        if(!vr)return ;
        if(!this.hiveId) {
            alert("you must create the record first before associating images to it");
            return ;
        }
        
        var url="?cmdr=objSetFile&id="+this.hiveId +"&content="+vr.data;
        ajaxDynaRequestPage(url, {objCls: this.objCls, callback:'onSavedPhoto', filename: vr.file }, vjObjAjaxCallback, true);
        
        if(this.gModalClose==gModalClose)
            setTimeout("gModalClose('"+this.myFloaterName+this.specialFloater+"Div')",300);
        else this.gModalClose(this.myFloaterName+this.specialFloater+"NDiv");
    }
    this.onSavedPhoto=function(params,container)
    {
        this.parsePopupList ([this.hiveId+"/"+params.filename]);
    }
    
    
}


