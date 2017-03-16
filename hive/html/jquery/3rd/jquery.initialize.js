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
// @link https://github.com/adampietrasiak/jquery.initialize
// @author Adam Pietrasiak
// @author Damien Bezborodov
;(function($) {
    // MutationSelectorObserver represents a selector and it's associated initialization callback.
    var MutationSelectorObserver = function(selector, callback) {
        this.selector = selector;
        this.callback = callback;
    }

    // List of MutationSelectorObservers.
    var msobservers = [];
    msobservers.initialize = function(selector, callback) {

        // Wrap the callback so that we can ensure that it is only
        // called once per element.
        var seen = [];
        callbackOnce = function() {
            if (seen.indexOf(this) == -1) {
                seen.push(this);
                $(this).each(callback);
            }
        }

        // See if the selector matches any elements already on the page.
        $(selector).each(callbackOnce);

        // Then, add it to the list of selector observers.
        this.push(new MutationSelectorObserver(selector, callbackOnce));
    };

    // The MutationObserver watches for when new elements are added to the DOM.
    var observer = new MutationObserver(function(mutations) {

        // For each MutationSelectorObserver currently registered.
        for (var j = 0; j < msobservers.length; j++) {
            $(msobservers[j].selector).each(msobservers[j].callback);
        }
    });

    // Observe the entire document.
    observer.observe(document.documentElement, {childList: true, subtree: true, attributes: true});

    // Handle .initialize() calls.
    $.fn.initialize = function(callback) {
        msobservers.initialize(this.selector, callback);
    };
})(jQuery);
