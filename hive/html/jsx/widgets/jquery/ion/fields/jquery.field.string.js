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
//# sourceURL=jquery.field.string.js

$(function () {
    $.widget("ion.field_string", $.ion.field_base, {

        generateControl: function () {
            if (this.hasLimit()) {
                if (this.isChoice()) {
                    return this.generateChoice();
                }
                else if (this.isChoicePlus()) {
                    return $(document.createElement('div'))
                                    .addClass('primitive form-control')
                                    .addClass(this.options.spec._type)
                                    .text('choice+: not implemented yet!')
                }
                else if (this.isSearchPlus()) {
                    return $(document.createElement('div'))
                                    .addClass('primitive form-control')
                                    .addClass(this.options.spec._type)
                                    .text('search+: not implemented yet!')
                }
            }
            else {
                var field = $(document.createElement('input'))
                    .addClass('primitive form-control')
                    .addClass(this.options.spec._type)
                    .attr({
                        name: this.options.name,
                        type: 'text'
                    });

                return field;
            }
        },

        generateChoice: function () {
            var field = $(document.createElement('select'))
                    .addClass('primitive form-control')
                    .addClass(this.options.type)
                    .append(
                        $(document.createElement('option'))
                            .attr({
                                value: ''
                            })
                    )
                    .click(function () {
                        $(this).data('prev', $(this).val());
                    });

            if ($.isArray(this.options.spec._limit.choice)) {
                $(this.options.spec._limit.choice).each(function (index, item) {
                    field.append(
                            $(document.createElement('option'))
                                .attr({
                                    value: item
                                })
                                .text(item)
                        );
                })
            }

            return field;
        }
    });

}(jQuery));
