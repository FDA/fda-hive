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

function vjSVG_Plot(source)
{
    vjSVG_primitive.call(this,source);

    this.collection=new Array();

    this.add=function(series )
    {
        this.collection.push(series);
    };

    this.minmax=function()
    {
        this.min=this.collection[0].min;
        this.max=this.collection[0].max;
        
        if (!this.min || !this.max)
        {
            this.min = {};
            this.max = {};
        }

        for( var ic=0; ic<this.collection.length; ++ic ){
            if(this.collection[ic].isNonTrivialData)continue;
            if( this.collection[ic].min && this.min.x > this.collection[ic].min.x )
                this.min.x = this.collection[ic].min.x;
            if( this.collection[ic].max && this.max.x < this.collection[ic].max.x )
                this.max.x = this.collection[ic].max.x;

            if( this.collection[ic].min && this.min.y > this.collection[ic].min.y )
                this.min.y = this.collection[ic].min.y;
            if( this.collection[ic].max && this.max.y < this.collection[ic].max.y )
                this.max.y = this.collection[ic].max.y;
        }
        //alerJ("max Constrcut minmax ", this.max)
    };
    
    this.getPlotScale = function() {
        return { x:parseFloat(1./(this.max.x-this.min.x)), y:parseFloat(1./(this.max.y-this.min.y)), z:parseFloat(1./(this.max.z-this.min.z)) };
    };
    //alert(this.selectCallback)
    this.construct=function(axis,chartarea,scene)
    {

        this.minmax();
        this.scaleCrd=this.getPlotScale();

        if(scene)this.ownerSVG = scene.svg ;

        var beforeAxis = { "column": true };

        for (var ic=0; ic<this.collection.length; ic++) {
            var serie = this.collection[ic];
            if (!beforeAxis[serie.type] || serie.isNonTrivialData)
                continue;

            if (serie.has_nontrivial_data())
                this.constructorFunction[serie.type].call(this, serie);
        }

        if(this.legend && this.constructLegend) {
            for( var ic=0; ic<this.collection.length; ++ic ){
                var serie = this.collection[ic];
                this.constructLegend(serie);
            }
            scene.scaleMatrix({x:0.8,y:1,z:1});//TODO: set by an input instead of hardcoded 0.9;
        }
        
        if (axis) {
            axis.x.max = this.max.x ;
            axis.x.min = this.min.x ;

            axis.y.max = this.max.y;
            axis.y.min = this.min.y;
            if (this.collection[0].tblArr && this.collection[0].tblArr.rows !== undefined && this.collection[0].tblArr.rows.length) {
                this.drawAxis(axis,chartarea);
            }
        }
        

        for( var ic=0; ic<this.collection.length; ++ic ){

            //Axis go first to stay in the background
            var serie = this.collection[ic];
            if (beforeAxis[serie.type])
                continue;

            if (serie.has_nontrivial_data()){

                if(this.constructorFunction[serie.type])this.constructorFunction[serie.type].call(this, serie);
            }
        }
    };

    this.constructorFunction=new Object();
    this.constructorFunction['scatter']=function(serie)
    {
        if(!serie.symbolType)serie.symbolType='rectangle';
        if(!serie.symbolSize)serie.symbolSize=0.005;

        for( var ip=0;ip<serie.tblArr.rows.length; ++ip)
        {
            var node=serie.tblArr.rows[ip];
            if (!node.z) node.z=0;

            var crdNode={x:(node.x-this.min.x)*this.scaleCrd.x,y:(node.y-this.min.y)*this.scaleCrd.y,z:(node.z-this.min.z)*this.scaleCrd.z};

            var point=new vjSVG_symbol(  {definition:serie.symbolType, crd:crdNode , size:serie.symbolSize}) ;

            this.children.push(point);
        }
    };

    this.constructorFunction['anotBox']=function(serie)
    {
        //var color = ["red","yellow","blue","green","cyan"];
        //alert("plot" + this.max.y)
        var familyName = "anotBox_" + Math.random();
        serie['coordinates'] = serie.tblArr.rows;

        if (!serie.color) {
            serie.color = "darkred";
        }
        if (!serie.boxSize) serie.boxSize = 0.25;
        var boxList = new Array();
        if (serie.forceScaleY) {
            this.scaleCrd.y = 0.35;
        }
        for( var ip=0; ip<serie.coordinates.length; ++ip)
        {
            var row=serie.coordinates[ip];
            //if (serie.forceScaleY) row.y = 0.25;
            var node = new Object();
            if (!node.z) row.z=0;
            node.start = row[serie.columnDefinition.start];
            node.end = row[serie.columnDefinition.end];
            if (!row.height) row.height = 0.45 * serie.boxSize * this.scaleCrd.y;
            if (!row.step) row.step = 0.5;

            var crdNodeStart = new Object();
            var box = new Object();
            var showText = "Id: " + row[serie.columnDefinition.label] + "\n" + "rangeStart: " + node.start +"\n"+ "rangeEnd: " + node.end + "\n";
            if (serie.labels){
                showText = "";
                for (var ll=0;ll<serie.labels.length;ll++){
                    showText = showText + serie.labels[ll] + ": " + row[serie.labels[ll]] + "\n";
                }
            }
            //alert(this.scaleCrd.x)
                crdNodeStart = { x: (node.start-this.min.x)*this.scaleCrd.x,
                                   y:  row.step* row.y *this.scaleCrd.y,
                                 z: (node.z-this.min.z) *this.scaleCrd.z
                                };
                var width = (node.end - node.start)*this.scaleCrd.x; //alert("box width " + width)
                box = new vjSVG_box(  {crd:crdNodeStart, width:width, height:row.height}) ;
               // alert(box)
                box.brush={"fill":serie.color};
                box.pen={ "stroke-width" : 0.6};
                box.svgID = showText;
                box.title = showText;
                box.attribute = {"name":row[serie.columnDefinition.label],"subId":showText, "family": familyName, "start": node.start, "end": node.end, "Xscale": this.scaleCrd.x};
                if (serie.tag) {
                    for (var tg=0;tg<serie.tag.length;tg++){
                        box.attribute[serie.tag[tg]] = row[serie.tag[tg]];
                    }
                }

                box.cols = "";
                for (var ii=0;ii<serie.tblArr.hdr.length;ii++) {
                    box.cols = box.cols + serie.tblArr.hdr[ii].name;
                    if (ii<serie.tblArr.hdr.length-1) box.cols = box.cols + ",";
                    box[serie.tblArr.hdr[ii].name] = serie.tblArr.rows[ip][serie.tblArr.hdr[ii].name];
                }
                box["min"] = this.min.x;
                box["max"] = this.max.x;
                box.handler = {
                    "onmouseover":"function:vjObjFunc('mouseOverShowId','" + this.objID + "')",
                    "onmouseout":"function:vjObjFunc('mouseOutShowId','" + this.objID + "')",
                    "onclick":"function:vjObjFunc('clickForCallingBack','" + this.objID + "')"
                };

                if (ip==0)boxList.push(box);
                if (serie.coordinates.length==1){
                    this.children.push(box);
                }
                boxList.push(box);

                if(ip==serie.coordinates.length-1) {
                    for (var i=0;i<boxList.length;i++){
                        this.children.push(boxList[i]);
                    }
                    break;
                }
            }
    };


    this.constructorFunction['line']=function(serie,parent)
    {
        var myGrp = new vjSVG_group();
        myGrp.svgID = serie.name;
        if (serie.color === undefined) serie.color = "red";
        if (serie.width === undefined) serie.width = 1;
        if(!serie.coordinates){
            serie.coordinates=serie.tblArr.rows;
            for (var iii=0;iii<serie.coordinates.length;iii++){
                //alerJ("serie",serie.coordinates[iii]);
                serie.coordinates[iii].x=parseFloat(serie.coordinates[iii].x-this.min.x)*this.scaleCrd.x;
                serie.coordinates[iii].y=parseFloat(serie.coordinates[iii].y-this.min.y)*this.scaleCrd.y;
                if (!serie.coordinates[iii].z) serie.coordinates[iii].z=0;
                serie.coordinates[iii].z=parseFloat(serie.coordinates[iii].z=this.min.z)*this.scaleCrd.z;
                var lineP = new vjSVG_line({crd1:{x:serie.coordinates[iii].x,y:serie.coordinates[iii].y-0.05,z:0},crd2:{x:serie.coordinates[iii].x,y:serie.coordinates[iii].y+0.05,z:0},lineCmd:"L"});
                //lineP.svgID = serie.name;
                lineP.pen = {"stroke-width":1.5,"stroke-opacity":0,"stroke":serie.color};
                lineP.cols="";
                var attrObj = new Object();
                for (var ii=0;ii<serie.tblArr.hdr.length;ii++) {
                    lineP.cols = lineP.cols + serie.tblArr.hdr[ii].name;
                    if (ii<serie.tblArr.hdr.length-1) lineP.cols = lineP.cols + ",";
                    lineP[serie.tblArr.hdr[ii].name] = serie.tblArr.rows[iii][serie.tblArr.hdr[ii].name];
                }
                attrObj["valueX"] = serie.columnDefinition.x +":"+ serie.tblArr.rows[iii][serie.columnDefinition.x];
                attrObj["valueY"] = serie.columnDefinition.y +":"+ serie.tblArr.rows[iii][serie.columnDefinition.y];
                lineP.handler = {
                        "onmouseover": "function:vjObjFunc('mouseOverLine','" + this.objID + "')",
                        "onmouseout": "function:vjObjFunc('mouseOutLine','" + this.objID + "')"
                        //"onclick": "function:vjObjFunc('clickForCallingBack','" + this.objID + "')"
                };
                attrObj["family"]=""+serie.name;
                lineP.attribute = attrObj;
                //lineP.brush = {"fill":"blue","fill-opacity":0};
                myGrp.children.push(lineP);
                //this.children.push(lineP);
            }
        }

        var myTraj=new vjSVG_trajectory ( serie ) ;

        myTraj.pen={"stroke":serie.color, "stroke-width" : serie.width, "stroke-opacity" : 1};
        myTraj.attribute = {"family":serie.name};
        myTraj.svgID = serie.name;

        myTraj.handler = {
                "onclick": "function:vjObjFunc('clickForCallingBack','"+ this.objID + "')"
         };
        if (parent !== undefined && parent.brush){
            myTraj.svgID=myTraj.objID;
            myTraj.parent=parent;
            myTraj.pen={"stroke":parent.brush.fill, "stroke-width" : 2, "stroke-opacity" : 0};
            myTraj.brush={"fill":parent.brush.fill, "fill-opacity" : 0};//, "stroke-opacity" : 0};
            myTraj.handler = {
                    'onmouseover' : "function:vjObjFunc('testCallback','" + this.objID + "')"//,//'vjObjEvent(\"testCallback\", \"'+ this.objCls + '\");' //,'onmouseout' : 'testCallbackoFF(this)'
    //                'onmouseout': 'testCallbackoFF,'+this.objCls+''
            };
            for(var ii in parent.handler){
                var isThere=false;
                for(jj in myTraj.handler){
                    if(ii==jj)isThere=true;
                }
                if(!isThere)
                    myTraj.handler[ii]=parent.handler[ii];
            }
            parent.children.push( myTraj );
        }
        else {
            //myTraj.brush={"fill-opacity":0};
            myGrp.children.push(myTraj);
            //this.children.push(myTraj);
            this.children.push(myGrp);
        }
    };

    this.constructorFunction['column']=function(serie)
    {
        //return ;
        if (serie.hidden && serie.hidden===true)
            return;
        //var familyName = "column_" + Math.random();
        if (!serie.byX) serie.byX =false;
        if (!serie.makeGroup) serie.makeGroup = false;
        var myGrp = "";
        if (serie.makeGroup===true) {
            myGrp = new vjSVG_group();
            myGrp.svgID = serie.name ;
        }
        serie.columnDefinition = verarr(serie.columnDefinition);
        //alerJ ("column ",  serie.columnDefinition[0])
        var numberOfSerie = serie.columnDefinition.length;
        var numberOfElements = serie.tblArr.rows.length;
        var lengthOfOneUnit = numberOfElements ? 1/numberOfElements : 1;
        var stroke = numberOfSerie > 1 ? true : false;
        if (serie.byX === true) {
            lengthOfOneUnit = 1/(this.max.x-this.min.x);
            stroke = true;
        }
        var lengthOfOneSubUnit = lengthOfOneUnit;
        if (numberOfSerie > 1)
            lengthOfOneSubUnit *= 7 / (10 * numberOfSerie);

        var default_color = new Array("blue","darkred","green","orange","purple","pink","brown","black","cyan","magenta");
        var color = verarr(serie.color);
        for (var i=color.length; i<numberOfSerie && i<default_color.length; i++) {
            color.push(default_color[i]);
        }
        var strokeColor = [];
        for (var i=0; i<color.length; i++) {
            strokeColor.push(darkenColor(color[i]));
        }
        //if (serie.valueMax){
        //    if (serie.valueMax.x) {
        //        if (numberOfElements<this.max.x/100){
                    //lengthOfOneUnit = 1/(this.max.x-this.min.x);
                    //lengthOfOneUnit = (Math.round(this.max.x/this.max.x)/this.max.x);
                    //lengthOfOneSubUnit = Math.round(lengthOfOneUnit/numberOfSerie)/lengthOfOneUnit;
        //        }
        //    }
        //}
        var elementToPush = new Array();
        for( var ip=0;ip<serie.tblArr.rows.length; ++ip)
        {
            var element=serie.tblArr.rows[ip];
            //alerJ("elemeent",serie.tblArr.hdr[0]);
            if (!element.z) element.z=0;

            //var lengthOfOneSubUnit = (Math.round(lengthOfOneUnit/numberOfElements)/100);

            //var lengthOfOneSubSubUnit = Math.round(10/numberOfSerie)*lengthOfOneSubUnit;
            //alert("Sub " + lengthOfOneSubUnit + " One " + lengthOfOneUnit + "subsub  " + lengthOfOneSubSubUnit)
            var startPoint = (ip+0.15)*lengthOfOneUnit;
            //var startPre = 0;
            //alert("start Poi " + startPoint )
            for (var iSerie=0; iSerie<numberOfSerie; iSerie++){
                //alert("ele " + element[serie.columnDefinition[iSerie].y])
                //var startPoint = (element[serie.columnDefinition[iSerie].x]+0.15)*lengthOfOneUnit;
                //if (serie.valueMax){
                //    if (serie.valueMax.x) {
                //        if (numberOfElements<this.max.x/100){
                //            startPoint = (element[serie.columnDefinition[iSerie].x]*this.scaleCrd.x);
                //        }
                //    }
                //}
                //if (!serie.valueMax){
                if (serie.byX == true) startPoint = ((element[serie.columnDefinition[iSerie].x]-this.min.x)*this.scaleCrd.x);
                //alert("Unit " + lengthOfOneUnit + " subUni " + lengthOfOneSubUnit + " StartPoint " + startPoint)

                var point1 = {x:startPoint+(iSerie*lengthOfOneSubUnit)+(0.2*lengthOfOneSubUnit),y:0,z:0};
                var point2 = {x:startPoint+(iSerie*lengthOfOneSubUnit)+(0.2*lengthOfOneSubUnit),y:element[serie.columnDefinition[iSerie].y]*this.scaleCrd.y,z:0};
                var point3 = {x:startPoint+(iSerie*lengthOfOneSubUnit)+(0.8*lengthOfOneSubUnit),y:element[serie.columnDefinition[iSerie].y]*this.scaleCrd.y,z:0};
                var point4 = {x:startPoint+(iSerie*lengthOfOneSubUnit)+(0.8*lengthOfOneSubUnit),y:0,z:0};

                var crdList = new Array();
                crdList.push(point1,point2,point3,point4);

                var myColumn = new vjSVG_trajectory({coordinates:crdList,closed:1,lineCmd:"L"});//this.coordinates,this.closed,this.lineCmd,this.objID);

                myColumn.brush={'fill':color[iSerie], 'stroke':strokeColor[iSerie], 'stroke-opacity': stroke ? 1 : 0};
                if (!element[serie.columnDefinition[iSerie].label]) element[serie.columnDefinition[iSerie].label] = "ID";
                var showText = serie.columnDefinition[iSerie].y + ": " + element[serie.columnDefinition[iSerie].y] + "\n" + serie.columnDefinition[iSerie].x + ": " + element[serie.columnDefinition[iSerie].x];
                if (serie.labels){
                    showText = "";
                    for (var ll=0;ll<serie.labels.length;ll++){
                        showText = showText + serie.labels[ll] + ": " + element[serie.labels[ll]] + "\n";
                    }
                    myColumn.svgID = serie.labels[0] + ":" + element[serie.labels[0]];
                }

                myColumn.title = showText;
                if (serie.labels === undefined) serie.labels = [serie.columnDefinition[iSerie].y];
                myColumn.attribute = {"name":element[serie.labels[0]],"subId":showText, "valueX":element[serie.columnDefinition[iSerie].x], "family":""+serie.name+"_"+iSerie};
                if (serie.tag) {
                    for (var tg=0;tg<serie.tag.length;tg++){
                        myColumn.attribute[serie.tag[tg]] = element[serie.tag[tg]];
                    }
                }
                myColumn.cols = "";
                for (var ii=0;ii<serie.tblArr.hdr.length;ii++) {
                    myColumn.cols = myColumn.cols + serie.tblArr.hdr[ii].name;
                    if (ii<serie.tblArr.hdr.length-1) myColumn.cols = myColumn.cols + ",";
                    myColumn[serie.tblArr.hdr[ii].name] = serie.tblArr.rows[ip][serie.tblArr.hdr[ii].name];
                }
                myColumn["min"] = this.min.x;
                myColumn["max"] = this.max.x;
                //alerJ("", serie.tblArr.hdr[0])
                myColumn.handler = {
                        'onmouseover' : "function:vjObjFunc('mouseOverShowId','" + this.objID + "')",
                        'onmouseout': "function:vjObjFunc('mouseOutShowId','" + this.objID + "')",
                        "onclick":"function:vjObjFunc('clickForCallingBack','" + this.objID + "')"
                };

                 //this.children.push(myColumn);

                elementToPush.push(myColumn);
                 if (serie.addText!==undefined){
                     var textText = new vjSVG_text({
                         crd:{x:point3.x,y:point3.y},
                         text:element[serie.addText.name],
                         angle:serie.addText.angle
                     });
                     textText.svgID = "text_" + element[serie.addText.name];
                     textText.font={
                                "font-size": serie.addText.size ? serie.addText.size : "11",
                                "writing-mode" : (serie.addText.isVertical ? "tb" : null),
                                "glyph-orientation-vertical" : (serie.addText.isVertical ? "0" : null)
                            };
                     textText.brush={"fill-opacity":1};
                     textText.attribute = {"name":element[serie.labels[0]], "family":""+serie.name+"_"+iSerie};
                     elementToPush.push(textText);
                 }
            }

        }

        for (var ii=0;ii<elementToPush.length;ii++){
            if (serie.makeGroup===true && myGrp) {
                myGrp.children.push(elementToPush[ii]);
            }
            else this.children.push(elementToPush[ii]);
        }
        if (serie.makeGroup === true && myGrp) this.children.push(myGrp);

    };

    this.constructorFunction['flow']=function(serie,parent)
    {
        serie.coordinates=serie.tblArr.rows;
        if(!serie.mode)serie.mode="relative";
        if(!serie.shift)serie.shift=0;
        if(!serie.center)serie.center=0.2;
        var length=serie.tblArr.rows.length;
        for( var ip=0;ip<length; ++ip)
        {
            revI=2*length-ip-1;

            serie.coordinates[ip].x=(serie.coordinates[ip].x-this.min.x)*this.scaleCrd.x;
            var revx=serie.coordinates[ip].x;
            var revy=0,y=0;
            if(serie.mode=="absolute"){
                y=(serie.coordinates[ip].y+serie.shift)*this.scaleCrd.y;
                revy=serie.shift;
            }
            if(serie.mode=="relative"){
                y=(this.max.y*serie.center+(1-serie.center)*serie.coordinates[ip].y+serie.shift)*this.scaleCrd.y;
                revy=(this.max.y*serie.center-(serie.center*serie.coordinates[ip].y)+serie.shift)*this.scaleCrd.y;
            }
            if(serie.mode=="placed"){
                y=(serie.coordinates[ip].y+serie.shift)*this.scaleCrd.y;
                revy=(serie.coordinates[ip].y-serie.coordinates[ip].w+serie.shift)*this.scaleCrd.y;
            }
            serie.coordinates[revI]={x:revx,y:revy};
            serie.coordinates[ip].y=y;
            if (!serie.coordinates[ip].z) serie.coordinates[ip].z=0;
            serie.coordinates[ip].z=(serie.coordinates[ip].z=this.min.z)*this.scaleCrd.z;

        }
        var myTraj=new vjSVG_trajectory ( serie ) ;
        this.children.push( myTraj );
    };

    this.constructorFunction['area']=function(serie,parent){
        serie.mode="absolute";
        this.constructorFunction['flow'].call(this,serie);
    };

    if(typeof(vjSVG_Sankey)!="undefined")
        this.constructorFunction['sankey']=vjSVG_Sankey;


    this.drawAxis=function(axis,chartarea){

        if (axis) {
            var x=axis.x;
                x.labelSize='';
                x.titlePosition='';
                x.titleFontSize='';
                x.min=this.min.x;
                if (!x.max) x.max=this.max.x;
                else this.max.x=x.max;
                x.actualSize=chartarea.width;

            var y=axis.y;
                y.labelSize="",
                y.titlePosition='';
                y.titleFontSize='';
                y.min=this.min.y;
                if (!y.max) y.max=this.max.y;
                else this.max.y=y.max;
                y.actualSize=chartarea.height;


            var axisSystem=new vjSVG_CartesianAxis({x:x,y:y});
            this.max.x = axisSystem.axisX.max;
            this.max.y = axisSystem.axisY.max;
            this.children.push(axisSystem);
        }
    };
    this.savedBrushAttrs = [
        "stroke", "stroke-width", "stroke-opacity", "stroke-linejoin", "fill"
    ];
    this.saveBrush = function(obj) {
        if (obj.savedBrush) return;
        obj.savedBrush = {};
        for (var i=0; i<this.savedBrushAttrs.length; i++) {
            var attr = this.savedBrushAttrs[i];
            obj.savedBrush[attr] = obj.getAttribute(attr);
        }
    };
    this.restoreBrush = function(obj) {
        if (!obj.savedBrush) return;
        for (var i=0; i<this.savedBrushAttrs.length; i++) {
            var attr = this.savedBrushAttrs[i];
            obj.setAttribute(attr, obj.savedBrush[attr]);
        }
        delete obj.savedBrush;
    };
    this.mouseOverShowId = function(ir,eventObjID,evt){
        var parentObj = eventObjID.parentNode.id;

        //alert("parent Ofj " +  parentObj.split(" ")[0])
        //alert("Over "+  eventObjID.nameOfElement)
        this.saveBrush(eventObjID);
        eventObjID.style["stroke-width"]=2.5;
        eventObjID.style["stroke-opacity"]=1;
        eventObjID.style["stroke-linejoin"]="round";
        var nameValue = eventObjID.getAttributeNode("name").nodeValue;
        var listBrothers = document.getElementsByName(nameValue);
        for (var ii=0;ii<listBrothers.length;ii++){
            //alerJ(" ii " + ii + "length " + listBrothers.length,listBrothers[ii])

            var parentId = listBrothers[ii].parentNode.id;
            //if (parent != parentObj){
            if (parentId !=parentObj && parentId.split(" ")[0] === parentObj.split(" ")[0]){
                //alert("parent " + parentId+ " parentObj " + parentObj )
                this.saveBrush(listBrothers[ii]);
                listBrothers[ii].style["stroke-width"]=2.5;
                listBrothers[ii].style["stroke-linejoin"]="round";
                listBrothers[ii].style["fill"]="blue";
                if (listBrothers[ii].getAttribute("d")!== undefined){
                    var toolTipX = parseFloat(listBrothers[ii].getAttribute("d").split(" ")[2].split(",")[0].split("L")[1]);
                    var toolTipY = parseFloat(listBrothers[ii].getAttribute("d").split(" ")[2].split(",")[1]);
                    var text = listBrothers[ii].getAttribute("subId");
                    var coordinates = {x:toolTipX,y:toolTipY};
                    this.computeToolTip(coordinates,text,parentId); // Add ToolTip
                }
            }
        }

    };
    // ToolTip function
    this.computeToolTip=function(coordinates,text,parentId,params,handlers){
        if(!isok(params))params=0;
        var backgroundColor="lightyellow";
        var fontweight="bold";
        var boxOpacity = 0.8 ;
        var fontsize = "12px";
        var headerfontweight="bold";
        var headerfontsize = "13px";
        var ellipsizeWidth = 50;
        var textSeperator= "\n";
        var headertext=0;
        if(isok(params.backgroundColor)) backgroundColor=params.backgroundColor;
        if(isok(params.boxOpacity)) boxOpacity=params.boxOpacity;
        if(isok(params.fontweight)) fontweight=params.fontweight;
        if(isok(params.fontsize)) fontsize=params.fontsize;
        if(isok(params.ellipsizeWidth)) ellipsizeWidth=params.ellipsizeWidth;
        if (isok(params.separator))textSeperator = params.separator;
        if(isok(params.title)){
            if(isok(params.title.content)) headertext=params.title.content;
            if(isok(params.title.fontweight)) headerfontweight=params.title.fontweight;
            if(isok(params.title.fontsize)) headerfontsize=params.title.fontsize;
        }

        var boxHeight=0, boxWidth=0, maxR=0,maxLen=0;
        text=text.split(textSeperator);
        for(var ir=0; ir<text.length;++ir){
            var linelen=text[ir].length;
            if(linelen>maxLen){
                maxLen=linelen;
                maxR=ir;
            }
            if(!text[ir].length)text.splice(ir,1);
        }

        if (text.length>0){
            boxHeight = 12 * text.length;
            if(headertext)boxHeight+=16;
            coordinates.x = parseFloat(coordinates.x);
            coordinates.y = parseFloat(coordinates.y);
            var svgChd = document.createElementNS("http://www.w3.org/2000/svg","g"); // create SVG Element
            svgChd.setAttribute("id", "toolTipBox");
            svgChd.setAttribute("x", 0);
            svgChd.setAttribute("y",0);
            // svgChd.setAttribute("width",boxWidth+6);
            // svgChd.setAttribute("height",boxHeight+6);
            var toolTipBox = document.createElementNS(
                    "http://www.w3.org/2000/svg", "rect");
            toolTipBox.setAttribute("x", coordinates.x);
            toolTipBox.setAttribute("y", coordinates.y - boxHeight);
            toolTipBox.setAttribute("width", boxWidth + 6);
            toolTipBox.setAttribute("height", boxHeight + 6);
            toolTipBox.style["stroke-width"] = 0.5;
            toolTipBox.style["stroke"] = "black";
            toolTipBox.style["stroke-opacity"] = 1;
            toolTipBox.style["stroke-linecap"] = "round";
            toolTipBox.style["fill"] = backgroundColor;
            toolTipBox.style["fill-opacity"] = boxOpacity;

            if(isok(handlers)){
                for(ii in handlers){
                    toolTipBox.addEventListener(ii,handlers[ii]);
                }
            }
            svgChd.appendChild(toolTipBox); // add the box to the SVG Element
            var startY = coordinates.y + 12 - boxHeight;
            var startX = coordinates.x + 20 + (isok(params.colors) ? 2 : 0);
            if (headertext) {
                var headerTip = document.createElementNS(
                        "http://www.w3.org/2000/svg", "text");
                headerTip.setAttribute("x", coordinates.x + 2);
                headerTip.setAttribute("y", startY);
                headerTip.textContent = headertext;
                headerTip.style["font-size"] = headerfontsize;
                headerTip.style["font-weight"]= headerfontweight;
                headerTip.style["fill-opacity"] = 1;
                if(isok(handlers)){
                    for(ii in handlers){
                        headerTip.addEventListener(ii,handlers[ii]);
                    }
                }
                svgChd.appendChild(headerTip);
                startY += 16;
            }
            for ( var l = 0; l < text.length; l++) {

                var textTips = document.createElementNS(
                        "http://www.w3.org/2000/svg", "text");
                textTips.setAttribute("x", startX);
                textTips.setAttribute("y", startY);
                // textTips.setAttribute("font-family","Courier");
                textTips.style["font-size"]= fontsize;
                textTips.style["font-weight"]=fontweight;
                textTips.style["fill-opacity"]=0.8;
                var realText = text[l];
                if (ellipsizeWidth && realText.length > ellipsizeWidth) { //  && realText.indexOf(">") == -1 && realText.indexOf("<") == -1
                    var lastIndexToKeep = ("...") ? realText.lastIndexOf("...") : realText.length;
                    if(lastIndexToKeep==-1)lastIndexToKeep=realText.length;
                    realText=realText.substr(0,lastIndexToKeep);
                    if( realText.length > ellipsizeWidth+3 ) {
                        realText = realText.substr( 0, ellipsizeWidth/2) + "..." + realText.substr(realText.length-ellipsizeWidth/2);
                    }
                    var auto_title = document.createElementNS("http://www.w3.org/2000/svg","title");
                    auto_title.textContent = text[l];
                    textTips.appendChild(auto_title);
                }
                textTips.setAttribute("ellipsizeWidth", ellipsizeWidth);
                textTips.textContent = realText;
                svgChd.appendChild(textTips);

                if (isok(params.colors)) {
                    var lineTipColor = document.createElementNS(
                            "http://www.w3.org/2000/svg", "rect");
                    lineTipColor.setAttribute("x", startX - 20);
                    lineTipColor.setAttribute("y", startY - 10);
                    lineTipColor.setAttribute("width", 10);
                    lineTipColor.setAttribute("height", 10);
                    lineTipColor.style["fill"]=(isok(params.colors[l]) ? params.colors[l]: params.colors);

                    svgChd.appendChild(lineTipColor);
                }
                startY = startY + 12;
            }

            var parentElem=document.getElementById(parentId);
            if(parentElem)parentElem.appendChild(svgChd);
            var childIndex = (isok(params.colors)) ? 2 * maxR + 1 : maxR + 1;
            var maxPixels = 0;
            if (headertext) {
                maxPixels = svgChd.childNodes[1].getComputedTextLength() + 20;
                childIndex += 1;
            }
            var maxTextPixels = svgChd.childNodes[childIndex]
                    .getComputedTextLength() + 40;
            if (maxTextPixels > maxPixels)
                maxPixels = maxTextPixels;

            //svgChd.setAttribute("width", maxPixels);
            svgChd.childNodes[0].setAttribute("width", maxPixels);
            var curX = parseInt(svgChd.childNodes[0].getAttribute("x"));
            // var
            // maxHeight=+parseInt(svgChd.childNodes[0].getAttribute("height"));
            var translY = 0, translX = 0;
            if (parseInt(svgChd.childNodes[0].getAttribute("y")) < 0)
                translY = parseInt(svgChd.childNodes[0].getAttribute("height"));
            if (curX + maxPixels > parseInt(svgChd.ownerSVGElement
                    .getAttribute("width"))) {
                translX = -(maxPixels + 2);
            }
            if (translX || translY)
                svgChd.setAttribute('transform', "translate(" + translX + ","
                        + translY + ")");

            return svgChd;
        }
        else return alert("Please check the computeToolTip function");
    };



    this.mouseOutShowId=function(ir,eventObjID,evt){
        //alert("mouseOut");
        this.restoreBrush(eventObjID);
        var nameValue = eventObjID.getAttributeNode("name").nodeValue;
        var listBrothers = document.getElementsByName(nameValue);
        //document.removeChild(document.getElementById("toolTipBox"));
        for (var ii=0;ii<listBrothers.length;ii++){
            //alerJ(" ii " + ii + "length " + listBrothers.length,listBrothers[ii])
            this.restoreBrush(listBrothers[ii]);
            var parent = listBrothers[ii].parentNode.id;
            //alerJ("" +     parent + "mouse out ", document.getElementById(parent).childNodes)
            if (document.getElementById(parent).getElementById("toolTipBox")!=null)
                //alerJ("",document.getElementById("toolTipBox"))
                document.getElementById(parent).removeChild(document.getElementById("toolTipBox"));
        }
    };


    this.clickForCallingBack=function(ir,eventObjID,evt){
        //alert("I am calling you")
        //alerJ("fdfd",eventObjID)
        var func = this.selectCallback;
        var obj=vjObj[eventObjID.attributes.objID.value];
        if (!obj.children.length) funcLink(func,obj,evt);
        else {
            for(var i=0 ; i<obj.children.length;++i){
                //alerJ("obj child ", obj.children[i])
                //alerJ(" svgObj ", svgObj)
                var ObjC=obj.children[i];
                //alerJ("OBJC " + ObjC.objID)
                //alert("svgObjC " + ObjC.objID + " eventObjID " +  eventObjID.attributes.objID.value)
                if (i==20) break;
                if (ObjC.objID == eventObjID.attributes.objID.value){
                    funcLink(func,ObjC,evt);
                    //break;
                }
            }
        }
    };
    this.mouseOverLine = function(ir,eventObjID,evt){
        //alerJ("here",eventObjID);
        //for (var ii=0;ii<eventObjID.attributes.length;ii++){
            //alert(eventObjID.attributes[ii].name +": "+ eventObjID.getAttribute(eventObjID.attributes[ii].name));
        //}
        var crdToolTips = new Object();
        if (eventObjID.getAttribute("x") && eventObjID.getAttribute("y")){
            crdToolTips.x = eventObjID.getAttribute("x")+ 20;
            crdToolTips.y = eventObjID.getAttribute("y") - 30;
        }
        if (crdToolTips.x){
            var text = "";
            if (eventObjID.getAttribute("valueX") && eventObjID.getAttribute("valueY")){
                text = text + eventObjID.getAttribute("valueX") + "\n" + eventObjID.getAttribute("valueY");
            }
            var parentId = eventObjID.parentNode.id;
            var parentObj = gObject(parentId);
            this.computeToolTip(crdToolTips,text,parentId); // Add ToolTip
            var circle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
            circle.setAttributeNS(null,"cx",parseFloat(eventObjID.getAttribute("x")));
            circle.setAttributeNS(null,"cy",parseFloat(eventObjID.getAttribute("y"))-10);
            circle.setAttributeNS(null,"r",4.5);
            circle.setAttributeNS(null,"id","toolTipBox");
            var clor = "red";
            if (eventObjID.getAttribute("stroke")) clor = eventObjID.getAttribute("stroke");
            circle.setAttributeNS(null,"style","fill:"+clor+";fill-opacity:1");
            parentObj.appendChild(circle);
        }

    };

    this.mouseOutLine = function(ir,eventObjID,evt){
        var parentObj = gObject(eventObjID.parentNode.id);
        var arrObjToBeRemoved = new Array();
        for (var ii=0;ii<parentObj.childElementCount;ii++){
            if (parentObj.childNodes[ii].id.indexOf("toolTipBox")!==-1) {
                //parentObj.removeChild(parentObj.childNodes[ii]);
                arrObjToBeRemoved.push(parentObj.childNodes[ii]);
            }
        }
        if (arrObjToBeRemoved.length){
            for (var ii=0;ii<arrObjToBeRemoved.length;ii++)
                parentObj.removeChild(arrObjToBeRemoved[ii]);
        }

    };

    // return preferred { x, y, z } scale
    // Default implementation agrees with the proposal
    this.preferredScale = function(proposed) {
        return { x:proposed.x, y:proposed.y, z:proposed.z };
    };

    if (this.makeGroup) {
        this.primitiveRender = this.render;
        this.render = function(svg) {
            var g = svg.group(this.objID + "_chart_group");
            var old_svg_grpobj = svg.curGroupNode;
            //g.svgID = svg.svg.id;
            g.id = svg.svg.id + "_Chart_group";
            svg.curGroupNode = g;
            this.primitiveRender(svg);
            old_svg_grpobj.appendChild(g);
            svg.curGroupNode = old_svg_grpobj;
        };
    }
}










































