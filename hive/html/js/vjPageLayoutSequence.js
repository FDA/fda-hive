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

vjPAGE.initSequenceListViewerTab = function (dvname, tabname, tabico, recordviewer, formname, fullpanel, searchKey, active, checkcallback, selectCallback, url, bgClrMap,isdrag,hidebuttons)
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
};

vjPAGE.initShortSelectedSequenceListTab=function(dvname,dsname,tabname,title, tabico, emptyText)
{
    var textForEmpty="no sequences selected";
    if (emptyText!==undefined) textForEmpty = emptyText;
    var queryBriefViewer=new vjTableView( {
        data: dsname,
        cols: [
            {name:new RegExp(/.*/), hidden:true },
            {name:'^name', hidden: false, title:(title ? title : "Sequences"), wrap: true } ,
            {name:'rec-count', hidden: false, title:"Records" , align: "left", type: "largenumber" },
            {name:'^size$', hidden: false, title:"Bases" , align: "left", type: "bytes" }
            ],
        iconSize:0,
        defaultEmptyText: textForEmpty,
        maxTxtLen:32,
        isok:true });
    vjDV[dvname].add(tabname,tabico,"tab", [ queryBriefViewer ] );
};

vjPAGE.initSelectedElementListTab=function(dvname,dsname,tabname,title, tabico, colsToShow ,emptyText)
{
    var textForEmpty="no element selected";
    if (emptyText!==undefined) textForEmpty = emptyText;
    var queryBriefViewer=new vjTableView( {
        data: dsname,
        cols: [
            {name:new RegExp(/.*/), hidden:true },
            {name:'^name', hidden: false, title:(title ? title : "Sequences"), wrap: true } ,
            {name:"id",hidden:false,align:"left",title: "ID"}
            ],
        iconSize:0,
        defaultEmptyText: textForEmpty,
        maxTxtLen:32,

        isok:true });

    if (colsToShow !== undefined && colsToShow.length !== null) {
        for ( var i = 0; i < colsToShow.length; i++) {
            var nameCol = colsToShow[i];
            queryBriefViewer.cols.push(nameCol);
        }
    }
    vjDV[dvname].add(tabname,tabico,"tab", [ queryBriefViewer ] );
};

vjPAGE.colorsACGT=['#006AFF','#95BE4D','#556B2F', '#000080'];
vjPAGE.colorsACGTN=['#006AFF','#95BE4D','#556B2F', '#000080','#ff8000'];

