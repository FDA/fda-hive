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
#include <ulib/ufile.hpp>
#include <violin/hivegenericlauncher.hpp>
#include <memory>

#define PIDFNAME ".hive_pid"

using namespace slib;

static const char* getWorkDirName = "getWorkDir";

class sHiveGenericLauncher_getWorkDir : public qlang::BuiltinFunction
{
        idx m_jobId;
        const qlang::BuiltinFunction* m_wdParent;

    public:
        sHiveGenericLauncher_getWorkDir(const qlang::BuiltinFunction* wdFunc)
            : m_jobId(0), m_wdParent(wdFunc)
        {
            _name.printf(0, "builtin %s() generic launcher function", getWorkDirName);
        }

        void setJobId(const idx jobId)
        {
            m_jobId = jobId;
        }

        virtual bool call(sVariant& result, qlang::Context& ctx, sVariant* topic, sVariant* args, idx nargs) const {
            result.setNull();
            if( m_wdParent ) {
                sVariant res;
                m_wdParent->call(res, ctx, topic, args, nargs);
                if( !res.isNull() && !res.isNullish() ) {
                    if( m_jobId > 0 ) {
                        result.setSprintf("%s%" DEC "/", res.asString(), m_jobId);
                    } else {
                        result.setSprintf("%s", res.asString());
                    }
                    if( !sDir::makeDir(result.asString(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) ) {
                        ctx.setError(qlang::EVAL_OTHER_ERROR, "failed to establish temp working directory");
                        qlang::sUsrInternalContext * ictx = dynamic_cast<qlang::sUsrInternalContext*>(&ctx);
                        if( ictx ) {
                            ictx->getQPride()->logOut(sQPrideBase::eQPLogType_Error, "failed to create directory '%s'", result.asString());
                        }
                        return false;
                    }
                }
            }
            return !result.isNull();
        }
};

std::unique_ptr<sHiveGenericLauncher_getWorkDir> getWorkDirFunc;

sUsrQueryEngine * sHiveGenericLauncher::queryEngineInit(sUsrQueryEngine * qe)
{
    TParent::queryEngineInit(qe);
    if( qe ) {
        if( !getWorkDirFunc ) {
            getWorkDirFunc.reset(new sHiveGenericLauncher_getWorkDir(qe->getContext().getBuiltin(getWorkDirName)));
        }
        qe->registerBuiltinFunction(getWorkDirName, *getWorkDirFunc);
    }
    return qe;
}

bool sHiveGenericLauncher::readLog(const char * workDir, const char * logpath, const bool final)
{
    const idx sz = sFile::size(logpath);
    if( sz != m_logSize ) {
        sFil log(logpath);
        if( log.ok() ) {
            sStr tmpLog, logLines;
            logLines.mex()->init(sFile::mktemp(tmpLog, workDir));
            logLines.add(log.ptr(m_logPos), log.length() - m_logPos);
            log.destroy();
            if( !final ) {
                const char * n = strrchr(logLines.ptr(), '\n');
                const char * r = n ? n : strrchr(logLines.ptr(), '\r');
                if( n || r ) {
                    logLines.cut((n > r ? n : r) - logLines.ptr() + 1);
                } else {
                    logLines.cut0cut();
                }
            }
            if( logLines.length() ) {
                m_logPos += logLines.length();
                logLines.add0(10);
                sStr workDir00;
                workDir00.addString(workDir);
                workDir00.add0(2);
                sString::searchAndReplaceStrings(logLines.ptr(), logLines.length(), workDir00.ptr(0), "<workdir>" __, 0, true);
                sString::searchAndReplaceSymbols(logLines.ptr(), logLines.length(), "\n\r", 0, 0, true, true, true, false);
                logLines.add0(3);
                for(const char * l = logLines; l; l = sString::next00(l)) {
                    if( *l ) {
                        reqSetInfo(reqId, eQPInfoLevel_Info, "%s", l);
                    }
                }
            }
            logLines.destroy();
            sFile::remove(tmpLog);
            m_logSize = sz;
            return true;
        }
    }
    return false;
}

idx sHiveGenericLauncher::OnExecute(idx req)
{
    sStr scriptDriver, regExpResultList00, cmdLineTemplate, cmdLineTemplateFile;

    if( !cmdLineTemplate ) {
        if( !cmdLineTemplateFile ) {
            const char *algorithmScript = formValue("algo");
            if( algorithmScript ) {
                sUsrFile algorithmScriptFile(sHiveId(algorithmScript), user);
                algorithmScriptFile.getFile(cmdLineTemplateFile);
                algorithmScriptFile.propGet("script_driver", &scriptDriver);
                if( !regExpResultList00 ) {
                    algorithmScriptFile.propGet00("result_files_regex", &regExpResultList00);
                }
            }
        }
        if( cmdLineTemplateFile ) {
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
    reqProgress(1, 1, 100);
    reqSetInfo(req, eQPInfoLevel_Info, "Preparing Working Directory.");
    const udx maxIterSec = formUValue("maxIterMinutes", 4 * 60) * 60;

    sUsrQueryEngine *ql = queryEngineFactory();
    if( !ql ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Failed to initialize query engine");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    getWorkDirFunc->setJobId(jobId);
    sStr errorMsg;
    sVariant wd;
    const bool retval = ql->evalTemplate("$(getWorkDir())", 0, wd, &errorMsg);
    if( !retval ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Working directory setup failed");
#if _DEBUG
        reqSetInfo(req, eQPInfoLevel_Error, "%s", errorMsg.ptr());
#else
        logOut(eQPLogType_Error, "%s", errorMsg.ptr());
#endif
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    sDir temps;
    temps.list(sFlag(sDir::bitSubdirs) | sFlag(sDir::bitFiles) | sFlag(sDir::bitEntryFlags), wd.asString(), "*");
    for(idx i = 0; i < temps.dimEntries(); i++) {
        if( temps.getEntryFlags(i) & sDir::fIsDir ) {
            sDir::removeDir(temps.getEntryPath(i));
        } else {
            sFile::remove(temps.getEntryPath(i));
        }
    }

    const char * progresslogName = "qp-Progress.txt";
    const char * scriptName = "qp-script.sh";
    const char * outputlogName = "qp-output.log";

    sStr progressFlnm("%s%s", wd.asString(), progresslogName);
    ql->registerBuiltinStringPtr("progressFile", &progressFlnm);
    sStr scriptFileName("%s%s", wd.asString(), scriptName);
    ql->registerBuiltinStringPtr("script", &scriptFileName);
    sStr pidFileName("%s%s", wd.asString(), PIDFNAME);

    reqProgress(2, 1, 100);
    reqSetInfo(req, eQPInfoLevel_Info, "Preparing third party tool.");

    sStr realCommandLine;
    idx actually_inside_prepareForLaunch = 0;
    if( strcmp(requestStage, "init") == 0 ) {
        logOut(eQPLogType_Debug, "Request Stage: [ %s ] in Get realCommandLine", requestStage.ptr());
        realCommandLine.printf("cd %s;", wd.asString());

        sVariant evaledCommandLine;
        bool retval = ql->evalTemplate(cmdLineTemplate, 0, evaledCommandLine, &errorMsg);
        if( !retval ) {
#if _DEBUG
            reqSetInfo(req, eQPInfoLevel_Error, "%s", errorMsg.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", cmdLineTemplate.ptr());
#else
            logOut(eQPLogType_Error, "%s", errorMsg.ptr());
            logOut(eQPLogType_Error, "%s", cmdLineTemplate.ptr());
#endif
            reqSetInfo(req, eQPInfoLevel_Error, "Invalid command line");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        sVariant evaledScriptDriver;
        if( scriptDriver ) {
            retval = ql->evalTemplate(scriptDriver, 0, evaledScriptDriver, &errorMsg);
            if( !retval ) {
#if _DEBUG
                reqSetInfo(req, eQPInfoLevel_Error, "%s", errorMsg.ptr());
#else
                logOut(eQPLogType_Error, "%s", errorMsg.ptr());
#endif
                reqSetInfo(req, eQPInfoLevel_Error, "Invalid script driver");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
        }

        if( scriptDriver ) {
            sStr scriptFileName("%sqp-script.sh", wd.asString());
            sFil sf(scriptFileName);
            if( sf.ok() ) {
                sStr script;
                sString::searchAndReplaceStrings(&script, evaledCommandLine.asString(), 0, "#!/bin/bash\n" __, "#!/bin/bash\necho -n $$ >" PIDFNAME "\n" __, sIdxMax, true);
                sf.printf(0, "%s", script.ptr());
                sf.destroy();
                sFile::chmod(scriptFileName);
                realCommandLine.printf("nohup setsid %s 1>%s%s 2>&1 &", evaledScriptDriver.asString(), wd.asString(), outputlogName);
            }
        } else {
            if( !actually_inside_prepareForLaunch ) {
                realCommandLine.printf("nohup setsid %s 1>%s%s 2>&1 &", evaledCommandLine.asString(), wd.asString(), outputlogName);
            }
        }
    }

    if( reqGetStatus(reqId) > eQPReqStatus_Running ) {
        return 0;
    }

    logOut(eQPLogType_Debug, "Request Stage: [ %s ]", requestStage.ptr(0));
    if( strcmp(requestStage.ptr(), "init") == 0 ) {

        logOut(eQPLogType_Debug, "Launching command line: '%s'", realCommandLine.ptr());
        reqSetInfo(req, eQPInfoLevel_Info, "Launching third party tool.");
        reqProgress(2, 1, 100);
        idx res = system(realCommandLine);
        if( res ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Could not launch application");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
    }

    sStr buf, logpath("%s%s", wd.asString(), outputlogName), prevLogMsg;
    idx prgStatus = eQPReqStatus_Running, wdDirSize = 0, progressFlTm = 0;
    m_logPos = 0;
    m_logSize = 0;
    idx prgCount = sNotIdx, prgPercent = sNotIdx;
    idx scriptPid = 0;
    reqProgress(3, 1, 100);
    reqSetStatus(req, prgStatus);
    const udx sleepSec = 5;
    bool endedExternally = false;
    for(udx iter = 0; iter < maxIterSec; iter += sleepSec) {
        if( !scriptPid ) {
            sFil pidf(pidFileName, sMex::fReadonly);
            if( pidf.ok() && pidf.length() > 0 ) {
                if( sscanf(pidf.ptr(0), "%" DEC, &scriptPid) != 1 ) {
                    scriptPid = 0;
                }
            }
        }
        const idx tm = sFile::time(progressFlnm);
        if( tm != progressFlTm ) {
            progressFlTm = tm;
            sFil prg(progressFlnm, sMex::fReadonly);
            if( prg.ok() ) {
                buf.cut(0);
                sString::searchAndReplaceSymbols(&buf, prg.ptr(), prg.length(), ",", 0, 0, true, false, true, false);
                prg.destroy();
                prgCount = sNotIdx;
                prgPercent = sNotIdx;
                const char *p = buf.ptr();
                if( p && *p ) {
                    sscanf(p, "%" DEC, &prgCount);
                }
                p = sString::next00(p);
                if( p && *p ) {
                    sscanf(p, "%" DEC, &prgPercent);
                }
                p = sString::next00(p);
                if( !reqProgress(prgCount, prgPercent, 100) ) {
                    readLog(wd.asString(), logpath.ptr(), true);
                    reqSetInfo(reqId, eQPInfoLevel_Info, "Interrupted by user");
                    prgStatus = reqGetStatus(reqId);
                    endedExternally = true;
                    break;
                } else if( p ) {
                    sString::xscanf(p, "%n=0^any^waiting^processing^running^suspended^done^killed^progError^SysError^error;", &prgStatus);
                }
                p = sString::next00(p);
                if( prgStatus > eQPReqStatus_Running ) {
                    if( prgStatus > eQPReqStatus_Done ) {
                        errorMsg.printf(0, "Error reported: %s", (p && *p) ? p : "unknown");
                    }
                    break;
                } else if( p && *p && (!prevLogMsg || strcasecmp(p, prevLogMsg.ptr()) != 0) ) {
                    logOut(eQPLogType_Info, "%s", p);
                    prevLogMsg.printf(0, "%s", p);
                }
            } else if( iter > 1 ) {
                logOut(eQPLogType_Warning, "Oops no progress file found [iter: %" UDEC "]", iter);
                reqProgress(sNotIdx, sNotIdx, 100);
            }
        } else if( !reqProgress(prgCount, prgPercent, 100) ) {
            reqSetInfo(reqId, eQPInfoLevel_Info, "Interrupted by user");
            prgStatus = reqGetStatus(reqId);
            break;
        } else if( reqGetStatus(reqId) > eQPReqStatus_Running ) {
            endedExternally = true;
            break;
        }
        if( readLog(wd.asString(), logpath.ptr()) ) {
            iter = 0;
        } else {
            idx newSize = sDir::size(wd.asString(), true, true);
            if( wdDirSize != newSize ) {
                wdDirSize = newSize;
                iter = 0;
            }
        }
        sleepSeconds(sleepSec);
    }
    if( scriptPid ) {
        sPS ps;
        ps.killProcess(-scriptPid, 9);
    }
    readLog(wd.asString(), logpath.ptr(), true);
    if( endedExternally ) {
    } else if( prgStatus == eQPReqStatus_Running ) {
        prgStatus = eQPReqStatus_ProgError;
        errorMsg.printf(0, "Process is not responding, terminated");
    } else if( prgStatus == eQPReqStatus_Done && regExpResultList00 ) {
        sFilePath flnm;
        sStr dst;
        for(const char * mask = regExpResultList00; mask; mask = sString::next00(mask)) {
            if( strcmp(mask, "*") == 0 || strcmp(mask, "*.*") == 0 ) {
                errorMsg.printf(0, "DEVELOPER ERROR: result_files_regex mask '%s' is prohibited", mask);
                break;
            }
            sDir results;
            results.list(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitSubdirs), wd.asString(), mask, 0, 0);
            if( results ) {
                bool copyFailed = false;
                for(const char *ptr = results; ptr; ptr = sString::next00(ptr)) {
                    flnm.cut(0);
                    flnm.makeName(ptr, "%%flnm");
                    if( strcmp(flnm.ptr(), progresslogName) != 0 && strcmp(flnm.ptr(), scriptName) != 0 && strcmp(flnm.ptr(), outputlogName) != 0 ) {
                        dst.cut(0);
                        reqAddFile(dst, "%s", flnm.ptr());
                        logOut(eQPLogType_Debug, "picking up result dir/file '%s' to '%s'", ptr, dst.ptr());
                        if( sDir::exists(ptr) ) {
                            if( !sDir::copyDir(ptr, dst.ptr(), false)) {
                                logOut(eQPLogType_Error, "Dir copy failed '%s' to '%s'", ptr, dst.ptr());
                                copyFailed = true;
                                break;
                            }
                        } else {
                            if( !sFile::rename(ptr, dst.ptr()) ) {
                                logOut(eQPLogType_Error, "File copy failed '%s' to '%s'", ptr, dst.ptr());
                                copyFailed = true;
                                break;
                            }
                        }
                    }
                }
                if( copyFailed ) {
                    errorMsg.printf(0, "Failed to save computation results");
                    break;
                }
            }
        }
    }
    if( endedExternally ) {
#if !_DEBUG
        sDir::removeDir(wd.asString());
#endif
    } else if( errorMsg ) {
        reqSetInfo(req, eQPInfoLevel_Error, "%s", errorMsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    } else if( reqProgress(sNotIdx, 100, 100) ) {
        reqSetStatus(req, prgStatus);
#if !_DEBUG
        sDir::removeDir(wd.asString());
#endif
    }
    return 0;
}
