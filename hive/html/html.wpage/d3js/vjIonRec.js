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




function vjIonStruc ( content )
{
    jsonSetDefaults( this,{
        pagingSizes: [5,10,100,1000,"all"],
        pagingSizeSelect:5,
    }) ;
    this.searchDic={};
    
    this.buildRecData=function( content )
    {
        if(this.debug)alert(content);

        var node;
        if(typeof(content)=="string") {
            eval("node="+content+";");
            if(!node.__sub) {
                alert("no elements");
                return 0;
            }
        }
        else { 
            node=content;
        }
        
        
        var attach=jsonFindField(this.root, this.v_nodeReload,true);
        
        {
            var newatt={};
            for ( f in attach )  {if(this.v_nodeAppendMode || f.indexOf("__")==0 )newatt[f]=attach[f];}
            attach=newatt;
            for( var f in node) {attach[f]=node[f];}
            var cnt=0;
            for ( var f in attach )  {if(f.indexOf("__")==0 )++cnt;}
            attach.__cnt=cnt;
        }
        if(!this.root) {this.root=attach;this.root.__name="$root";this.root.__path="";}
        
                        
        this.iterateJsonFixArr(this.root);
        
        this.v_nodeReload=null;
        return attach;
    };
    
    this.search=function(nodePath,ovalue)
    {
        if(this.v_nodeReload)return 1;
        var node  = this.findeNodeByPath(this.root, nodePath, true); if(!node)return;
        this.v_nodeReload=nodePath;
        
        var url=this.getData(0).url;
        if(oovalue[0]==':') {
            url=urlExchangeParameter(url, "brInto", escape(ovalue.substring(1)));
            url=urlExchangeParameter(url, "brSearch", "-" );
        } else { 
            url=urlExchangeParameter(url, "brSearch", escape(ovalue));
            url=urlExchangeParameter(url, "brInto", "-" );
        }
        url=urlExchangeParameter(url, "sub", node.__sub);
        this.searchDic[node.__path]=ovalue;
        this.getData(0).reload(url,true);
        return 1;
    }

    
    this.page=function(nodePath,direction, pagesize, after)
    {
        if(this.v_nodeReload)return 1;
        var node  = this.findeNodeByPath(this.root, nodePath, true);
        this.v_nodeReload=nodePath;
        
        if(pagesize)node.__pagingSizeSelect=pagesize;
        
        var url=this.getData(0).url;
        var pgCnt=node.__pagingSizeSelect ? node.__pagingSizeSelect : this.pagingSizeSelect ;
        
        if(direction==1) { 
            var brStart=node.__start+node.__cnt;if(brStart>node.__dim-node.__cnt)brStart=node.__dim-node.__cnt;
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
        }else if(direction=="size"){ 
            url=urlExchangeParameter(url, "sub", node.__sub);
        }
        url=urlExchangeParameter(url, "brCnt", pgCnt );
        if(after)
            url=urlExchangeParameter(url, "brCntAfter", pgCnt);    
        
        this.getData(0).reload(url,true);
        return 1;
    }
    

    
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
                node[f].__path+"."+f;
                node[f].__name=f;
                node[f].__depth=node.__depth+1;
            }
        }
    }

    this.findeNodeByPath=function(start, nodePath )
    {
        var node  = jsonFindField(start, nodePath, true);
        if(!node)return this.root;
        else return node.node;
    }    

    if(content) {
        this.buildRecData(content);
        return this.root;
    }

}

