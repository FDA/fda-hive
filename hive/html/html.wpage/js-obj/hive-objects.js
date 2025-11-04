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
function vjHiveObjects() {
    this.length = 0;
    this.register = function(hiveObjType, hiveObject) {
        if (this[hiveObjType])
            return this[hiveObjType];

        if(!hiveObject)
            hiveObject=new Function;
        this[hiveObjType]=hiveObject;
        this[this.length]=this[hiveObjType];
        this[hiveObjType].parent=this;
        ++this.length;
        vjRegisterJavaScriptEngine(hiveObjType, this);

        vjHiveObjectBase.call(hiveObject);
        return this[hiveObjType];
    };

    this.get=function (typeName,attr,category)
    {
        var type=this[typeName];
        if(!type)
            return null;
        if(!type[attr]){
            if(type.inherit) {
                type=this[type.inherit];
        }
            if(!type || !type[attr])
                return null;
        }
        if(!type[attr][category]){
            if(typeof(type[attr])=='string' && type[attr].indexOf('==')>=0)
                return type[attr].replace(/==/g,typeName);
            return type[attr];
        }
        if(typeof(type[attr])=='string' && type[attr][category].indexOf('==')>=0)
            return type[attr][category].replace(/==/g,category);
        return type[attr][category];

    };

    this.mobileview=function (typeName, node, params,callbackDone,callbackConstructed)
    {
        this.mode='mobileview';
        params.node=node;
        return this.execute(typeName,"node=params.node;node._selected_nodes=params.node._selected_nodes;obj['"+typeName+"'].mode='"+this.mode+"';if(obj['"+typeName+"'].mobileview)obj['"+typeName+"'].mobileview(node,params);else if(obj['"+typeName+"'].defaultView)obj['"+typeName+"'].defaultView(node,params);",params,callbackDone,callbackConstructed);
    };

    this.preview=function (typeName, node, params,callbackDone,callbackConstructed)
    {
        this.mode='preview';
        params.node=node;
        return this.execute(typeName,"node=params.node;node._selected_nodes=params.node._selected_nodes;obj['"+typeName+"'].mode='"+this.mode+"';if(obj['"+typeName+"'].preview)obj['"+typeName+"'].preview(node,params);else if(obj['"+typeName+"'].defaultView)obj['"+typeName+"'].defaultView(node,params);",params,callbackDone,callbackConstructed);
    };
    this.fullview=function (typeName, node, params,callbackDone,callbackConstructed)
    {
        params.node=node;
        this.mode='fullview';
        return this.execute(typeName,"node=params.node;node._selected_nodes=params.node._selected_nodes;obj['"+typeName+"'].mode='"+this.mode+"';if(obj['"+typeName+"'].fullview)obj['"+typeName+"'].fullview(node,params);else if(obj['"+typeName+"'].defaultView)obj['"+typeName+"'].defaultView(node,params);",params,callbackDone,callbackConstructed);
    };
    this.setUpJsonForAlgo = function (typeName, node, params,callbackDone,callbackConstructed)
    {
        params.node=node;
        this.mode='setUpJsonForAlgo';
        return this.execute(typeName,"node=params.node;node._selected_nodes=params.node._selected_nodes;obj['"+typeName+"'].mode='"+this.mode+"';if(obj['"+typeName+"'].fullview)obj['"+typeName+"'].setUpJsonForAlgo(node,params);else if(obj['"+typeName+"'].defaultView)obj['"+typeName+"'].defaultView(node,params);",params,callbackDone,callbackConstructed);
    };


    this.execute=function( typeName, code, params, callbackDone, callbackConstructed)
    {
        var t=this[typeName];
        if(!typeName || !t)
                return ;
        if(callbackDone)
            code+=callbackDone;
        obj=this;

        if( t.isConstructed==true){
                eval(code);
                return t;
        }

        if(t.Constructor){
            t.Constructor();
            if( callbackConstructed ) {
                funcLink(callbackConstructed, t);
            }
            t.isConstructed=true;
            var returned = eval(code);
            if (returned)
                t.returned = returned;
            return t;

        }
        else if(t.inherit || t.jsFile ){
            this[typeName].params=params;

            var ttjs= (t.inherit ) ? t.inherit : (t.jsFile==true ? typeName : t.jsFile );
            var jsdir = t.inherit && this[t.inherit] ? this[t.inherit].jsDir : t.jsDir;

            var toExecute="javascript:obj['"+ttjs+"'].Constructor.call(obj['"+typeName+"']);var params=obj['"+typeName+"'].params;"+code;

            if(this[ttjs] && this[ttjs].Constructor){
                eval(toExecute.substring(11));
                return t;
            }

            vjHiveTypeExecute(ttjs, toExecute, this, jsdir);
        }
        t.isConstructed=true;

        return t;
    };




}

