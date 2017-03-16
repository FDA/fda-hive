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
#include <ssci/math.hpp>
#include "tblqryX4_cmd.hpp"

#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class CleanerCommand : public Command
        {
            private:
                sVec <idx> colSet, rowSet;
                bool _wraps_in_table;

            public:
                CleanerCommand(ExecContext & ctx) : Command(ctx)
                {
                    _wraps_in_table = false;
                }

                const char * getName() { return "cleaner"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return false; }
                bool wrapsInTable() { return _wraps_in_table; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdCleanerFactory(ExecContext & ctx) { return new CleanerCommand(ctx); }
    };
};

bool CleanerCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetVal = arg->getDicElt("rowSet"))
    {
        const char * p=colSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }

    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        const char * p=colSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }
    else {
        _ctx.logError("Missing colSet argument for cleaner operation");
        return false;
    }

    return true;
}

static bool contains (sVec <idx> vector, idx toCheck)
{
    for (idx i = 0; i < vector.dim(); i ++)
    {
        if (vector[i] == toCheck)
            return true;
    }

    return false;
}

bool CleanerCommand::compute(sTabular * tbl)
{
    if (!rowSet || rowSet.dim() < 1)
    {
        char p [16];
        sprintf (p, "0-%" DEC, tbl->rows()-1);
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }

    sDic < sDic < sVec < idx > > > rowsToUseDicDic;
    sText::categoryListParseCsv(tbl , &(rowSet), &rowsToUseDicDic , 0, &(colSet));

    // run the same algorihtm for every categorization schema
    if (colSet && colSet.dim() > 0)
    {
        //sVariantTbl * tblToReturn;
        sTxtTbl * anotherTbl = new sTxtTbl();
        const char * someVal = static_cast <const char *> (rowsToUseDicDic.id(0));
        sDic < sVec < idx > > & rowsToUseDic = rowsToUseDicDic[someVal];

        sMatrix matrixForCurCategory; //we use this matrix after the normalized


        sVec <idx> columnsForCurCategory;

        for (idx ii = 0; ii < tbl->cols(); ii++)
        {
            if (colSet[0] == ii)
            {
                columnsForCurCategory.vadd(1,ii);
                continue;
            }

            //all of the standard deviations for the different values in the category column
            sVec <real> stdDevForCurCategory; stdDevForCurCategory.add(rowsToUseDic.dim());

            for (idx x = 0; x < rowsToUseDic.dim(); x++)
            {
                const char * catVal = static_cast <const char *> (rowsToUseDic.id(x));
                sVec <idx> curRowVec = rowsToUseDic[catVal];
                real mean = 0;

                for (idx y = 0; y < curRowVec.dim(); y++)
                    mean += tbl->rval(curRowVec[y], ii);
                mean /= curRowVec.dim();
                stdDevForCurCategory[x] = 0;
                for (idx y = 0; y < curRowVec.dim(); y++)
                {
                    real val = tbl->rval(curRowVec[y], ii);
                    stdDevForCurCategory[x] += (val - mean) * (val - mean);
                }
                stdDevForCurCategory[x] /= curRowVec.dim();
                stdDevForCurCategory[x] = sqrt (stdDevForCurCategory[x]);
            }

            bool adding = true;
            for (idx x = 0; x < rowsToUseDic.dim(); x++)
            {
                if (stdDevForCurCategory[x] == 0)
                {
                    adding = false;
                    break;
                }
            }

            if (adding)
            {
                columnsForCurCategory.vadd(1,ii);
            }
            if (ii%1000 == 0)
                _ctx.logTrace("Current column is %" DEC "\n", ii);
        }

        //tblToReturn = new sVariantTbl (columnsForCurCategory.dim());
        anotherTbl->initWritable(columnsForCurCategory.dim(),sTblIndex::fTopHeader,",");

        for (idx rr = -1; rr < rowSet.dim(); rr++)
        {
            idx curRow = rr < 0 ? -1 : rowSet[rr];

            for (idx cc = 0; cc < columnsForCurCategory.dim(); cc++)
            {
                idx curCol = columnsForCurCategory[cc];
                sVariant tmp;
                tbl->val(tmp, curRow, curCol, true);
                anotherTbl->addCell(tmp);
            }
            anotherTbl->addEndRow();
        }

        _wraps_in_table = false;
        setOutTable(anotherTbl);
    } else {
        _wraps_in_table = true;
        setOutTable(tbl);
    }


    return true;
}

