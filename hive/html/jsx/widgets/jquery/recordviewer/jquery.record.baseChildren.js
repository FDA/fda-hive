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
$(function () {
    $.widget("recordviewer.record_children", $.recordviewer.record_base, {
        children: {},
        options: {
                hasChildren: true
        },
        
        
        appendCorrectChild: function (div, applyTo, spec){
            var name = spec.tmpObjName;
            if(spec._type == 'string') {
                div.record_string({
                    name: name,
                    spec: spec
                });
                this.children[name] = div.data("recordviewer-record_string");
            }
            else if(spec._type == 'integer' || spec._type == 'int' || spec._type == "real") {
                div.record_integer({
                    name: name,
                    spec: spec
                });
                this.children[name] = div.data("recordviewer-record_integer");
            }
            else if(spec._type == 'text') {
                div.record_text({
                        name: name,
                        spec: spec
                    });
                this.children[name] = div.data("recordviewer-record_text");
            }
            else if(spec._type == 'datetime') {
                div.record_datetime({
                        name: name,
                        spec: spec
                    });
                this.children[name] = div.data("recordviewer-record_datetime");
            }
            else if(spec._type == 'real') {
                div.record_real({
                        name: name,
                        spec: spec
                    });
                this.children[name] = div.data("recordviewer-record_real");
            }
            else if(spec._type == 'bool') {
                div.record_bool({
                        name: name,
                        spec: spec
                    });
                this.children[name] = div.data("recordviewer-record_bool");
            }
            else if(spec._type == 'password') {
                div.record_password({
                        name: name,
                        spec: spec
                    });
                this.children[name] = div.data("recordviewer-record_password");
            }
            else if(spec._type == 'list' || spec._layout == 'struct') {
                var ul = $(document.createElement('ul'));                
                ul.appendTo(applyTo)
                    .record_list({
                        name: name,
                        spec: spec,
                        children: spec._field
                    });
                this.children[name] = ul.data("recordviewer-record_list");
            }
            else if(spec._type == 'array' || spec._layout == "table") {
                var table = $(document.createElement('table'));
                table.appendTo(applyTo)
                    .record_array({
                        name: name,
                        spec: spec
                    });
                this.children[name] = table.data("recordviewer-record_array");
            }
            else {
                div.record_base({
                        name: name,
                        spec: spec
                    });
                this.children[name] = div.data("recordviewer-record_base");
            }
        },
        
        updateFields: function(fieldDesc){
            var oThis = this;
            this.fieldDesc = fieldDesc;
            
            jQuery.each(this.fieldDesc, function(key, value){
                if(oThis.children[key]){
                    if(oThis.children[key].eventNamespace.indexOf("list") > -1){
                        oThis.children[key].updateFields (value);
                    }
                    else if(oThis.children[key].eventNamespace.indexOf("array") > -1){
                        oThis.children[key].updateFields (value);
                    }
                    else
                        oThis.children[key].setValue(value);
                }
            });
        }
    });

}(jQuery));