function vjHiveObjectBase() {

    this.onObjectContructionCallback = undefined;

    this.onMobileLoadCallback = undefined;

    this.onPreviewLoadCallback = undefined;

    this.onFullviewLoadCallback = undefined;

    this.onMobileRenderCallback = undefined;

    this.onPreviewRenderCallback = undefined;

    this.onFullviewRenderCallback = undefined;

    this.dsName = {};

    
    this.addviewer = function(viewname, viewobj) {
        
        if(!this.viewers) {
            this.viewers=new Object();
            this.viewers.length=0;
        }
        
        if(!this.viewersToAdd)
            this.viewersToAdd=[];

        var vlist = verarr(viewobj);
        var nlist = viewname.split(",");

        for ( var iv = 0; iv < vlist.length; ++iv) {

            var ddd = verarr(vlist[iv].data);
            vlist[iv].data = new Array();

            for ( var id = 0; id < ddd.length; ++id) {
                var vn = ddd[id];

                if (vn != "dsVoid") {
                    var dsname = this.makeDSName(vn, true);
                    vlist[iv].data.push(dsname);
                }
                else
                    vlist[iv].data.push(vn);

            }


            this.viewers[nlist[iv]] = vlist[iv];
            this.viewers[this.viewers.length] = vlist[iv];
            this.viewersToAdd.push(this.viewers[this.viewers.length]);
            ++this.viewers.length;
        }

        for (var nm in this.dsName) {
            var us = this.urlSet[nm];

            var ds = vjDS[this.dsName[nm]];

            if (!ds) {
                ds = this.makeDS(nm);
            }
            if(us.isSeries){
                for ( var iv = 0; iv < vlist.length; ++iv) {
                    if(!vlist[iv].plots || !vlist[iv].plots.length) continue;
                    var seriesArr=vlist[iv].plots[0].collection;
                    for(var is=0 ; is<seriesArr.length; ++is){
                        var series=seriesArr[is];
                        if(nm!=series.name)continue;
                        series.changeDS(vjDS[this.dsName[series.name]]);
                    }
                }
            }
        }
    };

    this.makeAllDS = function(force) {
        for (var i in this.urlSet) {
            this.makeDS(i,force);
        }
    };
    
    this.makeDSName = function(nm, force) {
        if (this.dsName[nm] && !force)
            return this.dsName[nm];

        var dsSuffix = this.loadedID ? this.loadedID : (this.idNulldsName ? this.idNulldsName : 'default' );
        var dsname = ["ds", this.dvinfo, this.typeName, dsSuffix, nm].join("_");
        this.dsName[nm] = dsname;
        return dsname;
    };

    this.reloadDSobjID = function(nm, objID) {
        if ( arguments.length < 1) {
            console.error("missing argument in " + arguments.callee.name)
            return;
        }
        var ds = this.getDS(nm);
        if ( typeof(ds) == "undefined") {
            console.error("No datasource available :" + arguments.callee.name)
            return 
        }
        var us = this.urlSet[nm];
        
        if ( typeof(us) == "undefined") {
            console.error("No object url available :" + arguments.callee.name)
            return 
        }
        
        var url = ds.url;
        if ( typeof(objID) == "undefined" ) {
            url = us.inactive_url ? us.inactive_url : "static:
        } else {
            url = urlExchangeParameter(url, us.objs ? us.objs : "objs", objID);
        }
        ds.reload(url, true);
    }
    
    this.makeDS = function(nm, force) {
        var dsname = this.makeDSName(nm);

        if (vjDS[dsname] && !force)
            return vjDS[dsname];

        var url = "";
        var us = this.urlSet[nm];
        var title = us.title;

        var active = this.loadedID && !us.loadInactive;

        if (us.make_ds)
            return us.make_ds.call(this, this.dsTitle, dsname, active, this.loadedID);

        if (active) {
            url = us.active_url ? us.active_url : "static:
            if (us.make_active_url)
                url = us.make_active_url.call(this, url, this.loadedID);
        } else {
            url = us.inactive_url ? us.inactive_url : "static:
        }

        if(!us.make_active_url && !us.doNotChangeMyUrl)
            url = urlExchangeParameter(url, us.objs ? us.objs : "objs",
                    this.loadedID);

        return vjDS.add(title, dsname, url, us.callback, us.header);
    };
    
    this.getDSName = function(nm) {
        if (this.dsName[nm])
            return this.dsName[nm];
        return "";
    }

    this.getDS = function(nm) {
        if (this.dsName[nm])
            return vjDS[this.dsName[nm]];
        return vjDS["dsVoid"];
    };

    this.reload = function(loadedID) {
        if ( typeof(loadedID) == "undefined" ) {
            delete this.loadedID;
        } else {
            this.loadedID = loadedID;
        }
        
        for ( var nm in this.dsName ) {
            this.reloadDSobjID(nm,this.loadedID);
        }
    }
    
    this.reset = function (loadedID) {
        this.resetDS();
        this.resetViewers();
    };
    
    this.resetDS = function() {
        for ( var nm in this.dsName ) {
            delete vjDS[this.dsName[nm]];
        }
    };

    this.resetViewers = function(){
        for(var i in this.viewers){
            delete this.viewers[i];
        }
        for(var i in this.viewersToAdd){
            delete this.viewersToAdd[i];
        }
        if(this.viewers)
            this.viewers.length=0;
    };

    this.addUrlSet = function(set) {
        if (!this.urlSet)
            this.urlSet = {};

        for (var item in set) {
            this.urlSet[item] = set[item];
        }
    };
    
    this.defaultView = function (node,params){
        this._node = node;
        this._params = params;
    }
    
    this.inheritType = function(typeName, callbackDone) {
        var that=this.parent.execute(typeName,"",{node:""});
        var _this = this;
        function onInherit() {
            mergeObjPair(_this,that);
            switch(_this.mode) {
                case 'preview':
                    if(_this.preview)_this.preview(_this._node,_this._params);
                    break;
                case 'fullview':
                    if(_this.fullview)_this.fullview(_this._node,_this._params);
                    break;
                case 'mobileview':
                    if(_this.mobileview)_this.mobileview(_this._node,_this._params);
                    break;
                case 'setUpJsonForAlgo':
                    if(_this.setUpJsonForAlgo)_this.setUpJsonForAlgo(_this._node,_this._params);
                    break;
            }
        }
        that.onObjectContructionCallback=onInherit;
        return that;
    }
}

