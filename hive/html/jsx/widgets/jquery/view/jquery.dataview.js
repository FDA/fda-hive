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
                        view.options.height -= 20; // should be enough usually to draw download link div; this.view.downloadLink doesn't exist yet 
                    }
                }
                else if(view.hasOwnProperty('width')) {
                    if (view.changeWidth || view.changeWidth == undefined)
                        view.width = oThis.element.width();
                    if (view.changeHeight || view.changeHeight == undefined)
                        view.height = oThis.element.height();
                }
                //vjDV[divId] = allViews[i];

                view.container = viewDiv.attr('id');
                view.dataSourceEngine = vjDS;
                view.dataViewEngine = vjDV;
                
                $(verarr(view.data)).each(function(ii, data){ vjDS[data].refreshOnEmpty = true; })

                if(view.register_callback != null)
                    view.register_callback();

                oThis.element.append(viewDiv);

                vjDV[divId] = view;
                oThis.dataViews.push(view);

                if(oThis.element.is(':visible') || oThis.options.preload) {
                    oThis.element.data({ rendered: true});

                    if(vjDV[divId].load != null)
                        vjDV[divId].load();
                    if(vjDV[divId].render != null)
                        vjDV[divId].render();
                }
                else {
                    //    mark DIV as not rendered yet... 
                    //    it will be rendered as soon as becomes visible...
                    viewDiv.data({ rendered: false});
                }

                //    try to refresh widget if it becomes visible 
                $('#' + divId + ':visible').livequery(function() {
                    oThis._refresh();
                });

                viewDiv.bind('destroyed', function() {
                    delete vjDV[divId];
                });
            });

            this.element.closest('.layout-area').on('area-resize', function (event, params) {
                event.stopPropagation();
            });

            this.element.closest('.layout-area').on('area-resize-stop', function (event, params) {
                oThis._refresh();

                event.stopPropagation();
            });
        },

        _refresh: function() {
            var oThis = this;

            //console.log('refresh');

            if(!this.element.is(':visible') && !this.options.preload)
                return;

            //    if dataView is not rendered yet we have to load and render it first
            if(!this.element.data('rendered')) {
                this.element.data({ rendered: true});

                //If the jquery style is used here, then there is no way to access the this(oThis), which is required for the extraction of width and height
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
                        //    if we work with SVG Viewer we should resize it everytime...
                        size = {
                            width:0,
                            height:0
                        };
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
                                view.options.height -= 20; // should be enough usually to draw download link div
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
                        //    if we work with SVG Viewer we should resize it everytime...
                        size = {
                            width:0,
                            height:0
                        };
                    }

                    if(size == null || !vView.div)
                        continue;    //    we don't have anything to refresh...

                    //    refresh if element is visible and the size has changed
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
                                vView.options.height -= 20; // should be enough usually to draw download link div
                                vView.height -= 20;
                            }
                        }

                        //This is needed for graphs that should not take up the entire area. Ex: Heptagon results pages
                        if (vView.options && vView.options.changeWidth == false)
                            $.extend(vView.options, {
                                width: size.width
                            });
                        if (vView.options && vView.options.changeHeight == false)
                            $.extend(vView.options, {
                                height: size.height
                            });
                    }                    

                    vView.refresh();
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
