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

    
    
    
var gTbl_CollFormName=new Array();
var gTbl_CollArr=new Array () ;
var gTbl_CollHeaders=new Array () ;
var gTbl_CollDimensions=new Array () ;
var gTbl_CollMinMax=new Array();
var gTbl_CollGabarites=new Array();
var gTbl_CollContainers=new Array();
var gTbl_CollVisibles=new Array();
var gTbl_CollZooms=new Array();
var gTbl_CollFrames=new Array();
var gTbl_CollDensities=new Array();
var gTbl_CollYFrame=new Array();
var gTbl_CollSelect=new Array();
var gTbl_CollColorShift=new Array();
var gTbl_CollColor=new Array();
var gTbl_CollDoScroll=new Array();

var gTbl_CollLabelSteps=new Array ();


var gTbl_LabelCallback;


function TblChart_arrIdx(arr, cols,  icol, frmstart, frmend, maxrowcnt)
{
    
    var irstart=0, irend=parseInt( (arr.length/cols)-1);
    while( arr[cols*irstart+0]<frmstart && irstart<irend)++irstart;
    while ( arr[cols*irend+0]>frmend && irend>irstart) --irend;
        
    var jumpRow=parseInt((irend-irstart)/maxrowcnt); 
    if(jumpRow<1)jumpRow=1;
    jumpRow=parseInt(jumpRow);
    
    var gArrIdx = new Array ();
    gArrIdx.push(irstart);
    for (var ir=irstart;ir<irend;ir+=jumpRow){
        
        if(!arr) {
            gArrIdx.push(ir);
            continue;
        }
        
        var imin=ir, imax=ir;
        for(var ik=ir; ik<ir+jumpRow; ++ik){
            if(parseFloat(arr[cols*imin+icol])>parseFloat( arr[cols*ik+icol]))
                imin=ik;
            if(parseFloat(arr[cols*imax+icol])<parseFloat(arr[cols*ik+icol]))
                imax=ik;
        }
        
        if(imin<imax){
            gArrIdx.push(imax);
        }
        else if(imin>imax){
            gArrIdx.push(imin);
        }
        else {
            gArrIdx.push(imax);
        }
    }


    return gArrIdx;
}

function TblChart_colMinMax(arr, cols,  icol, arridx)
{
    var iminx=arridx[0];
    var imaxx=arridx[0];
    for (var ix=0;ix<arridx.length ; ++ix){
        var cval=parseFloat( arr[cols*arridx[ix]+icol]);
        
        if(parseFloat(arr[cols*iminx+icol])>cval) 
            iminx=arridx[ix];
        if(parseFloat(arr[cols*imaxx+icol])<cval){ 
            imaxx=arridx[ix];
        }
    }
    return new Array( parseFloat(arr[cols*iminx+icol]) , parseFloat(arr[cols*imaxx+icol] ))    ;
}

function TblChart_colVarArr(arr, cols,  icol, arridx)
{
    var arrll = new Array () ;
    
    for (var ix=0;ix<arridx.length ; ++ix){
        arrll[ix]=parseFloat( arr[cols*arridx[ix]+icol]);
        
    }
    return arrll;
}


var gZoomMin=1/10000;
var gTblChart_callbackZoom;

function TblChart_mouseWheel (nm,area)
{
    
    var delta=event.wheelDelta;
        var oper="0";
        if(area=="x") {
            if( delta < 0 ) oper="+0.1";
            else oper="-0.1";
        }else {
            if( delta < 0 ) oper="*1.1";
            else oper="/1.1";
        }
        if(gTbl_CollDoScroll[nm])
            TblChart_zoomShift(nm, oper, area);

        if(event && event.preventDefault)
            event.preventDefault();

        return true;

}


