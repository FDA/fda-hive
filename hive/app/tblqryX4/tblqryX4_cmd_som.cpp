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
#include <slib/utils/som.hpp>
#include "tblqryX4_cmd.hpp"
#include <ssci/math.hpp>
//#include <ssci/math/clust/clust2.hpp>



#define PRFX "som-"
#define COLORFILE "color.csv"
#define GEOMFILE "geom.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class SomCommand : public GraphCommand
        {
            public:
                SomCommand(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "som"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdSomFactory(ExecContext & ctx) { return new SomCommand(ctx); }
    };
};

bool SomCommand::init(const char * op_name, sVariant * arg)
{
    /*idx totalLength = sizeof(arg->getDicElt("colSetImg")->asString())+
                        sizeof(arg->getDicElt("colSetTree")->asString())+
                        sizeof(arg->getDicElt("rowSet")->asString()) +
                        sizeof("heatmap");*/
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

    if (arg->getDicElt("mapSize") != 0)
        mapSize = arg->getDicElt("mapSize")->asInt();

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

bool SomCommand::compute(sTabular * tbl)
{
    if (!colSetImg || colSetImg.dim() == 0)
    {
        for (idx i = 0; i < tbl->dimTopHeader(); i++)
            colSetImg.vadd(1,i);
    }
    if (!rowSet || rowSet.dim() == 0)
    {
        for (idx i = 0; i < tbl->dim(); i++)
            rowSet.vadd(1,i);
    }

    if(!uid || uid.dim() == 0)
        uid.vadd(1,0);


    idx rowCnt=(rowSet && rowSet.dim()) ? rowSet.dim() : tbl->rows(); //! the total number of rows to go through
    idx colCnt=(colSetImg && colSetImg.dim()) ? colSetImg.dim() : tbl->cols()-1;//! the total number of columns to go through

    if (mapSize == 0)
        mapSize = 50;

    real * tblP = (real *) sNew(rowCnt*(colCnt)*sizeof(real));

    for (idx i=0; i < rowCnt ; i ++)
    {
        idx iR=(rowSet && rowSet.dim()) ? rowSet[i] : i;
        for (idx j =0; j < colCnt; j++)
        {
            idx iC= (colSetImg && colSetImg.dim()) ? colSetImg[j] : j;
            tblP[i*(colCnt)+j] = tbl->rval(iR,iC);
        }
    }

    idx * destGeom = (idx *) sNew (sizeof(idx)*rowCnt*2);
    sSet(destGeom, -1, sizeof(idx)*rowCnt*2);
    sVec <sSom::pntclr> clr;

    sSom::som (tblP, rowCnt, colCnt, mapSize, &clr, destGeom);

    sFil colorFile;
    colorFile.printf("row,column,red,green,blue\r\n");
        for (idx i=0; i < clr.dim(); i++)
            colorFile.printf("%"DEC",%"DEC",%"DEC",%"DEC",%"DEC"\r\n", clr[i].point.r, clr[i].point.c, clr[i].color.r(), clr[i].color.g(), clr[i].color.b());
    _ctx.qproc().reqSetData(_ctx.outReqID(), COLORFILE, colorFile.mex());

    sFil destGeomFile;
    destGeomFile.printf("row,mapR,mapC\r\n");
    for (idx i = 0; i < rowCnt; i++)
    {
        idx iR=(rowSet && rowSet.dim()) ? rowSet[i] : i;
        destGeomFile.printf("%"DEC",%"DEC",%"DEC"\r\n", iR,destGeom[i*2], destGeom[i*2+1]);
    }
    _ctx.qproc().reqSetData(_ctx.outReqID(), GEOMFILE, destGeomFile.mex());

    return tbl;
}

