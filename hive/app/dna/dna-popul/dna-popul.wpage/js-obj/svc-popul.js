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

javaScriptEngine.include("?cmd=~^dna-popul$~vjPopulView.js",true);
javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js/vjTreeSeries.js");
javaScriptEngine.include("js/vjSankey.js");

javaScriptEngine.include("js/vjBumperLocater.js");
javaScriptEngine.include("?cmd=~^dna$~js/vjSVGGeneView.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("?cmd=~^dna$~js-graph/vjSVG_GenomeBrowser.js");

vjHO.register('svc-popul').Constructor=function ()
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
        this.load(dv,node.id,node.parent_id);
        if(this.onFullviewLoadCallback) {
            funcLink(this.onFullviewLoadCallback, this);
        }

        this.construct();
        if(this.onFullviewRenderCallback) {
            funcLink(this.onFullviewRenderCallback, this);
        }
        this.isRendered=true;
    };

    this.preview = function(node,dv)
    {
        this.parent.preview('svc',node,dv);
        if(!node.status || parseInt(node.status)<5)return ;

        this.load(dv,node.id,node.parent_id);
        if(this.onPreviewLoadCallback ) {
            funcLink(this.onPreviewLoadCallback, this);
        }

        this.construct();
        if(this.onPreviewRenderCallback) {
            funcLink(this.onPreviewRenderCallback, this);
        }

        this.isRendered=true;
    };
    this.maxViewersTree = 5;
    this.typeName="svc-popul";
    this.graphResolution = 200;
    this.graphResolutionZoom = 200;
    
    if(document.all) {
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }
    this.objCls="obj-svc-popul"+Math.random();
    vjObj.register(this.objCls,this);

    this.downloadSummaryTable = "data,down,digest,operation,blob,args\n"
        + "Summary,download,ico-file,popSummary,,\n"
        + "Coverages,download,ico-file,popCoverage,,\n"
        + "Sequences,download,ico-file,popConsesus,,\n";
    
    
    this.defCloneAbsUrl = "http:
    this.defCloneExportUrl ="http:
    this.defAnnotationUrl ="http:
    
    this.urlSet ={
            'summary': {
                active_url: "http:
                header:"Name,Start,End,Merge,Hierarchy",
                title: "creating summary"
            },
            "length_summary": {
                active_url: "http:
                title: "creating length summary"
            },
            'clone_abs': {
                isSeries:true,
                active_url: this.defCloneAbsUrl,
                title: "creating clones in absolute coverage"
            },
            "treeVoid": {
                title: "Retrieving clustering hierarchy",
                active_url: "static:
                objs: "ids",
                isSeries: true
            },
            'hitlist': {
                active_url: "http:
                make_active_url :function (url){return urlExchangeParameter(url,"objs",this.parentID);},
                title: "Retrieving list of hits",
                isSeries: true
            },
            'export': {
                active_url:this.defCloneExportUrl,
                title: "creating export samples"
            },
            'consensus': {
                loadInactive:true,
                active_url:"static:
                inactive_url:"static:
                title: "creating list of downloadables"
            },
            'annotationData': {
                isSeries:true,
                active_url:this.defAnnotationUrl,
                title: "creating list of downloadables"
            },'help' : {
                active_url:"http:
                inactive_url:"http:
                title: "building help section"
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
            dv.addTab("summary","table",[this.viewers['summary_panel'],this.viewers['summary']]);
            dv.render(true);
            dv.load('rerender');
        }
        else{
            this.dvname=this.current_dvORtab[0].name;
            this.dvinfo=this.current_dvORtab[1].name;
            this.dvTrees=this.current_dvORtab[2].name;

            this.current_dvORtab[0].addTab("Summary","table",[this.viewers['summary_panel'],this.viewers['summary']]);
            this.current_dvORtab[0].addTab("Lengths","table",[this.viewers['length_summary']]).forceLoadAll=true;

            this.current_dvORtab[1].addTab("Contigs","graph",[this.viewers['clone_abs_panel'],this.viewers['clone_abs'],this.viewers['anot_abs']]).viewtoggles = 1;
            this.current_dvORtab[1].addTab("Export","download",[this.viewers['export_panel'],this.viewers['export']]);
            this.current_dvORtab[1].addTab("Help","help",[this.viewers['help']]);

            var treeViewers = [];
            for (var i=0; i<this.maxViewersTree; i++) {
                treeViewers.push(this.viewers[this.treeViewerName(i)]);
            }

            this.current_dvORtab[2].addTab("trees","graph",treeViewers);
            this.current_dvORtab[0].render();
            this.current_dvORtab[1].render();
            this.current_dvORtab[2].render();

            this.current_dvORtab[0].load('rerender');
            this.current_dvORtab[1].load('rerender');

        }

    };


    this.load=function(dvORtab,id,parentID)
    {
        this.loadedID=id;
        this.loaded=true;
        this.constructed=false;
        if(parentID) this.parentID = parentID;
        this.cloneIDs = new Array();

        if(this.mode=='preview'){
            this.formName='';
            var formNode=gAncestorByTag(gObject(dvORtab.obj.name),"form");
            if(formNode)
                this.formName=formNode.attributes['name'].value;

            this.current_dvORtab=dvORtab;

            this.dvname=this.current_dvORtab.obj.name;

            this.addviewer('summary_panel,summary',new vjPopulSummaryControl ({
                data:'summary',
                checkBranchCallback:"function:vjObjFunc('onSummaryCheckOperation','"+this.objCls+"');",
                checkLeafCallback:"function:vjObjFunc('onSummaryCheckOperation','"+this.objCls+"')",
                formName:this.formName,
                isok:true}));

        }
        else{
            this.formName=['',''];
            var formNode=gAncestorByTag(gObject(dvORtab.obj[0].name),"form");
            if(formNode)
                this.formName[0]=formNode.attributes['name'].value;
            formNode=gAncestorByTag(gObject(dvORtab.obj[1].name),"form");
            if(formNode)
                this.formName[1]=formNode.attributes['name'].value;
            formNode=gAncestorByTag(gObject(dvORtab.obj[2].name),"form");

            this.current_dvORtab=dvORtab.obj;

            this.dvname=this.current_dvORtab[0].name;
            this.dvinfo=this.current_dvORtab[1].name;
            this.dvTrees=this.current_dvORtab[2].name;

            this.dvTree_visual=this.dvTrees.replace("Viewer","");
            var tree_flV = vjVIS.flVs[this.dvTree_visual];
            var treezoomVIS=undefined;
            if(tree_flV){
                treezoomVIS=new Object();
                treezoomVIS.obj=vjVIS;
                treezoomVIS.flV=tree_flV;
            }

            that = this;
            this.addviewer('summary_panel,summary',new vjPopulSummaryControl ({
                data:['summary','clone_abs','export' ],
                checkBranchCallback:"function:vjObjFunc('onSummaryCheckOperation','"+this.objCls+"');",
                checkLeafCallback:"function:vjObjFunc('onSummaryCheckOperation','"+this.objCls+"')",
                onMouseOverCallback:"function:vjObjFunc('onSummaryOverOperation','"+this.objCls+"')",
                resetPanelCallback:"function:vjObjFunc('onResetCallback','"+this.objCls+"')",
                hideClonesCallback:"function:vjObjFunc('onhideClonesCallback','"+this.objCls+"')",
                onSummaryRender:"function:vjObjFunc('onSummaryRenderCallback','"+this.objCls+"')",
                formName:this.formName[0],
                isok:true}));
            
            this.addviewer('length_summary',new vjPopulLengthSummary ({
                data:'length_summary',
                checkCallback:"function:vjObjFunc('onSummaryLengthCheckOperation','"+this.objCls+"')",
                formName:this.formName[0],
                isok:true}));
            
            this.addviewer('clone_abs_panel,clone_abs,anot_abs',new vjPopulCloneAbsControl({
                    data:['clone_abs','hitlist','annotationData'],
                    summaryViewerName:this.viewers['summary'],
                    colors:valgoGenomeColors.slice(1),
                    onCloneClick:"function:vjObjFunc('onSankeyClick','"+this.objCls+"')",
                    onCloneOver:"function:vjObjFunc('onSankeyOver','"+this.objCls+"')",
                    onCloneOut:"function:vjObjFunc('onSankeyOut','"+this.objCls+"')",
                    onSankeyRenderCallback:"function:vjObjFunc('onSankeyRender','"+this.objCls+"')",
                    formName:this.formName[1],
                    isok:true}));

            this.phylogram = new Array();
            for (var ip=0; ip < this.maxViewersTree; ip++) {
                this.makePhylogram(ip);
                this.addviewer(this.treeViewerName(ip), new vjSVGView({
                    chartArea: {width: "85%", height: "95%"},
                    formObject: formNode,
                    parentVIS:treezoomVIS,
                    plots: [this.phylogram[ip]]
                }));
            }

            this.addviewer('export_panel,export', new vjPopulExportControl ({
                data: ['export'],
                formName:this.formName[1],
                onExportDigestCallback:"function:vjObjFunc('onExportDigest','" + this.objCls + "')",
                selectCallback: "function:vjObjFunc('onPerformPopulationOperation','" + this.objCls + "')",
                isok:true}));

            this.addviewer('help', new vjHelpView ({
                data: 'help',
                formName:this.formName[1],
                isok:true}));
            

            this.getDS('clone_abs').title = "o"+this.loadedID+"-Sankey";
            this.getDS('clone_norm').title = "o"+this.loadedID+"-Sankey(normalized)";
            if(this.algoProc) {
                this.fastaNameTmpl = escape("$_(v)|object "+this.loadedID+"|"+this.algoProc.getValue("name"));
                this.getDS('clone_abs').title = "o"+this.loadedID+"-Sankey-"+this.algoProc.getValue("name");
                this.getDS('clone_norm').title = "o"+this.loadedID+"-Sankey(normalized)-"+this.algoProc.getValue("name");
            }
        }
    };
    this.onSummaryRenderCallback = function ()
    {
        

    };
    
    this.onhideClonesCallback = function ()
    {
        this.viewers['summary_panel'].tree.findByName("hideClones").value = this.hiddenIDs.join(",");
        this.viewers['summary_panel'].onUpdate();

    };

    this.onResetCallback = function(a,b){
        this.hiddenIDs = [];
        this.cloneIDs = [];
        this.onResetCloneDS();
    };

    this.onSelectReferenceID = function ( viewer, node)
    {
        this.referenceID = node?node.id:undefined;
        var annotUrl = this.getDS('annotationData').url;
        annotUrl = urlExchangeParameter(annotUrl, 'mySubID', this.referenceID);
        this.getDS('annotationData').reload(annotUrl,false);
        this.annotObjID = docLocValue('ionObjs','',annotUrl);
        
        if( this.referenceID && this.annotObjID) {
            annotUrl = urlExchangeParameter(annotUrl, 'ionObjs', this.annotObjID);
            this.getDS('annotationData').reload(annotUrl,false);
            vjDV[this.dvinfo].load();
        }
        
    };

    this.onSankeyRender = function(viewer,txt,page_req) {
        var plot = this.viewers['clone_abs'].plots[0];
        var pos_start = plot.min.x, pos_end = plot.max.x;
        var panelViewer = this.viewers['clone_abs_panel'];
        var paneltree = panelViewer.tree;
        if( !paneltree ) return;
        var nodeStart = paneltree.findByName('pos_start');
        var nodeEnd = paneltree.findByName('pos_end');
        nodeStart.value = pos_start;
        nodeEnd.value = pos_end;
        var annotUrl = this.getDS('annotationData').url;
        annotUrl = urlExchangeParameter(annotUrl, 'pos_start',pos_start);
        annotUrl = urlExchangeParameter(annotUrl, 'pos_end',pos_end);
        this.getDS('annotationData').reload(annotUrl,false);
        panelViewer.redrawMenuView();
    };
    
    this.onSankeyClick = function ( obj, ir ) {
        this.SankeyEvent = true;
        var node = this.viewers['summary'].tree.findByName(obj._branchID);
        if(node){
            this.viewers['summary'].mimicCheckmarkClick( node.path );
        }
        var pos = obj._positions;
        if( this.TreeList === undefined )
            ajaxDynaRequestPage("?cmd=objFile&filename=treesList.jsn&raw=1&ids="+this.loadedID,{objCls: this.objCls, callback:'parseTreeList',params:{pos:pos}}, vjObjAjaxCallback);
        else if(this.TreeList != "unavailable") {
            this.findTreesInPos(pos);
        }
        this.SankeyEvent = false;
    };

    this.onSankeyOver = function ( obj, ir ) {
        this.SankeyEvent = true;
        var node = this.viewers['summary'].tree.findByName(obj._branchID);
        if(node){
            this.viewers['summary'].onMouseOver(undefined,node.path,1);
        }
        var pos = obj._positions;
        if( this.TreeList === undefined )
            ajaxDynaRequestPage("?cmd=objFile&filename=treesList.jsn&raw=1&ids="+this.loadedID,{objCls: this.objCls, callback:'parseTreeList',params:{pos:pos}}, vjObjAjaxCallback);
        else if(this.TreeList != "unavailable") {
            this.findTreesInPos(pos);
        }
        this.SankeyEvent = false;
    };

    this.onSankeyOut = function ( obj, ir ) {
        this.SankeyEvent = true;
        var node = this.viewers['summary'].tree.findByName(obj._branchID);
        if(node){
            this.viewers['summary'].onMouseOver(undefined,node.path,0);
        }
        var pos = obj._positions;
        if( this.TreeList === undefined )
            ajaxDynaRequestPage("?cmd=objFile&filename=treesList.jsn&raw=1&ids="+this.loadedID,{objCls: this.objCls, callback:'parseTreeList',params:{pos:pos}}, vjObjAjaxCallback);
        else if(this.TreeList != "unavailable") {
            this.findTreesInPos(pos);
        }
        this.SankeyEvent = false;
    };
    this.onSummaryLengthCheckOperation = function (vv,node,currsel)
    {
        if(!this.SummaryEvent){
            this.LengthEvent = true;
            var tree_node = this.viewers['summary'].tree.findByName(node.clone);
            if(tree_node){
                this.viewers['summary'].mimicCheckmarkClick( tree_node.path );
            }
            this.LengthEvent = false;
        }
    };
    
    this.onSummaryCheckOperation = function(vv,node,op){
        this.SummaryEvent = true;
        var viewerSankey=this.viewers['clone_abs'];

        if(!this.LengthEvent){
            if(this.viewers['length_summary'].tblArr){
                var t_node = this.viewers['length_summary'].accumulate("node.clone==params.name","node.irow",{'name':node.name});
                if(t_node && t_node.length){
                    t_node = t_node[0];
                    this.viewers['length_summary'].mimicCheckmarkSelected( t_node, node.checked );
                } 
            }
        }

        function highlightPlot(plot,sankeyEvent) {
            var svgObj = plot.findSVGbyID(node.name);
            var obj = plot.getObjbySVG(svgObj);

            if( obj && svgObj && !sankeyEvent){
                obj.selected = obj.selected?0:1;
                plot.highlightSVGClone(svgObj,obj.selected?"selected":"highlighted");
            }

        }

        var plotAbs = viewerSankey.plots[0];
        if( viewerSankey.isVisible() ){
            highlightPlot(plotAbs,this.SankeyEvent);
        }

        var urlSankey=this.getDS('clone_abs').url,
        urlHier=this.getDS('summary').url;
        urlExport=this.getDS('export').url;
        var hiddenstring=docLocValue("hideClones", -1, urlSankey);
        var hidden=new Array();
        if(hiddenstring!=-1){
            hidden=hiddenstring.split(",");
        }

        var n_cloneIDs = this.viewers['summary'].getCheckedNode();
        this.cloneIDs=[];
        for(var i = 0 ; i < n_cloneIDs.length ; ++i )
        {
            if(n_cloneIDs[i].Name)this.cloneIDs.push( parseInt(n_cloneIDs[i].Name.split("_")[1]) );
        }
        if(hidden.length){
            urlSankey=urlExchangeParameter(urlSankey,"hideClones",hidden.join(","));
            urlExport=urlExchangeParameter(urlExport,"hideClones",hidden.join(","));
        }
        var current_export = docLocValue("cloneIDs","",urlExport);
        if(current_export != this.cloneIDs.join(",") && this.cloneIDs.length) {
            urlExport=urlExchangeParameter(urlExport,"cloneIDs",this.cloneIDs.join(","));
            this.getDS('export').reload(urlExport,false);
            vjDV[this.dvinfo].load();
        }

        this.hiddenIDs = hidden.concat(this.cloneIDs);

        this.SummaryEvent = false;
    };

    this.onSummaryOverOperation = function(vv,node,state,event) {
        var event_owner = gObject(sanitizeElementId(""+vv.container + node.path));
        if( event && !eventOrigins(event_owner,event) )return ;
        this.SummaryEvent = true;
        var viewerSankey=this.viewers['clone_abs'];
        var plotAbs = viewerSankey.plots[0];

        function highlightPlot(plot,sankeyEvent,state1) {
            var svgObj = plot.findSVGbyID(node.name);
            var obj = plot.getObjbySVG(svgObj);
            if (!obj){
                return ;
            }
            if(state1) {
                state1 = "highlighted";
            }
            else{
                state1 = obj.selected?"selected":"normal";
            }
            if( !sankeyEvent && svgObj && !plot.isCloneInState(svgObj,state1) ){
                plot.highlightSVGClone(svgObj,state1);
            }
        }

        if( viewerSankey.isVisible() ){
            highlightPlot(plotAbs,this.SankeyEvent,state);
        }


        this.SummaryEvent = false;
    };

    gColorMap_sankey = new Object();

    this.parseTreeList = function(ajax_obj,text,id){
        var params = ajax_obj.params;
        var pos = params.pos;
        this.TreeList = "unavailable";
        if(text) {
            this.TreeList = JSON.parse(text) ;
            var list_length = 0;
        }
        if(!this.TreeList || this.TreeList=="unavailable") {
            return;
        }
        this.findTreesInPos(pos);
    };




    this.onExportDigest = function (vv,aa) {
        var url = this.getDS('export').url;
        var _dataUrl_sep=url.indexOf("
        if(_dataUrl_sep>=0)
            url=url.substr(_dataUrl_sep+2);
        var print_type = docLocValue('contig_print','',url);
        if(!print_type) {
            return ;
        }
        url = urlExchangeParameter(url,'arch','1');

        var cloneIDs = docLocValue('cloneIDs','',url);
        var sLen = docLocValue('minCloneSup','',url);
        var sSup = docLocValue('minCloneLen','',url);
        var sCov = docLocValue('minCloneCov','',url);
        var hiddenCls = docLocValue('hideClones','',url);
        var merged = docLocValue('mergeHidden','',url);
        var title =  "Clone"+(cloneIDs.split(",").length<=1?"":"s")+ cloneIDs;
        if( sLen || sSup || sCov ) {
            title +=" filtered (";
            var filters = "";
            if(sLen){
                filters += ",Length:"+sLen;
            }
            if(sSup){
                filters += ",Support:"+sSup;
            }
            if(sCov){
                filters += ",Coverage:"+sCov;
            }
            title += filters.substr(1)+")";
        }
        if(hiddenCls){
            title += " (contigs "+hiddenCls+" ignored)";
        }
        if(merged){
            title += " merging hidden)";
        }
        
        title +=" from "+ this.loadedID;
        url = urlExchangeParameter(url,'clcnt',0);
        if(print_type=='seq') {
            url = urlExchangeParameter(url,'ext','fasta');
            title+=".fasta";
        }
        url = urlExchangeParameter(url, "arch_dstname", title);
        this.dsQPBG_digest.reload("qpbg_http:
    };
    
    this.buildTreeFileName = function (ind) {
        return "resul_"+ind+".tre";
    };

    this.findTreesInPos = function(pos) {
        var p = pos.cord;
        function compTreeList(a, b) {
            return a.end -b;
        }
        var treeInds = new Array();
        var iTr = arraybinarySearch(this.TreeList, p, compTreeList, true,false);
        treeInds.push(iTr);
        for(var itt = iTr+1 ; itt < this.TreeList.length ; ++itt ) {
            var start = this.TreeList[itt].start;
            if( start<= p ) {
                treeInds.push(itt);
            }
            else {
                break;
            }
        }
        var trRnd = new Array();
        for(var it = 0 ; it < treeInds.length ; ++it) {
            trRnd.push( this.TreeList[treeInds[it]] );
        }
        this.renderTrees(trRnd);

        vjVIS.winop("vjVisual", this.dvTree_visual, "open", true);
    };

    this.treeViewerName = function (id) {
        return "tree_" + id;
    };

    this.createTreeSource = function( treeObj ) {
        var treeID = treeObj.id;
        var dsname = ["ds", this.dvTrees, this.typeName, "tree", this.loadedID, treeID].join("_");
        if (vjDS[dsname])
            return vjDS[dsname];

        var title = "Getting tree results for range ["+treeObj.start+"-"+treeObj.end+"]";
        var url = "http:
        return vjDS.add(title, dsname, url);
    };

    this.renderTrees = function(treeObjs) {


        for (var i=0; i < this.maxViewersTree; i++) {
            var trObj = treeObjs[i];
            var viewer = this.viewers[this.treeViewerName(i)];
            if (i < treeObjs.length) {
                viewer.hidden = false;
                viewer.unregister_callback();

                var source = this.createTreeSource(trObj);
                source = viewer.plots[0].collection[0].changeDS(source);
                viewer.data = [source.name];
                viewer.register_callback();
                source.load();
            } else {
                viewer.hidden = true;
            }
            viewer.render();
        }
    };

    this.makePhylogram = function(ind) {
        if ( !this.phylogram ) this.phylogram = new Array();
        if (this.phylogram[ind])
            return this.phylogram[ind];

        function hiddenProfilesLabel(node) {
            function countLeaves(node, counter) {
                if (node.leafnode)
                    counter.count++;
                if (node.childrenCnt > 0 && node.childrenCnt > node.children.length)
                    counter.exact = false;
                for (var i=0; i<node.children.length; i++)
                    countLeaves(node.children[i], counter);
            }

            var counter = {count: 0, exact: true};
            countLeaves(node, counter);
            return (counter.exact ? "" : ">") + counter.count + " read" + (counter.count > 1 ? "s" : "") + " hidden";
        }

        var this_ = this;

        this.phylogram[ind] = new vjSVG_Phylogram({
            nodeTooltip: function(node) {
                var tip = this.defaultNodeTooltip.apply(this, arguments);
                if (node.leafnode && this.svc_clust_info[node.name]) {
                    tip = tip ? tip+"\n" : "";
                    tip += "ID: " + node.name + "\n" + this.svc_clust_info[node.name].name + "\n" + formaDatetime(this.svc_clust_info[node.name].created);
                } else if (!node.leafnode && !node.expanded) {
                    tip = tip ? tip+"\n" : "";
                    tip += hiddenProfilesLabel(node);
                }
                return tip;
            },
            nodeSymbolSize: function(node) {
                var weight = 1;
                if(node.leafnode){
                    var rptCnt = this.svc_popul_get_rpts(node);
                    if( isNumber(rptCnt) ){
                        weight = Math.log(1 + parseInt(rptCnt));
                    }
                }
                return this.defaultNodeSymbolSize.apply(this, arguments) * weight;
            },
            nodeLabelFont : function(node)
            {
                var ret = this.defaultNodeLabelFont.apply(this, arguments);
                var enlarge = 0;
                if(node.leafnode) {
                    var rptCnt = this.svc_popul_get_rpts(node);
                    if( isNumber(rptCnt) ){
                        enlarge = Math.log(1 + parseInt(rptCnt))-1;
                    }
                }
                ret["font-size"]+= enlarge;
                return ret;
            },
            svc_popul_get_rpts: function(node) {
                return node.name.split("&")[1].split(",")[1];
            },
            svc_clust_info: {}
        });
        var trSeries = this.makeTreeSeries(ind);
        this.phylogram[ind].add(trSeries);
        return this.phylogram[ind];
    };


    this.makeTreeSeries = function(ind) {
        if ( !this.treeSeries ) this.treeSeries = new Array();
        if (this.treeSeries[ind])
            return this.treeSeries[ind];

        this.treeSeries[ind] = new vjTreeSeries({
            name: "treeVoid",
            title: "hierarchical clustering tree",
            dataFormat: "newick",
            type: "rectangular"
        }, false);

        if (this.mode != "preview") {
            this.treeSeries[ind].register_callback({
                obj: this,
                func: function(param, content, page_request) {
                    function processSequenceIds(node) {
                        if (!node)
                            return;
                        if (node.leafnode) {
                            var controller = this;
                            this.phylogram[ind].registerLeafCheckedCallback(node.name, function(checked) {
                                controller.mimicLeafChecked(node.name, checked, "phylogram");
                            });
                        }
                        for (var i=0; i<node.children.length; i++)
                            processSequenceIds.call(this, node.children[i]);
                    }
                    processSequenceIds.call(this, this.treeSeries[ind].tree.root);
                }
            },
            "series_loaded");
        }

        return this.treeSeries[ind];
    };

    this.mimicLeafChecked = function(nodeId, checked, source, param)
    {
        return;
    };


    this.onResetCloneDS = function () {
        this.getDS('clone_abs').reload(this.defCloneAbsUrl + "&objs="+this.loadedID, false);
        this.getDS('export').reload(this.defCloneExportUrl+ "&objs="+this.loadedID, false);
        var annotUrl = this.defAnnotationUrl + "&objs="+this.loadedID;
        
        annotUrl = urlExchangeParameter(annotUrl,"mySubID",this.referenceID?this.referenceID:"-");
        annotUrl = urlExchangeParameter(annotUrl,"ionObjs",this.annotObjID ?this.annotObjID :"-");
        annotUrl = urlExchangeParameter(annotUrl,"multipleAlObjID",this.mutAlID?this.mutAlID:"-");
        
        this.getDS('annotationData').reload(annotUrl,false);
        this.resetSummaryDS();
        vjDV[this.dvinfo].load();
        vjDV[this.dvname].load();
    };

    this.resetSummaryDS = function() {
        return this.getDS('summary').reload("http:
    };

    this.makeProfileOperationURL = function (viewer, node, oper) {

        var url = "?cmd=objFile&ids=" + this.loadedID + "&filename=SNPprofile-" + this.referenceID + ".csv";

        return url;
    };

    
    this.initProc = function (algoProc, loadedID, parent_loaded_ID) {
       if(algoProc) this.algoProc = algoProc;
        if(loadedID) this.loadedID = loadedID;

        this.dna_aligner_ID = parent_loaded_ID;

        var url_export = this.getDS('export').url;
        if(this.algoProc) {
            this.fastaNameTmpl = escape("$_(v)|object "+this.loadedID+"|"+this.algoProc.getValue("name"));
            url_export = urlExchangeParameter(url_export,"fasta_tmplt", this.fastaNameTmpl);
            this.getDS('clone_abs').title = "o"+this.loadedID+"-Sankey-"+this.algoProc.getValue("name");

            this.mutAlID = this.algoProc.viewer.getElement("mutualAligmentID");
            if(this.mutAlID && this.mutAlID.value) {
                this.mutAlID = this.mutAlID.value;
            }
            
            var annotUrl = this.getDS('annotationData').url;
            annotUrl = urlExchangeParameter(annotUrl, 'mySubID', this.referenceID?this.referenceID:'-');
            this.getDS('annotationData').reload( urlExchangeParameter( annotUrl, 'multipleAlObjID', this.mutAlID?this.mutAlID:'-'),false);
        }

        this.getDS('export').reload(url_export, false);
    };
    
    this.onPerformSummaryOperation = function (viewer, node, ir, ic) {
        var url;
        if (node.blob)
            url = "?cmd=objFile&ids=" + this.loadedID + "&filename=" + node.blob;
        else {
            url = "?cmd=" + node.operation + "&objs=" + this.loadedID ;
        }
        if(node.args){
            url += "&" + node.args;
        }
        this.fastaNameTmpl = escape("$_(v)|object "+this.loadedID+"|"+this.algoProc.getValue("name"));
        url = urlExchangeParameter(url,"fasta_tmplt", this.fastaNameTmpl);
        if(url){
            document.location = url;
        }
    };
    
    this.onPerformPopulationOperation = function (viewer, node, ir, ic) {
        
        var url = "?cmd=" + node.operation + "&objs=" + this.loadedID ;
        var res = true;
        if(!this.cloneIDs.length) {
            if(node.operation != 'popPermutations'){
                res = confirm("You have no clones selected.\nDo you want to download all?");
            }
            else {
                alertI("You have no clones selected.\nPlease select at least one clone to download its permutations");
                res = false;
            }
        }
        if( !res ) {
            return ;
        }
        if(this.cloneIDs.length) {
            url = urlExchangeParameter( url,"cloneIDs",this.cloneIDs.join(",") );
        }

        if(node.args){
            url += "&" + node.args;
        }

        this.fastaNameTmpl = escape("$_(v)|object "+this.loadedID+"|"+this.algoProc.getValue("name"));
        url = urlExchangeParameter(url,"fasta_tmplt",this.fastaNameTmpl );
        if(url){
            document.location = url;
        }
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};



