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
javaScriptEngine.include("js/vjHiveseqView.js");




function vjDemoLengthwiseCountView(viewer)
{
    if(viewer.name === undefined)viewer.name="lengthwise_counts";
    if(viewer.options === undefined)viewer.options={ title:'Lengthwise position count', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:vjPAGE.colorsACGT, legend: 'none',
                                                    vAxis: {title:'base count'},
                                                    hAxis: {title:'position in the reads'}
                                                    };
    if(viewer.series === undefined)viewer.series=[
                                                  {label:false, name:'pos', title: 'Length'},
                                                  {name: 'countA', title: 'A' } ,
                                                  {name: 'countC', title: 'C' },
                                                  {name: 'countG', title: 'G' },
                                                  {name: 'countT', title: 'T' }
                                             ];
    if(viewer.type === undefined)viewer.type='line';
    viewer.cols=[{ name: new RegExp(/.*/),hidden:true}
                ,{ name: 'pos', order:1, align:'center',title: 'Position', hidden: false }
                ,{ name: 'countA', order:2,  title: 'count of A', hidden: false }
                ,{ name: 'countC', order:3,  title: 'count of C', hidden: false }
                ,{ name: 'countG', order:4,  title: 'count of G', hidden: false }
                ,{ name: 'countT', order:5,  title: 'count of T', hidden: false }];
    viewer.minRowsToAvoidRendering=3;
    viewer.switchToColumnMode=32;
    vjGoogleGraphView.call(this, viewer);
}

function vjDemoAIndelsCountView(viewer)
{
    if(viewer.name === undefined)viewer.name="in/del_counts";

    if(viewer.options === undefined)viewer.options={ title:'Indels count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: viewer.width?viewer.width:600, height: viewer.height?viewer.height:300, colors:vjPAGE.colorsACGT,  vAxis: { title: 'In/dels Count', minValue:0}  };
    if(viewer.series === undefined){
        viewer.series=[ {label:true, name:'Indel', title: 'In/Dels'}, {name: 'count', title: 'Count' } ];
    }
    if(viewer.type === undefined)viewer.type='pie';
    viewer.cols=[{ name: 'count',title: 'Count', hidden: false }];
    vjGoogleGraphView.call(this, viewer);
}

vjHO.register('svc-dna-demo').Constructor=function ()
{

    if(this.objCls)return;
    this.resolution=200;

    this.fullview=function(node,dv)
    {
        this.mode='fullview';
        this.create(dv,node.id);
    };

    this.preview = function(node,dv)
    {
        this.mode='preview';
        this.create(dv,node.id);
    };

    this.typeName="svc-dna-demo";


    this.objCls="obj-"+this.typeName+Math.random();
    vjObj.register(this.objCls,this);


    this.active_url=new Object();
    this.inactive_url=new Object();
    this.viewersToAdd=new Array();
    this.viewers=new Object();

    this.colorsACGT=['#006AFF','#95BE4D','#556B2F', '#000080'];
    this.idNulldsName="default";
    this.viewers.length=0;

    this.urlSet ={
            'ACGT' :{
                active_url:"http:
                objs:"ids"

            },
            'Indels' : {
                active_url:"http:
                objs:"ids"

            }
    };

    this.create=function(dvORtab, id, geometry, formName, stickytabs){

        this.load(dvORtab,id, geometry, formName);
        if(this.onPreviewLoadCallback && this.mode == 'preview') {
            funcLink(this.onPreviewLoadCallback, this);
        }
        if(this.onFullviewLoadCallback && this.mode !='preview') {
            funcLink(this.onFullviewLoadCallback, this);
        }

        this.construct(stickytabs);
        if(this.onPreviewRenderCallback && this.mode == 'preview') {
            funcLink(this.onPreviewRenderCallback, this);
        }
        if(this.onFullviewRenderCallback && this.mode !='preview') {
            funcLink(this.onFullviewRenderCallback, this);
        }
    };

    this.load = function(dvORtab, id, geometry,formName){
        this.loadedID=id;
        if(typeof(dvORtab)=="string")dvORtab=vjDV[dvORtab];
        if(formName)this.formName=formName;
        this.viewers.length=0;
        this.viewersToAdd=[];
        this.loaded=true;
        this.constructed=false;
        this.current_dvORtab=dvORtab.obj;


        this.formNames='';
        var formNode=gAncestorByTag(gObject(dvORtab.obj.name),"form");
        if(formNode)
            this.formName=formNode.attributes['name'].value;


        this.viewers.length=0;

        this.addviewer('InDelCount', new vjDemoAIndelsCountView ({
            data:'Indels'
        }));


        this.addviewer('lengthwiseCount', new vjDemoLengthwiseCountView ({
            data:'ACGT'
        }));

    };


    this.construct=function(stickytabs){

        if(!this.loaded || !this.current_dvORtab)return;

        this.constructed=true;

        if(this.current_dvORtab.tabs){
            this.current_dvORtab.addTab("InDel count","graph",[this.viewers['InDelCount']]);
            this.current_dvORtab.addTab("positional ATGC count","length",[this.viewers['lengthwiseCount']]);

        }
        else{
            this.current_dvORtab.remove();
            this.viewersToAdd=this.current_dvORtab.add(this.viewersToAdd).viewers;
        }
        this.current_dvORtab.render();
        this.current_dvORtab.load('rerender');
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
