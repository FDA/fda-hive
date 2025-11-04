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
const mapping = {
    default:{
        tabs: [["JsonTab", "JSON"], ["ShareTab", "Sharing"],['DetailTab','Details']]
    },
    image:{
        tabs:[["ImageTab", "Image"]]
    },
    "u-file":{},
    "csv-table": {
            parent: "u-file",
            tabs: [["TablePreviewTab", "Table Preview"]]
    },
    "tsv-table": {
            parent: "u-file",
            tabs: [["TablePreviewTab", "Table Preview"]]
    },
    "u-ionAnnot":{
        tabs:[["AnnotationTab", "Annotation"]]
    },
    process:{
        tabs:[["ProgressTab", "Progress"], ["DownloadsTab", "Download All"]]
    },
        "svc-dna-screening": {
            parent: "process"
        },
        "svc-compressor": {
            parent: "process"
        },
        "svc-archiver": {
            parent: "process"
        },
        "svc-computations-base" : {
            parent: "process"
        },
            "svc-dna-insilico":{
                parent: "svc-computations-base"
            },
        "svc-pipeline-base":{
            parent:"process"
        },
            "svc-pipeline-aad-full":{
                parent:"svc-pipeline-base"
            },
            "svc-pipeline-sra":{
                parent:"svc-pipeline-base"
            },
                "svc-pipeline-sra-sam":{
                    parent:"svc-pipeline-sra"
                },
        "svc-bcl2fastq":{
            parent:"process"
        },
        "svc-metaphlan2": {
            parent:"process"
        },
        "svc-ncbi-prefetch-2.10.7":{
            parent:"process"
        },
        "svc-ncbi-fasterq-dump-2.10.7":{
            parent:"process"
        },
    folder: {},
    svc:{
        tabs:[["ProgressTab", "Progress"] , ["DownloadsTab", "Download All"]]
    },
        "svc-adverse-event-dedup":{
            parent: "svc",
        },
        "svc-algorlda":{
            parent: "svc",
            tabs:[["EigenvectorsTab", "Eigenvectors"]]
        },
        "svc-align":{
            parent: "svc"
        },
            "svc-align-blat":{
                parent: "svc-align"
            },
            "svc-align-blast":{
                parent: "svc-align"
            },
            "svc-align-blastx":{
                parent: "svc-align"
            },
            "svc-align-tblastx":{
                parent: "svc-align"
            },
            "svc-align-bowtie":{
                parent: "svc-align"
            },
            "svc-align-bwa":{
                parent: "svc-align"
            },
            "svc-align-hexagon":{
                parent: "svc-align"
            },
            "svc-hexagon-batch":{
                parent: "svc-align"
            },
            "svc-align-tophat":{
                parent: "svc-align"
            },
            "svc-align-mafft":{
                parent: "svc-align"
            },
            "svc-align-multiple":{
                parent: "svc-align"
            },
                "svc-align-clustal":{
                    parent: "svc-align-multiple"
                },
            "svc-viral-mutation-comp": {
                parent: "svc-align"
            },
        "svc-alignment":{
            parent: "svc"
        },
        "svc-download":{
            parent: "svc"
        },
        "svc-hiveseq":{
            parent: "svc"
        },
    "svc-algo-annotMapper": {},
    "svc-clust": {},
    "svc-profiler-refcmp": {},
        "svc-profiler": {
            tabs:[["ProfileTab", "Profile"]]
        },
            "svc-profiler-heptagon":{
                parent: "svc-profiler"
            },
    "excel-file": {},
    "u-hivepack": {},
    "u-hiveseq": {
        tabs:[["SequencesTab", "Sequences"], ["HistogramTab", "Histogram"], ["ACGTTab", "ACGT"],["CodonTab", "Codon QC"], ["PositionalQCTab", "Positional QC"]]
    },
        "nuc-read": {
            parent: "u-hiveseq"
        },
        "genome": {
            parent: "u-hiveseq"
        }
};

export default mapping;