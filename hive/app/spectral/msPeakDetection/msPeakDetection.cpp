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
#include <ulib/ufile.hpp>

#include <ssci/chem/spectr/spectraFile.hpp>
#include <ssci/chem/spectr/spectraPeaks.hpp>

class spectraPeakDetectionProc: public sQPrideProc
{
    public:
        spectraPeakDetectionProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
        void populateMinMaxSatellite(spectraPeaks & gSpctrPks, const sUsrObjPropsTree * objPropsTree);
        void populateCalibPeaks(sSpctr & gSpctr,const  sUsrObjPropsTree * objPropsTree);
};

void spectraPeakDetectionProc::populateCalibPeaks(sSpctr & gSpctr, const sUsrObjPropsTree * objPropsTree) {
    const sUsrObjPropsNode * calib_peak_node = objPropsTree->find("Spectrum");

    if( calib_peak_node ) {
        for (const sUsrObjPropsNode * calib_peak = calib_peak_node->firstChild("topPeakNum"); calib_peak ; calib_peak = calib_peak ->nextSibling("topPeakNum")) {
                gSpctr.allTopI.vadd(1, calib_peak->ivalue());
        }
    }


}

void spectraPeakDetectionProc::populateMinMaxSatellite (spectraPeaks & gSpctrPks, const sUsrObjPropsTree * objPropsTree) {

    const sUsrObjPropsNode * satellite_peaks = objPropsTree->find("peakShape");
    if( satellite_peaks ) {
        for (const sUsrObjPropsNode * minMaxInt = satellite_peaks->firstChild("satelites"); minMaxInt ; minMaxInt = minMaxInt ->nextSibling("satelites")) {

            const sUsrObjPropsNode * minIntNode= minMaxInt->find("min_satelite_intensity");
            const sUsrObjPropsNode * maxIntNode= minMaxInt->find("max_satelite_intensity");
            const sUsrObjPropsNode * shiftNode= minMaxInt->find("satelite_shift");

            spectraPeaks::minMax_satellite_peaks * mm = gSpctrPks.peaks.minMax_satellite.add();
            mm->min_intensity = minIntNode->rvalue();
            mm->max_intensity = maxIntNode->rvalue();
            mm->shift = shiftNode->ivalue();

        }

    }

}


