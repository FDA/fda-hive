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
    $.widget("view.algomenuview", {
        options:{
            algoTitle: "",
            myAreaName: "menu",
            neighborName: "viewContent",
            submitButton: "Submit",
            submitButtonName: "menuSubmitButton",
            menuDesc: {
                algoID:[],
                subTabs: {
                    parameters: {
                        visibleDuring: ["preSubmit", "whileRunning"],
                        pageTabName: "Parameters",
                        pageTabChildren: [{
                            tabId: 'advanced',
                            tabName: "Advanced",
                            tabDependents: ["general","system", "batch"],
                            inactive: true
                        },
                        {
                            tabId: 'general',
                            tabName: "General",
                            inactive: false
                        },
                        {
                            tabId: 'system',
                            tabName: "System",
                            inactive: true
                        },
                        {
                            tabId: 'batch',
                            tabName: "Batch",
                            inactive: true
                        }]
                    },
                    progress: {
                        visibleDuring: ["whileRunning"],
                        pageTabName: "Progress",
                        pageTabChildren: [{
                            tabId: 'main',
                            tabName: "Main Progress"
                        },
                        {
                            tabId: 'inputObj',
                            tabName: "Input Objects",
                            position: {posId: 'progress_info', top:'50%%', bottom:'100%', left:'20%', right:'75%'},
                            viewerConstructor: {
                                preObjPos: 1,
                                inactive:true
                            },
                            autoOpen: ["whileRunning"]
                        },
                        {
                            tabId: 'objUsing',
                            tabName: "Objects Using This as Input"
                        },
                        {
                            tabId: 'info',
                            tabName: "Notifications"
                        },
                        {
                            tabId: 'downloads',
                            tabName: "Download"
                        }]
                    },
                    results: {
                        visibleDuring: ["computed"],
                        pageTabName: "Results",
                        pageTabChildren: []
                    }
                }
            }
            
        },
        _create: function() {
            this.options.container = "algomenu" + parseInt(Math.random() * 100000);
            this.element.attr("id", this.options.container);
            this.element.attr("data-name", "algomenu-view")
            
            var oThis = this;

            this.element.append($(document.createElement("div"))
                    .attr("id", "myAlgonav" + this.options.container)
                    .addClass("sidenav")
                    .append($(document.createElement("snap"))
                            .attr("href", "javascript:void(0)")
                            .addClass("sliderbtn")
                            .on("click", function(event){
                                oThis.toggleNav(event);
                            })
                            .append($(document.createElement("i"))
                                    .addClass("icon-chevron-left icomoon-liga")
                                    .attr("aria-hidden", "true")
                            )
                    )
                    .append($(document.createElement("div"))
                            .addClass("rotate sidenav_header")                            
                            .append($(document.createElement("h3"))
                                    .text(this.options.algoTitle)
                            )
                    )
                    .append($(document.createElement("div"))
                            .addClass("sidenav_cont")
                            .append($(document.createElement("div"))
                                    .addClass("file_info")
                                    .append($(document.createElement("button"))
                                            .addClass("accordion")
                                            .attr("type", "button")
                                            .text("Info")
                                            .on("click", function(event){
                                                $(this).toggleClass("active");
                                                $(this).siblings().toggle();
                                                
                                                var totalHeight = 0;
                                                $(this).siblings().each(function(){
                                                    totalHeight += this.scrollHeight;
                                                });
                                                
                                                $(this).siblings().css("max-height", totalHeight);
                                                
                                                event.stopImmediatePropagation();
                                            })
                                    )
                                    .append($(document.createElement("div"))
                                            .addClass("info")
                                            .attr("data-name", "objNameDiv")
                                    )
                            )
                            .append($(document.createElement("div"))
                                    .addClass("sidenav_list")
                                    .attr("data-menuName", "menu-main")
                                    .append($(document.createElement("ul")))
                            )
                    )                    
                    .append($(document.createElement("div"))
                        .attr("data-name", this.options.submitButtonName + "full")
                        .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("side_nav_btn")
                                .on("click", function(event){
                                    if (algoProcess.submitCallback)
                                        algoProcess.submitCallback();
                                    
                                    event.stopImmediatePropagation();
                                    algoProcess.submit();
                                })
                                .append($(document.createElement("snap"))
                                        .addClass("btn_name")                                                
                                        .text(this.options.submitButton)
                                )                                        
                        )
                    )
            );
            
            $("[data-name='" + this.options.submitButtonName + "full']").children("button").prepend(
                    $(document.createElement("i"))
                        .addClass("icon-checkmark icomoon-liga")
            );
            
            this.iterateMenuJson();           
            $(this.element).on ("check-tab", function(event, parameters){
                $("[type='checkbox'][data-name='"+parameters.name+"']").prop("checked", true);
            });
            $(this.element).on ("uncheck-tab", function(event, parameters){
                $("[type='checkbox'][data-name='"+parameters.name+"']").prop("checked", false);
            });
            
            $("div [data-name='objNameDiv']").toggle();


                vjDS["ds"+toolBar].register_callback(this._whatsNextToolbar);
                
        },
        
        toggleNav: function(event){            
            var pageHeight = parseInt($("div.layout-area[data-id='"+this.options.myAreaName+"']").css("height"));
            var contentWidth = parseInt($("div.layout-area[data-id='"+this.options.neighborName+"']").css("width"));
            
            if ($(".sidenav_cont").is(":hidden")) {
                var myRect = {top: 0, left: 0, right: 250, bottom: pageHeight};
                var neighborRect = {top: 0, left: 250, right: (contentWidth+250), bottom: pageHeight};
                var manager = $.getAlgoViewManager();

                manager.moveArea(this.options.neighborName, neighborRect, 0);
                manager.moveArea(this.options.myAreaName, myRect, 0);
                

                $(".sliderbtn").toggleClass("rotate180");
                $("#myAlgonav" + this.options.container).css("width", "250px");
                $(".sidenav_header").toggleClass("sidenav_rotate");
                $(".sidenav_cont").show();
                $(".side_nav_btn snap").show();
                
            } else if ($("div").hasClass("sidenav_cont")) {
                $(".sliderbtn").toggleClass("rotate180");
                $(".sidenav_cont").hide();
                $("#myAlgonav" + this.options.container).css("width", "50px");
                $(".sidenav_header").toggleClass("sidenav_rotate");
                $(".side_nav_btn snap").hide();
                
                var myRect = {top: 0, left: 0, right: 50, bottom: pageHeight};
                var neighborRect = {top: 0, left: 50, right: (contentWidth+250), bottom: pageHeight};
                var manager = $.getAlgoViewManager(); 
                
                manager.moveArea(this.options.myAreaName, myRect, 200);
                manager.moveArea(this.options.neighborName, neighborRect, 200);
            }
        },
        
        iterateMenuJson: function(appendTo){
            var oThis = this;
            var menuDescCur = this.options.menuDesc;
            var useAppend = appendTo;
            
            if (!useAppend) useAppend = "menu-main";
            
            var appendLoc = this.element.find("[data-menuName='"+useAppend+"']").children("ul");
            
            for (var key in menuDescCur.subTabs){
                var curState = menuDescCur.subTabs[key];
                var stateName = curState.pageTabName;
                var visibleDuring = verarr(curState.visibleDuring);
                var icons = curState.icon ? verarr(curState.icon).join(" ") : "icon-"+key;
                
                if (appendTo && key != useAppend) continue;
                                
                var allNodes = $(document.createElement("ul"));
                
                for(var i=0; i < visibleDuring.length; i++)
                    allNodes.addClass(visibleDuring[i]);
                
                for (var i = 0; i < curState.pageTabChildren.length; i++){
                    var curTab = curState.pageTabChildren[i];
                    
                    if(appendLoc.find("[data-name='"+curTab.tabId+"']").length > 0) continue;
                    
                    allNodes.append($(document.createElement("li"))
                            .append($(document.createElement("input"))
                                    .attr("type", "checkbox")
                                    .attr("data-parent", key)
                                    .attr("data-name", curTab.tabId)
                                    .on("click", function(event){
                                        var toPass = {
                                                elem: this,
                                                checked: $(this).is(":checked"),
                                                categ: $(this).attr("data-parent"),
                                                tabId: $(this).attr("data-name")
                                        }
                                        oThis.sendEvent("state-change", toPass);
                                    })
                            )
                            .append($(document.createElement("a"))
                                    .attr("href", "#" + curTab.tabId)
                                    .text(" " + curTab.tabTitle)
                                    .on("click", function(event){
                                        var toPass = {
                                                hash: this.hash
                                        };
                                        oThis.sendEvent("scroll-to", toPass);                                        

                                        event.preventDefault();
                                    })
                            )
                    );
                    
                }
                
                if(appendLoc.find("[data-menuName='"+key+"']").length <= 0 && !appendTo){
                    appendLoc.append($(document.createElement("li"))
                            .attr("data-menuName", key)
                            .append($(document.createElement("label"))
                                    .addClass("tree-toggle")
                                    .text(" " + stateName)
                                    .prepend($(document.createElement("i"))
                                            .addClass(icons)
                                      )
                                    .on("click", function(){
                                        $(this).parent().children("ul").toggle();
                                    })
                            )
                            .append(allNodes)                        
                    );
                }
                else{
                    appendLoc.append(allNodes.children());
                }
                appendLoc.find("ul").toggle();
            }
            
            
        },
        
        _findInJson: function (options, field, value){
            if (options.length < 1)
                options = this.optionsForPage.subTabs;

            for ( tab in options ) {
                  var curNode = options[tab];

                  for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                      var curView = curNode.pageTabChildren[ii];

                      for (smt in curView){
                          if (smt == field && curView[smt] == value)
                              return curView;
                      }

                      if (curView.subTabs){
                          var toReturn = this.findInJson (curView.subTabs, field, value);
                          if (toReturn)
                              return toReturn;
                      }
                  }
            }
        },
        
        addTabs: function (whatToAdd, whereToAdd) {
            var options = this.options;
            var oThis = this;
            var actualObjLoc = this.options.menuDesc;
            
            if (whereToAdd instanceof Array){
                actualObjLoc = options.menuDesc.subTabs;
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
            else if (!options.menuDesc.subTabs[whereToAdd])
                return false;
            else
                actualObjLoc = options.menuDesc.subTabs[whereToAdd];

            $(verarr(whatToAdd)).each(function (index, nObj){
                if ((!nObj.tabId && !nObj.tabName && !nObj.position &&
                        !nObj.viewerConstructor) || oThis._existsTab(nObj.tabId))
                    return false;

                if (nObj.tabOrder != null && nObj.tabOrder <= actualObjLoc.pageTabChildren.length){
                    actualObjLoc.pageTabChildren.splice(nObj.tabOrder, 0, nObj);
                }
                else {
                    actualObjLoc.pageTabChildren.push(nObj);                    
                }                
            });
            
            this.iterateMenuJson(whereToAdd);
        },
        
        _existsTab: function (lookForTabId, where) {
            if (!where)
                where = this.options.menuDesc.subTabs;

            for ( var tab in where ) {
                var curNode = where[tab];

                for (var ii = 0; ii< curNode.pageTabChildren.length; ii++) {
                    var curView = curNode.pageTabChildren[ii];
                    if (curView.tabId == lookForTabId)
                        return curView;

                    if (curView.subTabs) {
                        var ret = this.existsTab (lookForTabId, curView.subTabs);
                        if (ret != false)
                            return ret;
                    }
                }
            }

            return false;
        },
        
        _whatsNextToolbar: function(viewer, content){
            var menuUl = $("#algoMenu");
            var whatNext = $("[data-menuname='whatNext']");
            var locToAdd = whatNext.children("ul");            
            locToAdd.empty();
            
            var tbl = new vjTableView();
            tbl.initTblArr(content);

            var tblArr = tbl.tblArr;
            for (var i = 1; tblArr.rows && i < tblArr.rows.length; i++)
            {
                var url = tblArr.rows[i].cols[7];
                if(url == "") continue;
                locToAdd.append(
                       $(document.createElement("li"))
                                .addClass("link")
                             .append($(document.createElement("a"))
                                     .attr("href", url)
                                     .text(" " + tblArr.rows[i].cols[4])
                             )
                );
            }
        },
        
        hideSubmitButton: function(){
            $("[data-name='" + this.options.submitButtonName + "min']").hide();
            $("[data-name='" + this.options.submitButtonName + "full']").hide();
        },
        
        sendEvent: function (name, params) {
            if (params == null)
                params = {};

            if (name == 'state-change' && params.checked) {
                $(this.element).trigger('open-tab', params);
            }
            else if(name == 'state-change'){
                $(this.element).trigger('close-tab', params);
            }
            else{
                $(this.element).trigger(name, params);
            }
        }
    })
});