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
/*
    http://www.iconfinder.com/search/2/?q=iconset%3Aie_bright
    node :
    {
        name: root,
        children: [] // array of children 
            
        // optional inputs
        childrenCnt: 1 , // declared number of children 
        distance: 1.23, declared distance to parent
        size: 2.3 ,  // size of the node 

        // input and state 
        expanded: true, // the expansion status of the node 

        // state 
        depth: 4, // computed depth of the node 
        parent: object, //  parent Node 
        highlight: 'blue', // the highlight color of the element

    }
    expansion: 1, // the variable showing the expansion incrment 
    hideEmpty:false, // show hide  empty nodes
    showLeaf:false, //  show hide leaf nodes
    scaleDist:20, // the scale for distances
    showRoot:true, // show/hide root node 
    autoexpand:1, // the number of levels to automatically expand
    tblclass: '', // the class for the style of the whole tree
    icons{ empty:, plus:, minus:, leaf}, // icons 
    selectedNode: /path,  // the path of the selection item
    showChildrenCount: true, // show/hide the count of children 
    selectionObject:'' ,  // the name of the DOM object where the selection is copied
    selectionColor : 'blue', // the color for selection elements 
    highlightColor : 'blue', // the color for highlights
    linkLeafCallback: 'javascript:function(viewer,node,nodepath) or http://', // link callbacks for leafs
    linkBrancCallback: 'javascript: or http://', // link callbacks for branches
    excludeList: '/system/', // the list of elements to hide 
            
*/

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/

vDV.generateTreeView=function( viewer , content ) 
{

    var o=gObject(viewer.container);if(!o)return ;
    if(!viewer.expansion)viewer.expansion=1;
    if(!viewer.scaleDist)viewer.scaleDist=20;
    if(!viewer.showRoot)viewer.showRoot=1;
    if(!viewer.autoexpand)viewer.autoexpand=1;
    if(!viewer.icons) viewer.icons=new Object();
    if(!viewer.icons.empty)viewer.icons.empty='img/folderEmpty.gif';
    if(!viewer.icons.plus)viewer.icons.plus='img/folderPlus.gif';
    if(!viewer.icons.minus)viewer.icons.minus='img/folderMinus.gif';
    if(!viewer.icons.leaf)viewer.icons.leaf='img/user.gif';
    viewer.tree=vDV.parsePathTreeView(viewer, content , 0, viewer.autoexpand ) ; 
    vDV.redrawTreeView(viewer);
}

vDV.redrawTreeView=function(viewer)
{
    gObject(viewer.container).innerHTML=vDV.outputNodeTreeView(viewer,viewer.tree);
}

vDV.parsePathTreeView=function( viewer, content , rootNode, autoexpand) 
{
    var arr=content.split("\n");
    
    if(!rootNode) rootNode=new Object ({name:'root',  depth:0 , expanded: true, childrenCnt: 0, children: new Array() } );
    if(!rootNode.depth)rootNode.depth=0;

    var exclList;
    if(viewer.excludeList)exclList=viewer.excludeList.split(",");
    
    var curT;
    for ( var ia=0; ia<arr.length; ++ia){
        curT=rootNode;

        if(arr[ia].length==0)continue;
        var t=arr[ia].split(",");
        var path=t[0],j;
        
        if(viewer.excludeList) {
            for( j=0;j<exclList.length && path.indexOf(exclList[j])!=0; ++j) ;
            if(j<exclList.length)continue;
        }
        
        var nam=t.length>1 ? t[1] : path;
        var distance=t.length>2 ? t[2] : 1;
        var childrenCnt=t.length>3 ? t[3] : 0;
        var description=t.length>4 ? t[4] : '';
        
        var prvpos=1;
        for ( var pos=path.indexOf("/",1); pos!=-1; ) {
            var thisNode=path.substring(prvpos,pos);

            var ifnd=0;for( ifnd=0; ifnd<curT.children.length && curT.children[ifnd].name!=thisNode; ++ifnd ) ; // find this node

            parNode=curT;
            var newobj=curT.children[ifnd];
            if(ifnd==curT.children.length){
                curT.children[ifnd]=new Object ({name: thisNode, childrenCnt: childrenCnt, children: new Array() } ) ;
                newobj=curT.children[ifnd];
                newobj.depth=parNode.depth+1;
                newobj.path=path.substring(0,pos+1);
                newobj.parent=parNode;
                newobj.distance=distance;
                newobj.description=description;
                newobj.leaf=newobj.path.charAt(newobj.path.length-1)=="/" ? false : true ;
                newobj.childrenCnt= (pos==path.length) ? childrenCnt : 0;
                if(autoexpand && curT.children[ifnd].depth<=autoexpand)curT.children[ifnd].expanded=viewer.expansion;
            }
            
            curT=newobj;
            prvpos=pos+1;
            
            if(prvpos>=path.length)break;

            pos=path.indexOf("/",prvpos);
            if(pos==-1)pos=path.length;
        }

        if(nam.indexOf("id")==0){
            curT.id=parseInt(nam.substring(2));
            nam=curT.name;
        }
        if(!curT.title)curT.title=nam;
    }
    
    return rootNode;
}

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ tree manipulations
// _/
// _/_/_/_/_/_/_/_/_/_/_/