function TblChart_zoomShift(nm, oper, what )
{
    var zoomfactor=new Array(0,1);

    
    if(oper.substring(0,1)=='%'){
        if(gTbl_CollSelect[nm][0]!=-1 && gTbl_CollSelect[nm][1]!=-1){
            
            gTbl_CollFrames[nm][0]=parseInt(gTbl_CollArr[nm][gTbl_CollSelect[nm][0]*gTbl_CollDimensions[nm][0]+0]);
            gTbl_CollFrames[nm][1]=parseInt(gTbl_CollArr[nm][gTbl_CollSelect[nm][1]*gTbl_CollDimensions[nm][0]+0]);
            if(gTbl_CollFrames[nm][0] > gTbl_CollFrames[nm][1] ){
                t=gTbl_CollFrames[nm][0];
                gTbl_CollFrames[nm][0]=gTbl_CollFrames[nm][1];
                gTbl_CollFrames[nm][1]=t;
            }
        }else {
            gTbl_CollFrames[nm][0]=gTbl_CollFrames[nm][2] ;
            gTbl_CollFrames[nm][1]=gTbl_CollFrames[nm][3] ;
        }
    }
            
    else if(what && what=='y'){
        
        zoomfactor[0] = 0; 
        zoomfactor[1] = gTbl_CollYFrame[nm][1];
        zoomfacror=docZoomShift(zoomfactor,  oper ,gZoomMin);
        gTbl_CollYFrame[nm][0]=0;
        gTbl_CollYFrame[nm][1]=zoomfactor[1];
        
    } else {
        var frm=(gTbl_CollFrames[nm][3]-gTbl_CollFrames[nm][2]);
        zoomfactor = new Array ( (gTbl_CollFrames[nm][0]-gTbl_CollFrames[nm][2])/frm , (gTbl_CollFrames[nm][1]-gTbl_CollFrames[nm][2])/frm ) ;
        zoomfactor=docZoomShift(zoomfactor,  oper ,gZoomMin);
        gTbl_CollFrames[nm][0]=gTbl_CollFrames[nm][2]+parseInt( frm*zoomfactor[0] );
        gTbl_CollFrames[nm][1]=gTbl_CollFrames[nm][2]+parseInt( frm*zoomfactor[1] ) ;
    }
    if(gTblChart_callbackZoom && gTblChart_callbackZoom!=""){
        if( gTblChart_callbackZoom(nm)) return;
    }
    
    TblChart_showGraph(nm);
    
}

var gTblChart_ChangeRangeBusy=0;
function TblChar_changeRange (nm, xst, xed )
{
    if(gTblChart_ChangeRangeBusy)return ;
    gTblChart_ChangeRangeBusy=1;

    if(!xst) xst=parseInt(document.forms[gTbl_CollFormName[nm]].elements[nm+"_xLeft"].value);
    if(!xed) xed=parseInt(document.forms[gTbl_CollFormName[nm]].elements[nm+"_xRight"].value);
    
    if(xed==xst)xed=xst+1;
    if(xst>xed){var t=xst; xst=xed; xed=t;}
    if(xst<gTbl_CollFrames[nm][2])xst=gTbl_CollFrames[nm][2]; 
    if(xed>=gTbl_CollFrames[nm][3]) xed=gTbl_CollFrames[nm][3];
    gTbl_CollFrames[nm][0]=xst;
    gTbl_CollFrames[nm][1]=xed;
    var ret=0;
    if(gTblChart_callbackZoom && gTblChart_callbackZoom!="")
        ret=gTblChart_callbackZoom(nm);
    if(!ret)
        TblChart_showGraph(nm,1);
    
}

function TblChart_onSelect(nm, ir)
{

    if(gTbl_CollSelect[nm][0]==-1) {
        gTbl_CollSelect[nm][0]=ir;
        gTbl_CollSelect[nm][1]=-1;
    }
    else if(gTbl_CollSelect[nm][1]==-1) {
        gTbl_CollSelect[nm][1]=ir;
    }
    else {
        gTbl_CollSelect[nm][0]=-1;
        gTbl_CollSelect[nm][1]=-1;
    }
    TblChart_showGraph(nm);
    
}



