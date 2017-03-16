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

dna_profx_tools() {
    echo "samtools-0.1.18"
}

#dna_profx_prepare () {
#
#}

dna_profx_run () {

    echo "samtools view -bS ${workDir}/alignment-${subID}.sam > ${workDir}/alignment-${subID}.bam"
    samtools view -bS ${workDir}/alignment-${subID}.sam > ${workDir}/alignment-${subID}.bam
    if [ $? != 0 ]; then exit $?; fi

    echo "samtools sort ${workDir}/alignment-${subID}.bam ${workDir}/alignment_sorted-${subID}"
    samtools sort ${workDir}/alignment-${subID}.bam ${workDir}/alignment_sorted-${subID} 
    if [ $? != 0 ]; then exit $?; fi

    echo "samtools index ${workDir}/alignment_sorted-${subID}.bam"
    samtools index ${workDir}/alignment_sorted-${subID}.bam
    if [ $? != 0 ]; then exit $?; fi
    
    echo "samtools mpileup -Eugf $subjectFastaFile ${workDir}/alignment_sorted-${subID}.bam | bcftools view -vcg - > ${workDir}/SNP-${subID}.vcf"
    samtools mpileup -Eugf $subjectFastaFile ${workDir}/alignment_sorted-${subID}.bam | bcftools view -vcg - > ${workDir}/SNP-${subID}.vcf
    if [ $? != 0 ]; then exit $?; fi
}

#dna_profx_finalize () {
#
#}