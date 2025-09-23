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
#include <qlib/QPrideGenericLauncher.hpp>

using namespace slib;




idx sQPrideGenericLauncher::OnExecute(idx req)
{


    sStr scriptDriver;

    if( !cmdLineTemplate ) {
        if( !cmdLineTemplateFile ) {
            const char * algorithmScript = formValue("algo");
            if(algorithmScript) {
                sUsrFile algorithmScriptFile(sHiveId(algorithmScript),user);
                algorithmScriptFile.getFile(cmdLineTemplateFile);
                algorithmScriptFile.propGet("script_driver", &scriptDriver);

                if(!regExpResultList00) {
                    algorithmScriptFile.propGet00("result_files_regex",&regExpResultList00);
                }
            }
        }

        if(cmdLineTemplateFile) {
            sFil f(cmdLineTemplateFile.ptr(), sMex::fReadonly);
            cmdLineTemplate.add(f.ptr(), f.length());
            cmdLineTemplate.add0();
        }
    }

    if( !cmdLineTemplate ) {
        reqSetInfo(req, eQPInfoLevel_Error, "No valid command line template specified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    reqProgress(1, 5, 100);
    reqSetInfo(req, eQPInfoLevel_Info, "Copying Files into the Working Directory.");


    launcherDir.cut(0);
    cfgStr(&launcherDir, 0, "qm.genericLauncherFolder", "/tmp/");
    launcherDir.printf("qp_generic_%" DEC "/",req);
    if( strcmp( requestStage.ptr(), "init" )==0) {
        sDir::removeDir(launcherDir,true);
        if( ! sDir::makeDir(launcherDir) ){
            reqSetInfo(req, eQPInfoLevel_Error, "could not create a launch directory");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
    }

    sStr sessionID;
    formValue("sessionID", &sessionID);
    idx maxIterMinutes = formIValue("maxIterMinutes", 4 * 60);
    idx resubmitMode = formBoolValue("resubmitMode", false);



    sStr progressFlnm("%sqp-Progress.txt", launcherDir.ptr());
    sStr triggerFlnm("%sqp-Trigger.txt", launcherDir.ptr());

    reqProgress(2, 6, 100);
    reqSetInfo(req, eQPInfoLevel_Info, "Preparing third party tool.");

    sStr realCommandLine;

    idx actually_inside_prepareForLaunch = 0;
    if( strcmp(requestStage, "init") == 0 ) {
        logOut(eQPLogType_Debug, "Request Stage: [ %s ] in Get realCommandLine", requestStage.ptr());
        realCommandLine.printf("cd %s;", launcherDir.ptr());
        sUsrInternalQueryEngine sQLanLocal(*user);
        if( !sQLan ) {
            sQLan = &sQLanLocal;
        }
        if( objs.dim() != 0 ) {
            sQLan->registerBuiltinThis(objs[0].Id());
        } else if( serviceToRegister ) {
            sQLan->registerBuiltinThisPropertiesForm(serviceToRegister.ptr(), *pForm);
        }
        sStr progressFlnm("%sqp-Progress.txt", launcherDir.ptr());
        sQLan->registerBuiltinStringPtr("progressFile", &progressFlnm);

        sQLan->registerBuiltinStringPtr("workDir", &launcherDir);
        sQLan->registerBuiltinStringPtr("sessionID", &sessionID);

        sStr platformName("os" SLIB_PLATFORM);
        sQLan->registerBuiltinStringPtr("os", &platformName);

        sStr url;
        cfgStr(&url, 0, "internalWWW");
        sQLan->registerBuiltinStringPtr("url", &url);

        registerCallbackFunctions("path" _ "dirPath" _ "spObjPath" __);
        if( addDispatchedFunctions00 ) {
            registerCallbackFunctions(addDispatchedFunctions00);
        }
        sStr scriptFileName("%sqp-script.sh", launcherDir.ptr());
        sQLan->registerBuiltinStringPtr("script", &scriptFileName);

        sStr errorMsg;
        sVariant evaledCommandLine, evaledScriptDriver;
        sQLan->parseTemplate(cmdLineTemplate, 0, &errorMsg);
        if( !errorMsg ) {
            sQLan->eval(evaledCommandLine, &errorMsg);
        }
        if( errorMsg ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Invalid command line");
            reqSetStatus(req, eQPReqStatus_ProgError);
            logOut(eQPLogType_Error, "%s", errorMsg.ptr());
            sQLan = 0;
            return 0;
        }
        sQLan->reset();
        if( scriptDriver ) {
            sQLan->parseTemplate(scriptDriver, 0, &errorMsg);
            if( !errorMsg ) {
                sQLan->eval(evaledScriptDriver, &errorMsg);
            }
            if( errorMsg ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Invalid script driver line to evaluate");
                reqSetStatus(req, eQPReqStatus_ProgError);
                logOut(eQPLogType_Error, "%s", errorMsg.ptr());
                sQLan = 0;
                return 0;
            }
            sQLan->reset();
        }
        sQLan = 0;

        if( prepareForLaunch(&realCommandLine, &errorMsg, &actually_inside_prepareForLaunch) ) {
            logOut(eQPLogType_Error, "%s", errorMsg.ptr());
            return 0;
        }
        if( scriptDriver ) {
            sStr scriptFileName("%sqp-script.sh", launcherDir.ptr());
            sFil sf(scriptFileName);
            if( sf.ok() ) {
                const char * p = evaledCommandLine.asString();
                sf.add(p, sLen(p));
                sf.destroy();
                sFile::chmod(scriptFileName);
                realCommandLine.printf("nohup %s >%soutput.log &", evaledScriptDriver.asString(), launcherDir.ptr());
            }
        } else {
            if( !actually_inside_prepareForLaunch ) {
                realCommandLine.printf("nohup %s >%soutput.log &", evaledCommandLine.asString(), launcherDir.ptr());
            }
        }
    }

    logOut(eQPLogType_Debug, "Request Stage: [ %s ]", requestStage.ptr(0));
    if( strcmp(requestStage.ptr(), "init") == 0 ) {

        logOut(eQPLogType_Debug, "Launching command line: '%s'", realCommandLine.ptr());
        reqSetInfo(req, eQPInfoLevel_Info, "Launching third party tool.");
        reqProgress(2, 10, 100);
        idx res = system(realCommandLine);


        if( res ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Could not launch application");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        } else if (resubmitMode) {
            reqSetData(req, "_reqStage", "monitoring");
        }
    }

    reqProgress(3, 15, 100);
    reqSetInfo(req, eQPInfoLevel_Info, "Monitoring the application while it works.");
    reqSetStatus(req, eQPReqStatus_Running);

    sStr buf, errmsg;
    idx prgCount = 0, prgPercent = 0, status = 0;
    for(idx iter = 0; iter < maxIterMinutes; ++iter) {
        sFil prg(progressFlnm, sMex::fReadonly);
        if( prg ) {
            buf.cut(0);
            sString::searchAndReplaceSymbols(&buf, prg.ptr(), prg.length(), ",", 0, 0, true, false, true, false);
            prg.destroy();
            const char * p = buf.ptr();
            if( !p ) {
                continue;
            }
            sscanf(p, "%" DEC, &prgCount);
            p = sString::next00(p);
            if( !p ) {
                continue;
            }
            sscanf(p, "%" DEC, &prgPercent);
            p = sString::next00(p);
            if( !p ) {
                continue;
            }
            sString::xscanf(p, "%n=0^any^waiting^processing^running^suspended^done^killed^progError^SysError^error;", &status);
            p = sString::next00(p);
            if( p && *p ) {
                logOut(eQPLogType_Info, "%s\n", p);
            }
            if( status > eQPReqStatus_Running ) {
                if( status > eQPReqStatus_Done ) {
                    errmsg.printf(0, "Error reported: %s", p && *p ? p : "unknown");
                }
                break;
            }
            if( !reqProgress(prgCount, prgPercent, 100) ) {
                logOut(eQPLogType_Debug, "Interrupted by user");
                break;
            }
            reqSetStatus(req, status);
        } else {
            logOut(eQPLogType_Warning, "Oops no progress file found [iter: %" DEC "]\n",iter );
            reqProgress(0, 0, 100);
        }
        if(resubmitMode) {
            reqReSubmit(req, 60);
            return 0;
        } else {
            sleepSeconds(60);
        }
    }




    if( regExpResultList00 ) {
        sFilePath flnm;
        sStr dst;
        for(const char * regExpResultList = regExpResultList00; regExpResultList; regExpResultList = sString::next00(regExpResultList)) {
            sDir results;
            results.list(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive), launcherDir, regExpResultList, 0, 0);
            if( results ) {
                for(const char * ptr = results; ptr; ptr = sString::next00(ptr)) {
                    flnm.cut(0);dst.cut(0);
                    flnm.makeName(ptr,"%%flnm");
                    reqAddFile(dst, "%s", flnm.ptr());
                    if( deleteWhenFinish && (!errmsg || !errmsg.length()) ) {
                        sFile::rename(ptr, dst.ptr());
                    } else {
                        sFile::copy(ptr, dst.ptr());
                    }
                    logOut(eQPLogType_Debug, "moving %s to %s\n", ptr, dst.ptr());
                }
            }
        }
    }


    if( errmsg ) {
        reqSetInfo(req, eQPInfoLevel_Error, "%s", errmsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }



    if( deleteWhenFinish ) {
        sDir::removeDir(launcherDir);
    }

    reqProgress(prgCount, prgPercent, 100);
    reqSetStatus(req, eQPReqStatus_Done);


    return 0;
}
