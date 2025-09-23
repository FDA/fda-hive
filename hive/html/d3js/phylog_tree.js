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

function vjD3JS_TreeView ( viewer )
{
    loadCSS("d3js/css/phylog_tree.css");
    vjD3View.call(this,viewer);

    if (!viewer.width) this.width=1400;
    else this.width = viewer.width;
    
    if (!viewer.height) this.height=900;
    else this.height = viewer.height;
    
    if(!viewer.margin) this.margin={top: 50, right: 100, bottom: 36, left: 20};
    else this.margin=viewer.margin;
    
    if (viewer.comparator) this.comparator = viewer.comparator;
    else this.comparator = undefined;
        
    if (viewer.cols) this.cols=verarr(viewer.cols);
    if (viewer.notCols) this.notCols=verarr(viewer.notCols);
    if (viewer.labelCol) this.labelCol=verarr(viewer.labelCol);
    if (viewer.openLevels) this.openLevels=viewer.openLevels;
    
    this.pagedNodeInfo={};
    this.lastNode = undefined;
    
    var max=Number.MIN_VALUE, min=Number.MAX_VALUE;
    var that=this;
    var idGenerator = 0;
    
    
    this.pagedNodeInfo = {};
    
    
    this.parseTaxTreeTbl = function (data){
        if (!data || data.length == 0) return;
        var treeRoot={};
        var nodesArr=[];
        
        var treeRoot = {name:data[0].name, length:0, curLength:0, children:[], taxid:data[0].taxid, totalChildren:data[0].numChild, parentTax: data[0].parent};
        nodesArr[data[0].taxid] = treeRoot;
        
        for (var i = 1; i < data.length; i++){
            var curRow = data[i];
            
            if (isNaN(parseInt(curRow.taxid)) || isNaN(parseInt(curRow.taxid))) continue;
            var node = {name:curRow.name, length:0, curLength:0, children:[], taxid:curRow.taxid, totalChildren: curRow.numChild, parentTax: curRow.parent};
            if (nodesArr[curRow.taxid]){
                nodesArr[curRow.taxid].name = curRow.name;
                nodesArr[curRow.taxid].totalChildren = curRow.numChild;
            }
            else
                nodesArr[curRow.taxid] = node;
            if (!nodesArr[curRow.parent])
                nodesArr[curRow.parent] = {children:[], taxid:curRow.parent, length: 0, curLength: 0};
            
            var found = false;
            for (var j = 0; j < nodesArr[curRow.parent].children.length; j++){
                if (nodesArr[curRow.parent].children[j].taxid == node.taxid){
                    found = true;
                    break;
                }
            }
            if (!found)
                nodesArr[curRow.parent].children.push(nodesArr[curRow.taxid]);
            
            if (this.colorOnSelect && this.lastNode && nodesArr[curRow.parent][this.comparator] == this.lastNode[this.comparator]){
                nodesArr[curRow.parent].color = this.lastNode.color;
            }
        }
        this.filterTree(treeRoot);
        
        return treeRoot;
    };
    
    this.filterTree = function (tree){
        if (!tree || tree.children.length < 1) return;
        
        var toSplice = [];
        for (var i = 0; i < tree.children.length; i++){
            if (this.pagedNodeInfo[tree.taxid] && (i < this.pagedNodeInfo[tree.taxid].start || i > this.pagedNodeInfo[tree.taxid].end)){ 
                toSplice.push(i);
            }
            else
                this.filterTree(tree.children[i]);
        }
        
        for (var i = toSplice.length-1; i >= 0; i--){
            tree.children.splice(toSplice[i],1);
        }
    };
    
    this.parseNewickToJson = function(data) {        
        var tmpTree = new vjTree();
        var fromTreeView = tmpTree.parseNewick (data);
        var tree = {};
        
        this.collectTree(fromTreeView, tree);
        
        this.setAppropriateLength(tree);
        return tree;
    };
    
    this.collectTree = function (curTree, finalTree){
        if (!curTree.children)
            return;
        
        finalTree.children = [];
        for (var i = 0; i < curTree.children.length; i++){
            finalTree.children.push (new Object());
            this.collectTree (curTree.children[i], finalTree.children[i]);
        }
        if (!curTree.distance)
            finalTree.length = 0;
        else
            finalTree.length = curTree.distance;
        if (curTree.name == 0) curTree.name = "";
        finalTree.name = curTree.name;        
    };
    
    this.setAppropriateLength = function (tree)
    {
        if (!tree.length)
        {
            tree.length = 0;
            tree.curLength = 0;
        }
        
        tree.curLength += Math.abs(tree.length);
        
        if (!tree.children || tree.children.length == 0)
            return;
                
        for (var i = 0; i < tree.children.length; i++)
        {
            var child = tree.children[i];
            
            child.curLength = tree.curLength;
            this.setAppropriateLength (child);
        }
    };
    
    this.d3Compose=function(data)
    {
        if (data == "") return;
        
        var jsonTree;
        if (this.taxTree) jsonTree = this.parseTaxTreeTbl(data);
        else jsonTree = this.parseNewickToJson(data);
        
        if (!jsonTree) return;
        
        if (this.height <= 150) this.height = 150;
        if (this.width <= 150) this.width = 150;
        
        this.margin = {top: 20, right: 20, bottom: 20, left: 100},
        width = this.width - this.margin.right - this.margin.left,
        height = this.height - this.margin.top - this.margin.bottom-20;
        
        var i = 0;
        this.duration = 750;
    
        this.tree = d3.layout.tree()
            .size([height, width]);
    
        this.diagonal = d3.svg.diagonal()
            .projection(function(d) { return [d.y, d.x]; });
    
        this.svg = this.d3svg
            .attr("width", width + this.margin.right + this.margin.left)
            .attr("height", height + this.margin.top + this.margin.bottom)
            .append("g")
                .attr("transform", "translate(" + this.margin.left + "," + this.margin.top + ")");
    
       
        this.root = jsonTree;
        this.root.x0 = height / 2;
        this.root.y0 = 0;
        
        function collapse(d) {
            if (d.children) {
                d._children = d.children;
                d._children.forEach(collapse);
                d.children = null;
            }
        }
        
        function findLevelsToCollapse(node, level)
        {
            if (!node.children)
                return 0;
            
            if (level == that.openLevels-1)
                node.children.forEach(collapse);
            
            for (var i = 0; i < node.children.length; i++)
                findLevelsToCollapse(node.children[i], level+1);
        }
        
        findLevelsToCollapse(this.root, 1);
        this.update(this.root);
    };
    
    this.orderOfNodes = function (node)
    {
        if (!node) return [];
        
        if (!node.children)
            return [node];
        
        var top = this.orderOfNodes(node.children[0]);
        
        if (node.children.length == 1){
            top.push(node);
            return top;
        }
        
        for (var i = 1; i < node.children.length; i++){
            if (i == Math.round(node.children.length/2))
                top.push(node);
            var tmp = this.orderOfNodes(node.children[i]);
            top = top.concat(tmp);
        }
        
        return top;
    }
    
    this.correctXTree = function (curRoot, height)
    {
        var totalOpen = this.orderOfNodes(curRoot);
        
        for (var i = 0; i < totalOpen.length; i++)
        {
            var node = totalOpen[i];
            node.x = height/(totalOpen.length-1)*i;
        }
    }
    
    this.update = function (source) 
    {
        var nodes = this.tree.nodes(this.root).reverse(),
              links = this.tree.links(nodes);
        
        if (nodes.length*10 > this.height){
            this.height = nodes.length*10;
            
            this.margin = {top: 20, right: 120, bottom: 20, left: 120},
            width = this.width - this.margin.right - this.margin.left,
            height = this.height - this.margin.top - this.margin.bottom-20;
        
            this.tree = d3.layout.tree()
                .size([this.height, this.width]);
        
            this.diagonal = d3.svg.diagonal()
                .projection(function(d) { return [d.y, d.x]; });

            this.d3svg.attr("height", height + this.margin.top + this.margin.bottom);
            
            this.root.x0 = this.height / 2;
            this.root.y0 = 0;
            this.update(this.root);
            return;
        }
        var maxDepth = -1;
        nodes.forEach(function(d) { 
            if (!that.useDepthForTree && (d.curLength && d.curLength >= 0 && d.curLength+d.depth > maxDepth))
                maxDepth = d.curLength + d.depth; 
            else if (d.depth > maxDepth)
                maxDepth = d.depth;
        });
        
        var lengthToUse;
        if (maxDepth > 100)
            lengthToUse = (width-100) / Math.log10(maxDepth);
        else
            lengthToUse = (width-100) / maxDepth;
        if (lengthToUse < 5) lengthToUse = 5;

        nodes.forEach(function(d) { 
            if (!that.useDepthForTree && (d.curLength && d.curLength >= 0)){
                if (maxDepth > 100)
                    d.y = (lengthToUse * Math.log10((d.curLength) + d.depth)); 
                else
                    d.y = (lengthToUse * ((d.curLength) + d.depth)); 
            }
            else
                d.y = d.depth * (lengthToUse);
        });
        
        this.correctXTree(this.root, this.height-60);

        var node = this.svg.selectAll("g.node")
             .data(nodes, function(d, i) { 
                 return d.id || (d.id = ++idGenerator); });

        var nodeEnter = node.enter().append("g")
              .attr("class", "node")
              .attr("transform", function(d) { 
                  return "translate(" + source.y0 + "," + source.x0 + ")"; })
              .attr("id", function(d){
                  if (that.idElem) return d[that.idElem];
              });

        nodeEnter.append("circle")
            .style("fill", function(d) { 
                  if (d.color == "red" && d[that.comparator] == that.lastNode[that.comparator]) 
                      return "red";
                return (d._children && d._children.length > 0) ? "lightsteelblue" : "#fff"; })
            .on("click", this.click);

        nodeEnter.append("text")
              .attr("x", function(d) { 
                  return d.children || d._children ? -10 : 10; })
              .attr("dy", ".35em")
              .attr("text-anchor", function(d) { 
                  return d.children || d._children ? "end" : "start"; })
              .text(function(d) { 
                  return d.name; })
              .style("fill-opacity", 1e-6)
              .on("click", this.clickText);

         var nodeUpdate = node.transition()
             .duration(this.duration)
              .attr("transform", function(d) { 
                  return "translate(" + d.y + "," + d.x + ")"; });

         nodeUpdate.select("circle")
              .attr("r", 4.5)
              .style("fill", function(d) { 
                  if (d.color == "red" && d[that.comparator] == that.lastNode[that.comparator]) 
                      return "red";
                  return (d._children && d._children.length > 0) ? "lightsteelblue" : "#fff"; });

         nodeUpdate.select("text")
              .style("fill-opacity", 1);

         var nodeExit = node.exit().transition()
              .duration(this.duration)
              .attr("transform", function(d) { 
                  return "translate(" + source.y + "," + source.x + ")"; })
              .remove();

         nodeExit.select("circle")
              .attr("r", 1e-6);

         nodeExit.select("text")
              .style("fill-opacity", 1e-6);

         var link = this.svg.selectAll("path.link")
              .data(links, function(d) { 
                  return d.target.id; });

         link.enter().insert("path", "g")
              .attr("class", "link")
              .attr("d", function(d) {
                  var o = {x: source.x0, y: source.y0};
                  if(d.target.x > that.height)
                      that.d3svg.attr("height", d.target.x + that.margin.top + that.margin.bottom+200);
                  if(d.target.y > (that.width - that.margin.right - that.margin.left - 70)){
                      that.width = d.target.y + that.margin.left + that.margin.right+300;
                      that.d3svg.attr("width", that.width);
                  }
                  return that.diagonal({source: o, target: o});
              });

         link.transition()
              .duration(this.duration)
              .attr("d", this.diagonal);

         link.exit().transition()
              .duration(this.duration)
              .attr("d", function(d) {
                  var o = {x: source.x, y: source.y};
                  return that.diagonal({source: o, target: o});
              })
              .remove();

         nodes.forEach(function(d) {
             d.x0 = d.x;
             d.y0 = d.y;
         });
         
         this.nodes = nodes;
         this.links = links;
         
         if (this.onFinishUpdate) 
             this.onFinishUpdate(this);
    };
    
    this.click = function(d) {
        if (that.lastNode){
            that.lastNode.color = "";
        }
        if (that.colorOnSelect){
            that.lastNode = d;
            d.color = "red";
        }
        
        if (d.children) {
            d._children = d.children;
            d.children = null;
            if (d.children || d._children || d.color)
                that.update(d);
            return;
        } else {
            d.children = d._children;
            d._children = null;
        }
        
        if (that.onClickCallback)
            that.onClickCallback(d);
        if (d.children || d._children || d.color)
            that.update(d);
        if (that.onClickPostUpdateCallback)
            that.onClickPostUpdateCallback(d);
    }
    
    this.clickText = function (d){
        if (that.lastNode){
            that.lastNode.color = "";
        }
        if (that.colorOnSelect){
            that.lastNode = d;
            d.color = "red";
        }
        
        if (that.onClickTextCallback)
            that.onClickTextCallback(d);
        if (d.color)
            that.update(d);
        if (that.onClickTextPostUpdateCallback)
            that.onClickTextPostUpdateCallback(d);
    }
};


