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

vjHO.register('svc-generic-launcher').Constructor=function ()
{                    
    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsDetails", "static://");
        vjDS.add("", "dsResult", "http://?cmd=propget&ids="+id+"&files=*&mode=csv");
        vjDS.add("", "dsDownload", "static://");
        vjDS["dsResult"].parser = this.parseData;
        
        var filesStructureToAdd = {
                tabId: 'resultFiles',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0', bottom:'30%', left:'20%', right:'75%'},
                viewerConstructor: {
                    dataViewer: 'vjTableView',
                    dataViewerOptions: {
                        data: "dsResult",
                        cols:[
                                   {name:"archive", url: this.archiveFile},
                                   {name:"file", url: this.showFile},
                                   {name:"download", url: this.downloadFile}
                        ],
                        bgColors:['#f2f2f2','#ffffff'],
                        formObject:document.forms[formName],
                        isok:true
                    }
                }
            };
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };  
    
    
    //
    // This function archives the file into HIVE when it is clicked on in the interface.  It is a callback function for the Results Table (vjTableView)
    //
    this.archiveFile = function (table,row,col)
    {
        if (table.ingestedRow.indexOf(row.irow) !=-1) return;
        var url = "?cmd=objFile&arch=1&ids=" + processID + "&filename=" + row["velvet results file"] + "&arch_dstname="+ row["velvet results file"]+"&backend=1";
        alert("ingesting ..." + url)
        vjDS["dsDownload"].reload("qpbg_http://" +url,true);
        alert ("Your selected item is being ingested. You can monitor the progress from within data loading tab");
        row.download = "<img src='img/done.gif' width=12 height=12 />";
        table.ingestedRow.push(row.irow);
        gObject("ingest_" + row.irow).style.display = "none";
        gObject("done_" + row.irow).style.visibility = "visible";
    };
    
    //
    // This function shows the file in the preview window when it is clicked on in the interface.  It is a callback function for the Results Table (vjTableView)
    //
    this.showFile = function (table,row,col)
    {
        if (!algoWidgetObj.existsTab("details"))
        {
            var detailsStructureToAdd = {
                    tabId: 'details',
                    tabName: "Details",
                    openUp: true,
                    position: {posId: 'detailsView', top:'30%', bottom:'100%', left:'20%', right:'75%'},
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
    
    //
    // This function downloads the file into the user's harddrive when it is clicked on in the interface.  It is a callback function for the Results Table (vjTableView)
    //
    this.downloadFile = function (table,row,col)
    {
        var detailsStructureToAdd = {
                tabId: 'details',
                tabName: "Details",
                position: {posId: 'detailsView', top:'30%', bottom:'100%', left:'20%', right:'75%'},
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
    
        processID = docLocValue("id")
        var url = "http://?cmd=objFile&ids=" + processID + "&filename=" + row["velvet results file"];
        vjDS["dsDetails"].reload(url,true,"download");
    };
    
    this.parseData = function  (ds, data){
        var tt = new vjTable(data,0,vjTable_propCSV);
        var fileList = tt.rows[0]["_file"];
        var newTable = "velvet results file,archive file,download file\n";
        for (var i=0; i < fileList.length; ++i){                        
            newTable += "" + fileList[i] + ",<img src='img/copy.png' width=12 height=12 id=ingest_"+ i+" /> <img src='img/done.gif' width=12 height=12 id=done_"+ i+" style='visibility:hidden;'/>,<img src='img/download.gif' width=12 height=12 id=ingest_"+ i+" /> \n";
        }
        return newTable;
    };
};



