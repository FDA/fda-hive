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
#ifndef sHiveAl_hpp
#define sHiveAl_hpp

#include <ssci/bio.hpp>

namespace sviolin
{
    class sHiveal : public sBioalSet
    {
        private:
            sStr bioalSourceNames;
            struct Bioal{
                sBioal* bioal;
                bool selfManaged;
                idx ofsFileName;
            };

            sDic < Bioal > bioallist;
            const sUsr * user;

            sBioseq::EBioMode parseMode(const char * filename, bool isSub);

        public:
            void registerBioal(sBioal * bioseq, const char * filename=0, idx alNum=0, idx alCnt=0,  bool selfman=true);
            idx digestFile( const char * filename , idx alNum=1, idx alCnt=0,  bool selfman=true);
            sHiveal * parse (const char * filename, sUsr * luser=0 );

            sBioal * getBioal(const char * filename, bool &selfman);

            sBioseq::EBioMode parseSubMode(const char * filename){ return parseMode(filename, true);}
            sBioseq::EBioMode parseQryMode(const char * filename){ return parseMode(filename, false);}

            idx expandHiveal( sStr * buf, const char * filename, idx alStart=1, idx alEnd=0, const char * separ=0 , const char * sourceFile=0);

            sStr log;

            sHiveal (const sUsr * usr=0, const char * filename=0, idx mode=0 )
            {
                user=usr;

                if(!filename)return;
                parse(filename);
            }


            ~sHiveal (){

                for(idx i=0; i<bioallist.dim(); ++i){
                    if( bioallist[i].selfManaged )
                        delete bioallist[i].bioal;
                }
            }

    };


}

#endif 