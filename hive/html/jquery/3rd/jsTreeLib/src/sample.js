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
/*global jQuery */
// wrap in IIFE and pass jQuery as $
(function ($, undefined) {
    "use strict";

    // some private plugin stuff if needed
    var private_var = null;

    // extending the defaults
    $.jstree.defaults.sample = {
        sample_option : 'sample_val'
    };

    // the actual plugin code
    $.jstree.plugins.sample = function (options, parent) {
        // own function
        this.sample_function = function (arg) {
            // you can chain this method if needed and available
            if(parent.sample_function) { parent.sample_function.call(this, arg); }
        };

        // *SPECIAL* FUNCTIONS
        this.init = function (el, options) {
            // do not forget parent
            parent.init.call(this, el, options);
        };
        // bind events if needed
        this.bind = function () {
            // call parent function first
            parent.bind.call(this);
            // do(stuff);
        };
        // unbind events if needed (all in jquery namespace are taken care of by the core)
        this.unbind = function () {
            // do(stuff);
            // call parent function last
            parent.unbind.call(this);
        };
        this.teardown = function () {
            // do not forget parent
            parent.teardown.call(this);
        };
        // state management - get and restore
        this.get_state = function () {
            // always get state from parent first
            var state = parent.get_state.call(this);
            // add own stuff to state
            state.sample = { 'var' : 'val' };
            return state;
        };
        this.set_state = function (state, callback) {
            // only process your part if parent returns true
            // there will be multiple times with false
            if(parent.set_state.call(this, state, callback)) {
                // check the key you set above
                if(state.sample) {
                    // do(stuff); // like calling this.sample_function(state.sample.var);
                    // remove your part of the state, call again and RETURN FALSE, the next cycle will be TRUE
                    delete state.sample;
                    this.set_state(state, callback);
                    return false;
                }
                // return true if your state is gone (cleared in the previous step)
                return true;
            }
            // parent was false - return false too
            return false;
        };
        // node transportation
        this.get_json = function (obj, options, flat) {
            // get the node from the parent
            var tmp = parent.get_json.call(this, obj, options, flat), i, j;
            if($.isArray(tmp)) {
                for(i = 0, j = tmp.length; i < j; i++) {
                    tmp[i].sample = 'value';
                }
            }
            else {
                tmp.sample = 'value';
            }
            // return the original / modified node
            return tmp;
        };
    };

    // attach to document ready if needed
    $(function () {
        // do(stuff);
    });

    // you can include the sample plugin in all instances by default
    $.jstree.defaults.plugins.push("sample");
})(jQuery);