function TblChart_addGraph(formName, nm, container, header, arr, cols, rows, winframeS, winframeE, density, graphWidth, graphHeight)
{
    gTbl_CollFormName[nm]=formName;
    gTbl_CollArr[nm]=arr;
    gTbl_CollHeaders[nm]=header;
    gTbl_CollDimensions[nm]=new Array (cols,rows) ;
    gTbl_CollGabarites[nm]=new Array(graphWidth, graphHeight);
    gTbl_CollContainers[nm]=container;
    gTbl_CollZooms[nm]=new Array(0.,1.);
    if(!gTbl_CollFrames[nm])gTbl_CollFrames[nm]=new Array(winframeS,winframeE,winframeS,winframeE);
    gTbl_CollDensities[nm]=density;
    gTbl_CollLabelSteps[nm]=new Array (5,5);
    gTbl_CollYFrame[nm]=new Array (0,1);
    gTbl_CollSelect[nm]=new Array(-1,-1);
    if(!gTbl_CollDoScroll[nm])gTbl_CollDoScroll[nm]=false;
    gTbl_CollColorShift[nm]=2;
    gTbl_CollMinMax[nm]=new Array(-1000000,-1000000,-1000000,-1000000);
}

function TblChart_showGraph(nm,noResetCtrl)
{
    var icol,ir;
  
    var arrList=new Array () ; 
    var viscols=0, visrows=0;
    var arridx = new Array();


    for ( icol=0;  icol<gTbl_CollDimensions[nm][0]; ++icol ) { if( gTbl_CollVisibles[nm+"_"+gTbl_CollHeaders[nm][icol]]!=1) continue;
                
        arridx=TblChart_arrIdx(gTbl_CollArr[nm],gTbl_CollDimensions[nm][0],icol,gTbl_CollFrames[nm][0],gTbl_CollFrames[nm][1],gTbl_CollDensities[nm]);
        if(arrList.length<=icol || arrList[icol].length==0) 
            arrList[icol]=TblChart_colVarArr(gTbl_CollArr[nm],gTbl_CollDimensions[nm][0],icol,arridx);
        ++viscols;
    }
    visrows=arridx.length;
    
    var rMin=10000000; rMax=-10000000;
    for( ir=0; ir<visrows; ++ ir) {
        var sumY=0;
        for ( icol=0;  icol<gTbl_CollDimensions[nm][0]; ++icol ) {  if(gTbl_CollVisibles[nm+"_"+gTbl_CollHeaders[nm][icol]]!=1) continue;
            sumY+=arrList[icol][ir];
        }
        if(rMin>sumY)rMin=parseFloat(sumY);
        if(rMax<sumY)rMax=parseFloat(sumY);
    }
    if(rMin>0)rMin=0;
    rMax=parseFloat(rMax*gTbl_CollYFrame[nm][1]);
    if(gTbl_CollMinMax[nm][3]!=-1000000)rMax=gTbl_CollMinMax[nm][3];

    var totHeight=gTbl_CollGabarites[nm][1];
    var totWidth=gTbl_CollGabarites[nm][0];
        
    if(!viscols || !visrows) {
        var v;
        v=gObject(gTbl_CollContainers[nm]);if(v)v.innerHTML="<table width="+totWidth+" height="+totHeight+" class='tb_chart'><tr><td align=center valign=middle><big>?</§big></td></tr></table>";
        v=gObject(gTbl_CollContainers[nm]+"_x");if(v)v.innerHTML="";
        v=gObject(gTbl_CollContainers[nm]+"_y");if(v)v.innerHTML="";
        v=gObject(gTbl_CollContainers[nm]+"_ctrl");if(v)v.innerHTML="";
        return ;
    }

    var width=Math.round(totWidth/visrows);if(width<3) width=3;
    var boxStart="<span style='font-size : 0pt; margin:0px; padding:0px; display:-moz-inline-block; display:-moz-inline-box; display:inline-block; ";
    var boxStop="</span><br />";
    var isSel=(gTbl_CollSelect[nm][1]!=-1 && gTbl_CollSelect[nm][0]!=-1) ? true : false ;
          
    
    var vTbl="<table class='tb_chart' border=0 width=100% height="+totHeight+" onmousewheel='TblChart_mouseWheel(\""+nm+"\",\"Graph\")'><tr>";
    for ( ir=0;  ir<visrows; ++ir ) { 
        
        vTbl+="<td valign=bottom onclick='TblChart_onSelect(\""+nm+"\","+arridx[ir]+")' width="+width+" >";
        var totlen=0;  
        var colCont="";
        for ( icol=0;  icol<=gTbl_CollDimensions[nm][0]; ++icol ) { if( icol!=gTbl_CollDimensions[nm][0] && gTbl_CollVisibles[nm+"_"+gTbl_CollHeaders[nm][icol]]!=1) continue;
            var llen, clr ;
            if( icol==gTbl_CollDimensions[nm][0] ) {
                llen=( totHeight-totlen);
                if( isSel && (arridx[ir]-gTbl_CollSelect[nm][1])*(arridx[ir]-gTbl_CollSelect[nm][0]) <= 0 ) 
                    clr="background-color:#ffff00; ";
                else break;
                               
            }
            else { 
                var val=parseFloat(arrList[icol][ir]);
                llen=parseInt(totHeight*(val-rMin)/(rMax-rMin)) ;
                if(totlen+llen>totHeight) llen=totHeight-totlen; if(llen<0)llen=0;
                clr="background-color:";
                if(gTbl_CollColor[nm+"_clr"+gTbl_CollHeaders[nm][icol]])clr+=gTbl_CollColor[nm+"_clr"+gTbl_CollHeaders[nm][icol]];
                else clr+=gClrTable[gTbl_CollColorShift[nm]+icol];
                clr+="; ";
                popupTxt="TblChart_showInfoPopup("+icol+" , \""+gTbl_CollHeaders[nm][icol]+"\" ,"+(arridx[ir])+" , "+arrList[icol][ir]+" )'";
                            
            }
            
            if(llen) {
                colCont=boxStart+clr+"width:"+width+"; height:"+llen+"px'  onmouseover='"+popupTxt+"' >"+boxStop + colCont;
                totlen+=llen;
            }
        }
        vTbl+=colCont;
        vTbl+="</td>";
        
      }

    vTbl+="</tr></table>";
    
    
    arridx = TblChart_arrIdx(gTbl_CollArr[nm], gTbl_CollDimensions[nm][0],0,gTbl_CollFrames[nm][0],gTbl_CollFrames[nm][1],gTbl_CollLabelSteps[nm][0]);
    arrX=TblChart_colVarArr(gTbl_CollArr[nm],gTbl_CollDimensions[nm][0],0,arridx);
    width=parseInt(100/(arridx.length-1)); if(width<1)width=1;
    
      var vX="<table class='tb_chart' width=100% onmousewheel='TblChart_mouseWheel(\""+nm+"\",\"x\")' ><tr>";
    for ( ir=0;  ir<arridx.length-1; ++ir ) { 
        var vl=gTbl_LabelCallback ? gTbl_LabelCallback(nm,"x", arridx[ir], parseFloat(arrX[ir]) ) : "";
        if(vl=="") vl=arrX[ir];
        vX+="<td width="+width+"% align=left valign=top><small><small>"+vl+"</small></small></td>";
    }
    vX+="</tr>";
    vX+="</table>";

        
    var prec;
    for( prec=1; prec<.5*(rMax-rMin) ; prec*=10);
    for( ; prec>.5*(rMax-rMin) ; prec/=10);
    
    var ystp=(rMax-rMin)/(gTbl_CollLabelSteps[nm][0]);
    var height=parseInt(100/gTbl_CollLabelSteps[nm][1]); if(height<1)height=1;
    
    var vY="", vYPrc="";
    if(ystp!=0) { 
        vY+="<table height="+gTbl_CollGabarites[nm][1]+" class='tb_chart' onmousewheel='TblChart_mouseWheel(\""+nm+"\",\"y\")' ><tr>";
        vYPrc=vY;
        for( var iy=0; iy<=gTbl_CollLabelSteps[nm][0]; ++iy) {
            vY+="<tr><td valign=top height="+height+"% ><small><small>";
            vYPrc+="<tr><td valign=top height="+height+"% ><small><small>";
            
            var yval=gTbl_LabelCallback ? gTbl_LabelCallback(nm,"y", iy, yval ) : "";
            if(yval==""){
                yval=rMax-iy*ystp;
                if(prec>=1)yval=Math.round(yval);
                else yval=Math.round(yval/prec)/(1/prec);
            }
            var yvalPrc=100*yval/rMax;
            if(prec>=1)yvalPrc=Math.round(yvalPrc);
            else yvalPrc=Math.round(yvalPrc/prec)/(1/prec);
                        
            
            vY+=yval;
            vYPrc+=yvalPrc+"%";
            vY+="</small></small></td></tr>";
            vYPrc+="</small></small></td></tr>";
        }
        vY+="</tr></table>";
        vYPrc+="</tr></table>";
    }

    var vCtrl="";
    if(!noResetCtrl){
        vCtrl+="<table width=100% border=0><tr>";
        vCtrl+="<td align=left><input class='inputEditable' type=text name='"+nm+"_xLeft' value="+gTbl_CollFrames[nm][0]+" onchange='TblChar_changeRange(\""+nm+"\")'  size=4 /></td>";
        vCtrl+="<td align=center>";
        vCtrl+="<small><small>Zoom:&nbsp;";
                vCtrl+="<a href=\"javascript:TblChart_zoomShift('"+nm+"','/1.3')\">in +</a>&nbsp;|&nbsp;";
                vCtrl+="<a href=\"javascript:TblChart_zoomShift('"+nm+"','*1.3')\">out &divide;</a>&nbsp;|&nbsp;";
                vCtrl+="<a href=\"javascript:TblChart_zoomShift('"+nm+"','0')\">reset &times;</a>&nbsp;|&nbsp;";
                vCtrl+="<a href=\"javascript:TblChart_zoomShift('"+nm+"','%0')\">scale &harr;</a>&nbsp;|&nbsp;";
                vCtrl +="Shift:&nbsp;";
                vCtrl+="<a href=\"javascript:TblChart_zoomShift('"+nm+"','-0.5')\">left &laquo;</a>&nbsp;|&nbsp;";
                vCtrl+="<a href=\"javascript:TblChart_zoomShift('"+nm+"','+0.5')\">right &raquo;</a>";
            vCtrl+="</small></small>";
        vCtrl+="</td>";
        vCtrl+="<td align=right><input class='inputEditable' type=text name='"+nm+"_xRight' value="+gTbl_CollFrames[nm][1]+" onchange='TblChar_changeRange(\""+nm+"\")' size=4 /></td>";
        vCtrl+="</tr></table>";
    }
      
    
    if(gTbl_CollContainers[nm]) {  
        var v;
        v=gObject(gTbl_CollContainers[nm]);if(v)v.innerHTML=vTbl;
        v=gObject(gTbl_CollContainers[nm]+"_x");if(v)v.innerHTML=vX;
           v=gObject(gTbl_CollContainers[nm]+"_y");if(v)v.innerHTML=vY;
           v=gObject(gTbl_CollContainers[nm]+"_yPrc");if(v)v.innerHTML=vYPrc;
           if(!noResetCtrl){v=gObject(gTbl_CollContainers[nm]+"_ctrl");if(v)v.innerHTML=vCtrl;}
    }
    

    

                    
    gTblChart_ChangeRangeBusy=0;
    return vTbl;    

}    

