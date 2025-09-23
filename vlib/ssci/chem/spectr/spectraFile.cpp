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
#include <math.h>
#include <ssci/chem/spectr/spectraFile.hpp>
#include <stdio.h>

bool sSpctr::miniMax(idx * pCnt, real * pMin, real * pMax)
{
    real intensity;
    char buf[1024];
    FILE * fp = fopen(m_filename, "r");
    if( !fp )
        return false;
    while( !feof(fp) ) {
        fgets(buf, sizeof(buf), fp);
        if( sscanf(buf, "%lf %lf\n", pMin, &intensity) == 2 )
            break;
    }
    fseek(fp, -((idx) sizeof(buf)), SEEK_END);
    *pMax = -REAL_MAX;
    while( fgets(buf, sizeof(buf), fp) ) {
        real mmax = 0;
        sscanf(buf, "%lf", &mmax);
        if( *pMax < mmax )
            *pMax = mmax;
    }
    fclose(fp);

    *pMin = floor(*pMin);
    *pMax = ceil(*pMax);
    real bbs = (*pMax - *pMin) / (allStp);
    *pCnt = (idx) bbs + 1;

    return true;
}

idx sSpctr::binData(idx cntBin, real * pshift, real smin, real shiftIsoPeaks)
{

    sSpectrMS::isoDistribution::absShiftAll = shiftIsoPeaks;

    sStr * dstnamBin = m_tmpFiles.add(1);
    dstnamBin->printf("%s.%s",resultPathPrefix.ptr(), "bin");

    sMin = smin;
    sVec<real> dst(sMex::fSetZero, dstnamBin->ptr());

    dst.cut(0);

    real * d = dst.add(cntBin);
    dst.set(0);
    sStr * dstnamX = m_tmpFiles.add(1);
    dstnamX->printf(0,"%s.%s",resultPathPrefix.ptr(), "x");

    sVec<real> xx(dstnamX->ptr());
    xx.cut(0);
    xx.resize(cntBin);


    sVec<real> ox, oy;
    real daltons, intensity, minx = REAL_MAX, maxx = -REAL_MAX;
    sVec <real> calTop; calTop.resize(allTopI.dim());

    idx rows = 0;
    sVec <idx> itop; itop.resize(calTop.dim());


    for(idx it = 0; it < calTop.dim() ; ++it) {
        calTop[it] = IsoD.isoDistr[allTopI[it] - 1].getMass();
        itop[it] = 0;
    }

    sSort::sort(calTop.dim(), calTop.ptr(0));

    char buf[1024];
    FILE * fp = fopen(m_filename, "r");
    if( !fp )
        return false;

    real maxInt = 0;
    while( fgets(buf, sizeof(buf), fp) ) {
        if( sscanf(buf, "%lf %lf", &daltons, &intensity) != 2 )
            continue;

        ox.vadd(1, daltons);
        oy.vadd(1, intensity);
        if( daltons < minx )
            minx = daltons;
        if( daltons > maxx )
            maxx = daltons;

        for(idx it = 0; it < calTop.dim() ; ++it) {
            if( daltons >= calTop[it] - 0.5 && daltons <= calTop[it] + 0.5 ) {
                if( itop[it] == 0 || intensity > oy[itop[it]] )
                    itop[it] = rows;
                break;
            }
        }

        maxInt = sMax(maxInt, intensity);
        ++rows;
    }

    if( rows == 0 ) {
        return -1;
    }

    idx cntTop = 0, iMaxTop = 0;
    sVec <real> aTop, coefs;
    aTop.resize(calTop.dim() ); coefs.resize(calTop.dim());


    for(idx it = 0; it < calTop.dim() ; ++it) {
        if( calTop[it] == 0 )
            continue;
        if( oy[itop[it]] < 0.1 * maxInt ) {
            continue;
        } else{
        }
        itop[cntTop] = itop[it];
        aTop[cntTop] = calTop[it];
        coefs[cntTop] = 0;

        real ax = oy[itop[cntTop]];
        real bx = oy[itop[iMaxTop]];
        if( cntTop && oy[itop[cntTop]] > oy[itop[iMaxTop]] )
            iMaxTop = cntTop;
        ax += bx;
        ++cntTop;
    }
    if( cntTop == 0 ){
    }
    fclose(fp);
    if( pshift )
        *pshift = 0;

    if( cntTop ) {
        real shift = 0;
        if( cntTop ) {
            if( calibration == 0 ) {
                shift = aTop[iMaxTop] - ox[itop[iMaxTop]];
                for(idx is = 0; is < ox.dim(); ++is)
                    ox[is] += shift;
                if( pshift )
                    *pshift = shift;

            } else {
                for(idx is = 0, it = 0; is < ox.dim(); ++is) {
                    for(it = 0; it < cntTop && is >= itop[it]; ++it)
                        ;
                    if( it )
                        --it;
                    if( !coefs[it] )
                        coefs[it] = aTop[it] / ox[itop[it]];
                    ox[is] *= coefs[it];
                }
            }
        }
    }

    sVec<real> secDeriv;
    real * sd = secDeriv.add(2 * rows);
    sFunc::spline::secDeriv(ox, oy, rows, REAL_MAX, REAL_MAX, sd, sd + rows);
    for(idx is = 0; is < cntBin; ++is) {
        xx[is] = smin + is * allStp;
        dst[is] = sFunc::spline::calc(ox, oy, sd, rows, xx[is], 0);
        if( dst[is] < 0 )
            dst[is] = 0;
    }



    for(idx ib = 0; ib <= (idx) (1. / allStp); ++ib) {
        d[ib] = 0;
        d[cntBin - 1 - ib] = 0;
    }

    real rMax = -REAL_MAX;
    for(idx is = 0; is < cntBin; ++is) {
        rMax = sMax(rMax, dst[is]);
    }

    rMax = 1000 / rMax;
    for(idx is = 0; is < cntBin; ++is)
        d[is] *= rMax;

    sStr dstCSV("%s%s",resultPathPrefix.ptr(), "-bin.csv");
    sFil myContent(dstCSV);
    for (idx i=0;i<dst.dim();i++){
        myContent.printf("%.2lf,%.2lf\n",smin+allStp*i,dst[i]);
    }

    return rows;
}

