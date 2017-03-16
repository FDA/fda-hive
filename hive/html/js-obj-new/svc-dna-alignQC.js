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

vjHO.register('svc-dna-alignQC').Constructor=function ()
{
    
    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-dna-alignQC.js");
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
//            vjDS.add("Loading stacked column chart....", "dsResult", "qpbg_tblqryx4://dna-test-Results.csv//cnt=20&raw=1&cols=0-200&objs="+thisProcessID);
//            vjDS.add("Loading complexity stats....", "dsResult2", "qpbg_tblqryx4://dna-complex-Results.csv//cnt=20&raw=1&cols=0-200&objs="+thisProcessID);
        vjDS.add("Loading...", "matrst", "qpbg_tblqryx4://dna-alignQC-match.csv//cnt=20000&raw=1&cols=0-200&objs="+thisProcessID);
        vjDS.add("Loading...", "insrst", "qpbg_tblqryx4://dna-alignQC-insert-delet.csv//cnt=20000&raw=1&cols=0-200&objs="+thisProcessID);
//        vjDS.add("Loading...", "delrst", "qpbg_tblqryx4://dna-alignQC-deletion.csv//cnt=20000&raw=1&cols=0-200&objs="+thisProcessID);
        vjDS.add("Loading...", "leftrst", "qpbg_tblqryx4://dna-alignQC-lefttail.csv//cnt=20000&raw=1&cols=0-200&objs="+thisProcessID);
        vjDS.add("Loading...", "rightrst", "qpbg_tblqryx4://dna-alignQC-righttail.csv//cnt=20000&raw=1&cols=0-200&objs="+thisProcessID);
        /*********************************** 
         * Create a Table View from a datasource
         * Mandatory:
         *         data: datasourcename
         */

        var match_panel = new vjPanelView({
            data: ["dsVoid",'matrst'],
            iconSize: 24,
            rows: [
                   {name:"mode",icon:"reqstatus2", showTitle:true,title:"Mode",description:"",order:1,align:"left"},
                   {name:"normalized", path: "/mode/normalized",title:"Normalized",description:"Show Normalized Mismatch Data",order:1,align:"left",url: switchMode},
                   {name:"raw", path: "/mode/raw",title:"Raw",description:"Show Raw Mismatch Data",order:2,align:"left",url: switchMode},
                   ],
            formObject: document.forms[formName],
        });
        
        var left_panel = new vjPanelView({
            data: ["dsVoid",'leftrst'],
            iconSize: 24,
            rows: [
                   {name:"lmintail",icon:"reqstatus2", showTitle:true,title:"Min-Tail",description:"",order:1,align:"left"},
                   {name:"left6", path: "/lmintail/left6",title:"6",description:"",order:1,align:"left",url: switchLeft},
                   {name:"left10", path: "/lmintail/left10",title:"10",description:"",order:2,align:"left",url: switchLeft},
                                      ],
            formObject: document.forms[formName],
        });
        
        var right_panel = new vjPanelView({
            data: ["dsVoid",'rightrst'],
            iconSize: 24,
            rows: [
                   {name:"rmintail",icon:"reqstatus2", showTitle:true,title:"Min-Tail",description:"",order:1,align:"left"},
                   {name:"right6", path: "/rmintail/right6",title:"6",description:"",order:1,align:"left",url: switchRight},
                   {name:"right10", path: "/rmintail/right10",title:"10",description:"",order:2,align:"left",url: switchRight},
                                      ],
            formObject: document.forms[formName],
        });
        
        var match=new vjGoogleGraphView({
            data: 'matrst',
            type: "column",
            series:[ 
                    {col:"0"},  // it can be defined by the column header name or column number
                    {col:"2"},
            ],
              options:{
                  title: "Normalized Mismatch Data",
                  hAxis: { title: 'Query Position', minValue:0},
                  vAxis: {title: 'Percentage'},
                  legend: { position: 'top', alignment:'end' }
                            
               }
        });
        var insertion=new vjGoogleGraphView({
            data: 'insrst',
            type: "line",
            series:[ 
                    {col:"0"},  // it can be defined by the column header name or column number
                    {col:"1"}
            ],
              options:{
                  legend: { position: 'top', alignment:'end' }        
               }
        });
        
        var deletion=new vjGoogleGraphView({
            data: 'insrst',
            type: "line",
            series:[ 
                    {col:"0"},  // it can be defined by the column header name or column number
                    {col:"2"},
                    {col:"3"},
                    {col:"4"}
            ],
              options:{
                  legend: { position: 'top', alignment:'end' }        
               }
        });
        var left=new vjGoogleGraphView({
            data: 'leftrst',
            type: "line",
            series:[ 
                    {col:"0"},  // it can be defined by the column header name or column number
                    {col:"1"},
            ],
              options:{
                  legend: { position: 'bottom' },
                  vAxis:{title: 'Appearance'},
                hAxis:{
                    viewWindow:{
                        min:6
                    },
                    title: 'Length'
                },
                legend: { position: 'top', alignment:'end' },

               }
        });
        var right=new vjGoogleGraphView({
            data: 'rightrst',
            type: "line",
            series:[ 
                    {col:"0"},  // it can be defined by the column header name or column number
                    {col:"1"},
            ],
              options:{
                  vAxis:{title: 'Appearance', alignment:'right'},
                  hAxis: {direction: -1,
                      title: 'Length',
                      viewWindow:{
                        min:6
                    }    
                  },
                  legend: { position: 'top', alignment:'end' },


               }
        });
        
           var filesStructureToAdd = [
      {
           tabId: 'matchgraph', 
           tabName: "Mismatch",
           position: {posId: 'graphArea1', top:'0', bottom:'50%', left:'20%', right:'60%'},
           viewerConstructor: {
               instance: [match_panel,match]
           },
             autoOpen: ["computed"]
       },
       {
           tabId: 'insertgraph',
           tabName: "Insertion",
           position: {posId: 'graphArea2', top:'50%', bottom:'100%', left:'60%', right:'100%'},
           viewerConstructor: {
               instance: [insertion]
           },
             autoOpen: ["computed"]
       },
       {
              tabId: 'deletegraph',
              tabName: "Deletion",
              position: {posId: 'graphArea3', top:'0', bottom:'50%', left:'60%', right:'100%'},
              viewerConstructor: {
                  instance: [deletion]
              },
             autoOpen: ["computed"]
       },
       {
             tabId: 'leftgraph',
            tabName: "Left Tail",
            position: {posId: 'graphArea4', top:'50%', bottom:'100%', left:'20%', right:'40%'},
            viewerConstructor: {
                instance: [left_panel,left]
            },
              autoOpen: ["computed"]
            },
       {
           tabId: 'rightgraph',
           tabName: "Right Tail",
           position: {posId: 'graphArea5', top:'50%', bottom:'100%', left:'40%', right:'60%'},
           viewerConstructor: {
               instance: [right_panel,right] // 
           },
           autoOpen: ["computed"]
       }
      ];
//        
//        var pieChart = new vjGoogleGraphView({
//            data: 'dsResult2',
//            type: "pie",
//            series:[ 
//                   {col: "0", label:true}  // it can be defined by the column header name or column number
//                  ,{col: "1"}
//                   ],
//            options:{
//                is3D: true
//            }
//        })
//
//
//           var filesStructureToAdd = [
//          {
//               tabId: 'resultsTable', 
//               tabName: "Results Table",
//               position: {posId: 'resultsTable', top:'0', bottom:'65%', left:'20%', right:'100%'},
//               viewerConstructor: {
//                   instance: [tbl]
//               },
//                 autoOpen: ["computed"]
//           },
//           {
//               tabId: 'ColumnChart',
//               tabName: "Column Chart",
//               position: {posId: 'graphArea1', top:'65%', bottom:'100%', left:'20%', right:'55%'},
//               viewerConstructor: {
//                   instance: [stackedColumn]
//               },
//                 autoOpen: ["computed"]
//           },
//           
//           {
//               tabId: 'complexStats',
//               tabName: "Complexity Stats",
//               position: {posId: 'graphArea2', top:'65%', bottom:'100%', left:'60%', right:'90%'},
//               viewerConstructor: {
//                   instance: [pieChart]
//               },
//               autoOpen: ["computed"]
//           }
//          ];

        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };
    
    function someCallback (viewer, node, irow)
    {
        console.log("calling from someCallback()")
    };
    
    function switchMode(viewer, node, irow) {
        var graph_viewer = algoWidgetObj.existsTab("matchgraph").viewerConstructor.instance[1];
        var title = "Normalized Data";
        var v = 'Percentage';
        if (node.name=="raw") {
            graph_viewer.series[1].col = "1";
            title = "Raw Data";
            v = 'Appearance';
        } 
        else {
            graph_viewer.series[1].col = "2";
        }
        graph_viewer.options.title = title;
        graph_viewer.options.vAxis.title = v;
        graph_viewer.refresh();
    }
    
    function switchLeft(viewer, node, irow){
        var graph_viewer = algoWidgetObj.existsTab("leftgraph").viewerConstructor.instance[1];
        var minl = 6;
        if (node.name=="left10"){
            minl = 10;
        }
        graph_viewer.options.hAxis.viewWindow.min = minl;
        graph_viewer.refresh();
    }
    function switchRight(viewer, node, irow){
        var graph_viewer = algoWidgetObj.existsTab("rightgraph").viewerConstructor.instance[1];
        var minr = 6;
        if (node.name=="right10"){
            minr = 10;
        }
        graph_viewer.options.hAxis.viewWindow.min = minr;
        graph_viewer.refresh();
    }
};
