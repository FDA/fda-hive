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
#ifndef sLib_bioproject_hpp
#define sLib_bioporject_hpp

#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>

class bioproject
{
    private:
        const char * getonelinefromBuffer(char * buf, const char * fileContent);
        void ParseOneline(idx idCnt, const char * buf, sStr * string, idx * id, ...);

    public:
        sVioDB DB;    
    public: 
       bioproject(const char * InputFilename=0)
        {
            init(InputFilename);
        }

        bioproject * init (const char * InputFilename=0)
        {
            if(InputFilename) DB.init(InputFilename,"bioproject",0,0);
            return this;
        }
        bool isok(void)
        {
            return DB.isok("bioproject")? true : false;
        }
        idx getTaxidByProjectID(idx projectid=0);
        idx getProjectIDByTaxID(idx taxid=0);
        void Parsefile(const char* src, const char* inputfilename, bool combineFile=true);
        
}; 

#endif