idx sSpctr::smoothSavGol(const char * srcsfx, const char * dstsfx)
{
    sStr srcnam("%s.%s",resultPathPrefix.ptr(),srcsfx);

    sVec<real> src((const char*) srcnam);
    real * s = src.ptr();
    idx cnt = src.dim();
    if( !cnt )
        return -1;

    sStr * dstnam = m_tmpFiles.add(1);
    dstnam->printf("%s.%s",resultPathPrefix.ptr(), dstsfx);
    sVec<real> dst(sMex::fSetZero, dstnam->ptr());
    dst.cut(0);
    real * d = dst.add(cnt);
    dst.set(0);

    sMathNR::SavGol savgolB(cnt, savgol.left, savgol.right, savgol.degree);

    savgolB.smooth(d, s);

    for(idx is = 0; is < cnt; ++is)
        if( dst[is] < 0 )
            dst[is] = 0;

    sStr dstCSV("%s-%s.csv",resultPathPrefix.ptr(), dstsfx);
    sFil myContent(dstCSV);
    for (idx i=0;i<dst.dim();i++){
        myContent.printf("%.2lf,%.2lf\n",sMin+allStp*i,dst[i]);
    }
    return dst.dim();
}

idx sSpctr::smoothWavelett(const char * srcsfx, const char * dstsfx)
{
    sStr srcnam("%s.%s", resultPathPrefix.ptr(), srcsfx);

    sVec<real> src(srcnam.ptr());
    real * s = src.ptr();
    idx cnt = src.dim();
    if( !cnt )
        return -1;

    sStr * dstnam = m_tmpFiles.add(1);
    dstnam->printf("%s.%s", resultPathPrefix.ptr(), dstsfx);
    sVec<real> dst(sMex::fSetZero, dstnam->ptr());
    dst.cut(0);
    real * d = dst.add(cnt);
    dst.set(0);

    idx k, is;

    sMathNR::pwtset(wavlet.daubNum);

    for(k = 1; k < cnt; k <<= 1)
        ;
    real * coef = (real *) sNew(2 * (k + 1) * sizeof(real));
    real * unscoef = coef + k + 1;
    memcpy(&coef[1], s, sizeof(real) * cnt);
    memset((void*) (coef + 1 + cnt), 0, sizeof(real) * (k - cnt));

    sMathNR::wt1(coef, k, 1, sMathNR::pwt);

    for(is = 1; is <= k; ++is)
        unscoef[is] = (coef[is] < 0) ? -coef[is] : coef[is];
    real thresh = sMathNR::select((idx) ((1.0 - wavlet.fracPercent * 0.01) * k), k, unscoef);
    for(is = 1; is < k; is++) {
        if( fabs(coef[is]) <= thresh )
            coef[is] = 0.0;
    }

    sMathNR::wt1(coef, k, -1, sMathNR::pwt);
    memcpy(d, (void*) (coef + 1), sizeof(real) * cnt);

    for(idx is = 0; is < cnt; ++is)
        if( dst[is] < 0 )
            dst[is] = 0;

    sStr dstCSV("%s-%s.csv",resultPathPrefix.ptr(), dstsfx);
    sFil myContent(dstCSV);
    for (idx i=0;i<dst.dim();i++){
        myContent.printf("%.2lf,%.2lf\n",sMin+allStp*i,dst[i]);
    }

    sDel(coef);
    return dst.dim();
}

