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
javaScriptEngine.include("js/vjAlignmentView.js");

vjHO.register('svc-alignment-multiple').Constructor=function ()
{

    if(this.objCls)return;
    this.typeName="svc-alignment-multiple";
    this.autoClickedReference=1;
    this.objCls="obj-svc-alignment-multiple"+Math.random();
    vjObj.register(this.objCls,this);
    this.idNulldsName="default";

    this.onArchiveSubmit = function (viewer,content){
        var icon;
        var txt = "";
        if(!isok(content) ) {
            txt = "Program error: could not submit dataset to archiver";
            icon = "img/redInfo.png";
        }
        else if(content.indexOf("error")>=0){
            txt = content;
            icon = "img/redInfo.png";
        }
        else {
            txt = "Your data succesfully submitted for digestion (reqId:"+content+ ")!\n";
            txt += "You may monitor the process in you inbox under the 'data loading' tab";
            icon = "img/done.gif";
        }
        alertI(txt,undefined,{icon:icon});
    };
    this.dsQPBG_digest = vjDS.add('preparing to archive','ds'+this.objCls+'_QPBG_digest','static:
    
    this.fullview=function(node,dv)
    {
        this.load(dv,node.id);
        if(this.onFullviewLoadCallback) {
            funcLink(this.onFullviewLoadCallback, this);
        }

        this.construct();
        if(this.onFullviewRenderCallback) {
            funcLink(this.onFullviewRenderCallback, this);
        }
        return this;
    };

    this.preview = function(node,dv)
    {
        this.parent.preview('svc',node,dv);
        if(!node.status || parseInt(node.status)<5)return ;
        this.load(dv,node.id);
        if(this.onPreviewLoadCallback) {
        funcLink(this.onPreviewLoadCallback, this);
        }

        this.construct();
        if(this.onPreviewRenderCallback) {
            funcLink(this.onPreviewRenderCallback, this);
        }
        return this;
    };

    this.mobileview = function(node,dv)
    {
        this.parent.mobileview('svc',node,dv);
        this.load(dv,node.id);
        if(this.onMobileLoadCallback) {
        funcLink(this.onMobileLoadCallback, this);
        }

        this.construct();
        if(this.onMobileRenderCallback) {
            funcLink(this.onMobileRenderCallback, this);
        }
        return this;
    };

    this.urlSet ={
            'alStack' :{
                active_url: "http:
                title: "Visualizing alignments in stack"
            },
            'consensus' :{
                active_url: "http:
                title: "Generating consensus"
            },
            'overlap' :{
                active_url: "http:
                title: "Generation overlap"
            },
            'downloads' : {
                active_url:"static:
                    "Multiple Alignment,download,ico-file,alStack,&cnt=0&mySubID=1&multiple=1&rangeEnd=100,\n" +
                    "Alignments in fasta,download,dna,alFasta,&wrap=100&info=1&mySubID=1&multiple=1&objs=3031174&raw=1&cnt=0,\n"+
                    "Consensus in fasta,download,dna,alConsensus,&multiple=1&wrap=100,\n"+
                    "Overlap in fasta,download,dna,alConsensus,&multiple=1&wrap=100&overlap=1,",
                title: "infrastructure: Creating download menu"
            },
            'help' : {
                active_url:"http:
                inactive_url:"http:
                title: "Infrastructure: Creating help"
            }
    };


    this.construct=function(){
        if(!this.loaded || !this.current_dvORtab)return;
        this.constructed=true;
        
        if(this.mode=='preview' || this.mode=='mobileview'){
            var dv=this.current_dvORtab.obj;
            var origNumofTabs=dv.tabs.length;
            this.dvname=dv.name;
            this.dvinfo=dv.name;
            dv.addTab("stack","list",[this.viewers['stack_panel'],this.viewers['stack']]);
            dv.render();
            dv.load('rerender');
        }
        else{
            this.dvname=this.current_dvORtab.name;

            this.current_dvORtab.addTab("stack","dna",[this.viewers['stack_panel'],this.viewers['stack']]);
            this.current_dvORtab.addTab("consensus","dna",[this.viewers['consensus']]);
            this.current_dvORtab.addTab("overlap","dna",[this.viewers['overlap']]);
            this.current_dvORtab.addTab("downloads","table",[this.viewers['downloads']]);
            this.current_dvORtab.addTab("help","help",[this.viewers['help']]);

            this.current_dvORtab.render();
            this.current_dvORtab.load('rerender');
        }
    };

    this.load=function(dvORtab,id)
    {
        if (this.mode=='preview' || this.mode=='mobileview'){

            this.loadedID=id;
            this.formName='';
            var formNode=gAncestorByTag(gObject(dvORtab.obj.name),"form");
            if(formNode)
                this.formName=formNode.attributes['name'].value;

            this.loaded=true;
            this.constructed=false;
            this.current_dvORtab=dvORtab;

            this.dvname=this.current_dvORtab.obj.name;
            this.dvinfo=this.current_dvORtab.obj.name;

            this.addviewer('stack_panel,stack', new vjAlignmentMultipleStackControl ({
                data: 'alStack',
                formName:this.formName,
                width:"100%",
                isok:true}));

        }
        else{

            this.current_dvORtab=dvORtab.obj;
            if(this.current_dvORtab instanceof Array) {
                this.current_dvORtab = this.current_dvORtab[0];
            }
            
            this.loadedID=id;
            this.formName=['',''];
            var formNode=gAncestorByTag(gObject(this.current_dvORtab.name),"form");
            if(formNode)
                this.formName[0]=formNode.attributes['name'].value;
            if(formNode)
                this.formName[1]=formNode.attributes['name'].value;

            this.loaded=true;
            this.constructed=false;

            this.dvname=this.current_dvORtab.name;
            this.dvinfo=this.current_dvORtab.name;


            this.addviewer('stack_panel,stack', new vjAlignmentMultipleStackControl ({
                data: 'alStack',
                formName:this.formName[1],
                isok:true}));

            this.addviewer('downloads', new vjAlignmentDownloadsView ({
                data: 'downloads',
                formName:this.formName[1],
                selectCallback: "function:vjObjFunc('onPerformReferenceOperation','" + this.objCls + "')",
                   isok:true}));
            
            this.addviewer('consensus', new vjAlignmentConsensusView ({
                data: 'consensus',
                formName:this.formName[1],
                isok:true}));
            
            this.addviewer('overlap', new vjAlignmentConsensusView ({
                data: 'overlap',
                formName:this.formName[1],
                isok:true}));

            this.addviewer('help', new vjHelpView ({
                data: 'help',
                formName:this.formName[1],
                isok:true}));
        }
    };

    this.onPerformReferenceOperation = function (viewer, node, ir, ic) {
        var oper = node.operation;
        var args = node.arguments;
        try {
            var params = JSON.parse(node.params);
        } catch (e) {
            var params = undefined;
        }
        var url;

        url = this.makeReferenceOperationURL(viewer, node, node.operation, node.arguments, params);
        
        if(viewer.tblArr.hdr[ic].name=="down"){
            url = urlExchangeParameter(url, "down", '1');

            if( docLocValue("backend",0,url) ){
                var saveAs = "o"+this.loadedID+"-"+oper+"-"+this.referenceID+".fa";
            }
            else{
                document.location = url;
            }
        }
        else if (viewer.tblArr.hdr[ic].name=="arch") {
            url = urlExchangeParameter(url, "arch", 1);
            var dstName = node.data + " ("+this.loadedID+")";
            var ext = "-";
            switch (node.arch) {
            case "dna":
                ext = "ma";
                dstName += ".fasta";
                break;
            case "dnaold":
                ext = "fasta";
                dstName += ".fasta";
                break;
            default :
                ext= "-";
            }
            
            url = urlExchangeParameter(url, "arch_dstname", dstName);
            url = urlExchangeParameter(url, "ext", ext);
            this.dsQPBG_digest.reload("qpbg_http:
        }
    };

    this.makeReferenceOperationURL = function (viewer, node, oper, args, params) {
        
        var qtySamAligns = 0;
        
        var url = "?cmd=" + oper + "&objs=" + this.loadedID;
        if (this.reqID)
            url += "&req=" + this.reqID;

        url+=args;
        return url;
    };


    this.reload = function(loadedID, reqid, autoClickedReference)
    {
        if (loadedID)
            this.loadedID = loadedID;
        if (!this.loadedID)
            return;

        var url = "http:
                + "&start=0&cnt=50";
        if (this.profilerList)
            url += "&childProcessedList=" + this.subsetCount;
        if (reqid) {
            this.reqID = reqid;
            url += "&req=" + this.reqID;
        }
        if (autoClickedReference) {
            this.autoClickedReference = autoClickedReference;
            this.viewers["hitlist"].callbackRendered = "function:vjObjFunc('onLoadedHitList','"
                    + this.objCls + "')";
        }
        this.getDS('hitlist').reload(url, true);

    };

    this.update = function(proc) {
        this.algoProc = proc;
        var checkedIDS = this.viewers["hitlist"].accumulate("node.checked", "node.id");
        return isok(checkedIDS) ? 1 : 0;
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};




