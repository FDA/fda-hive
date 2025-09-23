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
#include <ssci/math/stat/stat.hpp>
#include <ssci/math/rand/rand.hpp>
#include <math.h>

using namespace slib;

void sStat::MixtureFactorAnalyzers::logProbs (void) {
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
    real total = 0.0;


    for (idx k=0; k<numComp; k++) {
        real logProb = p[k];

        for (idx i=0; i<numVar; i++) {
            logProb += mu[i][k][observation[i]];
        }
        l[k] = exp(logProb);
        total += l[k];
    }


    for (idx k=0; k<numComp; k++) {
        l[k] /= total;
    }

}

real sStat::MixtureFactorAnalyzers::logLikelihood(idx ** observations, idx numObs) {
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
    real * pCgO = new real[numComp];




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



    for (idx i=0; i<numObs; i++) {
        conditionalProbs(pCgO,observations[i]);

        for (idx k=0; k<numComp; k++) {
            q[k] += pCgO[k];
            for (idx d=0; d<numVar; d++) {
                nu[d][k][observations[i][d]] += pCgO[k];
            }
        }
    }



    for (idx d=0; d<numVar; d++) {
            for (idx k=0; k<numComp; k++) {
                for (idx i=0; i<numVal[k]; i++) {
                    nu[d][k][i] /= q[k];
                }
            }
    }

    for (idx k=0; k<numComp; k++) {
        q[k] /= numObs;
    }




    logProbs();

    for (idx k=0; k<numComp; k++) {
        p[k] = q[k];
        for (idx d=0; d<numVar; d++) {
            for (idx i=0; i<numVal[d]; i++) {
                mu[d][k][i] = nu[d][k][i];
            }
        }
    }


}

void sStat::MixtureFactorAnalyzers::cumProbs(real * q, real *** nu) {
    sRand::cumulative(q,p,numComp);

    for (idx d=0; d<numVar; d++) {
        for (idx k=0; k<numComp; k++) {
            sRand::cumulative(nu[d][k],mu[d][k],numVal[d]);
        }
    }
}

void sStat::MixtureFactorAnalyzers::sample(idx ** obs, idx numSamp) {

    real * cdfP = new real[numComp];
    real *** cdfMu = new real**[numVar];
    for (int d=0; d<numVar; d++) {
        cdfMu[d] = new real*[numComp];
        for (int k=0; k<numComp; k++) {
            cdfMu[d][k] = new real[numVal[d]];
        }
    }


    cumProbs(cdfP,cdfMu);




    for (int n=0; n<numSamp; n++) {
        int latentVar = sRand::categoricalRand(cdfP,numComp);
        for (int d=0; d<numVar; d++) {
            obs[n][d] = sRand::categoricalRand(cdfMu[d][latentVar],numVal[d]);
        }
    }
}
