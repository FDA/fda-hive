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

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class DataCalibCommand : public Command
        {
            private:
                idx translate,redundencyWay,sigmaOutliers,calibration,normalize;
                sVec <idx> normalizeCols;
                sVec <idx> categoryCols;
                sVec <idx> calibrationCols;

                bool contains (idx num, sVec <idx> arr);

            public:
                DataCalibCommand(ExecContext & ctx) : Command(ctx)
                {
                    translate = 0;
                    redundencyWay = 0;
                    sigmaOutliers = 0;
                    calibration = 0;
                    normalize = 0;
                }

                const char * getName() { return "dataCalib"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdDataCalibFactory(ExecContext & ctx) { return new DataCalibCommand(ctx); }
    };
};

bool DataCalibCommand::init(const char * op_name, sVariant * arg)
{
    if (arg->getDicElt("translate") != 0)
        translate = arg->getDicElt("translate")->asInt();

    if (arg->getDicElt("redundencyWay") != 0)
        redundencyWay = arg->getDicElt("redundencyWay")->asInt();

    if (arg->getDicElt("sigmaOutliers") != 0)
        sigmaOutliers = arg->getDicElt("sigmaOutliers")->asInt();

    if (arg->getDicElt("normalize") != 0)
        normalize = arg->getDicElt("normalize")->asInt();


    if (sVariant * normalizeColsVal = arg->getDicElt("normalizeCols"))
    {
        if (normalizeColsVal->isList())
        {
            const char * p=normalizeColsVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(normalizeCols),0ll,0ll,0ll);
        }
        else
            return false;
    }

    if (sVariant * calibrationColsVal = arg->getDicElt("calibrationCols"))
    {
        if (calibrationColsVal->isList())
        {
            const char * p=calibrationColsVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(calibrationCols),0ll,0ll,0ll);
        }
        else
            return false;
    }

    if (sVariant * categoryColsVal = arg->getDicElt("categoryCols"))
    {
        if (categoryColsVal->isList())
        {
            const char * p=categoryColsVal->asString();if(*p=='[')++p;
            sString::scanRangeSet(p,0,&(categoryCols),0ll,0ll,0ll);
        }
        else
            return false;
    }

    return true;
}

bool DataCalibCommand::contains (idx num, sVec <idx> arr){
    for (idx i = 0; i < arr.dim(); i++){
        if (arr[i]==num) return true;
    }
    return false;
}

