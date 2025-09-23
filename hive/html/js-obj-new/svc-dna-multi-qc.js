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
google.load("visualization", "1", {packages:["corechart"]});

vjHO.register('svc-dna-multi-qc').Constructor=function ()
{

    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        
        vjDS.add("", "dsResult", "qpbg_tblqryx4:
        vjDS.add("", "dsFP", "qpbg_tblqryx4:

        var myTable = new vjTableControlX2({
            data: "dsResult"
            ,formObject: document.forms[formName]
            ,cols: [
                { name: new RegExp(/.*/), hidden: true, align:"right" },
                { name: 'FileName', hidden: false, align:"left"},
                { name: 'Avg File Qual', hidden: false},
                { name: 'Position Outlier Count', hidden: false},
                { name: 'Num Reads', hidden: false},
                { name: 'min Read Length', hidden: false},
                { name: 'avg Read Length', hidden: false},
                { name: 'max Read Length', hidden: false},
                { name: 'gc Content', hidden: false}
            ],
            onClickCellCallback: tableCallBack,
        });
        
        var posGraph=new vjGoogleGraphView({
            data: 'dsFP'
            ,type:"line"
            ,series:[ 
                {col:0, label:true}
                ,{col:2}
                ,{col:1, role:'interval'}
                ,{col:3, role:'interval'}
                ,{col:5}
                ,{col:7, type:'string', role:'tooltip'}
                ,{col:6}
                ,{col:8, type:'string', role:'tooltip'}
            ]
            ,options:{
                series: {
                    0: {color: '#43459d'}
                }
                ,intervals:{ style:'area' }
                , hAxis: { title: "Position"}
                , vAxis: { title: "Quality"}
             } 
        });

        var readHist=new vjGoogleGraphView({
            data: 'dsResult'
            ,type:"scatter"
            ,series:[ 
                {col:0, label:true}
                ,{col:14}
                ,{col:15}
                ,{col:16}
                ,{name:"annot", role:'annotation'}
            ]
            ,options:{
                hAxis: { 
                    title: "File", 
                    slantedText:true
                },
                vAxis: { title: "Length"}
            }
            ,precompute:"if (node.annot==undefined){node.annot=null;}"
        });

        
        var relativeBasePop=new vjGoogleGraphView({
            data: 'dsResult'
            ,type:"column"
            ,series:[ 
                {col:0, label:true}
                ,{col:3}
                ,{col:5}
                ,{col:7}
                ,{col:1}
                ,{name:"annot", role:'annotation'}
            ]
            ,options:{
                isStacked: 'percent'
                ,hAxis: { 
                    title: "File",
                    slantedText:true
                },
                vAxis: { title: "Relative Population"}
            } 
            ,precompute:"if (node.annot==undefined){node.annot=null;}"
            ,selectCallback: highlightTablePop
        });

        var baseQual=new vjGoogleGraphView({
            data: 'dsResult',
            type:"column",
            series:[ 
                {col:0, label:true},
                {col:2},
                {col:8},
                {col:6},
                {col:4},
                {col:9},
                {name:"annot", role:'annotation'}
            ],
            options:{
                isStacked: 'false',
                vAxis:{
                    title: "Quality",
                    minValue: 0,
                    maxValue: 40
                },
                hAxis: { 
                    title: "File",
                    slantedText:true,
                    textStyle: {
                        textOverflow: 'ellipsis'
                    }
                }
            },
            precompute:"if (node.annot==undefined){node.annot=null;}",
            selectCallback: highlightTableQual
        });
        
        
        
        var filesStructureToAdd = [
       {
            tabId: 'summaryTable', 
            tabName: "Summary Table",
            position: {posId: 'summaryTable', top:'0', bottom:'65%', left:'50%', right:'100%'},
            viewerConstructor: {
                instance: myTable.arrayPanels
            },
            autoOpen: ["computed"],
        },
        {
            tabId: 'positionStatistics', 
            tabName: "Position Quality",
            position: {posId: 'graphArea1', top:'0', bottom:'35%', left:'20%', right:'50%'},
            viewerConstructor: {
                instance: [posGraph]
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'ReadLengths', 
            tabName: "Read Lengths",    
            position: {posId: 'graphArea2', top:'35%', bottom:'65%', left:'20%', right:'50%'},
            viewerConstructor: {
                instance: [readHist]
            },
              autoOpen: ["computed"]
            ,selectCallback: highlightTableLen
        },
        {
            tabId: 'RelativeBasePopulation',
            tabName: "Relative Base Population",
            position: {posId: 'graphArea3', top:'65%', bottom:'100%', left:'20%', right:'55%'},
            viewerConstructor: {
                instance: [relativeBasePop]
            },
              autoOpen: ["computed"]
              ,selectCallback: highlightTablePop
        },
        {
            tabId: 'AverageBaseQuality',
            tabName: "Average Base Quality",
            position: {posId: 'graphArea4', top:'65%', bottom:'100%', left:'55%', right:'100%'},
            viewerConstructor: {
                instance: [baseQual]
            },
              autoOpen: ["computed"]
            ,selectCallback: highlightTableQual
        }

        ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("downloadAllFiles");
    };
    
    function highlightTablePop (viewer, node, irow)
    {
        console.log("calling from highlightTablePop()");
        var tblV= algoWidgetObj.existsTab("summaryTable");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };

    function highlightTableQual (viewer, node, irow)
    {
        console.log("calling from highlightTableQual()");
        var tblV= algoWidgetObj.existsTab("summaryTable");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };


    function highlightTableLen (viewer, node, irow)
    {
        
        var tblV= algoWidgetObj.existsTab("summaryTable");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };

    
    function tableCallBack (viewer, node)
    {
        console.log("calling from tableCallBack()");
        console.log(node);
        
        var cur_objV = viewer;
        if (typeof(viewer)=="string" && typeof(node)=="number") {
            cur_objV = vjObj[viewer];
            node = cur_objV.tblArr.rows[node];
        }
        var baseQualV= algoWidgetObj.existsTab("AverageBaseQuality");
        var basePopV= algoWidgetObj.existsTab("RelativeBasePopulation");
        var readLenV= algoWidgetObj.existsTab("ReadLengths");
        for (i = 0; i < baseQualV.viewerConstructor.instance[0].dim(); i++) {

            baseQualV.viewerConstructor.instance[0].row(i).annot=null
            basePopV.viewerConstructor.instance[0].row(i).annot=null
            readLenV.viewerConstructor.instance[0].row(i).annot=null
            
            if (baseQualV.viewerConstructor.instance[0].row(i).FileName == node.FileName){
                
                baseQualV.viewerConstructor.instance[0].row(i).annot="Here";
                basePopV.viewerConstructor.instance[0].row(i).annot="Here";
                readLenV.viewerConstructor.instance[0].row(i).annot="Here";
            }
        }    
        baseQualV.viewerConstructor.instance[0].refresh();
        basePopV.viewerConstructor.instance[0].refresh();
        readLenV.viewerConstructor.instance[0].refresh();
    };

    
};


    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
