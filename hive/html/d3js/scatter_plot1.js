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
/*
var chocolates = [{
    "name": "Dairy Milk",
        "manufacturer": "cadbury",
        "price": 45,
        "rating": 2
}, {
    "name": "Galaxy",
        "manufacturer": "Nestle",
        "price": 42,
        "rating": 3
}, {
    "name": "Lindt",
        "manufacturer": "Lindt",
        "price": 80,
        "rating": 4
}, {
    "name": "Hershey",
        "manufacturer": "Hershey",
        "price": 40,
        "rating": 1
}, {
    "name": "Dolfin",
        "manufacturer": "Lindt",
        "price": 90,
        "rating": 5
}, {
    "name": "Bournville",
        "manufacturer": "cadbury",
        "price": 70,
        "rating": 2
}];
*/

function vjD3JS_ScatterPlot1 ( viewer )
{
    loadCSS("d3js/scatter_plot1.css");
    vjD3CategoryView.call(this,viewer); // inherit default behaviours of the DataViewer

    this.width=800;
    this.height=800;
    
    this.d3Compose=function(data){
        
        this.d3Compose_prv(data);
        var thiSS=this;
        var svg=this.d3area.append("svg");
        
        var id=this.id;
        this.xname= this.traits[0];
        this.yname= this.traits[1];
        
                
        // just to have some space around items. 
        var margins = {
            "left": 40,
                "right": 30,
                "top": 30,
                "bottom": 30
        };
        
        var width = this.width;
        var height = this.height;
        
        // this will be our colour scale. An Ordinal scale.
        //var colors = d3.scale.category10();

        // we add the SVG component to the scatter-load div
        svg.attr("width", width).attr("height", height).append("g")
            .attr("transform", "translate(" + margins.left + "," + margins.top + ")");

        // this sets the scale that we're using for the X axis. 
        // the domain define the min and max variables to show. In this case, it's the min and max prices of items.
        // this is made a compact piece of code due to d3.extent which gives back the max and min of the price variable within the dataset
        var x = d3.scale.linear()
            .domain(d3.extent(data, function (d) {
            return parseFloat(d[thiSS.xname]);
        }))
        // the range maps the domain to values from 0 to the width minus the left and right margins (used to space out the visualization)
            .range([0, width - margins.left - margins.right]);

        // this does the same as for the y axis but maps from the rating variable to the height to 0. 
        var y = d3.scale.linear()
            .domain(d3.extent(data, function (d) {
            //return d.rating;
            return parseFloat(d[thiSS.yname]);
        }))
        // Note that height goes first due to the weird SVG coordinate system
        .range([height - margins.top - margins.bottom, 0]);

        // we add the axes SVG component. At this point, this is just a placeholder. The actual axis will be added in a bit
        svg.append("g").attr("class", "x axis").attr("transform", "translate(0," + y.range()[0] + ")");
        svg.append("g").attr("class", "y axis");

        // this is our X axis label. Nothing too special to see here.
        svg.append("text")
            .attr("fill", "#414241")
            .attr("text-anchor", "end")
            .attr("x", width / 2)
            .attr("y", height - 35)
            .text("major vector");


        // this is the actual definition of our x and y axes. The orientation refers to where the labels appear - for the x axis, below or above the line, and for the y axis, left or right of the line. Tick padding refers to how much space between the tick and the label. There are other parameters too - see https://github.com/mbostock/d3/wiki/SVG-Axes for more information
        var xAxis = d3.svg.axis().scale(x).orient("bottom").tickPadding(2);
        var yAxis = d3.svg.axis().scale(y).orient("left").tickPadding(2);

        // this is where we select the axis we created a few lines earlier. See how we select the axis item. in our svg we appended a g element with a x/y and axis class. To pull that back up, we do this svg select, then 'call' the appropriate axis object for rendering.    
        svg.selectAll("g.y.axis").call(yAxis);
        svg.selectAll("g.x.axis").call(xAxis);

        // now, we can get down to the data part, and drawing stuff. We are telling D3 that all nodes (g elements with class node) will have data attached to them. The 'key' we use (to let D3 know the uniqueness of items) will be the name. Not usually a great key, but fine for this example.
        var chocolate = svg.selectAll("g.node").data(data, function (d) {
            return d[id];
        });

        // we 'enter' the data, making the SVG group (to contain a circle and text) with a class node. This corresponds with what we told the data it should be above.
        
        var chocolateGroup = chocolate.enter().append("g").attr("class", "node")
        // this is how we set the position of the items. Translate is an incredibly useful function for rotating and positioning items 
        .attr('transform', function (d) {
            return "translate(" + x(d[thiSS.xname]) + "," + y(d[thiSS.yname]) + ")";
        });
        
        //var arc = d3.svg.symbol().type('triangle-up').size(18);

        chocolateGroup.append("circle")
            .attr("class", "dot")
            .attr("r", 12)
            //.attr("d", d3.svg.symbol().type('rectangle'))//function (d,i) {return d3.svg.symbol().type('triangle-up');})
            //.attr("size",38)
            .style("fill", function (d) {return clr(d);})
            .on("mouseover", function (d){return thiSS.tooltipOn(d);} )
            .on("mousemove", function (d){return thiSS.tooltipMove(d);})
            //.on("mouseout", function (d){return thiSS.tooltipOff(d);} )
            ;
        
 
        // now we add some text, so we can see what each item is.
        chocolateGroup.append("text")
            .style("text-anchor", "middle")
            .attr("dy", -10)
            .text(function (d) {return d[id];})
            .style("font-size","14px");
        
        function clr(d){return thiSS.clr(d);}
        
                
    };
}




            

/*
thiSS.tooltip.transition()        
                    .duration(200)      
                    .style("opacity", .9);      
                thiSS.tooltip.html("dolomite")  
                    .style("left", (d3.event.pageX) + "px")     
                    .style("top", (d3.event.pageY - 28) + "px");    
                 
*/
