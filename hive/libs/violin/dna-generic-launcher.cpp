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
#include <violin/violin.hpp>

//#include <regex.h>
#include <dmlib/dmlib.hpp>

#define contextGetArg( _v_argnum ) (nargs) >= (_v_argnum)+1 && !args[(_v_argnum)].isNull() ? args[_v_argnum].asString() : 0
#define contextGetArgI( _v_argnum ) (nargs) >= (_v_argnum)+1 && !args[(_v_argnum)].isNull() ? atoidx(args[_v_argnum].asString()) : 0
#define contextGetArgB( _v_argnum ) (nargs) >= (_v_argnum)+1 && !args[(_v_argnum)].isNull() ? (atoidx(args[_v_argnum].asString()) ? true : false ) : 0

bool DnaGenericLauncherProc::cleanObjectIDs(sStr &t, sVariant *args, sVec <idx> &objids) {
    sStr t1;
    sString::searchAndReplaceStrings(&t1, args[0].asString(), 0, "obj" __, " " __, 0, true);
    sString::searchAndReplaceSymbols(&t, t1.ptr(), 0, "[]", 0, 0, true, true, true, true);
    sString::scanRangeSet(t.ptr(), 0, &objids, 0, 0, 0);
    return false;
}

bool DnaGenericLauncherProc::cleanObjectIDs(sStr &t, sVariant *args, sVec <sHiveId> &objids) {
    sStr t1;
    sString::searchAndReplaceStrings(&t1, args[0].asString(), 0, "obj" __, " " __, 0, true);
    sString::searchAndReplaceSymbols(&t, t1.ptr(), 0, "[]", 0, 0, true, true, true, true);
    sHiveId::parseRangeSet(objids, t.ptr());
    return false;
}

