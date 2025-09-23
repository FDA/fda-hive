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
function vjD3JS_CorrelationView ( viewer )
{
    loadCSS("d3js/css/scatter_correlations.css");
    vjD3CategoryView.call(this,viewer);

    this.blocksize= 180;
    this.padding=10;
    
    
    
    this.d3Compose=function(flowers){
        
        this.d3Compose_prv(flowers);
        var thiSS=this;
        var svg=this.d3svg;
        
                
        var size = this.blocksize,
        padding = this.padding,
        n = this.traits.length,
        species=this.species,
        categories=this.categories,
        categoryColors=this.categoryColors,
        colorshift=this.colorshift;
        
        var thiSS=this;
        
            
        var x = {}, y = {};
        this.traits.forEach(function(trait) {
            flowers.forEach(function(d) {d[trait] = +d[trait];});

            var value = function(d) {return d[trait];},
            domain = [d3.min(flowers, value), d3.max(flowers, value)],
            range = [padding / 2, size - padding / 2];
            x[trait] = d3.scale.linear().domain(domain).range(range);
            y[trait] = d3.scale.linear().domain(domain).range(range);
        });
        
        var axis = d3.svg.axis()
            .ticks(5)
            .tickSize(size * n);
        
        var brush = d3.svg.brush()
            .on("brushstart", brushstart)
            .on("brush", brush)
            .on("brushend", brushend);
        
        svg.attr("width", 1280*size/140)
            .attr("height", 800*size/140)
            .append("svg:g");
        
        var legend = svg.selectAll("g.legend")
            .data(this.categories)
            .enter().append("svg:g")
            .attr("class", "legend")
            .attr("transform", function(d, i) {return "translate("+24+"," + (n*size+36+i*24) + ")";});
        
        legend.append("svg:circle")
            .style("fill", function (d,i ) {return categoryColors[(colorshift+i)%categoryColors.length];})
            .attr("r", 5);
        
        legend.append("svg:text")
            .attr("x", 12)
            .attr("dy", ".31em")
            .text(function(d) {return d;});
        this.legend = legend;
        
        svg.selectAll("g.x.axis")
            .data(this.traits)
            .enter().append("svg:g")
            .attr("class", "x axis")
            .attr("transform", function(d, i) {return "translate(" + i * size + ",0)";})
            .each(function(d) {d3.select(this).call(axis.scale(x[d]).orient("bottom"));});
        
        svg.selectAll("g.y.axis")
            .data(this.traits)
            .enter().append("svg:g")
            .attr("class", "y axis")
            .attr("transform", function(d, i) {return "translate(0," + i * size + ")";})
            .each(function(d) {d3.select(this).call(axis.scale(y[d]).orient("right"));});
        
        var cell = svg.selectAll("g.cell")
            .data(cross(this.traits, this.traits))
            .enter().append("svg:g")
            .attr("class", "cell")
            .attr("transform", function(d) {return "translate(" + d.i * size + "," + d.j * size + ")";})
            .each(plot);
        
        cell.filter(function(d) {return d.i == d.j;}).append("svg:text")
            .attr("x", padding)
            .attr("y", padding)
            .attr("dy", ".71em")
            .text(function(d) {return d.x;});
        
        function plot(p) {
            var cell = d3.select(this);
        
            cell.append("svg:rect")
                .attr("class", "frame")
                .attr("x", padding / 2)
                .attr("y", padding / 2)
                .attr("width", size - padding)
                .attr("height", size - padding);
            
            cell.selectAll("circle")
                .data(flowers)
                .enter().append("svg:circle")
                .style("fill", clr)
                .attr("cx", function(d) {return x[p.x](d[p.x]);})
                .attr("cy", function(d) {return y[p.y](d[p.y]);})
                .attr("r", 5)
                .on("mouseover", function (d){return thiSS.tooltipOn(d);} )
                .on("mousemove", function (d){return thiSS.tooltipMove(d);})
                ;
            
            
            if(this.showlabels) { 
                cell.selectAll("pointlabels")
                .data(flowers)
                .enter().append("svg:text")                
                .attr("x", function(d) {return x[p.x](d[p.x]);})
                .attr("y", function(d) {return y[p.y](d[p.y]);})
                .text(function(d) {return d[species];})
            }
        
            cell.call(brush.x(x[p.x]).y(y[p.y]));
        }

        function clr(d){
            return thiSS.clr(d);}
        function brushstart(p) {
            if (brush.data !== p) {
                cell.call(brush.clear());
                brush.x(x[p.x]).y(y[p.y]).data = p;
            }
        }

        function brush(p) {
            var e = brush.extent();
            svg.selectAll(".cell circle").style("fill", function(d) {
                return e[0][0] <= d[p.x] && d[p.x] <= e[1][0]
                && e[0][1] <= d[p.y] && d[p.y] <= e[1][1]
                ? clr(d) : null;
            });
        }


        function brushend() {
            if (brush.empty()) svg.selectAll(".cell circle").style("fill", clr);
        }

        function cross(a, b) {
            var c = [], n = a.length, m = b.length, i, j;
            for (i = -1; ++i < n;) for (j = -1; ++j < m;) c.push( {x: a[i], i: i, y: b[j], j: j});
            return c;
        }
    };
}





