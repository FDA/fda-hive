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
#ifndef sLib_TaxIon_hpp
#define sLib_TaxIon_hpp

#include <slib/core.hpp>
#include <slib/utils/tbl.hpp>

namespace slib {
    class sTaxIon
    {
            sIon myION, *ION;
            //static idx ion_TaxTraverserCallback(sIon * ion, sIonWander * ts, idx command );
            sIonWander ts, ts1, tsTree, ts2, ts3, ts4;
            sIonWander tsTIBG;
            idx * argTIBG, *argT1, *argTTree, *argT2, *argT3, *argT4;
            sStr taxbuf;
            sStr errormsg;

        public:
            sTaxIon(const char * ionName = 0, bool precompile = true)
            {
                ION = &myION;
                if( ionName )
                    init(ionName, precompile);
            }
            sTaxIon(sIon * ion)
            {
                if( ion )
                    init(ion);
            }
            sTaxIon * init(const char * ionName, bool precompile)
            {
                ION->init(ionName, sMex::fReadonly);
                ts.addIon(ION);
                ts1.addIon(ION);
                tsTree.addIon(ION);
                ts2.addIon(ION);
                ts3.addIon(ION);
                ts4.addIon(ION);
                tsTIBG.addIon(ION);
                if (precompile){
                    const char *err = initPrecompile();
                    if (err){
                        errormsg.printf("%s", err);
                    }
                }
                return this;
            }
            sTaxIon * init(sIon * ion)
            {
                ION = ion;
                return this;
            }
            const char * initPrecompile(void);
            bool isok(sStr *errmsg = 0)
            {
                if (errormsg.length()){
                    if (errmsg){
                        errmsg->add(errormsg.ptr(),errormsg.length());
                    }
                    return false;
                }
                return ION->ok();
            }
            sIon * getIon()
            {
                return ION;
            }

        public:
            const char * getAccByGi(const char * ginum, idx gilen );
            const char * getTaxIdsByAccession(const char * acclist);
            const char * getTaxIdsByRankandAcc(const char * acclist, const char * rank);
            const char * getParentTaxIds(const char * taxlist, sStr *taxParent, sStr *taxRank = 0);
            const char * getTaxTreeChildrenInfo(const char * taxlist, idx taxlen = 0, sStr *buf = 0);
            const char * getTaxIdsByName(const char *name, idx limit, sStr *taxResults00 = 0);
            const char * extractTaxIDfromSeqID(sStr *taxid, sStr *ginum, sStr *accnum, const char *seqid, idx seqlen = 0, const char *defaultStr = 0);

            /* getTaxIdInfo:
             * input:
             *    taxid information and length of the id
             * output: returns information of a specific taxid (comma separated):
             * 1. current taxid
             * 2. parent taxid
             * 3. rank (contains species, genus, family, etc)
             * 4. name (scientific name)
             * 5. number of nodes under this taxid
             */
            const char * getTaxIdInfo(const char *taxid, idx taxidlen = 0);
            idx getTaxCount(const char * rank, idx ranklen = 0);
            idx getRecordCount(idx recordTypeIndex);
            idx dnaScreeningGItoAcc(sStr &dst, const char *src);
            const char *precompileGItoAcc(void);
            bool filterbyParent (idx taxPar, idx taxid, sStr *path = 0, bool taxidOnly = false);
    };
} // namespace

#endif // sLib_TaxIon_hpp
