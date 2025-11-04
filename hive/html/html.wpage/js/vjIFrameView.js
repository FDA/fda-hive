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
function vjIFrameView ( viewer )
{
    if(!this.destinationDIV)this.destinationDIV="iFrameDiv";
    if(!this.delayTimeout)this.delayTimeout=500;
    if(!this.attempts)this.attempt=10;

    vjDataViewViewer.call(this,viewer);



    this.composerFunction=function( viewer , content)
    {

        var dv=undefined;
        if (this.tab && this.tab.parent) {
            dv=this.tab.parent;
        }
        if (dv == undefined) {            
            if (viewer.width) dv.width = viewer.width;
            if (viewer.height) dv.height = viewer.height;
        }
        if (dv == undefined && this.maxAreaHeight) {
            dv = {height: this.maxAreaHeight, width: this.maxAreaHeight}
            
        }
        if (dv == undefined) dv = {width: 400, height: 400};
        this.frmnm=this.container+"-iframe";
        var t=content;
        const iframeContent = "`" + t + "`";
        if (viewer.srcLink) {
            this.div.innerHTML=`<iframe width="${dv.width * 0.9}" height="${dv.height*0.9}" src="${viewer.srcLink}"></iframe>`;
        } else {
            this.div.innerHTML=`<iframe width="${dv.width *0.9}" height="${dv.height*0.9}" srcdoc="${iframeContent.replace(/"/g, '&quot;')}"></iframe>`;
        }


    };



    this.delayedCompose=function()
    {
        var layer=window.frames[this.frmnm].document.getElementById(this.destinationDIV);
        if(!layer && this.attempts>=0){
            --this.attempts;
            setTimeout("vjObjEvent('delayedCompose', '"+this.objCls+"');",this.delayTimeout);
            return ;
        }
        if(!layer)return;

        layer.style.width=parseInt(this.tab.parent.width*90/100);
        layer.style.height=parseInt(this.tab.parent.height*90/100);

        if(this.submitCallback || this.submitAutohide)
            setTimeout("vjObjEvent('delayedSubmitResponse', '"+this.objCls+"');",this.delayTimeout);
    };

    this.delayedSubmitResponse=function()
    {
        var layer=window.frames[this.frmnm].document.getElementById(this.destinationDIV);
        if(layer){
            setTimeout("vjObjEvent('delayedSubmitResponse', '"+this.objCls+"');",this.delayTimeout);
            return ;
        }


        if(this.submitCallback)
            funcLink(this.submitCallback);
        if(this.submitAutohide)
            this.tab.parent.hide(this.tab.num,true);
    };

}
