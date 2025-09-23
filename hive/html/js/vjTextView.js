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

function vjTextView ( viewer )
{
    this.argWidth = viewer.width;
    this.argHeight = viewer.height;
    vjDataViewViewer.call(this,viewer);
    if (viewer.font)
        this.font = viewer.font;
    else
        this.font = "";
    
    if (viewer.readOnly==false) {
        this.readOnly = viewer.readOnly;
    }
    else this.readOnly = true;


    this.composerFunction=function( viewer , content)
    {
        this.txt=content;
        this.refresh();
    };


    this.refresh = function ()
    {
        this.div.innerHTML=this.composeText(this.txt);
    };



    this.composeText=function(content)
    {
        if (this.isJSON == true) {
            var contentObj = null;
            try {
                contentObj = JSON.parse(content);
            } catch (e) {
                contentObj = null;
                console.log("Unable to parse content as javascript object; defaulting to normal text view");
            }
            if (contentObj != null) {
                content = JSON.stringify(contentObj, null, 2);
            }
        }
        var t="";
        t+="<textarea cols=80 rows=20 "+ ((this.readOnly) ? "readonly=readonly" : "" )+" class='"+this.font+"' style='" + (this.argWidth ? ("width:" + this.argWidth + ";") : "") +"" + (this.argHeight ? (";height:" + this.argHeight + ";") : "") + "'>";
        t+=content;
        t+="</textarea>";
        return t;
    };
}


