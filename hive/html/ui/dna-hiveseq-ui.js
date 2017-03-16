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

var process_formName = "form-process";
var process_ID = docLocValue("id");if (!process_ID) process_ID = 0;
var algoProcess, algoProgress;

gInitList += "process_init();";
setLocationTitle("dna-hiveseq: HIVE");

var process_visibleParameters=["name"];
var process_visualImage="";
var process_help=[];
var process_initialPresets={AlgorithmsName: "Randomizer"};
var process_cmdMode=docLocValue("cmdMode");
var process_qpsvc="dna-hiveseq";
var process_svc="svc-hiveseq";
var queryIds = docLocValue("ids");
var hiveseqIds = docLocValue("id");
function isMode(mode)
{
    return (!process_cmdMode || (process_cmdMode.indexOf("-"+mode)==-1 && process_cmdMode.indexOf(mode)!=0)) ? false : true;
}

google.load("visualization", "1", {packages:["corechart"]});

var hiveseq_GTotalSequences=0;
var hiveseq_hdr="id,range-start,range-end,name";
var default_name_suggestion="new hiveseq name";
var set_sequences="sequences";
var genomes = "genomes";
var dna_hiveseq_ID=docLocValue("id");if(!dna_hiveseq_ID)dna_hiveseq_ID=0;
var dna_algo_cansubmit = (isok(dna_hiveseq_ID) && dna_hiveseq_ID[0] != '-' ) ? false : true ;



// _/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Main initialization
// _/

function process_inputLoaded()
{
    if (queryIds) {


//        var getIds_url = "?cmd=objList&type=u-hiveseq+&mode=csv&prop_val="+queryIds+"&prop=rec-count,name&raw=1";
        var getIds_url = "?cmd=propget&mode=csv&ids="+queryIds+"&prop=rec-count,name&raw=1";
        ajaxDynaRequestPage(getIds_url, "", function initialize_ids_submission(ajax, contentReturn){
            var tbl = new vjTable(contentReturn, 0, vjTable_propCSV);
            hiveseq_safeInsertIntoHiveseq(tbl.rows);
            });
    }
    if (hiveseqIds){
        var getIds_url = "?cmd=objList&type=u-hiveseq+&mode=csv&ids="+queryIds+"&prop=rec-count,name&raw=1";
        ajaxDynaRequestPage(getIds_url, "", function initialize_ids_submission(ajax, contentReturn){
            var tbl = new vjTable(contentReturn, 0, vjTable_propCSV);
            hiveseq_safeInsertIntoHiveseq(tbl.rows);
            });

    }
};



