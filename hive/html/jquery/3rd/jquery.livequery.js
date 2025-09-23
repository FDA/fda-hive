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
(function (factory) {
    if (typeof define === 'function' && define.amd) {
        define(['jquery'], factory);
    } else if (typeof exports === 'object') {
        factory(require('jquery'));
    } else {
        factory(jQuery);
    }
}(function ($, undefined) {

function _match(me, query, fn, fn2) {
    return me.selector == query.selector &&
        me.context == query.context &&
        (!fn || fn.$lqguid == query.fn.$lqguid) &&
        (!fn2 || fn2.$lqguid == query.fn2.$lqguid);
}

$.extend($.fn, {
    livequery: function(fn, fn2) {
        var me = this, q;

        $.each( $jQlq.queries, function(i, query) {
            if ( _match(me, query, fn, fn2) )
                    return (q = query) && false;
        });

        q = q || new $jQlq(me.selector, me.context, fn, fn2);

        q.stopped = false;

        q.run();

        return me;
    },

    expire: function(fn, fn2) {
        var me = this;

        $.each( $jQlq.queries, function(i, query) {
            if ( _match(me, query, fn, fn2) && !me.stopped)
                    $jQlq.stop(query.id);
        });

        return me;
    }
});

var $jQlq = $.livequery = function(selector, context, fn, fn2) {
    var me = this;

    me.selector = selector;
    me.context  = context;
    me.fn       = fn;
    me.fn2      = fn2;
    me.elements = $([]);
    me.stopped  = false;

    me.id = $jQlq.queries.push(me)-1;

    fn.$lqguid = fn.$lqguid || $jQlq.guid++;
    if (fn2) fn2.$lqguid = fn2.$lqguid || $jQlq.guid++;

    return me;
};

$jQlq.prototype = {
    stop: function() {
        var me = this;
        if ( me.stopped ) return;

        if (me.fn2)
            me.elements.each(me.fn2);

        me.elements = $([]);

        me.stopped = true;
    },

    run: function() {
        var me = this;
        if ( me.stopped ) return;

        var oEls = me.elements,
            els  = $(me.selector, me.context),
            newEls = els.not(oEls),
            delEls = oEls.not(els);

        me.elements = els;

        newEls.each(me.fn);

        if ( me.fn2 )
            delEls.each(me.fn2);
    }
};

$.extend($jQlq, {
    guid: 0,
    queries: [],
    queue: [],
    running: false,
    timeout: null,
    registered: [],

    checkQueue: function() {
        if ( $jQlq.running && $jQlq.queue.length ) {
            var length = $jQlq.queue.length;
            while ( length-- )
                $jQlq.queries[ $jQlq.queue.shift() ].run();
        }
    },

    pause: function() {
        $jQlq.running = false;
    },

    play: function() {
        $jQlq.running = true;
        $jQlq.run();
    },

    registerPlugin: function() {
        $.each( arguments, function(i,n) {
            if (!$.fn[n] || $.inArray(n, $jQlq.registered) > 0) return;

            var old = $.fn[n];

            $.fn[n] = function() {
                var r = old.apply(this, arguments);

                $jQlq.run();

                return r;
            };

            $jQlq.registered.push(n);
        });
    },

    run: function(id) {
        if (id !== undefined) {
            if ( $.inArray(id, $jQlq.queue) < 0 )
                $jQlq.queue.push( id );
        }
        else
            $.each( $jQlq.queries, function(id) {
                if ( $.inArray(id, $jQlq.queue) < 0 )
                    $jQlq.queue.push( id );
            });

        if ($jQlq.timeout) clearTimeout($jQlq.timeout);
        $jQlq.timeout = setTimeout($jQlq.checkQueue, 20);
    },

    stop: function(id) {
        if (id !== undefined)
            $jQlq.queries[ id ].stop();
        else
            $.each( $jQlq.queries, $jQlq.prototype.stop);
    }
});

$jQlq.registerPlugin('append', 'prepend', 'after', 'before', 'wrap', 'attr', 'removeAttr', 'addClass', 'removeClass', 'toggleClass', 'empty', 'remove', 'html', 'prop', 'removeProp');

$(function() { $jQlq.play(); });

}));