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
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <sra/sradb-priv.h>

#include "reader.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>

struct TRunReader
{
    const VCursor* curs;
    const TRunReaderColumn* cols;
    int64_t col_qty;
    int64_t curr;
};

static
rc_t CC TRunReader_MakeCursor( const TRunReader** cself, const VCursor* cursor,
                               TRunReaderColumn* cols )
{
    rc_t rc = 0;
    TRunReader* obj = NULL;

    if ( cself == NULL || cursor == NULL || cols == NULL || cols->name == NULL ) {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcInvalid );
    } else if ( ( obj = calloc( 1, sizeof( *obj ) ) ) == NULL ) {
        rc = RC( rcAlign, rcType, rcConstructing, rcMemory, rcExhausted );
    } else {
        rc = VCursorAddRef( cursor );
        if ( rc == 0 ) {
            obj->curs = cursor;
            obj->cols = cols;
            while ( rc == 0 && cols->name != NULL) {
                if ( !( cols->flags & ercol_Skip ) ) {
                    rc = VCursorAddColumn( obj->curs, &cols->idx, cols->name );
                    if ( rc != 0 ) {
                        if ( ( rc != 0 && ( cols->flags & ercol_Optional ) ) || GetRCState( rc ) == rcExists ) {
                            rc = 0;
                        }
                    }
                }
                cols++;
                obj->col_qty++;
            }
            if( rc == 0 ) {
                rc = VCursorOpen( obj->curs );
            }
        }
    }
    if( rc == 0 ) {
        *cself = obj;
    } else {
        TRunReader_Whack( obj );
    }
    return rc;
}

rc_t CC TRunReader_Make( const TRunReader** cself, const SRATable* table,
                         TRunReaderColumn* cols, int64_t cache )
{
    rc_t rc = 0;
    const VTable* vtbl = NULL;
    const VCursor* curs;

    if ( table == NULL ) {
        rc = RC( rcAlign, rcType, rcConstructing, rcParam, rcInvalid );
    } else if( (rc = SRATableGetVTableRead(table, &vtbl)) == 0 ) {
        if( (rc = VTableCreateCachedCursorRead(vtbl, &curs, cache)) == 0 ) {
            rc = VCursorPermitPostOpenAdd(curs);
        }
        VTableRelease(vtbl);
        if ( rc == 0 ) {
            rc = TRunReader_MakeCursor( cself, curs, cols );
            VCursorRelease( curs );
        }
    }
    return rc;
}

void CC TRunReader_Whack( const TRunReader* cself )
{
    if ( cself != NULL ) {
        VCursorRelease( cself->curs );
        free( ( TRunReader* ) cself );
    }
}

rc_t CC TRunReader_IdRange( const TRunReader* cself, int64_t* first, uint64_t* count )
{
    rc_t rc = 0;
    if ( cself == NULL ) {
        rc = RC( rcAlign, rcType, rcRetrieving, rcSelf, rcNull );
    } else {
        rc = VCursorIdRange( cself->curs, 0, first, count );
    }
    return rc;
}

rc_t CC TRunReader_ReadCell(const TRunReader* cself, int64_t rowid, int64_t colid)
{
    rc_t rc = 0;
    if( cself == NULL || colid <= 0 || colid > cself->col_qty ) {
        rc = RC(rcAlign, rcType, rcOpening, rcParam, rcInvalid);
    } else {
        TRunReader* self = (TRunReader*)cself;
        if( cself->curr != rowid ) {
            if( (rc = VCursorCloseRow( cself->curs )) == 0 &&
                (rc = VCursorSetRowId( cself->curs, rowid )) == 0 ) {
                rc = VCursorOpenRow( cself->curs );
                self->curr = rowid;
            }
        }
        if( rc == 0 ) {
            TRunReaderColumn* c = ( TRunReaderColumn* )( &cself->cols[colid - 1] );
            if( c->idx == 0 ) {
                /* add column on fly */
                if( (rc = VCursorAddColumn(self->curs, &c->idx, c->name)) != 0 ) {
                    if( (c->flags & ercol_Optional) || GetRCState(rc) == rcExists ) {
                        rc = 0;
                    }
                }
            }
            if( rc == 0 && c->idx != 0 ) {
                rc = VCursorCellData(cself->curs, c->idx, &c->elem_bits, (const void**)&c->base.var, &c->boff, &c->len);
                if( rc != 0 ) {
                    if( c->flags & ercol_Optional ) {
                        rc = 0;
                    }
                    c->base.var = NULL;
                    c->len = 0;
                    c->boff = 0;
                    c->elem_bits = 0;
                }
            }
        }
    }
    return rc;
}

