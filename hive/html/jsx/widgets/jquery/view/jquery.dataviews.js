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
    $.widget("view.dataviews", {

        options: {
            id: null,
            layoutType: 'stack',
            orientation: 'vertical',
            border: 'none',
            overflowY: 'hidden',
            overflowX: 'hidden',
            onAreaClose: function(params) {
            },
            onAreaHide: function(params) {
            },
            views: []    //    array of HIVE data views,
        },

        _create: function () {
            var oThis = this;

            //    if we don't have anything to render...
            if(this.options.views == null || this.options.views.length == 0)
                return;

/*Commented out because tabs would not switch panels. Since the ID of the tab does not match the ID of panel           
 * if(this.options.id)
                this.element.attr({ id: this.options.id });*/

            this.panel = $(document.createElement('div'))
                            .css({
                                'overflow-y': this.options.overflowY,
                                'overflow-x': this.options.overflowX
                            })
                            .appendTo(this.element);

            this._transform();

            var items = [];

            //    process views' size attribute and pre-calculate if necessary...
            this._processSizes();

            $(this.options.views).each(function(index, view) {
                items.push({
                    id: view.id != null ? view.id : oThis._randomId(),
                    size: view.size,
                    overflow: view.overflow,
                    allowClose: view.allowClose,
                    allowHide: view.allowHide,
                      view: {
                        name: 'dataview',
                        options: {
                            instance: view.instance,
                            dataViewer: view.dataViewer,
                            dataViewerOptions: view.dataViewerOptions
                        }
                    }
                  });
            });

            this.panel.layoutmanager({
                type: this.options.layoutType,
                orientation: this.options.orientation,
                border: this.options.border,
                config: {
                      layout: {
                          layoutType: this.options.layoutType,
                          items: items
                    }
                }
            })
            .on('area-closed', function(event, params) {
                if(oThis.options.onAreaClose)
                    oThis.options.onAreaClose(params);
                    
                //console.log('area closed', event, params);
            })
            .on('area-hide', function(event, params) {
                //console.log('area hidden', event, params);

                if(oThis.options.onAreaHide)
                    oThis.options.onAreaHide(params);
            });

            this.manager = this.panel.layoutmanager('instance');
        },

        _randomId:function() {
            return 'dataview_' + Math.floor((Math.random() * 1000000) + 1);
        },
        
        _transform: function() {
            var oThis = this;
            
            $(this.options.views).each(function(index, view) {
                //    presume we got HIVE data view object and need to transfer it to the appropriate format...
                if(view.constructor.name != 'Object') {
                    oThis.options.views[index] = { instance: view };
                }
            });
        },

        _processSizes: function() {
            var oThis = this;
            
            var percents = 0;
            var pixels = 0;
            var hasStar = false;
            var numberWithoutSize = 0;
            
            $(this.options.views).each(function(index, view) {
                if (typeof view.size == 'string') {
                    if(view.size.endsWith('%')) {
                        var percent = parseFloat(view.size);
                        if (!isNaN(percent))
                            percents += percent;
                    }
                    else if(view.size == '*') {
                        hasStar = true;
                    }
                    else {
                        var px = parseFloat(view.size);
                        if (!isNaN(px))
                            pixels += px;
                    }
                }
                else if(view.size == null) {
                    numberWithoutSize++;
                }
            });

            var totalSize = this.isVertical() ? this.element.height() : this.element.width();
            var percentsFree = 100 - percents - (pixels * 100 / totalSize).toFixed(2);

            $(this.options.views).each(function(index, view) {
                if(view.size == null) {
                    oThis.options.views[index].size = (percentsFree / numberWithoutSize).toFixed(2) + '%'; 
                }
            });
        },

        isVertical: function() {
            return this.options.layoutType == 'vertical';
        },

        add: function(view) {

            if(view.constructor.name != 'Object') {
                view = { instance: view };
            }

            var size = this._getHiveDataViewSize(view.instance);
            
            this.manager.append({
                layout: {
                    items:[
                    {
                        id: view.id != null ? view.id : this._randomId(),
                        size: view.size,
                        overflow: view.overflow,
                        allowClose: view.allowClose,
                        allowHide: view.allowHide,
                        onClose: view.onClose,
                        onHide: view.onHide,
                          view: {
                            name: 'dataview',
                            options: {
                                instance: view.instance,
                                dataViewer: view.dataViewer,
                                dataViewerOptions: view.dataViewerOptions
                            }
                        }
                    }]
                }
            });
        },

        _getHiveDataViewSize: function(view) {
            var size = null;
            if(view != null) {
                if(view.hasOwnProperty('width') && view.hasOwnProperty('height')) {
                    size = {
                        width: view.width,
                        height: view.height
                    };
                }
                else if(view.options != null && view.options.hasOwnProperty('width') && view.options.hasOwnProperty('height')) {
                    size = {
                        width: view.options.width,
                        height: view.options.height
                    };
                }
            }

            return size;
        },

        insert: function(index, config) {
            console.log('insert new view...')
        }
    });

}(jQuery));
