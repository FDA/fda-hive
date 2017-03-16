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

/*********************
 *
 *      VIEWS
 *
 **********************/

javaScriptEngine.include("d3js/annotation_box.js");

function vjPopulPanelSummaryView(viewer)
{

    this.iconSize=24;
    this.icons={expand:null};
    this.LastIndexCol=0;
    this.allowPartialLoad = true;

    var rowlist = [ {
        name : 'Filter',
        path : '/Filters',
        title : 'Filters',
        icon : 'filter-plain',
        description : 'Filter clones',
        showTitle:true,
        iconSize:14,
        order:2,
        align:'right'
    }, {
        name :'hideClones',
        isSubmitable : true,
        hidden:true,
    }, {
        name :'hiddenClones',
        path : '/Filters/hiddenClones',
        order : 1,
        showTitle:true,
        title: 'Hide selected' ,
        iconSize: 14,
        description: 'Reset the content of the control' ,
        url : viewer.hideClones,
        icon : 'delete'
    }, {
        name : 'sep1',
        path : '/Filters/sep1',
        order: 2,
        type : 'separator'
    }, {
        name : 'minCloneCov',
        path : '/Filters/minCloneCov',
        title : '0',
        order : 3,
        size:1,
        type : 'text',
        description : 'Specify coverage threshold',
        isSubmitable : true,
        prefix : 'Coverage'
    }, {
        name : 'minCloneLen',
        path : '/Filters/minCloneLen',
        title : '0',
        type : 'text',
        order : 4,
        size:1,
        description : 'Specify length threshold',
        isSubmitable : true,
        prefix : 'Length'
    }, {
        name : 'minCloneSup',
        path : '/Filters/minCloneSup',
        title : '0',
        order : 5,
        size:1,
        type : 'text',
        description : 'Specify supporting mutation threshold',
        isSubmitable : true,
        prefix : 'Support'
    }, {
        name : 'sep2',
        path : '/Filters/sep2',
        order: 6,
        type : 'separator'
    }, {
        name : 'mergeHidden',
        path : '/Filters/mergeHidden',
        type : 'checkbox',
        isSubmitable : true,
        title : 'Collapse',
        value : true,
        description : 'Collapse filtered clones to their parents',
        order : 7
    },{
        name : 'select_all',
        path : '/Filters/select_all',
        url : viewer.selectAllCallback,
        align:'left',
        title : 'Toggle selection',
        description : 'Toggle selection',
        value : false,
        showTitle:true,
        icon : 'refresh',
        iconSize : 14,
        order : 0.1
    }, {
        name : 'noGapsFrame',
        path : '/Filters/noGapsFrame',
        type : 'select',
        isNAlignRight:true,
        options : [[0, 'Show coordinates based on the mutual alignment frame','no gaps'], [1,'Gaps introduced by multiple alignment will be ignored','multiple alignment gaps'],[2,'All gaps, including read alignment supported gaps, will be removed ','all gaps']],
        isSubmitable : true,
        title : 'Ignore',
        description : 'Ignore gaps introduced by the multiple alignment or all gaps.',
        value : 2,
        order : 20
    }, {
        name :'refresh',
        align:'left',
        order : -1,
        showTitle:true,
        title: 'Reset' ,
        iconSize: 14,
        description: 'Reset the content of the control' ,
        url : viewer.resetCallback,
        icon : 'recRevert'
    }, {
        name:'apply',
        order:18 ,
        title: 'Apply' ,
        showTitle:true,
        icon:'recSet' ,
        iconSize: 14,
        description: 'load data',
        isSubmitter: true
    }
            ];
    this.rows = verarr(this.rows).concat(rowlist);

    vjPanelView.call(this, viewer);
};
function vjPopulPanelCloneAbsView(viewer)
{

    this.iconSize=24;
    this.datastack=true,
    this.icons={expand:null};
    this.LastIndexCol=0;
    this.allowPartialLoad = true;

    var rowlist = new Object([ {
        name : 'back',
        icon : 'search',
        type:'',
        iconSize : '12',
        align : 'right',
        description : 'zoom out',
        url : viewer.backCallback
    }, {
        name : 'edit',
        path : '/edit',
        title : 'Edit',
        icon : 'eye',
        description : 'Edit graph',
        showTitle:true,
        iconSize:18,
        order : 1,
        align:'left'
    }, {
        name :'thrsldTiles',
        type : 'text',
        showTitle:false,
        path : '/edit/thrsldTiles',
        title : '0',
        prefix: 'Threshold' ,
        align : 'right',
        size : 3,
        order : 4,
        description: 'Set threshold for mosaic and tile view. In ragne [0,100]',
        url : viewer.toggletileClones,
    }, {
        name :'toggleTiles',
        path : '/edit/toggleTiles',
        type : 'button',
        showTitle:true,
        title: 'Similarities' ,
        align : 'right',
        order : 3,
        description: 'Switch between mosaic and tile view',
        url : viewer.toggletileClones,
    }, {
        name : 'sepd1',
        path : '/edit/sepd1',
        order: 2,
        type : 'separator'
    }, {
        name : 'ionObjs',
        path : '/ionObjs',
        order : 1,
        align : 'left',
        title : 'Annotation ID',
        isSubmitable : true,
        description : 'Annotation ID to map',
        type : "explorer",
        explorerObjType : "u-ionAnnot",
        size : 1,
        multiSelect : true
    }, {
        name:'applyAnnot',
        path : '/ionObjs/applyAnnot',
        order:1.1 ,
        title: 'Apply' ,
        showTitle:true,
        icon:'recSet' ,
        iconSize: 10,
        description: 'load data',
        url : viewer.annotationCallbackClones
    }, {
        name :'toggleMosaic',
        path : '/edit/toggleMosaic',
        type : 'button',
        showTitle:true,
        title: 'Toggle Mosaic' ,
        align : 'right',
        order : 6,
        description: 'Turn on and off mosaic',
        url : viewer.toggleMosaicClones,
    }, {
        name : 'sepd2',
        path : '/edit/sepd2',
        order: 5,
        type : 'separator'
    }, {
        name :'toggleAverage',
        path : '/edit/toggleAverage',
        type : 'button',
        showTitle:true,
        title: 'Average Coverage' ,
        align : 'right',
        order : 0,
        description: 'Average the coverage',
        url : viewer.averageCoverageClones,
    }, {
        name : 'sepd001',
        path : '/edit/sepd001',
        order: 0.01,
        type : 'separator'
    }, {
        name :'toggleMergeLine',
        path : '/edit/toggleMergeLine',
        type : 'button',
        showTitle:true,
        title: 'Toggle merging lines' ,
        align : 'right',
        order : 0.2,
        description: 'Show/Hide line indicating merging events',
        url : viewer.toggleMergeLinesClones,
    },  {
        name :'toggleBifLine',
        path : '/edit/toggleBifLine',
        type : 'button',
        showTitle:true,
        title: 'Toggle bifurcating lines' ,
        align : 'right',
        order : 0.1,
        description: 'Show/Hide line indicating bifurcating events',
        url : viewer.toggleBifurcateLinesClones,
    }, {
        name : 'sepd021',
        path : '/edit/sepd021',
        order: 0.21,
        type : 'separator'
    },   {
        name :'normCloneCov',
        path : '/edit/normCloneCov',
        type : 'checkbox',
        showTitle:true,
        isSubmitable : true,
        title: 'Normalized coverage' ,
        align : 'right',
        value : false,
        order : 1,
        description: 'Get the normalized version of the graph'
    },{
        name : 'pos_start',
        title : ' ',
        type : 'text',
        size : '4',
        description : 'Specify start position',
        forceUpdate : true,
        isSubmitable : true,
        align:'right',
        prefix : 'Range: '
    }, {
        name : 'pos_end',
        title : ' ',
        type : 'text',
        size : '4',
        description : 'Specify end position',
        forceUpdate : true,
        isSubmitable : true,
        align:'right',
        prefix : '- &nbsp'
    }, {
        name:'submit',
        align:'right',
        type : 'submitter',
        description: 'zoom in'
    }, {
        name:'applyEdit',
        path : '/edit/applyEdit',
        order:1.1 ,
        title: 'Apply' ,
        showTitle:true,
        icon:'recSet' ,
        iconSize: 10,
        description: 'load data',
        isSubmitter: true
    }
        ]);
    this.rows = verarr(this.rows).concat(rowlist);

    vjPanelView.call(this, viewer);
};