idx sSpctr::fftCutoff(const char * srcsfx, const char * dstsfx)
{
    sStr srcnam("%s.%s",resultPathPrefix.ptr(), srcsfx);
    sVec<real> src(srcnam.ptr());
    real * s = src.ptr();
    idx cnt = src.dim();
    if( !cnt )
        return -1;

    sStr * dstnam = m_tmpFiles.add(1);
    dstnam->printf("%s.%s",resultPathPrefix.ptr(), dstsfx);
    sVec<real> dst(sMex::fSetZero, dstnam->ptr());
    dst.cut(0);
    real * d = dst.add(cnt);
    dst.set(0);

    sMathNR::fftCutoff(s, d, cnt, fft.daubNum, (idx) pow(10., (real) fft.fftMinLog), (idx) pow(10., (real) fft.fftMaxLog));

    return dst.dim();
}

idx sSpctr::computeBaseline(const char * srcsfx, const char * dstsfx)
{
    sFilePath flnm;
    flnm.printf(0,"%s.%s",resultPathPrefix.ptr(),srcsfx);
    sVec<real> src(flnm.ptr());

    real * s = src.ptr();
    idx cnt = src.dim();
    if( !cnt )
        return -1;

    sStr * flnmBasePoints = m_tmpFiles.add(1);
    flnmBasePoints->printf(0,"%s.%s",resultPathPrefix.ptr(), "basepoints");
    sVec<real> dstb(sMex::fSetZero, flnmBasePoints->ptr());

    dstb.cut(0);
    real * db = dstb.add(cnt);
    dstb.set(0);

    sStr * flnmBaseLine = m_tmpFiles.add(1);
    flnmBaseLine->printf(0,"%s.%s",resultPathPrefix.ptr(), "baseline");
    sVec<real> dstf(sMex::fSetZero, flnmBaseLine->ptr());

    dstf.cut(0);
    real * df = dstf.add(cnt);
    dstf.set(0);

    sStr dstnam3("%s.%s",resultPathPrefix.ptr(), "nobaseline");

    flnm.printf(0,"%s.%s",resultPathPrefix.ptr(), dstsfx);
    sVec<real> dstn(sMex::fSetZero, flnm.ptr());

    dstn.cut(0);
    real * dn = dstn.add(cnt);
    dstn.set(0);


    sVec<real> xx;
    real * x = xx.add(cnt);
    sVec<real> yy;
    real * y = yy.add(cnt);
    sVec<real> secDeriv;
    real * sd = secDeriv.add(2 * cnt);
    idx is, id = 0;
    real smin = 0;
    for(is = 1; is < cnt - 1; ++is) {
        if( s[is] < s[is - 1] && s[is] < s[is + 1] ) {
            y[id] = s[is];
            x[id] = smin + is * allStp;
            ++id;
        }
    }
    if( id < 5 ) {
        return -1;
    }

    sFunc::spline::secDeriv(x, y, id, REAL_MAX, REAL_MAX, sd, sd + id);
    for(is = 0; is < cnt; ++is) {
        db[is] = sFunc::spline::calc(x, y, sd, id, smin + is * allStp, 0);
    }
    for(idx is = 0; is < cnt; ++is)
        if( db[is] < 0 )
            db[is] = 0;

    real * d = db;


    if( baseline.maxFreq != 0 ) {
        sMathNR::fftCutoff(db, df, cnt, 0, 0, baseline.maxFreq);
        d = df;
    }

    real maxH = 0;
    for(is = 0; is < cnt; ++is) {
        if( s[is] > maxH )
            maxH = s[is];
    }
    for(is = 0; is < cnt; ++is) {
        dn[is] = s[is] - d[is];
        if( dn[is] < baseline.thresholdPercent * 0.01 * maxH )
            dn[is] = 0;
    }

    for(idx is = 0; is < cnt; ++is)
        if( dn[is] < 0 )
            dn[is] = 0;
    sStr dstbCSV("%s-%s.csv",resultPathPrefix.ptr(), "basepoints");
       sFil myContentb(dstbCSV);
       for (idx i=0;i<dstb.dim();i++){
           myContentb.printf("%.2lf,%.2lf\n",sMin+allStp*i,dstb[i]);
       }

    sStr dstfCSV("%s-%s.csv",resultPathPrefix.ptr(), "baseline");
    sFil myContentf(dstfCSV);
    for (idx i=0;i<dstf.dim();i++){
      myContentf.printf("%.2lf,%.2lf\n",sMin+allStp*i,dstf[i]);
    }

    sStr dstnCSV("%s-%s.csv",resultPathPrefix.ptr(), dstsfx);
    sFil myContentn(dstnCSV);
    for (idx i=0;i<dstn.dim();i++){
        myContentn.printf("%.2lf,%.2lf\n",sMin+allStp*i,dstn[i]);
    }


    return cnt;
}

