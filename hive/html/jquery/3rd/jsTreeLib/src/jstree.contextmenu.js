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
        define('jstree.contextmenu', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.contextmenu) { return; }

    $.jstree.defaults.contextmenu = {
        select_node : true,
        show_at_node : true,
        items : function (o, cb) {
            return {
                "create" : {
                    "separator_before"    : false,
                    "separator_after"    : true,
                    "_disabled"            : false,
                    "label"                : "Create",
                    "action"            : function (data) {
                        var inst = $.jstree.reference(data.reference),
                            obj = inst.get_node(data.reference);
                        inst.create_node(obj, {}, "last", function (new_node) {
                            setTimeout(function () { inst.edit(new_node); },0);
                        });
                    }
                },
                "rename" : {
                    "separator_before"    : false,
                    "separator_after"    : false,
                    "_disabled"            : false,
                    "label"                : "Rename",
                    "action"            : function (data) {
                        var inst = $.jstree.reference(data.reference),
                            obj = inst.get_node(data.reference);
                        inst.edit(obj);
                    }
                },
                "remove" : {
                    "separator_before"    : false,
                    "icon"                : false,
                    "separator_after"    : false,
                    "_disabled"            : false,
                    "label"                : "Delete",
                    "action"            : function (data) {
                        var inst = $.jstree.reference(data.reference),
                            obj = inst.get_node(data.reference);
                        if(inst.is_selected(obj)) {
                            inst.delete_node(inst.get_selected());
                        }
                        else {
                            inst.delete_node(obj);
                        }
                    }
                },
                "ccp" : {
                    "separator_before"    : true,
                    "icon"                : false,
                    "separator_after"    : false,
                    "label"                : "Edit",
                    "action"            : false,
                    "submenu" : {
                        "cut" : {
                            "separator_before"    : false,
                            "separator_after"    : false,
                            "label"                : "Cut",
                            "action"            : function (data) {
                                var inst = $.jstree.reference(data.reference),
                                    obj = inst.get_node(data.reference);
                                if(inst.is_selected(obj)) {
                                    inst.cut(inst.get_top_selected());
                                }
                                else {
                                    inst.cut(obj);
                                }
                            }
                        },
                        "copy" : {
                            "separator_before"    : false,
                            "icon"                : false,
                            "separator_after"    : false,
                            "label"                : "Copy",
                            "action"            : function (data) {
                                var inst = $.jstree.reference(data.reference),
                                    obj = inst.get_node(data.reference);
                                if(inst.is_selected(obj)) {
                                    inst.copy(inst.get_top_selected());
                                }
                                else {
                                    inst.copy(obj);
                                }
                            }
                        },
                        "paste" : {
                            "separator_before"    : false,
                            "icon"                : false,
                            "_disabled"            : function (data) {
                                return !$.jstree.reference(data.reference).can_paste();
                            },
                            "separator_after"    : false,
                            "label"                : "Paste",
                            "action"            : function (data) {
                                var inst = $.jstree.reference(data.reference),
                                    obj = inst.get_node(data.reference);
                                inst.paste(obj);
                            }
                        }
                    }
                }
            };
        }
    };

    $.jstree.plugins.contextmenu = function (options, parent) {
        this.bind = function () {
            parent.bind.call(this);

            var last_ts = 0, cto = null, ex, ey;
            this.element
                .on("contextmenu.jstree", ".jstree-anchor", $.proxy(function (e, data) {
                        e.preventDefault();
                        last_ts = e.ctrlKey ? +new Date() : 0;
                        if(data || cto) {
                            last_ts = (+new Date()) + 10000;
                        }
                        if(cto) {
                            clearTimeout(cto);
                        }
                        if(!this.is_loading(e.currentTarget)) {
                            this.show_contextmenu(e.currentTarget, e.pageX, e.pageY, e);
                        }
                    }, this))
                .on("click.jstree", ".jstree-anchor", $.proxy(function (e) {
                        if(this._data.contextmenu.visible && (!last_ts || (+new Date()) - last_ts > 250)) {
                            $.vakata.context.hide();
                        }
                        last_ts = 0;
                    }, this))
                .on("touchstart.jstree", ".jstree-anchor", function (e) {
                        if(!e.originalEvent || !e.originalEvent.changedTouches || !e.originalEvent.changedTouches[0]) {
                            return;
                        }
                        ex = e.pageX;
                        ey = e.pageY;
                        cto = setTimeout(function () {
                            $(e.currentTarget).trigger('contextmenu', true);
                        }, 750);
                    })
                .on('touchmove.vakata.jstree', function (e) {
                        if(cto && e.originalEvent && e.originalEvent.changedTouches && e.originalEvent.changedTouches[0] && (Math.abs(ex - e.pageX) > 50 || Math.abs(ey - e.pageY) > 50)) {
                            clearTimeout(cto);
                        }
                    })
                .on('touchend.vakata.jstree', function (e) {
                        if(cto) {
                            clearTimeout(cto);
                        }
                    });

            $(document).on("context_hide.vakata.jstree", $.proxy(function () { this._data.contextmenu.visible = false; }, this));
        };
        this.teardown = function () {
            if(this._data.contextmenu.visible) {
                $.vakata.context.hide();
            }
            parent.teardown.call(this);
        };

        this.show_contextmenu = function (obj, x, y, e) {
            obj = this.get_node(obj);
            if(!obj || obj.id === $.jstree.root) { return false; }
            var s = this.settings.contextmenu,
                d = this.get_node(obj, true),
                a = d.children(".jstree-anchor"),
                o = false,
                i = false;
            if(s.show_at_node || x === undefined || y === undefined) {
                o = a.offset();
                x = o.left;
                y = o.top + this._data.core.li_height;
            }
            if(this.settings.contextmenu.select_node && !this.is_selected(obj)) {
                this.activate_node(obj, e);
            }

            i = s.items;
            if($.isFunction(i)) {
                i = i.call(this, obj, $.proxy(function (i) {
                    this._show_contextmenu(obj, x, y, i);
                }, this));
            }
            if($.isPlainObject(i)) {
                this._show_contextmenu(obj, x, y, i);
            }
        };
        this._show_contextmenu = function (obj, x, y, i) {
            var d = this.get_node(obj, true),
                a = d.children(".jstree-anchor");
            $(document).one("context_show.vakata.jstree", $.proxy(function (e, data) {
                var cls = 'jstree-contextmenu jstree-' + this.get_theme() + '-contextmenu';
                $(data.element).addClass(cls);
            }, this));
            this._data.contextmenu.visible = true;
            $.vakata.context.show(a, { 'x' : x, 'y' : y }, i);
            this.trigger('show_contextmenu', { "node" : obj, "x" : x, "y" : y });
        };
    };

    (function ($) {
        var right_to_left = false,
            vakata_context = {
                element        : false,
                reference    : false,
                position_x    : 0,
                position_y    : 0,
                items        : [],
                html        : "",
                is_visible    : false
            };

        $.vakata.context = {
            settings : {
                hide_onmouseleave    : 0,
                icons                : true
            },
            _trigger : function (event_name) {
                $(document).triggerHandler("context_" + event_name + ".vakata", {
                    "reference"    : vakata_context.reference,
                    "element"    : vakata_context.element,
                    "position"    : {
                        "x" : vakata_context.position_x,
                        "y" : vakata_context.position_y
                    }
                });
            },
            _execute : function (i) {
                i = vakata_context.items[i];
                return i && (!i._disabled || ($.isFunction(i._disabled) && !i._disabled({ "item" : i, "reference" : vakata_context.reference, "element" : vakata_context.element }))) && i.action ? i.action.call(null, {
                            "item"        : i,
                            "reference"    : vakata_context.reference,
                            "element"    : vakata_context.element,
                            "position"    : {
                                "x" : vakata_context.position_x,
                                "y" : vakata_context.position_y
                            }
                        }) : false;
            },
            _parse : function (o, is_callback) {
                if(!o) { return false; }
                if(!is_callback) {
                    vakata_context.html        = "";
                    vakata_context.items    = [];
                }
                var str = "",
                    sep = false,
                    tmp;

                if(is_callback) { str += "<"+"ul>"; }
                $.each(o, function (i, val) {
                    if(!val) { return true; }
                    vakata_context.items.push(val);
                    if(!sep && val.separator_before) {
                        str += "<"+"li class='vakata-context-separator'><"+"a href='#' " + ($.vakata.context.settings.icons ? '' : 'style="margin-left:0px;"') + ">&#160;<"+"/a><"+"/li>";
                    }
                    sep = false;
                    str += "<"+"li class='" + (val._class || "") + (val._disabled === true || ($.isFunction(val._disabled) && val._disabled({ "item" : val, "reference" : vakata_context.reference, "element" : vakata_context.element })) ? " vakata-contextmenu-disabled " : "") + "' "+(val.shortcut?" data-shortcut='"+val.shortcut+"' ":'')+">";
                    str += "<"+"a href='#' rel='" + (vakata_context.items.length - 1) + "'>";
                    if($.vakata.context.settings.icons) {
                        str += "<"+"i ";
                        if(val.icon) {
                            if(val.icon.indexOf("/") !== -1 || val.icon.indexOf(".") !== -1) { str += " style='background:url(\"" + val.icon + "\") center center no-repeat' "; }
                            else { str += " class='" + val.icon + "' "; }
                        }
                        str += "><"+"/i><"+"span class='vakata-contextmenu-sep'>&#160;<"+"/span>";
                    }
                    str += ($.isFunction(val.label) ? val.label({ "item" : i, "reference" : vakata_context.reference, "element" : vakata_context.element }) : val.label) + (val.shortcut?' <span class="vakata-contextmenu-shortcut vakata-contextmenu-shortcut-'+val.shortcut+'">'+ (val.shortcut_label || '') +'</span>':'') + "<"+"/a>";
                    if(val.submenu) {
                        tmp = $.vakata.context._parse(val.submenu, true);
                        if(tmp) { str += tmp; }
                    }
                    str += "<"+"/li>";
                    if(val.separator_after) {
                        str += "<"+"li class='vakata-context-separator'><"+"a href='#' " + ($.vakata.context.settings.icons ? '' : 'style="margin-left:0px;"') + ">&#160;<"+"/a><"+"/li>";
                        sep = true;
                    }
                });
                str  = str.replace(/<li class\='vakata-context-separator'\><\/li\>$/,"");
                if(is_callback) { str += "</ul>"; }
                if(!is_callback) { vakata_context.html = str; $.vakata.context._trigger("parse"); }
                return str.length > 10 ? str : false;
            },
            _show_submenu : function (o) {
                o = $(o);
                if(!o.length || !o.children("ul").length) { return; }
                var e = o.children("ul"),
                    x = o.offset().left + o.outerWidth(),
                    y = o.offset().top,
                    w = e.width(),
                    h = e.height(),
                    dw = $(window).width() + $(window).scrollLeft(),
                    dh = $(window).height() + $(window).scrollTop();
                if(right_to_left) {
                    o[x - (w + 10 + o.outerWidth()) < 0 ? "addClass" : "removeClass"]("vakata-context-left");
                }
                else {
                    o[x + w + 10 > dw ? "addClass" : "removeClass"]("vakata-context-right");
                }
                if(y + h + 10 > dh) {
                    e.css("bottom","-1px");
                }
                e.show();
            },
            show : function (reference, position, data) {
                var o, e, x, y, w, h, dw, dh, cond = true;
                if(vakata_context.element && vakata_context.element.length) {
                    vakata_context.element.width('');
                }
                switch(cond) {
                    case (!position && !reference):
                        return false;
                    case (!!position && !!reference):
                        vakata_context.reference    = reference;
                        vakata_context.position_x    = position.x;
                        vakata_context.position_y    = position.y;
                        break;
                    case (!position && !!reference):
                        vakata_context.reference    = reference;
                        o = reference.offset();
                        vakata_context.position_x    = o.left + reference.outerHeight();
                        vakata_context.position_y    = o.top;
                        break;
                    case (!!position && !reference):
                        vakata_context.position_x    = position.x;
                        vakata_context.position_y    = position.y;
                        break;
                }
                if(!!reference && !data && $(reference).data('vakata_contextmenu')) {
                    data = $(reference).data('vakata_contextmenu');
                }
                if($.vakata.context._parse(data)) {
                    vakata_context.element.html(vakata_context.html);
                }
                if(vakata_context.items.length) {
                    vakata_context.element.appendTo("body");
                    e = vakata_context.element;
                    x = vakata_context.position_x;
                    y = vakata_context.position_y;
                    w = e.width();
                    h = e.height();
                    dw = $(window).width() + $(window).scrollLeft();
                    dh = $(window).height() + $(window).scrollTop();
                    if(right_to_left) {
                        x -= (e.outerWidth() - $(reference).outerWidth());
                        if(x < $(window).scrollLeft() + 20) {
                            x = $(window).scrollLeft() + 20;
                        }
                    }
                    if(x + w + 20 > dw) {
                        x = dw - (w + 20);
                    }
                    if(y + h + 20 > dh) {
                        y = dh - (h + 20);
                    }

                    vakata_context.element
                        .css({ "left" : x, "top" : y })
                        .show()
                        .find('a').first().focus().parent().addClass("vakata-context-hover");
                    vakata_context.is_visible = true;
                    $.vakata.context._trigger("show");
                }
            },
            hide : function () {
                if(vakata_context.is_visible) {
                    vakata_context.element.hide().find("ul").hide().end().find(':focus').blur().end().detach();
                    vakata_context.is_visible = false;
                    $.vakata.context._trigger("hide");
                }
            }
        };
        $(function () {
            right_to_left = $("body").css("direction") === "rtl";
            var to = false;

            vakata_context.element = $("<ul class='vakata-context'></ul>");
            vakata_context.element
                .on("mouseenter", "li", function (e) {
                    e.stopImmediatePropagation();

                    if($.contains(this, e.relatedTarget)) {
                        return;
                    }

                    if(to) { clearTimeout(to); }
                    vakata_context.element.find(".vakata-context-hover").removeClass("vakata-context-hover").end();

                    $(this)
                        .siblings().find("ul").hide().end().end()
                        .parentsUntil(".vakata-context", "li").addBack().addClass("vakata-context-hover");
                    $.vakata.context._show_submenu(this);
                })
                .on("mouseleave", "li", function (e) {
                    if($.contains(this, e.relatedTarget)) { return; }
                    $(this).find(".vakata-context-hover").addBack().removeClass("vakata-context-hover");
                })
                .on("mouseleave", function (e) {
                    $(this).find(".vakata-context-hover").removeClass("vakata-context-hover");
                    if($.vakata.context.settings.hide_onmouseleave) {
                        to = setTimeout(
                            (function (t) {
                                return function () { $.vakata.context.hide(); };
                            }(this)), $.vakata.context.settings.hide_onmouseleave);
                    }
                })
                .on("click", "a", function (e) {
                    e.preventDefault();
                    if(!$(this).blur().parent().hasClass("vakata-context-disabled") && $.vakata.context._execute($(this).attr("rel")) !== false) {
                        $.vakata.context.hide();
                    }
                })
                .on('keydown', 'a', function (e) {
                        var o = null;
                        switch(e.which) {
                            case 13:
                            case 32:
                                e.type = "mouseup";
                                e.preventDefault();
                                $(e.currentTarget).trigger(e);
                                break;
                            case 37:
                                if(vakata_context.is_visible) {
                                    vakata_context.element.find(".vakata-context-hover").last().closest("li").first().find("ul").hide().find(".vakata-context-hover").removeClass("vakata-context-hover").end().end().children('a').focus();
                                    e.stopImmediatePropagation();
                                    e.preventDefault();
                                }
                                break;
                            case 38:
                                if(vakata_context.is_visible) {
                                    o = vakata_context.element.find("ul:visible").addBack().last().children(".vakata-context-hover").removeClass("vakata-context-hover").prevAll("li:not(.vakata-context-separator)").first();
                                    if(!o.length) { o = vakata_context.element.find("ul:visible").addBack().last().children("li:not(.vakata-context-separator)").last(); }
                                    o.addClass("vakata-context-hover").children('a').focus();
                                    e.stopImmediatePropagation();
                                    e.preventDefault();
                                }
                                break;
                            case 39:
                                if(vakata_context.is_visible) {
                                    vakata_context.element.find(".vakata-context-hover").last().children("ul").show().children("li:not(.vakata-context-separator)").removeClass("vakata-context-hover").first().addClass("vakata-context-hover").children('a').focus();
                                    e.stopImmediatePropagation();
                                    e.preventDefault();
                                }
                                break;
                            case 40:
                                if(vakata_context.is_visible) {
                                    o = vakata_context.element.find("ul:visible").addBack().last().children(".vakata-context-hover").removeClass("vakata-context-hover").nextAll("li:not(.vakata-context-separator)").first();
                                    if(!o.length) { o = vakata_context.element.find("ul:visible").addBack().last().children("li:not(.vakata-context-separator)").first(); }
                                    o.addClass("vakata-context-hover").children('a').focus();
                                    e.stopImmediatePropagation();
                                    e.preventDefault();
                                }
                                break;
                            case 27:
                                $.vakata.context.hide();
                                e.preventDefault();
                                break;
                            default:
                                break;
                        }
                    })
                .on('keydown', function (e) {
                    e.preventDefault();
                    var a = vakata_context.element.find('.vakata-contextmenu-shortcut-' + e.which).parent();
                    if(a.parent().not('.vakata-context-disabled')) {
                        a.click();
                    }
                });

            $(document)
                .on("mousedown.vakata.jstree", function (e) {
                    if(vakata_context.is_visible && !$.contains(vakata_context.element[0], e.target)) {
                        $.vakata.context.hide();
                    }
                })
                .on("context_show.vakata.jstree", function (e, data) {
                    vakata_context.element.find("li:has(ul)").children("a").addClass("vakata-context-parent");
                    if(right_to_left) {
                        vakata_context.element.addClass("vakata-context-rtl").css("direction", "rtl");
                    }
                    vakata_context.element.find("ul").hide().end();
                });
        });
    }($));
}));