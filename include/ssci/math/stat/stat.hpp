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
#pragma once
#ifndef sMath_stat_hpp
#define sMath_stat_hpp

#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <slib/utils/sort.hpp>
#include <ssci/math/rand/rand.hpp>

namespace slib
{
    class sStat { // lower level functions
    public:

        static void statTest(real * actmat, idx rows, idx cols, sVec < sVec < idx > >  * colset, real * probvals, real * tstatvals);
        static real studentsT(real t, real df);
        static real studentsF(real f, real df1, real df2);
/*
        static real min(sVec<idx> * array); //! will return minimum value of given array with related index
        static real max(sVec<idx> * array); //! will return maximum value of given array with related index
        static real mean(sVec<idx> * array); //! will return mean value of given array
        static real median(sVec<idx> * array); //! will return median value of given array
        static real mode(sVec<idx> * array); //! will return mode value of given array
        static real range(sVec<idx> * array); //! will return range value of given array
        static real variance(sVec<idx> * array); //! will return variance value of given array
        static real stDev(sVec<idx> * array); //! will return standard deviation value of given array
        */

        template <class T> static real min(sVec<T> * array) //! will return minimum value of given array with related index
        {
            if(array->dim()==0){
                return sNotIdx;
            }
            idx itemp=0;
            for(idx i=1;i < array->dim();++i){
                if(*array->ptr(itemp) > *array->ptr(i)){
                    itemp = i;
                }
            }
           /* if (pos){
                *pos=(idx)itemp;
            }*/
            return *array->ptr(itemp);

        }
        template <class T> static real max(sVec<T> * array) //! will return miximum value of given array with related index
        {
            if(array->dim()==0){
                return sNotIdx;
            }
            idx itemp = 0;
            for(idx i=1;i < array->dim();++i){
                if(*array->ptr(itemp) < *array->ptr(i)){
                    itemp = i;
                }
            }
           /* if (pos){
                *pos=itemp;
            }*/
            return *array->ptr(itemp);

        }
        template <class T> static real mean(sVec<T> * array){//!The "mean" is the "average"
            if(array->dim()==0){
                    return sNotIdx;
                }
            real temp=0.0;
            for(idx i=0;i < array->dim();++i){
                temp+=(real)*array->ptr(i);

            }
            return temp/(array->dim());

        }
        template <class T> static real median(sVec<T> * array){ //!The "median" is the "middle" value in the list of numbers.
            if(array->dim()==0){
                        return sNotIdx;
                    }


            sVec<idx> ind;
            sVec <T> vector;
            ind.add(array->dim());
            vector.add(array->dim());
            for(idx i = 0; i < array->dim(); ++i) {
                vector[i] = *array->ptr(i);
            }
            sSort::sort(array->dim(), vector.ptr(0), ind.ptr(0));

            if(array->dim()%2!=0){


                return(*array->ptr(ind[(((array->dim()+1)/2)-1)]));


            }else{
                return(*array->ptr(ind[(array->dim()/2)-1]));

            }


        }
        template <class T> static real mode(sVec<T> * array){//! The "mode" is the value that occurs most often. If no number is repeated, then there is no mode for the list.
            if(array->dim()==0){
                           return sNotIdx;
                       }
            sVec<idx> ind;
            sVec<T> vector;
            ind.add(array->dim());
            vector.add(array->dim());
            for (idx i = 0; i < array->dim(); ++i) {
                vector[i]=0;
                idx j=0;
                while ((j < i) && (*array->ptr(i) != *array->ptr(j))) {
                    if (*array->ptr(i) != *array->ptr(j)) {
                                j++;
                    }
                    }
                        (vector[j])++;
                    }
            idx iMaxRepeat = 0;
                for (idx i = 1; i < array->dim(); ++i) {
                    if (vector[i] > vector[iMaxRepeat]) {
                        iMaxRepeat = i;
                    }
                }
                //to do no mode mode
            if(iMaxRepeat==0) return sNotIdx;
            return *array->ptr(iMaxRepeat);

        }


        template <class T> static real range(sVec<T> * array){//!The "range" is just the difference between the largest and smallest values.
            real minimum=min(array);
            real maximum=max(array);
            return maximum-minimum;



        }
        template <class T> static real variance(sVec<T> * array){
            if(array->dim()==0){
                               return sNotIdx;
                           }
            real sumsq,average;

            sumsq=0;
            average=0;
            average=mean(array);
            real temp;
            for (idx i = 0; i < array->dim(); ++i) {
                temp=*array->ptr(i)-average;
                sumsq+=temp*temp;

            }
            return sumsq/array->dim();


        }
        template <class T> static real stDev(sVec<T> * array){
            return sqrt(variance(array));
        }

