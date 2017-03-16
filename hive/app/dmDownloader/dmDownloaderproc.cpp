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
#include <qlib/QPrideProc.hpp>
#include <slib/core.hpp>
#include <slib/std/app.hpp>
#include <ulib/uproc.hpp>
#include <dmlib/dmlib.hpp>
#include <qpsvc/archiver.hpp>

#define NCBI_TIMEOUT 30

class dmDownloaderProc: public sQPrideProc
{
    public:
        dmDownloaderProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv), m_currFileSize(0), m_totalBytes(0)
        {
        }

        virtual idx OnExecute(idx);

        idx NCBIDownloader(sStr & destPrefix, const char * uri);

        idx countSequenceRecords(const char * filename)
        {
            sFil myFile(filename);
            if( !myFile ) {
                return 0;
            }
            idx giCount = 0;
            idx length = myFile.length();
            const char * fileContent = myFile;
            for(idx il = 0; il < length; ++il) {
                if( (il == 0 || fileContent[il - 1] == '\n') && fileContent[il] == '>' ) {
                    ++giCount;
                }
            }
            return giCount;
        }

        static idx progressCB(void * cbparam, idx bytes)
        {
            dmDownloaderProc * dp = static_cast<dmDownloaderProc *>(cbparam);
            if( dp ) {
#if _DEBUG
                fprintf(stderr, "%s: downloaded %" DEC " of %" DEC " bytes\n", __func__, bytes, dp->m_currFileSize);
#endif
                if(dp->reqProgress(dp->m_totalBytes + bytes, bytes, dp->m_currFileSize) == 0 ) {
#if _DEBUG
                    fprintf(stderr, "%s: stopped download %" DEC "\n", __func__, dp->reqId);
#endif
                    return 0; // interrupted
                }
            }
            return 1;
        }

    private:
        idx m_currFileSize;
        idx m_totalBytes;
};

