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
//# sourceURL=jquery.ion.base.js

$(function () {
    
    $.Ion = function () {
        return {

            isReady: false,

            options: {
                //url: '/usr/alexp/i.cgi?cmd=brgetjson&sub=$root&raw=1'
                url: '/usr/alexp/jsx/widgets/jquery/ion/data/system.json'
                //url: '/data/system.json'
            },

            onReady: [],

            ready: function(callback) {
                if(this.isReady) {
                    callback();
                }
                else {
                    this.onReady.push(callback);
                }
            },
            
            init: function() {
                var that = this;
                
                $.getJSON(this.options.url, function(json) {
                    that._prepare(json);
                })
                .fail(function(jqxhr, textStatus, error) {
                    console.log( "Error: can't get data from " + that.options.url + '\n' + error.toString() );
                });
            },
            
            _prepare: function(json) {
                this.core = {};
                    
                for(var key in json.primitives) {
                    if(json.primitives[key].hasOwnProperty('_type')) {
                        this.core[key] = json.primitives[key];
                    }
                }
                this.types = {};

                for(var key in json.types){
                    if(json.types[key].hasOwnProperty('_type') && json.types[key]._type == 'type') {
                        this.types[key] = json.types[key];
                    }
                }

                this.isReady = true;

                while(this.onReady.length > 0) {
                    var callback = this.onReady.pop();
                    if(callback)
                        callback();
                }
            },

            isCore: function(name) {
                return this.core.hasOwnProperty(name);
            },
            
            hasType: function(name) {
                return this.types.hasOwnProperty(name);
            },
            
            getType: function(name) {
                return !this.types.hasOwnProperty(name) ? null : this.types[name];
            },
            
            lookup: function(id, callback) {
                //var lookup_url = this.options.url + '&brSearch=_id~' + id + '%26%26~';
                var lookup_url = this.options.url + '?timestamp=' + (new Date().getTime());

                $.getJSON(lookup_url, function(json) {
                    var res = JSON.find(json, '_id', id);
                    
                    if(res.length > 0 && callback != null) {
                        callback(res[0]);
                    }
                })
                .fail(function(jqxhr, textStatus, error) {
                    console.error( "can't get data from " + lookup_url + '\n' + error.toString() );
                });
            }

        };
    };
}(jQuery));

$.ion = $.Ion();
$.ion.init();