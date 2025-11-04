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
var imgMap ={Kill: "stop", Suspend: "pause", Resume: "run", Run: "resubmit"};
var infoLevels = {
    'Trace': "bug-trace-color",
    'Info': "info-round-color",
    'Debug': "bug-color",
    'Warning': "warning-color",
    'Error': "error-color",
    'Suspend': "pause",
    'Resume': "play",
    'Kill': "stop",
    'Run': "resubmit"
};
loadJS('jsx/widgets/vanilla/button.template.js')
loadJS('jsx/widgets/vanilla/actionButton.js')

function vjProgress2View ( viewer )
{
    
    vjTableView.call(this,viewer);
    loadJS('jsx/widgets/jquery/components/ProgressMsgsDialog.js');
    loadJS('jsx/widgets/jquery/components/ProgressPartsDialog.js');

    this.nonSticky = false;

    this.refreshCount=0;
    this.doneReported=0;
    this.refreshDelay=10000;
    this.maxRefeshCounter=10;
    this.action_onDepth = 2;
    this.isStickyHeader=true;
    this.selectCallback="function:vjObjFunc('onSelectCallback','" + this.objCls + "')";
    this._svcs = new Object();

    if(!this.data){
        var ds = "dsProgress" + this.objcls;
        vjDS.add("", ds, "http:
        this.data = ds;
    }
    
    this.postcompute = `params.updateTitle(node);
                        params.setReportIcon(node);
                        params.setActionIcon(node);
                        if(node.reqID)params.updateSvcs(node); 
                        node.progress100 = node.progress100 + '|' + node.stat;
                        params.hideEmptyCols(node);
                        `

    this.appendCols=[{header:{name:"path",type:"treenode",order:1},cell:""}];
    this.treeColumn="path";

    this.updateSvcs = function(node) {
        if (!node.svcID || !parseInt(node.svcID)) {
            return 0;
        }
        if (!this._svcs[node.svcID]) {
            if (!this._svcs['length']) {
                this._svcs['length'] = 0;
            }
            ++this._svcs['length'];
            this._svcs[node.svcID] = new Object({
                id : 0,
                reqCnt : 0,
                title : node.svcName
            });
        }
        var c_svc = this._svcs[node.svcID];

        if (node.reqID && parseInt(node.reqID) && node.treenode && node.treenode.depth>1) {
            ++c_svc.reqCnt;
        }
        return 0;
    };

    this.hideEmptyCols = function ( node ) {
        for(var i = 0 ; i < this.cols.length ; ++i) {
            var col = this.cols[i];
            if( col.hidden && col.autohide && node[col.name] && node[col.name] != '0') {
                col.hidden = false;
            }
        }
        for(var i = 0 ; i < this.tblArr.hdr.length ; ++i) {
            var col = this.tblArr.hdr[isok(this.sortIndex) ? this.sortIndex[i].idx : i];
            if( col.hidden && col.autohide && node[col.name] && node[col.name] != '0') {
                col.hidden = false;
            }
        }
    }

    this.setReportIcon = function (node) {
        if( isok(node) && node.reportType) {
            if( !infoLevels[node.reportType] ) {
                return 0;
            }
            node.info = node.reportType;
            node.reportType = "<div style='height:2em; width: 2em; border-radius: 50%; background-color: #f2f2f2; padding: 3px; box-shadow: 0px 1px 4px 0px #bfbfbf; margin: 2px 0px;'><i class='rv-" + infoLevels[node.reportType] + "' title='" + node.reportType + "'/> </div>";
        }
        else {
            node.reportType = "";
        }
    }
    
    this.setActionIcon = function (node){
        if (node.action != "" && node.action != undefined && node.action != "0"){
            node.actName = node.action;
            node.action = ProgressActionButton({
                action: node.action
            });
        }else{
            node.action = ''
        }
    }

    this.updateTitle = function(node){
        node.title = node.name
        if(node.pieces && node.pieces != 0){
            let pieces = parseInt(node.pieces)
            let button;
            if( pieces > 1 ) {
                node.title = node.name.split(':')[0]
                button = createFakeButton({name:`&times${pieces}`  , classes:'hv-btn-link', data:{button: 'pieces'}})
            } 
            node.title += `<span style="padding-left:20px; font-size: 14px; "> ${button || ''}</span>`
        } 
    }


    this.setElapsedTooltip = function (viewr,node,ic) {
        var startCol = 8, endCol = 9;

        var startTxt = formaDatetime(node[this.cols[startCol].name],true,false,true);
        var endTxt = formaDatetime(node[this.cols[endCol].name],true,false,true);
        return "started: "+startTxt+ ", finished: "+endTxt +" ";
    };

    this.cols=[{ name: new RegExp(/.*/),hidden:false}
                ,{ name: 'path', order:1, align:'left',title: "<div class='linker' id=\""+this.objCls+"-progress-icon\" onclick='javascript:vjObjEvent(\"onRefreshProgress\",\""+this.objCls + "\",0); stopDefault(event);'><img border=0 width=16 src='img/24/refresh-white-outline.png' class='linker-icon'/>&nbsp;refresh &nbsp;</div>", hidden: false }
                ,{ name: 'pieces', order: 1.5 , hidden: true, autohide:false , title:'Parts'}
                ,{ name: 'progress', order:3.1, align:'center',title: 'Items', hidden: true, autohide:true , type:'largenumber' , desciption: 'percent of the total progress' }
                ,{ name: 'progress100', order:2, align:'center',title: 'Progress', width:300, hidden: true, type:'qp_progress2' }
                ,{ name: 'stat', order:3, align:'left',title: 'Status', hidden: true, type:'reqstatus' }
                ,{ name: 'reportType', order:4, align:'left',title: 'Info', hidden: true, autohide:true }
                ,{ name: 'execTime', order:5, align:'center', title: 'Elapsed', hidden: false, type:'timespan', tooltip:this.setElapsedTooltip }
                ,{ name: 'actTime', order:6, title: 'Submitted', hidden: false, type:'datetime' }
                ,{ name: 'takenTime', order:7, title: 'Started', hidden: true, type:'datetime' }
                ,{ name: 'doneTime', order:8, title: 'Finished', hidden: true, type:'datetime' }
                ,{ name: 'waitTime', order:9, align:'center', title: 'Waited', hidden: false, type:'timespan' }
                ,{ name: 'runningBefore', order:10, title: 'Load', hidden: true}
                ,{ name: 'parent', hidden: true}
                ,{ name: 'name', hidden: true}
                ,{ name: 'reqID', hidden: true }
                ,{ name: 'cnt', hidden: true}
                ,{ name: 'grpID', hidden: true}
                ,{ name: new RegExp(/^act$/), hidden: true}
                ,{ name: 'svcID', hidden: true}
                ,{ name: 'orderExecute', hidden: true}
                ,{ name: 'objID', hidden: true}
                ,{ name: 'action', hidden: false, title: "Action"}
               ];

    this.tblComposerFunction=this.composerFunction;

    this.composerFunction=function( viewer , content )
    {
        var contentToUse = "name,parent,cnt,reqID,grpID,svcID,pieces,stat,progress,progress100,takenTime,doneTime,waitTime,execTime,reportType,orderExecute,runningBefore,act,objID,action\n";
        this._svcs = new Object();
        if (content.indexOf("unknown") == 0) {
            contentToUse += "Total Progress,0,0,0,0,0,0,Done,0,100,0,0,0,0,0,0,0,0";
        }
        else{
            var progressTree = JSON.parse(content);
            function iterateTree(node, parentName, parent = null){
                var nodeName = `${node.name}: ${node.reqID}`;
                
                var action = "";
                if(node.status == "Running") action = "Kill";
                else if (node.status == "Waiting") action = "Suspend";
                else if (node.status == "Suspended") action = "Resume";
                else if (node.status == "Killed" || node.status == "ProgError" || node.status == "SysError") action = "Run";
                else if (node.status == "Done") action = "Run";


                let rows = nodeName + "," + parentName + "," + node.childCount + "," +
                        node.reqID + "," +  node.grpID + "," + node.svcID + "," + node.pieces +"," + node.status + "," + node.progress + "," +
                        node.progress100 + "," + node.takenTime + "," + node.doneTime + "," + node.waitTime + "," +
                        node.execTime + "," + (node.severity || '') + "," + node.orderExecute + "," + node.runningBefore + "," + node.act + "," + node.objID + "," + action +"\n";
               if(parent && node.reqID === parent.reqID) rows =''

                if(node.children) {
                    var i;
                    for(i = 0; i < node.children.length; i++){
                        parent = node
                        rows += iterateTree(node.children[i], nodeName , node);
                    }
                }

                return rows;
            }

            contentToUse += iterateTree(progressTree.Head, "root");
        }
        
        this.tblComposerFunction(viewer, contentToUse);
        if (this.tree.root && this.tree.root.children && this.tree.root.children.length) {
            if (this.timeout)
                clearTimeout(this.timeout);
            if ((this.tree.root.children[0].stat != "Done" && this.tree.root.children[0].stat != "Killed" && this.tree.root.children[0].stat != "ProgError"  && this.tree.root.children[0].stat != "SysError") 
                    && this.refreshDelay && (this.refreshCount < this.maxRefeshCounter || this.maxRefeshCounter === -1)) {

                this.timeout = setTimeout("vjObjEvent(\"onRefreshProgress\",\"" + this.objCls + "\",1)", this.refreshDelay);
                ++this.refreshCount;
            }else
                this.refreshCount=this.maxRefeshCounter;

            if (this.doneCallback && (this.reportEvenWhenNotDone || (this.tree.root.children[0].stat != "Waiting" && this.tree.root.children[0].stat != "Any" && this.tree.root.children[0].stat != "Processing"))) {

                funcLink(this.doneCallback, this, this.tree.root.children[0].reqID, this.tree.root.children[0].stat, this.refreshCount);

                if (this.tree.root.children[0].stat == "Done" || this.tree.root.children[0].stat != "Killed" || this.tree.root.children[0].stat != "ProgError"  || this.tree.root.children[0].stat != "SysError")
                    ++this.doneReported;
            }
        } else if (this.waitForNewRequests) {
            this.timeout=setTimeout("vjObjEvent(\"onRefreshProgress\",\"" + this.objCls + "\",1)", this.refreshDelay);
        }

        this.refresh();
        var o=gObject(this.objCls+"-progress-icon");
        if(o){
            if(this.refreshCount < this.maxRefeshCounter || this.maxRefeshCounter === -1) {
                o.innerHTML = "<img border=0 width=16 src='img/progress.gif' class='linker-icon'/>&nbsp;computing";
            } else {
                this.maxRefeshCounter = -1;
                this.refreshDelay = 60000;
                o.innerHTML = "<img border=0 width=16 src='img/24/refresh-white-outline.png'/ class='linker-icon'>&nbsprefresh";
            }
        }
        if(this.postCompose) this.postCompose()
    };

    this.onClickExpandNode_parent = this.onClickExpandNode;

    this.onClickExpandNode = function(container, nodepath, expanded) {
        this.onClickExpandNode_parent(container, nodepath, expanded);
        var o=gObject(this.objCls+"-progress-icon");
        if(o){
            if(this.refreshCount < this.maxRefeshCounter) {
                o.innerHTML = "<img border=0 width=16 src='img/progress.gif' class='linker-icon'/>&nbsp;computing";
            } else {
                o.innerHTML = "<img border=0 width=16 src='img/24/refresh-white-outline.png' class='linker-icon'/>&nbsp;refresh";
            }
        }
    };

    this.onSelectCallback=function(viewer,node, ir, ic , col, event )
    {
        if(col.name === "action" && node.action != ""){
            linkCmd("?cmdr=-qpReqSetAction&req=" + node.reqID + "&act=" + node.actName  + "&isGrp=" + (node.reqID == node.grpID));
            this.doneReported=0;
            this.onRefreshProgress();
            return;
        }
        else if (col.name === "reportType" && parseInt(node.pieces) <= 1){
            if(node.info && node.info !== '' && node.info != '0'){
                this.parent.lastSelect = "progressViewer";
            
                let dsInfo_params = {
                    objs: this.id, 
                    req: node.reqID
                }
                let msg_dialog  = new ProgressMsgDialog({params: dsInfo_params})
            }
        }
        else if( (col.name === "path" && event.target.dataset.button === "pieces") || (col.name === "reportType" && parseInt(node.pieces) > 1 && node.info && node.info !== '' && node.info != '0')){
            if(node.pieces && node.pieces !== "" && node.pieces != '0'){
                let params = {
                    reqObjID: node.objID,
                    req: node.reqID,
                    cnt: node.pieces
                }

                let parts_dialog  = new ProgressPartsDialog({params: params}) 
            }
        }
    };

    this.onRefreshProgress=function(cont,autocall)
    {
        for ( var id=0; id< this.data.length; ++id) {
            if(this.getData(id).state=='done')
                this.getData(id).reload(null,id==this.data.length-1 ? true : false);
        }

        if(!autocall)
            this.refreshCount=0;
    };

    this.reset=function( )
    {
        this.tree=new vjTree();
    };
}

function vjStandardProgressViewer(viewer)
{
    this.icon='tree';
    this.showRoot= true;
    this.showLeaf = true;
    this.checkLeafs= true;
    this.checkBranches= true;
    this.icons= { leaf: 'img/progress.gif' };
    this.showChildrenCount= true;
    this.autoexpand= "all";
    this.name= "Progress";
    this.icon= "progress";
    this.width = viewer.width;
    if(viewer.formName)viewerformObject=document.forms[viewer.formName];

    vjProgress2View.call (this,viewer);
};

function vjProgress2Control(viewer)
{
    for (var i in viewer) this[i] = viewer[i];
    this.id = viewer.id;
    this.scrollIn = null
    this.scrollTo = null

    if (!this.shape) this.shape = "compact";

    var formObject = document.forms[this.formName];

    this.progressViewer = new vjStandardProgressViewer({
        data: viewer.data,
        divName: "DV_StandardProgressViewer",
        formObject: formObject,
        prefixHTML: viewer.prefixHTML,
        doneCallback: this.doneCallback,
        reportEvenWhenNotDone: true,
        width: viewer.width,
        id: viewer.id,
        parent: this,
        isok: true
    });

    this.progressViewer.postCompose = function(){
        this.scrollIn = this.isStickyHeader 
                        ? this.div.querySelector(`#${this.container}_table_div`) 
                        :  this.div
        if(this.scrollIn && this.scrollTo){
            this.scrollIn.scrollTop = this.scrollTo;
        }
    }

    this.progressViewer.onRefreshProgress=function(cont,autocall)
    {
        this.scrollIn = this.isStickyHeader 
                        ? this.div.querySelector(`#${this.container}_table_div`) 
                        :  this.div
       
        this.scrollTo = this.scrollIn.scrollTop

        var viewer = this;
        for ( var id=0; id< viewer.data.length; ++id) {
            if(viewer.getData(id).state=='done')
                viewer.getData(id).reload(null,id==viewer.data.length-1 ? true : false);
        }
        
        if (viewer.dontRefreshAll){
            if(!autocall)
                this.refreshCount=0;
            return;
        }
        
        if (viewer.tab)
        {
            viewer = viewer.tab.viewers[viewer.iview+3];
            if( viewer._rendered ) {
                for ( var id=0; id< viewer.data.length; ++id) {
                    if(viewer.getData(id).state=='done')
                        viewer.getData(id).reload(null,id==viewer.data.length-1 ? true : false);
                }
            }
    
            if(!autocall)
                this.refreshCount=0;
        }
        else if (viewer.allToRefresh)
        {
            for (var i = 0; i < viewer.allToRefresh.length; i++)
            {
                var tempViewer = vjDV[viewer.allToRefresh[i]];
                if (tempViewer)
                    tempViewer.getData(0).reload(null,0==tempViewer.data.length-1 ? true : false);
            }
                
        }
    };

    this.progressViewer.options = {isVisible: true };

    return [this.progressViewer];
}