function vjPopulPanelExportView(viewer){
    this.iconSize=24;
    this.datastack=true,
    this.icons={expand:null};
    this.LastIndexCol=0;
    this.allowPartialLoad = true;

    
    var rowlist = [ {
        name :'thrs',
        title : 'Thresholds',
        description : 'Edit thresholds',
        showTitle:true,
        iconSize:18,
        order : 0,
        align : 'right',
        order : 10,
    },{
        name :'minFrequency',
        type : 'text',
        path : '/thrs/minFrequency',
        showTitle:false,
        title : '',
        isSubmitable : true,
        prefix: 'Minimum frequency' ,
        value:"0.5",
        align : 'right',
        size : 1,
        order : 0.7,
        description: 'Minimum frequency of the inferred sequences. In ragne [0,100]',
    }, {
        name :'minDiversity',
        type : 'checkbox',
        path : '/thrs/minDiversity',
        showTitle:true,
        title : 'Minimal diversity',
        isSubmitable : true,
        value: true,
        align : 'right',
        order : 0.8,
        description: 'Compute probabilities under the minimal diversity. If off frequencies are computed based on randomly seeded MC',
    }, {
        name : 'sepd12',
        path : '/thrs/sepd12',
        order: 0.9,
        type : 'separator'
    }, {
        name :'simThrs',
        type : 'text',
        path : '/thrs/simThrs',
        showTitle:false,
        title : '',
        isSubmitable : true,
        prefix: 'Breakpoint threshold' ,
        value:"50",
        align : 'right',
        size : 1,
        order : 2,
        description: 'Set threshold for breakpoints view. In ragne [0,100]',
    }, {
        name :'maskLowDiversity',
        type : 'checkbox',
        path : '/thrs/maskLowDiversity',
        showTitle:true,
        title : 'Mask low diversity',
        isSubmitable : true,
        value: true,
        align : 'right',
        order : 2,
        description: 'Position where no reference exceeds the similarity threshold will be mask by the last valid reference',
    }, {
        name : 'sepd2',
        path : '/thrs/sepd2',
        order: 3,
        type : 'separator'
    },  {
        name :'covThrs',
        type : 'text',
        path : '/thrs/covThrs',
        showTitle:false,
        title : '',
        isSubmitable : true,
        prefix: 'Coverage threshold' ,
        value:"0",
        align : 'right',
        size : 2,
        order : 10,
        description: 'Set threshold for minimum depth of coverage',
    }, {
        name : 'download',
        showTitle : true,
        isNAlignRight:true,
        title : 'Download',
        icon : 'download',
        align : 'left',
        description : 'Download all',
        url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else {funcLink(urlExchangeParameter(urlExchangeParameter(url,'clcnt',0),'down','1'));}"
    }, {
        name : 'archive',
        showTitle : true,
        isNAlignRight:true,
        isSubmitable : true,
        title : 'Digest',
        icon : 'dnaold',
        align : 'left',
        description : 'Create sequence object',
        url : viewer.onDigestCallback
    }, {
        name :'cmd',
        type : 'select',
        title : 'Select',
        options : [['popContig', 'Get only contig','contig'], ['popExtended','Get extended contigs','extended'],['popPermutations','Get permuations','permutations'],['popPredictedGlobal','Infer global haplotypes','Infer global']],
        isSubmitable : true,
        isNAlignRight:true,
        align : 'right',
        order : 0,
        value : 'cmdContig',
    }, {
        name :'clcnt',
        type : 'text',
        prefix : 'Limit',
        title : '',
        isSubmitable : true,
        inactive : true,
        align : 'right',
        size : 1,
        order : -1,
        value : '5',
    }, {
        name:'apply',
        order:18 ,
        title: 'Apply' ,
        showTitle:true,
        icon:'recSet' ,
        iconSize: 14,
        description: 'load data',
        isSubmitter: true
    }, {
        name:'contig_print',
        type : 'select',
        options : [['seq', 'Print consensus','consensus'], ['prev', 'Print prevalence in csv','prevalence'], ['al', 'Print alignments in fasta','alignments'], ['cov', 'Print depth of Coverage','coverage'], ['comp','Print contig composition','composition'],['breaks','Print breakpoints','breakpoints'],['sum','Get summary','summary']],
        title: 'Print' ,
        isSubmitable : true,
        align : 'right',
        order : 1,
        isNAlignRight:true
    } ];
    this.rows = verarr(this.rows).concat(rowlist);

    vjPanelView.call(this, viewer);
    
    this.onChangeElementValueParent = this.onChangeElementValue;
    
    this.onChangeElementValue = function(container, nodepath, elname) {
        var node = this.tree.findByPath(nodepath);
        if( node.name == "cmd" ) {
            var el = this.findElement(node.name, container);
            if( el ) {
                var sel = this.findElement("contig_print",container);
                if( sel && sel.options ) {
                    for(a in sel.options ) {
                        if(sel.options[a].value=="sum") {
                            sel.options[a].disabled = (el.value != "popContig");
                            if( sel.options[a].selected ) {
                                var snode = this.tree.findByPath("/contig_print");
                                sel.value = "seq";
                                if( snode ) {
                                    snode.value = sel.value;
                                }
                            }
                        }
                    }
                }
            }
        }
        this.onChangeElementValueParent(container, nodepath, elname);
    }
}

