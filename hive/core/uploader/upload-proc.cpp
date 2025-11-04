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
#include <slib/utils.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>

#include <slib/utils/multipart_parser.hpp>
#include <slib/utils/json/parser.hpp>
#include <dirent.h>

#include "common.hpp"

class serializable
{
    public:
        void setIdxValue(const char *name, idx &value, sVariant &tqs_arg)
        {
            sVariant *dummy = tqs_arg.getDicElt(name);
            if(dummy && dummy->isInt()) {
                value = dummy->asInt();
            }
        }

        void setStringValue(const char *name, sStr &value, sVariant &tqs_arg)
        {
            sVariant *dummy = tqs_arg.getDicElt(name);
            if(dummy && dummy->isString()) {
                value.printf(0, dummy->asString());
            }
        }
};

struct FilePartDescriptor : public serializable
{
    public:
        idx id;
        idx size;
        sStr path;
        idx lastMod;
        idx start;
        idx end;
        idx chunks;

        FilePartDescriptor()
            : id(-1), size(-1), lastMod(-1), start(-1), end(-1), chunks(0)
        {}

        bool init(sVariant *tqs_arg)
        {
            if(tqs_arg) {
                setIdxValue("id", id, *tqs_arg);
                setIdxValue("size", size, *tqs_arg);
                setStringValue("path", path, *tqs_arg);
                setIdxValue("lastModified", lastMod, *tqs_arg);
                setIdxValue("totalChunks", chunks, *tqs_arg);
                setIdxValue("start", start, *tqs_arg);
                setIdxValue("end", end, *tqs_arg);
                if(chunks == 1 && start < 0 && end < 0) {
                    start = 0;
                    end = size;
                }
                if(id < 0 || size < 0 || !path || chunks < 1 || start > end || end > size) {
                    return false;
                }
                lastMod /= 1000;
            }
            return tqs_arg;
        }

        idx getBlockLength()
        {
            return end - start;
        }
};

class UploadDescriptor : public serializable
{
    public:
        idx index;
        idx total;
        FilePartDescriptor * file;
        int fileNo;

        UploadDescriptor()
        {
            reset();
        }

        void reset()
        {
            index = -1;
            total = -1;
            file = 0;
            fileNo = -1;
            files.empty();
        }

