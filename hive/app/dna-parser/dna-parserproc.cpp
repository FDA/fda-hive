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
#include <violin/violin.hpp>
#include <qpsvc/dna-qc.hpp>
#include <qpsvc/qpsvc-dna-align-parser.hpp>
#include <qpsvc/archiver.hpp>
#include <qpsvc/screening.hpp>

class DnaParserProc: public sQPrideProc
{
    public:
        DnaParserProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
};

idx DnaParserProc::OnExecute(idx req)
{
    sStr errmsg;
    bool reporterrorasWarning = false;
#define RELFIXTBL "vioseq-relationFixtable.bin"
#define COUNTLONG "vioseq-longcount"

    do {
        const sHiveId objID(formValue("obj"));
        if( !objID ) {
            errmsg.printf("Invalid objID in %" DEC " request", req);
            break;
        }
        sUsrFile obj(objID, user);
        if( !obj.Id() ) {
            errmsg.printf("Invalid obj %s in %" DEC " request", objID.print(), req);
            break;
        }
        logOut(eQPLogType_Info, "processing object %s\n", obj.Id().print());
        sStr sourceSequenceFilePath;
        formValue("sourceSequenceFilePath", &sourceSequenceFilePath, 0);
        if( !sourceSequenceFilePath ) {
            errmsg.printf("Invalid source sequence file path in %" DEC " request", req);
            break;
        }
        idx fsize = sFile::size(sourceSequenceFilePath);
        if( !fsize ) {
            errmsg.printf("Source sequence filesize is empty in %" DEC " request", req);
            break;
        }

        logOut(eQPLogType_Info, "processing file '%s'\n", sourceSequenceFilePath.ptr());
        const bool isHiveseq = formUValue("hiveseq", 0);
        const bool isMerged = ((formUValue("merge", 0) == 0) || isHiveseq ) ? false : true;
        const bool isSingle = formUValue("single", 0);
        const udx filetype = formUValue("parseAsType",0);

        sStr dstType;
        formValue("dstType", &dstType, 0);
        if( !dstType ) {
            dstType.printf("nuc-read");
            logOut(eQPLogType_Info, "defaulting to type '%s'\n", dstType.ptr());
        }

        if (!isHiveseq) {

            sVioseq2 v;

            v.m_callback = reqProgressStatic;
            v.m_callbackParam = this;

            // We must append a number to the name of the file and send it to parse
            sStr newfile;
            if( !obj.addFilePathname(newfile, true, "~tmp.%" DEC ".vioseq2", reqSliceId) ) {
                errmsg.printf("failed to create destination");
                break;
            }
            sStr partfile;
            reqSetData(req, "file://" RELFIXTBL, 0, 0); // create and empty file for this request
            if( !reqDataPath(req, RELFIXTBL, &partfile) ) {
                errmsg.printf("Can't write the " RELFIXTBL);
                break;
            }
            sFile::remove(partfile.ptr(0));
            {{
                sVec<sVioseq2::Infopart> partitionList(partfile.ptr());
                idx flags = 0;
                flags |= sVioseq2::eParsedQualities;
                if( isSingle ) {
                    flags |= sVioseq2::eParseMultiVioDB;
                }
                if (filetype){
                        flags |= filetype;
                }
                logOut(eQPLogType_Info, "sFile::size(\"%s\") is %" DEC "\n", sourceSequenceFilePath.ptr(), fsize);

                idx ires = v.parseSequenceFile(newfile, sourceSequenceFilePath, flags, 0, 0, 0, v.getPrefixLength(reqSliceCnt), reqSliceId, &partitionList);
//                isMultipleAlignment = alignReferenceIncluded || (v.getFlags() & sVioseq2::eTreatAsMA);
                if( ires < 0 ) {
                    for (idx j=0; v.listErrors[0].errN != 0;++j){
                        if (v.listErrors[j].errN == ires ){
                            errmsg.printf ("%s", v.listErrors[j].msg);
                            break;
                        }
                    }
                    if (!errmsg){
                        errmsg.printf("Parser returns Error Code: %" DEC, ires);
                    }
                    break;
                }
                reqSetData(req, COUNTLONG , "%" DEC, ires);
                sStr lngloc;
                if(!reqGetData(req, COUNTLONG , &lngloc) ) {
                    errmsg.printf("Can't write the " COUNTLONG);
                    break;
                }
                const idx prefixLen = v.getPrefixLength(reqSliceCnt);
                sStr pfx("%.*s", (int)prefixLen, "________");
                idx base = reqSliceId;
                for(idx i = 0; i < prefixLen; ++i) {
                    *(pfx.ptr(prefixLen - i - 1)) = sBioseq::mapRevATGC[base % 4];
                    base /= 4;
                }
                // switch to short for report
                v.setmode(sBioseq::eBioModeShort);
#if _DEBUG
                reqSetInfo(req, eQPInfoLevel_Info, "parsed '%s' part %" DEC " prefix '%s' sequence count short: %" DEC " long: %" DEC "\n", sourceSequenceFilePath.ptr(), reqSliceId, pfx.ptr(), v.dim(), ires);
#else
                logOut(eQPLogType_Info, "parsed '%s' part %" DEC " prefix '%s' sequence count short: %" DEC " long: %" DEC "\n", sourceSequenceFilePath.ptr(), reqSliceId, pfx.ptr(), v.dim(), ires);
#endif
            }}
        }
        if (errmsg){
            break;
        }
        const char * mysvcName = vars.value("serviceName");
        if( isLastInMasterGroup(mysvcName) ) {
            idx shortres = 0, longres = 0;
            sStr outfile;
            outfile.cut(0);
            sStr ext;
            if (isHiveseq){
                ext.printf("hiveseq");
            }else if (isMerged){
                ext.printf("vioseq2");
            }else{
                ext.printf("vioseqlist");
            }
            do {
                if( isHiveseq ) {
                    // Do not parse the file, because it is a hiveseq file
                    // We need to wait for dmCompressor to copy the file into destination
                    obj.getFile(outfile);
                    if( !sFile::exists(outfile.ptr()) ) {
                        reqReSubmit(req, 60);
                        reqProgress(-1, 80, 100);
                        return 0;
                    }
                    // Test the file in next {} block
                    break;
                }
                sVec<idx> reqList;
                grp2Req(masterId, &reqList, mysvcName);
                // check if all parts are done w/o error
                sVec<sVec<sVioseq2::Infopart> > partList;
                partList.resize(reqSliceCnt);
                sVec<idx> countRes;
                countRes.resize(reqSliceCnt);

                shortres = 0;
                sStr partFiles00;
                // Generate the list of files to concatenate or to create the vioseqlist
                for(idx i = 0; i < reqSliceCnt; ++i) {
                    sStr newtempfile;
                    if( !obj.getFilePathname(newtempfile, "~tmp.%" DEC ".vioseq2", i) ) {
                        errmsg.printf("failed to access chunk file %" DEC, i);
                        break;
                    }
                    sStr partfile;
                    reqDataPath(reqList[i], RELFIXTBL, &partfile);
                    if( !partfile) {
    //                    errmsg.printf("There is no relation FixTable for req: %" DEC, reqList[i]);
                        countRes[i] = 0;
                        continue;
                    }
                    sStr l;
                    reqGetData(reqList[i], COUNTLONG, &l);
                    if( !l) {
    //                    errmsg.printf("There is no relation FixTable for req: %" DEC, reqList[i]);
                        continue;
                    }
                    idx lres=0;
                    sscanf(l,"%" DEC,&lres);
                    sVioseq2 bioseq(newtempfile);
                    if( !bioseq.dim() ) {
    //                    errmsg.printf("Error reading vioseq file: %s", newtempfile.ptr());
                        countRes[i] = 0;
                        continue;
                    }
                    bioseq.setmode(sBioseq::eBioModeShort);
                    countRes[i] = bioseq.dim();
                    shortres += countRes[i];
                    longres += lres;
                    /*partList[i].init( partfile.ptr(), sMex::fReadonly);
                    if( !partList[i].ok()) {
                        errmsg.printf("Sequence ID line mapping failed %" DEC, i);
                        break;
                    }*/
                    if( reqSliceCnt > 1 && !sFile::size(partfile)) {
                        errmsg.printf("Sequence ID line mapping failed %" DEC, i);
                        break;
                    }
                    partFiles00.add(partfile.ptr(),partfile.length());
                    partFiles00.add0(1);
                }

                if (longres==0){
                    errmsg.printf("The file has 0 valid sequences");
                    reporterrorasWarning = true;
                }
                reqProgress(-1, 90, 100);

                partFiles00.add0(1);

                if( errmsg ) {
                    break;
                }

                //  Fix the Relations of the first file
                sStr firstfile;
                if( !obj.getFilePathname(firstfile, "~tmp.0.vioseq2") ) {
                    errmsg.printf("Failed to access sequence chunk file 0");
                    break;
                }

                if( reqSliceCnt > 1 ) {  // If there is only 1 slice, there is no need to fix relationships
                    // I must fix AddRelation for the first vioseq2
                    sVioDB dbi(firstfile, 0, 0, 0);
                    idx rc = sVioseq2::fixAddRelation(&dbi, 0/*&partList*/, countRes,partFiles00.ptr(), reqProgressStatic, this);
                    if( rc != 0 ) {
                        errmsg.printf("failed to fix relation %" DEC, rc);
                        break;
                    }
//                    longres = dbi.GetRecordCnt(sVioseq2::eRecID_TYPE);    // Get the count of long mode sequences to return
                }

                // Create a list with the files
                sStr filenames;
                for(idx i = 0; i < reqSliceCnt; i++) {
                    sStr oldfile, newfile;
                    if ( (countRes[i] == 0) && (i != 0) ){continue;}
                    if( !obj.getFilePathname(oldfile, "~tmp.%" DEC ".vioseq2", i) ) {
                        errmsg.printf("failed to access chunk file %" DEC " (1)", i);
                        break;
                    }
                    if( !obj.addFilePathname(newfile, true, ".%" DEC ".vioseq2", i) ) {
                        errmsg.printf("failed to add chunk file %" DEC " (1)", i);
                        break;
                    }
                    if( !isMerged ) {  // Append file:// to the names
                        sFilePath tmpfile(newfile, "%%flnm");
                        filenames.printf("file://%s,0,%" DEC "\n", tmpfile.ptr(0), countRes[i]);
                    } else {  // Separate only by ','
                        if( i != 0 ) {
                            filenames.printf(",");
                        }
                        filenames.printf("%s", oldfile.ptr(0));
                    }
                }
                reqProgress(-1, 93, 100);
                if( !obj.addFilePathname(outfile, true, ".%s", ext.ptr(0)) ) {
                    errmsg.printf("failed to create destination");
                    break;
                }
                if( !isMerged ) {
                    //  Put the files in the vioseqlist
                    sFil fp(outfile);
                    fp.cut(0);
                    if( fp.ok() ) {
                        fp.printf("%s", filenames.ptr(0));
                    } else {
                        errmsg.printf("Can't open/write in vioseqlist final destination");
                        break;
                    }
                    // Rename all the files
                    for(idx i = 0; i < reqSliceCnt; i++) {
                        if ( (countRes[i] == 0) && (i != 0) ){continue;}
                        sStr oldfile, newfile;
                        if( !obj.getFilePathname(oldfile, "~tmp.%" DEC ".vioseq2", i) ) {
                            errmsg.printf("failed to access chunk file %" DEC " (2) to rename it", i);
                            break;
                        }
                        if( !obj.addFilePathname(newfile, true, ".%" DEC ".vioseq2", i) ) {
                            errmsg.printf("failed to add chunk file %" DEC " (2) to rename it", i);
                            break;
                        }
                        sVioDB dbren(oldfile);
                        dbren.renameAllFiles(newfile);
                    }
                } else {
                    // Do not create a vioseqlist, but a single file instead
                    sVioDB db;
                    db.concatFiles(outfile, filenames, "vioseq2", isSingle, true);
                }
                reqProgress(-1, 94, 100);
            } while (false);

            if(!errmsg ) {
                // cast object
                sUsrObj* hobj = obj.cast(dstType);
                if( !hobj ) {
                    errmsg.printf("Cannot cast to '%s'", dstType.ptr());
                    break;
                }
                reqProgress(-1, 95, 100);
                sHiveseq hs(user, outfile);
                reqProgress(-1, 96, 100);
                if( isHiveseq ) {
                    shortres = hs.dim();
                    longres = hs.longcount();
                }
                reqProgress(-1, 97, 100);
                hobj->propSetI("rec-dim", shortres);
                hobj->propSetI("rec-count", longres);
//                hobj->propSetI("rec-count", hs.longcount());

                if( hobj != &obj ) {
                    delete hobj;
                }
                hobj = 0;
                if( hs.dim() ) {
                    if (dmArchiver::getQCFlag(*this) != 0){
                        // Will launch dna-qc
                        DnaQC dnaqc(*this, objID);
                        idx reqsubmitedQC = dnaqc.launch(*user, grpId);
                        logOut(eQPLogType_Info, "Submitted %s request %" DEC "\n", dnaqc.getSvcName(), reqsubmitedQC);
                    }
                    if (dmArchiver::getScreenFlag(*this) != 0){
                        // Will launch dna-screening for hexagon and blast
                        // Uncomment these 3 lines to launch dna-screening:
                        DnaScreening dnascreen(*this, objID, DnaScreening::eBlastVsNT);
                        idx priority = 1000;
                        idx reqsubmitScreen = dnascreen.launch(*user, grpId, 0, priority); // grpID=0 not to attach to the main group - otherwise its too long and makes it seem slower to parse
                        logOut(eQPLogType_Info, "Submitted %s request %" DEC "\n", dnascreen.getSvcName(), reqsubmitScreen);
                    }
                } else {
                    errmsg.printf("Failed to read the final file: hs.dim() is 0");
                    break;
                }
            }

            if( errmsg ) {
                // Clean Files: Vioseq files and blob only
                sVec<idx> reqList;
                grp2Req(masterId, &reqList, mysvcName);
                for(idx i = 0; i < reqSliceCnt; i++) {
                    reqProgress(-1, 99, 100);
                    // Delete vioseq2 files
                    sStr newfile;
                    if( !obj.getFilePathname(newfile, ".%" DEC ".vioseq2", i) ) {
                        continue;
                    }
                    sVioDB dbren(newfile);
                    dbren.deleteAllJobs();
                    // Delete reqDataPath
                    sStr partfile;
                    reqDataPath(reqList[i], RELFIXTBL, &partfile);
                    sFile::remove(partfile);
                }
            }
#ifndef _DEBUG
            // Delete fixRelation files after parser has completed last step, in release build only
            if (!errmsg){
                sVec<idx> reqList;
                grp2Req(masterId, &reqList, mysvcName);
                sStr partfile;
                for(idx i = 0; i < reqSliceCnt; i++) {
                    // Delete reqDataPath
                    partfile.cut(0);
                    reqDataPath(reqList[i], RELFIXTBL, &partfile);
                    sFile::remove(partfile);
                }
            }
#endif
        }
    } while( false );

    if( !errmsg ) {
        if ( reqProgress(-1, 100, 100) ) { //Do not change status if request was stopped.
            reqSetProgress(req, -1, 100);
            reqSetStatus(req, eQPReqStatus_Done);
        }
    } else {
        eQPInfoLevel outInfolevel = reporterrorasWarning ? eQPInfoLevel_Warning : eQPInfoLevel_Error;
        sStr usrFileName;
        const char * filename = formValue("userFileName", &usrFileName, 0);
        if (filename){
            reqSetInfo(req, outInfolevel, "File %s: %s", filename, errmsg.ptr());
        }
        else {
            reqSetInfo(req, outInfolevel, "%s", errmsg.ptr());
        }
        if (reporterrorasWarning){
            // Cast object to u-file and report Status as DONE
            const sHiveId objID(formValue("obj"));
            sUsrFile obj(objID, user);
            sUsrObj* hobj = obj.cast("u-file");
            if( !hobj ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Cannot cast to u-file");
            }
            reqProgress(-1, 100, 100);
            reqSetStatus(req, eQPReqStatus_Done);
        }
        else {
            reqSetStatus(req, eQPReqStatus_ProgError);
        }
    }
    return 0;

}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaParserProc backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-parser",argv[0]));
    return (int)backend.run(argc,argv);
}

