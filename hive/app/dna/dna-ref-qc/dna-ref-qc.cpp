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
#include <violin/violin.hpp>
#include <slib/std/file.hpp>
#include <ssci/chem/spectr/spectraFile.hpp>
#include <ssci/chem/spectr/spectraPeaks.hpp>
#include <ssci/math/nr/nrutil.h>

#if _DEBUG
  idx debug=1;
#else
  idx debug=0;
#endif

class DnaRefQCProc: public sQPrideProc
{
    public:
        DnaRefQCProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
  
        }
        struct StatResult {
    const char *ref;
    idx startPos;
    idx endPos;
    idx in;
    sVec<real> shanWindows;
};
        virtual idx OnExecute(idx);
  static real ShannonCalc( const sVec <idx> &map, idx seqLength, idx N);
  static idx slidingWindow(const char *seqBits, idx startPos, idx length, idx windowSize, idx N, const char *ref, sVec <idx> *regions, sFil &regionsFp);
  static idx computeStat(const char *seqBits, idx startPos, idx endPos, idx windowSize, idx N, const char *ref, StatResult *res, sVec<sVec<real> > *shanAgg);
  static idx bitToChar(const sVec <idx> &map, idx N);
  static idx detectLowQuality(const sVec<real> *shanWindows, idx N_mer, const char *ref, sVec <idx> *regions, sFil &regionsFp);
};

class reportRefQC {

  public:
    struct regionsData
    {
      const char *ref;
      idx start;
      idx end;
    };
    
    reportRefQC(sviolin::sHiveseq &Refs);
    sVec<regionsData> getRegions(sFil &regionsFp);
  
  private:
    sviolin::sHiveseq &References;
    
};

 reportRefQC::reportRefQC(sviolin::sHiveseq &Refs) : References(Refs) {
  
}

sVec<reportRefQC::regionsData> reportRefQC::getRegions(sFil &regionsFp){

  sVec<regionsData> allRegions;

  for ( idx ir=0; ir<References.dim(); ++ir ) {
    
    idx length = References.len(ir);
    const char * sequenceBits=References.seq(ir); 
    const char * refName=References.id(ir);
    idx windowSize = 20;
    idx start = 0;
    idx n = 4;
    sVec<idx> region;

    DnaRefQCProc::slidingWindow(sequenceBits, start, length, windowSize, n, refName, &region, regionsFp);

    for (idx i = 0; i < region.dim(); i += 2) {
      regionsData rd;
      rd.ref = refName;
      rd.start = region[i];
      rd.end = region[i + 1];
      allRegions.vadd(1, rd);
    }

  }

  return allRegions;

}






