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
#include <violin/algolinks.hpp>
using namespace slib;

void AlgoLinks::loadHeptagonLinks(sUsr * user,sVec < sHiveId >  & heptids, sVec < Heptagon > & hepts, idx mapFiles)
{
    sVec < sHiveId >  hexids;

    sStrT buf;

    for(idx ihp=0; ihp<heptids.dim();++ihp) {
        Heptagon * hept=hepts.add(1);
        new (&(hept->obj)) sUsrObj(*user,heptids[ihp]);

        hept->obj.propGetHiveIds("parent_proc_ids",hexids);
        for(idx ihx=0; ihx<hexids.dim();++ihx) {
            Hexagon * hex =hept->hexs.add(1);
            new (&(hex->obj)) sUsrObj(*user,hexids[ihx]);

            new (&(hex->al)) sHiveal(user,hex->obj.IdStr());

            hex->obj.propGet00("query",&buf,";");
            hex->qry.parse(buf, sBioseq::eBioModeLong, false, user);buf.cut(0);

            hex->obj.propGet00("subject",&buf,";");
            hex->ref.parse(buf, sBioseq::eBioModeShort, false, user);buf.cut(0);

            hex->al.Sub=&(hex->ref);

        }
        hexids.cut(0);
        hept->ref0=&(hept->hexs.ptr(0)->ref);
        hept->qry0=&(hept->hexs.ptr(0)->qry);
        if(mapFiles&eMapSNPFile) {
            hept->obj.getFilePathname00(buf, "SNPprofile.csv" __);
            hept->flSNP.init(buf,sMex::fReadonly);buf.cut(0);
        }
        if(mapFiles&eMapProfileInfoFile) {
            hept->obj.getFilePathname00(buf, "ProfileInfo.csv" __);
            hept->flPI.init(buf,sMex::fReadonly);buf.cut(0);
        }

    }
}
