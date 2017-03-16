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
 * Renders an SVG phylogram with collapsible branches and selectable leaves from a vjTreeSeries
 * (circular or rectangular, depending on the vjTreeSeries type). Basic usage:
 *
 * var p = new vjSVG_Phylogram({
 *   nodeLabel: function(node) { // labels nodes in the tree
 *      if (node.leafnode) return "leaf " + node.name;
 *      return this.defaultNodeLabel.apply(this, arguments);
 *   },
 *   nodeTooltip: function(node) { // adds tooltips to node symbols and labels
 *      return this.defaultNodeTooltip.apply(this, arguments) + "\n" + foo(node);
 *   },
 *   nodeSymbolSize: function(node) { // controls size of node
 *      return this.defaultNodeSymbolSize.apply(this, arguments) * node.customSizeFactor;
 *   },
 * });
 * p.add(new vjTreeSeries(type: "circular"));
 * p.registerLeafCheckedCallback(nodeName, function(checked) {if(checked) alert(nodeName + " is now checked");});
 * var v = new vjSVGView({ plots: [p], ... });
 */
function vjSVG_Phylogram(source)
{
    vjSVG_Plot.call(this, source);
    if (!this.name) this.name = this.objID;
    if (!this.branchLengthScale) this.branchLengthScale = "linear";

    // determine this experimentally based on leaf label font size
    if (!this.glyphHeight) this.glyphHeight = 0.020;
    if (this.equalizeLeaves == undefined) this.equalizeLeaves = false;
    if (this.enableExpander == undefined) this.enableExpander = true;
    if (this.enableRerooter == undefined) this.enableRerooter = true;

    this.leafCheckedCallbacks = new Object(); // { "nodeId1" : [ {func: function(checked){}, "this": this, param: null}, ... }, .... ], ....}
    this.onclickCallbacks = new Object();

    // FIXME: what if there is more than 1 series in a collection?
    this.depthCoordCache = {};
    this.depthToParent = function(node) {
        if (!node.parent) {
            return 0;
        }
        if (this.branchLengthScale == "none") {
            if (this.collection[0].showRoot || node.parent.parent) {
                return 1;
            } else {
                return 0;
            }
        } else if (this.branchLengthScale == "linear") {
            return node.distance;
        } else if (this.branchLengthScale == "log") {
            if (node.distance && this.collection[0].minAbsDistance) {
                var d = Math.log(Math.E * Math.abs(node.distance) / this.collection[0].minAbsDistance);
                return node.distance > 0 ? d : -d;
            } else {
                return 0;
            }
        }
    };
    this.depthCoord = function(node) {
        if (this.branchLengthScale == "none") {
            return node.depth - this.collection[0].showRoot;
        } else if (this.branchLengthScale == "linear") {
            return node.totalDistance;
        } else if (this.branchLengthScale == "log") {
            if (!this.depthCoordCache.byPath) {
                this.depthCoordCache.byPath = {};
            }
            if (this.depthCoordCache.byPath[node.path] === undefined) {
                this.depthCoordCache.byPath[node.path] = this.depthToParent(node) + (node.parent ? this.depthCoord(node.parent) : 0);
            }
            return this.depthCoordCache.byPath[node.path];
        }
    };
    this.maxDepthCoord = function() {
        var max;
        if (this.branchLengthScale == "none") {
            max = this.collection[0].maxDepth;
        } else if (this.branchLengthScale == "linear") {
            max = this.collection[0].maxTotalDistance;
        } else if (this.branchLengthScale == "log") {
            if (this.depthCoordCache.max === undefined) {
                var findMax = function(node) {
                    if (!node) {
                        return;
                    }
                    var d = this.depthCoord(node);
                    if (this.depthCoordCache.max == undefined || this.depthCoordCache.max < d) {
                        this.depthCoordCache.max = d;
                    }
                    if (node.expanded) {
                        for (var i=0; i<node.children.length; i++) {
                            findMax.call(this, node.children[i]);
                        }
                    }
                };
                findMax.call(this, this.collection[0].tree.root);
            }
            max = this.depthCoordCache.max;
        }
        return max > 0 ? max : 1;
    };
    this.maxLeafCoord = function(type) {
        var max = this.collection[0].numLeaves;
        if (type == "rectangular")
            max--;

        return max>0 ? max : 1;
    };
    this.minmax = function()
    {
        this.min = {x:0, y:0, z:0};
        this.max = {x:1, y:1, z:0};
    };
    this.enumerate = function(operation, params, ischecked, leaforbranch, node)
    {
        if (!this.collection || !this.collection[0] || !this.collection[0].tree || !this.collection[0].tree.root) return;
        return this.collection[0].tree.enumerate(operation, params, ischecked, leaforbranch, node);
    };
    this.accumulate = function(checker, collector, params, ischecked, leaforbranch, node)
    {
        if (!this.collection || !this.collection[0] || !this.collection[0].tree || !this.collection[0].tree.root) return;
        return this.collection[0].tree.accumulate(checker, collector, params, ischecked, leaforbranch, node);
    };
    this.crdNodeToPlot = function(type, node, param, parent_crd) {
        if (type == "unrooted" && parent_crd && node.parent) {
            // unrooted node is drawn on a line from parent_crd to node's leafCoord on circle boundary
            var r = this.crdTreeToPlot(type, node.leafCoord, this.depthToParent(node), {polar: true}).r;
            var edge_crd = this.crdTreeToPlot(type, node.leafCoord, this.maxDepthCoord());
            var dist = Math.sqrt((edge_crd.x - parent_crd.x) * (edge_crd.x - parent_crd.x) + (edge_crd.y - parent_crd.y) * (edge_crd.y - parent_crd.y));
            return {x : parent_crd.x + (edge_crd.x - parent_crd.x) * r / dist, y: parent_crd.y + (edge_crd.y - parent_crd.y) * r / dist, z: 0};
        } else {
            return this.crdTreeToPlot(type, node.leafCoord, this.depthCoord(node), param);
        }
    };
    this.crdTreeToPlot = function(type, leafCoord, depthCoord, param) {
        if (type === "circular" || type === "unrooted") {
            var r = 0.45 * depthCoord / this.maxDepthCoord();
            if (this.collection[0].showRoot > 0)
                r = 0.05 + 0.40 * depthCoord / this.maxDepthCoord();

            var theta = leafCoord * 2 * Math.PI / this.maxLeafCoord(type);

            if (param && param.polar)
                return {r: r, theta: theta};
            else
                return {x: 0.5 + r * Math.cos(theta), y: 0.5 + r * Math.sin(theta), z: 0};
        } else { // rectangular
            var label_space = this.nodeLabel ? 0.3 : 0;
            var crd = {x: (1 - label_space) * (depthCoord / this.maxDepthCoord()), y: 1 - leafCoord / this.maxLeafCoord(type), z: 0};
            return crd;
        }
    };
    this.crdOrigin = function(type) {
        if (type === "circular" || type === "unrooted") {
            return {x: 0.5, y: 0.5, z: 0};
        } else {
            return {x: 0, y: 0.5, z: 0};
        }
    };

    this.defaultSeriesSymbolSize = function() {
        return this.isNodeSymbolSizeUnscaled ? 5 : 0.012;
    }
    this.constructorFunction = new Object(); // elements indexed by vjTreeSeries.type
    // series is a vjTreeSeries instance
    this.constructorFunction.rectangular = function(series)
    {
        if(!series.symbolSize) series.symbolSize=this.defaultSeriesSymbolSize();
        // FIXME: can series.tree have no nodes?
        this.depthCoordCache = {};
        this.constructorHelper("rectangular", series, series.tree.root);
    };
    this.constructorFunction.circular = function(series)
    {
        if(!series.symbolSize) series.symbolSize=this.defaultSeriesSymbolSize();
        // FIXME: can series.tree have no nodes?
        this.leftLabels = [];
        this.rightLabels = [];
        this.depthCoordCache = {};
        this.constructorHelper("circular", series, series.tree.root);
        this.bounceLabels(this.leftLabels, this.glyphHeight, "y");
        this.bounceLabels(this.rightLabels, this.glyphHeight, "y");
    };
    this.constructorFunction.unrooted = function(series)
    {
        if(!series.symbolSize) series.symbolSize=this.defaultSeriesSymbolSize();
        // FIXME: can series.tree have no nodes?
        this.leftLabels = [];
        this.rightLabels = [];
        this.depthCoordCache = {};
        this.constructorHelper("unrooted", series, series.tree.root);
        this.bounceLabels(this.leftLabels, this.glyphHeight, "y");
        this.bounceLabels(this.rightLabels, this.glyphHeight, "y");
    };

    this.constructorHelper = function(type, series, node, parent_crd)
    {
        var crd = this.crdNodeToPlot(type, node, null, parent_crd);
        var crd_node = cpyObj(crd);

        if (series.showRoot > node.depth) {
            for (var i=0; i<node.children.length; i++)
                this.constructorHelper(type, series, node.children[i], crd_node);
            return;
        }

        if (node.children.length && node.expanded) {
            if (type != "unrooted") {
                var minLeafCoord = node.leafCoord;
                var maxLeafCoord = node.leafCoord;
                for (var i=0; i<node.children.length; i++) {
                    if (node.children[i].leafCoord < minLeafCoord)
                        minLeafCoord = node.children[i].leafCoord;
                    if (node.children[i].leafCoord > maxLeafCoord)
                        maxLeafCoord = node.children[i].leafCoord;
                }
                if (type === "rectangular") {
                    this.children.push(new vjSVG_line({
                        crd1: this.crdTreeToPlot(type, minLeafCoord, this.depthCoord(node)),
                        crd2: this.crdTreeToPlot(type, maxLeafCoord, this.depthCoord(node))
                    }));
                } else if (type === "circular") {
                    var polar1 = this.crdTreeToPlot(type, minLeafCoord, this.depthCoord(node), {polar:true});
                    var polar2 = this.crdTreeToPlot(type, maxLeafCoord, this.depthCoord(node), {polar:true});
                    this.children.push(new vjSVG_arc({
                        brush: {"fill-opacity": 0},
                        crd: this.crdOrigin(type),
                        startAngle: polar1.theta * 180 / Math.PI,
                        endAngle: polar2.theta * 180 / Math.PI,
                        rx: polar1.r, ry: polar2.r,
                        rotation: 0,
                        howToClose: "open"
                    }));
                }
            }
        } else if (this.stretchChildlessNodes) {
            crd = this.crdTreeToPlot(type, node.leafCoord, this.maxDepthCoord());
            this.children.push(new vjSVG_line({
                crd1: cpyObj(crd_node),
                crd2: cpyObj(crd),
                brush: {"fill-opacity": 0},
                pen: {"stroke": "#aaaaaa", "stroke-width": DefaultPen["stroke-width"]}
            }));
        }

        if (node.parent) {
            if (type == "unrooted") {
                this.children.push(new vjSVG_line({
                    crd1: cpyObj(crd_node),
                    crd2: cpyObj(parent_crd)
                }));
            } else {
                this.children.push(new vjSVG_line({
                    crd1: cpyObj(crd_node),
                    crd2: this.crdTreeToPlot(type, node.leafCoord, this.depthCoord(node.parent))
                }));
            }
        }

        var plot = this; // for use in closures

        var label = this.nodeLabel ? this.nodeLabel(node, series) : undefined;
        if (label) {
            var labelCrd = {x: crd.x, y: crd.y, z: crd.z};

            var textAnchor = "start";
            var textWidth;
            var bounceList = null;
            var dx = 0;
            var dy = 0;

            if (type === "rectangular") {
                if (series.rectangularLabelInline || node.leafnode || !node.expanded || !node.children.length) {
                    if (this.isNodeSymbolSizeUnscaled)
                        dx = 1.5 * this.nodeSymbolSize(node, series) + "px";
                    else
                        labelCrd.x += 1.5 * this.nodeSymbolSize(node, series);

                    //labelCrd.y -= this.glyphHeight/2;
                    dy = ".3em";
                    textWidth = this.max.x - labelCrd.x;
                } else {
                    if (this.isNodeSymbolSizeUnscaled)
                        dy = this.nodeSymbolSize(node, series) + "px";
                    else
                        labelCrd.y += this.nodeSymbolSize(node, series);

                    textWidth = 2 * Math.min(this.max.x - labelCrd.x, labelCrd.x - this.min.x);
                    textAnchor = "middle";
                }
            } else if (type === "circular" || type === "unrooted") {
                var polar = this.crdNodeToPlot(type, node, {polar:true});
                // on sides of circle, align label to center of leaf
                if (polar.theta < Math.PI/3 || polar.theta > 2*Math.PI/3)
                    dy = ".3em";
//                    labelCrd.y -= this.glyphHeight/2;
                // on bottom of circle, align label to bottom of leaf
                if (polar.theta >= 4*Math.PI/3 && polar.theta <= 5*Math.PI/3)
                    dy = "1em";
                // on left of circle, move label a bit to left and anchor its end to the ccoordinate
                if (polar.theta > Math.PI/2 && polar.theta < 3*Math.PI/2) {
                    textAnchor = "end";
                    if (this.isNodeSymbolSizeUnscaled)
                        dx = -1.5 * this.nodeSymbolSize(node, series) + "px";
                    else
                        labelCrd.x -= 1.5 * this.nodeSymbolSize(node, series);

                    textWidth = labelCrd.x - this.min.x;
                    bounceList = this.leftLabels;
                } else {
                    // on right side of circle, move label a tiny bit to the right
                    if (this.isNodeSymbolSizeUnscaled)
                        dx = 1.5 * this.nodeSymbolSize(node, series) + "px";
                    else
                        labelCrd.x += 1.5 * this.nodeSymbolSize(node, series);

                    textWidth = this.max.x - labelCrd.x;
                    bounceList = this.rightLabels;
                }
            }
            var labelObj = new vjSVG_text({
                font: this.nodeLabelFont(node, series),
                text: label,
                anchor: textAnchor,
                ellipsizeWidth: textWidth,
                crd: labelCrd,
                dx: dx,
                dy: dy,
                title: this.nodeTooltip(node, series, true),
                handler: {onclick: function() {
                    plot.callOnclickCallbacks(node.path, "onLabel");
                }}
            });
            this.children.push(labelObj);
            if (bounceList)
                bounceList.push(labelObj);
        }

        if (node.expanded)
            for (var i=0; i<node.children.length; i++)
                this.constructorHelper(type, series, node.children[i], crd_node);

        /* We draw the node symbols last so they go on top of lines etc. */
        if (node.leafnode) {
            if (this.leafSymbolType) {
                var leafSvgID = this.name + " leaf id=" + node.name;
                var leafSymbol = new vjSVG_symbol({
                    definition: this.leafSymbolType,
                    crd:crd,
                    size: this.nodeSymbolSize(node, series),
                    isUnscaled : this.isNodeSymbolSizeUnscaled,
                    svgID: leafSvgID,
                    brush: this.nodeSymbolBrush(node, series),
                    handler: {onclick: function(ir, svgElt, evt) {
                        if (plot.enableRerooter && gKeyCtrl) {
                            series.reroot(node.orig_tree_path ? node.orig_tree_path : node.path);
                            plot.children = new Array();
                            series.refreshWithoutRebuildingTree();
                        } else {
                            plot.mimicLeafChecked(node.name, !node.checked, node, svgElt);
                        }
                        plot.callOnclickCallbacks(node.path, "onSymbol");
                    }},
                    title: this.nodeTooltip(node, series)
                });
                // TODO: should we run mimicLeafChecked(node.name, node.checked) now?

                if (node.checked)
                    leafSymbol.brush.fill = this.nodeCheckedColor(node); //"red";

                this.children.push(leafSymbol);
            }
        } else if (node.expanded && node.children.length &&(!node.childrenCnt||node.childrenCnt <=node.children.length)) { // expanded inner node
            this.children.push(new vjSVG_symbol({
                definition:"box-minus",
                crd:crd,
                size: this.nodeSymbolSize(node, series),
                isUnscaled : this.isNodeSymbolSizeUnscaled,
                brush: this.nodeSymbolBrush(node, series),
                handler: {onclick: function() {
                    if (plot.enableExpander || plot.enableRerooter) {
                        if (plot.enableRerooter && gKeyCtrl) {
                            series.reroot(node.orig_tree_path ? node.orig_tree_path : node.path);
                        } else {
                            plot.nodeExpander(node, false);
                        }
                        plot.children = new Array();
                        series.refreshWithoutRebuildingTree();
                    }
                    plot.callOnclickCallbacks(node.path, "onSymbol");
                }},
                title: this.nodeTooltip(node, series)
            }));
        } else { // collapsed inner node
            this.children.push(new vjSVG_symbol({
                definition:"box-plus",
                crd:crd,
                size: this.nodeSymbolSize(node, series),
                isUnscaled : this.isNodeSymbolSizeUnscaled,
                brush: this.nodeSymbolBrush(node, series),
                handler: {onclick: function() {
                    if (plot.enableExpander) {
                        plot.nodeExpander(node, true);
                        plot.children = new Array();
                        series.refreshWithoutRebuildingTree();
                    }
                    plot.callOnclickCallbacks(node.path, "onSymbol");
                }},
                title: this.nodeTooltip(node, series)
            }));
        }
    };

    this.preferredScale = function(proposed) {
        // Estimate: 50 pixels for average branch; 20 pixels of vertical space per leaf in rectangular mode
        if (!this.collection[0])
            return proposed;

        var prefBranch = 50;
        var prefVertSep = 20;
        var prefScale = {x:proposed.x, y:proposed.y, z:proposed.z};
        if (this.collection[0].type == "rectangular") {
            prefScale.x = this.collection[0].maxDepth * prefBranch;
            prefScale.y = this.collection[0].numLeaves * prefVertSep;
        } else if (this.collection[0].type == "circular" || this.collection[0].type == "unrooted") {
            prefScale.x = prefScale.y = this.collection[0].maxDepth * prefBranch;
        }
        return prefScale;
    };

    this.nodeExpandDepth = 3;

    this.nodeExpander = function(node, value, depth, recursing)
    {
        if (depth == undefined || depth == null)
            depth = this.nodeExpandDepth;

        if (depth <= 0)
            return;

        node.expanded = value;
      //  alert(node.childrenCnt)
        if ( node.childrenCnt && node.childrenCnt > node.children.length && !recursing) {
            this.collection[0].getMoreNodes(node);
            return;
        }
        for (var i=0; i<node.children.length; i++)
            this.nodeExpander(node.children[i], value, depth-1, true);
    };

    // A Monte-Carlo type function to prevent a list of items from overlapping each other too much
    // along one axis.
    this.bounceLabels = function(list, separation, axis)
    {
        if (!list || !list.length)
            return;

        var crdOrig = [];
        for (var i=0; i<list.length; i++)
            crdOrig.push(getCrd(i));

        for (var n=0; n<8; n++) {
            var totalBadness = 0;
            var listBadness = [];

            for (var i=0; i<list.length; i++) {
                var badness = getBadness(i);
                listBadness.push(badness);
                totalBadness += badness;
            }

            if (totalBadness < separation)
                break;

            for (var i=0; i<list.length; i++) {
                if (!listBadness[i])
                    continue;

                for (var m = 4; m >= 1; m--) {
                    var crd = getCrd(i) + separation * (Math.random() - 0.5) * m / n;
                    var badness = getBadness(i, crd);
                    if (badness < listBadness[i]) {
                        setCrd(i, crd);
                        listBadness[i] = badness;
                    }
                }
            }
        }

/*
        var crdNew = [];
        for (var i=0; i<list.length; i++)
            crdNew.push(getCrd(i));
*/

        return;

        function getCrd(i) {
            return list[i].crd[axis];
        }

        function setCrd(i, crd) {
            list[i].crd[axis] = crd;
        }

        function getBadness(i, crd) {
            var badness = 0;

            if (crd === undefined)
                crd = getCrd(i);

            var d = (crd - crdOrig[i]) / separation;
            badness += d*d;

            for (var j=0; j<list.length; j++) {
                if (i == j)
                    continue;

                var sepij = Math.abs(crd - getCrd(j));

                if (sepij >= separation)
                    continue;

                if (sepij <= separation/100)
                    d = 100;
                else
                    d = separation/sepij;

                badness += d;
            }
            return badness;
        }
    };

    this.nodeFindByName = function(nodeName)
    {
        return this.collection[0].tree.findByName(nodeName);
    };

    // rotate through the HCL color wheel based on node's leaf coordinate
    this.defaultNodeCheckedColor = function(node, wantHCL)
    {
        var hue = this.crdTreeToPlot("circular", node.leafCoord, this.depthCoord(node), {polar:true}).theta * 180 / Math.PI;
        var color = vjHCL(hue, 0.7, 0.4);
        if (wantHCL)
            return color;
        else
            return color.hex();
    };
    if (!this.nodeCheckedColor)
        this.nodeCheckedColor = this.defaultNodeCheckedColor;

    this.defaultNodeTooltip = function(node, series, isLabelText)
    {
        if (isLabelText)
            return null;

        var ret;
        if (node.leafnode) {
            ret = node.checked ? "Click to unselect" : "Click to select";
        } else {
            ret = node.expanded ? "Click to collapse" : "Click to expand";
        }

        if (this.enableRerooter) {
            ret += "\nCtrl+click to set as root";
        }
        return ret;
    };
    if (!this.nodeTooltip)
        this.nodeTooltip = this.defaultNodeTooltip;

    this.defaultNodeLabel = function(node, series)
    {
        return node.leafnode ? node.name : null;
    };
    if (this.nodeLabel == undefined)
        this.nodeLabel = this.defaultNodeLabel;

    this.defaultNodeLabelFont = function(node, series)
    {
        var ret = { "font-size": 9, "font-weight": "bold" };
        if (!node.expanded) {
            ret["font-size"] = 8;
            ret["font-style"] = "italic";
        }
        return ret;
    };
    if (!this.nodeLabelFont)
        this.nodeLabelFont = this.defaultNodeLabelFont;

    this.defaultNodeSymbolBrush = function(node, series)
    {
        return {fill:"white", "fill-opacity":1};
    };
    if (!this.nodeSymbolBrush)
        this.nodeSymbolBrush = this.defaultNodeSymbolBrush;

    this.defaultNodeSymbolSize = function(node, series)
    {
        var size = series ? series.symbolSize : this.defaultSeriesSymbolSize();

        // want expand/collapse boxes a bit larger by default for easier clicking
        if (!node.leafnode)
            size *= 1.5;
        return size;
    }
    if (!this.nodeSymbolSize)
        this.nodeSymbolSize = this.defaultNodeSymbolSize;
    this.isNodeSymbolSizeUnscaled = true;

    this.defaultLeafSymbolType = "circle";
    if (this.leafSymbolType == undefined)
        this.leafSymbolType = this.defaultLeafSymbolType;

    this.mimicLeafChecked = function(nodeName, checked, node, svgElt)
    {
        if (!node)
            node = this.collection[0].tree.findByName(nodeName);

        if (!node)
            return;

        node.checked = checked;

        if (!svgElt)
            svgElt = gObject(this.name + " leaf id=" + nodeName);
        if (svgElt) {
            svgElt.style.fill = checked ? this.nodeCheckedColor(node) : "white";
            vjSVGSetTrivialChild(svgElt, "title", this.nodeTooltip(node, this.collection[0]), {unshift:true});
        }

        var c = this.leafCheckedCallbacks[nodeName];
        if (!c)
            return;

        for (var i=0; i<c.length; i++) {
            c[i].callback.call(c[i]["this"] ? c[i]["this"] : this, node.checked, c[i].param);
        }
    };

    this.registerLeafCheckedCallback = function(nodeName, callback, callbackParam, callbackThis)
    {
        if (!this.leafCheckedCallbacks[nodeName])
            this.leafCheckedCallbacks[nodeName] = new Array();
        this.leafCheckedCallbacks[nodeName].push({callback: callback, param: callbackParam, "this": callbackThis});
    };

    this.registerOnclickCallback = function(node, callback, callbackParam, callbackThis, options)
    {
        if (!options)
            options = { onSymbol: true, onLabel: false };

        if (!this.onclickCallbacks[node.path])
            this.onclickCallbacks[node.path] = new Array();
        this.onclickCallbacks[node.path].push({callback: callback, param:callbackParam, "this": callbackThis, options: options});
    };

    this.callOnclickCallbacks = function(nodePath, type)
    {
        var c = this.onclickCallbacks[nodePath];
        if (!c)
            return;

        var node = this.collection[0].tree.findByPath(nodePath);

        if (!node)
            return;

        for (var i=0; i<c.length; i++) {
            if (type && !c[i].options[type])
                continue;

            c[i].callback.call(c[i]["this"] ? c[i]["this"] : this, node, c[i].param);
        }
    };
}
