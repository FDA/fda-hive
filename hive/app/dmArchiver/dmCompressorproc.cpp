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
#include <xlib/dmlib.hpp>
#include <ssql/mysql.hpp>
#include <ulib/uobj.hpp>
#include <ulib/usr.hpp>
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

            eQPReqStatus onObjHivePack(void)
            {
                sStr err;
                sVec<sHiveId> vobjs;
                objIds(vobjs, err);
                if( vobjs.dim() && user && objs.dim() ) {
                    const char * flnm = formValue("containerName", 0, 0);
                    sStr filenm;
                    if( flnm && flnm[0] ) {
                        sDir::cleanUpName(flnm, filenm, false);
                        filenm.cut(-1);
                        filenm.printf(".hivepack");
                    } else {
                        filenm.printf("objects.hivepack");
                    }
                    sStr dst;
                    objs[0].addFilePathname(dst, true, "%s", filenm.ptr());
                    if( dst ) {
                        sRC rc = user->objHivepack(vobjs, dst, reqProgressFSStatic, this);
                        rc.print(&err);
                    } else {
                        err.printf("failed to save file");
                    }
                } else {
                    err.printf("empty object list");
                }
                if( err ) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "%s", err.ptr());
                    return eQPReqStatus_ProgError;
                } else {
                    selfDestruct();
                }
                reqProgress(-1, 100, 100);
                return eQPReqStatus_Done;
            }

            eQPReqStatus onObjFiles2(void)
            {
                sStr err, mask;
                formValue("files2mask", &mask);
                if( !mask ) {
                    err.printf("file mask not set");
                } else {
                    sVec<sHiveId> vobjs;
                    objIds(vobjs, err);
                    if( vobjs.dim() && user ) {
                        const char * flnm = formValue("containerName", 0, "download");
                        sStr dst, wdir;
                        if( objs.dim() ) {
                            objs[0].addFilePathname(dst, true, "~tmp.%s.tmp", flnm);
                        } else {
                            reqAddFile(dst, "cgi_output");
                        }
                        if( dst ) {
                            cfgStr(&wdir, 0, "qm.tempDirectory");
                            if( wdir ) {
                                wdir.printf("%" DEC "/", reqId);
                                if( sDir::makeDir(wdir) ) {
                                    logOut(eQPLogType_Debug, "wdir %s", wdir.ptr());
                                    sVarSet v;
                                    if( user->objFilesExport(vobjs, v, wdir, mask) ) {
                                        if( v.rows ) {
                                            reqProgress(-1, 10, 100);
                                            const dmLib::EPackAlgo compressionAlgo = getAlgo(dmLib::eZip);
                                            sStr log, msg, outName;
                                            const bool pack = dmLib::pack(wdir, dst, compressionAlgo, &log, &msg, &outName, reqProgressFSStatic, this, svc.lazyReportSec / 3, false);
                                            if( pack ) {
                                                reqProgress(-1, 99, 100);
                                                if( objs.dim() ) {
                                                    sFilePath withext(outName, "%s%s%%ext", flnm, flnm[strlen(flnm) - 1] == '.' ? "" : ".");
                                                    sStr dst2;
                                                    if( !objs[0].addFilePathname(dst2, true, "%s", withext.ptr()) || !sFile::rename(outName, dst2) ) {
                                                        err.printf("failed to save file");
                                                    }
                                                    logOut(eQPLogType_Debug, "result %s", dst2.ptr());
                                                } else {
                                                    if( !sFile::rename(outName, dst) ) {
                                                        err.printf("failed to save file");
                                                    }
                                                    logOut(eQPLogType_Debug, "result %s", dst.ptr());
                                                }
                                            } else {
                                                err.printf("compression failed");
                                            }
                                            if( err) {
                                                sFile::remove(dst);
                                                sFile::remove(outName);
                                            }
                                        } else {
                                            err.printf("No file were found for mask '%s'", mask.ptr());
                                        }
                                    } else {
                                        err.printf("file list preparation failed");
                                    }
#if !_DEBUG
                                    sDir::removeDir(wdir);
#endif
                                } else {
                                    err.printf("failed to create temporary space");
                                }
                            } else {
                                err.printf("missing configuration");
                            }
                        } else {
                            err.printf("failed to save file");
                        }
                    } else {
                        err.printf("empty object list");
                    }
                }
                if( err ) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "%s", err.ptr());
                    return eQPReqStatus_ProgError;
                } else {
                    selfDestruct();
                }
                reqProgress(-1, 100, 100);
                return eQPReqStatus_Done;
            }

            eQPReqStatus onObjSetFile(void)
            {
                std::unique_ptr<sUsrFile> obj;
                bool success = false;
                do {
#if _DEBUG
                    for(idx i = 0; i < pForm->dim(); ++i) {
                        logOut(eQPLogType_Info, "form: '%s'='%s'\n", (const char *)pForm->id(i), pForm->value((const char*)(pForm->id(i))));
                    }
#endif
                    sStr err;
                    sVec<sHiveId> vobjs;
                    objIds(vobjs, err);
                    if( err || vobjs.dim() != 1 ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Error, "%s", err ? err.ptr() : "must be exactly one object");
                        break;
                    }
                    std::unique_ptr<sUsrObj> obj1;
                    obj1.reset(user->objFactory(vobjs[0]));
                    if( !obj1.get() || !obj1->Id() ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Error, "Object %s not found or access denied", vobjs[0].print());
                        break;
                    }
                    obj.reset(dynamic_cast<sUsrFile*>(obj1.get()));
                    if( !obj.get() || !obj->Id() ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Error, "Object %s is not a file type", vobjs[0].print());
                        break;
                    }
                    obj1.release();
                    logOut(eQPLogType_Info, "Using object %s", obj->Id().print());
                    reqProgress(-1, 1, 100);
                    sStr inputFilePathName("%s", pForm->value("inputFile")), sobj;
                    if( !inputFilePathName ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Error, "Missing file name");
                        break;
                    }
                    const dmLib::EPackAlgo compressionAlgo = getAlgo(dmLib::eUnspecified);
                    dmLib::EPackAlgo useAlgo = compressionAlgo;
                    const udx osz = obj->propGetU("size");
                    if( compressionAlgo == dmLib::eUnspecified && osz > (300L * 1024 * 1024) ) {
                        useAlgo = dmLib::eZip;
                    }
                    sStr tmpName, outName;
                    obj->addFilePathname(tmpName, true, "~compressing_%s", sFilePath::nextToSlash(inputFilePathName));
                    sStr log, msg;
                    bool pack = dmLib::pack(inputFilePathName, tmpName, useAlgo, &log, &msg, &outName, reqProgressFSStatic, this, svc.lazyReportSec / 3);
                    udx csz = sFile::size(outName);
                    if( !pack || (compressionAlgo == dmLib::eUnspecified && ((osz - csz) < (200L * 1024 * 1024))) ) {
                        logOut(eQPLogType_Error, "%s, stored original", pack ? "Compression inefficient" : "Compression failed");
                        csz = 0;
                        sFile::remove(outName);
                        outName.printf(0, "%s", inputFilePathName.ptr());
                    }
                    logOut(pack ? eQPLogType_Info : eQPLogType_Error, "%s", log.ptr());
                    if( msg ) {
                        reqSetInfo(reqId, pack ? eQPInfoLevel_Info : eQPInfoLevel_Error, "%s", msg.ptr());
                    }
                    reqProgress(-1, 50, 100);
                    if( !user->updateStart() ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Error, "Failed to initiate transaction");
                        break;
                    }
                    if( !obj->setFile_donotuse(outName, obj->propGet("name"), 0, osz) ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Error, "FATAL: File assignment failed '%s' to object %s", outName.ptr(), obj->Id().print());
                        break;
                    }
                    if( osz != csz && csz > 0 ) {
                        obj->propSetI("compressed-size", csz);
                    }
                    reqProgress(-1, 60, 100);
                    sMD5 md5(inputFilePathName);
                    obj->propSet("md5", md5.sum());
                    reqProgress(-1, 80, 100);
                    success = true;
                } while( false );

                if( !user->updateComplete() ) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Failed to commit transaction");
                }
                if( obj.get() ) {
                    if( !success ) {
                        obj->purge();
                    } else {
                        obj->cleanup();
                        reqProgress(-1, 100, 100);
                    }
                }
                return success ? eQPReqStatus_Done : eQPReqStatus_ProgError;
            }

            virtual idx OnExecute(idx)
            {
                const char * func = formValue("function");
                eQPReqStatus reqStatus = eQPReqStatus_ProgError;
                if( !func || sIsExactly(func, "objSetFile") ) {
                    reqStatus = onObjSetFile();
                } else if( sIsExactly(func, "objHivePack") ) {
                    reqStatus = onObjHivePack();
                } else if( sIsExactly(func, "objFiles2") ) {
                    reqStatus = onObjFiles2();
                } else {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Unknown function '%s'", func ? func : "");
                }
                reqSetStatus(reqId, reqStatus);
                return 0;
            }

        private:

            dmLib::EPackAlgo getAlgo(const dmLib::EPackAlgo dflt)
            {
                const dmLib::EPackAlgo algo = (dmLib::EPackAlgo) formUValue("compression", dflt);
                return algo < dmLib::eUnspecified || algo >= dmLib::eMax ? dmLib::eUnspecified : algo;
            }

            bool objIds(sVec<sHiveId> & vobjs, sStr & err)
            {
                sStr pobjs_buf;
                const char * pobjs = formValue("objs", &pobjs_buf);
                const bool with_dependences = formBoolValue("withDependencies", false);

                if( !pobjs ) {
                    err.printf("Missing id(s)");
                } else {
                    if( with_dependences && !sIs("query://", pobjs) ) {
                        pobjs_buf.add0(2);
                        const idx pos = pobjs_buf.length();
                        pobjs_buf.printf("query://([\"");
                        sString::searchAndReplaceSymbols(&pobjs_buf, pobjs, 0, ";,\n\r ", "\",\"", 0, true, true, true, true);
                        pobjs_buf.shrink00();
                        pobjs_buf.printf("\"] as objlist).allusedby({recurse:true,with_topic:true})");
                        pobjs = pobjs_buf.ptr(pos);
                    }
                    if( sIs("query://", pobjs) && user ) {
                        sUsrQueryEngine * qengine = queryEngineFactory();
                        sStr lerr;
                        const char * qry_txt = pobjs + 8;
                        if( !qengine->parse(qry_txt, 0, &lerr) ) {
                            err.printf("parse error: '%s', query %s", lerr.ptr(), pobjs);
                        } else {
                            lerr.cut0cut();
                            sVariant * v = qengine->run(&lerr);
                            if( lerr ) {
                                err.printf("parse execution: '%s', query %s", lerr.ptr(), qry_txt);
                            } else if( v ) {
                                v->asHiveIds(vobjs);
                            }
                        }
                        delete qengine;
                    } else if( !sHiveId::parseRangeSet(vobjs, pobjs) ) {
                        err.printf("invalid id(s) '%s'", pobjs);
                    }
                }
                return err;
            }

            void selfDestruct(void)
            {
                sStr cfgvar("%s.selfExpirationDays", svc.name ? svc.name : "dmCompressor");
                const udx days = cfgInt(0, cfgvar);
                if( days > 0 && objs.dim() ) {
                    reqSetInfo(reqId, eQPInfoLevel_Warning, "Object will be automatically removed from the system in %" UDEC " days", days);
                    objs[0].actDelete(days);
                }
            }
    };
}

int main(int argc, const char * argv[])
{
    sApp::args(argc, argv);
    sStr tmp;
    sQPrideProc::QPrideSrvName(&tmp, "dmCompressor", argv[0]);
    dmCompressorProc backend("config=qapp.cfg" __, tmp);
    return (int) backend.run(argc, argv);
}
