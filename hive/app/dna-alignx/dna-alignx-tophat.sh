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

dna_alingx_build() {

    echo -e "@dummy1\nATGCGCGTGCGCTAATGCGCGTGCGCTAATGCGCGTGCGCTAATGCGCGTGCGCTAATGCGCGT\n+\nIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII\n" > $indexPath-fict.fq

    # tophat adds .fa
    local indexPathFile=${indexPath%.*}
    cmd="bowtie2-build  $addArgs $referenceFile $indexPathFile" 
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi

    cmd="python-run.os${os} 2.7 tophat --transcriptome-index `dirname ${indexPath}`/gtfIndex -p 8 -g 1 --library-type fr-unstranded --no-coverage-search --no-novel-indels -G $annotationFile -M $indexPathFile $indexPath-fict.fq"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
}

dna_alingx_align() {

    nm="accepted_hits"
    local indexPathFile=${indexPath%.*}

    # launch tophat

    # echo "tophat --output-dir $outPath --transcriptome-index `dirname ${indexPath}`/gtfIndex -p 8 -g 1 --library-type fr-unstranded --no-coverage-search --no-novel-indels -G $annotationFile -M ${indexPath%.*} $queryFiles" > mycommandline.txt

    cmd="python-run.os${os} 2.7 tophat --output-dir $outPath --transcriptome-index `dirname ${indexPath}`/gtfIndex -p 1 -g 1 --library-type fr-unstranded --no-coverage-search --no-novel-indels -G $annotationFile -M $indexPathFile $queryFiles"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
    # sort .bam file 
    cmd="samtools sort -n $outPath/${nm}.bam $outPath/${nm}_sorted"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
    # make a .sam out of bam 
    cmd="samtools view -o $outPath.sam $outPath/${nm}_sorted.bam"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
}

dna_alingx_finalize() {

    resDir=`dirname ${resultPath}`
    nm="all_accepted_hits"
    listBams=""
    cntBams=0
    if [[ ${finalFiles:0:1} == "@" ]]; then
        finalFiles=`cat ${finalFiles:1}`
    fi
    for di in ${finalFiles}; do
        let cntBams=$((cntBams+1))
        listBams="${listBams} ${di}"
    done
    # merge all bam files if there are multiple
    if [[ $cntBams -gt 1 ]]; then
        cmd="samtools merge -f ${outPath}/${nm}.bam $listBams"
    else
        cmd="ln -s $listBams ${outPath}/${nm}.bam"
    fi
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
    # sort the resulting bam   
    cmd="samtools sort ${outPath}/${nm}.bam ${outPath}/${nm}_sorted"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
    # make a .sam out of bam 
    cmd="samtools view -h -o ${outPath}/${nm}_sorted.sam ${outPath}/${nm}_sorted.bam"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi

    # attach result in SAM format as downloadble
    cp -pv ${outPath}/${nm}_sorted.sam ${resDir}/${nm}_sorted.sam

    # calculate gene.fpkm_tracking, write into cufflinks_out dir
    cmd="cufflinks --quiet --no-update-check -o ${outPath} -p 1 -G $annotationFile ${outPath}/${nm}_sorted.bam"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi

    # rename files to be grabbed by alignx
    mv -v ${outPath}/genes.fpkm_tracking ${resDir}/genes_fpkm.txt
    mv -v ${outPath}/isoforms.fpkm_tracking ${resDir}/isoforms_fpkm.txt
    # calculate flagstat.txt, write into current dir
    cmd="samtools flagstat ${outPath}/${nm}.bam > ${resDir}/flagstat.txt"
    echo "$cmd"
    $cmd; if [ $? != 0 ]; then exit $?; fi
}
