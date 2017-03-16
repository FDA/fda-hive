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
var blockedByVahan=0;
if (!javaScriptEngine)
    var javaScriptEngine = vjJS["undefined"];

vjHO.register('svc-align-blastx').Constructor = function() {

    if (this.objCls)
        return; // stupid chrome loads from both cached file and the one coming
                // from server.
    //var _that=this.inheritType('svc-align');
    this.typeName = "svc-align-blastx";
    
    this.objCls = this.typeName + Math.random();
    vjObj.register(this.objCls, this);
    // two public functions which must be supported
    this.fullview = function(node, dv) {
        this.load(dv, node); // node.id
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
        'dsDownloadType' : {
            active_url : "static://",
            title : "Loading output file list"
        },
        'dsArchive' : {
            active_url : "static://",
            title : "Loading output file list"
        },
        'help' : {
            active_url : "http://help/hlp.view.results.alignment.html",
            inactive_url : "http://help/hlp.view.results.alignment.html",
            doNotChangeMyUrl : true,
            title : "Infrastructure: Creating help"
        }

    };

    this.construct = function() {
        if (!this.loaded || !this.current_dvORtab)
            return;
        this.constructed = true;
        // var t_viewersArr=[];

        if (this.mode == 'mobileview') {
          
        }

        else if (this.mode == 'preview') {
          
        } else {
            this.dvname = this.current_dvORtab.name;
            

            this.current_dvORtab.addTab("Results Table", "table",
                    [ this.viewers['downloadList'] ]);
            
            
            this.current_dvORtab.render();
            this.current_dvORtab.load('rerender');
           
        }

    };

    this.load = function(dvORtab, node) {
        var id = node.id;
        if (this.mode == 'mobileview') {}

        else if (this.mode == 'preview') {} 
        else {

            this.loadedID = id;
            this.formName = [ '', '' ];
            var formNode = gAncestorByTag(gObject(dvORtab.obj.name), "form");
            if (formNode)
                this.formName[0] = formNode.attributes['name'].value;

            this.loaded = true;
            this.constructed = false;
            this.current_dvORtab = dvORtab.obj;

            this.dvname = this.current_dvORtab.name;
         //   this.dvinfo = this.current_dvORtab[1].name;

            var outputType = algoProcess.getValue("output_fmt");
            // Check which type of files should be retrieved depending on the user settings.
            if (outputType=="blast_out") {
               this.urlSet['dsDownloadType'].active_url = "http://?cmd=propget&ids=" + this.loadedID + "&files=All_Blast_Output.{zip}&mode=csv";
            } else if (outputType=="tsv") {
                this.urlSet['dsDownloadType'].active_url = "http://?cmd=propget&ids=" + this.loadedID + "&files=*blast*.{tsv}&mode=csv";
            } else {
                this.urlSet['dsDownloadType'].active_url = "http://?cmd=propget&ids=" + this.loadedID + "&files=*.{blast_out,tsv}&mode=csv";
            }
            
            this.addviewer('downloadList', 
                    new vjTableView({
                        data: "dsDownloadType"
                               ,formObject: document.forms["form-process"]
                               //,selectCallback: ingestSomethingLikeThat
                               ,cols:[
                                      {name:"archive file", url: "function:vjObjFunc('ingestSomethingLikeThat','"+ this.objCls + "')"}
                                      ,{name:"results files"}
                                      ,{name:"download file", url: "function:vjObjFunc('downloadResultsFile','"+ this.objCls + "')"}
                                  ]    
                               ,bgColors:['#f2f2f2','#ffffff']
                               ,ingestedRow: []
                        }));
            this.addviewer('help', new vjHelpView({
                data : 'help',
                formName : this.formName[1],
                isok : true
            }));
            
            this.getDS("dsDownloadType").parser = function (ds,data) {
                var tt = new vjTable(data,0,vjTable_propCSV);
                var fileList = verarr(tt.rows[0]["_file"]);
                var newTable = "results files,archive file,download file\n";
                for (var i=0; i < fileList.length; ++i){                                
                        newTable += "" + fileList[i] + ",<img src='img/copy.png' width=12 height=12 id=ingest_"+ i+" /> <img src='img/done.gif' width=12 height=12 id=done_"+ i+" style='visibility:hidden;'/>,<img src='img/download.gif' width=12 height=12 id=ingest_"+ i+" /> \n";
                }
                return newTable;
            }

        }
    };
    
    this.ingestSomethingLikeThat = function (table,row,col){
        if (table.ingestedRow.indexOf(row.irow) !=-1) return;
        var url = "?cmd=objFile&arch=1&ids=" + this.loadedID + "&filename=" + row.cols[0] + "&arch_dstname="+ row.cols[0]+"&backend=1";
        //alert("ingesting ..." + url)
        this.getDS("dsArchive").reload("qpbg_http://" +url,true);
        alert ("Your selected item is being ingested. You can monitor the progress from within data loading tab");
        row.download = "<img src='img/done.gif' width=12 height=12 />";
        table.ingestedRow.push(row.irow);
        gObject("ingest_" + row.irow).style.display = "none";
        gObject("done_" + row.irow).style.visibility = "visible";
    }


    this.downloadResultsFile = function(table,row,col){
        var url = "http://?cmd=objFile&ids=" + this.loadedID + "&filename=" + row.cols[0];
        this.getDS("dsArchive").reload(url,true,"download");
    }



    
};
