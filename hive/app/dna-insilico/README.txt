HIVE Insilico version 0.5

Contributors:  Luis V. Santana and Vahan Simonyan
Contact:  
Luis V. Santana -> luis.santana-quintero@fda.hhs.gov
Vahan Simonyan -> vahansim@gmail.com

========================================================================
Table of Contents:

  1. Definitions
    1.1 Objective
    1.2 Description
  2. Installation
    2.1 Requirements
    2.2 Downloads
      2.2.1 Content
  3. Command Reference
    3.1 vParse
    3.2 ion
    3.3 vInsilico
      3.3.1 outformat
      3.3.2 showId
      3.3.3 numReads
      3.3.4 minLen
      3.3.5 maxLen
      3.3.6 strand
      3.3.7 pairedEnd
      3.3.8 pEndmin
      3.3.9 pEndmax
      3.3.10 quaMin
      3.3.11 quaMax
      3.3.12 lowCompEntropy
      3.3.13 lowCompWindow
      3.3.14 filterN
      3.3.15 noisePerc
      3.3.16 randMutations
      3.3.17 mutFile
      3.3.18 annotFile
  4. Usage Examples
    4.1 Generate example files to use as a source.
    4.2 How to Compile a FastA/FastQ file
    4.3 How to Compile a GTF file
    4.4 How to generate short reads with insilico tool
      4.4.1. Set of commands to generate random reads
      4.4.2. Set of commands to add mutations to the resultant files
      4.4.3. Set of commands to use the annotation file for targeting sequencing76
      4.4.4. Creating short reads and using output filters

========================================================================
1. Definitions

  1.1 Objective:

    HIVE Insilico contains a set of tools for generating insilico reads, parse fastA files and annotation files in GTF format.

  1.2 Description:

    The HIVE Insilico tool generates synthetic reads to be used in testing and validation pipelines for various NGS platforms and appliances. The user can input genomes of reference organisms, along with specifications for noise, quality, and other attributes, and will return properly formatted in silico data in accordance with the user's requests. 


========================================================================
2. Installation

  2.1 Requirements:
    - Linux (redhat)
    - x86_64 processor

  2.2 Downloads:

    https://hive.biochemistry.gwu.edu/apps/hive_insilico.zip

    2.2.1 Content:

========================================================================
3. Command Reference

  3.1 vParse
    This command is used to validate the fastA file format into a binary file that is used by HIVE insilico to generate short reads, the basic use of it is:

  $hive_insilico -vParse namefile.fasta

    NOTE: output is a set of binary files with "vioseq2" extension being the main file

  3.2 ion
    This command is necessary to validate the GTF file that HIVE insilico needs to generate short reads in between the ranges specified by the GTF file, example of it is:


  $hive_ion -ionCreate ion-out -gtfParse namefile.gtf 

    NOTE: output is a set of binary files with "ion" extension being the main file


  3.3 vInsilico
    This command is used to generate short reads from the "vioseq2" file (output of vParse command).  the basic use of it is:

  $hive_insilico -insilico "param1=val1&param2=val2&param3=val3" -vInsilico infile.vioseq2 outfilePrefix

    NOTE: output is a set of files with "fastA/fastQ" extension containing the resultant files in the specific format.

    Description of all the parameters is described next:

    3.3.1  
      outformat (DEFAULT 0)= "0" or "fasta" means FASTA format
                           = "1" or "fastq" or "fq" means FASTQ format

    3.3.2  
      showId (DEFAULT 0) = 0 shows sequential numbers only
           = 1 shows information of position and mutations

    3.3.3
      numReads (1000) = value number of short reads

    3.3.4
      minLen (100) = value minimum Length of short reads

    3.3.5
      maxLen (100) = value maximum Length of short reads

    3.3.6
      strand (2) = 0 print information on Forward only
                 = 1 print information on Reverse only
                 = 2 print information on Forward and Reverse (randomly)

    3.3.7
      pairedEnd (0) = 0 produce a single file
                    = 1 produce paired end files

    3.3.8
      pEndmin (100) = value minimum length for the paired end library

    3.3.9
      pEndmax (100) = value maximum length for the paired end library

    3.3.10
      quaMin (25) = value minimum value of Phred Score use to generate FastQ reads

    3.3.11
      quaMax (40) = value maximum value of Phred Score use to generate FastQ reads

    3.3.12
      Allows exclusion of any regions of the randomly generated reads or recombinants that are considered to be of low complexity according to their computed Shannon entropy.
      lowCompEntropy (0) = realvalue "1.0" lenient or "1.2" reluctant or "1.6" strict 

    3.3.13
      lowCompWindow (0) = realvalue "15" or "25" or "32" output low complexity filter

    3.3.14
      filterN (-1) = "-1" do not filter N's from the output reads
                   = "0" do not tolerate any N's from the output reads
                   = "100" tolerate reads with up to 99 percent of N's from the output reads

    3.3.15
      noisePerc (0) = "1" apply 1 percent of random white noise to the reads
                "5" apply 5 percent of random white noise to the reads

    3.3.16
      randMutations (0) = "10" generate 10 random mutations

    3.3.17
      mutFile (0) = "namefilepath" path and filename of the mutation file to extract mutations from.

     The mutation file should be in either "csv" or "vcf" format with the columns as described next:
        - for the csv file header:
          chrom, Position start, position end, reference, mutation, frequency, quality, zygosity, note
          2, 5425, 5425, A, C, -1, -1, heterozygous, notes 1
          chr7, 5426, 5427, AC, CA, 89, -1, homozygous, notes 2


        - for the vcf file header:
          chrom, position, ID, REF, ALT, QUAL, FILTER, INFO, FORMAT, NA12878
          2 6525  . T G 6586  PASS  info  format  extras
          7 65325 . A C 4620  PASS  info  format  extras

    3.3.18
      annotFile (0) = "ionannotfilepath" path and filename of the annotation file to generate sequences in those ranges specified by the annotFile

      The file needs to be precompiled using "hive_ion" eg.
        $hive_ion -ionCreate ion-out -gtfParse file.gtf