vDV.findPathTreeViewFind=function( rootNode, path ) 
{
    var curT=rootNode;
    
    var prvpos=1;
    for ( var pos=path.indexOf("/",1); pos!=-1; ) {
        var thisNode=path.substring(prvpos,pos);

        var ifnd=0;for( ifnd=0; ifnd<curT.children.length && curT.children[ifnd].name!=thisNode; ++ifnd ) ; // find this node
        
        if(ifnd==curT.children.length)
            return ;
        
        parNode=curT;
        curT=curT.children[ifnd];
//        curT.depth=parNode.depth+1;

        prvpos=pos+1;
        
        if(prvpos>=path.length)break;
        pos=path.indexOf("/",prvpos);
        if(pos==-1)pos=path.length;
    }

    return curT;
}

vDV.findNodeTreeView=function( node , nodename ) 
{
    //if( node.name==nodename) return node; 

    for( var i=0 ;i<node.children.length; ++i) {
        if(node.children[i].name==nodename) return node.children[i];
        var ret=vDV.findNodeTreeView( node.children[i], nodename ) ;
        if(ret)return ret;
    }
    return ;
}

vDV.expantNodeTreeView=function( thisNode , state, withParents) 
{
    if(state==-1)thisNode.expanded=-thisNode.expanded;
    else if(state>=0)thisNode.expanded=state;
    if(!withParents)return;
    
    while(thisNode.parent) {
        thisNode=thisNode.parent;
        if(state==-1)thisNode.expanded=-thisNode.expanded;
        else if(state>=0)thisNode.expanded=state;
    }
    return thisNode; // returns the root
}

/*vDV.rootNodeTreeView=function( thisNode ) 
{
    return vDV.expantNodeTreeView( thisNode , -2, false) ;
}
*/


vDV.enumerateNodesTreeView=function( viewer , node, operation, ischecked, leaforbranch ) // leaforbranch==0 : all , =1 leafs , ==2 branches 
{
    var cntCh=node.children.length;
    if(node.childrenCnt>0)cntCh=node.childrenCnt;

    if( (!ischecked) || node.checked) {
        if( !leaforbranch || (leaforbranch==1 && node.leaf) || (leaforbranch==2 && !node.leaf) )
        if(operation.indexOf("callback:")==0)eval(operaton.substring(0,9)+"("+viewer+","+node+","+operation+")")
        else eval(operation);
    }
    
    if( node.children.length ){ // && node.expanded 
        for( var i=0 ;i<node.children.length; ++i) {
           vDV.enumerateNodesTreeView ( viewer , node.children[i], operation, ischecked, leaforbranch ) ;
        }
    }
}
// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ event handlers
// _/
// _/_/_/_/_/_/_/_/_/_/_/

vDV.onClickExpandTreeView=function(state, event, container, nodepath )
{

    var o=gObject(container);if(!o)return ;

    var viewer=vDV.findviewByContainer(container);
    var root=viewer.tree;
    var node=vDV.findPathTreeViewFind(root, nodepath);
    
    vDV.expantNodeTreeView( node, state, false) ;

    o.innerHTML=vDV.outputNodeTreeView(viewer,viewer.tree);
}

vDV.onClickNodeTreeView=function(event, container, nodepath )
{
    var viewer=vDV.findviewByContainer(container);
    var node=vDV.findPathTreeViewFind(viewer.tree, nodepath);

    viewer.selectedNode=nodepath;

    if(viewer.selectionObject && viewer.selectionObject.length!=0 ) {

        var o=gObject(viewer.selectionObject);if(o && o.innerHTML )o.innerHTML=nodepath;
        o=gObject(viewer.selectionObject+"_title");if(o && o.innerHTML )o.innerHTML=node.title ? node.title : node.name ;
        if(viewer.formObject && document.forms[viewer.formObject]) {
            o=document.forms[viewer.formObject].elements[viewer.selectionObject];
            if(o)o.value=nodepath;
            o=document.forms[viewer.formObject].elements[viewer.selectionObject+"_title"];
            if(o)o.value=node.title ? node.title : node.name ;
        }
    }


    var linkCB;
    if(node.leaf && viewer.linkLeafCallback)linkCB=viewer.linkLeafCallback;
    else if(viewer.linkBranchCallback)linkCB=viewer.linkBranchCallback;
    
    if(linkCB) {
        if(linkCB.indexOf("javascript:")==0)
            eval(linkCB.substring(11))(viewer,node) ;
        else
            document.location=linkLeafCallback; 
    }

    
    if(viewer.selectionColor)
        gObject(container).innerHTML=vDV.outputNodeTreeView(viewer,viewer.tree);
    
}