var gTbl_PopupIdNum=0;
function TblChart_showInfoPopup(icol,colname, ir, val)
{
    if(!gObject("TblChart_popup"))return ;
    
    var tt="<table class='tb_chart' ><tr><td>";
    tt+="<span style='background-color:##ffffcf;'>"+colname+"["+(ir+1)+"]"+":"+val+"</span>";
    tt+="</td></tr></table>";
    gObjectSet("TblChart_popup",gMoX, gMoY,tt,"show","-","-");
     gTbl_PopupIdNum++;
     setTimeout("TblChart_hideInfoPopup("+gTbl_PopupIdNum+")",3000);
}
function TblChart_hideInfoPopup(idnum)
{
    if(gTbl_PopupIdNum==idnum)
        gObjectSet('TblChart_popup','-','-','-','hide','-','-');
}






var gListList=new Array () ;
var gListCnt=new Array();
var gListStart=new Array();
var gListCntshow=new Array();
var gListSize=new Array();
var gListContainer=new Array();
var gListSelIndex = new Array();

var gList_ElemNameCallback;
var gList_ElemSelCallback;

function VList_addList(container, nm, arr, cnt ,cntshow, sizeshow )
{
    gListList[nm]=arr;
    gListCnt[nm]=cnt;
    gListStart[nm]=0;
    gListCntshow[nm]=cntshow;
    gListSize[nm]=sizeshow;
    if(container)gListContainer[nm]=container;
    
    var tco="";
    tco="<select onchange='VList_onSel()' onscroll='VList_onScroll()' name='"+nm+"' size="+gListSize[nm]+" multiple='multiple'>";
    tco+="</select>";
    if(gListContainer[nm])  { 
        gObject(gListContainer[nm]).innerHTML=tco;
    }
    return tco;
    
}

