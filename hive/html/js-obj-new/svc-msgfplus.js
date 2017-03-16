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
google.setOnLoadCallback();


var selectedProteinId = '';
var selectedPeptideId = '';
var selectedSpectrumId = '';
var selectedPeptideSeq = '';

var selectedProteinObj = {};
var peptideObjList = {};
var proteinTracks = [];

var tabList = {};


 
vjHO.register('svc-msgfplus').Constructor=function (){


function loadedCallbackAndUpdateSelectors (a,b,c,d) {
        parentLoadedCallback(a,b,c,d);
        updateSelectors();
}
   


 
    this.fullview=function(node, whereToAdd){
        var coid = docLocValue("id");
        //var inObjId = algoProcess.getValue("spectrumfile");
       
        var main_objCls = "obj-MSGF"+Math.random();
            vjObj.register(main_objCls,node);

        var style = 'font-size:12px;color:#777;display:none;background:#fff;border-top:1px solid #ccc;';
        var htmlTable = '<table style="display:none;" id=browsetable  border=0>' +
            '<tr><td colspan=3>&nbsp;</td></tr>' +
            '<tr><td style="width:280px;padding:0 0 0 20px;">Select protein</td>' +
            '<td style="width:420px;padding:0 0 0 20px;">Select pepetide</td>' +
            '<td style="padding:0 0 0 20px;">Select scan</td></tr>' +
            '<tr><td style="padding:0 0 0 20px;" id=selector1 valign=top></td>' +
            '<td style="padding:0 0 0 20px;" id=selector2 valign=top></td>' + 
            '<td style="padding:0 0 0 20px;" id=selector3 valign=top></td></tr>' +
            '</table>';



    vjDS.add("Loading ..... ", "dsTable1", "qpbg_tblqryx4://sample_summary.csv//objs="+coid);
     vjDS.add("Loading ..... ", "dsTable2", "qpbg_tblqryx4://identification_summary.csv//objs="+coid);
     vjDS.add("Loading ..... ", "dsText1", "static://1,2,3,4,5\n");
    vjDS.add("Loading ..... ", "dsText2", "static://"+htmlTable+"\n");
    
    var mytext2 = new vjHTMLView({data: "dsText2",type: "column"});


    var mytable1 = new vjTableView({
        data: "dsTable1"
        ,bgColors : [ '#f2f2f2', '#ffffff' ]
        ,formObject: document.forms[formName]
        ,checkable:false
         ,cols:[{name:"sampleName", order:1, url: showSampleGraphs}]
        ,precompute:"if (node.Rawfile) { var tmp = node.Rawfile; tmp = tmp.split('\\\\\'); node.Rawfile = tmp[tmp.length -1];} if (node.Use == 'True') {node.checked=true;}"
        ,parentObjCls: main_objCls
         
           });


    var mypanel2 = new vjBasicPanelView({
                        data: ['dsVoid',"dsTable2"],
                        formObject: document.forms[formName],
                        parentObjCls: main_objCls
                });

                var mytable2 = new vjTableView({
                        data: "dsTable2"
                        ,bgColors : [ '#f2f2f2', '#ffffff' ]
                        ,formObject: document.forms[formName]
                        ,checkable:false
                        ,cols:[
                                {name:"SpecFile", order:1}
                                ,{name:"ScanNum", order:2}
                                ,{name:"Protein", order:3,url: showProteinView}
                                ,{name:"Peptide",order:4}
                                ,{name:"SpecID",hidden:true}
                                ,{name:"FragMethod",hidden:true}
                        ]
                        ,precompute:"if (node.Rawfile) { var tmp = node.Rawfile; tmp = tmp.split('\\\\\'); node.Rawfile = tmp[tmp.length -1];} if (node.Use == 'True') {node.checked=true;}"
                        ,parentObjCls: main_objCls
                });

    viewStructureToAdd = [];
                viewStructureToAdd[0] = {tabId: 'level1_tab1', tabName: "Samples",
                        position: {posId: 'pos_1', top:'0', bottom:'50%', left:'20%', right:'75%'},
                        viewerConstructor: {instance: [mytable1]},
                        autoOpen: ["computed"],
            inactive:true
                };
                viewStructureToAdd[1] = {tabId: 'level1_tab2', tabName: "Identifications list",
                        position: {posId: 'pos_1', top:'0', bottom:'50%', left:'20%', right:'75%'},
                        viewerConstructor: {instance: [mypanel2, mytable2]},
                        autoOpen: ["computed"],
            inactive:true
                };

    viewStructureToAdd[2] = {tabId: 'level1_tab3', tabName: "Browse identifications",
                        position: {posId: 'pos_1', top:'0', bottom:'50%', left:'20%', right:'75%'},
                        viewerConstructor: {instance: [mytext2]},
                        autoOpen: ["computed"],
            inactive:false
                };

        algoWidgetObj.addTabs(viewStructureToAdd, "results");
        //algoWidgetObj.closeTab("basicHelp");
               
        updateSelectors();
                if(algoProcess && algoProcess.callbackLoaded) {
            parentLoadedCallback = algoProcess.callbackLoaded;
                }
        algoProcess.callbackLoaded = loadedCallbackAndUpdateSelectors;



    };
    
}

