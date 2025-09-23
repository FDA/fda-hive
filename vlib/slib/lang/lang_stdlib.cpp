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

#include <slib/std/file.hpp>
#include <slib/std/online.hpp>
#include <slib/lang.hpp>

#include <fcntl.h>

using namespace slib;

#define V(_v_nam)           (lg->curScope->getVar((char *)(_v_nam)))
#define VI(_v_nam)          (lg->charint((char *)(_v_nam)))
#define Vlogic(_v_nam)      ((lg->charint((char *)(_v_nam)))? true : false)
#define Vcase               (lg->charcase())
#define G(_v_nam)           (lg->globalScope.getVar(_v_nam))
#define GI(_v_nam)          (lg->charint((_v_nam),&lg->globalScope))
#define VcaseI                (lg->charcase()==sString::eCaseNone ? true : false)

#ifdef WIN32
    #pragma warning (disable: 4100 )
#endif




const char * sLang::stdlibDeclarations = 
" \n"
"app.verbose=1;"
"app.printf( fmt, ...  ){} "
"app.env( nam) {} "

"string.endline=\"\n\";"
"string.blank=\" \t\r\n\";"
"string.space=\" \t\";"
"string.case=0;"
"string.ismatch=1;"
"string.tabs=0;"
"string.len( src ) {}"
"string.cat( src1, src2 ) {}"
"string.cmp( src1, src2 ){}"
"string.cnt( src, separ ){}"
"string.skip( src, num , separ ){}"
"string.extract( src, num , separ , case , isquotes ){}"
"string.compareuntil( src1, src2 , until , case ){}"
"string.search( src, find, occurence, stop, case ){}"
"string.replacesymb( src, find, replace, maxtags , ismatch , isskipmult , isquotes ){}" 
"string.cntsymb( src, separ ){}"
"string.replacestr( src, find, replace, maxtags , case ){}" 
"string.crlf(src, isunix ){}"
"string.cstyle(src){}"
"string.cleanmarkup( src, start, end, replace, maxtags , isinside, case ){}" 
"string.cleanends( src, find, ismatch ){}" 
"string.hungarian( src, isname, isnointblanks, case ){}" 
"string.changecase( src, case ){}" 
"string.printf( fmt, ...  ){} "
"string.unescape( src){} "
"string.enumerate ( callback, separ, fmt, ...  ) {} "

"file.exists( flnm ) {}"
"file.open( flnm ) {}"
"file.close( handle ) {}"
"file.len( flnmhandle ) {}"

"file.getpos( handle ) {}"
"file.setpos( handle , pos ) {}"
"file.gets( handle , endline ) {}"
"file.read ( handle, size ) {}"
"file.write( handle, content, size ) {}"

"file.remove( flnm ) {}"
"file.rename( flnm1 , flnm2 ) {}"
"file.content( flnm ) {}"
"file.printf(flnm, fmt, ... ){}"
"file.find( flnm , separ, maxfind, isrecursive, issubdir, isdirs, isfiles  ) {}"
"file.timestamp( flnm ) {}"
"file.makename( flnm , fmt, ... ) {}"
"file.makedir( flnm, isfile ){}"
"file.rmdir( flnm, isfile ){}"
"file.curdir( ){}"

"online.httpget( url, data ){}"
"online.httpbatch( callback, urlbase, dirbase){string.enumerate( callback , \"\r\n\", string.printf(\"file.printf('%s',online.httpget('%s'))\", dirbase,urlbase) );}"

"make.listincludes ( incdir, flnm , maxlen , incstart, incend, incsymbs, isnorepeat, isrecursive) {} "
"make.isnew( timestamp, flnm ) {}"
"make.getmarker ( var ) {}"
"make.setmarker ( var , val ) {}"



"dic.set( dicnm, name, item ) {} "
"dic.get( dicnm, name ) {} "



"";






idx sLang::eval_app_printf(sLang * lg, idx il)
{
    if( GI("app.verbose")==0 )return 0;

    eval_string_printf(lg,il);
    idx tabs=GI("string.tabs");
    for(idx i=0; i<tabs; ++i)lg->msg->printf("  ");
    lg->msg->printf("%s",lg->curScope->getVar("__ret"));
    fflush(0);
    return 0;
}

