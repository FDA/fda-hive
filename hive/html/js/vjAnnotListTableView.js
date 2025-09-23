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




function vjAnnotListTableControl(viewer){
    this.clone=function (viewbase)
    {
        if(!viewbase)viewbase=this;
        var i=0;
        for ( i in viewbase ) {
            this[i] = viewbase[i];
        }

    };
    this.clone(viewer);
    
    var main_objCls = "obj-AnnotListTableControl"+Math.random();
    vjObj.register(main_objCls,this);
    
    this.defaultAnnotUrl = "http:
    this.viewersArr =[];
    this.objAdded ={};
    this.rowsCnt = 0;
    
    this.maxTxtLen= this.maxTxtLen ? this.maxTxtLen : 25;
    this.bgColors= this.bgColors ? this.bgColors : [ '#f2f2f2', '#ffffff' ];
    this.defaultEmptyText = this.defaultEmptyText ? this.defaultEmptyText : 'no information to show';
    this.callbackSubmit= this.callbackSubmit ? this.callbackSubmit : 0;
    
    if (!this.vjDS) this.vjDS = vjDS;
    if (!this.vjDV) this.vjDV = vjDV;
    if (!this.formObject) this.formObject = document.forms[this.formName];
    if (!this.className) this.className = "ANNOTLISTCONTROL_table";
    
    if (!this.submitButton) this.submitButton = {}; 
    if (!this.submitButton['name']) this.submitButton.name= "Submit";
    if (!this.submitButton['hidden']) this.submitButton.hidden= false;
    
    if (!this.annotationButton) this.annotationButton= {};
    if (!this.annotationButton['name']) this.annotationButton.name = "Annotation Files";
    if (!this.annotationButton['hidden']) this.annotationButton.hidden = false;
    
    if (!this.searchTypeButton) this.searchTypeButton= {};
    if (!this.searchTypeButton['hidden']) this.searchTypeButton.hidden= true;
    
    if (!this.referenceObjList) this.referenceObjList = [];
    if (!this.annotObjList) this.annotObjList = [];
     
    var dsname = "dsAnnotList_table";
    this.annotTypeDS = this.vjDS.add("Loading annotation ", dsname, "static:
    this.checkTypeDS = this.vjDS.add("Loading results", dsname+"-checked", "static:
    
    
    var _mainControl_ = this;
    
    function checkDirection (dicPath, curPath) {
        var myArr = Object.keys(dicPath);
        for (var ia=0; ia<myArr.length; ++ia) {
            if (curPath.indexOf(myArr[ia])!=-1) {
                return dicPath[myArr[ia]];
            }
        }
        return 1;
    }
    
    function accumulateRows (viewer, node, ir,ic) {
        var objAdded = _mainControl_.objAdded;
        var range = "", seqID ="", type = "", id= "",strand = "+", checked = true;
        if (viewer.myName && viewer.myName=="manualPanel") 
        {    
            seqID = viewer.tree.findByName("seqID").value;
            var curStart = viewer.tree.findByName("start").value;
            curStart = isNaN(curStart) ? (-2) : curStart;
            var curEnd = viewer.tree.findByName("end").value;
            curEnd = isNaN(curEnd) ? (-2) : curEnd;
            if (curStart==-2 || curEnd==-2) {
                range= "not_valid";
            }
            else {
                range =  curStart + "-" + curEnd;
            }
            viewer.render();
        }
        else {
            range = node.start + "-" + node.end;
            seqID = node.seqID; type = node.type; id = node.id;
            strand = checkDirection(viewer.dicPaths, node.path);
            checked = node.checked;
        }
        
        if ( range != "not_valid")
        {
            if (!checked && objAdded[seqID]) 
            {
                if (objAdded[seqID][range]) {
                    delete objAdded[seqID][range];
                    _mainControl_.rowsCnt-=1;
                }
            }
            else 
            {
                if (!objAdded[seqID]) {
                    objAdded[seqID] ={}; 
                }
                objAdded[seqID][range]= {"defLine":type + ":" + id, "strand": strand};
                _mainControl_.rowsCnt+=1;
            }
            _mainControl_.updateInfoForSubmitPanel();
            if (_mainControl_.checkCallback) 
            {
                funcLink(_mainControl_.checkCallback, _mainControl_, viewer);
            }
         }
        if (viewer.myName && viewer.myName=="manualPanel") 
        {    
            var url = "static:
            _mainControl_.checkTypeDS.reload(url,true);
        }
    }
    
    function returnRowsChecked (submitPanel,treeNode,objNode) {
        _mainControl_.retValue = _mainControl_.constructPreviewTableUrl(_mainControl_.objAdded,true);
        if (_mainControl_.callbackSubmit){
            funcLink(_mainControl_.callbackSubmit, _mainControl_, objNode);
        }
        
        _mainControl_.anotTable.checkedCnt=0; _mainControl_.anotTable.checkedNodes=[];
        _mainControl_.anotTable.hidden = false;
        _mainControl_.annotTypeDS.reload(0,true);
        
        _mainControl_.checkedTable.hidden = true;
        _mainControl_.checkedTable.render();
        
        _mainControl_.manualPanel.hidden = true;
        _mainControl_.manualPanel.render();
        
        submitPanel.tree.findByName("showChecked").value = "Preview Checked Elements";
        submitPanel.tree.findByName("removeSelected").hidden = true;
        submitPanel.refresh();
        
        onClearAll(submitPanel);
        
        _mainControl_.retValue0 = _mainControl_.retValue;
        _mainControl_.retValue = "" ;
        
    }
    
     function onClearAll (submitPanel,treeNode,objNode) {
        _mainControl_.rowsCnt =0;
        _mainControl_.objAdded ={};
        
        var infoNode = _mainControl_.anotSubmitPanel.tree.findByName("info");
        infoNode.value = 'Select range(s) to add';
        _mainControl_.anotSubmitPanel.refresh();
        _mainControl_.checkTypeDS.reload("static:
        
        var checkedNodes = _mainControl_.anotTable.checkedNodes;
        for (var i=0; i<checkedNodes.length; ++i) {
            var curNode = checkedNodes[i];
            _mainControl_.anotTable.mimicCheckmarkSelected(curNode.irow,false);
            _mainControl_.anotTable.onCheckmarkSelected(_mainControl_.anotTable.container,0,curNode.irow);
        }
        if (_mainControl_.clearCallback) {
            funcLink(_mainControl_.clearCallback, _mainControl_, submitPanel);
        }
    }
        
     _mainControl_.constructPreviewTableUrl = function (obj, isOutput) {
        var t = "seqID,ranges\n"; var len = t.length;
        if (isOutput){
            t ="index,seqID,start,end,direction,defLine\n"; len=t.length;
        }
        var objKeyArr = Object.keys(obj);
        var iCnt=0;
        for (var i=0; i< objKeyArr.length; ++i) {
            var curObj = obj[objKeyArr[i]];
            for (var range in curObj) {
                if (isOutput) {
                    if (curObj[range]["strand"]>0) {
                        t += "" + iCnt + "," + objKeyArr[i] + "," + range.split("-")[0] + "," + range.split("-")[1] + "," + ( 0 ) + "," + curObj[range]["defLine"] +"\n"; ++iCnt;
                    }
                    else {
                        t += "" + iCnt + "," + objKeyArr[i] + "," + range.split("-")[0] + "," + range.split("-")[1] + "," + ( 1 ) + "," + curObj[range]["defLine"] +"\n"; ++iCnt;
                    }
                } else {
                    t += "" + objKeyArr[i] + "," + range + "\n";
                }
            }
        }
        return (t.length > len) ? t: "";
    }

    
     function previewCheckedElement(submitPanel,treeNode,objNode) {
        var showCheckedNode = submitPanel.tree.findByName("showChecked");
        var removeNode = submitPanel.tree.findByName("removeSelected");
        
        if (showCheckedNode.value.indexOf("Back")==-1) {
            _mainControl_.anotTable.hidden=true;
            _mainControl_.anotTable.render();
            
            var url = "static:
            _mainControl_.checkedTable.hidden=false;
            _mainControl_.checkTypeDS.reload(url,true);
            
            showCheckedNode.value = "Back";
            removeNode.hidden = false;
            submitPanel.refresh();
        }
        else {
            _mainControl_.anotTable.checkedCnt=0; _mainControl_.anotTable.checkedNodes=[];
            _mainControl_.anotTable.hidden = false;
            _mainControl_.annotTypeDS.reload(0,true);
            
            _mainControl_.checkedTable.hidden = true;
            _mainControl_.checkedTable.render();
            
            showCheckedNode.value = "Preview Checked Elements";
            removeNode.hidden = true;
            submitPanel.refresh();
        }
        
    }
    
     function removeSelectedElement(submitPanel,treeNode,objNode) {
        console.log("removing selected element");
        var checkedTbl = _mainControl_.checkedTable; 
        var objAdded = _mainControl_.objAdded;
        
        for (var i=0; i<checkedTbl.selectedCnt; ++i) {
            var curNode = checkedTbl.selectedNodes[i];
            if (objAdded[curNode.seqID]) {
                var curObj =  objAdded[curNode.seqID];
                if (curObj[curNode.ranges]) {
                    delete curObj[curNode.ranges];
                    --_mainControl_.rowsCnt;
                }
            }
            if (objAdded[curNode.seqID] && !Object.keys(objAdded[curNode.seqID]).length) {
                delete objAdded[curNode.seqID];
            }
        }
        var url =  "static:
        _mainControl_.checkTypeDS.reload(url,true);
        checkedTbl.selectedCnt=0; checkedTbl.selectedNodes=[];
        _mainControl_.updateInfoForSubmitPanel();
        if (_mainControl_.removeCallback) {
            funcLink(_mainControl_.removeCallback, _mainControl_, objNode);
        }
    }
        
     _mainControl_.reload = function () {
        if (this.annotObjList.length){
            var ionList = this.annotObjList.join(",");
            var refList = this.referenceObjList.join(";");
            this.annotTypeDS.url = urlExchangeParameter(this.defaultAnnotUrl, "ionObjs", ionList);
            this.annotTypeDS.url = urlExchangeParameter(this.annotTypeDS.url, "refGenomeList", refList);
            this.annotTypeDS.reload(0,true);
        }
    }

    _mainControl_.updateInfoForSubmitPanel = function () {
        var infoNode = this.anotSubmitPanel.tree.findByName("info");
        if (this.rowsCnt<=0){
            this.rowsCnt =0;
            infoNode.value = 'Select range(s) to add';
        }
        else {
            infoNode.value = ''+ this.rowsCnt +' range(s) added';
        }
        this.anotSubmitPanel.refresh();
    }
    function openManualPanel (panel, treeNode, objNode){
        var mPanel = _mainControl_.manualPanel;
        if (!treeNode.value)
        {
            treeNode.value = 1;
            mPanel.hidden = false;
        }    
        else
        {
            treeNode.value =0;
            mPanel.hidden = true;
        }
        mPanel.render();
    }
    
    _mainControl_.constructPanel = function () {
        var anotPanel = new vjBasicPanelView({
            data:["dsVoid",this.annotTypeDS.name],
            rows:[
                    {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                    {name:'pager', icon:'page' , title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                    { name: 'search', align: 'right', type: ' search', prefix:"Search Id: ",order:10, isSubmitable: true, title: 'Search', description: 'search id',order:'1', url: "?cmd=objFile&ids=$(ids)" },
                    { name: 'searchType', title:"search type", prefix:"Search Type: ", align: 'right', type: ' text',isSubmitable: true, description: 'search type',order:'1',path:"/search/searchType", hidden: this.searchTypeButton.hidden},
                    { name : 'ionObjs', type:"text", align: 'left' , order : 1, prefix: this.annotationButton.name, isSubmitable: true, hidden: this.annotationButton.hidden},
                    { name : 'manualInput', align: 'right' , order : -1, icon:"arrow_sort_down_highlighted.gif",title: "Insert Ranges",showTitle:true, description:"Manual Input",url: openManualPanel, iconSize:18, value:0}
            ],
            parentObjCls: main_objCls,
            formObject:this.formObject
        });
        
        var manualPanel = new vjPanelView( {
            data:["dsVoid"],
            rows: [
                     { name: 'seqID', align: 'left', type: 'text', prefix:"Sequence Id: ",order:1,  title: 'Sequence Id', description: 'Sequence Identifier',order:'1', size: '8' },
                     { name: 'start', align: 'left', type: 'text', prefix:"Start Position: ",order:2,  title: 'Start Position', description: 'Start Position',order:'2',size: '8' },
                     { name: 'end', align: 'left', type: 'text', prefix:"End Position: ",order:3,  title: 'End Position', description: 'End Position',order:'3',size: '8' },
                     { name : 'add', title:'Add', icon:"plus.gif", iconSize:"18" ,showTitle:true ,align: 'left' , order : 4, url: accumulateRows }
            ],
            parentObjCls: main_objCls,
            hidden:true,
            myName: "manualPanel",
            formObject:this.formObject
        });
        
        this.anotPanel = anotPanel;
        this.manualPanel = manualPanel;
        this.viewersArr.push(this.anotPanel);
    }
    
    
    _mainControl_.constructTable = function () {
        var anotTable = new vjTableView({
            data: this.annotTypeDS.name
            ,formObject:this.formObject
            ,parentObjCls: main_objCls
            ,bgColors: this.bgColors
            ,defaultEmptyText: this.defaultEmptyText
            ,maxTxtLen: this.maxTxtLen
            ,treeColumn: "start"
            ,checkable:true
            , appendCols :  [{header:{name:"path",title:'Annotation', type:"treenode",order:1,maxTxtLen:32},cell:""}]
            ,cols : [{ name: 'seqID', hidden:true }]
            ,treeColumn: "path"
            ,precompute: "node.name=node.seqID+'['+node.start + ':'+node.end+']';node.path='/'+node.name; \
                          if(this.viewer.dicPaths[node.path]){if (node.type.trim()=='strand'){this.viewer.dicPaths[node.path]= (node.id.trim()=='+') ? 1 : -1; };node.path+='/'+node.type+':'+node.id.replace(/\\//g,'.');} \
                          else {this.viewer.dicPaths[node.path]=1;if (node.type.trim()=='strand'){this.viewer.dicPaths[node.path]= (node.id.trim()=='+') ? 1 : -1; };} \
                          "
            ,postcompute:"if(node.treenode && node.treenode.depth>=2){node.styleNoCheckmark=true;node.name='';node.start='';node.end='';}"
            ,dicPaths: {}
            ,checkCallback: accumulateRows
            ,myName:"mainTable"
        });
        var checkElementTable = new vjTableView({
            data: this.checkTypeDS.name
            ,formObject:this.formObject
            ,parentObjCls: main_objCls
            ,bgColors: this.bgColors
            ,defaultEmptyText: this.defaultEmptyText
            ,maxTxtLen: this.maxTxtLen
            ,hidden:true
            ,myName:"checkTable"
        });
        
        this.anotTable = anotTable;
        this.checkedTable = checkElementTable;
        this.viewersArr.push(this.anotTable, this.checkedTable);
    }
    
    _mainControl_.constructSubmitPanel = function () {
        var rows=[{ name : 'info', type : 'text', title : 'Select range(s) to add', value : 'Select range(s) to add', align:'right', readonly:true, size:40, prefix:'Selected range(s):  ', order : 1},
                    { name : 'submit', type : 'button', value: this.submitButton['name'], align: 'right' , order : 2, url: returnRowsChecked, hidden: this.submitButton['hidden']},
                    { name : 'showChecked', type : 'button', value:'Preview Checked Elements', align: 'left' , order : 1, url: previewCheckedElement},
                    { name : 'removeSelected', type : 'button', value:'remove selected Elements', align: 'left' , hidden: true ,order : 2, url: removeSelectedElement},
                    { name : 'clear', type : 'button', value:'Clear', align: 'right' , order : 3, url: onClearAll }
                    ];
        
        var anotPanel = new vjPanelView({
                data:["dsVoid"],
                rows: rows,
                formObject: this.formObject,
                parentObjCls: main_objCls,
                myName: "submitPanel",
                isok: true 
        } );
        this.anotSubmitPanel = anotPanel;
        this.viewersArr.push(this.anotSubmitPanel);
    }

    _mainControl_.constructViewers = function() {
        this.constructPanel();
        this.constructTable();
        this.constructSubmitPanel();
        this.reload();
    }
    
    this.constructViewers();
    
    return [this.anotPanel, this.manualPanel,this.anotTable, this.checkedTable ,this.anotSubmitPanel];    
}

