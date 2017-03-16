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
#ifndef _QPrideCgi_qLib_hpp
#define _QPrideCgi_qLib_hpp

#include <slib/std/app.hpp>
#include <qlib/QPrideClient.hpp>
#include <qlib/QPrideProc.hpp>
#include <ssql/sql.hpp>
#include <ulib/ucgi.hpp>


namespace slib
{
//    class QCGIProc: public sQPrideProc
//    {
//        public:
//            QCGIProc(const char * defline00, const char * srv)
//                : sQPrideProc(defline00, srv)
//            {
//            }
//            virtual idx OnExecute(idx);
//    };

    class sQPrideCGI: public sQPrideClient , public sUsrCGI
    {
        public:
            sQPrideCGI(const char * defline00 = 0, const char * service = "qm", idx argc = 0, const char * * argv = 0, const char * * envp = 0, FILE * readfrom = stdin, bool isCookie = false, bool immediate = true)
                : sQPrideClient(defline00, service), sUsrCGI(argc, argv, envp, readfrom, isCookie, immediate), cntParallel(0), requiresGroupSubmission(false)

            {
                proc_obj=0;
            }
            virtual ~sQPrideCGI()
            {
            }

        public:
            virtual void printf(const char * formatDescription , ...) __attribute__((format(printf, 2, 3)))
            {
                sCallVarg(dataForm.vprintf,formatDescription);
            }
            const char * formValue(const char * prop, sStr * buf=0, const char * defaultValue=0, idx iObj=0 );
            idx formIValue(const char * prop, idx defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                idx ival=defaultValue;
                if(p)sscanf(p,"%" DEC,&ival);
                return ival;
            }
            idx formUValue(const char * prop, udx defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                udx uval=defaultValue;
                if(p)sscanf(p,"%" UDEC,&uval);
                return uval;
            }
            real formRValue(const char * prop, real defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                real rval=defaultValue;
                if(p)sscanf(p,"%lf",&rval);
                return rval;
            }
            bool formBoolValue(const char * prop, bool defaultValue=false, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                bool bval=defaultValue;
                if(p) bval = sString::parseBool(p);
                return bval;
            }
            idx formBoolIValue(const char * prop, idx defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                idx ival=defaultValue;
                if(p) ival = sString::parseIBool(p);
                return ival;
            }
            char * listObjOrReqFiles( sStr * pathlist, const char * flnm00, const char * svc=0, const char* separator=",", bool forceReq=false );



            virtual idx Cmd(const char * cmd);
            virtual bool OnCGIInit(void);
            virtual idx customizeSubmission(sVar * pForm , sUsr * user, sUsrProc * obj, Service * pSvc, sStr * log, sMex **pFileSlices = 0){return 1;}
            virtual bool runAsBackEnd(void);
            virtual idx run(const char * rcmd = 0);

            idx cntParallel;
            bool requiresGroupSubmission;
            sVec < sUsrObj > objs;
            void * proc_obj;
            //sVar * pForm;

//            virtual idx run(const char * rcmd = 0);
        private:
            idx QPSubmit(sVar & forma, bool withObjs, sQPride::Service & S, sStr * strObjList = 0);
    };


    // this is a class for Backendable CGI's
    class sQPrideCGIProc : public sQPrideCGI
    {

        sQPrideCGI * procCGI_qapp;
        const char * svcName;

        public:

            sQPrideCGIProc(const char * defline00 = 0, const char * service = "qm", idx argc = 0, const char * * argv = 0, const char * * envp = 0, FILE * readfrom = stdin, bool isCookie = false, bool immediate = true)
                : sQPrideCGI ( defline00 , service , argc , argv , envp , readfrom , isCookie , immediate )
            {
                procCGI_qapp = 0;
                svcName=service;
            }
            virtual ~sQPrideCGIProc(){
                if(procCGI_qapp)
                    delete procCGI_qapp;
            }

            virtual bool OnInit(void)
            {
                if( !procCGI_qapp )
                    procCGI_qapp = new sQPrideCGI("config=qapp.cfg" __,svcName, sApp::argc, sApp::argv, sApp::envp, stdin, true, true);
                return procCGI_qapp->OnInit();
            }


            virtual idx OnExecute(idx req);
            idx run();


            virtual bool checkETag(sStr & etagBuf, idx len, idx timeStamp)
            {
                return runAsBackEnd() ? false : sQPrideCGI::checkETag(etagBuf, len, timeStamp);
            }

            virtual void voutBinUncached(const void * buf, idx len, const char * etag, bool asAttachment, const char * flnmFormat, va_list marker)
            {
                if( runAsBackEnd() ) {
                    if( len ) {
                        outBinData(buf, len);
                    }
                } else {
                    sQPrideCGI::voutBinUncached(buf, len, etag, asAttachment, flnmFormat, marker);
                }
            }


    };
} // namespace slib

#endif // _QPrideCgi_qLib_hpp



