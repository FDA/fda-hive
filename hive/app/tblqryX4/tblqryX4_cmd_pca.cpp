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

#include <ssci/math/clust/clust2.hpp>

using namespace slib::tblqryx4;

#define PRFX "pca-"
#define OUTFILE "pca.csv"

idx tblqryx4::tblqryCmd_pca(sQPrideBase * qp, idx req, sTabular & tbl, sVar * pForm)
{
    sVec < idx > columnsToUse;
    sVec < idx > rowsToUse;

    const char * s=pForm->value(PRFX"colSet");
    if(s)sString::scanRangeSet(s,0,&columnsToUse,0,0,0);
    s=pForm->value(PRFX"rowSet");
    if(s)sString::scanRangeSet(s,0,&rowsToUse,0,0,0);



    sStr pathT;
    qp->reqSetData(req,"file://"OUTFILE,0,0);
    qp->reqDataPath(req,OUTFILE,&pathT);
    sFile::remove(pathT);

    sFil out(pathT);



    return 0;
}

