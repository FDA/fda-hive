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
#ifndef sLib_sIonHC_hpp
#define sLib_sIonHC_hpp

#include <ion/sIon-birel.hpp>

namespace slib {

    class sIonHC : public sIonBirel
    {
        public:

            sIO tbuf;

            static const char * typeExpansionIQL;

        public:
            sIonHC(const char * baseName=0, idx lopenMode=0, sIO * out=0, sIO * lerrIO=0):sIonBirel(baseName,lopenMode, out, lerrIO){
                objectIteratorCallback=realObjectIteratorCallback;
            }



            sVar typeAliasDic;
            idx getInheritedTypes( const char * typenames,  idx lentp, sIO * out );
            void attachTypeIterators( sIO * out=0);



            callbackIterator objectIteratorCallback;
            void attachObjectListTraverseQuery(sIO * out);
            idx iterateObjects( const char * typenames=0, idx typenameslen=0, const char * ids =0, idx idslen=0, const char* baseline="?obj");
            idx iterateObjectsIQL( const char * typenames=0, idx typenameslen=0, const char * ids =0, idx idslen=0);
            static idx realObjectIteratorCallback(void * param, LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout );



            void attachGroupListListTraverseQuery (sIO * out);
            void attachUserIterators( sIO * out);
            static idx userIteratorCallback(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout );


            idx addRule( const char * act, const char * party, const char * obj, const char * infparty, const char * infobj );

    };

};

#endif
