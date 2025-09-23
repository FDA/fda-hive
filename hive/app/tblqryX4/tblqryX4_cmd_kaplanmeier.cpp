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
#include <slib/std/string.hpp>
#include <ssci/math.hpp>
#include "tblqryX4_cmd.hpp"

#include "utils.hpp"
#include "sort.hpp"

#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

#define PRFX "kaplan-meier-"
#define OUTFILE "kaplan-meier.csv"

using namespace slib;
using namespace slib::tblqryx4;

static idx dateDifference(char * inital, char * final);
static sVec < idx > parseDate (char * date);

namespace slib {
    namespace tblqryx4 {
        class KaplanMeierCommand : public Command
        {
            private:
                sVec<idx> colSet;

            public:
                KaplanMeierCommand(ExecContext & ctx) : Command(ctx) {}

                const char * getName() { return "kaplanmeier"; }
                bool computesOutTable() { return false; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdKaplanMeierFactory(ExecContext & ctx) { return new KaplanMeierCommand(ctx); }
    };
};

bool KaplanMeierCommand::init(const char * op_name, sVariant * arg)
{
    if (sVariant * colSetVal = arg->getDicElt("initEvents"))
    {
            if (colSetVal->isList())
            {
                for (idx i = 0; i < colSetVal->dim(); i++)
                    colSet.vadd(1, colSetVal->getListElt(i)->asInt());
            }
            else
                return false;
    }

    if (sVariant * colSetVal = arg->getDicElt("finEvents"))
        {
                if (colSetVal->isList())
                {
                    for (idx i = 0; i < colSetVal->dim(); i++)
                        colSet.vadd(1, colSetVal->getListElt(i)->asInt());
                }
                else
                    return false;
        }

    if (sVariant * colSetVal = arg->getDicElt("cenEvents"))
        {
                if (colSetVal->isList())
                {
                    for (idx i = 0; i < colSetVal->dim(); i++)
                        colSet.vadd(1, colSetVal->getListElt(i)->asInt());
                }
                else
                    return false;
        }

    return true;
}



bool KaplanMeierCommand::compute(sTabular * tbl)
{
    sVec < idx > dayCounts;
    sVec < idx > sortedCountsIndex;
    sVec < idx > isFinal;

    sVec < double > proportions;
    sVec < idx > durations;

    proportions.vadd(1,(double) 1);
    durations.vadd(1,0);

    sStr t2;
    _ctx.qproc().reqSetData(_ctx.outReqID(),"file://" OUTFILE,0,0);
    _ctx.qproc().reqDataPath(_ctx.outReqID(),OUTFILE,&t2);
    sFile::remove(t2);
    sFil dateFile(t2);

    for (idx i=0; i < tbl->rows(); i++) {


        sStr s;
        sStr f;
        sStr c;
        tbl->printCell(s,i,colSet[0]);
        tbl->printCell(f,i,colSet[1]);
        tbl->printCell(c,i,colSet[2]);
        char * initDate = s.ptr();
        char * finDate = f.ptr();
        char * cenDate = c.ptr();

        if (strcmp(finDate," ") == 0 || strcmp(finDate,"") == 0) {
            if (strcmp(cenDate," ") != 0 && strcmp(cenDate,"") !=0) {
                dayCounts.vadd(1,dateDifference(initDate,cenDate));
                isFinal.vadd(1,0);
            }
        } else {
            if (strcmp(cenDate," ") == 0 || strcmp(cenDate,"") == 0) {
                dayCounts.vadd(1,dateDifference(initDate,finDate));
                isFinal.vadd(1,1);
            }
        }

    }

    sortedCountsIndex.add(dayCounts.dim());
    sSort::sort(dayCounts.dim(),dayCounts.ptr(),sortedCountsIndex.ptr());


    idx countDown = sortedCountsIndex.dim();
    idx durationIndex = 0;
    double lastProportion = 1.0;

    while (countDown > 0) {
        idx currentDay = dayCounts[sortedCountsIndex[durationIndex]];
        idx numFinal = 0;
        idx totalLost = 0;

        while ((durationIndex < sortedCountsIndex.dim()) && (currentDay == dayCounts[sortedCountsIndex[durationIndex]]))  {
            totalLost++;
            if (isFinal[sortedCountsIndex[durationIndex]])
                numFinal++;

            durationIndex++;
        }

        if (numFinal > 0) {
            double newProportion = lastProportion * ((double)(countDown - numFinal) / (double)countDown);
            proportions.vadd(1,newProportion);
            durations.vadd(1,currentDay);
            lastProportion = newProportion;
        }

        countDown -= totalLost;

    }


    for (idx i = 0; i < proportions.dim(); i++) {

        dateFile.printf("%" DEC,durations[i]);
        dateFile.printf(",");
        dateFile.printf("%f",proportions[i]);
        dateFile.printf("\n");

    }

    return tbl;
}


static idx dateDifference (char * initial, char * final) {

    sVec <idx> iInts = parseDate(initial);
    sVec <idx> fInts = parseDate(final);

    idx diff = 365 * (fInts[2] - iInts[2]);

    static const int monthLength[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (iInts[1] < fInts[1]) {
        diff += monthLength[iInts[1]] - iInts[0];
        diff += fInts[0];
        for (idx i = iInts[1]+1; i< fInts[1]; i++) {
            diff += monthLength[i];
        }
    } else if (iInts[1] == fInts[1]) {
        diff += fInts[0] - iInts[0];
    } else {
        diff += monthLength[iInts[1]] - iInts[0];
        diff += fInts[0];
        for (idx i = iInts[1] + 1; i<12; i++) {
            diff += monthLength[i];
        }
        for (idx i = 0; i < fInts[1]; i++) {
            diff += monthLength[i];
        }
    }

    return diff;
}

static sVec < idx > parseDate (char * date) {
    sVec <idx> DayMonthYear;

    char * end = 0;

    char * day = new char[2];
    day[0] = date[0];
    day[1] = date[1];
    sStr dayStr;
    dayStr.printf("%s", day);

    DayMonthYear.vadd(1,strtoidx(dayStr, &end, 0));

    char month[4];
    month[0] = date[2];
    month[1] = date[3];
    month[2] = date[4];
    month[3] = 0;

    static const char * months = "JAN" _ "FEB" _ "MAR" _ "APR" _ "MAY" _ "JUN" _ "JUL" _ "AUG" _ "SEP" _ "OCT" _ "NOV" _ "DEC" _;
    idx mthIdx = -1;

    sString::compareChoice(month, months, &mthIdx, true, 0, false);
    DayMonthYear.vadd(1,mthIdx);

    char year[4];
    year[0] = date[5];
    year[1] = date[6];
    year[2] = date[7];
    year[3] = date[8];
    sStr yearStr;
    yearStr.printf("%s", year);

    DayMonthYear.vadd(1,strtoidx(yearStr, &end, 0));

    delete day;
    return DayMonthYear;
}


