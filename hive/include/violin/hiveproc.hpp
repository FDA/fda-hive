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
#ifndef sHiveProc_hpp
#define sHiveProc_hpp

#include <ssci/bio.hpp>
#include <ulib/ulib.hpp>
#include <qlib/QPrideProc.hpp>

class sHiveProc: public sQPrideProc
{
    typedef sQPrideProc TParent;

    public:
        sHiveProc(const char * defline00, const char * service)
            : TParent(defline00, service)
        {
        }
        virtual ~sHiveProc()
        {
        }
        static bool traveseProp(sUsrObj & so, sUsr & user, const char * prop, const char * traverse_prop, const char * type_filter, sStr * log);
        static idx customizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log, sMex **pFileSlices = 0);
        static idx frontEndCustomizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log, sMex **pFileSlices = 0);

        virtual sUsrQueryEngine * queryEngineFactory(idx flags = 0);


    protected:
        virtual splittertFunction getSplitFunction(const char * type);

    private:
        static sHiveProc gHP;
        idx m_customizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log, sMex **pFileSlices = 0);

        static const char * listTypes;

        static idx splitSequences(sVar * pForm, sUsr * user, sUsrProc * obj, const char * fld_val, idx slice);
        static idx splitAlignments(sVar * pForm, sUsr * user, sUsrProc * obj, const char * fld_val, idx slice);
};

#endif
