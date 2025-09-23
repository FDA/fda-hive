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
#include <slib/core/def.hpp>
#include <xlib/xls2csv/Worksheet.h>
#include <xlib/xls2csv/Xls.h>

#include <stdio.h>
#include <string.h>

extern "C" {
#include <libxls/xls.h>
}

using namespace std;

namespace Xls {

    Worksheet::Worksheet(xlsWorkSheet *pWS, const char *name)
        : m_pWS(pWS), m_pName(name ? strdup(name) : NULL)
    {
    }

    Worksheet::~Worksheet()
    {
        free(m_pName);
    }

    unsigned int Worksheet::CountRows() const
    {
        return m_pWS->rows.lastrow;
    }

    unsigned int Worksheet::CountColumns() const
    {
        return m_pWS->rows.lastcol;
    }

    const char * Worksheet::CellValue(unsigned int i, unsigned int j) const
    {
        static char buf[64];

        if ( i < CountRows() || j < CountColumns() ) {
            const st_row::st_row_data &row = m_pWS->rows.row[i];
            const st_cell::st_cell_data &cell = row.cells.cell[j];

            if ( cell.id == 0x201 )
                return "";
            else if ( cell.id == 0xfd || cell.id == 0xbd || cell.id == 0x27e ) {
                return cell.str;
            }
            else if ( cell.id == 0x203 ) {
                sprintf(buf, "%g", cell.d);
                return buf;
            }
            else if ( cell.str != NULL ) {
                return cell.str;
            }
        }

        return NULL;
    }

    bool Worksheet::ExportCsv(const char *file) const
    {
        static char _buf[32768];

        FILE *pf = fopen(file, "w");

        for ( size_t i=0; i<CountRows(); i++ ) {
            for ( size_t j=0; j<CountColumns(); j++ ) {
                if ( j > 0 )
                    fprintf(pf, ",");
                fprintf(pf, "%s", csvEncode(_buf, CellValue(i, j), sizeof(_buf)));
            }
            fprintf(pf, "\n");
        }

        fclose(pf);

        return true;
    }
}
