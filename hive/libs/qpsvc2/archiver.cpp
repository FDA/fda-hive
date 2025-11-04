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

#include <qpsvc/archiver.hpp>
#include <xlib/dmlib.hpp>
#include <slib/std/file.hpp>
#include <ulib/ufolder.hpp>

dmArchiver::TKnownTypes dmArchiver::m_knownTypes[] = {
    {
        "prop-auto-detect" __,
        dmLib::eNone,
        "prop" __,
        0 },
    {
        "hivepack" __,
        dmLib::eNone,
        "hivepack" __,
        0 },
    {
        "nuc-read" __,
        dmLib::eGzip,
        "fasta" _ "fa" _ "fas" _ "fsa" _ "fna" _ "fq" _ "fastq" __,
        0 },
    {
        "nuc-read" __,
        dmLib::eNone,
        "vioseq2" _ "hiveseq" __,
        0 },
    {
        "nuc-read" _ "svc-align-dnaseq" __,
        dmLib::eGzip,
        "sam" __,
        "" _ "status,5\nsubmitter,dna-hexagon&cmdMode=dna-hexagon" __ },
    {
        "nuc-read" __,
        dmLib::eGzip,
        "srr" _ "sra" __,
        0 },
    {
        "image" __,
        dmLib::eNone,
        "jpg" _ "jpeg" _ "bmp" _ "pcx" _ "gif" _ "png" _ "tiff" _ "tif" _ "tga" _ "img" _ "ico" _ "svg" __,
        0 },
    {
        "viodb" __,
        dmLib::eNone,
        "viodb" __,
        0 },
    {
        "algorithm-script" __,
        dmLib::eNone,
        "ash" __,
        0 },
    {
        "genome" _ "svc-align-multiple" __,
        dmLib::eGzip,
        "ma" __,
        "" _ "status,5\nsubmitter,dna-hexagon&cmdMode=mafft" __ },
    {
        "genome" _ "u-ionAnnot" __,
        dmLib::eGzip,
        "gb" _ "gbk" _ "gbff" __,
        0 },
    {
        "u-idList" __,
        dmLib::eNone,
        "genelist" __,
        0 },
    {
        "u-ionExpress" __,
        dmLib::eNone,
        "ion" __,
        0 },
    {
        "excel-file" __,
        dmLib::eNone,
        "xls" _ "xlsx" __,
        0 },
    {
        "csv-table" __,
        dmLib::eNone,
        "csv" __,
        0 },
    {
        "tsv-table" __,
        dmLib::eNone,
        "tsv" _ "tab" __,
        0 },
    {
        "prot-seq" __,
        dmLib::eGzip,
        "faa" __,
        0 },
    {
        "target_library" __,
        dmLib::eGzip,
        "bt2" __,
        0 },
    {
        "dicom" __,
        dmLib::eNone,
        "dcm" _ "dicom" __,
        0 },
    {
        "EDFfile" __,
        dmLib::eNone,
        "edf" _ "edf+" __,
        0 },
    {
        "u-file" __,
        dmLib::eNone,
        0,
        0 }
};

dmArchiver::dmArchiver(sQPride& qp, const char * path, const char * dataSource, const char * formatHint, const char * name)
    : TParent(qp)
{
    setInput("%s", path);
    setInputName(name && name[0] ? name : path);
    setDataSource(dataSource);
    setFormatHint(formatHint);
}

dmArchiver::~dmArchiver()
{
}

void dmArchiver::setInput(const char * path, ...)
{
    sStr s;
    if( path ) {
        sCallVarg(s.vprintf, path);
    }
    setVar("inputFile", "%s", s.ptr());
}

void dmArchiver::setInputName(const char * name, ...)
{
    sStr s;
    if( name ) {
        sCallVarg(s.vprintf, name);
    }
    setVar("inputName", "%s", s.ptr());
}

void dmArchiver::setDataSource(const char* ds)
{
    setVar("datasource", "%s", ds ? ds : "");
}

void dmArchiver::setFormatHint(const char * dataType)
{
    if( dataType && dataType[0] ) {
        setVar("datatype", "%s", dataType);
    }
}

void dmArchiver::setFolder(sUsrFolder & folder)
{
    setVar("folder", "%" UDEC, folder.Id().objId());
}

void dmArchiver::setFolderId(const char * folderId)
{
    setVar("folder", "%s",folderId);
}


