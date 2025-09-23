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
#ifndef dmCompressor_hpp
#define dmCompressor_hpp

#include <qpsvc/qpsvc.hpp>
#include <xlib/dmlib.hpp>

class dmCompressor: public sQPSvc
{
        typedef sQPSvc TParent;
    public:

        dmCompressor(sQPride & qp, const char * file, const sHiveId & objId, dmLib::EPackAlgo algo);
        dmCompressor(sQPride & qp, sVec<sHiveId> & objs, const char * hivepack_name);
        dmCompressor(sQPride & qp, const char * query, const char * hivepack_name);
        dmCompressor(sQPride & qp, sVec<sHiveId> & objs, const char * mask, const char * arc_name, dmLib::EPackAlgo algo = dmLib::eZip);
        dmCompressor(sQPride & qp, const char * query, const char * mask, const char * arc_name, dmLib::EPackAlgo algo = dmLib::eZip);
        virtual ~dmCompressor();

        virtual const char * getSvcName() const
        {
            return "dmCompressor";
        }

        void setFunction(const char * function);

        void setFile(const char * file, ...) __attribute__((format(printf, 2, 3)));
        void setCompression(dmLib::EPackAlgo algo);

        void setContainerName(const char * filename, ...) __attribute__((format(printf, 2, 3)));
        void setObjs(const sHiveId & objId, const bool with_dependences = true);
        void setObjs(sVec<sHiveId> & objs, const bool with_dependences = true);
        void setObjs(const char * query);
        void setFiles2Mask(const char * mask);
};

#endif 