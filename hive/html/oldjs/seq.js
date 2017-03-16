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

/******************************************************
*
* Sequence data manipulation routines
*
******************************************************/
    
var gSeq_Parsing="";
var gSeq_dirListDest="";
var gSeq_dirInfo="";
var gSeq_dirSingleSel=0;
var gSeq_fastaInfo="";
var gSeq_fileListPrepend="";

function vSeq_compiledVioseq(container, txt)
{
    alert(txt+" reads were read from sequence file.");
    gSeq_Parsing="";
    vDir_listPath(container,"","vDir_listDirContent",1);
    
}


function vSeq_fastaInfo(seqfile, prfx) 
{
    
    if (seqfile.indexOf("--")==0)seqfile=cookieGet("vSeq_selected_"+seqfile.substring(2));
    
    var ccmd=protectFields('-verbose 0 -wrap 60 -cnt 100 -print len=1 -fSeq '+prfx+seqfile+' 0 128 ');
    var t="";
    t+="<table class='QP_tbl' style='position:relative; left:-100px; background-color:#FFFFFF;' width='100%'>";
        t+="<tr><th style='border:1px dotted #4282B1;' width='100%'><img border=0 height=16 width=48  src='img/dna.gif'>"+seqfile+"<img border=0 height=16 width=48  src='img/dna.gif'><a style='float:right;' href='#' onclick='javascript:vis(\"floatDiv\",\"sectHid\")'>&times;</a></th></tr>";
        t+="<tr><td style='border:1px dotted #4282B1;' width='100%'><div style='overflow: auto; height: 200px; width: 100%; padding: 0px;' id='preFastaInfo' >fasta</div></td></tr>"
    t+="</table>";
    
    gObjectSet("floatDiv",gMoX,gMoY,t,"show","-","-");
    
    gObject("floatDiv").className="sectVis";
     
    linkCmd("viotools&raw=1&exec="+ccmd,"preFastaInfo","-");
    setTimeout("vis(\"floatDiv\",\"sectHid\")",10000);
}

/*
function vSeq_vioinfoDownloaded (container, text)
{
    var o=document.forms[gVDir_formName].elements[container];
    if( o ){o.value=text; return ;}
    o=gObject(container);
    if(o)o.innerHTML=text;
}

function vSeq_vioinfoUpload (pathcontainer,infocontainer)
{
    if(!gSeq_dirInfo || !gSeq_dirInfo.length)return;
    var o=document.forms[gVDir_formName].elements[infocontainer];
    if( !o || !o.value || o.value.length==0)return ;
    url="?cmd=infoSet&file="+document.forms[gVDir_formName].elements[pathcontainer].value+"vioinfo.html&info="+protectFields(o.value);
    //alert(url);
    ajaxDynaRequestAction(url,false);
}
*/
function vSeq_dirListItemClicked(dirtxt,container, callback, isdir)
{
    var inf=dirtxt.split(":");
    var dir=inf[0];

    if(!isdir){
        
        if(dir.indexOf(".vioseq")!=-1) {
            //if(gSeq_dirSingleSel || !(gVDir_concatMode) )document.forms[gVDir_formName].elements[gSeq_dirListDest].value="";
            
            var ls=document.forms[gVDir_formName].elements[gSeq_dirListDest].value.split("; ");
            var isin=0, allbut="";
            for ( var li=0 ; li<ls.length; ++li ){ 
                if(ls[li]==dir )isin=1; // this field was in the list
                //else {
                    if(allbut.length>0)allbut+="; ";
                    allbut+=ls[li];
                //}
            }
            //if(isin) document.forms[gVDir_formName].elements[gSeq_dirListDest].value=allbut;
            //else {
            if(!isin){
                if(allbut.length>0)allbut+="; ";
                allbut+=dir;
            }
            document.forms[gVDir_formName].elements[gSeq_dirListDest].value=allbut;
            //}
            
            cookieSet("vSeq_selected_"+gSeq_dirListDest,allbut);
            return dir;
        }
                
        if(gSeq_Parsing.length) {
            alert("the file "+gSeq_Parsing+" is being parsed now\nPlease wait for it to finish");
            return; 
        }
        var rval=confirm("The file "+dir+" has not been parsed yet\nCompilation may take sime time\nDo you want to compile it now?");
        if(!rval)return ;
        gSeq_Parsing=dir;
        linkCmd("viotools&raw=1&exec="+protectFields("-verbose 0 -fParse %QUERY%"+dir),container,vSeq_compiledVioseq);
    }
    if(isdir){
        linkCmd("dirList&raw=1&path="+dir, container, callback);
        //if(gSeq_dirInfo && gSeq_dirInfo.length)linkCmd("infoGet&raw=1&file="+dir+"vioinfo.html", gSeq_dirInfo, vSeq_vioinfoDownloaded);
    }
    return dir;
}