idx sLang::eval_app_env(sLang * lg, idx il)
{
    const char * env=V("nam");
    env= env ? getenv( env) : 0 ;
    lg->curScope->setVar("__ret","%s", env ? env : "");
    return 0;
}


idx sLang::eval_dic_get (sLang * lg, idx il)
{
    const char * dicnm=V("dicnm");
    const char * nam=V("name");

    idx * itOfs=0;
    sDic < idx > * ppDic=lg->dicDic.get(dicnm);
    if(ppDic)itOfs=ppDic->get(nam);
    
    lg->curScope->setVar("__ret","%s",itOfs ? lg->dicDicData.ptr(*itOfs) : "");
    return 0;
}

idx sLang::eval_dic_set(sLang * lg, idx il)
{
    const char * dicnm=V("dicnm");
    const char * nam=V("name");
    const char * item=V("item");
    
    sDic < idx > * ppDic=lg->dicDic.get(dicnm);
    if(!ppDic){
        ppDic=lg->dicDic.set(dicnm);
    }
    
    if(ppDic) { 
        lg->dicDicData.add(_,1);idx itOfs=lg->dicDicData.length();
        lg->dicDicData.printf("%s",item);lg->dicDicData.add(__,2);
        idx * pItem=ppDic->set(nam);
        if(pItem)*pItem=itOfs;
    }
    lg->curScope->setVar("__ret","%" DEC,ppDic ? ppDic->dim() : 0 );
    return 0;
}


idx sLang::eval_string_len(sLang * lg, idx il)
{
    lg->curScope->setVar("__ret","%" DEC,strlen(V("src")));
    return 0;
}

idx sLang::eval_string_cat(sLang * lg, idx il)
{
    lg->curScope->setVar("__ret","%s%s",V("src1"),V("src2"));
    return 0;
}

idx sLang::eval_string_cmp(sLang * lg, idx il)
{
    lg->curScope->setVar("__ret","%" DEC,strcmp(V("src1"),V("src2")) ? 1 : 0 );
    return 0;
}

idx sLang::eval_string_cnt(sLang * lg, idx il)
{
    const char * src=V("src");
    idx cnt=0;
    if(*src){
        const char * separ=V("separ");
        for ( cnt=0, --src; src ; ++cnt) src=sString::searchSubstring( src+1, 0, separ , 1, 0, 0 );
        --cnt;
    }
    lg->curScope->setVar("__ret","%" DEC,cnt);
    return 0;
}

idx sLang::eval_string_cntsymb(sLang * lg, idx il)
{
    const char * src=V("src");
    idx cnt=0, i;
    if(*src){
        const char * separ=V("separ");
        for( cnt=i=0; src[i]; ++i) { 
            if(strchr(separ,src[i])) ++cnt;
        }
    }
    lg->curScope->setVar("__ret","%" DEC,cnt);
    return 0;
}

idx sLang::eval_string_skip(sLang * lg, idx il)
{
    idx cnt, num=VI("num");
    const char * src=V("src");
    const char * separ = V("separ");
    for ( cnt=0; src && cnt< num; ++cnt) src=sString::searchSubstring( src, 0, separ , 1, 0, 0 );
    lg->curScope->setVar("__ret","%s",src);
    return 0;
}

idx sLang::eval_string_extract(sLang * lg, idx il)
{
    idx num=VI("num");
    const char * src=V("src");
    sStrT dst;
    sString::extractSubstring(&dst,src,0,num,V("separ"), VcaseI , Vlogic("isquotes") );
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}

idx sLang::eval_string_compareuntil(sLang * lg, idx il)
{
    lg->curScope->setVar("__ret", "%" DEC, sString::compareUntil( V("src1"), V("src2"), V("until"), VcaseI) ? 1 : 0 );
    return 0;
}

idx sLang::eval_string_search(sLang * lg, idx il)
{
    const char * ptr=sString::searchSubstring( V("src"), 0,  V("find"), VI("occurence"), V("stop"), VcaseI);
    lg->curScope->setVar("__ret","%s",  ptr ? ptr : "" );
    return 0;
}

