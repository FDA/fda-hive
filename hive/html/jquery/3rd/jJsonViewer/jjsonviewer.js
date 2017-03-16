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
!function($){

    'use strict';

    $.fn.jJsonViewer = function(jjson, options) {
    return this.each( function() {
        var self = $(this);
        if (typeof jjson == 'string') {
              self.data('jjson', jjson);
        }
        else if(typeof jjson == 'object') {
              self.data('jjson', JSON.stringify(jjson))
        }
        else {
              self.data('jjson', '');
        }
        new JJsonViewer(self, options);
    });
    };

    function JJsonViewer(self, options) {
        self.html('<ul class="jjson-container"></ul>');
        try {
            var json = $.parseJSON(self.data('jjson'));
            options = $.extend({}, this.defaults, options);
            var expanderClasses = getExpanderClasses(options.expanded);
            self.find('.jjson-container').append(json2html([json], expanderClasses));
        } catch(e) {
            self.prepend('<div class="jjson-error" >' + e.toString() + ' </div>');
            self.find('.jjson-container').append(self.data('jjson'));
        }
    }

    function getExpanderClasses(expanded) {
        if(!expanded) return 'expanded collapsed hidden';
        return 'expanded';
    }

    function json2html(json, expanderClasses) {
        var html = '';
        for(var key in json) {
            if (!json.hasOwnProperty(key)) {
                continue;
            }

            var value = json[key],
                type = typeof json[key];

            html = html + createElement(key, value, type, expanderClasses);
        }
        return html;
    }

    function encode(value) {
        return $('<div/>').text(value).html();
    }

    function createElement(key, value, type, expanderClasses) {
        var klass = 'object',
            open = '{',
            close = '}';
        if ($.isArray(value)) {
            klass = 'array';
              open = '[';
              close = ']';
        }
        if(value === null) {
            return '<li><span class="key">"' + encode(key) + '": </span><span class="null">null</span></li>';
        }
        if(type == 'object') {
            var object = '<li><span class="'+ expanderClasses +'"></span><span class="key">"' + encode(key) + '": </span> <span class="open">' + open + '</span> <ul class="' + klass + '">';
            object = object + json2html(value, expanderClasses);
            return object + '</ul><span class="close">' + close + '</span></li>';
        }
        if(type == 'number' || type == 'boolean') {
            return '<li><span class="key">"' + encode(key) + '": </span><span class="'+ type + '">' + encode(value) + '</span></li>';
        }
        return '<li><span class="key">"' + encode(key) + '": </span><span class="'+ type + '">"' + encode(value) + '"</span></li>';
    }

    $(document).on('click', '.jjson-container .expanded', function(event) {
        event.preventDefault();
        event.stopPropagation();
        var $self = $(this);
        $self.parent().find('>ul').slideUp(100, function() {
            $self.addClass('collapsed');
        });
    });

    $(document).on('click', '.jjson-container .expanded.collapsed', function(event) {
        event.preventDefault();
        event.stopPropagation();
        var $self = $(this);
        $self.removeClass('collapsed').parent().find('>ul').slideDown(100, function() {
            $self.removeClass('collapsed').removeClass('hidden');
        });
    });

    JJsonViewer.prototype.defaults = {
        expanded: true
    };

}(window.jQuery);
