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
#ifndef sDnaProfX_hpp
#define sDnaProfX_hpp

#include <slib/std.hpp>
#include <ssci/bio.hpp>
#include <qlib/QPrideProc.hpp>
#include <dmlib/dmlib.hpp>

#include <violin/violin.hpp>
#include <ssci/bio/viosam.hpp>

class DnaProfX
{
    public:

    sQPrideProc * qp;
    sStr workDir, algorithm, resourceRoot;

    virtual idx Profile (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0) {return 0;};
    virtual idx PrepareData ( sUsr& user, const char * parentIDs, const char * workDir, sStr &errMsg) {return 0;}
    virtual idx Finalize (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0) {return 0;};
    virtual ~DnaProfX() {}

    protected:

    const char * getGTFFilePath(sStr &path, const sHiveId & fileid)
    {
        std::auto_ptr < sUsrObj > GTFObj(qp->user->objFactory(fileid));
        if( GTFObj.get() && GTFObj->Id() ) {
            sUsrFile * GTFFileObj = dynamic_cast<sUsrFile*>(GTFObj.get());
            if( GTFFileObj ) {
                GTFFileObj->getFile(path);
            }
        }
        return path.ptr();
    }

};

class DnaProfXsamtools: public DnaProfX
{
    public:

    virtual idx Profile (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0);
    virtual idx PrepareData ( sUsr& user, const char * parentIDs, const char * workDir, sStr &errMsg);
    virtual idx Finalize (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0);


};

class DnaProfXvarscan: public DnaProfX
{
    public:

    virtual idx Profile (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0);
    virtual idx PrepareData ( sUsr& user, const char * parentIDs, const char * workDir, sStr &errMsg);
    virtual idx Finalize (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0);


};

class DnaProfXcuffdiff: public DnaProfX
{
    public:

    virtual idx Profile (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0);
    virtual idx PrepareData ( sUsr& user, const char * parentIDs, const char * workDir, sStr &errMsg);
    //virtual idx Finalize (sIO * log, sStr * outFile, const char * workDir, sUsr& user, const char * parentIDs, const char * additionalCommandLineParameters=0);
};

#endif // DnaProfX

