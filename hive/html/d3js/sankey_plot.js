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

    
function vjD3JS_SankeyView ( viewer )
{
    loadCSS("d3js/css/sankey_plot.css");
    vjD3CategoryView.call(this,viewer);

    this.width=1200;
    this.height=740;
    this.maxContributors=13;

    
    this.d3Compose=function(data){
            
        this.d3Compose_prv(data);
        var thiSS=this;
        var svg=this.d3svg;
        
        var units = "Widgets";
         
        var margin = {top: 10, right: 10, bottom: 10, left: 10},
            width = this.width- margin.left - margin.right,
            height = this.height - margin.top - margin.bottom;
         
        var formatNumber = d3.format(",.0f"),
            format = function(d) { return formatNumber(d) + " " + units; },
            color = d3.scale.category20();
         
        svg.attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom)
          .append("g")
            .attr("transform", 
                  "translate(" + margin.left + "," + margin.top + ")");
         
        var sankey = d3.sankey()
            .nodeWidth(36)
            .nodePadding(10)
            .size([width, height]);
         
        var path = sankey.link();
        var graph=data;
         
        {
             
            var nodeMap = {};
            graph.nodes.forEach(function(x) { nodeMap[x.name] = x; });
            graph.links = graph.links.map(function(x) {
              return {
                source: nodeMap[x.source],
                target: nodeMap[x.target],
                value: x.value
              };
            });
         
          sankey
              .nodes(graph.nodes)
              .links(graph.links)
              .layout(32);
         
          var link = svg.append("g").selectAll(".link")
              .data(graph.links)
            .enter().append("path")
              .attr("class", "link")
              .attr("d", path)
              .style("stroke-width", function(d) { return Math.max(1, d.dy); })
              .sort(function(a, b) { return b.dy - a.dy; });
         
          link.append("title")
                .text(function(d) {
                  return d.source.name + " → " + 
                        d.target.name + "\n" + format(d.value); });
         
          var node = svg.append("g").selectAll(".node")
              .data(graph.nodes)
            .enter().append("g")
              .attr("class", "node")
              .attr("transform", function(d) { 
                  return "translate(" + d.x + "," + d.y + ")"; })
            .call(d3.behavior.drag()
              .origin(function(d) { return d; })
              .on("dragstart", function() { 
                  this.parentNode.appendChild(this); })
              .on("drag", dragmove));
         
          node.append("rect")
              .attr("height", function(d) { return d.dy; })
              .attr("width", sankey.nodeWidth())
              .style("fill", function(d) { 
                  return d.color = color(d.name.replace(/ .*/, "")); })
              .style("stroke", function(d) { 
                  return d3.rgb(d.color).darker(2); })
            .append("title")
              .text(function(d) { 
                  return d.name + "\n" + format(d.value); });
         
          node.append("text")
              .attr("x", -6)
              .attr("y", function(d) { return d.dy / 2; })
              .attr("dy", ".35em")
              .attr("text-anchor", "end")
              .attr("transform", null)
              .text(function(d) { return d.name; })
            .filter(function(d) { return d.x < width / 2; })
              .attr("x", 6 + sankey.nodeWidth())
              .attr("text-anchor", "start");
         
          function dragmove(d) {
            d3.select(this).attr("transform", 
                "translate(" + (
                       d.x = Math.max(0, Math.min(width - d.dx, d3.event.x))
                    ) + "," + (
                           d.y = Math.max(0, Math.min(height - d.dy, d3.event.y))
                    ) + ")");
            sankey.relayout();
            link.attr("d", path);
          }
        }
        
    }
        
}



     
    