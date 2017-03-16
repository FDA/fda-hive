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
#include <ssci/math/clust/clust.hpp>
#include <xlib/image.hpp>
#include "utils.hpp"

#define PRFX "heatmap-"
#define OUTFILE "heatmap.png"
#define HORIZONTAL "horizontal.tre"
#define VERTICAL "vertical.tre"
#define OUTCSV "heatmap.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class HeatmapCommand : public GraphCommand
        {
            public:
                HeatmapCommand(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "heatmap"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdHeatmapFactory(ExecContext & ctx) { return new HeatmapCommand(ctx); }
    };
};

bool HeatmapCommand::init(const char * op_name, sVariant * arg)
{
    if (arg->getDicElt("colorMethod") != 0)
        colorMethod = arg->getDicElt("colorMethod")->asInt();
    else
        colorMethod = 0;

    if (arg->getDicElt("buildMethod") != 0)
        buildMethod = (sTree::DistanceMethods) arg->getDicElt("buildMethod")->asInt();
    else
        buildMethod = (sTree::DistanceMethods) 0;

    if (sVariant * colSetImgVal = arg->getDicElt("colSetImg"))
    {
        if (colSetImgVal->isList())
        {
            for (idx i = 0; i < colSetImgVal->dim(); i++)
                colSetImg.vadd(1, colSetImgVal->getListElt(i)->asInt());
        }
        else
        {
            const char * p=colSetImgVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(colSetImg),0ll,0ll,0ll);
        }
    }

    if (sVariant * colSetTreeVal = arg->getDicElt("colSetTree"))
    {
        if (colSetTreeVal->isList())
        {
            for (idx i=0; i<colSetTreeVal->dim(); i++) {
               colSetTree.vadd(1, colSetTreeVal->getListElt(i)->asInt());
           }
        }
        else
        {
            const char * p=colSetTreeVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(colSetTree),0ll,0ll,0ll);
        }
    }

    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
    {
        if (rowSetVal->isList())
        {
            for (idx i = 0; i < rowSetVal->dim(); i++)
                rowSet.vadd(1, rowSetVal->getListElt(i)->asInt());
        }
        else
        {
            const char * p=rowSetVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
        }
    }

    if (sVariant * uidVal = arg->getDicElt("uid"))
    {
        if (uidVal->isList())
        {
            for (idx i = 0; i < uidVal->dim(); i++)
                uid.vadd(1, uidVal->getListElt(i)->asInt());
        }
        else
        {
            const char * p=uidVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(uid),0ll,0ll,0ll);
        }
    }

    return true;
}

bool contains (sVec <idx> arr, idx val){

    for (idx i = 0; i < arr.dim(); i++)
        if (arr[i]==val) return true;

    return false;
}

bool HeatmapCommand::compute(sTabular * tbl)
{
    idx cx=4;
    idx cy=4;

    if( colSetImg.dim() == 0 ) {
        idx ncols = tbl->cols();
        colSetImg.resize(ncols - uid.dim());
        idx curCnt = 0;
        for(idx i = 0; i < ncols; i++) {
            if (!contains(uid, i)){
                colSetImg[curCnt] = i;
                curCnt++;
            }
        }
    }

    //copy appropriately here
    if( colSetTree.dim() == 0 ) {
        idx ncols = colSetImg.dim();
        colSetTree.resize(ncols);
        for(idx i = 0; i < ncols; i++) {
            colSetTree[i] = colSetImg[i];
        }
    }

    if( rowSet.dim() == 0 ) {
        idx nrows = tbl->rows();
        rowSet.resize(nrows);
        for(idx i = 0; i < nrows; i++) {
            rowSet[i] = i;
        }
    }

    sHeatmap::ColorLimits limitsFullScale;

    sStr pathT;
    sVec <idx> actualRowOrder;
    sVec < sVec< real > > vals;
    sVec <idx> actualColOrder;

    {

        _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" OUTFILE,0,0);
        _ctx.qproc().reqDataPath(_ctx.outReqID(),OUTFILE,&pathT);
        sFile::remove(pathT);

        sStr t1;
        _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" HORIZONTAL,0,0);
        _ctx.qproc().reqDataPath(_ctx.outReqID(),HORIZONTAL,&t1);
        sFile::remove(t1);

        sFil horizontalTree(t1);


        //sTree::generateTree(horizontalTree, &treeColumnsToUse, &rowsToUse,tbl,&actualRowOrder,1,0,method);
        sTree::generateTree(horizontalTree, &(colSetTree), &(rowSet),tbl,&actualRowOrder,1,0,buildMethod);
    }

    {
        sStr t2;
        _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" VERTICAL,0,0);
        _ctx.qproc().reqDataPath(_ctx.outReqID(),VERTICAL,&t2);
        sFile::remove(t2);

        sFil verticalTree(t2);

        //sTree::generateTree(verticalTree, &imgColumnsToUse, &rowsToUse,tbl,&actualColOrder,0,0,method);
        sTree::generateTree(verticalTree, &(colSetImg), &(rowSet),tbl,&actualColOrder,0,0,buildMethod);

        sVec < sVec <sClr > > colors;

        sHeatmap::generateRangeMap( &vals, tbl, &actualColOrder,  &actualRowOrder,colorMethod);
        sImage::generateHeatmap (pathT, &vals, cx, cy , &limitsFullScale);
        //sHeatmap::generatePNG(filename,  pvals, cx, cy , limits, &colors);

       //sHeatmap::generatePNG(pathT,  &tbl, &actualColOrder, &actualRowOrder,colorMethod, cx, cy , &limitsFullScale,&vals);
    }

    if (!uid || uid.dim() == 0)
        uid.vadd(1,0);

    {
        sStr t3;
        _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" OUTCSV,0,0);
        _ctx.qproc().reqDataPath(_ctx.outReqID(),OUTCSV,&t3);
        sFile::remove(t3);

        sFil heatmapCSV(t3);

        heatmapCSV.printf("%s","rows");
        for( idx ic=0; ic<actualColOrder.dim(); ++ic) {
            heatmapCSV.printf(",");
            sStr escaped, tmp;
            tbl->printTopHeader(tmp,actualColOrder[ic]);
            sString::escapeForCSV(escaped,tmp);
            heatmapCSV.printf("%s", escaped.ptr());
        }
        heatmapCSV.printf("\n");

        for( idx ir=0; ir<actualRowOrder.dim(); ++ir) {
            idx iR=actualRowOrder[ir];
            for( idx iu=0; iu<uid.dim(); ++iu  ) {
                if(iu)heatmapCSV.printf(" ");
                sStr tmp, escaped;
                tbl->printCell(tmp,iR,uid[iu]);
                sString::escapeForCSV(escaped,tmp);
                heatmapCSV.printf("%s", escaped.ptr());
            }
            for( idx ic=0; ic<actualColOrder.dim(); ++ic)
            {
                if (vals[ir][ic]==-1)
                    heatmapCSV.printf(",");
                else
                    heatmapCSV.printf(",%g",vals[ir][ic]);
            }
            heatmapCSV.printf("\n");
        }
    }

    return true;
}
