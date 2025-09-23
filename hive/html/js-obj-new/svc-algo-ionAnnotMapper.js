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

javaScriptEngine.include("js/vjTableViewX2.js");
javaScriptEngine.include("js/vjPluggableToolbar.js");
javaScriptEngine.include("js/vjAnnotationView.js");



google.load("visualization", "1", {packages:["corechart"]});

vjHO.register('svc-algo-ionAnnotMapper').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-algo-ionAnnotMapper.js");
        var id = docLocValue("id");
        
        vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
        vjDS.add("", "dsSeq","http:
        
        
        vjDS.add("Retrieving cross-ranges", "crossmap","qpbg_tblqryx4:
        vjDS.add("Retrieving annotations", 'annotations',"static:
        vjDS.add("loading ... ","dsAnnotFiles","http:
        vjDS.add("Retrieving Statistics",'annot_stat',"static:
        
        var that = {};
        
        var main_objCls = "obj-annotMapper"+Math.random();
        vjObj.register(main_objCls,that);
        
        that.result_viewerArr = new vjAnnotationMapControl({
            data:'crossmap',
            formName: formName,
            selectCallback : "function:vjObjFunc('onClickRange','thisObjCls')",
            parentCls: main_objCls,
            isok:true});
        
            
        
        var preload_input = {"annot":""};
        if (algoProcess) {
            preload_input["annot"] = algoProcess.getValue("annot","join");
        }
        
        that.annotation_viewerArr = new vjAnnotationHitsControl({
            data: ["annotations"],
            annotFileData:"dsAnnotFiles",
            height:240,
            formName:formName,
            preload_input:preload_input,
            parentCls: main_objCls,
            isok:true
        });
        
        that.stat_viewerArr = new vjAnnotationHitsGraphControl({
            data: "annot_stat",
            height:240,
            formName:formName,
            preload_input:preload_input,
            parentCls: main_objCls,
            isok:true
        });
        
        var filesStructureToAdd = [
                   {
                        tabId: 'resultsTable',
                        tabName: "Results",
                        position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'100%'},
                        viewerConstructor: {
                            instance: that.result_viewerArr
                        },
                        autoOpen: ["computed"]
                   }
        ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");

    };
    
    this.onClickRange = function ( viewer, node) {
        var seqID = node.Reference;

        var parentObj = vjObj[viewer.parentCls];
        var ionObjs = parentObj["annotation_viewerArr"][0].tree.findByName("ionObjs").value;
        if (!ionObjs)
            return ;
        var rangeToAdd = "&pos_start=" + node.start + "&pos_end=" + node.end;
        var url = "qpbg_http:
        
        vjDS["annotations"].reload(url,true);
        vjDS["annotations"].parser = function (ds, text) {
            var tt= new vjTable(text, 0, vjTable_propCSV);
            var toReturn = "";
            if (tt.rows.length) {
                for (var ih=0; ih < tt.hdr.length; ++ih) {
                    if (ih) toReturn += ",";
                    toReturn += "" + tt.hdr[ih].name;
                }
                toReturn +=",id\n";
                for (var ir=0; ir < tt.rows.length; ++ir) {
                    toReturn += "\"" + tt.rows[ir].cols[0] + "\"," + tt.rows[ir].cols[1] + "," + tt.rows[ir].cols[2] + ",";
                    var type = tt.rows[ir].cols[3].split(";");
                    for (var it=0; it< type.length; ++it) {
                        if (it) toReturn += "\n,,,";
                        toReturn += "\"" + type[it].split(":")[0] + "\",\"" + type[it].split(":")[1] + "\"";  
                    }
                    toReturn += "\n";
                }
            }
            return toReturn.length ? toReturn : text;
        }
    };

};
