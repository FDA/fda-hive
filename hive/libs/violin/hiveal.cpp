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
#include <violin/hiveal.hpp>

#include "common.hpp"

using namespace sviolin;

typedef enum
{
    eFileHiveAl, eFileVioal, eFileLast = sNotIdx
} ETypeList;
const char * const typeList00 = ".hiveal" _ ".vioal" _ "alignment.hiveal" __;

sHiveal * sHiveal::parse (const char * filename, sUsr * luser )
{
    if(luser)
        user=luser;

    sStr list,dst,d;

    sString::searchAndReplaceSymbols(&dst,filename,0,";" sString_symbolsEndline,0,0,true,true,false, false);
    for ( const char * ff=dst; ff ; ff=sString::next00(ff) ) {
        expandHiveal( &list, ff);
    }
    list.add0(2);

    for ( const char * s, *p=list; p ; p=sString::next00(p) ) {
        d.cut(0);sString::searchAndReplaceSymbols(&d,p,0,",",0,0,true,true,false, false);
        const char * locFilename=d.ptr(0);
        idx locStart=1;s=sString::next00(locFilename);if(s)sscanf(s,"%" DEC,&locStart);
        idx locEnd=0;s=sString::next00(s);if(s)sscanf(s,"%" DEC,&locEnd);
        digestFile(locFilename ,locStart,locEnd, true);
    }
    return this;
}

idx sHiveal::digestFile( const char * filename , idx alStart, idx alEnd,  bool selfman)
{
    if(!filename)return 0;


    Bioal * b = bioallist.get(filename);
    if(b) {
        registerBioal (0, filename, alStart, alEnd, false);
        return 1;
    }

    sBioal * bioal = 0;

    bool isSelfman;
    bioal = getBioal(filename, isSelfman);
    if(!isSelfman) isSelfman = selfman;

    if( bioal )
        registerBioal (bioal, filename, alStart, alEnd,  isSelfman);

    return 1;

}

sBioal *sHiveal::getBioal(const char * filename, bool &selfman)
{
    sStr flnm("%s",filename);

    if( !getObjFilePathname00(user, flnm, flnm, typeList00) ) {
        return 0;
    }
    char * ext=strrchr(flnm,'.');
    if(ext && strncmp(ext,".gz",3)==0) *ext=0;

    idx fileType=sNotIdx;
    if(ext)sString::compareChoice( ext, typeList00,&fileType,true, 0,true);

    selfman = false;

    sBioal * bioal = 0;

    if( fileType==sNotIdx ){
        sFilePath path(flnm,"%%pathx.vioal");
        bioal=new sVioal(path, 0,0,sMex::fReadonly);

        if(!bioal->dimAl()){
            delete bioal;
            bioal=0;
        }
        else {
            selfman = true;
        }
    }

    else if(fileType==eFileVioal){
        bioal=new sVioal(flnm, 0,0,sMex::fReadonly);
    }
    return bioal;
}

void sHiveal::registerBioal (sBioal * bioal, const char * filename, idx alStart, idx alEnd,  bool selfman)
{

    Bioal * b = bioallist.set(filename);
    if(bioal){
        b->bioal=bioal;
        b->selfManaged=selfman;
        b->ofsFileName=bioalSourceNames.printf("%s",filename)-bioalSourceNames.ptr();
        bioalSourceNames.add0(2);
    }else
        bioal=b->bioal;

    if(!bioal)return ;

    idx alCnt=(alEnd>=alStart) ? alEnd-alStart+1 : 1;

    if(!alEnd)alCnt=0;

    sBioalSet::attach(bioal, alStart, alCnt);

}

