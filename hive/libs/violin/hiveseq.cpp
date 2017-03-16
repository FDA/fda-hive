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
#include <ulib/ulib.hpp>
#include <violin/violin.hpp>
#include "common.hpp"

using namespace sviolin;

const char * const sHiveseq::defaultSliceSizeS = "1000000";
const idx sHiveseq::defaultSliceSizeI = 1000000;
const char * sHiveseq::default_log_suffix = "\n";

typedef enum
{
    eFileHiveseq,
    eFileVioseqList,
    eFileSRASeq,
    eFileVioseq2,
    eFileVioseq,
    eFileLast
} ETypeList;
const char * const typeList00 = ".hiveseq" _ ".vioseqlist" _ ".sra" _ ".vioseq2" _ ".vioseq" __;

bool sHiveseq::parse(const char * filename, EBioMode mode, bool allowDigestFailure, const sUsr * luser)
{
    if(luser)
        user=luser;

    sStr list,dst,d;

    EBioMode tmode = mode;
    if (!isVioseqlist){
        //sString::searchAndReplaceStrings(&dst,filename,0, sString_symbolsEndline, 0,0,false );
        sString::searchAndReplaceSymbols(&dst,filename,0,";" sString_symbolsEndline,0,0,true,true,false, false);
        for ( const char * ff=dst; ff ; ff=sString::next00(ff) ) {
            idx isValid = expandHiveseq( &list, ff);
            if( isValid == 0 ) {
                if( log ) {
                    log->printf("%sFailed to parse sequence object %s belonging to the following list: ", log_prefix, ff);
                    sString::searchAndReplaceSymbols(log, filename, 0, ";" sString_symbolsEndline, ";", 0, true, true, false, false);
                    log->addString(log_suffix);
                }
                if( !allowDigestFailure ) {
                    empty();
                    return false;
                }
            } else {
                objSourceIDs.printf("%s",ff);
                objSourceIDs.add0();
            }
        }
        objSourceIDs.add0(2);
        list.add0(2);
    }
    else{
        sString::searchAndReplaceSymbols(&list,filename,0,";" sString_symbolsEndline,0,0,true,true,false, false);
        list.add0(2);
        setDimVioseqlist((idx)-2);  // Initialize to -2 to identify a vioseqlist in short mode
    }
    if (isVioseqlist && mode == eBioModeLong){
        setDimVioseqlist((idx)-1);  // Initialize to -1 to identify a vioseqlist in long mode
        tmode = eBioModeLong;
        mode = eBioModeShort;
    }

    bool digestFailure = false;
    for ( const char * s, *p=list; p ; p=sString::next00(p) ) {
        d.cut(0);sString::searchAndReplaceSymbols(&d,p,0,",",0,0,true,true,false, false);
        const char * locFilename=d.ptr(0);
        idx locStart=1;s=sString::next00(locFilename);if(s)sscanf(s,"%" DEC,&locStart);
        idx locEnd=0;s=sString::next00(s);if(s)sscanf(s,"%" DEC,&locEnd);
        if( !digestFile(locFilename ,locStart,locEnd, 0,0, true, tmode, allowDigestFailure) ) {
            digestFailure = true;
        }
        tmode = mode;
    }
    if( digestFailure && !allowDigestFailure ) {
        empty();
        return false;
    }

    return true;
}

