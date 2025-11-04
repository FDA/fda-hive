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
#include <qlib/QPrideCGI.hpp>
#include <time.h>
#include "common.hpp"

#if _DEBUG
#define DBG(fmt,...) ::fprintf(stderr, "%s:%u: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)
#else
#define DBG(...)
#endif


class UploadProcessorSvc: public sQPSvc
{
        typedef sQPSvc TParent;

    public:
        UploadProcessorSvc(sQPride & qp)
            : TParent(qp)
        {
        }
        ~UploadProcessorSvc()
        {
        }

        virtual const char* getSvcName() const
        {
            return CFGNM();
        }
};

class UploadCGI: public sQPrideCGI
{
    public:
        UploadCGI(const char * defline00, const char * service, idx argc, const char ** argv, const char ** envp, FILE * readfrom, bool isCookie, bool immediate)
            : sQPrideCGI(defline00, service, argc, argv, envp, readfrom, isCookie, immediate),
              m_concurrency(4), m_chunkSize(10485760), m_compress(false)

        {
            m_concurrency = cfgInt(0, CFGNM(.concurrency), m_concurrency);
            m_chunkSize = cfgInt(0, CFGNM(.chunkSize), m_chunkSize);
            m_compress = sString::parseBool(cfgStr(0, 0, CFGNM(.compress) "false"));
        }

        virtual ~UploadCGI()
        {
        }

    protected:
        idx m_concurrency;
        idx m_chunkSize;
        bool m_compress;
        sStr m_uploadPath;

    public:

        virtual idx Cmd(const char * cmd)
        {
            raw = 1;
            if( reqId > 0 && objs.dim() <= 0 ) {
                sStr strObjList;
                requestGetPar(reqId, eQPReqPar_Objects, &strObjList, true);
                if( grpId && strObjList.length() < 2) {
                    strObjList.cut0cut();
                    requestGetPar(grpId, eQPReqPar_Objects, &strObjList, false);
                }
                sVec<sHiveId> objIds;
                if( strObjList ) {
                    sHiveId::parseRangeSet(objIds, strObjList, strObjList.length());
                }
                if(objIds.dim() > 0) {
                    idx cnt = objs.dim();
                    sUsrObj * p = objs.add(1);
                    new (p) sUsrObj(*user, objIds[0]);
                    if( !objs[cnt].Id() ) {
                        objs.cut(cnt);
                    }
                }
            }

            const EUploadStatus status = getUploadStatus(*this);
            switch(status) {
                case eUnknown:
                case eInitialized:
                case eUploading:
                case ePaused:
                    {{
                    const idx stat = reqGetStatus(reqId);
                    switch(stat) {
                        case eQPReqStatus_Any:
                        case eQPReqStatus_Waiting:
                        case eQPReqStatus_Processing:
                        case eQPReqStatus_Running:
                            {{
                                const char * method = getenv("REQUEST_METHOD");
                                if( sIsExactly(method, "GET") ) {
                                    if( sIsExactly(cmd, "config") ) {
                                        response(200);
                                    } else if( sIsExactly(cmd, "init") ) {
                                        initUpload();
                                    } else if( sIsExactly(cmd, "pause") ) {
                                        pauseReq(ePaused);
                                    } else if( sIsExactly(cmd, "resume") ) {
                                        pauseReq(eUploading);
                                    } else {
                                        response(400);
                                    }
                                } else if( sIsExactly(method, "POST") ) {
                                    receiveParts();
                                } else if( sIsExactly(method, "DELETE") ) {
                                    deleteReq();
                                } else if( sIsExactly(method, "PUT") ) {
                                    finishReq();
                                } else {
                                    response(400);
                                }
                            }}
                            break;
                        case eQPReqStatus_Suspended:
                            response(409, "Upload process is suspended by user, please resume to continue");
                            break;
                        case eQPReqStatus_Done:
                            response(409, "Upload process is stopped by user, please restart to continue");
                            break;
                        case eQPReqStatus_ProgError:
                        case eQPReqStatus_SysError:
                            response(409, "Upload process has failed, try again later");
                            break;
                    }
                    }}
                    break;
                case eFinished:
                    response(400, "Upload has already finished");
                    break;
                case eDeleted:
                    response(400, "Upload was deleted");
                    break;
            }
            return 1;
        }

