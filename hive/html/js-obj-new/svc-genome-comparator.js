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
javaScriptEngine.include("js/vjGoogleGraphView.js");

vjHO.register('svc-genome-comparator').Constructor=function ()
{        
    var chordDiagram = new vjD3JS_CorrelationCord({
        data: "dsSimilar",
        type:"column",
        oneColorPerCateg: true,
        sortLables: true,
        staticColor: true,
        pairs: {array: [["ref1","range1"], ["ref2","range2"]]},
        wrapLength:32,
        onClickCallback: diagramCallback,
        pairValue : ["similarity"]                     
    });
    var table = new vjTableControlX2({
        data: "dsSimilar",
        isStickyHeader: true, 
        onClickCellCallback: tableCallback,
        formObject:document.forms[formName]
    });
    var oldTableIndex = -1, oldDiagramIndex = -1;
    var mummerGraph = new vjGoogleGraphView({ 
        data: "dsMatrix",
        options: {height: "400px'", colors:[]},
        changeHeight: false,
        type:"scatter",
        series:[]
    });
    
    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsSimilar", "qpbg_tblqryx4:
        vjDS.add("", "dsMatrix", "static:
        vjDS.dsSimilar.register_callback(similarCallback);
                
        var filesStructureToAdd = [{
                tabId: 'resultTable',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'55%'},
                viewerConstructor: {
                    instance: table.arrayPanels
                },
                  autoOpen: ["computed"]
            },{
                tabId: 'chordGraph',
                tabName: "Chord Graph",
                overflow: "hidden",
                position: {posId: 'resultGraph', top:'0', bottom:'100%', left:'55%', right:'100%'},
                viewerConstructor: {
                    instance: chordDiagram
                },
                  autoOpen: ["computed"]
            },{
                tabId: 'mummerGraph',
                tabName: "Mummer Graph",
                overflow: "auto",
                position: {posId: 'resultGraph', top:'0', bottom:'100%', left:'55%', right:'100%'},
                viewerConstructor: {
                    instance: mummerGraph
                },
                  autoOpen: ["computed"]
            }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("basicHelp");
    };
    
    function similarCallback(viewer, content)
    {
        var parsed = chordDiagram.csvParserFunction(content);
        var matrixObj = new Array();
        var matrixLabels = [parsed[0].ref1, parsed[0].ref2];
        
           for (var i = 0; i < parsed.length; i++)
        {
            var pos1 = parseInt(parsed[i].range1);
            var pos2 = parseInt(parsed[i].range2);
            var label1 = parsed[i].ref1;
            var label2 = parsed[i].ref2;
            if (pos1 == NaN || pos2 == NaN) continue;
            
            matrixObj.push([pos1, pos2]);
            matrixObj.push([pos2, pos1]);
        }
        
        
        var stringToPut = matrixLabels.toString() + "\n";
        mummerGraph.series=[];
        mummerGraph.options.colors=[];
        mummerGraph.series.push({name:matrixLabels[0]});
        mummerGraph.series.push({name:matrixLabels[1]});
        mummerGraph.series.push({name:matrixLabels[0]});
        for (var i = 0; i < matrixObj.length; i++)
        {
            stringToPut += matrixObj[i].toString() + "\n";

            mummerGraph.options.colors.push("blue");
            mummerGraph.options.colors.push("red");
        }
        
        vjDS.dsMatrix.reload("static:
    }
    
    function tableCallback (viewer, node, ir, ic)
    {
        var colToUse = ic-(ic%2);
        var indexOfLabel = chordDiagram.matrixObjCpy.labels.indexOf(node.cols[colToUse].split(" ")[0]+":"+node.cols[colToUse+1].split(" ")[0]);
        if (indexOfLabel < 0)
            return;
        
        if (oldTableIndex >= 0)
            chordDiagram.performTheFade(1,oldTableIndex);
        chordDiagram.performTheFade(0.1,indexOfLabel);
        
        oldTableIndex = indexOfLabel;
    };
    
    function diagramCallback (node, index)
    {
        var curLabels = chordDiagram.matrixObjCpy.labels[index].split(":");
        var rowsToColor = [];
        
        table.tableViewer.enumerate (function (view, tbl, ir){ 
            var node = tbl.rows[ir]; 
            if ((node.ref1.indexOf(curLabels[0]) >=0 && node.range1.indexOf(curLabels[1]) >= 0) ||
                    (node.ref2.indexOf(curLabels[0]) >=0 && node.range2.indexOf(curLabels[1]) >= 0))
                rowsToColor.push (ir);
        }, table.tableViewer);
        
        if (oldTableIndex != -1)
        {
            for (var i = 0; i < oldTableIndex.length; i++)
                table.tableViewer.colorRow(oldTableIndex[i],0,"#ffe282");
        }
        
        for (var i = 0; i < rowsToColor.length; i++)
            table.tableViewer.colorRow(rowsToColor[i],0,"#ffe282");
        
        oldTableIndex = rowsToColor;
    };
};

