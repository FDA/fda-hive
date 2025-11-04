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
import React, { Component } from "react";
import "antd/dist/antd.css";
import "@ag-grid-community/all-modules/dist/styles/ag-grid.css";
import "@ag-grid-community/all-modules/dist/styles/ag-theme-balham.css";
import controllers from '../hivelib/controller/controller_collector';
import * as UrlModal from '../hivelib/modal/url_modal';

export class Changelog extends Component {
  UrlPrefix = UrlModal.getPrefix();
  UrlPrefixPlain = UrlModal.getPrefixPlain();

  constructor(props){
    super(props)
    let { CookieConstructor } = controllers
    let cookie = new CookieConstructor()
    this.username = cookie.getCookie("user_name");
    let footer_config = {data: {"data-theme": 'light'}}
    sessionStorage.setItem('footer' , JSON.stringify(footer_config))
    this.state = {
      reloadExplorerCounter: 0, // increment this when we need to reload the FileExplorer, i.e. on project change
      isActive: false,
      maxHeight: "0px"
    };
  }

  toggleNoteBox = (event) => {
    event.target.classList.toggle("active");

    if (event.target.nextElementSibling.style.maxHeight !== '100%')
        event.target.nextElementSibling.style.maxHeight = '100%';
    else
        event.target.nextElementSibling.style.maxHeight = '0px';
  };