        template<class T> static idx timeSeriesPeakDetection(sVec<T> &signal, sDic<idx> &peaks, idx window, real threshold, real coefficient)
        {
            sVec<T> fltW(sMex::fSetZero | sMex::fExactSize);
            sVec<real> avgW(sMex::fSetZero | sMex::fExactSize);
            sVec<real> stdW(sMex::fSetZero | sMex::fExactSize);
            fltW.resize(window);
            avgW.resize(window);
            stdW.resize(window);
            idx cnt = 0;
            for(idx i = 0; i < window; ++i) {
                fltW[i] = signal[i];
            }
            avgW[window - 1] = mean(&fltW);
            stdW[window - 1] = stDev(&fltW);
            for(idx i = window; i < signal.dim(); ++i) {
                if( sAbs(signal[i] - avgW[(i - 1) % window]) > threshold * stdW[(i - 1) % window] ) {
                    if( signal[i] > avgW[(i - 1) % window] ) {
                        peaks[&i] = 1;
                    } else {
                        peaks[&i] = -1;
                    }
                    ++cnt;
                    fltW[i % window] = coefficient * signal[i] + (1 - coefficient) * fltW[(i - 1) % window];
                } else {
                    fltW[i % window] = signal[i];
                }
                avgW[i % window] = mean(&fltW);
                stdW[i % window] = stDev(&fltW);
            }
            return cnt;
        }
        /*! Detect peaks in vector  based on the amplitude and other features. Returns the number of peaks.
        *    signal : input vector.
        *    peaks : results with the indices of the peaks detected
        *    groups : (optional) vector with mapped positions of input to each peak
        *    minPeak : detect peaks that are greater than minimum peak height, optional (default = 0)
        *    threshold :  optional (default = 0)
        *        detect peaks (valleys) that are greater (smaller) than `threshold`
        *        in relation to their immediate neighbors.
        *    minDistance :  optional (default = 1)
        *        detect peaks (valleys) that are separated at least minDistance positions
        *        from each other.
        *    edge : {0:None, 1:'rising', 2:'falling', 3:'both'}, optional (default = 0)
        *        for a flat peak, keep only the rising edge ('rising'), only the
        *        falling edge ('falling'), both edges ('both'), or don't detect a
        *        flat peak (None).
        *    valley : bool, optional (default = False)
        *        if True (1), detect valleys (local minima) instead of peaks.
        */

        template<class T> static idx peakDetection(sVec<T> &signal, sVec<idx> * peak_inds, sVec<idx> * groups = 0, bool reportBoundaries = false,idx minDistance = 1, idx reportSideOfPlateau = 0, bool valleyDetection = false, T minPeak = 0, T threshold = 0)
        {
            sVec<T> * p_peaks, l_pks;
            sVec<idx> *p_peak_inds, l_pk_inds;
            p_peaks = &l_pks;

            if(!peak_inds) {
                p_peak_inds = &l_pk_inds;
            } else {
                p_peak_inds = peak_inds;
            }
            if( !peak_inds && !groups )
                return 0;
            sVec<T> dx;
            dx.resize(signal.dim() - 1);
            for(idx i = 0; i < dx.dim(); ++i)
                dx[i] = signal[i + 1] - signal[i];
            if( valleyDetection ) {
                for(idx i = 0; i < signal.dim(); ++i)
                    signal[i] = -signal[i];
            }
            bool selected = false;
            idx last_valey = 0, last_peak = -1;
            T dp, da;
            idx trim = reportBoundaries?0:1;
            for(idx ix = trim; ix < signal.dim() - trim; ++ix) {
                dp = ix?dx[ix-1]:signal[ix];
                da = (ix < dx.dim() ) ? dx[ix] : -signal[ix];
                selected = false;
                if( reportSideOfPlateau <= 0 ) {
                    if( da < 0 && dp > 0 )
                        selected = true;
                } else {
                    if( (reportSideOfPlateau == 1 || reportSideOfPlateau >= 3) && da <= 0 && dp > 0 )
                        selected = true;
                    else if( (reportSideOfPlateau == 2 || reportSideOfPlateau >= 3) && (da < 0 && dp >= 0) )
                        selected = true;
                }
                if( selected && sMin(dp,-da) >= threshold && signal[ix] >= minPeak ) {
                    if( groups ) {
                        for(idx ig = last_valey; ig <= ix; ++ig)
                            *groups->ptrx(ig) = ix;
                    }
                    p_peak_inds->vadd(1,ix);
                    p_peaks->vadd(1,signal[ix]);
                    last_peak = ix;
                } else if ( last_peak >= 0 ){
                    if( groups ) {
                        *groups->ptrx(ix) = last_peak;
                    }
                    if( da > 0 && dp <= 0 )
                        last_valey = ix;
                }
            }
            if( groups )
                *groups->ptrx(signal.dim() - 1) = last_peak;

            if( minDistance > 1 ) {
                sVec<idx> p_peaks_sort_inds(sMex::fSetZero|sMex::fExactSize);
                p_peaks_sort_inds.resize(p_peaks->dim());
                sSort::sort(p_peaks->dim(),p_peaks->ptr(),p_peaks_sort_inds.ptr());

                for(idx i = p_peaks->dim() - 1 ; i >= 0 ; --i ) {
                    for(idx j = i-1 ; j >= 0 && *p_peak_inds->ptr(p_peaks_sort_inds[i]) >= 0 ; --j ) {
                        if( *p_peak_inds->ptr(p_peaks_sort_inds[j]) >= 0 && sAbs(*p_peak_inds->ptr(p_peaks_sort_inds[i]) - *p_peak_inds->ptr(p_peaks_sort_inds[j])) < minDistance) {
                            *p_peak_inds->ptr(p_peaks_sort_inds[j]) = -(*p_peak_inds->ptr(p_peaks_sort_inds[i])) -1 ;
                        }
                    }
                }

                if( groups ) {
                    idx prev_peak = -1;
                    for(idx i = 0, ig = 0; i < p_peak_inds->dim() ; ++i) {
                        if( *p_peak_inds->ptr(i) >= 0 ) {
                            while ( ig < groups->dim() && *groups->ptr(ig) == *p_peak_inds->ptr(i) ) ++ig;
                        } else {
                            prev_peak = *groups->ptr(ig);
                            while ( ig < groups->dim() && *groups->ptr(ig) == prev_peak ) {
                                *groups->ptr(ig++) = -(*p_peak_inds->ptr(i) + 1);
                            }
                        }
                    }
                }
                if(peak_inds) {
                    idx ip = 0;
                    for(idx i = 0; i < p_peak_inds->dim() ; ++i) {
                        if( *p_peak_inds->ptr(i) >=0 )
                            *p_peak_inds->ptr(ip++) = *p_peak_inds->ptr(i);
                    }
                    p_peak_inds->cut(ip);
                }
            }


            return peak_inds ? p_peaks->dim() : groups->dim();
        }


