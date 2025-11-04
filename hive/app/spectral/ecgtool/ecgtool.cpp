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
#include <slib/std/string.hpp>
#include <slib/utils.hpp>
#include <xlib/dmlib.hpp>
#include <violin/violin.hpp>
#include <errno.h>

class ecgTools : public sQPrideProc
{
    public:
        ecgTools(const char * defline00,const char * srv) : sQPrideProc(defline00,srv)
        {
        }
        virtual idx OnExecute(idx);

        virtual sRC OnSplit(idx,idx &);

        const char * getLine(sStr & buf, const char * fileContent)
        {
            if( !fileContent[0] ) {
                return fileContent;
            }
            int i;
            buf.cut(0);
            for(i = 0; fileContent[i] && fileContent[i] != '\n' && fileContent[i] != '\r'; ++i) {
            }
            buf.add(fileContent, (fileContent[i] == '\r') ? (i + 1) : i);
            buf.add0();
            if( fileContent[i] == '\r' && fileContent[i + 1] == '\n' ) {
                ++i;
            }
            return fileContent + i + 1;
        }

        bool Copy_No_Header(const char * filesrc, const char * filedst, bool doAppend, bool follow_link)
        {
            if( sFile::sameInode(filesrc, filedst, follow_link) )
                return false;

            struct stat st;

            if( !follow_link && sFile::isSymLink(filesrc) ) {
                sStr buf;
                const char * target = sFile::followSymLink(filesrc, buf, &st, 1);
                if( !target )
                    return false;

                if( sFile::exists(filedst, false) && !sFile::remove(filedst) )
                    return false;

                return sFile::symlink(target, filedst);
            }

            bool ok = false;
            idx fin = open(filesrc, O_RDONLY, S_IREAD);
            if( fin >= 0 ) {
                idx fout = open(filedst, O_WRONLY | O_CREAT | (doAppend ? O_APPEND : 0), S_IREAD | S_IWRITE);
                if( fout >= 0 ) {
                    static const idx size = 4 * 1024 * 1024;
                    char headbuf[2];
                    char buf[size];
                    idx len, i, j;
                    idx written = len = i = j = 0;
                    errno = 0;

                    while( (j = read(fin, headbuf, 1)) != 0 && errno == 0 ) {
                        if(headbuf[0] == 10 || headbuf[0] == 13){
                            break;
                        }else{
                            ++i;
                        }
                    }

                    while( (len = read(fin, buf, size)) != 0 && errno == 0 ) {
                        while( len - written > 0 && errno == 0 ) {
                            written += write(fout, buf, len - written);
                        }
                        written = 0;
                    }
                    ok = errno == 0;
                    if( fstat(fin, &st) == 0 ) {
                        sFile::fsetAttributes(fout, &st);
                    } else {
                        ok = false;
                    }
                    ok |= close(fout) == 0;
                }
                close(fin);
            }
            return ok;
        }

        void ConcatenateFiles(const char* outputfile, const char* inputBaseFile, const char* extension, const char* workingDirectory, idx numberOfFiles)
        {
            sStr OutFile("%s%s", workingDirectory, outputfile);
            sStr infile, in;
            for(int iter = 0; iter < numberOfFiles; iter++) {
                in.cut(0);
                infile.cut(0);
                in.printf("%s_%d.%s", inputBaseFile, iter, extension);
                infile.printf("%s%s", workingDirectory, in.ptr(0));

                if(iter != 0){
                    if(!Copy_No_Header(infile, OutFile, true, true)){
                    #if _DEBUG
                        ::printf("\nERROR: \tCannot open concatenate without header %s in destination.\n", in.ptr(0));
                    #endif

                    }
                }else{
                    if(!sFile::copy(infile, OutFile, false, true)){
                    #if _DEBUG
                        ::printf("\nERROR: \tCannot open concatenate %s in destination.\n", in.ptr(0));
                    #endif
                    }
                }
            }
        }
};

