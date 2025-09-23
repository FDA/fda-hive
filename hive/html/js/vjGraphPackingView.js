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

function vjGraphPackingView( viewer )
{

    vjDataViewViewer.call(this,viewer);
    this.delayedEventHandler=1;


    this.composerFunction=function( viewer , content )
    {

        if(content.indexOf("error:")==0) {
            gObject(this.container).innerHTML=content;
            return;
        }

        var tblArr=new vjTable(content, 0, vjTable_propCSV);


        this.collapsedArr= function(locuscols,startcols,endcols){

            var locusObject = new Object();
            for (var ir=0;ir<tblArr.length;ir++){

                var locustag=tblArr.cols[locuscols];

                if (!locusObject[locustag]){
                    locusObject[locustag]={x1: valuex1, x2: valuex2};
                }
            }
        };


        this.refresh();
    };

    this.refresh = function ()
    {
       this.graphData = new google.visualization.DataTable();




        for ( var ic=0; ic< this.series.length; ++ic ) {
            this.graphData.addColumn(this.series[ic].label ? "string" : "number", this.series[ic].title ? this.series[ic].title : this.series[ic].name);
        }


        for ( var ir=0; ir< this.tblArr.rows.length; ++ir ) {

            this.graphData.addRows(1);

            var totSum=0;
            if(this.scaleTo) {
                for ( var ic=0; ic< this.series.length; ++ic ){
                    if(this.series[ic].label)continue;
                    if(this.series[ic].notScaled)continue;

                    totSum+=parseFloat(this.tblArr.rows[ir][this.series[ic].name]);
                }
            }

            for ( var ic=0; ic< this.series.length; ++ic ){

                var value=this.tblArr.rows[ir][this.series[ic].name];

                if(!value){
                    alert("DEVELOPER ALERT: vjGoogleGraph column "+ this.series[ic].name + " is not in dataset " );
                    return ;
                }
                if(!this.series[ic].label)
                    value=parseFloat(value);

                if(this.scaleTo && (!this.series[ic].label) && (!this.series[ic].notScaled) ) {

                    value=totSum ? (this.scaleTo*value)/totSum : 0 ;
                    value=parseFloat(value.toPrecision(2));
                }
                this.graphData.setValue(ir, ic , value) ;
            }

        }


        this.obj  = new google.visualization.AreaChart(this.div);


        this.obj.draw(this.graphData, this.options);

        var funcname=vjObjEventGlobalFuncName("onSelectHandler",this.objCls);
        window[funcname]= eval(vjObjEventGlobalFuncBody("onSelectHandler",this.objCls, this.delayedHandler ));
        google.visualization.events.addListener( this.obj , 'select', eval(funcname));

        return;

    };



    this.onSelectHandler = function () {
        var irow = -1;
        var icol = -1;
        var node = null;
        var funcCB = null ;
        if (this.obj.getSelection().length != 0) {
            irow = this.obj.getSelection()[0].row;
            icol = this.obj.getSelection()[0].column;
            node = this.tblArr.rows[irow];
            funcCB = node.url;
        }

        if (!funcCB && icol < this.tblArr.hdr.lengh) funcCB = this.tblArr.hdr[icol].url;
        if (!funcCB) funcCB = this.selectCallback;

        if (funcCB)
            funcLink(funcCB, this, node, irow);

    };


}

