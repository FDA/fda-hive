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


var vDS = new Array ();
var vDS_reuseTblBlobs=0;
var vDS_raw=1;
var debug=0;
vDS.add=function( name, url, callbacks,  header )
{
    if(vDS[name])  {
        vDS[name].url=url;
        vDS[name].callbacks=callbacks;
        vDS[name].header=header;
        return;
    }

    var ds={
        'name' : name,
        'url' : url,
        'header' : header,
        'parser': '',
        'data' : '',
        'page_request' : '',
        'callbacks' : callbacks ,
        'evaluator': false,

        'load': function (loadmode){return vDS.load(this,loadmode);}, 
        'refresh': function (){return vDS.refresh(this);},
        'hook' : function(  cbfun ) { return vDS.hook(this, cbfun); },
        'unhook' : function(  cbfun ) { return vDS.unhook(this, cbfun); },
        
        'progressor' :'QP_bgProgress',
        'dataurl': '',
        'state' : ''
    };

    vDS[name]=ds;
    return ds;
};

vDS.refresh=function( ds ) 
{
    ds.callbacks=verarr(ds.callbacks); 
    if(debug=='flow')alert( "CALLBACING: "+ds.name +"\n"+"url="+ds.url + "\n"+ds.state+"\n\n\n"+jjj(ds.callbacks));
    for ( var i=0 ; i<ds.callbacks.length; ++i) {
        var func=eval(ds.callbacks[i].func);
        if( isok(func) )
            func( ds.callbacks[i].obj, ds.callbacks[i].param , ds.data, ds.page_request );
        else if( isok(ds.callbacks[i].obj) && isok(ds.callbacks[i].obj.refresh) )
            ds.callbacks[i].obj.refresh( ds.callbacks[i].obj, ds.callbacks[i].param , ds.data, ds.page_request); 
        else{
            eval(ds.callbacks[i])( ds.callbacks[i], ds.data, ds.page_request );
        }
    }
};

vDS.hook=function (ds, cbfun )
{
    if(!ds)return ;

    ds.callbacks=verarr(ds.callbacks) ;
    ds.callbacks[ds.callbacks.length]=cbfun;
};

vDS.unhook=function (ds, cbfun )
{
    for( var ic=0; ic<ds.callbacks.length; ++ic) {
        if( ds.callbacks[ic] == cbfun)
            ds.callbacks=ds.callbacks.splice(ic,1);
    }
};