///////////////////////////
function proteinSelectorHandler(){
    var val = $('select[name=protein]').val();
        selectedProteinId = (val == undefined ? selectedProteinId : val);

    selectedPeptideId = '';
    selectedSpectrumId = '';        
    
    updateSelectors();
    
}

////////////////////////
function peptideSelectorHandler(){
        
    if (selectedProteinId == ''){
        var val = $('select[name=protein]').val();
            selectedProteinId = (val == undefined ? selectedProteinId : val);
        }
    var val = $('select[name=peptide]').val();
        selectedPeptideId = (val == undefined ? selectedPeptideId : val);
        var val = $('select[name=spectrum]').val();
        selectedSpectrumId = (val == undefined ? selectedSpectrumId : val);
    updateSelectors();


}







///////////////////////////
function updateSelectors(){


    var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");

    console.log(coid + "|should have been multivalue = " + inObjId)
    var url = "qpbg_msgfplusinteractive://::protein_summary_"+inObjId+".1.json//poid="+inObjId;
        url += "&coid="+coid+"&runprogram=2&proteinid="+selectedProteinId+"&peptideid="+selectedPeptideId;
    url += "&spectrumid="+selectedSpectrumId;

        vjDS["dsText1"].reload(url,true);
        vjDS["dsText1"].parser = function (ds, text) {
        //console.log(this.data);
        var jsonObj = JSON.parse(this.data);
        var proteinObjList = jsonObj["proteinlist"];
            selectedProteinId =(jsonObj["selectedproteinid"] ? jsonObj["selectedproteinid"] : 
                selectedProteinId);
        var selector1 = '<select name=protein multiple style="width:250px;height:125px;"' + 
                    ' onchange="proteinSelectorHandler();">';
        for (var i in proteinObjList){
            var proteinAc = proteinObjList[i]["ac"];
            var proteinId = proteinObjList[i]["id"];
            var s = (proteinId == selectedProteinId ? "selected" : "");
            selectedProteinObj = (proteinId == selectedProteinId ? proteinObjList[i] : 
                        selectedProteinObj);
            var label = proteinAc + ' (' + proteinId + ')';
            selector1 += '<option value="'+proteinId+'" '+s+'>'+label +'</option>';
        }
        selector1 += '</select>';

        peptideObjList = jsonObj["peptidelist"];
        proteinTracks = jsonObj["tracks"];

        selectedPeptideId =(jsonObj["selectedpeptideid"] ? jsonObj["selectedpeptideid"] : 
                    selectedPeptideId);
        selectedPeptideSeq = (selectedPeptideId in peptideObjList ? 
                    peptideObjList[selectedPeptideId]["seq"] : '');
        
    
        var selector2 = '<select name=peptide multiple style="width:400px;height:125px;"' + 
                    ' onchange="peptideSelectorHandler();">';
        for (var peptideId in peptideObjList){
                    peptideSeq = peptideObjList[peptideId]["seq"];
            var s = (peptideId == selectedPeptideId ? "selected" : "");
            var label = peptideSeq + ' (' + peptideId + ')';
            selector2 += '<option value="'+peptideId+'" '+s+'>'+label+'</option>';
        }
        selector2 += '</select>';

        var spectrumObjList = jsonObj["spectrumlist"];
                var selector3 = '<select name=spectrum multiple style="width:400px;height:125px;" ' + 
                    ' onchange="updatePlotContainer();">';
                for (var spectrumId in spectrumObjList){
                        var rt = spectrumObjList[spectrumId]["rt"];
            var evalue = spectrumObjList[spectrumId]["evalue"];
             var s = (spectrumId == selectedSpectrumId ? "selected" : "");
                        var label = spectrumId + ' (evalue:'+evalue+')';
                        selector3 += '<option value="'+spectrumId+'" '+s+'>'+label+'</option>';
                }
                selector3 += '</select>';

        $("#selector1").html(selector1);
        $("#selector2").html(selector2);
        $("#selector3").html(selector3);
        $("#browsetable").css("display", "block");
        
        //Clear level2 tabs and render protein view tab
        clearTabs();
            plotProteinGraph();


        return text;
    };


}


