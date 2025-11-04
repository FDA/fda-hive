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
(()=>{"use strict";var e,r,t,a,o={},n={};function d(e){var r=n[e];if(void 0!==r)return r.exports;var t=n[e]={id:e,loaded:!1,exports:{}};return o[e].call(t.exports,t,t.exports,d),t.loaded=!0,t.exports}d.m=o,e=[],d.O=(r,t,a,o)=>{if(!t){var n=1/0;for(l=0;l<e.length;l++){for(var[t,a,o]=e[l],i=!0,f=0;f<t.length;f++)(!1&o||n>=o)&&Object.keys(d.O).every(e=>d.O[e](t[f]))?t.splice(f--,1):(i=!1,o<n&&(n=o));i&&(e.splice(l--,1),r=a())}return r}o=o||0;for(var l=e.length;l>0&&e[l-1][2]>o;l--)e[l]=e[l-1];e[l]=[t,a,o]},d.n=e=>{var r=e&&e.__esModule?()=>e.default:()=>e;return d.d(r,{a:r}),r},d.d=(e,r)=>{for(var t in r)d.o(r,t)&&!d.o(e,t)&&Object.defineProperty(e,t,{enumerable:!0,get:r[t]})},d.f={},d.e=e=>Promise.all(Object.keys(d.f).reduce((r,t)=>(d.f[t](e,r),r),[])),d.u=e=>(592===e?"common":e)+"."+{164:"021ce185546e5a7f21f9",260:"a1a7e77cf3038e80e331",354:"ae481eb4208eba4938e5",422:"75886c3ec4fa4ee551cf",541:"db22dd85354e910bf6c0",583:"7de8475130dd6ed7eff4",592:"06df2261b63889461d6a",661:"58db34a40c41834e9e18",683:"3902eff4088354b6192e",829:"0e0b004dff574ca229db",887:"3782b4ee26106cae53f8",925:"d858e061c295c5683d77",982:"a1f153fb91effca987b6"}[e]+".js",d.miniCssF=e=>"styles.83ba280efdf6ded28b99.css",d.o=(e,r)=>Object.prototype.hasOwnProperty.call(e,r),r={},t="hive-fe:",d.l=(e,a,o,n)=>{if(r[e])r[e].push(a);else{var i,f;if(void 0!==o)for(var l=document.getElementsByTagName("script"),c=0;c<l.length;c++){var u=l[c];if(u.getAttribute("src")==e||u.getAttribute("data-webpack")==t+o){i=u;break}}i||(f=!0,(i=document.createElement("script")).charset="utf-8",i.timeout=120,d.nc&&i.setAttribute("nonce",d.nc),i.setAttribute("data-webpack",t+o),i.src=d.tu(e)),r[e]=[a];var s=(t,a)=>{i.onerror=i.onload=null,clearTimeout(p);var o=r[e];if(delete r[e],i.parentNode&&i.parentNode.removeChild(i),o&&o.forEach(e=>e(a)),t)return t(a)},p=setTimeout(s.bind(null,void 0,{type:"timeout",target:i}),12e4);i.onerror=s.bind(null,i.onerror),i.onload=s.bind(null,i.onload),f&&document.head.appendChild(i)}},d.r=e=>{"undefined"!=typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(e,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(e,"__esModule",{value:!0})},d.nmd=e=>(e.paths=[],e.children||(e.children=[]),e),d.tu=e=>(void 0===a&&(a={createScriptURL:e=>e},"undefined"!=typeof trustedTypes&&trustedTypes.createPolicy&&(a=trustedTypes.createPolicy("angular#bundler",a))),a.createScriptURL(e)),d.p="te/",(()=>{var e={666:0};d.f.j=(r,t)=>{var a=d.o(e,r)?e[r]:void 0;if(0!==a)if(a)t.push(a[2]);else if(666!=r){var o=new Promise((t,o)=>a=e[r]=[t,o]);t.push(a[2]=o);var n=d.p+d.u(r),i=new Error;d.l(n,t=>{if(d.o(e,r)&&(0!==(a=e[r])&&(e[r]=void 0),a)){var o=t&&("load"===t.type?"missing":t.type),n=t&&t.target&&t.target.src;i.message="Loading chunk "+r+" failed.\n("+o+": "+n+")",i.name="ChunkLoadError",i.type=o,i.request=n,a[1](i)}},"chunk-"+r,r)}else e[r]=0},d.O.j=r=>0===e[r];var r=(r,t)=>{var a,o,[n,i,f]=t,l=0;for(a in i)d.o(i,a)&&(d.m[a]=i[a]);if(f)var c=f(d);for(r&&r(t);l<n.length;l++)d.o(e,o=n[l])&&e[o]&&e[o][0](),e[n[l]]=0;return d.O(c)},t=self.webpackChunkhive_fe=self.webpackChunkhive_fe||[];t.forEach(r.bind(null,0)),t.push=r.bind(null,t.push.bind(t))})()})();