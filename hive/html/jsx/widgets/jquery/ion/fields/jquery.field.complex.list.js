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
//# sourceURL=jquery.filed.complex.list.js

$(function () {
    $.widget("ion.field_complex_list", $.ion.field_complex_base, {

        generateControl: function () {
            var self = this;

            var field = $(document.createElement('ul'))
                            .addClass('complex form-group form-group-sm');

            this.fields = this._collectFields(this.options.spec);

            for (var name in this.fields) {
                var spec = this.fields[name];

                var li = $(document.createElement('li'))
                            .append(
                                $(document.createElement('label'))
                                    .addClass('title control-label')
                                    .append(
                                        $(document.createTextNode(spec.title || name))
                                    )
                            )
                            .append(
                                this._displayControl(name, spec)
                            )
                            .on('hide-field', function () {
                                $(this).hide();

                                return false;
                            })
                            .on('show-field', function () {
                                $(this).show();

                                return false;
                            })
                            .appendTo(field);

                if (this.isComplex(spec)) {
                    li.on('change-field', function () {
                        var label = $(this).children('label').first();
                        var complexField = $(this).children('.field-container').first();

                        if($('.form-control.changed', this).length > 0) {
                            if($('.control-panel', label).length == 0) {
                                $(label).append($(complexField).field_complex('generateControlPanel'));
                            }
                        }
                        else {
                            var controlPanel = $(complexField).field_complex('generateControlPanel');
                            $(controlPanel).detach();
                        }
                    })
                    //.on('clean-undo', function () {
                        //if ($('.undo', $(this).children('ul')).length == 0) {
                        //    $('.undo, .save', $(this).children('label')).remove();
                        //    oThis.element.trigger('clean-undo');
                        //}

                    //    return false;
                    //});

                    if (this.options.collapsed) {
                        li.addClass('collapsed');
                    }

                    li.children('label').prepend(
                        $(document.createElement('i'))
                            .addClass('closed glyphicon glyphicon-menu-right')
                    )
                    .prepend(
                        $(document.createElement('i'))
                            .addClass('opened glyphicon glyphicon-menu-down')
                    )
                    .click(function () {
                        if ($(this).closest('li').is('.collapsed')) {
                            $(this).closest('li').removeClass('collapsed');
                        }
                        else {
                            $(this).closest('li').addClass('collapsed');
                        }
                    });
                }

                if (spec.descr != null) {
                    li.children('label').attr({ title: spec.descr })
                }

                if (spec._hidden == 'true') {
                    li.hide();
                }
            }

            return field;
        },
    });

}(jQuery));
