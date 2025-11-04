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
#include <slib/utils/multipart_parser.hpp>
#include <slib/std/app.hpp>
#include <ulib/ufolder.hpp>
#include <qlib/QPrideCGI.hpp>
#include <errno.h>
#include <fcntl.h>

using namespace slib;

class HiveCGI: public sQPrideCGI, private sMultipartParser
{

    public:
        HiveCGI(const char * defline00, const char * service, idx argc, const char * * argv, const char * * envp, FILE * readfrom, bool isCookie, bool immediate)
            : sQPrideCGI(defline00, service, argc, argv, envp, readfrom, isCookie, immediate), m_isProp(-1), m_contentRead(0), m_contentLength(0), html_output(false)
        {
            m_curFile.started = 0;
            m_curFile.size = 0;
            m_curFile.fileNo = 0;
        }

        virtual idx Cmd(const char * cmd)
        {
            idx ret = 0;
            if( sIsExactly("usave", cmd) ) {
                ret = propSetEx();
            } else {
                ret = sQPrideCGI::Cmd(cmd);
            }
            return ret;
        }

        int propSetEx()
        {
            html_output = pForm->boolvalue("framed", false);
            raw = html_output ? 0 : 1;
            const char * pvar = getenv("CONTENT_LENGTH");
            if( pvar ) {
                sscanf(pvar, "%" UDEC, &m_contentLength);
            }
            const char * method = getenv("REQUEST_METHOD");
            bool retcode = false;
            if( sIsExactly(method, "POST") ) {
                sStr headers;
                outHeaders(&headers);
                ::printf("%s", headers.ptr());
                if( html_output ) {
                    ::printf("<html>\n<body>\n<pre>\n");
                }

                sStr boundary;
                const char * pct = getenv("CONTENT_TYPE"), *pbndr = pct;
                if( pct && sString::searchSubstring(pct, 0, "multipart/form-data" __, 1, __, true) && (pbndr = sString::searchSubstring(pct, 0, "boundary=" __, 1, __, true)) && sLen(pbndr + 9) ) {
                    pbndr += 9;
                    sString::copyUntil(&boundary, pbndr, sLen(pbndr), " \r\n" __);
                    retcode = parse(boundary);
                } else {
                    log(eQPInfoLevel_Debug, "error: %s: '%s'", pbndr ? "Content-type 'multipart/form-data' expected" : "'boundary=' not found", pct ? pct : "");
                }
                log(eQPInfoLevel_Info, "HIVE Received %" DEC " bytes", m_contentRead);
                log(eQPInfoLevel_Info, "Browser Submitted %" DEC " bytes", m_contentLength);
                if( html_output ) {
                    ::printf("</pre>\n");
                    if( !pForm->boolvalue("debug") ) {
                        ::printf("<script>window.close()</script>\n");
                    }
                    ::printf("</body>\n</html>\n");
                }
            }
            if( !retcode ) {
                log(eQPInfoLevel_Error, "Upload failed");
            } else {
                reqSetProgress(reqId, -1, 99);
                sVec<sHiveId> new_ids;
                m_User.propSet(*pForm, dataForm, &new_ids, 0, 0);
                for(idx i = 0; retcode && i < new_ids.dim(); ++i) {
                    sUsrObj * obj = m_User.objFactory(new_ids[i]);
                    if( obj ) {
                        attachToFolder(*obj);
                        if( dynamic_cast<sUsrProc *>(obj) ) {
                            sQPride::Service Svc;
                            obj->propSet("svc", pForm->value("svc"));
                            sVec<sUsrProc> vp;
                            sUsrProc * proc = vp.add(1);
                            new (proc) sUsrProc(m_User, new_ids[i]);
                            sStr strObjList("%s", proc->Id().print());
                            strObjList.add0(2);
                            if( sUsrProc::standardizedSubmission(this, pForm, &m_User, vp, 1, 0, &Svc, 0, &strObjList, 0, 0) == 0 ) {
                                m_uploadProc->propSet("procid", new_ids[i].print());
                                m_uploadProc->actDelete(3);
                                sUsrFolder * trash = sSysFolder::Trash(*user);
                                if( trash ) {
                                    trash->attach(*m_uploadProc.get());
                                    sUsrFolder * inbox = sSysFolder::Inbox(*user);
                                    if( inbox ) {
                                        inbox->detach(*m_uploadProc.get());
                                    }
                                }
                            } else {
                                retcode = false;
                                log(eQPInfoLevel_Error, "Failed to launch process");
                            }
                        }
                        delete obj;
                    }
                }
                reqSetStatus(reqId, retcode ? eQPReqStatus_Done : eQPReqStatus_ProgError);
                reqSetProgress(reqId, -1, 100);
            }
            return 0;
        }