/////////////////////////
function updatePlotContainer(){


    clearTabs();

    var coid = docLocValue("id");
    var poId = algoProcess.getValue("spectrumfile");
    var fragPpm = algoProcess.getValue("precursormasstolerance");
    var val = $('select[name=spectrum]').val();
    var scanId = val.toString();


    var tabId = "level2_scan_" + scanId;
    tabId = tabId.replace(".", "_");

    var tabName = "Scan-" + scanId;
    tabName = tabName.replace(".", "_");
    if (tabId in tabList){
        algoWidgetObj.openTab(tabId);
        return;
    }


    var dsId1 = "ds1_" + scanId;
    var dsId2 = "ds2_" + scanId;
        vjDS.add("Loading .......",dsId1,"static://\n");
    vjDS.add("Loading .......",dsId2,"static://\n");        

        var myplot = new vjHTMLView({
                                data: dsId1
                                ,type: "column"
                        });
    
    var tmp = {tabId: tabId, tabName: tabName,
                position: {posId: 'pos_2', top:'50%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {instance: [myplot]},
                autoOpen: ["computed"],
        inactive:false
    };
    tabList[tabId] = 1;
        algoWidgetObj.addTabs([tmp], "results");
    algoWidgetObj.openTab(tabId);



    var fileName = "spectrumplotdata.scan."+scanId+".csv";
    var url = "qpbg_msgfplusinteractive://::"+fileName+"//scanid="+scanId+"&coid="+coid+"&poid="+poId;
        url += "&runprogram=3&proteinid=" + selectedProteinId + "&peptideid="+selectedPeptideId;
    url += "&fragppm="+fragPpm

    vjDS[dsId2].reload(url,true);
    vjDS[dsId2].dsId1 = dsId1;
    vjDS[dsId2].tabId = tabId;
    vjDS[dsId2].parser = function (ds,text){    
        var plotDivId = this.dsId1 + "_plot";
        //var plotHtml = '<DIV id="'+plotDivId+'" style="'+divStyle+'">Hi there</DIV>';
          var aaList = selectedPeptideSeq.split("");
        var plotTitle = aaList.join("-");
                        
        var style1 = 'position:absolute;left:0px;top:30px;width:100%;height:25px;z-index:3;';
        style1 += 'font-size:12px;color:#777;text-align:center;';
        var style2 = 'position:absolute;left:0px;top:0px;width:100%;height:200px;z-index:2;';
        var plotHtml = '<DIV>'+
                '<DIV style="'+style1+'">'+plotTitle+'</DIV>'+
                '<DIV style="'+style2+'">'+
                '<table width=98% height=100% border=0>' + 
                '<tr><td id="'+plotDivId+'">Hello from tab: '+ this.tabId +'</td></tr>' +
                '</table>' +
                '</DIV>'+
                '</DIV>';
        vjDS[this.dsId1].reload("static://"+plotHtml,true);
        var plotData = [];
        var lines = this.data.split("\n");
        for (var i=0; i < lines.length; i++){
            if(lines[i]){
                //console.log(lines[i]);
                var values = lines[i].split(",");
                values[3] = (values[3] == "NA" ? "" : values[3]);
                plotData.push({"x":parseFloat(values[0]), "y1":parseFloat(values[1]),
                        "style":values[2], "annotation":values[3]});
            }
        }
        var xTitle = 'm/z';
            var yTitle = 'Intensity';
            var tickFreq = 'showTextEvery:1';
        drawSingleSeries(plotData,plotDivId, xTitle, yTitle, tickFreq);
        return text;
    }


}


/////////////////
function plotProteinGraph(){

    var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");
        
        var dsName = "dsText_" + inObjId;
        vjDS.add("Loading .......",dsName,"static://1,2,3,4,5\n");
        
        var mytext = new vjHTMLView({
                                data: dsName
                                ,type: "column"
                        });     
        var url = "qpbg_msgfplusinteractive://::protein_summary_"+inObjId+".1.json//poid="+inObjId
        url += "&coid="+coid+"&runprogram=2";
        //vjDS[dsName].reload(url,true);


    var style1 = 'position:relative;width:840px;height:300px;background:#fff;';
    var style21 = 'position:absolute;left:20px;top:5px;width:20px;height:15px;' +
                        'background:#fff;font-size:11px;text-align:left;';
    var style22 = 'position:absolute;left:20px;top:20px;width:800px;height:20px;' + 
            'background:#DAA520;font-size:11px;text-align:center;padding:3px 0 0 0;';
    var style23 = 'position:absolute;right:20px;top:5px;width:20px;height:15px;' +
                        'background:#fff;font-size:11px;text-align:right;';
        
    var cn = '<DIV style="'+style1+'">' +
        '<DIV style="'+style21+'">1</DIV>' +
        '<DIV style="'+style22+'">'+selectedProteinObj["ac"]+'</DIV>' +
        '<DIV style="'+style23+'">'+selectedProteinObj["length"]+'</DIV>';

    
    var scaleFactor =  800/selectedProteinObj["length"];
    
    var style = 'position:absolute;height:10px;padding:0px 0px 0px 0px;' +
                        'background:#F0E68C;font-size:8px;text-align:center;vertical-align:middle;';

    for (var i=0; i < proteinTracks.length; i ++){
        for (var j=0; j < proteinTracks[i].length; j ++){
            var peptideId = proteinTracks[i][j];
            //var pepLbl = peptideId.split("|")[1]
            var pepLbl = '';
            var start = 20 + parseInt(peptideId.split("|")[1].split("-")[0] * scaleFactor);
                    var end =  20 + parseInt(peptideId.split("|")[1].split("-")[1] * scaleFactor);
                    var width = (end - start + 1);
                    var top = 45 + i * 25;
            var s = style + 'left:'+start+'px;width:'+width+'px;top:' + top + 'px;';
                    s += (peptideId == selectedPeptideId ? "background:red;" : "");
                    cn += '<DIV style="'+s+'">'+pepLbl+'</DIV>';
            //console.log(i + " " + j + " "  + proteinTracks[i][j]);
        }
    }
    cn += '</DIV>';

    var htmlTable = '<table width=100% border=0>' +
        '<tr height=200><td valign=top><br>'+cn+'</td></tr>' + 
        '</table>';
        vjDS[dsName].reload("static://"+htmlTable,true);

 
        var tabId = "level2_protein_" + inObjId;
        var tabName = selectedProteinObj["ac"];
        var tmp = {tabId: tabId, tabName: tabName,
                position: {posId: 'pos_2', top:'50%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {instance: [mytext]},
                autoOpen: ["computed"]
        };      
    tabList[tabId] = 1;
        algoWidgetObj.addTabs([tmp], "results");


}


////////////////////////////
function showProteinView (viewer,node,irow,icol) {

        var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");
    plotProteinGraph();
}



////////////////////////////////////////
function showSampleGraphs (viewer,node,irow,icol) {


        clearTabs();

        var sampleName = node['sampleName'];
        var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");

    var dsId1 = "ds1_sample";
        vjDS.add("Loading .......",dsId1,"static://");
        var myHtml1 = new vjHTMLView({data: dsId1, type: "column"});
       
    var tabId = "level2_sampletab"
        var tabName = "Sample plots"
    var tmp = {tabId: tabId, tabName: tabName,
                position: {posId: 'pos_2', top:'50%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {instance: [myHtml1]},
                autoOpen: ["computed"],
                inactive:false
        };
        tabList[tabId] = 1;
        algoWidgetObj.addTabs([tmp], "results");
        algoWidgetObj.openTab(tabId);



    var dsId2 = "ds2_sample"
        vjDS.add("Loading .......",dsId2,"static://");
    var url = "qpbg_tblqryx4://sampleplot."+sampleName+".csv//objs="+coid
        vjDS[dsId2].reload(url,true);
        vjDS[dsId2].dsId1 = dsId1;
        vjDS[dsId2].tabId = tabId;
        vjDS[dsId2].sampleName = sampleName;
    vjDS[dsId2].parser = function (ds,text){
        var plotDivId1 = this.dsId1 + "_plot1";
        var plotDivId2 = this.dsId1 + "_plot2";
                var plotTitle = 'Hi there'
                var style1 = 'position:absolute;left:0px;top:0px;width:100%;height:300px;z-index:2;';
                var plotHtml = '<DIV>'+
                                '<DIV style="'+style1+'">'+
                                '<table width=98% height=100% border=0>' +
                                '<tr>' + 
                '<td id="'+plotDivId1+'" width=50%>plot-1</td>' + 
                '<td id="'+plotDivId2+'">plot-2</td>' +
                '</tr>' +
                                '</table>' +
                                '</DIV>'+
                                '</DIV>';
                vjDS[this.dsId1].reload("static://"+plotHtml,true);
                var plotData1 = [];
                var plotData2 = [];
        var lines = this.data.split("\n");
        for (var i=0; i < lines.length; i++){
                        if(lines[i]){
                                var values = lines[i].split(",");
                                //console.log(lines[i]);
                plotData2.push({"x":parseFloat(values[1]), "y1":parseFloat(values[2]),"style":""});
                if (values[3] != ''){
                    plotData1.push({"x":parseFloat(values[3]), "y1":parseFloat(values[4]),
                        "y2":parseFloat(values[5]), "style":""});
                }
                        }
                }
                var plotTitle = 'Score density for ' + this.sampleName
        var xTitle = 'score';
                var yTitle = 'count';
                var tickFreq = 'showTextEvery:1';
                var colNames = ['score', 'decoy', 'real']
        drawAreaPlot(plotData1,colNames, plotDivId1, plotTitle, xTitle, yTitle, tickFreq);
        
        var plotTitle = 'Retention time vs m/z for ' + this.sampleName
        var xTitle = 'retention time';
                var yTitle = 'm/z';
                var tickFreq = 'showTextEvery:1';
        drawScatterPlot(plotData2,plotDivId2, plotTitle, xTitle, yTitle, tickFreq);
        return text;
        }




}


////////////////////////////////////////
function showSampleGraphsOld (viewer,node,irow,icol) {

    clearTabs();


    var sampleName = node['sampleName'];
    var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");


    var url = "qpbg_tblqryx4://density."+sampleName+".csv//objs="+coid
    vjDS.add("Loading ..... ", "dsPlot1", url);
    var myplot1 = new vjGoogleGraphView({
                                data: "dsPlot1", type: "area",
                                series:[{name:"x"}, {name:"y2"}]
                });
    vjDS["dsPlot1"].reload(url,true);
            

    var url = "qpbg_tblqryx4://rtvsmz."+sampleName+".csv//objs="+coid
        vjDS.add("Loading ..... ", "dsPlot2", url);
    var myplot2 = new vjGoogleGraphView({
                                data: "dsPlot2", type: "scatter",
                                series:[{name:"rt"}, {name:"mz"}]
                });
          vjDS["dsPlot2"].reload(url,true);

 
    var tabarray = []
    var posId = "scoredistribution_pos"
        var tabId = "level2_scoredistribution_" + sampleName;
    var tabName = "Score distribution"
    tabarray[0] = {tabId: tabId, tabName: tabName,
                    position: {posId: posId, top:'50%', bottom:'100%', left:'20%', right:'100%'},
                    viewerConstructor: {instance: [myplot1]},
                    autoOpen: ["computed"]
        };
    tabList[tabId] = 1;
    
        var tabId = "level2_retentiontime_" + sampleName;
    var tabName = "Retention time vs mz"
        
    tabarray[1] = {tabId: tabId, tabName: tabName,
                                position: {posId: posId, top:'50%', bottom:'100%', left:'20%', right:'100%'},
                                viewerConstructor: {instance: [myplot2]},
                                autoOpen: ["computed"]
                };

    tabList[tabId] = 1;
    algoWidgetObj.addTabs(tabarray, "results");

}


//////////////////////
function clearTabs(){

    for (var tabId in tabList){
        algoWidgetObj.removeTabs(tabId, "results");
    }
    return;

}
