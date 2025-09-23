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

(function ($) {
    $.widget("layout.layoutmanager", {

        options: {
            type: 'flex',
            orientation: 'vertical',
            saveState: false,
            resizable: false,
            config: null,
            bind: null,
            border: 'all',
            margin: 'all',
            allowResize: true,
            roundup: 20,
            allGrids:{}
        },

        _create: function () {
            var oThis = this;

            this.resizedAreas = [];

            this._onBeforeInit();

            this._loadOptions();

            this.element.addClass('layout-manager');

            if(this.isStackLayout()) {
                if(this.options.orientation == 'vertical') {
                    this.element.css({ 'overflow-y': 'auto' });
                }
                else if(this.options.orientation == 'horizontal') {
                    this.element.css({ 'overflow-x': 'auto' });
                }
            }
            else {
                this.element.parent().css({ overflow: 'hidden' });
            }

            this.areaWid = this.options.layoutType != "gridstackArea" ? "area" : "gridstackArea";
                
            if (!this.element.is('.layout-area')) {
                this.element[this.areaWid]({
                    rect: {
                        top: 0,
                        left: 0,
                        right: this.element.parent().outerWidth(),
                        bottom: this.element.parent().outerHeight()
                    },
                    resizable: false
                });
            }

            this.area = this.element[this.areaWid]('instance');

            this.element.on('area-resize', function (event, params) {
                if ($(event.target).is(this)) {
                    $(this).children('div.layout-area').each(function () {
                        var areaWid = "area";
                        if ($(this).hasClass("layout-gridArea"))
                            areaWid = "gridstackArea";
                        
                        var area = $(this)[areaWid]('instance');
                        area.setRect(oThis.calculateAreaRect(this));
                        area.sendEvent('area-resize');
                    });
                }

                event.stopPropagation();
            });

            this.element.on('area-resize-stop', function (event, params) {
                if ($(event.target).is(this)) {
                    $(this).children('div.layout-area').each(function () {
                        var areaWid = "area";
                        if ($(this).hasClass("layout-gridArea"))
                            areaWid = "gridstackArea";
                        
                        var area = $(this)[areaWid]('instance');
                        area.sendEvent('area-resize-stop');
                    });
                }

                event.stopPropagation();
            });

            if(this.isVerticalLayout() || this.isHorizontalLayout() || this.isStackLayout()) {
                this.element.on('area-closed', function (event, params) {
                    oThis._recalculateSizes();
                    
                    oThis.findNeighbours();

                    event.stopPropagation();
                });
            }

            if (this.isFlexLayout()) {
                $(this.element).droppable({
                    accept: function(elem) {
                        if($(elem).closest('div.layout-area.maximize').length > 0) {
                            return false;
                        }

                        return $(elem).is('.layout-infobox .nav-tabs > li.ui-draggable');
                    },
                    drop: function (event, ui) {
                        var parentPos = $(ui.draggable).closest('div.layout-area').position();
                        var position = $(ui.draggable).position();
                        position.left += parentPos.left;
                        position.top += parentPos.top;

                        var sourceTabs = $(ui.draggable).closest('.layout-area').infoboxtabs('instance');
                        var options = sourceTabs.getOptions();

                        var tabs = oThis.createTabs({
                            left: (position.left * 100.0 / oThis.element.width()).toFixed(2) + '%',
                            top: (position.top * 100.0 / oThis.element.height()).toFixed(2) + '%',
                            right: ((position.left + 300) * 100.0 / oThis.element.width()).toFixed(2) + '%',
                            bottom: ((position.top + 300) * 100.0 / oThis.element.height()).toFixed(2) + '%',
                            allowMaximize: options.allowMaximize,
                            allowClose: options.allowClose,
                            allowHide: options.allowHide,
                            allowSplit: options.allowSplit
                        });

                        oThis.element.append(tabs);

                        $(tabs).on('tabs-create', function (event, params) {
                            $(params.tabs).infoboxtabs('moveTab', ui.draggable);
                        });

                        oThis._initChild(tabs);

                        oThis.findNeighbours();
                    }
                });

                $(this.element).on('split-area', function (event, params) {
                    if (params.direction == 'center') {
                        if ($(params.ui.helper).is('.layout-infobox-tabs .nav-tabs > li')) {
                            $(params.area).infoboxtabs('moveTab', params.ui.draggable);
                        }
                    }
                    else {
                        if(!this.areaWid) this.areaWid = "area";
                        var area = params.area[this.areaWid]('instance');
                        var infobox = params.infobox.infoboxtabs('instance');
                        var rect = area.getRect();

                        var newRect = {};
                        var newTabsParams = {};

                        if (params.direction == 'south') {
                            newRect = {
                                top: rect.top,
                                left: rect.left,
                                right: rect.right,
                                bottom: oThis._round(rect.top + (rect.bottom - rect.top) / 2, oThis.element.outerHeight())
                            };

                            newTabsParams = {
                                left: (rect.left * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                top: (newRect.bottom * 100.0 / oThis.element.height()).toFixed(2) + '%',
                                right: (rect.right * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                bottom: (rect.bottom * 100.0 / oThis.element.height()).toFixed(2) + '%'
                            };
                        }
                        else if (params.direction == 'north') {
                            newRect = {
                                top: oThis._round(rect.top + (rect.bottom - rect.top) / 2, oThis.element.outerHeight()),
                                left: rect.left,
                                right: rect.right,
                                bottom: rect.bottom
                            };

                            newTabsParams = {
                                left: (rect.left * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                top: (rect.top * 100.0 / oThis.element.height()).toFixed(2) + '%',
                                right: (rect.right * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                bottom: (newRect.top * 100.0 / oThis.element.height()).toFixed(2) + '%'
                            };
                        }
                        else if (params.direction == 'west') {
                            newRect = {
                                top: rect.top,
                                left: oThis._round(rect.left + (rect.right - rect.left) / 2, oThis.element.outerWidth()),
                                right: rect.right,
                                bottom: rect.bottom
                            };

                            newTabsParams = {
                                left: (rect.left * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                top: (rect.top * 100.0 / oThis.element.height()).toFixed(2) + '%',
                                right: (newRect.left * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                bottom: (rect.bottom * 100.0 / oThis.element.height()).toFixed(2) + '%'
                            };
                        }
                        else if (params.direction == 'east') {
                            newRect = {
                                top: rect.top,
                                left: rect.left,
                                right: oThis._round(rect.left + (rect.right - rect.left) / 2, oThis.element.outerWidth()),
                                bottom: rect.bottom
                            };

                            newTabsParams = {
                                left: (newRect.right * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                top: (rect.top * 100.0 / oThis.element.height()).toFixed(2) + '%',
                                right: (rect.right * 100.0 / oThis.element.width()).toFixed(2) + '%',
                                bottom: (rect.bottom * 100.0 / oThis.element.height()).toFixed(2) + '%'
                            };
                        }

                        area.setRect(newRect);
                        
                        oThis.updateAreaState(params.area);

                        var ibOptions = infobox.getOptions();
                        
                        $.extend(newTabsParams, {
                            allowMaximize: ibOptions.allowMaximize,
                            allowClose: ibOptions.allowClose,
                            allowHide: ibOptions.allowHide,
                            allowSplit: ibOptions.allowSplit
                        });
                        
                        var tabs = oThis.createTabs(newTabsParams);

                        oThis.element.append(tabs);

                        $(tabs).on('tabs-create', function (event, prms) {
                            $(prms.tabs).infoboxtabs('moveTab', params.ui.draggable);
                        });

                        oThis._initChild(tabs);

                        area.sendEvent('area-resize');
                        area.sendEvent('area-resize-stop');
                    }

                    oThis.findNeighbours();
                });
            }

            if (this.options.saveState) {
                var id = this.element.attr('id');

                if (!id)
                    console.log('ERROR: layout Id must be specified in order to use save state mode! Please assign unique Id to the layout manager!');

                this.element.on('state-changed', function () {
                    var config = oThis.save();
                    $.localStorage.set(id + '-layout', config);
                });
            }

            if (this.options.config) {
                if (this.options.saveState) {
                    var id = this.element.attr('id');
                    var config = $.localStorage.get(id + '-layout');
                    if (config)
                        this.options.config = config;
                }

                if (this.options.bind) {
                    $(this.options.bind).each(function () {
                        JSON.bind(oThis.options.config, this.attrName, this.attrValue, this.value);
                    });
                }

                this.load(this.options.config);
            }
            else {
                this._initChildren();
            }

            
            $(".tab-pane").css("overflow", "");
            $(".tab-pane").css("overflow-y", "auto");
            $(".tab-pane").css("overflow-x", "auto");
            
            for (var key in oThis.options.allGrids){
                oThis.options.allGrids[key].appendAll(oThis);
            }
            
            if ($(this.element).data("layout-gridstackArea"))
                $(this.element).data("layout-gridstackArea").appendAll(oThis);
            
            this._onAfterInit();
            if (this.options._onAfterInit)
                this.options._onAfterInit();
            if (vjDSNew)
                vjDSNew.loadAll();
        },

        isHorizontalLayout: function() {
            return this.options.type == 'horizontal';
        },

        isVerticalLayout: function() {
            return this.options.type == 'vertical';
        },

        isFlexLayout: function() {
            return this.options.type == 'flex';
        },

        isStackLayout: function() {
            return this.options.type == 'stack';
        },
        
        isGridLayout: function() {
            return this.options.type == 'gridstack';
        },
        
        _onBeforeInit: function() {
        },

        _onAfterInit: function() {
        },

        _recalculateSizes: function() {
            var oThis = this;

            if(this.isStackLayout()) {
                this.element.children('div.layout-area:not(.hidden)').each(function(index, area) {
                    var rect = $(area)[this.areaWid]('getRect');
                    
                    if(index == 0) {
                        $(area)[this.areaWid]('setRect', {
                            top: 0,
                            left: oThis._round(rect.left, oThis.element.width()),
                            right: oThis._round(rect.right, oThis.element.width()),
                            bottom: oThis._round(rect.bottom - rect.top)
                        });
                    }
                    else {
                        var prevArea = $(area).prevAll('div.layout-area:not(.hidden):first');
                        var prevRect = $(prevArea)[this.areaWid]('getRect');
                        
                        $(area)[this.areaWid]('setRect', {
                            top: oThis._round(prevRect.bottom),
                            left: oThis._round(rect.left, oThis.element.width()),
                            right: oThis._round(rect.right, oThis.element.width()),
                            bottom: oThis._round(prevRect.bottom) + oThis._round(rect.bottom - rect.top)
                        });
                    }
    
                });
            }
            else {
                var totalPx = 0;
                if(this.isVerticalLayout()) {
                    totalPx = this.element.height();
                }
                else if(this.isHorizontalLayout()) {
                    totalPx = this.element.width();
                }
    
                console.log('total: ' + totalPx);
    
                var size = 0;
                this.element.children('div.layout-area').each(function() {
                    size += parseFloat($(this).data('size'));
                });
    
                this.element.children('div.layout-area').each(function() {
                    var newSize = (parseFloat($(this).data('size')) * 100 / size).toFixed(2) + '%';
                    $(this).data({ size: newSize }).attr({ 'data-size': newSize });
                });
    
                this.element.children('div.layout-area').each(function() {
                    var area = $(this)[this.areaWid]('instance');
                    var newRect = oThis.calculateAreaRect(this);
                    area.setRect(newRect);
    
                    area.sendEvent('area-resize');
                    area.sendEvent('area-resize-stop');
                });
            }
        },
        
        _loadOptions: function () {
            if ($(this.element).is('[data-layout-manager]'))
                this.options.type = $(this.element).data('layout-manager');

            if ($(this.element).is('[data-layout-border]'))
                this.options.border = $(this.element).data('layout-border');

            if ($(this.element).is('[data-layout-margin]'))
                this.options.margin = $(this.element).data('layout-margin');

            if ($(this.element).is('[data-layout-overflow]'))
                this.options.overflow = $(this.element).data('layout-overflow');

            if ($(this.element).is('[data-layout-allow-resize]'))
                this.options.allowResize = $(this.element).data('layout-allow-resize');

            if ($(this.element).is('[data-layout-save-state]'))
                this.options.saveState = $(this.element).data('layout-save-state');
        },

        _initChildren: function () {
            var oThis = this;

            this.element.find('div[data-layout~="infobox"][data-init!=1], div[data-layout~="manager"]').each(function () {
                oThis._initChild(this);
            });

            $.unique($('div[data-need-init-widget=1]', this.element).closest('div.layout-infobox')).each(function() {
                $(this).trigger('init-widgets');
            });

            this.findNeighbours();
        },

        _initChild: function (area, otherOptions) {
            var oThis = this;

            if(this.isStackLayout()) {
                this._prepareAreaOptionsStack(area);
            }
            else {
                this._prepareAreaOptions(area);
            }

            var options = parseInfoboxOptions(area);

            options.rect = oThis.calculateAreaRect(area);
            options.dragTab = this.options.dragTab;
            $.extend(options, otherOptions);

            $(area)[this.areaWid](options);

            if (options.resizable) {
                $(area).on('area-resize-start', function (event, params) {
                    var areaWid = "area";
                    if ($(this).hasClass("layout-gridArea"))
                        areaWid = "gridstackArea";
                    $(this).data({
                        initialRect: $(this)[areaWid]('getRect')
                    });
                });

                $(area).on('area-resize-stop', function (event, params) {
                    oThis.findNeighbours();

                    oThis.sendEvent('state-changed');

                    event.stopPropagation();
                });

                $(area).on('area-resize-move', function (event, params) {
                    oThis._onAreaResizeMove(this, params);
                });
            }

            $(area).on('area-drag-stop', function (event, params) {
                var areaWid = "area";
                if ($(this).hasClass("layout-gridArea"))
                    areaWid = "gridstackArea";
                
                $(this)[areaWid]('findNeighbours');
                
                oThis.updateAreaState(this);

                oThis.sendEvent('state-changed');
            });

            if ($(area).is('div[data-layout~="panel"]')) {
                $(area).infoboxpanel(options);
            }
            else if ($(area).is('div[data-layout~="tabs"]')) {
                $(area).infoboxtabs(options);
            }
            else if ($(area).is('div[data-layout~="manager"]')) {
                options.type = $(area).data('layout-manager');
                $(area).layoutmanager(options);
            }
            else {
                $(area).infobox(options);
            }

            $(area).attr({'data-init': 1});
        },

        _onAreaResizeMove: function(area, params) {
            if(this.isStackLayout()) {
                this._resizeAreaStack(area, params);
                return;
            }
            else {
                this.resizedAreas = [];
                this._resizeArea(area, params.border);
            }
        },

        _resizeArea: function (area, border, allowMax) {
            var oThis = this;

            this.resizedAreas.push(area);

            var a = $(area)[this.areaWid]('instance');

            var position = {
                    top: parseInt($(area).css('top')),
                    left: parseInt($(area).css('left')),
                };

            var newRect;
            if(allowMax){
                newRect = {
                        top: oThis._round(position.top),
                        left: oThis._round(position.left),
                        right: oThis._round(position.left + $(area).outerWidth()),
                        bottom: oThis._round(position.top + $(area).outerHeight())
                    };
            }
            else{
                newRect = {
                        top: oThis._round(position.top, this.element.outerHeight()),
                        left: oThis._round(position.left, this.element.outerWidth()),
                        right: oThis._round(position.left + $(area).outerWidth(), this.element.outerWidth()),
                        bottom: oThis._round(position.top + $(area).outerHeight(), this.element.outerHeight())
                    };
            }

            a.setRect(newRect);

            if (border == 'west') {
                $(a.neighbours.west).each(function () {
                    if(oThis.resizedAreas.indexOf(this) != -1 || !$(this).is(':visible'))
                        return;

                    var prevRect = $(this)[oThis.areaWid]('getRect');
                    $(this)[oThis.areaWid]('setRect', {
                        top: prevRect.top,
                        left: prevRect.left,
                        bottom: prevRect.bottom,
                        right: newRect.left
                    });

                    oThis._resizeArea(this, 'east');

                    oThis.updateAreaState(this);

                    $(this)[oThis.areaWid]('sendEvent', 'area-resize');
                });
            }
            else if (border == 'east') {
                $(a.neighbours.east).each(function () {
                    if(oThis.resizedAreas.indexOf(this) != -1 || !$(this).is(':visible'))
                        return;

                    var prevRect = $(this)[oThis.areaWid]('getRect');
                    $(this)[oThis.areaWid]('setRect', {
                        top: prevRect.top,
                        left: newRect.right,
                        bottom: prevRect.bottom,
                        right: prevRect.right
                    });

                    oThis._resizeArea(this, 'west');

                    oThis.updateAreaState(this);

                    $(this)[oThis.areaWid]('sendEvent', 'area-resize');
                });
            }
            else if (border == 'north') {
                $(a.neighbours.north).each(function () {
                    if(oThis.resizedAreas.indexOf(this) != -1 || !$(this).is(':visible'))
                        return;

                    var prevRect = $(this)[oThis.areaWid]('getRect');
                    $(this)[oThis.areaWid]('setRect', {
                        top: prevRect.top,
                        left: prevRect.left,
                        bottom: newRect.top,
                        right: prevRect.right
                    });

                    oThis._resizeArea(this, 'south');

                    oThis.updateAreaState(this);

                    $(this)[oThis.areaWid]('sendEvent', 'area-resize');
                });
            }
            else if (border == 'south') {
                $(a.neighbours.south).each(function () {
                    if(oThis.resizedAreas.indexOf(this) != -1 || !$(this).is(':visible'))
                        return;

                    var prevRect = $(this)[oThis.areaWid]('getRect');
                    $(this)[oThis.areaWid]('setRect', {
                        top: newRect.bottom,
                        left: prevRect.left,
                        bottom: prevRect.bottom,
                        right: prevRect.right
                    });

                    oThis._resizeArea(this, 'north');

                    oThis.updateAreaState(this);

                    $(this)[oThis.areaWid]('sendEvent', 'area-resize');
                });
            }

            this.updateAreaState(area);

            a.sendEvent('area-resize');
        },

        _resizeAreaStack: function (area, params) {
            var oThis = this;

            var border = params.border;
            if (border == 'west') {
                console.log('WEST border not supported yet!');
            }
            else if (border == 'east') {
                console.log('EAST border not supported yet!');
            }
            else if (border == 'north') {
                var prevArea = $(area).prev();

                var prevAreaRect = $(prevArea)[this.areaWid]('getRect');

                $(prevArea)[this.areaWid]('setRect', {
                    top: prevAreaRect.top,
                    left: prevAreaRect.left,
                    bottom: this._round(params.position.top),
                    right: this._round(params.position.left + params.size.width, this.element.outerWidth())
                });

                $(area)[this.areaWid]('setRect', {
                    top: this._round(params.position.top),
                    left: this._round(params.position.left, this.element.outerWidth()),
                    bottom: this._round(params.position.top) + this._round($(area).data('initialRect').bottom - $(area).data('initialRect').top),
                    right: this._round(params.position.left + params.prevSize.width, this.element.outerWidth())
                });

                this.updateAreaState(area);

                $(area)[this.areaWid]('sendEvent', 'area-resize');
                $(prevArea)[this.areaWid]('sendEvent', 'area-resize');

                $(area).nextAll().each(function() {
                    var rect = $(this)[oThis.areaWid]('getRect');
                    
                    var prevAreaRect = $($(this).prev())[oThis.areaWid]('getRect'); 

                    $(this)[oThis.areaWid]('setRect', {
                        top: prevAreaRect.bottom,
                        left: rect.left,
                        right: rect.right,
                        bottom: prevAreaRect.bottom + rect.bottom - rect.top
                    });
                });
            }
            else if (border == 'south') {
                $(area)[this.areaWid]('setRect', {
                    top: this._round(params.position.top),
                    left: this._round(params.position.left, this.element.outerWidth()),
                    right: this._round(params.position.left + params.prevSize.width, this.element.outerWidth()),
                    bottom: this._round(params.position.top) + this._round($(area).height())
                });

                $(area)[this.areaWid]('sendEvent', 'area-resize');
                
                $(area).nextAll().each(function() {
                    var rect = $(this)[oThis.areaWid]('getRect');
                    
                    var prevAreaRect = $($(this).prev())[this.areaWid]('getRect'); 

                    $(this)[oThis.areaWid]('setRect', {
                        top: prevAreaRect.bottom,
                        left: rect.left,
                        right: rect.right,
                        bottom: prevAreaRect.bottom + rect.bottom - rect.top
                    });
                });
            }
        },
        
        findNeighbours: function() {
            var oThis = this;
            this.element.children('.layout-area').each(function(index, area) {
                $(area)[oThis.areaWid]('findNeighbours');
            });
        },

        _getAbsValue: function (val, range) {
            if (typeof val == 'string' && val.endsWith('%')) {
                var percent = parseFloat(val);
                if (!isNaN(percent))
                    return Math.round(range * percent / 100);
            }
            else
                return val;
        },

        _round: function (val, max) {
            var round = Math.round(val / this.options.roundup) * this.options.roundup;

            if(max != null) {
                if (round > max) return max;
    
                if (max - round < this.options.roundup) return max;
            }

            return round;
        },

        calculateAreaRect: function (area) {
            var oThis = this;

            if (this.isHorizontalLayout()) {
                var size = $(area).data('size');

                var prevLeft = $(area).prev().length == 1 ? parseInt($(area).prev().css('left')) + $(area).prev().outerWidth() : 0;

                if (size == '*') {
                    var otherAreasSize = 0;

                    $(area).siblings('div:layout-area').each(function () {
                        otherAreasSize += oThis._round(oThis._getAbsValue($(this).data('size'), oThis.element.outerWidth()), oThis.element.outerWidth());
                    });

                    return {
                        top: 0,
                        left: prevLeft,
                        right: prevLeft + this.element.outerWidth() - otherAreasSize,
                        bottom: this.element.outerHeight()
                    };
                }
                else {
                    return {
                        top: 0,
                        left: prevLeft,
                        right: this.isAreaFixed(area) ? prevLeft + this._getAbsValue(size, this.element.outerWidth()) : this._round(prevLeft + this._getAbsValue(size, this.element.outerWidth()), this.element.outerWidth()),
                        bottom: this.element.outerHeight()
                    };
                }
            }
            else if (this.isVerticalLayout()) {
                var size = $(area).data('size');

                var prevBottom = $(area).prev().length == 1 ? parseInt($(area).prev().css('top')) + $(area).prev().outerHeight() : 0;

                if (size == '*') {
                    var otherAreasSize = 0;

                    $(area).siblings('div:layout-area').each(function () {
                        otherAreasSize += oThis._round(oThis._getAbsValue($(this).data('size'), oThis.element.outerHeight()), oThis.element.outerHeight());
                    });

                    return {
                        top: prevBottom,
                        left: 0,
                        right: this.element.outerWidth(),
                        bottom: prevBottom + this.element.outerHeight() - otherAreasSize
                    };
                }
                else {
                    return {
                        top: prevBottom,
                        left: 0,
                        right: this.element.outerWidth(),
                        bottom: this.isAreaFixed(area) ? prevBottom + this._getAbsValue(size, this.element.outerHeight()) : this._round(prevBottom + this._getAbsValue(size, this.element.outerHeight()), this.element.outerHeight())
                    };
                }
            }
            else if (this.isFlexLayout()) {
                return {
                    top: this._round(this._getAbsValue($(area).data('top'), this.element.outerHeight()), this.element.outerHeight()),
                    left: this._round(this._getAbsValue($(area).data('left'), this.element.outerWidth()), this.element.outerWidth()),
                    right: this._round(this._getAbsValue($(area).data('right'), this.element.outerWidth()), this.element.outerWidth()),
                    bottom: this._round(this._getAbsValue($(area).data('bottom'), this.element.outerHeight()), this.element.outerHeight()),
                };
            }
            else if (this.isStackLayout()) {
                return this.calculateAreaRectStack(area);
            }
            else if (this.isGridLayout()){
                return {
                    top: 0,
                    left: 0,
                    right: $(area).parent().outerWidth(),
                    bottom: $(area).parent().outerHeight(),
                };
            }
        },

        calculateAreaRectStack: function (area) {
            var oThis = this;

            if(this.options.type != 'stack')
                return null;

            if (size == '*') {
                console.error('Size * not supported for Stack type layout!');
            }

            if (this.options.orientation == 'horizontal') {
                var size = $(area).data('size');

                var prevLeft = $(area).prev().length == 1 ? parseInt($(area).prev().css('left')) + $(area).prev().outerWidth() : 0;

                return {
                    top: 0,
                    left: prevLeft,
                    right: prevLeft + this._getAbsValue(size, this.element.outerWidth()),
                    bottom: this.element.outerHeight()
                };
            }
            else if (this.options.orientation == 'vertical') {
                var size = $(area).data('size');

                var prevBottom = $(area).prev().length == 1 ? parseInt($(area).prev().css('top')) + $(area).prev().outerHeight() : 0;

                return {
                    top: prevBottom,
                    left: 0,
                    right: this.element.outerWidth(),
                    bottom: prevBottom + this._getAbsValue(size, this.element.outerHeight())
                };
            }
        },
        
        isAreaFixed: function (area) {
            return $(area).data('resizable') != null && !$(area).data('resizable');


        },

        _prepareAreaOptions: function (area) {
            if (this.isAreaFixed(area) || !this.options.allowResize)
                $(area).attr('data-resizable', false);

            if (this.options.border == 'none' || this.options.border == 'between') {
                $('.layout-infobox', area).css({ 'border-color': 'transparent' });
            }

            if (this.options.margin == 'none') {
                $('.layout-infobox', area).css({ 'margin': '0' });
            }

            if (this.isHorizontalLayout()) {
                $(area).attr('data-draggable', false);

                if ($(area).prev('div.layout-area').length == 0) {
                    if ($(area).next('div.layout-area').length > 0 && !this.isAreaFixed($(area).next('div.layout-area')))
                        $(area).attr('data-resizable-handles', 'e');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('east-border');

                    if (this.options.margin == 'between') {
                        $('.layout-infobox', area).css({ 'margin-top': '0', 'margin-left': '0', 'margin-bottom': '0' });
                    } else if (this.options.margin == 'outside') {
                        $('.layout-infobox', area).css({ 'margin-right': '0' });
                    }
                }
                else if ($(area).next('div.layout-area').length == 0) {
                    if ($(area).prev('div.layout-area').length > 0 && !this.isAreaFixed($(area).prev('div.layout-area')))
                        $(area).attr('data-resizable-handles', 'w');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('west-border');

                    if (this.options.margin == 'between') {
                        $('.layout-infobox', area).css({ 'margin-top': '0', 'margin-right': '0', 'margin-bottom': '0' });
                    } else if (this.options.margin == 'outside') {
                        $('.layout-infobox', area).css({ 'margin-left': '0' });
                    }
                }
                else {
                    var handles = '';
                    if (!this.isAreaFixed($(area).prev('div.layout-area')))
                        handles = 'w';

                    if (!this.isAreaFixed($(area).next('div.layout-area')))
                        handles += handles == '' ? 'e' : ',e';

                    if(handles != '')
                        $(area).attr('data-resizable-handles', handles);
                    else
                        $(area).attr('data-resizable', false);

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('east-border west-border');

                    if (this.options.margin == 'between') {
                        $('.layout-infobox', area).css({ 'margin-top': '0', 'margin-bottom': '0' });
                    } else if (this.options.margin == 'outside') {
                        $('.layout-infobox', area).css({ 'margin-left': 0, 'margin-right': '0' });
                    }
                }
            }
            else if (this.isVerticalLayout()) {
                $(area).attr('data-draggable', false);

                if ($(area).prev('div.layout-area').length == 0) {
                    if ($(area).next('div.layout-area').length > 0 && !this.isAreaFixed($(area).next('div.layout-area')))
                        $(area).attr('data-resizable-handles', 's');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('south-border');

                    if (this.options.margin == 'between') {
                        $('.layout-infobox', area).css({ 'margin-left': '0', 'margin-top': '0', 'margin-right': '0' });
                    } else if (this.options.margin == 'outside') {
                        $('.layout-infobox', area).css({ 'margin-bottom': '0' });
                    }
                }
                else if ($(area).next('div.layout-area').length == 0) {
                    if ($(area).prev('div.layout-area').length > 0 && !this.isAreaFixed($(area).prev('div.layout-area')))
                        $(area).attr('data-resizable-handles', 'n');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('north-border');

                    if (this.options.margin == 'between') {
                        $('.layout-infobox', area).css({ 'margin-left': '0', 'margin-right': '0' });
                    } else if (this.options.margin == 'outside') {
                        $('.layout-infobox', area).css({ 'margin-top': '0', 'margin-bottom': '0' });
                    }
                }
                else {
                    var handles = '';
                    if (!this.isAreaFixed($(area).prev('div.layout-area')))
                        handles = 'n';

                    if (!this.isAreaFixed($(area).next('div.layout-area')))
                        handles += handles == '' ? 's' : ',s';

                    if(handles != '')
                        $(area).attr('data-resizable-handles', handles);
                    else
                        $(area).attr('data-resizable', false);

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('north-border south-border');

                    if (this.options.margin == 'between') {
                        $('.layout-infobox', area).css({ 'margin-left': '0', 'margin-bottom': '0', 'margin-right': '0' });
                    } else if (this.options.margin == 'outside') {
                        $('.layout-infobox', area).css({ 'margin-top': '0' });
                    }
                }
            }
        },

        _prepareAreaOptionsStack: function (area) {
            if (this.isAreaFixed(area))
                $(area).attr('data-resizable', false);

            if (this.options.border == 'none' || this.options.border == 'between') {
                $('.layout-infobox', area).css({ 'border-color': 'transparent' });
            }

            if (this.options.orientation == 'horizontal') {
                $(area).attr('data-draggable', false);

                if ($(area).prev('div.layout-area').length == 0) {
                    if ($(area).next('div.layout-area').length > 0 && !this.isAreaFixed($(area).next('div.layout-area')))
                        $(area).attr('data-resizable-handles', 'e');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('east-border');
                }
                else if ($(area).next('div.layout-area').length == 0) {
                    if ($(area).prev('div.layout-area').length > 0 && !this.isAreaFixed($(area).prev('div.layout-area')))
                        $(area).attr('data-resizable-handles', 'w');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('west-border');
                }
                else {
                    var handles = '';
                    if (!this.isAreaFixed($(area).prev('div.layout-area')))
                        handles = 'w';

                    if (!this.isAreaFixed($(area).next('div.layout-area')))
                        handles += handles == '' ? 'e' : ',e';

                    if(handles != '')
                        $(area).attr('data-resizable-handles', handles);
                    else
                        $(area).attr('data-resizable', false);

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('east-border west-border');
                }
            }
            else if (this.options.orientation == 'vertical') {
                $(area).attr('data-draggable', false);

                if ($(area).prev('div.layout-area').length == 0) {
                    if ($(area).next('div.layout-area').length > 0 && !this.isAreaFixed($(area).next('div.layout-area')))
                        $(area).attr('data-resizable-handles', 's');

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('south-border');
                }
                else if ($(area).next('div.layout-area').length == 0) {
                    var handles = '';

                    if ($(area).prev('div.layout-area').length > 0 && !this.isAreaFixed($(area).prev('div.layout-area')))
                        handles += 'n';

                    if (!this.isAreaFixed(area))
                        handles += handles == '' ? 's' : ',s';

                    if(handles != '')
                        $(area).attr('data-resizable-handles', handles);
                    else
                        $(area).attr('data-resizable', false);
                    
                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('north-border');
                }
                else {
                    var handles = '';
                    if (!this.isAreaFixed($(area).prev('div.layout-area')))
                        handles = 'n';

                    if (!this.isAreaFixed($(area).next('div.layout-area')))
                        handles += handles == '' ? 's' : ',s';

                    if(handles != '')
                        $(area).attr('data-resizable-handles', handles);
                    else
                        $(area).attr('data-resizable', false);

                    if (this.options.border == 'between')
                        $('.layout-infobox', area).addClass('north-border south-border');
                }
            }
        },
        
        updateAreaState: function(area) {
            if (this.isHorizontalLayout() || (this.options.type == 'stack' && this.options.orientation == 'horizontal')) {
                var areaSize = $(area).data("size");
                if(!areaSize || (typeof areaSize == "string" &&  areaSize.indexOf("%") > -1)){
                    $(area).data({
                        size: ($(area).outerWidth() * 100 / this.element.outerWidth()).toFixed(3) + '%'
                    });
                }
                else{
                    $(area).data({
                        size: $(area).outerWidth()
                    });
                }
                $(area).attr({
                    'data-size': $(area).data('size')
                });
            }
            else if (this.isVerticalLayout() || (this.options.type == 'stack' && this.options.orientation == 'vertical')) {
                var areaSize = $(area).data("size");
                if(!areaSize || (typeof areaSize == "string" && $(area).data("size").indexOf("%") > -1)){
                    $(area).data({
                        size: ($(area).outerHeight() * 100 / this.element.outerHeight()).toFixed(3) + '%'
                    });
                }
                else{
                    $(area).data({
                        size: $(area).outerWidth()
                    });
                }
                $(area).attr({
                    'data-size': $(area).data('size')
                });
            }
            else if (this.isFlexLayout()) {
                var rect = $(area)[this.areaWid]('getRect');
                
                $(area).data({
                    top: (rect.top * 100 / this.element.outerHeight()).toFixed(3) + '%',
                    left: (rect.left * 100 / this.element.outerWidth()).toFixed(3) + '%',
                    right: (rect.right * 100 / this.element.outerWidth()).toFixed(3) + '%',
                    bottom: (rect.bottom * 100 / this.element.outerHeight()).toFixed(3) + '%'
                });
                $(area).attr({
                    'data-top': $(area).data('top'),
                    'data-left': $(area).data('left'),
                    'data-right': $(area).data('right'),
                    'data-bottom': $(area).data('bottom')
                });
            }
        },

        createArea: function (params) {
            var area = $(document.createElement('div'))
                .attr({
                    'data-id': params.id
                })
                .addClass((params.layout && params.layout.layoutType && params.layout.layoutType=="gridstack") ? "layout-gridArea" : 'layout-area');
            
            if(params.onClose) {
                area.on('area-closed', function(event, areaParams) {
                    params.onClose(areaParams);
                })
            }
            
            if(params.onHide) {
                area.on('area-hide', function(event, areaParams) {
                    params.onHide(areaParams);
                })
            }
            
            if(params.hasOwnProperty('top'))
                area.attr('data-top', params.top);
            if(params.hasOwnProperty('left'))
                area.attr('data-left', params.left);
            if(params.hasOwnProperty('right'))
                area.attr('data-right', params.right);
            if(params.hasOwnProperty('bottom'))
                area.attr('data-bottom', params.bottom);
            if(params.hasOwnProperty('size'))
                area.attr('data-size', params.size);
            if(params.hasOwnProperty('toggler'))
                area.attr('data-toggler', params.toggler);
            if (params.hasOwnProperty('class'))
                area.attr({ 'data-class': params.class })
            if (params.hasOwnProperty('allowResize'))
                area.attr('data-resizable', params.allowResize);
            if (params.hasOwnProperty('dragTabItself'))
                area.attr('data-dragTabItself', params.dragTabItself);

            
            return area;
        },

        createInfobox: function (params) {
            var oThis = this;

            var area = this.createArea(params);

            area.attr({ 'data-layout': 'infobox' });

            var infobox = $(document.createElement('div'))
                            .addClass("layout-infobox")
                            .appendTo(area);

            if (params.hasOwnProperty('id'))
                infobox.attr({ 'data-id': params.id })

            if (params.hasOwnProperty('allowMaximize'))
                infobox.attr({ 'data-allow-maximize': params.allowMaximize })

            if (params.hasOwnProperty('allowClose'))
                infobox.attr({ 'data-allow-close': params.allowClose })

            if (params.hasOwnProperty('allowHide'))
                infobox.attr({ 'data-allow-hide': params.allowHide })

            if (params.hasOwnProperty('allowSplit'))
                infobox.attr({ 'data-allow-split': params.allowSplit })

            if (params.hasOwnProperty('droppable'))
                infobox.attr({ 'data-droppable': params.droppable })

            return area;
        },

        createLayout: function (params) {
            var oThis = this;

            var area = this.createArea(params);

            area.attr({
                'data-layout': 'manager',
                'data-layout-manager': params.layout.layoutType
            });

            if (params.layout.border !== undefined) {
                area.attr('data-layout-border', params.layout.border);
            }

            if (params.layout.margin !== undefined) {
                area.attr('data-layout-margin', params.layout.margin);
            }

            if (params.layout.allowResize !== undefined) {
                area.attr('data-layout-allow-resize', params.layout.allowResize);
            }

            if (params.layout.overflow !== undefined) {
                area.attr('data-layout-overflow', params.layout.overflow);
            }

            if (params.layout && params.layout.layoutType && params.layout.layoutType=="gridstack"){
                var gridWid = area.gridstackArea().first().gridstackArea("instance");
                params.instance = gridWid;
                
                $(params.layout.items).each(function () {
                    if (this.tabs) {
                        if (this.tabs.items)
                            this.tabs.items = this.tabs.items.splice(0,1);
                        var tabs = oThis.createTabs(this);
                        gridWid.appendTabs(tabs, this.tabs);
                        oThis.options.allGrids[gridWid.areaName] = gridWid;
                    }
                    else if (this.layout) {
                        console.log("went into creating layout for gridstack");
                        return;
                        if (this.title) {
                            var panel = oThis.createPanel(this);
                            gridWid.appendPanel(panel, this.id);
                        }
                        else {
                            var layout = oThis.createLayout(this);
                            gridWid.append(layout, this.id);
                        }
                    }
                    else {
                        console.log("went into creating panels for gridstack");
                        return;
                        var panel = oThis.createPanel(this);
                        gridWid.appendPanel(panel, this.id);
                    }
                });
            }            
            else{
                $(params.layout.items).each(function () {
                    if (this.tabs) {
                        var tabs = oThis.createTabs(this);
                        area.append(tabs);
                    }
                    else if (this.layout) {
                        if (this.title) {
                            var panel = oThis.createPanel(this);
                            area.append(panel);
                        }
                        else {
                            var layout = oThis.createLayout(this);
                            area.append(layout);
                        }
                    }
                    else {
                        var panel = oThis.createPanel(this);
                        area.append(panel);
                    }
                });
            }

            return area;
        },

        createTabs: function (params) {
            var area = this._createTabsArea(params);
            this._addTabs(area, params);
            
            return area;
        },

        _createTabsArea: function(params) {
            var area = this.createInfobox(params);

            var infobox = $('.layout-infobox', area);

            infobox.addClass("panel panel-default");

            infobox.append(
                    $(document.createElement('div'))
                        .addClass('panel-heading')
                        .css({ position: 'relative' })
                        .append(
                            $(document.createElement('ul'))
                                .addClass('nav nav-tabs')
                                .attr({
                                    role: 'tablist'
                                })
                        )
                )
                .append(
                    $(document.createElement('div'))
                        .addClass('panel-body tab-content')
                );

            $(area).attr('data-layout', $(area).data('layout') + ' tabs');

            if(params.tabs != null && params.tabs.onActivate) {
                $(area).on('tab-shown', function(event, p) {
                    params.tabs.onActivate(p.index, p.tab);
                });
            }

            $(area).on('tab-shown', function(event, p) {
                $(this).trigger('tab-open', {
                    index: p.index,
                    name: $('a', p.tab).attr('data-name'),
                    area: $(p.tab).closest('div.layout-area').attr('data-id'),
                    tabs: p.tabs
                });
            });

            return area;
        },

        _addTabs: function(area, params) {
            var oThis = this;

            if (params.tabs) {
                $(params.tabs.items).each(function (index, tab) {
                    if($('a[data-name="' + tab.name + '"]').length > 0) {
                        if($('a[data-name="' + tab.name + '"]').length == 1 && $('a[data-name="' + tab.name + '"]').is(':hidden')) {
                            oThis.show(tab.name);
                            return;
                        }
                        else {
                            console.log('ERROR: Tab with name "' + tab.name + '" already exists!');
                            return;
                        }
                    }
                    
                    if ($('ul.nav-tabs', area).children(":not(.hidden)").length < 1)
                        tab.active = true;

                    if (tab.active)
                    {
                        $('ul.nav-tabs', area).children().removeClass("active");
                        $('.tab-content', area).children().removeClass("active");
                    }

                    var li = $(document.createElement('li'))
                                .addClass(tab.active ? 'active' : '');

                    if (tab.title && tab.newIcon){
                        li.append($(document.createElement('a'))
                                    .attr({
                                        href: '#' + tab.name + '-tab',
                                        role: 'tab',
                                        'data-toggle': 'tab',
                                        'data-name': tab.name
                                    })
                                    .text(" " + tab.title)
                            );
                        li.children("a").prepend($(document.createElement('i'))
                                .addClass(tab.class))
                    } else if(tab.title) {
                        li.append(
                                $(document.createElement('a'))
                                    .addClass(tab.class)
                                    .attr({
                                        href: '#' + tab.name + '-tab',
                                        role: 'tab',
                                        'data-toggle': 'tab',
                                        'data-name': tab.name
                                    })
                                    .text(tab.title)
                            );
                        }

                    if (tab.allowClone) {
                        $('a', li).append(
                                $(document.createElement('span'))
                                .attr({
                                    title: 'Clone'
                                })
                                .addClass('activity-icon clone-tab icomoon-liga')
                                .click(function () {
                                    var config = $(area).infoboxtabs('buildTabConfig', tab.name);

                                    $.extend(config, {
                                        onBeforeClone: tab.onBeforeClone,
                                        onAfterClone: tab.onAfterClone,
                                    })
                                    
                                    if(tab.onBeforeClone)
                                        tab.onBeforeClone(config);

                                    var index = 2;
                                    while($('a[data-name="' + config.name + '"]', oThis.element).length > 0) {
                                        config.name += '-' + index++;
                                    }

                                    oThis.append({
                                        layout: {
                                            items:[
                                            {
                                                id: params.id,
                                                tabs:{
                                                    items: [config]
                                                }
                                            }]
                                        }
                                    });
                                    
                                    if(tab.onAfterClone)
                                        tab.onAfterClone(config);
                                    
                                    return false;
                                })
                            )
                            .attr({
                                'data-allow-clone': tab.allowClone
                            });
                    }

                    if (tab.allowClose || tab.allowHide) {
                        var a = $('a', li).append(
                           $(document.createElement('i'))
                            .attr({
                                title: 'Close'
                            })
                            .addClass('activity-icon close-tab icomoon-liga rv-close-thin')
                            .click(function () {
                                if(tab.allowClose) {
                                    oThis.remove($(this).closest('a').data('name'));
                                }
                                else if(tab.allowHide) {
                                    oThis.hide($(this).closest('a').data('name'));
                                }

                                return false;
                            })
                        );
                        
                        if(tab.allowClose)
                            a.attr({ 'data-allow-close': tab.allowClose });

                        if(tab.allowHide)
                            a.attr({ 'data-allow-hide': tab.allowHide });
                    }

                    $('ul.nav-tabs', area).append( li );

                    var tabPane = $(document.createElement('div'))
                                        .addClass('tab-pane view-area .fade')
                                        .addClass(tab.active ? 'active' : '')
                                        .css({
                                            overflow: tab.overflow == null ? 'hidden' : tab.overflow
                                        })
                                        .attr({
                                            id: tab.name + '-tab'
                                        });

                    $('div.tab-content', area).append(tabPane);

                    if (tab.layout) {
                        var layout = oThis.createLayout(tab);
                        tabPane.append(layout);
                    }

                    if (tab.view) {
                        tabPane.data('jquery-widget', tab.view.name).attr('data-need-init-widget', 1);

                        if (tab.view.options) {
                            tabPane.data('jquery-widget-options', tab.view.options);
                        }
                    }

                    $(area).trigger('new-tab-added', { tab: li, pane: tabPane });
                });
            }
        },
        
        createPanel: function (params) {
            var area = this.createInfobox(params);

            var infobox = $('.layout-infobox', area);

            if(!params.overflow)
                params.overflow = 'hidden';

            infobox.addClass("panel panel-default");

            if(params.title) {
                infobox.append(
                        $(document.createElement('div'))
                            .addClass('panel-heading')
                            .addClass(params.class)
                            .css({ position: 'relative' })
                            .append(
                                $(document.createElement('div'))
                                    .addClass('title')
                                    .text(params.title)
                            )
                    );
            }

            infobox.append(
                $(document.createElement('div'))
                    .addClass('panel-body view-area')
                    .css({
                        overflow: params.overflow
                    })
                    .attr({
                        id: params.id + '-panel'
                    })
            );
            
            $(area).attr('data-layout', $(area).data('layout') + ' panel');

            if (params.name)
                $(infobox).attr({ 'data-name': params.name });
 
            if (params.layout) {
                var layout = this.createLayout({ layout: params.layout });
                $('.view-area', area).append(layout);
            }

            if(params.view) {
                $('.view-area', area).data('jquery-widget', params.view.name).attr('data-need-init-widget', 1);
                
                if(params.view.options) {
                    $('.view-area', area).data('jquery-widget-options', params.view.options);
                }
            }

            return area;
        },

        save: function() {
            var config = {
                layoutType: this.options.type,
                resizable: this.options.resizable,
                border: this.options.border,
                overflow: this.options.overflow,
                items: []
            };

            this.element.children('div[data-layout~="infobox"], div[data-layout~="manager"]').each(function (index, area) {
                if ($(area).is('div.layout-manager')) {
                    config.items.push($(area).layoutmanager('save'));
                }
                else if ($(area).is('div[data-layout~="panel"]')) {
                    config.items.push($(area).infoboxpanel('save'));
                }
                else if ($(area).is('div[data-layout~="tabs"]')) {
                    config.items.push($(area).infoboxtabs('save'));
                }
            });

            var state = this.area.save();
            state.layout = config;

            return state;
        },

        load: function (config) {
            this.append(config);
            
            this.sendEvent('manager-loaded')
        },

        append: function (config, extraElem) {
            if (config.layout) {
                var oThis = this;

                $(config.layout.items).each(function () {
                    var area = $();
                    if(this.hasOwnProperty('id')) {
                        area = $('div.layout-area[data-id="' + this.id + '"]');
                    }
                    
                    if (this.tabs) {
                        if(area.length == 1) {
                            area.removeClass("hidden");
                            oThis._addTabs(area, this);
                        }
                        else if(area.length == 0) {
                            var tabs = oThis.createTabs(this);
                            if (oThis.isGridLayout()){
                                extraElem.appendTabs(tabs, this.tabs);
                            }else
                                oThis.element.append(tabs);
                        }
                        else {
                            console.log('ERROR: too many areas found by ID: ' + this.id);
                        }
                    }
                    else if (this.layout) {
                        if (this.title) {
                            var panel = oThis.createPanel(this);
                            oThis.element.append(panel);
                        }
                        else {
                            var layout = oThis.createLayout(this);
                            oThis.element.append(layout);
                        }
                    }
                    else {
                        var panel = oThis.createPanel(this);
                        oThis.element.append(panel);
                    }
                });

                this._initChildren();
            }
        },
        
        appendTo: function (config, id) {
            if (config.layout) {
                var oThis = this;

                $(config.layout.items).each(function () {
                    var area = $();
                    if(this.hasOwnProperty('id')) {
                        area = $('div.layout-area[data-id="' + this.id + '"]');
                    }
                    
                    if (this.tabs) {
                        if(area.length == 1) {
                            area.removeClass("hidden");
                            oThis._addTabs(area, this);
                        }
                        else if(area.length == 0) {
                            var tabs = oThis.createTabs(this);
                            oThis.element.append(tabs);
                        }
                        else {
                            console.log('ERROR: too many areas found by ID: ' + this.id);
                        }
                    }
                    else if (this.layout) {
                        if (this.title) {
                            var panel = oThis.createPanel(this);
                            oThis.element.append(panel);
                        }
                        else {
                            var layout = oThis.createLayout(this);
                            oThis.element.append(layout);
                        }
                    }
                    else {
                        var panel = oThis.createPanel(this);
                        oThis.element.append(panel);
                    }
                });

                this._initChildren();
            }
        },

        remove: function(id) {
            var area = $('div.layout-area[data-id="' + id + '"]');

            if(area.length == 1) {
                area.addClass('hidden');
                
                this.sendEvent('area-closed', { area: area });

                area.remove();
            }
            else if(area.length == 0) {
                var tab = $('a[data-name="' + id + '"]');
                if(tab.length == 1) {
                    var infobox = tab.closest('div.layout-infobox');

                    var ul = tab.closest('ul');

                    var tabPane = $(tab.attr('href'), infobox);

                    tab.parent().remove();
                    tabPane.remove();

                    infobox.trigger('tab-close', id);

                    if(ul.children('li').length == 0) {
                        infobox.trigger('area-closed', ul.closest('div.layout-area').data('id'));
                        
                        ul.closest('div.layout-area').remove();
                    }
                    else if(ul.children('li').length == ul.children('li.hidden').length) {
                        ul.closest('div.layout-area').addClass('hidden');

                        infobox.trigger('area-hide', ul.closest('div.layout-area').data('id'));
                    }
                }
            }
            else {
                console.log('ERROR: too many areas found by ID: ' + id);
            }
        },

        hide: function(id) {
            var area = $('div.layout-area[data-id="' + id + '"]');

            if(area.length == 1) {
                area.addClass('hidden');

                this.sendEvent('area-hide', { area: area });
            }
            else if(area.length == 0) {
                var tab = $('a[data-name="' + id + '"]');
                if(tab.length == 1) {
                    var infobox = tab.closest('div.layout-infobox');

                    var ul = tab.closest('ul');

                    var tabPane = $(tab.attr('href'), infobox);

                    tab.parent().addClass('hidden');
                    tabPane.addClass('hidden');

                    infobox.trigger('tab-hide', id);

                    if(ul.children(':not(.hidden)').length == 0) {
                        ul.closest('div.layout-area').addClass('hidden');

                        infobox.trigger('area-hide', ul.closest('div.layout-area').data('id'));
                    }
                }
            }
            else {
                console.log('ERROR: too many areas found by ID: ' + id);
            }
        },

        show: function(id) {
            area = $('div.layout-area[data-id="' + id + '"]');

            if(area.length == 1) {
                area.removeClass('hidden');

                area[this.areaWid]('setFocus');

                return true;
            }
            else if(area.length == 0) {
                var tab = $('a[data-name="' + id + '"]');
                if(tab.length == 1) {
                    var infobox = tab.closest('div.layout-infobox');
                    var area = tab.closest('div.layout-area');

                    var ul = tab.closest('ul');

                    var tabPane = $(tab.attr('href'), infobox);
                    
                    if(ul.children(':not(.hidden)').length == 0) {
                        ul.closest('div.layout-area').removeClass('hidden');
                    }

                    tab.parent().removeClass('hidden');
                    tabPane.removeClass('hidden');
                    tab.closest('div.layout-area').removeClass('hidden');
                    
                    $('ul.nav-tabs', area).children().removeClass("active");
                    $('.tab-content', area).children().removeClass("active");
                    tabPane.closest(".tab-content").children(":not(.hidden)").removeClass("active");
                    ul.closest(".nav").children(":not(.hidden)").removeClass("active");
                    tabPane.addClass("active");
                    tab.parent().addClass("active");

                    $(area)[this.areaWid]('setFocus');
                    
                    return true;
                }
                return false;
            }
            else {
                console.log('ERROR: too many areas found by ID: ' + id);
                return false;
            }
        },
        
        clear: function(id) {
            var area = $('div.layout-area[data-id="' + id + '"]');

            if(area.length == 1) {
                $('.layout-infobox>.panel-heading>ul', area).empty();
                $('.layout-infobox>.panel-body', area).empty();
            }
            else if(area.length == 0) {
                var tab = $('a[data-name="' + id + '"]');
                if(tab.length == 1) {
                    var infobox = tab.closest('div.layout-infobox');
                    
                    var ul = tab.closest('ul');
                    
                    var tabPane = $(tab.attr('href'), infobox);

                    tabPane.empty();
                }
            }
            else {
                console.log('ERROR: too many areas found by ID: ' + id);
            }
        },

        moveArea: function(id, rect, duration) {
            var area = $('div.layout-area[data-id="' + id + '"]');

            if(area.length == 1) {
                var a = $(area)[this.areaWid]('instance');

                var newRect = {
                    top: this._round(this._getAbsValue(rect.top, this.element.outerHeight()), this.element.outerHeight()),
                    left: this._round(this._getAbsValue(rect.left, this.element.outerWidth()), this.element.outerWidth()),
                    right: this._round(this._getAbsValue(rect.right, this.element.outerWidth()), this.element.outerWidth()),
                    bottom: this._round(this._getAbsValue(rect.bottom, this.element.outerHeight()), this.element.outerHeight())
                };

                a.setRect(newRect, {
                    duration: duration,
                    done: function() {
                        a.sendEvent('area-resize');
                        a.sendEvent('area-resize-stop');
                        a.sendEvent('area-drag-stop', {fromResize: true});
                    }
                });
            }
            else if(area.length == 0) {
                console.log('ERROR: can\'t find area with ID: ' + id);
            }
            else {
                console.log('ERROR: too many areas found by ID: ' + id);
            }
            
        },

        sendEvent: function (name, params) {
            if (params == null)
                params = {};

            if (name == 'manager-loaded') {
                $(this.element).trigger('manager-loaded', params);
            }
            else if (name == 'state-changed') {
                $(this.element).trigger('state-changed', params);
            }
            else if (name == 'area-hide' || name == 'area-closed') {
                var config = {};
                
                if($(params.area).is('div[data-layout~="infobox"], div[data-layout~="manager"]')) {
                    if ($(params.area).is('div.layout-manager')) {
                        config = $(params.area).layoutmanager('save');
                    }
                    else if ($(params.area).is('div[data-layout~="panel"]')) {
                        config = $(params.area).infoboxpanel('save');
                    }
                    else if ($(params.area).is('div[data-layout~="tabs"]')) {
                        config = $(params.area).infoboxtabs('save');
                    }
                }
                
                $.extend(params, {
                    id: $(params.area).attr('data-id'),
                    config: config
                });

                $(params.area).trigger(name, params);
            }
        },

        getArea: function(id) {
            return $('div.layout-area[data-id="' + id + '"]', this.element);
        },

        getInfobox: function(id) {
            return $('div.layout-infobox[data-id="' + id + '"]', this.element);
        },
        
        destroy: function () {
            this.element.empty();
            $.Widget.prototype.destroy.call(this);
        },

        _destroy: function () {
        },

        _refresh: function () {
        },

        _setOption: function (key, value) {
            $.Widget.prototype._setOption.apply(this, arguments);
        },

        _setOptions: function () {
            $.Widget.prototype._setOptions.apply(this, arguments);
            this._refresh();
        }
    });
}(jQuery));

$(document).ready(function () {
    $('div[data-layout~="manager"]:first').each(function (index, manager) {
        var options = {
            type: $(this).data('layout-manager'),
            rect: {
                top: 0,
                left: 0,
                right: $(this).parent().outerWidth(),
                bottom: $(this).parent().outerHeight()
            },
            resizable: false
        };
        var areaWid = "area";
        if ($(this).hasClass("layout-gridArea"))
            areaWid = "gridstackArea";
        
        $(this)[areaWid](options).layoutmanager(options);
    });
});

var delayWindowResize = (function (event) {
    var timer = 0;
    return function (callback, ms) {
        clearTimeout(timer);
        timer = setTimeout(callback, ms);
    };
})();

$(window).resize(function (event) {
    if (event.target == window) {
        delayWindowResize(function () {
            $('div.layout-area').each(function () {
                if ($(this).parent().closest('div.layout-area').length == 0) {

                    var area;
                    if ($(this).hasClass("layout-gridArea"))
                        area = $(this).gridstackArea('instance')
                    else
                        area = $(this).area("instance");
                    
                    var parent = $(this).parent(".content");
                    if(parent.length == 0)
                        parent = $(this).closest(".content");

                    area.setRect({
                        top: 0,
                        left: 0,
                        right: parent.outerWidth(),
                        bottom: parent.outerHeight()
                    });

                    area.sendEvent('area-resize');
                    area.sendEvent('area-resize-stop');
                }
            });
        }, 250);
    }
});

