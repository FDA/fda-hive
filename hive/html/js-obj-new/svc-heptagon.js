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

vjHO.register('svc-heptagon1').Constructor=function ()
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
    
    this.typeName="svc-profiler";
    this.referenceID = 1;
    this.graphResolutionZoom = 200;
    this.graphResolution = 200;
    if(document.all) {
        this.graphResolution/=2;
        this.graphResolutionZoom/=2;
    }
    customizeOptions = new Array();
    var SNPprofileRangeFile = "SNPprofile.csv";
    this.windowZoomStack = new Array({windwStart:0,windwEnd:0});
    this.urlStack = new Array();
    this.loadedID = docLocValue ("id");
    this.profileTqs=[
                     {
                          op:"load-SNPprofile",
                          arg:{obj:this.loadedID, sub:this.referenceID, thumb:true}
                     }
    ];
    
    this.noiseTqs = [
                     {
                         op:"slice",
                         arg:{
                             start:{ col:0, value:this.referenceID, method:"firstEquals" },
                             end:{ col:0, value:this.referenceID, method:"lastEquals" }
                         }
                     }
                 ];
    this.seqalign_SNPprofile_cmd = "cols=1-3,8-19,22-27&hdr=1&minmaxCols=3-15&minmaxMainCol=5&abscissaCol=0&resolution=200&tqs="+JSON.stringify(this.profileTqs);
    this.seqalign_SNPprofile_cmd_zoom = "cols=1-3,8-19,22-27&hdr=1";
    this.seqalign_SNPprofile_cmdX = "cols=1-13&hdr=1&minmaxCols=1-12&resolution=200";
    this.seqalign_SNPprofile_cmdX_zoom = "cols=1-13&hdr=1";
    this.seqalign_SNPprofile_cmd_subrange_dl = "hdr=1";
    
    
    this.autoClickedReference=1;
    this.objCls="obj-svc-profiler"+Math.random();
    vjObj.register(this.objCls,this);
    
    vjDS.add("Building profile diagram", 'zoom_profile', this.seqalign_SNPprofile_cmd_zoom);
    vjDS.add("infrastructure: Creating profilerX", 'zoom_profileX', "static:
    vjDS.add("Retrieving open reading frames", 'zoom_orf', "http:
    vjDS.add("Retrieving snps calls", 'zoom_snp_calls_graph', "http:
    vjDS.add("Composing full summary", 'full_summary', "http:
    vjDS.add("Composing contigs", 'contigs', "http:
    vjDS.add("Finding relevant annotation files", 'annot_files', "http:
    vjDS.add("Building profile diagram", 'profile', this.seqalign_SNPprofile_cmd);
    vjDS.add("infrastructure: Creating profilerX", 'profileX', this.seqalign_SNPprofile_cmdX);
    vjDS.add("Building SNP profile for position range", 'profile_subrange_dl', this.seqalign_SNPprofile_cmd_subrange_dl);
    vjDS.add("infrastructure: Computing frequency histogram", 'freq_histogram', "cols=1-13&hdr=1");
    vjDS.add("infrastructure: Computing coverage frequency histogram", 'freq_histogram_popup', "http:
    vjDS.add("infrastructure: Computing coverage frequency histogram integral", 'freq_histogram_integral', "http:
    vjDS.add("Building consensus", 'consensus', "http:
    vjDS.add("Calling snps", 'snp_calls', "http:
    vjDS.add("Calling non synonymous snps", 'aa_calls', "qpbg_tblqryx4:
    vjDS.add("Ploting snp calls", 'snp_calls_graph', "http:
    vjDS.add("Ploting snp calls (zoom)", 'snp_calls_graph_zoom', "http:
    vjDS.add("Displaying open reading frames (zoom)", 'orf_zoom', "http:
    vjDS.add("Displaying open reading frames", 'orf', "http:
    vjDS.add("infrastructure: creating list of downloadables", 'downloads', "static:
    vjDS.add("infrastructure: building help page", 'help', "http:

    profilerZoomGraphs = {};
    
    this.dataSourcesUnique = [ "profile", "summary", "contigs", "consensus", "snp_calls", "aa_calls", "annot_files", "profileX", "noise", "orf", "noise_integral", "freq_histogram", "freq_histogram_integral", "snp_calls_graph", "downloads"];
    this.tabsToInfoMap = {};
    this.currentProfGraph;
    
    this.fullview=function(node,dv)
    {
        var parentProc = node.parentProc;
        
        vjDS.add("Retrieving list of hits", "hitlist", "http:
        vjDS.add("Prepering alignments", 'zoom_alView', "http:
        vjDS.add("Visualizing alignments in stack", 'zoom_alStack', "http:
        vjDS.add("Creating mutation bias diagram", 'zoom_alMutBias', "http:
        vjDS.add("Composing summary", 'summary', "http:
        vjDS.add("Creating noise profiles", 'noise', "qpbg_tblqryx4:
        vjDS.add("infrastructure: Computing noise integrals", 'noise_integral', "http:
        vjDS.add("Prepering annotation viewer", 'annotationData', "static:
        
        vjDS['annotationData'].returnCallbackErrorRequest = true;
        vjDS['annotationData'].refreshOnEmpty = true;
        vjDS['annotationData'].register_callback( function(_, data, request){ 
            if((!data || data === "" || request.status >= 400)){ this.data = null } 
        })

        vjDS["profile"].url="qpbg_tblqryx4:
        vjDS['contigs'].url=urlExchangeParameter(urlExchangeParameter(vjDS['contigs'].url, "objs",this.loadedID),"idSub",this.referenceID);
        vjDS['consensus'].url=urlExchangeParameter(urlExchangeParameter(vjDS['consensus'].url, "objs",this.loadedID),"idSub",this.referenceID);
        vjDS['snp_calls'].url=urlExchangeParameter(urlExchangeParameter(vjDS['snp_calls'].url, "objs",this.loadedID),"idSub",this.referenceID);
        vjDS['aa_calls'].url="qpbg_tblqryx4:
        vjDS['profileX'].url="qpbg_tblqryx4:
        vjDS['freq_histogram'].url=urlExchangeParameter(urlExchangeParameter(vjDS['freq_histogram'].url, "ids",this.loadedID),"filename","FreqProfile.csv");
        vjDS['freq_histogram_integral'].url=urlExchangeParameter(urlExchangeParameter(vjDS['freq_histogram_integral'].url, "ids",this.loadedID),"filename","FreqProfileIntegral.csv");
        vjDS['snp_calls_graph'].url=urlExchangeParameter(urlExchangeParameter(vjDS['snp_calls_graph'].url, "objs",this.loadedID),"subID",this.referenceID);
        vjDS['orf'].url = urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(vjDS['orf'].url, "objs",this.loadedID),"subID",this.referenceID), "searchField", "seqID");
        

        var profzoomVIS=new Object();
        profzoomVIS.obj=vjVIS;
        
        var profilerGraphs = new vjProfilerSNPProfileControl({
            data: {'profile':'profile', 'profileX': 'profileX', 'annotationData' : 'annotationData'},
            width:800,
            selectCallback: "function:vjObjFunc('onZoomProfileOperation','" + this.objCls + "')",
            anotSelectCallback: null, 
            divName: "profileGraphs-tab",
            formName:formName,
            isok:true
        });

        profilerGraphs[0].callerObject = { onClickMenuNode: this.profilerClickFunction};
        
        this.annotControl = new vjProfilerAnnotationListControlView({
            data: 'annot_files',
            formName:formName,
            callbackForPanel:"function:vjObjFunc('onRenderIonAnnotList','" + this.objCls + "')",
            isok:true
        });
        
        profilerZoomGraphs = new vjProfilerZoomSNPProfileControl({
            data:  {'profile':'zoom_profile', 'profileX': 'zoom_profileX', 'annotationData' : 'annotationData'},
            width:0.95*gPgW,
            urlrefresh:"javascript:var res=vjObjFunc('zoomProfileOnPosition','" + this.objCls + "'); res.func.call(res.obj,'$(::objcls.parent.children[0].value)');",
            selectCallback: "function:vjObjFunc('onZoomFromZoom','" + this.objCls + "')",
            divName: "profileZoom-tab",
            formName: modalFormName,
            isok:true
        });
        var aaTableController1 = new vjTableControlX2({
            data : "aa_calls",
            formObject : document.forms[formName]
        });
        
        var filesStructureToAdd = [{
            tabId: 'summary',
            tabName: "Summary",
            position: {posId: 'heptSummary', top:'30%', bottom:'100%', left:'20%', right:'50%'},
            inactive: false,
            viewerConstructor: {
                dataViewer: "vjProfilerSummaryView",
                dataViewerOptions:{
                    data: 'summary',
                    width:'80%',
                    formName: formName,
                    isok : true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'downloadsHept',
            tabName: "Downloads",
            position: {posId: 'heptSummary', top:'30%', bottom:'100%', left:'20%', right:'50%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjProfilerDownloadsView",
                dataViewerOptions:{
                    data: 'downloads',
                    width:800,
                    formName:formName,
                    selectCallback: "function:vjObjFunc('onPerformProfileOperation','" + this.objCls + "')",
                    isok:true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'contigs',
            tabName: "Contigs",
            position: {posId: 'heptSummary', top:'30%', bottom:'100%', left:'20%', right:'50%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjProfilerContigControl",
                dataViewerOptions:{
                    data: 'contigs',
                    formName:formName,
                    isok:true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'annot_files',
            tabName: "Annotation Files",
            position: {posId: 'heptSummary', top:'30%', bottom:'100%', left:'20%', right:'50%'},
            inactive: true,
            viewerConstructor: {
                instance:this.annotControl
            },
              autoOpen: "computed"
        },{
            tabId: 'profileGraphs',
            tabName: "Profile Graphs",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: false,
            viewerConstructor: {
                instance: profilerGraphs
            },
              autoOpen: "computed"
        },{
            tabId: 'help',
            tabName: "Help",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjHelpView",
                dataViewerOptions:{
                    data: "help",
                    formName:formName,
                    isok:true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'noiseGraphs',
            tabName: "Sequencing Noise",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjProfilerNoiseControl",
                dataViewerOptions:{
                    data: {'noise':'noise','integral':'noise_integral'},
                    formName:formName,
                    isok:true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'freq_histogram',
            tabName: "Frequency Histogram",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjProfilerFrequencyHistogramControl",
                dataViewerOptions:{
                    data: {'histogram':'freq_histogram','integral':'freq_histogram_integral'},
                    formName:formName,
                    isok:true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'consensus',
            tabName: "Consensus",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjProfilerConsensusControl",
                dataViewerOptions:{
                    data: 'consensus',
                    formName:formName,
                    isok:true
                }
            },
              autoOpen: "computed"
        },{
            tabId: 'snp_calls',
            tabName: "SNP Calls",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                instance: new vjProfilerSNPcallsControl({
                    data:'snp_calls',
                    annotFileData: "annot_files",
                    formName:formName,
                    isok:true
                })
            },
              autoOpen: "computed"
        },{
            tabId: 'aa_calls',
            tabName: "Phased Mutations",
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                instance: aaTableController1.arrayPanels
            },
              autoOpen: "computed"
        }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.moveTab("general", {top:"0%", left: "20%", right: "100%", bottom: "40%"}, 0);
        algoWidgetObj.closeTab("basicHelp");
        
        for (var i = 0; i < profilerGraphs.length; i++){
            if (profilerGraphs[i].hidden)
                $("#" + profilerGraphs[i].container).addClass("hidden");
        }
        
        algoWidgetObj.moveTab("hitList", {top:"0%", left: "20%", right: "60%", bottom: "30%"}, 0);
        algoWidgetObj.moveTab("alignments", {top:'0%', bottom:'30%', left:'60%', right:'75%'}, 0);
        
        var dsMapping={
                profile: "profile",
                summary: "summary",
                contigs: "contigs",
                consensus: "consensus",
                snp_calls: "snp_calls",
                profileX: "profileX",
                noise: "noise",
                noise_integral: "noise_integral",
                freq_histogram: "freq_histogram",
                freq_histogram_integral: "freq_histogram_integral",
                snp_calls_graph: "snp_calls_graph",
                downloads: "downloads",
                orf: "orf",
                annot_files: "annot_files"
        };
        this.tabsToInfoMap["profileGraphs"] = {};
        this.tabsToInfoMap["profileGraphs"].profilerGraphs = profilerGraphs;
        this.tabsToInfoMap["profileGraphs"].dataSources = dsMapping;
        this.tabsToInfoMap["profileGraphs"].active = true;
        this.tabsToInfoMap.totalElements = 1;
        this.currentProfGraph = "profileGraphs";
    };
    
    
    if( vjDV[algoProcess.recViewerName].file  && vjDV[algoProcess.recViewerName].file["ProfileInfo.csv"] ) {
        this.isProfileInfo = true;
    }
    if( vjDV[algoProcess.recViewerName].file  && vjDV[algoProcess.recViewerName].file["SNPthumb.csv"] ) {
        this.isSNPThumb = true;
        if( vjDV[algoProcess.recViewerName].file["ThumbInfo.csv"] ) {
            this.isThumbInfo = true;
        }
    }
    if( vjDV[algoProcess.recViewerName].file  && vjDV[algoProcess.recViewerName].file["HistProfile.csv"] ) {
        this.isHistProfile = true;
    }
   

    this.onSelectReferenceID = function ( viewer, node) {
        var currentMapping = this.tabsToInfoMap[this.currentProfGraph];
        var curDS = currentMapping.dataSources;
        this.referenceID = node.id;
        var reference = node.Reference;
        
        this.colorId = node.irow;
        var arrCol=new Array(vjGenomeColors[this.colorId% gClrTable.length], shadeColor(vjGenomeColors[this.colorId % gClrTable.length], 0.5));
        

        for (let i = 0; i < currentMapping.profilerGraphs.length; i++){
            let itemInTab = currentMapping.profilerGraphs[i]
            if(itemInTab.options && (itemInTab.name == "coverage" || itemInTab.name == "unaligned_tails")){
                itemInTab.options.colors = arrCol;
            }
            if(itemInTab.name === "menu" && itemInTab.rows){
                for(let j = 0; j < itemInTab.rows.length; j++){
                    if(itemInTab.rows[j].name === "nameOfReference"){
                        itemInTab.rows[j].title = "Reference: " + reference;
                        j = itemInTab.rows.length
                    }
                }
            }
        }

        this.profileTqs=[
                  {
                      op:"load-SNPprofile",
                      arg:{obj:this.loadedID, sub:this.referenceID, thumb:true}
                  }
        ]; 
        this.seqalign_SNPprofile_cmd = "cols=1-3,8-19,22-27&hdr=1&minmaxCols=3-15&minmaxMainCol=5&abscissaCol=0&resolution="+this.graphResolution+"&tqs="+JSON.stringify(this.profileTqs);
        this.noiseTqs = [
                        {
                            op:"slice",
                            arg:{
                                start:{ col:0, value: parseInt(this.referenceID), method:"firstEquals" },
                                end:{ col:0, value: parseInt(this.referenceID), method:"lastEquals" }
                            }
                        }
                    ];
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
        
        var urlSum=urlExchangeParameter(urlExchangeParameter(vjDS[curDS['summary']].url, "objs",this.loadedID),"idSub",this.referenceID);
        urlSum=urlExchangeParameter(urlSum, "gapThreshold",parseInt(algoProcess.getValue("minCover"))-1);
        
        vjDS[curDS["profile"]].reload(urlExchangeParameter(vjDS[curDS["profile"]].url, "tqs",JSON.stringify(this.profileTqs)), true);
        vjDS[curDS['summary']].reload(urlSum, true);
        
        if (vjDS[curDS['contigs']].state != "") 
            vjDS[curDS['contigs']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['contigs']].url, "objs",this.loadedID),"idSub",this.referenceID), true);
        else
            vjDS[curDS['contigs']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['contigs']].url, "objs",this.loadedID),"idSub",this.referenceID));
        if (vjDS[curDS['consensus']].state != "") 
            vjDS[curDS['consensus']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['consensus']].url, "objs",this.loadedID),"idSub",this.referenceID), true);
        else
            vjDS[curDS['consensus']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['consensus']].url, "objs",this.loadedID),"idSub",this.referenceID));
        if (vjDS[curDS['snp_calls']].state != "") 
            vjDS[curDS['snp_calls']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['snp_calls']].url, "objs",this.loadedID),"idSub",this.referenceID), true);
        else
            vjDS[curDS['snp_calls']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['snp_calls']].url, "objs",this.loadedID),"idSub",this.referenceID));
        
        var tt = urlExchangeParameter(urlExchangeParameter (vjDS[curDS["annot_files"]], "objs",this.loadedID),"subID",this.referenceID);
        tt = urlExchangeParameter(tt,"idToSearch",node.Reference);
        tt = urlExchangeParameter(tt,"idTypeToSearch","seqID");
        
        if (this.isProfileInfo) {
            var url = "qpbg_tblqryx4:
            if (vjDS[curDS['profileX']].state != "")
                vjDS[curDS['profileX']].reload(url, true);
            else
                vjDS[curDS['profileX']].reload(url);
        }
        
        if (vjDS[curDS['noise']].state != "") 
            vjDS[curDS['noise']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['noise']].url, "objs",this.loadedID),"tqs", JSON.stringify(this.noiseTqs)), true);
        else
            vjDS[curDS['noise']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['noise']].url, "objs",this.loadedID),"tqs", JSON.stringify(this.noiseTqs)));
        if (vjDS[curDS['noise_integral']] != "") 
            vjDS[curDS['noise_integral']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['noise_integral']].url, "objs",this.loadedID),"idSub",this.referenceID), true);
        else
            vjDS[curDS['noise_integral']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['noise_integral']].url, "objs",this.loadedID),"idSub",this.referenceID));
        if (vjDS[curDS['freq_histogram']].state != "") 
            vjDS[curDS['freq_histogram']].reload("qpbg_tblqryx4:
        else
            vjDS[curDS['freq_histogram']].reload("qpbg_tblqryx4:
        if (vjDS[curDS['freq_histogram_integral']] != "") 
            vjDS[curDS['freq_histogram_integral']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['freq_histogram_integral']].url, "objs",this.loadedID),"idSub",this.referenceID), true);
        else
            vjDS[curDS['freq_histogram_integral']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['freq_histogram_integral']].url, "objs",this.loadedID),"idSub",this.referenceID));
        if (vjDS[curDS['snp_calls_graph']].state != "") 
            vjDS[curDS['snp_calls_graph']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['snp_calls_graph']].url, "objs",this.loadedID),"subID",this.referenceID), true);
        else
            vjDS[curDS['snp_calls_graph']].reload(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['snp_calls_graph']].url, "objs",this.loadedID),"subID",this.referenceID));
       
        var srchFd = "seqID";
        if (node.Reference.indexOf("gi|")==0) srchFd = "gi";
        if (vjDS[curDS['orf']].state != "") vjDS[curDS['orf']].reload(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(vjDS[curDS['orf']].url, "objs",this.loadedID),"subID",this.referenceID), "searchField", srchFd), "search", node.Reference), true);
        
        var subrangeDownloads = [
             { title: "SNP profile", blob: "SNPprofile.csv", dsname: "profile_subrange_dl", onClickFunc: this.onPerformProfileOperation, download: true},
             { title: "SNP profile in VCF format", operation: "profVCF" , onClickFunc: this.onPerformProfileOperation, download: true},
             { title: "SNP profile in VCF format (collapsed deletions)", operation: "profVCF&collapseConsecutiveDeletions=1" , onClickFunc: this.onPerformProfileOperation, download: true},
             { title: "Annotation Data", dsname: 'annotationData' ,operation:"ionGenBankAnnotPosMap" , download: true}
         ];
        
        currentMapping.profilerGraphs[0].setDownloads(subrangeDownloads);
    
        var t = "data,down,arch,operation,blob\n";
        for (var i=0; i<subrangeDownloads.length; i++) {
            var d = subrangeDownloads[i];
            if (d.title) t += d.title;
            t += ",download,upload,";
            if (d.operation) t += d.operation;
            t += ",";
            if (d.blob) t+= d.blob;
            t += "\n";
        }
        t +="SNP profile of all references in VCF format,download,upload,profVCFAll\n" +
                "SNP summary on selected reference,download,upload,profSNPcalls\n" +
                "SNP summary of all references,download,upload,profAllSNPcalls\n" +
                "AA-mutation summary of all references,download,upload,,AAprofile.csv\n" +
                "Noise profile,download,upload,,Noise.csv\n" +
                "Noise cuttoffs,download,upload,profNoiseIntegral\n" +
                "Extended profile information,download,upload,,ProfileInfo.csv\n" +
                "Frequency histogram,download,upload,,FreqProfile.csv\n" +
                "Frequency histogram integrals,download,upload,profFreqIntegral\n" +
                "Gaps and Contigs of all References,download,upload,profContigAll\n" +
                "CSV Summary of all References,download,upload,profSummaryAll\n" +
                "Consensus with gaps,download,dna,profConsensus,\n" +
                "Consensus with gaps replaced by reference,download,dna,profConsensus&gaps=fill,\n" +
                "Consensus where gaps are skipped,download,dna,profConsensus&gaps=skip,\n" +
                "Consensus split on gaps,download,dna,profConsensus&gaps=split,";
        vjDS[curDS['downloads']].reload("static:
    };
    
    this.onDblClickReference = function (viewer, node){
        

        var totalElements = this.tabsToInfoMap.totalElements;
        var profileGraphName = "profileGraphs" + totalElements;
        var profileGraphTitle = "Profile Graphs " + totalElements;
        
        var newDSArr={
                profile: "profile" + totalElements,
                summary: "summary" + totalElements,
                contigs: "contigs" + totalElements,
                consensus: "consensus" + totalElements,
                snp_calls: "snp_calls" + totalElements,
                profileX: "profileX" + totalElements,
                noise: "noise" + totalElements,
                noise_integral: "noise_integral" + totalElements,
                freq_histogram: "freq_histogram" + totalElements,
                freq_histogram_integral: "freq_histogram_integral" + totalElements,
                snp_calls_graph: "snp_calls_graph" + totalElements,
                downloads: "downloads" + totalElements,
                orf: "orf" + totalElements,
                annot_files: "annot_files" + totalElements
        };
        vjDS.add("", newDSArr.profile, vjDS["profile"].url);
        vjDS.add("", newDSArr.summary, vjDS["summary"].url);
        vjDS.add("", newDSArr.contigs, vjDS["contigs"].url);
        vjDS.add("", newDSArr.consensus, vjDS["consensus"].url);
        vjDS.add("", newDSArr.snp_calls, vjDS["snp_calls"].url);
        vjDS.add("", newDSArr.profileX, vjDS["profileX"].url);
        vjDS.add("", newDSArr.noise, vjDS["noise"].url);
        vjDS.add("", newDSArr.noise_integral, vjDS["noise_integral"].url);
        vjDS.add("", newDSArr.freq_histogram, vjDS["freq_histogram"].url);
        vjDS.add("", newDSArr.freq_histogram_integral, vjDS["freq_histogram_integral"].url);
        vjDS.add("", newDSArr.snp_calls_graph, vjDS["snp_calls_graph"].url);
        vjDS.add("", newDSArr.downloads, vjDS["downloads"].url);
        vjDS.add("", newDSArr.orf, vjDS["orf"].url);
        vjDS.add("", newDSArr.annot_files, vjDS["annot_files"].url);
        
        var tmpProfilerGraphs = new vjProfilerSNPProfileControl({
            data: {'profile': newDSArr.profile, 'profileX': newDSArr.profileX, 'annotationData' : 'annotationData'},
            width:800,
            selectCallback: "function:vjObjFunc('onZoomProfileOperation','" + this.objCls + "')",
            divName: "profileGraphs-tab",
            formName:formName,
            isok:true
        });

        tmpProfilerGraphs[0].callerObject = { onClickMenuNode: this.profilerClickFunction};
        
        var tabsToAdd = [{
            tabId: profileGraphName,
            tabName: profileGraphTitle,
            position: {posId: 'heptMain', top:'30%', bottom:'100%', left:'50%', right:'75%'},
            inactive: false,
            viewerConstructor: {
                instance: tmpProfilerGraphs
            },
              autoOpen: "computed"
        }];
        
        algoWidgetObj.addTabs(tabsToAdd, "results");
        
        for (var i = 0; i < tmpProfilerGraphs.length; i++){
            if (tmpProfilerGraphs[i].hidden)
                $("#" + tmpProfilerGraphs[i].container).addClass("hidden");
        }
                
        this.tabsToInfoMap[profileGraphName] = {};
        this.tabsToInfoMap[profileGraphName].profilerGraphs = tmpProfilerGraphs;
        this.tabsToInfoMap[profileGraphName].dataSources = newDSArr;
        this.tabsToInfoMap[profileGraphName].active = true;
        this.tabsToInfoMap.totalElements ++;
        this.currentProfGraph = profileGraphName;
        
        viewer.onClickCell(viewer, node.irow, 0);
    };
    
    $(document).on("tab-active", function (event, areaInfo) {
        if (dna_profilerProfile && areaInfo && areaInfo.name && areaInfo.name.indexOf("profileGraphs") > -1)
            dna_profilerProfile.currentProfGraph = areaInfo.name;
    });
    let that = this
    this.profilerClickFunction = function(container, node, menuViewer) {
        if (node.onClickFunc) {
            node.onClickFunc(container, node, menuViewer);
            return;
        }else if(node.makeUrlParam && node.makeUrlParam.onClickFunc){
            node.makeUrlParam.onClickFunc(menuViewer, node.makeUrlParam)
            return;
        }
         
         var url = that.makeProfileOperationURL(menuViewer, node.makeUrlParam, node.makeUrlParam.operation);

         var ds = node.makeUrlParam.dsname ? vjDS[node.makeUrlParam.dsname] : null;
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
    };

    this.makeProfileOperationURL = function (viewer, node, oper, isArch)
    {
        var url;
        if (node.blob == "SNPprofile.csv") {
            if(isArch) {
                url = "qpbg_tblqryx4:
            } else {
                url = "qpbg_tblqryx4:
            }
            url +="
        } else if (node.blob){
            url = "qpbg_tblqryx4:
            if(isArch) url +="::arch-req.txt";
            else url+=":::-o"+this.loadedID+(this.referenceID!==undefined?("-reference"+this.referenceID):"")+"-"+node.blob
            url +="
        }
        else {
            url = "?cmd=" + node.operation + "&objs=" + this.loadedID + "&idSub=" + this.referenceID;
            if (node.operation == 'profNoise')
                url += "&noiseProfileMax=1";
            else if(node.operation=="profSNPcalls"){
                url = vjDS['snp_calls'].url.substring(7);
                url = urlExchangeParameter(url, "cnt", '-1');
                url = urlExchangeParameter(url, "start", '0');
            }
            else if(node.operation=="profAllSNPcalls"){
                url = vjDS['snp_calls'].url.substring(7);
                url = urlExchangeParameter(url, "idSub", '0');
                url = urlExchangeParameter(url, "cnt", '-1');
                url = urlExchangeParameter(url, "start", '0');
            }
            else if(node.operation=="profVCFAll"){
                url = vjDS['snp_calls'].url.substring(7);
                url = urlExchangeParameter(url, "cmd", 'profVCF');
                url = urlExchangeParameter(url, "idSub", '-1');
            }
            else if(node.operation=="profVCF"){
                url = vjDS['snp_calls'].url.substring(7);
                url = urlExchangeParameter(url, "cmd", 'profVCF');
                url = urlExchangeParameter(url, "start", '0');
                url = urlExchangeParameter(url, "cnt", '0');
            } 
            else if(node.operation == "profSummaryAll") {
                url = urlExchangeParameter(url, "cmd", 'profSummary');
                url = urlExchangeParameter(url, "idSub", '0');
            }
            else if(node.operation == "profContigAll") {
                url = urlExchangeParameter(url, "cmd", 'profContig');
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

    this.onPerformProfileOperation = (viewer, node, ir, ic) =>
    {
        var isArch = (viewer.tblArr && viewer.tblArr.hdr && viewer.tblArr.hdr[ic].name=="arch");
        var isDown = (viewer.tblArr && viewer.tblArr.hdr && viewer.tblArr.hdr[ic].name=="down");
        var url = that.makeProfileOperationURL(viewer, node, node.operation, isArch);
        if(url.indexOf("?") == 0) url = "http:
        if( isDown || node.download ){
            url = urlExchangeParameter(url, "down", '1');
            this.dsQPBG.reload(url, true, {loadmode:"download",saveas:node.blob});
        } else if (isArch) {
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
            case "ico-file":
                ext = "txt";
                dstName += "."+ext;
                break;
            default :
                ext= "-";
            }
            
            url = urlExchangeParameter(url, "arch_dstname", dstName);
            url = urlExchangeParameter(url, "ext", ext);
            if ( url.indexOf("qpbg")==0)
                this.dsQPBG_digest.reload(url,true);
            else
                this.dsQPBG_digest.reload("qpbg_http:
        }
    };
    
    this.onRenderIonAnnotList = function (viewer, node, ir, ic)
    {
        var table = this.annotControl[1];
        var idArr = [];
        
        for (var ia=0; ia < table.selectedNodes.length; ++ia){
            idArr.push(table.selectedNodes[ia]['id']);
        }
        var url = vjDS["annotationData"].url;
        if (url.indexOf("static:
            url = "http:
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
        var info = viewer.tree.findByName("announcement");
        if (typeArr.length){
            info.hidden = true;
            this.tabsToInfoMap.profileGraphs.profilerGraphs[8].hidden = false;
            $(`#${this.tabsToInfoMap.profileGraphs.profilerGraphs[8].container}`).removeClass("hidden")
            this.tabsToInfoMap.profileGraphs.profilerGraphs[0].tree.findByPath("/showGraphs").children[7].value=1
            vjDS["annotationData"].reload(url,true);   
        }else {
            info.hidden = false;
            info.title = "Please select a Type to show";
            viewer.refresh();
        }
        
    };
    
    this.onZoomFromZoom = function(viewer, node, ir, ic){
        profilerZoomGraphs.zoomFromZoom = true;
        this.onZoomProfileOperation(viewer, node, ir, ic);
    }
    
    this.onZoomProfileOperation = function (viewer, node, ir, ic) {
        if(node["Count Total"] == 0){
            $("body").append(
                    $(document.createElement("div"))
                        .attr("id", "errorDialog")
                        .attr("title", "No Coverage")
                        .append (
                                $(document.createElement("p")).text("Selected position has no coverage. Please select a non-zero position.")
                        )
                    );
                
                $("#errorDialog").dialog({
                    modal: true,
                    width: 500,
                    buttons: {
                        OK: function() {
                           $(this).dialog("close");
                        }
                    },
                    open: function() {
                        $(this).closest(".ui-dialog")
                            .find(".ui-dialog-titlebar-close")
                            .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                            .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                        $(this).closest(".ui-dialog")
                            .find(".ui-dialog-titlebar")
                            .addClass("error-header");
                    }
                });
                
            return;
        }
        
        var currentMapping = this.tabsToInfoMap[this.currentProfGraph];
        var curDS = currentMapping.dataSources;
        if (!node.Position) return;
        var Highlighted = parseInt(node.Position);
        var arrCol=new Array(vjGenomeColors[this.colorId% gClrTable.length], shadeColor(vjGenomeColors[this.colorId % gClrTable.length], 0.5));
        
        for (var i = 0; i < profilerZoomGraphs.length; i++){
            if(profilerZoomGraphs[i].options && (profilerZoomGraphs[i].name == "coverage" || profilerZoomGraphs[i].name == "unaligned_tails"))
                profilerZoomGraphs[i].options.colors = arrCol;
        }

        var menuProfileElem = currentMapping.profilerGraphs[0].tree.root.children[1];
        for (var i = 0; !profilerZoomGraphs.zoomFromZoom && i < menuProfileElem.children.length-1; i++){
            var menuElemName = menuProfileElem.children[i].name
            var j;
            for(j=i; j < profilerZoomGraphs[0].rows.length-1; j++){
                if(menuElemName + "Zoom" == profilerZoomGraphs[0].rows[j].name) break;
            }
            profilerZoomGraphs[0].rows[j].value = currentMapping.profilerGraphs[0].tree.root.children[1].children[i].value;
        }
        
        var zoomTqs = [
            { op: "load-SNPprofile", arg: { obj: this.loadedID, sub: this.referenceID}},
            { op:"predefine", arg:{name:"saved_referenceID",value:this.referenceID}},
            { op:"addMissingRows",
                arg:{
                    abscissa:{
                        col:1,
                        dense:true,
                        maxGap:1,
                        minValue:Math.max(0,Highlighted-100),
                        maxValue:{formula:"min(input_obj.refSeqLen(saved_referenceID),"+(Highlighted+100)+")"}},
                        add: [
                              { col: 0, value: this.referenceID },
                              { col: 2, value: { formula: "input_obj.refSeqLetter(saved_referenceID,cur_abscissa_val)" } },
                              { col: 3, value: { formula: "input_obj.refSeqLetter(saved_referenceID,cur_abscissa_val)" } },
                              { col: "*", value: 0 }
                        ]
                     },
                     start_end_slice:true
            }
        ];
        
        var sliceTqs = [
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
        

        var url = "qpbg_tblqryx4:
        url = urlExchangeParameter(url, "alHigh", Highlighted);
        vjDS['zoom_profile'].reload(url, true);
        
        if (this.isProfileInfo) {
            var tqsToUse = vjDS.escapeQueryLanguage(JSON.stringify(sliceTqs.concat(zoomTqs)));
            var url = "qpbg_tblqryx4:
            url = urlExchangeParameter(url, "alHigh", Highlighted);
            vjDS['zoom_profileX'].reload(url, true);
        }
        
        vjDS['zoom_alView'].reload("http:
        vjDS['zoom_alStack'].reload("http:
        vjDS['zoom_alMutBias'].reload("http:
        var config = {
                layout: {
                    items:[{
                        id: 'zoomPopup',
                        layoutType: "horizontal",
                        size: "100%",
                        tabs:{
                            items: [{
                                active: true,
                                allowResize: true,
                                title:'Profile Zoom',
                                name:'profileZoom',
                                id:'profileZoom',
                                overflow: 'auto',
                                view: {
                                    name: 'dataview',
                                    options: {
                                        instance: profilerZoomGraphs
                                    }
                                }
                            },{
                                active: false,
                                allowResize: true,
                                title: "Alignment",
                                name:"align2",
                                id:"align2",
                                overflow: "auto",
                                view: {
                                    name: "dataview",
                                    options:{
                                        dataViewer: "vjAlignmentControl",
                                        dataViewerOptions:{
                                            data: 'zoom_alView',
                                            width:0.95*gPgW,
                                            formName:modalFormName,
                                            isok:true
                                        }
                                    }
                                }
                            },{
                                active: false,
                                allowResize: true,
                                title: "Stack",
                                name:"stack2",
                                id:"stack2",
                                overflow: "auto",
                                view: {
                                    name: "dataview",
                                    options:{
                                        dataViewer: "vjAlignmentStackControl",
                                        dataViewerOptions:{
                                            data: {stack: 'zoom_alStack', basecallBias: 'zoom_alMutBias'},
                                            width:0.95*gPgW,
                                            formName:modalFormName,
                                            isok:true
                                        }
                                    }
                                }
                            }]
                        }
                    }]
                }
          };
        
        function afterZoomFunc(){
            profileTabViewers = $("#profileZoom-tab").children();
            for (var i = 1; i < profileTabViewers.length; i++){
                if(profilerZoomGraphs[0].rows[i+2].value == 1 || profilerZoomGraphs[0].rows[i+2].value == false)
                    $(profileTabViewers[i]).removeClass("hidden");
                else
                    $(profileTabViewers[i]).addClass("hidden");
            }
        };
        
        algoWidgetObj.createPopup(config, afterZoomFunc);
        profilerZoomGraphs[0].rebuildTree();
        profilerZoomGraphs[0].onClickMenuNode("", "", profilerZoomGraphs[0]);
        profilerZoomGraphs.zoomFromZoom = false;
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

        vjDS['snp_calls_graph_zoom'].reload(url, true);

        var url1 = vjDS['orf'].dataurl;
        url1 = urlExchangeParameter(url1,"start", startPosition);
        url1 = urlExchangeParameter(url1,"end", endPosition);
        vjDS['orf_zoom'].reload(url1, true);

        this.urlStack.push({snp:url,anot:url1});
        vjVIS.winop("vjVisual", this.dvzoomannot_visual, "open", true);
    };
    
    this.onZoomAnnotBox = function (node,evt)
    {
        this.windowZoomStack.push({start:node.min,end:node.max});
        var quarterLengthRange = Math.round((parseInt(node.max) - parseInt(node.min))/4);
        var position = Math.round((parseInt(node.rangeEnd)-parseInt(node.rangeStart))/2) + parseInt(node.rangeStart);
        var startPosition = position - quarterLengthRange; if (startPosition < 0 ) startPosition =0;
        var endPosition = position + quarterLengthRange;
        url = vjDS['orf'].dataurl;
        url = urlExchangeParameter(url,"start", startPosition);
        url = urlExchangeParameter(url,"end", endPosition);

        vjDS['orf_zoom'].reload(url, true);
        var resolution = endPosition - startPosition; if (resolution>200) resolution = 200;
        var url1 =  "http:
        this.urlStack.push({snp:url1,anot:url});
        vjDS['snp_calls_graph_zoom'].reload(url1, true);
        vjVIS.winop("vjVisual", this.dvzoomannot_visual, "open", true);
    };
    
    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};



