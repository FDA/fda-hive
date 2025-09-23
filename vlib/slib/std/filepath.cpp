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
#ifdef WIN32
    #include <direct.h>
#else
#endif
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

#include <slib/std/file.hpp>
#include <slib/std/string.hpp>

using namespace slib;

char * sFilePath::vmakeName(idx cutlen, const char * tmplt, const char * formatDescriptor, va_list ap)
{
    if( !tmplt ) {
        return 0;
    }
    sStr tbuf, fmtAfterVsPrintf;
    const char * lastSlash=strrchr(tmplt,'/');if(!lastSlash)lastSlash=strrchr(tmplt,'\\');
    const char * lastDot=strrchr(lastSlash ? lastSlash : tmplt,'.');
    idx len=sLen(tmplt),llen;
    #define mIs(_v_type)    !strncmp(fmt+is,"%" _v_type,(llen=sLen("%" _v_type)))

    idx is=0;
    struct stat fst;sSet(&fst);

    const char * fmt=fmtAfterVsPrintf.vprintf(formatDescriptor,ap);

    if(cutlen!=sIdxMax)
        sStr::cut(cutlen);
    for( is=0; fmt[is];){
        if(fmt[is]=='%'){
            idx skip=0;
            const char * first=tmplt;
            const char * last=tmplt+len;

            if( mIs("st_") && fst.st_atime==0)  {
                if( tmplt[len-1]=='/' || tmplt[len-1]=='\\' ) {
                    tbuf.add(tmplt,len-1); tbuf.add0();
                    stat(tbuf,&fst);
                }
                else stat(tmplt,&fst);
            }
            tbuf.cut(0);

            llen=0;
            if(funcall)
                llen = funcall(tbuf,fmt+is,tmplt);
            if(llen) {

            } else if( mIs("pathx") )
                {if(lastDot)last=lastDot;}
            else if( mIs("path") )
                {last=tmplt+len;}
            else if( mIs("dirx") )
                {
                if(lastSlash){
                    for(first=lastSlash-1; first>tmplt && !strchr("/\\",*(first-1)); --first) ;
                    last=lastSlash+1;
                }
            }
            else if( mIs("dir") )
                {last=lastSlash ? lastSlash : first;}
            else if( mIs("flnmx") )
                {if(lastSlash)first=lastSlash+1;if(lastDot)last=lastDot; }
            else if( mIs("flnm") )
                {if(lastSlash)first=lastSlash+1;}
            else if( mIs("ext") )
                {first=lastDot ? lastDot+1 : last;}
            else if( mIs("st_size") ) {
                if(fst.st_mode&S_IFDIR){sprintf(tbuf,"/");}
                else {sprintf(tbuf,"%" DEC,(idx)(fst.st_size) );}
            }
            else if( mIs("st_atime") )
                {sprintf(tbuf,"%s",ctime(&fst.st_atime));}
            else if( mIs("st_ctime") )
                {sprintf(tbuf,"%s",ctime(&fst.st_ctime));}
            else if( mIs("st_mtime") )
                {sprintf(tbuf,"%s",ctime(&fst.st_mtime));}
            else if( mIs("st_dev") )
                {sprintf(tbuf,"%c",(int)('A'+fst.st_rdev));}
            else if( mIs("st_attrib") )
                {sprintf(tbuf,"%x",fst.st_mode);}

            else skip=1;

            if(tbuf.length()){
                sString::copyUntil(this, tbuf.ptr(), tbuf.length(),sString_symbolsEndline);
                is+=llen;
            }
            else if(!skip) {
                is+=llen;
                if(last!=first)
                    add(first,(idx)(last-first));
            }
            else ++is;
            continue;
        }
        add(&fmt[is++],1);
    }

    add0();
    return ptr();
}


