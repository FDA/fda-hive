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

#include <slib/std/file.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

using namespace slib;

idx sPipe::exeFile(const char * commandLine, const char * stdinFile, const char * stdoutFile, const char * stderrFile)
{
    sStr cmd;
    if( stderrFile )
        cmd.printf("( ");
    cmd.printf("%s", commandLine);
    if( stdoutFile )
        cmd.printf(" > %s", stdoutFile);
    if( stdinFile )
        cmd.printf(" < %s", stdinFile);
    if( stderrFile )
        sprintf(cmd, " ) >& %s", stderrFile);

    return system(cmd);
}

struct setupExeFile00Param
{
    const char * stdinFile;
    const char * stdoutFile;
    const char * stderrFile;
};

static void setupExeFile00(void * param_)
{
    const setupExeFile00Param * param = static_cast<setupExeFile00Param*>(param_);
    if( param->stdinFile ) {
        freopen(param->stdinFile, "r", stdin);
    }
    if( param->stdoutFile ) {
        freopen(param->stdoutFile, "w", stdout);
    }
    if( param->stderrFile ) {
        freopen(param->stderrFile, "w", stderr);
    }
}

idx sPipe::exeFile00(const char * commandLine00, const char * stdinFile, const char * stdoutFile, const char * stderrFile)
{
    setupExeFile00Param param = { stdinFile, stdoutFile, stderrFile };
    return sPS::execute00(commandLine00, setupExeFile00, &param);
}

char * sPipe::exeSys( sStr * buf, const char * commandLine, const char * stdinBuf, idx lenstdin, idx * retCode)
{
    sStr tt, ti;
    if( stdinBuf ) {
        idx i = 0;
        do {
            ti.printf(0, "%sslib.%u.in", tt.ptr(), rand());
        } while( ++i < 100 && !sFile::exists(ti) );
        sFile::remove(ti);
        sFil dst(ti);
        dst.add(stdinBuf, lenstdin ? lenstdin : sLen(stdinBuf));
    }
    if( buf ) {
#ifdef SLIB_WIN
        tt.printf("C:/TEMP/");
#else
        tt.printf("/tmp/");
#endif
        tt.printf("slib.%u.ps", rand());
    }
    idx r;
    if( !retCode ) {
        retCode = &r;
    }
    *retCode = sPipe::exeFile(commandLine, ti, buf ? tt.ptr() : 0, 0);
    if( buf ) {
        int fd = open(tt, O_RDONLY);
        if( fd != -1 ) {
            idx to_read = sFile::size(tt);
            buf->resize(to_read);
            ssize_t q = 0, q1 = 0;
            do {
                q1 = read(fd, buf->ptr(q), to_read - q);
                q += q1;
            } while( q1 > 0 );
            if( q1 == -1 ) {
                *retCode = -1;
#if _DEBUG
                fprintf(stderr, "failed to read output file '%s': %s\n", tt.ptr(), strerror(errno));
#endif
            }
            close(fd);
        } else {
            *retCode = -1;
#if _DEBUG
            fprintf(stderr, "failed to open output file '%s'\n", tt.ptr());
#endif
        }
    }
    if( ti ) {
        sFile::remove(ti);
    }
    if( tt ) {
        sFile::remove(tt);
    }
    return buf ? buf->ptr() : 0;
}

char * sPipe::exePipe( sIO * dst, const char * commandLine, const char * inputBuf, idx * retCode)
{
    FILE * pipe = popen( commandLine, "r");
    sStr Buf; char * maxLine=Buf.resize(sSizePage);

    if( !pipe)return (0);
    if(inputBuf && *inputBuf) fputs( inputBuf, pipe );

    while( !feof( pipe ) ) {
        maxLine[0]=0;
        fgets( maxLine, sSizePage-1, pipe );
        if(dst) {
            dst->add(maxLine,sLen(maxLine));
        }
    }
    idx ret;
    if( !retCode ) {
        retCode = &ret;
    }
    *retCode = pclose( pipe );
    return dst ? dst->ptr() : 0;
}

struct sPipe_exeFS
{
        const char * input;
        const char * cmd;
        bool running;
        sIO * dst;
        idx * retval;
};

static
void * sPipe_exeFS_thread(void * arg)
{
    if( arg ) {
        sPipe_exeFS * data = (sPipe_exeFS *) arg;
        sPipe::exePipe(data->dst, data->cmd, data->input, data->retval);
        data->running = false;
    }
    pthread_exit(0);
}

idx sPipe::exeFS(sIO * dst, const char * cmdline, const char * inputBuf, callbackFuncType callbackFunc, void * callbackParam, const char * FSpath, const udx sleepSec)
{
    idx retval = -10;
    if( callbackFunc ) {
        const idx initial = FSpath && FSpath[0] ? sDir::size(FSpath) : -1;
        sPipe_exeFS data = {
            inputBuf,
            cmdline,
            true,
            dst,
            &retval };
        pthread_t tid;
        if( pthread_create(&tid, 0, sPipe_exeFS_thread, (void *) &data) == 0 ) {
            pthread_detach(tid);
            while( data.running ) {
                const idx sz = initial < 0 ? 0 : sDir::size(FSpath);
                const bool stop = callbackFunc(callbackParam, sz <= initial ? 0 : sz - initial) == 0;
                if( pthread_kill(tid, stop ? 9 : 0) != 0 ) {
                    pthread_cancel(tid);
                    retval = (!stop && data.running) ? -20 : 0;
                    break;
                }
                sleepSeconds(sleepSec);
            }
        } else {
            retval = -30;
        }
        callbackFunc(callbackParam, initial < 0 ? 0 : -sDir::size(FSpath));
    } else {
        exePipe(dst, cmdline, inputBuf, &retval);
    }
    return retval;
}


