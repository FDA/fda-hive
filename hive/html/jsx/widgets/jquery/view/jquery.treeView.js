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

(function($){
    $.widget("hive.treeView", {
        options: {
            treeName: "",
            data: "dsVoid",
            divArea: "",
            onlyLeafNodes: false,
            treeContentDiv: ("treeContent" + parseInt(Math.random() * 100000)),
            treeClasses: [["", "fa-sitemap"], ["child", "fa-sitemap"], ["grandchild", "fa-user"]]
        },
        _create: function(){                        
            vjDSNew.registerWidget(this.options.data, this);
        },
        draw: function (widget, dsStruct, content){
            this.options.tree = new vjTree(content);
            
            this._setUpArea();
            this.constructTree($("#" + this.options.treeContentDiv), this.options.tree.root, this.options.treeClasses);
            
            this._on (false,  $( ".tree"), {click: "_treeClick"});
            this._treeClick = function(event) {
                var element = event.currentTarget;
                var node = this.options.tree.findByName(element.id);
                if (! event.ctrlKey && !$(element).hasClass("selected")){
                    $( ".tree").removeClass("selected");
                }
                $( element ).toggleClass( "selected" );
                
                if (!event.preSelect){
                    var usrId = $(element).attr("treeid");
                    var name = $(element).attr("id");
                }

                if (!event.ctrlKey)
                    $( "[treeselector='usrTreeUl']" ).removeClass( "selected_inherited" );
                
                if(this.options.clickCallback){
                    this.options.clickCallback(this, element, node);
                }
                event.preventDefault();
            };
        },
        _setUpArea: function(){
            var appendTo = this.element;
            appendTo.append($(document.createElement("div"))
                    .addClass("treesCols sharingcols")
                    .append($(document.createElement("div"))
                            .addClass("boxBasic")
                            .attr({"style": "height: " + this.element.height() + "px;"})
                            .append($(document.createElement("div"))
                                    .addClass("row ban_top")
                                    .append($(document.createElement("div"))
                                            .addClass("col-xs-5")
                                            .append($(document.createElement("h5"))
                                                    .text(this.options.treeName)
                                            )
                                    )
                                    .append($(document.createElement("div"))
                                            .addClass("col-xs-5")
                                            .append($(document.createElement("div"))
                                                    .addClass("input-group")
                                                    .append($(document.createElement("div"))
                                                            .addClass("input-group-btn")
                                                            .append($(document.createElement("a"))
                                                                    .attr("href", "")
                                                                    .addClass("btn")
                                                                    .append($(document.createElement("i"))
                                                                            .addClass("fa fa-search")
                                                                    )
                                                                    .on("click", function(){
                                                                        event.preventDefault();
                                                                    })
                                                            )
                                                    )
                                                    .append($(document.createElement("input"))
                                                            .attr("type", "text")
                                                            .attr("placeholder", "Search")
                                                            .addClass("form-control")
                                                            .on("keyup", function(){
                                                                var filter = this.value.toUpperCase();
                                                                var li = $("[treeSelector='usrTreeLi']");

                                                                $(li).each(function (elem, i){
                                                                    var text = this.textContent;
                                                                    if (text.toUpperCase().indexOf(filter) > -1) {
                                                                        this.style.display = "";
                                                                    } else {
                                                                        this.style.display = "none";
                                                                    }
                                                                    
                                                                });
                                                            })
                                                    )
                                            )
                                    )
                            )
                            .append($(document.createElement("div"))
                                    .addClass("content con-pad ")
                                    .attr("id", this.options.treeContentDiv)
                            )
                    )
            );
        },
        constructTree: function (curLoc, node, treeClasses){
            if (node.depth == 0){
                for (var i = 0; i < node.children.length; i++){
                    this.constructTree(curLoc, node.children[i], treeClasses);
                }
                return;
            }
            
            var iClass = "fa-chevron-right rotate_90";
            if (!node.children || node.children.length == 0) iClass = "fa-user";
            
            var index = node.depth -1 ;
            if(index > treeClasses.length - 1) index = treeClasses.length - 1;
            
            var ii = $(document.createElement("i"))
                    .attr("style", "margin-right:8px;")
                    .addClass("fa " + iClass)
                    .attr("name", node.name);
            var li = $(document.createElement("li"))
                    .append($(document.createElement("p"))
                            .addClass ("tree treeNode")
                            .attr("style", "padding-left:" + (20*node.depth) + "px;")
                            .text(" "+node.title)
                            .attr("id", node.name)
                            .attr("treeId", node.id)
                            .attr("treeSelector", "usrTreeLi")
                    );
            li.children()
                .prepend(ii);
            curLoc.append(li);
            
            if (!node.children || node.children.length < 1) {
                return;
            }

            ii.click(function(){
                var selId = $(this).attr("name");
                $("[name='" + selId + "-ul").toggle();
                $(this).toggleClass("down");
                event.stopPropagation();
            });
            
            var ul = $(document.createElement("ul"))
                        .attr("name", node.name + "-ul")
                        .attr("treeSelector", "usrTreeUl");
            li.append(ul);
            
            for (var i = 0; i < node.children.length; i++){
                this.constructTree(ul, node.children[i], treeClasses);
            }
            return;
        },
        collapseTree: function (node){
            if (!node.children || node.children.length == 0) return;
            
            for (var i = 0; i < node.children.length; i++){
                this.collapseTree(node.children[i]);
            }
            
            if (node.expanded == false)
                $("#"+node.name).parent().children("ul").toggle();
            else
                $("#"+node.name).parent().find("i.fa.fa-chevron-right").first().toggleClass("down");
        }
    });    
} (jQuery));
    