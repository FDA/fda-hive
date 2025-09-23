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
    $.widget("layout.area", {

        options: {
            class: null,
            rect: null,
            resizable: {
                minWidth: 50,
                minHeight: 50,
                containment: 'parent',
                handles: "n, e, s, w",
                grid: [20, 20]
            },
            toggler: null
        },

        _create: function () {
            var oThis = this;

            this.element.addClass("layout-area").addClass(this.options.class);

            this.margin = (this.element.outerWidth(true) - this.element.outerWidth()) / 2;
            this.border = (this.element.outerWidth() - this.element.innerWidth()) / 2;

            if (this.options.resizable) {
                var options = $.extend(this.options.resizable, {
                    start: function (event, ui) {
                        var border = event.toElement ? event.toElement : event.originalEvent.target;
                        if ($(border).hasClass('ui-resizable-e')) {
                            oThis.borderResizeType = 'east';
                        }
                        else if ($(border).hasClass('ui-resizable-n')) {
                            oThis.borderResizeType = 'north';
                        }
                        else if ($(border).hasClass('ui-resizable-s')) {
                            oThis.borderResizeType = 'south';
                        }
                        else if ($(border).hasClass('ui-resizable-w')) {
                            oThis.borderResizeType = 'west';
                        }

                        $.extend(ui, {
                            area: this,
                            border: oThis.borderResizeType
                        });
                        
                        oThis.sendEvent('area-resize-start', ui);
                    },
                    stop: function (event, ui) {
                        var neighbours = oThis.neighbours.north.concat(oThis.neighbours.south).concat(oThis.neighbours.west).concat(oThis.neighbours.east);
                        
                        $(neighbours).each(function(index, area) {
                            $(area).area('sendEvent', 'area-resize-stop');
                        });
                        
                        oThis.sendEvent('area-resize-stop', ui);
                    },
                    resize: function (event, ui) {
                        $.extend(ui, {
                            border: oThis.borderResizeType
                        });
                        
                        oThis.sendEvent('area-resize-move', ui);
                    }
                });

                this.element.resizable(options);

                this.element.on('area-resize-stop', function (event, params) {
                    var newRect = oThis.getRect();
                    var isGrid = this.classList.contains("grid-stack-item");
                    
                    if (params.rect) newRect = params.rect;

                    if(oThis.rect.top == newRect.top && oThis.rect.left == newRect.left && oThis.rect.right == newRect.right && oThis.rect.bottom == newRect.bottom) {
                        event.stopImmediatePropagation();
                    }
                    else {
                        oThis.rect = newRect;
                    }
                });
            }

            if (this.options.toggler) {
                this.element.append(
                    $(document.createElement('div'))
                        .attr({
                            title: 'Close'
                        })
                        .addClass('toggler')
                        .addClass(this.options.toggler)
                        .click(function () {
                            var area = $(this).closest('.layout-area');

                            oThis.sendEvent('area-resize-start', { area: area, border: oThis.options.toggler });

                            if (oThis.options.toggler == 'east') {
                                if (area.is('.collapsed')) {
                                    area.width(area.data('prev-width'));
                                }
                                else {
                                    area.data('prev-width', area.width()).width(0);
                                }
                            }
                            else if (oThis.options.toggler == 'west') {
                                if (area.is('.collapsed')) {
                                    area.css({ left: area.position().left - area.data('prev-width') });
                                    area.width(area.data('prev-width'));
                                }
                                else {
                                    area.css({ left: area.position().left + area.width() });
                                    area.data('prev-width', area.width()).width(0);
                                }
                            }
                            else if (oThis.options.toggler == 'south') {
                                if (area.is('.collapsed')) {
                                    area.height(area.data('prev-height'));
                                }
                                else {
                                    area.data('prev-height', area.height()).height(0);
                                }
                            }
                            else if (oThis.options.toggler == 'north') {
                                if (area.is('.collapsed')) {
                                    area.css({ top: area.position().top - area.data('prev-height') });
                                    area.height(area.data('prev-height'));
                                }
                                else {
                                    area.css({ top: area.position().top + area.height() });
                                    area.data('prev-height', area.height()).height(0);
                                }
                            }

                            if (area.is('.collapsed')) {
                                area.removeClass('collapsed');
                                $('.layout-infobox', area).show();
                                $(this).attr({ title: 'Close' })
                            }
                            else {
                                area.addClass('collapsed');
                                $('.layout-infobox', area).hide();
                                $(this).attr({ title: 'Open' })
                            }

                            oThis.sendEvent('area-resize-move', { border: oThis.options.toggler });
                            oThis.sendEvent('area-resize-stop');

                            var neighbours = oThis.neighbours.north.concat(oThis.neighbours.south).concat(oThis.neighbours.west).concat(oThis.neighbours.east);
                            
                            $(neighbours).each(function(index, area) {
                                $(area).area('sendEvent', 'area-resize-stop');
                            });
                        })
                );
            }

            if (this.options.rect)
                this.setRect(this.options.rect);

            this.setFocus();

            this.sendEvent('area-create');
            
            this.rect = this.getRect();

            $('div.layout-area[data-id="' + this.element.data('id') + '"]:visible').livequery(function() {
                oThis.sendEvent('area-shown', { area: oThis });
            });
        },

        findNeighbours: function() {
            var oThis = this;
            
            var delta = 5;
            
            this.neighbours = {
                north: [],
                east: [],
                south: [],
                west: []
            };

            var rect = this.getRect();

            this.element.siblings('.layout-area').each(function(index, area) {
                var rect2 = $(area).area('getRect');

                if(rect.top > 0) {
                    if (Math.abs(rect.top - rect2.bottom) < delta && rect.left < rect2.right && rect2.left < rect.right) {
                        oThis.neighbours.north.push(area);
                    }
                }

                if(rect.left > 0) {
                    if(Math.abs(rect.left - rect2.right) < delta && rect.top < rect2.bottom && rect2.top < rect.bottom) {
                        oThis.neighbours.west.push(area);
                    }
                }

                if(rect.right < oThis.element.parent().width()) {
                    if(Math.abs(rect.right - rect2.left) < delta && rect.top < rect2.bottom && rect2.top < rect.bottom) {
                        oThis.neighbours.east.push(area);
                    }
                }

                if(rect.bottom < oThis.element.parent().height()) {
                    if(Math.abs(rect.bottom - rect2.top) < delta && rect.left < rect2.right && rect2.left < rect.right) {
                        oThis.neighbours.south.push(area);
                    }
                }
            });
        },

        _refresh: function () {
        },

        _destroy: function () {
        },

        height: function () {
            return this.element.height();
        },

        width: function () {
            return this.element.width();
        },

        sendEvent: function (name, params) {
            var event = window.event || new Event(name);
            if (params == null)
                params = {};

            $.extend(params, {
                oThis: this,
                infobox: this.element,
                rect: this.getRect(),
                position: {left: event.x, top: event.y},
                positionPage: {left: event.pageX, top: event.pageY}
            });
            
            params.size = {width: params.rect.right - params.rect.left, height: params.rect.bottom - params.rect.top};

            if (name == 'area-resize-start') {
                $(this.element).trigger('area-resize-start', params);
            }
            else if (name == 'area-resize-stop') {
                $(this.element).trigger('area-resize-stop', params);
            }
            else if (name == 'area-resize-move') {
                $(this.element).trigger('area-resize-move', params);
            }
            else if (name == 'area-drag-start') {
                $(this.element).trigger('area-drag-start', params);
            }
            else if (name == 'area-drag-stop') {
                $(this.element).trigger('area-drag-stop', params);
            }
            else if (name == 'area-drag-move') {
                $(this.element).trigger('area-drag-move', params);
            }
            else if (name == 'area-resize') {
                $(this.element).trigger({
                    type: 'area-resize',
                    target: this.element
                }, params);
            }
            else if (name == 'area-create') {
                $(this.element.parent()).trigger('area-create', params);
            }
            else if(name == 'area-shown') {
                $(this.element).trigger('area-shown', params);
            }
        },

        setRect: function (rect, options) {
            if(options != null) {
                this.element.animate({
                    top: (rect.top + this.margin) + 'px',
                    left: (rect.left + this.margin) + 'px',
                    width: (rect.right - rect.left - 2 * this.margin) + 'px',
                    height: (rect.bottom - rect.top - 2 * this.margin) + 'px'
                }, options);
            }
            else {
                this.element.css({
                    top: (rect.top + this.margin) + 'px',
                    left: (rect.left + this.margin) + 'px',
                    width: (rect.right - rect.left - 2 * this.margin) + 'px',
                    height: (rect.bottom - rect.top - 2 * this.margin) + 'px'
                });
            }
        },

        getRect: function () {
            var position = {
                top: parseInt(this.element.css('top')),
                left: parseInt(this.element.css('left')),
            };

            return {
                top: position.top,
                left: position.left,
                right: position.left + this.element.outerWidth(true),
                bottom: position.top + this.element.outerHeight(true)
            }
        },

        getAreaRect: function () {
            return this.rect;
        },
        
        setFocus: function (isMaximize) {

            var prevZIndex = parseInt($(this.element).css('z-index'));
            if(isNaN(prevZIndex))
                prevZIndex = 20;
            
            if(isMaximize) $(this.element).css({'z-index': 100});
            else $(this.element).css({'z-index': 18});
            
            $(this.element).siblings('.layout-area').each(function() {
                var zIndex = parseInt($(this).css('z-index'));
                
                if($(this).is(':visible') && !isNaN(zIndex)) {
                    if(zIndex >= prevZIndex) {
                        $(this).css('z-index', parseInt($(this).css('z-index')) - 1);
                    }
                }
            });
            
            return prevZIndex;
        },

        save: function () {
            var state = {
            };

            if (this.element.is('[id]'))
                state.id = this.element.attr('id');
            if (this.element.is('[data-top]'))
                state.top = this.element.data('top');
            if (this.element.is('[data-left]'))
                state.left = this.element.data('left');
            if (this.element.is('[data-right]'))
                state.right = this.element.data('right');
            if (this.element.is('[data-bottom]'))
                state.bottom = this.element.data('bottom');
            if (this.element.is('[data-size]'))
                state.size = this.element.data('size');

            return state;
        },

        getOptions: function () {
            return {
                rect: this.getAreaRect()
            }
        }
    });

}(jQuery));

function parseAreaOptions(element) {
    var options = {
        resizable: {}
    };

    if ($(element).is('[data-roundup]')) {
        options.roundup = parseInt($(element).data('roundup'));
        options.resizable.grid = [options.roundup, options.roundup];
        options.draggable.grid = [options.roundup, options.roundup];
    }

    if ($(element).is('[data-class]'))
        options.class = $(element).data('class');
    
    if ($(element).is('[data-resizable]'))
        options.resizable = $(element).data('resizable');

    if ($(element).is('[data-resizable-min-width]'))
        options.resizable.minWidth = $(element).data('min-width');

    if ($(element).is('[data-resizable-max-width]'))
        options.resizable.maxWidth = $(element).data('max-width');

    if ($(element).is('[data-resizable-min-height]'))
        options.resizable.minHeight = $(element).data('min-height');

    if ($(element).is('[data-resizable-max-height]'))
        options.resizable.maxHeight = $(element).data('max-height');

    if ($(element).is('[data-resizable-handles]'))
        options.resizable.handles = $(element).data('resizable-handles');

    if ($(element).is('[data-toggler]'))
        options.toggler = $(element).data('toggler');
    
    return options;
}
