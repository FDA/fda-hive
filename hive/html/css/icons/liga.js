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

(function () {
    'use strict';
    function supportsProperty(p) {
        var prefixes = ['Webkit', 'Moz', 'O', 'ms'],
            i,
            div = document.createElement('div'),
            ret = p in div.style;
        if (!ret) {
            p = p.charAt(0).toUpperCase() + p.substr(1);
            for (i = 0; i < prefixes.length; i += 1) {
                ret = prefixes[i] + p in div.style;
                if (ret) {
                    break;
                }
            }
        }
        return ret;
    }
    var icons;
    if (!supportsProperty('fontFeatureSettings')) {
        icons = {
            'share': '&#xe80d;',
            'download': '&#xe961;',
            'save': '&#xe961;',
            'blur_linear': '&#xe3a3;',
            'blur_on': '&#xe3a5;',
            'search': '&#xe986;',
            'magnifier': '&#xe986;',
            'loop2': '&#xea2e;',
            'repeat2': '&#xea2e;',
            'checkbox-checked': '&#xea52;',
            'checkbox': '&#xea52;',
            'pushpin': '&#xe946;',
            'pin': '&#xe946;',
            'drawer': '&#xe95c;',
            'box': '&#xe95c;',
            'download': '&#xe960;',
            'save': '&#xe960;',
            'user': '&#xe971;',
            'profile2': '&#xe971;',
            'hour-glass': '&#xe979;',
            'loading': '&#xe979;',
            'spinner11': '&#xe984;',
            'loading12': '&#xe984;',
            'enlarge': '&#xe989;',
            'expand': '&#xe989;',
            'shrink': '&#xe98a;',
            'collapse': '&#xe98a;',
            'enlarge2': '&#xe98b;',
            'expand2': '&#xe98b;',
            'shrink2': '&#xe98c;',
            'collapse2': '&#xe98c;',
            'equalizer': '&#xe992;',
            'sliders': '&#xe992;',
            'cog': '&#xe994;',
            'gear': '&#xe994;',
            'cogs': '&#xe995;',
            'gears': '&#xe995;',
            'stats-bars': '&#xe99c;',
            'stats3': '&#xe99c;',
            'plus': '&#xea0a;',
            'add': '&#xea0a;',
            'minus': '&#xea0b;',
            'subtract': '&#xea0b;',
            'checkmark': '&#xea10;',
            'tick': '&#xea10;',
            'move-down': '&#xea47;',
            'sort2': '&#xea47;',
            'cross': '&#xea0f;',
            'cancel': '&#xea0f;',
            'home': '&#xe92a;',
            'house': '&#xe92a;',
            'pen': '&#xe92c;',
            'write3': '&#xe92c;',
            'folder': '&#xe92f;',
            'directory': '&#xe92f;',
            'clock': '&#xe94e;',
            'time2': '&#xe94e;',
            'bell': '&#xe951;',
            'alarm2': '&#xe951;',
            'upload': '&#xe962;',
            'load': '&#xe962;',
            'user': '&#xe972;',
            'profile2': '&#xe972;',
            'flag': '&#xe9cc;',
            'report': '&#xe9cc;',
            'enter': '&#xea13;',
            'signin': '&#xea13;',
            'exit': '&#xea14;',
            'signout': '&#xea14;',
          '0': 0
        };
        delete icons['0'];
        window.icomoonLiga = function (els) {
            var classes,
                el,
                i,
                innerHTML,
                key;
            els = els || document.getElementsByTagName('*');
            if (!els.length) {
                els = [els];
            }
            for (i = 0; ; i += 1) {
                el = els[i];
                if (!el) {
                    break;
                }
                classes = el.className;
                if (/icomoon-liga/.test(classes)) {
                    innerHTML = el.innerHTML;
                    if (innerHTML && innerHTML.length > 1) {
                        for (key in icons) {
                            if (icons.hasOwnProperty(key)) {
                                innerHTML = innerHTML.replace(new RegExp(key, 'g'), icons[key]);
                            }
                        }
                        el.innerHTML = innerHTML;
                    }
                }
            }
        };
        window.icomoonLiga();
    }
}());
