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
refGTF=$2
refFasta=$3
gtfFileList=$4
bamFilesList=$5
#additionalCommandLineParameters=$6


if [ "$QPRIDE_BIN" = "" ]; then
    echo "QPRIDE_BIN is not set"
    QPRIDE_BIN='.'
fi

for tool in `echo -n 'samtools-0.1.18 tophat-2.0.6 cufflinks-2.0.2'`; do
    PATH=$QPRIDE_BIN/$tool:$PATH
done
echo "PATH '$PATH'"
echo "PYTHONPATH '$PYTHONPATH'"

cd "$workDir" || exit 10;
echo "cd $workDir"

#cuffmerge -o <out prefix> -g <reference gtf file> -p 4 -s $<reference fasta file> <list of transcript.gtf files from cufflinks to be merged>
#cuffmerge -p 8 -o cuffmerge -g $BIOTOOLS/Homo_sapiens/Ensembl/GRCh37/Annotation/Genes/genes.gtf -s $BIOTOOLS/Homo_sapiens/Ensembl/GRCh37/Sequence/WholeGenomeFasta/genome.fa gftFilesList.txt
cmd="cuffmerge -p 4 -o cuffmerge -g $refGTF -s $refFasta $gtfFileList"
echo $cmd
cuffmerge -p 4 -o cuffmerge -g "$refGTF" -s "$refFasta" "$gtfFileList"


#cuffdiff  -p 24 -v -o <output-directory --min-reps-for-js-test 2 --labels <labels for files> <path to merged.gtf file from cuffmerge> <bam files- replicates separated by a comma>
#cuffdiff -p 8 -v -o cuffdiff --min-reps-for-js-test 2  cuffmerge/merged.gtf ./231_pcDNA1-tophat_out-new/accepted_hits.bam,./231-pcDNA2-tophat_out/231pcDNA2-accepted_hits.bam ./231-HER21-tophat_out/231HER21-accepted_hits.bam,./231-HER22-tophat_out/231HER22-accepted_hits.bam
cmd="cuffdiff --quiet -p 4 -v -o cuffdiff --min-reps-for-js-test 2  cuffmerge/merged.gtf $bamFilesList"
echo $cmd
cuffdiff --quiet -p 4 -v -o cuffdiff cuffmerge/merged.gtf $bamFilesList