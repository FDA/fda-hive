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

#include <ssci/bio/bioseqqc.hpp>

DenovoStatsCollector::DenovoStatsCollector(sBioseq &genome) {

    contigs.resize(genome.dim());
    idx offset = 0;
    idx repeats;
    idx length;
    contigSum = 0;
    for (idx i = 0; i < genome.dim(); ++i) {
        repeats = genome.rpt(i);
        length = genome.len(i);
        contigs.add(repeats - 1);
        for (idx j = 0; j < repeats; ++j) {
            contigs[i + offset + j] = length;
            contigSum += length;
        }
        offset += repeats - 1;
    }
    sSort::sort(contigs.dim(), contigs.ptr());
}

DenovoStatsCollector::DenovoStatsCollector(const sVec<idx> &genome) {
    contigs.resize(genome.dim());
    contigSum = 0;
    for (idx i = 0; i < genome.dim(); ++i) {
        contigs[i] = genome[i];
        contigSum += contigs[i];
    }
    sSort::sort(contigs.dim(), contigs.ptr());
}

idx DenovoStatsCollector::numOfContigs() {
    return contigs.dim();
}

idx DenovoStatsCollector::minContigLen() {
    if (contigs.dim() == 0)
        return 0;
    return contigs[0];
}

idx DenovoStatsCollector::maxContigLen() {
    if (contigs.dim() == 0)
        return 0;
    return contigs[contigs.dim()-1];
}

real DenovoStatsCollector::meanContigLen() {
    if (contigs.dim() == 0)
        return 0;
    real sum = static_cast<real>(contigSum);
    return sum / contigs.dim();
}

NL DenovoStatsCollector::collectNL(int x) {
    sVec<int> xs;
    xs.add();
    xs[0] = x;
    sVec<NL> result;
    collectNLs(result, xs);
    return result[0];
}

void DenovoStatsCollector::collectNLs(sVec<NL> &nls, const sVec<int> &xs) {
    nls.resize(xs.dim());

    if (contigs.dim() == 0) {
        for (int i = 0; i < xs.dim(); ++i) {
            nls[i].x = xs[i];
            nls[i].n = 0;
            nls[i].l = 0;
        }
        return;
    }

    sVec<real> medians;
    medians.resize(xs.dim());
    for (idx i = 0; i < medians.dim(); ++i)
        medians[i] = contigSum * (xs[i] / 100.0);
    idx sum = 0;
    idx nlsFound = 0;
    idx l;
    for (l = 1; l <= contigs.dim(); ++l) {
        if (nlsFound >= medians.dim())
            break;
        sum += contigs[contigs.dim() - l];
        for (idx k = 0; k < medians.dim(); ++k) {
            if (medians[k] != -1 && sum >= medians[k]) {
                nls[k].x = xs[k];
                nls[k].n = contigs[contigs.dim() - l];
                nls[k].l = l;
                medians[k] = -1;
                nlsFound++;
            }
        }
    }
}

DenovoQCStats DenovoStatsCollector::collectStats() {
    DenovoQCStats stats;
    sVec<int> xs;
    xs.resize(2);
    xs[0] = 50;
    xs[1] = 75;
    sVec<NL> nls;
    collectNLs(nls, xs);
    stats.n50l50 = nls[0];
    stats.n75l75 = nls[1];
    stats.numOfContigs = numOfContigs();
    stats.meanContigLen = meanContigLen();
    stats.minContigLen = minContigLen();
    stats.maxContigLen = maxContigLen();
    return stats;
}

void DenovoStatsCollector::printStats(sStr &out, DenovoQCStats stats, const char * name) {
    out.printf("name, n50, l50, n75, l75, numOfContigs, meanContigLen, minContigLen, maxContigLen\n");
    out.printf("%s, %" DEC ", %" DEC ", %" DEC ", %" DEC ", %" DEC ", %.2f, %" DEC ", %" DEC "\n",
        name, stats.n50l50.n, stats.n50l50.l, stats.n75l75.n, stats.n75l75.l,
        stats.numOfContigs, stats.meanContigLen, stats.minContigLen, stats.maxContigLen);
}

void DenovoStatsCollector::printNLs(sStr &out, const sVec<NL> &nls) {
    out.printf("x, n, l\n");
    for (int i = 0; i < nls.dim(); ++i)
        out.printf("%d, %" DEC ", %" DEC "\n", nls[i].x,  nls[i].n, nls[i].l);
}