        void log(eQPInfoLevel lvl, const char * fmt, ...)
        {
            if( fmt ) {
                sStr s;
                sCallVarg(s.vprintf, fmt);
#ifdef _DEBUG
                if( html_output ) {
                    ::printf("%s\n", s.ptr());
                }
#endif
                reqSetInfo(reqId, lvl, "%s", s.ptr());
            }
        }

        bool attachToFolder(const sUsrObj & obj)
        {
            static std::auto_ptr<sUsrFolder> folder;
            if( !folder.get() ) {
                sHiveId id(pForm->value("HIVE-user-curdir_save", pForm->value("HIVE-user-curdir_open")));
                folder.reset(id ? new sUsrFolder(*user, id) : sSysFolder::Inbox(*user));
            }
            return folder.get() ? folder->attach(obj) : false;
        }

        const char * getPath()
        {
            static sStr uploadAreaPath;
            if( !uploadAreaPath ) {
                cfgStr(&uploadAreaPath, 0, "user.download", "");
                uploadAreaPath.printf("upload-%" DEC "/", reqId);
                sDir::removeDir(uploadAreaPath, true);
                if( !sDir::makeDir(uploadAreaPath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IWOTH) || !sDir::chDir(uploadAreaPath) ) {
                    log(eQPInfoLevel_Error, "Upload area is not accessible: %s", strerror(errno));
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                    return 0;
                }
            }
            return uploadAreaPath;
        }

        virtual bool on_next_chunk(const char ** buf, udx & len)
        {
            static sStr buffer;
            const idx sizeBuf = 10L * 1024 * 1024;
            if( !buffer) {
                buffer.add(0, sizeBuf);
            } else if( len > 0 ) {
                memmove(buffer, *buf, len);
            }
            clearerr(stdin);
            idx more = fread(buffer.ptr(len), 1, sizeBuf - len, stdin);
            if( ferror(stdin) ) {
                return false;
            }
            len += more;
            m_contentRead += more;
            *buf = buffer;

            const idx prgs = reqProgress(reqId, 2, m_contentRead, m_contentRead, m_contentLength);
            sUsrProc * p = dynamic_cast<sUsrProc *>(m_uploadProc.get());
            if( p ) {
                p->propSync();
            }
            if( prgs == 0 ) {
                log(eQPInfoLevel_Error, "Process interrupted by user");
            }
            return prgs != 0;
        }

