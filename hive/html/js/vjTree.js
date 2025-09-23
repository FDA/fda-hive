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

function vjTreeNode(nodeBase)
{
    for (var i in nodeBase)
        this[i] = nodeBase[i];

    if(this.name===undefined)this.name="";
    if(this.depth===undefined)this.depth=0;
    if(this.children===undefined)this.children = new Array();
    if(this.childrenCnt===undefined)this.childrenCnt=this.children.length;

    this.isLastSibling = function() {
        var prnt = this.parent;
        if(!this.parent)return true;
        if(this.parent.children.length==1)return true;
        var indx = prnt.children.indexOf(this);
        if(indx == prnt.children.length-1)return true;
        else return false;
    };

    this.push = function(child) {
        this.children.push(child);
        this.childrenCnt++;

        child.parent = this;
        child.depth = this.depth + 1;
        child.path = this.path + (this.path[this.path.length-1] === "/" ? "" : "/") + child.name;
    };
}


function vjTree( tbl, pathcolumn, excludeObjRegexp, debug  )
{

    if(this.newNode===undefined)this.newNode = function (base) {
        return new vjTreeNode(base);
    };

    this.root = this.newNode({name:"root", expanded:true , path:"/"});
    this.debug=debug;


    this.parseTable=function( tbl, rootNode, pathcolumn, excludeObjRegexp , isDropbox = false )
    {
        if(!pathcolumn)pathcolumn="path";

        var hierarchyType=1;
        var t_node = '';
        for ( var i=0; i<tbl.rows.length; ++i){
            t_node = tbl.rows[i];
            if(t_node.child){
                if(!hierarchyType!=2) {
                    hierarchyType=2;
                    t_node.child_hash={}
                }
                var t_children = verarr(t_node.child);
                for(var ic = 0 ; ic < t_children.length; ++ic ) {
                    t_node.child_hash[t_children[ic]] = true;
                }
            }
        }
        var curT;
        for ( var ia=0; ia<tbl.rows.length; ++ia){
            var fld=tbl.rows[ia];
            
            if(fld.nonAutoNode!==false)fld.nonAutoNode=true;

            while ( !fld[pathcolumn] ) {

                if(hierarchyType==1){
                    if(!fld.parent){
                        
                        if(!isDropbox){
                            fld[pathcolumn]="/"+fld.name;
                        } else {
                            fld[pathcolumn]= `/${fld.id}${fld.name}`;
                        
                            fld['cmdpath'] = `/${fld.name.split('/').slice(2).join('/')}`;
                            fld['cmdid'] = fld.id;
                            fld.id = fld[pathcolumn];

                            fld.name = fld.name.indexOf('/') === 0 ? fld.name.slice(1) : fld.name;
                            if(fld.name[fld.name.length - 1] === '/') {
                                fld.name = fld.name.substring(0, fld.name.length - 1);
                            };
                            fld.name = `${fld.name.split('/').slice(-1)}`;
                        }
                        
                        break;
                    }

                    var parToLink=this.findByName( fld.parent, rootNode ) ;
                    if(parToLink){
                        fld[pathcolumn]=parToLink.path+"/"+fld.name;
                        break;
                    }


                    for( var ir=ia+1; ir<tbl.rows.length; ++ir) {
                        if(tbl.rows[ir].name==fld.parent){
                            var tmpFld=fld;
                            fld=tbl.rows[ir];tbl.rows[ia]=fld;
                            tbl.rows[ir]=tmpFld;
                            break;
                        }
                    }
                    if(ir==tbl.rows.length)
                        {fld[pathcolumn]="/"+fld.name;break;}
                }
                else{
                    var id = fld.id?'id':'_id';
                    var params = {
                            'result':fld[id]
                    };
                    var path="";

                    while (isok(params.result)){
                        var t=params.result;
                        tbl.enumerate(function (params, that, ir) {
                            var node = that.rows[ir];
                            if (node.child && !params.scannedPath && (t in node.child_hash) ) {
                                params.result = node[id];
                                params.name = node.name;
                                if (node[pathcolumn]) {
                                    params.scannedPath = node[pathcolumn];
                                }
                            }
                        }, params);

                        if(t==params.result)
                            params.result='';
                        else {
                            if( params.scannedPath ) {
                                params.result='';
                                path = params.scannedPath + path ;
                            }
                            else{
                                path="/"+params.name+path;
                            }
                        }
                    }
                    fld[pathcolumn]=path+"/"+fld.name;
                }
            }

            if( isok(excludeObjRegexp) ) {
                if( matchObj( fld, excludeObjRegexp ) )
                    continue;
            }

            curT=rootNode;

            var prvpos=1;
            var pos=fld[pathcolumn].indexOf("/",1);
            if(pos==-1)pos=fld[pathcolumn].length;

            while( pos!=-1) {
                var thisNode=fld[pathcolumn].substring(prvpos,pos);

                var ifnd=0;
                for( ifnd=0; ifnd<curT.children.length && curT.children[ifnd].name!=thisNode; ++ifnd );
                
                parNode=curT;
                var newobj = this.newNode();
                
                if(ifnd<curT.children.length ){
                    newobj=curT.children[ifnd];
                    if(pos>=fld[pathcolumn].length) {
                        if(!newobj.tree_cnt_table_rows){
                            newobj.tree_cnt_table_rows=1;
                        }
                        else {
                            ++newobj.tree_cnt_table_rows;
                            fld.tree_duplicate=newobj;
                        }
                    }
                    
                    if(fld[pathcolumn].length-pos <= 1  && (curT.children[ifnd].hasOwnProperty('id') && fld.hasOwnProperty('id') && (curT.children[ifnd].id != fld.id) ) ){
                        newobj = this.newNode();
                        for ( vfeat in fld ) newobj[vfeat]=fld[vfeat];
                        newobj.path=fld[pathcolumn];
                        
                        newobj.children=new Array();
                        newobj.parent=parNode;

                        newobj.leafnode=newobj.path.charAt(newobj.path.length-1)=="/" ? false : true;

                        if(!newobj.depth)newobj.depth=parNode.depth+1;
                        if(!newobj.distance)newobj.distance=1 ;
                        if(!newobj.childrenCnt)newobj.childrenCnt=(pos==fld[pathcolumn].length) ? fld.childrenCnt : 0;
                        if(!newobj.title)newobj.title=newobj.name;
                        if(!newobj.childId)newobj.childId=curT.children.length;
                        curT.children.push(newobj);
                        
                    } else if(fld[pathcolumn].length-pos <= 1   ){
                        for ( vfeat in fld ) newobj[vfeat]=fld[vfeat];
                        newobj.path=fld[pathcolumn];
                    } else { 
                        newobj.leafnode=false;
                    }
                    
                    if(!fld.autoNode){
                        delete newobj.autoNode;
                    }
                }
                else if(ifnd==curT.children.length ){

                    newobj.path=fld[pathcolumn].substring(0,pos+1);
                    if(fld[pathcolumn].length-pos<=1) {
                        for ( vfeat in fld )
                            newobj[vfeat]=fld[vfeat];
                        newobj.path=fld[pathcolumn];
                    }
                    newobj.name=thisNode;
                    newobj.children=new Array();
                    newobj.parent=parNode;
                    if(!newobj.tree_cnt_table_rows){
                        newobj.tree_cnt_table_rows=1;
                    }

                    newobj.leafnode=newobj.path.charAt(newobj.path.length-1)=="/" ? false : true;

                    if(!newobj.depth)newobj.depth=parNode.depth+1;
                    if(!newobj.distance)newobj.distance=1 ;
                    if(!newobj.childrenCnt)newobj.childrenCnt=(pos==fld[pathcolumn].length) ? fld.childrenCnt : 0;
                    if(!newobj.title)newobj.title=newobj.name;
                    if(!newobj.childId)newobj.childId=ifnd;
               
                    curT.children.push(newobj);
                    
                    if(!fld.treenode)
                        fld.treenode=newobj;
                    if(!newobj.tablenode)
                        newobj.tablenode=fld;

                }


                curT=newobj;

                prvpos=pos+1;
                
                if(prvpos>=fld[pathcolumn].length)break;

                pos=fld[pathcolumn].indexOf("/",prvpos);
                if(pos==-1)pos=fld[pathcolumn].length;
            }
        }
        return rootNode;
    };

    this.parseJson=function( jsontxt, node )
    {
        var t;
        eval("t="+jsontxt+";");
        if (!node)
            node = this.root;
        if(!node.children) {
            node.children=new Array();
            node.childrenCnt = 0;
        }

        for( var field in t ) {
            node[field]=t[field];
            node.children[node.childrenCnt]=t[field];
            ++node.childrenCnt;
        }

        return node;

    }

    this.parseNewick=function(newick, rootNode)
    {
        var rootTemp = this.newNode(rootNode ? rootNode : {name:"root", expanded:true});
        rootTemp.children = new Array();
        rootTemp.childrenCnt = 0;

        if (parseNewickWorker.call(this, newickSkipSpace(0), rootTemp) > 0) {
            this.deanonymize(rootTemp, rootTemp.name);
            if (rootNode) {
                for (var i in rootTemp)
                    rootNode[i] = rootTemp[i];
                for (var i=0; i<rootTemp.children.length; i++)
                    rootNode.children[i].parent = rootNode;
                return rootNode;
            } else {
                return rootTemp;
            }
        }
        return null;


        function newickSkipSpace(pos)
        {
            while (pos < newick.length - 1 && /\s/m.test(newick[pos]))
                pos++;
            return pos;
        };

        function newickEof(pos, atroot)
        {
            if (pos >= newick.length)
                return true;
            if (atroot && newick[pos] === ';')
                return true;
            return false;
        }

        function parseNewickWorker(start, node)
        {
            var atroot = (node === rootTemp);
            var pos = newickSkipSpace(start);

            if (newickEof(pos, atroot))
                return 0;

            if (newick[pos] === '(') {
                node.leafnode = false;
                do {
                    pos++;
                    var child = this.newNode({depth: node.depth+1, parent:node, expanded:node.expanded});
                    var posnext = parseNewickWorker.call(this, pos, child);
                    node.children.push(child);
                    node.childrenCnt++;

                    if (posnext <= pos)
                        return 0;
                    pos = newickSkipSpace(posnext);
                    if (newickEof(pos, atroot))
                        return 0;
                } while (newick[pos] == ',');
                if (newick[pos] === ')')
                    pos++;
                else
                    return 0;
            } else
                node.leafnode = true;

            pos = newickSkipSpace(pos);
            var name = "";
            if (pos < newick.length && (newick[pos] == "'" || newick[pos] == '"')) {
                var quote = newick[pos++];
                var esc = false;
                while (pos < newick.length && (esc || newick[pos] !== quote)) {
                    if (newick[pos] == '\\') {
                        esc = !esc;
                    } else {
                        esc = false;
                    }
                    name += newick[pos++];
                }
                pos = newickSkipSpace(++pos);
                name = parseCString(name);
            } else {
                while (pos < newick.length && newick[pos] !== ':' && newick[pos] !== ';' && newick[pos] !== '(')
                    name += newick[pos++];
                name = name.trim();
            }
            node.name = name;

            if (pos < newick.length && newick[pos] === ':') {
                pos = newickSkipSpace(++pos);
                var diststring = "";
                while (pos < newick.length && newick[pos] === '.' || newick[pos] === 'e' || newick[pos] === 'E' || newick[pos] === '-' || newick[pos] === '+' || (newick[pos] >= '0' && newick[pos] <= 9))
                    diststring += newick[pos++];
                node.distance = parseFloat(diststring, 10);
            }
            pos = newickSkipSpace(pos);
            if (atroot && !newickEof(pos, atroot))
                return 0;
            return pos;
        }
    };

    this.deanonymize=function(node, name)
    {
        if (!node.name)
            node.name = "" + name;
        if (!node.path) {
            if (node.parent && node.parent.path)
                node.path = node.parent.path + (node.parent.path[node.parent.path.length-1] === "/" ? "" : "/") + node.name;
            else
                node.path = "/";
        }
        for (var i=0; i<node.children.length; i++)
            this.deanonymize(node.children[i], i);
    };

    this.enumerate=function( operation, params , ischecked, leaforbranch , node)
    {
        if (!node)
            node = this.root;

        if ((!ischecked) || node.checked) {
            if (!leaforbranch || (leaforbranch == 1 && node.leafnode) || (leaforbranch == 2 && !node.leafnode)){
                if (typeof (operation) == "function")
                    operation(params, node, operation);
                else if (operation.indexOf("javascript:") == 0) {
                    eval(operation.substring(11))(params, this, node, operation);
                } else {
                    eval(operation);
                }
            }
        }

        if (node.children && node.children.length) {
            for ( var i = 0; i < node.children.length; ++i) {
                this.enumerate(operation, params, ischecked, leaforbranch,
                        node.children[i]);
            }
        }

    };

    this.findByPath=function( path, node,attr  )
    {
        if(attr === undefined)
            attr = "name";
        if(!node)node=this.root;
        var curT=node;
        if(path=="/")return node;

        var prvpos=1;
        var pos=path.indexOf("/",1);
        if(pos==-1)pos=path.length;
        while( pos!=-1) {
            var thisNode=path.substring(prvpos,pos);


            var ifnd=0;for( ifnd=0; ifnd<curT.children.length && curT.children[ifnd][attr]!=thisNode; ++ifnd )
                ;

            if(ifnd==curT.children.length)
                return ;

            parNode=curT;
            curT=curT.children[ifnd];
            prvpos=pos+1;

            if(prvpos>=path.length)break;
            pos=path.indexOf("/",prvpos);
            if(pos==-1)pos=path.length;
        }

        return curT;
    };

    this.findByName=function( nodename , node  )
    {
        if(!node)node=this.root;
        for( var i=0 ;i<node.children.length; ++i) {
            if(node.children[i].name==nodename) return node.children[i];
            var ret=this.findByName( nodename, node.children[i]) ;
            if(ret)return ret;
        }
        return ;
    };

    this.findByFieldName=function( fieldname , node  )
    {
        if(!node)node=this.root;
        for( var i=0 ;i<node.children.length; ++i) {
            if(node.children[i].fld.name==fieldname) return node.children[i];
            var ret=this.findByFieldName( fieldname, node.children[i]) ;
            if(ret)return ret;
        }
        return ;
    };
    
    this.findByAttribute=function( attribute, value , node  )
    {
        if(!node)node=this.root;
        for( var i=0 ;i<node.children.length; ++i) {
            if(node.children[i][attribute]==value) return node.children[i];
            var ret=this.findByAttribute( attribute, value, node.children[i]) ;
            if(ret)return ret;
        }
        return ;
    };

    this.accumulate=function( checker, collector , params, ischecked, leaforbranch , node )
    {
        if(!params)params=new Object();
        params.findByOperList=new Array();

        this.enumerate( "var res="+checker+";if(res)params.findByOperList.push("+collector+");", params, ischecked, leaforbranch , node );
        return params.findByOperList;

    };
    this.empty=function()
    {
        this.root = this.newNode({name:"root", expanded:true,path:"/"});
    };

    this.cmpNodes = function(node1, node2) {
        var ret = 0;

        if (node1 == node2) {
            return 0;
        }

        if (node1.depth > node2.depth) {
            ret = 1;
            while (node1.depth > node2.depth) {
                node1 = node1.parent;
            }
        } else if (node1.depth < node2.depth) {
            ret = -1;
            while (node1.depth < node2.depth) {
                node2 = node2.parent;
            }
        }

        while (node1.depth && node2.depth) {
            if (node1.parent == node2.parent) {
               if (node1.childId < node2.childId) {
                   return -1;
               } else if (node1.childId > node2.childId) {
                   return 1;
               } else {
                   return ret;
               }
            }
            node1 = node1.parent;
            node2 = node2.parent;
        }

        return ret;
    };

    if(tbl && tbl.rows && tbl.rows.length)
        this.root=this.parseTable(tbl , this.root , pathcolumn, excludeObjRegexp );

    if (typeof(tbl) == "string"){
        var tblNew = new vjTable(tbl, 0, vjTable_propCSV);
        this.root=this.parseTable(tblNew , this.root , pathcolumn, excludeObjRegexp );
    }
}

