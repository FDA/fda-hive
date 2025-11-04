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
import { hideItem } from "./../view/tableView";
import axios from "axios";
import { getPrefix } from "./../config"

var bustcachevar = 1; //bust potential caching of external pages after initial request? (1=yes, 0=no)
var bustcacheparameter = "";

function ajaxDynaRequestPage(url, parameter, callbackfun, ispost, formData) {
    var page_request = false;

    if (window.XMLHttpRequest) // if Mozilla, Safari etc
        page_request = new XMLHttpRequest();
    else return false;

    page_request.parameter = parameter;

    var rawURL = parameter && parameter.isCORS;
    if (rawURL) {
        if (parameter.withCredentials) {
            page_request.withCredentials = true
        }
    }

    var posQuestion = url.indexOf('?');
    if (bustcachevar) { // if bust caching of external page
        page_request.sessionID = new Date().getTime();
        bustcacheparameter = (posQuestion !== -1) ? "&bust=" +
            page_request.sessionID : "";
    }

    if (formData) {
        page_request.open('POST', url + bustcacheparameter, true);
        page_request.send(formData);
    } else if (ispost || url.length >= 2000) {
        var paramset = "";
        if (posQuestion !== -1) {
            paramset = url.substring(posQuestion + 1);
            url = posQuestion < 1 ? "?" : url.substring(0, posQuestion);
        }
        paramset += bustcacheparameter;

        page_request.open('POST', url, true);
        page_request.setRequestHeader("Content-type",
            "application/x-www-form-urlencoded");

        page_request.send(paramset);
    } else {
        page_request.open('GET', url + bustcacheparameter, true);
        page_request.send(null);
    }
    return page_request;
}

const ajaxDynaRequestAction = (url, ispost) => {
        var page_request = false;
        if (window.XMLHttpRequest) // if Mozilla, Safari etc
            page_request = new XMLHttpRequest();

        else return false;

        var posQuestion = url.indexOf('?');
        if (bustcachevar) //if bust caching of external page
            bustcacheparameter = (url.indexOf("?") !== -1) ? "&bust=" + new Date().getTime() : "";

        if (ispost || url.length >= 2000) {
            var paramset = "";
            if (posQuestion !== -1) {
                paramset = url.substring(posQuestion + 1);
                url = url.substring(0, posQuestion);
            }
            paramset += bustcacheparameter;

            //alert("posting "+ url + "----------"+paramset);
            page_request.open('POST', url, true);
            page_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            //page_request.setRequestHeader("Content-length", paramset.length);
            page_request.send(paramset + bustcacheparameter);

        } else {
           return new Promise(resolve => {
                  console.log(getPrefix() + url + bustcacheparameter)
                  axios.get( getPrefix() + url + bustcacheparameter
                    ).then((response)=>{
                        console.log(response);
                        resolve(response);
                        const id = Object.keys(response.data)[0];
                        const signal = response.data[id].signal
                        if(response.status !== 200){
                            alert(`Object with ID ${id} did not ${signal}. Status ${response.status}`);
                        }
                    }).catch(error => console.error)
            })
        }
}

export const linkURL = async (url, newin, callback) => {
        if (newin === 'ajax') {
            if (callback === "-" || !callback){
                  return await ajaxDynaRequestAction(url);
            }else {
                ajaxDynaRequestPage(url, undefined, callback);
            }
        } else if (newin) {
            window.open(url, newin);
        }else {
            window.location.href = url;
        }
}

export const gClip = {

        isCopy: false,

        //content: new Array(),
        content: [],

        reset: function () {
            this.content = [];
        },

        add: function () {
            this.reset();

            var that = this;

            this.append.apply(that, arguments);
        },

        append: function () {
            for (var i = 0; i < arguments.length; ++i) {
                var argument = arguments[i];
                if (argument instanceof Array && (arguments.length === 1)) {
                    for (var r = 0; r < argument.length; ++r) {
                        this.content.push(argument[r]);
                    }
                } else
                    this.content.push(arguments[i]);
            }
        },

        copy: function (src) {
            console.log('I made a copy');
            this.isCopy = true;

            this._src = src;

            var that = this;

            var args = [].splice.call(arguments, 0);
            this.add.apply(that, args.splice(1));

            this.updateDependents();
        },

        cut: function (src) {
            console.log('I made a cut');
            this.isCopy = false;
            this._src = src;

            var that = this;

            var args = [].splice.call(arguments, 0);
            this.add.apply(that, args.splice(1));

            this.updateDependents();
        },

        paste: async function (dst, objCls) {
            console.log('paste works');
            var url = "?cmdr=";
            if (this.isCopy) {
                url += "objCopy";
            } else {
                url += "objCut";
            }
            if (this._src) {
                url += "&src=" + this._src;
            }else{
                url += "&src=" + 0;
            }
            url += "&ids=" + this.content.join(",");
            url += "&dest=" + dst;

            if (!this.isCopy) {
                this.reset();
            }
            const response = await linkURL(url, "ajax");
            return response;

        },

        _delete: async function (src, objCls, ids) {
            console.log('_delete works');
            const hideIds = ids.split(',');
            hideItem(hideIds);
            let url = "?cmdr=objRemove&src=";
            if (src) {
                url += src;
            } else {
                url += "root";
            }
            url += "&ids=" + ids;
            return await linkURL(url, "ajax");
            //ids = ids.split(',');
        },

        updateDependents: function () {
             return;
        }

    }