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

vjHO.register('svc-dna-codonQC').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-dna-codonQC.js");
        var id = docLocValue("id");
        
            vjDS.add("Loading stacked column chart....", "dsResult", "qpbg_tblqryx4:
            vjDS.add("Loading complexity stats....", "dsResult2", "qpbg_tblqryx4:

        var tbl = new vjTableView({
            data: "dsResult"
        });

        var stackedColumn=new vjGoogleGraphView({
              data: 'dsResult'
             ,type: "column"
             ,series:[ 
                  {col:"0",label:true}
                  ,{col:"1"}
                  ,{col:"2"}
           ]
           ,options:{

                isStacked: 'percent'                                        
           }
        });
        
        var pieChart = new vjGoogleGraphView({
            data: 'dsResult2',
            type: "pie",
            series:[ 
                   {col: "0", label:true}
                  ,{col: "1"}
                   ],
            options:{
                is3D: true
            }
        })


           var filesStructureToAdd = [
          {
               tabId: 'resultsTable', 
               tabName: "Results Table",
               position: {posId: 'resultsTable', top:'0', bottom:'65%', left:'20%', right:'100%'},
               viewerConstructor: {
                   instance: [tbl]
               },
                 autoOpen: ["computed"]
           },
           {
               tabId: 'ColumnChart',
               tabName: "Column Chart",
               position: {posId: 'graphArea1', top:'65%', bottom:'100%', left:'20%', right:'55%'},
               viewerConstructor: {
                   instance: [stackedColumn]
               },
                 autoOpen: ["computed"]
           },
           
           {
               tabId: 'complexStats',
               tabName: "Complexity Stats",
               position: {posId: 'graphArea2', top:'65%', bottom:'100%', left:'60%', right:'90%'},
               viewerConstructor: {
                   instance: [pieChart]
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