idx sLang::eval_string_replacesymb(sLang * lg, idx il)
{
    const char * src=V("src");
    sStrT dst;
    sString::searchAndReplaceSymbols(&dst,src,0,V("find"), V("replace"), VI("maxtags"), Vlogic("ismatch"), Vlogic("isskipmult") , Vlogic("isquotes") );
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}

idx sLang::eval_string_replacestr(sLang * lg, idx il)
{
    const char * src=V("src");
    sStrT tmp;
    sString::searchAndReplaceStrings(&tmp,src,0,V("find"), V("replace"),VI("maxtags"), VcaseI );
    lg->curScope->setVar("__ret","%s",tmp.ptr() );
    return 0;
}

idx sLang::eval_string_crlf(sLang * lg, idx il)
{
    const char * src=V("src");
    const char * find ="\r\n", * replace="\n";
    if(!Vlogic("isunix")){find="\n";replace="\r\n";}
    sStrT dst;
    sString::searchAndReplaceStrings(&dst,src,0,find, replace,0, VcaseI );
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}

idx sLang::eval_string_cstyle(sLang * lg, idx il)
{
    sStrT result;
    const char * src=V("src");
    sString::cStyle(&result,src,sLen(src));
    lg->curScope->setVar("__ret","%s",result.ptr());
    return 0;
}


idx sLang::eval_string_cleanmarkup(sLang * lg, idx il)
{
    const char * src=V("src");
    sStrT dst;
    sString::cleanMarkup(&dst,src,0,V("start"), V("end"), V("replace"), VI("maxtags"), Vlogic("isinside"), false, VcaseI );
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}

idx sLang::eval_string_cleanends(sLang * lg, idx il)
{
    const char * src=V("src");
    sStrT dst;
    sString::cleanEnds(&dst,src,0,V("find"), Vlogic("ismatch"));
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}

idx sLang::eval_string_hungarian(sLang * lg, idx il)
{
    const char * src=V("src");
    sStrT dst;dst.add(0,sLen(src)+4);
    sString::hungarianText(src,dst.ptr(),0, (sString::eCase)(Vcase), Vlogic("isname"), Vlogic("nonintblanks"));
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}

idx sLang::eval_string_changecase(sLang * lg, idx il)
{
    const char * src=V("src");
    sStrT tmp;
    sString::changeCase(&tmp,src,0,(sString::eCase)(Vcase) );
    lg->curScope->setVar("__ret","%s",tmp.ptr());
    return 0;
}



idx sLang::eval_string_printf(sLang * lg, idx il)
{
    const char * fmt=V("fmt");
    if(!fmt || !*fmt) fmt="%s";
    const char * argp;
    sStrT ret,tmp;

    const char * s,* n;
    idx i;
    for ( i=VI("__var"), s=fmt; s && *s ;  ){
        
        n=strpbrk(s, "%\\");
        if(!n)n=s+strlen(s);
        
        if(n!=s)ret.add(s,(unsigned int)(n-s));

        if(*n=='%') {
            if(*(n+1)=='%'){
                ret.add("%%",2);n+=2;
            }
            else {
                tmp.printf(0,"__%" DEC,i);
                argp=V(tmp.ptr());
                if(argp)ret.add(argp,(unsigned int)strlen(argp));
                while( *n && !strchr("\\ildxfgespc" sString_symbolsBlank,*n) ) ++n;
                if(*n)++n;
                ++i ;
            }
        }
        else if(*n=='\\') {
            char ch=*(n+1);
            if(ch=='n')ch='\n';
            else if(ch=='r')ch='\r';
            else if(ch=='t')ch='\t';
            ret.add(&ch,1);
            n+=2;
        }
        s=n;
    }
    ret.add(_,2);
    lg->curScope->setVar("__ret","%s",ret.ptr());
    
    return 0;
}

