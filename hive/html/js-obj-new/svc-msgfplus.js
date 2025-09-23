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
var modHash = {};

 
vjHO.register('svc-msgfplus').Constructor=function (){


    function loadedCallbackAndUpdateSelectors (a,b,c,d) {
            parentLoadedCallback(a,b,c,d);
            updateSelectors();
    }
   
 
    this.fullview=function(node, whereToAdd){
        var coid = docLocValue("id");
       
        var main_objCls = "obj-MSGF"+Math.random();
            vjObj.register(main_objCls,node);

        var style = 'font-size:12px;color:#777;display:none;background:#fff;border-top:1px solid #ccc;';
        var ttlstyle = 'padding:0 0 0 20px;font-size:12px;font-weight:bold';
        var htmlTable = '<table style="display:none;background:#f1f1f1;" id=browsetable  border=0>' +
            '<tr><td colspan=3>&nbsp;</td></tr>' +
            '<tr><td style="'+ttlstyle+'" valign=bottom>Select protein</td></tr>' +
            '<tr><td style="padding:0 0 0 20px;" id=selector1 valign=top></td></tr>' +
            '<tr height=20><td style="'+ttlstyle+'" valign=bottom>Select pepetide</td></tr>' +
            '<tr><td style="padding:0 0 0 20px;" id=selector2 valign=top></td></tr>' + 
            '<tr height=20><td style="'+ttlstyle+'" valign=bottom>Select scan</td></tr>' +
            '<tr><td style="padding:0 0 0 20px;" id=selector3 valign=top></td></tr>' +
            '<tr><td colspan=3>&nbsp;</td></tr>' +
                        '</table>';



        vjDS.add("Loading ..... ", "dsTable1", "qpbg_tblqryx4:
         vjDS.add("Loading ..... ", "dsTable2", "qpbg_tblqryx4:
         vjDS.add("Loading ..... ", "dsText1", "static:
        vjDS.add("Loading ..... ", "dsText2", "static:

    
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

        var mypanel1 = new vjBasicPanelView({
                    data: ['dsVoid',"dsTable1"]
                    ,formObject: document.forms[formName]
                    ,parentObjCls: main_objCls
            });

        var mypanel2 = new vjBasicPanelView({
            data: ['dsVoid',"dsTable2"]
            ,formObject: document.forms[formName]
            ,parentObjCls: main_objCls
        });

        vjDS.add("Loading ..... ", "dsTableDownload", "static:
        var downloadObjList = [
            {
                name : 'download', title : 'Download',icon : 'download',align : 'left',
                order : '1',description : 'Download content',path : "/download"
            },
            {
                name : 'downloadall', path : "/download/all", showTitle : true, title : 'All',
                icon : 'download', align : 'left',  description : 'Download full content',
                 url : "javascript:var url=vjDS['$(dataname)'].url; url=urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(url,'start','-'),'cnt','-'),'down','-'); vjDS['dsTableDownload'].reload(url, true,'download');"        
            },
            {
                            name : 'downloadpage',path : "/download/page",showTitle : true,title : 'Page',
                icon : 'download', align : 'left', description : 'Download content of the current page',
                url : "javascript:var url=vjDS['$(dataname)'].url; vjDS['dsTableDownload'].reload(url, true,'download');"
            }
        ];
        mypanel1.rows = mypanel1.rows.concat(downloadObjList);
        mypanel2.rows = mypanel2.rows.concat(downloadObjList);


                var mytable2 = new vjTableView({
                        data: "dsTable2"
                        ,bgColors : [ '#f2f2f2', '#ffffff' ]
                        ,formObject: document.forms[formName]
                        ,checkable:false
                        ,cols:[
                                {name:"samplename", order:1}
                                ,{name:"scannum", order:2}
                                ,{name:"spectrumid",order:3, hidden:true}
                                ,{name:"precursor_mz_experimental", order:4}
                ,{name:"precursor_mz_calculated", order:5}
                ,{name:"precursore_mz_error", order:6}
                ,{name:"isotopeerror", order:7}
                ,{name:"chargestate", order:8}
                ,{name:"proteinac", order:9}
                ,{name:"peptideid", order:10}
                                   ,{name:"peptideseq", order:11}
                ,{name:"modifiedpeptideseq", order:12}
                ,{name:"qvalue", order:13}
                ,{name:"evalue", order:14}
                ,{name:"modifications", order:15}
                                ,{name:"proteindesc", order:16}    
                ,{name:"fileindex", hidden:true}
                            ,{name:"peptideindex", hidden:true}
                            ,{name:"proteinid", hidden:true}
                            ,{name:"proteinlen", hidden:true} 
            ]
                        ,precompute:"if (node.Rawfile) { var tmp = node.Rawfile; tmp = tmp.split('\\\\\'); node.Rawfile = tmp[tmp.length -1];} if (node.Use == 'True') {node.checked=true;}"
                        ,parentObjCls: main_objCls
                });

        viewStructureToAdd = [];
        viewStructureToAdd[0] = {tabId: 'level1_tab3', tabName: "Browse identifications",
                        position: {posId: 'pos_1', top:'0', bottom:'50%', left:'20%', right:'75%'},
                        viewerConstructor: {instance: [mytext2]},
                        autoOpen: ["computed"],
                        inactive:false
                };
        viewStructureToAdd[1] = {tabId: 'level1_tab2', tabName: "Identifications list",
                        position: {posId: 'pos_1', top:'0', bottom:'100%', left:'20%', right:'75%'},
                        viewerConstructor: {instance: [mypanel2, mytable2]},
                        autoOpen: ["computed"],
                        inactive:true
                };
        viewStructureToAdd[2] = {tabId: 'level1_tab1', tabName: "Samples",
                        position: {posId: 'pos_1', top:'0', bottom:'50%', left:'20%', right:'75%'},
                        viewerConstructor: {instance: [mypanel1, mytable1]},
                        autoOpen: ["computed"],
            inactive:true
                };

        algoWidgetObj.addTabs(viewStructureToAdd, "results");
        updateSelectors();
                   if(algoProcess && algoProcess.callbackLoaded) {
               parentLoadedCallback = algoProcess.callbackLoaded;
                }
        algoProcess.callbackLoaded = loadedCallbackAndUpdateSelectors;
    };
    
}

function proteinSelectorHandler(){
    var val = $('select[name=protein]').val();
        selectedProteinId = (val == undefined ? selectedProteinId : val);

    selectedPeptideId = '';
    selectedSpectrumId = '';        
    
    updateSelectors();
    
}

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







function updateSelectors(){


    var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");

    var url = "qpbg_msgfplusinteractive:
        url += "&coid="+coid+"&runprogram=2&proteinid="+selectedProteinId+"&peptideid="+selectedPeptideId;
    url += "&spectrumid="+selectedSpectrumId;

        vjDS["dsText1"].reload(url,true);
        vjDS["dsText1"].parser = function (ds, text) {
        var jsonObj = JSON.parse(this.data);
        var proteinObjList = jsonObj["proteinlist"];
        proteinObjList = proteinObjList.sort(function(a, b){return b.pepcount - a.pepcount;});
        selectedProteinId =(jsonObj["selectedproteinid"] ? jsonObj["selectedproteinid"] : 
                proteinObjList[0]);
        var selector1 = '<select name=protein multiple style="width:100%;height:75px;"' + 
                    ' onchange="proteinSelectorHandler();">';
        for (var i=0; i < proteinObjList.length; i++){
            var proteinAc = proteinObjList[i]["ac"];
            var proteinId = proteinObjList[i]["id"];
            var proteinDesc = proteinObjList[i]["description"];
            if(proteinId != undefined && proteinAc.substring(0,4) != "XXX_"){
                var s = (proteinId == selectedProteinId ? "selected" : "");
                selectedProteinObj = (proteinId == selectedProteinId ? proteinObjList[i] : 
                            selectedProteinObj);
                var label = proteinAc + ' [' + proteinDesc + '] ('+proteinObjList[i]["pepcount"]+' peptides)';
                selector1 += '<option value="'+proteinId+'" '+s+'>'+label +'</option>';
            }
        }
        selector1 += '</select>';

        peptideObjList = jsonObj["peptidelist"];
        proteinTracks = jsonObj["tracks"];

        selectedPeptideId =(jsonObj["selectedpeptideid"] ? jsonObj["selectedpeptideid"] : 
                    selectedPeptideId);
        selectedPeptideSeq = (selectedPeptideId in peptideObjList ? 
                    peptideObjList[selectedPeptideId]["seq"] : '');

        var pepid2startpos = {};
        for (var peptideId in peptideObjList){
            pepid2startpos[peptideId] = peptideObjList[peptideId]["startpos"];
        }
        var sortedPepIdList = Object.keys(pepid2startpos).sort(function(a, b){return pepid2startpos[a] - pepid2startpos[b];});
        var selector2 = '<select name=peptide multiple style="width:100%;height:75px;"' + 
                    ' onchange="peptideSelectorHandler();">';
                for (var j=0; j < sortedPepIdList.length; j++){
            var peptideId = sortedPepIdList[j];
            peptideSeq = peptideObjList[peptideId]["seq"];
            var parts = peptideId.split("|");
            var s = (peptideId == selectedPeptideId ? "selected" : "");
            var label = peptideSeq;
            if ("mods" in peptideObjList[peptideId] > 0){
                var aaList = peptideSeq.split("");
                label = "";
                for(var k=0; k < aaList.length; k++){
                    var kk = (k+1).toString();
                    var mShift = (kk in peptideObjList[peptideId]["mods"] ? 
                                 peptideObjList[peptideId]["mods"][kk] : ""); 
                    label += aaList[k] + mShift;
                    if(peptideId == selectedPeptideId && mShift != ""){
                        modHash[kk] = mShift;
                    }
                }
            }    
            label += ' (' + parts[1] + ')';
            selector2 += '<option value="'+peptideId+'" '+s+'>'+label +'</option>';
        }
        selector2 += '</select>';


        var spectrumObjList = jsonObj["spectrumlist"];
        var spectrumid2evalue = {};
        for (var spectrumId in spectrumObjList){
                        spectrumid2evalue[spectrumId] = spectrumObjList[spectrumId]["evalue"];
                }
        var sortedSpectrumIdList = Object.keys(spectrumid2evalue).sort(function(a, b){
                return spectrumid2evalue[a] - spectrumid2evalue[b];});
                var selector3 = '<select name=spectrum multiple style="width:100%;height:75px;" ' + 
                    ' onchange="updatePlotContainer();">';
        var pepIndex = selectedPeptideId.split("_")[0].toLowerCase();
        for (var j=0; j < sortedSpectrumIdList.length; j++){
                        var spectrumId = sortedSpectrumIdList[j] ;
                   var rt = spectrumObjList[spectrumId]["rt"];
            var sampleName = spectrumObjList[spectrumId]["samplename"];
            var evalue = spectrumObjList[spectrumId]["evalue"];
            var precursor = spectrumObjList[spectrumId]["precursor"];
             var precursorerror = spectrumObjList[spectrumId]["precursorerror"];
            var s = (spectrumId == selectedSpectrumId ? "selected" : "");
                        var label =  spectrumId + ' (eValue:'+evalue+ ', precursor:' + precursor;
                        label +=  ', precursorerror:' + precursorerror + ')';
            var value = pepIndex + "_" + spectrumId;
            selector3 += '<option value="'+value+'" '+s+'>'+label+'</option>';
                }
                selector3 += '</select>';

        $("#selector1").html(selector1);
        $("#selector2").html(selector2);
        $("#selector3").html(selector3);
        $("#browsetable").css("display", "block");
        
        clearTabs();
            plotProteinGraph();


        return text;
    };


}


function updatePlotContainer(){

    clearTabs();

    var coid = docLocValue("id");
    var poId = algoProcess.getValue("spectrumfile");
    var fragPpm = algoProcess.getValue("precursormasstolerance");
    var val = $('select[name=spectrum]').val();
    var scanId = val.toString();



    var tabId = "level2_scan_" + scanId;
    tabId = tabId.replace(".", "_");
    tabId = tabId.replace("-", "_");


    var tabName = scanId;
    tabName = tabName.replace(".", "_");
    tabName = tabName.replace("-", "_");
    if (tabId in tabList){
        algoWidgetObj.openTab(tabId);
        return;
    }



    var dsId1 = "ds1_" + scanId;
    var dsId2 = "ds2_" + scanId;
    dsId1 = dsId1.replace("-", "_");
    dsId2 = dsId2.replace("-", "_");

        vjDS.add("Loading .......",dsId1,"static:
    vjDS.add("Loading .......",dsId2,"static:

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
    var url = "qpbg_msgfplusinteractive:
        url += "&runprogram=3&proteinid=" + selectedProteinId + "&peptideid="+selectedPeptideId;
    url += "&fragppm="+fragPpm

    vjDS[dsId2].reload(url,true);
    vjDS[dsId2].dsId1 = dsId1;
    vjDS[dsId2].tabId = tabId;
    vjDS[dsId2].parser = function (ds,text){    
        var plotDivId = this.dsId1 + "_plot";

        var aaList = selectedPeptideSeq.trim().split("");
        var seenFragPos = {};    
                var plotData = [];
                var lines = this.data.split("\n");
                for (var i=0; i < lines.length; i++){
                        if(lines[i]){
                                var values = lines[i].split(",");
                                values[3] = (values[3] == "N/A" ? "" : values[3]);
                                plotData.push({"x":parseFloat(values[0]), "y1":parseFloat(values[1]),
                                                "style":values[2], "annotation":values[3]});
                            if(values[3] != ""){
                    values[3] = values[3].replace("*", "");
                    values[3] = values[3].replace("o", "");
                    var parts = values[3].split("+"); 
                    if(values[3].substring(0, 1) == "b"){
                        var pos = parseInt(parts[0].substring(1));
                        seenFragPos[pos] = 1;
                    }
                    else if (values[3].substring(0, 1) == "y"){
                        var pos = aaList.length - parseInt(parts[0].substring(1));
                                                seenFragPos[pos] = 1;
                    }
                }
            }
                }

        var plotTitle = '<table align=center border=0><tr>';
        for (var i=0; i < aaList.length; i++){
            var style1 = '';
                        var style2 = '';
            if(seenFragPos[i]){ 
                style1 = 'border-top:1px solid red;'; 
                style1 = (i == 0  ? "" : style1);
            }
            if(seenFragPos[i+1]){
                            style2 = 'border-right:1px solid red;';
                            style2 = (i == aaList.length - 1 ? "" : style2);
            }
            
            var ii = (i+1).toString();
            if (ii in modHash){
                aaList[i] = aaList[i] + "<sup><font color=red><b>+</b></font></sup>"
            }                             
            plotTitle += '<td rowspan=2 style="'+style1+'">'+ aaList[i] + 
                    '</td><td style="'+style2+'">&nbsp;&nbsp;</td>';
                }
        plotTitle += '</tr><tr height=10>';
        for (var i=0; i < aaList.length; i++){
            var style2 = '';
            if(seenFragPos[i + 1]){
                style2 = 'border-right:1px solid blue;border-bottom:1px solid blue;';
                style2 = (i == aaList.length - 1 ? "" : style2);
            }
            plotTitle += '<td style="'+style2+'">&nbsp;&nbsp;</td>';
        }
        plotTitle += '</tr></table>';
                        
        var style1 = 'position:absolute;left:0px;top:10px;width:100%;height:25px;z-index:3;';
        style1 += 'font-size:12px;color:#777;text-align:center;';
        var style2 = 'position:absolute;left:0px;top:0px;width:100%;height:400px;z-index:2;';
        var plotHtml = '<DIV>'+
                '<DIV style="'+style1+'">'+plotTitle+'</DIV>'+
                '<DIV style="'+style2+'">'+
                '<table width=98% height=100% border=0>' + 
                '<tr><td id="'+plotDivId+'">Hello from tab: '+ this.tabId +'</td></tr>' +
                '</table>' +
                '</DIV>'+
                '</DIV>';
        vjDS[this.dsId1].reload("static:
        
        var xTitle = 'm/z';
            var yTitle = 'Intensity';
            var tickFreq = 'showTextEvery:1';
        drawSingleSeries(plotData,plotDivId, xTitle, yTitle, tickFreq);
        return text;
    }


}


function plotProteinGraph(){

    var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");
        
        var dsName = "dsText_" + inObjId;
        vjDS.add("Loading .......",dsName,"static:
        
        var mytext = new vjHTMLView({
                                data: dsName
                                ,type: "column"
                        });     
        var url = "qpbg_msgfplusinteractive:
        url += "&coid="+coid+"&runprogram=2";


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
            var pepLbl = '';
            var start = 20 + parseInt(peptideId.split("|")[1].split("-")[0] * scaleFactor);
                    var end =  20 + parseInt(peptideId.split("|")[1].split("-")[1] * scaleFactor);
                    var width = (end - start + 1);
                    var top = 45 + i * 25;
            var s = style + 'left:'+start+'px;width:'+width+'px;top:' + top + 'px;';
                    s += (peptideId == selectedPeptideId ? "background:red;" : "");
                    cn += '<DIV style="'+s+'">'+pepLbl+'</DIV>';
        }
    }
    cn += '</DIV>';

    var htmlTable = '<table width=100% border=0>' +
        '<tr height=200><td valign=top><br>'+cn+'</td></tr>' + 
        '</table>';
        vjDS[dsName].reload("static:


    clearTabs();
 
    var protId = selectedProteinObj["ac"];
    if (protId.indexOf("|") != -1){
        protId = protId.split("|")[1];
    }

    var tabId = "level2_protein_" + protId;    
    var tabName = protId;
    var tmp = {tabId: tabId, tabName: tabName,
                position: {posId: 'pos_2', top:'50%', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {instance: [mytext]},
                autoOpen: ["computed"]
        };      
    tabList[tabId] = 1;
    algoWidgetObj.addTabs([tmp], "results");


}


function showProteinView (viewer,node,irow,icol) {

        var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");
    plotProteinGraph();
}



function showSampleGraphs (viewer,node,irow,icol) {


        clearTabs();

        var sampleName = node['sampleName'];
        var coid = docLocValue("id");
        var inObjId = algoProcess.getValue("spectrumfile");

    var dsId1 = "ds1_sample";
        vjDS.add("Loading .......",dsId1,"static:
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
        vjDS.add("Loading .......",dsId2,"static:
    var url = "qpbg_tblqryx4:
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
                vjDS[this.dsId1].reload("static:
                var plotData1 = [];
                var plotData2 = [];
        var lines = this.data.split("\n");
        for (var i=0; i < lines.length; i++){
                        if(lines[i]){
                                var values = lines[i].split(",");
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




function clearTabs(){

    return;

    for (var tabId in tabList){
        algoWidgetObj.removeTabs(tabId, "results");
    }
    return;

}
