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
    $.widget("hive.hivetoolbar", {
        options: {
            minimizeDelay: 3000    //    ms
        },

        _create: function () {
            var oThis = this;

            var status = $.sessionStorage.get('header-status');

            if(status == null) {
                //    status is unknown... so, default behaviour is to initiate minimization process 
                var timer = setTimeout(oThis._minimize, oThis.options.minimizeDelay);
                $(document).on('mousemove', function(e) {
                    clearInterval(timer);
                    timer = setTimeout(oThis._minimize, oThis.options.minimizeDelay);
                });
            }
            else if(status == 'maximized') {
                
            }
            else if(status == 'minimized') {
                $('#header').addClass('minimized');
                $('div.content').css({ top: '45px' });
            }

            $('div.bottom', this.element).append(
                $(document.createElement('div'))
                    .addClass('control')
                    .append(
                        $(document.createElement('a'))
                            .attr({ href: '#' })
                            .click(function() {
                                if($('#header').is('.minimized')) {
                                    oThis._maximize();
                                }
                                else {
                                    oThis._minimize();
                                }
                
                                return false;
                            })
                    )
            );
        },
        
        _minimize: function() {
            $('#header').addClass('minimized', 400);

            $("div.content").animate({ top: '45px' }, { done: function() { $(window).trigger('resize'); } });

                  $(document).unbind('mousemove');
               
                  $.sessionStorage.set('header-status', 'minimized');
        },

        _maximize: function() {
            $('#header').removeClass('minimized', 400);

            $("div.content").animate({ top: '120px' }, { done: function() { $(window).trigger('resize'); } });

                  $.sessionStorage.set('header-status', 'maximized');
        }
    });
}(jQuery));
