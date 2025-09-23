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
        define('jstree.unique', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.unique) { return; }

    $.jstree.defaults.unique = {
        case_sensitive : false,
        duplicate : function (name, counter) {
            return name + ' (' + counter + ')';
        }
    };

    $.jstree.plugins.unique = function (options, parent) {
        this.check = function (chk, obj, par, pos, more) {
            if(parent.check.call(this, chk, obj, par, pos, more) === false) { return false; }
            obj = obj && obj.id ? obj : this.get_node(obj);
            par = par && par.id ? par : this.get_node(par);
            if(!par || !par.children) { return true; }
            var n = chk === "rename_node" ? pos : obj.text,
                c = [],
                s = this.settings.unique.case_sensitive,
                m = this._model.data, i, j;
            for(i = 0, j = par.children.length; i < j; i++) {
                c.push(s ? m[par.children[i]].text : m[par.children[i]].text.toLowerCase());
            }
            if(!s) { n = n.toLowerCase(); }
            switch(chk) {
                case "delete_node":
                    return true;
                case "rename_node":
                    i = ($.inArray(n, c) === -1 || (obj.text && obj.text[ s ? 'toString' : 'toLowerCase']() === n));
                    if(!i) {
                        this._data.core.last_error = { 'error' : 'check', 'plugin' : 'unique', 'id' : 'unique_01', 'reason' : 'Child with name ' + n + ' already exists. Preventing: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                    }
                    return i;
                case "create_node":
                    i = ($.inArray(n, c) === -1);
                    if(!i) {
                        this._data.core.last_error = { 'error' : 'check', 'plugin' : 'unique', 'id' : 'unique_04', 'reason' : 'Child with name ' + n + ' already exists. Preventing: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                    }
                    return i;
                case "copy_node":
                    i = ($.inArray(n, c) === -1);
                    if(!i) {
                        this._data.core.last_error = { 'error' : 'check', 'plugin' : 'unique', 'id' : 'unique_02', 'reason' : 'Child with name ' + n + ' already exists. Preventing: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                    }
                    return i;
                case "move_node":
                    i = ( (obj.parent === par.id && (!more || !more.is_multi)) || $.inArray(n, c) === -1);
                    if(!i) {
                        this._data.core.last_error = { 'error' : 'check', 'plugin' : 'unique', 'id' : 'unique_03', 'reason' : 'Child with name ' + n + ' already exists. Preventing: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                    }
                    return i;
            }
            return true;
        };
        this.create_node = function (par, node, pos, callback, is_loaded) {
            if(!node || node.text === undefined) {
                if(par === null) {
                    par = $.jstree.root;
                }
                par = this.get_node(par);
                if(!par) {
                    return parent.create_node.call(this, par, node, pos, callback, is_loaded);
                }
                pos = pos === undefined ? "last" : pos;
                if(!pos.toString().match(/^(before|after)$/) && !is_loaded && !this.is_loaded(par)) {
                    return parent.create_node.call(this, par, node, pos, callback, is_loaded);
                }
                if(!node) { node = {}; }
                var tmp, n, dpc, i, j, m = this._model.data, s = this.settings.unique.case_sensitive, cb = this.settings.unique.duplicate;
                n = tmp = this.get_string('New node');
                dpc = [];
                for(i = 0, j = par.children.length; i < j; i++) {
                    dpc.push(s ? m[par.children[i]].text : m[par.children[i]].text.toLowerCase());
                }
                i = 1;
                while($.inArray(s ? n : n.toLowerCase(), dpc) !== -1) {
                    n = cb.call(this, tmp, (++i)).toString();
                }
                node.text = n;
            }
            return parent.create_node.call(this, par, node, pos, callback, is_loaded);
        };
    };

}));
