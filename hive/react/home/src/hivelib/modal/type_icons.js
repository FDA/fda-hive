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
const TypeIcons = {
    "image": {
    },

    "u-file": {
        "icon": "ico-file.gif",
    },

    "process": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "action": {
        "icon": "rec.gif",
        "parent": "svc",
    },

    "email": {
        "icon": "rec.gif",
        "parent": "svc",
    },

    "folder": {
        "icon": "img/folder-open.gif",
        "parent": "svc",
    },

    "group": {
        "icon": "rec.gif",
        "parent": "svc",
    },

    "HIVE-experiment": {
        "icon": "bio-experiment.gif",
        "parent": "svc",
    },

    "HIVE-project": {
        "icon": "bio-project.gif",
        "parent": "svc",
    },

    "HIVE-run": {
        "icon": "bio-run.gif",
        "parent": "svc",
    },

    "HIVE-sample": {
        "icon": "bio-sample.gif",
        "parent": "svc",
    },

    "notification": {
        "icon": "rec.gif",
        "parent": "svc",
    },

    "svc": {
        "icon": "rec.gif",
        "parent": "svc",
    },

    "svc-algorlda2": {
        "icon": "img/scope.png",
        "parent": "svc",
    },

    "svc-algorlda": {
        "icon": "img/64/rlda.png",
        "parent": "svc",
    },

    "svc-align": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment",
    },

    "svc-align-blat": {
        "icon": "img-algo/==.gif",
        "parent": "svc-alignment",
    },

    "svc-align-blat-36": {
        "icon": "img-algo/svc-align-blat.gif",
        "parent": "svc-align-blat"
    },

    "svc-align-blast": {
        "icon": "img-algo/==.gif",
        "parent": "svc-alignment",
    },

    "svc-align-blast-2.9": {
        "icon": "img-algo/svc-align-blast.gif",
        "parent": "svc-alignment"
    },

    "svc-align-blastx": {
        "icon": "img-algo/==.gif",
        "parent": "svc-alignment",
    },

    "svc-align-blastx-2.9": {
        "icon": "img-algo/svc-align-blastx.gif"
    },

    "svc-align-tblastx": {
        "icon": "img-algo/==.gif",
        "parent": "svc-align-blastx",
    },


    "svc-align-tblastx-2.9": {
        "icon": "img-algo/svc-align-tblastx.gif",
        "parent": "svc-align-blastx"
    },

    "svc-align-bowtie": {
        "icon": "img-algo/==.gif",
        "parent": "svc-alignment",
    },

    "svc-align-bowtie-1.2.2": {
        "icon": "img-algo/svc-align-bowtie.gif",
        "parent": "svc-align-bowtie"
    },

    "svc-align-bwa": {
        "icon": "img-algo/==.gif",
        "parent": "svc-alignment",
    },

    "svc-align-hexagon": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment",
    },

    "svc-hexagon-batch": {
        "icon": "svc-align-hexagon",
        "parent": "svc-alignment",
    },

    "svc-align-magic": {
        "icon": "img-algo/svc-align-magic.gif",
        "parent": "svc-alignment",
    },

    "svc-align-tophat": {
        "icon": "img-algo/==.gif",
        "parent": "svc-alignment",
    },

    "svc-align-multiple": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment-multiple",
    },

    "svc-alignment-multiple": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment",
    },

    "svc-alignment": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment",
    },

    "svc-align-mafft": {
        "icon": "img-algo/svc-align-mafft.gif",
        "parent": "svc-alignment-multiple",
    },

    "svc-align-mafft-7.407": {
        "icon": "img-algo/svc-align-mafft.gif",
        "parent": "svc-alignment-multiple"
    },

    "svc-align-clustal": {
        "icon": "img-algo/svc-align-clustal.gif",
        "parent": "svc-alignment-multiple",
    },

    "svc-archiver": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-compressor": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-dna-screening": {
        "icon": "img/scope.png",
        "parent": "svc",
    },

    "svc-align-screening": {
        "icon": "process.gif",
        "parent": "svc-align-screening",
    },

    "svc-algo-annotMapper": {
        "icon": "img/scope.png",
        "parent": "svc-align-screening",
    },

    "svc-profiler-refcmp": {
        "icon": "svc-process",
        "parent": "svc-profiler-refcmp"
    },

    "user-info": {
        "icon": "help.gif",
        "parent": "user-info",
    },

    "svc-clust": {
        "icon": "img-algo/svc-clust.png",
        "parent": "svc-clust",
    },

    "svc-denove-velvet": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-dmNgsPred": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-dmSnvDis": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-dmStrDistri": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-dmUniPDBmap": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-download": {
        "icon": "download.gif",
        "parent": "svc",
    },

    "svc-hiveseq": {
        "icon": "hiveseq.gif",
        "parent": "svc",
    },

    "svc-panel": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-popul": {
        "icon": "img-algo/svc-popul.gif",
        "parent": "svc-popul",
    },

    "svc-profiler": {
        "icon": "img/heptagon.gif",
        "parent": "svc-profiler",
    },

    "svc-diprofiler": {
        "icon": "process.gif",
        "parent": "svc-profiler",
    },

    "svc-profiler-heptagon": {
        "icon": "img/heptagon.gif",
        "parent": "svc-heptagon",
    },

    "svc-heptagon1": {
        "icon": "img/heptagon.gif",
        "parent": "svc-heptagon",
    },

    "svc-recomb": {
        "icon": "img-algo/svc-recomb.gif",
        "parent": "svc-recomb",
    },

    "svc-sb-Donor": {
        "icon": "process.gif",
        "parent": "svc-recomb",
    },

    "svc-sb-Experiment": {
        "icon": "process.gif",
        "parent": "svc-recomb",
    },

    "svc-sb-Gene": {
        "icon": "process.gif",
        "parent": "svc-recomb",
    },

    "svc-sb-Sample": {
        "icon": "process.gif",
        "parent": "svc-recomb",
    },

    "svc-SingleCellPCR": {
        "icon": "process.gif",
        "parent": "svc-recomb",
    },

    "svc-sb-Treatment": {
        "icon": "process.gif",
        "parent": "svc-recomb",
    },

    "svc-spectraPeakDetection": {
        "icon": "img-algo/svc-spectraPeakDetection.jpg",
        "parent": "svc-spectraPeakDetection",
    },

    "spectra": {
        "icon": "ico-file.gif",
        "parent": "svc-spectraPeakDetection",
    },

    "spectra-MS": {
        "icon": "ico-file.gif",
        "parent": "spectra",
    },

    "spectra-lib": {
        "icon": "ico-file.gif",
        "parent": "spectra",
    },

    "svc-denovo-oases": {
        "icon": "process.gif",
        "parent": "spectra",
    },

    "u-annot": {
        "icon": "rec.gif",
        "parent": "spectra",
    },

    "u-idList": {
        "icon": "list.gif",
        "parent": "spectra",
    },

    "u-ionAnnot": {
        "icon": "ionAnnot-db.png",
        "parent": "spectra",
    },

    "csv-table": {
        "icon": "table.gif",
        "parent": "u-file",
    },

    "tsv-table": {
        "icon": "table.gif",
        "parent": "u-file",
    },

    "excel-file": {
        "icon": "table.gif",
        "parent": "u-file",
    },

    "u-hivepack": {
        "icon": "rec.gif",
        "parent": "u-file",
    },

    "u-hiveseq": {
        "icon": "dna.gif",
        "parent": "u-file",
    },

    "nuc-read": {
        "icon": "dnaold.gif",
        "parent": "u-hiveseq",
    },

    "genome": {
        "icon": "dna.gif",
        "parent": "u-hiveseq",
    },

    "user": {
        "icon": "user.gif",
        "parent": "u-hiveseq",
    },

    "viodb": {
        "icon": "rec.gif",
        "parent": "u-hiveseq",
    },

    "svc-dna-demo": {
        "icon": "process.gif",
        "parent": "u-hiveseq",
    },

    "svc-genemark": {
        "icon": "process.gif",
        "parent": "u-hiveseq",
    },

    "svc-textclustering": {
        "icon": "process.gif",
        "parent": "u-hiveseq",
    },

    "svc-mothur": {
        "icon": "process.gif",
        "parent": "u-hiveseq",
    },

    "svc-genome-comparator": {
        "icon": "process.gif",
        "parent": "u-hiveseq",
    },

    "svc-generic-launcher": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-msgfplus": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-glymps": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-affinity-viz-peak-detect": {
        "icon": "process.gif",
        "parent": "svc",
    },

    "svc-align2": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-blast2": {
        "icon": "img-algo/svc-align-blast.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-blastx2": {
        "icon": "img-algo/svc-align-blastx.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-tblastx2": {
        "icon": "img-algo/svc-align-tblastx.gif",
        "parent": "svc-align-blastx2",
    },

    "svc-align-bowtie1": {
        "icon": "img-algo/svc-align-bowtie.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-bwa2": {
        "icon": "img-algo/svc-align-bwa.jpg",
        "parent": "svc-alignment2",
    },

    "svc-align-hexagon2": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment2",
    },

    "svc-hexagon-batch2": {
        "icon": "svc-align-hexagon",
        "parent": "svc-alignment2",
    },

    "svc-align-magic2": {
        "icon": "img-algo/svc-align-magic.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-tophat2": {
        "icon": "img-algo/svc-align-tophat.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-hisat2": {
        "icon": "img-algo/svc-align-hisat2.gif",
        "parent": "svc-alignment2"
    },

    "svc-align-hisat2-2.1.0": {
        "icon": "img-algo/svc-align-hisat2.gif",
        "parent": "svc-alignment2"
    },
    "svc-hisat2-align-2.1.0": {
        "icon": "img-algo/hisat2.png",
        "parent": "svc"
    },

    "svc-align-blat2": {
        "icon": "img-algo/svc-align-blat.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-bowtie2": {
        "icon": "img-algo/svc-align-bowtie.gif",
        "parent": "svc-alignment2"
    },

    "svc-align-bowtie2-2.3.5": {
        "icon": "img-algo/svc-align-bowtie.gif",
        "parent": "svc-align-bowtie2"
    },

    "svc-alignment2": {
        "icon": "img/processSvc.gif",
        "parent": "svc-alignment2",
    },

    "svc-align-mafft2": {
        "icon": "img-algo/svc-align-mafft.gif",
        "parent": "svc-alignment-multiple2",
    },

    "svc-align-clustal2": {
        "icon": "img-algo/svc-align-clustal.gif",
        "parent": "svc-alignment-multiple2",
    },

    "svc-alignment-remapper": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-dna-codonQC": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-dna-multi-qc": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-dna-alignQC": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-dna-targetQC": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-dna-kmerQC": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-differential-profiler": {
        "icon": "process.gif",
        "parent": "svc-alignment2",
    },

    "svc-clust2": {
        "icon": "img-algo/svc-clust.png",
        "parent": "svc-alignment2",
    },

    "svc-orfFinder": {
        "icon": "img-algo/svc-orfFinder.png",
        "parent": "svc-alignment2",
    },
    //------------------------------------------------//
    //------------------------------------------------//
    //------------------------------------------------//
    "svc-bcl2fastq": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-canu": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-canu-1x": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-dna-spades": {
        "icon": "process.gif",
    },

    "svc-velvet-single": {
        "icon": "process.gif",
    },

    "svc-samtools-index-1.9.1": {
        "icon": "process.gif",
    },

    "svc-samtools-view-1.9.1": {
        "icon": "process.gif",
    },

    "svc-agilent-mbc": {
        "icon": "process.gif",
    },

    "svc-test_gl_callbacks": {
        "icon": "process.gif",
    },

    "svc-clin-data-uploader": {},

    "svc-gut-feeling": {},

    /// DRAGEN
    "svc-dragen-index": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-dnaseq": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-full": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-full-3.6": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-aligner": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-aligner": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-small-variant": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-only-small-variant": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-normal-small-variant": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-cnv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-panel-cnv1": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-panel-cnv2": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-only-cnv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-normal-cnv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-small-variant-cnv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-germline-sv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-only-sv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-tumor-normal-sv": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-short-tandem-repeats": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    "svc-dragen-methylation": {
        "icon": "img-algo/dragen_logo.png",
        "parent": "svc"
    },

    ///////////

    "svc-adverse-event-dedup": {
        "icon": "process.gif",
        "parent": "svc"
    },

    // NCBI TOOLKIT
    "svc-ncbi-prefetch": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-ncbi-fasterq-dump": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-ncbi-sam-dump": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-2stp": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-3stp": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-aad-full": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-aad-test": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-aad4step": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-debug": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-sra-fastq": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-sra-sam": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-upload": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-amr-short": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-upload-processor": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-run-deepvariant-1.0.0": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-multiqc-1.9": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-multiqc-1.11": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-fastp-0.20.0": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-variant-conserv-gen-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-HIVE-RNA-Seq": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-lofreq-alnqual-2.1.5": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "lofreq-call-2.1.5": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-lofreq-checkref-2.1.5": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-lofreq-indelqual-2.1.5": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-lofreq-somatic-2.1.5": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-lofreq-viterbi-2.1.5": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pyir-1.3.3": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-alphafold-2.1.2": {
        "icon": "process.gif",
        "parent": "svc"
    },

    "svc-download-http": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-download-ftp": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-download-dropbox": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-download-ncbi-genome": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-download-ncbi-exome": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-download-ncbi-genbank": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-download-http": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-download-ftp": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-download-ncbi-genbank": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-download-ncbi-genome": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-download-ncbi-exome": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-download-dropbox": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-cellranger-aggr-3.0.2": {
        "icon": "img-algo/10x_Genomix_logo.png",
        "parent": "svc"
    },
    "svc-cellranger-mkref-3.0.2": {
        "icon": "img-algo/10x_Genomix_logo.png",
        "parent": "svc"
    },
    "svc-cellranger-mkfastq-3.0.2": {
        "icon": "img-algo/10x_Genomix_logo.png",
        "parent": "svc"
    },
    "svc-cellranger-count-3.0.2": {
        "icon": "img-algo/10x_Genomix_logo.png",
        "parent": "svc"
    },
    "svc-cellranger_arc-mkfastq-2.0.1": {
        "icon": "img-algo/10x_Genomix_logo.png",
        "parent": "svc"
    },
    "svc-cellranger_arc-count-2.0.1": {
        "icon": "img-algo/10x_Genomix_logo.png",
        "parent": "svc"
    },
    "svc-minimap2-align-2.17": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-minimap2-index-2.17": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-trimmomatic-0.39": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-kallisto-quant-0.46.2": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-kallisto-index-0.46.2": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-price-1.2.1": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-hive-umitools.0.5.5": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-ANARCI-1.2": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-deseq": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-align-hisat2-2.2.1": {
        "icon": "img-algo/hisat2.png",
        "parent": "svc"
    },
    "svc-hisat2-index-2.2.1": {
        "icon": "img-algo/hisat2.png",
        "parent": "svc"
    },
    "svc-featureCounts-2.0.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-fastqc-0.11.9": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-humann2": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-docket-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-dna-insilico": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-snpEff-5.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-fdms-download-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-rgi-5.2.1": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-picard-markduplicates-2.26.11": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-picard-collectrnaseqmetrics-2.26.11": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-picard-collectinsertsizemetrics-2.26.11": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-picard-collecthsmetrics-2.26.11": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-unicycler-0.5.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-unicycler-0.4.8": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-docket": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-amr-merge": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-amr-merge-long": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-amr-merge-hybrid": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-dcgt_json_converter-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-dcgt-rnaseq": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-dcgt-dnaseq": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-rnaseqc-2.4.2": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-docket-ai-summary-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-docket-2.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-samtools-fastq-1.17": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-pipeline-DRM-oneclick": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-regulatory_keyword_search-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-otp-report-qc-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-otp-excel-qc-1.0": {
        "icon": "processSvc",
        "parent": "svc"
    },
    "svc-otp-qc-conversion-1.0": {
        "icon": "process.gif",
        "parent": "svc"
    },
    "svc-codon-usage": {
        "icon": "process.gif",
        "parent": "svc"
    },
}
export default TypeIcons;
