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
$(function () {
    var oThis;
    tqs=[];
    latestReqID=-1;
    $.widget("view.tableWidgetController", {
        options:{
            data: "dsVoid",
            layoutManagerFunc: "$.getLayoutManager()",
            dataCounter: 0,
            tableTabName: "Results Table",
            mainArea: "mainArea",
            graphicsArea: "mainArea",
            allGraphicsPlugins: ["basicGraph", "heatmap", "tree", "som", "statTest", "bStat", "pearsonCorr", "fourier", "kaplanmeier"],
            lengthOfTqs: -1,
            sizeAtUndo: -1,
            redoCommands: [],
            allPluginsList: []
        },
        _create: function () {
            oThis = this;
            
            vjDS.add("Student T-Test", "dsStatTest", "static:
            vjDS.add("Student T-Test", "dsStatTestManhattan", "static:
            vjDS.add("Bayesian Stat", "dsBStat", "static:
            vjDS.add("Pearson Correlation", "dsPearsonCorrHeat", "static:
            vjDS.add("Pearson Correlation", "dsPearsonCorrMax", "static:
            vjDS.add("Pearson Heatmap", "dsPearsonHeatmapMap", "static:
            vjDS.add("Fourier Even", "dsFourier", "static:
            vjDS.add("Fourier Graph", "dsFourierGraph", "static:
            vjDS.add("Kaplan Meier", "dsKaplanMeier", "static:
            vjDS.add("Dictionary Table", "dsDicTbl", "static:
            vjDS.add("Clustering Tree", "dsTree", "static:
            
            vjDS.add("Command Log", "dsCommandLog", "static:
            
            oThis.options.myGraphViewer = new vjGoogleGraphView({
                data: oThis.options.data,
                name: 'basicGraph',
                options:{
                    colors:vjPAGE.colorsACGT,
                    width: 800, height: 300, chartArea :{height:'100%', width: '100%' },
                    legend: 'none',
                    isStacked:true,
                    pieSliceText: 'label'
                    },
                  series:[]
            });
            
            oThis.options.tableViewer = new vjTableControlX2({
                data: oThis.options.data,
                name:'table',
                graph: oThis.options.myGraphViewer,
                formObject: document.forms[oThis.options.formName],
                multiSelect:true,
                colorCategorizer:true,
                callbackRendered: oThis._tableRendered,
                localSortDisable:true,
                menuUpdateCallbacks: oThis._menuUpdateCallback,
                isok:true
            });
            
            for (var key in oThis.options.tableViewer.arrayPanels.plugins)
                oThis.options.allPluginsList.push(key);
            
            oThis.options.commandLogViewer = new vjTableView({
                data: "dsCommandLog",
                formObject: document.forms[oThis.options.formName],
                bgColors:['#f2f2f2','#ffffff']
            });

            var commandLogViewerToolbar=new vjPanelView({
                data:['dsVoid', "dsCommandLog"],
                iconSize:24,
                formObject: document.forms[oThis.options.formName],
                showTitles:true,
                rows: [
                    {name:'saveCommands', align:'left', order:0 ,title: 'Save the commands in a TQS file',  icon:'save', url: oThis._downloadFunc},
                    {name:'undoCommand', align:'left', order:-1 ,title: 'Undo the latest command',  icon:'recRevert', url: oThis._undoFunc},
                    {name:'redoCommand', align:'left', order:1 ,title: 'Redo the latest command',  icon:'recRedo', url: oThis._redoFunc}
                    ]
                });
           
            var manager = eval(oThis.options.layoutManagerFunc);
            manager.append({
                layout: {
                    items:[
                    {
                        id: oThis.options.mainArea,
                        tabs:{
                            items: [{
                                active:true,
                                title: oThis.options.tableTabName,
                                name:'resultsTable',
                                allowClose: false,
                                overflow: "auto",
                                view: {
                                    name: 'dataview',
                                    options: {
                                        instance: oThis.options.tableViewer.arrayPanels
                                    }
                                }
                            }]
                        }
                    }, {
                        id: oThis.options.mainArea,
                        tabs:{
                            items: [{
                                active:false,
                                title: "Command Log",
                                name: 'commandLog',
                                allowClose: false,
                                overflow: "auto",
                                view: {
                                    name: 'dataview',
                                    options: {
                                        instance: [commandLogViewerToolbar, oThis.options.commandLogViewer]
                                    }
                                }
                            }]
                        }
                    }]
                }
            });
            manager.remove('results');
            if (this.options.completedCallback)
                this.options.completedCallback(manager);
        },
        _onAfterInit: function(){
            console.log("after init");
        },
        _menuUpdateCallback: function (viewer, url) {
               var curtqs = oThis.options.tableviewArr.arrayPanels[oThis.options.tableviewArr.arrayPanels.length-1].tqsObj;
               
               for (var i = 0; i < curtqs.length; i++)
               {
                   if (curtqs[i].op == "collapsewithstat" )
                   {
                       curtqs[i].arg["reverse"] = false;
                       var cols = curtqs[i].arg.colSet.toString();
                       var colArray = [];
           
                       
                       while (cols.indexOf(",") >= 0)
                       {
                           var toPut = cols.substring(0,cols.indexOf(","));
                           cols = cols.substring (cols.indexOf(",")+1);
                           colArray.push ("$"+toPut);
                       }
                       colArray.push ("$" + cols);
                       curtqs[i].arg["formulas"] = colArray;
                   } 
               }
               
                var ds=new vjDataSource();
               return urlExchangeParameter(url, "tqs", ds.escapeQueryLanguage(JSON.stringify(curtqs)));
          },
          _tableRendered: function (viewer,text){
              viewer = oThis.options.tableViewer.arrayPanels[oThis.options.tableViewer.arrayPanels.length-1];
              
              var manager = eval(oThis.options.layoutManagerFunc);
              for (var i = 0; i < oThis.options.allGraphicsPlugins.length; i++){
                  manager.remove (oThis.options.allGraphicsPlugins[i]);
              }
              
              tqs = viewer.tqsObj;
              if (tqs == []){
                  var dataUrl = vjDS[oThis.options.data].url;
                  if (dataUrl.indexOf(tqs) > -1){
                      var str = dataUrl.substr(dataUrl.indexOf(tqs)+4);
                      str = str.substr(0, str.indexOf("&"));
                      console.log(str);
                  }      
              }
            if (latestReqID == -1)
                latestReqID = vjDS[oThis.options.data].qpbg_params.RQ.dataID;
              
            
            for (var i = 0; tqs && i < tqs.length; i++){
                if( viewer.tqsObj[i].op == "heatmap") {
                    var myHeatmapPlot = new vjSVG_HeatMap({
                        color: {min: "blue", max: "#ffc200", mid:"white"},
                        heat_image_url: "http:
                        heat_image_min_cells: 10000,
                        legend_range: {min_text: "minimum", mid_text: "", max_text: "maximum"}
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
                    
                      manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Heatmap",
                                       name:'heatmap',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: myHeatmapViewer
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                      
                      vjDS["dsHeatmapMap"].reload("http:
                    vjDS["dsHeatmapTop"].reload("http:
                    vjDS["dsHeatmapLeft"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "tree" ) {
                    var treePanel = new vjPanelView({
                        data: ["dsVoid", "dsTree"],
                        iconSize: 24,
                        formObject: document.forms[oThis.options.formName],
                        showTitles:true,
                        rows: [
                            {name:'downloadTree', align:'left', order:1 ,title: 'Download Tree File' , icon: "download", showTitleForInputs: true, url: oThis._downloadTree}
                            ]
                    });
                    var treeViewer = new vjD3JS_TreeView({
                        data: 'dsTree',
                        newick: true,
                        useDepthForTree: oThis.options.useDepthForTree,
                        openLevels:-1
                    });
                    
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Tree",
                                       name:'tree',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: [treePanel, treeViewer]
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                    
                      vjDS["dsTree"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "som" ) {
                    var dsSom = new vjDataSeries({
                        url: "static:
                        name: "dsSom",
                        id: "som",
                        type: "raw",
                        isok: true
                    });
                    var dsRows = new vjDataSeries({
                        url: "static:
                        name: "dsRows",
                        id: "rows",
                        type: "raw",
                        isok: true
                    });

                    plot = new vjSVG_SOM();

                    plot.add(dsSom);
                    plot.add(dsRows);

                    var mySomViewer = new vjSVGView({
                        chartArea: {width: "95%", height: "95%"},
                        plots: [plot],
                        isok: true
                    });
                    
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Self Organizing Map",
                                       name:'som',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: mySomViewer
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                        
                      vjDS["dsSom"].reload("http:
                      vjDS["dsRows"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "statTest" ) {
                    var myStatTestViewer=new vjTableView({
                        data: 'dsStatTest',
                        formObject: document.forms[oThis.options.formName],
                           bgColors: ['#f2f2f2', '#ffffff'],

                        isok: true
                    });
                    
                    var statTestToolbar = new vjPanelView({
                        data:['dsVoid', 'dsTblQryX2'],
                        iconSize:24,
                        formObject: document.forms[oThis.options.formName],
                        showTitles:true,
                        rows: [
                            {name:'downloadPic', align:'left', order:1 ,title: 'Download files', icon: 'save', showTitleForInputs: true, path: '/downloadPic', url: oThis.downloadStatTest}
                            ]
                       });
                    
                    var statTestManhattanPlot= new vjGoogleGraphView({
                        data:"dsStatTestManhattan",
                        series:[
                                {col:0, title: "Identifier"},
                                {col:1, title: "Log P"}
                                ],
                        options:{
                            width : gPgW*0.6,
                            height : 250,
                            chartArea : {
                                top : 20,
                                left : 80,
                                height : '80%',
                                width : '80%'
                            },
                            hAxis: {title: "Identifier"},
                            vAxis : {title: "-log(p)"}
                        },
                        type:"scatter",
                        isok: true
                    });
                    
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Student T-Test",
                                       name:'statTest',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: [statTestToolbar, myStatTestViewer, statTestManhattanPlot]
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                    
                      vjDS["dsStatTest"].reload("http:
                      vjDS["dsStatTestManhattan"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "bStat" ) {
                    var myBStatViewer=new vjTableView({
                        data: 'dsBStat',
                        formObject: document.forms[oThis.options.formName],
                        bgColors: ['#f2f2f2', '#ffffff'],

                        isok: true
                    });
                    
                    var BStatToolbar = new vjPanelView({
                        data:['dsVoid', 'dsTblQryX2'],
                        iconSize:24,
                        formObject: document.forms[oThis.options.formName],
                        showTitles:true,
                        rows: [
                            {name:'downloadPic', align:'left', order:1 ,title: 'Download files', icon: 'save', showTitleForInputs: true, path: '/downloadPic', url: oThis.downloadBStat}
                            ]
                    });
                    
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Bayesian Stat",
                                       name:'bStat',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: [BStatToolbar, myBStatViewer]
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                    
                      vjDS["dsBStat"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "pearsonCorr" ) {
                    var myPearsonMatrix=new vjTableView({
                        data: 'dsPearsonCorrHeat',
                        formObject: document.forms[oThis.options.formName],
                        bgColors: ['#f2f2f2', '#ffffff'],

                        isok: true
                    });
                    
                    var myPearsonBest=new vjTableView({
                        data: 'dsPearsonCorrMax',
                        formObject: document.forms[oThis.options.formName],
                        bgColors: ['#f2f2f2', '#ffffff'],
                        selectCallback: oThis.myPearsonCallback, 
                        isok: true
                    });
                    
                    var myPearsonHeatmapPlot = new vjSVG_HeatMap({
                        color: {min: "blue", max: "yellow", mid:"black"},
                        heat_image_min_cells: 10000
                    });
                    myPearsonHeatmapPlot.add(new vjDataSeries({
                        url: "static:
                        name: "dsPearsonHeatmapMap",
                        id: "heat",
                        type: "raw",
                        minCellSize: 50,
                        isok: true
                    }));
                    
                    var myPearsonHeatmapViewer = new vjSVGView({
                        chartArea: {width: "95%", height: "95%"},
                        plots: [myPearsonHeatmapPlot]
                    });
                    
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Pearson Correlation",
                                       name:'pearsonCorr',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: [myPearsonHeatmapViewer, myPearsonMatrix, myPearsonBest]
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                    
                      vjDS["dsPearsonHeatmapMap"].reload("http:
                      vjDS["dsPearsonCorrHeat"].reload("http:
                      vjDS["dsPearsonCorrMax"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "fourier" ) {
                       var fourierViewerToolbar=new vjPanelView({
                        data:['dsVoid', 'dsTblQryX2'],
                        iconSize:24,
                        formObject: document.forms[oThis.options.formName],
                        showTitles:true,
                        rows: [
                            {name:'downloadPic', align:'left', order:1 ,title: 'Download files', icon: 'save', showTitleForInputs: true, path: '/downloadPic'},
                            {name: 'even', align:'left', order:1 ,title: 'Download Frequency Table',  showTitleForInputs: true, path: '/downloadPic/even', url: oThis.downloadEven}
                            ]
                        });
                    var myFourierEven=new vjTableView({
                        data: 'dsFourier',
                        formObject: document.forms[oThis.options.formName],
                        options: {width: "100%"},
                        bgColors: ['#f2f2f2', '#ffffff'],
                        selectCallback: oThis.myFourierCallback, 
                        isok: true
                    });
                    
                    myFourierGraph=new vjD3JS_StackableGroupBars({
                        data: "dsFourier",
                        cols: [],
                        labelCol:"ID"
                    });
                    
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Fourier Transform",
                                       name:'fourier',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: [fourierViewerToolbar, myFourierEven, myFourierGraph]
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                    
                      vjDS["dsFourier"].reload("http:
                  }
                else if( viewer.tqsObj[i].op == "kaplanmeier") {
                      vjDS["dsKaplanMeier"].reload("http:
                  }
                else if (viewer.tqsObj[i].op == "basicGraph"){
                    manager.append({
                        layout: {
                            items:[{
                               id: oThis.options.graphicsArea,
                               tabs:{
                                   items: [{
                                       active:true,
                                       title: "Graph",
                                       name:'basicGraph',
                                       allowClose: true,
                                       overflow: "auto",
                                       view: {
                                           name: 'dataview',
                                           options: {
                                               instance: oThis.options.myGraphViewer
                                           }
                                       }
                                   }]
                              }
                            }]
                        }
                      });
                }
            }
            
            oThis._constructLogViewerTable( oThis.options.commandLogViewer);
          },
          _constructLogViewerTable: function(that)
          {
              var strToUse = "Number,Operation\n";
              var num = 1;
              
              if (oThis.options.lengthOfTqs != 0){
                  for (var irow = 0; (oThis.options.lengthOfTqs>=0 && irow< oThis.options.lengthOfTqs) || irow < tqs.length; irow++)
                  {
                      var operation = tqs[irow];
                      var toPrint ="";
                      
                      if (operation.toPrint)
                          toPrint = operation.toPrint;
                      
                      if (toPrint != ""){
                          strToUse += (num + "," + toPrint + "\n");
                          num++;
                      }
                  }
              }
              
              vjDS["dsCommandLog"].reload("static:
          },
          _downloadFunc: function (vv, table, irow, icol) {
              var url = "?cmd=-qpData&req="+latestReqID+"&dname=tqs.json&dsaveas=tqs.json";
              oThis._download(url);
          },
          _downloadTree: function (view, a, b, c){
              console.log ("something");
              var url = vjDS["dsTree"].url;
              vjDS["dsTree"].reload(url,true,"download");
          },
          _download : function (url) {
              var pom = document.createElement('a');
              pom.setAttribute('href', url);
              pom.setAttribute('download', "tqs.json");
              pom.setAttribute ('onclick', "document.execCommand('SaveAs',true,'file.html');");
              document.execCommand('SaveAs',true,'file.html');
              document.body.appendChild(pom);
              pom.click();
              document.body.appendChild(pom);
          },
          _undoFunc: function (vv, table, irow, icol) {
              if (oThis.options.lengthOfTqs < 0)    
                  oThis.options.lengthOfTqs = tqs.length;
              
              if (oThis.options.lengthOfTqs <= 0){
                  oThis._createPopUp ("There are no operations to UNDO");
                  return;
              }
              
              oThis.options.redoCommands[oThis.options.redoCommands.length] = tqs[tqs.length-1];
              oThis.options.sizeAtUndo = tqs.length;
              if (oThis.options.allPluginsList.indexOf(tqs[tqs.length-1].op)){
                  oThis.options.sizeAtUndo--;
                  oThis.options.redoCommands[oThis.options.redoCommands.length] = tqs[tqs.length-2];
                  oThis.options.lengthOfTqs--;
              }

              var url = urlExchangeParameter(oldUrl, "tqsCnt", oThis.options.lengthOfTqs-1);
              vjDS["dsTblQryX2"].reload(url,true);
              oThis.options.lengthOfTqs--;              

            var manager = eval(oThis.options.layoutManagerFunc);
            manager.show('resultsTable');
          },
          _redoFunc: function(vv, table, irow, icol) {
              if (oThis.options.sizeAtUndo == -1 || oThis.options.lengthOfTqs >= tqs.length){
                  oThis._createPopUp ("There are no operations to REDO");
                  return;
              }
              oThis.options.sizeAtUndo++;

              var url = urlExchangeParameter(oldUrl, "tqsCnt", oThis.options.sizeAtUndo);
              url = urlExchangeParameter(url, "tqs",  vjDS["dsTblQryX2"].escapeQueryLanguage(JSON.stringify(tqs)));
              vjDS["dsTblQryX2"].reload(url,true);
              oThis.options.lengthOfTqs++;              

            var manager = eval(oThis.options.layoutManagerFunc);
            manager.show('resultsTable');
          },
          _createPopUp: function (text){
            $("body").append(
                    $(document.createElement("div"))
                        .attr("id", "dialog")
                        .attr("title", "Dialog")
                        .append (
                                $(document.createElement("p")).text(text)
                        )
                    );
                
                $("#dialog").dialog({
                    modal: true,
                    width: 500,
                    buttons: {
                        OK: function() {
                           $(this).dialog("close");
                        }
                    },
                    open: function() {
                        $(this).closest(".ui-dialog")
                        .find(".ui-dialog-titlebar-close")
                        .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                        .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                    }
                });
          },
          myFourierCallback: function (viewer, node, irow, icol, cellHeader) {
              if (cellHeader.name.indexOf(" Even") < 0 && cellHeader.name.indexOf(" Odd") < 0)
                  return;
              
              var actualColName;
              if (cellHeader.name.indexOf(" Even") > 0)
                  actualColName = cellHeader.name.substring (0, cellHeader.name.indexOf(" Even"));
              else
                  actualColName = cellHeader.name.substring (0, cellHeader.name.indexOf(" Odd"));
              
              myFourierGraph.cols = [{name: actualColName + " Even"}, {name: actualColName + " Odd"}];

              myFourierGraph.composerFunction(myFourierGraph, "", viewer.data);
          },
          myPearsonCallback: function (viewer, node) {
              var tqsToUse = [{op: "hidecol", arg: {cols: "*"}},
                              {op: "appendcol", arg: {name: "Name Column", formula: "$0"}},
                              {op: "appendcol", arg: {name: "New " + node.cols[0], formula: "${"+node.cols[0]+"}"}},
                              {op: "appendcol", arg: {name: "New " + node.cols[1], formula: "${"+node.cols[1]+"}"}},
                              ];
              if (!vjDS["dsPearsonNewDS"])
                  vjDS.add("Pearson Heatmap", "dsPearsonNewDS", "static:
              
              var nTqs = tqs.concat(tqsToUse);
              var nUrl = urlExchangeParameter (oldUrl, "tqs", vjDS["dsPearsonNewDS"].escapeQueryLanguage(JSON.stringify(nTqs))); 
                  
              vjDS["dsPearsonNewDS"].reload(nUrl, true);
          },
          downloadStatTest: function (viewer)
          {
              var url = "http:
              vjDS["dsStatTest"].reload(url,true,"download");
          },
          downloadBStat: function (viewer)
          {
              var url = "http:
              vjDS["dsBStat"].reload(url,true,"download");
          },
          downloadEven: function (viewer)
          {
              var url = "http:
              vjDS["dsFourier"].reload(url,true,"download");
          }
    });
}(jQuery));
