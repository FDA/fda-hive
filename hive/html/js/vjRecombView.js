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


function vjRecombDownloadsView(viewer) {
    this.iconSize = 16;
    this.icon = 'download';
    this.defaultEmptyText = "select reference genome to see available downloads";
    this.geometry = {
        width : "95%"
    };
    this.selectCallback = "function:vjObjFunc('onPerformProfileOperation','"
            + this.objCls + "')";
    this.cols = [ {
        name : 'icon',
        type : 'icon',
        hidden : true
    }, {
        name : 'operation',
        hidden : true
    }, {
        name : 'blob',
        hidden : true
    } ];
    this.maxTxtLen = 50;
    vjTableView.call(this, viewer);
}

function vjRecombSNPProfileCoverage(viewer) {
    this.name = "coverage";
    this.icon = 'graph';
    this.series = [ {
        name : 'Position',
    }, {
        name : 'Count Total',
    }];
    this.options = {
        focusTarget : 'category',
        width : viewer.width?viewer.width:'90%',
        height : viewer.height?viewer.height:120,
        vAxis : {
            title : 'Coverage'
        },
        hAxis : {},
        lineWidth : 1,
        isStacked : true,
        legend : 'top',
        chartArea : {
            top : 20,
            left : 70,
            width : '100%',
            height : "70%"
        },
        colors : [ 'blue', 'green' ]
    };
    this.type = 'area';
    vjGoogleGraphView.call(this, viewer);
}

function vjRecombCrossCovControl(viewer) {
    viewer.formObject=document.forms[viewer.formName];
    viewer.possibleGraphs=["area","line", "column"];
    viewer.name = "cross coverage";
    viewer.icon = 'graph';
    viewer.defaultEmptyText = 'Please select two sequences to see recombination results';

    viewer.series = [{ name: 'position' }, { col: 1 }, { col: 2 }];
    
    viewer.options = {
            focusTarget : 'category',
            width : viewer.width?viewer.width:'100%',
            height : viewer.height?viewer.height:360,
            vAxis : {
                title : 'Coverage'
            },
            hAxis : {},
            lineWidth : 1,
            legend : 'top',
            chartArea : {
                top : 20,
                left : 120,
                width : '90%',
                height : "70%"
            },
            colors : [ 'blue', 'green' ]
        };
        viewer.type = 'line';
        return vjGoogleGraphControl.call(this, viewer);
}

function vjRecombSNPProfileSNPView(viewer) {
    this.icon = 'hiveseq-cube';
    this.name = "SNP";
    this.options = {
        focusTarget : 'category',
        isStacked : true,
        width : viewer.width?viewer.width:'90%',
        height : 220,
        vAxis : {
            title : 'SNP %'
        },
        lineWidth : 2,
        isStacked: true,
        legend : 'bottom',
        chartArea : {
            top : 0,
            left : 70,
            height : '80%',
            width : '100%'
        },
        colors : [ 'black' ],
        isok : true
    };
    this.series = [
            {
                name : 'Position',
                label : false
            },
            {
                name : 'Cumulative SNP Frequency',
                eval : 'Math.max(parseFloat(row["Frequency A"]),parseFloat(row["Frequency C"]),parseFloat(row["Frequency G"]),parseFloat(row["Frequency T"]) )'
            } ];
    this.type = 'column';
    vjGoogleGraphView.call(this, viewer);
}

function vjRecombSNPProfileSNPInDelsView(viewer) {
    this.icon = 'hiveseq-cube';
    this.name = "in-dels";
    this.hidden = true;
    this.isNrenderOnHide = true;
    this.options = {
        focusTarget : 'category',
        isStacked : true, // true,
        width : viewer.width?viewer.width:'90%',
        height : 220,
        vAxis : {
            title : 'InDel %'
        },
        lineWidth : 1,
        legend : 'bottom',
        chartArea : {
            top : 0,
            left : 70,
            height : '80%',
            width : '100%'
        },
        colors : [ 'blue', 'red' ],
        isok : true
    };

    this.series = [
            {
                name : 'Position',
                label : false
            },
            {
                name : 'Insertions',
                eval : 'parseFloat(row["Count Total"]) ? parseInt(10000*parseFloat(row["Count Insertions"])/parseFloat(row["Count Total"]))/100 : 0'
            },
            {
                name : 'Deletions',
                eval : 'parseFloat(row["Count Total"]) ? parseInt(10000*parseFloat(row["Count Deletions"])/parseFloat(row["Count Total"]))/100 : 0'
            } ];
    this.type = 'column';
    vjGoogleGraphView.call(this, viewer);
}


