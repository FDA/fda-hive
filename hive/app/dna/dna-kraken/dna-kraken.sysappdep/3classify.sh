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

if [[ $# -lt 2 ]] ; then
    echo 'Please provide at least input and output file names'
    exit 1
fi

IMAGE_NAME=hive-tools-image
DB_NAME=kdb
CPU_COUNT=$([ $(uname) = 'Darwin' ] && sysctl -n hw.logicalcpu_max || lscpu -p | egrep -v '^#' | wc -l)
INPUT_FILE_NAME=$1
OUTPUT_FILE_NAME=$2
INPUT_FOLDER=${3:-${PWD}/data/input}
OUTPUT_FOLDER=${4:-${PWD}/data/output}
DB_FOLDER=${5:-${PWD}/$DB_NAME}
THREADS=${6:-$CPU_COUNT}
INPUT_FILE=/kraken/data/input/$INPUT_FILE_NAME
OUTPUT_FILE=/kraken/data/output/$OUTPUT_FILE_NAME

if [ ! -d "$INPUT_FOLDER" ]; then
    echo "Input folder ${INPUT_FOLDER} does not exist"
    exit 1
fi

if [ ! -d "$OUTPUT_FOLDER" ]; then
    echo "Output folder ${OUTPUT_FOLDER} does not exist"
    exit 1
fi

if [ ! -d "$DB_FOLDER" ]; then
    echo "DB folder ${DB_FOLDER} does not exist"
    exit 1
fi

INPUT=$INPUT_FOLDER/$INPUT_FILE_NAME
if [ ! -e "$INPUT" ]; then
    echo "Input file ${INPUT} does not exist"
    exit 1
fi

echo "Using $THREADS CPU cores for processing"
docker run --rm -it --network host -v $DB_FOLDER:/kraken/$DB_NAME -v $INPUT_FOLDER:/kraken/data/input -v $OUTPUT_FOLDER:/kraken/data/output -w /kraken $IMAGE_NAME -c "kraken2 --db /kraken/$DB_NAME --threads $THREADS --report $OUTPUT_FILE $INPUT_FILE > /dev/null"
