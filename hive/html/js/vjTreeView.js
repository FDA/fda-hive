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


function vjTreeView ( viewer )
{
    vjDataViewViewer.call(this,viewer);

    if(!this.dataFormat)this.dataFormat="csv";
    if(this.showIcons===undefined) this.showIcons=true;
    if(!this.expansion)this.expansion=1;
    if(!this.scaleDist)this.scaleDist=20;
    if(!this.showRoot)this.showRoot=1;
    if(!this.autoexpand)this.autoexpand=0;
    if(!this.icons) this.icons=new Object();
    if(!this.icons.empty)this.icons.empty='img/recItem.gif';
    if(!this.icons.plus)this.icons.plus='img/recExpand.gif';
    if(!this.icons.minus)this.icons.minus='img/recCollapse.gif';
    if(this.icons.leaf===undefined)this.icons.leaf='img/recItem.gif';
    if(!this.iconSize)this.iconSize=12;
    if(this.cmdMoreNodes && typeof this.disableBranches !== 'boolean'){ this.disableBranches = false; }
    if(!this.hierarchyColumn)this.hierarchyColumn='path';
    if(!this.className)this.className="TREEVIEW_table";
    
    if(!this.classNameSelected)this.classNameSelected=this.className+"_selected";
    if(!this.classNameHighlighted)this.classNameHighlighted=this.className+"_highlighted";
    if(this.checkIndeterminateTitle === undefined)
        this.checkIndeterminateTitle = "Partially selected";
    if(!this.isDropbox) this.isDropbox = false;
    if(this.drag)this.drag=new dragDrop();
    this.tree = new vjTree();

    this.exclusionObjRegex=verarr(this.exclusionObjRegex);
    if(this.exclusionObjRegex.length==0){
        this.exclusionObjRegex.push({info: /^info*/} );
        this.exclusionObjRegex.push({id: /^info*/} );
    }

    this.hasTreeSeries = function()
    {
        var data = this.getData(0);
        return data && data.isTreeSeries;
    };
    this.defaultParseContent=function(content)
    {   
        if (this.dataFormat === "newick") {
            if (!this.tree) this.tree = new vjTree();

            this.tree.parseNewick(content, this.tree.root);
        } else if (this.dataFormat === "json" ) {
            if (!this.tree) this.tree = new vjTree();

            this.tree.parseJson(content, this.tree.root);
        } else {
            var tbl=new vjTable(content, 0, vjTable_propCSV);
            tbl.mangleRows(this.exclusionObjRegex, "delete");
            if(this.preTreatNodesBeforeParsing)  this.preTreatNodesBeforeParsing(tbl,content);
            
            if(this.precompute) tbl.enumerate(this.precompute, this);
            
            if ( this.tree && !this.isDropbox){
                this.tree.parseTable(tbl, this.tree.root);
            } else if(this.tree && this.isDropbox){
                this.tree.parseTable(tbl, this.tree.root , false , false , this.isDropbox);
            }else {
                this.tree = new vjTree(tbl,this.hierarchyColumn);
            }
        }
    };
    if (!this.parseContent) {
        this.parseContent = this.defaultParseContent;
    }


    this.composerFunction=function( viewer , content )
    {
        if(this.debug)alert(content);
        if (this.hasTreeSeries()) {
            this.tree = this.getData(0).tree;
            this.refresh();
            return;
        }

        var content="";
        for(var id=0;id<this.data.length;++id) content+=this.getData(id).data;

        if(!this.maintainPreviousData ) {
            this.tree.empty();
        }

        this.parseContent(content);

        this.maintainPreviousData=false;

        this.tree.enumerate(function(params,node){
            if(params.autoexpand &&
                    (params.autoexpand=='all' || node.depth<=params.autoexpand) &&
                    !node.isNexpandable &&
                    node.children &&
                    node.children.length>0 )
                node.expanded=params.expansion;
        }, {autoexpand:this.autoexpand, expansion: this.expansion} ) ;

        if(this.postcompute)
            this.tree.enumerate(this.postcompute, this);
        this.refresh();
    };

    this.refresh=function(node)
    {
        var t="";
        var refresh_all = false;
        if( node ===undefined || !node.parent) {
            node = this.tree.root;
            refresh_all = true;
        }
        if(this.prefixHTML && refresh_all)t+=this.prefixHTML;

        t+=this.outputNodeTreeView( node );

        if (this.appendHTML && this.appendHTML.length && refresh_all) {
            t += this.appendHTML;
        }
        if( refresh_all ) {
            this.div.innerHTML=t;
        }
        else {
            var o = gObject(this.getNode_ElementID(node.parent, node.id ? node.id : node.path));
            if(o) {
                o.innerHTML = t;
            }
        }
        if (this.checkBranches || this.checkLeafs) {
            this.initCheckTriState();
        }

        if(this.drag)this.setDnDobjects();
    };

    this.setDnDobjects=function()
    {
        var dragOperation=this.setDragElementsOperation;
        var dropOperation=this.setDropElementsOperation;
        if(dragOperation===undefined){
            dragOperation=function(params,node) {
                if (node.path) {
                    var o = gObject(sanitizeElementId(params.that.container
                            + node.path));
                    if (o)
                        o = o.parentNode;
                    if (o) {
                        params.list.push(o);
                    }
                }
            }
        }
        if(dropOperation===undefined){
            dropOperation=function(params,node){
                if (node.path) {
                    var o = gObject(sanitizeElementId(params.that.container
                            + node.path));
                    if (o)
                        o = o.parentNode;
                    if (o) {
                        params.list.push(o);
                    }
                }
            }
        }
        this.dragListO=new Array();
        this.tree.enumerate(dragOperation,{list:this.dragListO,that:this});

        this.dropListO=new Array();
        this.tree.enumerate(dropOperation,{list:this.dropListO,that:this});

        for ( var il = 0; il < this.dropListO.length; ++il){
            this.drag.initDropElement(this.dropListO[il]);
        }
        for ( var il = 0; il < this.dragListO.length; ++il){
            this.drag.initDragElement(this.dragListO[il], {
                func : this.dragCallonDrop,
                obj : this
            }, {
                func : this.dragCallonStart,
                obj : this
            }, {
                func : this.dragCallonMove,
                obj : this
            }, {
                func : this.dragCallonTarget,
                obj : this
            }, {
                func : this.dragCallonStop,
                obj : this
            });
        }
    };

    this.findByDOM=function(obj){
        var node=null;
        for (var ia=0 ; ia< obj.childNodes.length ; ++ia) {
            var elemID=obj.childNodes[ia].id;
            if(!isok(elemID))
                elemID=obj.childNodes[ia]['_id'];
            if(isok(elemID)){
                var contInd=elemID.indexOf(this.container);
                if(contInd!=-1){
                    var nodes = [];
                    var sanitized_path = elemID.slice(this.container.length);
                    this.enumerate(function(viewer, node) {
                        if (node.path && sanitizeElementId(node.path) === sanitized_path) {
                            nodes.push(node);
                        }
                    }, this);
                    if (nodes.length) {
                        if (nodes.length > 1) {
                            console.log("DEVELOPER WARNING: vjTreeView.findByDOM: multiple tree nodes matched sanitized path " + JSON.stringify(sanitized_path));
                        }
                        node = nodes[0];
                    }
                    break;
                }
            }
        }
        return node;
    };



    this.doExpand=function( thisNode , state, withParents)
    {
        if(state==-1)thisNode.expanded=-thisNode.expanded;
        else if(state>=0)thisNode.expanded=state;
        if(!withParents)return;

        while(thisNode.parent) {
            thisNode=thisNode.parent;
            if(state==-1)thisNode.expanded=-thisNode.expanded;
            else if(state>=0)thisNode.expanded=state;
        }
        return thisNode;
    };

    this.doSelect=function( thisNode , state, renderAll , autoExpand, original)
    {
        if (this.multiSelect) {
            if (this.oldShift !== undefined && gKeyShift) {
                var oldNode = this.tree.findByPath(this.oldShift);
                if (oldNode) {
                    var startSelectNode, endSelectNode;
                    if (this.tree.cmpNodes(thisNode, oldNode) < 1) {
                        startSelectNode = thisNode;
                        endSelectNode = oldNode;
                    } else {
                        startSelectNode = oldNode;
                        endSelectNode = thisNode;
                    }
                    this.enumerate(function(viewer, node) {
                        if (viewer.tree.cmpNodes(node, startSelectNode) >= 0 && viewer.tree.cmpNodes(node, endSelectNode) <= 0) {
                            node.selected = state;
                            if (renderAll) {
                                var o = gObject(sanitizeElementId(viewer.container + node.path));
                                if (o) {
                                    viewer.setClass.nodeSelected(node, o);
                                }
                            }
                        }
                    }, this);
                }
            } else {
                this.oldShift = thisNode.path;
            }
        } else {
            this.oldShift = undefined;
        }

        if ((!this.multiSelect || (!gKeyCtrl && !gKeyShift)) && !original) {
            if (renderAll) {
                this.enumerate(function(that,node){
                    node.selected=0;
                    var o=gObject(sanitizeElementId(that.container+node.path));
                    if(o) {
                        that.setClass.nodeSelected(node,o)
                    }
                }, this);
            } else {
                this.enumerate(function(that,node){node.selected=0}, this);
            }
        }

        thisNode.selected=state;
        if(autoExpand)
            this.doExpand( thisNode , this.expansion,true);
    };
    this.enumerate=function(operation,params, ischecked, leaforbranch ,node)
    {
        if(!this.tree || !this.tree.root)return ;
        return this.tree.enumerate(operation,params,ischecked,leaforbranch,node);
    };

    this.accumulate=function( checker, collector, params, ischecked, leaforbranch , node  )
    {
        if(!this.tree || !this.tree.root)return ;
        return this.tree.accumulate( checker, collector, params, ischecked, leaforbranch, node );
    };

    this.getCheckedNode = function()
    {
        return this.tree.accumulate(true,"node",new Object(),true);
    };

    this.getUncheckedNode = function()
    {
        return this.tree.accumulate("(!node.checked)","node",new Object());
    };

    this.getSelectedNode = function()
    {
        return this.tree.accumulate("(node.selected)","node",new Object());
    };

    this.getUnselectedNode = function()
    {
        return this.tree.accumulate("(!node.selected)","node",new Object());
    };


    this.onClickExpandNode=function(container, nodepath, state )
    {
        var node=this.tree.findByPath(nodepath);

        if (state>0) {
            var data = this.getData(0);
            if (data.isTreeSeries) {
                if (!data.cmdMoreNodes) data.cmdMoreNodes = this.cmdMoreNodes;
                data.getMoreNodes(node);
            } else if (this.cmdMoreNodes) {
                this.maintainPreviousData=true;
                var url=evalVars(this.cmdMoreNodes, "$(", ")", node);
                data.url=url;
                data.load();
            }
        }
        this.doExpand( node, state, false) ;
        var linkExpand = 0 ;

        if(!linkExpand && node.expandNodeCallback)
            linkExpand=node.expandNodeCallback;
        if(!linkExpand)
            linkExpand=this.expandNodeCallback;
        funcLink( linkExpand , this, node );


        this.refresh(node);
    };

    this.onClickNode=function(container, nodepath )
    {
        if(gDragEvent){
            gDragEvent=false;
            return ;
        }
        var node=this.tree.findByPath(nodepath);

        if( !this.notSelectable ) {
            this.doSelect( node, node.selected ? (this.alwaysSelected?1:0) : 1, true, false);

            var o=gObject(sanitizeElementId(this.container+node.path));
            if(o)this.setClass.nodeSelected(node,o);

            if(this.selectionObject && this.selectionObject.length!=0 ) {
                var o=gObject(this.selectionObject);
                if(o && o.innerHTML )o.innerHTML=nodepath;
                o=gObject(this.selectionObject+"_title");
                if(o && o.innerHTML )o.innerHTML=node.title ? node.title : node.name ;
                if(this.formObject ) {
                    o=this.formObject.elements[this.selectionObject];
                    if(o)o.value=nodepath;
                    o=this.formObject.elements[this.selectionObject+"_title"];
                    if(o)o.value=node.title ? node.title : node.name ;
                }
            }
        }


        var linkCB;
        linkCB=this.selectCaptureCallback;
        if(!linkCB && node.url)linkCB=node.url;
        if(!linkCB){
            if(node.leafnode && this.linkLeafCallback)linkCB=this.linkLeafCallback;
            else if(this.linkBranchCallback)linkCB=this.linkBranchCallback;
        }

        funcLink( linkCB, this, node );

        if(this.selectDoesExpand)
            this.onClickExpandNode(container,nodepath,(node.expanded>=this.expansion ? 0 : this.expansion ));


        if(this.selectInsteadOfCheck){
            this.tree.enumerate("if(node.selected)node.checked=1;else node.checked=0;");
            this.updateDependents();
        }

    };

    this.formCheckSwitchTriState = function(nodepath, state) {
        var checknam = sanitizeElementId(this.container+"_check_" + nodepath);
        var o = gObject(checknam);
        if (!o) {
            return;
        }
        if ((!state || state > 0) && !o.disabled) {
            o.checked = state;
            o.indeterminate = false;
            o.removeAttribute("title");
        } else if(!o.disabled){
            o.checked = true;
            o.indeterminate = true;
            if (this.checkIndeterminateTitle) {
                o.title = this.checkIndeterminateTitle;
            } else {
                o.removeAttribute("title");
            }
        }
    };

    this.setParentCheckTriState=function (elems, nodepath, doPropagateUp )
    {
        var node=this.tree.findByPath(nodepath);
        var cntChecked=0, cntSomeHalf=0;
        for( var ic=0; ic<node.children.length; ++ic){
            if(node.children[ic].checked){
                ++cntChecked;
                 if(node.children[ic].checked<0)
                     ++cntSomeHalf;
            }

        }


        node.checked=(cntChecked==node.children.length && cntSomeHalf==0) ? 1 : (cntChecked ? -1 : 0 );
        this.formCheckSwitchTriState( node.path, node.checked );
        if(doPropagateUp && node.parent && node.parent.path)
            this.setParentCheckTriState(elems, node.parent.path, doPropagateUp );
    };

    this.initCheckTriState=function(node, state) {
        if (!this.tree || !this.tree.root) {
            return;
        }
        if (!node) {
            node = this.tree.root;
        }
        var was_checked = node.checked;
        if (state !== undefined) {
            node.checked = state;
        }

        var cnt_checked = 0;
        for (var ic=0; ic<node.children.length; ic++) {
            cnt_checked += this.initCheckTriState(node.children[ic], node.checked > 0 && this.checkPropagate ? 1 : undefined);
        }
        if (node.children.length && !this.doNotPropagateUp) {
            if (cnt_checked == node.children.length) {
                node.checked = 1;
            } else if (cnt_checked) {
                node.checked = -1;
            } else {
                node.checked = 0;
            }
        }
        
        this.formCheckSwitchTriState(node.path, node.checked);
        return node.checked > 0 ? 1 : 0;
    };

    this.onCheckNode=function(container, nodepath )
    {
        var node=this.tree.findByPath(nodepath);
        if (!node.checked || node.checked < 0) {
            node.checked = 1;
        } else {
            node.checked = 0;
        }
        if(this.checkPropagate) {
            var state = node.checked;
            this.tree.enumerate(function(viewer, node) {
                node.checked = state;
                viewer.formCheckSwitchTriState(node.path, node.checked);
            }, this, undefined, undefined, node);
        }

        if(node.parent && node.parent.path && (!this.doNotPropagateUp ))
            this.setParentCheckTriState(this.formObject.elements, node.parent.path, !this.doNotPropagateUp );

        var linkCB=0;
        if(node.leafnode && this.checkLeafCallback)linkCB=this.checkLeafCallback;
        else if(this.checkBranchCallback)linkCB=this.checkBranchCallback;

        var res = 1;
        if(linkCB)
            res = funcLink( linkCB, this, node );

        this.updateDependents();
        return true;
    };

    this.mimicCheckmarkClick = function(nodepath)
    {
        var node=this.tree.findByPath(nodepath);
        this.formCheckSwitchTriState(nodepath, !node.checked || node.checked < 0);
        return this.onCheckNode(this.container, nodepath);
    };

    this.onMouseOver=function(container, nodepath , state,event )
    {
        var node=this.tree.findByPath(nodepath);
        var o=gObject(sanitizeElementId(this.container+node.path));
        if(!o)return;
        if(state>0)
            this.setClass.nodeHighlighted(node,o);
        else
           this.setClass.nodeSelected(node,o);

        if(this.onMouseOverCallback){
            funcLink( this.onMouseOverCallback, this, node, state,event );
        }
    };


    this.setIcons = {
        self : this,
        node : function(n) {
            var t = "";
            if(n.icon && this.self.iconSize && this.self.showIcons){
                var nSize=this.self.iconSize;
                if(n.iconSize)nSize=n.iconSize;
                t+="<td width="+nSize+"><img border=0 width="+nSize+" height="+nSize+" src='"+n.icon+"' valign=middle/></td>";
            }
            return t;
        },
        leaf : function() {
            var t="";
            if(this.self.icons.leaf)
                t= "<img border=0 width="+this.self.iconSize+" height="+this.self.iconSize+" src='"+this.self.icons.leaf+"' />";
            return t;
        },
        branch : function(node,cntCh) {
            var t = "";
            t+="<img border=0 width="+this.self.iconSize+" height="+this.self.iconSize+" src='"+( (node.expanded>=this.self.expansion && node.children.length>0 && cntCh <= node.children.length) ? this.self.icons.minus : this.self.icons.plus)+"' />";
            return t;
        },
        emptyBranch : function() {
            var t = "";
            if(!this.self.isNshowIconEmpty && this.self.icons.empty)
                t+="<img border=0 width="+this.self.iconSize+" height="+this.self.iconSize+" src='"+this.self.icons.empty+"' />";
            return t;
        },
        background_td : function(node) {
            return " ";
        }

    };

    this.setClass = {
        self : this,
        tag : "class",
        table : function() {
            var t = " ";
            if (this.self.className) {
                t = this.tag + "='" + this.self.className + "'";
            }
            return t;
        },
        tr : function(state) {
            var t = " ";
            if (!state) {
                t = this.tag + "='" + this.self.className + "'";
            } else if (state == 1) {
            } else if (state == 2) {
            } else {
                return alert("DEV alert: not tristate flag");
            }
            return t;
        },
        td : function(state) {
            var t = " ";
            if (!state) {
                t = this.tag + "='" + this.self.className + "'";
            } else {
                t = this.tag + "='" + this.self.classNameSelected + "" + state +  "'";
            }
            return t;
        },
        el : function(){
            return this.self.className;
        },
        nodeSelected : function(node,html_el) {
            html_el.className =  node.selected ? this.self.classNameSelected+node.selected : this.self.className;
        },
        nodeHighlighted : function(node,html_el) {
            html_el.className = this.self.classNameHighlighted;
        },
        branchButton: function() {
            return "class='linker'";
        }
    };

    this.getNode_ElementID = function (node, suff) {
        return sanitizeElementId(this.container + "_" + node.name + (suff===undefined?"":("::"+suff) ));
    };

    this.outputNodeTreeView=function( node )
    {
        if(!node)node=this.tree.root;
        if(!this.showLeaf && node.leafnode) return "";

        var cntCh=node.children ? node.children.length : 0 ;
        if(node.childrenCnt>0)cntCh=node.childrenCnt;

        if(this.hideEmpty && !node.leafnode && cntCh<1 || (node.hidden && node.depth) ) return "";
        var t="";


        if(this.showRoot<=node.depth && !node.doNOTshow) {
            t = `<table border=0 width='100%' ${this.setClass.table()}>
                    <tr>
                        <td width='${(this.scaleDist*(node.distance ? node.distance : 1) )}px' ${this.setClass.td()} align=right valign=middle>
                `
            if(!node.leafnode ){
                if((cntCh>=1 || node.size == -1)  && !node.isNexpandable){
                    t+="<button " + this.setClass.branchButton() + " type='button' onclick='javascript:vjObjEvent(\"onClickExpandNode\",\""+this.objCls+"\", \""+sanitizeElementAttrJS(node.path)+"\","+ ((node.expanded>=this.expansion&& cntCh <= node.children.length) ? 0 : this.expansion )+"); stopDefault(event);'>";
                    t += this.setIcons.branch(node,cntCh);
                    t+="</button>";
                }  else {
                    t += this.setIcons.emptyBranch(node);
                }
            }else{
                t += this.setIcons.leaf(node);
            }

            t+="</td>";

            t+=this.setIcons.node(node);

            var realText="";
            if( !this.showName && node.title)realText=node.title;
            else realText=node.name;
            var realVal=node.description ? node.description : realText;
            realText=this.formDataValue(realText , node.type, node, null);

            t+="<td id='"+sanitizeElementId(this.container+node.path)+"' ";
            t+= this.setClass.td(node.selected);
            t+=" onMouseOver='vjObjEvent(\"onMouseOver\",\""+this.objCls+"\", \""+sanitizeElementAttrJS(node.path)+"\", 1,event )' ";
            t+=" onMouseOut='vjObjEvent(\"onMouseOver\",\""+this.objCls+"\", \""+sanitizeElementAttrJS(node.path)+"\", 0,event )' ";
            t+=" title='"+sanitizeElementAttr(realVal)+"'";
            t+=" valign=middle>";
            t+="<div class='linker' tabindex='0'  onclick='javascript:vjObjEvent(\"onClickNode\",\""+this.objCls+"\", \""+sanitizeElementAttrJS(node.path)+"\"); stopDefault(event);'>";

            if( (!node.uncheckable) && ( (this.checkBranches && !node.leafnode) || (this.checkLeafs && node.leafnode) ) )  {
                let checkstate = node.checked === 1 ? "checked" : "" ;
                let disabled = this.disableBranches && !node.leafnode && cntCh < 1 ? 'disabled' : ""; 
                
                t+=`<input 
                        type=checkbox  
                        ${checkstate} 
                        ${disabled} 
                        id='${sanitizeElementId(this.container+"_check_"+node.path)}' 
                        onclick='javascript:vjObjEvent(\"onCheckNode\",\"${this.objCls}\", \"${sanitizeElementAttrJS(node.path)}\"); stopEventPropagation(event);' 
                    /> &nbsp;`;
            }
            t+=realText;
            if(node.size != undefined && Number(node.size) > -1)   t += `&nbsp; <snap class="tree-view--info"> &nbsp; ${formatBytes(node.size)}</snap>`
                
            if(cntCh>=1 && this.showChildrenCount) t+="&nbsp;<small><b>["+cntCh+"]</b></small>";
            t+="</div>";
            if(this.nodeRenderCallback){
                t+="<div class='linker' >";
                t+=funcLink(this.nodeRenderCallback, this, node);
                t+="</div>";
            }
            
            t+="</td></tr>";
        }
        var sectNam=this.getNode_ElementID(node , node.id ? node.id : node.path);
        if(node.expanded && cntCh>=1 ){
            if (this.showRoot <= node.depth) {
                t += "<tr";
                t+=">";
                t += "<td  valign='left' style='border:0;padding:0;";

                t += this.setIcons.background_td(node);
                t += "'></td>";
                t += "<td colspan=2 id='" + sectNam + "'>";
            }
        }

        if( cntCh>=1 && node.expanded) {
            for( var i=0 ;i<node.children.length && !node.doNOshowChildren ; ++i) {
                t+="<span id='"+this.getNode_ElementID(node, node.children[i].id ? node.children[i].id : node.children[i].path)+"'>";
                t+=this.outputNodeTreeView ( node.children[i] ) ;
                t+="</span>";
            }
        }
        if(this.showRoot<=node.depth)
            t+="</td></tr>";

        if(this.showRoot<=node.depth && !node.doNOTshow) {
            t+="</table>";
        }

        if(this.appendNodeHTML)t+=this.appendNodeHTML;
        return t;
    };
}