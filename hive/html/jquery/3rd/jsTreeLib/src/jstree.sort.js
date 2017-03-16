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
 * ### Sort plugin
 *
 * Automatically sorts all siblings in the tree according to a sorting function.
 */
/*globals jQuery, define, exports, require */
(function (factory) {
    "use strict";
    if (typeof define === 'function' && define.amd) {
        define('jstree.sort', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.sort) { return; }

    /**
     * the settings function used to sort the nodes.
     * It is executed in the tree's context, accepts two nodes as arguments and should return `1` or `-1`.
     * @name $.jstree.defaults.sort
     * @plugin sort
     */
    $.jstree.defaults.sort = function (a, b) {
        //return this.get_type(a) === this.get_type(b) ? (this.get_text(a) > this.get_text(b) ? 1 : -1) : this.get_type(a) >= this.get_type(b);
        return this.get_text(a) > this.get_text(b) ? 1 : -1;
    };
    $.jstree.plugins.sort = function (options, parent) {
        this.bind = function () {
            parent.bind.call(this);
            this.element
                .on("model.jstree", $.proxy(function (e, data) {
                        this.sort(data.parent, true);
                    }, this))
                .on("rename_node.jstree create_node.jstree", $.proxy(function (e, data) {
                        this.sort(data.parent || data.node.parent, false);
                        this.redraw_node(data.parent || data.node.parent, true);
                    }, this))
                .on("move_node.jstree copy_node.jstree", $.proxy(function (e, data) {
                        this.sort(data.parent, false);
                        this.redraw_node(data.parent, true);
                    }, this));
        };
        /**
         * used to sort a node's children
         * @private
         * @name sort(obj [, deep])
         * @param  {mixed} obj the node
         * @param {Boolean} deep if set to `true` nodes are sorted recursively.
         * @plugin sort
         * @trigger search.jstree
         */
        this.sort = function (obj, deep) {
            var i, j;
            obj = this.get_node(obj);
            if(obj && obj.children && obj.children.length) {
                obj.children.sort($.proxy(this.settings.sort, this));
                if(deep) {
                    for(i = 0, j = obj.children_d.length; i < j; i++) {
                        this.sort(obj.children_d[i], false);
                    }
                }
            }
        };
    };

    // include the sort plugin by default
    // $.jstree.defaults.plugins.push("sort");
}));