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
    # This needs to be changed to match the directory name that varscan2 is adopted into (in home/qpride/bin)
    echo "varscan2"
}

#dna_profx_prepare () {
#
#}

dna_profx_run () {

    # Convert dumped SAM file into BAM file for generation of pileup file
    echo "samtools view -bS ${workDir}/alignment-${subID}.sam > ${workDir}/alignment-${subID}.bam"
    samtools view -bS ${workDir}/alignment-${subID}.sam > ${workDir}/alignment-${subID}.bam
    if [ $? != 0 ]; then exit $?; fi

    # Create pileup file
    echo "samtools mpileup -f ${subjectFastaFile} ${workDir}/alignment-${subID}.bam > ${workDir}/alignment-${subID}.mpileup" 
    samtools mpileup -f ${subjectFastaFile} ${workDir}/alignment-${subID}.bam > ${workDir}/alignment-${subID}.mpileup
    if [ $? != 0 ]; then exit $?; fi
    
    # Launch pileup2snp with varscan
    ###### Need to fix with proper path to VarScan?  Since varscan can't be in global path?
    ###### Also need to be able to properly parse output file
    echo "java -jar ${QPRIDE_BIN}/varscan2.osLinux/VarScan.v2.4.0.jar pileup2snp ${workDir}/alignment-${subID}.mpileup --p-value 5e-02 > snpCalls.tsv"
    java -jar ${QPRIDE_BIN}/varscan2.osLinux/VarScan.v2.4.0.jar pileup2snp ${workDir}/alignment-${subID}.mpileup --p-value 5e-02 --output-vcf 1 > ${workDir}/SNP-${subID}.vcf
    if [ $? != 0 ]; then exit $?; fi
}

#dna_profx_finalize () {
#
#}
