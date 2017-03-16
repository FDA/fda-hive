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

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class FoldChangeCommand : public Command
        {
            private:
                sVec <idx> colSet_1;
                sVec <idx> colSet_2;
                real threshold;
                RowSorter sorter;   // used for sorting date

            public:
                FoldChangeCommand(ExecContext & ctx) : Command(ctx)
                {
                    threshold = -1;
                }

                const char * getName() { return "fold_change"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return false; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdFoldChangeFactory(ExecContext & ctx) { return new FoldChangeCommand(ctx); }
    };
};

bool FoldChangeCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * rowSetVal = arg->getDicElt("col_1"))
    {
        const char * p=rowSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet_1),0ll,0ll,0ll);
    }

    if (sVariant * colSetVal = arg->getDicElt("col_2"))
    {
        const char * p=colSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet_2),0ll,0ll,0ll);
    }
    if (sVariant * colSetVal = arg->getDicElt("threshold"))
    {
        //const char * p=colSetVal->asString();if(*p=='[')++p;
        //sString::scanRangeSet(p,0,&(colSet_2),0ll,0ll,0ll);
        threshold = colSetVal->asReal();
    }

    if (!colSet_1.dim() || !colSet_2.dim()){
        return false;
    }
    sStr tmpFormula;
    tmpFormula.printf("${Unique Col-%" DEC "} + ${Unique Col-%" DEC "}",colSet_1[0]+1, colSet_2[0]+1);
    sVariant sorter_arg;
    sorter_arg.setDic();
    sVariant * sorter_formula_arg = sorter_arg.setElt("formulas", 0);
    sorter_formula_arg->setList();
    sorter_formula_arg->push(tmpFormula.ptr()); // put uniques at bottom
    sorter_formula_arg->push("-abs(${Fold-Change})"); // put largest abs change at top
    sorter_formula_arg->push("-${Fold-Change}"); // put +change before -change
    if( !sorter.init(&sorter_arg, &_ctx, "fold-change") ) {
        return false;
    }

    return true;
}

bool FoldChangeCommand::compute(sTabular * tbl)
{
    if (!colSet_1 || colSet_1.dim() == 0)
    {
        _ctx.logError("%s operation: bad col_1 argument", getName());
        return false;
    }

    if (!colSet_2 || colSet_2.dim() == 0)
    {
        _ctx.logError("%s operation: bad col_2 argument", getName());
        return false;
    }

    idx col_1 = colSet_1[0];
    idx col_2 = colSet_2[0];

    sTxtTbl * toReturn = new sTxtTbl(); // add 3 more columns to the current one: fold-change,unique 1, unique 2

    toReturn->initWritable(tbl->cols()+3,sTblIndex::fTopHeader,",");

    real value_1, value_2;
    bool uniq_1 = false, uniq_2=false;

    sVariant tmp;

    // go from the header to the end of the table
    for (idx ir = -1; ir < tbl->rows(); ir++) // header index = -1
    {
        uniq_1 = uniq_2 = false;

        if (ir != -1) { // the value of the selected columns when not the header row
            value_1 = tbl->rval(ir, col_1, 0);
            value_2 = tbl->rval(ir, col_2, 0);
        }
        // compare those values to the threshold when not the header row
        if (ir != -1 && (value_1 < threshold || value_2 < threshold)) {
            continue;
        }

        // copy the row of the original table to the new table
        for (idx ic = 0; ic < tbl->cols(); ic++)
        {
            tmp.setNull();
            tbl->val(tmp, ir, ic, true);
            toReturn->addCell(tmp);
        }

        if (ir==-1){
            sStr hdr_col;
            hdr_col.printf("Unique Col-%" DEC "",col_1+1);
            toReturn->addCell("Fold-Change",11, sVariant::value_REAL);
            toReturn->addCell(hdr_col.ptr(),0,sVariant::value_INT);
            hdr_col.printf(0,"Unique Col-%" DEC "",col_2+1);
            toReturn->addCell(hdr_col.ptr(),0, sVariant::value_INT);
        }

        else {
            sVariant ratio;
            if (value_1==0 && value_2==0){
                ratio.setReal(1);
            } else if ( value_1 == 0 && value_2 != 0 ){
                uniq_2 = true;
            } else if (  value_1 != 0 && value_2 == 0  ){
                uniq_1 = true;
            } else {
                if (value_1 > value_2) {
                    ratio.setReal(value_1/value_2);
                }
                else {
                    ratio.setReal (- ( value_2/value_1 ));
                }
            }
            toReturn->addCell(ratio);
            toReturn->addBoolCell(uniq_1);
            toReturn->addBoolCell(uniq_2);

        }
        toReturn->addEndRow();
    }

    sReorderedTabular * reordered = new sReorderedTabular(toReturn, true);
    _ctx.qlangCtx().setTable(reordered);
    if( !sorter.sort(reordered) ) {
        _ctx.qlangCtx().setTable(0);
        delete reordered;
        return false;
    }
    _ctx.qlangCtx().setTable(0);
    //sTxtTbl * reordered = new sTxtTbl(); // add 3 more columns to the current one: fold-change,unique 1, unique 2
#if 0
    reordered->initWritable(toReturn->cols(),sTblIndex::fTopHeader,",");

    for (idx ir=-1; ir < sortedIndexes.dim(); ++ir){
        for (idx ic=0; ic < toReturn->cols(); ++ic){
            if (ir==-1){
                sVariant curCell;
                toReturn->val(curCell,ir,ic,true);
                reordered->addCell(curCell);
            } else {
                idx newIndex = sortedIndexes[ir];
                sVariant myCurCell;
                toReturn->val(myCurCell,newIndex,ic,true);
                reordered->addCell(myCurCell);
            }
        }
        reordered->addEndRow();
    }
#endif

//  setOutTable(toReturn);
    setOutTable(reordered);
    return true;
}
