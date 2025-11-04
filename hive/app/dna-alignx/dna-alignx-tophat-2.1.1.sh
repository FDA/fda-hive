#!/bin/sh
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

$(setenv("mico2", "tophat", false))

. "${QPRIDE_BIN}/dna-alignx.sh.os${os}"

# tophat adds .fa
idxName="$(.indexPath)"
# cut extension
indexPathFile=${idxName%.*}
annotIdxName=`dirname "${indexPathFile}"`
annotIdxName=`basename "${annotIdxName}"`

dna_alignx_index() {

    dna_alignx_exec bowtie2-build "$(.referenceFile)" "${indexPathFile}"
}

dna_alignx_align() {

    annotIdxPath="`dirname "$(.annotationFile)"`/${annotIdxName}"
    # check if index ver is not empty file
    if [[ ! -d "${annotIdxPath}" ]]; then
        let i=${RANDOM}%10
        sleep $i # to avoid parallel request collision, very bad impl...
        if [[ ! -d "${annotIdxPath}" ]]; then
            dna_alignx_exec mkdir "${annotIdxPath}"
            local fictSeq="${annotIdxPath}/fict.fq"
            cat <<EOF>"${fictSeq}"
@dummy
ATGCGCGTGCGCTAATGCGCGTGCGCTAATGCGCGTGCGCTAATGCGCGTGCGCTAATGCGCGT
+
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
EOF
            dna_alignx_exec tophat --transcriptome-index "${annotIdxPath}/gtfIndex" -p 8 -g 1 \
                --library-type fr-unstranded --no-coverage-search \
                --no-novel-indels -G "$(.annotationFile)" -M "${indexPathFile}" "${fictSeq}"
        fi
    fi
    dna_alignx_exec tophat --output-dir "$(.outPath)" --transcriptome-index "${annotIdxPath}/gtfIndex" -p 1 -g 1 \
        --library-type fr-unstranded --no-coverage-search --no-novel-indels -G "$(.annotationFile)" -M "${indexPathFile}" $(.queryFiles)

    local nm="accepted_hits" # tophat result filename
    $(setenv("mico3", "sam0119", true))
    dna_alignx_exec samtools sort -n "$(.outPath)/${nm}.bam" "$(.outPath)/${nm}_sorted"
    # make a .sam out of bam 
    dna_alignx_exec samtools view -o "$(.outPath).sam" "$(.outPath)/${nm}_sorted.bam"
}

dna_alignx_finalize() {

    $(setenv("mico3", "sam0119", true))

    resDir="`dirname $(.resultPath)`"
    outPath="`dirname $(.outPath)`"
    local nm="all_accepted_hits"
    local listBams=""
    local cntBams=0
    local finalFiles="$(.finalFiles)"
    if [[ ${finalFiles:0:1} == "@" ]]; then
        finalFiles=`cat ${finalFiles:1}`
    fi
    for di in ${finalFiles}; do
        let cntBams=${cntBams}+1
        listBams="${listBams} ${di}"
    done
    # merge all bam files if there are multiple
    if [[ $cntBams -gt 1 ]]; then
        dna_alignx_exec samtools merge -f "${outPath}/${nm}.bam" "$listBams"
    else
        dna_alignx_exec ln -s ${listBams} "${outPath}/${nm}.bam"
    fi
    # sort the resulting bam
    dna_alignx_exec samtools sort "${outPath}/${nm}.bam" "${outPath}/${nm}_sorted"
    # make a .sam out of bam
    dna_alignx_exec samtools view -h -o "${outPath}/${nm}_sorted.sam" "${outPath}/${nm}_sorted.bam"

    # attach result in SAM format as downloadble
    dna_alignx_exec cp -pv "${outPath}/${nm}_sorted.sam" "${resDir}/${nm}_sorted.sam"

    $(setenv("mico3", "cufflinks", true))
    # calculate gene.fpkm_tracking, write into cufflinks_out dir
    dna_alignx_exec cufflinks --quiet --no-update-check -o "${outPath}" -p 1 -G "$(.annotationFile)" "${outPath}/${nm}_sorted.bam"
    # cufflinks generate TRANSCRIPTS.GTF file
    # need to move from tmp out folder to result folder
    # otherwise, cuffdiff requires this file to run the analysis
    dna_alignx_exec mv -v "${outPath}/transcripts.gtf" "${resDir}/transcripts.gtf"

    $(setenv("mico3", "sam0119", true))
    # rename files to be grabbed by alignx
    dna_alignx_exec mv -v "${outPath}/genes.fpkm_tracking" "${resDir}/genes_fpkm.txt"
    dna_alignx_exec mv -v "${outPath}/isoforms.fpkm_tracking" "${resDir}/isoforms_fpkm.txt"
    # calculate flagstat.txt, write into current dir
    dna_alignx_exec samtools flagstat "${outPath}/${nm}.bam" > "${resDir}/flagstat.txt"

    # TODO calculate counts.txt, write into current dir
#    dna_alignx_exec htseq-count --quiet --stranded=no --mode=intersection-strict "${outPath}/${nm}_sorted.sam" "$(.annotationFile)" > "${outPath}/counts.txt"
#    dna_alignx_exec mv -v "${outPath}/counts.txt "${resDir}/counts.txt"
}

dna_alignx_main "$@"
