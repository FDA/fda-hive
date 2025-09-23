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
#include <qpsvc/compressor.hpp>

#include <slib/std/string.hpp>

dmCompressor::dmCompressor(sQPride& qp, const char * file, const sHiveId & objId, dmLib::EPackAlgo algo)
    : TParent(qp)
{
    setFunction("objSetFile");
    setFile("%s", file);
    setObjs(objId);
    setCompression(algo);
}

dmCompressor::dmCompressor(sQPride & qp, sVec<sHiveId> & objs, const char * hivepack_name)
    : TParent(qp)
{
    setFunction("objHivePack");
    setObjs(objs);
    setContainerName(hivepack_name);
}

dmCompressor::dmCompressor(sQPride & qp, const char * query, const char * hivepack_name)
    : TParent(qp)
{
    setFunction("objHivePack");
    setObjs(query);
    setContainerName(hivepack_name);
}

dmCompressor::dmCompressor(sQPride & qp, sVec<sHiveId> & objs, const char * mask, const char * arc_name, dmLib::EPackAlgo algo)
    : TParent(qp)
{
    setFunction("objFiles2");
    setObjs(objs);
    setFiles2Mask(mask);
    setContainerName(arc_name);
    setCompression(algo);
}

dmCompressor::dmCompressor(sQPride & qp, const char * query, const char * mask, const char * arc_name, dmLib::EPackAlgo algo)
    : TParent(qp)
{
    setFunction("objFiles2");
    setObjs(query);
    setFiles2Mask(mask);
    setContainerName(arc_name);
    setCompression(algo);
}

dmCompressor::~dmCompressor()
{
}

void dmCompressor::setFunction(const char * function)
{
    if( function ) {
        setVar("function", "%s", function);
    }
}

void dmCompressor::setFile(const char * file, ...)
{
    if( file ) {
        sStr s;
        s.cut0cut();
        sCallVarg(s.vprintf, file);
        setVar("inputFile", "%s", s.ptr());
    }
}

void dmCompressor::setCompression(dmLib::EPackAlgo algo)
{
    setVar("compression", "%u", algo);
}

void dmCompressor::setContainerName(const char * filename, ...)
{
    if( filename ) {
        sStr s;
        s.cut0cut();
        sCallVarg(s.vprintf, filename);
        setVar("containerName", "%s", s.ptr());
    }
}

void dmCompressor::setObjs(const sHiveId & objId, const bool with_dependences)
{
    if( objId ) {
        if( with_dependences ) {
            sVec<sHiveId> objs;
            sHiveId * c = objs.add(1);
            if( c ) {
                *c = objId;
                setObjs(objs, true);
            }
        } else {
            setVar("objs", "%s", objId.print());
        }
    }
}

void dmCompressor::setObjs(sVec<sHiveId> & objs, const bool with_dependences)
{
    if( objs.dim() ) {
        sStr s;
        sHiveId::printVec(s, objs);
        setVar("withDependencies", with_dependences ? "true" : "false");
        setVar("objs", "%s", s.ptr());
    }
}

void dmCompressor::setObjs(const char * query)
{
    if( query) {
        setVar("objs", "%s%s", sIs("query://", query) ? "" : "query://", query);
    }
}

void dmCompressor::setFiles2Mask(const char * mask)
{
    if( mask) {
        setVar("files2mask", "%s", mask);
    }
}
