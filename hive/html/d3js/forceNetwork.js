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
function vjD3JS_ForceNetwork ( viewer )
{
    loadCSS("d3js/css/forceNetwork.css");
    vjD3View.call(this,viewer); // inherit default behaviors of the DataViewer
    
    this.data = viewer.data;
    viewer.marin ? this.margin = viewer.margin : this.margin = {top:30, right:30, bottom:30, left:30};
    viewer.width ? this.width = viewer.width : this.width = 960;
    viewer.height ? this.height = viewer.height : this.height = 500;
    
    viewer.omitStart ? this.omitStart = viewer.omitStart : this.omitStart = "__";
    viewer.uniqueKeyStart ? this.uniqueKeyStart = viewer.uniqueKeyStart : this.uniqueKeyStart = "@";
    
    this.onClickCallback = viewer.onClickCallback;
    this.dicOfPos = {};
    this.nodes = [];
    this.links = [];
    
    this.d3Compose=function(content, parsedTree)
    {
        var tThis = this;
        //just in case there are some errors. to avoid the page from crashing
        if (!content  || content.indexOf("error") == 0)
            return;
        
        var svg = this.d3svg;
        
        var jsonStruct;
        eval("jsonStruct="+content+";");        
        var allLinks = this.iterateJSON (jsonStruct, "root", [], "root", 1);
        
        //var root1 = this.iterateJSON2 (jsonStruct, "root", [], "root", 1);
        //this.actualRoot = {name:"root", x:this.width/2, y:this.height/2, children:root1};
        
        var node, link, root;
        
        var nodes = {};

        // Compute the distinct nodes from the links.
        allLinks.forEach(function(link) {
            link.source = nodes[link.source] || (nodes[link.source] = {
                    name: link.source, 
                    realName: link.realSource, 
                    depth: link.depth, 
                    actualNode: link.actualNode});
            link.target = nodes[link.target] || (nodes[link.target] = {
                    name: link.target, 
                    realName: link.realTarget, 
                    depth: link.depth, 
                    actualNode: link.actualNode});
        });
        
//        var aa = this.concatinateNodes (this.nodes, d3.values(nodes));
//        var bb = this.concatinateLinks (this.links, allLinks);
        var aa = d3.values(nodes);
        var bb = allLinks;
        

//        this.links = bb;
//        this.nodes = aa;
        
        var force = d3.layout.force()
            .nodes(aa)
            .links(bb)
            .size([this.width - this.margin.left - this.margin.right, this.height - this.margin.top - this.margin.bottom])
            .linkDistance(this.width/15)
            .charge(-this.margin.left*5)
            .on("tick", tick)
            .start();
        this.nodes = force.nodes();
        this.links = force.links();
        
        var svg = this.d3svg
            .attr("width", this.width + this.margin.right + this.margin.left)
            .attr("height", this.height + this.margin.top + this.margin.bottom)
            .append("g")
                .attr("transform", "translate(" + this.margin.left + "," + this.margin.top + ")");
    
        // Per-type markers, as they don't inherit styles.
        svg.append("defs").selectAll("marker")
                .data(["suit", "licensing", "resolved"])
            .enter().append("marker")
                .attr("id", function(d) { return d; })
                .attr("viewBox", "0 -5 10 10")
                .attr("refX", 15)
                .attr("refY", -1.5)
                .attr("markerWidth", 6)
                .attr("markerHeight", 6)
                .attr("orient", "auto")
            .append("path")
                .attr("d", "M0,-5L10,0L0,5");
    
        var path = svg.append("g").selectAll("path")
                .data(force.links())
            .enter().append("path")
                .attr("class", function(d) { return "link " + d.type; })
                .attr("marker-end", function(d) { return "url(#" + d.type + ")"; });
    
        var circle = svg.append("g").selectAll("circle")
                .data(force.nodes())
            .enter().append("circle")
                .attr("r", 6)
                .on("click", function (node, index, fixed){
                    //the default behaviour will be to collapse on click, but it can be overriden.
                    if (node.name == "root") node.actualNode = jsonStruct;
                    if (tThis.onClickCallback){
                        tThis.onClickCallback (node, index, fixed);
                        return;
                    }
                    
                    if (d.children) {
                        d._children = d.children;
                        d.children = null;
                    } else {
                        d.children = d._children;
                        d._children = null;
                    }
                    tThis.refresh();
                })
                .call(force.drag);
        
        svg.append("g")
            .attr("class", "labels")
        svg.select(".labels").append("text");
    
        var text = svg.append("g").selectAll("text")
                .data(force.nodes())
            .enter().append("text")
                .attr("x", 8)
                .attr("y", ".31em")
                .text(function(d) { 
                    return d.realName; })
                .on("click", function (node, index, fixed){
                    alert("clicking text");
                    if (tThis.onClickTextCallback)
                        tThis.onClickTextCallback (node, index, fixed);
                });
    
    
    
        // Use elliptical arc path segments to doubly-encode directionality.
        function tick() {
            path.attr("d", linkArc);
            circle.attr("transform", transform);
            text.attr("transform", transform);
        }
    
        function linkArc(d) {
            var dx = d.target.x - d.source.x,
                dy = d.target.y - d.source.y,
                dr = Math.sqrt(dx * dx + dy * dy);
            return "M" + Math.abs(d.source.x) + "," + Math.abs(d.source.y) + "A" + dr + "," + dr + " 0 0,1 " + Math.abs(d.target.x) + "," + Math.abs(d.target.y);
        }
    
        function transform(d) {
            return "translate(" + Math.abs(d.x) + "," + Math.abs(d.y) + ")";
        }
    };
    
    this.iterateJSON = function (node, parent, toAccumulate, realParent, depth){
        for( var key in node){
            var random = parseInt(Math.random()*1000000);
            var keyToUse = key + "" + random;
            if (key.indexOf(this.omitStart) == 0) continue;
            else{
                if (key.indexOf(this.uniqueKeyStart) == 0) 
                    keyToUse = key;
                
                toAccumulate.push ({source: parent, target: keyToUse, type: "licensing", realSource:realParent, realTarget: key, depth: depth, actualNode: node[key]});
            }
            
            if (node[key] instanceof Object) this.iterateJSON (node[key], keyToUse, toAccumulate, key, depth+1);
        }
        
        return toAccumulate;
    };
    
    /*
     * node: current node
     * parent: string name of the parent (fake name generated to maintain uniqueness
     * whereToAdd: place where to put all the children
     * realParent: the real name of the parent
     * depth: current depth of the child (not sure if this is needed)
     */
    this.iterateJSON2 = function (node, parent, whereToAdd, realParent, depth){
        for( var key in node){
            var random = parseInt(Math.random()*1000000);
            var keyToUse = key + "" + random;
            if (key.indexOf(this.omitStart) == 0) continue;
            else{
                if (key.indexOf(this.uniqueKeyStart) == 0) keyToUse = key;
                
                whereToAdd.push({name: key, type: "licensing", children:[]});
                //toAccumulate.push ({source: parent, target: keyToUse, type: "licensing", realSource:realParent, realTarget: key, depth: depth, actualNode: node});
            }
            
            if (node[key] instanceof Object) this.iterateJSON2 (node[key], keyToUse, whereToAdd[whereToAdd.length-1].children, key, depth+1);
        }
        
        return whereToAdd;
    };
    
    this.concatinateNodes = function (nodesWithPos, allNodes){
        var toReturn = [];
        
        for (var an = 0; an < allNodes.length; an++){
            var curAllNode = allNodes[an];
            var pushed = false;
            
            for (var nwp = 0; nwp < nodesWithPos.length; nwp++){
                var curNodeWithPos = nodesWithPos[nwp];
                
                if (curAllNode.actualNode.__path == curNodeWithPos.actualNode.__path){
                    toReturn.push(curNodeWithPos);
                    pushed = true;
                    break;
                }
            }
            
            if (!pushed) toReturn.push(curAllNode);
        }
        return toReturn;
    };
    
    this.concatinateLinks = function (linksWithPos, allLinks){
        var toReturn = [];
        
        for (var an = 0; an < allLinks.length; an++){
            var curLink = allLinks[an];
            var pushed = false
            
            for (var nwp = 0; nwp < linksWithPos.length; nwp++){
                var curLinkWithPos = linksWithPos[nwp];
                
                if (curLink.source.actualNode.__path == curLinkWithPos.source.actualNode.__path && 
                        curLink.target.actualNode.__path == curLinkWithPos.target.actualNode.__path){
                    toReturn.push (curLinkWithPos);
                    pushed = true;
                    break;
                }
            }
            
            if (!pushed) toReturn.push(curLink);
        }
        return toReturn;
    };
    
    return this;
}




