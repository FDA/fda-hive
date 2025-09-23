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
    $.widget("ion.ion_types", {
        options: {
        },

        _create: function () {
            var oThis = this;

            this._buildTypesPanel();
            
            $.ion.ready(function() {
                for(var key in $.ion.types){
                    oThis._showType(key, $.ion.types[key]);
                }
            });
        },

        _buildTypesPanel: function() {
            this.typesContainer = $(document.createElement('div'));

            $(document.createElement('div'))
                .addClass('panel panel-default')
                .append(
                        $(document.createElement('div'))
                            .addClass('panel-heading')
                            .append(
                                    $(document.createElement('h3'))
                                        .addClass('panel-title')
                                        .text('Types')
                            )
                )
                .append(
                        $(document.createElement('div'))
                            .addClass('panel-body')
                            .append(this.typesContainer)
                )
                .appendTo(this.element)
        },
        
        _showType: function (name, type) {
            var div = $(document.createElement('div'))
                .appendTo(this.typesContainer)
                .addClass('panel panel-default')
                .append(
                    $(document.createElement('div'))
                        .addClass('panel-heading')
                        .append(
                            $(document.createElement('h4'))
                                .addClass('panel-title clearfix')
                                .append(
                                    $(document.createElement('a'))
                                        .attr({
                                            'data-toggle': 'collapse',
                                            href: '#type_' + name
                                        })
                                        .text(name)
                                )
                                .append(
                                    $(document.createTextNode(type.title != null ? '  - ' + type.title : ''))
                                )
                                .append(
                                    $(document.createElement('span'))
                                        .addClass('pull-right')
                                        .text(type.descr)
                                )
                        )
                )
                .append(
                    $(document.createElement('div'))
                        .attr({
                            id: 'type_' + name,
                        })
                        .addClass('panel-collapse collapse')
                        .append(
                            $(document.createElement('div'))
                                .addClass('panel-body')
                                .append(
                                    $(document.createElement('div'))
                                        .addClass('row')
                                        .append(
                                            $(document.createElement('div'))
                                                .addClass('col-md-8 recordviewer')
                                        )
                                        .append(
                                                $(document.createElement('div'))
                                                    .addClass('col-md-4')
                                                    .append(
                                                        $(document.createElement('pre'))
                                                            .css({
                                                                overflow: 'auto',
                                                                'max-height': '250px'
                                                            })
                                                            .text(JSON.stringify(type, null, ' '))
                                                       )
                                            )
                                )
                           )
                );

            $('div.recordviewer', div)
                .ion_recordviewer({
                    spec: type,
                    collapsed: false
                });
        }
    });
}(jQuery));

jQuery.grepObject = function(obj, func) {
    var res = [];

    $.map(obj, function(prop, key) {
        if(func != null && func(prop)) {
            var newObj = {};
            newObj[key] = prop;
            res.push(newObj);
        }

        if(prop !== null && typeof prop === 'object')
            res = res.concat($.grepObject(prop, func));
    });
    
    return res;
}

jQuery.keysObject = function(obj) {
    var keys = [];
    for(var key in obj){
        keys.push(key);
    }
    
    return keys;
}