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

if (!javaScriptEngine)
    var javaScriptEngine = vjJS["undefined"];

javaScriptEngine.include("js/vjTableViewX2.js");
javaScriptEngine.include("d3js/correlation_cords.js");

vjHO.register('svc-diprofiler').Constructor = function() {
    
    this.objCls="obj-svc-diprofiler"+Math.random();
    vjObj.register(this.objCls,this);

    this.preview = function(node, dv) {
        this.parent.preview('svc', node, dv);
    }
    this.fullview = function(node, whereToAdd) {
        this.objID = docLocValue("id");
        vjDS.add("", "dsRawResult", "qpbg_tblqryx4:
        vjDS.add("", "dsFilteredResult", "qpbg_tblqryx4:
        vjDS.add("", "dsDIReads", "qpbg_http:
        vjDS.add("", "dsDIAlignments", "qpbg_http:

        var that = this;
        this.tableController1 = new vjTableControlX2({
            data : "dsRawResult",
            onClickCellCallback:function(){
                return that.onSelectRow.apply(that,arguments);
                },
            formObject : document.forms[formName]
        });
        var download_reads_row1 = {name:'di_reads', order:-10 ,title: 'Download DI reads' , icon:'save.gif', inactive : true, description: 'Download reads supporting the selected DI particle' ,  url: "javascript:vjObjEvent(\"onDownloadReadsRaw\", \""+this.objCls+"\");" };
        var download_reads_row3 = {name:'di_alignments', order:-10 ,title: 'Download DI alignments' , icon:'save.gif', inactive : true, description: 'Download alignments supporting the selected DI particle' ,  url: "javascript:vjObjEvent(\"onDownloadAlignmentsRaw\", \""+this.objCls+"\");" };
        this.tableController1.tableViewer.actualBigPanel.plugins.general.rows.push(download_reads_row1);
        this.tableController1.tableViewer.actualBigPanel.plugins.general.rows.push(download_reads_row3);
        
        this.tableController2 = new vjTableControlX2({
            data : "dsFilteredResult",
            onClickCellCallback:function(){
                return that.onSelectRow.apply(that,arguments);
                },
            formObject : document.forms[formName]
        });

        var download_reads_row2 = {name:'di_reads', order:-10 ,title: 'Download DI reads' , icon:'save.gif', inactive : true, description: 'Download reads supporting the selected DI particle' ,  url: "javascript:vjObjEvent(\"onDownloadReadsGrouped\", \""+this.objCls+"\");" };
        var download_reads_row4 = {name:'di_alignments', order:-11 ,title: 'Download DI alignments' , icon:'save.gif', inactive : true, description: 'Download alignments supporting the selected DI particle' ,  url: "javascript:vjObjEvent(\"onDownloadAlignmentsGrouped\", \""+this.objCls+"\");" };
        this.tableController2.tableViewer.actualBigPanel.plugins.general.rows.push(download_reads_row2);
        this.tableController2.tableViewer.actualBigPanel.plugins.general.rows.push(download_reads_row4);

        this.chord_viewer=new vjD3JS_CorrelationCord({
            data: 'dsFilteredResult'
            ,pairs: {array: [["Subject (left)"],["Subject (right)"]]}
             ,pairValue: ["#COL[NaN]"]
             ,oneColorPerCateg: true
             ,sortLables: false
            ,label_rules: {
                max_length: 20
            }
            ,downloadSvg:true
            ,formObject:document.forms[formName]
            ,fontOptions: {
                size: "10px"
                ,weight:"bold"
            }
        });

        var structureToAdd = [ {
            tabId : 'rawTable',
            tabName : "All hits",
            position : {
                posId : 'resultsTable',
                top : '0',
                bottom : '100%',
                left : '20%',
                right : '75%'
            },
            inactive : true,
            viewerConstructor : {
                instance : this.tableController1.arrayPanels
            },
            autoOpen : [ "computed" ]
        }, {
            tabId : 'filteredTable',
            tabName : "Grouped hits",
            position : {
                posId : 'resultsTable',
                top : '0',
                bottom : '100%',
                left : '20%',
                right : '75%'
            },
            viewerConstructor : {
                instance : this.tableController2.arrayPanels
            },
            autoOpen : [ "computed" ]
        }, {
            tabId : 'filteredTableChord',
            tabName : "Grouped hits Chord Diagram",
            inactive: true,
            position : {
                posId : 'resultsTable',
                top : '0',
                bottom : '100%',
                left : '20%',
                right : '75%'
            },
            viewerConstructor : {
                instance : [this.chord_viewer]
            },
            autoOpen : [ "computed" ]
        } ];

        algoWidgetObj.addTabs(structureToAdd, "results");
    };
    this.onSelectRow = function(viewer,node,ir,ic) {
        var menu_node = viewer.actualBigPanel.tree.findByName("di_reads");
        var menu_node_al = viewer.actualBigPanel.tree.findByName("di_alignments");
        if(!menu_node || !menu_node_al)return;
        var prev_state = menu_node.inactive;
        menu_node.inactive = (viewer.selectedNodes.length==0?true:false);
        menu_node_al.inactive = (viewer.selectedNodes.length==0?true:false);
        if(prev_state != menu_node.inactive) {
            viewer.actualBigPanel.redrawMenuView();
        }
        return ;
    }
    
    var isMultipleSubjects = function(my_tbl) {
        var res = false;
        try {
            res = my_tbl.tblArr.hdr[0].name.match(/Subj/);
        } catch (err) {
            console.error(err)
        }
        return res;
    }
    
    var get_DI_raw_description_byNode = function(my_tbl, node) {
        var is_multi_subj = isMultipleSubjects(my_tbl);
        if( !is_multi_subj )
            return node.cols.slice(0,4).join(",");
        return "\""+node.cols[0]+"\","+node.cols.slice(1,3)+",\""+node.cols[3]+"\","+node.cols.slice(4,6)
    }
    
    var get_DI_group_description_byNode = function(my_tbl, node) {
        var is_multi_subj = isMultipleSubjects(my_tbl);
        if( !is_multi_subj )
            return node.cols.slice(0,8).join(",");
        return "\""+node.cols[0]+"\","+node.cols.slice(1,5)+",\""+node.cols[5]+"\","+node.cols.slice(6,10);
    }
    
    this.onDownloadReadsRaw = function () {
        var my_tbl = this.tableController1.tableViewer;
        var node = my_tbl.selectedNodes[0];
        var di_description = get_DI_raw_description_byNode(my_tbl,node);
        this.downloadreads(di_description);
    };
    this.onDownloadAlignmentsRaw = function () {
        var my_tbl = this.tableController1.tableViewer;
        var node = my_tbl.selectedNodes[0];
        var di_description = get_DI_raw_description_byNode(my_tbl,node);
        this.downloadalignments(di_description);
    };
    this.onDownloadReadsGrouped = function () {
        var my_tbl = this.tableController2.tableViewer;
        var node = my_tbl.selectedNodes[0];
        var di_description = get_DI_group_description_byNode(my_tbl,node);
        this.downloadreads(di_description);
    };
    this.onDownloadAlignmentsGrouped = function () {
        var my_tbl = this.tableController2.tableViewer;
        var node = my_tbl.selectedNodes[0];
        var di_description = get_DI_group_description_byNode(my_tbl,node);
        this.downloadalignments(di_description);
    };
    
    this.downloadreads = function (di_description) {
        di_description = encodeURIComponent(di_description);
        vjDS["dsDIReads"].reload(this.constructURLforReads(di_description,"reads","fasta"), true,'download');
    }
    this.downloadalignments = function (di_description) {
        di_description = encodeURIComponent(di_description);
        vjDS["dsDIAlignments"].reload(urlExchangeParameter(this.constructURLforReads(di_description,"alignments","csv"),"alignments",1), true,'download');
    }
    
    this.constructURLforReads = function (di_description, type, ext) {
        let a="qpbg_http:
        a=a.replace(/\(/g,"_").replace(/\)/g,"_").replace(/\\/g,"_").replace(/\'/g,"_").replace(/\"/g,"_");
        return a;
    }
};
