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
#include <ssci/math.hpp>

#define PRFX "tree-"
#define OUTFILE "cluster.tre"
#define OUTFILE1 "cluster1.tre"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class TreeCommand : public GraphCommand
        {
            public:
                TreeCommand(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "tree"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdTreeFactory(ExecContext & ctx) { return new TreeCommand(ctx); }
    };
};

bool TreeCommand::init(const char * op_name, sVariant * arg)
{
    if (arg->getDicElt("dataMode") != 0)
        dataMode = arg->getDicElt("dataMode")->asInt();

    if (arg->getDicElt("readNumsAsNums") != 0)
        readNumsAsNums = (sTree::DistanceMethods) arg->getDicElt("readNumsAsNums")->asInt();

    if (arg->getDicElt("buildMethod") != 0)
        buildMethod = (sTree::DistanceMethods) arg->getDicElt("buildMethod")->asInt();

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

    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
    {
        if (rowSetVal->isList())
        {
            for (idx i = 0; i < rowSetVal->dim(); i++)
                rowSet.vadd(1,rowSetVal->getListElt(i)->asInt());
        }
        else
            return false;
    }

    if (sVariant * uidVal = arg->getDicElt("uid"))
    {
        if (uidVal->isList())
        {
            for (idx i = 0; i < uidVal->dim(); i++)
                uid.vadd(1,uidVal->getListElt(i)->asInt());
        }
        else
            return false;
    }

    return true;
}


bool TreeCommand::compute(sTabular * tbl)
{
    if (!colSetImg || colSetImg.dim() == 0)
    {
        for (idx i = 0; i < tbl->cols(); i++)
            colSetImg.vadd(1,i);
    }
    if (!rowSet || rowSet.dim() == 0)
    {
        for (idx i = 0; i < tbl->rows(); i++)
            rowSet.vadd(1,i);
    }


    if(!uid || uid.dim() == 0)
        uid.vadd(1,0);


    sStr pathT;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" OUTFILE,0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(),OUTFILE,&pathT);
    sFile::remove(pathT);
    sFil out(pathT);

    sVec <idx> nothing;
    sTree::generateTree(out, &(colSetImg), &(rowSet),tbl, &nothing,1,&(uid), buildMethod);

    sMatrix mat;
    mat.parseTabular(tbl, &(rowSet), &(colSetImg), 0, 0,0,dataMode,readNumsAsNums ? true : false);

    return true;
}

