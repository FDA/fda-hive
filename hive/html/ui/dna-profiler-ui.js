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
google.load("visualization", "1", { packages: ["corechart"] });

var process_formName = "form-process";
var process_ID = docLocValue("id"); if (!process_ID) process_ID = 0;
var algoProcess, algoProgress;

gInitList += "process_init();";
setLocationTitle("HIVE-heptagon profiler");


var alignID=docLocValue("parent_proc_ids"); if(isok(alignID))alignID=alignID.split(",");
var genomeID=0; // reference genome(s) of alignID; used for clustering
var process_visibleParameters=["name","snpCompare","minCover","parent_proc_ids"];
var process_visualImage="";
var process_help=[];
var process_initialPresets={};//{name:""};
var process_cmdMode=docLocValue("cmdMode");
var process_qpsvc="dna-profiler";
var process_svc="svc-profiler";
var batch_alignment_redraw=true;

var process_showParentList=1;
var dna_hexagonHitList, dna_profilerProfile;

function isMode(mode)
{
    return (!process_cmdMode || (process_cmdMode.indexOf("-"+mode)==-1 && process_cmdMode.indexOf(mode)!=0)) ? false : true;
}

// for clustering
function load_genomeID()
{
    if (!alignID)
        return;

    linkCmd('objQry&raw=1&qry=("' + alignID.join(",") + '"%20as%20objlist).map({.subject}).reduce(function(x,y){x.append(y)})',
        "",
        function(param, text) {
            try {
                genomeID = JSON.parse(text);
                if (genomeID.join) {
                    genomeID = genomeID.join();
                }
                if (!genomeID) {
                    genomeID = 0;
                }
            } catch (e) {
                genomeID = 0;
            }
        }
    );
}
load_genomeID();

function process_init()
{
    gUserLoginAccess(0);
    var visualArray=new Array(
        {     name: 'dvProcess',
            //passive: !process_defaults_ui.visualElements.parameters,
            //role: 'input',
            title: "Variation calling parameters",
            //rightTitle:appended,
            icon: 'img/processSvc.gif',
            icoSize: 64,
            align: 'left',
            briefSpans: process_visibleParameters,
            briefSpanTag: "RV-",
            brief:"<table class='HIVE_sect1'><tr ><td id='dvHitListViewer' valign=top></td><td id='dvHitListInfoViewer' valign=top></td></tr></table>",
            popupCloser: true,
            isok:true},
        { name: 'dvProfilerZoom',
                onlyPopup:true,
                role: 'output',
                title: "Profile Zoom",
                align: 'left' ,
                popupCloser: true
        },
        {     name: 'dvAnnotZoom',
            onlyPopup:true,
            role: 'output',
            title: "Annot Zoom",
            align: 'left' ,
            popupCloser: true
        },
        {     name: 'dvhistpopup',
                  onlyPopup:true,
                  role: 'output',
                  title: "Coverage Histogram",
                  align: 'left' ,
                  popupCloser: true
              }
        /*,
        { name: 'dvProcessParentList',
              role: 'input',
              noicon:true,
              title:"<input type=button value='on selected alignments' />",
              align: 'right' ,
              noBGImage:true,
              popupCloser: true
          }*/
    );
    vjVIS.generate( visualArray );


    ///////////////////////////////////Process/Progress///////////////////////////////////////
    algoProcess = new valgoProcess(process_ID, process_formName, "dvProcess", process_qpsvc,process_svc, "    Analyse    ");
    algoProcess.callbackLoaded = process_inputLoaded;
    algoProcess.callbackChanged = process_inputChanged;
    algoProcess.initialPresets = isok(process_ID) ? null : process_initialPresets;
    algoProcess.subject_help=process_help;

    algoProcess.toolBar="processToolbar";
    algoProcess.generate(); // process_IDD
    //algoProcess.viewer.fldEnumerate="if(node.name=='parent_proc_ids') { node.title='Alignment(s)'; node.type=''; } ";


    visibool("dvProcessSubmitter", false);
    if(algoProcess.modeActive) {
        visibool("dvProcessSubmitterAll", true);
        return ;
    }
    visibool("dvProcessSubmitterAll", false);

    algoProgress = new valgoProgress(process_ID, process_formName, "Progress");
    algoProgress.callbackDoneComputing = process_doneComputing;
    algoProgress.generate();


    return;
}

function process_inputChanged(viewer, elem) {

    if (elem.fld.name == "batch_param" && batch_alignment_redraw) {
        var o = viewer.getElement("batch_value");
        if (!o)
            return;

        o.fld.value = "";
        o.fld.default_value = "";
        viewer.redraw();
        batch_alignment_redraw = false;
    }

    if (elem.fld.name == "batch_value") {
        viewer.redraw();
    }

}


