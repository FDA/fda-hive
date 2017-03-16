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
                bool doTranspose; // transpose rows and columns of original table
                bool cellHasStdev;
                bool zeroForColSet1; // impute Zero For Empty Cell in colSet1
                bool zeroForColSet2; // impute Zero For Empty Cell in colSet2

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
            xCol = yCol = -2; // -2 by default because, the sTxtTbl considers -1 as the column header
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
    // doTranspose
    if (arg->getDicElt("doTranspose") != 0){
         doTranspose =  arg->getDicElt("doTranspose")->asBool();
    }

    //cellHasStdev
    if (arg->getDicElt("cellHasStdev") != 0){
         cellHasStdev =  arg->getDicElt("cellHasStdev")->asBool();
    }

    // impute zero for the empty cell from colSet1
    if (arg->getDicElt("imputeZeroForEmptyCellColSet1") != 0){
         zeroForColSet1 =  arg->getDicElt("imputeZeroForEmptyCellColSet1")->asBool();
    }

    // impute zero for the empty cell from colSet2
    if (arg->getDicElt("imputeZeroForEmptyCellColSet2") != 0){
         zeroForColSet2 =  arg->getDicElt("imputeZeroForEmptyCellColSet2")->asBool();
    }


    return true;
}

//
static real getValue(sStr & cell, bool stdev=false) {
    real rval;
    if (!cell.length()) {
        return sNotIdx;
    }
    if (stdev) {
        const char * nxt=strpbrk(cell.ptr(0),"+/-"); // find the separator
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
        // looping through each row from 1 column
        // => calculate the total value
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
            //total += reordered_table.rval(r,col);
            total += val;

            ++rCount; // increment each count if has value
        }
        averages [c] = rCount ? total/rCount : 0 ; // calculate average based on the total and number of count
    }
}


static void getListOfColumns(sVec < idx > & colSet, sReorderedTabular & reordered_table) {
    // get the list of columns for a colSet

    /// commented by VSim
    ///char p [16];
    ///sprintf (p, "0-%" DEC, tbl->cols()-1);
    ///sString::scanRangeSet(p,0,&(colSet1),0ll,0ll,0ll);
    idx * ip = colSet.add(reordered_table.cols()-1);
    for(idx ic=1; ic<reordered_table.cols(); ++ic){
        ip[ic-1]=ic;
    }
}


