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
#include <slib/core.hpp>
#include <xlib/md5.hpp>
#include <dmlib/dmlib.hpp>
#include <signal.h>
#ifdef HAS_LIBZIP
#include <zip.h>
#endif

sRC dmRemoteFile::getFile(const char * filename, idx * size /* = 0 */, const char * wget_opts /* = 0 */, const char * url /* = 0 */, ...)
{
    sRC rc;
    sStr surl;
    sCallVarg(surl.vprintf, url);
    if( surl && filename && filename[0] ) {
#if _DEBUG
        fprintf(stderr, "%s url is '%s'\n", __func__, surl.ptr());
#endif
        const bool isRecursive = filename[sLen(filename) - 1] == '\\' || filename[sLen(filename) - 1] == '/';
        if( size ) {
            *size = 0;
            if( !isRecursive ) {
                // this request is not essential so we shorten the timeout
                sStr cmd("wget --no-check-certificate --timeout=450 --spider '%s' 2>&1", surl.ptr());
                sIO spider;
                idx ret = sPipe::exeFS(&spider, cmd, 0, m_callback, m_callbackParam, 0, m_callbackSecs);
                if( ret == 0 && spider ) {
                    const char * p = spider;
                    if( sIs("http", surl.ptr()) ) {
                        p = strstr(p, "Length:");
                        if( p ) {
                            p = p + 7;
                        }
                    } else {
                        p = strstr(p, "SIZE");
                        if( p ) {
                            p = strstr(p + 5, "...");
                        }
                        if( p ) {
                            p = p + 3;
                        }
                    }
                    if( p ) {
                        sscanf(p, "%" UDEC, size);
                    }
                }
#if _DEBUG
                fprintf(stderr, "Remote file's size is %" UDEC "\n", *size);
#endif
            }
        }
        sStr commandline("wget --no-check-certificate ");
        if( sIs(surl, "http") ) {
            commandline.printf("--tries=5 --wait=2 --continue");
        } else {
            commandline.printf("--no-verbose --retr-symlinks");
        }
        if( isRecursive ) {
            commandline.printf(" --recursive --no-parent --no-host-directories --directory-prefix='%s'", filename);
        } else {
            commandline.printf(" --output-document='%s'", filename);
        }
        commandline.printf(" %s '%s'", wget_opts ? wget_opts : "", surl.ptr());
#if _DEBUG
        fprintf(stderr, "%s cmd: '%s'\n", __func__, commandline.ptr());
#endif
        sIO output;
        idx ret = sPipe::exeFS(&output, commandline, 0, m_callback, m_callbackParam, filename, m_callbackSecs);
#if _DEBUG
        fprintf(stderr, "%s: downloaded '%s' %" DEC "\n", __func__, filename, ret);
#endif
        if( ret == 0 ) {
            rc = sDir::size(filename) > 0 ? sRC::zero : sRC(sRC::eDownload, sRC::eFile, sRC::eSize, sRC::eZero);
        } else {
            ret = (ret >> 8) & 0xFF;
#if _DEBUG
        fprintf(stderr, "%s: downloaded '%s' %" DEC "\n", __func__, filename, ret);
#endif
            switch( ret ) {
                case 2:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eCommand, sRC::eInvalid);
                    break;
                case 3:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eIO, sRC::eFailed);
                    break;
                case 4:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eNetwork, sRC::eFailed);
                    break;
                case 5:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eCertificate, sRC::eInvalid);
                    break;
                case 6:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eAuthentication, sRC::eFailed);
                    break;
                case 7:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eProtocol, sRC::eFailed);
                    break;
                case 8:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eServer, sRC::eFailed);
                    break;
                case 1:
                default:
                    rc = sRC(sRC::eDownload, sRC::eFile, sRC::eProcess, sRC::eFailed);
                    break;
            }
        }
    }
    if( rc ) {
#if _DEBUG
        fprintf(stderr, "%s: downloaded '%s' %s\n", __func__, filename, rc.print());
#endif
        sDir::removeDir(filename, true);
        sFile::remove(filename);
    }
    return rc;
}

dmLib::File::File()
    : _location(sMex::fExactSize), _path(sMex::fExactSize), _dir(sMex::fExactSize), _name(0), _size(0), _id(0)
{
    _md5[0] = '\0';
}

