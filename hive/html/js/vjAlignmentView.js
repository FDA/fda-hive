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



function vjAlignmentGenericPanelView(viewer)
{

    this.iconSize=24;
    this.icons={expand:null};
    this.LastIndexCol=0;
//    this.showTitles=true;
        this.rows = verarr(this.rows)
            .concat(
                    [
                            {
                                name : 'download',
                                title : 'Download',
                                icon : 'download',
                                align : 'left',
                                order : '1',
                                description : 'Download content',
                                path : "/download"
                            },
                            {
                                name : 'downloadall',
                                path : "/download/all",
                                showTitle : true,
                                title : 'All',
                                icon : 'download',
                                align : 'left',
                                description : 'Download full content',
                                url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(url,'start','-'),'cnt','-'),'down','1'))"
                            },
                            {
                                name : 'downloadpage',
                                path : "/download/page",
                                showTitle : true,
                                title : 'Page',
                                icon : 'download',
                                align : 'left',
                                description : 'Download content of the current page',
                                url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(url,'down','1'))"
                            },
                            {
                                name : 'downloadfasta',
                                title : 'Fasta',
                                icon : 'download',
                                showTitle : true,
                                order : 9,
                                description : 'Download corresponding results in fasta format',
                                path : "/download/fasta"
                            },
                            {
                                name : 'downloadfastaspage',
                                path : "/download/fasta/fastapage",
                                showTitle : true,
                                title : 'Page',
                                icon : 'download',
                                align : 'left',
                                description : 'Download reads of the current page in fasta format',
                                url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(url,'queryAsFasta','1'),'down','1'))"
                            },
                            {
                                name : 'downloadfastasall',
                                path : "/download/fasta/fastaall",
                                showTitle : true,
                                title : 'All',
                                icon : 'download',
                                align : 'left',
                                description : 'Download results in fasta format',
                                url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(url,'queryAsFasta','1'),'start','-'),'cnt','-'),'down','1'))"
                            },
                            {
                                name : 'downloadreads',
                                title : 'Reads',
                                icon : 'download',
                                showTitle : true,
                                order : 9,
                                description : 'Download corresponding reads in fasta format',
                                path : "/download/reads"
                            },
                            {
                                name : 'downloadreadspage',
                                path : "/download/reads/fastapage",
                                showTitle : true,
                                title : 'Page',
                                icon : 'download',
                                align : 'left',
                                description : 'Download reads of the current page in fasta format',
                                url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(url,'readsAsFasta','1'),'down','1'))"
                            },
                            {
                                name : 'downloadreadsall',
                                path : "/download/reads/fastaall",
                                showTitle : true,
                                title : 'All',
                                icon : 'download',
                                align : 'left',
                                description : 'Download reads in fasta format',
                                url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(urlExchangeParameter(url,'readsAsFasta','1'),'start','-'),'cnt','-'),'down','1'))"
                            },
                            {
                                name : 'separ',
                                type : 'separator',
                                align : 'left',
                                align : 'left',
                                order : '2'
                            },
                            {
                                name : 'refresh',
                                title : 'Refresh',
                                icon : 'refresh',
                                align : 'left',
                                order : '3',
                                description : 'refresh the content of the control to retrieve up to date information',
                                url : "javascript:vjDS['$(dataname)'].reload(null,true);"
                            },
                            {
                                name : 'pager',
                                icon : 'page',
                                title : 'per page',
                                align : 'left',
                                order : '4',
                                description : 'page up/down or show selected number of objects in the control',
                                type : 'pager',
                                counters : [ 10, 20, 50, 100, 1000, 'all' ]
                            },
                            {
                                name : 'separ2',
                                type : 'separator',
                                align : 'left',
                                align : 'left',
                                order : '5'
                            },
                            {
                                name : 'alHigh',
                                path : '/alHigh',
                                type : 'text',
                                forceUpdate : true,
                                size : '7',
                                isSubmitable : true,
                                prefix : 'Position',
                                align : 'right',
                                description : 'Jump to specific genomic position',
                                order : '1',
                                title : ' ',
                                value : ' '
                            },
                            {
                                name : 'search',
                                path : '/search',
                                isRgxpControl : true,
                                rgxpOff : true,
                                align : 'right',
                                order : Number.MAX_VALUE,
                                type : ' search',
                                isSubmitable : true,
                                title : 'Search',
                                description : 'Search sequences. Regular expressions are currently off',
                                url : "?cmd=objFile&ids=$(ids)"
                            },
                            {
                                name : 'alTouch',
                                path : '/alHigh/touching',
                                title : 'Show alignments touching specified position',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : true,
                                align : 'right',
                                order : '2',
                                description : 'Show only reads that touch the position'
                            },
                            {
                                name : 'alVarOnly',
                                path : '/alHigh/alVarOnly',
                                title : 'Mutated',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : false,
                                align : 'right',
                                order : '5',
                                description : 'Show only alignments with mutation at the specific position'
                            },
                            {
                                name : 'randomize',
                                path : '/search/shuffle',
                                title : 'Shuffle alignments randomly',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : false,
                                align : 'right',
                                order : '2',
                                description : 'Show results in non sorted order'
                            },
                            {
                                name : 'alPosSearch',
                                title : 'Find only patterns touching specified position',
                                path : '/alHigh/alPosSearch',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : true,
                                align : 'right',
                                order : '3',
                                description : 'Search pattern occurs only in selected position'
                            },
                            {
                                name : 'collapseRpts',
                                path : '/search/collapseRpts',
                                title : 'Collapse repeats',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : true,
                                align : 'right',
                                order : '0',
                                description : 'Collapse identical reads'
                            },
                            {
                                name : 'printNs',
                                path : '/search/printNs',
                                title : 'Print \'N\'s',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : true,
                                align : 'right',
                                order : '3',
                                description : 'print Ns'
                            },
                            {
                                name : 'separ21',
                                type : 'separator',
                                path : '/search/separ21',
                                order : '3.01'
                            },
                            {
                                name : 'printForward',
                                path : '/search/printForward',
                                title : 'Forward alignments',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : true,
                                align : 'right',
                                order : '3.1',
                                description : 'Include alignments that have been aligned to the forward strand'
                            },
                            {
                                name : 'printReverse',
                                path : '/search/printReverse',
                                title : 'Reverse alignment',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : true,
                                align : 'right',
                                order : '3.2',
                                description : 'Include alignments that have been aligned to the reverse strand'
                            },
                            {
                                name : 'separ22',
                                type : 'separator',
                                path : '/search/separ22',
                                order : '3.21'
                            },
                            {
                                name : 'alNonPerfOnly',
                                path : '/search/alNonPerfOnly',
                                title : 'Non-perfect alignments',
                                type : 'checkbox',
                                isSubmitable : true,
                                value : false,
                                align : 'right',
                                order : '7',
                                description : 'Show non-perfect alignments only'
                            },
                            {
                                name : 'separ23',
                                type : 'separator',
                                path : '/search/separ23',
                                order : '7.21'
                            },
                            {
                                name : 'filterTails',
                                prefix : 'Filter overhang less than',
                                path : '/search/filterTails',
                                type : 'text',
                                isSubmitable : true,
                                title : ' ',
                                align : 'right',
                                order : '12',
                                description : 'Filter alignments with overhang less than the specified cutoff'
                            },
                            {
                                name : 'printTailsOnly',
                                path : '/search/printTailsOnly',
                                title : 'Overhang format',
                                type : 'select',
                                options: [['0','Alignment only', 'Alignment only'],['1','Alignments with overhang', 'Alignments with overhang'],['2','Overhang only','Overhang only']],
                                isSubmitable : true,
                                value : 0,
                                align : 'right',
                                order : '12',
                                description : 'Show only alignments with overhang over a given length'
                            }


                    ]);

    vjPanelView.call(this, viewer);
}

