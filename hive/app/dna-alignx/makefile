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

PROGRAMS = dna-alignx

APPS = \
		BLAST-(Bacteria-specific):1 \
		BLAST-(Distant-relative):1 \
		BLAST-(Exome-specific):1 \
		BLAST-(Human-whole-genome-specific):1 \
		BLAST-(Standard):1 \
		BLAST-(Transcriptome-specific):1 \
		BLAST-(Virus-specific):1 \
		BLASTX-(Bacteria-specific):1 \
		BLASTX-(Distant-relative):1 \
		BLASTX-(Exome-specific):1 \
		BLASTX-(Human-whole-genome-specific):1 \
		BLASTX-(Standard):1 \
		BLASTX-(Transcriptome-specific):1 \
		BLASTX-(Virus-specific):1 \
		BLAT:1 \
		BWA-(Bacteria-specific):1 \
		BWA-(Distant-relative):1 \
		BWA-(Exome-specific):1 \
		BWA-(Human-whole-genome-specific):1 \
		BWA-(Standard):1 \
		BWA-(Transcriptome-specific):1 \
		BWA-(Virus-specific):1 \
		Bowtie-(Bacteria-specific):1 \
		Bowtie-(Distant-relative):1 \
		Bowtie-(Exome-specific):1 \
		Bowtie-(Human-whole-genome-specific):1 \
		Bowtie-(Standard):1 \
		Bowtie-(Transcriptome-specific):1 \
		Bowtie-(Virus-specific):1 \
		Bowtie2-(Bacteria-specific):1 \
		Bowtie2-(Distant-relative):1 \
		Bowtie2-(Exome-specific):1 \
		Bowtie2-(Human-whole-genome-specific):1 \
		Bowtie2-(Standard):1 \
		Bowtie2-(Transcriptome-specific):1 \
		Bowtie2-(Virus-specific):1 \
		Clustal-Multiple-ASlignment:1 \
		DNA-Batch:1 \
		HISAT2-(Human-Alignment):1 \
		HISAT2-(Rat-Alignment):1 \
		MAFFT-Multiple-Alignment:1 \
		TBLASTX-(Bacteria-specific):1 \
		TBLASTX-(Distant-relative):1 \
		TBLASTX-(Exome-specific):1 \
		TBLASTX-(Human-whole-genome-specific):1 \
		TBLASTX-(Standard):1 \
		TBLASTX-(Transcriptome-specific):1 \
		TBLASTX-(Virus-specific):1 \
		Tophat-(Human-Alignment):1 \
		Tophat-(Rat-Alignment):1 \
MENUITEMS = \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAST_Alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLASTX_Alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BLAT \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie_Alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/Bowtie2_Alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/BWA_alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/DNA_Batch \
\
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/TBLASTX_Alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/Clustal_multiple_alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Standard \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/Multiple_Alignment/MAFFT_multiple_alignment/Virus_specific \
\
/Portal/Sequence_Alignment_on_Genome/RNA-seq/HISAT2_RNA_alignment \
/Portal/Sequence_Alignment_on_Genome/RNA-seq/HISAT2_RNA_alignment/Human_Alignment \
/Portal/Sequence_Alignment_on_Genome/RNA-seq/HISAT2_RNA_alignment/Rat_Alignment \
\
/Portal/Sequence_Alignment_on_Genome/RNA-seq/Tophat_RNA_alignment \
/Portal/Sequence_Alignment_on_Genome/RNA-seq/Tophat_RNA_alignment/Human_Alignment \
/Portal/Sequence_Alignment_on_Genome/RNA-seq/Tophat_RNA_alignment/Rat_Alignment


PROGRAMS_LIBS = -lviolib $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB)

