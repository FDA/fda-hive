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
        class RankCollapser : public GraphCommand
        {
            private:
                idx collapseColumn, countColumn;
                idx rank;

            public:
                RankCollapser(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "rankCollapser"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdRankCollapserFactory(ExecContext & ctx) { return new RankCollapser(ctx); }
    };
};

bool RankCollapser::init(const char * op_name, sVariant * arg)
{
    if (sVariant * cols = arg->getDicElt("collapseColumn"))
    {
        if (cols->isList() && cols->dim() > 0)
            collapseColumn = cols->getListElt(0)->asInt();
        else
            return false;
    }

    if (sVariant * cols = arg->getDicElt("countColumn"))
    {
        if (cols->isList() && cols->dim() > 0)
            countColumn = cols->getListElt(0)->asInt();
        else
            return false;
    }

    if (sVariant * val = arg->getDicElt("rank")){
        rank = val->asInt();
    }


    return true;
}

bool RankCollapser::compute(sTabular * tbl)
{

    sVariantTbl * outTabular = new sVariantTbl (2, 0);



    sDic <idx> taxIdRankDic;

    for (idx r = 0; r < tbl->rows(); r++){
        sStr curCellPath;
        tbl->printCell(curCellPath, r, collapseColumn);

        sVariant matchCntCell;
        tbl->val(matchCntCell, r, countColumn);

        idx * val = taxIdRankDic.set(curCellPath, curCellPath.length());
        *val += matchCntCell.asInt();
    }

    outTabular->setVal(-1,0, "Path");
    outTabular->setVal(-1,1, "Count");

    for (idx i = 0; i < taxIdRankDic.dim(); i++){
        const char * key = static_cast <const char *> (taxIdRankDic.id(i));
        idx val = taxIdRankDic[key];

        outTabular->setVal(i, 0, key);
        outTabular->setVal(i, 1, val);
    }
    setOutTable(outTabular);

    return true;
}
