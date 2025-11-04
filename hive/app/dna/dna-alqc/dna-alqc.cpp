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


#include <qlib/QPrideProc.hpp>
using namespace slib;

#include <violin/violin.hpp>
#include <ion/sJson.hpp>
#include <slib/std/file.hpp>
#include <ssci/chem/spectr/spectraFile.hpp>
#include <ssci/chem/spectr/spectraPeaks.hpp>
#include <ssci/math/nr/nrutil.h>

#define NGSQC "ngsqc"
#define SEQPOS "seqpos"
#define REFSEQ "refseq"
#define BIOCOMPUTE "BCO:HIVE/ALQC"
#define BIOCOMPUTE_DB "algorithm"
#define BIOCOMPUTE_VERSION "1.0 Babajanyan"
#define HIVE_SOURCE "HIVE"
#define HIVE_DB "genome"
#define HIVE_VERSION "1.0"


struct QCRecord {

    idx genome_size, cnt_contigs, coverage_contigs, cnt_gaps, size_contigs, size_gaps, coverage_gaps, genome_coverage;
    real contig_percentile, gap_percentile;
    idx N50,L50,N75,L75,N90,L90,N95,L95;
    real phred_average,GC;

    real contig_momentum, aligned_momentum, overhang_momentum;
    idx count_major_mutations, count_major_indels;
    real indels_momentum, mutation_momentum, major_indels_momentum, major_mutation_momentum;
    real entropic_momentum,alignment_disbalance, alignment_anisotropy;

    idx reads_aligned, reads_unaligned;
    real rpkm;


};


class DnaAlQCProc: public sQPrideProc
{
    public:
        DnaAlQCProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {

        }
        struct Hexagon {
            sUsrObj obj;
            sHiveal al;
            sHiveseq ref;
            sHiveseq qry;
            };
        struct Heptagon {
            sUsrObj obj;
            sVec < Hexagon > hexs;
            sDic < sVec < idx > > refSet;
            };
        struct StatResult {
            const char *ref;
            idx startPos;
            idx endPos;
            idx in;
            sVec<real> shanWindows;
            };
        struct regionsData {
            const char *ref;
            idx start;
            idx end;
            };    

        idx OnExecute(idx req);
        idx referenceQC(Heptagon * hept, JSNode & jPos, JSNode & jAll);
        idx seqQC(Hexagon * hex, JSNode & jNGS);
        idx outPos(sHiveseq * ref, JSNode & jPos, sDic < sBioseqSNP::GapInfo  > * giList, idx whatToCollect);
        idx gbAnnotation(sHiveseq * ref, JSNode & js);

        static real ShannonCalc( const sVec <idx> &map, idx seqLength, idx N);
        static idx slidingWindow(const char *seqBits, idx startPos, idx length, idx windowSize, idx N, const char *ref, sVec <idx> *regions, sFil &regionsFp);
        static idx computeStat(const char *seqBits, idx startPos, idx endPos, idx windowSize, idx N, const char *ref, StatResult *res, sVec<sVec<real> > *shanAgg);
        static idx bitToChar(const sVec <idx> &map, idx N);
        static idx detectLowQuality(const sVec<real> *shanWindows, idx N_mer, const char *ref, sVec <idx> *regions, sFil &regionsFp);

        sVec < Heptagon > hepts;


};

real DnaAlQCProc::ShannonCalc(const sVec <idx> &map, idx seqLength, idx N){
  
  
  idx numCombinations = 1 << (2*N);

  real shan = 0.0;
  real shanUni = 0.0;
  real totalUni = 0.0;
  real total = 0.0; 

    for (idx in=0; in < map.dim(); ++in){

      real probUni = 1.0 / numCombinations;
      totalUni += probUni;
      if (probUni > 0){
        shanUni -= probUni * log2(probUni);
      }
      if (in < map.dim() && map[in] > 0){
        real prob = (real)map[in] / (real)(seqLength - N + 1);
        total += prob;
        if(prob > 0){
          shan -= prob * log2(prob);
        }
      }
    }

  
  shan = shan * 2 / shanUni;

  return shan;
}

