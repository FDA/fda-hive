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

(function( factory ){
    if ( typeof define === 'function' && define.amd ) {
        define( ['jquery', 'datatables.net'], function ( $ ) {
            return factory( $, window, document );
        } );
    }
    else if ( typeof exports === 'object' ) {
        module.exports = function (root, $) {
            if ( ! root ) {
                root = window;
            }

            if ( ! $ || ! $.fn.dataTable ) {
                $ = require('datatables.net')(root, $).$;
            }

            return factory( $, root, root.document );
        };
    }
    else {
        factory( jQuery, window, document );
    }
}(function( $, window, document, undefined ) {
'use strict';
var DataTable = $.fn.dataTable;

var meta = $('<meta class="foundation-mq"/>').appendTo('head');
DataTable.ext.foundationVersion = meta.css('font-family').match(/small|medium|large/) ? 6 : 5;
meta.remove();


$.extend( DataTable.ext.classes, {
    sWrapper:    "dataTables_wrapper dt-foundation",
    sProcessing: "dataTables_processing panel"
} );


$.extend( true, DataTable.defaults, {
    dom:
        "<'row'<'small-6 columns'l><'small-6 columns'f>r>"+
        "t"+
        "<'row'<'small-6 columns'i><'small-6 columns'p>>",
    renderer: 'foundation'
} );


DataTable.ext.renderer.pageButton.foundation = function ( settings, host, idx, buttons, page, pages ) {
    var api = new DataTable.Api( settings );
    var classes = settings.oClasses;
    var lang = settings.oLanguage.oPaginate;
    var aria = settings.oLanguage.oAria.paginate || {};
    var btnDisplay, btnClass;
    var tag;
    var v5 = DataTable.ext.foundationVersion === 5;

    var attach = function( container, buttons ) {
        var i, ien, node, button;
        var clickHandler = function ( e ) {
            e.preventDefault();
            if ( !$(e.currentTarget).hasClass('unavailable') && api.page() != e.data.action ) {
                api.page( e.data.action ).draw( 'page' );
            }
        };

        for ( i=0, ien=buttons.length ; i<ien ; i++ ) {
            button = buttons[i];

            if ( $.isArray( button ) ) {
                attach( container, button );
            }
            else {
                btnDisplay = '';
                btnClass = '';
                tag = null;

                switch ( button ) {
                    case 'ellipsis':
                        btnDisplay = '&#x2026;';
                        btnClass = 'unavailable disabled';
                        tag = null;
                        break;

                    case 'first':
                        btnDisplay = lang.sFirst;
                        btnClass = button + (page > 0 ?
                            '' : ' unavailable disabled');
                        tag = page > 0 ? 'a' : null;
                        break;

                    case 'previous':
                        btnDisplay = lang.sPrevious;
                        btnClass = button + (page > 0 ?
                            '' : ' unavailable disabled');
                        tag = page > 0 ? 'a' : null;
                        break;

                    case 'next':
                        btnDisplay = lang.sNext;
                        btnClass = button + (page < pages-1 ?
                            '' : ' unavailable disabled');
                        tag = page < pages-1 ? 'a' : null;
                        break;

                    case 'last':
                        btnDisplay = lang.sLast;
                        btnClass = button + (page < pages-1 ?
                            '' : ' unavailable disabled');
                        tag = page < pages-1 ? 'a' : null;
                        break;

                    default:
                        btnDisplay = button + 1;
                        btnClass = page === button ?
                            'current' : '';
                        tag = page === button ?
                            null : 'a';
                        break;
                }

                if ( v5 ) {
                    tag = 'a';
                }

                if ( btnDisplay ) {
                    node = $('<li>', {
                            'class': classes.sPageButton+' '+btnClass,
                            'aria-controls': settings.sTableId,
                            'aria-label': aria[ button ],
                            'tabindex': settings.iTabIndex,
                            'id': idx === 0 && typeof button === 'string' ?
                                settings.sTableId +'_'+ button :
                                null
                        } )
                        .append( tag ?
                            $('<'+tag+'/>', {'href': '#'} ).html( btnDisplay ) :
                            btnDisplay
                        )
                        .appendTo( container );

                    settings.oApi._fnBindAction(
                        node, {action: button}, clickHandler
                    );
                }
            }
        }
    };

    attach(
        $(host).empty().html('<ul class="pagination"/>').children('ul'),
        buttons
    );
};


return DataTable;
}));