function vjPopulSummaryView(viewer)
{
    this.name='hierarchy';
    this.icon='tree';
    this.hierarchyColumn= 'path';
    this.showLeaf = true;
    this.checkLeafs= true;
    this.checkBranches= true;
    this.hideEmpty= false;
    this.icons= { leaf: 'img/dna.gif' };
    this.showChildrenCount= true;
    this.checkPropagate = false;
    this.doNotPropagateUp = true;
    this.checkBranchCallback=viewer.checkBranchCallback;
    this.checkLeafCallback=viewer.checkLeafCallback;
    this.autoexpand='all';
    this.maxTxtLen = 42;
    this.precompute = "row.path=(row.Hierarchy ? row.Hierarchy : '/') +row['Name'];";

    vjTreeView.call(this, viewer);
};

function vjPopulLengthSummary(viewer) 
{
    this.name='length_summary';
    this.maxTxtLen = 28;
    this.geometry = {
        width : "95%"
    };
    this.checkable = true;
    this.selectCallback = viewer.selectCallback;
    if(viewer.formName){
        this.formObject=document.forms[viewer.formName];
    }
    this.isNCheckableHeader = true;
    this.cols = [ {
        name : 'clone',
        title: 'Clone ID',
        order:1,
        hidden : false
    }, {
        name : 'stacked_size',
        title: 'Extended Length',
        align:'center',
        order:2,
        hidden : false
    }, {
        name : 'region_size',
        title: 'Length',
        align:'center',
        order:3,
        hidden : false
    } ];
    vjTableView.call(this, viewer);
}

