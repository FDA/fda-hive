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


var sort_prefix = 'css_right ui-icon ui-icon-';
var toolbar_prefix = 'fg-toolbar ui-toolbar ui-widget-header ui-helper-clearfix ui-corner-';

$.extend( true, DataTable.defaults, {
    dom:
        '<"'+toolbar_prefix+'tl ui-corner-tr"lfr>'+
        't'+
        '<"'+toolbar_prefix+'bl ui-corner-br"ip>',
    renderer: 'jqueryui'
} );


$.extend( DataTable.ext.classes, {
    "sWrapper":            "dataTables_wrapper dt-jqueryui",

    "sPageButton":         "fg-button ui-button ui-state-default",
    "sPageButtonActive":   "ui-state-disabled",
    "sPageButtonDisabled": "ui-state-disabled",

    "sPaging": "dataTables_paginate fg-buttonset ui-buttonset fg-buttonset-multi "+
        "ui-buttonset-multi paging_",

    "sSortAsc":            "ui-state-default sorting_asc",
    "sSortDesc":           "ui-state-default sorting_desc",
    "sSortable":           "ui-state-default sorting",
    "sSortableAsc":        "ui-state-default sorting_asc_disabled",
    "sSortableDesc":       "ui-state-default sorting_desc_disabled",
    "sSortableNone":       "ui-state-default sorting_disabled",
    "sSortIcon":           "DataTables_sort_icon",

    "sScrollHead": "dataTables_scrollHead "+"ui-state-default",
    "sScrollFoot": "dataTables_scrollFoot "+"ui-state-default",

    "sHeaderTH":  "ui-state-default",
    "sFooterTH":  "ui-state-default"
} );


DataTable.ext.renderer.header.jqueryui = function ( settings, cell, column, classes ) {
    var noSortAppliedClass = sort_prefix+'carat-2-n-s';
    var asc = $.inArray('asc', column.asSorting) !== -1;
    var desc = $.inArray('desc', column.asSorting) !== -1;

    if ( !column.bSortable || (!asc && !desc) ) {
        noSortAppliedClass = '';
    }
    else if ( asc && !desc ) {
        noSortAppliedClass = sort_prefix+'carat-1-n';
    }
    else if ( !asc && desc ) {
        noSortAppliedClass = sort_prefix+'carat-1-s';
    }

    $('<div/>')
        .addClass( 'DataTables_sort_wrapper' )
        .append( cell.contents() )
        .append( $('<span/>')
            .addClass( classes.sSortIcon+' '+noSortAppliedClass )
        )
        .appendTo( cell );

    $(settings.nTable).on( 'order.dt', function ( e, ctx, sorting, columns ) {
        if ( settings !== ctx ) {
            return;
        }

        var colIdx = column.idx;

        cell
            .removeClass( classes.sSortAsc +" "+classes.sSortDesc )
            .addClass( columns[ colIdx ] == 'asc' ?
                classes.sSortAsc : columns[ colIdx ] == 'desc' ?
                    classes.sSortDesc :
                    column.sSortingClass
            );

        cell
            .find( 'span.'+classes.sSortIcon )
            .removeClass(
                sort_prefix+'triangle-1-n' +" "+
                sort_prefix+'triangle-1-s' +" "+
                sort_prefix+'carat-2-n-s' +" "+
                sort_prefix+'carat-1-n' +" "+
                sort_prefix+'carat-1-s'
            )
            .addClass( columns[ colIdx ] == 'asc' ?
                sort_prefix+'triangle-1-n' : columns[ colIdx ] == 'desc' ?
                    sort_prefix+'triangle-1-s' :
                    noSortAppliedClass
            );
    } );
};


if ( DataTable.TableTools ) {
    $.extend( true, DataTable.TableTools.classes, {
        "container": "DTTT_container ui-buttonset ui-buttonset-multi",
        "buttons": {
            "normal": "DTTT_button ui-button ui-state-default"
        },
        "collection": {
            "container": "DTTT_collection ui-buttonset ui-buttonset-multi"
        }
    } );
}


return DataTable;
}));
