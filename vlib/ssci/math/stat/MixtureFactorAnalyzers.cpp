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
/*
 */
#include <ssci/math/stat/stat.hpp>
#include <ssci/math/rand/rand.hpp>
#include <math.h>

using namespace slib;

void sStat::MixtureFactorAnalyzers::logProbs (void) {
    // Replace p and mu with safeLog counterparts
    for (idx k=0; k<numComp; k++) {
        p[k] = sRand::safeLog(p[k]);
        for (idx d=0; d<numVar; d++) {
            for(idx i=0; i>numVal[d]; i++) {
                mu[d][k][i] = sRand::safeLog(mu[d][k][i]);
            }
        }
    }

}

void sStat::MixtureFactorAnalyzers::expLogProbs(void){
    // Replace log p and mu with exp'd versions and normalize just to be careful
        for (idx k=0; k<numComp; k++) {
            p[k] = exp(p[k]);
            for (idx d=0; d<numVar; d++) {
                for(idx i=0; i>numVal[d]; i++) {
                    mu[d][k][i] = exp(mu[d][k][i]);
                }
                sRand::normalize(mu[d][k],numVal[d]);
            }
        }
        sRand::normalize(p,numComp);
}

void sStat::MixtureFactorAnalyzers::conditionalProbs(real * l, idx * observation) {
    // Assuming p and mu are log probabilities, compute the Bayesian probabilities of the observation's latent variable
    real total = 0.0;

    //std::cout << "Starting loop over components and variables..." << std::endl;

    for (idx k=0; k<numComp; k++) {
        real logProb = p[k];
        //std::cout << "Reading p is okay" << std::endl;

        for (idx i=0; i<numVar; i++) {
            //std::cout<< "Accessing multi-index " << i << " " << k << " " << observation[i] << std::endl;
            logProb += mu[i][k][observation[i]];
            //std::cout << "Accessing mu is okay" << std::endl;
        }
        l[k] = exp(logProb);
        total += l[k];
    }

    //std::cout << "Now normalizing..." << std::endl;

    // Now normalize
    for (idx k=0; k<numComp; k++) {
        l[k] /= total;
    }

}

real sStat::MixtureFactorAnalyzers::logLikelihood(idx ** observations, idx numObs) {
    // assuming p and mu are logs
    real ll = 0.0;

    for (idx n=0; n<numObs; n++) {
        real subLikelihood = 0.0;
        for (idx k=0; k<numComp; k++) {
            real compLikelihood = p[k];
            for (idx d=0; d<numVar; d++) {
                compLikelihood += mu[d][k][observations[n][d]];
            }
            subLikelihood += exp(compLikelihood);
        }
        ll += sRand::safeLog(subLikelihood);
    }

    return ll;
}