static void performPearsonCorrCalculation (sReorderedTabular & reordered_table, sVec <idx> & colSet1, sVec <idx> & colSet2, sVec <real> & averages1, sVec <real> & averages2,  sVec <matchInfo> & allPairs, sTxtTbl * matrixTbl,  sFil & finalMatrix, bool hasStdev=false) {
    sStr tmp, cellValue;
    for (idx x = 0; x < colSet1.dim(); x++)
       {
           //idx maxCol = -1;
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
                   //matrixTbl->setVal(x, y, "");
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

                   //real xLocal =  reordered_table.rval(r, xCol);
                   cellValue.cut(0);
                   reordered_table.printCell(cellValue,r,xCol);
                   real xLocal = getValue(cellValue);

                   //real yLocal = reordered_table.rval(r, yCol);
                   cellValue.cut(0);
                   reordered_table.printCell(cellValue,r,yCol);
                   real yLocal = getValue(cellValue);
                   real stdevY = (hasStdev) ? ( getValue(cellValue,true) ) : ( sNotIdx );

                   totalSum += ( xLocal - averages1[x] ) * ( yLocal - averages2[y]); // Sum[ (Xi - meanX)*(Yi-meanY) ]
                   totalXSum += ( xLocal - averages1[x] ) * ( xLocal - averages1[x] ); // Sum[(Xi-meanX)^2]
                   totalYSum += ( yLocal - averages2[y] ) * ( yLocal - averages2[y] ); // Sum[(Yi-meanY)^2]

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
                   pearsonCorrCoef = totalSum / ( sqrt(totalXSum) * sqrt(totalYSum) ); // pearsonCorr = Sum[ (Xi - meanX)*(Yi-meanY) ] / ( sqrt(Sum[(Xi-meanX)^2]) * sqrt(Sum[(Yi-meanY)^2]))
               }else
                   pearsonCorrCoef = 0; // meaning no Correlation

               if (pearsonCorrCoef > maxVal)
               {
                   maxVal = pearsonCorrCoef;
                   //maxCol = yCol;
               }

               finalMatrix.printf (",%g", pearsonCorrCoef);
               sVariant toPutVar;
               toPutVar.setReal(pearsonCorrCoef);
               matrixTbl->addCell(toPutVar);
               //matrixTbl->setVal(x, y, toPut);

               // --------------------------------------------------------------------
               // regression line: y = slope * x + intercept
               // slope = Sum[ (Xi - meanX)*(Yi-meanY) ] / Sum[(Xi-meanX)^2]
               // intercept = meanY - slope * meanX
               // coefficient confidence = (predicted Y at max X - predicted Y at min X) / 2 (sigma of Y at min X + sigma of Y  at max X)
               // --------------------------------------------------------------------
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

// Main Function

// Plugin: Calculate the pearson correlation between 2 set of columns
// is a measure of the linear correlation between two variables X and Y
// giving a value between +1 and −1 inclusive, where 1 is total positive correlation, 0 is no correlation, and −1 is total negative correlation
// ==> Outputs:
//          - Matrix of top header is the list of column header from set 1 and left header is the list of column header from set 2
//          - Table of correlation values sorted from Highest absolute value
//          - Heatmap table

bool PearsonCorrCommand::compute(sTabular * tbl)
{
    if ((!colSet1 && colSet1.dim() < 1) && (!colSet2 && colSet2.dim() < 1)) {
        return tbl;
    }
    sReorderedTabular reordered_table (tbl, false);
    if (doTranspose){
        //reordered_table.setTransposed(true);
        reordered_table.setTransposed(true,1,1); // when transposed, top header =1 and left header =1
    }

    // When user doesn't specify any columns
    // => take all columns
    // get the list of columns for 1st set
    if (!colSet1 && colSet1.dim() < 1)
    {
        getListOfColumns(colSet1, reordered_table);
    }

    // get the list of columns for 2nd set
    if (!colSet2 && colSet2.dim() < 1)
    {
        getListOfColumns(colSet2, reordered_table);
    }

    // setting one of the output result file to the request id
    sStr outputPath;
    _ctx.qproc().reqSetData(_ctx.outReqID(), "file://matrix.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(), "matrix.csv",&outputPath); // getting the path

    sFile::remove(outputPath); // if for some reasons the file existed, remove it

    sFil finalMatrix(outputPath); // open the file to write
    finalMatrix.printf(",");

    sTxtTbl * matrixTbl = new sTxtTbl();
    matrixTbl->initWritable(colSet1.dim(), sTblIndex::fTopHeader | sTblIndex::fSaveRowEnds);

    sVec <idx> heatmapColSet, heatmapRowSet;

    // preparing the dimension of the heatmap: width, height
    char p [16], r [16];
    sprintf (p, "0-%" DEC, colSet1.dim()-1);
    sString::scanRangeSet(p,0,&(heatmapColSet),0ll,0ll,0ll);
    sprintf (r, "0-%" DEC, colSet2.dim()-1);
    sString::scanRangeSet(r,0,&(heatmapRowSet),0ll,0ll,0ll);

    // Setting another output result file to the request id
    outputPath.cut(0);
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://bestMatch.csv",0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(), "bestMatch.csv",&outputPath);

    sFile::remove(outputPath);

    sVec <real> averages1, averages2;
    averages1.add(colSet1.dim());
    averages2.add(colSet2.dim());

    sVec <matchInfo> allPairs; // use for sorting



    // Looping through each column from 1st column set
    // Goal: calculate the total and average values for each column
    sStr cellValue, tmp;

    calculateArrayOfAverage(reordered_table, colSet1, averages1, zeroForColSet1);

    // Looping through each column from 2nd column set
    // Goal: calculate the total and average values for each column
    for (idx c = 0; c < colSet2.dim(); c++)
    {
        idx col = colSet2[c];
        real total = 0;
        idx rCount=0;
        // looping through each row from 1 column
        // => calculate the total value
        for (idx r = 0; r < reordered_table.rows(); r++) {
            if(reordered_table.missing(r, col))
                continue;
            cellValue.cut(0);
            reordered_table.printCell(cellValue,r,col);
            real val = getValue(cellValue);
            total += val;
//            total += reordered_table.rval(r,col);
            ++rCount;
        }
        tmp.cut(0);
        reordered_table.printCell(tmp, -1, col);
        if (c)finalMatrix.printf (",");
        finalMatrix.printf ("\"%s\"",tmp.ptr(0));

        //averages2 [c] = total/tbl->rows();
        averages2 [c] = rCount ? total/rCount : 0 ;
    }
    finalMatrix.printf("\n");

    /************
     *  Actual calculate the Pearson correlation
     */

    performPearsonCorrCalculation (reordered_table, colSet1, colSet2, averages1, averages2, allPairs, matrixTbl, finalMatrix, cellHasStdev);


    /************
     *  sorting absolute correlation value in Descending Order the table
     */
    sVec <idx> indexes;
    indexes.add(allPairs.dim());
    sSort::sortSimpleCallback(mySortCallback, 0, allPairs.dim(), allPairs.ptr(), indexes.ptr());
    //sSort::sort(vals.dim(), vals.ptr(), indexes.ptr());

    sFil bestMatch(outputPath);
    //bestMatch.printf ("Measurement 1,Measurement 2,Correlation,Correlation Confidence\n"); // header
    if (cellHasStdev) {
        bestMatch.printf ("Measurement 1,Measurement 2,Correlation,Slope,Intercept,Correlation Confidence\n"); // header
    }
    else bestMatch.printf ("Measurement 1,Measurement 2,Correlation,Slope,Intercept\n"); // header

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

    /************
     *  Generating HeatMap
     */
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
