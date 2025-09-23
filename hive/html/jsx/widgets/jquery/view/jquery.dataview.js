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
    $.widget("view.dataview", {

        options: {
            dataViewer: null,
            dataViewerOptions: null,
            instance: null,
            preload: false,
        },

        _create: function () {
            var oThis = this;

            if (this.options.dataViewer != null) {
                this.view = new window[this.options.dataViewer](this.options.dataViewerOptions);
            }
            else if(this.options.instance != null) {
                this.view = this.options.instance;
            }

            this.dataViews = [];
            
            $(verarr(this.view)).each(function(index, view) {
                var divId ='DV_' + Math.round(Math.random() * 10000000);
                if(view.divName)
                    divId = view.divName;

                var viewDiv = $(document.createElement('div')).attr({ id: divId }).css({ width: '100%' });

                if (view.options) {
                    if (view.options.changeWidth || view.options.changeWidth == undefined)
                        view.options.width = oThis.element.width();
                    if (view.options.changeHeight || view.options.changeHeight == undefined)
                        view.options.height = oThis.element.height();
                    if(view.downloadLinkManage) {
                        view.options.height -= 20;
                    }
                    
                    if(view.options.isVisible == true){
                        viewDiv.css("height", "100%");
                    }
                    else if(view.options.isVisible == false){
                        viewDiv.css("visibility", "hidden");
                        viewDiv.css("height", "0");
                    }
                }
                else if(view.hasOwnProperty('width')) {
                    if (view.changeWidth || view.changeWidth == undefined)
                        view.width = oThis.element.width();
                    if (view.changeHeight || view.changeHeight == undefined)
                        view.height = oThis.element.height();
                }

                view.container = viewDiv.attr('id');
                view.dataSourceEngine = vjDS;
                view.dataViewEngine = vjDV;
                
                $(verarr(view.data)).each(function(ii, data){ vjDS[data].refreshOnEmpty = true; })

                if(view.register_callback != null)
                    view.register_callback();

                oThis.element.append(viewDiv);

                vjDV[divId] = view;
                oThis.dataViews.push(view);
                
                vjDV[divId].widgetCallbackRendered = oThis._onRenderedCallback;
                view.maxAreaHeight = oThis.element.height();
                view.dataWidget = oThis;
                
                if(oThis.element.is(':visible') || oThis.options.preload) {
                    oThis.element.data({ rendered: true});
                    
                    if(vjDV[divId].load != null)
                        vjDV[divId].load();
                    if(vjDV[divId].render != null)
                        vjDV[divId].render();
                }
                else {
                    viewDiv.data({ rendered: false});
                }

                $('#' + divId + ':visible').livequery(function() {
                    oThis._refresh();
                });

                viewDiv.bind('destroyed', function() {
                    delete vjDV[divId];
                });
            });

            this.element.closest('.layout-area').on('area-resize', function (event, params) {
            });

            this.element.closest('.layout-area').on('area-resize-stop', function (event, params) {
                oThis._refresh();

            });
            this.totalViews = this.dataViews.length;            
        },

        _refresh: function() {
            var oThis = this;


            if(!this.element.is(':visible') && !this.options.preload)
                return;

            if(!this.element.data('rendered')) {
                this.element.data({ rendered: true});

                for (var index = 0; index < this.dataViews.length; index++){
                    var view = this.dataViews[index];
                    
                    if(view.hasOwnProperty('width')) {
                        size = { 
                            width: view.width, 
                            height: view.height
                        };
                    }
                    else if(view.options != null && view.options.hasOwnProperty('width')) {
                        size = {
                            width: view.options.width, 
                            height: view.options.height
                        };
                    }
                    else if(view.constructor.name == 'vjSVGView') {
                        size = {
                            width:0,
                            height:0
                        };
                        $.extend(view.geometry, {
                            width: this.element.width()
                        });
                    }

                    if (view.options)
                        $.extend(view.options, {
                            width: this.element.width(),
                            height: this.element.height()
                        });
                    $.extend(view, {
                        width: this.element.width(),
                        height: this.element.height()
                    });

                    if(view.downloadLinkManage) {
                        if (view.downloadLink && view.downloadLink.getBoundingClientRect) {
                            if (view.options)
                                view.options.height -= view.downloadLink.getBoundingClientRect().height + 3;
                            else
                                view.height -= view.downloadLink.getBoundingClientRect().height + 3;
                        }
                        else {
                            if (view.options)
                                view.options.height -= 20;
                            else
                                view.height -= 20;
                        }
                    }

                    if(view.load != null)
                        view.load();
                    if(view.render != null)
                        view.render();
                };
            }
            else {
                var viewArr = verarr(this.view);
                var totalHeight = this.element.height();
                for (var kk = 0; kk < viewArr.length; kk++)
                {
                    var vView = viewArr[kk];
                    if (!vView) continue;

                    var size = null;

                    if(vView.hasOwnProperty('width')) {
                        size = { 
                            width: vView.width, 
                            height: vView.height
                        };
                    }
                    if(vView.options != null && vView.options.hasOwnProperty('width')) {
                        size = {
                            width: vView.options.width, 
                            height: vView.options.height
                        };
                    }
                    else if(vView.constructor.name == 'vjSVGView') {
                        size = {
                            width:0,
                            height:0
                        };
                    }

                    if(size == null || !vView.div)
                        continue;

                    if(this.element.is(':visible') && (size.width != this.element.width() || size.height != this.element.height())) {
                        if (vView.options)
                            $.extend(vView.options, {
                                width: this.element.width(),
                                height: this.element.height()
                            });
                        if (vView.resize_info)
                            $.extend(vView.resize_info, {
                                newW: this.element.width(),
                                newH: this.element.height()
                            });
                        
                        $.extend(vView, {
                            width: this.element.width(),
                            height: this.element.height()
                        });
                        
                        if((vView.downloadLinkManage || vView.downloadSvg) && vView.options) {
                            if (vView.downloadLink && vView.downloadLink.getBoundingClientRect) {
                                vView.options.height -= vView.downloadLink.getBoundingClientRect().height + 3;
                                vView.height -= vView.downloadLink.getBoundingClientRect().height + 3;
                            } else {
                                vView.options.height -= 20;
                                vView.height -= 20;
                            }
                        }

                        if (vView.options && vView.options.changeWidth == false)
                            $.extend(vView.options, {
                                width: size.width
                            });
                        if (vView.options && vView.options.changeHeight == false)
                            $.extend(vView.options, {
                                height: size.height
                            });
                        if (vView.changeWidth == false)
                            $.extend(vView, {
                                width: size.width
                            });
                        if (vView.changeHeight == false)
                            $.extend(vView, {
                                height: size.height
                            });
                    }                    
                    if (vView.nonSticky === false)
                        vView.height = totalHeight;
                    
                    if (vView.refresh) vView.refresh();
                    if (vView.nonSticky === undefined || vView.nonSticky)
                        totalHeight -= $("#"+vView.container).height();
                    oThis._onRenderedCallback(vView);
                }
            }
        },
        _onRenderedCallback: function (view){
            var dv = view.dataWidget;
            if (!dv) return;
            dv.totalViews--;
            
            if (dv.totalViews <= 0){
                var dataViews = dv.dataViews;
                
                var nonSticky = [];
                var heightSticky = 0;
                
                for (var i = 0; i < dataViews.length; i++){
                    if ((dataViews[i].options && dataViews[i].options.changeHeight == false) || 
                            (dataViews[i].nonSticky || (dataViews[i].options && dataViews[i].options.nonSticky)))
                        nonSticky.push(dataViews[i]);
                    else
                        heightSticky += $("#"+dataViews[i].container).height();
                }
                
                var heightLeft = dv.element.height() - heightSticky;
                var heightPerDV = Math.round(heightLeft/nonSticky.length)-2;
                
                for (var i = 0; i < nonSticky.length; i++){
                    if(dataViews[i].options && dataViews[i].options.changeHeight == false)
                        continue;
                    $("#"+nonSticky[i].container + "_table_div").css("height", heightPerDV);
                    if (nonSticky[i].stickyHeader) 
                        nonSticky[i].stickyHeader();
                }
            }
        }
    });

}(jQuery));

(function($) {
  $.event.special.destroyed = {
    remove: function(o) {
      if (o.handler) {
        o.handler()
      }
    }
  }
})(jQuery)