sBioseq::EBioMode sHiveal::parseMode(const char * filename, bool isSub) {
    sStr list,dst,d;

    sString::searchAndReplaceSymbols(&dst,filename,0,";" sString_symbolsEndline,0,0,true,true,false, false);
    for ( const char * ff=dst; ff ; ff=sString::next00(ff) ) {
        expandHiveal( &list, ff);
    }
    list.add0(2);

    sBioal * bioal = 0;
    sBioseq::EBioMode ret = sBioseq::eBioModeShort;
    bool selfman = false;
    for ( const char * s, *p=list; p ; p=sString::next00(p) ) {
        d.cut(0);sString::searchAndReplaceSymbols(&d,p,0,",",0,0,true,true,false, false);
        const char * locFilename=d.ptr(0);
        idx locStart=1;s=sString::next00(locFilename);if(s)sscanf(s,"%" DEC,&locStart);
        idx locEnd=0;s=sString::next00(s);if(s)sscanf(s,"%" DEC,&locEnd);

        bioal = getBioal(locFilename, selfman);
        if(bioal &&  sBioseq::isBioModeLong( isSub?bioal->getSubMode():bioal->getQryMode() ) )
            return sBioseq::eBioModeLong;
    }
    return ret;
}

idx sHiveal::expandHiveal( sStr * buf, const char * filename , idx alStart, idx alEnd, const char * separ, const char * sourcePath)
{
    if(!filename)return 0;

    sStr flnm("%s",filename);
    const char * s;

    if( !getObjFilePathname00(user, flnm, flnm, typeList00) ) {
        return 0;
    }

    if(sourcePath  &&  strncmp(flnm,"file://",7)==0 ){
        sFilePath tmpfile(sourcePath, "%%dir/%s",flnm.ptr(7));
        flnm.printf(0,"%s",tmpfile.ptr());
    }

    char * ext=strrchr(flnm,'.');
    if(ext && strncmp(ext,".gz",3)==0) *ext=0;
    ext=strrchr(flnm,'.');
    if(!ext || strcmp(ext,".hiveal")!=0 ) {
        sStr d;
        sString::searchAndReplaceSymbols(&d,filename,0,",",0,0,true,true,false, false);
        const char * locFilename = d.ptr();
        idx locStart=0;
        s=sString::next00(locFilename);
        if(s && sscanf(s,"%" DEC,&locStart)){
            alStart = locStart;
        }
        idx locEnd=0;
        s=s ? sString::next00(s):0;
        if(s && sscanf(s,"%" DEC,&locEnd)){
            alEnd = locEnd;
        }

        buf->printf("%s,%" DEC ",%" DEC,flnm.ptr(0),alStart,alEnd);
        buf->addSeparator(separ);
        return 1;
    }

    sStr dst;
    sFil hiveseqcontent(flnm,sMex::fReadonly);
    if(!hiveseqcontent)return 0;

    sString::searchAndReplaceSymbols(&dst,hiveseqcontent.ptr(),hiveseqcontent.length(), ";" sString_symbolsEndline,0,0,true,false,false, false);

    idx cntParsed=0,firstShift=alStart-1;
    for ( const char * p=dst; p; p=sString::next00(p)) {
        sStr d;
        sString::searchAndReplaceSymbols(&d,p,0,",",0,0,true,true,false, false);
        const char * locFilename =d.ptr();

        idx locStart = 0;
        s = sString::next00(locFilename);
        sscanf(s, "%" DEC, &locStart);
        idx locEnd = 0;
        s = sString::next00(s);
        sscanf(s, "%" DEC, &locEnd);

        idx rStart=locStart+firstShift;

        idx rEnd=alEnd ? rStart+(alEnd-alStart) : locEnd ;

        bool isInRange= (rEnd<locStart || rStart>locEnd) ? false : true;

        if(rStart<locStart)rStart=locStart;
        if(rEnd>locEnd)rEnd=locEnd;


        if(isInRange)
            cntParsed+=expandHiveal( buf, locFilename , rStart, rEnd, separ, flnm);

        firstShift-=locEnd-locStart+1;


    }
    return cntParsed;
}