function vjAlignmentView(viewer)
{
    if(!this.name)
        this.name="alignment";
    this.iconSize=0;
    this.rowSpan=3;
    this.defaultEmptyText="Select a reference genome from Hit List table to see detail information";
    this.dataTypeAppliesToHeaders=true;
    this.cols=verarr(this.cols).concat([{ name: new RegExp(/.*/), isNmatchCol: true },
        { name: "Element #", hidden: true },
        { name: "Direction", hidden: true },
        { name: "#", hidden: true },
        // { name: "Sequence", maxTxtLen : 32 , hidden: true },
        { name: "Alignment", type: 'alignment', isNmatchCol: false },
        { name: new RegExp(/Position.*/), hidden: true },
        { name: new RegExp(/Motif.*/), hidden: true}
        ,{ name: new RegExp(/PosHigh/), hidden: true}]);
    if(!this.precompute)this.precompute="";
    this.precompute+="node.bgcolor=(node.cols[2].indexOf('-')==-1  ?'#e4e4ff' : '#dfffdf');"+
                    "if(node['Motif Start']){node.matchSubstring={start: parseInt(node['Motif Start']), end:parseInt(node['Motif End'])};"+
                    "node['Motif End']=parseInt(node['Motif End'])+parseInt(node.Start);}" +
                    "if(docLocValue('printTailsOnly', 0, vjDS[params.data[0]].url)==2) params.rowSpan=1; else params.rowSpan=3;";
    this.matchColor='#FF9900';
    this.maxTxtLen=4096;

    vjTableView.call(this, viewer);
}

