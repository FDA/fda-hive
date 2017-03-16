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
//# sourceURL=jquery.record.base.js

$(function () {
    $.widget("recordviewer.record_base", {
        options: {
            name: null,
            spec: null
        },

        _create: function () {
            var oThis = this;

            this._prepareData(function() {
                oThis._buildField();
            });
        },

        _prepareData: function(callback) {
            if(callback != null)
                callback();
        },

        _buildField: function() {
            var oThis = this;

            this.field = this.generateField();

            this.field.focusin(function() {
                //    save initial value in case we will have to undo it...
                if(!$(this).data('initial-value')) {
                    oThis.setInitialValue();
                }
            })
            .change(function() {
                $(this).addClass('changed');
                $(this).trigger('field-changed');
            })
            .on('field-undo', function() {
                oThis.resetInitialValue();
                $(this).removeClass('changed');

                return false;
            });

            this.element.append(this.field);

            this.setDefaultValue();

            if(this.isReadonly())
                this.setReadonly();

            if(this.options.spec.is_multi)
                this.setMulti();
        },
        
        isReadonly: function() {
            return this.options.spec.is_readonly;
        },

        getValue: function(val) {
            return $(this.field).val();
        },
        
        setValue: function(val) {
            $(this.field).val(val);
        },
        
        setInitialValue: function() {
            $(this.field).data({ 'initial-value': this.getValue() });
        },

        resetInitialValue: function() {
            this.setValue($(this.field).data('initial-value'));
            $(this.field).removeAttr('data-changed');
        },
        
        generateField: function() {
            return $(document.createElement('span')).html('<i>Not supported type yet: ' + this.options.spec.type + '</i>' );
        },

        setDefaultValue: function() {
            this.setValue(this.options.spec.default_value);
        },
        
        setReadonly: function() {
            this.field.addClass('readonly').prop('readonly', true);
        },

        setMulti: function() {
            if(this.options.spec.is_readonly || !this.element.is('.field-container'))
                return;

            var oThis = this;

            var multiToolbar = $(document.createElement('div'))
                                    .addClass('toolbar')
                                    .append(
                                        $(document.createElement('button'))
                                        .addClass('btn btn-default btn-xs pull-right')
                                        .attr({ type: 'button' })
                                        .append(
                                            $(document.createElement('span'))
                                                .addClass('glyphicon glyphicon-plus')
                                                .attr({ 'aria-hidden': true })
                                        )
                                        .append( $(document.createTextNode(' Add')) )
                                        .click(function() {
                                            oThis.element.prepend(oThis.generateField());
                                        })
                                    )
                                    .appendTo(this.element);
                
            console.log('set multi...');
        }
    });
}(jQuery));