function VList_onScroll(e, realelem)
{
    var elem=realelem;
    if(!elem) { 
        if(!e)e=event;
        var elem=event.srcElement;
    }
    VList_update(elem, 1 );
}

function VList_onSel(e )
{
    if(!gList_ElemSelCallback)return ;
    if(!e)e=event;
    var elem=event.srcElement;
    var nm=elem.name;
    var ipos=elem.value;
    gList_ElemSelCallback(nm,gListList[nm][ipos],ipos );
}


function VList_update(elem , isscrll )
{
    if(!elem)return ;
    var nm=elem.name;
    var sel=elem.selectedIndex;
    var realIndex=gListStart[nm]+sel;
    
    var onelemsize=elem.scrollHeight/gListCntshow[nm];
    var curpos=Math.round(elem.scrollTop/onelemsize);
    var realCurPos=gListStart[nm]+curpos;
    var scrl=0;
    
    if(curpos>=gListCntshow[nm]-gListSize[nm]) { 
        realCurPos=realCurPos-gListSize[nm];
        scrl=gListSize[nm];
    }
    else if(curpos==0) {
        realCurPos=realCurPos-gListCntshow[nm]+gListSize[nm];
        if(isscrll)
            scrl=gListCntshow[nm]-2*gListSize[nm];
            
            
    }else 
        return ;

    if(realCurPos<0)realCurPos=0;
    if(realCurPos>gListCnt[nm]-gListCntshow[nm]-1)
        realCurPos=gListCnt[nm]-gListCntshow[nm]-1;
    if(realCurPos<0)realCurPos=0;
        
    if(realCurPos!=gListStart[nm] || (!isscrll) ){

        gListStart[nm]=realCurPos;
        gListSelIndex[nm]=realIndex+1;
        elem.options.length=0;
        
        for( var ic=0, ishown=0; ishown<gListCntshow[nm]; ++ic) {
            var ipos=ic+gListStart[nm];
            if(ipos>=gListCnt[nm])break;
                
            var vl=gList_ElemNameCallback ? gList_ElemNameCallback(nm,gListList[nm][ipos], ipos) : gListList[nm][ipos];
            if(vl=="")continue;
            var opt=new Option(vl,ipos);
            elem.add(opt);
            elem.value=ipos;
            ++ishown;
        }
        elem.scrollTop=scrl*onelemsize;
    }    
}



