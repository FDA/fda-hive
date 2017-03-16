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
// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/
//var ggg=0;
function vjProgressView ( viewer )
{
    vjTableView.call(this,viewer); // inherit default behaviors of the TreeView

    this.checkBranches=false;
    this.checkLeaves=false;
    this.showChildrenCount=false;
    this.maintainPreviousData=true;
    this.refreshCount=0;
    this.doneReported=0;
    this.selectDoesExpand=true;
    this.callCounter=0;
    this.hideZero=false;
    this.waitForNewRequests=true;
    this.hideServiceName=false;
    if(!this.refreshDelay)this.refreshDelay=10000;
    if(!this.maxRefeshCounter)this.maxRefeshCounter=10;
    if(!this.showTotalOnly)this.showTotalOnly=0;
    if (this.isNrefreshOnLoad===undefined) this.isNrefreshOnLoad=true;          //Don't refresh panels
    if(this.infoDetailsContainerSuffix===undefined)this.infoDetailsContainerSuffix="infoDetails";
    this.infoDetailsContainer_ID=new Object();
    this.info_grpID='grp';
    this.action_onDepth = 2;
    this.isStickyHeader=true;
    this.selectCallback="function:vjObjFunc('onSelectCallback','" + this.objCls + "')";
    this.isNSortEnabled=true;
    this.width = viewer.width;
    this._svcs = new Object();
    //this.callbackDone;

    //this.prefixHTML="<span id=\""+this.objCls+"-progress-icon\"></span>";
    this.postcompute="node.title=node.name+(node.cnt>1 ? '<big>&nbsp;&times;</big>'+node.cnt : '' );" +
            "params.setReportIcon(node);if(node.reqID)params.updateSvcs(node);params.hideEmptyCols(node);" +
            "node.progress100 = node.progress100+'|'+node.stat;";

    //this.appendCols=[{header:{name:"path",type:"treenode",order:1},cell:"eval:this.pretreatProgressItems(node);"}];
    this.appendCols=[{header:{name:"path",type:"treenode",order:1},cell:""}];
    this.treeColumn="path";
    this.debug=1;

    var t_black = pastelColor('#000000');

    this.infoLevels = {
            'Trace':{icon:'blueInfo.png',bgcolor:t_black,title:'Trace',iconSize:24},
            'Info':{icon:'blueInfo.png',bgcolor:t_black,title:'Info',iconSize:24},
            'Debug':{icon:'blueInfo.png',bgcolor:t_black,title:'Debug',iconSize:24},
            'Warning':{icon:'yellowInfo.png',bgcolor:pastelColor('#ECB613'),title:'Warning',iconSize:24},
            'Error':{icon:'error.gif',bgcolor:pastelColor('#ff3300'),title:'Error',iconSize:24}
    };
    /*this.actions = {
            "3":{id: 3, name:"stop",title: "stop the request",icon:{url:"img/reqAction-stop.gif",wd:24}}//, wd);
            ,"4":{id: 4, name:"pause", title: "suspend the request", icon:{url:"img/reqAction-pause.gif",wd:24}}//, wd);
            ,"5":{id: 5, name:"run", title: "resume the request",icon:{url:"img/reqAction-run.gif",wd:24}}
            ,"-2":{id: -2, name:"resubmit", title:"resubmit the request", icon:{url:"img/reqAction-resubmit.gif",wd:24}}
    };*/

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

        if (node.reqID && parseInt(node.reqID) && node.treenode.depth>1) {
            ++c_svc.reqCnt;
        }
        return 0;
    };

    this.hideEmptyCols = function ( node ) {
        for(var i = 0 ; i < this.cols.length ; ++i) {
            var col = this.cols[i];
            if( col.hidden && col.autohide && node[col.name] ) {
                col.hidden = false;
            }
        }
        for(var i = 0 ; i < this.tblArr.hdr.length ; ++i) {
            var col = this.tblArr.hdr[isok(this.sortIndex) ? this.sortIndex[i].idx : i];
            if( col.hidden && col.autohide && node[col.name] ) {
                col.hidden = false;
            }
        }
    };

    this.unknownInfo = {icon:'blueInfo.png',bgcolor:'#000000',title:'Unknown'};

    this.setReportIcon = function (node) {
        if( isok(node) && node.reportType) {
            if( !this.infoLevels[node.reportType] ) {
                return 0;
            }
            var info_obj = this.infoLevels[node.reportType];
            var icon = info_obj.icon;

            var iconSize=this.iconSize;
            if( isok(node.iconSize) ) {
                iconSize=node.iconSize;
            }
            if( info_obj.iconSize ){
                iconSize=info_obj.iconSize;
            }

            var widthTxt = "";
            if(this.iconWidth){
                widthTxt = " width='" + this.iconWidth + "' ";
            }
            node.reportType = "<img src='" + makeImgSrc(icon) + "' height=" + iconSize + widthTxt+ " border=0 />";
        }
        else {
            node.reportType = "";
        }
    };

    this.setElapsedTooltip = function (viewr,node,ic) {
        var startCol = 8, endCol = 9;

        var startTxt = formaDatetime(node[this.cols[startCol].name],true,false,true);
        var endTxt = formaDatetime(node[this.cols[endCol].name],true,false,true);
        return "started: "+startTxt+ ", finished: "+endTxt +" ";
    };

    this.cols=[{ name: new RegExp(/.*/),hidden:false}
                ,{ name: 'path', order:1, align:'left',title: "<div class='linker' id=\""+this.objCls+"-progress-icon\" onclick='javascript:vjObjEvent(\"onRefreshProgress\",\""+this.objCls + "\",0); stopDefault(event);'><img border=0 width=16 src='img/24/refresh-white-outline.png' class='linker-icon'/>&nbsp;refresh</div>", hidden: false }
                ,{ name: 'progress', order:3.1, align:'center',title: 'Items', hidden: true, autohide:true , type:'largenumber' , desciption: 'percent of the total progress' }
                ,{ name: 'progress100', order:2, align:'center',title: 'Status', width:300, hidden: true, type:'qp_progress' }
                ,{ name: 'stat', order:3, align:'left',title: 'Status', hidden: true, type:'reqstatus' }
//                ,{ name: 'qp_progress100', order:3, align:'center',title: 'Status', width:200, hidden: false, type:'qp_progress' }
                ,{ name: 'reportType', order:4, align:'left',title: 'Info', hidden: true, autohide:true }
                ,{ name: 'execTime', order:5, align:'center', title: 'Elapsed', hidden: false, type:'timespan', tooltip:this.setElapsedTooltip }
                ,{ name: 'actTime', order:6, title: 'Submitted', hidden: false, type:'datetime' }
                ,{ name: 'takenTime', order:7, title: 'Started', hidden: true, type:'datetime' }
                ,{ name: 'doneTime', order:8, title: 'Finished', hidden: true, type:'datetime' }
                ,{ name: 'waitTime', order:9, align:'center', title: 'Waited', hidden: false, type:'timespan' }
                ,{ name: 'runningBefore', order:10, title: 'Load', hidden: true, autohide:true }
                ,{ name: 'parent', hidden: true}
                ,{ name: 'name', hidden: true}
                ,{ name: 'reqID', hidden: true }
                ,{ name: 'cnt', hidden: true}
                ,{ name: 'grpID', hidden: true}
                ,{ name: new RegExp(/^act$/), hidden: true}
                ,{ name: 'svcID', hidden: true}
                ,{ name: 'orderExecute', hidden: true}
               ];

    this.tblComposerFunction=this.composerFunction;
    this.composerFunction=function( viewer , content )
    {
        this._svcs = new Object();
        //this.debug = 1;
        if (content.indexOf("unknown") == 0) {
            //alert(content)
            content = "name,parent,cnt,reqID,grpID,svcID,stat,progress,progress100,actTime,takenTime,doneTime,waitTime,execTime,reportType,orderExecute,runningBefore,act\n"
                    ///+ "Process Finished,0,0,0,0,0,0,5,0,100,0,0,0,0,0,0";
                    +"Total Progress,0,0,0,0,0,5,0,100,0,0,0,0,0,0,0,0,0";
        }
        this.tblComposerFunction(viewer, content);
//        var stat=5; // progress100=0,
        if (this.tree.root && this.tree.root.children && this.tree.root.children.length) {
            //progress100=this.tree.root.children[0].progress100;
//            stat=this.tree.root.children[0].stat;
            if (this.timeout)
                clearTimeout(this.timeout);
            if (this.tree.root.children[0].stat < 5 && this.refreshDelay && this.refreshCount < this.maxRefeshCounter) {

                this.timeout = setTimeout("vjObjEvent(\"onRefreshProgress\",\"" + this.objCls + "\",1)", this.refreshDelay);
                ++this.refreshCount;
            }else
                this.refreshCount=this.maxRefeshCounter;

            if ((!this.doneReported) && this.doneCallback && (this.reportEvenWhenNotDone || this.tree.root.children[0].stat >= 2)) {

                funcLink(this.doneCallback, this, this.tree.root.children[0].reqID, this.tree.root.children[0].stat, this.refreshCount);

                if (this.tree.root.children[0].stat >= 5)
                    ++this.doneReported;
            }
        } else if (this.waitForNewRequests) {
            this.timeout=setTimeout("vjObjEvent(\"onRefreshProgress\",\"" + this.objCls + "\",1)", this.refreshDelay);
        }
        this.refresh();
        var o=gObject(this.objCls+"-progress-icon");
        if(o){
            if(this.refreshCount < this.maxRefeshCounter) {
                o.innerHTML = "<img border=0 width=16 src='img/progress.gif' class='linker-icon'/>&nbsp;computing";//&nbsp;<small>"+(this.maxRefeshCounter-this.refreshCount);
            } else {
                o.innerHTML = "<img border=0 width=16 src='img/24/refresh-white-outline.png'/ class='linker-icon'>&nbsprefresh";
            }
        }


    };

    this.onClickExpandNode_parent = this.onClickExpandNode;

    this.onClickExpandNode = function(container, nodepath, expanded) {
        this.onClickExpandNode_parent(container, nodepath, expanded);
        var o=gObject(this.objCls+"-progress-icon");
        if(o){
            if(this.refreshCount < this.maxRefeshCounter) {
                o.innerHTML = "<img border=0 width=16 src='img/progress.gif' class='linker-icon'/>&nbsp;computing";//&nbsp;<small>"+(this.maxRefeshCounter-this.refreshCount);
            } else {
                o.innerHTML = "<img border=0 width=16 src='img/24/refresh-white-outline.png' class='linker-icon'/>&nbsp;refresh";
            }
        }
    };

    this.getStatus = function ()
    {
        return this.tree.root.children[0].stat;
    };

    this.actionButton=function( node, img, action, title, wd)
    {
        var reqID=node.reqID;
        if (!parseInt(node.reqID) && parseInt(node.grpID)) {
            reqID = node.grpID;
            action += "&isGrp=1";
            if( Int(node.svcID) ) {
                action += "&svcID="+node.svcID;
            }
        }
        img="img/reqAction-"+img+".gif";
        return "<td><div class='linker' onclick='javascript:vjObjEvent(\"onReqSetStatus\",\""+ this.objCls + "\"," + sanitizeElementAttrJS(reqID) + ",\""+sanitizeElementAttrJS(action)+"\"); stopDefault(event);'><img src='"+img+"' width="+wd+" border=0 title='"+sanitizeElementAttr(title)+"'/></div></td>";
    };

    this.onSelectCallbackZZZ=function(viewer,node, ir, ic , col )
     {
        if (col.name == 'reportType') {
            funcLink(this.onInfoLevelSelectCallback,viewer, node, ir, ic);
            return ;
        }
        if (col.name == 'path' ) {
            funcLink(this.onPathNameSelectCallback,viewer, node, ir, ic);
            return;
        }
        if (!node.selected)
            return;
        if (!parseInt(node.reqID) && !parseInt(node.grpID))
            return;
        if (col.name != "stat" && col.name != 'progress100')
            return;
//        var isGrp = (!parseInt(node.reqID) && parseInt(node.grpID));
        var t = "";
        t += "<table bgcolor=white border=0 ><tr>";

        var c_action = 0;
        if(node.stat<5){
            c_action = this.actions[node.stat+""];
        }
        else{
            c_action = this.actions[-2+""];
        }
        if( c_action ) {
            t += this.actionButton(node, c_action.icon.url, c_action.id, c_action.title, c_action.icon.wd);
        }

        t += "</tr></table>";

        gFloatPermanent = 0;
        gFloatButtons = 0;
        gFloatShiftY = -this.actions[-2+""].icon.wd / 2;

        floatDivOpen("Actions", t, false);
    };

    this.onSelectCallback=function(viewer,node, ir, ic , col )
    {
        if (col.name == 'reportType') {
            funcLink(this.onInfoLevelSelectCallback,viewer, node, ir, ic);
            return ;
        }
        if (col.name == 'path' ) {
            funcLink(this.onPathNameSelectCallback,viewer, node, ir, ic);
            return;
        }
        if(!node.selected)return ;
        if(!parseInt(node.reqID) && !parseInt(node.grpID))return ;
        if(col.name!="stat" && col.name!='progress100')return ;

        if( node.treenode.depth < this.action_onDepth )return ; //only single requests can be handled.

//        var isGrp=(!parseInt(node.reqID) && parseInt(node.grpID));
        var wd=24;
        var t="";
        t+="<table bgcolor=white border=0 ><tr>";
        if(node.stat==3)t+=this.actionButton(node,"stop",3,"stop the request", wd);
        if(node.stat<3)t+=this.actionButton(node,"pause",4,"suspend the request", wd);
        if(node.stat==4 )t+=this.actionButton(node,"run",5,"resume the request", wd);
        if(node.stat>=5)t+=this.actionButton(node,"resubmit",-2,"resubmit the request", wd);

        t+="</tr></table>";


        gFloatPermanent=0;
        gFloatButtons=0;
        gFloatShiftY=-wd/2;

        floatDivOpen("Actions",t, false);
        //var o = gObject("dvFloatingViewer");
        //o.innerHTML=t;
        //dvFloatingDiv_show(gMoX,gMoY);
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

    this.onReqSetStatus = function(cont, reqId, act)
    {
        //if( confirm("Are you sure you want to terminate this process?") ) {
        //    linkCmd("-qpKillReq&req=" + reqId+"&act="+act);
        //}
        linkCmd("-qpReqSetAction&req=" + reqId+"&act="+act);
        floatDivClose();
        this.doneReported=0;
        this.onRefreshProgress();

    };

    this.attach=function( req, objID )
    {
        var progress_url = "static://unknown";
        var info_url = "static://";

        if (isok(objID)) {
            progress_url = "http://?cmd=-qpRawCheck&reqObjID=" + objID;
            info_url = "http://?cmd=-qpReqInfo&reqObjID=" + objID;
        } else if (!(parseInt(req) > 0)) {
            return;
        } else if (isok(req)) {
            progress_url = "http://?cmd=-qpRawCheck&req=" + req;
            info_url = "http://?cmd=-qpReqInfo&req=" + req;
        }
        this.refreshCount=0;
        this.getData(0).reload(progress_url, true );
        var dsInfo=this.getData(1);
        if(dsInfo)
            dsInfo.reload(info_url, true );
    };
    this.reset=function( )
    {
        this.tree=new vjTree();
    };
}