========================================================================
4. Usage Examples
  4.1 Generate example files to use as a source.
    To test HIVE Insilico, you can create simple files with the next information:

    4.1.1 testfile.fasta:
>chr1
AGAAGGAAAACGGGAAACTTCACAATTAGTGAATATTTAAAAACAGACTCTTAAGAAACCAAAGGATCAAGGAAGATACCACAGGGAAAAATAGAGAATATCTCAAGACAAATGAAAACAAAAACGCAACATACCAAAACTTATAGGATGCAATGAGAGCAGTATTAAGAGGGAAATTCATAGAGGTGAAAACTTACATTTAAAAAGAAGATGGATCTCAAATAAACAACCTAACTTTACATCTCAAGGAACTAGAAAAAGAAGAACAAGTTAAACATGAATTAGCAGAAAGAAGGAAATGGTAATGATTAGAACAGAGATAAACACAATAATAGAAAACAATAGAAAAATCAACAAACTGAAGAGCTGGATTTTTGAAAAGATCAACAAAATTAACAGAAACTCTTAGCTAGATTAGCTAAGAAAAAAAGAGGGAAGACTCAATTAAATCAGAAATGAAAGAGGCCCCTTACAACTGATGCCACATAAATAAAAAATATTGTAAGAGAATGTCATGAACAATGGCTATATACCAACAAATTGGGTAATCTGGAAGAAATTGAAAAATTCCTAGAAATATACAACCTACCAAGATTGAATCATGAACAAATAATTATCTGAAAAGACCTATAACTAGTAAAAGATTGAATTAGTCATCAAAAATCTCCCAAAAAAGAAAAGCCCAGGACCAGATGGCTTTACCGGAGAATTCTACCAAGGATTTAAATAATTAACAGCAATCCTCCTCACATTCTTCTGAAAAGCTAAACAAGAGGAACACTTCCAACCTCAATGTATAAGGCCAGCATTATCCTGATACCAAGCCCAGACAAGAAAGCTACAGGAAAAGAAAACTACAGACCGATTTTCCCGATAACTGCTGATGCCAAATCCCCAACAAAATACTAGCAAACTGTATTCCGTAGCACATTAAAGGATTATACTCCCTGACCAAGTGGGATTTACTCCTGGAATGGAAGGATGGCTCAAAATATGAACA
>chr2
TCTTGACACTGATTGATCTGCCAAAAGGGGAAGAATGAGTCCAGCTAGAATCCAGGACTAACCAGCGGGTGAGCTTCAAGGAACAAAGGGCTTCCGCTGGGTCAGCCCACGAGAGGGAGCTGCCTGCAGGTACCTGGGAGGGCACAGCCACCGTGTCTGTTCCTTGGGAGCAGGGCTCCTTGGAAGGCAGCGCCAGCTCCAGAAAGGCCACTGTTCCCCCTCCCACCCACCCCAGGTGGCAACTGCCCCTGCAGGTCACGGTCCCAGGGCCTCGCTCGGCCGGCCTAGAGAGAGAAAAGGGAAGATGCCCAATTAGCTGGACGCCCATGGCCCCAGGGCTGGGGGACCTTGGGTTATCCTGAAAGAAGGACAATGACCACCTGCAAAGGACGATTTCAAGTGGAGCCTGGAGGAGGCGGCAAATAGCTCAGTCCGCACTCAACTCCCTTCCCAGCCGTCTGGTGGAAGGTGAGGAGCAAAAGCTCATGCTGCTGAGGCCTGGGGCCACCCAGGCACCGGCCTCCCCGTGCTGAGCAGAGACAGCAGCCCCCTAGCTGCAGGAGTGTGCCCCGATGCGCACCTGCCGACTTTCCTTCTGATGCAGACATGGTCCCCACTGGGGCAAGGCTGCAGTTTTCTTTTAAAAAACAGATCTTTCAAAATAAAAGGGGCCTTGAAGAAAATCGTGGAATAAATAACAGTCTGGGGTGGAGGCGGTTCTGGCGAAAGTGTCCTGAGGGTGGTGTGTGATGGACGGGACTTTGGGCAGTGCTGATGGGGCCTTGCCTGTCACTTGAGGCCTCCAGGAGAGTCGAGGACTGGCAGATCCAGGTCCAGAGCAGGTGCAGGCTCCCAGCTCTTCCAGCCCCTTACCTCGCAGTAGGTTCCCACAGTTCTAGCCTCCTTGATGCCAGCCCCAGGGTGCCCACTGCTGGCCAACTATGTCCTTTTCCTCAAAACCAGGTCCCTCGGTGCCCAGGGTGAATGGGTATATGAGCCT

    4.1.2 testmutation.csv:
