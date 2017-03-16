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
#include <slib/utils/tbltbl.hpp>
#include <qlib/QPrideProc.hpp>
#include <ulib/ulib.hpp>
#include <xlib/md5.hpp>

#ifdef HAS_IMAGEMAGICK
#include <xlib/image.hpp>
#endif
#include <xlib/xls2csv/Xls.h>
#include <dmlib/dmlib.hpp>
#include <qpsvc/archiver.hpp>
#include <qpsvc/dna-parser.hpp>
#include <qpsvc/qpsvc-dna-align-parser.hpp>
#include <qpsvc/compressor.hpp>
#include <qpsvc/annot.hpp>
#include <ulib/utype2.hpp>

typedef struct TObjsProp_struct
{
        sHiveId id;
        idx pos;
        static sStr props;
} TObjsProp;

sStr TObjsProp::props;

namespace slib {
    class dmArchiverProc: public sQPrideProc
    {
        public:
            dmArchiverProc(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv)
            {
            }
            virtual idx OnExecute(idx);

            bool loadProp(idx req, sStr * outlog, const char * file_location, const char * file_path, sDic<sHiveId> * new_ids_map = 0)
            {
                sStr log;
                outlog = outlog ? outlog : &log;
                logOut(eQPLogType_Info, "Parsing property table '%s' %" DEC " bytes", file_location, sFile::size(file_location));
                sVec<sHiveId> uids;
                sDic<sHiveId> nmap;
                new_ids_map = new_ids_map ? new_ids_map : &nmap;
                bool ok = user->propSet(file_location, log, 0, &uids, new_ids_map);
                if( !ok ) {
                    sStr l;
                    sString::searchAndReplaceSymbols(&l, log.ptr(), log.length(), "\n", 0, 0, true, true, false, true);
                    for(const char * p = l.ptr(); p; p = sString::next00(p)) {
                        reqSetInfo(req, eQPInfoLevel_Error, "File '%s' load failed: %s", file_path, p);
                    }
                } else {
                    const udx qty = uids.dim() + new_ids_map->dim();
                    log.cut(0);
                    if( new_ids_map->dim() ) {
                        outlog->printf("new ids:");
                        for(idx i = 0; i < new_ids_map->dim(); ++i) {
                            outlog->printf(" %s->", (const char *) new_ids_map->id(i));
                            new_ids_map->ptr(i)->print(*outlog);
                            std::auto_ptr<sUsrObj> o(user->objFactory(*(new_ids_map->ptr(i))));
                            if( o.get() ) {
                                toFolder(req, qty > 1 ? file_path : 0, *(o.get()));
                            }
                        }
                    }
                    if( uids.dim() ) {
                        outlog->printf(" updated ids:");
                        for(idx i = 0; i < uids.dim(); ++i) {
                            outlog->printf(" ");
                            uids[i].print(*outlog);
                            std::auto_ptr<sUsrObj> o(user->objFactory(uids[i]));
                            if( o.get() ) {
                                toFolder(req, qty > 1 ? file_path : 0, *(o.get()));
                            }
                        }
                    }
                    if( outlog == &log ) {
                        reqSetInfo(req, eQPInfoLevel_Info, "%s", log.ptr());
                    }
                }
                return ok;
            }

