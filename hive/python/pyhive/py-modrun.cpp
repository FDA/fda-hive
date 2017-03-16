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

#ifndef DEFAULT_ENV_QPRIDE_BIN
#define DEFAULT_ENV_QPRIDE_BIN "/home/qpride/bin"
#endif

using namespace slib;

// This is written in C and not Bash purely to allow tracking pyhive-based requests
// by Linux command line substring (Bash rewrites the command line). Needed until
// we can track launched requests by pid (which is non-trivial, since it would require
// special support for SGE and other environments where the pid is not immediately
// known).

void printUsage()
{
    fprintf(stderr, "Usage: either\npy-modrun.os" SLIB_PLATFORM " SERVICE [EXTRA ARGS ...]\n" \
        "where SERVICE.py is a pyhive service file in the current directory;\n\n" \
        "or alternatively:\n\n" \
        "SERVICE.os" SLIB_PLATFORM " [EXTRA ARGS ...]\n" \
        "if SERVICE.os" SLIB_PLATFORM " is a symlink to py-modrun.os" SLIB_PLATFORM " and\n" \
        "SERVICE.py is a pyhive service file in the current directory.\n\n" \
        "Python will be loaded from ${QPRIDE_BIN}/python-2.7.os" SLIB_PLATFORM "\n");
}

idx getElRelease(const char * filename)
{
    sFil release(filename, sMex::fReadonly);
    if( release.ok() && release.ptr() ) {
        sRegExp re("release ([0-9]+)[^0-9]", 0);
        sVec<sMex::Pos> re_result;
        if( re.exec(re_result, release.ptr(), release.length()) > 1 ) {
            return atoidx(release.ptr(re_result[1].pos));
        }
    }
    return 0;
}

