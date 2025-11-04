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

#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

#define PRFX "rlda-"
#define OUTFILE "rlda.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class RldaCommand : public GraphCommand
        {
            public:
                RldaCommand(ExecContext & ctx) : GraphCommand(ctx) {}

                const char * getName() { return "rlda"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return false; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdHeatmapFactory(ExecContext & ctx) { return new RldaCommand(ctx); }
    };
};

bool RldaCommand::init(const char * op_name, sVariant * arg)
{
    if (arg->getDicElt("dataMode") != 0)
        dataMode = (sTree::DistanceMethods) arg->getDicElt("dataMode")->asInt();

    if (arg->getDicElt("readNumsAsNums") != 0)
        readNumsAsNums = (sTree::DistanceMethods) arg->getDicElt("readNumsAsNums")->asInt();

    if (sVariant * colSetVal = arg->getDicElt("colSet"))
    {
        sString::scanRangeSet(colSetVal->asString(),0,&(colSetImg),0ll,0ll,0ll);
    }

    if (sVariant * categoriesVal = arg->getDicElt("categories"))
    {
        sString::scanRangeSet(categoriesVal->asString(),0,&(categories),0ll,0ll,0ll);
    }

    if (sVariant * rowSetVal = arg->getDicElt("rowSet"))
        {
            sString::scanRangeSet(rowSetVal->asString(),0,&(rowSet),0ll,0ll,0ll);
        }

        if (sVariant * uidVal = arg->getDicElt("uid"))
        {

            sString::scanRangeSet(uidVal->asString(),0,&(uid),0ll,0ll,0ll);
        }

    return true;
}

bool RldaCommand::compute(sTabular * tbl)
{
    if (!rowSet || rowSet.dim() == 0)
    {
        for (idx i = 0; i < tbl->dim(); i++)
            rowSet.vadd(1,i);
    }

    sDic < sDic < sVec < idx > > > rowsToUseDicDic;
    sText::categoryListParseCsv(tbl , &(rowSet), &rowsToUseDicDic , 0, &(categories));


    sMatrix Mat;
    sDic <idx> rowIDs, colIDs;
    sRlda Rlda;
    sMatrix * ldaTransformVecs=&Rlda.ldaTransformVecs;
    sVec < real > * ldaTransformVals=&Rlda.ldaTransformVals;
    idx useEValScaling=1;
    idx quit=0;


    Mat.parseTabular(tbl, &(rowSet), &(colSetImg), &colIDs, &rowIDs,0,dataMode,readNumsAsNums ? true : false);


    sVec < idx > RandomizerBuf;
    idx iterMax=1, cntSample=4, * randomizerBuf=0;
    sMatrix newmat,samplingLdaTransforVecCumulator;
    sVec < real > samplingLdaTransforValCumulator;

    if(iterMax>1) {
        randomizerBuf=RandomizerBuf.add(Mat.cols());

        idx cntMaxCategsForCategGroup=0;
        for( idx ir=0; ir<rowsToUseDicDic.dim() ; ++ir ) {
            if( cntMaxCategsForCategGroup<rowsToUseDicDic.ptr(ir)->dim())
                cntMaxCategsForCategGroup=rowsToUseDicDic.ptr(ir)->dim();
        }
    }

    for (idx c=0; c<categories.dim(); c++)
    {
        sDic < sVec < idx > > & rowsToUseDic = rowsToUseDicDic[c];


        if(iterMax > 1 ) {
            samplingLdaTransforVecCumulator.resize(Mat.cols(),rowsToUseDic.dim()-1);
            samplingLdaTransforValCumulator.resize(Mat.cols());
            samplingLdaTransforVecCumulator.set(0);
            samplingLdaTransforValCumulator.set(0);
            for( idx iter=0; iter<iterMax; ++iter ) {

                for( idx irnd=0; irnd<RandomizerBuf.dim() ; ++irnd ) {
                    randomizerBuf[irnd]=(((idx)(sRand::random1()*0x7FFFFFFF))<<32)|irnd;
                }
                sSort::sort(RandomizerBuf.dim(),randomizerBuf);
                for( idx irnd=0; irnd<cntSample; ++irnd ) {
                    randomizerBuf[irnd]&=0x00000000FFFFFFFFll;
                }
                RandomizerBuf.cut(cntSample);

                newmat.empty();
                Mat.extractColset(newmat, RandomizerBuf) ;

                RandomizerBuf.resize(Mat.cols());

                Rlda.compute(newmat, rowsToUseDic);


                for( idx icol=0; icol<cntSample; ++icol ){
                    real ev=Rlda.ldaTransformVals[icol];ev=ev*ev;
                    samplingLdaTransforValCumulator[icol]+=ev;

                    idx iRealCol=randomizerBuf[icol];
                    for( idx ivar=0; ivar<rowsToUseDic.dim()-1; ++ivar){
                        real v=Rlda.ldaTransformVecs.val(icol,ivar);v=v*v;
                        if(useEValScaling)v*=ev;
                        samplingLdaTransforVecCumulator.val(iRealCol,ivar)+=v;
                    }
                }
            }

        } else {
            Rlda.compute(Mat, rowsToUseDic);
        }


        sStr dstFilePathBuf;
        const char * dstFilePath = 0;

        sMatrix::MatrixDicHeaders mh;
        mh.cols=&colIDs;mh.rows=&colIDs;

        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Vectors-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            ldaTransformVecs->out(&ot, &mh, false, true);
        }

        sDic <idx> combinations;


        dstFilePathBuf.cut(0);
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Contributors-%" DEC ".csv", c+1);
        sStr ttt;
        {
            sFil ot(dstFilePath);
            ot.printf("#LDA,StdDev,StdDev%%,\"Contributions (%%weight, peak, ... ) in order of importance\"\n");
            for(idx ic=0; ic<ldaTransformVecs->rows(); ++ic) {
                ldaTransformVecs->outSingleEvecSrt(&ot, ldaTransformVals->ptr(0), ic, &colIDs,&ttt);
                *combinations.set(ttt)=ic;
            }
        }

        sMatrix::MatrixDicHeaders ho;
        ho.cols=&colIDs;
        ho.rows=&rowIDs;

        dstFilePathBuf.cut(0);
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Source-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            Mat.out(&ot, &ho, false, true, "%lf",0,0, 0, 0, 0);
        }

        sMatrix translatedCoordinates;
        translatedCoordinates.multiplyMatrixes(Mat,*ldaTransformVecs);
        dstFilePathBuf.cut(0);
        ho.cols=&combinations;
        dstFilePath = _ctx.qproc().reqAddFile(dstFilePathBuf, "Translated-%" DEC ".csv", c+1);
        {
            sFil ot(dstFilePath);
            translatedCoordinates.out(&ot, &ho, false, true, "%lf",0,0, 0, 0, 0);
        }


        if (quit==1) break;
    }

    return true;
}