function vjProgressInfoView(viewer){
    this.hidden = true;
    this.autoexpand= 2;
    vjProgressView.call(this,viewer);
    this.defaultEmptyText = "No information for the selected process";
    var legend = "<span><table class='TABLEVIEW_table' style='border:0;padding:0;border-space:0;font-size:75%' ><tr>"
            + "<td bgcolor=\"#000000\" width=\"15\" ></td><td>Info</td>"
            + "<td width=\"10\"></td><td></td>"
            + "<td bgcolor=\"#ECB613\" width=\"15\"></td><td>Warning</td>"
            + "<td width=\"10\"></td><td></td>"
            + "<td bgcolor=\"#ff3300\" width=\"15\"></td><td>Error</td>"
            + "</tr></table></span>";
    this.cols = [ {
        name : new RegExp(/.*/),
        hidden : true
    }, {
        name : 'path',
        order : 1,
        align : 'left',
        title : legend,
        width : 100,
        hidden : false
    }, {
        name : 'infoTxt',
        order : 2,
        align : 'left',
        title : 'Message',
        width : 2000,
        hidden : false
    }, {
        name : 'infoLvl',
        width : 30,
        order : 3,
        align : 'center',
        title : 'Severity',
        hidden : false
    }, {
        name : 'infoLvlNum',
        hidden : true
    }
    ];

    this.composerFunction=function( viewer , content )
    {
        this._svcs = new Object();
        this.debug = 1;

        this.tblComposerFunction(viewer, content);
        this.refresh();
    };


    this.setReportIcon = function (node) {
        if( isok(node) && node.infoLvl) {
            if( !this.infoLevels[node.infoLvl] ) {
                return 0;
            }
            var info_obj = this.infoLevels[node.infoLvl];
            var icon = info_obj.icon;

            var iconSize=this.iconSize;
            if( isok(node.iconSize) ) {
                iconSize=node.iconSize;
            }
            if(isok(info_obj.iconSize)){
                iconSize=info_obj.iconSize;
            }

            var widthTxt = "";
            if(this.iconWidth){
                widthTxt = " width='" + this.iconWidth + "' ";
            }
            node.infoLvl = "<img src='" + makeImgSrc(icon) + "' height=" + iconSize + widthTxt+ " border=0 />";
        }
        else {
            node.infoLvl = "";
        }
    };
    this.infoRowSort = function(a,b){

        var are_siblings = false;
        if( a.treenode && b.treenode) {
            if(a.treenode.parent && b.treenode.parent) {
                if(a.treenode.parent == b.treenode.parent)
                    are_siblings = true;
            }
        }
        while( !are_siblings) {
            var result = (a.treenode.depth - b.treenode.depth);
            if( result > 0 ){
                a = a.treenode.parent.tablenode;
            }
            else if ( result < 0 ) {
                b = b.treenode.parent.tablenode;
            }
            else {
                b = b.treenode.parent.tablenode;
                a = a.treenode.parent.tablenode;
            }
            if( a == b && result ) {
                return result;
            }
            if(a.treenode.parent == b.treenode.parent)
                are_siblings = true;
        }

        var at=isok(a["svcName"])?a["svcName"]:a["name"],bt=isok(b["svcName"])?b["svcName"]:b["name"];
        var result = 0;
        if(at && bt){
            at=at.toLowerCase();
            bt=bt.toLowerCase();
            result=(bt==at)?0:(at>bt?1:-1);

        }
        else {
            return 0;
        }
        if( !result ) {
            at = parseInt(a["reqID"]);bt = parseInt(b["reqID"]);
            if(at && bt)
                result=(!bt?0:bt) - (!at?0:at);
            else{
                return 0;
            }
        }
        if( !result ) {
            at = parseInt(a["infoLvlNum"]);bt = parseInt(b["infoLvlNum"]);
            if(at && bt)
                result=(!bt?0:bt) - (!at?0:at);
            else {
                return 0;
            }
        }
        if( !result ) {
            at = parseInt(a["name"]);bt = parseInt(b["name"]);
            if(at && bt)
                result=(!at?0:at) - (!bt?0:bt);
            else {
                return 0;
            }
        }
        return result;
    };

    this.sortTreeColumnFunc = this.infoRowSort;
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

    vjProgressView.call (this,viewer);
};

