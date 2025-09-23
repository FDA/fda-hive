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
javaScriptEngine.include("js/vjTreeSeries.js");
javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");


function vjHiveseqView(viewer) {
    if(viewer.icon===undefined)viewer.icon= 'list';
    if(viewer.bgColors===undefined)viewer.bgColors= ['#f2f2f2','#ffffff'];
    if(viewer.name===undefined)viewer.name= 'list';
    if(viewer.geometry===undefined)viewer.geometry= {width:"100%"};
    if(viewer.cols===undefined)viewer.cols= [{name:'seq', title:'Sequence', maxTxtLen:300, type: 'pre'}];
    if(viewer.maxTxtLen===undefined)viewer.maxTxtLen= 32;

    vjTableView.call(this, viewer);
}

function vjSequenceListViewer(viewer)
{

    var cntViewersIn = (vjDV[dvname].tabs && vjDV[dvname].tabs[tabname]) ? vjDV[dvname].tabs[tabname].viewers.length : 0;

    var dsurl = "http:
    if (url!==undefined && url.indexOf("http:
    if (!active) active = "tree";
    if (searchKey) dsurl += "&prop_val=" + searchKey;
    var hideDBactions=[{name:'upload',hidden:true},
                       {name:'create',hidden:true},
                       {name:'convert',hidden:true}];
    if(hidebuttons)hideDBactions=hidebuttons.concat(hideDBactions);
    vjPAGE.initListAndTreeViewerTab(dsurl, dvname, tabname, tabico, recordviewer, formname, fullpanel, active, hideDBactions, selectCallback, bgClrMap,isdrag);

    var viewer = vjDV.locate(dvname + "." + tabname + "." + (cntViewersIn + 1));
    viewer.cols = ([
           { name: new RegExp(/.*/), hidden: true },
           {name: '^name', hidden: false, title: 'Name', wrap: false, order: 2 },
           { name: 'id', hidden: false, title: 'ID', order: 1 },
           { name: 'rec-count', hidden: false, title: "Records", align: "left", type: "largenumber", order: 3 },
           { name: '^size', hidden: false, title: "Size", align: "right", type: "bytes", order: 4 },
           { name: 'created', hidden: false, title: "Created", align: "right", type: "datetime", order: 5 },
           { name: 'search', align: 'right', isSubmitable: true }
           ]);
    viewer.iconSize = 0;
    var tviewer = viewer;
    if (checkcallback) {
        viewer = vjDV.locate(dvname + "." + tabname + "." + (cntViewersIn + 1)); viewer.checkCallback = checkcallback;
        viewer = vjDV.locate(dvname + "." + tabname + "." + (cntViewersIn + 2)); viewer.checkLeafCallback = checkcallback; viewer.checkBrunchCallback = checkcallback;
    }



    var viewerPanel = vjDV.locate(dvname + "." + tabname + "." + cntViewersIn);
    var objtype = (searchKey && searchKey.indexOf("genomic") == 0 ? "subject" : "query");
    viewerPanel.objType=objtype;
    var addCmd = [
            { name: 'hiveseq', align: 'left', order: -3, target: 'hiveseq', icon: 'hiveseq', showTitle: false, title: 'hive-seq', description: 'Create a new composite sequence file: HIVESEQ', url: '?cmd=dna-hiveseq&ids=$(ids)' },
            { name: 'separ', type: 'separator', order: -1 }
    ];
    viewerPanel.rows = viewerPanel.rows.concat(addCmd);
    return tviewer;

}

function vjHiveseqPanelView(viewer)
{
    var dsName='';
    if(viewer.iconSize === undefined) viewer.iconSize=24;
    if(viewer.hideViewerToggle) viewer.hideViewerToggle=true;
    if(viewer.rows === undefined) viewer.rows = new Array();
    if(viewer.data === undefined){
        alert('DEVELOPER WARNING: UNDEFINED data for HiveseqPanelView');
    }
    else{
        var dataArr=verarr(viewer.data);
        var data=dataArr[1];
        if(data)dsName=data;
        this.formObject = viewer.formObject;
    }
    if (viewer.QC) 
    {
        viewer.rows=viewer.rows.concat([
          {name:'launchQC', type:'button' ,align:'left', hidden: (viewer.hideQCLauncher != undefined) ? viewer.hideQCLauncher : true ,isSubmitable:true, title:'Launch QC', description: 'QC launcher', url: launchQC_Screen, dsCmdLink: "launchSvc&key=dna-qc&query=" + viewer.objId}
        ]);
    }
    else 
    {    
        viewer.rows=viewer.rows.concat([
               {name:'pager', align:'right', icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
               {name:'search', align:'right', isSubmitable:true, type:' search', title:'Search', description: 'search sequences by ID', url:"?cmd=objFile&ids=$(ids)"}
               ]);
    }
    vjPanelView.call(this,viewer);
}

function launchQC_Screen(viewer, node) {
    var dsname = "ds" + node.name; 
    if (!vjDS[dsname]) {
        vjDS.add("Loading " + node.description, dsname, "static:
    }
    var url = "http:
    vjDS[dsname].launched = false;
    vjDS[dsname].checkRequest= false;
    vjDS[dsname].dependentData = viewer.dependentData;
    vjDS[dsname].progressView = viewer.progressView;
    vjDS[dsname].objId = viewer.objId;
    vjDS[dsname].reload(url,true);
    
    vjDS[dsname].parser = checkLauncher;
    vjDS[dsname].intervalID = setInterval(checkLauncher,5000, vjDS[dsname]);
    viewer.tree.findByName(node.name).inactive = true;
    viewer.refresh();
}

function checkLauncher(ds, text){
    var launchQCds = ds;
    if (!launchQCds.launched) {
        alert ("Your request has been submitted to the system.\nThe graph will show up when the computation is finished.");
        var reqid = launchQCds.data.split("=")[1];
        launchQCds.reqid = reqid;
        if (launchQCds.name.indexOf("launchQC")!=-1) {
            if (typeof(storage) !== undefined)
                localStorage.setItem("" + launchQCds.objId +"_QC_request",reqid);
        }
        else if (launchQCds.name.indexOf("DnaScreen")!=-1) {
            if (typeof(storage) !== undefined) 
                localStorage.setItem("" + launchQCds.objId +"_Screen_request",reqid);
        }
    }
    else {
        if (launchQCds.checkRequest) {
            var elapsedTime = (Date.now() - launchQCds.startTime)/1000;
            var tbl = new vjTable(launchQCds.data,0,vjTable_propCSV);
            if (tbl.rows && tbl.rows.length &&tbl.rows[0].stat>=5) {
                clearInterval(launchQCds.intervalID);
                launchQCds.dependentData.ds.reload(launchQCds.dependentData.url + "&objs="+ launchQCds.objId,true);
            }
            else if (elapsedTime > 90) {
                clearInterval(launchQCds.intervalID);
            }
        }
        else {
            var url = "http:
            launchQCds.reload(url,true);
            launchQCds.checkRequest = true;
            launchQCds.startTime = Date.now();
        }
        if (launchQCds.reqid) {
            var url = "http:
            var progDs =launchQCds.progressView.ds; 
            progDs.reload(url,true);
            launchQCds.progressView.dv.hidden = false;
        }
        
    }
    launchQCds.launched = true;
    return text;
}


function vjHiveseqPositionalCountControl(viewer)
{
    if(viewer.name === undefined)viewer.name="positional_counts";
    if(viewer.options === undefined)viewer.options={ title:'Length count', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:['a0a0ff'], legend: 'none',
                                                    vAxis: {title:'number of reads'},
                                                    hAxis: {title:'length of the read'}
                                                    };
    if(viewer.series === undefined)viewer.series=[ {label:false, name:'position', title: 'Position on a query sequence'}, {name: 'count', title: 'Count of sequences of given length' } ];
    if(viewer.type === undefined)viewer.type='column-area';
    viewer.minRowsToAvoidRendering=3;
    viewer.switchToColumnMode=32;
    viewer.cols=[{ name: 'count', order:1, title: '# of Sequences', hidden: false }
              ,{ name: 'position', order:2, title: 'Length', hidden: false }
              ,{ name: 'quality', hidden: true }];
    if(viewer.formName)viewer.formObject=document.forms[viewer.formName];
    viewer.possibleGraphs=["area","line", "column"];
    return vjGoogleGraphControl.call(this, viewer);
}

function vjHiveseqPositionalQualityView(viewer)
{
    if(viewer.name === undefined)
        viewer.name="positional_quality";
    if(viewer.options === undefined)
        viewer.options={  title:'Quality length count', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:['ff0000'], legend: 'none',
                        vAxis: {title:'quality'},
                        hAxis: {title:'length of the sequences'}
                        };
    if(viewer.series === undefined)
        viewer.series=[ { name:'position', title: 'Position on a query sequence'}, {name: 'quality', title: 'Quality of sequences of given length' } ];
    if(viewer.type === undefined)
        viewer.type='column';
    viewer.cols=[{ name: 'count', order:1, title: '# of Sequences', hidden: false }
                ,{ name: 'position', hidden: true }
                ,{ name: 'quality', order:2,  title: 'Quality', hidden: false }];
    viewer.minimumChangeAllowed=0.00001;
    return vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqACGTCountView(viewer)
{
    if(viewer.name === undefined)viewer.name="ACGT_counts";
    
    if(viewer.options === undefined)viewer.options={ title:'ACGT base count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:vjPAGE.colorsACGTN,  vAxis: { title: 'Letter Count', minValue:0}  };
    if(viewer.series === undefined)viewer.series=[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence',eval:"if (!this.showN){if(row['letter']=='N'){row['count']=0}else{row['count']=row['count'];}} else {row['count']=row['count'];}" } ];
    if(viewer.type === undefined)viewer.type='pie';
    viewer.cols=[{ name: 'letter', order:1, align:'center',title: 'Nucleobase', hidden: false }
                ,{ name: 'count', order:2,  title: 'Count', hidden: false }
                ,{ name: 'quality', hidden: true }];
    vjGoogleGraphView.call(this, viewer);

}

function vjHiveseqACGTQualityView(viewer)
{
    if(viewer.name === undefined)viewer.name="ACGT_quality";
    if(viewer.options === undefined)viewer.options={ title:'Average quality per base', focusTarget:'category',
                width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:['red'], vAxis: { title: 'Average Quality', minValue:0}, hAxis: {title: 'Nucleotide'}, legend: {position: 'none'} };
    if(viewer.series === undefined)viewer.series=[ { label:true, name:'letter', title: 'Nucleotides'}, {name: 'quality', title: 'Nucleotide' } ];
    if(viewer.type === undefined)viewer.type='column';
    viewer.minimumChangeAllowed=0.00001;
    viewer.cols=[{ name: 'letter', order:1,align:'center', title: 'Nucleobase', hidden: false }
                ,{ name: 'count', hidden: true }
                ,{ name: 'quality', order:2,  title: 'Quality', hidden: false }];
    vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqCodonQualityView(viewer)
{
    if(viewer.name === undefined)viewer.name="Codon_quality";
    if(viewer.options === undefined)viewer.options={title:'Codon Quality Control', isStacked: 'percent',width: viewer.width?viewer.width:1000, height: viewer.height?viewer.height:300, hAxis: { title: 'Codon Table'}};
    if(viewer.series === undefined)viewer.series=[ {col:"0",label:true},{col:"1"},{col:"2"}  ];
    if(viewer.type === undefined)viewer.type='column';
    vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqComplexityView(viewer)
{
    if(viewer.name === undefined)viewer.name="Complexity";
    if(viewer.options === undefined)viewer.options={title:'Complexity Stats',is3D:true, width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300};
    if(viewer.series === undefined)viewer.series=[ {col:"0",label:true},{col:"1"}];
    if(viewer.type === undefined)viewer.type='pie';
    vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqNCountView(viewer)
{
    if(viewer.name === undefined)viewer.name="Ncount";
    if(viewer.options === undefined)viewer.options={ title:'N Count', 
            focusTarget:'category', 
            width: viewer.width?viewer.width:600, 
            height: viewer.height?viewer.height:300, 
            colors:vjPAGE.colorsACGT, 
            legend: 'none',
            hAxis: {title:'density of Ns per read'},
            vAxis: {title:'cumulative percentage of reads'}
            };
    if(viewer.series === undefined)viewer.series=[ {col:"0",label:true},{col:"1"}];
    if(viewer.type === undefined)viewer.type='line';
    vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqLengthwiseCountView(viewer)
{
    if(viewer.name === undefined)viewer.name="lengthwise_counts";
    if(viewer.options === undefined)viewer.options={ title:'Lengthwise position count', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:vjPAGE.colorsACGT, legend: 'none',
                                                    vAxis: {title:'base count'},
                                                    hAxis: {title:'position in the reads'}
                                                    };
    if(viewer.series === undefined)viewer.series=[
                                                  {label:false, name:'pos', title: 'Length'},
                                                  {name: 'countA', title: 'A' } ,
                                                  {name: 'countC', title: 'C' },
                                                  {name: 'countG', title: 'G' },
                                                  {name: 'countT', title: 'T' }
                                             ];
    if(viewer.type === undefined)viewer.type='line';
    viewer.cols=[{ name: new RegExp(/.*/),hidden:true}
                ,{ name: 'pos', order:1, align:'center',title: 'Position', hidden: false }
                ,{ name: 'countA', order:2,  title: 'count of A', hidden: false }
                ,{ name: 'countC', order:3,  title: 'count of C', hidden: false }
                ,{ name: 'countG', order:4,  title: 'count of G', hidden: false }
                ,{ name: 'countT', order:5,  title: 'count of T', hidden: false }];
    viewer.minRowsToAvoidRendering=3;
    viewer.switchToColumnMode=32;
    vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqLengthwiseQualityView(viewer)
{
    if(viewer.name === undefined)viewer.name="lengthwise_quality";
    if(viewer.options === undefined)viewer.options={ title:'Quality position count', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:vjPAGE.colorsACGT.concat(['red']), legend: 'bottom',
                                                    vAxis: {title:'average quality'},
                                                    hAxis: {title:'position in the reads'}
                                                    };
    if(viewer.series === undefined)viewer.series=[
                                                  {label:false, name:'pos', title: 'Length'},
                                                  {name: 'qualityA', title: 'A' } ,
                                                  {name: 'qualityC', title: 'C' },
                                                  {name: 'qualityG', title: 'G' },
                                                  {name: 'qualityT', title: 'T' },
                                                  {name: 'Threshold', title: 'Threshold', eval: "20" }
                                              ];
    if(viewer.type === undefined)viewer.type='line';
    viewer.cols=[{ name: new RegExp(/.*/),hidden:true}
                ,{ name: 'pos', order:1, align:'center',title: 'Position', hidden: false }
                ,{ name: 'qualityA', order:2, align:'center', title: 'quality of A', hidden: false }
                ,{ name: 'qualityC', order:3, align:'center', title: 'quality of C', hidden: false }
                ,{ name: 'qualityG', order:4, align:'center', title: 'quality of G', hidden: false }
                ,{ name: 'qualityT', order:5, align:'center', title: 'quality of T', hidden: false }];
    viewer.minimumChangeAllowed=0.00001;
    vjGoogleGraphView.call(this, viewer);
}

function vjHiveseqACGTCountViewControl(viewer) {
    if(viewer.formObject===undefined)viewer.formObject= document.forms[viewer.formName];

    this.ACGTCountViewer = new vjHiveseqACGTCountView(viewer);

    this.panelV = new vjPanelView({
           data:['dsVoid'],
           iconSize:24,
           showTitles:true,
           formObject:document.forms[viewer.formName],
           rows:[
                 { name: 'show', icon: 'img/down.gif', title: 'Options', path:'/show', align:'left', description: "show flag" }
                ,{ name: 'showN',title: 'Show % of N', path:'/show/showN', align:'left', description: 'show N',url:showHideN}
                ,{ name: 'hideN',title: 'Hide % of N', path:'/show/hideN', align:'left', description: 'hide N',url:showHideN}
           ]
    });
    this.panelV.dependantViewer=this.ACGTCountViewer;

    function showHideN(panel,row,node) {
        var name=node.name;
        if (name.indexOf("hideN")!=-1){
            panel.dependantViewer.showN = false;
            panel.dependantViewer.title= 'ACGT base count'
        }else {panel.dependantViewer.showN = true;panel.dependantViewer.title= 'ACGTN base count'}
        panel.dependantViewer.render();

    }
    return [this.panelV,this.ACGTCountViewer];
}


function vjHiveseqControl(viewer)
{
    if(viewer.formObject===undefined)viewer.formObject= document.forms[viewer.formName];

    this.hiveseqViewer = new vjHiveseqView(viewer);

    this.panelViewer = new vjHiveseqPanelView({
        data: ["dsVoid",viewer.data],
        addRows: viewer.addRows,
        formObject: viewer.formObject,
    });

    this.hiveseqViewer.objectsDependOnMe=[this.panelViewer];
    return [this.panelViewer,this.hiveseqViewer];


}