bool dmLib::File::init(const char * path, const char * name)
{
    if( sFile::exists(path) ) {
        _location.printf(0, "%s", path);
        _path.printf(0, "%s", name);
        _name = _path ? sFilePath::nextToSlash(_path) : 0;
        if( _name && _name != _path.ptr() ) {
            _dir.printf("%.*s", (int)(_name - _path.ptr()), _path.ptr());
        }
        _size = sFile::size(_location, false);
        if( _size < (10L * 1024 * 1024  * 1024) ) {
            sMD5 m(_location);
            strncpy(_md5, m.sum, sizeof(_md5));
            _md5[sizeof(_md5) - 1] = '\0';
        }
    }
    return _location;
}

bool dmLib::unpack(const char * path, const char * name /* = 0 */, sStr * log /* = 0 */, sStr * msg /* = 0 */, callbackFunc callb /* = 0 */, void * callbParam /* = 0 */, idx callbsecs /* = 10 */)
{
    if( path && path[0] && (sDir::exists(path) || sFile::exists(path)) ) {
        sStr dummy;
        if( !log ) {
            log = &dummy;
        }
        if( !msg ) {
            msg = &dummy;
        }
        _unpack_depth = 0;
        _collect_depth = 0;
        idx start_size = 0;
        sFilePath dd(path, "%%dir");
        clean(dd);
        if( callb ) {
            start_size = sDir::size(dd);
        }
        // final report on top level, with '-' to force update
        if( !collect(path, sFilePath::nextToSlash(name ? name : path), *log, *msg, 0, callb, callbParam, callbsecs) ||
            ( callb && !callb(callbParam, start_size - sDir::size(dd)) ) ) {
            msg->printf("Interrupted by user\n");
            return false;
        }
    } else if( log ) {
        log->printf("Path '%s' not found\n", path ? path : "");
    }
    return _list.dim() != 0;
}

#define DMLIB_TMP_PREFIX "dm"

