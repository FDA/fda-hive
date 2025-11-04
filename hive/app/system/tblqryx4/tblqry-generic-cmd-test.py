#!/usr/bin/env python
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

#
# Demo Python generic command for tblqry: append a "Sum" column to a table,
# and in that column, print the sum of values of columns from a specified list
#
#
# The following TQS object:
# { "op": "generic-cmd-test", "arg" : { "cols": [0, 1, 2] } }
#
# Will result in the following command-line invocation:
# python tblqry-generic-command-test.py --i /some/dir/blah-in.csv -o /some/dir/blah-out.csv --req=12345 --cols=0,1,2
#

# We use CSV format for input and output
import csv

# For parsing command-line arguments
import argparse
import re

def intlist(s):
    return [int(i) for i in re.compile(r'[,;\s]').split(s)]

def parse_args():
    parser = argparse.ArgumentParser(description='Demo Python generic command for tblqry')
    parser.add_argument('-i', metavar='IN.CSV', required=True, type=argparse.FileType('r'), help='input table filename')
    parser.add_argument('-o', metavar='OUT.CSV', required=True, type=argparse.FileType('w'), help='output table filename')
    parser.add_argument('--req', metavar='REQ_ID', type=int, help='HIVE request ID')
    parser.add_argument('--cols', metavar='LIST', required=True, type=intlist, help='indices of columns to sum')
    return parser.parse_args()

def compute(args):
    reader = csv.reader(args.i)
    writer = csv.writer(args.o)
    header = reader.next()
    header.append('Sum')
    writer.writerow(header)

    for row in reader:
        sum = 0
        for col in args.cols:
            try:
                sum = sum + float(row[col])
            except ValueError:
                # ignore non-numeric columns
                pass

        row.append(sum)
        writer.writerow(row)

if __name__ == '__main__':
    args = parse_args()
    compute(args)
