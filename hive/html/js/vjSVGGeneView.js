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


    vjSVGView.call(this,viewer);
    if (!this.boxSize) this.boxSize = 16;
    if (this.computeSizeOfBox == undefined) this.computeSizeOfBox = true;
    if (!this.layerMargin == undefined) this.layerMargin =  16;
    if(this.exonView === undefined) this.exonView = 4;

    this.hasPlotwithData = function () {
        if (!this.bp) return false;
        return true;
    }

    this.compose= function () {

         var width = this.geometry.width;
         var height = this.geometry.height;
         
         var sourceIDCounter = {};
         var colorCounter = 0;
         var idCounter = 0;
         
         var color = ["#E3E3E3", "#FFE0CC", "#85E0FF", "DarkSeaGreen", "yellow", "DarkCyan"];
         this.plots[0].children = new Array();
         
         var numberOfBases = this.bp.max - this.bp.min;
         
         for (var bumperNumber = 0; bumperNumber < this.bp.bumper.length; bumperNumber++) {
             
             var exonList = {};
             
             for (var rangeNumber = 0; rangeNumber < this.bp.bumper[bumperNumber].ranges.length; rangeNumber++) {
                 
                 var myRange = this.bp.bumper[bumperNumber].ranges[rangeNumber];
                 
                 var _idTypes_id = myRange.info["idType-id"].split(";");
                 var idTypes = new Array();
                 
                 for (var i = 0; i < _idTypes_id.length; i++) {
                     var checkMatch = _idTypes_id[i].match(/""([^"]+)""/);
                     var check;
                 }    
                 
                 var _coordinantList = this.returnPoints(bumperNumber, rangeNumber, width, height, numberOfBases, myRange.info["idType-id"], exonList);
                 var coordinantList = _coordinantList.coordinantList;
                 exonList = _coordinantList.exonList;
                 
                 if (!(myRange.sourceID in sourceIDCounter)) {
                     sourceIDCounter[myRange.sourceID] = color[colorCounter];
                     colorCounter++
                 }

                 
                 var objID="vjSVG_Gene"+Math.random();
                 vjObj.register(objID,this);

                 box = new vjSVG_trajectory(  {coordinates:coordinantList,closed:true ,lineCmd:"L", objID:objID}) ;
                 box.attribute = {"name":myRange.id, "family": myRange.sourceID, "start": myRange.info.start, "end": myRange.info.end, "idType-id": myRange.info["idType-id"]};
                 box.brush={"fill": sourceIDCounter[myRange.sourceID]};
                 box.handler = {
                        "onmouseover":"function:vjObjFunc('mouseOverShowId','" + box.objID + "')",
                        "onmouseout":"function:vjObjFunc('mouseOutShowId','" + box.objID + "')",
                        "onclick":"function:vjObjFunc('clickForCallingBack','" + box.objID + "')",
                        "onmousemove":"function:vjObjFunc('mouseMove','" + box.objID + "')"
                    };

                 this.plots[0].children.push(box);
                 idCounter++;
             }
             
             for (var key in exonList) {
                 
                 var _exonPoints = exonList[key];
                 if (_exonList.length == 1) continue;
                 
                 if (_exonPoints[0] === undefined) alert("DEVELOPER WARNING: No Exon Array at this point.");
                 
                 _exonPoints.sort(function(firstElement,secondElement){ 
                        if (firstElement.topleft < secondElement.topleft) return -1;
                        else return 1;
                     });
                 
                 for (var jj = 0; jj < _exonPoints.length - 1; jj++) {
                     var point1 = _exonPoints[jj].topright;
                     var point3 = _exonPoints[jj+1].topleft;
                     var midX = (_exonPoints[jj].topright.x + _exonPoints[jj+1].topleft.x)/2;
                     
                     var layerHeight = 0;
                     if (!this.computeSizeOfBox) layerHeight = this.boxSize;
                     else layerHeight = Math.floor((SVGheight - 10) / this.numberOfLayers);

                     var margin = 0;
                     if (!this.layerMargin) margin = layerHeight;
                     else margin = this.layerMargin;
                     var newY = _exonPoints[jj].topright.y - (margin /2);
                     var point2 = {x:midX,y:newY,x:0};
                     
                     var intron = new vjSVG_trajectory({coordinates:[point1,point2,point3],closed:false,lineCmd:'L',objID:"" });
                     this.plots[0].children.push(intron);
                 }
             }
         }
         
         this.plots[0].render(this.scene);

    }
    this.mouseOverShowId = function(ir,eventObjID,evt){
    }
    this.clickForCallingBack = function(ir,eventObjID,evt){

    }
    this.mouseOutShowId = function(ir,eventObjID,evt){
        var parentID = eventObjID.parentNode.id;
          if (document.getElementById(parentID).getElementById("toolTipBox")!=null){
              document.getElementById(parentID).removeChild(document.getElementById("toolTipBox"));
          }
    }

    this.mouseMove = function(ir,eventObjID,evt){
        var parentObj = gObject(eventObjID.parentNode.id);
        var cord = new Object();
        cord.x = evt.offsetX;
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


    }

    this.toolTipsInDiv = function (cord,text,parentId,params){

    }
    
    this.determineIntrons = function() {
        
    }

    this.returnPoints=function (bumperNumber, rangeNumber, SVGwidth, SVGheight, refBases, idType, exonList) {
        var coordinantList = new Array ();
        
        var basesPerPx = refBases / (SVGwidth * .98);

        var layer = this.bp.bumper[bumperNumber];
        var polygon = layer.ranges[rangeNumber];
        var layerHeight = 0;
        if (!this.computeSizeOfBox) layerHeight = this.boxSize;
        else layerHeight = Math.floor((SVGheight - 10) / this.numberOfLayers);

        var margin = 0;
        if (!this.layerMargin) margin = layerHeight;
        else margin = this.layerMargin;
        
        
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
        

        if (this.exonView == 0) {
            var _idTypes_id = idType.split(";");
            var idTypes = new Array();
            
            var exon = false;
            var intron = false;
            var geneId = undefined;
            var direction = -1;
            
            for (var i = 0; i < _idTypes_id.length; i++) {
                var checkMatch = _idTypes_id[i].match(/""([^"]+)""/);
                var idMatch = _idTypes_id[i].match(/(\w+|.)$/);
                if (checkMatch == null || checkMatch == undefined) continue;
                if (idMatch == null || idMatch == undefined) continue;
                                
                if (checkMatch[1].toUpperCase() === "GENE_ID") {
                    if (idMatch[0] in exonList) {
                        geneId = idMatch[0];
                    } else {
                        exonList[idMatch[0]] = new Array();
                        geneId = idMatch[0];
                    }    
                }
                
                if (checkMatch[1].toUpperCase() === "FEATURE") {
                    if (idMatch[0].toUpperCase() === "EXON")
                        exon = true;
                    else if (idMatch[0].toUpperCase() === "INTRON")
                        intron = true;
                }
                
                if (checkMatch[1].toUpperCase() === "STRAND") {
                    if (idMatch[0].toUpperCase() === "+")
                        direction = 0;
                    else if (idMatch[0].toUpperCase() === "-")
                        direction = 1;
                }
            }
            
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
                
                if (geneId != undefined) {
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
    
    
    this.drawChromatinBands = function (chromatinList, SVGwidth, SVGheight, referenceStart, referenceEnd) {
        var verticalMargin = SVGheight * .0001;
        var horizontalMargin = 0;
        var bpPerPixel = (referenceEnd-referenceStart) / SVGwidth;
        
        var returnPoints = new Array();
        
        for (var i = 0; i < chromatinList.length; i++) {
            if (chromatinList[i].start >= referenceStart && chromatinList[i].end <= referenceEnd) {
                                
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
    }
}

