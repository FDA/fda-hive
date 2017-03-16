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

    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-dna-multi-qc.js");
        // This is how to get the an argument from URL
        // In this case, we want to know about the 'id'
        var id = docLocValue("id");
        

        /***********************************
         *  Create a datasource 
         *  @param1: the title which is shown in the loading box
         *  @param2: the name of the datasource
         *  @param3: the url
         *  @param4: refresh call back (optional)
         *  @param5: add the header (optional) => use when you when you that the returned data from the server does not have the header  
         */
        vjDS.add("", "dsResult", "qpbg_tblqryx4://mqc.fileTable.csv//cnt=-1&raw=1&cols=0-200&objs="+thisProcessID);
        vjDS.add("", "dsFP", "qpbg_tblqryx4://mqc.positionTable.csv//cnt=-1&raw=1&cols=0-200&objs="+thisProcessID);
        //vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http://?cmd=propget&files=*.{csv,sc,zip,pdb}&mode=csv&prop=none&ids="+algoProcessID, 0, "id,name,path,value");
        /*********************************** 
         * Create a Table View from a datasource
         * Mandatory:
         *         data: datasourcename
         */

            var myTable = new vjTableControlX2({
                data: "dsResult"
                ,cols: [
                        { name: new RegExp(/.*/), hidden: true, align:"right" },
                        { name: 'FileName', hidden: false, align:"left"},
                        { name: 'Avg File Qual', hidden: false},
                        { name: 'Position Outlier Count', hidden: false},
                        { name: 'Num Reads', hidden: false},
                        { name: 'min Read Length', hidden: false},
                        { name: 'avg Read Length', hidden: false},
                        { name: 'max Read Length', hidden: false}
                     ],
                     onClickCellCallback: tableCallBack,
                   
            });
        
    //    var myTable = new vjTableView({
    //        data: "dsResult"
    //        ,cols: [
    //                { name: new RegExp(/.*/), hidden: true, align:"right" },
    //                { name: 'FileName', hidden: false, align:"left"},
    //                { name: 'qFile', hidden: false},
    //                { name: 'readLength', hidden: false},
    //                { name: 'totOutlierCount', hidden: false}
    //             ],
    //           
    //    });
        
        var posGraph=new vjGoogleGraphView({
            data: 'dsFP'
                ,type:"line"
                    ,series:[ 
                        {col:0, label:true}  // it can be defined by the column header name or column number
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
            ,intervals:{style:'area'}
             } 
        });

        //vjGoogleGraphControl readHist[0], readHist[1] will also have a panel to change chart type and to log scale        
        var readHist=new vjGoogleGraphView({
            data: 'dsResult'
                ,type:"scatter"
                    ,series:[ 
                        {col:0, label:true}  // it can be defined by the column header name or column number
                        ,{col:14}
                        ,{col:15}
                        ,{col:16}
                          ,{name:"annot", role:'annotation'}
                        ]
        ,precompute:"if (node.annot==undefined){node.annot=null;}"
        });

        
        var relativeBasePop=new vjGoogleGraphView({
              data: 'dsResult'
             ,type:"column"
             ,series:[ 
                       {col:0, label:true}  // it can be defined by the column header name or column number
                      ,{col:3}
                      ,{col:5}
                      ,{col:7}
                      ,{col:1}
                      ,{name:"annot", role:'annotation'}
                     ]
             ,options:{
                 isStacked: 'percent'
             } 
        ,precompute:"if (node.annot==undefined){node.annot=null;}"
        ,selectCallback: highlightTablePop

        });

        var baseQual=new vjGoogleGraphView({
            data: 'dsResult'
                ,type:"column"
                    ,series:[ 
                        {col:0, label:true}  // it can be defined by the column header name or column number
                        ,{col:2}
                        ,{col:8}
                        ,{col:6}
                        ,{col:4}
                        ,{col:9}
                        ,{name:"annot", role:'annotation'}
                        ]
        ,options:{
            isStacked: 'false'
                ,vAxis:{
                    minValue: 0,
                    maxValue: 40
                }
        }
             ,precompute:"if (node.annot==undefined){node.annot=null;}"
        ,selectCallback: highlightTableQual
        });
        
        
        
        /*************************************** 
         * Create a Google Graph View
         * Mandatory:
         *         data: datasourcename
         *         type: type of graph (pie, column, line ....)
         *         series: array of columns to use for drawing
         *                 the first element is considered as X
         *                 the rest are multiple Y
         * Optional:
         *         doHistogram: will draw based on the dictionary of X
         *         selectCallback: get called when select something in the graph
         */
        /**
         *    Create an array of tabs
         *    Mandatory:
         *        tabId: unique
         *        tabName: name of the tab will be shown on the interface
         *        position: relative position to the top bottom left right
         *        viewerCOntructor: pass the array of viewers          
         */
        var filesStructureToAdd = [
       {
            tabId: 'summary table', 
            tabName: "Summary Table",
            position: {posId: 'summaryTable', top:'0', bottom:'65%', left:'50%', right:'100%'},
            viewerConstructor: {
                instance: myTable.arrayPanels
            },
  //          viewerConstructor: {
  //              instance: myTable.arrayPanels[2]
  //          },
              autoOpen: ["computed"],
        },
        {
            tabId: 'position statistics', 
            tabName: "Position Quality",
            position: {posId: 'graphArea1', top:'0', bottom:'35%', left:'20%', right:'50%'},
            viewerConstructor: {
                instance: [posGraph]
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'Read Lengths', 
            tabName: "Read Lengths",    
            position: {posId: 'graphArea2', top:'35%', bottom:'65%', left:'20%', right:'50%'},
            viewerConstructor: {
                instance: [readHist]
            },
              autoOpen: ["computed"]
            ,selectCallback: highlightTableLen
        },
        {
            tabId: 'Relative Base Population',
            tabName: "Relative Base Population",
            position: {posId: 'graphArea3', top:'65%', bottom:'100%', left:'20%', right:'55%'},
            viewerConstructor: {
                instance: [relativeBasePop]
            },
              autoOpen: ["computed"]
              ,selectCallback: highlightTablePop
        },
        {
            tabId: 'Average Base Quality',
            tabName: "Average Base Quality",
            position: {posId: 'graphArea4', top:'65%', bottom:'100%', left:'55%', right:'100%'},
            viewerConstructor: {
                instance: [baseQual]
            },
              autoOpen: ["computed"]
            ,selectCallback: highlightTableQual
        }

        ];
        
        /** 
         * This is how you add a big tab view which contains list of small tabs
         * @param1: array of viewers
         * @param2: name of the tab
         */
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    
    };
    
    function highlightTablePop (viewer, node, irow)
    {
        console.log("calling from highlightTablePop()");
        var tblV= algoWidgetObj.existsTab("summary table");
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
        var tblV= algoWidgetObj.existsTab("summary table");
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
        console.log("calling from highlightTableLen()");
        var tblV= algoWidgetObj.existsTab("summary table");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };

    
    //function tableCallBack (viewer, node, irow, icol)
    function tableCallBack (viewer, node)
    {
        console.log("calling from tableCallBack()");
        console.log(node);
        
        var cur_objV = viewer;
        if (typeof(viewer)=="string" && typeof(node)=="number") {
            cur_objV = vjObj[viewer];
            node = cur_objV.tblArr.rows[node];
        }
        var tblV= algoWidgetObj.existsTab("summary table");
        //tblV.viewerConstructor.instance.colorRow(irow, 0, "#ffe282");
        var baseQualV= algoWidgetObj.existsTab("Average Base Quality");
        var basePopV= algoWidgetObj.existsTab("Relative Base Population");
        var readLenV= algoWidgetObj.existsTab("Read Lengths");
        for (i = 0; i < baseQualV.viewerConstructor.instance[0].dim(); i++) {  //population and quality graphs are never sorted, so row i will refer to the same file for both of them
            console.log(i);
            baseQualV.viewerConstructor.instance[0].row(i).annot=null
            basePopV.viewerConstructor.instance[0].row(i).annot=null
            readLenV.viewerConstructor.instance[0].row(i).annot=null
            if (baseQualV.viewerConstructor.instance[0].row(i).FileName == node.FileName){
                console.log("here here here");
                baseQualV.viewerConstructor.instance[0].row(i).annot="Here";
                basePopV.viewerConstructor.instance[0].row(i).annot="Here";
                readLenV.viewerConstructor.instance[0].row(i).annot="Here";
            }
        }    
        //baseQualV.viewerConstructor.instance[0].series = [ 
        //       {col:0, label:true}  // it can be defined by the column header name or column number
        //          ,{col:3}
        //          ,{col:5}
        //          ,{col:7}
        //         ]
        //baseQualV.viewerConstructor.instance[0].options.isStacked = true
        baseQualV.viewerConstructor.instance[0].refresh();
        basePopV.viewerConstructor.instance[0].refresh();
        readLenV.viewerConstructor.instance[0].refresh();
    };

    
};


    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
