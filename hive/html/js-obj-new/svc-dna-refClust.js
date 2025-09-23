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
javaScriptEngine.include("js/vjHiveseqView.js");

google.load("visualization", "1", {packages:["corechart"]});

vjHO.register('svc-dna-refClust').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-dna-refClust.js");
        var id = docLocValue("id");
        
        
        
        vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
        vjDS.add("", "dsSeq","http:
        
        var hiveseqControl= new vjHiveseqControl({
            data: 'dsSeq',
            checkable: false,
            formName: formName
        });
        
        var jumpToAlignPanel = new vjPanelView({
            data:["dsVoid"]
            ,rows: [{ name : 'run_Alignment', type:"button",icon: "hive-hexagon.gif", title: "Launch Alignment", description: "run Alignment", showTitle: true, align: 'left' , order : 1, url: jumpToAlignment}
                    ]
            ,formObject: document.forms[formName]
            ,iconSize:20
            ,isok: true 
        });
        
        hiveseqControl = hiveseqControl.concat(jumpToAlignPanel);
        
        var filesStructureToAdd = [{
            tabId: 'resultsTable',
            tabName: "Results Table",
            position: {posId: 'resultsTable', top:'0', bottom:'75%', left:'20%', right:'75%'},
            viewerConstructor: {
                instance: hiveseqControl
            },
              autoOpen: ["computed"]
        }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        
    };
    
    function jumpToAlignment(viewer,node,irow) {
        var iid = docLocValue("id");
        var url = "?cmd=dna-hexagon&query="+iid+"&keepAllMatches=2&doubleStagePerfect=0";
        linkURL(url,1);
    }

};
