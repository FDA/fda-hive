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
#ifndef Vstd_dll_hpp
#define Vstd_dll_hpp

#include <slib/core/dic.hpp>

namespace slib { 

#ifdef WIN32
    #define sDll_FlnmExtension ".dll"
    #include <windows.h>
#else
    #include <dlfcn.h>
    #define sDll_FlnmExtension ".so"
#endif
    
    

    class sDll
    {
    public:
        typedef void * Handle;
        typedef void * (* Proc)( ... );
    
    private:
        Handle hDll;
        sDic < Handle > dynaLibs;
       
    
#ifdef WIN32
        Handle dllopen(const char * flnm) {return LoadLibrary(flnm);}
        void dllclose(Handle hdl){ FreeLibrary((HMODULE)hdl);}
        Proc dllproc(Handle hdl, const char * procname){return (Proc)GetProcAddress((HMODULE)hdl,procname);}
#else 
        Handle dllopen(const char * flnm) {return dlopen(flnm,RTLD_LAZY);}
        void dllclose(Handle hdl){ dlclose(hdl);}
        Proc dllproc(Handle hdl, const char * procname){return (Proc)dlsym(hdl,procname);}
#endif

    public:
        sDll () {}
        ~sDll () {
            Handle hDll;
            for ( idx i=0; i< dynaLibs.dim(); ++ i ) {
                if( (hDll = * dynaLibs.ptr(i)) !=0) 
                    dllclose(hDll);
            }
        }
        Proc loadProc (const char * dll, const char * proc) ;
    public:

    };      



}
#endif



