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

javaScriptEngine.include("js/vjBumperLocater.js");
javaScriptEngine.include("js/vjSVGGeneView.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js-graph/vjSVG_GenomeBrowser.js");

javaScriptEngine.include("d3js/annotation_box.js");

function vjProfilerSummaryView(viewer) {
    this.maxTxtLen = 48;
    this.defaultEmptyText = 'no information to show';
    this.newSectionWord = "-";
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.isNSortEnabled = true;
    vjTableView.call(this, viewer);
}

function vjProfilerDownloadsView(viewer) {
    this.maxTxtLen = 28;
    this.iconSize = 0;
    this.defaultEmptyText = "select reference genome to see available downloads";
    this.geometry = {
        width : "65%"
    };
    this.selectCallback = "function:vjObjFunc('onPerformProfileOperation','"
            + this.objCls + "')";
    this.cols = [ {
        name : 'data',
        title: 'Data',
        order:1,
        hidden : false
    }, {
        name : 'down',
        type : 'icon',
        title: 'Get',
        iconSize:16,
        align:'center',
        order:4,
        hidden : false
    }, {
        name : 'arch',
        type : 'icon',
        title: 'Archive',
        iconSize:16,
        align:'center',
        order:5,
        hidden : false
    }, {
        name : 'operation',
        hidden : true
    }, {
        name : 'blob',
        hidden : true
    } ];
    this.forceRepeatedTooltip = true;
    vjTableView.call(this, viewer);
}

function vjProfilerContigControl(viewer) {
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentHitListControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentHitListControl");
    }
    this.formObject=document.forms[this.formName];
    this.panelViewer=new vjBasicPanelView({
        data:["dsVoid",this.data],
        width:this.width,
        rows: [{name:'refresh', title: 'Refresh' ,order:3, icon:'refresh' ,align :'left', description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
               {
                    name : 'download',
                    title : 'Download',
                    icon : 'download',
                    align : 'left',
                    order : '2',
                    description : 'Download content',
                    path : "/download"
                },
                {
                    name : 'downloadall',
                    path : "/download/all",
                    showTitle : true,
                    title : 'All',
                    icon : 'download',
                    align : 'left',
                    description : 'Download full content',
                    url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(url,'start','-'),'cnt','-'),'down','1'))"
                },
                {
                    name : 'downloadpage',
                    path : "/download/page",
                    showTitle : true,
                    title : 'Page',
                    icon : 'download',
                    align : 'left',
                    description : 'Download content of the current page',
                    url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(url,'down','1'))"
                },{
                    name : 'Parameters',
                    path: '/Parameters',
                    title : 'Parameters',
                    showTitle: true,
                    icon : "reqstatus2",
                    align : 'left',
                    order: '1',
                    description : 'Parameters'
                },
                {
                    name : 'gapWindowSize',
                    prefix: 'Gap Windows Size: ',
                    path: '/Parameters/gapWindowSize',
                    title : 'Gap Windows Size',
                    type:"text",
                    align : 'left',
                    value:"30",
                    order: '1',
                    size: "8",
                    isSubmitable: true,
                    description : 'Gap Windows Size'
                },
                {
                    name : 'gapThreshold',
                    prefix: 'Gap Threshold: ',
                    path: '/Parameters/gapThreshold',
                    title : 'Gap Coverage Threshold',
                    type:"text",
                    align : 'left',
                    value: "1",
                    order: '1',
                    size: "8",
                    isSubmitable: true,
                    description : 'Gap Threshold'
                },
                {
                    name: "update",
                    align: "right",
                    order: "100",
                    type:"submitter",
                    showTitle: true,
                    description: "Update",
                    isSubmitable: true,
                    path: "/Parameters/update"
                }
                ,{
                    name : 'separ',
                    type : 'separator',
                    align : 'left',
                    align : 'left',
                    order : '2'
                },
               {name:'pager', icon:'page' , title:'per page',order:2,align:'right', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
              ],
        empty:false,
        loadedID: this.loadedID,
        formObject:this.formObject
    });
    this.contigViewer = new vjProfilerContigView({
        data: this.data,
        formObject: this.formObject,
        width:this.width,
    });
    return [this.panelViewer, this.contigViewer];
}

function vjProfilerContigView(viewer) {
    this.maxTxtLen = 48;
    this.defaultEmptyText = 'no information to show';
    this.newSectionWord = "-";
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    vjTableView.call(this, viewer);
}

