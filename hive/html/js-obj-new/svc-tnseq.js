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
javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");
javaScriptEngine.include("js/vjTreeSeries.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js/vjTreeSeries.js");

javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js-graph/vjSVG_HeatMap.js");


vjHO.register('svc-tnseq').Constructor=function ()
{        
    var table = new vjTableControlX2({
        data: "dsTNseq",
        maxTxtLen:32,
        formObject:document.forms[formName]
    });
    
    var table_2 = new vjTableControlX2({
        data: "dsTNseqGeneInfo",
        maxTxtLen:32,
        formObject:document.forms[formName]
    });
    
    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsTNseq", "qpbg_tblqryx4://tnseq.csv//spcTypeObj=tblqryx2&cnt=100&raw=1&cols=0-200&objs="+id);
        vjDS.add("", "dsTNseqGeneInfo", "qpbg_tblqryx4://geneSpecInfo.csv//spcTypeObj=tblqryx2&cnt=100&raw=1&cols=0-200&objs="+id);
        
        // coverage
        vjDS.add("", "dsCovHeatmapData", "qpbg_tblqryx4://cov_heatMapData.csv//spcTypeObj=tblqryx2&cnt=100&raw=1&cols=0-200&objs="+id);
        vjDS.add("", "dsCovHeatmapMap", "qpbg_tblqryx4://cov_heatMap.csv//spcTypeObj=tblqryx2&cnt=100&raw=1&cols=0-200&objs="+id);
        
        vjDS.add("","dsCovHeatmapLeft","http://?cmd=objFile&ids="+id+"&filename=cov_horizontal.tre");
        vjDS.add("","dsCovHeatmapTop","http://?cmd=objFile&ids="+id+"&filename=cov_vertical.tre");
        
        // insert
        vjDS.add("", "dsInsHeatmapData", "qpbg_tblqryx4://ins_heatMapData.csv//spcTypeObj=tblqryx2&cnt=100&raw=1&cols=0-200&objs="+id);
        vjDS.add("", "dsInsHeatmapMap", "qpbg_tblqryx4://ins_heatMap.csv//spcTypeObj=tblqryx2&cnt=100&raw=1&cols=0-200&objs="+id);
        
        vjDS.add("","dsInsHeatmapLeft","http://?cmd=objFile&ids="+id+"&filename=ins_horizontal.tre");
        vjDS.add("","dsInsHeatmapTop","http://?cmd=objFile&ids="+id+"&filename=ins_vertical.tre");

        
        // var overall_link = "?cmd=objFile&ids=" + thisProcessID ;
        
        function createHeatMapPlot(heatMap_DS, leftTree_DS, topTree_DS) {
            
            var heatMapPlot = new vjSVG_HeatMap({
                color: {min: "blue", max: "#ffc200", mid:"white"},
                //heat_image_url: "http://?cmd=objFile&ids="+id+"&filename=heatmap.png",
                heat_image_min_cells: 10000
            });
            
            heatMapPlot.add(new vjDataSeries({
                url: vjDS[heatMap_DS].url,
                name: heatMap_DS,
                id: "heat",
                type: "raw",
                minCellSize: 50,
                isok: true
            }));
            var leftSeries = new vjTreeSeries({
                url: vjDS[leftTree_DS].url,
                name: leftTree_DS,
                id: "left",
                isok: true
            });
            var topSeries = new vjTreeSeries({
                url: vjDS[topTree_DS].url,
                name: topTree_DS,
                id: "top",
                isok: true
            });
            heatMapPlot.add(leftSeries);
            heatMapPlot.add(topSeries);
            return heatMapPlot;
        }
        
        // Coverage HeatMap
        var myCovHeatmapPlot = createHeatMapPlot("dsCovHeatmapMap", "dsCovHeatmapLeft","dsCovHeatmapTop");
        
        var myCovHeatmapViewer = new vjSVGView({
            chartArea: {width: "95%", height: "95%"},
            plots: [myCovHeatmapPlot],
            defaultEmptyText: "<p style='background-color:#0d47a1;color:white'>nothing to show</p>"
        });
        
        var cov_table = new vjTableControlX2({
            data: "dsCovHeatmapData",
            maxTxtLen:32,
            formObject:document.forms[formName]
        });
        
        // Insert HeatMap
        var myInsHeatmapPlot = createHeatMapPlot("dsInsHeatmapMap", "dsInsHeatmapLeft","dsInsHeatmapTop");
        
        var myInsHeatmapViewer = new vjSVGView({
            chartArea: {width: "95%", height: "95%"},
            plots: [myInsHeatmapPlot],
            defaultEmptyText: "<p style='background-color:#0d47a1;color:white'>nothing to show</p>"
        });
        
        var ins_table = new vjTableControlX2({
            data: "dsInsHeatmapData",
            maxTxtLen:32,
            formObject:document.forms[formName]
        });
        
        // 
        var filesStructureToAdd = [
           {
                tabId: 'resultTable',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    instance: table.arrayPanels
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'geneTable',
                tabName: algoProcess.typeTabName ? algoProcess.typeTabName : "Genes Table",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    instance: table_2.arrayPanels
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'covTable',
                tabName: "Coverages Table",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    instance: [myCovHeatmapViewer,cov_table]
                }/*,
                  autoOpen: ["computed"]*/
            },
            {
                tabId: 'insTable',
                tabName: "Inserts Table",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    instance: [myInsHeatmapViewer,ins_table]
                }/*,
                  autoOpen: ["computed"]*/
            }
           ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.openTab("resultTable");
        algoWidgetObj.closeTab("basicHelp");
    };
    
};

