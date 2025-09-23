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
    $.widget("recordviewer.recordviewer2", {
        options: {
            propSpecUrl: "http:
            propGetUrl: "http:
            collapsed: false,
            style: 'inline'
        },
        
        _create: function () {
            var oThis = this;

            this.element.addClass('record-viewer').addClass(this.options.style);
            this.element.css("overflow", "auto");
            
            this.container = "recordViewer" + parseInt(Math.random() * 100000);
            this.recContainer = "recList" + parseInt(Math.random() * 100000);
            this.element.attr("id", this.options.container);
            
            this.dataSpec = "dsRVSpec" + this.container;
            this.dataGet = "dsRVGet" + this.container;
            this.options.propSpecUrl += "&type=" + this.options.type;
            if(this.options.objId) this.options.propGetUrl += "&ids=" + this.options.objId;
            vjDSNew.addDS("", this.dataSpec, this.options.propSpecUrl);
            if(this.options.objId) vjDSNew.addDS("", this.dataGet, this.options.propGetUrl);
            vjDSNew.registerWidget ([this.dataSpec, this.dataGet], this);
        },
        
        draw: function(widget, dsStruct, content){
            var tree = new vjTree(vjDS[this.dataSpec].data);
            var typeDesc;
            var oThis = this;
            
            try{
                typeDesc = JSON.parse(vjDS[this.dataSpec].data);
            } catch (e){
                console.log("The datasource " + this.data + " returned poorly formatted JSON");
                return false;
            }
            
            this._sortJson (typeDesc, "_field")
            
            this.recordTree = typeDesc;            
            
            this.element.append($(document.createElement("div"))
                    .addClass("row")
                    .append($(document.createElement("div"))
                            .addClass("col-xs-5")
                            .append($(document.createElement("h5"))
                                    .text(typeDesc.title)
                            )                            
                    )
                    .append($(document.createElement("div"))
                            .addClass("col-xs-4")
                            .append($(document.createElement("input"))
                                    .attr("type", "text")
                                    .attr("placeholder", "Search")
                            )                            
                    )
                    .append($(document.createElement("div"))
                            .addClass("col-xs-1")
                            .append($(document.createElement("button"))
                                    .attr("data-typename", typeDesc.name)
                                    .attr("data-container", this.recContainer)
                                    .attr("type", "button")
                                    .text("Clear All")
                                    .on("click", function(){
                                        console.log("clear");
                                    })
                            )                            
                    )
                    .append($(document.createElement("div"))
                            .addClass("col-xs-1")
                            .append($(document.createElement("button"))
                                    .attr("type", "button")
                                    .text("Save Record")
                            )                            
                    )
                    .append($(document.createElement("div"))
                            .addClass("col-xs-1")
                            .append($(document.createElement("a"))
                                    .text("Help")
                            )                            
                    )
            );
            
            var element = $(document.createElement("ul"))
                .addClass("col-md-6")
                .attr("data-typeName", typeDesc.name)
                .attr("data-typeDesc", typeDesc._id);
            
            this.element.append(element);
            
            typeDesc.path = this.container + "." + typeDesc.name;
            
            element.record_list({
                spec: typeDesc,
                container: this.recContainer
            });
            
            this.recordViewer = element.data("recordviewer-record_list");
            
            this.updateFields();
            
            return true;
        },
        
        _sortJson: function (jsonToSort, childrenName){
                       
            for(var key in jsonToSort){
                if(key == childrenName){
                    var arr = [];
                    
                    for(var keyin in jsonToSort[key]){
                        var tmpObj = jsonToSort[key][keyin];
                        tmpObj.tmpObjName = keyin;
                        arr.push(tmpObj);
                        
                        this._sortJson(tmpObj, childrenName);
                    }
                    
                    arr.sort(this._sortFunc);
                    jsonToSort[key] = arr;
                }
            }
        },
        
        _sortFunc: function (first, second){
            if((first._order || first._order == 0) && (second._order || second._order == 0)){
                if(first._order < second._order) return -1;
                else if (first._order > second._order) return 1;                
                return 0;
            }
            
            if(first.title < second.title) return -1;
            else if (first.title > second.title ) return 1;            
            return 0;
        },
        
        updateFields: function(){
            if(!vjDS[this.dataGet].data) return;
            
            if(!this.allFields){
                try{
                    var fieldJson = JSON.parse(vjDS[this.dataGet].data);
                }catch (e){
                    console.log("prop get returner poorly formatted JSON");
                    return;
                }
                this.allFields = fieldJson
                console.log(fieldJson);
            }
            
            this.recordViewer.updateFields(this.allFields);
            
        },
        
        updateObject: function(){
            
        },
        
        iterate: function(){
            
        }
    });
}(jQuery));