function vjProfilerAnnotationListPanel(viewer){
      var orfZoom = [ {
            name : 'show annotation',
            title:'Show Annotation',
            icon : 'done',
            order:1,
            align : 'left',
            description : 'Apply',
            showTitle:true,
            url : viewer.callbackForPanel
        },{
            name : 'types',
            icon : 'down',
            align : 'left',
            order:0,
            description : 'types to show',
            title:'Types',
            showTitle:true
        },{
            name : 'nothing',
            title:'Please select a file',
            align : 'left',
            description : 'select an annotation file',
            order: 0,
            value:0,
            path:'/types/nothing'
        },{
            name : 'announcement',
            align : 'left',
            icon:"redInfo.png",
            order:0,
            description : '',
            title:'Annoucement',
            showTitle:true,
            hidden:true
        }];
        this.iconSize = 24;
        this.rows = orfZoom;
        this.rows = this.rows.concat(
                {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                {name:'pager', icon:'page', title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                { name: 'search', align: 'right', type: ' search',order:1, isSubmitable: true, title: 'Search', description: 'search sequences by ID',order:'1', url: "?cmd=objFile&ids=$(ids)" }
                );
        vjPanelView.call(this, viewer);
    
}

function vjProfilerAnnotationListView(viewer) {

    
    this.formObject=document.forms[viewer.formName];

    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.cols = [ 
      {
        name : 'created',
        hidden : false,
        title : 'created',
        type : 'datetime'
      },
      {
          name : 'size',
          title : 'Size',
          type:'bytes'
      }
    ];
    this.multiSelect = false;
    vjTableView.call(this, viewer);
}

function vjProfilerConsensusView(viewer) {
    this.maxTxtLen = 48;
    this.composer = 'preview';
    this.defaultEmptyText = 'no information to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    vjHTMLView.call(this, viewer);
}

function vjProfilerNoiseView(viewer) {
    this.name = 'noise';
    this.maxTxtLen = 48;
    this.defaultEmptyText = 'no information to show';
    this.newSectionWord = "-";
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.options = {
        vAxis : {
            title : 'Count'
        },
        hAxis : {
            title : 'Frequency'
        },
        focusTarget : 'category',
        lineWidth : 1,
        isStacked : false,
        smoothLine : true,
        legend : 'right',
        chartArea : {
            top : 20,
            left : 70,
            width : '80%',
            height : "80%"
        }

    };
    this.series = [ {
        name : 'Frequency',
        title : 'Frequency of occurence',
        label:true
    }, {
        name : 'A-C'
    }, {
        name : 'A-G'
    }, {
        name : 'A-T'
    }, {
        name : 'C-A'
    }, {
        name : 'C-G'
    }, {
        name : 'C-T'
    }, {
        name : 'G-A'
    }, {
        name : 'G-C'
    }, {
        name : 'G-T'
    }, {
        name : 'T-A'
    }, {
        name : 'T-C'
    }, {
        name : 'T-G'
    } ];
    this.type = 'line';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerFrequencyHistogramView(viewer)
{
    this.name = 'frequency_histograms';
    this.maxTxtLen = 48;
    this.defaultEmptyText = 'no information to show';
    this.newSectionWord = "-";
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.options = {
        vAxis : {
            title : 'Count'
        },
        hAxis : {
            title : 'Frequency'
        },
        focusTarget : 'category',
        lineWidth :2,
        isStacked : true,
        smoothLine : false,
        legend : 'right',
        chartArea : {
            top : 20,
            left : 70,
            width : '80%',
            height : "70%"
        }

    };
    this.logGraph=10;
    this.series = [ {
        name : 'Frequency',
        title : 'Frequency of occurence'
    }, {
        name : 'A-C'
    }, {
        name : 'A-G'
    }, {
        name : 'A-T'
    }, {
        name : 'C-A'
    }, {
        name : 'C-G'
    }, {
        name : 'C-T'
    }, {
        name : 'G-A'
    }, {
        name : 'G-C'
    }, {
        name : 'G-T'
    }, {
        name : 'T-A'
    }, {
        name : 'T-C'
    }, {
        name : 'T-G'
    } ];
    this.type = 'column';
    vjGoogleGraphView.call(this, viewer);
}


function vjProfilerNoiseIntegralView(viewer) {
    this.name = 'thresholds';
    this.maxTxtLen = 48;
    this.cols = [ {
        name : 'Threshold',
        type : 'percent*100'
    } ];
    this.defaultEmptyText = 'no information to show';
    this.newSectionWord = "-";
    this.bgColorst=['#f2f2f2', '#ffffff'];
    vjTableView.call(this, viewer);
}

function vjProfilerFrequencyHistogramIntegralView(viewer) {
    this.name = 'thresholds';
    this.maxTxtLen = 48;
    this.cols = [ {
        name : 'Threshold',
        type : 'percent*100'
    } ];
    this.defaultEmptyText = 'no information to show';
    this.newSectionWord = "-";
    this.bgColorst=['#ffffDf', '#ffffff'];
    vjTableView.call(this, viewer);
}


function vjProfilerConsensusMenu(viewer) {
    this.name = "menu";
    this.iconSize = 24;
    this.isNpagerUpdate = true;
    this.showInfo = true;
    vjPanelView.call(this, viewer);

    this.defaultRows = [
        {
            name : 'gaps',
            type : 'select',
            isSubmitable : true,
            isSubmitter : true,
            isNAlignRight:true,
            options : [[0,'Gaps will be represented with "-" and "+"','displayed'],['fill','Gaps will be replaced by reference sequence','filled'],['skip','Gaps will be ignored','skipped'],['split','Gaps are excluded and consensus is split to smaller sequences','split']],
            align : 'right',
            order : '1',
            title : 'Gaps',
            value: 0
        },
        {
            name : 'consensus_thrshld',
            type : 'text',
            forceUpdate : true,
            size : '1',
            isSubmitable : true,
            prefix : 'Threshold',
            align : 'right',
            order : '0',
            description : "Position whose consensus is below the threshold [0-100] will be treated as gaps",
            title : '0.00'
        },
        {
            name : 'update',
            align : 'left',
            type : "button",
            order : 10,
            value : 'Update',
            description : 'Update content',
            url : "javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls
                    + "\")"
        }
    ];

    this.rows = [].concat(this.defaultRows);
}

function vjProfilerSNPProfileMenu(viewer) {

    
    this.onUpdateSNPGraphs = function (container, node, menuViewer)
    {
        var children = menuViewer.tree.findByName("showGraphs").children;
        var parentChildren = $("#"+menuViewer.parent.divName).children();
        
        for (var i = 0; i < children.length; i++)
        {
            var tmp = children[i];
            
            if (tmp.value && (parseInt(tmp.value) == 1 || tmp.value == true))
                $(parentChildren[i+1]).removeClass("hidden");
            else
                $(parentChildren[i+1]).addClass("hidden");
        }
    }
    
    this.name = "menu";
    this.iconSize = 24;
    this.showInfo = true;
    this.defaultRows = [
        {
            name: "nameOfReference",
            title: "",
            align: "left",
            order: "3",
            description: "Name of Reference",
        },
        {
            name: "download",
            title: "Download",
            icon: "download",
            align: "left",
            order: "-1",
            description: "Download content",
            path: "/download"
        },
        {
            name: "showGraphs",
            title: "Show Graphs",
            icon: "folderMinus",
            align: "left",
            order: "0",
            description: "Choose which graphs will be shown",
            path: "/showGraphs"
        },
        {
            name: "coverage",
            title: "Coverage Histogram",
            align: "left",
            value: "1",
            order: "2",
            type: "checkbox",
            path: "/showGraphs/coverage"
        },{
            name: "unaligned_tails",
            title: "Unaligned Tails",
            align: "left",
            order: "3",
            type: "checkbox",
            path: "/showGraphs/unaligned_tails"
        },
        {
            name: "disbalance",
            title: "Profile Disbalance",
            align: "left",
            order: "4",
            type: "checkbox",
            path: "/showGraphs/disbalance"
        },
        {
            name: "entropy",
            title: "Profile Entropy",
            align: "left",
            order: "5",
            type: "checkbox",
            path: "/showGraphs/entropy"
        },
        {
            name: "quality",
            title: "Profile Quality",
            align: "left",
            order: "6",
            type: "checkbox",
            path: "/showGraphs/quality"
        },
        {
            name: "snp",
            title: "SNP",
            align: "left",
            value: "1",
            order: "7",
            type: "checkbox",
            path: "/showGraphs/snp"
        },
        {
            name: "in.del",
            title: "Insertions/Deletions",
            align: "left",
            order: "8",
            type: "checkbox",
            path: "/showGraphs/in.del"
        },
        {
            name: "annotation",
            title: "Profile Annotations",
            align: "left",
            order: "9",
            type: "checkbox",
            path: "/showGraphs/annotation"
        },
        {
            name: "update",
            title: "Update",
            align: "left",
            order: "100",
            type: "button",
            path: "/showGraphs/update",
            onClickFunc: this.onUpdateSNPGraphs,
            url: "javascript:vjObjEvent(\"onUpdateSNPGraphs\", \"" + this.objCls+ "\");"
        },
        {
            name: "start",
            title: "default",
            type: "text",
            align: "right",
            order: "10",
            prefix: "Start position:&nbsp;",
            description: "Start position",
            path: "/start",
            isSubmitable: true
        },
        {
            name: "end",
            title: "default",
            type: "text",
            align: "right",
            order: "11",
            prefix: "End position:&nbsp;",
            description: "End position",
            path: "/end",
            isSubmitable: true
        },
        {
            name: "submitter",
            type: "submitter",
            align: "right",
            order: "100",
            description: "Use specified start/end positions",
            path: "/submitter",
            isSubmitable: true
        }
    ];
    this.rows = [].concat(this.defaultRows);

    this.setDownloads = function(downloads) {
        this.rows = [].concat(this.defaultRows);
        for (var i=0; i<downloads.length; i++) {
            var d = downloads[i];
            var node = {
                name: "download-item-" + i,
                title: d.title,
                path: "/download/download-item-" + i,
                isSubmitable: true,
                url: "static:
                makeUrlParam: d
            };
            this.rows.push(node);
        }
        this.rebuildTree();
        this.refresh();
    };

    this.urlExchangeParameter = function(url, parname, newvalue) {
        if (parname == "start" || parname == "end" || parname == "cnt" || parname == "tqs") {
            var elStart = this.formObject.elements[this.objCls + "_start"];
            var elEnd = this.formObject.elements[this.objCls + "_end"];
            var start = !isNaN(parseInt(elStart.value)) ? parseInt(elStart.value) : 0;
            var end = !isNaN(parseInt(elEnd.value)) ? parseInt(elEnd.value) : 0;
            var tqs;
            try {
                if (parname == "tqs") {
                    tqs = JSON.parse(newvalue);
                } else {
                    tqs = JSON.parse(docLocValue("tqs", "[]", url));
                }
            } catch (e) { }
            if (!tqs) {
                tqs = [];
            }
            var iop_slice = -1;
            for (var i=0; i<tqs.length; i++) {
                if (tqs[i].start_end_slice) {
                    iop_slice = i;
                    break;
                }
            }
            if (start || end) {
                var op_slice =  {
                    op: "slice",
                    start_end_slice: true,
                    arg: {}
                };
                if (start) {
                    op_slice.arg.start = {
                        col: { "name" : "Position" },
                        value: start - 1,
                        method: "firstGreaterThan"
                    };
                }
                if (end) {
                    op_slice.arg.end = {
                        col: { "name" : "Position" },
                        value: end + 1,
                        method: "lastLessThan"
                    };
                }
                if (iop_slice >= 0) {
                    tqs[iop_slice] = op_slice;
                } else {
                    tqs.push(op_slice);
                }
            }

            if (tqs.length) {
                return urlExchangeParameter(url, "tqs", vjDS.escapeQueryLanguage(JSON.stringify(tqs)));
            } else {
                return urlExchangeParameter(url, "tqs", "-");
            }
        } else {
            return urlExchangeParameter(url, parname, newvalue);
        }
    };

    this.isDefault = function() {
        return this.urlExchangeParameter("?", "start") == "?";
    };

    this.computeSubmitableValue = function(name, defval, url) {
        if (name == "start" || name == "end") {
            var tqs;
            try {
                tqs = JSON.parse(docLocValue("tqs", "[]", url));
            } catch (e) { }
            if (!tqs) {
                tqs = [];
            }
            for (var i=0; i<tqs.length; i++) {
                if (tqs[i].start_end_slice) {
                    var arg = tqs[i].arg;
                    if (name == "start") {
                        return arg && arg.start ? arg.start.value + 1 : defval;
                    } else if (name == "end") {
                        return arg && arg.end ? arg.end.value - 1 : defval;
                    }
                    break;
                }
            }
        }
        return docLocValue(name, defval, url);
    };

    vjPanelView.call(this, viewer);
}

function vjProfilerSNPProfileCoverage(viewer) {
    this.name = "coverage";
    this.icon = 'graph';
    this.series = [ {
        name : 'Position'
    }, {
        name : 'Count Forward'
    }, {
        name : 'Count Reverse'
    } ];
    this.options = {
        focusTarget : 'category',
        width : viewer.width?viewer.width:700,
        height : viewer.height?viewer.height:150,
        changeHeight: false,
        vAxis : {
            title : 'Coverage'
        },
        hAxis : {},
        lineWidth : 1,
        isStacked : true,
        legend : 'top',
        chartArea : {
            top : 20,
            left : 70,
            width : '90%',
            height : "70%"
        },
        color : [ 'blue', 'green' ]
    };

    this.type = 'area';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerSNPProfileDisbalanceView(viewer) {
    this.hidden = true;
    this.icon = 'graph2';
    this.name = "disbalance";
    this.options = {
        width : viewer.width?viewer.width:700,
        height : 150,
        changeHeight: false,
        vAxis : {
            title : 'Disbalance',
            minValue : 0
        },
        lineWidth : 1,
        hAxis : {},
        legend : 'none',
        chartArea : {
            top : 5,
            left : 70,
            width : '90%',
            height : "70%"
        },
        color : [ 'red', 'blue' ]
    };
    this.series = [
            {
                name : 'Position',
                label : false
            },
            {
                name : 'Disbalance Forward',
                eval : 'parseFloat(row["Count Reverse"]) ? parseFloat(row["Count Forward"])/parseFloat(row["Count Reverse"]) : 0'
            },
            {
                name : 'Disbalance Reverse',
                eval : 'parseFloat(row["Count Forward"]) ? parseFloat(row["Count Reverse"])/parseFloat(row["Count Forward"]) : 0'
            } ];
    this.type = 'line';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerSNPProfileEntropyView(viewer) {
    this.hidden = true;
    this.icon = 'graph2';
    this.name = "entropy";
    this.options = {
        width : viewer.width?viewer.width:700,
        height : 150,
        changeHeight: false,
        vAxis : {
            title : 'Entropy',
            minValue : 0,
            maxValue : 1
        },
        lineWidth : 1,
        hAxis : {},
        legend : 'none',
        focusTarget : 'category',
        chartArea : {
            top : 5,
            left : 70,
            width : '90%',
            height : "70%"
        },
        colors : [ 'lightblue', 'navy', 'red', 'green' ]
    };
    this.series = [ {
        name : 'Position'
    }, {
        name : 'Entropy'
    }, {
        name : 'SNP-Entropy'
    }, {
        name : 'Entropy Forward'
    }, {
        name : 'Entropy Reverse'
    } ];
    this.type = 'line';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerSNPProfileAnnotationView(viewer) {
    this.hidden = true;
    this.icon = 'graph2';
    this.name = "annotation";

    this.columnToPick = {refID:"seqID",start:"start",end:"end","idType-id": "idType-id"};
    this.data = viewer.data;
    this.graphOptions= {
                   collapse:true,
                   boxSize: 10,
                   showTipFull: true
               };
    this.downloadSvg =true
    
    this.width = viewer.width ? viewer.width : 700;
    this.height = viewer.height ? viewer.height : 180;
    this.changeHeight = false;
    
    this.selectCallback = viewer.anotSelectCallback;
    this.margin = {right: 60,left:70};
    vjD3JS_annotationBox.call(this,viewer);
}

function vjProfilerSNPProfileQualityView(viewer) {
    this.hidden = true;
    this.name = "quality";
    this.icon = 'hiveseq-cube';
    this.options = {
        width : viewer.width?viewer.width:700,
        height : 150,
        changeHeight: false,
        vAxis : {
            title : 'Quality',
            minValue : 0
        },
        lineWidth : 1,
        hAxis : {},
        legend : 'none',
        areaOpacity : .8,
        chartArea : {
            top : 5,
            left : 70,
            width : '90%',
            height : '70%'
        },
        colors : [ 'red' ]
    };
    this.series = [ {
        name : 'Position'
    }, {
        name : 'Quality'
    } ];
    this.type = 'area';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerCoverageHistogramView(viewer)
{
    this.name = "unaligned_tails";
    this.icon = 'graph';
    this.series = [ {
        name : 'Position'
    },{
        name : 'Left Tail'
    } , {
        name : 'Right Tail'
    },{
        name : 'Length Anisotropy'
    }  ];
    this.hidden=true;

    this.options = {
        focusTarget : 'category',
        width : viewer.width?viewer.width:700,
        height : 150,
        changeHeight: false,
        vAxis : {
            title : 'Dangling tails'
        },
        hAxis : {},
        lineWidth : 1,
        isStacked : true,
        legend : 'top',
        smoothLine:true,
        chartArea : {
            top : 5,
            left : 70,
            width : '90%',
            height : "70%"
        },
        colors : [ 'green', 'cyan', 'brown' , 'navy' , 'green' , 'blue'  ]
    };
    this.type = 'line';

    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerSNPProfileSNPView(viewer) {
    this.icon = 'hiveseq-cube';
    this.name = "SNP";
    this.options = {
        focusTarget : 'category',
        isStacked : true,
        width : viewer.width?viewer.width:700,
        height : 150,
        changeHeight: false,
        vAxis : {
            title : 'SNP %'
        },
        lineWidth : 2,
        isStacked: true,
        legend : 'bottom',
        chartArea : {
            top : 5,
            left : 70,
            height : '70%',
            width : '90%'
        },
        colors : [ 'black' ],
        isok : true
    };
    this.series = [
            {
                name : 'Position',
                label : false
            },
            {
                name : 'Cumulative SNP Frequency',
                eval : 'Math.max(parseFloat(row["Frequency A"]),parseFloat(row["Frequency C"]),parseFloat(row["Frequency G"]),parseFloat(row["Frequency T"]) )'
            } ];
    this.type = 'column';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerSNPProfileSNPInDelsView(viewer) {
    this.icon = 'hiveseq-cube';
    this.name = "in-dels";
    this.hidden = true;
    this.isNrenderOnHide = true;
    this.options = {
        focusTarget : 'category',
        isStacked : true,
        width : viewer.width?viewer.width:700,
        height : 150,
        changeHeight: false,
        vAxis : {
            title : 'InDel %'
        },
        lineWidth : 1,
        legend : 'bottom',
        chartArea : {
            top : 5,
            left : 70,
            height : '70%',
            width : '90%'
        },
        colors : [ 'blue', 'red' ],
        isok : true
    };

    this.series = [
            {
                name : 'Position',
                label : false
            },
            {
                name : 'Insertions',
                eval : 'parseFloat(row["Count Total"]) ? parseInt(10000*parseFloat(row["Count Insertions"])/parseFloat(row["Count Total"]))/100 : 0'
            },
            {
                name : 'Deletions',
                eval : 'parseFloat(row["Count Total"]) ? parseInt(10000*parseFloat(row["Count Deletions"])/parseFloat(row["Count Total"]))/100 : 0'
            } ];
    this.type = 'column';
    vjGoogleGraphView.call(this, viewer);
}

function vjProfilerEntropiesACGTView(viewer) {
    this.hidden = true;
    this.icon = 'graph2';
    this.name = "entropy ACGT";
    this.options = {
        width : viewer.width?viewer.width:700,
        height : 220,
        changeHeight: false,
        vAxis : {
            title : 'Entropy for Bases',
            minValue : 0
        },
        lineWidth : 1,
        smoothLine : true,
        focusTarget : 'category',
        hAxis : {},
        legend : 'none',
        chartArea : {
            top : 0,
            left : 70,
            width : '90%',
            height : '70%'
        },
        colors : vjPAGE.colorsACGT
    };
    this.series = [ {
        name : 'Position',
        label : false
    }, {
        name : 'Entropy A'
    }, {
        name : 'Entropy C'
    }, {
        name : 'Entropy G'
    }, {
        name : 'Entropy T'
    } ];
    this.type = 'line';
    vjGoogleGraphView.call(this, viewer);
}


function vjProfilerPanelSNPcallsView(viewer) {
    var snpCall = [
            {
                name : 'cutOffCall',
                type : 'text',
                forceUpdate : true,
                size : '15',
                isSubmitable : true,
                prefix : 'Threshold',
                align : 'left',
                order : '1',
                description : 'Jump to specific genomic position',
                title : '0.05'
            },
            {
                name : 'isORF',
                type : 'checkbox',
                path : '/Annotations/isORF',
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'Translates mutations into AA mutations if the reference is an ORF',
                title : 'ORF'
            },
            {
                name : 'codonScale',
                type : 'checkbox',
                path : '/Annotations/codonScale',
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'Scale reported frequencies based on codons',
                title : 'scale frequencies on codons'
            },
            {
                name : 'consensusAAMode',
                type : 'checkbox',
                path : '/Annotations/consensusAAMode',
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'Use consensus codon in AA mutations',
                title : 'consensusAA'
            },
            {
                name : 'AAcode',
                type : 'select',
                path : '/Annotations/AAcode',
                options: "single,Single letter\ntriple,Triple letter\nfull,Full name",
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'representation of amino acids',
                title : 'AA code'
            },
            {
                name : 'Files',
                type : 'select',
                addMoreRowsToOptionsMenuFromData:true,
                data : {
                    source : viewer.annotFileData,
                    name : "id",
                    title : "name",
                },
                isSubmitable : true,
                align : 'left',
                order : '3',
                title : 'Annotations',
                path : '/Annotations/Files',
                value: 0

            },
            {
                name : 'nsSNV',
                type : 'checkbox',
                path : '/Annotations/nsSNV',
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'show only mutations that lead to nsSNV',
                title : 'non-synonymous'
            },
            {
                name : 'jumpToMapper',
                align : 'right',
                order : '0',
                description : 'Jump to Annotation Mapper',
                url: annotMapperLink,
                title : 'Annotation-Mapper'
            },
            {
                name : 'hideSubjectName',
                title : 'hide reference name',
                path : '/Annotations/hideSubjectName',
                type : 'checkbox',
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'Prints the name of the file istead of the name of the reference'
            }
            ,{
                name : 'rsID',
                title : 'show rsID',
                path : '/Annotations/rsID',
                type : 'checkbox',
                isSubmitable : true,
                align : 'right',
                order : '0',
                description : 'Prints rsID the reference'
            }
            ];

    this.icons = {
        expand : null
    };

    vjSubmitPanelView.call(this, viewer);

    this.rows[2].order = Number.MAX_VALUE;
    this.rows = this.rows.concat(snpCall);
}

function annotMapperLink(){

    var iid = docLocValue("id");
    var pfx = location.origin + location.pathname + "?cmd=annotationMapper";

    var thrshd = this.tree.findByName("cutOffCall").value;
    if (!thrshd) thrshd = "0.05";

    var myURL = pfx + "&profiler=" + iid + "&dbSNP=1"+"&threshold="+thrshd;
    window.open(myURL);
}

function vjProfilerPanelSNPcallsForORFView(viewer) {
    var snpCall = [
            {
                name : 'cutOffCall',
                type : 'text',
                forceUpdate : true,
                size : '1',
                isSubmitable : true,
                prefix : 'threshold',
                align : 'left',
                order : '1',
                description : 'Jump to specific genomic position',
                title : '0.05'
            },
            {
                name : 'Files',
                type : 'checkbox',
                data : {
                    source : viewer.annotFileData,
                    name : "id",
                    title : "name",
                    description : "name",
                    path : "/Annotations/Files/"
                },
                parentDependant:true,
                isSubmitable : true,
                align : 'left',
                order : '0',
                title : 'Annotations',
                path : '/Annotations/Files'
            }];

    this.icons = {
        expand : null
    };

    vjBasicPanelView.call(this, viewer);

    this.rows[2].order = Number.MAX_VALUE;
    this.rows = this.rows.concat(snpCall);
}


function vjProfilerSNPcallsView(viewer) {
    this.maxTxtLen = 20;
    this.defaultEmptyText = 'no information to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.subInnerHTML = "<div style='text-align: center; background-color: #f4f4f4;'><p style='color:#b2b2b2; font-weight:600; font-size: 20px; padding: 50px 0px'>No data returned for SNP Calls</p></div>";
    vjTableView.call(this, viewer);
}

function vjProfilerAnnotationPanelView(viewer) {
    var anotCall = [  {
        name : 'position',
        type : 'text',
        isSubmitable : true,
        align : 'left',
        order : '2',
        prefix : 'Position',
        description : 'Jump to the position',
        title : "1"
    },{
        name : 'objIDs',
        type : 'select',
        addMoreRowsToOptionsMenuFromData:true,
        defaultOptionsValue:{"name":"none","title":"none"},
        data: {
            source : viewer.annotFileData,
            name : "id",
            title: "name"
        },
        isSubmitable : true,
        order : '0',
        title : 'Annotations',
        path : '/Annotations/objIDs',
        value : 0
    },
    {
        name : 'Show',
        type : 'select',
        options:[["0","All","All"],["CDS","CDS","show CDS"],["mat_peptide","mat_peptide","show mat_peptide"]],
        isSubmitable:true,
        path : '/Annotations/Show',
        align : 'right',
        order : '1',
        description : 'show',
        title : '',
        value: 0
    }
    ];

    vjBasicPanelView.call(this, viewer);
    this.rows[2].order = Number.MAX_VALUE;
    this.rows = this.rows.concat(anotCall);

}

function vjProfilerORFView(viewer) {
    this.fullurlproteinid = function(vv, table, irow, icol) {
        var url = "http:
                + table["protein_id"].replace(/\s+/g, '');
        window.open(url, "_blank");
    };

    this.fullurllocus = function(vv, table, irow, icol) {
        var url = "http:
                + table["locus_tag"].replace(/\s+/g, '');
        window.open(url, "_blank");
    };
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.appendCols=[ {header: {name: 'Link'}, cell: ''} ];
    this.cols = [ {
        name : "protein_id"
    }, {
        name : "rangeStart"
    }, {
        name : "rangeEnd"
    }, {
        name : "product"
    }, {
        name : "locus_tag",
        url : this.fullurllocus
    } , {
        name: 'Link'
        ,icon: 'img/ncbi.gif'
        ,type: 'icon'
        ,order: 1
        ,url: this.fullurlproteinid
    }
    ];


    this.maxTxtLen = 40;
    this.iconSize=24;
    vjTableView.call(this, viewer);
}

function vjProfilerSNPCallGraphSVGView(viewer) {

    var snpSerie =new vjDataSeries({
        name: viewer.data,
        title:"Computing SNP table",
        url:"static:
        columnDefinition:{x:"Position",y:"Frequency"},
        type: "column",
        valueMax:{x:"Length",y:1},
        byX : true,
        labels:["ProteinId","AA Ref","AA Sub","Position"],
        tag: ["start","end"],
        uniq:Math.random(),
        isok: true
      });

    var plotSNP=new vjSVG_Plot();
        plotSNP.add(snpSerie);

    this.chartArea = {
        width : "80%",
        height : "80%"
    };
    this.plots=[ plotSNP ];
    this.Axis = {
        x : {
            title : "SNP Positions",
            showGrid : true,
            textTickSize : "10px",
            showArrowHead : false
        },
        y : {
            title : "Frequency",
            showGrid : true,
            isHidden : false,
            showArrowHead : false
        }
    };
    this.hideOnEmptyData = true;
    vjSVGView.call(this, viewer);
}

function vjProfilerGraphORFSVGView(viewer) {


    var serie1=new vjDataSeries({
        title:"ORF",
        name:viewer.data,
        columnDefinition:{end:'rangeEnd',start:'rangeStart',label:"protein_id"},
        type:'anotBox',
        boxSize: 0.20,
        color: "salmon",
        valueMax:{x:"Length"},
        labels:["protein_id","locus_tag","product","rangeStart","rangeEnd"],
        tag:["Length"],
        isok:true}, viewer.wantToRegister);

    var plot1=new vjSVG_Plot();

    plot1.add(serie1);

    this.chartArea = {
            width : "80%",
            height : "80%"
        };

    this.plots= [ plot1 ];

    this.Axis = {
        x : {
            title : "ORF Annotation",
            showGrid : false,
            textTickSize : "10px"
        },
        y : {
            title : "Locus",
            showGrid : false,
            isHidden : true
        }
    },
    this.hideOnEmptyData = true;
    vjSVGView.call(this, viewer);
}





function vjProfilerZoomSNPProfilePanelView(viewer)
{
    var defaultRows = [
        {
            name: "showGraphsZoom",
            title: "Show Graphs",
            icon: "folderMinus",
            align: "left",
            order: "10",
            description: "Choose which graphs will be shown",
            path: "/showGraphsZoom"
        },
        {
            name: "coverageZoom",
            title: "Coverage Histogram",
            align: "left",
            order: "2",
            value: 1,
            type: "checkbox",
            path: "/showGraphsZoom/coverageZoom"
        },
        {
            name: "disbalanceZoom",
            title: "Profile Disbalance",
            align: "left",
            order: "3",
            value: 1,
            type: "checkbox",
            path: "/showGraphsZoom/disbalanceZoom"
        },
        {
            name: "entropyZoom",
            title: "Profile Entropy",
            align: "left",
            order: "4",
            value: 0,
            type: "checkbox",
            path: "/showGraphsZoom/entropyZoom"
        },
        {
            name: "qualityZoom",
            title: "Profile Quality",
            align: "left",
            order: "5",
            value: 0,
            type: "checkbox",
            path: "/showGraphsZoom/qualityZoom"
        },
        {
            name: "snpZoom",
            title: "SNP",
            align: "left",
            order: "6",
            value: 0,
            type: "checkbox",
            path: "/showGraphsZoom/snpZoom"
        },
        {
            name: "in.delZoom",
            title: "Insertions/Deletions",
            align: "left",
            order: "7",
            value: 0,
            type: "checkbox",
            path: "/showGraphsZoom/in.delZoom"
        },
        {
            name: "annotationZoom",
            title: "Profile Annotations",
            align: "left",
            order: "8",
            value: 0,
            type: "checkbox",
            path: "/showGraphsZoom/annotationZoom"
        },
        {
            name: "updateZoom",
            title: "Update",
            align: "left",
            order: "9",
            type: "button",
            path: "/showGraphsZoom/updateZoom",
            onClickFunc: this.onUpdateSNPGraphsZoom,
            url: "javascript:vjObjEvent(\"onUpdateSNPGraphsZoom\", \"" + this.objCls+ "\");"
        }
    ];
    
    var rows= [
                {name:'refresh', title: 'Refresh' ,order:'1', icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,url:viewer.urlrefresh?viewer.urlrefresh:''},
                {name: 'alHigh', title: ' ', value: ' ', type: 'text', size: '10', prefix: 'Position', isSubmitable:true, align: 'left', description: 'Jump to specific genomic position', order: '0' }
            ];
    viewer.iconSize=24;
    viewer.hideViewerToggle=true;
    vjPanelView.call(this, viewer);
    this.rows=rows.concat(defaultRows);
    this.selectedCounter=0;
    this.graphResolutionZoom=200;
    this.urlExchangeParameter = function(url, parname, newvalue) {
        if (parname == "alHigh") {
            newvalue = parseInt(newvalue);
            var tqs;
            try {
                tqs = JSON.parse(docLocValue("tqs", "[]", url));
            } catch (e) { }
            if (!tqs) {
                tqs = [];
            }
            var iop_slice = -1;
            for (var i=0; i<tqs.length; i++) {
                if (tqs[i].start_end_slice) {
                    iop_slice = i;
                    break;
                }
            }
            var op_slice = {
                op: "slice",
                start_end_slice: true,
                arg: {
                    start: {
                        col: { "name" : "Position" },
                        value: Math.ceil(newvalue - this.graphResolutionZoom / 2) - 1,
                        method: "firstGreaterThan"
                    },
                    end: {
                        col: { "name" : "Position" },
                        value: Math.floor(newvalue + this.graphResolutionZoom / 2) + 1,
                        method: "lastLessThan"
                    }
                }
            };
            if (iop_slice >= 0) {
                if (tqs[iop_slice].op == "addMissingRows") {
                    tqs[iop_slice].arg.abscissa.minValue = Math.max(1, Math.floor(newvalue - this.graphResolutionZoom / 2));
                    var referenceID;
                    tqs[iop_slice].arg.abscissa.maxValue = { formula: "min(input_obj.refSeqLen(saved_referenceID)," + Math.ceil(newvalue + this.graphResolutionZoom / 2) + ")" };
                } else {
                    tqs[iop_slice] = op_slice;
                }
            } else {
                tqs.push(op_slice);
            }
            url = urlExchangeParameter(url, "tqs", vjDS.escapeQueryLanguage(JSON.stringify(tqs)));
        }
        return urlExchangeParameter(url, parname, newvalue);
    };
    
    this.onUpdateSNPGraphsZoom = function (container, node, menuViewer)
    {
        var children = menuViewer.tree.findByName("showGraphs").children;
        var parentChildren = $("#"+menuViewer.parent.divName).children();
        
        for (var i = 0; i < children.length; i++)
        {
            var tmp = children[i];
            
            if (tmp.value && (parseInt(tmp.value) == 1 || tmp.value == true))
                $(parentChildren[i+1]).removeClass("hidden");
            else
                $(parentChildren[i+1]).addClass("hidden");
        }
    }
};

function vjProfilerZoomSNPCoverageView(viewer) {
    this.name= "coverage";
    this.icon= 'graph';
    this.series= [{ name: 'Position' }, { name: 'Count Forward' }, { name: 'Count Reverse'}],
    this.options= {
        width: viewer.width?viewer.width:700, height: '25%',
        changeHeight: false,
        vAxis: { title: 'Coverage' }, hAxis: {},
        lineWidth: 1, isStacked: true,
        legend: 'top',
        focusTarget: 'category',
        chartArea: { top: 20, left: 70, width: '90%', height: "70%" }, colors: ['blue', 'green']
    },
    this.type= 'column',


    vjGoogleGraphView.call(this, viewer);

}
function vjProfilerZoomSNPProfileSNPView(viewer) {
    this.icon = 'hiveseq-cube';
    this.name = "SNP";
    this.options = {
            width: viewer.width?viewer.width:700, height: '25%',
            vAxis: { title: 'SNP %' },
            changeHeight: false,
            lineWidth: 2,
            isStacked: true,
            focusTarget: 'category',
            hAxis: {}, legend: 'bottom',
            chartArea: { top: 20, left: 70, height: '70%', width: '90%' },
            colors: vjPAGE.colorsACGT
    };
    this.series = [{ label: true, name: 'Position', eval:'row["Position"]+" " + row["Letter"]+(row["Consensus"]!=row["Letter"] ? "/"+row["Consensus"] : "")' }, { name: 'Frequency A' }, { name: 'Frequency C' }, { name: 'Frequency G' }, { name: 'Frequency T'}];
    this.type = 'column';
    vjGoogleGraphView.call(this, viewer);
}


function vjProfilerZoomSNPProfilerACGTQualityView(viewer)
{
    this.hidden = true;
    this.name = "quality ACGT";
    this.icon = 'hiveseq-cube';
    this.options = {
        width : viewer.width?viewer.width:700,
        height : 220,
        changeHeight: false,
        focusTarget: 'category',
        vAxis : {
            title : 'Quality',
            minValue : 0
        },
        lineWidth : 1,
        hAxis : {},
        legend : 'none',
        areaOpacity : .8,
        chartArea : {
            top : 20,
            left : 70,
            width : '90%',
            height : '70%'
        },
        colors : ['#006AFF','#95BE4D','#556B2F', '#000080'],
    };
    this.series = [ {
        name : 'Position'
    }, {
        name : 'Quality Forward A'
    }, {
        name : 'Quality Forward C'
    }, {
        name : 'Quality Forward G'
    }, {
        name : 'Quality Forward T'
    }, {
        name : 'Quality Reverse A'
    }, {
        name : 'Quality Reverse C'
    }, {
        name : 'Quality Reverse G'
    }, {
        name : 'Quality Reverse T'
    } ];
    this.type = 'line';
    vjGoogleGraphView.call(this, viewer);
}

function vjFreqHistPopUp(viewer){

        this.name= "coverage";
        this.icon= 'graph';
        this.series= [{ name: 'Coverage' }, { name: 'CountA' },{ name: 'CountT' },{ name: 'CountG' },{ name: 'CountC' }],
        this.options= {
            width: viewer.width?viewer.width:700, height: '100%',
            vAxis: { title: 'Count of positions for SNP frequency' }, hAxis: {title:'Coverage'},
            lineWidth: 1, isStacked: true,
            legend: 'bottom',
            chartArea: { top: 10, left: 80, width: '80%', height: "70%" },
        };
        this.type= 'column';


        vjGoogleGraphView.call(this, viewer);
}

function vjProfilerSNPCallGraphZoomSVGView(viewer) {

    var snpSerie =new vjDataSeries({
        name: viewer.data,
        title: "Zooming to CDS Table",
        url:"static:
        columnDefinition:{x:"Position",y:"Frequency"},
        type: "column",
        isNXminBased: true,
        byX: true,
        labels:["ProteinId","AA Ref","AA Sub","Position"],
        isok: true
      });

    var plotSNP=new vjSVG_Plot();
        plotSNP.add(snpSerie);

    this.chartArea = {
        width : "80%",
        height : "80%"
    };
    this.plots=[ plotSNP ];
    this.Axis = {
        x : {
            title : "SNP Positions",
            showGrid : false,
            textTickSize : "10px"
        },
        y : {
            title : "Frequency",
            showGrid : false,
            isHidden : false
        }
    };
    this.hideOnEmptyData = true;
    vjSVGView.call(this, viewer);
}

function vjProfilerGraphORFZoomSVGView(viewer) {


    var serie1=new vjDataSeries({
        title:"ORFZoom",
        name:viewer.data,
        url: "static:
        columnDefinition:{end:'rangeEnd',start:'rangeStart',label:"protein_id"},
        type:'anotBox',
        boxSize: 0.20,
        color: "salmon",
        labels:["protein_id","locus_tag","product","rangeStart","rangeEnd"],
        isNXminBased: true,
        isok:true});

    var plot1=new vjSVG_Plot();

    plot1.add(serie1);

    this.chartArea = {
            width : "80%",
            height : "80%"
        };

    this.plots= [ plot1 ];

    this.Axis = {
        x : {
            title : "ORF Annotation",
            showGrid : false,
            textTickSize : "12px"
        },
        y : {
            title : "Locus",
            showGrid : false,
            isHidden : true
        }
    },
    vjSVGView.call(this, viewer);
}



function vjProfilerZoomPanelAnnotView(viewer) {
    var orfZoom = [ {
        name : 'back',
        icon : 'back',
        size : '1',
        align : 'left',
        description : 'ZOOM BACK',
        url : viewer.panelurl
    } ];
    vjBasicPanelView.call(this, viewer);
    this.rows = orfZoom;
}







function vjProfilerSNPProfileControl(viewer)
{

    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }
    this.formObject=document.forms[this.formName];

    this.menuViewer=new vjProfilerSNPProfileMenu({
        data: [ "dsVoid", this.data.profile],
        width: this.width,
        parent: this,
        formObject: this.formObject
    });
    this.coverageViewer=new vjProfilerSNPProfileCoverage({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.coverageHistogramViewer=new vjProfilerCoverageHistogramView({
        data: this.data.profileX,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.disbalanceViewer=new vjProfilerSNPProfileDisbalanceView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.entropyViewer=new vjProfilerSNPProfileEntropyView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.qualityViewer=new vjProfilerSNPProfileQualityView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.snpViewer=new vjProfilerSNPProfileSNPView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.indelsViewer=new vjProfilerSNPProfileSNPInDelsView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.annotationViewer=new vjProfilerSNPProfileAnnotationView({
        data: this.data['annotationData'],
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.anotSelectCallback,
        subInnerHTML: "<div style='text-align: center; background-color: #f4f4f4;'><p style='color:#b2b2b2; font-weight:600; font-size: 20px; padding: 50px 0px'>Profile Annotations Graph: No data returned please select another type and/or another Annotation File</p></div>"
    });
    this.allViewers = [ this.menuViewer, this.coverageViewer, this.coverageHistogramViewer,
                        this.disbalanceViewer, this.entropyViewer, this.qualityViewer,
                        this.snpViewer, this.indelsViewer, this.annotationViewer];

    return this.allViewers;
}

function vjProfilerAnnotationListControlView(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    
    var that = this;

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerConsensusControl");
    }
    if(this.formName==undefined){
        this.formName='form-profiler';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerConsensusControl");
    }

    this.formObject=document.forms[this.formName];

    this.panelViewer = new vjProfilerAnnotationListPanel({
        data : [ "dsVoid", this.data ],
        callbackForPanel: this.callbackForPanel,
        formObject : this.formObject
    });
    
    this.tableViewer = new vjProfilerAnnotationListView({
        data : this.data,
        formName : this.formName,
        callbackRendered : this.callbackRendered,

        selectCallback : (function() {
            that.onSelectIonAnnotList.apply(that, arguments);
        }),
        width : this.width
    });

    vjDS.add("loading Annot Types","dsAnnotTypes","static:
    vjDS["dsAnnotTypes"].refreshOnEmpty = true;
    vjDS["dsAnnotTypes"].parser = function (ds,text){
        text = text ? text : "";
        var panelV = that.panelViewer;
        var nodeList = [];
        for (var i=0; i<panelV.rows.length; ++i){
            var n=panelV.rows[i];
            if (n.path == undefined || n.path.indexOf("/types/")==-1){
                nodeList.push(n);
            }
        }
        panelV.rows = nodeList;
        if (text.length){
            var textSplit = text.split("\n");
            for (var iN=0; iN<textSplit.length; ++iN){
                var name = textSplit[iN];
                if (!name || !name.length) continue;
                if (name.indexOf("....")!=-1) continue;
                var nodeName = name.replace(/\s+/g,"_");
                var myNode = {name: nodeName, title: name, align: "left", value:0, description: name, type: "checkbox", path: "/types/" + nodeName};
                panelV.rows.push(myNode);
                if (name.toLowerCase().indexOf("features")!=-1){
                    panelV.rows.push({name: "CDS", title: "CDS", align: "left", value:0, description: "CDS", type: "checkbox", path: "/types/" + "CDS"});
                    panelV.rows.push({name: "misc_feature", title: "misc_feature", align: "left", value:0, description: "misc_feature", type: "checkbox", path: "/types/" + "misc_feature"});
                    panelV.rows.push({name: "mat_peptide", title: "mat_peptide", align: "left", value:0, description: "mat_peptide", type: "checkbox", path: "/types/" + "mat_peptide"});
                }
            }
        }else{
            panelV.rows.push({
                name : 'nothing',
                title:'No types in this file',
                align : 'left',
                description : 'select an annotation file',
                order: 0,
                value:0,
                path:'/types/nothing'
            })
        }
        panelV.render();
        return text;
    }
    
    this.onSelectIonAnnotList = function (viewer, node, ir, ic){
        var nodeIdSelected = [];
        for (var i=0; i<viewer.selectedNodes.length; ++i){
            nodeIdSelected.push(viewer.selectedNodes[i].id);
        }

        vjDS["dsAnnotTypes"].reload("http:
        vjDS["dsAnnotTypes"].viewer=viewer;
        
    };
    
    return [this.panelViewer,this.tableViewer];
}

function vjProfilerConsensusControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerConsensusControl");
    }
    if(this.formName==undefined){
        this.formName='form-profiler';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerConsensusControl");
    }

    this.formObject=document.forms[this.formName];

    this.menuViewer = new vjProfilerConsensusMenu({
        data: [ "dsVoid", this.data ],
        width: this.width,
        formObject: this.formObject
    });
    this.consensusViewer = new vjProfilerConsensusView( {
        data : this.data,
        width:this.width,
        formObject : this.formObject
    });

    return [this.menuViewer,this.consensusViewer];
}

function vjProfilerNoiseControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-profiler';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }
    this.formObject=document.forms[this.formName];

    this.noiseViewer=new vjProfilerNoiseView({
        data: this.data.noise,
        width:this.width,
        formObject: this.formObject
    });
    this.noise_integralViewer=new vjProfilerNoiseIntegralView({
        data: this.data.integral,
        width:this.width,
        formObject: this.formObject
    });
    return [ this.noiseViewer, this.noise_integralViewer];
}


function vjProfilerFrequencyHistogramControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }
    this.formObject=document.forms[this.formName];

    this.frequency_histogramViewer=new vjProfilerFrequencyHistogramView({
        data: this.data.histogram,
        width:this.width,
        formObject: this.formObject,
        selectCallback: this.selectCallback
    });
    this.frequency_histogram_integralViewer=new vjProfilerFrequencyHistogramIntegralView({
        data: this.data.integral,
        width:this.width,
        formObject: this.formObject
    });
    return [ this.frequency_histogramViewer, this.frequency_histogram_integralViewer];
}



function vjProfilerSNPcallsControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    if(this.data===undefined){
        console.warn("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        console.warn("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }
    this.formObject=document.forms[this.formName];

    this.panel_snpcallsViewer=new vjProfilerPanelSNPcallsView({
        data: ["dsVoid",this.data, 'annot_files'],
        annotFileData:this.annotFileData,
        width:this.width,
        formObject: this.formObject,
    });

    this.snpcallsViewer=new vjProfilerSNPcallsView({
        data: this.data,
        width:this.width,
        formObject: this.formObject
    });

    return [ this.panel_snpcallsViewer, this.snpcallsViewer];
}

function vjProfilerAnnotationControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }

    this.formObject=document.forms[this.formName];

    this.panel_snpcallsForORFViewer=new vjProfilerPanelSNPcallsForORFView({
        data: ["dsVoid",this.data],
        annotFileData:this.annotFileData,
        width:this.width,
        formObject: this.formObject
    });

    this.orf_panelViewer=new vjProfilerAnnotationPanelView({
        data: ["dsVoid",this.data.orf],
        width:this.width,
        annotFileData:this.annotFileData,
        options:this.options,
        formObject: this.formObject
    });

    this.orf_tableViewer=new vjProfilerORFView({
        data: this.data.orf,
        width:this.width,
        formObject: this.formObject
    });

    this.snpcallsViewer=new vjProfilerSNPCallGraphSVGView({
        data: this.data.snpcalls,
        width:this.width,
        formObject: this.formObject,
        selectCallback: this.selectCallback.snpcalls
    });

    this.orf_graphViewer=new vjProfilerGraphORFSVGView({
        data: this.data.orf,
        width:this.width,
        formObject: this.formObject,
        wantToRegister: this.wantToRegister,
        selectCallback: this.selectCallback.orf
    });

    return [ this.orf_panelViewer, this.orf_tableViewer, this.snpcallsViewer, this.orf_graphViewer];
}



function vjProfilerZoomAnnotationControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }

    this.formObject=document.forms[this.formName];

    this.orf_panelViewer=new vjProfilerZoomPanelAnnotView({
        data: ["dsVoid",this.data.orf],
        panelurl:this.panelurl,
        width:this.width,
        parentVIS:this.parentVIS,
        formObject: this.formObject
    });

    this.snpcallsViewer=new vjProfilerSNPCallGraphZoomSVGView({
        data: this.data.snpcalls,
        width:this.width,
        formObject: this.formObject,
        selectCallback: this.selectCallback.snpcalls
    });

    this.orf_graphViewer=new vjProfilerGraphORFZoomSVGView({
        data: this.data.orf,
        width:this.width,
        formObject: this.formObject,
        selectCallback: this.selectCallback.orf
    });

    return [ this.orf_panelViewer, this.snpcallsViewer, this.orf_graphViewer];
}


function vjProfilerZoomSNPProfileControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjProfilerSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjProfilerSNPProfileControl");
    }
    
    this.formObject=document.forms[this.formName];
    if(this.formObject === undefined){
       let newform = document.createElement("form")
        newform.setAttribute("id", this.formName);
        document.body.appendChild(newform);
    }
    this.formObject=document.forms[this.formName];

    this.panelView=new vjProfilerZoomSNPProfilePanelView({
        data: ["dsVoid",this.data.profile],
        width:this.width,
        parentVIS:this.parentVIS,
        parent: this,
        formObject: this.formObject,
        urlrefresh : this.urlrefresh
    });
    this.panelView.callerObject = { onClickMenuNode: function (container, node, menuViewer)
    {
        var children = menuViewer.tree.findByName("showGraphsZoom").children;
        var parentChildren = $("#"+menuViewer.parent.divName).children();
        
        for (var i = 0; i < children.length; i++)
        {
            var tmp = children[i];
            menuViewer.rows[3+i].value = tmp.value; 
            
            if (tmp.value && (parseInt(tmp.value) == 1 || tmp.value == true))
                $(parentChildren[i+1]).removeClass("hidden");
            else
                $(parentChildren[i+1]).addClass("hidden");
        }
    }};

    this.coverageViewer=new vjProfilerZoomSNPCoverageView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.disbalanceViewer=new vjProfilerSNPProfileDisbalanceView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.entropyViewer=new vjProfilerSNPProfileEntropyView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.qualityViewer=new vjProfilerSNPProfileQualityView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });

    this.snpViewer=new vjProfilerZoomSNPProfileSNPView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.indelsViewer=new vjProfilerSNPProfileSNPInDelsView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.entropy_acgtViewer=new vjProfilerEntropiesACGTView({
        data: this.data.profile,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });


    this.allViewers = [ this.panelView, this.coverageViewer,
                        this.disbalanceViewer, this.entropyViewer, this.qualityViewer,
                        this.snpViewer, this.indelsViewer,
                        this.entropy_acgtViewer ];

    return this.allViewers;
}