idx DnaAlQCProc::computeStat(const char *seqBits, idx startPos, idx endPos, idx windowSize, idx N, const char *ref, StatResult *res, sVec<sVec<real> > *shanAgg) {
    
  res->shanWindows.resize(1);

  if (shanAgg->dim() == 0) {
      shanAgg->resize(1);
  }

  sVec<idx> windowCounts(sMex::fSetZero);
  idx combinations = 1 << (2 * 4);
  windowCounts.add(combinations);

  idx stepSize = windowSize / 2;

  if (stepSize > 1) {
      for (idx i = startPos; i <= startPos + windowSize - 4; ++i) {
          idx combinedLet = 0;
          for (idx nmer = 0; nmer < 4; ++nmer) {
              char letter = sBioseqAlignment::_seqBits(seqBits, i + nmer, 0);
              combinedLet = (combinedLet << 2) | letter;
          }
          windowCounts[combinedLet]++;
      }

      for (idx i = startPos + stepSize; i <= endPos - windowSize + 1; i += stepSize) {
          idx firstKmer = 0;
          for (idx nmer = 0; nmer < 4; ++nmer) {
              char letter = sBioseqAlignment::_seqBits(seqBits, i - stepSize + nmer, 0);
              firstKmer = (firstKmer << 2) | letter;
          }
          windowCounts[firstKmer]--;

          idx newKmer = 0;
          for (idx nmer = 0; nmer < 4; ++nmer) {
              char letter = sBioseqAlignment::_seqBits(seqBits, i + windowSize - 4 + nmer, 0);
              newKmer = (newKmer << 2) | letter;
          }
          windowCounts[newKmer]++;
      }
  } 
  else {
      for (idx i = startPos; i <= endPos - 4 + 1; ++i) {
          idx combinedLet = 0;
          for (idx nmer = 0; nmer < 4; ++nmer) {
              char letter = sBioseqAlignment::_seqBits(seqBits, i + nmer, 0);
              combinedLet = (combinedLet << 2) | letter;
          }
          windowCounts[combinedLet]++;
      }
  }


  real shanWindow = ShannonCalc(windowCounts, windowSize, 4);

  if (res->shanWindows.dim() > 0) {
      res->shanWindows[0] = shanWindow;
  } else {
      printf("Error: shanWindows is not properly resized\n");
      return -1;
  }

  sVec<real> *shan = shanAgg->ptr(0);
  real *val = shan->add(1);
  *val = shanWindow;

  return 0;
}

idx DnaAlQCProc::detectLowQuality(const sVec<real> *shanWindows, idx N_mer, const char *ref, sVec<idx> *regions, sFil &regionsFp) {
    
  bool inRegion = false;
  idx startPos = 0;
  idx endPos = 0;
  real sumEntropy = 0.0, sqrSumEntropy = 0.0;

  sVec<real> diff1(sMex::fSetZero);
  sVec<real> diff2(sMex::fSetZero);
  diff1.resize(shanWindows->dim());
  diff2.resize(shanWindows->dim());

  if (shanWindows->dim() == 0) {
    return 0;
  }

  for (idx i = 1; i < shanWindows->dim(); ++i) {
    diff1[i] = (*shanWindows)[i] - (*shanWindows)[i - 1];
    sumEntropy += (*shanWindows)[i];
    sqrSumEntropy += (*shanWindows)[i] * (*shanWindows)[i];
  }

  for (idx j = 1; j < diff1.dim(); ++j) {
    diff2[j] = diff1[j] - diff1[j - 1];
  }

  real meanEntropy = sumEntropy / shanWindows->dim();
  real varianceEntropy = (sqrSumEntropy / shanWindows->dim()) - (meanEntropy * meanEntropy);
  real stdDevEntropy = sqrt(varianceEntropy);

  real cut = 1.5; 
  real minVal = meanEntropy - (cut * stdDevEntropy);

  for (idx pos = 1; pos < diff2.dim(); ++pos) {

    if ((*shanWindows)[pos] < minVal) {
      if (!inRegion) {
        inRegion = true;
        startPos = pos;
        endPos = pos;
      } else {
        endPos = pos;  
      }
    } else {
      if (inRegion) {
        inRegion = false;
        if (startPos < endPos) {
          regions->add(2);
          (*regions)[regions->dim() - 2] = startPos;
          (*regions)[regions->dim() - 1] = endPos;
        }
      }
    }
  }

  if (inRegion && startPos < endPos) {
    regions->add(2);
    (*regions)[regions->dim() - 2] = startPos;
    (*regions)[regions->dim() - 1] = endPos;
  }

  return 0;
}

idx DnaAlQCProc::slidingWindow(const char *seqBits, idx startPos, idx length, idx windowSize, idx N, const char *ref, sVec <idx> *regions, sFil &regionsFp) {

  regions->empty();
  idx windowStart = startPos;
  idx windowEnd = startPos + windowSize - 1;
  sVec<StatResult> allResults(sMex::fSetZero);
  sVec<idx> lowQuality(sMex::fSetZero);
  sVec<real> allShan(sMex::fSetZero);
  sVec<idx> startPositions(sMex::fSetZero);
  sVec<real> smoothCurve(sMex::fSetZero);
  sVec<sVec<real> > shanAgg(sMex::fSetZero);

  while (windowEnd < length) {
    StatResult *result = allResults.add(1);
    result->ref = ref;
    result->startPos = windowStart;
    result->endPos = windowEnd;
    

    computeStat(seqBits, windowStart, windowEnd, windowSize, N, ref, result, &shanAgg);

    

    windowStart+=windowSize/2;
    windowEnd+=windowSize/2;

    
  }







  
  if (shanAgg.dim() > 0) {  
    sVec<real> *shan = &shanAgg[0]; 

    idx padSize = shan->dim() * 0.02;  

    real shanMax = (*shan)[0];
    for (idx c = 0; c < shan->dim(); ++c) {
        if ((*shan)[c] > shanMax) {
            shanMax = (*shan)[c];
        }
    }


    for (idx buff = 0; buff < padSize; ++buff) {
        shan->insert(0, 1);
        (*shan)[0] = shanMax; 
        shan->insert(shan->dim(), 1);
        (*shan)[shan->dim() - 1] = shanMax; 
    }
  }


  for (idx i = 0; i < shanAgg.dim(); ++i) {

    idx padLen = shanAgg[i].dim();
    udx n = pow(2,ceil(log2(padLen)));
    sMathNR::realft(shanAgg[i], n, 1);
    
    idx half = n/2; 
    idx lowPass = 15;

    for (idx ii = lowPass; ii < half; ++ii){
      shanAgg[i][2 * ii] = 0.0;
      shanAgg[i][2 * ii +1] = 0.0;
    }

    sMathNR::realft(shanAgg[i], n, -1);

    for (idx j = 0; j < shanAgg[i].dim(); ++j) {
      shanAgg[i][j] /= n; 
    }
    
    for (idx jj= 0; jj < shanAgg[i].dim(); ++jj) {
    }  

    detectLowQuality(&shanAgg[0], 4, ref, regions, regionsFp);
    
  }
  

  idx K = 4;
  idx shan_i = 0;

  for (idx i = 0; i < allResults.dim(); ++i) {
    StatResult result = allResults[i];
    

    for (idx j = 0; j < result.shanWindows.dim(); ++j) {
      if (K-1 == j){
        allShan.add(1);
        allShan[shan_i] = result.shanWindows[K-1];
        startPositions.add(1);
        startPositions[shan_i] = result.startPos;
        shan_i++;
      }
      
      
    }
    
  }

  return 0;
}






