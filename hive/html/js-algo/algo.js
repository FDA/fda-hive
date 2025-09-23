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
    this.modeResults = loadedID.length && loadedID[0] != '-';
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
    this.callbackOnSubmit=null;
    this.callbackSubmited=null;
    this.initialPresets=null;
    this.generate=function(forceID)
    {
        this.vjDV.add(this.dvname+"Viewer",this.width,this.height).forceLoadAll=true;

        this.vjDS.add("Retrieving Computational Parameters and Results", "ds"+this.dvname+"-vals" ,"static:
        this.vjDS.add("infrastructure: Algorithmic parameters specifications", "ds" + this.dvname + "-spec", "static:

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
                    constructionPropagateDown:10,
                    autoexpand:0,
                    showRoot:false,
                    autoDescription:0,
                    autoStatus:1,
                    autoDescription:0,
                    fldPresets:this.fldPresets ? this.fldPresets : null,
                    RVtag: this.RVtag,
                    formObject:document.forms[this.formName],
                    isok:true});


        var t="";
        var sysRVtags=["comment","system"];
        for( var ir=0; ir< sysRVtags.length; ++ir) {
            t+="<span id='RV-"+sysRVtags[ir]+"'></span>";
        }
        this.vjDS.add("infrastructure: RVs","dsRVTags","static:

        var batchRVtags=["batch", "batch_ignore_errors"];
        if (this.modeResults) {
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
        }
        else {
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
        
        this.makeBatchableButton(loadedID); 
           

        if(this.submitButon) {
            this.makeSubmitButton( this.submitButon );
        }


        if(this.toolBar) {
            this.vjDS.add("infrastructure: Constructing Toolbar", "ds"+this.toolBar, "innerHTML:
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
        if(qpSvc)
            this.qpSvc=qpSvc;
        this.vjDS["dsHelp"+this.dvname].reload("http:

        this.vjDS["ds"+this.dvname+"-spec"].reload("http:
        this.vjDS["ds"+this.dvname+"-vals"].reload(this.loadedID ? "http:

        this.viewer.cmdPropSet="?cmd=-qpProcSubmit&svc="+this.qpSvc;
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
        return this.viewer.getElementValue(name,which);
    };

    this.submit=function(cbFunc,cnts)
    {
        var proceed=true;
        var subbutton = gObject(this.dvname+"SubmitterInput");
        if(cbFunc)this.callbackSubmited=cbFunc;
        if(this.callbackOnSubmit) {
            if(!funcLink(this.callbackOnSubmit,this,this) ) {
                if(subbutton)subbutton.disabled = false;
                return 0;
            }
        }
        if(subbutton)subbutton.disabled=true;
        this.viewer.saveValues(null, "auto", "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");
    };
    this.makeSubmitButton=function( btnname )
    {
        var o=gObject( this.dvname+"Submitter") ;if(!o)return ;
        o.innerHTML="<input id='"+this.dvname+"SubmitterInput' type=button onClick='vjObjEvent(\"onSubmitRequest\",\""+this.objCls+"\")' name='BUTTON-"+this.dvname+"_submitter' size=20 value='"+sanitizeElementAttr(btnname)+"' />";
    };
    this.makeBatchableButton=function( loadID )
    {
        var o=gObject( this.dvname+"Batchable");
        if(!o) return;
        
        if (loadID === 'undefined' || loadID <= 0) {
               if(isMode('batch')) {
                   if (process_initialPresets.batch_svc === 'undefined') return;
                   o.innerHTML="<input type=button onClick=\"location.href='?cmd=" + process_initialPresets.batch_svc + "';\" value='Switch to Single Mode'></input>";
               } else {
                   if (process_qpsvc === 'undefined') return;
                   o.innerHTML="<input type=button onClick=\"location.href='?cmd=" + process_qpsvc + "&cmdMode=batch';\" value='Switch to Batch Mode'></input>";    
               }
        } else {
            o.style.display = "none";
        }
        
    };
    this.activateSubmitButton=function(cond)
    {
        if(!this.submitButon)return ;

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
        if(cmdMode)cmd+="&cmdMode="+cmdMode;
        this.setValue ("submitter" , cmd );
        if(this.modeActive)
            this.setValue ("action" , 2 );

        if(this.callbackLoaded)
            funcLink(this.callbackLoaded,viewer,this);
    };


    this.onRedirectProcSubmitted=function (param, text )
    {

        var reqID=0, objID=0;
        var nums=isok(text) ? text.split(",") : new Array();
        if(nums.length>=2) {
               reqID=parseInt(nums[0]);
               objID=parseInt(nums[1]);
        }
        if(!reqID || !objID) {
            alert("Error: could not submit the computation request!\n"+text);
            var a = gObject( this.dvname+"SubmitterInput");
            if( a) { a.disabled = false; }
            return ;
        }
        this.loadedID=objID;

        if(this.callbackSubmited) {
            funcLink(this.callbackSubmited,this,this.viewer);
        }
        else {
            document.location=makeCmdSafe(docLocValue("cmd"))+"&id="+this.loadedID+"&cmdMode="+docLocValue("cmdMode");

        }
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
            var par=algoProcess.getValue(namearr[i],"join");
            if( !par) {
                par = docLocValue(namearr[i]);
                if( isok(par) ) {
                    var o=new Object();
                    o[namearr[i]]=par.split(",");
                    this.setValueList(o);
                }
            }
        }
    };

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