bool sHiveseq::digestFile( const char * filename , idx seqStart, idx seqEnd, idx partialRangeStart, idx partialRangeLen,  bool selfman, sBioseq::EBioMode mode, bool allowSubSeqFailure)
{
    if( !sFile::exists(filename) ) {
        if( log ) {
            log->printf("%sHiveseq file does not exist%s", log_prefix, log_suffix);
        }
        return false;
    }

    Bios * b = bioseqlist.get(filename);
    if(b) {
        registerBioseq (0, filename, seqStart, seqEnd, partialRangeStart, partialRangeLen,  false);
        return true;
    }

    sStr flnm("%s",filename);
    sHiveId id(flnm.cmp("obj://", 6) ? flnm.ptr() : flnm.ptr(6));

    if( !getObjFilePathname00(user, flnm, flnm, typeList00) ) {
        if( log ) {
            log->printf("%sObject %s is inaccessible or doesn't have hiveseq data%s", log_prefix, flnm.ptr(), log_suffix);
        }
        return false;
    }
    if(strncmp(flnm,"./",2)==0 ){
        flnm.printf(0,"%s",flnm.ptr(2));
    }
    // first make sure this is not a gzip file name
    // if it is - strip the .gz suffix before looking for filename
    char * ext=strrchr(flnm,'.');

    // now try to deduce the final sequence
    // file name from the given filename
    idx fileType=sNotIdx;
    if( ext ) {
        sString::compareChoice(ext, typeList00, &fileType, true, 0, true);
    }
    // if not a recognized file format
    // try to find one in the same directory
    if( fileType == sNotIdx ) {
        sFilePath path(flnm, "%%pathx.vioseq");
        if( sFile::exists(path) && sFile::size(path) > 0 ) {
            fileType = eFileVioseq;
            flnm.printf(0,"%s",path.ptr(0));
        } else {
            path.cut(0);
            path.makeName(flnm, "%%pathx.vioseqlist");
            if( sFile::exists(path) && sFile::size(path) > 0 ) {
                fileType = eFileVioseqList;
                flnm.printf(0,"%s",path.ptr(0));
            } else {
                path.cut(0);
                path.makeName(flnm, "%%pathx.vioseq2");
                if( sFile::exists(path) && sFile::size(path) > 0 ) {
                    fileType = eFileVioseq2;
                    flnm.printf(0,"%s",path.ptr(0));
                }
            }
        }
    }
    sBioseq * bioseq = 0;
    if(fileType==eFileVioseq){
        bioseq=new sVioseq(flnm, true);
    } else if(fileType==eFileVioseq2){
        bioseq=new sVioseq2(flnm);
    } else if(fileType==eFileVioseqList){
        sFil vioseqcontent(flnm,sMex::fReadonly);
        if(!vioseqcontent)return 0;
        sStr hlist;
        sFilePath flnmpath(filename, "%%dir/");
        sString::searchAndReplaceStrings(&hlist,vioseqcontent.ptr(),vioseqcontent.length(),"file://" __, (flnmpath.length() > 2) ? flnmpath.ptr() : "./" __,0,false);
        sHiveseq * subhiveseq = new sHiveseq(0, 0, mode, true, false, log, log_prefix, log_suffix);
        if( subhiveseq->parse(hlist.ptr(), mode, allowSubSeqFailure) ) {
            bioseq = subhiveseq;
        } else {
            delete subhiveseq;
        }
    /*} else (fileType==eFileSRAseq){
        bioseq=new sVioseq2(flnm);
        */
    }
    if( !bioseq ) {
        if( log ) {
            log->printf("%sObject %s doesn't have valid sequence data%s", log_prefix, id.print(), log_suffix);
        }
        return false;
    }
    bioseq->setmode(mode);
    if (getDimVioseqlist() == -1){
        bioseq->setmode(eBioModeLong);
        setDimVioseqlist(bioseq->dim());
        bioseq->setmode(eBioModeShort);
    }
    registerBioseq (bioseq, filename, seqStart, seqEnd, partialRangeStart, partialRangeLen,  selfman);
    bioseq->setmode(mode);

    return true;
}

void sHiveseq::registerBioseq (sBioseq * bioseq, const char * filename, idx seqStart, idx seqEnd, idx partialRangeStart, idx partialRangeLen,  bool selfman)
{

    Bios * b = bioseqlist.set(filename);
    if(bioseq){
        b->bioseq=bioseq;
        b->selfManaged=selfman;
        b->ofsFileName=bioseqSourceNames.printf("%s",filename)-bioseqSourceNames.ptr();
        bioseqSourceNames.add0(2);
    }else
        bioseq=b->bioseq;

    if(!bioseq)return ;
    if (seqStart == 0) {seqStart = 1;}

    idx seqCnt=(seqEnd>=seqStart) ? seqEnd-seqStart+1 : 1;

    if(!seqEnd)seqCnt=0;

    sBioseqSet::attach(bioseq, seqStart, seqCnt, partialRangeStart, partialRangeLen);

}




/*
sHiveseq::Bios * sHiveseq::exposeBioseq (idx * inum )
{
    sBioseq * bioseq=ref(inum);

    for( idx i=0; i<bioseqlist.dim(); ++i) {
        if(bioseqlist[i].bioseq==bioseq)
            return bioseqlist.ptr(i);

    }
    return 0;
}


*/

