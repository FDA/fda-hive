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

var a=7;

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjHiveseqView.js");
javaScriptEngine.include("js/vjTaxonomyView.js");
javaScriptEngine.include("d3js/sunburst_hierarchy.js");

vjHO.register('u-hiveseq').Constructor=function ()
{

    if(this.objCls)return;
    this.resolution=200;

    this.fullview=function(node,dv)
    {
        this.node=node;
        this.mode='fullview';
        this.create(dv,node.id);
    };

    this.preview = function(node,dv)
    {
        this.mode='preview';
        this.node=node;
        this.hideQCLauncher = true;
        this.hideScreenLauncher = true;
        this.hideProg_QC = true;
        this.hideProg_Screen = true;
        this.keyStorage_QC = "" + node.id + "_QC_request";
        this.keyStorage_Screen = "" + node.id + "_Screen_request";
        this.checkQC(this.objCls, dv, node.id);
    };

    this.typeName="u-hiveseq";

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
            'sequences' :{
                active_url:"http:
                objs:"ids"
            },
            'histograms' : {
                active_url:"qpbg_tblqryx4:

            },
            'ATGC' :{
                active_url:"http:
                objs:"ids"

            },
            'codonQC' :{
                active_url:"http:
                objs:"ids"

            },
            'complexity' :{
                active_url:"http:
                objs:"ids"
            },
            'Ncount' :{
                active_url:"static:
                objs:"ids"
            },
            'lengthwise' : {
                loadInactive:true,
                active_url:"qpbg_tblqryx4:
            },
            'blastNTset': {
                active_url:"http:
                isSeries:true,
                title : 'retrieve data taxonomy tree',
                objs:"screenId"
             },
             'dsProgressV_QC':{
                 active_url:"http:
             },
             'dsProgressV_Screen':{
                 active_url:"http:
             }
    };
    
    this.checkQC = function(objCls,dv,id) {
        var qcCheckUrl = "http:
        if ( !vjDS["dsQCdataCheck"] ){ 
            vjDS.add("check QC data","dsQCdataCheck",qcCheckUrl);
        }    
        vjDS["dsQCdataCheck"].parentObjCls = objCls;
        vjDS["dsQCdataCheck"].parentDV = dv;
        vjDS["dsQCdataCheck"].parentID = id;
        vjDS["dsQCdataCheck"].keyStorage_QC = this.keyStorage_QC;
        vjDS["dsQCdataCheck"].keyStorage_Screen = this.keyStorage_Screen;
        vjDS["dsQCdataCheck"].parser = function (ds,text) {
            var tbl = new vjTable(text,0,vjTable_propCSV);
            var parentObj = vjObj[this.parentObjCls];
            if (tbl.rows.length && tbl.rows[0]["_file"] != undefined)
            {
                var match_num = 0;
                var f_arr = tbl.rows[0]["_file"];
                var QC_filesToMap = ["_.qc2.countsAtPositionTable.csv", "_.qc2.sumLetterTable.csv", "_.qc2.sumPositionTable.csv"];
                for (var i=0; i< QC_filesToMap.length; ++i) {
                    for (var j=0; j< f_arr.length; j++) {
                        if (QC_filesToMap[i] == f_arr[j]) {
                            match_num++;
                            break;
                        }
                    }
                }
                parentObj.hideQCLauncher = (match_num == QC_filesToMap.length) ? true : false;
                match_num =0;
                var Screen_filesToMap = ["dna-alignx_screenResult.csv", "dna-alignx_screenShannon.csv", "dna-alignx_acclist.csv"];
                for (var i=0; i< Screen_filesToMap.length; ++i) {
                    for (var j=0; j< f_arr.length; j++) {
                        if (Screen_filesToMap[i] == f_arr[j]) {
                            match_num++;
                            break;
                        }
                    }
                }
                parentObj.hideScreenLauncher = (match_num == Screen_filesToMap.length) ? true : false;
                if (typeof(storage) !== undefined) {
                    if (parentObj.hideQCLauncher) localStorage.removeItem(parentObj.keyStorage_QC);
                    if (parentObj.hideScreenLauncher) localStorage.removeItem(parentObj.keyStorage_Screen);
                }
            }
            else {
                parentObj.hideQCLauncher = false;
                parentObj.hideScreenLauncher = false;
            }
            if (typeof(storage) !== undefined) {
                var stored_req_QC = localStorage[parentObj.keyStorage_QC];
                var stored_req_Screen = localStorage[parentObj.keyStorage_Screen];
                if (stored_req_QC) {
                    parentObj.urlSet["dsProgressV_QC"].active_url = urlExchangeParameter(parentObj.urlSet["dsProgressV_QC"].active_url,"req",stored_req_QC);
                    parentObj.hideProg_QC = false;
                    parentObj.hideQCLauncher = true;
                }
                if (stored_req_Screen) {
                    parentObj.urlSet["dsProgressV_Screen"].active_url = urlExchangeParameter(parentObj.urlSet["dsProgressV_Screen"].active_url,"req",stored_req_Screen);
                    parentObj.hideProg_Screen = false;
                    parentObj.hideScreenLauncher = true;
                }
            }
            vjObj[this.parentObjCls].create(this.parentDV,this.parentID);
            return "";
        }
        vjDS["dsQCdataCheck"].reload(qcCheckUrl,true);
    }
    

    this.create=function(dvORtab, id, geometry, formName, stickytabs){
        this.load(dvORtab,id, geometry, formName);
        if( this.onFullviewLoadCallback && this.mode != 'preview' ) {
            funcLink(this.onFullviewLoadCallback, this);
        }
        if( this.onPreviewLoadCallback && this.mode == 'preview' ) {
            funcLink(this.onPreviewLoadCallback, this);
        }
        if(this.onLoad){
            funcLink(this.onLoad, this, this.viewers);
        }

        this.construct(stickytabs);
        if( this.onFullviewRenderCallback && this.mode != 'preview' ) {
            funcLink(this.onFullviewRenderCallback, this);
        }
        if( this.onPreviewRenderCallback && this.mode == 'preview' ) {
            funcLink(this.onPreviewRenderCallback, this);
        }

        if(this.onRender){
            funcLink(this.onRender, this, this.viewers);
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
        this.dvinfo=this.current_dvORtab.name;

        this.dvname=this.current_dvORtab.name;

        this.formNames='';
        var formNode=gAncestorByTag(gObject(dvORtab.obj.name),"form");
        if(formNode)
            this.formName=formNode.attributes['name'].value;


        this.viewers.length=0;

        this.addviewer('content_panel,content',new vjHiveseqControl({
            data: 'sequences',
            checkable: this.mode=='preview' ? false : true,

            formName:this.formName
        }));

        this.addviewer('positionalCount_panel,positionalCount', new vjHiveseqPositionalCountControl({
            data: 'histograms',
            formName:this.formName
        }));

        this.addviewer('positionalQuality', new vjHiveseqPositionalQualityView ({
            data:'histograms',
            hidden: this.node._type=='genome' ? true: false
        }));

        this.addviewer('ATGCCountPanel,ATGCCount', new vjHiveseqACGTCountViewControl ({
            data:'ATGC',
            showN:false,
            formName: this.formName
        }));

        this.addviewer('ATGCQuality',  new vjHiveseqACGTQualityView ({
            data:'ATGC',
            hidden: this.node._type=='genome' ? true: false
        }));
        
        this.addviewer('CodonQuality', new vjHiveseqCodonQualityView ({
            data:'codonQC'
        }));
        
        this.addviewer('Complexity', new vjHiveseqComplexityView ({
            data:'complexity'
        }));

        this.addviewer('Ncount', new vjHiveseqNCountView ({
            data:'Ncount'
        }));

        this.addviewer('lengthwiseCount', new vjHiveseqLengthwiseCountView ({
            data:'lengthwise'
        }));
        
        this.addviewer('progressV_QC', new vjProgressView ({
            data:'dsProgressV_QC',
            hidden: (this.hideProg_QC!=undefined) ? this.hideProg_QC : true 
        }));
        
        this.addviewer('progressV_Screen', new vjProgressView ({
            data:'dsProgressV_Screen',
            hidden: (this.hideProg_Screen!=undefined) ? this.hideProg_Screen : true
        }));
        
        this.addviewer('QCLauncherPanel', new vjHiveseqPanelView ({
            data:'dsVoid',
            dependentData: {ds:this.getDS("lengthwise"), url: this.urlSet["lengthwise"].active_url},
            progressView: {dv: this.viewers["progressV_QC"], ds: this.getDS("dsProgressV_QC")},
            QC: true,
            hideQCLauncher: this.hideQCLauncher,
            formObject: document.forms[this.formName],
            objId: id
        }));

        this.addviewer('lengthwiseQuality', new vjHiveseqLengthwiseQualityView ({
            data:'lengthwise',
            hidden: this.node._type=='genome' ? true: false
        }));
          
        this.addviewer('blastPanel,blastNTset', new vjTaxonomicControl ({
            data:'blastNTset',
            icon:'img-algo/ncbi-blast.jpeg',
            updateURL:id,
            ifFromShortRead:true,
            taxTreeSelected: "function:vjObjFunc('taxonomyElementSelected','" + this.objCls + "')",
            formName:this.formName,
            dependentData: {ds:this.getDS("blastNTset"), url: this.urlSet["blastNTset"].active_url},
            progressView: {dv: this.viewers["progressV_Screen"], ds: this.getDS("dsProgressV_Screen")},
            hideScreenLauncher: this.hideScreenLauncher,
        }));
        
        this.addviewer('sunburstPanel', new vjD3JS_SunburstHierarchy ({
            data:'blastNTset',
            formName:this.formName
        }));
        
        vjDS[this.makeDSName("Ncount")].parser = function(ds,text) {
            
            var table = new vjTable(text, undefined, vjTable_propCSV);
            
            var total = 0;
            for(var i=0; i < table.rows.length; i++){
                total += parseInt(table.rows[i].count);
            }            
            var final = "percentage,total_percentage\n";
            var acumul = 0;
            for(var i=0; i < table.rows.length; i++){
                if (table.rows[i].percentage != 0){
                    final += table.rows[i].percentage + "," +  ((acumul + parseInt(table.rows[i].count))/total * 100) + "\n";
                    acumul += parseInt(table.rows[i].count);
                }
            }
            return final;
        } 
        vjDS[this.makeDSName("Ncount")].reload ("http:
    };


    this.construct=function(stickytabs){

        if(!this.loaded || !this.current_dvORtab)return;

        this.constructed=true;

        if(this.current_dvORtab.tabs){
            if (this.hideQCLauncher) 
                this.getDS("lengthwise").reload(this.urlSet['lengthwise'].active_url + "&objs=" + this.loadedID,true);
            this.current_dvORtab.addTab("ACGT","graph",[this.viewers['ATGCCountPanel'],this.viewers['ATGCCount'],this.viewers['ATGCQuality']]);
            this.current_dvORtab.addTab("sequences","dna",[this.viewers['content_panel'],this.viewers['content']]);
            this.current_dvORtab.addTab("histogram","area",[this.viewers['positionalCount_panel'],this.viewers['positionalCount'],this.viewers['positionalQuality']]);
            this.current_dvORtab.addTab("positionalQC","length",[this.viewers['QCLauncherPanel'],this.viewers['progressV_QC'],this.viewers['lengthwiseCount'],this.viewers['lengthwiseQuality']]);
            this.current_dvORtab.addTab("taxonomy","img/scope.png",[this.viewers['blastPanel'],this.viewers["progressV_Screen"],this.viewers['blastNTset']]);
            this.current_dvORtab.addTab("sunburst","img/scope.png",[this.viewers['sunburstPanel']]);
            this.current_dvORtab.addTab("codonQC","length",[this.viewers['CodonQuality'],this.viewers['Complexity'],this.viewers['Ncount']]);
        }
        else{
            this.current_dvORtab.remove();
            this.viewersToAdd=this.current_dvORtab.add(this.viewersToAdd).viewers;
        }
        this.current_dvORtab.render();
        this.current_dvORtab.load('rerender');
    };


    this.taxonomyElementSelected=function(viewer,node)
    {
        if(!node) node = viewer;
        var hdr = "id,name,path,value\n";
        var newUrl="static:
        if(node.taxid){
            newUrl = "http:
        }

        vjDS[this.viewers['taxDetailViewer'].data[1]].reload(newUrl, true);

    };


    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