  render(){
    return (
    <>
        <title>Changelog</title>

        <div className="note-box collapsible" >
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title active"
                onClick={(event) => this.toggleNoteBox(event)}
            > HIVE Release 23.05
            </h4>
            <div className="category-sections columns note-box_content"
                style={{maxHeight: '100%'}}
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>samtools-fastq v1.17</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>FastP: output file name format</li>
                    <li>Attachment handling for dockets</li>
                    <li>Change Email Notification Setting's Process Default</li>
                    <li>HISAT2 added safe mode to process big files</li>
                    <li>Improve progress update</li>
                    <li>Increased password length requirement to 15 characters</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>RNASEq pipeline fails due to very long names</li>
                    <li>SRA Fastq Downloader</li>
                    <li>HIVE Network did not work with Project correctly</li>
                    <li>Generic launcher fails under heavy load</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            > HIVE Release 23.03
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>Tool to export reads in .fastq.gz format</li>
                    <li>RNASeq pipeline</li>
                    <li>DNASeq pipeline</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>AlphaFold added link to external documentation</li>
                    <li>Validate downloaded file size and display warnings</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Hexagon/Heptagon sometimes not finishing last step</li>
                    <li>Security issue in response with data blob</li>
                    <li>Security fixes</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 23.02
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>Features:</p>
                    <li>Updated Alphafold link to version information</li>
                    <li>Update token management interface</li>
                    <li>New Home added menus to access external downloading tools</li>
                    <li>System will warn if externally downloaded data is of a wrong size vs reported</li>
                    <li>Updated project information display on top menu bar</li>
                    <li>An antivirus integration for system internal use</li>
                    <li>Added option "quiet" in hisat2 integration</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.12
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>Features:</p>
                    <li>Surecalltrimmer 2.6</li>
                    <li>HIVE Projects Administration</li>
                    <li>Function to Retrieve Hit Table from Hexagon</li>
                    <li>Email notifications on computations finish</li>
                    <li>Debug flag in Advanced tab for S3 downloader</li>
                    <li>S3 downloader improved/extended logging</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Multiple security fixes</li>
                    <li>Hexagon downloads</li>
                    <li>SSO for users coming from CBER Connect</li>
                    <li>Project support</li>
                    <li>Broken links in New Home page "Download All" component</li>
                    <li>Progress Error Log shows system messages</li>
                    <li>AMR pipeline</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.11
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>Features:</p>
                    <li>Add general file support to AMR Long Reads Pipeline</li>
                    <li>HIVE AMR: Hybrid nucleotide pipeline</li>
                    <li>RNA-SeQC: update to support BAM file as an input u-file</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Generic launcher failure</li>
                    <li>Hierarchical clustering inputs</li>
                    <li>HexaHedron produces different results for computations</li>
                    <li>Project support</li>
                    <li>Convert to Genome as fasta issue</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.09
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>Features:</p>
                    <li>install/makefile support for installation location (site)</li>
                    <li>CBER Connect linking</li>
                    <li>DCGT JSON Conversion Tool</li>
                    <li>projectID for pipeline children</li>
                    <li>Inherit submission_project in pipeline steps</li>
                    <li>Recordviewer support for type field roles</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Pipeline steps objects not in destination folder</li>
                    <li>AMR Short Reads pipeline - CARD fields are empty</li>
                    <li>Pipeline within pipeline integration issue</li>
                    <li>Uploading automatic processing logic error</li>
                    <li>AlphaFold file input error</li>
                    <li>User-setting type invalid defaults</li>
                    <li>Dockets page not showing input files for specific folders</li>
                    <li>SRA Pipelines lack automatic processing option</li>
                    <li>Convert from file to reads does not work</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.08
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>Features:</p>
                    <li>Add pid tracking to generic launcher</li>
                    <li>Batch mode of AlphaFold v2</li>
                    <li>Replace Error with Warning if field already exists in the type</li>
                    <li>Range header support on file download</li>
                    <li>Tutorial videos for JupyterLab and AlphaFold</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Unknown parent in types</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.07
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>HIVE AMR Long read (PacBio/ONT) nucleotide pipeline</li>
                    <li>Magic-BLAST v1.5.0</li>
                    <li>RNA-SeQC v2.4.2</li>
                    <li>STAR v2.7.10a</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>AWS S3: Download without credentials</li>
                    <li>AWS S3: quick access links from Upload page </li>
                    <li>dna-panel: Convert input csv files to hivepack</li>
                    <li>JSmol viewer for pdb files</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Alphafold db type fix object name (_brief)</li>
                    <li>Hexagon: Incorrect alignment if the title of the reference sequence is wrapped in quotes</li>
                    <li>Pipelines upload _.progress.json files</li>
                    <li>Multiple NGS QC: incorrect behavior if input reads does not have quality control data</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.06
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>Unicyler v0.4.8</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>AMR pipeline: add HIVE curated CARD library index file as an input parameter</li>
                    <li>AMR pipeline: prepopulate CARD input parameters</li>
                    <li>AMR pipeline: add presets, move pipeline parameters to Advanced tab</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>New uploader is accessible without logging in</li>
                    <li>Download Pipelines: error in the developer console</li>
                    <li>Bad characters in JSON response</li>
                    <li>Docket Processing fails on ZIP files without any HTML file</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.05
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>HIVE Docket Pipeline</li>
                    <li>FDMS Downloader v2.26.11</li>
                    <li>QUAST v5.0.2</li>
                    <li>QUAST-LG v5.0.2</li>
                    <li>MetaQuast v5.0.2</li>
                    <li>MarkDuplicates (Picard) v2.26.11</li>
                    <li>CollectHsMetrics (Picard) v2.26.11</li>
                    <li>CollectInsertSizeMetrics (Picard) v2.26.11</li>
                    <li>CollectRnaSeqMetrics (Picard) v2.26.11</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Generic Launcher: The 'Done' status is set before copying output files</li>
                    <li>Files with "_." in filename are filered out from output results </li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 22.04
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>HIVE AMR Pipeline for short reads</li>
                    <li>10X Genomix: cellranger-arc v2.0.1</li>
                    <li>Resistance Gene Identifier (RGI) v5.2.1</li>
                    <li>Unicycler v0.5.0</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Heptagon: shows wrong stats and wrong numbers</li>
                    <li>Project List 'project completion date' absent when project 'status' is finished</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>Convert files to reads: disable screening</li>
                    <li>Do not show Failed computations as inputs</li>
                    <li>HIVE New Home: preview tabs for PDF and text files</li>
                    <li>HIVE-RNA-seq: accept general files as input of read type</li>
                    <li>New HIVE networks (former dropbox) page</li>
                    <li>Output Percent Identity for Hexagon, Blast (nucleotide)</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
                >HIVE Release 22.03
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New Tools:</p>
                    <li>SnpEFF v5.0</li>
                    <li>AlphaFold v2.1.2</li>
                    <li>AWS S3 download Tool</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>HTTP Downloader: Error reported on Data Archivation</li>
                    <li>Alignment Parser: Corrupted quality data</li>
                    <li>HIVE Hexagon: Incorrect fileDate format in VCF files</li>
                    <li>HIVE Hexagon: Exported SAM file is not recognized by FeatureCounts</li>
                    <li>Canu: fails with PacBio reads</li>
                    <li>HIVE Heptagon: Archive function for SNP profile fails</li>
                    <li>HIVE Heptagon: Invalid numbers in the percent column</li>
                    <li>Flat List is not showing</li>
                    <li>Generic launcher: stalling on large apps</li>
                    <li>Project Editor: Status filter</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>Token management (new Home page)</li>
                    <li>Token based registration for the HIVE python API (New Home page)</li>
                    <li>Command Line Interface (CLI) to Python module</li>
                    <li>Docket Processing: Show all csv download files</li>
                    <li>SRA Fastq downloader improvement</li>
                    <li>HIVE Hexagon: export aligned and unaligned reads in sam format with paired end info</li>
                    <li>HIVE Heptagon: interface improvement</li>
                    <li>MultiQC v1.11: Options to separate reports for multiple inputs</li>
                    <li>Added CAR-T menuitem to HIVE Portal</li>
                    <li>New Home: Interface improvements</li>
                    <li>New Uploader: functionality improvements</li>
                    <li>Dragen: Usability improvement to Select Reference Index dialogue</li>
                    <li>Performance improvement: group requests logging</li>
                    <li>Project Editor: Add column 'computation date' to project list view</li>
                    <li>Project Editor: Add ObjId to info popup and a choice of last month to dropdown</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 2.6
            </h4>
            <div className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New tools:</p>
                    <li>10x Genomix Cell Ranger mkfastq</li>
                    <li>10x Genomix Cell Ranger mkref</li>
                    <li>PyIR</li>
                    <li>docket</li>
                    <li>minimap2</li>
                    <li>Kallisto</li>
                    <li>Trimmomatic</li>
                    <li>ANARCI</li>
                </ul>
                <ul className="category-section"><p>Bug fixes:</p>
                    <li>Cell Ranger errors are not displayed if computations fails</li>
                    <li>Cell Ranger computations are killed after 4hours</li>
                    <li>NCBI SRA download issues</li>
                    <li>Progress View: incorrect drawing</li>
                    <li>Uploader backend file size/completeness validation</li>
                    <li>Spades Assembler fails on HIV reads</li>
                    <li>Reprocess a BAM file fails to create a request</li>
                    <li>Codon-Usage Table crashing on a specific RefSeq file</li>
                    <li>Problems with SAM download from Hexagon</li>
                    <li>New Home page issues</li>
                    <li>New Uploader Issues</li>
                    <li>HIVE "news" not visible unless logged in</li>
                    <li>Error when severity or log is missing in one out of many computation's request IDs</li>
                    <li>Prefetch fails on file sizes greater than 20G</li>
                    <li>DNA Parser: Splitting of 0 size file fails</li>
                    <li>Filename of multiple alignment consensus has incorrect extension</li>
                </ul>
                <ul className="category-section"><p>Features:</p>
                    <li>Customizable header/footer for Home page</li>
                    <li>HISAT2 - Summary file flag ON (by default)</li>
                    <li>Allow users see all objects recursively under a directory</li>
                    <li>HTML viewer</li>
                    <li>stderr/stdout handling for integrated tools</li>
                    <li>Batch mode for FastP</li>
                    <li>SRA fasterq-dump update to 2.11.2</li>
                    <li>COVID Spike Conservation Tool: modify to work on larger datasets</li>
                    <li>Update memory options passed in cellranger based on capacity</li>
                    <li>DRAGEN:
                        <i>Enable-sort parameter</i>
                        <i>Add gVCF Genotyper Options</i>
                    </li>
                </ul>
                <ul className="category-section"><p>Updates:</p>
                    <li>SRA fasterq-dump v2.11.2</li>
                    <li>MultiQC v1.11</li>
                    <li>Hisat2 v2.2.1</li>
                    <li>FastP: full integration</li>
                </ul>
            </div>
            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 2.5
            </h4>
            <div
                className="category-sections columns note-box_content"
                data-collapsible="content"
            >
                <ul className="category-section"><p>New tools:</p>
                    <li>DRAGEN v3.6 optimized pipelines:
                        <i>Reference Indexing</i>
                        <i>DNA pipeline (somatic and germline mutations CNV calling)</i>
                    </li>
                    <li>Agilent Surecall Trimmer</li>
                    <li>RNA-Seq Express pipeline</li>
                    <li>dNCBI fasterq-dump</li>
                    <li>NCBI SAM-dump</li>
                    <li>DeepVariant</li>
                    <li>FastQC</li>
                    <li>SARS-CoV-2 Spike - Conservation Tool</li>
                </ul>
                <ul className="category-section"><p>Bug fixes:</p>
                    <li>Pipeline issues fixed</li>
                    <li>bcl2fastq</li>
                    <li>Home page fixes</li>
                    <li>BAM handling</li>
                    <li>SPAdes</li>
                    <li>DNA Differential Profiler</li>
                    <li>SVG graphs on computation pages</li>
                    <li>VDJ graphs</li>
                </ul>

                <ul className="category-section"><p>Features:</p>
                    <li>Display object ids in selections</li>
                    <li>Concatenate reads option in hive-rna-seq-pipeline</li>
                    <li>MultiQC additional report in HIVE RNA-seq pipeline</li>
                    <li>Display bcl2fastq conversion stats</li>
                    <li>HIVESEQ Identification deduplication:
                        <i>fasta/q reads deduplication based on ID</i>
                    </li>
                    <li>Upload now can be associated with a project</li>
                    <li>Presentation:
                        <i>Creating a Predicting Model for Immune Related Adverse Events Using Germline Mutations and Machine Learning</i>
                    </li>
                    <li>SRA toolkit 2.10.7</li>
                </ul>
            </div>

            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 2.4</h4>
            <div className="category-sections columns note-box_content" data-collapsible="content">
                    <ul className="category-section"><p>New Tools:</p>
                        <li>HumanN 2.8.1</li>
                        <li>MetaPhlAn 2.7.7</li>
                        <li>Lumpy 0.2.14</li>
                        <li>bcl2fastq 2.20</li>
                        <li>UMI Tools 1.0</li>
                        <li>FastP 0.20</li>
                    </ul>
                    <ul className="category-section"><p>New Features/Improvements:</p>
                        <li>Updates and additions to Portal</li>
                        <li>New Taxonomy and NT databases</li>
                        <li>Projects selection support for computation launch</li>
                        <li>DI-profiler renamed to Defective Viral Genomes Detection</li>
                        <li>GeneMark tool removed</li>
                        <li>Project management portal updates and reports</li>

                        <li>BCO editor
                            <i>New page with summary view</i>
                        </li>
                        <li>HIVESEQ
                            <i>filter N's</i>
                            <i>run in batch mode</i>
                        </li>
                        <li>Heptagon
                            <i>download consensus without stop codons </i>
                        </li>
                    </ul>
                    <ul className="category-section"><p>Tools Upgrade</p>
                        <li>Blast Tools 2.9</li>
                        <li>Blat 36</li>
                        <li>Bowtie 1.2.2</li>
                        <li>Bowtie 2.3.5</li>
                        <li>Hisat2 2.1.0</li>
                        <li>Mafft 7.407</li>
                        <li>Tophat 2.1.1</li>
                        <li>Samtools 1.9</li>
                        <li>Canu 1.6</li>
                        <li>Pathoscope 2.0.7</li>
                        <li>PRICE 1.2.1</li>
                        <li>Velvet 1.2.10</li>
                    </ul>
                    <ul className="category-section"><p>Bug Fixes:</p>
                        <li>Update to Adverse Event Deduplication tool</li>
                        <li>Dropbox
                            <i>submit button doesn't work</i>
                        </li>
                        <li>DESeq2
                            <i>    Typo in deseq script file</i></li>
                        <li>Canu
                        <i>    Canu Assembler fails on Nanopore and Illumina Reads</i>
                        <i>    Canu incorrectly requires both PacBio and Nanopore reads</i>
                        <i>    Error message to use mico environment</i></li>
                        <li>HIVEPack
                        <i>    Export: hivepack is not downloadable after creation</i>
                        <i>    Exporting hivepack may produce invalid filename</i></li>
                        <li>Hexagon
                        <i>    position search box on alignments window is not searching position</i></li>
                        <li>Heptagon
                        <i>    profiler doesn't display data correctly when the start position is within the first 200 bases</i>
                        <i>profiler graph position controls draw incorrect profiler graph </i>
                        <i>When there is no coverage detected (or passed the filters) it's unclear if the process is done</i>
                        <i>downloading results in vcf format is not taking the proper value from the user input, but it uses the default value</i></li>
                        <li>Sequence Hierarchical Clustering
                            <i>computation is done, but no results are shown  </i></li>
                        <li>HIV drug resistance Panel analysis
                            <i>invalid property name</i></li>
                        <li>MultiQC
                            <i>summary table not responsive</i></li>
                        <li>TNSeq
                            <i>fails to calculate gene and annotation information for certain references</i></li>
                        <li>Extra characters in process progress view </li>
                        <li>SAM files handling bugfixes</li>
                    </ul>
                    <ul className="category-section"><p>Tutorials</p>
                        <li>HIVE Tutorial for DESeq </li>
                        <li>HIVE Tutorial for RNAseq </li>
                        <li>HIVE Tutorial for Using DNA-Insilico to Extract Regions from References with a CSV Table</li>
                        <li>Object Sharing Tutorial</li>
                        <li>HIVE Annotation Mapper Tutorial</li>
                        <li>HIVE File Loader and Sharing Tutorial</li>
                    </ul>

            </div>

            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 2.3
            </h4>
            <div className="category-sections columns note-box_content" data-collapsible="content">
                <ul className="category-section"><p>New Tools:</p>
                    <li>DenovoQC: <i>simple statistics on assembled sequence such as N50 etc</i></li>
                    <li>Agilent SureCallTrimmer</li>
                    <li>IMGT StatClonoType</li>
                    <li>RNAseq QC</li>
                    <li>Targeted Alignments QC</li>
                </ul>
                <ul className="category-section"><p>New Features/Improvements:</p>
                    <li>Deseq2:
                        <i>new features for input validation and interface categories sorting, filtering</i>
                        <i>visual graphic of results from DESeq</i>
                    </li>
                    <li>DI profiler: <i>batch submission</i></li>
                    <li>DNA Insilico: <i >batch submission</i></li>
                    <li>Heptagon:
                        <i>lower case bases displayed in consensus when filled with reference</i>
                        <i>New feature for changing scale of profile graphs</i>
                    </li>
                    <li>Octagon:
                        <i>minor optimization</i>
                    </li>
                    <li></li>
                    <li>HIVE Portal:
                        <i>search feature addition</i>
                        <i>new Status column</i>
                        <i>filter projects by Status</i>
                    </li>
                    <li>Implement SSO using FDA AD</li>
                    <li>Scratch data storage performance improvement</li>
                    <li>Remove obsolete JavaScript dependencies from web pages</li>
                    <li>Prohibit users from editing their own permissions along with other users</li>
                    <li>Enforce limit on consecutive invalid logon attempts</li>
                    <li></li>
                    <li>Object selection pop up: change the button from "submit" to "select"</li>
                    <li>Left panel in computation pages sorted the same way as the tabs displayed</li>
                    <li>Project Portal:"Visibility Scope" field defaults "Public"</li>
                    <li>Add banner to web pages:MS Internet Explorer not supported</li>
                </ul>
                <ul className="category-section"><p>Bug Fixes:</p>
                    <li>Aligners:
                        <i>modify and resubmit computations allows change of algorithm</i>
                        <i>modify and resubmit losses algorithm name</i>
                    </li>
                    <li>Annotations:
                        <i>searching files over 150G fails</i>
                        <i>sometimes fails to display results</i>
                    </li>
                    <li>Censuscope:
                        <i>allows submission missing required fields</i>
                        <i>unaligned hits are based on unique instead of total hits</i>
                    </li>
                    <li>FASTA exporting:
                        <i>ID line of collapsed reads is not using field separator</i>
                    </li>
                    <li>Heptagon:
                        <i>zoom feature is not working when Start or End positions are specified</i>
                        <i>annotation graph is not working</i>
                        <i>tool sometimes is not displaying alignments when zoomed in</i>
                        <i>tool is missing from HIVE Portal</i>
                    </li>
                    <li>Hexagon:
                        <i>hovering over download item "Alignments in ... al identifiers" does not show the full title</i>
                        <i>"Download" panel is not displayed properly</i>
                    </li>
                    <li>MultiQC: <i>notifications displayed even when no errors occurred</i></li>
                    <li>Reads QC: <i>ACGT plots are not accounting for 'N'</i></li>
                    <li>Table Query:
                        <i>transpose results in data corruption</i>
                        <i>tool does not pull up color selection buttons in algorithmic plugins</i>
                        <i>buttons of analysis plugins not showing up</i>
                    </li>
                    <li></li>
                    <li>Downloading post computational results are sometimes corrupt</li>
                    <li>Parsing table to annotations fails</li>
                    <li>Processing files into HIVE is sometimes extremely slow</li>
                    <li>Projects list: filter not using all fields</li>
                    <li>Multiple failed attempts to login not blocked</li>
                    <li>Project Portal:  search doesn't return complete results</li>
                    <li>Metadata import sometimes fails</li>
                    <li>Home page tools' buttons for Hexagon and HIVESeq has invalid links</li>
                    <li></li>
                    <li>Footer on pages displayed behind content</li>
                    <li>Displaying tables overlaps scrollbars</li>
                    <li>Sunburst diagram text and layout are overlapping in case of small drawing area</li>
                    <li>AlgoRLDA Classifier Column parameter not displaying column names</li>
                    <li>Typo in pop-up window on computation submission</li>
                </ul>
            </div>

            <h4
                style={{marginTop: '0px'}}
                data-collapsible="switch"
                className="note-box_title"
                onClick={(event) => this.toggleNoteBox(event)}
            >HIVE Release 2.2</h4>
            <div className="category-sections columns note-box_content" data-collapsible="content">
                <ul className="category-section"><p>New tools:</p>
                    <li>orfFinder</li>
                    <li>DI profiler</li>
                    <li>codon QC</li>
                    <li>denovo QC</li>
                    <li>codon usage</li>
                </ul>
                <ul className="category-section"><p>3rd party tools integration:</p>
                    <li>Canu</li>
                    <li>GPS</li>
                    <li>Glymps</li>
                    <li>deseq</li>
                    <li>cuffdiff</li>
                    <li>Rosetta scripts <i>new version</i></li>
                </ul >
                <ul className="category-section"><p> New features:</p>
                <li>Heptagon</li>
                <li>Hexagon</li>

                </ul>
                <ul className="category-section"><p>Bug fixes:</p>
                    <li>Emails contain proper urls </li>
                    <li>Type cache handling</li>
                    <li>Batching service</li>
                    <li>Projects support, time logging</li>
                    <li>Usage collection</li>
                    <li>Interface <i>bug fixes + updates</i></li>
                    <li>ObjQuery</li>
                    <li>TableQuery</li>
                    <li>Python <i>integration updates</i></li>
                    <li>Glyco tools <i>updates from GWU</i></li>
                    <li>CAR-T<i>updates</i></li>
                    <li>Alignment comparator</li>
                    <li>Hexagon <i>bug fixes + optimizations</i></li>
                    <li>Heptagon <i>bug fixes + optimizations</i></li>
                    <li>Hexahedron<i>bug fixes + optimizations</i></li>
                    <li>RLDA <i>bug fixes + optimizations</i></li>
                    <li>Censuscope <i>bug fixes + optimizations</i></li>
                </ul>
            </div>
        </div>
    </>
    )
  }
}