function vjAnnotationView(viewer) {
    this.hidden = true;
    this.icon = 'graph2';
    this.name = 'annotation';

    this.columnToPick = {refID:"seqID",start:"start",end:"end","idType-id": "idType-id"};
    this.data = viewer.data;
    this.graphOptions= {
                collapse:true,
                boxSize: 10,
                showTipFull: true
            };
    
    // If width already exists then use it
    if (!viewer.width) viewer.width = 700;//viewer.width;
    else viewer.width = 700;
    if (viewer.height) viewer.height = viewer.height;
    else viewer.height = 200;
    
    viewer.margin = {right: 1,left:50};
    vjD3JS_annotationBox.call(this,viewer);
    
}

function vjPopulAbsView(viewer)
{
    this.name = "sankey";
    
    var serie1=new vjDataSeries({
        title:"clone series",
        name:viewer.data[0],
        center:0.5,
        symbolSize:"0.17",
        isNXminBased: true,
        byX : true,
        columnDefinition:{
            branchID : 'Clone ID',
            start : 'showStart',
            end : 'showEnd',
            trueStart : 'Start',
            trueEnd : 'End',
            weight : 'Max Coverage',
            mergeID : 'Merged ID',
            bifurcateID : 'Bifurcated ID',
            coverage : 'Coverage',
            branchStart : 'Bifurcation Pos',
            branchEnd : 'Merge Pos',
            firstDiff : 'First Diff',
            lastDiff : 'Last Diff',
            support : '# of points of support'
        },
        type:'sankey',
        id:this.dvname+'cloneSerie',
        isok:true});
    var serie2=new vjDataSeries({
        title:"clone legend series",
        name:viewer.data[1],
        isNonTrivialData:true,
        isNXminBased: false,
        type:'raw',
        id:this.dvname+'cloneSerieLegend',
        isok:true});

    var plot1=new vjSVG_Sankey();
    plot1.add(serie1);
    plot1.add(serie2);
    plot1.onCloneClick = viewer.onCloneClick;
    plot1.onCloneOut = viewer.onCloneOut;
    plot1.onCloneOver = viewer.onCloneOver;
    plot1.onCloneMove = viewer.onCloneMove;
    plot1.cloneHierarchyViewer = viewer.cloneHierarchyViewer;
    plot1.colors = viewer.colors?viewer.colors:vjGenomeColors.slice(1);


    this.plots=[plot1];
    this.Axis={
        x:{title:"Position",showGrid:true,showArrow:false},
        y:{title:"SNP",showArrow:false,showTicks:true,showGrid:true,showTitle:false}};
    this.chartArea={width:'95%',height:"85%",left:"5%"};

    vjSVGView.call(this, viewer);
};