idx sHiveseq::expandHiveseq(sStr * buf, const char * filename, idx seqStart, idx seqEnd, idx partialRangeStart, idx partialRangeLen, const char * separ, const char * sourceFilePath)
{
    if( !filename ) {
        if( log ) {
            log->printf("%sNull object ID cannot be used as hiveseq", log_prefix);
        }
        return 0;
    }

    sStr flnm("%s", filename);
    sHiveId id(flnm.cmp("obj://", 6) ? flnm.ptr() : flnm.ptr(6));
    const char * s;

    if( !getObjFilePathname00(user, flnm, flnm, typeList00) ) {
        if( log ) {
            log->printf("%sObject %s is inaccessible or doesn't have hiveseq data%s", log_prefix, filename, log_suffix);
        }
        return 0;
    }
    // Should remove prefix of file
    if( sourceFilePath && strncmp(flnm, "file://", 7) == 0 ) {
        sFilePath tmpfile(sourceFilePath, "%%dir%s", flnm.ptr(7));
        flnm.printf(0, "%s", tmpfile.ptr());
    }

    // first make sure this is not a gzip file name
    // if it is - strip the .gz suffix before looking for filename
    char * ext = strrchr(flnm, '.');
    if( ext && strncmp(ext, ".gz", 3) == 0 )
        *ext = 0;
    ext = strrchr(flnm, '.');
    // if not a recognized file format
    // try to find one in the same directory
    if( !ext || strcmp(ext, ".hiveseq") != 0 ) {
        sStr d;
        sString::searchAndReplaceSymbols(&d, filename, 0, ",", 0, 0, true, true, false, false);
        const char * locFilename = d.ptr();
        idx locStart = 0;
        s = sString::next00(locFilename);
        if( s && sscanf(s, "%" DEC, &locStart) ) {
            seqStart = locStart;
        }
        idx locEnd = 0;
        s = s ? sString::next00(s) : 0;
        if( s && sscanf(s, "%" DEC, &locEnd) ) {
            seqEnd = locEnd;
        }

        buf->printf("%s,%" DEC ",%" DEC ",%" DEC ",%" DEC, flnm.ptr(0), seqStart, seqEnd, partialRangeStart, partialRangeLen);
        //fprintf(stderr,"%s,%" DEC ",%" DEC ",%" DEC ",%" DEC,flnm.ptr(0),seqStart,seqEnd,partialRangeStart, partialRangeLen);
        buf->addSeparator(separ);
        return 1;
    }

    sStr dst;
    sFil hiveseqcontent(flnm, sMex::fReadonly);
    if( !hiveseqcontent ) {
        if( log ) {
            log->printf("%sObject %s doesn't have valid hiveseq data%s", log_prefix, id.print(), log_suffix);
        }
        return 0;
    }

    //sString::searchAndReplaceStrings(&dst,hiveseqcontent.ptr(),hiveseqcontent.length(), sString_symbolsEndline, 0,0,false );
    sString::searchAndReplaceSymbols(&dst, hiveseqcontent.ptr(), hiveseqcontent.length(), ";" sString_symbolsEndline, 0, 0, true, false, false, false);

    idx cntParsed = 0, firstShift = seqStart - 1;
    for(const char * p = dst; p; p = sString::next00(p)) {
        sStr d;
        sString::searchAndReplaceSymbols(&d, p, 0, ",", 0, 0, true, true, false, false);
        const char * locFilename = d.ptr();

        idx locStart = 0;
        s = sString::next00(locFilename);
        sscanf(s, "%" DEC, &locStart);
        idx locEnd = 0;
        s = sString::next00(s);
        sscanf(s, "%" DEC, &locEnd);

        //idx rStart=locStart-shift;
        //rStart+=firstShift;
        idx rStart = locStart + firstShift;

        idx rEnd = seqEnd ? rStart + (seqEnd - seqStart) : locEnd;        //: rStart+(locEnd-locStart); // -shift

        bool isInRange = (rEnd < locStart || rStart > locEnd) ? false : true;

        if( rStart < locStart )
            rStart = locStart;
        if( rEnd > locEnd )
            rEnd = locEnd;

        //if(firstShift>=locStart-locEnd)

        // check if this range overlaps with the required range
        if( isInRange )
            cntParsed += expandHiveseq(buf, locFilename, rStart, rEnd, partialRangeStart, partialRangeLen, 0, flnm);

        //shift+=rEnd-rStart+1;
        firstShift -= locEnd - locStart + 1;

        //seqStart-=shift;
        //if(seqEnd)seqEnd-=shift;

    }
    if( !cntParsed && log ) {
        log->printf("%sObject %s contains a composite hiveseq without valid elements%s", log_prefix, id.print(), log_suffix);
    }
    return cntParsed;
}