            bool toFolder(const idx req, const char * path, const sUsrObj & obj) {
                if( !_folder.get() ) {
                    sHiveId folder(formValue("folder"));
                    if( !folder && objs.dim() && user ) {
                        // grab folder from process object
                        sUsrObjRes res;
                        user->objs2("folder", res, 0, "child", objs[0].IdStr(), "", false, 0, 1);
                        if(  res.dim()) {
                            folder = *res.firstId();
                        }
                    }
                    _folder.reset( folder ? new sUsrFolder(*user, folder) : sSysFolder::Inbox(*user));
                }
                sUsrFolder * folder = _folder.get();
                sStr folderName;
                if( folder ) {
                    if( path && path[0] ) {
                        sUsrFolder * subFolder = folder->createSubFolder(path);
                        if( subFolder ) {
                            folder = subFolder;
                        } else {
                            folderName.printf("%s/%s", folder->name(), path);
                            folderName.add0();
                            folderName.printf("%s", folder->name());
                        }
                    }
                    folder->attach(obj);
                    if( folder != _folder.get() ) {
                        delete folder;
                    }
                } else {
                    folderName.printf("specified");
                    folderName.add0();
                    folderName.printf("All Objects");
                }
                if( folderName ) {
                    reqSetInfo(req, eQPInfoLevel_Warning, "Object %s could not be placed in %s and is available in '%s'", obj.Id().print(), folderName.ptr(), sString::next00(folderName));
                    return false;
                }
                return true;
            }
#ifdef HAS_IMAGEMAGICK
            bool subImage(sUsrObj & obj, sImage & img, const char * name, const char * type, const udx dim)
            {
                if( name && type ) {
                    sStr nm("%s.%s", name, type), pbuf;
                    if( obj.addFilePathname(pbuf, true, "%s", nm.ptr()) ) {
                        sFilePath tmp(img.filename(), "%%dir/~%s-1-%s", obj.Id().print(), nm.ptr());
                        std::auto_ptr<sImage> icon(img.convert(tmp, type));
                        if( icon.get() ) {
                            if( (icon->width() > dim || icon->height() > dim) &&
                                ((dim * 1. / icon->width()) < .1 || (dim * 1. / icon->height()) < .1) &&
                                (((icon->width() * 1.0 / icon->height()) > 5) || ((icon->height() * 1.0 / icon->width()) > 5)) ) {
                                udx l = 0, t = 0, w, h;
                                // square down to a lesser dimension
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
            void addProps(sUsrObj & obj, const char * properties1, const char * properties2)
            {
                // set properties defined by the submitter or by extension table: nm1=val1[,nm2=val2,...]
                logOut(eQPLogType_Debug, "Property for obj %s '%s','%s'", obj.Id().print(), properties1 ? properties1 : "" , properties2 ? properties2 : "");
                sTxtTbl props1;
                props1.setBuf(properties1, sLen(properties1));
                props1.parseOptions().flags = sTblIndex::fSaveRowEnds;
                props1.parse();
                sTxtTbl props2;
                props2.setBuf(properties2, sLen(properties2));
                props2.parseOptions().flags = sTblIndex::fSaveRowEnds;
                props2.parse();
                sCatTabular all;
                all.pushSubTable(&props1, false);
                all.pushSubTable(&props2, false);
                logOut(eQPLogType_Debug, "Property for obj %s rows %" DEC " cols %" DEC, obj.Id().print(), all.rows(), all.cols());
                for(idx ir = 0; ir < all.rows(); ++ir) {
                    if( all.cell(ir, 0, 0) ) {
                        sStr buf1, buf2;
                        all.printCell(buf1, ir, 0);
                        all.printCell(buf2, ir, 1);
                        if( buf1 && buf1.ptr()[0] ) {
                            logOut(eQPLogType_Debug, "Property set for obj %s '%s'='%s'", obj.Id().print(), buf1.ptr(), buf2.ptr());
                            if( obj.propSet(buf1, buf2) != 1 ) {
                                logOut(eQPLogType_Warning, "Property '%s'='%s' is not assigned to '%s' type object", buf1.ptr(), buf2.ptr(), obj.getTypeName());
                            }
                        } else if( buf2 && buf2[0] ) {
                            logOut(eQPLogType_Warning, "Property for obj %s with value '%s' has empty name", obj.Id().print(), buf2.ptr());
                        }
                    }
                }
            }
            bool isFinal(idx progress, sVec<TObjsProp> & objects, const char * properties, const char* inPath)
            {
                if( progress >= 0 ) {
                    // save intermediate data
                    reqSetData(reqId, "progress", "%" DEC, progress);
                    if( objects.dim() ) {
                        reqSetData(reqId, "objects", objects.mex());
                        reqSetData(reqId, "objects_props", TObjsProp::props.mex());
                    }
                } else {
                    sStr final;
                    reqGetData(reqId, "progress", final.mex());
                    if( !final ) {
                        return false;
                    }
                    sscanf(final, "%" DEC, &progress);
                }
                // wait for all in group to finish
                bool keepSrc = false;
                sVec<sQPrideBase::Request> r;
                requestGetForGrp(grpId, &r);
                serviceID(); // init self svcID
                for(idx ri = 0; ri < r.dim(); ++ri) {
                    const idx st = r[ri].stat;
                    // do not wait for other archivers in the group, they must be parallel to this one
                    if( r[ri].svcID != svcID && st <= eQPReqStatus_Running ) {
                        // some are not done yet
                        reqReSubmit(reqId, 60);
                        reqProgress(-1, 99, 100);
                        logOut(eQPLogType_Debug, "waiting for %" DEC " to finish ", r[ri].reqID);
                        return true;
                    }
                    // do not cleanup download area in case smth on hold or went wrong
                    keepSrc |= st == eQPReqStatus_Suspended || st > eQPReqStatus_Done;
                }
                // assign submitter properties on more time since they might be for specific type and were not assigned above
                objects.empty();
                reqGetData(reqId, "objects", objects.mex());
                if( objects.dim() ) {
                    TObjsProp::props.cut0cut();
                    reqGetData(reqId, "objects_props", TObjsProp::props.mex());
                }
                for(idx i = 0; i < objects.dim(); ++i) {
                    std::auto_ptr<sUsrObj> obj(user->objFactory(objects[i].id));
                    if( obj.get() ) {
                        addProps(*obj.get(), properties, TObjsProp::props.ptr(objects[i].pos));
                    }
                }
                if( !keepSrc ) {
                    if( sDir::exists(inPath) ) {
                        sDir::removeDir(inPath, true);
                    } else {
                        sFilePath dir(inPath, "%%dir");
                        sFile::remove(inPath);
                        dmLib::clean(dir);
                        // if something left in 'dir' (next to original source), do not erase it!
                        sDir::removeDir(dir, false);
                    }
                }
                reqProgress(-1, progress, 100);
                reqSetStatus(reqId, progress == 100 ? eQPReqStatus_Done : eQPReqStatus_ProgError);
                return true;
            }
            std::auto_ptr<sUsrFolder> _folder;
    };
}

using namespace slib;

idx dmArchiverProc::OnExecute(idx req)
{
    sStr inPath, inName, ds, properties, formatHint, convert2Type, sExmap;
    formValue("properties", &properties);
    formValue("inputFile", &inPath);
    if( !inPath ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Missing input");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    const udx dissect = dmArchiver::getDepth(*this);
    const idx run_index = dmArchiver::getIndexFlag(*this);
    formValue("inputName", &inName);
    if( !inName ) {
        inName.printf("%s", sFilePath::nextToSlash(inPath));
    }
    formValue("datasource", &ds);
    // object type conversion indicator
    const sHiveId cnvObjID(formValue("convertObj"));
    formValue("datatype", &formatHint);
    if( cnvObjID ) {
        formValue("convertTypeName", &convert2Type);
        logOut(eQPLogType_Info, "conversion for object %s to '%s' requested", cnvObjID.print(), convert2Type.ptr());
        if( !convert2Type ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Missing new type name");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
    }
    sDic<sHiveId> exmap;
    formValue("existing_map", &sExmap);
    if( sExmap ) {
        exmap.serialIn(sExmap.ptr(), sExmap.length());
#if _DEBUG
        for(idx i = 0; i < exmap.dim(); ++i) {
            logOut(eQPLogType_Info, "Existing object: '%s' -> '%s'", (char *)exmap.id(i), exmap.ptr(i)->print());
        }
#endif
    }
    sVec<TObjsProp> objects; // all object id and there additional properties
    if( isFinal(-1, objects, properties, inPath) ) {
        return 0;
    }
    reqProgress(-1, 1, 100);
    // unpack archives and retrieve the list of files to work with
    sStr dmlog, dmmsg;
    dmLib DM(dissect);
    bool ok = DM.unpack(inPath, inName, &dmlog, &dmmsg, reqProgressFSStatic, this, svc.lazyReportSec / 3);
    logOut(eQPLogType_Info, "%s", dmlog.ptr());
    if( cnvObjID && DM.dim() > 1 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Only single object is accepted for conversion");
        ok = false;
    } else if( dmmsg ) {
        reqSetInfo(req, eQPInfoLevel_Error, "%s", dmmsg.ptr());
    }
    if( !ok ) {
        if( reqGetStatus(req) != eQPReqStatus_Killed ) {
            reqSetStatus(req, eQPReqStatus_ProgError);
        }
        return 0;
    }
    // scan files one by one
    idx good_files = 0;
    _folder.reset();
    for(const dmLib::File * curFile = DM.first(); !DM.end(curFile); curFile = DM.next(curFile)) {
        bool success = false, isHiveseq = false;
        do {
            const char * objType00 = 0, * defaultObjType = 0;
            idx objTypeID = 0;
            const dmArchiver::TKnownTypes * knownTypes = dmArchiver::getKnownTypes();
            while(knownTypes[++objTypeID].ext00);
            defaultObjType = knownTypes[objTypeID].objType00; //last in list is THE default
            if( !dissect && !cnvObjID ) {
                objType00 = defaultObjType;
            } else if( cnvObjID ) {
                // conversion
                for(idx objTypeID = 0; knownTypes[objTypeID].ext00; ++objTypeID) {
                    if( sString::compareChoice(convert2Type, knownTypes[objTypeID].objType00, 0, true, 0, true) != sNotIdx ) {
                        objType00 = knownTypes[objTypeID].objType00;
                        break;
                    }
                }
                if( !objType00 ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Conversion of object %s failed", cnvObjID.print());
                    logOut(eQPLogType_Error, "Conversion for object %s to unrecognized type '%s' failed", cnvObjID.print(), convert2Type.ptr());
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    break;
                }
            } else {
                // Determine the type of the file by its extension
                const sFilePath ext(curFile->name(), "%%ext");
                if( ext && strcmp(ext, "md5") == 0 ) {
                    // TODO add handling of md5 files as a list of checksums for uploads being processed
                    continue;
                }
                isHiveseq = ext && strcmp(ext, "hiveseq") == 0;
                // loop must always happen, at least to set objTypeID to last one (default)
                for(objTypeID = 0; knownTypes[objTypeID].ext00; ++objTypeID) {
                    if( ext && sString::compareChoice(ext, knownTypes[objTypeID].ext00, 0, true, 0, true) != sNotIdx ) {
                        objType00 = knownTypes[objTypeID].objType00;
                        break;
                    }
                }
                if( !objType00 ) {
                    objType00 = defaultObjType;
                    logOut(eQPLogType_Debug, "input file '%s' set to '%s'", curFile->path(), objType00);
                }
            }
            // special files not of u-file derived type
            if( dissect ) {
                if( strcmp(objType00, "prop-auto-detect") == 0 ) {
                    sDic<sHiveId>  new_ids_map;
                    success = loadProp(req, 0, curFile->location(), curFile->path(), &new_ids_map);
                    if( success ) {
                        reqSetInfo(req, eQPInfoLevel_Info, "Loaded %" DEC " object(s) from %s", new_ids_map.dim(), curFile->path());
                    }
                    objType00 = 0;
                    break;
                    objType00 = defaultObjType;
                } else if( strcmp(objType00, "hivepack") == 0 ) {
                    sStr hl("%s-zip", curFile->location());
                    if( !sFile::symlink(curFile->location(), hl) ) {
                        reqSetInfo(req, eQPInfoLevel_Error, "Failed to open package");
                        logOut(eQPLogType_Error, "symlink failed '%s' -> '%s'", curFile->location(), hl.ptr());
                    } else {
                        dmlog.cut(0);
                        dmmsg.cut(0);
                        dmLib hp(1, 2);
                        bool ok = hp.unpack(hl, 0, &dmlog, &dmmsg, reqProgressFSStatic, this, svc.lazyReportSec / 3);
                        logOut(eQPLogType_Error, "%s", dmlog.ptr());
                        if( dmmsg ) {
                            reqSetInfo(req, eQPInfoLevel_Error, "%s", dmmsg.ptr());
                        }
                        sFile::remove(hl);
                        idx cnt = 0;
                        if( ok ) {
                            sStr maps;
                            sDic<sHiveId> new_ids_map;
                            success = hp.dim();
                            for(const dmLib::File * pf = hp.first(); !hp.end(pf); pf = hp.next(pf)) {
                                const char * nm = pf->name();
                                if( sLen(nm) > 5 && strcmp(&nm[sLen(nm) - 5], ".prop") == 0 ) {
                                    const bool res = loadProp(req, &maps, pf->location(), pf->name(), &new_ids_map);
                                    success &= res;
                                }
                            }
                            if( success && new_ids_map.dim() ) {
                                cnt = new_ids_map.dim();
                                reqSetInfo(req, eQPInfoLevel_Info, "%s", maps.ptr());
                            } else {
                                // cleanup all imported objects
                                for(idx o = 0; o < new_ids_map.dim(); ++o) {
                                    sUsrObj * obj = user->objFactory(*new_ids_map.ptr(o));
                                    if( obj ) {
                                        obj->purge();
                                    }
                                }
                            }
                        }
                        if( success ) {
                            if( cnt == 0 ) {
                                reqSetInfo(req, eQPInfoLevel_Error, "Package is empty");
                            } else {
                                reqSetInfo(req, eQPInfoLevel_Info, "Loaded %" DEC " objects from package", cnt);
                            }
                        }
                    }
                    objType00 = 0;
                    break;
                }
            }
            idx prev_reqid = 0;
            for(const char * objType = objType00, * objProp = knownTypes[objTypeID].props00; objType; objType = sString::next00(objType), objProp = sString::next00(objProp)) {
                // create/get appropriate object
                const sUsrType2 * typ = sUsrType2::ensure(*user, objType);
                if( !typ ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Object type %s is not found", objType);
                    continue;
                }
                const bool notAFile = !typ->isDescendentOf("file");
                sRC rc;
                std::auto_ptr<sUsrObj> obj;
                const sHiveId * exId = cnvObjID ? &cnvObjID : exmap.get(curFile->name());
                // TODO create temporary object location until successfully processed or
                // better yet run all this under special "file-owner" account and
                // share processes with user but reassign file to user only upon success
                if( !exId ) {
                    sHiveId nid;
                    rc = user->objCreate(nid, objType);
                    if( !rc.isSet() ) {
                        obj.reset(user->objFactory(nid));
                        if( obj.get() ) {
                            reqSetInfo(req, eQPInfoLevel_Info, "created object %s", obj->Id().print());
                        }
                    }
                } else {
                    obj.reset(user->objFactory(*exId));
                    if( !obj.get() ) {
                        rc.set(sRC::eAccessing, sRC::eObject, sRC::eFactory, sRC::eFailed);
                    }
                }
                if( rc.isSet() ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Cannot find/create object %s: %s", exId->print(), rc.print());
                    continue;
                }
                logOut(eQPLogType_Info, "Using object %s", obj->Id().print());
                sUsrFile* fobj = dynamic_cast<sUsrFile*>(obj.get());
                if( !notAFile && !fobj ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Cannot cast object type, not 'u-file'-based type, check inheritance of types for '%s'", objType);
                    continue;
                }
                // !!below only sourceFile variable must be used to obtain original file location!!
                sStr sourceFile("%s", curFile->location());
                if( !exId ) {
                    // new object, not cast, it is critical to have certain props set
                    toFolder(req, curFile->dir(), *obj.get());
                    sStr src("archiver/");
                    if( objs.dim() ) {
                        objs[0].Id().print(src, true);
                    } else {
                        src.printf("%" DEC, grpId);
                    }
                    // this property is not that important to fail the object
                    obj->propSet("base_tag", src.ptr());
                    obj->propSet("name", curFile->name());
                    if( !notAFile ) {
                        fobj->propSet("orig_name", curFile->name());
                        const char * ext = strrchr(curFile->name(), '.');
                        bool prop_ok = fobj->propSet("ext", ext ? &ext[1] : 0);
                        prop_ok &= fobj->propSetI("size", curFile->size());
                        src.printf(0, "file://%s", inName.ptr());
                        fobj->propSet("source", ds ? ds : src);
                        if( !prop_ok ) {
                            // delete the object since it will be invalid
                            fobj->actDelete();
                            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                            logOut(eQPLogType_Error, "Cannot assign major properties, object deleted");
                            continue;
                        }
                        dmCompressor cmp(*this, sourceFile, fobj->Id(), knownTypes[objTypeID].compressor);
                        idx rcmp = cmp.launch(*user, grpId);
                        if( !rcmp ) {
                            // fall back to simple copy
                            logOut(eQPLogType_Error, "Failed to launch Compressor, trying a simple copying: '%s' to object %s", sourceFile.ptr(), fobj->Id().print());
                            if( !fobj->setFile(sourceFile, curFile->name(), strrchr(curFile->name(), '.'), curFile->size()) ) {
                                reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                                logOut(eQPLogType_Error, "FATAL: File assignment failed '%s' to object %s - object deleted", sourceFile.ptr(), fobj->Id().print());
                                fobj->actDelete();
                                continue;
                            }
                            // above call to setFile MOVES the file so we need to adjust our paths
                            sourceFile.cut(0);
                            fobj->getFile(sourceFile);
                        } else {
                            logOut(eQPLogType_Info, "%s request submission %" DEC " %s", cmp.getSvcName(), rcmp, rcmp ? "submitted" : "failed");
                        }
                    }
                }
                // remember objid and its extra props
                TObjsProp * p = objects.add();
                if( p ) {
                    p->id = obj->Id();
                    p->pos = p->props.printf("%s", objProp) - p->props.ptr();
                    p->props.add0();
                } else {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "FATAL: out of memory adding to '%s' objects vector", fobj->Id().print());
                    fobj->actDelete();
                    continue;
                }
                // set properties once, they might be needed by other service launched below
                addProps(*obj.get(), properties, objProp);
                // next special ops for certain files, not critical if fail
                if( dissect ) {
                    bool svc_set = true;
                    std::auto_ptr<sQPSvc> svc;
                    if( strcasecmp(objType, "nuc-read") == 0 || strcasecmp(objType, "genome") == 0 ) {
                        if( run_index ) {
                            svc.reset(new DnaParser(*this, sourceFile, obj->Id(), objType, isHiveseq, ((udx) 2) * 1024 * 1024 * 1024, formatHint, curFile->path()));
                            svc_set = !svc.get();
                        }
                    } else if( strcasecmp(objType, "svc-align-dnaseq") == 0 || strcasecmp(objType, "svc-align-multiple") == 0 ) {
                        if( run_index && prev_reqid ) {
                            svc.reset(new QPSvcDnaAlignParser(*this, sourceFile, prev_reqid, obj->Id(), formValue("upload_subject"), objects.last(-2)->id, ((udx) 2) * 1024 * 1024 * 1024, curFile->path()));
                            svc_set = !svc.get();
                        }
                    } else if( strcasecmp(objType, "u-ionAnnot") == 0 ) {
                        svc.reset(new dmAnnot(*this, sourceFile, obj->Id(),objects.last(-2)->id ));
                        svc_set = !svc.get();
                    } else if( strcasecmp(objType, "u-ionExpress") == 0 ) {
                        sStr ionPath, experiment, ionName, hasData, description;
                        formValue("ionFile", &ionName, "ion");
                        obj->addFilePathname(ionPath, true, "ion");
                        formValue("experiment", &experiment, "experiment");
                        formValue("hasdata", &hasData);
                        formValue("description", &description);
                        sIonExpression ionExp(ionPath, sMex::fMapRemoveFile);
                        idx isOK = ionExp.parseConventionalExpression(sourceFile, experiment);
                        if( isOK ) {
                            sUsrObj * tmpObj = obj->cast("u-ionExpress");
                            if( !tmpObj ) {
                                reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                                logOut(eQPLogType_Error, "Cannot cast %s '%s' to '%s'", obj->Id().print(), obj->getTypeName(), objType);
                                continue;
                            }
                            obj.reset(tmpObj);
                            fobj = 0;
                            if( hasData.length() ) {
                                tmpObj->propSet("hasdata", hasData.ptr());
                            }
                            if( description.length() ) {
                                tmpObj->propSet("annot_description", description.ptr());
                            }

                        } else {
                            ionExp.deleteIonContainers(ionPath);
                            reqSetInfo(req, eQPInfoLevel_Error, "%s", ionExp.errorMsg.ptr());
                            logOut(eQPLogType_Error, "Cannot cast %s '%s' to '%s'", obj->Id().print(), obj->getTypeName(), objType);
                        }
                    } else if( !obj->isTypeOf(objType) ) {
                        sUsrObj * tmp = obj->cast(objType);
                        if( !tmp ) {
                            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                            logOut(eQPLogType_Error, "Cannot cast %s '%s' to '%s'", obj->Id().print(), obj->getTypeName(), objType);
                            continue;
                        }
                        obj.reset(tmp);
                        fobj = 0;
                        addProps(*obj.get(), properties, objProp);
                    }
                    // fobj pointer is invalid after this point
                    if( svc.get() ) {
                        prev_reqid = svc->launch(*user, grpId);
                        logOut(eQPLogType_Info, "%s request %s: %" DEC, svc->getSvcName(), prev_reqid ? "submitted" : "failed", prev_reqid);
                    } else if( !svc_set ) {
                        reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Info, "Failed to allocate service launcher for '%s'", objType);
                        continue;
#ifdef HAS_IMAGEMAGICK
                    } else if( strcasecmp(objType, "image") == 0 ) {
                        // extend image attributes, create icon and thumbnail
                        sImage img(sourceFile);
                        if( img.ok() &&
                            subImage(*obj, img, "icon", "png", 64) &&
                            subImage(*obj, img, "thumb", "png", 800) ) {
                            obj->propSetU("height", img.height());
                            obj->propSetU("width", img.width());
                            obj->propSetR("x_res", img.xResolution());
                            obj->propSetR("y_res", img.yResolution());
                            if( img.taken() ) {
                                obj->propSetDTM("taken", img.taken());
                            }
                        } else {
                            obj->cast("u-file");
                            reqSetInfo(req, eQPInfoLevel_Warning, "Unsupported image format '%s'", sFilePath::nextToSlash(sourceFile));
                            logOut(eQPLogType_Warning, "Image properties are not assigned '%s'", sourceFile.ptr());
                        }
#endif
                    } else if( strcasecmp(objType, "excel-file") == 0 ) {
                        sStr path, error_buf;
                        obj->addFilePathname(path, true, "~tmp");
                        path.cut0cut( -4);
                        if( Xls::excel2csv(sourceFile, path, 0, &error_buf) < 1 ) {
                            reqSetInfo(req, eQPInfoLevel_Error, "Failed to parse file '%s'%s%s", sFilePath::nextToSlash(sourceFile), error_buf.ptr() ? ": " : "", error_buf.ptr() ? error_buf.ptr() : "");
                            obj->cast("u-file");
                        }
                    }
                }
                success |= true;
            }
        } while( false );
        good_files += success ? 1: 0;
        if( !reqProgress(-1, good_files, DM.dim()) ) {
            reqSetInfo(req, eQPInfoLevel_Info, "Interrupted by user");
            return 0;
        }
    }
    logOut(eQPLogType_Info, "Successfully processed %" UDEC " of %" UDEC " files", good_files, DM.dim());
    _folder.reset();
    const idx prgs = good_files * (100. / DM.dim());
    reqProgress(-1, sMin(prgs, (idx)100) - 10, 100); // (100 - 10)%
    isFinal(prgs, objects, properties, inPath);
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future
    dmArchiverProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dmArchiver", argv[0]));
    return (int) backend.run(argc, argv);
}
