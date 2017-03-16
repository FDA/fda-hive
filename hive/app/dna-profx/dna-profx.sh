#!/bin/sh
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

exec 2>&1 # redirect stderr to stdout

dna_profx_error() {
    echo "ERROR: $*"
    exit 99;
}

profiler="dna-profx-$1.sh.os${os}"

if [[ ! -d "$QPRIDE_BIN" ]]; then
    dna_profx_error "QPRIDE_BIN is not valid path"
fi

if [[  -f "$QPRIDE_BIN/$profiler" ]]; then
    source "$QPRIDE_BIN/$profiler"
elif [[ -f "$profiler" ]]; then
    source "./$profiler"
else
    dna_profx_error "Script $profiler not found in '`pwd` : $QPRIDE_BIN'"
fi

tools=`dna_profx_tools`
echo "Setting up tools: $tools..."

for tool in $tools; do
    # inject os designation before 1st '/'
    x=${tool/\//os${os}/}
    if [[ "$x" == "$tool" ]]; then
        # no / ? just append than
        x="$tool.os${os}"
    fi
    if [[ -d "$QPRIDE_BIN/$x" ]]; then
        PATH=$QPRIDE_BIN/$x:$PATH
    else
        dna_profx_error "Directory for tool '$tool' not found in '$QPRIDE_BIN'"
    fi
done

__cmd="$2"
varname="orphans"

shift 2

while [[ $# -gt 0 ]]; do
    str="$1"
    begins="${str:0:2}"
    if [[ "$begins" == "--" ]]; then
        varname=${str:2}
        val=""
    else 
        eval val=\$$varname
        if [[ "$val" ]]; then
            val="$val "
        fi
        val="$val$str"
        export $varname="$val"
    fi
    shift 1
done

# Strip '"' from start and end of cmdLine
cmdLine=${cmdLine#\"}
cmdLine=${cmdLine##\"}

case $__cmd in
prepare)
    dna_profx_prepare "$@"
    ;;
run)
    dna_profx_run "$@"
    ;;
finalize)
    dna_profx_finalize "$@"
    ;;
*)
    dna_profx_error "unknown command $__cmd"
    ;;
esac
