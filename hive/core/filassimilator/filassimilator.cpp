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
#include <slib/utils/json/parser.hpp>
#include <slib/std/app.hpp>
#include <slib/core/rc.hpp>
#include <ulib/ufile.hpp>
#include <ulib/ufolder.hpp>
#include <xlib/md5.hpp>
#include <xlib/sha256.hpp>

#ifdef HAS_IMAGEMAGICK
#include <xlib/image.hpp>
#endif
#include <xlib/xls2csv/Xls.h>

namespace slib
{
    class sFilAssimilator : public sQPrideProc
    {
        public:
            sFilAssimilator(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv)
            {
            }

            typedef struct __sCopyData {
                    __sCopyData(udx sz, sQPrideProc * Q)
                        : count(0),
                          size(sz),
                          qp(Q)
                    {
                    }
                    udx count, size;
                    sQPrideProc * qp;
                    sMD5 md5;
                    sSHA_256 sha256;
            } sCopyData;

            static bool copyCallback(void * callbackParam, const char * buffer, const idx len)
            {
                sCopyData * data = static_cast<sCopyData *>(callbackParam);
                if( data ) {
                    data->count += len;
                    if( !data->qp->reqProgress(data->count, data->count / data->size, data->size) ) {
                        return false;
                    }
                    data->md5.parse_buffer(buffer, len);
                    data->sha256.parse_buffer(buffer, len);
                    return true;
                }
                return false;
            }

            bool castObj(std::unique_ptr<sUsrObj> & obj, const sUsrType2 * type)
            {
                sUsrObj * pi = obj->cast(type->name());
                if( pi && sIsExactly(type->name(), pi->getTypeName()) ) {
                    obj.reset(pi);
                } else {
                    delete pi;
                }
                return obj.get() == pi;
            }