    protected:

        const char * getUploadPath(const bool create = false) {
            if( !m_uploadPath ) {
                sStr tmp;
                if( reqId ) {
                    reqGetData(reqId, uploadPathName, m_uploadPath.mex());
                    if( !sDir::exists(m_uploadPath) ) {
                        DBG("Upload area '%s' is not found: %s", m_uploadPath.ptr(), strerror(errno));
                        m_uploadPath.cut0cut();
                    }
                } else if( create && !sDir::mktemp(m_uploadPath, cfgStr(&tmp, 0, "user.download"), "upload-XXXXXX") ) {
                    m_uploadPath.cut0cut();
                }
            }
            return m_uploadPath.ptr();
        }

        void response(const idx status, const char * error = 0)
        {
            if( status ) {
                headerSet("Status", "%" DEC, status);
            }
            sJSONPrinter printer(&dataForm);
            printer.startObject();
            printer.addKey("reqid");
            printer.addValue(reqId);
            if( error && error[0] ) {
                printer.addKey("error");
                printer.addValue(error);
            } else {
                sStr tmp;
                printer.addKeyValue("IdleTimeoutSec", cfgInt(0, CFGNM(.IdleTimeoutSec)));
                printer.addKeyValue("OnPauseIdleTimeoutSec", cfgInt(0, CFGNM(.OnPauseIdleTimeoutSec)));
                printer.addKeyValue("chunkSize", m_chunkSize);
                printer.addKeyValue("concurrency", m_concurrency);
                printer.addKeyValue("compress", m_compress);
                printer.addKeyValue("reverseFileRegex", cfgStr(&tmp, 0, CFGNM(.reverseFileRegex)));
                const char * v = cfgStr(&tmp, 0, CFGNM(.alignments));
                if( v && v[0] ) {
                    printer.addKey("alignments");
                    printer.addRaw(v);
                }
                v = cfgStr(&tmp, 0, CFGNM(.reference_genomes));
                if( v && v[0] ) {
                    printer.addKey("reference_genomes");
                    printer.addRaw(v);
                }
            }
            printer.finish();
            outHtml();
        }

        void initUpload()
        {
            const char * err = "Uploading is not available at this time, try again later.";
            if( getUploadPath(true) ) {
                reqId = launchTool(*pForm);
                if( !reqId ) {
                    err = "Request ID was not assaigned. Uploading is not available at this time, try again later.";
                }
                if( !reqId || !reqSetData(reqId, uploadPathName, "%s", m_uploadPath.ptr()) || !setUploadStatus(*this, eInitialized) ) {
                    setUploadStatus(*this, eDeleted);
                } else {
                     err = 0;
                }
            }
            response(err ? 418 : 200, err);
        }

        void pauseReq(const EUploadStatus status)
        {
            const char * err = setUploadStatus(*this, status) ? 0 : "Operation failed";
            response(err ? 418 : 200, err);
        }

