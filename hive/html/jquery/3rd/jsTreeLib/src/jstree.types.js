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
        define('jstree.types', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.types) { return; }

    $.jstree.defaults.types = {
        'default' : {}
    };
    $.jstree.defaults.types[$.jstree.root] = {};

    $.jstree.plugins.types = function (options, parent) {
        this.init = function (el, options) {
            var i, j;
            if(options && options.types && options.types['default']) {
                for(i in options.types) {
                    if(i !== "default" && i !== $.jstree.root && options.types.hasOwnProperty(i)) {
                        for(j in options.types['default']) {
                            if(options.types['default'].hasOwnProperty(j) && options.types[i][j] === undefined) {
                                options.types[i][j] = options.types['default'][j];
                            }
                        }
                    }
                }
            }
            parent.init.call(this, el, options);
            this._model.data[$.jstree.root].type = $.jstree.root;
        };
        this.refresh = function (skip_loading, forget_state) {
            parent.refresh.call(this, skip_loading, forget_state);
            this._model.data[$.jstree.root].type = $.jstree.root;
        };
        this.bind = function () {
            this.element
                .on('model.jstree', $.proxy(function (e, data) {
                        var m = this._model.data,
                            dpc = data.nodes,
                            t = this.settings.types,
                            i, j, c = 'default';
                        for(i = 0, j = dpc.length; i < j; i++) {
                            c = 'default';
                            if(m[dpc[i]].original && m[dpc[i]].original.type && t[m[dpc[i]].original.type]) {
                                c = m[dpc[i]].original.type;
                            }
                            if(m[dpc[i]].data && m[dpc[i]].data.jstree && m[dpc[i]].data.jstree.type && t[m[dpc[i]].data.jstree.type]) {
                                c = m[dpc[i]].data.jstree.type;
                            }
                            m[dpc[i]].type = c;
                            if(m[dpc[i]].icon === true && t[c].icon !== undefined) {
                                m[dpc[i]].icon = t[c].icon;
                            }
                        }
                        m[$.jstree.root].type = $.jstree.root;
                    }, this));
            parent.bind.call(this);
        };
        this.get_json = function (obj, options, flat) {
            var i, j,
                m = this._model.data,
                opt = options ? $.extend(true, {}, options, {no_id:false}) : {},
                tmp = parent.get_json.call(this, obj, opt, flat);
            if(tmp === false) { return false; }
            if($.isArray(tmp)) {
                for(i = 0, j = tmp.length; i < j; i++) {
                    tmp[i].type = tmp[i].id && m[tmp[i].id] && m[tmp[i].id].type ? m[tmp[i].id].type : "default";
                    if(options && options.no_id) {
                        delete tmp[i].id;
                        if(tmp[i].li_attr && tmp[i].li_attr.id) {
                            delete tmp[i].li_attr.id;
                        }
                        if(tmp[i].a_attr && tmp[i].a_attr.id) {
                            delete tmp[i].a_attr.id;
                        }
                    }
                }
            }
            else {
                tmp.type = tmp.id && m[tmp.id] && m[tmp.id].type ? m[tmp.id].type : "default";
                if(options && options.no_id) {
                    tmp = this._delete_ids(tmp);
                }
            }
            return tmp;
        };
        this._delete_ids = function (tmp) {
            if($.isArray(tmp)) {
                for(var i = 0, j = tmp.length; i < j; i++) {
                    tmp[i] = this._delete_ids(tmp[i]);
                }
                return tmp;
            }
            delete tmp.id;
            if(tmp.li_attr && tmp.li_attr.id) {
                delete tmp.li_attr.id;
            }
            if(tmp.a_attr && tmp.a_attr.id) {
                delete tmp.a_attr.id;
            }
            if(tmp.children && $.isArray(tmp.children)) {
                tmp.children = this._delete_ids(tmp.children);
            }
            return tmp;
        };
        this.check = function (chk, obj, par, pos, more) {
            if(parent.check.call(this, chk, obj, par, pos, more) === false) { return false; }
            obj = obj && obj.id ? obj : this.get_node(obj);
            par = par && par.id ? par : this.get_node(par);
            var m = obj && obj.id ? (more && more.origin ? more.origin : $.jstree.reference(obj.id)) : null, tmp, d, i, j;
            m = m && m._model && m._model.data ? m._model.data : null;
            switch(chk) {
                case "create_node":
                case "move_node":
                case "copy_node":
                    if(chk !== 'move_node' || $.inArray(obj.id, par.children) === -1) {
                        tmp = this.get_rules(par);
                        if(tmp.max_children !== undefined && tmp.max_children !== -1 && tmp.max_children === par.children.length) {
                            this._data.core.last_error = { 'error' : 'check', 'plugin' : 'types', 'id' : 'types_01', 'reason' : 'max_children prevents function: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                            return false;
                        }
                        if(tmp.valid_children !== undefined && tmp.valid_children !== -1 && $.inArray((obj.type || 'default'), tmp.valid_children) === -1) {
                            this._data.core.last_error = { 'error' : 'check', 'plugin' : 'types', 'id' : 'types_02', 'reason' : 'valid_children prevents function: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                            return false;
                        }
                        if(m && obj.children_d && obj.parents) {
                            d = 0;
                            for(i = 0, j = obj.children_d.length; i < j; i++) {
                                d = Math.max(d, m[obj.children_d[i]].parents.length);
                            }
                            d = d - obj.parents.length + 1;
                        }
                        if(d <= 0 || d === undefined) { d = 1; }
                        do {
                            if(tmp.max_depth !== undefined && tmp.max_depth !== -1 && tmp.max_depth < d) {
                                this._data.core.last_error = { 'error' : 'check', 'plugin' : 'types', 'id' : 'types_03', 'reason' : 'max_depth prevents function: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                                return false;
                            }
                            par = this.get_node(par.parent);
                            tmp = this.get_rules(par);
                            d++;
                        } while(par);
                    }
                    break;
            }
            return true;
        };
        this.get_rules = function (obj) {
            obj = this.get_node(obj);
            if(!obj) { return false; }
            var tmp = this.get_type(obj, true);
            if(tmp.max_depth === undefined) { tmp.max_depth = -1; }
            if(tmp.max_children === undefined) { tmp.max_children = -1; }
            if(tmp.valid_children === undefined) { tmp.valid_children = -1; }
            return tmp;
        };
        this.get_type = function (obj, rules) {
            obj = this.get_node(obj);
            return (!obj) ? false : ( rules ? $.extend({ 'type' : obj.type }, this.settings.types[obj.type]) : obj.type);
        };
        this.set_type = function (obj, type) {
            var t, t1, t2, old_type, old_icon;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.set_type(obj[t1], type);
                }
                return true;
            }
            t = this.settings.types;
            obj = this.get_node(obj);
            if(!t[type] || !obj) { return false; }
            old_type = obj.type;
            old_icon = this.get_icon(obj);
            obj.type = type;
            if(old_icon === true || (t[old_type] && t[old_type].icon !== undefined && old_icon === t[old_type].icon)) {
                this.set_icon(obj, t[type].icon !== undefined ? t[type].icon : true);
            }
            return true;
        };
    };
}));