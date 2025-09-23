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
#include <slib/utils/som.hpp>

void sSom::som (real * tbl, idx rowCnt, idx colCnt, idx mapSize, sVec<pntclr> * clr, idx * destGeom)
{
    sVec < idx > allPoints;

    idx primeNumberStep = 429101923;
    idx curRandom = 4293027 / 2;
    idx ceiling = 429304949;

    idx iteration = 0;
    idx adjFact = 0;
    idx numberOfInputs = mapSize;
    real continueTill = curRandom%250;



    sIndex < IJO >  allOffsets;


    while (adjFact < continueTill)
    {
        curRandom += primeNumberStep;

        if (curRandom > ceiling)
            curRandom %= ceiling;

        idx rowToTake = curRandom%rowCnt;

        idx dist;
        pnt index, winner;
        idx indexOffset, winnerOffset = 0;
        real minDist = sIdxMax;

        for (idx i=0; i < allOffsets.dim(); i++)
        {
            index = allOffsets[i].point;
            indexOffset = allOffsets[i].offsetMyData;

            if (index.c == -1 || index.r == -1 || indexOffset == -1)
                continue;

            dist = 0;

            for (idx k = indexOffset; k < colCnt; k++)
                dist += (allPoints[k]-tbl[rowToTake*colCnt+i])*(allPoints[k]-tbl[rowToTake*colCnt+i]);

            dist = sqrt(dist);

            if (dist < minDist)
            {
                winner = index;
                minDist = dist;
                winnerOffset=indexOffset;
            }
            else if (dist == minDist)
            {
                curRandom += primeNumberStep;

                if (curRandom > ceiling)
                    curRandom %= ceiling;

                idx coin = curRandom%2;

                if (coin == 1)
                {
                    winner = index;
                    minDist = dist;
                    winnerOffset=indexOffset;
                }
            }
        }

        curRandom += primeNumberStep;
        if (curRandom > ceiling)
            curRandom %= ceiling;
        index.r = curRandom % mapSize;

        curRandom += primeNumberStep;
        if (curRandom > ceiling)
            curRandom %= ceiling;
        index.c = curRandom % mapSize;

        idx toFind = ((index.r<<32) | index.c);
        idx found = allOffsets.find(toFind);
        IJO ijo;
        idx i;
        idx offset = 0;
        pnt original;
        original.r = index.r;
        original.c = index.c;

        if (found)
        {
            do
            {
                index.c ++;
                if (index.c >= mapSize)
                {
                    index.r++;
                    index.c=0;
                }
                if (index.r >= mapSize)
                    index.r=0;

                toFind = ((index.r<<32) | index.c);
                found = allOffsets.find(toFind);
            }while (found && !(original.r != index.r && original.c != index.c));
        }

        if (!found)
        {
            ijo.defint = ((index.r<<32) | index.c);
            ijo.point=index;
            ijo.offsetMyData = allPoints.dim();
            offset = ijo.offsetMyData;

            allOffsets.add(((index.r<<32) | index.c), &ijo);
            allPoints.add(colCnt+3);
            for (i=offset; i < offset + colCnt; i++)
            {
                curRandom += primeNumberStep;
                if (curRandom > ceiling)
                    curRandom %= ceiling;
                allPoints[i] = curRandom;
            }
            curRandom += primeNumberStep;
            if (curRandom > ceiling)
                curRandom %= ceiling;
            allPoints[i] = curRandom%256;

            curRandom += primeNumberStep;
            if (curRandom > ceiling)
                curRandom %= ceiling;
            allPoints[i+1] = curRandom%256;

            curRandom += primeNumberStep;
            if (curRandom > ceiling)
                curRandom %= ceiling;
            allPoints[i+2] = curRandom%256;

            idx minColor = -1;
            if (allPoints[i] <= allPoints[i+1] && allPoints[i] <= allPoints[i+2])
                minColor = allPoints[i];
            else if (allPoints[i+1] <= allPoints[i] && allPoints[i+1] <= allPoints[i+2])
                minColor = allPoints[i+1];
            else
                minColor = allPoints[i+2];
            allPoints[minColor] = 0;

            dist = 0;

            for (i = 0; i < colCnt; i++)
                dist += (allPoints[i + offset]-tbl[rowToTake+i])*(allPoints[i + offset]-tbl[rowToTake+i]);

            dist = sqrt(dist);

            if (dist < minDist)
            {
                winner = index;
                minDist = dist;
                winnerOffset=offset;
            }
            else if (dist == minDist)
            {
                curRandom += primeNumberStep;
                if (curRandom > ceiling)
                    curRandom %= ceiling;
                idx coin = curRandom%2;

                if (coin == 1)
                {
                    winner = index;
                    minDist = dist;
                    winnerOffset=offset;
                }
            }
        }

        destGeom[rowToTake*2] = winner.r;
        destGeom[rowToTake*2+1] = winner.c;

        real adj1 = adjFact/1000;
        if (adj1 >= 1)
            adj1=0.999;
        real adj2 = 100 - adjFact/100;
        if (adj2 <= 0)
            adj2=0.01;

        real threshold = (1.0 - adj1) * exp((winner.r * winner.r  + winner.c * winner.c) / adj2);
        real radius = 2*sqrt (-log(threshold/(1-adj1)));

        if (-log(threshold/(1-adj1)) <=  0)
            radius = mapSize/sqrt(mapSize);


        idx startr = sMax<idx>((idx)(winner.r - radius), 0);
        idx endr = sMin<idx>((idx)(winner.r + radius), mapSize);

        idx startc = sMax<idx>((idx)(winner.c - radius), 0);
        idx endc = sMin<idx>((idx)(winner.c + radius), mapSize);

        for (idx r = startr; r < endr; r++)
        {
            for (idx c=startc; c < endc; c++)
            {
                if (sqrt((r-winner.r)*(r-winner.r) + (c-winner.c)*(c-winner.c)) > radius)
                    continue;

                real gaussian = (1.0 - adj1) * exp(-sqrt((winner.r-r) * (winner.r-r)  + (winner.c-c)*(winner.c-c)) / adj2);
                real diff = 0;
                idx found = allOffsets.find(((r<<32) | c));

                if (!found)
                {
                    IJO ijo;
                    ijo.defint = ((r<<32) | c);
                    ijo.point.r=r;
                    ijo.point.c=c;
                    ijo.offsetMyData = allPoints.dim();
                    offset = ijo.offsetMyData;

                    allOffsets.add(((r<<32) | c), &ijo);
                    allPoints.add(colCnt+3);

                    for (i=ijo.offsetMyData; i < ijo.offsetMyData + colCnt; i++)
                    {
                        curRandom += primeNumberStep;
                        if (curRandom > ceiling)
                            curRandom %= ceiling;
                        allPoints[i] = curRandom;
                    }
                    curRandom += primeNumberStep;
                    if (curRandom > ceiling)
                        curRandom %= ceiling;
                    allPoints[i] = curRandom%256;

                    curRandom += primeNumberStep;
                    if (curRandom > ceiling)
                        curRandom %= ceiling;
                    allPoints[i+1] = curRandom%256;

                    curRandom += primeNumberStep;
                    if (curRandom > ceiling)
                        curRandom %= ceiling;
                    allPoints[i+2] = curRandom%256;

                    idx minColor = -1;
                    if (allPoints[i] <= allPoints[i+1] && allPoints[i] <= allPoints[i+2])
                        minColor = allPoints[i];
                    else if (allPoints[i+1] <= allPoints[i] && allPoints[i+1] <= allPoints[i+2])
                        minColor = allPoints[i+1];
                    else
                        minColor = allPoints[i+2];
                    allPoints[minColor] = 0;
                }

                for (i = 0; i < colCnt+3; i++)
                {
                    diff = allPoints[winnerOffset+i] -  allPoints[offset+i];
                    allPoints[offset+i] += (gaussian * diff);

                    if (i >= colCnt)
                        allPoints[offset+i] %= 256;
                }
            }
        }


        iteration++;

        if (iteration == numberOfInputs)
        {
            iteration = 0;
            adjFact++;
        }
    }

    idx i;

    clr->add(allOffsets.dim());

    for (i = 0; i < allOffsets.dim(); i++)
    {
        IJO ijo=allOffsets[i];
        pntclr toAdd;
        (*clr)[i].point = ijo.point;
        (*clr)[i].color.set(allPoints [ijo.offsetMyData + colCnt], allPoints [ijo.offsetMyData + colCnt+1], allPoints [ijo.offsetMyData + colCnt+2]);
    }
}