function process_inputLoaded()
{
    var par=algoProcess.viewer.getElement( "parent_proc_ids");
    par.fld.constraint_data="svc-align*";
    par.fld.checkCallback=process_parentCheckCallback;
    par.fld.selectCallback=function (viewer,node){dna_hexagonHitList.reload(node.id,0,0);};

/*
    par=algoProcess.viewer.getElement( "subject");
    par.fld.constraint_data="svc-align*";
    par.fld.checkCallback=process_parentCheckCallback;
    par.fld.selectCallback=function (viewer,node){dna_hexagonHitList.reload(node.id,0,0);};
*/
    var nam=algoProcess.getValue("name");
    var subset=algoProcess.getValue("subSet");

    if(alignID) {
        algoProcess.setValue('parent_proc_ids',alignID);
    } else {
        alignID=algoProcess.getValue('parent_proc_ids',"array");
        load_genomeID();
    }
    var okToSubmit = isok(subset) && isok(alignID) && algoProcess.modeActive;

    if(okToSubmit) {
        if( !nam ) {
            algoProcess.setValueList({name: "query:"+"a=["+alignID+"] as objlist ;return a[0].name;"});
        }
    }
    if(algoProcess.modeActive){
        algoProcess.setValue("scissors", "hiveal");
        algoProcess.setValue("split", "parent_proc_ids");
    }

    if(!dna_hexagonHitList){ ///////////////////////////////////Input///////////////////////////////////////
        var node={_type:'svc-align',id:alignID[0]};
        var dvname=vjDV.add("dvHitListViewer", 350, 350);
        var dvinfo=vjDV.add("dvHitListInfoViewer",800,350);
        dna_hexagonHitList = vjHO.fullview(node._type,node,{obj:[dvname,dvinfo]});
        dna_hexagonHitList.isInput=true;
        dna_hexagonHitList.checkable = (algoProcess.modeActive) ? true : false;
        dna_hexagonHitList.callbackChecked = process_checkedReference;
        dna_hexagonHitList.callbackSelected = process_selectedReference;
        dna_hexagonHitList.algoProc=algoProcess;
        if(isok(subset))dna_hexagonHitList.subsetCount=str2ranges(subset,";");
    }

    visibool("dvProcessSubmitter", okToSubmit);
}


function process_parentCheckCallback(viewer,node,nn)
{
    if(!node)return;

    var url=vjDS[viewer.data].url;
    url=urlExchangeParameter(url,"prop_name","status"+(node.checked ? ",subject" :"") ) ;
    url=urlExchangeParameter(url,"prop_val","5"+(node.checked ? ","+node.subject : "" )) ;
    var checkedIds=viewer.accumulate("node.checked" , "node.id");

    //viewer.precompute="";
    //if(isok(checkedIds))viewer.precompute="if(arrayCross(["+checkedIds+"],verarr(node.id)))node.checked=1;";
    //vjDS[viewer.data].reload(url,true);
    //algoProcess.setValue('parent_proc_ids',checkedIds);
    dna_hexagonHitList.reload(node.id,0,0);
}

function process_checkedReference(viewer)
{
    if (algoProcess.modeActive) {
        algoProcess.setValue("subSet", viewer.accumulate("node.checked", "node.id").join(";"));
        process_inputLoaded();
    }
}

function process_doneComputing(viewer, reqid, stat, callcounter)
{
    if (stat >= 5 ){

        node={_type:'svc-profiler',id:process_ID};
        dvname=vjDV.add("dvProfilerViewer", 350, 350);
        dvinfo=vjDV.add("dvProfilerInfoViewer",900,350);
        var dvzoom=vjDV.add("dvProfilerZoomViewer",0.95*gPgW,450);
        var dvannotzoom=vjDV.add("dvAnnotZoomViewer",0.7*gPgW,350);
        var dvHistpopup=vjDV.add("dvhistpopupViewer",0.7*gPgW,350);
        algoProcess.reload(undefined,true);//to load files
        dna_profilerProfile = vjHO.fullview(node._type,node,{obj:[dvname,dvinfo,dvzoom,dvannotzoom,dvHistpopup]});
        dna_profilerProfile.algoProc = algoProcess;
        if(dna_hexagonHitList && dna_hexagonHitList.referenceNode){
            dna_profilerProfile.onFullviewRenderCallback = process_selectedReference(null,dna_hexagonHitList.referenceNode);
        }
        vjDS["ds"+algoProcess.toolBar].reload("innerText://ds"+algoProcess.toolBar+"DoneDV", true);
        visibool("resultBlock", true);
    }
}

function process_selectedReference(viewer, node)
{
    if (!dna_profilerProfile || !dna_profilerProfile.onSelectReferenceID || !node.id || ("" + node.id == "0") || node.id == "+")
        return;

    dna_profilerProfile.onSelectReferenceID(null,node);
    var viewer = vjDV.locate("dvProfilerViewer.annot-files.0");
}

function process_submitAll(button)
{
    if(!confirm("Do you want to build profiles for ALL references in this alignment?"))
        return;
      //alert("0:"+docLocValue('profx',0));
      if (docLocValue('cmdMode',0)=="profx" ) {
        algoProcess.setValue("svc", "dna-profx");
        algoProcess.setValue("slice", "1000000000000");
        algoProcess.setValue("split", "-");
        if (docLocValue('svc-profx-samtools',0) ) {
            algoProcess.setValue("profSelector", "svc-profx-samtools");
            //alert("profSelector set to svc-profx-samtools");
        }
        else if (docLocValue('svc-profx-cuffdiff',0) ) {
            algoProcess.setValue("profSelector", "svc-profx-cuffdiff");
            //alert("profSelector set to svc-profx-cuffdiff");
        }
      }
    algoProcess.setValue("subSet", "");
    visibool("dvProcessSubmitter", false);
    visibool("dvProcessSubmitterAll", false);
    algoProcess.onSubmitRequest();
    return ;
}



