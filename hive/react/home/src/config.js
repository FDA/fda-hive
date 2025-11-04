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
export const comb = 'https://comb.fda.gov';
export const user_path = '/usr/marianna.faradzheva/dna.cgi';
export const objList_json = 'cmdr=objList&mode=json';
export const propget_json = 'cmdr=propget&mode=json';
export const objQry = 'cmd=objQry&qry=';
export const table_items = '&actions=1&prop=name,created,_brief&flatten=1&info=1';
export const action_items = '&prop=_id,name,title,description,url,path&type=^action$&flatten=1';

export const addCGI = (url) => {
    console.log(url);
    if(url && url.indexOf('?') === 0){
        url = getPrefix() + url;
    }
    console.log(url);
    return url;
}

export const getPrefix = function (){
    let href = document.location.href;

    //Handle production/preproduction mode
    let prefix = href.indexOf("/r/") === -1 ? href : href.substring(0, href.indexOf("/r/"));

    //Handle NodeJS development enviroment
    prefix = prefix.replace('nodejs','usr');
    prefix = prefix[prefix.length - 1] === '/' ? prefix.slice(0, -1) : prefix;
    return (prefix + "/dna.cgi");
};

export const getPrefixPlain = function (){
    //let href = "https://comb.fda.gov/usr/ekaterina.minina/r/bcoeditor?id=123";
    let href = document.location.href;
    //Handle development mode
    let prefix = href.indexOf("/r/") === -1 ? href : href.substring(0, href.indexOf("/r/"));

    //Handle NodeJS development enviroment
    prefix = prefix.replace('nodejs','usr');
    return (prefix);
};

export const addURLParam = (url,name,value) => {
    url += (url.indexOf("?")) === -1 ? "?" : "&";
    url += encodeURIComponent(name) + "=" + encodeURIComponent(value);
    return url;
}

export const cookieSet = (sNm, sValue, days) => {
    var expires = ";";
    if(days) {
        var date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        expires += " expires=" + date.toGMTString();
    }
    document.cookie = sNm + "=" + encodeURIComponent(sValue) + "; Secure; SameSite=Strict; Path=/" + expires;
}