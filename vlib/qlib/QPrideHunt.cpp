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
#include <qlib/QPrideProc.hpp>

using namespace slib;

class QPrideHunt:public sQPrideProc
{
public:
    QPrideHunt(const char * defline00,const char * srv):sQPrideProc(defline00,srv) {}
    virtual idx OnExecute(idx req);
    virtual bool OnCommand(const char * command, const char * value);
};


idx QPrideHunt::OnExecute(idx req)
{
    logOut(eQPLogType_Info,"Grabbed %" DEC " request for execution\n",req);
    idx grp=req2Grp(req);
    
    sStr data;
    reqGetData(grp,"stdin",&data);

    sStr data1;
    reqGetData(grp,"arg1",&data1);

    if(data.length()){
        printf("-------------RETRIEVED---------------\n%s\n---------------------------------\n%s\n--------------------------\n",data.ptr(),data1.ptr());
    }

    reqSetData(req, "stdout", "this is output blob for %" DEC " request", req );
    reqSetData(req, "test.out", "testout blob for  %" DEC " request", req );

    reqSetStatus(req, eQPReqStatus_Done);

    logOut(eQPLogType_Info,"Done processing %" DEC " request for execution\n",req);
    
    return 0;
}
bool QPrideHunt::OnCommand(const char * cmd, const char * value)
{
    if( sIs(cmd,"halt")){
        sleepMS(1000000);
        return true;
    }
    return false;
}

int main(int argc, const char * argv[])
{
    sApp::args(argc,argv);
    sStr tmp;
    QPrideHunt backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"test",argv[0]));
    backend.run(argc,argv);
}
