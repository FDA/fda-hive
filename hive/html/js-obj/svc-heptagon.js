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

vjHO.register('svc-heptagon').Constructor=function ()
{

    if(this.objCls)return;         //stupid chrome loads from both cached file and the one coming from server. 
    this.dsQPBG = vjDS.add('preparing to download','ds'+this.objCls+'_QPBG','static://');
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
    this.dsQPBG_digest = vjDS.add('preparing to archive','ds'+this.objCls+'_QPBG_digest','static://',{func :this.onArchiveSubmit,obj : this});

    // two public functions which must be supported

    //
    // Loads the viewers and then constructs the viewers in the interface
    //
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

    //
    // Loads the viewers and then constructs the viewers in the interface
    // for the preview view
    //
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

    //
    // Loads the viewers and then constructs the viewers in the interface
    // for the mobile view
    //
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
    if(document.all) { // IE Cannot show more than some little number of HTML5 s
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }
     

    this.windowZoomStack = new Array({windwStart:0,windwEnd:0});
    this.urlStack = new Array();

    // column 5 is Count Total; use it as main col (so Position will be output for Count Total's min/max values in each row group)
    // note that minmaxCols and minmaxMainCol and abscissaCol are indices of *output* columns
    this.seqalign_SNPprofile_cmd = "cols=1-3,8-19,22-27&hdr=1&minmaxCols=3-15&minmaxMainCol=5&abscissaCol=0&resolution="+this.graphResolution;

    this.seqalign_SNPprofile_cmd_zoom = "cols=1-3,8-19,22-27&hdr=1";

    this.seqalign_SNPprofile_cmdX = "cols=1-13&hdr=1&minmaxCols=1-12&abscissaCol=0" +
        "&resolution="+this.graphResolution;
        //"out=0,1,2,3,4,5,6,7,8,9,10,11,12&hdr=1&reuse=1&jumpbase=0" +
        //"&minmax=1,2,3,4,5,6,7,8,9,10,11,12&resolution="+this.graphResolution;

    this.seqalign_SNPprofile_cmdX_zoom = "cols=1-13&hdr=1";

    this.seqalign_SNPprofile_cmd_subrange_dl = "hdr=1";
    
    this.SNPprofile_cmd_concatenated = "cols=1-13&hdr=1";

    this.autoClickedReference=1;

    this.objCls="obj-svc-profiler"+Math.random();
    vjObj.register(this.objCls,this);
    this.urlSet ={
            'hitlist': {
                active_url: "http://?cmd=alCount&objLink=parent_proc_ids&start=0&cnt=50",
                objs:'objs',
                title: "Retrieving list of hits"
            },
            'zoom_alView':{
                loadInactive:true,
                active_url:"http://?cmd=alView&objLink=parent_proc_ids&start=0&cnt=50&info=1",
                header:"Element #,#,Direction,Start,Alignment,End,Sequence,Repeats,Position,PosHigh,Motif Start,Motif End",
                title: "Prepering alignments"
            },
            'zoom_alStack' :{
                loadInactive:true,
                active_url: "http://?cmd=alStack&objLink=parent_proc_ids&start=0&cnt=50&info=1",
                title: "Visualizing alignments in stack"
            },
            'zoom_alMutBias' :{
                loadInactive:true,
                active_url: "http://?cmd=alMutBias&objLink=parent_proc_ids&start=0&cnt=50&info=1",
                header : "Position,Count-A,Count-C,Count-G,Count-T",
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
                title: "infrastructure: Creating profilerX"    //it won't be displayed because is infrustructure
            },
            'zoom_orf' : {
                loadInactive:true,
                active_url:"http://?cmd=anotDefinition&dataName=CDS&whatToOutPut=locus|rangeStart|rangeEnd|id",
                title: "Retrieving open reading frames",
            },
            'zoom_snp-calls-graph' :{
                loadInactive:true,
                active_url: "http://?cmd=profSNPcalls&start=0&isORF=1&resolution=300",
                title: "Retrieving snps calls"
            },
            'summary' : {
                loadInactive:true,
                active_url:"http://?cmd=profSummary",
                title: "Composing summary"
            },
            'full_summary' : {
                loadInactive:true,
                active_url:"http://?cmd=profSummary",
                title: "Composing full summary"
            },
            'contigs' : {
                loadInactive:true,
                active_url:"http://?cmd=profContig&start=0&cnt=20",
                title: "Constructing contigs"
            },
            'annot-files' : {
                //active_url:"http://_.csv//dataCmd=cmd%3DanotFiles%26type%3Du-ionAnnot%26cnt%3D-1",
                active_url:'http://?cmd=objList&type=u-ionAnnot&mode=csv&prop=name,size,created&start=0&cnt=10',
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
                title: "infrastructure: Creating profilerX"    //it won't be displayed because is infrustructure
            },
            'profile_subrange_dl' : {
                loadInactive: false,
                active_url:this.seqalign_SNPprofile_cmd_subrange_dl,
                title: "Building SNP profile for position range"
            },
            'noise' : {
                loadInactive:true,
                active_url:this.SNPprofile_cmd_concatenated,
                title: "Creating noise profiles"
            },
            'noise_integral' : {
                loadInactive:true,
                active_url:"http://?cmd=profNoiseIntegral",
                title: "Infrastructre: Computing noise integrals"
            },
            'freq_histogram' : {
                loadInactive:true,
                active_url:this.SNPprofile_cmd_concatenated,
                title: "Infrastructre: Computing frequency histogram"
            },
            'freq_histogram_popup' :
            {
                loadInactive:false,
                active_url:this.SNPprofile_cmd_concatenated,
                title: "Infrastructre: Computing coverage frequency histogram"
            },
            'freq_histogram_integral' : {
                loadInactive:true,
                active_url:"http://?cmd=profFreqIntegral",
                title: "Infrastructre: Computing frequency histogram integral"
            },
            'consensus' : {
                loadInactive:true,
                active_url:"http://?cmd=profConsensus&cnt=120000",
                title: "Building consensus"
            },
            'snp-calls' : {
                loadInactive:true,
                active_url:"http://?cmd=profSNPcalls&start=0&cnt=20",
                title: "Calling snps"
            },
            'snp-calls-graph' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http://?cmd=profSNPcalls&sub_start=0&isORF=1&resolution=300",
                title: "Ploting snp calls"
            },
            'aa-calls' : {
                loadInactive:true,
                active_url:"http://?cmd=objFile&filename=AAprofile.csv",
                title: "Viewing AA calls"
            },
            'snp-calls-graph-zoom' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http://?cmd=profSNPcalls&sub_start=0&isORF=1&resolution=300",
                title: "Ploting snp calls (zoom)"
            },
            'orf-zoom' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http://?cmd=anotSearch&isProfiler=1&whatToPrint=seqID|gi|rangeStart|rangeEnd|locus_tag|protein_id|product",
                title: "Displaying open reading frames (zoom)"
            },
            'orf' : {
                loadInactive:true,
                isSeries:true,
                //active_url:"http://?cmd=anotDefinition&dataName=CDS&whatToOutPut=locus|rangeStart|rangeEnd|id",
                active_url:"http://?cmd=anotSearch&isProfiler=1&whatToPrint=seqID|gi|rangeStart|rangeEnd|locus_tag|protein_id|product",
                title: "Displaying open reading frames"
            }, // this.defaultFormat="posID|seqID|annotRange"; active_url:"http://?cmd=anotDumper&annot_format="+this.defaultFormat,
            'downloads' : {
                loadInactive:true,
                active_url:"static://",
                inactive_url:"static://",
                title: "infrastructure: creating list of downloadables"
            },
            'help' : {
                active_url:"http://help/hlp.obj.svc-profiler.info.html",
                inactive_url:"http://help/hlp.obj.svc-profiler.info.html",
                doNotChangeMyUrl : true,
                title: "infrastructure: building help page"
            },
            'annotationData' : {
                loadInactive:false,//true,
                isSeries:true,
                active_url:"static://",
                //active_url:"http://?cmd=anotBrowser&refSeqID=chrLam&refObjID=3030711&resolution=100&density=17&srch=3030768%5Bsource%5D&start=0&cnt=20",
                //active_url: "http://?cmd=ionGenBankAnnotPosMap&objs=3080380&ionObjs=3080478&mySubID=1&cnt=-1",
//                active_url:"http://?cmd=anotSearch&isProfiler=1&whatToPrint=seqID|gi|rangeStart|rangeEnd|locus_tag|protein_id|product",
//                active_url:"http://?cmd=anotBrowser&srch=3017874%5BAccession|Position(N)|Ref(N)%5D%2C3017932%5Bfeature|gene_name%5D&cnt=50&start=0&end=4000",
                //header:"Element #,#,Direction,Start,Alignment,End,Sequence,Repeats,Position,PosHigh,Motif Start,Motif End",
                title: "Prepering annotation viewer"
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

    //
    // Construct the viewers (draw in interface)
    //
    
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
            //this.hit_list_location=2;
            dv.render(true);
            dv.load('rerender');
        }
        else if(this.mode=='mobile'){
            var dv=this.current_dvORtab.obj;
            var origNumofTabs=dv.tabs.length;
            this.dvname=dv.name;
            this.dvinfo=dv.name;
            dv.addTab("profile","table",[this.viewers['hitlist'],this.viewers['profile_coverage']]).columns=2;
            //this.hit_list_location=2;
            dv.render(true);
            dv.load('rerender');
        }
        else{
            // Set up name for left results viewer 
            this.dvname=this.current_dvORtab[0].name;
            // Set up name for right results viewer
            this.dvinfo=this.current_dvORtab[1].name;

            // Add tabs to left viewer
            this.current_dvORtab[0].addTab("summary","table",[this.viewers['summary']]);
            this.current_dvORtab[0].addTab("downloads","table",[this.viewers['downloads']]);
            this.current_dvORtab[0].addTab("gaps and contigs","dna",[this.viewers['contigs_panel'],this.viewers['contigs']]);
            this.current_dvORtab[0].addTab("annot-files","graph",[this.viewers['annot_files_panel'],this.viewers['annot_files']]);

            // Add tabs to the right viewer
            this.current_dvORtab[1].addTab("profile","graph",[this.viewers['profile_menu'],this.viewers['profile_coverage'],this.viewers['profile_unaligned_terminus'],this.viewers['profile_disbalance'],this.viewers['profile_entropy'], this.viewers['profile_quality'],this.viewers['profile_snp'],this.viewers['profile_indels'],this.viewers['profile_annotViewer']]).viewtoggles = 1;
           // this.current_dvORtab[1].addTab("profile","graph",[this.viewers['profile_menu'],this.viewers['profile_coverage'],this.viewers['profile_unaligned_terminus'],this.viewers['profile_disbalance'],this.viewers['profile_entropy'], this.viewers['profile_quality'],this.viewers['profile_snp'],this.viewers['profile_indels']]).viewtoggles = 1;
            this.current_dvORtab[1].addTab("help","help",[this.viewers['help']]);
            this.current_dvORtab[1].addTab("sequencing noise","graph",[this.viewers['noise'],this.viewers['noise_integral']]);
            this.current_dvORtab[1].addTab("frequency histogram","graph",[this.viewers['freq_histogram'],this.viewers['freq_histogram_integral']]);
            this.current_dvORtab[1].addTab("consensus","dna",[this.viewers['consensus_menu'],this.viewers['consensus']]);
            this.current_dvORtab[1].addTab("SNP calls","dna",[this.viewers['snp_calls_panel'],this.viewers['snp_calls']]);
            this.current_dvORtab[1].addTab("AA calls","dna",[this.viewers['aa_calls']]);
            //this.current_dvORtab[1].addTab("ORF visualization","graph",[this.viewers['annot_panel'],this.viewers['annot_orf_table'],this.viewers['annot_snp_calls_graph'],this.viewers['annot_orf_graph']]);

            // Profiler zoom viewer (Add tabs)
            this.current_dvORtab[2].addTab("profile","graph",[this.viewers['zoom_profile_panel'], this.viewers['zoom_profile_coverage'], this.viewers['zoom_profile_unaligned_terminus'],this.viewers['zoom_profile_disbalance'], this.viewers['zoom_profile_entropy'], this.viewers['zoom_profile_quality'],this.viewers['zoom_profile_quality_acgt'], this.viewers['zoom_profile_snp'], this.viewers['zoom_profile_indels'],this.viewers['zoom_profile_acgt_entropy']]).viewtoggles = 1;
            this.current_dvORtab[2].addTab("alignments","dna",[this.viewers['zoom_alignment_panel'],this.viewers['zoom_alignment']]);
            this.current_dvORtab[2].addTab("stack","dna",[this.viewers['zoom_stack_panel'],this.viewers['zoom_stack_mutbias'],this.viewers['zoom_stack']]);

            // Annotation zoom viewer (add tabs)
            this.current_dvORtab[3].addTab("ORF visualization","graph",[this.viewers['zoom_annot_panel'],this.viewers['zoom_annot_snp_calls_graph'],this.viewers['zoom_annot_orf_graph']]);

            // Histogram popup viewer (add tabs)
            this.current_dvORtab[4].addTab("coverage","graph",[this.viewers['freqHistPopup']]);

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
//            this.viewers['hitlist'].rowspan=2;

            this.addviewer('profile_coverage', new vjProfilerSNPProfileCoverage({
                data: 'profile',//{'profile':'profile', 'profileX': 'profileX'},
                width:'100%',
                height:240,
                formName:this.formName,
                isok:true}));

            this.addviewer('null', new vjHTMLView ({data:"dsVoid",width:"100%"}));

            //this.addviewer('profile_acgt_qualities', this.viewersSNPprogile[7]);
            //this.addviewer('profile_histograms', this.viewersSNPprogile[8]);

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
            vjDS.add("loading Annot Types","dsAnnotTypes","static://");
            
            this.addviewer('summary',  new vjProfilerSummaryView ({
                data: 'summary',
                width:'100%',
                maxWidth: "100%",
                formName:this.formName[0],
                isok:true}));

            this.addviewer('downloads', new vjProfilerDownloadsView ({
                data: 'downloads',
                formName:this.formName[0],
                selectCallback: "function:vjObjFunc('onPerformProfileOperation','" + this.objCls + "')",
                isok:true}));

            this.addviewer('contigs_panel,contigs', new vjProfilerContigControl ({
                data: 'contigs',
                formName:this.formName[0],
                isok:true}));

            this.addviewer('annot_files_panel,annot_files',new vjProfilerAnnotationListControlView ({
                data: 'annot-files',
                formName:this.formName[0],
                //width:'90%',
                //callbackRendered:"function:vjObjFunc('onRenderIonAnnotList','" + this.objCls + "')",
                callbackForPanel:"function:vjObjFunc('onRenderIonAnnotList','" + this.objCls + "')",
                selectCallbackForTable:"function:vjObjFunc('onSelectIonAnnotList','" + this.objCls + "')",
                isok:true}));

            // Adds viewers (graphs, etc.) for the "profile" tab to render.
            this.addviewer('profile_menu,profile_coverage,profile_unaligned_terminus,profile_disbalance,profile_entropy,profile_quality,profile_snp,profile_indels,profile_annotViewer', new vjProfilerSNPProfileControl ({
            //this.addviewer('profile_menu,profile_coverage,profile_unaligned_terminus,profile_disbalance,profile_entropy,profile_quality,profile_snp,profile_indels', new vjProfilerSNPProfileControl ({
                data: {'profile':'profile', 'profileX': 'profileX', 'annotationData' : 'annotationData'},
                width:800,
                selectCallback: "function:vjObjFunc('onZoomProfileOperation','" + this.objCls + "')",
                formName:this.formName[1],
                isok:true}));

            // Adds viewers (graphs, etc.) for the "help" tab to render.
            this.addviewer('help', new vjHelpView ({
                data: 'help',
                formName:this.formName[1],
                isok:true}));

         // Adds viewers (graphs, etc.) for the "sequencing noise" tab to render.
            this.addviewer('noise,noise_integral', new vjProfilerNoiseControl ({
                data: {'noise':'noise','integral':'noise_integral'},
                formName:this.formName[1],
                isok:true}));

         // Adds viewers (graphs, etc.) for the "frequency histogram" tab to render.
            this.addviewer('freq_histogram,freq_histogram_integral', new vjProfilerFrequencyHistogramControl ({
                data: {'histogram':'freq_histogram','integral':'freq_histogram_integral'},
                selectCallback: "function:vjObjFunc('onFreqHistClickOperation','" + this.objCls + "')",
                logGraph:true,
                formName:this.formName[1],
                isok:true}));

            this.addviewer('freqHistPopup', new vjFreqHistPopUp ({
                data:'freq_histogram_popup',
               // selectCallback:{'snpcalls' : "function:vjObjFunc('onZoomAnnotOperation','" + this.objCls + "')",'orf' : "function:vjObjFunc('onZoomAnnotBox','" + this.objCls + "')"},
                formName:this.formName[4],
                parentVIS:freqhistpopVIS,
                isok:true}));

         // Adds viewers (graphs, etc.) for the "consensus" tab to render.
            this.addviewer('consensus_menu,consensus', new vjProfilerConsensusControl ({
                data: 'consensus',
                formName:this.formName[1],
                isok:true}));

         // Adds viewers (graphs, etc.) for the "SNP calls" tab to render.
            this.addviewer('snp_calls_panel,snp_calls', new vjProfilerSNPcallsControl ({
                data:'snp-calls',
                annotFileData: "eval:vjObj['"+this.objCls+"'].getDS('annot-files').name",
                formName:this.formName[1],
                isok:true}));
            this.addviewer('aa_calls', new vjTableView ({
                data:'aa-calls',
                formName:this.formName[1],
                isok:true}));
            
         // Adds viewers (graphs, etc.) for the "ORF Visualization" tab to render.
            this.addviewer('annot_panel,annot_orf_table,annot_snp_calls_graph,annot_orf_graph',new vjProfilerAnnotationControl ({
                data:{'snpcalls':'snp-calls-graph','orf':'orf'},
                annotFileData: "eval:vjObj['"+this.objCls+"'].getDS('annot-files').name",
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

    this.resolveAvailableInfo = function( refID ) {
        if (refID === undefined ) {
            refID = "";
        }
        else{
            refID = "-"+refID;
        }
        if(this.algoProc && this.algoProc.recordViewer && this.algoProc.recordViewer.file ) {
            if( this.algoProc.recordViewer.file["ProfileInfo.csv"] ) {
                this.isProfileInfo = true;
            }
            if( this.algoProc.recordViewer.file["SNPthumb.csv"] ) {
                this.isSNPThumb = true;
                if( this.algoProc.recordViewer.file["ThumbInfo.csv"] ) {
                    this.isThumbInfo = true;
                }
            }
            if( this.algoProc.recordViewer.file["HistProfile.csv"] ) {
                this.isHistProfile = true;
            }
        }
    };

    this.reload=function(subID,loadedID,colors, node)
    {
        if(subID){
            this.referenceID=subID;
        }
        if(loadedID){
            this.loadedID=loadedID;
        }

        this.resolveAvailableInfo(this.referenceID);

        var SNPprofileFile = "SNPprofile";

        var ProfileInfoFile = "ProfileInfo";

        this.tqs_profile = [
                        {
                            op: "load-SNPprofile",
                            arg: {
                                obj: this.loadedID,
                                sub: this.referenceID,
                                thumb: this.isSNPThumb,
                                autoAddMissingRows: true // ignored by backend if thumbnail file exists and has data for this.referenceID
                            }
                        }
                    ];
        this.tqs_profile_quoted = vjDS.escapeQueryLanguage(JSON.stringify(this.tqs_profile));

        this.tqs_load_zoom_profile = [
                        {
                            op: "load-SNPprofile",
                            arg: {
                                obj: this.loadedID,
                                sub: this.referenceID
                            }
                        }
                    ];
        this.tqs_load_zoom_profile_quoted = vjDS.escapeQueryLanguage(JSON.stringify(this.tqs_load_zoom_profile));

        this.tqs_slice = [
                         {
                            op: "slice",
                            arg: {
                                start: {
                                    col: 0,
                                    value: parseInt(this.referenceID),
                                    method: "firstEquals"
                                },
                                end: {
                                    col: 0,
                                    value: parseInt(this.referenceID),
                                    method: "lastEquals"
                                }
                            }
                        }
                    ];
        this.tqs_slice_quoted = vjDS.escapeQueryLanguage(JSON.stringify(this.tqs_slice));

        this.tqs_zoom = [
                {
                    op: "predefine",
                    arg: {
                        // save the referecence ID in a local variable in tqs because we will
                        // use this value in formulas composed by vjProfilerZoomSNPProfilePanelView
                        name: "saved_referenceID",
                        value: parseInt(this.referenceID)
                    }
                },
                {
                    op: "addMissingRows",
                    arg: {
                        abscissa: {
                            col: { name: "Position" },
                            dense: true,
                            maxGap: 1,
                            minValue: 1,
                            maxValue: 1
                        },
                        add: [
                            { col: { name: "Reference" }, value: parseInt(this.referenceID) },
                            { col: { name: "Letter" }, value: { formula: "input_obj.refSeqLetter(saved_referenceID,cur_abscissa_val)" } },
                            { col: { name: "Consensus" }, value: { formula: "input_obj.refSeqLetter(saved_referenceID,cur_abscissa_val)" } },
                            { col: "*", value: 0 }
                        ]
                    },
                    start_end_slice: true
                }
        ];
        this.tqs_zoom_quoted = vjDS.escapeQueryLanguage(JSON.stringify(this.tqs_zoom));

        this.tqs_zoom_profile = this.tqs_load_zoom_profile.concat(this.tqs_zoom);
        this.tqs_zoom_profile_quoted = vjDS.escapeQueryLanguage(JSON.stringify(this.tqs_zoom_profile));

        this.tqs_zoom_profileX = this.tqs_slice.concat(this.tqs_zoom);
        this.tqs_zoom_profileX_quoted = vjDS.escapeQueryLanguage(JSON.stringify(this.tqs_zoom_profileX));
        
        if(this.mode=='preview'){
            var viewer = vjDV.locate(this.dvinfo+ "._active.3");
            if(viewer)viewer.options.colors = colors;
            this.getDS('profile').reload("qpbg_tblqryx4://"+this.seqalign_SNPprofile_cmd+"&tqs="+this.tqs_profile_quoted,false);
            //alerJ('ffff',this.getdsName('profile') );
            vjDV.locate(this.dvinfo + "._active.").load(true);
        }
        else{
            var this_ = this; // for closure
            var urlSum=urlExchangeParameter(urlExchangeParameter(this.urlSet['summary'].active_url, "objs",this.loadedID),"idSub",this.referenceID);
            urlSum=urlExchangeParameter(urlSum, "gapThreshold",parseInt(algoProcess.getValue("minCover"))-1);
            this.getDS('summary').reload(urlSum,false);
            this.getDS('contigs').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['contigs'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('consensus').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['consensus'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);

            //var tt = urlExchangeParameter(urlExchangeParameter(this.urlSet['annot-files'].active_url, "objs",this.loadedID),"subID",this.referenceID);
           // tt = urlExchangeParameter(tt,"idToSearch",node.Reference);
           // tt = urlExchangeParameter(tt,"idTypeToSearch","seqID");
            this.getDS('annot-files').reload(this.urlSet['annot-files'].active_url,true);

            this.getDS('snp-calls').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['snp-calls'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('aa-calls').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['aa-calls'].active_url, "ids",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('profile').reload("qpbg_tblqryx4://"+this.seqalign_SNPprofile_cmd+"&tqs="+this.tqs_profile_quoted,false);
            // profile_subrange_dl doesn't have any viewers attached, so make it directly
            this.makeDS('profile_subrange_dl').reload("qpbg_tblqryx4://"+this.urlSet['profile_subrange_dl'].active_url+"&tqs="+this.tqs_load_zoom_profile_quoted,false);
            if( this.isProfileInfo) {
                var url = "qpbg_tblqryx4://ProfileInfo.csv//objs=" + this.loadedID + "&" + this.seqalign_SNPprofile_cmdX + "&tqs=" + this.tqs_slice_quoted;
                this.getDS('profileX').reload(url, true);
                
//                this.getDS('profileX').reload("qpbg_tblqryx4://"+ProfileInfoFile+" + ".csv//"+urlExchangeParameter(this.urlSet['profileX'].active_url, "objs",this.loadedID),false);
            }
            this.getDS('noise').reload("qpbg_tblqryx4://Noise.csv//"+urlExchangeParameter(this.urlSet['noise'].active_url, "objs",this.loadedID)+"&tqs="+this.tqs_slice_quoted,false);
            this.getDS('noise_integral').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['noise_integral'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('freq_histogram').reload("qpbg_tblqryx4://FreqProfile.csv//"+urlExchangeParameter(this.urlSet['freq_histogram'].active_url, "objs",this.loadedID)+"&tqs="+this.tqs_slice_quoted,false);
            this.getDS('freq_histogram_integral').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['freq_histogram_integral'].active_url, "objs",this.loadedID),"idSub",this.referenceID),false);
            this.getDS('snp-calls-graph').reload(urlExchangeParameter(urlExchangeParameter(this.urlSet['snp-calls-graph'].active_url, "objs",this.loadedID),"subID",this.referenceID),false);
            
            this.getDS("profile").parser = function (ds,txt){
                var anotDs = ds.name.replace(/_profile$/g,"_annotationData");
                var myUrl = vjDS[anotDs].url;
                var panel_tree = dvinfo.tabs[0].viewers[0].tree;
                if (panel_tree!=undefined && txt && txt.length) {
                    var startVal = panel_tree.findByName("start").value;
                    var endVal = panel_tree.findByName("end").value;
                    myUrl = urlExchangeParameter(myUrl, "pos_start", startVal);
                    myUrl = urlExchangeParameter(myUrl, "pos_end", endVal);
                    
                    dvinfo.tabs[0].viewers[8].min = startVal; // find different way
                    dvinfo.tabs[0].viewers[8].max = endVal;
                    
                    vjDS[anotDs].reload(myUrl,true);
                }
                return txt;
            };

            // ##########
            // Add annotation viewer
        
            // 
            this.getDS("annotationData").reload(this.urlSet['annotationData'].active_url,true);

            
            // ##################### Annotation viewer
            
            var uu = urlExchangeParameter(urlExchangeParameter(this.urlSet['orf'].active_url, "objs",this.loadedID),"subID",this.referenceID);

            //var qryRange = "0-" + node.Length;
            var srchFd = "seqID";
            if (node.Reference.indexOf("gi|")==0) srchFd = "gi";
            uu = urlExchangeParameter(uu,"searchField", srchFd); //
            uu = urlExchangeParameter(uu,"search", node.Reference);
            this.getDS('orf').reload(uu,false);

            var subrangeDownloads = [
                { title: "SNP profile", blob: "SNPprofile.csv", dsname: "profile_subrange_dl" },
//                { title: "SNP profile (filled with zeros)", operation: "profWithZeros" },
                { title: "SNP profile in VCF format", operation: "profVCF" },
                { title: "Annotation Data", operation:"ionGenBankAnnotPosMap" }
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
                    url = menuViewer.urlExchangeParameter(url, "start"); // fills current value
                    url = menuViewer.urlExchangeParameter(url, "end"); // fills current value

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
            // Removed below since samtools no working?
            //"VCF from samtools,download,ico-file,,SNP.vcf\n" +
            t +="SNP profile of all references in VCF format,download,ico-file,profVCFAll\n" +
                "SNP summary on selected reference,download,ico-file,profSNPcalls\n" +
                "SNP summary of all references,download,ico-file,profAllSNPcalls\n" +
                "AA-mutation summary of all references,download,ico-file,,AAprofile.csv\n" +
                "Noise profile,download,ico-file,,Noise.csv\n" +
                "Noise cuttoffs,download,ico-file,profNoiseIntegral\n" +
                "Extended profile information,download,ico-file,,ProfileInfo.csv\n" +
                "Frequency histogram,download,ico-file,,FreqProfile.csv\n" +
                "Frequency histogram integrals,download,ico-file,profFreqIntegral\n" +
                "Gaps and Contigs of all References,download,ico-file,profContigAll\n" +
                "CSV Summary of all References,download,ico-file,profSummaryAll\n" +
                "Consensus with gaps,download,dna,profConsensus,\n" +
                "Consensus with gaps replaced by reference,download,dna,profConsensus&gaps=fill,\n" +
                "Consensus where gaps are skipped,download,dna,profConsensus&gaps=skip,\n" +
                "Consensus split on gaps,download,dna,profConsensus&gaps=split,";
            this.getDS('downloads').reload("static://" + t, false);

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


    this.makeProfileOperationURL = function (viewer, node, oper,isArch)
    {
        var url;
        if (node.blob == "SNPprofile.csv") {
            if(isArch) {
                url = "qpbg_tblqryx4://::arch-req.txt";
            } else {
                url = "qpbg_tblqryx4://:::-o"+this.loadedID+"-SNPprofile.csv";
            }
            url +="//tqs="+this.tqs_load_zoom_profile_quoted;
        } else if (node.blob){
            url = "qpbg_tblqryx4://"+node.blob;
            if(isArch) url +="::arch-req.txt";
            else url+=":::-o"+this.loadedID+(this.referenceID!==undefined?("-reference"+this.referenceID):"")+"-"+node.blob
            url +="//objs="+this.loadedID+"&tqs="+this.tqs_slice_quoted;
        }
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
                url = urlExchangeParameter(url, "idSub", '-1');
                var cutOff = docLocValue("cutOffCall",0,this.getDS("snp-calls").url);
                if (cutOff.length)
                    url = urlExchangeParameter(url, "cutOffCall",cutOff)
            }
            else if(node.operation=="profVCF"){
                var cutOff = docLocValue("cutOffCall",0,this.getDS("snp-calls").url);
                if (cutOff.length)
                    url = urlExchangeParameter(url, "cutOffCall",cutOff)
            }
            else if(node.operation == "profSummaryAll") {
                url=urlExchangeParameter(url, "cmd", 'profSummary');
                url = urlExchangeParameter(url, "idSub", '0');
            }
            else if(node.operation == "profContigAll") {
                url=urlExchangeParameter(url, "cmd", 'profContig');
                url = urlExchangeParameter(url, "idSub", '0');
            }
            else if (node.operation.indexOf("profConsensus")>=0){
                url = urlExchangeParameter(url,"consensus_thrshld",docLocValue("consensus_thrshld","-",this.getDS('consensus').url) );
            }
            else if (node.operation.indexOf("ionGenBankAnnotPosMap")>=0){
                url = this.getDS("annotationData").url;
                url = url.replace(/http:\/\/\?/,"?");
            }
        }
        return url;
    };

    this.onPerformProfileOperation = function (viewer, node, ir, ic)
    {
        var isArch = (viewer.tblArr.hdr[ic].name=="arch");
        var isDown = (viewer.tblArr.hdr[ic].name=="down");
        var url = this.makeProfileOperationURL(viewer, node, node.operation,isArch);

        
        if(isDown){
            url = urlExchangeParameter(url, "down", '1');
            
            if ( url.indexOf("qpbg")==0)
                this.dsQPBG.reload(url,true,{loadmode:"download",saveas:node.blob});
            else if( docLocValue("backend",0,url) ){
                var saveAs = "o"+this.loadedID+"-Summary-All.csv";
                this.dsQPBG.reload("qpbg_http://"+url,true,{loadmode:"download",saveas:saveAs});
            }
            else
                document.location = url;
        }
        else if(isArch) {
            url = urlExchangeParameter(url, "arch", 1);
            var dstName = node.data + " ("+this.loadedID+")";
            var ext = "-";
            switch (node.arch) {
            case "dna":
                ext = "genome";
                dstName += ".fasta";
                break;
            case "dnaold":
                ext = "fasta";
                dstName += ".fasta";
                break;
            case "ico-file":
                ext = "txt";
                dstName += "."+ext;
                break;
            default :
                ext= "-";
            }
            url = urlExchangeParameter(url, "arch_dstname", dstName);
            url = urlExchangeParameter(url, "cgi_dstname", dstName);
            url = urlExchangeParameter(url, "ext", ext);
            if ( url.indexOf("qpbg")==0)
                this.dsQPBG_digest.reload(url,true);
            else
                this.dsQPBG_digest.reload("qpbg_http://"+url,true);
        }
    };

    this.onZoomProfileOperation = function (viewer, node, ir, ic)
    {
        if (!node.Position) return;
        else
            this.zoomProfileOnPosition(node.Position);
    };
    
    this.onRenderIonAnnotList = function (viewer, node, ir, ic)
    {
        
        var table = viewer.tab.viewers[1];
        var idArr = [];
        /*for (var ia=0; ia < table.checkedNodes.length; ++ia){
            idArr.push(table.checkedNodes[ia]['id']);
        }*/
        for (var ia=0; ia < table.selectedNodes.length; ++ia){
            idArr.push(table.selectedNodes[ia]['id']);
        }
        console.log(idArr);
        var url = this.getDS("annotationData").url;
        if (url.indexOf("static://")!=-1){
            url = "http://?cmd=ionGenBankAnnotPosMap&cnt=-1&objs=" + this.loadedID + "&mySubID=" + this.referenceID;
        }
        url = urlExchangeParameter(url, "ionObjs", idArr.join(","));
        var typeArr = [];
        var typeNode = viewer.tree.findByName("types");
        if (typeNode){
            for (var it=0; it<typeNode.children.length; ++it){
                var childNode = typeNode.children[it];
                if (childNode.value!=undefined && childNode.value) {typeArr.push(childNode.name);}
            }
            url = urlExchangeParameter(url, "features", typeArr.join(","));
        }
        //
        var info = viewer.tree.findByName("announcement");
        var aa = dvinfo.tabs[0];
        if (typeArr.length){
            info.hidden = true;
            this.getDS("annotationData").reload(url,true);
            viewer.refresh();
            aa.viewers[8].hidden=false;
            aa.refresh();
        }
        else {
            info.hidden = false;
            info.title = "Please select a type to show";
            viewer.refresh();
            aa.viewers[8].hidden=true;
            aa.refresh();
        }
        
    };
    this.onSelectIonAnnotList = function (viewer, node, ir, ic){
        var nodeIdSelected = [];
        for (var i=0; i<viewer.selectedNodes.length; ++i){
            nodeIdSelected.push(viewer.selectedNodes[i].id);
        }
        vjDS["dsAnnotTypes"].reload("http://?cmd=ionAnnotTypes&fromComputation=0&cnt=-1&ionObjs=" + nodeIdSelected.join(","),true);
        vjDS["dsAnnotTypes"].viewer=viewer;
        
        vjDS["dsAnnotTypes"].parser = function (ds,text){
            console.log("reconstruct panel ")
            var panelV = this.viewer.tab.viewers[0];
            var nodeList = [];
            for (var i=0; i<panelV.rows.length; ++i){
                var n=panelV.rows[i];
                if (n.path == undefined || n.path.indexOf("/types/")==-1){
                    nodeList.push(n);
                }
            }
            panelV.rows = nodeList;
            if (text.length){
                console.log(text)
                var textSplit = text.split("\n");
                for (var iN=0; iN<textSplit.length; ++iN){
                    var name = textSplit[iN];
                    if (!name || !name.length) continue;
                    if (name.indexOf("....")!=-1) continue;
                    var nodeName = name.replace(/\s+/g,"_");
                    var myNode = {name: nodeName, title: name, align: "left", value:0, description: name, type: "checkbox", path: "/types/" + nodeName};
                    panelV.rows.push(myNode);
                    if (name.toLowerCase().indexOf("features")!=-1){
                        panelV.rows.push({name: "CDS", title: "CDS", align: "left", value:0, description: "CDS", type: "checkbox", path: "/types/" + "CDS"});
                        panelV.rows.push({name: "misc_feature", title: "misc_feature", align: "left", value:0, description: "misc_feature", type: "checkbox", path: "/types/" + "misc_feature"});
                        panelV.rows.push({name: "mat_peptide", title: "mat_peptide", align: "left", value:0, description: "mat_peptide", type: "checkbox", path: "/types/" + "mat_peptide"});
                    }
                }
                panelV.render();
            }
            return text;
        }
    };

    this.zoomProfileOnPosition=function(Highlighted)
    {
        var panel = vjDV.locate(this.dvzoom + "." + "profile" + ".0");
        panel.graphResolutionZoom = this.graphResolutionZoom;
        var url = "qpbg_tblqryx4://" + this.seqalign_SNPprofile_cmd_zoom + "&tqs=" + this.tqs_zoom_profile_quoted;
        // alHigh does nothing in tblqry. It serves as flag for the panel to show the highlighted chosen position and generate the right tqs.
        url = panel.urlExchangeParameter(url, "alHigh", Highlighted);
        this.getDS('zoom_profile').reload(url, true);

        if (this.isProfileInfo) {
            var url = "qpbg_tblqryx4://ProfileInfo.csv//objs=" + this.loadedID + "&" + this.seqalign_SNPprofile_cmdX_zoom + "&tqs=" + this.tqs_zoom_profileX_quoted;
            url = panel.urlExchangeParameter(url, "alHigh", Highlighted);
            this.getDS('zoom_profileX').reload(url, true);
        }
        this.getDS('zoom_alView').reload("http://?cmd=alView&objs=" + this.loadedID + "&req=" + this.reqID + "&objLink=parent_proc_ids&mySubID=" + this.referenceID + "&alHigh=" + Highlighted + "&start=0&alTouch=1&alWinSize=" + this.graphResolution + "&cnt=20", true);
        this.getDS('zoom_alStack').reload("http://?cmd=alStack&objs=" + this.loadedID + "&req=" + this.reqID + "&objLink=parent_proc_ids&mySubID=" + this.referenceID + "&alHigh=" + Highlighted + "&start=0&alTouch=1&alWinSize=" + this.graphResolution + "&cnt=20", true);
        this.getDS('zoom_alMutBias').reload("http://?cmd=alMutBias&objs=" + this.loadedID + "&req=" + this.reqID + "&objLink=parent_proc_ids&mySubID=" + this.referenceID + "&alHigh=" + Highlighted + "&start=0&alTouch=1&alWinSize=" + this.graphResolution + "&cnt=50", true);

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
        url =  "http://?cmd=profSNPcalls" + "&objs=" + this.loadedID + "&idSub=" + this.referenceID + "&sub_start=" + startPosition +"&sub_end="+ endPosition +"&isORF=1&resolution="+resolution;

        this.getDS('snp-calls-graph-zoom').reload(url, true);

        var url1 = this.getDS('orf').dataurl;
        url1 = urlExchangeParameter(url1,"start", startPosition);
        url1 = urlExchangeParameter(url1,"end", endPosition);
     // orf-zoom
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
               var letter = tblArr.rows[il].cols[2];

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

        var histo_tqs = this.tqs_slice.concat([
            {
                op: "filter",
                arg: {
                    incol: 1,
                    method: "equals",
                    value: node.Frequency,
                }
            }
        ]);
        var histo_tqs_quoted = vjDS.escapeQueryLanguage(JSON.stringify(histo_tqs));
        if( !this.isHistProfile ) {
            alertI("Frequency Histogram has not been generated");
            return ;
        }
        var url = "qpbg_tblqryx4://HistProfile.csv//objs="+this.loadedID+"&tqs=" + histo_tqs_quoted + "&hdr=1";
        this.getDS("freq_histogram_popup").parser = function (ds,text) { text=parseHistProfileTableFromCell(text, 3, "Coverage,CountA,CountT,CountG,CountC", "true");  return text;};
        this.getDS("freq_histogram_popup").reload(url,true);
        //this.viewers["freqHistPopup"].options.hAxis.title = "Frequency = " + node.Frequency;
        gObject(vjDV.dvhistpopupViewer.tabs.coverage.container + "_tabname").innerHTML="Frequency " + node.Frequency+" coverage histogram";
        vjVIS.winop("vjVisual", this.dvhistpopup_visual, "open", true);
    };

    this.onZoomAnnotBox = function (node,evt)
    {
        this.windowZoomStack.push({start:node.min,end:node.max});
        var quarterLengthRange = Math.round((parseInt(node.max) - parseInt(node.min))/4);
        var position = Math.round((parseInt(node.rangeEnd)-parseInt(node.rangeStart))/2) + parseInt(node.rangeStart); //alert("psoition " + position)
        var startPosition = position - quarterLengthRange; if (startPosition < 0 ) startPosition =0;
        var endPosition = position + quarterLengthRange;
        //url = "http://?cmd=anotNumberOfRange&subID=" + this.referenceID + "&objs=" + this.loadedID + "&dataName=CDS" + "&whatToOutPut=locus|rangeStart|rangeEnd|id" + "&startPosition=" + startPosition + "&endPosition=" + endPosition; //LAM
        url = this.getDS('orf').dataurl;
        url = urlExchangeParameter(url,"start", startPosition);
        url = urlExchangeParameter(url,"end", endPosition);

        this.getDS('orf-zoom').reload(url, true);
        //alerJ("",this.getDS("orf-zoom"));
        var resolution = endPosition - startPosition; if (resolution>200) resolution = 200;
        var url1 =  "http://?cmd=profSNPcalls" + "&objs=" + this.loadedID + "&idSub=" + this.referenceID + "&sub_start=" + startPosition + "&sub_end=" + endPosition + "&isORF=1&resolution=" + resolution;
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



