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
#ifndef _h_hive_reader_
#define _h_hive_reader_

#include <insdc/insdc.h>
#include <insdc/sra.h>

enum TRunReaderColumn_Array {
    ercol_Optional = 0x01,
    ercol_Skip = 0x02
};

/* use ercol_Ignore to skip optional column when cursor is created */
typedef struct TRunReaderColumn_struct {
    uint32_t idx;
    const char* name;
    union {
        const void* var;
        const char* str;
        const bool* buul;
        const uint8_t* u8;
        const int16_t* i16;
        const uint16_t* u16;
        const int32_t* i32;
        const uint32_t* u32;
        const int64_t* i64;
        const uint64_t* u64;
        const INSDC_coord_one* coord1;
        const INSDC_coord_zero* coord0;
        const INSDC_coord_len* coord_len;
        const INSDC_coord_val* coord_val;
        const INSDC_SRA_xread_type* read_type;
        const INSDC_SRA_read_filter* read_filter;
        const INSDC_quality_phred* qual_phred;
    } base;
    uint32_t len;
    uint32_t flags;
    uint32_t boff;
    uint32_t elem_bits;
} TRunReaderColumn;

typedef struct TRunReader TRunReader;

rc_t CC TRunReader_Make(const TRunReader** cself, const SRATable* table, TRunReaderColumn* cols, int64_t cache);

void CC TRunReader_Whack(const TRunReader* cself);

rc_t CC TRunReader_IdRange(const TRunReader* cself, int64_t* first, uint64_t* count);

rc_t CC TRunReader_ReadCell(const TRunReader* cself, int64_t rowid, int64_t colid);

#endif /* _h_hive_reader_ */
