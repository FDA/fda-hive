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

function vjD3JS_StackableGroupBars ( viewer )
{
    vjD3View.call(this,viewer);

    if (!viewer.width) this.width=1200;
    else this.width = viewer.width;
    
    if (!viewer.height) this.height=900;
    else this.height = viewer.height;
    
    if(!viewer.margin) this.margin={top: 40, right: 100, bottom: 20, left: 20};
    else this.margin=viewer.margin;
        
    if (viewer.cols) this.cols=verarr(viewer.cols);
    if (viewer.notCols) this.notCols=verarr(viewer.notCols);
    if (viewer.labelCol) this.labelCol=verarr(viewer.labelCol);
    
    var max=Number.MIN_VALUE, min=Number.MAX_VALUE;
    
    this.d3Compose=function(data){
        
        this.d3Compose_prv(data);

        if (!this.cols && data.length > 0)    
        {
            this.cols=[];
            for (var colElem in data[0])
            {
                if (this.notCols && this.notCols.indexOf(colElem) > -1)
                    continue;
                this.cols.push ({name: colElem});
            }
        }        
        var cols = this.cols;
        var svg=this.d3svg;
        

        var n = this.cols.length,
            m = data.length,
            stack = d3.layout.stack(),
            layers = stack(d3.range(n).map(function(d) { 
                    return bumpLayer(data, cols[d].name ); 
                })),
            yGroupMax = d3.max(layers, function(layer) { return d3.max(layer, function(d) { return d.y; }); }),
            yStackMax = d3.max(layers, function(layer) { return d3.max(layer, function(d) { return d.y0 + d.y; }); }),
            yStackMin = d3.min(layers, function(layer) { return d3.min(layer, function(d) { return d.y0 + d.y; }); });
            

        var margin = this.margin, 
            width = this.width - margin.left - margin.right,
            height = this.height-100 - margin.top - margin.bottom;

        var x = d3.scale.ordinal()
            .domain(d3.range(m))
            .rangeRoundBands([0, width], .08);

        var y = d3.scale.linear()
            .domain([yStackMin, yStackMax])
            .range([height, 0]);
        
        var labelCol = this.labelCol;
        if (!labelCol)
            labelCol = -1;
        var label = [];
        for (var i = 0; i < data.length; i++)
        {
            if (labelCol == -1)
                label.push (i);
            label.push (data[i][this.labelCol]);
        }
        var minVal = 0, maxVal = 0;
        if (Math.abs(min) > Math.abs(max) && min < 0)
        {
            minVal = min;
            maxVal = -min;
        }
        else if (min < 0)
        {
            maxVal = max;
            minVal = -max;
        }
        else
            maxVal = max;
        
        var graphHeight = maxVal-minVal;
        var coef = height / graphHeight;
        var colHeaders = [];
        if (data.length > 0)
        {
            for (var curElem in data[0])
            {
                for (var i = 0; i < this.cols.length; i++)
                {
                    if (curElem == this.cols[i].name)
                    {
                        if (this.cols[i].title)
                            colHeaders.push(this.cols[i].title);
                        else
                            colHeaders.push(curElem);
                        
                        break;
                    }
                }
            }
        }
        
        
        var color = d3.scale.linear()
            .domain([0, n - 1])
            .range(["#0d47a1", "#90caf9"]);

        var otherX = d3.scale.ordinal()
            .domain(label)
            .rangeRoundBands([0, width], .08);
        
        var otherY = d3.scale.linear()
            .domain([minVal, maxVal])
            .range([height, 0]);
            
        var xAxis = d3.svg.axis()
            .scale(otherX)
            .tickSize(5)
            .tickPadding(6)
            .orient("bottom");
        
        var yAxis = d3.svg.axis()
            .scale(otherY)
            .ticks(5)
            .orient("right");
        
        this.d3area.append("label")
            .text("Grouped")
            .append("input")
                .attr("type", "radio")
                .attr("name","mode")
                .attr("value","grouped");
        
        this.d3area.append("label")
            .text("Stacked")
            .append("input")
                .attr("type", "radio")
                .attr("name","mode")
                .attr("value","stacked")
                .attr("checked");
            
        svg.attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom)
            .append("g")
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
        
        
        var layer = svg.selectAll(".layer")
            .data(layers)
          .enter().append("g")
            .attr("class", "layer")
            .style("fill", function(d, i) { return color(i); });

        var rect = layer.selectAll("rect")
            .data(function(d) { return d; })
          .enter().append("rect")
            .attr("x", function(d) { return x(d.x); })
            .attr("y", height)
            .attr("width", x.rangeBand())
            .attr("height", 0);

        rect.transition()
            .delay(function(d, i) { return i * 10; })
            .attr("y", function(d) { 
                return Math.abs(y(d.y0 + d.y)); })
            .attr("height", function(d) { 
                return Math.abs(y(d.y0) - y(d.y0 + d.y)); });

        svg.append("g")
            .attr("class", "x axis")
            .attr("transform", "translate(0," + height + ")")
            .call(xAxis);
        svg.append("g")
            .attr("class", "axis")
            .attr("transform", "translate(0,0)")
            .call(yAxis);

        var size = 0;
        
        var legend = svg.selectAll("g.legend")
            .data(colHeaders)
            .enter().append("svg:g")
            .attr("class", "legend")
            .attr("transform", function(d, i) {
                return "translate("+width+"," + (n*size+36+i*24) + ")";});
        
        legend.append("svg:circle")
            .style("fill", function (d,i ) {
                return color(i);})
            .attr("r", 5);
        
        legend.append("svg:text")
            .attr("x", 12)
            .attr("dy", ".31em")
            .text(function(d) {return d;});
            
        d3.selectAll("input").on("change", change);

        var timeout = setTimeout(function() {
          d3.select("input[value=\"grouped\"]").property("checked", true).each(change);
        }, 2000);

        function change() {
          clearTimeout(timeout);
          if (this.value === "grouped") transitionGrouped();
          else transitionStacked();
        }

        function transitionGrouped() {
          y.domain([0, yGroupMax]);

          rect.transition()
              .duration(500)
              .delay(function(d, i) { return i * 10; })
              .attr("x", function(d, i, j) { 
                  return x(d.x) + x.rangeBand() / n * j; })
              .attr("width", x.rangeBand() / n)
            .transition()
              .attr("y", function(d) { 
                  if (d.y < 0)
                      return height/2;
                  if (min < 0)
                      return (graphHeight/2-d.y) * coef
                  return height - d.y*coef; })
              .attr("height", function(d) {
                  if (d.y < 0)
                      return -d.y * coef;
                  return d.y * coef;});
        }

        function transitionStacked() {
          y.domain([0, yStackMax]);

          rect.transition()
              .duration(500)
              .delay(function(d, i) { return i * 10; })
              .attr("y", function(d) { 
                  return y(d.y0 + d.y); })
              .attr("height", function(d) { 
                  return y(d.y0) - y(d.y0 + d.y); })
            .transition()
              .attr("x", function(d) { 
                  return x(d.x); })
              .attr("width", x.rangeBand());
        }

        function bumpLayer(data, nm) {

            var a=[];
             for ( var i=0; i<data.length; ++i) {
                 var val = parseFloat(data[i][nm]);

                 a[i]={x: i, y: val, label:nm }; 
                if (val < min)
                    min = val;
                if (val > max)
                    max = val;
             }    
             return a;
        }
    };
}




            

