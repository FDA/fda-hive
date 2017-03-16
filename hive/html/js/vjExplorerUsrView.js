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
                               url : { type : "^HIVE_Development_Project_List$"
                               }
                           }, {
                               tabname : "annotations",
                               tabico : "rec",
                               types: "^u-annot$,^u-ionAnnot$",
                               url : { type : "^u-annot$,^u-ionAnnot$,^u-ionExpress$" }
                           }, {
                               tabname : "files",
                               tabico : "ico-file",
                               types: "^u-file$,^excel-file$,-table$", // exclude all svc
                               url : { type : "^u-file$,^table$%2B,^u-idList$" }
                           }, {
                               tabname : "spectra",
                               tabico : "ico-file",
                               types: "^spectra$,^spectra-MS$,^spectra-lib$",
                               url : { type : "spectra,spectra-MS,spectra-lib" }
                           }, {
                               tabname : "images",
                               tabico : "ico-image",
                               types: "^image$",
                               url : { type : "^image$" }
                           }, {
                               tabname : "computations",
                               tabico : "svc-process",
                               types: "^svc(?!-(archiver|download)$).*$",
                               url : { type : "svc-.*,!svc-archiver,!svc-download",//align,svc-prof,svc-recomb,svc-popul,svc-clust,svc-algo,svc-spectra,svc-dmTableToViodbParser,svc-algo-annotMapper,svc-hiveseq,svc-genemark,svc-textClustering,svc-dna-screening,svc-mothur",
                                   prop:"svcTitle,status,progress100,name,uri,created,svc,submitter"}
                           }, {
                               tabname : "data-loading",
                               tabico : "dataload",
                               types: "^svc-archiver$,^svc-download$",
                               url : { type : "^svc-archiver$,^svc-download$",
                                   prop:"svcTitle,status,progress100,name,uri,created"
                                   }
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

    this.folderURL =  "http://?cmd=objList&type=sysfolder,folder&actions=1&cnt=1000000&prop=_type,name,child&mode=csv&showtrash=true";
    vjExplorerBaseView.call(this,viewer);
}

//# sourceURL = getBaseUrl() + "/js/vjExplorerUsrView.js"