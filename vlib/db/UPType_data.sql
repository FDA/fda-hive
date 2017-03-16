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

TRUNCATE `UPType`;

INSERT INTO `UPType` VALUES (1,NULL,0,1,'type','object type','Object type');
INSERT INTO `UPType` VALUES (3,'viodb',1,1,'u-hiveseq','HIVESeq',NULL);
INSERT INTO `UPType` VALUES (7,'base_system_type',0,1,'view','Object type view',NULL);
INSERT INTO `UPType` VALUES (8,'base_system_type',0,1,'action','Object type action',NULL);
INSERT INTO `UPType` VALUES (9,'base_system_type',0,1,'menuitem','web page menu item',NULL);
INSERT INTO `UPType` VALUES (10,'base_system_type',0,1,'webpage','Web Page','Web application entries');
INSERT INTO `UPType` VALUES (13,NULL,1,1,'base','Base type for all types',NULL);
INSERT INTO `UPType` VALUES (14,'base_user_type',0,1,'email','Email Message',NULL);
INSERT INTO `UPType` VALUES (15,'base_user_type',0,1,'notification','message',NULL);
INSERT INTO `UPType` VALUES (16,'base_user_type',1,1,'process','Process details',NULL);
INSERT INTO `UPType` VALUES (17,'svc-align-dnaseq',0,1,'svc-align-hexagon','HIVE-Hexagon','HIVE hexagon alignment engine optimized for HIVE cloud performance on Next Generation data');
INSERT INTO `UPType` VALUES (18,'svc-data-loading-base',0,1,'svc-download','Download service',NULL);
INSERT INTO `UPType` VALUES (19,'base_user_type,file',0,1,'u-file','User File',NULL);
INSERT INTO `UPType` VALUES (22,'u-file',0,1,'viodb','Generic VioDB',NULL);
INSERT INTO `UPType` VALUES (25,'svc-dna',0,1,'svc-align','Sequence Alignment Engines','Sequence Alignment Engines General Service');
INSERT INTO `UPType` VALUES (26,'svc-align-dnaseq',0,1,'svc-align-blast','Blast Alignment','Blast alignment engine optimized for HIVE performance');
INSERT INTO `UPType` VALUES (27,'svc-dna',0,1,'svc-profiler','Sequence Profiling Engine','Engine to perform coverage analysis ,base-calling, SNP calling ');
INSERT INTO `UPType` VALUES (28,'svc-align-dnaseq',0,1,'svc-align-bowtie','BowTie Alignment','BowTie alignment engine optimized for HIVE ');
INSERT INTO `UPType` VALUES (29,'svc-computations-base',0,1,'svc-dna','DNA Services','Services Associates with Sequences and Alignments');
INSERT INTO `UPType` VALUES (35,'svc-computations-base',0,1,'svc-recomb','Recombination Polyplot Computation Engine','Recombination Polyplot Computation Engine');
INSERT INTO `UPType` VALUES (40,'svc-align-rnaseq',0,1,'svc-align-tophat','TopHat aligment','TopHat based workflow');
INSERT INTO `UPType` VALUES (42,'svc-computations-base',0,1,'svc-clust','Hierarchical clustering engine','Hierarchical clustering tool for alignment profiling results');
INSERT INTO `UPType` VALUES (45,'viodb',0,1,'u-annot','Annotation File','Sequence Annotation Object');
INSERT INTO `UPType` VALUES (51,'svc-computations-base',0,1,'svc-hiveseq','Hiveseq Service','Service to manipulate sequences files');
INSERT INTO `UPType` VALUES (54,'svc-data-loading-base',0,1,'svc-archiver','File processing service',NULL);
INSERT INTO `UPType` VALUES (55,'svc-align-dnaseq',0,1,'svc-align-bwa','BWA Service','Burrows Wheeler Aligner');
INSERT INTO `UPType` VALUES (56,'svc-align',0,1,'svc-denovo-velvet','Velvet de novo service','Velvet de novo assembler');
INSERT INTO `UPType` VALUES (58,'svc-align-hexagon',0,1,'svc-align-hexagon-batch','HIVE-Hexagon Batch','HIVE-hexagon batch submitter');
INSERT INTO `UPType` VALUES (59,'svc-align',0,1,'svn-denovo-oases','Oases denovo service','Oases de novo assembler');
INSERT INTO `UPType` VALUES (62,NULL,1,1,'directory','Directory Object','Directory Object for hierarchial organization of objects');
INSERT INTO `UPType` VALUES (63,'base_user_type,directory',0,1,'folder','Folder','Folders to organize objects hierarchically');
INSERT INTO `UPType` VALUES (64,'base_system_type',0,1,'u-usage','User Usage ','Collect User information ');
INSERT INTO `UPType` VALUES (65,'u-file',0,1,'image','Image Object ','Show different kinds of image');
INSERT INTO `UPType` VALUES (66,'base_system_type',0,1,'special','Special Object','Manage internal object');
INSERT INTO `UPType` VALUES (67,'base',1,1,'base_user_type','Base type for all user types','Type from which all visible user types are inherited');
INSERT INTO `UPType` VALUES (68,'svc-computations-base',0,1,'svc-generic-launcher','Launcher Service','Service to launch an external application');
INSERT INTO `UPType` VALUES (69,'svc-align-multiple',0,1,'svc-align-clustal','Clustal multiple alignment',NULL);
INSERT INTO `UPType` VALUES (70,'svc-align-multiple',0,1,'svc-align-mafft','Mafft multiple alignment',NULL);
INSERT INTO `UPType` VALUES (71,'svc-align',0,1,'svc-denovo-abyss','Abyss de novo service','Abyss de novo assembler');
INSERT INTO `UPType` VALUES (72,'u-hiveseq',0,1,'genome','Genomic sequence','Sequence which represents genome of an organism');
INSERT INTO `UPType` VALUES (73,'u-hiveseq',0,1,'nuc-read','Nucleotide read','Nucleotide read');
INSERT INTO `UPType` VALUES (75,'svc-computations-base',0,1,'svc-censuScope','CensuScope: Metagenome Analyzer','Rapid Metagenome Profiler');
INSERT INTO `UPType` VALUES (76,'svc-dna',0,1,'svc-profx','External Sequence Profiling Engine','General external profiler engine');
INSERT INTO `UPType` VALUES (77,'svc-profx',0,1,'svc-profx-samtools','Samtools based SNP caller','Samtools based SNP caller');
INSERT INTO `UPType` VALUES (82,'svc-dna',0,1,'svc-dna-screening','Screen Short Read Against Genome','Random pick sequences to run alignment with specific genome');
INSERT INTO `UPType` VALUES (83,'svc-dna',0,1,'svc-dna-recomer','Recombinant K-mer','K-mer based recombinant analysis tool');
INSERT INTO `UPType` VALUES (84,'base',1,1,'base_system_type','Base type for all system objects','Type from which all system level types are inherited');
INSERT INTO `UPType` VALUES (85,'sysfile',1,1,'algorithm','Algorithm objects','All executable algorithms base object');
INSERT INTO `UPType` VALUES (86,'algorithm',1,1,'algorithm-alignment','Alignment algorihtms','Alignment algorihtms');
INSERT INTO `UPType` VALUES (89,'algorithm-alignment',0,1,'algorithm-alignment-rnaseq','RNA-seq Alignment algorihtms','RNA-seq Alignment algorihtms');
INSERT INTO `UPType` VALUES (90,'svc-align',0,1,'svc-align-dnaseq','DNA-seq Alignments','DNA-seq alignmnent');
INSERT INTO `UPType` VALUES (91,'svc-align',0,1,'svc-align-rnaseq','RNA-seq Alignments','RNA-seq Alignments');
INSERT INTO `UPType` VALUES (92,'algorithm-alignment',0,1,'algorithm-alignment-dnaseq','DNA-seq Alignment Algorithms','DNA-seq Alignment Algorithms');
INSERT INTO `UPType` VALUES (93,'algorithm-alignment',0,1,'algorithm-alignment-multiple','Multiple Alignment Algorithms','Multiple Alignment Algorithms');
INSERT INTO `UPType` VALUES (94,'svc-align',0,1,'svc-align-multiple','Multiple Alignments','Multiple Alignments');
INSERT INTO `UPType` VALUES (96,'svc-generic-launcher',0,1,'svc-dna-hexagon-batch','Dna Hexagon Batch','Launches Dna Hexagon in Batch Mode');
INSERT INTO `UPType` VALUES (98,'base_system_type',0,1,'user-settings','User Settings','Collection of User Setting Parameters');
INSERT INTO `UPType` VALUES (99,'base_system_type,directory',0,1,'sysfolder','System Folder','Folder managed by the system (Inbox etc)');
INSERT INTO `UPType` VALUES (101,'svc-profx',0,1,'svc-profx-cuffdiff','Cuffdiff diferential RNA-Seq','Cuffdiff diferential RNA-Seq analysis tool');
INSERT INTO `UPType` VALUES (111,'base_user_type',0,1,'base_descriptive_type','HIVE Database Information','Reoccuring database fields');
INSERT INTO `UPType` VALUES (113,'svc-align-dnaseq',0,1,'svc-align-bowtie2','BowTie2 Alignment','BowTie version 2 engine optimized for HIVE');
INSERT INTO `UPType` VALUES (115,'svc-computations-base',0,1,'svc-algo-annotMapper','Annotation Mapper','Maps 2 or more annotations to each other');
INSERT INTO `UPType` VALUES (116,'svc-computations-base',0,1,'svc-profiler-refcmp','Sequence Profile Match to References','Compares alignment profiling result to arbitrary reference sequences');
INSERT INTO `UPType` VALUES (121,'svc-computations-base',0,1,'svc-annovar','Annovar Service','This is a front end service for Annovar');
INSERT INTO `UPType` VALUES (122,'base_user_type',0,1,'omics','Set of Omics measurements','Pre-processed microarray, proteomic, or ChIP-chip measurements');
INSERT INTO `UPType` VALUES (123,'svc-computations-base',0,1,'svc-textClustering','Text Clustering','Text Based Clustering Algorithm');
INSERT INTO `UPType` VALUES (124,'svc-align-dnaseq',0,1,'svc-align-blat','Blat Alignment','Blat alignment engine parallellized by HIVE');
INSERT INTO `UPType` VALUES (126,'base_system_type,file',0,1,'sysfile','System File',NULL);
INSERT INTO `UPType` VALUES (127,'sysfile',0,1,'system-image','System Image',NULL);
INSERT INTO `UPType` VALUES (140,NULL,0,1,'xref','External Reference','Reference to records from external sources');
INSERT INTO `UPType` VALUES (152,'svc-align-hexagon',0,1,'svc-cvm-1','CVM Bacterial Analysis','CVM Bacterial Analysis for antibiotic resistant genes.');
INSERT INTO `UPType` VALUES (153,'base_user_type',1,1,'tblqryx4','Table Query X4',NULL);
INSERT INTO `UPType` VALUES (154,'u-file,table',0,1,'excel-file','Microsoft Excel file',NULL);
INSERT INTO `UPType` VALUES (202,'svc-generic-launcher',0,1,'svc-base-shell-script','Shell Script','Generic script to be executed by generic software adapter');
INSERT INTO `UPType` VALUES (204,'algorithm',0,1,'algorithm-script','Algorithmic Script','Adopted Script Driving an Algorithm');
INSERT INTO `UPType` VALUES (205,'svc-profiler',0,1,'svc-profiler-heptagon','Sequence Profiling Engine','Engine to perform coverage analysis ,base-calling, SNP calling ');
INSERT INTO `UPType` VALUES (207,'base_system_type',0,1,'plugin_tblqry','Table Query Plugin','DEfinition of Pluging input parameters and dynamic load lib');
INSERT INTO `UPType` VALUES (210,'base_user_type',0,1,'base_contact_type','Name and Contact Information','About: Name, address, phone, fax, email and affiliation for any individual');
INSERT INTO `UPType` VALUES (211,NULL,0,1,'Ireference','Internal References List','About: Internal references arranged as array BROKEN(cannot modify 141)');
INSERT INTO `UPType` VALUES (215,'base_user_type',0,1,'ionDB','Generic ionDB','Generic ionDB type');
INSERT INTO `UPType` VALUES (216,'ionDB',0,1,'u-ionAnnot','ionAnnot type','Annotation file in ionDB format');
INSERT INTO `UPType` VALUES (218,'svc-generic-launcher',0,1,'svc-dna-velvet','Velvet de Novo','Svc to launch velvet de novo via generic launcher');
INSERT INTO `UPType` VALUES (219,'svc-computations-base',0,1,'svc-dna-pipeline','Dna-Pipeline','Svc to lauch dna-pipeline to string together other services using each one\'s output as the input for the next.');
INSERT INTO `UPType` VALUES (221,'svc-base-shell-script',0,1,'svc-velvet-pipeline','Velvet Pipeline','Pipeline for Velvet');
INSERT INTO `UPType` VALUES (224,'svc-base-shell-script',0,1,'svc-single_velvet-pipeline','Velvet Small Genome Pipeline','Velvet de Novo (Viral/Small Genome)');
INSERT INTO `UPType` VALUES (225,'ionDB',0,1,'u-ionQL','ion QL Script','ion QL Script files to store query procedure');
INSERT INTO `UPType` VALUES (226,'u-file',0,1,'u-idList','id list','list of ids (genes, proteins...)');
INSERT INTO `UPType` VALUES (230,NULL,1,1,'file','File',NULL);
INSERT INTO `UPType` VALUES (231,'svc-generic-launcher',0,1,'svc-r-script','R-services','R based services');
INSERT INTO `UPType` VALUES (232,'algorithm-script',0,1,'algorithm-r-script','Algorithmic-R-script','R-scripted adapted to HIVE');
INSERT INTO `UPType` VALUES (233,'algorithm-r-script',0,1,'algorithm-r-expression-analysis','R- expression Analysis','Tools for expression analysis implemented as R scripts');
INSERT INTO `UPType` VALUES (234,'svc-align-blast',0,1,'svc-align-tblastx','TBlastX Alignment','TBlastX alignment engine optimized for HIVE performance');
INSERT INTO `UPType` VALUES (236,'u-file',0,1,'u-tqs','TQS File Type','TQS File Type');
INSERT INTO `UPType` VALUES (238,'svc-base-shell-script',0,1,'svc-viral-mutation-comp','Viral Mutation Comparator','Regulatory Viral Mutation Comparator');
INSERT INTO `UPType` VALUES (239,'svc-computations-base',0,1,'svc-genome-comparator','Genome Comparator','Algorithm to compare genome similarities');
INSERT INTO `UPType` VALUES (241,'svc-align-blast',0,1,'svc-align-blastx','BlastX Alignment','BlastX alignment engine optimized for HIVE performance');
INSERT INTO `UPType` VALUES (242,'svc-base-shell-script',0,1,'svc-idba-ud','IDBA-ud','IDBA is the basic iterative de Bruijn graph assembler for second-generation sequencing reads. IDBA-UD, an extension of IDBA, is designed to utilize paired-end reads to assemble low-depth regions and use progressive depth on contigs to reduce errors in high-depth regions.');
INSERT INTO `UPType` VALUES (243,'svc-computations-base',0,1,'svc-dna-insilico','Insilico Genome Randomizer','Generate Artificial reads from a Genome');
INSERT INTO `UPType` VALUES (244,'base_user_type',0,1,'core_metadata_type','Core metadata type','All metadata objects must inherit from this');
INSERT INTO `UPType` VALUES (245,'svc-computations-base',0,1,'svc-tnseq','TNSeq Analyzer','Algorithm to detect transposon insertion positions');
INSERT INTO `UPType` VALUES (250,'svc-computations-base',0,1,'svc-alignment-comparator','Alignment Comparator','Alignment Comparator utility');
INSERT INTO `UPType` VALUES (251,'u-file',0,1,'prot-seq','Protein','Protein sequences');
INSERT INTO `UPType` VALUES (252,'base_user_type',0,1,'core_descriptive_type','HIVE Database Information','Reoccuring database fields');
INSERT INTO `UPType` VALUES (253,'u-file',0,0,'target_library','Target/Filter Library','Target.Filter libraries for Pathoscope integration.');
INSERT INTO `UPType` VALUES (254,'svc-base-shell-script',0,1,'svc-computation-test','Computation Test','Computation Test Page');
INSERT INTO `UPType` VALUES (257,'base_system_type',0,1,'dropbox','Data Dropbox','Custom method for uploading data files');
INSERT INTO `UPType` VALUES (260,'svc-base-shell-script',0,1,'svc-pipeline-soap','SOAP Pipeline','Pipeline for SOAP');
INSERT INTO `UPType` VALUES (261,'svc-computations-base',0,0,'svc-pyhive-demo','PyHIVE demo service',NULL);
INSERT INTO `UPType` VALUES (266,'svc-computations-base',0,1,'svc-dna-refClust','Dna Reference Cluster','Dna Reference Cluster');
INSERT INTO `UPType` VALUES (272,'svc-base-shell-script',0,1,'svc-pipeline-sam-to-csv','SAM to CSV','Convert SAM files to CSV files with some formatting');
INSERT INTO `UPType` VALUES (274,'svc-base-shell-script',0,1,'svc-pipeline-HIV-resistance','HIV resistances','Analyzes the nucleotide sequence and determines if there is a resistance to HIV drugs');
INSERT INTO `UPType` VALUES (297,'svc-base-shell-script',0,1,'svc-pipeline-falcon','FALCON Pipeline','Pipeline for FALCON');
INSERT INTO `UPType` VALUES (298,NULL,1,1,'table','Abstract table type','Any comma, tab separated something reconized by Table Query');
INSERT INTO `UPType` VALUES (299,'u-file,table',0,1,'csv-table','Comma separated table file',NULL);
INSERT INTO `UPType` VALUES (300,'u-file,table',0,1,'tsv-table','Tab separated table file',NULL);
INSERT INTO `UPType` VALUES (301,'sysfile',0,1,'multimedia','Multimedia Object','A type defining any multimedia object (poster, publication, video, etc)');
INSERT INTO `UPType` VALUES (302,'base_system_type',0,1,'sysconfig','System configuration','Type describing the singleton system configuration object');
INSERT INTO `UPType` VALUES (303,'base_system_type',0,1,'domain-id-descr','HIVE domain ID specification','Object describing a HIVE domain ID recognized on this HIVE installation');
INSERT INTO `UPType` VALUES (304,'svc-computations-base',0,1,'svc-dna-compress','HIVE Environment Demo','Performs sequence file compression on a sequence file');
INSERT INTO `UPType` VALUES (305,'svc-base-shell-script',0,1,'svc-pipeline-nameconcat','Demo GL Pipeline','Demo GL Pipeline');
INSERT INTO `UPType` VALUES (306,'svc-algo-annotMapper',0,1,'svc-algo-ionAnnotMapper','Ion Annotation Mapper','Annotation Mapper between annotation in iondb and/or profilers');
INSERT INTO `UPType` VALUES (307,'svc-base-shell-script',0,1,'svc-velvet-pipeline-paired','Velvet Pipeline (Paired End Reads)','Pipeline for Velvet using paired end reads');
INSERT INTO `UPType` VALUES (309,'svc-computations-base',0,1,'svc-affinity-viz-peak-detect','Promiscuity Peak Detection','Promiscuity Peak Detection for the Affinity Visualization project');
INSERT INTO `UPType` VALUES (310,'svc-computations-base',0,1,'svc-algo-diffSat','Differential Saturation','Differential Saturation');
INSERT INTO `UPType` VALUES (311,'svc-computations-base',0,1,'svc-multi-download','Multiple Results Download','Download results from multiple different processes');
INSERT INTO `UPType` VALUES (313,'svc-computations-base',0,0,'svc-ionseq','ionseq','ionseq project');
INSERT INTO `UPType` VALUES (314,'svc-dna',0,0,'svc-dna-codonQC','Dna Codon QC','Dna Codon QC');
INSERT INTO `UPType` VALUES (315,'svc-dna',0,0,'svc-dna-multi-qc','Dna Multi QC','Dna multi file QC');
INSERT INTO `UPType` VALUES (316,'svc-computations-base',0,0,'svc-renamer','Renamer','Renaming interface');
INSERT INTO `UPType` VALUES (317,'svc-computations-base',0,0,'svc-affinity-viz-simulated-annealing','Simulated Annealing','Simulated Annealing for the Affinity Visualization project');
INSERT INTO `UPType` VALUES (318,'svc-dna',0,0,'svc-dna-alignQC','Dna Alignment QC','Dna Alignment QC');
INSERT INTO `UPType` VALUES (320,'svc-computations-base',0,0,'svc-agn-classifier','Agnostic Classifier','Agnostic Classifier');
INSERT INTO `UPType` VALUES (325,'svc-computations-base',0,0,'svc-dna-differential-profiler','Differential Profiler','Find differences among different Profiling outputs');
INSERT INTO `UPType` VALUES (326,'svc-dna',0,0,'svc-dna-kmerQC','Dna Kmer QC','Dna Kmer QC');
INSERT INTO `UPType` VALUES (327,'base',0,0,'test-encoding','Field Encoding & Encryption Demo','Demo for field value encoded and encrypted storage');
INSERT INTO `UPType` VALUES (328,'svc-base-shell-script',0,1,'svc-pipeline-guinier-analysis','Matlab Based Guinier Analysis','Matlab Based Guinier Analysis');
INSERT INTO `UPType` VALUES (329,'svc-data-loading-base',0,1,'svc-compressor','Data Compression Engine',NULL);
INSERT INTO `UPType` VALUES (330,'process',1,1,'svc-data-loading-base','data loading','Base type for data loading backends');
INSERT INTO `UPType` VALUES (331,'process',1,1,'svc-computations-base','computations','Base type for all computations');
