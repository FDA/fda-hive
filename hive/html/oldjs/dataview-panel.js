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

 


vDV.generatePanelView=function( viewer , text) 
{
    var o=document.getElementById(viewer.container);if(!o)return;
    if(!viewer.lastSearch)viewer.lastSearch="";
    if(!viewer.pagehistory || viewer.pagehistory.length==0) viewer.pagehistory=new Array ("0");
   
    if(!viewer.internalOnLoad){
        viewer.lastSearch="";
        viewer.pagehistory=new Array( "0" );
        viewer.selectedCounter=20;
    }
    else {     
        var el=document.forms[viewer.formObject].elements[viewer.container+"_pageCounter"];
        if(el)viewer.selectedCounter=el.value;
        else viewer.selectedCounter=20;
        viewer.internalOnLoad=false;
    }
    
    var arr=text.split("\n");
    viewer.cntElements=0;
    if(arr.length) { 
        viewer.begin=parseInt(arr[1].split(",")[viewer.paginate-1]);
        var ilast;
        for (ilast=arr.length-1; ilast>=0; --ilast ) { 
            var el=arr[ilast].split(",");
            if( el.length ){viewer.end=parseInt( el[viewer.paginate-1]);}
            if(viewer.paginate && viewer.end>0)break;
            if(!viewer.paginate && el.length)break;
        }
        viewer.cntElements=ilast+1;
        if(!viewer.paginate)--viewer.cntElements; 
    }
    var t=vDV.generatePanelViewText( viewer, '' ) ;
    o.innerHTML=t;
    if(viewer.searchbox) { 
        var srch="vDV.updatePanelView('"+viewer.container+"');";
         if(gIdleSearchList.indexOf(srch)==-1)
            gIdleSearchList+=srch;
        
    }
};

vDV.generatePanelViewText=function( viewer, arr ) 
{
    var tt="",a,ac;
    if(viewer.prefixHTML&& viewer.prefixHTML.length) {
        tt+=viewer.prefixHTML;
    }
    
    var a="<a href='javascript:vDV.updatePanelView(\""+viewer.container+"\")'>";
    tt+="<table border=0 "+(viewer.tblClass ? ("class='"+viewer.tblClass+"'") : "" )+" ><tr>";

    if(viewer.searchbox && viewer.searchbox!=''){
        var searchoptions= "selectBoxOptions='Canada;Denmark;Finland;Germany;Mexico;Norway;Sweden;United Kingdom;United States'";
        tt+="<td valign=center font-weight:bold ><small>"+a+"Search:</a></small></td><td valign=center aligh=left>"+a+"<img src='img/search.gif' border=0 valign=center aligh=left width=24 height=24 /></a></td>";
        tt+="<td valign=center aligh=left><input type='text' "+searchoptions+" size="+viewer.searchbox+" valign=center name='"+viewer.container+"_search' value='"+viewer.lastSearch+"'/></td>";
    }

    
    if(viewer.paginate ){        
        
        a="<a href='javascript:vDV.updatePanelView(\""+viewer.container+"\",-1)'>";
        ac="</a>";
        if(viewer.pagehistory.length<2){a=ac="";}
        tt+="<td valign=center  aligh=right ><small>"+a+"previous<img border=0 valign=center width=16 src='img/previous.gif'/>";
        tt+=ac+"</small></td>";
        tt+="<td>" + viewer.pagehistory.length+ac+"</small></td>";
        
        a="<a href='javascript:vDV.updatePanelView(\""+viewer.container+"\",1)'>";
        ac="</a>";
        if(viewer.cntElements<viewer.selectedCounter){a=ac="";}
        tt+="<td valign=center aligh=right ><small>"+a+"<img border=0 valign=center width=16 src='img/next.gif'/> next";
        tt+=ac+"</small></td>";
        if(viewer.pagecount)tt+="<td valign=center aligh=right><small>"+vDV.generatePageCountPanelViewer(viewer.container,viewer.pagecount, viewer.selectedCounter)+"</a></small></td>";               
    }
    
    if(viewer.customInputs) {
        for( var i=0; i<viewer.customInputs.length ; ++i){
            
            var realVal=viewer.customInputs[i].value;
            if(realVal && realVal.indexOf("eval:")==0)
                realVal=eval(realVal.substring(5));
            
            if(viewer.customInputs[i].type=="html"){
                tt+=realVal;
                continue;
            }
            
            if(viewer.customInputs[i].type=="statictext"){
                tt+="<td>"+realVal+"</td>";
                continue;
            }
            tt+="<td><input type='"+viewer.customInputs[i].type+"' ";
            if(realVal) tt+=" value='"+realVal+"' ";
            else tt+=" value='' ";
            if(viewer.customInputs[i].name)        tt+=" name='"+viewer.customInputs[i].name+"' ";
                                        else     tt+= " name='"+viewer.container+"-"+viewer.customInputs[i].type+"-"+i+"' ";
            if(viewer.customInputs[i].size)         tt+=" size="+viewer.customInputs[i].size +" ";
            if(viewer.customInputs[i].callback)    tt+=" onclick='javascript:"+viewer.customInputs[i].callback+"(\""+viewer.customInputs[i].param+"\",\""+viewer.container+"\")' ";
            tt+="/></td>"
        }
    }
    
    tt+="</tr></table>";
    
    
    
    if(viewer.appendHTML && viewer.appendHTML.length) {
        tt+=viewer.appendHTML;
    }
    
    return tt;
};

