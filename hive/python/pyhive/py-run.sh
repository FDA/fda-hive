#!/bin/bash
#/*
# *  ::718604!
# * 
# * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
# * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
# * Affiliation: Food and Drug Administration (1), George Washington University (2)
# * 
# * All rights Reserved.
# * 
# * The MIT License (MIT)
# * 
# * Permission is hereby granted, free of charge, to any person obtaining
# * a copy of this software and associated documentation files (the "Software"),
# * to deal in the Software without restriction, including without limitation
# * the rights to use, copy, modify, merge, publish, distribute, sublicense,
# * and/or sell copies of the Software, and to permit persons to whom the
# * Software is furnished to do so, subject to the following conditions:
# * 
# * The above copyright notice and this permission notice shall be included
# * in all copies or substantial portions of the Software.
# * 
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# */

: ${QPRIDE_BIN:=/home/qpride/bin}
: ${os:=Linux}

HIVE_PY_PKG=python-2.7.os${os}

# detect CentOS 6 and RHEL 6 - need different binaries for their ancient libc version
if [[ $os = "Linux" ]] && ( grep -E 'release 6([^0-9]|$)' /etc/redhat-release 2> /dev/null || grep -E 'release 6([^0-9]|$)' /etc/centos-release 2> /dev/null ); then
    HIVE_PY_PKG=${HIVE_PY_PKG}.el6
fi

HIVE_PY_HOME=${QPRIDE_BIN}/${HIVE_PY_PKG}
if [[ ! -d "${HIVE_PY_HOME}" ]]; then
    echo "python-2.7 for ${os} not found in ${HIVE_PY_HOME}"
    exit 1
fi

export PYTHONHOME=${HIVE_PY_HOME}/usr
export PYTHONPATH=${QPRIDE_BIN}/python2.7:${HIVE_PY_HOME}/usr/lib/python2.7
export PYTHONIOENCODING=utf-8

# for libfontconfig, pango, other freedestkop stuff
export XDG_CONFIG_HOME=${HIVE_PY_HOME}/etc
export FONTCONFIG_PATH=${HIVE_PY_HOME}/etc/fonts

# for text UI
export TERMINFO=${HIVE_PY_HOME}/usr/lib/terminfo

export PATH=${HIVE_PY_HOME}/usr/bin:${PATH}
export LD_LIBRARY_PATH=${HIVE_PY_HOME}/usr/lib:${HIVE_PY_HOME}/lib:${LD_LIBRARY_PATH}

exec ${HIVE_PY_HOME}/usr/bin/python "$@"
