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


#include <qlib/QPrideCGI.hpp>
#include <violin/violin.hpp>
#include <violin/hiveproc.hpp>
#include <ssci/bio/ion-bio.hpp>




class IonCGI: public sQPrideCGI {
    public:

    IonCGI(const char * defline00, const char * service, idx argc, const char * * argv, const char * * envp, FILE * readfrom, bool isCookie, bool immediate)
        :sQPrideCGI(defline00, service, argc, argv, envp, readfrom, isCookie, immediate){
    }
    virtual idx Cmd(const char * cmd);
    idx customizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj, Service * pSvc, sStr * log, sMex **pFileSlices = 0)
    {
        return sHiveProc::customizeSubmission(pForm, user, obj, pSvc, log, pFileSlices);
    }


};

idx IonCGI::Cmd(const char * cmd)
{

    outHtml();
    sHiveIon hi(user);
    if(!hi.Cmd(static_cast < sIO * >(this), cmd, pForm ))
        return sQPrideCGI::Cmd(cmd );

    return 1;

}



int main(int argc, const char *argv[], const char *envp[])
{
    sApp::args(argc, argv, envp);
    IonCGI qapp("config=qapp.cfg" __,"ionCGI", argc, argv, envp, stdin, true, true);
    qapp.run();
    return 0;
}

