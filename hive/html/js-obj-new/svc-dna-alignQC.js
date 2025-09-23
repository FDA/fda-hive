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
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-dna-alignQC.js");
        var id = docLocValue("id");
        
        vjDS.add("Loading Left Tail Data", "dsLeftTail", "qpbg_tblqryx4:
        vjDS.add("Loading Right Tail Data", "dsRightTail", "qpbg_tblqryx4:
        vjDS.add("Loading Position Data", "dsPosStats", "qpbg_tblqryx4:
        
        var deletionGraph = new vjGoogleGraphView({
            data: "dsPosStats",
            type: "line",
            series: [ 
                    { col: 0 }, 
                    { col: 4, title: "1 Contiguous Deletion" },
                    { col: 5, title: "2 Contiguous Deletions" },
                    { col: 6, title: "3+ Contiguous Deletions" }
            ],
              options: {
                  title: "Deletions per Query Position",
                  legend: { position: 'right' },
                 hAxis: { title: "Query Position" },
                 vAxis: { title: "Number of Deletions" },
             }
        });
        
        var insertionGraph = new vjGoogleGraphView({
            data: "dsPosStats",
            type: "line",
            series:[ 
                    { col: 0 }, 
                    { col: 3 }
            ],
              options: {
                  title: "Insertions per Query Position",
                 legend: { position: 'none' },
                 hAxis: { title: "Query Position" },
                 vAxis: { title: "Number of Insertions" },
             }
        });
        
        var mismatchGraph = new vjGoogleGraphView({
            data: "dsPosStats",
            type: "line",
            series: [ 
                    { col: 0 }, 
                    { col: 2 }
            ],
              options:{
                  title: "Mismatches per Query Position",
                 legend: { position: 'none' },
                 hAxis: { title: "Query Position" },
                 vAxis: { title: "Number of Mismatches" },
             }
        });
        
        var leftTailGraph = new vjGoogleGraphView({
            data: "dsLeftTail",
            type: "line",
            series:[ 
                    { col: 0 }, 
                    { col: 1 }
            ],
              options: {
                  title: "Left Tail Length Count",
                 legend: { position: 'none' },
                hAxis: { title: "Tail Length", scaleType: 'log' },
                 vAxis: { title: "Frequency" },
             }
        });
        
        var rightTailGraph = new vjGoogleGraphView({
            data: "dsRightTail",
            type: "line",
            series:[ 
                    { col: 0 }, 
                    { col: 1 }
            ],
              options: {
                  title: "Right Tail Length Count",
                 legend: { position: 'none' },
                hAxis: { title: "Tail Length", scaleType: 'log' },
                 vAxis: { title: "Frequency" }
             }
        });
        
        var filesStructureToAdd = [
            {
                tabId: "insertionTab",
                tabName: "Insertions",
                position: {posId: 'insertonPos', top:'50%', bottom:'100%', left:'60%', right:'100%'},
                viewerConstructor: {
                    instance: insertionGraph
                },
                autoOpen: ["computed"]
            },
            {
                tabId: "deletionTab",
                tabName: "Deletions",
                position: {posId: 'deletionPos', top:'50%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    instance: deletionGraph
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'MismatchTab',
                tabName: "Mismatches",
                position: {posId: 'mismatchPos', top:'0', bottom:'50%', left:'60%', right:'100%'},
                viewerConstructor: {
                    instance: mismatchGraph
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'leftTailTab',
                tabName: "Left Tail",
                position: {posId: 'tailPos', top:'0', bottom:'50%', left:'20%', right:'60%'},
                viewerConstructor: {
                    instance: leftTailGraph
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'rightTailTab',
                tabName: "Right Tail",
                position: {posId: 'tailPos'},
                viewerConstructor: {
                    instance: rightTailGraph
                },
                autoOpen: ["computed"]
            }
        ];

        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.openTab("leftTailTab");
    };
    
};
