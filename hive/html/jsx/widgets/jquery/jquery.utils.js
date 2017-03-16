
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

//Non-standard methods added to built-in objects for convenience

//  Checks that string starts with the specific string...
if (typeof String.prototype.startsWith != 'function') {
    String.prototype.startsWith = function (str) {
        return this.slice(0, str.length) == str;
    };
}

//  Checks that string ends with the specific string...
if (typeof String.prototype.endsWith != 'function') {
    String.prototype.endsWith = function (str) {
        return this.slice(-str.length) == str;
    };
}

//  Right trim of the specific string...
if (typeof String.prototype.rtrim != 'function') {
    String.prototype.rtrim = function (s) {
        return this.replace(new RegExp(s + "*$"), '');
    };
}

//  Left trim of the specific string...
if (typeof String.prototype.ltrim != 'function') {
    String.prototype.ltrim = function (s) {
        return this.replace(new RegExp("^" + s + "*"), '');
    };
}

if (typeof String.prototype.random != 'function') {
    String.prototype.random = function (len) {
        if (len == null)
            len = 10;

        var text = "";
        //var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        for (var i = 0; i < len; i++)
            text += this.charAt(Math.floor(Math.random() * this.length));

        return text;
    };
}

//Parse function string and convert it to real javascript function...
if (typeof String.prototype.parseFunction != 'function') {
    String.prototype.parseFunction = function () {
        var funcReg = /function *[a-zA-Z0-9_]* *\(([^()]+)\)[ \n\t]*{(.*)}/gmi;
        var match = funcReg.exec(this.replace(/\n/g, ' '));

        if(match) {
            return new Function(match[1].split(','), match[2]);
        }

        return null;
    };
}

//  Capitalize first letter of each word
if (typeof String.prototype.capitalize != 'function') {
    String.prototype.capitalize = function () {
        return this.replace(/(^|\s)([a-z])/g, function (m, p1, p2) { return p1 + p2.toUpperCase(); });
    };
}

//  clear array and remove all items
if (typeof Array.prototype.clear != 'function') {
    Array.prototype.clear = function () {
        this.length = 0;
    };
}

//  split array on chunks and returns array of chunks' specified size
if (typeof Array.prototype.chunks != 'function') {
    Array.prototype.chunks = function (size) {
        var array = this;
        return [].concat.apply([],
            array.map(function (elem, i) {
                return i % size ? [] : [array.slice(i, i + size)];
            })
        );
    }
}

//  clone array and return the copy
if (typeof Array.prototype.clone != 'function') {
    Array.prototype.clone = function () {
        return this.slice(0);
    }
}

//  remove duplicates from the array
if (typeof Array.prototype.unique != 'function') {
    Array.prototype.unique = function () {
        var a = this.concat();
        for (var i = 0; i < a.length; ++i) {
            for (var j = i + 1; j < a.length; ++j) {
                if (a[i] === a[j])
                    a.splice(j--, 1);
            }
        }

        return a;
    };
}

// compare two arrays
if (typeof Array.prototype.equals != 'function') {
    Array.prototype.equals = function (array) {
        // if the other array is a falsy value, return
        if (!array)
            return false;

        // compare lengths - can save a lot of time 
        if (this.length != array.length)
            return false;

        for (var i = 0, l = this.length; i < l; i++) {
            // Check if we have nested arrays
            if (this[i] instanceof Array && array[i] instanceof Array) {
                // recurse into the nested arrays
                if (!this[i].equals(array[i]))
                    return false;
            }
            else if (this[i] != array[i]) {
                // Warning - two different object instances will never be equal: {x:20} != {x:20}
                return false;
            }
        }
        return true;
    }

    // Hide method from for-in loops
    Object.defineProperty(Array.prototype, "equals", { enumerable: false });
}

if (typeof String.prototype.toJsObject != 'function') {
    String.prototype.toJsObject = function () {
        try {
            var o = eval('(' + this + ')');
            
            if (o && typeof o === "object" && o !== null) {
                return o;
            }            
        }
        catch (e) {
        }

        return null;
    };
}

