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
export const addCGI = (url) => {
    if(url && url.indexOf('?') === 0){
        url = getPrefix() + url;
    }
    return url;
}

export const getPrefix = function (){
    let prefix = getPrefixPlain();
    prefix = prefix[prefix.length - 1] === '/' ? prefix.slice(0, -1) : prefix;

    return (prefix + "/dna.cgi");
};

export const getPrefixPlain = function (){
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

export const urlExchangeParameter = (url, parname, newvalue, doNotForce) => {
        url = "" + url; // in case if this is a document.location

        var sepRe = "(&|\\?|//)";
        var parRe = sepRe + parname + "=[^&]*";

        if(doNotForce && url.search(new RegExp(parRe) )===-1 )
            return url;

        if (newvalue === "-") {
            return url.replace(new RegExp(parRe + "(&?)"), function(match, sep, endsep) {
                return (sep === "&") ? endsep : sep;
            });
        }

        var replacement = parname + "=" + newvalue;
        var parFound = false;

        url = url.replace(new RegExp(parRe), function(match, sep) {
            parFound = true;
            return sep + replacement;
        });

        if (parFound)
            return url;

        if (url === "")
            return "?" + replacement;

        url = url.replace(new RegExp(sepRe + "?$"), function(match, sep) {
            if (!sep)
                sep = "&";
            return sep + replacement;
        });
        return url;
    };