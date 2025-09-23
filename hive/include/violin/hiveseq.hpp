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
#ifndef sHiveSeq_hpp
#define sHiveSeq_hpp

#include <slib/std.hpp>
#include <ssci/bio.hpp>
#include <ulib/ulib.hpp>

namespace sviolin
{
    class sHiveseq : public sBioseqSet
    {
        private:
            sStr bioseqSourceNames;
            sStr objSourceIDs;
            struct Bios{
                sBioseq* bioseq;
                bool selfManaged;
                idx ofsFileName;
            };

            sDic < Bios > bioseqlist;
            const sUsr * user;
            bool isVioseqlist;

        protected:
            void registerBioseq (sBioseq * bioseq, const char * filename=0, idx seqNum=0, idx seqCnt=0, idx partialRangeStart=0, idx partialRangeLen=0,  bool selfman=true);
            friend class DnaHiveseqProc;

        public:
            bool digestFile( const char * filename , idx seqNum=1, idx seqCnt=0, idx partialRangeStart=0, idx partialRangeLen=0,  bool selfman=true, EBioMode mode = eBioModeShort, bool allowSubSeqFailure=false);
            bool parse (const char * filename, EBioMode mode = eBioModeShort, bool allowDigestFailure = false, const sUsr * luser=0);

            idx expandHiveseq( sStr * buf, const char * filename, idx seqStart=1, idx seqEnd=0, idx partialRangeStart=0, idx partialRangeLen=0, const char * separ=0, const char * sourceFilePath=0);

            sStr * log;
            const char * log_prefix, * log_suffix;
            static const char * default_log_suffix;

            sHiveseq (const sUsr * usr=0, const char * filename=0, EBioMode mode = eBioModeShort, bool isVlist = false, bool allowDigestFailure = false, sStr * log_ = 0, const char * log_prefix_ = 0, const char * log_suffix_ = 0)
            {
                user=usr;
                isVioseqlist = isVlist;
                log = log_;
                log_prefix = log_prefix_? log_prefix_ : sStr::zero;
                log_suffix = log_suffix_  == 0 ? log_prefix[0] == '\n' ? sStr::zero : default_log_suffix : log_suffix_;

                if(!filename)return;
                parse(filename, mode, allowDigestFailure);
            }

            idx getMinLen()
            {
                idx minLen = sIdxMax, l_min = 0;
                for(const char * ff = objSourceIDs.ptr(); ff; ff = sString::next00(ff)) {
                    sHiveId id(strstr(ff, "obj://") ? ff + 6 : ff);
                    std::unique_ptr<sUsrObj> o(user->objFactory(sHiveId(id)));
                    if( o.get() ) {
                        l_min = o->propGetI("len-min");
                        if( l_min ) {
                            minLen = sMin(minLen, l_min);
                        }
                    }
                }
                return minLen;
            }

            idx getMaxLen()
            {
                idx maxLen = 0;
                for(const char * ff = objSourceIDs.ptr(); ff; ff = sString::next00(ff)) {
                    sHiveId id(strstr(ff, "obj://") ? ff + 6 : ff);
                    std::unique_ptr<sUsrObj> o(user->objFactory(sHiveId(id)));
                    if( o.get() ) {
                        maxLen = sMax(o->propGetI("len-min"), maxLen);
                    }
                }
                return maxLen;
            }

            real getAvLen()
            {
                real avLen = 0, l_avLen;
                idx cnt = 0, l_cnt = 0;
                for(const char * ff = objSourceIDs.ptr(); ff; ff = sString::next00(ff)) {
                    sHiveId id(strstr(ff, "obj://") ? ff + 6 : ff);
                    std::unique_ptr<sUsrObj> o(user->objFactory(sHiveId(id)));
                    if( o.get() ) {
                        l_cnt = o->propGetI("rec-count");
                        l_avLen = o->propGetR("len-avg");
                        if( l_avLen && l_cnt ) {
                            if( avLen ) {
                                avLen *= (real) cnt / (cnt + l_cnt);
                            }
                            cnt += l_cnt;
                            avLen += l_avLen * ((real) l_cnt / cnt);
                        }
                    }
                }
                return avLen;
            }

            ~sHiveseq (){

                destroy();
            }

            virtual void destroy(bool justMemoryFree=false)
            {

                for(idx i=0; i<bioseqlist.dim(); ++i){
                    if( bioseqlist[i].selfManaged ){
                        if(justMemoryFree)
                            bioseqlist[i].bioseq->destroy(justMemoryFree);
                        else
                            delete bioseqlist[i].bioseq;
                    }
                }
            }

            virtual void empty()
            {
                sBioseqSet::empty();
                destroy();
                bioseqlist.empty();
            }

            const static idx defaultSliceSizeI;
            const static char * const defaultSliceSizeS;
    };


}


#endif 