function vjAlignmentPanelView(viewer)
{
    this.rows=verarr(viewer.rows).concat(
        [
        { name: 'rgSub', path: '/search/rgSub', title: 'Search on reference', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '4', description: 'Show only reads that touch the position' },
        { name: 'rgInt', path: '/search/rgInt', title: 'Search on comparison', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '5', description: 'Show only reads that touch the position' },
        { name: 'rgQry', title: 'Search on read',path: '/search/rgQry', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '6', description: 'Show only reads that touch the position'},
        { name : 'rgSepar2', type : 'separator', path: '/search/rgSepar2' , order : '6.001'}
        ]);
    this.rowSpan = 3;

    vjAlignmentGenericPanelView.call(this, viewer);
}

function vjAlignmentStackView(viewer)
{
    this.iconSize=0;
    this.defaultEmptyText="Select a reference genome from Hit List table to see detail information";
    if(!this.name)
        this.name="stack";

    if(!this.precompute)this.precompute="";
    this.precompute+="node.bgcolor=(node.cols[2].indexOf('-')==-1 ?'#f2f2f2' : '#f2f2f2');"+
                    "if(node.cols[9])node.PosHigh=parseInt(node.cols[9]);"+
                    "if(node.cols[10]){node.matchSubstring={start: parseInt(node.cols[10]), end:parseInt(node.cols[11])};}";
    this.matchColor='#FF9900';
    this.dataTypeAppliesToHeaders=true;
    this.cols=verarr(this.cols).concat([{ name: new RegExp(/.*/), type: 'alignment', hidden: true, isNmatchCol: true }
           ,{ name: new RegExp(/\S/), hidden: false, isNmatchCol: true  }
           ,{ name: 'Element #', hidden: true }
           ,{ col: 1, hidden: true }
           ,{ col: 2, hidden: true }
           ,{ col: 3, hidden: false }
           ,{ col: 4, isNmatchCol: false}
           ]);

    this.maxTxtLen=4096;

    vjTableView.call(this, viewer);
}

function vjAlignmentConsensusView(viewer) {
    this.maxTxtLen = 48;
    this.composer = 'preview';
    this.defaultEmptyText = 'no information to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    vjHTMLView.call(this, viewer);
}

