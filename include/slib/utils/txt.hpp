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
#pragma once
#ifndef sLib_utils_txt_hpp
#define sLib_utils_txt_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/var.hpp>
#include <slib/utils/tbl.hpp>

#include <regex.h>

namespace slib
{
    class sText { 

        public:
            static idx categoryListParseCsv(sStr * flbuf , const char * lrnsrc, idx lrnlen, sDic < sDic < sVec < idx > > >  * lcolset , sDic < idx > * ids, const char * empty=0, bool totalset=false, bool supportQuote=false);
            static idx categoryListParseCsv(sTabular * tbl, sVec < idx > * rowSet, sDic < sDic < sVec < idx > >  > * lcolset , sDic < idx > * ids, sVec < idx > * colsToAccumulateForCategorization=0, const char * empty=0, bool totalset=0) ;
            static idx categoryListInverse(sDic < sVec < idx > >  * lcolset , sVec <idx> * nocolset, sDic < idx > * ids, idx maxRows);
            static void categoryListToDic(sDic < sVec < idx > >  * pcolset , sDic < idx > * ids);
            static void categoryListOut(sStr * str, sDic < sDic < sVec < idx > >  > * lcolset , sDic < idx > * ids);



            static void fmtParseLeftTag(sVar * flist, const char * src, idx len, const char * subsects00=0, idx caselo=2, bool constid=false, const char * separsvar=0, const char * separsval=0);


            struct SearchStruc {
                const char * srch;
                idx len;
                regex_t rex;
            };

            static idx compileSearchStrings(const char * str00, sVec < SearchStruc > * ssv);
            static idx matchSearchToString(const char * str, idx len, SearchStruc * ss, idx cnt, idx maxmatch=1);
    };

}

#endif // sLib_utils_txt_hpp
