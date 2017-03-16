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
        vjD3View.call(this,viewer); // inherit default behaviours of the DataViewer
        //this.csvParserFunction=d3.csv.parseRows;
        
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
        
        this.categClrArr={};//{'superkingdom':0, 'kingdom':1, 'family':2 , 'maxlight': 5};        
        this.click = viewer.funclick;


        // Use d3.text and d3.csv.parseRows so that we do not need to have a header
        // row, and can receive the csv as an array of arrays.
        /*d3.text("visit-sequences.csv", function(text) {
          var csv = d3.csv.parseRows(text);
          var json = buildHierarchy(csv);
          createVisualization(json);
        });
*/
    this.d3Compose=function(data){
        // Main function to draw and set up the visualization, once we have the data.
        //function createVisualization(json) {
                
        this.d3Compose_prv(data);
        
        var trail= this.d3svg.append("g").attr("id","trail");//thiSS.d3area.append("svg").attr("id","#trail"); // d3.select("#trail")
        
        var json = buildHierarchy(data,this.colPath,this.colCount, this.colTaxid);
                //createVisualization(json);
        
        // Dimensions of sunburst.
        var width = this.width;
        var height = this.height;
        var radius = Math.min(width, height) / 2  ;

        // Breadcrumb dimensions: width, height, spacing, width of tip/tail.
        var b = {
          w: 95, h: 30, s: 3, t: 10
        };

        // Mapping of step names to colors.
        function clr(d,i){
                if(!d.color){
                    return 'lightgray';
                }
                var shade = 2*d.lev_depth/thiSS.maxLevel;
                var max_lev = 0.75;
                return lightenColor (d.color,1-(1./(1.1+shade)));
                //return d.color;
                //var shade=thiSS.categClrArr[d.category] ? thiSS.categClrArr[d.category] : (thiSS.categClrArr[d.category]=thiSS.categClrDensity++);
//                var shade=thiSS.categClrArr[d.category] ? thiSS.categClrArr[d.category] : (d.lev_depth);
//                shade =  
//                Math.exp (d.lev_depth - thiSS.maxLevel);
//                return lightenColor (d.color, Math.pow (1.1, d.lev_depth - thiSS.maxLevel) );
                }

        
        
        // Total size of all segments; we set this later, after loading the data.
        var totalSize = 0; 

        var vis = this.d3svg
            .attr("width", width)
            .attr("height", height)
            .append("svg:g")
            .attr("id", "container")
            .attr("transform", "translate(" + width / 2 + "," + height / 2 + ")");
        var explanation= vis.append("g").attr("id","explanation");//thiSS.d3area.append("svg").attr("id","#trail"); // d3.select("#trail")
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

        

          // Basic setup of page elements.
          initializeBreadcrumbTrail();

          // Bounding circle underneath the sunburst, to make it easier to detect
          // when the mouse leaves the parent g.
          vis.append("svg:circle")
              .attr("r", radius)
              .style("opacity", 0);

          // For efficiency, filter nodes to keep only those large enough to see.
          var nodes = partition.nodes(json)
              .filter(function(d) {
              return (d.dx > 0.005); // 0.005 radians = 0.29 degrees
              });

          var path = vis.data([json]).selectAll("path")
              .data(nodes)
              .enter().append("svg:path")
              .attr("display", function(d) { return d.depth ? null : "none"; })
              .attr("d", arc)
              .attr("fill-rule", "evenodd")
              .style("fill", function(d,i) {return clr(d,i);})//{ return colors[d.name]; })
              .style("opacity", 1)
                .on("click", extclick)
              .on("mouseover", mouseover);

          // Add the mouseleave handler to the bounding circle.
          d3.select("#container").on("mouseleave", mouseleave);

          // Get total size of the tree = value of root node from partition.
          totalSize = path.node().__data__.value;
         

        function extclick(node){
            if(thiSS.click)
                funcLink( thiSS.click, this, node );
        }

        // Fade all but the current sequence, and show it in the breadcrumb trail.
        function mouseover(d) {

          var percentage = (100 * d.value / totalSize).toPrecision(3);
          var percentageString = percentage + "%";
          if (percentage < 0.1) {
            percentageString = "< 0.1%";
          }

//          perc.text(percentageString);
  //        perc.style("visibility","");
         // .attr("transform", "translate(" + width / 2 + "," + height / 2 + ")");
          
          d3.select("#percentage")
              .text(percentageString);

          d3.select("#explanation")
              .style("visibility", "");

          var sequenceArray = getAncestors(d);
          updateBreadcrumbs(sequenceArray, percentageString);

          // Fade all the segments.
          d3.selectAll("path")
              .style("opacity", 0.3);

          // Then highlight only those that are an ancestor of the current segment.
          vis.selectAll("path")
              .filter(function(node) {
                        return (sequenceArray.indexOf(node) >= 0);
                      })
              .style("opacity", 1);
        }

        // Restore everything to full opacity when moving off the visualization.
        function mouseleave(d) {

          // Hide the breadcrumb trail
          d3.select("#trail")
                .style("visibility", "hidden");

          // Deactivate all segments during transition.
          d3.selectAll("path").on("mouseover", null);

          // Transition each segment to full opacity and then reactivate it.
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

        // Given a node in a partition layout, return an array of all of its ancestor
        // nodes, highest first, but excluding the root.
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
          // Add the svg area.
          //var trail //=d3.select("#sequence").append("svg:svg")
          trail.attr("width", width)
              .attr("height", 50)
              .attr("font-weight", 600)
              .style("fill", "#000");
          // Add the label at the end, for the percentage.
          trail.append("svg:text")
            .attr("id", "endlabel")
            .style("fill", "#000");
        }

        // Generate a string that describes the points of a breadcrumb polygon.
        function breadcrumbPoints(d, i) {
          var points = [];
          points.push("0,0");
          points.push(b.w + ",0");
          points.push(b.w + b.t + "," + (b.h / 2));
          points.push(b.w + "," + b.h);
          points.push("0," + b.h);
          if (i > 0) { // Leftmost breadcrumb; don't include 6th vertex.
            points.push(b.t + "," + (b.h / 2));
          }
          return points.join(" ");
        }

        // Update the breadcrumb trail to show the current sequence and percentage.
        function updateBreadcrumbs(nodeArray, percentageString) {

                var Nout=5; // By default there is 1
                var printable=nodeArray.length-Nout+1;
                var space = 1;
                if (printable < 2){
                    space = 0;
                }
                if(printable<1){
                    printable=1;
                }
          // Data join; key function combines name and depth (= position in sequence).
          var g = // thiSS.d3area.append("svg") // d3.select("#trail")
              //.attr("id","#trail")
              trail.selectAll("g")
              .data(nodeArray, function(d) { return d.shortname + d.depth; });

          // Add breadcrumb and label for entering nodes.
          var entering = g.enter().append("svg:g");

          entering.append("svg:polygon")
              .attr("points", breadcrumbPoints)
              .style("fill", function(d,i) {return clr(d);});//{ return colors[d.name]; });

          entering.append("svg:text")
              .attr("x", (b.w + b.t) / 2)
              .attr("y", b.h / 2)
              .attr("dy", "0.35em")
              .attr("text-anchor", "middle")
              .text(function(d) { return d.shortname; })
              .style("font-size","10px");;

          // Set position for entering and updating nodes.
          g.attr("transform", function(d, i) {
                  if(d.lev_depth==1)
                          return "translate(0, 0)";
                  else if(d.lev_depth>printable)
                                return "translate(" + ((d.lev_depth-printable) * (b.w + b.s)+ (space)*b.w/2) + ", 0)";
                  else return "translate(-200, 0)";
          });

          // Remove exiting nodes.
          g.exit().remove();

          if (Nout > nodeArray.length){
              Nout = nodeArray.length;
          }
         
          // Now move and update the percentage at the end.
          trail.select("#endlabel")
              .attr("x", (Nout+0.5) * (b.w + b.s) + (space)*b.w/2)
              .attr("y", b.h / 2)
              .attr("dy", "0.35em")
              .attr("text-anchor", "middle")
              .text(percentageString);

          // Make the breadcrumb trail visible, if it's hidden.
                trail.style("visibility", function (d,i){
                  //if(d.lev_depth>0 && d.lev_depth<printable)
                //        return "hidden";
                  //else 
                          return "";
                });

        }

        // Take a 2-column CSV and transform it into a hierarchical structure suitable
        // for a partition layout. The first column is a sequence of step names, from
        // root to leaf, separated by hyphens. The second column is a count of how 
        // often that sequence occurred.
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
                                // Append the current node
                                sequence = sequence.concat (csv[i].matchname+":"+colTax+":"+csv[i].rank+":0/");
                                if (isNaN(size)) { // e.g. if this is a header row
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
                                            nodeName = "no match";
                                            j++;
                                        }
                                        var nodeRank = spl[2];
                                        
                                        var childNode;
                                        if (j + 1 < parts.length) {
                                                // Not yet at the end of the sequence; move down the tree.
                                                var foundChild = false;
                                                for (var k = 0; k < children.length; k++) {
                                                        if (children[k]["name"] == nodeName) {
                                                                childNode = children[k];
                                                                foundChild = true;
                                                                break;
                                                        }
                                                }
                                                // If we don't already have a child node for this branch, create it.
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
                                                // Reached the end of the sequence; create a leaf node.
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