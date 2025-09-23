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
#include <slib/std/app.hpp>
#include "QPrideCmdline.hpp"
#include <qlib/QPrideBase.hpp>

using namespace slib;

sQPrideCmdline::sQPrideCmdline( idx argc , const char ** argv, const char ** envp)
{
    if(!argc) {
        argc=sApp::argc;
        argv=sApp::argv;
        envp=sApp::envp;
    }
    cmdl.init(argc,argv,envp);
}

sQPrideCmdline::~sQPrideCmdline ()
{
    sApp::cfgsave("QPCfg",true);
}



char * sQPrideCmdline::QP_configGet( sStr * vals00, const char * pars00, bool single)
{
    idx pos=vals00->length();

    if(!pars00)return vals00->ptr(pos);
    if(single==true)
        vals00->printf("%s",sApp::cfgget("QPCfg",pars00,""));
    else {

        for ( const char *p=pars00; p ; ++p) {
            vals00->printf("%s", sApp::cfgget("QPCfg", p, ""));
            vals00->add0();
            vals00->printf("%s", p);
            vals00->add0();
        }
    }
    vals00->add0(2);
    return vals00->ptr(pos);
}


bool sQPrideCmdline::QP_configSet(const char * par, const char * val)
{
    sApp::var->inp(par,val,sLen(val));
    return true;
}

idx sQPrideCmdline::QP_requestGet(idx req , void * R, bool isGrp, char * serviceName)
{
    memset(R,0,sizeof(sQPrideBase::Request));
    return req;
}




char * sQPrideCmdline::QP_reqDataGet(idx , const char * dataName, sMex * data)
{
    idx pos=data->pos();

    if(strcmp(dataName,"stdin")==0){
        data->readIO(stdin);
        return (char*)data->ptr(pos);
    }

    const char * dblb=cmdl.next(dataName);
    if(!dblb)return 0;


    if(strncmp((char*)dblb,"map://",6)==0){
        data->destroy();
        data->init(dblb+6);
    }
    else if(strncmp(dblb,"file://",7)==0){
        sFil f(dblb+7);
        if(f.length()){
            data->cut(pos);
            data->add(f.ptr(),f.length());
        }
     }
     else data->add(dblb,sLen(dblb)+1);

     return data->pos() ? (char*)data->ptr(pos) : 0 ;
}

bool sQPrideCmdline::QP_reqDataSet(idx , const char * dataName, idx dsize , const void * data)
{
    if(strcmp(dataName,"stdout")==0){
        fwrite(data,dsize,1,stdout);
        return true;
    }

    sFil ff(dataName);
    ff.add((const char *)data,dsize);
    return true;
}



idx sQPrideCmdline::QP_serviceGet(void * Svc, const char * , idx )
{
    memset(Svc,0,sizeof(sQPrideBase::Service));
    sQPrideBase::Service * S=((sQPrideBase::Service * )Svc);
    S->svcID=1;
    S->maxJobs=64;
    S->sleepTime=3000;
    S->maxLoops=1;
    S->parallelJobs=1;
    S->cleanUpDays=1;
    S->runInMT=1;
    S->isUp=sNotIdx;

    return 0;
}
