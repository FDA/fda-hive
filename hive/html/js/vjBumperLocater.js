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
function vjBumperLocator(source,wantRegisterDS)
{

    vjDataSeries.call(this, source,wantRegisterDS);

    this.dataLoaded = 'onDataLoaded';

    if(!this.columnsToPick) this.columsToPick= new Object ({sourceID:"source",rangeID:"seqID",start:"start",end:"end"});
    if(this.collapse === undefined) this.collapse = true;
    if(this.maxZoom === undefined) this.maxZoom = false;
    
    if(!this.referenceName) this.referenceName = "my reference";

    this.bumper=new Array();
    this.reference = {
            start:0,
            end:0,
            absoluteStart:-1,
            absoluteEnd:-1
    }
    
    
     this.onDataLoaded = function(callback, content, param) {
             this.level = 0.25 ;
            this.tblArr = new vjTable(content, 0, vjTable_propCSV);
            if (this.bumper.length)
                this.bumper = new Array();
            if(this.precompute)
                this.tblArr.enumerate(this.precompute);
            this.init();
            
    };

    this.init = function() {
        this.min = 1e+300;
        this.max = -1e+300;
        for (var i=0; i< this.tblArr.rows.length; ++i){
            var row = this.tblArr.rows[i];
            if (row["sourceID"] == "virtual") continue;
            var start = parseInt(row[this.columnsToPick["start"]]);
            var end = parseInt(row[this.columnsToPick["end"]]);
            var additionalInfo = row;
            if (i==0) {
                this.reference.start = start;
                this.reference.end = end;
                if (this.reference.absoluteEnd < 0 && this.reference.absoluteStart < 0) {
                    this.reference.absoluteStart = start;
                    this.reference.absoluteEnd = end;
                }
            }

            this.add(row[this.columnsToPick["rangeID"]],row[this.columnsToPick["sourceID"]],start,end,this.collapse, additionalInfo);
        }

    }


    this.add=function (rangeID, srcID, s, e , collapseAcrossSources, additionalInfo)
    {

        if (this.min > s) this.min = s;
        if (this.max < e) this.max = e;

        var nextLayer = false;
        var newLayer = true;

        for( var ib = 0; ib < this.bumper.length; ++ib) {
            var layer=this.bumper[ib];
            if(collapseAcrossSources || layer.sourceID==srcID   || nextLayer) {
                nextLayer = false;
                var draw = true;
                for ( var ii=0; ii<layer.ranges.length; ++ ii) {
                    var r=layer.ranges[ii]


                    if((r.start > e) || (r.end < s)) {
                        draw = true;

                    }
                    if ((r.start >= s && r.start <= e) || (r.end >= s && r.start <= e) || (r.start <= s && r.end >= e) || (r.start > s && r.end < e)) {
                        nextLayer = true;
                        draw = false;
                        break;
                    }
                }
                if (draw) {
                    layer.ranges.push({id: rangeID , start:s , end: e, sourceID: srcID, info: additionalInfo });
                    newLayer = false;
                    break;
                }

            }
        }
        if (newLayer) {
            layer ={ sourceID: srcID, ranges:[] };
            layer.ranges.push({id: rangeID , start:s , end: e, sourceID: srcID , info: additionalInfo});
            this.bumper.push ( layer ) ;
        }
    }

}



