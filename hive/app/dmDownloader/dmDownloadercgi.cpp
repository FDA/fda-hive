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
#include <slib/utils/tbl.hpp>
#include <slib/utils/multipart_parser.hpp>
#include <slib/std/app.hpp>
#include <ulib/ufolder.hpp>
#include <qlib/QPrideCGI.hpp>
#include <qpsvc/archiver.hpp>
#include <errno.h>
#include <fcntl.h>


class dmDownloaderCGI: public sQPrideCGI, private sMultipartParser
{

    public:
        dmDownloaderCGI(const char * defline00, const char * service, idx argc, const char * * argv, const char * * envp, FILE * readfrom, bool isCookie, bool immediate)
            : sQPrideCGI(defline00, service, argc, argv, envp, readfrom, isCookie, immediate), m_mode(eNONE), m_arch(*this, 0, 0), m_uploadProc(0), m_contentRead(0), m_contentLength(0)
        {
        }

        ~dmDownloaderCGI()
        {
            delete m_uploadProc;
        }

        virtual idx customizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log, sMex **pFileSlices = 0);


        int Uploader(int argc, const char *argv[], const char *envp[])
        {
            this->OnCGIInit();

            m_logBuf.cut(0);
            outHeaders(&m_logBuf);
            m_logBuf.printf("<pre>\n");
            ::printf("%s", m_logBuf.ptr());
            m_logBuf.cut(0);
            m_fi.fno = -1;
            m_fi.started = 0;
            m_subject.cut0cut();
            m_project.cut0cut();
            m_onBehalf.cut0cut();
            m_folder.cut0cut();
            m_arch.setDepth(0);
            m_arch.setScreenFlag(false);
            m_arch.setQCFlag(false);


            sQPrideClient::Service svc;
            serviceGet(&svc, 0, 0);
            jobId = jobRegister(svc.name, vars.value("thisHostName"), vars.ivalue("pid"), 0);

            const char * pcl = getenv("CONTENT_LENGTH");
            if( pcl ) {
                sscanf(pcl, "%" UDEC, &m_contentLength);
            }
            idx archReqId = 0;
            sStr boundary;
            const char * pct = getenv("CONTENT_TYPE"), *pbndr = pct;
            if( pct && sString::searchSubstring(pct, 0, "multipart/form-data" __, 1, __, true) &&
               (pbndr = sString::searchSubstring(pct, 0, "boundary=" __, 1, __, true)) && sLen(pbndr + 9) ) {
                    pbndr += 9;
                    sString::copyUntil(&boundary, pbndr, sLen(pbndr), " \r\n" __);
            } else {
                m_logBuf.printf("error: %s: '%s'\n", pbndr ? "Content-type 'multipart/form-data' expected" : "'boundary=' not found", pct ? pct : "");
            }
            bool q = parse(boundary);
            if( !q ) {
                m_logBuf.printf("<b>Upload failed</b>\n");
            } else {
                m_arch.setForm(pForm, false);
                m_arch.setInput("%s", m_uploadAreaPath.ptr());
                m_arch.setInputName(m_uploadAreaPath);
                m_arch.setDataSource(m_uploadProc->propGet("uri"));
                m_arch.setSubject(m_subject);
                if( m_uploadProc ) {
                    if( m_onBehalf ) {
                        m_uploadProc->propSet("onUserBehalf", m_onBehalf);
                    }
                    if( m_project ) {
                        m_uploadProc->propSet("submission_project", m_project);
                    }
                    sUsrFolder * fptr = 0;
                    setFolder(*m_uploadProc, &fptr);
                    if( fptr ) {
                        m_arch.setFolder(*fptr);
                        delete fptr;
                    }
                }
                archReqId = m_arch.launch(*user, reqId);
                if( !archReqId ) {
                    m_logBuf.printf("Failed to launch processing\n");
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                } else {
                    reqSetStatus(reqId, eQPReqStatus_Done);
                    m_logBuf.printf("HIVE Received %" DEC " bytes\nBrowser Submitted %" DEC " bytes\n", m_contentRead, m_contentLength);
                }
            }
            m_logBuf.printf("Success %" DEC ",%s!\n", archReqId, m_uploadProc ? m_uploadProc->Id().print() : "0");
            sStr l;
            sString::searchAndReplaceSymbols(&l, m_logBuf.ptr(), m_logBuf.length(), "\n", 0, 0, true, true, false, true);
            for(const char * p = l.ptr(); p; p = sString::next00(p)) {
                reqSetInfo(reqId, archReqId ? eQPInfoLevel_Info : eQPInfoLevel_Error, "%s", p);
            }
            if( archReqId ) {
                reqSetProgress(reqId, -1, 100);
            }
            ::printf("%s</pre>", m_logBuf.ptr());
            return 0;
        }

        bool updateProgress(udx done, udx total)
        {
            idx prgs = reqProgress(reqId, 2, done, done, total);
            sUsrProc * p = dynamic_cast<sUsrProc *>(m_uploadProc);
            if( p ) {
                p->propSync();
            }
            if( !prgs ) {
                m_logBuf.printf("Process interrupted by user\n");
                return true;
            }
            return false;
        }

        sUsrObj * useOrSubmitReq()
        {
            if( !reqId ) {
                reqId = grpSubmit("dmDownloader", 0, 0, 1);
                if( reqId ) {
                    reqSetStatus(reqId, eQPReqStatus_Processing);
                    grpId = req2Grp(reqId);
                    jobSetReq(jobId, reqId);
                    m_uploadProc = new sUsrProc(*user, "svc-download");
                    udx q = (m_uploadProc && m_uploadProc->Id()) ? 1 : 0;
                    if( q ) {
                        reqSetPar(reqId, sQPrideBase::eQPReqPar_Objects, m_uploadProc->IdStr());
                        q += m_uploadProc->propSetI("reqID", reqId);
                        q += m_uploadProc->propSet("datasource", "file://");
                        q += m_uploadProc->propSet("svcTitle", "File Upload");
                        q += m_uploadProc->propSetBool("is_upload", true);
                        setFolder(*m_uploadProc);
                    }
                    if( q == 5 ) {
                        reqSetStatus(reqId, eQPReqStatus_Running);
                        reqSetAction(reqId, eQPReqAction_Run);
                    } else {
                        reqSetStatus(reqId, eQPReqStatus_ProgError);
                        reqSetAction(reqId, eQPReqAction_Kill);
                        m_logBuf.printf("Internal error %u (%s)\n", __LINE__, m_uploadProc->Id().print());
                        reqId = 0;
                        m_uploadProc->actDelete();
                        delete m_uploadProc;
                        m_uploadProc = 0;
                    }
                }
            }
            return m_uploadProc;
        }

        void setFolder(sUsrObj & obj, sUsrFolder ** fptr = 0)
        {
            sHiveId id(m_folder ? m_folder.ptr() : pForm->value("HIVE-user-curdir_save", pForm->value("HIVE-user-curdir_open")));
            sUsrFolder * folder = id ? new sUsrFolder(*user, id) : 0;
            sUsrFolder * inbox = sSysFolder::Inbox(*user);
            if( folder ) {
                if( inbox ) {
                    inbox->detach(obj);
                }
                folder->attach(obj);
                if( fptr ) {
                    * fptr = folder;
                } else {
                    delete folder;
                }
            } else if( inbox ) {
                inbox->attach(obj);
                if( fptr ) {
                    * fptr = inbox;
                } else {
                    delete inbox;
                }

            }
        }

        virtual bool on_next_chunk(const char ** buf, udx & len)
        {
            const idx sizeBuf = 10L * 1024 * 1024;
            if( !m_uploadBuf ) {
                m_uploadBuf.add(0, sizeBuf);
                m_propName.cut(0);
            } else if( len > 0 ) {
                memmove(m_uploadBuf, *buf, len);
            }
            clearerr(stdin);
            idx more = fread(m_uploadBuf.ptr(len), 1, sizeBuf - len, stdin);
            if( ferror(stdin) ) {
                return false;
            }
            len += more;
            m_contentRead += more;
            *buf = m_uploadBuf;
            updateProgress(m_contentRead, m_contentLength);
            return true;
        }

        virtual bool on_header_value(const char * buf, udx len)
        {
            if( len ) {
                const char * fnd = 0;
                if( sString::searchSubstring(buf, len, "form-data; name=\"chkauto_" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eUnpackDepth;
                } else if( sString::searchSubstring(buf, len, "form-data; name=\"qc_" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eQC;
                } else if( sString::searchSubstring(buf, len, "form-data; name=\"screen_" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eScreen;
                } else if( sString::searchSubstring(buf, len, "form-data; name=\"subj_" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eSubject;
                } else if( sString::searchSubstring(buf, len, "form-data; name=\"onbehalf_" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eOnBehalf;
                } else if( sString::searchSubstring(buf, len, "form-data; name=\"folder_" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eFolder;
                } else if( sString::searchSubstring(buf, len, "form-data; name=\"submission_project" __, 1, "\n" __, true) != 0 ) {
                    m_mode = eProject;
                } else if( (fnd = sString::searchSubstring(buf, len, "; filename=\"" __, 1, "\"\n" __, true)) != 0 ) {
                    sStr fff;
                    sString::copyUntil(&fff, fnd + 12, len - 12, "\"\r\n");
                    m_mode = eFILE;
#if _DEBUG
                    m_logBuf.printf("Uploading file (orig) %s\n", fff.ptr());
#endif
                    m_fi.name.printf(0, "%s", sFilePath::nextToSlash(fff));
                    m_logBuf.printf("Uploading file %s\n", m_fi.name.ptr());
                    if( !m_uploadAreaPath ) {
                        cfgStr(&m_uploadAreaPath, 0, "user.download", "");
                        m_uploadAreaPath.printf("upl-%" DEC "/", reqId);
                        if( !sDir::makeDir(m_uploadAreaPath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IWOTH) ) {
                            m_logBuf.printf("Upload area is not accessible: %s\n", strerror(errno));
                            reqSetStatus(reqId, eQPReqStatus_ProgError);
                            return false;
                        }
                    }
                    m_fi.src.printf(" http://%s", m_fi.name.ptr());
                    m_fi.path.cut(0);
                    sDir::uniqueName(m_fi.path, "%s%s", m_uploadAreaPath.ptr(), m_fi.name.ptr());
#if _DEBUG
                    m_logBuf.printf("Destination '%s'\n", m_fi.path.ptr());
#endif
                    m_fi.started = time(0);
                    errno = 0;
                    m_fi.fno = open(m_fi.path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                    if( m_fi.fno < 0 ) {
                        m_logBuf.printf("ERROR: Cannot create destination file: %s\n", strerror(errno));
                        reqSetStatus(reqId, eQPReqStatus_ProgError);
                        return false;
                    }
                    sFile::chmod(m_fi.path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                    m_fi.buf.cut(0);
                    m_propName.printf(", %s", m_fi.name.ptr());
                    if( m_uploadProc->propSet("name", m_propName.ptr(2)) + m_uploadProc->propSet("uri", m_fi.src.ptr(1)) != 2 ) {
                        return false;
                    }
                }
            }
            return sMultipartParser::on_header_value(buf, len);
        }

        virtual bool on_headers_complete()
        {
            if( !m_uploadProc && !useOrSubmitReq() ) {
                return false;
            }
            return sMultipartParser::on_headers_complete();
        }

        virtual bool on_part_data(const char * buf, udx len)
        {
            switch(m_mode) {
                case eNONE:
                    break;
                case eUnpackDepth:
                case eQC:
                case eScreen:
                case eSubject:
                case eOnBehalf:
                case eFolder:
                case eProject:
                    m_partData.add(buf, len);
                    break;
                case eFILE:
                    if( !buf || m_fi.buf.pos() >= (100 * 1024 * 1024) ) {
                        idx written = 0, togo = m_fi.buf.pos();
                        errno = 0;
                        while( togo - written > 0 && errno == 0 ) {
                            written += write(m_fi.fno, m_fi.buf.ptr(written), togo - written);
                        }
                        if( errno != 0 ) {
                            reqSetStatus(reqId, eQPReqStatus_ProgError);
                            m_logBuf.printf("ERROR: Cannot write to destination file: %s\n", strerror(errno));
                            return false;
                        }
                        m_fi.buf.cut(0);
                    }
                    if( buf ) {
                        m_fi.buf.add(buf, len);
                    }
                    break;
            }
            return true;
        }

        virtual bool on_part_data_end()
        {
            udx u;
            idx i;
            switch(m_mode) {
                case eNONE:
                    break;
                case eUnpackDepth:
                    u = 0;
                    if( sString::bufscanf(m_partData, m_partData.last(), "%" DEC, &u) ) {
                        m_arch.setDepth(u);
                        if( u ) {
                            m_logBuf.printf("Upload marked for processing %" DEC "\n", u = ~0 ? -1 : u);
                        }
                    }
                    break;
                case eQC:
                    i = 0;
                    if( sString::bufscanf(m_partData, m_partData.last(), "%" DEC, &i) ) {
                        m_arch.setQCFlag(i);
                        if( i ) {
                            m_logBuf.printf("Upload marked for QC\n");
                        }
                    }
                    break;
                case eScreen:
                    i = 0;
                    if( sString::bufscanf(m_partData, m_partData.last(), "%" DEC, &i) ) {
                        m_arch.setScreenFlag(i);
                        if( i ) {
                            m_logBuf.printf("Upload marked for screening\n");
                        }
                    }
                    break;
                case eSubject:
                    m_subject.printf("%.*s;", (int)m_partData.length(), m_partData.ptr());
                    break;
                case eOnBehalf:
                    m_onBehalf.printf(0, "%.*s", (int)m_partData.length(), m_partData.ptr());
                    break;
                case eFolder:
                    m_folder.printf(0, "%.*s", (int)m_partData.length(), m_partData.ptr());
                    if( m_uploadProc ) {
                        setFolder(*m_uploadProc);
                    }
                    break;
                case eProject:
                    m_project.printf("%.*s", (int)m_partData.length(), m_partData.ptr());
                    if( m_project ) {
                        m_logBuf.printf("Upload associated with project %s\n", m_project.ptr());
                    }
                    break;
                case eFILE:
                    on_part_data(0, 0);
                    errno = 0;
                    if( close(m_fi.fno) != 0 || errno != 0 ) {
                        reqSetStatus(reqId, eQPReqStatus_ProgError);
                        m_logBuf.printf("ERROR: Cannot close destination file: %s\n", strerror(errno));
                        return false;
                    }
                    idx flen = sFile::size(m_fi.path);
                    if( m_fi.started && flen ) {
                        idx elapsed = time(0) - m_fi.started;
                        elapsed = elapsed ? elapsed : 1;
                        m_logBuf.printf("Uploaded in %" DEC " seconds\n", elapsed);
                        const char * suff = " KMGTPEZY";
                        real spd = flen / elapsed;
                        while( spd > 1024 ) {
                            spd /= 1024.0;
                            ++suff;
                        }
                        if( spd > 0.01 ) {
                            m_logBuf.printf("Upload speed ~%.2f%cb/sec\n", spd, *suff);
                        }
                    }
                    m_logBuf.printf("File Size %" DEC " bytes\n", flen);
                    break;
            }
            m_mode = eNONE;
            m_partData.cut0cut();
            return true;
        }

    private:

        enum eModes
        {
            eNONE,
            eFILE,
            eUnpackDepth,
            eQC,
            eScreen,
            eSubject,
            eOnBehalf,
            eFolder,
            eProject
        } m_mode;

        struct {
            int fno;
            sMex buf;
            idx started;
            sStr src;
            sStr name;
            sStr path;
        } m_fi;

        dmArchiver m_arch;
        sStr m_logBuf;
        sStr m_partData;
        sUsrObj * m_uploadProc;
        sStr m_uploadBuf;
        udx m_contentRead;
        udx m_contentLength;
        sStr m_uploadAreaPath;
        sStr m_propName;
        sStr m_subject;
        sStr m_project;
        sStr m_onBehalf;
        sStr m_folder;
};

static void addURI(sUsrObj & obj, const char * uri, const char * wget_opts, const char * filename)
{
    static sStr grp;
    static udx cnt = 0;
    const char * pgrp = grp.printf(0, "100.%" UDEC, ++cnt);
    obj.propSet("uri_data_uri", &pgrp, &uri, 1, true);
    if( wget_opts && wget_opts[0] ) {
        obj.propSet("uri_data_wget", &pgrp, &wget_opts, 1, true);
    }
    if( filename && filename[0] ) {
        obj.propSet("uri_data_filename", &pgrp, &filename, 1, true);
    }
}

idx dmDownloaderCGI::customizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj_proc, sQPride::Service * pSvc, sStr * log, sMex **pFileSlices)
{
    cntParallel = 0;
    sStr errlog, obj_id("0");
    const char * pid = strstr(log->ptr(), "prop.");
    if( pid != 0 ) {
        pid += 5;
        const char * dot = strchr(pid, '.');
        obj_id.printf(0, "%.*s", (int)(dot ? dot - pid : sLen(pid)), pid);
    }
    sUsrObj * obj = obj_proc;
    do {
        if( !obj_proc ) {
            errlog.printf("err.%s._err=Internal error %u", obj_id.ptr(), __LINE__);
            break;
        }
        sStr uri;
        obj->propGet("uri", &uri);
        if( !uri ) {
            errlog.printf("err.%s.uri=URLs and Identifiers list is empty: nothing to do", obj_id.ptr());
            break;
        }
        const char * const schemas = "http" _ "https" _ "ftp" _ "ncbi_nuccore" _ "ncbi_nuccds" _ "sra" _ "genbank" _ "dropbox" __;
        sStr base;
        obj->propGet("baseURL", &base);
        sString::cleanEnds(&base, base, 0, sString_symbolsBlank, true);
        char * baseUrl = base.ptr();
        while( baseUrl && *baseUrl && strchr(sString_symbolsBlank, *baseUrl) ) {
            ++baseUrl;
        }
        char * s = baseUrl ? strstr(baseUrl, "://") : 0;
        if( s ) {
            sStr schema("%.*s", (int)(s - baseUrl), baseUrl);
            idx bu_id = -1;
            sString::compareChoice(schema, schemas, &bu_id, true, 0, true);
            if( bu_id < 0 ) {
                errlog.printf("err.%s.baseURL=baseURL has not a valid schema: '%s://'", obj_id.ptr(), schema.ptr());
                break;
            }
        } else {
            base.cut(0);
            baseUrl = 0;
        }
        const char * datasource = obj->propGet("datasource");
        if( datasource ) {
            idx ds_id = -1;
            sString::compareChoice(datasource, schemas, &ds_id, true, 0, true);
            datasource = (ds_id != -1) ? sString::next00(schemas, ds_id) : 0;
        }
        sTxtTbl tbl;
        tbl.setBuf(&uri);
        tbl.parseOptions().flags = 0;
        tbl.parseOptions().colsep = ",;" sString_symbolsSpace;
        tbl.parse();
        regex_t sra_rgx, sra_rgx_bad, accession_rgx;
        if( regcomp(&sra_rgx_bad, "^[DES]R[X][0-9]+$", REG_EXTENDED | REG_ICASE) != 0 ||
            regcomp(&sra_rgx, "^[DES]R[PRSZ][0-9]+$", REG_EXTENDED | REG_ICASE) != 0 ||
            regcomp(&accession_rgx, "^[A-Z]*[0-9]+\\.?[0-9]*$", REG_EXTENDED | REG_ICASE) != 0 ) {
            errlog.printf("err.%s._err=Internal error %u", obj_id.ptr(), __LINE__);
            break;
        }
        const char * const oname = obj->propGet("name");
        if( !oname || !oname[0] ) {
            sStr name, cell;
            for(idx r = 0; r < tbl.rows(); ++r) {
                for(idx c = 0; c < tbl.cols(); ++c) {
                    if( name.length() >= 128 + sLen(datasource) ) {
                        name.printf("...");
                        break;
                    }
                    cell.cut(0);
                    tbl.printCell(cell, r, c);
                    if( cell ) {
                        const char * flnm = sFilePath::nextToSlash(cell);
                        if( flnm && *flnm ) {
                            const char * f = strrchr(flnm, '=');
                            flnm = (f++ && f[0]) ? f : flnm;
                        } else {
                            flnm = cell;
                        }
                        name.printf(", %s", flnm);
                    }
                }
                if( name.length() >= 128 + sLen(datasource) ) {
                    break;
                }
            }
            cell.printf(0, "%s%s%s", datasource ? datasource : "", datasource && name ? ": " : "", name ? name.ptr(2) : "");
            obj->propSet("name", cell);
        }
        obj->propDel("uri_data_uri", 0, 0);
        obj->propDel("uri_data_wget", 0, 0);
        obj->propDel("uri_data_filename", 0, 0);
        sStr buf, ex_auth("%s", "");
        {{
            sStr ex_login(sMex::fBlockCompact), ex_passwd(sMex::fBlockCompact);
            obj->propGet("external_login", &ex_login);
            obj->propGet00("external_password", &ex_passwd);
            if( ex_login ) {
                ex_auth.printf("\"--user=%s\"", ex_login.ptr());
                if( ex_passwd ) {
                    ex_auth.printf(" \"--password=%s\"", ex_passwd.ptr());
                }
            } else if( ex_passwd ) {
                errlog.printf("err.%s.external_login=Missing user name\n", obj_id.ptr());
                break;
            }
        }}
        for(idx r = 0; r < tbl.rows(); ++r) {
            for(idx c = 0; c < tbl.cols(); ++c) {
                buf.cut(0);
                tbl.printCell(buf, r, c);
                if( !buf ) {
                    continue;
                }
                const char * p = buf;
                const char * schema = strstr(p, "://");
                if( schema ) {
                    sStr sc("%.*s", (int)(schema - p), p);
                    idx sc_id = -1;
                    sString::compareChoice(sc, schemas, &sc_id, true, 0, true);
                    if( sc_id < 0 ) {
                        errlog.printf("err.%s.uri=Invalid URL schema: '%s://'\n", obj_id.ptr(), sc.ptr());
                        continue;
                    }
                    addURI(*obj, p, ex_auth, 0);
                } else {
                    sStr ids;
                    if( (idx)(strspn(p, "0123456789-")) == sLen(p) ) {
                        sVec<idx> range;
                        sDic<idx> uniq;
                        sString::scanRangeSet(p, 0, &range, 0, 0, 0);
                        for(idx i = 0; i < range.dim(); ++i) {
                            idx * p = uniq.set(&range[i], sizeof(range[i]));
                            if( p ) {
                                *p = range[i];
                            }
                        }
                        for(idx i = 0; i < uniq.dim(); ++i) {
                            ids.printf("%" DEC, uniq[i]);
                            ids.add0();
                        }
                        ids.add0();
                    } else {
                        ids.printf("%s", p);
                        ids.add0(2);
                    }
                    for(char * id = ids.ptr(); id; id = sString::next00(id) ) {
                        sStr composite, wgeto, filename;
                        if( baseUrl ) {
                            composite.printf("%s%s", baseUrl, id);
                            wgeto.printf("%s", ex_auth.ptr());
                        } else if( datasource ) {
                            if( strcmp("sra", datasource) == 0 ) {
                                sStr first3, filename, byWhat;
                                if( regexec(&sra_rgx_bad, id, 0, NULL, 0) != REG_NOMATCH ) {
                                    errlog.printf("err.%s._err=NCBI SRA no longer supports downloads by experiment or sample", obj_id.ptr());
                                    break;
                                }
                                if( regexec(&sra_rgx, id, 0, NULL, 0) == REG_NOMATCH ) {
                                    errlog.printf("err.%s.uri=invalid SRA accession: '%s'\n", obj_id.ptr(), id);
                                    continue;
                                }
                                sString::changeCase(id, 0, sString::eCaseHi);
                                first3.printf("%.*s", 3, id);
                                wgeto.printf("--cut-dirs=7");
                                if( first3[2] == 'R' ) {
                                    byWhat.printf(0, "reads/ByRun/sra");
                                    filename.printf("/%s.sra", id);
                                    wgeto.cut0cut();
                                } else if( first3[2] == 'P' ) {
                                    byWhat.printf(0, "analysis/ByStudy");
                                    filename.printf("/");
                                } else if( first3[2] == 'S' ) {
                                    byWhat.printf(0, "analysis/BySample");
                                    filename.printf("/");
                                } else if( first3[2] == 'Z' ) {
                                    byWhat.printf(0, "analysis/ByAnalysis");
                                    filename.printf("/");
                                }
                                composite.printf("ftp://ftp-trace.ncbi.nlm.nih.gov/sra/sra-instant/%s/%s/%.*s/%s%s", byWhat.ptr(), first3.ptr(), 6, id, id, filename.ptr());
                            } else if( strcmp("genbank", datasource) == 0 ) {
                                composite.printf("http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=nuccore&rettype=gbwithparts&retmode=text&id=%s", id);
                                filename.printf("%s.gb", id);
                            } else if( sIs("ncbi_", datasource) ) {
                                if( regexec(&accession_rgx, id, 0, NULL, 0) == REG_NOMATCH ) {
                                    errlog.printf("err.%s.uri=invalid accession: '%s'\n", obj_id.ptr(), id);
                                    continue;
                                }
                                composite.printf("%s://%s", datasource, id);
                            } else if( strcmp("uniprot", datasource) == 0 ) {
                                filename.printf("%s", id);
                                if( strcasecmp(&id[sLen(id) > 4 ? sLen(id) - 4 : 0], ".txt") != 0 ) {
                                    filename.printf(".txt");
                                }
                                composite.printf("http://www.uniprot.org/uniprot/%s", filename.ptr());
                            } else {
                                composite.printf("%s://%s", datasource, id);
                                wgeto.printf("%s", ex_auth.ptr());
                            }
                        } else {
                            errlog.printf("err.%s.uri=Neither Data Source nor Base URL is provided for Identifier: '%s'\n", obj_id.ptr(), p);
                            continue;
                        }
                        addURI(*obj, composite, wgeto, filename);
                    }
                }
            }
        }
        regfree(&sra_rgx);
        regfree(&accession_rgx);
    } while(false);
    udx count = 0;
    if( !errlog ) {
        count = sString::cnt00(obj->propGet00("uri_data_uri"));
        if( count ) {
            idx chunkSize = obj->propGetI("download_concurrency");
            if( !chunkSize ) {
                chunkSize = 1;
            }
            cntParallel += (count - 1) / chunkSize + 1;
            requiresGroupSubmission = true;
        } else {
            errlog.printf("err.%s._err=Internal error %u", obj_id.ptr(), __LINE__);
        }
    }
    if( errlog ) {
        log->printf(0, "%s", errlog.ptr());
        if( obj_proc ) {
            obj_proc->actDelete();
        }
    }
    return count;
}

int main(int argc, const char *argv[], const char *envp[])
{
    sApp app;
    char get[] = "REQUEST_METHOD=GET";
    if( strstr(argv[0], "dmUp") ) {
        putenv(get);
    }
    dmDownloaderCGI qapp("config=qapp.cfg" __, "dmDownloader", argc, argv, envp, strstr(argv[0], "dmUp") ? 0 : stdin, true, true);
    if( strstr(argv[0], "dmUp") ) {
        return qapp.Uploader(argc, argv, envp);
    }
    qapp.run();
    return 0;
}

