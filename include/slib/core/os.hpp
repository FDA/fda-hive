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
#ifndef sLib_core_os_h
#define sLib_core_os_h

#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_extension
#define __has_extension __has_feature
#endif

#ifdef WIN32

        #define popen _popen
        #define pclose _pclose

        #define chsize    _chsize
        #define open    _open
        #define lseek    _lseek
        #define    close    _close
        #define    read    _read
        #define    rmdir    _rmdir

        #include <conio.h>
        #define    getch    _getch

        #define    mkdir    _mkdir
        #define    getcwd    _getcwd

        #include <process.h>
        #define getpid _getpid
        #define gettid _gettid

        #define strcasecmp stricmp
        #define strncasecmp strnicmp

        #define ioControl(v_var1,v_var2,v_var3)        ioctlsocket((v_var1),(v_var2),(unsigned long *)(v_var3))
        #define sleepSeconds(v_var)     Sleep((v_var)*1000)
        #define sleepMS(v_var)            Sleep((DWORD)(v_var))
        #include <io.h>
        #define ioModeBin(v_stream)    _setmode(fileno(v_stream),_O_BINARY);
        #define O_TEMPORARY _O_TEMPORARY
        #define threadBegin(v_func,v_par,v_err)   (v_err)=_beginthread((v_func),0,(v_par))

        #define S_IRUSR S_IREAD
        #define S_IWUSR S_IWRITE
        #define S_IXUSR 0
        #define S_IRGRP S_IREAD
        #define S_IWGRP S_IWRITE
        #define S_IXGRP 0
        #define S_IROTH S_IREAD
        #define S_IWOTH S_IWRITE
        #define S_IXOTH 0
    #else
        #undef S_IREAD
        #undef S_IWRITE
        #undef S_IEXEC
            #define S_IREAD    (S_IRUSR|S_IRGRP|S_IROTH)
            #define S_IWRITE (S_IWUSR|S_IWGRP)
            #define S_IEXEC (S_IXUSR|S_IXGRP|S_IXOTH)

        #define chsize ftruncate

        #define ioControl(v_var1,v_var2,v_var3)        ioctl((v_var1),(v_var2),(v_var3))
        #define sleepSeconds(v_var)     sleep((v_var))
        #define sleepMS(v_var)            usleep((v_var)*1000)
        #define ioModeBin(v_stream)    {}


        #include <curses.h>
        #include <unistd.h>
        #include <pthread.h>

        #define threadBegin(v_func,v_par,v_err)   {pthread_t tid;(v_err)=pthread_create(&tid,NULL,(void* (*)(void*))(v_func),(void *)(v_par));}
    #endif

    #define S_READWRITE (S_IWRITE|S_IREAD)


#ifdef WIN32
    #if defined(_M_X64) || defined(__amd64__)
        #define SLIB64 1
    #endif
#else
    #if defined(__LP64__) || defined(_LP64)
        #define SLIB64 1
    #endif
#endif

#ifdef __APPLE__
    #define SLIB_PLATFORM    "Mac"
    #define SLIB_MAC
#elif WIN32
    #define SLIB_PLATFORM    "Win"
    #define SLIB_WIN
#else
    #define SLIB_PLATFORM    "Linux"
    #define SLIB_LINUX
#endif


namespace slib
{
    #ifndef va_copy
        #define va_copy(dest, src) (dest) = (src)
    #endif

    #define sSizePage           (4096)
    #define sSizeBlock          (512)

}
#endif 

    #ifdef WIN32
        #ifdef SLIB64
        #endif
    #else
    #endif