bool DataCalibCommand::compute(sTabular * tbl)
{
    sDic < sVec <idx> > mappingRowsCols;
    sDic < sVec <idx> > mappingCatDic;

    if (translate > 0)
    {
        idx pos = -1;
        if (tbl->dimLeftHeader() == 0)
            pos = 0;

        for (idx i = -1; i < tbl->rows(); i++)
        {
            sStr header;
            tbl->printCell(header,i,pos);

            idx size = mappingRowsCols[header.ptr()].dim();
            mappingRowsCols[header.ptr()].add(1);
            mappingRowsCols[header.ptr()][size] = i;
        }
    }
    else if (tbl->dimTopHeader() > 0)
    {
        for (idx i = 0; i < tbl->cols(); i++)
        {
            sStr header;
            tbl->printCell(header,-1,i);

            idx size = mappingRowsCols[header.ptr()].dim();
            mappingRowsCols[header.ptr()].add(1);
            mappingRowsCols[header.ptr()][size] = i;
        }
    }

    if (categoryCols.dim() > 0 && !translate){
        if (!calibrationCols.dim()){
            sStr p;
            p.printf("0-%" DEC, tbl->cols());
            sString::scanRangeSet(p.ptr(),0,&(calibrationCols),0ll,0ll,0ll);
        }
        for (idx r = 0; r < tbl->rows(); r++){
            sStr concatCells;
            for (idx i = 0; i < categoryCols.dim(); i++){
                idx realC = categoryCols[i];
                sStr cell;

                tbl->printCell(cell, r, realC);
                concatCells.printf("%s:", cell.ptr());
            }

            idx size = mappingCatDic[concatCells.ptr()].dim();
            mappingCatDic[concatCells.ptr()].add(1);
            mappingCatDic[concatCells.ptr()][size] = r;
        }
    }else if (categoryCols.dim() > 0){
        if (!calibrationCols.dim()){
            sStr p;
            p.printf("0-%" DEC, tbl->cols());
            sString::scanRangeSet(p.ptr(),0,&(calibrationCols),0ll,0ll,0ll);
        }
        for (idx r = 0; r < tbl->cols(); r++){
            sStr concatCells;
            for (idx i = 0; i < categoryCols.dim(); i++){
                idx realC = categoryCols[i];
                sStr cell;

                tbl->printCell(cell, realC, r);
                concatCells.printf("%s:", cell.ptr());
            }

            idx size = mappingCatDic[concatCells.ptr()].dim();
            mappingCatDic[concatCells.ptr()].add(1);
            mappingCatDic[concatCells.ptr()][size] = r;
        }
    }

    sVariantTbl * outTabular = new sVariantTbl (mappingRowsCols.dim(), 0);
    idx rowLength = translate ? tbl->cols()-1 : tbl->rows();

    if (categoryCols.dim() > 0){
        sDic < sVec <real> > values;

        for (idx x = -1; x < tbl->cols(); x++){
            sStr cell;
            if (x < 0)
                tbl->printTopHeader(cell, x);
            else
                tbl->printCell(cell, -1, x);

            outTabular->setVal (-1, x, cell.ptr());
        }


        for (idx x = 0; x < tbl->cols(); x++){
            if (!DataCalibCommand::contains (x, calibrationCols)){
                for (idx r = 0; r < tbl->rows(); r++){
                    sStr cell;
                    tbl->printCell(cell, r, x);
                    outTabular->setVal (r, x, cell.ptr());
                }
                continue;
            }
            idx c = x;
            values.empty();

            for (idx r = 0; r < tbl->rows(); r++){
                sStr concatCells;
                for (idx i = 0; i < categoryCols.dim(); i++){
                    idx realC = categoryCols[i];
                    sStr cell;

                    if (!translate)
                        tbl->printCell(cell, r, realC);
                    else
                        tbl->printCell(cell, realC, r);

                    concatCells.printf("%s:", cell.ptr());
                }

                if (!concatCells.ptr() || tbl->missing(r,c)) continue;

                sVariant variant;
                tbl->val (variant, r, c, 1);

                idx size = values[concatCells.ptr()].dim();
                values[concatCells.ptr()].add(1);
                values[concatCells.ptr()][size] = variant.asReal();
            }
            for (idx r = 0; r < tbl->rows(); r++){
                sStr concatCells;
                for (idx i = 0; i < categoryCols.dim(); i++){
                    idx realC = categoryCols[i];
                    if (realC == c){
                        continue;
                    }

                    sStr cell;

                    if (!translate)
                        tbl->printCell(cell, r, realC);
                    else
                        tbl->printCell(cell, realC, r);

                    concatCells.printf("%s:", cell.ptr());
                }

                if (!concatCells.ptr()) continue;

                sVec <real> valsForCurKey = values[concatCells.ptr()];
                sVariant variant;
                tbl->val (variant, r, c, 1);

                real actualVal = variant.asReal();
                real average = 0, variance = 0;

                for (idx i = 0; i < valsForCurKey.dim(); i++){
                    average += valsForCurKey[i];
                    variance += valsForCurKey[i] * valsForCurKey[i];
                }

                real sigma = sqrt (variance);
                average /= valsForCurKey.dim();

                if (abs(actualVal) > abs(average - 2*sigma) || tbl->missing(r,c)){
                    outTabular->setVal (r, c, average);
                }else{
                    outTabular->setVal (r, c, actualVal);
                }
            }

        }
        setOutTable(outTabular);
        return true;
    }

    if (redundencyWay != 0)
    {
        for (idx i = 0; i < mappingRowsCols.dim(); i++)
        {
            const char * header = static_cast <const char *> (mappingRowsCols.id(i));
            sVec <idx> curVec = mappingRowsCols[header];

            //if going over the average over the multiple rows
            for (idx r = -1; r < rowLength; r++)
            {

                if (curVec.dim() == 1)
                {
                    sVariant tmp;

                    if (translate)
                        tbl->val (tmp, curVec[0], r+1, 1);
                    else
                        tbl->val (tmp, r, curVec[0], 1);

                    outTabular->setVal (r, i, tmp);

                    continue;
                }

                real average = 0;
                sVec <real> vecForMedian;
                real variance = 0;
                real median;

                for (idx x = 0; x < curVec.dim(); x++)
                {

                    real val;
                    sVariant variant;
                    idx colType;

                    if (translate)
                    {
                        tbl->val (variant, curVec[x], r+1, 1);
                        colType = tbl->coltype(r+1);
                    }
                    else
                    {
                        tbl->val (variant, r, curVec[x], 1);
                        colType = tbl->coltype(curVec[x]);
                    }

                    if (r < 0 || (colType != sVariant::value_INT && colType != sVariant::value_REAL && colType != sVariant::value_UINT))
                    {
                        outTabular->setVal(r,i, variant);
                        continue;
                    }

                    val = variant.asReal();

                    if (redundencyWay == 1)
                        average += val;
                    else
                    {
                        vecForMedian.add(1);
                        vecForMedian[vecForMedian.dim() - 1] = val;
                    }
                    variance += val*val;
                }

                if (sigmaOutliers == 0 && redundencyWay == 1 && average != 0)
                    outTabular->setVal(r,i, average/curVec.dim());
                else if (sigmaOutliers == 0 && redundencyWay == 2 && vecForMedian.dim() > 0)
                {
                    sSort::sort(vecForMedian.dim(), vecForMedian.ptr());

                    if (curVec.dim() %2 != 0)
                        median = vecForMedian[curVec.dim() / 2];
                    else
                        median = (vecForMedian[curVec.dim() / 2 ]-vecForMedian[curVec.dim() / 2-1])/2;

                    outTabular->setVal(r,i, median);
                }
                else if (sigmaOutliers != 0)
                {
                    real sigma = sqrt (variance);
                    real nAverage = 0;
                    sVec <real> nVecForMedian;
                    idx total = 0;

                    for (idx y = 0; y < curVec.dim(); y++)
                    {
                        real val;
                        sVariant variant;
                        idx colType;

                        if (translate)
                        {
                            tbl->val (variant, curVec[y], r+1,1);
                            colType = tbl->coltype(r);
                        }
                        else
                        {
                            tbl->val (variant, r, curVec[y], 1);
                            colType = tbl->coltype(curVec[y]);
                        }

                        if (r < 0 || (colType != sVariant::value_INT && colType != sVariant::value_REAL && colType != sVariant::value_UINT))
                        {
                            outTabular->setVal(r,i, variant);
                            continue;
                        }

                        val = variant.asReal();

                        //make sure the sign here is correct
                        if (abs(val - average) >= abs(sigma))
                        {
                            if (redundencyWay == 1)
                                nAverage += val;
                            else
                            {
                                nVecForMedian.add(1);
                                nVecForMedian[nVecForMedian.dim() - 1] = val;
                            }

                            total ++;
                        }
                    }
                    if (redundencyWay == 1 && nAverage != 0)
                        outTabular->setVal( r, i, nAverage/total);
                    else if (redundencyWay == 2 && nVecForMedian.dim() > 0)
                    {
                        sSort::sort(nVecForMedian.dim(), nVecForMedian.ptr());

                        if (curVec.dim() %2 != 0)
                            median = nVecForMedian[curVec.dim() / 2];
                        else
                            median = (nVecForMedian[nVecForMedian.dim() / 2 ]-nVecForMedian[nVecForMedian.dim() / 2 -1])/2;

                        if (median != 0)
                            outTabular->setVal (r,i, median);
                    }
                }
            }
        }
    }
    else
    {
        idx counter = 0;
        for (idx i = 0; i < mappingRowsCols.dim(); i++)
        {
            const char * header = static_cast <const char *> (mappingRowsCols.id(i));
            sVec <idx> curVec = mappingRowsCols[header];

            for (idx x = 0; x < curVec.dim(); x++)
            {
                for (idx r = -1; r < rowLength; r++)
                {
                    sVariant tmp;

                    if (translate)
                    {
                        tbl->val(tmp, curVec[x], r+1,1);

                        idx colType = tbl->coltype(r+1);
                        if ((colType != sVariant::value_INT && colType != sVariant::value_REAL && colType != sVariant::value_UINT))
                        {
                            real possibleVal;

                            if (tmp.asString()[0] == '-')
                                possibleVal= atof(tmp.asString()+1)*-1;
                            else
                                possibleVal = atof(tmp.asString());
                            if (possibleVal != 0.0)
                                tmp.setReal(possibleVal);
                        }

                        outTabular->setVal (r, curVec[x]+1, tmp);
                    }
                    else
                    {
                        tbl->val (tmp, r, curVec[x],1);
                        outTabular->setVal (r, curVec[x], tmp);
                    }
                }

                counter++;
            }
        }
    }

    if (calibrationCols.dim() > 0)
    {
        for (idx r = 0; r < outTabular->rows(); r++)
        {
            real minimum = sIdxMax;
            real average = 0;
            sVec <real> median;

            for (idx i = 0; i < calibrationCols.dim(); i++)
            {
                real val;
                sVariant tmp;

                outTabular->val (tmp, r, calibrationCols[i], 1);
                idx colType =  tmp.getType();

                if ((colType != sVariant::value_INT && colType != sVariant::value_REAL && colType != sVariant::value_UINT))
                    continue;

                val = tmp.asReal();

                if (redundencyWay != 2)
                    average += val;
                else
                {
                    median.add(1);
                    median [median.dim()-1] = val;
                }

                outTabular->setVal(r, calibrationCols[i], "");
            }

            if (redundencyWay != 2 && average != 0)
                minimum = average / calibrationCols.dim();
            else if (redundencyWay == 2 && median.dim() != 0)
            {
                if (median.dim() % 2 != 0)
                    minimum = median[median.dim()/2];
                else
                    minimum = median[median.dim()/2] - median[median.dim()/2-1];
            }

            if (minimum == sIdxMax) continue;

            for (idx c = 1; c < outTabular->cols(); c++)
            {
                real val;
                sVariant tmp;
                idx colType = outTabular->coltype(c);

                outTabular->val (tmp, r, c, 1);
                if (r < 0 || (colType != sVariant::value_INT && colType != sVariant::value_REAL && colType != sVariant::value_UINT))
                   continue;

                val = tmp.asReal();

                outTabular->setVal(r, c, val-minimum);
            }
        }
    }

    if (normalizeCols.dim() > 0)
    {
        if (normalize == 1)
        {
            real sum = 0;

            for (idx c = 0; c < normalizeCols.dim(); c++)
            {
                for (idx r = 0 ; r < outTabular->rows(); r++)
                {
                    real val;
                    sVariant tmp;

                    outTabular->val(tmp, r, normalizeCols[c], 1);
                    val = tmp.asReal();

                    sum += val;
                }

                for (idx r = 0 ; r < outTabular->rows(); r++)
                {
                    real val;
                    sVariant tmp;

                    outTabular->val (tmp, r, normalizeCols[c], 1);
                    val = tmp.asReal();

                    outTabular->setVal(r, normalizeCols[c], val/sum);
                }
            }
        }
        else if (normalize == 2)
        {
            real variance = 0;
            real sigma = 0;

            for (idx c = 0; c < normalizeCols.dim(); c++)
            {
                for (idx r = 0 ; r < outTabular->rows(); r++)
                {
                    real val;
                    sVariant tmp;

                    outTabular->val (tmp, r, normalizeCols[c], 1);
                    val = tmp.asReal();

                    variance += val * val;
                }

                sigma = sqrt(variance);

                for (idx r = 0 ; r < outTabular->rows(); r++)
                {
                    real val;
                    sVariant tmp;

                    outTabular->val (tmp, r, normalizeCols[c], 1);
                    val = tmp.asReal();

                    outTabular->setVal(r, normalizeCols[c], val/sigma);
                }
           }
        }
        else if (normalize == 3)
        {
            real min = sIdxMax;
            real max = -sIdxMax;

            for (idx c = 0; c < normalizeCols.dim(); c++)
            {
                for (idx r = 0 ; r < outTabular->rows(); r++)
                {
                    real val;
                    sVariant tmp;

                    outTabular->val (tmp, r, normalizeCols[c], 1);
                    val = tmp.asReal();

                    if (val < min)
                        min = val;
                    if (val > max)
                        max = val;
                }

                for (idx r = 0 ; r < outTabular->rows(); r++)
                {
                    real val;
                    sVariant tmp;

                    outTabular->val(tmp, r, normalizeCols[c], 1);
                    val = tmp.asReal();

                    outTabular->setVal(r, normalizeCols[c], (val-min)/(max-min));
                }
           }
        }
    }

    setOutTable(outTabular);
    return true;
}
