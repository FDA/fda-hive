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
kmerSize=$2
ends=$3
#ends=1 for SingleEnd, end=2 for PairedEnd input

if [ "$QPRIDE_BIN" = "" ]; then
    echo "QPRIDE_BIN is not set"
    QPRIDE_BIN='./'
fi

if [ "$kmerSize" = "" ]; then
    echo "no kmerSize is given, using kmerSize=32"
    kmerSize=32
fi

cd "$workDir" || exit 10;
echo "cd $workDir"

if [ "$ends" = "2" ]; then
    echo "abyss-pe name=this $kmerSize in='input1.fastq input2.fastq'"
    abyss-pe name=this $kmerSize in='input1.fastq input2.fastq'
fi

if [ "$ends" = "1" ]; then
        echo "ABYSS $kmerSize input.fa -o this-contigs.fa"
        ABYSS $kmerSize input.fa -o this-contigs.fa
fi