static const char * varNames []={
    "coverage",
    "mutations",
    "inserts",
    "deletions",
    "entropy",
    "phred_score",
    "fwd_rev_disbalance",
    "overhang_left" ,
    "overhang_right" ,
    "anisotropy" ,
    "partial_alignment"
};

void sBioseqSNP_refHead(JSNode & js, idx objId, const char * bId)
{
        js.link("analysis_platform_object_id",objId);
        js.link("assembled_genome_acc",bId);
        js.link("analysis_platform", "HIVE");
        js.link("id", BIOCOMPUTE "." BIOCOMPUTE_VERSION);

}

idx DnaAlQCProc::gbAnnotation(sHiveseq * ref, JSNode & seqannot)
{
    sStr bId,path,nm;
    udx cnt;

    JSNode iobj;

    for(idx is=0 ;is<ref->dim(); ++is) {

        const char * id=ref->id(is);const char * p=strpbrk(id,sString_symbolsBlank);if(!p)p=id+sLen(id);
        bId.printf(0,"%.*s",(int)(p-id),id);

        sUsrObjRes obj_res;
        user->objs2("^u-ionAnnot$", obj_res,&cnt, "name", bId.ptr(0),"_id,name");
        user->removeTrash(obj_res);
        idx inum=0;
        for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it),++inum) {



            if (!obj_res.id(it)) {
                printf("Invalid object at iteration: %" DEC "\n", inum);
                continue;
            }

            sUsrFile ufile(*obj_res.id(it), user);
            ufile.makeFilePathname(path, "ion");
            if (!sFile::exists(path)) {
                printf("File not found: %s\n", path.ptr());
                path.cut(0);
                continue;
            }

            sIonWander iWander;
            if (!iWander.addIon(0)->ion->init(path, sMex::fReadonly)) {
                printf("Ion initialization failed for file: %s\n", path.ptr());
                continue;
            }

            iWander.traverseCompile("k=foreach.record('1');a=find.annot(record=k.1);print(a.seqID,a.pos,a.type,a.id);");
            if (!iWander.traverse()) {
                printf("Traverse failed for file: %s\n", path.ptr());
                continue;
            }

            if (!iWander.pTraverseBuf->ptr() || iWander.pTraverseBuf->length() == 0) {
                printf("No data found during traverse for file: %s\n", path.ptr());
                continue;
            }

            sTbl tbl;
            tbl.parse(iWander.pTraverseBuf->ptr(), iWander.pTraverseBuf->length());

            idx prvstart = -1, prvend = -1;

            for (idx ir = 0; ir < tbl.rows(); ++ir) {
                idx start = tbl.ivalue(ir, 1);
                idx end = tbl.ivalue(ir, 2), tl, vl;
                const char *t = tbl.cell(ir, 3, &tl);
                const char *v = tbl.cell(ir, 4, &vl);

                if (prvstart != start || prvend != end) {
                    iobj = seqannot.linkobj("#");
                    sBioseqSNP_refHead(iobj, atoidx(ref->objSourceIDs.ptr(0)), bId);
                    JSNode irange = iobj.linkarr("range");
                    JSNode jrange = irange.linkobj("#");
                    jrange.link("genomic_coordinates_start", start);
                    jrange.link("genomic_coordinates_end", end);
                    prvstart = start;
                    prvend = end;
                }

                JSNode ifeat = iobj.linkarr("#");
                JSNode ift = ifeat.linkobj("#");
                ift.link("feature", nm.printf(0, "%.*s", (int)tl, t));
                ift.link("value", nm.printf(0, "%.*s", (int)vl, v));
            }

            path.cut(0);
                
        }

    }


    return 0;
}

