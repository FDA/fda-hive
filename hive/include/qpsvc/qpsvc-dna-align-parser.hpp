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
#pragma once
#ifndef QPSVC_DnaAlignParser_hpp
#define QPSVC_DnaAlignParser_hpp

#include <qpsvc/qpsvc.hpp>

class QPSvcDnaAlignParser: public sQPSvc
{
        typedef sQPSvc TParent;

    public:

        QPSvcDnaAlignParser(sQPride & qp, const char * file, idx parser_reqid, const sHiveId & objId, const char *subId, const sHiveId &qryId, udx chunkSize, const char * userFilename = 0)
            : TParent(qp), m_fileSize(0), m_maxChunkSize(chunkSize)
        {
            setFile("%s", file);
            setObjId(objId);
            setQryId(qryId);
            setSubId(subId);
            setParserReqid(parser_reqid);
            setUserFilename("%s", userFilename);
        }
        virtual ~QPSvcDnaAlignParser()
        {
        }
        void setFile(const char * file, ...) __attribute__((format(printf, 2, 3)))
        {
            sStr s;
            if( file && file[0] ) {
                sCallVarg(s.vprintf, file);
                m_fileSize = sFile::size(s);
                setVar("sourceSequenceFilePath", "%s", s.ptr());
            }
        }
        void setUserFilename(const char * file, ...) __attribute__((format(printf, 2, 3)))
        {
            if( file && file[0] ) {
                sStr s;
                sCallVarg(s.vprintf, file);
                setVar("userFileName", "%s", s.ptr());
            }
        }
        void setParserReqid(const idx reqid)
        {
            setVar("parser_reqid", "%" DEC, reqid);
        }
        void setObjId(const sHiveId & objId)
        {
            setVar("obj", "%s", objId.print());
        }
        void setQryId(const sHiveId & qryId)
        {
            setVar("qry", "%s", qryId.print());
        }
        void setSubId(const char * subId)
        {
            if( subId) {
                setVar("sub", "%s", subId);
            }
        }
        virtual const char* getSvcName() const
        {
            return "dna-align-parser";
        }

        idx m_fileSize;
        udx m_maxChunkSize;
};

#endif
