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
    $.widget("layout.gridstackArea", {

        options: {
            rect: null,
            resizable: {
                minWidth: 50,
                minHeight: 50,
                containment: 'parent',
                grid: [20, 20]
            },
            toggler: null,
            tabsToAdd:[],
            margin:1
        },

        _create: function () {
            var oThis = this;
            
            var gridstackOptions = this.options.gridstackOptions;
            if (!gridstackOptions){
                gridstackOptions = {
                    cellHeight: 100,
                    draggable:{handle: '.panel-heading'},
                    width: 12,
                    resizable: {
                        autoHide:true
                    }
                };
            }

            this.areaName = "gridstack"+ Math.round(Math.random()*100);            
            
            this.gridDiv = $(document.createElement("div")).attr("data-areaType", "gridstack");
            this.gridDiv.gridstack(gridstackOptions);
            
            this.gridDiv.addClass("grid-stack");
            this.gridDiv.attr("grids-id", this.areaName)
            this.grid = $(this.gridDiv).data('gridstack');
            
            $(this.element).append(this.gridDiv);
            $(this.element).css("overflow-y", "auto");
            $(this.element).addClass("grid-stack-area");
            
            this._initResize();
            this._initDrag();
            
            return this.grid;
        },
        appendTabs: function (tabs, tabsInfo){
            var oThis = this;
            
            var gridWid = this.grid;
            var tabArea = tabsInfo.area ? tabsInfo.area : {defaultLoc:{}};
            var extraAreaInfo = {};
            $.extend (extraAreaInfo, tabArea);
            extraAreaInfo.node12 = tabArea.defaultLoc;

            tabs.attr("id", tabsInfo.name+"-gridstack");
            var tmp = $(document.createElement("div"))
                        .append($(document.createElement("div"))
                                .addClass("grid-stack-item-content")
                                .append(tabs)
                        );
            gridWid.addWidget(tmp, tabArea.defaultLoc.x, tabArea.defaultLoc.y, tabArea.defaultLoc.width, tabArea.defaultLoc.height,
                    undefined, undefined, undefined, undefined, undefined, tabsInfo.name, extraAreaInfo);
            tmp.css("position", "");
            
            tabs.on("area-resize-stop", function(event, parameters){
                if ($(this).hasClass("maximize")) return;
                $(this).height(parameters.rect.bottom - parameters.rect.top);
                $(this).width(parameters.rect.right - parameters.rect.left);
                if(parameters.oThis && parameters.oThis.options && parameters.oThis.options.layoutManager)
                    parameters.oThis.options.layoutManager._resizeArea(this, null, true);
            });
            $(tabs).css("overflow", "hidden");
        },
        appendAll: function(layoutManager){ 
            this.options.layoutManager = layoutManager;
            this._updateColumns(this.element);
        },
        
        closeFromInfo: function(infobox){
            var elForGrid = infobox.closest(".grid-stack-item");
            if (elForGrid.length > 0){
                this.grid.removeWidget(elForGrid);
                return true;
            }
            return false;
        },
       
        hideFromInfo: function(infobox){
            var returned = this.toggleArea(infobox, false);
            
            if (returned && this.options.algoManager){
                var tabToHide = $(infobox).closest("[data-layout='infobox tabs']").attr("id");
                tabToHide = tabToHide.substring(0, tabToHide.indexOf("-"));
                this.options.algoManager.closeTab({tabId: tabToHide}, true);
                
                return true;
            }
            else if (returned && this.options.layoutManager){
                var tabToHide = $(infobox).closest("[data-layout='infobox tabs']").attr("id");
                tabToHide = tabToHide.substring(0, tabToHide.indexOf("-"));
                this.options.layoutManager.closeTab({tabId: tabToHide}, true);
                
                return true;
            }
            return returned;
        },
        
        openArea: function(infobox){
            return this.toggleArea(infobox, true);
        },
        
        toggleArea: function(infobox, fadeIn){
            var elForGrid = infobox.closest(".grid-stack-item");
            if (elForGrid.length > 0 && fadeIn){
                elForGrid.fadeIn(100);
                return true;
            }
            else if (elForGrid.length > 0){
                elForGrid.fadeOut(100);
                return true;
            }
            return false;
        },

        _refresh: function () {
        },

        _destroy: function () {
            this.grid.destroy();
        },

        height: function () {
            return this.element.height();
        },

        width: function () {
            return this.element.width();
        },
        
        _updateColumns: function(elem){
            var windowWidth = $(elem).width();
            if(windowWidth <= 700){
                
            }else if(windowWidth > 700 && windowWidth <= 1000){
                this.grid.setGridWidth(2);
            }else if( windowWidth > 1000 && windowWidth <= 1500){
                this.grid.setGridWidth(3);
            }else if(windowWidth > 1500 && windowWidth <= 1800){
                this.grid.setGridWidth(4);
            }else{
                this.grid.setGridWidth(12);
            }
        },

        _initResize: function(){
            var oThis = this;
            this.element.on('resizestart', function(event, ui) {
                console.log("resize start");
                var element = event.target;
                oThis.sendEvent("area-resize-start", null, event.target);
            });
            
            this.element.on ("gsresizestop", function(event, ui){
                console.log ("resize stop");
                var element = event.target;
                oThis.sendEvent("area-resize-stop", null, ui);
            });
            this.element.on("area-resize-stop", function(event, params){
                var oThis = params.oThis;
                if(!oThis.options.layoutManager) return;

                oThis._updateColumns(this);    

                var allAreas = $(this).find(".grid-stack-item-content");
                allAreas.each(function (i, elem){
                    var curGrid = elem;
                    var curRect = oThis.getRect(curGrid);
                    var curArea = $(curGrid).find(".layout-area");
                    
                    curArea.height(curRect.bottom - curRect.top);
                    curArea.width(curRect.right - curRect.left);
                    oThis.options.layoutManager._resizeArea(curArea, null, true);
                    curArea.area("instance").sendEvent('area-resize-stop');
                });
            });
        },
        
        _initDrag: function(){
            var oThis = this;
            var gridWid = this.grid;
            this.gridDragEvents = gridWid._prepareElementsByNode();
            
            $(this.element).on("area-drag-start", function(event, ui){
                var oThis = $(this).gridstackArea("instance");
                var gridElem = event.target.closest(".grid-stack-item");
                if($(".grid-stack").offset()){
                    ui.position.left = ui.positionPage.left-$(".grid-stack").offset().left;
                    ui.position.top = ui.positionPage.top-$(".grid-stack").offset().top;
                }
                oThis.gridDragEvents.forStart(event, ui, gridElem);
                event.stopImmediatePropagation();
            });
            $(this.element).on("area-drag-stop", function(event, ui){
                var oThis = $(this).gridstackArea("instance");
                var areaElem = event.target;
                var gridElem = areaElem.closest(".grid-stack-item");
                if($(".grid-stack").offset()){
                    ui.position.left = ui.positionPage.left-$(".grid-stack").offset().left;
                    ui.position.top = ui.positionPage.top-$(".grid-stack").offset().top;
                }
                if(ui.fromResize != true){
                    $(areaElem).css("top", "0");
                    $(areaElem).css("left", "0");
                }
                else if (ui.fromResize){
                    var allGridElem = $(areaElem).find(".grid-stack-item");
                    
                    for(var i = 0; i < allGridElem.length; i++){
                        var curGrid = allGridElem[i];
                        var curRect = oThis.getRect(curGrid);
                        var curArea = $(curGrid).find(".layout-area");
                        
                        curArea.height(curRect.bottom - curRect.top);
                        curArea.width(curRect.right - curRect.left-10);
                        oThis.options.layoutManager._resizeArea(curArea, null, true);
                    }
                }
                oThis.gridDragEvents.forStop(event, ui, gridElem);
                event.stopImmediatePropagation();
            });
            $(this.element).on("area-drag-move", function(event, ui){
                var oThis = $(this).gridstackArea("instance");
                var gridElem = event.target.closest(".grid-stack-item");
                if($(".grid-stack").offset()){
                    ui.position.left = ui.positionPage.left-$(".grid-stack").offset().left;
                    ui.position.top = ui.positionPage.top-$(".grid-stack").offset().top;
                }
                oThis.gridDragEvents.forDrag(event, ui, gridElem);
            });
        },
        
        sendEvent: function (name, params, gridElem) {
            if (params == null)
                params = {};
            
            var area = this.element;
            if(gridElem){
                area = $(gridElem).find(".layout-area");
            }
            
            $.extend(params, {
                oThis: this,
                infobox: area,
                rect: this.getRect(gridElem)
            });

            
            if (name == 'area-resize-start') {
                $(area).trigger('area-resize-start', params);
            }
            else if (name == 'area-resize-stop') {
                $(area).trigger('area-resize-stop', params);
            }
            else if (name == 'area-resize-move') {
                $(area).trigger('area-resize-move', params);
            }
            else if (name == 'area-drag-start') {
                $(area).trigger('area-drag-start', params);
            }
            else if (name == 'area-drag-stop') {
                $(area).trigger('area-drag-stop', params);
            }
            else if (name == 'area-drag-move') {
                $(area).trigger('area-drag-move', params);
            }
            else if (name == 'area-resize') {
                $(area).trigger({
                    type: 'area-resize',
                    target: this.element
                }, params);
            }
            else if (name == 'area-create') {
                $(area.parent()).trigger('area-create', params);
            }
            else if(name == 'area-shown') {
                $(area).trigger('area-shown', params);
            }
        },

        setRect: function (rect, options) {
            if(options != null) {
                this.element.animate({
                    top: (rect.top + this.options.margin) + 'px',
                    left: (rect.left + this.options.margin) + 'px',
                    width: (rect.right - rect.left - 2 * this.options.margin) + 'px',
                    height: (rect.bottom - rect.top - 2 * this.options.margin) + 'px'
                }, options);
            }
            else {
                this.element.css({
                    top: (rect.top + this.options.margin) + 'px',
                    left: (rect.left + this.options.margin) + 'px',
                    width: (rect.right - rect.left - 2 * this.options.margin) + 'px',
                    height: (rect.bottom - rect.top - 2 * this.options.margin) + 'px'
                });
            }
        },

        getRect: function (elem) {
            if (!elem) elem = this.element;
            else elem = $(elem);
            
            var position = {
                top: parseInt(elem.css('top')),
                left: parseInt(elem.css('left')),
            };

            return {
                top: position.top,
                left: position.left,
                right: position.left + elem.outerWidth(true),
                bottom: position.top + elem.outerHeight(true)
            }
        },

        getAreaRect: function () {
            return this.rect;
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