idx sLang::eval_string_unescape( sLang * lg, idx il)
{
    const char * cont=V("src");
    sStrT ret;
    sString::searchAndReplaceStrings(&ret,cont,0,"\\n" _ "\\r" _ "\\t" __,"\n" _ "\r" _ "\t" __,0,0);
    char * ptr =ret.ptr();
    lg->curScope->setVar("__ret","%s",ptr ? ptr : "");
    return 0;
}

idx sLang::eval_string_enumerate(sLang * lg, idx il)
{
    const char * cont=V("callback");
    idx cnt;
    const char * separ=V("separ"), * srch;
    sStrT dst, dfl, calb;
    sStrT enumerator_counter;
    
    sStrT rslt;

    sString::searchAndReplaceSymbols(&dst,cont,0,separ," ",0,true,true,true);
    sString::cleanEnds(dst.ptr(),0,separ,true);

    for ( cnt=0 , srch=dst; srch && *srch ; ++cnt) { 
        srch=sString::extractSubstring( &dfl, srch, 0, 0, " " , 1, 1 );
        sString::searchAndReplaceSymbols(&calb,dfl.ptr(),0,"()",0,0,1,1,1);

        
        if(lg->dicFun.get(calb.ptr())==0 ){
            eval_string_printf(lg,il);
            sFilePath fp;fp.makeName( dfl.ptr(), lg->curScope->getVar("__ret"));
            calb.printf(0, "%s", fp.simplifyPath());
    
        }else calb.printf(0, "%s", dfl.ptr());

        if(calb.length()){ 
            rslt.cut(0);
            lg->expressionCompute(&rslt, calb.ptr(), sLen(calb.ptr()));
            if(rslt.length())enumerator_counter.printf(" %s", rslt.ptr());
        }
    }

    char * ptr =enumerator_counter.ptr();
    lg->curScope->setVar("__ret","%s",ptr ? ptr : "");
    
    return 0;
}





#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
    #include <fcntl.h>
#endif

idx sLang::eval_file_exists(sLang * lg, idx il)
{
    const char * flnm=V("flnm");
    bool fh = sFile::exists(flnm);

    lg->curScope->setVar("__ret","%" DEC,fh );
    return 0;
}

idx sLang::eval_file_open(sLang * lg, idx il)
{
    idx fh=open(V("flnm"), O_RDWR , S_IWRITE);
    lg->curScope->setVar("__ret","%" DEC,fh>0 ? fh : 0);
    return 0;
}

idx sLang::eval_file_close(sLang * lg, idx il)
{
    idx fh=VI("handle");
    if(fh>0)close((int)fh);
    lg->curScope->setVar("__ret","%" DEC,fh>0 ? fh : 0);
    return 0;
}

idx sLang::eval_file_len(sLang * lg, idx il)
{
    const char * flnm=V("flnh");
    idx fh,hdl=atoi(flnm);
    idx size=0;
        
    if(hdl>0)fh=open(flnm,O_RDONLY, S_IREAD);
    else fh=hdl;

    if(fh>0) {
        idx lpos = lseek((int)fh,0,SEEK_CUR);
        size = lseek((int)fh,0,SEEK_END);
        lseek((int)fh,(long)lpos,SEEK_SET);
    }
    
    if(!hdl)close((int)fh);

    lg->curScope->setVar("__ret","%" DEC,size);
    return 0;
}


idx sLang::eval_file_getpos(sLang * lg, idx il)
{
    idx fh=VI("handle");
    idx lpos =0;
    if(fh>0)lpos = lseek((int)fh,0,SEEK_CUR);
    lg->curScope->setVar("__ret","%" DEC,lpos);
    return 0;
}

idx sLang::eval_file_setpos(sLang * lg, idx il)
{
    idx fh=VI("handle");
    idx pos=VI("pos");
    idx lpos=VI("pos");
    if(fh>0)lpos = lseek((int)fh,(long)pos,SEEK_SET);
    lg->curScope->setVar("__ret","%" DEC,lpos);
    return 0;
}

