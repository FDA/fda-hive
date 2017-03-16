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
 * ### State plugin
 *
 * Saves the state of the tree (selected nodes, opened nodes) on the user's computer using available options (localStorage, cookies, etc)
 */
/*globals jQuery, define, exports, require */
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
    /**
     * stores all defaults for the state plugin
     * @name $.jstree.defaults.state
     * @plugin state
     */
    $.jstree.defaults.state = {
        /**
         * A string for the key to use when saving the current tree (change if using multiple trees in your project). Defaults to `jstree`.
         * @name $.jstree.defaults.state.key
         * @plugin state
         */
        key        : 'jstree',
        /**
         * A space separated list of events that trigger a state save. Defaults to `changed.jstree open_node.jstree close_node.jstree`.
         * @name $.jstree.defaults.state.events
         * @plugin state
         */
        events    : 'changed.jstree open_node.jstree close_node.jstree check_node.jstree uncheck_node.jstree',
        /**
         * Time in milliseconds after which the state will expire. Defaults to 'false' meaning - no expire.
         * @name $.jstree.defaults.state.ttl
         * @plugin state
         */
        ttl        : false,
        /**
         * A function that will be executed prior to restoring state with one argument - the state object. Can be used to clear unwanted parts of the state.
         * @name $.jstree.defaults.state.filter
         * @plugin state
         */
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
                /**
                 * triggered when the state plugin is finished restoring the state (and immediately after ready if there is no state to restore).
                 * @event
                 * @name state_ready.jstree
                 * @plugin state
                 */
                this.trigger('state_ready');
            }, this);
            this.element
                .on("ready.jstree", $.proxy(function (e, data) {
                        this.element.one("restore_state.jstree", bind);
                        if(!this.restore_state()) { bind(); }
                    }, this));
        };
        /**
         * save the state
         * @name save_state()
         * @plugin state
         */
        this.save_state = function () {
            var st = { 'state' : this.get_state(), 'ttl' : this.settings.state.ttl, 'sec' : +(new Date()) };
            $.vakata.storage.set(this.settings.state.key, JSON.stringify(st));
        };
        /**
         * restore the state from the user's computer
         * @name restore_state()
         * @plugin state
         */
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
        /**
         * clear the state on the user's computer
         * @name clear_state()
         * @plugin state
         */
        this.clear_state = function () {
            return $.vakata.storage.del(this.settings.state.key);
        };
    };

    (function ($, undefined) {
        $.vakata.storage = {
            // simply specifying the functions in FF throws an error
            set : function (key, val) { return window.localStorage.setItem(key, val); },
            get : function (key) { return window.localStorage.getItem(key); },
            del : function (key) { return window.localStorage.removeItem(key); }
        };
    }($));

    // include the state plugin by default
    // $.jstree.defaults.plugins.push("state");
}));