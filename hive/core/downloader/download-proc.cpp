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

#include <qpsvc/qpsvc.hpp>
#include <violin/hiveproc.hpp>
#include <xlib/dmlib.hpp>


struct URIData
{
    sStr uris;
    sStr filenames;

    void add_uri(const char * uri, const char * filename = 0)
    {
        uris.addString(uri);
        uris.add0();
        if ( filename ) {
            filenames.addString(filename);
            filenames.add0();
        }
    }

    const char * get_uri(idx i) const { return sString::next00(uris.ptr(0), i); }
    const char * get_filename(idx i) const { return sString::next00(filenames.ptr(0), i); }
    idx dim() const { return sString::cnt00(uris.ptr(0)); }
    bool has_filenames() const { return filenames.length() > 0 && sString::cnt00(filenames.ptr(0)) == dim(); }
    void finalize() { uris.add0(2); filenames.add0(2); } 
};

class DownloadProc : public sHiveProc
{

    protected:

        idx getURIs(URIData & out, sStr & err);
        idx NCBIDownloader(sStr & destPrefix, const char * uri);
        void set_name(const URIData & uris);

        bool is_hive_net()
        {
            const char * datasource = formValue("datasource");
            return datasource && !strcmp(datasource, "hive");
        }
        
        static idx progressCB(void * cbparam, idx bytes)
        {
            DownloadProc * dp = static_cast<DownloadProc *>(cbparam);
            if( dp ) {
#if _DEBUG
                fprintf(stderr, "%s: downloaded %" DEC " of %" DEC " bytes\n", __func__, bytes, dp->m_currFileSize);
#endif
                if(dp->reqProgress(dp->m_totalBytes + bytes, bytes, dp->m_currFileSize) == 0 ) {
#if _DEBUG
                    fprintf(stderr, "%s: stopped download %" DEC "\n", __func__, dp->reqId);
#endif
                    return 0;
                }
            }
            return 1;
        }

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

    public:
        DownloadProc(const char *defline00, const char *srv)
            : sHiveProc(defline00, srv)
        {}

        virtual ~DownloadProc()
        {
        }

        virtual sRC OnSplit(idx req, idx &cnt)
        {
            URIData uris;
            sStr err;
            if ( getURIs(uris, err) ) {
                reqSetInfo(req, eQPInfoLevel_Error, err.ptr(0));
                reqSetStatus(req, eQPReqStatus_ProgError);
                return RC(sRC::eSplitting,sRC::eRequest,sRC::eBlob, sRC::eUndefined);
            }

            set_name(uris);

            if ( is_hive_net() )
                cnt = 1;
            else 
                cnt = uris.dim();

            return sRC::zero;
        }

        virtual idx OnExecute(idx req);

        private:
            idx m_currFileSize;
            idx m_totalBytes;

};

