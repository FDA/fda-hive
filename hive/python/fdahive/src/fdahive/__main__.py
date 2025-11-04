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

import argparse
import csv
import sys
from .session import HiveSession
from .__version__ import __version__

parser = argparse.ArgumentParser(prog='fdahive', description='Access HIVE from the command-line. Consists of three subcommands: down, up, and view. Use --help on each subcommand for more information')
parser.add_argument('-u', '--url', help='url of the HIVE instance. Will be prompted for if not provided')
parser.add_argument('-n', '--username', help='username for logging in')
parser.add_argument('--cred-file', default='~/.hive-creds', help='file storing credentials (default ~/.hive-creds)')
parser.add_argument('-s', '--save-creds', action='store_true', default=False, help='save login credentials if login is successful')
parser.add_argument('-i', '--insecure', action='store_true', default=False, help='disable SSL verification. NOT RECOMMENDED')
parser.add_argument('--cacert', help='specify alternative certificate location')
parser.add_argument('--project', help='specify project to operate in by project ID')
parser.add_argument('--version', action='version', version='%(prog)s {version}'.format(version=__version__))

subparsers = parser.add_subparsers(title='commands', dest='subparser_name')

dwn_parser = subparsers.add_parser('down', help='download objects')
dwn_parser.add_argument('objects', nargs='+', help='object IDs to download')
dwn_parser.add_argument('-d', '--dst', help='directory to download files to (default current working directory)')
dwn_parser.add_argument('-o', '--overwrite', default=False, action='store_true', help='overwrite existing files (default False)')
dwn_parser.add_argument('-m', '--make-obj-dir', default=False, action='store_true', help='place each object in a seperate directory named after the object ID (default False)')
dwn_parser.add_argument('--csv-profile', default=False, action='store_true', help='download Heptagon profiles in CSV format, as opposed to standard VCF format (default False)')
dwn_parser.add_argument('--af-threshold', help='The allele frequency threshold used when downloading a profile in heptagon')

up_parser = subparsers.add_parser('up', help='upload files')
up_parser.add_argument('files', nargs='+', help='files to upload')
up_parser.add_argument('-t', '--threads', type=int, help='maximum number of threads to use')
up_parser.add_argument('-f', '--folder', help='object ID of destination folder')
up_parser.add_argument('-r', '--reference', help='object ID of HIVE reference to associate with uploaded alignments')
up_parser.add_argument('--qc', action='store_true', help='perform quality control on upload (default False)')
up_parser.add_argument('--screen', action='store_true', help='perform screening on upload (default False)')

view_parser = subparsers.add_parser('view', help='view objects in system')
view_parser.add_argument('type', help='type of objects to view. Either one of the follwing groups: all|folders|genomes|reads|files|computations or an internal HIVE type')
view_parser.add_argument('--csv', action='store_true', default=False, help='output table as csv instead of pretty-printing')

args = parser.parse_args()

url = args.url
if url is None:
    url = input('Enter url of HIVE instance to connect to: ')

ssl_verify = True
if args.insecure:
    ssl_verify = False
if args.cacert:
    ssl_verify = args.cacert

with HiveSession(url, ssl_verify) as hs:
    if args.project:
        hs.project = args.project
    hs.login(username=args.username, cred_file=args.cred_file, save_creds=args.save_creds)
    if args.subparser_name == 'down':
        hs.download(args.objects, dst_dir=args.dst, overwrite=args.overwrite, make_obj_dir=args.make_obj_dir, heptagon_csv_profile=args.csv_profile, heptagon_threshold=args.af_threshold)
    elif args.subparser_name == 'up':
        hs.upload(args.files, max_threads=args.threads, folder=args.folder, reference=args.reference, qc=args.qc, screen=args.screen)
    elif args.subparser_name == 'view':
        table = None
        if args.type == 'all':
            table = hs.get_all_objs(print_table=not args.csv)
        elif args.type == 'folders':
            table = hs.get_folders(print_table=not args.csv)
        elif args.type == 'genomes':
            table = hs.get_genomes(print_table=not args.csv)
        elif args.type == 'reads':
            table = hs.get_reads(print_table=not args.csv)
        elif args.type == 'files':
            table = hs.get_files(print_table=not args.csv)
        elif args.type == 'computations':
            table = hs.get_computations(print_table=not args.csv)
        else:
            table = hs.get_all_objs(args.type, print_table=not args.csv)

        if table is not None:
            csv_wrt = csv.writer(sys.stdout)
            csv_wrt.writerow(['obj', 'name', 'created', 'type'])
            csv_wrt.writerows(table)