sRC ecgTools::OnSplit(idx req,idx &cntParallel)
{
    if (!objs.dim()){
        reqSetInfo(req, eQPInfoLevel_Error, "Failure: Object not initialized during splitting\n");
        return sRC(sRC::eSplitting, sRC::eRequest, sRC::eObject, sRC::eUninitialized);
    }
    cntParallel=1;
    sStr sSplitField,sSplitSlice;
    const char * splitField = getSplitField(0, 0, &sSplitField);
    const char * splitSize = getSplitSize(0, 0, &sSplitSlice);

    sVec <sHiveId> files;
    if (objs.dim()){
        objs[0].propGetHiveIds(splitField, files);
    } else {

    }
    sStr fileSlices;
    idx totCnt = 0;
    idx fileCnt = 0;
    idx slice = atoidx(splitSize);
    idx file_slice_remainder = slice;
    fileSlices.printf(0,"file index,start row,cnt\n0,0,%" DEC "\n",slice);
    for (idx ifile = 0; ifile<files.dim(); ++ifile){
        sHiveId id(files.ptr(ifile)->print());
        fileCnt = 0;
        if( id.valid() ) {
            std::auto_ptr<sUsrObj> obj(user->objFactory(id));
            sUsrFile * file = dynamic_cast<sUsrFile *>(obj.get());
            if( file ) {
                sStr buf;
                sFil f(file->getFile(buf), sMex::fReadonly);
                if( f.ok() ) {
                    fileCnt = dmLib::arcDim(buf.ptr(0));
                }
            }
        }
        totCnt += fileCnt;
        idx locCount = fileCnt + (slice - file_slice_remainder);
        while (locCount > slice){
            fileSlices.printf("%" DEC ",%" DEC ",%" DEC "\n", ifile, file_slice_remainder,slice);
            locCount -= slice;
            file_slice_remainder += slice;
            fileCnt -= slice;
        }
        file_slice_remainder = (file_slice_remainder - fileCnt)% slice;
    }
    cntParallel *= totCnt > 0 ? ((totCnt - 1) / slice + 1) : 0;

    if( cntParallel > 1 && !reqSetData(req,"file://file_slices_table", &fileSlices)){
        reqSetInfo(req, eQPInfoLevel_Error, "Failure: reqSetData failed during splitting\n");
        return sRC(sRC::eSplitting, sRC::eRequest, sRC::eBlob, sRC::eFailed);
    }
    return sRC::zero;
}