if (typeof String.prototype.isJsObject != 'function') {
    String.prototype.isJsObject = function () {
        return this.toJsObject() != null;
    };
}

//Non-standard method, available only in IE
if (typeof Boolean.parse != 'function') {
    Boolean.parse = function(val) {
        var falsy = /^(?:f(?:alse)?|no?|0+)$/i;
        return !falsy.test(val) && !!val;
    };
}

if (typeof JSON.find != 'function') {
    JSON.find = function (obj, key, val) {
        var objects = [];
        for (var p in obj) {
            if (!obj.hasOwnProperty(p)) continue;

            if (typeof obj[p] == 'object') {
                objects = objects.concat(JSON.find(obj[p], key, val));
            }
            else if (p == key && obj[key] == val) {
                objects.push(obj);
            }
        }

        return objects;
    };
}

if (typeof JSON.bind != 'function') {
    JSON.bind = function (obj, key, val, values) {
        var objects = JSON.find(obj, key, val);

        $(objects).each(function () {
            $.extend(true, this, values);
        });
    };
}

if (typeof JSON.isJson != 'function') {
    JSON.isJson = function (str) {
        try {
            var o = JSON.parse(str);
            
            if (o && typeof o === "object" && o !== null) {
                return o;
            }            
        }
        catch (e) {
        }

        return false;
    };
}

jQuery.loadCSS = function(url) {
    if (!$('link[href="' + url + '"]').length)
        $('head').append('<link rel="stylesheet" type="text/css" href="' + url + '">');
}

jQuery.loadScript = function(url) {
    if (!$('script[src="' + url + '"]').length) {
        document.write('<script type="text/javascript" src="' + url + '"></script>');
    }
}

jQuery.loadScriptAsync = function(url, callback) {
    if (!$('script[src="' + url + '"]').length) {
        jQuery.getScript(url, callback);
    }
}

jQuery.loadScripts = function(scripts, success) {
    var newScripts = [];

    //    check if scripts already loaded to the page...
    $(scripts).each(function(index, url) {
        if (!$('script[src="' + url + '"]').length) {
            newScripts.push(url);
        }
    });
    
    var length = newScripts.length;
    var counter = 0;
    var handler = function() { counter++; }
    var deferreds = [];
    
    for (var idx = 0 ; idx < length; idx++ ) {
        deferreds.push(
            $.getScript( newScripts[idx], handler)
        );
    }

    jQuery.when.apply( null, deferreds ).then(function() {
        success && success();
    });
};

$.fn.loadScript = function (url, callback) {

    if (location.protocol == 'https:') {
        url = url.replace('http://', 'https://');
    }

    var scripts = $('script[src="' + url + '"]');

    if (scripts.length > 0) {
        if ($(scripts).attr('loading')) {
            $(scripts).on('load', function () {
                callback();

                $(this).removeAttr('loading')
            });
        }
        else {
            //  script already loaded
            if (callback != null) {
                callback();

                $(this).removeAttr('loading')
            }
        }

        return this;
    }
    else {
        var script = document.createElement("script")
        script.type = "text/javascript";
        $(script).attr({loading: true});

        if (script.readyState) {  //IE
            script.onreadystatechange = function () {
                if (script.readyState == "loaded" || script.readyState == "complete") {
                    script.onreadystatechange = null;
                    callback();

                    $(this).removeAttr('loading')
                }
            };
        }
        else {  //Others
            script.onload = function () {
                callback();

                $(this).removeAttr('loading')
            };
        }

        script.src = url;
        this.get(0).appendChild(script);

        return this;
    }
};

jQuery.fn.extend({
    getPath: function () {
        var path, node = this;
        while (node.length) {
            var realNode = node[0], name = realNode.localName;
            if (!name) break;
            name = name.toLowerCase();

            var parent = node.parent();

            var sameTagSiblings = parent.children(name);
            if (sameTagSiblings.length > 1) { 
                allSiblings = parent.children();
                var index = allSiblings.index(realNode) + 1;
                if (index > 1) {
                    name += ':nth-child(' + index + ')';
                }
            }

            path = name + (path ? '>' + path : '');
            node = parent;
        }

        return path;
    }
});