vDV.onCheckNodeTreeView=function(event, container, nodepath )
{
    var viewer=vDV.findviewByContainer(container);
    var node=vDV.findPathTreeViewFind(viewer.tree, nodepath);
    
    vDV.enumerateNodesTreeView( viewer , node, "node.checked="+(node.checked ? "false" : "true" )+";" );
    if(node.parent)node.parent.checked = 0 ;
    gObject(container).innerHTML=vDV.outputNodeTreeView(viewer,viewer.tree);
}

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ generators
// _/
// _/_/_/_/_/_/_/_/_/_/_/

vDV.outputNodeTreeView=function( viewer , node ) 
{
    if(!viewer.showLeaf && node.leaf) return "";
    
    var cntCh=node.children.length;
    if(node.childrenCnt>0)cntCh=node.childrenCnt;

    if(viewer.hideEmpty && !node.leaf && cntCh<1 ) return "";
    var t="";
    var linkCB;
    if(node.leaf && viewer.linkLeafCallback ) linkCB=viewer.linkLeafCallback;
    else if(viewer.linkBranchCallback )linkCB=viewer.linkBrancCallback;
    
    if(viewer.showRoot<=node.depth) { 
        t="<table border=0 ";
        if(viewer.tblclass)t+="class='"+viewer.tblclass+"'";
        t+="><tr>";
        t+="<td width="+ (viewer.scaleDist*(node.distance ? node.distance : 1) )+" align=right>";
            if(!node.leaf ){
                if(cntCh>=1){
                    t+="<a href='javascript:vDV.onClickExpandTreeView("+ (node.expanded>=viewer.expansion ? 0 : viewer.expansion )+",event,\""+viewer.container+"\", \""+node.path+"\")'>";
                    t+="<img border=0 width=24 height=24 src='"+( node.expanded>=viewer.expansion ? viewer.icons.minus : viewer.icons.plus)+"' />";
                    //t+=node.expanded>=viewer.expansion ? "-" : "+";
                    t+="</a>";
                }
                else t+="<img border=0 width=24 height=24 src='"+viewer.icons.empty+"' />";
            }else {
                t+="<img border=0 width=24 height=24 src='"+viewer.icons.leaf+"' />";
            }
        t+="</td><td "+ ( (viewer.selectedNode && node.path==viewer.selectedNode && viewer.selectionColor) ? (" bgcolor="+viewer.selectionColor) : "" ) + (viewer.clsStyle ? (" class='"+viewer.clsStyle+"'") : "") + " >";
            if(viewer.selectionObject || linkCB ) 
                t+="<a href='javascript:vDV.onClickNodeTreeView(event,\""+viewer.container+"\", \""+node.path+"\")'>";
            if( (viewer.checkBranches && !node.leaf) || (viewer.checkLeafs && node.leaf) )  {
                var checkstate=node.checked ? "checked='checked'" : "" ;
                //if(node.checked==-1) checkstate+=" interminate=true " ;
                t+="<input type=checkbox "+checkstate +" id='"+viewer.container+"_check_"+node.path+"' onclick='vDV.onCheckNodeTreeView(event,\""+viewer.container+"\", \""+node.path+"\")' />&nbsp;";
            }

            if( !viewer.showName && node.title)t+=node.title;
            else t+=node.name;
            if(cntCh>=1 && viewer.showChildrenCount)t+="&nbsp;<small><b>["+cntCh+"]</b></small>";
            if(viewer.selectionObject || linkCB ) 
                t+="</a>";
        t+="</td></tr>";
    }

    var sectNam=viewer.container+"_"+node.name;
    if(node.expanded && cntCh>=1 ){
        if(viewer.showRoot<=node.depth) 
            t+="<tr><td></td><td id='"+sectNam+"'>";
        if( cntCh>=1 ) {
            for( var i=0 ;i<node.children.length; ++i) {
               t+=vDV.outputNodeTreeView ( viewer , node.children[i] ) ;
            }
        }
        if(viewer.showRoot<=node.depth) 
            t+="</td></tr>";
    }
    if(viewer.showRoot<=node.depth) {
        t+="</table>";
    }
    
    return t;
}





// _/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Registration
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/

vDV.registerViewer( "treeview", vDV.generateTreeView) ;

