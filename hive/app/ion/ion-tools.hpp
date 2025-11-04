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
#pragma once
#ifndef sIonTools_hpp
#define sIonTools_hpp


#include <slib/std.hpp>
#include <ion/sIon.hpp>
#include <ion/sIon-client.hpp>

namespace slib {

class sSql;
class sUsr;

class sIonTools
{
    public:

    idx gSilent,gVerbose,gNoLimit,ion_LevelMax,gLazy;
    idx gSeparField, gSeparRec,gSeparAttribs,gComment,gUseSynonym;
    idx debug,stream,basicIndex,gModeGeneral,hashmode;
    idx gStart,gCnt;
    char recordProtectQuote,vaxQuoteDoNotProtect;
    sStr buf;
    sUsr * user;
    sSql * qdb;

    sIonTools ()
    {
        gSilent=0;
        gVerbose=1;
        gNoLimit=0;
        gLazy=0;
        gModeGeneral=0;
        gUseSynonym=0;
        hashmode=0;
        user=0;
        ion_LevelMax=1024;
        gSeparField=sNotIdx;
        gSeparRec=sNotIdx;
        gSeparAttribs=sNotIdx;
        gComment=sNotIdx;
        debug=0;
        stream=0;
        basicIndex=0;
        recordProtectQuote=0;
        vaxQuoteDoNotProtect=0;
    }

    virtual void printf(const char * formatDescription , ...) {
        if(gSilent)return;
        sStr str;sCallVarg(str.vprintf,formatDescription);
        if(str.length())fwrite(str.ptr(), str.length(),1,stdout) ;
    }

    static sCmdLine::exeCommand cmdExes[];

};

class sIon_QLibrary
{
        public:
        struct iqLibElement{ const char * name; const char * iql; };
        private:
        static iqLibElement IQLIB[];
        public:
        static iqLibElement * find(const char * name ) ;

};

}

#endif
