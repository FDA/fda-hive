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


$.extend( true, DataTable.defaults, {
    dom:
        "<'mdl-grid'"+
            "<'mdl-cell mdl-cell--6-col'l>"+
            "<'mdl-cell mdl-cell--6-col'f>"+
        ">"+
        "<'mdl-grid dt-table'"+
            "<'mdl-cell mdl-cell--12-col'tr>"+
        ">"+
        "<'mdl-grid'"+
            "<'mdl-cell mdl-cell--4-col'i>"+
            "<'mdl-cell mdl-cell--8-col'p>"+
        ">",
    renderer: 'material'
} );


$.extend( DataTable.ext.classes, {
    sWrapper:      "dataTables_wrapper form-inline dt-material",
    sFilterInput:  "form-control input-sm",
    sLengthSelect: "form-control input-sm",
    sProcessing:   "dataTables_processing panel panel-default"
} );


DataTable.ext.renderer.pageButton.material = function ( settings, host, idx, buttons, page, pages ) {
    var api     = new DataTable.Api( settings );
    var classes = settings.oClasses;
    var lang    = settings.oLanguage.oPaginate;
    var aria = settings.oLanguage.oAria.paginate || {};
    var btnDisplay, btnClass, counter=0;

    var attach = function( container, buttons ) {
        var i, ien, node, button, disabled, active;
        var clickHandler = function ( e ) {
            e.preventDefault();
            if ( !$(e.currentTarget).hasClass('disabled') && api.page() != e.data.action ) {
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
                active = false;

                switch ( button ) {
                    case 'ellipsis':
                        btnDisplay = '&#x2026;';
                        btnClass = 'disabled';
                        break;

                    case 'first':
                        btnDisplay = lang.sFirst;
                        btnClass = button + (page > 0 ?
                            '' : ' disabled');
                        break;

                    case 'previous':
                        btnDisplay = lang.sPrevious;
                        btnClass = button + (page > 0 ?
                            '' : ' disabled');
                        break;

                    case 'next':
                        btnDisplay = lang.sNext;
                        btnClass = button + (page < pages-1 ?
                            '' : ' disabled');
                        break;

                    case 'last':
                        btnDisplay = lang.sLast;
                        btnClass = button + (page < pages-1 ?
                            '' : ' disabled');
                        break;

                    default:
                        btnDisplay = button + 1;
                        btnClass = '';
                        active = page === button;
                        break;
                }

                if ( active ) {
                    btnClass += ' mdl-button--raised mdl-button--colored';
                }

                if ( btnDisplay ) {
                    node = $('<button>', {
                            'class': 'mdl-button '+btnClass,
                            'id': idx === 0 && typeof button === 'string' ?
                                settings.sTableId +'_'+ button :
                                null,
                            'aria-controls': settings.sTableId,
                            'aria-label': aria[ button ],
                            'data-dt-idx': counter,
                            'tabindex': settings.iTabIndex,
                            'disabled': btnClass.indexOf('disabled') !== -1
                        } )
                        .html( btnDisplay )
                        .appendTo( container );

                    settings.oApi._fnBindAction(
                        node, {action: button}, clickHandler
                    );

                    counter++;
                }
            }
        }
    };

    var activeEl;

    try {
        activeEl = $(host).find(document.activeElement).data('dt-idx');
    }
    catch (e) {}

    attach(
        $(host).empty().html('<div class="pagination"/>').children(),
        buttons
    );

    if ( activeEl ) {
        $(host).find( '[data-dt-idx='+activeEl+']' ).focus();
    }
};


return DataTable;
}));