function vjHiveObjectSvcBase() {
    vjHiveObjectBase.call(this);

    if(!this.defaultDownloadWildcard)
        this.defaultDownloadWildcard="*.{csv,json,png,tsv,txt,fa,fq,fasta,fastq}";

    if(!this.defaultDownloadURL)
        this.defaultDownloadURL="http:

    this.urlSet = {
        "downloadables": {
            active_url: this.defaultDownloadURL,
            objs: "ids",
            header: "id,name,path,value",
            title: "Retrieving list of downloadable files"
        }
    };

    this.addDownloadViewer = function(descriptions) {
        this.addviewer('download', new vjTableView({
            debug:0,
            parsemode: vjTable_hasHeader,
            data: "downloadables" ,
            formName: this.formName,
            selectCallback: function (viewer,node,ir,col){
                if (col>1)
                    return;
                if (node.url) {
                    document.location = node.url;
                } else {
                    var idlist="";
                    if(this.all_selected_nodes && this.all_selected_nodes.length>1) {
                        idlist=this.all_selected_nodes[0].id;
                        for (var i =1 ; i <this.all_selected_nodes.length; ++i) {
                            idlist+=","+this.all_selected_nodes[i].id;
                        }
                        document.location = "?cmd=objFile&ids="+idlist+"&filename="+node.value;
                    }
                    else document.location = "?cmd=objFile&ids="+node.id+"&filename="+node.value;
                    
                }
            },
            iconSize:24,
            all_selected_nodes: this.node._selected_nodes,
            cols:[
                { name: new RegExp(/.*/), hidden:true },
                { name: "name", hidden:true },
                { name: "pretty_name", title: "name", hidden: false },
                { name: "description", hidden: false },
                { name: "icon", hidden: true }
            ],
            appendCols : [{header: {name: "icon"}, cell: "download" }, {header: {name: "pretty_name", title: "name"}}, {header: {name: "description"}}],
            bgColors:['#f2f2f2','#ffffff'],
            getPrettyName: function(filename) { return filename; },
            getDescription: this.generateDownloadViewerGetDescription(descriptions),
            precompute: function(viewer, tbl, ir) {
                var node = tbl.rows[ir];
                if (!node.value || node.name != "_file") {
                    node.hidden = true;
                    return;
                }
                if (!node.prety_name) {
                    node.pretty_name = viewer.getPrettyName(node.value);
                }
                node.cols[node.cols.length-2] = node.pretty_name;
                if (!node.description) {
                    node.description = viewer.getDescription(node.value);
                }
                node.cols[node.cols.length-1] = node.description;
            },
            preEditTable: function(viewer) {
                viewer.tblArr.sortTbl(0, 0, function(a, b) {
                    return cmpCaseNatural("" + a.name + a.value, "" + b.name + b.value);
                });
            }
        }));
    };

    this.generateDownloadViewerGetDescription = function(descriptions) {
        return function(filename) {
            if (descriptions) {
                var f;
                for (f in descriptions) {
                    if (f == filename)
                        return descriptions[f];
                }
            }

            if (!filename)
                return;

            return filename.replace(/[^.]*\./, "").toUpperCase() + " file";
        };
    };

    this.customizeDownloads = function(descriptions, fileglob) {
        this.viewers["download"].getDescription = this.generateDownloadViewerGetDescription(descriptions);

        if (fileglob) {
            var ds = this.getDS("downloadables");
            ds.reload(urlExchangeParameter(ds.url, "files", fileglob), true);
        }
    };
}

