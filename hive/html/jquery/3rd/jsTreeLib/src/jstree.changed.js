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
/**
 * ### Changed plugin
 *
 * This plugin adds more information to the `changed.jstree` event. The new data is contained in the `changed` event data property, and contains a lists of `selected` and `deselected` nodes.
 */
/*globals jQuery, define, exports, require, document */
(function (factory) {
    "use strict";
    if (typeof define === 'function' && define.amd) {
        define('jstree.changed', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.changed) { return; }

    $.jstree.plugins.changed = function (options, parent) {
        var last = [];
        this.trigger = function (ev, data) {
            var i, j;
            if(!data) {
                data = {};
            }
            if(ev.replace('.jstree','') === 'changed') {
                data.changed = { selected : [], deselected : [] };
                var tmp = {};
                for(i = 0, j = last.length; i < j; i++) {
                    tmp[last[i]] = 1;
                }
                for(i = 0, j = data.selected.length; i < j; i++) {
                    if(!tmp[data.selected[i]]) {
                        data.changed.selected.push(data.selected[i]);
                    }
                    else {
                        tmp[data.selected[i]] = 2;
                    }
                }
                for(i = 0, j = last.length; i < j; i++) {
                    if(tmp[last[i]] === 1) {
                        data.changed.deselected.push(last[i]);
                    }
                }
                last = data.selected.slice();
            }
            /**
             * triggered when selection changes (the "changed" plugin enhances the original event with more data)
             * @event
             * @name changed.jstree
             * @param {Object} node
             * @param {Object} action the action that caused the selection to change
             * @param {Array} selected the current selection
             * @param {Object} changed an object containing two properties `selected` and `deselected` - both arrays of node IDs, which were selected or deselected since the last changed event
             * @param {Object} event the event (if any) that triggered this changed event
             * @plugin changed
             */
            parent.trigger.call(this, ev, data);
        };
        this.refresh = function (skip_loading, forget_state) {
            last = [];
            return parent.refresh.apply(this, arguments);
        };
    };
}));