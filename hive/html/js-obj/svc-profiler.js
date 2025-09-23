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
javaScriptEngine.include("js/vjProfilerView.js");

vjHO.register('svc-profiler').Constructor=function ()
{

    if(this.objCls)return;
    this.dsQPBG = vjDS.add('preparing to download','ds'+this.objCls+'_QPBG','static:
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
        if( this.onFullviewLoadCallback ) {
            funcLink(this.onFullviewLoadCallback, this);
        }

        this.construct();
        if( this.onFullviewRenderCallback ) {
            funcLink(this.onFullviewRenderCallback, this);
        }
        this.isRendered=true;
    };

    this.preview = function(node,dv)
    {
        this.parent.preview('svc',node,dv);
        if(!node.status || parseInt(node.status)<5)return ;
        this.load(dv,node.id);
        if( this.onPreviewLoadCallback ) {
            funcLink(this.onPreviewLoadCallback, this);
        }

        this.construct();
        if( this.onPreviewRenderCallback ) {
            funcLink(this.onPreviewRenderCallback, this);
        }
        this.isRendered=true;
    };

    this.mobile = function(node,dv)
    {
        this.parent.preview('svc',node,dv);
        if(!node.status || parseInt(node.status)<5)return ;
        this.load(dv,node.id);
        if( this.onPreviewLoadCallback ) {
            funcLink(this.onPreviewLoadCallback, this);
        }
        
        this.construct();
        if( this.onPreviewRenderCallback ) {
            funcLink(this.onPreviewRenderCallback, this);
        }
        this.isRendered=true;
    };
    
    
    this.typeName="svc-profiler";
    this.graphResolution = 200;
    this.graphResolutionZoom = 200;
    if(document.all) {
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }
     customizeOptions = new Array();

    this.windowZoomStack = new Array({windwStart:0,windwEnd:0});
    this.urlStack = new Array();

    this.seqalign_SNPprofile_cmd = "cols=0-2,7-18,21-26&hdr=1&minmaxCols=3-15&minmaxMainCol=5&resolution="+this.graphResolution;

    this.seqalign_SNPprofile_cmd_zoom = "cols=0-2,7-18,21-26&hdr=1";

    this.seqalign_SNPprofile_cmdX = "cols=0-12&hdr=1&minmaxCols=1-12" +
        "&resolution="+this.graphResolution;

    this.seqalign_SNPprofile_cmdX_zoom = "cols=0-12&hdr=1";

    this.seqalign_SNPprofile_cmd_subrange_dl = "hdr=1";

    this.autoClickedReference=1;

    this.objCls="obj-svc-profiler"+Math.random();
    vjObj.register(this.objCls,this);
    this.urlSet ={
            'hitlist': {
                active_url: "http:
                objs:'objs',
                title: "Retrieving list of hits"
            },
            'zoom_alView':{
                loadInactive:true,
                active_url:"http:
                header:"Element #,#,Direction,Start,Alignment,End,Sequence,Repeats,Position,PosHigh,Motif Start,Motif End",
                title: "Prepering alignments"
            },
            'zoom_alStack' :{
                loadInactive:true,
                active_url: "http:
                title: "Visualizing alignments in stack"
            },
            'zoom_alMutBias' :{
                loadInactive:true,
                active_url: "http:
                title: "Creating mutation bias diagram"
            },
            'zoom_profile' : {
                loadInactive:true,
                active_url:this.seqalign_SNPprofile_cmd_zoom,
                title: "Building profile diagram"
            },
            'zoom_profileX' : {
                loadInactive:true,
                active_url:this.seqalign_SNPprofile_cmdX_zoom,
                title: "infrastructure: Creating profilerX"
            },
            'zoom_orf' : {
                loadInactive:true,
                active_url:"http:
                title: "Retrieving open reading frames",
            },
            'zoom_snp-calls-graph' :{
                loadInactive:true,
                active_url: "http:
                title: "Retrieving snps calls"
            },
            'summary' : {
                loadInactive:true,
                active_url:"http:
                title: "Composing summary"
            },
            'full_summary' : {
                loadInactive:true,
                active_url:"http:
                title: "Composing full summary"
            },
            'contigs' : {
                loadInactive:true,
                active_url:"http:
                title: "Constructing contigs"
            },
            'annot-files' : {
                active_url:"http:
                title: "Finding relevant annotation files"
            },
            'profile' : {
                loadInactive:true,
                active_url:this.seqalign_SNPprofile_cmd,
                title: "Building profile diagram"
            },
            'profileX' : {
                loadInactive:true,
                active_url:this.seqalign_SNPprofile_cmdX,
                title: "infrastructure: Creating profilerX"
            },
            'profile_subrange_dl' : {
                loadInactive: false,
                active_url:this.seqalign_SNPprofile_cmd_subrange_dl,
                title: "Building SNP profile for position range"
            },
            'noise' : {
                loadInactive:true,
                active_url:"http:
                title: "Creating noise profiles"
            },
            'noise_integral' : {
                loadInactive:true,
                active_url:"http:
                title: "Infrastructre: Computing noise integrals"
            },
            'freq_histogram' : {
                loadInactive:true,
                active_url:"http:
                title: "Infrastructre: Computing frequency histogram"
            },
            'freq_histogram_popup' :
            {
                loadInactive:false,
                active_url:"http:
                title: "Infrastructre: Computing coverage frequency histogram"
            },
            'freq_histogram_integral' : {
                loadInactive:true,
                active_url:"http:
                title: "Infrastructre: Computing frequency histogram integral"
            },
            'consensus' : {
                loadInactive:true,
                active_url:"http:
                title: "Building consensus"
            },
            'snp-calls' : {
                loadInactive:true,
                active_url:"http:
                title: "Calling snps"
            },
            'snp-calls-graph' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http:
                title: "Ploting snp calls"
            },
            'snp-calls-graph-zoom' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http:
                title: "Ploting snp calls (zoom)"
            },
            'orf-zoom' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http:
                title: "Displaying open reading frames (zoom)"
            },
            'orf' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http:
                title: "Displaying open reading frames"
            },
            'downloads' : {
                loadInactive:true,
                active_url:"static:
                inactive_url:"static:
                title: "infrastructure: creating list of downloadables"
            },
            'help' : {
                active_url:"http:
                inactive_url:"http:
                doNotChangeMyUrl : true,
                title: "infrastructure: building help page"
            }

    };

    this.resetViewers = function(){
        for(var i in this.viewers){
            delete this.viewers[i];
        }
        if(this.viewers)
            this.viewers.length=0;
        this.parent['svc'].resetViewers();
    };

    this.construct=function()
    {
        if(!this.loaded || !this.current_dvORtab)return;
        this.constructed=true;

        if(this.mode=='preview'){
            var dv=this.current_dvORtab.obj;
            var origNumofTabs=dv.tabs.length;
            this.dvname=dv.name;
            this.dvinfo=dv.name;
            dv.addTab("profile","table",[this.viewers['hitlist'],this.viewers['profile_coverage']]).columns=2;
            dv.render(true);
            dv.load('rerender');
        }
        else if(this.mode=='mobile'){
            var dv=this.current_dvORtab.obj;
            var origNumofTabs=dv.tabs.length;
            this.dvname=dv.name;
            this.dvinfo=dv.name;
            dv.addTab("profile","table",[this.viewers['hitlist'],this.viewers['profile_coverage']]).columns=2;
            dv.render(true);
            dv.load('rerender');
        }
        else{

            this.dvname=this.current_dvORtab[0].name;
            this.dvinfo=this.current_dvORtab[1].name;

            this.current_dvORtab[0].addTab("summary","table",[this.viewers['summary']]);

            this.current_dvORtab[0].addTab("downloads","table",[this.viewers['downloads']]);
            this.current_dvORtab[0].addTab("gaps and contigs","dna",[this.viewers['contigs']]);
            this.current_dvORtab[0].addTab("annot-files","graph",[this.viewers['annot_files']]);

            this.current_dvORtab[1].addTab("profile","graph",[this.viewers['profile_menu'],this.viewers['profile_coverage'],this.viewers['profile_unaligned_terminus'],this.viewers['profile_disbalance'],this.viewers['profile_entropy'], this.viewers['profile_quality'],this.viewers['profile_snp'],this.viewers['profile_indels']]).viewtoggles = 1;
            this.current_dvORtab[1].addTab("help","help",[this.viewers['help']]);
            this.current_dvORtab[1].addTab("sequencing noise","graph",[this.viewers['noise'],this.viewers['noise_integral']]);
            this.current_dvORtab[1].addTab("frequency histogram","graph",[this.viewers['freq_histogram'],this.viewers['freq_histogram_integral']]);
            this.current_dvORtab[1].addTab("consensus","dna",[this.viewers['consensus_menu'],this.viewers['consensus']]);
            this.current_dvORtab[1].addTab("SNP calls","dna",[this.viewers['snp_calls_panel'],this.viewers['snp_calls']]);
            this.current_dvORtab[1].addTab("ORF Visualization","graph",[this.viewers['annot_panel'],this.viewers['annot_orf_table'],this.viewers['annot_snp_calls_graph'],this.viewers['annot_orf_graph']]);

            this.current_dvORtab[2].addTab("profile","graph",[this.viewers['zoom_profile_panel'], this.viewers['zoom_profile_coverage'], this.viewers['zoom_profile_unaligned_terminus'],this.viewers['zoom_profile_disbalance'], this.viewers['zoom_profile_entropy'], this.viewers['zoom_profile_quality'],this.viewers['zoom_profile_quality_acgt'], this.viewers['zoom_profile_snp'], this.viewers['zoom_profile_indels'],this.viewers['zoom_profile_acgt_entropy']]).viewtoggles = 1;
            this.current_dvORtab[2].addTab("Alignments","dna",[this.viewers['zoom_alignment_panel'],this.viewers['zoom_alignment']]);
            this.current_dvORtab[2].addTab("Stack","dna",[this.viewers['zoom_stack_panel'],this.viewers['zoom_stack_mutbias'],this.viewers['zoom_stack']]);

            this.current_dvORtab[3].addTab("ORF Visualization","graph",[this.viewers['zoom_annot_panel'],this.viewers['zoom_annot_snp_calls_graph'],this.viewers['zoom_annot_orf_graph']]);

            this.current_dvORtab[4].addTab("Coverage","graph",[this.viewers['freqHistPopup']]);

            this.current_dvORtab[0].render();
            this.current_dvORtab[1].render();
            this.current_dvORtab[2].render();
            this.current_dvORtab[3].render();
            this.current_dvORtab[4].render();

            this.current_dvORtab[0].load('rerender');
            this.current_dvORtab[1].load('rerender');

        }

    };



    this.load=function(dvORtab,id)
    {
        this.loadedID=id;
        this.loaded=true;
        this.constructed=false;

        if(this.mode=='preview'){
            this.formName='';
            var formNode=gAncestorByTag(gObject(dvORtab.obj.name),"form");
            if(formNode)
                this.formName=formNode.attributes['name'].value;


            this.current_dvORtab=dvORtab;

            this.dvname=this.current_dvORtab.obj.name;
            this.dvinfo=this.current_dvORtab.obj.name;

            this.addviewer('hitlist_panel,hitlist', new vjAlignmentHitListControl({
                data:'hitlist',
                formName:this.formName,
                width:'20%',
                selectCallback: "function:vjObjFunc('onSelectReferenceID','" + this.objCls + "')",
                isok:true}));
            this.viewers['hitlist'].callbackRendered = "function:vjObjFunc('onLoadedHitList','" + this.objCls + "')";

            this.addviewer('profile_coverage', new vjProfilerSNPProfileCoverage({
                data: 'profile',
                width:'80%',
                height:240,
                formName:this.formName,
                isok:true}));

            this.addviewer('null', new vjHTMLView ({data:"dsVoid",width:"80%"}));


        }
        else{

            this.formName=['','','','',''];

            var formNode=gAncestorByTag(gObject(dvORtab.obj[0].name),"form");
            if(formNode)
                this.formName[0]=formNode.attributes['name'].value;

            formNode=gAncestorByTag(gObject(dvORtab.obj[1].name),"form");
            if(formNode)
                this.formName[1]=formNode.attributes['name'].value;

            formNode=gAncestorByTag(gObject(dvORtab.obj[2].name),"form");
            if(formNode)
                this.formName[2]=formNode.attributes['name'].value;

            formNode=gAncestorByTag(gObject(dvORtab.obj[3].name),"form");
            if(formNode)
                this.formName[3]=formNode.attributes['name'].value;

            formNode=gAncestorByTag(gObject(dvORtab.obj[4].name),"form");
            if(formNode)
                this.formName[4]=formNode.attributes['name'].value;

            this.current_dvORtab=dvORtab.obj;

            this.dvname=this.current_dvORtab[0].name;
            this.dvinfo=this.current_dvORtab[1].name;
            this.dvzoom=this.current_dvORtab[2].name;
            this.dvzoomannot=this.current_dvORtab[3].name;


            this.dvhistpopup=this.current_dvORtab[4].name;

            this.dvzoom_visual=this.dvzoom.replace("Viewer","");
            this.dvzoomannot_visual=this.dvzoomannot.replace("Viewer","");
            this.dvhistpopup_visual=this.dvhistpopup.replace("Viewer","");


            var prof_flV=vjVIS.flVs[this.dvzoom_visual];
            var profzoomVIS=undefined;
            if(prof_flV){
                profzoomVIS=new Object();
                profzoomVIS.obj=vjVIS;
                profzoomVIS.flV=prof_flV;
            }

            var annot_flV=vjVIS.flVs[this.dvzoomannot_visual];
            var annotzoomVIS=undefined;
            if(annot_flV){
                annotzoomVIS=new Object();
                annotzoomVIS.obj=vjVIS;
                annotzoomVIS.flV=annot_flV;
            }

            var freqhistpop_flV=vjVIS.flVs[this.dvhistpopup_visual];
            var freqhistpopVIS=undefined;
            if (freqhistpop_flV)
            {
                freqhistpopVIS=new Object();
                freqhistpopVIS.obj=vjVIS;
                freqhistpopVIS.flV=freqhistpop_flV;
            }

            this.addviewer('summary',  new vjProfilerSummaryView ({
                data: 'summary',
                width:'80%',
                formName:this.formName[0],
                isok:true}));

            this.addviewer('downloads', new vjProfilerDownloadsView ({
                data: 'downloads',
                width:800,
                formName:this.formName[0],
                selectCallback: "function:vjObjFunc('onPerformProfileOperation','" + this.objCls + "')",
                isok:true}));

            this.addviewer('contigs', new vjProfilerContigView ({
                data: 'contigs',
                formName:this.formName[0],
                isok:true}));

            this.addviewer('annot_files',new vjProfilerAnnotationListView ({
                data: 'annot-files',
                formName:this.formName[0],
                isok:true}));

            this.addviewer('profile_menu,profile_coverage,profile_unaligned_terminus,profile_disbalance,profile_entropy,profile_quality,profile_snp,profile_indels', new vjProfilerSNPProfileControl ({
                data: {'profile':'profile', 'profileX': 'profileX'},
                width:800,
                selectCallback: "function:vjObjFunc('onZoomProfileOperation','" + this.objCls + "')",
                formName:this.formName[1],
                isok:true}));

            this.addviewer('help', new vjHelpView ({
                data: 'help',
                formName:this.formName[1],
                isok:true}));

            this.addviewer('noise,noise_integral', new vjProfilerNoiseControl ({
                data: {'noise':'noise','integral':'noise_integral'},
                formName:this.formName[1],
                isok:true}));

            this.addviewer('freq_histogram,freq_histogram_integral', new vjProfilerFrequencyHistogramControl ({
                data: {'histogram':'freq_histogram','integral':'freq_histogram_integral'},
                selectCallback: "function:vjObjFunc('onFreqHistClickOperation','" + this.objCls + "')",
                logGraph:true,
                formName:this.formName[1],
                isok:true}));

            this.addviewer('freqHistPopup', new vjFreqHistPopUp ({
                data:'freq_histogram_popup',
                formName:this.formName[4],
                parentVIS:freqhistpopVIS,
                isok:true}));

            this.addviewer('consensus_menu,consensus', new vjProfilerConsensusControl ({
                data: 'consensus',
                formName:this.formName[1],
                isok:true}));

            this.addviewer('snp_calls_panel,snp_calls', new vjProfilerSNPcallsControl ({
                data:'snp-calls',
                annotFileData: "eval:vjHO['"+this.typeName+"'].getDS('annot-files').name",
                formName:this.formName[1],
                isok:true}));

            this.addviewer('annot_panel,annot_orf_table,annot_snp_calls_graph,annot_orf_graph',new vjProfilerAnnotationControl ({
                data:{'snpcalls':'snp-calls-graph','orf':'orf'},
                annotFileData: "eval:vjHO['"+this.typeName+"'].getDS('annot-files').name",
                selectCallback:{'snpcalls' : "function:vjObjFunc('onZoomAnnotOperation','" + this.objCls + "')",'orf' : "function:vjObjFunc('onZoomAnnotBox','" + this.objCls + "')"},
                formName:this.formName[1],
                isok:true}));

            this.addviewer('zoom_profile_panel,zoom_profile_unaligned_terminus,zoom_profile_coverage,zoom_profile_disbalance,zoom_profile_entropy,zoom_profile_quality,zoom_profile_quality_acgt,zoom_profile_snp,zoom_profile_indels,zoom_profile_acgt_entropy', new vjProfilerZoomSNPProfileControl ({
                data:  {'profile':'zoom_profile', 'profileX': 'zoom_profileX'},
                width:0.95*gPgW,
                parentVIS:profzoomVIS,
                urlrefresh:"javascript:var res=vjObjFunc('zoomProfileOnPosition','" + this.objCls + "'); res.func.call(res.obj,'$(::objcls.parent.children[0].value)');",
                selectCallback: "function:vjObjFunc('onZoomProfileOperation','" + this.objCls + "')",
                formName:this.formName[2],
                isok:true}));

            this.viewersAlignment = new vjAlignmentControl ({
                data: 'zoom_alView',
                width:0.95*gPgW,
                parentVIS:profzoomVIS,
                formName:this.formName[2],
                isok:true});
            this.addviewer('zoom_alignment_panel,zoom_alignment', [this.viewersAlignment[0],this.viewersAlignment[1]]);

            this.viewersStack = new vjAlignmentStackControl ({
                data: {stack: 'zoom_alStack', basecallBias: 'zoom_alMutBias'},
                width:0.95*gPgW,
                parentVIS:profzoomVIS,
                formName:this.formName[2],
                isok:true});
            this.addviewer('zoom_stack_panel,zoom_stack_mutbias,zoom_stack', [this.viewersStack[0],this.viewersStack[1], this.viewersStack[2] ] );

            this.addviewer('zoom_annot_panel,zoom_annot_snp_calls_graph,zoom_annot_orf_graph', new vjProfilerZoomAnnotationControl ({
                data:{'snpcalls':'snp-calls-graph-zoom','orf':'orf-zoom'},
                selectCallback:{'snpcalls' : "function:vjObjFunc('onZoomAnnotOperation','" + this.objCls + "')",'orf' : "function:vjObjFunc('onZoomAnnotBox','" + this.objCls + "')"},
                formName:this.formName[3],
                parentVIS:annotzoomVIS,
                panelurl:"function:vjObjFunc('ZoomBack','" + this.objCls + "')",
                isok:true}));
        }
    };

    this.onSelectReferenceID = function ( viewer, node) {
        this.referenceID = node.id;
        var arrCol=new Array(vjGenomeColors[node.irow% gClrTable.length], shadeColor(vjGenomeColors[node.irow % gClrTable.length], 0.5));
        this.reload(this.referenceID,null,arrCol,node);
    };

    this.reload=function(subID,loadedID,colors, node)
    {
        if(subID){
            this.referenceID=subID;
        }
        if(loadedID){
            this.loadedID=loadedID;
        }
        var SNPprofileFile = "SNPprofile-"+this.referenceID+".csv";
        var SNPprofileThumbFile = "SNPthumb-"+this.referenceID+".csv";
        var SNPprofileRangeFile = "SNPprofile-"+this.referenceID+".csv";
        var ProfileInfoFile = "ProfileInfo-"+this.referenceID+".csv";
        var ProfileInfoThumbFile = "ThumbInfo-"+this.referenceID+".csv";
        if(this.algoProc && this.algoProc.recordViewer && this.algoProc.recordViewer.file ) {
            if(this.algoProc.recordViewer.file[SNPprofileThumbFile]) {
                this.isSNPThumb = true;
                SNPprofileFile = SNPprofileThumbFile;
            }
            if(this.algoProc.recordViewer.file[ProfileInfoThumbFile]) {
                this.isSNPThumb = true;
                ProfileInfoFile = ProfileInfoThumbFile;
            }
        }

        if(this.mode=='preview'){
            var viewer = vjDV.locate(this.dvinfo+ "._active.3");
            if(viewer)viewer.options.colors = colors;
            this.getDS('profile').url="qpbg_tblqryx4:
            vjDV.locate(this.dvinfo + "._active.").load(true);
        }
        else{
            var this_ = this;
            this.getDS('summary').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['summary'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('contigs').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['contigs'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('consensus').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['consensus'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);

            var tt = urlExchangeParameter(urlExchangeParameter(this.urlSet['annot-files'].active_url, "objs",this.loadedID),"subID",this.referenceID);
            this.getDS('annot-files').reload(tt,true);

            this.getDS('snp-calls').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['snp-calls'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('profile').reload("qpbg_tblqryx4:
            this.makeDS('profile_subrange_dl').reload("qpbg_tblqryx4:
            this.getDS('profileX').reload("qpbg_tblqryx4:
            this.getDS('noise').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['noise'].active_url, "ids",this.loadedID),"filename","Noise-"+this.referenceID+".csv"),false);
            this.getDS('noise_integral').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['noise_integral'].active_url, "ids",this.loadedID),"filename","NoiseIntegral-"+this.referenceID+".csv"),false);
            this.getDS('freq_histogram').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['freq_histogram'].active_url, "ids",this.loadedID),"filename","FreqProfile-"+this.referenceID+".csv"),false);
            this.getDS('freq_histogram_integral').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['freq_histogram_integral'].active_url, "ids",this.loadedID),"filename","FreqProfileIntegral-"+this.referenceID+".csv"),false);
            this.getDS('snp-calls-graph').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['snp-calls-graph'].active_url, "objs",this.loadedID),"subID",this.referenceID),false);

            var uu = urlExchangeParameter(urlExchangeParameter(this.urlSet['orf'].active_url, "objs",this.loadedID),"subID",this.referenceID);

            var srchFd = "seqID";
            if (node.Reference.indexOf("gi|")==0) srchFd = "gi";
            uu = urlExchangeParameter(uu,"searchField", srchFd);
            uu = urlExchangeParameter(uu,"search", node.Reference);
            this.getDS('orf').reload(uu,false);

            var subrangeDownloads = [
                { title: "SNP profile", blob: SNPprofileRangeFile, dsname: "profile_subrange_dl" },
                { title: "SNP profile (filled with zeros)", operation: "profWithZeros" },
                { title: "VCF of selected reference", operation: "profVCF" },
            ];

            this.viewers["profile_menu"].setDownloads(subrangeDownloads);
            this.viewers["profile_menu"].callerObject = {
                onClickMenuNode: function(container, node, menuViewer) {
                    var url = this_.makeProfileOperationURL(menuViewer, node.makeUrlParam, node.makeUrlParam.operation);
                    if (menuViewer.isDefault()) {
                        document.location = url;
                        return;
                    }
                    var ds = node.makeUrlParam.dsname ? this_.getDS(node.makeUrlParam.dsname) : null;
                    if (ds) {
                        url = ds.url;
                    }
                    url = menuViewer.urlExchangeParameter(url, "start");
                    url = menuViewer.urlExchangeParameter(url, "end");

                    if (ds) {
                        ds.reload(url, true, "download");
                    } else {
                        document.location = url;
                    }
                }
            };

            var t = "data,down,arch,operation,blob\n";
            for (var i=0; i<subrangeDownloads.length; i++) {
                var d = subrangeDownloads[i];
                if (d.title) t += d.title;
                t += ",download,ico-file,";
                if (d.operation) t += d.operation;
                t += ",";
                if (d.blob) t+= d.blob;
                t += "\n";
            }
            t +="VCF of all references,download,ico-file,profVCFAll,\n" +
                "SNP summary on selected reference,download,ico-file,profSNPcalls,\n" +
                "SNP summary of all references,download,ico-file,profAllSNPcalls,\n" +
                "VCF from samtools,download,ico-file,,SNP.vcf\n" +
                "gene_exp.diff from cuffdiff,download,rec,,gene_exp.diff\n" +
                "Noise profile,download,ico-file,,Noise-" + this.referenceID + ".csv\n" +
                "Noise cuttoffs,download,ico-file,,NoiseIntegral-" + this.referenceID + ".csv\n" +
                "Extended profile information,download,ico-file,,ProfileInfo-" + this.referenceID + ".csv\n" +
                "Frequency histogram,download,ico-file,,ico-FreqProfile-" + this.referenceID + ".csv\n" +
                "Frequency histogram integrals,download,ico-file,,FreqProfileIntegral-" + this.referenceID + ".csv\n" +
                "CSV Summary of all References,download,ico-file,profSummaryAll,\n" +
                "Consensus with gaps,download,dna,profConsensus,\n" +
                "Consensus with gaps replaced by reference,download,dna,profConsensus&gaps=fill,\n" +
                "Consensus where gaps are skipped,download,dna,profConsensus&gaps=skip,\n" +
                "Consensus split on gaps,download,dna,profConsensus&gaps=split,";
            this.getDS('downloads').reload("static:

            var viewer = this.viewers['profile_coverage'];
            if(viewer) viewer.options.colors = colors;

            var viewer = this.viewers['profile_unaligned_terminus'];
            if(viewer) viewer.options.colors = colors;

            vjDV.locate(this.dvname + "._active.").load(true);
            vjDV.locate(this.dvinfo + "._active.").load(true);
        }
    };


    this.ZoomBack = function()
    {
        if (this.urlStack.length > 1) {
            this.urlStack.pop();
            var snpUrl = this.urlStack[this.urlStack.length - 1].snp;
            this.getDS('snp-calls-graph-zoom').reload(snpUrl,true);
            var anotUrl = this.urlStack[this.urlStack.length - 1].anot;
            this.getDS('orf-zoom').reload(anotUrl,true);
            vjVIS.winop("vjVisual", this.dvzoomannot_visual, "open", true);
        }
    };


    this.makeProfileOperationURL = function (viewer, node, oper)
    {
        var url;
        if (node.blob)
            url = "?cmd=objFile&ids=" + this.loadedID + "&filename=" + node.blob;
        else {
            url = "?cmd=" + node.operation + "&objs=" + this.loadedID + "&idSub=" + this.referenceID;
            if (node.operation == 'profNoise')
                url += "&noiseProfileMax=1";
            else if(node.operation=="profSNPcalls"){
                url=this.getDS('snp-calls').url.substring(7);
                url = urlExchangeParameter(url, "cnt", '-1');
                url = urlExchangeParameter(url, "start", '0');
            }
            else if(node.operation=="profAllSNPcalls"){
                url=this.getDS('snp-calls').url.substring(7);
                url = urlExchangeParameter(url, "idSub", '0');
                url = urlExchangeParameter(url, "cnt", '-1');
                url = urlExchangeParameter(url, "start", '0');
            }
            else if(node.operation=="profVCFAll"){
                url=urlExchangeParameter(url, "cmd", 'profVCF');
                url = urlExchangeParameter(url, "idSub", '0');
            }
            else if(node.operation == "profSummaryAll") {
                url=urlExchangeParameter(url, "cmd", 'profSummary');
                url = urlExchangeParameter(url, "idSub", '0');
            }
            else if (node.operation.indexOf("profConsensus")>=0 ){
                url = urlExchangeParameter(url,"consensus_thrshld",docLocValue("consensus_thrshld","-",this.getDS('consensus').url) );
            }
        }
        return url;
    };


    this.onPerformProfileOperation = function (viewer, node, ir, ic)
    {
        var url = this.makeProfileOperationURL(viewer, node, node.operation);
        if(viewer.tblArr.hdr[ic].name=="down"){
            url = urlExchangeParameter(url, "down", '1');
            if( docLocValue("backend",0,url) ){
                var saveAs = "o"+this.loadedID+"-Summary-All.csv";
                this.dsQPBG.reload("qpbg_http:
            }
            else
                document.location = url;
        }
        else if(viewer.tblArr.hdr[ic].name=="arch") {
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

    this.onZoomProfileOperation = function (viewer, node, ir, ic)
    {
        if (!node.Position) return;
        else
            this.zoomProfileOnPosition(node.Position);
    };

    this.zoomProfileOnPosition=function(Highlighted)
    {
        Highlighted = parseInt(Highlighted);
        var panel = vjDV.locate(this.dvzoom + "." + "profile" + ".0");
        panel.graphResolutionZoom = this.graphResolutionZoom;
        var url = "qpbg_tblqryx4:
        url = panel.urlExchangeParameter(url, "alHigh", Highlighted);
        this.getDS('zoom_profile').reload(url, true);

        var url = "qpbg_tblqryx4:
        url = panel.urlExchangeParameter(url, "alHigh", Highlighted);
        this.getDS('zoom_profileX').reload(url, true);

        this.getDS('zoom_alView').reload("http:
        this.getDS('zoom_alStack').reload("http:
        this.getDS('zoom_alMutBias').reload("http:

        var viewer = vjDV.locate(this.dvzoom + "."+"profile"+".1");
        viewer.options.colors = new Array(valgoGenomeColors[this.referenceID % gClrTable.length], shadeColor(valgoGenomeColors[this.referenceID % gClrTable.length], 0.5));
        var viewer = vjDV.locate(this.dvzoom + "."+"profile"+".2");
        viewer.options.colors = new Array(valgoGenomeColors[this.referenceID % gClrTable.length], shadeColor(valgoGenomeColors[this.referenceID % gClrTable.length], 0.5));

        vjVIS.winop("vjVisual", this.dvzoom_visual, "open", true);
    };

    this.onZoomAnnotOperation = function (node, evt)
    {
        var url;
        var Highlighted = parseInt(node.Position);
        var quarterLengthRange = Math.round((parseInt(node.max) - parseInt(node.min))/4);
        var startPosition = Highlighted - quarterLengthRange; if (startPosition < 0 ) startPosition =0;
        var endPosition = Highlighted + quarterLengthRange;
        var resolution = endPosition - startPosition; if (resolution>200) resolution = 200;
        this.windowZoomStack.push({start:startPosition,end:endPosition});
        url =  "http:

        this.getDS('snp-calls-graph-zoom').reload(url, true);

        var url1 = this.getDS('orf').dataurl;
        url1 = urlExchangeParameter(url1,"start", startPosition);
        url1 = urlExchangeParameter(url1,"end", endPosition);
        this.getDS('orf-zoom').reload(url1, true);

        this.urlStack.push({snp:url,anot:url1});
        vjVIS.winop("vjVisual", this.dvzoomannot_visual, "open", true);
    };


    this.onFreqHistClickOperation = function (viewer, node, ir, ic)
    {

        parseHistProfileTableFromCell = function(text, column, hdr, criteria)
        {

            var tblArr = new vjTable(text, 0, vjTable_propCSV);
            var count = new Object();
            var XYvalues = hdr+"\n";

            for( var il=0; il<tblArr.rows.length; ++il ){
               var cols=tblArr.rows[il].cols[column].split(",");
               if (!eval(criteria)) continue;
               var letter = tblArr.rows[il].cols[1];

               for( var ic=0; ic<cols.length; ic+=2 ){
                   if (!count[cols[ic]]) count[cols[ic]] = new Object();
                   count[cols[ic]][letter] = cols[ic+1];
               }

           }

          for (ii in count){
               var cntA=(count[ii]['A'])?(count[ii]['A']):0;
               var cntT=(count[ii]['T'])?(count[ii]['T']):0;
               var cntG=(count[ii]['G'])?(count[ii]['G']):0;
               var cntC=(count[ii]['C'])?(count[ii]['C']):0;
               XYvalues+=ii+","+cntA+","+cntT+","+cntG+","+cntC +"\n";
           }
           return XYvalues;
        };

        var histo_tqs = [
            {
                op: "filter",
                arg: {
                    col: 0,
                    method: "equals",
                    value: node.Frequency
                }
            }
        ];
        var histo_tqs_quoted = vjDS.escapeQueryLanguage(JSON.stringify(histo_tqs));
        var url;
        url = "qpbg_tblqryx4:


        var dd = this.getDS("freq_histogram_popup");
        dd.parser = function (ds,text) { text=parseHistProfileTableFromCell(text, 2, "Coverage,CountA,CountT,CountG,CountC", "true");  return text;};
        this.getDS("freq_histogram_popup").reload(url,true);
        gObject("DV_dvhistpopupViewer_"+"Coverage"+"_tabname").innerHTML="Frequency " + node.Frequency+" coverage histogram";
        vjVIS.winop("vjVisual", this.dvhistpopup_visual, "open", true);
    };

    this.onZoomAnnotBox = function (node,evt)
    {
        this.windowZoomStack.push({start:node.min,end:node.max});
        var quarterLengthRange = Math.round((parseInt(node.max) - parseInt(node.min))/4);
        var position = Math.round((parseInt(node.rangeEnd)-parseInt(node.rangeStart))/2) + parseInt(node.rangeStart);
        var startPosition = position - quarterLengthRange; if (startPosition < 0 ) startPosition =0;
        var endPosition = position + quarterLengthRange;
        url = this.getDS('orf').dataurl;
        url = urlExchangeParameter(url,"start", startPosition);
        url = urlExchangeParameter(url,"end", endPosition);

        this.getDS('orf-zoom').reload(url, true);
        var resolution = endPosition - startPosition; if (resolution>200) resolution = 200;
        var url1 =  "http:
        this.urlStack.push({snp:url1,anot:url});
        this.getDS('snp-calls-graph-zoom').reload(url1, true);
        vjVIS.winop("vjVisual", this.dvzoomannot_visual, "open", true);
    };

    this.onLoadedHitList = function ()
    {
        var viewer = vjDV.locate(this.dvinfo+ "._active.0");

        if(this.autoClickedReference!==false || this.autoClickedReference!==undefined || this.autoClickedReference!==null)
            viewer.mimicClickCell(this.autoClickedReference,0);
    };

    this.link2Proc = function(proc) {
        this.algoProc = proc;
        var checkedIDS = this.viewer.accumulate("node.checked", "node.id");
        return isok(checkedIDS) ? 1 : 0;
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};



