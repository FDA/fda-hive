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
// _/ Help viewer
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/

vDV.generateHelpView=function( viewer , text) 
{
    var o=document.getElementById(viewer.container);if(!o)return;
    var t=vDV.generateHelpViewText( viewer, text) ;
    o.innerHTML=t;
}

vDV.generateHelpViewText=function( viewer, text) 
{
    var pos=text.indexOf("\n");
    var title="";
    
    if(pos!=-1){
        title=text.substring(0,pos);
        text=text.substring(pos);
    }else title="Documentation";
    //alert(pos + "="+title+"\n--"+text);

    var t="";
    
    t+="<table width=100% border=1 class='HIVE_table' >"; 
        t+="<tr>";
            t+="<td valign=middle width=70 >";
                t+="<img border=0 width=48 src='img/help.gif' />";
            t+="</td>";
            t+="<td valign=middle>";
                t+="<b>"+title+"</b>";
            t+="</td>";
        t+="</tr>";

        t+="<tr>";
            t+="<td valign=middle colspan=2>";
                t+=text;
            t+="</td>";
        t+="</tr>";

    t+="</table>";
    
    return t;
}


// _/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Registration
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/

vDV.registerViewer( "helpview", vDV.generateHelpView) ;

