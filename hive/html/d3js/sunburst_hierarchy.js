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
function vjD3JS_SunburstHierarchy( viewer )
{
    var thiSS = this;
    
        loadCSS("d3js/css/sunburst_hierarchy.css");
        vjD3View.call(this,viewer);
        
        if (viewer.width) this.width=viewer.width; else this.width=750;
        if (viewer.height) this.height=viewer.height; else this.height=600;
        
        viewer.colPath ? this.colPath = viewer.colPath :this.colPath="path";
        viewer.colCount ? this.colCount = viewer.colCount : this.colCount="matchCnt";
        viewer.colTaxid ? this.colTaxid = viewer.colTaxid : this.colTaxid="taxid";
        viewer.colorshift ? this.colorshift = viewer.colorshift : this.colorshift=3;
        viewer.seedcolor ? this.seedcolor = viewer.seedcolor : this.seedcolor = randRangeColor(40, 100);
        viewer.categClrDensity ? this.categClrDensity = viewer.categClrDensity : this.categClrDensity=0;
        viewer.maxLevel ? this.maxLevel = viewer.maxLevel : this.maxLevel = 0;
        this.colorCol = viewer.colorCol;
        
        this.categClrArr={};
        this.click = viewer.funclick;


    this.d3Compose=function(data){
                
        this.d3Compose_prv(data);
        
        var trail= this.d3svg.append("g").attr("id","trail");
        
        var json = buildHierarchy(data,this.colPath,this.colCount, this.colTaxid);
        
        var width = this.width;
        var height = this.height;
        var radius = Math.min(width, height-80) / 2  ;

        var b = {
          w: 95, h: 30, s: 3, t: 10
        };

        function clr(d,i){
            if(!d.color){
                    return 'lightgray';
            }
            var shade = 2*d.lev_depth/thiSS.maxLevel;
            var max_lev = 0.75;
            
            var colorCode = d.color;
            var tmpColor = new vjColor(colorCode);
            if(tmpColor.lightness() < 0.4)
                    return colorCode;
            
            return lightenColor (colorCode,1-(1./(1.1+shade)));
        }

        
        
        var totalSize = 0; 

        var vis = this.d3svg
            .attr("width", width)
            .attr("height", height)
            .append("svg:g")
            .attr("id", "container")
            .attr("transform", "translate(" + width / 2 + "," + height / 2 + ")");
        var explanation= vis.append("g").attr("id","explanation");
        var perc = explanation.append("svg:text").attr("id","percentage")
            .attr("transform", "translate(-40,10)");

        var partition = d3.layout.partition()
            .size([2 * Math.PI, radius * radius])
            .value(function(d) { return d.size; });

        var arc = d3.svg.arc()
            .startAngle(function(d) { return d.x; })
            .endAngle(function(d) { return d.x + d.dx; })
            .innerRadius(function(d) { return Math.sqrt(d.y); })
            .outerRadius(function(d) { return Math.sqrt(d.y + d.dy); });

        

          initializeBreadcrumbTrail();

          vis.append("svg:circle")
              .attr("r", radius)
              .style("opacity", 0);

          var nodes = partition.nodes(json)
              .filter(function(d) {
              return (d.dx > 0.005);
              });
          
          var maxDepth = 0;
          for(var i = 0; i < nodes.length; i++){
              if (maxDepth < nodes[i].depth) maxDepth = nodes[i].depth;
          }
          
          if(maxDepth >= 30)
              $(perc[0]).css("font-size", "1em");
          else if(maxDepth >= 20)
              $(perc[0]).css("font-size", "1.5em");
          else if(maxDepth >= 8)
              $(perc[0]).css("font-size", "2em");


          var path = vis.data([json]).selectAll("path")
              .data(nodes)
              .enter().append("svg:path")
              .attr("display", function(d) { return d.depth ? null : "none"; })
              .attr("d", arc)
              .attr("fill-rule", "evenodd")
              .style("fill", function(d,i) {return clr(d,i);})
              .style("opacity", 1)
                .on("click", extclick)
              .on("mouseover", mouseover);

          d3.select("#container").on("mouseleave", mouseleave);

          totalSize = path.node().__data__.value;
         

        function extclick(node){
            if(thiSS.click)
                funcLink( thiSS.click, this, node );
        }

        function mouseover(d) {

          var percentage = (100 * d.value / totalSize).toPrecision(3);
          var percentageString = percentage + "%";
          if (percentage < 0.1) {
            percentageString = "< 0.1%";
          }
        
          d3.select("#percentage")
              .text(percentageString);

          d3.select("#explanation")
              .style("visibility", "");

          var sequenceArray = getAncestors(d);
          updateBreadcrumbs(sequenceArray, percentageString);

          d3.selectAll("path")
              .style("opacity", 0.3);

          vis.selectAll("path")
              .filter(function(node) {
                        return (sequenceArray.indexOf(node) >= 0);
                      })
              .style("opacity", 1);
        }

        function mouseleave(d) {

          d3.select("#trail")
                .style("visibility", "hidden");

          d3.selectAll("path").on("mouseover", null);

          d3.selectAll("path")
              .transition()
              .duration(1000)
              .style("opacity", 1)
              .each("end", function() {
                      d3.select(this).on("mouseover", mouseover);
                    });

          d3.select("#explanation")
          .style("visibility", "hidden");
        }

        function getAncestors(node) {
          var path = [];
          var current = node;
          while (current.parent) {
            path.unshift(current);
            current = current.parent;
          }
          return path;
        }

        function initializeBreadcrumbTrail() {
          trail.attr("width", width)
              .attr("height", 50)
              .attr("font-weight", 600)
              .style("fill", "#000");
          trail.append("svg:text")
            .attr("id", "endlabel")
            .style("fill", "#000");
        }

        function breadcrumbPoints(d, i) {
          var points = [];
          points.push("0,0");
          points.push(b.w + ",0");
          points.push(b.w + b.t + "," + (b.h / 2));
          points.push(b.w + "," + b.h);
          points.push("0," + b.h);
          if (i > 0) {
            points.push(b.t + "," + (b.h / 2));
          }
          return points.join(" ");
        }

        function updateBreadcrumbs(nodeArray, percentageString) {

                var Nout=5;
                var printable=nodeArray.length-Nout+1;
                var space = 1;
                if (printable < 2){
                    space = 0;
                }
                if(printable<1){
                    printable=1;
                }
          var g = trail.selectAll("g")
                    .data(nodeArray, function(d) { return d.shortname + d.depth; });

          var entering = g.enter().append("svg:g");

          entering.append("svg:polygon")
              .attr("points", breadcrumbPoints)
              .style("fill", function(d,i) {return clr(d);});

          entering.append("svg:text")
              .attr("x", (b.w + b.t) / 2)
              .attr("y", b.h / 2)
              .attr("dy", "0.35em")
              .attr("text-anchor", "middle")
              .text(function(d) { return d.shortname; })
              .style("font-size","10px");;

          g.attr("transform", function(d, i) {
                  if(d.lev_depth==1)
                          return "translate(0, 0)";
                  else if(d.lev_depth>printable)
                                return "translate(" + ((d.lev_depth-printable) * (b.w + b.s)+ (space)*b.w/2) + ", 0)";
                  else return "translate(-200, 0)";
          });

          g.exit().remove();

          if (Nout > nodeArray.length){
              Nout = nodeArray.length;
          }
         
          trail.select("#endlabel")
              .attr("x", (Nout+0.5) * (b.w + b.s) + (space)*b.w/2)
              .attr("y", b.h / 2)
              .attr("dy", "0.35em")
              .attr("text-anchor", "middle")
              .text(percentageString);

                trail.style("visibility", function (d,i){
                          return "";
                });

        }

                function buildHierarchy(csv, colPath, colCount, colTaxid) {
                        var colorNum = 0;
                        
                        var root = {
                                "name" : "root",
                                "shortname" : "root",
                                "children" : [],
                                "inGroupOrder":0,
                                "lev_depth":0,
                                "color": thiSS.seedcolor
                        };
                        for (var i = 0; i < csv.length; i++) {
                                var sequence = csv[i][colPath];
                                var size = csv[i][colCount];
                                var colTax = csv[i][colTaxid];

                                if (colTax == "n/a"){colTax="-1";}
                                sequence = sequence.concat (csv[i].matchname+":"+colTax+":"+csv[i].rank+":0/");
                                if (isNaN(size)) {
                                        continue;
                                }
                                sequence = sequence.substring(0, sequence.length - 1);
                                var parts = sequence.split("/");

                                if (parts.length > thiSS.maxLevel){
                                    thiSS.maxLevel = parts.length;
                                }
                                var currentNode = root;
                                if (thiSS.colorCol && isNaN(parseInt(csv[i][thiSS.colorCol]))) currentNode.color = "#000000";
                                else if (thiSS.colorCol) currentNode.color = gClrTable[csv[i][thiSS.colorCol] % gClrTable.length];
                                colorNum++;
                                for (var j = 1; j < parts.length; j++) {
                                        var children = currentNode["children"];
                                        var nodeText = parts[j];
                                        var spl = nodeText.split(":");
                                        var nodeName = spl[0];
                                        if (nodeName == "all")
                                                continue;
                                        var shortName =  nodeName.length>15 ? nodeName.substr(0,15-1)+'...' : nodeName;
                                        var nodeTaxid = spl[1];
                                        if (colTax == "-1"){
                                            nodeName = "unaligned";
                                            j++;
                                        }
                                        if (colTax == "NO_INFO"){
                                            nodeName = "no_match";
                                            j++;
                                        }
                                        var nodeRank = spl[2];
                                        
                                        var childNode;
                                        if (j + 1 < parts.length) {
                                                var foundChild = false;
                                                for (var k = 0; k < children.length; k++) {
                                                        if (children[k]["name"] == nodeName) {
                                                                childNode = children[k];
                                                                foundChild = true;
                                                                break;
                                                        }
                                                }
                                                if (!foundChild) {
                                                    
                                                        childNode = {
                                                                "name" : nodeName,
                                                                "shortname" : shortName,
                                                                "children" : [],
                                                                "category" : nodeRank,
                                                                "taxid" : nodeTaxid,
                                                                "color": 0,
                                                                "inGroupOrder":currentNode.inGroupOrder+currentNode.children.length,
                                                                "lev_depth":currentNode.lev_depth+1
                                                        };
                                                        myOwnColor = hueshiftColor(currentNode.color,childNode.inGroupOrder*127+colorNum++);
                                                        if (currentNode.color)
                                                            if (currentNode.inGroupOrder == childNode.inGroupOrder){
                                                                childNode.color = currentNode.color;
                                                            }
                                                            else{
                                                                childNode.color = myOwnColor;
                                                            }
                                                        else if (myOwnColor)
                                                                childNode.color = myOwnColor;

                                                        children.push(childNode);
                                                }
                                                currentNode = childNode;
                                                
                                        } else {
                                                childNode = {
                                                        "name" : nodeName,
                                                        "shortname" : shortName,
                                                        "size" : size,
                                                        "category" : nodeRank,
                                                        "taxid" : nodeTaxid,
                                                        "color": 0,
                                                        "children":[],
                                                        "inGroupOrder": currentNode.inGroupOrder+currentNode.children.length,
                                                        "lev_depth":currentNode.lev_depth+1
                                                };
                                                myOwnColor = hueshiftColor(currentNode.color,childNode.inGroupOrder*127+colorNum++);
                                                if (currentNode.color){
                                                    if (currentNode.inGroupOrder == childNode.inGroupOrder){
                                                        childNode.color = currentNode.color;
                                                    }
                                                    else{
                                                        childNode.color = myOwnColor;
                                                    }
                                                }
                                                else if (myOwnColor)
                                                        childNode.color = myOwnColor;
                                                children.push(childNode);
                                        }
                                }
                        }
                        return root;
                };
    };
}