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
//# sourceURL=jquery.view.recordviewer.js

$(function () {
    $.widget("view.recordviewer", {
        options: {
            propSpecUrl: null,
            collapsed: false,
            style: 'inline'    //    form|inline
        },
        
        _create: function () {
            var oThis = this;

            this.element.addClass('record-viewer').addClass(this.options.style);

            this.title = $(document.createElement('label'))
                            .addClass('title')
                            .appendTo(this.element);

            this.ul = $(document.createElement('ul'))
                        .appendTo(this.element);

            this.loadSpec();
        },

        loadSpec: function() {
            var oThis = this;

            if(this.options.propSpecUrl) {
                $.ajax({
                    dataType: "json",
                    url: this.options.propSpecUrl,
                    success: function(data) {
                        //console.log(data);
                          oThis.renderSpecs(data);
                    }
                });
            }
        },

        renderSpecs: function(specs) {
            var oThis = this;

            this.title.text(specs.title);

            this.addCollapser(this.element);

            $(Object.keys(specs._attributes)).each(function(index, name) {
                oThis.generateSpec(name, specs._attributes[name], oThis.ul);
            });

            $('input.field, select.field, textarea.field', this.element)
                .focus(function() {
                    //    save initial value in case we will have to undo it...
                    if(!$(this).data('initial-value')) {
                        oThis.setInitialValue(this);
                    }
                })
                .change(function() {
                    if(!$(this).next().is('.undo')) {
                        $(this).attr('data-changed', true)

                        oThis.createUndoButtons(this);
                    }
                });
        },

        addCollapser: function(container) {
            var label = container.children('label').first();

            if(label == null)
                return;

            if($('.collapser', label).length > 0)
                return;

            var collapserClass = $(label).next().is(':visible') ? 'glyphicon-chevron-down' : 'glyphicon-chevron-right';
            
            label.prepend(
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
        },

        generateTitle: function(name, spec) {
            var titleDiv = $(document.createElement('label'))
                                .addClass('title')
                                .attr({
                                    'for': name
                                })
                                .text(spec.title);

            return titleDiv;
        },

        generateSpec: function(name, spec, container) {
            var oThis = this;

            var parentType = container.data('type');

            var li = $(document.createElement('li'))
                        .addClass(spec.type)
                        .addClass('clearfix')
                        .attr({
                            'data-type': spec.type,
                            'data-name': name
                        })
                        .appendTo(container);

            if(spec.type == 'bool' && parentType == 'list') {
                li.append(oThis.generateField(spec)).append(oThis.generateTitle(spec));
            }
            else {
                var title = this.generateTitle(name, spec);
                var field = oThis.generateField(spec);
                
                if(spec.type == 'list' || spec.type == 'array') {
                    li.append(title).append(field);
                }
                else {
                    li.append(title).append(
                            $(document.createElement('div'))
                                .append(field)
                        );
                    
                    oThis.setDefaultValue(field, spec);
                }

                if(spec._children && spec.type != 'array') {
                    $(Object.keys(spec._children)).each(function(index, name) {
                        oThis.generateSpec(name, spec._children[name], field);
                    });

                    oThis.addCollapser(li);
                }
            }
        },

        generateField: function(spec) {
            var oThis = this;
            
            var field = null;

            if(spec.type == 'list') {
                field = $(document.createElement('ul'))
                                .attr({
                                    'data-type': spec.type,
                                })
                                .addClass('list');
                
                if(this.options.collapsed)
                    field.hide();
            }
            else if(spec.type == 'array') {
                field = this.arrayField(spec);

                //if(this.options.collapsed)
                //    field.hide();
            }
            else if(spec.type == 'integer') {
                if(spec.constraint == 'choice') {
                    field = this.choiceField(spec);
                }
                else {
                    field = $(document.createElement('input'))
                                    .attr({
                                        type: 'text'
                                    });
                }
            }
            else if(spec.type == 'string') {
                if(spec.constraint == 'choice') {
                    field = this.choiceField(spec);
                }
                else {
                    field = $(document.createElement('input'))
                                    .attr({
                                        type: 'text'
                                    });
                }
            }
            else if(spec.type == 'text') {
                field = $(document.createElement('textarea')).attr({
                    rows: 3
                });
            }
            else if(spec.type == 'bool') {
                field = $(document.createElement('input'))
                                .attr({
                                    type: 'checkbox'
                                });
            }
            else if(spec.type == 'datetime') {
                field = $(document.createElement('input'))
                .attr({
                    type: 'text'
                });
            }
            else if(spec.type == 'real') {
                field = $(document.createElement('input'))
                .attr({
                    type: 'text'
                });
            }
            else {
                field = $(document.createElement('span'))
                    .html('<i>Not supported type yet: ' + spec.type + '</i>' )
            }

            if(field != null) {
                field
                    .addClass('field')
                    .addClass(spec.type)
                    .attr({
                        name: spec.name,
                        id: spec.name,
                    });
            }

            return field;
        },

        arrayField: function(spec) {
            var oThis = this;

            var field = $(document.createElement('table'))
                .addClass('array');

            var header = $(document.createElement('thead'))
                            .appendTo(field);

            var body = $(document.createElement('tbody'))
                            .appendTo(field);

            if(spec._children) {
                var headTr = $(document.createElement('tr')).appendTo(header);
                //var bodyTr = $(document.createElement('tr')).appendTo(body);

                $(Object.keys(spec._children)).each(function(index, name) {
                    var child = spec._children[name];

                    $(document.createElement('th'))
                        .text(child.title)
                        .appendTo(headTr);

                    //$(document.createElement('td'))
                    //    .append(oThis.generateField(child))
                    //    .appendTo(bodyTr);
                });

                body.append( oThis._generateTableRow(spec._children) );

                var footerTr = $(document.createElement('tr'))
                    .append(
                        $(document.createElement('td'))
                            .attr({
                                colspan: Object.keys(spec._children).length
                            })
                            .append(
                                $(document.createElement('button'))
                                    .addClass('btn btn-default btn-xs pull-right')
                                    .attr({
                                        type: 'button'
                                    })
                                    .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-plus')
                                            .attr({ 'aria-hidden': true })
                                    )
                                    .append( $(document.createTextNode(' Add')) )
                                    .click(function() {
                                        $(this).closest('tr').before( oThis._generateTableRow(spec._children) );
                                    })
                            )
                    )
                    .appendTo(body);
            }

            return field;
        },

        _generateTableRow: function(items) {
            var oThis = this;

            var tr = $(document.createElement('tr'));

            $(Object.keys(items)).each(function(index, name) {
                var item = items[name];

                $(document.createElement('td'))
                    .append(oThis.generateField(item))
                    .appendTo(tr);
            });

            $(document.createElement('td'))
                .addClass('toolbar hidden')
                .append(
                        $(document.createElement('span'))
                            .attr({
                                'aria-hidden': true
                            })
                            .addClass('remove')
                )
                .appendTo(tr);

            return tr;
        },

        choiceField: function(spec) {
            var oThis = this;

            var field = $(document.createElement('select'))
                            .addClass(spec.type);

            if(spec.constraint_data) {
                $(spec.constraint_data.split('|')).each(function(index, option) {
                    var vals = option.split('///');
                    $(document.createElement('option'))
                        .attr({
                            value: vals[0]
                        })
                        .text(vals[1] == null ? vals[0] : vals[1])
                        .appendTo(field);
                });
            }

            return field;
        },

        setDefaultValue: function(field, spec) {
            if(    spec.type == 'integer' ||
                spec.type == 'real' ||
                   spec.type == 'string' ||
                   spec.type == 'text' ||
                   spec.type == 'datetime') {
                field.val(spec.default_value);
            }
            else if(spec.type == 'bool') {
                if(Boolean.parse(spec.default_value))
                    field.attr({ checked: true });
            }
        },

        setInitialValue: function(field) {
            var value = null;

            if($(field).is('input[type="checkbox"]')) {
                value = $(field).is(':checked');
            }
            else {
                value = $(field).val();
            }

            $(field).data({ 'initial-value': value });
        },

        resetInitialValue: function(field) {
            var value = $(field).data('initial-value');

            if($(field).is('input[type="checkbox"]')) {
                $(field).prop('checked', value);
            }
            else {
                $(field).val(value);
            }

            $(field).removeAttr('data-changed');
        },

        //    create Undo buttons for the field and all parents lists or arrays...
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

        cleanButtons: function() {
            var oThis = this;

            $('.undo', this.element).each(function() {
                if($('.field[data-changed]', this.closest('li', oThis.element)).length == 0)
                    $(this).remove();
            });

            $('.save', this.element).each(function() {
                if($('.field[data-changed]', this.closest('li', oThis.element)).length == 0)
                    $(this).remove();
            });
        },

        createUndoButton: function() {
            var oThis = this;

            return $(document.createElement('span'))
                        .attr({
                            'aria-hidden': true,
                            //'data-toggle': 'tooltip',
                            //'data-placement': 'top',
                            //title: 'Discard changes'
                        })
                        //.tooltip()
                        .addClass('undo')
                        .click(function() {
                            if($(this).prev().is('input.field, select.field, textarea.field')) {
                                //    one field undo...
                                oThis.resetInitialValue($(this).prev());

                                $(this).remove();
                            }
                            else if($(this).parent().is('label.title')) {
                                //    group undo...
                                var node = null;
                                
                                if($.contains(oThis.title.get(0), this)) {
                                    //    top level undo...
                                    node = oThis.element;
                                }
                                else {
                                    //    all others...
                                    node = $(this).closest('li');
                                }

                                $('.field[data-changed]', node).each(function() {
                                    oThis.resetInitialValue(this);

                                    $(this).next('.undo').remove();
                                })

                                $(this).remove();
                            }

                            oThis.cleanButtons();
                            
                            return false;
                        });
        },

        createSaveButton: function() {
            var oThis = this;

            return $(document.createElement('span'))
                        .attr({
                            'aria-hidden': true,
                            //'data-toggle': 'tooltip',
                            //'data-placement': 'top',
                            //title: 'Save changes'
                        })
                        //.tooltip()
                        .addClass('save')
                        .click(function() {
                            return false;
                        });
            
        }
        
    });
}(jQuery));
