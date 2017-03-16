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
    $.loadScript('jquery/3rd/jsTreeLib/jstree.js');
    $.loadCSS('jquery/3rd/jsTreeLib/themes/default/style.css');    
    $.loadCSS("jsx/widgets/css/view.treewidget.css");
    
    var oThis;
    $.widget("view.treewidget", {
        options:{
            data: "dsVoid",
            dataCounter: 0,
            maintainPreviousData: true,
            hierarchyColumn: "path",
            showRoot: 1,
            autoexpand:0,
            oldContent: "",
            treeStruct: []
        },        
        _create: function () {
            oThis = this;
            
            oThis.options.container = "treeWidgetDiv" + parseInt(Math.random() * 100000);
            oThis.element.append($(document.createElement("div")).attr("id", oThis.options.container)); // add random number generation to the end of this 
            
            oThis.options.data = verarr(oThis.options.data);
            $(oThis.options.data).each(function(i, dsName){
                if (vjDS[dsName].state == "done"){
                    oThis.allReady ({}, vjDS[dsName].data);
                    return;
                }
                
                vjDS[dsName].register_callback(oThis.allReady);
                vjDS[dsName].load();
            });
        },
        allReady: function(viewer, content){
            if (++oThis.options.dataCounter < oThis.options.data.length)
                return;
                
            // If we are using a tree series, the series handles all precompute/postcompute steps
            if (oThis._hasTreeSeries()) {
                oThis.options.tree = vjDS[oThis.options.data[0]].tree;
                oThis.refresh();
                return;
            }

            var content="";
            for(var id = 0; id < oThis.options.data.length ; id++) 
                content += vjDS[oThis.options.data[id]] ? vjDS[oThis.options.data[id]].data : "";

            if (content != oThis.options.oldContent){
                if(!oThis.options.maintainPreviousData  && oThis.options.dataFormat != "ion") {
                    oThis.tree.empty();
                }
                
                oThis._parseContent(content);
                oThis.options.maintainPreviousData=false;
                oThis.tree.enumerate( "if(params.autoexpand && (params.autoexpand=='all' || node.depth<=params.autoexpand) && !node.isNexpandable && node.children && node.children.length>0)node.__expanded=params.expansion;" , {autoexpand:oThis.options.autoexpand, expansion: oThis.options.expansion} ) ;
            }
            oThis.options.oldContent = content; 
            
            if(oThis.postcompute)
                oThis.options.tree.enumerate(oThis.postcompute, oThis);
            
            $("#"+oThis.options.container).remove();
            oThis.element.append($(document.createElement("div")).attr("id", oThis.options.container));
            oThis.refresh();
        },
        _hasTreeSeries: function(){
             var data = vjDS[oThis.options.data[0]];
             return data && data.isTreeSeries;
        },
        _parseContent: function(content){
            if (oThis.options.dataFormat === "newick") {
                if (!oThis.tree)
                    oThis.tree = new vjTree();

                oThis.tree.parseNewick(content, oThis.tree.root);
            } else if (oThis.options.dataFormat === "json" ) {
                if (!oThis.tree)
                    oThis.tree = new vjTree();

                oThis.tree.parseJson(content, oThis.tree.root);
            } else if (oThis.options.dataFormat === "ion" ) {
                if (!oThis.tree)
                    oThis.tree = new vjTree();

                var node;
                eval("node="+content+";");
                
                
                var attach = oThis.findNodeByFakePath(oThis.tree.root, oThis.options.lastSelectedNode);    
                if(!oThis.options.lastSelectedNode){
                    attach = node;
                    oThis.tree.root = jQuery.extend({}, node);
                }
                if(!attach) attach = oThis.tree.root;
                
                
/*                var usefulVars = {};
                for (var key in attach){
                    if (key.indexOf ("__") == 0)
                        usefulVars[key] = attach[key];
                }
                attach = node;*/
                for (var key in attach){
                    if (key.indexOf ("__") != 0)
                        delete attach[key];
                }
                for (var key in node)
                    attach[key] = node[key];
/*                    
                for(var key in usefulVars)
                    attach[key] = usefulVars[key];*/
                
                $("#"+oThis.options.lastSelectedNode).children("span").remove();
                oThis.options.lastSelectedNode = null;
                
                if (oThis.options.onRefreshDoneCallback)
                    oThis.options.onRefreshDoneCallback (attach);
                
                /*oThis.tree.root = node;*/
            }else {
                var tbl=new vjTable(content, 0, vjTable_propCSV);
                tbl.mangleRows(oThis.options.exclusionObjRegex, "delete");
                if(oThis.options.preTreatNodesBeforeParsing)
                    oThis.options.preTreatNodesBeforeParsing(tbl,content);
                if(oThis.options.precompute)
                    tbl.enumerate(oThis.options.precompute, oThis);

                if (oThis.tree)
                    oThis.tree.parseTable(tbl, oThis.tree.root);
                else
                    oThis.tree = new vjTree(tbl,oThis.options.hierarchyColumn);
            }
        },
        
        //actually compose the data structure that will go to jstree in order to get drawn
        //can set flags for DragAndDrop and Checkboxes
        refresh: function(node){            
            var t;
            var refresh_all = false;
            if( node ===undefined || !node.parent) {
                node = oThis.tree.root;
                refresh_all = true;
            }
            treeStruct = {core: {data: []}, plugins: []};
            //compusing the json tree structure
            oThis.options.treeStruct = [];
            t = oThis._outputNodeTreeView( node, 1, "root");
            treeStruct.core.data = t;
            //oThis.treeStruct = t;

            //the actual checkboxing and DnD            
            if (oThis.options.checkBranches || oThis.options.checkLeafs){
                treeStruct.plugins.push("checkbox");

                $('#'+oThis.options.container).on("select_node.jstree deselect_node.jstree", function(event, data) {
                    //node.selected needs to be set
                    //might need to correct for the tri-select state of the checkboxes. do not take care of parents right now
                    var node=oThis.tree.findByPath(data.node.id);
                    
                    node.checked = (event.type == "select_node") ? 1 : 0;

                    var linkCB=0;
                    if(node.leafnode && oThis.options.checkLeafCallback)linkCB=oThis.options.checkLeafCallback;
                    else if(oThis.options.checkBranchCallback)linkCB=oThis.options.checkBranchCallback;

                    if (linkCB)
                        linkCB(oThis, node);
                });
            }

            //if Drag and Drop is activated. The callbacks passed into the widget will check for the information needed.
            if(oThis.options.drag) {
                treeStruct.plugins.push("dnd");
                treeStruct.core.check_callback = true;
                $(document).on('dnd_start.vakata', function (event, data) {
                    //console.log('Started');
                    if (oThis.options.dragCallonStart) oThis.options.dragCallonStart(oThis, data);
                });
                $(document).on('dnd_stop.vakata', function (event, data) {
                    //console.log('Stopped');
                    if (oThis.options.dragCallonStop) oThis.options.dragCallonStop(oThis, data);
                });
            }
            
            if (oThis.options.contextMenuOptions){
                treeStruct.plugins.push("contextmenu");
                var structToReturn = {};
                
                /* 
                 * The contextMenuOptions need to be in this format"
                 * {
                 *         name: name for the structure, to reference by
                 *         title: this is what will appear to the user
                 *         callback: the function that will be called once selected from the list
                 * }
                 */
                
                for (var jj = 0; jj < oThis.options.contextMenuOptions.length; jj++){
                    var curOptions = oThis.options.contextMenuOptions[jj];
                    var func = curOptions.callback;
                    structToReturn[curOptions.name] = {
                         label: curOptions.title,
                         action: function (obj) { 
                             for (var i = 0; i < oThis.options.contextMenuOptions.length; i++){
                                 if (oThis.options.contextMenuOptions[i].title == obj.item.label) 
                                     oThis.options.contextMenuOptions[i].callback(obj);
                             }
                                 
                         }    
                    };
                }
                
                treeStruct["contextmenu"] = {         
                    "items": function($node) { return structToReturn; }
                };
            }
            
            oThis.options.treeJson = treeStruct;
            $('#'+oThis.options.container).jstree(treeStruct);
            //callback for opening a node in a tree
            $('#'+oThis.options.container).on("before_open.jstree", function(eventData, selection){
                var node;
                if (oThis.options.dataFormat == "ion")
                    node = oThis.findNodeByFakePath (oThis.options.treeStruct, selection.node.id);
                else
                    node = oThis.tree.findByPath(selection.node.id);
                
                if (!node) return;
                
                if (oThis.options.dataFormat == "ion"){
                    oThis._additionalTreeButtons(node, selection.node.id, 1);
                }
            });
            $('#'+oThis.options.container).on("after_open.jstree", function(eventData, selection){
                var node;
                if (oThis.options.dataFormat == "ion")
                    node = oThis.findNodeByFakePath (oThis.options.treeStruct, selection.node.id);
                else
                    node = oThis.tree.findByPath(selection.node.id);
                
                if (!node) return;
                node.__expanded = true;
                
                var data = vjDS[oThis.options.data[0]];
                if (data.isTreeSeries) {
                    if (!data.cmdMoreNodes) 
                        data.cmdMoreNodes = oThis.options.cmdMoreNodes;
                    data.getMoreNodes(node);
                } 
                else if (oThis.options.cmdMoreNodes) {
                    oThis.options.maintainPreviousData=true;
                    var url=evalVars(oThis.options.cmdMoreNodes, "$(", ")", node);
                    data.url=url;
                    data.load();
                }

                var linkExpand = 0 ;
                if(!linkExpand && node.expandNodeCallback)
                    linkExpand=node.expandNodeCallback;
                if(!linkExpand)
                    linkExpand=oThis.options.expandNodeCallback;
                if (linkExpand)
                    linkExpand (oThis, node );

                oThis.refresh(node);
                eventData.stopPropagation();
                eventData.preventDefault();
                eventData.cancelBubble=true;
/*                if (oThis.options.dataFormat == "ion"){
                    oThis._additionalTreeButtons(node, selection.node.id, 1);
                }*/
            });
            $('#'+oThis.options.container).on("after_close.jstree", function(eventData, selection){
                var node;
                if (oThis.options.dataFormat == "ion")
                    node = oThis.findNodeByFakePath (oThis.options.treeStruct, selection.node.id);
                else
                    node = oThis.tree.findByPath(selection.node.id);
                
                if (!node) return;
                node.__expanded = false;
            });
            //callback for selecting a node from a tree
            $('#'+oThis.options.container).on("select_node.jstree", function(event, selection){
                var node;
                if (oThis.options.dataFormat == "ion")
                    node = oThis.findNodeByFakePath (oThis.options.treeStruct, selection.node.id);
                else
                    node = oThis.tree.findByPath(selection.node.id);

                if( oThis.options.notSelectable )
                    event.stopPropagation();

                var linkCB;
                linkCB=oThis.options.selectCaptureCallback;
                if(!linkCB && node.url)
                    linkCB=node.url;
                if(!linkCB){
                    if(node.leafnode && oThis.options.linkLeafCallback)
                        linkCB=oThis.options.linkLeafCallback;
                    else if(oThis.options.linkBranchCallback)
                        linkCB=oThis.options.linkBranchCallback;
                }
                if (linkCB)
                    linkCB (oThis, node );
            });
            //Expanding the first n levels of the tree and also all of the nodes that were expanded previously
            $('#'+oThis.options.container).bind('loaded.jstree', function (event, data) {
                var depth = oThis.options.autoexpand ;
                var allToOpen = oThis._accumulateOpenNodes(oThis.tree.root, depth, [], "root_");
                
                for (var ii = 0; ii < allToOpen.length; ii++)
                    $("#"+oThis.options.container).jstree("open_node", "#"+allToOpen[ii]);
                

                if (oThis.options.additionalTreeButtons){
                    oThis._additionalTreeButtons(oThis.tree.root, "root_", 1);
                    
                    $("[name='" + oThis.options.formName + "']").submit(function (eventData){
                        return false;
                    });
                }
            });
            $('#'+oThis.options.container).bind("hover_node.jstree", function (eventData, selection){
                if (oThis.options.dataFormat != "ion") return;
                if (oThis.options.oldHover) $(oThis.options.oldHover).addClass("navigation-span");
                $("#"+selection.node.id+"span").removeClass("navigation-span");
                oThis.options.lastSelectedNode = selection.node.id;
                oThis.options.oldHover = "#"+selection.node.id+"span";
            });
        },
        _outputNodeTreeView: function(node, depth, path, realPath) {
            if (!node) node = oThis.tree.root;
            if (!depth) depth = 1;
            if (!path) path = "_";
            
            if (oThis.options.dataFormat == "ion"){
                var toPush = [];
                
                if(depth <= oThis.options.showRoot && path == "root") 
                {
                    var toPush = {text: "$root", id:"root_", children:[]};
                    oThis.options.treeStruct.push({text: "root", children:[], id:"root_", other: node});

                    var returned = oThis._outputNodeTreeView(node, depth+1, "root_");
                    if (returned) toPush.children = returned;

                    return toPush;
                }
                else{
                    for (var key in node){
                        var tmpKey = oThis.clean(path+key);
                        
                        if (typeof node[key] == "object"){
                            var tmp = {text: key, children:[], id:tmpKey+"_"};
                            oThis.options.treeStruct.push({text: key, children:[], id:tmpKey+"_", other: node[key]});
                            var returned = oThis._outputNodeTreeView(node[key], depth+1, path+key+"_", path+key+"_");
                            if (returned) tmp.children = returned;
                            toPush.push(tmp);
                        }else if (typeof node[key] == "array"){
                            toPush.push({text: key, children:[]});
                            for (var i = 0; i < node[key].length; i++){
                                var returned = oThis._outputNodeTreeView(node[i], depth+1, path+key+"_", path+key+"_");
                                if (returned) toPush.children.push(returned);                            
                            }
                        }else{
                            if (key.indexOf ("__") != 0){
                                toPush.push({text: key + ":" + node[key], id: tmpKey+"_"});
                                oThis.options.treeStruct.push({text: key + ":" + node[key], id: tmpKey+"_", other: node[key]})
                            }
                        }                    
                    }
                    
                    return toPush;
                }
            }else{
                if(!oThis.options.showLeaf && node.leafnode) return {text: node.title, id:node.path};
    
                var cntCh=node.children ? node.children.length : 0 ;
                if (node.childrenCnt>0) cntCh=node.childrenCnt;
                if(oThis.options.hideEmpty && !node.leafnode && cntCh<1 || (node.hidden && node.depth) ) return false;
                
                if(oThis.options.showRoot <= node.depth) 
                {
                    var toPush = {text: (node.title ? node.title : node.name), id:node.path, children:[]};
                    
                    $(node.children).each (function (ii, curChild){
                        var returned = oThis._outputNodeTreeView(curChild, depth+1);
                        if (returned) toPush.children.push(returned);
                    });
                    
                    return toPush;
                }
                else
                {
                    var toPush = [];
                    
                    $(node.children).each (function (ii, curChild){
                        var returned = oThis._outputNodeTreeView(curChild, depth+1);
                        if (returned) toPush.push(returned);
                    });
                    
                    return toPush;
                }
            }
        },
        _additionalTreeButtons: function (node, path, depth){
            if (!node) node = oThis.tree.root;
            if (!path) path = "_";
            if (!depth) depth = 1;
            
            var element;
            
            if (oThis.options.dataFormat == "ion"){
                if(depth <= oThis.options.showRoot && path == "root_") 
                {
                    element = $(document.createElement("span"))
                        .attr("class", "navigation-span")
                        .attr("id", "root_span")
                        .append($(document.createElement("small"))
                                .append($(document.createElement("span"))
                                        .text((node["__start"] == 0 ? "" : " ◄ "))
                                        .attr("style", "cursor: pointer;")
                                        .on ("click", function (eventData, selection){
                                            var nodeId = this.parentElement.closest("span").id;
                                            oThis._onPage(nodeId, "left");
                                        })
                                )
                                .append($(document.createElement("small"))
                                        .text((node["__start"] + 1) + "-" + (node["__start"] + node["__cnt"]) + " of " +  node["__dim"] + "   ")
                                        .append($(document.createElement("select"))
                                            .attr("id", "root_select")
                                            .append($(document.createElement("option"))
                                                    .attr("value", node["__cnt"])
                                                    .attr("selected", true)
                                                    .text(node["__cnt"])
                                            )
                                            .append($(document.createElement("option"))
                                                    .attr("value", 50)
                                                    .text("50")
                                            )
                                            .append($(document.createElement("option"))
                                                    .attr("value", 100)
                                                    .text("100")
                                            )
                                            .append($(document.createElement("option"))
                                                    .attr("value", 1000)
                                                    .text("1000")
                                            )
                                            .append($(document.createElement("option"))
                                                    .attr("value", 1000000)
                                                    .text("all")
                                            )
                                            .change( function (eventData){
                                                //to figure out which options was selected go through:
                                                //eventData.target.options and see which ones selected is set to true
                                                oThis._onPage(eventData.target.id, "select", this.value);
                                            })
                                        )
                                )
                                .append($(document.createElement("span"))
                                        .attr("style", "cursor: pointer;")
                                        .text((node["__start"] + node["__cnt"] < node["__dim"] ? " ►" : " "))
                                        .on ("click", function (eventData, selection){
                                            var nodeId = this.parentElement.closest("span").id;
                                            oThis._onPage(nodeId, "right");
                                        })
                                )
                                .append($(document.createElement("small"))
                                        .attr("style", "height: 20px;")
                                        .append($(document.createElement("input"))
                                                .attr("style", "margin-left:2em;")
                                                .attr("placeholder", "Search")
                                                .attr("size", "30")
                                                .attr("input", "submit")
                                                .bind("enterKey", function (eventData){
                                                    var elem = $("#" + oThis.options.lastSelectedNode + "span").find ("input");
                                                    oThis._onPage(oThis.options.lastSelectedNode+"span", "search", elem.val());
                                                })
                                                .keyup(function(e){
                                                    if(e.keyCode == 13)
                                                        $(this).trigger("enterKey");
                                                })
                                        )
                                )
                        );
                    $("#root_").children("span").remove();
                    //$("#root_").children("ul").prepend();
                    if ($("#root_").children("ul").length) element.insertBefore($("#root_").children("ul"));
                    else $("#root_").append(element);
                    
                    oThis._additionalTreeButtons(node, "root_", depth+1 );
                    return;
                }
                for (var key in node){
                    if (typeof node[key] == "object"){
                        var tmpKey = oThis.clean(path+key);
                        element = $(document.createElement("span"))
                            .attr("class", "navigation-span")
                            .attr("id", tmpKey+"_span")
                            .append($(document.createElement("small"))
                                    .append($(document.createElement("span"))
                                            .text((node[key]["__start"] == 0 ? "" : " ◄ "))
                                            .attr("style", "cursor: pointer;")
                                            .on ("click", function (eventData, selection){
                                                var nodeId = this.parentElement.closest("span").id;
                                                oThis._onPage(nodeId, "left");
                                            })
                                    )
                                    .append($(document.createElement("small"))
                                            .text((node[key]["__start"] + 1) + "-" + (node[key]["__start"] + node[key]["__cnt"]) + " of " +  node[key]["__dim"] + "   ")
                                            .append($(document.createElement("select"))
                                                .attr("id", tmpKey+"_select")
                                                .append($(document.createElement("option"))
                                                        .attr("value", node[key]["__cnt"])
                                                        .attr("selected", true)
                                                        .text(node[key]["__cnt"])
                                                )
                                                .append($(document.createElement("option"))
                                                        .attr("value", 50)
                                                        .text("50")
                                                )
                                                .append($(document.createElement("option"))
                                                        .attr("value", 100)
                                                        .text("100")
                                                )
                                                .append($(document.createElement("option"))
                                                        .attr("value", 1000)
                                                        .text("1000")
                                                )
                                                .append($(document.createElement("option"))
                                                        .attr("value", 1000000)
                                                        .text("all")
                                                )
                                                .on("change", function (eventData){
                                                    //to figure out which options was selected go through:
                                                    //eventData.target.options and see which ones selected is set to true
                                                    
                                                    oThis._onPage(eventData.target.id, "select", this.value);
                                                })
                                            )
                                    )
                                    .append($(document.createElement("span"))
                                            .attr("style", "cursor: pointer;")
                                            .text((node[key]["__start"] + node[key]["__cnt"] < node[key]["__dim"] ? " ►" : " "))
                                            .on ("click", function (eventData, selection){
                                                var nodeId = this.parentElement.closest("span").id;
                                                oThis._onPage(nodeId, "right");
                                            })
                                    )
                                    .append($(document.createElement("small"))
                                            .append($(document.createElement("input"))
                                                    .attr("style", "margin-left:2em;")
                                                    .attr("placeholder", "Search")
                                                    .attr("size", "30")
                                                    .attr("input", "submit")
                                                    .bind("enterKey", function (eventData){
                                                        var elem = $("#" + oThis.options.lastSelectedNode + "span").find ("input");
                                                        oThis._onPage(oThis.options.lastSelectedNode+"span", "search", elem.val());
                                                    })
                                                    .keyup(function(e){
                                                        if(e.keyCode == 13)
                                                            $(this).trigger("enterKey");
                                                    })
                                            )
                                    )
                            );
                        $("#"+tmpKey+"_").children("span").remove();
                        
                        if ($("#"+tmpKey+"_").children("ul").length) element.insertBefore($("#root_").children("ul"));
                        else $("#"+tmpKey+"_").append(element);
                        
                        oThis._additionalTreeButtons(node[key], path+key+"_", depth+1);
                    }else if (typeof node[key] == "array"){
                        oThis._additionalTreeButtons(node[i], path+key+"_", depth+1);
                    }
                }
            }
        },
        _accumulateOpenNodes: function (node, depth, toAccumulate, path){
            if (oThis.options.dataFormat == "ion"){
                if (typeof node != "object") return;
                if (node.__expanded) toAccumulate.push(path);
                
                for (var key in node){
                    oThis._accumulateOpenNodes(node[key], depth, toAccumulate, path+key+"_");
                }
                
                return toAccumulate;  
            }
            else{
                if (node.depth <= depth) toAccumulate.push(node.path);
                else if (node.__expanded) toAccumulate.push(node.path);
                
                for (var ii = 0; node.children && ii < node.children.length; ii++)
                    oThis._accumulateOpenNodes(node.children[ii], depth, toAccumulate);
                
                return toAccumulate;                
            }
        },
        enumerate: function(operation, params, ischecked, leaforbranch, node){
            if(!oThis.tree || !oThis.tree.root)return ;
            return oThis.tree.enumerate(operation, params, ischecked, leaforbranch, node);
        },
        accumulate: function(checker, collector, params, ischecked, leaforbranch, node){
            if(!oThis.tree || !oThis.tree.root)return ;
            return oThis.tree.accumulate(checker, collector, params, ischecked, leaforbranch, node);
        },
        getTree: function(){
            return oThis.tree;
        },
        findJsonByPath: function (node, path){
            if (!node) node = oThis.tree.root;
            if (!path) return node;
            
            if (path.indexOf("_") < 0 && node[path]) return node[path];
            
            var firstPart = path.split("_")[0];
            
            if (!node[firstPart])
                return null;
            
            return oThis.findJsonByPath(node[firstPart], path.substring(path.indexOf("_")+1));
        },
        findNodeByFakePath: function (arrayOfNodes, path){
            if (!arrayOfNodes) arrayOfNodes = oThis.options.treeStruct;
            if (!path) return null;
            
            for (var i = 0; i < arrayOfNodes.length; i++){
                if (arrayOfNodes[i].id == path) return arrayOfNodes[i].other;
            }
            
            return null;
        },
        clean: function (string){
            var tmpStr = string;
            
            while (tmpStr.indexOf("@") > -1) tmpStr = tmpStr.substring (0, tmpStr.indexOf("@")) + tmpStr.substring(tmpStr.indexOf("@")+1);
            while (tmpStr.indexOf("[") > -1) tmpStr = tmpStr.substring (0, tmpStr.indexOf("[")) + tmpStr.substring(tmpStr.indexOf("[")+1);
            while (tmpStr.indexOf("]") > -1) tmpStr = tmpStr.substring (0, tmpStr.indexOf("]")) + tmpStr.substring(tmpStr.indexOf("]")+1);
            
            return tmpStr;
        },
        _onPage: function(nodePathId, direction, params)
        {
            var url = vjDS[oThis.options.data[0]].url;
            var actualNodePath;
            
            if (direction == "select") actualNodePath = nodePathId.substring (0, nodePathId.indexOf("_select"));
            else actualNodePath = nodePathId.substring (0, nodePathId.indexOf("_span"));
            
            //oThis.options.lastSelectedNode = actualNodePath;
            
            var node = oThis.findNodeByFakePath(oThis.options.treeStruct, actualNodePath+"_");
            if (!node) return;
            
            url=urlExchangeParameter(url, "sub", node.__sub);
            if (direction == "select"){
                url=urlExchangeParameter(url, "brCnt", params );
                
                //cmd=brgetjson&sub=$root&brCnt=all&brStart=0&brSearchTotals=1

            }
            else if (direction == "right"){
                var brStart=node.__start+node.__cnt;
                if(brStart>=node.__dim) brStart=node.__dim-node.__cnt;
                
                url=urlExchangeParameter(url, "brStart", brStart);
                url=urlExchangeParameter(url, "sub", node.__sub);
            }
            else if (direction == "left"){
                var brStart=node.__start-node.__cnt;
                if(brStart<0)brStart=0;
                
                url=urlExchangeParameter(url, "brStart", brStart);
                url=urlExchangeParameter(url, "sub", node.__sub);
            }
            else{ //for search
                if(params[0] == ':') {
                    url=urlExchangeParameter(url, "brInto", escape(params.substring(1)));
                    url=urlExchangeParameter(url, "brSearch", "-" );
                } else { 
                    url=urlExchangeParameter(url, "brSearch", escape(params));
                    url=urlExchangeParameter(url, "brInto", "-" );
                }
/*                url=urlExchangeParameter(url, "sub", node.__sub);
                this.searchDic[node.__path]=o.value;
                this.getData(0).reload(url,true);
                return 1;
                console.log("search");*/
            }
            
            vjDS[oThis.options.data[0]].reload(url, true);
            return;
        }
    });
}(jQuery));