function vjAlignmentMultipleStackView(viewer)
{
    this.iconSize=0;
    this.defaultEmptyText="Select a reference genome from Hit List table to see detail information";
    if(!this.name)
        this.name="stack";

    if(!this.precompute)this.precompute="";
    this.precompute+="if(node.cols[9])node.PosHigh=parseInt(node.cols[9]);"+
                    "if(node.cols[10]){node.matchSubstring={start: parseInt(node.cols[10]), end:parseInt(node.cols[11])};}";
    this.matchColor='#FF9900';
    this.preEditTable="javascript:thisObj.rowSpan=Int((thisObj.tblArr.rows.length-1)/(Int(thisObj.row(thisObj.tblArr.rows.length-1).cols[0])+1))+1;thisObj.panelObj.rowSpan=thisObj.rowSpan;thisObj.panelObj.render();";
    this.dataTypeAppliesToHeaders=true;
    this.cols=verarr(this.cols).concat([
            { name: new RegExp(/.*/), type: 'alignment', hidden: true, isNmatchCol: true }
//           ,{ name: new RegExp(/\S/), hidden: false, isNmatchCol: true  }
//           ,{ name: 'Element #', hidden: false ,order:0}
           ,{ name: 'Alignment', isNmatchCol: false, hidden: false ,order:3}
           ,{ name: 'Subject$', hidden: false, order:1 }
           ,{ name: 'End', hidden: false,order:4 }
           ,{ name: 'Start', hidden: false,order:2 }
//           ,{ col: 4, isNmatchCol: false}
           ]);
    this.maxTxtLen=4096;

    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    vjTableView.call(this, viewer);
}

function vjAlignmentStackMultiplePanelView(viewer)
{
    vjAlignmentGenericPanelView.call(this, viewer);
    this.excludedDataRefresh.push(2);
}

function vjAlignmentStackPanelView(viewer)
{
    vjAlignmentGenericPanelView.call(this, viewer);
    this.excludedDataRefresh.push(2);
}

function vjAlignmentBasecallBiasView(viewer)
{
    this.series=[{ name: 'Position' },
             { name: 'Count-A'},
             { name: 'Count-C'},
             { name: 'Count-G'},
             { name: 'Count-T'}
             ];

    this.options={
            width: "100%", height: 120, vAxis: { title: 'Base-call Bias' },
            hAxis: {title:'Distribution of read position mapping current reference position'}, lineWidth: 1, legend: 'top', chartArea: { top: 20, left: 70, width: '100%', height: "60%" },
            colors: vjGenomeColors,
            changeHeight: false,
            focusTarget: 'category',
            isStacked:true
    };
    this.type= 'column';

    vjGoogleGraphView.call(this, viewer);
}

function vjAlignmentMatchView(viewer)
{
    if (!this.name)
        this.name = "match_list";

    this.iconSize = 0;
    this.bgColors = [ '#efefff', '#ffffff' ];
    this.defaultEmptyText = "Select a reference genome from Hit List table to see detail information";
    this.startAutoNumber = 1;
    this.maxTxtLen = 32;

    vjTableView.call(this, viewer);
}

function vjAlignmentHitPieView(viewer)
{

    if (!this.name)
        this.name = "match_list";

    this.options = {
        width : this.width ? this.width : 300,
        height : this.height ? this.height : 300,
        changeHeight: false,
        chartArea : {
            height : '95%',
            width : '95%'
        },
        legend : 'top',
        colors : vjGenomeColors,
        is3D : true,
        pieSliceText : 'label'
    };
    this.series = [ {
        label : true,
        name : 'Reference',
        title : 'Reference Genes'
    }, {
        name : 'Hits',
        title : 'Hits'
    } ];
    this.type = 'pie';
    if (!this.precompute)
        this.precompute = "";
    this.precompute += "if(node.id=='+' && node.Reference=='total')node.hidden=true;";

    vjGoogleGraphView.call(this, viewer);
}

function vjAlignmentHitListView(viewer)
{
    this.maxTxtLen = 32;
    this.defaultEmptyText = 'no reference sequences to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.checkColors = vjGenomeColors;
    this.colIDColors = vjGenomeColors;
    this.checkable = true;
    this.cols = [ {
        name : 'Hits',
        type : 'largesci'
    }, {
        name : 'Hits Unique',
        type : 'largenumber',
        hidden : true
    }, {
        name : 'Length',
        type : 'largenumber'
    }, {
        name : 'RPKM',
        type : 'largenumber'
    }, {
        name : 'Reference',
        maxTxtLen : 15
    }, {
        name : 'name',
        hidden : true
    }, {
        name : 'value',
        hidden : true
    } ];

    if (!this.precompute)
        this.precompute = "";
    this.precompute += "if(node.id==0 || node.id=='+')node.styleNoCheckmark=true;";

    vjTableView.call(this, viewer);
}

