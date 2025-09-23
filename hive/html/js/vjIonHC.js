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

function vjIonHCView ( viewer )
{
    loadCSS("css/hivesys.css");
    vjDataViewViewer.call(this,viewer);
    
    this.onRefreshDoneCallback = viewer.onRefreshDoneCallback;
    
    setDefaults( this,{
        className : "JSON_TREE",
        lastSearch: "",
        pagingSizes: [20,50,100,1000,"all"],
        pagingSizeSelect:20,
        icons: {empty: 'img/tree-white_minus_slim_dotted.png', leaf : 'img/recItem.gif' , size : 16 },
        autoExpand : 2
    }) ;

    this.searchDic={};

    
    this.composerFunction=function( viewer , content )
    {
        if(this.debug)alert(content);

        var node;
        eval("node="+content+";");
        if(!node.__sub) {
            alert("no elements");
            return ;
        }
        
        if(!this.root) {
            this.root=node;
            this.root.__depth=0;
            this.root.__name="$root";
            this.v_nodeReload=this.root;
            this.root.__path="";
            this.refresh();
        }
        else {
            
            var attach=findJsonField(this.root, this.v_nodeReload,true);    
            if(!attach)attach={ parent: this , field : "root" , node: this.root } ;
            
            if(this.v_nodeAppendMode) {
                for ( f in node )  {
                    if(f=="__cnt")attach.node[f]+=node[f];
                    else attach.node[f]=node[f];
                }
            }else {
                
                if(node.__sub!="$root") {
                    node.__path=attach.parent.__path ? attach.parent.__path+"."+attach.field : attach.field ;
                    node.__name=attach.field;
                    node.__depth=attach.parent[attach.field].depth;
                }else {
                    node.__path="";
                }
                
                if(attach.node.__pagingSizeSelect)
                    node.__pagingSizeSelect=attach.parent[attach.field].__pagingSizeSelect;
                if(!node.__pagingSizeSelect)
                    node.__pagingSizeSelect=this.pagingSizes[0];                
                node.__collapsed=attach.parent[attach.field].__collapsed;
                
                if(node.__cntChildren==undefined) {
                    node.__cntChildren=0;
                    for( var f in node) { 
                        if(f.indexOf("__")==0)continue;
                        ++node.__cntChildren;
                    }
                }
                attach.parent[attach.field]=node;
            }

            var t=this.outputNodeChildren(attach.parent[attach.field],-1);
            if (node.__path == "root"){
                gObject(this.div.id).children[0].innerHTML=t;
            }else{
                gObject((this.div.id+node.__path)+"-children").innerHTML=t;
            }
            this.v_prvMouseOverNode=undefined;
            
            this.doNodeControlText(node);
            this.doNodeInfoText(node);
        }
        
        this.iterateJsonFixArr(this.root);
        
        this.v_nodeReload=null;
        
        if (this.onRefreshDoneCallback)
            this.onRefreshDoneCallback (node);
    };

    this.iterateJsonFixArr=function(node) 
    {
        for( var f in node) { 
            if( node[f] instanceof Object ) {
                if(f.indexOf("__[]")==0){
                    var cn=node[f.substring(4)];

                    cn.__dim=node[f].__dim;
                    cn.__start=node[f].__start;
                    cn.__cnt=node[f].__cnt;
                    cn.__tot=node[f].__tot;
                }
                
                this.iterateJsonFixArr(node[f]);
            }
        }
    }
    
    
    
    this.refresh=function(node)
    {
        if( !node ) { 
            if(this.renderStartNodePath)  { 
                node=findJsonField(start, this.renderStartNodePath , true);
                if(!node)node=this.root;else node=node.node;
            }
            else 
                node = this.root;
        }

        var t="";
        if(this.prefixHTML && node==this.root )
            t+=this.prefixHTML;
        
        t+="<ul class='"+this.className+"'><li class='"+this.className+"_last' ";
        t+=" onClick='vjObjEvent(\"onCollapseNode\",\""+this.objCls+"\",\""+(node.__path)+"\");' ";
        t+=" onMouseOver='vjObjEvent(\"onMouseOver\",\""+this.objCls+"\",\""+(node.__path)+"\",1);' ";
        t+=">"+this.outputNode( node ,-1)+"</li></ul>";
        
        if (this.appendHTML && node==this.root ) 
            t += this.appendHTML;

        gObject((this.div.id + this.root.__path)).innerHTML=t;
    };

    
    this.findeNodeByPath=function(start, nodePath )
    {
        var node  = findJsonField(start, nodePath, true);
        if(!node)return this.root;
        else return node.node;
    }
    
    this.onSearchChange=function(cls,nodePath)
    {
        this.grabMouse=false;
        if(this.v_nodeReload)return 1;
        var node  = this.findeNodeByPath(this.root, nodePath, true);
        this.v_nodeReload=nodePath;
        
        var o = gObject((this.div.id+node.__path)+"-search");if(!o)return 0;
        this.lastSearch=o.value;
        var url=this.getData(0).url;
        if(o.value[0]==':') {
            url=urlExchangeParameter(url, "brInto", escape(o.value.substring(1)));
            url=urlExchangeParameter(url, "brSearch", "-" );
        } else { 
            url=urlExchangeParameter(url, "brSearch", escape(o.value));
            url=urlExchangeParameter(url, "brInto", "-" );
        }
        url=urlExchangeParameter(url, "sub", node.__sub);
        this.searchDic[node.__path]=o.value;
        this.getData(0).reload(url,true);
        return 1;
    }
    
    this.onPage=function(cls,nodePath,direction)
    {
        this.grabMouse=false;
        if(this.v_nodeReload)return 1;
        var node  = this.findeNodeByPath(this.root, nodePath, true);
        
        this.v_nodeReload=nodePath;
        var url=this.getData(0).url;
        var pgCnt=node.__pagingSizeSelect ? node.__pagingSizeSelect : this.pagingSizeSelect ;
        
        if(direction==1) { 
            var brStart=node.__start+node.__cnt;if(brStart>=node.__dim)brStart=node.__dim-node.__cnt;
            url=urlExchangeParameter(url, "brStart", brStart);
            url=urlExchangeParameter(url, "sub", node.__sub);
        }
        else if(direction==-1) {
            var brStart=node.__start-node.__cnt;if(brStart<0)brStart=0;
            url=urlExchangeParameter(url, "brStart", brStart);
            url=urlExchangeParameter(url, "sub", node.__sub);
        }
        else if(direction==0) { 
            pgCnt=0;
            url=urlExchangeParameter(url, "brStart", 0);
            url=urlExchangeParameter(url, "sub", node.__sub);
        }else { 
            url=urlExchangeParameter(url, "sub", node.__sub);
        }
        url=urlExchangeParameter(url, "brCnt", pgCnt );
        if(gKeyShift)
            url=urlExchangeParameter(url, "brCntAfter", pgCnt);    
        
        this.getData(0).reload(url,true);
        return 1;
    }
    
    this.onPagingSizeSelect=function(cls, nodePath)
    {
        this.grabMouse=false;
        var node  = this.findeNodeByPath(this.root, nodePath, true);
        node.__pagingSizeSelect=event.srcElement.value;
        this.onPage(cls,nodePath,node.__pagingSizeSelect);
    }

    this.onCollapseNode=function(cls,nodePath)
    {
        this.grabMouse=false;  
        var node  = this.findeNodeByPath(this.root, nodePath, true);
        
        var o = gObject((this.div.id+node.__path)+"-children");
                        
        var ochld = gObject((this.div.id+node.__path)+"-children-info");
        if(o) { 
            if( !node.__collapsed ) {
                node.__collapsed=o.className;
                o.className="sectHid";
                if(ochld)ochld.className=node.__collapsed;
            } else {
                o.className=node.__collapsed;
                node.__collapsed=false;
                if(ochld)ochld.className="sectHid";
            }
        }
        this.v_prvMouseOverNode=undefined;
        this.doNodeControlText(node);
        
        if (this.clickNodeCallback) this.clickNodeCallback( this, nodePath);
        
        return true;
    }
    
    
    this.onMouseOver=function(cls,nodePath,what)
    {
        if(this.v_paginmode)return 1;
        if(this.grabMouse)return 1;
        var node  = this.findeNodeByPath(this.root, nodePath, true);
        
        if(this.v_prvMouseOverNode==node.__path)return 1;
        this.doNodeControlText(node);
                    
        return 1;
    }
    
    this.onClickInput=function(cls,nodePath,what){this.grabMouse=true;}
    this.doReleaseMouse=function(cls,nodePath,what){this.grabMouse=false;}
    gOnDocumentClickCallbacks+="vjObjEvent(\"doReleaseMouse\",\""+this.objCls+"\");";
    

    this.doNodeControlText=function(node)
    {
        var o = gObject((this.div.id+node.__path)+"-children-page");if(!o)return 1;
        
        o.innerHTML=this.nodeControlText(node);
        
        if( !(this.v_prvMouseOverNode===undefined) && gObject((this.div.id+this.v_prvMouseOverNode)+"-children-page")) {
            gObject((this.div.id+this.v_prvMouseOverNode)+"-children-page").innerHTML="";
        }
        this.v_prvMouseOverNode=node.__path; 
            
        return 1;
        
    }

    this.doNodeInfoText=function(node)
    {
        var o = gObject((this.div.id+node.__path)+"-children-info");if(!o)return 1;
        
        o.innerHTML=this.nodeInfoText(node);
        return 1;
        
    }
    
    this.nameManipulate=function(node,name)
    {
        return (name[0]=='_') ? "<small>&equiv;</small>"+name.substring(1) : name;
    }
    
    


    this.outputNode=function( node , v_maxLevel )
    {

        if( ( node.__invisible ) ||
            ( this.hideEmpty && !node.__cntChildren ) || 
            ( this.showDepth && node.__nodeDepth && this.showDepth<node.__nodeDepth )
            ) return "";

        if(node.__cntChildren==undefined) {
            node.__cntChildren=0;
            for( var f in node) { if(f.indexOf("__")==0)continue;
                ++node.__cntChildren;
            }
        }
        if(this.autoExpand>0 && node.__depth>this.autoExpand) {  
            node.__collapsed=this.className;
        }
        
        var t="";
        t+=this.nameManipulate(node,node.__name);
        t+="&nbsp;";
        t+="<span class='";
        t+=(node.__collapsed) ? this.className : "sectHid";
        t+="' id='"+(this.div.id+node.__path)+"-children-info'>"+this.nodeInfoText(node)+"</span>";
        if(node.__cntChildren)  {
            t+="&nbsp;<span id='"+(this.div.id+node.__path)+"-children-page'>";
            if(node.__path==this.v_nodeReload)t+=this.nodeControlText(node);
            t+="</span>";
            
            t+="<ul class='"
            
            if(node.__collapsed) { 
                t+="sectHid";
            }else 
                t+=this.className;
            t+="' ";
            t+="id='"+(this.div.id+node.__path)+"-children'";
            t+=" >";

            if(v_maxLevel)
                t+=this.outputNodeChildren( node , v_maxLevel  );            
            t+="</ul>";
        }
        
        
        return t;
    };

    this.outputNodeChildren=function( node , v_maxLevel  )
    {
        var icnt=0;
        var t="";
        for( var f in node ) {if(f.indexOf("__")==0)continue;
            var prefix= (node.__path) ? (node.__path+".") : "" ;
            
            t+="<li ";
            t+=" id='"+(this.div.id+node.__path+f)+"' ";
            if(icnt==node.__cntChildren-1){
                t+="class='"+this.className+"_last' ";
            }
            t+=" onClick='vjObjEvent(\"onCollapseNode\",\""+this.objCls+"\",\""+(prefix+f)+"\");' ";
            t+=" onMouseOver='vjObjEvent(\"onMouseOver\",\""+this.objCls+"\",\""+(prefix+f)+"\",1);' ";
            t+=">";
            
            var cn=node[f];
            
            if( typeof(cn)=="object" && f.indexOf("__[]")!=0 ){
                cn.__depth=node.__depth+1;
                cn.__name=f;
                cn.__path=prefix+cn.__name;
                if(v_maxLevel)
                    t+=this.outputNode ( cn , v_maxLevel==-1 ? v_maxLevel : v_maxLevel-1 ) ;
            } else {
                t+=this.nameManipulate(node,f);
                t+="&nbsp;:&nbsp;";
                if(f.indexOf("descr")==0 || f.indexOf("comment")==0)t+="<textarea onClick='return 0;' rows=2 class='"+this.className+"-input' type='text' cols=40 />"+(cn)+"</textarea>";
                else t+="<input onClick='return 0;' class='"+this.className+"-input' type='text' size=40 value='"+(cn)+"' />";
                
                
            }
            
            t+="</li>";
            ++icnt;
        }
        return t;
    }
    
    

    this.nodeControlText=function(node)
    {
        var t="";
        if( node instanceof Array  ) {
        }
        if(node.__cntChildren)t+="<small>"+(node.__collapsed ? "&#x21e9;" : "&#x21ea;")+"</small>";
        t+="<small><span onChange='javascript:vjObjEvent(\"onSearchChange\",\""+this.objCls+"\",\""+(node.__path)+"\");' >?</span></small>"
        if(node.__collapsed)
            return t;
        
        if(node.__start)t+="<span class='"+this.className+"' onClick='javascript:vjObjEvent(\"onPage\",\""+this.objCls+"\",\""+(node.__path)+"\",-1);' >&#x25C4;</span>";
        else t+="<span class='"+this.className+"' onClick='return eventStop(event);' style='color:white' >&#x25C4;</span>";
        if(node.__cnt!=1)t+="<small><small>&nbsp;"+(node.__start+1)+"-"+(+node.__start+node.__cntChildren)+" of "+node.__dim+"</small></small>&nbsp;";
        if(node.__cnt<node.__dim){
            if(this.pagingSizes && this.pagingSizes.length){
                t+="<small><select class='"+this.className+"-control' style='width:48px' "+
                    "onChange='javascript:vjObjEvent(\"onPagingSizeSelect\",\""+this.objCls+"\",\""+(node.__path)+"\",-1);' "+
                    "onClick='if(event.stopPropagation)event.stopPropagation();event.cancelBubble=true;return 0;' "+
                    " >";
                for( var i=0; i<this.pagingSizes.length; ++i ){
                    t+="<option value='"+this.pagingSizes[i]+"' ";
                    if(node.__pagingSizeSelect==this.pagingSizes[i])t+=" selected ";
                    else if(this.pagingSizeSelect==this.pagingSizes[i])t+=" selected ";
                    t+=" >";
                    t+=this.pagingSizes[i];
                    t+="</option>";
                }
                t+="</select></small>";
            }

        }
        if(node.__start + node.__cnt <node.__dim)t+="<span class='"+this.className+"' onClick='javascript:vjObjEvent(\"onPage\",\""+this.objCls+"\",\""+(node.__path)+"\",+1);' >&#x25BA;</span>";
        t+="&nbsp;<input class='"+this.className+"-control' id='"+(this.div.id+node.__path)+"-search'  "+
            "onChange='javascript:vjObjEvent(\"onSearchChange\",\""+this.objCls+"\",\""+(node.__path)+"\");' "+
            "onClick='javascript:vjObjEvent(\"onClickInput\",\""+this.objCls+"\",\""+(node.__path)+"\");if(event.stopPropagation){event.stopPropagation();}event.cancelBubble=true;return 0;' "+
            "class='"+this.className+"-search' type='text' size=40 value='"+(this.searchDic[node.__path] ? this.searchDic[node.__path] : this.lastSearch)+"' />&nbsp;";
    
        
        return t;
    }

    this.nodeInfoText=function(node)
    {
        return t="";
    }    

}