        bool init(sVariant &tqs_arg)
        {
            reset();
            setIdxValue("index", index, tqs_arg);
            setIdxValue("total", total, tqs_arg);
            if(index < total) {
                sVariant *array = tqs_arg.getDicElt("files");
                if(array && array->isList()) {
                    for(idx i = 0; i < array->dim(); ++i) {
                        FilePartDescriptor *fd = files.add();
                        if(!fd || !fd->init(array->getListElt(i))) {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        FilePartDescriptor *getFilePartDescriptor(const char *index)
        {
            sVariant v(index);
            idx k = v.asInt();
            for(idx i = 0; i < files.dim(); ++i) {
                if(files[i].id == k) {
                    return &files[i];
                }
            }
            return 0;
        }

    private:
        sVec<FilePartDescriptor> files;
};

struct sFileData {
    idx index;
    idx size;
    idx written;
    idx lastMod;
    
    sFileData()
    : index(-1), size(-1), written(-1), lastMod(-1)
    {}
};

class UploadProc : public sHiveProc, private sMultipartParser
{
    protected:
        sStr m_uploadPath;
        UploadDescriptor m_descr;
        sStr * m_json;
        sFil * m_currFile;
        sDic<sFileData> m_filesMap;
        sDic<sHtml::sPartPair::TParts> m_partsMap;
        sDic<sStr> m_headers;
        sStr m_headerName, * m_headerValue;

    public:
        UploadProc(const char *defline00, const char *srv)
            : sHiveProc(defline00, srv), m_json(0), m_currFile(0), m_headerValue(0)
        {}

        virtual ~UploadProc()
        {
            reset();
        }

        void reset()
        {
            m_uploadPath.cut0cut();
            m_descr.reset();
            delete m_json;
            m_json = 0;
            delete m_currFile;
            m_currFile = 0;
            m_filesMap.empty();
            m_headers.empty();
            m_headerName.cut0cut();
            m_headerValue = 0;
        }

        const char * getUploadPath()
        {
            if(!m_uploadPath) {
                if( !reqGetData(reqId, uploadPathName, m_uploadPath.mex()) ) {
                    reqGetData(grpId, uploadPathName, m_uploadPath.mex());
                }
                if(!sDir::exists(m_uploadPath)) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Upload area not found: terminated");
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                    m_uploadPath.cut0cut();
                } else {
                    m_uploadPath.shrink00();
                    m_uploadPath.printf("/");
                }
            }
            return m_uploadPath.ptr();
        }

        bool mapSave(void) {
            sStr buf;
            if( m_filesMap.dim() ) {
                m_filesMap.serialOut(buf);
                if( !reqSetData(reqId, "files_map", buf.mex()) ) {
                    return false;
                }
                for(idx i = 0; i < m_partsMap.dim(); ++i) {
                    if( !reqSetData(reqId, (const char *)(m_partsMap.id(i)), m_partsMap.ptr(i)->mex()) ) {
                        return false;
                    }
                }
            } else {
                reqSetData(reqId, "files_map", buf.mex());
            }
            return true;
        }

        bool mapLoad(void)
        {
            if( !m_filesMap.dim() ) {
                sStr buf;
                reqGetData(reqId, "files_map", buf.mex());
                if( buf ) {
                    m_filesMap.serialIn(buf.ptr(), buf.length());
                }
            }
            return true;
        }

        sHtml::sPartPair::TParts * mapLoadParts(const char * fid)
        {
            sHtml::sPartPair::TParts *  retval = 0;
            if( fid && fid[0] ) {
                retval = m_partsMap.get(fid);
                if( !retval ) {
                    retval = m_partsMap.setString(fid);
                    reqGetData(reqId, fid, retval->mex());
                }
            }
            return retval;
        }

        idx processFiles()
        {
            idx qty = 0;
            const char *path = getUploadPath();
            if(!path) {
                return -1;
            }
            sFileGlob partGlob;
            partGlob.compile(TMP_PREFIX "*" PART_SUFFIX, false);
            struct dirent *dirEntry;
            DIR *dr = opendir(path);
            if(dr == NULL) {
                logOut(eQPLogType_Error, "Cannot read an input directory: %s", strerror(errno));
                reqSetStatus(reqId, eQPReqStatus_ProgError);
                return -1;
            }
            while((dirEntry = readdir(dr)) != NULL) {
                if(dirEntry->d_type != DT_DIR) {
                    if(partGlob.match(dirEntry->d_name)) {
                        sStr partFilePath("%s%s", path, dirEntry->d_name);
                        m_currFile = new sFil(partFilePath.ptr(), sMex::fReadonly);
                        if(m_currFile->ok()) {
                            sStr boundary;
                            sString::copyUntil(&boundary, m_currFile->ptr(2), m_currFile->length(), " \r\n" __);
                            if( !parse(boundary) ) {
                                logOut(eQPLogType_Error, "Failed to parse file: '%s'", partFilePath.ptr());
                            } else {
                                ++qty;
                                sFile::remove(partFilePath);
                            }
                            reqProgress(qty, -1, 100);
                            delete m_currFile;
                            m_currFile = 0;
                        } else {
                            logOut(eQPLogType_Error, "Failed to open file: '%s'", partFilePath.ptr());
                            break;
                        }
                    }
                }
            }
            closedir(dr);
            return mapSave() ? qty : -1;
        }

        bool fileBlockWritten(FilePartDescriptor & descr, const idx written)
        {
            if(mapLoad()) {
                sStr fid("%" UDEC, descr.id);
                sHtml::sPartPair::TParts * parts = mapLoadParts(fid.ptr());
                if( !parts ) {
                    return false;
                }
                sHtml::sPartPair part(descr.start, descr.end);
                sFileData * fd = m_filesMap.get(descr.path);
                if( !fd ) {
                    sHtml::sPartPair * ppart = parts->add(1);
                    fd = m_filesMap.setString(descr.path);
                    if( fd && ppart ) {
                        *ppart = part;
                        fd->index = descr.id;
                        fd->size = descr.size;
                        fd->written = written;
                        fd->lastMod = descr.lastMod;
                    } else {
                        return false;
                    }
                } else {
                    sHtml::sPartPair * ppart = parts->add(1);
                    if( ppart ) {
                        *ppart = part;
                        sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sHtml::sPartPair::comparator, 0, parts->dim(), parts->ptr());
                        if(parts->dim() > 1) {
                            for(idx i = parts->dim() - 2; i >= 0; --i) {
                                if((*parts)[i].neighbour((*parts)[i + 1]) || (*parts)[i].overlaps((*parts)[i + 1])) {
                                    (*parts)[i].merge((*parts)[i + 1]);
                                    parts->del(i + 1);
                                    i = parts->dim() - 1;
                                }
                            }
                        }
                        fd->written += written;
                    } else {
                        return false;
                    }
                }
            } else {
                return false;
            }
            return true;
        }

        bool finalCheck()
        {
            logOut(eQPLogType_Debug, "Validation");
            bool isok = false;
            if( mapLoad() ) {
                const char * path = getUploadPath();
                isok = path;
                if(isok) {
                    sDir udir;
                    udir.list(sFlag(sDir::bitSubdirs) | sFlag(sDir::bitFiles) | sFlag(sDir::bitEntryFlags), path, TMP_PREFIX "*.tmp");
                    logOut(eQPLogType_Debug, "Destination path '%s', temp files %" DEC, path, udir.dimEntries());
                    for(idx i = 0; i < udir.dimEntries(); ++i) {
                        if( !(udir.getEntryFlags(i) & sDir::fIsDir) ) {
                            logOut(eQPLogType_Debug, "Temp file found: '%s'", (char*)udir.getEntryPath(i));
                            sFileData * fd = m_filesMap.get( (((char*)udir.getEntryPath(i)) + strlen(path) + 1) );
                            if( !fd ) {
                                logOut(eQPLogType_Debug, "Temp file removed: '%s'", (char*)udir.getEntryPath(i));
                                sFile::remove(udir.getEntryPath(i));
                            }
                        }
                    }
                    sStr nm, fid;
                    for(idx i = 0; i < m_filesMap.dim(); ++i) {
                        idx len = 0;
                        const char * key = static_cast<const char *>(m_filesMap.id(i, &len));
                        sFileData * fd = m_filesMap.ptr(i);
                        fid.printf(0, "%" UDEC, fd->index);
                        nm.printf(0, "%s%.*s", path, (int)len, key);
                        sHtml::sPartPair::TParts * parts = mapLoadParts(fid.ptr());
                        if(parts->dim() == 0) {
                            logOut(eQPLogType_Error, "File %s is missing", key);
                            isok = false;
                        } else if(parts->dim() > 1) {
                            logOut(eQPLogType_Error, "File %s incomplete, missing blocks", key);
                            for(idx i = 0; i < parts->dim(); ++i) {
                                logOut(eQPLogType_Error, "File %s received block [%" UDEC ",%" UDEC "]", key, parts->ptr(i)->start, parts->ptr(i)->end);
                            }
                            isok = false;
                        } else if((*parts)[0].end != (udx)fd->size) {
                            logOut(eQPLogType_Error, "File %s is not complete: written %" DEC " of %" DEC, key, (*parts)[0].end, fd->size);
                            for(idx i = 0; i < parts->dim(); ++i) {
                                logOut(eQPLogType_Error, "File %s received block [%" UDEC ",%" UDEC "]", key, parts->ptr(i)->start, parts->ptr(i)->end);
                            }
                            isok = false;
                        }
                        if(fd->lastMod > 0 && !sFile::touch(nm, fd->lastMod)) {
                            logOut(eQPLogType_Warning, "File %s timestamp not set", key);
                        }
                    }
                }
            }
            if( !ok ) {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Upload did not go through, try again later");
            }
            return isok;
        }

        void terminate()
        {
            logOut(eQPLogType_Debug, "terminating");
            sStr emptyS;
            for(idx i = 0; i < m_filesMap.dim(); ++i) {
                const char * key = static_cast<const char *>(m_filesMap.id(i));
                reqSetData(reqId, key, emptyS.mex());
            }
            m_filesMap.empty();
            mapSave();
            const char * path = getUploadPath();
            if(path) {
                sDir::removeDir(path);
            }
        }

        bool isTimedout(const idx delaySec, const bool adjust)
        {
            const idx now = (idx)time(NULL);
            sStr final;
            reqGetData(reqId, "heartbeat", final.mex());
            if(final) {
                idx when = 0;
                sscanf(final, "%" DEC, &when);
                if(when && (now - when > delaySec)) {
                    logOut(eQPLogType_Warning, "idle timeout (%" DEC ") reached at %" DEC " from %" DEC, delaySec, now, when);
                    return true;
                }
            }
            if( !final || adjust ) {
                reqSetData(reqId, "heartbeat", "%" DEC, now);
            }
            return false;
        }

        virtual idx OnExecute(idx)
        {
            reqProgress(m_filesMap.dim(), 50, 100);
            switch (getUploadStatus(*this))
            {
            case eUnknown:
            case eInitialized:
                if(isTimedout(cfgInt(0, CFGNM(.IdleTimeoutSec), 24 * 60 * 60), false)) {
#if !_DEBUG
                    terminate();
#endif
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Upload was idle for too long: terminated");
                } else {
                    reqReSubmit(reqId, cfgInt(0, CFGNM(.ResubmitSec), 10));
                }
                break;
            case eUploading:
                if(isTimedout(cfgInt(0, CFGNM(.IdleTimeoutSec), 24 * 60 * 60), processFiles() > 0)) {
#if !_DEBUG
                    terminate();
#endif
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Upload was idle for too long: terminated");
                } else {
                    logOut(eQPLogType_Debug, "waiting for more parts");
                    reqReSubmit(reqId, cfgInt(0, CFGNM(.ResubmitSec), 10));
                }
                break;

            case ePaused:
                if(isTimedout(cfgInt(0, CFGNM(.OnPauseIdleTimeoutSec), 5 * 24 * 60 * 60), false)) {
#if !_DEBUG
                    terminate();
#endif
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Upload was on pause for too long: terminated");
                } else {
                    reqSetInfo(reqId, eQPInfoLevel_Info, "Upload paused by user");
                    reqReSubmit(reqId, cfgInt(0, CFGNM(.OnPauseResubmitSec), 10 * 60));
                }
                break;

            case eDeleted:
                terminate();
                reqSetStatus(reqId, eQPReqStatus_Killed);
                reqSetInfo(reqId, eQPInfoLevel_Warning, "Upload terminated by user");
                break;

            case eFinished:
                if(processFiles() < 0 || !finalCheck()) {
#if !_DEBUG
                    terminate();
#endif
                    reqSetStatus(reqId, eQPReqStatus_ProgError);
                } else {
                    reqSetStatus(reqId, eQPReqStatus_Done);
                    reqProgress(m_filesMap.dim(), 100, 100);
                }
                break;
            }
            reset();
            return 0;
        }

        virtual bool on_next_chunk(const char ** buf, udx & length)
        {
            if(*buf) {
                *buf = NULL;
                length = 0;
                return true;
            }
            *buf = *m_currFile;
            length = m_currFile->length();
            return true;
        }

        virtual bool on_part_begin()
        {
            m_headers.empty();
            m_headerName.cut0cut();
            m_headerValue = 0;
            return sMultipartParser::on_part_begin();
        }

        virtual bool on_header_field(const char * at, udx length)
        {
            m_headerName.add(at, length);
            if( m_headerValue ) {
                m_headerValue->add0(2);
            }
            m_headerValue = 0;
            return sMultipartParser::on_header_field(at, length);
        }

        virtual bool on_header_value(const char *buf, udx len)
        {
            if( !m_headerValue ) {
                m_headerName.add0(2);
                m_headerValue = m_headers.get(m_headerName);
                if( !m_headerValue ) {
                    m_headerValue = m_headers.setString(m_headerName);
                } else if( m_headerValue->length() ) {
                    m_headerValue->add(",", 1);
                }
                m_headerName.cut0cut();
            }
            if( !m_headerValue ) {
                logOut(eQPLogType_Error, "Out of memory in %s", __func__);
                return false;
            }
            m_headerValue->add(buf, len);
            return sMultipartParser::on_header_value(buf, len);
        }

        virtual bool on_headers_complete()
        {
            const char * cdh = "Content-Disposition";
            sStr * h = m_headers.get(cdh);
            if( h ) {
                sStr pName, pFileName;
                const char *fnd = 0;
                if( (fnd = strstr(h->ptr(), " filename=\"")) != 0 ) {
                    sString::copyUntil(&pFileName, fnd + 11, h->length(), "\"");
                    pFileName.shrink00();
                }
                if( (fnd = strstr(h->ptr(), " name=\"" )) != 0 ) {
                    sString::copyUntil(&pName, fnd + 7, h->length(), "\"");
                    pName.shrink00();
                }
                if(sIsExactly(pName, "info") && sIsExactly(pFileName, "blob")) {
                    m_json = new sStr;
                } else if( (m_descr.file = m_descr.getFilePartDescriptor(pName)) ) {
                    logOut(eQPLogType_Debug, "Receiving '%s' from %" DEC " to %" DEC, m_descr.file->path.ptr(), m_descr.file->start, m_descr.file->end);
                    sFilePath flnm(m_descr.file->path, "%%flnm");
                    if(sIsExactly(flnm, pFileName)) {
                        sStr fullpath("%s%s", m_uploadPath.ptr(), m_descr.file->path.ptr());
                        logOut(eQPLogType_Debug, "Destination path '%s'", fullpath.ptr());
                        sFilePath dir(fullpath, "%%dir");
                        if(dir && !sDir::exists(dir) && !sDir::makeDir(dir.ptr())) {
                            reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to create folder");
                            logOut(eQPLogType_Error, "Cannot create folder %s: %s", dir.ptr(), strerror(errno));
                            return false;
                        }
                        errno = 0;
                        m_descr.fileNo = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                        if(m_descr.fileNo < 0) {
                            reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to open file");
                            logOut(eQPLogType_Error, "Cannot open file %s: %s", fullpath.ptr(), strerror(errno));
                            return false;
                        }
                        if( lseek64(m_descr.fileNo, m_descr.file->start, SEEK_SET) < 0 || errno != 0 ) {
                            reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to write file");
                            logOut(eQPLogType_Error, "Cannot seek file %s: %s", fullpath.ptr(), strerror(errno));
                            return false;
                        }
                    } else {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to match file");
                        logOut(eQPLogType_Error, "File name mismatch for blob '%s' != '%s'", pFileName.ptr(), flnm.ptr());
                        return false;
                    }
                } else {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Bad file descriptor");
                    logOut(eQPLogType_Error, "File to find %s:%s", pName.ptr(), pFileName.ptr());
                    return false;
                }
            } else {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Bad request header");
                logOut(eQPLogType_Error, "Missing header %s", cdh);
                return false;
            }
            return sMultipartParser::on_headers_complete();
        }

        bool ifBlockRepeated(const char *at, udx length)
        {
            const idx TEST_BUF_SIZE = 2048;
            static char testbuf[TEST_BUF_SIZE];
            sStr fullpath("%s%s", m_uploadPath.ptr(), m_descr.file->path.ptr());

            idx readsize = read(m_descr.fileNo, testbuf, TEST_BUF_SIZE > length ? length : TEST_BUF_SIZE);
            switch (readsize)
            {
            case -1:
                reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to read file");
                logOut(eQPLogType_Error, "Cannot read file %s: %s", fullpath.ptr(), strerror(errno));
                break;
            case 0:
                break;
            default:
                if (lseek64(m_descr.fileNo, m_descr.file->start, SEEK_SET) < 0 || errno != 0) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to seek file");
                    logOut(eQPLogType_Error, "Cannot seek_set file %s: %s", fullpath.ptr(), strerror(errno));
                    return false;
                }
                if (!memcmp(at, testbuf, readsize)) {
                    return true;
                }
                break;
            }
            return false;
        }

        bool on_part_data(const char *at, udx length)
        {
            if( m_json ) {
                m_json->add(at, length);
            } else if( m_descr.file && m_descr.fileNo >= 0 ) {
                udx written = 0;
                errno = 0;

                while(errno == 0 && length - written > 0) {
                    written += write(m_descr.fileNo, at + written, length - written);
                }
                if(errno != 0 || written != length ) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to write file");
                    return false;
                }
                if( !fileBlockWritten(*m_descr.file, written) ) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to save file data");
                    logOut(eQPLogType_Error, "out of memory writing file block info '%s'", m_descr.file->path.ptr());
                    return false;
                }
            } else {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error %u", __LINE__);
                return false;
            }
            return sMultipartParser::on_part_data(at, length);
        }

        virtual bool on_part_data_end()
        {
            if( m_json ) {
                m_json->add0(2);
                sJSONParser json_parser;
                bool ok = json_parser.parse(*m_json) > 0;
                ok = ok ? m_descr.init(json_parser.result()) : ok;
                if( !ok ) {
                    logOut(eQPLogType_Error, "Invalid json blob '%s'", m_json->ptr());
                }
                delete m_json;
                m_json = 0;
                if( !ok ) {
                    return false;
                }
            } else if( m_descr.file && m_descr.fileNo >= 0 ) {
                const bool ok = close(m_descr.fileNo) == 0;
                if( !ok ) {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to save file");
                    logOut(eQPLogType_Error, "Cannot close file %s: %s", m_descr.file->path.ptr(), strerror(errno));
                }
                m_descr.file = 0;
                m_descr.fileNo = -1;
                if( !ok ) {
                    return false;
                }
            } else {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error %u", __LINE__);
                return false;
            }
            return sMultipartParser::on_part_data_end();
        }
};

int main(int argc, const char *argv[])
{
    sStr tmp;
    sApp::args(argc, argv);

    UploadProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, CFGNM(), argv[0]));
    return (int)backend.run(argc, argv);
}