function vjAlignmentHitListViewMobile(viewer)
{
    this.maxTxtLen = 32;
    this.defaultEmptyText = 'no reference sequences to show';
    this.bgColors = [ '#f2f2f2', '#ffffff' ];
    this.checkColors = vjGenomeColors;
    this.colIDColors = vjGenomeColors;
    this.checkable = true;
    if (viewer.columnToDisplay != undefined && viewer.columnToDisplay.length){
        this.cols = [
           {
             name: new RegExp(/.*/),
             hidden:true
           }];
        for (var ii=0; ii< viewer.columnToDisplay.length; ++ii){
            if(viewer.columnToDisplay[ii].hidden == undefined || viewer.columnToDisplay[ii].hidden==true) viewer.columnToDisplay[ii].hidden = false;
            this.cols.push(viewer.columnToDisplay[ii]);
        }
    }
    if (!this.precompute)
        this.precompute = "";
    this.precompute += "if(node.id==0 || node.id=='+')node.styleNoCheckmark=true;";
    this.bgColors = [ 'FAFAFA', '#dod9ff' ];

    vjMobileTableView.call(this, viewer);
}
function vjAlignmentHistogramControl(viewer)
{
    if (!viewer.name)
        viewer.name = "histogram";
    viewer.formObject=document.forms[viewer.formName];
    viewer.possibleGraphs=["area","line", "column"];
    viewer.options = {
        width : this.width ? this.width : 700,
        height : this.height ? this.height : 300,
        changeHeight: false,
        /*chartArea : {
            height : '95%',
            width : '95%'
        },*/
        vAxis:{title:'count'},
        hAxis:{title:'length'},
        legend : 'bottom',
        focusTarget: 'category',
        lineWidth: 1,


        smoothLine: true,
        colors : ['#7f7f7f', 'blue', 'aqua'],
        is3D : true
    };
    viewer.logGraph=true;
    viewer.series = [ {
        //label : true,
        name : 'position'
    },{
        name : 'Unaligned Reads'
    } , {
        name : 'Aligned Reads'
    } , {
        name : 'Alignments'
    } ];
    viewer.type = 'area';
/*
 *   {
        name : 'All Reads'
    } , {
        name : 'Left Incomplete Alignments'
    } , {
        name : 'Right Incomplete Alignments'
    }
 */
    return vjGoogleGraphControl.call(this, viewer);
}

function vjAlignmentDownloadsView(viewer)
{
    this.iconSize = 0;
    this.icon = 'download';
    this.newSectionWord = "-";
    this.defaultEmptyText = "Select a reference genome from Hit List table to see detail information";
    this.geometry = { width: 500 };
    this.selectCallback = "function:vjObjFunc('onPerformReferenceOperation','" + this.objCls + "')";
    this.cols = [ {
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
    }, {
        name : 'arch',
        type : 'icon',
        title: 'Archive',
        iconSize:16,
        align:'center',
        order:5,
        hidden : false
    }, {
        name : 'arguments',
        hidden : true
    }, {
        name : 'operation',
        hidden : true
    }, {
        name : 'params',
        hidden : true
    } ];
    this.maxTxtLen = 28;

    vjTableView.call(this,viewer);
}


/*********************
 *
 *      CONTROLS
 *
 **********************/

function vjAlignmentControl(viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer = new vjAlignmentPanelView({
        data: ["dsVoid",this.data],
        parentVIS:this.parentVIS,
        width:this.width,
        formObject: this.formObject
    });

    this.alignViewer = new vjAlignmentView({
        data: this.data,
        width:this.width,
        formObject: this.formObject
    });

    return [this.panelViewer,this.alignViewer];
}

