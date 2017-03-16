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
// _/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Sequence viewer
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/

vDV.generateSeqView=function( viewer , text) 
{
    var o=document.getElementById(viewer.container);if(!o)return;
    var t=vDV.generateSeqViewText( viewer, text) ;
    o.innerHTML=t;
}

vDV.generateSeqViewText=function( viewer, text) 
{
    var wrap = ( isok(viewer.wrap) ) ? parseInt(viewer.wrap) : 60;
    var txtarr=text.split("\n");if(txtarr.length<=1)return ;
    
    var mode='seq';  // a sequence is expected : 1 , mode is 0 
    var curlin='';
    var tseq="";
    for (var i=0 ; i<=txtarr.length ; ++i ) {
        var lin=(i<txtarr.length) ? txtarr[i] : ">";if(!lin.length)continue;
        // see if the mode has changed 
        var prvmode=mode;
        if(lin.charAt(0)=='>')mode='id';
        else mode='seq'; 
        
       
        if(mode==prvmode){ // mode has not changed -- glue and continue
            curlin+=lin;
            continue;
        }
        if(!curlin.length){ // mode has changed but there was nothing to draw
            curlin=lin;
            continue;
        }

    
        if(prvmode=='id') { 
            tseq+="<tr><td>";
            tseq+="<b>&gt;</b></td><td><b>"+curlin.substring(1)+"</b>";
            tseq+="</td></tr>";
            curlin=lin;
            continue;
        }

        
        // if this is a sequence we are outputting
        curlin=curlin.replace(" ","").replace("\r","").replace("\t","");
        var wr = (wrap==0) ? curlin.length : wrap;
        for (var ipos=0; ipos<curlin.length; ipos+=wr){
            if(ipos+wr>curlin.length)wr=curlin.length-ipos;
            var ss=curlin.substr(ipos, wr ); // get this wrapped piece

            //tseq+="&nbsp;&nbsp;&nbsp&nbsp;"+ss+"<br/>";
            tseq+="<tr><td></td><td class='HIVE_seq'>"+ss+"</td></tr>";
            if(!wr)break;
        }
        curlin=lin;
    }


    var tt="";
    if(viewer.prefixHTML&& viewer.prefixHTML.length) {
        tt+=viewer.prefixHTML;
    }
    tt+="<table border=1 width='100%' width='100%' class='DV_table' >";
        //tt+="<tr>";
            //tt+="<td>";
                tt+=tseq;
            //tt+="</td>";
        //tt+="</tr>";
    tt+="</table>";
    if(viewer.appendHTML && viewer.appendHTML.length) {
        tt+=viewer.appendHTML;
    }

    return tt;
}


// _/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Registration
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/
vDV.registerViewer( "sequenceview", vDV.generateSeqView) ;

