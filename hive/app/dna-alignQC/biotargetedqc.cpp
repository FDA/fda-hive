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
#include <cstring>
#include "biotargetedqc.hpp"

idx sort_idx_dic_by_key(void * param, void * arr, idx i1, idx oper, idx i2)
{
    sDic<idx> * idCnt = (sDic<idx> *) arr;
    idx first = *static_cast<idx *>(idCnt->id(i1));
    idx second = *static_cast<idx *>(idCnt->id(i2));
    idx res = first - second;
    if( oper == sSort::eSort_EQ && res == 0 )
        return true;
    else if( oper == sSort::eSort_GT && res > 0 )
        return true;
    else if( oper == sSort::eSort_GE && res >= 0 )
        return true;
    else if( oper == sSort::eSort_LT && res < 0 )
        return true;
    else if( oper == sSort::eSort_LE && res <= 0 )
        return true;
    return false;
}

idx CoverageHist::coverageAt(idx index) const
{
    idx i = isSorted() ? order_[index] : index;
    return *static_cast<const idx *>(histogram_.id(i));
}

idx CoverageHist::countAt(idx index) const
{
    idx i = isSorted() ? order_[index] : index;
    return *histogram_.ptr(i);
}

idx CoverageHist::countOfCoverage(idx cov) const
{
    const idx* count = histogram_.get(&cov, sizeof(cov));
    if( count == 0 )
        return 0;
    return *count;
}

void CoverageHist::countCoverageInBuffer(Buffer &buf, idx accum_point)
{
    if (accum_point == 0)
        accum_point = buf.dim();
    for(idx j = 0; j < accum_point; ++j) {
        if( buf[j] > 0 ) {
            *histogram_.set(&buf[j], sizeof(idx)) += 1;
            ++buf.pos_hit;
        }
    }
}

void CoverageHist::accumulateCoverageInBuffer(Interval interval, idx repeats, Buffer &buf)
{
    idx start = interval.start;
    idx end = interval.end;
    idx & cur_pos = buf.cur_pos;

    idx min_buf_size = (end - start) + 1;
    if( buf.dim() < min_buf_size )
        buf.resize(min_buf_size);
    if( end < (cur_pos + buf.dim()) ) {
        for(idx j = start - cur_pos; j <= end - cur_pos; ++j)
            buf[j] += repeats;
    } else {
        bool overlap_buf = start < (cur_pos + buf.dim());
        idx accum_point = overlap_buf ? (start - cur_pos) : buf.dim();
        countCoverageInBuffer(buf, accum_point);
        if( overlap_buf ) {
            idx remaining_buf = buf.dim() - (start - cur_pos);
            std::memmove(buf.ptr(0), buf.ptr(start - cur_pos), sizeof(idx) * remaining_buf);
            for(idx j = remaining_buf; j < buf.dim(); ++j)
                buf[j] = 0;
        } else {
            buf.set(0);
        }
        cur_pos = start;
        for(idx j = start - cur_pos; j <= end - cur_pos; ++j)
            buf[j] += repeats;
    }
}

void CoverageHist::addSubject(sBioalSet &alignments, idx sub_num)
{
    const idx init_buf_size = 2000;
    Buffer buf;
    buf.add(init_buf_size);
    idx sub_len = alignments.Sub->len(sub_num);
    idx num_als_for_sub = 0;
    idx al_start_index = alignments.listSubAlIndex(sub_num, &num_als_for_sub) - 1;
    idx al_end_index = al_start_index + num_als_for_sub;
    for(idx i = al_start_index; i < al_end_index; ++i) {
        sBioseqAlignment::Al * hdr = alignments.getAl(i);
        idx * m = alignments.getMatch(i);
        Interval interval;
        interval.start = hdr->getSubjectStart(m);
        interval.end = hdr->getSubjectEnd(m);
        idx repeats = count_repeats_ ? alignments.getRpt(i) : 1;
        aligned_reads_ += repeats;
        accumulateCoverageInBuffer(interval, repeats, buf);
    }
    countCoverageInBuffer(buf);
    idx zed = 0;
    *histogram_.set(&zed, sizeof(zed)) += sub_len - buf.pos_hit;
    total_bases_ += sub_len;
    is_sorted_ = false;
}

void CoverageHist::sort()
{
    order_.resize(histogram_.dim());
    sSort::sortCallback(sort_idx_dic_by_key, 0, histogram_.dim(), &histogram_, order_.ptr());
    is_sorted_ = true;
}

void CoverageHist::printCSV(sStr &out) const
{
    out.addString("coverage, count\n");
    for(idx i = 0, hist_dim = dim(); i < hist_dim; ++i) {
        out.addNum(coverageAt(i));
        out.addString(", ");
        out.addNum(countAt(i));
        out.addString("\n");
    }
}

void CoverageHist::printCoverageStats(sStr &out) const
{
    out.printf("Name, Number\n");
    out.printf("Total Number of Bases, %lld\n", totalBases());
    out.printf("Minimum Coverage, %lld\n", minCoverage());
    out.printf("Maximum Coverage, %lld\n", maxCoverage());
    out.printf("Average Coverage, %.2f\n", avgCoverage());
}

