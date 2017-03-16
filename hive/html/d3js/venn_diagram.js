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

//document.write('<link rel="stylesheet" type="text/css"  href="d3js/scatter_correlations.css">')

function vjD3JS_vennDiagram ( viewer )
{
    loadCSS("d3js/css/venn_diagram.css");
    vjD3View.call(this,viewer); // inherit default behaviours of the DataViewer
    
    this.createVenn = function (venn) {
        "use strict";
        /** given a list of set objects, and their corresponding overlaps.
        updates the (x, y, radius) attribute on each set such that their positions
        roughly correspond to the desired overlaps */
        
        venn.venn = function(sets, overlaps, parameters) {
            parameters = parameters || {};
            parameters.maxIterations = parameters.maxIterations || 500;
            var lossFunction = parameters.lossFunction || venn.lossFunction;
            var initialLayout = parameters.layoutFunction || venn.greedyLayout;

            // initial layout is done greedily
            sets = initialLayout(sets, overlaps);

            // transform x/y coordinates to a vector to optimize
            var initial = new Array(2*sets.length);
            for (var i = 0; i < sets.length; ++i) {
                initial[2 * i] = sets[i].x;
                initial[2 * i + 1] = sets[i].y;
            }

            // optimize initial layout from our loss function
            var totalFunctionCalls = 0;
            var solution = venn.fmin(
                function(values) {
                    totalFunctionCalls += 1;
                    var current = new Array(sets.length);
                    for (var i = 0; i < sets.length; ++i) {
                        current[i] = {x: values[2 * i],
                                      y: values[2 * i + 1],
                                      radius : sets[i].radius,
                                      size : sets[i].size};
                    }
                    return lossFunction(current, overlaps);
                },
                initial,
                parameters);

            // transform solution vector back to x/y points
            var positions = solution.solution;
            for (i = 0; i < sets.length; ++i) {
                sets[i].x = positions[2 * i];
                sets[i].y = positions[2 * i + 1];
                sets[i].curIndex = i;
            }

            return sets;
        };

        /** Returns the distance necessary for two circles of radius r1 + r2 to
        have the overlap area 'overlap' */
        venn.distanceFromIntersectArea = function(r1, r2, overlap) {
            // handle complete overlapped circles
            if (Math.min(r1, r2) * Math.min(r1,r2) * Math.PI <= overlap) {
                return Math.abs(r1 - r2);
            }

            return venn.bisect(function(distance) {
                return circleIntersection.circleOverlap(r1, r2, distance) - overlap;
            }, 0, r1 + r2);
        };

        /// gets a matrix of euclidean distances between all sets in venn diagram
        venn.getDistanceMatrix = function(sets, overlaps) {
            // initialize an empty distance matrix between all the points
            var distances = [];
            for (var i = 0; i < sets.length; ++i) {
                distances.push([]);
                for (var j = 0; j < sets.length; ++j) {
                    distances[i].push(0);
                }
            }

            // compute distances between all the points
            for (i = 0; i < overlaps.length; ++i) {
                var current = overlaps[i];
                if (current.sets.length !== 2) {
                    continue;
                }

                var left = current.sets[0],
                    right = current.sets[1],
                    r1 = Math.sqrt(sets[left].size / Math.PI),
                    r2 = Math.sqrt(sets[right].size / Math.PI),
                    distance = venn.distanceFromIntersectArea(r1, r2, current.size);
                distances[left][right] = distances[right][left] = distance;
            }
            return distances;
        };

        /** Lays out a Venn diagram greedily, going from most overlapped sets to
        least overlapped, attempting to position each new set such that the
        overlapping areas to already positioned sets are basically right */
        venn.greedyLayout = function(sets, overlaps) {
            // give each set a default position + radius
            var setOverlaps = {};
            for (var i = 0; i < sets.length; ++i) {
                setOverlaps[i] = [];
                sets[i].radius = Math.sqrt(sets[i].size / Math.PI);
                sets[i].x = sets[i].y = 0;
            }

            // map each set to a list of all the other sets that overlap it
            for (i = 0; i < overlaps.length; ++i) {
                var current = overlaps[i];
                if (current.sets.length !== 2) {
                    continue;
                }

                var left = current.sets[0], right = current.sets[1];
                setOverlaps[left].push ({set:right, size:current.size});
                setOverlaps[right].push({set:left,  size:current.size});
            }

            // get list of most overlapped sets
            var mostOverlapped = [];
            for (var set in setOverlaps) {
                if (setOverlaps.hasOwnProperty(set)) {
                    var size = 0;
                    for (i = 0; i < setOverlaps[set].length; ++i) {
                        size += setOverlaps[set][i].size;
                    }

                    mostOverlapped.push({set: set, size:size});
                }
            }

            // sort by size desc
            function sortOrder(a,b) {
                return b.size - a.size;
            }
            mostOverlapped.sort(sortOrder);

            // keep track of what sets have been laid out
            var positioned = {};
            function isPositioned(element) {
                return element.set in positioned;
            }

            // adds a point to the output
            function positionSet(point, index) {
                sets[index].x = point.x;
                sets[index].y = point.y;
                positioned[index] = true;
            }

            // add most overlapped set at (0,0)
            positionSet({x: 0, y: 0}, mostOverlapped[0].set);

            // get distances between all points
            var distances = venn.getDistanceMatrix(sets, overlaps);

            for (i = 1; i < mostOverlapped.length; ++i) {
                var setIndex = mostOverlapped[i].set,
                    overlap = setOverlaps[setIndex].filter(isPositioned);
                set = sets[setIndex];
                overlap.sort(sortOrder);

                if (overlap.length === 0) {
                    throw "Need overlap information for set " + JSON.stringify( set );
                }

                var points = [];
                for (var j = 0; j < overlap.length; ++j) {
                    // get appropriate distance from most overlapped already added set
                    var p1 = sets[overlap[j].set],
                        d1 = distances[setIndex][overlap[j].set];

                    // sample positions at 90 degrees for maximum aesthetics
                    points.push({x : p1.x + d1, y : p1.y});
                    points.push({x : p1.x - d1, y : p1.y});
                    points.push({y : p1.y + d1, x : p1.x});
                    points.push({y : p1.y - d1, x : p1.x});

                    // if we have at least 2 overlaps, then figure out where the
                    // set should be positioned analytically and try those too
                    for (var k = j + 1; k < overlap.length; ++k) {
                        var p2 = sets[overlap[k].set],
                            d2 = distances[setIndex][overlap[k].set];

                        var extraPoints = circleIntersection.circleCircleIntersection(
                            { x: p1.x, y: p1.y, radius: d1},
                            { x: p2.x, y: p2.y, radius: d2});

                        for (var l = 0; l < extraPoints.length; ++l) {
                            points.push(extraPoints[l]);
                        }
                    }
                }

                // we have some candidate positions for the set, examine loss
                // at each position to figure out where to put it at
                var bestLoss = 1e50, bestPoint = points[0];
                for (j = 0; j < points.length; ++j) {
                    sets[setIndex].x = points[j].x;
                    sets[setIndex].y = points[j].y;
                    var loss = venn.lossFunction(sets, overlaps);
                    if (loss < bestLoss) {
                        bestLoss = loss;
                        bestPoint = points[j];
                    }
                }

                positionSet(bestPoint, setIndex);
            }

            return sets;
        };

        /// Uses multidimensional scaling to approximate a first layout here
        venn.classicMDSLayout = function(sets, overlaps) {
            // get the distance matrix
            var distances = venn.getDistanceMatrix(sets, overlaps);

            // get positions for each set
            var positions = mds.classic(distances);

            // translate back to (x,y,radius) coordinates
            for (var i = 0; i < sets.length; ++i) {
                sets[i].x = positions[i][0];
                sets[i].y = positions[i][1];
                sets[i].radius = Math.sqrt(sets[i].size / Math.PI);
            }
            return sets;
        };

        /** Given a bunch of sets, and the desired overlaps between these sets - computes
        the distance from the actual overlaps to the desired overlaps. Note that
        this method ignores overlaps of more than 2 circles */
        venn.lossFunction = function(sets, overlaps) {
            var output = 0;

            function getCircles(indices) {
                return indices.map(function(i) { return sets[i]; });
            }

            for (var i = 0; i < overlaps.length; ++i) {
                var area = overlaps[i], overlap;
                if (area.sets.length == 2) {
                    var left = sets[area.sets[0]],
                        right = sets[area.sets[1]];
                    overlap = circleIntersection.circleOverlap(left.radius, right.radius,
                                    circleIntersection.distance(left, right));
                } else {
                    overlap = circleIntersection.intersectionArea(getCircles(area.sets));
                }

                output += (overlap - area.size) * (overlap - area.size);
            }

            return output;
        };

        /** Scales a solution from venn.venn or venn.greedyLayout such that it fits in
        a rectangle of width/height - with padding around the borders. */
        venn.scaleSolution = function(solution, width, height, padding) {
            var minMax = function(d) {
                var hi = Math.max.apply(null, solution.map(
                                        function(c) { return c[d] + c.radius; } )),
                    lo = Math.min.apply(null, solution.map(
                                        function(c) { return c[d] - c.radius;} ));
                return {max:hi, min:lo};
            };

            width -= 2*padding;
            height -= 2*padding;

            var xRange = minMax('x'),
                yRange = minMax('y'),
                xScaling = width  / (xRange.max - xRange.min),
                yScaling = height / (yRange.max - yRange.min),
                scaling = Math.min(yScaling, xScaling);

            for (var i = 0; i < solution.length; ++i) {
                var set = solution[i];
                set.radius = scaling * set.radius;
                set.x = padding + (set.x - xRange.min) * scaling;
                set.y = padding + (set.y - yRange.min) * scaling;
            }
            solution.scaling = scaling;

            return solution;
        };

        function weightedSum(a, b) {
            var ret = new Array(a[1].length || 0);
            for (var j = 0; j < ret.length; ++j) {
                ret[j] = a[0] * a[1][j] + b[0] * b[1][j];
            }
            return ret;
        }

        function centerVennDiagram( diagram, width, height, padding ) {
            var diagramBoundaries;
            var allowedWidth = width - ( 2 * ( padding || 0 ) );
            var allowedHeight = height - ( 2 * ( padding || 0 ) );
            var scale;
            var transformX, transformY;
            var transform = "";

            if ( diagram ) {
                diagramBoundaries = diagram[ 0 ][ 0 ].getBBox();
                if ( diagramBoundaries && width && height ) {

                    //  See if we need to scale to fit the width/height
                    if ( diagramBoundaries.width > allowedWidth ) {
                        scale = allowedWidth / diagramBoundaries.width;
                    }
                    if ( diagramBoundaries.height > allowedHeight ) {
                        if ( !scale || ( allowedHeight / diagramBoundaries.height ) < scale ) {
                            scale = allowedHeight / diagramBoundaries.height;
                        }
                    }

                    if ( scale ) {
                        transform = "scale(" + scale + ")";
                    }
                    else {
                        scale = 1;
                    }

                    transformX = Math.floor( ( allowedWidth - ( diagramBoundaries.width * scale ) ) / 2 );
                    transformY = Math.floor( ( allowedHeight - ( diagramBoundaries.height * scale ) ) / 2 );
                    diagram.attr( "transform", "translate(" + transformX + ","  + transformY + ") " + transform );
                }
            }
        }

        /** finds the zeros of a function, given two starting points (which must
         * have opposite signs */
        venn.bisect = function(f, a, b, parameters) {
            parameters = parameters || {};
            var maxIterations = parameters.maxIterations || 100,
                tolerance = parameters.tolerance || 1e-10,
                fA = f(a),
                fB = f(b),
                delta = b - a;

            if (fA * fB > 0) {
                throw "Initial bisect points must have opposite signs";
            }

            if (fA === 0) return a;
            if (fB === 0) return b;

            for (var i = 0; i < maxIterations; ++i) {
                delta /= 2;
                var mid = a + delta,
                    fMid = f(mid);

                if (fMid * fA >= 0) {
                    a = mid;
                }

                if ((Math.abs(delta) < tolerance) || (fMid === 0)) {
                    return mid;
                }
            }
            return a + delta;
        };

        /** minimizes a function using the downhill simplex method */
        venn.fmin = function(f, x0, parameters) {
            parameters = parameters || {};

            var maxIterations = parameters.maxIterations || x0.length * 200,
                nonZeroDelta = parameters.nonZeroDelta || 1.1,
                zeroDelta = parameters.zeroDelta || 0.001,
                minErrorDelta = parameters.minErrorDelta || 1e-5,
                rho = parameters.rho || 1,
                chi = parameters.chi || 2,
                psi = parameters.psi || -0.5,
                sigma = parameters.sigma || 0.5,
                callback = parameters.callback;

            // initialize simplex.
            var N = x0.length,
                simplex = new Array(N + 1);
            simplex[0] = x0;
            simplex[0].fx = f(x0);
            for (var i = 0; i < N; ++i) {
                var point = x0.slice();
                point[i] = point[i] ? point[i] * nonZeroDelta : zeroDelta;
                simplex[i+1] = point;
                simplex[i+1].fx = f(point);
            }

            var sortOrder = function(a, b) { return a.fx - b.fx; };

            for (var iteration = 0; iteration < maxIterations; ++iteration) {
                simplex.sort(sortOrder);
                if (callback) {
                    callback(simplex);
                }

                if (Math.abs(simplex[0].fx - simplex[N].fx) < minErrorDelta) {
                    break;
                }

                // compute the centroid of all but the worst point in the simplex
                var centroid = new Array(N);
                for (i = 0; i < N; ++i) {
                    centroid[i] = 0;
                    for (var j = 0; j < N; ++j) {
                        centroid[i] += simplex[j][i];
                    }
                    centroid[i] /= N;
                }

                // reflect the worst point past the centroid  and compute loss at reflected
                // point
                var worst = simplex[N];
                var reflected = weightedSum([1+rho, centroid], [-rho, worst]);
                reflected.fx = f(reflected);

                var replacement = reflected;

                // if the reflected point is the best seen, then possibly expand
                if (reflected.fx <= simplex[0].fx) {
                    var expanded = weightedSum([1+chi, centroid], [-chi, worst]);
                    expanded.fx = f(expanded);
                    if (expanded.fx < reflected.fx) {
                        replacement = expanded;
                    }
                }

                // if the reflected point is worse than the second worst, we need to
                // contract
                else if (reflected.fx >= simplex[N-1].fx) {
                    var shouldReduce = false;
                    var contracted;

                    if (reflected.fx <= worst.fx) {
                        // do an inside contraction
                        contracted = weightedSum([1+psi, centroid], [-psi, worst]);
                        contracted.fx = f(contracted);
                        if (contracted.fx < worst.fx) {
                            replacement = contracted;
                        } else {
                            shouldReduce = true;
                        }
                    } else {
                        // do an outside contraction
                        contracted = weightedSum([1-psi * rho, centroid], [psi*rho, worst]);
                        contracted.fx = f(contracted);
                        if (contracted.fx <= reflected.fx) {
                            replacement = contracted;
                        } else {
                            shouldReduce = true;
                        }
                    }

                    if (shouldReduce) {
                        // do reduction. doesn't actually happen that often
                        for (i = 1; i < simplex.length; ++i) {
                            simplex[i] = weightedSum([1 - sigma, simplex[0]],
                                                     [sigma - 1, simplex[i]]);
                            simplex[i].fx = f(simplex[i]);
                        }
                    }
                }

                simplex[N] = replacement;
            }

            simplex.sort(sortOrder);
            return {f : simplex[0].fx,
                    solution : simplex[0]};
        };

        /** returns a svg path of the intersection area of a bunch of circles */
        venn.intersectionAreaPath = function(circles) {
            var stats = {};
            circleIntersection.intersectionArea(circles, stats);
            var arcs = stats.arcs;

            if (arcs.length == 0) {
                return "M 0 0";
            }

            var ret = ["\nM", arcs[0].p2.x, arcs[0].p2.y];
            for (var i = 0; i < arcs.length; ++i) {
                var arc = arcs[i], r = arc.circle.radius, wide = arc.width > r;
                ret.push("\nA", r, r, 0, wide ? 1 : 0, 1, arc.p1.x, arc.p1.y);
            }

            return ret.join(" ");
        }
        
        venn.drawD3Diagram = function(element, dataset, width, height, parameters) {
            parameters = parameters || {};

            var colours = d3.scale.category10(),
                circleFillColours = parameters.circleFillColours || colours,
                circleStrokeColours = parameters.circleStrokeColours || circleFillColours,
                circleStrokeWidth = parameters.circleStrokeWidth || function(i) { return 0; },
                textFillColours = parameters.textFillColours || colours,
                textStrokeColours = parameters.textStrokeColours || textFillColours,
                nodeOpacity = parameters.opacity || 0.3,
                padding = parameters.padding || 6;

            dataset = venn.scaleSolution(dataset, width, height, padding);
            var svg = element.append("svg")
                    .attr("width", width)
                    .attr("height", height);
         
           
            var diagram = svg.append( "g" );
           
            
            var nodes = diagram.append("g").selectAll("g")
                             .data(dataset, function(d) {
                               return d.label;
                             });
            var nodeEnter = nodes.enter()
              .append("g")
              .attr('class', 'node');

            nodeEnter.append("circle")
                   .attr("r",  function(d) { return d.radius; })
                   .style("fill-opacity", nodeOpacity)
                   .attr("id", function(d) { return "single " + d.label; })
                   .attr("cx", function(d) { return d.x; })
                   .attr("cy", function(d) { return d.y; })
                   .style("stroke", function(d, i) { return circleStrokeColours(i); })
                   .style("stroke-width", function(d, i) { return circleStrokeWidth(i); })
                   .style("fill", function(d, i) { return pastelColor(circleFillColours(i),0.4); }); // pastelColor(color[ia],0.3)

            nodeEnter.append("text")
                   .attr("x", function(d) { return d.x; })
                   .attr("y", function(d) { return d.y; })
                   .attr("text-anchor", "middle")
                   .attr("dy", "0.35em")
                   .style("stroke", function(d, i) { return textStrokeColours(i); })
                   .style("fill", function(d, i) { return textFillColours(i); })
                   .text(function(d) { return d.label; });

            var circles = nodes.selectAll("circle");

            var text = nodes.selectAll("text");
            centerVennDiagram( diagram, width, height, padding );

            return {'svg' : svg,
                    'nodes' : nodes,
                    'circles' : circles,
                    'text' : text };
        };

        venn.updateD3Diagram = function(element, dataset, parameters) {
            parameters = parameters || {};

            var colours = d3.scale.category10(),
                circleFillColours = parameters.circleFillColours || colours,
                circleStrokeColours = parameters.circleStrokeColours || circleFillColours,
                circleStrokeWidth = parameters.circleStrokeWidth || function(i) { return 0; },
                textFillColours = parameters.textFillColours || colours,
                textStrokeColours = parameters.textStrokeColours || textFillColours,
                nodeOpacity = parameters.opacity || 0.3,
                padding = parameters.padding || 6;

            var svg = element.select("svg"),
                width = parseInt(svg.attr('width'), 10),
                height = parseInt(svg.attr('height'), 10),
                circles, texts;

            dataset = venn.scaleSolution(dataset, width, height, 6);
            var nodes = svg.select('g').select('g').selectAll("g.node")
                .data(dataset, function(d) { return d.label; });

            nodes.exit().remove();

            var nodeEnter = nodes.enter()
              .append("g")
              .attr('class', 'node');

            nodeEnter.append("circle")
                   .style("fill-opacity", nodeOpacity)
                   .style("stroke", function(d, i) { return circleStrokeColours(i); })
                   .style("stroke-width", function(d, i) { return circleStrokeWidth(i); })
                   .style("fill", function(d, i) { return circleFillColours(i); });

            nodeEnter.append("text")
                   .attr("text-anchor", "middle")
                   .attr("dy", "0.35em")
                   .style("stroke", function(d, i) { return textStrokeColours(i); })
                   .style("fill", function(d, i) { return textFillColours(i); })

            nodes.select("circle")
                .transition()
                .duration(400)
                .attr("cx", function(d) { return d.x; })
                .attr("cy", function(d) { return d.y; })
                .attr("r",  function(d) { return d.radius; });

            nodes.select("text")
                .transition()
                .duration(400)
                .text(function(d) { return d.label; })
                .attr("x", function(d) { return d.x; })
                .attr("y", function(d) { return d.y; });
        };
    };
    

    this.createCircle = function (circleIntersection) {
        "use strict";
        var SMALL = 1e-10;

        /** Returns the intersection area of a bunch of circles (where each circle
         is an object having an x,y and radius property) */
        circleIntersection.intersectionArea = function(circles, stats) {
            // get all the intersection points of the circles
            var intersectionPoints = getIntersectionPoints(circles);

            // filter out points that aren't included in all the circles
            var innerPoints = intersectionPoints.filter(function (p) {
                return circleIntersection.containedInCircles(p, circles);
            });

            var arcArea = 0, polygonArea = 0, arcs = [], i;

            // if we have intersection points that are within all the circles,
            // then figure out the area contained by them
            if (innerPoints.length > 1) {
                // sort the points by angle from the center of the polygon, which lets
                // us just iterate over points to get the edges
                var center = circleIntersection.getCenter(innerPoints);
                for (i = 0; i < innerPoints.length; ++i ) {
                    var p = innerPoints[i];
                    p.angle = Math.atan2(p.x - center.x, p.y - center.y);
                }
                innerPoints.sort(function(a,b) { return b.angle - a.angle;});

                // iterate over all points, get arc between the points
                // and update the areas
                var p2 = innerPoints[innerPoints.length - 1];
                for (i = 0; i < innerPoints.length; ++i) {
                    var p1 = innerPoints[i];

                    // polygon area updates easily ...
                    polygonArea += (p2.x + p1.x) * (p1.y - p2.y);

                    // updating the arc area is a little more involved
                    var midPoint = {x : (p1.x + p2.x) / 2,
                                    y : (p1.y + p2.y) / 2},
                        arc = null;

                    for (var j = 0; j < p1.parentIndex.length; ++j) {
                        if (p2.parentIndex.indexOf(p1.parentIndex[j]) > -1) {
                            // figure out the angle halfway between the two points
                            // on the current circle
                            var circle = circles[p1.parentIndex[j]],
                                a1 = Math.atan2(p1.x - circle.x, p1.y - circle.y),
                                a2 = Math.atan2(p2.x - circle.x, p2.y - circle.y);

                            var angleDiff = (a2 - a1);
                            if (angleDiff < 0) {
                                angleDiff += 2*Math.PI;
                            }

                            // and use that angle to figure out the width of the
                            // arc
                            var a = a2 - angleDiff/2,
                                width = circleIntersection.distance(midPoint, {
                                    x : circle.x + circle.radius * Math.sin(a),
                                    y : circle.y + circle.radius * Math.cos(a)
                                });

                            // pick the circle whose arc has the smallest width
                            if ((arc === null) || (arc.width > width)) {
                                arc = { circle : circle,
                                        width : width,
                                        p1 : p1,
                                        p2 : p2};
                            }
                        }
                    }
                    arcs.push(arc);
                    if (arc && arc.circle)
                        arcArea += circleIntersection.circleArea(arc.circle.radius, arc.width);
                    p2 = p1;
                }
            } else {
                // no intersection points, is either disjoint - or is completely
                // overlapped. figure out which by examining the smallest circle
                var smallest = circles[0];
                for (i = 1; i < circles.length; ++i) {
                    if (circles[i].radius < smallest.radius) {
                        smallest = circles[i];
                    }
                }

                // make sure the smallest circle is completely contained in all
                // the other circles
                var disjoint = false;
                for (i = 0; i < circles.length; ++i) {
                    if (circleIntersection.distance(circles[i], smallest) > Math.abs(smallest.radius - circles[i].radius)) {
                        disjoint = true;
                        break;
                    }
                }

                if (disjoint) {
                    arcArea = polygonArea = 0;

                } else {
                    arcArea = smallest.radius * smallest.radius * Math.PI;
                    arcs.push({circle : smallest,
                               p1: { x: smallest.x,        y : smallest.y + smallest.radius},
                               p2: { x: smallest.x - SMALL, y : smallest.y + smallest.radius},
                               width : smallest.radius * 2 });
                }
            }

            polygonArea /= 2;
            if (stats) {
                stats.area = arcArea + polygonArea;
                stats.arcArea = arcArea;
                stats.polygonArea = polygonArea;
                stats.arcs = arcs;
                stats.innerPoints = innerPoints;
                stats.intersectionPoints = intersectionPoints;
            }

            return arcArea + polygonArea;
        };

        /** returns whether a point is contained by all of a list of circles */
        circleIntersection.containedInCircles = function(point, circles) {
            for (var i = 0; i < circles.length; ++i) {
                if (circleIntersection.distance(point, circles[i]) > circles[i].radius + SMALL) {
                    return false;
                }
            }
            return true;
        };

        /** Gets all intersection points between a bunch of circles */
        function getIntersectionPoints(circles) {
            var ret = [];
            for (var i = 0; i < circles.length; ++i) {
                for (var j = i + 1; j < circles.length; ++j) {
                    var intersect = circleIntersection.circleCircleIntersection(circles[i],
                                                                  circles[j]);
                    for (var k = 0; k < intersect.length; ++k) {
                        var p = intersect[k];
                        p.parentIndex = [i,j];
                        ret.push(p);
                    }
                }
            }
            return ret;
        }

        circleIntersection.circleIntegral = function(r, x) {
            var y = Math.sqrt(r * r - x * x);
            return x * y + r * r * Math.atan2(x, y);
        };

        /** Returns the area of a circle of radius r - up to width */
        circleIntersection.circleArea = function(r, width) {
            return circleIntersection.circleIntegral(r, width - r) - circleIntersection.circleIntegral(r, -r);
        };


        /** euclidean distance between two points */
        circleIntersection.distance = function(p1, p2) {
            return Math.sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                             (p1.y - p2.y) * (p1.y - p2.y));
        };


        /** Returns the overlap area of two circles of radius r1 and r2 - that
        have their centers separated by distance d. Simpler faster
        circle intersection for only two circles */
        circleIntersection.circleOverlap = function(r1, r2, d) {
            // no overlap
            if (d >= r1 + r2) {
                return 0;
            }

            // completely overlapped
            if (d <= Math.abs(r1 - r2)) {
                return Math.PI * Math.min(r1, r2) * Math.min(r1, r2);
            }

            var w1 = r1 - (d * d - r2 * r2 + r1 * r1) / (2 * d),
                w2 = r2 - (d * d - r1 * r1 + r2 * r2) / (2 * d);
            return circleIntersection.circleArea(r1, w1) + circleIntersection.circleArea(r2, w2);
        };


        /** Given two circles (containing a x/y/radius attributes),
        returns the intersecting points if possible.
        note: doesn't handle cases where there are infinitely many
        intersection points (circles are equivalent):, or only one intersection point*/
        circleIntersection.circleCircleIntersection = function(p1, p2) {
            var d = circleIntersection.distance(p1, p2),
                r1 = p1.radius,
                r2 = p2.radius;

            // if to far away, or self contained - can't be done
            if ((d >= (r1 + r2)) || (d <= Math.abs(r1 - r2))) {
                return [];
            }

            var a = (r1 * r1 - r2 * r2 + d * d) / (2 * d),
                h = Math.sqrt(r1 * r1 - a * a),
                x0 = p1.x + a * (p2.x - p1.x) / d,
                y0 = p1.y + a * (p2.y - p1.y) / d,
                rx = -(p2.y - p1.y) * (h / d),
                ry = -(p2.x - p1.x) * (h / d);

            return [{ x: x0 + rx, y : y0 - ry },
                    { x: x0 - rx, y : y0 + ry }];
        };

        /** Returns the center of a bunch of points */
        circleIntersection.getCenter = function(points) {
            var center = { x: 0, y: 0};
            for (var i =0; i < points.length; ++i ) {
                center.x += points[i].x;
                center.y += points[i].y;
            }
            center.x /= points.length;
            center.y /= points.length;
            return center;
        };
    }
    
    this.convertToJSONFromCSVTable = function (data, threshold){
        var tbl = new vjTable(data,0,vjTable_propCSV);
        var defaultColumnToStart = 1; // first column, because it's just a left header    
        // skip the first column
        var sets = []; // label = {0:'ref 1', 1: 'ref 2'}
        var overlapObj= {}; // setCollection = {'0':8,'0,1':15 }
        var overlaps= []; // setCollection = {'0':8,'0,1':15 }
        var categoryDic={}; // use for the index
        
        var listOfColumns=[]; // 
        var firstRound=true;
        
        for (var ir=0; ir < tbl.rows.length; ++ir){
            var row=tbl.rows[ir];
            var stackCollection = []; // [0,1,2,3,4]
            var longestK="",longestScore=0, lastSelf=-1;
            var score =0;
            var myKeyValue="";
            
            
            if (this.toExclude!=undefined && this.toSearch!=undefined && this.toSearch.length){
                var toSearch = new RegExp(this.toSearch,'g');
                var cellToLookInto = row.cols[0];
                if (this.toExclude &&  cellToLookInto.match(toSearch)){
                    //alert("Excluding //" + cellToLookInto)
                    continue;
                } else if (!this.toExclude && !cellToLookInto.match(toSearch)){
                    //alert("Not found // " + cellToLookInto)
                    continue;
                }
                //alert("SATISFY //"+ cellToLookInto)
            }
            for (var ic= defaultColumnToStart; ic<tbl.hdr.length; ++ic){
                
                score = ( parseFloat(row.cols[ic]) >= threshold) ? 1 : 0; // score based on the threshold
                var colName = tbl.hdr[ic].name;
                myKeyValue = "" + (ic-1);
                
                if (this.useCategory != undefined && this.useCategory){
                    // looking for category
                    //alert("looking for category")
                    var positionOfCategory =colName.indexOf("category="); // when the header contains category 
                    if (positionOfCategory!=-1){
                        positionOfCategory+= 9; // 9 because of the length of the word "category="
                        colName = colName.slice(positionOfCategory);
                        if (categoryDic[colName]==undefined){ // using of the dictionary for the index
                            categoryDic[colName] = Object.keys(categoryDic).length;
                        }
                        myKeyValue=categoryDic[colName]; // we need the index of the same category, in case of columns having the same category
                    }
                }
                if (firstRound) listOfColumns.push(myKeyValue); 
                if(!score){
                    continue;
                }  
                if (stackCollection.indexOf(myKeyValue.toString())!=-1) {
                    listOfColumns.pop();
                    continue;
                }
                if(sets[myKeyValue]===undefined)sets[myKeyValue]={size:0,label:colName,uniq:0};
                sets[myKeyValue].size+=score;
                lastSelf=myKeyValue;
                longestScore=score;
                                
                var cntStack=stackCollection.length;
                for ( var is=0; is<=cntStack; ++is) {
                    if(is==0)k="";
                    else k=stackCollection[is-1];
                    k+=(k.length ? "," :"" )+ myKeyValue;//(ic-1);
                    if (k.split(',').length>1){
                        k = k.split(',').sort().join(',');
                    }
                    stackCollection.push(k);
                    if(is>0) {
                        if( overlapObj[k]===undefined ) overlapObj[k]=overlaps.length;
                        if(overlaps[overlapObj[k]]===undefined)overlaps[overlapObj[k]]={sets:k.split(","),size:0,uniq:0};
                        overlaps[overlapObj[k]].size+=score;
                        
                        if(is==cntStack) { 
                            longestK=k;
                            longestScore=score;
                        }
                    }
                }
            }
            if( longestScore ) { 
                if(longestK.length)
                    overlaps[overlapObj[longestK]].uniq+=longestScore;
                if(longestK.length==0)
                    sets[lastSelf].uniq+=longestScore;
            }
            firstRound=false;
        }
        // overlaps is required, if it is empty => error, So need to fill it up
        if (!overlaps.length) {//overlaps=[{sets:[0,1],size:0, uniq:0},{sets:[0,2],size:0, uniq:0},{sets:[1,2],size:0, uniq:0},{sets:[0,1,2],size:0, uniq:0}];
            var iStart =1;
            for (var ie=0; ie<listOfColumns.length; ++ie){
                for (var imove=iStart; imove < listOfColumns.length; ++imove){
                    overlaps.push({sets:[listOfColumns[ie],listOfColumns[imove]],size:0,uniq:0});
                }
                ++iStart;
            }
        }    
        return {sets:sets, overlaps: overlaps };
        // Populate the JSON parse label and sets
        /*var myJSON = {
                        "sets": [
                              {"label": "Radiohead", "size": 77348},
                              {"label": "Thom Yorke", "size": 5621},
                              {"label": "John Lennon", "size": 7773}]
                         , "overlaps": [
                              {"sets": [0, 1], "size": 4832},
                              {"sets": [0, 2], "size": 2602},
                              {"sets": [0, 3], "size": 6141}]
                        };
        */
    }
    
    this.d3Compose=function(data){ 
        // data has to be plain text and comma separated      
        
        this.d3Compose_prv(data);
        var thiSS=this;
        var svg=this.d3svg;

        var defaultWidth = 500;
        var defaultHeight = 500;
        
        var chartAreaPercentage = '90%';
        if (this.options !=undefined && this.options.chartArea != undefined && this.options.chartArea.length){
            var chA = parseFloat(this.options.chartArea);
            if (chA > 1) chartAreaPercentage = chA/100;
        }
        else chartAreaPercentage = parseFloat(chartAreaPercentage)/100;
        
        var parentDiv = gObject(this.container);
        var maxHeight = parseInt(parentDiv.style.maxHeight);
        var maxWidth = parseInt(parentDiv.style.maxWidth);
        
        var width = this.width ? this.width : (this.options!=undefined ? (this.options.width ? this.options.width : 0) : 0);
        var height = this.height ? this.height : (this.options!=undefined ? (this.options.height ? this.options.height : 0) : 0);
        if (!width) width = maxWidth ? maxWidth : defaultWidth;
        if (!height) height = maxHeight ? maxHeight : defaultHeight;
        
        width = width * chartAreaPercentage;
        height = height * chartAreaPercentage;
        svg.attr('width',width);
        svg.attr('height',height);
        
        //var dataRaw = document.getElementById('dvVennRaw').innerText;
        var threshold = 0.05;
        if (this.threshold) threshold = this.threshold;
        var dataRaw = this.csvTbl ? this.convertToJSONFromCSVTable(data, threshold) : data;
        
        var diagram;
       
       var tooltip = d3.select("body").append("div")
                     .attr("class", "venntooltip"),
            init = true;

       var elem = svg;
       
       
       
    //  function load(rawJSONString) {
       var vennObj, sets, overlaps, jsonData;
        
        try {
          if(typeof (dataRaw) == "String" ) { 
              jsonData = JSON.parse(dataRaw);
          } else {  
              jsonData =dataRaw;
          }
          sets = jsonData.sets;
          overlaps = jsonData.overlaps
        } catch(e) {
          alert("JSON parse error. Please check your data and make sure 'csvTbl' flag is set");
          return;
        }
        // get positions for each set
        venn = new Object();
        circleIntersection = new Object();
        
        this.createVenn(venn);
        this.createCircle(circleIntersection);
        vennObj = venn.venn(sets, overlaps);
            
            if (init) {
              // draw the diagram in the 'venn' div
              diagram = venn.drawD3Diagram(elem, vennObj, width, height);
              init = false;
            }
            else {
                venn.updateD3Diagram(elem, vennObj);
            }
    
            // hover on all the circles
            diagram.circles
              .style("stroke-opacity", 0)
              .style("stroke", "white")
              .style("stroke-width", "2")
              .on("mousemove", function() {
                tooltip.style("left", (d3.event.pageX +10) + "px")
                .style("top", (d3.event.pageY - 30) + "px");
              })
              .on("mouseover", function(d, i) {
                var selection = d3.select(this);
                d3.select(this).moveParentToFront()
                .transition()
                .style("fill-opacity", .5)
                .style("stroke-opacity", 1);
    
                tooltip.transition().style("opacity", .9);
                if (d.uniq){
                    tooltip.text(d.size + " hits| "+d.uniq + "[" + vennObj[d.curIndex].label+ "]");
                } else tooltip.text(d.size + " hits");
                tooltip.each(function(){
                    var outText = "", lineTotal=1;
                    outText += '';
                    var textContent = this.innerText;
                    if (textContent.indexOf('|')==-1) return;
                    var firstPart = textContent.split('|')[0];
                    outText += firstPart + "</br>"; ++lineTotal;
                    var secondPart = textContent.split('|')[1];
                    var split = secondPart.split(' ');
                    var curLength = 0, longest=20, maxWidth = 200;
                    for (var i=0; i< split.length; ++i){
                        var precomputeLength = curLength+split[i].length;
                        if ((precomputeLength-longest) >0) {maxWidth += (precomputeLength-longest)*3}
                        if ( i && (precomputeLength )> 20 ) {outText += '<br>';++lineTotal; curLength=0; longest = precomputeLength;}
                        outText+= "" + split[i] + " ";
                        curLength+=split[i].length +1;
                    }
                    var newHeight = lineTotal * 20;
                    this.style.height = ""+ (newHeight+20)+'px';
                    this.style.width = ""+(maxWidth+20)+'px';
                    this.innerHTML = outText;
                    
                });
              })
              .on("mouseout", function(d, i) {
                d3.select(this).transition()
                .style("fill-opacity", .3)
                .style("stroke-opacity", 0);
                tooltip.transition().style("opacity", 0);
              });
    
            // draw a path around each intersection area, add hover there as well
            var intersections = diagram.svg.select("g").selectAll("path")
              .data(overlaps)
            intersections
              .enter()
              .append("path")
              .style("fill-opacity","0")
              .style("fill", "black")
              .style("stroke-opacity", 0)
              .style("stroke", "white")
              .style("stroke-width", "2")
              
              
            intersections
              .attr('class','overlaps')
              .attr("sets", function(d) { 
                return d.sets; 
              })
              .attr("score", function(d) { 
                return d.size; 
              })
              .attr("d", function(d) { 
                return venn.intersectionAreaPath(d.sets.map(function(j) { return vennObj[j]; })); 
              })
              .on("mouseover", function(d, i) {
                d3.select(this).transition()
                .style("fill-opacity", .1)
                .style("stroke-opacity", 1);
                var totalCommon =0, totalUniq=0;
                var curSetsLength = d.sets.length;
                var curSetsArr = d.sets;
                
                var totalSize = d.size;
                var unique = totalSize - totalCommon;
                tooltip.style("left", (d3.event.pageX + 10) + "px");
                tooltip.style("top", (d3.event.pageY - 40) + "px");
                tooltip.transition().style("opacity", .9);
                var overlapsName='';
                for (var i=0; i<curSetsArr.length; ++i){
                    if (i) overlapsName+=', ';
                    overlapsName+= vennObj[curSetsArr[i]].label;
                }
                if (d.uniq){
                    tooltip.text(totalSize + " total |" + d.uniq + " [" + overlapsName + "]");
                }    
                else tooltip.text(totalSize + " total");
                tooltip.each(function(){
                    var outText = "", lineTotal=1;
                    outText += '';
                    var textContent = this.innerText;
                    if (textContent.indexOf('|')==-1) return;
                    var firstPart = textContent.split('|')[0];
                    outText += firstPart + "</br>"; ++lineTotal;
                    var secondPart = textContent.split('|')[1];
                    var split = secondPart.split(' ');
                    var curLength = 0, longest=20, maxWidth = 200;
                    for (var i=0; i< split.length; ++i){
                        var precomputeLength = curLength+split[i].length;
                        if (!i && (precomputeLength-longest) >0) {maxWidth += (precomputeLength-longest)*3}
                        if ( i && (precomputeLength )> 20 ) {outText += '<br>';++lineTotal; curLength=0; longest = precomputeLength;}
                        outText+= "" + split[i] + " ";
                        curLength+=split[i].length +1;
                    }
                    var newHeight = lineTotal * 20;
                    this.style.height = ""+(newHeight+20)+'px';
                    this.style.width = ""+(maxWidth+10)+'px';
                    this.innerHTML = outText;
                    
                });
              })
              .on("mouseout", function(d, i) {
                d3.select(this).transition()
                .style("fill-opacity", 0)
                .style("stroke-opacity", 0);
                tooltip.transition().style("opacity", 0);
              })
              .on("mousemove", function() {
                tooltip.style("left", (d3.event.pageX+10) + "px")
                .style("top", (d3.event.pageY - 30) + "px");
              });
            
            intersections.exit().remove();
          //}  
           
          d3.selection.prototype.moveParentToFront = function() {
            return this.each(function(){
              this.parentNode.parentNode.appendChild(this.parentNode);
            });
          };

    };
}


// ##################

