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

    vjDataViewViewer.call(this,viewer); // inherit default behaviours of the DataViewer


    // _/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ IFrame viewer constructors
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/

    this.composerFunction=function( viewer , content)
    {
        //if(this.composedAlready)return;

//        var dv=this.tab.parent;
        this.frmnm=this.container+"-iframe";
        //"+content+"
        //var t="<iframe name='"+this.frmnm+"' height='"+dv.height+"' width='"+dv.width+"' marginheight='0' marginwidth='0' padding=0px  frameborder='0' scrolling='no' src='"+content+"' ></iframe>";
        var t=content;//+"<br/><iframe name='"+this.frmnm+"'marginheight='0' marginwidth='0' padding=0px  frameborder='0' scrolling='no' src='tmpl/iframeViewer.html' ></iframe>";
        this.div.innerHTML=t;
        //this.composedAlready=true;

        //setTimeout("vjObjEvent('delayedCompose', '"+this.container+"');",this.delayTimeout);

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

        //height='"+dv.height+"' width='"+dv.width+"'
        layer.style.width=parseInt(this.tab.parent.width*90/100);
        layer.style.height=parseInt(this.tab.parent.height*90/100);
        //alerJ("o",layer.style);

        if(this.submitCallback || this.submitAutohide)
            setTimeout("vjObjEvent('delayedSubmitResponse', '"+this.objCls+"');",this.delayTimeout);
        //layer.innerHTML=this.getData(0).data;
    };

    this.delayedSubmitResponse=function()
    {
        var layer=window.frames[this.frmnm].document.getElementById(this.destinationDIV);
        if(layer){
            setTimeout("vjObjEvent('delayedSubmitResponse', '"+this.objCls+"');",this.delayTimeout);
            return ;
        }

       // alert(this.submitAutohide);

        if(this.submitCallback)
            funcLink(this.submitCallback);
        if(this.submitAutohide)
            this.tab.parent.hide(this.tab.num,true);
    };

}

//# sourceURL = getBaseUrl() + "/js/vjIFrameView.js"