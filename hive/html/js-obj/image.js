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

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjImageView.js");

vjHO.register('image').Constructor=function ()
{

    // two public functions which must be supported
    this.fullview=function(node,dv)
    {
        this.preview(node,dv);
    };

    this.preview = function(node,dv)
    {
        var url="?cmd=objFile&propname=thumb&prefix=0&ids="+node.id;
        var dsname= "dsImage-"+node.id;
        vjDS.add("infrastructure: loading image", dsname, "static://"+url );
        this.viewer=new vjImageView({
            data: dsname,
//            iconHeight:300,
//            iconWidth:1000,
            ok: true
            });
        var tab=dv.obj.addTab("image","graph",[this.viewer]);
        dv.obj.select("image");
        dv.obj.render();

        if(this.onPreviewRenderCallback) {
            funcLink(this.onPreviewRenderCallback, this);
        }
        else if(this.onFullviewRenderCallback) {
            funcLink(this.onFullviewRenderCallback, this);
        }


        var divtab=0;
        if(tab){
            divtab=gObject(tab.viewers[0].container+"_cell");
        }

        if(divtab){
            divtab.style.display="inline-block";
        }

        dv.obj.load(true);
        if(this.onPreviewLoadCallback) {
            funcLink(this.onPreviewLoadCallback, this);
        }
        else if(this.onFullviewLoadCallback) {
            funcLink(this.onFullviewLoadCallback, this);
        }

    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};