var gVDir_listCallbackClick;
var gVDir_listCallbackText;
var gVDir_formName;
var gVDir_event;

function vDir_listPath( container, dirlist, concat , isdir)
{
    if(concat && gVDir_event ) concat=gVDir_event.shiftKey;

    if(!isdir && !concat)
        document.forms[gVDir_formName].elements[gSeq_dirListDest].value="";
    
    var dirarr=dirlist.split(",");
    if(!isdir && gVDir_listCallbackClick){
        for(var id=0; id<dirarr.length; ++id) { 
            var dir=dirarr[id];
            gVDir_listCallbackClick(document.forms[gVDir_formName].elements[container].value+dir,container,vDir_listDirContent,isdir );
        }
        return ;
    }
    
    if(isdir)
        document.forms[gVDir_formName].elements[container].value=dirlist;
    else { 
        for(var id=0; id<dirarr.length; ++id)  
            document.forms[gVDir_formName].elements[container].value+=dirarr[id];
    }

    dir=document.forms[gVDir_formName].elements[container].value;
    if(gVDir_listCallbackClick)gVDir_listCallbackClick(dir,container,vDir_listDirContent,isdir);
    else {
        if(isdir) linkCmd("dirList&raw=1&path="+dir, container, vDir_listDirContent);
    }
}

