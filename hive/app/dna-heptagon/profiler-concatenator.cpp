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
#include <slib/std/file.hpp>
#include <slib/std/app.hpp>
#include <slib/utils/sort.hpp>
#include <qlib/QPrideProc.hpp>
#include <qlib/QPrideClient.hpp>
#include <slib/utils/tbl.hpp>

using namespace slib;

const char * helpTxt =
"help: dna-profiler-concatenate [-dir dir] [--qdb]\n\n" \
"It scans in the folder for files named like Noise-<num>.csv, sorts by <num> and concatenates them\n" \
"If input directory is provided it only work in this directory (!!It does NOT change the type of the object)\n" \
"If not then it looks in database to find all objects of type svc-profile and jumps to their folder.\n"\
"Finally it converts them to 'svc-heptagon'.\n";

const char * profiler_qdb_type ="svc-profiler";
const char * heptagon_qdb_type ="svc-profiler-heptagon";
const char * profiler_submitter ="dna-profiler";
const char * heptagon_submitter ="dna-heptagon";

enum enumFileNames{ eNoise,eNoiseIntegral,eFreqProfile,eFreqProfileIntegral,
                        eHistProfile,eSNPprofile,eProfileInfo,eSNPthumb,eThumbInfo};

const char * fileNames00 = "Noise" _ "NoiseIntegral" _ "FreqProfile" _ "FreqProfileIntegral" _
                           "HistProfile" _ "SNPprofile" _ "ProfileInfo" _ "SNPthumb" _ "ThumbInfo" __;


typedef struct {
    sStr path;
    real weight;
    udx min_free;
    udx from, to;
    udx free;
} sRoots;

static const char* skipToNextLine(const char *buf, const char *bufend)
{
    while (buf < bufend && *buf && *buf != '\r' && *buf != '\n')
        buf++;
    while (buf < bufend && (*buf == '\r' || *buf == '\n'))
        buf++;
    return buf;
}

const char * filePathname(sFilePath & subject, bool overwrite,const char * dir, const char* key, ...)
{
    const idx pos = subject.pos();
    if( dir && key ) {
        sStr ext;
        sCallVarg(ext.vprintf, key);

        subject.makeName(dir,"%%path/%s",ext.ptr());

        if( sFile::exists(subject) ) {
            if( overwrite && !sFile::remove(subject) ) {
                subject[pos + 1] = '\0';
                subject.cut(pos);
            }
        }
    }
    if( subject.pos() != pos ) {
        return subject.ptr(pos);
    }
    return 0;
}

idx getSubSortedFileInd(sDir & dir, sVec<idx> & ind)
{
    if( !dir.dimEntries() ) {
        return 0;
    }
    ind.resize(dir.dimEntries());
    udx i = 0 ;
    for(const char * path = dir.ptr(); path ; path = sString::next00(path), ++i) {
        sscanf(path,"%*[^-]-%" DEC,ind.ptr(i));
    }
    sSort::sort(ind.dim(),ind.ptr());
    return ind.dim();
}

const char * indexTblFile(sFil * conc_prof, const char * tblIdxName ) {
    sTxtTbl tbl(tblIdxName);
    tbl.borrowFile(conc_prof);
PERF_START("PROFILE_INDEX");
    const char * res = tbl.parse();
PERF_END();
    return res;
}

const char * indexTblFile( const char * path_conc_prof, const char * tblIdxName ) {
    sFil conc_prof(path_conc_prof);
    return indexTblFile(&conc_prof, tblIdxName);
}

idx concatenatedResults(const char * dir)
{

    idx typeCnt = sString::cnt00(fileNames00),flsCnct=0;
    sFil tmpH;
    sFilePath dstProfilePath;
    sStr extHeader;

    for(idx fi = 0 ; fi < typeCnt ; ++fi ) {
        const char * file_type = sString::next00(fileNames00,fi);
        dstProfilePath.printf(0,"%s-*",file_type);
        sVec<idx> subInd;
        sDir results;bool keepHeader = true;
        results.list(sFlag(sDir::bitFiles),dir,dstProfilePath,0,0);
        udx flcnt = getSubSortedFileInd(results,subInd);
        sIO c_file;
        for( udx ic = 0 ; ic < flcnt ; ++ic) {
            dstProfilePath.cut(0);filePathname(dstProfilePath,false,dir, "%s-%" DEC ".csv",file_type,subInd[ic]);
            if( sFile::exists(dstProfilePath) ) {
                tmpH.destroy();tmpH.init(dstProfilePath.ptr(),sMex::fReadonly);
                const char * buf = tmpH.ptr();const char * n_buf = buf;const char * bufend = tmpH.last();
                if( !keepHeader )
                    buf = skipToNextLine(buf, bufend);
                else{
                    dstProfilePath.cut(0);filePathname(dstProfilePath,false,dir,"%s.csv",file_type);
                    c_file.destroy();c_file.init(dstProfilePath.ptr());
                }
                while (*buf && buf < bufend) {
                    n_buf = skipToNextLine(buf, bufend);
                    if( keepHeader ){
                        c_file.add("Reference,");
                        c_file.add(buf,((n_buf?n_buf:bufend)-buf));
                        keepHeader = false;
                    }
                    else{
                        c_file.printf("%" DEC ",",subInd[ic]);
                        c_file.add(buf,((n_buf?n_buf:bufend)-buf));
                    }
                    if(c_file.length() && c_file.ptr(c_file.length()-1)[0]!='\n' )c_file.addString("\n");
                    buf = n_buf;
                }
                keepHeader = false;
            }
        }
    }

    for(idx fi = 0 ; fi < typeCnt ; ++fi ) {
        dstProfilePath.cut(0);
        filePathname(dstProfilePath,false,dir,"%s.csv",sString::next00(fileNames00,fi));
        tmpH.destroy();tmpH.init(dstProfilePath.ptr(),sMex::fReadonly);
        if( !tmpH.length() ) {
            sFile::remove(dstProfilePath);
        }
        else{
            ++flsCnct;
        }
    }

    if(true) {

    }

    return flsCnct;
}

