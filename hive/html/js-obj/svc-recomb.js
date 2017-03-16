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
javaScriptEngine.include("js/vjRecombView.js");

vjHO.register('svc-recomb').Constructor=function ()
{

    if(this.objCls)return;         //stupid chrome loads from both cached file and the one coming from server.

    // two public functions which must be supported
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

    this.typeName="svc-recomb";
    this.graphResolution = 200;
    this.graphResolutionZoom = 200;
    if(document.all) { // IE Cannot show more than some little number of HTML5 s
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }


    this.windowZoomStack = new Array({windwStart:0,windwEnd:0});
    this.urlStack = new Array();

    this.recomb_cmd = "hdr=1&resolution=" + this.graphResolution;
    this.profile_cmd = "cols=0,1,6-12&hdr=1&minmaxCols=2-8&resolution="+this.graphResolution;

    this.mimicCheckmarkSelected=1;
    
    this.objCls="obj-svc-recomb"+Math.random();
    vjObj.register(this.objCls,this);
    this.urlSet ={
            'hitlist': {
                active_url: "http://?cmd=alCount&objLink=parent_proc_ids&start=0&cnt=50&info=1",
                objs:'objs',
                title: "Retrieving list of hits"
            },
            'polyplot' : {
                loadInactive:true,
                active_url:"qpbg_tblqryx4://RecombPolyplotSimilarity.csv//"+this.recomb_cmd,
                title: "Building profile diagram"
            },
            'coverage' : {
                loadInactive:true,
                active_url:"qpbg_tblqryx4://RecombPolyplotCoverage.csv//"+this.recomb_cmd,
                title: "infrastructure: Creating profilerX"    //it won't be displayed because is infrustructure
            },
            'cross_cov1' : {
                loadInactive:true,
                active_url:"http://?cmd=recombCross",
                inactive_url:"static://Select two reference sequences",
                title: "infrastructure: Creating cross coverage"    //it won't be displayed because is infrustructure
            },
            'profiler' : {
                loadInactive:true,
                active_url:"qpbg_tblqryx4://RecombSNPProfile.csv//"+this.profile_cmd,
                title: "Retrieving open reading frames",
            },
            'downloads' : {
                loadInactive:true,
                active_url:"static://",
                inactive_url:"static://",
                title: "infrastructure: creating list of downloadables"
            },
            'help' : {
                active_url:"http://help/hlp.svc-recomb.results.html",
                inactive_url:"http://help/hlp.svc-recomb.results.html",
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
            dv.addTab("similarity","graph",[this.viewers['hitlist'],this.viewers['polyplot_scaled']]).columns=2;
            dv.render(true);
            dv.load('rerender');
        }
        else{

            this.dvname=this.current_dvORtab[0].name;

            this.current_dvORtab[0].addTab("similarity","graph",[this.viewers['polyplot'],this.viewers['polyplot_scaled']]);
            this.current_dvORtab[0].addTab("coverage","graph",[this.viewers['coverage'],this.viewers['coverage_scaled']]);
            this.current_dvORtab[0].addTab("profile","graph",[this.viewers['profile_coverage'],this.viewers['profile_snp'],this.viewers['profile_indels']]).viewtoggles = 1;
            this.current_dvORtab[0].addTab("recombinations","graph",[this.viewers['cross_cov_panel'],this.viewers['cross_cov']]);
            this.current_dvORtab[0].addTab("downloads","table",[this.viewers['downloads']]);
            this.current_dvORtab[0].addTab("help","help",[this.viewers['help']]);

            this.current_dvORtab[0].render();

            this.current_dvORtab[0].load('rerender');

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

            this.addviewer('hitlist_panel,hitlist', new vjAlignmentHitListControl({
                data:'hitlist',
                formName:this.formName,
                width:'20%',
                checkable : true ,
                checkCallback : "function:vjObjFunc('dna_recomb_checkedReference','" + this.objCls + "')" ,
                isok:true}));
            this.viewers['hitlist'].callbackRendered = "function:vjObjFunc('onLoadedHitList','" + this.objCls + "')";
//            this.viewers['hitlist'].rowspan=2;

            this.addviewer('polyplot,polyplot_scaled', new vjRecombPolyplotControl({
                data: 'polyplot',//{'profile':'profile', 'profileX': 'profileX'},
                width:'80%',
                height:240,
                formName:this.formName,
                isok:true}));

            this.addviewer('null', new vjHTMLView ({data:"dsVoid",width:"80%"}));

        }
        else{

            this.formName=[''];

            var formNode=gAncestorByTag(gObject(dvORtab.obj[0].name),"form");
            if(formNode)
                this.formName[0]=formNode.attributes['name'].value;

            this.current_dvORtab=dvORtab.obj;

            this.dvname=this.current_dvORtab[0].name;

            this.addviewer('polyplot,polyplot_scaled', new vjRecombPolyplotControl ({
                data: 'polyplot',
//                selectCallback: "function:vjObjFunc('','" + this.objCls + "')",
                logGraph:true,
                formName:this.formName[0],
                isok:true}));

            this.addviewer('coverage,coverage_scaled', new vjRecombCoverageControl ({
                data: 'coverage',
//                selectCallback: "function:vjObjFunc('','" + this.objCls + "')",
                logGraph:true,
                formName:this.formName[0],
                isok:true}));
            
            this.addviewer('cross_cov_panel,cross_cov',new vjRecombCrossCovControl({
               data: 'cross_cov1',
                formName:this.formName[0],
                isok:true}));

            this.addviewer('profile_coverage,profile_snp,profile_indels', new vjRecombSNPProfileControl ({
                data: 'profiler',
//                selectCallback: "function:vjObjFunc('','" + this.objCls + "')",
                formName:this.formName[0],
                isok:true}));

          this.addviewer('downloads', new vjRecombDownloadsView ({
              data: 'downloads',
              formName:this.formName[0],
              selectCallback: "function:vjObjFunc('onPerformRecombinatorOperation','" + this.objCls + "')",
              isok:true}));

            this.addviewer('help', new vjHelpView ({
                data: 'help',
                formName:this.formName[0],
                isok:true}));
        }
    };



    this.dna_recomb_checkedReference = function ( viewer, node) {
        var subIDList=vjDV.locate(this.dvname+ ".similarity.0").accumulate("node.checked","node.id");
        this.reload(null, subIDList);
    };

    this.reload=function(loadedID,subID)
    {
        if(subID){
            this.referenceID=verarr(subID);
        }
        if(loadedID){
            this.loadedID=loadedID;
        }
        var subSet=this.referenceID.join(",");
        this.referenceID.forEach(function(el){subSet+=","+el.id;},subSet);
        subSet = subSet.substring(1);
        if(this.mode=='preview'){
            var viewer = vjDV.locate(this.dvname+ "._active.3");
            if(viewer)viewer.options.colors = colors;

            this.getDS('polyplot').reload( urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(this.urlSet['polyplot'].active_url, "minmaxCols","1-"+this.referenceID.length), "cols","0,"+subSet), "objs",this.loadedID),false);
            var doseries=new Array({name:'position',notScaled:true});

            var clrs=new Array () ;
            for(var is=0; is<this.referenceID.length; ++is) {
                doseries.push({col:is+1});
                clrs.push(vjGenomeColors[(Int(is+1))%vjGenomeColors.length]);
            }
            v=vjDV.locate(this.dvname+ ".similarity.1");v.series=doseries;v.options.colors=clrs;

            vjDV.locate(this.dvname + "._active.").load(true);
        }
        else{

            var t = "download,icon,blob\n";
            t += "Recombination Similarity Polyplot,download,RecombPolyplotSimilarity\n" +
                "Recombination Coverage Polyplot,download,RecombPolyplotCoverage\n\n";
            this.getDS('downloads').reload("static://" + t, false);


            this.getDS('polyplot').reload(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(this.urlSet['polyplot'].active_url, "minmaxCols","1-"+this.referenceID.length), "cols","0,"+subSet), "objs",this.loadedID),false);
            this.getDS('coverage').reload(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(this.urlSet['coverage'].active_url, "minmaxCols","1-"+this.referenceID.length), "cols","0,"+subSet), "objs",this.loadedID),false);
            this.getDS('profiler').reload(urlExchangeParameter(this.urlSet['profiler'].active_url, "objs",this.loadedID),false);
            if( this.referenceID.length==2 ) {
                var url = this.urlSet['cross_cov1'].active_url;
                url = urlExchangeParameter(url,"sub1",this.referenceID[0].id);
                url = urlExchangeParameter(url,"sub2",this.referenceID[1].id);
                url = urlExchangeParameter(url,"objs",this.loadedID);
                this.getDS('cross_cov1').reload(url,false);
            }


            var doseries=new Array({name:'position',notScaled:true});

            var clrs=new Array () ;
            for(var is=0; is<this.referenceID.length; ++is) {
                doseries.push({col:is+1});
                clrs.push(vjGenomeColors[(Int(this.referenceID[is].irow))%vjGenomeColors.length]);
            }

            var v;
            v=vjDV.locate(this.dvname+ ".similarity.0");v.series=doseries;v.options.colors=clrs;
            v=vjDV.locate(this.dvname+ ".similarity.1");v.series=doseries;v.options.colors=clrs;
            v=vjDV.locate(this.dvname+ ".coverage.0");v.series=doseries;v.options.colors=clrs;
            v=vjDV.locate(this.dvname+ ".coverage.1");v.series=doseries;v.options.colors=clrs;
            v=vjDV.locate(this.dvname+ ".recombinations.1");v.options.colors=clrs;


            vjDV.locate(this.dvname + "._active.").load(true);

        }
    };

    this.onPerformRecombinatorOperation = function(viewer, node, ir, ic)
    {
        var url = "?cmd=objFile&ids=" + dna_recomb_ID + "&filename="+node.blob+".csv";
        document.location = url;
    };



    this.onLoadedHitList = function ()
    {
        var viewer = vjDV.locate(this.dvname+ "._active.0");

        if(this.mimicCheckmarkSelected!==false || this.mimicCheckmarkSelected!==undefined || this.mimicCheckmarkSelected!==null)
            viewer.mimicCheckmarkSelected(this.mimicCheckmarkSelected,true);
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



