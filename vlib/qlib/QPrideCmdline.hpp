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
#ifndef _QPrideCmdline_qLib_hpp
#define _QPrideCmdline_qLib_hpp

#include <slib/std/cmdline.hpp>

#include "QPrideConnection.hpp"

namespace slib
{
    class sSql;



    class sQPrideCmdline: public sQPrideConnection
    {
        private:
            sCmdLine cmdl;
        public:
            sQPrideCmdline( idx argc=0, const char ** argv=0, const char ** envp=0);
            ~sQPrideCmdline ();

        public:
            char * QP_configGet( sStr * vals00, const char * pars00, bool single=true);
            bool QP_configSet(const char * par, const char * val);

            idx QP_serviceList(sStr * lst00, void * svcVecList){return 0;}

        public:
            idx QP_requestGet(idx req , void * r, bool isGrp=0, char * serviceName=0) ;

        public:
            char * QP_reqDataGet(idx req, const char * dataName,    sMex * data);
            bool QP_reqDataSet(idx req, const char * dataName, idx dsize , const void * data);

        public:
            idx QP_serviceGet(void * Svc, const char * serviceName=0, idx svcId=0);


    };

}

#endif 