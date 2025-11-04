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
const ActionsData ={
    "objs": [
        {
            "_id": 26,
            "_type": "action",
            "name": "edit",
            "path": "/Edit/edit",
            "url": "?cmd=record&ids=$(ids)&types=$(types)",
            "title": "Edit",
            "description": "Examine, review, edit details of the object",
            "icon": 'form'
        },
        {
            "_id": 239,
            "_type": "action",
            "name": "cast2spectra",
            "path": "/Convert/To Spectra/cast2spectra",
            "url": "?cmd=spectraPeakDetection&SpectraFile=$(ids)&selfDir=1",
            "title": "Spectra",
            "description": "Convert file formatted as CSV to a Spectra object"
        },
        {
            "_id": 238,
            "_type": "action",
            "name": "cast2spectra-lib",
            "path": "/Convert/To Spectra/cast2spectra-lib",
            "url": "?cmd=scast&type=spectra-lib&ids=$(ids)",
            "title": "Spectra Library",
            "description": "Convert file formatted as CSV to Spectra Library object"
        },
        {
            "_id": 237,
            "_type": "action",
            "name": "cast2spectra-MS",
            "path": "/Convert/To Spectra/cast2spectra-MS",
            "url": "?cmd=spectraPeakDetection&SpectraFile=$(ids)&selfDir=1",
            "title": "Spectra MS",
            "description": "Convert file formatted as CSV to Spectra-MS object"
        },
        {
            "_id": 236,
            "_type": "action",
            "name": "spectra",
            "url": "?cmd=spectraPeakDetection&&SpectraFile=$(ids)",
            "title": "Spectral Analyzer",
            "description": "Spectral Data Analyzer"
        },
        {
            "_id": 235,
            "_type": "action",
            "name": "spectra",
            "url": "?cmd=spectraPeakDetection&&SpectraFile=$(ids)",
            "title": "Spectral Analyzer",
            "description": "Spectral Data Analyzer"
        },
        {
            "_id": 234,
            "_type": "action",
            "name": "censuscope_nuc_read",
            "url": "?cmd=dna-screening&query=$(ids)",
            "title": "censuScope",
            "description": "Analyze using CensuScope",
            "icon":'/scope'
        },
        {
            "_id": 233,
            "_type": "action",
            "name": "TreatAssra",
            "path": "/Convert/Reprocess/TreatAssra",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=sra",
            "title": "Treat as sra file (.sra)",
            "description": "Process file compressed as sra"
        },
        {
            "_id": 232,
            "_type": "action",
            "name": "convert2fasta",
            "path": "/Convert/To Reads/convert2fasta",
            "url": "?cmd=archive&dissect=1000&objtype=nuc-read&ids=$(ids)&datatype=fasta",
            "title": "Treat as FastA file",
            "description": "Convert file formatted as fastA to Reads object"
        },
        {
            "_id": 231,
            "_type": "action",
            "name": "convert2fastq",
            "path": "/Convert/To Reads/convert2fastq",
            "url": "?cmd=archive&dissect=1000&objtype=nuc-read&ids=$(ids)&datatype=fastq",
            "title": "Treat as FastQ file",
            "description": "Convert file formatted as fastQ to Reads object"
        },
        {
            "_id": 230,
            "_type": "action",
            "name": "convert2genome",
            "path": "/Convert/convert2genome",
            "url": "?cmd=archive&dissect=1000&category=genomic&objtype=genome&ids=$(ids)&datatype=fasta",
            "title": "To Genome as FastA file",
            "description": "Convert file formatted as fastA to Genome object"
        },
        {
            "_id": 229,
            "_type": "action",
            "name": "convert2sam",
            "path": "/Convert/To Reads/convert2sam",
            "url": "?cmd=archive&dissect=1000&objtype=nuc-read&ids=$(ids)&datatype=sam",
            "title": "Treat as SAM File",
            "description": "Convert file formatted as SAM to Reads object"
        },
        {
            "_id": 228,
            "_type": "action",
            "name": "cast2genome",
            "path": "/Convert/cast2genome",
            "url": "?cmd=scast&type=genome&ids=$(ids)",
            "title": "Convert to Genome",
            "description": "Convert from Reads to Genome"
        },
        {
            "_id": 227,
            "_type": "action",
            "name": "cast2nuc-read",
            "path": "/Convert/cast2nuc-read",
            "url": "?cmd=scast&type=nuc-read&ids=$(ids)",
            "title": "Convert to Reads",
            "description": "Convert from Genome to Reads"
        },
        {
            "_id": 226,
            "_type": "action",
            "name": "export_hivepack",
            "path": "/Export/export_hivepack",
            "url": "?cmdr=-qpProcSubmit&svc=dmCompressor&prop.x._type=svc-compressor&prop.x.name=Objects%20HIVEPack&prop.x.function=objHivePack&prop.x.withDependencies=true&prop.x.objs=$(ids)",
            "title": "Export as HIVEPack",
            "description": "Export the object as HIVEPack with dependencies",
            "icon": 'export',
            "reload": true
        },
        {
            "_id": 225,
            "_type": "action",
            "name": "exportEx",
            "path": "/Export/exportEx",
            "url": "?cmd=fileCompressor&objs=$(ids)",
            "title": "ExportEx",
            "description": "More export options",
            "icon": 'export'
        },
        {
            "_id": 224,
            "_type": "action",
            "name": "upload",
            "path": "/Edit/upload",
            "title": "Upload a new file",
            "description": "Upload a file",
            "icon": 'cloud-upload'
        },
        {
            "_id": 223,
            "_type": "action",
            "name": "TreatAsAuto",
            "path": "/Convert/Reprocess/TreatAsAuto",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)",
            "title": "Auto detect",
            "description": "Reprocess recognizing file type by its extension"
        },
        {
            "_id": 222,
            "_type": "action",
            "name": "TreatAsbz2",
            "path": "/Convert/Reprocess/TreatAsbz2",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=bz2",
            "title": "Treat as bzip2 file (.bz2)",
            "description": "Process file compressed as bzip2"
        },
        {
            "_id": 221,
            "_type": "action",
            "name": "TreatAstbz2",
            "path": "/Convert/Reprocess/TreatAstbz2",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=tbz2",
            "title": "Treat as tar bzip2 file (.tbz2)",
            "description": "Process file compressed as tar bzip2"
        },
        {
            "_id": 220,
            "_type": "action",
            "name": "TreatAsgz",
            "path": "/Convert/Reprocess/TreatAsgz",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=gz",
            "title": "Treat as gzip file (.gz)",
            "description": "Process file compressed as gzip"
        },
        {
            "_id": 219,
            "_type": "action",
            "name": "TreatAstgz",
            "path": "/Convert/Reprocess/TreatAstgz",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=tgz",
            "title": "Treat as tar gzip file (.tgz)",
            "description": "Process file compressed as tar gzip"
        },
        {
            "_id": 218,
            "_type": "action",
            "name": "TreatAstar",
            "path": "/Convert/Reprocess/TreatAstar",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=tar",
            "title": "Treat as tar file (.tar)",
            "description": "Process file compressed as tar"
        },
        {
            "_id": 217,
            "_type": "action",
            "name": "TreatAsbam",
            "path": "/Convert/Reprocess/TreatAsbam",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=bam",
            "title": "Treat as bam file (.bam)",
            "description": "Process file compressed as bam"
        },
        {
            "_id": 216,
            "_type": "action",
            "name": "TreatAszip",
            "path": "/Convert/Reprocess/TreatAszip",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&ext=zip",
            "title": "Treat as zip file (.zip)",
            "description": "Process file compressed as zip"
        },
        {
            "_id": 215,
            "_type": "action",
            "name": "convert2protein",
            "path": "/Convert/convert2protein",
            "url": "?cmd=archive&dissect=1000&objtype=prot-seq&ids=$(ids)&datatype=fasta",
            "title": "To Protein Sequence as FastA file",
            "description": "Convert file formatted as fastA to Protein Sequence object"
        },
        {
            "_id": 214,
            "_type": "action",
            "name": "convert2geneExpressOmics",
            "path": "/Convert/To Gene Expression/convert2geneExpressOmics",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&objtype=u-ionExpress&isExpr=1&hasdata=omics&invitro=0&experiment=$(?:Please provide experiment)",
            "title": "To Omics data ",
            "description": "Convert file formatted as CSV to Omics gene expression object"
        },
        {
            "_id": 213,
            "_type": "action",
            "name": "convert2geneExpressInVitro",
            "path": "/Convert/To Gene Expression/convert2geneExpressInVitro",
            "url": "?cmd=archive&dissect=1000&ids=$(ids)&objtype=u-ionExpress&isExpr=1&hasdata=measurements&invitro=1&experiment=$(?:Please provide experiment)",
            "title": "To In Vitro data",
            "description": "Convert file formatted as CSV to gene expression object"
        },
        {
            "_id": 212,
            "_type": "action",
            "name": "convert2geneExpressGeneList",
            "path": "/Convert/To Gene Expression/convert2geneExpressGeneList",
            "url": "?cmd=ingestGeneList&objToConvert=$(ids)&convert=1",
            "title": "To Gene List ",
            "description": "Convert file formatted as CSV to gene list for Gene Expression Analysis"
        },
        {
            "_id": 211,
            "_type": "action",
            "name": "hiveseq",
            "url": "?cmd=menu&root=Portal&selected=General_DNA_Filters_and_Tools&ids=$(ids)",
            "title": "Hiveseq editor",
            "description": "Go to Tools and Filters Menu",
            "icon":'/hiveseq'
        },
        {
            "_id": 210,
            "_type": "action",
            "name": "jumpToTblqry",
            "path": "/jumpToTblqry",
            "url": "?cmd=tblqry-new&tqsId=$(ids)",
            "title": "Go to TableQuery",
            "description": "Go to TableQuery with current TQS"
        },
        {
            "_id": 209,
            "_type": "action",
            "name": "cast2tqs",
            "path": "/Convert/cast2tqs",
            "url": "?cmd=scast&type=u-tqs&ids=$(ids)",
            "title": "Convert to TQS",
            "description": "Convert current object to TQS type"
        },
        {
            "_id": 208,
            "_type": "action",
            "name": "tblQry",
            "url": "?cmd=tblqry-new&objs=$(ids)",
            "title": "Analyze",
            "description": "Analyze table with Table Query"
        },
        {
            "_id": 181,
            "_type": "action",
            "name": "hexagon_nuc_read",
            "url": "?cmd=menu&root=Portal&selected=Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon&query=$(ids)",
            "title": "Hive-hexagon",
            "description": "Align using hive-hexagon",
            //"icon":'/hive-hexagon'
        },
        {
            "_id": 180,
            "_type": "action",
            "name": "hexagon_genome",
            "url": "?cmd=menu&root=Portal&selected=Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon&query=$(ids)",
            "title": "Hive-hexagon",
            "description": "Align using hive-hexagon",
            //"icon":'/hive-hexagon'
        },
        {
            "_id": 29,
            "_type": "action",
            "name": "create",
            "path": "/Edit/create",
            "url": "?cmd=act&act=create&type=$(type)",
            "title": "Create",
            "description": "Create an object",
            "icon": 'file-add'
        },
        {
            "_id": 28,
            "_type": "action",
            "name": "search",
            "url": "?cmd=objList&type=$(type)&prop_val=$(search)&start=$(start)&cnt=$(cnt)",
            "title": "Search",
            "description": "create"
        },
        {
            "_id": 27,
            "_type": "action",
            "name": "detail",
            "path": "/Edit/detail",
            "url": "?cmd=record&ids=$(ids)&types=$(types)&readonly=1",
            "title": "View details",
            "description": "See objects metadata",
            "icon": 'file-text'
        },
        {
            "_id": 25,
            "_type": "action",
            "name": "export_json",
            "path": "/Export/export_json",
            "url": "?cmd=propget&mode=json&ids=$(ids)&raw=1",
            "title": "Export as JSON",
            "description": "Export the object metadata",
            "icon": 'export'
        },
        {
            "_id": 24,
            "_type": "action",
            "name": "copy",
            "path": "/Edit/copy",
            "url": "javascript:gClip.copy($(srcFolder),$(ids))",
            "title": "Copy",
            "icon": 'copy'
        },
        {
            "_id": 23,
            "_type": "action",
            "name": "cut",
            "path": "/Edit/cut",
            "url": "javascript:gClip.cut($(srcFolder),$(ids))",
            "title": "Cut",
            "icon": 'scissor'
        },
        {
            "_id": 22,
            "_type": "action",
            "name": "delete",
            "path": "/Edit/delete",
            "url": "javascript:gClip._delete( $(s:srcFolder),$(s:dcls),$(s:ids) )",
            "title": "Delete",
            "description": "Delete object",
            "icon": 'delete',
            "reload": true
        },
        {
            "_id": 20,
            "_type": "action",
            "name": "share",
            "path": "/Edit/share",
            "url": "?cmd=sharing&ids=$(ids)",
            "title": "Share",
            "description": "share the object with other users of the system; esablish permissions.",
            "icon": 'share-alt'
        },
        {
            "_id": 19,
            "_type": "action",
            "name": "rename",
            "path": "/Edit/rename",
            "url": "?cmd=propset&prop.$(ids).name.1=$(?: Please provide new name:)",
            "title": "Rename",
            "description": "Quickly rename an object.",
            "icon": 'edit',
            "reload": true
        },
        {
            "_id": 18,
            "_type": "action",
            "name": "subfolder",
            "path": "/Edit/subfolder",
            "url": "?cmd=folderCreate&ids=$(ids)&name=$(?: Please provide new name:)",
            "title": "Create subfolder",
            "description": "Create a subfolder in selected folder",
            "icon": 'folder-add',
            "reload": true
        },
        {
            "_id": 17,
            "_type": "action",
            "name": "paste",
            "path": "/Edit/paste",
            "url": "javascript:gClip.paste($(ids),$(s:dcls))",
            "title": "Paste",
            "reload": true
        },
        {
            "_id": 16,
            "_type": "action",
            "name": "download",
            "path": "/Export/download",
            "url": "?cmd=objFile&ids=$(ids)",
            "description": "Download original file to local computer",
            "title": "Download file",
            "icon": 'cloud-download'
        },
        {
            "_id": 15,
            "_type": "action",
            "name": "cast2systemImage",
            "path": "/Convert/cast2systemImage",
            "url": "?cmd=scast&type=system-image&ids=$(ids)",
            "title": "Convert to system image",
            "description": "Convert from Image to System Image"
        },
        {
            "_id": 14,
            "_type": "action",
            "name": "cast2multimedia",
            "path": "/Convert/cast2multimedia",
            "url": "?cmd=scast&type=multimedia&ids=$(ids)",
            "title": "Convert to multimedia object",
            "description": "Convert from File to Multimedia Object"
        },
        {
            "_id": 13,
            "_type": "action",
            "name": "procClone",
            "path": "/Edit/procClone",
            "url": "?cmd=-qpProcClone&src_ids=$(ids)",
            "title": "Resubmit",
            "description": "Resubmit the process with same parameters (without confirmation)"
        }
    ]
}

export default ActionsData;