idx DownloadProc::OnExecute(idx req)
{
    URIData uris;
    sStr err;

    if ( getURIs(uris, err) ) {
        reqSetInfo(req, eQPInfoLevel_Error, err.ptr(0));
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 1;
    }
    
    sStr varName("user.download"), outDir;
    cfgStr(&outDir, 0, varName, 0);
    if( !outDir ) {
        logOut(eQPLogType_Error, "dmDownloader.path not set in config");
        reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    outDir.printf("%" UDEC, grpId);

    if( !objs.dim() ) {
        logOut(eQPLogType_Error, "missing request object: obj.dim() == 0");
        reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    m_totalBytes = 0;
    dmRemoteFile remContent(progressCB, this, svc.lazyReportSec / 2);

    sUsrObjRes dropboxList;
    user->objs2("dropbox", dropboxList, 0, 0, 0, "dropbox_path,dropbox_name");
    
    idx myStart = reqSliceId;
    idx myEnd;
    if ( is_hive_net() )
        myEnd = reqSliceId + uris.dim();
    else
        myEnd = reqSliceId + 1;
    
    for(idx i = myStart; i < myEnd; ++i) {
        const char * ptr = uris.get_uri(i);
        sStr substitutedURI(sMex::fExactSize), whichDropBox(sMex::fExactSize);
        sFilePath dropboxSubdir;
        if( strncmp(ptr, "hive", 4) == 0 ) {
            if( !dropboxList.dim() ) {
                logOut(eQPLogType_Error, "dropbox not found '%s'\n", ptr);
                reqSetInfo(req, eQPInfoLevel_Error, "No dropboxes defined for this user.");
                return 1;
            }
            sString::extractSubstring(&whichDropBox, ptr, 0, 2, "/" __, false, false);
            sHiveId dropbox_id(whichDropBox);
            if( const sUsrObjRes::TObjProp * dropbox_prop = dropboxList.get(dropbox_id) ) {
                const char * dropbox_path = 0;
                const char * dropbox_name = 0;
                if( const sUsrObjRes::TPropTbl * t = dropboxList.get(*dropbox_prop, "dropbox_path") ) {
                    dropbox_path = dropboxList.getValue(t);
                }
                if( const sUsrObjRes::TPropTbl * t = dropboxList.get(*dropbox_prop, "dropbox_name") ) {
                    dropbox_name = dropboxList.getValue(t);
                }
                if( dropbox_path && *dropbox_path ) {
                    if( !strstr(dropbox_path, "://") ) {
                        substitutedURI.printf("file://");
                    }
                    const char * x = ptr + sLen("hive://") + sLen(whichDropBox) + 1;
                    if ( dropbox_name )
                        dropboxSubdir.printf("%s/", dropbox_name);
                    dropboxSubdir.printf("%s/", x);
                    dropboxSubdir.shrink00();
                    substitutedURI.printf("%s%s", dropbox_path, x);
                }
            }
            if( !substitutedURI ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Dropbox '%s' configuration error", whichDropBox.ptr());
                return 1;
            }
            ptr = substitutedURI;
        }

        sStr protocolBuf;
        protocolBuf.addString(ptr);
        char * protocol = protocolBuf.ptr(0), * objectId = strstr(protocol, "://");
        if( !objectId ) {
            return 1;
        }
        objectId[0] = '\0';
        objectId += 3;
        ptr = objectId;
        sStr url("%s://%s", protocol, objectId);
        sStr destFile("%s/%s", outDir.ptr(), dropboxSubdir.ptr() ? dropboxSubdir.ptr() : "");
        if( !sDir::makeDir(destFile) ) {
            logOut(eQPLogType_Error, "mkdir failed '%s'\n", destFile.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        const char * destfilename = 0;
        if ( uris.has_filenames() )
            destfilename = uris.get_filename(i);
        if ( !destfilename || !destfilename[0] )
            destfilename = sFilePath::nextToSlash(objectId);
        sStr safe_nm;
        sDir::cleanUpName(destfilename, safe_nm, false);
        if( safe_nm ) {
            safe_nm.shrink00();
            destFile.printf("%s", safe_nm.ptr());
        }
        logOut(eQPLogType_Info, "Downloading '%s' into '%s'\n", url.ptr(), destFile.ptr());
        idx length = 0;
        if( whichDropBox && strcasecmp(protocol, "file") == 0 ) {
            if( !sFile::exists(objectId) && !sDir::exists(objectId) ) {
                reqSetInfo(req, eQPInfoLevel_Error, "'%s' not found in dropbox '%s'", objectId, whichDropBox.ptr());
            } else {
                idx p = 0;
                while(destFile[p = strlen(destFile) - 1] == '/' ) {
                    destFile[p] = '\0';
                }
                while(objectId[p = strlen(objectId) - 1] == '/' ) {
                    objectId[p] = '\0';
                }
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

            sStr ex_auth;
            sStr ex_login(sMex::fBlockCompact), ex_passwd(sMex::fBlockCompact);
            objs[0].propGet00("external_login", &ex_login);
            objs[0].propGet00("external_password", &ex_passwd);
            if( ex_login ) {
                ex_auth.printf("\"--user=%s\"", ex_login.ptr());
                if( ex_passwd ) {
                    ex_auth.printf(" \"--password=%s\"", ex_passwd.ptr());
                }
            } else if( ex_passwd ) {
                reqSetInfo(req, eQPInfoLevel_Error, "err.external_login=Missing user name\n");
                return 1;
            }

            sStr http_dest_filename, http_safe_filename, http_safe_dir;
            if ( uris.has_filenames() )
                http_dest_filename.printf("%s/%s", outDir.ptr(), uris.get_filename(i));
            else
                http_dest_filename.printf("%s/%s", outDir.ptr(), objectId);
            sDir::cleanUpName(http_dest_filename.ptr(), http_safe_filename);
            http_safe_dir.addString(http_safe_filename.ptr(0), sFilePath::nextToSlash(http_safe_filename.ptr()) - http_safe_filename.ptr());
            if ( !sDir::makeDir(http_safe_dir.ptr()) ) {
                logOut(eQPLogType_Error, "mkdir failed '%s'\n", destFile.ptr());
                reqSetInfo(req, eQPInfoLevel_Error, "Internal error %u", __LINE__);
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }

            sRC rc = remContent.getFile(http_safe_filename.ptr(), &m_currFileSize, ex_auth.ptr(0), true, "%s", url.ptr());
            if( rc && rc.val.parts.bad_entity == sRC::eCertificate && rc.val.parts.state == sRC::eInvalid ) {
                reqSetInfo(rc, eQPInfoLevel_Warning, "Cannot validate certificate for url '%s', proceeding to download in insecure mode", url.ptr());
                rc = remContent.getFile(http_safe_filename.ptr(), &m_currFileSize, ex_auth.ptr(0), false, "%s", url.ptr());
            }
            if( rc ) {
                reqSetInfo(req, eQPInfoLevel_Error, "'%s': %s", url.ptr(), rc.print());
            } else {
                length = sFile::size(http_safe_filename.ptr());
                if( !length ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "File '%s' is empty", url.ptr());
                }
                if( length != m_currFileSize ) {
                    reqSetInfo(req, eQPInfoLevel_Warning, "File '%s' is different size than expected: Expected %" DEC "; Found %" DEC, url.ptr(), length, m_currFileSize);
                }
            }
            m_currFileSize = 0;
        } else if( !whichDropBox && strncmp(protocol, "ncbi_", 5) == 0 ) {
            length = NCBIDownloader(destFile, url);
        } else {
            reqSetInfo(req, eQPInfoLevel_Error, "Schema '%s://' is not supported", protocol);
            return 1;
        }
        
        if( length ) {
            destFile.cut0cut(destFile.pos() - dropboxSubdir.pos() - safe_nm.pos());
            logOut(eQPLogType_Info, "Downloaded '%s://%s' into '%s' %" DEC " bytes\n", protocol, objectId, destFile.ptr(), sFile::size(destFile));
            m_totalBytes += length;
        } else {
#if !_DEBUG
            sDir::removeDir(outDir);
#endif
        }
    }
    
    if( isLastInMasterGroup() ) {
        sStr out_prop_buf;
        out_prop_buf.printf("%" UDEC "/", grpId);
        const char * out_prop = out_prop_buf.ptr(0);
        objs[0].propSet("output", 0, &out_prop, 1);
    }

    reqProgress(-1, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}

void DownloadProc::set_name(const URIData & uris)
{
    const idx MAX_NAME_LEN = 128;
    sStr name;
    
    const char * prev_name = objs[0].propGet00("name");
    name.printf("%s ", prev_name ? prev_name : "");

    for (idx i = 0; i < uris.dim(); ++i) {
        const char * name_to_add = 0;
        if ( uris.has_filenames() )
            name_to_add = uris.get_filename(i);
        else
            name_to_add = uris.get_uri(i);
        name.addString(name_to_add);
        if ( i < uris.dim() - 1 )
            name.addString(", ");
        if ( name.length() >= MAX_NAME_LEN ) {
            name.cut(MAX_NAME_LEN - 3);
            name.addString("...");
            break;
        }
    }
    name.add0();
    const char * name_ptr = name.ptr(0);
    objs[0].propSet("name", 0, &name_ptr, 1);
}

idx DownloadProc::getURIs(URIData & out, sStr & err)
{
    sStr uri;
    formValue("uri", &uri);
    if( !uri ) {
        err.printf("URLs and Identifiers list is empty: nothing to do");
        return 1;
    }

    const char * const schemas = "http" _ "https" _ "ftp" _ "ncbi_nuccore" _ "ncbi_nuccds" _ "genbank" _ "hive" __;

    sStr base;
    formValue("baseURL", &base);
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
            err.printf("baseURL has not a valid schema: '%s://'", schema.ptr());
            return 1;
        }
    } else {
        base.cut(0);
        baseUrl = 0;
    }
    const char * datasource = formValue("datasource");
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

    regex_t accession_rgx;
    if( regcomp(&accession_rgx, "^[A-Z]*[0-9]+\\.?[0-9]*$", REG_EXTENDED | REG_ICASE) != 0 ) {
        err.printf("Internal error %u", __LINE__);
        return 1;
    }
    sStr buf;
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
                    err.printf("Invalid URL schema: '%s://'\n", sc.ptr());
                    continue;
                }
                out.add_uri(p);
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
                    sStr composite, filename;
                    if( baseUrl ) {
                        composite.printf("%s%s", baseUrl, id);
                    } else if( datasource ) {
                        if( strcmp("genbank", datasource) == 0 ) {
                            composite.printf("http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=nuccore&rettype=gbwithparts&retmode=text&id=%s", id);
                            filename.printf("%s.gb", id);
                        } else if( sIs("ncbi_", datasource) ) {
                            if( regexec(&accession_rgx, id, 0, NULL, 0) == REG_NOMATCH ) {
                                err.printf("invalid accession: '%s'\n", id);
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
                        }
                    } else {
                        err.printf("Neither Data Source nor Base URL is provided for Identifier: '%s'\n", p);
                        continue;
                    }
                    out.add_uri(composite.ptr(0), filename.length() ? filename.ptr(0) : 0);
                }
            }
        }
    }
    regfree(&accession_rgx);
    out.finalize();

    return 0;
}

