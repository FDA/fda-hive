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

cd "$workDir" || exit 10;
echo "cd $workDir"

cmd="cuffmerge -p 4 -o cuffmerge -g $refGTF -s $refFasta $gtfFileList"
echo $cmd
cuffmerge -p 4 -o cuffmerge -g "$refGTF" -s "$refFasta" "$gtfFileList"

cmd="cuffdiff --quiet -p 4 -v -o cuffdiff --min-reps-for-js-test 2  cuffmerge/merged.gtf $bamFilesList"
echo $cmd
cuffdiff --quiet -p 4 -v -o cuffdiff cuffmerge/merged.gtf $bamFilesList
