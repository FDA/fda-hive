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
//# sourceURL=jquery.record.choice.js

$(function () {
    $.widget("recordviewer.record_choice", $.recordviewer.record_base, {

        options: {
        },

        _create: function () {
            this._super();
        },

        _prepareData: function(callback) {
            this.items = [];
            
            if(this.isStatic()) {
                this._parseStaticContent();
                
                if(callback != null)
                    callback();
            }
            else if(this.isDynamic()){
                this._parseDynamicContent(function() {
                    if(callback != null)
                        callback();
                });
            }
        },

        _parseStaticContent: function() {
            var oThis = this;

            $(this.options.spec.constraint_data.split('|')).each(function(index, option) {
                var vals = option.split('///');

                oThis.items.push({
                    id: vals[0],
                    name: vals[1] || vals[0]
                })
            });
        },

        _parseDynamicContent: function(callback) {
            var oThis = this;

            var config = this.options.spec.constraint_data.toJsObject();

            Papa.parse(config.url.replace('http://', 'dna.cgi'), {
                download: true,
                header: true,
                complete: function(data) {
                    $(data.data).each(function(index, item) {
                        oThis.items.push({
                            id: item.id,
                            name: item.name
                        })
                    });

                    if(callback != null)
                        callback();
                }
            })
            
            console.log(config)
        },

        generateField: function() {
            var oThis = this;
            
            this.titleField = $(document.createElement('input')).attr({ type: 'text', readonly: 'readonly' });

            this.valueField = $(document.createElement('input')).attr({ type: 'hidden' });

            this.field = $(document.createElement('div'))
                            .addClass('field choice')
                            .addClass(this.options.spec.type)
                            .attr({
                                id: this.options.name,
                                name: this.options.name
                            })
                            .append( this.valueField )
                            .append( this.titleField )
                            .append(
                                $(document.createElement('span')).addClass("caret")
                            );

            if(!this.isReadonly()) {
                var popup = this._generatePopup();

                this.field.append(popup)
                    .click(function() {
                        //    close all previously opened...
                        $(".record-viewer div.choice.open").each(function() {
                            if(this != oThis.field.get(0))
                                $(this).removeClass('open');
                        });

                        $(this).toggleClass('open');
                        $(this).trigger('focus');

                        return false;
                    });
            }

            return this.field;
        },

        getValue: function(val) {
            return $(this.valueField).val();
        },
        
        setValue: function(val) {
            var selectedItem = null;
            for(var i=0; i<this.items.length; i++) {
                if(this.items[i].id == val) {
                    selectedItem = this.items[i];
                    break;
                }
            }

            if(selectedItem != null) {
                this.valueField.val(selectedItem.id);
                this.titleField.val(selectedItem.name);
            }
        },

        _generatePopup: function() {
            var oThis = this;

            var body = $(document.createElement('tbody'));

            var popup = $(document.createElement('div'))
                            .addClass('popup')
                            .append(
                                $(document.createElement('table'))
                                    .addClass('table table-condensed table-hover')
                                    .append(body)
                            );

            $(this.items).each(function(index, item) {
                $(document.createElement('tr'))
                    .data({
                        value: item.id
                    })
                    .append(
                        $(document.createElement('td'))
                            .text(item.name)
                    )
                    .click(function() {
                        if(oThis.getValue() != $(this).data('value')) {
                            oThis.setValue($(this).data('value'));
    
                            oThis.field.trigger('change');
                        }
                    })
                    .appendTo(body);
            });
            
            return popup;
        },

        isDynamic: function() {
            return this.options.spec.constraint_data != null && this.options.spec.constraint_data.isJsObject();
        },

        isStatic: function() {
            return !this.isDynamic() && (typeof this.options.spec.constraint_data === 'string' || this.options.spec.constraint_data instanceof String);
        }
    });

}(jQuery));

$(document).click(function (e) {
    var container = $(".record-viewer div.choice.open");

    if (!container.is(e.target) // if the target of the click isn't the container...
        && container.has(e.target).length === 0) // ... nor a descendant of the container
    {
        container.removeClass('open');
    }
});
