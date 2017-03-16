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
 * ### Conditionalselect plugin
 *
 * This plugin allows defining a callback to allow or deny node selection by user input (activate node method).
 */
/*globals jQuery, define, exports, require, document */
(function (factory) {
    "use strict";
    if (typeof define === 'function' && define.amd) {
        define('jstree.conditionalselect', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.conditionalselect) { return; }

    /**
     * a callback (function) which is invoked in the instance's scope and receives two arguments - the node and the event that triggered the `activate_node` call. Returning false prevents working with the node, returning true allows invoking activate_node. Defaults to returning `true`.
     * @name $.jstree.defaults.checkbox.visible
     * @plugin checkbox
     */
    $.jstree.defaults.conditionalselect = function () { return true; };
    $.jstree.plugins.conditionalselect = function (options, parent) {
        // own function
        this.activate_node = function (obj, e) {
            if(this.settings.conditionalselect.call(this, this.get_node(obj), e)) {
                parent.activate_node.call(this, obj, e);
            }
        };
    };

}));