idx dmDownloaderProc::OnExecute(idx req)
{
    sStr varName("user.download"), outDir;
    cfgStr(&outDir, 0, varName, 0);
    if( !outDir ) {
        logOut(eQPLogType_Error, "dmDownloader.path not set in config");
        reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    outDir.printf("%" UDEC, req);

    if( !objs.dim() ) {
        logOut(eQPLogType_Error, "missing request object: obj.dim() == 0");
        reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    const bool upload = objs[0].propGetBool("is_upload");
    if( upload ) {
        reqSetInfo(req, eQPInfoLevel_Error, "User aborted upload or connection with server was interrupted");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    sVarSet uris; // 3 columns: uri, [wget options], [filename]
    const sUsrObjPropsTree * tree = objs[0].propsTree(); // can fail only if we run out of memory
    const sUsrObjPropsNode * vars_node_array = tree ? tree->find("uri_data") : 0;
    for(const sUsrObjPropsNode * row = vars_node_array ? vars_node_array->firstChild() : 0; row && row->field()->isArrayRow(); row = row->nextSibling()) {
        const char * u = row->findValue("uri_data_uri");
        if( u && u[0] ) {
            uris.addRow().addCol(u).addCol(row->findValue("uri_data_wget")).addCol(row->findValue("uri_data_filename"));
        }
    }
    if( uris.rows == 0 ) {
        // grab in old way
        sStr uriList;
        formValue("uri", &uriList);
        sString::searchAndReplaceSymbols(uriList, 0, "\n", 0, 0, true, true, false, false);
        for(const char * p = uriList; p; p = sString::next00(p)) {
            uris.addRow().addCol(p).addCol((char*) 0).addCol((char*) 0);
        }
    }
    if( !uris.rows ) {
        logOut(eQPLogType_Error, "ID list is missing or corrupted\n");
        reqSetInfo(req, eQPInfoLevel_Error, "ID list is empty");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    for(idx r = 0; r < uris.rows; ++r) {
        logOut(eQPLogType_Info, "uri %" DEC " '%s','%s','%s'\n", r, uris.val(r, 0), uris.val(r, 1), uris.val(r, 2));
    }
    // determine the geometry of the list and our chunk
    udx chunkSize = formUValue("download_concurrency");
    if( !chunkSize ) {
        chunkSize = 1;
    }
    idx myStart = reqSliceId * chunkSize;
    idx myEnd = myStart + chunkSize;
    if( myEnd > uris.rows ) {
        myEnd = uris.rows;
    }
    logOut(eQPLogType_Info, "processing part %" DEC " of %" DEC " rows from %" DEC " to %" DEC " \n", reqSliceId + 1, reqSliceCnt, myStart + 1, myEnd);
    m_totalBytes = 0;
    dmRemoteFile remContent(progressCB, this, svc.lazyReportSec / 2);

    sUsrObjRes dropboxList;
    user->objs2("dropbox", dropboxList, 0, 0, 0, "dropbox_path,dropbox_name");
    idx i = myStart;
    for(; i < myEnd; ++i) {
        char * ptr = uris.val(i, 0);
        sStr substitutedURI(sMex::fExactSize), whichDropBox(sMex::fExactSize);
        // substitutions of protocol
        if( strncmp(ptr, "dropbox", 7) == 0 ) {
            if( !dropboxList.dim() ) {
                logOut(eQPLogType_Error, "dropbox not found '%s'\n", ptr);
                reqSetInfo(req, eQPInfoLevel_Error, "No dropboxes defined for this user.");
                continue;
            }
            sString::extractSubstring(&whichDropBox, ptr, 0, 2, "/" __, false, false);
            sHiveId dropbox_id(whichDropBox);
            if( const sUsrObjRes::TObjProp * dropbox_prop = dropboxList.get(dropbox_id) ) {
                const char * dropbox_path = 0;
                if( const sUsrObjRes::TPropTbl * t = dropboxList.get(*dropbox_prop, "dropbox_path") ) {
                    dropbox_path = dropboxList.getValue(t);
                }
                if( dropbox_path && *dropbox_path ) {
                    if( !strstr(dropbox_path, "://") ) {
                        substitutedURI.printf("file://");
                    }
                    substitutedURI.printf("%s%s", dropbox_path, ptr + sLen("dropbox://") + sLen(whichDropBox) + 1);
                }
            }
            if( !substitutedURI ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Dropbox '%s' configuration error", whichDropBox.ptr());
                continue;
            }
            ptr = substitutedURI;
        }
        char * protocol = ptr, * objectId = strstr(protocol, "://");
        if( !objectId ) {
            continue;
        }
        objectId[0] = '\0';
        objectId += 3;
        ptr = objectId;
        // compose a complete url from parts
        sStr url("%s://%s", protocol, objectId);
        sStr destFile("%s_%04" UDEC "/", outDir.ptr(), i + 1);
        if( !sDir::makeDir(destFile) ) {
            logOut(eQPLogType_Error, "mkdir failed '%s'\n", destFile.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        const char * destfilename = uris.val(i, 2);
        if( !destfilename || !destfilename[0] ) {
            destfilename = sFilePath::nextToSlash(objectId);
        }
        sStr safe_nm;
        sDir::cleanUpName(destfilename, safe_nm, false);
        if( safe_nm ) {
            destFile.printf("%s", safe_nm.ptr());
        }
        logOut(eQPLogType_Info, "Downloading '%s' into '%s'\n", url.ptr(), destFile.ptr());
        idx length = 0;
        // files are only supported within dropbox! security!
        if( whichDropBox && strcasecmp(protocol, "file") == 0 ) {
            if( !sFile::exists(objectId) ) {
                reqSetInfo(req, eQPInfoLevel_Error, "File '%s' not found in dropbox '%s'", objectId, whichDropBox.ptr());
            } else {
                sFile::remove(destFile);
                if( !sFile::symlink(objectId, destFile) ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Cannot establish connection with file '%s' in dropbox %s", url.ptr(), whichDropBox.ptr());
                } else {
                    length = sFile::size(destFile);
                    if( !length ) {
                        reqSetInfo(req, eQPInfoLevel_Error, "File '%s' in dropbox '%s' is empty", objectId, whichDropBox.ptr());
                    }
                }
            }
        } else if( strcasecmp(protocol, "http") == 0 || strcasecmp(protocol, "https") == 0 || strcasecmp(protocol, "ftp") == 0 ) {
            m_currFileSize = 0;
            sRC rc = remContent.getFile(destFile, &m_currFileSize, uris.val(i, 1), "%s", url.ptr());
            if( rc ) {
                reqSetInfo(req, eQPInfoLevel_Error, "'%s': %s", url.ptr(), rc.print());
            } else {
                length = sFile::size(destFile);
                if( !length ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "File '%s' is empty", url.ptr());
                }
            }
            m_currFileSize = 0;
        } else if( !whichDropBox && strncmp(protocol, "ncbi_", 5) == 0 ) {
            // this call will print all its errors by itself
            length = NCBIDownloader(destFile, url);
            destfilename = destFile;
        } else {
            reqSetInfo(req, eQPInfoLevel_Error, "Schema '%s://' is not supported", protocol);
            continue;
        }
        // launch a service which would futher process the file into an object
        if( length ) {
            logOut(eQPLogType_Info, "Downloaded '%s://%s' into '%s' %" DEC " bytes\n", protocol, objectId, destFile.ptr(), sFile::size(destFile));
            dmArchiver arch(*this, destFile, url, 0, destfilename);
            arch.addObjProperty("hierarchy", "%s", formValue("hierarchy", 0, ""));
            arch.addObjProperty("category", "%s", formValue("category", 0, ""));
            arch.addObjProperty("source", "%s://%s", protocol, objectId);
            arch.setSubject(formValue("upload_subject"));
            arch.setDepth(*this, objs[0].propGetI("chkauto_Downloader"));
            arch.setIndexFlag(*this, objs[0].propGetBool("idx_Downloader"));
            arch.setQCFlag(*this, objs[0].propGetBool("qc_Downloader"));
            arch.setScreenFlag(*this, objs[0].propGetBool("screen_Downloader"));
            idx req = arch.launch(*user, grpId);
            logOut(eQPLogType_Info, "Launching dmArchiver request %" DEC "\n", req);
            m_totalBytes += length;
        } else {
#if !_DEBUG
            sDir::removeDir(outDir);
#endif
        }
        if( !reqProgress(-1, i - myStart + 1, myEnd - myStart + 1) ) {
            break;
        }
    }
    if( i >= myEnd ) {
        reqProgress(-1, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }
    return 0;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ NCBI download functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx dmDownloaderProc::NCBIDownloader(sStr & destPrefix, const char * uri)
{
    sStr u;
    sString::searchAndReplaceStrings(&u, uri, 0, "://" __, 0, 1, false);
    const char * db = u.ptr(5); //skip ncbi_ prefix, it was tested in caller for it, see above
    const char * id = sString::next00(db);

    dmRemoteFile remContent(progressCB, this);

    sStr rettype("fasta"), suffix;
    sStr database;
    if( strcmp(db, "nuccds") == 0 ) {
        rettype.printf("_cds_na");
        database.printf("nuccore");
        suffix.printf("-exon.fa");
    } else {
        database.printf("%s", db);
        if( strcmp(db, "nuccore") == 0 ) {
            suffix.printf("-gene.fa");
        } else if( strcmp(db, "protein") == 0 ) {
            suffix.printf("-protein.pfa");
        } else {
            reqSetInfo(reqId, eQPInfoLevel_Info, "Database %s not supported.", db);
            return 0;
        }
    }

    // TODO one more step here to esearch in bioproject and use resulting ids in request to elink this way you can specify search not just id
    // though your elink result will be limited to the first 1000 with links
    if( sIs("PRJNA", id) ) {
        id += 5;
    }
    sStr strError, query_key, webenv;
    // elink from bioProject id to the other database using NCBI history
    sStr tempFileName("%s.step1.xml", destPrefix.ptr());
    sFile::remove(tempFileName);
    sRC rc = remContent.getFile(tempFileName, 0, 0, "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?cmd=neighbor_history&dbfrom=bioproject&db=%s&linkname=bioproject_%s&id=%s", database.ptr(), database.ptr(), id);
    sFil httpContent(tempFileName);
    idx len = httpContent.length();
    if( rc ) {
        logOut(eQPLogType_Error, "elink error %s: %s\n", uri, rc.print());
    } else if( len == 0 ) {
        logOut(eQPLogType_Error, "elink response is empty for %s\n", uri);
    } else {
        sString::cleanMarkup(&strError, httpContent, len, "<ERROR>" __, "</ERROR>" __, 0, 0, true, false, true);
        sString::cleanMarkup(&query_key, httpContent, len, "<QueryKey>" __, "</QueryKey>" __, 0, 0, true, false, true);
        sString::cleanMarkup(&webenv, httpContent, len, "<WebEnv>" __, "</WebEnv>" __, 0, 0, true, false, true);
        if( strError.length() > 0 && *strError.ptr(1) ) {
            logOut(eQPLogType_Error, "elink response error: '%s' for %s\n", strError.ptr(1), uri);
        } else {
            const char * qkey = query_key.ptr(1);
            const char * we = webenv.ptr(1);
            if( !qkey || !qkey[0] || !we || !we[0] ) {
                logOut(eQPLogType_Error, "elink missing %s for %s\n", we && we[0] ? "query_key" : "webenv", uri);
                query_key.cut(0);
                webenv.cut(0);
            } else {
                query_key.del(0, 1);
                webenv.del(0, 1);
#if _DEBUG
        fprintf(stderr, "%s webenv '%s', query_key '%s'\n", __func__, webenv.ptr(), query_key.ptr());
#endif
            }
        }
    }
    httpContent.destroy();
#ifndef _DEBUG
    sFile::remove(tempFileName);
#endif
    // search database using query_key to obtain results count
    idx count = 0;
    if( webenv && query_key ) {
        tempFileName.printf(0, "%s.step2.xml", destPrefix.ptr());
        sFile::remove(tempFileName);
        sRC rc = remContent.getFile(tempFileName, 0, 0, "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=%s&WebEnv=%s&query_key=%s&usehistory=y", database.ptr(), webenv.ptr(), query_key.ptr());
        httpContent.init(tempFileName.ptr());
        len = httpContent.length();
        if( rc ) {
            logOut(eQPLogType_Error, "elink error %s: %s\n", uri, rc.print());
        } else if( len == 0 ) {
            logOut(eQPLogType_Error, "error: esearch response is empty for %s\n", uri);
        } else {
            sStr searchresult;
            strError.cut(0);
            sString::cleanMarkup(&searchresult, httpContent, len, "<Count>" __, "</Count>" __, 0, 0, true, false, true);
            sString::cleanMarkup(&strError, httpContent, len, "<ERROR>" __, "</ERROR>" __, 0, 0, true, false, true);
            if( strError.length() > 0 && *strError.ptr(1) ) {
                logOut(eQPLogType_Error, "esearch response error: '%s' for %s\n", strError.ptr(1), uri);
            } else {
                count = searchresult.length() > 0 ? atol(searchresult.ptr(1)) : 0;
            }
        }
    }
    httpContent.destroy();
#ifndef _DEBUG
    sFile::remove(tempFileName);
#endif
    if( !query_key || !webenv || !count ) {
        reqSetInfo(reqId, eQPInfoLevel_Info, "Search for NCBI BioProject '%s' links to database '%s' did not produce any results.", id, database.ptr());
        return 0;
    }
    idx countgi = 0;
    const idx chunk = (count >= 10) ? count / 10 : 1;
    for(idx attempt = 0; attempt < 5; ++attempt) {
        countgi = 0;
        sleepSeconds(attempt ? 60 : 0);
        bool retry = false;
        for(idx i = 0; i < count; i += chunk) {
            const idx max = (i + chunk > count) ? count - i : chunk;
            sStr chunkfile("%s-%03" DEC "-to-%03" DEC "%s", destPrefix.ptr(), i, i + max - 1, suffix.ptr());
            if( !sFile::exists(chunkfile) ) {
                sStr url("http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=%s&WebEnv=%s&query_key=%s&retmode=text&rettype=%s&retstart=%" DEC "&retmax=%" DEC "",
                        database.ptr(), webenv.ptr(), query_key.ptr(), rettype.ptr(), i, max);
                logOut(eQPLogType_Info, "Downloading: try %" DEC ", from %03" DEC " to %03" DEC ", url '%s'\n", attempt, i, i + max - 1, url.ptr());
                sRC rc = remContent.getFile(chunkfile, 0, 0, "%s", url.ptr());
                if( !rc ) {
                    idx gis = countSequenceRecords(chunkfile);
                    if( gis ) {
                        countgi += gis;
                    } else {
                        logOut(eQPLogType_Info, "Downloaded empty file of %" DEC " bytes? try %" DEC ", from %03" DEC " to %03" DEC "\n", sFile::size(chunkfile), attempt, i, i + max - 1);
                        sFile::remove(chunkfile);
                        retry |= true;
                    }
                } else {
                    logOut(eQPLogType_Error, "efetch error %s: %s\n", url.ptr(), rc.print());
                }
                // make intervals not to abuse NCBI
                sleepSeconds(3);
            } else {
                countgi += countSequenceRecords(chunkfile);
            }
        }
        if( !retry ) {
            break;
        }
    }
    sStr finalConcat("%s-concat%s", destPrefix.ptr(), suffix.ptr());
    sFile::remove(finalConcat);
    for(idx i = 0; i < count; i += chunk) {
        const idx max = (i + chunk > count) ? count - i : chunk;
        sStr chunkfile("%s-%03" DEC "-to-%03" DEC "%s", destPrefix.ptr(), i, i + max - 1, suffix.ptr());
        if( sFile::exists(chunkfile) ) {
            if( !sFile::copy(chunkfile, finalConcat, true) ) {
                logOut(eQPLogType_Error, "Cannot append chunk to all '%s' += '%s', disk space?\n", finalConcat.ptr(), chunkfile.ptr());
                reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error %u", __LINE__);
                sFile::remove(finalConcat);
                break;
            } else {
#ifdef _DEBUG
                sStr debug("%s.debug", chunkfile.ptr());
                sFile::rename(chunkfile, debug);
#else
                sFile::remove(chunkfile);
#endif
            }
        } else {
            logOut(eQPLogType_Error, "Missing chunk '%s'\n", chunkfile.ptr());
            reqSetInfo(reqId, eQPInfoLevel_Error, "Couldn't make a complete download for '%s' in database '%s'", id, database.ptr());
            sFile::remove(finalConcat);
            break;
        }
    }
    if( sFile::exists(finalConcat) ) {
        // count for this db is never same?
        if( strcmp(db, "nuccds") == 0 || count == countgi ) {
            sStr dst("%s%s", destPrefix.ptr(), suffix.ptr());
            if( !sFile::rename(finalConcat, dst) ) {
                logOut(eQPLogType_Error, "Cannot rename final file '%s' -> '%s'\n", finalConcat.ptr(), dst.ptr());
                reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error %u", __LINE__);
            } else {
                destPrefix.printf("%s", suffix.ptr());
            }
        } else {
            reqSetInfo(reqId, eQPInfoLevel_Error, "Couldn't download all %" DEC " sequences for '%s' in database '%s'", count, id, database.ptr());
        }
    }
    return sFile::size(destPrefix);
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Main
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future
    dmDownloaderProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dmDownloader", argv[0]));
    return (int) backend.run(argc, argv);
}

