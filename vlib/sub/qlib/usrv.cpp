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
#include <ulib/uobj.hpp>
#include <ulib/ufile.hpp>
#include <ulib/uusage.hpp>
#include <errno.h>
#include <fcntl.h>
#include "QPrideSrv.hpp"

namespace slib {

    class sUSrv: public sQPrideSrv
    {
            typedef sQPrideSrv TParent;
        public:
            sUSrv(const char * defline00, const char * srv)
                : TParent(defline00, srv), m_statisticsTimeLast(0), m_statisticsTimeFrequencySecs(0), m_statisticsTimeUnit(0)
            {
            }

            virtual bool OnCommand(const char * command, const char * value);
            virtual bool OnCommandUsage(const char * command, const char * value);
            virtual bool OnCommandAudit(const char * command, const char * value);

            virtual idx OnMaintain(void);

        protected:
            bool init(void);
            void purge(TPurgeData & data);

        private:

            sTime m_statisticsTimeLast;
            idx m_statisticsTimeFrequencySecs;
            idx m_statisticsTimeUnit;
    };
}
;

using namespace slib;

bool sUSrv::init(void)
{
    TParent::init();
    m_statisticsTimeFrequencySecs = cfgInt(0, "qm.statisticsTimeFrequencySecs", 300);
    m_statisticsTimeUnit = cfgInt(0, "qm.statisticsTimeUnit", 3600);
    return true;
}

bool sUSrv::OnCommand(const char * command, const char * value)
{
    idx ll;
#define jobIsCmd(_cmd) (!strncmp(command, _cmd, (ll = sLen(_cmd))))

    if( !init() ) {
        return false;
    } else if( jobIsCmd("usage")) {
        return OnCommandUsage(command, value);
    } else if( jobIsCmd("audit")) {
        return OnCommandAudit(command, value);
    }
    return TParent::OnCommand(command, value);
#undef jobIsCmd
}

idx sUSrv::OnMaintain(void)
{
    idx res = TParent::OnMaintain();

    if( m_statisticsTimeFrequencySecs ) {
        sTime t;
        idx timeHasPassedSinceLastTime = t.time(&m_statisticsTimeLast);
        if( timeHasPassedSinceLastTime > m_statisticsTimeFrequencySecs ) {
            m_statisticsTimeLast = t;
            OnCommand("audit", 0);
        }
    }
    return res;
}

bool sUSrv::OnCommandUsage(const char * command, const char * value)
{
    return false;
}

bool sUSrv::OnCommandAudit(const char * command, const char * value)
{
    sUsr qpride("queen", true);
    if( !qpride.Id() ) {
        logOut(eQPLogType_Warning, "Cannot sign in\n");
    } else {
        const udx keep = cfgInt(0, "qm.auditKeepHours", 0);
        if( keep ) {
            sStr path;
            cfgStr(&path, 0, "qm.auditDumpFileTmpl");
            const udx now = time(0) - keep * 60 * 60;
            udx q = 1;
            if( path ) {
                path.printf(".%" UDEC, now);
                int dump = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                if( dump >= 0 ) {
                    sVarSet tbl;
                    sStr buf;
                    q = 0;
                    do {
                        tbl.empty();
                        qpride.db().getTable(&tbl, "SELECT historyID, createTm, sid, userID, operation, comment FROM UPHistory WHERE createTm < FROM_UNIXTIME(%" UDEC ") LIMIT %" UDEC ",10000", now, q);
                        if( tbl.rows ) {
                            const char * nm[] = { "id", "time", "session", "user", "operation", "details" };
                            buf.cut(0);
                            tbl.printCSV(buf, q ? 0 : 6, nm);

                            idx written = 0, togo = buf.length();
                            errno = 0;
                            while( togo - written > 0 && errno == 0 ) {
                                written += write(dump, buf.ptr(written), togo - written);
                            }
                            if( errno != 0 ) {
                                logOut(eQPLogType_Error, "Cannot write audit dump file '%s': %s\n", path.ptr(), strerror(errno));
                                q = 0;
                                break;
                            }
                            q += tbl.rows;
                        }
                    } while( tbl.rows );
                    fdatasync(dump);
                    close(dump);
                    if( !q ) {
                        sFile::remove(path);
                    } else {
                        logOut(eQPLogType_Info, "Audit file '%s' %" UDEC " lines\n", path.ptr(), q);
                    }
                } else {
                    logOut(eQPLogType_Error, "Cannot open audit dump file '%s': %s\n", path.ptr(), strerror(errno));
                }
            }
            if( q ) {
                qpride.db().execute("DELETE FROM UPHistory WHERE createTm < FROM_UNIXTIME(%" UDEC ")", now);
            }
        }
    }
    return true;
}