vDV.generatePageCountPanelViewer=function(container, pagecounts, selectedCounter){
    var t;
    if(!pagecounts) {t="&nbsp;";}
    else 
     {
      var narr=pagecounts.split(",");

        t="<select name='" + container + "_pageCounter'"
            + " onchange=" + '"' + "vDV.updatePanelView('" + container +"')" + '"'
            + ">";
       for (var i=0; i<narr.length; ++i){        
           t+="<option value=" + '"' + narr[i] + '"';  
               if(narr[i]==selectedCounter) t+=" selected ";
               t+= ">";
               t+= narr[i];
            t+= "</option>";
       }
       t+="</select>";
    }
    return t;
}



vDV.updatePanelView=function(container, page )
{
    var viewer=vDV.findviewByContainer(container);
    
    var el=document.forms[viewer.formObject].elements[viewer.container+"_search"];
    var curSearchString= el ? el.value : "";
    
    el=document.forms[viewer.formObject].elements[viewer.container+"_pageCounter"];
    var currentPageCounter=el ? parseInt(el.value) : viewer.selectedCounter;
    
    if(curSearchString!=viewer.lastSearch || currentPageCounter!=viewer.selectedCounter || page )
        viewer.needsUpdate=true;
    
    if(!viewer.needsUpdate)return;
    viewer.needsUpdate=false;
    
    var newurl=vDS[viewer.data].url;
     
    if(!page) {
        viewer.pagehistory=new Array( "0" );
        newurl=urlExchangeParameter(newurl, "start");
    }
    else if(page==1) {
        newurl=urlExchangeParameter(newurl, "start",viewer.end);
        viewer.pagehistory[viewer.pagehistory.length]=viewer.end;
    }else if(page==-1) {
        newurl=urlExchangeParameter(newurl, "start",viewer.pagehistory[viewer.pagehistory.length-2]);
        viewer.pagehistory.splice(viewer.pagehistory.length-1,1);
    }
    
    if(viewer.selectedCounter!=currentPageCounter)viewer.pagehistory=new Array( "0" );
    viewer.selectedCounter=currentPageCounter;
    newurl=urlExchangeParameter(newurl, "cnt",viewer.selectedCounter);

    if(viewer.lastSearch!=curSearchString)viewer.pagehistory=new Array( "0" );
    viewer.lastSearch=curSearchString;
    newurl=urlExchangeParameter(newurl, "search",viewer.lastSearch);
        
    if(newurl!=vDS[viewer.data].url) {
        viewer.internalOnLoad=true;
        vDS[viewer.data].url=newurl;
        vDS[viewer.data].load();
    }
    return ;
};


vDV.registerViewer( "panelview", vDV.generatePanelView) ;