idx DnaAlQCProc::outPos(sHiveseq * ref, JSNode & jPos,sDic < sBioseqSNP::GapInfo  > * giList, idx whatToCollect)
{
    sStr bId;
    for(idx is=0 ;is<giList->dim(); ++is) {

        const char * id=ref->id(is);
        const char * p=strpbrk(id,sString_symbolsBlank);
        if(!p)p=id+sLen(id);

        idx isout= is==0 ? 0 : 1;

        sVec < sBioseqSNP::ProfileGap > *spg=&(giList->ptr(is)->spg);

        for(idx i=0 ;i<spg->dim(); ++i) {
            sBioseqSNP::ProfileGap * wpg=spg->ptr(i);

            if(wpg->hasCoverage)continue;
            if(wpg->averageCoverage==0 && (whatToCollect==sBioseqSNP::eQuality || whatToCollect==sBioseqSNP::ePartiAl || whatToCollect==sBioseqSNP::eAnisotropy))
                continue;


            JSNode site=jPos.linkobj("#");
                sBioseqSNP_refHead(site, atoidx(ref->objSourceIDs.ptr(0)),bId.printf(0,"%.*s",(int)(p-id),id));
                JSNode a=site.linkarr("range");
                    JSNode s=a.linkobj("0");
                        s.link("start",wpg->start+1);
                        s.link("end",wpg->end+1);

                JSNode b=site.linkarr("features");
                    JSNode q=b.linkobj("0");
                        q.link("feature",varNames[whatToCollect]);
                        q.link("value",wpg->averageCoverage);
                site.link("pass",wpg->hasCoverage);

            ++isout;
        }

    }


    return(0);
}


