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

function vjTaxonomicControl(viewer)
{
    if (!this.objID) this.objID = node.id;
    if (!viewer.extraColumns)
        viewer.extraColumns = [];

    function printExtraColumns() {
        var ret = viewer.extraColumns.join("|");
        if (ret.length)
            ret = "|" + ret;
        return ret;
    }

    var extraColumnTitles = {
        num: "%",
        min: "Min",
        max: "Max",
        mean: "Mean",
        stddev: "Std Dev",
        intval: "Conf Intval"
    };

    function matchResults(node) {
        if (node === undefined)
            node = this.tree.root;

        if (node.matchResults !== undefined)
            return node.matchResults;

        if (node.leafnode || node.leaf) {
            node.matchResults = parseInt(node.matchCnt);
        } else {
            if(node.matchCnt)
                node.matchResults = parseInt(node.matchCnt);
            else node.matchResults = 0;
            for (var i = 0; i < node.children.length; i++)
                node.matchResults += matchResults(node.children[i]);
        }
        return node.matchResults;
    };

    function registerNodeClickCallbacks(phylogram, node) {
        phylogram.registerOnclickCallback(node,eval(viewer.taxTreeSelected.substring(9)).func,0,eval(viewer.taxTreeSelected.substring(9)).obj, {onSymbol:true, onLabel:true});
    }

    var phylogram;

    function treeSeriesLoaded() {
        matchResults(this.tree.root);
        this.tree.enumerate(function(phylogram, node) {
            if (node.doneInitialExpansion)
                return;
            node.expanded = true;
            node.doneInitialExpansion = true;
        }, phylogram);
        this.refreshWithoutRebuildingTree();
        this.tree.enumerate(registerNodeClickCallbacks, phylogram, 0, 0, this.tree.root);
    }

    var series = new vjTreeSeries({
        name: viewer.data,
        title: "Tree series",
        showRoot:1,
        dataFormat: "csv",
        type: "rectangular",
        rectangularLabelInline: true,
        precompute:"node.path=node.path",
        cmdMoreNodes: "http:
        postcompute: function(serie, node) {
            if (node.name) {
                var spl = node.name.split(':');
                node.title = spl[0];
                node.taxid = spl[1];
                node.childrenCnt = parseInt(spl[2]);
            }
        }
    });
    series.register_callback({ obj: series, func: treeSeriesLoaded }, "series_loaded", "refreshed");
    phylogram = new vjSVG_Phylogram({
        nodeLabel: function (node, series) {
            if(node.matchResults)
                return node.title + "[" + node.matchResults+"]";
            else if(node.childrenCnt && viewer.taxTreeOnly)
                return node.title + "[" + node.childrenCnt+"]";
            else return node.title;
        },
        nodeTooltip: function (node, series, isLabelText) {
            if (isLabelText)
                return node.title;
            return this.defaultNodeTooltip.apply(this, arguments);
        },
        nodeSymbolBrush: function (node, series1) {
            if(node.matchCnt)
                return {fill:"#33FF99", 'fill-opacity':1, opacity:1};
            else if(node.matchResults)
                return {fill:"#9999FF", 'fill-opacity':1, opacity:1};
            else if(node.childrenCnt && viewer.taxTreeOnly)
                return {fill:"#9999FF", 'fill-opacity':1, opacity:1};
            else return {fill:"#CCCCCC", 'fill-opacity':1, opacity:1};
        },
        nodeSymbolSize: function (node, series) {
            return node.leafnode ? 5 : 6;
        },
        nodeLabelFont: function (node) {
            var font = { "font-size": 10};
            return font;
        }
    });
    phylogram.add(series);
    
    
    var downloadURL = "http:
    var downloadAccURL = null;
    var downloadPATHURL = null;
    if(viewer.updateURL && !viewer.taxTreeOnly){
        downloadURL += "&screenId="+viewer.updateURL;
        if(viewer.data=='blastNTset') {
            downloadURL += "&screenType=dna-alignx_screenResult.csv&percentage=1";
            downloadAccURL = "http:
            downloadPATHURL = "http:
        }
    }
    if (viewer.cmdUpdateURL)
       downloadURL = viewer.cmdUpdateURL;
    this.myRows = new Array();

    if (viewer.viewMode == "fullview") {
        this.myRows.push({
            name: 'downloadAll',
            align: 'left',
            showTitle: true,
            hidden: false,
            title: 'Taxid Based Result',
            description: 'Simplified Result',
            icon: 'download',
            url: function(panel, node) {
                var myUrl = location.href.substring(0,location.href.indexOf("dna.cgi?")) + downloadURL.substring(7) + "&mode=csv";
                funcLink(myUrl);
            }
        });
        this.myRows.push({
            name: 'downloadAccession',
            align: 'left',
            showTitle: true,
            hidden: false,
            title: 'Accession Based Result',
            description: 'Full Detail Result',
            icon: 'download',
            url: function(panel, node) {
                var myUrl = location.href.substring(0,location.href.indexOf("dna.cgi?"))  + downloadAccURL.substring(7) + "&mode=csv";
                funcLink(myUrl);
            }
        });
        this.myRows.push({
            name: 'downloadPathogen',
            align: 'left',
            showTitle: true,
            hidden: false,
            title: 'Pathogen Table Result',
            description: 'Pathogen Table',
            icon: 'download',
            url: function(panel, node) {
                var myUrl = location.href.substring(0,location.href.indexOf("dna.cgi?"))  + downloadPATHURL.substring(7) + "&mode=csv";
                funcLink(myUrl);
            }
        });
    }
    if(viewer.ifFromShortRead){
        this.myRows.push({name:'screen', title: 'Screen' ,order:2,align:'left', icon:'img/scope.gif' , description: 'open screening information' ,  url: "?cmd=dna-screening&id="+viewer.updateURL});
        this.myRows.push({name:'launchDnaScreen', type:'button' ,align:'left', hidden: (viewer.hideScreenLauncher != undefined) ? viewer.hideScreenLauncher : true ,isSubmitable:true, title:'Launch Dna-Screening', description: 'Dna-screening launcher', url: launchQC_Screen, dsCmdLink: "launchSvc&key=dna-screening&query=" + viewer.updateURL});
        this.myRows.push({name:'refresh', title: 'Refresh' , align: "right",order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"});
    }
    
    this.myPanel = new vjPanelView({
        data: ["dsVoid", viewer.data],
        iconSize: 24,
        rows:viewer.rows? viewer.rows : this.myRows,
        formObject: document.forms[viewer.formName],
    });
    if(viewer.ifFromShortRead){
        this.myPanel.dependentData = viewer.dependentData;
        this.myPanel.progressView = viewer.progressView;
        this.myPanel.objId = viewer.updateURL;
    }

    this.myTaxTree = new vjSVGView({
        icon:viewer.icon,
        name: 'phylogram',
        height: 300,
        width: 500,
        defaultEmptyText:"Analyze using CensuScope to view content",
        chartArea:{height:'95%'},
        objectsDependOnMe: this.myPanel,
        plots:[phylogram],
        precompute:"this.tree.enumerate('node.expanded=true');",
        formObject: document.forms[viewer.formName]
    });

    this.myTaxTreeText = new vjTreeView({
        icon:'tree',
        data: viewer.data,
        name: 'tree',
        hidden:true,
        chartArea:{height:'95%'},
        objectsDependOnMe: this.myPanel,
        formObject: document.forms[viewer.formName],
        showRoot: true,
        showLeaf: true,
        checkLeafs: false,
        doNotPropagateUp: true,
        icons: { leaf: 'img/scope.gif' },
        showChildrenCount: true,
        autoexpand:0,
        checkable: false,
        isok: true,
        linkLeafCallback:viewer.taxTreeSelected,
        linkBranchCallback:viewer.taxTreeSelected
    });

    var myTaxListCols = [ { name:new RegExp(/./), hidden:true },
                          { name: 'taxid', hidden: false, title: 'Taxid',order:1 },
                          { name: 'name', hidden: false, title: 'Name' ,order:2},
                          { name: 'path', hidden: false, title: 'Full Lineage' ,order:15},
                          { name: 'matchCnt', hidden: true, title: 'Total Hit(s)',order:3 },
                          { name: 'matchCnt_pct', hidden: false, title: 'Percentage',order:3},
                          { name: 'min', hidden: false, title: 'Min',order:4},
                          { name: 'min_pct', hidden: false, title: 'Min Pct',order:5},
                          { name: 'max', hidden: false, title: 'Max',order:6},
                          { name: 'max_pct', hidden: false, title: 'Max Pct',order:7},
                          { name: 'mean', hidden: false, title: 'Mean',order:8},
                          { name: 'mean_pct', hidden: false, title: 'Mean Pct',order:9},
                          { name: 'stddev', hidden: false, title: 'Std Dev',order:10},
                          { name: 'intval', hidden: false, title: 'Interval',order:11},
                          { name: 'childrenCnt', hidden: false, title: 'Numbers of Children',order:5 } ];
    
    this.myTaxList = new vjTableView({
        icon:'list',
        data: viewer.data,
        name: 'list',
        hidden:true,
        cols: myTaxListCols,
        precompute:"node.path=node.path.replace(/:*[0-9]/g,'');",
        
        bgColors: ['#f2f2f2', '#ffffff'],
        formObject: document.forms[viewer.formName],
        maxTxtLen :50,
        geometry:{height:'100%'},
        selectCallback: viewer.taxTreeSelected ,
        checkable:false,
        chartArea:{height:'95%'},
        objectsDependOnMe: this.myPanel
    });
    
    vjDS[viewer.data].url = downloadURL;
    
    return [this.myPanel,this.myTaxTree,this.myTaxTreeText,this.myTaxList];
}

function vjTaxonomicDetailsView(viewer)
{
    this.myRecordViewer = new vjRecordView({
        data: [viewer.data.dsTaxonomyViewerSpec, viewer.data.taxDetails],
        name:'details',
        icon:'file',
        formObject: document.forms[viewer.formName],
        showRoot: false,
        hideViewerToggle:true,
        autoStatus: 3,
        autoDescription: false,
        implementSaveButton: false,
        implementTreeButton: false,
        showReadonlyInNonReadonlyMode: true,
        constructAllNonOptionField: true,
        implementSetStatusButton: false,
        isok: true
    });

    this.myHelpViewer = new vjHelpView ({ data:viewer.data.HelpInfo})

    return [this.myRecordViewer,this.myHelpViewer];
}