function vjProgressControl_ProgressDownload(viewer)
{
    this.checkBranches = false;
    this.checkLeaves = false;
    this.showRoot = true;
    this.showLeaf = true;
    this.name = "download";
    this.icon =  "download";
    this.autoexpand = "all";
    this.appendCols = [{header:{name:"path",type:"treenode",order:1},cell:""}];
    this.treeColumn = "path";
    this.cols = [{ name: new RegExp(".*"), hidden: true }
            ,{ name: 'path', order:1, align:'left',title: "<span id=\""+this.objCls+"-progress-icon\">Level</span>", hidden: false }
            ,{ name: 'stat', order:3, align:'left',title: 'Status', hidden: false, type:'reqstatus' }];
    this.formObject = viewer.formObject;
    this.postcompute = "node.icon='download';node.title=node.name+(node.cnt>1 ? '<big>&nbsp;&times;</big>'+node.cnt : '' );";
    this.isNSortEnabled = true;
    this.icons = { leaf: 'img/process.gif' };

    this.defaultSelectObjectCallback = function(viewer, node, ir, ic, col) {
        var reqID= node.reqID;
        if(!parseInt(node.reqID) ) {
            if(node.grpID){
                reqID=node.grpID;
            }
            else{
                reqID="";
            }
        }

        var svcID = "";
        if( node.svcID && node.parent=='Total Progress' ){
            svcID = node.svcID;
        }

        var url = vjDS[viewer.data].url;

        url=urlExchangeParameter(url,"req",reqID);
        url=urlExchangeParameter(url,"svcID",svcID);
        url=urlExchangeParameter(url,"showreqs","1");
        url=urlExchangeParameter(url,"raw","1");
        url=urlExchangeParameter(url,"down","1");
        url=urlExchangeParameter(url,"cnt","0");

        url = "?"+url.split("?").splice(1).join("?");

        if( docLocValue("backend",0,url) ){
            //TODO: add backend ajax here
        }
        else{
            document.location = url;
        }

    };

    if (this.selectCallback == undefined)
        this.selectCallback = this.defaultSelectObjectCallback;

    vjTableView.call (this,viewer);
};

