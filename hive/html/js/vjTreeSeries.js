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
 * s = new vjSVG_TreeSeries({
 *  name: "dsFoo",
 *  url: "qp_data://foo.tre"
 *    dataFormat: "newick", // tree serialization format - "newick" (the default) or "csv"
 * });
 *
 * // data fields
 * s.tree;
 * s.maxHeight;
 * s.maxTotalDistance;
 * s.numLeaves;
 * s.numNodes;
 * s.skipOnDataLoaded // if true, onDataLoaded() will exit immediately
 * // for each node in s.tree
 * node.height; // steps from root
 * node.totalDistance; // distance from root
 * node.leafCoord; // for leaves: order in a leaf traversal; for inner nodes: mean of leafCoord of children
 * It's assumed that node.leafnode is true iff node.children.length === 0.
 */
function vjTreeSeries(source)
{
    vjDataSeries.call(this, source);
    if (!this.dataFormat) this.dataFormat="newick";
    if (this.showRoot == undefined) this.showRoot = 0; // nodes with depth below this are hidden
    if (this.rectangularLabelInline == undefined) this.rectangularLabelInline = false; // space for a fake middle child node (e.g. for a label in a phylogram)

    this.window = undefined;
    this.bumpRawData = undefined;
    this.scaleRawData = undefined;
    this.isTreeSeries = true;

    this.onDataLoaded = function(callback, content, param)
    {
        if (this.skipOnDataLoaded)
            return;

        if (this.dataFormat === "newick") {
            if (this.precompute)
                alert("DEVELOPER ALERT: vjTreeSeries does not support precompute() for Newick format");

            if (this.tree) {
                if (!this.maintainPreviousData)
                    this.tree.empty();
            } else
                this.tree = new vjTree();

            this.tree.parseNewick(content, this.tree.root);
        } else if (this.dataFormat === "csv") {
            var tbl=new vjTable(content, 0, vjTable_propCSV);
            if (this.precompute)
                tbl.enumerate(this.precompute, this);

            if (this.tree) {
                if (!this.maintainPreviousData)
                    this.tree.empty();
                this.tree.parseTable(tbl, this.tree.root);
            } else
                this.tree = new vjTree(tbl);
        }

        this.maintainPreviousData = false;

        this.parseRawData();
        this.call_callbacks("series_loaded");
    };

    this.has_nontrivial_data = function() {
        return this.state == "done" && this.tree && (this.tree.root.childrenCnt || this.tree.root.children.length);
    };

    this.reroot = function(orig_tree_path) {
        if (!this.orig_tree) {
            this.orig_tree = this.tree;
        }
        
        function copyRerooted(dest, src, no_src_parent, no_src_child) {
            dest.orig_tree_path = src.path;
            dest.children = [];
            dest.childrenCnt = src.childrenCnt;
            for (var i=0; i<src.children.length; i++) {
                if (no_src_child === i) {
                    continue;
                }
                var dest_child = new vjTreeNode({name: src.children[i].name, expanded: true, depth: dest.depth + 1, distance: src.children[i].distance, leafnode: src.children[i].leafnode});
                dest_child.parent = dest;
                dest.children.push(dest_child);
                copyRerooted(dest_child, src.children[i], true);
            }
            if (src.parent && !no_src_parent) {
                // trivial root elimination
                if (!src.parent.parent && src.parent.children.length == 2) {
                    var src_sibling = src.parent.children[0] === src ? src.parent.children[1] : src.parent.children[0];
                    var dest_child = new vjTreeNode({name: src_sibling.name, expanded: true, depth: dest.depth + 1, distance: src.distance + src_sibling.distance, leafnode: src_sibling.leafnode});
                    dest_child.parent = dest;
                    dest.children.push(dest_child);
                    copyRerooted(dest_child, src_sibling, true);
                } else {
                    var ic = 0;
                    for (; ic < src.parent.children.length; ic++) {
                        if (src == src.parent.children[ic]) {
                            break;
                        }
                    }
                    var dest_child = new vjTreeNode({name: src.parent.name, expanded: true, depth: dest.depth + 1, distance: src.distance, leafnode: src.parent.leafnode});
                    dest_child.parent = dest;
                    dest.children.push(dest_child);
                    copyRerooted(dest_child, src.parent, false, ic);
                }
            }
        }

        this.rerooted_tree = new vjTree();
        var orig_root;
        if (orig_tree_path) {
            orig_root = this.orig_tree.findByPath(orig_tree_path);
        }
        if (!orig_root) {
            orig_root = this.orig_tree.root;
        }
        this.rerooted_tree.root.name = orig_root.name;
        this.rerooted_tree.root.expanded = orig_root.expanded;
        copyRerooted(this.rerooted_tree.root, orig_root);
        this.rerooted_tree.deanonymize(this.rerooted_tree.root);
    };

    this.parseRawData = function(allowNegativeDistance) {
        if (this.postcompute)
            this.tree.enumerate(this.postcompute, this);

        this.maxDepth = 0;
        this.maxTotalDistance = 0;
        this.minDistance = undefined;
        this.numLeaves = 0;
        this.numNodes = 0;
        if (this.rerooted_tree) {
            this.tree = this.rerooted_tree;
        } else if (this.type == "unrooted") {
            if (!this.unrooted_orig_tree) {
                this.orig_tree = this.tree;
                function countChildLeaves(node) {
                    node.countLeaves = { children: [], parent: 0, min: 0, max: 0, imbalance: 0 };
                    if (!node.children.length) {
                        return 1;
                    }
                    var sum = 0;
                    for(var i=0; i<node.children.length; i++) {
                        node.countLeaves.children[i] = countChildLeaves(node.children[i]);
                        sum += node.countLeaves.children[i];
                    }
                    return sum;
                };
                countChildLeaves(this.orig_tree.root);
                function countParentLeaves(node) {
                    node.countLeaves.min = node.countLeaves.parent;
                    node.countLeaves.max = node.countLeaves.parent;
                    var least_imbalanced_child = null;
                    for (var i=0; i<node.children.length; i++) {
                        var child = node.children[i];
                        child.countLeaves.parent = node.countLeaves.parent;
                        for (var j=0; j<node.countLeaves.children.length; j++) {
                            if (i != j) {
                                child.countLeaves.parent += node.countLeaves.children[j];
                            }
                        }
                        node.countLeaves.min = Math.min(node.countLeaves.min, node.countLeaves.children[i]);
                        node.countLeaves.max = Math.max(node.countLeaves.max, node.countLeaves.children[i]);
                        countParentLeaves(child);
                        if (least_imbalanced_child) {
                            if (child.countLeaves.imbalance < least_imbalanced_child.countLeaves.imbalance) {
                                least_imbalanced_child = child;
                            }
                        } else {
                            least_imbalanced_child = child;
                        }
                    }
                    node.countLeaves.imbalance = node.children.length ? node.countLeaves.max - node.countLeaves.min : Infinity;
                    if (least_imbalanced_child && least_imbalanced_child.countLeaves.imbalance < node.countLeaves.imbalance) {
                        return least_imbalanced_child;
                    } else {
                        return node;
                    }
                }
                var least_imbalanced_node = countParentLeaves(this.orig_tree.root);
                this.reroot(least_imbalanced_node.path);
                this.unrooted_orig_tree = this.rerooted_tree;
                this.tree = this.unrooted_orig_tree;
                delete this.rerooted_tree;
            }
            this.tree = this.unrooted_orig_tree;
        } else if (this.orig_tree) {
            this.tree = this.orig_tree;
        }
        pass1.call(this, this.tree.root);
        if (this.type != "rectangular" || !this.rectangularLabelInline)
            pass2.call(this, this.tree.root);


        
        // find height, totalDistance; find leafCoordinate for leaves
        function pass1(node) {
            if (!node)
                return;

            this.numNodes++;
            if (this.showRoot <= node.depth) {
                if (!node.distance)
                    node.distance = 0;

                if (node.parent && this.showRoot <= node.parent.depth)
                    node.totalDistance = node.distance + node.parent.totalDistance;
                else
                    // rootlike nodes have a distance coordinate of 0
                    node.totalDistance = 0;

                if (node.totalDistance > this.maxTotalDistance)
                    this.maxTotalDistance = node.totalDistance;
                if (node.depth > this.maxDepth + this.showRoot)
                    this.maxDepth = node.depth - this.showRoot;
                if (node.distance && (this.minAbsDistance === undefined || this.minAbsDistance > Math.abs(node.distance))) {
                   this.minAbsDistance = Math.abs(node.distance);
                }
                if (!node.expanded || !node.children.length)
                    node.leafCoord = node.minLeafCoord = node.maxLeafCoord = this.numLeaves++;
            }

            if (node.expanded) {
                if (!allowNegativeDistance) {
                    // clamp distances to non-negative value, while attempting to preserve their sum
                    var dsum = 0;
                    for (var i=0; i<node.children.length; i++) {
                        if (!node.children[i].distance)
                            node.children[i].distance = 0;
                        dsum += node.children[i].distance;
                    }
                    dsum = Math.max(dsum, 0);
                    var dcum = 0;
                    for (var i=0; i<node.children.length; i++) {
                        var d = node.children[i].distance;
                        var maxd = Math.max(0, Math.min(dsum, dsum - dcum));
                        if (d < 0) {
                            node.children[i].distance = 0;
                        } else if (d > maxd) {
                            node.children[i].distance = maxd;
                        }
                        dcum += d;
                    }
                }

                var mid = parseInt(node.children.length/2);
                for (var i=0; i<mid; i++)
                    pass1.call(this, node.children[i]);
                if (this.type == "rectangular" && this.rectangularLabelInline && node.expanded && node.children.length)
                    node.leafCoord = node.minLeafCoord = node.maxLeafCoord = this.numLeaves++;
                for (var i=mid; i<node.children.length; i++)
                    pass1.call(this, node.children[i]);
            }
        }

        // find leafCoord for inner nodes
        function pass2(node) {
            if (!node || node.leafnode || !node.expanded || !node.children.length)
                return;

            var sum = 0;
            node.minLeafCoord = undefined;
            node.maxLeafCoord = undefined;
            for (var i=0; i<node.children.length; i++) {
                pass2.call(this, node.children[i]);
                if (this.showRoot <= node.depth) {
                    sum += node.children[i].leafCoord;
                    if (node.minLeafCoord === undefined) {
                        node.minLeafCoord = node.children[i].leafCoord;
                    } else {
                        node.minLeafCoord = Math.min(node.minLeafCoord, node.children[i].leafCoord);
                    }
                    if (node.maxLeafCoord === undefined) {
                        node.maxLeafCoord = node.children[i].leafCoord;
                    } else {
                        node.maxLeafCoord = Math.max(node.maxLeafCoord, node.children[i].leafCoord);
                    }
                }
            }
            if (this.showRoot <= node.depth) {
                node.leafCoord = sum / node.children.length;
                if (node.minLeafCoord === undefined) {
                    node.minLeafCoord = node.leafCoord;
                }
                if (node.maxLeafCoord === undefined) {
                    node.maxLeafCoord = node.leafCoord;
                }
            }
        }
    };

    this.refreshWithoutRebuildingTree = function() {
        this.skipOnDataLoaded = true;
        this.parseRawData(); // recalculate tree statistics if existing nodes were modified
        this.call_refresh_callbacks();
        delete this.skipOnDataLoaded;
    };

    this.getMoreNodes = function(node) {
        if (!this.cmdMoreNodes)
            return;

        this.url = evalVars(this.cmdMoreNodes, "$(", ")", node);
        this.maintainPreviousData = true;
        this.load();
    };
}

//# sourceURL = getBaseUrl() + "/js/vjTreeSeries.js"