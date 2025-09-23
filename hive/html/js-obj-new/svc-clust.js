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

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjTreeSeries.js");
javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");


vjHO.register('svc-clust2').Constructor = function() {
    if (this.objCls) return;

    if (!this.defaultDownloadWildcard)
        this.defaultDownloadWildcard = "*.{csv,fa,tre}";

    vjHiveObjectSvcBase.call(this);
    
    this.objCls = "obj-svc-clust"+Math.random();
    vjObj.register(this.objCls, this);

    this.maxViewersSNP = 5;

    this.downloadViewerDescriptions = {
        "result.tre" : "Clusterization tree (Newick format)",
        "distance-matrix.csv" : "Distance matrix (CSV format)",
        "shrunk-genomes.fa" : "Shrunk genomes (FASTA format)",
        "shrunk-snp.csv" : "Shrunk SNP table for classification (CSV format)",
        "shrunk-snp-classification.csv" : "Shrunk SNP table for classification (CSV format)",
        "shrunk-snp-per-position.csv" : "Shrunk SNP table for per-position analysis (CSV format)",
        "SNPcompare-match.csv" : "Matching SNPs (CSV format)",
        "SNPcompare-mismatch.csv" : "Mismatching SNPs (CSV format)"
    };

    this.references = {};    
    
    this_ = this;
   
    this.fullview = function(node, dv) {
        vjDS.add("Retrieving list of compared profiler computations", "profiles",  'http:
        vjDS.add("Retrieving reference genome", "references", "static:
        vjDS.add("Retrieving list of examined references", "ref_examined", "http:
        vjDS.add("", "options", "static:
                "<tr>",
                    "<td>Phylogram shape:</td>",
                    "<td>",
                        "<select id='dvClustPhylogramOptionsShape'>",
                            "<option>circular</option>",
                            "<option selected>rectangular</option>",
                            "<option>unrooted</option>",
                        "</select>",
                    "</td>",
                "</tr>",
                "<tr>",
                    "<td>Branch length scale:</td>",
                    "<td>",
                        "<select id='dvClustPhylogramOptionsScale'>",
                            "<option selected>linear</option>",
                            "<option value='log'>logarithmic</option>",
                            "<option>none</option>",
                        "</select>",
                    "</td>",
                "</tr>",
                "<tr>",
                    "<td>Leaf labels:</td>",
                    "<td>",
                        "<select id='dvClustPhylogramOptionsLeafLabels'>",
                            "<option>ID</option>",
                            "<option>name</option>",
                            "<option selected>ID: name</option>",
                        "</select>",
                    "</td>",
                "</tr>",
            "</table>"].join(""));
        vjDS.add("", "clustHelp", "http:
        
        this.create(node.id);
    };

    this.create = function(id) {
        this.loadedID = id;
        this.formName = "algoForm";
        this.references[id] = {};
        this.nodeInfo = {};

        this.phylogram = new vjSVG_Phylogram({
            nodeLabel: this.makePhylogramNodeLabel(),
            nodeTooltip: function(node) {
                var tip = this.defaultNodeTooltip.apply(this, arguments);
                if (node.leafnode && this.svc_clust_info[node.name]) {
                    tip = tip ? tip+"\n" : "";
                    tip += "ID: " + node.name + "\n" + this.svc_clust_info[node.name].name + "\n" + formaDatetime(this.svc_clust_info[node.name].created);
                } else if (!node.leafnode && !node.expanded) {
                    tip = tip ? tip+"\n" : "";
                    tip += this_.hiddenProfilesLabel(node);
                }
                return tip;
            },
            svc_clust_info: {}
        });
        this.treeSeries = new vjTreeSeries({
            name: "tree",
            title: "hierarchical clustering tree",
            url: "static:
            dataFormat: "newick",
            type: "rectangular"
        }, false);
        this.phylogram.add(this.treeSeries);        
        var treeViewer = new vjSVGView({
            chartArea: {width: "95%", height: "95%"},
            formObject: document.forms[formName],
            downloadLinkManage: true,
            options: {},
            plots: [this.phylogram]
        });
        vjDS.add("Retriving clustering hierarchy", "tree",  "http:
        this.treeSeries.register_callback({
            obj: this,
            func: function(param, content, page_request) {
                function processSequenceIds(node) {
                    if (!node)
                        return;
                    if (node.leafnode) {
                        var controller = this;
                        this.phylogram.registerLeafCheckedCallback(node.name, function(checked) {
                            controller.mimicLeafChecked(node.name, checked, "phylogram");
                        });
                    }
                    for (var i=0; i<node.children.length; i++)
                        processSequenceIds.call(this, node.children[i]);
                }
                processSequenceIds.call(this, this.treeSeries.tree.root);
            }
        }, "series_loaded");
        vjDS["tree"].register_callback(function(viewer, content){
            var tr = new vjTree();
            var newickTr = tr.parseNewick (content);
            this_.maxViewersSNP = this_.treeSize(newickTr, 0);
            this_.constructNeededSNP();
        });        
        
        var treeOptions = new vjHTMLView({
            data: "options"
        });
        
        this.profilesView = new vjTableView({
            data: "profiles",
            formObject: document.forms[formName],
            checkable: true,
            cols: [
               { name: new RegExp(/.*/), hidden: true },
               { name: 'id', hidden: false, title: 'ID', order: 1 },
               { name: 'name', hidden: false, title: 'Name', wrap: false, order: 2 },
               { name: 'created', hidden: false, title: "Created", align: "right", type: "datetime", order: 3 }
            ],
            checkCallback: function(viewer, node, cursel, dummy) {
                this_.mimicLeafChecked(node.id, node.checked, "profiles");
            }
        });
        this.profilesView.precompute = function(param, table, ir) {
            this_.rebgcolorViewerProfilesRow(ir, table.rows[ir].id, table.rows[ir].checked);
        };
        
        var referencesView = new vjTableView({
            data: "references",
            formObject: document.forms[formName],
            bgColors:['#f2f2f2', '#ffffff'],
            checkable: false,
            cols: [
               { name: new RegExp(/.*/), hidden: true },
               { name: "#", hidden: false, title: "ID", order: 1 },
               { name: "id", hidden: false, title: "Name", wrap: false, order: 2 },
               { name: "len", hidden: false, title: "Length", wrap: false, order: 3 }
            ],
            selectCallback: function(viewer, node, cursel, dummy) {
                this_.references[id].selected = node["#"];
                this_.renderSNP();
            },
            precompute: function(param, table, ir) {
                if (ir == 0) {
                    table.rows[ir].selected = true;
                    this_.references[id].selected = table.rows[ir]["#"];
                }
            }
        });
        
        var filesStructureToAdd = [{
            tabId: 'clusteringPhylo',
            tabName: "Clustering Phylogram",
            position: {posId: 'clustInfo', top:'0%', bottom:'100%', left:'20%', right:'50%'},
            inactive: false,
            viewerConstructor: {
                instance: [treeViewer, treeOptions]
            },
              autoOpen: "computed"
        },{
            tabId: 'profilings',
            tabName: "Profilings",
            position: {posId: 'refDetails', top:'0%', bottom:'100%', left:'50%', right:'75%'},
            inactive: false,
            viewerConstructor: {
                instance: this.profilesView
            },
              autoOpen: "computed"
        },{
            tabId: 'references',
            tabName: "References",
            position: {posId: 'refDetails', top:'0%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                instance: referencesView
            },
              autoOpen: "computed"
        }];
        
        treeOptions.callbackRendered = function(view, data) {
            gObject("dvClustPhylogramOptionsShape").onchange = function() {
                this_.treeSeries.type = this.value;
                if (this_.treeSeries.state == "done")
                    this_.treeSeries.refreshWithoutRebuildingTree();
            };
            gObject("dvClustPhylogramOptionsScale").onchange = function() {
                this_.phylogram.branchLengthScale = this.value;
                if (this_.treeSeries.state == "done")
                    this_.treeSeries.refreshWithoutRebuildingTree();
            };
            gObject("dvClustPhylogramOptionsLeafLabels").onchange = function() {
                this_.phylogram.nodeLabel = this_.makePhylogramNodeLabel(this.value);
                if (this_.treeSeries.state == "done")
                    this_.treeSeries.refreshWithoutRebuildingTree();
            };
        };

        vjDS["profiles"].register_callback({
            obj: this,
            func: function(param, content, page_request) {
                var table = new vjTable(content, 0, vjTable_propCSV);
                for (var ir=0; ir<table.rows.length; ir++) {
                    var row = table.rows[ir];
                    this.phylogram.svc_clust_info[row.id] = { name: row.name, created: row.created, ir: ir };
                }
                if (this.treeSeries.state == "done")
                    this.treeSeries.refreshWithoutRebuildingTree();
            }
        });
        vjDS["profiles"].load();
        vjDS["ref_examined"].register_callback({
            obj: this_,
            func: function(param, content, page_request) {
                var recViewer = vjDV[algoProcess.recViewerName];
                var referenceGenomeElt = recViewer.getElement("referenceID");
                this.references[id].genome = referenceGenomeElt.value;
                
                this.references[id].examined = content.replace(/[\r\n]+/, "");
                this.references[id].selected = this.references[id].examined.split(/[,;]/)[0];
                var url = "http:
                vjDS["references"].reload (url, true);
            }
        });
        vjDS["ref_examined"].load();

       
        var filesStr = "{";
        for (var key in this.downloadViewerDescriptions){
            filesStr += key + ",";
        }
        filesStr = filesStr.substr(0, filesStr.length-1)+"}";
        vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };
    
    this.constructNeededSNP = function (){
        this.snpFreqGraphs = [];
        this.coverageGraphs = [];
        for (var i=0; i < this.maxViewersSNP; i++) {
            this.snpFreqGraphs.push( new vjGoogleGraphView({
                hidden: true,
                data: "dsVoid",
                icon: "grap",
                name: "Cumulative SNP frequency chart #" + i,
                formObject: document.forms[formName],
                options: {
                    focusTarget: 'category',
                    isStacked: true,
                    width: 500, height: 200,
                    vAxis: { title: "SNP %" },
                    lineWidth: 1,
                    chartArea: { top: 5, left: 70, height: '80%', width: '100%' },
                    legend: { position: "bottom" },
                    changeHeight: false
                },
                series: [{ name: 'Position', label: false }, { name: "Cumulative SNP frequency" }],
                type: 'column',
                isok: true
            }));

            this.coverageGraphs.push( new vjGoogleGraphView({
                hidden: true,
                data: "dsVoid",
                icon: "grap",
                name: "Coverage chart #" + i,
                formObject: document.forms[formName],
                options: {
                    focusTarget: 'category',
                    isStacked: true,
                    width: 500, height: 120,
                    vAxis: { title: "Coverage" },
                    lineWidth: 1,
                    chartArea: { top: 5, left: 70, height: '80%', width: '100%' },
                    legend: { position: "bottom" },
                    changeHeight: false
                },
                series: [{ name: 'Position', label: false }, { name: "Count Forward" }, { name: "Count Reverse" }],
                type: "area",
                isok: true
            }));
        }
        
        var filesStructureToAdd = [{
            tabId: 'snpFreq',
            tabName: "SNP Frequencies",
            position: {posId: 'refDetails', top:'0%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            multiView: true,
            allowClose: true,
            viewerConstructor: {
                id:"snpFreqGraphs",
                layoutType: "stack",
                orientation: "vertical",
                views: this.snpFreqGraphs
            },
              autoOpen: "computed"
        },{
            tabId: 'coverage',
            tabName: "Coverage",
            position: {posId: 'refDetails', top:'0%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            multiView: true,
            viewerConstructor: {
                id: "coverageGraphs",
                layoutType: "stack",
                orientation: "vertical",
                views: this.coverageGraphs
            },
              autoOpen: "computed"
        }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        
    };
    
    this.treeSize = function (root, curSize){
        if (!root.children || root.children.length < 1)
            return 1;
        
        for (var i = 0; i < root.children.length; i++)
            curSize += this.treeSize(root.children[i], curSize);
        
        return curSize;
    }

    this.hiddenProfilesLabel = function(node) {
        function countLeaves(node, counter) {
            if (node.leafnode)
                counter.count++;
            if (node.childrenCnt > 0 && node.childrenCnt > node.children.length)
                counter.exact = false;
            for (var i=0; i<node.children.length; i++)
                countLeaves(node.children[i], counter);
        }

        var counter = {count: 0, exact: true};
        countLeaves(node, counter);
        return (counter.exact ? "" : ">") + counter.count + " profile" + (counter.count > 1 ? "s" : "") + " hidden";
    };

    this.makePhylogramNodeLabel = function(format) {
        var hiddenProfilesLabel = this.hiddenProfilesLabel;
        return function(node) {
            var lab = this.defaultNodeLabel.apply(this, arguments);
            if (node.leafnode && this.svc_clust_info[node.name]) {
                if (format === "ID") {
                    return node.name;
                } else if (format === "name") {
                    return this.svc_clust_info[node.name].name;
                } else {
                    return node.name + ": " + this.svc_clust_info[node.name].name;
                }
            } else if (!node.leafnode && !node.expanded) {
                return hiddenProfilesLabel(node);
            }
            return lab;
        }
    };

    this.rebgcolorViewerProfilesRow = function(ir, nodeId, checked) {
        if (!this.profilesView.bgColors)
            this.profilesView.bgColors = [];
        if (checked) {
            this.profilesView.bgColors[ir] = this.phylogram.nodeCheckedColor(this.phylogram.nodeFindByName(nodeId));
        } else {
            this.profilesView.bgColors[ir] = "#fff";
        }
    };

    this.mimicLeafChecked = function(nodeId, checked, source, param)
    {
        if (this.inMimicLeafChecked)
            return;

        this.inMimicLeafChecked = true;
        var ir = this.phylogram.svc_clust_info[nodeId].ir;

        if (source !== "phylogram")
            this.phylogram.mimicLeafChecked(nodeId, checked);
        if (source !== "profiles")
            this_.profilesView.mimicCheckmarkSelected(ir, checked);

        this.renderSNP();

        this.rebgcolorViewerProfilesRow(ir, nodeId, checked);
        this_.profilesView.refresh();

        delete this.inMimicLeafChecked;
    };

    this.getCheckedProfiles = function() {
        var profilesMapping = new Array (this.maxViewersSNP);
        for (var i = 0; i < this.profilesView.checkedNodes.length; i++)
            profilesMapping[this.profilesView.checkedNodes[i].irow] = this.profilesView.checkedNodes[i].id;
        
        return profilesMapping;
    };

    this.ensureSNPSource = function(profileID, reference) {
        var dsname = ["ds", this.dvinfo, "snp", profileID, reference].join("_");
        if (vjDS[dsname])
            return vjDS[dsname];

        var title = "Cumulative SNP frequency for profiling result "+profileID+" at ref #"+reference;
        var tqs = [
            {
                op: "load-SNPprofile",
                arg: {
                    obj: profileID,
                    sub: reference,
                    thumb: true,
                    autoAddMissingRows: true
                }
            },
            {
                op: "appendcol",
                arg: {
                    name: "Cumulative SNP frequency",
                    formula: "max(${Frequency A},${Frequency C},${Frequency G},${Frequency T})"
                }
            },
            {
                op: "hidecol",
                arg: {
                    cols: { regex: "^(Position|Cumulative SNP frequency|Count Forward|Count Reverse)", negate: true }
                }
            }
        ];
        var url = "qpbg_tblqryx4:
        return vjDS.add(title, dsname, url);
    };

    this.renderSNP = function() {
        var reference = this.references[this.loadedID].selected;
        if (reference == undefined)
            return;

        var profiles = this.getCheckedProfiles();

        for (var i=0; i < this.maxViewersSNP; i++) {
            var snpViewer = this.snpFreqGraphs[i].instance; 
            var coverageViewer = this.coverageGraphs[i].instance; 
            
            if (!profiles[i]){
                continue;
            }
            
            var profileID = profiles[i];
            
            if (i < profiles.length) {
                var color = this.phylogram.nodeCheckedColor(this.phylogram.nodeFindByName(profileID));
                snpViewer.hidden = false;
                snpViewer.options.colors = [ color ];
                snpViewer.series[1].title = "Cumulative SNP frequency for profiling result "+profileID+" at ref #"+reference;

                coverageViewer.hidden = false;
                coverageViewer.options.colors = [ color, darkenColor(color, 0.5) ];
                coverageViewer.series[1].title = "Count forward for profiling result "+profileID+" at ref #"+reference;
                coverageViewer.series[2].title = "Count reverse for profiling result "+profileID+" at ref #"+reference;

                if (i+1 == profiles.length || i+1 == this.maxViewersSNP) {
                    snpViewer.options.legend.position = "bottom";
                    snpViewer.options.hAxis = {textPosition: "out"};

                    coverageViewer.options.legend.position = "bottom";
                    coverageViewer.options.hAxis = {textPosition: "out"};
                } else {
                    snpViewer.options.legend.position = "bottom";
                    snpViewer.options.hAxis = {textPosition: "none"};

                    coverageViewer.options.legend.position = "bottom";
                    coverageViewer.options.hAxis = {textPosition: "none"};
                }

                snpViewer.unregister_callback();
                coverageViewer.unregister_callback();
                var source = this.ensureSNPSource(profileID, reference);
                snpViewer.data = [source.name];
                snpViewer.register_callback();
                coverageViewer.data = [source.name];
                coverageViewer.register_callback();
                source.load();
            }
        }
    };

    if(this.onObjectContructionCallback)
        this.onObjectContructionCallback(this);
};