OBJECT_TYPES= \
    svc-align-blast \
    svc-align-blastx \
    svc-align-blat \
    svc-align-bowtie \
    svc-align-bowtie2 \
    svc-align-bwa \
    svc-align-clustal \
    svc-align-mafft \
    svc-align-tblastx \
    svc-align-tophat \
    svc-align-hisat2 \
    svc-align-bowtie2-2.3.5 \
    svc-align-mafft-7.407 \
    svc-align-hisat2-2.1.0 \
    svc-align-blat-36 \
    svc-align-bowtie-1.2.2 \
    svc-align-blast-base \
    svc-align-blast-2.9 \
    svc-align-blastx-2.9 \
    svc-align-tblastx-2.9 \

dna-alignx_SRCS = dna-alignx.cpp

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist:: algo_install
	$(call dist-app,qm,dna-alignx,dna-alignx.os$(SLIB_PLATFORM))

	$(call dist-service-register,dna-alignx-blast, NCBI Nucleotide-Nucleotide BLASTn,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-blast, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-blast,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-blast,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-blast,dna-alignx-blast-common.sh.os$(SLIB_PLATFORM),dna-alignx-blast-common.sh)
	$(call dist-src,dna-alignx-blast,dna-alignx-blast-2.9.sh.os$(SLIB_PLATFORM),dna-alignx-blast-2.9.sh)
	$(call dist-src,dna-alignx-blast,dna-alignx-blast-2.2.30.sh.os$(SLIB_PLATFORM),dna-alignx-blast-2.2.30.sh)

	$(call dist-service-register,dna-alignx-blastx,NCBI Translated Query-Protein Subject BLASTx,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-blastx, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-blastx,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-blastx,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-blastx,dna-alignx-blast-common.sh.os$(SLIB_PLATFORM),dna-alignx-blast-common.sh)
	$(call dist-src,dna-alignx-blastx,dna-alignx-blastx-2.2.30.sh.os$(SLIB_PLATFORM),dna-alignx-blastx-2.2.30.sh)
	$(call dist-src,dna-alignx-blastx,dna-alignx-blastx-2.9.sh.os$(SLIB_PLATFORM),dna-alignx-blastx-2.9.sh)

	$(call dist-service-register,dna-alignx-tblastx,NCBI Translated Query-Translated Subject tBLASTx,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-tblastx, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-tblastx,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-tblastx,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-tblastx,dna-alignx-blast-common.sh.os$(SLIB_PLATFORM),dna-alignx-blast-common.sh)
	$(call dist-src,dna-alignx-tblastx,dna-alignx-tblastx-2.2.30.sh.os$(SLIB_PLATFORM),dna-alignx-tblastx-2.2.30.sh)
	$(call dist-src,dna-alignx-tblastx,dna-alignx-tblastx-2.9.sh.os$(SLIB_PLATFORM),dna-alignx-tblastx-2.9.sh)

	$(call dist-service-register,dna-alignx-blat,BLAT fast sequence search,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-blat, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-blat,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-blat,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-blat,dna-alignx-blat-35x1.sh.os$(SLIB_PLATFORM),dna-alignx-blat-35x1.sh)
	$(call dist-src,dna-alignx-blat,dna-alignx-blat-36.sh.os$(SLIB_PLATFORM),dna-alignx-blat-36.sh)

	$(call dist-service-register,dna-alignx-bowtie,Bowtie short read aligner,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-bowtie, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-bowtie,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-bowtie,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-bowtie,dna-alignx-bowtie-0.12.8.sh.os$(SLIB_PLATFORM),dna-alignx-bowtie-0.12.8.sh)
	$(call dist-src,dna-alignx-bowtie,dna-alignx-bowtie-1.2.2.sh.os$(SLIB_PLATFORM),dna-alignx-bowtie-1.2.2.sh)

	$(call dist-service-register,dna-alignx-bowtie2,Bowtie 2 read alignment,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-bowtie2, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-bowtie2,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-bowtie2,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-bowtie2,dna-alignx-bowtie2-2.1.0.sh.os$(SLIB_PLATFORM),dna-alignx-bowtie2-2.1.0.sh)
	$(call dist-src,dna-alignx-bowtie2,dna-alignx-bowtie2-2.3.5.sh.os$(SLIB_PLATFORM),dna-alignx-bowtie2-2.3.5.sh)

	$(call dist-service-register,dna-alignx-bwa,Burrows-Wheeler Aligner \(BWA\),/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-bwa, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-bwa,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-bwa,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-bwa,dna-alignx-bwa-0.7.17.sh.os$(SLIB_PLATFORM),dna-alignx-bwa-0.7.17.sh)

	$(call dist-service-register,dna-alignx-clustal,Clustal Omega multiple sequence alignment,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-clustal, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-clustal,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-clustal,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-clustal,dna-alignx-clustal-1.2.4.sh.os$(SLIB_PLATFORM),dna-alignx-clustal-1.2.4.sh)

	$(call dist-service-register,dna-alignx-mafft,MAFFT multiple sequence alignment,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-mafft, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-mafft,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-mafft,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-mafft,dna-alignx-mafft-7.130.sh.os$(SLIB_PLATFORM),dna-alignx-mafft-7.130.sh)
	$(call dist-src,dna-alignx-mafft,dna-alignx-mafft-7.407.sh.os$(SLIB_PLATFORM),dna-alignx-mafft-7.407.sh)

	$(call dist-service-register,dna-alignx-tophat,TopHat splice junction mapper,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-tophat, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-tophat,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-tophat,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-tophat,dna-alignx-tophat-2.1.1.sh.os$(SLIB_PLATFORM),dna-alignx-tophat-2.1.1.sh)

	$(call dist-service-register,dna-alignx-hisat2,HISAT2 alignment,/Sequencing/, \
        maxJobs=4 \
    )
	$(call dist-service-set-config,dna-alignx-hisat2, \
        indexDirectory='@global_scratch_dir@data/' \
    )
	$(call dist-link,dna-alignx-hisat2,dna-alignx.os$(SLIB_PLATFORM))
	$(call dist-src,dna-alignx-hisat2,dna-alignx.sh.os$(SLIB_PLATFORM),dna-alignx.sh)
	$(call dist-src,dna-alignx-hisat2,dna-alignx-hisat2-2.0.5.sh.os$(SLIB_PLATFORM),dna-alignx-hisat2-2.0.5.sh)
	$(call dist-src,dna-alignx-hisat2,dna-alignx-hisat2-2.1.0.sh.os$(SLIB_PLATFORM),dna-alignx-hisat2-2.1.0.sh)

