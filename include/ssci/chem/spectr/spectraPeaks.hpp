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
#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>
#include <ssci/math.hpp>
#include <ssci/chem.hpp>

namespace slib{
    class spectraPeaks : public sSpctrMolecule
    {
        public:
            spectraPeaks(){
                sSet(&peaks);
                sSet(&collect);
            }
            ~spectraPeaks(){
                cleanTmpFiles();
            }
            struct satellite_peaks {
                    idx shift;
                    real intensity;
            };


            struct minMax_satellite_peaks {
                    idx shift;
                    real min_intensity;
                    real max_intensity;
            };

            struct Peaks
            {
                    real threshold;
                    real shiftIsoPeaks;
                    idx maxPeaks;
                    idx dogenerate;
                    sVec < satellite_peaks > satellite;
                    sVec < minMax_satellite_peaks > minMax_satellite;

                    idx satelliteControlI;
                    idx outputPeakfile;
                    idx scaleOut;

                    real widerLeftPeak, widerRightPeak;
                    idx showStage;
            } peaks;

            struct Collect
            {
                    char pmajfiles[1024];
                    idx column, join;
                    sStr exclpeaks, inclpeaks, exclfiles;
                    sStr alwaysPeaks;

                    real wobble, wobbleExcl;
                    char colout[128], rowout[128];
                    real wobbleAlwaysDetect;
                    idx doPCA, renormalize;
            } collect;

            static satellite_peaks * peakGroup;

            struct knownPeak
            {
                idx iMol, iGrp, iMatch, iMolOri;
                real coef, itgrl, rmass, itgrl0, shiftOri;
                real firstIntensity;
                real sats[7], fint[7];
                real maxAmplitude;
                real matchPosX, matchPosY;

                knownPeak(idx lmol = 0, idx lgrp = 0, idx limatch = 0, real lcoef = 0, real litgrl = 0, real lrmass = 0, real lmaxalt = 0, real lfirstIntensity = 0)
                {
                    sSet(sats);
                    sSet(fint);
                    iMol = lmol;
                    iGrp = lgrp;
                    iMatch = limatch;
                    coef = lcoef;
                    itgrl = itgrl0 = litgrl;
                    rmass = lrmass;
                    maxAmplitude = lmaxalt;
                    firstIntensity = lfirstIntensity;
                    iMolOri = matchPosX = matchPosY = shiftOri = 0;
                }
                ~knownPeak()
                {
                }
                bool operator >(knownPeak &o)
                {
                    return (rmass + (peakGroup+ iGrp)->shift > o.rmass + (peakGroup + o.iGrp)->shift);
                }
                bool operator <(knownPeak &o)
                {
                    return (rmass + (peakGroup+iGrp)->shift < o.rmass + (peakGroup + o.iGrp)->shift );
                }
                bool operator >=(knownPeak &o)
                {
                    return (rmass + (peakGroup + iGrp)->shift >= o.rmass + (peakGroup+o.iGrp)->shift );
                }
                bool operator <=(knownPeak &o)
                {
                    return (rmass + (peakGroup + iGrp)->shift <= o.rmass + (peakGroup+o.iGrp)->shift );
                }

            };
            idx peaksDetect(const char * nam, const char * srcsfx, sVec<knownPeak> * kpl, sVec<knownPeak> * kpl2, real allStp);
            idx peakeDetectSingle(sVec<knownPeak> & kpl, idx imol, idx cntBin, real smin, real smax, real * d, real * bins, real * p, idx grpStart, idx cntGrps, real threshold, bool takeorphans, knownPeak * kParent, int runnumber, sVec<
                real> * alwaysDetect, real allStp);
            idx prepareIsopeakBins(idx imol, real * bins, idx cnt, real stp, real minx, real maxx, idx & leftbin, idx & rightbin, idx & topBin, idx & firstMaxBin);
            idx peaksGenerateKnown(const char * path, real allStp, real smin = 0, real smax = 10000, bool doredo = false );

            void peakKnownOut(const char * nam, sVec<knownPeak> & kpl, const char * flnm, real smin, real * d, real * r, real * s, real * p, real * u, idx cnt, idx istart, idx * sortord, real allStp);
            void cleanTmpFiles(void){
              for (idx ii=0;ii<m_tmpFiles.dim();++ii){ sFile::remove(m_tmpFiles[ii]);}
            };
        private:
               sVec <sStr> m_tmpFiles;
    };

}



