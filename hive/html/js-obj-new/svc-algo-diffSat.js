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
    
    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-algo-diffSat.js");
        // This is how to get the an argument from URL
        // In this case, we want to know about the 'id'
        algoWidgetObj.closeTab("basicHelp");
        var id = docLocValue("id");
        
        /***********************************
         *  Create a datasource 
         *  @param1: the title which is shown in the loading box
         *  @param2: the name of the datasource
         *  @param3: the url
         *  @param4: refresh call back (optional)
         *  @param5: add the header (optional) => use when you when you that the returned data from the server does not have the header  
         */
        vjDS.add("Loading ....", "dsResult", "qpbg_tblqryx4://result.csv//cnt=20&raw=1&cols=0-200&objs="+id);
        vjDS.add("Loading ....", "dsResultGraph", "qpbg_tblqryx4://result.csv//cnt=-1&raw=1&cols=0-200&objs="+id);
        
        /*********************************** 
         * Create a Table View from a datasource
         * Mandatory:
         *         data: datasourcename
         */
        var tbl = new vjTableView({
            data: "dsResult"
        });
        
        /***********************************
         * Create a Panel View from a datasource
         * For the panel, you need to specify the formObject
         * Mandatory:
         *         data: datasourcename
         *         formObject: 
         */
        var panel = new vjBasicPanelView({
            data: ["dsVoid","dsResult"]
            ,formObject:document.forms[formName] // the panel need the formObject
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
        var pieGraph=new vjGoogleGraphView({
              data: 'dsResultGraph'
             ,type:"pie"
             ,doHistogram: true
             ,series:[ 
                       {col:0, label:true}  // it can be defined by the column header name or column number
                      ,{col: 1}
                     ]
             ,selectCallback: someCallback 
        });
    
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
        
        /** 
         * This is how you add a big tab view which contains list of small tabs
         * @param1: array of viewers
         * @param2: name of the tab
         */
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };
    
    function someCallback (viewer, node, irow)
    {
        console.log("calling from someCallback()")
    };
    
};
