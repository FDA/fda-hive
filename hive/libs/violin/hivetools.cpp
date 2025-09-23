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
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <ulib/uquery.hpp>
#include <dmlib/dmlib.hpp>

bool sHiveTools::traveseProp(sUsrObj & obj, sUsr & user, const char * prop, const char * traverse_prop, const char * type_filter, sStr * log)
{
    const char * val0  = obj.propGet00(prop, 0, ",");
    if( !val0 ) {
        return true;
    }
    sStr qry("result = dic(); "
        "f = { "
        "    if (._type == %s) {"
        "        result[this] = 1;"
        "    } else if (.child) {"
        "        .%s.foreach(f);"
        "    }"
        "};"
        "((objlist)'%s').foreach(f);"
        "return result;", type_filter, traverse_prop, val0);

    idx logpos1 = 0, logpos2 = 0;
    if( log ) {
        logpos1 = log->pos();
        log->printf("\nerr.%s._err=", obj.IdStr());
        logpos2 = log->pos();
    }
    sVariant * v = 0;
    sUsrQueryEngine qe(user);
    if( qe.parse(qry, qry.length(), log) ) {
        v = qe.run(log);
    }
    if( !v || !v->isDic() || !v->dim() ) {
        if( log && log->pos() == logpos2 ) {
            log->cut0cut(logpos1);
            return true;
        }
        return false;
    }
    if( log ) {
        log->cut0cut(logpos1);
    }
    sVariant tmpv;
    sVec<const char *> all;
    all.resize(v->dim());
    for(idx i = 0; i < v->dim(); ++i) {
        all[i] = v->getDicKeyVal(i, tmpv);
    }
    if( obj.propSet(prop, 0, &all[0], all.dim()) != (udx) all.dim() ) {
        if( log ) {
            log->printf("\nerr.%s.%s=Failed to save property", obj.IdStr(), prop);
        }
        return false;
    }
    return true;
}

