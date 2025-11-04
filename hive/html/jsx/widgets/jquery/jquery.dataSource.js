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

(function($){
    $.widget("hive.dataSource", {
        dataSources:{
            
        },
        options: {
            
        },
        _create: function(){            
            this.element.addClass("dataSource");
        },
        addDS: function (title, name, url){
            vjDS.add(title, name, url);
            vjDS[name].register_callback(this.loaded);
            
            vjDSNew.dataSources[name] = {widgets:[]};
        },
        loadAll: function(){
            this.options.allWidgets = [];
            
            for(var ds in vjDSNew.dataSources){
                for (var i=0; i < this.dataSources[ds].widgets.length; i++){
                    var position = this._findWidget(this.dataSources[ds].widgets[i], this.options.allWidgets);
                    if (position == -1)
                        this.options.allWidgets.push ({widget: this.dataSources[ds].widgets[i], total: 1});
                    else
                        this.options.allWidgets[position].total++;
                }
                
                vjDS[ds].load();
            }
        },
        registerWidget: function (name, widget){
            vjDSNew.dataSources[name].widgets.push(widget);
        },
        loaded: function (dsStruct, content){
            for (var i = 0; vjDSNew.dataSources[this.name].widgets && i < vjDSNew.dataSources[this.name].widgets.length; i++){
                var widget = vjDSNew.dataSources[this.name].widgets[i];
                var pos = vjDSNew._findWidget (widget, vjDSNew.options.allWidgets);
                if (pos >= 0)
                    vjDSNew.options.allWidgets[pos].total --;
                if (vjDSNew.options.allWidgets[pos].total <= 0)
                    widget.draw(widget, this, content);
            }
        },
        _findWidget: function (toCheck, arr){
            for (var i = 0; i < arr.length; i++){
                if (this._checkWidget(toCheck, arr[i].widget))
                    return i;
            }
            return -1;
        },
        _checkWidget: function (widget1, widget2){
            return widget1.options.container == widget2.options.container;
        }
    });    
} (jQuery));
    