function vjD3TaxonomyControl(viewer){
    var originalurl = "http:
    vjDS.add("","dsTaxTree","http:
    vjDS.add("", "dsRecord", "http:
    vjDS.add("","dsTreeViewerSpec","static:
                                +"taxonomy,name_list,Names,list,,,0,1,0,0,0,0,0,,,,,\n"
                                +"taxonomy,taxName,Tax-name,string,name_list,,0,1,1,1,0,0,0,,,,,\n"
                                +"taxonomy,taxid,Taxonomy ID,integer,,,0,1,0,0,0,0,0,,,,,\n"
                                +"taxonomy,parentid,Parent,integer,,,0,1,0,0,0,0,0,,,,,\n"
                                +"taxonomy,path,Ancestry,string,,,0,1,0,0,0,0,0,,,,,\n"
                                +"taxonomy,rank,Rank,string,,,0,1,0,0,0,0,0,,,,,\n"
                                +"taxonomy,bioprojectID,BioProjectID,integer,,,0,1,0,0,0,0,0,,,,,\n");
    vjDS.add("", "dsTreeChildren", "static:
    vjDS.dsTaxTree.register_callback(function (viewer, content){
        vjDS.dsTreeChildren.reload("static:
    });
    vjDS.add("", "dsSearch", "static:
    vjDS["dsSearch"].register_callback(onSearchResults);

    if (viewer.taxIds){
        var tt = "";
        this.taxIds = verarr(viewer.taxIds);
        tt += this.taxIds[0];
        for (var i = 1; i < this.taxIds.length; i++){
            tt += "," + this.taxIds[i];
        }
        vjDS.dsTaxTree.reload("http:
        origianalurl = "http:
    }
    else
        vjDS.dsTaxTree.reload("http:
    
    var graphViewer=new vjD3JS_TreeView({
        data: 'dsTreeChildren',
        height: 800,
        taxTree:true,
        openLevels:-1,
        downloadFileName: "blah",
        idElem: "taxid",
        comparator: "taxid",
        onClickCallback: onClickCircle,
        onClickTextPostUpdateCallback: onLoadChildren,
        onClickTextCallback: updateNodeInfo,
        colorOnSelect: true,
        onFinishUpdate: onFinishCallback
    }); 
    
    var panel = new vjPanelView({
        data: ["dsVoid"],
         formObject: document.forms[viewer.formName],
         prevSearch:"",
         onKeyPressCallback: smartSearchFunc,
         iconSize: 24
    });
    
    var recViewer = new vjRecordView({
        data: ["dsTreeViewerSpec", "dsRecord"],
        name:'details',
        formObject: document.forms[viewer.formName],
        showRoot: false,
        hideViewerToggle:true,
        autoStatus: 3,
        autoDescription: false,
        implementSaveButton: false,
        implementTreeButton: false,
        showReadonlyInNonReadonlyMode: true,
        constructAllNonOptionField: true,
        implementSetStatusButton: false
    });
    
    lastNodeController = 0;    
    var nodeToUrl={};
    
    function onClickCircle (dElement){
        if (!dElement.children || (dElement.children.length <= 0 && dElement.totalChidren != 0))
                updateNodeInfo(dElement);
        lastNodeController = dElement;
    }
    
    function updateNodeInfo (dElement){
        lastNodeController = dElement;
        var val = 10;
        if (nodeToUrl[lastNodeController.taxid]) val = nodeToUrl[lastNodeController.taxid].cnt;
        
        var url = vjDS["dsRecord"].url;
        url = urlExchangeParameter(url, "taxid", dElement.taxid);
        vjDS["dsRecord"].reload(url, true);
        
        if (dElement.totalChildren && parseInt(dElement.totalChildren) < 1){
            panel.rows = [
                { name : 'refresh', icon: "refresh", order: 0, align:"left", description: "Refresh to original", url: onRefresh}
            ];
            panel.rebuildTree();
            panel.redrawMenuView();
        }
        else{
            var options = [0, 10,20,50,100,1000];
            var actualOptions = [];
            
            for (var i = 1; i < options.length; i++){
                if (options[i-1] <= dElement.totalChildren && dElement.totalChildren <= options[i]){
                    actualOptions.push([dElement.totalChildren, dElement.totalChildren]);
                    break;
                }
                actualOptions.push([options[i], options[i]]);
            }
            
            panel.rows = [
                { name : 'refresh', icon: "refresh", order: 0, align:"left", description: "Refresh to original", url: onRefresh},
                { name : 'prev', icon: "previous", order: 1, align:"right", url: onPage},
                { name : 'ppager', align:'right', order:1, title:'at a time', description: 'Select Number of Children to Load' , type:'select', options: actualOptions, value:val, onChangeCallback: onPage, url: onPage},
                { name : 'next', icon: "next", order: 2, align:"right", url: onPage},
                { name : 'serachType', type : 'select', options:[[0, "Attach to Root"], [1, "Generate New Root"]], order:3, align:"right", description : 'Select way to search for node', title:"Search Option", onChangeCallback: onChngSrch},
                { name : 'search', type : 'text', rgxpOff: true, align : 'right', size: 32, order : 4, description : 'Search', url: onSearch, path: "/search" }
            ];
            panel.rebuildTree();
            panel.refresh();
        }
        
        
    };
    function onLoadChildren (viewer, node, a,b,c){
         if(!lastNodeController || lastNodeController.totalChildren == 0) return;
        
        var dsName = "dsTreeChildren" + Math.round(Math.random()*1000);
        var tmpUrl;
        
        if (nodeToUrl[lastNodeController.taxid])
            tmpUrl = nodeToUrl[lastNodeController.taxid].url;
        else{
            tmpUrl = "http:
            nodeToUrl[lastNodeController.taxid] = {url: tmpUrl, cnt:10};
        }
        
        if (!lastNodeController.parent){
            var dsName1 = "dsTreeChildren" + Math.round(Math.random()*1000);
            vjDS.add("", dsName1, "http:
            vjDS[dsName1].register_callback(onTreeParentLoaded);
            
            vjDS[dsName1].load();
        }

        vjDS.add("", dsName, tmpUrl);
        vjDS[dsName].register_callback(onTreeChildrenLoaded);
        
        vjDS[dsName].load();
    };
    
    function onTreeChildrenLoaded(viewer, content){
        vjDS["dsTreeChildren"].reload(vjDS["dsTreeChildren"].url + content, true);
    };
    
    function onTreeParentLoaded(viewer, content){
        var tmp = vjDS["dsTreeChildren"].url.substring(9);
        vjDS["dsTreeChildren"].reload("static:
    };
    
    function appendAllChildren (appendTo, node){
        if (!node) return;
        if (!appendTo.children) appendTo.children = [];
        for (var i = 0; i < node.children.length; i++){
            appendTo.children.push(node.children[i]);
            appendAllChildren(node.children[i]);
        }
    };
    
    function onRefresh (viewer, node){
        vjDS["dsTaxTree"].reload("http:
    };
    
    function onPage (viewer, node){
        console.log(node);
        
        if (node.name == "prev"){
            var cnt = viewer.tree.root.children[3].value;
            var actCnt, actStart;
            
            if (!graphViewer.pagedNodeInfo[lastNodeController.taxid]) return;
            
            if (graphViewer.pagedNodeInfo[lastNodeController.taxid].start - cnt < 0){
                actCnt = graphViewer.pagedNodeInfo[lastNodeController.taxid].start;
                actStart = 0;
            }
            else{
                actCnt = cnt;
                actStart = graphViewer.pagedNodeInfo[lastNodeController.taxid].start - cnt;
            }
            graphViewer.pagedNodeInfo[lastNodeController.taxid].end = graphViewer.pagedNodeInfo[lastNodeController.taxid].start;
            graphViewer.pagedNodeInfo[lastNodeController.taxid].start = actStart;
            graphViewer.pagedNodeInfo[lastNodeController.taxid].cnt = actCnt;

            vjDS["dsTreeChildren"].reload(vjDS["dsTreeChildren"].url, true);
        }else if (node.name == "ppager"){
            var info = nodeToUrl[lastNodeController.taxid];
            var nUrl;
            
            if (!graphViewer.pagedNodeInfo[lastNode.taxid]){
                if (info){
                    if (info.cnt == node.value) return;
                    
                    nUrl = info.url;
                    nUrl = urlExchangeParameter(nUrl, "cnt", node.value);
                    info.url = nUrl;
                    info.cnt = node.value;
                }
                else{
                    nUrl = "http:
                    nodeToUrl[lastNodeController.taxid] = {url: nUrl, cnt: node.value};
                }
            }
            else{
                var nodeInfo = graphViewer.pagedNodeInfo[lastNodeController.taxid];
                nodeInfo.cnt = node.value;
                
                if (nodeInfo.start + nodeInfo.cnt > lastNodeController.totalChildren)
                    nodeInfo.end = lastNodeController.totalChildren;
                else
                    nodeInfo.end = nodeInfo.start + nodeInfo.cnt;
                
                nUrl =  "http:
            }
            
            var dsName = "dsTreeChildren" + Math.round(Math.random()*1000);
            vjDS.add("", dsName, nUrl);
            vjDS[dsName].register_callback(onTreeChildrenLoaded);
            
            vjDS[dsName].load(); 
        }else if (node.name == "next"){
            var cnt = viewer.tree.root.children[3].value;
            var actEnd, actCnt, actStart;
            
            if (cnt >= lastNodeController.totalChildren) return;
            
            if (graphViewer.pagedNodeInfo[lastNodeController.taxid]){
                var node = graphViewer.pagedNodeInfo[lastNodeController.taxid];
                
                if (node.end == lastNodeController.totalChildren) return;
                
                actCnt = cnt;
                actStart = node.start + node.cnt;
                
                if (actStart + node.cnt > lastNodeController.totalChildren-1){
                    actEnd = lastNodeController.totalChildren;
                }
                else
                    actEnd = actStart + node.cnt;
            }else{
                actCnt = cnt;
                graphViewer.pagedNodeInfo[lastNodeController.taxid] = {};
                
                if (2*cnt > lastNodeController.totalChildren-1){
                    actEnd = lastNodeController.totalChildren;
                    actStart = cnt;
                    actCnt = actEnd - actStart;
                }
                else{
                    actEnd = 2*cnt;
                    actStart = cnt;
                }
            }
            graphViewer.pagedNodeInfo[lastNodeController.taxid].start = actStart;
            graphViewer.pagedNodeInfo[lastNodeController.taxid].cnt = actCnt;
            graphViewer.pagedNodeInfo[lastNodeController.taxid].end = actEnd;
            
            if (graphViewer.pagedNodeInfo[lastNodeController.taxid].totalLoaded >= actEnd)
                return;
            
            var dsName = "dsTreeChildren" + Math.round(Math.random()*1000);
            vjDS.add("", dsName, "http:
            vjDS[dsName].register_callback(onTreeChildrenLoaded);
            
            vjDS[dsName].load();
        }        
    };
    
    var searchOption = 0;
    function onChngSrch (viewer, node){
        searchOption = parseInt(node.value);
    }
    
    function onSearch (viewer, node, irow){
        viewer.tree.root.children[6].value = node.taxID;
        viewer.rebuildTree();
        
        if (isNaN(parseInt(viewer.tree.root.children[6].value)) && searchOption == 2){
             if(!lastNodeController) return;
            
            var dsName = "dsTreeChildren" + Math.round(Math.random()*1000);
            var tmpUrl = "http:
            vjDS.add("", dsName, tmpUrl);
            vjDS[dsName].register_callback(onTreeChildrenLoaded);
            
            vjDS[dsName].load();
            panel.rebuildTree();
            panel.redrawMenuView();
            
            return;
        }
        
        if (searchOption == 0){
             if(!lastNodeController) return;
            
            var dsName = "dsTreeChildren" + Math.round(Math.random()*1000);
            var tmpUrl = "http:
            vjDS.add("", dsName, tmpUrl);
            vjDS[dsName].register_callback(onTreeChildrenLoaded);
            
            vjDS[dsName].load();
        }
        else if (searchOption == 1){
            vjDS["dsTaxTree"].reload("http:
        }
        
        panel.rows.splice(1);
        panel.rebuildTree();
        panel.redrawMenuView();
    };
    
    function onFinishCallback (viewer){
        if (lastNodeController) 
            $("#"+lastNodeController.taxid).click();
    };
    
    function smartSearchFunc (container, e, nodepath, elname){
        var node = this.tree.findByPath(nodepath);
        var el = this.findElement(node.name, container);
        
        if (el.value && el.value.length >= 3){
            var nUrl = "http:
            searchVal = el.value + String.fromCharCode(e.which);
            
            vjDS["dsSearch"].reload(nUrl, true);
        }
    };
        
    var searchVal;
    function onSearchResults (viewer, content, requestInfo){
        var table = new vjTable (content);
        
        
        panel.rows.splice(7);
        panel.rows[5].value = searchOption;
        panel.rows[6].value = searchVal;
        
        for(var i = 1; i < table.rows.length; i++){
            panel.rows.push({name:table.rows[i].cols[1].replace(/[_\W]+/g, "-"), order:i ,title: table.rows[i].cols[1], isSubmitable: true, path: "/search/"+table.rows[i].cols[1].replace(/[_\W]+/g, "-"), taxID: table.rows[i].cols[0], url:onSearch});
        }
        
        panel.rebuildTree();
        panel.redrawMenuView();
    };    
    
    return [graphViewer, panel, recViewer];    
};




            

