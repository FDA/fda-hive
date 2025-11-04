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
#include <violin/hiveutils.hpp>



using namespace sviolin;

idx sHiveUtils::tblReadInfoSources(sUsr * user, sVec < sHiveId > * infoSrcList, sVec < TBLINFO > * tblSet, sStr * srcNames, sDic <idx > * columnIds)
{
    for (idx ii=0; ii<infoSrcList->dim() ; ++ii) {
        sUsrObj inf(*user,(*infoSrcList)[ii]);
        sStr infPath;inf.getFilePathname(infPath,"_.csv");

        TBLINFO * ti=tblSet->add();
        if(srcNames){
            if(ii>0)srcNames->printf(",");
            inf.propGet("name",srcNames);srcNames->shrink00();
        }
        ti->infCSV.init(infPath,sMex::fReadonly);
        ti->tbl.parse(ti->infCSV.ptr(),ti->infCSV.length());
        ti->type=TBLINFO::eCSV;

        if(columnIds) {
            for ( idx ic=1; ic<ti->tbl.cols(); ++ic) {
                idx lh;const char * hdr=ti->tbl.cell(0,ic,&lh);
                if(!lh)break;
                *columnIds->set(hdr,lh)=1;
            }
        }
        for ( idx ir=1; ir<ti->tbl.rows(); ++ir) {
            idx len;const char * gene=ti->tbl.cell(ir,(idx)0,&len);
            *ti->infoDic.set(gene,len)=ir;
        }
    }
    if(srcNames)srcNames->add0();
    return tblSet->dim();
}

idx sHiveUtils::tblOutInfoSources(sVar * var, const char * key,sVec < TBLINFO > * tblSet, const char * separ)
{
    for(idx i=0;i<tblSet->dim(); ++i) {
        TBLINFO * ti=tblSet->ptr(i);

        idx * pGeneInfo=ti->infoDic.get(key,sLen(key));
        if(pGeneInfo){
            for(idx ic=1;ic<ti->tbl.cols();++ic){
                idx lh;const char * hdr=ti->tbl.cell(0,ic,&lh);
                if(!lh)break;
                idx len;const char * val=ti->tbl.cell(*pGeneInfo,ic,&len);
                if(len)
                    var->inp(hdr, val,len,lh);
            }
        }
    }
    return var->dim();
}



