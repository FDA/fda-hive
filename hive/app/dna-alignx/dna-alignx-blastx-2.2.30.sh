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

# old source "${QPRIDE_BIN}/dna-alignx-blast-common.sh.os${os}"

$(setenv("mico2", "blast", false))

# include general alignX shell functions
. "${QPRIDE_BIN}/dna-alignx.sh.os${os}"

# include common blast tools' shell functions
. "${QPRIDE_BIN}/dna-alignx-blast-common.sh.os${os}"

dna_alignx_index() {
    dna_alignx_exec makeblastdb -in "$(.referenceFile)" -dbtype prot -title "$(.indexPath)" -out "$(.indexPath)"
}

dna_alignx_align() {
    blastxParams="-lcase_masking"
    if [ ! -z "$(.score_threshold)" ]; then
        blastxParams=" -threshold $(.score_threshold) ${blastxParams}"
    fi
    if [ ! -z "$(.max_intron_length)" ]; then
        blastxParams=" -max_intron_length $(.max_intron_length) ${blastxParams}"
    fi
    if [ ! -z "$(.wordSize)" ]; then
        blastxParams=" -word_size $(.wordSize) ${blastxParams}"
    fi
    if [ ! -z "$(.evalueFilter)" ]; then
        blastxParams=" -evalue $(.evalueFilter) ${blastxParams}"
    fi
    if [[ ! -z "$(.blastn_task)" ]]; then
        blastxParams=" ${blastxParams} -task $(.blastn_task)"
    fi
    if [[ "$(.output_fmt)" == "tsv" ]]; then
        dna_alignx_exec blastx -num_threads 1 -db "$(.indexPath)" -query "$(.queryFiles)" -outfmt 6 -out "$(.outPath).tsv" ${blastxParams}
    else
        dna_alignx_exec blastx -num_threads 1 -db "$(.indexPath)" -query "$(.queryFiles)" -out "$(.outPath).blast_out" ${blastxParams}
    fi
}

dna_alignx_finalize() {
    dna_alignx_protein_blast_finalize "$(.finalFiles)" "$(.output_fmt)" "$(.resultPath)"
}

dna_alignx_main "$@"

