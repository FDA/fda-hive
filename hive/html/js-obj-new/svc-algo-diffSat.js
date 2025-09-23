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

vjHO.register('svc-algo-diffSat').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-algo-diffSat.js");
        algoWidgetObj.closeTab("basicHelp");
        var id = docLocValue("id");
        
        vjDS.add("Loading ....", "dsResult", "qpbg_tblqryx4:
        vjDS.add("Loading ....", "dsResultGraph", "qpbg_tblqryx4:
        
        var tbl = new vjTableView({
            data: "dsResult"
        });
        
        var panel = new vjBasicPanelView({
            data: ["dsVoid","dsResult"]
            ,formObject:document.forms[formName]
        });
        
        var pieGraph=new vjGoogleGraphView({
              data: 'dsResultGraph'
             ,type:"pie"
             ,doHistogram: true
             ,series:[ 
                       {col:0, label:true}
                      ,{col: 1}
                     ]
             ,selectCallback: someCallback 
        });
    
        var filesStructureToAdd = [
       {
            tabId: 'resultsTable', 
            tabName: "Results Table",
            position: {posId: 'resultsTable', top:'0', bottom:'65%', left:'20%', right:'100%'},
            viewerConstructor: {
                instance: [panel,tbl]
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'pieGraph',
            tabName: "Pie Graph",
            position: {posId: 'graphArea1', top:'65%', bottom:'100%', left:'20%', right:'55%'},
            viewerConstructor: {
                instance: [pieGraph]
            },
              autoOpen: ["computed"]
        }
       ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };
    
    function someCallback (viewer, node, irow)
    {
        console.log("calling from someCallback()")
    };
    
};