function vjProgressControl_makeQrySuffix(params)
{
    var suffix = "";
    if (params.search != undefined && params.search != null) {
        suffix += '.filter({(.id%20as%20string)%3D~/' + params.search + '/%20||%20._type%3D~/' + params.search + '/%20||%20._brief%3D~/' + params.search + '/})';
    }
    suffix += '.csv(["_type","_brief","created","svc","submitter"],{info:true';
    if (params.start != undefined && params.start != null)
        suffix += ',start:' + params.start;
    if (params.count != undefined && params.count != null)
        suffix += ',cnt:' + params.count;
    suffix += '})';
    return suffix;
}

function vjProgressControl_InputsDataSource(sourcebase)
{
    for (var i in sourcebase) this[i] = sourcebase[i];
    vjDataSource.call(this, sourcebase);

    this.makeURL = function (params, forceReload) {
        if (params.id) this.id = params.id;
        var url = 'http://?cmd=objQry&qry=allusedby(' + this.id + ')' + vjProgressControl_makeQrySuffix(params);
        if (forceReload) this.reload(url, true);
        return url;
    };
    return this;
}

function vjProgressControl_OutputsDataSource(sourcebase)
{
    for (var i in sourcebase) this[i] = sourcebase[i];
    vjDataSource.call(this, sourcebase);

    this.makeURL = function (params, forceReload) {
        if (params.id) this.id = params.id;
        var url = 'http://?cmd=objQry&qry=allthatuse(' + this.id + ').filter({._type!~/folder/})' + vjProgressControl_makeQrySuffix(params);
        if (forceReload) this.reload(url, true);
        return url;
    };
    return this;
}

