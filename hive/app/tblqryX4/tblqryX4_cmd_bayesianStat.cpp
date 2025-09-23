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
#include "tblqryX4_cmd.hpp"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class BStatCommand : public Command
        {
            private:
                sVec <idx> rowSet, colSet, categories;
                real margin;
                idx controlRow;

            public:
                BStatCommand(ExecContext & ctx) : Command(ctx)
                {
                    margin = 0.05;
                    controlRow = -1;
                }

                const char * getName() { return "bStat"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdBStatFactory(ExecContext & ctx) { return new BStatCommand(ctx); }
    };
};

bool BStatCommand::init(const char * op_name, sVariant * arg)
{
    if (arg->getDicElt("margin") != 0)
        margin = arg->getDicElt("margin")->asReal();

    if (sVariant * controlRowVal = arg->getDicElt("controlRow"))
    {
        if (controlRowVal->isList())
            controlRow = controlRowVal->getListElt(0)->asInt();
        else
            controlRow = arg->getDicElt("controlRow")->asInt();
    }

    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
    {
        if (rowSetVal->isList())
        {
            const char * p=rowSetVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
        }
    }

    if (sVariant * colSetVal = arg->getDicElt("categories"))
    {
        if (colSetVal->isList())
        {
            const char * p=colSetVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(categories),0ll,0ll,0ll);
        }
        else {
            _ctx.logError("Bad categories argument for bStat command");
            return false;
        }
    }

    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        if (colSetVal->isList())
        {
            const char * p=colSetVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
        }
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

bool BStatCommand::compute(sTabular * tbl)
{
    if (!colSet && colSet.dim() < 1)
    {
        for (idx i = 0; i < tbl->cols(); i++)
        {
            if (!contains (categories, i))
                colSet.vadd(1, i);
        }
    }
    if (!rowSet && rowSet.dim() < 1)
    {
        for (idx i = 0; i < tbl->rows(); i++)
        {
            if (i != controlRow)
                rowSet.vadd(1,i);
        }
    }

    sDic < sDic < sVec < idx > > > rowsToUseDicDic;
    sText::categoryListParseCsv(tbl , &(rowSet), &rowsToUseDicDic , 0, &(categories));

    sStr t1;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://bayesianStat.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(), "bayesianStat.csv",&t1);
    sFile::remove(t1);

    sFil bayesian(t1);
    bayesian.printf ("Column,Category,Statistic\n");

    for (idx c = 0; c < colSet.dim(); c++)
    {
        for (idx h = 0; h < rowsToUseDicDic.dim(); h++)
        {
            real proFreSum = 0;
            sVec <real> probFreqVec;
            const char * header = static_cast <const char *> (rowsToUseDicDic.id(h));
            real controlVal = 0;

            if (controlRow > -1)
                controlVal = tbl->rval(controlRow, colSet[c]);
            else
            {
                for (idx i = 0; i < rowSet.dim(); i++)
                    controlVal += tbl->rval(rowSet[i], colSet[c]);

                controlVal /= rowSet.dim();
            }

            for (idx v = 0; v < rowsToUseDicDic[header].dim(); v++)
            {
                const char * value = static_cast <const char *> (rowsToUseDicDic[header].id(v));
                real prob = 0;
                real frequency = 0;

                for (idx r = 0; r < rowsToUseDicDic[header][value].dim(); r++)
                {
                    if (contains(rowSet, rowsToUseDicDic[header][value][r]))
                    {
                        prob++;
                        real val = tbl->rval(rowsToUseDicDic[header][value][r], colSet[c]);
                        if (val >= controlVal*(margin + 1))
                            frequency++;
                    }
                }

                frequency /= prob;
                prob /= rowSet.dim();
                probFreqVec.vadd (1, frequency * prob);
                proFreSum += (frequency * prob);
            }

            for (idx v = 0; v < rowsToUseDicDic[header].dim(); v++)
            {
                const char * value = static_cast <const char *> (rowsToUseDicDic[header].id(v));
                sVariant tmp;
                tbl->val(tmp, -1, colSet[c], true);

                if (isnan(probFreqVec[v]/proFreSum))
                    bayesian.printf ("%s,%s:%s, \n", tmp.asString(), header, value);
                else
                    bayesian.printf ("%s,%s:%s,%g\n", tmp.asString(), header, value, probFreqVec[v]/proFreSum);
            }
        }
    }



    return true;
}
