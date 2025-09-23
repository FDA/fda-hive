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

function vjD3JS_annotationBox ( viewer )
{
    loadCSS("d3js/css/annotation_box.css");
    vjD3CategoryView.call(this,viewer);
    var divElement;
    var that = this;
    this.determine_lane = function(lanes_collection, item_start, item_end) {
        var corrected_lane = 0;
        var nextLane = true;
        var newLane = true;
        var nbOfLanes = lanes_collection.length;
        for( var il = 0; il < nbOfLanes; ++il) {
            var lane = lanes_collection[il];
            if(nextLane) {
                nextLane = false;
                var foundFreeSpot = true;
                for ( var ii=0; ii< lane.length; ++ ii) {
                    var start = lane[ii][0];
                    var end = lane[ii][1];
                    if((start > item_end) || (end <item_start)) {
                        foundFreeSpot = true;
                    }
                    if ((start >= item_start && start <= item_end) || (end >= item_start && start <= item_end) || (start <= item_start && end >= item_end) || (start > item_start && end < item_end)) {
                        nextLane = true;
                        foundFreeSpot = false;
                        break;
                    }
                }
                if (foundFreeSpot) {
                    corrected_lane = il;
                    newLane = false;
                    break;
                }
           }
        }
        if (newLane) {
            corrected_lane = nbOfLanes;
        }
        return corrected_lane;
    }
    
    this.data_treatment = function (data) {
        var items = [];
        var collection = {};
        var listOfKeys = [];
        var ir=0;
        
        if (this.graphOptions.has_reference)
        { 
            var ref_info = data[0];
            this.myReference = {
                                "id"    : ref_info[this.columnToPick["refID"]],
                                "start"    : Number(ref_info[this.columnToPick["start"]]), 
                                "end"      : Number(ref_info[this.columnToPick["end"]]),
                                "idType-id": ref_info[this.columnToPick["idType-id"]]
                                };
            ir=1;
        }
        for (; ir < data.length; ++ir) 
        {
            var cur_element = data[ir];
            var cur_ref = cur_element[this.columnToPick["refID"]];
            var cur_start = Number(cur_element[this.columnToPick["start"]]);
            var cur_end = Number(cur_element[this.columnToPick["end"]]);
            
            if (this.min > cur_start && !this.graphOptions.fixedByMinMax) {
                this.min = cur_start;
            }
            if (this.max < cur_end && !this.graphOptions.fixedByMinMax) {
                this.max = cur_end;
            }
            if (listOfKeys.indexOf(cur_ref)==-1){
                listOfKeys.push(cur_ref);
            }
            if (!collection[cur_ref]){
                collection[cur_ref] = [];
            }    
            collection[cur_ref].push(ir);
        }
        
        
        var lanes_count = -1;
        var lanes_collection = [];
        if (!this.graphOptions.collapse) {
            lanes_collection = {};
        }
        for (var ik=0; ik < listOfKeys.length; ++ik) 
        {
            var cur_ref = listOfKeys[ik];
            var myColor = this.colorFunc(ik);
            var listOfRows = collection[cur_ref];
            var cur_level = lanes_count + 1;
            for (var ir=0; ir < listOfRows.length; ++ir) 
            {
                var cur_rowIndex = listOfRows[ir],
                    cur_element = data[cur_rowIndex];
                
                var itemToPut = {"lane": 0 , 
                                 "id": cur_ref, 
                                 "start": Number(cur_element[this.columnToPick["start"]]),
                                 "end": Number(cur_element[this.columnToPick["end"]]),
                                 "idType-id": cur_element[this.columnToPick["idType-id"]],
                                 "color": myColor
                                };
                var subLane_collection;
                if (!this.graphOptions.collapse) {
                    if (!lanes_collection[ik]) lanes_collection[ik]=[];
                    subLane_collection = lanes_collection[ik];
                }
                else subLane_collection = lanes_collection;
                var corrected_lane = this.determine_lane(subLane_collection, itemToPut.start,itemToPut.end);
                if (!subLane_collection[corrected_lane]) {    
                    subLane_collection[corrected_lane] = [];
                }
                subLane_collection[corrected_lane].push([itemToPut.start,itemToPut.end]);
                if (!this.graphOptions.collapse) {
                    corrected_lane +=  cur_level;
                }
                if (itemToPut.lane != corrected_lane) {
                    itemToPut.lane = corrected_lane;
                }
                if (lanes_count < corrected_lane) {
                    lanes_count = corrected_lane;
                }
                items.push(itemToPut);
            }
            
        }
        return {items: items, lanes_count: (lanes_count + 1)};
    }
    
    
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
        this.columnToPick = this.columnToPick || {"refID": "ref", "start": "start","end":"end", "idType-id": "idType-id"};
        this.min = this.min ? Number(this.min) :  Number.MAX_VALUE;
        this.max = this.max ? Number(this.max) : Number.MIN_VALUE;
        this.colorFunc = d3.scale.category20();
        this.graphId = this.graphId || Math.random();
        
        if (!this.graphOptions || this.graphOptions == undefined) this.graphOptions = {};
        if (this.graphOptions.collapse == undefined) this.graphOptions.collapse = false;
        if (this.graphOptions.showBoxLabel == undefined) this.graphOptions.showBoxLabel = false;
        if (this.graphOptions.has_reference == undefined) this.graphOptions.has_reference = false;
        if (this.graphOptions.show_reference == undefined) this.graphOptions.show_reference = false;
        if (this.graphOptions.showTipFull == undefined) this.graphOptions.showTipFull = false;
        if (this.graphOptions.fixedByMinMax == undefined) this.graphOptions.fixedByMinMax = false;
        if (this.min == Number.MAX_VALUE || this.max == Number.MIN_VALUE) this.graphOptions.fixedByMinMax = false;
        else this.graphOptions.fixedByMinMax = true;
        
        if (this.margin==undefined) this.margin = {};
        
        
        var margin = {top: this.margin.top ? this.margin.top : 20, right: this.margin.right ? this.margin.right : 5, bottom: this.margin.bottom ? this.margin.bottom : 15, left: this.margin.left ? this.margin.left : 5};
        var w = this.width - margin.right - margin.bottom,
            h = this.height - margin.top - margin.bottom;
        var refAreaHeight =  0.05 * h;
        if (!this.graphOptions.show_reference) {
            refAreaHeight = 0;
        }
        var boxAreaHeight = h - refAreaHeight - 0.1*h;
        
        if (data==undefined || !data.length)
            return;
        var data_info = this.data_treatment(data);
        
        var items = data_info.items;
        var nbOfLanes = data_info.lanes_count;
        var zoom_begin = this.min;
        var zoom_end = this.max;
        if (this.myReference) {
            zoom_begin = this.graphOptions.fixedByMinMax ? zoom_begin : this.myReference.start;
            zoom_end = this.graphOptions.fixedByMinMax ? zoom_end : this.myReference.end;
        }
        
        var whereToStart = 0;
        if (nbOfLanes < 3) {
            whereToStart = boxAreaHeight/2;
        }
        var x_scale = d3.scale.linear()
                .domain([zoom_begin, zoom_end])
                .range([0, w]);
        
        var boxHeight_scale = d3.scale.linear()
                .domain([0, nbOfLanes])
                .range([whereToStart, boxAreaHeight]);
        
        var refHeight_scale = d3.scale.linear()
                .domain([0, 1])
                .range([0, refAreaHeight]);
    
        var xAxis = d3.svg.axis().scale(x_scale).orient("bottom").tickSize(8).tickPadding(8);
        
        var chartClassName = "d3_annotationBox_";
        var chart = svg.attr("width", w + margin.right + margin.left)
                    .attr("height", h + margin.top + margin.bottom)
                    .attr("class", chartClassName);
         
        var boxArea = chart.append("g")
                    .attr("transform", "translate(" + (margin.left) + "," + (margin.top) + ")")
                    .attr("width", w)
                    .attr("height", boxAreaHeight)
                    .attr("class", "boxArea");
    
        var refArea;
        if (this.graphOptions.show_reference) {
            refArea= chart.append("g")
                        .attr("transform", "translate(" + margin.left + "," + (boxAreaHeight + margin.top) + ")")
                        .attr("width", w)
                        .attr("height", refAreaHeight)
                        .attr("class", "refArea");
        }
        chart.append("g")
             .attr("class", "x axis")
             .attr("transform", "translate(" + margin.left + "," + (h*0.95) + ")")
             .transition()
             .call(xAxis);
        
        
        
           var tip_id= "d3-tip-annotationBox";
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
                .html(function(d, ind, showFull) {
                  var toReturn = "<table style='font-family:courier;font-size:14px'>";
                  toReturn += "<tr><td><strong>Id</strong></td><td>: <span style='color:red'>" + d.id + "</span></td></tr>";
                  toReturn += "<tr><td><strong> Start</strong></td><td>: " + d.start + "</td></tr>";
                  toReturn += "<tr><td><strong>End</strong></td><td>: " + d.end + "</td></tr>";
                  var info = d3.dsv(";").parseRows(d["idType-id"])[0];
                  for (var it=0; it < info.length; ++it) {
                      var cur_length = info[it].length;
                      var split = info[it].split(":");
                      var type = split[0];
                      var id = split.slice(1).join("");
                      if (showFull) {
                          if (id.length > 100) {
                             var nlines = id.length/100;
                             for (var i=0; i<nlines-1; ++i) {
                                 var tt = type;
                                 var semiColon = ": ";
                                 if (i) {tt="  ";semiColon="  ";}
                                 toReturn += "<tr><td><strong>" + tt + "</strong></td><td>" + semiColon + id.substring(i*100,(i+1)*100) + "</span></td></tr>";
                             }
                             toReturn += "<tr><td><strong>" + tt + "</strong></td><td>" + semiColon + id.substring((nlines-1)*100) + "</span></td></tr>";
                          }
                          else {
                              toReturn += "<tr><td><strong>" + type + "</strong></td><td>: " + id + "</span></td></tr>";
                          }      
                      }
                      else 
                          toReturn += "<tr><td><strong>" + type + "</strong></td><td>: " + id.substring(0,25)+ ( (id.length > 25) ? ("...") : ("")) + "</span></td></tr>";
                  }
                  toReturn +="</table>";
                  return toReturn;
                });
            chart.call(tip);
    
        
        boxArea.append("g")
            .attr("transform", "translate(" + (0) + "," + (0) + ")")
            .selectAll("refAreaItems")
            .data(items)
            .enter().append("rect")
                .attr("class", function(d) {return "boxAreaItem" + d.lane;})
                .attr("rx", 3)
                .attr("ry", 3)
                .attr("fill",function(d,i) {
                    return d.color;
                })
                .attr("x", function(d) {
                    var start = d.start;
                    if (thiSS.graphOptions.fixedByMinMax){
                        start = (start - thiSS.min) <0 ? thiSS.min : start;
                    }
                    return x_scale(start);
                })
                .attr("y", function(d) {
                    return boxHeight_scale(nbOfLanes-1-d.lane);
                })
                .attr("width", function(d) {
                    var start = d.start;
                    var end = d.end;
                    if (thiSS.graphOptions.fixedByMinMax){
                        start = (start - thiSS.min) <0 ? thiSS.min : start;
                        end = (end - thiSS.max) >0 ? thiSS.max : end;
                    }
                    var width = x_scale(end) - x_scale(start);
                    if (!width)  width = 2.5;
                    return width;
                })
                .attr("height", function(d) {
                    var box_height = .8 * boxHeight_scale(1);
                    if (thiSS.graphOptions.boxSize) {
                        box_height = thiSS.graphOptions.boxSize;
                    }
                    return box_height;
                })
                .on("mouseover",function(d,i){
                    tip.show(d,i,thiSS.graphOptions.showTipFull);
                    this.style.strokeWidth= 2;
                    this.style.stroke= "black";
                })
                .on("mouseout",function(d,i){
                    tip.hide();
                    this.style.strokeWidth= 0;
                    this.style.stroke= d.color;
                })
                .on("click", function(d,i){
                    return thiSS.selectCallback ? thiSS.selectCallback(thiSS,d,i) : "";
                });
            
        
        if (this.graphOptions.showBoxLabel) {
            boxArea.append("g").selectAll(".refAreaLabels")
                .data(items)
                .enter().append("text")
                .text(function(d) {
                    return d.id + " [" + d.start + "-" + d.end + "]";
                })
                .attr("x", function(d) {return x_scale(d.start);})
                .attr("y", function(d) {
                    return boxHeight_scale((nbOfLanes-1-d.lane) + .5) ;
                })
                .attr("dy", ".5ex");
        }
        if (this.graphOptions.show_reference && this.myReference) {
             refArea.append("g").append("line")
                .attr("x1", margin.right)
                .attr("y1", refHeight_scale(0))
                .attr("x2", w)
                .attr("y2", refHeight_scale(0))
                .attr("stroke", "lightgray"); 
            
             refArea.append("g").append("line")
                .attr("x1", margin.right)
                .attr("y1", refHeight_scale(1))
                .attr("x2", w)
                .attr("y2", refHeight_scale(1))
                .attr("stroke", "lightgray"); 
        
            refArea.append("g").append("text").text("REFERENCE")
                .attr("x", -margin.right)
                .attr("y", refAreaHeight * .5)
                .attr("dy", ".5ex")
                .attr("text-anchor", "end")
                .attr("class", "laneText");
            
            refArea.append("g").append("rect")
                .data(verarr(this.myReference))
                .attr("class", "refAreaItem")
                .attr("rx", 5)
                .attr("ry", 5)
                .attr("x",  x_scale(this.myReference.start))
                .attr("y", refHeight_scale(0.2))
                .attr("width", x_scale(this.myReference.end) - x_scale(this.myReference.start))
                .attr("height", refHeight_scale(0.7))
                .on("mouseover",function(d,i){
                    tip.show(d,i, thiSS.graphOptions.showTipFull);
                    this.style.strokeWidth= 2;
                    this.style.stroke= "black";
                })
                .on("mouseout",function(d,i){
                    tip.hide();
                    this.style.strokeWidth= 0;
                    this.style.stroke= d.color;
                })
                .on("click", function(d,i){
                    return thiSS.selectCallback ? thiSS.selectCallback(thiSS,d,i) : "";
                });
        }
    
    };
}

