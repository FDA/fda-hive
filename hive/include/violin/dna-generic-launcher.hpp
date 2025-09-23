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
#ifndef dnaGenericLauncher_hpp
#define dnaGenericLauncher_hpp

#include <qlib/QPrideGenericLauncher.hpp>
#include <ssci/bio/bioseq.hpp>

class DnaGenericLauncherProc: public sQPrideGenericLauncher
{
    public:
        DnaGenericLauncherProc(const char * defline00, const char * srv) : sQPrideGenericLauncher(defline00, srv){
            sBioseq::initModule(sBioseq::eACGT);
            addDispatchedFunctions00="pathfasta" _ "pathfastq" _ "pathsam" _ "pathbam" _ "pathbt2" _ "pathfile" __;
        }
        virtual bool dispatcher_callback(sVariant &result, const qlang::BuiltinFunction &funcObj, qlang::Context &ctx, sVariant *topic, sVariant *args, idx nargs,void * param);
        sDic <idx > filesDumped;
        bool cleanObjectIDs(sStr &t, sVariant * args, sVec <idx> &objid);
        bool cleanObjectIDs(sStr &t, sVariant *args, sVec <sHiveId> &objids);

};

#endif 