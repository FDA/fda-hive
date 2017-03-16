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
    $.widget("layout.infobox", {

        options: {
            draggable: {
                handle: '.panel-heading',
                cursor: 'move',
                containment: 'parent',
                grid: [20, 20]
            },
            droppable: {
                greedy: true
            },
            allowMaximize: false,
            allowClose: false,
            allowHide: false,
            allowSplit: true
        },

        _create: function () {
            var oThis = this;

            this.infobox = this.element.children(':not(.ui-resizable-handle)').first();

            this._loadOptions();

            if (this.infobox.length == 0) {
                this.infobox = $(document.createElement('div'))
                                .appendTo(this.element);
            }

            this.infobox.addClass("layout-infobox");

            this.margin = (this.infobox.outerWidth(true) - this.infobox.outerWidth()) / 2;
            this.border = (this.infobox.outerWidth() - this.infobox.innerWidth()) / 2;

            this.area = this.element.area('instance');

            this.manager = this.element.parent().layoutmanager('instance');

            this.adjustHeight();

            this.element.on('area-resize', function (event, params) {
                oThis.adjustHeight();
            });

            this.createToolbar();

            this._initDraggable();

            this._initDroppable();

            this.onInfoboxInit();

            this._processKeyboard();

            if(this.options.allowSplit)
                this._initSplitAreas();

            this.sendEvent('infobox-create');

            this.element.on('maximize', function (event, params) {
                oThis.element.children('.toggler').hide();
                
                oThis.element.children('.ui-resizable-handle').hide();
            });

            this.element.on('minimize', function (event, params) {
                oThis.element.children('.toggler').show();

                oThis.element.children('.ui-resizable-handle').show();
            });
        },

        _initDraggable: function () {
            var oThis = this;

            if (this.options.draggable) {
                $(this.options.draggable.handle, this.element).addClass('draggable');

                var options = $.extend(this.options.draggable, {
                    start: function (event, ui) {
                        oThis.area.setFocus();
                        oThis.area.sendEvent('area-drag-start');
                    },
                    drag: function () {
                        oThis.area.sendEvent('area-drag-move');
                    },
                    stop: function () {
                        oThis.area.sendEvent('area-drag-stop');
                    }
                });

                this.element
                    .draggable(options)
                    .click(function () {
                        oThis.area.setFocus();
                    });
            }
        },

        _initDroppable: function () {
            var oThis = this;

            if (this.options.droppable) {
                var options = $.extend(this.options.droppable, {
                    accept: function(elem) {
                        if($(elem).closest('div.layout-area.maximize').length > 0) {
                            return false;
                        }

                        return $(elem).is('.layout-infobox .nav-tabs > li.ui-draggable');
                    },
                    over: function (event, ui) {
                        //    if we try to move tab from the infobox where it belongs to, we don't have to show north and center drop areas...
                          var yourself = $.contains(oThis.element.get(0), ui.draggable.get(0));
                        oThis._showSplitAreas(yourself);
                    },
                    out: function (event, ui) {
                        oThis._hideSplitAreas();
                    }
                });

                this.element.droppable(options);
            }
        },

        _initSplitAreas: function () {
            var oThis = this;

            //    check that infobox is not inside another infobox... 
            //    for now we support split mode only for first level infoboxes
            if(this.element.closest('.layout-infobox').length > 0)
                return;

            if (this.options.droppable) {
                this.infobox.append(
                    $(document.createElement('div'))
                        .addClass('split-area center-split-area invisible')
                        .data({
                            'area-name': 'center'
                        })
                );
                this.infobox.append(
                    $(document.createElement('div'))
                        .addClass('split-area north-split-area invisible')
                        .data({
                            'area-name': 'north'
                        })
                );
                this.infobox.append(
                    $(document.createElement('div'))
                        .addClass('split-area west-split-area invisible')
                        .data({
                            'area-name': 'west'
                        })
                );
                this.infobox.append(
                    $(document.createElement('div'))
                        .addClass('split-area east-split-area invisible')
                        .data({
                            'area-name': 'east'
                        })
                );
                this.infobox.append(
                    $(document.createElement('div'))
                        .addClass('split-area south-split-area invisible')
                        .data({
                            'area-name': 'south'
                        })
                );

                $('.split-area', this.infobox).droppable({
                    accept: function(elem) {
                        if($(elem).closest('div.layout-area.maximize').length > 0) {
                            return false;
                        }

                        return $(elem).is('.layout-infobox .nav-tabs > li.ui-draggable');
                    },
                    over: function (event, ui) {
                        $(this).addClass('active');
                    },
                    out: function (event, ui) {
                        $(this).removeClass('active');
                    },
                    drop: function (event, ui) {
                        oThis._dropHandler($(this).data('area-name'), this, ui);
                    }
                });
            }
        },

        _showSplitAreas: function (yourself) {
            if(!yourself) {
                $('.split-area', this.infobox).removeClass("invisible");
            }
            else {
                $('.split-area.west-split-area,.split-area.east-split-area,.split-area.south-split-area', this.infobox).removeClass("invisible");
            }
        },

        _hideSplitAreas: function () {
            $('.split-area', this.infobox).addClass("invisible");
        },

        _dropHandler: function(direction, area, ui) {
            this._hideSplitAreas();

            this.sendEvent('state-changed');
        },

        _loadOptions: function () {
            //  load options from HTML data attributes...
            if ($(this.infobox).is('[data-allow-maximize]'))
                this.options.allowMaximize = $(this.infobox).data('allow-maximize');
            if ($(this.infobox).is('[data-allow-close]'))
                this.options.allowClose = $(this.infobox).data('allow-close');
            if ($(this.infobox).is('[data-allow-hide]'))
                this.options.allowHide = $(this.infobox).data('allow-hide');
            if ($(this.infobox).is('[data-allow-split]'))
                this.options.allowSplit = $(this.infobox).data('allow-split');
            if ($(this.infobox).is('[data-draggable]'))
                this.options.draggable = $(this.infobox).data('draggable');
            if ($(this.infobox).is('[data-droppable]'))
                this.options.droppable = $(this.infobox).data('droppable');
        },

        _processKeyboard: function() {
            var oThis = this;

            this.element.click(function() {
                oThis.element.focus();
            });

            this.element.keyup(function(e) {
                if (e.keyCode == 27) {
                    oThis.minimize();
                }
            });
        },

        onInfoboxInit: function() {
        },

        createToolbar: function () {
            var oThis = this;

            this.toolbar = $(document.createElement('div'))
                                .addClass('infobox-toolbar');

            if (this.options.allowMaximize) {
                this.maxMinButton = $(document.createElement('button'))
                    .attr({
                        type: 'button',
                        title: 'Maximize'
                    })
                    .addClass('btn btn-default btn-xs')
                    .append(
                        $(document.createElement('span'))
                            .addClass('glyphicon glyphicon-resize-full')
                    )
                    .click(function (event) {
                        if ($('span', this).is('.glyphicon-resize-full')) {
                            oThis.maximize();
                        }
                        else if ($('span', this).is('.glyphicon-resize-small')) {
                            oThis.minimize();
                        }

                        event.stopPropagation();
                    });

                this.toolbar.append(this.maxMinButton);
            }

            if (this.options.allowClose) {
                this.closeButton = $(document.createElement('button'))
                    .attr({
                        type: 'button',
                        title: 'Close'
                    })
                    .addClass('btn btn-default btn-xs')
                    .append(
                        $(document.createElement('span'))
                            .addClass('glyphicon glyphicon-remove-circle')
                    )
                    .click(function (event) {
                        oThis.close();
                        event.stopPropagation();
                    });

                this.toolbar.append(this.closeButton);
            }

            if (this.options.allowHide) {
                this.hideButton = $(document.createElement('button'))
                    .attr({
                        type: 'button',
                        title: 'Hide'
                    })
                    .addClass('btn btn-default btn-xs')
                    .append(
                        $(document.createElement('span'))
                            .addClass('glyphicon glyphicon-remove-circle')
                    )
                    .click(function (event) {
                        oThis.hide();
                        event.stopPropagation();
                    });

                this.toolbar.append(this.hideButton);
            }
            
            if($('.panel-heading', this.infobox).length > 0)
                this.toolbar.appendTo($('.panel-heading', this.infobox));
            else
                this.toolbar.appendTo(this.infobox);
        },

        // events bound via _bind are removed automatically
        // revert other modifications here
        _destroy: function () {
            this.element.empty();
        },

        sendEvent: function (name, params) {
            if (params == null)
                params = {};

            $.extend(params, {
                oThis: this,
                infobox: this.element,
                rect: this.area.getRect()
            });

            if (name == 'infobox-drop') {
                $(this.element).trigger('infobox-drop', params);
            }
            else if (name == 'infobox-create') {
                $(this.element.parent()).trigger('infobox-create', params);
            }
            else if (name == 'state-changed') {
                $(this.element).trigger('state-changed', params);
            }
            else if (name == 'split-area') {
                $(this.element).trigger('split-area', params);
            }
            else if (name == 'maximize') {
                $(this.element).trigger('maximize', params);
            }
            else if (name == 'minimize') {
                $(this.element).trigger('minimize', params);
            }
            else {
                this.area.sendEvent(name, params);
            }
        },

        adjustHeight: function () {
            //    set absolute height...
            this.infobox.height(this.area.height() - 2 * (this.margin + this.border));
            this.infobox.width(this.area.width() - 2 * (this.margin + this.border));
        },

        height: function () {
            return this.infobox.height();
        },

        width: function () {
            return this.infobox.width();
        },
        
        maximize: function () {
            this.maxMinButton.attr({title: 'Minimize'});
            $('span', this.maxMinButton).removeClass('glyphicon-resize-full').addClass('glyphicon-resize-small');

            this.sendEvent('maximize');
            
            this.element.addClass('maximize');
            
            this.element.area('sendEvent', 'area-resize')
                        .area('sendEvent', 'area-resize-stop');
            
            this.area.setFocus();
        },

        minimize: function () {
            this.maxMinButton.attr({title: 'Maximize'});
            $('span', this.maxMinButton).removeClass('glyphicon-resize-small').addClass('glyphicon-resize-full');

            this.sendEvent('minimize');
            
            this.element.removeClass('maximize');

            this.element.area('sendEvent', 'area-resize')
                        .area('sendEvent', 'area-resize-stop');

            this.area.setFocus();
        },

        close: function() {
            this.manager.remove(this.infobox.data('id'));
        },

        hide: function() {
            this.manager.hide(this.infobox.data('id'));
        },
        
        save: function () {
            var config = this.area.save();

            config.allowMaximize = this.options.allowMaximize;
            config.allowClose = this.options.allowClose;
            config.allowHide = this.options.allowHide;
            config.allowSplit = this.options.allowSplit;

            return config;
        },

        getOptions: function () {
            var options = this.area.getOptions();

            $.extend(options, {
                allowMaximize: this.options.allowMaximize,
                allowClose: this.options.allowClose,
                allowHide: this.options.allowHide,
                allowSplit: this.options.allowSplit
            });

            return options;
        },
        
        _initWidgets: function() {
            var oThis = this;
            
            $('div[data-need-init-widget=1]', this.element).each(function() {
                var panel = this;
                
                if ($(panel).data('jquery-widget')) {
                    var jQueryWidget = $(panel).data('jquery-widget');
                    var jQueryWidgetOptions = null;
                    if ($(panel).data('jquery-widget-options')) {
                        jQueryWidgetOptions = $(panel).data('jquery-widget-options');
                    }

                    //try {
                        $(panel)[jQueryWidget](jQueryWidgetOptions);

                        var widget = $(panel)[jQueryWidget]('instance');
                        if ($.isFunction(widget.setSize)) {
                            widget.setSize($(panel).parent().width(), $(panel).parent().height());

                            oThis.element.on('area-resize', function (event, params) {
                                widget.setSize($(panel).parent().width(), $(panel).parent().height());
                            });
                        }
                    /*}
                    catch(e) {
                        console.log('ERROR: cannot initiate widget: ' + jQueryWidget + '. Message: ' + e.message);
                        console.log(jQueryWidgetOptions);
                    }*/
                }

                $(panel).removeAttr('data-need-init-widget');
            });
        }
    });

}(jQuery));

function parseInfoboxOptions(element) {
    var options = parseAreaOptions(element);

    if ($(element).is('[data-draggable]'))
        options.draggable = $(element).data('draggable');

    if ($(element).is('[data-droppable]'))
        options.droppable = $(element).data('droppable');

    if ($(element).is('[data-allow-maximize]'))
        options.allowMaximize = $(element).data('allow-maximize');

    if ($(element).is('[data-allow-close]'))
        options.allowClose = $(element).data('allow-close');

    if ($(element).is('[data-allow-hide]'))
        options.allowHide = $(element).data('allow-hide');

    return options;
}
