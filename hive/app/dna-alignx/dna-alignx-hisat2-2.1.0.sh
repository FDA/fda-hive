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
$(setenv("mico3", "base", false))

# include general alignX shell functions
. "${QPRIDE_BIN}/dna-alignx.sh.os${os}"

dna_alignx_index() {
    build_options=""
    if [[ ! -z "$(.score_threshold)" ]] ; then
        if [[ "$(.hisat_gtf_to_ss)" = 1 ]]; then
            gtf_ss_parsed_path="$(.indexPath)-ss-gtf.txt"
            echo "$gtf_ss_parsed_path"
            dna_alignx_exec hisat2_extract_splice_sites.py "$(.annotationFile)" >  ${gtf_ss_parsed_path}
            if [ $? != 0 ]; then exit $?; fi
            build_options=" --ss ${gtf_ss_parsed_path} "
        fi
        if [[ "$(.hisat_gtf_to_exon)" = 1 ]]; then
            gtf_exons_parsed_path="$(.indexPath)-exon-gtf.txt"
            dna_alignx_exec hisat2_extract_exons.py "$(.annotationFile)" >   ${gtf_exons_parsed_path}
            if [ $? != 0 ]; then exit $?; fi
            build_options="${build_options} --exon ${gtf_exons_parsed_path} "
        fi
    fi

    dna_alignx_exec hisat2-build "${build_options}" "$(.referenceFile)" "$(.indexPath)"
}

dna_alignx_align() {
    gtf_exons_parsed_path="$(.indexPath)-exon-gtf.txt"
    ss_infile=""
    if [[ ! -z "$(.annotationFile)" ]] ; then
        ss_infile=" --known-splicesite-infile ${gtf_exons_parsed_path} "
    fi
    if [ -z "$(.paired_queryFiles)" ] ; then
        dna_alignx_exec hisat2 "${ss_infile}" -x "$(.indexPath)" -q -U "$(.queryFiles)" -S "$(.outPath).sam"
    else
        dna_alignx_exec hisat2 "${ss_infile}" -x "$(.indexPath)" -q -1 "$(.queryFiles)" -2 "$(.paired_queryFiles)" -S "$(.outPath).sam"
    fi
 }

dna_alignx_main "$@"
