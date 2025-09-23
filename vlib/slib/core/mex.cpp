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

#include <slib/core/mex.hpp>
#include <slib/core/os.hpp>

using namespace slib;

#ifdef SLIB_WIN
    #include <windows.h>
    #include <Psapi.h>
#else
    #include <sys/mman.h>
    #include <sys/types.h>
#endif
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>


const idx sMex::_zero=0;
const idx sMex::_aligner[]={1,sizeof(idx),16,sSizePage};
const idx sMex::_extnder[]={1024,128,sSizePage,1024};
const char * sMex::_hexCode="0123456789ABCDEF";
const idx sMex::_defaultFlags=sMex::fBlockDoubling;
idx sMex::_totMexSize=0;
idx sMex::newSize = 0;
sMex::UriCallback sMex::uri_callback = 0;

sMex * sMex::init(const char * flnm , idx flg, idx offset, idx sizemap)
{
    flags|=flg;

    char pathbuf[PATH_MAX];
    if( uri_callback ) {
        flnm = uri_callback(pathbuf, PATH_MAX, flnm, flags & fReadonly);
    }

    if(!flnm)return this;

    if( (flags&fMapRemoveFile) && !(flags&fReadonly))
        ::remove(flnm);

    idx hFile;
    #ifdef WIN32
        HANDLE hf;
        if(flags&fReadonly) hf=CreateFile( flnm, GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING ,FILE_ATTRIBUTE_NORMAL,0);
        else hf= CreateFile( flnm, GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
        if(hf==INVALID_HANDLE_VALUE)return this;
        _curSize=GetFileSize(hf,0);
        hFile = _open_osfhandle((intptr_t)hf, flags&fReadonly ? O_RDONLY : (O_RDWR|O_CREAT) );
    #else
        int open_flags = O_RDWR | O_CREAT;
        if( flags & fReadonly ) {
            open_flags = O_RDONLY;
        } else if( flags & fCreatExcl ) {
            open_flags = O_RDWR | O_CREAT | O_EXCL;
        }
        hFile = open(flnm, open_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if( hFile < 1 ) {
            return this;
        }
        _curSize=lseek(hFile, 0, SEEK_END);
    #endif

    alloc(0, _buf);

    flags|=(hFile<<(sMex_FileHandleShift));
    if( sizemap ) {
        _curSize=sizemap;
        flags|=fPartialMap;
        _buf=alloc(_curSize,0,offset,sizemap);if(!_buf)_curSize=0;
        _curPos=_curSize;
    }
    else if(_curSize){
        _buf=alloc(_curSize,0);if(!_buf)_curSize=0;
        _curPos=_curSize;
    }
    return this;
}

void sMex::destroy(void)
{
    idx hFile=(flags>>(sMex_FileHandleShift)) & 0xFFFF;
    alloc(0,_buf);
    if(hFile>0) {
        if(_curSize>0 && !(flags&fPartialMap)) {
            if((flags&fMaintainAlignment))_curPos=sAlign(_curPos, _aligner[(flags&0x03)]);
            chsize((int)hFile,(long)_curPos);
        }
        else {
        }
        close((int)hFile);
        flags&=0x0000FFFF;
    }
    _buf=0;_curPos=_curSize=0;

}

idx sMex::mtime(void) const
{
    idx hFile = (flags>>(sMex_FileHandleShift)) & 0xFFFF;
    if( hFile > 0 ) {
        struct stat st;
        sSet(&st, 0);
        return fstat((int)hFile, &st) == -1 ? sIdxMax : (idx) st.st_mtime;
    }
    return sIdxMax;
}


idx sMex::replace(idx pozReplace,const void * add,idx sizeAdd,idx sizeDel)
{

    if( (flags&fDirectFileStream)  ) return 0;
    if(flags&fReadonly)return 0;
    idx sizeRequired;
    void *  newBfr;

    sizeRequired=_curPos+sizeAdd-sizeDel;
    if(sizeRequired==0){
        alloc(sizeRequired, _buf);
        _buf=0;_curPos=_curSize=0;
        return 0;
    }

    if(sizeRequired > _curSize ) {
        if(_buf && (flags&fNoRealloc))return sNotIdx;

        sizeRequired=sAlign(sizeRequired, _aligner[(flags&0x03)]);
        sizeRequired=sMax(sizeRequired,_extnder[((flags>>2)&0x03)]);

        newSize = 0;
        if((flags&fExactSize))newSize=sizeRequired;
        else if( (flags&fBlockDoubling)!=fBlockDoubling ) newSize=sAlign(sizeRequired,_extnder[((flags>>2)&0x03)]);
        else for(newSize=_curSize ? _curSize : sizeof(idx) ; newSize<sizeRequired; newSize<<=1);

        newBfr=alloc(newSize, _buf);
        if( (newBfr)==0 ) return sNotIdx;
        _buf=newBfr;
        _curSize=newSize;
        newSize = 0;

    } else newBfr=_buf;


    if( _curPos-pozReplace-sizeDel > 0 ) memmove( ((char *)newBfr)+pozReplace+sizeAdd , ((char *)_buf)+pozReplace+sizeDel , _curPos-pozReplace-sizeDel );

    if(sizeAdd)    {
        if(add) memmove( ((char *)_buf)+pozReplace , add , sizeAdd );
        else if((flags&fSetZero)) {
             #ifdef SLIB64
                char * ps=(((char *)_buf)+pozReplace);
                idx bb= ((idx)ps)&0x7 ;
                idx * ps64=(idx*)(bb ? (ps+8-bb) : ps) ;
                char * pe=(((char *)_buf)+pozReplace+sizeAdd);
                idx ba=((idx)pe)&0x7 ;
                idx* pe64=(idx*)(ba && ps+ba<=pe ? (pe-ba) : pe);

                for(;ps<(char * )ps64;++ps)
                    *ps=0;
                for(;ps64<pe64;++ps64)
                    *ps64=0;
                for(ps=(char*)pe64;ps<pe;++ps)
                    *ps=0;

            #else
                memset( ((char *)_buf)+pozReplace , 0 , sizeAdd );
            #endif

        }
    }


    _curPos+=sizeAdd-sizeDel;
    _curPos=sAlign(_curPos,_aligner[(flags&0x03)]);

    return pozReplace;
}

void * sMex::remap( idx setflags, idx unsetflags, bool onlyIfFlagsChanged , idx offset, idx sizemap)
{
    static const idx flagmask=0xFFFF;
    idx hFile = (flags>>(sMex_FileHandleShift)) & 0xFFFF;

    if( flags & fReadonly && unsetflags & fReadonly && hFile ) {
        return 0;
    }

    idx oldFlags=flags;
    flags=(flags | (setflags & flagmask)) & ~(unsetflags & flagmask);
    flags |= fForceRemapTruncate;
    if( flags == oldFlags && onlyIfFlagsChanged ) {
        return ptr();
    }
    _buf=sMex::alloc(pos(), ptr(),offset, sizemap);
    _curSize=pos();
    flags &= ~fForceRemapTruncate;
    return _buf;
}

void * sMex::alloc(idx size, void * ptr, idx offset, idx sizemap)
{
    idx hFile=(flags>>(sMex_FileHandleShift)) & 0xFFFF;
    if(hFile==0 ){
        return sNew(size, ptr);
    }else if ( flags&fMapMemoryLazyFile ){
        if(size==0 && _curSize) {
            lseek(hFile,0,SEEK_SET);
            write(hFile,_buf,_curSize);
            ftruncate(hFile,_curSize);
            sDel(ptr);
        }else {
            void * b=sNew(size,ptr);
            if(_buf==0 && size) {
                lseek(hFile,0,SEEK_SET);
                read(hFile,b,size);
            }
            return b;
        }
    }


    if(ptr) {
        #ifdef WIN32
            UnmapViewOfFile( ptr) ;
        #else
            munmap( ptr, _curSize);
        #endif
            ptr=0;
    }

    if(size>0) {
        #ifdef WIN32
            HANDLE hMapObject = CreateFileMapping( (void*) _get_osfhandle((int)hFile), NULL,flags&fReadonly ? PAGE_READONLY : PAGE_READWRITE,0,(DWORD)size,0);
            if ( hMapObject != 0 ){
                ptr = MapViewOfFile( hMapObject,flags&fReadonly ? FILE_MAP_READ : FILE_MAP_WRITE,(offset>>32)&0xFFFFFFFF,(offset&0xFFFFFFFF),offset ? size : 0 );
                CloseHandle(hMapObject);
            }
        #else
            if(size && (!offset && !sizemap) && ((flags&fReadonly)==0 || (flags&fForceRemapTruncate) ) )
                ftruncate(hFile, size);
            idx mapFlags=MAP_SHARED;
            if(flags&fMapPreloadPages)
                mapFlags|=MAP_POPULATE;

            ptr = mmap(0, size, flags&fReadonly ? PROT_READ  : (PROT_READ | PROT_WRITE), mapFlags, hFile, offset);

            if(ptr ==(caddr_t) -1) ptr=0;
        #endif
    }


    return ptr;
}



idx sMex::readIO(FILE * fp, const char * endCharList)
{
    char buf[sSizePage+1];

    idx lpos=pos();

    while(!feof(fp) ) {
        idx len=(idx )fread(buf,1,sizeof(buf)-1,fp);
        if(endCharList) {
            buf[len]=0;
            const char * fnd=strpbrk(buf,endCharList);
            if(fnd ) {
                idx l=fnd-buf;
                if( l!=len)
                    fseek(fp, l-len+1, SEEK_CUR);
                len=l;
                add(buf,len);
                return pos()-lpos;
            }

        }
        add(buf,len);
    }

    return pos()-lpos;
}


idx sMex::readIO(int fh, const char * endCharList)
{
    char buf[sSizeBlock+1];

    idx lpos=pos();
    idx len;

    while( (len=(idx )read(fh,buf,sizeof(buf)-1))!=EOF  ) {
        if(endCharList) {
            buf[len]=0;
            const char * fnd=strpbrk(buf,endCharList);
            if(fnd ) {
                idx l=fnd-buf;
                if( l!=len)
                    lseek(fh, l-len+1, SEEK_CUR);
                len=l;
                add(buf,len);
                return pos()-lpos;
            }

        }
        add(buf,len);
    }

    return pos()-lpos;
}


