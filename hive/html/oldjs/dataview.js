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
// datasource and dataview
/*
function isok( obj )
{
    if( !obj || obj.length==0 ) return false;
    return true;
}

function verobj( obj )
{
    if( !obj || !obj.length ) return 0;
    return obj;
}

function verarr( obj )
{
    if( !obj ) return new Array(); // empty array
    if( (obj instanceof Array) ) return obj;
    return  new Array( obj ) ;
   // return obj;|| !obj.length
}

function evalVars(text, starter, ender )
{
    var spl=text.split("<!--") ;
    if(spl.length>1){
        text="";
        for (var i=0; i<spl.length; ++i ) {
            var posend=spl[i].indexOf("//-->");
                
            if(posend==-1){ text+=spl[i];continue; } // this one doesn't have ending parenthesis 
            text+=eval(spl[i].substring(0,posend)) + spl[i].substring(posend+5);  // replace the variable and concat
        }
    }
    return text;
}

*/


// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ datasource constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/
var vDS = new Array ();
var vDS_reuseTblBlobs=0;
var vDS_raw=1;
var debug=0;//'flow';
vDS.add=function( name, url, callbacks,  header )
{
    if(vDS[name])  { // if exists 
        vDS[name].url=url;
        vDS[name].callbacks=callbacks;
        vDS[name].header=header;
        return;
    }

    var ds={
        'name' : name,  // the name by which this data source is specified in tabs
        'url' : url, // the url to load the data: can have http:// qp_data:// ... etc
        'header' : header, // automatically added to data retrieved from source 
        'parser': '', // should be of a prototype parser ( ds, text)    and return text or null
        'data' : '', // the data itself 
        'page_request' : '', // complete page_request response from ajax layer
        'callbacks' : callbacks , // new Array(),  //  array of {obj,func,param}  triples
        'evaluator': false,  //  automatically parsing of variables in data sources is turned off 

        'load': function (loadmode){return vDS.load(this,loadmode);}, 
        'refresh': function (){return vDS.refresh(this);},  // call refresh for all callbacks
        'hook' : function(  cbfun ) { return vDS.hook(this, cbfun); }, // add a new hook to our data 
        'unhook' : function(  cbfun ) { return vDS.unhook(this, cbfun); }, // remove an existing hook to our data 
        
        'progressor' :'QP_bgProgress', // the progress bar container during background data downloads
        'dataurl': '', // the url for which we got the data already: check this in lazy mode
        'state' : ''  // can be '', 'load' , 'done'
    };

    vDS[name]=ds; // this makes sure element is refferencable by its name in array
    return ds;
};

vDS.refresh=function( ds ) 
{
    ds.callbacks=verarr(ds.callbacks); 
    if(debug=='flow')alert( "CALLBACING: "+ds.name +"\n"+"url="+ds.url + "\n"+ds.state+"\n\n\n"+jjj(ds.callbacks));
    for ( var i=0 ; i<ds.callbacks.length; ++i) {
        var func=eval(ds.callbacks[i].func);
        if( isok(func) ) // if a callback function is specieid call it
            func( ds.callbacks[i].obj, ds.callbacks[i].param , ds.data, ds.page_request );
        else if( isok(ds.callbacks[i].obj) && isok(ds.callbacks[i].obj.refresh) ) // call the objects function 
            ds.callbacks[i].obj.refresh( ds.callbacks[i].obj, ds.callbacks[i].param , ds.data, ds.page_request); 
        else{ // if no object is specified the whole callback is a function 
            eval(ds.callbacks[i])( ds.callbacks[i], ds.data, ds.page_request );
        }
    }
};

vDS.hook=function (ds, cbfun )
{
    if(!ds)return ;

    ds.callbacks=verarr(ds.callbacks) ; // turn it into an array if it was just a function
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
    //  remember the data
    if (isok(ds.header)) ds.data = ds.header + "\n" + text;
    else ds.data = text;
    ds.page_request = page_request;
    ds.state = 'done';
    if (debug == 'flow') alert("FiNISHED: " + ds.name + "\n" + "url=" + ds.url + "\n" + ds.state);
    if (!isok(ds.data)) { ds.state = 'err'; return; }

    if (ds.evaluator) { // evaluate variables which are included as comments
        ds.data = evalVars(ds.data, "<!--", "//-->");
        
    }
    if (isok(ds.parser)) { // if parser is specified ... do parse 
        var parsed = eval(ds.parser)(ds, ds.data);
        if (!parsed) return;
        ds.data = parsed;
    }

    // refresh callbacks for this 
    vDS.refresh(ds);
};

