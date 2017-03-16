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
function vjImageView ( viewer )
{

    vjDataViewViewer.call(this,viewer); // inherit default behaviours of the DataViewer

//    if(this.iconHeight===undefined)this.iconHeight=100;

//    if(this.iconWidth===undefined) this.iconWidth=100;

    // _/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ HTML viewer constructors
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/

    this.composerFunction=function( viewer , content)
    {

        if(content.length<4)return ;
        this.txt=content;
        this.refresh();
    };


    this.refresh = function ()
    {
        this.div.innerHTML=this.composeText(this.txt);

    };



    this.composeText=function(content)
    {

        var t="";

        var widthTxt = "";
        var heightTxt = "";
        if(this.iconHeight!==undefined) {
            heightTxt = " width='"+this.iconHeight+"' ";
        }
        if(this.iconWidth!==undefined) {
            widthTxt = " width='"+this.iconWidth+"' ";
        }
        if (this.prefixHTML != undefined && this.prefixHTML.length) t+=this.prefixHTML;
        t+="<table>";
        t+="<tr><td><a href='"+content+"' >";
        t+="<img src='"+content+"' "+ heightTxt + widthTxt +" border=0 />";
        t+="</a></td></tr>";
        t+="</table>";
        if (this.suffixHTML != undefined && this.suffixHTML.length) t+=this.suffixHTML;
        return t;
    };
}

//# sourceURL = getBaseUrl() + "/js/vjImageView.js"