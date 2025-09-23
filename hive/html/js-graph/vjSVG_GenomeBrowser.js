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






function vjSVG_GenomeBrowser (source){
    
    vjSVG_Plot.call(this, source);
    
    if (!this.name) this.name = this.objID;
    
    var GB = this; 
    
    GB.colorArray = gClrTable;
    
    
    this.defaultCrd = function(){
        this.x =0; this.y=0; this.z=0;
    }
    
    this.sectionSystem = function (){
        this.topLeft = new GB.defaultCrd();
        this.topRight = new GB.defaultCrd();
        this.bottomLeft = new GB.defaultCrd();
        this.bottomRight = new GB.defaultCrd();
    }
    
    if (!this.exonView) this.exonView = 4;
    
    ;
    
    this.verticalSpace = 0.01;
    this.horizontalSpace = 0.01;
    if (this.title == undefined) this.title = "Here is the graph title";
    
    if (this.padding == undefined) this.padding = new Object();
    if (this.padding["left"] == undefined) this.padding["left"] = 0.02;
    if (this.padding["right"] == undefined) this.padding["right"] = 0.05;
    if (this.padding["top"] == undefined) this.padding["top"] = 0.02;
    if (this.padding["bottom"] == undefined) this.padding["bottom"] = 0.02;
    
    this.annotationPercent = 0.7;
    
    this.defaultValue = new Object({
         "legend": {
                      "position": "top-right"
                     ,"percent": 0.1    
        }
        ,"reference": {
                     "position":"bottom"
                    ,"type":"chromosome"
                    ,"percentHeight":0.1    
        } 
        ,"annotation":{
                    "percentHeight":0.8
        }
        ,"graphSourceArea":{
                    "percentWidth":0.1
        }
        ,'source':{'hidden':true}
    }); 
    
 
    
    if (GB["options"] == undefined) GB.options = new Object();
    
    var GBOpt = GB.options;
    if (GB["options"]["legend"] === undefined) GB.options.legend = new Object();
    if (GB["options"]["graph"] === undefined) GB.options.graph = new Object();
    
    if (GB["options"]["legend"]["position"] === undefined) GB.options.legend.position = this.defaultValue.legend.position; 
    if (GB["options"]["legend"]["percentSize"] === undefined) GB.options.legend.percentSize = this.defaultValue.legend.percent;
    if (GB["options"]["legend"]["hidden"] === undefined) GB.options.legend.hidden = true; 
    
    if (GB["options"]["graph"]["reference"] === undefined) GB.options.graph.reference = new Object();
    var GBOptRef = GB.options.graph.reference;
    
    if (GB["options"]["graph"]["reference"]["position"] === undefined) GBOptRef.position = this.defaultValue.reference.position; 
    if (GB["options"]["graph"]["reference"]["type"] === undefined) GBOptRef.type = this.defaultValue.reference.type;
    if (GB["options"]["graph"]["reference"]["color"] === undefined) GBOptRef.color = "black";
    if (GB["options"]["graph"]["annotation"] == undefined) GB.options.graph.annotation = new Object();
    var GBOptAnot = GB.options.graph.annotation;
    if (GB["options"]["graph"]["annotation"]["color"] === undefined) GBOptAnot.color =  GB.colorArray ;
    if (GB.options.graph.annotation.color.length) {
        GBOptAnot.color = verarr(GBOptAnot.color); 
        GBOptAnot.color = GBOptAnot.color.concat(GB.colorArray);    
    }
    
    if (GB["options"]["axis"] == undefined) GB.options.axis = new Object();
    if (GB["options"]["axis"]['hidden'] == undefined) GB.options.axis.hidden = false;
    
    if (GB["options"]["axis"]["maxValue"] == undefined) GB.options.axis.maxValue = new Object();
    if (GB["options"]["axis"]["maxValue"]["offset"] == undefined) GB.options.axis.maxValue.offSet = 0.02;
    if (GB["options"]["axis"]["maxValue"]["value"] == undefined) GB.options.axis.maxValue.value = -1;
    
    if (GB["options"]["axis"]["minValue"] == undefined) GB.options.axis.minValue = new Object();
    if (GB["options"]["axis"]["minValue"]["offset"] == undefined) GB.options.axis.minValue.offSet = 0.02;
    if (GB["options"]["axis"]["minValue"]["value"] == undefined) GB.options.axis.minValue.value = -1;
    
    
    
    if (GB["options"]["source"] == undefined) GB.options.source = new Object();
    if (GB["options"]["source"]["hidden"] == undefined) GB.options.hidden = true;
    
    this.construct = function (axis,chartArea,scene){
        this.chartArea = chartArea;
        this.referenceExists = false;
        
        this.overallMin = 1e+300;
        this.overallMax = -1e+300;
        
        this.determineCollectionType();
        this.prepareLegendGraphSection();
        this.prepareInsideGraphSection();
        
        var referenceDone = false;
        var annotationDone = false;
        this.colorBackground();
        if (this.reference_collection != null){
            referenceDone = this.constructReference();
        }
        else referenceDone=true;
        if (this.annotation_collection && this.annotation_collection.length){
            annotationDone = this.constructAnnotation();
        }
        if (referenceDone && annotationDone){
            if (!this.options.axis.hidden)
                this.constructAxis();
            this.constructTitle();
            this.constructLegend();
            if (!this.options.source.hidden)
                this.constructSourceArea();
            
        }
    }
    
    
    this.colorBackground = function (){
        if (!this.backgroundColor){
            this.backgroundColor = 'white';
        }
        if (this.dataObj !== undefined){
            var myObjCls = this.dataObj.gbViewerObjCls;
            var parentCls = vjObj[this.parentObjCls];
            var idx = parentCls.gbChildren.indexOf(myObjCls); 
            if (idx!=-1 && idx % 2 !=0){
                this.backgroundColor = "#f6f6f6";
            }
            else this.backgroundColor = 'white';
        }
        var backgroundBox = new vjSVG_box({
            crd:{
                 x: 0
                ,y:0
                ,z:0
                }
            ,width: 1
            ,height: 1.01    
        }); 
        backgroundBox.brush = {fill: this.backgroundColor};
        backgroundBox.pen = {'stroke-width':0};
        this.children.push(backgroundBox);
    }
    
    this.determineCollectionType = function(){
        this.reference_collection = null;
        this.annotation_collection = new Array();
        
        for (var ic=0; ic<this.collection.length; ++ic){
            var serie = this.collection[ic];
            if (serie.isReference!= undefined || serie.is_reference!= undefined){
                this.referenceExists = true;
                if (serie.isReference || serie.is_reference){
                    if (this.reference_collection) alert("there is only one reference allowed, please check your data series");
                    else this.reference_collection = serie; 
                }
                else {
                    this.annotation_collection.push(serie);
                    var minMaxObj = extractMinMaxFromUrl(serie.url,"pos_start","pos_end");
                    if (minMaxObj.min !=-1 ) {
                        this.overallMin = minMaxObj.min;
                    }
                    else {
                        if (serie.min < this.overallMin) this.overallMin = serie.min;
                    }
                    if (minMaxObj.max !=-1 ) {
                        this.overallMax = minMaxObj.max;
                    }
                    else{
                        if (serie.max > this.overallMax) this.overallMax = serie.max;
                    }
                    if (serie.exonView) this.exonView = serie.exonView;
                }
            }
            else {
                this.annotation_collection.push(serie);
                var minMaxObj = extractMinMaxFromUrl(serie.url,"pos_start","pos_end");
                if (minMaxObj.min !=-1 ) {
                    this.overallMin = minMaxObj.min;
                }
                else {
                    if (serie.min < this.overallMin) this.overallMin = serie.min;
                }
                if (minMaxObj.max !=-1 ) {
                    this.overallMax = minMaxObj.max;
                }
                else{
                    if (serie.max > this.overallMax) this.overallMax = serie.max;
                }
                if (serie.exonView) this.exonView = serie.exonView;
            }
        }
        
    }
    
    this.prepareLegendGraphSection = function (){
            
        
        var legendPosition = GB.options.legend.position;
        if (legendPosition.split("-").length==1){
            legentPosition = this.defaultValue.legend.position;
        }
        
        var leftOrRight = legendPosition.split("-")[1];
        var verticalPosition = legendPosition.split("-")[0];
        this.legendArea = new GB.sectionSystem();
        this.graphArea = new GB.sectionSystem();
        if (!GB.options.legend.hidden){
            if (leftOrRight=="left"){ 
                this.legendArea.topLeft.x = this.verticalSpace + this.padding.left;
                this.legendArea.topRight.x = this.legendArea.topLeft.x + GB.options.legend.percentSize;
                this.legendArea.bottomLeft.x = this.legendArea.topLeft.x;                
                this.legendArea.bottomRight.x = this.legendArea.topRight.x;
                
                this.graphArea.topLeft.x =  this.legendArea.topRight.x + this.verticalSpace;
                this.graphArea.topLeft.y = 1 - this.horizontalSpace - this.padding.top;
                this.graphArea.topRight.x = 1 - this.verticalSpace - this.padding.right;
                this.graphArea.topRight.y = this.graphArea.topLeft.y;
                this.graphArea.bottomLeft.x = this.graphArea.topLeft.x;
                this.graphArea.bottomLeft.y = this.horizontalSpace + this.padding.bottom;
                this.graphArea.bottomRight.x = this.graphArea.topRight.x;
                this.graphArea.bottomRight.y = this.graphArea.bottomLeft.y;
                this.graphArea.side = "right";
            } else {
                this.legendArea.topLeft.x = 1 - this.verticalSpace - GB.options.legend.percentSize ;
                this.legendArea.topRight.x = 1 - this.verticalSpace - this.padding.right;
                this.legendArea.bottomLeft.x = this.legendArea.topLeft.x;
                this.legendArea.bottomRight.x = this.legendArea.topRight.x;
                
                this.graphArea.topLeft.x = this.verticalSpace + this.padding.left;
                this.graphArea.topLeft.y = 1 - this.horizontalSpace - this.padding.top;
                this.graphArea.topRight.x = this.legendArea.topLeft.x - this.verticalSpace;
                this.graphArea.topRight.y = this.graphArea.topLeft.y;
                this.graphArea.bottomLeft.x = this.graphArea.topLeft.x;
                this.graphArea.bottomLeft.y = this.horizontalSpace + this.padding.bottom;
                this.graphArea.bottomRight.x = this.graphArea.topRight.x;
                this.graphArea.bottomRight.y = this.graphArea.bottomLeft.y ;
                this.graphArea.side = "left";
            }    
            switch (verticalPosition){
                case "middle":
                    this.legendArea.topLeft.y =  0.6;
                    this.legendArea.topRight.y =  this.legendArea.topLeft.y;
                    break;
                case "bottom":
                    this.legendArea.topLeft.y = 0.4;
                    this.legendArea.topRight.y =  this.legendArea.topLeft.y;
                    break;
                default:
                    this.legendArea.topLeft.y = 0.8;
                    this.legendArea.topRight.y =  this.legendArea.topLeft.y;
                    break;
            }
        }
        else {
            this.graphArea.topLeft.x = this.verticalSpace + this.padding.left;
            this.graphArea.topLeft.y = 1 - this.horizontalSpace - this.padding.top;
            this.graphArea.topRight.x = 1 - this.verticalSpace - this.padding.right;
            this.graphArea.topRight.y = this.graphArea.topLeft.y;
            this.graphArea.bottomLeft.x = this.graphArea.topLeft.x;
            this.graphArea.bottomLeft.y = this.horizontalSpace + this.padding.bottom;
            this.graphArea.bottomRight.x = this.graphArea.topRight.x;
            this.graphArea.bottomRight.y = this.graphArea.bottomLeft.y ;
            this.graphArea.side = "none";
        }
        
        this.legendArea.width = (this.legendArea.topRight.x - this.legendArea.topLeft.x);
        this.legendArea.height = (this.legendArea.topLeft.y - this.legendArea.bottomLeft.y);
        
        this.graphArea.width = (this.graphArea.topRight.x - this.graphArea.topLeft.x);
        this.graphArea.height = (this.graphArea.topLeft.y - this.graphArea.bottomLeft.y);
    }
    
    this.prepareInsideGraphSection = function (){
        this.graphSourceArea = new GB.sectionSystem();
        this.graphDrawingArea = new GB.sectionSystem();
        this.annotationArea = new GB.sectionSystem();
        this.referenceArea = new GB.sectionSystem();
        this.axisArea = new GB.sectionSystem();
        this.titleArea =new GB.sectionSystem();
        if (!this.options.source.hidden){
            this.graphSourceArea.topLeft = this.graphArea.topLeft;
            this.graphSourceArea.topRight.x = this.graphSourceArea.topLeft.x + this.defaultValue.graphSourceArea.percentWidth * this.graphArea.width ;
            this.graphSourceArea.topRight.y = this.graphSourceArea.topLeft.y;
        }
        else {
            this.graphSourceArea.topLeft = this.graphArea.topLeft;
            this.graphSourceArea.topRight.x = this.graphSourceArea.topLeft.x;
            this.graphSourceArea.topRight.y = this.graphSourceArea.topLeft.y;
        }
        this.graphSourceArea.bottomLeft = this.graphArea.bottomLeft;
        this.graphSourceArea.bottomRight.x = this.graphSourceArea.topRight.x;
        this.graphSourceArea.bottomRight.y = this.graphArea.bottomLeft.y;
        
        this.graphDrawingArea.topLeft = this.graphSourceArea.topRight;
        this.graphDrawingArea.topRight = this.graphArea.topRight;
        this.graphDrawingArea.bottomLeft = this.graphSourceArea.bottomRight;
        this.graphDrawingArea.bottomRight = this.graphArea.bottomRight;

        this.graphDrawingArea.width = (this.graphDrawingArea.topRight.x - this.graphDrawingArea.topLeft.x);
        this.graphDrawingArea.height = (this.graphDrawingArea.topLeft.y - this.graphDrawingArea.bottomLeft.y);
        
        if (this.referenceExists && GBOptRef.position=="bottom"){
            this.annotationArea.topLeft = this.graphDrawingArea.topLeft;
            this.annotationArea.topRight = this.graphDrawingArea.topRight;
            
            this.annotationArea.bottomLeft.x = this.annotationArea.topLeft.x;
            this.annotationArea.bottomLeft.y = this.annotationArea.topLeft.y - this.defaultValue.annotation.percentHeight * this.graphDrawingArea.height;
            this.annotationArea.bottomRight.x = this.annotationArea.topRight.x;
            this.annotationArea.bottomRight.y = this.annotationArea.bottomLeft.y;
            
            this.referenceArea.topLeft = this.annotationArea.bottomLeft;
            this.referenceArea.topRight = this.annotationArea.bottomRight;
            this.referenceArea.bottomLeft.x = this.referenceArea.topLeft.x;
            this.referenceArea.bottomLeft.y = this.referenceArea.topLeft.y - this.defaultValue.reference.percentHeight * this.graphDrawingArea.height;
            this.referenceArea.bottomRight.x = this.referenceArea.topRight.x;
            this.referenceArea.bottomRight.y = this.referenceArea.bottomLeft.y;
            
            this.axisArea.topLeft = this.referenceArea.bottomLeft;
            this.axisArea.topRight = this.referenceArea.bottomRight;
            
            
        } else if (this.referenceExists) {
            
            this.referenceArea.topLeft = this.graphDrawingArea.topLeft;
            this.referenceArea.topRight = this.graphDrawingArea.topRight;
            
            this.referenceArea.bottomLeft.x = this.referenceArea.topLeft.x;
            this.referenceArea.bottomLeft.y = this.referenceArea.topLeft.y - this.defaultValue.reference.percentHeight * this.graphDrawingArea.height;
            this.referenceArea.bottomRight.x = this.referenceArea.topRight.x;
            this.referenceArea.bottomRight.y = this.referenceArea.bottomLeft.y;
            
            this.annotationArea.topLeft = this.referenceArea.bottomLeft;
            this.annotationArea.topRight = this.referenceArea.bottomRight;
            this.annotationArea.bottomLeft.x = this.annotationArea.topLeft.x;
            this.annotationArea.bottomLeft.y = this.annotationArea.topLeft.y - this.defaultValue.annotation.percentHeight * this.graphDrawingArea.height;
            this.annotationArea.bottomRight.x = this.annotationArea.topRight.x;
            this.annotationArea.bottomRight.y = this.annotationArea.bottomLeft.y;
            
            this.axisArea.topLeft = this.annotationArea.bottomLeft;
            this.axisArea.topRight = this.annotationArea.bottomRight;
        
        }
        else {
            this.annotationArea.topLeft = this.graphDrawingArea.topLeft;
            this.annotationArea.topRight = this.graphDrawingArea.topRight;
            this.annotationArea.bottomLeft.x = this.annotationArea.topLeft.x;
            this.annotationArea.bottomLeft.y = this.annotationArea.topLeft.y - this.defaultValue.annotation.percentHeight * this.graphDrawingArea.height;
            this.annotationArea.bottomRight.x = this.annotationArea.topRight.x;
            this.annotationArea.bottomRight.y = this.annotationArea.bottomLeft.y;
            
            this.axisArea.topLeft = this.annotationArea.bottomLeft;
            this.axisArea.topRight = this.annotationArea.bottomRight;
        }
        this.axisArea.bottomLeft.x = this.axisArea.topLeft.x;
        this.axisArea.bottomLeft.y = this.axisArea.topLeft.y - 0.1 * this.graphDrawingArea.height;
        this.axisArea.bottomRight.x = this.axisArea.topRight.x;
        this.axisArea.bottomRight.y = this.axisArea.bottomLeft.y;
        
        this.titleArea.topLeft = this.axisArea.bottomLeft;
        this.titleArea.topRight = this.axisArea.bottomRight;
        this.titleArea.bottomLeft.x = this.titleArea.topLeft.x;
        this.titleArea.bottomLeft.y = this.titleArea.topLeft.y - 0.1 * this.graphDrawingArea.height;
        this.titleArea.bottomRight.x = this.titleArea.topRight.x;
        this.titleArea.bottomRight.y = this.titleArea.bottomLeft.y;
        
    }
    
    this.constructReference = function (){
        var refGroup = new vjSVG_group();
        refGroup.svgID = "referenceGroup";
        var ref = new vjSVG_box({
            crd:{
                 x: this.referenceArea.bottomLeft.x
                ,y:this.referenceArea.bottomLeft.y
                ,z:0
                }
            ,width: this.graphDrawingArea.width
            ,height: this.referenceArea.topLeft.y - this.referenceArea.bottomLeft.y    
        }); 
        ref.brush = {fill: "white"};
        refGroup.children.push(ref);
        
        var refText = new vjSVG_text({
            crd: {
                 x: this.referenceArea.topLeft.x + 0.35 * this.graphDrawingArea.width
                ,y: this.referenceArea.bottomLeft.y + 0.5 * (this.referenceArea.topLeft.y - this.referenceArea.bottomLeft.y)
                ,z:0
            }
               ,font: {"font-size": 9, "font-weight": "bold"}
               ,text: "chromosome"
               ,title: "myTitle-title"
        });
        
        var referenceData = this.reference_collection.tblArr.rows;
        if (GBOptRef.type=="chromosome"){
            this.constructChromosome(); 
        }
        this.children.push(refText);
        return true;
    }
    
    this.constructAnnotation = function(){        
        this.constructAnnotationBox();
        return true;
    }
    this.constructAnnotationBox = function (){
        
        var width = (this.annotationArea.topRight.x - this.annotationArea.topLeft.x);

        var height = (this.annotationArea.topLeft.y - this.annotationArea.bottomLeft.y);
        height = (height / this.annotation_collection.length)  ;
        
        var sourceIDCounter = {};
        var colorCounter = 0;
        var idCounter = 0;
        
        var exonList = {};
        
         var color = verarr(GBOptAnot.color);
        
             
        if (this.options.axis.maxValue.value!=undefined && this.options.axis.maxValue.value.length && this.options.axis.maxValue.value != -1 ) {
            this.annotation_collection[0].max = this.options.axis.maxValue.value;
            this.overallMax = this.options.axis.maxValue.value;
        }
        if (this.options.axis.minValue.value!=undefined && this.options.axis.minValue.value.length && this.options.axis.minValue.value != -1) {
            this.annotation_collection[0].min = this.options.axis.minValue.value;
            this.overallMin = this.options.axis.minValue.value;
        }
        var numberOfBases = this.overallMax - this.overallMin;
        
        
        var step = height + this.horizontalSpace*2
        var startingPoint = new GB.defaultCrd();
        startingPoint.x = this.annotationArea.bottomLeft.x;
        startingPoint.y = this.annotationArea.bottomLeft.y - step;
        
   for (var ia=0; ia< this.annotation_collection.length; ++ia){
       startingPoint.y += step ;
        for (var bumperNumber = 0; bumperNumber < this.annotation_collection[ia].bumper.length; bumperNumber++) {
            
            for (var rangeNumber = 0; rangeNumber < this.annotation_collection[ia].bumper[bumperNumber].ranges.length; rangeNumber++) {
                
                if (this.annotation_collection[ia].style == 'axis'){
                    this.constructAxis(this.annotationArea);
                    continue;
                }
                var myRange = this.annotation_collection[ia].bumper[bumperNumber].ranges[rangeNumber];
                
                var _idTypes_id = myRange.info["idType-id"].split(";");
                var idTypes = new Array();
                
                for (var i = 0; i < _idTypes_id.length; i++) {
                    var checkMatch = _idTypes_id[i].match(/""([^"]+)""/);
                    var check;
                }    
                
                
                var _coordinantList = this.returnPoints(startingPoint, bumperNumber, rangeNumber, width, height, numberOfBases,myRange, myRange.info["idType-id"], exonList);
                var coordinantList = _coordinantList.coordinantList;
                exonList = _coordinantList.exonList;
                if (myRange.sourceID==undefined || !myRange.sourceID.length){
                    myRange.sourceID = myRange.id;
                }
                 if (!(myRange.sourceID in sourceIDCounter)) {
                     sourceIDCounter[myRange.sourceID] = color[colorCounter];
                     colorCounter++
                 }
                
                 var objID="vjSVG_Gene"+Math.random();
                vjObj.register(objID,this);

                box = new vjSVG_trajectory(  {coordinates:coordinantList,closed:true ,lineCmd:"L", objID:objID}) ;
                box.svgID = objID;
                
                box.attribute = {"name":myRange.id, "family": myRange.sourceID, "start": myRange.info.start, "end": myRange.info.end, "idType-id": myRange.info["idType-id"].replace(/\"/g,"")};
                box.brush={"fill": pastelColor(color[ia],0.3)};
                box.pen={"stroke": pastelColor(color[ia],0.3)};
                box.handler = {
                       "onmouseover":"function:vjObjFunc('mouseOverShowId','" + box.objID + "')",
                       "onmouseout":"function:vjObjFunc('mouseOutShowId','" + box.objID + "')",
                       "onclick":"function:vjObjFunc('clickForCallingBack','" + box.objID + "')"
                   };

                this.children.push(box);
                idCounter++;
            }
            for (var key in exonList) {
                
                    var _exonPoints = exonList[key];
                    if (_exonPoints.length == 1) continue;
                    
                    if (_exonPoints[0] === undefined) console.log("DEVELOPER WARNING: No Exon Array at this point.");
                    
                    _exonPoints.sort(function(firstElement,secondElement){ 
                           if (firstElement.topleft.x < secondElement.topleft.x) return -1;
                           else return 1;
                        });
                    
                    for (var jj = 0; jj < _exonPoints.length - 1; jj++) {
                        var point1 = _exonPoints[jj].topright;
                        var point3 = _exonPoints[jj+1].topleft;
                        var midX = (_exonPoints[jj].topright.x + _exonPoints[jj+1].topleft.x)/2;
                        
                         var layerHeight = 0;
                      
                      if (!this.computeSizeOfBox) layerHeight = this.boxSize / (height * this.chartArea.height);
                      else layerHeight = (height * this.chartArea.height) / this.numberOfLayers;
    
                      var margin = 0;
                      if (!this.layerMargin) margin = layerHeight;
                      else margin = this.layerMargin;
                      
                      var yOffset;
                      
                      if (_exonPoints[jj].topright.y > _exonPoints[jj+1].topright.y) {
                          yOffset = _exonPoints[jj].topright.y;
                      } else {
                          yOffset = _exonPoints[jj+1].topright.y;
                      }
                      
                        var newY = yOffset + (margin /2); 
                        var point2 = {x:midX,y:newY,z:0};
                        
                        var intron = new vjSVG_trajectory({coordinates:[point1,point2,point3],closed:false,lineCmd:'L',objID:"" });
                        this.children.push(intron);
                    }
            }
        }
      }  
    }
    this.mouseOverShowId = function(ir,eventObjID,evt){
        
        if (this.mouseoverCallback){
            funcLink(this.mouseoverCallback, this, eventObjID,ir);
            return;
        }
        var parentObj = gObject(eventObjID.parentNode.id);
        var cord = new Object();
        cord.x = evt.offsetX;
        cord.y = evt.offsetY;
        var text =  " ID    : " + eventObjID.getAttribute("name") + "\n" + "start : " + eventObjID.getAttribute("start") + "\n"  + "end    : " + eventObjID.getAttribute("end") + "\n";
        var idTypeId_txt = eventObjID.getAttribute("idType-id").replace(/; /g,' ');
        if (idTypeId_txt){
            for (var ii=0; ii < idTypeId_txt.split(";").length; ++ii){
                var myText = idTypeId_txt.split(";")[ii].replace(/\"/g,"");
                if (ii >8) {
                    text += "\n..."
                    break;
                }
                if (ii>0) text += "\n";
                if (myText.length > 32) text +=myText.substring(0,15) + "...";
                else text += myText;
            }
        }
        var params = new Object(
                {
                    boxOpacity: 1
                    ,fontsize: "14px"
                }
                );
        this.computeToolTip(cord,text,parentObj.id, params);
    }
    this.clickForCallingBack = function(ir,eventObjID,evt){
        if (this.selectCallback)
            funcLink(this.selectCallback, this, eventObjID,ir);
    }
    this.mouseOutShowId = function(ir,eventObjID,evt){
        if (this.mouseoutCallback){
            funcLink(this.mouseoutCallback, this, eventObjID,ir);
            return;
        }
        var parentID = eventObjID.parentNode.id;
          if (document.getElementById("toolTipBox")!=null){
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
            idTypeId_legend += idTypeId_txt.split(";")[ii].replace(/\"/g,"");
        }
        var text =  " ID    : " + eventObjID.getAttribute("name") + "\n" + "start : " + eventObjID.getAttribute("start") + "\n"  + "end    : " + eventObjID.getAttribute("end") + "\n" + idTypeId_legend ;
        var params = new Object(
                {
                    boxOpacity: 1
                    ,fontsize: "14px"
                }
                );

        this.computeToolTip(cord,text,parentObj.id, params);


    }
    this.returnPoints=function (startingPoint,bumperNumber, rangeNumber, SVGwidth, SVGheight, refBases, myRange,idType,exonList) {
        if (!this.boxSize) this.boxSize = 10;
        if(!this.numberOfLayers) this.numberOfLayers = this.annotation_collection[0].bumper.length;
        var coordinantList = new Array ();
        
        var basesPerPx = refBases / (SVGwidth);

        var polygon = myRange;
        var layerHeight = 0;
        
        var heightInPixel = this.graphArea.height * this.chartArea.height;
        if (this.computeSizeOfBox !=undefined && this.computeSizeOfBox) layerHeight = this.boxSize / (heightInPixel);
        else layerHeight = (SVGheight * 0.8) / this.numberOfLayers;

        var margin = 0;
        if (!this.layerMargin) margin = layerHeight * 0.2;
        else margin = this.layerMargin;
        
        
        var L1 = (bumperNumber * layerHeight) + (bumperNumber * margin);
        var L2 = ((bumperNumber + 1) * layerHeight) + (bumperNumber * margin);

        if (polygon.start < this.overallMin) {
            polygon.start = this.overallMin;
        }
        if (polygon.end > this.overallMax) {
            polygon.end = this.overallMax;
        }
        
        var x1 = startingPoint.x + (polygon.start - this.overallMin )/ basesPerPx;
        var x2 = startingPoint.x + (polygon.end - this.overallMin) / basesPerPx;
        var directionShift = 0;
        
        var y1 = startingPoint.y + L2;
        var y2 = startingPoint.y + ((L1 + L2) / 2);
        var y3 = startingPoint.y + L1;

        var point1 = {x:x1 , y:y1, z:0};
        var point2 = {x:x1+directionShift, y:y2, z:0};
        var point3 = {x:x1, y:y3, z:0};
        
        var point4 = {x:x2-directionShift, y:y3, z:0};
        var point5 = {x:x2, y:y2, z:0};
        var point6 = {x:x2-directionShift, y:y1, z:0};
        
        var direction = idType.match(/\;?strand\:[+/-]/);
        if (direction && direction.length) {
            direction = (direction[0].split(":")[1]=="+") ? true : false;
        }
        else direction = true;
        
        if (!direction) {
            point1.x = point1.x + directionShift
            point2.x = point2.x - directionShift;
            point3.x = point1.x;
            
            point4.x = point4.x + directionShift;
            point6.x = point4.x;
            point5.x = point5.x - directionShift;
            
        }        

        coordinantList.push(point1,point2,point3,point4,point5,point6);
      
        return {
            coordinantList: coordinantList,
            exonList: exonList
        };  
    }
    
    this.constructAxis = function(areaToDraw){
        if (!areaToDraw){ 
            areaToDraw = this.axisArea;
        }    
        var line = new vjSVG_line({
             crd1:  {
                 x:  areaToDraw.bottomLeft.x
                ,y:  areaToDraw.bottomLeft.y + (areaToDraw.topLeft.y - areaToDraw.bottomLeft.y) * 0.5
                ,z: 0
            }
            ,crd2:{
                 x: areaToDraw.bottomRight.x
                ,y: areaToDraw.bottomRight.y + (areaToDraw.topRight.y - areaToDraw.bottomRight.y)*0.5 
            }
            ,lineCmd:"L"
            ,objID: "lineAxis"
        });
        var arrowHead=new vjSVG_symbol({
            definition:"arrowhead",
            crd:line.crd2,
            size:0.02,
            rotation:{
                crd:line.crd2,
                vec:{x:0,y:0,z:1},
                angle:270
            }
        });
        arrowHead.brush={color:'black',rule:"nonzero"};
        this.children.push(line);
        this.children.push(arrowHead);
        
        var minValue = this.overallMin;
        var minText = new vjSVG_text({
            crd: {
                 x: areaToDraw.bottomLeft.x
                ,y: areaToDraw.bottomLeft.y
                ,z:0
            }
           ,font: {"font-size": 10, "font-weight": "bold"}
           ,text: minValue
           ,title: "min: " + String(minValue)
           ,dx: ".3em"
           ,dy: ".3em"
        });
        minText.svgID = "minText";
        this.children.push(minText);
        
        var maxValue = this.overallMax;
        var offSetLeft = this.options.axis.maxValue.offset;
        if (parseInt(maxValue)>1e8) {
            offSetLeft = 0.08;
        }
        var maxText = new vjSVG_text({
            crd: {
                 x: areaToDraw.bottomRight.x - offSetLeft
                ,y: areaToDraw.bottomRight.y
                ,z:0
            }
           ,font: {"font-size": 10, "font-weight": "bold"}
           ,text: maxValue
           ,title: "max: " + String(maxValue) 
           ,dx: ".3em"
           ,dy: ".3em"   
        });
        maxText.svgID = "maxText";
        this.children.push(maxText);
    }
    
    this.constructTitle = function(){
        var title = new vjSVG_text({
             crd: {
                 x: this.titleArea.bottomLeft.x + 0.35 * this.graphDrawingArea.width
                ,y: this.titleArea.bottomLeft.y + 0.5 * (this.titleArea.topLeft.y - this.titleArea.bottomLeft.y)
                ,z:0
            }
            ,font: {"font-size": 10, "font-weight": "bold"}
            ,text: this.title
            ,title: "myTitle-title"
        });
        title.svgID = "title";
        this.children.push(title);
    }
    
    this.constructLegend = function(){
        if (!this.options.legend.visible) return;
        var legend = new vjSVG_box({
                crd:{
                 x: this.legendArea.bottomLeft.x
                ,y: this.legendArea.bottomLeft.y + 0.8 * (this.legendArea.topLeft.y - this.legendArea.bottomLeft.y)
                ,z: 0
            }
            ,width: this.legendArea.width
            ,height: this.legendArea.height/3
        });
        legend.svgID = "legend";
        legend.brush = {fill: "white"};
        
        var legendText = new vjSVG_text({
            crd: {
                 x: this.legendArea.bottomLeft.x + 0.25 * this.legendArea.width
                ,y: this.legendArea.bottomLeft.y + 1.01 * (this.legendArea.topLeft.y - this.legendArea.bottomLeft.y)
                ,z:0
            }
               ,font: {"font-size": 10, "font-weight": "bold"}
               ,text: this.collection[0].title
               ,title: "myTitle-title"
               ,dx: ".3em"
            ,dy: ".3em"
        });
        
        this.children.push(legend);
        this.children.push(legendText);
    }
    
    this.constructSourceArea = function(){
        var objID="vjSVG_Close"+Math.random();
        vjObj.register(objID,this);
        
        var xText = new vjSVG_image({
            crd: {
                 x: this.graphSourceArea.bottomLeft.x + 0.05 * ( this.graphSourceArea.topRight.x - this.graphSourceArea.topLeft.x)
                 ,y: this.graphSourceArea.bottomLeft.y + (this.graphArea.height) 
                 ,z:0
            }
               ,title: "Close"
               ,url:'img/delete.gif'
                ,width:   -(10/this.chartArea.width)
                ,height: -(10/this.chartArea.height)
               ,objID:objID   
        });
        xText.handler = {
                "onmouseover":"function:vjObjFunc('mouseOverClose','" + xText.objID + "')",
                "onmouseout":"function:vjObjFunc('mouseOutClose','" + xText.objID + "')",
                "onclick":"function:vjObjFunc('clickForClose','" + xText.objID + "')"
        };
        xText.attribute = {'cursor':'pointer'};
        
        var source = new vjSVG_box({
                crd:{
                x: this.graphSourceArea.bottomRight.x - (this.graphSourceArea.topRight.x - this.graphSourceArea.topLeft.x - this.verticalSpace) *0.2
                ,y: this.graphSourceArea.bottomLeft.y
                ,z: 0
            }
            ,width: (this.graphSourceArea.topRight.x - this.graphSourceArea.topLeft.x - this.verticalSpace) *0.1
            ,height: this.graphArea.height *0.85    
        });
        source.svgID = "source";
        source.brush = {fill: "gray"};
        
        var sourceText = new vjSVG_text({
            crd: {
                 x: this.graphSourceArea.bottomLeft.x + 0.35 * ( this.graphSourceArea.topRight.x - this.graphSourceArea.topLeft.x)
                ,y: this.graphSourceArea.bottomLeft.y + 0.9 * (this.graphArea.height) 
                ,z:0
            }
               ,font: {"font-size": 11, "font-weight": "bold"}
               ,text: this.options.source.title ? this.options.source.title : ""
               ,title: this.options.source.title ? this.options.source.title : ""
        });
        
        this.children.push(source);
        this.children.push(sourceText);
    };
    
    this.mouseOverClose = function(ir,eventObjID,evt){
        if (this.mouseOverForCloseCallback){
            funcLink(this.mouseOverForCloseCallback, this, eventObjID,ir);
        }
    };
    this.mouseOutClose = function(ir,eventObjID,evt){
        if (this.mouseOutForCloseCallback){
            funcLink(this.mouseOutForCloseCallback, this, eventObjID,ir);
        }
    };
    this.clickForClose = function(ir,eventObjID,evt){
        if (this.clickForCloseCallback){
            funcLink(this.clickForCloseCallback, this, eventObjID,ir);
        }
    };
    
    this.constructChromosome = function (){
        var chromosomeColor = "salmon";
        var width = this.referenceArea.topRight.x - this.referenceArea.topLeft.x;
        var height = this.referenceArea.topLeft.y - this.referenceArea.bottomLeft.y;
        
        var leftCurveCrd = new Object();
        leftCurveCrd.top = new GB.defaultCrd();
        leftCurveCrd.middle = new GB.defaultCrd();
        leftCurveCrd.bottom = new GB.defaultCrd();
        
        leftCurveCrd.top.x = this.referenceArea.topLeft.x + 0.05 * (width);
        leftCurveCrd.top.y = this.referenceArea.topLeft.y - 0.2* height;
        
        leftCurveCrd.middle.x = this.referenceArea.topLeft.x;
        leftCurveCrd.middle.y = this.referenceArea.bottomLeft.y     + 0.5 * (height);
        
        leftCurveCrd.bottom.x = leftCurveCrd.top.x;
        leftCurveCrd.bottom.y = this.referenceArea.bottomLeft.y + 0.2* height;
        
        var chromosomeHeight = leftCurveCrd.top.y - leftCurveCrd.bottom.y;
        
        
        var leftCurve = new vjSVG_trajectory({
                             coordinates: [leftCurveCrd.top,leftCurveCrd.middle,leftCurveCrd.bottom]
                            ,closed:0
                            ,lineCmd:"S"
                        });
        leftCurve.brush = {fill:chromosomeColor};
        leftCurve.pen = {stroke:chromosomeColor};
        this.children.push(leftCurve);
        
        var middlePartArr = new Array();
        var middlePart= new Object();
        middlePart.point1 = new GB.defaultCrd();
        middlePart.point1 = leftCurveCrd.top;
        middlePartArr.push(middlePart.point1);
        
        middlePart.point2 = new GB.defaultCrd();
        middlePart.point2 = leftCurveCrd.bottom;
        middlePartArr.push(middlePart.point2);
        
        middlePart.point3 = new GB.defaultCrd();
        middlePart.point3.x = this.referenceArea.bottomLeft.x + 0.4 * width;
        middlePart.point3.y = middlePart.point2.y;
        middlePartArr.push(middlePart.point3);
        
        middlePart.point4 = new GB.defaultCrd();
        middlePart.point4.x = this.referenceArea.bottomLeft.x + 0.45 * width;
        middlePart.point4.y = leftCurveCrd.bottom.y + 0.3 * (chromosomeHeight);
        middlePartArr.push(middlePart.point4);
        
        middlePart.point5 = new GB.defaultCrd();
        middlePart.point5.x = this.referenceArea.bottomLeft.x + 0.5 * width;
        middlePart.point5.y = middlePart.point3.y;
        middlePartArr.push(middlePart.point5);
        
        middlePart.point6 = new GB.defaultCrd();
        middlePart.point6.x = this.referenceArea.bottomLeft.x + 0.9 * width;
        middlePart.point6.y = middlePart.point3.y;
        middlePartArr.push(middlePart.point6);
        
        middlePart.point7 = new GB.defaultCrd();
        middlePart.point7.x = middlePart.point6.x;
        middlePart.point7.y = middlePart.point1.y;
        middlePartArr.push(middlePart.point7);
        
        middlePart.point8 = new GB.defaultCrd();
        middlePart.point8.x = middlePart.point5.x;
        middlePart.point8.y = middlePart.point1.y;
        middlePartArr.push(middlePart.point8);
        
        middlePart.point9 = new GB.defaultCrd();
        middlePart.point9.x = middlePart.point4.x;
        middlePart.point9.y = leftCurveCrd.bottom.y + 0.6 * chromosomeHeight;
        middlePartArr.push(middlePart.point9);
        
        middlePart.point10 = new GB.defaultCrd();
        middlePart.point10.x = middlePart.point3.x;
        middlePart.point10.y = middlePart.point1.y;
        middlePartArr.push(middlePart.point10);
        
        var middlePartObj = new vjSVG_trajectory({
                     coordinates: middlePartArr
                    ,closed:1
                    ,lineCmd:"L"
                });
        middlePartObj.brush = {fill:chromosomeColor};
        middlePartObj.pen = {stroke:chromosomeColor};
        this.children.push(middlePartObj);
        
        var rightCurveCrd = new Object();
        rightCurveCrd.top = new GB.defaultCrd();
        rightCurveCrd.top = middlePart.point7;
        
        rightCurveCrd.bottom = new GB.defaultCrd();
        rightCurveCrd.bottom = middlePart.point6;
        
        rightCurveCrd.middle = new GB.defaultCrd();
        rightCurveCrd.middle.x = this.referenceArea.bottomRight.x;
        rightCurveCrd.middle.y = leftCurveCrd.middle.y;
        
        var rightCurve = new vjSVG_trajectory({
                  coordinates: [rightCurveCrd.top, rightCurveCrd.middle, rightCurveCrd.bottom]
                ,closed:1
                ,lineCmd:"S"
            });
        rightCurve.brush = {fill: chromosomeColor};
        rightCurve.pen = {stroke: chromosomeColor};
        this.children.push(rightCurve);
        
        var portion = [0.01,0.015,0.03,0.05,0.08,0.1,0.13,0.16,0.5,0.58,0.8];
        for (var i=0; i< portion.length; ++i){
                var band1= new vjSVG_box({
                         crd: {
                                 x: middlePart.point2.x + portion[i] * width
                                ,y: middlePart.point2.y
                                ,z:0
                        }
                        ,width: 0.01 * width
                        ,height: chromosomeHeight
                        ,brush: {fill:"black"}
                });
                this.children.push(band1);
     
        }
    }
    
}

function vjSVG_reference(source){
    vjSVG_Plot.call(this, source);
    if (this.options == undefined) this.options = new Object();
    if (this.options.padding == undefined) this.options.padding = new Object();
    
    if (this.options.padding.left == undefined) this.options.padding.left = 0.01;
    if (this.options.padding.right == undefined) this.options.padding.right = 0.01;
    if (this.options.padding.bottom == undefined) this.options.padding.bottom = 0.01;
    if (this.options.padding.top == undefined) this.options.padding.top = 0.01;
    
    
    this.construct = function (axis,chartArea,scene){
        this.chartArea = chartArea;
        
        var minMaxObj = extractMinMaxFromUrl(this.collection[0].url,"pos_start","pos_end");
        if (minMaxObj.min !=-1 ) {
            this.overallMin = minMaxObj.min;
        }
        else {
            this.overallMin = this.collection[0].min;
        }
        if (minMaxObj.max !=-1 ) {
            this.overallMax = minMaxObj.max;
        }
        else{
            this.overallMax = this.collection[0].max;
        }
        if (this.type=='chromosome'){
            this.contructChromosome();
        }
        else {
            this.constructSimpleAxis();
        }
        return true;        
    }
    
    this.constructSimpleAxis = function (){
        var refGroup = new vjSVG_group();
        refGroup.svgID = "referenceGroup";
        var source = new vjSVG_box({
            crd:{
                x: 0.01
                ,y: 0.01
                ,z: 0
            }
            ,width: 1
            ,height: 0.9    
        });
        source.pen = {fill:"white"};
        source.brush = {fill: "#f2f2f2"};
        
        this.children.push(source);
        
        var line = new vjSVG_line({
            crd1:  {
                x:  this.options.padding.left
               ,y: 0.5
           }
           ,crd2:{
                x: 1 - this.options.padding.right
               ,y: 0.5 
           }
           ,lineCmd:"L"
           ,objID: "lineAxis"
       });
       var arrowHead=new vjSVG_symbol({
           definition:"arrowhead",
           crd:line.crd2,
           size:0.02,
           rotation:{
               crd:line.crd2,
               vec:{x:0,y:0,z:1},
               angle:270
           }
       });
       arrowHead.brush={color:'black',rule:"nonzero"};
       
       this.children.push(line);
       this.children.push(arrowHead);
       
       var minValue = this.minValue ? this.minValue : this.overallMin;
       var minText = new vjSVG_text({
           crd: {
                x: this.options.padding.left
               ,y: 0.5 + 0.2
           }
          ,font: {"font-size": 10, "font-weight": "bold"}
          ,text: minValue
          ,title: "min: " + String(minValue)
          ,dx: ".3em"
          ,dy: ".3em"
       });
       minText.svgID = "minText";
       this.children.push(minText);
       
       var maxValue = this.maxValue ? this.maxValue : this.overallMax;
       var offSetLeft = 0.06;
       if (parseInt(maxValue)>1e8) {
           offSetLeft = 0.08;
       }
       var maxText = new vjSVG_text({
           crd: {
                x: 1 - this.options.padding.right - offSetLeft
               ,y: 0.5 + 0.2
           }
          ,font: {"font-size": 10, "font-weight": "bold"}
          ,text: maxValue
          ,title: "max: " + String(maxValue) 
          ,dx: ".3em"
          ,dy: ".3em"   
       });
       maxText.svgID = "maxText";
       this.children.push(maxText);
       
       var title = new vjSVG_text({
           crd: {
                x: (1 - this.options.padding.right - offSetLeft)/3
               ,y: 0.5 + 0.2
           }
          ,font: {"font-size": 12, "font-weight": "bold"}
          ,text: (this.title.length < 80 ) ? this.title : (this.title.substring(0,25) + "..." + this.title.substring(this.title.length -25, this.title.length)) 
          ,title: this.title
          ,dx: ".3em"
          ,dy: ".3em"
       });
       this.children.push(title);
    
    };
    
    this.contructChromosome = function(){
        
    };
}


function vjSVG_GenomeBrowserControl (source){
    var main_objCls = "obj-GBrowser"+Math.random();
    vjObj.register(main_objCls,this);
    
    this.clone=function (viewbase)
    {
        if(!viewbase)viewbase=this;
        var i=0;
        for ( i in viewbase ) {
            this[i] = viewbase[i];
        }

    };
    this.clone(source);
    
    
    if (this.formObject==undefined || !this.formObject) this.formObject = document.forms["formGeneric"];
    
    if (this.bumper== undefined || !this.bumper) this.bumper = new Object();
    if (this.bumper.numberOfLayers===undefined || !this.bumper.numberOfLayers) this.bumper.numberOfLayers=10;
    if (this.bumper.computeSizeOfBox===undefined || !this.bumper.computeSizeOfBox) this.bumper.computeSizeOfBox = false;
    if (this.bumper.boxSize===undefined || !this.bumper.boxSize) this.bumper.boxSize = 10;
    
    if (this.columnsToPick == undefined || !this.columnsToPick) this.columnsToPick = new Object();
    if (this.columnsToPick.sourceID===undefined || !this.columnsToPick.sourceID) this.columnsToPick.sourceID="";
    if (this.columnsToPick.rangeID===undefined || !this.columnsToPick.rangeID) this.columnsToPick.rangeID="seqID";
    if (this.columnsToPick.start===undefined || !this.columnsToPick.start) this.columnsToPick.start ="start";
    if (this.columnsToPick.end===undefined || !this.columnsToPick.end) this.columnsToPick.end ="end";

    this.datasourceListForBoxList = [];
    this.addedTrackCnt = 0;
    this.removedTrackCnt = 0;
    this.existingTrackCnt = 0;
    this.gbChildren=[];
    
    function constructTypeListTable(param,text){
        var tt="#,types\n";
        var split = text.split("\n");
        var j=1;
        for (var i=0; i<split.length; ++i){
            if (!split[i].length) continue;
            tt+= ""  +j + ","+ split[i] + "\n";
            ++j;
            if (split[i].toLowerCase().indexOf("features")!=-1){
                tt+= ""  +j + ","+ "CDS" + "\n"; ++j;
                tt+= ""  +j + ","+ "mat_peptide" + "\n"; ++j;
                tt+= ""  +j + ","+ "misc_feature" + "\n"; ++j;
            }
        }
        return tt;
    }
    
    function constructSeqListPanel(param, text) {
        
        return text;
    }
    
    this.init = function(){
        this.dataBoxListDsname = this.data;
        this.dataBoxUrl = this.url;
        
        if (!this.dataBoxListDsname) this.dataBoxListDsname = "dsGB_boxList";
        if (!this.dataBoxUrl)  this.dataBoxUrl = "static:
        
        this.dataObjList = {
                refGenomeList:{
                    dsname: "dsGB_refGenomeList",
                    url: "http:
                    title: "Loading reference genome list ..."
                },
                refSeqIDList:{
                    dsname: "dsGB_refSeqIDList",
                    url: "static:
                    title: "Loading sequence ID list ..."
                },
                annotList: {
                    dsname: "dsGB_annotList",
                    url: "http:
                    title: "Loading annotation file list ..."
                },
                typeList : {
                    dsname: "dsGB_typeList",
                    url:"static:
                    title: "Loading annotation type list ..."
                },
                seqList : {
                    dsname: "dsGB_seqList",
                    url:"static:
                    title: "Loading annotation sequence list ..."
                },
                boxList : {
                    dsname: this.dataBoxListDsname,
                    url: this.dataBoxUrl,
                    title: "Loading annotation box ..."
                },
                infoList:{
                    dsname: 'dsGB_infoList',
                    url: 'static:
                    title: "Loading annotation information ..."
                }
        };
    
        vjDS.add(this.dataObjList["refGenomeList"].title, this.dataObjList["refGenomeList"].dsname, this.dataObjList["refGenomeList"].url);
        vjDS.add(this.dataObjList["refSeqIDList"].title, this.dataObjList["refSeqIDList"].dsname, this.dataObjList["refSeqIDList"].url);
        vjDS.add(this.dataObjList["annotList"].title, this.dataObjList["annotList"].dsname, this.dataObjList["annotList"].url);
        vjDS.add(this.dataObjList["typeList"].title, this.dataObjList["typeList"].dsname, this.dataObjList["typeList"].url).parser = constructTypeListTable;
        vjDS.add(this.dataObjList["seqList"].title, this.dataObjList['seqList'].dsname, this.dataObjList['seqList'].url).parserr = constructSeqListPanel;
        vjDS.add(this.dataObjList["boxList"].title, this.dataObjList["boxList"].dsname, this.dataObjList["boxList"].url);
        vjDS.add(this.dataObjList["infoList"].title, this.dataObjList['infoList'].dsname, this.dataObjList['infoList'].url);
        
    }

    this.init();
    
    this.refGenomeList_panel = new vjBasicPanelView({
        data: ["dsVoid", this.dataObjList["refGenomeList"].dsname],
        formObject: this.formObject,
        parentObjCls: main_objCls,
        relatedDataSource: {
            annotList: {dsname: vjDS[this.dataObjList["annotList"].dsname].name, url: vjDS[this.dataObjList["annotList"].dsname].url},
            typeList: {dsname: vjDS[this.dataObjList["typeList"].dsname].name, url: vjDS[this.dataObjList["typeList"].dsname].url},
            seqList: {dsname: vjDS[this.dataObjList["seqList"].dsname].name, url: vjDS[this.dataObjList["seqList"].dsname].url},
            infoList: {dsname: vjDS[this.dataObjList["infoList"].dsname].name, url: vjDS[this.dataObjList["infoList"].dsname].url},
            boxList: {dsname: vjDS[this.dataObjList["boxList"].dsname].name, url: vjDS[this.dataObjList["boxList"].dsname].url}
        }
    });

    this.refGenomeList_viewer = new vjConstructRefGenomeList_viewer({
        data: this.dataObjList["refGenomeList"].dsname,
        parentObjCls: main_objCls,
        relatedDataSourceList: ["refSeqIDList","annotList","typeList","seqList","infoList","boxList"],
        dataObjList: this.dataObjList
    });
    
    vjDS[this.dataObjList["refGenomeList"].dsname].myAssociatedViewers = [this.refGenomeList_panel,this.refGenomeList_viewer];
    
    this.refSeqIDList_panel = new vjBasicPanelView({
        data: ["dsVoid", this.dataObjList["refSeqIDList"].dsname],
        formObject: this.formObject,
        parentObjCls: main_objCls,
        relatedDataSource: {
            refGenomeList:{dsname: vjDS[this.dataObjList["refGenomeList"].dsname].name, url: vjDS[this.dataObjList["refGenomeList"].dsname].url},
            annotList: {dsname: vjDS[this.dataObjList["annotList"].dsname].name, url: vjDS[this.dataObjList["annotList"].dsname].url},
            typeList: {dsname: vjDS[this.dataObjList["typeList"].dsname].name, url: vjDS[this.dataObjList["typeList"].dsname].url},
            seqList: {dsname: vjDS[this.dataObjList["seqList"].dsname].name, url: vjDS[this.dataObjList["seqList"].dsname].url},
            infoList: {dsname: vjDS[this.dataObjList["infoList"].dsname].name, url: vjDS[this.dataObjList["infoList"].dsname].url},
            boxList: {dsname: vjDS[this.dataObjList["boxList"].dsname].name, url: vjDS[this.dataObjList["boxList"].dsname].url}
        }
    });

    this.refSeqIDList_viewer = new vjConstructRefSeqIDList_viewer({
        data: this.dataObjList["refSeqIDList"].dsname,
        parentObjCls: main_objCls,
        selectCallback: refSeqIDListSelectCallback,
        relatedDataSourceList: ["refSeqIDList","annotList","typeList","seqList","infoList","boxList"],
        dataObjList: this.dataObjList
    });
    
    vjDS[this.dataObjList["refSeqIDList"].dsname].myAssociatedViewers = [this.refSeqIDList_panel,this.refSeqIDList_viewer];
    
    this.annotList_panel = new vjBasicPanelView({
        data: ["dsVoid", this.dataObjList["annotList"].dsname],
        formObject: this.formObject,
        parentObjCls: main_objCls,
        relatedDataSource: {
            refGenomeList:{dsname: vjDS[this.dataObjList["refGenomeList"].dsname].name, url: vjDS[this.dataObjList["refGenomeList"].dsname].url},
            refSeqIDList: {dsname: vjDS[this.dataObjList["refSeqIDList"].dsname].name, url: vjDS[this.dataObjList["refSeqIDList"].dsname].url},
            typeList: {dsname: vjDS[this.dataObjList["typeList"].dsname].name, url: vjDS[this.dataObjList["typeList"].dsname].url},
            seqList: {dsname: vjDS[this.dataObjList["seqList"].dsname].name, url: vjDS[this.dataObjList["seqList"].dsname].url},
            infoList: {dsname: vjDS[this.dataObjList["infoList"].dsname].name, url: vjDS[this.dataObjList["infoList"].dsname].url},
            boxList: {dsname: vjDS[this.dataObjList["boxList"].dsname].name, url: vjDS[this.dataObjList["boxList"].dsname].url}
        }
    });

    this.annotList_viewer = new vjConstructAnnotList_viewer({
        data: this.dataObjList["annotList"].dsname,
        parentObjCls:main_objCls,
        relatedDataSourceList: ["refGenomeList","refSeqIDList","typeList","seqList","infoList","boxList"],
        dataObjList: this.dataObjList,
        selectCallback: annotListSelectCallback
    });
    
    vjDS[this.dataObjList["annotList"].dsname].myAssociatedViewers = [this.annotList_panel,this.annotList_viewer];

    this.typeList_panel = new vjPanelView({
        data: ["dsVoid", this.dataObjList["typeList"].dsname],
        parentObjCls: main_objCls,
        formObject: this.formObject,
        iconSize : 24,
        rows: [
                {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                {name:'pager', icon:'page', title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']}
               ],
        relatedDataSource: {
            refGenomeList:{dsname: vjDS[this.dataObjList["refGenomeList"].dsname].name, url: vjDS[this.dataObjList["refGenomeList"].dsname].url},
            refSeqIDList: {dsname: vjDS[this.dataObjList["refSeqIDList"].dsname].name, url: vjDS[this.dataObjList["refSeqIDList"].dsname].url},
            annotList: {dsname: vjDS[this.dataObjList["annotList"].dsname].name, url: vjDS[this.dataObjList["annotList"].dsname].url},
            seqList: {dsname: vjDS[this.dataObjList["seqList"].dsname].name, url: vjDS[this.dataObjList["seqList"].dsname].url},
            infoList: {dsname: vjDS[this.dataObjList["infoList"].dsname].name, url: vjDS[this.dataObjList["infoList"].dsname].url},
            boxList: {dsname: vjDS[this.dataObjList["boxList"].dsname].name, url: vjDS[this.dataObjList["boxList"].dsname].url}
        }
    });
    
    
    this.typeList_viewer = new vjConstructTypeList_viewer({
        data: this.dataObjList["typeList"].dsname,
        parentObjCls: main_objCls,
        formObject: this.formObject,
        callbackRendered: processOutputForTypeList,
        relatedDataSourceList: ["refGenomeList","refSeqIDList","annotList","seqList","infoList","boxList"],
        dataObjList: this.dataObjList
    });
    
    this.addAnnot_panel = new vjPanelView({
        data: ["dsVoid", this.dataObjList["typeList"].dsname],
        parentObjCls: main_objCls,
        formObject: this.formObject,
        iconSize : 24,
        rows: [
                { name: 'showAnnotation',align: 'right', type: 'button',isSubmitable: true, showTitle: true, hidden: false, order: 1, title: 'Add Single Track', description: 'show single annotation', url: addTrack},
                { name: 'showAnnotationList',align: 'right', type: 'button',isSubmitable: true, showTitle: true, hidden: false,isMultipleTrack: true,order: 1, title: 'Add Multiple Tracks', description: 'show multiple annotation', url: addTrack}
               ],
        relatedDataSource: {
            refGenomeList:{dsname: vjDS[this.dataObjList["refGenomeList"].dsname].name, url: vjDS[this.dataObjList["refGenomeList"].dsname].url},
            refSeqIDList: {dsname: vjDS[this.dataObjList["refSeqIDList"].dsname].name, url: vjDS[this.dataObjList["refSeqIDList"].dsname].url},
            annotList: {dsname: vjDS[this.dataObjList["annotList"].dsname].name, url: vjDS[this.dataObjList["annotList"].dsname].url},
            seqList: {dsname: vjDS[this.dataObjList["seqList"].dsname].name, url: vjDS[this.dataObjList["seqList"].dsname].url},
            infoList: {dsname: vjDS[this.dataObjList["infoList"].dsname].name, url: vjDS[this.dataObjList["infoList"].dsname].url},
            boxList: {dsname: vjDS[this.dataObjList["boxList"].dsname].name, url: vjDS[this.dataObjList["boxList"].dsname].url}
        }
    });
    
    vjDS[this.dataObjList["typeList"].dsname].myAssociatedViewers = [this.typeList_panel,this.typeList_viewer,this.addAnnot_panel];
    
    this.infoList_viewer = new vjConstructInfoList_viewer({
        data: this.dataObjList["infoList"].dsname,
        parentObjCls: main_objCls,
        formObject: this.formObject,
        maxTxtLen:1024,
        cols:[
              {name:"ID",wrap:true}
         ],
        relatedDataSourceList: ["refGenomeList","refSeqIDList","annotList","seqList","typeList","boxList"],
        dataObjList: this.dataObjList
    });
    
    vjDS[this.dataObjList["infoList"].dsname].myAssociatedViewers = [this.infoList_viewer];
    
    
    this.annotList_viewer.relatedViewer = {typeList: this.typeList_viewer};
    this.annotList_panel.relatedViewer = {typeList: this.typeList_viewer, annotList: this.annotList_viewer,infoList: this.infoList_viewer};
    this.typeList_viewer.relatedViewer = {annotList: this.annotList_viewer};
    this.typeList_panel.relatedViewer = {annotList: this.annotList_viewer, typeList: this.typeList_viewer, infoList: this.infoList_viewer};
    this.addAnnot_panel.relatedViewer = {annotList: this.annotList_viewer, typeList: this.typeList_viewer, infoList: this.infoList_viewer};
    this.infoList_viewer.relatedViewer = {annotList: this.annotList_viewer, typeList: this.typeList_viewer};
    
    
    
    var bp = new vjBumperLocator ({
        name: this.dataObjList["boxList"].dsname
        ,title: this.dataObjList["boxList"].title
        ,url:  'static:
        ,columnsToPick: this.columnsToPick
        ,collapse: false
   },1);
   
   var my_plot = new vjSVG_reference({
           options:{
               padding: {left:0.11,bottom:0.05,right:0.01}
        }
        ,title:"Please Select a Reference Sequence"
   });
   
   my_plot.add(bp);
   
   
   this.svgGeneViewer = new vjSVGView({
        plots: [my_plot],
        numberOfLayers: 10,
        computeSizeOfBox: false,
        parentObjCls: main_objCls,
        downloadLink: false,
        defaultEmptyText :"Please select a type",
        boxSize: 10
   });
   
   this.svgGene_panel = new vjPanelView({
           data: ['dsVoid', this.dataObjList["boxList"].dsname],
           parentObjCls: main_objCls,
           formObject: this.formObject,
           iconSize: 20,
           rows:[
               { name: 'refSeqID', prefix:"Sequence: ", align: 'left',isSubmitable: false ,showTitle: true, hidden: true, order: 1, title: '', size:10, description: 'Sequence'},
               { name: 'pos_start',prefix:"start position", align: 'right',isSubmitable: true ,type:"text",showTitle: true, hidden: false, order: 1, title: 'start position',size:10, description: 'start position'},
               { name: 'pos_end',prefix:"end position", align: 'right',isSubmitable: true ,type:"text",showTitle: true, hidden: false, order: 2, title: 'end position', size:15,description: 'end position'},
               { name: 'submit', align: 'right',showTitle: false, hidden: false, order: 4,title: 'submit', icon:"search",description: 'submit', url: zoomFunc}
                 ],
           isok: true,
           relatedDataSource: {
                   refGenomeList:{dsname: vjDS[this.dataObjList["refGenomeList"].dsname].name, url: vjDS[this.dataObjList["refGenomeList"].dsname].url},
                   refSeqIDList: {dsname: vjDS[this.dataObjList["refSeqIDList"].dsname].name, url: vjDS[this.dataObjList["refSeqIDList"].dsname].url},
                   annotList: {dsname: vjDS[this.dataObjList["annotList"].dsname].name, url: vjDS[this.dataObjList["annotList"].dsname].url},
                   seqList: {dsname: vjDS[this.dataObjList["seqList"].dsname].name, url: vjDS[this.dataObjList["seqList"].dsname].url},
                   typeList: {dsname: vjDS[this.dataObjList["typeList"].dsname].name, url: vjDS[this.dataObjList["typeList"].dsname].url},
                   boxList: {dsname: vjDS[this.dataObjList["boxList"].dsname].name, url: vjDS[this.dataObjList["boxList"].dsname].url}
               }
   });
   this.svgGene_panel.relatedViewer = {annotList: this.annotList_viewer, typeList: this.typeList_viewer, infoList: this.infoList_viewer,svgGeneViewer:this.svgGeneViewer};
   this.refSeqIDList_viewer.relatedViewer = {svg_panel: this.svgGene_panel, svgGeneViewer: this.svgGeneViewer};

   
   function zoomFunc (panel, ic, ir) {
       var new_start = panel.tree.findByName("pos_start").value;
       var new_end = panel.tree.findByName("pos_end").value;
       for (var i=2; i< panel.data.length; ++i) {
           var dsname = panel.data[i];
           var url = vjDS[dsname].url;
           url = urlExchangeParameter(url, "pos_start", new_start);
           url = urlExchangeParameter(url, "pos_end", new_end);
           vjDS[dsname].reload(url,true);
       }
       
       panel.relatedViewer.svgGeneViewer.plots[0].minValue = new_start;
       panel.relatedViewer.svgGeneViewer.plots[0].maxValue = new_end;
       panel.relatedViewer.svgGeneViewer.refresh();
   }
   this.typeList_panel.relatedViewer['svgGene_panel']=this.svgGene_panel;
   
   this.addDataSource = function(dataNameByHive,url,dataUserNaming){
       var el = {hiveDsname: dataNameByHive, url: url, userDsname: dataUserNaming};
       this.datasourceListForBoxList.push(el);
       vjDS.add('Loading ' + dataUserNaming, dataNameByHive, url);
   }
   
   this.removeDataSource = function(dataNameByHive){
       var len = this.datasourceListForBoxList.length;
       for (var i=0; i< len; ++i){
           if (this.datasourceListForBoxList[i].hiveDsname == dataNameByHive) {
               this.datasourceListForBoxList.splice(i,1);
               break;
           }
       }
       len = this.svgGene_panel.data.length;
       for (var i=0; i< len; ++i){
           if (this.svgGene_panel.data[i] == dataNameByHive) {
               this.svgGene_panel.data.splice(i,1);
               break;
           }
       }
       return 'success';
   }
   
   this.constructGBrowserViewer = function (dataNameByHive, url, titleOfUrl, graphTitle){
       if (titleOfUrl == undefined || titleOfUrl=='') title = 'loading viewers ...';
       if (graphTitle == undefined) graphTitle == '';
       
       var track_name = graphTitle; 
        var bp = new vjBumperLocator ({
            name: dataNameByHive,
            title: title,
            url: url,
            columnsToPick: this.columnsToPick,
            collapse: false
       },1);

       var my_plot = new vjSVG_GenomeBrowser({
               options:{
                graph:{
                    reference:{
                        type:"box"
                    }
                    ,annotation:{
                        color:randColor()
                    }
                }
                ,axis:{
                    maxValue:{
                            offset:0.05
                    }
                    ,hidden: true
                }
                ,source:{visible:true, title: graphTitle}
            }
            ,padding: {left:0.01,bottom:0.05,right:0}
            ,title:  ""
            ,selectCallback: displayInformation
            ,mouseoverCallback: displayTooltip
            ,mouseoutCallback: undisplayTooltip
            ,clickForCloseCallback: trackListSelectCallback
            ,computeSizeOfBox: true
            ,boxSize: 10
            ,parentObjCls: main_objCls
            ,dataObj:{trackName: track_name,hiveDSname: dataNameByHive}
       });
       my_plot.add(bp);

       
       var svgGeneViewer = new vjSVGView({
            plots: [my_plot],
            numberOfLayers: 10,
            computeSizeOfBox: false,
            parentObjCls: main_objCls,
            noSize:true,
            defaultEmptyText :"Please select a type",
            track_name: track_name
       });
       
       my_plot.dataObj['gbViewerObjCls']=svgGeneViewer.objCls;
       return svgGeneViewer;
   }
   
   this.addGBrowserViewer = function(hiveDSname,trackName) {
       
       var url = vjDS[hiveDSname].url;
       var gbv = this.constructGBrowserViewer(hiveDSname,url,'',trackName);
       var a = this.svgGene_panel;
        a.tab.add(gbv);    
        a.tab.parent.render();
        a.tab.parent.load();
        return gbv.objCls;
   }
   
   this.addTrack = function(trackName,url){
     if (trackName == undefined || !trackName) trackName = 'track ' + (this.addedTrackCnt);
       var hiveDSname = this.dataBoxListDsname + "_" + trackName.replace(/\s/g,'_');
        this.addDataSource(hiveDSname,url,trackName);
        var gbv = this.constructGBrowserViewer(hiveDSname,url,'',trackName);
        
       var dataViews = $('#'+this.tabId).dataviews('instance');
           dataViews.add({instance:gbv, size: '150', allowClose: true});
        this.svgGene_panel.data.push(hiveDSname);
       this.addedTrackCnt +=1;
       this.existingTrackCnt +=1;
   }
   
   this.removeTrack = function(trackName, hiveDSname, gbViewerObjCls){
       this.removeDataSource(hiveDSname);
       this.removeGBrowserViewer(trackName, gbViewerObjCls);
       this.removedTrackCnt +=1;
       this.existingTrackCnt-=1;
       if (vjDS[hiveDSname]!= undefined) delete vjDS[hiveDSname];
       this.svgGene_panel.tab.parent.render();
       this.svgGene_panel.tab.parent.load();
   }
   
   this.removeGBrowserViewer = function(trackName,gbViewerObjCls){
       for (var iv=0; iv < this.svgGene_panel.tab.viewers.length; ++iv){
           var vv = this.svgGene_panel.tab.viewers[iv];
           if (vv.objCls == gbViewerObjCls){
               this.svgGene_panel.tab.viewers.splice(iv,1);
               break;
           }
       }
       var idx = this.gbChildren.indexOf(gbViewerObjCls);
       if (idx !=-1) this.gbChildren.splice(idx,1);
   }
   
   function displayInformation(viewer, htmlElement){
       var idType_id = htmlElement.getAttribute('idType-id').replace(/; /g,' ');
       idType_id = idType_id.split(';');
       var mainObj = vjObj[viewer.parentObjCls];
       var infoDS = mainObj.infoList_viewer.data[0];
       var start = htmlElement.getAttribute('start');
       var end = htmlElement.getAttribute('end');
       var url = 'static:
       url += 'Start,' + start + "\n" + 'End,' + end + "\n"; 
       
       for (var i=0; i<idType_id.length; ++i){
           var info = idType_id[i].split(':');
           url+= '' + info[0] + ',\"'+ info[1] + '\"\n';
       }
       vjDS[infoDS].reload(url,true);
   }
   
  
   function displayTooltip(viewer, htmlElement){
       var mainTabId = vjObj[viewer.parentObjCls].tabId;
       if (gObject("GBtooltip")==undefined) {
           var tooltip = d3.select("#"+mainTabId).append("div")
             .attr("id", "GBtooltip")
             .attr('style','position: fixed;text-align: left;width: 128px;height: 60px;background: lightyellow;color: black;border: 1px;opacity: 0;border-radius: 8px;font-size: 12px;padding: 10px;font-weight: 600;border-color: black;border-style: solid;');
       }
       var tooltip = d3.select("#GBtooltip");
       tooltip.style("left", (event.clientX ) + "px")
       .style("top", (event.clientY ) + "px")
       .style("fill-opacity", .5)
                .style("stroke-opacity", 1);
            tooltip.transition().style("opacity", .9);
        tooltip.style("z-index",999);    
        tooltip.text("Start: " + htmlElement.getAttribute('start') + '|End: ' + htmlElement.getAttribute('end') + ';' + htmlElement.getAttribute('idType-id'));
        tooltip.each(function(){
            var outText = "", lineTotal=1;
            outText += '';
            var textContent = this.innerText;
            if (textContent.indexOf('|')==-1) return;
            var firstPart = textContent.split('|')[0];
            outText += firstPart + "</br>"; ++lineTotal;
            var secondPart = textContent.split('|')[1];
            var split = secondPart.split(';');
            var curLength = 0, longest=18, maxWidth = 100;
            for (var i=0; i< split.length; ++i){
                var precomputeLength = curLength+split[i].length;
                if ((precomputeLength-longest) >0) {maxWidth += (precomputeLength-longest)*3}
                if ( i && (precomputeLength )> 18 ) {outText += '<br>';++lineTotal; curLength=0; longest = precomputeLength;}
                outText+= "" + split[i] + " ";
                curLength+=split[i].length +1;
            }
            var newHeight = lineTotal * 20;
            this.style.height = ""+newHeight+'px';
            this.style.width = ""+(maxWidth*1.5)+'px';
            this.innerHTML = outText;
            
        });
        
   }
   function undisplayTooltip(viewer, htmlElement){
       var tooltip = d3.select("#GBtooltip");
       tooltip.transition()
       .style("fill-opacity", 0)
       .style("stroke-opacity", 0);
       tooltip.transition().style("opacity", 0);
       
       tooltip.style("left", "-1000px")
       .style("top","-1000px");
   }
  
   function refSeqIDListSelectCallback(viewer,node,ir,ic) {
       console.log("select");
       viewer.relatedViewer.svgGeneViewer.plots[0].title = node.id;
       viewer.relatedViewer.svgGeneViewer.plots[0].maxValue = node.len;
       viewer.relatedViewer.svgGeneViewer.refresh();
       
   }
   
   function annotListSelectCallback (viewer,node,ir,ic){
        var nodeIdSelected = [];
        for (var i=0; i<viewer.selectedNodes.length; ++i){
            nodeIdSelected.push(viewer.selectedNodes[i].id);
        }
        
        var url = "http:
        vjDS[viewer.relatedDataSource["typeList"].dsname].reload(url,true);
        url += "&recordTypes=seqID&cnt=-1";
    }
   
    function trackListSelectCallback(viewer,node,ir,ic){
        var main = vjObj[viewer.parentObjCls];
        main.removeTrack(viewer.dataObj.trackName,viewer.dataObj.hiveDSname ,viewer.dataObj.gbViewerObjCls);
    }
    
    function processOutputForTypeList(viewer, node, ir, ic){
        
    }
    function addTrack (viewer, node,ir, ic){
        var annotListVV = viewer.relatedViewer["annotList"];
        var idArr = []; 
        var trackTitle = "";
        for (var i=0; i<annotListVV.selectedNodes.length; ++i){
            var id_ = annotListVV.selectedNodes[i].id;
            if (idArr.indexOf(id_)!=-1) continue;
            idArr.push(id_);
            trackTitle = annotListVV.selectedNodes[i].name;
        }
        var typeArr = [];
        for (var j=0; j<viewer.relatedViewer["typeList"].selectedNodes.length; ++j){
            var checkedType = viewer.relatedViewer["typeList"].selectedNodes[j].types;
            if (typeArr.indexOf(checkedType)!=-1) continue;
            typeArr.push(checkedType);
        }
        
        var main = vjObj[viewer.parentObjCls];
        var new_start = -1;
        var new_end = -1;
        var rangeToAdd = undefined;
        if (main.svgGene_panel) {
            var svgGene_panel = main.svgGene_panel;
            var start = svgGene_panel.tree.findByName("pos_start").value;
            new_start = start ?  start : new_start;
            var end = svgGene_panel.tree.findByName("pos_end").value;
            new_end = end ?  end : new_end;
            if (new_start!=-1 && new_end !=-1) {
                rangeToAdd += "&pos_start=" + new_start + "&pos_end=" + new_end;
            } 
        }
        if (node.isMultipleTrack) {
            for (var it=0; it<typeArr.length; ++it) {
                var cur_title = trackTitle;
                var cur_type = typeArr[it];
                var url = "http:
                if (rangeToAdd!=undefined) {
                    url += rangeToAdd;
                } 
                cur_title += " [" + cur_type + "]";
                main.addTrack(cur_title, url);
                vjDS[viewer.data[1]].reload(null,true);
            }
        }
        else {
            
            var url = "http:
            if (rangeToAdd!=undefined) {
                url += rangeToAdd;
            }
            trackTitle+= " [" + typeArr.join("|") + "]";
            main.addTrack(trackTitle, url);
            vjDS[viewer.data[1]].reload(null,true);
        }
    }

   return [this.refGenomeList_panel,this.refGenomeList_viewer,
           this.refSeqIDList_panel,this.refSeqIDList_viewer, 
           this.annotList_panel, this.annotList_viewer, 
           this.typeList_panel,this.typeList_viewer, 
           this.svgGene_panel, this.svgGeneViewer,
           this.infoList_viewer, this.addAnnot_panel];
}

function extractMinMaxFromUrl(url,keywordForMin,keywordForMax){
    var minWord = 'start';
    var maxWord = 'end';
    if (keywordForMin) minWord = keywordForMin;
    if (keywordForMax) maxWord = keywordForMax;
    return {min: docLocValue(minWord,-1,url), max: docLocValue(maxWord,-1,url)}    ;
    
}

function addRelatedDS(listOfDataSource, dataObjList) {
    var relatedDataSource = {};
    var len = listOfDataSource.length;
    for (var i=0; i< len; ++i) 
    {
        var dsname = listOfDataSource[i];
        relatedDataSource[dsname]={'dsname': vjDS[dataObjList[dsname].dsname].name,'url': vjDS[dataObjList[dsname].dsname].url};
    }
    return relatedDataSource;
}

function vjGBInitValue(viewer){
    this.data=[viewer.data];
    this.parentObjCls=viewer.parentObjCls.main_objCls;
    this.maxTxtLen=25;
    this.bgColors=[ '#f2f2f2', '#ffffff' ];
}

function vjConstructAnnotList_viewer(viewer){
    vjGBInitValue.call(this,viewer);
    
    this.cols=[
               {name:new RegExp(/.*/), hidden: true},
               {name:"id", hidden: false, order:1, align:"middle"},
               {name:"^name", hidden: false, order:2},
               {name:"created", hidden: false, order: 3, type:"datetime",align:"middle"}
    ];
    
    this.defaultEmptyText = 'no information to show';
    this.selectCallback= viewer.selectCallback;
    this.callbackRendered=     function (viewer,node,ir,ic){
        viewer.mimicClickCell(0,1);
    };
    this.relatedDataSource = addRelatedDS(viewer.relatedDataSourceList, viewer.dataObjList);     
    vjTableView.call(this,viewer);
    
}

function vjConstructRefGenomeList_viewer(viewer){
    vjGBInitValue.call(this,viewer);
    
    this.cols= [
           {name:new RegExp(/.*/), hidden: true},
           {name:"id", hidden: false, order:1, align:"middle"},
           {name:"^name", hidden: false, order:2},
           {name:"created", hidden: false, order: 3, type:"datetime",align:"middle"}
    ];
    this.defaultEmptyText = 'no information to show';
    this.selectCallback=function (viewer,node,ir,ic){
           var url = "http:
           vjDS[viewer.relatedDataSource["refSeqIDList"].dsname].reload(url,true);
    };
    this.callbackRendered= function (viewer,node,ir,ic){
        viewer.mimicClickCell(0,1);
    };
    this.relatedDataSource = addRelatedDS(viewer.relatedDataSourceList, viewer.dataObjList);
    vjTableView.call(this,viewer);
}

function vjConstructTypeList_viewer(viewer){
    vjGBInitValue.call(this,viewer);
    
    this.formObject=viewer.formObject;
    this.defaultEmptyText = 'no information to show';
    this.cols=[
          {name:"#",hidden:false,align:"middle"},
          {name:"types",align:"left"}
    ];
    this.geometry= {
        width:100
    };
    this.multiSelect = true;
    this.callbackRendered=viewer.callbackRendered,
    this.relatedDataSource = addRelatedDS(viewer.relatedDataSourceList, viewer.dataObjList);    
    vjTableView.call(this,viewer);
}


function vjConstructRefSeqIDList_viewer(viewer) {
    vjGBInitValue.call(this,viewer);
    
    this.defaultEmptyText = 'no information to show';
    this.cols=[
          {name:"seq",hidden:true},
          {name:"rpt",hidden:true},
          {name:"#",order:1},
          {name:"^id",order:2,title:"Identifier"},
          {name:"^len",order:3,title:"Length"}
    ];
    
    this.selectCallback=viewer.selectCallback,
    this.callbackRendered=function (viewer,node,ir,ic){
    };
    this.relatedDataSource =addRelatedDS(viewer.relatedDataSourceList, viewer.dataObjList);    
    vjTableView.call(this,viewer);
}



function vjConstructInfoList_viewer(viewer) {
    vjGBInitValue.call(this,viewer);
    
    this.defaultEmptyText='Please click on annotation box',
    this.formObject=viewer.formObject;
    this.geometry= {
        width:100
    },
    this.relatedDataSource = addRelatedDS(viewer.relatedDataSourceList, viewer.dataObjList);    
    vjTableView.call(this,viewer);
}