        virtual bool on_header_value(const char * buf, udx len)
        {
            if( len ) {
                const char * fnd = 0;
                if( (fnd = sString::searchSubstring(buf, len, "; filename=\"" __, 1, "\n\r" __, true)) != 0 ) {
                    sStr fff;
                    sString::copyUntil(&fff, fnd + 12, len - 12, "\"\n\r");
                    log(eQPInfoLevel_Debug, "Uploading file (orig) %s", fff.ptr());
                    m_curFile.name.printf(0, "%s", sFilePath::nextToSlash(fff));
                    m_curFile.started = time(0);
                    m_curFile.size = 0;
                    m_curFile.buf.cut(0);
                    m_curFile.fileNo = 0;
                    log(eQPInfoLevel_Info, "Uploading file %s", m_curFile.name.ptr());
                }
                if( (fnd = sString::searchSubstring(buf, len, ".prop." __, 1, "\n\r" __, true)) != 0 ||
                    (fnd = sString::searchSubstring(buf, len, "\"prop." __, 1, "\n\r" __, true)) != 0 ) {
                    m_partName.cut0cut();
                    sString::copyUntil(&m_partName, ++fnd, len - 1, "\"\n\r");
                    m_partName.shrink00();
                    m_isProp = abs(m_isProp);
                } else if( (fnd = sString::searchSubstring(buf, len, " name=\"" __, 1, "\n\r" __, true)) != 0 ) {
                    const idx offset = 7;
                    m_partName.cut0cut();
                    sString::copyUntil(&m_partName, fnd + offset, len - offset, "\";\n\r");
                    m_partName.shrink00();
                }
            }
            return sMultipartParser::on_header_value(buf, len);
        }

        virtual bool on_headers_complete()
        {
            if( !m_uploadProc.get() ) {
                if( !reqId ) {
                    sQPrideClient::Service svc;
                    serviceGet(&svc, 0, 0);
                    reqId = grpSubmit(svc.name, 0, 0, 1);
                    if( reqId ) {
                        grpId = req2Grp(reqId);
                        reqSetStatus(reqId, eQPReqStatus_Processing);
                        jobId = jobRegister(svc.name, vars.value("thisHostName"), vars.ivalue("pid"), 0);
                        jobSetReq(jobId, reqId);
                        m_uploadProc.reset(new sUsrProc(*user, "svc-http-post"));
                        udx q = (m_uploadProc.get() && m_uploadProc->Id()) ? 1 : 0;
                        if( q ) {
                            reqSetPar(reqId, sQPrideBase::eQPReqPar_Objects, m_uploadProc->IdStr());
                            q += m_uploadProc->propSetI("reqID", reqId);
                            q += m_uploadProc->propSet("svcTitle", "File Upload");
                            q += m_uploadProc->propSet("svc", svc.name);
                            for(idx i = 0; i < pForm->dim(); ++i) {
                                const char * pnm = (const char*)pForm->id(i);
                                sStr path("1.%" DEC, i);
                                const char * p = path.ptr(), * v = pForm->value(pnm);
                                m_uploadProc->propSet(pnm, &p, &v, 1, true, 00);
                            }
                            attachToFolder(*m_uploadProc);
                        }
                        if( q == 4 ) {
                            reqSetStatus(reqId, eQPReqStatus_Running);
                            reqSetAction(reqId, eQPReqAction_Run);
                        } else {
                            reqSetStatus(reqId, eQPReqStatus_ProgError);
                            reqSetAction(reqId, eQPReqAction_Kill);
                            log(eQPInfoLevel_Error, "Internal error %u (%s)", __LINE__, m_uploadProc->Id().print());
                            m_uploadProc->actDelete();
                            reqId = 0;
                            m_uploadProc.reset(0);
                        }
                    }
                }
            }
            return m_uploadProc.get() ? sMultipartParser::on_headers_complete() : false;
        }