function  vDir_listDirContent(container, txt)
{
    var curdir=document.forms[gVDir_formName].elements[container].value;
    var updir="";
    if(curdir){
    
        var pos=curdir.lastIndexOf("/");
        if(pos==curdir.length-1)pos=curdir.substring(0,pos-1).lastIndexOf("/");
        if(pos!=-1)updir=curdir.substring(0,pos+1);
    }        
    var lst1=txt.split("|");
    var tl="";
    if(curdir.length)tl+="<a href='javascript:;' onclick='javascript:gVDir_event=event;vDir_listPath(\""+container+"\",\""+updir+"\",0,1)'><img border=0 src='img/back.gif' width=16 height=16 />&nbsp;<small>back</small></a><br>";
    for(var i=0 ;i< lst1.length; ++i ){
        var lst2=lst1[i].split(",");
        if(lst2.length>1){
            tl+="<table border=0 >";
        }
        for(var j=0 ;j< lst2.length; ++j ){
        
            if(lst2[j].length<1)continue;
            var isdir= (lst2[j].indexOf("/")!=-1) ? 1 : 0 ;
            var bod="-";
            if(gVDir_listCallbackText) {
                bod=gVDir_listCallbackText(isdir, lst2[j], txt, document.forms[gVDir_formName].elements[container].value );
            }
        
            if(bod=="-") { 
                if(isdir)bod="<img border=0  src='img/dir.gif' height=12>&nbsp;"+lst2[j];
                else bod=lst2[j];
            }
        
            if(!bod.length)continue;
            if(lst2.length>1){
                tl+="<tr>";
                if(j==0)tl+="<td border=0 width=10 rowspan="+lst2.length+"><a href='javascript:;' onclick='javascript:gVDir_event=event;vDir_listPath(\""+container+"\",\""+lst1[i]+"\",1, 0)'><img src='img/pair.gif' border=0 width=24 height="+(12*lst2.length)+" /></a></td>";
                tl+="<td>";
            }
            if(isdir)tl+="<a href='javascript:;' onclick='javascript:gVDir_event=event;vDir_listPath(\""+container+"\",\""+curdir+lst2[j]+"\",1, "+isdir+")'>"+bod+"</a><br>";
            else tl+="<a href='javascript:;' onclick='javascript:gVDir_event=event;vDir_listPath(\""+container+"\",\""+lst2[j]+"\",1, 0)'>"+bod+"</a><br>";
            if(lst2.length>1)tl+="</td></tr>";
        }
        if(lst2.length>1)tl+="<table>";
    }
    gObject(container+"_list").innerHTML=tl;
}