            const sUsrType2 * detectType(const char * ext)
            {
                static sDic<sStr> cfg;

                if( cfg.dim() == 0 ) {
                    sStr s("%s.ext", svc.name), txt;
                    cfgStr(&txt, 0, s);
                    sJSONParser parser;
                    if( !parser.parse(txt.ptr(0)) && !parser.result().isDic() ) {
                        reqSetInfo(reqId, eQPInfoLevel_Warning, "Parsing extension table failed");
                    } else {
                        sVariant & json = parser.result();
                        for( idx i = 0; i < json.dim(); ++i ) {
                            sVariant * l = json.getDicElt(i);
                            if( l && l->isList() ) {
                                for( idx e = 0; e < l->dim(); ++e ) {
                                    sVariant * ext = l->getListElt(e);
                                    if( sLen(ext->asString()) > 0 ) {
                                        sStr * t = cfg.setString(ext->asString());
                                        if( t ) {
                                            t->printf(0, "%s", json.getDicKey(i));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                sStr * name = cfg.get(ext, sLen(ext));
                if( name && sLen(name->ptr()) ) {
                    return sUsrType2::ensure(*user, name->ptr());
                }
                return nullptr;
            }

            sRC setupFolderTree(sDic<bool> & paths) {
                std::unique_ptr<sUsrFolder> fobj;
                sHiveId folder(formValue("folder"));
                if( !folder && user ) {
                    if( objs.dim() ) {
                        sUsrObjRes res;
                        user->objs2("^folder$+", res, 0, "child", objs[0].IdStr(), "", false, 0, 1);
                        if( res.dim()) {
                            folder = *res.firstId();
                        }
                    }
                    fobj.reset( folder ? new sUsrFolder(*user, folder) : sSysFolder::Inbox(*user));
                }
                if( fobj.get() ) {
                    reqSetData(grpId, "_folder", fobj->IdStr());
                    for(idx i = 0; i < paths.dim(); ++i) {
                        sUsrFolder * f = fobj->createSubFolder("%s", (const char*)(paths.id(i)));
                        if( f == nullptr ) {
                            return RC(sRC::eCreating, sRC::eDirectory, sRC::eOperation, sRC::eFailed, "%s", (const char*)(paths.id(i)));
                        }
                    }
                    return sRC::zero;
                }
                return RC(sRC::eCreating, sRC::eDirectory, sRC::eParent, sRC::eNotFound);
            }

            virtual sRC OnSplit(idx, idx &cnt) {
                sStr inPath;
                sRC rc;
                do {
                    formValue("path", &inPath);
                    if( !inPath || inPath[0] == '\0' ) {
                        RCSET(rc, sRC::eAccessing, sRC::eParameter, sRC::eValue, sRC::eEmpty, "path");
                        break;
                    }
                    if( inPath[0] != '/' ) {
                        sStr tmp;
                        cfgStr(&tmp, 0, "user.download");
                        if( tmp ) {
                            tmp.printf("%s", inPath.ptr());
                            inPath.printf(0, "%s", tmp.ptr());
                        } else {
                            RCSET(rc, sRC::eReading, sRC::eConfig, sRC::eValue, sRC::eEmpty, "user.download");
                            break;
                        }
                    }
                    sDir fileList;
                    idx path_pfx = 0, qty = 0;
                    const bool isDir = sDir::exists(inPath.ptr());
                    if( isDir ) {
                        path_pfx = inPath.length() - 1;
                        reqSetData(grpId, "_prefix", inPath.ptr());
                        qty = fileList.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitEntryFlags), inPath.ptr());
                        qty += fileList.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitEntryFlags), inPath.ptr(), ".*");
                    } else if( sFile::exists(inPath.ptr()) ) {
                        sFilePath dir(inPath.ptr(), "%%dir");
                        reqSetData(grpId, "_prefix", dir.ptr());
                        qty = fileList.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitEntryFlags), dir.ptr(), sFilePath::nextToSlash(inPath.ptr()));
                    } else {
                        RCSET(rc, sRC::eAccessing, sRC::eDirectory, sRC::eFile, sRC::eNotFound, "[path]");
                        break;
                    }
                    sVec<idx> fvec;
                    sStr files;
                    sDic<bool> paths;
                    fvec.resize(qty);
                    for(idx i = 0; i < fileList.dimEntries(); ++i) {
                        const char * flnm = fileList.getEntryPath(i);
                        sFil file(flnm, sMex::fReadonly);
                        if( file.ok() ) {
                            fvec[i] = files.add(flnm, sLen(flnm) + 1) - files.ptr();
                        } else {
                            logOut(eQPLogType_Error, "Cant'read file %s, maybe more file like this", flnm);
                            RCSET(rc, sRC::eReading, sRC::eFile, sRC::ePermission, sRC::eInsufficient, "%s", &flnm[path_pfx]);
                            break;
                        }
                        if( isDir ) {
                            sFilePath dir(&flnm[path_pfx], "%%dir");
                            if( sLen(dir.ptr()) ) {
                                paths.setString(dir.ptr());
                            }
                        }
                    }
#if _DEBUG
                    for(idx k = 0; k < qty; ++k) {
                        logOut(eQPLogType_Trace, "[%" DEC "]='%s'", k, files.ptr(fvec[k]));
                    }
#endif
                    if( paths.dim() ) {
                        sRC rc = setupFolderTree(paths);
                        if( rc.isSet() ) {
                            reqSetInfo(reqId, eQPInfoLevel_Warning, "folder structure was not created, all files will be in All Objects: %s", rc.print());
                        }
                    }
                    reqSetData(grpId, "_index", fvec.mex());
                    reqSetData(grpId, "_files", files.mex());
                    cnt = qty;
                } while( false );
                return rc;
            }

            virtual idx OnExecute(idx)
            {
                sVec<idx> fvec;
                sStr files, buf;
                reqGetData(grpId, "_prefix", buf.mex());
                reqGetData(grpId, "_index", fvec.mex());
                reqGetData(grpId, "_files", files.mex());
                const char * src_file = files.ptr(fvec[reqSliceId]);
                const char * file_path = &src_file[buf.length()];
                buf.cut0cut();
#if _DEBUG
                logOut(eQPLogType_Trace, "%s", file_path);
#endif
                bool success = false;
                std::unique_ptr<sUsrObj> obj;
                const char * ext = strrchr(file_path, '.');
                ext = ext ? ext + 1 : "";
                do {
                    reqProgress(-1, 1, 100);
                    sHiveId id;
                    sRC rc = user->objCreate(id, "u-file");
                    if( rc.isSet() ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "%s", rc.print());
                        break;
                    }
                    obj.reset(user->objFactory(id));
                    if( !obj.get() || !obj->Id() ) {
                        logOut(eQPLogType_Error, "Object %s not found or access denied", id.print());
                        break;
                    }
                    selfDestruct(*obj, true);
                    reqProgress(-1, 3, 100);
                    const udx file_sz = sFile::size(src_file);
                    const char * name = sFilePath::nextToSlash(file_path);
                    sCopyData data(file_sz, this);
                    if( !obj->addFilePathname(buf, true, ".%s", ext) || !(sFile::copy(src_file, buf, false, true, copyCallback, &data) || file_sz == 0) ) {
                        logOut(eQPLogType_Error, "failed to copy %s", src_file);
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to save file '%s' to object %s", file_path, obj->IdStr());
                        break;
                    }
                    reqProgress(-1, 97, 100);
                    udx q = obj->propSet("orig_name", name);
                    q += obj->propSet("name", name);
                    q += obj->propSet("ext", ext);
                    q += obj->propSetU("size", file_sz);
                    q += obj->propSet("md5", data.md5.sum());
                    q += obj->propSet("sha256", data.sha256.sum());
                    if( q != 6 ) {
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to set object properties");
                        break;
                    }
                    reqGetData(grpId, "_folder", buf.mex(), true);
                    if( buf ) {
                        sHiveId fid(buf.ptr());
                        sUsrFolder fld(*user, fid);
                        if( fld.Id() ) {
                            sFilePath dir(file_path, "%%dir");
                            if( sLen(dir) ) {
                                sUsrFolder * x = fld.find(dir.ptr());
                                if( x ) {
                                    x->attach(*obj.get());
                                }
                            } else {
                                fld.attach(*obj.get());
                            }
                        }
                    }
                    reqSetData(reqId, "_obj", "%s", obj->IdStr());

                    typeAnalisys(ext, src_file, obj, name);
                    obj->cleanup();
                    reqProgress(-1, 99, 100);
#if _DEBUG
                    logOut(eQPLogType_Trace, "object %s", obj->IdStr());
#endif
                    logOut(eQPLogType_Debug, "Object %s <- %s", obj->IdStr(), file_path);
                    selfDestruct(*obj, false);
                    success = true;
                } while( false );
                if( !success ) {
                    reqSetInfo(reqId, eQPInfoLevel_Warning, "Object deleted for '%s'", file_path);
                    obj->purge();
                } else if( isLastInGroup(svc.name) ) {
                    sVec<idx> reqs;
                    grp2Req(grpId, &reqs);
                    sVec<sHiveId> ids;
                    ids.resize(reqs.dim());
                    idx k = 0;
                    for( idx i = 0; i < reqs.dim(); ++i ) {
                        sHiveId id(reqGetData(reqs[i], "_obj", buf.mex()));
                        if( id ) {
                            ids[k++] = id;
                        }
                    }
                    ids.cut(k);
                    if( objs[0].sUsrObj::propSetHiveIds("objid", ids) == ids.dim() ) {
                        reqSetInfo(reqId, eQPInfoLevel_Info, "Assimilated %" DEC " files", ids.dim());
#if !_DEBUG
                        if( !formBoolValue("keep_source") ) {
                            const char * path = formValue("path");
                            sDir::removeDir(path);
                        }
#endif
                    } else {
                        success = false;
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to set outputs");
                    }
                }
                reqProgress(-1, 100, 100);
                reqSetStatus(reqId, success ? eQPReqStatus_Done : eQPReqStatus_ProgError);
                return 0;
            }