.PHONY: algo_install
algo_install::
	$(call prop-import-object,algo-blast-2.2.30.json)
	$(call prop-import-object,algo-blast-2.9.json)
	$(call prop-import-object,algo-blastx-2.2.30.json)
	$(call prop-import-object,algo-blastx-2.9.json)
	$(call prop-import-object,algo-tblastx-2.2.30.json)
	$(call prop-import-object,algo-tblastx-2.9.json)
	$(call prop-import-object,algo-blat-35x1.json)
	$(call prop-import-object,algo-blat-36.json)
	$(call prop-import-object,algo-bowtie-0.12.8.json)
	$(call prop-import-object,algo-bowtie-1.2.2.json)
	$(call prop-import-object,algo-bowtie2-2.1.0.json)
	$(call prop-import-object,algo-bowtie2-2.3.5.json)
	$(call prop-import-object,algo-bwa-0.7.17.json)
	$(call prop-import-object,algo-clustal-1.2.4.json)
	$(call prop-import-object,algo-hisat2-2.0.5.json)
	$(call prop-import-object,algo-hisat2-2.1.0.json)
	$(call prop-import-object,algo-mafft-7.130.json)
	$(call prop-import-object,algo-mafft-7.407.json)
	$(call prop-import-object,algo-tophat-2.1.1.json)
