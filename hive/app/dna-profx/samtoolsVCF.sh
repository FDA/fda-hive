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

workDir=$1
subjectFastaFile=$2
#additionalCommandLineParameters=$3


if [ "$QPRIDE_BIN" = "" ]; then
    echo "QPRIDE_BIN is not set"
    QPRIDE_BIN='.'
fi

cd "$workDir" || exit 10;
echo "cd $workDir"

echo "samtools view -bS alignment.sam > alignment.bam"
samtools view -bS alignment.sam > alignment.bam

echo "samtools sort alignment.bam alignment_sorted"
samtools sort alignment.bam alignment_sorted 

echo "samtools index alignment_sorted.bam"
samtools index alignment_sorted.bam

# Default (BAQ is on) does not output SNPs for .sam files without quality bytes, such as in .bwa output  
# -E in pileup command make samtools use less restrictive Base alignment quality (BAQ) computation which is turned on by default.
# http://massgenomics.org/2012/03/5-things-to-know-about-samtools-mpileup.html
#
echo "samtools mpileup -Eugf $subjectFastaFile alignment_sorted.bam | bcftools view -vcg - > SNP.vcf"
samtools mpileup -Eugf $subjectFastaFile alignment_sorted.bam | bcftools view -vcg - > SNP.vcf