static bool setupPythonEnv(sPipe2 & python)
{
    // : ${QPRIDE_BIN:=/home/qpride/bin}
    sStr qpride_bin;
    qpride_bin.addString(getenv("QPRIDE_BIN"));
    if( !qpride_bin.length() ) {
        qpride_bin.addString(DEFAULT_ENV_QPRIDE_BIN);
    }
    if( qpride_bin[qpride_bin.length() - 1] == '/' ) {
        qpride_bin.cut0cut(qpride_bin.length() - 1);
    }
    python.setEnv("QPRIDE_BIN", qpride_bin);

    // : ${os:=Linux}
    python.setEnv("os", SLIB_PLATFORM);

    // HIVE_PY_PKG=python-2.7.os${os}
    // # detect CentOS 6 and RHEL 6 - need different binaries for their ancient libc version
    // if [[ $os = "Linux" ]] && ( grep -E 'release 6([^0-9]|$)' /etc/redhat-release 2> /dev/null || grep -E 'release 6([^0-9]|$)' /etc/centos-release 2> /dev/null ); then
    //     HIVE_PY_PKG=${HIVE_PY_PKG}.el6
    // fi
    // HIVE_PY_HOME=${QPRIDE_BIN}/${HIVE_PY_PKG}
    sStr hive_py_home("%s/python-2.7.os%s", qpride_bin.ptr(), SLIB_PLATFORM);
    if( strcmp(SLIB_PLATFORM, "Linux") == 0 ) {
        // check if we need binaries for CentOS/RHEL 6 or 7; fall back to default
        idx el_release = getElRelease("/etc/redhat-release");
        if( !el_release ) {
            el_release = getElRelease("/etc/centos-release");
        }

        if( el_release == 6 || el_release == 7 ) {
            hive_py_home.printf(".el%" DEC, el_release);
        }
    }

    idx hive_py_home_len = hive_py_home.length();
    if( !sDir::exists(hive_py_home) ) {
        fprintf(stderr, "python-2.7 for " SLIB_PLATFORM " not found in %s (using QPRIDE_BIN=%s)\n", hive_py_home.ptr(), qpride_bin.ptr());
        return false;
    }

    // export PYTHONHOME=${HIVE_PY_HOME}/usr
    hive_py_home.printf("/usr");
    python.setEnv("PYTHONHOME", hive_py_home.ptr());
    hive_py_home.cut0cut(hive_py_home_len);

    // export PYTHONPATH=${QPRIDE_BIN}/python2.7:${HIVE_PY_HOME}/usr/lib/python2.7
    sStr python_path("%s/python2.7:%s/usr/lib/python2.7", qpride_bin.ptr(), hive_py_home.ptr());
    python.setEnv("PYTHONPATH", python_path.ptr());

    // export PYTHONIOENCODING=utf-8
    python.setEnv("PYTHONIOENCODING", "utf-8");

    // for matplotlib - use a non-interactive, non-X mode by default
    // : ${MPLBACKEND:=Agg}
    // export MPLBACKEND
    const char * old_mplbackend = getenv("MPLBACKEND");
    if( !old_mplbackend || !old_mplbackend[0] ) {
        python.setEnv("MPLBACKEND", "Agg");
    }

    // for libfontconfig, pango, other freedestkop stuff

    // export XDG_CONFIG_HOME=${HIVE_PY_HOME}/etc
    hive_py_home.cutAddString(hive_py_home_len, "/etc");
    python.setEnv("XDG_CONFIG_HOME", hive_py_home.ptr());
    hive_py_home.cut0cut(hive_py_home_len);

    // export FONTCONFIG_PATH=${HIVE_PY_HOME}/etc/fonts
    hive_py_home.cutAddString(hive_py_home_len, "/etc/fonts");
    python.setEnv("FONTCONFIG_PATH", hive_py_home.ptr());
    hive_py_home.cut0cut(hive_py_home_len);

    // for text UI

    // export TERMINFO=${HIVE_PY_HOME}/usr/lib/terminfo
    hive_py_home.cutAddString(hive_py_home_len, "/usr/lib/terminfo");
    python.setEnv("TERMINFO", hive_py_home.ptr());
    hive_py_home.cut0cut(hive_py_home_len);

    // for launching things

    // export PATH=${HIVE_PY_HOME}/usr/bin:${PATH}
    const char * old_path = getenv("PATH");
    sStr path("%s/usr/bin", hive_py_home.ptr());
    if( old_path && old_path[0] ) {
        path.printf(":%s", old_path);
    }
    python.setEnv("PATH", path.ptr());

    // export LD_LIBRARY_PATH=${HIVE_PY_HOME}/usr/lib:${HIVE_PY_HOME}/lib:${LD_LIBRARY_PATH}
    const char * old_ld_library_path = getenv("LD_LIBRARY_PATH");
    sStr ld_library_path("%s/usr/lib:%s/lib", hive_py_home.ptr(), hive_py_home.ptr());
    if( old_ld_library_path && old_ld_library_path[0] ) {
        ld_library_path.printf(":%s", old_ld_library_path);
    }
    python.setEnv("LD_LIBRARY_PATH", ld_library_path.ptr());

    hive_py_home.cutAddString(hive_py_home_len, "/usr/bin/python");
    python.setExe(hive_py_home.ptr());

    return true;
};

int main(int argc, char ** argv)
{
    // if the executable name is called SERVICE or SERVICE.osLinux, and SERVICE.py exists
    // in the currect directory, then assume service name is SERVICE
    sStr svc;
    if( argc ) {
        const char * exe_name = argv[0];
        const char * exe_filename = exe_name ? sFilePath::nextToSlash(exe_name) : 0;
        if( exe_filename && exe_filename[0] ) {
            svc.addString(exe_filename);
            // remove .osLinux suffix
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
        // ... otherwise, read service name from the first command line argument
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

    // first argument to python executable: "-c" to read python_script from next arg
    python.addArg("-c");
    // second argument: tiny script to launch pyhive.proc.run(); sys.argv[1:] to skip initial "-c" in sys.argv
    // before passing to pyhive.proc.run()
    sStr python_script("import sys; import pyhive; pyhive.proc = pyhive.Proc('%s'); import %s; pyhive.proc.run(%s, sys.argv[1:])", svc.ptr(), svc.ptr(), svc.ptr());
    python.addArg(python_script);

    // pass our remaining command-line arguments to python (which will pass them to python_script)
    for( idx i = first_py_iarg; i < argc; i++ ) {
        python.addArg(argv[i]);
    }

    idx python_retcode = 0;
    sRC rc = python.execute(&python_retcode);
    if( rc ) {
        fprintf(stderr, "%s\n", rc.print());
        return python_retcode ? python_retcode : 1;
    }

    return python_retcode;
}
