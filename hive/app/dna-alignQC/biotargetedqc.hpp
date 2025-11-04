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
#include <ssci/bio.hpp>

class CoverageHist
{
    public:
        CoverageHist(idx total_reads, bool count_repeats = true) :
            histogram_(), order_(), total_bases_(0),
            count_repeats_(count_repeats), is_sorted_(false),
            aligned_reads_(0), total_reads_(total_reads)
        { };
        virtual ~CoverageHist() { };
        virtual void addSubject(sBioalSet &alignments, idx sub_num);
        virtual void printHitStats(sStr &out) const;
        void printCSV(sStr &out) const;
        void printCoverageStats(sStr &out) const;
        idx minCoverage() const;
        idx maxCoverage() const;
        real avgCoverage() const;
        idx coverageAt(idx index) const;
        idx countAt(idx index) const;
        idx countOfCoverage(idx coverage) const;
        void sort();
        bool isSorted() const { return is_sorted_; }
        idx totalBases() const { return total_bases_; }
        idx dim() const { return histogram_.dim(); }
        idx alignedReads() const { return aligned_reads_; }
        idx totalReads() const { return total_reads_; }
        real alignedReadsPercentage() const { return static_cast<real>(aligned_reads_) / total_reads_ * 100.0; }
    protected:
        struct Interval
        {
            idx start;
            idx end;
            bool operator==(const Interval &rhs) const { return start == rhs.start; }
            bool operator<(const Interval &rhs) const { return start < rhs.start; }
            bool operator<=(const Interval &rhs) const { return start <= rhs.start; }
            bool operator>(const Interval &rhs) const { return start > rhs.start; }
            bool operator>=(const Interval &rhs) const { return start >= rhs.start; }
        };
        class Buffer : public sVec<idx>
        {
            public:
            idx cur_pos;
            idx pos_hit;
            Buffer() : sVec(sMex::fSetZero), cur_pos(0), pos_hit(0) { };
        };
        void accumulateCoverageInBuffer(Interval interval, idx repeats, Buffer &buf);
        void countCoverageInBuffer(Buffer &buf, idx accum_point = 0);
        sDic<idx> histogram_;
        sVec<idx> order_;
        idx total_bases_;
        bool count_repeats_;
        bool is_sorted_;
        idx aligned_reads_;
        idx total_reads_;
};

class TargetedCoverageHist : public CoverageHist
{
    public:
        TargetedCoverageHist(sIonWander * wander, idx total_reads, bool count_repeats = true) :
            CoverageHist(total_reads, count_repeats), wander_(wander), on_target_reads_(0)
        {
            wander_->traverseCompile("c=find.annot(seqID=$seqID1);unique.1(c.pos);print(c.pos);");
        };
        ~TargetedCoverageHist() { };
        virtual void addSubject(sBioalSet &alignments, idx sub_num);
        virtual void printHitStats(sStr &out) const;
        idx onTargetReads() const { return on_target_reads_; }
        real onTargetPercentage() const { return static_cast<real>(on_target_reads_) / aligned_reads_ * 100.0; }
    protected:
        void getTargetedRegions(sBioalSet &alignments, idx sub_num, sVec<Interval> &regions);
        sIonWander * wander_;
        idx on_target_reads_;
};

class NormCoverageHist
{
    public:
        NormCoverageHist(const CoverageHist& coverage_hist);
        void printCSV(sStr &out) const;
        idx totalBases() const { return total_bases_; }
        idx dim() const { return histogram_.dim(); }
        real normCoverageAt(idx index) const { return *static_cast<const real*>(histogram_.id(index)); }
        idx countAt(idx index) const { return *histogram_.ptr(index); }
        idx countOfNormCoverage(real norm_coverage) const;
    protected:
        sDic<idx> histogram_;
        const idx total_bases_;
};

class CumulNormCoverageTable
{
    public:
        CumulNormCoverageTable(const NormCoverageHist& norm_cov_hist, sVec<real> norm_cov_input);
        void printCSV(sStr &out) const;
        idx dim() const { return dim_; }
        real normCoverageAt(idx index) const { return norm_cov_[index]; }
        idx cumulCountAt(idx index) const { return cumul_count_[index]; }
        real cumulFracAt(idx index) const { return cumul_frac_[index]; }
    protected:
        const idx dim_;
        sVec<real> norm_cov_;
        sVec<idx> cumul_count_;
        sVec<real> cumul_frac_;
};

class CoverageReqTable
{
    public:
        CoverageReqTable(const CumulNormCoverageTable& cov_table, sVec<real> frac_input);
        void printCSV(sStr &out) const;
        idx dim() const { return dim_; }
        real cumulFracAt(idx index) const { return cumul_frac_[index]; }
        real meanNormCoverageAt(idx index) const { return mean_norm_cov_[index]; }
        real coverageReq10xAt(idx index) const { return cov_req_10x_[index]; }
        real coverageReq30xAt(idx index) const { return cov_req_30x_[index]; }
    protected:
        real meanNormCoverage(const CumulNormCoverageTable &cov_table, real frac) const;
        const idx dim_;
        sVec<real> cumul_frac_;
        sVec<real> mean_norm_cov_;
        sVec<real> cov_req_10x_;
        sVec<real> cov_req_30x_;
};