var vjHO= new vjHiveObjects();

vjHO.register('image' , {
    jsFile: true,
    icon:"?cmd=objFile&propname=icon&prefix=0&ids=$(id)"
});
vjHO.register('u-file' , {
    icon:'ico-file',
    jsFile: true
});


vjHO.register('process' , {
    inherit: 'svc',
    hideProgress: true,
    icon : 'process'
});

vjHO.register('action' , {
        icon : 'rec'
});

vjHO.register('email' , {
        icon : 'rec'
});

vjHO.register('folder' , {
        icon : 'img/folder-hive.gif'
});

vjHO.register('group' , {
    icon : 'rec'
});

vjHO.register('lab-freezer-LN' , {
    icon : 'rec'
});

vjHO.register('HIVE-experiment' , {
    icon : 'bio-experiment'
});

vjHO.register('HIVE-project' , {
    icon : 'bio-project'
});

vjHO.register('HIVE-run' , {
    icon : 'bio-run'
});

vjHO.register('HIVE-sample' , {
    icon : 'bio-sample'
});

vjHO.register('notification' , {
    icon : 'rec'
});

vjHO.register('svc' , {
    inherit: 'svc',
    icon:"process"
});

vjHO.register('svc-algorlda2' , {
    icon : 'img/scope.png',
    inherit: 'svc',
    jsDir: "js-obj-new"
});

vjHO.register('svc-algorlda' , {
    jsFile: true,
    icon:"img/64/rlda.png"
});

vjHO.register('svc-align' , {
    icon : 'img/processSvc.gif',
    inherit: 'svc-alignment',
    defaultDownloadWildcard: "*.{csv,json,png,tsv,txt,fa,fastq,fasta,fq}"
});

vjHO.register('svc-align-blat' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-align-blast' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-align-blastx' , {
    icon : '?cmd=~^dna~==.gif'
    , jsFile:true
});

vjHO.register('svc-align-tblastx' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-align-blastx'
});

vjHO.register('svc-align-bowtie2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-align-bwa' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-dna-kraken' , {
    icon : 'img/process.gif'
    , inherit: 'svc'
});

vjHO.register('svc-align-hexagon' , {
    inherit: 'svc-alignment'
    , icon : 'img/processSvc.gif',
    defaultDownloadWildcard: "*.{csv,json,png,tsv,txt,fa,fastq,fasta,fq}"
});

