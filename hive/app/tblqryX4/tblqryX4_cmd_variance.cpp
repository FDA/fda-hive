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
        class VarianceCommand : public GraphCommand
        {
            private:
                sVec <idx> colSetA;
                sVec <idx> colSetB;
                sVec <idx> rowSet;

            public:
                VarianceCommand(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "variance"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdVarianceFactory(ExecContext & ctx) { return new VarianceCommand(ctx); }
    };
};

bool VarianceCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetAVal = arg->getDicElt("colSetA"))
    {
        const char * p=colSetAVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSetA),0ll,0ll,0ll);
    }
    if (sVariant * colSetBVal = arg->getDicElt("colSetB"))
    {
        const char * p=colSetBVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSetB),0ll,0ll,0ll);
    }
    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
    {
        const char * p=rowSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }

    return true;
}

bool VarianceCommand::compute(sTabular * tbl)
{
    if (!colSetA || colSetA.dim() == 0 || !colSetB || colSetB.dim() == 0)
        return false;

    if( !rowSet || rowSet.dim() == 0 ) {
        char p[24];
        ::sprintf(p, "0-%" DEC, tbl->rows() - 1);
        sString::scanRangeSet(p, 0, &(rowSet), 0ll, 0ll, 0ll);
    }

    sVec <real> meanA, meanB;
    sVec <real> diff;
    sVec <real> variance;
    real totalDiff = 0;

    sSort::sort(rowSet.dim(), rowSet.ptr());
    for (idx iR = 0; iR < rowSet.dim(); iR++){
        idx row = rowSet[iR];

        real meanSetA = 0;
        for(idx iC1 = 0; iC1 < colSetA.dim(); iC1++){
            idx col = colSetA[iC1];
            real val = tbl->rval(row, col);
            meanSetA += val;
        }
        meanA.add(1);
        meanA[meanA.dim()-1] = meanSetA/colSetA.dim();

        real meanSetB = 0;
        for(idx iC1 = 0; iC1 < colSetB.dim(); iC1++){
            idx col = colSetB[iC1];
            real val = tbl->rval(row, col);
            meanSetB += val;
        }
        meanB.add(1);
        meanB[meanB.dim()-1] = meanSetB/colSetB.dim();

        real v = meanSetA/colSetA.dim()-meanSetB/colSetB.dim();
        diff.add(1);
        diff[diff.dim()-1] = (v);
        if (v < 0) v*= -1;
        totalDiff += v;
    }
    totalDiff /= diff.dim();

    for (idx i = 0; i < diff.dim(); i++){
        variance.add(1);
        variance[i] = diff[i]/totalDiff;
    }
    sVariantTbl * outTabular = new sVariantTbl (tbl->cols()+2, 0);

    idx lastR = 0;
    for (idx r = -1; r < tbl->rows(); r++){
        for (idx c = 0; c < tbl->cols(); c++){
            if (r == -1){
                sStr tmp;
                tbl->printTopHeader(tmp, c);
                outTabular->setVal(r,c,tmp.ptr());
            }
            else{
                sStr tmp;
                tbl->printCell(tmp, r, c);
                outTabular->setVal(r,c,tmp.ptr());
            }
        }
        if (r == -1){
            sStr tmp;
            tmp.printf("Difference");
            outTabular->setVal(r,tbl->cols(),tmp.ptr());
            tmp.printf(0,"J5");
            outTabular->setVal(r,tbl->cols()+1,tmp.ptr());

            continue;
        }

        if (rowSet[lastR] == r){
            sStr tmp;
            tmp.printf("%f", diff[lastR]);
            outTabular->setVal(r, tbl->cols(), tmp.ptr());
            tmp.cut(0);
            tmp.printf("%f", variance[lastR]);
            outTabular->setVal(r, tbl->cols()+1, tmp.ptr());

            lastR++;
        }
    }

    setOutTable(outTabular);
    return true;
}
