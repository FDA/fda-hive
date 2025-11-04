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
    $.widget("layout.news", {

        options: {
            data:[]
        },
        
        _create: function(){
            this.element.addClass("news__container");
            for(var i = 0; i < this.options.data.length; i++){
                this._appendElement(this.options.data[i], totalNewsCount - i);
            }
        },
        
        _appendElement: function (appendElem, appCnt){
            var oThis = this;
            
            if(appendElem.text){
                this.element.append($(document.createElement("div"))
                        .addClass("news__item " + (appCnt > 0 ? "unread" : ""))
                        .append($(document.createElement("figure"))
                                .append($(document.createElement("div"))
                                        .addClass("header accordion")
                                        .append($(document.createElement("h4"))
                                                .addClass("news__title " + (appendElem.importance == 0 ? "low" : "high"))
                                                .text(appendElem.title)
                                        )
                                        .append($(document.createElement("p"))
                                                .addClass("news__date")
                                                .text(appendElem.activation_date ? appendElem.activation_date.substring(0, appendElem.activation_date.indexOf("T")) : appendElem.creation_date.substring(0, appendElem.creation_date.indexOf("T")))
                                        )
                                        .append($(document.createElement("i"))
                                                .addClass("rv-eye view__description")
                                                .on("click", function(){
                                                    oThis._toggleInfo (this);
                                                })
                                        )
                                )
                                .append($(document.createElement("div"))
                                        .addClass("news__description")
                                        .html(appendElem.text.replace(new RegExp("&lt;",'g'), "<").replace(new RegExp("&gt;", 'g'), ">"))
                                )
                        )
                )
            }
            else{
                this.element.append($(document.createElement("div"))
                        .addClass("news__item " + (appCnt > 0 ? "unread" : ""))
                        .append($(document.createElement("figure"))
                                .append($(document.createElement("div"))
                                        .addClass("header accordion")
                                        .append($(document.createElement("h4"))
                                                .addClass("news__title " + (appendElem.importance == 0 ? "low" : "high"))
                                                .text(appendElem.title) 
                                        )
                                        .append($(document.createElement("p"))
                                                .addClass("news__date")
                                                .text(appendElem.activation_date ? appendElem.activation_date.substring(0, appendElem.activation_date.indexOf("T")) : appendElem.creation_date.substring(0, appendElem.creation_date.indexOf("T")))
                                        )
                                )
                        )
                )
            }
        },
        
        _toggleInfo: function(elem){
            $(elem).closest(".unread").removeClass("unread");
            $(elem).toggleClass("rv-eye-no rv-eye");
            
            var description = $(elem).closest("div").siblings(".news__description");
            if (description.css("maxHeight") && parseInt(description.css("maxHeight")) != 0){
                description.css("maxHeight", "0px");
                description.css("display", 'none');
            } else {
                description.css("display", 'block');
                description.css("maxHeight", description[0].scrollHeight + "px");
            }
        }
    });

}(jQuery));