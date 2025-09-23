/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

function vjExplorerUsrView (viewer) {
    this.subTablesAttrs = [
                           {
                               tabname : "all",
                               tabico : "folder-apps",
                               url : { type : "-" , prop:"_brief,created,submitter,status" },
                               types: ".*",
                               hideListCols :[{ name : 'status',hidden : true}]

                           }, {
                               tabname : "folders",
                               tabico : "folder-hive",
                               types: "^folder$",
                               url : { type : "^folder$" }
                           }, {
                               tabname : "genomes",
                               tabico : "dna",
                               types: "^genome$",
                               url : { type : "^genome$" }
                           }, {
                               tabname : "reads",
                               tabico : "dnaold",
                               types: "^nuc-read$",
                               url : { type : "^nuc-read$"
                               }
                           }, {
                               tabname : "research projects",
                               tabico : "bio-project",
                               types: "^HIVE_Development_Project_List$",
                               url : { type : "^HIVE_Development_Project_List$%2B"
                               }
                           }, {
                               tabname : "annotations",
                               tabico : "rec",
                               types: "^u-annot$,^u-ionAnnot$",
                               url : { type : "^u-annot$,^u-ionAnnot$%2B" }
                           }, {
                               tabname : "files",
                               tabico : "ico-file",
                               types: "^u-file$,^table$,^u-idList$",
                               url : { type : "^u-file$,^table$%2B,^u-idList$" }
                           }, {
                               tabname : "spectra",
                               tabico : "ico-file",
                               types: "^spectra$,^spectra-MS$,^spectra-lib$",
                               url : { type : "^spectra" }
                           }, {
                               tabname : "images",
                               tabico : "ico-image",
                               types: "^image$",
                               url : { type : "^image$%2B" }
                           }, {
                               tabname : "computations",
                               tabico : "svc-process",
                               types: "svc-.*,!svc-archiver,!svc-download",
                               url : { type : "^svc-computations-base$%2B" }
                           }, {
                               tabname : "data-loading",
                               tabico : "dataload",
                               types: "^svc-archiver$,^svc-download$,^svc-compressor$",
                               url : { type : "^svc-data-loading-base$%2B" }
                           }, {
                               tabname : "projects",
                               tabico : "bio-project",
                               types: "^HIVE-project$",
                               url : { type : "^HIVE-project$" }
                           }, {
                               tabname : "samples",
                               tabico : "bio-sample",
                               types: "^HIVE-sample$,^HIVEBioSample",
                               url : { type : "^HIVE-sample$,^HIVEBioSample" }
                           }, {
                               tabname : "experiments",
                               tabico : "bio-experiment",
                               types: "^HIVE-experiment$",
                               url : { type : "^HIVE-experiment$" }
                           }, {
                               tabname : "runs",
                               tabico : "bio-run",
                               types: "^HIVE-run$",
                               url : { type : "^HIVE-run$" }
                           }, {
                               tabname : "protocol",
                               tabico : "ico-file",
                               types: "^HIVE-protocol$",
                               url : { type : "^HIVE-protocol$" }
                           } ];

    this.folderURL =  "http:
    this.aa_testAttribute = true;
    vjExplorerBaseView.call(this,viewer);
}

