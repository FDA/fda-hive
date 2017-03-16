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

    //this.vjType = "vjBumperLocater";
    this.dataLoaded = 'onDataLoaded';
    //this.data;  Datasource

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
    
    //this.loaded = false;
    
     this.onDataLoaded = function(callback, content, param) {
           // this.loaded = true;
             this.level = 0.25 ;
            this.tblArr = new vjTable(content, 0, vjTable_propCSV);
            if (this.bumper.length)
                this.bumper = new Array();
            //alert("tabl length " + this.tblArr.rows.length)
            if(this.precompute)
                this.tblArr.enumerate(this.precompute);
            this.init();
            
    };

    this.init = function() {
        // Reference is included in the datasource
        //var referenceLayer = 0;
        //this.bumper.add(referenceLayer,this.referenceName,this.referenceLength.min,this.referenceLength.max,this.collapse);
        this.min = 1e+300;
        this.max = -1e+300;
        for (var i=0; i< this.tblArr.rows.length; ++i){
            var row = this.tblArr.rows[i];
            if (row["sourceID"] == "virtual") continue; // exclude the virtual rows
            var start = parseInt(row[this.columnsToPick["start"]]);
            var end = parseInt(row[this.columnsToPick["end"]]);
            var additionalInfo = row;
            if (i==0) {
                // Assign the current reference start and end, the first row returned in the reference
                this.reference.start = start;
                this.reference.end = end;
                // Check to see if this is the first time based on assignment to absoluteStart and End to get absolute reference start and end positions
                if (this.reference.absoluteEnd < 0 && this.reference.absoluteStart < 0) {
                    this.reference.absoluteStart = start;
                    this.reference.absoluteEnd = end;
                }
            }

            this.add(row[this.columnsToPick["rangeID"]],row[this.columnsToPick["sourceID"]],start,end,this.collapse, additionalInfo);
        }

    }

    // rangeID - the seqID/chromosome/reference name
    // srcID - the source ID/annotation ID
    // s - start position
    // e - end position
    // collapseAcrossSources - bool flag to collapse or not

    // Function to add an annotation
    // need special case if exon/intron?
    this.add=function (rangeID, srcID, s, e , collapseAcrossSources, additionalInfo)
    {

        // Checks for min and max in order to map later (will be reference range if reference given)
        if (this.min > s) this.min = s;
        if (this.max < e) this.max = e;

        var nextLayer = false;
        var newLayer = true;

        for( var ib = 0; ib < this.bumper.length; ++ib) {
            var layer=this.bumper[ib];
            // If we should collapse across all sources OR the current layer
            // has the same ID as the current annotation
            if(collapseAcrossSources || layer.sourceID==srcID   || nextLayer) {
                nextLayer = false; // reset next layer toggle
                var draw = true;
                // check if this bumper has free space at new annotation
                for ( var ii=0; ii<layer.ranges.length; ++ ii) {
                    var r=layer.ranges[ii]
                    // Need to check every range start and end for conflict
                    // Three possible scenarios:
                    //        1. Both the end and starts are the same for the range
                    // and new annotation.  In this case we need to do nothing, annotation
                    // is already drawn.
                    //        2. The new annotation's range overlaps with the past range, but
                    // is not identical.  In this case a new annotation block needs to be drawn
                    // in a new bumper layer.
                    //        3. The annotation's range does not intersect at all with past range.
                    //    Draw annotation in current layer


                    // No need to have below, just for illustration
                    if((r.start > e) || (r.end < s)) {
                        // Doesn't overlap at all
                        draw = true;

                    }
                    // Overlap!
                    if ((r.start >= s && r.start <= e) || (r.end >= s && r.start <= e) || (r.start <= s && r.end >= e) || (r.start > s && r.end < e)) {
                        // Need to add to a new layer
                        // Can we call this function with a new object?
                        // Or some way of checking next layer for overlap
                        nextLayer = true;
                        draw = false;
                        break;
                    }
                }
                // If it needs to be drawn, push the object into the layer
                if (draw) {
                    layer.ranges.push({id: rangeID , start:s , end: e, sourceID: srcID, info: additionalInfo });
                    newLayer = false;
                    break;
                }

                // Need some way of
            }
        }
        // If not match found...
        if (newLayer) {
            layer ={ sourceID: srcID, ranges:[] };
            layer.ranges.push({id: rangeID , start:s , end: e, sourceID: srcID , info: additionalInfo});
            this.bumper.push ( layer ) ;
        }
    }

}


//// after parsing  : see vjTableView.js
//var tblArr = new vjTable(content, 0, vjTable_propCSV);
//// tblRanges = [{s,e,srcID,chromosome,id,idtye}];
//
//var bumper=new vjBumberLocator();
//for ( i=0; i< tblRanges.length; ++i)
//{
//    bumper.add(i,tblRanges[i].srcID,tblRanges[i].start,tblRanges[i].end,0);
//}
//
//
//
//// draw
//for( var ib = 0 ; ib< bumper.length; ++ib) {
//    var layer=bumper[ib];
//    for ( var ii=0; ii<layer.ranges.length; ++ ii) {
//        var r=layer.ranges[ii];
//
//        var myRangeObject=tblArr[r.rangeID];
//        //myRangeObject.start, end , cromosome, sourceID, id:  idtype .... alll of them
//        // myDrawingLayerCoordinate is ib
//
//    }
//}

//# sourceURL = getBaseUrl() + "/js/vjBumperLocater.js"