function vjD3JS_ForceNetworkIonController ( viewer )
{
    loadCSS("d3js/css/forceNetwork.css");

    vjD3View.call(this,viewer); // inherit default behaviors of the DataViewer
    
    jTHis = this;
    
    this.data = viewer.data;
    viewer.marin ? this.margin = viewer.margin : this.margin = {top:30, right:30, bottom:30, left:30};
    viewer.width ? this.width = viewer.width : this.width = 960;
    viewer.height ? this.height = viewer.height : this.height = 500;
    !viewer.differentDS ? this.differentDS = true : this.differentDS = viewer.differentDS;
    
    viewer.omitStart ? this.omitStart = viewer.omitStart : this.omitStart = "__";
    viewer.uniqueKeyStart ? this.uniqueKeyStart = viewer.uniqueKeyStart : this.uniqueKeyStart = "@";
    
    this.otherOnClickCallback = viewer.onClickCallback;
    
    this.graphClickFunc = function (node, index, fixed){
        jTHis.viewerHC.refresh(node.actualNode);
    };    
    
    this.data = viewer.data;
    if (this.differentDS){
        this.data += "_viewerDS";
        vjDS.add ("", this.data, "static://");        
        
        this.onRefreshIonCallback = function (node){
            //console.log ("helloi");
            vjDS[jTHis.data].reload("static://"+JSON.stringify(this.root), true);
        }
    }
    
    this.graph = new vjD3JS_ForceNetwork ({
        data:this.data, 
        width: this.width, 
        height: this.height, 
        csvTbl: true,
        onClickCallback: this.graphClickFunc,
        onClickTextCallback: this.graphClickFunc
    });

    
    this.viewerHC = new vjIonHCView ({
        formObject : document.forms[viewer.formFolders],
        name : 'hierarchy',
        icon : 'tree',
        data : viewer.data,
        dataFormat : "json",
        doNotShowRefreshIcon: true,
        
        hierarchyColumn : 'path',
        highlightRow : true,
        expandSize : 12,
        folderSize : 24,
        showRoot : 0,
        showLeaf : true,
        onRefreshDoneCallback: this.onRefreshIonCallback,
        isok : true
    });
    
    return [this.viewerHC, this.graph];
}