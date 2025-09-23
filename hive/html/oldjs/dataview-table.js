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

    


vDV.generateTableView=function( viewer , text) 
{
    if(!viewer.tblClass)viewer.tblClass='DV_table';
    var o=document.getElementById(viewer.container);if(!o)return;
    viewer.tblArr=vDV.generateTableViewParse( viewer, text);
    vDV.redrawTableView (viewer);
};
vDV.redrawTableView=function( viewer) 
{   
    document.getElementById(viewer.container).innerHTML=vDV.generateTableViewText( viewer, viewer.tblArr);
};
vDV.generateTableViewControls=function( viewer , text) 
{
    var o=document.getElementById(viewer.container);if(!o)return;
    var t=vDV.generateTableViewControlText( viewer, text ) ;
    o.innerHTML=t;
};

vDV.generateTableViewParse=function( viewer, text) 
{
    var txarr=text.split("\n");if(!txarr.length)return ;
        
    var arr=new Array();
    for(var it=0;it<txarr.length;++it){
        var carr=txarr[it].split(",");if(carr.length<2)continue;
        arr[it]=new Array();
        for(var ic=0;ic<carr.length;++ic){
            arr[it][ic]=carr[ic];
        }
    }

    return arr;
};

vDV.generateTableViewText=function( viewer, arr ) 
{
    if(arr.length==0)return;
    var prfx=viewer.container+"_check_";
    var Pcolors=viewer.colors;
    var checks=viewer.checkmarks;
    var selected=viewer.selected;
    var colhide=viewer.colhide;
    var colscolor=viewer.colscolor;
    var startAutoNumber=0;
    
    var tt="";
    if(viewer.prefixHTML&& viewer.prefixHTML.length) {
        tt+=viewer.prefixHTML;
    }
    
    tt+="<table "+(viewer.tblClass ? ("class='"+viewer.tblClass+"'") : "" ) +" ><thead>";
        tt+="<tr>";
            if(viewer.type=="checktable"){
                tt+="<th>";
                    tt+="<input name='"+prfx+"all' type=checkbox ";
                    tt+="onClick='vDV.checkmarkSelTable(this,-1,"+(arr.length-1)+",\""+prfx+"\");";
                    if(viewer.checkcallback)tt+=viewer.checkcallback+"(-1)";
                    tt+="'>";
                tt+="</th>";
            }
            
            if(isok(viewer.autonumber) && viewer.autonumber!='none'){
                startAutoNumber=parseInt(viewer.autonumber);
                tt+="<th>#</th>";
            }
            
            var addCols=(viewer.appendCols && viewer.appendCols.length) ?  viewer.appendCols.length : 0;
            for( var ic=0; ic<arr[0].length+addCols; ++ic) {
                var hideThisCol=(colhide && colhide.length>=ic) ? colhide.charAt(ic) : ""; 
                if(hideThisCol=='-')continue;

                tt+="<th>";
                    if(ic<arr[0].length)tt+=arr[0][ic];
                    else tt+=viewer.appendCols[ic-arr[0].length].header;
                tt+="</th>";
            }
        tt+="</tr></thead>";
        
        tt+="<tbody style='overflow:scroll;'>";
        var skiprows=viewer.skiprows ? viewer.skiprows : 0 ;
        for ( var ir=1+skiprows ; ir<arr.length ; ++ir ) {
            if(!arr[ir]) continue;
            if( arr[ir].length && arr[ir][0].length==0  ) continue;
            var iR=ir-1;

            var tag="td"; 
            if(isok(viewer.newSectionWord) && arr[ir][0].indexOf(viewer.newSectionWord)==0){
                tag="th";
            }
                

            tt+="<tr>";
                
                var tcheck="";
                if (viewer.type=="checktable" ) {
                    tcheck+="<"+tag+" ";
                    if(viewer.colors && viewer.colors.length) tcheck+="bgcolor="+Pcolors[iR];
                    tcheck+=" >";
                        var charCheck=(checks && checks.length>=iR) ? checks.charAt(iR) : ""; 
                        var selCheck=(selected && selected.length>=iR) ? selected.charAt(iR) : ""; 
                        
                        if(charCheck!="-") {
                            tcheck+="<input name='"+(prfx+(iR))+"' type=checkbox ";
                            tcheck+=(selCheck=="1" ? "checked" : "" );
                            tcheck+=" onClick='vDV.checkmarkSelTable(this,"+(iR)+","+(arr.length-1)+",\""+prfx+"\");";
                            if(viewer.checkcallback)tcheck+=viewer.checkcallback+"("+iR+",-1)";
                            tcheck+="' ";
                            if(viewer.checkdisabled) tcheck+=" disabled ";
                            if(charCheck=="1")tcheck+=" checked ";
                            tcheck+=" >";
                        }
                    tcheck+="</"+tag+" >";
                }
                tt+=tcheck;                
                
                if( isok(viewer.autonumber) && viewer.autonumber!='none'){
                    tt+="<"+tag+">"+(startAutoNumber+iR-skiprows)+"</"+tag+">";
                }
                
                var irealcol=0;
                var addCols=(viewer.appendCols && viewer.appendCols.length) ?  viewer.appendCols.length : 0;
                for( var ic=0; ic<arr[ir].length+addCols; ++ic) {
                    var hideThisCol=(colhide && colhide.length>=ic) ? colhide.charAt(ic) : ""; 
                    if(hideThisCol=='-')continue;

                    var dolink="", docall="";
                    if( viewer.links && viewer.links.length>=ic && viewer.links[irealcol] && viewer.links[irealcol].length ){ 
                        if(viewer.links[irealcol].indexOf("javascript:")==0)docall=viewer.links[irealcol];
                        else dolink=viewer.links[irealcol];
                    }
                    
                    if( viewer.links && viewer.links.length>=ic && viewer.links[irealcol] && viewer.links[irealcol].length ){ 
                        if(viewer.links[irealcol].indexOf("javascript:")==0)docall=viewer.links[irealcol];
                        else dolink=viewer.links[irealcol];
                    }
                    
                    var clr=( isok(viewer.colors) && colscolor && colscolor.length>ic && colscolor.charAt(ic)=="+" ) ? "bgcolor="+Pcolors[iR] : "";
                    var alg=( isok(viewer.colAlign) && viewer.colAlign.length>ic ) ? "align="+viewer.colAlign[ic] : "";
                    tt+="<"+tag+" "+clr+" "+alg+" >";
                        if(docall!="")tt+="<a href='"+docall+"(\""+viewer.container+"\","+iR+","+ic+",\""+arr[ir][ic]+"\")'>";
                        else if(dolink!=""){ tt+="<a href='"+dolink.replace("$value",arr[ir][ic]).replace("$row",iR).replace("$col",ic)+"'>";} 

                        var realText="";
                        if(ic<arr[ir].length) realText=arr[ir][ic];
                        else realText=viewer.appendCols[ic-arr[ir].length].cell;

                        if( realText.indexOf("eval:")==0 ) {
                            realText=eval( realText.substring(5) );
                        }

                        if(viewer.colTypes && viewer.colTypes.length>=ic) {
                            
                            if(viewer.colTypes[ic]=="percent") {
                                var prc=parseInt(realText);
                                realText="<table width='100%'><tr><"+tag+" bgcolor='blue' width='"+prc+"%' >"+(prc>=50 ? prc : "")+"</"+tag+"><"+tag+" bgcoor='white' width='"+(100-prc)+"%'>"+(prc<50 ? prc : "")+"</"+tag+"></tr></table>";
                            }
                            else if(viewer.colTypes[ic]=="largenumber") {
                                realText+= '';
                                var v = realText.split('.');
                                var v1 = v[0];
                                var v2 = v.length > 1 ? '.' + v[1] : '';
                                var rgx = /(\d+)(\d{3})/;
                                while (rgx.test(v1)) {
                                    v1 = v1.replace(rgx, '$1' + ',' + '$2');
                                }
                                realText=v1+v2;
                            }
                        }
                        tt+=realText;

                        if(dolink.legnth!=0 || docall.length!="")tt+="</a>";
                    tt+="</"+tag+">";
                    ++irealcol;
                }
                
            tt+="</tr>";
        }
        tt+="</tbody>";
    tt+="</table>";
    
    if(viewer.appendHTML && viewer.appendHTML.length) {
        tt+=viewer.appendHTML;
    }

    return tt;
};