const char * sSpctr::_config_lst = "" __;
bool sSpctr::inputParams(const char * configFile)
{

    sString::SectVar cfgVars[]={


        {0,"[Algorithm]" _ "DualOutput" __,"%n=0",0,&dualOutput},

        {"/Spectrum","[Spectrum]" _ "Resolution" __,"%lf=0.05>1e-2<0.5","%g",&allStp,99},
        {"/Spectrum","[Spectrum]" _ "TopPeakNum" __,"%d=41","%d",&allTopI[0]},
        {"/Spectrum","[Spectrum]" _ "TopPeakNum2" __,"%d=5","%d",&allTopI[1]},
        {"/Spectrum","[Spectrum]" _ "TopPeakNum3" __,"%d=7","%d",&allTopI[2]},
        {"/Spectrum","[Spectrum]" _ "TopPeakNum4" __,"%d=13","%d",&allTopI[3]},
        {"/Spectrum","[Spectrum]" _ "TopPeakNum5" __,"%d=27","%d",&allTopI[4]},

        {"/Smoothing","[SavGol]" _ "Left Points" __,"%d=4>2<10","%d",&savgol.left,8},
        {"/Smoothing","[SavGol]" _ "Right Points" __,"%d=4>2<10","%d",&savgol.right,8},
        {"/Smoothing","[SavGol]" _ "Degree Polynomial" __,"%d=8>4<20","%d",&savgol.degree,16},

        {"/Denoising","[Wavlet]" _ "Daubechies" __,"%n=20^DAUB4=4^DAUB12=12^DAUB20=20;","%d",&wavlet.daubNum},
        {"/Denoising","[Wavlet]" _ "Underrepresented frequency filter" __,"%lf=66>0<100;","%g",&wavlet.fracPercent},

            {0,"[FFT]" _ "Daubechies" __,"%n=20^DAUB4^DAUB12^DAUB20;","%d",&fft.daubNum},
            {0,"[FFT]" _ "Min" __,"%d=10>1","%d",&fft.fftMin},
            {0,"[FFT]" _ "Max" __,"%n=1000000^UNLIMITED=1000000>1;","%d",&fft.fftMax},

        {"/Baseline","[Baseline]" _ "Maximum Accepted Frequency" __,"%d=100>0<1000","%d",&baseline.maxFreq},
        {"/Baseline","[Baseline]" _ "Threshold Intensity" __,"%lf=0>0<100","%g",&baseline.thresholdPercent},


        {"/Miscelaneous","[Misc]" _ "Excel" __,"%s=excel.exe","%s",&misc.excel},


        {"/Filter","[Filter]" _ "categoryUse" __,"%n=0;",0,&filter.iCateg},
        {"/Filter","[Filter]" _ "filterFrequency" __,"%lf=10>0<100","%lf%%",&filter.filterFrequency},
        {"/Filter","[Filter]" _ "filterANDOR1" __,"%n=0^AND^OR;",0,&filter.andor1},
        {"/Filter","[Filter]" _ "filterIntensity" __,"%lf=10>0<1000","%lf",&filter.filterIntensity},
        {"/Filter","[Filter]" _ "filterANDOR2" __,"%n=0^AND^OR;",0,&filter.andor2},
        {"/Filter","[Filter]" _ "filterTProb" __,"%lf=0.0>0<1","%lf",&filter.filterTProb},

        {"/LDA","[LDA]" _ "categoryUse" __,"%n=0;",0,&lda.iCateg},
        {"/LDA","[LDA]" _ "Axis 1" __,"%n=0^First^Second^Third^Fourth^Fifth^Sixth^Seventh^Eighth^Nineth^Tenth;",0,&lda.ax1},
        {"/LDA","[LDA]" _ "Axis 2" __,"%n=1^First^Second^Third^Fourth^Fifth^Sixth^Seventh^Eighth^Nineth^Tenth;",0,&lda.ax2},
        {"/LDA","[LDA]" _ "Axis 3" __,"%n=2^First^Second^Third^Fourth^Fifth^Sixth^Seventh^Eighth^Nineth^Tenth;",0,&lda.ax3},
        {"/LDA","[LDA]" _ "Regularization" __,"%lf=0.01>0.001<1;",0,&lda.regulAlpha},
        {"/LDA","[LDA]" _ "bootSpace" __,"%d=-3>-20<20",0,&lda.bootSpace},
        {"/LDA","[LDA]" _ "maxIter" __,"%d=1>1<1000",0,&lda.maxIter},

        {"/LDA","[LDA]" _ "Show Clusters" __,"%b=1|Show Mesh|Show Wireframe|Smoothed|Original Data;",0,&lda.showMesh},
        {"/LDA","[LDA]" _ "Transparency" __,"%d=128>0<255;",0,&lda.meshTransparency},
        {"/LDA","[LDA]" _ "Accuracy" __,"%d=50>20<100;",0,&lda.meshSteps},
        {"/LDA","[LDA]" _ "Diffusion" __,"%lf=0.05>0.01<0.3;",0,&lda.meshDiffusion},
        {"/LDA","[LDA]" _ "Coloration" __,"%n=0^Single Color^Color By Z;",0,&lda.meshColoration},
        {"/LDA","[LDA]" _ "SymbolSize" __,"%d=4>1<10",0,&lda.symbolSize},


            {0,"[LearnSet]" __,"%S",0,&coding.learn},
            {0,"[CheckSet]" __,"%S",0,&coding.check},

        {0, 0}
    };

    sStr rst;
    if( strstr(configFile, ".lst") ) {
        sFil inp(configFile, sFil::fReadonly);
        if( inp.length() )
            sString::cleanMarkup(&rst, inp.ptr(), inp.length(), "//" _ "/*" __, "\n" _ "*/" __, "\n", 0, false, false, true);
    }
    if( !rst.length() ) {
        sString::cleanMarkup(&rst, sSpctr::_config_lst, sLen(sSpctr::_config_lst), "//" _ "/*" __, "\n" _ "*/" __, "\n", 0, false, false, true);
    }

    sString::xscanSect(rst.ptr(), rst.length(), cfgVars);

    if( !inputMolecules(rst, inpMolList) ) {

        return false;
    }

    fft.fftMaxLog = (idx) log10((real) fft.fftMax);
    fft.fftMinLog = (idx) log10((real) fft.fftMin);

    sStr catBuf;
    sDic<sDic<sVec<idx> > > catSet;
    sText::categoryListParseCsv(&catBuf, coding.learn.ptr(), coding.learn.length(), &catSet, 0, 0, false);
    catList.printf(0, "%%n=0");
    for(idx ic = 0; ic < catSet.dim(); ++ic)
        catList.printf("^%s", (const char *)catSet.id(ic));
    catList.printf(";");

    for(idx ip = 0; ip < sDim(cfgVars) ; ++ip) {
        const char * nm = sString::next00(cfgVars[ip].loc);
        if( !nm )
            continue;
        if( strstr(nm, "SatelliteControl") || strstr(nm, "TopPeakNum") )
            cfgVars[ip].fmtin = inpMolList.ptr();
        if( strstr(nm, "categoryUse") )
            cfgVars[ip].fmtin = catList.ptr();
    }

    return true;
}


