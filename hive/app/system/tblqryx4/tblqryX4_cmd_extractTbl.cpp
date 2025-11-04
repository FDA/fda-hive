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

#include <slib/utils.hpp>
#include "tblqryX4_cmd.hpp"
#include "utils.hpp"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class ExtractCommand : public Command
        {
            private:
                sVec <idx> colSet;
                sVec <idx> rowSet;

            public:
                ExtractCommand(ExecContext & ctx) : Command(ctx)
                {
                }

                const char * getName() { return "extract"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdExtractFactory(ExecContext & ctx) { return new ExtractCommand(ctx); }
    };
};

bool ExtractCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
    {
        const char * p=rowSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }

    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        const char * p=colSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }

    return true;
}

bool ExtractCommand::compute(sTabular * tbl)
{
    if (!colSet || colSet.dim() == 0)
    {
        char p [16];
        sprintf (p, "0-%" DEC, tbl->cols());
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }

    if (!rowSet || rowSet.dim() == 0)
    {
        char p [16];
        sprintf (p, "0-%" DEC, tbl->rows());
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }

    if (rowSet.dim() == tbl->rows() && colSet.dim() == tbl->cols())
        return tbl;

    sTxtTbl * toReturn = new sTxtTbl();

    toReturn->initWritable(colSet.dim(),sTblIndex::fTopHeader,",");

    for (idx rr = -1; rr < rowSet.dim(); rr++)
    {
        idx curRow = rr < 0 ? -1 : rowSet[rr];

        for (idx cc = 0; cc < colSet.dim(); cc++)
        {
            idx curCol = colSet[cc];
            sVariant tmp;
            tbl->val(tmp, curRow, curCol, true);
            toReturn->addCell(tmp);
        }
        toReturn->addEndRow();
    }

    setOutTable(toReturn);
    return true;
}