vjHO.register('svc-hexagon-batch' , {
    icon : 'svc-align-hexagon'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-align-magic' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-align-tophat' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment'
});

vjHO.register('svc-align-multiple' , {
    icon : 'img/processSvc.gif',
    inherit: 'svc-alignment-multiple',
});

vjHO.register('svc-alignment-multiple' , {
    icon : 'img/processSvc.gif',
    inherit: 'svc-alignment',
});

vjHO.register('svc-alignment' , {
    icon : 'img/processSvc.gif',
    defaultDownloadWildcard: "*.{csv,json,png,tsv,txt,fa,fastq,fasta,fq}"
    
});

vjHO.register('svc-align-mafft' , {
    icon : '?cmd=~^dna~==.gif',
    inherit: 'svc-alignment-multiple'
});

vjHO.register('svc-align-clustal' , {
    icon : '?cmd=~^dna~==.gif',
    inherit: 'svc-alignment-multiple'
});

vjHO.register('svc-archiver' , {
    icon : 'process',
    inherit: 'svc'
});

vjHO.register('svc-compressor' , {
    icon : 'process',
    inherit: 'svc',
    defaultDownloadWildcard : '*.hivepack'

});

vjHO.register('svc-dna-screening' , {
    icon : 'img/scope.png',
    inherit: 'svc',
    jsDir: "js-obj-new"
});

vjHO.register('svc-dna-alqc' , {
    icon : 'img/svc-process.gif',
    inherit: 'svc',
    jsDir: "js-obj-new"
});

vjHO.register('svc-align-screening' , {
    icon : 'process',
    inherit: 'svc-align-screening'
});

vjHO.register('svc-algo-annotMapper' , {
    icon : 'img/scope.png',
    jsFile:true
});

vjHO.register('svc-profiler-refcmp', {
    icon : 'svc-process', inherit: 'svc-profiler-refcmp'
});

vjHO.register('user-info' , {
    icon : 'help'
    ,inherit: 'user-info'
});

vjHO.register('svc-dna-pemerger' , {
    icon : 'process'
    ,inherit: 'svc'
});

vjHO.register('svc-modular' , {
    icon : 'process',
    inherit: 'svc'
});

vjHO.register('ex-otvi1' , {
    icon : 'process',
    inherit: 'svc'
});
vjHO.register('ex-otvi1-raw' , {
    icon : 'process',
    inherit: 'svc'
});
vjHO.register('ex-otvi1-base' , {
    icon : 'process',
    inherit: 'svc'
});
vjHO.register('ex-otvi2-base' , {
    icon : 'process',
    inherit: 'svc'
});
vjHO.register('ex-otvi1-merged' , {
    icon : 'process',
    inherit: 'svc'
});
vjHO.register('svc-dna-umi-collapse' , {
    icon : 'process',
    inherit: 'svc'
});


vjHO.register('svc-clust' , {
    icon : '?cmd=~^dna~==.png'
    , inherit: 'svc-clust'
});

vjHO.register('svc-denove-velvet' , {
    icon : 'process',
    inherit: 'svc'
});

vjHO.register('svc-slicer3D' , {
    icon : 'process',
    inherit: 'svc'
});

vjHO.register('svc-dmNgsPred' , {
    icon : 'process'
        ,inherit: 'svc'
});

vjHO.register('svc-dmSnvDis' , {
    icon : 'process'
        ,inherit: 'svc'
});

vjHO.register('svc-dmStrDistri' , {
    icon : 'process'
    ,inherit: 'svc'
});

vjHO.register('svc-dmUniPDBmap' , {
    icon : 'process'
        ,inherit: 'svc'
});

vjHO.register('svc-download' , {
    icon : 'download'
    ,inherit: 'svc'
});

vjHO.register('svc-hiveseq' , {
    icon : 'hiveseq'
    ,inherit: 'svc'
});

vjHO.register('svc-panel' , {
    icon : 'process'
    ,jsFile: true
    ,jsDir: 'js-obj-new'
});

vjHO.register('svc-popul' , {
    icon : '?cmd=~^dna~==.gif'
        ,inherit: 'svc-popul'
});

vjHO.register('svc-profiler-heptagon' , {
    icon : 'img/heptagon.gif'
        , inherit: 'svc'
});
vjHO.register('svc-profiler' , {
    icon : 'img/heptagon.gif'
        , inherit: 'svc-profiler'
});