function vjPopulExportView(viewer) {
    this.maxTxtLen = 48;
    this.composer = 'preview';
    this.defaultEmptyText = 'no information to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    vjHTMLView.call(this, viewer);
}

function vjPopulDownloads(viewer){
    this.iconSize= 0;
    this.icon= 'download';
    this.defaultEmptyText= "select contig(s) to see available downloads";
    this.geometry= { width: "95%" };
    this.selectCallback=viewer.selectCallback;
    this.cols = [ {
        name : '.*',
        hidden : true
    }, {
        name : 'data',
        title: 'Data',
        order:1,
        hidden : false
    }, {
        name : 'down',
        type : 'icon',
        title: 'Get',
        iconSize:16,
        align:'center',
        order:4,
        hidden : false
    }/*, {
        name : 'digest',
        type : 'icon',
        title: 'Digest',
        iconSize:16,
        align:'center',
        order:5,
        hidden : false
    }*/ ];
    this.maxTxtLen= 32;
    vjTableView.call(this, viewer);
}

function vjPopulSummaryDownloads(viewer){
    vjPopulDownloads.call(this,viewer);
    vjTableView.call(this, viewer);
}
//

/*********************
 *
 *      CONTROLS
 *
 **********************/
function vjPopulExportControl (viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    
    this.objID="vjControl_"+Math.random();
    vjObj.register(this.objID,this);

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for SummaryControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for SummaryControl");
    }
    this.formObject=document.forms[this.formName];

    this.onSelectAllCallback = function(){
        this.cloneSumView.enumerate((function(params,node){params.mimicCheckmarkClick.call(params,node.path);}), this.cloneSumView);
    };
    
    this.panelViewer = new vjPopulPanelExportView({
        data: ["dsVoid"].concat(this.data),
        parentVIS:this.parentVIS,
        width:this.width,
        onDigestCallback:this.onExportDigestCallback,
        formObject: this.formObject
    });

    this.exportView = new vjPopulExportView({
        data: verarr(this.data)[0],
        width:this.width,
//        selectCallback : viewer.selectCallback,
        formObject: this.formObject
    });

    return [this.panelViewer,this.exportView];
}
function vjPopulSummaryControl(viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    
    this.objID="vjControl_"+Math.random();
    vjObj.register(this.objID,this);

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for SummaryControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for SummaryControl");
    }
    this.formObject=document.forms[this.formName];

    this.onSelectAllCallback = function(){
        this.cloneSumView.enumerate((function(params,node){params.mimicCheckmarkClick.call(params,node.path);}), this.cloneSumView);
    };
    
    this.panelViewer = new vjPopulPanelSummaryView({
        data: ["dsVoid"].concat(this.data),
        resetCallback:this.resetPanelCallback,
        hideClones:this.hideClonesCallback,
        selectAllCallback : "javascript:vjObjEvent( 'onSelectAllCallback','"+this.objID+"');",
        parentVIS:this.parentVIS,
        callbackRendered:this.panelCallback,
        width:this.width,
        formObject: this.formObject
    });

    this.cloneSumView = new vjPopulSummaryView({
        checkBranchCallback:this.checkBranchCallback,
        checkLeafCallback:this.checkLeafCallback,
        onMouseOverCallback:this.onMouseOverCallback,
        callbackRendered:this.onSummaryRender,
        data: verarr(this.data)[0],
        width:this.width,
        formObject: this.formObject
    });


    return [this.panelViewer,this.cloneSumView];
}

