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
        define('jstree.search', ['jquery','jstree'], factory);
    }
    else if(typeof exports === 'object') {
        factory(require('jquery'), require('jstree'));
    }
    else {
        factory(jQuery, jQuery.jstree);
    }
}(function ($, jstree, undefined) {
    "use strict";

    if($.jstree.plugins.search) { return; }

    $.jstree.defaults.search = {
        ajax : false,
        fuzzy : false,
        case_sensitive : false,
        show_only_matches : false,
        show_only_matches_children : false,
        close_opened_onclear : true,
        search_leaves_only : false,
        search_callback : false
    };

    $.jstree.plugins.search = function (options, parent) {
        this.bind = function () {
            parent.bind.call(this);

            this._data.search.str = "";
            this._data.search.dom = $();
            this._data.search.res = [];
            this._data.search.opn = [];
            this._data.search.som = false;
            this._data.search.smc = false;
            this._data.search.hdn = [];

            this.element
                .on("search.jstree", $.proxy(function (e, data) {
                        if(this._data.search.som && data.res.length) {
                            var m = this._model.data, i, j, p = [];
                            for(i = 0, j = data.res.length; i < j; i++) {
                                if(m[data.res[i]] && !m[data.res[i]].state.hidden) {
                                    p.push(data.res[i]);
                                    p = p.concat(m[data.res[i]].parents);
                                    if(this._data.search.smc) {
                                        p = p.concat(m[data.res[i]].children_d);
                                    }
                                }
                            }
                            p = $.vakata.array_remove_item($.vakata.array_unique(p), $.jstree.root);
                            this._data.search.hdn = this.hide_all(true);
                            this.show_node(p);
                        }
                    }, this))
                .on("clear_search.jstree", $.proxy(function (e, data) {
                        if(this._data.search.som && data.res.length) {
                            this.show_node(this._data.search.hdn);
                        }
                    }, this));
        };
        this.search = function (str, skip_async, show_only_matches, inside, append, show_only_matches_children) {
            if(str === false || $.trim(str.toString()) === "") {
                return this.clear_search();
            }
            inside = this.get_node(inside);
            inside = inside && inside.id ? inside.id : null;
            str = str.toString();
            var s = this.settings.search,
                a = s.ajax ? s.ajax : false,
                m = this._model.data,
                f = null,
                r = [],
                p = [], i, j;
            if(this._data.search.res.length && !append) {
                this.clear_search();
            }
            if(show_only_matches === undefined) {
                show_only_matches = s.show_only_matches;
            }
            if(show_only_matches_children === undefined) {
                show_only_matches_children = s.show_only_matches_children;
            }
            if(!skip_async && a !== false) {
                if($.isFunction(a)) {
                    return a.call(this, str, $.proxy(function (d) {
                            if(d && d.d) { d = d.d; }
                            this._load_nodes(!$.isArray(d) ? [] : $.vakata.array_unique(d), function () {
                                this.search(str, true, show_only_matches, inside, append);
                            }, true);
                        }, this), inside);
                }
                else {
                    a = $.extend({}, a);
                    if(!a.data) { a.data = {}; }
                    a.data.str = str;
                    if(inside) {
                        a.data.inside = inside;
                    }
                    return $.ajax(a)
                        .fail($.proxy(function () {
                            this._data.core.last_error = { 'error' : 'ajax', 'plugin' : 'search', 'id' : 'search_01', 'reason' : 'Could not load search parents', 'data' : JSON.stringify(a) };
                            this.settings.core.error.call(this, this._data.core.last_error);
                        }, this))
                        .done($.proxy(function (d) {
                            if(d && d.d) { d = d.d; }
                            this._load_nodes(!$.isArray(d) ? [] : $.vakata.array_unique(d), function () {
                                this.search(str, true, show_only_matches, inside, append);
                            }, true);
                        }, this));
                }
            }
            if(!append) {
                this._data.search.str = str;
                this._data.search.dom = $();
                this._data.search.res = [];
                this._data.search.opn = [];
                this._data.search.som = show_only_matches;
                this._data.search.smc = show_only_matches_children;
            }

            f = new $.vakata.search(str, true, { caseSensitive : s.case_sensitive, fuzzy : s.fuzzy });
            $.each(m[inside ? inside : $.jstree.root].children_d, function (ii, i) {
                var v = m[i];
                if(v.text && (!s.search_leaves_only || (v.state.loaded && v.children.length === 0)) && ( (s.search_callback && s.search_callback.call(this, str, v)) || (!s.search_callback && f.search(v.text).isMatch) ) ) {
                    r.push(i);
                    p = p.concat(v.parents);
                }
            });
            if(r.length) {
                p = $.vakata.array_unique(p);
                for(i = 0, j = p.length; i < j; i++) {
                    if(p[i] !== $.jstree.root && m[p[i]] && this.open_node(p[i], null, 0) === true) {
                        this._data.search.opn.push(p[i]);
                    }
                }
                if(!append) {
                    this._data.search.dom = $(this.element[0].querySelectorAll('#' + $.map(r, function (v) { return "0123456789".indexOf(v[0]) !== -1 ? '\\3' + v[0] + ' ' + v.substr(1).replace($.jstree.idregex,'\\$&') : v.replace($.jstree.idregex,'\\$&'); }).join(', #')));
                    this._data.search.res = r;
                }
                else {
                    this._data.search.dom = this._data.search.dom.add($(this.element[0].querySelectorAll('#' + $.map(r, function (v) { return "0123456789".indexOf(v[0]) !== -1 ? '\\3' + v[0] + ' ' + v.substr(1).replace($.jstree.idregex,'\\$&') : v.replace($.jstree.idregex,'\\$&'); }).join(', #'))));
                    this._data.search.res = $.vakata.array_unique(this._data.search.res.concat(r));
                }
                this._data.search.dom.children(".jstree-anchor").addClass('jstree-search');
            }
            this.trigger('search', { nodes : this._data.search.dom, str : str, res : this._data.search.res, show_only_matches : show_only_matches });
        };
        this.clear_search = function () {
            if(this.settings.search.close_opened_onclear) {
                this.close_node(this._data.search.opn, 0);
            }
            this.trigger('clear_search', { 'nodes' : this._data.search.dom, str : this._data.search.str, res : this._data.search.res });
            if(this._data.search.res.length) {
                this._data.search.dom = $(this.element[0].querySelectorAll('#' + $.map(this._data.search.res, function (v) {
                    return "0123456789".indexOf(v[0]) !== -1 ? '\\3' + v[0] + ' ' + v.substr(1).replace($.jstree.idregex,'\\$&') : v.replace($.jstree.idregex,'\\$&');
                }).join(', #')));
                this._data.search.dom.children(".jstree-anchor").removeClass("jstree-search");
            }
            this._data.search.str = "";
            this._data.search.res = [];
            this._data.search.opn = [];
            this._data.search.dom = $();
        };

        this.redraw_node = function(obj, deep, callback, force_render) {
            obj = parent.redraw_node.apply(this, arguments);
            if(obj) {
                if($.inArray(obj.id, this._data.search.res) !== -1) {
                    var i, j, tmp = null;
                    for(i = 0, j = obj.childNodes.length; i < j; i++) {
                        if(obj.childNodes[i] && obj.childNodes[i].className && obj.childNodes[i].className.indexOf("jstree-anchor") !== -1) {
                            tmp = obj.childNodes[i];
                            break;
                        }
                    }
                    if(tmp) {
                        tmp.className += ' jstree-search';
                    }
                }
            }
            return obj;
        };
    };

    (function ($) {
        $.vakata.search = function(pattern, txt, options) {
            options = options || {};
            options = $.extend({}, $.vakata.search.defaults, options);
            if(options.fuzzy !== false) {
                options.fuzzy = true;
            }
            pattern = options.caseSensitive ? pattern : pattern.toLowerCase();
            var MATCH_LOCATION    = options.location,
                MATCH_DISTANCE    = options.distance,
                MATCH_THRESHOLD    = options.threshold,
                patternLen = pattern.length,
                matchmask, pattern_alphabet, match_bitapScore, search;
            if(patternLen > 32) {
                options.fuzzy = false;
            }
            if(options.fuzzy) {
                matchmask = 1 << (patternLen - 1);
                pattern_alphabet = (function () {
                    var mask = {},
                        i = 0;
                    for (i = 0; i < patternLen; i++) {
                        mask[pattern.charAt(i)] = 0;
                    }
                    for (i = 0; i < patternLen; i++) {
                        mask[pattern.charAt(i)] |= 1 << (patternLen - i - 1);
                    }
                    return mask;
                }());
                match_bitapScore = function (e, x) {
                    var accuracy = e / patternLen,
                        proximity = Math.abs(MATCH_LOCATION - x);
                    if(!MATCH_DISTANCE) {
                        return proximity ? 1.0 : accuracy;
                    }
                    return accuracy + (proximity / MATCH_DISTANCE);
                };
            }
            search = function (text) {
                text = options.caseSensitive ? text : text.toLowerCase();
                if(pattern === text || text.indexOf(pattern) !== -1) {
                    return {
                        isMatch: true,
                        score: 0
                    };
                }
                if(!options.fuzzy) {
                    return {
                        isMatch: false,
                        score: 1
                    };
                }
                var i, j,
                    textLen = text.length,
                    scoreThreshold = MATCH_THRESHOLD,
                    bestLoc = text.indexOf(pattern, MATCH_LOCATION),
                    binMin, binMid,
                    binMax = patternLen + textLen,
                    lastRd, start, finish, rd, charMatch,
                    score = 1,
                    locations = [];
                if (bestLoc !== -1) {
                    scoreThreshold = Math.min(match_bitapScore(0, bestLoc), scoreThreshold);
                    bestLoc = text.lastIndexOf(pattern, MATCH_LOCATION + patternLen);
                    if (bestLoc !== -1) {
                        scoreThreshold = Math.min(match_bitapScore(0, bestLoc), scoreThreshold);
                    }
                }
                bestLoc = -1;
                for (i = 0; i < patternLen; i++) {
                    binMin = 0;
                    binMid = binMax;
                    while (binMin < binMid) {
                        if (match_bitapScore(i, MATCH_LOCATION + binMid) <= scoreThreshold) {
                            binMin = binMid;
                        } else {
                            binMax = binMid;
                        }
                        binMid = Math.floor((binMax - binMin) / 2 + binMin);
                    }
                    binMax = binMid;
                    start = Math.max(1, MATCH_LOCATION - binMid + 1);
                    finish = Math.min(MATCH_LOCATION + binMid, textLen) + patternLen;
                    rd = new Array(finish + 2);
                    rd[finish + 1] = (1 << i) - 1;
                    for (j = finish; j >= start; j--) {
                        charMatch = pattern_alphabet[text.charAt(j - 1)];
                        if (i === 0) {
                            rd[j] = ((rd[j + 1] << 1) | 1) & charMatch;
                        } else {
                            rd[j] = ((rd[j + 1] << 1) | 1) & charMatch | (((lastRd[j + 1] | lastRd[j]) << 1) | 1) | lastRd[j + 1];
                        }
                        if (rd[j] & matchmask) {
                            score = match_bitapScore(i, j - 1);
                            if (score <= scoreThreshold) {
                                scoreThreshold = score;
                                bestLoc = j - 1;
                                locations.push(bestLoc);
                                if (bestLoc > MATCH_LOCATION) {
                                    start = Math.max(1, 2 * MATCH_LOCATION - bestLoc);
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                    if (match_bitapScore(i + 1, MATCH_LOCATION) > scoreThreshold) {
                        break;
                    }
                    lastRd = rd;
                }
                return {
                    isMatch: bestLoc >= 0,
                    score: score
                };
            };
            return txt === true ? { 'search' : search } : search(txt);
        };
        $.vakata.search.defaults = {
            location : 0,
            distance : 100,
            threshold : 0.6,
            fuzzy : false,
            caseSensitive : false
        };
    }($));

}));