real DnaRefQCProc::ShannonCalc(const sVec <idx> &map, idx seqLength, idx N){
  
  
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


idx DnaRefQCProc::computeStat(const char *seqBits, idx startPos, idx endPos, idx windowSize, idx N, const char *ref, StatResult *res, sVec<sVec<real> > *shanAgg) {
  
  for (idx in = 1; in <= N; ++in) {
    sVec<idx> windowCounts(sMex::fSetZero);
    idx combinations = 1 << (2 * in);
    windowCounts.add(combinations);

    for (idx i = startPos; i <= endPos - in + 1; ++i) {
      idx combinedLet = 0;

      for (idx nmer = 0; nmer < in; ++nmer) {
        char letter = sBioseqAlignment::_seqBits(seqBits, i + nmer, 0);
        combinedLet = (combinedLet << 2) | letter;
      }
      windowCounts[combinedLet]++;
    }
    
    real shanWindow = ShannonCalc(windowCounts, windowSize, in);
    res->shanWindows[in-1] = shanWindow;
  
    if(shanAgg->dim() < in){
      shanAgg->resize(in);
    }

    sVec<real> *shan = shanAgg->ptr(in-1);
    real *val = shan->add(1);
    *val = shanWindow;
  }

  return 0;
}

idx DnaRefQCProc::detectLowQuality(const sVec<real> *shanWindows, idx N_mer, const char *ref, sVec <idx> *regions, sFil &regionsFp) {
    
    
  bool inRegion = false;
  idx startPos = 0;
  idx endPos = 0;
  sVec<real> diff1(sMex::fSetZero);
  sVec<real> diff2(sMex::fSetZero);

  diff1.resize(shanWindows->dim());
  diff2.resize(shanWindows->dim());

  for (int i = 1; i < shanWindows->dim(); ++i) {
    diff1[i] = (*shanWindows)[i] - (*shanWindows)[i-1];
  }

  for (int j = 1; j < diff1.dim(); ++j) {
    diff2[j] = diff1[j] - diff1[j-1];
    
      printf("%f ", diff2[j-1]);
    
    
  }

  real minVal = 1.05 - (0.2 * (N_mer + 1));

  for (idx pos = 1; pos < diff2.dim(); ++pos) {
    bool inflection = diff2[pos] * diff2[pos-1] < 0;

    if (inflection) {
      if (inRegion) {
        real localMin = (*shanWindows)[startPos];
        idx localMinPos = startPos;
        for (idx m = startPos; m <= endPos; ++m) {
          if ((*shanWindows)[m] < localMin) {
            localMin = (*shanWindows)[m];
            localMinPos = m;
          }
        }

        if (localMinPos > 0 && localMinPos < shanWindows->dim() - 1) {
          if (localMin < (*shanWindows)[localMinPos - 1] && localMin < (*shanWindows)[localMinPos + 1]) {
            if (localMin < minVal && startPos != endPos) {

              regions->add(2);
              (*regions)[regions->dim() - 2] = startPos;
              (*regions)[regions->dim() - 1] = endPos;
              
            }
          }
        }

        inRegion = false;

      }
      inRegion = true;
      startPos = endPos = pos;
            
    } else if (inRegion) {
      endPos = pos;
    } 
  }


  return 0;
}

idx DnaRefQCProc::slidingWindow(const char *seqBits, idx startPos, idx length, idx windowSize, idx N, const char *ref, sVec <idx> *regions, sFil &regionsFp) {

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

    

    windowStart++;
    windowEnd++;

    
  }



  for (idx b = 0; b < N; ++b) {

    idx padSize = shanAgg[b].dim() * 0.06;

    real shanMax = shanAgg[b][0];
    for (idx c = 0; c < shanAgg[b].dim(); ++c) {
      if (shanAgg[b][c] > shanMax) {
        shanMax = shanAgg[b][c];
      }
    }

    for (idx buff = 0; buff < padSize; ++buff) {
      shanAgg[b].insert(0, 1);
      shanAgg[b][0] = shanMax;
      shanAgg[b].insert(shanAgg[b].dim(), 1);
      shanAgg[b][shanAgg.dim()-1] = shanMax;
    }
  }

  for (idx i = 0; i < N; ++i) {

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

    detectLowQuality(&shanAgg[i], i, ref, regions, regionsFp);
    
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





idx DnaRefQCProc::OnExecute(idx req)
{
  

  sStr sequence;

  sHiveseq Refs(user,formValue("references"));

  reportRefQC report(Refs);
  sFil regionsFp(ProcFile("regions.csv", false), sMex::fMapRemoveFile);
  regionsFp.printf("Referece,start,end\n");

  sVec<reportRefQC::regionsData> lowComplexityRegions = report.getRegions(regionsFp);

  for (idx i = 0; i < lowComplexityRegions.dim(); ++i) {
    printf("Reference: %s, Start: %lld, End: %lld\n", lowComplexityRegions[i].ref, lowComplexityRegions[i].start, lowComplexityRegions[i].end);
  }
    
  return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc,argv);
    DnaRefQCProc bacinend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-ref-qc",argv[0]));
    return (int)bacinend.run(argc,argv);
}

