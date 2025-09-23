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



function vjD3JS_circularGraph ( viewer )
{
    vjD3CategoryView.call(this,viewer);
    var divElement;
    var that = this;
    
    if (this.margin == undefined || !this.margin) {this.margin = 100;}
    if (this.showTicks == undefined) {this.showTicks=0;}
    if (this.showLabels == undefined) {this.showLabels=0;}

    
    this.d3Compose=function(data){       
        
        this.d3Compose_prv(data);
        var thiSS=this;
        var svg=this.d3svg;
        if (this.container) {
            divElement = gObject(this.container); 
            if (this.width == undefined || !this.width){this.width = parseInt(divElement.style.maxWidth);}
            if (this.height == undefined || !this.height){this.height = parseInt(divElement.style.maxHeight);}
            if (!this.height) this.height=800;
            if (!this.width) this.width=800;
        }
        
        this.setType = function (type ) {
            this._type = type;
        }
        
        this.switchType = function () {
            var type = this._type;
            
            this.setType(type);;
        }
        
        this.refresh = function () {
            this.reDraw();
        }
        
        this.reDraw = function () {
            this._draw();
        }
        

        if (!this.graphOptions || this.graphOptions == undefined) this.graphOptions = {};
        if (!this.graphOptions.x || this.graphOptions.x == undefined) this.graphOptions.x = {};
        if (!this.graphOptions.x.title || this.graphOptions.x.title == undefined) this.graphOptions.x.title = "";
        
        if (!this.graphOptions.y || this.graphOptions.y == undefined) this.graphOptions.y = {};
        if (!this.graphOptions.y.title || this.graphOptions.y.title == undefined) this.graphOptions.y.title = "";
        
        if (this.graphOptions.showGrid == undefined) this.graphOptions.showGrid = true;
        
        if (this.graphOptions.zoomStatic == undefined) this.graphOptions.zoomStatic= true;
        
        var default_options = {
                color: "steelblue"
                ,vAxis: {
                    title: ''
                }
                ,hAxis: {
                    title: ''
                }
        };
        
        var _parseOptions = function (serie){
            if (!serie.options) {serie.options= default_options;}
            if (!serie.options.color) serie.optinos.color = default_options.color;
        }
        
    
        var _getMinMax = function (data,x_col,y_col){
            var xMin=Number.MAX_VALUE, xMax=Number.MIN_VALUE, yMin=Number.MAX_VALUE, yMax=Number.MIN_VALUE;
            for (var i=0; i<data.length;++i) {
                var xVal = Number(data[i][x_col]);
                var yVal = Number(data[i][y_col]);
                
                if (xVal < xMin) xMin = xVal;
                if (yVal < yMin) yMin = yVal;
                
                if (xVal > xMax) xMax = xVal;
                if (yVal > yMax) yMax = yVal;
            }
            return {xMin: xMin,xMax: xMax,yMin:yMin,yMax: yMax};
        }

        blabla=0;
        var bandPos = [-1, -1];
        var pos;
        var xdomain, ydomain;
        
        var colors = this.colors || ["steelblue", "green","#a34974","orange","purple","#40E0D0"];

        var margin = {
              top: 40,
              right: 40,
              bottom: 50,
              left: 60
        }
        
        var width = this.width - margin.left - margin.right;
        var height = this.height - margin.top - margin.bottom;
        
        this.data = verarr(this.data);
        
        var nbOfSections = this.data.length +1;
        
        var originx = width/2;
        var originy = height/2;
        var pi = Math.PI;
        var DRratio = pi/180;

        var baseRadius = Math.min(width,height)/2 - Math.max((margin.top+margin.bottom),(margin.right+margin.left));
        baseRadius = baseRadius/(nbOfSections);
        baseRadius = 0.9 *baseRadius;

        var xMin=Number.MAX_VALUE, xMax=Number.MIN_VALUE, yMin=Number.MAX_VALUE, yMax=Number.MIN_VALUE;
        
        var _drawLineChart = function (canvas,data,x_col,y_col,innerRadius,outterRadius,minMax,options){

            var y = d3.scale.linear()
                     .range([innerRadius, outterRadius])
                     .domain([minMax.yMin, minMax.yMax]);
            
            var line = d3.svg.line()
              .x(function(d) {
                return originx + ( d.radius* Math.cos(d.angle));
              })
              .y(function(d) {
                return originy +(d.radius * Math.sin(d.angle));
              });
            
            var degPerUnit = 360/(minMax.xMax-minMax.xMin);
            var radPerUnit = degPerUnit*DRratio;
            
            var lineGroup = canvas.append("g")
                                .attr("class","circular_line");
            lineGroup.append("circle")
            .attr("cx", originx)
            .attr("cy", originx)
            .attr("r", outterRadius)
            .attr("fill","none")
            .attr("stroke","none")
            .attr("class", "center_circle");
            
            blabla +=1;
            var pointList = [];
            for (var i=0; i<data.length;++i) {
                var cur_val = radPerUnit * data[i][x_col];
                var radius = y(data[i][y_col]);
                pointList.push({angle:cur_val,radius:radius})
                lineGroup.append("circle")
                        .attr("cx", originx + ( radius* Math.cos(cur_val)))
                        .attr("cy", originy + (radius * Math.sin(cur_val)))
                        .attr("r",1)
                        .attr("fill",colors[blabla])
                        .attr("class", "cirlar_line line");
            }
            lineGroup.append("path")
                     .datum(pointList)
                     .attr("stroke",colors[blabla])
                     .attr("fill","none")
                     .attr("d",line);
        }    
        
        var _drawColumnChart = function(canvas,data,x_col,y_col,innerRadius,outterRadius,minMax,options) {

                data.options = options;
                var pie = d3.layout.pie()
                    .sort(null)
                    .value(function(d) { 
                        return d[x_col]; 
                    });
                
                var y = d3.scale.linear()
                    .range([innerRadius, outterRadius])
                     .domain([0, minMax.yMax]);
                
                var arc = d3.svg.arc()
                    .outerRadius(function(d){
                        return y(d.data[y_col]);
                    })
                    .innerRadius(function(d){
                        return y(d.data[y_col] - d.data[y_col] *0.3);
                    })
                
                var arcGroup = canvas.append("g")
                                 .attr("transform", "translate(" + originx + "," + originy + ")")
                                 .attr("class","circular_arc");
                
                 var g = arcGroup.selectAll(".arc")
                             .data(pie(data))
                         .enter().append("g")
                             .attr("class", "arc");
            
                 g.append("path")
                     .attr("d", arc)
                     .style("stroke",function(d) { 
                         return gClrTable[d.value]; 
                      })
                     .style("fill", function(d) { 
                         return gClrTable[d.value]; 
                      });
                 
                 g.append("text")
                     .attr("transform", function(d) { return "translate(" + arc.centroid(d) + ")"; })
                     .attr("dy", ".35em")
                     .attr("font-size","10px")
                     .text(function(d) { return d.data[x_col] + "-" + d.data[y_col]; });
                 
                 
        }
        var _draw = function (canvas, cur_serie, serie_idx) 
        {
                var dsname = cur_serie.dsname;
                var graphType = cur_serie.type;
                var x_col = cur_serie.x;
                var y_col = cur_serie.y;
                var options = cur_serie.options;
                
                var data = d3.csv.parse(vjDS[dsname].data); 
                var innerRadius = baseRadius * (serie_idx+1);
                var outterRadius = innerRadius + baseRadius;
                    
                if (!serie_idx) {
                    canvas.append("circle")
                        .attr("cx", originx)
                        .attr("cy", originy)
                        .attr("r", baseRadius)
                        .attr("stroke", "red")
                        .attr("fill", "none")
                        .attr("class", "center_circle");
                }
                var minMax = _getMinMax(data,x_col,y_col);
                
                switch (graphType) {
                    case "line":
                        _drawLineChart(canvas,data,x_col,y_col,innerRadius,outterRadius,minMax,options);
                        break;
                    case "column":
                        _drawColumnChart(canvas,data,x_col,y_col,innerRadius,outterRadius,minMax,options);
                        break;
                    case "annotationBox":
                        
                        break;
                    default:
                        console.log("No graph type specified !");
                        return ;
                }
            
        }
        svg.attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom);
        
        var gg = svg.append("g")
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

        for (var is=0; is < this.series.length; ++is) {
            _parseOptions(this.series[is]);
            _draw(gg,this.series[is], is);
        }
        

        
        
        function zoom() {}

        var zoomOut = function() {}
    }
}

