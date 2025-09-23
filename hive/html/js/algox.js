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


function valgoCore(cls, loadedID,  formname, dvname)
{
    this.vjDS=vjDS;
    this.vjDV=vjDV;
    this.vjVIS=vjVIS;

    this.modeActive=true;
    if(loadedID.length && loadedID[0]!='-')this.modeActive=false;
    if(loadedID.length && loadedID[0]=='-')loadedID=loadedID.substring(1);
    this.loadedID=loadedID;
    this.formName=formname;
    this.dvname=dvname;
    if(!this.helpSize)
        this.helpSize={width:500};


    this.objCls="algo-"+cls+"-"+Math.random();
    vjObj.register(this.objCls,this);


    this.selfUpdate=0;

    this.graphResolution = 200;
    this.graphResolutionZoom = 200;
    if(document.all) {
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }
}

function valgoProgress(loadedID,  formname, tagname)
{
    valgoCore.call(this,"progress", loadedID,  formname, "dv"+tagname);

    this.callbackDoneComputing=null;
    this.dsname="ds"+tagname;

    this.generate=function()
    {
        this.vjDS.add("infrastructure: Summarizing Cloud Progress",this.dsname,"static:
        this.vjDS.add("infrastructure: Summarizing Cloud Progress",this.dsname+"_download","static:
        this.vjDS.add("infrastructure: Summarizing Cloud Report",this.dsname+"_info","static:
        new vjProgressControl_InputsDataSource({name: this.dsname+"_inputs", url: "static:
        new vjProgressControl_OutputsDataSource({name: this.dsname+"_outputs", url: "static:
        this.vjDV.add(this.dvname+"Viewer","100%","300").frame='notab';
        var tab = vjDV[this.dvname+"Viewer"].add("progress", "process", "tab", new vjProgressControl({
            data : {
                progress: this.dsname,
                progress_download: this.dsname+"_download",
                inputs: this.dsname+"_inputs",
                outputs: this.dsname+"_outputs",
                progress_info:this.dsname+"_info"
            },
            formName: this.formName,
            doneCallback: this.callbackDoneComputing
        }));
        tab.viewtoggles = -1;
        this.vjDV[this.dvname+"Viewer"].render();
        this.vjDV[this.dvname+"Viewer"].load();
    };
}

function valgoProcess(loadedID,  formname, dvname, qpSvc, svcProcType, submitButton )
{
    valgoCore.call(this,"process", loadedID,  formname, dvname);
    var ver="";
    var vl=""+document.location;
    var ps1=vl.indexOf("version~");
    if(ps1!=-1){
        var ps2= vl.indexOf("/",ps1+1) ;
        if(ps2==-1)ps2=vl.indexOf("=",ps1+1) ;
        if(ps2==-1)ver=vl.substring(ps1+8);
        else ver=vl.substring(ps1+7,ps2);
    }

    this.svcProcType=svcProcType;
    this.width=1000;
    this.height=400;
    this.RVtag="RV";
    this.qpSvc=qpSvc+ver;

    this.submitButon=submitButton;
    this.progressTag="Progress";
    this.childrenProcessesDV=null;

    this.callbackLoaded=null;
    this.callbackSubmited=null;
    this.initialPresets=null;
    
    this.generate=function(forceID)
    {
        this.vjDV.add(this.dvname+"Viewer",this.width,this.height).forceLoadAll=true;

        this.vjDS.add("Retrieving Computational Parameters and Results", "ds"+this.dvname+"-vals" ,"static:
        this.vjDS.add("infrastructure: Algorithmic parameters specifications", "ds" + this.dvname + "-spec", "static:

        var batchFldPreset = {
            batch_svc: {
                constraint:'choice+', constraint_data:'single
            batch_param: {
                is_optional_fg: isBatch() ? 0 : 1
            },
            batch_value: {
                is_optional_fg: isBatch() ? 0 : 1
            }
        };
        if(this.fldPresets && !this.fldPresets.batch_svc ) {
            this.fldPresets["batch_svc"]=batchFldPreset.batch_svc;
        }

        this.recordViewer=new vjRecordView( {
                    kind:"valgoProcess",
                    data: this.loadedID ? [ "ds"+this.dvname+"-spec", "ds"+this.dvname+"-vals" ] : [ "ds"+this.dvname+"-spec"],
                    hiveId: forceID ? forceID : this.svcProcType ,
                    objType: forceID ? forceID : this.svcProcType ,
                    cmdPropSet:"?cmd=-qpProcSubmit&svc="+this.qpSvc,
                    readonlyMode: this.modeActive ? false : true,
                    callbackRendered : "function:vjObjFunc('onRecordLoaded','"+this.objCls+"')",
                    onChangeCallback : "function:vjObjFunc('onRecordChanged','"+this.objCls+"')",
                    onAddElementCallback : "function:vjObjFunc('onRecordAddedElement','"+this.objCls+"')",
                    accumulateWithNonModified:true,
                    showReadonlyInNonReadonlyMode:this.showReadonlyInNonReadonlyMode,
                    cloneMode:isResubmit(),
                    constructionPropagateDown:10,
                    autoexpand:0,
                    showRoot:false,
                    autoDescription:0,
                    autoStatus:3,
                    autoDescription:0,
                    fldPresets: this.fldPresets ? cpyObj(batchFldPreset,this.fldPresets) : batchFldPreset,
                    RVtag: this.RVtag,
                    formObject:document.forms[this.formName],
                    isok:true});


        var t="";
        var sysRVtags=["comment","system"];
        for( var ir=0; ir< sysRVtags.length; ++ir) {
            t+="<span id='RV-"+sysRVtags[ir]+"'></span>";
        }
        this.vjDS.add("infrastructure: RVs","dsRVTags","static:
        t = "";

        var batchRVtags=["batch", "batch_ignore_errors"];
        if (isResultsMode()) {
            batchRVtags.push("batch_children_proc_ids");
        }
        for( var ir=0; ir< batchRVtags.length; ++ir) {
            t+="<span id='RV-"+batchRVtags[ir]+"'></span>";
        }
        this.vjDS.add("infrastructure: RVs","dsBatchTags","static:

        this.sysviewer=new vjHTMLView({prohibitReRender:true, data:"dsRVTags"});
        this.batchviewer=new vjHTMLView({prohibitReRender:true, data:"dsBatchTags"});

        this.vjDV[this.dvname+"Viewer"].add("parameters","dna","tab", [ this.recordViewer ] );
        this.vjDV[this.dvname+"Viewer"].add("system parameters","dna","tab", [ this.sysviewer] );
        this.vjDV[this.dvname+"Viewer"].add("batch parameters","dna","tab", [ this.batchviewer] );
        if( isBatch() ) {
            this.vjDV[this.dvname+"Viewer"].tabs["batch parameters"].hidden=true;
            this.vjDV[this.dvname+"Viewer"].tabs["batch parameters"].invisible=true;
        }
        if (this.tabToAdd != undefined && this.tabToAdd.length){
            for (var iTab = 0; iTab < this.tabToAdd.length; ++iTab){
                this.vjDS.add(this.tabToAdd[iTab].title, this.tabToAdd[iTab].dsname, this.tabToAdd[iTab].url);
                this.tabToAdd[iTab].viewer.data = this.tabToAdd[iTab].dsname;
                this.vjDV[this.dvname+"Viewer"].add(this.tabToAdd[iTab].tabname,this.tabToAdd[iTab].icon,"tab", [ this.tabToAdd[iTab].viewer] );
            }
        }

        var hlpArr=[{description:"infrastructure: Algorithmic parameters help documentation", dsName:"dsHelp"+this.dvname, name:"help",url:"static:
        if(isok(this.subject_help))
            hlpArr=hlpArr.concat(verarr(this.subject_help));

        var hlpDV=(this.dvname+"Viewer");
        if(gObject(this.dvname+"HelpViewer") ) {
            hlpDV=this.dvname+"HelpViewer";
            this.vjDV.add(hlpDV,(this.helpSize && this.helpSize.width) ? this.helpSize.width : this.width ,(this.helpSize && this.helpSize.height) ? this.helpSize.height : this.height );
            valgoProcess_helpTabIndex = this.vjDV[this.dvname+"Viewer"].tabs.length-1;
        }
        else {
            valgoProcess_helpTabIndex = this.vjDV[this.dvname+"Viewer"].tabs.length;
            hlpDV=this.dvname+"Viewer";
        }
        
        for (var i=0; i<hlpArr.length; i++) {
            var h = hlpArr[i];
            var name = h.name ? h.name : "help";
            var icon = h.icon ? h.icon : "help";
            var description = h.description ? h.description : "infrastructure: Help document for this particular page";
            var dsname = h.dsName ? h.dsName : "dsSubjectHelp_tab_" + i;
            var url = h.url;
            this.vjDS.add(description, dsname, url);
            this.vjDV[hlpDV].add(name, icon, "tab", [ new vjHelpView ( { data: dsname} ) ] );
        }



        this.viewer=this.vjDV.locate(this.dvname+"Viewer"+".parameters.0");
        this.setSvcProc( null, forceID ? forceID : this.loadedID, this.qpSvc);

        this.vjDV[this.dvname+"Viewer"].render();
        this.vjDV[this.dvname+"Viewer"].load();

        if(hlpDV!=this.dvname+"Viewer"){
            this.vjDV[hlpDV].render();
            this.vjDV[hlpDV].load();

        }
        
           

        if(this.submitButon) {
            this.makeSubmitButton( this.submitButon );
        }


        if(this.toolBar) {
            this.vjDS.add("infrastructure: Constructing Toolbar", "ds"+this.toolBar, "static:
            
            this.vjDV.add("dv"+this.toolBar,"100%","100%").frame="none";
            this.vjDV["dv"+this.toolBar].add( "toolbar", "", "menu",new vjPanelView({data:"ds"+this.toolBar,formObject:document.forms[this.formName],iconSize:48,showTitles:true}));
            this.vjDV["dv"+this.toolBar].render();
            this.vjDV["dv"+this.toolBar].load();
        }

    };

    this.setSvcProc=function(svcProcType, svcProcObjID, qpSvc)
    {
        this.loadedID=svcProcObjID;
        if(svcProcType)
            this.svcProcType=svcProcType;

        if(qpSvc) {
            this.qpSvc=qpSvc;
        }
        this.vjDS["dsHelp"+this.dvname].reload("http:

        this.vjDS["ds"+this.dvname+"-spec"].reload("http:
        this.vjDS["ds"+this.dvname+"-vals"].reload(this.loadedID ? "http:

        var serviceToLaunch = this.qpSvc;
        if(isBatch() && !isResultsMode()) {
            serviceToLaunch = 'svc-batcher';
        }
        this.viewer.cmdPropSet="?cmd=-qpProcSubmit&svc="+serviceToLaunch;
        this.viewer.hiveId=this.modeActive ? this.svcProcType : this.loadedID ;
        this.viewer.objType=this.svcProcType  ;
        this.viewer.load();

    };

    this.reload=function(newurl, force)
    {
        this.vjDS["ds"+this.dvname+"-vals"].reload(newurl, force);
    };

    this.setValueList=function(obj)
    {
        this.viewer.changeValueList(obj);

    };



    this.setValue=function(name,value)
    {
        this.viewer.changeElementValue(name,value);
    };
    
    this.getValue=function(name,which)
    {
        if (this.viewer)
            return this.viewer.getElementValue(name, which);
        return;
    };
    
    this.isValidateBatchParemeters = function() {
        return true;
        var batchfld = this.viewer.fldTree.findByName("batch");
        var batchNodes = this.viewer.nodeTree.findByAttribute("fld",batchfld,null,true);
        var maxChunks = [];
        for(var i = 0 ; i < batchNodes.length ; ++i ) {
            var batch_list = batchNodes[i].children[0];
            var batch_sat = batchNodes[i].children[1].value?true:false;
            maxChunks[i] = -1;
            for(var j = 0 ; j < batch_list.children.length ; j+=2 ) {
                var batch_params = batch_list.children[j];
                var batch_fldName = batch_params.value;
                var batch_val = batch_list.children[j+1];
                if( !batch_fldName || !batch_val ) continue;
                batch_fldName = verarr(batch_fldName)[0];
                var fldB = this.viewer.fldTree.findByName(batch_fldName);
                batch_val = Int(batch_val.value);
                if(!batch_val) {
                    alert("Missing value for batch parameter:\""+fldB.title+"\"");
                    return 0;
                }
                var nodeB = this.viewer.nodeTree.findByAttribute("fld",fldB,null,true);
                var batch_cnt = (Int((nodeB.length-1)/batch_val)+1);
                if(nodeB.length < batch_val) {
                }
                if(maxChunks[i] < 0 )
                    maxChunks[i] = batch_cnt;
                else {
                    if(batch_sat && maxChunks[i] < batch_cnt) {
                        maxChunks[i] = batch_cnt;
                    } else if( !batch_sat && maxChunks[i] > batch_cnt) {
                        maxChunks[i] = batch_cnt;
                    }
                }
            }
        }
        var tot = 1;
        for ( var t =0 ; t < maxChunks.length ; ++t ) tot *= maxChunks[t];
        if( !confirm("You are about to submit "+tot+ " "+ this.svcProcType + " computations.\nProceed?") )
            tot = 0;
        return tot;
    }
    
    this.submit=function(cbFunc,cnts)
    {
        var proceed=true;
        if(cbFunc)this.callbackSubmited=cbFunc;
        if( isBatch() && !this.isValidateBatchParemeters() ) {
            return;
        }
        else
            this.viewer.saveValues(null, "auto", "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");
    };
    
    this.makeSubmitButton=function( btnname )
    {
        var _submitButtonDIV=gObject( this.dvname+"Submitter");
        if( !_submitButtonDIV ) {
            return;
        }
        
        _submitButtonDIV.innerHTML="<input id='"+this.dvname+"SubmitterInput' type=button onClick='this.disabled=\"disabled\";vjObjEvent(\"onSubmitRequest\",\""+this.objCls+"\")' name='BUTTON-"+this.dvname+"_submitter' size=20 value='"+sanitizeElementAttr(btnname)+"' />";
        
        if ( !this.modeActive ) {
            var _submitButtonElement = gObject( this.dvname+"SubmitterInput");
            _submitButtonElement.disabled = true;
        }
    };
   
    this.activateSubmitButton=function(cond)
    {
        if ( !this.submitButon ) {
            return;
        }

        visibool(this.dvname + "Submitter", (cond && this.modeActive) );
        if(cond && this.modeActive) {
            var a = gObject( this.dvname+"SubmitterInput");
            a.disabled = false;

        }
    };
    this.onSubmitRequest=function(  )
    {
        this.submit();
    };


    this.attachProgress=function(dsprogress)
    {
        if(vjDS[dsprogress]){
            var req=this.getValue( "reqID" );
            var progress_url = "static:
            var info_url = "static:
            if (isok(this.loadedID)) {
                progress_url = "http:
                info_url = "http:
            } else if (isok(req)) {
                progress_url = "http:
                info_url = "http:
            }
            vjDS[dsprogress].reload(progress_url, true );


            var dsDownload=vjDS[dsprogress+"_download"];
            if(dsDownload)
                dsDownload.reload(progress_url, true );

            var dsInfo=vjDS[dsprogress+"_info"];
            if(dsInfo)
                dsInfo.reload(info_url, false );

            var dsInputs = vjDS[dsprogress+"_inputs"];
            if (dsInputs) {
                if (this.loadedID.length && !this.modeActive)
                    dsInputs.makeURL({id:this.loadedID, count:20}, true);
                else
                    dsInputs.reload("static:
            }

            var dsOutputs = vjDS[dsprogress+"_outputs"];
            if (dsOutputs) {
                if (this.loadedID.length && !this.modeActive)
                    dsOutputs.makeURL({id:this.loadedID, count:20}, true);
                else
                    dsOutputs.reload("static:
            }
        }
    };

    this.onRecordLoaded=function(viewer,text)
    {
        if(this.initialPresets){
            this.setValueList(this.initialPresets);
        }

        var nodearr=viewer.fldTree.accumulate("node.is_batch_fg","node");
        var constraint_text="";
        for (var iin=0;iin<nodearr.length;++iin){
            if(iin>0)constraint_text+="|";
            constraint_text+=nodearr[iin].name+"
        }
        var el=viewer.getElement("batch_param");
        if(el){
            var fld=el.fld;
            fld.constraint_data=constraint_text;
            viewer.redraw();
        }

        if(this.vjDS["ds"+this.progressTag])
            this.attachProgress("ds"+this.progressTag);

        var cmd=docLocValue("cmd");
        var cmdMode=docLocValue("cmdMode");
        if(isok(cmdMode))cmd+="&cmdMode="+cmdMode;
        this.setValue ("submitter" , cmd );
        if(this.modeActive)
            this.setValue ("action" , 2 );

        if(this.callbackLoaded)
            funcLink(this.callbackLoaded,viewer,this);
    };


    this.onRedirectProcSubmitted=function (param, text )
    {
        var parse_res = vjQP.parseQProcSubmitResponse(text,true);
        if(!parse_res)return ;
        var reqID=parse_res[0], objID=parse_res[1];
        this.loadedID=objID;

        if(this.callbackSubmited) {
            if( !funcLink(this.callbackSubmited,this,this.viewer) )
                return ;
        }
        var urlArguments="";
        if (docLocValue("batchMode")) {
            urlArguments+="&batchMode=" + docLocValue("batchMode");
        }
        
        if (docLocValue("cmdMode")) {
            urlArguments += "&cmdMode="+docLocValue("cmdMode");
        }
        document.location=makeCmdSafe(docLocValue("cmd"))+"&id="+this.loadedID + urlArguments;
    };


    this.onRecordChanged=function(viewer,elem)
    {
        if(this.callbackChanged)
            funcLink(this.callbackChanged,viewer,elem);
    };

    this.onRecordAddedElement=function(viewer,elem)
    {
        if(this.callbackAddedElement)
            funcLink(this.callbackAddedElement,viewer,elem);
    };

    this.readFromDocLoc=function(namearr)
    {
        for( var i=0; i<namearr.length ; ++ i) {
                par = docLocValue(namearr[i]);
                if( isok(par) ) {
                    var o=new Object();
                    o[namearr[i]]=par.split(",");
                    this.setValueList(o);
                }
        }
    };
    
}

function valgoProcess_helpCallback(viewer,flv)
{
    var dvName = flv.name;
    var dv = vjDV[dvName+"Viewer"];
    var tb = dv.tabs[valgoProcess_helpTabIndex];
    if (tb) {
        dv.select(tb.name,true);
    }
}

function valgoProcess_init(algoDiv)
{
    gAjaxPreSubmissionCallback=function(){
        gUserLoginAccess(0);
    }
    var proc_visibleParameters = typeof valgoProcess_visibleParameters!="undefined"?valgoProcess_visibleParameters:undefined;
    valgoProcess_DefaultVisibleParamenter = proc_visibleParameters.concat(valgoProcess_DefaultVisibleParamenter);
    var dvName = "dvProcess";
    var visualArray=new Array(
            {
                name : dvName,
                role : 'input',
                title : typeof valgoProcess_parameterTitle!="undefined"?valgoProcess_parameterTitle:undefined,
                align : 'left',
                help: true,
                helpcallback: valgoProcess_helpCallback,
                icon: 'img/processSvc.gif',
                bgImage: typeof valgoProcess_visualImage!="undefined"?valgoProcess_visualImage:undefined,
                icoSize:64,
                briefSpans: valgoProcess_DefaultVisibleParamenter,
                brief: typeof valgoProcess_brief!="undefined"?valgoProcess_brief:undefined,
                popupCloser: true,
                briefSpanTag: "RV-",
            isok:true});
    vjVIS.generate(visualArray);

    algoProcess = new valgoProcess(valgoProcess_ID, valgoProcess_formName,dvName, valgoProcess_qpsvc,valgoProcess_svc, "    "+valgoProcess_submitButtonName+"    ");
    if(valgoProcess_inputLoaded)algoProcess.callbackLoaded = valgoProcess_inputLoaded;
    algoProcess.callbackChanged = valgoProcess_inputCommandModeChanged;
    if(valgoProcess_initialPresets)algoProcess.initialPresets = isok(valgoProcess_ID) ? null : valgoProcess_initialPresets;
    if(valgoProcess_help)algoProcess.subject_help=valgoProcess_help;
    algoProcess.fldPresets=valgoProcess_fields;
    algoProcess.toolBar="processToolbar";
    if(typeof valgoProcess_tabToAdd != "undefined" ){ algoProcess.tabToAdd = valgoProcess_tabToAdd;}
    
    
    algoProcess.generate();

    if(isBatch() && !isResultsMode()) { 
        algoProcess.recordViewer.getValidateSeparatorCb = function(fld_name) {
            if(fld_name && verarr(algoProcess.recordViewer.getElementValue("batch_param", "array")).indexOf(fld_name) >= 0) {
                return ';';
            } else {
                return null;
            }
        }
        if(!algoProcess.initialPresets)algoProcess.initialPresets={};
        algoProcess.initialPresets.batch_svc = algoProcess.qpSvc ;
        algoProcess.qpSvc = "svc-batcher";
    }
    
    if(algoProcess.modeActive)
        return ;
    algoProgress = new valgoProgress(valgoProcess_ID, valgoProcess_formName, "Progress");
    algoProgress.callbackDoneComputing = valgoProcess_computingFinished;
    algoProgress.generate();

    
        
    return;
}

function valgoSectionHeader(title, icon, sectionsControl, rightside, docwrite, params)
{
    var t="";
    var barclass = "HIVE_bar";
    if (params && params.extraBarClasses) {
        for (var i=0; i<params.extraBarClasses.length; i++) {
            barclass += " " + params.extraBarClasses[i];
        }
    }

    idstring = "";
    if (params && params.id)
        idstring = " id='" + params.id + "'";

    t+="<table border='0' class='" + barclass + "'" + idstring + " >";
        t+="<tr>";
            t+="<td width='1%'>"+ovis(sectionsControl,"sectVis")+"</td>";
            t+="<td align=left width='1%' ><img src='"+icon+"' height=48 /></td>";
            t+="<td class='HIVE_section_title' width='"+(rightside ? 15 : 98)+"%' >"+title+"</td>";
            if(isok(rightside))t+="<td width='80%'></td><td align='right'>"+(rightside ?  rightside : "" )+"</td>";
        t+="</tr>";
    t+="</table>";
    if(docwrite)
        document.write(t);
    else return t;
}

function valgoSectionPageStart(docwrite)
{
    var txt="<form name='"+valgoProcess_formName+"' method=get POST action='dna.cgi' enctype='multipart/form-data'>"+
        "<table width='100%' border='0' id='dvWholePanel'>"+
            "<tr style='vertical-align:top'>"+
                "<td width='48%' id='dvLeftPanel'></td>"+
                "<td width='2%' align=center>"+
                    "<div class='HIVE_oneliner' style='width:100%'>"+
                     "<div id='dvProcess-visual' ></div>"+
                     "<div id='dvProcessBatchable'></div>"+
                     "<img border=0 width=48 src='img/algoin.gif' /><br/>"+
                     "<div id='dvProcessSubmitter'></div>"+
                     "<div id='dvProgressViewer'></div>"+
                     "<img border=0 src='img/algoout.gif' width=48 />"+
                     "<div id='dvProcessHelpViewer' class='HIVE_onelined' ></div>"+
                    "</div>"+
                "</td>"+
                "<td width='48%' id='dvRightPanel'></td>"+
            "</tr>"+
        "</table>"+
        "<table width='100%' border='0'>"+
            "<tr>"+
                "<td width='1%'></td>"+
                    "<td width='98%' align=center>"+
                    "<span id='resultBlock' class='sectHid'>"+
                    "<div class='HIVE_sect1' >"+
                         valgoSectionHeader('Results','img/results.gif','sectOutput1' , "<span class='sectVis' id='dvprocessToolbar'></span>", false)+
                         
    "";
    if(docwrite)document.write(txt);
    else return txt;
}
function valgoSectionPageEnd(docwrite)
{
    var txt=
                    "</div>"+
                    "</span>"+
                "</td>"+
                "<td width='1%'></td>"+
            "</tr>"+
        "</table>"+
    "</form>";
    if(docwrite)document.write(txt);
    else return txt;
}
    


function valgoProcess_computingFinished(viewer, reqid, stat)
{
    if(valgoProcess_doneComputing){
        valgoProcess_doneComputing(viewer,reqid,stat);
    }
    
    if (stat >= 5) {
        vjDS["ds"+algoProcess.toolBar].reload("static:
    }
    
}
function valgoProcess_inputCommandModeChanged(viewer, elem) {
    for (f in valgoProcess_cmdModeLst) {
        var svcs = valgoProcess_cmdModeLst[f];
        if (elem.fld.name == f) {
            var newsvc = "";
            var v = verarr(elem.value)[0];

            var bool = false;

            for (var is = 0; is < svcs.length && v; ++is) {
                var look = svcs[is];
                if (look.indexOf("bool:") == 0) {
                    bool = true;
                    look = look.substring(5);
                }

                var _service = v.substring(10);
                var poss = v.indexOf(look);
                if (_service != look)
                    continue;
                var pose = v.substr(poss).indexOf("-");
                if (pose == -1)
                    pose = v.length;
                newsvc = v.substring(poss, pose);
                break;
            }
            var removeBatch = false;
            var addBatch = false;
            if (bool) {
                if (is == 0) {
                    if (newsvc == "single") {
                        removeBatch=true;
                    }
                    newsvc = "";
                }
                else {
                    newsvc = svcs[1];
                    if (newsvc == "batch") {
                        addBatch = true;
                    }
                }
            } else {
                v = valgoProcess_cmdMode;
                for (var is = 0; is < svcs.length; ++is) {
                    if (v != svcs[is]) {
                        continue;
                    }
                    var poss = v.indexOf(svcs[is]);
                    var pose = poss + svcs[is].length;
                    newsvc = v.substring(0, poss) + newsvc + v.substr(pose);
                    break;
                }
            }
            if (addBatch) {
                newsvc = urlExchangeParameter("" + document.location, "batchMode", "true");
            } else if (removeBatch) {
                newsvc = urlExchangeParameter("" + document.location, "batchMode", "");
                newsvc = newsvc.replace("&batchMode=", "");
            } else {
                newsvc = urlExchangeParameter("" + document.location, "cmdMode", newsvc);
            }
                    
            document.location = newsvc;
            return;
        }
    }
    if (valgoProcess_inputChanged)
        valgoProcess_inputChanged(viewer, elem);
}

function isResubmit()
{
    if ( valgoProcess_ID && (valgoProcess_ID[0] == "-" || valgoProcess_ID < 0) ) {
        return true;
    }
    return false;
}

function isMode(mode)
{
    return ( (!valgoProcess_cmdMode || (valgoProcess_cmdMode.indexOf("-"+mode)==-1 && valgoProcess_cmdMode.indexOf(mode)!=0))) ? false : true;
}

function isBatch()
{
    var svc = algoProcess?algoProcess.getValue('svc'):'';
    if( svc ) {
        if(svc.indexOf('batch')!=-1)
            return true;
        else 
            return false;
    }
    var batchmode;
    if (valgoProcess_batchMode == "true") {
        batchmode=true;
    } else {
        batchmode=false;
    }
    return ( batchmode && !isResultsMode() );
}

function isResultsMode()
{
    return valgoProcess_ID && !isResubmit();
}


var valgoProcess_inputChanged;
var valgoProcess_inputLoaded;
var valgoProcess_doneComputing;
var valgoProcess_ID = docLocValue("id");if (!valgoProcess_ID) valgoProcess_ID = 0;
var valgoProcess_formName = "form-process";
var valgoProcess_parameterTitle="Advanced Parameters";
var valgoProcess_visibleParameters=["name"];
var valgoProcess_DefaultVisibleParamenter = [];
var valgoProcess_visualImage="";
var valgoProcess_help=[];
var valgoProcess_initialPresets={name:"> "};
var valgoProcess_cmdMode=docLocValue("cmdMode");
var valgoProcess_batchMode=docLocValue("batchMode");
var valgoProcess_qpsvc="generic-launcher";
var valgoProcess_svc="svc-generic-launcher";
var valgoProcess_fields={};
var valgoProcess_tabToAdd;
var valgoProcess_submitButtonName="Submit";
var valgoProcess_cmdModeLst= {
        batch_svc: ["bool:single","batch"]
    };
if( !isResultsMode() ) {
    valgoProcess_DefaultVisibleParamenter.push( "batch_svc" );
    if( isBatch() ) {
        valgoProcess_DefaultVisibleParamenter.push( "batch" );
        valgoProcess_DefaultVisibleParamenter.push("batch_ignore_errors");
    }
}

var algoProcess, algoProgress;

var valgoToolbarWaitingList="type,align,order,name,title,icon,path,url,description\n"+
        "html,right,0,next,<b>while you are waiting: &rarr;</b>,,/next,,Choose what would you like to do next\n"+  
        ",right,1,files,Modify and Resubmit,img/recRevert.gif,/resubmit,"+urlExchangeParameter(document.location,"id","-"+valgoProcess_ID)+",Modify parameters and resubmit this computation using the same template\n"+
        "";
        
var valgoToolbarDoneList="";