var gVTree_callbackName;

function  vTree_nexusSplit(txt, scl)
{
    var tr="";
    var i=0; var start=0; var irow=0;

    var st="font-size : 0pt; margin:0px; padding:0px; border=0px;display:-moz-inline-block; display:-moz-inline-box; display:inline-block; ";
    var boxStart="<span style='"+st;
    var boxStop="</span><br />";
    

    var nam=""; var dist=""; var ori=""; var llen=0, cellheight=20, linewidth=1;
    var clr="#000000";
    var issubtree=false;
    for ( i=0 ; i < txt.length; ++i) {
        cha=txt.charAt(i);
        
        if(cha=='(') {
            
            var lev=1;
            for( var j=i+1; j<txt.length && lev>0; ++j){
                if(txt.charAt(j)=='(')++lev;
                if(txt.charAt(j)==')')--lev;
            }
            nam=vTree_nexusSplit(txt.substring(i+1,j-1),scl);
            i=j-1;
            issubtree=true;
                        
            continue;
        }
        
        if( cha==':' ) { 
            ori=txt.substring(start,i);
            if(nam=="")nam=ori;
            start=i+1;
            continue;
        }
        
        if( cha==',' ) {
            dist=txt.substring(start,i);
            if(nam==""){
                ori=nam;
                nam=dist;dist=0;
                issubtree=false;
            }
            llen=parseInt(dist*scl);
            
            start=i+1;
                                
            tr+="<table class='vtree' height="+cellheight+">";
                tr+="<tr width="+cellheight+">";
                    
                    tr+="<td valign=bottom >";
                    tr+="<table class='vtree' height="+ (irow ? 100 : 50)+"% ><tr><td style='background-color: "+clr+"; ' ></td></tr></table>";
                    tr+="</td>";
                
                    tr+="<td align=right style='vertical-align:middle' >";
                    tr+=boxStart+"background-color: "+clr+"; width:"+llen+"; height:"+linewidth+"px' >"+boxStop ;
                    tr+="</td>";
                    
                    tr+="<td>";
                    var tnam=(gVTree_callbackName && !issubtree) ? gVTree_callbackName(nam) : "-";
                    if(tnam=="-")tnam=nam;
                    tr+=tnam;
                    tr+="</td>";
                tr+="</tr>";
            tr+="</table>";
            issubtree=false;
            ++irow;

            nam="";ori="";dist="";
                        
        }
                
        
    }
    dist=txt.substring(start,i);
    if(ori==""){ori=dist;dist=0;}
    llen=parseInt(dist*scl);
                
            
    tr+="<table class='vtree' height="+cellheight+">";
                
    tr+="<tr >";
    
    tr+="<td valign=top>";
    if(dist)tr+="<table class='vtree' height=50% ><tr><td style='background-color: "+clr+"; ' ></td></tr></table>";
    tr+="</td>";


    tr+="<td align=right style='vertical-align:middle'>";
    tr+=boxStart+"background-color: "+clr+"; width:"+llen+"; height:"+linewidth+"px' >"+boxStop ;
    tr+="</td>";

    tr+="<td>";
    var tnam=(gVTree_callbackName && !issubtree) ? gVTree_callbackName(nam) : "-";
    if(tnam=="-")tnam=nam;
    tr+=tnam;
    tr+="</td></tr>";
    tr+="</table>";

    return tr;
}

function  vTree_nexusTree(container, txt, scl)
{
    var tt=vTree_nexusSplit(txt,scl);
    var v=gObject(container);
    if(v)v.innerHTML=tt;
}

