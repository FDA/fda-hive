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

$(function () {
    $.widget("layout.algoviewNew", $.layout.layoutmanager, {
        options:{
            modalFormName: "algoModalForm",
            type: "horizontal",
            divName: "viewContent",
            algoObj: {},
            jsonForPage:{},
            svcType: "",
            algoTitle:"",
            showSubmitByDefault: true,
            paramsPos: undefined,
            closeHelp: true,
            dragTab: false,
            
            roundup:10
        },
        _onBeforeInit: function() {
             var node = {
                 _type : this.options.svcType,
                 id : docLocValue("id")
             };
             this.options.jsonForPage = this._setUpJsonForAlgo(this.options.algoTitle, this.options.algoObj, this.options.paramsPos);

             jsonOptions = this.options.jsonForPage;

             for (var i = 0; i < jsonOptions.dataSourceArray.length; i++)
             {
                 var curDS = jsonOptions.dataSourceArray[i];
                 var url=curDS.url;

                 if( algoProcess.docLocsToBorrow ) {
                     for( var ir=0; ir<algoProcess.docLocsToBorrow.length; ++ir ) {
                         url=urlExchangeParameter(url, algoProcess.docLocsToBorrow[ir], docLocValue(algoProcess.docLocsToBorrow[ir] , "-"), true);
                     }
                 }
                 else
                     algoProcess.docLocsToBorrow=[];
                 
                 algoProcess.docLocsToBorrow.push("id");
                 
                 var idVal = docLocValue("id" , "-");
                 if (idVal.length > 1 && idVal.indexOf("-") == 0)
                     idVal = idVal.substring(1);
                 url=urlExchangeParameter(url, "ids", idVal, true);
                 url=urlExchangeParameter(url, "reqObjID", idVal, true);
                 url=urlExchangeParameter(url, "submitter", docLocValue("cmdMode" , "-"), true);
                 ir = algoProcess.docLocsToBorrow.indexOf("id");
                 if (url.indexOf("$id") > -1 && algoProcess.docLocsToBorrow && docLocValue(algoProcess.docLocsToBorrow[ir] , "-") != '-')
                     url = url.substring(0, url.indexOf('$id')) + docLocValue(algoProcess.docLocsToBorrow[ir] , "-") + url.substring(url.indexOf('$id')+3);

                 vjDS.add('', curDS.name, url);
                 if (curDS.parser)
                     vjDS[curDS.name].parser = curDS.parser;
             }
             
             
             this.options.config = {
                 layout: {
                     layoutType: "horizontal",
                     allowResize: false,
                       items:[
                       {
                           id: 'menu',
                           size:"250",
                           overflow: "hidden",
                           allowResize: false,
                         view:{
                             name: "algomenuview",
                             options:{ 
                                 algoTitle: this.options.algoTitle,
                                 menuDesc: this.options.jsonForPage,
                                 submitButton: algoProcess.submitButtonName
                             }
                         }
                   },{
                           id: 'viewContent',
                           size:"*",
                           overflow: "auto",
                           allowResize: true,
                           layout: {
                             border: 'between',
                             overflow: 'auto',
                             layoutType:'gridstack',
                             id: "help2",
                             name: "help2",
                                allowResize: false,
                             items: []
                           }
                   }]
                }
             };
             
             this.options.container = "algoview" + parseInt(Math.random() * 100000);
         },
         _onAfterInit: function(){
             var oThis = this;
             var parent = $("#"+this.options.contentId).parent();
             var formObj = $(document.createElement("form")).attr("name", algoFormName).append($("#"+this.options.contentId));
             
             parent.empty();
             parent.append(formObj);
             this.element.addClass("algoviewNew");
             
             this.gridstack = $("[data-id='"+this.options.divName+"']").first().gridstackArea('instance');
             this.gridstack.options.algoManager = this;
             this.curManager = null;
             $('.layout-manager').each(function(){
                 if ($(this).children(".grid-stack"))
                     oThis.curManager = $(this).first().layoutmanager('instance');
             });
             this.curMenu = $('[data-name="algomenu-view"]').first().algomenuview('instance');
             
             var tableview = new vjTableView({
                data: "dsDemo" 
             });
             
             var helpToAdd = {
                     allowHide: true,
                     allowMaximize: true,
                     position: { x:0, y: 0, width:2, height:6 },
                     tabId: "someToolbar",
                     tabTitle: "Some Toolbar",
                     viewerConstructor: {
                         dataViewerInstance: tableview
                     }
             };
             
             this._iterateAlgoJson(this.options.jsonForPage.subTabs);
             
             this.curMenu.element.on("open-tab", function(event, params){
                 $.getAlgoViewManager().openTab(params, null, null, null, false);
             });
             this.curMenu.element.on("close-tab", function(event, params){
                 $.getAlgoViewManager().closeTab(params, false);
              });
             this.curMenu.element.on("scroll-to", function(event, params){
                 $.getAlgoViewManager().scrollTo(params.hash);
              });
         },
         
         openTab: function (curTab, preObj, fromIterateJSON, preObj, sendEvent, categ){
             if (!sendEvent){
                 var infobox = $("#" + curTab.tabId + "-gridstack").children(".layout-infobox");
                 this.gridstack.openArea(infobox);
                 return;
             }
             
             var viewToUse = {};
             
             if(curTab.viewerConstructor.dataViewer){
                 viewToUse = {
                        name: "dataview", 
                        options:{
                            dataViewer: curTab.viewerConstructor.dataViewer,
                            dataViewerOptions: curTab.viewerConstructor.dataViewerOptions
                        }
                 };
             }
             else if (curTab.viewerConstructor.dataViewerInstance){
                 viewToUse = {
                         name: "dataview",
                         options:{
                             instance: curTab.viewerConstructor.dataViewerInstance
                         }
                 };
             }
             else if (curTab.viewerConstructor.preObjPos >= 0){
                 viewToUse = {
                         name: "dataview",
                         options: {
                             instance: preObj[curTab.viewerConstructor.preObjPos]
                         }
                 };
             }
             else if (curTab.viewerConstructor.instance){
                 viewToUse = {
                         name: "dataview",
                         options: { instance: curTab.viewerConstructor.instance}
                 };
             }
             
             var layoutToAdd = {
                     layout: {
                         items:[{
                             allowHide: curTab.allowClose ? curTab.allowClose : true,
                             allowMaximize: curTab.allowMaximize ? curTab.allowMaximize : true,
                             tabs: {
                                 border: 'between',
                                 overflow: 'auto',
                                 droppable: false,
                                 resizable: false,
                                 name: curTab.tabId,
                                 area: {
                                     defaultLoc: curTab.position,
                                     node2: curTab.node2,
                                     node3: curTab.node3,
                                     node4: curTab.node4
                                 },
                                 items: [{
                                     title: curTab.tabTitle,
                                     newIcon: true,
                                     class: "icomoon-liga icon-" + categ,
                                     name: curTab.tabId,
                                     overflow: 'auto',
                                     view: viewToUse
                                 }]
                             }
                         }]
                     }
             };
             this.curManager.append(layoutToAdd, this.gridstack);
             curTab.visible = true;

             this.sendEvent ("check-tab", {name: curTab.tabId, sendingLoc: this.curMenu.element});
         },
         
         closeTab: function(curTab, sendToMenu){
             curTab.visbile = false;
             if (sendToMenu){
                 this.sendEvent ("uncheck-tab", {name: curTab.tabId, sendingLoc: this.curMenu.element});
             }
             else{
                 $("#"+curTab.tabId+"-gridstack").find("button[title='Hide']").click();
             }
         },
         
         hideButton: function(){
             this.curMenu.hideSubmitButton();
         },
         
         scrollTo: function (scrollTabName){
            if (scrollTabName !== "" || scrollTabName !== undefined) {
                $("[data-id='"+this.options.divName+"']").animate({
                    scrollTop: $(scrollTabName+"-tab").closest(".grid-stack-item").position().top
                }, 800);
            }
            
         },
         
         _setUpJsonForAlgo:function (algoTitle, algoObj, paramsPos)
         {
             var batchParams = ["batch", "batch_ignore_errors"];
             if (currentCompletionState == "whileRunning" || currentCompletionState == "computed") {
                 batchParams.push("batch_children_proc_ids");
             }
             var moveParams=[
                 { ds:"dsSystemParams", params: ["comment","system"]},
                 { ds:"dsBatchParams", params: batchParams },
                 { ds:"dsInputParams", params: algoProcess.visibleParameters }
             ];
             if (algoProcess.moveParams){
                 moveParams = moveParams.concat(algoProcess.moveParams);
             }
             
             var dsArray=[
                 {name: "dsAlgoSpec", url: "http:
                 {name: "dsAlgoVals", url: "http:
                 {name: "dsProgress", url: "http:
                 {name: "dsProgress_download", url: "http:
                 {name: "dsProgress_info", url: "http:
                 {name: "dsProgress_inputs", url: 'http:
                 {name: "dsProgress_outputs", url: 'http:
                 {name: "dsHelp", url: "http:
             ];


             var batchInGeneral = false;
             for(i=0; i<moveParams.length; ++i)
             {
                 var t="";
                 for( var ir=0; moveParams[i].params && ir < moveParams[i].params.length; ++ir)
                 {
                     t+="<span id='RV-"+moveParams[i].params[ir]+"'></span>";
                     if (moveParams[i].params[ir] == "batch-svc")
                         batchInGeneral = true;
                 }
                 dsArray.push( {name: moveParams[i].ds, url: "static:
             }
             if (!batchInGeneral){
                 for (var i = 0; i < dsArray.length; i++)
                     if (dsArray[i].name == "dsBatchParams")
                         dsArray[i].url += "<span id='RV-batch_svc'></span>";
             }

             var batchFldPreset={batch_svc:{constraint:'choice+', constraint_data:'single
             var submitButtons = algoProcess.submitButtons ? algoProcess.submitButtons :  ("<button id='submitterInput' style='visibility:hidden;' type=button class='myButton submitterInputButton' onClick='vjObjEvent(\"onSubmitRequest\",\""+algoProcess.objCls+"\")' name='BUTTON-submitter' size=20>"+(algoProcess.submitButtonName ? algoProcess.submitButtonName : "SUBMIT")+"</button>");
             var propHiveId = (algoProcess.svcRecViewer ?algoProcess.svcRecViewer: algoProcess.svcProcType);
             var submitCmd = "-qpProcSubmit";
             if (algoProcess.overwriteID && algoProcess.loadedID) {
                 propHiveId = algoProcess.loadedID;
                 submitCmd = "-qpProcReSubmit";
             }
             algoProcess.parametersDiv = 'DV_Parameter_view';
             var setUpForPage = {
                 algorithmTitle: algoTitle,
                 parametersDiv: algoProcess.parametersDiv,
                 dataSourceArray: dsArray,
                 subTabs: {
                     parameters: {
                         visibleDuring: ["preSubmit", "whileRunning"],
                         pageTabName: "Parameters",
                         pageTabChildren: [
                         {
                             tabId: 'general',
                             tabTitle: "General",
                             showSubmitButton: submitButtons,
                             position: { x:0, y: 0, width:4, height:6 },
                             node2: { x:0, y: 0, width:6, height:6 },
                             node3: { x:0, y: 0, width:4, height:6 },
                             node4: { x:0, y: 0, width:3, height:6 },
                             viewerConstructor: {
                                 dataViewer: 'vjHTMLView',
                                 dataViewerOptions: {
                                     data: "dsInputParams"
                                 }
                             },
                             preload: true,
                             active: true,
                             allowClose: true,
                             allowMaximize: true,
                             autoOpen: ["preSubmit", "whileRunning"]
                         }, {
                             tabId: 'advanced',
                             tabTitle: "Advanced",
                             showSubmitButton: submitButtons,
                             tabDependents: ["general","system", "batch"],
                             active: true,
                             position: { x:4, y: 0, width:2, height:6 },
                             node2: { x:6, y: 6, width:6, height:6 },
                             node3: { x:4, y: 0, width:4, height:6 },
                             node4: { x:3, y: 0, width:3, height:6 },
                             viewerConstructor: {
                                 dataViewer: 'vjRecordView',
                                 dataViewerOptions: {
                                     divName: algoProcess.parametersDiv,
                                     kind:"valgoProcess",
                                     data: algoProcess.loadedID ? [ "dsAlgoSpec", "dsAlgoVals", "dsCurrentUserSpecLoaded" ] : ["dsAlgoSpec", "dsVoid", "dsCurrentUserSpecLoaded" ],
                                     hiveId: propHiveId,
                                     objType:  algoProcess.svcRecViewer ? algoProcess.svcRecViewer : algoProcess.svcProcType  ,
                                     cmdPropSet:makeCmdSafe(submitCmd)+"&svc="+algoProcess.qpSvc,
                                     readonlyMode: algoProcess.modeActive ? false : true,
                                     callbackRendered : "function:vjObjFunc('onRecordLoaded','"+algoProcess.objCls+"')",
                                     onChangeCallback : "function:vjObjFunc('onRecordChanged','"+algoProcess.objCls+"')",
                                     onAddElementCallback : "function:vjObjFunc('onRecordAddedElement','"+algoProcess.objCls+"')",
                                     accumulateWithNonModified:true,
                                     showReadonlyInNonReadonlyMode: algoProcess.showReadonlyInNonReadonlyMode,
                                     constructionPropagateDown:10,
                                     autoexpand: algoProcess.autoexpand ?  algoProcess.autoexpand : 1,
                                     showRoot:false,
                                     autoDescription:0,
                                     autoStatus:3,
                                     autoDescription:0,
                                     fldPresets: algoProcess.fldPresets ? cpyObj(batchFldPreset,algoProcess.fldPresets) : batchFldPreset,
                                     RVtag: "RV",
                                     formName: algoFormName,
                                     isok:true
                                 }
                             },
                             autoOpen: ["preSubmit", "whileRunning"],
                             allowClose: true,
                             allowMaximize: true,
                             preload: true
                         }, {
                             tabId: 'system',
                             tabTitle: "System",
                             showSubmitButton: submitButtons,
                             active: true,
                             position: { x:6, y: 0, width:2, height:6 },
                             node2: { x:0, y: 6, width:6, height:6 },
                             node3: { x:0, y: 8, width:4, height:6 },
                             node4: { x:6, y: 0, width:3, height:6 },
                             viewerConstructor: {
                                 dataViewer: 'vjHTMLView',
                                 dataViewerOptions: {
                                     data: "dsSystemParams"
                                 }
                             },
                             preload: true,
                             allowClose: true,
                             allowMaximize: true,
                             autoOpen: ["preSubmit", "whileRunning"]
                         }, {
                             tabId: 'batch',
                             tabTitle: "Batch",
                             showSubmitButton: submitButtons,
                             active: true,
                             position: { x:8, y: 0, width:2, height:6 },
                             node2: { x:6, y: 6, width:6, height:6 },
                             node3: { x:0, y: 6, width:4, height:6 },
                             node4: { x:9, y: 0, width:3, height:6 },
                             viewerConstructor: {
                                 dataViewer: 'vjHTMLView',
                                 dataViewerOptions: {
                                     data: "dsBatchParams"
                                 }
                             },
                             preload: true,
                             allowClose: true,
                             allowMaximize: true,
                             autoOpen: ["preSubmit", "whileRunning"]
                         }, {
                             tabId: 'helpMain',
                             tabTitle: "Main Help",
                             active: true,
                             position:{ x:10, y: 0, width:2, height:10 },
                             node2: { x:6, y: 6, width:6, height:6 },
                             node3: { x:8, y: 6, width:4, height:6 },
                             node4: { x:9, y: 6, width:3, height:6 },
                             viewerConstructor: {
                                 dataViewer: 'vjHelpView',
                                 dataViewerOptions: {
                                     data: "dsHelp",
                                 }
                             },
                             preload: true,
                             allowClose: true,
                             allowMaximize: true,
                             autoOpen: ["preSubmit", "whileRunning"]
                         }],
                         icon: ["icon-parameters", "icomoon-liga"]
                     },
                     progress:
                     {
                         preConstructor: {
                             dataViewer: 'vjProgressControl',
                             dataViewerOptions: {
                                 data : {
                                     progress: "dsProgress",
                                     progress_download:  "dsProgress_download",
                                     inputs:  "dsProgress_inputs",
                                     outputs:  "dsProgress_outputs",
                                     progress_info: "dsProgress_info"
                                 },
                                 width: "100%",
                                 newViewer: true,
                                 dontRefreshAll: true,
                                 formName: algoFormName,
                                 doneCallback: algoProcess.callbackDoneComputing
                             }
                         },
                         visibleDuring: ["whileRunning"],
                         pageTabName: "Progress",
                         pageTabChildren: [
                             {
                                 tabId: 'main',
                                 tabTitle: "Main Progress",
                                 tabDependents: ["inputObj", "objUsing", "info", "downloads"],
                                 active: true,
                                 position: { x:0, y: 6, width:2, height:6 },
                                 viewerConstructor: {
                                     preObjPos: 0,
                                 },
                                 autoOpen: ["whileRunning"]
                             },
                             {
                                 tabId: 'inputObj',
                                 tabTitle: "Input Objects",
                                 active: true,
                                 position: { x:2, y: 6, width:2, height:6 },
                                 viewerConstructor: {
                                     preObjPos: 1,
                                     inactive:true
                                 },
                                 autoOpen: ["whileRunning"]
                             },
                             {
                                 tabId: 'objUsing',
                                 tabTitle: "Objects Using This as Input",
                                 active: true,
                                 position: { x:4, y: 6, width:2, height:6 },
                                 viewerConstructor: {
                                     preObjPos: 2,
                                     inactive:true
                                 },
                                 autoOpen: ["whileRunning"]
                             },
                             {
                                 tabId: 'info',
                                 tabTitle: "Notifications",
                                 active: true,
                                 position: { x:6, y: 6, width:2, height:6 },
                                 viewerConstructor: {
                                     preObjPos: 3,
                                     inactive:true
                                 },
                                 autoOpen: ["whileRunning"]
                             },
                             {
                                 tabId: 'downloads',
                                 tabTitle: "Download",
                                 active: true,
                                 position: { x:8, y: 6, width:2, height:6 },
                                 viewerConstructor: {
                                     preObjPos: 4,
                                     inactive:true
                                 },
                                 autoOpen: ["whileRunning"]
                             }
                         ],
                         icon: ["icon-progress", "icomoon-liga"]
                     },
                     results: {
                         visibleDuring: ["computed"],
                         pageTabName: "Results",
                         pageTabChildren: [],
                         icon: ["icon-results", "icomoon-liga"]
                         
                     },
                     whatNext:{
                         visibleDuring: ["computed"],
                         pageTabName: "What's Next?",
                         pageTabChildren: [],
                         icon: ["icon-move-down", "i-sm", "icomoon-liga"]
                     }
                 }
             };

             if (algoObj.tabsToAdd && algoObj.tabsToAdd.length > 0)
             {
                 for (var i = 0; i < algoObj.tabsToAdd.length; i++)
                 {
                     var whereToAdd = algoObj.tabsToAdd[i].whereToAdd;
                     var whatToAdd = algoObj.tabsToAdd[i].whatToAdd;

                     if (!setUpForPage.subTabs[whereToAdd]){
                         setUpForPage.subTabs[whereToAdd] = {};
                         setUpForPage.subTabs[whereToAdd].visibleDuring = algoObj.tabsToAdd[i].visibleDuring
                         setUpForPage.subTabs[whereToAdd].pageTabName = algoObj.tabsToAdd[i].pageTabName;
                         setUpForPage.subTabs[whereToAdd].pageTabChildren = [];
                     }
                     setUpForPage.subTabs[whereToAdd].pageTabChildren.push (whatToAdd);
                 }
             }

             return setUpForPage;
         },
         
         _iterateAlgoJson: function (options, appendTo) {
            if (options.length < 1)
                 return;

            for ( var tab in options ) {
                if(appendTo != tab && appendTo != undefined) continue; 
                
                var curNode = options[tab];
                var visibleDuring = "";
                var preObj = [];

                for (var i = 0; curNode.visibleDuring && i < curNode.visibleDuring.length; i++)
                    visibleDuring += " " + curNode.visibleDuring[i];
                   
                if(curNode.preConstructor){
                    var objToCreate = eval(curNode.preConstructor.dataViewer);
                    preObj = new objToCreate (curNode.preConstructor.dataViewerOptions);
                }

                for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                     var curView = curNode.pageTabChildren[ii];

                    if (!curView.visible && ((curView.autoOpen && curView.autoOpen.indexOf(currentCompletionState) > -1) || curView.openUp)){
                           this.openTab (curView, -1, true, preObj, true, tab);
                    }

                    if (curView.subTabs)
                        toReturn += this._iterateAlgoJSON (curView.subTabs, tab);
                }
            }
         },
         
         sendEvent: function (name, params) {
             if (params == null)
                 params = {};
             
             $(params.sendingLoc).trigger(name, params);
         },
         
         addTabs: function (whatToAdd, whereToAdd) {
             var options = this.options;
            var oThis = this;
             var actualObjLoc = options.jsonForPage;
             
            if (whereToAdd instanceof Array){
                actualObjLoc = options.jsonForPage.subTabs;
                 for (var i = 0; i < whereToAdd.length-2; i++){
                     if (!actualObjLoc[whereToAdd[i]]){
                         var key = whereToAdd[i];
                         actualObjLoc[key]={};
                     }
                     if (!actualObjLoc[whereToAdd[i]].pageTabChildren){
                         var key = whereToAdd[i];
                         if (!actualObjLoc[key].subTabs) actualObjLoc[key].subTabs = {};
                         actualObjLoc =  actualObjLoc[key].subTabs;
                         break;
                     }
                     
                     actualObjLoc = actualObjLoc[whereToAdd[i]].pageTabChildren;
                 }
                 
                 if (!actualObjLoc[whereToAdd[whereToAdd.length-2]]) actualObjLoc[whereToAdd[whereToAdd.length-2]] = {};
                 actualObjLoc = actualObjLoc[whereToAdd[whereToAdd.length-2]];
                 actualObjLoc.pageTabName = whereToAdd[whereToAdd.length-1];
                 if (!actualObjLoc.pageTabChildren) actualObjLoc.pageTabChildren = [];
            }
            else if (!options.jsonForPage.subTabs[whereToAdd])
                 return false;
            else{
                actualObjLoc = options.jsonForPage.subTabs[whereToAdd];
            }

            $(verarr(whatToAdd)).each(function (index, nObj){
                if ((!nObj.tabId && !nObj.tabName && !nObj.position &&
                        !nObj.viewerConstructor) || oThis.existsTab(nObj.tabId))
                    return false;

                if (nObj.tabOrder != null && nObj.tabOrder <= actualObjLoc.pageTabChildren.length){
                    actualObjLoc.pageTabChildren.splice(nObj.tabOrder, 0, nObj);
                }
                else {
                    actualObjLoc.pageTabChildren.push(nObj);
                }
                 
            });

            this.curMenu.addTabs(whatToAdd, whereToAdd);
            var appendToMenu = this._iterateAlgoJson (options.jsonForPage.subTabs, whereToAdd);
            if (appendToMenu == "")
                 return false;
         },
         
         existsTab: function (lookForTabId, where) {
             if (!where)
                 where = this.options.jsonForPage.subTabs;

             for ( var tab in where )
             {
                 var curNode = where[tab];

                 for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                     var curView = curNode.pageTabChildren[ii];
                     if (curView.tabId == lookForTabId)
                         return curView;

                     if (curView.subTabs)
                     {
                         var ret = this.existsTab (lookForTabId, curView.subTabs);
                         if (ret != false)
                             return ret;
                     }
                 }
             }

             return false;
         }

    })
});

jQuery.getAlgoViewManager = function() {
    if($('.layout-manager.algoviewNew').length == 0)
        console.log('ERROR: cannot find Layout Manager!');
    else
        return $('.layout-manager.algoviewNew').first().algoviewNew('instance');
}