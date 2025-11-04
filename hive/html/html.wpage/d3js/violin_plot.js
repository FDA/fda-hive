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

function vjD3JS_ViolinPlot ( viewer )
{
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
    
    function makeViolinPlot(dataset, xGroup, yValue) {
    
        var chart = {};
    
        var colorFunct = d3.scale.category10();
    
        function formatAsFloat(d) {
            if (d % 1 !== 0) {
                return d3.format(".2f")(d);
            } else {
                return d3.format(".0f")(d);
            }
    
        }
    
        function logFormatNumber(d) {
            var x = Math.log(d) / Math.log(10) + 1e-6;
            return Math.abs(x - Math.floor(x)) < 0.6 ? formatAsFloat(d) : "";
        }
    
        chart.yFormatter = formatAsFloat;
    
        chart.data = dataset;
    
        chart.xGroup = xGroup;
        chart.yValue = yValue;
        chart.groupObjs = {};
        chart.objs = {mainDiv: null, chartDiv: null, g: null, xAxis: null, yAxis: null};
    
    
        function updateColorFunction(colorOptions) {
            if (typeof colorOptions == 'function') {
                return colorOptions
            } else if (Array.isArray(colorOptions)) {
                var colorMap = {}, cColor = 0;
                for (var cName in chart.groupObjs) {
                    colorMap[cName] = colorOptions[cColor];
                    cColor = (cColor + 1) % colorOptions.length;
                }
                return function (group) {
                    return colorMap[group];
                }
            } else if (typeof colorOptions == 'object') {
                return function (group) {
                    return colorOptions[group];
                }
            }
        }
    
        function updateGroupWidth(boxWidth) {
            var boxSize = {left: null, right: null, middle: null};
            var width = chart.xScale.rangeBand() * (boxWidth / 100);
            var padding = (chart.xScale.rangeBand() - width) / 2;
            boxSize.middle = chart.xScale.rangeBand() / 2;
            boxSize.left = padding;
            boxSize.right = boxSize.left + width;
            return boxSize;
        }
    
        function tooltipHover(name, metrics) {
            var tooltipString = "Group: " + name;
            tooltipString += "<br\>Max: " + formatAsFloat(metrics.max, 0.1);
            tooltipString += "<br\>Q3: " + formatAsFloat(metrics.quartile3);
            tooltipString += "<br\>Median: " + formatAsFloat(metrics.median);
            tooltipString += "<br\>Q1: " + formatAsFloat(metrics.quartile1);
            tooltipString += "<br\>Min: " + formatAsFloat(metrics.min);
            return function () {
                chart.objs.tooltip.transition().duration(200).style("opacity", 0.9);
                chart.objs.tooltip.html(tooltipString)
            };
        }
    
        function prepareData() {
    
            function calcMetrics(values) {
    
                var metrics = {
                    max: null,
                    upperOuterFence: null,
                    upperInnerFence: null,
                    quartile3: null,
                    median: null,
                    mean: null,
                    iqr: null,
                    quartile1: null,
                    lowerInnerFence: null,
                    lowerOuterFence: null,
                    min: null
                };
    
                metrics.min = d3.min(values);
                metrics.quartile1 = d3.quantile(values, 0.25);
                metrics.median = d3.median(values);
                metrics.mean = d3.mean(values);
                metrics.quartile3 = d3.quantile(values, 0.75);
                metrics.max = d3.max(values);
                metrics.iqr = metrics.quartile3 - metrics.quartile1;
    
                var LIF = metrics.quartile1 - (1.5 * metrics.iqr);
                var UIF = metrics.quartile3 + (1.5 * metrics.iqr);
                for (var i = 0; i <= values.length; i++) {
                    if (values[i] < LIF) {
                        continue;
                    }
                    if (!metrics.lowerInnerFence && values[i] >= LIF) {
                        metrics.lowerInnerFence = values[i];
                        continue;
                    }
                    if (values[i] > UIF) {
                        metrics.upperInnerFence = values[i - 1];
                        break;
                    }
                }
    
                metrics.lowerOuterFence = metrics.quartile1 - (3 * metrics.iqr);
                metrics.upperOuterFence = metrics.quartile3 + (3 * metrics.iqr);
                if (!metrics.lowerInnerFence) {
                    metrics.lowerInnerFence = metrics.min;
                }
                if (!metrics.upperInnerFence) {
                    metrics.upperInnerFence = metrics.max;
                }
                return metrics
            }
    
            var current_x = null;
            var current_y = null;
            var current_row;
    
            for (current_row = 0; current_row < chart.data.length; current_row++) {
                current_x = chart.data[current_row][chart.xGroup];
                current_y = chart.data[current_row][chart.yValue];
                if (chart.groupObjs.hasOwnProperty(current_x)) {
                    chart.groupObjs[current_x].values.push(current_y);
                } else {
                    chart.groupObjs[current_x] = {};
                    chart.groupObjs[current_x].values = [current_y];
                }
            }
    
            var cName;
            for (cName in chart.groupObjs) {
                chart.groupObjs[cName].values.sort(d3.ascending);
                chart.groupObjs[cName].metrics = {};
                chart.groupObjs[cName].metrics = calcMetrics(chart.groupObjs[cName].values);
    
            }
        }
    
        prepareData();
    
        chart.update = function () {
    
            if (!chart.objs.g) {
                return false;
            }
    
            chart.width = parseInt(chart.objs.chartDiv.style("width"), 10) - (chart.margin.left + chart.margin.right);
            chart.height = parseInt(chart.objs.chartDiv.style("height"), 10) - (chart.margin.top + chart.margin.bottom);
            chart.xScale.rangeBands([0, chart.width]);
            chart.yScale.range([chart.height, 0]);
    
            chart.objs.g.select('.x.axis').attr("transform", "translate(0," + chart.height + ")").call(chart.objs.xAxis)
                .selectAll("text")
                .attr("y", 5)
                .attr("x", -5)
                .attr("transform", "rotate(-45)")
                .style("text-anchor", "end");
            chart.objs.g.select('.x.axis .label').attr("x", chart.width / 2);
            chart.objs.g.select('.y.axis').call(chart.objs.yAxis.innerTickSize(-chart.width));
            chart.objs.g.select('.y.axis .label').attr("x", -chart.height / 2);
            chart.objs.chartDiv.select('svg').attr("width", chart.width + (chart.margin.left + chart.margin.right)).attr("height", chart.height + (chart.margin.top + chart.margin.bottom));
    
            return chart;
        };
    
        chart.bind = function (selector, chartOptions) {
    
            function getBaseData() {
                if (chartOptions && chartOptions.margin) {
                    chart.margin = margin;
                } else {
                    chart.margin = {top: 15, right: 40, bottom: 35, left: 50};
                }
                if (chartOptions && chartOptions.chartSize) {
                    chart.divWidth = chartOptions.chartSize.width;
                    chart.divHeight = chartOptions.chartSize.height;
                } else {
                    chart.divWidth = 800;
                    chart.divHeight = 400;
                }
    
                chart.width = chart.divWidth - chart.margin.left - chart.margin.right;
                chart.height = chart.divHeight - chart.margin.top - chart.margin.bottom;
    
                if (chartOptions && chartOptions.axisLabels) {
    
                    chart.xAxisLable = chartOptions.axisLabels.xAxis;
                    chart.yAxisLable = chartOptions.axisLabels.yAxis;
                } else {
                    chart.xAxisLable = xGroup;
                    chart.yAxisLable = yValue;
                }
                if (chartOptions && chartOptions.scale === 'log') {
                    chart.yScale = d3.scale.log();
                    chart.yFormatter = logFormatNumber;
                } else {
                    chart.yScale = d3.scale.linear();
                }
    
    
                if (chartOptions && chartOptions.constrainExtremes === true) {
                    var fences = [];
                    for (var cName in chart.groupObjs) {
                        fences.push(chart.groupObjs[cName].metrics.lowerInnerFence);
                        fences.push(chart.groupObjs[cName].metrics.upperInnerFence);
                    }
                    chart.range = d3.extent(fences);
    
                } else {
                    chart.range = d3.extent(chart.data, function (d) {
                        return d[chart.yValue];
                    });
                }
    
                if (chartOptions && chartOptions.colors) {
                    colorFunct = updateColorFunction(chartOptions.colors);
                }
    
    
                chart.yScale.range([chart.height, 0]).domain(chart.range).clamp(true);
                chart.xScale = d3.scale.ordinal().domain(Object.keys(chart.groupObjs)).rangeBands([0, chart.width]);
                chart.objs.yAxis = d3.svg.axis()
                    .scale(chart.yScale)
                    .orient("left")
                    .tickFormat(chart.yFormatter)
                    .outerTickSize(0)
                    .innerTickSize(-chart.width + (chart.margin.right + chart.margin.left));
                chart.objs.xAxis = d3.svg.axis().scale(chart.xScale).orient("bottom").tickSize(5);
    
            }
    
            getBaseData();
    
            chart.objs.mainDiv = d3.select(selector)
                .style("max-width", chart.divWidth + "px");
            chart.objs.mainDiv.append("div")
                .attr("class", "inner-wrapper")
                .style("padding-bottom", (chart.divHeight / chart.divWidth) * 100 + "%")
                .append("div").attr("class", "outer-box")
                .append("div").attr("class", "inner-box");
            chart.chartSelector = selector + " .inner-box";
            chart.objs.chartDiv = d3.select(chart.chartSelector);
            d3.select(window).on('resize.' + chart.chartSelector, chart.update);
    
    
            chart.objs.g = chart.objs.chartDiv.append("svg")
                .attr("class", "chart-area")
                .attr("width", chart.width + (chart.margin.left + chart.margin.right))
                .attr("height", chart.height + (chart.margin.top + chart.margin.bottom))
                .append("g")
                .attr("transform", "translate(" + chart.margin.left + "," + chart.margin.top + ")");
    
            chart.objs.axes = chart.objs.g.append("g").attr("class", "axis");
            chart.objs.axes.append("g")
                .attr("class", "x axis")
                .attr("transform", "translate(0," + chart.height + ")")
                .call(chart.objs.xAxis);
    
            chart.objs.axes.append("g")
                .attr("class", "y axis")
                .call(chart.objs.yAxis)
                .append("text")
                .attr("class", "label")
                .attr("transform", "rotate(-90)")
                .attr("y", -42)
                .attr("x", -chart.height / 2)
                .attr("dy", ".71em")
                .style("text-anchor", "middle")
                .text(chart.yAxisLable);
    
    
            chart.objs.tooltip = chart.objs.mainDiv.append('div').attr('class', 'tooltip');
            for (var cName in chart.groupObjs) {
                chart.groupObjs[cName].g = chart.objs.g.append("g").attr("class", "group");
                chart.groupObjs[cName].g.on("mouseover", function () {
                    chart.objs.tooltip.style("display", null).style("left", (d3.event.pageX) + "px").style("top", (d3.event.pageY - 28) + "px");
                }).on("mouseout", function () {
                    chart.objs.tooltip.style("display", "none");
                }).on("mousemove", tooltipHover(cName, chart.groupObjs[cName].metrics))
            }
            chart.update();
    
            return chart;
        };
    
        chart.renderViolinPlot = function (chartOptions) {
            chart.violinPlots = {};
            chart.violinPlots.plots = {};
            chart.violinPlots.violinOptions = chartOptions;
            var vOpts = chart.violinPlots.violinOptions;
    
            chart.violinPlots.calculateNumBins = function (cGroup) {
                var iqr;
                if (chart.boxPlots) {
                    iqr = chart.groupObjs[cGroup].metrics.iqr
                } else {
                    var quartile1 = d3.quantile(chart.groupObjs[cGroup].values, 0.25);
                    var quartile3 = d3.quantile(chart.groupObjs[cGroup].values, 0.75);
                    iqr = quartile3 - quartile1;
                }
                return Math.max(Math.round(2 * (iqr / Math.pow(chart.groupObjs[cGroup].values.length, 1 / 3))), 50)
            };
    
            function prepareViolin() {
    
                var cName;
                for (cName in chart.groupObjs) {
                    chart.groupObjs[cName].violin = {};
                    chart.groupObjs[cName].violin.objs = {};
                    chart.groupObjs[cName].violin.histogramFunct = d3.layout.histogram().frequency(1);
                }
    
            }
    
            prepareViolin();
    
            chart.violinPlots.change = function (updateOptions) {
                if (updateOptions) {
                    for (var key in updateOptions) {
                        vOpts[key] = updateOptions[key]
                    }
                }
    
                mapObjects(true);
                chart.violinPlots.update()
            };
    
    
            chart.violinPlots.update = function () {
                var cName, cViolinPlot;
    
                for (cName in chart.groupObjs) {
                    cViolinPlot = chart.groupObjs[cName].violin;
    
                    if (vOpts && vOpts.resolution) {
                        cViolinPlot.histogramFunct.bins(vOpts.resolution);
                    } else {
                        cViolinPlot.histogramFunct.bins(chart.violinPlots.calculateNumBins(cName));
                    }
                    cViolinPlot.histogramData = cViolinPlot.histogramFunct(chart.groupObjs[cName].values);
    
                    var groupWidth = {left: null, right: null, middle: null};
                    if (vOpts && vOpts.violinWidth) {
                        groupWidth = updateGroupWidth(vOpts.violinWidth)
                    } else {
                        groupWidth = updateGroupWidth(100)
                    }
    
                    var leftBound = chart.xScale(cName) + groupWidth.left;
                    var rightBound = chart.xScale(cName) + groupWidth.right;
                    var width = (rightBound - leftBound) / 2;
    
                    var xV = chart.yScale.copy();
                    var yV = d3.scale.linear()
                        .range([width, 0])
                        .domain([0, Math.max(chart.range[1], d3.max(cViolinPlot.histogramData, function (d) {
                            return d.y;
                        }))])
                        .clamp(true);
    
                    var area = d3.svg.area()
                        .interpolate('basis')
                        .x(function (d) {
                            return xV(d.x);
                        })
                        .y0(width)
                        .y1(function (d) {
                            return yV(d.y);
                        });
    
                    var line = d3.svg.line()
                        .interpolate('basis')
                        .x(function (d) {
                            return xV(d.x);
                        })
                        .y(function (d) {
                            return yV(d.y);
                        });
    
                    if (cViolinPlot.objs.left.area) {
                        cViolinPlot.objs.left.area
                            .datum(cViolinPlot.histogramData)
                            .attr("d", area);
                    }
    
                    if (cViolinPlot.objs.left.line) {
                        cViolinPlot.objs.left.line
                            .datum(cViolinPlot.histogramData)
                            .attr("d", line);
                    }
    
                    if (cViolinPlot.objs.right.area) {
                        cViolinPlot.objs.right.area
                            .datum(cViolinPlot.histogramData)
                            .attr("d", area);
                    }
                    if (cViolinPlot.objs.right.line) {
                        cViolinPlot.objs.right.line
                            .datum(cViolinPlot.histogramData)
                            .attr("d", line);
                    }
    
                    cViolinPlot.objs.left.g.attr("transform", "rotate(90,0,0)   translate(0,-" + leftBound + ")  scale(1,-1)");
                    cViolinPlot.objs.right.g.attr("transform", "rotate(90,0,0)  translate(0,-" + rightBound + ")");
                }
            };
    
            function mapObjects(clear) {
    
                var cName, cViolinPlot;
    
                if (vOpts && vOpts.colors) {
                    chart.violinPlots.color = updateColorFunction(vOpts.colors);
                } else {
                    chart.violinPlots.color = colorFunct
                }
    
                for (cName in chart.groupObjs) {
                    cViolinPlot = chart.groupObjs[cName].violin;
    
                    if (clear) {
                        cViolinPlot.objs.g.remove()
                    }
    
                    cViolinPlot.objs.g = chart.groupObjs[cName].g.append("g").attr("class", "violin-plot");
                    cViolinPlot.objs.left = {area: null, line: null, g: null};
                    cViolinPlot.objs.right = {area: null, line: null, g: null};
    
                    cViolinPlot.objs.left.g = cViolinPlot.objs.g.append("g");
                    cViolinPlot.objs.right.g = cViolinPlot.objs.g.append("g");
    
                    if (!vOpts || (vOpts && vOpts.showArea !== false)) {
                        cViolinPlot.objs.left.area = cViolinPlot.objs.left.g.append("path")
                            .attr("class", "area")
                            .style("fill", chart.violinPlots.color(cName));
                        cViolinPlot.objs.right.area = cViolinPlot.objs.right.g.append("path")
                            .attr("class", "area")
                            .style("fill", chart.violinPlots.color(cName));
                    }
    
                    if (!vOpts || (vOpts && vOpts.showLine !== false)) {
                        cViolinPlot.objs.left.line = cViolinPlot.objs.left.g.append("path")
                            .attr("class", "line")
                            .attr("fill", 'none')
                            .style("stroke", chart.violinPlots.color(cName));
                        cViolinPlot.objs.right.line = cViolinPlot.objs.right.g.append("path")
                            .attr("class", "line")
                            .attr("fill", 'none')
                            .style("stroke", chart.violinPlots.color(cName));
                    }
                }
    
            }
    
            mapObjects();
    
            d3.select(window).on('resize.' + chart.chartSelector + '.violinPlot', chart.violinPlots.update);
            chart.violinPlots.update();
            return chart.violinPlots;
        };
    
        chart.renderBoxPlot = function (chartOptions) {
            chart.boxPlots = {};
            chart.boxPlots.chartOptions = chartOptions;
            var bOpts = chart.boxPlots.chartOptions;
    
    
            for (var cName in chart.groupObjs) {
                chart.groupObjs[cName].boxPlot = {};
                chart.groupObjs[cName].boxPlot.objs = {};
            }
            function calcOutliers(obj, values, metrics) {
    
                var cExtremes = [];
                var cOutliers = [];
                var cOut, idx;
                for (idx = 0; idx <= values.length; idx++) {
                    cOut = {value: values[idx]};
    
                    if (cOut.value < metrics.lowerInnerFence) {
                        if (cOut.value < metrics.lowerOuterFence) {
                            cExtremes.push(cOut);
                        } else {
                            cOutliers.push(cOut);
                        }
                    } else if (cOut.value > metrics.upperInnerFence) {
                        if (cOut.value > metrics.upperOuterFence) {
                            cExtremes.push(cOut);
                        } else {
                            cOutliers.push(cOut);
                        }
                    }
                }
                obj.outliers = cOutliers;
                obj.extremes = cExtremes;
            }
    
            function calcAllOutliers() {
                if (!bOpts || (bOpts && bOpts.showOutliers !== false)) {
                    for (var cName in chart.groupObjs) {
                        calcOutliers(chart.groupObjs[cName].boxPlot.objs, chart.groupObjs[cName].values, chart.groupObjs[cName].metrics);
                    }
                }
            }
    
            calcAllOutliers();
    
            chart.boxPlots.change = function (updateOptions) {
                if (updateOptions) {
                    for (var key in updateOptions) {
                        bOpts[key] = updateOptions[key]
                    }
                }
                mapObjects(true);
                chart.boxPlots.update()
            };
    
            chart.boxPlots.update = function () {
                var cName, cBoxPlot;
    
                for (cName in chart.groupObjs) {
                    cBoxPlot = chart.groupObjs[cName].boxPlot;
    
                    var groupWidth = {left: null, right: null, middle: null};
                    if (bOpts && bOpts.boxWidth) {
                        groupWidth = updateGroupWidth(bOpts.boxWidth)
                    } else {
                        groupWidth = updateGroupWidth(30)
                    }
                    var leftBound = chart.xScale(cName) + groupWidth.left;
                    var rightBound = chart.xScale(cName) + groupWidth.right;
                    var middle = chart.xScale(cName) + groupWidth.middle;
    
                    var sMetrics = {};
                    for (var attr in chart.groupObjs[cName].metrics) {
                        sMetrics[attr] = null;
                        sMetrics[attr] = chart.yScale(chart.groupObjs[cName].metrics[attr]);
                    }
    
                    if (cBoxPlot.objs.box) {
                        cBoxPlot.objs.box
                            .attr("x", leftBound)
                            .attr('width', rightBound - leftBound)
                            .attr("y", sMetrics.quartile3)
                            .attr("rx", 1)
                            .attr("ry", 1)
                            .attr("height", -sMetrics.quartile3 + sMetrics.quartile1)
                    }
                    if (cBoxPlot.objs.upperWhisker) {
                        cBoxPlot.objs.upperWhisker.fence
                            .attr("x1", leftBound)
                            .attr("x2", rightBound)
                            .attr('y1', sMetrics.upperInnerFence)
                            .attr("y2", sMetrics.upperInnerFence);
                        cBoxPlot.objs.upperWhisker.line
                            .attr("x1", middle)
                            .attr("x2", middle)
                            .attr('y1', sMetrics.quartile3)
                            .attr("y2", sMetrics.upperInnerFence);
    
                        cBoxPlot.objs.lowerWhisker.fence
                            .attr("x1", leftBound)
                            .attr("x2", rightBound)
                            .attr('y1', sMetrics.lowerInnerFence)
                            .attr("y2", sMetrics.lowerInnerFence);
                        cBoxPlot.objs.lowerWhisker.line
                            .attr("x1", middle)
                            .attr("x2", middle)
                            .attr('y1', sMetrics.quartile1)
                            .attr("y2", sMetrics.lowerInnerFence);
                    }
                    if (cBoxPlot.objs.median) {
                        cBoxPlot.objs.median.line
                            .attr("x1", leftBound)
                            .attr("x2", rightBound)
                            .attr('y1', sMetrics.median)
                            .attr("y2", sMetrics.median);
                        cBoxPlot.objs.median.circle
                            .attr("cx", middle)
                            .attr("cy", sMetrics.median)
                    }
    
                    if (cBoxPlot.objs.mean) {
                        cBoxPlot.objs.mean.line
                            .attr("x1", leftBound)
                            .attr("x2", rightBound)
                            .attr('y1', sMetrics.mean)
                            .attr("y2", sMetrics.mean);
                        cBoxPlot.objs.mean.circle
                            .attr("cx", middle)
                            .attr("cy", sMetrics.mean);
                    }
                    var pt;
                    if (cBoxPlot.objs.outliers) {
                        for (pt in cBoxPlot.objs.outliers) {
                            cBoxPlot.objs.outliers[pt].point
                                .attr("cx", middle)
                                .attr("cy", chart.yScale(cBoxPlot.objs.outliers[pt].value));
                        }
                    }
                    if (cBoxPlot.objs.extremes) {
                        for (pt in cBoxPlot.objs.extremes) {
                            cBoxPlot.objs.extremes[pt].point
                                .attr("cx", middle)
                                .attr("cy", chart.yScale(cBoxPlot.objs.extremes[pt].value));
                        }
                    }
                }
            };
    
            function mapObjects(clear) {
                var cName, cBoxPlot;
    
                if (bOpts && bOpts.colors) {
                    chart.boxPlots.colorFunct = updateColorFunction(bOpts.colors);
                } else {
                    chart.boxPlots.colorFunct = colorFunct
                }
    
                for (cName in  chart.groupObjs) {
                    cBoxPlot = chart.groupObjs[cName].boxPlot;
    
                    if (clear) {
                        cBoxPlot.objs.g.remove()
                    }
    
                    cBoxPlot.objs.g = chart.groupObjs[cName].g.append("g").attr("class", "box-plot");
    
                    if (!bOpts || (bOpts && bOpts.showBox !== false)) {
                        cBoxPlot.objs.box = cBoxPlot.objs.g.append("rect")
                            .attr("class", "box")
                            .style("fill", chart.boxPlots.colorFunct(cName));
                    }
    
                    if (!bOpts || (bOpts && bOpts.showMedian !== false)) {
                        cBoxPlot.objs.median = {line: null, circle: null};
                        cBoxPlot.objs.median.line = cBoxPlot.objs.g.append("line")
                            .attr("class", "median");
                        cBoxPlot.objs.median.circle = cBoxPlot.objs.g.append("circle")
                            .attr("class", "median")
                            .attr('r', 3)
                            .style("fill", chart.boxPlots.colorFunct(cName));
                    }
    
                    if (bOpts && bOpts.showMean) {
                        cBoxPlot.objs.mean = {line: null, circle: null};
                        cBoxPlot.objs.mean.line = cBoxPlot.objs.g.append("line")
                            .attr("class", "mean");
                        cBoxPlot.objs.mean.circle = cBoxPlot.objs.g.append("circle")
                            .attr("class", "mean")
                            .attr('r', 3)
                            .style("fill", chart.boxPlots.colorFunct(cName));
                    }
    
                    if (!bOpts || (bOpts && bOpts.showWhiskers !== false)) {
                        cBoxPlot.objs.upperWhisker = {fence: null, line: null};
                        cBoxPlot.objs.lowerWhisker = {fence: null, line: null};
                        cBoxPlot.objs.upperWhisker.fence = cBoxPlot.objs.g.append("line")
                            .attr("class", "upper whisker")
                            .style("stroke", chart.boxPlots.colorFunct(cName));
                        cBoxPlot.objs.upperWhisker.line = cBoxPlot.objs.g.append("line")
                            .attr("class", "upper whisker")
                            .style("stroke", chart.boxPlots.colorFunct(cName));
    
                        cBoxPlot.objs.lowerWhisker.fence = cBoxPlot.objs.g.append("line")
                            .attr("class", "lower whisker")
                            .style("stroke", chart.boxPlots.colorFunct(cName));
                        cBoxPlot.objs.lowerWhisker.line = cBoxPlot.objs.g.append("line")
                            .attr("class", "lower whisker")
                            .style("stroke", chart.boxPlots.colorFunct(cName));
                    }
    
                    if (!bOpts || (bOpts && bOpts.showOutliers !== false)) {
                        if (!cBoxPlot.objs.outliers) calcAllOutliers();
                        var pt;
                        if (cBoxPlot.objs.outliers.length) {
                            var outDiv = cBoxPlot.objs.g.append("g").attr("class", "boxplot outliers");
                            for (pt in cBoxPlot.objs.outliers) {
                                cBoxPlot.objs.outliers[pt].point = outDiv.append("circle")
                                    .attr("class", "outlier")
                                    .attr('r', 2)
                                    .style("fill", chart.boxPlots.colorFunct(cName));
                            }
                        }
    
                        if (cBoxPlot.objs.extremes.length) {
                            var extDiv = cBoxPlot.objs.g.append("g").attr("class", "boxplot extremes");
                            for (pt in cBoxPlot.objs.extremes) {
                                cBoxPlot.objs.extremes[pt].point = extDiv.append("circle")
                                    .attr("class", "extreme")
                                    .attr('r', 2)
                                    .style("stroke", chart.boxPlots.colorFunct(cName));
                            }
                        }
                    }
    
    
                }
            }
    
            mapObjects();
    
            d3.select(window).on('resize.' + chart.chartSelector + '.boxPlot', chart.boxPlots.update);
            chart.boxPlots.update();
    
            return chart.boxPlots;
    
        };
    
        return chart;
    }

}