void CoverageHist::printHitStats(sStr &out) const
{
    out.printf("Name, Number\n");
    out.printf("Total Reads, %lld\n", totalReads());
    out.printf("Aligned Reads, %lld\n", alignedReads());
    out.printf("Aligned Percentage, %.0f%%\n", alignedReadsPercentage());
}

idx CoverageHist::minCoverage() const
{
    idx min_cov = 0;
    if (isSorted()) {
        min_cov = coverageAt(0);
    } else {
        for (idx i = 0, hist_dim = dim(); i < hist_dim; ++i) {
            if (i == 0)
                min_cov = coverageAt(0);
            min_cov = sMin(min_cov, coverageAt(i));
        }
    }
    return min_cov;
}

idx CoverageHist::maxCoverage() const
{
    idx max_cov = 0;
    if (isSorted()) {
        max_cov = coverageAt(dim() - 1);
    } else {
        for (idx i = 0, hist_dim = dim(); i < hist_dim; ++i) {
            max_cov = sMax(max_cov, coverageAt(i));
        }
    }
    return max_cov;
}

real CoverageHist::avgCoverage() const
{
    real avg_cov = 0;
    for(idx i = 0, hist_dim = dim(); i < hist_dim; ++i)
        avg_cov += coverageAt(i) * countAt(i);
    avg_cov /= total_bases_;
    return avg_cov;
}

void TargetedCoverageHist::addSubject(sBioalSet &alignments, idx sub_num)
{
    const idx init_buf_size = 5000;
    Buffer buf;
    buf.add(init_buf_size);
    sVec<Interval> regions;
    idx targeted_base_count = 0;
    getTargetedRegions(alignments, sub_num, regions);
    if (regions.dim() == 0)
        return;
    idx num_als_for_sub = 0;
    idx al_start_index = alignments.listSubAlIndex(sub_num, &num_als_for_sub) - 1;
    idx al_end_index = al_start_index + num_als_for_sub;
    idx al_index = al_start_index;
    idx al_index_reached_for_targeted = -1;
    idx al_index_reached_for_total = -1;
    idx first_intersect_al = al_start_index;
    for (idx region_index = 0, region_dim = regions.dim(); region_index < region_dim; ++region_index) {
        idx region_start = regions[region_index].start;
        idx region_end = regions[region_index].end;
        targeted_base_count += region_end - region_start + 1;
        bool check_next_region;
        idx next_region_start;
        if (region_index >= region_dim - 1) {
            check_next_region = false;
        } else {
            check_next_region = true;
            next_region_start = regions[region_index + 1].start;
        }
        for (al_index = first_intersect_al; al_index < al_end_index; ++al_index) {
            sBioseqAlignment::Al * hdr = alignments.getAl(al_index);
            idx * m = alignments.getMatch(al_index);
            idx al_start = hdr->getSubjectStart(m);
            idx al_end = hdr->getSubjectEnd(m);
            idx al_repeats = count_repeats_ ? alignments.getRpt(al_index) : 1;
            if (check_next_region && (al_end >= next_region_start)) {
                first_intersect_al = al_index;
                check_next_region = false;
            }
            if (al_index > al_index_reached_for_total) {
                aligned_reads_ += al_repeats;
                al_index_reached_for_total = al_index;
            }
            if (al_start > region_end) {
                break;
            } else if ((al_start <= region_end) && (al_end >= region_start)) {
                Interval new_interval;
                new_interval.start = sMax(region_start, al_start);
                new_interval.end = sMin(region_end, al_end);
                accumulateCoverageInBuffer(new_interval, al_repeats, buf);
                if (al_index > al_index_reached_for_targeted) {
                    on_target_reads_ += al_repeats;
                    al_index_reached_for_targeted = al_index;
                }
            }
        }
        if (check_next_region)
            first_intersect_al = al_index;
    }
    for (; al_index < al_end_index; ++al_index) {
        idx al_repeats = count_repeats_ ? alignments.getRpt(al_index) : 1;
        if (al_index > al_index_reached_for_total) {
            aligned_reads_ += al_repeats;
            al_index_reached_for_total = al_index;
        }
    }
    countCoverageInBuffer(buf);
    idx zed = 0;
    *histogram_.set(&zed, sizeof(zed)) += targeted_base_count - buf.pos_hit;
    total_bases_ += targeted_base_count;
    is_sorted_ = false;
}

void TargetedCoverageHist::printHitStats(sStr &out) const
{
    CoverageHist::printHitStats(out);
    out.printf("On Target Reads, %lld\n", onTargetReads());
    out.printf("On Target Percentage, %.0f%%\n", onTargetPercentage());
}

