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

vjHO.register('svc-CDRH-scripts').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-CDRH-scripts.js");
        var id = docLocValue("id");
        
        vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
        vjDS.add("", "dsResult", "http:
        
        var scatterGraph=new vjGoogleGraphView({
            data: 'dsResult',
             type:"scatter",
             selectCallback: someCallback
        });
    
        tbl = new vjTableControlX2({
            data: "dsResult"
        });
        
        menu = new vjPanelView({
             data: ['dsVoid','dsResult'],
             formObject: document.forms[formName],
             iconSize: 24,
             rows:[
                   { name : 'maxY', type : 'text', size : '6', isSubmitable : true, prefix : 'Maximum Y', align : 'left', order : '4', title: "max"},
                   { name : 'apply', order:100 , title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, align: "right", url: updateTables}
                   ]
         });
        
        var filesStructureToAdd = [{
                tabId: 'resultsGraph',
                tabName: "Results Graph",
                position: {posId: 'resultsGraph', top:'0', bottom:'75%', left:'20%', right:'75%'},
                viewerConstructor: {
                    instance: [menu, scatterGraph]
                },
                  autoOpen: ["computed"]
        },{
            tabId: 'resultsTable',
            tabName: "Results Table",
            position: {posId: 'resultsTable', top:'75%', bottom:'100%', left:'20%', right:'75%'},
            viewerConstructor: {
                instance: tbl.arrayPanels[2]
            },
              autoOpen: ["computed"]
        }];
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };
    
    function someCallback (viewer, node, irow)
    {
        console.log("calling from someCallback()")
    };
    
    function updateTables (viewer, node, ir)
    {
        console.log("calling from updateTables()")
    };
};


