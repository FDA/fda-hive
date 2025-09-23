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
    $.widget("ion.ion_primitives", {
        options: {
        },

        _create: function () {
            var oThis = this;

            this._buildPrimitivesPanel();

            $.ion.ready(function() {
                for(var key in $.ion.core) {
                    oThis._showPrimitive(key, $.ion.core[key]);
                }
            });
        },

        _buildPrimitivesPanel: function() {
            this.primitivesTable = $(document.createElement('tbody'));
            
            $(document.createElement('div'))
                .addClass('panel panel-default')
                .append(
                        $(document.createElement('div'))
                            .addClass('panel-heading')
                            .append(
                                    $(document.createElement('h3'))
                                        .addClass('panel-title')
                                        .text('Primitives')
                            )
                )
                .append(
                        $(document.createElement('div'))
                            .addClass('panel-body')
                            .append(
                                $(document.createElement('table'))
                                    .addClass('table table-striped')
                                    .append(
                                            $(document.createElement('thead'))
                                                .append(
                                                    $(document.createElement('tr'))
                                                        .append(
                                                                $(document.createElement('th'))
                                                                    .text('Type')
                                                        )
                                                        .append(
                                                                $(document.createElement('th'))
                                                                    .text('Field')
                                                        )
                                                )
                                    )
                                    .append(this.primitivesTable)
                            )
                )
                .appendTo(this.element)
        },
        
        _showPrimitive: function(name, type) {
            var fieldTd = $(document.createElement('td'));

            switch (name) {
                case 'int':
                    fieldTd.field_int();
                    break;
                case 'uint':
                    fieldTd.field_uint();
                    break; 
                case 'memory':
                    fieldTd.field_memory();
                    break; 
                case 'string':
                    fieldTd.field_string();
                    break; 
                case 'hiveid':
                    fieldTd.field_hiveid();
                    break; 
                case 'password':
                    fieldTd.field_password();
                    break; 
                case 'email':
                    fieldTd.field_email();
                    break; 
                case 'bool':
                    fieldTd.field_bool();
                    break; 
                case 'text':
                    fieldTd.field_text();
                    break; 
                case 'cmdline':
                    fieldTd.field_cmdline();
                    break; 
                case 'datetime':
                    fieldTd.field_datetime();
                    break; 
                default: 
                    fieldTd.text(name + ': not implemented yet!');
                    break;
            }
            
            this.primitivesTable.append(
                    $(document.createElement('tr'))
                        .append(
                                $(document.createElement('td'))
                                    .text(name)
                        )
                        .append(fieldTd)
            );
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