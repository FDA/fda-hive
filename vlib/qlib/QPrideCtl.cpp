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

class QPrideCtl : public sQPrideCGI
{
    public:
        QPrideCtl(const char * defline00=0, idx argc=0, const char * * argv=0, const char * * envp=0)
            :sQPrideCGI(defline00, "qm", argc, argv, envp, stdin, false, true ){
            //comma="//";
            //endl="//\n";
        }
        virtual ~QPrideCtl(){

        }

    public:
        virtual idx Cmd(const char * cmd);

};

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/  Command Handler
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

idx QPrideCtl::Cmd(const char * cmd)
{
    //
    // Default commands
    //
    idx ret=sQPrideCGI::Cmd(cmd); if(ret)return ret;


    enum enumCommands{
        eNoCommand,
        eLast
    };
    const char * listCommands=
        "-noCommand"_
        __;
    idx cmdnum=-1;
    sString::compareChoice( cmd, listCommands,&cmdnum,false, 0,true);
    sStr tmp;

    switch(cmdnum) {
        case eNoCommand:{
            outHtml();
        }break;
        default: return 0;
    }

    return 1;
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/  Initialization
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int main(int argc, const char *argv[])
{
    QPrideCtl qapp("config=qapp.cfg"__, argc, argv);
    qapp.run();
    return 0;
}