function vjRecombPolyplotView (viewer) {
    this.options = {
        focusTarget: 'category',
        isStacked: true, //true,
        width: '90%', height: 220,
        vAxis: { maxValue: 1, title: 'Recombination rate' },
        lineWidth: 1,
        hAxis: {}, legend: 'right',
        chartArea: { top: 0, left: 70, height: '80%', width: '100%' }
    };
    this.series = [{ name: 'position' }, { col: 1 }];
    this.type = 'area';

    vjGoogleGraphView.call(this,viewer);
}

function vjRecombPolyplotScaledView (viewer) {
    this.positionalScaleTo = 100;

    vjRecombPolyplotView.call(this,viewer);
}
function vjRecombCoverageView (viewer) {
    this.options = {
        focusTarget: 'category',
        isStacked: true, //true,
        width: '90%', height: 220,
        vAxis: { maxValue: 1, title: 'Recombination rate' },
        lineWidth: 1,
        hAxis: {}, legend: 'right',
        chartArea: { top: 0, left: 70, height: '80%', width: '100%' }
    };
    this.series = [{ name: 'position' }, { col: 1 }];
    this.type = 'area';

    vjGoogleGraphView.call(this,viewer);
}

function vjRecombCoverageScaledView (viewer) {
    this.positionalScaleTo = 100;

    vjRecombCoverageView.call(this,viewer);
}




/*********************
 *
 *      CONTROLS
 *
 **********************/
function vjRecombSNPProfileControl(viewer)
{

    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjRecombSNPProfileControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjRecombSNPProfileControl");
    }
    this.formObject=document.forms[this.formName];

    this.coverageViewer=new vjRecombSNPProfileCoverage({
        data: this.data,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.snpViewer=new vjRecombSNPProfileSNPView({
        data: this.data,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });
    this.indelsViewer=new vjRecombSNPProfileSNPInDelsView({
        data: this.data,
        width:this.width,
        formObject: this.formObject,
        selectCallback : this.selectCallback
    });

    return [ this.coverageViewer , this.snpViewer, this.indelsViewer];
}

function vjRecombPolyplotControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjRecombPolyplotControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjRecombPolyplotControl");
    }
    this.formObject=document.forms[this.formName];

    this.polyplot=new vjRecombPolyplotView({
        data: this.data,
        width:this.width,
        formObject: this.formObject
    });
    this.polyplot_scaled=new vjRecombPolyplotScaledView({
        data: this.data,
        width:this.width,
        formObject: this.formObject
    });
    return [ this.polyplot, this.polyplot_scaled];
}


function vjRecombCoverageControl(viewer){
    for ( i in viewer ) {
        this[i] = viewer[i];
    }

    if(this.data===undefined){
        alert("DEVELOPER WARNING: UNDEFINED data for vjRecombCoverageControl");
    }
    if(this.formName==undefined){
        this.formName='form-alignment';
        alert("DEVELOPER WARNING: UNDEFINED formName for vjRecombCoverageControl");
    }
    this.formObject=document.forms[this.formName];

    this.coverage=new vjRecombCoverageView({
        data: this.data,
        width:this.width,
        formObject: this.formObject,
        selectCallback: this.selectCallback
    });
    this.coverage_scaled=new vjRecombCoverageScaledView({
        data: this.data,
        width:this.width,
        formObject: this.formObject
    });
    return [ this.coverage, this.coverage_scaled];
}

//# sourceURL = getBaseUrl() + "/js/vjRecombView.js"