idx DnaAlQCProc::referenceQC(Heptagon * hept, JSNode & annot, JSNode & refseq)
{
    sStr path;

  hept->obj.getFilePathname(path, "SNPprofile.csv");
    sFil fl1(path,sMex::fReadonly);
    path.cut(0);hept->obj.getFilePathname00(path, "ProfileInfo.csv" __);
    sFil fl2(path,sMex::fReadonly);

    idx gapWindowSize=1, minGapLength=1 ;
    sDic < sBioseqSNP::GapInfo  > Gi;
    sBioseqSNP::GapInfo * gi;
    QCRecord * qc;

    sHiveseq * ref= & hept->hexs.ptr(0)->ref;



    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength, sBioseqSNP::eCoverage, 1.);
    reqProgress(5, 6, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eCoverage );

    sVec < QCRecord > Qc;
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.add(1);sSet(qc);
        gi=Gi.ptr(is);

        const char * seq=ref->seq(is);
        idx len=ref->len(is);

        idx acgt[4];sSet(acgt, 0, sizeof(acgt));
        for( idx ip=0; ip<len; ++ip) {
            char let=sBioseqAlignment::_seqBits(seq, ip, 0);
            ++acgt[(idx)let];
        }
        qc->GC=(real)(acgt[1]+acgt[2])/len;

        qc->genome_size=len;
        qc->coverage_contigs=gi->ps.averageContigCoverage;
        qc->coverage_gaps=gi->ps.averageGapCoverage;
        qc->genome_coverage=gi->ps.totalGenomeCoverage;
        qc->cnt_contigs=gi->ps.totalContigsNumber;
        qc->cnt_gaps=gi->ps.totalGapsNumber;
        qc->size_gaps=gi->ps.totalGapLength;
        qc->size_contigs=gi->ps.totalContigLength;
        qc->contig_percentile=gi->ps.contigsPart;
        qc->gap_percentile=gi->ps.gapsPart;



        sVec < idx > ordLen;
        for( idx iw=0; iw< gi->spg.dim(); ++iw) {
            sBioseqSNP::ProfileGap * wpg=gi->spg.ptr(iw);
            if( !wpg->hasCoverage ) continue;
            idx cl=wpg->end-wpg->start+1;
            ordLen.vadd(1,-cl);
            qc->contig_momentum+=cl*wpg->averageCoverage;
        }
        sSort::sort(ordLen.dim(),ordLen.ptr());

        idx totLen=0;
        for( idx io=0; io< ordLen.dim(); ++io ) {
            if(totLen<qc->genome_size*0.5){
                qc->N50+=-ordLen[io];
                ++qc->L50;
            }
            if(totLen<qc->genome_size*0.75){
                qc->N75+=-ordLen[io];
                ++qc->L75;
            }
            if(totLen<qc->genome_size*0.90){
                qc->N90+=-ordLen[io];
                ++qc->L90;
            }
            if(totLen<qc->genome_size*0.95){
                qc->N95+=-ordLen[io];
                ++qc->L95;
            }
            totLen+=-ordLen[io];
        }
        qc->contig_momentum/=qc->genome_size;
    }


    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength,sBioseqSNP::eQuality, 21);
    reqProgress(5, 7, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eQuality );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->phred_average=gi->ps.averageContigCoverage;
    }

    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength,sBioseqSNP::eMutations, 0.5);
    reqProgress(5, 8, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eMutations);
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->count_major_mutations=gi->ps.totalGapLength;
        qc->major_mutation_momentum=gi->ps.averageGapCoverage;
        qc->mutation_momentum=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage)/(qc->size_contigs+qc->size_gaps);

    }

    idx totLen=0, totLenMaj=0;
    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength,sBioseqSNP::eInserts, 0.5);
    reqProgress(5, 9, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eInserts );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->count_major_indels+=gi->ps.totalGapLength;
        qc->major_indels_momentum+=gi->ps.averageGapCoverage*gi->ps.totalGapLength;
        qc->indels_momentum+=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage);
        totLen+=gi->ps.totalContigLength+gi->ps.totalGapLength;
        totLenMaj+=gi->ps.totalGapLength;
    }
    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength,sBioseqSNP::eDeletions, 0.5);
  reqProgress(5, 10, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eDeletions );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->count_major_indels+=gi->ps.totalGapLength;
        qc->major_indels_momentum+=gi->ps.averageGapCoverage*gi->ps.totalGapLength;
        qc->indels_momentum+=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage);
        totLen+=gi->ps.totalContigLength+gi->ps.totalGapLength;
        totLenMaj+=gi->ps.totalGapLength;
    }

    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        qc->indels_momentum/=totLen;
        if(totLenMaj)qc->major_indels_momentum/=totLenMaj;
    }
    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength,sBioseqSNP::eEntropy, 0.3);
  reqProgress(5, 11, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eEntropy );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->entropic_momentum=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage)/(gi->ps.totalGapLength+gi->ps.totalContigLength);
    }

    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl1, gapWindowSize, minGapLength,sBioseqSNP::eDisbalance, 10);
  reqProgress(5, 12, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eDisbalance);
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->alignment_disbalance=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage)/(gi->ps.totalGapLength+gi->ps.totalContigLength);
    }

    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl2, gapWindowSize, minGapLength,sBioseqSNP::eAnisotropy, 75 );
  reqProgress(5, 13, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eAnisotropy );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->alignment_anisotropy=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage)/(gi->ps.totalGapLength+gi->ps.totalContigLength);
    }

    totLen=0;
    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl2, gapWindowSize, minGapLength,sBioseqSNP::eOverhangLeft, 25 );
  reqProgress(5, 14, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eOverhangLeft );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->overhang_momentum+=gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage;
        totLen+=gi->ps.totalGapLength+gi->ps.totalContigLength;
    }
    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl2, gapWindowSize, minGapLength,sBioseqSNP::eOverhangRight, 25 );
  reqProgress(5, 15, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::eOverhangRight);
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->overhang_momentum+=gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage;
        totLen+=gi->ps.totalGapLength+gi->ps.totalContigLength;
    }
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        qc->overhang_momentum/=totLen;
    }

    Gi.empty();sBioseqSNP::createWindowSliders(ref, &Gi, &fl2, gapWindowSize, minGapLength,sBioseqSNP::ePartiAl, 0.75 );
  reqProgress(5, 16, 100);
    outPos(ref,annot,&Gi,sBioseqSNP::ePartiAl );
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        gi=Gi.ptr(is);
        qc->aligned_momentum=(gi->ps.totalGapLength*gi->ps.averageGapCoverage+gi->ps.totalContigLength*gi->ps.averageContigCoverage)/(gi->ps.totalGapLength+gi->ps.totalContigLength);
    }



    sVec<sBioal::Stat> stat;
    idx totReads=0;
    for ( idx ia=0; ia<hept->hexs.dim(); ++ia) {
        sBioal * ha=&hept->hexs[ia].al;
        ha->countAlignmentSummaryBySubject(stat);

        for(idx is=0 ;is<Gi.dim(); ++is) {
            qc=Qc.ptr(is);
            qc->reads_unaligned+=stat[0].foundRpt;
            qc->reads_aligned+=stat[is+1].foundRpt;
            totReads+=stat[is].foundRpt;
        }
    }
  totReads+= stat[Gi.dim()].foundRpt;
    for(idx is=0 ;is<Gi.dim(); ++is) {
        qc=Qc.ptr(is);
        qc->rpkm=qc->reads_aligned*1000000000./qc->genome_size/totReads;
    }



    sStr bId;
    for(idx is=0 ;is<Gi.dim(); ++is) {
        const char * id=ref->id(is);
        const char * p=strpbrk(id,sString_symbolsBlank);
        if(!p)p=id+sLen(id);

        qc=Qc.ptr(is);

        JSNode iseq=refseq.linkobj("#");
            sBioseqSNP_refHead(iseq, atoidx(ref->objSourceIDs.ptr(0)), bId.printf(0,"%.*s",(int)(p-id),id) );
            iseq.link("length", qc->genome_size);
            iseq.link("size_gaps",qc->size_gaps);
            iseq.link("size_contigs",qc->size_contigs);
            iseq.link("genome_coverage",qc->genome_coverage);
            iseq.link("coverage_contigs",qc->coverage_contigs);
            iseq.link("coverage_gaps",qc->coverage_gaps);
            iseq.link("cnt_contigs",qc->cnt_contigs);
            iseq.link("cnt_gaps",qc->cnt_gaps);
            iseq.linkpercent("contig_percentile",qc->contig_percentile);
            iseq.linkpercent("gap_percentile",qc->gap_percentile);
            iseq.link("contig_momentum",qc->contig_momentum);
            iseq.link("n50",qc->N50);
            iseq.link("l50",qc->L50);
            iseq.link("n75",qc->N75);
            iseq.link("l75",qc->L75);
            iseq.link("n90",qc->N90);
            iseq.link("l90",qc->L90);
            iseq.link("n95",qc->N95);
            iseq.link("l95",qc->L95);

            iseq.linkpercent("assembly_gc_content",qc->GC*100);
            iseq.link("phred_average",qc->phred_average);
            iseq.link("count_major_mutations",qc->count_major_mutations);
            iseq.link("count_major_indels",qc->count_major_indels);
            iseq.link("mutation_momentum",qc->mutation_momentum);
            iseq.link("indels_momentum",qc->indels_momentum);
            iseq.link("major_mutation_momentum",qc->major_mutation_momentum);
            iseq.link("major_indels_momentum",qc->major_indels_momentum);
            iseq.link("alignment_anisotropy",qc->alignment_anisotropy);
            iseq.link("overhang_momentum",qc->overhang_momentum);
            iseq.link("aligned_momentum",qc->aligned_momentum);
            iseq.link("entropic_momentum",qc->entropic_momentum);
            iseq.link("reads_unaligned",qc->reads_unaligned);
            iseq.link("reads_aligned",qc->reads_aligned);
            iseq.linkpercent("percent_reads_aligned",qc->reads_aligned*100./totReads);
            iseq.linkpercent("percent_reads_unaligned",qc->reads_unaligned*100./totReads);
            iseq.link("rpkm",qc->rpkm);

    }

  reqProgress(5, 19, 100);
    return 0;
}


