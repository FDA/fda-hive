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
    $.widget("view.test_horizontal_toolbar", {
        _create: function () {
            var oThis = this;

            this.toolbar = $(document.createElement('div'))
                                .addClass('toolbar')
                                .appendTo(this.element);

            this.addViewButton = $(document.createElement('button'))
                                .attr({
                                    type: 'button'
                                })
                                .addClass('btn btn-default')
                                .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-plus-sign')
                                )
                                .append(
                                        $(document.createTextNode(' Add View'))
                                )
                                .click(function() {
                                    var dataViews = $('#dynamicViews').dataviews('instance');
                                    
                                    dataViews.add({
                                        instance: new vjGoogleGraphView({
                                            id: 'dataview_' + Math.floor((Math.random() * 10000) + 1),
                                            data: "dsPieChart",
                                            type:"pie",
                                            series:[ {label:true, name:'letter', title: 'Nucleotides'}, {name: 'count', title: 'Count at position on a sequence' } ],
                                            options: { title:'ACGT base count', legend: 'none',is3D:true,pieSliceText: 'label', focusTarget:'category', width: 600, height: 600, colors:vjPAGE.colorsACGT,  vAxis: { title: 'Letter Count', minValue:0}  },
                                            cols:[{ name: 'letter', order:1, align:'center',title: 'Nucleobase', hidden: false },
                                                  { name: 'count', order:2,  title: 'Count', hidden: false },
                                                  { name: 'quality', hidden: true }]
                                        }),
                                        size: '300',
                                        allowClose: true
                                    });
                                })
                                .appendTo(this.toolbar);
        },
    });
}(jQuery));
