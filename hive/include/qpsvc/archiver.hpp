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
#ifndef dmArchiver_hpp
#define dmArchiver_hpp

#include <qpsvc/qpsvc.hpp>
#include <dmlib/dmlib.hpp>

class dmArchiver: public sQPSvc
{
        typedef sQPSvc TParent;
    public:

        dmArchiver(sQPride& qp, const char * path, const char * dataSource, const char * formatHint = 0, const char * name = 0);
        virtual ~dmArchiver();

        virtual const char* getSvcName() const
        {
            return "dmArchiver";
        }
        virtual sUsrProc * makeObj(sUsr& user) const;

        // sets physical location to process
        void setInput(const char * path, ...) __attribute__((format(printf, 2, 3)));
        // set name (meta) for the location
        void setInputName(const char * name, ...) __attribute__((format(printf, 2, 3)));
        void setDataSource(const char* ds);
        void setFolder(sUsrFolder & folder);
        void setSubject(const char * subject);
        bool convertObj(const sHiveId & objId, const char * typeName);

        //! only applied when it is object conversion
        /** DataTypes supported:
         * - fasta
         * - fastq
         * - sam
         * - annotation
         */
        void setFormatHint(const char *s);
        void addObjProperty(const char* name, const char * value, ...) __attribute__((format(printf, 3, 4)));
        // file(s) decompression depth
        static void setDepth(sQPrideBase & qp, udx max_depth);
        static udx getDepth(sQPrideBase & qp);
        // file(s) needs to be index
        static void setIndexFlag(sQPrideBase & qp, idx flag);
        static idx getIndexFlag(sQPrideBase & qp);
        // file(s) needs to be QC-ed
        static void setQCFlag(sQPrideBase & qp, idx flag);
        static idx getQCFlag(sQPrideBase & qp);
        // set file(s) needs to be screened
        static void setScreenFlag(sQPrideBase & qp, idx flag);
        static idx getScreenFlag(sQPrideBase & qp);

        typedef struct {
            const char * objType00;
            enum dmLib::EPackAlgo compressor;
            const char * ext00;
            const char * props00; // additional properties to assign
        } TKnownTypes;

        static const TKnownTypes * getKnownTypes(void)
        { return m_knownTypes; }

    protected:

        sStr m_properties;

        static TKnownTypes m_knownTypes[];

};

#endif // dmArchiver_hpp
