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
function vjSVGGeneView ( viewer )
{

    // Have this.bp which is the bumperLocater object

    vjSVGView.call(this,viewer); // inherit default behaviours of the DataViewer
    if (!this.boxSize) this.boxSize = 16;
    if (this.computeSizeOfBox == undefined) this.computeSizeOfBox = true;
    if (!this.layerMargin == undefined) this.layerMargin =  16;
    if(this.exonView === undefined) this.exonView = 4;

    // Check to see if there is data
    this.hasPlotwithData = function () {
        if (!this.bp) return false;
        return true;
    }

    this.compose= function () {

         // Set height and width to max for using the full viewing area 
         var width = this.geometry.width;
         var height = this.geometry.height;
         
         var sourceIDCounter = {};
         var colorCounter = 0;
         var idCounter = 0;
         
         // Generate colors here - use random generator?
         var color = ["#E3E3E3", "#FFE0CC", "#85E0FF", "DarkSeaGreen", "yellow", "DarkCyan"];
         // Create a new array for the plots information.
         this.plots[0].children = new Array();
         
         //  Determine the number of bases that are to be displayed in this frame
         var numberOfBases = this.bp.max - this.bp.min;
         
         // Loop through the length of the bumper.  Each loop corresponds to a new 'layer' on the visualization (a new 'row' of annotations)
         for (var bumperNumber = 0; bumperNumber < this.bp.bumper.length; bumperNumber++) {
             
             // Define exon array to maintain exon values
             var exonList = {}; // Create an object for exons
             
             // Loop through each individual annotation within a given layer
             for (var rangeNumber = 0; rangeNumber < this.bp.bumper[bumperNumber].ranges.length; rangeNumber++) {
                 
                 // Grab the range for the particular element
                 var myRange = this.bp.bumper[bumperNumber].ranges[rangeNumber];
                 
                 var _idTypes_id = myRange.info["idType-id"].split(";");
                 var idTypes = new Array();
                 
                 for (var i = 0; i < _idTypes_id.length; i++) {
                     // Parse for idTypes.  Returns array of two, first has quotes around, second does not (??)
                     var checkMatch = _idTypes_id[i].match(/""([^"]+)""/);
                     var check;
                 }    
                 
                 // Returns the coordinants of visualization for this element.  Returned as an object, six points.  Typically formed into a box, but can
                 // do other shapes depending on the sourceID
                 var _coordinantList = this.returnPoints(bumperNumber, rangeNumber, width, height, numberOfBases, myRange.info["idType-id"], exonList);
                 var coordinantList = _coordinantList.coordinantList;
                 exonList = _coordinantList.exonList;
                 
                 // Check to see if the sourceID has already been seen here before.
                 if (!(myRange.sourceID in sourceIDCounter)) {
                     // Key doesn't exist (sourceID hasn't been seen yet)
                     // Assign the next color to it and then move the color counter to the following color for the next layer
                     sourceIDCounter[myRange.sourceID] = color[colorCounter];
                     colorCounter++
                 }

                 //var objID = "" + myRange.id +"_" +myRange.start;
                 //mySVGTest.trajectory(coordinantList, true, "L", objID);
                 // (this.coordinates,this.closed,this.lineCmd,this.objID);
                 
                 // Register the object with HIVE
                 var objID="vjSVG_Gene"+Math.random();
                 vjObj.register(objID,this);

                 // Generate the new SVG object based on the vjSVG_trajectory object and assign properties including mouse-over information,
                 // coordinants, etc.
                 box = new vjSVG_trajectory(  {coordinates:coordinantList,closed:true ,lineCmd:"L", objID:objID}) ;
                 box.attribute = {"name":myRange.id, "family": myRange.sourceID, "start": myRange.info.start, "end": myRange.info.end, "idType-id": myRange.info["idType-id"]};
                 box.brush={"fill": sourceIDCounter[myRange.sourceID]};
                 box.handler = {
                        "onmouseover":"function:vjObjFunc('mouseOverShowId','" + box.objID + "')",
                        "onmouseout":"function:vjObjFunc('mouseOutShowId','" + box.objID + "')",
                        "onclick":"function:vjObjFunc('clickForCallingBack','" + box.objID + "')",
                        "onmousemove":"function:vjObjFunc('mouseMove','" + box.objID + "')"
                    };

                 // Add the new shape to the plot that is being drawn.
                 this.plots[0].children.push(box);
                 idCounter++;
             }
             // Can draw introns for a given layer here
             
             for (var key in exonList) {
                 
                 var _exonPoints = exonList[key]; // Copy array for a specific gene ID into a tmp array to work with
                 if (_exonList.length == 1) continue;  // Don't need to do anything if there is just one                
                 
                 // verify array exists
                 if (_exonPoints[0] === undefined) alert("DEVELOPER WARNING: No Exon Array at this point.");
                 
                // Sort from lowest to highest by topleft value
                 _exonPoints.sort(function(firstElement,secondElement){ 
                        if (firstElement.topleft < secondElement.topleft) return -1;
                        else return 1;
                     });
                 
                 // determine intron carrot points
                 for (var jj = 0; jj < _exonPoints.length - 1; jj++) {
                     // Need to connect topright point of current one to topleft point of next one
                     var point1 = _exonPoints[jj].topright; // start point
                     var point3 = _exonPoints[jj+1].topleft; // final point
                     // Compute middle x
                     var midX = (_exonPoints[jj].topright.x + _exonPoints[jj+1].topleft.x)/2;
                     
                     var layerHeight = 0;
                     if (!this.computeSizeOfBox) layerHeight = this.boxSize;
                     else layerHeight = Math.floor((SVGheight - 10) / this.numberOfLayers);

                     // This sets the margin for use between layers.
                     var margin = 0;
                     if (!this.layerMargin) margin = layerHeight;
                     else margin = this.layerMargin;
                     var newY = _exonPoints[jj].topright.y - (margin /2); // NEEDS TO CHANGE WHEN COORDINANT SYSTEM CHANGES
                     var point2 = {x:midX,y:newY,x:0};
                     
                     var intron = new vjSVG_trajectory({coordinates:[point1,point2,point3],closed:false,lineCmd:'L',objID:"" });
                     this.plots[0].children.push(intron); // push to children list => be drawn latter
                 }
             }
         }
         
         // Display the plot on the screen based on the culmative shapes added
         this.plots[0].render(this.scene);

    }
    this.mouseOverShowId = function(ir,eventObjID,evt){
        /*var parentObj = gObject(eventObjID.parentNode.id);
        var cord = new Object();
        //cord.x = eventObjID.getAttribute("x");
        cord.x = evt.offsetX;
        //cord.y = eventObjID.getAttribute("y");
        cord.y = evt.offsetY;
        var text =  " ID    : " + eventObjID.getAttribute("name") + "\n" + "start : " + eventObjID.getAttribute("start") + "\n"  + "end    : " + eventObjID.getAttribute("end");
        var params = new Object(
                {
                    boxOpacity: 1
                    ,fontsize: "14px"
                }
                );
        this.plots[0].computeToolTip(cord,text,parentObj.id, params);*/
    }
    this.clickForCallingBack = function(ir,eventObjID,evt){

    }
    this.mouseOutShowId = function(ir,eventObjID,evt){
        var parentID = eventObjID.parentNode.id;
          if (document.getElementById(parentID).getElementById("toolTipBox")!=null){
              //alerJ("",document.getElementById("toolTipBox"))
              document.getElementById(parentID).removeChild(document.getElementById("toolTipBox"));
          }
    }

    this.mouseMove = function(ir,eventObjID,evt){
        var parentObj = gObject(eventObjID.parentNode.id);
        var cord = new Object();
        //cord.x = eventObjID.getAttribute("x");
        cord.x = evt.offsetX;
        //cord.y = eventObjID.getAttribute("y");
        cord.y = evt.offsetY;
        if (eventObjID) {
            this.mouseOutShowId(ir,eventObjID,evt);
        }
        var idTypeId_txt = eventObjID.getAttribute("idType-id");
        var idTypeId_legend = "";
        for (var ii=0; ii < idTypeId_txt.split(";").length; ++ii){
            if (ii >5) {
                idTypeId_legend += "\n..."
                break;
            }
            if (ii>0) idTypeId_legend += "\n";
            idTypeId_legend += idTypeId_txt.split(";")[ii].split(" ")[0].replace(/\"/g,"") + ": " + idTypeId_txt.split(";")[ii].split(" ")[1].replace(/\"/g,"");
        }
        var text =  " ID    : " + eventObjID.getAttribute("name") + "\n" + "start : " + eventObjID.getAttribute("start") + "\n"  + "end    : " + eventObjID.getAttribute("end") + "\n" + idTypeId_legend ;
        var params = new Object(
                {
                    boxOpacity: 1
                    ,fontsize: "14px"
                }
                );

        this.plots[0].computeToolTip(cord,text,parentObj.id, params);


        //this.mouseOutShowId(ir,eventObjID,evt);
    }

    this.toolTipsInDiv = function (cord,text,parentId,params){

    }
    
    this.determineIntrons = function() {
        
    }

    this.returnPoints=function (bumperNumber, rangeNumber, SVGwidth, SVGheight, refBases, idType, exonList) {
        //
        // Include sourceID in order to check for biologically significant fields (we determine) so 
        // they can be displayed non-generically.
        //
        var coordinantList = new Array ();
        
        // Determine how many bases each pixel is representing on the screen given use of 98% of the available real estate
        var basesPerPx = refBases / (SVGwidth * .98);

        var layer = this.bp.bumper[bumperNumber];
        var polygon = layer.ranges[rangeNumber];
        var layerHeight = 0;
        if (!this.computeSizeOfBox) layerHeight = this.boxSize;
        else layerHeight = Math.floor((SVGheight - 10) / this.numberOfLayers);

        // This sets the margin for use between layers.
        var margin = 0;
        if (!this.layerMargin) margin = layerHeight;
        else margin = this.layerMargin;
        
        // Generate the six points for the object below if in the form of a box
        
        var L1 = (bumperNumber * layerHeight) + (bumperNumber * margin);
        var L2 = ((bumperNumber + 1) * layerHeight) + (bumperNumber * margin);

        var x1 = (SVGwidth * .01) + ((polygon.start - this.bp.min) / basesPerPx) + 1;
        var x2 = (SVGwidth * .01) + ((polygon.end - this.bp.min)/ basesPerPx) - 1;
        
        var y1 = SVGheight - L2;
        var y2 = SVGheight - ((L1 + L2) / 2);
        var y3 = SVGheight - L1;

        var point1 = {x:x1, y:y1, z:0};
        var point2 = {x:x1, y:y2, z:0};
        var point3 = {x:x1, y:y3, z:0};
        var point4 = {x:x2, y:y3, z:0};
        var point5 = {x:x2, y:y2, z:0};
        var point6 = {x:x2, y:y1, z:0};
        
        // Check to see if it is exon information to give directional information with.
        // idType is given in the format:
        //       """"seqID"" chr4;""REF"" T;""ALT"" C;""QUAL"" 32;""INFO"" DP=10;AC=1;AF=0.10;CG=TC""
        // Where everything in double quotes are the idTypes for this given range.

        if (this.exonView == 0) {
            var _idTypes_id = idType.split(";");
            var idTypes = new Array();
            
            // Flags for refined drawing
            var exon = false;
            var intron = false;
            var geneId = undefined;
            var direction = -1; // -1 no information; 0 is forward (+); 1 is reverse (-)
            
            for (var i = 0; i < _idTypes_id.length; i++) {
                // Parse for idTypes.  Returns array of two, first has quotes around, second does not (??)
                var checkMatch = _idTypes_id[i].match(/""([^"]+)""/);
                var idMatch = _idTypes_id[i].match(/(\w+|.)$/);
                if (checkMatch == null || checkMatch == undefined) continue;
                if (idMatch == null || idMatch == undefined) continue;
                                
                // Save gene id in case needed
                if (checkMatch[1].toUpperCase() === "GENE_ID") {
                    if (idMatch[0] in exonList) { // Key exists
                        geneId = idMatch[0];
                    } else { // add key
                        exonList[idMatch[0]] = new Array();
                        geneId = idMatch[0];
                    }    
                }
                
                // Set any flags as needed
                // Check as exon or intron
                if (checkMatch[1].toUpperCase() === "FEATURE") {
                    if (idMatch[0].toUpperCase() === "EXON")
                        exon = true;
                    else if (idMatch[0].toUpperCase() === "INTRON")
                        intron = true;
                }
                
                // Check for direction
                if (checkMatch[1].toUpperCase() === "STRAND") {
                    if (idMatch[0].toUpperCase() === "+")
                        direction = 0;
                    else if (idMatch[0].toUpperCase() === "-")
                        direction = 1;
                }
            }
            // Need to save Exon information 
            
            // Move points to create arrow
            if (exon == true) {
                if (direction == 0) {
                    point2.x = point2.x + 5;
                    point4.x = point4.x - 5;
                    point6.x = point6.x - 5;
                } else if (direction == 1) {
                    point5.x = point5.x - 5;
                    point1.x = point1.x + 5;
                    point3.x = point3.x + 5;
                }
                
                // save the upper left, upper right, and direction in the array for this gene id
                if (geneId != undefined) { // A gene id was found along with an exon
                    exonList[geneId].push({topleft:point3,topright:point4});
                }
            }
        }    

        coordinantList.push(point1,point2,point3,point4,point5,point6);
      
        return {
            coordinantList: coordinantList,
            exonList: exonList
        };  
    }
    
    this.returnIntrons = function (coordinantList) {
        
    }

    this.init = function () {
        this.bp = vjDS[this.data[0]];
    }

    this.init();
    
    //
    // chromatinList is an array of objects with start and end values
    // chromatinList[0].start = 1000
    // chromatinList[0].end = 1300
    // ... etc.
    //
    
    this.drawChromatinBands = function (chromatinList, SVGwidth, SVGheight, referenceStart, referenceEnd) {
        var verticalMargin = SVGheight * .0001;
        var horizontalMargin = 0;
        var bpPerPixel = (referenceEnd-referenceStart) / SVGwidth;
        
        var returnPoints = new Array();
        
        for (var i = 0; i < chromatinList.length; i++) {
            if (chromatinList[i].start >= referenceStart && chromatinList[i].end <= referenceEnd) {
                //chromatin is shown here; generate points
                                
                var xleft = referenceStart / bpPerPixel;
                var xright = referenceEnd / bpPerPixel;
                var ytop = SVGheight - verticalMargin;
                var ybottom = verticalMargin;
                var chromatin = {
                        topleft :{
                            x:xleft, 
                            y:ytop, 
                            z:0
                        },
                        topright : {
                            x:xright, 
                            y:ytop, 
                            z:0
                        },
                        bottomleft : {
                            x:xleft, 
                            y:ybottom, 
                            z:0
                        },
                        bottomright : {
                            x:xright, 
                            y:ybottom, 
                            z:0
                        }        
                };
                
                returnPoints.push(chromatin);
            }
        }
        
        return returnPoints;
        // returnPoints array is a list of chromatin objects
        // each chromatin object has 
        // topleft, topright, bottomleft, and bottomright point objects
        // each point object has x, y, and z
        // EXAMPLE
        // returnPoints[5].topright.x <== this is the x coordinant for the top right point in the 6th chromatin marker (index 5)
    }
}

//# sourceURL = getBaseUrl() + "/js/vjSVGGeneView.js"