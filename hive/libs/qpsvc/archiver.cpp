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
#include <qlib/QPrideProc.hpp>

dmArchiver::TKnownTypes dmArchiver::m_knownTypes[] = {
    {
        "hivepack" __,
        dmLib::eNone,
        "hivepack" __,
        0 },
    {
        "nuc-read" __,
        dmLib::eZip,
        "fasta" _ "fa" _ "fas" _ "fsa" _ "fna" _ "fq" _ "fastq" __,
        0 },
    {
        "nuc-read" __,
        dmLib::eNone,
        "vioseq2" _ "hiveseq" __,
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
        dmLib::eZip,
        "ma" __,
        "" _ "status,5\nsubmitter,dna-hexagon&cmdMode=mafft" __ },
    {
        "genome" _ "u-ionAnnot" __,
        dmLib::eZip,
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
        dmLib::eZip,
        "faa" __,
        0 },
    {
        "target_library" __,
        dmLib::eZip,
        "bt2" __,
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
    setDepth(0);
    setQCFlag(true);
    setScreenFlag(false);
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

void dmArchiver::setSubject(const char * subject)
{
    if( subject && subject[0] ) {
        setVar("upload_subject", "%s", subject);
    }
}

static const char * const g_depth_name = "dissect";
static const char * const g_run_qc_name = "run_qc";
static const char * const g_run_screenname = "run_screen";

void dmArchiver::setDepth(udx max_depth)
{
    setVar(g_depth_name, "%" UDEC, max_depth);
}
udx dmArchiver::getDepth(sQPrideBase & qp)
{
    udx dflt = ~0;
    sStr lvar;
    qp.formValue(g_depth_name, &lvar);
    if( lvar ) {
        sscanf(lvar, "%" DEC, &dflt);
    }
    return dflt;
}

void dmArchiver::setQCFlag(const bool flag)
{
    setVar(g_run_qc_name, "%s", flag ? "true" : "false");
}
bool dmArchiver::getQCFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.formValue(g_run_qc_name, &lvar);
    if( lvar ) {
        return sString::parseBool(lvar);
    }
    return false;
}

void dmArchiver::setScreenFlag(const bool flag)
{
    setVar(g_run_screenname, "%s", flag ? "true" : "false");
}
bool dmArchiver::getScreenFlag(sQPrideBase & qp)
{
    sStr lvar;
    qp.formValue(g_run_screenname, &lvar);
    if( lvar ) {
        return sString::parseBool(lvar);
    }
    return false;
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

sUsrProc * dmArchiver::makeObj(sUsr& user, sUsrProc * p) const
{
    sHiveId oid;
    sRC rc = user.objCreate(oid, "svc-archiver");
    if( !rc ) {
        if(p) {
            new(p) sUsrProc(user, oid);
        } else{
            p = new sUsrProc(user, oid);
        }

        if( p ) {
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
        }
    }
    return p;
}
