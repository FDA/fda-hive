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
        class PearsonCorrCommand : public Command
        {
            private:
                sVec <idx> colSet1, colSet2;
                bool doTranspose;
                bool cellHasStdev;
                bool zeroForColSet1;
                bool zeroForColSet2;

            public:
                PearsonCorrCommand(ExecContext & ctx) : Command(ctx)
                {
                    cellHasStdev = false;
                    doTranspose = zeroForColSet1 = zeroForColSet2 =false;
                }

                const char * getName() { return "pearsonCorr"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdPearsonCorrFactory(ExecContext & ctx) { return new PearsonCorrCommand(ctx); }
    };
};

struct matchInfo
{
        idx xCol, yCol;
        real value;
        real coef_confidence;
        real slope;
        real intercept;
        matchInfo() {
            intercept = slope = coef_confidence = 0;
            xCol = yCol = -2;
            value = 0;
        }
};

static idx mySortCallback (void * param, void * arr, idx i1, idx i2 )
{
    matchInfo * tmp = ( matchInfo *) arr;

    if (fabs(tmp[i1].value) < fabs(tmp [i2].value))
        return 1;
    else if (fabs(tmp[i1].value) == fabs(tmp [i2].value))
        return 0;

    return -1;
}

bool PearsonCorrCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetVal = arg->getDicElt("colSet1"))
    {
        if (colSetVal->isList())
        {
            const char * p=colSetVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(colSet1),0ll,0ll,0ll);
        }
    }

    if (sVariant * colSetVal = arg->getDicElt("colSet2"))
    {
        if (colSetVal->isList())
        {
            const char * p=colSetVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(colSet2),0ll,0ll,0ll);
        }
    }
    if (arg->getDicElt("doTranspose") != 0){
         doTranspose =  arg->getDicElt("doTranspose")->asBool();
    }

    if (arg->getDicElt("cellHasStdev") != 0){
         cellHasStdev =  arg->getDicElt("cellHasStdev")->asBool();
    }

    if (arg->getDicElt("imputeZeroForEmptyCellColSet1") != 0){
         zeroForColSet1 =  arg->getDicElt("imputeZeroForEmptyCellColSet1")->asBool();
    }

    if (arg->getDicElt("imputeZeroForEmptyCellColSet2") != 0){
         zeroForColSet2 =  arg->getDicElt("imputeZeroForEmptyCellColSet2")->asBool();
    }


    return true;
}

static real getValue(sStr & cell, bool stdev=false) {
    real rval;
    if (!cell.length()) {
        return sNotIdx;
    }
    if (stdev) {
        const char * nxt=strpbrk(cell.ptr(0),"+/-");
        if(!nxt || *nxt==' '){
            return sNotIdx;
        }
        sscanf(nxt+3,"%lf",&rval);
        return rval;
    }
    else {
        sscanf(cell.ptr(0),"%lf",&rval);
        return rval;
    }

}

static void calculateArrayOfAverage(sReorderedTabular & reordered_table, sVec <idx> & colSet, sVec <real> & averages, bool imputeZero) {
    sStr cellValue;
    for (idx c = 0; c < colSet.dim(); c++)
    {
        idx col = colSet[c];
        real total = 0;

        idx rCount=0;
        for (idx r = 0; r < reordered_table.rows(); r++) {
            if(reordered_table.missing(r, col)){
                if (imputeZero) {
                    ++rCount;
                }
                continue;
            }
            cellValue.cut(0);
            reordered_table.printCell(cellValue,r,col);
            real val = getValue(cellValue);
            total += val;

            ++rCount;
        }
        averages [c] = rCount ? total/rCount : 0 ;
    }
}


static void getListOfColumns(sVec < idx > & colSet, sReorderedTabular & reordered_table) {

    idx * ip = colSet.add(reordered_table.cols()-1);
    for(idx ic=1; ic<reordered_table.cols(); ++ic){
        ip[ic-1]=ic;
    }
}


