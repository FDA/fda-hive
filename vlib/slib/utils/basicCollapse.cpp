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
#include <slib/utils/basicCollapse.hpp>

void sBasicCollapse::collapse(sTabular * tbl,  sVec <idx > colsToUse, sVariantTbl * outTbl)
{
    sVec <idx> colsActual; //the are the columns on which the statistics will be accumulated

    for (idx i = 0; i < tbl->cols(); i++)
    {
        idx contains = -1;

        for (idx k = 0; k < colsToUse.dim(); k++)
        {
            if (colsToUse[k] == i)
            {
                contains = k;
                break;
            }
        }

        if (contains > -1)
            continue;

        colsActual.add(1);
        colsActual[colsActual.dim()-1] = i;
    }

    idx colStep = colsToUse.dim();
    idx colPos = 0; //current position in the column vector
    idx col = colsToUse[0]; //the actual column from the column vector

    idx rowInTbl = 1; //the current row in the tab;e
    idx startRow = 0; //the starting row in the table (this will be used for calculating the number of elements for the statistics)

    sVec <idx> endingRow;
    sVec <idx> startingRow;
    endingRow.add(colStep);
    startingRow.add(colStep);
    endingRow[0] = 0;

    while (colPos < colsToUse.dim() && endingRow[endingRow.dim()-1] < tbl->rows())
    {
        sVariant curElement;
        tbl->val(curElement, startRow, col, true); //current element (this will change with every new element discovered in the collapsing rows
        startingRow[colPos] = startRow;

        sVec<real> median; //all of the median value according to the values in the curElement
        sVec<real> sumOfSquares; //will be used for the standard deviation
        median.add(colsActual.dim());
        sumOfSquares.add(colsActual.dim());

        //initializing all of the values in the vectors for proper calculations
        for (idx i = 0; i < colsActual.dim(); i++)
        {
            sVariant cur;
            tbl->val(cur, startRow, colsActual[i], true);
            if (cur.isNumeric())
            {
                median[i]=cur.asReal();
                sumOfSquares[i] = cur.asReal()*cur.asReal();
            }
            else
            {
                median[i]=-sIdxMax;
                sumOfSquares[i] = -sIdxMax;
            }
        }

        idx end = tbl->rows();
        if (colPos > 0)
            end = endingRow[colPos-1];

        while (rowInTbl < end)
        {
            sVariant next;
            tbl->val(next, rowInTbl, col, true);

            if (next != curElement)
                break;

            for (idx i = 0; i < colsActual.dim(); i++)
            {
                sVariant cur;
                tbl->val(cur, rowInTbl, colsActual[i]);

                if (median[i] == -sIdxMax)
                    continue;

                if (cur.isNumeric())
                {
                    median[i] += cur.asReal();
                    sumOfSquares[i] += cur.asReal()*cur.asReal();
                }
                else
                {
                    median[i]=-sIdxMax;
                    sumOfSquares[i] = -sIdxMax;
                }
            }

            rowInTbl++;
        }
        endingRow[colPos] = rowInTbl;

        for (idx i = startRow; i < rowInTbl; i++)
        {
            for (idx k = 0; k < colsActual.dim(); k++)
            {
                sStr toPut;
                real curMedian = median[k]/(rowInTbl-startRow);
                real std = sqrt(sumOfSquares[k]/(rowInTbl-startRow) - curMedian*curMedian);
                // \302\261 is octal code for UTF8 encoding of plus/minus
                toPut.printf("%g\302\261%g", curMedian, std);
                outTbl->setVal(i, colStep + k*colStep + colPos, toPut.ptr());
            }

        }

        colPos++;

        if (colPos < endingRow.dim())
        {
            col = colsToUse[colPos];
            startRow = startingRow [colPos-1];
            rowInTbl = startRow+1;
        }
        else
        {
            idx x;
            for (x = 1; x < endingRow.dim(); x++)
            {
                if (endingRow[colPos-x] < endingRow[colPos-x-1])
                    break;
            }

            startRow = endingRow[colPos-x];
            rowInTbl = startRow + 1;
            colPos -= x;
            col = colsToUse [colPos];

        }
    }

}
