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

vjHO.register('svc-velvet').Constructor = function() {

    if (this.objCls)
        return;

    this.typeName = "svc-velvet";
    this.objCls = "obj-svc-velvet" + Math.random();
    vjObj.register(this.objCls, this);
    
    
    this.dsQPBG_digest = vjDS.add('preparing to archive', 'ds' + this.objCls
            + '_QPBG_digest', 'static:
        func : this.onArchiveSubmit,
        obj : this
    });

    this.fullview = function(node, dv) {
        this.load(dv, node);
        if (this.onFullviewLoadCallback) {
            funcLink(this.onFullviewLoadCallback, this);
        }

        this.construct();
        if (this.onFullviewRenderCallback) {
            funcLink(this.onFullviewRenderCallback, this);
        }
        return this;
    };

    this.preview = function(node, dv) {
        this.parent.preview('svc', node, dv);
        if (!node.status || parseInt(node.status) < 5)
            return;
        this.load(dv, node.id);
        if (this.onPreviewLoadCallback) {
            funcLink(this.onPreviewLoadCallback, this);
        }

        this.construct();
        if (this.onPreviewRenderCallback) {
            funcLink(this.onPreviewRenderCallback, this);
        }
        return this;
    };

    this.mobileview = function(node, dv) {
        this.parent.mobileview('svc', node, dv);
        this.load(dv, node.id);
        if (this.onMobileLoadCallback) {
            funcLink(this.onMobileLoadCallback, this);
        }

        this.construct();
        if (this.onMobileRenderCallback) {
            funcLink(this.onMobileRenderCallback, this);
        }
        return this;
    };

    this.urlSet = {
        'hitlist' : {
            active_url : "http:
                    + (this.subsetCount ? "&childProcessedList="
                            + this.subsetCount : ""),
            title : "Retrieving list of hits"
        },
        'histograms' : {

            active_url : "qpbg_tblqryx4:
                    + vjDS.escapeQueryLanguage(JSON.stringify([ {
                        op : "filter",
                        arg : {
                            cols : [ 1, 2, 3 ],
                            method : "range",
                            value : {
                                min : 1,
                                exclusive : true
                            }
                        }
                    } ])) + "&resolution=" + this.graphResolution,
            title : "Building histogram"
        },
        'help_histograms': {
            active_url: "http:
            doNotChangeMyUrl : true,
            title: "Alignment histogram help"
        },
        'alView' : {
            active_url : "http:
            title : "Preparing alignments",
            header : "Element #,#,Direction,Start,Alignment,End,Sequence,Repeats,Position,PosHigh,Motif Start,Motif End"
        },
        'alStack' : {
            active_url : "http:
            title : "Visualizing alignments in stack"
        },
        'alMatch' : {
            active_url : "http:
            title : "Fetching alignments"
        },
        'alMutBias' : {
            active_url : "http:
            header : "Position,Count-A,Count-C,Count-G,Count-T",
            title : "Creating mutation bias diagram"
        },
        'downloads' : {
            active_url : "static:
            title : "infrastructure: Creating download menu"
        },
        'downloadAll' : {
            active_url : "static:
                    + "Hit list,download,ico-file,hitlist,&qty=-1,\n"
                    + "Unaligned Reads,download,dnaold,alFasta,&backend=1,all\n"
                    + "Aligned Reads,download,dnaold,alFasta,&qty=-1,\n"
                    + "Alignments,download,ico-file,alView,&qty=-1,\n"
                    + "Alignments in non-standard SAM format,download,ico-file,alSam,&qty=-1&useOriginalID=1,\n"
                    + "Alignments in SAM format,download,ico-file,alSam,&qty=-1,\n"
                    + "Hit Table,download,ico-file,alMatch,&qty=-1,",
            title : "infrastructure: Creating download menu"
        },
        'help' : {
            active_url : "http:
            inactive_url : "http:
            doNotChangeMyUrl : true,
            title : "Infrastructure: Creating help"
        }

    };

    this.construct = function() {
        if (!this.loaded || !this.current_dvORtab)
            return;
        this.constructed = true;

        if (this.mode == 'mobileview') {
            var dv = this.current_dvORtab.obj;
            var origNumofTabs = 2;
            this.dvname = dv.name;
            this.dvinfo = dv.name;
            dv.addTab("stack", "list", [ this.viewers['hitlist'],
                    this.viewers['hitpie'], this.viewers['alignment'] ]).columns = 1;
            changeTabs("Hit List", "Pie Chart", "Alignment");
            dv.render();
            dv.load('rerender');
            running();

        }

        else if (this.mode == 'preview') {
            var dv = this.current_dvORtab.obj;
            var origNumofTabs = dv.tabs.length;
            this.dvname = dv.name;
            this.dvinfo = dv.name;
            dv.addTab("stack", "list", [ this.viewers['hitlist_panel'],
                    this.viewers['stack_panel'], this.viewers['hitlist'],
                    this.viewers['stack'] ]).columns = 2;
            dv.addTab("histogram", "area", [ this.viewers['histograms_panel'],
                    this.viewers['histograms'] ]);
            dv.render();
            dv.load('rerender');
        } else {
            this.dvname = this.current_dvORtab[0].name;
            this.dvinfo = this.current_dvORtab[1].name;

            this.current_dvORtab[0].addTab("piechart", "pie",
                    [ this.viewers['hitpie'] ]);
            this.current_dvORtab[0].addTab("list", "list", [
                    this.viewers['hitlist_panel'], this.viewers['hitlist'] ]);
            this.current_dvORtab[0].addTab("downloads", "table",
                    [ this.viewers['downloadAll'] ]);
            this.current_dvORtab[1].addTab("alignments", "dna",
                    [ this.viewers['alignment_panel'],
                            this.viewers['alignment'] ]);
            this.current_dvORtab[1].addTab("stack", "dna",
                    [ this.viewers['stack_panel'],
                            this.viewers['stack_mutation_bias'],
                            this.viewers['stack'] ]);
            this.current_dvORtab[1].addTab("hit tables", "table",
                    [ this.viewers['hit_table_panel'],
                            this.viewers['hit_table'] ]);
            this.current_dvORtab[1].addTab("histogram", "area", [
                    this.viewers['histograms_panel'],
                    this.viewers['histograms'] ]);
            this.current_dvORtab[1].addTab("downloads", "table",
                    [ this.viewers['downloads'] ]);
            this.current_dvORtab[1].addTab("help", "help",
                    [ this.viewers['help'] ]);

            this.current_dvORtab[0].selected = 1;
            this.current_dvORtab[0].render();
            this.current_dvORtab[1].render();
            this.current_dvORtab[0].load('rerender');
            this.current_dvORtab[1].load('rerender');
        }

        this.viewers["hitlist"].callbackRendered = "function:vjObjFunc('onLoadedHitList','"
                + this.objCls + "')";
    };

    this.load = function(dvORtab, node) {
        var id = node.id;
        if (this.mode == 'mobileview') {
            this.loadedID = id;
            this.formName = '';
            var formNode = gAncestorByTag(gObject(dvORtab.obj.name), "form");
            if (formNode)
                this.formName = formNode.attributes['name'].value;
            this.loaded = true;
            this.constructed = false;
            this.current_dvORtab = dvORtab;

            this.dvname = this.current_dvORtab.obj.name;
            this.dvinfo = this.current_dvORtab.obj.name;

            this.addviewer('hitlist_panel,hitlist',
                    new vjAlignmentHitListControlMobile({
                        data : 'hitlist',
                        formName : this.formName,
                        columnToDisplay : [

                        {
                            name : new RegExp(/^Hits$/),
                            type : 'largenumber',
                            hidden : false

                        }, {
                            name : 'Reference',
                            maxTxtLen : 15
                        }, {
                            name : 'Density',
                            maxTxtLen : 15
                        } ],
                        width : '100%',

                        selectCallback : "function:vjObjFunc('onSelectedHitListItem','"
                                + this.objCls + "')",
                        isok : true
                    }));

            this.addviewer('hitpie', 
                    new vjAlignmentHitPieView({
                        data : 'hitlist',
                        formName : this.formName[0],
                        selectCallback : "function:vjObjFunc('onSelectedHitListItem','"
                                + this.objCls + "')",
                        isok : true
                    }));

            this.addviewer('alignment_panel,alignment', 
                    new vjAlignmentControl({
                        data : 'alView',
                        formName : this.formName[1],
                        isok : true
                    }));
        }

        else if (this.mode == 'preview') {

            this.loadedID = id;
            this.formName = '';
            var formNode = gAncestorByTag(gObject(dvORtab.obj.name), "form");
            if (formNode)
                this.formName = formNode.attributes['name'].value;

            this.loaded = true;
            this.constructed = false;
            this.current_dvORtab = dvORtab;

            this.dvname = this.current_dvORtab.obj.name;
            this.dvinfo = this.current_dvORtab.obj.name;

            this.addviewer('hitlist_panel,hitlist',
                    new vjAlignmentHitListControl({
                            data : 'hitlist',
                            formName : this.formName,
                            width : '20%',
                            selectCallback : "function:vjObjFunc('onSelectedHitListItem','"
                                    + this.objCls + "')",
                            isok : true
                        }));

            this.addviewer('stack_panel,stack_mutation_bias,stack',
                    new vjAlignmentStackControl({
                        data : {
                            stack : 'alStack',
                            basecallBias : 'dsVoid'
                        },
                        formName : this.formName,
                        width : "100%",
                        isok : true
                    }));

            this.addviewer('histograms_panel,histograms',
                    new vjAlignmentHistogramControl({
                        data : 'histograms',
                        dataHelp: 'help_histograms',
                        formName : this.formName,
                        isok : true
                    }));

        } else {

            this.loadedID = id;
            this.formName = [ '', '' ];
            var formNode = gAncestorByTag(gObject(dvORtab.obj[0].name), "form");
            if (formNode)
                this.formName[0] = formNode.attributes['name'].value;
            formNode = gAncestorByTag(gObject(dvORtab.obj[1].name), "form");
            if (formNode)
                this.formName[1] = formNode.attributes['name'].value;

            this.loaded = true;
            this.constructed = false;
            this.current_dvORtab = dvORtab.obj;

            this.dvname = this.current_dvORtab[0].name;
            this.dvinfo = this.current_dvORtab[1].name;
            
            vjDS.add("Loading: ","dsIonAnnot","http:
            var getIonAnnotUrl = "http:
            vjDS["dsIonAnnot"].clear_callbacks();
            vjDS["dsIonAnnot"].register_callback({
                func: function(param, data) {
                    
                    var panel = vjDV.dvHitListViewer.tabs.list.viewers[0];
                    panel.rows = panel.rows.concat (
                            {name:"Additional_Info",title:"Extended-hit-list",url:""  }
                    );
                    if (panel.ionArr == undefined) panel.ionArr = new Array();
                    var tbl = new vjTable(data, 0, vjTable_propCSV);
                    if (!tbl.rows.length) return;
                    for (var ir=0; ir<tbl.rows.length; ++ir){
                        var element_id = tbl.rows[ir].id;
                        var element_name = tbl.rows[ir].name;
                        panel.rows = panel.rows.concat({name:"Additional_Info/" +  element_id,title: element_name, type:"checkbox", url:retrieve_additionalInfo, isChecked:0});
                    }
                    if (node!=undefined && node.profiler!=undefined && node.profiler.length >1 ) {
                        panel.profilerID ="";
                        panel.rows = panel.rows.concat (
                                {name:"Additional_Info/" + node.profiler,title:"this is profiler",type:"checkbox",url:retrieve_additionalInfo ,order:998,isChecked :0, isProfiler:1 }
                        );
                    }
                    panel.rows = panel.rows.concat (
                            {name:"Additional_Info/submit",title:"submit",type:"button",url:retrieve_additionalInfo ,order:999  }
                    );
                 }
            });
            vjDS["dsIonAnnot"].reload(getIonAnnotUrl,true);
            
            this.addviewer('hitpie', 
                    new vjAlignmentHitPieView({
                            data : 'hitlist',
                            formName : this.formName[0],
                            selectCallback : "function:vjObjFunc('onSelectedHitListItem','"
                                    + this.objCls + "')",
                            isok : true
                        }));

            this.addviewer('hitlist_panel,hitlist',
                    new vjAlignmentHitListControl({
                        data : 'hitlist',
                        loadedID: this.loadedID,
                        formName : this.formName[0],
                        ionAnnotList:"eval:vjHO['"+this.typeName+"'].getDS('dsIonAnnot').name",
                        checkable : this.checkable,
                        selectCallback : "function:vjObjFunc('onSelectedHitListItem','"
                                + this.objCls + "')",
                        checkCallback : "function:vjObjFunc('onCheckReferenceGenomes','"
                                + this.objCls + "')",
                        isok : true
                    }));

            this.addviewer('downloadAll',
                    new vjAlignmentDownloadsView({
                        data : 'downloadAll',
                        formName : this.formName[0],
                        geometry : 0,
                        selectCallback : "function:vjObjFunc('onPerformReferenceOperation','"
                                + this.objCls + "')",
                        isok : true
                    }));

            this.addviewer('histograms_panel,histograms',
                    new vjAlignmentHistogramControl({
                        data : 'histograms',
                        dataHelp: 'help_histograms',
                        formName : this.formName[1],
                        isok : true
                    }));

            this.addviewer('alignment_panel,alignment', 
                    new vjAlignmentControl({
                        data : 'alView',
                        formName : this.formName[1],
                        isok : true
                    }));

            this.addviewer('stack_panel,stack_mutation_bias,stack',
                    new vjAlignmentStackControl({
                        data : {
                            stack : 'alStack',
                            basecallBias : 'alMutBias'
                        },
                        formName : this.formName[1],
                        isok : true
                    }));

            this.addviewer('hit_table_panel,hit_table',
                    new vjAlignmentMatchControl({
                        data : 'alMatch',
                        formName : this.formName[1],
                        isok : true
                    }));

            this.addviewer('downloads',
                    new vjAlignmentDownloadsView({
                        data : 'downloads',
                        formName : this.formName[1],
                        maxTxtLen : 50,
                        selectCallback : "function:vjObjFunc('onPerformReferenceOperation','"+ this.objCls + "')",
                        isok : true
                    }));

            this.addviewer('help', new vjHelpView({
                data : 'help',
                formName : this.formName[1],
                isok : true
            }));

        }
    };

    this.onPerformReferenceOperation = function(viewer, node, ir, ic) {
        var isArch = (viewer.tblArr.hdr[ic].name == "arch");
        var isDown = (viewer.tblArr.hdr[ic].name == "down");
        var oper = node.operation;
        var args = node.arguments;
        try {
            var params = JSON.parse(node.params);
        } catch (e) {
            params = node.params;
        }
        var url;

        if (oper == 'alStack' || oper == 'alView' || oper == 'alMatch'
                || oper == 'alMutBias') {
            var operDS = this.getDS(oper);
            if (!operDS)
                return;
            url = operDS.url;
            url = urlExchangeParameter(url, "cnt", '0');
            if (args)
                url += args;
            url = url.substring(7);
        } else
            url = this.makeReferenceOperationURL(viewer, node, node.operation,
                    node.arguments, params);
        url = urlExchangeParameter(url, "down", '1');

        if (isDown) {
            if (docLocValue("backend", 0, url)) {
                var saveAs = "o" + this.loadedID + "-" + oper + "-"
                        + this.referenceID + ".fa";
                this.dsQPBG.reload("qpbg_http:
                    loadmode : "download",
                    saveas : saveAs
                });
            } else {
                document.location = url;
            }
        } else if (isArch) {
            url = urlExchangeParameter(url, "arch", 1);
            var dstName = node.data + " (" + this.loadedID + ")";
            var ext = "-";
            switch (node.arch) {
            case "dna":
                ext = "genome";
                dstName += ".fasta";
                break;
            case "dnaold":
                ext = "fasta";
                dstName += ".fasta";
                break;
            default:
                ext = "-";
            }

            url = urlExchangeParameter(url, "arch_dstname", dstName);
            url = urlExchangeParameter(url, "ext", ext);
            if (url.indexOf("qpbg") == 0)
                this.dsQPBG_digest.reload(url, true);
            else
                this.dsQPBG_digest.reload("qpbg_http:
        }
    };

    this.makeReferenceOperationURL = function(viewer, node, oper, args, params) {
        if (oper == 'hitlist') {
            var url = urlExchangeParameter(
                    this.viewers["hitlist"].getData(0).url.substring(7), "cnt",
                    '0');
            return url;
        }


        var url = "?cmd=" + oper + "&objs=" + this.loadedID;
        if (this.reqID)
            url += "&req=" + this.reqID;

        var referenceID = params && params.referenceID != undefined ? params.referenceID
                : this.referenceID;
        if (params === 'all')
            referenceID = 0;
        if (parseInt(referenceID) == 0) {
            url += "&found=0";
        } else if (referenceID == '+') {
            url += "&found=1";
        } else {
            url += "&mySubID=" + referenceID + "&found=1";
        }
        url += args;
        return url;
    };

    this.onSelectedHitListItem = function(viewer, node) {
        if (!isNumber(node.id))
            return 0;
        this.referenceNode = node;
        this.referenceID = node.id;

        if (this.mode == 'preview') {
            if (!this.isUnalignedSelected(node)) {
                this.getDS('alView').reload(
                        "http:
                                + this.makeReferenceOperationURL(viewer, node,
                                        'alView') + "&cnt=20", false);
                this.getDS('alStack').reload(
                        urlExchangeParameter(this.urlSet['alStack'].active_url,
                                "objs", this.loadedID)
                                + "&mySubID=" + this.referenceID + "&cnt=20",
                        false);
                this.getDS('histograms').reload(
                        urlExchangeParameter(
                                this.urlSet['histograms'].active_url, "objs",
                                this.loadedID), false);
                vjDV.locate(this.dvinfo + "._active.").load();
            }
        } else {
            var viewer = vjDV.locate(this.dvinfo + ".downloads.0");
            if (viewer) {
                var t = "data,down,arch,operation,arguments,params\n";
                if (this.isUnalignedSelected(node)) {
                    t += "Unaligned Reads,download,dnaold,alFasta,&backend=1,"
                            + quoteForCSV(JSON.stringify({
                                referenceID : 0
                            })) + "\n";
                    viewer.prefixHTML = "There were <b>"
                            + viewer.formDataValue("" + node.Hits,
                                    'largenumber', node)
                            + "</b> sequences that did not match any of the reference genomes.";
                    this.viewInUnalignedMode(true);
                } else {
                    this.viewInUnalignedMode(false);
                    viewer.prefixHTML = "<b>"
                            + viewer.formDataValue("" + node.Hits,
                                    'largenumber', node)
                            + "</b> sequences were aligned to <b>"
                            + node.Reference + "</b>";
                    t += "Aligned Reads,download,dnaold,alFasta,,\n"
                            + "Hit Table,download,ico-file,alMatch,,\nAlignments,download,ico-file,alView,,\n"
                            + "Alignments to currently selected reference in non-standard SAM format,download,ico-file,alSam,&useOriginalID=1,\n"
                            + "Alignments to currently selected reference in SAM format,download,ico-file,alSam,,\n"
                            + "Stack,download,ico-file,alStack,,\nStack of non perfect alignments,download,ico-file,alStack,&alNonPerfOnly=1,\n"
                            + "Stack of alignments with mutation on specified position,download,ico-file,alStack,&alVarOnly=1,\n"
                            + "Mutation bias,download,ico-file,alMutBias,,\n";

                    if (this.algoProc) {
                        var filelist = this.algoProc.viewer.attached("file");
                        var tAux = "";
                        for (ff in filelist) {
                            if (ff.indexOf(".csv") == -1
                                    && ff.indexOf(".txt") == -1
                                    && ff.indexOf(".sam") == -1
                                    && ff.indexOf(".fpkm_tracking") == -1
                                    && ff.indexOf(".gtf") == -1
                                    && ff.indexOf(".ma") == -1)
                                continue;
                            tAux += ff + ",download,,objFile,&ids="
                                    + this.loadedID + "&filename=" + ff + ",\n";
                        }
                        if (tAux.length)
                            t += "-Auxiliary information,,,,\n" + tAux;
                    }
                }

                this.getDS('downloads').reload("static:
            }
            if (!this.isUnalignedSelected(node)) {
                this.getDS('alView').reload(
                        "http:
                                + this.makeReferenceOperationURL(viewer, node,
                                        'alView') + "&cnt=20", false);
                this.getDS('alMatch').reload(
                        "http:
                                + this.makeReferenceOperationURL(viewer, node,
                                        'alMatch') + "&cnt=20", false);
                this.getDS('alStack')
                        .reload(
                                "http:
                                        + this.makeReferenceOperationURL(
                                                viewer, node, 'alStack')
                                        + "&mySubID=" + this.referenceID
                                        + "&cnt=20&alHigh=50", false);
                this.getDS('alMutBias').reload(
                        "http:
                                + this.makeReferenceOperationURL(viewer, node,
                                        'alMutBias') + "&cnt=20", false);
                this.getDS('histograms').reload(
                        urlExchangeParameter(
                                this.urlSet['histograms'].active_url, "objs",
                                this.loadedID), false);
                vjDV.locate(this.dvinfo + "._active.").load();
                vjDV.locate(this.dvname + "._active.").load();
            }
        }

        this.selfUpdate = 1;
        if (this.callbackSelected)
            funcLink(this.callbackSelected, this.viewer, node);
        this.selfUpdate = 0;
    };

    this.onCheckReferenceGenomes = function(viewer, node) {

        this.selfUpdate = 1;
        if (this.callbackChecked)
            funcLink(this.callbackChecked, viewer, this);
        this.selfUpdate = 0;
    };

    this.isUnalignedSelected = function(row) {
        var unalignedSelected = false;
        var nodeID = row.id;
        if (isNumber(nodeID) && !Int(nodeID))
            unalignedSelected = true;

        return unalignedSelected;
    };

    this.viewInUnalignedMode = function(mode) {
        this.current_dvORtab[1].tabs[0].invisible = mode;
        this.current_dvORtab[1].tabs[1].invisible = mode;
        this.current_dvORtab[1].tabs[2].invisible = mode;
        this.current_dvORtab[1].tabs[3].invisible = mode;
        this.current_dvORtab[1].tabs[0].hidden = mode;
        this.current_dvORtab[1].tabs[1].hidden = mode;
        this.current_dvORtab[1].tabs[2].hidden = mode;
        this.current_dvORtab[1].tabs[3].hidden = mode;
        if (mode) {
            this.current_dvORtab[1].selected = 4;
        }
        this.current_dvORtab[1].render();
    };

    this.onLoadedHitList = function() {
        var rows = this.viewers["hitlist"].tblArr.rows;

        if (this.autoClickedReference !== false
                || this.autoClickedReference !== undefined
                || this.autoClickedReference !== null) {
            var nodeID = rows[this.autoClickedReference].id;
            if (!isNumber(nodeID))
                this.autoClickedReference = 0;
            this.viewers["hitlist"]
                    .mimicClickCell(this.autoClickedReference, 0);
        }
    };

    this.reload = function(loadedID, reqid, autoClickedReference)
    {
        if (loadedID)
            this.loadedID = loadedID;
        if (!this.loadedID)
            return;

        var url = "http:
                + "&start=0&cnt=50";
        if (this.profilerList)
            url += "&childProcessedList=" + this.subsetCount;
        if (reqid) {
            this.reqID = reqid;
            url += "&req=" + this.reqID;
        }
        if (autoClickedReference) {
            this.autoClickedReference = autoClickedReference;
            this.viewers["hitlist"].callbackRendered = "function:vjObjFunc('onLoadedHitList','"
                    + this.objCls + "')";
        }
        this.getDS('hitlist').reload(url, true);

    };

    this.update = function(proc) {
        this.algoProc = proc;
        var checkedIDS = this.viewers["hitlist"].accumulate("node.checked",
                "node.id");
        return isok(checkedIDS) ? 1 : 0;
    };

    if (this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
    
};
