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

# redirect my stderr to my stdout
exec 2>&1

echo "working directory: '`pwd`'"

file=$1
if [ "x$file" = "x" ]; then
    echo "missing 1st argument: SRA file"
    exit 1
fi
echo "Input file $file"

which qapp
if [ ! "$?" = "0" ]; then
    echo "qapp not found"
    exit 3
fi
pushd . 1>/dev/null 2>&1
cd $QPRIDE_BIN
downpath=`qapp -configGet user.download`;
if [ "x$downpath" = "x" ]; then
    echo "Cannot find 'user.download' in config"
    exit 4
fi
sraftpurl=`qapp -configGet 'dmArchiver.SRA-FTP'`;
if [ "x$sraftpurl" = "x" ]; then
    sraftpurl='http://ftp-trace.ncbi.nlm.nih.gov/sra'
    echo "Using default SRA FTP $sraftpurl"
fi
echo "user.download='$downpath'"
echo "dmArchiver.SRA-FTP='$sraftpurl'"
popd 1>/dev/null 2>&1

# need to copy sra binaries since they look for kfg file local to binaries
sra_prefetch=`which sra_prefetch`
if [ "x$sra_prefetch" = "x" ]; then
    echo "sra_prefetch not found"
    exit 2
fi
cp -pv $sra_prefetch ~dm_sra_prefetch

fastq_dump=`which fastq-dump`
if [ "x$fastq_dump" = "x" ]; then
    echo "fastq-dump not found"
    exit 3
fi
cp -pv $fastq_dump ~dm_fastq_dump

# create global ncbi storage
mkdir -p $downpath/ncbi_sra
if [ ! -d $downpath/ncbi_sra ]; then
    echo "Cannot create directory $downpath/ncbi_sra"
    exit 5
fi

# create local config directory, prefetch needs it!
mkdir ncbi
if [ ! -d ncbi ]; then
    echo "Cannot create directory `pwd`/ncbi"
    exit 6
fi
ncbi_kfg=ncbi/default.kfg
cat >$ncbi_kfg <<KFG_EOF
## DEFAULT CONFIGURATION FOR SRA-TOOLKIT
#.
#  These settings are intended to allow the tools to work under conditions
#  when the user has not yet performed any configuration.
/config/default = "true"

# The user's default public repository
/repository/user/main/public/apps/refseq/volumes/refseq = "refseq"
/repository/user/main/public/apps/sra/volumes/sraFlat = "sra"
/repository/user/main/public/apps/wgs/volumes/wgsFlat = "wgs"
/repository/user/main/public/cache-enabled = "true"
/repository/user/main/public/root = "${downpath}ncbi_sra"

# Remote access to NCBI's public repository
/repository/remote/main/NCBI/apps/refseq/volumes/refseq = "refseq"
/repository/remote/main/NCBI/apps/sra/volumes/fuse1000 = "sra-instant/reads/ByRun/sra"
/repository/remote/main/NCBI/apps/wgs/volumes/fuseWGS = "wgs"
/repository/remote/main/NCBI/root = "${sraftpurl}"

# Remote access to NCBI's protected repository
/repository/remote/protected/CGI/resolver-cgi = "http://www.ncbi.nlm.nih.gov/Traces/names/names.cgi"
KFG_EOF

if [ ! -s $ncbi_kfg ]; then
    echo "SRA config was not created `pwd`/$ncbi_kfg"
    exit 7
fi
cat $ncbi_kfg

# this doesn't report error status properly so there is nothing to check
echo "Fetching dependencies if any..."
./~dm_sra_prefetch -v -L info $file

accession=`basename $file .sra`

echo "Dumping fastQ..."
./~dm_fastq_dump --accession "$accession" --outdir ./ --split-files --origfmt --skip-technical "$file"
err=$?

rm -f $ncbi_kfg
rm -f ~dm_sra_prefetch
rm -f ~dm_fastq_dump

if [ ! "$err" = "0" ]; then
    echo "fastq-dump failed: $err"
    exit 8
fi
