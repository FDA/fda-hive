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
(function ($, undefined) {
    "use strict";

    var private_var = null;

    $.jstree.defaults.sample = {
        sample_option : 'sample_val'
    };

    $.jstree.plugins.sample = function (options, parent) {
        this.sample_function = function (arg) {
            if(parent.sample_function) { parent.sample_function.call(this, arg); }
        };

        this.init = function (el, options) {
            parent.init.call(this, el, options);
        };
        this.bind = function () {
            parent.bind.call(this);
        };
        this.unbind = function () {
            parent.unbind.call(this);
        };
        this.teardown = function () {
            parent.teardown.call(this);
        };
        this.get_state = function () {
            var state = parent.get_state.call(this);
            state.sample = { 'var' : 'val' };
            return state;
        };
        this.set_state = function (state, callback) {
            if(parent.set_state.call(this, state, callback)) {
                if(state.sample) {
                    delete state.sample;
                    this.set_state(state, callback);
                    return false;
                }
                return true;
            }
            return false;
        };
        this.get_json = function (obj, options, flat) {
            var tmp = parent.get_json.call(this, obj, options, flat), i, j;
            if($.isArray(tmp)) {
                for(i = 0, j = tmp.length; i < j; i++) {
                    tmp[i].sample = 'value';
                }
            }
            else {
                tmp.sample = 'value';
            }
            return tmp;
        };
    };

    $(function () {
    });

    $.jstree.defaults.plugins.push("sample");
})(jQuery);