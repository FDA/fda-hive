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
        define(['jquery'], factory);
    }
    else if(typeof module !== 'undefined' && module.exports) {
        module.exports = factory(require('jquery'));
    }
    else {
        factory(jQuery);
    }
}(function ($, undefined) {
    "use strict";

    if($.jstree) {
        return;
    }


    var instance_counter = 0,
        ccp_node = false,
        ccp_mode = false,
        ccp_inst = false,
        themes_loaded = [],
        src = $('script:last').attr('src'),
        document = window.document,
        _node = document.createElement('LI'), _temp1, _temp2;

    _node.setAttribute('role', 'treeitem');
    _temp1 = document.createElement('I');
    _temp1.className = 'jstree-icon jstree-ocl';
    _temp1.setAttribute('role', 'presentation');
    _node.appendChild(_temp1);
    _temp1 = document.createElement('A');
    _temp1.className = 'jstree-anchor';
    _temp1.setAttribute('href','#');
    _temp1.setAttribute('tabindex','-1');
    _temp2 = document.createElement('I');
    _temp2.className = 'jstree-icon jstree-themeicon';
    _temp2.setAttribute('role', 'presentation');
    _temp1.appendChild(_temp2);
    _node.appendChild(_temp1);
    _temp1 = _temp2 = null;


    $.jstree = {
        version : '{{VERSION}}',
        defaults : {
            plugins : []
        },
        plugins : {},
        path : src && src.indexOf('/') !== -1 ? src.replace(/\/[^\/]+$/,'') : '',
        idregex : /[\\:&!^|()\[\]<>@*'+~#";.,=\- \/${}%?`]/g,
        root : '#'
    };
    $.jstree.create = function (el, options) {
        var tmp = new $.jstree.core(++instance_counter),
            opt = options;
        options = $.extend(true, {}, $.jstree.defaults, options);
        if(opt && opt.plugins) {
            options.plugins = opt.plugins;
        }
        $.each(options.plugins, function (i, k) {
            if(i !== 'core') {
                tmp = tmp.plugin(k, options[k]);
            }
        });
        $(el).data('jstree', tmp);
        tmp.init(el, options);
        return tmp;
    };
    $.jstree.destroy = function () {
        $('.jstree:jstree').jstree('destroy');
        $(document).off('.jstree');
    };
    $.jstree.core = function (id) {
        this._id = id;
        this._cnt = 0;
        this._wrk = null;
        this._data = {
            core : {
                themes : {
                    name : false,
                    dots : false,
                    icons : false
                },
                selected : [],
                last_error : {},
                working : false,
                worker_queue : [],
                focused : null
            }
        };
    };
    $.jstree.reference = function (needle) {
        var tmp = null,
            obj = null;
        if(needle && needle.id && (!needle.tagName || !needle.nodeType)) { needle = needle.id; }

        if(!obj || !obj.length) {
            try { obj = $(needle); } catch (ignore) { }
        }
        if(!obj || !obj.length) {
            try { obj = $('#' + needle.replace($.jstree.idregex,'\\$&')); } catch (ignore) { }
        }
        if(obj && obj.length && (obj = obj.closest('.jstree')).length && (obj = obj.data('jstree'))) {
            tmp = obj;
        }
        else {
            $('.jstree').each(function () {
                var inst = $(this).data('jstree');
                if(inst && inst._model.data[needle]) {
                    tmp = inst;
                    return false;
                }
            });
        }
        return tmp;
    };
    $.fn.jstree = function (arg) {
        var is_method    = (typeof arg === 'string'),
            args        = Array.prototype.slice.call(arguments, 1),
            result        = null;
        if(arg === true && !this.length) { return false; }
        this.each(function () {
            var instance = $.jstree.reference(this),
                method = is_method && instance ? instance[arg] : null;
            result = is_method && method ?
                method.apply(instance, args) :
                null;
            if(!instance && !is_method && (arg === undefined || $.isPlainObject(arg))) {
                $.jstree.create(this, arg);
            }
            if( (instance && !is_method) || arg === true ) {
                result = instance || false;
            }
            if(result !== null && result !== undefined) {
                return false;
            }
        });
        return result !== null && result !== undefined ?
            result : this;
    };
    $.expr[':'].jstree = $.expr.createPseudo(function(search) {
        return function(a) {
            return $(a).hasClass('jstree') &&
                $(a).data('jstree') !== undefined;
        };
    });

    $.jstree.defaults.core = {
        data            : false,
        strings            : false,
        check_callback    : false,
        error            : $.noop,
        animation        : 200,
        multiple        : true,
        themes            : {
            name            : false,
            url                : false,
            dir                : false,
            dots            : true,
            icons            : true,
            stripes            : false,
            variant            : false,
            responsive        : false
        },
        expand_selected_onload : true,
        worker : true,
        force_text : false,
        dblclick_toggle : true
    };
    $.jstree.core.prototype = {
        plugin : function (deco, opts) {
            var Child = $.jstree.plugins[deco];
            if(Child) {
                this._data[deco] = {};
                Child.prototype = this;
                return new Child(opts, this);
            }
            return this;
        },
        init : function (el, options) {
            this._model = {
                data : {},
                changed : [],
                force_full_redraw : false,
                redraw_timeout : false,
                default_state : {
                    loaded : true,
                    opened : false,
                    selected : false,
                    disabled : false
                }
            };
            this._model.data[$.jstree.root] = {
                id : $.jstree.root,
                parent : null,
                parents : [],
                children : [],
                children_d : [],
                state : { loaded : false }
            };

            this.element = $(el).addClass('jstree jstree-' + this._id);
            this.settings = options;

            this._data.core.ready = false;
            this._data.core.loaded = false;
            this._data.core.rtl = (this.element.css("direction") === "rtl");
            this.element[this._data.core.rtl ? 'addClass' : 'removeClass']("jstree-rtl");
            this.element.attr('role','tree');
            if(this.settings.core.multiple) {
                this.element.attr('aria-multiselectable', true);
            }
            if(!this.element.attr('tabindex')) {
                this.element.attr('tabindex','0');
            }

            this.bind();
            this.trigger("init");

            this._data.core.original_container_html = this.element.find(" > ul > li").clone(true);
            this._data.core.original_container_html
                .find("li").addBack()
                .contents().filter(function() {
                    return this.nodeType === 3 && (!this.nodeValue || /^\s+$/.test(this.nodeValue));
                })
                .remove();
            this.element.html("<"+"ul class='jstree-container-ul jstree-children' role='group'><"+"li id='j"+this._id+"_loading' class='jstree-initial-node jstree-loading jstree-leaf jstree-last' role='tree-item'><i class='jstree-icon jstree-ocl'></i><"+"a class='jstree-anchor' href='#'><i class='jstree-icon jstree-themeicon-hidden'></i>" + this.get_string("Loading ...") + "</a></li></ul>");
            this.element.attr('aria-activedescendant','j' + this._id + '_loading');
            this._data.core.li_height = this.get_container_ul().children("li").first().height() || 24;
            this.trigger("loading");
            this.load_node($.jstree.root);
        },
        destroy : function (keep_html) {
            if(this._wrk) {
                try {
                    window.URL.revokeObjectURL(this._wrk);
                    this._wrk = null;
                }
                catch (ignore) { }
            }
            if(!keep_html) { this.element.empty(); }
            this.teardown();
        },
        teardown : function () {
            this.unbind();
            this.element
                .removeClass('jstree')
                .removeData('jstree')
                .find("[class^='jstree']")
                    .addBack()
                    .attr("class", function () { return this.className.replace(/jstree[^ ]*|$/ig,''); });
            this.element = null;
        },
        bind : function () {
            var word = '',
                tout = null,
                was_click = 0;
            this.element
                .on("dblclick.jstree", function (e) {
                        if(e.target.tagName && e.target.tagName.toLowerCase() === "input") { return true; }
                        if(document.selection && document.selection.empty) {
                            document.selection.empty();
                        }
                        else {
                            if(window.getSelection) {
                                var sel = window.getSelection();
                                try {
                                    sel.removeAllRanges();
                                    sel.collapse();
                                } catch (ignore) { }
                            }
                        }
                    })
                .on("mousedown.jstree", $.proxy(function (e) {
                        if(e.target === this.element[0]) {
                            e.preventDefault();
                            was_click = +(new Date());
                        }
                    }, this))
                .on("mousedown.jstree", ".jstree-ocl", function (e) {
                        e.preventDefault();
                    })
                .on("click.jstree", ".jstree-ocl", $.proxy(function (e) {
                        this.toggle_node(e.target);
                    }, this))
                .on("dblclick.jstree", ".jstree-anchor", $.proxy(function (e) {
                        if(e.target.tagName && e.target.tagName.toLowerCase() === "input") { return true; }
                        if(this.settings.core.dblclick_toggle) {
                            this.toggle_node(e.target);
                        }
                    }, this))
                .on("click.jstree", ".jstree-anchor", $.proxy(function (e) {
                        e.preventDefault();
                        if(e.currentTarget !== document.activeElement) { $(e.currentTarget).focus(); }
                        this.activate_node(e.currentTarget, e);
                    }, this))
                .on('keydown.jstree', '.jstree-anchor', $.proxy(function (e) {
                        if(e.target.tagName && e.target.tagName.toLowerCase() === "input") { return true; }
                        if(e.which !== 32 && e.which !== 13 && (e.shiftKey || e.ctrlKey || e.altKey || e.metaKey)) { return true; }
                        var o = null;
                        if(this._data.core.rtl) {
                            if(e.which === 37) { e.which = 39; }
                            else if(e.which === 39) { e.which = 37; }
                        }
                        switch(e.which) {
                            case 32:
                                if(e.ctrlKey) {
                                    e.type = "click";
                                    $(e.currentTarget).trigger(e);
                                }
                                break;
                            case 13:
                                e.type = "click";
                                $(e.currentTarget).trigger(e);
                                break;
                            case 37:
                                e.preventDefault();
                                if(this.is_open(e.currentTarget)) {
                                    this.close_node(e.currentTarget);
                                }
                                else {
                                    o = this.get_parent(e.currentTarget);
                                    if(o && o.id !== $.jstree.root) { this.get_node(o, true).children('.jstree-anchor').focus(); }
                                }
                                break;
                            case 38:
                                e.preventDefault();
                                o = this.get_prev_dom(e.currentTarget);
                                if(o && o.length) { o.children('.jstree-anchor').focus(); }
                                break;
                            case 39:
                                e.preventDefault();
                                if(this.is_closed(e.currentTarget)) {
                                    this.open_node(e.currentTarget, function (o) { this.get_node(o, true).children('.jstree-anchor').focus(); });
                                }
                                else if (this.is_open(e.currentTarget)) {
                                    o = this.get_node(e.currentTarget, true).children('.jstree-children')[0];
                                    if(o) { $(this._firstChild(o)).children('.jstree-anchor').focus(); }
                                }
                                break;
                            case 40:
                                e.preventDefault();
                                o = this.get_next_dom(e.currentTarget);
                                if(o && o.length) { o.children('.jstree-anchor').focus(); }
                                break;
                            case 106:
                                this.open_all();
                                break;
                            case 36:
                                e.preventDefault();
                                o = this._firstChild(this.get_container_ul()[0]);
                                if(o) { $(o).children('.jstree-anchor').filter(':visible').focus(); }
                                break;
                            case 35:
                                e.preventDefault();
                                this.element.find('.jstree-anchor').filter(':visible').last().focus();
                                break;
                        }
                    }, this))
                .on("load_node.jstree", $.proxy(function (e, data) {
                        if(data.status) {
                            if(data.node.id === $.jstree.root && !this._data.core.loaded) {
                                this._data.core.loaded = true;
                                if(this._firstChild(this.get_container_ul()[0])) {
                                    this.element.attr('aria-activedescendant',this._firstChild(this.get_container_ul()[0]).id);
                                }
                                this.trigger("loaded");
                            }
                            if(!this._data.core.ready) {
                                setTimeout($.proxy(function() {
                                    if(this.element && !this.get_container_ul().find('.jstree-loading').length) {
                                        this._data.core.ready = true;
                                        if(this._data.core.selected.length) {
                                            if(this.settings.core.expand_selected_onload) {
                                                var tmp = [], i, j;
                                                for(i = 0, j = this._data.core.selected.length; i < j; i++) {
                                                    tmp = tmp.concat(this._model.data[this._data.core.selected[i]].parents);
                                                }
                                                tmp = $.vakata.array_unique(tmp);
                                                for(i = 0, j = tmp.length; i < j; i++) {
                                                    this.open_node(tmp[i], false, 0);
                                                }
                                            }
                                            this.trigger('changed', { 'action' : 'ready', 'selected' : this._data.core.selected });
                                        }
                                        this.trigger("ready");
                                    }
                                }, this), 0);
                            }
                        }
                    }, this))
                .on('keypress.jstree', $.proxy(function (e) {
                        if(e.target.tagName && e.target.tagName.toLowerCase() === "input") { return true; }
                        if(tout) { clearTimeout(tout); }
                        tout = setTimeout(function () {
                            word = '';
                        }, 500);

                        var chr = String.fromCharCode(e.which).toLowerCase(),
                            col = this.element.find('.jstree-anchor').filter(':visible'),
                            ind = col.index(document.activeElement) || 0,
                            end = false;
                        word += chr;

                        if(word.length > 1) {
                            col.slice(ind).each($.proxy(function (i, v) {
                                if($(v).text().toLowerCase().indexOf(word) === 0) {
                                    $(v).focus();
                                    end = true;
                                    return false;
                                }
                            }, this));
                            if(end) { return; }

                            col.slice(0, ind).each($.proxy(function (i, v) {
                                if($(v).text().toLowerCase().indexOf(word) === 0) {
                                    $(v).focus();
                                    end = true;
                                    return false;
                                }
                            }, this));
                            if(end) { return; }
                        }
                        if(new RegExp('^' + chr.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&') + '+$').test(word)) {
                            col.slice(ind + 1).each($.proxy(function (i, v) {
                                if($(v).text().toLowerCase().charAt(0) === chr) {
                                    $(v).focus();
                                    end = true;
                                    return false;
                                }
                            }, this));
                            if(end) { return; }

                            col.slice(0, ind + 1).each($.proxy(function (i, v) {
                                if($(v).text().toLowerCase().charAt(0) === chr) {
                                    $(v).focus();
                                    end = true;
                                    return false;
                                }
                            }, this));
                            if(end) { return; }
                        }
                    }, this))
                .on("init.jstree", $.proxy(function () {
                        var s = this.settings.core.themes;
                        this._data.core.themes.dots            = s.dots;
                        this._data.core.themes.stripes        = s.stripes;
                        this._data.core.themes.icons        = s.icons;
                        this.set_theme(s.name || "default", s.url);
                        this.set_theme_variant(s.variant);
                    }, this))
                .on("loading.jstree", $.proxy(function () {
                        this[ this._data.core.themes.dots ? "show_dots" : "hide_dots" ]();
                        this[ this._data.core.themes.icons ? "show_icons" : "hide_icons" ]();
                        this[ this._data.core.themes.stripes ? "show_stripes" : "hide_stripes" ]();
                    }, this))
                .on('blur.jstree', '.jstree-anchor', $.proxy(function (e) {
                        this._data.core.focused = null;
                        $(e.currentTarget).filter('.jstree-hovered').mouseleave();
                        this.element.attr('tabindex', '0');
                    }, this))
                .on('focus.jstree', '.jstree-anchor', $.proxy(function (e) {
                        var tmp = this.get_node(e.currentTarget);
                        if(tmp && tmp.id) {
                            this._data.core.focused = tmp.id;
                        }
                        this.element.find('.jstree-hovered').not(e.currentTarget).mouseleave();
                        $(e.currentTarget).mouseenter();
                        this.element.attr('tabindex', '-1');
                    }, this))
                .on('focus.jstree', $.proxy(function () {
                        if(+(new Date()) - was_click > 500 && !this._data.core.focused) {
                            was_click = 0;
                            var act = this.get_node(this.element.attr('aria-activedescendant'), true);
                            if(act) {
                                act.find('> .jstree-anchor').focus();
                            }
                        }
                    }, this))
                .on('mouseenter.jstree', '.jstree-anchor', $.proxy(function (e) {
                        this.hover_node(e.currentTarget);
                    }, this))
                .on('mouseleave.jstree', '.jstree-anchor', $.proxy(function (e) {
                        this.dehover_node(e.currentTarget);
                    }, this));
        },
        unbind : function () {
            this.element.off('.jstree');
            $(document).off('.jstree-' + this._id);
        },
        trigger : function (ev, data) {
            if(!data) {
                data = {};
            }
            data.instance = this;
            this.element.triggerHandler(ev.replace('.jstree','') + '.jstree', data);
        },
        get_container : function () {
            return this.element;
        },
        get_container_ul : function () {
            return this.element.children(".jstree-children").first();
        },
        get_string : function (key) {
            var a = this.settings.core.strings;
            if($.isFunction(a)) { return a.call(this, key); }
            if(a && a[key]) { return a[key]; }
            return key;
        },
        _firstChild : function (dom) {
            dom = dom ? dom.firstChild : null;
            while(dom !== null && dom.nodeType !== 1) {
                dom = dom.nextSibling;
            }
            return dom;
        },
        _nextSibling : function (dom) {
            dom = dom ? dom.nextSibling : null;
            while(dom !== null && dom.nodeType !== 1) {
                dom = dom.nextSibling;
            }
            return dom;
        },
        _previousSibling : function (dom) {
            dom = dom ? dom.previousSibling : null;
            while(dom !== null && dom.nodeType !== 1) {
                dom = dom.previousSibling;
            }
            return dom;
        },
        get_node : function (obj, as_dom) {
            if(obj && obj.id) {
                obj = obj.id;
            }
            var dom;
            try {
                if(this._model.data[obj]) {
                    obj = this._model.data[obj];
                }
                else if(typeof obj === "string" && this._model.data[obj.replace(/^#/, '')]) {
                    obj = this._model.data[obj.replace(/^#/, '')];
                }
                else if(typeof obj === "string" && (dom = $('#' + obj.replace($.jstree.idregex,'\\$&'), this.element)).length && this._model.data[dom.closest('.jstree-node').attr('id')]) {
                    obj = this._model.data[dom.closest('.jstree-node').attr('id')];
                }
                else if((dom = $(obj, this.element)).length && this._model.data[dom.closest('.jstree-node').attr('id')]) {
                    obj = this._model.data[dom.closest('.jstree-node').attr('id')];
                }
                else if((dom = $(obj, this.element)).length && dom.hasClass('jstree')) {
                    obj = this._model.data[$.jstree.root];
                }
                else {
                    return false;
                }

                if(as_dom) {
                    obj = obj.id === $.jstree.root ? this.element : $('#' + obj.id.replace($.jstree.idregex,'\\$&'), this.element);
                }
                return obj;
            } catch (ex) { return false; }
        },
        get_path : function (obj, glue, ids) {
            obj = obj.parents ? obj : this.get_node(obj);
            if(!obj || obj.id === $.jstree.root || !obj.parents) {
                return false;
            }
            var i, j, p = [];
            p.push(ids ? obj.id : obj.text);
            for(i = 0, j = obj.parents.length; i < j; i++) {
                p.push(ids ? obj.parents[i] : this.get_text(obj.parents[i]));
            }
            p = p.reverse().slice(1);
            return glue ? p.join(glue) : p;
        },
        get_next_dom : function (obj, strict) {
            var tmp;
            obj = this.get_node(obj, true);
            if(obj[0] === this.element[0]) {
                tmp = this._firstChild(this.get_container_ul()[0]);
                while (tmp && tmp.offsetHeight === 0) {
                    tmp = this._nextSibling(tmp);
                }
                return tmp ? $(tmp) : false;
            }
            if(!obj || !obj.length) {
                return false;
            }
            if(strict) {
                tmp = obj[0];
                do {
                    tmp = this._nextSibling(tmp);
                } while (tmp && tmp.offsetHeight === 0);
                return tmp ? $(tmp) : false;
            }
            if(obj.hasClass("jstree-open")) {
                tmp = this._firstChild(obj.children('.jstree-children')[0]);
                while (tmp && tmp.offsetHeight === 0) {
                    tmp = this._nextSibling(tmp);
                }
                if(tmp !== null) {
                    return $(tmp);
                }
            }
            tmp = obj[0];
            do {
                tmp = this._nextSibling(tmp);
            } while (tmp && tmp.offsetHeight === 0);
            if(tmp !== null) {
                return $(tmp);
            }
            return obj.parentsUntil(".jstree",".jstree-node").nextAll(".jstree-node:visible").first();
        },
        get_prev_dom : function (obj, strict) {
            var tmp;
            obj = this.get_node(obj, true);
            if(obj[0] === this.element[0]) {
                tmp = this.get_container_ul()[0].lastChild;
                while (tmp && tmp.offsetHeight === 0) {
                    tmp = this._previousSibling(tmp);
                }
                return tmp ? $(tmp) : false;
            }
            if(!obj || !obj.length) {
                return false;
            }
            if(strict) {
                tmp = obj[0];
                do {
                    tmp = this._previousSibling(tmp);
                } while (tmp && tmp.offsetHeight === 0);
                return tmp ? $(tmp) : false;
            }
            tmp = obj[0];
            do {
                tmp = this._previousSibling(tmp);
            } while (tmp && tmp.offsetHeight === 0);
            if(tmp !== null) {
                obj = $(tmp);
                while(obj.hasClass("jstree-open")) {
                    obj = obj.children(".jstree-children").first().children(".jstree-node:visible:last");
                }
                return obj;
            }
            tmp = obj[0].parentNode.parentNode;
            return tmp && tmp.className && tmp.className.indexOf('jstree-node') !== -1 ? $(tmp) : false;
        },
        get_parent : function (obj) {
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            return obj.parent;
        },
        get_children_dom : function (obj) {
            obj = this.get_node(obj, true);
            if(obj[0] === this.element[0]) {
                return this.get_container_ul().children(".jstree-node");
            }
            if(!obj || !obj.length) {
                return false;
            }
            return obj.children(".jstree-children").children(".jstree-node");
        },
        is_parent : function (obj) {
            obj = this.get_node(obj);
            return obj && (obj.state.loaded === false || obj.children.length > 0);
        },
        is_loaded : function (obj) {
            obj = this.get_node(obj);
            return obj && obj.state.loaded;
        },
        is_loading : function (obj) {
            obj = this.get_node(obj);
            return obj && obj.state && obj.state.loading;
        },
        is_open : function (obj) {
            obj = this.get_node(obj);
            return obj && obj.state.opened;
        },
        is_closed : function (obj) {
            obj = this.get_node(obj);
            return obj && this.is_parent(obj) && !obj.state.opened;
        },
        is_leaf : function (obj) {
            return !this.is_parent(obj);
        },
        load_node : function (obj, callback) {
            var k, l, i, j, c;
            if($.isArray(obj)) {
                this._load_nodes(obj.slice(), callback);
                return true;
            }
            obj = this.get_node(obj);
            if(!obj) {
                if(callback) { callback.call(this, obj, false); }
                return false;
            }
            if(obj.state.loaded) {
                obj.state.loaded = false;
                for(k = 0, l = obj.children_d.length; k < l; k++) {
                    for(i = 0, j = obj.parents.length; i < j; i++) {
                        this._model.data[obj.parents[i]].children_d = $.vakata.array_remove_item(this._model.data[obj.parents[i]].children_d, obj.children_d[k]);
                    }
                    if(this._model.data[obj.children_d[k]].state.selected) {
                        c = true;
                        this._data.core.selected = $.vakata.array_remove_item(this._data.core.selected, obj.children_d[k]);
                    }
                    delete this._model.data[obj.children_d[k]];
                }
                obj.children = [];
                obj.children_d = [];
                if(c) {
                    this.trigger('changed', { 'action' : 'load_node', 'node' : obj, 'selected' : this._data.core.selected });
                }
            }
            obj.state.failed = false;
            obj.state.loading = true;
            this.get_node(obj, true).addClass("jstree-loading").attr('aria-busy',true);
            this._load_node(obj, $.proxy(function (status) {
                obj = this._model.data[obj.id];
                obj.state.loading = false;
                obj.state.loaded = status;
                obj.state.failed = !obj.state.loaded;
                var dom = this.get_node(obj, true), i = 0, j = 0, m = this._model.data, has_children = false;
                for(i = 0, j = obj.children.length; i < j; i++) {
                    if(m[obj.children[i]] && !m[obj.children[i]].state.hidden) {
                        has_children = true;
                        break;
                    }
                }
                if(obj.state.loaded && !has_children && dom && dom.length && !dom.hasClass('jstree-leaf')) {
                    dom.removeClass('jstree-closed jstree-open').addClass('jstree-leaf');
                }
                dom.removeClass("jstree-loading").attr('aria-busy',false);
                this.trigger('load_node', { "node" : obj, "status" : status });
                if(callback) {
                    callback.call(this, obj, status);
                }
            }, this));
            return true;
        },
        _load_nodes : function (nodes, callback, is_callback) {
            var r = true,
                c = function () { this._load_nodes(nodes, callback, true); },
                m = this._model.data, i, j, tmp = [];
            for(i = 0, j = nodes.length; i < j; i++) {
                if(m[nodes[i]] && ( (!m[nodes[i]].state.loaded && !m[nodes[i]].state.failed) || !is_callback)) {
                    if(!this.is_loading(nodes[i])) {
                        this.load_node(nodes[i], c);
                    }
                    r = false;
                }
            }
            if(r) {
                for(i = 0, j = nodes.length; i < j; i++) {
                    if(m[nodes[i]] && m[nodes[i]].state.loaded) {
                        tmp.push(nodes[i]);
                    }
                }
                if(callback && !callback.done) {
                    callback.call(this, tmp);
                    callback.done = true;
                }
            }
        },
        load_all : function (obj, callback) {
            if(!obj) { obj = $.jstree.root; }
            obj = this.get_node(obj);
            if(!obj) { return false; }
            var to_load = [],
                m = this._model.data,
                c = m[obj.id].children_d,
                i, j;
            if(obj.state && !obj.state.loaded) {
                to_load.push(obj.id);
            }
            for(i = 0, j = c.length; i < j; i++) {
                if(m[c[i]] && m[c[i]].state && !m[c[i]].state.loaded) {
                    to_load.push(c[i]);
                }
            }
            if(to_load.length) {
                this._load_nodes(to_load, function () {
                    this.load_all(obj, callback);
                });
            }
            else {
                if(callback) { callback.call(this, obj); }
                this.trigger('load_all', { "node" : obj });
            }
        },
        _load_node : function (obj, callback) {
            var s = this.settings.core.data, t;
            if(!s) {
                if(obj.id === $.jstree.root) {
                    return this._append_html_data(obj, this._data.core.original_container_html.clone(true), function (status) {
                        callback.call(this, status);
                    });
                }
                else {
                    return callback.call(this, false);
                }
            }
            if($.isFunction(s)) {
                return s.call(this, obj, $.proxy(function (d) {
                    if(d === false) {
                        callback.call(this, false);
                    }
                    this[typeof d === 'string' ? '_append_html_data' : '_append_json_data'](obj, typeof d === 'string' ? $($.parseHTML(d)).filter(function () { return this.nodeType !== 3; }) : d, function (status) {
                        callback.call(this, status);
                    });
                }, this));
            }
            if(typeof s === 'object') {
                if(s.url) {
                    s = $.extend(true, {}, s);
                    if($.isFunction(s.url)) {
                        s.url = s.url.call(this, obj);
                    }
                    if($.isFunction(s.data)) {
                        s.data = s.data.call(this, obj);
                    }
                    return $.ajax(s)
                        .done($.proxy(function (d,t,x) {
                                var type = x.getResponseHeader('Content-Type');
                                if((type && type.indexOf('json') !== -1) || typeof d === "object") {
                                    return this._append_json_data(obj, d, function (status) { callback.call(this, status); });
                                }
                                if((type && type.indexOf('html') !== -1) || typeof d === "string") {
                                    return this._append_html_data(obj, $($.parseHTML(d)).filter(function () { return this.nodeType !== 3; }), function (status) { callback.call(this, status); });
                                }
                                this._data.core.last_error = { 'error' : 'ajax', 'plugin' : 'core', 'id' : 'core_04', 'reason' : 'Could not load node', 'data' : JSON.stringify({ 'id' : obj.id, 'xhr' : x }) };
                                this.settings.core.error.call(this, this._data.core.last_error);
                                return callback.call(this, false);
                            }, this))
                        .fail($.proxy(function (f) {
                                callback.call(this, false);
                                this._data.core.last_error = { 'error' : 'ajax', 'plugin' : 'core', 'id' : 'core_04', 'reason' : 'Could not load node', 'data' : JSON.stringify({ 'id' : obj.id, 'xhr' : f }) };
                                this.settings.core.error.call(this, this._data.core.last_error);
                            }, this));
                }
                t = ($.isArray(s) || $.isPlainObject(s)) ? JSON.parse(JSON.stringify(s)) : s;
                if(obj.id === $.jstree.root) {
                    return this._append_json_data(obj, t, function (status) {
                        callback.call(this, status);
                    });
                }
                else {
                    this._data.core.last_error = { 'error' : 'nodata', 'plugin' : 'core', 'id' : 'core_05', 'reason' : 'Could not load node', 'data' : JSON.stringify({ 'id' : obj.id }) };
                    this.settings.core.error.call(this, this._data.core.last_error);
                    return callback.call(this, false);
                }
            }
            if(typeof s === 'string') {
                if(obj.id === $.jstree.root) {
                    return this._append_html_data(obj, $($.parseHTML(s)).filter(function () { return this.nodeType !== 3; }), function (status) {
                        callback.call(this, status);
                    });
                }
                else {
                    this._data.core.last_error = { 'error' : 'nodata', 'plugin' : 'core', 'id' : 'core_06', 'reason' : 'Could not load node', 'data' : JSON.stringify({ 'id' : obj.id }) };
                    this.settings.core.error.call(this, this._data.core.last_error);
                    return callback.call(this, false);
                }
            }
            return callback.call(this, false);
        },
        _node_changed : function (obj) {
            obj = this.get_node(obj);
            if(obj) {
                this._model.changed.push(obj.id);
            }
        },
        _append_html_data : function (dom, data, cb) {
            dom = this.get_node(dom);
            dom.children = [];
            dom.children_d = [];
            var dat = data.is('ul') ? data.children() : data,
                par = dom.id,
                chd = [],
                dpc = [],
                m = this._model.data,
                p = m[par],
                s = this._data.core.selected.length,
                tmp, i, j;
            dat.each($.proxy(function (i, v) {
                tmp = this._parse_model_from_html($(v), par, p.parents.concat());
                if(tmp) {
                    chd.push(tmp);
                    dpc.push(tmp);
                    if(m[tmp].children_d.length) {
                        dpc = dpc.concat(m[tmp].children_d);
                    }
                }
            }, this));
            p.children = chd;
            p.children_d = dpc;
            for(i = 0, j = p.parents.length; i < j; i++) {
                m[p.parents[i]].children_d = m[p.parents[i]].children_d.concat(dpc);
            }
            this.trigger('model', { "nodes" : dpc, 'parent' : par });
            if(par !== $.jstree.root) {
                this._node_changed(par);
                this.redraw();
            }
            else {
                this.get_container_ul().children('.jstree-initial-node').remove();
                this.redraw(true);
            }
            if(this._data.core.selected.length !== s) {
                this.trigger('changed', { 'action' : 'model', 'selected' : this._data.core.selected });
            }
            cb.call(this, true);
        },
        _append_json_data : function (dom, data, cb, force_processing) {
            if(this.element === null) { return; }
            dom = this.get_node(dom);
            dom.children = [];
            dom.children_d = [];
            if(data.d) {
                data = data.d;
                if(typeof data === "string") {
                    data = JSON.parse(data);
                }
            }
            if(!$.isArray(data)) { data = [data]; }
            var w = null,
                args = {
                    'df'    : this._model.default_state,
                    'dat'    : data,
                    'par'    : dom.id,
                    'm'        : this._model.data,
                    't_id'    : this._id,
                    't_cnt'    : this._cnt,
                    'sel'    : this._data.core.selected
                },
                func = function (data, undefined) {
                    if(data.data) { data = data.data; }
                    var dat = data.dat,
                        par = data.par,
                        chd = [],
                        dpc = [],
                        add = [],
                        df = data.df,
                        t_id = data.t_id,
                        t_cnt = data.t_cnt,
                        m = data.m,
                        p = m[par],
                        sel = data.sel,
                        tmp, i, j, rslt,
                        parse_flat = function (d, p, ps) {
                            if(!ps) { ps = []; }
                            else { ps = ps.concat(); }
                            if(p) { ps.unshift(p); }
                            var tid = d.id.toString(),
                                i, j, c, e,
                                tmp = {
                                    id            : tid,
                                    text        : d.text || '',
                                    icon        : d.icon !== undefined ? d.icon : true,
                                    parent        : p,
                                    parents        : ps,
                                    children    : d.children || [],
                                    children_d    : d.children_d || [],
                                    data        : d.data,
                                    state        : { },
                                    li_attr        : { id : false },
                                    a_attr        : { href : '#' },
                                    original    : false
                                };
                            for(i in df) {
                                if(df.hasOwnProperty(i)) {
                                    tmp.state[i] = df[i];
                                }
                            }
                            if(d && d.data && d.data.jstree && d.data.jstree.icon) {
                                tmp.icon = d.data.jstree.icon;
                            }
                            if(tmp.icon === undefined || tmp.icon === null || tmp.icon === "") {
                                tmp.icon = true;
                            }
                            if(d && d.data) {
                                tmp.data = d.data;
                                if(d.data.jstree) {
                                    for(i in d.data.jstree) {
                                        if(d.data.jstree.hasOwnProperty(i)) {
                                            tmp.state[i] = d.data.jstree[i];
                                        }
                                    }
                                }
                            }
                            if(d && typeof d.state === 'object') {
                                for (i in d.state) {
                                    if(d.state.hasOwnProperty(i)) {
                                        tmp.state[i] = d.state[i];
                                    }
                                }
                            }
                            if(d && typeof d.li_attr === 'object') {
                                for (i in d.li_attr) {
                                    if(d.li_attr.hasOwnProperty(i)) {
                                        tmp.li_attr[i] = d.li_attr[i];
                                    }
                                }
                            }
                            if(!tmp.li_attr.id) {
                                tmp.li_attr.id = tid;
                            }
                            if(d && typeof d.a_attr === 'object') {
                                for (i in d.a_attr) {
                                    if(d.a_attr.hasOwnProperty(i)) {
                                        tmp.a_attr[i] = d.a_attr[i];
                                    }
                                }
                            }
                            if(d && d.children && d.children === true) {
                                tmp.state.loaded = false;
                                tmp.children = [];
                                tmp.children_d = [];
                            }
                            m[tmp.id] = tmp;
                            for(i = 0, j = tmp.children.length; i < j; i++) {
                                c = parse_flat(m[tmp.children[i]], tmp.id, ps);
                                e = m[c];
                                tmp.children_d.push(c);
                                if(e.children_d.length) {
                                    tmp.children_d = tmp.children_d.concat(e.children_d);
                                }
                            }
                            delete d.data;
                            delete d.children;
                            m[tmp.id].original = d;
                            if(tmp.state.selected) {
                                add.push(tmp.id);
                            }
                            return tmp.id;
                        },
                        parse_nest = function (d, p, ps) {
                            if(!ps) { ps = []; }
                            else { ps = ps.concat(); }
                            if(p) { ps.unshift(p); }
                            var tid = false, i, j, c, e, tmp;
                            do {
                                tid = 'j' + t_id + '_' + (++t_cnt);
                            } while(m[tid]);

                            tmp = {
                                id            : false,
                                text        : typeof d === 'string' ? d : '',
                                icon        : typeof d === 'object' && d.icon !== undefined ? d.icon : true,
                                parent        : p,
                                parents        : ps,
                                children    : [],
                                children_d    : [],
                                data        : null,
                                state        : { },
                                li_attr        : { id : false },
                                a_attr        : { href : '#' },
                                original    : false
                            };
                            for(i in df) {
                                if(df.hasOwnProperty(i)) {
                                    tmp.state[i] = df[i];
                                }
                            }
                            if(d && d.id) { tmp.id = d.id.toString(); }
                            if(d && d.text) { tmp.text = d.text; }
                            if(d && d.data && d.data.jstree && d.data.jstree.icon) {
                                tmp.icon = d.data.jstree.icon;
                            }
                            if(tmp.icon === undefined || tmp.icon === null || tmp.icon === "") {
                                tmp.icon = true;
                            }
                            if(d && d.data) {
                                tmp.data = d.data;
                                if(d.data.jstree) {
                                    for(i in d.data.jstree) {
                                        if(d.data.jstree.hasOwnProperty(i)) {
                                            tmp.state[i] = d.data.jstree[i];
                                        }
                                    }
                                }
                            }
                            if(d && typeof d.state === 'object') {
                                for (i in d.state) {
                                    if(d.state.hasOwnProperty(i)) {
                                        tmp.state[i] = d.state[i];
                                    }
                                }
                            }
                            if(d && typeof d.li_attr === 'object') {
                                for (i in d.li_attr) {
                                    if(d.li_attr.hasOwnProperty(i)) {
                                        tmp.li_attr[i] = d.li_attr[i];
                                    }
                                }
                            }
                            if(tmp.li_attr.id && !tmp.id) {
                                tmp.id = tmp.li_attr.id.toString();
                            }
                            if(!tmp.id) {
                                tmp.id = tid;
                            }
                            if(!tmp.li_attr.id) {
                                tmp.li_attr.id = tmp.id;
                            }
                            if(d && typeof d.a_attr === 'object') {
                                for (i in d.a_attr) {
                                    if(d.a_attr.hasOwnProperty(i)) {
                                        tmp.a_attr[i] = d.a_attr[i];
                                    }
                                }
                            }
                            if(d && d.children && d.children.length) {
                                for(i = 0, j = d.children.length; i < j; i++) {
                                    c = parse_nest(d.children[i], tmp.id, ps);
                                    e = m[c];
                                    tmp.children.push(c);
                                    if(e.children_d.length) {
                                        tmp.children_d = tmp.children_d.concat(e.children_d);
                                    }
                                }
                                tmp.children_d = tmp.children_d.concat(tmp.children);
                            }
                            if(d && d.children && d.children === true) {
                                tmp.state.loaded = false;
                                tmp.children = [];
                                tmp.children_d = [];
                            }
                            delete d.data;
                            delete d.children;
                            tmp.original = d;
                            m[tmp.id] = tmp;
                            if(tmp.state.selected) {
                                add.push(tmp.id);
                            }
                            return tmp.id;
                        };

                    if(dat.length && dat[0].id !== undefined && dat[0].parent !== undefined) {
                        for(i = 0, j = dat.length; i < j; i++) {
                            if(!dat[i].children) {
                                dat[i].children = [];
                            }
                            m[dat[i].id.toString()] = dat[i];
                        }
                        for(i = 0, j = dat.length; i < j; i++) {
                            m[dat[i].parent.toString()].children.push(dat[i].id.toString());
                            p.children_d.push(dat[i].id.toString());
                        }
                        for(i = 0, j = p.children.length; i < j; i++) {
                            tmp = parse_flat(m[p.children[i]], par, p.parents.concat());
                            dpc.push(tmp);
                            if(m[tmp].children_d.length) {
                                dpc = dpc.concat(m[tmp].children_d);
                            }
                        }
                        for(i = 0, j = p.parents.length; i < j; i++) {
                            m[p.parents[i]].children_d = m[p.parents[i]].children_d.concat(dpc);
                        }
                        rslt = {
                            'cnt' : t_cnt,
                            'mod' : m,
                            'sel' : sel,
                            'par' : par,
                            'dpc' : dpc,
                            'add' : add
                        };
                    }
                    else {
                        for(i = 0, j = dat.length; i < j; i++) {
                            tmp = parse_nest(dat[i], par, p.parents.concat());
                            if(tmp) {
                                chd.push(tmp);
                                dpc.push(tmp);
                                if(m[tmp].children_d.length) {
                                    dpc = dpc.concat(m[tmp].children_d);
                                }
                            }
                        }
                        p.children = chd;
                        p.children_d = dpc;
                        for(i = 0, j = p.parents.length; i < j; i++) {
                            m[p.parents[i]].children_d = m[p.parents[i]].children_d.concat(dpc);
                        }
                        rslt = {
                            'cnt' : t_cnt,
                            'mod' : m,
                            'sel' : sel,
                            'par' : par,
                            'dpc' : dpc,
                            'add' : add
                        };
                    }
                    if(typeof window === 'undefined' || typeof window.document === 'undefined') {
                        postMessage(rslt);
                    }
                    else {
                        return rslt;
                    }
                },
                rslt = function (rslt, worker) {
                    if(this.element === null) { return; }
                    this._cnt = rslt.cnt;
                    this._model.data = rslt.mod;

                    if(worker) {
                        var i, j, a = rslt.add, r = rslt.sel, s = this._data.core.selected.slice(), m = this._model.data;
                        if(r.length !== s.length || $.vakata.array_unique(r.concat(s)).length !== r.length) {
                            for(i = 0, j = r.length; i < j; i++) {
                                if($.inArray(r[i], a) === -1 && $.inArray(r[i], s) === -1) {
                                    m[r[i]].state.selected = false;
                                }
                            }
                            for(i = 0, j = s.length; i < j; i++) {
                                if($.inArray(s[i], r) === -1) {
                                    m[s[i]].state.selected = true;
                                }
                            }
                        }
                    }
                    if(rslt.add.length) {
                        this._data.core.selected = this._data.core.selected.concat(rslt.add);
                    }

                    this.trigger('model', { "nodes" : rslt.dpc, 'parent' : rslt.par });

                    if(rslt.par !== $.jstree.root) {
                        this._node_changed(rslt.par);
                        this.redraw();
                    }
                    else {
                        this.redraw(true);
                    }
                    if(rslt.add.length) {
                        this.trigger('changed', { 'action' : 'model', 'selected' : this._data.core.selected });
                    }
                    cb.call(this, true);
                };
            if(this.settings.core.worker && window.Blob && window.URL && window.Worker) {
                try {
                    if(this._wrk === null) {
                        this._wrk = window.URL.createObjectURL(
                            new window.Blob(
                                ['self.onmessage = ' + func.toString()],
                                {type:"text/javascript"}
                            )
                        );
                    }
                    if(!this._data.core.working || force_processing) {
                        this._data.core.working = true;
                        w = new window.Worker(this._wrk);
                        w.onmessage = $.proxy(function (e) {
                            rslt.call(this, e.data, true);
                            try { w.terminate(); w = null; } catch(ignore) { }
                            if(this._data.core.worker_queue.length) {
                                this._append_json_data.apply(this, this._data.core.worker_queue.shift());
                            }
                            else {
                                this._data.core.working = false;
                            }
                        }, this);
                        if(!args.par) {
                            if(this._data.core.worker_queue.length) {
                                this._append_json_data.apply(this, this._data.core.worker_queue.shift());
                            }
                            else {
                                this._data.core.working = false;
                            }
                        }
                        else {
                            w.postMessage(args);
                        }
                    }
                    else {
                        this._data.core.worker_queue.push([dom, data, cb, true]);
                    }
                }
                catch(e) {
                    rslt.call(this, func(args), false);
                    if(this._data.core.worker_queue.length) {
                        this._append_json_data.apply(this, this._data.core.worker_queue.shift());
                    }
                    else {
                        this._data.core.working = false;
                    }
                }
            }
            else {
                rslt.call(this, func(args), false);
            }
        },
        _parse_model_from_html : function (d, p, ps) {
            if(!ps) { ps = []; }
            else { ps = [].concat(ps); }
            if(p) { ps.unshift(p); }
            var c, e, m = this._model.data,
                data = {
                    id            : false,
                    text        : false,
                    icon        : true,
                    parent        : p,
                    parents        : ps,
                    children    : [],
                    children_d    : [],
                    data        : null,
                    state        : { },
                    li_attr        : { id : false },
                    a_attr        : { href : '#' },
                    original    : false
                }, i, tmp, tid;
            for(i in this._model.default_state) {
                if(this._model.default_state.hasOwnProperty(i)) {
                    data.state[i] = this._model.default_state[i];
                }
            }
            tmp = $.vakata.attributes(d, true);
            $.each(tmp, function (i, v) {
                v = $.trim(v);
                if(!v.length) { return true; }
                data.li_attr[i] = v;
                if(i === 'id') {
                    data.id = v.toString();
                }
            });
            tmp = d.children('a').first();
            if(tmp.length) {
                tmp = $.vakata.attributes(tmp, true);
                $.each(tmp, function (i, v) {
                    v = $.trim(v);
                    if(v.length) {
                        data.a_attr[i] = v;
                    }
                });
            }
            tmp = d.children("a").first().length ? d.children("a").first().clone() : d.clone();
            tmp.children("ins, i, ul").remove();
            tmp = tmp.html();
            tmp = $('<div />').html(tmp);
            data.text = this.settings.core.force_text ? tmp.text() : tmp.html();
            tmp = d.data();
            data.data = tmp ? $.extend(true, {}, tmp) : null;
            data.state.opened = d.hasClass('jstree-open');
            data.state.selected = d.children('a').hasClass('jstree-clicked');
            data.state.disabled = d.children('a').hasClass('jstree-disabled');
            if(data.data && data.data.jstree) {
                for(i in data.data.jstree) {
                    if(data.data.jstree.hasOwnProperty(i)) {
                        data.state[i] = data.data.jstree[i];
                    }
                }
            }
            tmp = d.children("a").children(".jstree-themeicon");
            if(tmp.length) {
                data.icon = tmp.hasClass('jstree-themeicon-hidden') ? false : tmp.attr('rel');
            }
            if(data.state.icon !== undefined) {
                data.icon = data.state.icon;
            }
            if(data.icon === undefined || data.icon === null || data.icon === "") {
                data.icon = true;
            }
            tmp = d.children("ul").children("li");
            do {
                tid = 'j' + this._id + '_' + (++this._cnt);
            } while(m[tid]);
            data.id = data.li_attr.id ? data.li_attr.id.toString() : tid;
            if(tmp.length) {
                tmp.each($.proxy(function (i, v) {
                    c = this._parse_model_from_html($(v), data.id, ps);
                    e = this._model.data[c];
                    data.children.push(c);
                    if(e.children_d.length) {
                        data.children_d = data.children_d.concat(e.children_d);
                    }
                }, this));
                data.children_d = data.children_d.concat(data.children);
            }
            else {
                if(d.hasClass('jstree-closed')) {
                    data.state.loaded = false;
                }
            }
            if(data.li_attr['class']) {
                data.li_attr['class'] = data.li_attr['class'].replace('jstree-closed','').replace('jstree-open','');
            }
            if(data.a_attr['class']) {
                data.a_attr['class'] = data.a_attr['class'].replace('jstree-clicked','').replace('jstree-disabled','');
            }
            m[data.id] = data;
            if(data.state.selected) {
                this._data.core.selected.push(data.id);
            }
            return data.id;
        },
        _parse_model_from_flat_json : function (d, p, ps) {
            if(!ps) { ps = []; }
            else { ps = ps.concat(); }
            if(p) { ps.unshift(p); }
            var tid = d.id.toString(),
                m = this._model.data,
                df = this._model.default_state,
                i, j, c, e,
                tmp = {
                    id            : tid,
                    text        : d.text || '',
                    icon        : d.icon !== undefined ? d.icon : true,
                    parent        : p,
                    parents        : ps,
                    children    : d.children || [],
                    children_d    : d.children_d || [],
                    data        : d.data,
                    state        : { },
                    li_attr        : { id : false },
                    a_attr        : { href : '#' },
                    original    : false
                };
            for(i in df) {
                if(df.hasOwnProperty(i)) {
                    tmp.state[i] = df[i];
                }
            }
            if(d && d.data && d.data.jstree && d.data.jstree.icon) {
                tmp.icon = d.data.jstree.icon;
            }
            if(tmp.icon === undefined || tmp.icon === null || tmp.icon === "") {
                tmp.icon = true;
            }
            if(d && d.data) {
                tmp.data = d.data;
                if(d.data.jstree) {
                    for(i in d.data.jstree) {
                        if(d.data.jstree.hasOwnProperty(i)) {
                            tmp.state[i] = d.data.jstree[i];
                        }
                    }
                }
            }
            if(d && typeof d.state === 'object') {
                for (i in d.state) {
                    if(d.state.hasOwnProperty(i)) {
                        tmp.state[i] = d.state[i];
                    }
                }
            }
            if(d && typeof d.li_attr === 'object') {
                for (i in d.li_attr) {
                    if(d.li_attr.hasOwnProperty(i)) {
                        tmp.li_attr[i] = d.li_attr[i];
                    }
                }
            }
            if(!tmp.li_attr.id) {
                tmp.li_attr.id = tid;
            }
            if(d && typeof d.a_attr === 'object') {
                for (i in d.a_attr) {
                    if(d.a_attr.hasOwnProperty(i)) {
                        tmp.a_attr[i] = d.a_attr[i];
                    }
                }
            }
            if(d && d.children && d.children === true) {
                tmp.state.loaded = false;
                tmp.children = [];
                tmp.children_d = [];
            }
            m[tmp.id] = tmp;
            for(i = 0, j = tmp.children.length; i < j; i++) {
                c = this._parse_model_from_flat_json(m[tmp.children[i]], tmp.id, ps);
                e = m[c];
                tmp.children_d.push(c);
                if(e.children_d.length) {
                    tmp.children_d = tmp.children_d.concat(e.children_d);
                }
            }
            delete d.data;
            delete d.children;
            m[tmp.id].original = d;
            if(tmp.state.selected) {
                this._data.core.selected.push(tmp.id);
            }
            return tmp.id;
        },
        _parse_model_from_json : function (d, p, ps) {
            if(!ps) { ps = []; }
            else { ps = ps.concat(); }
            if(p) { ps.unshift(p); }
            var tid = false, i, j, c, e, m = this._model.data, df = this._model.default_state, tmp;
            do {
                tid = 'j' + this._id + '_' + (++this._cnt);
            } while(m[tid]);

            tmp = {
                id            : false,
                text        : typeof d === 'string' ? d : '',
                icon        : typeof d === 'object' && d.icon !== undefined ? d.icon : true,
                parent        : p,
                parents        : ps,
                children    : [],
                children_d    : [],
                data        : null,
                state        : { },
                li_attr        : { id : false },
                a_attr        : { href : '#' },
                original    : false
            };
            for(i in df) {
                if(df.hasOwnProperty(i)) {
                    tmp.state[i] = df[i];
                }
            }
            if(d && d.id) { tmp.id = d.id.toString(); }
            if(d && d.text) { tmp.text = d.text; }
            if(d && d.data && d.data.jstree && d.data.jstree.icon) {
                tmp.icon = d.data.jstree.icon;
            }
            if(tmp.icon === undefined || tmp.icon === null || tmp.icon === "") {
                tmp.icon = true;
            }
            if(d && d.data) {
                tmp.data = d.data;
                if(d.data.jstree) {
                    for(i in d.data.jstree) {
                        if(d.data.jstree.hasOwnProperty(i)) {
                            tmp.state[i] = d.data.jstree[i];
                        }
                    }
                }
            }
            if(d && typeof d.state === 'object') {
                for (i in d.state) {
                    if(d.state.hasOwnProperty(i)) {
                        tmp.state[i] = d.state[i];
                    }
                }
            }
            if(d && typeof d.li_attr === 'object') {
                for (i in d.li_attr) {
                    if(d.li_attr.hasOwnProperty(i)) {
                        tmp.li_attr[i] = d.li_attr[i];
                    }
                }
            }
            if(tmp.li_attr.id && !tmp.id) {
                tmp.id = tmp.li_attr.id.toString();
            }
            if(!tmp.id) {
                tmp.id = tid;
            }
            if(!tmp.li_attr.id) {
                tmp.li_attr.id = tmp.id;
            }
            if(d && typeof d.a_attr === 'object') {
                for (i in d.a_attr) {
                    if(d.a_attr.hasOwnProperty(i)) {
                        tmp.a_attr[i] = d.a_attr[i];
                    }
                }
            }
            if(d && d.children && d.children.length) {
                for(i = 0, j = d.children.length; i < j; i++) {
                    c = this._parse_model_from_json(d.children[i], tmp.id, ps);
                    e = m[c];
                    tmp.children.push(c);
                    if(e.children_d.length) {
                        tmp.children_d = tmp.children_d.concat(e.children_d);
                    }
                }
                tmp.children_d = tmp.children_d.concat(tmp.children);
            }
            if(d && d.children && d.children === true) {
                tmp.state.loaded = false;
                tmp.children = [];
                tmp.children_d = [];
            }
            delete d.data;
            delete d.children;
            tmp.original = d;
            m[tmp.id] = tmp;
            if(tmp.state.selected) {
                this._data.core.selected.push(tmp.id);
            }
            return tmp.id;
        },
        _redraw : function () {
            var nodes = this._model.force_full_redraw ? this._model.data[$.jstree.root].children.concat([]) : this._model.changed.concat([]),
                f = document.createElement('UL'), tmp, i, j, fe = this._data.core.focused;
            for(i = 0, j = nodes.length; i < j; i++) {
                tmp = this.redraw_node(nodes[i], true, this._model.force_full_redraw);
                if(tmp && this._model.force_full_redraw) {
                    f.appendChild(tmp);
                }
            }
            if(this._model.force_full_redraw) {
                f.className = this.get_container_ul()[0].className;
                f.setAttribute('role','group');
                this.element.empty().append(f);
            }
            if(fe !== null) {
                tmp = this.get_node(fe, true);
                if(tmp && tmp.length && tmp.children('.jstree-anchor')[0] !== document.activeElement) {
                    tmp.children('.jstree-anchor').focus();
                }
                else {
                    this._data.core.focused = null;
                }
            }
            this._model.force_full_redraw = false;
            this._model.changed = [];
            this.trigger('redraw', { "nodes" : nodes });
        },
        redraw : function (full) {
            if(full) {
                this._model.force_full_redraw = true;
            }
            this._redraw();
        },
        draw_children : function (node) {
            var obj = this.get_node(node),
                i = false,
                j = false,
                k = false,
                d = document;
            if(!obj) { return false; }
            if(obj.id === $.jstree.root) { return this.redraw(true); }
            node = this.get_node(node, true);
            if(!node || !node.length) { return false; }

            node.children('.jstree-children').remove();
            node = node[0];
            if(obj.children.length && obj.state.loaded) {
                k = d.createElement('UL');
                k.setAttribute('role', 'group');
                k.className = 'jstree-children';
                for(i = 0, j = obj.children.length; i < j; i++) {
                    k.appendChild(this.redraw_node(obj.children[i], true, true));
                }
                node.appendChild(k);
            }
        },
        redraw_node : function (node, deep, is_callback, force_render) {
            var obj = this.get_node(node),
                par = false,
                ind = false,
                old = false,
                i = false,
                j = false,
                k = false,
                c = '',
                d = document,
                m = this._model.data,
                f = false,
                s = false,
                tmp = null,
                t = 0,
                l = 0,
                has_children = false,
                last_sibling = false;
            if(!obj) { return false; }
            if(obj.id === $.jstree.root) {  return this.redraw(true); }
            deep = deep || obj.children.length === 0;
            node = !document.querySelector ? document.getElementById(obj.id) : this.element[0].querySelector('#' + ("0123456789".indexOf(obj.id[0]) !== -1 ? '\\3' + obj.id[0] + ' ' + obj.id.substr(1).replace($.jstree.idregex,'\\$&') : obj.id.replace($.jstree.idregex,'\\$&')) );
            if(!node) {
                deep = true;
                if(!is_callback) {
                    par = obj.parent !== $.jstree.root ? $('#' + obj.parent.replace($.jstree.idregex,'\\$&'), this.element)[0] : null;
                    if(par !== null && (!par || !m[obj.parent].state.opened)) {
                        return false;
                    }
                    ind = $.inArray(obj.id, par === null ? m[$.jstree.root].children : m[obj.parent].children);
                }
            }
            else {
                node = $(node);
                if(!is_callback) {
                    par = node.parent().parent()[0];
                    if(par === this.element[0]) {
                        par = null;
                    }
                    ind = node.index();
                }
                if(!deep && obj.children.length && !node.children('.jstree-children').length) {
                    deep = true;
                }
                if(!deep) {
                    old = node.children('.jstree-children')[0];
                }
                f = node.children('.jstree-anchor')[0] === document.activeElement;
                node.remove();
            }
            node = _node.cloneNode(true);

            c = 'jstree-node ';
            for(i in obj.li_attr) {
                if(obj.li_attr.hasOwnProperty(i)) {
                    if(i === 'id') { continue; }
                    if(i !== 'class') {
                        node.setAttribute(i, obj.li_attr[i]);
                    }
                    else {
                        c += obj.li_attr[i];
                    }
                }
            }
            if(!obj.a_attr.id) {
                obj.a_attr.id = obj.id + '_anchor';
            }
            node.setAttribute('aria-selected', !!obj.state.selected);
            node.setAttribute('aria-level', obj.parents.length);
            node.setAttribute('aria-labelledby', obj.a_attr.id);
            if(obj.state.disabled) {
                node.setAttribute('aria-disabled', true);
            }

            for(i = 0, j = obj.children.length; i < j; i++) {
                if(!m[obj.children[i]].state.hidden) {
                    has_children = true;
                    break;
                }
            }
            if(obj.parent !== null && m[obj.parent] && !obj.state.hidden) {
                i = $.inArray(obj.id, m[obj.parent].children);
                last_sibling = obj.id;
                if(i !== -1) {
                    i++;
                    for(j = m[obj.parent].children.length; i < j; i++) {
                        if(!m[m[obj.parent].children[i]].state.hidden) {
                            last_sibling = m[obj.parent].children[i];
                        }
                        if(last_sibling !== obj.id) {
                            break;
                        }
                    }
                }
            }

            if(obj.state.hidden) {
                c += ' jstree-hidden';
            }
            if(obj.state.loaded && !has_children) {
                c += ' jstree-leaf';
            }
            else {
                c += obj.state.opened && obj.state.loaded ? ' jstree-open' : ' jstree-closed';
                node.setAttribute('aria-expanded', (obj.state.opened && obj.state.loaded) );
            }
            if(last_sibling === obj.id) {
                c += ' jstree-last';
            }
            node.id = obj.id;
            node.className = c;
            c = ( obj.state.selected ? ' jstree-clicked' : '') + ( obj.state.disabled ? ' jstree-disabled' : '');
            for(j in obj.a_attr) {
                if(obj.a_attr.hasOwnProperty(j)) {
                    if(j === 'href' && obj.a_attr[j] === '#') { continue; }
                    if(j !== 'class') {
                        node.childNodes[1].setAttribute(j, obj.a_attr[j]);
                    }
                    else {
                        c += ' ' + obj.a_attr[j];
                    }
                }
            }
            if(c.length) {
                node.childNodes[1].className = 'jstree-anchor ' + c;
            }
            if((obj.icon && obj.icon !== true) || obj.icon === false) {
                if(obj.icon === false) {
                    node.childNodes[1].childNodes[0].className += ' jstree-themeicon-hidden';
                }
                else if(obj.icon.indexOf('/') === -1 && obj.icon.indexOf('.') === -1) {
                    node.childNodes[1].childNodes[0].className += ' ' + obj.icon + ' jstree-themeicon-custom';
                }
                else {
                    node.childNodes[1].childNodes[0].style.backgroundImage = 'url('+obj.icon+')';
                    node.childNodes[1].childNodes[0].style.backgroundPosition = 'center center';
                    node.childNodes[1].childNodes[0].style.backgroundSize = 'auto';
                    node.childNodes[1].childNodes[0].className += ' jstree-themeicon-custom';
                }
            }

            if(this.settings.core.force_text) {
                node.childNodes[1].appendChild(d.createTextNode(obj.text));
            }
            else {
                node.childNodes[1].innerHTML += obj.text;
            }


            if(deep && obj.children.length && (obj.state.opened || force_render) && obj.state.loaded) {
                k = d.createElement('UL');
                k.setAttribute('role', 'group');
                k.className = 'jstree-children';
                for(i = 0, j = obj.children.length; i < j; i++) {
                    k.appendChild(this.redraw_node(obj.children[i], deep, true));
                }
                node.appendChild(k);
            }
            if(old) {
                node.appendChild(old);
            }
            if(!is_callback) {
                if(!par) {
                    par = this.element[0];
                }
                for(i = 0, j = par.childNodes.length; i < j; i++) {
                    if(par.childNodes[i] && par.childNodes[i].className && par.childNodes[i].className.indexOf('jstree-children') !== -1) {
                        tmp = par.childNodes[i];
                        break;
                    }
                }
                if(!tmp) {
                    tmp = d.createElement('UL');
                    tmp.setAttribute('role', 'group');
                    tmp.className = 'jstree-children';
                    par.appendChild(tmp);
                }
                par = tmp;

                if(ind < par.childNodes.length) {
                    par.insertBefore(node, par.childNodes[ind]);
                }
                else {
                    par.appendChild(node);
                }
                if(f) {
                    t = this.element[0].scrollTop;
                    l = this.element[0].scrollLeft;
                    node.childNodes[1].focus();
                    this.element[0].scrollTop = t;
                    this.element[0].scrollLeft = l;
                }
            }
            if(obj.state.opened && !obj.state.loaded) {
                obj.state.opened = false;
                setTimeout($.proxy(function () {
                    this.open_node(obj.id, false, 0);
                }, this), 0);
            }
            return node;
        },
        open_node : function (obj, callback, animation) {
            var t1, t2, d, t;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.open_node(obj[t1], callback, animation);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            animation = animation === undefined ? this.settings.core.animation : animation;
            if(!this.is_closed(obj)) {
                if(callback) {
                    callback.call(this, obj, false);
                }
                return false;
            }
            if(!this.is_loaded(obj)) {
                if(this.is_loading(obj)) {
                    return setTimeout($.proxy(function () {
                        this.open_node(obj, callback, animation);
                    }, this), 500);
                }
                this.load_node(obj, function (o, ok) {
                    return ok ? this.open_node(o, callback, animation) : (callback ? callback.call(this, o, false) : false);
                });
            }
            else {
                d = this.get_node(obj, true);
                t = this;
                if(d.length) {
                    if(animation && d.children(".jstree-children").length) {
                        d.children(".jstree-children").stop(true, true);
                    }
                    if(obj.children.length && !this._firstChild(d.children('.jstree-children')[0])) {
                        this.draw_children(obj);
                    }
                    if(!animation) {
                        this.trigger('before_open', { "node" : obj });
                        d[0].className = d[0].className.replace('jstree-closed', 'jstree-open');
                        d[0].setAttribute("aria-expanded", true);
                    }
                    else {
                        this.trigger('before_open', { "node" : obj });
                        d
                            .children(".jstree-children").css("display","none").end()
                            .removeClass("jstree-closed").addClass("jstree-open").attr("aria-expanded", true)
                            .children(".jstree-children").stop(true, true)
                                .slideDown(animation, function () {
                                    this.style.display = "";
                                    t.trigger("after_open", { "node" : obj });
                                });
                    }
                }
                obj.state.opened = true;
                if(callback) {
                    callback.call(this, obj, true);
                }
                if(!d.length) {
                    this.trigger('before_open', { "node" : obj });
                }
                this.trigger('open_node', { "node" : obj });
                if(!animation || !d.length) {
                    this.trigger("after_open", { "node" : obj });
                }
                return true;
            }
        },
        _open_to : function (obj) {
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            var i, j, p = obj.parents;
            for(i = 0, j = p.length; i < j; i+=1) {
                if(i !== $.jstree.root) {
                    this.open_node(p[i], false, 0);
                }
            }
            return $('#' + obj.id.replace($.jstree.idregex,'\\$&'), this.element);
        },
        close_node : function (obj, animation) {
            var t1, t2, t, d;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.close_node(obj[t1], animation);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            if(this.is_closed(obj)) {
                return false;
            }
            animation = animation === undefined ? this.settings.core.animation : animation;
            t = this;
            d = this.get_node(obj, true);
            if(d.length) {
                if(!animation) {
                    d[0].className = d[0].className.replace('jstree-open', 'jstree-closed');
                    d.attr("aria-expanded", false).children('.jstree-children').remove();
                }
                else {
                    d
                        .children(".jstree-children").attr("style","display:block !important").end()
                        .removeClass("jstree-open").addClass("jstree-closed").attr("aria-expanded", false)
                        .children(".jstree-children").stop(true, true).slideUp(animation, function () {
                            this.style.display = "";
                            d.children('.jstree-children').remove();
                            t.trigger("after_close", { "node" : obj });
                        });
                }
            }
            obj.state.opened = false;
            this.trigger('close_node',{ "node" : obj });
            if(!animation || !d.length) {
                this.trigger("after_close", { "node" : obj });
            }
        },
        toggle_node : function (obj) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.toggle_node(obj[t1]);
                }
                return true;
            }
            if(this.is_closed(obj)) {
                return this.open_node(obj);
            }
            if(this.is_open(obj)) {
                return this.close_node(obj);
            }
        },
        open_all : function (obj, animation, original_obj) {
            if(!obj) { obj = $.jstree.root; }
            obj = this.get_node(obj);
            if(!obj) { return false; }
            var dom = obj.id === $.jstree.root ? this.get_container_ul() : this.get_node(obj, true), i, j, _this;
            if(!dom.length) {
                for(i = 0, j = obj.children_d.length; i < j; i++) {
                    if(this.is_closed(this._model.data[obj.children_d[i]])) {
                        this._model.data[obj.children_d[i]].state.opened = true;
                    }
                }
                return this.trigger('open_all', { "node" : obj });
            }
            original_obj = original_obj || dom;
            _this = this;
            dom = this.is_closed(obj) ? dom.find('.jstree-closed').addBack() : dom.find('.jstree-closed');
            dom.each(function () {
                _this.open_node(
                    this,
                    function(node, status) { if(status && this.is_parent(node)) { this.open_all(node, animation, original_obj); } },
                    animation || 0
                );
            });
            if(original_obj.find('.jstree-closed').length === 0) {
                this.trigger('open_all', { "node" : this.get_node(original_obj) });
            }
        },
        close_all : function (obj, animation) {
            if(!obj) { obj = $.jstree.root; }
            obj = this.get_node(obj);
            if(!obj) { return false; }
            var dom = obj.id === $.jstree.root ? this.get_container_ul() : this.get_node(obj, true),
                _this = this, i, j;
            if(dom.length) {
                dom = this.is_open(obj) ? dom.find('.jstree-open').addBack() : dom.find('.jstree-open');
                $(dom.get().reverse()).each(function () { _this.close_node(this, animation || 0); });
            }
            for(i = 0, j = obj.children_d.length; i < j; i++) {
                this._model.data[obj.children_d[i]].state.opened = false;
            }
            this.trigger('close_all', { "node" : obj });
        },
        is_disabled : function (obj) {
            obj = this.get_node(obj);
            return obj && obj.state && obj.state.disabled;
        },
        enable_node : function (obj) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.enable_node(obj[t1]);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            obj.state.disabled = false;
            this.get_node(obj,true).children('.jstree-anchor').removeClass('jstree-disabled').attr('aria-disabled', false);
            this.trigger('enable_node', { 'node' : obj });
        },
        disable_node : function (obj) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.disable_node(obj[t1]);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            obj.state.disabled = true;
            this.get_node(obj,true).children('.jstree-anchor').addClass('jstree-disabled').attr('aria-disabled', true);
            this.trigger('disable_node', { 'node' : obj });
        },
        hide_node : function (obj, skip_redraw) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.hide_node(obj[t1], true);
                }
                this.redraw();
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            if(!obj.state.hidden) {
                obj.state.hidden = true;
                this._node_changed(obj.parent);
                if(!skip_redraw) {
                    this.redraw();
                }
                this.trigger('hide_node', { 'node' : obj });
            }
        },
        show_node : function (obj, skip_redraw) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.show_node(obj[t1], true);
                }
                this.redraw();
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            if(obj.state.hidden) {
                obj.state.hidden = false;
                this._node_changed(obj.parent);
                if(!skip_redraw) {
                    this.redraw();
                }
                this.trigger('show_node', { 'node' : obj });
            }
        },
        hide_all : function (skip_redraw) {
            var i, m = this._model.data, ids = [];
            for(i in m) {
                if(m.hasOwnProperty(i) && i !== $.jstree.root && !m[i].state.hidden) {
                    m[i].state.hidden = true;
                    ids.push(i);
                }
            }
            this._model.force_full_redraw = true;
            if(!skip_redraw) {
                this.redraw();
            }
            this.trigger('hide_all', { 'nodes' : ids });
            return ids;
        },
        show_all : function (skip_redraw) {
            var i, m = this._model.data, ids = [];
            for(i in m) {
                if(m.hasOwnProperty(i) && i !== $.jstree.root && m[i].state.hidden) {
                    m[i].state.hidden = false;
                    ids.push(i);
                }
            }
            this._model.force_full_redraw = true;
            if(!skip_redraw) {
                this.redraw();
            }
            this.trigger('show_all', { 'nodes' : ids });
            return ids;
        },
        activate_node : function (obj, e) {
            if(this.is_disabled(obj)) {
                return false;
            }
            if(!e || typeof e !== 'object') {
                e = {};
            }

            this._data.core.last_clicked = this._data.core.last_clicked && this._data.core.last_clicked.id !== undefined ? this.get_node(this._data.core.last_clicked.id) : null;
            if(this._data.core.last_clicked && !this._data.core.last_clicked.state.selected) { this._data.core.last_clicked = null; }
            if(!this._data.core.last_clicked && this._data.core.selected.length) { this._data.core.last_clicked = this.get_node(this._data.core.selected[this._data.core.selected.length - 1]); }

            if(!this.settings.core.multiple || (!e.metaKey && !e.ctrlKey && !e.shiftKey) || (e.shiftKey && (!this._data.core.last_clicked || !this.get_parent(obj) || this.get_parent(obj) !== this._data.core.last_clicked.parent ) )) {
                if(!this.settings.core.multiple && (e.metaKey || e.ctrlKey || e.shiftKey) && this.is_selected(obj)) {
                    this.deselect_node(obj, false, e);
                }
                else {
                    this.deselect_all(true);
                    this.select_node(obj, false, false, e);
                    this._data.core.last_clicked = this.get_node(obj);
                }
            }
            else {
                if(e.shiftKey) {
                    var o = this.get_node(obj).id,
                        l = this._data.core.last_clicked.id,
                        p = this.get_node(this._data.core.last_clicked.parent).children,
                        c = false,
                        i, j;
                    for(i = 0, j = p.length; i < j; i += 1) {
                        if(p[i] === o) {
                            c = !c;
                        }
                        if(p[i] === l) {
                            c = !c;
                        }
                        if(!this.is_disabled(p[i]) && (c || p[i] === o || p[i] === l)) {
                            this.select_node(p[i], true, false, e);
                        }
                        else {
                            this.deselect_node(p[i], true, e);
                        }
                    }
                    this.trigger('changed', { 'action' : 'select_node', 'node' : this.get_node(obj), 'selected' : this._data.core.selected, 'event' : e });
                }
                else {
                    if(!this.is_selected(obj)) {
                        this.select_node(obj, false, false, e);
                    }
                    else {
                        this.deselect_node(obj, false, e);
                    }
                }
            }
            this.trigger('activate_node', { 'node' : this.get_node(obj), 'event' : e });
        },
        hover_node : function (obj) {
            obj = this.get_node(obj, true);
            if(!obj || !obj.length || obj.children('.jstree-hovered').length) {
                return false;
            }
            var o = this.element.find('.jstree-hovered'), t = this.element;
            if(o && o.length) { this.dehover_node(o); }

            obj.children('.jstree-anchor').addClass('jstree-hovered');
            this.trigger('hover_node', { 'node' : this.get_node(obj) });
            setTimeout(function () { t.attr('aria-activedescendant', obj[0].id); }, 0);
        },
        dehover_node : function (obj) {
            obj = this.get_node(obj, true);
            if(!obj || !obj.length || !obj.children('.jstree-hovered').length) {
                return false;
            }
            obj.children('.jstree-anchor').removeClass('jstree-hovered');
            this.trigger('dehover_node', { 'node' : this.get_node(obj) });
        },
        select_node : function (obj, supress_event, prevent_open, e) {
            var dom, t1, t2, th;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.select_node(obj[t1], supress_event, prevent_open, e);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            dom = this.get_node(obj, true);
            if(!obj.state.selected) {
                obj.state.selected = true;
                this._data.core.selected.push(obj.id);
                if(!prevent_open) {
                    dom = this._open_to(obj);
                }
                if(dom && dom.length) {
                    dom.attr('aria-selected', true).children('.jstree-anchor').addClass('jstree-clicked');
                }
                this.trigger('select_node', { 'node' : obj, 'selected' : this._data.core.selected, 'event' : e });
                if(!supress_event) {
                    this.trigger('changed', { 'action' : 'select_node', 'node' : obj, 'selected' : this._data.core.selected, 'event' : e });
                }
            }
        },
        deselect_node : function (obj, supress_event, e) {
            var t1, t2, dom;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.deselect_node(obj[t1], supress_event, e);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            dom = this.get_node(obj, true);
            if(obj.state.selected) {
                obj.state.selected = false;
                this._data.core.selected = $.vakata.array_remove_item(this._data.core.selected, obj.id);
                if(dom.length) {
                    dom.attr('aria-selected', false).children('.jstree-anchor').removeClass('jstree-clicked');
                }
                this.trigger('deselect_node', { 'node' : obj, 'selected' : this._data.core.selected, 'event' : e });
                if(!supress_event) {
                    this.trigger('changed', { 'action' : 'deselect_node', 'node' : obj, 'selected' : this._data.core.selected, 'event' : e });
                }
            }
        },
        select_all : function (supress_event) {
            var tmp = this._data.core.selected.concat([]), i, j;
            this._data.core.selected = this._model.data[$.jstree.root].children_d.concat();
            for(i = 0, j = this._data.core.selected.length; i < j; i++) {
                if(this._model.data[this._data.core.selected[i]]) {
                    this._model.data[this._data.core.selected[i]].state.selected = true;
                }
            }
            this.redraw(true);
            this.trigger('select_all', { 'selected' : this._data.core.selected });
            if(!supress_event) {
                this.trigger('changed', { 'action' : 'select_all', 'selected' : this._data.core.selected, 'old_selection' : tmp });
            }
        },
        deselect_all : function (supress_event) {
            var tmp = this._data.core.selected.concat([]), i, j;
            for(i = 0, j = this._data.core.selected.length; i < j; i++) {
                if(this._model.data[this._data.core.selected[i]]) {
                    this._model.data[this._data.core.selected[i]].state.selected = false;
                }
            }
            this._data.core.selected = [];
            this.element.find('.jstree-clicked').removeClass('jstree-clicked').parent().attr('aria-selected', false);
            this.trigger('deselect_all', { 'selected' : this._data.core.selected, 'node' : tmp });
            if(!supress_event) {
                this.trigger('changed', { 'action' : 'deselect_all', 'selected' : this._data.core.selected, 'old_selection' : tmp });
            }
        },
        is_selected : function (obj) {
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) {
                return false;
            }
            return obj.state.selected;
        },
        get_selected : function (full) {
            return full ? $.map(this._data.core.selected, $.proxy(function (i) { return this.get_node(i); }, this)) : this._data.core.selected.slice();
        },
        get_top_selected : function (full) {
            var tmp = this.get_selected(true),
                obj = {}, i, j, k, l;
            for(i = 0, j = tmp.length; i < j; i++) {
                obj[tmp[i].id] = tmp[i];
            }
            for(i = 0, j = tmp.length; i < j; i++) {
                for(k = 0, l = tmp[i].children_d.length; k < l; k++) {
                    if(obj[tmp[i].children_d[k]]) {
                        delete obj[tmp[i].children_d[k]];
                    }
                }
            }
            tmp = [];
            for(i in obj) {
                if(obj.hasOwnProperty(i)) {
                    tmp.push(i);
                }
            }
            return full ? $.map(tmp, $.proxy(function (i) { return this.get_node(i); }, this)) : tmp;
        },
        get_bottom_selected : function (full) {
            var tmp = this.get_selected(true),
                obj = [], i, j;
            for(i = 0, j = tmp.length; i < j; i++) {
                if(!tmp[i].children.length) {
                    obj.push(tmp[i].id);
                }
            }
            return full ? $.map(obj, $.proxy(function (i) { return this.get_node(i); }, this)) : obj;
        },
        get_state : function () {
            var state    = {
                'core' : {
                    'open' : [],
                    'scroll' : {
                        'left' : this.element.scrollLeft(),
                        'top' : this.element.scrollTop()
                    },
                    'selected' : []
                }
            }, i;
            for(i in this._model.data) {
                if(this._model.data.hasOwnProperty(i)) {
                    if(i !== $.jstree.root) {
                        if(this._model.data[i].state.opened) {
                            state.core.open.push(i);
                        }
                        if(this._model.data[i].state.selected) {
                            state.core.selected.push(i);
                        }
                    }
                }
            }
            return state;
        },
        set_state : function (state, callback) {
            if(state) {
                if(state.core) {
                    var res, n, t, _this, i;
                    if(state.core.open) {
                        if(!$.isArray(state.core.open) || !state.core.open.length) {
                            delete state.core.open;
                            this.set_state(state, callback);
                        }
                        else {
                            this._load_nodes(state.core.open, function (nodes) {
                                this.open_node(nodes, false, 0);
                                delete state.core.open;
                                this.set_state(state, callback);
                            }, true);
                        }
                        return false;
                    }
                    if(state.core.scroll) {
                        if(state.core.scroll && state.core.scroll.left !== undefined) {
                            this.element.scrollLeft(state.core.scroll.left);
                        }
                        if(state.core.scroll && state.core.scroll.top !== undefined) {
                            this.element.scrollTop(state.core.scroll.top);
                        }
                        delete state.core.scroll;
                        this.set_state(state, callback);
                        return false;
                    }
                    if(state.core.selected) {
                        _this = this;
                        this.deselect_all();
                        $.each(state.core.selected, function (i, v) {
                            _this.select_node(v, false, true);
                        });
                        delete state.core.selected;
                        this.set_state(state, callback);
                        return false;
                    }
                    for(i in state) {
                        if(state.hasOwnProperty(i) && i !== "core" && $.inArray(i, this.settings.plugins) === -1) {
                            delete state[i];
                        }
                    }
                    if($.isEmptyObject(state.core)) {
                        delete state.core;
                        this.set_state(state, callback);
                        return false;
                    }
                }
                if($.isEmptyObject(state)) {
                    state = null;
                    if(callback) { callback.call(this); }
                    this.trigger('set_state');
                    return false;
                }
                return true;
            }
            return false;
        },
        refresh : function (skip_loading, forget_state) {
            this._data.core.state = forget_state === true ? {} : this.get_state();
            if(forget_state && $.isFunction(forget_state)) { this._data.core.state = forget_state.call(this, this._data.core.state); }
            this._cnt = 0;
            this._model.data = {};
            this._model.data[$.jstree.root] = {
                id : $.jstree.root,
                parent : null,
                parents : [],
                children : [],
                children_d : [],
                state : { loaded : false }
            };
            this._data.core.selected = [];
            this._data.core.last_clicked = null;
            this._data.core.focused = null;

            var c = this.get_container_ul()[0].className;
            if(!skip_loading) {
                this.element.html("<"+"ul class='"+c+"' role='group'><"+"li class='jstree-initial-node jstree-loading jstree-leaf jstree-last' role='treeitem' id='j"+this._id+"_loading'><i class='jstree-icon jstree-ocl'></i><"+"a class='jstree-anchor' href='#'><i class='jstree-icon jstree-themeicon-hidden'></i>" + this.get_string("Loading ...") + "</a></li></ul>");
                this.element.attr('aria-activedescendant','j'+this._id+'_loading');
            }
            this.load_node($.jstree.root, function (o, s) {
                if(s) {
                    this.get_container_ul()[0].className = c;
                    if(this._firstChild(this.get_container_ul()[0])) {
                        this.element.attr('aria-activedescendant',this._firstChild(this.get_container_ul()[0]).id);
                    }
                    this.set_state($.extend(true, {}, this._data.core.state), function () {
                        this.trigger('refresh');
                    });
                }
                this._data.core.state = null;
            });
        },
        refresh_node : function (obj) {
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            var opened = [], to_load = [], s = this._data.core.selected.concat([]);
            to_load.push(obj.id);
            if(obj.state.opened === true) { opened.push(obj.id); }
            this.get_node(obj, true).find('.jstree-open').each(function() { opened.push(this.id); });
            this._load_nodes(to_load, $.proxy(function (nodes) {
                this.open_node(opened, false, 0);
                this.select_node(this._data.core.selected);
                this.trigger('refresh_node', { 'node' : obj, 'nodes' : nodes });
            }, this));
        },
        set_id : function (obj, id) {
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            var i, j, m = this._model.data;
            id = id.toString();
            m[obj.parent].children[$.inArray(obj.id, m[obj.parent].children)] = id;
            for(i = 0, j = obj.parents.length; i < j; i++) {
                m[obj.parents[i]].children_d[$.inArray(obj.id, m[obj.parents[i]].children_d)] = id;
            }
            for(i = 0, j = obj.children.length; i < j; i++) {
                m[obj.children[i]].parent = id;
            }
            for(i = 0, j = obj.children_d.length; i < j; i++) {
                m[obj.children_d[i]].parents[$.inArray(obj.id, m[obj.children_d[i]].parents)] = id;
            }
            i = $.inArray(obj.id, this._data.core.selected);
            if(i !== -1) { this._data.core.selected[i] = id; }
            i = this.get_node(obj.id, true);
            if(i) {
                i.attr('id', id).children('.jstree-anchor').attr('id', id + '_anchor').end().attr('aria-labelledby', id + '_anchor');
                if(this.element.attr('aria-activedescendant') === obj.id) {
                    this.element.attr('aria-activedescendant', id);
                }
            }
            delete m[obj.id];
            obj.id = id;
            obj.li_attr.id = id;
            m[id] = obj;
            return true;
        },
        get_text : function (obj) {
            obj = this.get_node(obj);
            return (!obj || obj.id === $.jstree.root) ? false : obj.text;
        },
        set_text : function (obj, val) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.set_text(obj[t1], val);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            obj.text = val;
            if(this.get_node(obj, true).length) {
                this.redraw_node(obj.id);
            }
            this.trigger('set_text',{ "obj" : obj, "text" : val });
            return true;
        },
        get_json : function (obj, options, flat) {
            obj = this.get_node(obj || $.jstree.root);
            if(!obj) { return false; }
            if(options && options.flat && !flat) { flat = []; }
            var tmp = {
                'id' : obj.id,
                'text' : obj.text,
                'icon' : this.get_icon(obj),
                'li_attr' : $.extend(true, {}, obj.li_attr),
                'a_attr' : $.extend(true, {}, obj.a_attr),
                'state' : {},
                'data' : options && options.no_data ? false : $.extend(true, {}, obj.data)
            }, i, j;
            if(options && options.flat) {
                tmp.parent = obj.parent;
            }
            else {
                tmp.children = [];
            }
            if(!options || !options.no_state) {
                for(i in obj.state) {
                    if(obj.state.hasOwnProperty(i)) {
                        tmp.state[i] = obj.state[i];
                    }
                }
            }
            if(options && options.no_id) {
                delete tmp.id;
                if(tmp.li_attr && tmp.li_attr.id) {
                    delete tmp.li_attr.id;
                }
                if(tmp.a_attr && tmp.a_attr.id) {
                    delete tmp.a_attr.id;
                }
            }
            if(options && options.flat && obj.id !== $.jstree.root) {
                flat.push(tmp);
            }
            if(!options || !options.no_children) {
                for(i = 0, j = obj.children.length; i < j; i++) {
                    if(options && options.flat) {
                        this.get_json(obj.children[i], options, flat);
                    }
                    else {
                        tmp.children.push(this.get_json(obj.children[i], options));
                    }
                }
            }
            return options && options.flat ? flat : (obj.id === $.jstree.root ? tmp.children : tmp);
        },
        create_node : function (par, node, pos, callback, is_loaded) {
            if(par === null) { par = $.jstree.root; }
            par = this.get_node(par);
            if(!par) { return false; }
            pos = pos === undefined ? "last" : pos;
            if(!pos.toString().match(/^(before|after)$/) && !is_loaded && !this.is_loaded(par)) {
                return this.load_node(par, function () { this.create_node(par, node, pos, callback, true); });
            }
            if(!node) { node = { "text" : this.get_string('New node') }; }
            if(typeof node === "string") { node = { "text" : node }; }
            if(node.text === undefined) { node.text = this.get_string('New node'); }
            var tmp, dpc, i, j;

            if(par.id === $.jstree.root) {
                if(pos === "before") { pos = "first"; }
                if(pos === "after") { pos = "last"; }
            }
            switch(pos) {
                case "before":
                    tmp = this.get_node(par.parent);
                    pos = $.inArray(par.id, tmp.children);
                    par = tmp;
                    break;
                case "after" :
                    tmp = this.get_node(par.parent);
                    pos = $.inArray(par.id, tmp.children) + 1;
                    par = tmp;
                    break;
                case "inside":
                case "first":
                    pos = 0;
                    break;
                case "last":
                    pos = par.children.length;
                    break;
                default:
                    if(!pos) { pos = 0; }
                    break;
            }
            if(pos > par.children.length) { pos = par.children.length; }
            if(!node.id) { node.id = true; }
            if(!this.check("create_node", node, par, pos)) {
                this.settings.core.error.call(this, this._data.core.last_error);
                return false;
            }
            if(node.id === true) { delete node.id; }
            node = this._parse_model_from_json(node, par.id, par.parents.concat());
            if(!node) { return false; }
            tmp = this.get_node(node);
            dpc = [];
            dpc.push(node);
            dpc = dpc.concat(tmp.children_d);
            this.trigger('model', { "nodes" : dpc, "parent" : par.id });

            par.children_d = par.children_d.concat(dpc);
            for(i = 0, j = par.parents.length; i < j; i++) {
                this._model.data[par.parents[i]].children_d = this._model.data[par.parents[i]].children_d.concat(dpc);
            }
            node = tmp;
            tmp = [];
            for(i = 0, j = par.children.length; i < j; i++) {
                tmp[i >= pos ? i+1 : i] = par.children[i];
            }
            tmp[pos] = node.id;
            par.children = tmp;

            this.redraw_node(par, true);
            if(callback) { callback.call(this, this.get_node(node)); }
            this.trigger('create_node', { "node" : this.get_node(node), "parent" : par.id, "position" : pos });
            return node.id;
        },
        rename_node : function (obj, val) {
            var t1, t2, old;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.rename_node(obj[t1], val);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            old = obj.text;
            if(!this.check("rename_node", obj, this.get_parent(obj), val)) {
                this.settings.core.error.call(this, this._data.core.last_error);
                return false;
            }
            this.set_text(obj, val);
            this.trigger('rename_node', { "node" : obj, "text" : val, "old" : old });
            return true;
        },
        delete_node : function (obj) {
            var t1, t2, par, pos, tmp, i, j, k, l, c, top, lft;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.delete_node(obj[t1]);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            par = this.get_node(obj.parent);
            pos = $.inArray(obj.id, par.children);
            c = false;
            if(!this.check("delete_node", obj, par, pos)) {
                this.settings.core.error.call(this, this._data.core.last_error);
                return false;
            }
            if(pos !== -1) {
                par.children = $.vakata.array_remove(par.children, pos);
            }
            tmp = obj.children_d.concat([]);
            tmp.push(obj.id);
            for(k = 0, l = tmp.length; k < l; k++) {
                for(i = 0, j = obj.parents.length; i < j; i++) {
                    pos = $.inArray(tmp[k], this._model.data[obj.parents[i]].children_d);
                    if(pos !== -1) {
                        this._model.data[obj.parents[i]].children_d = $.vakata.array_remove(this._model.data[obj.parents[i]].children_d, pos);
                    }
                }
                if(this._model.data[tmp[k]].state.selected) {
                    c = true;
                    pos = $.inArray(tmp[k], this._data.core.selected);
                    if(pos !== -1) {
                        this._data.core.selected = $.vakata.array_remove(this._data.core.selected, pos);
                    }
                }
            }
            this.trigger('delete_node', { "node" : obj, "parent" : par.id });
            if(c) {
                this.trigger('changed', { 'action' : 'delete_node', 'node' : obj, 'selected' : this._data.core.selected, 'parent' : par.id });
            }
            for(k = 0, l = tmp.length; k < l; k++) {
                delete this._model.data[tmp[k]];
            }
            if($.inArray(this._data.core.focused, tmp) !== -1) {
                this._data.core.focused = null;
                top = this.element[0].scrollTop;
                lft = this.element[0].scrollLeft;
                if(par.id === $.jstree.root) {
                    this.get_node(this._model.data[$.jstree.root].children[0], true).children('.jstree-anchor').focus();
                }
                else {
                    this.get_node(par, true).children('.jstree-anchor').focus();
                }
                this.element[0].scrollTop  = top;
                this.element[0].scrollLeft = lft;
            }
            this.redraw_node(par, true);
            return true;
        },
        check : function (chk, obj, par, pos, more) {
            obj = obj && obj.id ? obj : this.get_node(obj);
            par = par && par.id ? par : this.get_node(par);
            var tmp = chk.match(/^move_node|copy_node|create_node$/i) ? par : obj,
                chc = this.settings.core.check_callback;
            if(chk === "move_node" || chk === "copy_node") {
                if((!more || !more.is_multi) && (obj.id === par.id || $.inArray(obj.id, par.children) === pos || $.inArray(par.id, obj.children_d) !== -1)) {
                    this._data.core.last_error = { 'error' : 'check', 'plugin' : 'core', 'id' : 'core_01', 'reason' : 'Moving parent inside child', 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                    return false;
                }
            }
            if(tmp && tmp.data) { tmp = tmp.data; }
            if(tmp && tmp.functions && (tmp.functions[chk] === false || tmp.functions[chk] === true)) {
                if(tmp.functions[chk] === false) {
                    this._data.core.last_error = { 'error' : 'check', 'plugin' : 'core', 'id' : 'core_02', 'reason' : 'Node data prevents function: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                }
                return tmp.functions[chk];
            }
            if(chc === false || ($.isFunction(chc) && chc.call(this, chk, obj, par, pos, more) === false) || (chc && chc[chk] === false)) {
                this._data.core.last_error = { 'error' : 'check', 'plugin' : 'core', 'id' : 'core_03', 'reason' : 'User config for core.check_callback prevents function: ' + chk, 'data' : JSON.stringify({ 'chk' : chk, 'pos' : pos, 'obj' : obj && obj.id ? obj.id : false, 'par' : par && par.id ? par.id : false }) };
                return false;
            }
            return true;
        },
        last_error : function () {
            return this._data.core.last_error;
        },
        move_node : function (obj, par, pos, callback, is_loaded, skip_redraw, origin) {
            var t1, t2, old_par, old_pos, new_par, old_ins, is_multi, dpc, tmp, i, j, k, l, p;

            par = this.get_node(par);
            pos = pos === undefined ? 0 : pos;
            if(!par) { return false; }
            if(!pos.toString().match(/^(before|after)$/) && !is_loaded && !this.is_loaded(par)) {
                return this.load_node(par, function () { this.move_node(obj, par, pos, callback, true, false, origin); });
            }

            if($.isArray(obj)) {
                if(obj.length === 1) {
                    obj = obj[0];
                }
                else {
                    for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                        if((tmp = this.move_node(obj[t1], par, pos, callback, is_loaded, false, origin))) {
                            par = tmp;
                            pos = "after";
                        }
                    }
                    this.redraw();
                    return true;
                }
            }
            obj = obj && obj.id ? obj : this.get_node(obj);

            if(!obj || obj.id === $.jstree.root) { return false; }

            old_par = (obj.parent || $.jstree.root).toString();
            new_par = (!pos.toString().match(/^(before|after)$/) || par.id === $.jstree.root) ? par : this.get_node(par.parent);
            old_ins = origin ? origin : (this._model.data[obj.id] ? this : $.jstree.reference(obj.id));
            is_multi = !old_ins || !old_ins._id || (this._id !== old_ins._id);
            old_pos = old_ins && old_ins._id && old_par && old_ins._model.data[old_par] && old_ins._model.data[old_par].children ? $.inArray(obj.id, old_ins._model.data[old_par].children) : -1;
            if(old_ins && old_ins._id) {
                obj = old_ins._model.data[obj.id];
            }

            if(is_multi) {
                if((tmp = this.copy_node(obj, par, pos, callback, is_loaded, false, origin))) {
                    if(old_ins) { old_ins.delete_node(obj); }
                    return tmp;
                }
                return false;
            }
            if(par.id === $.jstree.root) {
                if(pos === "before") { pos = "first"; }
                if(pos === "after") { pos = "last"; }
            }
            switch(pos) {
                case "before":
                    pos = $.inArray(par.id, new_par.children);
                    break;
                case "after" :
                    pos = $.inArray(par.id, new_par.children) + 1;
                    break;
                case "inside":
                case "first":
                    pos = 0;
                    break;
                case "last":
                    pos = new_par.children.length;
                    break;
                default:
                    if(!pos) { pos = 0; }
                    break;
            }
            if(pos > new_par.children.length) { pos = new_par.children.length; }
            if(!this.check("move_node", obj, new_par, pos, { 'core' : true, 'origin' : origin, 'is_multi' : (old_ins && old_ins._id && old_ins._id !== this._id), 'is_foreign' : (!old_ins || !old_ins._id) })) {
                this.settings.core.error.call(this, this._data.core.last_error);
                return false;
            }
            if(obj.parent === new_par.id) {
                dpc = new_par.children.concat();
                tmp = $.inArray(obj.id, dpc);
                if(tmp !== -1) {
                    dpc = $.vakata.array_remove(dpc, tmp);
                    if(pos > tmp) { pos--; }
                }
                tmp = [];
                for(i = 0, j = dpc.length; i < j; i++) {
                    tmp[i >= pos ? i+1 : i] = dpc[i];
                }
                tmp[pos] = obj.id;
                new_par.children = tmp;
                this._node_changed(new_par.id);
                this.redraw(new_par.id === $.jstree.root);
            }
            else {
                tmp = obj.children_d.concat();
                tmp.push(obj.id);
                for(i = 0, j = obj.parents.length; i < j; i++) {
                    dpc = [];
                    p = old_ins._model.data[obj.parents[i]].children_d;
                    for(k = 0, l = p.length; k < l; k++) {
                        if($.inArray(p[k], tmp) === -1) {
                            dpc.push(p[k]);
                        }
                    }
                    old_ins._model.data[obj.parents[i]].children_d = dpc;
                }
                old_ins._model.data[old_par].children = $.vakata.array_remove_item(old_ins._model.data[old_par].children, obj.id);

                for(i = 0, j = new_par.parents.length; i < j; i++) {
                    this._model.data[new_par.parents[i]].children_d = this._model.data[new_par.parents[i]].children_d.concat(tmp);
                }
                dpc = [];
                for(i = 0, j = new_par.children.length; i < j; i++) {
                    dpc[i >= pos ? i+1 : i] = new_par.children[i];
                }
                dpc[pos] = obj.id;
                new_par.children = dpc;
                new_par.children_d.push(obj.id);
                new_par.children_d = new_par.children_d.concat(obj.children_d);

                obj.parent = new_par.id;
                tmp = new_par.parents.concat();
                tmp.unshift(new_par.id);
                p = obj.parents.length;
                obj.parents = tmp;

                tmp = tmp.concat();
                for(i = 0, j = obj.children_d.length; i < j; i++) {
                    this._model.data[obj.children_d[i]].parents = this._model.data[obj.children_d[i]].parents.slice(0,p*-1);
                    Array.prototype.push.apply(this._model.data[obj.children_d[i]].parents, tmp);
                }

                if(old_par === $.jstree.root || new_par.id === $.jstree.root) {
                    this._model.force_full_redraw = true;
                }
                if(!this._model.force_full_redraw) {
                    this._node_changed(old_par);
                    this._node_changed(new_par.id);
                }
                if(!skip_redraw) {
                    this.redraw();
                }
            }
            if(callback) { callback.call(this, obj, new_par, pos); }
            this.trigger('move_node', { "node" : obj, "parent" : new_par.id, "position" : pos, "old_parent" : old_par, "old_position" : old_pos, 'is_multi' : (old_ins && old_ins._id && old_ins._id !== this._id), 'is_foreign' : (!old_ins || !old_ins._id), 'old_instance' : old_ins, 'new_instance' : this });
            return obj.id;
        },
        copy_node : function (obj, par, pos, callback, is_loaded, skip_redraw, origin) {
            var t1, t2, dpc, tmp, i, j, node, old_par, new_par, old_ins, is_multi;

            par = this.get_node(par);
            pos = pos === undefined ? 0 : pos;
            if(!par) { return false; }
            if(!pos.toString().match(/^(before|after)$/) && !is_loaded && !this.is_loaded(par)) {
                return this.load_node(par, function () { this.copy_node(obj, par, pos, callback, true, false, origin); });
            }

            if($.isArray(obj)) {
                if(obj.length === 1) {
                    obj = obj[0];
                }
                else {
                    for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                        if((tmp = this.copy_node(obj[t1], par, pos, callback, is_loaded, true, origin))) {
                            par = tmp;
                            pos = "after";
                        }
                    }
                    this.redraw();
                    return true;
                }
            }
            obj = obj && obj.id ? obj : this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }

            old_par = (obj.parent || $.jstree.root).toString();
            new_par = (!pos.toString().match(/^(before|after)$/) || par.id === $.jstree.root) ? par : this.get_node(par.parent);
            old_ins = origin ? origin : (this._model.data[obj.id] ? this : $.jstree.reference(obj.id));
            is_multi = !old_ins || !old_ins._id || (this._id !== old_ins._id);

            if(old_ins && old_ins._id) {
                obj = old_ins._model.data[obj.id];
            }

            if(par.id === $.jstree.root) {
                if(pos === "before") { pos = "first"; }
                if(pos === "after") { pos = "last"; }
            }
            switch(pos) {
                case "before":
                    pos = $.inArray(par.id, new_par.children);
                    break;
                case "after" :
                    pos = $.inArray(par.id, new_par.children) + 1;
                    break;
                case "inside":
                case "first":
                    pos = 0;
                    break;
                case "last":
                    pos = new_par.children.length;
                    break;
                default:
                    if(!pos) { pos = 0; }
                    break;
            }
            if(pos > new_par.children.length) { pos = new_par.children.length; }
            if(!this.check("copy_node", obj, new_par, pos, { 'core' : true, 'origin' : origin, 'is_multi' : (old_ins && old_ins._id && old_ins._id !== this._id), 'is_foreign' : (!old_ins || !old_ins._id) })) {
                this.settings.core.error.call(this, this._data.core.last_error);
                return false;
            }
            node = old_ins ? old_ins.get_json(obj, { no_id : true, no_data : true, no_state : true }) : obj;
            if(!node) { return false; }
            if(node.id === true) { delete node.id; }
            node = this._parse_model_from_json(node, new_par.id, new_par.parents.concat());
            if(!node) { return false; }
            tmp = this.get_node(node);
            if(obj && obj.state && obj.state.loaded === false) { tmp.state.loaded = false; }
            dpc = [];
            dpc.push(node);
            dpc = dpc.concat(tmp.children_d);
            this.trigger('model', { "nodes" : dpc, "parent" : new_par.id });

            for(i = 0, j = new_par.parents.length; i < j; i++) {
                this._model.data[new_par.parents[i]].children_d = this._model.data[new_par.parents[i]].children_d.concat(dpc);
            }
            dpc = [];
            for(i = 0, j = new_par.children.length; i < j; i++) {
                dpc[i >= pos ? i+1 : i] = new_par.children[i];
            }
            dpc[pos] = tmp.id;
            new_par.children = dpc;
            new_par.children_d.push(tmp.id);
            new_par.children_d = new_par.children_d.concat(tmp.children_d);

            if(new_par.id === $.jstree.root) {
                this._model.force_full_redraw = true;
            }
            if(!this._model.force_full_redraw) {
                this._node_changed(new_par.id);
            }
            if(!skip_redraw) {
                this.redraw(new_par.id === $.jstree.root);
            }
            if(callback) { callback.call(this, tmp, new_par, pos); }
            this.trigger('copy_node', { "node" : tmp, "original" : obj, "parent" : new_par.id, "position" : pos, "old_parent" : old_par, "old_position" : old_ins && old_ins._id && old_par && old_ins._model.data[old_par] && old_ins._model.data[old_par].children ? $.inArray(obj.id, old_ins._model.data[old_par].children) : -1,'is_multi' : (old_ins && old_ins._id && old_ins._id !== this._id), 'is_foreign' : (!old_ins || !old_ins._id), 'old_instance' : old_ins, 'new_instance' : this });
            return tmp.id;
        },
        cut : function (obj) {
            if(!obj) { obj = this._data.core.selected.concat(); }
            if(!$.isArray(obj)) { obj = [obj]; }
            if(!obj.length) { return false; }
            var tmp = [], o, t1, t2;
            for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                o = this.get_node(obj[t1]);
                if(o && o.id && o.id !== $.jstree.root) { tmp.push(o); }
            }
            if(!tmp.length) { return false; }
            ccp_node = tmp;
            ccp_inst = this;
            ccp_mode = 'move_node';
            this.trigger('cut', { "node" : obj });
        },
        copy : function (obj) {
            if(!obj) { obj = this._data.core.selected.concat(); }
            if(!$.isArray(obj)) { obj = [obj]; }
            if(!obj.length) { return false; }
            var tmp = [], o, t1, t2;
            for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                o = this.get_node(obj[t1]);
                if(o && o.id && o.id !== $.jstree.root) { tmp.push(o); }
            }
            if(!tmp.length) { return false; }
            ccp_node = tmp;
            ccp_inst = this;
            ccp_mode = 'copy_node';
            this.trigger('copy', { "node" : obj });
        },
        get_buffer : function () {
            return { 'mode' : ccp_mode, 'node' : ccp_node, 'inst' : ccp_inst };
        },
        can_paste : function () {
            return ccp_mode !== false && ccp_node !== false;
        },
        paste : function (obj, pos) {
            obj = this.get_node(obj);
            if(!obj || !ccp_mode || !ccp_mode.match(/^(copy_node|move_node)$/) || !ccp_node) { return false; }
            if(this[ccp_mode](ccp_node, obj, pos, false, false, false, ccp_inst)) {
                this.trigger('paste', { "parent" : obj.id, "node" : ccp_node, "mode" : ccp_mode });
            }
            ccp_node = false;
            ccp_mode = false;
            ccp_inst = false;
        },
        clear_buffer : function () {
            ccp_node = false;
            ccp_mode = false;
            ccp_inst = false;
            this.trigger('clear_buffer');
        },
        edit : function (obj, default_text, callback) {
            var rtl, w, a, s, t, h1, h2, fn, tmp, cancel = false;
            obj = this.get_node(obj);
            if(!obj) { return false; }
            if(this.settings.core.check_callback === false) {
                this._data.core.last_error = { 'error' : 'check', 'plugin' : 'core', 'id' : 'core_07', 'reason' : 'Could not edit node because of check_callback' };
                this.settings.core.error.call(this, this._data.core.last_error);
                return false;
            }
            tmp = obj;
            default_text = typeof default_text === 'string' ? default_text : obj.text;
            this.set_text(obj, "");
            obj = this._open_to(obj);
            tmp.text = default_text;

            rtl = this._data.core.rtl;
            w  = this.element.width();
            this._data.core.focused = tmp.id;
            a  = obj.children('.jstree-anchor').focus();
            s  = $('<span>');
            t  = default_text;
            h1 = $("<"+"div />", { css : { "position" : "absolute", "top" : "-200px", "left" : (rtl ? "0px" : "-1000px"), "visibility" : "hidden" } }).appendTo("body");
            h2 = $("<"+"input />", {
                        "value" : t,
                        "class" : "jstree-rename-input",
                        "css" : {
                            "padding" : "0",
                            "border" : "1px solid silver",
                            "box-sizing" : "border-box",
                            "display" : "inline-block",
                            "height" : (this._data.core.li_height) + "px",
                            "lineHeight" : (this._data.core.li_height) + "px",
                            "width" : "150px"
                        },
                        "blur" : $.proxy(function (e) {
                            e.stopImmediatePropagation();
                            e.preventDefault();
                            var i = s.children(".jstree-rename-input"),
                                v = i.val(),
                                f = this.settings.core.force_text,
                                nv;
                            if(v === "") { v = t; }
                            h1.remove();
                            s.replaceWith(a);
                            s.remove();
                            t = f ? t : $('<div></div>').append($.parseHTML(t)).html();
                            this.set_text(obj, t);
                            nv = !!this.rename_node(obj, f ? $('<div></div>').text(v).text() : $('<div></div>').append($.parseHTML(v)).html());
                            if(!nv) {
                                this.set_text(obj, t);
                            }
                            this._data.core.focused = tmp.id;
                            setTimeout($.proxy(function () {
                                var node = this.get_node(tmp.id, true);
                                if(node.length) {
                                    this._data.core.focused = tmp.id;
                                    node.children('.jstree-anchor').focus();
                                }
                            }, this), 0);
                            if(callback) {
                                callback.call(this, tmp, nv, cancel);
                            }
                        }, this),
                        "keydown" : function (e) {
                            var key = e.which;
                            if(key === 27) {
                                cancel = true;
                                this.value = t;
                            }
                            if(key === 27 || key === 13 || key === 37 || key === 38 || key === 39 || key === 40 || key === 32) {
                                e.stopImmediatePropagation();
                            }
                            if(key === 27 || key === 13) {
                                e.preventDefault();
                                this.blur();
                            }
                        },
                        "click" : function (e) { e.stopImmediatePropagation(); },
                        "mousedown" : function (e) { e.stopImmediatePropagation(); },
                        "keyup" : function (e) {
                            h2.width(Math.min(h1.text("pW" + this.value).width(),w));
                        },
                        "keypress" : function(e) {
                            if(e.which === 13) { return false; }
                        }
                    });
                fn = {
                        fontFamily        : a.css('fontFamily')        || '',
                        fontSize        : a.css('fontSize')            || '',
                        fontWeight        : a.css('fontWeight')        || '',
                        fontStyle        : a.css('fontStyle')        || '',
                        fontStretch        : a.css('fontStretch')        || '',
                        fontVariant        : a.css('fontVariant')        || '',
                        letterSpacing    : a.css('letterSpacing')    || '',
                        wordSpacing        : a.css('wordSpacing')        || ''
                };
            s.attr('class', a.attr('class')).append(a.contents().clone()).append(h2);
            a.replaceWith(s);
            h1.css(fn);
            h2.css(fn).width(Math.min(h1.text("pW" + h2[0].value).width(),w))[0].select();
        },


        set_theme : function (theme_name, theme_url) {
            if(!theme_name) { return false; }
            if(theme_url === true) {
                var dir = this.settings.core.themes.dir;
                if(!dir) { dir = $.jstree.path + '/themes'; }
                theme_url = dir + '/' + theme_name + '/style.css';
            }
            if(theme_url && $.inArray(theme_url, themes_loaded) === -1) {
                $('head').append('<'+'link rel="stylesheet" href="' + theme_url + '" type="text/css" />');
                themes_loaded.push(theme_url);
            }
            if(this._data.core.themes.name) {
                this.element.removeClass('jstree-' + this._data.core.themes.name);
            }
            this._data.core.themes.name = theme_name;
            this.element.addClass('jstree-' + theme_name);
            this.element[this.settings.core.themes.responsive ? 'addClass' : 'removeClass' ]('jstree-' + theme_name + '-responsive');
            this.trigger('set_theme', { 'theme' : theme_name });
        },
        get_theme : function () { return this._data.core.themes.name; },
        set_theme_variant : function (variant_name) {
            if(this._data.core.themes.variant) {
                this.element.removeClass('jstree-' + this._data.core.themes.name + '-' + this._data.core.themes.variant);
            }
            this._data.core.themes.variant = variant_name;
            if(variant_name) {
                this.element.addClass('jstree-' + this._data.core.themes.name + '-' + this._data.core.themes.variant);
            }
        },
        get_theme_variant : function () { return this._data.core.themes.variant; },
        show_stripes : function () { this._data.core.themes.stripes = true; this.get_container_ul().addClass("jstree-striped"); },
        hide_stripes : function () { this._data.core.themes.stripes = false; this.get_container_ul().removeClass("jstree-striped"); },
        toggle_stripes : function () { if(this._data.core.themes.stripes) { this.hide_stripes(); } else { this.show_stripes(); } },
        show_dots : function () { this._data.core.themes.dots = true; this.get_container_ul().removeClass("jstree-no-dots"); },
        hide_dots : function () { this._data.core.themes.dots = false; this.get_container_ul().addClass("jstree-no-dots"); },
        toggle_dots : function () { if(this._data.core.themes.dots) { this.hide_dots(); } else { this.show_dots(); } },
        show_icons : function () { this._data.core.themes.icons = true; this.get_container_ul().removeClass("jstree-no-icons"); },
        hide_icons : function () { this._data.core.themes.icons = false; this.get_container_ul().addClass("jstree-no-icons"); },
        toggle_icons : function () { if(this._data.core.themes.icons) { this.hide_icons(); } else { this.show_icons(); } },
        set_icon : function (obj, icon) {
            var t1, t2, dom, old;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.set_icon(obj[t1], icon);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            old = obj.icon;
            obj.icon = icon === true || icon === null || icon === undefined || icon === '' ? true : icon;
            dom = this.get_node(obj, true).children(".jstree-anchor").children(".jstree-themeicon");
            if(icon === false) {
                this.hide_icon(obj);
            }
            else if(icon === true || icon === null || icon === undefined || icon === '') {
                dom.removeClass('jstree-themeicon-custom ' + old).css("background","").removeAttr("rel");
                if(old === false) { this.show_icon(obj); }
            }
            else if(icon.indexOf("/") === -1 && icon.indexOf(".") === -1) {
                dom.removeClass(old).css("background","");
                dom.addClass(icon + ' jstree-themeicon-custom').attr("rel",icon);
                if(old === false) { this.show_icon(obj); }
            }
            else {
                dom.removeClass(old).css("background","");
                dom.addClass('jstree-themeicon-custom').css("background", "url('" + icon + "') center center no-repeat").attr("rel",icon);
                if(old === false) { this.show_icon(obj); }
            }
            return true;
        },
        get_icon : function (obj) {
            obj = this.get_node(obj);
            return (!obj || obj.id === $.jstree.root) ? false : obj.icon;
        },
        hide_icon : function (obj) {
            var t1, t2;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.hide_icon(obj[t1]);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj === $.jstree.root) { return false; }
            obj.icon = false;
            this.get_node(obj, true).children(".jstree-anchor").children(".jstree-themeicon").addClass('jstree-themeicon-hidden');
            return true;
        },
        show_icon : function (obj) {
            var t1, t2, dom;
            if($.isArray(obj)) {
                obj = obj.slice();
                for(t1 = 0, t2 = obj.length; t1 < t2; t1++) {
                    this.show_icon(obj[t1]);
                }
                return true;
            }
            obj = this.get_node(obj);
            if(!obj || obj === $.jstree.root) { return false; }
            dom = this.get_node(obj, true);
            obj.icon = dom.length ? dom.children(".jstree-anchor").children(".jstree-themeicon").attr('rel') : true;
            if(!obj.icon) { obj.icon = true; }
            dom.children(".jstree-anchor").children(".jstree-themeicon").removeClass('jstree-themeicon-hidden');
            return true;
        }
    };

    $.vakata = {};
    $.vakata.attributes = function(node, with_values) {
        node = $(node)[0];
        var attr = with_values ? {} : [];
        if(node && node.attributes) {
            $.each(node.attributes, function (i, v) {
                if($.inArray(v.name.toLowerCase(),['style','contenteditable','hasfocus','tabindex']) !== -1) { return; }
                if(v.value !== null && $.trim(v.value) !== '') {
                    if(with_values) { attr[v.name] = v.value; }
                    else { attr.push(v.name); }
                }
            });
        }
        return attr;
    };
    $.vakata.array_unique = function(array) {
        var a = [], i, j, l, o = {};
        for(i = 0, l = array.length; i < l; i++) {
            if(o[array[i]] === undefined) {
                a.push(array[i]);
                o[array[i]] = true;
            }
        }
        return a;
    };
    $.vakata.array_remove = function(array, from, to) {
        var rest = array.slice((to || from) + 1 || array.length);
        array.length = from < 0 ? array.length + from : from;
        array.push.apply(array, rest);
        return array;
    };
    $.vakata.array_remove_item = function(array, item) {
        var tmp = $.inArray(item, array);
        return tmp !== -1 ? $.vakata.array_remove(array, tmp) : array;
    };
}));
