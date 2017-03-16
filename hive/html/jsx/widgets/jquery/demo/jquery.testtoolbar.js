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
    $.widget("view.testtoolbar", {
        _create: function () {
            var oThis = this;

            this.toolbar = $(document.createElement('div'))
                                .addClass('toolbar')
                                .appendTo(this.element);

            this.addTabButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-plus-sign')
                                )
                                .append(
                                        $(document.createTextNode(' Add Tab'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();

                                    manager.append({
                                        layout: {
                                            items:[
                                            {
                                                id: 'tabs1',
                                                tabs:{
                                                    items: [{
                                                        active:true,
                                                        title:'New Tab',
                                                        name:'new',
                                                        class: 'details icon',
                                                        allowClose: true,
                                                        view: {
                                                            name: 'areastats'
                                                        }
                                                    }]
                                                }
                                            }]
                                        }
                                    });
                                })
                                .appendTo(this.toolbar);
            
            this.removeTabButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-minus-sign')
                                )
                                .append(
                                        $(document.createTextNode(' Remove Tab'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.remove('new');
                                })
                                .appendTo(this.toolbar);

            this.clearTabButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-trash')
                                )
                                .append(
                                        $(document.createTextNode(' Clear Tab'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.clear('new');
                                })
                                .appendTo(this.toolbar);
            
            this.addAreaButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-plus-sign')
                                )
                                .append(
                                        $(document.createTextNode(' Add Area'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                    
                                    manager.append({
                                        layout: {
                                            items:[
                                            {
                                                    id: 'tabs4',
                                                top: '40%',
                                                left: '70%',
                                                right: '100%',
                                                bottom: '100%',
                                                allowMaximize: true,
                                                allowClose: true,
                                                tabs:{
                                                    items: [{
                                                        active:true,
                                                        title:'Graph',
                                                        name:'graph',
                                                        class: 'images icon',
                                                        view: {
                                                            name: 'areastats'
                                                            /*name: 'dataview',
                                                            options: {
                                                                dataViewer: 'vjGoogleGraphView',
                                                                dataViewerOptions: {
                                                                    data: "dsPieChart",
                                                                    type:"pie",
                                                                    series:[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence' } ],
                                                                    options: { title:'ACGT base count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: 600, height: 600, colors:vjPAGE.colorsACGT,  vAxis: { title: 'Letter Count', minValue:0}  },
                                                                    cols:[{ name: 'letter', order:1, align:'center',title: 'Nucleobase', hidden: false },
                                                                          { name: 'count', order:2,  title: 'Count', hidden: false },
                                                                          { name: 'quality', hidden: true }]
                                                                }
                                                            }*/
                                                        }
                                                    }]
                                                }
                                            }]
                                        }
                                    });
                                })
                                .appendTo(this.toolbar);

            this.removeAreaButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-minus-sign')
                                )
                                .append(
                                        $(document.createTextNode(' Remove Area'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.remove('tabs4');
                                })
                                .appendTo(this.toolbar);

            this.hideAreaButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-resize-small')
                                )
                                .append(
                                        $(document.createTextNode(' Hide Area'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.hide('tabs1');
                                })
                                .appendTo(this.toolbar);

            this.showAreaButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-resize-full')
                                )
                                .append(
                                        $(document.createTextNode(' Show Area'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.show('tabs1');
                                })
                                .appendTo(this.toolbar);

            
            this.clearAreaButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-trash')
                                )
                                .append(
                                        $(document.createTextNode(' Clear Area'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.clear('tabs4');
                                })
                                .appendTo(this.toolbar);

            this.hideTabButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-resize-small')
                                )
                                .append(
                                        $(document.createTextNode(' Hide Tab'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.hide('help');
                                })
                                .appendTo(this.toolbar);
            

            this.showTabButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-resize-full')
                                )
                                .append(
                                        $(document.createTextNode(' Show Tab'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.show('help');
                                    
                                    return false;
                                })
                                .appendTo(this.toolbar);

            this.moveAreaButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-arrow-right')
                                )
                                .append(
                                        $(document.createTextNode(' Move Area'))
                                )
                                .click(function() {
                                    var manager = $.getLayoutManager();
                                    manager.moveArea('tabs1', { top: '40%', left: '70%', right: '100%', bottom: '100%' }, 500);
                                    
                                    return false;
                                })
                                .appendTo(this.toolbar);

            this.modalWindowButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-film')
                                )
                                .append(
                                        $(document.createTextNode(' Modal Window'))
                                )
                                .click(function() {
                                    var config = {
                                            layout: {
                                              border: 'between',
                                            layoutType:'horizontal',
                                              items:[{
                                                size: '50%',
                                                overflow: 'hidden',
                                                view: {
                                                    name: 'dataview',
                                                    options: {
                                                        dataViewer: 'vjGoogleGraphView',
                                                        dataViewerOptions: {
                                                            data: "dsPieChart",
                                                            type:"pie",
                                                            series:[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence' } ],
                                                            options: { title:'ACGT base count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: 600, height: 600, colors:vjPAGE.colorsACGT,  vAxis: { title: 'Letter Count', minValue:0}  },
                                                            cols:[{ name: 'letter', order:1, align:'center',title: 'Nucleobase', hidden: false },
                                                                  { name: 'count', order:2,  title: 'Count', hidden: false },
                                                                  { name: 'quality', hidden: true }]
                                                        }
                                                     }
                                                }
                                              }, {
                                                size: '50%',
                                                overflow: 'hidden',
                                                view: {
                                                    name: 'dataview',
                                                    options: {
                                                        dataViewer: 'vjGoogleGraphView',
                                                        dataViewerOptions: {
                                                            data: "dsPieChart",
                                                            type:"pie",
                                                            series:[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence' } ],
                                                            options: { title:'ACGT base count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: 600, height: 600, colors:vjPAGE.colorsACGT,  vAxis: { title: 'Letter Count', minValue:0}  },
                                                            cols:[{ name: 'letter', order:1, align:'center',title: 'Nucleobase', hidden: false },
                                                                  { name: 'count', order:2,  title: 'Count', hidden: false },
                                                                  { name: 'quality', hidden: true }]
                                                        }
                                                     }
                                                }
                                              }]
                                            }
                                    };

                                    $('#testModalWindow').on('shown.bs.modal', function (e) {
                                        $('#testModalLayout').layoutmanager({
                                            type: 'horizontal',
                                            saveState: false,
                                            config: config
                                        });
                                    });
                                        
                                    $('#testModalWindow').modal('show');
                                    
                                    return false;
                                })
                                .appendTo(this.toolbar);

            $('body').append(this.buildModalWindow());
        },
        
        buildModalWindow: function() {
            var wnd = $(document.createElement('div'))
                            .addClass('modal fade')
                            .attr({
                                id: 'testModalWindow',
                                tabindex: -1,
                                role: "dialog",
                                'aria-labelledby': "testModalLabel"
                            })
                            .append(
                                $(document.createElement('div'))
                                    .addClass('modal-dialog modal-lg')
                                    .attr({ role: "document" })
                                    .append(
                                        $(document.createElement('div'))
                                            .addClass('modal-content')
                                            .append(
                                                $(document.createElement('div'))
                                                    .addClass('modal-header')
                                                    .append(
                                                        $(document.createElement('button'))
                                                            .addClass('close')
                                                            .attr({
                                                                type: "button",
                                                                'data-dismiss': "modal",
                                                                'aria-label': "Close"
                                                            })
                                                            .append(
                                                                $(document.createElement('span'))
                                                                    .attr({ 'aria-hidden': "true" })
                                                                    .html('&times;')
                                                            )
                                                    )
                                                    .append(
                                                        $(document.createElement('h4'))
                                                            .addClass('modal-title')
                                                            .attr({ id: 'testModalLabel' })
                                                            .text('Modal title')
                                                    )

                                            )
                                            .append(
                                                $(document.createElement('div'))
                                                    .addClass('modal-body')
                                                    .append(
                                                        $(document.createElement('div'))
                                                        .attr({ id: 'testModalLayout' })
                                                    )
                                            )
                                            .append(
                                                $(document.createElement('div'))
                                                    .addClass('modal-footer')
                                                    .append(
                                                        $(document.createElement('button'))
                                                            .addClass('btn btn-default')
                                                            .attr({
                                                                type: "button",
                                                                'data-dismiss': "modal",
                                                                'aria-label': "Close"
                                                            })
                                                            .text('Close')
                                                    )
                                            )
                                    )
                            )
            return wnd;
        }
    });
}(jQuery));