        // A class to house the mixture of categorical factor analyzers
        class MixtureFactorAnalyzers {

            private:
                idx numComp;
                idx numVar;
                idx * numVal;
                real * p; // Latent variables
                real *** mu; // List of probabilities organized by variable, component, value probability

                void randomMFA(void) {
                    // Initialize the probabilities of the latent classes
                    p = new real[numComp];
                    sRand::dirichletRand(p,numComp);

                    // Initialize the probabilities of the factor models
                    mu = new real**[numVar];
                    for (idx d=0; d < numVar; d++) {
                        mu[d] = new real*[numComp];
                        for (idx k=0; k<numComp; k++) {
                            mu[d][k] = new real[numVal[d]];
                            sRand::dirichletRand(mu[d][k],numVal[d]);
                        }
                    }
                };

                void logProbs (void);
                void expLogProbs(void);
                void conditionalProbs(real * l, idx * observation);
                void EMUpdate(idx ** observations, idx numObs);
                void cumProbs(real * q, real *** nu);

            public:
                // Constructors

                // Random MFA
                MixtureFactorAnalyzers(idx _numComp, idx _numVar, idx * _numVal) {
                    numComp = _numComp;
                    numVar = _numVar;
                    numVal = _numVal;
                    randomMFA();
                };

                // Construct from EM training algorithm
                MixtureFactorAnalyzers(idx _numComp, idx _numVar, idx * _numVal, idx ** observations, idx numObs, real stopThresh, idx maxIter) {
                                    // First, perform random initialization
                                    numComp = _numComp;
                                    numVar = _numVar;
                                    numVal = _numVal;
                                    randomMFA();

                                    // Perform the first EM step
                                    real oldLL = logLikelihood(observations,numObs) / ((real) numObs);
                                    EMUpdate(observations,numObs);
                                    real newLL = logLikelihood(observations,numObs) / ((real) numObs);
                                    idx iter = 1;

                                    // Iterate up to maxIter or until the average logLikelihood is small enough
                                    while ( iter < maxIter && newLL-oldLL < stopThresh) {
                                        oldLL = newLL;
                                        EMUpdate(observations,numObs);
                                        newLL = logLikelihood(observations,numObs) / ((real) numObs);
                                    }

                                    //TODO Consider verbose mode
                                };

                real logLikelihood(idx ** observations, idx numObs);
                void sample(idx ** obs, idx numSamp);


        };

    };

}

#endif // sMath_stat_hpp