#define workWithFile(_v_flnm) {obj.getFilePathname00(nam, _v_flnm __ );sFil fl(nam,sMex::fReadonly);cont.cut(0);cont.add(fl.ptr(0),fl.length());cont.add0();nam.cut(0);}

idx DnaAlQCProc::seqQC(Hexagon * hex , JSNode & n)
{
    sStr buf,cod;

    hex->obj.propGet00("query",&buf);
    formValue("codingTable",&cod,"Standard");


    sStr nam,cont;

    const char * possible="acgtryswkmbdhvn";

    for( const char * q=buf; q; q=sString::next00(q)) {
        sUsrObj obj(*user,sHiveId(q));

        sStr rawQCFile; obj.getFilePathname(rawQCFile,"%s","qcRaw.csv");
        sDic <idx> counts(0,sMex::fSetZero), quas(0,sMex::fSetZero) ;
        idx totCounts=0,totPhred=0, totCountsWN=0,totPhredWN=0;
        real gc=0,at=0;
        bool rawQC=false;

        if(rawQCFile.ok()) {
            sFil fl(rawQCFile,sMex::fReadonly);
            if(fl.ok()){
                rawQC=true;
                sTbl tbl;tbl.parse(fl.ptr(),fl.length());
                for (idx it=0; it<tbl.rows(); ++it) {
                    idx llet;char let=*(tbl.cell(it,(idx)0,&llet));
                    let=tolower(let);
                    if(strchr(possible,let)==0)continue;
                    idx cnt=tbl.ival(it,1), qua=tbl.ival(it,2);
                    *counts.set((const void*)&let,1)+=cnt;
                    *quas.set((const void*)&let,1)+=qua;
                    if(let=='g' || let=='c' || let=='a' || let=='t') {
                        totCounts+=cnt;
                        totPhred+=qua;
                    }
                    if(let=='g' || let=='c') gc+=cnt;
                    else if(let=='a' || let=='t') at+=cnt;
                    totCountsWN+=cnt;
                    totPhredWN+=qua;
        }}}


        JSNode nod=n.linkobj("#");

        sHiveseq hs(user,q);

        sBioseq::SeqQC * qc=hs.qcProfile();

        sBioseqSNP_refHead(nod, atoidx(q), obj.propGet("name"));
        nod.link("max_read_length",qc->maxLen);
        nod.link("min_read_length",qc->minLen);
        nod.link("max_duplicate_read",qc->maxRpt);
        nod.link("avg_read_length",qc->aveLenRpt);

        nod.link("num_reads",qc->cntRpt);
        nod.link("num_reads_unique",qc->cnt);
        JSNode cnt=nod.linkobj("bases");

        if(rawQC) {
            char TTC[24];strcpy(TTC,"count_X");
            char TTQ[24];strcpy(TTQ,"avg_quality_X");
            char TTP[24];strcpy(TTP,"percent_X");
            for ( idx icc=0, ll=sLen(possible) ; icc<ll; ++icc){
                char let=possible[icc];
                idx * pCC=counts.get((const void *)&let,1);if(!pCC) continue;
                idx cc=*pCC;if(!cc) continue;
                TTC[6]=let;
                TTQ[12]=let;
                TTP[8]=let;
                cnt.link(TTC,cc);
                cnt.link(TTQ,1.*(*(quas.get((const void *)&let,1)))/cc);
                cnt.link(TTP,100.*cc/totCountsWN);
            }

        } else {
            cnt.link("count_a",qc->countsRpt[0]);
            cnt.link("count_c",qc->countsRpt[1]);
            cnt.link("count_g",qc->countsRpt[2]);
            cnt.link("count_t",qc->countsRpt[3]);
            cnt.link("count_n",qc->countsRpt[4]);
            cnt.linkpercent("percent_a",100.*qc->countsRpt[0]/qc->numBasesRpt);
            cnt.linkpercent("percent_c",100.*qc->countsRpt[1]/qc->numBasesRpt);
            cnt.linkpercent("percent_g",100.*qc->countsRpt[2]/qc->numBasesRpt);
            cnt.linkpercent("percent_t",100.*qc->countsRpt[3]/qc->numBasesRpt);
            cnt.link("avg_quality_a",qc->phredsRpt[0]);
            cnt.link("avg_quality_c",qc->phredsRpt[1]);
            cnt.link("avg_quality_g",qc->phredsRpt[2]);
            cnt.link("avg_quality_t",qc->phredsRpt[3]);
            gc=qc->gcRpt;
        }

        if(rawQC) {
            nod.link("avg_phred_score",1.*totPhred/totCounts);
            nod.link("count_all",totCounts);
            nod.link("count_all_WN",totCountsWN);
        }else {
            nod.link("avg_phred_score",qc->aveQuaRpt);
            nod.link("count_all",qc->numBasesRpt);
        }
        nod.link("stdev_quality",sqrt(qc->quaDev2Rpt));
        nod.linkpercent("ngs_gc_content",100.*gc/(gc+at));

        workWithFile("_.qc2.ComplexityTable.csv");
        sscanf(cont,"Reads,Count\n"
            "Complex,%" DEC "\n"
            "Not Complex,%" DEC "\n"
            ,&qc->cntComplex
            ,&qc->cntNotComplex
            );


        nod.linkpercent("complexity_percent",100.*qc->cntComplex/(qc->cntComplex+qc->cntNotComplex));
        nod.linkpercent("non_complexity_percent",100.*qc->cntNotComplex/(qc->cntComplex+qc->cntNotComplex));

        workWithFile("_.qc2.codonQCTable.csv");
        sTbl tbl;tbl.parse(cont.ptr(), cont.length(),sTbl::fPreserveQuotes);
        for(    idx it=0; it<tbl.rows(); ++it) {
            idx len=tbl.len(it,0);
            const char * p=tbl.cell(it,(idx)0);
            if(len==sLen(cod.ptr()) && memcmp(cod.ptr(0),p,len)==0){
                qc->cntCoding=tbl.ivalue(it,2);
                qc->cntNonCoding=tbl.ivalue(it,1);
                break;
            }
        }
        nod.linkpercent("percent_coding",100.*qc->cntCoding/(qc->cntCoding+qc->cntNonCoding));
        nod.linkpercent("percent_non_coding",100.*qc->cntNonCoding/(qc->cntCoding+qc->cntNonCoding));
        nod.link("coding_system",cod.ptr());

        workWithFile("dna-alignx_screenResult.csv");
        sTbl t;t.parse(cont, cont.length());
        JSNode itax=nod.linkarr( "taxonomy");
        idx totHits=0;
        for(idx i=1; i<t.rows(); ++i){
            totHits+=atoidx(t.get(&nam,i,1));nam.cut(0);
        }

        if(totHits){
            for(idx i=1; i<t.rows(); ++i){
                const char * tn=t.get(&nam,i,(idx)0);if(!tn || !*tn)continue;
                JSNode to=itax.linkobj("#");
                    to.link( "taxid",tn);nam.cut(0);
                    to.linkpercent("hits",100.*atoidx(t.get(&nam,i,1))/totHits);nam.cut(0);
            }
        }
       nam.cut(0);
    }

    return 0;
}



