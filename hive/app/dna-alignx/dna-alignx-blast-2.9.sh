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

$(setenv("mico3", "blast", false))

# include general alignX shell functions
. "${QPRIDE_BIN}/dna-alignx.sh.os${os}"

# include common blast tools' shell functions
. "${QPRIDE_BIN}/dna-alignx-blast-common.sh.os${os}"


dna_alignx_index() {
    dna_alignx_exec makeblastdb -in "$(.referenceFile)" -dbtype nucl -title "$(.indexPath)" -out "$(.indexPath)"
}

dna_alignx_align() {
    flags=""
    maxMissQueryPercent=$(.maxMissQueryPercent)
    if [[ ! -z "$(.maxMissQueryPercent)" ]]; then
        percent=`expr 100 - $(.maxMissQueryPercent)`
        if [[ ! -z "${percent}" ]]; then
            flags=" ${flags} -perc_identity ${percent}"
        else
            echo "Cannot read parameter 'Percent Allowed'. Running with default value."
        fi
    fi
    if [[ ! -z "$(.evalueFilter)" ]]; then
        flags=" ${flags} -evalue $(.evalueFilter)"
    fi
    if [[ ! -z "$(.seedSize)" ]]; then
        flags=" ${flags} -word_size $(.seedSize)"
    fi
    if [[ ! -z "$(.num_alignments)" ]]; then
        flags=" ${flags} -num_alignments $(.num_alignments)"
    fi
    if [[ ! -z "$(.num_descriptions)" ]]; then
        flags=" ${flags} -num_descriptions $(.num_descriptions)"
    fi
    if [[ ! -z "$(.best_hit_score_edge)" ]]; then
        flags=" ${flags} -best_hit_score_edge $(.best_hit_score_edge)"
    fi
    if [[ ! -z "$(.best_hit_overhang)" ]]; then
        flags=" ${flags} -best_hit_overhang $(.best_hit_overhang)"
    fi
    if [[ ! -z "$(.blastn_task)" ]]; then
        flags=" ${flags} -task $(.blastn_task)"
    fi
    echo "dna_alignx_exec blastn ${flags} -num_threads 1 -db \"$(.indexPath)\" -query \"$(.queryFiles)\" -out \"$(.outPath).blast_out\""
    dna_alignx_exec blastn ${flags} -num_threads 1 -db "$(.indexPath)" -query "$(.queryFiles)" -out "$(.outPath).blast_out"
}

dna_alignx_main "$@"