idx ecgTools::OnExecute(idx req)
{
    sHiveId objID;
    objID = objs[0].Id();
    sUsrObj obj(*user, objID);

    if( !obj.Id() ) {
        logOut(eQPLogType_Info, "Object %s not found or access denied", objID.print());
        reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", objID.print());
        return 1;
    } else {
        logOut(eQPLogType_Info, "processing object %s\n", objID.print());
    }

    sVec<sHiveId> objids;
    obj.propGetHiveIds("arcdim", objids);


    idx fileIndex = 0;
    idx startRow = 0;
    idx SliceSize = 0;
    if (reqSliceCnt > 1){
        sStr partfile;
        if( reqGetData(masterId, "file_slices_table", &partfile) ) {
            sTxtTbl partTable;
            partTable.setBuf(&partfile);
            partTable.parse();
            idx numRows = partTable.rows();
            if (numRows == reqSliceCnt){
                fileIndex = partTable.ival(reqSliceId, 0, 0);
                startRow = partTable.ival(reqSliceId, 1, 0);
                SliceSize = partTable.ival(reqSliceId, 2, 0);
            }
            else {
                reqSetInfo(req, eQPInfoLevel_Warning, "reqGetData file_slice_table is invalid (numRows != sliceCount)");
            }
        }
        else {
            reqSetInfo(req, eQPInfoLevel_Error, "reqGetData to access file_slices_table was not found or access denied");
            return 1;
        }
    }

    sStr filepath;
    sStr destpath;
    sStr outfile;
    sStr zipfileName;
    sStr ecgsRaw("ecgs/Raw");

    obj.addFilePathname(outfile, true, ecgsRaw.ptr(0));
    sDir::makeDir(outfile.ptr(0));

    for(idx i = 0; i < objids.dim(); ++i){
        sUsrFile sf(objids[i], user);
        if( sf.Id() ) {
            filepath.cut(0);
            zipfileName.cut(0);
            zipfileName.printf(sf.propGet("name",0));
            sf.getFile(filepath);
            destpath.cut(0);
            destpath.printf("%s/%s", outfile.ptr(0), zipfileName.ptr(0));

            if (!sFile::exists(destpath.ptr(0))){
                sFile::symlink(filepath.ptr(0), destpath.ptr());
                logOut(eQPLogType_Info,"Creating symlink: %s to %s\n",filepath.ptr(0),destpath.ptr());
            }
        }
    }


    sStr string_param1;
    sStr string_param2;
    sStr string_param3;
    idx int_param1;
    string_param1.printf(obj.propGet("ecg_hl7dump_prefix"));
    string_param2.printf(obj.propGet("ecg_hl7dump_annotator"));

    const char*defVal_hl7prefix = "/Study_Name";
    if ((string_param1.length() == sLen(defVal_hl7prefix)) && (strncmp (string_param1.ptr(0), defVal_hl7prefix, string_param1.length()) == 0)){
        sFilePath srcfile (zipfileName.ptr(0), "%%flnmx");
        if (srcfile.length()){
            string_param1.printf(0, "/%s", srcfile.ptr(0));
        }
    }

    int_param1 = obj.propGetI("ecg_hl7dump_loglevel");
    if(int_param1 == 1){
        string_param3.printf("trace");
    }else if(int_param1 == 2){
        string_param3.printf("debug");
    }else if(int_param1 == 3){
        string_param3.printf("info");
    }else if(int_param1 == 5){
        string_param3.printf("error");
    }else{
        string_param3.printf("warning");
    }

    filepath.cut(0);
    destpath.cut(0);
    sStr log;
    sStr BackTrace;
    sStr temp1;
    sStr index;

    index.printf("index_raw_%" DEC ".idx", reqSliceId);
    obj.addFilePathname(filepath, true, index);

    sFilePath workDir (filepath, "%%dir");
    sFilePath workDir2 (filepath, "%%dir/");

    temp1.cut(0);
    temp1.printf("hl7dump_log_%" DEC ".csv", reqSliceId);
    obj.addFilePathname(log, true, temp1);

    temp1.cut(0);
    temp1.printf("hl7dump_log-bt_%" DEC ".csv", reqSliceId);
    obj.addFilePathname(BackTrace, true, temp1);

    temp1.cut(0);
    temp1.printf("info_raw_%" DEC ".out", reqSliceId);
    obj.addFilePathname(destpath, true, temp1);

    sStr cmdLine("ecgtools-run.osLinux hl7dump --input-dir %s --output-index %s --output-info %s --output-prefix %s --working-dir %s --logfile %s --slicesize %" DEC " --zip-index 1 --start-zip %" DEC " --start-row %" DEC " --annotator %s --loglevel %s --logfile-backtrace %s", ecgsRaw.ptr(0), filepath.ptr(0), destpath.ptr(0), string_param1.ptr(0), workDir.ptr(0), log.ptr(0), SliceSize, fileIndex, startRow, string_param2.ptr(0), string_param3.ptr(0), BackTrace.ptr(0));
    sIO log1;
    logOut(eQPLogType_Info,"\nCOMMAND: %s\n", cmdLine.ptr(0));
    const idx execret1 = exec(cmdLine, 0, 0, &log1, 1);

#if _DEBUG
    ::printf("OUTPUT: \n%s", log1.ptr());
#endif
    logOut(execret1 ? eQPLogType_Error : eQPLogType_Debug, "Return: %" DEC " \nOutput:\n%s", execret1, log1.ptr());
    if( execret1 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "hl7dump produced an error");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return -1;
    }
    if (!reqProgress(0, 15, 100)){
        reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
        reqSetStatus(req, eQPReqStatus_Killed);
        return -1;
    }


    string_param1.cut(0);
    string_param2.cut(0);
    string_param3.cut(0);
    bool bool_param1 = obj.propGetBool("ecg_preprocess_keepmedian");
    bool bool_param2 = obj.propGetBool("ecg_preprocess_savefull");
    bool bool_param3= obj.propGetBool("ecg_preprocess_savemedian");
    int_param1 = obj.propGetI("ecg_preprocess_precut");

    idx int_param2 = obj.propGetI("ecg_preprocess_loglevel");
    if(int_param2 == 1){
        string_param1.printf("trace");
    }else if(int_param2 == 2){
        string_param1.printf("debug");
    }else if(int_param2 == 3){
        string_param1.printf("info");
    }else if(int_param2 == 5){
        string_param1.printf("error");
    }else{
        string_param1.printf("warning");
    }

    idx tmp1, tmp2, tmp3;
    tmp1 = 0;
    tmp2 = 0;
    tmp3 = 0;
    if(bool_param1){ tmp1 = 1;}
    if(bool_param2){ tmp2 = 1;}
    if(bool_param3){ tmp3 = 1;}

    destpath.cut(0);
    filepath.cut(0);
    log.cut(0);
    BackTrace.cut(0);
    outfile.cut(0);

    obj.getFilePathname(filepath, index);
    index.cut(0);

    temp1.cut(0);
    temp1.printf("index_preprocess_%" DEC ".idx", reqSliceId);
    obj.addFilePathname(index, true, temp1);

    temp1.cut(0);
    temp1.printf("preprocess_%" DEC ".out", reqSliceId);
    obj.addFilePathname(outfile, true, temp1);

    temp1.cut(0);
    temp1.printf("preprocess_log_%" DEC ".csv", reqSliceId);
    obj.addFilePathname(log, true, temp1);

    temp1.cut(0);
    temp1.printf("preprocess_log-bt_%" DEC ".csv", reqSliceId);
    obj.addFilePathname(BackTrace, true, temp1);

    cmdLine.cut(0);
    cmdLine.printf("ecgtools-run.osLinux ecgindex-preprocess --ecgindex-in %s --working-dir %s --ecgindex-out %s --output %s --precut %" DEC " --plugins /home/qpride/bin/ecgplugins/formats/ --logfile %s --loglevel %s --output-dir Preprocessed --keepmedian %" DEC " --savefull %" DEC " --savemedian %" DEC " --logfile-backtrace %s", filepath.ptr(0), workDir2.ptr(0), index.ptr(0), outfile.ptr(0), int_param1, log.ptr(0), string_param1.ptr(0), tmp1, tmp2, tmp3, BackTrace.ptr(0));
    sIO log2;
    logOut(eQPLogType_Info, "\nCOMMAND: %s\n", cmdLine.ptr(0));
    const idx execret2 = exec(cmdLine, 0, 0, &log2, 1);

    #if _DEBUG
        ::printf("OUTPUT: \n%s", log2.ptr());
    #endif
    logOut(execret2 ? eQPLogType_Error : eQPLogType_Debug, "Return: %" DEC " \nOutput:\n%s", execret2, log2.ptr());
    if( execret2 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Preprocess produced an error");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return -1;
    }
    if (!reqProgress(0, 25, 100)){
        reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
        reqSetStatus(req, eQPReqStatus_Killed);
        return -1;
    }


    sStr indexDelineate;

    BackTrace.cut(0);
    log.cut(0);
    outfile.cut(0);
    string_param1.cut(0);
    string_param1.printf(obj.propGet("ecg_delineate_annout"));
    string_param2.printf(obj.propGet("ecg_delineate_delineation"));
    int_param1 = obj.propGetI("ecg_delineate_precut");
    int_param2 = obj.propGetI("ecg_delineate_loglevel");

    if(int_param2 == 1){
        string_param3.printf("trace");
    }else if(int_param2 == 2){
        string_param3.printf("debug");
    }else if(int_param2 == 3){
        string_param3.printf("info");
    }else if(int_param2 == 5){
        string_param3.printf("error");
    }else{
        string_param3.printf("warning");
    }

    temp1.cut(0);
    temp1.printf("delineate_%" DEC ".out", reqSliceId);
    obj.addFilePathname(outfile, true, temp1);

    temp1.cut(0);
    temp1.printf("index_delineate_%" DEC ".idx", reqSliceId);
    obj.addFilePathname(indexDelineate, true, temp1);

    temp1.cut(0);
    temp1.printf("delineate_log_%" DEC ".csv", reqSliceId);
    obj.addFilePathname(log, true, temp1);

    temp1.cut(0);
    temp1.printf("delineate_log-bt_%" DEC ".csv", reqSliceId);
    obj.addFilePathname(BackTrace, true, temp1);

    cmdLine.cut(0);
    cmdLine.printf("ecgtools-run.osLinux ecgindex-delineate --ecgindex-in %s --working-dir %s --ecgindex-out %s --output %s --plugins /home/qpride/bin/ecgplugins/formats --logfile %s --delineation %s --annout %s --loglevel %s --logfile-backtrace %s", index.ptr(0), workDir.ptr(0), indexDelineate.ptr(0), outfile.ptr(0), log.ptr(0), string_param2.ptr(0), string_param1.ptr(0), string_param3.ptr(0), BackTrace.ptr(0));
    sIO log3;
    logOut(eQPLogType_Info, "\nCOMMAND: %s\n", cmdLine.ptr(0));
    const idx execret3 = exec(cmdLine, 0, 0, &log3, 1);

    #if _DEBUG
        ::printf("OUTPUT: \n%s", log3.ptr());
    #endif
    logOut(execret3 ? eQPLogType_Error : eQPLogType_Debug, "Return: %" DEC " \nOutput:\n%s", execret3, log3.ptr());
    if( execret3 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Delineate produced an error");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return -1;
    }
    if (!reqProgress(0, 50, 100)){
        reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
        reqSetStatus(req, eQPReqStatus_Killed);
        return -1;
    }


    string_param1.cut(0);
    string_param2.cut(0);
    string_param3.cut(0);

    sStr buf00, buf01, buf02;
    const char* iter2 = 0;
    const char* iter3 = 0;
    idx num = 0;

    if(formValues00("ecg_analyze_plugin", &buf00) && formValues00("ecg_analyze_loglevel", &buf01) && formValues00("ecg_analyze_enable", &buf02)) {
        for (const char *iter1 = buf00.ptr(); iter1; iter1 = sString::next00(iter1)) {
            if(num == 0){
                iter2 = buf01.ptr();
                iter3 = buf02.ptr();
            }
            string_param1.cut(0);
            string_param2.cut(0);

            idx p1 = atoi(iter1);
            idx p2 = atoi(iter2);
            idx p3 = atoi(iter3);

            if(p3 == 1){
                if(p1 == 1){
                    string_param1.printf("vcg");
                }else{
                    string_param1.printf("repdur");
                }

                outfile.cut(0);
                temp1.cut(0);
                temp1.printf("analyze_%s_%" DEC ".out", string_param1.ptr(0), reqSliceId);
                obj.addFilePathname(outfile, true, temp1);

                log.cut(0);
                temp1.cut(0);
                temp1.printf("analyze_%s_log_%" DEC ".csv", string_param1.ptr(0), reqSliceId);
                obj.addFilePathname(log, true, temp1);

                BackTrace.cut(0);
                temp1.cut(0);
                temp1.printf("analyze_%s_log-bt_%" DEC ".csv", string_param1.ptr(0), reqSliceId);
                obj.addFilePathname(BackTrace, true, temp1);

                if(p2 == 1){
                    string_param2.printf("trace");
                }else if(p2 == 2){
                    string_param2.printf("debug");
                }else if(p2 == 3){
                    string_param2.printf("info");
                }else if(p2 == 5){
                    string_param2.printf("error");
                }else{
                    string_param2.printf("warning");
                }

                cmdLine.cut(0);
                cmdLine.printf("ecgtools-run.osLinux ecgindex-analyze --analyzer-plugin %s --ecgindex-in %s --working-dir %s --output %s --plugins /home/qpride/bin/ecgplugins/formats,/home/qpride/bin/ecgplugins/analyze --logfile %s --loglevel %s --logfile-backtrace %s", string_param1.ptr(0), indexDelineate.ptr(0), workDir.ptr(0), outfile.ptr(0), log.ptr(0), string_param2.ptr(0), BackTrace.ptr(0));
                sIO log4;
                logOut(eQPLogType_Info, "\nCOMMAND: %s\n", cmdLine.ptr(0));
                const idx execret4 = exec(cmdLine, 0, 0, &log4, 1);

                #if _DEBUG
                    ::printf("OUTPUT: \n%s", log4.ptr());
                #endif
                logOut(execret4 ? eQPLogType_Error : eQPLogType_Debug, "Return: %" DEC " \nOutput:\n%s", execret4, log4.ptr());
                if( execret4 ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Analyze produced an error");
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return -1;
                }
            }
            iter2 = sString::next00(iter2, 1);
            iter3 = sString::next00(iter3, 1);
            ++num;
        }
        buf00.cut(0);
        buf01.cut(0);
        buf02.cut(0);
        num = 0;
    }

    if (!reqProgress(0, 75, 100)){
        reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
        reqSetStatus(req, eQPReqStatus_Killed);
        return -1;
    }


    sStr buf03;
    const char* iter4;

    if(formValues00("ecg_getmetrics_annset", &buf00) && formValues00("ecg_getmetrics_filter", &buf01) && formValues00("ecg_getmetrics_loglevel", &buf02) && formValues00("ecg_getmetrics_enable", &buf03)) {
        for (const char *iter1 = buf00.ptr(); iter1; iter1 = sString::next00(iter1)) {
            if(num ==0){
                iter2 = buf01.ptr();
                iter3 = buf02.ptr();
                iter4 = buf03.ptr();
            }

            string_param1.cut(0);
            string_param2.cut(0);
            string_param3.cut(0);

            string_param2.printf(iter2);

            idx p1 = atoi(iter1);
            idx p2 = atoi(iter3);
            idx p3 = atoi(iter4);

            if(p3 == 1){
                if(p1 == 1){
                    string_param1.printf("CRO");
                }else{
                    string_param1.printf("automatic");
                }

                if(p2 == 1){
                    string_param3.printf("trace");
                }else if(p2 == 2){
                    string_param3.printf("debug");
                }else if(p2 == 3){
                    string_param3.printf("info");
                }else if(p2 == 5){
                    string_param3.printf("error");
                }else{
                    string_param3.printf("warning");
                }

                outfile.cut(0);
                temp1.cut(0);
                temp1.printf("metrics_%s_%" DEC ".out", string_param1.ptr(0), reqSliceId);
                obj.addFilePathname(outfile, true, temp1);

                log.cut(0);
                temp1.cut(0);
                temp1.printf("getmetrics_%s_log_%" DEC ".csv", string_param1.ptr(0), reqSliceId);
                obj.addFilePathname(log, true, temp1);

                BackTrace.cut(0);
                temp1.cut(0);
                temp1.printf("getmetrics_%s_log-bt_%" DEC ".csv", string_param1.ptr(0), reqSliceId);
                obj.addFilePathname(BackTrace, true, temp1);

                cmdLine.cut(0);
                cmdLine.printf("ecgtools-run.osLinux ecgindex-getmetrics --ecgindex %s --filter %s --output-file %s --output-format csv --annset %s --working-dir %s --logfile %s --loglevel %s --plugins /home/qpride/bin/ecgplugins/formats --logfile-backtrace %s", indexDelineate.ptr(0), string_param2.ptr(0), outfile.ptr(0), string_param1.ptr(0), workDir2.ptr(0), log.ptr(0), string_param3.ptr(0), BackTrace.ptr(0));
                sIO log6;
                logOut(eQPLogType_Info, "\nCOMMAND: %s\n", cmdLine.ptr(0));
                idx execret6 = exec(cmdLine, 0, 0, &log6, 1);

                #if _DEBUG
                    ::printf("LOG: %s", log6.ptr());
                #endif
                logOut(execret6 ? eQPLogType_Error : eQPLogType_Debug, "Return: %" DEC " \nOutput:\n%s", execret6, log6.ptr());
                if( execret6 ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Get Metrics produced an error");
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return -1;
                }

                ++num;
                iter2 = sString::next00(iter2, 1);
                iter3 = sString::next00(iter3, 1);
                iter4 = sString::next00(iter4, 1);
            }
        }
        buf00.cut(0);
        buf01.cut(0);
        buf02.cut(0);
        buf03.cut(0);
        num = 0;
    }

    if (!reqProgress(0, 90, 100)){
        reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
        reqSetStatus(req, eQPReqStatus_Killed);
        return -1;
    }


    ecgsRaw.cut(0);
    ecgsRaw.addString("ECG Analyze");
    const char * mysvcName = vars.value((const char *)ecgsRaw);
    if( isLastInMasterGroup(mysvcName) )
    {
        sStr out("out");
        sStr ind("idx");
        sStr csv("csv");

        sStr Index1("index_raw.idx");
        sStr Index2("index_raw");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,ind.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("index_delineate.idx");
        Index2.addString("index_delineate");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,ind.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("index_preprocess.idx");
        Index2.addString("index_preprocess");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,ind.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("hl7dump_log.csv");
        Index2.addString("hl7dump_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("hl7dump_log-bt.csv");
        Index2.addString("hl7dump_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("delineate_log.csv");
        Index2.addString("delineate_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("delineate_log-bt.csv");
        Index2.addString("delineate_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("preprocess_log.csv");
        Index2.addString("preprocess_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("preprocess_log-bt.csv");
        Index2.addString("preprocess_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("analyze_vcg_log.csv");
        Index2.addString("analyze_vcg_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("analyze_vcg_log-bt.csv");
        Index2.addString("analyze_vcg_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("analyze_repdur_log.csv");
        Index2.addString("analyze_repdur_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("analyze_repdur_log-bt.csv");
        Index2.addString("analyze_repdur_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("getmetrics_CRO_log.csv");
        Index2.addString("getmetrics_CRO_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("getmetrics_CRO_log-bt.csv");
        Index2.addString("getmetrics_CRO_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("getmetrics_automatic_log.csv");
        Index2.addString("getmetrics_automatic_log");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        Index1.cut(0);
        Index2.cut(0);
        Index1.addString("getmetrics_automatic_log-bt.csv");
        Index2.addString("getmetrics_automatic_log-bt");
        obj.delFilePathname(Index1.ptr(0));
        ConcatenateFiles(Index1.ptr(0), Index2.ptr(0) ,csv.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("info_raw.out");
        outfile.addString("info_raw");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("preprocess.out");
        outfile.addString("preprocess");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("delineate.out");
        outfile.addString("delineate");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("analyze_vcg.out");
        outfile.addString("analyze_vcg");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("analyze_repdur.out");
        outfile.addString("analyze_repdur");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("metrics_automatic.out");
        outfile.addString("metrics_automatic");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        index.cut(0);
        outfile.cut(0);
        index.addString("metrics_CRO.out");
        outfile.addString("metrics_CRO");
        obj.delFilePathname(index.ptr(0));
        ConcatenateFiles(index.ptr(0), outfile.ptr(0), out.ptr(0), workDir2.ptr(0), reqSliceCnt);

        if (!reqProgress(0, 95, 100)){
            reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
            reqSetStatus(req, eQPReqStatus_Killed);
            return -1;
        }


        for(int iter = 0; iter < reqSliceCnt; iter++)
        {
            out.cut(0);
            out.printf("_%d.out", iter);
            ind.cut(0);
            ind.printf("_%d.idx", iter);
            csv.cut(0);
            csv.printf("_%d.csv", iter);

            temp1.cut(0);
            temp1.addString("index_raw");
            temp1.addString(ind.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("index_delineate");
            temp1.addString(ind.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("index_preprocess");
            temp1.addString(ind.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("hl7dump_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("hl7dump_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("preprocess_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("preprocess_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("delineate_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("delineate_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("analyze_vcg_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("analyze_vcg_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("analyze_repdur_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("analyze_repdur_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("getmetrics_automatic_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("getmetrics_automatic_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("getmetrics_CRO_log");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("getmetrics_CRO_log-bt");
            temp1.addString(csv.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("info_raw");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("preprocess");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("delineate");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("analyze_vcg");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("analyze_repdur");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("metrics_automatic");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }

            temp1.cut(0);
            temp1.addString("metrics_CRO");
            temp1.addString(out.ptr(0));
            if(!obj.delFilePathname(temp1.ptr(0)))
            {
#if _DEBUG
                ::printf("\nERROR:\tCannot delete %s in destination after concatenation.\n", temp1.ptr(0));
#endif
            }
        }
    }

    if (!reqProgress(0, 100, 100)){
        reqSetInfo(req, eQPInfoLevel_Error, "user killed the process");
        reqSetStatus(req, eQPReqStatus_Killed);
        return -1;
    }

    reqSetStatus(req, eQPReqStatus_Done);

    return 0;
}


int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc,argv);

    ecgTools backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp,"ecgtool",argv[0]));
    return (int)backend.run(argc,argv);
}