vjHO.register('svc-diprofiler' , {
    icon : 'process',
    jsDir:"js-obj-new",
    jsFile:true
});

vjHO.register('svc-heptagon1' , {
    icon : 'img/heptagon.gif'
});

vjHO.register('svc-recomb' , {
    icon : '?cmd=~^dna~==.gif'
        , inherit: 'svc-recomb'
});

vjHO.register('svc-sb-Donor' , {
    icon : 'process'
});

vjHO.register('svc-sb-Experiment' , {
    icon : 'process'
});

vjHO.register('svc-sb-Gene' , {
    icon : 'process'
});

vjHO.register('svc-sb-Sample' , {
    icon : 'process'
});

vjHO.register('svc-SingleCellPCR' , {
    icon : 'process'
});

vjHO.register('svc-sb-Treatment' , {
    icon : 'process'
});

vjHO.register('svc-spectraPeakDetection' , {
    jsFile:true,
    icon : '?cmd=~^dna~==.jpg'
});

vjHO.register('spectra' , {
    icon:'ico-file',
    jsFile:true
});

vjHO.register('spectra-MS' , {
    icon:'ico-file',
    inherit: 'spectra'
});

vjHO.register('spectra-lib' , {
    icon:'ico-file',
    inherit: 'spectra'
});

vjHO.register('svc-denovo-oases' , {
    icon : 'process'
});

vjHO.register('u-annot' , {
    icon : 'rec'
});

vjHO.register('u-idList' , {
    icon : 'list',
    jsFile: true
});

vjHO.register('u-ionAnnot' , {
    icon : 'ionAnnot-db.png',
    jsFile: true
});
vjHO.register('u-ionTable' , {
    icon : 'ionAnnot-db.png',
    inherit: 'u-ionAnnot'
});

vjHO.register('csv-table' , {
    inherit:'u-file',
    icon:'table'
});

vjHO.register('stdm-base' , {
    jsFile : true,
    icon:'table'
});

vjHO.register('tsv-table' , {
    inherit:'u-file',
    icon:'table'
});

vjHO.register('excel-file', {
    jsFile: true,
    icon: 'table'
});

vjHO.register('u-hivepack' , {
    icon : 'rec'
});

vjHO.register('u-hiveseq' , {
    jsFile: true,
    icon:'dna'
});
vjHO.register('nuc-read' , {
    inherit:'u-hiveseq',
    icon:'dnaold'
});

vjHO.register('genome' , {
    inherit:'u-hiveseq',
    icon:'dna'
});

vjHO.register('user' , {
    icon : '=='
});

vjHO.register('viodb' , {
    icon : 'rec'
});

vjHO.register('svc-dna-demo' , {
    icon : 'process',
    jsFile: true
});

vjHO.register('svc-genemark' , {
    icon : 'process',
    jsFile: true
});

vjHO.register('svc-textclustering' , {
    icon : 'process',
    jsFile: true
});
vjHO.register('svc-mothur' , {
    icon : 'process',
    jsFile: true
});

vjHO.register('svc-wflo' , {
    icon : 'process',
    inherit: 'svc',
    jsFile: true
});

vjHO.register('svc-argos-alqc' , {
    icon : 'process',
    inherit: 'svc',
    jsFile: true
});

vjHO.register('svc-genome-comparator' , {
    icon : 'process',
    jsFile: true,
    jsDir: "js-obj-new"
});

vjHO.register('svc-dna-seqmanip' , {
    icon : 'process',
    jsFile: true,
    jsDir: "js-obj-new"
});

vjHO.register('svc-generic-launcher' , {
    icon : 'process',
    jsFile: true,
    jsDir: "js-obj-new",
    inherit: 'svc'
});

vjHO.register('svc-msgfplus' , {
    icon : 'process',
    inherit: 'svc'
});

vjHO.register('svc-glymps' , {
    icon : 'process',
    inherit: 'svc'
});




vjHO.register('svc-affinity-viz-peak-detect', {
    icon: 'process',
    jsFile: true,
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align2' , {
    icon : 'img/processSvc.gif',
    inherit: 'svc-alignment2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-blast2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-blastx2' , {
    icon : '?cmd=~^dna~==.gif'
    , jsFile:true,
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-tblastx2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-align-blastx2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-bowtie1' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment2'
});

vjHO.register('svc-align-bwa2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment2'
});

