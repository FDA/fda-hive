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
        define('jstree.dnd', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.dnd) { return; }

    $.jstree.defaults.dnd = {
        copy : true,
        open_timeout : 500,
        is_draggable : true,
        check_while_dragging : true,
        always_copy : false,
        inside_pos : 0,
        drag_selection : true,
        touch : true,
        large_drop_target : false,
        large_drag_target : false
    };
    $.jstree.plugins.dnd = function (options, parent) {
        this.bind = function () {
            parent.bind.call(this);

            this.element
                .on('mousedown.jstree touchstart.jstree', this.settings.dnd.large_drag_target ? '.jstree-node' : '.jstree-anchor', $.proxy(function (e) {
                    if(this.settings.dnd.large_drag_target && $(e.target).closest('.jstree-node')[0] !== e.currentTarget) {
                        return true;
                    }
                    if(e.type === "touchstart" && (!this.settings.dnd.touch || (this.settings.dnd.touch === 'selected' && !$(e.currentTarget).closest('.jstree-node').children('.jstree-anchor').hasClass('jstree-clicked')))) {
                        return true;
                    }
                    var obj = this.get_node(e.target),
                        mlt = this.is_selected(obj) && this.settings.dnd.drag_selection ? this.get_top_selected().length : 1,
                        txt = (mlt > 1 ? mlt + ' ' + this.get_string('nodes') : this.get_text(e.currentTarget));
                    if(this.settings.core.force_text) {
                        txt = $.vakata.html.escape(txt);
                    }
                    if(obj && obj.id && obj.id !== $.jstree.root && (e.which === 1 || e.type === "touchstart") &&
                        (this.settings.dnd.is_draggable === true || ($.isFunction(this.settings.dnd.is_draggable) && this.settings.dnd.is_draggable.call(this, (mlt > 1 ? this.get_top_selected(true) : [obj]), e)))
                    ) {
                        this.element.trigger('mousedown.jstree');
                        return $.vakata.dnd.start(e, { 'jstree' : true, 'origin' : this, 'obj' : this.get_node(obj,true), 'nodes' : mlt > 1 ? this.get_top_selected() : [obj.id] }, '<div id="jstree-dnd" class="jstree-' + this.get_theme() + ' jstree-' + this.get_theme() + '-' + this.get_theme_variant() + ' ' + ( this.settings.core.themes.responsive ? ' jstree-dnd-responsive' : '' ) + '"><i class="jstree-icon jstree-er"></i>' + txt + '<ins class="jstree-copy" style="display:none;">+</ins></div>');
                    }
                }, this));
        };
    };

    $(function() {
        var lastmv = false,
            laster = false,
            lastev = false,
            opento = false,
            marker = $('<div id="jstree-marker">&#160;</div>').hide();

        $(document)
            .on('dnd_start.vakata.jstree', function (e, data) {
                lastmv = false;
                lastev = false;
                if(!data || !data.data || !data.data.jstree) { return; }
                marker.appendTo('body');
            })
            .on('dnd_move.vakata.jstree', function (e, data) {
                if(opento) { clearTimeout(opento); }
                if(!data || !data.data || !data.data.jstree) { return; }

                if(data.event.target.id && data.event.target.id === 'jstree-marker') {
                    return;
                }
                lastev = data.event;

                var ins = $.jstree.reference(data.event.target),
                    ref = false,
                    off = false,
                    rel = false,
                    tmp, l, t, h, p, i, o, ok, t1, t2, op, ps, pr, ip, tm;
                if(ins && ins._data && ins._data.dnd) {
                    marker.attr('class', 'jstree-' + ins.get_theme() + ( ins.settings.core.themes.responsive ? ' jstree-dnd-responsive' : '' ));
                    data.helper
                        .children().attr('class', 'jstree-' + ins.get_theme() + ' jstree-' + ins.get_theme() + '-' + ins.get_theme_variant() + ' ' + ( ins.settings.core.themes.responsive ? ' jstree-dnd-responsive' : '' ))
                        .find('.jstree-copy').first()[ data.data.origin && (data.data.origin.settings.dnd.always_copy || (data.data.origin.settings.dnd.copy && (data.event.metaKey || data.event.ctrlKey))) ? 'show' : 'hide' ]();


                    if( (data.event.target === ins.element[0] || data.event.target === ins.get_container_ul()[0]) && ins.get_container_ul().children().length === 0) {
                        ok = true;
                        for(t1 = 0, t2 = data.data.nodes.length; t1 < t2; t1++) {
                            ok = ok && ins.check( (data.data.origin && (data.data.origin.settings.dnd.always_copy || (data.data.origin.settings.dnd.copy && (data.event.metaKey || data.event.ctrlKey)) ) ? "copy_node" : "move_node"), (data.data.origin && data.data.origin !== ins ? data.data.origin.get_node(data.data.nodes[t1]) : data.data.nodes[t1]), $.jstree.root, 'last', { 'dnd' : true, 'ref' : ins.get_node($.jstree.root), 'pos' : 'i', 'origin' : data.data.origin, 'is_multi' : (data.data.origin && data.data.origin !== ins), 'is_foreign' : (!data.data.origin) });
                            if(!ok) { break; }
                        }
                        if(ok) {
                            lastmv = { 'ins' : ins, 'par' : $.jstree.root, 'pos' : 'last' };
                            marker.hide();
                            data.helper.find('.jstree-icon').first().removeClass('jstree-er').addClass('jstree-ok');
                            return;
                        }
                    }
                    else {
                        ref = ins.settings.dnd.large_drop_target ? $(data.event.target).closest('.jstree-node').children('.jstree-anchor') : $(data.event.target).closest('.jstree-anchor');
                        if(ref && ref.length && ref.parent().is('.jstree-closed, .jstree-open, .jstree-leaf')) {
                            off = ref.offset();
                            rel = data.event.pageY - off.top;
                            h = ref.outerHeight();
                            if(rel < h / 3) {
                                o = ['b', 'i', 'a'];
                            }
                            else if(rel > h - h / 3) {
                                o = ['a', 'i', 'b'];
                            }
                            else {
                                o = rel > h / 2 ? ['i', 'a', 'b'] : ['i', 'b', 'a'];
                            }
                            $.each(o, function (j, v) {
                                switch(v) {
                                    case 'b':
                                        l = off.left - 6;
                                        t = off.top;
                                        p = ins.get_parent(ref);
                                        i = ref.parent().index();
                                        break;
                                    case 'i':
                                        ip = ins.settings.dnd.inside_pos;
                                        tm = ins.get_node(ref.parent());
                                        l = off.left - 2;
                                        t = off.top + h / 2 + 1;
                                        p = tm.id;
                                        i = ip === 'first' ? 0 : (ip === 'last' ? tm.children.length : Math.min(ip, tm.children.length));
                                        break;
                                    case 'a':
                                        l = off.left - 6;
                                        t = off.top + h;
                                        p = ins.get_parent(ref);
                                        i = ref.parent().index() + 1;
                                        break;
                                }
                                ok = true;
                                for(t1 = 0, t2 = data.data.nodes.length; t1 < t2; t1++) {
                                    op = data.data.origin && (data.data.origin.settings.dnd.always_copy || (data.data.origin.settings.dnd.copy && (data.event.metaKey || data.event.ctrlKey))) ? "copy_node" : "move_node";
                                    ps = i;
                                    if(op === "move_node" && v === 'a' && (data.data.origin && data.data.origin === ins) && p === ins.get_parent(data.data.nodes[t1])) {
                                        pr = ins.get_node(p);
                                        if(ps > $.inArray(data.data.nodes[t1], pr.children)) {
                                            ps -= 1;
                                        }
                                    }
                                    ok = ok && ( (ins && ins.settings && ins.settings.dnd && ins.settings.dnd.check_while_dragging === false) || ins.check(op, (data.data.origin && data.data.origin !== ins ? data.data.origin.get_node(data.data.nodes[t1]) : data.data.nodes[t1]), p, ps, { 'dnd' : true, 'ref' : ins.get_node(ref.parent()), 'pos' : v, 'origin' : data.data.origin, 'is_multi' : (data.data.origin && data.data.origin !== ins), 'is_foreign' : (!data.data.origin) }) );
                                    if(!ok) {
                                        if(ins && ins.last_error) { laster = ins.last_error(); }
                                        break;
                                    }
                                }
                                if(v === 'i' && ref.parent().is('.jstree-closed') && ins.settings.dnd.open_timeout) {
                                    opento = setTimeout((function (x, z) { return function () { x.open_node(z); }; }(ins, ref)), ins.settings.dnd.open_timeout);
                                }
                                if(ok) {
                                    lastmv = { 'ins' : ins, 'par' : p, 'pos' : v === 'i' && ip === 'last' && i === 0 && !ins.is_loaded(tm) ? 'last' : i };
                                    marker.css({ 'left' : l + 'px', 'top' : t + 'px' }).show();
                                    data.helper.find('.jstree-icon').first().removeClass('jstree-er').addClass('jstree-ok');
                                    laster = {};
                                    o = true;
                                    return false;
                                }
                            });
                            if(o === true) { return; }
                        }
                    }
                }
                lastmv = false;
                data.helper.find('.jstree-icon').removeClass('jstree-ok').addClass('jstree-er');
                marker.hide();
            })
            .on('dnd_scroll.vakata.jstree', function (e, data) {
                if(!data || !data.data || !data.data.jstree) { return; }
                marker.hide();
                lastmv = false;
                lastev = false;
                data.helper.find('.jstree-icon').first().removeClass('jstree-ok').addClass('jstree-er');
            })
            .on('dnd_stop.vakata.jstree', function (e, data) {
                if(opento) { clearTimeout(opento); }
                if(!data || !data.data || !data.data.jstree) { return; }
                marker.hide().detach();
                var i, j, nodes = [];
                if(lastmv) {
                    for(i = 0, j = data.data.nodes.length; i < j; i++) {
                        nodes[i] = data.data.origin ? data.data.origin.get_node(data.data.nodes[i]) : data.data.nodes[i];
                    }
                    lastmv.ins[ data.data.origin && (data.data.origin.settings.dnd.always_copy || (data.data.origin.settings.dnd.copy && (data.event.metaKey || data.event.ctrlKey))) ? 'copy_node' : 'move_node' ](nodes, lastmv.par, lastmv.pos, false, false, false, data.data.origin);
                }
                else {
                    i = $(data.event.target).closest('.jstree');
                    if(i.length && laster && laster.error && laster.error === 'check') {
                        i = i.jstree(true);
                        if(i) {
                            i.settings.core.error.call(this, laster);
                        }
                    }
                }
                lastev = false;
                lastmv = false;
            })
            .on('keyup.jstree keydown.jstree', function (e, data) {
                data = $.vakata.dnd._get();
                if(data && data.data && data.data.jstree) {
                    data.helper.find('.jstree-copy').first()[ data.data.origin && (data.data.origin.settings.dnd.always_copy || (data.data.origin.settings.dnd.copy && (e.metaKey || e.ctrlKey))) ? 'show' : 'hide' ]();
                    if(lastev) {
                        lastev.metaKey = e.metaKey;
                        lastev.ctrlKey = e.ctrlKey;
                        $.vakata.dnd._trigger('move', lastev);
                    }
                }
            });
    });

    (function ($) {
        $.vakata.html = {
            div : $('<div />'),
            escape : function (str) {
                return $.vakata.html.div.text(str).html();
            },
            strip : function (str) {
                return $.vakata.html.div.empty().append($.parseHTML(str)).text();
            }
        };
        var vakata_dnd = {
            element    : false,
            target    : false,
            is_down    : false,
            is_drag    : false,
            helper    : false,
            helper_w: 0,
            data    : false,
            init_x    : 0,
            init_y    : 0,
            scroll_l: 0,
            scroll_t: 0,
            scroll_e: false,
            scroll_i: false,
            is_touch: false
        };
        $.vakata.dnd = {
            settings : {
                scroll_speed        : 10,
                scroll_proximity    : 20,
                helper_left            : 5,
                helper_top            : 10,
                threshold            : 5,
                threshold_touch        : 50
            },
            _trigger : function (event_name, e) {
                var data = $.vakata.dnd._get();
                data.event = e;
                $(document).triggerHandler("dnd_" + event_name + ".vakata", data);
            },
            _get : function () {
                return {
                    "data"        : vakata_dnd.data,
                    "element"    : vakata_dnd.element,
                    "helper"    : vakata_dnd.helper
                };
            },
            _clean : function () {
                if(vakata_dnd.helper) { vakata_dnd.helper.remove(); }
                if(vakata_dnd.scroll_i) { clearInterval(vakata_dnd.scroll_i); vakata_dnd.scroll_i = false; }
                vakata_dnd = {
                    element    : false,
                    target    : false,
                    is_down    : false,
                    is_drag    : false,
                    helper    : false,
                    helper_w: 0,
                    data    : false,
                    init_x    : 0,
                    init_y    : 0,
                    scroll_l: 0,
                    scroll_t: 0,
                    scroll_e: false,
                    scroll_i: false,
                    is_touch: false
                };
                $(document).off("mousemove.vakata.jstree touchmove.vakata.jstree", $.vakata.dnd.drag);
                $(document).off("mouseup.vakata.jstree touchend.vakata.jstree", $.vakata.dnd.stop);
            },
            _scroll : function (init_only) {
                if(!vakata_dnd.scroll_e || (!vakata_dnd.scroll_l && !vakata_dnd.scroll_t)) {
                    if(vakata_dnd.scroll_i) { clearInterval(vakata_dnd.scroll_i); vakata_dnd.scroll_i = false; }
                    return false;
                }
                if(!vakata_dnd.scroll_i) {
                    vakata_dnd.scroll_i = setInterval($.vakata.dnd._scroll, 100);
                    return false;
                }
                if(init_only === true) { return false; }

                var i = vakata_dnd.scroll_e.scrollTop(),
                    j = vakata_dnd.scroll_e.scrollLeft();
                vakata_dnd.scroll_e.scrollTop(i + vakata_dnd.scroll_t * $.vakata.dnd.settings.scroll_speed);
                vakata_dnd.scroll_e.scrollLeft(j + vakata_dnd.scroll_l * $.vakata.dnd.settings.scroll_speed);
                if(i !== vakata_dnd.scroll_e.scrollTop() || j !== vakata_dnd.scroll_e.scrollLeft()) {
                    $.vakata.dnd._trigger("scroll", vakata_dnd.scroll_e);
                }
            },
            start : function (e, data, html) {
                if(e.type === "touchstart" && e.originalEvent && e.originalEvent.changedTouches && e.originalEvent.changedTouches[0]) {
                    e.pageX = e.originalEvent.changedTouches[0].pageX;
                    e.pageY = e.originalEvent.changedTouches[0].pageY;
                    e.target = document.elementFromPoint(e.originalEvent.changedTouches[0].pageX - window.pageXOffset, e.originalEvent.changedTouches[0].pageY - window.pageYOffset);
                }
                if(vakata_dnd.is_drag) { $.vakata.dnd.stop({}); }
                try {
                    e.currentTarget.unselectable = "on";
                    e.currentTarget.onselectstart = function() { return false; };
                    if(e.currentTarget.style) { e.currentTarget.style.MozUserSelect = "none"; }
                } catch(ignore) { }
                vakata_dnd.init_x    = e.pageX;
                vakata_dnd.init_y    = e.pageY;
                vakata_dnd.data        = data;
                vakata_dnd.is_down    = true;
                vakata_dnd.element    = e.currentTarget;
                vakata_dnd.target    = e.target;
                vakata_dnd.is_touch    = e.type === "touchstart";
                if(html !== false) {
                    vakata_dnd.helper = $("<div id='vakata-dnd'></div>").html(html).css({
                        "display"        : "block",
                        "margin"        : "0",
                        "padding"        : "0",
                        "position"        : "absolute",
                        "top"            : "-2000px",
                        "lineHeight"    : "16px",
                        "zIndex"        : "10000"
                    });
                }
                $(document).on("mousemove.vakata.jstree touchmove.vakata.jstree", $.vakata.dnd.drag);
                $(document).on("mouseup.vakata.jstree touchend.vakata.jstree", $.vakata.dnd.stop);
                return false;
            },
            drag : function (e) {
                if(e.type === "touchmove" && e.originalEvent && e.originalEvent.changedTouches && e.originalEvent.changedTouches[0]) {
                    e.pageX = e.originalEvent.changedTouches[0].pageX;
                    e.pageY = e.originalEvent.changedTouches[0].pageY;
                    e.target = document.elementFromPoint(e.originalEvent.changedTouches[0].pageX - window.pageXOffset, e.originalEvent.changedTouches[0].pageY - window.pageYOffset);
                }
                if(!vakata_dnd.is_down) { return; }
                if(!vakata_dnd.is_drag) {
                    if(
                        Math.abs(e.pageX - vakata_dnd.init_x) > (vakata_dnd.is_touch ? $.vakata.dnd.settings.threshold_touch : $.vakata.dnd.settings.threshold) ||
                        Math.abs(e.pageY - vakata_dnd.init_y) > (vakata_dnd.is_touch ? $.vakata.dnd.settings.threshold_touch : $.vakata.dnd.settings.threshold)
                    ) {
                        if(vakata_dnd.helper) {
                            vakata_dnd.helper.appendTo("body");
                            vakata_dnd.helper_w = vakata_dnd.helper.outerWidth();
                        }
                        vakata_dnd.is_drag = true;
                        $.vakata.dnd._trigger("start", e);
                    }
                    else { return; }
                }

                var d  = false, w  = false,
                    dh = false, wh = false,
                    dw = false, ww = false,
                    dt = false, dl = false,
                    ht = false, hl = false;

                vakata_dnd.scroll_t = 0;
                vakata_dnd.scroll_l = 0;
                vakata_dnd.scroll_e = false;
                $($(e.target).parentsUntil("body").addBack().get().reverse())
                    .filter(function () {
                        return    (/^auto|scroll$/).test($(this).css("overflow")) &&
                                (this.scrollHeight > this.offsetHeight || this.scrollWidth > this.offsetWidth);
                    })
                    .each(function () {
                        var t = $(this), o = t.offset();
                        if(this.scrollHeight > this.offsetHeight) {
                            if(o.top + t.height() - e.pageY < $.vakata.dnd.settings.scroll_proximity)    { vakata_dnd.scroll_t = 1; }
                            if(e.pageY - o.top < $.vakata.dnd.settings.scroll_proximity)                { vakata_dnd.scroll_t = -1; }
                        }
                        if(this.scrollWidth > this.offsetWidth) {
                            if(o.left + t.width() - e.pageX < $.vakata.dnd.settings.scroll_proximity)    { vakata_dnd.scroll_l = 1; }
                            if(e.pageX - o.left < $.vakata.dnd.settings.scroll_proximity)                { vakata_dnd.scroll_l = -1; }
                        }
                        if(vakata_dnd.scroll_t || vakata_dnd.scroll_l) {
                            vakata_dnd.scroll_e = $(this);
                            return false;
                        }
                    });

                if(!vakata_dnd.scroll_e) {
                    d  = $(document); w = $(window);
                    dh = d.height(); wh = w.height();
                    dw = d.width(); ww = w.width();
                    dt = d.scrollTop(); dl = d.scrollLeft();
                    if(dh > wh && e.pageY - dt < $.vakata.dnd.settings.scroll_proximity)        { vakata_dnd.scroll_t = -1;  }
                    if(dh > wh && wh - (e.pageY - dt) < $.vakata.dnd.settings.scroll_proximity)    { vakata_dnd.scroll_t = 1; }
                    if(dw > ww && e.pageX - dl < $.vakata.dnd.settings.scroll_proximity)        { vakata_dnd.scroll_l = -1; }
                    if(dw > ww && ww - (e.pageX - dl) < $.vakata.dnd.settings.scroll_proximity)    { vakata_dnd.scroll_l = 1; }
                    if(vakata_dnd.scroll_t || vakata_dnd.scroll_l) {
                        vakata_dnd.scroll_e = d;
                    }
                }
                if(vakata_dnd.scroll_e) { $.vakata.dnd._scroll(true); }

                if(vakata_dnd.helper) {
                    ht = parseInt(e.pageY + $.vakata.dnd.settings.helper_top, 10);
                    hl = parseInt(e.pageX + $.vakata.dnd.settings.helper_left, 10);
                    if(dh && ht + 25 > dh) { ht = dh - 50; }
                    if(dw && hl + vakata_dnd.helper_w > dw) { hl = dw - (vakata_dnd.helper_w + 2); }
                    vakata_dnd.helper.css({
                        left    : hl + "px",
                        top        : ht + "px"
                    });
                }
                $.vakata.dnd._trigger("move", e);
                return false;
            },
            stop : function (e) {
                if(e.type === "touchend" && e.originalEvent && e.originalEvent.changedTouches && e.originalEvent.changedTouches[0]) {
                    e.pageX = e.originalEvent.changedTouches[0].pageX;
                    e.pageY = e.originalEvent.changedTouches[0].pageY;
                    e.target = document.elementFromPoint(e.originalEvent.changedTouches[0].pageX - window.pageXOffset, e.originalEvent.changedTouches[0].pageY - window.pageYOffset);
                }
                if(vakata_dnd.is_drag) {
                    $.vakata.dnd._trigger("stop", e);
                }
                else {
                    if(e.type === "touchend" && e.target === vakata_dnd.target) {
                        var to = setTimeout(function () { $(e.target).click(); }, 100);
                        $(e.target).one('click', function() { if(to) { clearTimeout(to); } });
                    }
                }
                $.vakata.dnd._clean();
                return false;
            }
        };
    }($));

}));