        void receiveParts()
        {

            idx isok = 412;
            const char * uparea = getUploadPath();
            if( uparea ) {
                const char * pvar = getenv("CONTENT_LENGTH");
                const char * pct = getenv("CONTENT_TYPE");
                const char * pboundary = pct;
                sStr boundary("--");
                if( pct && sIs("multipart/form-data;", pct) &&
                    (pboundary = sString::searchSubstring(pct, 0, "boundary=" __, 1, __, true)) && sLen(pboundary + 9) ) {
                        pboundary += 9;
                        sString::copyUntil(&boundary, pboundary, sLen(pboundary), " \r\n" __);
                        pboundary = 0;
                }
                if( boundary && pvar && *pvar ) {
                    udx contentLength = 0;
                    sscanf(pvar, "%" UDEC, &contentLength);
                    sStr tmp;
                    sFile::mktemp(tmp, uparea, "tmp", TMP_PREFIX "XXXXXX");
                    sMex pst;
                    pst.init(tmp.ptr());
                    if( pst.ok() ) {
                        pst.readIO(stdin);
                        if( (udx) pst.pos() != contentLength ) {
                            DBG("body length %" DEC " != CONTENT_LENGTH %" DEC, pst.pos(), contentLength);
                            contentLength = 0;
                        }
                        if( contentLength && !sIs(boundary.ptr(), (const char *)pst.ptr()) ) {
                            DBG("body is starting with '%.*s' and is not boundary %s", (int)sLen(boundary), (char *)pst.ptr(), boundary.ptr());
                            contentLength = 0;
                        }
                        pst.destroy();
                        if( contentLength ) {
                            sFilePath dst(tmp, "%%pathx" PART_SUFFIX);
                            if( sFile::rename(tmp, dst) ) {
                                if( setUploadStatus(*this, eUploading) ) {
                                    isok = 200;
                                } else {
                                    sFile::remove(dst);
                                }
                            } else { 
                                DBG("can't move %s -> %s: %s", tmp.ptr(), dst.ptr(), strerror(errno));
                            }
                        }
                        else {
                            sFile::remove(tmp);
                        }
                    } else {
                        DBG("bad tmp file %s", tmp.ptr());
                    }
                    if( isok != 200 ) {
                        pst.destroy();
                        sFile::remove(tmp);
                    }
                } else {
                    isok = 400;
                }
            }
            response(isok, isok == 200 ? 0 : "Block upload failed");
        }

        void deleteReq()
        {
            const char * err = "Upload not found";
            if( getUploadPath() && setUploadStatus(*this, eDeleted) ) {
                err = 0;
            }
            response(err ? 404 : 200, err);
        }

        void finishReq()
        {
            if( !getUploadPath() || !setUploadStatus(*this, eFinished) ) {
                response(418, "Upload unfinished");
            } else {
                sJSONPrinter printer(&dataForm);
                printer.startObject();
                if( objs.dim() ) {
                    printer.addKeyValue("id", objs[0].IdStr());
                    sStr url("cmd=pipeline&type=%s&id=%s", pipelineType, objs[0].IdStr());
                    printer.addKeyValue("goto", url.ptr());
                }
                printer.finish();
                outHtml();
            }
        }

        const char * printFormKey(const sUsrType2 & type, const char * name, const char * path = 0)
        {
            static sStr keybuf;
            keybuf.printf(0, "prop.%s.%s", type.name(), name);
            if( path && sLen(path) ) {
                keybuf.printf(".%s", path);
            }
            return keybuf.ptr();
        }

        bool getReqProperties(sVar & form, sVar & reqForm)
        {
            sStr tq("^%s$", pipelineType);
            sVec<const sUsrType2 *> types;
            sUsrType2::find(*user, &types, tq.ptr());
            const sUsrType2 * type = types.dim() ? types[0] : 0;
            if( !type ) {
                DBG("Cannot load type %s fields", tq.ptr());
                return false;
            }
            sVec<const sUsrTypeField*> fields;
            type->getFields(*user, fields);
            sVar paths;
            rebuildFlatTree(fields, 17, paths);
            sStr pfx("prop.%s.", pipelineType);
            const idx pfx_pos = pfx.pos();
            for(idx ifield = 0; ifield < fields.dim(); ++ifield) {
                const sUsrTypeField * field = fields[ifield];
                const char * fname = field->name();
                pfx.printf(pfx_pos, "%s", fname);
                const char * nm = printFormKey(*type, fname, paths[fname]);
                for(idx j = 0; j < reqForm.dim(); ++j) {
                    const char * fnm = (const char *)reqForm.id(j);
                    if( sIs(pfx, fnm) ) {
                        form.inp(nm, reqForm.value(fnm));
                        fname = 0;
                    }
                }
                if( fname ) {
                    sStr buf;
                    const char * dvalue = field->defaultValue();
                    if( sIsExactly(fname, uploadPathName) ) {
                        dvalue = buf.printf(0, "%s/", sFilePath::nextToSlash(m_uploadPath.ptr()));
                    }
                    if( dvalue && *dvalue && !sIs("eval:", dvalue) ) {
                        form.inp(nm, dvalue);
                    }
                }
            }
            form.inp(printFormKey(*type, "_type"), type->name());
            form.inp(printFormKey(*type, "svc"), "pipeline");
            if( const udx projID = user->getProject() ) {
                sStr sp("%" UDEC, projID);
                form.inp("projectID", sp.ptr());
            }
            return true;
        }

