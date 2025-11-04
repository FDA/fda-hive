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
    $.widget("view.menuportal", {

        options: {
                data: "dsVoid",
                menuNodeName: "tools",
                usedParams: ["root"]
        },

        _create: function() {
                this.options.menuNodeName = this.options.menuNodeName.toLowerCase();
            
                this.options.container = "menuportal" + parseInt(Math.random() * 100000);
                this.element.attr("id", this.options.container);
                this.element.css("overflow", "auto");
                
                vjDSNew.registerWidget(this.options.data, this);
        },
        
        draw: function(widget, dsStruct, content){
                var tree = new vjTree (content);
                if (!tree.root || !tree.root.children || tree.root.children.length < 1) return;
                
                var i;
                for (i = 0; i < tree.root.children.length; i++){
                    if(tree.root.children[i].name.toLowerCase() == this.options.menuNodeName)
                        break;
                }
                if(i == tree.root.children.length ){
                    this.element.append($(document.createElement("div"))
                            .addClass("center-wrap")
                            .attr("data-id", "menu-portal")
                            .text("No items found with root \"" + this.options.menuNodeName + "\"")
                            );
                    return;
                }
                
                if( document.URL.indexOf("&") > 0)
                    this.options.urlExtra = document.URL.substring(document.URL.indexOf("&"));
                else
                    this.options.urlExtra = "";
                
                for(var ii = 0; ii < this.options.usedParams.length; ii++){
                    this.options.urlExtra = urlExchangeParameter(this.options.urlExtra, this.options.usedParams[ii], "-");
                }
                
                this.options.root = tree.root.children[i];
                this.options.menu = tree.root.children[i];
                this.selectedNodes = new Array;
                this.selectedNodes.push(this.options.menu);
                
                if(this.options.selection){
                    this.selectedArr = this.options.selection.split("/");
                    
                    for(var ii = 0; ii < this.selectedArr.length; ii++){
                        var i;
                            for (i = 0; i < this.options.menu.children.length; i++){
                                if(this.options.menu.children[i].name.toLowerCase() == this.selectedArr[ii].toLowerCase())
                                    break;
                            }
                            if(i == this.options.menu.children.length ){
                                window.location = urlExchangeParameter(document.URL, "selected", "-");
                            }
                            
                            this.options.menu = this.options.menu.children[i];
                            this.selectedNodes.push(this.options.menu);
                    }
                }
                
                this._prepareUrl(this.options.root);
                
                this.element.append($(document.createElement("div"))
                        .addClass("center-wrap")
                        .attr("data-id", "menu-portal")
                        );
                this._generateMenu(this.options.menu);
        },
        
        _generateMenu: function(curRoot){
                var oThis = this;
                
                var appendToMain = $("[data-id='menu-portal']");
                var appendToSection = $("div .category-sections");
                var breadcrumUl = $(".breadcrumb");
                
                if(breadcrumUl.length == 0){
                    breadcrumUl = $(document.createElement("ul"))
                                .addClass("breadcrumb");
                }
                else
                    breadcrumUl.empty();
                
                var i = 0;
                for (; i < this.selectedNodes.length-1; i++){
                    var url = this.selectedNodes[i].url
                    if(this.options.urlExtra.length > 2)
                        url += this.options.urlExtra;
                    
                    var title = this.selectedNodes[i].title;
                    title = title.replace(new RegExp("__", 'g'), "&");
                    title = title.replace(new RegExp("_", 'g'), " ");
                    title = title.replace(new RegExp("&", 'g'), "_");
                    breadcrumUl.append($(document.createElement("li"))
                            .append($(document.createElement("a"))
                                    .attr("href", url)
                                    .text(title)
                            )
                    );                    
                }
                var title = this.selectedNodes[i].title;
                title = title.replace(new RegExp("__", 'g'), "&");
                title = title.replace(new RegExp("_", 'g'), " ");
                title = title.replace(new RegExp("&", 'g'), "_");
                breadcrumUl.append($(document.createElement("li"))
                        .append($(document.createElement("a"))
                                .text(title)
                        )
                );    
                
                if(appendToSection.length == 0){
                    $("[data-id='menu-portal']").append($(document.createElement("div"))
                            .addClass("search-container")
                            .append($(document.createElement("form"))
                                    .append($(document.createElement("input"))
                                            .attr("type", "search") 
                                            .attr("placeholder", "Search ...")
                                            .attr("name", "menuportal-search")
                                            .keypress (function (event){
                                                if(event.which == 13) {
                                                    oThis._onSearch();
                                                    event.preventDefault();
                                                }
                                            })
                                    )
                                    .append($(document.createElement("button"))
                                            .attr("type", "submit")
                                            .append($(document.createElement("i"))
                                                    .addClass("rv-search")
                                            )
                                            .click(function(event){
                                                oThis._onSearch();
                                                event.preventDefault();
                                            })
                                    )
                            )
                    );

                    appendToMain.append(breadcrumUl);
                    appendToSection = $(document.createElement("div"))
                            .addClass("category-sections");                    
                }
                appendToSection.empty();
                
                var currentSorted = this._sort(curRoot.children);
                var hasChildren = this._checkLevels(curRoot) == 2;
                
                if(!hasChildren){
                    if (currentSorted.length <= 17){
                        var ul = $(document.createElement("ul"))
                                .addClass("category-section");
                        for(var ii = 0; ii < currentSorted.length; ii++){
                            var href = "";
                            if(currentSorted[ii].url) href = currentSorted[ii].url;
                            if(this.options.urlExtra.length > 2) href += this.options.urlExtra;
                            
                            var title = currentSorted[ii].title;
                            title = title.replace(new RegExp("__", 'g'), "&");
                            title = title.replace(new RegExp("_", 'g'), " ");
                            title = title.replace(new RegExp("&", 'g'), "_");
                            ul.append($(document.createElement("li"))
                                    .append($(document.createElement("a"))
                                        .attr("href", href) 
                                        .attr ("name", currentSorted[ii].name)
                                        .text(title + (currentSorted[ii].children && currentSorted[ii].children.length ? " (" + currentSorted[ii].children.length + ")" : ""))
                                        .attr("title", currentSorted[ii].description)
                                    )
                            );
                        }
                        appendToSection.append(ul);
                    }
                    else{
                        this._moreThan17 (currentSorted, appendToSection, false);
                    }
                }
                else{
                    if(currentSorted.length > 17){
                        this._moreThan17 (currentSorted, appendToSection, true);
                    }
                    else{
                        var selection;
                        for(var i = 0; i < currentSorted.length; i++){
                            var href = "";
                            
                            if(currentSorted[i].path){
                                selection = currentSorted[i].path.substring(currentSorted[i].path.substring(1).indexOf("/")+2);
                            }
                            if(currentSorted[i].url) href = currentSorted[i].url;
                            else{
                                var tmp; 
                                if(selection && selection[selection.length-1] == "/"){
                                    tmp = selection.substring(0, selection.length -1);
                                }
                                else tmp = currentSorted[i].name;
                                
                                var url = document.URL;
                                href = url.substring(url.indexOf("?"));
                                href = urlExchangeParameter(href, "selected", tmp);
                            }
                            if(this.options.urlExtra.length > 2) href += this.options.urlExtra;
                            
                            var title = currentSorted[i].title;
                            title = title.replace(new RegExp("__", 'g'), "&");
                            title = title.replace(new RegExp("_", 'g'), " ");
                            title = title.replace(new RegExp("&", 'g'), "_");
                            var ul = $(document.createElement("ul"))
                                .addClass("category-section");
                            ul.append($(document.createElement("a"))
                                    .attr("href", href) 
                                    .attr ("name", currentSorted[i].name)
                                    .text(title)
                                    .attr("title", currentSorted[i].description)
                                    .addClass(currentSorted[i].children && currentSorted[i].children.length ? "list__title" : "")
                                );
                            
                            var currentChildrenSorted = this._sort(currentSorted[i].children)
                            var ii;
                            for (ii = 0; ii < currentChildrenSorted.length && ii < 17; ii++ ){
                                var href2 = "";
                                if(currentChildrenSorted[ii].url && !currentChildrenSorted[ii].children && !currentChildrenSorted[ii].children.length < 1) href2 = currentChildrenSorted[ii].url;
                                else{
                                    var selection2 = selection + currentChildrenSorted[ii].name;
                                    href2 = urlExchangeParameter(href, "selected", selection2);
                                }
                                if(this.options.urlExtra.length > 2) href2 += this.options.urlExtra;
                                
                                var title = currentChildrenSorted[ii].title;
                                title = title.replace(new RegExp("__", 'g'), "&");
                                title = title.replace(new RegExp("_", 'g'), " ");
                                title = title.replace(new RegExp("&", 'g'), "_");
                                ul.append($(document.createElement("li"))
                                    .append($(document.createElement("a"))
                                            .attr("href", href2) 
                                            .attr ("name", currentChildrenSorted[ii].name)
                                            .text (title + (currentChildrenSorted[ii].children && currentChildrenSorted[ii].children.length ? " (" + currentChildrenSorted[ii].children.length + ")" : ""))
                                            .attr("title", currentChildrenSorted[ii].description)
                                    )    
                                );
                            }
                            if (ii < currentChildrenSorted.length)
                                ul.append($(document.createElement("li"))
                                        .append($(document.createElement("a"))
                                                .attr("href", href)
                                                .text ("See more ...")
                                        )    
                                    );
                                
                            appendToSection.append(ul);
                        }
                    }
                    
                }
                appendToMain.append(appendToSection);             
        },
        
        _moreThan17: function(drawFrom, appendTo, withChildren){
            var curLetter = "a";
            var currentUl;
            var currentUlNoChild = $(document.createElement("li"))
                                    .addClass("category-section alphabetizing")
                                    .attr("alt", "a")
                                    .appendTo(appendTo);
            
            for(var ii = 0; ii < drawFrom.length; ii++){
                var href = "";
                if(drawFrom[ii].url) href = drawFrom[ii].url;
                if(this.options.urlExtra.length > 2) href += this.options.urlExtra;
                
                var title = drawFrom[ii].title;
                title = title.replace(new RegExp("__", 'g'), "&");
                title = title.replace(new RegExp("_", 'g'), " ");
                title = title.replace(new RegExp("&", 'g'), "_");
                
                if(withChildren){
                    appendTo.append(currentUl);
                    currentUl = $(document.createElement("ul"))
                        .addClass("category-section alphabetizing")
                        .attr("alt", "a")
                        .append($(document.createElement("li"))
                            .append($(document.createElement("a"))
                                    .attr("href", href)
                                    .text(title + (drawFrom[ii].children && drawFrom[ii].children.length ? " (" + drawFrom[ii].children.length + ")" : ""))
                                    .attr("title", drawFrom[ii].description)
                            ));
                }
                else{
                    currentUlNoChild.append($(document.createElement("li"))
                            .append($(document.createElement("a"))
                                .attr("href", href) 
                                .attr ("name", drawFrom[ii].name)
                                .text(title + (drawFrom[ii].children && drawFrom[ii].children.length ? " (" + drawFrom[ii].children.length + ")" : ""))
                                .attr("title", drawFrom[ii].description)
                            )
                    );
                }
                

                       
            }
            appendTo.append(currentUl); 
        },
        
        _showAll: function(){
            
        },
        
        _onSearch: function (){
            var searchVal = $("[name='menuportal-search'").val();
            if(searchVal == ""){
                this._generateMenu(this.options.menu);
                return;
            }
            var breadcrumUl = $(".breadcrumb");
            
            if(breadcrumUl.length > 0){
                var url = this.selectedNodes[0].url
                if(this.options.urlExtra.length > 2)
                    url += this.options.urlExtra;
                
                breadcrumUl.empty();
                breadcrumUl.append($(document.createElement("li"))
                        .append($(document.createElement("a"))
                                .attr("href", url)
                                .text(this.selectedNodes[0].title)
                        )
                );
            }

            var allFound = this._findAll(searchVal.toLowerCase(), this.options.root.children);
            
            allFound = this._sort(allFound);
            var ul = $(document.createElement("ul")).addClass("category-section searched");
            
            if(allFound.length == 0)
                ul.append($(document.createElement("li"))
                    .append($(document.createElement("a"))
                            .text("No Results on your search: " + searchVal)
                    )    
                );
            else{
                for(var i = 0; i < allFound.length; i++){
                    var url = allFound[i].url
                    if(this.options.urlExtra.length > 2)
                        url += this.options.urlExtra;
                    
                    var title = allFound[i].title;
                    title = title.replace(new RegExp("__", 'g'), "&");
                    title = title.replace(new RegExp("_", 'g'), " ");
                    title = title.replace(new RegExp("&", 'g'), "_");
                    ul.append($(document.createElement("li"))
                        .append($(document.createElement("a"))
                                .attr("href", url)
                                .text(title)
                        )    
                    );
                }
            }  
            
            var catSec = $("div .category-sections");
            catSec.empty();
            catSec.append(ul);
        },
        
        _findAll: function (searchStr, curArr){
            if(!curArr) return [];
            var toReturn = [];
            
            for(var i = 0; i < curArr.length; i++){
                var child = curArr[i];
                var title = child.title;
                title = title.replace(new RegExp("__", 'g'), "&");
                title = title.replace(new RegExp("_", 'g'), " ");
                title = title.replace(new RegExp("&", 'g'), "_");
                if(title.toLowerCase().indexOf(searchStr) >= 0)
                    toReturn.push(child);
                
                var tmp = this._findAll(searchStr, child.children);
                toReturn = toReturn.concat(tmp);
            }
            
            return toReturn;
            
        },
        
        _sort: function(curArr){
                return curArr.sort(function(a,b){
                    if(a.order != undefined && b.order != undefined){
                        if(a.order < b.order) return -1;
                        if(a.order > b.order) return 1;                        
                    }
                    else if (a.order == undefined && b.order != undefined) return 1;
                    else if (b.order == undefined && a.order != undefined) return -1;
                    
                    if(a.title.toLowerCase() < b.title.toLowerCase()) return -1;
                    if(a.title.toLowerCase() > b.title.toLowerCase()) return 1;
                    return 0;
                });
        },
        
        _checkLevels: function (curRoot){
                for (var i = 0; i < curRoot.children.length; i++){
                    if (curRoot.children[i].children && curRoot.children[i].children.length > 0)
                        return 2;
                }
                
                return 1;
        },
        
        _prepareUrl: function (root){
            if(!root.url){
                root.url = document.URL;
                var path = root.path;
                var index = path.substring(1).indexOf("/");

                if(index > 0 && index != path.length - 2){
                    if(path[path.length - 1] == "/")
                        root.url = urlExchangeParameter(root.url, "selected", path.substring(index+2, path.length-1));
                    else
                        root.url = urlExchangeParameter(root.url, "selected", path.substring(index+2, path.length));
                }
                else
                    root.url = urlExchangeParameter(root.url, "selected", "-");
            }
            
            for(var i = 0; i < root.children.length; i++)
                this._prepareUrl(root.children[i]);
        }
    })
});