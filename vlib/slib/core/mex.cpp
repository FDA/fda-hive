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
    //#include <unistd.h>
    #include <sys/mman.h>
    //#include <asm-generic/mman.h>
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
    //return this;
    //printf("ERRRRR\n");exit(1);
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
        //DWORD l=GetLastError();
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
        hFile = open(flnm, open_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // S_IREAD|S_IWRITE
        if( hFile < 1 ) {
            return this;
        }
        _curSize=lseek(hFile, 0, SEEK_END);
    #endif

    alloc(0, _buf);

    flags|=(hFile<<(sMex_FileHandleShift));
    if( sizemap ) { // partial mapping
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
    idx hFile=(flags>>(sMex_FileHandleShift)) & 0xFFFF; // this is where file handles are kept
    /*
    #ifdef WIN32
        TCHAR pszFilename[MAX_PATH+1];
        if(hFile && _curSize==0){
            _buf=alloc(1024,_buf);
            if (!GetMappedFileName (GetCurrentProcess(), _buf, pszFilename,MAX_PATH)) pszFilename[0]=0;
        }
    #endif
    */
    alloc(0,_buf);
    if(hFile>0) {
        if(_curSize>0 && !(flags&fPartialMap)) {
            if((flags&fMaintainAlignment))_curPos=sAlign(_curPos, _aligner[(flags&0x03)]);
            chsize((int)hFile,(long)_curPos);
        }
        else {
            //if(pszFilename[0])::remove(pszFilename);
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

    //if( (flags&fDirectFileStream) || isFlag(flags,fLockedForChange) ) return 0;
    if( (flags&fDirectFileStream)  ) return 0;
    if(flags&fReadonly)return 0;
    idx sizeRequired;
    void *  newBfr;

    // compute the reuqired size
    sizeRequired=_curPos+sizeAdd-sizeDel;
    if(sizeRequired==0){ // emptying ? nothing to add and remove all ?
        alloc(sizeRequired, _buf); // to zero size
        _buf=0;_curPos=_curSize=0;
        return 0; // completely reasonable (!=-1) answer
    }

    // if the size needed is more than the buffer size already allocated : reallocate
    if(sizeRequired > _curSize ) {
        // check if this is noReallocatable vMEX
        if(_buf && (flags&fNoRealloc))return sNotIdx;

        // fix the requirement size by dataalignment and block size
        sizeRequired=sAlign(sizeRequired, _aligner[(flags&0x03)]); // size must be aligned
        sizeRequired=sMax(sizeRequired,_extnder[((flags>>2)&0x03)]); // if the size cannot be smaller than the required blocksize

        // determine the new size : it is supposed to be bigger than the sizeRequired
        // because we use quantized memory allocation strategy: we always get more in advance
        newSize = 0;
        if((flags&fExactSize))newSize=sizeRequired; // unless if exact size is specified: in this case we just allocate that
        else if( (flags&fBlockDoubling)!=fBlockDoubling ) newSize=sAlign(sizeRequired,_extnder[((flags>>2)&0x03)]); // if not required to double the size  we allocate by aligning the size to blocks
        else for(newSize=_curSize ? _curSize : sizeof(idx) ; newSize<sizeRequired; newSize<<=1); // size doubling is required

        // reallocate the buffer
        newBfr=alloc(newSize, _buf);
        if( (newBfr)==0 ) return sNotIdx; // allocation failure: the MEX still is valid but the last operation not performed
        _buf=newBfr;
        _curSize=newSize;
        newSize = 0;

    } else newBfr=_buf; // buffer is big enough but still some moves might be necessary


    if( _curPos-pozReplace-sizeDel > 0 ) memmove( ((char *)newBfr)+pozReplace+sizeAdd , ((char *)_buf)+pozReplace+sizeDel , _curPos-pozReplace-sizeDel );// move the content after insertion pozition

    // if new data were added
    if(sizeAdd)    {
        if(add) memmove( ((char *)_buf)+pozReplace , add , sizeAdd ); // copy to the space allocated for it
        //else if((flags&fSetZero)) memset( ((char *)_buf)+pozReplace , 0 , sizeAdd );  // initialize to zero if needed
        else if((flags&fSetZero)) {
             #ifdef SLIB64
                char * ps=(((char *)_buf)+pozReplace);
                idx bb= ((idx)ps)&0x7 ; // bytes before 64 bit
                idx * ps64=(idx*)(bb ? (ps+8-bb) : ps) ; // where we start 64 bit copy
                char * pe=(((char *)_buf)+pozReplace+sizeAdd);
                idx ba=((idx)pe)&0x7 ; // bytes after 64 bit
                idx* pe64=(idx*)(ba && ps+ba<=pe ? (pe-ba) : pe);

                for(;ps<(char * )ps64;++ps)
                    *ps=0;
                for(;ps64<pe64;++ps64)
                    *ps64=0;
                for(ps=(char*)pe64;ps<pe;++ps)
                    *ps=0;

            #else
                memset( ((char *)_buf)+pozReplace , 0 , sizeAdd );  // initialize to zero if needed
            #endif

        }
    }


    // adjust the position of the used top
    _curPos+=sizeAdd-sizeDel;
    _curPos=sAlign(_curPos,_aligner[(flags&0x03)]);// take care of required alignment

    // return the position where we added the last piece
    return pozReplace;
}

void * sMex::remap( idx setflags, idx unsetflags, bool onlyIfFlagsChanged , idx offset, idx sizemap)
{
    static const idx flagmask=0xFFFF; // upper bits reserved for file handles etc.
    idx hFile = (flags>>(sMex_FileHandleShift)) & 0xFFFF; // this is where file handles are kept

    // Linux doesn't allow readonly filehandles to be reopened or mapped as readwrite
    if( flags & fReadonly && unsetflags & fReadonly && hFile ) {
        // fprintf(stderr, "%s:%u: ERROR: cannot remap readonly file as read-write\n", __FILE__, __LINE__);
        // NO printing to stderr from low level libs ... no stderr .. .forget about
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
    idx hFile=(flags>>(sMex_FileHandleShift)) & 0xFFFF; // this is where file handles are kept
    if(hFile==0 ){
        return sNew(size, ptr);  // no file handles means in memory operations
    }else if ( flags&fMapMemoryLazyFile ){
        if(size==0 && _curSize) { // destorying the container , need to write memory representation into file
            lseek(hFile,0,SEEK_SET);
            write(hFile,_buf,_curSize);
            ftruncate(hFile,_curSize);
            sDel(ptr);
        }else { // reading first time , need to read from existing file
            void * b=sNew(size,ptr);
            if(_buf==0 && size) { // first time reading this, read it into memory
                lseek(hFile,0,SEEK_SET);
                read(hFile,b,size);
            }
            return b;
        }
    }


    if(ptr) { // reallocation ?
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
                //idx err=GetLastError();
                CloseHandle(hMapObject);
            }
        #else
            //lseek(hFile, size, SEEK_SET);write(hFile,"\0",1);
            if(size && (!offset && !sizemap) && ((flags&fReadonly)==0 || (flags&fForceRemapTruncate) ) )
                ftruncate(hFile, size);
            idx mapFlags=MAP_SHARED;
            //if(size>1024&1024 )
            //    mapFlags|=MAP_HUGETLB;
            //mapFlags|=MAP_NONBLOCK;
            //mapFlags|=MAP_UNINITIALIZED;
            if(flags&fMapPreloadPages)
                mapFlags|=MAP_POPULATE;

            ptr = mmap(0, size, flags&fReadonly ? PROT_READ  : (PROT_READ | PROT_WRITE), mapFlags, hFile, offset);
            // ptr = mmap(0, size, flags&fReadonly ? PROT_READ|PROT_NORESERVE  : (PROT_READ | PROT_WRITE), MAP_SHARED, hFile, 0);

            if(ptr ==(caddr_t) -1) ptr=0;
        #endif
    }

    //unsigned long rsize=lseek(hFile, 0, SEEK_END);
    //if(rsize!=siz)chsize(hFile,siz);

    return ptr;
}



idx sMex::readIO(FILE * fp, const char * endCharList)
{
    char buf[sSizePage+1];

    idx lpos=pos();

    while(!feof(fp) ) { // we stream it with the stream-accumulator, not to consume too much memory
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

    while( (len=(idx )read(fh,buf,sizeof(buf)-1))!=EOF  ) { // we stream it with the stream-accumulator, not to consume too much memory
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