idx cleanOldFiles(const char * dir) {
    sStr dstProfilePath;
    idx fileCnt = sString::cnt00(fileNames00), flsDltd =0;
    for(idx fi = 0 ; fi < fileCnt ; ++fi ) {
        const char * file_type = sString::next00(fileNames00,fi);
        sFilePath filePath;filePath.printf(0,"%s-*",file_type);

        sDir results;
        idx type_flcnt = results.list(sFlag(sDir::bitFiles),dir,filePath,0,0);

        if(type_flcnt)++flsDltd;
        idx iSub = 0;
        for( idx ic = 0 ; ic < type_flcnt ; ) {
            filePath.cut(0);filePathname(filePath,false,dir, "%s-%" DEC ".csv",file_type,iSub++);
            if( sFile::exists(filePath) ) {
                sFile::remove(filePath);
                ++ic;
            }
        }
    }
    return flsDltd;
}
idx fixProfileObjects(sQPrideClient * QP, const char * cmd, const char * , const char * ,sVar * pForm) {
    idx iCnvrt = 0;
    if( sIs("-qdb", cmd) ) {
        std::unique_ptr<sUsr> s_user(new sUsr("qpride", true));
        sStr rootPath;
        sRC rc = sUsrObj::initStorage(QP->cfgStr(&rootPath, 0, "user.rootStoreManager"), QP->cfgInt(0, "user.storageMinKeepFree", (udx)20 * 1024 * 1024 * 1024));
        if( !rc ) {
            idx cnt = pForm->ivalue("cnt");
            if(!cnt)cnt = sIdxMax;
            sUsrObjRes objIds;
            s_user->objs2(profiler_qdb_type, objIds, 0);
            sStr dir;
            for(sUsrObjRes::IdIter it = objIds.first(); objIds.has(it); objIds.next(it)) {
                std::unique_ptr<sUsrObj> obj(s_user->objFactory(*objIds.id(it)));
                dir.cut(0);obj->propGet("_dir",&dir, true);
                idx reqId = obj->propGetI("reqID");
                sVec<idx> status;
                QP->grpGetStatus(reqId,&status);
                for(idx ir = 0 ; ir < status.dim() ; ++ir) {
                    if( status[ir] < sQPrideBase::eQPReqStatus_Done){
                        break;
                    }
                }
                if(concatenatedResults(dir.ptr())){
                    if( obj->cast(heptagon_qdb_type) ) {
                        cleanOldFiles(dir.ptr());
                        ++iCnvrt;
                        QP->printf("Converted %s\n", objIds.id(it)->print() );
                    }
                }
            }
        }
    }
    return iCnvrt;
}


idx fixSingleProfile(sQPrideClient * QP, const char * cmd, const char * , const char * ,sVar * pForm) {
    idx clnd = 0;
    if( sIs("-dir", cmd) ) {
        const char * dir = pForm->value("directory");
        if(!dir){
            return 0;
        }
        if(concatenatedResults(dir)){
            clnd = cleanOldFiles(dir);
        }
    }
    return clnd;
}


bool testTblIndex(sQPrideClient * QP, const char * cmd, const char * , const char * ,sVar * pForm) {
    const char * file = pForm->value("file");
    sFilePath dstProfileIdx;
    sFil src;
    src.destroy();src.init(file,sMex::fReadonly);

    dstProfileIdx.makeName(file,"%%pathx");
    sStr dst;dst.printf("%s.idx2",dstProfileIdx.ptr(0));
    if(!indexTblFile(&src,dst))
        return false;
    return true;
}


void onHelp (sQPrideClient * QP) {
    QP->printf("%s", helpTxt);
}


sCmdLine::exeCommand concatenatorCmdExes[]={
    {0,0,0,"","List of arguments commands"},

    {0,(sCmdLine::exeFunType)&fixSingleProfile,sCmdLine::argOneByOne,          "-dir"," directory // set path to directory to work with "},
    {0,(sCmdLine::exeFunType)&fixProfileObjects,sCmdLine::argOneByOne,          "-qdb"," cnt // Get <cnt> objects from database, concatenate and convert them "},
    {0,(sCmdLine::exeFunType)&testTblIndex,sCmdLine::argOneByOne,          "-tblIndex"," file // Time indexing file"},


    {0,0,0,      "\t","\n\nHelp commands\n"},
    {0,(sCmdLine::exeFunType)&onHelp,sCmdLine::argNone,     "-help"," // help"},

    {sNotPtr,0}
};

int main(int argc, const char * argv[])
{
    sStr appLog, dbgLog;
    sCmdLine cmd;
    if( argc > 1 ) {
        cmd.init(argc, argv);
    } else {
        cmd.init("-help");
    }

    sQPrideClient qapp("config=qapp.cfg" __);
    for(idx i = 0; concatenatorCmdExes[i].param != sNotPtr; ++i)
        if( concatenatorCmdExes[i].cmdFun )
            concatenatorCmdExes[i].param = (void *) &qapp;
    return cmd.exec(concatenatorCmdExes, 0, &appLog, &dbgLog);
}