function vjPopulCloneAbsControl(viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    this.objID="vjControl_"+Math.random();
    vjObj.register(this.objID,this);

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentControl");
    }
    this.formObject=document.forms[this.formName];

    this.toggleTiles = function (){
        if( this.cloneAbsView.plots[0].tile === undefined ) {
            this.cloneAbsView.plots[0].tile = 0;
        }
        var thrs_node = this.panelViewer.tree.findByName('thrsldTiles');
        var tile_value = this.cloneAbsView.plots[0].tile;
        
        if(thrs_node && thrs_node.value && isNumber(thrs_node.value)) {
            tile_value = parseFloat(thrs_node.value);
        }
        else if (tile_value) {
            tile_value = false;
        }
        else {
            tile_value = true;
        }
        this.cloneAbsView.plots[0].tile = tile_value;

        this.cloneAbsView.refresh();
    };
    
    this.averageCoverage = function (){
        if( this.cloneAbsView.plots[0].average === undefined ) {
            this.cloneAbsView.plots[0].average = false;
        }
        this.cloneAbsView.plots[0].average ^= true;

        this.cloneAbsView.refresh();
    };
    
    this.toggleBifurcateLines = function (){
        if( this.cloneAbsView.plots[0].isNdrawBifurcateLines === undefined ) {
            this.cloneAbsView.plots[0].isNdrawBifurcateLines = false;
        }
        this.cloneAbsView.plots[0].isNdrawBifurcateLines ^= true;

        this.cloneAbsView.refresh();
    };
    
    this.toggleMergeLines = function (){
        if( this.cloneAbsView.plots[0].isNdrawMergeLines === undefined ) {
            this.cloneAbsView.plots[0].isNdrawMergeLines = false;
        }
        this.cloneAbsView.plots[0].isNdrawMergeLines ^= true;

        this.cloneAbsView.refresh();
    };
    
    this.toggleMosaic = function () {
        var p_sankey = this.cloneAbsView.plots[0];
        if(!p_sankey)return;
        
        if( p_sankey._hideMosaic === undefined ) {
            p_sankey._hideMosaic = false;
        }
        p_sankey._hideMosaic ^= true;

        this.cloneAbsView.refresh();
    };
    this.annotationCallback = function () {
        var ionObjs = this.panelViewer.tree.findByName('ionObjs');
        if( ionObjs && ionObjs.value ) {
            ionObjs = ionObjs.value;
        }
        var ds = vjDS[this.panelViewer.data[2]];
        var url = urlExchangeParameter(ds.url,'ionObjs',ionObjs);
        ds.reload(url,true);
    };

    this.panelViewer = new vjPopulPanelCloneAbsView({
        data: ["dsVoid",this.data[0],this.data[2]],
        backCallback:this.backCallback,
        parentVIS:this.parentVIS,
        width:this.width,
        toggletileClones : "javascript:vjObjEvent( 'toggleTiles','"+this.objID+"');",
        toggleMosaicClones : "javascript:vjObjEvent( 'toggleMosaic','"+this.objID+"');",
        averageCoverageClones : "javascript:vjObjEvent( 'averageCoverage','"+this.objID+"');",
        toggleBifurcateLinesClones : "javascript:vjObjEvent( 'toggleBifurcateLines','"+this.objID+"');",
        toggleMergeLinesClones : "javascript:vjObjEvent( 'toggleMergeLines','"+this.objID+"');",
        annotationCallbackClones :"javascript:vjObjEvent( 'annotationCallback','"+this.objID+"');",
        formObject: this.formObject
    });

    this.cloneAbsView = new vjPopulAbsView({
        cloneHierarchyViewer:this.summaryViewerName,
        callbackRendered:this.onSankeyRenderCallback,
        data: [this.data[0],this.data[1]],
        width:this.width,
        onCloneClick:this.onCloneClick,
        onCloneOut:this.onCloneOut,
        onCloneOver:this.onCloneOver,
        onCloneMove:this.onCloneMove,
        formObject: this.formObject
    });
    
    this.annotationViewer = new vjAnnotationView({
        data: this.data[2],
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });

    return [this.panelViewer,this.cloneAbsView, this.annotationViewer];
}

//# sourceURL = getBaseUrl() + "/js/vjPopulView.js"