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
if (typeof String.prototype.evalOnChange != 'function') {
    String.prototype.evalOnChange = function (onChange) {
        var evalJs = this.replace('eval-js:', '');

        var names = [];

        var re = /\$([a-zA-Z]+)/g;

        var m;
        do {
            m = re.exec(this);
            if (m) {
                names.push(m[1]);
            }
        }
        while (m);

        $(names).each(function(index, name) {
            evalJs = evalJs.replace('$' + name, '$("[data-name=\'' + name + '\'] > .primitive").val()');
        });

        $(names).each(function (index, name) {
            $('[data-name="' + name + '"] > .primitive')
                .focus(function () {
                    $(this).data('prev', $(this).val());
                })
                .change(function () {
                    if (onChange) {
                        onChange(eval(evalJs), $(this).data('prev'));
                    }
                });
        });
    };
}

if (typeof String.prototype.parseEval != 'function') {
    String.prototype.parseEval = function () {
        var evalJs = this.replace('eval-js:', '');

        var names = [];

        var re = /\$([a-zA-Z]+)/g;

        var m;
        do {
            m = re.exec(this);
            if (m) {
                names.push(m[1]);
            }
        }
        while (m);

        $(names).each(function (index, name) {
            evalJs = evalJs.replace('$' + name, '$("[data-name=\'' + name + '\'] > .primitive").val()');
        });

        var elements = [];

        $(names).each(function (index, name) {
            elements.push($('[data-name="' + name + '"] > .primitive'));
        });

        return $(elements);
    };
}

if (typeof String.prototype.evalJs != 'function') {
    String.prototype.evalJs = function () {
        var js = this.replace('eval-js:', '');

        var names = [];

        var re = /\$([a-zA-Z]+)/g;

        var m;
        do {
            m = re.exec(this);
            if (m) {
                names.push(m[1]);
            }
        }
        while (m);

        $(names).each(function (index, name) {
            js = js.replace('$' + name, '$("[data-name=\'' + name + '\'] > .primitive").val()');
        });

        try {
            return eval(js);
        }
        catch (e) {
            console.error(js, e);
        }
    };
}
