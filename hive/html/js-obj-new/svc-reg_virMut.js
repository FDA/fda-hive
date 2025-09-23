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

vjHO.register('svc-viral-mutation-comp').Constructor=function ()
{                    
    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        vjDS.add("", "dsResult", "qpbg_tblqryx4:
        vjDS.add("", "dsInputResult", "qpbg_tblqryx4:
        vjDS.add("", "dsFilter_InputResult", "qpbg_tblqryx4:
        vjDS.add("", "dsFilteredResult", "qpbg_tblqryx4:
        
        var tableController1 = new vjTableControlX2 ({
            data: "dsResult",
            formObject:document.forms[formName]
        });
        var tableController2 = new vjTableControlX2 ({
            data: "dsInputResult",
            formObject:document.forms[formName]
        });
        var tableController3 = new vjTableControlX2 ({
            data: "dsFilteredResult",
            formObject:document.forms[formName]
        });
        var tableController4 = new vjTableControlX2 ({
            data: "dsFilter_InputResult",
            formObject:document.forms[formName]
        });        
        
        
        var filesStructureToAdd = [{
                tabId: 'resultTable',
                tabName: "Result Table",
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'75%'},
                inactive: true,
                viewerConstructor: {
                    instance: tableController1.arrayPanels
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'inputTable',
                tabName: "Input Table",
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'75%'},
                inactive: true,
                viewerConstructor: {
                    instance: tableController2.arrayPanels
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'inputFilteredTable',
                tabName: "Intermediate Table",
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'75%'},
                inactive: true,
                viewerConstructor: {
                    instance: tableController4.arrayPanels
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'graphTab',
                tabName: "Graph",
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'75%'},
                inactive: true,
                viewerConstructor: {
                    dataViewer: 'vjGoogleGraphView',
                    dataViewerOptions: {
                        data: "dsResult",
                        type:"column",
                        series:[ {label:true, name:'SUB10+-'}, {name: 'Number OF Subjects'} ],
                        options: { title:'Prominent Substitutions', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: 600, height: 600, colors:vjPAGE.colorsACGT,  vAxis: { title: 'Count', minValue:0}  },
                        cols:[{ name: 'SUB10+-', order:1, align:'center',title: 'Substitution', hidden: false },
                              { name: 'Number OF Subjects', order:2,  title: 'Count', hidden: false }]
                    }
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'filteredTable',
                tabName: "Filtered Table",
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'75%'},
                viewerConstructor: {
                    instance: tableController3.arrayPanels
                },
                  autoOpen: ["computed"]
            }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
    };  
};


