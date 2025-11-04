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
function vjD3JS_mahattan ( viewer )
{
    
    vjD3CategoryView.call(this,viewer);
    var divElement;
    var that = this;
    
    this.categToNumber=0;

    this.parseOptions = function () {
        this.options = (this.options == undefined) ? {} : this.options;
        this.options.isChromosome = (this.options.isChromosome==undefined) ? false : this.options.isChromosome;
        
        this.options.title = (this.options.title == undefined) ? {} : this.options.title;
        
        this.options.title.h = this.options.title.x ? this.options.title.x : ( this.options.title.horizontal ? this.options.title.horizontal : "Axis");
        this.options.title.v = this.options.title.y ? this.options.title.y : ( this.options.title.vertical ? this.options.title.vertical : "Axis");

    }

    this.composeToolTips = function () {
           var tip_id= "d3-tip-mahattan";
           
           if (this.graphId) {  tip_id+= "_" + this.graphId;}
           
           var tipObj = gObject(tip_id);
           if (tipObj) {
               tipObj.remove();
           }
           
           this.toolTip = d3.tooltip()
            .attr('class', 'd3-tip')
            .attr("id", tip_id)
            .offset([-10, 0])
            .html(function(d, bb) {
              var toReturn = "";
              var keyarr=Object.keys(d.info);
              for (var i=0; i<keyarr.length; ++i) {
                  if (i==0) toReturn+="</br>";
                  toReturn = "<strong>"+ keyarr[i].toUpperCase() +":</strong>" + d.info[keyarr[i]];
              }
              return toReturn;
            })
            .style({"background": "#FAF5F5","color": "#544343","border": "1px","border-radius": "2px","border-color": "black","box-shadow": "3px 3px 1px #888888"});

    }
    
    this.generateChromosomeGroups = function(data_objs) {
        var group_names=[];
        for (var ichr=1; ichr<23; ++ichr) {
            var chr = ichr.toString();
            group_names.push(chr);
            if (data_objs.minMax[chr]==undefined) {
                data_objs.minMax[chr]={min:0, max:1};
            }
        }
        var toAdd=["X","Y","MT",""];
        for (var ichr=0; ichr<toAdd.length; ++ichr) {
            group_names.push(toAdd[ichr]);
            data_objs.minMax[toAdd[ichr]]={min:0, max:1};
        }
        return group_names;
    }
    
    this.d3Compose=function(data){
        
        function data_treatment(data, cols, isLog10) {
            var minMax={};
            var group_data=[];
            var yMax = Number.MIN_VALUE;

            var group_names=[];
            for (var ir=0; ir<data.length; ++ir) {
                var chr=data[ir][cols["group"]];
                var pos=Number(data[ir][cols["pos"]]);
                var val=Number(data[ir][cols["val"]]);;
                var info={};
                for (var i=0; i<cols["info"].length; ++i) {
                    info[cols["info"][i]]=data[ir][cols["info"][i]]
                }

                if (isLog10 != undefined && isLog10) {
                    val=Math.log10(val);
                }
                group_data.push({"group":chr,"pos":pos,"val":val,"info":info});
                if (minMax[chr] == undefined) {
                    group_names.push(chr);
                    minMax[chr]={"min": Number.MAX_VALUE,"max": Number.MIN_VALUE}
                }
                if (pos < minMax[chr].min) {
                    minMax[chr].min=pos;
                }
                if (pos> minMax[chr].max) {
                    minMax[chr].max=pos;
                }
                if (val>yMax) {
                    yMax=val;
                }
            }
            return {y: yMax, minMax:minMax,data:group_data,group_names:group_names};
        }
        this.tmpRandom = this.originalRandom;
        this.d3Compose_prv(data);
        
        var svg=this.d3svg;
        
        if (this.container) {
            divElement = gObject(this.container); 
            if (this.width == undefined || !this.width){this.width = parseInt(divElement.style.maxWidth);}
            if (this.height == undefined || !this.height){this.height = parseInt(divElement.style.maxHeight);}
            if (!this.height) this.height=400;
            if (!this.width) this.width=800;
        }
        
        this.parseOptions();
        this.composeToolTips();

        var margin = { top: 20, right: 20, bottom: 30, left: 40 };
        var width = 1260 - margin.left - margin.right;
        var height = 500 - margin.top - margin.bottom;
        var color = d3.scale.category20();

        var data_objects = data_treatment(data, this.cols,this.options.isLog10);
        
        var group_names;
        var totg;
        
        if (this.options.isChromosome) {
            group_names = this.generateChromosomeGroups(data_objects);            
        } else {
            group_names=data_objects.group_names;
            group_names.push("");
            totg=group_names.length-1;
        }
        
        var xscale_groups = d3.scale.ordinal().domain(group_names).rangePoints([0,width]);
        
        var scale_per_group={};
        var tick_vals=[];

        var padding=10;
        for (var ik=0; ik<group_names.length; ++ik) {
            var k=group_names[ik];
            if (k==""){
                continue;
            }
            var k_1=group_names[ik+1];
            var cur_scale = d3.scale.linear()
                                     .domain([data_objects.minMax[k].min,data_objects.minMax[k].max])
                                     .range([xscale_groups(k) + padding,xscale_groups(k_1) - padding]);

            scale_per_group[k]=cur_scale;
            tick_vals.push(k);
        
        }
        
        var y = d3.scale.linear()
                    .range([height, 0])
                    .domain([0,data_objects.y]);

        var xAxis = d3.svg.axis().scale(xscale_groups).orient('bottom');
        var yAxis = d3.svg.axis().scale(y).orient('left');
        

        svg.attr('width', width + margin.left + margin.right)
               .attr('height', height + margin.top + margin.bottom);
               
         var gg=svg.append('g')
               .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')');
            gg.append("g")
                .attr("class", "x_axis")
                .attr("transform", "translate(0," + height + ")")
                .style({ 'stroke': 'black', 'fill': 'none', 'stroke-width': '1px',"font-size":"11px"})
                .call(xAxis)
                .append("text")
                    .attr("class", "label x")
                     .attr("x", (width *0.95))
                     .attr("y", margin.bottom)
                    .text(this.options.title.h);
            
            gg.append("g")
                .attr("class", "y_axis")
                .attr("dy", ".5em")
                .style({ 'stroke': 'black', 'fill': 'none', 'stroke-width': '1px',"font-size":"11px"})
                .call(yAxis)
                .append("text")
                .attr("class", "label y")
                    .attr("transform", "rotate(-90)")
                    .attr("x", 0-height/2)
                    .attr("y", 0 - (margin.left ))
                    .attr("dy", ".71em")
                    .style("text-anchor", "end")
                    .text(this.options.title.v);
            
            gg.selectAll(".x_axis text").attr("dx",(xscale_groups(group_names[1]) - xscale_groups(group_names[0])) *0.5);
            gg.call(that.toolTip);
            var chromosome = gg.selectAll(".chr")
                                    .data(data_objects.data)
                                        .enter()
                                            .append("g")                                            
                                            .attr("class", "chr");

            var circles = chromosome.selectAll("circle")
                            .data(data_objects.data)
                            .enter().append("circle")
                                .style("fill",function(d){
                                    return color(d["group"]);
                                })
                                .attr("r", 3)
                                .attr("cx", function (d) {
                                    return scale_per_group[d["group"]](d["pos"]);
                                })
                                .attr("cy", function (d) {
                                    return y(d["val"]);
                                })
                                .on("mouseover", function(d){
                                    that.toolTip.show(d);
                                })
                                .on("mouseout", function() {
                                    that.toolTip.hide(d);
                                });
    };
}

