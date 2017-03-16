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
//javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");
javaScriptEngine.include("js-graph/vjSVG_HeatMap.js");

//javaScriptEngine.include("d3js/annotation_box.js");

vjHO.register('svc-differential-profiler').Constructor = function() {

    if(this.objCls)return;
    this.resolution=200;

    this.fullview=function(node, dv)
    {
        this.loadedID = docLocValue("id");
        
        // add Heatmap
        var myHeatmapPlot = new vjSVG_HeatMap({
            color: {min: "blue", max: "#ffc200", mid:"white"},
            valueRange: {min: 0, mid: undefined, max: 100},
            heat_image_url: "http://?cmd=-qpData&req="+this.loadedID+"&dname=heatmap.png",
            heat_image_min_cells: 10000
        });
        
        var heatmap_tqs = [{
              op: "insertcol",
              arg: {
                 col: 0,
                 name: "id",
                 formula: "cat($0,':', $1,'-',$2)"
              }
            },
            {
              op: "hidecol",
              arg: {
                 cols: [1,2,3]
              }
            }];
        var heatmap_tqs_string = vjDS.escapeQueryLanguage(JSON.stringify(heatmap_tqs));
        var heatMap_url = "qpbg_tblqryx4://coverage_diff.csv//objs="+this.loadedID+"&tqs=" + heatmap_tqs_string; 

        
        myHeatmapPlot.add(new vjDataSeries({
            url: heatMap_url,
            name: "dsHeatmapMap",
            id: "heat",
            type: "raw",
            minCellSize: 50,
            isok: true
        }));

        var myHeatmapViewer = new vjSVGView({
            chartArea: {width: "95%", height: "95%"},
            plots: [myHeatmapPlot]
        });

        var filesStructureToAdd = [ 
        {
            tabId: 'covmap',
            tabName: "Coverage Map",
            inactive: true,
            position: {posId: 'resultsGraph', top:'0%', bottom:'100%', left:'60%', right:'100%'},
            viewerConstructor: {
                instance: myHeatmapViewer
            },
              autoOpen: ["computed"]
        }        
        ];
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
//        algoWidgetObj.closeTab("downloadAllFiles");        
    }
    
    function onRenderQueryList(viewer,node,irow){
        if (viewer.tblArr && viewer.tblArr.rows.length) { 
            viewer.mimicClickCell(0,0);
        }
    }
    
}