function vjAlignmentStackControl(viewer)
{
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentStackControl");
        if(this.data.stack===undefined){
            alert("DEVELOPER WARNING: UNDEFINED data.stack for alignmentStackControl");
        }
        if(this.data.basecallBias===undefined){
            alert("DEVELOPER WARNING: UNDEFINED data.mutBias for alignmentStackControl");
        }
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentStackControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer=new vjAlignmentStackPanelView({
        data: ["dsVoid", this.data.stack,this.data.basecallBias],
        parentVIS:this.parentVIS,
        width:this.width,
        formObject: this.formObject
    });
    this.panelViewer.excludedDataRefresh.push(2);

    this.basecallBiasViewer=new vjAlignmentBasecallBiasView({
        data: this.data.basecallBias,
        width:this.width,
        formObject: this.formObject
    });

    this.stackViewer=new vjAlignmentStackView({
        data: this.data.stack,
        width:this.width,
        formObject: this.formObject});
    this.stackViewer.precompute+="node.bgcolor=(node.cols[2].indexOf('-')==-1  ?'#e4e4ff' : '#dfffdf');"+
    "if(node['Motif Start']){node.matchSubstring={start: parseInt(node['Motif Start']), end:parseInt(node['Motif End'])};"+
    "node['Motif End']=parseInt(node['Motif End'])+parseInt(node.Start);}";
    
    return [this.panelViewer,this.basecallBiasViewer,this.stackViewer];
}

function vjAlignmentMultipleStackControl(viewer) {
    for ( i in viewer ) {
        this[i] = viewer[i];
    }
    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentStackControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentStackControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer=new vjAlignmentStackMultiplePanelView({
        data: ["dsVoid", this.data],
        parentVIS:this.parentVIS,
        width:this.width,
        formObject: this.formObject
    });
    this.panelViewer.excludedDataRefresh.push(2);


    this.stackViewer=new vjAlignmentMultipleStackView({
        data: this.data,
        width:this.width,
        panelObj:this.panelViewer,
        formObject: this.formObject});

    return [this.panelViewer,this.stackViewer];
}

function vjAlignmentHitListControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentHitListControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentHitListControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer=new vjBasicPanelView({
        data:["dsVoid",this.data],
        width:this.width,
        loadedID: this.loadedID,
        formObject:this.formObject
    });
    this.panelViewer.rows = this.panelViewer.rows.concat([
                          {
                            name : 'ionObjs',
                            path : '/ionObjs',
                            order : 99,
                            align : 'right',
                            title : 'click here to select annotation files',
                            isSubmitable : true,
                            description : 'List of Annotation Objects to map. Press and hold Ctrl for multi-selection',
                            type : "explorer",
                            explorerObjType : "u-ionAnnot",
                            size : 1,
                            multiSelect : true 
                          },{
                            name:'extend',
                            path:"/ionObjs/extend",
                            title:"Show Extended-Hit-list",
                            type:"button",
                            url: retrieve_additionalInfo,
                            order:99  
                           },{
                                name : 'sepd1',
                                path : '/ionObjs/sepd1',
                                order: 98,
                                type : 'separator'
                            }
                          
    ]);
    if (viewer.isPartOfProfiler!= undefined && viewer.isPartOfProfiler.length!=0){
        this.panelViewer.rows = this.panelViewer.rows.concat([
              {name: "profilerID", path:"/ionObjs/" + "profilerID",title:"&nbsp;Include this profiler&nbsp;",type:"checkbox",order:1, profilerID: node.profiler }
        ]);
    }
    
    this.hitListViewer=new vjAlignmentHitListView({
        data: this.data,
        formObject: this.formObject,
        width:this.width,
        checkable: viewer.checkable, //  ? true:false,
        selectCallback:this.selectCallback,
        checkCallback:this.checkCallback,
        dbClickCallback: this.dbClickCallback
    });

    return [this.panelViewer,this.hitListViewer];
}

function retrieve_additionalInfo(panel,row,node){
    var ionList = panel.tree.findByName('ionObjs').value;
    var url = "?cmd=tblqry-new&extendAnnot=1&cnt=50&objs=" + this.loadedID;
    var isChange = 0;
    var profilerIDButton = panel.tree.findByPath("/ionObjs/profilerID");
    if (profilerIDButton && profilerIDButton.profilerID && profilerIDButton.value) {
        url=urlExchangeParameter(url,"profilerID",profilerIDButton.profilerID);
        isChange =1;
    }
    if (ionList != undefined && ionList.length){
        url=urlExchangeParameter(url,"ionObjs",ionList);
        isChange =1;
    }
    if (isChange) {
        linkSelf(url,true);
    }
}

