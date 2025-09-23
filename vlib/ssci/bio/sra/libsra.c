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
#include "extern.h"
#include "libsra.vers.h"

#include <sysalloc.h>

#include <vdb/table.h>
#include <vdb/cursor.h>
#include <sra/sradb.h>
#include <bitstr.h>
#include "reader.h"

#include <stdlib.h>
#include <string.h>

enum TRunReaderColumnIds {
    eColName,
    eColLabel,
    eColLabelStart = 0,
    eColLabelLen,
    eColSpotLen,
    eColReadStart = 0,
    eColReadLen,
    eColType,
    eColDna,
    eCol4na,
    eCol2na,
    eColQual
};

static const TRunReaderColumn TColumns[] =
{
    {0, "NAME", {NULL}, 0, ercol_Skip | ercol_Optional, 0, 0},
    {0, "LABEL", {NULL}, 0, ercol_Skip | ercol_Optional, 0, 0},
    {0, "LABEL_START", {NULL}, 0, ercol_Skip | ercol_Optional, 0, 0},
    {0, "LABEL_LEN", {NULL}, 0, ercol_Skip | ercol_Optional, 0, 0},
    {0, "(INSDC:coord:len)SPOT_LEN", {NULL}, 0, 0, 0, 0},
    {0, "(INSDC:coord:zero)READ_START", {NULL}, 0, 0, 0, 0},
    {0, "(INSDC:coord:len)READ_LEN", {NULL}, 0, 0, 0, 0},
    {0, "READ_TYPE", {NULL}, 0, 0, 0, 0},
    {0, "(INSDC:dna:text)READ", {NULL}, 0, ercol_Skip, 0, 0},
    {0, "(INSDC:4na:bin)READ", {NULL}, 0, ercol_Skip, 0, 0},
    {0, "(INSDC:2na:bin)READ", {NULL}, 0, ercol_Skip, 0, 0},
    {0, "(INSDC:quality:phred)QUALITY", {NULL}, 0, ercol_Skip | ercol_Optional, 0, 0},
    {0, NULL, {NULL}, 0, 0, 0, 0}
};

typedef struct
{
    int64_t row_min, row_max;
    const TRunReader* reader;
    TRunReaderColumn cols[sizeof(TColumns) / sizeof(TColumns[0])];
} Handle;
static uint64_t obj_qty = 0;
static enum TRunReaderColumnIds read_col = eCol2na;

static const SRAMgr* libSRA_GetSRAMgr(bool release)
{
    static const SRAMgr* mgr = NULL;
    if(mgr && release) {
        SRAMgrRelease(mgr);
        mgr = NULL;
    } else if(!mgr || SRAMgrMakeRead(&mgr) != 0) {
        mgr = NULL;
    }
    return mgr;
}


LIBSRA_EXTERN uint64_t CC libSRA_Version(void)
{
    return (uint64_t) LIBSRA_VERS;
}

LIBSRA_EXTERN void CC libSRA_SetReadAsDna(void)
{
    read_col = eColDna;
}

LIBSRA_EXTERN void CC libSRA_SetReadAs4na(void)
{
    read_col = eCol4na;
}

LIBSRA_EXTERN void CC libSRA_SetReadAs2na(void)
{
    read_col = eCol2na;
}

LIBSRA_EXTERN const void* CC libSRA_SRAMgrOpenTableRead(const char* path, uint64_t cache_size)
{
    Handle* h = NULL;
    if(path != NULL) {
        h = calloc(1, sizeof(*h));
        memcpy(h->cols, TColumns, sizeof(TColumns));
        if(h != NULL) {
            const SRATable* tbl = NULL;
            if(SRAMgrOpenTableRead(libSRA_GetSRAMgr(false), &tbl, "%s", path) != 0 || TRunReader_Make(&h->reader, tbl, h->cols, cache_size) != 0
                    || TRunReader_IdRange(h->reader, &h->row_min, (uint64_t*) &h->row_max) != 0) {
                SRATableRelease(tbl);
                free(h);
                h = NULL;
            } else {
                SRATableRelease(tbl);
                h->row_min--;
                h->row_max += h->row_min;
                obj_qty++;
            }
        }
    }
    return h;
}

LIBSRA_EXTERN void CC libSRA_SRATableRelease(const void* handle)
{
    Handle* h = (Handle*) handle;
    if(h) {
        TRunReader_Whack(h->reader);
        free(h);
        if(--obj_qty == 0) {
            libSRA_GetSRAMgr(true);
        }
    }
}

LIBSRA_EXTERN uint64_t CC libSRA_SRATableRows(const void* handle)
{
    const Handle* h = handle;
    return h ? h->row_max - h->row_min : 0;
}

LIBSRA_EXTERN uint64_t CC libSRA_GetReadNumber(const void* handle, uint64_t row, uint64_t read_types)
{
    Handle* h = (Handle*) handle;
    uint64_t num = 0;
    if(h) {
        if(TRunReader_ReadCell(h->reader, h->row_min + row, eColType) == 0) {
            if(read_types == 0 || read_types == 2) {
                uint32_t i;
                INSDC_SRA_xread_type mask = read_types == 0 ? READ_TYPE_BIOLOGICAL : READ_TYPE_TECHNICAL;
                for(i = 0; i < h->cols[eColType].len; i++) {
                    if(h->cols[eColType].base.read_type[i] & mask) {
                        num++;
                    }
                }
            } else {
                num = h->cols[eColType].len;
            }
        }
    }
    return num;
}

