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
#
# Script to check binaries in /home/qpride/bin and QPResource table for dynamic linking failure
#

shopt -s expand_aliases
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "${SCRIPT_DIR}/hive_bash_rc"

check_linking() {
    local filepath=$1
    local printable=$2
    [[ -z $printable ]] && printable=${filepath}

    if ldd "$filepath" 2>&1 | grep -Eq "not a dynamic executable"; then
        return 0
    fi

    if ldd -r "$filepath" 2>&1 | grep -Eq "not found|undefined"; then
        echo "$printable link failure:"
        ldd -r "$filepath"
        return 1
    fi
}


QPRIDE_BIN=$(echo "SELECT val FROM QPCfg WHERE par = 'qm.resourceRoot';" | qdb -N)
: ${os:=Linux}

REL_PREFIX=prefix.os${os}

# detect CentOS 6 and RHEL 6 - need different binaries for their ancient libc version
if [[ $os = "Linux" ]] && ( grep -Eq 'release 6([^0-9]|$)' /etc/redhat-release 2> /dev/null || grep -Eq 'release 6([^0-9]|$)' /etc/centos-release 2> /dev/null ); then
    REL_PREFIX=${REL_PREFIX}.el6
elif [[ $os = "Linux" ]] && ( grep -Eq 'release 7([^0-9]|$)' /etc/redhat-release 2> /dev/null || grep -Eq 'release 7([^0-9]|$)' /etc/centos-release 2> /dev/null ); then
    REL_PREFIX=${REL_PREFIX}.el7
fi

export LD_LIBRARY_PATH=$QPRIDE_BIN:$QPRIDE_BIN/${REL_PREFIX}/usr/lib:$QPRIDE_BIN/lib64:$LD_LIBRARY_PATH

ret=0

echo "Checking binary linking in ${QPRIDE_BIN} ..."
for fil in $(ls "${QPRIDE_BIN}"); do
    if [[ -f ${QPRIDE_BIN}/${fil} ]]; then
        if ! check_linking "${QPRIDE_BIN}/${fil}"; then
            ret=1
        fi
    fi
done

echo "Checking binary linking in QPResource table ..."
tmp_dir=$(mktemp -d)
names_tab=${tmp_dir}/_names.tab
max_blob_size=16777216 # 16 MB
echo "SELECT svcName, dataName FROM QPResource WHERE dataName NOT LIKE '%>%' AND dataName NOT LIKE '%.sh%' AND OCTET_LENGTH(dataBlob) < ${max_blob_size};" | qdb -B -N > "${names_tab}"
num_blobs=$(wc -l "${names_tab}" | cut -f 1 -d ' ')
for line in $(seq 1 ${num_blobs}); do
    svcName=$(head -n $line "${names_tab}" | tail -n 1 | cut -f 1)
    dataName=$(head -n $line "${names_tab}" | tail -n 1 | cut -f 2)
    filepath=${tmp_dir}/$(basename "${dataName}")

    #echo "$svcName > $dataName > $filepath"

    echo "SELECT dataBlob FROM QPResource WHERE svcName = '${svcName}' AND dataName = '${dataName}' LIMIT 1;" | qdb -N > "${filepath}"
    if ! check_linking "${filepath}" "${svcName} ${dataName}"; then
        ret=1
    fi
done
[[ -d ${tmp_dir} ]] && rm -r "${tmp_dir}"

exit ${ret}
