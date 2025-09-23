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

function vjAnnotationMapView(viewer)
{
    this.maxTxtLen = 100;
    this.defaultEmptyText = 'no annotations to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.cols = [ {
        name : 'Reference',
        hidden : false
    }, {
        name : 'start',
        title : 'Start',
        hidden : false
    }, {
        name : 'end',
        title : 'End',
        hidden : false
    } ];

    viewer.debug=true;
    this.parentCls = viewer.parentCls;
    vjTableView.call(this, viewer);
    if (this.selectCallback && this.selectCallback.indexOf("thisObjCls") != -1) {
        this.selectCallback = this.selectCallback.replace(/thisObjCls/,this.objCls);
    }
}

function vjAnnotationHitsView(viewer)
{
    this.maxTxtLen = 32;
    this.defaultEmptyText = 'no annotations to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.cols = [ {
        name : 'query',
        hidden : true
    }, {
        name : 'start',
        title : 'Start',
        hidden : false
    }, {
        name : 'end',
        title : 'End',
        hidden : false
    }, {
        name : 'id',
        title : 'Annotation',
        hidden : false
    }, {
        name : 'idType',
        title : 'Annotation type',
        hidden : false,
        wrap:true
    }, {
        name : 'sequenceID',
        title : 'Sequence',
        hidden : true
    } ];
    this.parentCls = viewer.parentCls;
    vjTableView.call(this, viewer);
    
}


function vjAnnotationHitsGraphView(viewer)
{
    this.hidden = false;
    this.icon = 'graph2';
    this.name = "annotation";

    this.data = viewer.data;

    
    this.selectCallback = viewer.anotSelectCallback;
    this.margin = {right: 20,left:20};

    this.type = "column";
    this.series = [
                   {name: "Type" , label: true}, 
                   {name: "Count"}
                ];
    this.options= {
            legen: "bottom",
            vAxis: {
                title:""
            }
    };
    this.parentCls = viewer.parentCls;
    vjGoogleGraphView.call(this, viewer);
}

function vjAnnotationDownloadView(viewer)
{
    this.iconSize = 16;
    this.icon = 'download';
    this.newSectionWord = "-";
    this.defaultEmptyText = "select a reference genome to see detail information";
    this.geometry = { width: 500 };
    this.bgColors = [ '#f2f2f2', '#ffffff' ];

    this.parentCls = viewer.parentCls;
    vjTableView.call(this,viewer);
}



function vjAnnotationMapControl(viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for annotation_mapControl");
    }
    if(this.formName==undefined){
        this.formName='form-annotation_map';
        alert("DEVELOPER WARNING: UNDEFINED formName for annotation_mapControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer = new vjBasicPanelView({
        data: ["dsVoid",this.data],
        parentVIS:this.parentVIS,
        width:this.width,
        formObject: this.formObject
    });
    var download = [
                    { name: 'download', align: 'left', showTitle: true, hidden: false, order: 1, title: 'Download', description: 'Download result', icon: 'download', url: "javascript:var url = \"http:
                    ];
    this.panelViewer.rows = this.panelViewer.rows.concat(download);

    this.mapViewer = new vjAnnotationMapView({
        data: this.data,
        width:this.width,
        formObject: this.formObject,
        parentCls: viewer.parentCls,
        selectCallback: this.selectCallback
    });

    var combMapTable = new vjTableControlX2({
        data: this.data,
        maxTxtLen:32,
        onClickCellCallback: this.selectCallback,
        formObject:this.formObject
    });
    return combMapTable.arrayPanels;
}

function downloadResult(panel,row) {
    var url = "http:
    
     var anotArr = panel.tree.findByName("ionObjs").value;
    if (!anotArr || anotArr.length <2) {
        return;
    } 
     
    url = "http:
    
    vjDS.dsVoid.reload(url, true ,"download");
     
}

function vjAnnotationHitsControl(viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for annotation_hitsControl");
    }
    if(this.formName==undefined){
        this.formName='form-annotation_hits';
        alert("DEVELOPER WARNING: UNDEFINED formName for annotation_hitsControl");
    }
    this.formObject=document.forms[this.formName];

    var preload_anot = "";
    if (this.preload_input && this.preload_input["annot"]) {
        preload_anot = this.preload_input["annot"];
    }
    this.panelViewer=new vjPanelView({
        data: ["dsVoid", this.data[0]],
        width:this.width,
        parentCls: this.parentCls,
        formObject: this.formObject,
        iconSize:"24",
        rows: [
               { name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
               { name: 'search', align: 'right', type: ' search',order:10, isSubmitable: true, title: 'Search', description: 'search sequences by ID',order:'1', url: "?cmd=objFile&ids=$(ids)" },
               { name:"download", align: 'left',order:3, title: "Download",icon:"download", showTitle:true},
               { name: 'download/downloadSimplified', align: 'left', showTitle: true, hidden: false, order: 1, title: 'Current Range', description: 'Download Annotation For Current Range ', url: downloadMap, icon: 'download'},
               { name: 'download/downloadFull', align: 'left', showTitle: true, hidden: false, order: 2, title: 'All Ranges', description: 'Download Annotation For All Ranges', url: downloadResult, icon: 'download'},
               {
                       name : 'ionObjs',
                    path : '/ionObjs',
                    order : 1,
                    align : 'left',
                    title : 'click here to select annotation files',
                    isSubmitable : true,
                    description : 'List of Annotation Objects to map.',
                    type : "explorer",
                    explorerObjType : "u-ionAnnot,u-annot",
                    multiSelect : true,
                    value: preload_anot 
                 }
               ]
    });
    
    this.hitsViewer=new vjAnnotationHitsView({
        data: this.data[0],
        width:this.width,
        parentCls: this.parentCls,
        formObject: this.formObject
    });

    return [this.panelViewer,this.hitsViewer];
}