idx DownloadProc::NCBIDownloader(sStr & destPrefix, const char * uri)
{
    sStr u;
    sString::searchAndReplaceStrings(&u, uri, 0, "://" __, 0, 1, false);
    const char * db = u.ptr(5);
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

    if( sIs("PRJNA", id) ) {
        id += 5;
    }
    sStr strError, query_key, webenv;
    sStr tempFileName("%s.step1.xml", destPrefix.ptr());
    sFile::remove(tempFileName);
    sRC rc = remContent.getFile(tempFileName, 0, 0, true, "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?cmd=neighbor_history&dbfrom=bioproject&db=%s&linkname=bioproject_%s&id=%s", database.ptr(), database.ptr(), id);
    if( rc && rc.val.parts.bad_entity == sRC::eCertificate && rc.val.parts.state == sRC::eInvalid ) {
        reqSetInfo(rc, eQPInfoLevel_Warning, "Cannot validate certificate for url '%s', proceeding to download in insecure mode", uri);
        rc = remContent.getFile(tempFileName, 0, 0, false, "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?cmd=neighbor_history&dbfrom=bioproject&db=%s&linkname=bioproject_%s&id=%s", database.ptr(), database.ptr(), id);
    }
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
    idx count = 0;
    if( webenv && query_key ) {
        tempFileName.printf(0, "%s.step2.xml", destPrefix.ptr());
        sFile::remove(tempFileName);
        sRC rc = remContent.getFile(tempFileName, 0, 0, true, "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=%s&WebEnv=%s&query_key=%s&usehistory=y", database.ptr(), webenv.ptr(), query_key.ptr());
        if( rc && rc.val.parts.bad_entity == sRC::eCertificate && rc.val.parts.state == sRC::eInvalid ) {
            reqSetInfo(rc, eQPInfoLevel_Warning, "Cannot validate certificate for url '%s', proceeding to download in insecure mode", uri);
            rc = remContent.getFile(tempFileName, 0, 0, false, "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=%s&WebEnv=%s&query_key=%s&usehistory=y", database.ptr(), webenv.ptr(), query_key.ptr());
        }
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
                sRC rc = remContent.getFile(chunkfile, &m_currFileSize, 0, true, "%s", url.ptr());
                if( rc && rc.val.parts.bad_entity == sRC::eCertificate && rc.val.parts.state == sRC::eInvalid ) {
                    reqSetInfo(rc, eQPInfoLevel_Warning, "Cannot validate certificate for url '%s', proceeding to download in insecure mode", url.ptr());
                    rc = remContent.getFile(chunkfile, &m_currFileSize, 0, false, "%s", url.ptr());
                }
                if( !rc ) {
                    idx gis = countSequenceRecords(chunkfile);
                    if( gis ) {
                        countgi += gis;
                    } else {
                        logOut(eQPLogType_Info, "Downloaded empty file of %" DEC " bytes? try %" DEC ", from %03" DEC " to %03" DEC "\n", sFile::size(chunkfile), attempt, i, i + max - 1);
                        sFile::remove(chunkfile);
                        retry |= true;
                    }
                    if( sFile::size(chunkfile.ptr()) != m_currFileSize ) {
                        reqSetInfo(rc, eQPInfoLevel_Warning, "File '%s' is different size than expected: Expected %" DEC "; Found %" DEC, url.ptr(), sFile::size(chunkfile.ptr()), m_currFileSize);
                    }
                } else {
                    logOut(eQPLogType_Error, "efetch error %s: %s\n", url.ptr(), rc.print());
                }
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

int main(int argc, const char *argv[])
{
    sStr tmp;
    sApp::args(argc, argv);

    DownloadProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "download-processor", argv[0]));
    return (int)backend.run(argc, argv);
}
