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
#include <slib/utils/basicCollapse.hpp>

#define PRFX "collapse-"
#define OUTFILE "collapse.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class CollapseWithStatCommand : public GraphCommand
        {
            public:
                CollapseWithStatCommand(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "collapsewithstat"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdCollapseWithStatFactory(ExecContext & ctx) { return new CollapseWithStatCommand(ctx); }
    };
};

bool CollapseWithStatCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        if (colSetVal->isList())
        {
            for (idx i = 0; i < colSetVal->dim(); i++)
                colSetImg.vadd(1,colSetVal->getListElt(i)->asInt());
        }
        else
            return false;
    }

    Parser parser;
    sorter.init(arg, &_ctx, getName(), true);

    return true;
}

bool CollapseWithStatCommand::compute(sTabular * tbl)
{
    sReorderedTabular reordered (tbl, false);

    if (!colSetImg || colSetImg.dim() == 0)
        return tbl;

    sVariantTbl * outTabular = new sVariantTbl (colSetImg.dim() + colSetImg.dim()*(tbl->cols()-colSetImg.dim()), 0);

    sorter.sort(&reordered);

    sBasicCollapse::collapse(&reordered, colSetImg, outTabular);

    //write into the correct table
    for (idx c = 0; c < colSetImg.dim(); c++)
    {
        sStr out;
        tbl->printTopHeader(out,colSetImg[c]);
        outTabular->setVal(-1,c,out.ptr());
        for (idx r = 0; r < tbl->rows(); r++)
        {
            out.cut(0);
            reordered.printCell(out,r,colSetImg[c]);
#if _DEBUG
            fprintf(stderr, "(%"DEC",%"DEC") -> %s\n", r, colSetImg[c], out.ptr(0));
#endif
            outTabular->setVal(r,c,out.ptr());
        }
    }

    sVec <idx> colsActual; //the are the columns on which the statistics will be accumulated

    for (idx i = 0; i < tbl->cols(); i++)
    {
        idx contains = -1;

        for (idx k = 0; k < colSetImg.dim(); k++)
        {
            if (colSetImg[k] == i)
            {
                contains = k;
                break;
            }
        }

        if (contains > -1)
            continue;

        colsActual.add(1);
        colsActual[colsActual.dim()-1] = i;
    }

    for (idx c = 0; c < colsActual.dim(); c++)
    {
        for (idx cc = 0; cc < colSetImg.dim(); cc++)
        {
            sStr actualStr;
            tbl->printTopHeader(actualStr,colsActual[c]);
            sStr addStr;
            tbl->printTopHeader(addStr,colSetImg[cc]);
            sStr nHeader;
            nHeader.printf("%s/%s", actualStr.ptr(), addStr.ptr());
            outTabular->setVal(-1,colSetImg.dim() + c*colSetImg.dim() + cc + 1,nHeader.ptr());
        }
    }

    return outTabular;
}