            void typeAnalisys(const char * ext, const char * file, std::unique_ptr<slib::sUsrObj> & obj, const char * name)
            {
                const sUsrType2 * type = detectType(ext);
                if( type ) {
#ifdef HAS_IMAGEMAGICK
                    if( type->isDescendentOf("image") ) {
                        sImage img(file);
                        if( img.ok() ) {
                            if( castObj(obj, type) ) {
                                obj->propSetU("height", img.height());
                                obj->propSetU("width", img.width());
                                obj->propSetR("x_res", img.xResolution());
                                obj->propSetR("y_res", img.yResolution());
                                if( img.taken() ) {
                                    obj->propSetDTM("taken", img.taken());
                                }
                                subImage(*obj, img, "icon", "png", 64);
                                subImage(*obj, img, "thumb", "png", 800);
                            } else {
                                reqSetInfo(reqId, eQPInfoLevel_Warning, "Image format analisys failed '%s'", name);
                            }
                        } else {
                            reqSetInfo(reqId, eQPInfoLevel_Warning, "Unsupported image format '%s'", name);
                        }
                    } else
#endif
                    if( type->isDescendentOf("excel-file") ) {
                        sStr path, err;
                        obj->addFilePathname(path, true, "~tmp");
                        path.cut0cut(-4);
                        if( Xls::excel2csv(file, path, &err) < 1 ) {
                            reqSetInfo(reqId, eQPInfoLevel_Warning, "Failed to parse MS Excel file '%s' %s", name, err.ptr());
                        } else if( !castObj(obj, type) ) {
                            sDir csv_list;
                            csv_list.list(sFlag(sDir::bitFiles), path.ptr(), "*.csv");
                            for( idx i = 0; i < csv_list.dimEntries(); ++i ) {
                                sFile::remove(csv_list.getEntryPath(i));
                            }
                        }
                    } else if( !castObj(obj, type) ) {
                        reqSetInfo(reqId, eQPInfoLevel_Warning, "Failed to set object type");
                    }
                }
            }

#ifdef HAS_IMAGEMAGICK
            bool subImage(sUsrObj & obj, sImage & img, const char * name, const char * type, const udx dim)
            {
                if( name && type ) {
                    sStr nm("%s.%s", name, type), pbuf;
                    if( obj.addFilePathname(pbuf, true, "%s", nm.ptr()) ) {
                        sFilePath tmp(img.filename(), "%%dir/~%s-1-%s", obj.Id().print(), nm.ptr());
                        std::unique_ptr<sImage> icon(img.convert(tmp, type));
                        if( icon.get() ) {
                            if( (icon->width() > dim || icon->height() > dim) && ((dim * 1. / icon->width()) < .1 || (dim * 1. / icon->height()) < .1) && (((icon->width() * 1.0 / icon->height()) > 5) || ((icon->height() * 1.0 / icon->width()) > 5)) ) {
                                udx l = 0, t = 0, w, h;
                                if( icon->width() > icon->height() ) {
                                    l = (icon->width() - icon->height()) / 2;
                                    w = h = icon->height();
                                } else {
                                    t = (icon->height() - icon->width()) / 2;
                                    w = h = icon->width();
                                }
                                sFilePath tmp1(img.filename(), "%%dir/~%s-2-%s", obj.Id().print(), nm.ptr());
                                sImage * n = icon->crop(tmp1, l, t, w, h);
                                if( n ) {
                                    sFile::remove(icon->filename());
                                    icon.reset(n);
                                } else {
                                    icon.reset(0);
                                }
                            }
                            if( icon.get() ) {
                                udx w = 0, h = 0;
                                sImage::EAspect asp;
                                if( icon->width() < dim && icon->height() < dim ) {
                                    w = icon->width();
                                    h = icon->height();
                                    asp = sImage::eAspectExact;
                                } else if( icon->width() >= icon->height() ) {
                                    w = dim;
                                    asp = sImage::eAspectWidth;
                                } else {
                                    h = dim;
                                    asp = sImage::eAspectHeight;
                                }
                                sImage * n = icon->resize(pbuf, w, h, asp);
                                if( n ) {
                                    sFile::remove(icon->filename());
                                    icon.reset(n);
                                } else {
                                    icon.reset(0);
                                }
                            }
                        }
                        if( icon.get() && icon->ok() ) {
                            return obj.propSet(name, nm) == 1;
                        }
                    }
                }
                return false;
            }
#endif

        private:
            void selfDestruct(sUsrObj & obj, const bool on)
            {
                user->objSetHidden(obj.Id(), on);
                obj.actDelete(on ? 30 : -1);
            }
    };
};

using namespace slib;

int main(int argc, const char * argv[])
{
    sApp::args(argc, argv);
    sStr tmp;
    sQPrideProc::QPrideSrvName(&tmp, "filassimilator", argv[0]);
    sFilAssimilator backend("config=qapp.cfg" __, tmp);
    return (int)backend.run(argc, argv);
}
