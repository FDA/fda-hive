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
#include <ssci/chem/spectr/spectraMolecule.hpp>

namespace slib {
    class sSpctr : public sSpctrMolecule
    {
        public:

            sSpctr(const char * nam)
        : sSpctrMolecule()
            {
                allStp = allMax = dualOutput = calibration = sMin= 0;
                sSet(&misc);
                sSet(&savgol);
                sSet(&wavlet);
                sSet(&fft);
                sSet(&baseline);
                sSet(&filter);
                sSet(&lda);
                sSet(&coding);
                m_filename.printf(0,"%s", nam ? nam : "");
            }

            ~sSpctr() {
               cleanTmp();
            }


            bool miniMax(idx * pCnt, real * pMin, real * pMax);
            idx binData( idx cntBin, real * pshift, real smin, real shiftIsoPeaks);
            idx smoothSavGol( const char * src, const char * dst);
            idx smoothWavelett(const char * srcsfx, const char * dstsfx);
            idx fftCutoff( const char * srcsfx, const char * dstsfx);
            idx computeBaseline(const char * srcsfx, const char * dstsfx);
            idx normalize(const char * srcsfx, const char * dstsfx);
            void doDetectPeaks(idx param, idx interactWithInterface = true);

            real allStp, allMax;

            sVec <idx> allTopI;

            idx dualOutput;
            idx calibration;
            sStr inpMolList;
            sStr catList;
            real sMin;
            sStr resultPathPrefix;
            struct Misc
            {
                    char excel[1024];
            } misc;
            struct SavGol
            {
                    idx left, right, degree;
            } savgol;
            struct Wavlet
            {
                    idx daubNum;
                    real fracPercent;
            } wavlet;
            struct FFT
            {
                    idx daubNum, fftMin, fftMax, fftMinLog, fftMaxLog;
            } fft;
            struct Baseline
            {
                    idx maxFreq;
                    real thresholdPercent;
            } baseline;

            struct Filter
            {
                    idx iCateg;
                    real filterFrequency, filterIntensity, filterTProb;
                    idx andor1, andor2;
            } filter;
            struct LDA
            {
                    idx iCateg;
                    idx ax1, ax2, ax3;
                    real regulAlpha;
                    idx showMesh;
                    idx meshTransparency, symbolSize;
                    idx meshSteps, meshColoration;
                    real meshDiffusion;
                    idx maxIter, bootSpace;
            } lda;

            struct Coding
            {
                    sStr learn, check;
            } coding;

            struct rMassPrf
            {
                    real rMass, rPrfx;
            };
            struct clsPeak
            {
                    real intensity;
                    idx iNum, Rpt;
                    rMassPrf rmp;
            };
            struct clsFile
            {
                    idx cplistIdx, Rpt;
            };

            sStr m_filename;


            static void setDefaultParams(const char * config_lst)
            {
                _config_lst = config_lst;
            }
            bool inputParams(const char * configFile);

            void cleanTmp(void){
                for (idx ii=0;ii<m_tmpFiles.dim();++ii){ sFile::remove(m_tmpFiles[ii]);}
            };

        private:

            sVec<sStr> m_tmpFiles;
            static const char * _config_lst;
    };

}
;

