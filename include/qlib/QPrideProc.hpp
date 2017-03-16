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
#ifndef _QPrideProc_qLib_hpp
#define _QPrideProc_qLib_hpp

#include <slib/core/tim.hpp>
#include <slib/std/online.hpp>
#include <slib/std/file.hpp>
#include <qlib/QPride.hpp>
#include <ulib/usr.hpp>
#include <ulib/uproc.hpp>

namespace slib
{
    class sSql;
    class sIO;

    class sQPrideProc: public sQPride
    {

        public:
            Service svc;
            sTime tmfix,tmExec;

            bool alwaysRun;
            idx loopCnt;
            idx noGrabs;
            idx maxMemSize;
            idx reqSliceId, reqSliceCnt; // within group
            idx prvReqId; // previous non zero reqID
            idx prvGrabReqID; // previous value onGrab has returned

            idx exitCode;
            idx inBundle;
            idx socketSelect,isProblemReported;
            idx doStdin;
            bool tempDBConnection;
            bool maxmemMailSent;
            idx isRegrab;
            idx rand_seed;
            idx sleepTimeOverride;
            sStr requestStage;

            sVec < sUsrProc > objs;
            idx lastInGroup;
            //char platformOS[128];

            sPS ps;

            sConIP udp;
            sVar reqForm, * pForm;

         public:
            sQPrideProc(const char * defline00, const char * service = "qm");
            virtual ~sQPrideProc();

            idx run(idx argc, const char *argv[]);
         public:

            enum eQPErr{ // CURRENT ACTION FOR THE REQUEST
                eQPErr_None=0,
                eQPErr_ServiceStopped_None,
                eQPErr_DB_LostConnection,
                eQPErr_ServiceStopped,
                eQPErr_TooManyJobs,
                eQPErr_Max
            } ;

        public:
            virtual idx OnGrab(idx forceReq=0);
            virtual bool OnLogOut(idx typ, const char * message){
                promptOK=sQPrideBase::OnLogOut(typ, message);
                return promptOK;
            }
            virtual bool OnCommand(const char * , const char * ){return false;}
            virtual idx OnSplit(idx ){return 0;}
            virtual idx OnExecute(idx ){return 0;}
            //! Called lazy in reqProgress before actual report
            /**
             * \param reqId - current request id
             * \return false to abort report and process
             */
            virtual bool OnProgress(idx reqId)
            {
                return true;
            }
            virtual idx OnCollect(idx ){return 0;}
            virtual idx OnReleaseRequest(idx ){return 0;}
            virtual idx OnFinalize(idx ){return 0;}

            virtual idx OnPrepare(idx ) {return 0;}
            virtual idx OnCompute(idx ){ return 0;}
            virtual idx OnComputeThread(idx , ThreadSpecific * ){ return 0;}
            virtual idx OnCleanup(idx){return 0;}

            virtual bool OnInit(void){return true;}
            virtual void OnQuit(void){};

            // show message on process command line in top/ps
            void psMessage(const char * fmt, ...);


        protected:
            idx executeThreads(idx req,idx rangeCnt, idx rangeStart=0);
            bool jobShouldRun(void) ;
            idx selectSleep(idx slpTm=0);
            //! Report memory statistics to db
            /**
                 \return true if memory abuse detected
             */
            bool memReport(const char * svcName);

        private:

            static void executeRequest(void * procthis);
            static void executeAThread(void * param);


            bool initializeTriggerPorts(void);
            void releaseTriggerPorts(void);

            bool executeCommand(const char * nam, const char * val);
            idx selectSleepSingle(idx slpTm=0);

            // used to alter command line for top/ps
            char * _argvBuf;
            int _argvBufLen;
            bool _argvBufChanged;

        public:
            idx hostNumInPool(Service * svc, idx * pCntList, idx * pmaxjob=0);

            const char * formValue(const char * prop, sStr * buf=0, const char * defaultValue=0, idx iObj=0);
            idx formIValue(const char * prop, idx defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                idx ival=defaultValue;
                if(p){
                    sscanf(p,"%" DEC,&ival);
                }
                return ival;
            }
            udx formUValue(const char * prop, udx defaultValue = 0, idx iObj = 0)
            {
                sStr t;
                const char * p = formValue(prop, &t, 0, iObj);
                udx uval = defaultValue;
                if( p )
                    sscanf(p, "%" UDEC, &uval);
                return uval;
            }
            real formRValue(const char * prop, real defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                real rval=defaultValue;
                if(p)sscanf(p,"%lf",&rval);
                return rval;
            }
            bool formBoolValue(const char * prop, bool defaultValue=false, idx iObj=0)
            {
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                bool bval=defaultValue;
                if(p) bval=sString::parseBool(p);
                return bval;
            }
            idx formBoolIValue(const char * prop, idx defaultValue=0, idx iObj=0)
            {
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                idx ival=defaultValue;
                if(p) ival=sString::parseIBool(p);
                return ival;
            }

