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

javaScriptEngine.include("js/vjSVGView.js");

javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js/vjTreeSeries.js");

javaScriptEngine.include("js-graph/vjSVG_HeatMap.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");

$.loadCSS('d3js/css/venn_diagram.css');

vjHO.register('svc-alignment-comparator').Constructor=function ()
{        
    var latestReqID = -1;
    var idCol = 0;
    
    var tblHits = new vjTableControlX2({
        data: "dsActivityHits",
        formObject:document.forms[formName],
        maxTxtLen:32,
        selectedCounter: 1000,
        callbackRendered: tableRendered
    });
    var tblRPKM = new vjTableControlX2({
        data: "dsActivityRPKM",
        selectedCounter: 1000,
        maxTxtLen:32,
        formObject:document.forms[formName],
        callbackRendered: tableRendered
    });
    
    var refMapTable = new vjTableControlX2({
        data: "dsRefMap",
        selectedCounter: 1000,
        maxTxtLen:32,
        onClickCellCallback: combTableSelected,
        formObject:document.forms[formName]
    });

    var combMapTable = new vjTableControlX2({
        data: "dsCombMap",
        selectedCounter: 1000,
        maxTxtLen:32,
        onClickCellCallback: combTableSelected,
        formObject:document.forms[formName]
    });
    
    var myHeatmapPlot = new vjSVG_HeatMap({
        color: {min: "blue", max: "#ffc200", mid:"white"},
        heat_image_url: "http:
        heat_image_min_cells: 10000
    });
    
    myHeatmapPlot.add(new vjDataSeries({
        url: "static:
        name: "dsHeatmapMap",
        id: "heat",
        type: "raw",
        minCellSize: 50,
        isok: true
    }));
    var leftSeries = new vjTreeSeries({
        url: "static:
        name: "dsHeatmapLeft",
        id: "left",
        isok: true
    });
    var topSeries = new vjTreeSeries({
        url: "static:
        name: "dsHeatmapTop",
        id: "top",
        isok: true
    });
    myHeatmapPlot.add(leftSeries);
    myHeatmapPlot.add(topSeries);
    
    var myHeatmapViewer = new vjSVGView({
        chartArea: {width: "95%", height: "95%"},
        plots: [myHeatmapPlot]
    });
    var parCoordsViewer = new vjD3JS_ParallelCoordAC_Control({
        data: 'dsGraphDS',
        dataCol: 2,
        rangeCol: 2,
        labelCol:idCol,
        colSet:[{colIndex:1, strings:true},{colIndex:2, strings:true},{colIndex:3, strings:true}],
        weightCol: {name:"sequence", splitBy:"rpt=", indexBy:1},
        colors: ["blue","yellow"],
        csvTbl: true,
        accumulateWeight: true, 
        selectedCallback: parCoordsSelectFunc
    });
    
    vennDiagram_viewer =  new vjD3JS_vennDiagram({
        data: 'dsForVenn',
        csvTbl : true,
        threshold: 1,
        toSearch:"",
        toExclude: false 
    });
    
     var vennDiagram_panel = new vjPanelView({
         data: ['dsVoid', 'dsForVenn'],
         formObject: document.forms[formName],
         menuUpdateCallback: function(panel, newurl) {
             modifyThresholdAndRefreshViewer(panel);
             return false;
         },
         prevSearch:"",
         iconSize: 24,
         rows:[{
                    name : 'threshold',
                    type : 'text',
                    forceUpdate : true,
                    size : '6',
                    isSubmitable : true,
                    prefix : 'Threshold',
                    align : 'right',
                    order : '3',
                    description : 'filter hits',
                    title : '1'
                },{
                    name : 'toSearch',
                    path:"/toSearch/",
                    type : 'text',
                    forceUpdate : true,
                    size : '10',
                    isSubmitable : true,
                    prefix : 'Search Term',
                    align : 'right',
                    order : '2',
                    description : 'what to search in the first column'
                },{
                    name : 'toExclude',
                    path:"/toSearch/toExclude",
                    type : 'checkbox',
                    isSubmitable : true,
                    description : 'exclude',
                    title:"exclude the search term",
                    value:false
                },{
                    name : 'useCategory',
                    type : 'checkbox',
                    isSubmitable : true,
                    value:false,
                    order:"-1",
                    align:"right",
                    description : 'Looking for category from the header',
                    title:"Use Category"
                },{
                    name : 'dataname',
                    type : 'select',
                    isSubmitable : true,
                    options:[["activity-Hits","Hits","Hits"],["activity-RPKM","RPKM","RPKM"]],
                    prefix : 'Table',
                    align : 'left',
                    order : '1',
                    description : 'table to display',
                    title:"Table ",
                    value:"activity-Hits"
                },{
                    name : 'submit',
                    forceUpdate : true,
                    icon:'search',
                    size : '12',
                    isSubmitable : true,
                    prefix : 'Submit',
                    align : 'right',
                    order : '3',
                    description : 'submit',
                    title : 'submit',
                    url: modifyThresholdAndRefreshViewer
                    }                                
                   ]
         ,isok: true
     });
     
     var chord_viewer=new vjD3JS_CorrelationCord({
            data: 'dsCombMap'
            ,pairs: {array: "#COLS[1:-2]"}
             ,pairValue: ["#COL[-1]"]
             ,oneColorPerCateg: true
             ,sortLables: false
            ,formObject:document.forms[formName]
    });
    
    
    this.fullview=function(node, whereToAdd)
    {
        var tqs = [{
                        op: "heatmap",
                        arg: {rowSet:"0-100", uid:"0"}
                   }];
        var id = docLocValue("id");
        vjDS.add("", "dsActivityHitsTmp", "qpbg_tblqryx4:
        vjDS.add("", "dsActivityHits", "static:
        vjDS.add("", "dsActivityRPKM", "qpbg_tblqryx4:
        vjDS.add("", "dsHeat", "qpbg_tblqryx4:
        vjDS.add("", "dsRefMap", "qpbg_tblqryx4:
        vjDS.add("", "dsCombMap", "qpbg_tblqryx4:
        vjDS.add("", "dsGraphDS", "qpbg_tblqryx4:
        vjDS.add("", "dsHeatmapMap","static:
        vjDS.add("", "dsHeatmapTop","static:
        vjDS.add("", "dsHeatmapLeft","static:
        

        
        vjDS["dsGraphDS"].register_callback(function(viewer, data){
            var tbl = new vjTable(data);
            
            parCoordsViewer[1].colSet = [];
            for (var i = 1; tbl.rows[0] && i < tbl.rows[0].cols.length-1; i++){
                parCoordsViewer[1].colSet.push ({colIndex:i, strings:true});
            }
            parCoordsViewer[1].refresh();
        });
        vjDS["dsActivityHitsTmp"].load();
        vjDS["dsActivityHitsTmp"].register_callback(function (viewer, data){
            var tbl = new vjTable(data);
            
            tqs = [{
                op: "heatmap",
                arg: {rowSet:"0-"+(tbl.rows.length-2), uid:"0"}
            }];
            vjDS["dsActivityHits"].reload("qpbg_tblqryx4:
        });
        

        vjDS.add("", "dsForVenn", "qpbg_tblqryx4:
        vjDS["dsActivityHits"].register_callback(tableRendered);
                
        var filesStructureToAdd = [{
                tabId: 'refMapTbl',
                tabName: "Reference Mapping Table",
                inactive: false,
                allowClose: true,
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    instance: refMapTable.arrayPanels
                },
                  autoOpen: ["computed"]
            },{
                tabId: 'hits',
                tabName: "Hits",
                inactive: true,
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    instance: tblHits.arrayPanels
                },
                  autoOpen: ["computed"]
            },
           {
                tabId: 'tabRPKM',
                tabName: "RPKM",
                inactive: true,
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    instance: tblRPKM.arrayPanels
                },
                autoOpen: ["computed"]
            },{
                tabId: 'combMapTbl',
                tabName: "Combination Mapping Table",
                inactive: true,
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    instance: combMapTable.arrayPanels
                },
                autoOpen: ["computed"]
            },{
                tabId: 'heatmap',
                tabName: "Heatmap",
                inactive: true,
                position: {posId: 'resultsGraph', top:'0%', bottom:'100%', left:'60%', right:'100%'},
                viewerConstructor: {
                    instance: myHeatmapViewer
                },
                  autoOpen: ["computed"]
            },{
                tabId: 'parCoords',
                tabName: "Parallel Coordinates",
                inactive: false,
                preload: true,
                position: {posId: 'resultsGraph', top:'0%', bottom:'100%', left:'60%', right:'100%'},
                viewerConstructor: {
                    instance: parCoordsViewer
                },
                  autoOpen: ["computed"]
            },{
                tabId: 'vennDiagram',
                tabName: "Venn Diagram",
                inactive: true,
                position: {posId: 'resultsGraph', top:'0%', bottom:'100%', left:'60%', right:'100%'},
                viewerConstructor: {
                    instance: [vennDiagram_panel, vennDiagram_viewer]
                },
                  autoOpen: ["computed"]
            },{
                tabId: 'corChordDiagram',
                tabName: "Chord Diagram",
                inactive: true,
                position: {posId: 'resultsGraph', top:'0%', bottom:'100%', left:'60%', right:'100%'},
                viewerConstructor: {
                    instance: [chord_viewer]
                },
                  autoOpen: ["computed"]
            }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("basicHelp");
    };
    
    function tableRendered (viewer,text)
    {       
    
        if (latestReqID == -1)
            latestReqID = vjQP.req;

        vjDS["dsHeatmapMap"].reload("http:
        vjDS["dsHeatmapTop"].reload("http:
        vjDS["dsHeatmapLeft"].reload("http:
    };
    
    var oldTableIndex = -1;
    function parCoordsSelectFunc (viewer, labelsVisible)
    {       
        if (!labelsVisible) return;

        var idKey = refMapTable.tableViewer.tblArr.hdr[idCol].name;
        var rowsToColor = new Array();
        refMapTable.tableViewer.enumerate (function (view, tbl, ir){ 
            var node = tbl.rows[ir]; 
            
            for (var i = 0; i < labelsVisible.length; i++)
            {
                if (labelsVisible[i][idKey] == node[idKey])
                {
                    rowsToColor.push(ir);
                    break;
                }
                
            }
        }, refMapTable.tableViewer);
        
        if (oldTableIndex != -1)
        {
            for (var i = 0; i < oldTableIndex.length; i++)
                refMapTable.tableViewer.colorRow(oldTableIndex[i],0,"#ffe282");
        }
        
        for (var i = 0; i < rowsToColor.length; i++)
            refMapTable.tableViewer.colorRow(rowsToColor[i],0,"#ffe282");
        
        oldTableIndex = rowsToColor;
    }
    
    function combTableSelected(viewer, node )
    {
        var t="";
        for(var ih=0; ih<viewer.tblArr.hdr.length; ++ih ) {
            var cname=viewer.tblArr.hdr[ih].name;
            if(cname!="count" && cname!="sequence") {
                if(t.length)
                    t+=" && ";
                t+="node[\""+viewer.tblArr.hdr[ih].name+"\"]==\""+node[viewer.tblArr.hdr[ih].name]+"\""
            }
        }
        if(t.length) {
            t="if("+t+")node.selected=1;";
            if(!gKeyCtrl)t+=" else node.selected=0;";
            var vi= viewer.data =="dsRefMap" ? combMapTable : refMapTable;   
            vi.tableViewer.enumerate(t);
            vi.tableViewer.refresh();
        }
    }
    function modifyThresholdAndRefreshViewer(viewer,node,ir){
        var v=viewer.tree.findByName("threshold").value;
        vennDiagram_viewer.threshold=v;
        
        vennDiagram_viewer.useCategory= viewer.tree.findByName("useCategory").value ? true : false;
        
        var toExclude = viewer.tree.findByName("toExclude").value ? true : false;
        var toSearch= viewer.tree.findByName("toSearch").value ? viewer.tree.findByName("toSearch").value : "";
        
        var dataname=viewer.tree.findByName("dataname").value;        
        var url = vjDS["dsForVenn"].url;
        var requireBackend = (url.indexOf(dataname)==-1) ? 1 : 0 ;
        if (viewer.prevSearch != toSearch){
            requireBackend = 1;
            viewer.prevSearch = toSearch;
        }
        if (!requireBackend){
            vennDiagram_viewer.refresh();    
        }
        else {
            if (toSearch.length){
                var tqs = "[{\"op\":\"filter\",\"arg\":{\"col\":\"0\",\"method\":\"regex\",\"value\":\""+ toSearch+"\",\"negate\":\""+toExclude+"\"}}]";
                url = urlExchangeParameter(url, 'tqs', tqs);
                url = urlExchangeParameter(url, 'toSearch', toSearch);
                url = urlExchangeParameter(url, 'toExclude', toExclude);
            }
            else {
                url = urlExchangeParameter(url, 'tqs', "[]");
                url = urlExchangeParameter(url, 'toSearch', toSearch);
                url = urlExchangeParameter(url, 'toExclude', toExclude);
            }
            url = url.replace(/activity-[A-Z]*[a-z]*.csv/g,dataname+".csv");
            url = urlExchangeParameter(url, 'dataname', dataname);
            url = urlExchangeParameter(url, 'threshold', v);
            vjDS["dsForVenn"].reload(url,true);
        }
        
    }
};

