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
//# sourceURL=jquery.record.array.js

$(function () {
    $.widget("recordviewer.record_array", {

        options: {
            spec: null,
            name: null,
        },

        _create: function () {
            if(this.element.prop("tagName") != 'TABLE') {
                console.log('ERROR: RECORD_ARRAY widget must be applyed to TABLE element only!');
                return;
            }

            this.generateTable();

            this.generateCotrolPanel();
        },

        generateCotrolPanel: function () {
            var oThis = this;

            this.controlPanel = $(document.createElement('div'))
                .addClass('array-control-panel hidden')
                .append(
                    $(document.createElement('span'))
                        .addClass('remove')
                        .attr({
                            'aria-hidden': true,
                            title: 'Remove'
                        })
                        .click(function () {
                            oThis.activeTr.remove();
                            oThis.controlPanel.addClass('hidden');

                            return false;
                        })
                )
                .hover(function() {
                    clearTimeout(oThis.hideControlPanelTimer);
                },
                function() {
                    $(this).addClass('hidden');
                })
                .appendTo(this.element.parent());
        },

        generateTable: function() {
            var oThis = this;

            this.element
                .addClass('field')
                .addClass(this.options.spec.type);

            var header = $(document.createElement('thead'))
                            .appendTo(this.element);

            var body = $(document.createElement('tbody'))
                            .appendTo(this.element);

            if(this.options.spec._children) {
                var headTr = $(document.createElement('tr')).appendTo(header);

                $(Object.keys(this.options.spec._children)).each(function(index, name) {
                    var child = oThis.options.spec._children[name];

                    $(document.createElement('th'))
                        .text(child.title)
                        .appendTo(headTr);
                });

                body.append( this._generateTableRow(this.options.spec._children) );

                var footerTr = $(document.createElement('tr'))
                    .append(
                        $(document.createElement('td'))
                            .attr({
                                colspan: Object.keys(this.options.spec._children).length
                            })
                            .append(
                                $(document.createElement('button'))
                                    .addClass('btn btn-default btn-xs pull-right')
                                    .attr({
                                        type: 'button'
                                    })
                                    .append(
                                        $(document.createElement('span'))
                                            .addClass('glyphicon glyphicon-plus')
                                            .attr({ 'aria-hidden': true })
                                    )
                                    .append( $(document.createTextNode(' Add')) )
                                    .click(function() {
                                        $(this).closest('tr').before( oThis._generateTableRow(oThis.options.spec._children) );
                                    })
                            )
                    )
                    .appendTo(body);
            }
        },

        _generateTableRow: function(items) {
            var oThis = this;

            var tr = $(document.createElement('tr'));

            $(Object.keys(items)).each(function(index, name) {
                var spec = items[name];

                var td = $(document.createElement('td'))
                    .appendTo(tr);
                
                if(spec.type == 'string') {
                    if(spec.constraint == 'choice') {
                        td.record_choice({
                            name: name,
                            spec: spec
                        });
                    }
                    else {
                        td.record_string({
                            name: name,
                            spec: spec
                        });
                    }
                }
                else if(spec.type == 'integer') {
                    if(spec.constraint == 'choice' || spec.constraint == 'search') {
                        td.record_choice({
                            name: name,
                            spec: spec
                        });
                    }
                    else {
                        td.record_integer({
                            name: name,
                            spec: spec
                        });
                    }
                }
                else if(spec.type == 'text') {
                        td.record_text({
                            name: name,
                            spec: spec
                        });
                }
                else if(spec.type == 'datetime') {
                        td.record_datetime({
                            name: name,
                            spec: spec
                        });
                }
                else if(spec.type == 'real') {
                        td.record_real({
                            name: name,
                            spec: spec
                        });
                }
                else if(spec.type == 'bool') {
                        td.record_bool({
                            name: name,
                            spec: spec
                        });
                }
/*
                else if(spec.type == 'list') {
                        td.record_list({
                            name: name,
                            spec: spec,
                            children: spec._children
                        });
                }
                else if(spec.type == 'array') {
                        td.record_array({
                            name: name,
                            spec: spec
                        });
                }
*/
                else {
                        td.record_base({
                            name: name,
                            spec: spec
                        });
                }
            });
/*
            $(document.createElement('td'))
                .css({ width: '25px' })
                .addClass('toolbar')
                .append(
                    $(document.createElement('span'))
                        .addClass('remove hidden')
                        .attr({
                            'aria-hidden': true,
                            title: 'Remove'
                        })
                        .click(function() {
                            $(this).closest('tr').remove();
                            return false;
                        })
                )
                .appendTo(tr);
*/
            tr.hover(function() {
                if ($('tr', oThis.element).length > 3) {
                    oThis.activeTr = $(this);

                    clearTimeout(oThis.hideControlPanelTimer);

                    oThis.controlPanel.removeClass('hidden').css({
                        top: $(this).position().top,
                        left: $(this).position().left + $(this).width()
                    });
                }
            },
            function() {
                if ($('tr', oThis.element).length > 3) {
                    oThis.hideControlPanelTimer = setTimeout(function () {
                        oThis.controlPanel.addClass('hidden');
                    }, 500);

                }
            });

            return tr;
        },
    });

}(jQuery));
