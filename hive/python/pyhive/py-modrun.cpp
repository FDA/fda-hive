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
#include <slib/std/pipe2.hpp>
#include <slib/std/regexp.hpp>
#include <slib/std/string.hpp>

#include <stdlib.h>

#ifdef _DEBUG
#ifndef DEFAULT_ENV_QPRIDE_BIN
#define DEFAULT_ENV_QPRIDE_BIN "/home/qpride/bin"
#endif
#endif

using namespace slib;


void printUsage()
{
    fprintf(stderr, "Usage: either\npy-modrun.os" SLIB_PLATFORM " SERVICE [EXTRA ARGS ...]\n" \
        "where SERVICE.py is a pyhive service file in the current directory;\n\n" \
        "or alternatively:\n\n" \
        "SERVICE.os" SLIB_PLATFORM " [EXTRA ARGS ...]\n" \
        "if SERVICE.os" SLIB_PLATFORM " is a symlink to py-modrun.os" SLIB_PLATFORM " and\n" \
        "SERVICE.py is a pyhive service file in the current directory.\n\n" \
        "Python will be loaded from ${QPRIDE_BIN}/prefix.os" SLIB_PLATFORM "\n");
}

static bool setupPythonEnv(sPipe2 & python)
{
    sStr qpride_bin;
    qpride_bin.addString(getenv("QPRIDE_BIN"));
#ifdef _DEBUG
    if( !qpride_bin.length() ) {
        qpride_bin.addString(DEFAULT_ENV_QPRIDE_BIN);
    }
#endif
    if( qpride_bin[qpride_bin.length() - 1] == '/' ) {
        qpride_bin.cut0cut(qpride_bin.length() - 1);
    }
    python.cmdLine().env("QPRIDE_BIN", qpride_bin);

    sStr python_path("%s/python3", qpride_bin.ptr());
    python.cmdLine().env("PYTHONPATH", python_path.ptr());

    python.cmdLine().exe("python3");

    return true;
};

int main(int argc, char ** argv)
{
    sStr svc;
    if( argc ) {
        const char * exe_name = argv[0];
        const char * exe_filename = exe_name ? sFilePath::nextToSlash(exe_name) : 0;
        if( exe_filename && exe_filename[0] ) {
            svc.addString(exe_filename);
            if( char * os_suffix = strstr(svc.ptr(), ".os" SLIB_PLATFORM) ) {
                svc.cut0cut(os_suffix - svc.ptr());
            }
            idx svc_len = svc.length();
            svc.addString(".py");
            if( sFile::exists(svc) ) {
                svc.cut0cut(svc_len);
            } else {
                svc.cut0cut(0);
            }
        }
    }

    if( argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) ) {
        printUsage();
        return 1;
    }

    idx first_py_iarg;
    if( svc.length() == 0 ) {
        first_py_iarg = 2;
        if( argc >= first_py_iarg ) {
            svc.addString(argv[first_py_iarg - 1]);
        }
        if( !svc.length() ) {
            printUsage();
            return 1;
        }
    } else {
        first_py_iarg = 1;
    }

    sPipe2 python;
    if( !setupPythonEnv(python) ) {
        return 1;
    }

    python.cmdLine().arg("-c");
    sStr python_script("import sys; import pyhive; pyhive.proc = pyhive.Proc('%s'); import %s; pyhive.proc.run(%s, sys.argv[1:])", svc.ptr(), svc.ptr(), svc.ptr());
    python.cmdLine().arg(python_script);

    for( idx i = first_py_iarg; i < argc; i++ ) {
        python.cmdLine().arg(argv[i]);
    }

    idx python_retcode = 0;
    sRC rc = python.execute(&python_retcode);
    if( rc ) {
        fprintf(stderr, "%s\n", rc.print());
        return python_retcode ? python_retcode : 1;
    }

    return python_retcode;
}