vjHO.register('svc-align-hexagon2' , {
    inherit: 'svc-alignment2'
    , icon : 'img/processSvc.gif'
});

vjHO.register('svc-hexagon-batch2' , {
    icon : 'svc-align-hexagon'
    , inherit: 'svc-alignment2'
});

vjHO.register('svc-align-magic2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment2'
});

vjHO.register('svc-align-tophat2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment2'
});

vjHO.register('svc-align-blat2' , {
    icon : '?cmd=~^dna~==.gif'
    , inherit: 'svc-alignment2'
});


vjHO.register('svc-alignment2' , {
    icon : 'img/processSvc.gif',
    jsDir: "js-obj-new" 
});

vjHO.register('svc-align-mafft' , {
    icon : '?cmd=~^dna~==.gif',
    inherit: 'svc-alignment-multiple2'
});

vjHO.register('svc-align-clustal' , {
    icon : '?cmd=~^dna~==.gif',
    inherit: 'svc-alignment-multiple2'
});

vjHO.register('svc-alignment-remapper' , {
    icon : 'process'
    , inherit: 'svc-alignment2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-dna-codonQC' , {
    icon : 'process',
    jsDir: "js-obj-new"
});

vjHO.register('svc-dna-multi-qc' , {
    icon : 'process',
    jsDir: "js-obj-new"
});



vjHO.register('svc-dna-guide' , {
    icon : 'process',
    inherit: 'svc'
});


vjHO.register('svc-diu-antidote-cflow' , {
    icon : 'process',
    inherit: 'svc'
});

vjHO.register('svc-dna-ref-qc' , {
    icon : 'process',
    inherit: 'svc'
});


vjHO.register('svc-dna-alignQC' , {
    icon : 'process',
    jsDir: "js-obj-new"
});


vjHO.register('svc-dna-targetQC' , {
    icon : 'process',
    jsDir: "js-obj-new"
});


vjHO.register('svc-dna-kmerQC' , {
    icon : 'process',
    jsDir: "js-obj-new"
});

vjHO.register('svc-differential-profiler' , {
    icon : 'process',
    jsDir: "js-obj-new"
});

vjHO.register('svc-clust2' , {
    icon : '?cmd=~^dna~==.png',
    jsDir: "js-obj-new"
});

vjHO.register('svc-orfFinder' , {
    icon : '?cmd=~^dna~==.png',
    jsDir: "js-obj-new"
});


vjHO.register('svc-alignment-2' , {
    jsDir: "js-obj-new" 
});
vjHO.register('svc-align-2' , {
    inherit: 'svc-alignment-2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-blast-2' , {
    inherit: 'svc-alignment-2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-blastx-2' , {
    jsFile:true,
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-tblastx-2' , {
    inherit: 'svc-align-blastx-2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-align-bowtie-2' , {
    inherit: 'svc-alignment-2'
});

vjHO.register('svc-align-bwa-2' , {
    inherit: 'svc-alignment-2'
});

vjHO.register('svc-align-hexagon-2' , {
    inherit: 'svc-alignment-2'
});

vjHO.register('svc-hexagon-batch2' , {
    inherit: 'svc-alignment-2'
});

vjHO.register('svc-align-magic-2' , {
    inherit: 'svc-alignment-2'
});

vjHO.register('svc-align-tophat-2' , {
    inherit: 'svc-alignment-2'
});

vjHO.register('svc-align-blat-2' , {
    inherit: 'svc-alignment-2'
});


vjHO.register('svc-alignment-multiple-2' , {
    inherit: 'svc-alignment-2',
    jsDir: "js-obj-new" 
});

vjHO.register('svc-align-mafft-2' , {
    inherit: 'svc-alignment-multiple-2'
});

vjHO.register('svc-align-clustal-2' , {
    inherit: 'svc-alignment-multiple-2'
});

vjHO.register('svc-alignment-remapper-2' , {
    inherit: 'svc-alignment-2',
    jsDir: 'js-obj-new'
});

vjHO.register('svc-clin-data-uploader' , {
    jsFile:true,
    jsDir: 'js-obj-new'
});

vjHO.register('svc-gut-feeling' , {
    jsFile:true,
    jsDir: 'js-obj-new'
});