vDS.load_callback = function (dsname, text, page_request) {
    var ds = vDS[dsname];
    if (isok(ds.header)) ds.data = ds.header + "\n" + text;
    else ds.data = text;
    ds.page_request = page_request;
    ds.state = 'done';
    if (debug == 'flow') alert("FiNISHED: " + ds.name + "\n" + "url=" + ds.url + "\n" + ds.state);
    if (!isok(ds.data)) { ds.state = 'err'; return; }

    if (ds.evaluator) {
        ds.data = evalVars(ds.data, "<!--", "
        
    }
    if (isok(ds.parser)) {
        var parsed = eval(ds.parser)(ds, ds.data);
        if (!parsed) return;
        ds.data = parsed;
    }

    vDS.refresh(ds);
};

vDS.load=function( ds , loadmode )
{

    if(loadmode!='download') {

        if(debug=='flow')alert( "INQUIRY: "+ds.name +"\n"+"url="+ds.url +"\n"+"dataurl="+ds.dataurl+ "\n"+ds.state);

        if( isok(ds.data) && isok(ds.dataurl) && ds.dataurl==ds.url && (ds.state=='done' || ds.state=='err') ){
            if(debug=='flow')alert( "LAZY: "+ds.name +"\n"+"url="+ds.url + "\n"+ds.state);
            return true;
        }
        if(ds.state=='load' &&  ds.dataurl==ds.url )
            return true;

        if(debug=='flow')alert( "LOAD: "+ds.name +"\n"+"url="+ds.url + "\n"+ds.state);
        ds.state='load';
        ds.dataurl=ds.url;
        
    }
    
    
    var spl=ds.url.split(":
    var src=spl[0];
    var url=spl[1]; 
    
    url=evalVars(url,"$(",")");
    
    if(src.indexOf("qpbg_")==0 ){
        spl=url.split("
        dataname=spl[0];
        url=spl[1];
    }

    var dataReq=0;
    if(src=="qpbg_tblqry"){
        dataReq=req;
        if(loadmode!='download' && QPride_findDataName( "Tbl-"+dataname )){
            src="qp_data";
            url="Tbl-"+dataname;
        } else {
            url="-qpSubmit&svc=tblqry&oper=list&tbl="+dataname+"&dataGrpID="+req+"&resID="+req+"&"+url;
            dataname="Tbl-"+dataname;
            src="qpbg_data";
        }
    }
    

    if( loadmode=='download' && src.indexOf("qpbg_")!=0 )  {
        if(src=='qp_data')url="?cmd=-qpData&req="+req+"&grp=1&raw=1&dname="+url;
        document.location=url;
        return ;
    }


    if(src =="http" ){
        ajaxDynaRequestPage(url, ds.name , vDS.load_callback );
        return true;
    } else if(src=="innerHTML")  {
        var o=document.getElementById(url);
        if(o)vDS.load_callback(ds.name ,o.innerHTML);
        return true;
    } else if(src=='static') {
        vDS.load_callback( ds.name ,url);
        return true;
    } else if(src=="qp_data") {
        ajaxDynaRequestPage("?cmd=-qpData&req="+req+"&raw=1&grp=1&dname="+url, ds.name ,  vDS.load_callback );
        return true;
    } 

    if(src.indexOf("qpbg_")==0 ) {
        QPride_backgroundExecution(url, verobj(ds.progressor) , (loadmode=='download')  ? "download" : "vDS.load_callback" , dataname , ds.name, dataReq  );
        return true;
    }
    return false;
};


vDS.forceload=function (dsprefix)
{
    for (var i=0 ; i< vDS.length; ++i) { 
        if( !isok(dsprefix) || vDS[i].name.indexOf(dsprefix)==0) {
            vDS[i].state='';
        }
    }
};



var vDV = new Array ();
var vDV_registeredViewers=new Array();

vDV.add=function( name, width, height , maxtabs )
{
    var dv={
        'name': name, 
        'container' : name,
        'tabs': new Array (),
        'width': (width ? width  : 0 ),
        'height' : (height ? height : 0 ) ,
        'maxtabs' : (maxtabs ? maxtabs : 0 ),
        'tabstart' : 0 ,
        'selected': 0,
        'tabs' : new Array () ,
        
        'active': '',

        'add' : function (name, icon, type, viewers){return vDV.addtab(this, name, icon, type, viewers);},
        'compose' : function (reuse){return vDV.compose(this,reuse);},
        'load': function (loadmode) {return vDV.load(this,loadmode);},
        'findtab' : function (tbname) {return vDV.findtab(this.name, tbname) ; },
        'findview': function (tbname, iview ) { return vDV.findview(this.name,tbname,iview);} ,
        'hook': function(dsname,tbname, iv ) {return vDV.hookview(dsname, this.name, tbname, iv ); },
        'icoSize':16,
        'tabSeparation':10, 
        'tabHeight':16, 

        'last': 0
    };
    vDV[name]=dv;
    return dv;
};

vDV.addtab=function( dv, name, icon, type, viewers )
{
    var tb={
        'name':name, 
        'icon' : (icon ? icon : name ),
        'viewers': new Array() ,
        'type': type,
        'columns': 1,
        'width': '100%',
        'height' : '100%',
        'valign': 'top',
        'align': 'left',
        'viewtoggles' : false,
        
        'add' : function (viewers){return vDV.addview(this, viewers);},
        'load': function (loadmode) {return vDV.loadtab(this,loadmode);},
        
        'last':''
    };
    tb.add(viewers);
    dv.tabs[dv.tabs.length]=tb;
    return tb;
};

vDV.addview=function(tb, viewers)
{
    var vie=verarr(viewers);
    
    for ( var iv=0;  iv< vie.length ; ++ iv) { 
        vie[iv].uniqueId=Math.random();
        tb.viewers[tb.viewers.length]=vie[iv];
        if( isok(vie[iv].data) ) {
            vDS.hook( vDS[vie[iv].data],  {'func':'vDV.refreshview', 'obj' : vie[iv], 'param' : tb } );
        }
    }
};

vDV.hookview=function(dsname, dvname, tbname, iv )
{
    
    var dv=vDV[dvname];
    var tb=dv.tabs[vDV.findtab( dvname, tbname )];
    var viewer=tb.viewers[iv];
    
    viewer.data=dsname;
    vDS.hook( vDS[viewer.data],  {'func':'vDV.refreshview', 'obj' : viewer, 'param' : tb } );
};

vDV.unhookview=function( viewer )
{
    var ds=vDS[viewer.data];
    var callbacks=verarr(ds.callbacks); 
    for ( var ic=callbacks.length-1; ic>=0; --ic  ) {   
        if(callbacks[ic].obj && callbacks[ic].obj.uniqueId==viewer.uniqueId ) {
            callbacks=callbacks.splice(ic);
        }
    }

};

vDV.findtab=function( dvname, tbname )
{
    var dv=vDV[dvname];
    var tbs=verarr(dv.tabs); 
    for ( var it=0;  it< tbs.length ; ++ it) { 
        if(tbname==tbs[it].name)return it;
    }
    return -1;
};

vDV.findview=function( dvname, tbname, iview )
{
    var dv=vDV[dvname];
    var itb=vDV.findtab(dvname,tbname);if(itb<0)return -1;
    return dv.tabs[itb].viewers[parseInt(iview)];
};

vDV.findviewByContainer=function( container )
{
    var ss=container.split("_");
    return vDV.findview(ss[1],ss[2],parseInt(ss[3]));
};

vDV.registerViewer=function( viewerTypeName, viewerFunction) 
{
    vDV_registeredViewers[viewerTypeName]=viewerFunction;
};


vDV.load=function( dv , loadmode )
{
    var tbs=verarr(dv.tabs); 
    for ( var it=0;  it< tbs.length ; ++ it) { 
        if(tbs[it].type=='tab')
            vDV.loadtab(tbs[it],loadmode);
        else if(tbs[it].type=='menu')
            vDV.loadtab(tbs[it],loadmode);
    }
};


vDV.loadtab=function( tb, loadmode  )
{
    var vie=verarr(tb.viewers); 
    for ( var iv=0;  iv< vie.length ; ++ iv) { 
        vDV.loadview(vie[iv], loadmode);
    }
    
};

vDV.loadview=function( viewer , loadmode )
{
    var text=0;

    if(!viewer.data) return ;
    if( !vDS[viewer.data] ) {
        alert("DEVELOPER ALERT: data source '"+viewer.data+"' does not exist");
        return ;
    }

    if( loadmode=='download'){
        vDS.load(vDS[viewer.data], loadmode );
        return ;
    }
    if(debug=='flow')alert("VIEWLOAD ANALIZING "+vDS[viewer.data].url +"\n\n"+ vDS[viewer.data].dataurl +"\n\n"+ vDS[viewer.data].state) ; 

    if( (vDS[viewer.data].url && vDS[viewer.data].url!=vDS[viewer.data].dataurl) || (vDS[viewer.data].state!='done' && vDS[viewer.data].state!='err') ) {
        if(debug=='flow')alert("VIEWLOAD attempts to get the data " +  vDS[viewer.data].data ) ;
        vDS.load(vDS[viewer.data], loadmode );
        return ;
    }
    if(debug=='flow')alert("VIEWLOAD had the data from " + viewer.data ) ;

    vDV.refreshview(viewer, viewer.param, text ? text : vDS[viewer.data].data ) ;
};


vDV.refreshview=function( viewer, param, text, page_request)
{

    
    if(debug=='flow')alert(" RENDEInG " + viewer.container );
    
    var o=document.getElementById(viewer.container+"_cell"); 
    if(o) {
        if(viewer.hidden) o.className="sectHid";
        else o.className="sectVis";
    }
        

    var o=document.getElementById(viewer.container); if(!o)return ;

    if(isok(viewer.composer) && vDV_registeredViewers[viewer.composer]) {
        vDV_registeredViewers[viewer.composer](viewer,text,page_request);
        return;
    }
    if( isok( viewer.composer) && viewer.composer=='preview' ) {
    
        var o=document.getElementById(viewer.container);if(!o)return ;
        var t="";
        if(viewer.prefixHTML)t+=viewer.prefixHTML;
        t+="<pre>"+text+"</pre>";
        if(viewer.appendHTML)t+=viewer.appendHTML;
        if(o)o.innerHTML=t;
        return ;
    } 
    else { 
        var o=document.getElementById(viewer.container);
        if(o)o.innerHTML=text;
        return ;
    }
    
};


 

vDV.compose=function(dv,reuse)
{
    var o=gObject(dv.container);if(!o)return;

    if(!reuse){
        o.innerHTML=vDV.composeText(dv);
        return;
    }
    var nm;
    for ( var it=0; it<dv.tabs.length; ++it){ 
        nm="DV_"+dv.name+"_"+dv.tabs[it].name;
        o=gObject(nm);
        if(o)o.className= (it==dv.selected) ? "sectVis" :"sectHid";
        var viewer=verarr(dv.tabs[it].viewers);
        for ( var iv=0; iv<viewer.length; ++iv) {
            viewer[iv].tabvisible=(dv.selected==it) ? true: false;
        }
                           
    }
    
    nm="DV_"+dv.name+"_tablist";
    o=gObject(nm);
    if(o)o.innerHTML=vDV.composeText(dv, 2);
    nm="DV_"+dv.name+"_viewlist";
    o=gObject(nm);
    if(o){
        o.innerHTML=vDV.composeText(dv, 4);
    }
};

vDV.composeText=function(dv, whattocompose)
{
    if(dv.frame=="none"){
        var t1="";
        for (var i=0 ; i<dv.tabs.length; ++i ) {
            var tb=dv.tabs[i];
            var viewer=verarr(tb.viewers);
            var cntviewer=viewer.length;
            
            for ( var iv=0; iv<cntviewer; ++iv) {
                viewer[iv].container="DV_"+dv.name+"_"+tb.name+"_"+iv;
                viewer[iv].tabvisible=(dv.selected==i) ? true : false;

                t1+="<div id='"+viewer[iv].container+"'  >";
                t1+="</div>";
                if(iv<cntviewer+1 && ((iv+1)%tb.columns)==0)t1+="<br/>";
            }
        }
        return t1;
    }
    
    if(!whattocompose)whattocompose=11;

    var sp="cellspacing=0 cellpadding=0 border=0";
    var t1="";
    if(whattocompose&1) {
        t1+="<table "+sp+" width='100%' height='100%'>";
            t1+="<tr><td  valign=top width=100%>";
                for (var i=0 ; i<dv.tabs.length; ++i ) {
                    var tb=dv.tabs[i];
                
                    var viewer=verarr(tb.viewers);
                    var cntviewer=viewer.length;
                    t1+="<div id='DV_"+dv.name+"_"+tb.name+"' class='"+( dv.selected==i ? "sectVis" : "sectHid" ) +"' style='overflow:auto;" + (dv.height ? ("height:"+dv.height+";") : "") + "' >";
                        t1+="<table "+sp+" width='100%' height='100%'>";
                            t1+="<tr>";
                            for ( var iv=0; iv<cntviewer; ++iv) {
                                viewer[iv].container="DV_"+dv.name+"_"+tb.name+"_"+iv;
                                viewer[iv].tabvisible=(dv.selected==i) ? true : false;

                                t1+="<td class='"+(dv.selected==i ? "sectVis" : "sectHid" )+ "' id='"+viewer[iv].container+"_cell' valign='"+(tb.valign)+"' align='"+(tb.align)+"' " + (tb.width ? ("width="+tb.width) : "") +" " + (tb.height ? ("height="+tb.height) : "")  +" >";
                                    var szWd=dv.width;
                                    var szHi=dv.height;
                                    t1+="<div id='"+viewer[iv].container+"' style='position:relative;overflow:auto;margin:0px;padding: 0px;max-height:"+szHi+"px;max-width:"+szWd+"px;' >";
                                    t1+="</div>";
                                t1+="</td>";
                                if(iv<cntviewer+1 && ((iv+1)%tb.columns)==0)t1+="</tr><tr>";
                            }
                            t1+="</tr>";
                        t1+="</table>";
                    t1+="</div>";
                }
            t1+="</td></tr>";
        t1+="</table>";
        if(whattocompose==1) return t1;
    }

    var t2="";
    if(whattocompose&2 && dv.frame!="none"){
        t2+="<table  "+sp+" " + (dv.width ? ("width="+dv.width) : "") +" " + (dv.tabHeight ? ("height="+dv.tabHeight) : "") + " >";
            t2+="<tr>";
            for (var i=0; i<dv.tabs.length ; ++i ) {
                if(i<dv.tabstart)continue;
                if(dv.maxtabs && i-dv.tabstart>=dv.maxtabs)continue;
                if(dv.active && (dv.active.length<=i || dv.active.charAt(i)!='1'))continue;
            
                var tb=dv.tabs[i];
                var cls=( dv.selected==i ? "DV_tab_selected" : "DV_tab" );
            
                t2+="<td "+sp+" id='DV_"+dv.name+"_"+tb.name+"_tab"+i+"' valign=top";
                if(dv.frame!="none")t2+=" class='"+ cls +"' ";
                t+=" height="+ dv.tabHeight+" width=10>";
                    t2+="<span style='white-space: nowrap'>";
                    t2+="<a href='javascript:vDV.clicktab(\""+dv.name+"\","+i+",\""+tb.name+"\")'>";
                    t2+="<img border=0 src='img/white.gif' width="+dv.tabSeparation+" height=1 />&nbsp;";
                    t2+="<img border=0 src='img/"+tb.icon+".gif' width="+dv.icoSize+" height="+dv.icoSize+" />";
                    t2+=tb.name;
                    t2+="<img border=0 src='img/white.gif' width="+dv.tabSeparation+" height=1 />";
                    t2+="</a>";
                    t2+="</span>";
                    
                t2+="</td>";
            }
            t2+="<td ";
            t+=" class='DV_tab' ";
            t+="></td>";
            t2+="</tr>";

        t2+="</table>";
        if(whattocompose==2)return t2;
    }
    
    var t3="";
    if(whattocompose&4 ){
        
        var tb=dv.tabs[dv.selected];
        var viewer=verarr(tb.viewers);
        var cntviewer=viewer.length;
        if(cntviewer>1 && tb.viewtoggles ){
            t3+="<table >";
            t3+="<tr>";
            for ( var iv=0; iv<cntviewer; ++iv) {
                    t3+="<td>";
                        t3+="<img border=0 src='img/white.gif' width="+dv.tabSeparation+" height=1 />&nbsp;";
                        t3+="<a href='javascript:vDV.clickViewToggle(\""+dv.name+"\","+dv.selected+","+iv+")'><img src='img/"+ (viewer[iv].hidden ? "plus" : "xlose" ) +".gif' width=12 height=12 border=0 /></a>";
                    t3+="</td>";
                    t3+="<td>";
                        t3+="<small><a href='javascript:vDV.clickViewToggle(\""+dv.name+"\","+dv.selected+","+iv+")'>";
                            t3+=isok(viewer[iv] && viewer[iv].name ) ? viewer[iv].name : iv;
                        t3+="</small></a><br/>";
                    t3+="</td>";
            }
            t3+="</tr>";
            t3+="</table>";
        }
        var o=gObject("DV_"+dv.name+"_viewlist");
        if(o)o.className=t3.length ? "DV_content" : "sectHid";
        if(whattocompose==4)return t3;
    }

    
    var t="";
    if(whattocompose&8) {
        t+="<table border=0 "+sp+" " + (dv.width ? ("width="+dv.width) : "") +" " + (dv.height ? ("height="+dv.height) : "") + " >";
            t+="<tr>";
                t+="<td colspan=3  id='DV_"+dv.name+"_tabcontainer' valign=top ";
                if(dv.frame!="none")t+=" class='DV_content' ";
                t+=">";
                    t+=t1;
                t+="</td>";
            t+="</tr>";

                t+="<tr  height=0 >";
                    t+="<td colspan=3 id='DV_"+dv.name+"_viewlist' class='sectHid' >";
                        t+=t3;
                    t+="</td>";
                t+="</tr>";
            
            t+="<tr>";
                t+="<td height="+dv.tabHeight;
                if(dv.frame!="none")t+=" class='DV_tab' ";
                t+=" width="+(dv.tabSeparation*2)+" valign=top >";
                    if(dv.maxtabs){
                        t+="<a href='javascript:vDV.clicktabmove(\""+dv.name+"\",-1)'>";
                        t+="<img src='img/left.gif' border=0 height="+dv.tabHeight+" width="+(dv.tabSeparation*2)+" />";
                        t+="</a>";
                     }
                t+="</td>";
                t+="<td height="+dv.tabHeight+" id='DV_"+dv.name+"_tablist'  >";
                    t+=t2;
                t+="</td>";
                t+="<td height="+dv.tabHeight;
                if(dv.frame!="none")t+=" class='DV_tab' ";
                t+=" width="+(dv.tabSeparation*2)+" valign=top >";
                    if(dv.maxtabs){
                        t+="<a href='javascript:vDV.clicktabmove(\""+dv.name+"\",1)'>";
                        t+="<img src='img/right.gif' border=0 height="+dv.tabHeight+" width="+(dv.tabSeparation*2)+" />";
                        t+="</a>";
                    }
                t+="</td>";
            t+="</tr>";
        t+="</table>";
    }
    
    return t;
};


 
vDV.clickViewToggle=function(dvname, itb , ivi) 
{
    var dv=vDV[dvname];
    var tb=dv.tabs[itb];
    
    if(ivi>=0){
        var viewer=tb.viewers[ivi];
        viewer.hidden = ( viewer.hidden==true ) ? false : true ;
        var o= document.getElementById(viewer.container+"_cell"); if(!o)return ;
        o.className= (viewer.hidden) ? "sectHid" : "sectVis";
    }

    var cols=tb.columns ? tb.columns : 1 , rows=0;
    for ( var iv=0; iv<tb.viewers.length; ++iv)if( !tb.viewers[iv].hidden ) rows++;if(!rows)rows=1;
    for ( var iv=0; iv<tb.viewers.length; ++iv) {
        viewer=tb.viewers[iv];
        viewer.widthAvail=parseInt(dv.width/cols);
        viewer.heightAvail=parseInt(dv.height/rows);
        vDV.loadview(viewer);
    }

    dv.compose(true);

};

vDV.clicktabmove=function( dvname , what ) 
{
    var dv=vDV[dvname];
    dv.tabstart+=what;
    if(dv.maxtabs && dv.tabstart>dv.tabs.length-dv.maxtabs)dv.tabstart=dv.tabs.length-dv.maxtabs;
    if(dv.tabstart<0)dv.tabstart=0;
    dv.compose(true);
};

vDV.clicktab=function(dvname , itb)
{
    var dv=vDV[dvname];
    var tb=dv.tabs[itb];
    
    if( tb.type=='tab' ) {
        dv.selected=itb;
        if(dv.maxtabs){
            if(dv.selected<=dv.tabstart)dv.tabstart=dv.selected-1;
            if(dv.selected>=dv.tabstart+dv.maxtabs-1)dv.tabstart=dv.selected+1-dv.maxtabs+1;
            if(dv.tabstart<0)dv.tabstart=0;
            if(dv.tabstart>=dv.tabs.length)dv.tabstart=dv.tabs.length-1;
        }
        dv.compose(true);
    }
    else if( tb.type=='download' ) {
        tb.load("download");
    }
    else if(tb.type=='link'){
        document.location=tb.src;
    }else if(tb.type=='function'){
        if(tb.viewers)
            eval(tb.viewers+"()");
            
    }
    
};








vDS.add( "dsVoid" , "static:



