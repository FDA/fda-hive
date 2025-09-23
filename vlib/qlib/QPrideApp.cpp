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
#include <slib/core.hpp>
#include <slib/std/file.hpp>
#include <slib/std/app.hpp>
#include <qlib/QPrideProc.hpp>
#include <qlib/QPrideClient.hpp>

class QAppProc: public sQPrideProc
{
    public:
        QAppProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
};

idx QAppProc::OnExecute(idx req)
{
    sStr Exec;
    const char * exec = 0;
    reqGetData(req, "exec", &Exec);
    sVar rForm;
    if( !Exec.length() ) {
        sVar * pForm = reqGetData(req, "formT.qpride", &rForm);
        exec = pForm->value("exec", 0);
    } else {
        exec = Exec.ptr();
    }
    printf(":::::::::::%s\n", exec);
    if( !exec || *exec == 0 ) {
        return 0;
    }
    sCmdLine cmd;
    cmd.parse(exec);
    sQPrideClient qapp("config=qapp.cfg" __);
    qapp.reqId = req;
    sStr out;
    qapp.outP = &out;
    cmd.exec(qapp.cmdExes, 0);
    reqSetData(req, "stdout", out.mex());
    reqSetStatus(req, eQPReqStatus_Done);
    reqProgress(1, 100, 100);
    return 0;
}

int main(int argc, const char * argv[])
{
    sStr appLog, dbgLog;
    sCmdLine cmd;
    if( argc > 1 ) {
        cmd.init(argc, argv);
    } else {
        cmd.init("-help");
    }
    if( sString::parseBool( cmd.next("-daemon") ) ) {
        sStr tmp;
        sApp::args(argc, argv);
        QAppProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "qapp", argv[0]));
        return (int) backend.run(argc, argv);
    }
    sQPrideClient qapp("config=qapp.cfg" __);
    return cmd.exec(qapp.cmdExes, 0, &appLog, &dbgLog);
}