bool dmLib::collect(const char * path, const char * meta, sStr & log, sStr & msg, idx level, callbackFunc callb, void * callbd, idx callbSecs)
{
    sStr cleaned(sMex::fExactSize);
    if (_unpack_depth < _max_unpack_depth){
        if( !sDir::cleanUpName(path, cleaned, true) ) {
            msg.printf("Internal error unpack [%u]\n", __LINE__);
            log.printf("Failed to cleanup name '%s', skipped\n", path);
            return false;
        }
        if( strcmp(path, cleaned) ) {
            log.printf("%4" DEC " '%s' renamed to safe '%s'\n", level, path, cleaned.ptr());
        }
        path = cleaned;
    }
    bool retval = true;
    if( sDir::exists(path) ) {
        log.printf("%4" DEC " directory '%s' meta: '%s'\n", level, path, meta);
        sDir dir;
        // follow links only on top level since actual request to process data could consist of links
        const idx links = level ? sFlag(sDir::bitNoLinks) : 0;
        dir.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | links, path, "*");
        sStr m(sMex::fExactSize);
        m.printf("%s%s", meta, (meta && meta[0]) ? "/" : "");
        idx mpos = m.pos();
        for(const char * p = dir; p && retval; p = sString::next00(p)) {
            if( p && *p ) {
                m.printf(mpos, "%s", sFilePath::nextToSlash(p));
                ++_collect_depth;
                retval = collect(p, m, log, msg, level + 1, callb, callbd, callbSecs);
                --_collect_depth;
            }
        }
    } else {
        static sDic<sStr> decompressors;
        if( !decompressors.dim() ) {
            // when adding new tool check if its successful exit code is zero and adjust command line!!!
            decompressors.set(".bz2")->printf("bzip2 -cd \"%%s\" >\"%%s\"");
            decompressors.set(".bzip2")->printf("bzip2 -cd \"%%s\" >\"%%s\"");
            decompressors.set(".tbz2")->printf("bzip2 -cd \"%%s\" >\"%%s.tar\"");
            decompressors.set(".gz")->printf("gzip -cd \"%%s\" >\"%%s\"");
            decompressors.set(".gzip")->printf("gzip -cd \"%%s\" >\"%%s\"");
            decompressors.set(".tgz")->printf("gzip -cd \"%%s\" >\"%%s.tar\"");
            decompressors.set(".tar")->printf("tar --overwrite -xvf \"%%s\"");
            decompressors.set(".zip")->printf("unzip -o \"%%s\"");
#if HAS_SRA // more like if whole adoption is present
            decompressors.set(".sra")->printf("sra2fastq.os" SLIB_PLATFORM" \"%%s\"");
            decompressors.set(".bam")->printf("samtools view -h \"%%s\" > \"%%s.sam\"");
            decompressors.set(".bcl")->printf("bcl2fastq2.sh.os" SLIB_PLATFORM" \"%%s\" \"%%s\"");
            // TODO decompressors.set(".fast5")->printf("poretools fastq \"%%s\" \"%%s\"");
#endif
            decompressors.set(".hivepack-zip")->printf("unzip -o \"%%s\"");
        }
        // get LAST extension
        const char * ext = strrchr(path, '.');
        sStr * dc = decompressors.get(ext);
        if( dc && _unpack_depth < _max_unpack_depth) {
            sFilePath workDir(path, "%%dir/" DMLIB_TMP_PREFIX "%" DEC "-%%flnm/", level);
            sDir::removeDir(workDir);
            if( sDir::makeDir(workDir) ) {
                // only needed on rear occasion of curtain file extensions above
                // like .tgz when we need to say that output file will be .tar
                sFilePath outFile(path, "%%flnmx");
                sFilePath cmd(meta, dc->ptr(), path, outFile.ptr());
                sStr cmdLine("cd \"%s\" && %s", workDir.ptr(), cmd.ptr());
                log.printf("Decompressing: '%s'...\n", cmdLine.ptr());
                sIO io;
                sFilePath watch(path, "%%dir");
                idx ret = sPipe::exeFS(&io, cmdLine, 0, callb, callbd, watch, callbSecs);
                log.printf("%s", io.ptr());
                if( ret ) {
                    msg.printf("(%" DEC ") File '%s' corrupt or incomplete: %" DEC " bytes\n", ret, sFilePath::nextToSlash(meta), sFile::size(path));
                    log.printf("\nDecompressing FAILED [%" DEC "] '%s'\n", ret, cmdLine.ptr());
                    sDir::removeDir(workDir);
                } else {
                    log.printf("\nDecompressing OK '%s'\n", path);
                    sDir qtyFile, qtyAll;
                    qtyFile.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitNamesOnly), workDir, "*");
                    qtyAll.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | sFlag(sDir::bitNamesOnly), workDir, "*");
                    if( sString::cnt00(qtyFile) == sString::cnt00(qtyAll) && sString::cnt00(qtyFile) == 1 ) {
                        char * sl = sFilePath::nextToSlash(meta);
                        // remove self prefix if container has single file which name starts with name of the container
                        if( sl ) {
                            sFilePath s(meta, "%%flnmx"), q(qtyFile, "%%flnmx"), qx(qtyFile, "%%flnm");
                            if( s && ((q && strcasecmp(s, q) == 0) || (qx && strcasecmp(s, qx) == 0)) ) {
                                log.printf("%4" DEC " dropped archive name from meta '%s' vs '%s'\n", level, s.ptr(), q.ptr());
                                *sl = '\0';
                            }
                        }
                    }
                    if( callb ) {
                        retval = callb(callbd, -1);
                    }
                    ++_unpack_depth;
                    retval = !retval ? retval : collect(workDir, meta, log, msg, level + 1, callb, callbd, callbSecs);
                    --_unpack_depth;
                }
            } else {
                msg.printf("An error occurred while unpacking file '%s'\n", sFilePath::nextToSlash(meta));
                log.printf("mkdir FAILED (disk free space: %" UDEC ") '%s'\n", sDir::freeSpace(workDir), workDir.ptr());
            }
        } else if( _collect_depth < _max_collect_depth ) {
            File * f = _list.add();
            if( f ) {
                f->init(path, meta);
                if( callb ) {
                    retval = callb(callbd, f->size());
                }
                f->_id = _list.dim();
                log.printf("%4" DEC " added file[%" DEC "] '%s' meta: '%s'\n", level, f->_id, path, meta);
            } else {
                log.printf("Failed to allocate File object '%s'\n", meta);
            }
        }
    }
    return retval;
}

// static
void dmLib::clean(const char * path)
{
    sDir dir;
    dir.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs), path, DMLIB_TMP_PREFIX "[0-9]-*");
    for(const char * p = dir; p; p = sString::next00(p)) {
        if( !sDir::removeDir(p, true) ) {
            sFile::remove(p);
        }
    }
}

// static
idx dmLib::arcDim(const char * path)
{
    idx ret = -1;
    const char * ext = path ? strrchr(path, '.') : 0;
    if( ext ) {
        ++ext;
#ifdef HAS_LIBZIP
        if( strcasecmp(ext, "zip") == 0 ) {
            struct zip * zz  = zip_open(path, 0, 0);
            if( zz ) {
                ret = zip_get_num_entries(zz, 0);
                zip_close(zz);
            }
        } else
#endif
                if ( strcasecmp(ext, "gz") == 0 || strcasecmp(ext, "bz2") == 0 || strcasecmp(ext, "bzip2") == 0 ) {
                ret = 1;
        }
    }
    return ret;
}