static void performPearsonCorrCalculation (sReorderedTabular & reordered_table, sVec <idx> & colSet1, sVec <idx> & colSet2, sVec <real> & averages1, sVec <real> & averages2,  sVec <matchInfo> & allPairs, sTxtTbl * matrixTbl,  sFil & finalMatrix, bool hasStdev=false) {
    sStr tmp, cellValue;
    for (idx x = 0; x < colSet1.dim(); x++)
       {
           real maxVal = -sIdxMax;
           idx xCol = colSet1[x];

           tmp.cut(0);reordered_table.printCell (tmp, -1, xCol);
           finalMatrix.printf ("\"%s\"", tmp.ptr());

           for (idx y = 0; y < colSet2.dim(); y++)
           {
               idx yCol = colSet2[y];

               if (xCol == yCol)
               {
                   finalMatrix.printf(",1");
                   continue;
               }

               real totalSum = 0, totalXSum = 0, totalYSum = 0;
               real xMinLocal = REAL_MAX;
               real xMaxLocal = REAL_MIN;
               real stdevY_xMinLocal=sNotIdx, stdevY_xMaxLocal=sNotIdx;
               idx elementCnt = 0;
               for (idx r = 0; r < reordered_table.rows(); r++)
               {
                   if ( reordered_table.missing(r, xCol) || reordered_table.missing(r, yCol) )
                       continue;

                   cellValue.cut(0);
                   reordered_table.printCell(cellValue,r,xCol);
                   real xLocal = getValue(cellValue);

                   cellValue.cut(0);
                   reordered_table.printCell(cellValue,r,yCol);
                   real yLocal = getValue(cellValue);
                   real stdevY = (hasStdev) ? ( getValue(cellValue,true) ) : ( sNotIdx );

                   totalSum += ( xLocal - averages1[x] ) * ( yLocal - averages2[y]);
                   totalXSum += ( xLocal - averages1[x] ) * ( xLocal - averages1[x] );
                   totalYSum += ( yLocal - averages2[y] ) * ( yLocal - averages2[y] );

                   if (xLocal < xMinLocal) {
                       xMinLocal = xLocal;
                       stdevY_xMinLocal = stdevY;
                   }
                   if (xLocal > xMaxLocal) {
                       xMaxLocal = xLocal;
                       stdevY_xMaxLocal = stdevY;
                   }
                   elementCnt++;
               }

               real pearsonCorrCoef;

               if (totalXSum != 0 && totalYSum != 0){
                   pearsonCorrCoef = totalSum / ( sqrt(totalXSum) * sqrt(totalYSum) );
               }else
                   pearsonCorrCoef = 0;

               if (pearsonCorrCoef > maxVal)
               {
                   maxVal = pearsonCorrCoef;
               }

               finalMatrix.printf (",%g", pearsonCorrCoef);
               sVariant toPutVar;
               toPutVar.setReal(pearsonCorrCoef);
               matrixTbl->addCell(toPutVar);

               real slope =  totalXSum ? ( totalSum/totalXSum ) : 0 ;
               real intercept = averages2[y] - slope * averages1[x];

               real predictedMinY = slope * xMinLocal + intercept;
               real predictedMaxY = slope * xMaxLocal + intercept;
               real confidence = sNotIdx;
               if (hasStdev || stdevY_xMinLocal != sNotIdx) {
                   confidence = fabs(predictedMaxY - predictedMinY) / (2 * ( stdevY_xMinLocal + stdevY_xMaxLocal ) );
               }

               if ((x >= y && colSet1.dim() >= colSet2.dim()) || (x <= y && colSet1.dim() < colSet2.dim()))
               {
                   matchInfo info;
                   info.xCol = xCol;
                   info.yCol = yCol;
                   info.value = pearsonCorrCoef;
                   info.coef_confidence = (confidence!=sNotIdx) ? confidence : sNotIdx;
                   info.slope = slope;
                   info.intercept = intercept;
                   allPairs.add(1);
                   allPairs[allPairs.dim()-1] = info;
               }
           }

           matrixTbl->addEndRow();
           finalMatrix.printf("\n");
       }
}