function vjProgressControl(viewer)
{
    for (var i in viewer) this[i] = viewer[i];
    var control = this;

    if (!this.shape) this.shape = "compact";
    if (!this.data || !this.data.progress || !this.data.inputs || !this.data.outputs)
        alert("DEVELOPER WARNING: undefined or incomplete data for vjProgressControl");

    if (this.formName == undefined)
        alert("DEVELOPER WARNING: undefined formName for vjProgressControl");

    var formObject = document.forms[this.formName];

    this.defaultSelectObjectCallback = function(viewer, node, ir, ic, col) {
        var cmd = node.submitter;
        var ids = "&id="+node.id;
        if (!cmd) cmd = node.svc;
        if (!cmd && (node._type == "nuc-read" || node._type == "genome")) {
            cmd = "dna-hiveseq";
            ids = "&ids="+node.id;
        }
        if (!cmd) return;
        linkURL("?cmd="+cmd+ids, "process"+node.id);
    };
    if (this.selectObjectCallback == undefined)
        this.selectObjectCallback = this.defaultSelectObjectCallback;

    this.progressViewer = new vjStandardProgressViewer({
        allToRefresh: ["DV_StandardProgressViewer", "DV_ProgressInfoView", "DV_ProgressControlInputs", "DV_ProgressControlOutputs", "DV_ProgressControlDownloads"],
        divName: "DV_StandardProgressViewer",
        data: this.data.progress,
        formObject: formObject,
        prefixHTML: viewer.prefixHTML,
        doneCallback: this.doneCallback,
        reportEvenWhenNotDone: true,
        dontRefreshAll: viewer.dontRefreshAll,
        width: viewer.width,
        isok: true
    });


    this.progressInfoViewer = new vjProgressInfoView({
        divName: "DV_ProgressInfoView",
        data: this.data.progress_info,
        name: 'Info',
        icon: 'process',
        formObject: formObject,
        width: viewer.width,
        showRoot: true,
        showLeaf: true
    });


    this.showLevelInfo = function (viewer,node,ir,ic) {
        var reqID=node.reqID;
        if(!parseInt(node.reqID) ) {
            if(node.grpID){
                reqID=node.grpID;
            }
        }
        var svcID= "";
        if( node.svcID && node.parent=='Total Progress' ){
            svcID = "&svcID="+node.svcID;
        }
        
        var ds = vjDS[this.progressInfoViewer.data];
        if (this.newViewer)
        {
            $("[data-name='info']").click();
            return;
        }
        var c_tab = viewer.tab;
        var c_dv = c_tab.parent;
        c_dv.onClickViewToggle('info', c_tab.num , 3 );
        ds.reload("http://?cmd=-qpReqInfo&req="+reqID+svcID,true);
    };
    this.refreshOnService = function (viewer,node,ir,ic) {

        if(node.reqID && parseInt(node.reqID))return; //no action when selecting request nodes
        var selected_svcs = new Array();
        for (var i in viewer._svcs){
            if(node.svcID == i){
                if( !viewer._svcs[i].reqCnt )
                    selected_svcs.push(i);
            }
            else if(viewer._svcs[i].reqCnt) {
                selected_svcs.push(i);
            }
        }
        var ds = viewer.getData(0);

        if(this.default_showreqs===undefined && ds.url) {
            this.default_showreqs = docLocValue("showreqs",0,ds.url);
        }

        if(node.grpID && parseInt(node.grpID) ) {
            if(selected_svcs.length ) {
                var new_url = urlExchangeParameter(ds.url, "svcID", selected_svcs.join(","));
                new_url = urlExchangeParameter(new_url, "showreqs", 1);
                ds.reload(new_url,true);
            }
            else{
                var new_url = urlExchangeParameter(ds.url, "svcID", "-");
                new_url = urlExchangeParameter(new_url, "showreqs", 0);// By Vahan : the collapsed expansion doesn't work-  I need to seqq reqIDs
                ds.reload(new_url,true);
            }
        }
        else{
            viewer.autoexpand = 1;
            var new_url = urlExchangeParameter(ds.url, "cnt", 0);
            new_url = urlExchangeParameter(new_url, "showreqs", this.default_showreqs);
            ds.reload(new_url,true);
        }
        return;
    };
    var that=this;
    this.progressViewer.onInfoLevelSelectCallback = function(viewer,node,ir,ic){that.showLevelInfo(viewer,node,ir,ic);};
    this.progressViewer.onPathNameSelectCallback = function(viewer,node,ir,ic){that.refreshOnService(viewer,node,ir,ic);};

    //refresh both progress and info if needed. Progress must always be reloaded in order to check status.
    this.progressViewer.onRefreshProgress=function(cont,autocall)
    {
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


    function vjProgressControl_IOView(viewer) {
        this.icon = "table";
        this.formObject = formObject;
        this.cols = verarr(this.cols).concat([
            { name: "id", title: "ID", hidden: false },
            { name: "_type", hidden: true },
            { name: "svc", hidden: true },
            { name: "submitter", hidden: true },
            { name: "_brief", title: viewer.summaryColTitle ? viewer.summaryColTitle : "Summary", hidden: false },
            { name: "created", title: "Created", type: "datetime" }
        ]);
        if (control.shape == "compact") this.hidden = true;

        this.bgColors = [ '#f2f2f2', '#ffffff' ];
        this.selectInsteadOfCheck = true;
        if (control.selectObjectCallback)
            this.selectCallback = control.selectObjectCallback;

        this.defaultIcon = "rec";
        this.iconSize = 24;

        this.precompute = function(viewer, table, ir) {
            var node = table.rows[ir];
            if (node._type) {
                var icon = vjHO ? vjHO.get(node._type, "icon", node.category) : null;
                node.icon = icon ? icon : "rec";
            } else {
                node.icon = "recError";
                node._brief = "<strong>Either object " + node.id + " was deleted or you don't have permission to view it</strong>";
                node.cols[2] = node._brief;

                // Ensure the user can see that some input objects are missing
                viewer.hidden = false;
                var o = document.getElementById(viewer.container+"_cell");
                if (o) o.className = "progressVisibility";
            }
        };


        vjTableView.call(this, viewer);

    }

    this.inputsViewer = new vjProgressControl_IOView({
        divName: "DV_ProgressControlInputs",
        data: this.data.inputs,
        name: "Input Objects",
        icon:"algoin",
        width: viewer.width,
        defaultEmptyText: "no input objects used",
        summaryColTitle: "Input Objects Description"
    });

    this.outputsViewer = new vjProgressControl_IOView({
        divName: "DV_ProgressControlOutputs",
        data: this.data.outputs,
        name: "Objects Using This as Input",
        icon:"algoout",
        width: viewer.width,
        defaultEmptyText: "no other computations are using this one's results",
        summaryColTitle: "Output Objects Description"
    });

    this.downloadProgressViewer = new vjProgressControl_ProgressDownload({
        divName: "DV_ProgressControlDownloads",
        hidden : true,
        data: this.data.progress_download,
        iconSize : 16,
        width: viewer.width,
        isok: true
    });

    return [this.progressViewer,
            //this.inputsViewer.panelViewer,
            this.inputsViewer,
            //this.outputsViewer.panelViewer,
            this.outputsViewer,
            this.progressInfoViewer,
            this.downloadProgressViewer];
}

//# sourceURL = getBaseUrl() + "/js/vjProgressView.js"