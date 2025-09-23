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

vjHO.register('excel-file').Constructor=function ()
{
    if (this.objCls) return;
    vjHiveObjectSvcBase.call(this);

    this.fullview=function(node,dv)
    {
        this.mode = 'fullview';
        this.preview(node,dv);
    };

    this.preview = function(node,dv)
    {
        this.mode = 'preview';
        this.loadedID = node.id;
        var formObject = gAncestorByTag(gObject(dv.obj.name), "form");
        var previewViewers = [];

        this.addDownloadViewer();
        var this_ = this;
        var oldDownloadPreEditTable = this.viewers["download"].preEditTable;
        this.viewers["download"].preEditTable = function(viewer) {
            for (var ir=0; ir<viewer.tblArr.rows.length; ir++) {
                var node = viewer.tblArr.rows[ir];
                if (node.name == "orig_name") {
                    node.hidden = false;
                    node.pretty_name = node.value;
                    node.description = "Excel file";
                    node.url = "?cmd=objFile&ids=" + node.id;
                }
            }
            oldDownloadPreEditTable.call(viewer, viewer);
        };
        this.viewers["download"].getDescription = function(filename) { return "Excel sheet"; };
        this.addviewer("table_preview_panel", new vjBasicPanelView({
            data: ["dsVoid", "table_preview", "table_sheets"],
            formObject: formObject,
            urlExchangeParameter: function(url, arg, val) {
                if (url.indexOf("qpbg_") != 0) {
                    return url;
                } else if (arg == "tbl") {
                    return url.replace(new RegExp('^(qpbg_[^:]+:
                } else if (arg == "start" || arg == "cnt") {
                    url = urlExchangeParameter(url, arg, val);
                    var start = parseInt(this.computeSubmitableValue("start", "0", url));
                    if (!start)
                        start = 0;
                    var cnt = parseInt(this.computeSubmitableValue("cnt", "1000", url));
                    if (!cnt)
                        cnt = 1000;
                    if (start + cnt < 100 && this.computeSubmitableValue("src", "-", url) != "-") {
                        url = urlExchangeParameter(url, "parseCnt", start + cnt + 1)
                    } else {
                        url = urlExchangeParameter(url, "parseCnt", "-");
                    }
                    return url;
                } else {
                    return urlExchangeParameter(url, arg, val);
                }
            },
            computeSubmitableValue: function(arg, def, url) {
                if (arg == "tbl" && url.indexOf("qpbg_") == 0) {
                    var filename = url.split("
                    this.evalVariables.tbl = filename ? filename : "-";
                    return filename;
                } else {
                    return docLocValue(arg, def, url);
                }
            },
            evalVariables: { tbl : "-" }
        }));
        this.viewers["table_preview_panel"].rows.push({
            name: "tbl",
            order: 0,
            title: "Sheet",
            description: "Select sheet file",
            type: "select",
            isSubmitable: true,
            isSubmitter: true
        }, {
            name: "tblQry",
            order: 0.5,
            title: "Analyze",
            description: "Analyze table with Table Query",
            url: "?cmd=tblqry-new&objs=" + this.loadedID + "&tbl=$(tbl)",
            target: "new",
            icon: "rec"
        });
        var oldPreviewPanelComposerFunction = this.viewers["table_preview_panel"].composerFunction;
        this.viewers["table_preview_panel"].composerFunction = function(viewer, content) {
            var tblRow;
            for (var i=0; i<this.rows.length; i++) {
                if (this.rows[i].name == "tbl") {
                    tblRow = this.rows[i];
                    break;
                }
            }

            if (!tblRow.options) {
                tblRow.options = [];
                var fileTable = new vjTable(vjDS[this.data[2]].data, 0, vjTable_propCSV);
                var defaultFilename;
                if (fileTable.rows.length) {
                    for (var i=0; i<fileTable.rows.length; i++) {
                        var filename = fileTable.rows[i].name;
                        tblRow.options.push([filename]);
                    }
                    tblRow.options.sort(cmpCaseNatural);
                    if (tblRow.options[0]) {
                        tblRow.value = defaultFilename = tblRow.options[0][0];
                    }
                }
                if (this.tree) {
                    var treeValues = {};
                    this.tree.enumerate(function(viewer, node) {
                          treeValues[node.path] = node.value;
                    }, this);
                    this.rebuildTree();
                    this.tree.enumerate(function(viewer, node) {
                          if (treeValues[node.path] != undefined)
                              node.value = treeValues[node.path];
                    }, this);
                    this.redrawMenuView();
                }
                if (vjDS[this.data[1]].url == "static:
                    this_.readyPreview(defaultFilename);
                }
            }
            oldPreviewPanelComposerFunction.call(this, viewer, content);
        };
        this.addviewer("table_preview", new vjTableView({
            data: "table_preview",
            formObject: formObject,
            bgColors: ['#f2f2f2','#ffffff'],
            geometry: { width: "100%"},
            isok: true
        }));
        dv.obj.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["table_preview_panel"], this.viewers["table_preview"]]);
        dv.obj.addTab(this.tabs.names[1], this.tabs.icons[1], [this.viewers["download"]]);
        dv.obj.render(true);
        dv.obj.load("rerender");
    };

    this.readyPreview = function(filename) {
        var panel = this.viewers["table_preview_panel"];
        if (!filename) {
            var tblNode = panel.tree ? panel.tree.findByPath("/tbl") : null;
            filename = tblNode ? tblNode.value : null;
        }
        if (!filename) {
            return;
        }
        panel.evalVariables.tbl = filename;
        var ds = this.makeDS("table_preview");
        ds.reload("qpbg_tblqryx4:
    };

    this.addUrlSet({
        "table_sheets": {
            title: "Retrieving list of sheets",
            active_url: "http:
            objs: "ids",
            header: "id,file,path,name"
        },
        "table_preview": {
            title: "Retrieving table preview"
        },
    });

    this.urlSet["downloadables"].header = null;
    this.urlSet["downloadables"].active_url = "http:

    this.tabs = {
        icons: ["table", "download"],
        names: ["table preview", "available downloads"]
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};

vjTable_hasHeader;
