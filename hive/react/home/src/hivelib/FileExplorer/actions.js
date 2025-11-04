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
const ActionsData = {
    "objs": [{
        "_id": 139231,
        "_type": "action",
        "is_default": true,
        "name": "Open",
        "order": 0,
        "required_permission": 3,
        "single_obj_only": true,
        "path": "/open",
        "target": "_blank",
        "title": "Open",
        "url": "?cmd=$(submitter)&id=$(ids)",
        "confirmation": false
    }, {
        "_id": 239,
        "_type": "action",
        "name": "cast2spectra",
        "order": 12,
        "path": "/Convert/To Spectra/cast2spectra",
        "required_permission": 4,
        "target": "new",
        "url": "?cmd=spectraPeakDetection&SpectraFile=$(ids)&selfDir=1",
        "title": "Spectra",
        "description": "Convert file formatted as CSV to a Spectra object"
    }, {
        "_id": 238,
        "_type": "action",
        "name": "cast2spectra-lib",
        "order": 12,
        "path": "/Convert/To Spectra/cast2spectra-lib",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=scast&type=spectra-lib&ids=$(ids)",
        "title": "Spectra Library",
        "description": "Convert file formatted as CSV to Spectra Library object"
    }, {
        "_id": 237,
        "_type": "action",
        "name": "cast2spectra-MS",
        "order": 12,
        "path": "/Convert/To Spectra/cast2spectra-MS",
        "required_permission": 4,
        "target": "new",
        "url": "?cmd=spectraPeakDetection&SpectraFile=$(ids)&selfDir=1",
        "title": "Spectra MS",
        "description": "Convert file formatted as CSV to Spectra-MS object"
    }, {
        "_id": 236,
        "_type": "action",
        "name": "spectra",
        "order": -3,
        "required_permission": 2,
        "target": "true",
        "url": "?cmd=spectraPeakDetection&&SpectraFile=$(ids)",
        "title": "Spectral Analyzer",
        "description": "Spectral Data Analyzer"
    }, {
        "_id": 235,
        "_type": "action",
        "name": "spectra",
        "order": 6,
        "required_permission": 2,
        "target": "true",
        "url": "?cmd=spectraPeakDetection&&SpectraFile=$(ids)",
        "title": "Spectral Analyzer",
        "description": "Spectral Data Analyzer"
    }, {
        "_id": 234,
        "_type": "action",
        "icon": "/img/logos/scope.svg",
        "name": "censuscope_nuc_read",
        "order": 5,
        "required_permission": 2,
        "target": "true",
        "url": "?cmd=dna-screening&query=$(ids)",
        "title": "censuScope",
        "description": "Analyze using CensuScope"
    }, {
        "_id": 233,
        "_type": "action",
        "name": "TreatAssra",
        "order": 7,
        "path": "/Convert/Reprocess/TreatAssra",
        "required_permission": 4,
        "single_obj_only": false,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=sra",
        "title": "Treat as sra file (.sra)",
        "description": "Process file compressed as sra"
    },{
        "_id": '232conv',
        "_type": "action",
        "name": "Convert",
        "order": 4,
        "title": "Convert",
        "path": "/Convert"
    }, {
        "_id": 232,
        "_type": "action",
        "name": "convert2fasta",
        "order": 12,
        "path": "/Convert/To Reads/convert2fasta",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&objtype=nuc-read&ids=$(ids)&datatype=fasta",
        "title": "Treat as FastA file",
        "description": "Convert file formatted as fastA to Reads object"
    }, {
        "_id": 231,
        "_type": "action",
        "name": "convert2fastq",
        "order": 12,
        "path": "/Convert/To Reads/convert2fastq",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&objtype=nuc-read&ids=$(ids)&datatype=fastq",
        "title": "Treat as FastQ file",
        "description": "Convert file formatted as fastQ to Reads object"
    }, {
        "_id": 230,
        "_type": "action",
        "name": "convert2genome",
        "order": 12,
        "path": "/Convert/convert2genome",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&category=genomic&objtype=genome&ids=$(ids)&datatype=fasta",
        "title": "To Genome as FastA file",
        "description": "Convert file formatted as fastA to Genome object"
    }, {
        "_id": 229,
        "_type": "action",
        "name": "convert2sam",
        "order": 12,
        "path": "/Convert/To Reads/convert2sam",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&objtype=nuc-read&ids=$(ids)&datatype=sam",
        "title": "Treat as SAM File",
        "description": "Convert file formatted as SAM to Reads object"
    }, {
        "_id": 228,
        "_type": "action",
        "name": "cast2genome",
        "order": 12,
        "path": "/Convert/cast2genome",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=scast&type=genome&ids=$(ids)",
        "title": "Convert to Genome",
        "description": "Convert from Reads to Genome"
    }, {
        "_id": 227,
        "_type": "action",
        "name": "cast2nuc-read",
        "order": 12,
        "path": "/Convert/cast2nuc-read",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=scast&type=nuc-read&ids=$(ids)",
        "title": "Convert to Reads",
        "description": "Convert from Genome to Reads"
    },
    {
        "_id": 57704,
        "description": "Export BCO",
        "icon": "/img/logos/small/bco_logo.png",
        "name": "export_bco",
        "order": 10,
        "path": "/Export/export_bco",
        "required_permission": 128,
        "target": "true",
        "title": "Export BCO",
        "url": "?follow=///r/home/?tab=bcoeditor&objs=$(ids)&projectID=$(projectID)",
        "_type": "action"
    },
    {
        "_id": 226,
        "_type": "action",
        "name": "export_hivepack",
        "order": 10,
        "path": "/Export/export_hivepack",
        "required_permission": 128,
        "target": "ajax",
        "url": "?cmdr=-qpProcSubmit&svc=dmCompressor&prop.x._type=svc-compressor&prop.x.name=Objects%20HIVEPack&prop.x.function=objHivePack&prop.x.withDependencies=true&prop.x.objs=$(ids)",
        "title": "Export as HIVEPack",
        "description": "Export the object as HIVEPack with dependencies",
        "icon": "export",
        "reload": true
    }, {
        "_id": 225,
        "_type": "action",
        "name": "exportEx",
        "order": 1000,
        "path": "/Export/exportEx",
        "required_permission": 128,
        "target": "true",
        "url": "?cmd=fileCompressor&objs=$(ids)&projectID=$(projectID)",
        "title": "ExportEx",
        "description": "More export options",
        "icon": "export"
    }, {
        "_id": 224,
        "_type": "action",
        "icon": "cloud-upload",
        "name": "upload",
        "order": 13,
        "required_permission": 0,
        "title": "Upload a new file",
        "description": "Upload a file",
        "path": "/Edit/upload"
    }, {
        "_id": 223,
        "_type": "action",
        "name": "TreatAsAuto",
        "order": 1,
        "path": "/Convert/Reprocess/TreatAsAuto",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)",
        "title": "Auto detect",
        "description": "Reprocess recognizing file type by its extension"
    }, {
        "_id": 222,
        "_type": "action",
        "name": "TreatAsbz2",
        "order": 5,
        "path": "/Convert/Reprocess/TreatAsbz2",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=bz2",
        "title": "Treat as bzip2 file (.bz2)",
        "description": "Process file compressed as bzip2"
    }, {
        "_id": 221,
        "_type": "action",
        "name": "TreatAstbz2",
        "order": 6,
        "path": "/Convert/Reprocess/TreatAstbz2",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=tbz2",
        "title": "Treat as tar bzip2 file (.tbz2)",
        "description": "Process file compressed as tar bzip2"
    }, {
        "_id": 220,
        "_type": "action",
        "name": "TreatAsgz",
        "order": 3,
        "path": "/Convert/Reprocess/TreatAsgz",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=gz",
        "title": "Treat as gzip file (.gz)",
        "description": "Process file compressed as gzip"
    }, {
        "_id": 219,
        "_type": "action",
        "name": "TreatAstgz",
        "order": 4,
        "path": "/Convert/Reprocess/TreatAstgz",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=tgz",
        "title": "Treat as tar gzip file (.tgz)",
        "description": "Process file compressed as tar gzip"
    }, {
        "_id": 218,
        "_type": "action",
        "name": "TreatAstar",
        "order": 2,
        "path": "/Convert/Reprocess/TreatAstar",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=tar",
        "title": "Treat as tar file (.tar)",
        "description": "Process file compressed as tar"
    }, {
        "_id": 217,
        "_type": "action",
        "name": "TreatAsbam",
        "order": 8,
        "path": "/Convert/Reprocess/TreatAsbam",
        "required_permission": 4,
        "single_obj_only": true,
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=bam",
        "title": "Treat as bam file (.bam)",
        "description": "Process file compressed as bam"
    }, {
        "_id": 216,
        "_type": "action",
        "name": "TreatAszip",
        "order": 9,
        "path": "/Convert/Reprocess/TreatAszip",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=zip",
        "title": "Treat as zip file (.zip)",
        "description": "Process file compressed as zip"
    }, {
        "_id": 215,
        "_type": "action",
        "name": "convert2protein",
        "order": 12,
        "path": "/Convert/convert2protein",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&objtype=prot-seq&ids=$(ids)&datatype=fasta",
        "title": "To Protein Sequence as FastA file",
        "description": "Convert file formatted as fastA to Protein Sequence object"
    }, {
        "_id": 214,
        "_type": "action",
        "name": "convert2geneExpressOmics",
        "order": 12,
        "path": "/Convert/To Gene Expression/convert2geneExpressOmics",
        "required_permission": 3,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&objtype=u-ionExpress&isExpr=1&hasdata=omics&invitro=0&experiment=$(?:Please provide experiment)",
        "title": "To Omics data ",
        "description": "Convert file formatted as CSV to Omics gene expression object"
    }, {
        "_id": 213,
        "_type": "action",
        "name": "convert2geneExpressInVitro",
        "order": 12,
        "path": "/Convert/To Gene Expression/convert2geneExpressInVitro",
        "required_permission": 3,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=archive&dissect=1000&ids=$(ids)&objtype=u-ionExpress&isExpr=1&hasdata=measurements&invitro=1&experiment=$(?:Please provide experiment)",
        "title": "To In Vitro data",
        "description": "Convert file formatted as CSV to gene expression object"
    }, {
        "_id": 212,
        "_type": "action",
        "name": "convert2geneExpressGeneList",
        "order": 12,
        "path": "/Convert/To Gene Expression/convert2geneExpressGeneList",
        "required_permission": 3,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=ingestGeneList&objToConvert=$(ids)&convert=1",
        "title": "To Gene List ",
        "description": "Convert file formatted as CSV to gene list for Gene Expression Analysis"
    }, {
        "_id": 211,
        "_type": "action",
        "confirmation": false,
        "icon": "/img/logos/hiveseq.svg",
        "name": "hiveseq",
        "order": -3,
        "required_permission": 2,
        "single_obj_only": true,
        "target": "true",
        "url": "?cmd=menu&root=Portal&selected=General_DNA_Filters_and_Tools&ids=$(ids)&projectID=$(projectID)",
        "title": "Hiveseq editor",
        "description": "Go to Tools and Filters Menu"
    }, {
        "_id": 210,
        "_type": "action",
        "name": "jumpToTblqry",
        "order": 12,
        "path": "/jumpToTblqry",
        "required_permission": 4,
        "target": "new",
        "url": "?cmd=tblqry-new&tqsId=$(ids)",
        "title": "Go to TableQuery",
        "description": "Go to TableQuery with current TQS"
    }, {
        "_id": 209,
        "_type": "action",
        "name": "cast2tqs",
        "order": 12,
        "path": "/Convert/cast2tqs",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=scast&type=u-tqs&ids=$(ids)",
        "title": "Convert to TQS",
        "description": "Convert current object to TQS type"
    }, {
        "_id": 208,
        "_type": "action",
        "icon": "/img/logos/analyze.svg",
        "name": "tblQry",
        "order": 13,
        "required_permission": 2,
        "target": "new",
        "url": "?cmd=tblqry-new&objs=$(ids)",
        "title": "Analyze",
        "description": "Analyze table with Table Query"
    }, {
        "_id": 181,
        "_type": "action",
        "confirmation": false,
        "icon": "/img/logos/small/hive-hexagon_logo.png",
        "name": "hexagon_nuc_read",
        "order": 6,
        "required_permission": 2,
        "single_obj_only": false,
        "target": "true",
        "url": "?cmd=menu&root=Portal&selected=Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon&query=$(ids)",
        "title": "Hive-hexagon",
        "description": "Align using hive-hexagon"
    }, {
        "_id": 180,
        "_type": "action",
        "confirmation": false,
        "icon": "/img/logos/small/hive-hexagon_logo.png",
        "name": "hexagon_genome",
        "order": 6,
        "required_permission": 2,
        "single_obj_only": false,
        "target": "true",
        "url": "?cmd=menu&root=Portal&selected=Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon&query=$(ids)",
        "title": "Hive-hexagon",
        "description": "Align using hive-hexagon"
    }, {
        "_id": 29,
        "_type": "action",
        "icon": "file-add",
        "name": "create",
        "order": 0,
        "required_permission": 0,
        "target": "true",
        "url": "?cmd=act&act=create&type=$(type)",
        "title": "Create",
        "description": "Create an object",
        "path": "/Edit/create"
    }, {
        "_id": 28,
        "_type": "action",
        "name": "search",
        "order": 0,
        "required_permission": 0,
        "target": "ajax",
        "url": "?cmd=objList&type=$(type)&prop_val=$(search)&start=$(start)&cnt=$(cnt)",
        "title": "Search",
        "description": "create"
    },
    {
        "_id": '27edit',
        "_type": "action",
        "name": "Edit",
        "order": 2,
        "title": "Edit",
        "path": "/Edit"
    }, {
        "_id": 27,
        "_type": "action",
        "icon": "file-text",
        "name": "detail",
        "order": 10,
        "required_permission": 2,
        "single_obj_only": true,
        "target": "true",
        "url": "?cmd=record&ids=$(ids)&types=$(types)&readonly=1",
        "title": "View details",
        "description": "See objects metadata",
        "path": "/Edit/detail"
    }, {
        "_id": 26,
        "_type": "action",
        "icon": "form",
        "name": "edit",
        "order": 9,
        "path": "/Edit/edit",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "true",
        "url": "?cmd=record&ids=$(ids)&types=$(types)",
        "title": "Edit",
        "description": "Examine, review, edit details of the object"
    },
    {
        "_id": '25ex',
        "_type": "action",
        "name": "Export",
        "order": 3,
        "title": "Export",
        "path": "/Export"
    },
    {
        "_id": 25,
        "_type": "action",
        "name": "export_json",
        "order": 10,
        "path": "/Export/export_json",
        "required_permission": 2,
        "target": "true",
        "url": "?cmd=propget&mode=json&ids=$(ids)&raw=1",
        "title": "Export as JSON",
        "description": "Export the object metadata",
        "icon": "export"
    }, {
        "_id": 24,
        "_type": "action",
        "icon": "copy",
        "name": "copy",
        "order": 4,
        "path": "/Edit/copy",
        "required_permission": 2,
        "url": "javascript:gClip.copy($(srcFolder),$(ids))", // eslint-disable-next-line
        "title": "Copy",
        "note_duration": 0.5,
        "note_type": "info"
    }, {
        "_id": 23,
        "_type": "action",
        "icon": "scissor",
        "name": "cut",
        "order": 4,
        "path": "/Edit/cut",
        "required_permission": 2,
        "url": "javascript:gClip.cut($(srcFolder),$(ids))", // eslint-disable-next-line
        "title": "Cut",
        "note_duration": 0.5,
        "note_type": "info"
    }, {
        "_id": 22,
        "_type": "action",
        "confirmation": true,
        "icon": "delete",
        "name": "delete",
        "order": 3,
        "required_permission": 2,
        "url": "javascript:gClip._delete( $(s:srcFolder),$(s:dcls),$(s:ids) )", // eslint-disable-next-line
        "title": "Delete",
        "description": "Delete object",
        "path": "/Edit/delete",
        "note_duration": 0,
        "reload": true
    }, {
        "_id": 21,
        "_type": "action",
        "icon": "/img/logos/admin.gif",
        "name": "admin",
        "order": 2,
        "required_permission": 32,
        "target": "true",
        "url": "?cmd=admin&ids=$(ids)",
        "title": "Admin",
        "description": "Administer object"
    }, {
        "_id": 20,
        "_type": "action",
        "icon": "share-alt",
        "name": "share",
        "order": 8,
        "required_permission": 96,
        "target": "true",
        "url": "?cmd=sharing&ids=$(ids)",
        "title": "Share",
        "description": "share the object with other users of the system; esablish permissions.",
        "path": "/Edit/share"
    }, {
        "_id": 19,
        "_type": "action",
        "icon": "edit",
        "name": "rename",
        "order": 10,
        "path": "/Edit/rename",
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmdr=propset&prop.$(ids).name.1=$(?: Please provide new name: ${title})",
        "title": "Rename",
        "description": "Quickly rename an object.",
        "note_duration": 3,
        "reload": true
    }, {
        "_id": 18,
        "_type": "action",
        "icon": "folder-add",
        "name": "subfolder",
        "order": 7,
        "required_permission": 4,
        "single_obj_only": true,
        "target": "ajax",
        "url": "?cmd=folderCreate&ids=$(ids)&name=$(?: Please provide new name:)",
        "title": "Create subfolder",
        "description": "Create a subfolder in selected folder",
        "path": "/Edit/subfolder",
        "note_duration": 3,
        "reload": true
    }, {
        "_id": 17,
        "_type": "action",
        "icon": "paste",
        "name": "paste",
        "order": 6,
        "required_permission": 4,
        "single_obj_only": true,
        "url": "javascript:gClip.paste($(ids),$(s:dcls))", // eslint-disable-next-line
        "title": "Paste",
        "path": "/Edit/paste",
        "note_duration": 0,
        "reload": true
    }, {
        "_id": 16,
        "_type": "action",
        "icon": "cloud-download",
        "name": "download",
        "order": -1,
        "required_permission": 128,
        "single_obj_only": true,
        "url": "?cmd=objFile&ids=$(ids)&projectID=$(projectID)",
        "title": "Download file",
        "description": "Download original file to local computer",
        "path": "/Export/download"
    }, {
        "_id": 15,
        "_type": "action",
        "name": "cast2systemImage",
        "order": 12,
        "path": "/Convert/cast2systemImage",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=scast&type=system-image&ids=$(ids)",
        "title": "Convert to system image",
        "description": "Convert from Image to System Image"
    }, {
        "_id": 14,
        "_type": "action",
        "name": "cast2multimedia",
        "order": 12,
        "path": "/Convert/cast2multimedia",
        "required_permission": 4,
        "target": "ajax",
        "url": "?cmd=scast&type=multimedia&ids=$(ids)",
        "title": "Convert to multimedia object",
        "description": "Convert from File to Multimedia Object"
    }, {
        "_id": 13,
        "_type": "action",
        "name": "procClone",
        "order": 12,
        "required_permission": 2,
        "target": "ajax",
        "url": "?cmd=-qpProcClone&src_ids=$(ids)",
        "title": "Resubmit",
        "description": "Resubmit the process with same parameters (without confirmation)",
        "path": "/Edit/procClone"
    }]
}


export default ActionsData;
