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
#include <slib/std.hpp>
#include <ulib/ulib.hpp>

#include <violin/dna-generic-launcher.hpp>

class SOAPLauncherProc: public DnaGenericLauncherProc
{
    public:
        SOAPLauncherProc(const char * defline00, const char * srv) : DnaGenericLauncherProc(defline00, srv)
        {
            defaultMemUlimit=4*1024*1024;
        }

        virtual idx OnExecute(idx req)
        {
            //regExpResultList.printf("*.csv");
            //cmdLineTemplate.printf("cat $(pathfasta(.query as intlist,\"*.vioseqlist\"))");
            //pathFunctions00="path" _ "pathfasta" _ "pathfastq" __;


            return DnaGenericLauncherProc::OnExecute(req);
        }
};


int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future
    SOAPLauncherProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "soap-launcher", argv[0]));
    return (int) backend.run(argc, argv);
}