function downloadMap () {
    var prfx = location.origin + location.pathname;
    var myUrl = prfx + this.dataSourceEngine[this.data[1]].dataurl.substr(7);
    funcLink(myUrl);
}

function vjAnnotationHitsGraphControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    
    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for annotation_hitsControl");
    }
    
    if(this.formName==undefined){
        this.formName='form-annotation_hits';
        alert("DEVELOPER WARNING: UNDEFINED formName for annotation_hitsControl");
    }
    this.formObject=document.forms[this.formName];
    
    var preload_anot = "";
    if (this.preload_input && this.preload_input["annot"]) {
        preload_anot = this.preload_input["annot"];
    }
    
    this.orf_panelViewer=new vjPanelView({
        data: ["dsVoid",this.data],
        width:this.width,
        formObject: this.formObject,
        parentCls: this.parentCls,
        iconSize: "22",
        rows: [
               {name: "cnt", prefix: "Count: ", order: 5,align: 'left', title: "Count",isSubmitable: true, hidden: false, type:"text", value: "1000", showTitle: true, size: 6, description: "how many elements to show. Trying to show all when value is -1"},
               {name: "start",prefix: "Start From: ", order:4, align: 'left',title: "Start",isSubmitable: true, hidden:false, type:"text", value: "0", showTitle: true, size:6, description: "start from"},
               {
                      name : 'anotFiles',
                    path : '/anotFiles',
                    order : 2,
                    align : 'left',
                    title : 'click here to select annotation files',
                    isSubmitable : true,
                    description : 'List of Annotation Objects to map',
                    type : "explorer",
                    explorerObjType : "u-ionAnnot",
                    value: preload_anot
                 },
                 {name: "typeToShow", prefix: "Type: ", order: 3,align: 'left', title: "Count",isSubmitable: true, hidden: false, type:"text", value: "Gene Name", showTitle: true, size: 10, description: "type to show"},
                 {name: "graphType", order: 1, align: 'left', title: "Graph", icon: "graph",hidden: false, showTitle: true, size: 10, description: "Graph Type"},
                 {
                     name: "column", 
                     path: "/graphType/column",
                     order: 0, align: 'left', title: "Column", icon: "graph",
                     showTitle: true, size: 10, description: "Column Chart",
                     url: "javascript: thisObj.relatedViewer.type = node.name; thisObj.relatedViewer.refresh();"
                 },
                 {
                     name: "pie", 
                     path: "/graphType/pie",
                     order: 0, align: 'left', title: "Pie", icon: "pie",
                     showTitle: true, size: 10, description: "Pie Chart",
                     url: "javascript: thisObj.relatedViewer.type = node.name; thisObj.relatedViewer.refresh();"
                 },
                 {name: "getAnnotation", prefix: 'submit',title: "submit",icon: "done.gif", order:-2, align: "right",url: showAnnotationStat},
                 {name: "download", title: "Download Full Dataset",icon: "download", order:-1, align: "right", fromDownload: true, url: showAnnotationStat}
        ]
    });

    this.orf_graphViewer=new vjAnnotationHitsGraphView({
        data: this.data,
        width:this.width,
        formObject: this.formObject,
        parentCls: this.parentCls
    });
    
    this.orf_table = new vjTableView({
        data: this.data,
        defaultEmptyText : 'no annotations to show',
        bgColors : [ '#f2f2f2', '#ffffff' ],
        parentCls: this.parentCls
    });
    this.orf_panelViewer.relatedViewer= this.orf_graphViewer;
    this.orf_graphViewer.relatedViewer= this.orf_panelViewer;
    
    return [ this.orf_panelViewer, this.orf_graphViewer, this.orf_table];
}

function showAnnotationStat(viewer, node,ir,ic) {
    var anotFiles = viewer.tree.findByName("anotFiles").value;
    var title ="";
    var type = viewer.tree.findByName("typeToShow").value;
    type = type.trim(); title=""+type;
    type = encodeURI(type);
    var count = viewer.tree.findByName("cnt").value;
    var start = viewer.tree.findByName("start").value;
    if (!anotFiles)
        return alert("Please select an annotation file");
    
    var url = "http:
    if (node.fromDownload){
        url = urlExchangeParameter(url, "cnt", "-1");
        url = urlExchangeParameter(url, "start", "0");
        vjDS["dsVoid"].reload(url,true,"download");
    }
    else {
        viewer.relatedViewer.options.vAxis.title = "" + title;
        vjDS[viewer.data[1]].reload(url,true);
    }
}

