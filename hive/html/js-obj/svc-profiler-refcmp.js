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

function vjProfilerRefcmpPanel(viewer) {
    this.name = "refcmp-panel";
    this.showInfo = true;
    this.iconSize = 24;
    this.refcmpParams = viewer.refcmpParams;
    this.rows = [
        {
            name: "cutoff",
            path: "/cutoff",
            value: "" + this.refcmpParams.cutoff,
            type: "search",
            prefix: "Match cutoff at&nbsp;",
            title: "Match cutoff",
            rgxpOff: true,
            isRgxpControl: false,
            isSubmitable: true,
            align: "left",
            order: 1
        },
        {
            name: "show-match",
            path: "/cutoff/show-match",
            type: "checkbox",
            title: "show matching positions",
            isSubmitable: true,
            value: this.refcmpParams.showMatch ? "1" : "0",
            order: 3
        },
        {
            name: "show-mismatch",
            path: "/cutoff/show-mismatch",
            type: "checkbox",
            title: "show mismatching positions",
            isSubmitable: true,
            value: this.refcmpParams.showMismatch ? "1" : "0",
            order: 4
        },
        {
            name: "pager",
            icon: "page",
            title: "per page",
            align: "right",
            type: "pager",
            counters: [10, 20, 50, 100, 1000, "all"],
            order: 100
        }
    ];

    this.urlExchangeParameter = function(url, parname, value) {
        if (parname == "cutoff") {
            this.refcmpParams.cutoff = parseFloat(value);
            if (!(this.refcmpParams.cutoff >= 0 && this.refcmpParams.cutoff <= 1)) {
                this.refcmpParams.cutoff = this.refcmpParams.cutoffDefault;
            }
        } else if (parname == "show-match") {
            this.refcmpParams.showMatch = parseBool(value) && value != "-";
        } else if (parname == "show-mismatch") {
            this.refcmpParams.showMismatch = parseBool(value) && value != "-";
        } else {
            return urlExchangeParameter(url, parname, value);
        }

        var prepIn = vjDS.escapeQueryLanguage("match_cols = rowcells(-1).kv().filter({this[1] =~ /^Match/}).map({this[0]});");
        var condOut = vjDS.escapeQueryLanguage("match = 0; mismatch = 0; match_cols.foreach({if(cell(curRow, this) < " + this.refcmpParams.cutoff + ") { mismatch++; } else { match++; }}); return ");
        if (this.refcmpParams.showMatch) {
            if (this.refcmpParams.showMismatch) {
                condOut = "-";
                prepIn = "-";
            } else {
                condOut += "match;";
            }
        } else {
            if (this.refcmpParams.showMismatch) {
                condOut += "mismatch;";
            } else {
                condOut = "0";
            }
        }
        url = urlExchangeParameter(url, "prepIn", prepIn);
        url = urlExchangeParameter(url, "condOut", condOut);
        return url;
    };

    this.computeSubmitableValue = function(name, defval, url) {
        if (name == "cutoff") {
            return "" + this.refcmpParams.cutoff;
        } else if (name == "show-match") {
            return this.refcmpParams.showMatch ? "1" : "0";
        } else if (name == "show-mismatch") {
            return this.refcmpParams.showMismatch ? "1" : "0";
        }
        return docLocValue(name, defval, url);
    };
    vjPanelView.call(this, viewer);
};

vjHO.register("svc-profiler-refcmp").Constructor = function() {
    if (this.objCls) return;
    vjHiveObjectSvcBase.call(this);

    this.customizeSvcDownloads = function(obj) {
        this.parent["svc"].customizeDownloads(this.downloadViewerDescriptions);
    };

    this.fullview = function(node, dv) {
        if (this.isRendered) return;
        this.create(dv, node.id);
        this.isRendered=true;
    };

    this.preview = function(node, dv) {
        this.parent.preview("svc", node, dv, "vjObj['"+this.objCls+"'].customizeSvcDownloads();");
        if(!node.status || parseInt(node.status)<5) return;

        this.create(dv, node.id);
        this.isRendered=true;
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

        var this_svc = this; // for use in closures;

        this.addviewer("refcmp", new vjTableView({
            data: "refcmp",
            bgColors: [],
//            cellColorCallback: this.coloringCells,
            cellColorCallback: "function:vjObjFunc('coloringCells','" + this.objCls + "')",
//            selectCallback: "function:vjObjFunc('onSelectedHitListItem','" + this.objCls + "')",

            precompute: function(viewer, tbl, ir) {
                var node = tbl.rows[ir];

                var match = 1;
                for (var ic=0; ic<tbl.hdr.length; ic++) {
                    if (tbl.hdr[ic].name != "Match")
                        continue;
                    match = Math.min(match, parseFloat(node.cols[ic]));
                }
                if (match < this_svc.refcmpParams.cutoff) {
                    viewer.bgColors[ir] = vjHCL(45, 1 - match, (1 + match)/2).hex();
                } else {
                    viewer.bgColors[ir] = "#ffffff";
                }
            }
        }));

        this.addviewer("refcmp-panel", new vjProfilerRefcmpPanel({
            data: ["dsVoid", "refcmp"],
            formObject: formObject,
            refcmpParams: this.refcmpParams
        }));
    };

    this.construct = function(dv, id) {
        if (!this.loaded) return;
        this.constructed = true;
        this.viewersToAdd = [];

        var dv_obj = dv.obj.length ? dv.obj[0] : dv.obj;

        var tab = dv_obj.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["refcmp-panel"], this.viewers["refcmp"]]);
        dv_obj.render();
        dv_obj.load("rerender");
    };

    this.typeName = "svc-profiler-refcmp";
    this.objCls = "obj-" + this.typeName + "-" + Math.random();
    vjObj.register(this.objCls, this);

    this.viewersToAdd = [];
    this.viewers = {length: 0};
    this.dsTitle = "data for profile match to reference";
    this.tabs = {
        icons: ["table", "table"],
        names: ["Results", "Downloads"]
    };
    this.addUrlSet({
        "refcmp" : {
            title: "Retrieving match data",
            active_url: "qpbg_tblqryx4://refcmp.csv//start=0&cnt=20",
            objs: "objs"
        }
    });

    this.downloadViewerDescriptions = {
        "refcmp.csv" : "match of references to sequence alignment profile"
    };

    this.refcmpParams = {
        cutoffDefault: 0.5,
        cutoff: 0.5,
        showMatch: true,
        showMismatch: true
    };

    this.coloringCells = function (viewer, realText, col, row){
        var match = Math.min(1, parseFloat(realText));
        var cutoff = this.refcmpParams.cutoff;
        var color = "#FFFFFF"
        if (col.name.indexOf("Match") == -1){
            return 0;
        }
        if (match < cutoff){
            match = (1-match) < 0.1 ? 0.9 : match;
            color = vjHCL(45, 1 - match, (1 + match)/2).hex();
        }
        return color;
    }
};