function vjLegendSVG (viewer){
    loadCSS("d3js/css/scatter_correlations.css");
    vjD3CategoryView.call(this,viewer);
    
    this.blocksize= 180;
    this.padding=10;
    

    this.d3Compose=function(flowers, content){            
        this.d3Compose_prv(flowers);
        var thiSS=this;
        var svg=this.d3svg;
                
        var size = this.blocksize,
        padding = this.padding,
        n = this.traits.length,
        species=this.species,
        categories=this.categories,
        categoryColors=this.categoryColors,
        colorshift=this.colorshift;
        
        var thiSS=this;
        
            
        var x = {}, y = {};
        this.traits.forEach(function(trait) {
            flowers.forEach(function(d) {d[trait] = +d[trait];});

            var value = function(d) {return d[trait];},
            domain = [d3.min(flowers, value), d3.max(flowers, value)],
            range = [padding / 2, size - padding / 2];
            x[trait] = d3.scale.linear().domain(domain).range(range);
            y[trait] = d3.scale.linear().domain(domain).range(range);
        });
        
        var axis = d3.svg.axis()
            .ticks(5)
            .tickSize(size * n);
        
        var brush = d3.svg.brush()
            .on("brushstart", brushstart)
            .on("brush", brush)
            .on("brushend", brushend);
        
        svg.attr("width", 640)
            .attr("height",this.categories.length*24+36+n)
            .append("svg:g");
        
        var legend = svg.selectAll("g.legend")
            .data(this.categories)
            .enter().append("svg:g")
            .attr("class", "legend")
            .attr("transform", function(d, i) {return "translate("+24+"," + (n+36+i*24) + ")";});
        
        legend.append("svg:circle")
            .style("fill", function (d,i ) {return categoryColors[(colorshift+i)%categoryColors.length];})
            .attr("r", 5);
        
        legend.append("svg:text")
            .attr("x", 12)
            .attr("dy", ".31em")
            .text(function(d) {return d;});
    }
    
    this.clr = function (d) {return thiSS.clr(d);}
    function brushstart(p) {
        if (brush.data !== p) {
            cell.call(brush.clear());
            brush.x(x[p.x]).y(y[p.y]).data = p;
        }
    }

    function brush(p) {
        var e = brush.extent();
        svg.selectAll(".cell circle").style("fill", function(d) {
            return e[0][0] <= d[p.x] && d[p.x] <= e[1][0]
            && e[0][1] <= d[p.y] && d[p.y] <= e[1][1]
            ? thiSS.clr(d) : null;
        });
    }


    function brushend() {
        if (brush.empty()) svg.selectAll(".cell circle").style("fill", thiSS.clr);
    }

    function cross(a, b) {
        var c = [], n = a.length, m = b.length, i, j;
        for (i = -1; ++i < n;) for (j = -1; ++j < m;) c.push( {x: a[i], i: i, y: b[j], j: j});
        return c;
    }
    
    return this;
}

function vjScatterLegendSeparate (viewer){
    if (this.objCls) return;

    
    this.scatter = new vjD3JS_CorrelationView (viewer);
    this.legend = new vjLegendSVG (viewer);
    
    return [this.scatter, this.legend];    
}




            

