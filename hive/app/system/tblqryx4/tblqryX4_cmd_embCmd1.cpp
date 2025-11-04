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

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class EmbCmdCollapseCols : public Command
        {
            private:
                sVec <idx> colSet,colIds;

            public:
                EmbCmdCollapseCols(ExecContext & ctx) : Command(ctx)
                {
                }

                const char * getName() { return "embCmd1Collapse"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }
                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdEmbCmdCollapseCols(ExecContext & ctx) { return new EmbCmdCollapseCols(ctx); }
    };
};

bool EmbCmdCollapseCols::init(const char * op_name, sVariant * arg)
{
    if (sVariant * val = arg->getDicElt("colIds")){
        const char * p=val->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colIds),0ll,0ll,0ll);
    }
    if(!colIds.dim())colIds.vadd(1,0);

    if (sVariant * val = arg->getDicElt("colSet")){
        const char * p=val->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }

    return true;
}

bool EmbCmdCollapseCols::compute(sTabular * tbl)
{

    sVariantTbl * outTbl = new sVariantTbl (colIds.dim()+1,0);
    sStr out;


    idx colNew=0;
    if(!colSet){

        for (idx ic = colIds[colIds.dim()-1]+1; ic < tbl->cols(); ++ic) {
            colSet.vadd(1,ic);
        }

    }


    for (idx ic = 0; ic < colIds.dim(); ++ic){
        out.cut(0);
        tbl->printCell(out, -1, colIds[ic]);
        outTbl->setVal(-1, colNew++, out.ptr(0) );
    }
    outTbl->setVal(-1, colNew++, "total_sum" );


    for (idx ir = 0; ir < tbl->rows(); ir++){
        idx newIcols=0;
        for (idx ic = 0; ic < colIds.dim(); ++ic){
            out.cut(0);
            tbl->printCell(out, ir, colIds[ic]);
            outTbl->setVal(ir,newIcols++, out.ptr(0));
        }

        idx coldim=colSet ? colSet.dim() : tbl->cols();
        real sum=0., rval=0;
        for (idx ic = 0; ic < coldim ; ic++){
            idx c=colSet[ic];
            if(c>=tbl->cols())break;

            out.cut(0);
            tbl->printCell(out, ir, c);
            if(out.length()<1)continue;
            rval=0;
            sscanf(out.ptr(0),"%lf",&rval);
            sum+=rval;
        }

        out.printf(0,"%lg",sum);
        outTbl->setVal(ir,colIds.dim(), out.ptr(0));
    }


    setOutTable(outTbl);

    return true;
}

