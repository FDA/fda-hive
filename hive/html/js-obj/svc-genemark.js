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
javaScriptEngine.include("js/vjAlignmentView.js");

vjHO.register('svc-genemark').Constructor=function ()
{

    if(this.objCls)return;

    if (!this.defaultDownloadWildcard)
        this.defaultDownloadWildcard = "*.{gff,out,blast_out}";

    vjHiveObjectSvcBase.call(this);

    this.resolution=200;

    this.fullview=function(node,dv)
    {
        this.mode='fullview';
        this.query=node.query;
        this.loadedID=node.id;
        this.create(dv,node);
    };

    this.customizeSvcDownloads = function(obj) {
        this.parent["svc"].customizeDownloads(this.downloadViewerDescriptions, this.defaultDownloadWildcard);
    };

    this.preview = function(node,dv)
    {
        this.parent.preview("svc", node, dv, "vjObj['"+this.objCls+"'].customizeSvcDownloads();");
        if(!node.status || parseInt(node.status)<5) return;
        this.mode='preview';
        this.create(dv,node.id);
    };

    this.typeName="svc-genemark";


    this.objCls="obj-"+this.typeName+Math.random();
    vjObj.register(this.objCls,this);


    this.active_url=new Object();
    this.inactive_url=new Object();
    this.viewersToAdd=new Array();
    this.viewers=new Object();

    this.colorsACGT=['#006AFF','#95BE4D','#556B2F', '#000080'];
    this.idNulldsName="default";
    this.viewers.length=0;

    this.addUrlSet({
            'blast-table' :{
                title: "Retrieving text from blast output file",
                active_url: "static:
                objs:"ids"
            },
            'blastTextOutput' : {
                title: "Retrieving text from blast output file",
                active_url: "static:
            },
            'sequence' :{
                active_url: "static:
                objs:"ids"
            },
            'help' : {
                active_url:"http:
                inactive_url:"http:
                title: "Genemark: building help page"
            }
    });

    this.create = function(dv, node) {
        this.load(dv, node);
        this.construct(dv, node.id);
    };

    this.load = function(dvORtab, node, geometry,formName){
        this.loadedID=node.id;
        if(typeof(dvORtab)=="string")dvORtab=vjDV[dvORtab];
        if(formName)this.formName=formName;
        this.viewers.length=0;
        this.viewersToAdd=[];
        this.loaded=true;
        this.constructed=false;
        this.dvresults=dvORtab.obj.length ? dvORtab.obj[0] : dvORtab.obj;
        this.dvinforesults=dvORtab.obj.length ? dvORtab.obj[1] : null;

        this.formName='';
        var formNode = gAncestorByTag(gObject(this.dvresults.name),"form");
        if(formNode)
            this.formName=formNode.attributes['name'].value;


        this.viewers.length=0;



        if (this.mode == "preview")
            return;


        var listcontrolviewer=new vjAlignmentHitListControl({
            data:'blast-table',
            formName: this.formName,
            width:'100%',
            selectCallback: "function:vjObjFunc('rowDisplay','" + this.objCls + "')",
            isok:true
        });
        listcontrolviewer[1].cols=[
                           { name: 'query_row', title: 'row', hidden: true},
                           { name: 'query_name', title: 'Input Sequence', maxTxtLen: 32 },
                           { name: 'query_start', title: 'Start', type: 'largenumber'},
                           { name: 'query_end', title: 'End', type: 'largenumber' },
                           { name: 'sub_name', title: 'Homologous', maxTxtLen: 95 },
                           { name: 'length_align', title: "length", hidden: false },
                           { name: 'offset', hidden: true }
                     ];

        this.addviewer('blast-pagoutput,blast-table',listcontrolviewer );

        this.addviewer ("blast-text", new vjTextView({
            data : "blastTextOutput",
            formObject : formName,
            defaultEmptyText : 'select a row to show content',
            geometry : {
                width : '100%'
            },
            isok : true
        }));

        this.addviewer("sequenceTable", new vjTextView ({
            data: "sequence",
            cols: [
                { name: '#', title: 'row #', maxTxtLen: 32 },
                { name: 'len', title: 'length', type: 'largenumber',  hidden: true },
                { name: 'rpt', title: 'rpt', type: 'largenumber',  hidden: true },
                { name: 'id', hidden: true },
                { name: 'seq', title: "interval sequence selected"},
            ],
            isok: true
        }));

        this.addviewer('help', new vjHelpView ({
            data: 'help',
            formName: formName,
            isok:true
        }));



        this.addDownloadViewer(this.downloadViewerDescriptions);
        if (node.file) {
            this.getDS('blast-table').reload("qpbg_tblqryx4:
        }
        else
            this.getDS('blast-table').reload("qpbg_tblqryx4:

    };


    this.construct=function(dv, id){

        if(!this.loaded || !this.dvresults) return;

        this.constructed=true;


        if(this.mode=='preview'){

        }
        else {
            this.dvresults.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers['blast-pagoutput'],this.viewers['blast-table']]);
            this.dvresults.addTab(this.tabs.names[1], this.tabs.icons[1], [this.viewers["download"]]);

            this.dvinforesults.addTab(this.tabs.names[2], this.tabs.icons[2], [this.viewers["blast-text"]]);
            this.dvinforesults.addTab(this.tabs.names[3], this.tabs.icons[3], [this.viewers["sequenceTable"]]);
            this.dvinforesults.addTab(this.tabs.names[4], this.tabs.icons[4], [this.viewers["help"]]);

            this.dvresults.render();
            this.dvinforesults.render();
            this.dvresults.load('rerender');
            this.dvinforesults.load('rerender');
        }

    };

    this.rowDisplay = function(viewer, node){

        this.referenceNode = node;
        var blastOffset = node.offset;
        var fileSizeView = 2500;
        
        if (node.file) {
            this.getDS('blastTextOutput').reload("http:
        } else {
            this.getDS('blastTextOutput').reload("http:
        }
        this.getDS('sequence').reload("http:

        this.dvinforesults.render();
        this.dvinforesults.load('rerender');
    }

    this.tabs = {
            names: ["Genemark annotations", "Downloads", "Blast Evidence", "Sequence", "help"],
            icons: ["list", "table", "table", "table", "help"]
        };

    this.downloadViewerDescriptions = {
        "blastx.blast_out" : "BlastX output file",
        "qp-blastp.out" : "BlastP output file",
        "genmark_sequence.gff" : "Genemark outuput file"
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
