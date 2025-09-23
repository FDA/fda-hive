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

#include <xlib/dmlib.hpp>
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

            void logTextToReq(idx lvl, sStr & txt)
            {
                sString::searchAndReplaceSymbols(txt.ptr(), txt.length(), "\n\r", 0, 0, true, true, true, false);
                txt.add0(3);
                for( const char * l = txt.ptr(); l; l = sString::next00(l) ) {
                    if( l && *l ) {
                        if( lvl < eQPInfoLevel_Min ) {
                            logOut((eQPLogType)lvl, "%s", l);
                        } else {
                            reqSetInfo(reqId, lvl, "%s", l);
                        }
                    }
                }
            }

            bool importHivepack(const char * propfile, const char * import_folder, udx & cnt)
            {
                sStr log, l2;
                logOut(eQPLogType_Info, "Parsing property table '%s' %" DEC " bytes", sFilePath::nextToSlash(propfile), sFile::size(propfile));
                sVec<sHiveId> newids, uids;
                sDic<sHiveId> new_ids_map;
                bool ok = user->propSet(propfile, log, &newids, &uids, &new_ids_map);
                if( !ok ) {
                    if( !log ) {
                        log.printf("Parsing property table '%s' %" DEC " bytes - failed", sFilePath::nextToSlash(propfile), sFile::size(propfile));
                    }
                    for (idx o = 0; o < newids.dim(); ++o) {
                        sUsrObj* obj = user->objFactory(*newids.ptr(o));
                        if (obj) {
                            obj->purge();
                        }
                    }
                } else {
                    const udx qty = uids.dim() + (newids.dim() ? newids.dim() : new_ids_map.dim());
                    l2.cut(0);
                    if( newids.dim() ) {
                        cnt += newids.dim();
                        l2.printf(0, "new ids:");
                        for(idx i = 0; i < newids.dim(); ++i) {
                            log.printf(" ");
                            newids[i].print(l2);
                            sUsrObj * o = user->objFactory(newids[i]);
                            if( o ) {
                                toFolder(reqId, qty > 1 ? import_folder : 0, *o);
                                delete o;
                            }
                        }
                        reqSetInfo(reqId, eQPInfoLevel_Info, "%s", l2.ptr());
                    } else if( new_ids_map.dim() ) {
                        cnt += new_ids_map.dim();
                        l2.printf(0, "new ids:");
                        for(idx i = 0; i < new_ids_map.dim(); ++i) {
                            l2.printf(" %s->", (const char *) new_ids_map.id(i));
                            new_ids_map.ptr(i)->print(l2);
                            sUsrObj * o = user->objFactory(*(new_ids_map.ptr(i)));
                            if( o ) {
                                toFolder(reqId, qty > 1 ? import_folder : 0, *o);
                                delete o;
                            }
                        }
                        reqSetInfo(reqId, eQPInfoLevel_Info, "%s", l2.ptr());
                    }
                    if( uids.dim() ) {
                        cnt += uids.dim();
                        l2.printf(0, " updated ids:");
                        for(idx i = 0; i < uids.dim(); ++i) {
                            l2.printf(" ");
                            uids[i].print(l2);
                            sUsrObj * o = user->objFactory(uids[i]);
                            if( o ) {
                                toFolder(reqId, qty > 1 ? import_folder : 0, *o);
                                delete o;
                            }
                        }
                        reqSetInfo(reqId, eQPInfoLevel_Info, "%s", l2.ptr());
                    }
                }
                l2.cut0cut();
                sString::searchAndReplaceSymbols(&l2, log.ptr(), log.length(), "\n", 0, 0, true, true, false, true);
                for(const char * p = l2.ptr(); p; p = sString::next00(p)) {
                    reqSetInfo(reqId, ok ? eQPInfoLevel_Info : eQPInfoLevel_Error, "File '%s': %s", sFilePath::nextToSlash(propfile), p);
                }
                return ok;
            }

            bool toFolder(const idx req, const char * path, const sUsrObj & obj) {
                if( !_folder.get() ) {
                    sHiveId folder(formValue("folder"));
                    if( !folder && objs.dim() && user ) {
                        sUsrObjRes res;
                        user->objs2("^folder$+", res, 0, "child", objs[0].IdStr(), "", false, 0, 1);
                        if( res.dim()) {
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
            void addProps(sUsrObj & obj, const char * properties1, const char * properties2)
            {
                if( properties1 && properties1[0] && properties2 && properties2[0] ) {
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
            }
            bool isFinal(idx progress, sVec<TObjsProp> & objects, const char * properties, const char* inPath, const bool keepFiles)
            {
                if( progress >= 0 ) {
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
                bool keepSrc = (progress != 100);
                sVec<sQPrideBase::Request> r;
                requestGetForGrp(reqId, &r);
                serviceID();
                for(idx ri = 0; ri < r.dim(); ++ri) {
                    const idx st = r[ri].stat;
                    if( r[ri].svcID != svcID && st <= eQPReqStatus_Running ) {
                        reqReSubmit(reqId, 60);
                        reqProgress(-1, 99, 100);
                        logOut(eQPLogType_Debug, "waiting for %" DEC " to finish ", r[ri].reqID);
                        return true;
                    }
                    keepSrc |= st == eQPReqStatus_Suspended || st > eQPReqStatus_Done;
                }
                objects.empty();
                reqGetData(reqId, "objects", objects.mex());
                if( objects.dim() ) {
                    TObjsProp::props.cut0cut();
                    reqGetData(reqId, "objects_props", TObjsProp::props.mex());
                }
                for(idx i = 0; i < objects.dim(); ++i) {
                    std::unique_ptr<sUsrObj> obj(user->objFactory(objects[i].id));
                    if( obj.get() ) {
                        addProps(*obj.get(), properties, TObjsProp::props.ptr(objects[i].pos));
                    }

                    recordLoadedID(objects[i].id);
                }
                if( !keepSrc && !keepFiles ) {
                    if( sDir::exists(inPath) ) {
                        sDir::removeDir(inPath, true);
                    } else {
                        sFilePath dir(inPath, "%%dir");
                        sFile::remove(inPath);
                        dmLib::clean(dir);
                        sDir::removeDir(dir, false);
                    }
                }
                reqProgress(-1, progress, 100);
                reqSetStatus(reqId, progress == 100 ? eQPReqStatus_Done : eQPReqStatus_ProgError);
                return true;
            }

            void recordLoadedID(sHiveId& pid)
            {
                sStr buf;
                pid.print(buf);
                buf.add0(3);

                sVec<const char *> id_values;
                const char * id_value = buf.ptr();
                id_values.resize(1);
                id_values[0] = id_value;

                if( pid && objs.dim() ) {
                    sVec<sHiveId> res;
                    objs[0].propGetHiveIds("objID", res);
                    sHiveId * id = res.add();
                    if( id ) {
                        *id = pid;
                        objs[0].propSetHiveIds("objID", res);
                    }
                }
            }
            std::unique_ptr<sUsrFolder> _folder;
    };
}

using namespace slib;

idx dmArchiverProc::OnExecute(idx req)
{
    sStr inPath, inName, ds, properties, formatHint, convert2Type, sExmap;

    formValue("properties", &properties);
    formValue("inputFile", &inPath);
    const bool keepSrc = formBoolValue("keep_source", false);
    if( !inPath || inPath[0] == '\0' ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Missing input");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    if( inPath[0] != '/' ) {
        sStr tmp;
        cfgStr(&tmp, 0, "user.download");
        if( tmp ) {
            tmp.printf("%s", inPath.ptr());
            inPath.printf(0, "%s", tmp.ptr());
        }
    }
    const udx dissect = dmArchiver::getDepth(*this);
    formValue("inputName", &inName);
    if( !inName && inPath[inPath.length() - 1] != '/' ) {
        inName.printf("%s", sFilePath::nextToSlash(inPath));
    }
    formValue("datasource", &ds);
    const sHiveId cnvObjID(formValue("convertObj"));
    formValue("datatype", &formatHint);
    if( cnvObjID ) {
        formValue("convertTypeName", &convert2Type);
        logOut(eQPLogType_Info, "conversion for object %s to '%s' requested", cnvObjID.print(), convert2Type.ptr());
        if( !convert2Type || !sUsrType2::ensure(*user, convert2Type.ptr()) ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Invalid new type name '%s'", convert2Type ? convert2Type.ptr() : "<empty>");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
        convert2Type.add0(3);
    }
    sDic<sHiveId> exmap;
    formValue("existing_map", &sExmap);
    if( sExmap ) {
        exmap.serialIn(sExmap.ptr(), sExmap.length());
    }
    sVec<TObjsProp> objects;
    if( isFinal(-1, objects, properties, inPath, keepSrc ) ) {
        return 0;
    }
    reqProgress(-1, 1, 100);
    sStr dmlog, dmmsg;
    dmLib DM(dissect);
    bool ok = DM.unpack(inPath, inName, &dmlog, &dmmsg, reqProgressFSStatic, this, svc.lazyReportSec / 3);
    logTextToReq(eQPLogType_Info, dmlog);
    if( cnvObjID && DM.dim() > 1 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Only single object is accepted for conversion");
        ok = false;
    } else if( dmmsg ) {
        logTextToReq(eQPInfoLevel_Error, dmmsg);
    }
    if( !ok ) {
        if( reqGetStatus(req) != eQPReqStatus_Killed ) {
            reqSetStatus(req, eQPReqStatus_ProgError);
        }
        return 0;
    }
    idx good_files = 0;
    _folder.reset();
    for(const dmLib::File * curFile = DM.first(); !DM.end(curFile); curFile = DM.next(curFile)) {
        bool success = false, isHiveseq = false;
        do {
            const char * objType00 = 0, * defaultObjType = 0;
            idx objTypeID = 0;
            const dmArchiver::TKnownTypes * knownTypes = dmArchiver::getKnownTypes();
            while(knownTypes[++objTypeID].ext00);
            defaultObjType = knownTypes[objTypeID].objType00;
            if( !dissect && !cnvObjID ) {
                objType00 = defaultObjType;
            } else if( cnvObjID ) {
                objType00 = convert2Type.ptr();
            } else {
                const sFilePath ext(curFile->name(), "%%ext");
                if( ext && strcmp(ext, "md5") == 0 ) {
                    continue;
                }
                isHiveseq = ext && strcmp(ext, "hiveseq") == 0;
                for(objTypeID = 0; knownTypes[objTypeID].ext00; ++objTypeID) {
                    if( ext && sString::compareChoice(ext, knownTypes[objTypeID].ext00, 0, true, 0, true) != sNotIdx ) {
                        objType00 = knownTypes[objTypeID].objType00;
                        break;
                    }
                }
                if( !objType00 ) {
                    objType00 = defaultObjType;
                }
            }
            if( dissect ) {
                if( strcmp(objType00, "hivepack") == 0 ) {
                    sStr hl("%s-zip", curFile->location());
                    if( !sFile::symlink(curFile->location(), hl) ) {
                        reqSetInfo(req, eQPInfoLevel_Error, "Failed to open package");
                        logOut(eQPLogType_Error, "symlink failed '%s' -> '%s'", curFile->location(), hl.ptr());
                    } else {
                        dmlog.cut(0);
                        dmmsg.cut(0);
                        dmLib hp(1, 2);
                        bool ok = hp.unpack(hl, 0, &dmlog, &dmmsg, reqProgressFSStatic, this, svc.lazyReportSec / 3);
                        logTextToReq(eQPLogType_Info, dmlog);
                        if( dmmsg ) {
                            logTextToReq(eQPInfoLevel_Error, dmmsg);
                        }
                        sFile::remove(hl);
                        udx cnt = 0;
                        if( ok ) {
                            success = hp.dim();
                            for(const dmLib::File * pf = hp.first(); !hp.end(pf); pf = hp.next(pf)) {
                                const char * nm = pf->name();
                                if( (sLen(nm) > 14 && sIs("hivepack-", nm) && strcmp(&nm[sLen(nm) - 5], ".json") == 0) || (sLen(nm) > 5 && strcmp(&nm[sLen(nm) - 5], ".prop") == 0) ) {
                                    sFilePath folder("%%dir", curFile->path());
                                    const bool res = importHivepack(pf->location(), folder.ptr(), cnt);
                                    success &= res;
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
                const sUsrType2 * typ = sUsrType2::ensure(*user, objType);
                if( !typ ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Object type %s is not found", objType);
                    continue;
                }
                const bool notAFile = !typ->isDescendentOf("file");
                sRC rc;
                std::unique_ptr<sUsrObj> obj;
                const sHiveId * exId = cnvObjID ? &cnvObjID : exmap.get(curFile->name());
                if( !exId ) {
                    sHiveId nid;
                    rc = user->objCreate(nid, objType);
                    if( !rc.isSet() ) {
                        obj.reset(user->objFactory(nid));
                        if( obj.get() ) {
                        }
                    }
                } else {
                    obj.reset(user->objFactory(*exId));
                    if( !obj.get() ) {
                        RCSET(rc, sRC::eAccessing, sRC::eObject, sRC::eFactory, sRC::eFailed);
                    }
                }
                if( rc.isSet() ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Cannot find/create object %s: %s", exId ? exId->print() : "null", rc.print());
                    continue;
                }
                sUsrFile* fobj = dynamic_cast<sUsrFile*>(obj.get());
                if( !notAFile && !fobj ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                    logOut(eQPLogType_Error, "Cannot cast object type, not 'u-file'-based type, check inheritance of types for '%s'", objType);
                    continue;
                }
                sStr sourceFile("%s", curFile->location());
                if( !exId ) {
                    toFolder(req, curFile->dir(), *obj.get());
                    sStr src("archiver/");
                    if( objs.dim() ) {
                        objs[0].Id().print(src, true);
                    } else {
                        src.printf("%" DEC, grpId);
                    }
                    obj->propSet("base_tag", src.ptr());
                    obj->propSet("name", curFile->name());
                    if( !notAFile ) {
                        fobj->propSet("orig_name", curFile->name());
                        const char * ext = strrchr(curFile->name(), '.');
                        udx prop_ok = fobj->propSet("ext", ext ? &ext[1] : 0);
                        prop_ok += fobj->propSetI("size", curFile->size());
                        src.printf(0, "file://%s", inName.ptr());
                        fobj->propSet("source", ds ? ds : src);
                        if( prop_ok != 2 ) {
                            fobj->actDelete();
                            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                            logOut(eQPLogType_Error, "Cannot assign major properties, object deleted");
                            continue;
                        }
                        dmCompressor cmp(*this, sourceFile, fobj->Id(), knownTypes[objTypeID].compressor);
                        idx rcmp = cmp.launch(*user, reqId);
                        if( !rcmp ) {
                            logOut(eQPLogType_Error, "Failed to launch Compressor, trying a simple copying: '%s' to object %s", sourceFile.ptr(), fobj->Id().print());
                            if( !fobj->setFile_donotuse(sourceFile, curFile->name(), 0, curFile->size()) ) {
                                reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                                logOut(eQPLogType_Error, "FATAL: File assignment failed '%s' to object %s - object deleted", sourceFile.ptr(), fobj->Id().print());
                                fobj->actDelete();
                                continue;
                            }
                            sourceFile.cut(0);
                            fobj->getFile(sourceFile);
                        }
                    }
                }
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
                addProps(*obj.get(), properties, objProp);
                if( dissect ) {
                    bool svc_set = true;
                    std::unique_ptr<sQPSvc> svc;
                    if( strcasecmp(objType, "nuc-read") == 0 || strcasecmp(objType, "genome") == 0 ) {
                        svc.reset(new DnaParser(*this, sourceFile, obj->Id(), objType, isHiveseq, formatHint, curFile->path()));
                        svc_set = !svc.get();
                    } else if( strcasecmp(objType, "svc-align-dnaseq") == 0 || strcasecmp(objType, "svc-align-multiple") == 0 ) {
                        if( prev_reqid ) {
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
                    if( svc.get() ) {
                        svc->setForm(pForm, false);
                        svc->setVar("splitOnFrontEnd", "%s", "false");
                        prev_reqid = svc->launch(*user, reqId);
                        logOut(eQPLogType_Info, "%s request %s: %" DEC, svc->getSvcName(), prev_reqid ? "submitted" : "failed", prev_reqid);
                    } else if( !svc_set ) {
                        reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                        logOut(eQPLogType_Info, "Failed to allocate service launcher for '%s'", objType);
                        continue;
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
    reqProgress(-1, sMin(prgs, (idx)100) - 10, 100);
    isFinal(prgs, objects, properties, inPath, keepSrc);
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc, argv);
    dmArchiverProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dmArchiver", argv[0]));
    return (int) backend.run(argc, argv);
}