void TargetedCoverageHist::getTargetedRegions(sBioalSet &alignments, idx sub_num, sVec<Interval> &regions)
{
    const char * id = alignments.Sub->id(sub_num);
    wander_->setSearchTemplateVariable("$seqID1", 7, id, 0);
    wander_->resetResultBuf();
    wander_->traverse();
    if (wander_->traverseBuf.length() == 0)
        return;
    char * cur_line = wander_->traverseBuf.ptr(0);
    while (*cur_line != '\0') {
        char * start_str = cur_line;
        char * end_str = strchr(start_str, ',') + 1;
        Interval region;
        region.start = atol(start_str);
        region.end = atol(end_str) - 1;
        regions.add();
        regions[regions.dim() - 1] = region;
        cur_line = strchr(cur_line, '\n') + 1;
    }
}


NormCoverageHist::NormCoverageHist(const CoverageHist& coverage_hist)
    : histogram_(), total_bases_(coverage_hist.totalBases())
{
    real avg_cov = coverage_hist.avgCoverage();
    for(idx i = 0, hist_dim = coverage_hist.dim(); i < hist_dim; ++i) {
        real norm_cov = static_cast<idx>((1000.0 * coverage_hist.coverageAt(i) / avg_cov + 0.5)) / 1000.0;
        *histogram_.set(&norm_cov, sizeof(norm_cov)) += coverage_hist.countAt(i);
    }
}

void NormCoverageHist::printCSV(sStr& out) const
{
    out.addString("norm_coverage, count\n");
    for(idx i = 0, hist_dim = dim(); i < hist_dim; ++i) {
        out.printf("%.2f", normCoverageAt(i));
        out.addString(", ");
        out.addNum(countAt(i));
        out.addString("\n");
    }
}

idx NormCoverageHist::countOfNormCoverage(real norm_coverage) const
{
    const idx* count = histogram_.get(&norm_coverage, sizeof(norm_coverage));
    if( count == 0 )
        return 0;
    return *count;
}

CumulNormCoverageTable::CumulNormCoverageTable(const NormCoverageHist& norm_cov_hist, sVec<real> norm_cov_input)
    : dim_(norm_cov_input.dim()), norm_cov_(), cumul_count_(), cumul_frac_()
{
    norm_cov_.add(dim_);
    cumul_count_.add(dim_);
    cumul_frac_.add(dim_);
    idx total_bases = norm_cov_hist.totalBases();
    sSort::sort(norm_cov_input.dim(), norm_cov_input.ptr(0));
    for(idx i = 0; i < dim_; ++i) {
        norm_cov_[i] = norm_cov_input[i];
        cumul_count_[i] = 0;
        for(idx j = 0, hist_dim = norm_cov_hist.dim(); j < hist_dim; ++j) {
            if( norm_cov_hist.normCoverageAt(j) >= norm_cov_[i] )
                cumul_count_[i] += norm_cov_hist.countAt(j);
        }
        cumul_frac_[i] = static_cast<real>(cumul_count_[i]) / total_bases;
    }
}


void CumulNormCoverageTable::printCSV(sStr &out) const
{
    out.addString("norm_coverage, cumul_count, cumul_frac\n");
    for(idx i = 0; i < dim_; ++i)
        out.printf("%.2f, %" DEC ", %.2f\n", norm_cov_[i], cumul_count_[i], cumul_frac_[i]);
}

CoverageReqTable::CoverageReqTable(const CumulNormCoverageTable& cov_table, sVec<real> frac_input)
    : dim_(frac_input.dim()), cumul_frac_(), mean_norm_cov_(), cov_req_10x_(), cov_req_30x_()
{
    cumul_frac_.add(dim_);
    mean_norm_cov_.add(dim_);
    cov_req_10x_.add(dim_);
    cov_req_30x_.add(dim_);
    for(idx i = 0; i < dim_; ++i) {
        cumul_frac_[i] = frac_input[i];
        mean_norm_cov_[i] = meanNormCoverage(cov_table, cumul_frac_[i]);
        cov_req_10x_[i] = 10.0 / mean_norm_cov_[i];
        cov_req_30x_[i] = 30.0 / mean_norm_cov_[i];
    }
}

real CoverageReqTable::meanNormCoverage(const CumulNormCoverageTable& cov_table, real frac) const
{
    idx table_dim = cov_table.dim();
    idx i;
    for (i = 0; i < table_dim; ++i) {
        if (cov_table.cumulFracAt(i) < frac)
            break;
    }
    if (i == 0)
        i = 1;
    real x1 = cov_table.normCoverageAt(i-1);
    real y1 = cov_table.cumulFracAt(i-1);
    real x2 = cov_table.normCoverageAt(i);
    real y2 = cov_table.cumulFracAt(i);
    real m = (y2 - y1) / (x2 - x1);
    real b = y2 - m * x2;
    return (frac - b) / m;
}

void CoverageReqTable::printCSV(sStr &out) const
{
    out.addString("perc_bases, mean_norm_cov, coverage_req_10x, coverage_req_30x\n");
    for(idx i = 0; i < dim_; ++i) {
        out.printf("%.0f%%, %.2f, %.0fx, %.0fx\n", cumul_frac_[i] * 100.0, mean_norm_cov_[i], cov_req_10x_[i], cov_req_30x_[i]);
    }
}
