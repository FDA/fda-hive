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
#ifndef sLib_tblqryx4_cmdlist_h
#define sLib_tblqryx4_cmdlist_h

#include "tblqryX4_cmd.hpp"

/* List of "external" commands.
   Entries of form X("foobar", cmdFoobarFactory), where cmdFoobarFactory is defined as follows
   in the appropriate .cpp file:

namespace slib {
    namespace tblqryx4 {
        Command * cmdFoobarFactory(ExecContext * ctx) { return new cmdFoobar(ctx); }
    };
};
*/
#define TBLQRYX4_EXT_FACTORIES \
    X("som", cmdSomFactory) \
    X("fourier", cmdFourierFactory) \
    X("tree", cmdTreeFactory) \
    X("statTest", cmdStatTestFactory) \
    X("kaplanmeier", cmdKaplanMeierFactory) \
    X("simulation", cmdSimulationFactory) \
    X("collapsewithstat", cmdCollapseWithStatFactory) \
    X("compAnalysis", cmdCompAnalysisFactory) \
    X("dataCalib", cmdDataCalibFactory) \
    X("extract", cmdExtractFactory) \
    X("bStat", cmdBStatFactory) \
    X("cleaner", cmdCleanerFactory) \
    X("pearsonCorr", cmdPearsonCorrFactory) \
    X("data-trending", cmdDataTrendingFactory) \
    X("addMissingRows", cmdAddMissingRowsFactory) \
    X("load-SNPprofile", cmdLoadSNPprofileFactory) \
    X("fold_change", cmdFoldChangeFactory) \
    X("sdtm_summary", cmdSdtmSummaryFactory) \
    X("dictionaryTable", cmdDicTestFactory) \
    X("generic-cmd-test", cmdGenericFactory) \
    X("variance", cmdVarianceFactory)


#ifdef HAS_IMAGEMAGICK
#define TBLQRYX4_EXT_IMAGEMAGICK_FACTORIES \
    X("heatmap", cmdHeatmapFactory)
#else
#define TBLQRYX4_EXT_IMAGEMAGICK_FACTORIES
#endif


namespace slib {
    namespace tblqryx4 {
#define X(cmdname, func) Command * func(ExecContext & ctx);
        TBLQRYX4_EXT_FACTORIES
        TBLQRYX4_EXT_IMAGEMAGICK_FACTORIES
#undef X
    };
};

#endif