Chromosome,Position Start,Position End,Reference,Mutation,frequency,quality,Zygosity,Note
chr1,110,110,C,T,-1,-1,Heterozygous,C>T
chr2,171,171,G,A,-1,-1,Heterozygous,G>A
chr2,885,885,G,-,-1,-1,Heterozygous,DELG>T
chr1,569,570,TA,GC,80,-1,Homozygous,TA>GC


    4.1.3 testmutation.vcf (columns are TAB separated):
##fileformat=VCFv4.1
#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO	FORMAT	NA12878
1	511	.	A	G	34	PASS	Info	format	1/1:150:250
1	892	.	T	C	34	PASS	Info	format	1/1:150:250
2	11	.	C	G	34	PASS	Info	format	1/1:150:250
2	452	.	G	A	34	PASS	Info	format	1/1:150:250
2	921	.	A	T	34	PASS	Info	format	1/1:150:250

    4.1.4 testannotation.gtf (columns are TAB separated):
1       Human   Sure5    655   956   .       +       .       -
2       Human   Sure5    65   500   .       +       .       -
2       Human   Sure5    600   995   .       +       .       -


========================================================================

  4.2 How to Compile a FastA/FastQ file

    Before starting using HIVE_insilico tool, we need to validate the input genome file, for that we will type:

    $hive_insilico -vParse testfile.fasta

    This generates a set of files with "vioseq2" extension, containing a binary validated version of the fasta file.


  4.3 How to Compile a GTF file

    First you need to parse the annotation file with the next command:

        $hive_ion -ionCreate ionOutfile -gtfParse testannotation.gtf



  4.4 How to generate short reads with insilico tool
    4.4.1. Set of commands to generate random reads:

      4.4.1.1 If you need to generate 1000 short reads of length=150 bases from the genome: 

    $hive_insilico -insilico "numReads=1000&minLen=150&maxLen=150" -vInsilico testfile.vioseq2 outfile


      4.4.1.1 If you need to show Id information to the reads: 

    $hive_insilico -insilico "numReads=1000&minLen=150&maxLen=150&showId=yes" -vInsilico testfile.vioseq2 outfile


      4.4.1.2. If you need to change the output format from fastA to fastQ format:

    $hive_insilico -insilico "numReads=1000&minLen=150&maxLen=150&showId=yes&outformat=fastq" -vInsilico testfile.vioseq2 outfile


      4.4.1.3. If you need to generate paired end files from previous experiment:

    $hive_insilico -insilico "numReads=1000&minLen=150&maxLen=150&showId=yes&outformat=fastq&pairedEnd=yes" -vInsilico testfile.vioseq2 outfile


      4.4.1.4. If you need to change the library parameters to (200, 400) of the paired end files:

    $hive_insilico -insilico "numReads=1000&minLen=150&maxLen=150&showId=yes&outformat=fastq&pairedEnd=yes&pEndmin=200&pEndmax=400" -vInsilico testfile.vioseq2 outfile

      4.4.1.5. If you need to add a 1 percent noise to the short reads:

    $hive_insilico -insilico "numReads=1000&minLen=150&maxLen=150&showId=yes&outformat=fastq&pairedEnd=yes&pEndmin=200&pEndmax=400&noisePerc=1" -vInsilico testfile.vioseq2 outfile


    4.4.2. Set of commands to add mutations to the resultant files:

      4.4.2.1 If you need to generate 1000 short reads of variable length between 100 and 150 bases in fastA format from the genome: 

    $hive_insilico -insilico "numReads=1000&minLen=100&maxLen=150" -vInsilico testfile.vioseq2 outfile


      4.4.2.2 If you need to add 1 thousand random mutations to the short reads:

    $hive_insilico -insilico "numReads=1000&minLen=100&maxLen=150&randMutations=1000" -vInsilico testfile.vioseq2 outfile


      4.4.2.3 If you want to use the csv file to dictate the variant calling information:

    $hive_insilico -insilico "numReads=1000&minLen=100&maxLen=150&mutFile=testmutation.csv" -vInsilico testfile.vioseq2 outfile


      4.4.2.4 If you want to use the vcf file instead of the csv to dictate the variant calling information:

    $hive_insilico -insilico "numReads=1000&minLen=100&maxLen=150&mutFile=testmutation.vcf" -vInsilico testfile.vioseq2 outfile


      4.4.2.5 If you want to use the vcf file and add 1000 random mutations, you can do it with:

    $hive_insilico -insilico "numReads=1000&minLen=100&maxLen=150&mutFile=testmutation.vcf&randMutations=1000" -vInsilico testfile.vioseq2 outfile


    4.4.3. Set of commands to use the annotation file for targeting sequencing:

      4.4.3.1 First you need to parse the annotation file with the next command:

        $hive_ion -ionCreate ionOutfile -gtfParse file.gtf

      4.4.3.2 If you want to generate 1 million short reads of length = 200 inside the ranges specified by the annotation file:

        $hive_insilico -insilico "numReads=1000&minLen=200&maxLen=200&annotFile=ionOutfile" -vInsilico testfile.vioseq2 outfile

      4.4.3.3 If you want to add 1 thousand random mutations:

        $hive_insilico -insilico "numReads=1000&minLen=200&maxLen=200&annotFile=ionOutfile&randMutations=1000" -vInsilico testfile.vioseq2 outfile


    4.4.4. Creating short reads and using output filters

      4.4.4.1  If you want to generate 1000 short reads, but prevent them from being reads with low complexity regions, eg. entropy of 1.2 and a window size of 30:

    $hive_insilico -insilico "numReads=1000&lowCompEntropy=1.2&lowCompWindow=30" -vInsilico testfile.vioseq2 outfile

      4.4.4.2  If you want to prevent the short reads to contain bases with letter 'N's:

    $hive_insilico -insilico "numReads=1000&lowCompEntropy=1.2&lowCompWindow=30&filterN=0" -vInsilico testfile.vioseq2 outfile
