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
        define('jstree.wholerow', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.wholerow) { return; }

    var div = document.createElement('DIV');
    div.setAttribute('unselectable','on');
    div.setAttribute('role','presentation');
    div.className = 'jstree-wholerow';
    div.innerHTML = '&#160;';
    $.jstree.plugins.wholerow = function (options, parent) {
        this.bind = function () {
            parent.bind.call(this);

            this.element
                .on('ready.jstree set_state.jstree', $.proxy(function () {
                        this.hide_dots();
                    }, this))
                .on("init.jstree loading.jstree ready.jstree", $.proxy(function () {
                        this.get_container_ul().addClass('jstree-wholerow-ul');
                    }, this))
                .on("deselect_all.jstree", $.proxy(function (e, data) {
                        this.element.find('.jstree-wholerow-clicked').removeClass('jstree-wholerow-clicked');
                    }, this))
                .on("changed.jstree", $.proxy(function (e, data) {
                        this.element.find('.jstree-wholerow-clicked').removeClass('jstree-wholerow-clicked');
                        var tmp = false, i, j;
                        for(i = 0, j = data.selected.length; i < j; i++) {
                            tmp = this.get_node(data.selected[i], true);
                            if(tmp && tmp.length) {
                                tmp.children('.jstree-wholerow').addClass('jstree-wholerow-clicked');
                            }
                        }
                    }, this))
                .on("open_node.jstree", $.proxy(function (e, data) {
                        this.get_node(data.node, true).find('.jstree-clicked').parent().children('.jstree-wholerow').addClass('jstree-wholerow-clicked');
                    }, this))
                .on("hover_node.jstree dehover_node.jstree", $.proxy(function (e, data) {
                        if(e.type === "hover_node" && this.is_disabled(data.node)) { return; }
                        this.get_node(data.node, true).children('.jstree-wholerow')[e.type === "hover_node"?"addClass":"removeClass"]('jstree-wholerow-hovered');
                    }, this))
                .on("contextmenu.jstree", ".jstree-wholerow", $.proxy(function (e) {
                        e.preventDefault();
                        var tmp = $.Event('contextmenu', { metaKey : e.metaKey, ctrlKey : e.ctrlKey, altKey : e.altKey, shiftKey : e.shiftKey, pageX : e.pageX, pageY : e.pageY });
                        $(e.currentTarget).closest(".jstree-node").children(".jstree-anchor").first().trigger(tmp);
                    }, this))
                .on("click.jstree", ".jstree-wholerow", function (e) {
                        e.stopImmediatePropagation();
                        var tmp = $.Event('click', { metaKey : e.metaKey, ctrlKey : e.ctrlKey, altKey : e.altKey, shiftKey : e.shiftKey });
                        $(e.currentTarget).closest(".jstree-node").children(".jstree-anchor").first().trigger(tmp).focus();
                    })
                .on("click.jstree", ".jstree-leaf > .jstree-ocl", $.proxy(function (e) {
                        e.stopImmediatePropagation();
                        var tmp = $.Event('click', { metaKey : e.metaKey, ctrlKey : e.ctrlKey, altKey : e.altKey, shiftKey : e.shiftKey });
                        $(e.currentTarget).closest(".jstree-node").children(".jstree-anchor").first().trigger(tmp).focus();
                    }, this))
                .on("mouseover.jstree", ".jstree-wholerow, .jstree-icon", $.proxy(function (e) {
                        e.stopImmediatePropagation();
                        if(!this.is_disabled(e.currentTarget)) {
                            this.hover_node(e.currentTarget);
                        }
                        return false;
                    }, this))
                .on("mouseleave.jstree", ".jstree-node", $.proxy(function (e) {
                        this.dehover_node(e.currentTarget);
                    }, this));
        };
        this.teardown = function () {
            if(this.settings.wholerow) {
                this.element.find(".jstree-wholerow").remove();
            }
            parent.teardown.call(this);
        };
        this.redraw_node = function(obj, deep, callback, force_render) {
            obj = parent.redraw_node.apply(this, arguments);
            if(obj) {
                var tmp = div.cloneNode(true);
                if($.inArray(obj.id, this._data.core.selected) !== -1) { tmp.className += ' jstree-wholerow-clicked'; }
                if(this._data.core.focused && this._data.core.focused === obj.id) { tmp.className += ' jstree-wholerow-hovered'; }
                obj.insertBefore(tmp, obj.childNodes[0]);
            }
            return obj;
        };
    };
}));
