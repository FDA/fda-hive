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


vjHO.register('svc-clust').Constructor = function() {
    if (this.objCls) return;

    if (!this.defaultDownloadWildcard)
        this.defaultDownloadWildcard = "*.{csv,fa,tre}";

    vjHiveObjectSvcBase.call(this);

    this.fullview = function(node, dv) {
        if (this.isRendered) return;
        this.create(dv, node.id);
        this.isRendered=true;
    };

    this.customizeSvcDownloads = function(obj) {
        this.parent["svc"].customizeDownloads(this.downloadViewerDescriptions, this.defaultDownloadWildcard);
    };

    this.preview = function(node, dv) {
        this.parent.preview("svc", node, dv, "vjObj['"+this.objCls+"'].customizeSvcDownloads();");
        if(!node.status || parseInt(node.status)<5) return;

        this.create(dv, node.id);
        this.isRendered=true;
    };

    this.inherited_resetViewers = this.resetViewers;
    this.resetViewers = function() {
        this.inherited_resetViewers();

        delete this.loadedID;
        this.isRendered = false;
        this.loaded = false;
        this.constructed = false;

        for (var i=0; i<this.dvs.length; i++)
            delete this.dvs[i];
        this.dvs = [];

        delete this.nodeInfo;
        delete this.treeSeries;
        this.phylogram.unlink();
        delete this.phylogram;
    };

    this.create = function(dv, id) {
        this.load(dv, id);
        this.construct(dv, id);
    };

    this.load = function(dv, id) {
        this.loadedID = id;
        this.loaded = true;
        this.constructed = false;
        this.viewers.length = 0;

        this.formName = "";
        var dv_obj = dv.obj.length ? dv.obj[0] : dv.obj;
        var formObject = gAncestorByTag(gObject(dv_obj.name), "form");
        if (formObject)
            this.formName = formObject.name;

        this.references[id] = {};
        if (dv.reference_genome)
            this.references[id].genome = dv.reference_genome;

        this.nodeInfo = {};
        this.makePhylogram();
        this.addviewer("phylogram", new vjSVGView({
            chartArea: {width: "95%", height: "95%"},
            formObject: formObject,
            plots: [this.phylogram]
        }));
        if (this.mode == "preview") {
            this.viewers["phylogram"].geometry = {width: 300, height: 300};
        }
        this.addviewer("options", new vjHTMLView({ data: "options" }));

        var this_ = this;
        this.viewers["options"].callbackRendered = function(view) {
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

        this.makeDS("profiles").register_callback({
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
        this.makeDS("profiles").load();

        if (this.mode == "preview")
            return;


        this.addDownloadViewer(this.downloadViewerDescriptions);

        this.addviewer("profiles", new vjTableView({
            data: "profiles",
            formObject: formObject,
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
        }));
        this.viewers["profiles"].precompute = function(param, table, ir) {
            this_.rebgcolorViewerProfilesRow(ir, table.rows[ir].id, table.rows[ir].checked);
        };

        this.addviewer("references", new vjTableView({
            data: "references",
            formObject: formObject,
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
        }));

        this.makeDS("ref_examined").register_callback({
            obj: this_,
            func: function(param, content, page_request) {
                this.references[id].examined = content.replace(/[\r\n]+/, "");
                this.references[id].selected = this.references[id].examined.split(/[,;]/)[0];
                this.makeDS("references", true);
            }
        });
        this.makeDS("ref_examined").load();

        this.addviewer("help", new vjHelpView({
            data: "help"
        }));

        for (var i=0; i < this.maxViewersSNP; i++) {
            this.addviewer(this.snpViewerName(i), new vjGoogleGraphView({
                hidden: true,
                data: "dsVoid",
                icon: "grap",
                name: "Cumulative SNP frequency chart #" + i,
                formObject: formObject,
                options: {
                    focusTarget: 'category',
                    isStacked: true,
                    width: 500, height: 200,
                    vAxis: { title: "SNP %" },
                    lineWidth: 1,
                    chartArea: { top: 5, left: 70, height: '80%', width: '100%' },
                    legend: { position: "bottom" },
                    isok: true
                },
                series: [{ name: 'Position', label: false }, { name: "Cumulative SNP frequency" }],
                type: 'column',
                isok: true
            }));

            this.addviewer(this.coverageViewerName(i), new vjGoogleGraphView({
                hidden: true,
                data: "dsVoid",
                icon: "grap",
                name: "Coverage chart #" + i,
                formObject: formObject,
                options: {
                    focusTarget: 'category',
                    isStacked: true,
                    width: 500, height: 120,
                    vAxis: { title: "Coverage" },
                    lineWidth: 1,
                    chartArea: { top: 5, left: 70, height: '80%', width: '100%' },
                    legend: { position: "bottom" },
                    isok: true
                },
                series: [{ name: 'Position', label: false }, { name: "Count Forward" }, { name: "Count Reverse" }],
                type: "area",
                isok: true
            }));
        }
    };

    this.construct = function(dv, id) {
        if (!this.loaded) return;
        this.constructed = true;
        this.viewersToAdd = [];

        if (this.mode == "preview") {
            this.dvs[0] = dv.obj;
            var origNumTabs = this.dvs[0].tabs.length;
            var tab = this.dvs[0].addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["phylogram"], this.viewers["options"]]);
            this.dvs[0].render();
            this.dvs[0].load("rerender");
        } else {
            this.dvs[0] = dv.obj[0];
            this.dvs[0].addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["phylogram"], this.viewers["options"]]);
            this.dvs[0].addTab(this.tabs.names[1], this.tabs.icons[1], [this.viewers["download"]]);
            this.dvs[0].render();
            this.dvs[0].load("rerender");

            this.dvs[1] = dv.obj[1];
            var snpViewers = [], coverageViewers = [];
            for (var i=0; i<this.maxViewersSNP; i++) {
                snpViewers.push(this.viewers[this.snpViewerName(i)]);
                coverageViewers.push(this.viewers[this.coverageViewerName(i)]);
            }
            this.dvs[1].addTab(this.tabs.names[2], this.tabs.icons[2], snpViewers);
            this.dvs[1].addTab(this.tabs.names[3], this.tabs.icons[3], coverageViewers);
            this.dvs[1].addTab(this.tabs.names[4], this.tabs.icons[4], [this.viewers["profiles"]]);
            this.dvs[1].addTab(this.tabs.names[5], this.tabs.icons[5], [this.viewers["references"]]);
            this.dvs[1].addTab(this.tabs.names[6], this.tabs.icons[6], [this.viewers["help"]]);
            this.dvs[1].render();
            this.dvs[1].load("rerender");
        }
    };

    this.makeTreeSeries = function() {
        if (this.treeSeries)
            return this.treeSeries;

        this.treeSeries = new vjTreeSeries({
            name: "tree",
            title: "hierarchical clustering tree",
            url: "static:
            dataFormat: "newick",
            type: "rectangular"
        }, false);

        if (this.mode != "preview") {
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
            },
            "series_loaded");
        }

        return this.treeSeries;
    };

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

    this.makePhylogram = function() {
        if (this.phylogram)
            return this.phylogram;

        var this_ = this;

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
        this.makeTreeSeries();
        this.phylogram.add(this.treeSeries);
        return this.phylogram;
    };

    this.rebgcolorViewerProfilesRow = function(ir, nodeId, checked)
    {
        if (!this.viewers["profiles"].bgColors)
            this.viewers["profiles"].bgColors = [];
        if (checked) {
            this.viewers["profiles"].bgColors[ir] = this.phylogram.nodeCheckedColor(this.phylogram.nodeFindByName(nodeId));
        } else {
            this.viewers["profiles"].bgColors[ir] = "#fff";
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
            this.viewers["profiles"].mimicCheckmarkSelected(ir, checked);

        this.renderSNP();

        this.rebgcolorViewerProfilesRow(ir, nodeId, checked);
        this.viewers["profiles"].refresh();

        delete this.inMimicLeafChecked;
    };

    this.getCheckedProfiles = function() {
        function op(list, node) {list.push(node.name);}
        var checkedProfiles = [];
        this.phylogram.enumerate(op, checkedProfiles, true, 1);
        return checkedProfiles;
    };

    this.snpViewerName = function(i) {
        return "snp_" + i;
    };

    this.coverageViewerName = function(i) {
        return "coverage_" + i;
    };

    this.ensureSNPSource = function(profileID, reference) {
        var dsname = ["ds", this.dvinfo, this.typeName, "snp", profileID, reference].join("_");
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
            var profileID = profiles[i];
            var snpViewer = this.viewers[this.snpViewerName(i)];
            var coverageViewer = this.viewers[this.coverageViewerName(i)];
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
                    snpViewer.options.height = 200;
                    snpViewer.options.chartArea.height = "80%";

                    coverageViewer.options.legend.position = "bottom";
                    coverageViewer.options.hAxis = {textPosition: "out"};
                    coverageViewer.options.height = 150;
                    coverageViewer.options.chartArea.height = "80%";
                } else {
                    snpViewer.options.legend.position = "bottom";
                    snpViewer.options.hAxis = {textPosition: "none"};
                    snpViewer.options.height = 170;
                    snpViewer.options.chartArea.height = 160;

                    coverageViewer.options.legend.position = "bottom";
                    coverageViewer.options.hAxis = {textPosition: "none"};
                    coverageViewer.options.height = 130;
                    coverageViewer.options.chartArea.height = 120;
                }

                snpViewer.unregister_callback();
                coverageViewer.unregister_callback();
                var source = this.ensureSNPSource(profileID, reference);
                snpViewer.data = [source.name];
                snpViewer.register_callback();
                coverageViewer.data = [source.name];
                coverageViewer.register_callback();
                source.load();
            } else {
                snpViewer.hidden = true;
            }
            snpViewer.render();
        }
    };

    this.typeName = "svc-clust";
    this.objCls = "obj-svc-clust"+Math.random();
    vjObj.register(this.objCls, this);

    this.viewersToAdd = [];
    this.viewers = {length:0};
    this.dsTitle = "hierarchical clustering data";
    this.tabs = {
        icons: ["dna", "table", "graph", "graph", "table", "table", "help"],
        names: ["Clustering phylogram", "Downloads", "SNP frequencies", "Coverage", "Select profilings", "Select reference", "help"]
    };
    this.addUrlSet({
        "tree": {
            title: "Retrieving clustering hierarchy",
            active_url: "http:
            objs: "ids",
            isSeries: true
        },
        "profiles": {
            title: "Retrieving list of compared profiler computations",
            make_active_url: function(url, id) {
                return 'http:
            }
        },
        "references": {
            title: "Retrieving reference genome",
            make_active_url: function(url, id) {
                if (!this.references[id] && this.references[id].genome && this.references[id].examined)
                    return "static:
                return "http:
            }
        },
        "ref_examined": {
            title: "Retrieving list of examined references",
            active_url: "http:
            objs: "ids"
        },
        "options": {
            active_url: "static:
                "<tr>",
                    "<td>Phylogram shape:</td>",
                    "<td>",
                        "<select id='dvClustPhylogramOptionsShape'>",
                            "<option selected>circular</option>",
                            "<option>rectangular</option>",
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
            "</table>"].join("")
        },
        "help": {
            active_url: "http:
            doNotChangeMyUrl: true
        }
    });

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
    this.dvs = [];

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