function vjAlignmentHitListControlMobile(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentHitListControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentHitListControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer=new vjBasicPanelView({
        data:["dsVoid",this.data],
        width:this.width,
        formObject:this.formObject
    });
    
   
    this.hitListViewer=new vjAlignmentHitListViewMobile({
        data: this.data,
        formObject: this.formObject,
        width:this.width,
        columnToDisplay:viewer.columnToDisplay,
        checkable: viewer.checkable, //  ? true:false,
        selectCallback:this.selectCallback,
        checkCallback:this.checkCallback
    });

    return [this.panelViewer,this.hitListViewer];
}


function vjAlignmentMatchControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for alignmentMatchControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for alignmentMatchControl");
    }
    this.formObject=document.forms[this.formName];

    this.panelViewer=new vjBasicPanelView({
        data:["dsVoid",this.data],
        width:this.width,
        formObject:this.formObject
    });

    this.matchViewer=new vjAlignmentMatchView({
        data: this.data,
        width:this.width,
        formObject: this.formObject
    });

    return [this.panelViewer,this.matchViewer];
}

function vjAlignmentSaturationControl(viewer){
    if (!viewer.name)
        viewer.name = "saturation";
    viewer.formObject=document.forms[viewer.formName];
    viewer.possibleGraphs=["area","line", "column"];
    viewer.options = {
        width : this.width ? this.width : 350,
        height : this.height ? this.height : 300,
        chartArea : {
            width : '75%',
            height : "72%"
        },
        changeHeight: false,
        pointSize:2,
        vAxis:{title:'Trasncripts'},
        hAxis:{title:'Reads'},
        legend : 'none',
        focusTarget: 'category',
        lineWidth: 1,
        colors : [ 'blue'],
        is3D : true
    };
    viewer.series = [
     {
         name:'reads',
         title:'Reads'
     }, {
         name : 'transcripts',
         title: 'Transcripts'
     } ];
    viewer.type = 'line';
    
    viewer.panel_rows = verarr(viewer.panel_rows).concat([
                                           {
                                               name : 'search',
                                               path : '/search',
                                               isRgxpControl : true,
                                               rgxpOff : true,
                                               align : 'right',
                                               order : Number.MAX_VALUE,
                                               type : ' search',
                                               isSubmitable : true,
                                               title : 'Search',
                                               description : 'Filter subjects based on name'
                                          },
                                          {
                                              name : 'wrap',
                                              path : '/search/wrap',
                                              align : 'right',
                                              order : '4',
                                              title : ' ',
                                              value: 50000,
                                              type : 'text',
                                              size : '4',
                                              prefix : 'Bin size: ',
                                              isSubmitable: true,
                                              description : 'Number of alignments per bin'
                                              
                                          },
                                          {
                                              name : 'minReads',
                                              path : '/search/minReads',
                                              align : 'right',
                                              order : '5',
                                              title : ' ',
                                              value: 0,
                                              type : 'text',
                                              size : '4',
                                              prefix : 'Min reads: ',
                                              isSubmitable: true,
                                              description : 'Number of alignments per bin'
                                              
                                          },
                                          {
                                              name : 'collapseRpts',
                                              path : '/search/collapseRpts',
                                              title : 'Collapse repeats',
                                              type : 'checkbox',
                                              isSubmitable : true,
                                              value : false,
                                              align : 'right',
                                              order : '0',
                                              description : 'Collapse identical reads'
                                          },
                                          {
                                              name : 'reducedSaturation',
                                              path : '/search/reducedSaturation',
                                              title : 'Reduced space',
                                              type : 'checkbox',
                                              isSubmitable : true,
                                              value : false,
                                              align : 'right',
                                              order : '9',
                                              description : 'Remove filtered reads'
                                          }
                                          ]);

    var viewers = vjGoogleGraphControl.call(this, viewer);
    viewers[0].rows = verarr(viewers[0].rows).concat(viewer.panel_rows);
    viewers[0].data = verarr(viewers[0].data).concat(viewer.data);
    return viewers;
}

//# sourceURL = getBaseUrl() + "/js/vjAlignmentView.js"