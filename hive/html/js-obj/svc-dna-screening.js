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

/*
 *  Copyright (c) 2005 Dr. Vahan Simonyan and Dr. Raja Mazumder.
 * This software is protected by U.S. Copyright Law and International
 * Treaties. Unauthorized use, duplication, reverse engineering, any
 * form of redistribution, use in part or as a whole, other than by
 * prior, express, written and signed agreement is subject to penalties.
 * If you have received this file in error, please notify copyright
 * holder and destroy this and any other copies. All rights reserved.
 */

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjTaxonomyView.js");
//javaScriptEngine.include("d3js/sunburst_hierarchy.js");

google.load("visualization", "1", {packages:["corechart"]});

vjHO.register('svc-dna-screening').Constructor=function ()
{

    if(this.objCls)return;
    this.resolution=200;


    this.fullview=function(node,dv)
    {
        this.node = node;
        this.node.cntNode = 30;
        this.mode='fullview';
        this.create(dv,node.id);

    };

    this.preview = function(node,dv)
    {
        this.node = node;
        this.node.cntNode = 150;
        this.parent.preview("svc", node, dv);
        if(!node.status || parseInt(node.status)<5)return ;

        this.mode='preview';
        this.create(dv,node.id);

    };

    this.mobileview = function(node,dv)
    {
         this.node = node;
         this.node.cntNode = 30;
         this.mode='mobileview';
         this.create(dv,node.id);
         changeTabs("List","-","-");


    };
    this.typeName="svc-dna-screening";

    this.objCls="obj-"+this.typeName+Math.random();
    vjObj.register(this.objCls,this);

    this.active_url=new Object();
    this.inactive_url=new Object();
    this.viewersToAdd=new Array();
    this.viewers=new Object();

    this.idNulldsName="default";
    this.viewers.length=0;
    this.defaultShowNode = 50;
//    if(this.params.node.showInFullView)    this.defaultShowNode = 150;

    this.urlSet ={
            'taxTreeBrower' : {
                active_url:"static://",
                isSeries:true,
                title:'Retrieving data for taxTreeBower',
                objs:"whatever"
                },
            'referenceSet' : {
                //active_url:"http://taxTree2.cgi?whatToPrint=taxid|path|allname|num|min|max|mean|stddev|intval&cnt="+this.defaultShowNode+"&cmd=ncbiTaxBrowseCurrent&rankfilters=no rank,species,phylum,family,order,genus,class,genera,kingdom&screenId=0",//no rank was deleted
                active_url:"http://dna.cgi?cnt="+this.defaultShowNode+"&cmd=ionncbiTax&screenId=0",//no rank was deleted
                isSeries:true,
                title:'Retrieving data for referenceSet',
                objs:"screenId"
                },
            'blastNTset': {
                active_url:"http://dna.cgi?cnt="+this.defaultShowNode+"&cmd=ionncbiTax&percentage=1&screenType=dna-alignx_screenResult.csv&screenId=0",
                isSeries:true,
                title:'Retrieving data for blastNTset',
                objs:"screenId"
                },
            'blastNTdownloadGI': {
                active_url:"http://dna.cgi?cnt="+this.defaultShowNode+"&cmd=ionncbiTax&screenType=dna-alignx_acclist.csv&ginumber=1&screenId=0",
                isSeries:true,
                title:'Retrieving GI data',
                objs:"screenId"
            },
            'taxDetails': {
                title:'Retrieving data for taxDetails',
                active_url:"static://"
            },
            'shannonEntropy': {
                title:'Retrieving data for Shannon Entropy graph',
                active_url:"static://"
            },
            'dsTaxonomyViewerSpec': {
                active_url: "static://type_id,name,title,type,parent,role,is_key_fg,is_readonly_fg,is_optional_fg,is_multi_fg,is_hidden_fg,is_brief_fg,is_summary_fg,order,default_value,constraint,constraint_data,description\n"
                    +"taxonomy,name_list,Names,list,,,0,1,0,0,0,0,0,,,,,\n"
                    +"taxonomy,taxName,Tax-name,string,name_list,,0,1,1,1,0,0,0,,,,,\n"
                    +"taxonomy,taxid,Taxonomy ID,integer,,,0,1,0,0,0,0,0,,,,,\n"
                    +"taxonomy,parentid,Parent,integer,,,0,1,0,0,0,0,0,,,,,\n"
                    +"taxonomy,path,Ancestry,string,,,0,1,0,0,0,0,0,,,,,\n"
                    +"taxonomy,rank,Rank,string,,,0,1,0,0,0,0,0,,,,,\n"
                    +"taxonomy,bioprojectID,BioProjectID,integer,,,0,1,0,0,0,0,0,,,,,\n",
                title:'Retrieving data for dsTaxonomyViewerSpec',
                doNotChangeMyUrl:true,
            },
            'HelpInfo': {
                doNotChangeMyUrl:true,
                title:'Retrieving data for HelpInfo',
                active_url:"http://help/hlp.page.view.taxonomy.html"
            },
           
    };

    this.create=function(dvORtab, id, geometry, formName, stickytabs){

        this.load(dvORtab,id, geometry, formName);


//        alert("second create");
    };

    this.load = function(dvORtab, id, geometry,formName)
    {
        if(this.node.whichTab!=undefined) {
            return this.realload ( {dvORtab: dvORtab, id: id , formName : formName } , this.node.whichTab) ;

        }
        var qry="return%20("+node.id+"%20as%20obj).algorithm;";
        linkCmd("objQry&raw=1&qry="+qry,{callback:'realload', objCls: this.objCls,  dvORtab: dvORtab, id: id , formName : formName },vjObjAjaxCallback );
        return ;
    };

    this.realload = function(parameters, content )//(dvORtab, id, geometry,formName)
    {

        dvORtab=parameters.dvORtab;
        id=parameters.id;
        formName=parameters.formName;
        this.node.whichTab=parseInt(content);
        if (!this.node.whichTab){
            this.node.whichTab = 1;
        }


        this.loadedID=id;
        this.dvs=(typeof(dvORtab)=="string") ? vjDV[dvORtab] : verarr(dvORtab.obj) ;
        if(!this.loadedID|| !this.dvs)
            return;

        if(formName)this.formName=formName;
        this.constructed=false;

        this.viewers.length=0;
        this.viewersToAdd=[];
        this.loaded=true;

        vjDS.add("loading Fake Data for pie Chart","dsShannonChart","http://?cmd=objFile&filename=dna-alignx_screenShannon.csv&ids="+id+"&raw=1&bust=1412200888504");
        
        var shannonChart = new vjGoogleGraphView({
            data: "dsShannonChart",
            type:"line",
            series:[ {name:'iter'},
                     {name:'leaf'},
                     {name: 'species'},
                     {name: 'genus'},
                     {name: 'family'},
                     {name: 'order'},
                     {name: 'class'},
                     {name: 'phylum'},
                     {name: 'kingdom'},
                   ],
            options: { title:'Shannon Entropy Timeline',
                //legend: 'none',
                lineWidth: 4,
//                lineDashStyle: [4,1],
                lineDashStyle: [2,2,20,2,20,2],
                focusTarget:'category', 
                width: 600, 
                height: 600, 
                vAxis: { title: 'Entropy', minValue:0},
                hAxis: {title: 'Iteration', gridlines: {number: 5}, minValue:0 },
                
            },
//               cols:[{ name: 'leaf', order:1, title: 'Leaf', hidden: false }
//               ,{ name: 'species', order:2,  title: 'Species', hidden: false }
//               ,{ name: 'genus', order:3, title: 'Genus', hidden: false }
//               ,{ name: 'family', order:4, title: 'Family', hidden: false }
//               ,{ name: 'order', order:5, title: 'Order', hidden: false }
//               ,{ name: 'class', order:6, title: 'Class', hidden: false }
//               ,{ name: 'phylum', order:7, title: 'Phylum', hidden: false }
//               ,{ name: 'kingdom', order:8, title: 'Kingdom', hidden: false }
//               ,{ name: 'weighted_sum', hidden: true}
//               ]
        });

        this.formNames='';
        var formNode=gAncestorByTag(gObject(this.dvs[0].name),"form");
        if(formNode)
            this.formName=formNode.attributes['name'].value;

        this.length=0;

        
        if(this.node.whichTab==0){
            this.addviewer('refPanel,referenceSet,referenceList,referenceTreeText', new vjTaxonomicControl ({
                data:'referenceSet',
                icon:'hive-hexagon',
                updateURL:id,
                callbackFun :this.taxonomyElementSelected,
                taxTreeSelected: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
                formName:this.formName,
                viewMode: this.mode,
                extraColumns: ["num", "min", "max", "mean", "stddev", "intval"]
            }));
        }
        else if(this.node.whichTab==1){
            this.addviewer('blastPanel,blastNTset,blastList,blastTreeText', new vjTaxonomicControl ({
                data: 'blastNTset',
//                downloadGIdata: this.makeDS("blastNTdownloadGI").name,
                icon:'img-algo/ncbi-blast.jpeg',
                updateURL:id,
                callbackFun :this.taxonomyElementSelected,
                taxTreeSelected: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
                formName:this.formName,
                viewMode: this.mode,
                extraColumns: ["min_pct", "max_pct", "mean_pct", "stddev", "intval"]
            }));
        }
        else if(this.mode=='mobileview'){
             this.addviewer('blastPanel,blastNTset,blastList,blastTreeText', new vjTaxonomicControl ({
                 data:'blastNTset',
                 icon:'img-algo/ncbi-blast.jpeg',
                 updateURL:id,
                 callbackFun :this.taxonomyElementSelected,
                 taxTreeSelected: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
                 formName:this.formName,
                 viewMode: this.mode,
                 extraColumns: ["min", "max", "mean", "stddev", "intval"]
             
             }));
            
        }
        else if(this.node.whichTab==2){
            this.addviewer('TaxPanel,TaxSet,TaxList,TaxTreeText', new vjTaxonomicControl ({
                data:'taxTreeBrower',
                icon:'tree',
                rows: [
                       //{ name: 'search', align: 'right', type: 'search', isSubmitable: true, url: '' }]
                       { name: 'search', isRgxpControl: true, rgxpOff: true, align: 'right', type: ' search', isSubmitable: true, title: 'Search', description: 'Search someting', url: '' },
                        { name: 'pager', icon: 'page', title: 'per page', description: 'page up/down or show selected number of objects in the control', type: 'pager', counters: [20,50, 100, 1000, 'all'], align: 'left' },
                        { name: 'searchon', showTitle: true, icon:'next',path:'/Search On', title: 'Search On', align: 'right', order: '5', description: 'Search on the following fields' },
                        { name: 'searchTaxID', path:'/Search On/searchTaxID', title: 'TaxId', type: 'checkbox', isSubmitable: true, value: true, align: 'left', order: '3', description: 'Search the Taxnonmy Tree by TaxID' },
                       { name: 'searchBioPID', path:'/Search On/searchBioPID',title: 'BioprojectID', type: 'checkbox', isSubmitable: true, value: false, align: 'left', order: '4', description: 'Search the Taxnonmy Tree by Bioproject ID' },
                       { name: 'searchAnyOther', path:'/Search On/searchAnyOther',title: 'anyID', type: 'checkbox', isSubmitable: true, value: false, align: 'left', order: '5', description: 'Search the Taxnonmy Tree by any other ids' },
                       { name: 'searchName', path:'/Search On/searchName',title: 'name', type: 'checkbox', isSubmitable: true, value: false, align: 'left', order: '6', description: 'Search the Taxnonmy Tree by speicies name' }]

                ,updateURL:id,
                cmdUpdateURL : "http://dna.cgi?cmd=ionncbiTax&cnt=50",
                taxTreeOnly:true,
                callbackFun :this.taxonomyElementSelected,
                taxTreeSelected: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
                formName:this.formName,
                extraColumns: ["num", "min", "max", "mean", "stddev", "intval"]
            }));
        }
        var sunburstPanel = new vjD3JS_SunburstHierarchy ({
            data:'blastNTset',
            funclick: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
            formName:this.formName
        });            

        this.addviewer('taxDetailViewer,HelpInfoViewer', new vjTaxonomicDetailsView ({
            data:{taxDetails:'taxDetails',dsTaxonomyViewerSpec:'dsTaxonomyViewerSpec',HelpInfo:'HelpInfo'},
            addHelp:true,
            icon:'tree',
            formName:this.formName
        }));

        if(this.onLoad){
            funcLink(this.onLoad, this, this.viewers);
        }


        this.constructed=true;
        
        //this.viewers['sunburstPanel'].data=['sunburst_test'];

        var tb;
        if(this.dvs[0].tabs){ //alert("start of construct");
            var v2add;

            if(this.node.whichTab==1) {
                v2add=[this.viewers['blastPanel'],this.viewers['blastNTset'],this.viewers['blastList'],this.viewers['blastTreeText']];

//                v2add=[this.viewers['sunburstPanel'],this.viewers['blastPanel'],this.viewers['blastNTset'],this.viewers['blastList'],this.viewers['blastTreeText']];
                
                if(this.dvs.length==1)
                    v2add.push(this.viewers['taxDetailViewer']);
                tb=this.dvs[0].addTab("Taxonomy NT","img/scope.png",v2add);
                
                this.dvs[0].addTab("Convergence","img/scope.png",shannonChart);
                this.dvs[0].addTab("Sunburst","img/scope.png",sunburstPanel);
                
                tb.viewtoggles=-1;
            }
            
            if(this.mode=='mobileview') {
                 v2add=[this.viewers['blastPanel'],this.viewers['blastTreeText']];
                 if(this.dvs.length==1)
                //     v2add.push(this.viewers['taxDetailViewer']);
                 tb=this.dvs[0].addTab("taxonomy NT","img/scope.png",v2add);
                 tb.viewtoggles=-1;
            }
            else if(this.node.whichTab==0){ // if(this.node.whichTab&02){
                v2add=[this.viewers['refPanel'],this.viewers['referenceSet'],this.viewers['referenceList'],this.viewers['referenceTreeText']];
                if(this.dvs.length==1)
                    v2add.push(this.viewers['taxDetailViewer']);
                tb=this.dvs[0].addTab("taxonomy HIVE","bee",v2add);
                tb.viewtoggles=-1;
            }
            else if(this.node.whichTab==2){ // if(this.node.whichTab&02){
                v2add=[this.viewers['TaxPanel'],this.viewers['TaxSet'],this.viewers['TaxList'],this.viewers['TaxTreeText']];
                if(this.dvs.length==1)
                    v2add.push(this.viewers['taxDetailViewer']);
                tb=this.dvs[0].addTab("Taxonomy HIVE","img/scope.png",v2add);
                tb.viewtoggles=-1;
            }


            if(this.dvs.length>1) {
                this.dvs[1].addTab("Taxonomy Info","bee",[this.viewers['taxDetailViewer']]);
                if(this.node.helpTab)    this.dvs[1].addTab("taxonomy help","help",[this.viewers['HelpInfoViewer']]);
                this.dvs[1].render();
                this.dvs[1].load('rerender');

            }

        }
        else{
            this.dvs[0].remove();
            this.viewersToAdd=this.dvs[0].add(this.viewersToAdd).viewers;
        }


        this.dvs[0].render();
        if(this.onPreviewRenderCallback && this.mode == 'mobile') {
            funcLink(this.onMobileRenderCallback, this);
        }
        if(this.onFullviewRenderCallback && this.mode != 'mobile') {
            funcLink(this.onFullviewRenderCallback, this);
        }

        this.dvs[0].load('rerender');
        if(this.onPreviewLoadCallback && this.mode == 'mobile') {
            funcLink(this.onMobileLoadCallback, this);
        }
        if(this.onFullviewLoadCallback && this.mode != 'mobile') {
            funcLink(this.onFullviewLoadCallback, this);
        }


        if(this.onRender){
            funcLink(this.onRender, this, this.viewers);
        }
    };

    this.taxonomyElementSelected=function(viewer,node)
    {
        if(!node.taxid) node = viewer;
        var hdr = "id,name,path,value\n";
        var newUrl="static://";
        if(node.taxid){
            newUrl = "http://dna.cgi?taxid="+node.taxid+"&depth=1&cmd=ionTaxInfo";
        }

        vjDS[this.viewers['taxDetailViewer'].data[1]].reload(newUrl, true);

    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }

};