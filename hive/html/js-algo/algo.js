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

    this.modeActive=true; // passive mode values cannot be modified: we are just reviewing older computation
    this.modeResults = loadedID.length && loadedID[0] != '-';
    //if(parseInt(loadedID)<0){this.modeActive=false;loadedID=Math.abs(parseInt(loadedID));}
    if(loadedID.length && loadedID[0]!='-')this.modeActive=false;
    if(loadedID.length && loadedID[0]=='-')loadedID=loadedID.substring(1);
    this.loadedID=loadedID;
    this.formName=formname;
    this.dvname=dvname;
    if(!this.helpSize)
        this.helpSize={width:500};


    this.objCls="algo-"+cls+"-"+Math.random();
    vjObj.register(this.objCls,this); // needed to find this object in events by object container


    this.selfUpdate=0;

    this.graphResolution = 200;
    this.graphResolutionZoom = 200;
    if(document.all) { // stupid IE Cannot show more than some little number of HTML5 s
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }
}

// Note: dvname needs to equal progressTag in the corresponding valgoProcess() instance;
// by default, that is "Progress".
function valgoProgress(loadedID,  formname, tagname)
{
    valgoCore.call(this,"progress", loadedID,  formname, "dv"+tagname);

//    this.prg="Progress";
    this.callbackDoneComputing=null;
    this.dsname="ds"+tagname;

    this.generate=function()
    {
        this.vjDS.add("infrastructure: Summarizing Cloud Progress",this.dsname,"static:// ");
        this.vjDS.add("infrastructure: Summarizing Cloud Progress",this.dsname+"_download","static:// ");
        this.vjDS.add("infrastructure: Summarizing Cloud Report",this.dsname+"_info","static:// ");
        new vjProgressControl_InputsDataSource({name: this.dsname+"_inputs", url: "static://", title: "infrastructure: Summarizing Input Processes"}).registerDS();
        new vjProgressControl_OutputsDataSource({name: this.dsname+"_outputs", url: "static://", title: "infrastructure: Summarizing Output Processes"}).registerDS();
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

    this.svcProcType=svcProcType;// "svc-proc"; // upProc type
    //this.qpSvc="dna-hexagon"; // QP service to submit to
    this.width=1000;
    this.height=400;
    this.RVtag="RV";
    this.qpSvc=qpSvc+ver;
    //this.loadedID = loadedID;

    this.submitButon=submitButton;
    //if(submitAllButton)this.submitAllButton=submitAllButton;
    this.progressTag="Progress";
    this.childrenProcessesDV=null;

    //this.prg="Progress";
    //this.callbackDoneComputing=null;
    this.callbackLoaded=null;
    this.callbackOnSubmit=null;
    this.callbackSubmited=null;
    this.initialPresets=null;
    this.generate=function(forceID)
    {
        this.vjDV.add(this.dvname+"Viewer",this.width,this.height).forceLoadAll=true;

        this.vjDS.add("Retrieving Computational Parameters and Results", "ds"+this.dvname+"-vals" ,"static://" );
        this.vjDS.add("infrastructure: Algorithmic parameters specifications", "ds" + this.dvname + "-spec", "static://");
        //this.vjDS.add("Loading...", "ds"+this.dvname+"-files" , "static://" );

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
                    //setDoneCallback:submittedCallback,
                    autoexpand:0,
                    showRoot:false,
                    //hideControls:true,
                    autoDescription:0,
                    //oneTitleForMultiValueArray :true,
                    //showRoot: true,
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
        this.vjDS.add("infrastructure: RVs","dsRVTags","static://"+t);

        var batchRVtags=["batch", "batch_ignore_errors"];
        if (this.modeResults) {
            // force record viewer to show batch_children_proc_ids only in read-only results mode
            batchRVtags.push("batch_children_proc_ids");
        }
        for( var ir=0; ir< batchRVtags.length; ++ir) {
            t+="<span id='RV-"+batchRVtags[ir]+"'></span>";
        }
        this.vjDS.add("infrastructure: RVs","dsBatchTags","static://"+t); 

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

        //this.vjDV[this.dvname+"Viewer"].add("parameters","dna","tab", [ this.recordViewer ] );

        /*
        if(this.loadedID && this.childrenProcessesDV && gObject(this.childrenProcessesDV)  ){
            this.vjDV.add(this.childrenProcessesDV,800,400);
            this.vjDS.add("infrastructure: Exporting Children Processes", "ds"+this.dvname+"ChildrenProcesses", "http://?cmd=objList&mode=csv&type=svc-*&prop=id,svcTitle,status,progress100,name,uri,svc,created&prop_name=parent_proc_ids&prop_val="+this.loadedID);

            var viewerChildrenList=vjPAGE.initStandardProcessViewer(this.childrenProcessesDV, "ds"+this.dvname+"ChildrenProcesses", this.formName , "javascript:alert('o')");
            viewerChildrenList.precompute="node.url='?cmd='+node.svc+'&id='+node.id;";
            this.vjDV[this.childrenProcessesDV].add("subsequent computations","process","tab", [ viewerChildrenList  ] );
         //alert(this.childrenProcessesDV)
            this.vjDV[this.childrenProcessesDV].render();
            this.vjDV[this.childrenProcessesDV].load();
        }*/


        var hlpArr=[{description:"infrastructure: Algorithmic parameters help documentation", dsName:"dsHelp"+this.dvname, name:"help",url:"static://" }];
        if(isok(this.subject_help))
            hlpArr=hlpArr.concat(verarr(this.subject_help));
        //this.vjDS.add("infrastructure: Algorithmic parameters help documentation", "dsHelp"+this.dvname,"static://");

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
        
        // Checks for batchability and toggles between batch and single
        this.makeBatchableButton(loadedID); 
           

        if(this.submitButon) {
            this.makeSubmitButton( this.submitButon );
        }
        //if(this.submitAllButton)
            //this.makeSubmitAllButton( this.submitAllButton.title, this.submitAllButton.inputdvname);


        if(this.toolBar) {
            this.vjDS.add("infrastructure: Constructing Toolbar", "ds"+this.toolBar, "innerHTML://ds"+this.toolBar+"WaitingDV" );
            this.vjDV.add("dv"+this.toolBar,"100%","100%").frame="none";
            this.vjDV["dv"+this.toolBar].add( "toolbar", "", "menu",new vjPanelView({data:"ds"+this.toolBar,formObject:document.forms[this.formName],iconSize:48,showTitles:true}));
            this.vjDV["dv"+this.toolBar].render();
            this.vjDV["dv"+this.toolBar].load();
        }

    };
    /*
    this.generateParentsList=function(parentList)
    {
        if( !parentList ) return ;
        this.parentList=parentList;

        if(!this.parentList.dv)this.parentList.dv= "dvProcessParentListViewer";
        var dv=this.parentList.dv;
        if(gObject(dv)){
            this.parentList.dsname="ds"+this.parentList.dv;
            vjDS.add("infrastructure: Algorithmic processes",this.parentList.dsname,
                    "http://?cmd=objList&"+this.parentList.dsurl+"&mode=csv&info=1&status=done&cnt=20");

            var viewerPanel = new vjPanelView({
                data:["dsVoid",this.parentList.dsname],
                formObject: document.forms[this.formName],
                iconSize:24,
                hideViewerToggle:true,
                rows: [
                        {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS."+this.parentList.dsname+".reload(null,true);"},
                        {name:'pager', icon:'page' , title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                        { name: 'search', align: 'right', type: ' search',order:10, isSubmitable: true, title: 'Search', description: 'search sequences by ID',order:'1', url: "?cmd=objFile&ids=$(ids)" }
                        ],
                isok:true });

            var viewerProcessList=new vjTableView( {
                data: this.parentList.dsname, // "dsProcessParentList",
                icon:"processSvc",
                bgColors:['#f2f2f2','#ffffff'] ,
                formObject: document.forms[this.formName],
                cols:[
                      {name:new RegExp(/.* $$$$$$$$$$$$$$$$$$$$$$$$$$ /), hidden:true },
                      {name:'name', hidden: false, title:'Name' , order: 2.5} ,
                      {name:'^id$', hidden: false, title:'ID' , order: 1 } ,
                      {name:'svcTitle', hidden: false, title:'Task', order: 4} ,
                      {name:'created', hidden: false, title:'Created', order: 4.5, type: 'datetime'} ,
                      {name:'uri', title:'Source', order: 5} ,
                      {name:'progress100', hidden: false, title:"Progress%" , align: "right", type: 'percent', order: 2} ,
                      {name:'status', order: 2.1233, hidden: false, title:"Status" , align: "left", type: 'choice', choice: ['unknown','waiting','processing','running','suspended','done','killed','failure','error','unknown'] }
                      ],
                checkable: true ,
                checkCallback: this.parentList.checkCallback ,
                selectCallback: this.parentList.selectCallback,
              //objectsDependOnMe:depends,
                geometry:{ width:'90%'},
                isNrefreshOnLoad:true,
                maxTxtLen:32,
                iconSize:0,
                isok:true});

            vjDV.add(dv, 500, 350).frame = 'notab';
            vjDV[dv].add("alignment processes", "processSvc", "tab", [viewerPanel,viewerProcessList ]);
            this.vjDV[dv].render();
            this.vjDV[dv].load();
        }
    }
*/
    this.setSvcProc=function(svcProcType, svcProcObjID, qpSvc)
    {
        this.loadedID=svcProcObjID;
        if(svcProcType)
            this.svcProcType=svcProcType;
        if(qpSvc)
            this.qpSvc=qpSvc;
        //alert(qpSvc);
        //return ;
        //alert("http://help/hlp."+this.svcProcType+".html");
        this.vjDS["dsHelp"+this.dvname].reload("http://help/hlp."+this.svcProcType+".html");
        //alert("http://help/hlp."+this.svcProcType+".html");

        this.vjDS["ds"+this.dvname+"-spec"].reload("http://?cmd=propspec&type="+this.svcProcType ,false);
        this.vjDS["ds"+this.dvname+"-vals"].reload(this.loadedID ? "http://?cmd=propget&mode=csv&ids="+this.loadedID+"&files=*" : "static:// ", false);

        this.viewer.cmdPropSet="?cmd=-qpProcSubmit&svc="+this.qpSvc;
        this.viewer.hiveId=this.modeActive ? this.svcProcType : this.loadedID ;
//        this.viewer.objType=this.loadedID ? this.loadedID  : this.svcProcType ;
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
    /*
    this.setValueList=function(arr)
    {
        for ( fld in arr )  {
            var qryStr=arr[fld];

            if(qryStr.indexOf("eval:")==0);
                qryStr=this.viewer.computeExpression(qryStr.substr(5),"join");
            if(qryStr.indexOf("objQuery:")==0)
                linkCmd("objQry&raw=1&qry="+escape(qryStr.substr(9)),{obj:this, val: fld },function (par,txt){par.obj.setValue(par.val,txt);});
            else
                this.viewer.changeElementValue(fld,arr[fld]);
        }
    };
    */
    /*
    this.setQryFields=function (qryStr, field, doEval)
    {

        if(doEval)
            qryStr=this.viewer.computeExpression(qryStr,"join");

        var text = qryStr;
        text = text.replace(/ /mg, "%20");
        text = text.replace(/[\r\n]/mg, "");
        text = text.replace(/\+/mg, "%2B");
        text = text.replace(/=/mg, "%3D");
        //var qry="o%3D"+alignID+"%20as%20obj;%20return%20cat(o.subject.name,%20\"%20versus%20\",%20o.query.name);";
        linkCmd("objQry&raw=1&qry="+text,{obj:this, val: field },function (par,txt){par.obj.setValue(par.val,txt);});
    }
*/

    this.getValue=function(name,which)
    {
        return this.viewer.getElementValue(name,which);
    };

    this.submit=function(cbFunc,cnts)
    {
        var proceed=true;
        /*if(this.submissionLimitField){
            proceed=false;
            var extension="";
            if(cnts===undefined){
                cnts=this.getValue(this.submissionLimitField);
                if(cnts!="" && cnts!="-" && cnts!="all"){
                    if(!this.submitSeperator)this.submitSeperator=",";
                    cnts=cnts.split(this.submitSeperator).length;
                }
                else
                    cnts="all posible requests";
            }
            if(cnts!==1)extension="s";
            if(!this.submitThreshold)this.submitThreshold=10;
            if(typeof(cnts)=='number' && cnts<this.submitThreshold)proceed=true;
            else if(confirm('You are about to submit '+cnts+' request'+extension)) proceed=true;
        }
        if(proceed){
        */
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
        //}
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
    /*this.makeSubmitAllButton=function( btnname)
    {
         var o=gObject( this.dvname+"SubmitterAll") ;if(!o)return ;
        o.innerHTML="<input type=button onClick='vjObjEvent(\"onSubmitAllRequest\",\""+this.objCls+"\")' name='BUTTON-"+this.dvname+"_all_submitter' size=20 value='"+sanitizeElementAttr(btnname)+"' />";
    };*/
    this.onSubmitRequest=function(  )
    {
        this.submit();
    };


/*
    this.onSubmitAllRequest=function(objCls,inputDVname,elementsChanges)
    {
        if(this.submitAllButton.elementsChange!==undefined){
            this.submitAllButton.elementsChange=verarr(this.submitAllButton.elementsChange);
            for(var i=0 ; i<this.submitAllButton.elementsChange.length ; ++i){
                this.setValue(this.submitAllButton.elementsChange[i].name,this.submitAllButton.elementsChange[i].value);
            }
        }

        var cntRefs=0;
        if (this.submitAllButton.inputdvname) {
            var viewer = this.vjDS[this.vjDV.locate(this.submitAllButton.inputdvname + "Viewer.list.1").data].data;
            cntRefs=parseInt(viewer.substring(viewer.search(/info/)).split(",")[2]);
        }
        else
            cntRefs='all possible';
        this.submit(null,cntRefs);

    };
*/


    this.attachProgress=function(dsprogress)
    {
        if(vjDS[dsprogress]){
            var req=this.getValue( "reqID" );
            var progress_url = "static://unknown";
            var info_url = "static://";
            if (isok(this.loadedID)) {
                progress_url = "http://?cmd=-qpRawCheck&showreqs=0&reqObjID=" + this.loadedID;
                info_url = "http://?cmd=-qpReqInfo&reqObjID=" + this.loadedID;
            } else if (isok(req)) {
                progress_url = "http://?cmd=-qpRawCheck&showreqs=0&req=" + req;
                info_url = "http://?cmd=-qpReqInfo&req=" + req;
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
                    dsInputs.reload("static://", true);
            }

            var dsOutputs = vjDS[dsprogress+"_outputs"];
            if (dsOutputs) {
                if (this.loadedID.length && !this.modeActive)
                    dsOutputs.makeURL({id:this.loadedID, count:20}, true);
                else
                    dsOutputs.reload("static://", true);
            }
        }
    };

    this.onRecordLoaded=function(viewer,text)
    {
        if(this.initialPresets){
            this.setValueList(this.initialPresets);
        }

        var nodearr=viewer.fldTree.accumulate("node.is_batch_fg","node");
        //var nodearr=[];
        var constraint_text="";
        for (var iin=0;iin<nodearr.length;++iin){
            if(iin>0)constraint_text+="|";
            constraint_text+=nodearr[iin].name+"///"+nodearr[iin].title;
        }

        var el=viewer.getElement("batch_param");
        if(el){
            var fld=el.fld;
            fld.constraint_data=constraint_text;
            viewer.redraw();
        }


        //if(this.modeActive && !isok(this.getValue("name")))
        //    this.setValue("name", (new Date()).toDateString());
        if(this.vjDS["ds"+this.progressTag])
            this.attachProgress("ds"+this.progressTag);

        //alert ( docLocValue("cmd") ) ;
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
//            alert(this.callbackSubmitted);
            funcLink(this.callbackSubmited,this,this.viewer);
        }
        else {
            //var url=urlExchangeParameter(document.location,"id",this.loadedID);
            //url=urlExchangeParameter(url,"parent_proc_ids","-");
//            alert("?cmd="+docLocValue("cmd")+"&id="+this.loadedID)
            document.location="?cmd="+docLocValue("cmd")+"&id="+this.loadedID+"&cmdMode="+docLocValue("cmdMode");

        }
            //document.location=urlExchange"?cmd=dna-hexagon&id="+this.loadedID;
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
            var par=algoProcess.getValue(namearr[i],"join");//.split(",");//.split(";").join(",");
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











// ./debug-Linux-x86_64/bin/dna.cgi "cmd=alView&start=0&cnt=50&info=1&mySubID=1&objs=16715&raw=1&bust=1391117178935&sessionID=1d9RCe2gfJPkRhBzQZzz%5Fx0apWuaM1R0tXgFOe0gAG%40UtgtkawdGgYAANz9kykAAAAB"