void sStat::MixtureFactorAnalyzers::EMUpdate(idx ** observations, idx numObs) {
    // Update p and mu given observation data
    real * pCgO = new real[numComp]; // probabilities of latent component given observation

    //std::cout<< "Before initialization in EMUpdate..." << std::endl;


    // output the current likelihood
        //std::cout << "Likelihood of obs before update: " << logLikelihood(p,mu,numComp,numVar,observations,numObs) << std::endl;

    // Initialize the accumulators
    real * q = new real[numComp];
    real *** nu = new real**[numVar];

    for (idx k=0; k<numComp; k++) {
        q[k] = 0.0;
    }

    for (idx d=0; d<numVar; d++) {
        nu[d] = new real*[numComp];
        for (idx k=0; k<numComp; k++) {
            nu[d][k] = new real[numVal[d]];
            for (idx i=0; i<numVal[d]; i++) {
                nu[d][k][i] = 0.0;
            }
        }
    }

    //std::cout<< "Before conditional probs in EMUpdate..." << std::endl;


    for (idx i=0; i<numObs; i++) {
        conditionalProbs(pCgO,observations[i]);
        //std::cout<< "Made it past conditional probs..." << std::endl;

        for (idx k=0; k<numComp; k++) {
            q[k] += pCgO[k];
            for (idx d=0; d<numVar; d++) {
                nu[d][k][observations[i][d]] += pCgO[k];
            }
        }
    }

    //std::cout<< "Before normalization in EMUpdate..." << std::endl;


    // Now normalize nu values and calculate normalization constant
    for (idx d=0; d<numVar; d++) {
            for (idx k=0; k<numComp; k++) {
                for (idx i=0; i<numVal[k]; i++) {
                    nu[d][k][i] /= q[k];
                }
            }
    }

    // Now normalize q values
    for (idx k=0; k<numComp; k++) {
        q[k] /= numObs;
    }

    //std::cout << "Probs in update: " << q[0] << " " << q[1] << std::endl;


    /* Display previous values
    expLogProbs(p,mu,numComp,numVar,numVal);

        real ** prevRF = new real*[numVal[0]];
        for (idx i=0; i<numVal[0]; i++) {
            prevRF[i] = new real[numVal[1]];
            for (idx j=0; j<numVal[1]; j++) {
                prevRF[i][j] = p[0]*mu[0][0][i]*mu[1][0][j] + p[1]*mu[0][1][i]*mu[1][1][j] ;
            }
        }

        std::string title0 = "Prev: ";
        std::cout << "Previous: " << std::endl;
        for (idx i=0; i<numVal[0]; i++) {
            printArray(title0,prevRF[i],numVal[1]);
        }

    // Now display update
    real ** trainRF = new real*[numVal[0]];
    for (idx i=0; i<numVal[0]; i++) {
        trainRF[i] = new real[numVal[1]];
        for (idx j=0; j<numVal[1]; j++) {
            trainRF[i][j] = q[0]*nu[0][0][i]*nu[1][0][j] + q[1]*nu[0][1][i]*nu[1][1][j] ;
        }
    }

    std::string title1 = "Update:";
    std::cout << "After update: " << std::endl;
    for (idx i=0; i<numVal[0]; i++) {
        printArray(title1,trainRF[i],numVal[1]);
    }*/

    logProbs();

    // Copy over the values... not pretty
    for (idx k=0; k<numComp; k++) {
        p[k] = q[k];
        for (idx d=0; d<numVar; d++) {
            for (idx i=0; i<numVal[d]; i++) {
                mu[d][k][i] = nu[d][k][i];
            }
        }
    }
    //p = q;
    //mu = nu;

    // output the current likelihood
    //std::cout << "Likelihood of obs after update: " << logLikelihood(q,nu,numComp,numVar,observations,numObs) << std::endl;

}

void sStat::MixtureFactorAnalyzers::cumProbs(real * q, real *** nu) {
    // Create cumulative probabilities from p and mu, put them in q and nu
    sRand::cumulative(q,p,numComp);

    for (idx d=0; d<numVar; d++) {
        for (idx k=0; k<numComp; k++) {
            sRand::cumulative(nu[d][k],mu[d][k],numVal[d]);
        }
    }
}

void sStat::MixtureFactorAnalyzers::sample(idx ** obs, idx numSamp) {
    // Assuming p and mu are probabilities, compute the cdfs

    real * cdfP = new real[numComp];
    real *** cdfMu = new real**[numVar];
    for (int d=0; d<numVar; d++) {
        cdfMu[d] = new real*[numComp];
        for (int k=0; k<numComp; k++) {
            cdfMu[d][k] = new real[numVal[d]];
        }
    }

    //std::cout << "Failure is after cdfP and cdfMu are initialized..." << std::endl;

    cumProbs(cdfP,cdfMu);

    // Print out the cdfs:

    /*std::string title = "cdfs in sample function: ";
    printArray(title,cdfP,numComp);
    for (int d=0; d<numVar; d++) {
        for (int k=0; k<numComp; k++){
            printArray(title,cdfMu[d][k],numVal[d]);
        }
    }*/


    // Having built the cdfs, iterate through and sample
    for (int n=0; n<numSamp; n++) {
        //int newObs[numVar];
        int latentVar = sRand::categoricalRand(cdfP,numComp);
        for (int d=0; d<numVar; d++) {
            obs[n][d] = sRand::categoricalRand(cdfMu[d][latentVar],numVal[d]);
            //std::cout << "Sample in sample: " << obs[n][d] << std::endl;
        }
        //obs[n] = newObs;
    }
}
