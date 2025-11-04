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
#include <ion/sIon-core.hpp>
#include <slib/utils/tbl.hpp>






idx sIonWander::traverseTable(sTbl * tbl, sTxtTbl * txttbl, idx countResultMax, idx startResult, idx contRowsMax )
{

    idx countRows=sMin ( (tbl ? tbl->rows() : txttbl->rows() ) , contRowsMax) , success = 0;
    idx cntCols=tbl ? tbl->cols() : txttbl->cols();


    sVec < ColMapStruct > ColMap(sMex::fBlockCompact);
    ColMapStruct * cm;
    for ( idx ip=0, size; ip< parametricArguments.dim() ; ++ip) {
        cm=ColMap.add();


        const char * colName=(const char*)parametricArguments.id(ip,&size);
        cm->ptrToChange=(idx*)getSearchDictionaryPointer(ip);
        if(colName[0]=='$'){++colName;--size;}
        char * end=(char*)(colName+size);

        cm-> columnInTbl=sNotIdx;
        for( idx ihdr=0,len; ihdr<cntCols; ++ihdr) {
            const char * hdrName=tbl ? tbl->cell(0,ihdr,&len) : txttbl->cell(-1,ihdr,&len);
            if( len!=size || memcmp((const void*)colName,hdrName,len )!=0 )
                continue;
            cm-> columnInTbl=ihdr;
            break;
        }
        if(cm-> columnInTbl==sNotIdx)
            cm-> columnInTbl=strtoidx(colName,&end,10)+1;
    }


    cm=ColMap.ptr();
    idx cntMaps=ColMap.dim();


    for (idx iRow=1; iRow<=countRows ; ++iRow ){


        if(tbl) {
            for( idx im=0; im<cntMaps; ++im) {
                cm[im].ptrToChange[0]=sConvPtr2Int( tbl->cell(iRow, cm[im].columnInTbl, &cm[im].ptrToChange[1] ) );
            }
        }else {
            for( idx im=0; im<cntMaps; ++im) {
                cm[im].ptrToChange[0]=sConvPtr2Int( txttbl->cell(iRow, cm[im].columnInTbl, &cm[im].ptrToChange[1] ) );
            }
        }

        traverseBuf.cut(0);
        traverse();

        if( traverseBuf.length()!=0 ) {
            ++success;
            if(success<=startResult)
                return success;
        }
     }
   return success;
}




