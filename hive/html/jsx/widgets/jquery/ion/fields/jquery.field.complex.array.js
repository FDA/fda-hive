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
    $.widget("ion.field_complex_array", $.ion.field_complex_base, {

        buildLayoutContainer: function () {
            this.fields = this._collectFields(this.options.spec);

            var tbody = $(document.createElement('tbody'));

            $(document.createElement('table'))
                .addClass('table table-striped table-hover table-condensed form-group form-group-sm')
                .append(
                    $(document.createElement('thead'))
                        .append(this._addTableHead())
                )
                .append(tbody)
                .appendTo(this.element);

            return tbody;
        },

        _addTableHead: function () {
            var tr = $(document.createElement('tr'));

            for (var name in this.fields) {
                var field = this.fields[name];

                var th = $(document.createElement('th')).text(field.title || name);

                if (field._hidden == 'true') {
                    th.hide();
                }

                tr.append(th);
            }

            return tr;
        },

        generateControl: function () {
            var tr = $(document.createElement('tr'))
                .addClass('complex')
                .on('hide-field', function () {
                    return false;
                });

            for (var name in this.fields) {
                var field = this.fields[name];

                var td = $(document.createElement('td')).appendTo(tr);

                if (field._hidden == 'true') {
                    td.hide();
                }

                td.append(this._displayControl(name, field));
            }

            return tr;
        }
    });

}(jQuery));
