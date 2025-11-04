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
#ifndef sLib_std_streamcgi_hpp
#define sLib_std_streamcgi_hpp

#include <slib/core/var.hpp>

namespace slib {


    class sStreamCGI
    {
        public:
            bool dataIsOpen;
            idx gDebug;
            idx streamBufSize;
            sVar vars,* pForm;

            const char * read_var;
            idx read_var_len;
            bool loadMode,verbose;

            sStreamCGI(void)
            {
                streamBufSize=sSizePage*1;
                dataIsOpen=false;
                read_var=0;
                read_var_len=0;
                gDebug=0;
                loadMode=false;
                verbose=false;
                pForm=&vars;

                stream_fread=fread;
                stream_feof=feof;
            }
            idx parseInput(int argc, const char * argv[], const char *envp[], FILE * fp=0);

            virtual idx dataClose(void)
                {return 0;}
            virtual idx dataOpen(const char * content_type, idx content_type_length,const char * input_name,idx input_name_length,const char * input_filename,idx input_filename_length)
                {return 0;}
            virtual idx dataStream(const char * content, idx content_length)
                {return 0;}
            virtual idx streamFinished(void)
                {return 0;}
            virtual idx Cmd(const char * csmd)
                {return 0;}

            void copyMetadata(sVar * meta, const char * input_name, idx input_name_length) ;


            size_t ( * stream_fread) (void * buf, size_t sz, size_t cnt, FILE * fp );
            int ( * stream_feof) (FILE * fp );
    };

}
#endif 