        virtual bool on_part_data(const char * buf, udx len)
        {
            if( m_curFile.name ) {
                if( buf && len ) {
                    m_curFile.buf.add(buf, len);
                    m_curFile.size += len;
                }
                const static idx FILE_BUF_SIZE = 100 * 1024 * 1024;
                if( !buf || m_curFile.buf.pos() >= FILE_BUF_SIZE ) {
                    if( !m_curFile.fileNo ) {
                        const char * path = getPath();
                        if( !path ) {
                            return false;
                        }
                        m_partData.cut0cut();
                        sDir::uniqueName(m_partData, "%s%s", path, m_curFile.name.ptr());
                        log(eQPInfoLevel_Debug, "Destination '%s'", m_partData.ptr());
                        errno = 0;
                        m_curFile.fileNo = open(m_partData, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                        if( m_curFile.fileNo < 0 ) {
                            log(eQPInfoLevel_Error, "Cannot create destination file: %s", strerror(errno));
                            reqSetStatus(reqId, eQPReqStatus_ProgError);
                            return false;
                        }
                        sFile::chmod(m_partData, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                    }
                    idx written = 0, togo = m_curFile.buf.pos();
                    errno = 0;
                    while( togo - written > 0 && errno == 0 ) {
                        written += write(m_curFile.fileNo, m_curFile.buf.ptr(written), togo - written);
                    }
                    if( errno != 0 ) {
                        reqSetStatus(reqId, eQPReqStatus_ProgError);
                        log(eQPInfoLevel_Error, "Cannot write to destination file: %s", strerror(errno));
                        return false;
                    }
                    m_curFile.buf.cut(0);
                }
            } else if( buf && len ) {
                m_partData.add(buf, len);
            }
            return true;
        }

        virtual bool on_part_data_end()
        {
            if( m_curFile.name ) {
                on_part_data(0, 0);
                if( m_curFile.started && m_curFile.size ) {
                    idx elapsed = time(0) - m_curFile.started;
                    elapsed = elapsed ? elapsed : 1;
                    const char * suff = " KMGTPEZY";
                    real spd = m_curFile.size / elapsed;
                    while( spd > 1024 ) {
                        spd /= 1024.0;
                        ++suff;
                    }
                    log(eQPInfoLevel_Info, "Uploaded %s in %" DEC " seconds%s%.2f%cb/s", m_curFile.name.ptr(), elapsed, spd > 0.01 ? " ~" : "", spd > 0.01 ? spd : 0, spd > 0.01 ? *suff : 0);
                }
                log(eQPInfoLevel_Info, "File %s size %" DEC " bytes", m_curFile.name.ptr(), m_curFile.size);
                m_curFile.name.cut0cut();
            }
            if( m_curFile.fileNo ) {
                errno = 0;
                if( close(m_curFile.fileNo) != 0 || errno != 0 ) {
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                    log(eQPInfoLevel_Error, "Cannot close destination file: %s", strerror(errno));
                    return false;
                }
                m_curFile.fileNo = 0;
            }
            const udx dlen = m_partData.length();
            m_partData.add0(2);
            if( m_isProp < 0 ) {
                sStr path("1.%" DEC, ++m_isProp);
                const char * p = path.ptr(), * v = m_partData.ptr();
                m_uploadProc->propSet(m_partName.ptr(), &p, &v, 1, true, 0, &dlen);
            } else {
                while( pForm->value((const char *)m_partName) ) {
                    m_partName.printf("1");
                }
            }
            m_isProp = -abs(m_isProp);
            pForm->inp(m_partName, m_partData.ptr(), dlen + 1);
            m_partName.cut0cut();
            m_partData.cut0cut();
            return true;
        }

    private:

        struct
        {
            sMex buf;
            idx started;
            sStr name;
            udx size;
            int fileNo;
        } m_curFile;

        sStr m_partName, m_partData;
        idx m_isProp;
        std::auto_ptr<sUsrObj> m_uploadProc;
        udx m_contentRead;
        udx m_contentLength;
        bool html_output;
};

int main(int argc, const char *argv[], const char *envp[])
{
    sApp app;

    const char * method = getenv("REQUEST_METHOD");
    char get[] = "REQUEST_METHOD=GET";
    if( method && sIsExactly(method, "POST") ) {
        putenv(get);
    } else {
        method = 0;
    }
    HiveCGI uapp("config=qapp.cfg" __, "dmDownloader", 0, 0, envp, 0, true, true);
    char post[] = "REQUEST_METHOD=POST";
    if( method ) {
        putenv(post);
    }
    return uapp.run();
}
