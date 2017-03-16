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

    var dsurl = "http://?cmd=objList&type=u-hiveseq%2B&mode=csv&info=1&actions=1&cnt=100&prop=id,name,rec-count,size,created,hierarchy,_type";
    if (url!==undefined && url.indexOf("http://")!==null) dsurl = url;
    if (!active) active = "tree";
    if (searchKey) dsurl += "&prop_val=" + searchKey;
    var hideDBactions=[{name:'upload',hidden:true,prohibit_new:true},
                       {name:'create',hidden:true,prohibit_new:true},
                       {name:'convert',hidden:true,prohibit_new:true}];
    if(hidebuttons)hideDBactions=hidebuttons.concat(hideDBactions);
    vjPAGE.initListAndTreeViewerTab(dsurl, dvname, tabname, tabico, recordviewer, formname, fullpanel, active, hideDBactions, selectCallback, bgClrMap,isdrag);

    var viewer = vjDV.locate(dvname + "." + tabname + "." + (cntViewersIn + 1));
    viewer.cols = ([
           { name: new RegExp(/.*/), hidden: true }, // , url: (( typeof(recordviewer) == "function" ) ? recordviewer : vjHC.recordViewNode) },
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

    //alert(jjj(vjDV.locate(dvname+"."+tabname+"."+cntViewersIn).rows));

    //if(!fullpanel)return ;

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
                //{name:'refresh', title: 'Refresh' , icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['"+dsName+"'].reload(null,true);"},
               {name:'pager', align:'right', icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
               {name:'search', align:'right', isSubmitable:true, type:' search', title:'Search', description: 'search sequences by ID', url:"?cmd=objFile&ids=$(ids)"}
               ]);
    }
    vjPanelView.call(this,viewer);
}