idx sHiveTools::customizeSubmission(sVar * pForm , sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log, sMex **pFileSlices)
{
    sUsrObj * so = (sUsrObj *) obj;
    sStr splitSubs, subSets, t, sScissors, sSplitOn;

    const char * batch = so ? so->propGet00("batch_param") : 0;
    if( batch && *batch ) {
        return 1;
    }
    const bool isDnaSvc = sIs("dna-", pSvc->name);
    if( so && isDnaSvc) {
        if( !traveseProp(*so, *user, "query", "child", "'nuc-read' | 'svc-dna-refClust'", log) ||
            !traveseProp(*so, *user, "subject", "child", "'genome'", log) ) {
            return 0;
        }
    }
    idx cntParallel = 1;

    const char * splitType = so ? so->propGet("scissors", &sScissors) : 0;
    if( !splitType ) {
        splitType = pForm->value("scissors");
    }
    if( !splitType ) {
        splitType = "hiveseq";
    }

    const char * splitOn = so ? so->propGet("split", &sSplitOn) : 0;
    if( !splitOn ) {
        splitOn = pForm->value("split", "query");
    }
    if( strcmp(splitOn, "-") == 0 ) {
        return cntParallel;
    }
    const char * splitPart = so ? so->propGet("slice", &splitSubs) : 0;
    if( !splitPart )
        splitPart = pForm->value("slice", sHiveseq::defaultSliceSizeS);
    idx singleSplitOn = (so && so->propGetI("splitSingle")) ? so->propGetI("splitSingle") : 0;

    const char * subSet = so ? so->propGet("subSet") : 0;
    sVec<idx> subsetN;
    if( subSet ) {
        sString::scanRangeSet(subSet, 0, &subsetN, -1, 0, 0);
    }
    const char * alignSelector = so ? so->propGet("alignSelector") : 0;
    if ( alignSelector && strncmp(alignSelector,"svc-align-blast",15)!=0 ){
        sVec<sHiveId> subectIDs;
        so->propGetHiveIds("subject",subectIDs);
        idx tot_size = 0;
        for( idx s_id = 0 ; s_id < subectIDs.dim() ; ++s_id){
            sUsrObj objSub(*user,subectIDs[s_id]);
            tot_size+=objSub.propGetI("size");
            if ( tot_size>100000000000 ){
                if( log ) {
                    log->printf("\n Size of reference genome file%s too big.\n"
                        " Use Blast or Censuscope for faster results.", subectIDs.dim()>1?"s":"");
                }
                return 0;
            }
        }
    }

    sStr log_prefix;
    if( splitOn ) {
        sStr lst;
        sString::searchAndReplaceSymbols(&lst, splitOn, 0, ",\r\n", 0, 0, true, true, true, true);
        sVec<idx> splitP;
        sString::scanRangeSet(splitPart, 0, &splitP, 0, 0, 0);
        const char * spl;
        idx is;

        idx hiveseqModeArg = so ? so->propGetI("hiveseqMode") : 0;
        sBioseq::EBioMode hiveseqMode = sBioseq::eBioModeShort;
        if( sBioseq::isBioModeLong( hiveseqModeArg ) ) {
            hiveseqMode = sBioseq::eBioModeLong;
        } else if ( !sBioseq::isBioModeShort( hiveseqModeArg ) ) {
            log->printf( "Unknown biomode: %" DEC, hiveseqModeArg);
            return 0;
        }
        for(spl = lst, is = 0; spl; spl = sString::next00(spl), is = sMin(is + 1, splitP.dim() - 1)) {
            const char * spl_vals = pForm->value(spl);
            if( !spl_vals && so ) {
                spl_vals = so->propGet00(spl, &splitSubs, "\n");
            }
            if( !spl_vals ) {
                cntParallel = cntParallel ? cntParallel : 1;
                continue;
            }
            if( strcmp(spl, "list") == 0 ) {
                sVec<idx> lstN;
                sString::scanRangeSet(spl_vals, 0, &lstN, 0, 0, 0);
                cntParallel *= lstN.dim();
            } else if( strcmp(spl, "nrepeat") == 0 ) {
                cntParallel *= atoidx(spl_vals);
            } else if( strcmp(spl, "arcdim") == 0 ) {
                sVec <sHiveId> files;
                if (so){
                    so->propGetHiveIds(spl, files);
                }
                sStr * fileSlices = 0;
                if (pFileSlices){
                    *pFileSlices = fileSlices = new sStr();
                    fileSlices->addString("file index,start row\n0,0\n");
                }
                idx totCnt = 0;
                idx fileCnt = 0;
                idx slice = splitP[is];
                idx file_slice_remainder = slice;
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
                    if (fileSlices){
                            idx locCount = fileCnt + (slice - file_slice_remainder);
                            while (locCount > slice){
                                fileSlices->printf("%" DEC ",%" DEC "\n", ifile, file_slice_remainder);
                                locCount -= slice;
                                file_slice_remainder += slice;
                                fileCnt -= slice;
                            }
                            file_slice_remainder = (file_slice_remainder - fileCnt)% slice;
                    }
                }
                cntParallel *= totCnt > 0 ? ((totCnt - 1) / splitP[is] + 1) : 0;
                if (cntParallel <= 1 && fileSlices ){
                    delete fileSlices;
                    fileSlices = 0;
                    *pFileSlices = 0;
                }
            } else if( strcmp(spl, "lines") == 0 ) {
                sHiveId id(spl_vals);
                if( id.valid() ) {
                    std::auto_ptr<sUsrObj> obj(user->objFactory(id));
                    sUsrFile * file = dynamic_cast<sUsrFile *>(obj.get());
                    if( file ) {
                        sStr buf;
                        sFil f(file->getFile(buf), sMex::fReadonly);
                        if( f.ok() ) {
                            cntParallel *= (f.recCnt(true) - 1) / splitP[is] + 1;
                        }
                    }
                }
            }
            if( !isDnaSvc ) {
                continue;
            }
            idx dim = 1, d;
            sStr spl_vals00;
            sString::searchAndReplaceSymbols(&spl_vals00, spl_vals, 0, ";,\n", 0, 0, true, true, true, true);
            if( strcmp(splitType, "hiveal") == 0 ) {
                dim = 0;
                for(const char *spl_val = spl_vals00; spl_val; spl_val = sString::next00(spl_val)) {
                    sHiveId objID(spl_val);
                    sUsrFile hobj(objID, user);
                    sStr path;
                    idx tdim = 0;
                    if( hobj.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __) ) {
                        sHiveal hiveal(user, path);
                        sBioal * bioal = &hiveal;
                        if( subsetN.dim() ) {
                            tdim = 0;
                            for(idx is = 0; is < subsetN.dim(); ++is) {
                                bioal->listSubAlIndex(subsetN[is], &d);
                                tdim += d;
                            }
                        } else {
                            tdim = bioal->dimAl();
                        }
                    }
                    if( !tdim ) {
                        tdim = 1;
                    }
                    tdim = ceil(((real) (tdim - 1)) / splitP[is]) * splitP[is];
                    dim += tdim;
                }
                if( !dim ) {
                    dim = 1;
                }
            } else {
                sStr splitters;
                for(const char *spl_val = spl_vals00; spl_val; spl_val = sString::next00(spl_val)) {
                    if( (singleSplitOn && spl_val == spl_vals00) || !singleSplitOn) {
                        splitters.printf(";%s", spl_val);
                    }
                }
                if( !splitters ) {
                    break;
                }
                log_prefix.printf(0, "\nerr.%s.%s=", so->IdStr(), spl);
                sHiveseq hs(user, splitters.ptr(1), hiveseqMode, false, false, log, log_prefix);
                dim = hs.dim();
            }
            if( !dim ) {
                cntParallel = 0;
                break;
            } else {
                cntParallel *= ceil(((real) dim) / splitP[is]);
            }
            if( singleSplitOn ) {
                break;
            }
        }
    }
    return cntParallel;
}