function process_init()
{

    vjVIS.generate(new Array(
        {
            name : 'dvProcess',
            // passive : (dna_hiveseq_defaults_ui.visualElements.parameters) ? false : true,
            role : 'input',
            title : "HIVE Sequence Manipulation Parameters",
            align : 'left',
            icon: 'img/processSvc.gif',
            bgImage: process_visualImage,
            icoSize:64,
            briefSpans: process_visibleParameters,
            popupCloser: true,
            briefSpanTag: "RV-",
        isok:true}));

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ FileList: u-hiveseq
    // _/

    vjDS.add("Scanning Available Sequences ", "dsSeqNucReads" , "static:// " );
    vjDS.add("Scanning Available Genomes", "dsSeqGenomes" , "static:// " );
    var dsname="dsSeqNucReads"; //'ds-'+dvname+'-'+tabname;
    var viewCmd= [
                  {name:new RegExp(/.*/), align:'left', hidden: true },
//                  {name:'add', align:'left', order:1, hidden:false, showTitle : true , title:'Add', description: 'add new objects to the list' , icon:'plus', prohibit_new: true },
                  //url:"javascript:vjDV.select('"+dvname+".add.0',true);"
                  {name:'refresh', align:'left', order:0, hidden: false, align:'left', title: 'Refresh' , icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['"+dsname+"'].reload(null,true);"},
                  {name:'detail', align:'left', order:1, hidden: false, icon: 'eye', description: 'examine, review, edit details of the object' ,hidden:true },
                  {name:'download', align:'left', order:2, hidden: false, align:'left', title:'Download', description: 'download the content of this object to local computer', url:"?cmd=objFile&ids=$(ids)", icon: 'download', single_obj_only:true},
                  {name:'pager', align:'right', order:99, hidden: false, icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                  {name:'search', align:'right', order:100, hidden: false, isSubmitable: true }
                  ];

    var rowlist= viewCmd;

    var collist= [
           {name:new RegExp(/.*/), hidden:true },// , url: (( typeof(recordviewer) == "function" ) ? recordviewer : vjHC.recordViewNode) },
           {name:'id', hidden: false, title:'ID', order: 1 } ,
           {name:'name', hidden: false, title:'Name', wrap: true, order: 2 } ,
           {name:'rec-count', hidden: false, title:'Record', type: "largenumber", wrap: true, order: 3 } ,
           {name:'size', hidden: false, title:"Size" , align: "right", type: "bytes", order: 4 } ,
//           {name:'hierarchy', hidden: false, title:"Hierarchy" , align: "left"} ,
           {name:'created', hidden: false, title:"Created" , align: "right", type: "datetime", order: 5 } ,
           {name:'source', hidden: false, title:'Source', wrap: true }
           ];

//    var viewerSequenceFilePanel = new vjPanelView({
//        data:["dsActions", dsname ],
//        formObject: document.forms['form-sequenceList'],
//        iconSize:24,
//        hideViewerToggle:true,
//        showTitles: false,
//        rows:rowlist,
//        isok:true }
//    );
//
//    var viewerSequenceFileList=new vjTableView( {
//        name:'list',
//        icon:'list',
//        data: dsname,
//        formObject: document.forms['form-sequenceList'],
//        //bgColors:bgColorMap,
//        cols:collist,
//        //vjHCAssociatedRecordViewer: recordviewer+".details.0" ,
//        selectCallback: hiveseq_selectSequenceFile,
//        checkCallback: hiveseq_checkSequenceFile,
//        checkable:true,
//        //objectsDependOnMe:[ dvname+'.'+tabname+'.'+cntViewersIn ],
//        defaultIcon:'rec',
//        iconSize:16,
//        maxTxtLen:this.maxTxtLen,
//        maxTextLenCutExtensionAfterSymbols: "." , // to cut extensions when maxTextLen is Reached
//        defaultEmptyText:'no accessible information to show',
//        geometry:{ width:'100%'},
//        drag:true,
//        dragRow:true,
//        isok:true });
//
//    var dsGenomeName="dsSeqGenomes"; //'ds-'+dvname+'-'+tabname;
//    var viewerGenomeFilePanel = new vjPanelView({
//        data:["dsActions", dsGenomeName ],
//        formObject: document.forms['form-sequenceList'],
//        iconSize:24,
//        hideViewerToggle:true,
//        showTitles: false,
//        rows:rowlist,
//        isok:true }
//    );
//    var viewerSequenceFileTree=new vjTableView( {
//        name:'list',
//        icon:'list',
//        data: dsGenomeName,
//        formObject: document.forms['form-sequenceList'],
//        //bgColors:bgColorMap,
//        cols: collist,
//        //vjHCAssociatedRecordViewer: recordviewer+".details.0" ,
//        selectCallback: hiveseq_selectSequenceFile,
//        checkCallback: hiveseq_checkSequenceFile,
//        checkable:true,
//        //objectsDependOnMe:[ dvname+'.'+tabname+'.'+cntViewersIn ],
//        defaultIcon:'rec',
//        iconSize:16,
//        maxTxtLen:this.maxTxtLen,
//        maxTextLenCutExtensionAfterSymbols: "." , // to cut extensions when maxTextLen is Reached
//        defaultEmptyText:'no accessible information to show',
//        geometry:{ width:'100%'},
//        drag:true,
//        dragRow:true,
//        isok:true });
//
//    vjDV.add("dvSequenceFileListViewer",500,300);
//    vjDV.dvSequenceFileListViewer.add(set_sequences, "dna", "tab", [ viewerSequenceFilePanel,viewerSequenceFileList ]);
//    vjDV.dvSequenceFileListViewer.add("genomes", "dna", "tab", [ viewerGenomeFilePanel, viewerSequenceFileTree]);
    var width=(parseInt(__getCurrentComputedStyle(gObject('element_toget_metrics_1'),'width'))
    +parseInt(__getCurrentComputedStyle(gObject('element_toget_metrics_2'),'width')))/4;
    var home_GFolderGeomX=300;//0.25*width-5;
    var home_GlistGeomY=250;
    var home_GObjectGeomX=width-home_GFolderGeomX-20;
    
    var explorer=new vjExplorerUsrView({
        container:"dvSequenceFileListViewer",
        preselectedFolder: "/all",
        onSelectFileCallback : hiveseq_selectSequenceFile,
        isNdisplay_Previews:true,
        isNdisplay_Folders:true,
        isNShowactions:true,
        drag: false,
        folders_DV_attributes : {
            width : home_GFolderGeomX,
            height : home_GlistGeomY,
            hideListCols :[{ name : 'home',hidden : true}]
        }, 
        tables_DV_attributes : {
            width : home_GObjectGeomX,
            height : home_GlistGeomY,
            maxtabs : 9,
            isok:true
        },
        useCookies : true,
        subTablesAttrs : [ {
            tabname : "genomes",
            tabico : "dna",
            url : {
                type : "^genome$"
            }
        }, {
            tabname : "reads",
            tabico : "dnaold",
            url : {
                type : "^nuc-read$"
            }
        } ],
        addCmd : [{
            name:'append', align:'left', 
            title: 'Add' ,
            showTitle:true,
            is_obj_action: true, 
            is_forced_action:true, icon:'plus' , 
            description: 'append this item to the HIVE seq' , 
            url: "javascript:hiveseq_addSequenceFile('dvSequenceFileListViewer_Tables._active._active')"
        },]
    });
    
    explorer.init();
    explorer.render();
    explorer.load(true, true);
    
// var dsurl =
// "http://?cmd=objList&mode=csv&info=1&actions=1&cnt=100&prop=id,name,rec-count,size,created,hierarchy,_type";
//    var dsnucreadurl = dsurl + "&type=nuc-read";
//    var dsgenomeurl = dsurl + "&type=genome";
//
//
//    vjDS["dsSeqNucReads"].reload(dsnucreadurl, true);
//    vjDS["dsSeqGenomes"].reload(dsgenomeurl, true);
//
//    vjDV.dvSequenceFileListViewer.render();
//    vjDV.dvSequenceFileListViewer.load();

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Sequence Content
    // _/
    var hiveseq_previewGeometryX=500;
    var hiveseq_previewGeometryY=250;

    vjDS.add("Scanning Available Sequence Lists", "dsSequenceContent" , "static:// " );
    vjDV.add("dvSequenceContentViewer",700,300);

    vjDS.add("help","dsContentHelp","http://help/hlp.page.hiveseq.input.html");
    var helpviewer = new vjHelpView({
        data:'dsContentHelp'
    });
    vjDV.dvSequenceContentViewer.addTab('help','help',helpviewer);
    vjDV.dvSequenceContentViewer.render();
    vjDV.dvSequenceContentViewer.load();

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ HIVESEq compositions
    // _/

    vjDS.add("infrastructure: Hiveseq composition", "dsHiveseqComposition" , "static:// " ).header=hiveseq_hdr;
    vjDV.add("dvHiveseqCompositionViewer",500,300);

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Panels:
    // _/

    var viewerHiveseqCompositionPanel = new vjPanelView({
        data:[ "dsVoid",  "dsHiveseqComposition" ],
        formObject: document.forms['form-hiveseqComposition'],
        iconSize:24,
        showTitles:true,
        rows: [
            {name: 'info', readonly:true, title:"eval:hiveseq_GTotalSequences+' sequences';" },
            {name: 'delete', icon:'hiveseq-delete', title: 'Remove marked', description:'remove an entry from HIVE-seq', url:hiveseq_deleteHiveseqEntries}
            ],

        isok:true });

    var viewerHiveseqComposition=new vjTableView( {
        name:'list',
        icon:'list',
        data: 'dsHiveseqComposition',
        formObject: document.forms['form-hiveseqComposition'],
        bgColors:['#f2f2f2','#ffffff'] ,
        geometry:{width:"100%"},
        checkable:true,
        maxTxtLen:32,
        defaultEmptyText:'please select sequences from above',
        isok:true });

    vjDV.dvHiveseqCompositionViewer.add(set_sequences, "dna", "tab", [ viewerHiveseqCompositionPanel,viewerHiveseqComposition ]);

/*  Here goes the HELP link */
    if( !dsHelpSource ){
        dsHelpSource = "http://help/hlp.page.hiveseq.html";
    }
    vjDS.add("infrastructure: Hiveseq composition help documentation","dsHelpHiveseq",dsHelpSource);
    vjDV.dvHiveseqCompositionViewer.add( "help", "help", "tab", [ new vjHelpView ({ data:'dsHelpHiveseq'})  ]);

    vjDV.dvHiveseqCompositionViewer.render();
    vjDV.dvHiveseqCompositionViewer.load();


    //algoProcess=new valgoProcess(dna_hiveseq_ID,"form-hiveseqParameters", "dvHiveseqParameters","dna-hiveseq","svc-hiveseq", "   LAUNCH HIVESEQ    ");
    algoProcess = new valgoProcess(process_ID, process_formName,"dvProcess", process_qpsvc,process_svc, "    Launch!    ");
    algoProcess.callbackLoaded = process_inputLoaded;
    algoProcess.initialPresets = isok(process_ID) ? null : process_initialPresets;
    algoProcess.subject_help=process_help;
    algoProcess.generate();
      process_inputLoaded();

    visibool(algoProcess.dvname + "Submitter", false);

    algoProgress=new valgoProgress(dna_hiveseq_ID,"form-hiveseqParameters" ,"Progress");
    if(!dna_algo_cansubmit) {
        algoProgress.callbackDoneComputing=dna_hiveseq_doneComputing;
        algoProgress.generate();
    }


    if(process_initialPresets.inputTitle){
        gObject('inputTitleSection').innerHTML = process_initialPresets.inputTitle;
    }
}

function hiveseq_checkSequenceFile(viewer, node){
    //var varseqdv = "dvSequenceFileListViewer."+set_sequences+".0";
//    var addRows= {name:'arow', align:'left', title: 'Add' ,showTitle:true, is_obj_action: true, is_forced_action:true, icon:'plus' , description: 'append this item' ,  url: "javascript:hiveseq_addSequenceFile('dvSequenceFileListViewer._active._active')" };
    var addRows = c;
    var panelVV;
    if (viewer.container.indexOf(set_sequences) != -1 ){
        panelVV = vjDV.locate ("dvSequenceFileListViewer."+set_sequences+".0");
    }
    else{
        panelVV = vjDV.locate ("dvSequenceFileListViewer."+genomes+".0");
    }
    if(viewer.checkedCnt == 0 && panelVV.rows.length>6) panelVV.rows.pop();
    else if (viewer.checkedCnt >0 && panelVV.rows.length==6) panelVV.rows = panelVV.rows.concat(addRows);
//    var seqList=vjDV.locate(viewer);
//    if(seqList)seqList.rows=seqList.rows.concat(addRows);

    panelVV.render();
    panelVV.load();
    //vjDV["dvSequenceFileListViewer"].render();
    //vjDV["dvSequenceFileListViewer"].load();
}
function dna_hiveseq_doneComputing(viewer ,reqid, stat)
{
    if(stat>=5) {
        alert("Done! You can see the resulting files in the home page.");
    }
}



// _/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Adding hiveseq
// _/
var nodeSelected;
function hiveseq_selectSequenceFile(viewer, node )
{

    nodeSelected=node;
    var tabs = vjDV.dvSequenceContentViewer.tabs;
    var tab_Names = new Array();
    for (var i = 1 ; i < tabs.length ; ++i ) {
        tab_Names.push(tabs[i].name);
    }
    if( tab_Names.length ) {
        vjDV.dvSequenceContentViewer.remove(tab_Names);
    }
    var dna_hiveseq_fullview = vjHO.fullview("u-hiveseq",node,{obj:vjDV.dvSequenceContentViewer},null,onConstruction);//,"hiveseq_SequenceContentViewer_loaded");
    
    
}
function onConstruction(obj){
    obj.onLoad=hiveseq_SequenceContentViewer_loaded;
    obj.onRender=hiveseq_SequenceContentViewer_rendered;
}
function hiveseq_SequenceContentViewer_rendered(hiveObj,viewers)
{

    var addRows = {name:'append', align:'left', title: 'Add' ,showTitle:true, is_obj_action: true, is_forced_action:true, icon:'plus' , description: 'append this item to the HIVE seq pool' ,  url: "javascript:hiveseq_addSequenceFile('dvSequenceContentViewer._active._active')" };
    var viewer=vjDV.locate("dvSequenceContentViewer.sequences.0");
    if(viewer)viewer.rows=viewer.rows.concat(addRows);

    viewer=vjDV.locate("dvSequenceContentViewer.sequences.1");
    var viewerList=vjDV.locate("dvSequenceFileListViewer_Tables._active._active");
    var ids=viewerList.accumulate("node.selected","node.id").join(",");
    if(!ids.length)return ;
    viewer.currentSequenceFileID=ids;
    var names=viewerList.accumulate("node.selected","node.name").join(",");
    viewer.currentSequenceFileName=names;
    var o=gObject("selectedSequenceFile");
    if(o)o.innerHTML=names +"<small><small>: ("+ids+")</small></small>";

}

function hiveseq_SequenceContentViewer_loaded(hiveObj,viewers)
{
   hiveseq_hideViewer(viewers[4]);
   hiveseq_hideViewer(viewers[6]);//"dvSequenceContentViewer.histogram.2");
   hiveseq_hideViewer(viewers[8]);//"dvSequenceContentViewer.lengthwise.1");
}

function hiveseq_hideViewer(viewer)
{
    if( (nodeSelected.name.indexOf(".fastq")==-1) && (nodeSelected.name.indexOf(".fq")==-1) ) {
        viewer.hidden=true;
    }
    else
        viewer.hidden=false;
}

function hiveseq_addSequenceFile ( viewerloc )
{

    var viewer=vjDV.locate(viewerloc);

    var idlist,subset,namelist;
    if(viewer.currentSequenceFileID){
        subset = viewer.accumulate("node.checked","parseInt(node['#'])");
        idlist=verarr({id: viewer.currentSequenceFileID, "rec-count": subset.length , name: viewer.currentSequenceFileName });
    }
    else {
        idlist  = viewer.accumulate((viewer.name=='list' ? "" : "node.leafnode && ")+"node.checked","{id: node.id, \"rec-count\": node['rec-count'], name: node.name }");
    }

    hiveseq_safeInsertIntoHiveseq(idlist, subset);

}

function hiveseq_safeInsertIntoHiveseq(idlist, subset, resetprevious)
{

    var hsview=vjDV.locate("dvHiveseqCompositionViewer."+set_sequences+".1");
    var oldData=hsview.getData(0).data.split("\n"),header="";
    if(oldData.length>0)header=oldData[0]+"\n";
    var content=(header.length?header:"") + (resetprevious ? "" : oldData.splice(header.length?1:0).join("\n") );

    for( var it=0; it< idlist.length; ++it ) {
        if(idlist[it].length<2)continue;

        if( subset && subset.length ){
            for( var is=0; is< subset.length; ){
                var inext;
                for(inext=is; inext<subset.length-1 && subset[inext]+1==subset[inext+1] ; ++inext ) // scan complete ranges
                    ;
                content+="\n"+idlist[it].id+","+subset[is]+","+subset[inext]+","+idlist[it].name;
                is=inext+1;
            }
        }
        else{
            content+="\n"+idlist[it].id+",1,"+idlist[it]["rec-count"]+","+idlist[it].name;
        }

    }

    var tbl=new vjTable(content, 0, vjTable_propCSV);

    hiveseq_InsertTblIntoHiveseq(tbl);

}

function hiveseq_InsertTblIntoHiveseq(tbl)
{
    tbl.enumerate("node.id=parseInt(node['id']);node.rangeStart=parseInt(node['range-start']);node.rangeEnd=parseInt(node['range-end']);");

    tbl.rows.sort(function hiveseq_rowSort(row1, row2)
    {
        var id1=parseInt(row1.id);
        var id2=parseInt(row2.id);
        if( id1 < id2 ) return -1;
        else if( id1 > id2) return 1;
        var range1=row1.rangeStart;
        var range2=row2.rangeStart;
        if( range1< range2 ) return -1;
        else return 1;
       });

    var t="",tForProc="";//hiveseq_hdr;
    var lstObjIDs = "";
    hiveseq_GTotalSequences=0;
    var oldId = 0;
    for (var ir=0, inext=0; ir< tbl.rows.length; ir=inext ) {
        var  cur=tbl.rows[ir];

        for(var inext=ir+1; inext<tbl.rows.length ; ++inext ){ // if this is not the last one we compare the current row with the following to it
            var  nxt =tbl.rows[inext];


            if( cur.id!=nxt.id )  // the same id ?
                break;


            if( cur.rangeStart=="search")
                break;
            //if(tbl.rows[ir].rangeEnd=="all")
            //    continue;
            if(cur.rangeEnd+1<nxt.rangeStart) // not continuous overlapping pieces ?
                break;
            if(cur.rangeEnd<nxt.rangeEnd) // take the maximal range
                cur.rangeEnd=nxt.rangeEnd;
         }
        t+=cur.id+","+cur.rangeStart+","+cur.rangeEnd+","+cur.name+"\n";
        if (cur.id != oldId){
            lstObjIDs += cur.id+";";
        }
        oldId = cur.id;
        if(cur.rangeEnd!="search" &&  !isNaN(cur.rangeEnd) && !isNaN(cur.rangeStart) && cur.rangeEnd>=cur.rangeStart   )
            hiveseq_GTotalSequences+=cur.rangeEnd-cur.rangeStart+1;

        tForProc+=cur.id+","+cur.rangeStart+","+cur.rangeEnd+"\n";
    }

    if ((lstObjIDs.split(';').length-1 > 1) && (process_initialPresets.AlgorithmsName == 2)){
        alert("dna-denovo contig extension algorithm can only accept one file at a time")
    }
    else {
        vjDS.dsHiveseqComposition.reload("static://" + t, true);
        algoProcess.setValue("hiveseqQry", tForProc);
    
        // To add parent_proc_ids
        // var lstObjIDs = hsview.accumulate("node.id");
        algoProcess.setValue("parent_proc_ids", lstObjIDs.substring(0,
                lstObjIDs.length - 1));
    
        if (!hiveseq_GTotalSequences) {
            visibool(algoProcess.dvname + "Submitter", false);
        } else {
            visibool(algoProcess.dvname + "Submitter", true);
        }        
    }
}

function hiveseq_saveHiveseq()
{
    var hsview=vjDV.locate("dvHiveseqCompositionViewer."+set_sequences+".1");var panelview=vjDV.locate("dvHiveseqCompositionViewer."+set_sequences+".0");
    //var content=hsview.accumulate("true","'obj://'+node.id+','+node['range-start']+','+node['range-end']");
    var content=hsview.accumulate("true","'obj://'+node.id+','+node['range-start']+','+node['range-end']");


    if(!isok(content)){
        alert("Cannot save empty sequence file.")
        return ;
    }
    var nm=document.forms["form-hiveseqComposition"].elements[panelview.container+"_hiveseq-assigned-name"].value;
    if( !isok(nm) || nm==default_name_suggestion){
        alert("Please choose a unique name for the generated sequence file.")
        return;
    }
    var frm=document.forms["form-hiveseqUploader"];
    frm.elements[0].name="file"+nm+".hiveseq";
    frm.elements[0].value=content.join("\n");
    frm.target=frm.name+"IFrame";
    frm.elements['submit'].click();
    setTimeout("vjDS.dsSequenceFileList.reload()",3000);
}

function hiveseq_deleteHiveseqEntries(viewer, node )
{
    var hsview=vjDV.locate("dvHiveseqCompositionViewer."+set_sequences+".1");
    var nodelist = hsview.accumulate("!(node.checked)","node");
    hsview.tblArr.rows=nodelist;
    hiveseq_InsertTblIntoHiveseq(hsview.tblArr);
}

