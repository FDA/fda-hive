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

#define PRFX "compAnalysis-"

using namespace slib;
using namespace slib::tblqryx4;

struct compRows
{
        idx rowGil;
        idx rowPvd75;
        idx rowQbvd;
        idx rowHIVE;
};

namespace slib {
    namespace tblqryx4 {
        class CompAnalysisCommand : public Command
        {
            private:
                idx multipleMethods;
                idx algoToReport;

            public:
                CompAnalysisCommand(ExecContext & ctx) : Command(ctx)
                {
                    multipleMethods = 0;
                    algoToReport = 2;
                }

                const char * getName() { return "compAnalysis"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdCompAnalysisFactory(ExecContext & ctx) { return new CompAnalysisCommand(ctx); }
    };
};

bool CompAnalysisCommand::init(const char * op_name, sVariant * arg)
{
    if (arg->getDicElt("multipleMethods") != 0)
        multipleMethods = arg->getDicElt("multipleMethods")->asInt();
    if (arg->getDicElt("algoToReport") != 0)
        algoToReport = arg->getDicElt("algoToReport")->asInt();

    return true;
}

bool CompAnalysisCommand::compute(sTabular * tbl)
{
    sVariantTbl * outTabular = new sVariantTbl (4, 0);
    outTabular->setVal(-1,0,"NS5B POS");
    outTabular->setVal(-1,1,"No. Subjects");
    outTabular->setVal(-1,2,"SUB10 (+/-)");
    outTabular->setVal(-1,3,"Ratio");

    sDic <sDic <sDic <sVec <idx> > > > originalMapping;

    sStr patient, position, company;
    idx patientCol = tbl->colId("USUBJID");
    idx posCol = tbl->colId("AAPOS");
    idx compCol = tbl->colId("VARDECT");
    idx visitCol = tbl->colId("VISIT");
    idx refCol = tbl->colId("AAREF");
    idx subCol = tbl->colId("AASUB");
    idx frequencyCol = tbl->colId("AAFREQ");

    if (patientCol == -sIdxMax || posCol == -sIdxMax || compCol == -sIdxMax || visitCol == -sIdxMax || refCol == -sIdxMax || subCol == -sIdxMax || frequencyCol == -sIdxMax)
        return tbl;

    sDic < sVec <compRows> > posToFinalRow; // this vector will map the position (from the table) to the row it is in


    for (idx r = 0; r < tbl->rows(); r++)
    {
        patient.cut(0);
        position.cut(0);
        company.cut(0);
        tbl->printCell(patient, r, patientCol);
        sString::changeCase(patient,0,sString::eCaseLo);
        tbl->printCell(position, r, posCol);
        sString::changeCase(position,0,sString::eCaseLo);
        tbl->printCell(company, r, compCol);
        sString::changeCase(company,0,sString::eCaseLo);
        idx size = originalMapping[patient.ptr()][position.ptr()][company.ptr()].dim();
        originalMapping[patient.ptr()][position.ptr()][company.ptr()].add(1);
        originalMapping[patient.ptr()][position.ptr()][company.ptr()][size] = r;
    }

    //iterating over patients
    for (idx p = 0; p < originalMapping.dim(); p++)
    {
        const char * pat = static_cast <const char *> (originalMapping.id(p));

        //iterating over positions
        for (idx po = 0; po < originalMapping[pat].dim(); po++)
        {
            const char * pos = static_cast <const char *> (originalMapping[pat].id(po));
            compRows toAdd;
            toAdd.rowGil=toAdd.rowPvd75=toAdd.rowQbvd=toAdd.rowHIVE=-1;

            idx howManyCompaniesInThisPos=0;
            idx iPrevComp=-1;

            //iterating over companies
            for (idx c = 0; c < originalMapping[pat][pos].dim(); c++)
            {
                const char * comp = static_cast <const char *> (originalMapping[pat][pos].id(c));

                idx baselineRow = -1;
                sStr tmp;
                real baselineFreq = 0;

                for (idx i = 0 ; i < originalMapping[pat][pos][comp].dim(); i++)
                {
                    tmp.cut(0);
                    tbl->printCell(tmp, originalMapping[pat][pos][comp][i], visitCol);

                    if (!strcasecmp (tmp.ptr(), "Baseline"))
                    {
                        baselineRow = originalMapping[pat][pos][comp][i];
                        baselineFreq += tbl->rval(baselineRow, frequencyCol);
                        //break;
                    }
                }

                //real baselineFreq = 0;
                //if (baselineRow > -1)
                //    baselineFreq = tbl->rval(baselineRow, frequencyCol);


                //real maxDelta = -sIdxMax;
                //idx row = -1;

                for (idx i = 0 ; i < originalMapping[pat][pos][comp].dim(); i++)
                {
                    real tempDelta = tbl->rval(originalMapping[pat][pos][comp][i],frequencyCol) - baselineFreq;

                    //if (tempDelta > maxDelta)
                    //{
                    //    maxDelta = tempDelta;
                    //    row = i;
                    //}

                    //if (maxDelta > 0.1)
                    if( tempDelta > 0.1)
                    {
                        tmp.cut(0);
                        tbl->printCell(tmp,originalMapping[pat][pos][comp][i],compCol);

                        if (!strcmp(tmp.ptr(), "PVD75"))
                            toAdd.rowPvd75 = originalMapping[pat][pos][comp][i];
                        else if (!strcmp(tmp.ptr(), "GIL"))
                            toAdd.rowGil = originalMapping[pat][pos][comp][i];
                        else if (!strcmp(tmp.ptr(), "QbVD"))
                            toAdd.rowQbvd = originalMapping[pat][pos][comp][i];
                        else if (!strcmp(tmp.ptr(), "HIVE"))
                            toAdd.rowHIVE = originalMapping[pat][pos][comp][i];
                        if(iPrevComp!=c) {
                            iPrevComp=c;
                            ++howManyCompaniesInThisPos;
                        }


                    }
                }
            }

            /*
            if (comp_analysis->multipleMethods || (toAdd.rowGil > -1 && toAdd.rowPvd75 > -1) ||
                (toAdd.rowGil > -1 && toAdd.rowQbvd > -1) ||
                (toAdd.rowPvd75 > -1 && toAdd.rowQbvd > -1) ||
                (toAdd.rowPvd75 > -1 && toAdd.rowHIVE > -1) ||
                (toAdd.rowGil > -1 && toAdd.rowHIVE > -1) ||
                (toAdd.rowGil > -1 && toAdd.rowHIVE > -1))*/
            if (multipleMethods || howManyCompaniesInThisPos>= algoToReport)
            {
                idx size = posToFinalRow[pos].dim();
                posToFinalRow[pos].add(1);
                posToFinalRow[pos][size] = toAdd;
            }
        }
    }

    idx curRow = 0;

    for (idx position = 0; position < posToFinalRow.dim(); position++)
    {
        const char * pos = static_cast <const char *> (posToFinalRow.id(position));

        if (posToFinalRow[pos].dim() < algoToReport)
            continue;

        sStr toPrint;
        toPrint.printf("%s", pos);
        outTabular->setVal(curRow,0,toPrint.ptr());

        toPrint.cut(0);
        toPrint.printf("%" DEC, posToFinalRow[pos].dim());
        outTabular->setVal(curRow,1,toPrint.ptr());

        sDic <idx> sub10ratio;

        for (idx x = 0; x < posToFinalRow[position].dim(); x++)
        {
            sStr sub10;
            sStr ref,sub;
            idx occurence = 0;

            if (posToFinalRow[pos][x].rowGil > -1)
            {
                ref.cut(0); sub.cut(0); sub10.cut(0);
                tbl->printCell(sub, posToFinalRow[pos][x].rowGil,subCol);
                tbl->printCell(ref, posToFinalRow[pos][x].rowGil,refCol);

                ref.cut(1);
                sub.cut(1);

                sub10.printf("%s%s%s",ref.ptr(), pos, sub.ptr());

                idx * exists = sub10ratio.get(sub10);

                if (exists && occurence == 0)
                    (*exists)++;
                else
                {
                    sub10ratio[sub10.ptr()] = 1;
                    occurence = 1;
                }
            }
            if (posToFinalRow[pos][x].rowQbvd > -1)
            {
                ref.cut(0); sub.cut(0); sub10.cut(0);
                tbl->printCell(sub, posToFinalRow[pos][x].rowQbvd,subCol);
                tbl->printCell(ref, posToFinalRow[pos][x].rowQbvd,refCol);

                ref.cut(1);
                sub.cut(1);

                sub10.printf("%s%s%s",ref.ptr(), pos, sub.ptr());

                idx * exists = sub10ratio.get(sub10);

                if (exists && occurence == 0)
                    (*exists)++;
                else
                {
                    sub10ratio[sub10.ptr()] = 1;
                    occurence = 1;
                }
            }
            if (posToFinalRow[pos][x].rowPvd75 > -1)
            {
                ref.cut(0); sub.cut(0); sub10.cut(0);
                tbl->printCell(sub, posToFinalRow[pos][x].rowPvd75,subCol);
                tbl->printCell(ref, posToFinalRow[pos][x].rowPvd75,refCol);

                ref.cut(1);
                sub.cut(1);

                sub10.printf("%s%s%s",ref.ptr(), pos, sub.ptr());

                idx * exists = sub10ratio.get(sub10);

                if (exists && occurence == 0)
                    (*exists)++;
                else
                {
                    sub10ratio[sub10.ptr()] = 1;
                    occurence = 1;
                }
            }
            if (posToFinalRow[pos][x].rowHIVE > -1)
            {
                ref.cut(0); sub.cut(0); sub10.cut(0);
                tbl->printCell(sub, posToFinalRow[pos][x].rowHIVE,subCol);
                tbl->printCell(ref, posToFinalRow[pos][x].rowHIVE,refCol);

                ref.cut(1);
                sub.cut(1);

                sub10.printf("%s%s%s",ref.ptr(), pos, sub.ptr());

                idx * exists = sub10ratio.get(sub10);

                if (exists && occurence == 0)
                    (*exists)++;
                else
                {
                    sub10ratio[sub10.ptr()] = 1;
                    occurence = 1;
                }
            }
        }

        sStr sub10Print, ratioPrint;

        for (idx x = 0; x < sub10ratio.dim(); x++)
        {
            const char * key = static_cast <const char *> (sub10ratio.id(x));

            if (x == 0)
            {
                sub10Print.printf("%s",key);
                ratioPrint.printf ("%" DEC, sub10ratio[x]);
            }
            else
            {
                sub10Print.printf("/%s",key);
                ratioPrint.printf ("-%" DEC, sub10ratio[x]);
            }
        }

        outTabular->setVal(curRow, 2, sub10Print.ptr());
        outTabular->setVal(curRow, 3, ratioPrint.ptr());

        curRow++;
    }

    setOutTable(outTabular);
    return true;
}
