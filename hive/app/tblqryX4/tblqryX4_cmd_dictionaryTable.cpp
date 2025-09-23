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
        class DicTestCommand : public Command
        {
            private:
                sVec <idx> rowSet, colSet, valSet;
                idx option;
                real defaultMissing ;

            public:
                DicTestCommand(ExecContext & ctx) : Command(ctx)
                {
                    option = 1;
                    defaultMissing = (real)sIdxMax;
                }

                const char * getName() { return "dictionaryTable"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdDicTestFactory(ExecContext & ctx) { return new DicTestCommand(ctx); }
    };
};

bool DicTestCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * val = arg->getDicElt("rowSet")){
        const char * p=val->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(rowSet),0ll,0ll,0ll);
    }
    if (sVariant * val = arg->getDicElt("colSet")){
        const char * p=val->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }
    if (sVariant * val = arg->getDicElt("valSet")){
        const char * p=val->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(valSet),0ll,0ll,0ll);
    }
    if (sVariant * val = arg->getDicElt("option")){
        option = val->asInt();
    }
    if (sVariant * val = arg->getDicElt("defaultMissing")){
        defaultMissing = val->asReal();
    }

    return true;
}

bool DicTestCommand::compute(sTabular * tbl)
{
    {
        sDic < sVec <sStr> > rowsColsDic;
        idx rowCol, colCol, valCol;
        sDic < idx > uniqueRows;
        sDic < idx > uniqueCols;

        if (!rowSet.dim() || !colSet.dim() || !valSet.dim()) return false;

        rowCol = rowSet[0];
        colCol = colSet[0];
        valCol = valSet[0];

        for (idx r = 0; r < tbl->rows(); r++){
            sStr rowCell, colCell, valCell;
            tbl->printCell(rowCell, r, rowCol);
            tbl->printCell(colCell, r, colCol);
            tbl->printCell(valCell, r, valCol);

            sStr keyVal;
            keyVal.printf("%s:%s", rowCell.ptr(), colCell.ptr());

            idx size = rowsColsDic[keyVal.ptr()].dim();
            rowsColsDic[keyVal.ptr()].add(1);
            rowsColsDic[keyVal.ptr()][size].printf("%s", valCell.ptr());

            if (!uniqueRows[rowCell.ptr()])
                uniqueRows[rowCell.ptr()] = uniqueRows.dim()+1;
            if (!uniqueCols[colCell.ptr()])
                uniqueCols[colCell.ptr()] = uniqueCols.dim();
        }

        sVariantTbl * tempTbl = new sVariantTbl (uniqueCols.dim(), 0);

        for (idx r = 0; r < uniqueRows.dim(); r++){
            const char * rowCell = static_cast <const char *> (uniqueRows.id(r));
            for (idx c = 0; c < uniqueCols.dim(); c++){
                const char * colCell = static_cast <const char *> (uniqueCols.id(c));
                sStr keyVal;
                keyVal.printf("%s:%s", rowCell, colCell);

                if (rowsColsDic[keyVal.ptr()].dim()){
                    if(defaultMissing != sIdxMax && ((option == 0 && rowsColsDic[keyVal.ptr()][0].length() < 1) ||
                            (option == 1 && rowsColsDic[keyVal.ptr()][rowsColsDic[keyVal.ptr()].dim()-1].length() < 1))){
                        sStr strToUse, dest;
                        strToUse.printf("%f", defaultMissing);
                        sString::escapeForCSV(dest, strToUse, strToUse.length());
                        tempTbl->setVal(uniqueRows[rowCell]-2, uniqueCols[colCell], strToUse.ptr());
                        continue;
                    }
                    if (option == 1){
                        tempTbl->setVal(uniqueRows[rowCell]-2, uniqueCols[colCell], rowsColsDic[keyVal.ptr()][rowsColsDic[keyVal.ptr()].dim()-1].ptr());
                    }
                    else if (option == 0){
                        tempTbl->setVal(uniqueRows[rowCell]-2, uniqueCols[colCell], rowsColsDic[keyVal.ptr()][0].ptr());
                    }
                    else{
                        sStr strToUse;
                        strToUse.printf("\"");
                        for (idx i=0; i < rowsColsDic[keyVal.ptr()].dim(); i++){
                            if (i>0) strToUse.printf(",%s", rowsColsDic[keyVal.ptr()][i].ptr());
                            else strToUse.printf("%s", rowsColsDic[keyVal.ptr()][i].ptr());
                        }
                        strToUse.printf("\"");
                        tempTbl->setVal(uniqueRows[rowCell]-2, uniqueCols[colCell], strToUse.ptr());
                    }
                }

            }
        }

        tempTbl->setVal(-1, 0, "id");
        for (idx c = 0; c < uniqueCols.dim(); c++){
            const char * colCell = static_cast <const char *> (uniqueCols.id(c));
            tempTbl->setVal(-1, c+1, colCell);
        }
        for (idx r = 0; r < uniqueRows.dim(); r++){
            const char * rowCell = static_cast <const char *> (uniqueRows.id(r));
            tempTbl->setVal(r, 0, rowCell);
        }

        if(defaultMissing != sIdxMax){
            sStr strToUse, dest;
            strToUse.printf("%f", defaultMissing);
            sString::escapeForCSV(dest, strToUse, strToUse.length());
            for (idx r = 0; r < uniqueRows.dim(); r++){
                for (idx c = 1; c < uniqueCols.dim()+1; c++){
                    if(tempTbl->missing(r, c)){
                        tempTbl->setVal(r, c, dest.ptr());
                    }
                }
            }
        }

        setOutTable(tempTbl);
    }

    return true;
}

