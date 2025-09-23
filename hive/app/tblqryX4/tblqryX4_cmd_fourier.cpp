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
#include <slib/utils/sort.hpp>
#include <ssci/math.hpp>
#include "tblqryX4_cmd.hpp"

#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

#define PRFX "rlda-"
#define OUTFILE "rlda.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class FourierCommand : public Command
        {
            private:
                sVec <idx> rowSet;
                sVec <idx> colSet;
                sVec <idx> categories;
                sVec <idx> uid;

            public:
                FourierCommand(ExecContext & ctx) : Command(ctx) {}

                const char * getName() { return "fourier"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return false; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdFourierFactory(ExecContext & ctx) { return new FourierCommand(ctx); }
    };
};

bool FourierCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        const char * p=colSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }
    else
        return false;

    if (sVariant * categoriesVal = arg->getDicElt("categories"))
    {
        const char * p=categoriesVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(categories),0ll,0ll,0ll);
    }
    else
    {
        categories.add(1);
        categories[0] = 0;
    }

    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
    {
        const char * p=rowSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }

    if (sVariant * uidVal = arg->getDicElt("uid"))
    {
        const char * p=uidVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(uid),0ll,0ll,0ll);
    }

    return true;
}

bool FourierCommand::compute(sTabular * tbl)
{
    if( !rowSet || rowSet.dim() < 1 ) {
        char p[24];
        sprintf(p, "0-%" DEC, tbl->rows() - 1);
        sString::scanRangeSet(p, 0, &(rowSet), 0ll, 0ll, 0ll);
    }
    categories.cut(1);

    if (!colSet || colSet.dim() < 1)
    {
        colSet.add (tbl->cols() - 1);

        idx pos = 0;
        for (idx i = 0; i < tbl->cols(); i++)
        {
            if (i == categories[0])
                continue;
            colSet[pos] = i;
            pos++;
        }
    }

    sDic < sDic < sVec < idx > > > rowsToUseDicDic;
    sText::categoryListParseCsv(tbl , &(rowSet), &rowsToUseDicDic , 0, &(categories));

    idx curCategory = 0;
    sDic < sVec < idx > > & rowsToUseDic = rowsToUseDicDic[curCategory];

    sTxtTbl * tmpTbl = new sTxtTbl();

    sStr dstFilePathBuf;
    const char * dstFilePath = 0;

    dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "FourierTransform.csv");
    sFil fFourier(dstFilePath);


    idx count2power = 2;
    while (count2power <= rowSet.dim())
        count2power *= 2;


    tmpTbl->initWritable(count2power/2-1,sTblIndex::fTopHeader,",");
    tmpTbl->addCell("ID");
    for (idx a = 0; a < count2power/2-1; a++)
    {
        char bb [32];
        sprintf (bb, "Harmonic %" DEC, a);
        tmpTbl->addCell(bb);
    }
    tmpTbl->addEndRow();

    for (idx c = 0; c < colSet.dim(); c++)
    {
        sVec < real > secDeriv;
        real * sd=secDeriv.add(2*rowSet.dim());
        sVec < real > x, y, realX;
        realX.add(rowSet.dim());
        x.add(rowSet.dim());
        y.add(rowSet.dim());
        idx id = 0;

        for (idx ii = 0; ii < rowsToUseDic.dim(); ii++)
        {
            char * cat = static_cast <char *> (rowsToUseDic.id(ii));
            for ( idx k= 0; k < rowsToUseDic[cat].dim(); k++)
            {
                realX[id] = rowsToUseDic[cat][id];
                x[id] = id;
                y[id] = tbl->rval(rowsToUseDic[cat][k], colSet[c]);
                id++;
            }
        }

        sFunc::spline::secDeriv(x, y, rowSet.dim(), REAL_MAX, REAL_MAX, sd, sd+rowSet.dim());

        sVec <real> coef;
        coef.add (2*count2power);
        for(idx is=0; is < count2power; ++is)
        {
            real xprime= is * rowSet.dim() * 1.0 / count2power;
            real yprime=sFunc::spline::calc(x, y, sd, rowSet.dim(), xprime, 0);

            coef[1+is] = yprime;
        }

        sMathNR::realft(coef,count2power,1);

        real max = -REAL_MAX;
        real min = REAL_MAX;
        for (idx a = 1; a < count2power+1; a++)
        {
            if (coef[a] < min)
                min = coef[a];
            if (coef[a] > max)
                max = coef[a];
        }
        sVec <real> scaledCoef;
        scaledCoef.add(count2power);
        for (idx a = 1; a < count2power+1; a++)
        {
            scaledCoef[a-1] =coef[a];
        }


        sStr topHeader;
        tbl->printTopHeader(topHeader, colSet[c]);
        char even [64];
        sprintf (even, "%s Even", topHeader.ptr());
        char odd [64];
        sprintf (odd, "%s Odd", topHeader.ptr());
        tmpTbl->addCell(even);
        for (idx a = 2; a < count2power; a+=2)
        {
            char bb [16];
            sprintf (bb, "%f", scaledCoef[a]);
            tmpTbl->addCell(bb);
        }
        tmpTbl->addEndRow();

        tmpTbl->addCell(odd);
        for (idx a = 1; a < count2power; a+=2)
        {
            char bb [16];
            sprintf (bb, "%f", scaledCoef[a]);
            tmpTbl->addCell(bb);
        }
        tmpTbl->addEndRow();
    }



    for (idx ic = 0; ic < tmpTbl->cols(); ic++)
    {
        for (idx ir = -1; ir < tmpTbl->rows(); ir++)
        {
            sStr tmpCell;
            if (ir < 0)
                tmpTbl->printTopHeader(tmpCell, ic);
            else
                tmpTbl->printCell(tmpCell, ir, ic);
            if (ir >= 0)
                fFourier.printf (",");
            fFourier.printf("%s", tmpCell.ptr());
        }
        fFourier.printf("\n");
    }

    return true;
}