function vSeq_size(idim) 
{
    var SZ="";
    if(idim>=10000){idim=idim/1000; SZ="K";}
    if(idim>=10000){idim/=1000; SZ="M";}
    if(idim>=10000){idim/=1000; SZ="G";}
    idim=parseInt(idim);
    if((""+idim).length>=4)idim=parseInt(idim/1000)+","+(idim+"").substring(1);
    idim+=SZ;
    return idim;
}

function vSeq_dirListItemText(isdir, itmtxt, txt, curdir)
{

    var inf=itmtxt.split(":");
    var itm=inf[0];
    var idim=vSeq_size((inf.length>1) ? parseInt(inf[1]) : 0);
    var sdim=(inf.length>1) ? ";&nbsp;<img border=0 height=12 width=24 src='img/dna.gif'>" : "";
        
    if(isdir)return "-";
    if(itm.indexOf(".vioseq")!=-1){
        //var ccmd=protectFields('-verbose 0 -cnt 100 -print len=1 -fSeq %QUERY%'+curdir+itm);
        //sdim="&nbsp;<a href=\"javascript:linkSelf('viotools&raw=1&exec="+ccmd+"','new');\" >"+sdim+"</a>";
        //var idim=parseInt(sdim);
        
        t="<img border=0 src='img/ok.gif' height=12 >&nbsp;"+itm + "&nbsp;&nbsp<small>["+idim+"]</small>";
        //if(!gSeq_dirSingleSel)t+="&nbsp;<img border=0 src='img/plus.gif' height=12 ></a>";
        if(gSeq_fastaInfo && gSeq_fastaInfo.length!=0)t+="&nbsp;<a href=\"javascript:"+gSeq_fastaInfo+"('"+curdir+itm+"','%QUERY%');\" >"+sdim+"</a>";
        return t;
        
    }
    
    var extpos=itm.lastIndexOf(".");
    var fnd=(extpos!=-1) ? itm.substring(0,extpos+1) : "";
    if(extpos!=-1){
        var isfound= txt.indexOf(fnd+"vioseq")==-1 ? 0 : 1 ;
        if(!isfound) return "<img border=0 src='img/process.gif' height=12 >&nbsp;"+itm;
    }
    
    return "";//<img border=0 src='ok.gif' height=12 >&nbsp;"+fnd+"vioseq";
}




function vSeq_onChangeQrySub(e)
{
    if(!e)e=event;
        
    var container=e.srcElement.name;
    cookieSet("vSeq_selected_"+container,e.srcElement.value);
}
function vSeq_loadedAvailableFiles(container, txt)
{

    if(gSeq_fileListPrepend.length)
        txt=gSeq_fileListPrepend+txt;

    var vrr=txt.split("\n");
    var defval=document.forms[gVDir_formName].elements[container].value;

    var arrt=vrr[0].split("|");
        
    var selt="<table><tr><td>";
    selt+="<select class='inputEditable' name='"+container+"' onchange='vSeq_onChangeQrySub()' >";
    for ( var im=0; im<arrt.length; ++im) {
        var inf=arrt[im].split(":");
        var flnm=inf[0];
        var dim= (inf.length>1) ? "  // "+vSeq_size(inf[1])+"" : "";
        selt+="<option value='"+flnm+"' "+((defval==flnm) ? "selected" : "") +">"+flnm+dim+"</option>";
    }
    selt+="</select>";
    selt+="</td><td>";
    if(flnm.indexOf("vioseq")!=-1)selt+="<a href=\"javascript:gSeq_fastaInfo('--subject','%SUBJECT%');\" ><img border=0 height=12 width=24 src='img/dna.gif'></a>";
    selt+="</td></tr></table>";
    gObject(container+"_layer").innerHTML=selt;
    
}


