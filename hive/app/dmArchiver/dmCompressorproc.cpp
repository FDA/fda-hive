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
#include <slib/std.hpp>
#include <slib/utils.hpp>
#include <qlib/QPrideProc.hpp>
#include <dmlib/dmlib.hpp>
#include <ssql/mysql.hpp>
#include <ulib/ulib.hpp>
#include <xlib/xlib.hpp>
#include <violin/violin.hpp>
#include <violin/hiveseq.hpp>
#include <qpsvc/archiver.hpp>

namespace slib {
    class dmCompressorProc: public sQPrideProc
    {
        public:
            dmCompressorProc(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv)
            {
            }
            virtual idx OnExecute(idx);
    };
}

using namespace slib;

idx dmCompressorProc::OnExecute(idx req)
{
    std::auto_ptr<sUsrFile> obj;
    bool success = false;
    do {
#if _DEBUG
        for(idx i = 0; i < pForm->dim(); ++i) {
            logOut(eQPLogType_Info, "form: '%s'='%s'\n", (const char *)pForm->id(i), pForm->value((const char*)(pForm->id(i))));
        }
#endif
        sHiveId objId(formValue("obj"));
        std::auto_ptr<sUsrObj> obj1;
        obj1.reset(user->objFactory(objId));
        if( !obj1.get() || !obj1->Id() ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Object %s not found or access denied\n", objId.print());
            break;
        }
        obj.reset(dynamic_cast<sUsrFile*>(obj1.get()));
        if( !obj.get() || !obj->Id() ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Object %s is not a file type\n", objId.print());
            break;
        }
        obj1.release();
        logOut(eQPLogType_Info, "Using object %s\n", obj->Id().print());
        reqProgress(-1, 1, 100);
        sStr inputFilePathName, sobj;
        formValue("inputFile", &inputFilePathName);
        if( !inputFilePathName ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Missing file name\n");
            break;
        }
        const dmLib::EPackAlgo compressionAlgo = (dmLib::EPackAlgo) formUValue("compression", dmLib::eNone);
        dmLib::EPackAlgo useAlgo = compressionAlgo;
        const udx osz = obj->propGetU("size");
        if( compressionAlgo == dmLib::eUnspecified && osz > (300L * 1024 * 1024) ) { // compress anything above 100MB
            useAlgo = dmLib::eZip;
        }
        // copy the file to its destination w/optional compression
        sStr tmpName, outName;
        obj->addFilePathname(tmpName, true, "~compressing_%s", sFilePath::nextToSlash(inputFilePathName));
        sStr log, msg;
        bool pack = dmLib::pack(inputFilePathName, tmpName, useAlgo, &log, &msg, &outName, reqProgressFSStatic, this, svc.lazyReportSec / 3);
        udx csz = sFile::size(outName);
        if( !pack || (compressionAlgo == dmLib::eUnspecified && ((osz - csz) < (200L * 1024 * 1024))) ) {
            logOut(eQPLogType_Error, "%s, stored original\n", pack ? "Compression inefficient" : "Compression failed");
            csz = 0;
            sFile::remove(outName);
            outName.printf(0, "%s", inputFilePathName.ptr());
        }
        logOut(pack ? eQPLogType_Info : eQPLogType_Error, "%s\n", log.ptr());
        if( msg ) {
            reqSetInfo(req, pack ? eQPInfoLevel_Info : eQPInfoLevel_Error, "%s", msg.ptr());
        }
        reqProgress(-1, 50, 100);
        if( !user->updateStart() ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Failed to initiate transaction\n");
            break;
        }
        if( !obj->setFile(outName, obj->propGet("name"), strchr(sFilePath::nextToSlash(outName), '.'), osz) ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "FATAL: File assignment failed '%s' to object %s\n", outName.ptr(), obj->Id().print());
            break;
        }
        if( osz != csz && csz > 0 ) {
            obj->propSetI("compressed-size", csz);
        }
        reqProgress(-1, 60, 100);
        sMD5 md5(inputFilePathName);
        obj->propSet("md5", md5.sum);
        if( !user->updateComplete() ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Failed to commit transaction\n");
            break;
        }
        reqProgress(-1, 80, 100);
        success = true;
    } while( false );

    if( obj.get() ) {
        if( !success ) {
            obj->purge();
        } else {
            obj->cleanup();
            reqProgress(-1, 100, 100);
        }
    }
    reqSetStatus(req, success ? eQPReqStatus_Done : eQPReqStatus_ProgError);
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future
    dmCompressorProc backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "dmCompressor", argv[0]));
    return (int) backend.run(argc, argv);
}
