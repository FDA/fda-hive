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
(window.webpackJsonp=window.webpackJsonp||[]).push([[0],{uxEF:function(e,n,t){e.exports=function(e){function n(r){if(t[r])return t[r].exports;var u=t[r]={exports:{},id:r,loaded:!1};return e[r].call(u.exports,u,u.exports,n),u.loaded=!0,u.exports}var t={};return n.m=e,n.c=t,n.p="",n(0)}([function(e,n,t){e.exports=t(3)},,,function(e,n,t){"use strict";function r(e,n,t){var r=[];return e[n]===t?r.push(t):r.push(p,t),r.push(p),r}function u(e,n,t,r){var u=s;return-1!==n&&(u=e.slice(n+1,-1===t?e.length:t)),(u=u.replace(new RegExp("[\\s"+r+"]",h),s))===f?a:u.length<1?d:u[u.length-1]===l?u.slice(0,u.length-1):u}function c(e,n,t,r){var u=s;return-1!==n&&(u=e.slice(n+1,e.length)),0===(u=u.replace(new RegExp("[\\s"+t+".]",h),s)).length?e[n-1]===l&&r!==e.length?a:s:u}function o(e,n){return e.split(s).map(function(e){return e===d?e:n?x:g})}Object.defineProperty(n,"__esModule",{value:!0});var i=function(e){return e&&e.__esModule?e:{default:e}}(t(4)),a="*",l=".",s="",f="@",p="[]",d=" ",h="g",g=/[^\s]/,x=/[^.\s]/,v=/\s/g;n.default={mask:function(e,n){e=e.replace(v,s);var t=n.placeholderChar,i=n.currentCaretPosition,a=e.indexOf(f),p=e.lastIndexOf(l),d=p<a?-1:p,h=r(e,a+1,f),g=r(e,d-1,l),x=function(e,n){return-1===n?e:e.slice(0,n)}(e,a),w=u(e,a,d,t),O=c(e,d,t,i);return x=o(x),w=o(w),O=o(O,!0),x.concat(h).concat(w).concat(g).concat(O)},pipe:i.default}},function(e,n){"use strict";function t(e){var n=0;return e.replace(u,function(){return 1==++n?r:c})}Object.defineProperty(n,"__esModule",{value:!0}),n.default=function(e,n){var u=n.currentCaretPosition,f=n.rawValue,p=n.previousConformedValue,d=n.placeholderChar,h=e,g=(h=t(h)).indexOf(o);if(null===f.match(new RegExp("[^@\\s."+d+"]")))return c;if(-1!==h.indexOf(a)||-1!==g&&u!==g+1||-1===f.indexOf(r)&&p!==c&&-1!==f.indexOf(i))return!1;var x=h.indexOf(r);return(h.slice(x+1,h.length).match(s)||l).length>1&&h.substr(-1)===i&&u!==f.length&&(h=h.slice(0,h.length-1)),h};var r="@",u=/@/g,c="",o="@.",i=".",a="..",l=[],s=/\./g}])}}]);