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

vjHO.register('svc-alignment-multiple2').Constructor=function ()
{
    if(this.objCls)return;         //stupid chrome loads from both cached file and the one coming from server.
    
    this.typeName="svc-alignment-multiple2";
    this.objCls="obj-svc-alignment-multiple"+Math.random();
    vjObj.register(this.objCls,this);

    this.onArchiveSubmit = function (viewer,content){
        var icon;
        var txt = "";
        if(!isok(content) ) {
            txt = "Program error: could not submit dataset to archiver";
            icon = "img/redInfo.png";
        }
        else if(content.indexOf("error")>=0){
            txt = content;
            icon = "img/redInfo.png";
        }
        else {
            txt = "Your data succesfully submitted for digestion (reqId:"+content+ ")!\n";
            txt += "You may monitor the process in you inbox under the 'data loading' tab";
            icon = "img/done.gif";
        }
        alertI(txt,undefined,{icon:icon});
    };
    this.dsQPBG_digest = vjDS.add('preparing to archive','ds'+this.objCls+'_QPBG_digest','static://',{func :this.onArchiveSubmit,obj : this});
    
    // two public functions which must be supported
    this.fullview=function(node,dv)
    {
        vjDS.add ("Visualizing alignments in stack", "alStack", "http://?cmd=alStack&cnt=50&info=1&mySubID=1&multiple=1&rangeEnd=100&objs=" + node.id);
        vjDS.add ("Generating consensus", "consensus", "http://?cmd=alConsensus&multiple=1&wrap=100&objs=" + node.id);
        vjDS.add ("Generation overlap", "overlap", "http://?cmd=alConsensus&multiple=1&wrap=100&overlap=1&objs=" + node.id);
        vjDS.add ("infrastructure: Creating download menu", "downloads", "static://data,down,arch,operation,arguments,params\n" +
                "Multiple Alignment,download,ico-file,alStack,&cnt=0&mySubID=1&multiple=1&rangeEnd=100,\n" +
                "Alignments in fasta,download,dna,alFasta,&wrap=100&info=1&mySubID=1&multiple=1&objs=3031174&raw=1&cnt=0,\n"+
                "Consensus in fasta,download,dna,alConsensus,&multiple=1&wrap=100,\n"+
                "Overlap in fasta,download,dna,alConsensus,&multiple=1&wrap=100&overlap=1,");
        vjDS.add ("Infrastructure: Creating help", "help", "http://help/hlp.view.results.alignment.html");
        
        
        var filesStructureToAdd = [{
            tabId: 'stack',
            tabName: "Stack",
            position: {posId: 'mainResults', top:'0', bottom:'100%', left:'20%', right:'75%'},
            inactive: false,
            viewerConstructor: {
                dataViewer: "vjAlignmentMultipleStackControl",
                dataViewerOptions:{
                    data: 'alStack',
                    formName: formName,
                    isok:true
                }
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'detailsDownloads',
            tabName: "Downloads",
            position: {posId: 'mainResults', top:'0', bottom:'100%', left:'20%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjAlignmentDownloadsView",
                dataViewerOptions:{
                    data: 'downloads',
                    formName:formName,
                    selectCallback: "function:vjObjFunc('onPerformReferenceOperation','" + this.objCls + "')",
                    isok:true
                }
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'consensus',
            tabName: "Consensus",
            position: {posId: 'mainResults', top:'0', bottom:'100%', left:'20%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjAlignmentConsensusView",
                dataViewerOptions:{
                    data: 'consensus',
                    formName: formName,
                    isok:true
                }
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'overlap',
            tabName: "Overlap",
            position: {posId: 'mainResults', top:'0', bottom:'100%', left:'20%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjAlignmentConsensusView",
                dataViewerOptions:{
                    data: 'overlap',
                    formName: formName,
                    isok:true
                }
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'resHelp',
            tabName: "Results Help",
            position: {posId: 'mainResults', top:'0', bottom:'100%', left:'20%', right:'75%'},
            inactive: true,
            viewerConstructor: {
                dataViewer: "vjHelpView",
                dataViewerOptions:{
                    data: 'help',
                    formName:formName,
                    isok:true
                }
            },
              autoOpen: ["computed"]
        }];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.moveTab("next", {top:"30%", left: "20%", right: "75%", bottom: "40%"}, 0);
    };

    this.onPerformReferenceOperation = function (viewer, node, ir, ic) {
        var oper = node.operation;
        var args = node.arguments;
        try {
            var params = JSON.parse(node.params);
        } catch (e) {
            var params = undefined;
        }
        var url;

        url = this.makeReferenceOperationURL(viewer, node, node.operation, node.arguments, params);
        
        if(viewer.tblArr.hdr[ic].name=="down"){
            url = urlExchangeParameter(url, "down", '1');

            if( docLocValue("backend",0,url) ){
                var saveAs = "o"+this.loadedID+"-"+oper+"-"+this.referenceID+".fa";
            }
            else{
                document.location = url;
            }
        }
        else if (viewer.tblArr.hdr[ic].name=="arch") {
            url = urlExchangeParameter(url, "arch", 1);
            var dstName = node.data + " ("+this.loadedID+")";
            var ext = "-";
            switch (node.arch) {
            case "dna":
                ext = "ma";
                dstName += ".fasta";
                break;
            case "dnaold":
                ext = "fasta";
                dstName += ".fasta";
                break;
            case "ico-file":
                ext = "txt";
                dstName += "."+ext;
                break;
            default :
                ext= "-";
            }
            
            url = urlExchangeParameter(url, "arch_dstname", dstName);
            url = urlExchangeParameter(url, "ext", ext);
            this.dsQPBG_digest.reload("qpbg_http://"+url,true);
        }
    };

    this.makeReferenceOperationURL = function (viewer, node, oper, args, params) {
        var qtySamAligns = 0;
        
        var url = "?cmd=" + oper + "&objs=" + this.loadedID;
        if (this.reqID)
            url += "&req=" + this.reqID;

        url+=args;
        return url;
    };
};
