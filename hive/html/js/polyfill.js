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

// standard javascript methods on default objects which are not implemented in some old browsers (e.g. old IE, old Safari)

// checks if an object is an Array; for IE8
if (!Array.isArray) {
    Array.isArray = function(a) {
        return Object.prototype.toString.call(a) === '[object Array]';
    };
}

// runs a callback for every array element; for IE8
if (!Array.prototype.every) {
    Array.prototype.every = function(callback, thisArg) {
        var len = this.length;
        for (var i=0; i<len; i++) {
            if (i in this) {
                if (!callback.call(thisArg, this[i], i, this)) {
                    return false;
                }
            }
        }
        return true;
    };
}

// create new array with elements that satisfy a specified condition; for IE8
if (!Array.prototype.filter) {
    Array.prototype.filter = function(callback, thisArg) {
        var len = +this.length;
        var res = [];
        for (var i = 0; i < len; i++) {
            if (i in this) {
                var val = this[i];
                if (callback.call(thisArg, val, i, this)) {
                    res.push(val);
                }
            }
        }
        return res;
    };
}

if (!Array.prototype.forEach) {
    Array.prototype.forEach = function(callback, thisArg) {
        var len = this.length;
        for (var i=0; i<len; i++) {
            if (i in this) {
                callback.call(thisArg, this[i], i, this);
            }
        }
        return true;
    };
}

// returns first index of a given element, or -1; for IE8
if (!Array.prototype.indexOf) {
    Array.prototype.indexOf = function(searchElement, fromIndex) {
        var i = +fromIndex || 0;
        var len = +this.length;
        if (i >= len) {
            return -1;
        }
        if (i < 0) {
            i = Math.max(0, len + i);
        }
        for (; i<len; i++) {
            if (i in this && this[i] === searchElement) {
                return i;
            }
        }
        return -1;
    };
}

// returns last index of a given element, or -1; for IE8
if (!Array.prototype.lastIndexOf) {
    Array.prototype.lastIndexOf = function(searchElement, fromIndex) {
        var i = arguments.length > 1 ? +fromIndex : this.length - 1;
        var len = +this.length;
        if (i >= len) {
            i = len - 1;
        }
        if (i < 0) {
            i = Math.max(0, len + i);
        }
        for (; i>=0; i--) {
            if (i in this && this[i] === searchElement) {
                return i;
            }
        }
        return -1;
    };
}

// checks that string starts with the specific string; needed for all IE and Safari versions
if (!String.prototype.startsWith) {
    String.prototype.startsWith = function (str) {
        return this.slice(0, str.length) == str;
    };
}

// checks that string ends with the specific string; needed for all IE and Safari versions
if (!String.prototype.endsWith) {
    String.prototype.endsWith = function (str) {
        return this.slice(-str.length) == str;
    };
}

// remove whitespace from both sides of a string; needed for IE8
if (!String.prototype.trim) {
    (function() {
        var re = /^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g;
        String.prototype.trim = function() {
            return this.replace(re, '');
        };
    })();
}

if (!console) {
    console = {};
}
if (!console.log) {
    console.log = function(msg) { };
}
if (!console.error) {
    console.error = function(msg) {
        window.setTimeout(function() { throw new Error(txt, ""); });
    };
}

//# sourceURL = getBaseUrl() + "/js/polyfill.js"