void dmArchiver::setSubject(const char * subject)
{
    if( subject && subject[0] ) {
        setVar("upload_subject", "%s", subject);
    }
}

static const char * const g_depth_name = "dissect";
static const char * const g_run_index_name = "run_index";
static const char * const g_run_qc_name = "run_qc";
static const char * const g_run_screenname = "run_screen";
static const char * const g_run_anonymize = "run_anonymize";
static const char * const g_run_compressor = "run_copmpressor";
static const char * const g_run_autoperm = "run_autoperm";


void dmArchiver::setDepth(sQPrideBase & qp, udx max_depth)
{
    qp.reqSetData(qp.grpId, g_depth_name, "%" UDEC, max_depth);
}
udx dmArchiver::getDepth(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_depth_name, &lvar);
    udx dflt = ~0;
    if(lvar) {
        sscanf(lvar, "%" UDEC, &dflt);
    }
    return dflt;
}

void dmArchiver::setIndexFlag(sQPrideBase & qp, idx flag)
{
    qp.reqSetData(qp.grpId, g_run_index_name, "%" DEC, flag);
}

idx dmArchiver::getIndexFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_run_index_name, &lvar);
    idx dflt = 1;
    if(lvar) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}

void dmArchiver::setQCFlag(sQPrideBase & qp, idx flag)
{
    qp.reqSetData(qp.grpId, g_run_qc_name, "%" DEC, flag);
}

idx dmArchiver::getQCFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_run_qc_name, &lvar);
    idx dflt = 1;
    if(lvar) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}

void dmArchiver::setScreenFlag(sQPrideBase & qp, idx flag)
{
    qp.reqSetData(qp.grpId, g_run_screenname, "%" DEC, flag);
}

void dmArchiver::setAnonymizeFlag(sQPrideBase & qp, idx flag)
{
    qp.reqSetData(qp.grpId, g_run_anonymize, "%" DEC, flag);
}

void dmArchiver::setCompressFlag(sQPrideBase & qp, idx flag)
{
    qp.reqSetData(qp.grpId, g_run_compressor, "%" DEC, flag);
}

void dmArchiver::setAutopermFlag(sQPrideBase & qp, idx flag)
{
    qp.reqSetData(qp.grpId, g_run_autoperm, "%" DEC, flag);
}


idx dmArchiver::getScreenFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_run_screenname, &lvar);
    idx dflt = 1;
    if(lvar) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}


idx dmArchiver::getCompressFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_run_compressor, &lvar);
    idx dflt = 1;
    if(lvar) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}

idx dmArchiver::getAutopermFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_run_autoperm, &lvar);
    idx dflt = 1;
    if(lvar) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}

idx dmArchiver::getAnonymizeFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.reqGetData(qp.grpId, g_run_anonymize, &lvar);
    idx dflt = 1;
    if(lvar) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}

bool dmArchiver::convertObj(const sHiveId & objId, const char * typeName)
{
    if( objId && typeName && typeName[0] ) {
        setVar("convertObj", "%s", objId.print());
        setVar("convertTypeName", "%s", typeName);
        return true;
    }
    return false;
}

void dmArchiver::addObjProperty(const char* name, const char * value, ...)
{
    if( name && name[0] && value ) {
        sStr s;
        sCallVarg(s.vprintf, value);
        if( s ) {
            if( m_properties ) {
                m_properties.printf(",");
            }
            m_properties.printf("%s,", name);
            sString::escapeForCSV(m_properties, s, s.length());
            m_properties.printf("\n");
            setVar("properties", "%s", m_properties.ptr());
        }
    }
}

sUsrProc * dmArchiver::makeObj(sUsr& user) const
{
    sUsrProc * p = new sUsrProc(user, "svc-archiver");
    ((sUsrObj*) p)->propSet("svcTitle", "File Processing");
    sStr objname("%s", getVar("convertObj"));
    if( objname ) {
        sHiveId id(objname);
        objname.cut(0);
        sUsrObj * o = user.objFactory(id);
        if( o && o->Id() ) {
            o->propGet("name", &objname);
        }
        delete o;
    }
    if( !objname ) {
        objname.printf(0, "%s", sFilePath::nextToSlash(getVar("inputName")));
    }
    ((sUsrObj*) p)->propSet("name", objname);
    return p;

}
