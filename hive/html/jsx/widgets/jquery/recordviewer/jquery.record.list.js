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
//# sourceURL=jquery.record.list.js

$(function () {
    $.widget("recordviewer.record_list", {

        options: {
            spec: null,
            name: null,
            children: null
        },

        _create: function () {
            this._render();
        },

        _render: function() {
            if(this.element.prop("tagName") != 'UL') {
                console.log('ERROR: RECORD_LIST widget must be applyed to UL element only!');
                return;
            }

            this.element
                .attr({
                    'data-type': this.options.spec.type,
                })
                .addClass(this.options.spec.type)
                .addClass('field');

            this.showList();
        },

        showList: function() {
            var oThis = this;

            if(this.options.spec._children) {
                $(Object.keys(this.options.children)).each(function(index, name) {
                    var spec = oThis.options.children[name];

                    //    do not process hidden fields...
                    if(spec.is_hidden)
                        return;

                    var li = $(document.createElement('li'))
                        .addClass(spec.type)
                        .addClass('clearfix')
                        .on('field-changed', function() {
                            if($(this).is('.list') || $(this).is('.array')) {
                                if($('.undo', $(this).children('label')).length == 0) {
                                    $(this).children('label').append(oThis.createUndoButton());
                                    $(this).children('label').append(oThis.createSaveButton());
                                }
                            }
                            else {
                                if($('.undo', $(this).children('div')).length == 0)
                                    $(this).children('div').append(oThis.createFieldControlPanel());
                            }

                            oThis.element.trigger('field-changed');

                            return false;
                        })
                        .on('clean-undo', function() {
                            if($('.undo', $(this).children('ul')).length == 0) {
                                $('.undo, .save', $(this).children('label')).remove();
                                oThis.element.trigger('clean-undo');
                            }

                            return false;
                        })
                        .appendTo(oThis.element);

                    $(document.createElement('label'))
                        .addClass('title')
                        .attr({
                            'for': name
                        })
                        .text(spec.title)
                        .appendTo(li);
                    
                    if(spec.type == 'string') {
                        var div = $(document.createElement('div'))
                                        .addClass('field-container')
                                        .appendTo(li);
                
                        if(spec.constraint == 'choice') {
                            div.record_choice({
                                name: name,
                                spec: spec
                            });
                        }
                        else {
                            div.record_string({
                                name: name,
                                spec: spec
                            });
                        }
                    }
                    else if(spec.type == 'integer') {
                        var div = $(document.createElement('div'))
                                        .addClass('field-container')
                                        .appendTo(li);
                
                        if(spec.constraint == 'choice' || spec.constraint == 'search') {
                            div.record_choice({
                                name: name,
                                spec: spec
                            });
                        }
                        else {
                            div.record_integer({
                                name: name,
                                spec: spec
                            });
                        }
                    }
                    else if(spec.type == 'text') {
                        $(document.createElement('div'))
                            .addClass('field-container')
                            .appendTo(li)
                            .record_text({
                                name: name,
                                spec: spec
                            });
                    }
                    else if(spec.type == 'datetime') {
                        $(document.createElement('div'))
                            .addClass('field-container')
                            .appendTo(li)
                            .record_datetime({
                                name: name,
                                spec: spec
                            });
                    }
                    else if(spec.type == 'real') {
                        $(document.createElement('div'))
                            .addClass('field-container')
                            .appendTo(li)
                            .record_real({
                                name: name,
                                spec: spec
                            });
                    }
                    else if(spec.type == 'bool') {
                        $(document.createElement('div'))
                            .addClass('field-container')
                            .appendTo(li)
                            .record_bool({
                                name: name,
                                spec: spec
                            });
                    }
                    else if(spec.type == 'list') {
                        $(document.createElement('ul'))
                            .appendTo(li)
                            .record_list({
                                name: name,
                                spec: spec,
                                children: spec._children
                            });
                    }
                    else if(spec.type == 'array') {
                        $(document.createElement('table'))
                            .appendTo(li)
                            .record_array({
                                name: name,
                                spec: spec
                            });
                    }
                    else {
                        $(document.createElement('div'))
                            .addClass('field-container')
                            .record_base({
                                name: name,
                                spec: spec
                            })
                            .appendTo(li);
                    }
                });

                this.addCollapser();
            }
        },

        addCollapser: function() {
            this.element.children('li.list').each(function(index, li) {
                $(li).children('label').each(function() {
                    var collapserClass = $(this).next().is(':visible') ? 'glyphicon-chevron-down' : 'glyphicon-chevron-right';

                    $(this).prepend(
                        $(document.createElement('span'))
                            .attr({ 'aria-hidden': true })
                            .addClass('collapser glyphicon')
                            .addClass(collapserClass)
                    )
                    .click(function() {
                        $(this).next().toggle();

                        if($(this).next().is(':visible'))
                            $('.collapser', this).removeClass('glyphicon-chevron-right').addClass('glyphicon-chevron-down');
                        else
                            $('.collapser', this).removeClass('glyphicon-chevron-down').addClass('glyphicon-chevron-right');
                    });

                })
                
            });
        },
/*
        createUndoButtons: function(field) {
            var oThis = this;

            $(field).after(this.createUndoButton());

            $(field).parentsUntil(this.element, 'li.list, li.array').each(function() {
                var label = $(this).children('label.title');
                if($('.undo', label).length == 0)
                    label.append(oThis.createUndoButton())
                        .append(oThis.createSaveButton());
            });

            if($('.undo', this.title).length == 0)
                this.title.append(this.createUndoButton())
                .append(this.createSaveButton());
        },
*/
        createUndoButton: function() {
            var oThis = this;

            return $(document.createElement('span'))
                        .attr({
                            'aria-hidden': true,
                            title: 'Undo'
                        })
                        .addClass('undo')
                        .click(function() {
                            if($(this).parent().is('.field-control') && $(this).parent().prev().is('input.field, select.field, textarea.field, div.field.choice')) {
                                //    one field undo...
                                $(this).parent().prev().trigger('field-undo');
                                $(this).remove();
                            }
                            else if($(this).parent().is('label.title')) {
                                $('input.field, select.field, textarea.field, div.field.choice', $(this).closest('li').children('ul.list, table.array')).trigger('field-undo');
                                $('.undo, .save, .field-control', $(this).closest('li')).remove();
                            }

                            if($('.undo', oThis.element).length == 0) {
                                oThis.element.trigger('clean-undo');
                            }

                            return false;
                        });
        },

        createSaveButton: function() {
            var oThis = this;

            return $(document.createElement('span'))
                        .attr({
                            'aria-hidden': true,
                            title: 'Save'
                        })
                        .addClass('save')
                        .click(function() {
                            return false;
                        });
        },
        
        createFieldControlPanel: function() {
            return $(document.createElement('div'))
                        .addClass('field-control')
                        .append(this.createUndoButton());
        }
    });

}(jQuery));
