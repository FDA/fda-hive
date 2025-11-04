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
(self.webpackChunkhive_fe=self.webpackChunkhive_fe||[]).push([[592],{94324:function(e){e.exports=function(e){function n(r){if(t[r])return t[r].exports;var u=t[r]={exports:{},id:r,loaded:!1};return e[r].call(u.exports,u,u.exports,n),u.loaded=!0,u.exports}var t={};return n.m=e,n.c=t,n.p="",n(0)}([function(e,n,t){e.exports=t(3)},,,function(e,n,t){"use strict";function r(e,n,t){var r=[];return e[n]===t?r.push(t):r.push(p,t),r.push(p),r}function u(e,n,t,r){var u=s;return-1!==n&&(u=e.slice(n+1,-1===t?e.length:t)),(u=u.replace(new RegExp("[\\s"+r+"]",d),s))===f?a:u.length<1?h:u[u.length-1]===o?u.slice(0,u.length-1):u}function c(e,n,t,r){var u=s;return-1!==n&&(u=e.slice(n+1,e.length)),0===(u=u.replace(new RegExp("[\\s"+t+".]",d),s)).length?e[n-1]===o&&r!==e.length?a:s:u}function i(e,n){return e.split(s).map(function(e){return e===h?e:n?x:g})}Object.defineProperty(n,"__esModule",{value:!0});var l=function(e){return e&&e.__esModule?e:{default:e}}(t(4)),a="*",o=".",s="",f="@",p="[]",h=" ",d="g",g=/[^\s]/,x=/[^.\s]/,v=/\s/g;n.default={mask:function(e,n){e=e.replace(v,s);var t=n.placeholderChar,l=n.currentCaretPosition,a=e.indexOf(f),p=e.lastIndexOf(o),h=p<a?-1:p,d=r(e,a+1,f),g=r(e,h-1,o),x=function(e,n){return-1===n?e:e.slice(0,n)}(e,a),O=u(e,a,h,t),_=c(e,h,t,l);return x=i(x),O=i(O),_=i(_,!0),x.concat(d).concat(O).concat(g).concat(_)},pipe:l.default}},function(e,n){"use strict";function t(e){var n=0;return e.replace(u,function(){return 1==++n?r:c})}Object.defineProperty(n,"__esModule",{value:!0}),n.default=function(e,n){var u=n.currentCaretPosition,f=n.rawValue,p=n.previousConformedValue,h=n.placeholderChar,d=e,g=(d=t(d)).indexOf(i);if(null===f.match(new RegExp("[^@\\s."+h+"]")))return c;if(-1!==d.indexOf(a)||-1!==g&&u!==g+1||-1===f.indexOf(r)&&p!==c&&-1!==f.indexOf(l))return!1;var x=d.indexOf(r);return(d.slice(x+1,d.length).match(s)||o).length>1&&d.substr(-1)===l&&u!==f.length&&(d=d.slice(0,d.length-1)),d};var r="@",u=/@/g,c="",i="@.",l=".",a="..",o=[],s=/\./g}])}}]);