idx spectraPeakDetectionProc::OnExecute(idx req)
{
    bool isValid = true;
    sStr fileListList;
    sString::searchAndReplaceSymbols(&fileListList, formValue("SpectraFile"), 0, ";\n", 0, 0, true, true, true, true);
    sStr library;
    sString::searchAndReplaceSymbols(&library, formValue("SpectraLibrary"), 0, ";\n", 0, 0, true, true, true, true);
    sVec<sStr> objFilePathList;
    idx selfDir = formIValue("selfDir",0);

    const sUsrObjPropsTree * objPropsTree=objs[0].propsTree();

    for(const char * p = fileListList.ptr(); p; p = sString::next00(p)) {
        sStr destination; destination.cut(0);
        sHiveId objId(p);
        sUsrFile obj(objId, user);
        sHiveId libId(sString::next00(library,0));
        sUsrFile objLib(libId, user);
        if( obj.Id() && objLib.Id() ) {
            sStr * d = objFilePathList.add();
            obj.getFile(*d);

            const char * flname = d->ptr();

            sSpctr gSpctr(flname);
            spectraPeaks gSpctrPks;
            sSpctrMolecule gSpctrMol;

            sStr pathMol;
            objLib.getFile(pathMol);
            sFil molFile(pathMol,sMex::fReadonly);

            gSpctr.inputMolecules(molFile,gSpctr.inpMolList);
            if( selfDir == 1 ) {
                const char * empty_key = "";
                obj.getFilePathname(gSpctr.resultPathPrefix, empty_key);
                gSpctr.resultPathPrefix.printf("%s-ms", obj.Id().print());
            } else {
                reqAddFile(gSpctr.resultPathPrefix, "%s-ms", obj.Id().print());
            }

            sChem::element::initTable();

            idx ret = -1;

            gSpctr.dualOutput = 0;

            gSpctr.allStp = formRValue("Resolution");
            gSpctr.calibration = 0;

            populateCalibPeaks(gSpctr, objPropsTree);

            gSpctr.savgol.left = formIValue("savGolLeft");
            gSpctr.savgol.right = formIValue("savGolRight");
            gSpctr.savgol.degree = formIValue("savGolDegree");

            gSpctr.wavlet.daubNum = formIValue("Daubechies");
            gSpctr.wavlet.fracPercent = formRValue("DaubechiesFilter");

            gSpctr.fft.daubNum = formIValue("daubechiesFastFourier");
            gSpctr.fft.fftMin = formIValue("minFastFourier");
            gSpctr.fft.fftMax = formIValue("maxFastFourier");

            gSpctr.baseline.maxFreq = formIValue("maxFrequencyBaseline");
            gSpctr.baseline.thresholdPercent = formRValue("thresholdBaseline");

            gSpctrPks.peaks.threshold = formRValue("thresholdPeakDetection");
            gSpctrPks.peaks.maxPeaks = formIValue("maxPeakCount");
            gSpctrPks.peaks.dogenerate = formIValue("findUnknownPeak");
            gSpctrPks.peaks.showStage = formIValue("showOutcome");
            gSpctrPks.peaks.outputPeakfile = formIValue("outPutPeakFile");
            gSpctrPks.peaks.scaleOut = formIValue("scaleOut");
            gSpctrPks.peaks.shiftIsoPeaks = formRValue("shiftIsoPeaks");

            gSpctrPks.peaks.widerLeftPeak = formIValue("widerLeftPeak");
            gSpctrPks.peaks.widerRightPeak = formIValue("widerRightPeak");
            gSpctrPks.peaks.satelliteControlI = formIValue("satelliteControlNum");

            populateMinMaxSatellite(gSpctrPks,objPropsTree);



                idx cntBin=0;
                real sMin=0, sMax=0, shift=0;
                isValid = gSpctr.miniMax(&cntBin, &sMin, &sMax);
                if( isValid ) {
                    ret = -1;
                    ret = gSpctr.binData(cntBin, &shift, sMin, gSpctrPks.peaks.shiftIsoPeaks);
                }

                if( ret ) {
                    ret = -1;
                    ret = gSpctr.smoothSavGol("bin", "savgol");
                }

                if( ret ) {
                    ret = -1;
                    ret = gSpctr.smoothWavelett("savgol", "wavlet");
                }

                if( ret ) {
                    ret = -1;
                    ret = gSpctr.computeBaseline("wavlet", "nobaseline");
                }

                sFilePath pathGKn(gSpctr.resultPathPrefix,"%%dir/");

                if( ret ) {
                    ret = -1;

                    ret = gSpctrPks.peaksGenerateKnown(pathGKn , gSpctr.allStp, 0, 10000, false);
                    if( ret ) {
                        sFilePath path1(gSpctr.resultPathPrefix, "%%dir/spectra.iso1peaks");
                        sVec<spectraPeaks::knownPeak> Kpl(path1.ptr());
                        Kpl.cut(0);

                        sFilePath path2(gSpctr.resultPathPrefix, "%%dir/spectra.iso2peaks");
                        sVec<spectraPeaks::knownPeak> Kpl2(path2.ptr());
                        Kpl2.cut(0);

                        ret = gSpctrPks.peaksDetect(gSpctr.resultPathPrefix, "nobaseline", &Kpl, &Kpl2, gSpctr.allStp);
                    }
                }




                if( selfDir == 1 ) {
                    obj.cast("spectra-MS");
                }
         }
    }

    sStr out;

    reqProgress(0, 99, 100);
    reqSetData(req, "output.csv", &out);
    reqSetStatus(req, eQPReqStatus_Done);
    reqProgress(0, 100, 100);
    return 0;
}

extern const char * config_lst;

int main(int argc, const char * argv[])
{
    sStr tmp;

    sApp::args(argc, argv);
    sSpctr::setDefaultParams(config_lst);

    spectraPeakDetectionProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "spectraPeakDetection", argv[0]));
    return (int) backend.run(argc, argv);
}

