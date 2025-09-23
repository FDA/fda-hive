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
function vjD3JS_lineGraph ( viewer )
{
    loadCSS("d3js/css/zoomable_lineGraph.css");
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

        if (!this.graphOptions || this.graphOptions == undefined) this.graphOptions = {};
        if (!this.graphOptions.x || this.graphOptions.x == undefined) this.graphOptions.x = {};
        if (!this.graphOptions.x.title || this.graphOptions.x.title == undefined) this.graphOptions.x.title = "";
        
        if (!this.graphOptions.y || this.graphOptions.y == undefined) this.graphOptions.y = {};
        if (!this.graphOptions.y.title || this.graphOptions.y.title == undefined) this.graphOptions.y.title = "";
        
        if (this.graphOptions.showGrid == undefined) this.graphOptions.showGrid = true;
        
        if (this.graphOptions.zoomStatic == undefined) this.graphOptions.zoomStatic= true;
        
        var xCol = this.columnDefinition.x, yCol = this.columnDefinition.y;
        if (!this.line_set || this.line_set == undefined) {
            this.line_set = [];
            var data_list = this.data;
            var data_length = this.data.length;
            for (var i=0; i< data_length; ++i) {
                var dsname = data_list[i];
                this.line_set[dsname] = {x : xCol , y: yCol , hidden: false, labels: [xCol,yCol]};
            }
        } 
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
        

        var xMin=Number.MAX_VALUE, xMax=Number.MIN_VALUE, yMin=Number.MAX_VALUE, yMax=Number.MIN_VALUE;
        
        var d1 = [];
        this.data_info=[];
        this.className_info ={};
        if (!data.length){
            xMin=0, xMax=10, yMin=0, yMax=10;
        }
        var first_dataSource_x = this.line_set[this.data[0]].x || xCol;
        var first_dataSource_y = this.line_set[this.data[0]].y || yCol;
        this.data_info.push({
                hidden: (this.line_set[this.data[0]].hidden!=undefined) ? this.line_set[this.data[0]].hidden : false,
                dsname: this.data[0]
        });
        for (var i=0; i<data.length;++i) {
            var xVal = Number(data[i][first_dataSource_x]);
            var yVal = Number(data[i][first_dataSource_y]);
            d1.push([xVal,yVal]);
            if (xVal < xMin) xMin = xVal;
            if (yVal < yMin) yMin = yVal;
            
            if (xVal > xMax) xMax = xVal;
            if (yVal > yMax) yMax = yVal;
        }
        xdomain = xMax; ydomain = yMax;
        data= [];
        data.push(d1);
        
        var line_DS_list = Object.keys(this.line_set);
        line_DS_list.splice(line_DS_list.indexOf(this.data[0]),1);
        
        for (var is=0; is< line_DS_list.length; ++is) {
             d1=[];
             var dsname = line_DS_list[is];
             var hidden = ( this.line_set[dsname].hidden!=undefined ) ? this.line_set[dsname].hidden : true;
             this.data_info.push({hidden: hidden, dsname: dsname});
             
                  var idxFromDataList = this.data.indexOf(dsname);
             var line_data = this.csvParserFunction(this.getData(idxFromDataList).data);
             var x_col = this.line_set[dsname].x;
             var y_col = this.line_set[dsname].y;
             for (var i=0; i< line_data.length;++i) {
                var xVal = Number(line_data[i][x_col]);
                var yVal = Number(line_data[i][y_col]);
                d1.push([xVal,yVal]);
                if (xVal < xMin) xMin = xVal;
                if (yVal < yMin) yMin = yVal;
                
                if (xVal > xMax) xMax = xVal;
                if (yVal > yMax) yMax = yVal;
            }
             data.push(d1);
        }
        xdomain = xMax; ydomain = yMax;
        
        var zoomArea = {
              x1: xMin,
              y1: yMin,
              x2: xdomain,
              y2: ydomain,
              xCol: xCol,
              yCol: yCol,
              graph_config:{
                  that: thiSS
              }
        };
        
        svg.attr("width", width + margin.left + margin.right)
          .attr("height", height + margin.top + margin.bottom);
        
       var gg = svg.append("g")
          .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

        var x = d3.scale.linear()
          .range([0, width])
          .domain([xMin, xdomain]);

        var xAxis = d3.svg.axis()
          .scale(x)
          .orient("bottom");
        
        var y = d3.scale.linear()
          .range([height, 0])
          .domain([yMin, ydomain]);

        var yAxis = d3.svg.axis()
          .scale(y)
          .orient("left");

        if (this.graphOptions.showGrid) {
            xAxis.innerTickSize(-height)
                .outerTickSize(0);
            yAxis.innerTickSize(-width)
                 .outerTickSize(0);
        }

        var line = d3.svg.line()
          .x(function(d) {
            return x(d[0]);
          })
          .y(function(d) {
            return y(d[1]);
          });
      
       var tip_id= "d3-tip-lineGraph";
       if (this.graphId) {
           tip_id+= "_" + this.graphId;
       }
       var tipObj = gObject(tip_id);
       if (tipObj) {
           tipObj.remove();
       }
       var tip = d3.tooltip()
        .attr('class', 'd3-tip')
        .attr("id", tip_id)
        .offset([-10, 0])
        .html(function(d, bb) {
          var toReturn = "";
          if (typeof d !== 'object'){
              var toSplit = d.split("\n");
              for (var i=0; i<toSplit.length; ++i) {
                  if (i) toReturn += "<br>";
                  toReturn += "<strong>" + toSplit[i] + "</strong>";  
              }
          }  
          else {
            toReturn = "<strong>" + zoomArea.xCol+ ": </strong> <span style='color:red'>" + d[0] + "</span><br>" +
                 "<strong> "+ zoomArea.yCol +": </strong> <span style='color:red'>" + d[1] + "</span>";
          }
          return toReturn;
        })
        gg.call(tip);

       var band_class = "band";
       if (this.graphId) {
           band_class += " " + this.graphId;
       }
        var band = gg.append("rect")
          .attr("width", 0)
          .attr("height", 0)
          .attr("x", 0)
          .attr("y", 0)
          .attr("class", band_class);

       gg.append("g")
          .attr("class", "x axis")
          .call(xAxis)
          .attr("transform", "translate(0," + height + ")");
       
          gg.append("text")
          .attr("transform", "translate(0," + height + ")")
            .attr("x", (width *0.95))
            .attr("y", margin.bottom/2)
            .attr("dy", ".71em")
            .style("text-anchor", "middle")
            .text( this.graphOptions.x.title);
          
       if (this.graphOptions.title) 
       {       
           gg.append("text")
           .attr("transform", "translate(0," + height + ")")
           .attr("id","kk")
           .attr("x", (width*0.45))
           .attr("y", margin.bottom/2)
           .attr("dy", ".71em")
           .style("text-anchor", "middle")
           .text(this.graphOptions.title);
       }

        gg.append("g")
          .attr("class", "y axis")
          .call(yAxis)
          .append("text")
            .attr("transform", "rotate(-90)")
            .attr("x", 0 - (height / 2) )
            .attr("y", 0 - (margin.left *0.8))
            .attr("dy", ".71em")
            .style("text-anchor", "middle")
            .text(this.graphOptions.y.title);

        gg.append("clipPath")
          .attr("id", "clip")
          .append("rect")
          .attr("width", width)
          .attr("height", height);

        for (var iidx=0; iidx < data.length; ++iidx) {
             var _color = this.data_info[iidx].hidden ? "None" : colors[iidx];
             var className = "line " + this.data_info[iidx].dsname;
             this.className_info[this.data_info[iidx].dsname] = {type: {line:{className: className}}};
             this.className_info[this.data_info[iidx].dsname]["type"]["line"]["attribute"] = "stroke";
             this.className_info[this.data_info[iidx].dsname]["type"]["line"]["stroke"] = colors[iidx];
             this.className_info[this.data_info[iidx].dsname]["type"]["line"]["default"] = "None";
              gg.append("path")
                .datum(data[iidx])
                .attr("class", className)
                .attr("clip-path", "url(#clip)")
                .style("stroke", _color)
                .attr("d", line);
        }
        
        var drag = d3.behavior.drag();
        var zoomOverlay = gg.append("rect")
          .attr("width", width - 10)
          .attr("height", height)
          .attr("class", "zoomOverlay")
          .call(drag);

        var zoomout = gg.append("g");
        zoomout.append("rect")
          .attr("class", "zoomOut")
          .attr("width", 65)
          .attr("height", 20)
          .attr("rx",5)
          .attr("ry",5)
          .attr("x", -12)
          .attr("y", height + (margin.bottom - 20))
          .on("click", function() {
            zoomOut();
          })
          .on("mouseover", function(){
            tip.show("click here");
          })
          .on("mouseout", function() {
            tip.hide();
          });

        zoomout.append("text")
          .attr("class", "zoomOutText")
          .attr("width", 75)
          .attr("height", 30)
          .attr("x", -10)
          .attr("y", height + (margin.bottom - 5))
          .text("Zoom Out");

        var zoomIn = gg.append("g");
            zoomIn.append("rect")
                .attr("class", "zoomOut")
                .attr("width", 60)
                .attr("height", 20)
                .attr("x", 80)
                .attr("y", height + (margin.bottom - 20))
                .attr("rx",5)
                .attr("ry",5)
                .on("mouseover", function(){
                  tip.show("Drag a rectangle around an area");
                })
                .on("mouseout", function() {
                  tip.hide();
                });

          zoomIn.append("text")
            .attr("class", "zoomOutText")
            .attr("width", 60)
            .attr("height", 30)
            .attr("x", 82)
            .attr("y", height + (margin.bottom - 5))
            .text("Zoom In");


        drag.on("dragend", function() {
          var pos = d3.mouse(this);
          var x1 = x.invert(bandPos[0]);
          var x2 = x.invert(pos[0]);

          if (x1 < x2) {
            zoomArea.x1 = x1;
            zoomArea.x2 = x2;
          } else {
            zoomArea.x1 = x2;
            zoomArea.x2 = x1;
          }

          var y1 = ( y.invert(pos[1]) >= 0 ) ?  y.invert(pos[1]) : 0 ;
          var y2 = ( y.invert(bandPos[1]) >=0 ) ? y.invert(bandPos[1]) : 0;
          
          if (x1 < x2) {
            zoomArea.y1 = y1;
            zoomArea.y2 = y2;
          } else {
            zoomArea.y1 = y2;
            zoomArea.y2 = y1;
          }

          bandPos = [-1, -1];
          var selector = d3.select(".band");
          if (zoomArea.graph_config.that.graphId) {
              selector = d3.select(".band"+"."+that.graphId)
          }
          
          selector.transition()
            .attr("width", 0)
            .attr("height", 0)
            .attr("x", bandPos[0])
            .attr("y", bandPos[1]);

          zoom();
        });

      
        
      for (var idx=0; idx <data.length; ++idx) {
          var size = this.data_info[idx].hidden ? 0 : 3;
          var className = "dot " + this.data_info[idx].dsname; 
          this.className_info[this.data_info[idx].dsname]["type"]["dot"]={className: className};
          this.className_info[this.data_info[idx].dsname]["type"]["dot"]["attribute"] = "r";
          this.className_info[this.data_info[idx].dsname]["type"]["dot"]["r"] = 3;
          this.className_info[this.data_info[idx].dsname]["type"]["dot"]["default"] = 0;
          gg.append("g").selectAll(".dot")
          .data(data[idx])
          .enter().append("circle")
          .attr("r", size)
          .attr("class",className)
          .attr("cx", function(d) {
            return x(d[0]);
          })
          .attr("cy", function(d) {
             return y(d[1]);
          })
          .on("mouseover", function(d){
              tip.show(d);
              this.style.opacity= "1";
          })
          .on("mouseout",function(d) {
              tip.hide(d);
              this.style.opacity= "0";
          });
      }
      
      
       
       if (this.data.length && this.data.length >1 && this.column_set){
           var column_DS_list = Object.keys(this.column_set);
           for (var ic=0; ic< column_DS_list.length; ++ic) {
               var dsname = column_DS_list[ic];
               var idxFromDataList = this.data.indexOf(dsname);
               var histo_data = this.csvParserFunction(this.getData(idxFromDataList).data);
               zoomArea.histoXCol = this.column_set[this.data[idxFromDataList]].x;
               zoomArea.histoYCol = this.column_set[this.data[idxFromDataList]].y;
               zoomArea.histoLabels = this.column_set[this.data[idxFromDataList]].labels;
               var size = this.column_set[this.data[idxFromDataList]].hidden ? 0 : 1.8;
               var className = "histo " + dsname;
               this.className_info[dsname]={type: {histo: {className: className}}};
               this.className_info[dsname]["type"]["histo"]["attribute"]="width";
               this.className_info[dsname]["type"]["histo"]["width"]=1.5;
               this.className_info[dsname]["type"]["histo"]["default"]=0;
              gg.append("g").selectAll(".histo")
                  .data(histo_data)
              .enter().append("g")
                  .append("rect")
                      .attr("class",className)
                      .attr("x", function(d) {
                        return (d[zoomArea.histoXCol]<xMin) ? -100 : x(d[zoomArea.histoXCol]);
                      })
                      .attr("y",function(d) {
                        return y(d[zoomArea.histoYCol]);
                      })
                      .attr("width", size)
                      .attr(zoomArea.histoXCol,function(d){return d[zoomArea.histoXCol];})
                      .attr(zoomArea.histoYCol,function(d){return d[zoomArea.histoYCol];})
                      .attr("height", function(d) { 
                          return height - y(d[zoomArea.histoYCol]); 
                      })
                      .on("mouseover", function(d){
                          var toShow = "";
                          for (var i=0; i<zoomArea.histoLabels.length; ++i) {
                              if (i) toShow += "\n ";
                              toShow += "" + zoomArea.histoLabels[i] + ": " + d[zoomArea.histoLabels[i]];
                          }
                          tip.show(toShow);
                      })
                      .on("mouseout",function(d) {
                          tip.hide();
                      });
              var peak_className = "peakInfo_text";
              if (zoomArea.graph_config.that.graphId) {
                  peak_className += " " + zoomArea.graph_config.that.graphId;
              }
              gg.append("g").selectAll(".peakInfo")
                  .data(histo_data)
                  .enter().append("g")
                    .append("text")
                        .attr("class",peak_className)
                        .attr("x",function(d){
                            return  (d[zoomArea.histoXCol]<xMin || d[zoomArea.histoXCol]>xMax) ? -1000 : (x(d[zoomArea.histoXCol]));
                        })
                        .attr("y",function(d,i) {
                            if (i%2!=0) {
                                return y(d[zoomArea.histoYCol]);
                            }
                            return y(d[zoomArea.histoYCol]);
                        })
                        .attr("font-size",function(d){
                            return (zoomArea.graph_config.that.graphOptions.hidePeakInfo) ? "0px" : "12px";
                        })
                        .attr("font-family", "monospace")
                        .attr("font-weight","bold")
                        .attr("fill","red")
                        .attr("text-decoration","underline")
                        .attr("transform",function (d,i){
                            var trans = "rotate(270 ";
                            if (zoomArea.graph_config.that.graphOptions.peakInfo && zoomArea.graph_config.that.graphOptions.peakInfo.horizontal) {
                                trans="translate(-"+ ( ( d[zoomArea.histoLabels[0]]).length *1.2)  +",-10)";
                            }
                            else {
                                trans+= "" + x(d[zoomArea.histoXCol]) + "," + (y(d[zoomArea.histoYCol]))+ ")";
                                trans+= "translate("+ ((d[zoomArea.histoLabels[0]]).length + (d[zoomArea.histoLabels[0]]).length * 0.1) +",0)" ;
                            }
                            return trans;
                        })
                        .text(function(d,i){
                            return (d[zoomArea.histoLabels[0]] ) ? ( d[zoomArea.histoLabels[0]].replace(/&lt;-&gt;/g, "") ) : d[zoomArea.histoXCol];
                        });
               
           }
        
       }
        drag.on("drag", function() {

          var pos = d3.mouse(this);
          
          var selector = d3.select(".band");
          if (zoomArea.graph_config.that.graphId) {
              selector = d3.select(".band"+"."+that.graphId)
          }
          
          if (pos[0] < bandPos[0]) {
            selector.
            attr("transform", "translate(" + (pos[0]) + "," + bandPos[1] + ")");
          }
          if (pos[1] < bandPos[1]) {
            selector.attr("transform", "translate(" + (pos[0]) + "," + pos[1] + ")");
          }
          if (pos[1] < bandPos[1] && pos[0] > bandPos[0]) {
            selector.attr("transform", "translate(" + (bandPos[0]) + "," + pos[1] + ")");
          }

          if (bandPos[0] == -1) {
            bandPos = pos;
            selector.attr("transform", "translate(" + bandPos[0] + "," + bandPos[1] + ")");
          }

          selector.transition().duration(1)
            .attr("width", Math.abs(bandPos[0] - pos[0]))
            .attr("height", Math.abs(bandPos[1] - pos[1]));
        });
        
        var tmpXmin, tmpXmax; 
        function zoom() {

          if (zoomArea.x1 > zoomArea.x2) {
            x.domain([zoomArea.x2, zoomArea.x1]);
            tmpXmin = zoomArea.x2;
            tmpXmax = zoomArea.x1;
          } else {
            x.domain([zoomArea.x1, zoomArea.x2]);
            tmpXmin = zoomArea.x1;
            tmpXmax = zoomArea.x2;
          }

          if (zoomArea.y1 > zoomArea.y2) {
            y.domain([zoomArea.y2, zoomArea.y1]);
          } else {
            y.domain([zoomArea.y1, zoomArea.y2]);
          }

          
          if (zoomArea.graph_config.that.graphOptions.zoomStatic) {
              var t = svg.transition().duration(750);
              t.select(".x.axis").call(xAxis);
              t.select(".y.axis").call(yAxis);
    
              t.selectAll(".line").attr("d", line);
              t.selectAll(".dot").attr("cx", function(d){
                                      return ( d[0]>=tmpXmin && d[0]<=tmpXmax) ? x(d[0]) : -100 ;
                                  })
                                 .attr("cy",function(d){
                                     return ( d[1]>=yMin && d[1]<=yMax) ? y(d[1]) : -100 ;
                                 });
              t.selectAll(".histo").attr("x", function(d){
                                              var retVal = ( d[zoomArea.histoXCol]>=tmpXmin && d[zoomArea.histoXCol]<=tmpXmax) ? x(d[zoomArea.histoXCol]) : -100 ;
                                              return retVal;
                                      })
                                  .attr("y", function(d){
                                        var retVal = ( d[zoomArea.histoYCol]>=yMin && d[zoomArea.histoYCol]<=yMax) ? (y(d[zoomArea.histoYCol])) : -100 ;
                                        return retVal;
                                   })
                                   .attr("height", function(d) { 
                                           return height - y(d[zoomArea.histoYCol]); 
                                   });
          }
          else {
            var line_data = Object.keys(zoomArea.graph_config.that.line_set);
            for (var il=0; il < line_data.length; ++il) {
                var col = 0;
                var cur_ds = vjDS[line_data[il]];
                var tqs = "[{\"op\":\"filter\",\"arg\":{\"col\":\"" + String(col) +"\",\"method\":\"range\",\"value\":{\"min\":\"" + String(tmpXmin) + "\",\"max\":\"" + String(tmpXmax) + "\"}}}]";
                var url = cur_ds.url;
                url = urlExchangeParameter(url, "tqs",tqs);
                cur_ds.reload(url,true);
            }
          }
        }

        var zoomOut = function() {
            if (zoomArea.graph_config.that.graphOptions.zoomStatic) 
            {
              x.domain([xMin, xdomain]);
              y.domain([yMin, ydomain]);
    
              var t = svg.transition().duration(750);
              t.select(".x.axis").call(xAxis);
              t.select(".y.axis").call(yAxis);
    
              t.selectAll(".line").attr("d", line);
              t.selectAll(".dot").attr("cx",function(d){return x(d[0]);})
                                 .attr("cy",function(d){return y(d[1]);});
              t.selectAll(".histo").attr("x",function(d){return x(d[zoomArea.histoXCol]);})
                                     .attr("y",function(d){return y(d[zoomArea.histoYCol]);})
                                   .attr("height", function(d) { 
                                           return height - y(d[zoomArea.histoYCol]); 
                                   });
            }
            else {
                var line_data = Object.keys(zoomArea.graph_config.that.line_set);
                for (var il=0; il < line_data.length; ++il) {
                    var cur_ds = vjDS[line_data[il]]; 
                    var url = cur_ds.url;
                    url = urlExchangeParameter(url, "tqs","[]");
                    cur_ds.reload(url,true);
                }
                
            }
        }
    };
}

