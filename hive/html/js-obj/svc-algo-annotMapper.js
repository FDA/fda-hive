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
javaScriptEngine.include("js/vjAnnotationView.js");

javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");

javaScriptEngine.include("d3js/annotation_box.js");

vjHO.register('svc-algo-annotMapper').Constructor=function ()
{

    if(this.objCls)return;         //stupid chrome loads from both cached file and the one coming from server.

    // two public functions which must be supported
    this.fullview=function(node,dv)
    {
        this.isIon = node.isIon;
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

    this.typeName="svc-algo-annotMapper";

    this.defaultFormat="posID|seqID|annotRange";

    this.urlStack = new Array();

    this.mimicCheckmarkSelected=1;

    this.objCls="obj-svc-algo-annotMapper"+Math.random();
    vjObj.register(this.objCls,this);
 
    this.urlSet ={
            'crossmap': {
                active_url:"qpbg_tblqryx4://crossingRanges.csv//cnt=100",
                title: "Retrieving cross-ranges",
            },
            'annotations' : {
                loadInactive:true,
                isSeries:true,
                active_url:"http://?cmd=anotDumper&annot_format="+this.defaultFormat,
                title: "Retrieving annotations"
            },
            'annot-files' : {
                //active_url:"http://?cmd=anotFiles",
                active_url:'http://?cmd=objList&type=u-ionAnnot&mode=csv&prop=name,size,created&start=0&cnt=10',
                title: "Finding relevant annotation files"
            },
            'annot_stat':{
                inactive_url:"static://",
                active_url:"http://?cmd=anotMapperResults&procID="+docLocValue("id")+"&anotFiles=&isIon=1&showStat=1&typeToShow=&cnt=1000&start=0",
                loadInactive:true,
                title: "Retrieving Statistics",
            },
            'help' : {
                active_url:"http://help/hlp.view.results.alignment.html",
                inactive_url:"http://help/hlp.view.results.alignment.html",
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
            this.dvmap=dv.name;
            dv.addTab("Annotations","graph",[this.viewers['map'],this.viewers['annotations']]).columns=2;
            dv.render(true);
            dv.load('rerender');
        }
        else{

            this.dvmap=this.current_dvORtab[0].name;

            this.current_dvORtab[0].addTab("map","graph",[this.viewers['map_panel'],this.viewers['map']]);
            this.current_dvORtab[0].addTab("help","help",[this.viewers['help']]);
            this.current_dvORtab[1].addTab("annotations","table",[this.viewers['hits_panel'],this.viewers['hits']]);
            if (this.isIon)
                this.current_dvORtab[1].addTab("statistics","graph",[this.viewers['hits_panel_graph'],this.viewers['hits_graph'], this.viewers['hits_graph_table']]);

            this.current_dvORtab[0].render();
            this.current_dvORtab[1].render();
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

            this.dvmap=this.current_dvORtab.obj.name;

            this.addviewer('map_panel,map', new vjAnnotationMapControl({
                data:'crossmap',
                formName:this.formName,
                width:'20%',
                isok:true}));


            this.addviewer('hits_panel,hits', new vjAnnotationHitsControl({
                data: ['annotations'],
                width:'80%',
                height:240,
                formName:this.formName,
                isok:true}));

        }
        else{
            vjDS.add("loading ... ","dsAnnotFiles","http://?cmd=anotFiles");
            // vjDS["dsAnnotFiles"].reload("http://?cmd=anotFiles",true);
            vjDS["dsAnnotFiles"].reload("http://?cmd=objList&type=u-annot,u-ionAnnot&mode=csv&prop=name,size,created&start=0&cnt=10",true);  // 'http://?cmd=objList&type=u-ionAnnot&mode=csv&prop=name,size,created&start=0&cnt=10'
            
            this.formName=[''];

            var formNode=gAncestorByTag(gObject(dvORtab.obj[0].name),"form");
            if(formNode)
                this.formName[0]=formNode.attributes['name'].value;

            this.current_dvORtab=dvORtab.obj;

            this.dvmap=this.current_dvORtab[0].name;
            this.dvannot=this.current_dvORtab[1].name;
            
            var preload_input = {"annot":""};
            if (algoProcess) {
                preload_input["annot"] = algoProcess.getValue("annot","join");
            }

            this.addviewer('map_panel,map', new vjAnnotationMapControl({
                data:'crossmap',
                formName:this.formName,
                isok:true}));


            this.addviewer('hits_panel,hits', new vjAnnotationHitsControl({
                data: ["annotations"],
                annotFileData:"dsAnnotFiles",
                height:240,
                formName:this.formName,
                preload_input:preload_input,
                isok:true
            }));
            
            if (this.isIon) {
                this.addviewer('hits_panel_graph,hits_graph,hits_graph_table', new vjAnnotationHitsGraphControl({
                    data: "annot_stat",
                    //width:'80%',
                    height:240,
                    formName:this.formName,
                    preload_input:preload_input,
                    isok:true}));
            }
            this.addviewer('help', new vjHelpView ({
                data: 'help',
                formName:this.formName[0],
                isok:true}));
        }

        this.viewers['map'].selectCallback = "function:vjObjFunc('onClickRange','" + this.objCls + "')";

        var dd = vjDS["dsAnnotFiles"];
        dd.clear_callbacks();
        dd.register_callback({
                 func: function(param, data) {
                     var hitsTab = vjDV.dvAnnotationViewer.find("annotations",0);
                     hitsTab.viewers[0].render();
                 }
             });

    };


    this.onClickRange = function ( viewer, node) {
        
        var seqID = node.Reference;
        if (this.isIon) {
            var ionObjs = this.viewers.hits_panel.tree.findByName("ionObjs").value;
            if (!ionObjs)
                return ;
            var rangeToAdd = "&pos_start=" + node.start + "&pos_end=" + node.end;
            var url = "http://?cmd=ionGenBankAnnotPosMap&cnt=-1&fromComputation=0&ionObjs="+ ionObjs + "&seqID="+ seqID + rangeToAdd;
            this.getDS("annotations").reload(url,true);
            this.getDS("annotations").parser = function (ds, text) {
                var tt= new vjTable(text, 0, vjTable_propCSV);
                var toReturn = "";
                if (tt.rows.length) {
                    for (var ih=0; ih < tt.hdr.length; ++ih) {
                        if (ih) toReturn += ",";
                        toReturn += "" + tt.hdr[ih].name;
                    }
                    toReturn +=",id\n";
                    for (var ir=0; ir < tt.rows.length; ++ir) {
                        toReturn += "\"" + tt.rows[ir].cols[0] + "\"," + tt.rows[ir].cols[1] + "," + tt.rows[ir].cols[2] + ",";
                        var type = tt.rows[ir].cols[3].split(";");
                        for (var it=0; it< type.length; ++it) {
                            if (it) toReturn += "\n,,,";
                            toReturn += "\"" + type[it].split(":")[0] + "\",\"" + type[it].split(":")[1] + "\"";  
                        }
                        toReturn += "\n";
                    }
                }
                return toReturn.length ? toReturn : text;
            }
        }
        else {
            var range = node.start+(node.end?("-"+node.end):"");
            var url = this.urlSet['annotations'].active_url;
    
            url = urlExchangeParameter(url, "seqID",seqID);
            url = urlExchangeParameter(url, "query_ranges",range);
            var preSetAnotIdToMap = "";
            if (vjDV[this.dvannot].tabs && vjDV[this.dvannot].tabs[0].viewers[0]){
                var panel = vjDV[this.dvannot].tabs[0].viewers[0];
                if (panel.tree.findByName("ionObjs") !== undefined){
                    if (panel.tree.findByName("ionObjs").value !== undefined){
                        if (compareToStringArray(panel.tree.findByName("ionObjs").value, ["","0","none","All"],"or") == false)
                            preSetAnotIdToMap = panel.tree.findByName("ionObjs").value ;
                    }
                }
            }
            if (preSetAnotIdToMap !== ""){
                url = urlExchangeParameter(url, "ionObjs",preSetAnotIdToMap);
            }
            this.getDS("annotations").reload(url,true);
        }
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};