vjPAGE.initSequenceQCViewerTabs=function(dvname, formname, geometryX, geometryY )
{

    var initTxt="preview:<img border=0 src='img/music.gif' /' width=128>";
    vjDS.add("Constructing Next-Gen Sequences Positional QC Data", "ds-" + dvname + "-SeqQualityControlPositional", "static:
    vjDS.add("Constructing Next-Gen Sequences Nucleotide Quality", "ds-" + dvname + "-SeqQualityControlATGC", "static:
    vjDS.add("Computing Quality Control Lengthwise Histograms", "ds-" + dvname + "-SeqQualityControlLengthwise", "static:
    vjDS.add("infrastructure: Hiveseq composition help documentation","dsHelpHiveseq1","http:

    var viewerPreviewGraphPositionalCount = new vjGoogleGraphView ({
        data: 'ds-'+dvname+'-SeqQualityControlPositional',
        name:'counts',
        options:{ focusTarget:'category', width: geometryX, height: geometryY, colors:['a0a0ff'], legend: 'top' },
        series:[ {label:false, name:'position', title: 'Position on a query sequence'}, {name: 'count', title: 'Count of sequences of given length' } ],
        type:'line',
        isok:true});
    var viewerPreviewGraphPositionalQuality = new vjGoogleGraphView ({
        data: 'ds-'+dvname+'-SeqQualityControlPositional',
        name:'counts',
        options:{ focusTarget:'category',width: geometryX, height: geometryY,colors:['ffa0a0'] , legend: 'top' },
        series:[ { name:'position', title: 'Position on a query sequence'}, {name: 'quality', title: 'Quality of sequences of given length' } ],
        type:'line',
        isok:true});

    var viewerPreviewGraphATGCCount= new vjGoogleGraphView ({
        name:'qualities',
        data: 'ds-'+dvname+'-SeqQualityControlATGC',
        options:{ focusTarget:'category',width: geometryX, height: geometryY, colors:['#a0a0ff'] , vAxis: { title: 'Letter Count', minValue:0} , legend: 'top' },
        series:[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence' } ],
        type:'column',
        isok:true});

    var viewerPreviewGraphATGCQuality = new vjGoogleGraphView ({
        name:'qualities',
        data: 'ds-'+dvname+'-SeqQualityControlATGC',
        options:{ focusTarget:'category',width: geometryX, height: geometryY, colors:['#ffa0a0'], vAxis: { title: 'Letter Quality', minValue:0} , legend: 'top' },
        series: [ { label:true, name:'letter', title: 'Nucleotides'}, {name: 'quality', title: 'Quality of sequences of given length' } ],
        type:'column',
        isok:true});

    var viewerPreviewGraphLengthwiseCount= new vjGoogleGraphView ({
        name:'qualities',
        data: 'ds-'+dvname+'-SeqQualityControlLengthwise',
        options:{ focusTarget:'category',width: geometryX, height: geometryY, colors:vjPAGE.colorsACGT  , legend: 'top' },
        series:[
                {label:false, name:'pos', title: 'Length'},
                {name: 'countA', title: 'A' } ,
                {name: 'countC', title: 'C' },
                {name: 'countG', title: 'G' },
                {name: 'countT', title: 'T' }
           ],

        type:'line',
        isok:true});
    var viewerPreviewGraphLengthwiseQuality =new vjGoogleGraphView ({
        name:'qualities',
        data: 'ds-'+dvname+'-SeqQualityControlLengthwise',
        options:{ focusTarget:'category',width: geometryX, height: geometryY, colors:vjPAGE.colorsACGT , legend: 'top'},
        series:[
                {label:false, name:'pos', title: 'Length'},
                {name: 'qualityA', title: 'A' } ,
                {name: 'qualityC', title: 'C' },
                {name: 'qualityG', title: 'G' },
                {name: 'qualityT', title: 'T' }
            ],
        type:'line',
        isok:true});


    vjDV[dvname].add( "histograms", "graph", "tab", [ viewerPreviewGraphPositionalCount ,viewerPreviewGraphPositionalQuality] );
    vjDV[dvname].add( "ACGT", "graph", "tab", [ viewerPreviewGraphATGCCount ,viewerPreviewGraphATGCQuality] );
    vjDV[dvname].add( "positionalQC", "graph", "tab", [ viewerPreviewGraphLengthwiseCount ,viewerPreviewGraphLengthwiseQuality] );


    vjDV[dvname].add( "help", "help", "tab", [ new vjHelpView ({ data:'dsHelpHiveseq1'})  ]);

};






vjPAGE.initSequenceContentViewer=function(dsName,dvname,tabname,formname)
{
    var viewerPanel = new vjPanelView({
         data:[ "dsVoid",  dsName],
         formObject: document.forms[formname],
         iconSize:24,
         hideViewerToggle:true,
         rows: [
                 {name:'refresh', title: 'Refresh' , icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['"+dsName+"'].reload(null,true);"},
                {name:'pager', align:'right', icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                {name:'search', align:'right', isSubmitable:true, type:' search', title:'Search', description: 'search sequences by ID', url:"?cmd=objFile&ids=$(ids)"}
                ],
         isok:true });


     var viewerSequenceContent=new vjTableView( {
         name:'list',
         icon:'list',
         data: dsName,
         formObject: document.forms[formname],
         bgColors:['#f2f2f2','#ffffff'] ,
         geometry:{width:"100%"},
         cols:[{name:'seq', title:'Sequence', maxTxtLen:300, type: 'pre'}],
         maxTxtLen:32,
         objectsDependOnMe:[ dvname+'.'+tabname+'.0' ],
         checkable:true,
         isok:true });

     vjDV[dvname].add( tabname, "dna", "tab", [ viewerPanel,viewerSequenceContent]);

};


