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
#ifndef sLib_std_html_hpp
#define sLib_std_html_hpp

#include <slib/core/sIO.hpp>
#include <slib/core/var.hpp>

namespace slib {

    class sHtml: public sIO
    {

        public:
            sHtml(bool immediate)
                : sIO(0, immediate ? (sIO::callbackFun) (::printf) : 0, immediate ? stdout : 0)
            {
            }

            static char * cleanEscapes(sStr * dst, sVec <sMex::Pos> * ofs, const char * src, idx len, bool issplit, bool is_http_header);
            static idx readCGIPost(sVec<sMex::Pos> * ofs, char * post, idx len = 0);

            static idx inputCGI(sStr * bfr, sVec<sMex::Pos> * ofs, FILE * fp, idx argc, const char * * argv, sVar * form = 0, const char * mangleNameChar = 0, bool isCookie = true, const char * method = 0, bool no_env = false);
            static idx inputCGI(FILE * fp, idx argc, const char * * argv, sVar * form, const char * mangleNameChar = 0, bool isCookie = true, const char * method = 0)
            {
                sStr bfr;
                sVec<sMex::Pos> ofs;
                return inputCGI(&bfr, &ofs, fp, argc, argv, form, mangleNameChar, isCookie, method);
            }
            enum eFieldsOut
            {
                fDoFieldName = 0x00000001,
                fDoFieldVal = 0x00000002,
                fDoFieldNameQuote = 0x00000004,
                fDoFieldValQuote = 0x00000008,
                fDoFieldValRequired = 0x00000010
            };
            static const char * headerStartMarker;
            static idx outFieldList(sStr * str, sVar * form, const char * fieldlist00, idx doflags = fDoFieldName | fDoFieldVal | fDoFieldValQuote | fDoFieldValRequired, const char * autoQuote = 0, const char * separSymb = ",",
                const char * quoteSymb = "\"", bool doValueClipEnds = true, const char * excludefields00 = 0);

            static const char * contentTypeByExt(const char * flnm = 0);

            struct sFileData
            {
                idx index;
                idx size;
                idx written;
                idx lastMod;

                sFileData()
                    : index(-1), size(-1), written(-1), lastMod(-1)
                {
                }
            };

            struct sPartPair
            {
                typedef sVec<sPartPair> TParts;
                udx start;
                udx end;

                sPartPair() : start(0), end(0) {}
                sPartPair(udx s, udx e) : start(s), end(e) {}

                bool overlaps(const sPartPair &pair) const
                {
                    return start <= pair.end && pair.start <= end;
                }
                bool neighbour(const sPartPair &pair) const
                {
                    return start == pair.end + 1 || pair.start == end + 1;
                }
                bool merge(const sPartPair &pair)
                {
                    if (overlaps(pair) || neighbour(pair))
                    {
                        start = sMin<udx>(start, pair.start);
                        end = sMax<udx>(end, pair.end);
                        return true;
                    }
                    return false;
                }
                bool operator<(const sPartPair &pair) const
                {
                    return end < pair.start;
                }
                bool operator>(const sPartPair &pair) const
                {
                    return start > pair.end;
                }
                static idx comparator(void *param, void *array, idx i1, idx i2)
                {
                    idx res = ((sPartPair *)array)[i1].start - ((sPartPair *)array)[i2].start;
                    if (!res)
                    {
                        res = i1 - i2;
                    }
                    return res;
                }
            };

            static sPartPair::TParts parseRange(char * rangeHeader,udx max = sUdxMax);

            static void outFormData(sVar * form, const char * formflnm = 0, bool truncate = false);
            static FILE * grabInData(FILE * readfrom, const char * * envp, const char * postFile = 0);
    };

}
#endif 