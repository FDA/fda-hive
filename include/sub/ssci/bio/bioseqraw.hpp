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
#ifndef sBio_seqraw_hpp
#define sBio_seqraw_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/var.hpp>
#include <ion/vax.hpp>

#include <zlib.h>

namespace slib {

    class sBioseqRaw
    {
         private:
            gzFile m_gz;
            FILE * m_file;
            idx chunkSize;
            bool isFastq;
        public:
            sIO errorMsg;

        public:
            sBioseqRaw(const char * filename, const char * extensionList, bool allowGz=true)
            {
                init(filename, extensionList, allowGz);
            }
            
            void destroy(){
                if (m_gz) gzclose(m_gz);
            }

            virtual ~sBioseqRaw()
            {
                   destroy();
            }
            
            sBioseqRaw * init(const char * filename, const char * extensionList, bool allowGz=true)
            {
                m_gz = NULL; m_file =0; isFastq=false;
                sStr ext, full_path;
                sString::searchAndReplaceSymbols(&ext , extensionList, 0 , "," , 0, 0, true, true, true, true);
                
                for( const char* p=ext.ptr(); p ; p=sString::next00(p) ){
                   full_path.printf(0,"%s%s",filename,p);
                   if (allowGz) {
                        full_path.printf("%s",".gz");
                        if (sFile::exists(full_path.ptr())) {
                            m_gz = gzopen(full_path.ptr(), "rb");
                            if (strncmp(p,".fastq",6)==0) isFastq=true;
                            break;
                        }
                   }
                };
                return this;
            }

            typedef idx (*processorCallbackType)(void * param, const char * id, idx idNum, idx curLength, const char * seq, const char * qualities, idx totalLength);
            
            idx processFastqGzRaw(processorCallbackType procCallback,void * param);

            idx processGzRaw(processorCallbackType procCallback,void * param,  idx chunkSize=80);

            void processRaw(processorCallbackType procCallback,void * param,  idx chunkSize=80){
                if (m_file) {
                }
                if (m_gz) {
                    if (isFastq) processFastqGzRaw(procCallback, param);
                    else processGzRaw(procCallback, param,  chunkSize);
                }
            };


    };

}

#endif 