bool DnaGenericLauncherProc::dispatcher_callback(sVariant &result, const qlang::BuiltinFunction &funcObj, qlang::Context &ctx, sVariant *topic, sVariant *args, idx nargs, void *param)
{
    if (!args) {
        // Null pointer, right now we're using args in each function so should be fixed
        logOut(eQPLogType_Debug, "*args is null in DnaGenericLauncher; arguments not properly passed");
        return false;

    }
    bool ret = true;

    const char * myFuncName = funcObj.getName();
    const char * workDir= static_cast<sUsrInternalContext&>(ctx).workDir();

    //
    // Determine which function is being called from the shell script
    // to dump certain files (fasta/fastq/bt2/sam, etc.).
    //

    if( (strncmp(myFuncName, "pathfast", 8) == 0) && ((myFuncName[8] == 'q')||(myFuncName[8] == 'a'))) {
        const char *  extension=myFuncName+sLen("path");
        char fastType=*(extension+sLen("fast")); // this will point o q or a

        //
        // Read in the different parameters from the pathfast(a/q) command from the generic launcher shell script
        // $(pathfastq(string var1, string var2, bool var3, bool var4, bool var5, string var6))
        //      var1 = Field in the front end where the name of the file is being sent (with a period before the name)
        //               ex: .reads
        //      var2 = The name that the output file should be (without the extension)
        //             Ex: reads
        //      var3 = Whether or not the output reads should be concatenated (1) or dumped separately (0)
        //      var4 = Whether or not the the original IDs should be used in the dumped file (1) or the HIVE Ids should be used (0)
        //      var5 = Whether or not is should be long mode (1) or short mode (0)
        //              - Short mode only prints a single read if reads are repeated
        //      var6 = What the separator should be (not relevant for this function)
        //      var7 = Use original file names

        // Read in arguments from shell script
        idx o = 1;
        const char * namefile = contextGetArg(o);
        ++o;
        idx concatenate = contextGetArgI(o);
        ++o;
        bool keepOriginalID = contextGetArgI(o);
        ++o;
        idx modearg = contextGetArgI(o);
        ++o;
        const char * separator = contextGetArg(o);
        ++o;
        if( !separator ) separator = " ";
        const idx useFileNames = contextGetArgI(o);
        ++o;

        // Set biomode
        sBioseq::EBioMode biomode = sBioseq::eBioModeShort;
        if( sBioseq::isBioModeLong( modearg ) ) {
            biomode = sBioseq::eBioModeLong;
        } else if ( !sBioseq::isBioModeShort( modearg ) ) {
            logOut(eQPLogType_Error, "Unknown biomode: %" DEC, modearg);
            return false;
        }

//        idx prvLen = 0;
        sStr path;

        /*
        _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
        _/
        _/ Prepare object list and if needed concatenated list of all objids
        _/
         */

        sStr t;
        sVec<sHiveId> objids;
        bool error = cleanObjectIDs(t, args, objids);
        if (error) {
            // Error here with the clean object ids
            logOut(eQPLogType_Error, "Error cleaning object IDs.");
            return false;
        }

        sStr objIdConcated;
        if(concatenate) {
            for(idx i = 0; i < objids.dim(); ++i) {
                if(i)objIdConcated.printf(";");
                objIdConcated.printf("%" DEC, objids[i].objId());
            }
            objids.cut(1);
        }

        sStr nameForObj;
        for(idx i = 0; i < objids.dim(); ++i) {
            nameForObj.cut(0);
            /*
            _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            _/
            _/ we want to create path fasta with specified name if namefile is any thing it will
            _/ put that as name of file with specified extension
            _/
             */
            nameForObj.printf("%s", workDir); // ^0xAAAAAAAA

            // If requested to use file names, grab object by file name property
            if (useFileNames) {
//                sHiveId readsId(objids[i]);
                sUsrObj sequence(*user, objids[i]);
                if ( !sequence.Id()){
                    logOut(eQPLogType_Info, "Object %s not found or access denied", objids[i].print());
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Object %s not found or access denied", objids[i].print());
                    return 1;
                }
                sequence.propGet("name", &nameForObj);
            } else if( namefile && *namefile ) {
                nameForObj.printf("%s",namefile); // ^0xAAAAAAAA
                if(objids.dim()>1) {
                    nameForObj.printf("%" DEC,i);
                }
                nameForObj.printf(".%s", extension);
            } else {
                nameForObj.printf("o_");
                if(objIdConcated) {
                    sString::searchAndReplaceSymbols(&nameForObj,objIdConcated,0,";","_",0,true,true,false,true);
                    nameForObj.shrink00();
                } else {
                    nameForObj.printf("%" DEC, objids[i].objId()); // ^0xAAAAAAAA
                }
                nameForObj.printf(".%s", extension);
            }

            /*
            _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            _/
            _/ generate fastaq and or concatenate its name o result
            _/
             */

            idx curLen = path.length();
            if ( curLen ) {
                path.printf("%s", separator);
            }
            const char * newpath=path.printf("%s", nameForObj.ptr());
            if ( !filesDumped.find(newpath) ) {
                /*
                _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                _/
                _/ open one or concatenated list of objids now as fastQA
                _/
                 */

                sHiveseq Qry(user);
                if(objIdConcated) {
                    Qry.parse( objIdConcated, biomode);
                }
                else {
                    Qry.parse(t.printf(0,"%" DEC,objids[i].objId()), biomode);
                }
                if( Qry.dim() == 0 ) {
                    continue;
                }


                /*
                _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                _/
                _/ generate fastaq and or concatenate its name o result
                _/
                 */
                sFile::remove(newpath);

                sFil fp( newpath  );
                if( fp.ok() ) {
                    //PERF_START("dump 1000");
                    // change back Qry.dim();
                    Qry.printFastX(&fp, fastType=='q' ? true : false , 0, Qry.dim(), 0, keepOriginalID, false, 0, 0);
                    //PERF_END();

                } else {
                    path.cut(curLen);
                }
            }
            path.length();
            filesDumped[newpath]=1;
        }

        result.setSprintf("%s", path.ptr());
        return ret;
    } else if( (strncmp(myFuncName, "pathsam", 7) == 0) ) {

        const char *  extension=myFuncName+sLen("path"); // get the extension 'bam' from pathbam
        //char fastType=*(extension+sLen("fast")); // this will point o q or a

        idx o=1;
        const char * namefile = contextGetArg(o); // return specified file name
        ++o;
        // Can't be concatinated for BAM
        //idx concatenate = contextGetArgI(o); // return if files should be concatinated or not
        ++o;
        //bool keepOriginalID= contextGetArgI(o);
        ++o;
        //bool isLong = contextGetArgI(o);
        ++o;
        const char * separator = contextGetArg(o);
        ++o;
        if (!separator) {
            separator=" ";
        }
//        idx prvLen = 0;
        sStr path;

        /*
           _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
           _/
           _/ Prepare object list and if needed concatenated list of all objids
           _/
         */

        sStr t;
        sVec<idx> objids;
        bool error = cleanObjectIDs(t, args, objids);
        if (error) {
            // Error here with the clean object ids
            logOut(eQPLogType_Debug, "Error cleaning object IDs.");
            return false;
        }


        sStr nameForObj;
        for(idx i = 0; i < objids.dim(); ++i){

            sHiveal hiveal(user);
            hiveal.parse(t.printf(0,"%" DEC,objids[i]));
            sBioal * bioal=&hiveal;
            sHiveId _tmpID(objids[i], 0);

            sUsrFile aligner(_tmpID, user);
            sStr subject; sStr query;
            aligner.propGet00("subject", &subject, ";");
            aligner.propGet00("query", &query, ";");

            sHiveseq Sub(user, subject.ptr(), hiveal.getSubMode());
            sHiveseq Qry(user, query.ptr(), hiveal.getQryMode());

            sStr errS; errS.cut(0);
            /*
             * Check to make sure there are subject/reference sequences available (not an empty object)
             */
            if(Sub.dim()==0) {
                errS.printf("Reference '%s' sequences are missing or corrupted\n", subject.length() ? subject.ptr() : "unspecified");
                reqSetInfo(reqId, eQPInfoLevel_Error, "%s",errS.ptr());
                reqSetStatus(reqId, eQPReqStatus_ProgError);
                return 0; // error
            }
            /*
             * Check to make sure that query/reads are available (not an empty object)
             */
            if(Qry.dim() == 0) {
                errS.printf("Query/Read '%s' sequences are missing or corrupted\n", query.length() ? query.ptr() : "unspecified");
                reqSetInfo(reqId, eQPInfoLevel_Error, "%s",errS.ptr());
                reqSetStatus(reqId, eQPReqStatus_ProgError);
                return 0; // error
            }

            nameForObj.cut(0);
            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ we want to create path with specified name if namefile is any thing it will
               _/ put that as name of file with specified extension
               _/
             */
            nameForObj.printf("%s", workDir); // ^0xAAAAAAAA
            if ( namefile && *namefile ) {
                nameForObj.printf("%s",namefile); // ^0xAAAAAAAA
                if(objids.dim()>1) {
                    nameForObj.printf("%" DEC,i);
                }
            } else {
                nameForObj.printf("o_");
                nameForObj.printf("%" DEC, objids[i]); // ^0xAAAAAAAA
            }
            nameForObj.printf(".%s", extension); // Now has the full directory and name

            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ generate fastaq and or concatenate its name o result
               _/
             */

            idx curLen = path.length();
            if ( curLen ) {
                path.printf("%s", separator);
            }

            const char * newpath=path.printf("%s", nameForObj.ptr());

            if( !filesDumped.find(newpath) ) {
                /*
                   _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                   _/
                   _/ generate bam file
                   _/
                 */
                sFile::remove(newpath);
                reqProgress(1,10,100);
                logOut(eQPLogType_Info, "\n\nBeginning SAM dumper. \n");
                reqSetInfo(reqId, eQPInfoLevel_Info, "Beginning SAM dumper.");
                sViosam::convertVioaltIntoSam(bioal, -1, &Qry, &Sub, true, newpath, 0, this, reqProgressStatic);
            }
            path.length();
            filesDumped[newpath]=1;
        }

        result.setSprintf("%s", path.ptr());
        return ret;
    } else if( (strncmp(myFuncName, "pathbam", 7) == 0) ) {
        //
        // THIS FUNCTION IS NOT VALIDATED; PROBABLY DOES NOT WORK PROPERLY
        //

        //
        // To get bam files already uploaded in the system (unprocessed)
        //
        const char *  extension=myFuncName+sLen("path"); // get the extension 'bam' from pathbam
        //char fastType=*(extension+sLen("fast")); // this will point o q or a
        idx o=1;
        const char * namefile = contextGetArg(o); // return specified file name
        ++o;
        // Can't be concatinated for BAM
        //idx concatenate = contextGetArgI(o); // return if files should be concatinated or not
        ++o;
        //bool keepOriginalID= contextGetArgI(o);
        ++o;
        //bool isLong = contextGetArgI(o);
        ++o;
        const char * separator = contextGetArg(o);
        ++o;

        if (!separator) {
            separator=" ";
        }

//        idx prvLen = 0;
        sStr path;

        /*
           _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
           _/
           _/ Prepare object list and if needed concatenated list of all objids
           _/
         */
        sStr t;
        sVec<idx> objids;
        bool error = cleanObjectIDs(t, args, objids);
        if (error) {
            // Error here with the clean object ids
            logOut(eQPLogType_Debug, "Error cleaning object IDs.");
            return false;
        }

        sStr nameForObj;
        sStr srcbuf;
        for(idx i = 0; i < objids.dim(); ++i){

            nameForObj.cut(0);
            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ we want to create path with specified name if namefile is any thing it will
               _/ put that as name of file with specified extension
               _/
             */
            nameForObj.printf("%s", workDir); // ^0xAAAAAAAA
            if ( namefile && *namefile ) {
                nameForObj.printf("%s",namefile); // ^0xAAAAAAAA
                if(objids.dim()>1) {
                    nameForObj.printf("%" DEC,i);
                }
            } else {
                nameForObj.printf("o_");
                nameForObj.printf("%" DEC, objids[i]); // ^0xAAAAAAAA
            }
            nameForObj.printf(".%s", extension); // Now has the full directory and name

            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ generate fastaq and or concatenate its name o result
               _/
             */

            idx curLen = path.length();
            if ( curLen ) {
                path.printf("%s", separator);
            }
            const char * newpath=path.printf("%s", nameForObj.ptr());
            if( !filesDumped.find(newpath) ) {
                sFile::remove(newpath);
                sFil fp( newpath  );
                if( fp.ok() ) {
                    sDir files;
                    char * globExpResultList = (char *)"*.bam";
                    files.list(sFlag(sDir::bitFiles)|sFlag(sDir::bitRecursive),launcherDir,globExpResultList,0,0);// |sFlag(sDir::bitOpenable)
                    if (files) {
                        for( const  char * ptr=files; ptr; ptr=sString::next00(ptr)){
                            sFilePath flnm(ptr,"%%flnm");
                            sStr dst;
                            dst.printf("%s/%s", workDir,flnm.ptr());
                            reqAddFile(dst,flnm);
                            sFile::copy(ptr, dst.ptr());
                            logOut(eQPLogType_Info,"moving %s to %s\n",ptr,dst.ptr());
                        }
                    }
                } else {
                    path.cut(curLen);
                }
            }
//            prvLen = path.length();
            filesDumped[newpath]=1;
            //path.printf("%s", filePath.ptr());
        }
        result.setSprintf("%s", path.ptr());
        return ret;
    } else if( (strncmp(myFuncName, "pathbt2", 7) == 0) ) {

        //const char *  extension=myFuncName+sLen("path"); // get the extension 'bt2' from pathbt2
        //char fastType=*(extension+sLen("fast")); // this will point o q or a
        idx o=5;

        const char * separator = contextGetArg(o);
        ++o;
        if (!separator) {
            separator=" ";
        }

//        idx prvLen = 0;
        sStr path;

        /*
           _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
           _/
           _/ Prepare object list and if needed concatenated list of all objids
           _/
         */
        sStr t;
        sVec<sHiveId> objids;
        bool error = cleanObjectIDs(t, args, objids);
        if (error) {
            // Error here with the clean object ids
            logOut(eQPLogType_Debug, "Error cleaning object IDs.");
            return false;
        }

        sStr nameForObj;
        sStr srcbuf;
        sStr fileName;
        for(idx i = 0; i < objids.dim(); ++i){

            nameForObj.cut(0);
            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ we want to create path with specified name if namefile is any thing it will
               _/ put that as name of file with specified extension
               _/
             */
            nameForObj.printf("%s", workDir); // ^0xAAAAAAAA

            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ generate fastaq and or concatenate its name o result
               _/
             */

            idx curLen = path.length();
            if ( curLen ) {
                path.printf("%s", separator);
            }
            const char * newpath=path.printf("%s", nameForObj.ptr());

            //if( !filesDumped.find(newpath) ) {
                //sFile::remove(newpath);
                //sStr objid;
                //objid.printf("%" DEC "",objids[i]);
                sUsrObj ufile(*(user), objids[i]);
                if (ufile.Id()) {
                    //srcbuf.cut0cut();
                    //const char * source = ufile.getFile(srcbuf);
                    //sFile::copy(ptr, dst.ptr());

                }

                sDir files;
//                const char * fileList00 = 0;
                char * globExpResultList = (char *)"*";
                ufile.files(files, sFlag(sDir::bitFiles), globExpResultList, "");
//                fileList00 = files.ptr();

                //files.list(sFlag(sDir::bitFiles)|sFlag(sDir::bitRecursive),launcherDir,globExpResultList,0,0);// |sFlag(sDir::bitOpenable)

                if (files) {
                    for( const  char * ptr=files; ptr; ptr=sString::next00(ptr)){
                        sFilePath flnm(ptr,"%%flnm");
                        sStr dst;
                        dst.printf("%s/%s", workDir,flnm.ptr());
                        fileName.cut(0);
                        fileName.printf("%s", flnm.ptr());
                        sFile::symlink(ptr, dst.ptr());
                        logOut(eQPLogType_Info,"Creating symlink: %s to %s\n",ptr,dst.ptr());
                    }
                }
//            prvLen = path.length();
            filesDumped[newpath]=1;
            //path.printf("%s", filePath.ptr());
        }

        result.setSprintf("%s", fileName.ptr());
        return ret;
    } else if( (strncmp(myFuncName, "pathfile", 8) == 0) ) {
        idx o = 1;
        const char * namefile = contextGetArg(o);
        ++o;
        idx concatenate = contextGetArgI(o);
        ++o;
        bool keepOriginalID = contextGetArgI(o);
        ++o;
        idx modearg = contextGetArgI(o);
        ++o;
        const char * separator = contextGetArg(o);
        ++o;

        if (!separator) separator=" ";

//        idx prvLen = 0;
        sStr path;

        /*
           _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
           _/
           _/ Prepare object list and if needed concatenated list of all objids
           _/
         */
        sStr t;
        sVec<sHiveId> objids;
        bool error = cleanObjectIDs(t, args, objids);
        if (error) {
            // Error here with the clean object ids
            logOut(eQPLogType_Debug, "Error cleaning object IDs.");
            return false;
        }

        sStr nameForObj;
        sStr srcbuf;
        sStr fileName;
        for(idx i = 0; i < objids.dim(); ++i){
            nameForObj.cut(0);
            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ we want to create path with specified name if namefile is any thing it will
               _/ put that as name of file with specified extension
               _/
             */
            nameForObj.printf("%s", workDir); // ^0xAAAAAAAA

            /*
               _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
               _/
               _/ generate fastaq and or concatenate its name o result
               _/
             */
            idx curLen = path.length();
            if ( curLen ) {
                path.printf("%s", separator);
            }
            const char * newpath=path.printf("%s", nameForObj.ptr());

            //if( !filesDumped.find(newpath) ) {
                //sFile::remove(newpath);
                //sStr objid;
                //objid.printf("%" DEC "",objids[i]);
                sUsrObj ufile(*(user), objids[i]);
//                if ( !ufile.Id()){
//                    logOut(eQPLogType_Info, "Object %s not found or access denied", objids[i].print());
//                    reqSetInfo(reqId, eQPInfoLevel_Error, "Object %s not found or access denied", objids[i].print());
//                    return 1;
//                }
                //if (ufile.Id()) {
                    //srcbuf.cut0cut();
                    //const char * source = ufile.getFile(srcbuf);
                    //sFile::copy(ptr, dst.ptr());

                //}

                sDir files;
//                const char * fileList00 = 0;
                char * globExpResultList = (char *)"*";
                ufile.files(files, sFlag(sDir::bitFiles), globExpResultList, "");
//                fileList00 = files.ptr();

                //files.list(sFlag(sDir::bitFiles)|sFlag(sDir::bitRecursive),launcherDir,globExpResultList,0,0);// |sFlag(sDir::bitOpenable)

                if (files) {
                    for( const  char * ptr=files; ptr; ptr=sString::next00(ptr)){
                        sFilePath flnm(ptr,"%%flnm");
                        sStr dst;
                        dst.printf("%s%s", workDir,namefile);
                        fileName.cut(0);
                        fileName.printf("%s", namefile);

                        sFile::copy(ptr, dst.ptr()); // if error, copy over computation folder

                        //sFile::symlink(ptr, dst.ptr());
                        logOut(eQPLogType_Info,"Creating symlink: %s to %s\n",ptr,dst.ptr());
                    }
                }
//            prvLen = path.length();
            filesDumped[newpath]=1;
            //path.printf("%s", filePath.ptr());
        }
        result.setSprintf("%s", fileName.ptr());
        return ret;
    } else {
        return sQPrideGenericLauncher::dispatcher_callback(result, funcObj, ctx, topic, args, nargs, param);
    }
}