// static
bool dmLib::pack(const char * srcPath, sStr & dstPath, EPackAlgo algo, sStr * log /* = 0 */, sStr * msg /* = 0 */, sStr * name /* = 0 */, callbackFunc callb /* = 0 */, void * callbParam /* = 0 */, idx callbSecs /* = 10 */, bool deletePackedFiles /* = false */, bool packAllFiles /* = false */)
{
    if( !srcPath[0] || !dstPath ) {
        return false;
    }
    sFilePath dstName;
    if( sDir::exists(dstPath) ) {
        dstName.makeName(srcPath, "%s/%%flnm", dstPath.ptr());
        dstName.shrink00();
    } else {
        dstName.printf("%s", dstPath.ptr());
    }
    char * dot = strrchr(dstName, '.');
    sStr cmd;
    switch(algo) {
        case eNone:
            break;
        case eTar:
            if( dot ) {
                dstName.cut(dot - dstName.ptr());
            }
            dstName.printf(".tar");
            cmd.printf("tar cf \"%s\" \"%s\"", dstName.ptr(), srcPath);
            break;
        case eGzip:
            if( sDir::exists(srcPath) ) {
                dstName.printf(".tar.gz");
                cmd.printf("tar zcf \"%s\" \"%s\"", dstName.ptr(), srcPath);
            } else {
                dstName.printf(".gz");
                cmd.printf("gzip -c \"%s\" > \"%s\"", srcPath, dstName.ptr());
            }
            break;
        case eBzip2:
            if( sDir::exists(srcPath) ) {
                dstName.printf(".tar.bz2");
                cmd.printf("tar jcf \"%s\" \"%s\"", dstName.ptr(), srcPath);
            } else {
                dstName.printf(".bz2");
                cmd.printf("bzip2 --compress -c \"%s\" > \"%s\"", srcPath, dstName.ptr());
            }
            break;
        case eZip:
            if( dot ) {
                dstName.cut(dot - dstName.ptr());
            }
            dstName.printf(".zip");
            if( sDir::exists(srcPath) ) {
                if (deletePackedFiles) {
                    cmd.printf("cd \"%s\" && zip -prym \"%s\" .", srcPath, dstName.ptr());
                } else {
                    cmd.printf("cd \"%s\" && zip -pry \"%s\" .", srcPath, dstName.ptr());
                }
            } else {
                if (deletePackedFiles) {
                    cmd.printf("zip -jm \"%s\" %s", dstName.ptr(), (packAllFiles ? "*" : srcPath));
                } else {
                    cmd.printf("zip -j \"%s\" %s", dstName.ptr(), (packAllFiles ? "*" : srcPath));
                }
            }
            break;
        default:
            break;
    }
    idx ret = 255;
    if( cmd ) {
        if( log ) {
            log->printf("Compressing: '%s'...\n", cmd.ptr());
        }
        sFile::remove(dstName);
        sIO output;
        ret = sPipe::exeFS(&output, cmd, 0, callb, callbParam, dstName, callbSecs);
        if( callb ) {
            callb(callbParam, -1);
        }
        if( log && output ) {
            log->printf("%s", output.ptr());
        }
        if( ret ) {
            if( msg ) {
                msg->printf("Compression failed (%" DEC "): '%s' \n", ret, sFilePath::nextToSlash(srcPath));
            }
            if( log ) {
                log->printf("Compressing FAILED [%" DEC "]: '%s'\n", ret, cmd.ptr());
            }
        } else if( log ) {
            log->printf("Compression done: '%s'\n", cmd.ptr());
        }
    } else if( algo == eNone ) {
        if( sDir::exists(srcPath) ) {
            ret = (sDir::makeDir(dstName) && sDir::copyDir(srcPath, dstName, false)) ? 0 : 567;
        } else {
            ret = sFile::copy(srcPath, dstName) ? 0 : 123;
        }
        log->printf("Copying '%s' to '%s' %s\n", srcPath, dstName.ptr(), ret ? "failed" : "ok");
    } else {
        log->printf("Unknown pack algorithm requested: %u\n", algo);
    }
    if( ret == 0 && name ) {
        name->printf("%s", dstName.ptr());
    }

    // Changed logic to move in the zip cmd instead of deleting here
//    if (deletePackedFiles) {
//        // Get file list for removal
//        sStr searchExt("*.*");
//        sDir results;
//        results.list(sFlag(sDir::bitFiles), srcPath, searchExt.ptr(), 0, 0);
//        for(const char * ptr = results; ptr && *ptr; ptr = sString::next00(ptr)) {
//            if (strcmp(ptr,dstName.ptr())==0) continue;
//            sFile::remove(ptr);
//        }
//    }
    return ret == 0;
}
