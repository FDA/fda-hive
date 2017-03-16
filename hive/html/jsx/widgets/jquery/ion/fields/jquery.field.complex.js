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
//# sourceURL=jquery.filed.complex.js

$(function () {
    $.widget("ion.field_complex", $.ion.field_base, {
        render: function () {
            this.element.empty();

            var self = this;

            //  need to destroy previously created widget first...
            if ($(this.element).field_complex_list("instance") !== undefined) {
                $(this.element).field_complex_list("destroy");
            }
            else if ($(this.element).field_complex_array("instance") !== undefined) {
                $(this.element).field_complex_array("destroy");
            }

            //  ... and then try to build new component... 
            if (this.isList()) {
                $(this.element).field_complex_list(this.options);
            }
            else if (this.isArray()) {
                $(this.element).field_complex_array(this.options);
            }

            //  ... and finally try to apply value
            if (this.options.spec.hasOwnProperty('_type') && this.options.spec.hasOwnProperty('_id')) {
                this.setValue(this.options.spec);
            }
        },

        _evalProcessor: function () {
            var self = this;

            if (typeof this.options.spec._layout == "string" && this.options.spec._layout.startsWith('eval-js:')) {
                this.options.spec._layout.parseEval().each(function () {
                    $(this)
                        .unbind('change.' + self.options.name)
                        .bind('change.' + self.options.name, function (event) {
                            //console.log('layout: ' + self.options.spec._layout.evalJs());
                            //console.log('prev: ' + $(event.target).data('prev'));

                            var value = self.options.spec._layout.evalJs();
                            var prev = $(event.target).data('prev');

                            var val;

                            //    save value in order to apply it later after rendering... 
                            if (prev && self.isPlural()) {
                                if (prev === 'array') {
                                    val = $(self.element).field_complex_array('getValue');
                                }
                                else {
                                    val = $(self.element).field_complex_list('getValue');
                                }
                            }

                            if (prev != value) {
                                self.render();

                                if (val) {
                                    self.setValue(val);
                                }
                            }
                        });
                });
            }

            this._super();
        },

        isList: function () {
            return !this.isArray();
        },

        isArray: function () {
            return this.isPlural() && this.getLayout() === 'array';
        },

        getLayout: function () {
            if (this.options.spec != null && this.options.spec._layout != null) {
                if (this.options.spec._layout.startsWith("eval-js:")) {
                    if (!this.element.isInDom())
                        return false;

                    return this.options.spec._layout.evalJs();
                }

                return this.options.spec._layout;
            }

            return null;
        },

        setValue: function (val) {
            if (this.isList()) {
                $(this.element).field_complex_list('setValue', val);
            }
            else if (this.isArray()) {
                $(this.element).field_complex_array('setValue', val);
            }
        },

        getValue: function () {
            if (this.isList()) {
                return $(this.element).field_complex_list('getValue');
            }
            else if (this.isArray()) {
                return $(this.element).field_complex_array('getValue');
            }
        },

        generateControlPanel: function () {
            var self = this;

            if (this.controlPanel == null) {
                this.controlPanel = $(document.createElement('span'))
                .addClass('control-panel')
                .append(
                    $(document.createElement('span'))
                        .addClass('undo glyphicon glyphicon-share-alt')
                        .attr({
                            'aria-hidden': true,
                            title: 'Undo'
                        })
                        .click(function () {
                            $('.form-control.changed', $(this).closest('.control-label').next()).each(function () {
                                $(this).trigger('rollback-field');
                            })

                            return false;
                        })
                )
                .append(
                    $(document.createElement('span'))
                        .addClass('save glyphicon glyphicon-floppy-save')
                        .attr({
                            'aria-hidden': true,
                            title: 'Save'
                        })
                        .click(function () {
                            var val = $(this).closest('.control-label').next().field_complex('getValue');

                            $(this).trigger('save-field', { value: val });

                            return false;
                        })
                );
            }

            return this.controlPanel;
        },

        addControl: function () {
            console.log('complex: add control...');
        }
    });

}(jQuery));
