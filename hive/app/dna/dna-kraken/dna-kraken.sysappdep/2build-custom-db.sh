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
set -u
set -e

if [[ $# -lt 1 ]] ; then
    echo 'Please provide at least input folder'
    exit 1
fi

IMAGE_NAME=hive-tools-image
DB_NAME=kdb
SAMPLES_DIR=samples
CPU_COUNT=$([ $(uname) = 'Darwin' ] && sysctl -n hw.logicalcpu_max || lscpu -p | egrep -v '^#' | wc -l)
INPUT_FOLDER=$1
DB_FOLDER=${2:-${PWD}/$DB_NAME}
THREADS=${3:-$CPU_COUNT}

if [ ! -d "$INPUT_FOLDER" ]; then
    echo "Input folder ${INPUT_FOLDER} does not exist"
    exit 1
fi

if [ ! -d "$DB_FOLDER" ]; then
    echo "DB folder ${DB_FOLDER} does not exist"
    exit 1
fi

echo "Using $THREADS CPU cores for processing"
docker run --rm -it --network host -v $DB_FOLDER:/kraken/$DB_NAME -v $INPUT_FOLDER:/kraken/data/$SAMPLES_DIR -w /kraken $IMAGE_NAME -c "find /kraken/data/$SAMPLES_DIR/ -name '*.fasta' -print0 | xargs -0 -I{} -n1 kraken2-build --add-to-library {} --db $DB_NAME ; kraken2-build --build --threads $THREADS --db /kraken/$DB_NAME"
