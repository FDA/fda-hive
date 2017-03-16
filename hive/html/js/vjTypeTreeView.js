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
function vjTypeTreeView(viewer) {
    vjTreeView.call(this, viewer);

    this.autoexpand = "all";
    this.showLeaf = true;
    this.preTreatNodesBeforeParsing = function(tbl) {
        if (tbl.rows) {
            // vjTree constructor expects parents of a node to be in source table before children
            tbl.rows.sort(function(a, b) { return a.path < b.path ? -1 : a.path > b.path ? 1 : 0; });
            tbl.rows.forEach(function(node) {
                // vjTree.findByPath() expects path to be made of node.name items
                node.type_name = node.name;
                node.name = node.id;
            });
        }
    }
    this.postcompute = function(param, node) {
        node.children.sort(function(a, b) {
            return cmpNatural(a.type_name, b.type_name);
        });
        if (node.title) {
            node.title = node.type_name + " &mdash; " + node.title;
        }
        if (node.id) {
            node.description = node.id + (node.description ? ", " + node.description : "");
        }
        node.leafnode = !node.children || !node.children.length;
    };

    if (!this.data) {
        this.data = "dsTypeTree_" + this.objCls;
        vjDS.add("Type Hierarchy", this.data, "http://?cmd=typeTree&utype2=1&useTypeDomainId=1&useTypeUPObj=1");
    }
}

//# sourceURL = getBaseUrl() + "/js/vjTypeTreeView.js"