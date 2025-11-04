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
var JSONViewer = (function(document) {
    var Object_prototype_toString = ({}).toString;
    var DatePrototypeAsString = Object_prototype_toString.call(new Date);
    
    function JSONViewer() {
        this._dom_container = document.createElement("pre");
        this._dom_container.classList.add("json-viewer");
    };

    JSONViewer.prototype.showJSON = function(jsonValue, inputMaxLvl, inputColAt) {
        var maxLvl = typeof inputMaxLvl === "number" ? inputMaxLvl : -1;
        var colAt = typeof inputColAt === "number" ? inputColAt : -1;
        
        this._dom_container.innerHTML = "";
        walkJSONTree(this._dom_container, jsonValue, maxLvl, colAt, 0);
    };

    JSONViewer.prototype.getContainer = function() {
        return this._dom_container;
    };

    function walkJSONTree(outputParent, value, maxLvl, colAt, lvl) {
        var isDate = Object_prototype_toString.call(value) === DatePrototypeAsString;
        var realValue = !isDate && typeof value === "object" && value !== null && "toJSON" in value ? value.toJSON() : value;
        if (typeof realValue === "object" && realValue !== null && !isDate) {
            var isMaxLvl = maxLvl >= 0 && lvl >= maxLvl;
            var isCollapse = colAt >= 0 && lvl >= colAt;
            
            var isArray = Array.isArray(realValue);
            var items = isArray ? realValue : Object.keys(realValue);

            if (lvl === 0) {
                var rootCount = _createItemsCount(items.length);
                var rootLink = _createLink(isArray ? "[" : "{");

                if (items.length) {
                    rootLink.addEventListener("click", function() {
                        if (isMaxLvl) return;

                        rootLink.classList.toggle("collapsed");
                        rootCount.classList.toggle("json-node-hide");

                        outputParent.querySelector("ul").classList.toggle("json-node-hide");
                    });

                    if (isCollapse) {
                        rootLink.classList.add("collapsed");
                        rootCount.classList.remove("json-node-hide");
                    }
                }
                else {
                    rootLink.classList.add("empty");
                }

                rootLink.appendChild(rootCount);
                outputParent.appendChild(rootLink);
            }

            if (items.length && !isMaxLvl) {
                var len = items.length - 1;
                var ulList = document.createElement("ul");
                ulList.setAttribute("data-level", lvl);
                ulList.classList.add("type-" + (isArray ? "array" : "object"));

                items.forEach(function(key, ind) {
                    var item = isArray ? key : value[key];
                    var li = document.createElement("li");

                    if (typeof item === "object") {
                        if (!item || item instanceof Date) {
                            li.appendChild(document.createTextNode(isArray ? "" : key + ": "));
                            li.appendChild(createSimpleViewOf(item ? item : null, true));
                        }
                        else {
                            var itemIsArray = Array.isArray(item);
                            var itemLen = itemIsArray ? item.length : Object.keys(item).length;

                            if (!itemLen) {
                                li.appendChild(document.createTextNode(key + ": " + (itemIsArray ? "[]" : "{}")));
                            }
                            else {
                                var itemTitle = (typeof key === "string" ? key + ": " : "") + (itemIsArray ? "[" : "{");
                                var itemLink = _createLink(itemTitle);
                                var itemsCount = _createItemsCount(itemLen);

                                if (maxLvl >= 0 && lvl + 1 >= maxLvl) {
                                    li.appendChild(document.createTextNode(itemTitle));
                                }
                                else {
                                    itemLink.appendChild(itemsCount);
                                    li.appendChild(itemLink);
                                }

                                walkJSONTree(li, item, maxLvl, colAt, lvl + 1);
                                li.appendChild(document.createTextNode(itemIsArray ? "]" : "}"));
                                
                                var list = li.querySelector("ul");
                                var itemLinkCb = function() {
                                    itemLink.classList.toggle("collapsed");
                                    itemsCount.classList.toggle("json-node-hide");
                                    list.classList.toggle("json-node-hide");
                                };

                                itemLink.addEventListener("click", itemLinkCb);

                                if (colAt >= 0 && lvl + 1 >= colAt) {
                                    itemLinkCb();
                                }
                            }
                        }
                    }
                    else {
                        if (!isArray) {
                            li.appendChild(document.createTextNode(key + ": "));
                        }

                        walkJSONTree(li, item, maxLvl, colAt, lvl + 1);
                    }

                    if (ind < len) {
                        li.appendChild(document.createTextNode(","));
                    }

                    ulList.appendChild(li);
                }, this);

                outputParent.appendChild(ulList);
            }
            else if (items.length && isMaxLvl) {
                var itemsCount = _createItemsCount(items.length);
                itemsCount.classList.remove("json-node-hide");

                outputParent.appendChild(itemsCount);
            }

            if (lvl === 0) {
                if (!items.length) {
                    var itemsCount = _createItemsCount(0);
                    itemsCount.classList.remove("json-node-hide");

                    outputParent.appendChild(itemsCount);
                }

                outputParent.appendChild(document.createTextNode(isArray ? "]" : "}"));

                if (isCollapse) {
                    outputParent.querySelector("ul").classList.add("json-node-hide");
                }
            }
        } else {
            outputParent.appendChild( createSimpleViewOf(value, isDate) );
        }
    };

    function createSimpleViewOf(value, isDate) {
        var spanEl = document.createElement("span");
        var type = typeof value;
        var asText = "" + value;

        if (type === "string") {
            asText = '"' + value + '"';
        } else if (value === null) {
            type = "null";
        } else if (isDate) {
            type = "date";
            asText = value.toLocaleString();
        }

        spanEl.className = "type-" + type;
        spanEl.textContent = asText;

        return spanEl;
    };

    function _createItemsCount(count) {
        var itemsCount = document.createElement("span");
        itemsCount.className = "items-ph json-node-hide";
        itemsCount.innerHTML = _getItemsTitle(count);

        return itemsCount;
    };

    function _createLink(title) {
        var linkEl = document.createElement("a");
        linkEl.classList.add("list-link");
        linkEl.href = "javascript:void(0)";
        linkEl.innerHTML = title || "";

        return linkEl;
    };

    function _getItemsTitle(count) {
        var itemsTxt = count > 1 || count === 0 ? "items" : "item";

        return (count + " " + itemsTxt);
    };

    return JSONViewer;
})(document);