idx DnaAlQCProc::OnExecute(idx req)
{
    sStr errmsg, buf;

    reqProgress(1, 1, 100);
    sVec < sHiveId > heptids, hexids, gmapids,refDirectIds;
    formHiveIdValues("heptagon_id",  &heptids, 0);
    hepts.cut(0);
    for(idx ihp=0; ihp<heptids.dim();++ihp) {
        Heptagon * hept=hepts.add(1);
        new (&(hept->obj)) sUsrObj(*user,heptids[ihp]);

        hept->obj.propGetHiveIds("parent_proc_ids",hexids);
        for(idx ihx=0; ihx<hexids.dim();++ihx) {
            Hexagon * hex =hept->hexs.add(1);
            new (&(hex->obj)) sUsrObj(*user,hexids[ihx]);

            new (&(hex->al)) sHiveal(user,hex->obj.IdStr());

            hex->obj.propGet00("query",&buf,";");
            hex->qry.parse(buf, sBioseq::eBioModeShort, false, user);buf.cut(0);

            hex->obj.propGet00("subject",&buf,";");
            hex->ref.parse(buf, sBioseq::eBioModeShort, false, user);buf.cut(0);

            hex->al.Sub=&(hex->ref);

        }
        hexids.cut(0);
    }
    formHiveIdValues("genemap_id",  &gmapids, 0);


    sStr buffRef, buffSrr;
    sHiveseq Refs(user, formValue("refs_direct_id", &buffRef));
    const char * srr = formValue("srr", &buffRef);

    if (srr && strncmp(srr, "0",1)==0){

      sFil regionsFp(ProcFile("regions.csv", false), sMex::fMapRemoveFile);

      regionsFp.printf("Reference,start,end\n");

      sVec<idx> regions;

      for (idx ir = 0; ir < Refs.dim(); ++ir) { 

          idx length = Refs.len(ir);
          const char *sequenceBits = Refs.seq(ir);
          const char *refName = Refs.id(ir);

          idx windowSize = 50;
          idx start = 0;
          idx N = 4;

          reqProgress(1, 2, 100);
          slidingWindow(sequenceBits, start, length, windowSize, N, refName, &regions, regionsFp);
          reqProgress(1, 4, 100);

          for (idx i = 0; i < regions.dim(); i += 2) {
              idx startPos = regions[i];
              idx endPos = regions[i + 1];
              regionsFp.printf("\"%s\",%lld,%lld\n", refName, startPos, endPos);
          }
          reqProgress(1, 5, 100);
      }
    }


    











    reqProgress(2, 20, 100);


    {
        sFil fbio(ProcFile("biosample.json"));
        formValue("bioSampleQC",&fbio,0);
        fbio.cut(-2);
    }

    sJson jAll;jAll.file(ProcFile("qcAll.json"));
    sJson jPos;jPos.file(ProcFile("qcPos.json"));
    sJson jNGS;jNGS.file(ProcFile("qcNGS.json"));
    sJson jAnnot;jAnnot.file(ProcFile("refAnnot.json"));
    sJson jMeta;jMeta.file(ProcFile("biosample-meta.json"));


    #define attachExternal(_v_json, _v_node,_v_name, _v_formname) {JSNode root=JSNode(&_v_json);_v_node=root.linkarr(_v_name);sJson jsx;sStr bbb;const char * jsonExternal=formValue(_v_formname,&bbb,0);if(jsonExternal){jsx.initMem(jsonExternal,sLen(jsonExternal));JSNode jsxroot(&jsx);jsxroot.path("$root.externalQC");if(jsxroot.isok)_v_node.copy(jsxroot);}}

    JSNode root=JSNode(&jAnnot);
    root.link("biosample",formValue("biosampleAcc"));
    root.link("shortReads",formValue("srr"));
    JSNode seqannot;attachExternal(jAnnot,seqannot,"seqannot","posAnnot");
    for(idx ihp=0; ihp<hepts.dim();++ihp) {
        Heptagon * hept=hepts.ptr(ihp);
        for(idx ihx=0; ihx<hept->hexs.dim();++ihx) {
            Hexagon * hex=hept->hexs.ptr(ihx);
            gbAnnotation(&(hex->ref), seqannot);
        }
    }

    reqProgress(3, 30, 100);

    sUsrObj contigMetrics(*user, "argos_Assm_Metrics");
    sUsrObj ngsMetrics(*user, "argos_Reads_Metrics");

    sJson contigJson, ngsJson;
      
    user->objJson(strtoidx(contigMetrics.IdStr(), nullptr, 10), &contigJson);
    user->objJson(strtoidx(ngsMetrics.IdStr(), nullptr, 10), &ngsJson);

    root=JSNode(&jAll);
    root.link("biosample",formValue("biosampleAcc"));
    root.link("shortReads",formValue("srr"));
    root.link("_id", contigJson.value("_id"));
    root.link("_type", contigJson.value("_type"));
    root.link("created", contigJson.value("created"));
    JSNode refseq;attachExternal(jAll,refseq,REFSEQ,"asmQC");
    
    
    
    root=JSNode(&jPos);
    root.link("biosample",formValue("biosampleAcc"));
    root.link("shortReads",formValue("srr"));
    JSNode refpos;attachExternal(jPos,refpos,SEQPOS,"posQC");

    const char * heptID = formValue("heptagon_id");
    bool heptOut = true;

    if(hepts.dim() >= 1){
      if (heptID && strncmp(heptID, "0",1)==0){
        heptOut = false;
      }
      if(heptOut){
        for(idx ihp=0; ihp<hepts.dim();++ihp) {
          referenceQC(hepts.ptr(ihp),refpos,refseq);
        }
      }
    }
    

    reqProgress(4, 50, 100);

    root=JSNode(&jNGS);
    root.link("biosample",formValue("biosampleAcc"));
    root.link("shortReads",formValue("srr"));
    root.link("_id", ngsJson.value("_id"));
    root.link("_type", ngsJson.value("_type"));
    root.link("created", ngsJson.value("created"));
    JSNode seqc;attachExternal(jNGS,seqc,NGSQC,"ngsQC");
    for(idx ihp=0; ihp<hepts.dim();++ihp) {
        Heptagon * hept=hepts.ptr(ihp);
        for(idx ihx=0; ihx<hept->hexs.dim();++ihx) {
            Hexagon * hex =hept->hexs.ptr(ihx);
            seqQC(hex,seqc);
        }
    }
    bool readMetrics = user->propSetJson(&jNGS);
    bool assmMetrics = user->propSetJson(&jAll);

    reqProgress(5, 80, 100);

    jAnnot.serialize();
    ::printf("%s\n",jAnnot.ret());
    jPos.serialize();
    ::printf("%s",jPos.ret());
    jAll.serialize();
    ::printf("%s\n",jAll.ret());
    jNGS.serialize();
    ::printf("%s\n",jNGS.ret());




    



    

    











    if( !errmsg ) {
        reqProgress(0, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    } else {
        logOut(eQPLogType_Error, "%s\n", errmsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }

    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc, argv);
    DnaAlQCProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-alqc", argv[0]));
    return (int) backend.run(argc, argv);
}

