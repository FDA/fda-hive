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
javaScriptEngine.include("js/vjTaxonomyView.js");

google.load("visualization", "1", {packages:["corechart"]});

vjHO.register('svc-dna-screening').Constructor=function ()
{

    if(this.objCls)return;
    this.resolution=200;


    this.fullview=function(node,dv)
    {
        var id= node.id;
        vjDS.add("","dsShannonChart","http://?cmd=objFile&filename=dna-alignx_screenShannon.csv&ids="+id+"&raw=1");
        vjDS.add("","dsTaxonomyViewerSpec","static://type_id,name,title,type,parent,role,is_key_fg,is_readonly_fg,is_optional_fg,is_multi_fg,is_hidden_fg,is_brief_fg,is_summary_fg,order,default_value,constraint,constraint_data,description\n"
                                            +"taxonomy,name_list,Names,list,,,0,1,0,0,0,0,0,,,,,\n"
                                            +"taxonomy,taxName,Tax-name,string,name_list,,0,1,1,1,0,0,0,,,,,\n"
                                            +"taxonomy,taxid,Taxonomy ID,integer,,,0,1,0,0,0,0,0,,,,,\n"
                                            +"taxonomy,parentid,Parent,integer,,,0,1,0,0,0,0,0,,,,,\n"
                                            +"taxonomy,path,Ancestry,string,,,0,1,0,0,0,0,0,,,,,\n"
                                            +"taxonomy,rank,Rank,string,,,0,1,0,0,0,0,0,,,,,\n"
                                            +"taxonomy,bioprojectID,BioProjectID,integer,,,0,1,0,0,0,0,0,,,,,\n");
        vjDS.add("","HelpInfo","http://help/hlp.page.view.taxonomy.html");
        vjDS.add("","taxDetails","static://");
        vjDS.add("Retrieving data for blastNTset", "blastNTset", "http://dna.cgi?cnt="+this.defaultShowNode+"&cmd=ionncbiTax&percentage=1&screenType=dna-alignx_screenResult.csv&screenId="+id);
        vjDS.add("","taxTreeBrower","static://");
        vjDS.add("","referenceSet","http://dna.cgi?cnt="+this.defaultShowNode+"&cmd=ionncbiTax&screenId=0");
        vjDS.add("","blastNTdownloadGI","http://dna.cgi?cnt="+this.defaultShowNode+"&cmd=ionncbiTax&screenType=dna-alignx_acclist.csv&ginumber=1&screenId=0");
        vjDS.add("","shannonEntropy","static://");

        this.node = node;
        this.node.cntNode = 30;
        this.mode='fullview';
        //this.create(dv,node.id);

        var filesStructureToAdd = [
               {
                tabId: 'taxDetails',
                tabName: "Taxonomy Details",
                position: {posId: 'otherResultsArea', top:'0%', bottom:'100%', left:'60%', right:'100%'},
                preConstructor:{
                    dataViewer: "vjTaxonomicDetailsView",
                    dataViewerOptions: {
                        data:{taxDetails:'taxDetails',dsTaxonomyViewerSpec:'dsTaxonomyViewerSpec',HelpInfo:'HelpInfo'},
                        addHelp:true,
                        icon:'tree',
                        formName: formName
                    }
                },
                tabDependents: ["taxHelp"],
                viewerConstructor: {
                    preObjPos: 0,
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'taxHelp',
                tabName: "Taxonomy Help",
                position: {posId: 'otherResultsArea', top:'0%', bottom:'100%', left:'60%', right:'100%'},
                viewerConstructor: {
                    preObjPos: 1,
                },
                autoOpen: ["computed"]
            }
        ];
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
                                    
        filesStructureToAdd = [
           {
                tabId: 'convergenceGraph',
                tabName: "Convergence",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    dataViewer: 'vjGoogleGraphView',
                    dataViewerOptions: {
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
                                lineWidth: 4,
                                lineDashStyle: [2,2,20,2,20,2],
                                focusTarget:'category', 
                                width: 600, 
                                height: 600, 
                                vAxis: { title: 'Entropy', minValue:0},
                                hAxis: {title: 'Iteration', gridlines: {number: 5}, minValue:0 },
                                
                            }
                    }
                }
            },
            {
                tabId: 'phyloTree',
                tabName: "Phylogenetic Tree",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                preConstructor: {
                    dataViewer: 'vjTaxonomicControl',
                    dataViewerOptions: {
                        data: 'blastNTset',
                        icon:'img-algo/ncbi-blast.jpeg',
                        updateURL:id,
                        callbackFun :this.taxonomyElementSelected,
                        taxTreeSelected: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
                        formName:formName,
                        viewMode: this.mode,
                        extraColumns: ["min_pct", "max_pct", "mean_pct", "stddev", "intval"]
                    }
                },
                tabDependents: ["textTree", "tableTree"],
                viewerConstructor: {
                    preObjPos: [0,1],
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'textTree',
                tabName: "Text Tree",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    preObjPos: 2,
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'tableTree',
                tabName: "Table",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    preObjPos: 3,
                },
                autoOpen: ["computed"]
            },
            {
                tabId: 'sunburst',
                tabName: "Sunburst",
                position: {posId: 'resultsTable', top:'0%', bottom:'100%', left:'20%', right:'60%'},
                viewerConstructor: {
                    dataViewer: 'vjD3JS_SunburstHierarchy',
                    dataViewerOptions: {
                        data:'blastNTset',
                        funclick: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
                        formName:formName,
                        colorCol: "taxid"
                    }
                },
                autoOpen: ["computed"]
            }
        ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        //algoWidgetObj.moveTab("next", {top:"0", bottom: "10%", left: "20%", right: "75%"}, 0);
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


    this.taxonomyElementSelected=function(viewer,node)
    {
        if(!node) node = viewer;
        var hdr = "id,name,path,value\n";
        var newUrl="static://";
        if(node.taxid){
            newUrl = "http://dna.cgi?taxid="+node.taxid+"&depth=1&cmd=ionTaxInfo";
        }

        vjDS["taxDetails"].reload(newUrl, true);
        
        //here, we will open the details tab if its not open
        algoWidgetObj.openTab (algoWidgetObj.optionsForPage.subTabs.results.pageTabChildren[1]);
        algoWidgetObj.openTab (algoWidgetObj.optionsForPage.subTabs.results.pageTabChildren[0]);

    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }

};