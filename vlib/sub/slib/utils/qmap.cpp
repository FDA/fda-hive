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

#include <slib/utils/qmap.hpp>
#include <slib/utils/tbl.hpp>

using namespace slib;

idx sQMap::mapCSV(sIO * jsonType,sIO * jsonObj, sStr * f, const char * tpname, const char * flags)
{

    sTbl tbl;tbl.parse(f->ptr(),f->length(),sTbl::fPreserveQuotes|sTbl::fAllowEmptyCells, ",");
    idx sz,sza,szo=0,poi=0;
    const char * p;

    sVec < idx > coltypes;
    const char * tpnames = "integer" _ "real" _ "datetime" _ "string" __;
    tbl.interpretCols(coltypes);

    idx rRole=-1,rOrder=-1,rHdr=-1,rMax=0;
    if(flags) {
        p=strchr(flags,'r');if(p){rRole=atoidx(p+1);rMax=sMax(rRole,rMax+1);}
        p=strchr(flags,'o');if(p){rOrder=atoidx(p+1);rMax=sMax(rOrder,rMax+1);}
        p=strchr(flags,'t');if(p){rHdr=atoidx(p+1);rMax=sMax(rHdr,rMax+1);}
    }
    if(rHdr<0)rHdr=rMax;

    sStr nm,ti,rl;
    sDic < idx > colnames;



    jsonType->printf("{\"new\":{\"_id\":\"$newid\",\"_type\":\"type\",\"name\":\"%s\",\"title\":\"%s\",\"description\":\"%s parsed from CSV\",\"parent\":{\"1\":\"$type.base_user_type\"},\"fields\":{",tpname,tpname,tpname);
    const char * po="";
    for ( idx ic=0; ic<tbl.cols(); ++ic) {
        nm.cut(0);ti.cut(0);
        p=tbl.cell(rHdr,ic,&sz);
        sString::searchAndReplaceSymbols(&nm,p,sz," %/+-\n\r\t#.(),;","_",0,true,true,false,true);
        sString::changeCase(nm.ptr(0),nm.length(), sString::eCaseLo);
        sString::searchAndReplaceSymbols(&ti,p,sz," \n\r\t"," ",0,true,true,false,true);

        idx *pi=colnames.get(nm.ptr());
        if(pi) {nm.shrink00();nm.printf("-%" DEC, *pi);(*pi)++;}
        *colnames.set(nm.ptr())=1;

        if(ic)jsonType->printf(",");
        jsonType->printf("\"%" DEC "\":{\"field_name\":\"%s\",\"field_title\":\"%s\",\"field_type\":\"%s\",\"field_is_optional_fg\": true"
            ,ic+1,nm.ptr(),ti.ptr(0),sString::next00(tpnames,coltypes[ic]));
        if(rRole>=0){
            p=tbl.cell(rRole,ic,&sz);
            jsonType->printf(",\"field_role\":\"%.*s\"",(int)sz,p);
            if(sz==9 && memcmp(p,"timestamp",sz)==0)
                coltypes[ic]=sTbl::eDate;
        }
        if(rOrder>=0){
            p=tbl.cell(rOrder,ic,&sz);
            if(sz) {
                po=p;
                poi=0;
                szo=sz;
                if(*po=='-' && szo==1) {
                    po=0;szo=0;
                }
            }
            if(po && szo ) {
                jsonType->printf(",\"field_order\":\"%" DEC ".%.*s\"",poi,(int)szo,po);
                ++poi;
            }
        }

        jsonType->printf("}");
    }
    jsonType->printf("},\"_perm\": [{\"party\": \"/system/\",\"act\": {\"browse\": true,\"read\": true,\"write\": true,\"exec\": true,\"del\": true,\"admin\": true,\"share\": true,\"download\": true },\"_infect\": {\"party\": [ \"member\" ] }}, {\"party\": \"/everyone/\",\"act\": {\"browse\": true,\"read\": true },\"_infect\": { \"party\": [ \"member\" ] } } ]}}");


    jsonObj->printf("{");
    for ( idx ir=rMax+1; ir<tbl.rows(); ++ir) {
        if(ir>rHdr+1)jsonObj->printf("\n,");
        jsonObj->printf("\"new-%" DEC "\":{\"_id\":\"$newid\",\"_type\":\"%s\"",ir , tpname);
        for ( idx ic=0; ic<tbl.cols(); ++ic) {
            const char * pv=tbl.cell(ir,ic,&sza);
            if(*pv==',' || *pv=='\n' || (*pv=='\r' && *(pv+1)=='\n') ) sza=0;

            if(sza==0)continue;
            const char * p=(const char*)colnames.id(ic,&sz);
            jsonObj->printf(",\"%.*s\":",(int)sz,p);
            jsonObj->printf("\"%.*s\"",(int)sza,pv);
        }
        jsonObj->printf("}");
    }
    jsonObj->printf("}");





    return 0;
}



