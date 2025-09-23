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

(function ($, undefined ) {
    $.fn.watch = function (options) {

        var opt = $.extend({
            properties: null,

            interval: 100,

            id: "_watcher_" + new Date().getTime(),

            watchChildren: false,

            callback: null
        }, options);

        return this.each(function () {
            var el = this;
            var el$ = $(this);
            var fnc = function (mRec, mObs) {                
                __watcher.call(el, opt.id, mRec, mObs);
            };

            var data = {
                id: opt.id,
                props: opt.properties.split(','),
                vals: [opt.properties.split(',').length],
                func: opt.callback,
                fnc: fnc,
                origProps: opt.properties,
                interval: opt.interval,
                intervalId: null
            };
            $.each(data.props, function (i) {
                var propName = data.props[i];
                if (data.props[i].startsWith('attr_'))
                    data.vals[i] = el$.attr(propName.replace('attr_', ''));
                else if(propName.startsWith('prop_'))
                    data.vals[i] = el$.prop(propName.replace('props_', ''));
                else
                    data.vals[i] = el$.css(propName);
            });

            el$.data(opt.id, data);

            hookChange(el$, opt.id, data);
        });

        function hookChange(element$, id, data) {
            element$.each(function () {
                var el$ = $(this);

                if (window.MutationObserver) {
                    var observer = el$.data('__watcherObserver');
                    if (observer == null) {
                        observer = new MutationObserver(data.fnc);
                        el$.data('__watcherObserver', observer);
                    }
                    observer.observe(this, {
                        attributes: true,
                        subtree: opt.watchChildren,
                        childList: opt.watchChildren,
                        characterData: true
                    });
                } else
                    data.intervalId = setInterval(data.fnc, opt.interval);
            });
        }

        function __watcher(id, mRec, mObs) {            
            var el$ = $(this);
            var w = el$.data(id);
            if (!w) return;
            var el = this;

            if (!w.func)
                return;            

            var changed = false;
            var i = 0;
            for (i; i < w.props.length; i++) {
                var key = w.props[i];
                
                var newVal = "";
                if (key.startsWith('attr_'))
                    newVal = el$.attr(key.replace('attr_', ''));
                else if(key.startsWith('prop_'))
                    newVal = el$.prop(key.replace('prop_', ''));
                else
                    newVal = el$.css(key);

                if (newVal == undefined)
                    continue;

                if (w.vals[i] != newVal) {
                    w.vals[i] = newVal;
                    changed = true;
                    break;
                }
            }
            if (changed) {
                el$.unwatch(id);

                w.func.call(el, w, i, mRec, mObs);

                hookChange(el$, id, w);
            }
        }
    }
    $.fn.unwatch = function (id) {
        this.each(function () {
            var el = $(this);
            var data = el.data(id);
            try {
                if (window.MutationObserver) {
                    var observer = el.data("__watcherObserver");
                    if (observer) {
                        observer.disconnect();
                        el.removeData("__watcherObserver");
                    }
                } else
                    clearInterval(data.intervalId);
            }
            catch (e) {
            }
        });
        return this;
    }
    String.prototype.startsWith = function (sub) {
        if (sub === null || sub === undefined) return false;        
        return sub == this.substr(0, sub.length);
    }
})(jQuery, undefined);