idx sLang::eval_file_gets(sLang * lg, idx il)
{
    idx fh=VI("handle");
    const char * endline =V("endline"); if(!endline || !*endline) endline=sString_symbolsEndline;
    char dst[sSizePage];dst[0]=0;
    idx len=0,i;
    
    if(fh>0) {
        idx lpos=lseek((int)fh,0,SEEK_CUR);
        len = read((int)fh,dst,sizeof(dst));
        dst[len]=0;
        for (i=0; i<len && !strchr(endline,dst[i]) ; ++i) ++i;
        ++i;dst[i]=0;
        lseek((int)fh,(long)(lpos+i),SEEK_SET);
    }
    lg->curScope->setVar("__ret","%s",dst);
    return 0;
}

idx sLang::eval_file_read(sLang * lg, idx il)
{
    idx fh=VI("handle");
    idx size=VI("size");
    idx len=0;
    sStr d(sMex::fExactSize);char* dst=d.add(0,size+4);dst[0]=0;

    if(fh>0)len=read((int)fh,(void*)dst,(unsigned int)size);
    dst[len]=0;
    
    lg->curScope->setVar("__ret","%s",dst);
    return 0;
}

idx sLang::eval_file_write(sLang * lg, idx il)
{
    idx fh=VI("handle");
    idx size=VI("size");
    const char * cont=V("content");
    idx len=0;
    
    if(!cont)cont="";
    if(!size)size=(idx )strlen(cont);

    if(fh>0)len=write((int)fh,(void*)cont,(unsigned int)size);
    
    lg->curScope->setVar("__ret","%" DEC,len);
    return 0;
}

idx sLang::eval_file_remove(sLang * lg, idx il)
{
    const char * flnm=V("flnm");
    remove (flnm);
    lg->curScope->setVar("__ret","1" );
    return 0;
}

idx sLang::eval_file_rename(sLang * lg, idx il)
{
    const char * flnm1=V("flnm1");
    const char * flnm2=V("flnm2");
    rename(flnm1,flnm2);
    lg->curScope->setVar("__ret","1" );
    return 0;
}

idx sLang::eval_file_content(sLang * lg, idx il)
{
    const char * flnm=V("flnm");
        
    sFil fil(flnm,sFil::fReadonly);
    sStr t(sMex::fExactSize);t.add(fil.ptr()); t.add0(1);
    lg->curScope->setVar("__ret","%s", t.ptr());
    
    return 0;
}

idx sLang::eval_file_printf(sLang * lg, idx il)
{
    eval_string_printf(lg,il);
    const char * flnm=V("flnm");
    sFil fil(flnm);
    
    fil.printf("%s",lg->curScope->getVar("__ret"));
    lg->curScope->setVar("__ret","1");
    return 0;
}


idx sLang::eval_file_find(sLang * lg, idx il)
{
    sStrT rslt;
    
    idx flags=0; 
    if(Vlogic("isrecursive"))flags|=sFlag(sDir::bitRecursive);
    if(Vlogic("issubdir"))flags|=sFlag(sDir::bitSubdirs);
    if(Vlogic("isfiles"))flags|=sFlag(sDir::bitFiles);
    sDir dr;dr.findMany(flags, V("dirs"), V("flnm"), V("separ"), VI("maxfind"));
    char * ptr=rslt.ptr();
    lg->curScope->setVar("__ret","%s", ptr ? ptr : ""  );
    return 0;
    
}

idx sLang::eval_file_timestamp(sLang * lg, idx il)
{
    struct stat fst;sSet(&fst,0,sizeof(struct stat));
    stat(V("flnm"),&fst);
    lg->curScope->setVar("__ret","%li", fst.st_mtime > 0 ? fst.st_mtime  : 0 );
    return 0;
}



idx sLang::eval_file_makename(sLang * lg, idx il)
{
    const char * flnm=V("flnm");
    
    eval_string_printf(lg,il);
    sFilePath dst(flnm,lg->curScope->getVar("__ret"));
    dst.simplifyPath();
    lg->curScope->setVar("__ret","%s",dst.ptr());
    return 0;
}