        idx launchTool(sVar & reqForm)
        {
            sVar form;
            if( getReqProperties(form, reqForm) ) {
                sStr log;
                sStr strObjList;
                sVec<sUsrProc> procObjs;

                sQPride::Service Svc;
                idx err = sUsrProc::createProcesForsubmission(this, &form, user, procObjs, &Svc, &strObjList, &log);
                if( err ) {
                    DBG("Failed to create process: %s", log.ptr());
                    return 0;
                }
                idx cntParallel = sHiveProc::customizeSubmission(&form, user, &procObjs[0], &Svc, &log);
                if( !cntParallel ) {
                    DBG("Failed to customize submission: %s", log.ptr());
                    return 0;
                }
                err = sUsrProc::standardizedSubmission(this, &form, user, procObjs, cntParallel, &reqId, &Svc, 0, &strObjList, &log, 0, true);
                if( err ) {
                    DBG("Failed to submit process: %s", log.ptr());
                    return 0;
                }
            }
            return reqId;
        }

        void rebuildChild(sVec<const sUsrTypeField*> & fields, sStr & group, sVar & paths)
        {
            sStr dummy, dummy2;
            for(idx ifield = 0; ifield < fields.dim(); ++ifield) {
                dummy.printf(0, "%s.%" DEC "", group.ptr(), ifield);
                paths.inp(fields[ifield]->name(), dummy.ptr());
                if( fields[ifield]->dimChildren() > 0 ) {
                    dummy2.printf(0, "%s", dummy.ptr());
                    sVec<const sUsrTypeField*> children;
                    fields[ifield]->getChildren(children);
                    rebuildChild(children, dummy2, paths);
                }
            }
        }

        void rebuildFlatTree(sVec<const sUsrTypeField*> & fields, idx bsIndex, sVar & paths)
        {
            sStr group;
            for(idx ifield = 0; ifield < fields.dim(); ++ifield) {
                const sUsrTypeField * field = fields[ifield];
                if( !field->parent() ) {
                    group.printf(0, "%" DEC, bsIndex++);
                    paths.inp(field->name(), group.ptr());
                    if( field->dimChildren() > 0 ) {
                        sVec<const sUsrTypeField*> children;
                        field->getChildren(children);
                        rebuildChild(children, group, paths);
                    }
                }
            }
        }
};

int main(int argc, const char * argv[], const char * envp[])
{
    const char * method = getenv("REQUEST_METHOD");
    if( method && !sIsExactly(method, "GET") ) {
        putenv((char*) "REQUEST_METHOD=GET");
    }
    
    UploadCGI qapp("config=qapp.cfg" __, "upload", argc, argv, envp, stdin, true, true);

    if( method ) {
        if( sIsExactly(method, "POST") ) {
            putenv((char*) "REQUEST_METHOD=POST");
        } else if( sIsExactly(method, "PUT") ) {
            putenv((char*) "REQUEST_METHOD=PUT");
        } else if( sIsExactly(method, "DELETE") ) {
            putenv((char*) "REQUEST_METHOD=DELETE");
        }
    }
    qapp.run();
    return 0;
}