LIBSRA_EXTERN uint64_t CC libSRA_GetSeqLen(const void* handle, uint64_t row, int64_t read_num)
{
    Handle* h = (Handle*) handle;
    uint64_t len = 0;
    if(h) {
        enum TRunReaderColumnIds c = eColReadLen;
        if(read_num <= 0) {
            c = eColSpotLen;
            read_num = 0;
        } else {
            read_num--;
        }
        if(TRunReader_ReadCell(h->reader, h->row_min + row, c) == 0 && read_num < h->cols[c].len) {
            len = h->cols[c].base.coord_len[read_num];
        }
    }
    return len;
}

LIBSRA_EXTERN int64_t CC libSRA_GetSeqName(const void* handle, uint64_t row, int64_t read_num, void* buf, int64_t buf_sz)
{
    rc_t rc = 0;
    int64_t bytes = 0;
    Handle* h = (Handle*) handle;
    if(h) {
        row += h->row_min;
        if( (rc = TRunReader_ReadCell(h->reader, row, eColName)) == 0 ) {
            if( read_num > 0 ) {
                if( (rc = TRunReader_ReadCell(h->reader, row, eColLabel)) == 0 &&
                    (rc = TRunReader_ReadCell(h->reader, row, eColLabelStart)) == 0 ) {
                    rc = TRunReader_ReadCell(h->reader, row, eColLabelLen);
                }
            }
            if( rc == 0 ) {
                INSDC_coord_len lbl_len = 0, nm_len = h->cols[eColName].len;
                bytes = nm_len;
                if( read_num > 0 && read_num < h->cols[eColLabelStart].len && read_num < h->cols[eColLabelLen].len) {
                    --read_num;
                    lbl_len = h->cols[eColLabelLen].base.coord_len[read_num];
                    if( nm_len > 0 && lbl_len > 0 ) {
                        bytes++;
                    }
                    bytes += lbl_len;
                }
                if( bytes <= buf_sz && bytes > 0 ) {
                    if( nm_len > 0 ) {
                        memcpy(buf, h->cols[eColName].base.str, nm_len);
                        if( lbl_len > 0 ) {
                            ((char*)buf)[nm_len++] = '_';
                        }
                    }
                    if( lbl_len > 0 ) {
                        read_num--;
                        INSDC_coord_zero start = h->cols[eColLabelStart].base.coord0[read_num];
                        memcpy(&((char*)buf)[nm_len], &(h->cols[eColLabel].base.str[start]), lbl_len);
                    }
                }
            }
        }
    }
    return rc == 0 ? bytes : 0;
}

LIBSRA_EXTERN int64_t CC libSRA_GetSeq(const void* handle, uint64_t row, int64_t read_num, void* buf, int64_t buf_sz)
{
    rc_t rc = 0;
    int64_t bytes = 0;
    Handle* h = (Handle*) handle;
    if(h) {
        row += h->row_min;
        if(( rc = TRunReader_ReadCell(h->reader, row, read_col)) == 0) {
            if( read_num <= 0 ) {
                bitsz_t bits = h->cols[read_col].elem_bits * h->cols[read_col].len;
                bytes = (bits - 1) / 8 + 1;
                if( bytes <= buf_sz ) {
                    bitcpy(buf, 0, h->cols[read_col].base.var, h->cols[read_col].boff, bits);
                }
            } else {
                read_num--;
                if( (rc = TRunReader_ReadCell(h->reader, row, eColReadLen)) == 0 ) {
                    INSDC_coord_len len = h->cols[eColReadLen].base.coord_len[read_num];
                    len *= h->cols[read_col].elem_bits;
                    bytes = (len - 1) / 8 + 1;
                    if( bytes <= buf_sz ) {
                        if( (rc = TRunReader_ReadCell(h->reader, row, eColReadStart)) == 0 ) {
                            INSDC_coord_zero start = h->cols[eColReadStart].base.coord0[read_num];
                            start *= h->cols[read_col].elem_bits;
                            bitcpy(buf, 0, h->cols[read_col].base.var, h->cols[read_col].boff + start, len);
                        }
                    }
                }
            }
        }
    }
    return rc == 0 ? bytes : 0;
}

LIBSRA_EXTERN int64_t CC libSRA_GetQual(const void* handle, uint64_t row, int64_t read_num, void* buf, int64_t buf_sz)
{
    rc_t rc = 0;
    int64_t bytes = 0;
    Handle* h = (Handle*) handle;
    if(h) {
        row += h->row_min;
        if(( rc = TRunReader_ReadCell(h->reader, row, eColQual)) == 0) {
            if( read_num <= 0 ) {
                bytes = h->cols[eColQual].len;
                if( bytes <= buf_sz ) {
                    memcpy(buf, h->cols[eColQual].base.qual_phred, bytes);
                }
            } else {
                read_num--;
                if( (rc = TRunReader_ReadCell(h->reader, row, eColReadLen)) == 0 ) {
                    bytes = h->cols[eColReadLen].base.coord_len[read_num];
                    if( bytes <= buf_sz ) {
                        if( (rc = TRunReader_ReadCell(h->reader, row, eColReadStart)) == 0 ) {
                            INSDC_coord_zero start = h->cols[eColReadStart].base.coord0[read_num];
                            memcpy(buf, &h->cols[eColQual].base.qual_phred[start], bytes);
                        }
                    }
                }
            }
        }
    }
    return rc == 0 ? bytes : 0;
}
