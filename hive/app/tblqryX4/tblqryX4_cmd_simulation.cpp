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

#include <slib/utils.hpp>
#include <slib/std/string.hpp>
#include <ssci/math.hpp>
#include "tblqryX4_cmd.hpp"

#include "utils.hpp"
#include "sort.hpp"

#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

#define PRFX "simulation-"
#define OUTFILE "simulation.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class SimulationCommand : public Command
        {
            private:
                idx numSamples;

            public:
                SimulationCommand(ExecContext & ctx) : Command(ctx)
                {
                    numSamples = 0;
                }

                const char * getName() { return "simulation"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdSimulationFactory(ExecContext & ctx) { return new SimulationCommand(ctx); }
    };
};

bool SimulationCommand::init(const char * op_name, sVariant * arg)
{
    _ctx.logDebug("Parsing the simulation command...");

    numSamples = arg->getDicElt("numSamples")->getListElt(0)->asInt();

    return true;
}



bool SimulationCommand::compute(sTabular * tbl)
{
    _ctx.logDebug("Running the simulation command...\n");

    // The MFA
    //sStat::MixtureFactorAnalyzers(idx _numComp, idx _numVar, idx * _numVal, idx ** observations, idx numObs, real stopThresh, idx maxIter);
    idx numComp = 2;
    idx numVar = tbl->cols();
    idx * numVal = new idx[numVar];
    idx numObs = tbl->rows();
    idx ** observations = new idx*[numObs];
    real stopThresh = 1e-3;
    idx maxIter = 10;

    // Populate numVal for the iteration
    for (idx i=0; i<numVar; i++) {
        numVal[i] = 0;
    }


    // Iterate through the rows of the table to populate numVal and observations
    for (idx i=0; i < tbl->rows(); i++) {
        observations[i] = new idx[numVar];
        for (idx j=0; j < tbl->cols(); j++) {
            observations[i][j] = tbl->ival(i,j);
            if (observations[i][j] > numVal[j]) {
                numVal[j] = observations[i][j];
            }
        }
    }

    // Construct the mfa
    sStat::MixtureFactorAnalyzers mfa = sStat::MixtureFactorAnalyzers(numComp, numVar, numVal, observations, numObs, stopThresh, maxIter);

    // Storage for the new samples
    idx ** obs = new idx*[numSamples];
    for (idx i=0; i<numSamples; i++) {
        obs[i] = new idx[numVar];
    }

    // Sample
    mfa.sample(obs,numSamples);

   // Now append the table with the samples...

    sTxtTbl * toReturn = new sTxtTbl();

    toReturn->initWritable(numSamples,sTblIndex::fTopHeader,",");

        for (idx r = 0; r < numSamples; r++)
        {
            for (idx c = 0; c < numVar; c++)
            {
                toReturn->addICell(obs[r][c]);
            }
            toReturn->addEndRow();
        }


    setOutTable(toReturn);
    return true;


}

