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

source "${QPRIDE_BIN}/dna-alignx-blast-common.sh.os${os}"

dna_alingx_build() {

    cmd="makeblastdb -in $referenceFile -dbtype nucl -title $indexPath -out $indexPath"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
}

dna_alingx_align() {

    tblastxParams=""
    if [[ ! -z "$evalueFilter" ]]; then
        tblastxParams=" -evalue $evalueFilter $tblastxParams"
    fi
    if [[ ! -z "$wordSize" ]]; then
        tblastxParams=" -word_size $wordSize $tblastxParams"
    fi
    if [ ! -z "$num_alignments" ]; then
        blastxParams=" -num_alignments $num_alignments $tblastxParams"
    fi
    if [ ! -z "$maxHitsPerRead" ]; then
        blastxParams=" -max_target_seqs $maxHitsPerRead $tblastxParams"
    fi
    if [[ "${output_fmt}" == "tsv" ]]; then
        cmd="tblastx $cmdLine -outfmt 6 $tblastxParams -db $indexPath -query $queryFiles -out $outPath.tsv"
    else
        cmd="tblastx ${cmdLine} ${tblastxParams} -db ${indexPath} -query ${queryFiles} -out ${outPath}.blast_out"
    fi
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
}
