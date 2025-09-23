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
function vjD3JS_ScatterPlot1 ( viewer )
{
    loadCSS("d3js/scatter_plot1.css");
    vjD3CategoryView.call(this,viewer);

    this.width=800;
    this.height=800;
    
    this.d3Compose=function(data){
        
        this.d3Compose_prv(data);
        var thiSS=this;
        var svg=this.d3area.append("svg");
        
        var id=this.id;
        this.xname= this.traits[0];
        this.yname= this.traits[1];
        
                
        var margins = {
            "left": 40,
                "right": 30,
                "top": 30,
                "bottom": 30
        };
        
        var width = this.width;
        var height = this.height;
        

        svg.attr("width", width).attr("height", height).append("g")
            .attr("transform", "translate(" + margins.left + "," + margins.top + ")");

        var x = d3.scale.linear()
            .domain(d3.extent(data, function (d) {
            return parseFloat(d[thiSS.xname]);
        }))
            .range([0, width - margins.left - margins.right]);

        var y = d3.scale.linear()
            .domain(d3.extent(data, function (d) {
            return parseFloat(d[thiSS.yname]);
        }))
        .range([height - margins.top - margins.bottom, 0]);

        svg.append("g").attr("class", "x axis").attr("transform", "translate(0," + y.range()[0] + ")");
        svg.append("g").attr("class", "y axis");

        svg.append("text")
            .attr("fill", "#414241")
            .attr("text-anchor", "end")
            .attr("x", width / 2)
            .attr("y", height - 35)
            .text("major vector");


        var xAxis = d3.svg.axis().scale(x).orient("bottom").tickPadding(2);
        var yAxis = d3.svg.axis().scale(y).orient("left").tickPadding(2);

        svg.selectAll("g.y.axis").call(yAxis);
        svg.selectAll("g.x.axis").call(xAxis);

        var chocolate = svg.selectAll("g.node").data(data, function (d) {
            return d[id];
        });

        
        var chocolateGroup = chocolate.enter().append("g").attr("class", "node")
        .attr('transform', function (d) {
            return "translate(" + x(d[thiSS.xname]) + "," + y(d[thiSS.yname]) + ")";
        });
        

        chocolateGroup.append("circle")
            .attr("class", "dot")
            .attr("r", 12)
            .style("fill", function (d) {return clr(d);})
            .on("mouseover", function (d){return thiSS.tooltipOn(d);} )
            .on("mousemove", function (d){return thiSS.tooltipMove(d);})
            ;
        
 
        chocolateGroup.append("text")
            .style("text-anchor", "middle")
            .attr("dy", -10)
            .text(function (d) {return d[id];})
            .style("font-size","14px");
        
        function clr(d){return thiSS.clr(d);}
        
                
    };
}