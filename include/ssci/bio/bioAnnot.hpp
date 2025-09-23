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
#ifndef sLib_bioAnnot_hpp
#define sLib_bioAnnot_hpp

#include <slib/utils.hpp>
#include <ssci/bio.hpp>
#include <slib/std.hpp>

namespace slib {


    class sIonAnnotBase
    {
        public:

            sIonWander ts1, ts2, ts3, ts4, ts5;

            sIO localBuf;
            sStr rBuf;
            idx ionCnt;
            sIO errIO;

            sIonAnnotBase(const char * pathList00 = 0, bool precompile = true) {
                ionCnt = 0;
                rBuf.cut(0);
                localBuf.cut(0);
                errIO.cut(0);
                if (pathList00){
                    init (pathList00, true, precompile);
                }
            }

            sIonAnnotBase * init(const char * pathList00=0, bool force = true, bool precompile = true)
            {
                localBuf.cut(0);
                ionCnt=0;
                ts1.attachIons(pathList00,sMex::fReadonly,sString::cnt00(pathList00));
                ionCnt = ts1.ionList.dim();
                if (!ionCnt){
                    errIO.printf("Can't open any ionFile");
                    return 0;
                }

                ts2.attachIons(pathList00,sMex::fReadonly,sString::cnt00(pathList00));
                ts3.attachIons(pathList00,sMex::fReadonly,sString::cnt00(pathList00));
                ts4.attachIons(pathList00,sMex::fReadonly,sString::cnt00(pathList00));
                ts5.attachIons(pathList00,sMex::fReadonly,sString::cnt00(pathList00));

                if (precompile){
                    const char *err = initPrecompile();
                    if (err){
                        errIO.printf("%s", err);
                        return 0;
                    }
                }
                return this;
            }

            sIonAnnotBase * addIon(sIon * curIon, bool precompile = true)
            {
                ts1.addIon(curIon);
                ionCnt = ts1.ionList.dim();
                if (!ionCnt){
                    errIO.printf("Can't open ionFile");
                    return 0;
                }
                ts2.addIon(curIon);
                ts3.addIon(curIon);
                ts4.addIon(curIon);
                ts5.addIon(curIon);
                if (precompile){
                    const char *err = initPrecompile();
                    if (err){
                        errIO.printf("%s", err);
                        return 0;
                    }
                }
                return this;
            }

            const char * initPrecompile(void);
            idx getNumType(sStr *lbuf = 0, const char * separator = ",");
            bool setGeneType (const char * geneType, idx geneid_len);
            idx getNumRecords (const char * seqid, idx seqlen, idx startpos, idx endpos);
            const char * prepareQuerywithTypes (const char * types00, idx format=0);
            const char * getRecordInfo (const char * seqid, idx seqlen, idx startpos, idx endpos, bool escapeCSV = false);
            idx getAllGenes(sStr *lbuf = 0, const char * separator = ",");
            const char * getGeneInfo (const char *seqid, idx seqlen, idx startpos, idx enpos);
    };


}
#endif
