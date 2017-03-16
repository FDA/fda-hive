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

vjHO.register('svc-velvet').Constructor=function ()
{
    this.setUpJsonForAlgo = function (node, algoProcess)
    {
        var moveParams=[
                {
                        ds:"dsSystemParams",
                        params: ["comment","system"]
                },
                {
                        ds:"dsBatchParams",
                        params: ["batch", "batch_ignore_errors"]
                },
                {
                        ds:"dsInputParams",
                        params: algoProcess.visibleParameters
                },
        ];
        var dsArray=[
                    {name: "dsNameHere", url: "http://?cmd=dodo&id=$id"},
                    {name: "dsAlgoSpec", url: "http://?cmd=propspec&type="+algoProcess.svcProcType},
                    {name: "dsAlgoVals", url: "http://?cmd=propget&mode=csv&ids=$id&files=*"},
                    {name: "dsProgress", url: "http://?cmd=-qpRawCheck&showreqs=0&reqObjID=$id"},
                    {name: "dsProgress_download", url: "http://?cmd=-qpRawCheck&showreqs=0&reqObjID=$id" },
                    {name: "dsProgress_info", url: "http://?cmd=-qpReqInfo&reqObjID=$id"},
                    {name: "dsProgress_inputs", url: "http://?cmd=objQry&qry=allusedby($id).csv(['_type','_brief','created','svc','submitter'],{info:true.cnt:20})"},
                    {name: "dsProgress_outputs", url: "http://?cmd=objQry&qry=allusedby($id).filter({._type!~/folder/}).csv(['_type','_brief','created','svc','submitter'],{info:true.cnt:20})"},
                    {name: "dsHelp", url: "http://help/hlp.svc-single_velvet-pipeline.html"}
        ];
        
        for(i=0; i<moveParams.length; ++i) { 
            var t="";
            for( var ir=0; ir< moveParams[i].params.length; ++ir) {
                t+="<span id='RV-"+moveParams[i].params[ir]+"'></span>";
            }
            dsArray.push( {name: moveParams[i].ds, url: "static://"+t});
        }
        
           
            var setUpForPage = {
                algorithmTitle: "DNA Pipeline Velvet",
                parametersDiv: 'DV_Parameter_view',
                dataSourceArray: dsArray,
                subTabs: {
                    parameters: {
                        visibleDuring: ["preSubmit", "whileRunning", "computed"],
                        pageTabName: "Parameters",
                        pageTabChildren: [
                           {
                                tabId: 'parameters',
                                tabName: "Advanced",
                                showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                                tabDependents: ["input","system", "batch"],
                                inactive: true,
                                position: {posId: 'layout_inputs', top:'0', bottom:'50%', left:'0', right:'75%'},
                                viewerConstructor: {
                                    dataViewer: 'vjRecordView',
                                    dataViewerOptions: {
                                        divName: 'DV_Parameter_view',
                                        kind:"valgoProcess",
                                        data: algoProcess.loadedID ? [ "dsAlgoSpec", "dsAlgoVals" ] : ["dsAlgoSpec"],
                                        hiveId: algoProcess.svcProcType ,
                                        objType: algoProcess.svcProcType ,
                                        cmdPropSet:"?cmd=-qpProcSubmit&svc="+algoProcess.qpSvc,
                                        readonlyMode: algoProcess.modeActive ? false : true,
                                        callbackRendered : "function:vjObjFunc('onRecordLoaded','"+algoProcess.objCls+"')",
                                        onChangeCallback : "function:vjObjFunc('onRecordChanged','"+algoProcess.objCls+"')",
                                        onAddElementCallback : "function:vjObjFunc('onRecordAddedElement','"+algoProcess.objCls+"')",
                                        accumulateWithNonModified:true,
                                        showReadonlyInNonReadonlyMode: algoProcess.showReadonlyInNonReadonlyMode,
                                        constructionPropagateDown:10,
                                        autoexpand:0,
                                        showRoot:false,
                                        autoDescription:0,
                                        autoStatus:1,
                                        autoDescription:0,
                                        fldPresets: algoProcess.fldPresets ? cpyObj(batchFldPreset,algoProcess.fldPresets) : batchFldPreset,
                                        RVtag: "RV",
                                        formObject:document.forms[formName],
                                        isok:true
                                    }
                                },
                                autoOpen: ["preSubmit", "whileRunning", "computed"],
                                subTabs : {
                                    basicStuff: {
                                        pageTabName: "Basic Stuff",
                                        pageTabChildren: [
                                            {
                                                tabId: 'otherStuff',
                                                tabName: "Some Other Stuff",
                                                visible: true, //this is also modified by the algoview. just lets know if the tab is visible or not
                                                position: {posId: 'blah', top:'50%', bottom:'100%', left:'0', right:'75%'}
                                            }
                                        ]
                                    }
                                },
                                helpDS: "dsHelp"
                            },
                            {
                                tabId: 'input',
                                tabName: "General",
                                showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                                position: {posId: 'layout_inputs', top:'0', bottom:'50%', left:'0', right:'75%'},
                                viewerConstructor: {
                                    dataViewer: 'vjHTMLView',
                                    dataViewerOptions: {
                                        data: "dsInputParams"
                                    }
                                },
                                helpDS: "dsHelp"
                                //some div name here or the data source 
                            },
                            {
                                tabId: 'system',
                                tabName: "System",
                                showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                                inactive: true,
                                position: {posId: 'layout_inputs', top:'0', bottom:'50%', left:'0', right:'75%'},
                                //some div name here or the data source
                                viewerConstructor: {
                                    dataViewer: 'vjHTMLView',
                                    dataViewerOptions: {
                                        data: "dsSystemParams"
                                    }
                                },
                                helpDS: "dsHelp"                                
                            },
                            {
                                tabId: 'batch',
                                tabName: "Batch",
                                showSubmitButton: "<input id='submitterInput' type=button class='toSubmitBtn' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20 value='"+algoProcess.submitButtonName+"' />",
                                inactive: true,
                                position: {posId: 'layout_inputs', top:'0', bottom:'50%', left:'0', right:'75%'},
                                //some div name here or the data source 
                                viewerConstructor: {
                                    dataViewer: 'vjHTMLView',
                                    dataViewerOptions: {
                                        data: "dsBatchParams"
                                    }
                                },
                                helpDS: "dsHelp"
                            }
                        ]
                    },
                      progress: 
                     {
                          visibleDuring: ["whileRunning", "computed"],
                          pageTabName: "Progress",
                          pageTabChildren: [
                                {
                                    tabId: 'main',
                                    tabName: "Main Progress",
                                    tabDependents: ["inputObj", "objUsing", "info", "downloads", "next"],
                                    position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'0', right:'75%'},
                                    preConstructor: {
                                        dataViewer: 'vjProgressControl',
                                        dataViewerOptions: {
                                            data : {
                                                progress: "dsProgress",
                                                progress_download:  "dsProgress_download",
                                                inputs:  "dsProgress_inputs",
                                                outputs:  "dsProgress_outputs",
                                                progress_info: "dsProgress_info"
                                            },
                                            formName: formName,
                                            doneCallback: algoProcess.callbackDoneComputing
                                        }
                                    },
                                    viewerConstructor: {
                                        preObjPos: 0,
                                    },
                                    autoOpen: ["whileRunning"],
                                    helpDS: "dsHelp"
                                },
                                {
                                    tabId: 'inputObj',
                                    tabName: "Input Objects",
                                    position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'0', right:'75%'},
                                    viewerConstructor: {
                                        preObjPos: 1,
                                    },
                                    helpDS: "dsHelp"
                                },
                                {
                                    tabId: 'objUsing',
                                    tabName: "Objects Using This as Input",
                                    position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'0', right:'75%'},
                                    viewerConstructor: {
                                        preObjPos: 2,
                                    },
                                    helpDS: "dsHelp"
                                },
                                {
                                    tabId: 'info',
                                    tabName: "Info",
                                    position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'0', right:'75%'},
                                    viewerConstructor: {
                                        preObjPos: 3,
                                    },
                                    helpDS: "dsHelp"
                                },
                                {
                                    tabId: 'downloads',
                                    tabName: "Download",
                                    position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'0', right:'75%'},
                                    viewerConstructor: {
                                        preObjPos: 4,
                                    },
                                    helpDS: "dsHelp"
                                },
                                {
                                    tabId: 'next',
                                    tabName: "What's Next?",
                                    position: {posId: 'progress_info', top:'50%', bottom:'100%', left:'0', right:'75%'},
                                    viewerConstructor: {
                                        dataViewer: 'vjPanelView',
                                        dataViewerOptions: {
                                            data:"ds"+toolBar,
                                            formObject:document.forms[formName],
                                            iconSize:48,
                                            showTitles:true
                                        }
                                    }
                                }
                                /* ,
                                {
                                    tabId: 'nextSteps',
                                    tabName: "Next Steps",
                                    visible: false,
                                    position: {posId: 'middle', top:'0', bottom:'100%', left:'0', right:'75%'},
                                    divId: 'DV_dvprocessToolbar_toolbar_0'
                                    //some div name here or the data source 
                                } */
                             ]
                         },
                          results: {
                            visibleDuring: ["computed"],
                              pageTabName: "Results",
                              pageTabChildren: [
                        ]
                    }
                }
            };
        
        return setUpForPage;
    };
                
                
    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsDetails", "static://");
        vjDS.add("", "dsResult", "http://?cmd=propget&ids="+id+"&files=*.{fa,log,txt,sh}&mode=csv");
        vjDS.add("", "dsDownload", "static://");
        vjDS["dsResult"].parser = this.someFunction;
        
        var filesStructureToAdd = {
                tabId: 'resultFiles',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0', bottom:'30%', left:'0', right:'75%'},
                viewerConstructor: {
                    dataViewer: 'vjTableView',
                    dataViewerOptions: {
                        data: "dsResult",
                        cols:[
                                   {name:"archive", url: this.ingestSomethingLikeThat},
                                   {name:"file", url: this.showingSomethingLikeThat},
                                   {name:"download", url: this.downloadResultsFile}
                        ],
                        bgColors:['#f2f2f2','#ffffff'],
                        formObject:document.forms[formName],
                        isok:true
                    }
                }
            };
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };

    this.preview = function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsDetails", "static://");
        vjDS.add("", "dsResult", "http://?cmd=propget&ids="+id+"&files=*.{fa,log,txt,sh}&mode=csv");
        vjDS.add("", "dsDownload", "static://");
        vjDS["dsResult"].parser = this.someFunction;
        
        var filesStructureToAdd = {
                tabId: 'resultFiles',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0', bottom:'30%', left:'0', right:'75%'},
                viewerConstructor: {
                    dataViewer: 'vjTableView',
                    dataViewerOptions: {
                        data: "dsResult",
                        cols:[
                                   {name:"archive", url: this.ingestSomethingLikeThat},
                                   {name:"file", url: this.showingSomethingLikeThat},
                                   {name:"download", url: this.downloadResultsFile}
                        ],
                        bgColors:['#f2f2f2','#ffffff'],
                        formObject:document.forms[formName],
                        isok:true
                    }
                }
            };
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };

    this.mobile = function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsDetails", "static://");
        vjDS.add("", "dsResult", "http://?cmd=propget&ids="+id+"&files=*.{fa,log,txt,sh}&mode=csv");
        vjDS.add("", "dsDownload", "static://");
        vjDS["dsResult"].parser = this.someFunction;
        
        var filesStructureToAdd = {
                tabId: 'resultFiles',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0', bottom:'30%', left:'0', right:'75%'},
                viewerConstructor: {
                    dataViewer: 'vjTableView',
                    dataViewerOptions: {
                        data: "dsResult",
                        cols:[
                                   {name:"archive", url: this.ingestSomethingLikeThat},
                                   {name:"file", url: this.showingSomethingLikeThat},
                                   {name:"download", url: this.downloadResultsFile}
                        ],
                        bgColors:['#f2f2f2','#ffffff'],
                        formObject:document.forms[formName],
                        isok:true
                    }
                }
            };
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };
    
    //these functions must be defined by the template
    this.ingestSomethingLikeThat = function (table,row,col)
    {
        algoWidgetObj.removeTabs("details", "results");
        
        processID = docLocValue("id")
        if (table.ingestedRow.indexOf(row.irow) !=-1) return;
        var url = "?cmd=objFile&arch=1&ids=" + processID + "&filename=" + row["velvet results file"] + "&arch_dstname="+ row["velvet results file"]+"&backend=1";
        //alert("ingesting ..." + url)
        vjDS["dsDownload"].reload("qpbg_http://" +url,true);
        alert ("Your selected item is being ingested. You can monitor the progress from within data loading tab");
        row.download = "<img src='img/done.gif' width=12 height=12 />";
        table.ingestedRow.push(row.irow);
        gObject("ingest_" + row.irow).style.display = "none";
        gObject("done_" + row.irow).style.visibility = "visible";
    };
    
    this.showingSomethingLikeThat = function (table,row,col)
    {
        if (!algoWidgetObj.existsTab("details"))
        {
            var detailsStructureToAdd = {
                    tabId: 'details',
                    tabName: "Details",
                    openUp: true,
                    position: {posId: 'detailsView', top:'30%', bottom:'100%', left:'0', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjTextView',
                        dataViewerOptions: {
                            data: "dsDetails",
                            font: "contClassJonny",
                            formObject:document.forms[formName],
                            isok:true
                        }
                    }
                };
            
            algoWidgetObj.addTabs(detailsStructureToAdd, "results");
        }
        
        processID = docLocValue("id")
        var url = "http://?cmd=objFile&ids=" + processID + "&filename=" + row["velvet results file"];
        vjDS["dsDetails"].reload(url,true);
    };
    
    this.downloadResultsFile = function (table,row,col)
    {
        if (!algoWidgetObj.existsTab("details"))
        {
            var detailsStructureToAdd = {
                    tabId: 'details',
                    tabName: "Details",
                    position: {posId: 'detailsView', top:'30%', bottom:'100%', left:'0', right:'75%'},
                    viewerConstructor: {
                        dataViewer: 'vjTextView',
                        dataViewerOptions: {
                            data: "dsDetails",
                            font: "contClassJonny",
                            formObject:document.forms[formName],
                            isok:true
                        }
                    }
                };
            
            algoWidgetObj.addTabs(detailsStructureToAdd, "results");
        }
        
        processID = docLocValue("id")
        var url = "http://?cmd=objFile&ids=" + processID + "&filename=" + row["velvet results file"];
        vjDS["dsDetails"].reload(url,true,"download");
    };
    
    this.someFunction = function  (ds, data){
        var tt = new vjTable(data,0,vjTable_propCSV);
        var fileList = tt.rows[0]["_file"];
        var newTable = "velvet results file,archive file,download file\n";
        for (var i=0; i < fileList.length; ++i){                        
            newTable += "" + fileList[i] + ",<img src='img/copy.png' width=12 height=12 id=ingest_"+ i+" /> <img src='img/done.gif' width=12 height=12 id=done_"+ i+" style='visibility:hidden;'/>,<img src='img/download.gif' width=12 height=12 id=ingest_"+ i+" /> \n";
        }
        return newTable;
    };
};



