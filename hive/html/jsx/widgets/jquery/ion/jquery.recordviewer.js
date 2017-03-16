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
//# sourceURL=jquery.recordviewer.js

$(function () {
    $.widget("ion.ion_recordviewer", $.ion.field_complex, {
        options: {
            url: null,
            spec: null,
            collapsed: true,
            style: 'inline'    //    form|inline
        },

        _create: function () {
            var self = this;

            this.element.addClass('record-viewer').addClass(this.options.style);

            this.options.recordviewer = this;

            $.ion.ready(function() {
                if(self.options.url) {
                    $.ajax({
                        dataType: "json",
                        url: self.options.url,
                        success: function(data) {
                            //    TODO: implement reading JSON object from URL.
                        }
                    });
                }
                else if(self.options.spec) {
                    self._super();

                    self.element.trigger('ready');
                    //that.showJson(that.options.json)
                }
            });

            $('.primitive', this.element).livequery(function () {
                $(this).hover(function () {
                    if ($(this).is('.changed') || $(this).closest('.field-container').is('.plural') && $(this).siblings('.primitive').length > 0) {
                        clearTimeout(self.hideControlPanelTimer);

                        if ($(this).closest('.field-container').is('.plural') && $(this).siblings('.primitive').length > 0)
                            $('.remove', self.controlPanel).show();
                        else
                            $('.remove', self.controlPanel).hide();

                        if ($(this).is('.changed')) {
                            $('.undo,.save', self.controlPanel).show();
                        }
                        else {
                            $('.undo,.save', self.controlPanel).hide();
                        }

                        var scrollOffset = 0;

                        $(this).parents().each(function() {
                            scrollOffset += $(this).scrollTop();
                        });

                        var position = {
                            top: $(this).offset().top - $(self.element).offset().top + scrollOffset,
                            left: $(this).offset().left - $(self.element).offset().left + $(this).outerWidth(true)
                        };

                        self.controlPanel.removeClass('hidden').css(position);

                        //    remember element that triggered the control panel... 
                        self.hoveredElement = this;
                    }
                },
                function () {
                    clearTimeout(self.hideControlPanelTimer);
                    self.hideControlPanelTimer = setTimeout(function () {
                        self.hideCotrolPanel();
                    }, 500);
                });
            });

            $('.complex', this.element).livequery(function() {
                $(this).hover(function () {
                    if ($(this).siblings('.complex').length > 0) {
                        clearTimeout(self.hideControlPanelTimer);

                        if ($(this).siblings('.complex').length > 0)
                            $('.remove', self.controlPanel).show();
                        else
                            $('.remove', self.controlPanel).hide();

                        if ($(this).is('.changed')) {
                            $('.undo,.save', self.controlPanel).show();
                        }
                        else {
                            $('.undo,.save', self.controlPanel).hide();
                        }

                        var scrollOffset = 0;

                        $(this).parents().each(function() {
                            scrollOffset += $(this).scrollTop();
                        });
                        
                        var position = {
                            top: $(this).offset().top - $(self.element).offset().top + scrollOffset,
                            left: $(this).offset().left - $(self.element).offset().left + $(this).outerWidth(true)
                        };

                        self.controlPanel.removeClass('hidden').css(position);

                        //    remember element that triggered the control panel... 
                        self.hoveredElement = this;
                    }

                    return false;
                },
                function () {
                    clearTimeout(self.hideControlPanelTimer);
                    self.hideControlPanelTimer = setTimeout(function () {
                        self.hideCotrolPanel();
                    }, 500);
                });
            });

            this.generateCotrolPanel();

            $(".field-container").initialize(function (index, elem) {
                if ($(elem).length == 0 || $(elem).parents('html').length == 0)
                    return;

                switch ($(elem).attr('data-type')) {
                    case 'int':
                        $(elem).field_int('runEvalProcessor');
                        break;
                    case 'uint':
                        $(elem).field_uint('runEvalProcessor');
                        break;
                    case 'memory':
                        $(elem).field_memory('runEvalProcessor');
                        break;
                    case 'string':
                        $(elem).field_string('runEvalProcessor');
                        break;
                    case 'hiveid':
                        $(elem).field_hiveid('runEvalProcessor');
                        break;
                    case 'password':
                        $(elem).field_password('runEvalProcessor');
                        break;
                    case 'email':
                        $(elem).field_email('runEvalProcessor');
                        break;
                    case 'bool':
                        $(elem).field_bool('runEvalProcessor');
                        break;
                    case 'text':
                        $(elem).field_text('runEvalProcessor');
                        break;
                    case 'cmdline':
                        $(elem).field_cmdline('runEvalProcessor');
                        break;
                    case 'datetime':
                        $(elem).field_datetime('runEvalProcessor');
                        break;
                    case 'keyval':
                        $(elem).field_keyval('runEvalProcessor');
                        break;
                    default:
                        $(elem).field_complex('runEvalProcessor');
                        break;
                }
            });
        },

        generateCotrolPanel: function () {
            var self = this;

            this.controlPanel = $(document.createElement('div'))
                .addClass('field-control-panel hidden')
                .append(
                    $(document.createElement('span'))
                        .addClass('remove glyphicon glyphicon-remove-circle')
                        .attr({
                            'aria-hidden': true,
                            title: 'Remove'
                        })
                        .hover(function () {
                            return false;
                        },
                        function () {
                            return false;
                        })
                        .click(function () {
                            //    initiate event to delete element...
                            $(self.hoveredElement).trigger('delete-field');

                            self.hideCotrolPanel();

                            return false;
                        })
                )
                .append(
                    $(document.createElement('span'))
                        .addClass('undo glyphicon glyphicon-share-alt')
                        .attr({
                            'aria-hidden': true,
                            title: 'Undo'
                        })
                        .hover(function () {
                            return false;
                        },
                        function () {
                            return false;
                        })
                        .click(function () {
                            //    initiate event to rollback changes...
                            $(self.hoveredElement).trigger('rollback-field');

                            self.hideCotrolPanel();

                            return false;
                        })
                )
                .append(
                    $(document.createElement('span'))
                        .addClass('save glyphicon glyphicon-floppy-save')
                        .attr({
                            'aria-hidden': true,
                            title: 'Save'
                        })
                        .hover(function () {
                            return false;
                        },
                        function () {
                            return false;
                        })
                        .click(function () {
                            return false;
                        })
                )
                .hover(function () {
                    clearTimeout(self.hideControlPanelTimer);
                },
                function () {
                    self.hideCotrolPanel();
                })
                .appendTo(this.element);
            /*
                        if(this.element.is('.plural')) {
                            $('.primitive,.ion-type', this.container).livequery(function () {
                                if($(this).closest('div.field-container').is('.plural')) {
                                    $(this).hover(function() {
                                        if(self.container.children('.primitive,.ion-type').length > 1 ||
                                            self.container.children('table').length == 1 && self.container.children('table').children('tbody').children('tr').length > 1) {
                                            self.activeField = $(this);

                                            clearTimeout(self.hideControlPanelTimer);

                                            self.controlPanel.removeClass('hidden').css({
                                                top: $(this).position().top,
                                                left: $(this).position().left + $(this).outerWidth(true)
                                            });
                                        }
                                    },
                                    function() {
                                        self.hideControlPanelTimer = setTimeout(function () {
                                            self.controlPanel.addClass('hidden');
                                        }, 500);
                                    });
                                }
                            });
                        }
            */
        },

        hideCotrolPanel: function() {
            this.controlPanel.addClass('hidden');
        }
    });
}(jQuery));