idx sLang::eval_file_makedir(sLang * lg, idx il)
{
    const char * flnm=V("flnm");
    idx isfile=Vlogic("isfile");
    char * dir, * nxt, ch;
    sStrT dst(flnm);

    for ( nxt=dir=dst.ptr(); (nxt=strpbrk(nxt,"\\/"))!=0 ; ++nxt ) {
        ch=*nxt; *nxt=0;
        sDir::makeDir(dir);
        *nxt=ch;
    }
    if(!isfile)
        sDir::makeDir(dir);
    
    return 0;
}

idx sLang::eval_file_rmdir(sLang * lg, idx il)
{
    const char * flnm=V("flnm");
    sDir::removeDir(flnm);
    return 0;
}

idx sLang::eval_file_curdir(sLang * lg, idx il)
{
    
    sFilePath d;d.curDir();
    sString::searchAndReplaceSymbols(d.ptr(),0,"\\","/",0,1,0,0);
    lg->curScope->setVar("__ret","%s",d.ptr());
    return 0;
}



idx sLang::eval_online_httpget(sLang * lg, idx il)
{
    sMex http;

    const char * url=V("url");
    const char * data=V("data");
    sConClient::getHTTP(&http, url,data);
    http.add(__,2);
    lg->curScope->setVar("__ret","%s",http.ptr(0));
    return 0;
}






void sLang::buildStdLib (void) 
{

    parse (stdlibDeclarations,sLen(stdlibDeclarations),0);
    dicLib[ "app.printf" ]=eval_app_printf;
    dicLib[ "app.env" ] = eval_app_env;
    
    dicLib[ "dic.get" ] = eval_dic_get ;
    dicLib[ "dic.set" ] = eval_dic_set ;

    dicLib[ "string.len"] = eval_string_len;
    dicLib[ "string.cat"] = eval_string_cat;
    dicLib[ "string.cmp"] = eval_string_cmp;
    dicLib[ "string.cnt"] = eval_string_cnt;
    dicLib[ "string.cntsymb"] = eval_string_cntsymb;
    dicLib[ "string.skip"] = eval_string_skip;
    dicLib[ "string.extract"] = eval_string_extract;
    dicLib[ "string.compareuntil"] = eval_string_compareuntil;
    dicLib[ "string.search"] = eval_string_search;
    dicLib[ "string.replacesymb"] = eval_string_replacesymb;
    dicLib[ "string.crlf"] = eval_string_crlf;
    dicLib[ "string.cstyle"] = eval_string_cstyle;
    
    dicLib[ "string.replacestr"] = eval_string_replacestr;
    dicLib[ "string.cleanmarkup"] = eval_string_cleanmarkup;
    dicLib[ "string.cleanends"] = eval_string_cleanends;
    dicLib[ "string.hungarian"] = eval_string_hungarian;
    dicLib[ "string.changecase"] = eval_string_changecase;
    dicLib[ "string.printf"] = eval_string_printf;
    dicLib[ "string.unescape"] = eval_string_unescape;
    dicLib[ "string.enumerate"] = eval_string_enumerate;

    dicLib[ "file.exists"] = eval_file_exists;
    dicLib[ "file.open"] = eval_file_open;
    dicLib[ "file.gets"] = eval_file_gets;
    dicLib[ "file.read"] = eval_file_read;
    dicLib[ "file.write"] = eval_file_write;
    dicLib[ "file.close"] = eval_file_close;
    dicLib[ "file.len"] = eval_file_len;
    dicLib[ "file.getpos"] = eval_file_getpos;
    dicLib[ "file.setpos"] = eval_file_setpos;
    dicLib[ "file.content"] = eval_file_content;
    dicLib[ "file.remove"] = eval_file_remove;
    dicLib[ "file.rename"] = eval_file_rename;
    dicLib[ "file.printf"] = eval_file_printf;
    dicLib[ "file.find"] = eval_file_find;
    dicLib[ "file.makename"] = eval_file_makename;
    dicLib[ "file.timestamp"] = eval_file_timestamp;
    dicLib[ "file.makedir"] = eval_file_makedir;
    dicLib[ "file.rmdir"] = eval_file_rmdir;
    dicLib[ "file.curdir"] = eval_file_curdir;

    dicLib[ "online.httpget"] = eval_online_httpget;

    
}
