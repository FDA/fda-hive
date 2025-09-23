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
        define('jstree.state', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.state) { return; }

    var to = false;
    $.jstree.defaults.state = {
        key        : 'jstree',
        events    : 'changed.jstree open_node.jstree close_node.jstree check_node.jstree uncheck_node.jstree',
        ttl        : false,
        filter    : false
    };
    $.jstree.plugins.state = function (options, parent) {
        this.bind = function () {
            parent.bind.call(this);
            var bind = $.proxy(function () {
                this.element.on(this.settings.state.events, $.proxy(function () {
                    if(to) { clearTimeout(to); }
                    to = setTimeout($.proxy(function () { this.save_state(); }, this), 100);
                }, this));
                this.trigger('state_ready');
            }, this);
            this.element
                .on("ready.jstree", $.proxy(function (e, data) {
                        this.element.one("restore_state.jstree", bind);
                        if(!this.restore_state()) { bind(); }
                    }, this));
        };
        this.save_state = function () {
            var st = { 'state' : this.get_state(), 'ttl' : this.settings.state.ttl, 'sec' : +(new Date()) };
            $.vakata.storage.set(this.settings.state.key, JSON.stringify(st));
        };
        this.restore_state = function () {
            var k = $.vakata.storage.get(this.settings.state.key);
            if(!!k) { try { k = JSON.parse(k); } catch(ex) { return false; } }
            if(!!k && k.ttl && k.sec && +(new Date()) - k.sec > k.ttl) { return false; }
            if(!!k && k.state) { k = k.state; }
            if(!!k && $.isFunction(this.settings.state.filter)) { k = this.settings.state.filter.call(this, k); }
            if(!!k) {
                this.element.one("set_state.jstree", function (e, data) { data.instance.trigger('restore_state', { 'state' : $.extend(true, {}, k) }); });
                this.set_state(k);
                return true;
            }
            return false;
        };
        this.clear_state = function () {
            return $.vakata.storage.del(this.settings.state.key);
        };
    };

    (function ($, undefined) {
        $.vakata.storage = {
            set : function (key, val) { return window.localStorage.setItem(key, val); },
            get : function (key) { return window.localStorage.getItem(key); },
            del : function (key) { return window.localStorage.removeItem(key); }
        };
    }($));

}));