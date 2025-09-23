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
(function (factory) {
    "use strict";
    if (typeof define === 'function' && define.amd) {
        define('jstree.massload', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.massload) { return; }

    $.jstree.defaults.massload = null;
    $.jstree.plugins.massload = function (options, parent) {
        this.init = function (el, options) {
            parent.init.call(this, el, options);
            this._data.massload = {};
        };
        this._load_nodes = function (nodes, callback, is_callback) {
            var s = this.settings.massload;
            if(is_callback && !$.isEmptyObject(this._data.massload)) {
                return parent._load_nodes.call(this, nodes, callback, is_callback);
            }
            if($.isFunction(s)) {
                return s.call(this, nodes, $.proxy(function (data) {
                    if(data) {
                        for(var i in data) {
                            if(data.hasOwnProperty(i)) {
                                this._data.massload[i] = data[i];
                            }
                        }
                    }
                    parent._load_nodes.call(this, nodes, callback, is_callback);
                }, this));
            }
            if(typeof s === 'object' && s && s.url) {
                s = $.extend(true, {}, s);
                if($.isFunction(s.url)) {
                    s.url = s.url.call(this, nodes);
                }
                if($.isFunction(s.data)) {
                    s.data = s.data.call(this, nodes);
                }
                return $.ajax(s)
                    .done($.proxy(function (data,t,x) {
                            if(data) {
                                for(var i in data) {
                                    if(data.hasOwnProperty(i)) {
                                        this._data.massload[i] = data[i];
                                    }
                                }
                            }
                            parent._load_nodes.call(this, nodes, callback, is_callback);
                        }, this))
                    .fail($.proxy(function (f) {
                            parent._load_nodes.call(this, nodes, callback, is_callback);
                        }, this));
            }
            return parent._load_nodes.call(this, nodes, callback, is_callback);
        };
        this._load_node = function (obj, callback) {
            var d = this._data.massload[obj.id];
            if(d) {
                return this[typeof d === 'string' ? '_append_html_data' : '_append_json_data'](obj, typeof d === 'string' ? $($.parseHTML(d)).filter(function () { return this.nodeType !== 3; }) : d, function (status) {
                    callback.call(this, status);
                    delete this._data.massload[obj.id];
                });
            }
            return parent._load_node.call(this, obj, callback);
        };
    };
}));