char * sFilePath::simplifyPath(eSimplifyPath spacehandle, const char * srcBuf)
{
    if(!srcBuf) srcBuf=ptr();
    else resize(sLen(srcBuf)+1);
    const char * src;
    char * dstBuf=ptr(), * prv=dstBuf, * lastNew=dstBuf, *dst=dstBuf;

    for( src=srcBuf; *src; ++src) {

        if(*(src)==':' && src>srcBuf ){
            lastNew=prv;
        }

        if( strchr( "/\\",*src) && *(src+1) && strchr( "/\\",*(src+1)) )
            continue;

        if( *src == '.' && (*(src + 1) == 0 || strchr("/\\", *(src + 1))) && (src == srcBuf || strchr("/\\", *(src - 1))) ) {
            for(; src[1] == '/' || src[1] == '\\'; src++);
            continue;
        }

        if( *src == '.' && *(src + 1) == '.' && (!*(src + 2) || strchr("/\\", *(src + 2))) && (src > srcBuf && strchr("/\\", *(src - 1))) ) {
            if( src[2] ) {
                src += 2;
            } else {
                src++;
            }
            for(dst--; dst > dstBuf && dst[-1] != '/'; dst--);
            if( dst < dstBuf ) {
                dst = dstBuf;
            }
            continue;
        }

        if(spacehandle==eSimplifySpaceRemove && strchr(sString_symbolsBlank,*src) )
            continue;

        *dst=*src;
        if(*dst=='/' || *dst=='\\') {
            prv=dst;
            *dst='/';
        }
        else if(spacehandle==eSimplifySpaceReplaceWithUnderscore && strchr(sString_symbolsBlank,*dst) )
            *dst='_';

        ++dst;
    }
    *dst=0;
    if(lastNew>dstBuf)
        strcpy(dstBuf,lastNew+1);
    cut(sLen(dstBuf)+1);
    return dstBuf;
}

char * sFilePath::composeWinStyleFileListPaths(const char * flnmlst, bool iszerolist)
{
    char * ptr, *first;
    for( ptr=first=sString::next00(flnmlst,1); ptr ; ptr=sString::next00(ptr,1) )
        sStr::printf( "%s%s/%s", ptr==first ? "" : " " , flnmlst, ptr );
    if(!first)sStr::printf( "%s", flnmlst);

    add(__,2);
    char * pp=sStr::ptr();
    for(char * p=pp; *p; ++p) {
            if(*p=='\\' ) *p='/';
            else if(iszerolist && *p==' ') *p=0;
    }
    return pp;
}


char * sFilePath::curDir(void)
{
    char dir[4096*16];
    if( ::getcwd(dir,sizeof(dir)-1) ) {
        sStr::add(dir,sLen(dir));
        sStr::printf("/");
    } else {
        sStr::cut0cut();
    }
    return sStr::ptr();
}
#ifdef WIN32
    #include "windows.h"
#endif

char * sFilePath::processPath(sFile::pid pid, bool justdir)
{
    sStr::cut(0);
    char * reslt=sStr::resize(PATH_MAX + 1);
    reslt[0]=0;

    #ifdef WIN32
        GetModuleFileName((HMODULE)(pid+((HMODULE)0)),reslt, sSizeMax);
        sString::searchAndReplaceSymbols(reslt,0,"\\", "/",0,1,0,0);
    #else
        char buf[128];
        if( !pid )
            pid = getpid();
        sprintf(buf, "/proc/%" DEC "/exe", pid);
        reslt[sMax<idx>(0, readlink(buf, reslt, sSizeMax))] = 0;
    #endif

    if(justdir) {
        char * p ;
        if( (p=strrchr(reslt,'/'))!=0 || (p=strrchr(reslt,'\\'))!=0 )
            *p=0;
    }
    return reslt;
}


char * sFilePath::procNameFromCmdLine(const char * cmdline )
{
    const char * spc=strpbrk(cmdline,sString_symbolsBlank), * l, * slash=cmdline;
    if(!spc)spc=cmdline+sLen(cmdline);
    for(l=cmdline; l<spc; ++l){
        if(*(l)=='/' || *(l)=='\\') slash=l+1;
    }

    sStr::add(slash,spc-slash);sStr::add0();
    return sStr::ptr();
}

