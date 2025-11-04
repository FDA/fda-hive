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
#include <slib/std.hpp>

#include <slib/utils.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <violin/hiveproc.hpp>
#include "biotargetedqc.hpp"

#define isSameBase(baseS,baseQ) (baseS!='N'&&baseQ!='N'&&baseS==baseQ)

class alignQC
{
    public:
        sHiveProc & hp;
        sHiveal * alignments;
        sHiveseq * Sub, * Qry;
        sStr err_buf;

        struct tailLen {
            idx left;
            idx right;
        };

        struct posStats {
            idx match;
            idx mismatch;
            idx insertion;
            idx deletion[3];
        };

        alignQC(sHiveProc & hiveproc) : hp(hiveproc)
        {
            alignments = 0;
            Sub = Qry = 0;
        }

        virtual ~alignQC() {
            delete Sub;
            delete Qry;
            delete alignments;
        }

        virtual bool init();
        virtual bool collectStats(void);

    protected:
        void countTailLens(sVec<tailLen> &tail_len_freq, sBioseqAlignment::Al * hdr, idx * match, idx repeats);

        void getPos(idx &sub_pos, idx &qry_pos, sBioseqAlignment::Al * hdr, idx * match, idx match_pos);
        void countDels(sVec<posStats> &pos_stats, idx qry_pos, idx consecutive_dels, idx repeats);
        void countPosStats(sVec<posStats> &pos_stats, sBioseqAlignment::Al * hdr, idx * match, idx repeats, sStr &uncomp_sub);

        void printTailHist(sStr &left_out, sStr & right_out, const sVec<tailLen> &tail_hist);
        void printPosStats(sStr &out, const sVec<posStats> &pos_stats);

};

class rnaSeqQC : public alignQC
{
        typedef alignQC TParent;
    public:
        sFil * profile;

        idx short_transcript_threshold, long_transcript_threshold;

        rnaSeqQC(sHiveProc & hiveproc)
            : alignQC(hiveproc)
        {
            profile = 0;
            short_transcript_threshold = long_transcript_threshold = 0;
        }
        virtual ~rnaSeqQC() {
            delete profile;
        }

        virtual bool init();
        virtual bool collectStats(void);

        struct MeanCov
        {
            idx cnt;
            real sumCov;
            MeanCov(){sSet(this,0);}
        };

};

class targetedSeqQC : public alignQC
{
        typedef alignQC TParent;

    public:
        enum eTargetMode { eTargetingOff = 1, eTargetingOn = 2 };
        sIonWander * wander;
        eTargetMode target_mode;

        targetedSeqQC(sHiveProc & hiveproc)
            : alignQC(hiveproc), wander(0), target_mode()
        {
        }

        virtual ~targetedSeqQC()
        {
            delete wander;
        }

        virtual bool init();
        virtual bool collectStats(void);

};

class DnaAlignQC : public sHiveProc
{
    public:
        std::unique_ptr<alignQC> qc;
        DnaAlignQC(const char * defline00, const char * srv)
            : sHiveProc(defline00, srv)
        {
        }

        virtual idx OnExecute(idx);
        virtual sRC OnSplit(idx req, idx &cnt);
        void cleanUp();

        bool isRNASeq(void);
        bool isTargeted(void);
};