function launchQC_Screen(viewer, node) {
    var dsname = "ds" + node.name; 
    if (!vjDS[dsname]) {
        vjDS.add("Loading " + node.description, dsname, "static://")
    }
    var url = "http://dna.cgi?cmd=" + node.dsCmdLink;
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
            var elapsedTime = (Date.now() - launchQCds.startTime)/1000; // convert to seconds
            var tbl = new vjTable(launchQCds.data,0,vjTable_propCSV);
            if (tbl.rows && tbl.rows.length &&tbl.rows[0].stat>=5) {
                clearInterval(launchQCds.intervalID);
                launchQCds.dependentData.ds.reload(launchQCds.dependentData.url + "&objs="+ launchQCds.objId,true);
            }
            else if (elapsedTime > 90) { // 90 seconds
                clearInterval(launchQCds.intervalID);
            }
        }
        else {
            var url = "http://?cmd=-qpRawCheck&raw=1&req=" + launchQCds.reqid;
            launchQCds.reload(url,true);
            launchQCds.checkRequest = true;
            launchQCds.startTime = Date.now();
        }
        if (launchQCds.reqid) {
            var url = "http://?cmd=-qpRawCheck&raw=1&req=" + launchQCds.reqid;
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
    
    if(viewer.options === undefined)viewer.options={ title:'ACGT base count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:vjPAGE.colorsACGT,  vAxis: { title: 'Letter Count', minValue:0}  };
    if(viewer.series === undefined)viewer.series=[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence' } ];
    if(viewer.type === undefined)viewer.type='pie';
    //viewer.minRowsToAvoidRendering=3;
    //viewer.switchToColumnMode=32;
    viewer.cols=[{ name: 'letter', order:1, align:'center',title: 'Nucleobase', hidden: false }
                ,{ name: 'count', order:2,  title: 'Count', hidden: false }
                ,{ name: 'quality', hidden: true }];
    //viewer.debug=1;
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
/*
function vjHiveseqTaxonomicControl(viewer)
{

     function matchResults(node) {
         if (node === undefined)
             node = this.tree.root;

         if (node.matchResults !== undefined)
             return node.matchResults;

         if (node.leafnode || node.leaf) {
             node.matchResults = parseInt(node.matchCnt);
         } else {
             if(node.matchCnt)
                 node.matchResults = parseInt(node.matchCnt);
             else node.matchResults = 0;
             for (var i = 0; i < node.children.length; i++)
                 node.matchResults += matchResults(node.children[i]);
         }
         return node.matchResults;
     };

        function registerNodeClickCallbacks(phylogram, node) {
        //    phylogram.registerLeafCheckedCallback(node.name, function() {alert(node.allname);});
        }
        var phylogram;

        function treeSeriesLoaded() {
            matchResults(this.tree.root);
            this.tree.enumerate(function(phylogram, node) {
             if (node.doneInitialExpansion)
                 return;
             node.expanded = true;
             node.doneInitialExpansion = true;
            }, phylogram);
            this.refreshWithoutRebuildingTree();
            //this.tree.enumerate(registerNodeClickCallbacks, phylogram, 0, 0, this.tree.root);

        }



     var series = new vjTreeSeries({
         name: viewer.data,
         title: "Tree series",
         showRoot:1,
         url: "static://",
         dataFormat: "csv", // or "newick"
         type: "rectangular",
         rectangularLabelInline: true,
         precompute:"node.path=node.path",
         cmdMoreNodes: "http://taxTree.cgi?whatToPrint=taxid|path|childrenCnt&taxid=$(taxid)&depth=1&cmd=ncbiTaxBrowseDown",
         postcompute: "if(node.name){"+
         "var spl=node.name.split(':');"+
         "node.title=spl[0];"+
         "node.taxid=spl[1];"+
         "node.childrenCnt=parseInt(spl[2]);}"
     });
     series.register_callback({ obj: series, func: treeSeriesLoaded }, "series_loaded", "refreshed");

     phylogram = new vjSVG_Phylogram({
         nodeLabel: function (node, series) {
           //  var tbl = series.tbl;
             // hide labels of inner nodes in non-branching chains
           //  if (node.expanded && !node.leafnode && node.children.length == 1)
           //      return null;
             if(node.matchResults)
                 return node.title + "[" + node.matchResults+"]";
             else return node.title;
         },
         nodeTooltip: function (node, series, isLabelText) {
             if (isLabelText)
                 return node.title;
             return this.defaultNodeTooltip.apply(this, arguments);
         },
         nodeSymbolBrush: function (node, series1) {
          if(node.matchCnt)    return {fill:"#33FF99", 'fill-opacity':1,opacity:1};
          else if(node.matchResults)
             return {fill:"#9999FF", 'fill-opacity':1,opacity:1};
          else return {fill:"#CCCCCC", 'fill-opacity':1,opacity:1};

         },
         nodeSymbolSize: function (node, series) {
             //                var size = series ? series.symbolSize : this.defaultSeriesSymbolSize();
             //return size *= (3 - 2*Math.exp(-2*node.matchResults/series.tree.root.matchResults));
             //var size = series ? series.symbolSize : this.defaultSeriesSymbolSize();
             //return size;
             return node.leafnode ? 5 : 6;
         },
         nodeLabelFont: function (node) {
             var font = { "font-size": 10};

             return font;
         }
     });
     phylogram.add(series);
     var newURL = "http://taxTree.cgi?whatToPrint=taxid|path|childrenCnt&cnt=150&cmd=ncbiTaxBrowseCurrent&screenId="+viewer.updateURL;
    if(viewer.data=='blastNTset')
            newURL += "&screenType=dna-alignx_screenResult.csv"
     this.myRows = new Array();
     this.myRows.push({ name: 'pager', icon: 'page', order:1,title: 'per page', description: 'page up/down or show selected number of objects in the control', type: 'pager', counters: [50,150,300,500,1000, 'all'] });
     if(viewer.ifFromShortRead)
         this.myRows.push({name:'screen', title: 'Screen' ,order:2,align:'left', icon:'tree' , description: 'screen the sequence again' ,  url: "?cmd=dna-screening&objIDs="+viewer.updateURL});
     this.myPanel = new vjPanelView({
         data: ["dsVoid",viewer.data],
         iconSize: 24,
         cmdUpdateURL:newURL ,
         rows:this.myRows,
         formObject: document.forms[viewer.formName],
     });

     this.myTaxTree = new vjSVGView({
          icon:viewer.icon,
          name: 'phylogram',
          chartArea:{height:'95%'},
          objectsDependOnMe: this.myPanel,
          plots:[phylogram],
          precompute:"this.tree.enumerate('node.expanded=true');",
          formObject: document.forms[viewer.formName]
      });

     this.myTaxTreeText = new vjTreeView({
           icon:viewer.icon,
           data: viewer.data,
           name: 'tree',
           hidden:true,
           chartArea:{height:'95%'},
           objectsDependOnMe: this.myPanel,
           //precompute:"this.tree.enumerate('node.expanded=true');",
           formObject: document.forms[viewer.formName]
       });

     this.myTaxList = new vjTableView({
           icon:viewer.icon,
           data: viewer.data,
           name: 'list',
           hidden:true,
           chartArea:{height:'95%'},
           objectsDependOnMe: this.myPanel,
           //precompute:"this.tree.enumerate('node.expanded=true');",
           formObject: document.forms[viewer.formName]
       });


    return [this.myPanel,this.myTaxTree,this.myTaxTreeText,this.myTaxList];

}

*/

//# sourceURL = getBaseUrl() + "/js/vjHiveseqView.js"