            //! Parses an array property into 0-terminated substrings of a 00-terminated buffer
            /*!
                \param prop property name
                \param buf00 valid pointer to a sStr where the result will be stored
                \param iObj object index
                \returns Pointer to first parsed substring in buf00, or 0 on error

                Example: \code
                    sStr buf00;
                    if(formValues00("foo", &buf00)) {
                        for (const char *p = buf00.ptr(); p; p = sString::next00(p)) {
                            printf("value: %s\n", p);
                        }
                    }
                \endcode
            */
            const char * formValues00(const char *prop, sStr *buf00, const char * altSeparator=0, idx iObj=0);


            //! Parses an array property and appends values to a vector of integers
            /*!
                \param prop property name
                \param values valid pointer to a vector where the values will be stored
                \param iObj object index
                \return Number of values parsed, or -1 on error
            */
            idx formIValues(const char *prop, sVec<idx> *values, idx iObj=0);
            idx formUValues(const char *prop, sVec<udx> *values, idx iObj=0);
            idx formHiveIdValues(const char *prop, sVec<sHiveId> *values, idx iObj=0);

            //! Parses an array property and appends values to a vector of reals
            /*!
                \param prop property name
                \param values valid pointer to a vector where the values will be stored
                \param iObj object index
                \returns Number of values parsed, or -1 on error
            */
            idx formRValues(const char *prop, sVec<real> *values, idx iObj=0);

            //! OBSOLETE DO NOT USE, use req(Add|Get)File instead
            const char * destPath(sStr * buf, const char * flnmFmt, ... ) __attribute__((format(printf, 3, 4)));
            //! Add a file to request
            /*!
             * \param buf - added file path is appended to buf
             * \returns pointer to file path inside buf or 0 in case of failure (buf stays unchanged)
             */
            const char * reqAddFile(sStr & buf, const char * flnmFmt, ... ) __attribute__((format(printf, 3, 4)));
            //! Get a file associated with request earlier using reqAddFile
            /*!
             * \param buf - found file path is appended to buf
             * \returns pointer to file path inside buf or 0 in case its not found (buf stays unchanged)
             */
            const char * reqGetFile(sStr & buf, const char * flnmFmt, ... ) __attribute__((format(printf, 3, 4)));

            //! Report a request as alive and update its progress level in db if more than X seconds passed since the last reqProgress() call
            /*!
                \param items number of "items" (bytes, rows, records, ...) processed, or -1 to ignore; values less than -1 means ignore limits on db update frequency, and force record abs(items) in db
                \param progress current raw completed progress level; sNotIdx means \a items parameter will be used as raw progress level
                \param progressMax maximum raw progress level; negative value means progress level will not be updated in db
                \returns 1 normally, or 0 if process is requested to be stopped
                \note
                The raw progress level is not directly recorded in db.
                \note
                The value of \a items is recorded in the 'progress' column in db.
                \note
                The progress percentage (0-100) is recorded in the 'progress100' column in db; it is calculated as
                sQPrideBase::progress100Start + (progress / progressMax * sQPrideBase::progress100End)
            */
            idx reqProgress(idx items, idx progress, idx progressMax);
            //! see reqProgress
            static idx reqProgressStatic(void * param, idx items, idx progress, idx progressMax);
            //! see reqProgress
            static idx reqProgressFSStatic(void * param, idx items);

            //! Launch command, monitor and report its progress as change in size of the given path. <b>Also provides necessary updates of backend alive status</b>
            /*
                \param cmdline command line to be executed
                \param input [optional] stdin content
                \param path [optional] path to a file or directory increase in size of which will be report as items of progress
                \param log [optional] buffer for command stdout content
                \return exit code of the command
             */
            idx exec(const char * cmdline, const char * input, const char * path, sIO * log, idx sleepSecForExec = sNotIdx);

            bool isLastInGroup(const char * svcName=0 );
            bool isLastInMasterGroup(const char * svcName=0);
    };

}

#endif // _QPrideProc_qLib_hpp