static const char * purgeCheckStatements[]={"SELECT objID FROM UPObj;","SELECT objID FROM UPPerm;","SELECT objID FROM UPObjField;"};
static const char * purgeExecuteStatements[]={"DELETE FROM UPObj where objID = %" DEC ";","DELETE FROM UPPerm where objID = %" DEC ";","DELETE FROM UPObjField where objID = %" DEC ";"};
static const char * purgeCheckDBs[]={"UPObj;","UPObjField;","UPPerm;"};
struct ObjFolderInfo {
        idx cntFiles;
        idx inDBObj[3];
        sVec < idx > dupFolderOfs;
        ObjFolderInfo (){ cntFiles=0;for(idx i=0; i<sDim(inDBObj); ++i)inDBObj[i]=0;}
    }  ;

void sUSrv::purge(TPurgeData & data)
{
    sUsr qpride("queen", true);
    if( !qpride.Id() ) {
        logOut(eQPLogType_Warning, "Cannot sign in\n");
    } else {
        sVarSet res;
        idx limit = cfgInt(0, "qm.purgeObjectLimit", 1000);
        limit = limit ? limit : -1;
        idx expireAfter = cfgInt(0, "qm.ObjectExpireDays", 30);
        expireAfter = expireAfter >= 0 ? expireAfter : 30;
        qpride.db().getTable(&res, "SELECT domainID, objID FROM UPObj WHERE softExpiration IS NOT NULL AND DATEDIFF(NOW(), softExpiration) >= %" UDEC " LIMIT %" UDEC, expireAfter, limit);
        if( res.rows ) {
            sStr deleted("\t");
            sStr not_deleted("\t");
            idx cnt_deleted = 0, cnt_not_deleted = 0;
            qpride.allowExpiredObjects(true);
            for(idx i = 0; i < res.rows; ++i) {
                const udx domain_id = res.uval(i, 0);
                const udx oid = res.uval(i, 1);
                const sHiveId id(domain_id, oid, 0);
                sUsrObj obj(qpride, id);
                if( obj.Id() && obj.purge() ) {
                    if( !domain_id ) {
                        data.objs.set(&id, sizeof(id));
                    }
                    deleted.printf("%s%s", id.print(), ((cnt_deleted + 1) % 5) ? ", " : "\n\t");
                    cnt_deleted++;
                } else {
                    not_deleted.printf("%s%s", id.print(), ((cnt_not_deleted + 1) % 5) ? ", " : "\n\t");
                    cnt_not_deleted++;
                }
            }
            qpride.allowExpiredObjects(false);

            if( cnt_deleted ) {
                deleted.cut0cut(deleted.length() - 2);
            }
            if( cnt_not_deleted ) {
                not_deleted.cut0cut(not_deleted.length() - 2);
            }

            logOut(eQPLogType_Info, "following %" DEC " objects have been purged:\n%s\n", cnt_deleted, deleted.ptr());
            if( cnt_not_deleted ) {
                logOut(eQPLogType_Warning, "following %" DEC " objects could not be purged:\n%s\n", cnt_not_deleted, not_deleted.ptr());
            }
        }
    }

    sStr buf; sStr storageList,mpath;
    cfgStr(&buf, 0, "user.rootStoreManager");
    sString::searchAndReplaceSymbols(&storageList, buf.ptr(), buf.length(),";,",0, 0, true, true, true, true, true);
    storageList.add0();

    sStr objFoldersListBuf;objFoldersListBuf.add0();

    sDic< ObjFolderInfo > objsDirs (0,sMex::fSetZero);
    sDic < idx > folderSubCnt(0,sMex::fSetZero);

    for( char * storageLoc=storageList.ptr(); storageLoc; storageLoc=sString::next00(storageLoc)){
        char * endpath=strchr(storageLoc,':') ; if(!endpath)endpath=strchr(storageLoc,'~');if(endpath)*endpath=0;

        sDir dirList;
        idx maxFind = 0;
        char dummy=0;
        dirList.find(sFlag(sDir::bitSubdirs)|sFlag(sDir::bitFiles)|sFlag(sDir::bitRecursive), storageLoc, 0, 0, maxFind, 0);

        for( const char * path=dirList.ptr(); path && *path ; path=sString::next00(path)){
            const char * pathRelStorage=path+sLen(storageLoc);
            idx firstLev=-1,secondLev=-1,objId=-1;

            idx cntScan=sscanf(pathRelStorage,"%" DEC  "/%" DEC "/%" DEC "%c",&firstLev,&secondLev,&objId,&dummy);
            if(cntScan<2)continue;

            const char * slash=strrchr(pathRelStorage,'/');
            idx * pcnt=folderSubCnt.set(path,slash-path);
            ++(*pcnt);

            if( objId==-1 )
                continue;

            if(cntScan!=3)
                continue;
            ObjFolderInfo * oi=objsDirs.set(&objId,sizeof(objId));
            *(oi->dupFolderOfs.add())=objFoldersListBuf.length();
            oi->cntFiles++;
            objFoldersListBuf.add(path);
            objFoldersListBuf.add0();
        }
    }


    for (idx is=0; is<sDim(purgeCheckStatements); ++is) {
        sVarSet res;
        qpride.db().getTable(&res, purgeCheckStatements[is] );
        for(idx i = 0; i < res.rows; ++i) {
            idx objId = res.ival(i, 0);
            ObjFolderInfo * oi=objsDirs.set(&objId,sizeof(objId));
            oi->inDBObj[is]=1;
        }
    }

    for (idx io=0; io<objsDirs.dim(); ++io) {
        ObjFolderInfo * oi=objsDirs.ptr(io);
        idx objId = *((idx * )objsDirs.id(io));
        oi->dupFolderOfs.dim() ? objFoldersListBuf.ptr(oi->dupFolderOfs[0]) : 0;

        if(oi->dupFolderOfs.dim()==0) {
        }
        if(oi->dupFolderOfs.dim()>1) {
            ::printf("object %" DEC " has %" DEC " folder associated to it",objId,oi->dupFolderOfs.dim());
            for(idx id=0;id<oi->dupFolderOfs.dim();++id) {
                ::printf("\t%s\n",objFoldersListBuf.ptr(oi->dupFolderOfs[id]));
            }
        }

        idx missing=0;
        for(idx idb=0; idb<sDim(oi->inDBObj) ; ++idb) {
            if(oi->inDBObj[idb]==0) {
                ::printf("object %" DEC " is missing a row in %s\n",objId,purgeCheckDBs[idb]);
                ++missing;
            }
        }

        if(missing<3 && missing>0) {
            ::printf("object %" DEC " has only partial informaiton in DB\n",objId);
            if(oi->inDBObj[0]==0 || oi->inDBObj[1]==0 ) {
                for(idx is=0; is<sDim(oi->inDBObj) ; ++is){
                    if(oi->inDBObj[is]==1)
                        ::printf("deleting object %" DEC " information from %s\n",objId,purgeCheckDBs[is]);
                        qpride.db().execute(purgeExecuteStatements[is],objId );
                }
            }
        }
    }

    TParent::purge(data);
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    sUSrv backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "qm", argv[0]));
    return (int) backend.run(argc, argv);
}