$.fn.isInDom = function () {
    return $(this).parents('html').length > 0;
}

jQuery.getLayoutManager = function() {
    if($('.layout-manager').length == 0)
        console.log('ERROR: cannot find Layout Manager!');
    else
        return $('.layout-manager').first().layoutmanager('instance');
}

jQuery.queryString = function (key) {
    key = key.replace(/[*+?^$.\[\]{}()|\\\/]/g, "\\$&"); // escape RegEx control chars
    var match = location.search.match(new RegExp("[?&]" + key + "=([^&]+)(&|$)"));
    return match && decodeURIComponent(match[1].replace(/\+/g, " "));
}

var DEBUG = $.queryString('debug') != null;

jQuery.log = function(msg) {
    DEBUG && console.log(msg, arguments);
}

jQuery.loadLayoutManager = function(success) {
    $.loadCSS('jquery/bootstrap/3.3.1/css/bootstrap.min.css');

    $.loadCSS('css/flexible_layout.css');
    $.loadCSS('css/slideout-menu.css');

    //    load main jQuery layout widgets
    $.loadCSS('jsx/widgets/css/layout.master.css');

    $.loadScript('jquery/bootstrap/3.3.1/js/bootstrap.min.js');
    $.loadScript('jquery/3rd/watch/jquery-watch.js');
    
    $.loadScript('jsx/widgets/jquery/layout/jquery.layout.area.js');
    $.loadScript('jsx/widgets/jquery/layout/jquery.layout.infobox.js');
    $.loadScript('jsx/widgets/jquery/layout/jquery.layout.manager.js');
    $.loadScript('jsx/widgets/jquery/layout/jquery.layout.tabs.js');
    $.loadScript('jsx/widgets/jquery/layout/jquery.layout.tabs.scroll.js');
    $.loadScript('jsx/widgets/jquery/layout/jquery.layout.panel.js');

    $.loadScript('jsx/widgets/jquery/view/jquery.dataview.js');
    $.loadScript('jsx/widgets/jquery/view/jquery.dataview.js');
    $.loadScript('jsx/widgets/jquery/view/jquery.dataviews.js');
    $.loadScript('jsx/widgets/jquery/view/jquery.areastats.js');
}

jQuery.loadLayoutManagerAsync = function(success) {
    $.loadCSS('jquery/bootstrap/3.3.1/css/bootstrap.min.css');

    $.loadCSS('css/flexible_layout.css');
    $.loadCSS('css/slideout-menu.css');

    //    load main jQuery layout widgets
    $.loadCSS('jsx/widgets/css/layout.master.css');
    
    var scripts = [
       'jquery/bootstrap/3.3.1/js/bootstrap.min.js',
       'jquery/3rd/watch/jquery-watch.js',
       'jsx/widgets/jquery/layout/jquery.layout.area.js',
       'jsx/widgets/jquery/layout/jquery.layout.infobox.js',
       'jsx/widgets/jquery/layout/jquery.layout.manager.js',
       'jsx/widgets/jquery/layout/jquery.layout.tabs.js',
       'jsx/widgets/jquery/layout/jquery.layout.tabs.scroll.js',
       'jsx/widgets/jquery/layout/jquery.layout.panel.js',

       'jsx/widgets/jquery/view/jquery.dataview.js',
       'jsx/widgets/jquery/view/jquery.dataviews.js',
       'jsx/widgets/jquery/view/jquery.areastats.js'
    ];
    
    $.loadScripts(scripts, success);
}

$.urlParam = function(name, url) {
    if (!url) {
     url = window.location.href;
    }
    var results = new RegExp('[\\?&]' + name + '=([^&#]*)').exec(url);
    if (!results) { 
        return undefined;
    }
    return results[1] || undefined;
}