bool PearsonCorrCommand::compute(sTabular * tbl)
{
    if ((!colSet1 && colSet1.dim() < 1) && (!colSet2 && colSet2.dim() < 1)) {
        return tbl;
    }
    sReorderedTabular reordered_table (tbl, false);
    if (doTranspose){
        reordered_table.setTransposed(true,1,1);
    }

    if (!colSet1 && colSet1.dim() < 1)
    {
        getListOfColumns(colSet1, reordered_table);
    }

    if (!colSet2 && colSet2.dim() < 1)
    {
        getListOfColumns(colSet2, reordered_table);
    }

    sStr outputPath;
    _ctx.qproc().reqSetData(_ctx.outReqID(), "file://matrix.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(), "matrix.csv",&outputPath);

    sFile::remove(outputPath);

    sFil finalMatrix(outputPath);
    finalMatrix.printf(",");

    sTxtTbl * matrixTbl = new sTxtTbl();
    matrixTbl->initWritable(colSet1.dim(), sTblIndex::fTopHeader | sTblIndex::fSaveRowEnds);

    sVec <idx> heatmapColSet, heatmapRowSet;

    char p[24], r[24];
    sprintf(p, "0-%" DEC, colSet1.dim() - 1);
    sString::scanRangeSet(p, 0, &(heatmapColSet), 0ll, 0ll, 0ll);
    sprintf(r, "0-%" DEC, colSet2.dim() - 1);
    sString::scanRangeSet(r, 0, &(heatmapRowSet), 0ll, 0ll, 0ll);

    outputPath.cut(0);
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://bestMatch.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(), "bestMatch.csv",&outputPath);

    sFile::remove(outputPath);

    sVec <real> averages1, averages2;
    averages1.add(colSet1.dim());
    averages2.add(colSet2.dim());

    sVec <matchInfo> allPairs;



    sStr cellValue, tmp;

    calculateArrayOfAverage(reordered_table, colSet1, averages1, zeroForColSet1);

    for (idx c = 0; c < colSet2.dim(); c++)
    {
        idx col = colSet2[c];
        real total = 0;
        idx rCount=0;
        for (idx r = 0; r < reordered_table.rows(); r++) {
            if(reordered_table.missing(r, col))
                continue;
            cellValue.cut(0);
            reordered_table.printCell(cellValue,r,col);
            real val = getValue(cellValue);
            total += val;
            ++rCount;
        }
        tmp.cut(0);
        reordered_table.printCell(tmp, -1, col);
        if (c)finalMatrix.printf (",");
        finalMatrix.printf ("\"%s\"",tmp.ptr(0));

        averages2 [c] = rCount ? total/rCount : 0 ;
    }
    finalMatrix.printf("\n");


    performPearsonCorrCalculation (reordered_table, colSet1, colSet2, averages1, averages2, allPairs, matrixTbl, finalMatrix, cellHasStdev);


    sVec <idx> indexes;
    indexes.add(allPairs.dim());
    sSort::sortSimpleCallback(mySortCallback, 0, allPairs.dim(), allPairs.ptr(), indexes.ptr());

    sFil bestMatch(outputPath);
    if (cellHasStdev) {
        bestMatch.printf ("Measurement 1,Measurement 2,Correlation,Slope,Intercept,Correlation Confidence\n");
    }
    else bestMatch.printf ("Measurement 1,Measurement 2,Correlation,Slope,Intercept\n");

    for (idx i = 0; i < indexes.dim(); i++)
    {
        matchInfo info = allPairs [indexes[i]];
        sStr col1, col2;
        reordered_table.printCell(col1, -1, info.xCol);
        reordered_table.printCell(col2, -1, info.yCol);
        if (cellHasStdev) {
            bestMatch.printf ("\"%s\",\"%s\",%.3lf,%.3lf,%.3lf,%.3lf\n", col1.ptr(), col2.ptr(), info.value, info.slope, info.intercept,info.coef_confidence);
        }
        else bestMatch.printf ("\"%s\",\"%s\",%.3lf,%.3lf,%.3lf\n", col1.ptr(), col2.ptr(), info.value, info.slope, info.intercept);
    }

    sVec < sVec< real > > vals;
    sHeatmap::generateRangeMap( &vals, matrixTbl, &heatmapRowSet, &heatmapColSet, 0);

    sStr t3;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://pearsonHeatmap.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(),"pearsonHeatmap.csv",&t3);
    sFile::remove(t3);
    sFil heatmapCSV(t3);

    heatmapCSV.printf("%s","rows");
    for( idx ic = 0; ic < colSet2.dim(); ++ic)
    {
        heatmapCSV.printf(",");
        reordered_table.printTopHeader(heatmapCSV, colSet2[ic]);
    }

    heatmapCSV.printf("\n");

    for( idx ir = 0; ir < colSet1.dim(); ++ir)
    {

        reordered_table.printTopHeader(heatmapCSV, colSet1[ir]);
        for( idx ic = 0; ic < colSet2.dim(); ++ic)
        {

            if (vals[ir][ic] == -1)
                heatmapCSV.printf(",");
            else
                heatmapCSV.printf(",%f",vals[ir][ic]);
        }
        heatmapCSV.printf("\n");
    }
    return tbl;
}
