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
#include <slib/utils.hpp>
#include <slib/core.hpp>
#include <violin/violin.hpp>
#include <ssci/math/stat/stat.hpp>
#include <qpsvc/archiver.hpp>
#include <ssci/bio/vax-bio.hpp>

#include <dmlib/dmlib.hpp>
class tblqryX4_virMut:public sQPrideGenericLauncher
{
    public:
        tblqryX4_virMut(const char * defline00,const char * srv):sQPrideGenericLauncher(defline00,srv) {

        }
        virtual idx OnExecute(idx);

};


idx tblqryX4_virMut::OnExecute(idx req)
{
    //requestStage.printf(0,"init");
    sQPrideGenericLauncher::OnExecute(req);

    //reqSetProgress(req,prgCount, 100);
       //reqSetStatus(req, eQPReqStatus_Running);
       //if(startProgPercent + lenProgPercent==100)
    //reqSetStatus(req, eQPReqStatus_Done);

    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    tblqryX4_virMut backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "viral-mutation-comp", argv[0]));
    return (int) backend.run(argc, argv);
}