vDS.load=function( ds , loadmode )
{

    if(loadmode!='download') {

        if(debug=='flow')alert( "INQUIRY: "+ds.name +"\n"+"url="+ds.url +"\n"+"dataurl="+ds.dataurl+ "\n"+ds.state);

        // be lazy about loading the data
        // check that data exists and it is from the same source
        if( isok(ds.data) && isok(ds.dataurl) && ds.dataurl==ds.url && (ds.state=='done' || ds.state=='err') ){
            //vDS.load_callback(ds,ds.data);
            //vDS.refresh(ds);
            if(debug=='flow')alert( "LAZY: "+ds.name +"\n"+"url="+ds.url + "\n"+ds.state);
            return true;
        }
        if(ds.state=='load' &&  ds.dataurl==ds.url ) // currently loading : do not attempt to get it twice  
            return true;

        if(debug=='flow')alert( "LOAD: "+ds.name +"\n"+"url="+ds.url + "\n"+ds.state);
        ds.state='load';
        //alert(ds.name+" is = " +ds.dataurl+"=="+ds.url);
        ds.dataurl=ds.url; // associate the data with the url
        
    }
    
    // ds.state='load';
    
    // split the source and the url
    var spl=ds.url.split("://"), dataname="";
    var src=spl[0];
    var url=spl[1]; 
    //if(url.indexOf("//")==0) url=url.substring(2);
    
    // now make the replacements
    //if(isok(ds.evaluator))
    url=evalVars(url,"$(",")"); // evaluate variables
    
    // for background retrievals there is a dataname in front of the job
    if(src.indexOf("qpbg_")==0 ){
        spl=url.split("//");
        dataname=spl[0];
        url=spl[1];
    }

    // reusage of the service with an alias 
    var dataReq=0;
    if(src=="qpbg_tblqry"){
        dataReq=req;
        //if(loadmode!='download' && QPride_findDataName( dataname+".tbl.csv" )){ // if this blob already exists 
        if(loadmode!='download' && QPride_findDataName( "Tbl-"+dataname )){ // if this blob already exists 
            src="qp_data";
            url="Tbl-"+dataname;
            //url=dataname+".tbl.csv";
        } else {
            url="-qpSubmit&svc=tblqry&oper=list&tbl="+dataname+"&dataGrpID="+req+"&resID="+req+"&"+url;
            //dataname+=".tbl.csv";
            dataname="Tbl-"+dataname;//.tbl.csv";
            src="qpbg_data";
        }
    }
    

    // now the operation itself 
    if( loadmode=='download' && src.indexOf("qpbg_")!=0 )  { // downlaods of direct links
        if(src=='qp_data')url="?cmd=-qpData&req="+req+"&grp=1&raw=1&dname="+url;
        document.location=url;
        return ;
    }

    //var oprogi=(ds.proginfo) ? gObject(ds.proginfo) : 0;

    if(src =="http" ){
        ajaxDynaRequestPage(url, ds.name , vDS.load_callback ); // +"&raw="+vDS_raw
        return true;
    } else if(src=="innerHTML")  { // if the data is associated with an innerHTML of some other layer
        var o=document.getElementById(url);
        if(o)vDS.load_callback(ds.name ,o.innerHTML);
        return true;
    } else if(src=='static') {
        vDS.load_callback( ds.name ,url);
        return true;
    } else if(src=="qp_data") { // if the data is assoociated with a datablob
        ajaxDynaRequestPage("?cmd=-qpData&req="+req+"&raw=1&grp=1&dname="+url, ds.name ,  vDS.load_callback );
        return true;
    } 

    if(src.indexOf("qpbg_")==0 ) { // if(src=="qpbg_data"){
        //if(oprogi) oprogi.innerHTML="downloading: "+dataname ;
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


// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Dataview constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/

var vDV = new Array ();
var vDV_registeredViewers=new Array();

vDV.add=function( name, width, height , maxtabs )
{
    var dv={
        'name': name, 
        'container' : name,  // the default container is the same as the name 
        'tabs': new Array (),
        'width': (width ? width  : 0 ),
        'height' : (height ? height : 0 ) ,
        'maxtabs' : (maxtabs ? maxtabs : 0 ),
        'tabstart' : 0 , //  current shift in tabs
        'selected': 0, // current selected tab
        'tabs' : new Array () ,  // the list of tabs
        
        'active': '', // the list of available tabs

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
        'viewers': new Array() , // one of the known types of viewers; currently "ggraph', seltable
        'type': type, // tab, link, download, redir  
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
        vie[iv].uniqueId=Math.random(); //
        tb.viewers[tb.viewers.length]=vie[iv];// remember this viewer in our list 
        if( isok(vie[iv].data) ) {
            vDS.hook( vDS[vie[iv].data],  {'func':'vDV.refreshview', 'obj' : vie[iv], 'param' : tb } ); // register ourselves a callback for a data updater
        }
            //vie[iv].load=function (){ return vDV.loadview(this); }
    }
};

vDV.hookview=function(dsname, dvname, tbname, iv )
{
    
    var dv=vDV[dvname];
    var tb=dv.tabs[vDV.findtab( dvname, tbname )];
    var viewer=tb.viewers[iv];
    
    viewer.data=dsname;
    vDS.hook( vDS[viewer.data],  {'func':'vDV.refreshview', 'obj' : viewer, 'param' : tb } ); // register ourselves a callback for a data updater
};

vDV.unhookview=function( viewer )
{
    var ds=vDS[viewer.data];
    var callbacks=verarr(ds.callbacks); 
    for ( var ic=callbacks.length-1; ic>=0; --ic  ) {   
        //alert(" considering " + viewer.uniqueId + " === " + jjj( callbacks[ic].obj ) ) ;
        if(callbacks[ic].obj && callbacks[ic].obj.uniqueId==viewer.uniqueId ) {
            callbacks=callbacks.splice(ic);
            //alert(" found to remove " + viewer.uniqueId + " === " + jjj( viewer ) ) ;
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
    //return 0;
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

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ load and view functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/

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
//    var viewerData=verarr(viewer.data);

    if(!viewer.data) return ; // no associated data source: can do nothing 
    if( !vDS[viewer.data] ) {
        alert("DEVELOPER ALERT: data source '"+viewer.data+"' does not exist");
        return ;
    }

    if( loadmode=='download'){
        vDS.load(vDS[viewer.data], loadmode ); // refresher will be called from data handler
        return ;
    }
    if(debug=='flow')alert("VIEWLOAD ANALIZING "+vDS[viewer.data].url +"\n\n"+ vDS[viewer.data].dataurl +"\n\n"+ vDS[viewer.data].state) ; 

    if( (vDS[viewer.data].url && vDS[viewer.data].url!=vDS[viewer.data].dataurl) || (vDS[viewer.data].state!='done' && vDS[viewer.data].state!='err') ) { // datasource is associated but data is not here 
        if(debug=='flow')alert("VIEWLOAD attempts to get the data " +  vDS[viewer.data].data ) ;
        vDS.load(vDS[viewer.data], loadmode ); // refresher will be called from data handler
        return ;
    }
    if(debug=='flow')alert("VIEWLOAD had the data from " + viewer.data ) ;

    vDV.refreshview(viewer, viewer.param, text ? text : vDS[viewer.data].data ) ;
};


vDV.refreshview=function( viewer, param, text, page_request)
{

    
    if(debug=='flow')alert(" RENDEInG " + viewer.container );
    // call the refresh function for this viewer .. if exists 
    /*if(isok(viewer.refresh)) {
        if( !viewer.refresh(viewer,param,text) )
            return ;
    }
    */
    
    //alert( viewer.container  + "==="+viewer.hidden);
    var o=document.getElementById(viewer.container+"_cell"); 
    if(o) {
        if(viewer.hidden) o.className="sectHid";
        else o.className="sectVis";
    }
        

    var o=document.getElementById(viewer.container); if(!o)return ;

    // now deal with built in viewers 
    if(isok(viewer.composer) && vDV_registeredViewers[viewer.composer]) {
        vDV_registeredViewers[viewer.composer](viewer,text,page_request);
        //alert(viewer.composer);
        return;
    }
    if( isok( viewer.composer) && viewer.composer=='preview' ) { // unformtatted blobs 
    
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


 
// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Dataview composition functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/

vDV.compose=function(dv,reuse)
{
    var o=gObject(dv.container);if(!o)return;

    if(!reuse){
        o.innerHTML=vDV.composeText(dv);
        return;
    }
    var nm;
    for ( var it=0; it<dv.tabs.length; ++it){ 
        nm="DV_"+dv.name+"_"+dv.tabs[it].name; // sub container name
        o=gObject(nm);
        if(o)o.className= (it==dv.selected) ? "sectVis" :"sectHid";
        var viewer=verarr(dv.tabs[it].viewers);
        for ( var iv=0; iv<viewer.length; ++iv) {
            viewer[iv].tabvisible=(dv.selected==it) ? true: false;
        }
                           
    }
    
    nm="DV_"+dv.name+"_tablist"; // the tablist is here 
    o=gObject(nm);
    if(o)o.innerHTML=vDV.composeText(dv, 2);
    nm="DV_"+dv.name+"_viewlist"; // the tablist is here 
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
                                    t1+="<div id='"+viewer[iv].container+"' style='position:relative;overflow:auto;margin:0px;padding: 0px;max-height:"+szHi+"px;max-width:"+szWd+"px;' >";  // height:"+szHi+"px;width:"+szWd+"px;
                                       // t1+=viewer[iv].container;
                                    t1+="</div>";
                                t1+="</td>";
                                if(iv<cntviewer+1 && ((iv+1)%tb.columns)==0)t1+="</tr><tr>"; // insert a new row  if new columns has to be started 
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
    if(whattocompose&2 && dv.frame!="none"){ //  tabs 
        //t2+="<tr><td colspan="+dv.tabs.length +" >"+t3+"</td></tr>";
        //t2+=t3;        
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
    if(whattocompose&4 ){ // view toggles 
        
        var tb=dv.tabs[dv.selected];
        var viewer=verarr(tb.viewers);
        var cntviewer=viewer.length;
        if(cntviewer>1 && tb.viewtoggles ){
            t3+="<table >";
            t3+="<tr>";
            for ( var iv=0; iv<cntviewer; ++iv) {
                //t3+="<tr>";
                    t3+="<td>";
                        t3+="<img border=0 src='img/white.gif' width="+dv.tabSeparation+" height=1 />&nbsp;";
                        t3+="<a href='javascript:vDV.clickViewToggle(\""+dv.name+"\","+dv.selected+","+iv+")'><img src='img/"+ (viewer[iv].hidden ? "plus" : "xlose" ) +".gif' width=12 height=12 border=0 /></a>";
                    t3+="</td>";
                    t3+="<td>";
                        t3+="<small><a href='javascript:vDV.clickViewToggle(\""+dv.name+"\","+dv.selected+","+iv+")'>";
                            t3+=isok(viewer[iv] && viewer[iv].name ) ? viewer[iv].name : iv;
                        t3+="</small></a><br/>";
                    t3+="</td>";
                //t3+="</tr>";
            }
            t3+="</tr>";
            t3+="</table>";
        }
        var o=gObject("DV_"+dv.name+"_viewlist");
        if(o)o.className=t3.length ? "DV_content" : "sectHid";
        if(whattocompose==4)return t3;
    }

    
    var t="";
    if(whattocompose&8) { // the framing of the whole dv
        t+="<table border=0 "+sp+" " + (dv.width ? ("width="+dv.width) : "") +" " + (dv.height ? ("height="+dv.height) : "") + " >";
            t+="<tr>";
                t+="<td colspan=3  id='DV_"+dv.name+"_tabcontainer' valign=top ";
                if(dv.frame!="none")t+=" class='DV_content' ";
                t+=">";
                    t+=t1;
                t+="</td>";
            t+="</tr>";

            //if(t3.length){
                t+="<tr  height=0 >";
                    t+="<td colspan=3 id='DV_"+dv.name+"_viewlist' class='sectHid' >";
                        t+=t3;
                    t+="</td>";
                t+="</tr>";
            //}
            
            t+="<tr>";
                t+="<td height="+dv.tabHeight;
                if(dv.frame!="none")t+=" class='DV_tab' ";
                t+=" width="+(dv.tabSeparation*2)+" valign=top >";
                    //if(dv.tabstart>0)
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
                    //if(dv.maxtabs && dv.tabstart+dv.maxtabs<dv.tabs.length) 
                    if(dv.maxtabs){
                        t+="<a href='javascript:vDV.clicktabmove(\""+dv.name+"\",1)'>";
                        t+="<img src='img/right.gif' border=0 height="+dv.tabHeight+" width="+(dv.tabSeparation*2)+" />";
                        t+="</a>";
                    }
                t+="</td>";
            t+="</tr>";
        t+="</table>";
        //return t;
    }
    //alert(t1);
    
    return t;
};


 
// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ handlers 
// _/
// _/_/_/_/_/_/_/_/_/_/_/
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
        //vDV.refreshview=function( viewer, viewer.param, viewer.datext)
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







// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Empty datasource
// _/
// _/_/_/_/_/_/_/_/_/_/_/

vDS.add( "dsVoid" , "static://void");



