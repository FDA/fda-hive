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



#include <slib/std/cgi.hpp>
#include <slib/std/app.hpp>
#include <slib/std/file.hpp>
#include <ion/sIon-client.hpp>

using namespace slib;

sIonClient ionClient;


class sIGI: public sCGI {
    public:

    sIGI(idx argc = 0, const char * * argv = 0, const char * * envp = 0, FILE * readfrom = stdin, bool isCookie = false, bool immediate = true, const char * forcedMethod = 0)
        :sCGI(argc ,argv , envp , readfrom , isCookie , immediate , forcedMethod ){
    }
    virtual idx Cmd(const char * cmd);


};

idx sIGI::Cmd(const char * cmd)
{
    sStr masterpath("./%s",pForm->value("ion_master","ion_master"));

    sDir dir;dir.list(sFlag(sDir::bitFiles)|sFlag(sDir::bitNoExtension)|sFlag(sDir::bitFollowLinks), masterpath.ptr(), "ion.ion", 0, 0);
    pForm->inptr("ion_master",dir.ptr(),dir.length());
    cmd = pForm->value("cmd");

    outHtml();
    idx res=cmd ? ionClient.Cmd(this,cmd,pForm) : 0 ;
    if(!res)return sCGI::Cmd(cmd);

    return 1;

}



int main(int argc, const char *argv[], const char *envp[])
{
    sApp::args(argc, argv, envp);
    sIGI igi(argc ,argv , envp , stdin, true, true);
    igi.run();
    return 0;
}

