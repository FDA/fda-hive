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

vjHO.register('svc-dna-pemerger').Constructor=function ()
{

    this.fullview=function(node, whereToAdd)
    {
        var id = docLocValue("id");
        
        gCGI=(document.location+"").substring(0,(document.location+"").indexOf("?"));
        vjDS.add("", "dsHTML", "static:
        vjDS.add("", "dsJSON", "http:
        var myHTML = new vjHTMLView({
            data: "dsHTML"
            ,formObject: document.forms[formName]
               
        });
        var myJSon = new vjHTMLView({
            data: "dsJSON"
            ,composer: "preview"
            ,formObject: document.forms[formName]
               
        });

        
        var filesStructureToAdd = [
            {
                tabId: 'HTML View',
                tabName: "fastP HTML",
                position: {posId: 'graphArea4', top:'0%', bottom:'100%', left:'20%', right:'80%'},
                viewerConstructor: {
                    instance: [myHTML]
                },
                  autoOpen: ["computed"]
            },
            {
                tabId: 'JSON View',
                tabName: "fastP json",
                position: {posId: 'graphArea2', top:'0%', bottom:'100%', left:'80%', right:'100%'},
                viewerConstructor: {
                    instance: [myJSon]
                },
                  autoOpen: ["computed"]
            }

        ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("downloadAllFiles");
    
    };
    
    function highlightTablePop (viewer, node, irow)
    {
        console.log("calling from highlightTablePop()");
        var tblV= algoWidgetObj.existsTab("summaryTable");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };

    function highlightTableQual (viewer, node, irow)
    {
        console.log("calling from highlightTableQual()");
        var tblV= algoWidgetObj.existsTab("summaryTable");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };


    function highlightTableLen (viewer, node, irow)
    {
        console.log("calling from highlightTableLen()");
        var tblV= algoWidgetObj.existsTab("summaryTable");
        tblV.viewerConstructor.instance[2].clearColors();
        for (i = 0; i < tblV.viewerConstructor.instance[2].dim(); i++) {
            if (node.FileName == tblV.viewerConstructor.instance[2].row(i).FileName){
                tblV.viewerConstructor.instance[2].colorRow(i, 0, "#ffe282");
                break;
                
            }
        }
    };

    
    function tableCallBack (viewer, node)
    {
        console.log("calling from tableCallBack()");
        console.log(node);
        
        var cur_objV = viewer;
        if (typeof(viewer)=="string" && typeof(node)=="number") {
            cur_objV = vjObj[viewer];
            node = cur_objV.tblArr.rows[node];
        }
        var tblV= algoWidgetObj.existsTab("summaryTable");
        var baseQualV= algoWidgetObj.existsTab("AverageBaseQuality");
        var basePopV= algoWidgetObj.existsTab("RelativeBasePopulation");
        var readLenV= algoWidgetObj.existsTab("ReadLengths");
        for (i = 0; i < baseQualV.viewerConstructor.instance[0].dim(); i++) {
            console.log(i);
            baseQualV.viewerConstructor.instance[0].row(i).annot=null
            basePopV.viewerConstructor.instance[0].row(i).annot=null
            readLenV.viewerConstructor.instance[0].row(i).annot=null
            if (baseQualV.viewerConstructor.instance[0].row(i).FileName == node.FileName){
                console.log("here here here");
                baseQualV.viewerConstructor.instance[0].row(i).annot="Here";
                basePopV.viewerConstructor.instance[0].row(i).annot="Here";
                readLenV.viewerConstructor.instance[0].row(i).annot="Here";
            }
        }    
        baseQualV.viewerConstructor.instance[0].refresh();
        basePopV.viewerConstructor.instance[0].refresh();
        readLenV.viewerConstructor.instance[0].refresh();
    };

    
};


    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
