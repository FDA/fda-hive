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
    $.widget("layout.infoboxtabs", $.layout.infobox, {

        options: {
            //droppable: {
                //accept: function (ui) {
                //    return $(ui.context).is('.layout-area:has(div.layout-infobox-tabs), .layout-infobox-tabs .nav-tabs > li');
                //}
            //}
        },

        onInfoboxInit: function () {
            var oThis = this;

            this.infobox.addClass("layout-infobox-tabs");

            this.adjustHeight();

            if (this.options.droppable) {
                this._initTabsDraggable();

                $('.tab-pane', this.infobox).each(function (index, tabPane) {
                    if ($('div[data-layout~="manager"]', this).length > 0) {
                        $('div[data-layout~="manager"]:first', this)
                            .area({
                                rect: {
                                    top: 0,
                                    left: 0,
                                    right: $(this).outerWidth(),
                                    bottom: $(this).outerHeight()
                                },
                                resizable: false
                            })
                            .layoutmanager();
                    }

                    $(tabPane).on('area-resize-move', function (event) {
                        //    if tab pane contains layout manager inside and if the areas inside 
                        //    are resized we must stop event propagation here
                        event.stopPropagation();
                    });
                });
            }

            //    in case if any layout manager placed inside the tab(s) we have to notify it about area resize...
            oThis.element.on('area-resize', function (event, params) {
                $.log('jquery.layout.tabs.js: area-resize');
                
                $('.tab-pane', oThis.infobox).each(function (index, tabPane) {
                    if($(tabPane).children('div.layout-manager').length > 0) {
                        var area = $(tabPane).children('div.layout-manager').first().area('instance')

                        area.setRect({
                            top: 0,
                            left: 0,
                            right: $(tabPane).outerWidth(),
                            bottom: $(tabPane).outerHeight()
                        });

                        area.sendEvent('area-resize');
                    }
                });
            });

            oThis.element.on('area-resize-stop', function (event, params) {
                $('.tab-pane', oThis.infobox).each(function (index, tabPane) {
                    if($(tabPane).children('div.layout-manager').length > 0) {
                        var area = $(tabPane).children('div.layout-manager').first().area('instance')

                        area.sendEvent('area-resize-stop');
                    }
                });
            });

            //    tabs infobox is initialized and we have to check if it has another layout manager inside...
            if (!this.element.is('div[data-layout~="manager"]')) {
                $('div[data-layout~="manager"]', this.element).each(function (index, manager) {
                    var options = parseAreaOptions(this);

                    options.type = $(this).data('layout-manager');
                    options.resizable = false;

                    $(this).layoutmanager(options);
                });
            }

            /*$(this.element).on('shown.bs.tab', 'a[data-toggle="tab"]', function (e) {
                oThis.sendEvent('state-changed');
                oThis.sendEvent('tab-shown', {
                    index: $(e.target).parent().index(),
                    tab: $(e.target).parent(),
                    tabs: oThis.getTabsInfo().tabs
                });
            });*/

            this.element.on('init-widgets', function () {
                oThis.adjustHeight();
                oThis._initWidgets();
            });

            this.element.on('tab-close', function () {
                //    if we close active tab...
                if ($('li.active', this).length == 0) {
                    if ($('li', this).length > 0)
                        $('li', this).first().find('a').click();
                }
            })
            .on('tab-hide', function() {
                //    if we hide active tab...
                if ($('li.active:visible', this).length == 0) {
                    if ($('li', this).length > 0)
                        $('li:visible', this).first().find('a').click();
                }
            });

            this.sendEvent('tabs-create', { tabs: this.element });

            this.element.on('new-tab-added', function(event, tab) {
                oThis._initTabDraggable(tab.tab);
            });

            this.element.on('area-shown', function (event, params) {
                oThis.adjustHeight();
            });

            //this.element.on('maximize', function (event, params) {
            //});

            //this.element.on('minimize', function (event, params) {
            //});

            $('.nav-tabs > li.active', this.element).livequery(function() {
                oThis.sendEvent('state-changed');
                oThis.sendEvent('tab-shown', {
                    index: $(this).index(),
                    tab: this,
                    tabs: oThis.getTabsInfo().tabs
                });
                oThis.sendEvent('tab-active', {
                    index: $(this).index(),
                    name: $('a', this).attr('data-name'),
                    area: $(this).closest('div.layout-area').attr('data-id'),
                    tabs: oThis.getTabsInfo().tabs
                });
            }, function() {
                oThis.sendEvent('tab-inactive', {
                    index: $(this).index(),
                    name: $('a', this).attr('data-name'),
                    area: $(this).closest('div.layout-area').attr('data-id'),
                    tabs: oThis.getTabsInfo().tabs
                });
            });
        },

        _initTabsDraggable: function() {
            var oThis = this;

            if (this.element.draggable) {
                $('.nav-tabs > li', this.element).each(function() {
                    oThis._initTabDraggable(this);
                });
            }
        },

        _initTabDraggable: function(li) {
            var oThis = this;

            if (this.element.draggable) {
                $(li).draggable({
                    revert: true,
                    distance: 10,
                    start: function (event, ui) {
                        oThis.area.setFocus();
                    },
                    stop: function (event, ui) {
                        //console.log('drag stops...')
                        oThis._hideSplitAreas();
                    }
                });
            }
        },
        
        // events bound via _bind are removed automatically
        // revert other modifications here
        _destroy: function () {
        },

        sendEvent: function (name, params) {
            if (params == null)
                params = {};

            if (name == 'tabs-create') {
                $(this.element).trigger('tabs-create', params);
            }
            else if (name == 'tab-shown') {
                $(this.element).trigger('tab-shown', params);
            }
            else if (name == 'tab-active') {
                $(this.element).trigger('tab-active', params);
            }
            else if (name == 'tab-inactive') {
                $(this.element).trigger('tab-inactive', params);
            }
            else {
                // Invoke the parent widget's sendEvent().
                this._super(name, params);
            }
        },

        _dropHandler: function (direction, area, ui) {
            if ($.contains(this.element.get(0), ui.draggable.get(0)) && ($(area).is('.north-split-area') || $(area).is('.center-split-area'))) {
                ui.draggable.draggable('option', 'revert', true);
                return;
            }

            this.sendEvent('split-area', {
                area: this.element,
                dropArea: area,
                direction: direction,
                ui: ui
            });

            this._super(name, area, ui);
        },

        moveTab: function (li) {
            var oThis = this;

            var fromInfobox = $(li).closest('.layout-infobox-tabs');

            var panelId = $('a', li).attr('href');

            var panel = $(panelId, fromInfobox).detach();

            var tab = $(li).detach().css({ top: '', left: '' });

            $('ul.nav-tabs', this.element).append(tab);
            $('div.panel-body', this.element).append(panel);

            if ($(tab).hasClass('active')) {
                //  need to deactivate previously active tab...
                $('.nav-tabs > li', this.element).removeClass('active');
                $(tab).addClass('active');

                $('.panel-body > div.tab-pane.active', this.element).removeClass('active');
                $(panel).addClass('active');
            }

            if ($('.nav-tabs > li.active', fromInfobox).length == 0) {
                //  source infobox doesn't have any active tab anymore...
                //  we have to activate one... let's say first one
                $('.nav-tabs > li:first', fromInfobox).addClass('active');
                $('.panel-body > div.tab-pane:first', fromInfobox).addClass('active');

                /*oThis.sendEvent('tab-shown', {
                    index: 0,
                    tab: $('.nav-tabs > li:first', fromInfobox).first(),
                    tabs: fromInfobox.parent().infoboxtabs('getTabsInfo').tabs
                });*/
            }

            //  if infobox does't have any tabs we probably have to close it...
            if ($('ul.nav-tabs > li', fromInfobox).length == 0) {
                if (fromInfobox.parent().is('.layout-area'))
                    fromInfobox.parent().remove();
            }

            if ($('ul.nav-tabs > li', this.element).length == 1) {
                $('.nav-tabs > li', this.element).addClass('active');
                $('.panel-body > div.tab-pane', this.element).addClass('active');

                /*oThis.sendEvent('tab-shown', {
                    index: 0,
                    tab: $('.nav-tabs > li:first', this.element).first(),
                    tabs: oThis.getTabsInfo().tabs
                });*/
            }

            this.adjustHeight();

            this.area.sendEvent('area-resize');
            this.area.sendEvent('area-resize-stop');
        },

        moveInfobox: function (fromInfobox, toInfobox) {
            $('ul.nav-tabs', toInfobox).append(
                $('ul.nav-tabs > li', fromInfobox).removeClass('active').detach()
            );

            $('div.panel-body', toInfobox).append(
                $('.panel-body > div.tab-pane', fromInfobox).removeClass('active').detach()
            );

            fromInfobox.remove();

            this.adjustHeight();
        },
        //    Save config information in order to reproduse the same Tabs infobos later on the fly
        save: function () {
            var oThis = this;

            var config = this._super();

            $.extend(config, {
                type: 'tabs',
                tabs: {
                    items: []
                }
            });

            $('ul.nav-tabs', this.element).children('li').each(function (index, li) {
                var a = $('a', li);
                var pane = $(a.attr('href'), oThis.element);

                var tabConfig = {
                    active: $(li).is('.active'),
                    title: a.text(),
                    name: a.data('name'),
                    class: a.attr('class'),
                    overflow: pane.css('overflow')
                };

                if (pane.children().length == 1 && pane.children().first().is('div.layout-manager')) {
                    //  layout manager inside panel...
                    $.extend(tabConfig, pane.children().first().layoutmanager('save'));
                }
                else {
                    if (pane.data('jquery-widget')) {
                        tabConfig.view = {
                            name: pane.data('jquery-widget')
                        };
                    }
                    if (pane.data('jquery-widget-options')) {
                        $.extend(tabConfig.view, {
                            options: pane.data('jquery-widget-options')
                        });
                    }
                }

                config.tabs.items.push(tabConfig);
            });

            return config;
        },

        getTabsInfo: function() {
            var info = {
                tabs: []
            };

            $('ul.nav-tabs', this.element).children('li').each(function (index, li) {
                var a = $('a', li);

                info.tabs.push({
                    active: $(li).is('.active'),
                    title: a.text(),
                    name: a.data('name'),
                    class: a.attr('class'),
                });
            });
            
            return info;
        },

        buildTabConfig: function(name) {
            var config = {};

            var a = $('ul.nav-tabs > li > a[data-name="' + name + '"]', this.element);

            if(a.length == 0) {
                console.log('ERROR: cannot find tab with name: ' + name);
            }
            else {
                var li = a.parent();
                var pane = $(a.attr('href'), this.element);

                $.extend(config, {
                    active: $(li).is('.active'),
                    title: a.text(),
                    name: a.data('name'),
                    class: a.attr('class'),
                    allowClose: a.data('allow-close'),
                    allowHide: a.data('allow-hide'),
                    allowClone: a.data('allow-clone'),
                    overflow: pane.css('overflow')
                });

                if (pane.children().length == 1 && pane.children().first().is('div.layout-manager')) {
                    //  layout manager inside panel...
                    $.extend(config, pane.children().first().layoutmanager('save'));
                }
                else {
                    if (pane.data('jquery-widget')) {
                        config.view = {
                            name: pane.data('jquery-widget')
                        };
                    }
                    if (pane.data('jquery-widget-options')) {
                        $.extend(config.view, {
                            options: pane.data('jquery-widget-options')
                        });
                    }
                }
            }

            return config;
        },

        adjustHeight: function () {
            this._super();

            $('.tab-pane', this.infobox)
                .height(this.height() - parseInt($('.panel-heading', this.infobox).css('height')))
                .width(this.width());
        }
    });

}(jQuery));
