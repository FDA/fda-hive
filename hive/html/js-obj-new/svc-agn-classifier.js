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

vjHO.register('svc-agn-classifier').Constructor=function ()
{

    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-agn-classifier.js");
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
        vjDS.add("", "dsResult", "qpbg_tblqryx4://agn-steps.out//cnt=10000&raw=1&cols=0-2&objs="+thisProcessID);
        //vjDS.add("", "dsFP", "qpbg_tblqryx4://mqc.positionTable.csv//cnt=-1&raw=1&cols=0-200&objs="+thisProcessID);
        //vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http://?cmd=propget&files=*.{csv,sc,zip,pdb}&mode=csv&prop=none&ids="+algoProcessID, 0, "id,name,path,value");
        /*********************************** 
         * Create a Table View from a datasource
         * Mandatory:
         *         data: datasourcename
         */

        
        var nrgGraph=new vjGoogleGraphView({
            data: 'dsResult'
                ,type:"line"
                    ,series:[ 
                        {col:0, label:true}  // it can be defined by the column header name or column number
                        ,{col:1}
                        ]
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
            tabId: 'energy per step', 
            tabName: "Energy Per Step",
            position: {posId: 'graphArea1', top:'0', bottom:'95%', left:'0%', right:'90%'},
            viewerConstructor: {
                instance: [nrgGraph]
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

    
    
};


    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
