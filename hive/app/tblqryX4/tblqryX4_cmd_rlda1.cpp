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
        class RldaCommand : public Command
        {
            private:
                sVec <idx> rowSet;
                sVec <idx> colSet;
                sVec <idx> categories;
                sVec <idx> uid;
                idx iterMin, cntSample, topChoiceForFinal, goodShannonMaxThreshold, useEvalScaling, validateNormal;
                real shannonThreshold;

            public:
                RldaCommand(ExecContext & ctx) : Command(ctx)
                {
                    iterMin = 1000;
                    cntSample = 10;
                    topChoiceForFinal = 500;
                    shannonThreshold = 0.001;
                    goodShannonMaxThreshold = 5;
                    useEvalScaling = 1;
                    validateNormal = 0;
                }

                const char * getName() { return "rlda"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return false; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdRldaFactory(ExecContext & ctx) { return new RldaCommand(ctx); }
    };
};

bool RldaCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        const char * p=colSetVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(colSet),0ll,0ll,0ll);
    }

    if (sVariant * categoriesVal = arg->getDicElt("categories"))
    {
        const char * p=categoriesVal->asString();if(*p=='[')++p;
        sString::scanRangeSet(p,0,&(categories),0ll,0ll,0ll);
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

    if (arg->getDicElt("iterMin") != 0)
        iterMin = arg->getDicElt("iterMin")->asInt();

    if (arg->getDicElt("cntSample") != 0)
        cntSample = arg->getDicElt("cntSample")->asInt();

    if (arg->getDicElt("topChoiceForFinal") != 0)
        topChoiceForFinal = arg->getDicElt("topChoiceForFinal")->asInt();

    if (arg->getDicElt("shannonThreshold") != 0)
        shannonThreshold = arg->getDicElt("shannonThreshold")->asReal();

    if (arg->getDicElt("goodShannonMaxThreshold") != 0)
        goodShannonMaxThreshold = arg->getDicElt("goodShannonMaxThreshold")->asInt();

    if (arg->getDicElt("useEvalScaling") != 0)
        useEvalScaling = arg->getDicElt("useEvalScaling")->asInt();

    if (arg->getDicElt("validateNormal") != 0)
        validateNormal = arg->getDicElt("validateNormal")->asInt();

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

bool RldaCommand::compute(sTabular * tbl)
{
    sDic < sDic < sVec < idx > > > rowsToUseDicDic;
    sText::categoryListParseCsv(tbl , &(rowSet), &rowsToUseDicDic , 0, &(categories));


    sMatrix OriMat;
    sDic <idx> OriRowIDs, OriColIDs, SubColIDs;
    sRlda Rlda;

    sMatrix * Mat=&OriMat;
    sDic <idx> * rowIDs=&OriRowIDs, * colIDs=&OriColIDs;


    idx cntCats=categories.dim();
    if(colSet.dim()==0){
        if(uid.dim())
            categories.glue(&(uid));
        else
            categories.vadd(1,0);

        for (idx i = 0; i < tbl->cols(); i++)
        {
            if (!contains (categories, i))
                colSet.vadd(1, i);
        }
    }
    useEvalScaling=0;
    OriMat.parseTabular(tbl, &(rowSet), &(colSet), &OriColIDs, &OriRowIDs, 0, 0,1., 1 ? true : false,0 , uid.dim() ? uid[0] : 0 );
    categories.cut(cntCats);



    idx iterMax=OriMat.cols()>500 ? cntSample*2*tbl->cols() : 0 ;

    validateNormal=0;

    for (idx c=0; c<categories.dim(); c++)
    {
        sDic < sVec < idx > > & rowsToUseDic = rowsToUseDicDic[c];
        sStr dstFilePathBuf;
        const char * dstFilePath = 0;

        sMatrix matrixForCurCategory;

        if (validateNormal)
        {
            sVec <idx> columnsForCurCategory;

            dstFilePathBuf.cut(0);
            dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Excluded-%" DEC ".csv", c+1);
            sFil excludedFile(dstFilePath);

            for (idx ii = 0; ii < colSet.dim(); ii++)
            {
                idx curCol = colSet[ii];

                sVec <real> stdDevForCurCategory; stdDevForCurCategory.add(rowsToUseDic.dim());

                for (idx x = 0; x < rowsToUseDic.dim(); x++)
                {
                    const char * catVal = static_cast <const char *> (rowsToUseDic.id(x));
                    sVec <idx> curRowVec = rowsToUseDic[catVal];
                    real mean = 0;

                    for (idx y = 0; y < curRowVec.dim(); y++)
                        mean += tbl->rval(curRowVec[y], curCol);
                    mean /= curRowVec.dim();
                    stdDevForCurCategory[x] = 0;
                    for (idx y = 0; y < curRowVec.dim(); y++)
                    {
                        real val = tbl->rval(curRowVec[y], curCol);
                        stdDevForCurCategory[x] += (val - mean) * (val - mean);
                    }
                    stdDevForCurCategory[x] /= curRowVec.dim();
                    stdDevForCurCategory[x] = sqrt (stdDevForCurCategory[x]);
                }

                bool adding = true;
                for (idx x = 0; x < rowsToUseDic.dim(); x++)
                {
                    if (stdDevForCurCategory[x] == 0)
                    {
                        adding = false;
                        break;
                    }
                }

                if (adding)
                {
                    columnsForCurCategory.vadd(1,curCol);
                }
                else
                {
                    sStr topHeader;
                    tbl->printTopHeader(topHeader, curCol);
                    excludedFile.printf("%s\n", topHeader.ptr());
                }
            }

            matrixForCurCategory.parseTabular(tbl, &(rowSet), &(columnsForCurCategory), &OriColIDs, &OriRowIDs, 0, 0, 1 ? true : false,columnsForCurCategory.dim()==0 ? &(categories) : 0 );
        }


        if(iterMax>1) {
            if (validateNormal)
                Rlda.computeExtraLarge(matrixForCurCategory,  rowsToUseDic, cntSample, topChoiceForFinal, iterMax, iterMin, shannonThreshold, goodShannonMaxThreshold, useEvalScaling);
            else
                Rlda.computeExtraLarge(OriMat,  rowsToUseDic, cntSample, topChoiceForFinal, iterMax, iterMin, shannonThreshold, goodShannonMaxThreshold, useEvalScaling);

            Mat=&Rlda.SubMatrix;
            SubColIDs.empty();
            for( idx it=0; it<Rlda.samplingSet.dim(); ++it) {
                const char * dd =(const char * )OriColIDs.id(Rlda.samplingSet[it]);

                SubColIDs.set(dd);
            }
            colIDs=&SubColIDs;

        } else
            Rlda.compute(*Mat, rowsToUseDic);


        sMatrix::MatrixDicHeaders mh;
        mh.cols=colIDs;mh.rows=colIDs;

        dstFilePathBuf.cut(0);
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Vectors-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            Rlda.ldaTransformVecs.out(&ot, &mh, false, true);
        }

        sDic <idx> combinations;


        dstFilePathBuf.cut(0);
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Contributors-%" DEC ".csv", c+1);
        sStr ttt;

        dstFilePathBuf.cut(0);
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "ContributorGraph-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            ot.printf("Column");
            for (idx y = 0; y < rowsToUseDic.dim()-1; y++)
                ot.printf (",Contribution Coefficient %" DEC, y);
            ot.printf("\n");

            sVec <real> firstCol;
            for (idx x = 0; x < Rlda.ldaTransformVecs.rows(); x++)
            {
                firstCol.add(1);
                firstCol[firstCol.dim()-1]=Rlda.ldaTransformVecs[x][0];
            }
            sVec <idx> index;
            index.add(firstCol.dim());
            sSort::sortabs(firstCol.dim(), firstCol.ptr(), index.ptr());

            for (idx x = (index.dim() > 500 ? 500 : index.dim()) -1; x > -1; x--)
            {
                idx curRow = index[x];

                ot.printf("%s",static_cast <const char *> (colIDs->id(curRow)));
                for (idx y = 0; y < rowsToUseDic.dim()-1; y++)
                    ot.printf (",%lf", Rlda.ldaTransformVecs[curRow][y]);
                ot.printf("\n");
            }
        }


        if(iterMax>1) {
            dstFilePathBuf.cut(0);
            dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Convergent-%" DEC ".csv", c+1);
            sFil ot(dstFilePath);

            ot.printf("Measurment,Sampling");
            for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar)
                ot.printf(",class-%" DEC "%%,contrib-%" DEC,ivar,ivar);
            ot.printf("\n");

            for(idx iii=0; iii<Rlda.samplingSet.dim(); ++iii){
                idx iRealCol=Rlda.samplingSet[iii];
                idx occ=Rlda.samplingLdaTransforOccurence[iRealCol];
                if(!occ)break;

                ot.printf("%" DEC ",%s,%" DEC "",iRealCol,(const char* ) colIDs->id(iii),occ);
                for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar){
                    real ave=Rlda.samplingLdaTransforVecCumulator.val(iRealCol,ivar) / occ ;
                    ot.printf(",%.2lf%%,%.4lf",100*ave/Rlda.totalVals[ivar], ave);
                }
                ot.printf("\n");
            }
        }


        sMatrix::MatrixDicHeaders ho;
        ho.cols=colIDs;
        ho.rows=rowIDs;

        dstFilePathBuf.cut(0);
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Source-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            Mat->out(&ot, &ho, false, true, "%lf",0,0, 0, 0, 0);
        }

        sMatrix translatedCoordinates;
        translatedCoordinates.multiplyMatrixes(*Mat,Rlda.ldaTransformVecs);
        dstFilePathBuf.cut(0);
        ho.cols=&combinations;
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Translated-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            translatedCoordinates.out(&ot, &ho, false, true, "%lf",0,0, 0, 0, 0);
        }


    }

    return true;
}

