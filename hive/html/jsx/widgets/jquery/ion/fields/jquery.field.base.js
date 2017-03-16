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
//# sourceURL=jquery.field.base.js

$(function () {
    $.widget("ion.field_base", {
        options: {
            recordviewer: null, //  main record viewer where field belongs to
            name: null,         //  field's name
            spec: {}            //  field's specification
        },

        _create: function () {
            var self = this;

            this.render();
        },

        render: function () {
            this.element.empty();

            this._buildControl();
        },

        runEvalProcessor: function () {
            this._evalProcessor();
        },

        _evalProcessor: function () {
            var self = this;

            if (!this.options.spec)
                return;

            var needToRerender = false;

            if (typeof this.options.spec._hidden == "string" && this.options.spec._hidden.startsWith('eval-js:')) {
                this.options.spec._hidden.parseEval().each(function () {
                    $(this)
                        .unbind('change.hidden_' + self.uniqueFieldId())
                        .bind('change.hidden_' + self.uniqueFieldId(), function (event) {
                            var value = self.options.spec._hidden.evalJs();

                            if (value) {
                                //  hide the field...
                                self.hide();
                                //  ... and send notification that it's hidden
                                self.element.trigger('hide-field');
                            }
                            else {
                                //  show the field...
                                self.show();
                                //  ... and send notification that it's visible
                                self.element.trigger('show-field');
                            }
                        });
                });

                needToRerender = true;
            }
            if (typeof this.options.spec._plural == "string" && this.options.spec._plural.startsWith('eval-js:')) {
                this.options.spec._plural.parseEval().each(function () {
                    $(this)
                        .unbind('change.plural_' + self.uniqueFieldId())
                        .bind('change.plural_' + self.uniqueFieldId(), function (event) {
                            self.render();
                        });
                });

                needToRerender = true;
            }

            if (needToRerender) {
                self.render();
            }
        },

        uniqueFieldId: function () {
            if (this._uniqueId == null) {
                this._uniqueId = this.options.name + '_' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890'.random();
            }

            return this._uniqueId;
        },

        _buildControl: function () {
            var self = this;

            this.container = this.buildLayoutContainer();

            this.field = this.addControl();

            if (this.isPlural()) {
                this.footer = this.buildFooter();
            }

            if (this.hasDefaultValue())
                this.setDefaultValue(this.defaultValue());

            if (this.isReadonly())
                this.setReadonly();

            if (this.isPlural()) {
                this.element.addClass('plural');
            }

            if (this.isHidden()) {
                this.hide();
            }
        },

        _onChange: function () {
            this.field.addClass('changed');
            this.field.trigger('change-field');
        },

        isReadonly: function () {
            //return this.options.spec.is_readonly;
        },

        getValue: function () {
            if (this.isPlural()) {
                var res = [];

                $('input, textarea, select', this.container).each(function () {
                    var val = $(this).val().trim();

                    if (val) {
                        res.push(val);
                    }
                });

                return res;
            }
            else {
                return $(this.field).val();
            }
        },

        setValue: function (value) {
            $(this.field).val(value);
        },

        setInitialValue: function () {
            $(this.field).data({ 'initial-value': this.getValue() });
        },

        resetInitialValue: function () {
            this.setValue($(this.field).data('initial-value'));
            //$(this.field).removeAttr('data-changed');
        },

        buildLayoutContainer: function () {
            //  *******************************************************************************************
            //  override this method in order to add additional component layout (list of array views etc.)
            //  *******************************************************************************************

            if (this.isPlural()) {
                return $(document.createElement('div')).appendTo(this.element);
            }

            return this.element;
        },

        buildAddButton: function () {
            var that = this;

            return $(document.createElement('button'))
                        .addClass('btn btn-default btn-xs pull-right')
                        .attr({ type: 'button' })
                        .append(
                            $(document.createElement('span'))
                                .addClass('glyphicon glyphicon-plus')
                                .attr({ 'aria-hidden': true })
                        )
                        .append($(document.createTextNode(' Add')))
                        .click(function () {
                            that.addControl();
                        });
        },

        buildFooter: function () {
            var footer = $(document.createElement('div'))
                                .addClass('toolbar clearfix')
                                .append(this.buildAddButton())
                                .appendTo(this.element);

            return footer;
        },

        generateControl: function () {
            //  ***************************************************
            //  override this method to proper control's generation
            //  ***************************************************
            return $(document.createElement('span')).html('<i>Not supported type yet: ' + this.options.type + '</i>');
        },

        hasDefaultValue: function () {
            return this.options.spec != null && this.options.spec._default;
        },

        hasLimit: function () {
            return this.options.spec != null && this.options.spec._limit;
        },

        defaultValue: function () {
            if (this.options.spec != null)
                return this.options.spec._default;

            return null;
        },

        setDefaultValue: function (val) {
            this.setValue(val);
        },

        setReadonly: function () {
            //this.field.addClass('readonly').prop('readonly', true);
        },

        isPlural: function () {
            if (this.options.spec != null && this.options.spec._plural != null) {
                if (this.options.spec._plural.startsWith("eval-js:")) {
                    if (!this.element.isInDom())
                        return false;

                    return this.options.spec._plural.evalJs();
                }

                return this.options.spec._plural === "true";
            }

            return false;
        },

        isHidden: function () {
            if (this.options.spec != null && this.options.spec._hidden != null) {
                if (this.options.spec._hidden.startsWith("eval-js:")) {
                    if (!this.element.isInDom())
                        return false;

                    return this.options.spec._hidden.evalJs();
                }

                return this.options.spec._hidden === "true";
            }

            return false;
        },

        hide: function () {
            this.element.hide();
        },

        show: function () {
            this.element.show();
        },

        isChoice: function () {
            return this.options.spec != null && this.options.spec._limit != null && this.options.spec._limit.choice != null;
        },

        isChoicePlus: function () {
            return this.options.spec != null && this.options.spec._limit != null && this.options.spec._limit.hasOwnProperty('choice+');
        },

        isSearchPlus: function () {
            return this.options.spec != null && this.options.spec._limit != null && this.options.spec._limit.hasOwnProperty('search+');
        },

        addControl: function () {
            var self = this;

            var control = this.generateControl();

            control.focusin(function () {
                //    save initial value in case we will have to undo it...
                if ($(this).data('initial-value') == undefined) {
                    $(this).data({ 'initial-value': $(this).val() });
                }
            })
            .change(function () {
                var initial = $(this).data('initial-value');

                if (!initial) {
                    initial = '';
                }

                if (initial === $(this).val())
                    $(this).removeClass('changed');
                else
                    $(this).addClass('changed');

                $(this).trigger('change-field');
            })
            .on({
                'delete-field': function () {
                    var container = $(this).parent();

                    $(this).remove();

                    $(container).trigger('change-field');

                    return false;
                }
            })
            .on('rollback-field', function () {
                $(this).val($(this).data('initial-value'));

                $(this).change();

                return false;
            });

            this.container.append(control);

            return control;
        },
    });
}(jQuery));