vDV.enumerateItemsTableView=function( viewer , operation, ischecked ) 
{
    for ( var i=0 ; i<viewer.tblArr.length; ++i) {
        var row=viewer.tblArr[i];
        if(ischecked && parseInt(viewer.selected.charAt(i-1))!=1)continue;
        if(operation.indexOf("callback:")==0)eval(operaton.substring(0,9)+"("+viewer+","+row+","+operation+")")
        else eval(operation);
    }
};

vDV.checkmarkSelTable=function(thisobject,cursel, maxel, prfx )
{
    var elems=thisobject.form.elements;

    var cntchk=0;
    var ischecked=elems[prfx+"all"].checked ;
    for ( var is=0; is<maxel; ++is ) {
        if( !elems[prfx+is] ) 
            continue;
        if(cursel==-1)
            elems[prfx+is].checked=ischecked;
        else if(elems[prfx+is].checked)
                ++cntchk;
    }
    if(cursel!=-1 && elems[prfx+"all"]) 
        elems[prfx+"all"].checked=(cntchk==maxel-1);

    var checks="";
    for ( var is=0; is<maxel; ++is )  { 
        if( !elems[prfx+is])
            checks+="-";
        else 
            checks+=elems[prfx+is].checked ? "1" : "0";
    }
    var spl=thisobject.name.split("_");
    var vi=vDV.findview(spl[1],spl[2],spl[3]);
    vi.selected=checks;
};


vDV.registerViewer( "tableview", vDV.generateTableView) ;
