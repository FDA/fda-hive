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

#include <slib/lang.hpp>
#include <slib/std/string.hpp>
#include <slib/std/file.hpp>

using namespace slib;



char * sLang::scriptClean(sStr * dst, const char * source, idx len)
{
    if( !len && source ) len = sLen(source);
    if( !len ) return 0;
    idx pos=dst->pos();

    char last;
    const char * s=source, *e=source+len, *p;
    #define ADD(_v_k_)    dst->add(&(last=*(_v_k_)),1)

    if(!dst->length())ADD(" ");
    else last=0;
    for( ; s<e ; ++s) {
        char ch=*s;
        if(ch=='/') {
            if(*(s+1)=='*')
                for( s+=2 ; s<e && !(*s=='*' && *(s+1)=='/') ; ++s); 
            if(*(s+1)=='/')
                for(s+=2 ; s<e && *s!='\n';++s);
        }
        else if(ch=='\'' || ch=='\"' || ch=='`') {
            ADD(s);++s;
            while( s<e && *s!=ch ){
                ADD(s);++s;
            }
            ADD(s);
            continue;
        }
        else if(ch==' ' || ch=='\t'){
            if( last!=' ' && last!='\n')
                ADD(" ");
            continue;
        }
        else if(ch=='\r' || ch=='\n'){
            if( last!='\n')
                ADD("\n");
            continue;
        }
        else if(ch=='\\') {
            for( p=s+1; p<e && (*p==' ' || *p=='\t' || *p=='\r' ); ++p);
            if(*p=='\n'){
                s=p;continue;
            }
        }
        ADD(s);

    }

    ADD(_);

    return dst->ptr(pos);
}



const char * sLang::scriptTokenize(sDic < Statement > * stats, const char * src , idx len, idx ofsPos)
{
    #define N(_v_ptr) (((idx)((_v_ptr)-src))+ofsPos)
    #define NOSET 0

    if( !len && src ) len = sLen(src);
    if( !len ) return 0;
    
    Statement * lx=0;
    idx ic;const char * ptr;
    for (ptr=src; *ptr && ptr<src+len; ++ptr ) {

        if(!lx && !strchr(sString_symbolsBlank,*ptr) ) {
            lx = stats->add(1);
            sSet(lx,0);
        }
        if(!lx)continue;
       
        if(lx->stat.Start==NOSET && !strchr(sString_symbolsBlank,*ptr) ){
            lx->stat.Start=N(ptr);
            lx->nam.Start=N(ptr);
        }
        
        if(strchr(sString_symbolsBlank,*ptr) && lx->nam.Start!=NOSET && lx->nam.End==NOSET)
            lx->nam.End=N(ptr);

        if(*ptr=='\n' && src[lx->stat.Start]=='#' ) {
            lx->stat.End=N(ptr); 
            if(lx->body.Start!=NOSET)lx->body.End=N(ptr);
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            lx=0;
        }

        else if( *ptr == '=' && *(ptr+1)!='=' && lx->equ.Start==NOSET ) {
            lx->equ.Start=N(ptr+1);
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            for ( ++ptr ; *ptr && ptr<src+len ; ++ptr ) {
                if( *ptr==';' ) break;
            }
            lx->equ.End=N(ptr);
            lx->stat.End=N(ptr);
            lx=0;
        }
        
        else if ( *ptr == '(' && lx->prth.Start==NOSET ){
            lx->prth.Start=N(ptr+1);
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            for ( ic=1, ++ptr; *ptr && ptr<src+len ; ++ptr ) {
                if( *ptr=='(' ) ++ic;
                else if( *ptr==')' ) --ic;
                if(!ic)break;
            }
            lx->prth.End=N(ptr);
        }

        else if ( *ptr == '{' ){
            lx->body.Start=N(ptr+1);
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            for ( ic=1, ++ptr; *ptr && ptr<src+len; ++ptr ) {
                if( *ptr=='{' ) ++ic;
                else if( *ptr=='}' ) --ic;
                if(!ic)break;
            }
            lx->body.End=N(ptr);
            lx->stat.End=N(ptr);
            lx=0;
        }
        
        else if( *ptr==';') {
            lx->stat.End=N(ptr);
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            lx=0;
        }
        if(!(*ptr) || ptr>=src+len)
            break;
    }
    
    if(lx){
        lx->stat.End=N(ptr);
        if(lx->body.End==NOSET && lx->body.Start!=NOSET)lx->body.End=N(ptr);
        if(lx->nam.End==NOSET && lx->nam.Start!=NOSET)lx->nam.End=N(ptr);
    }

    return src;
}


idx sLang::parse(const char * source, idx len , bool issubscript)
{
    const char * ptr;
    
    if(issubscript)
        ptr=source;
    else { 
        ptr=scriptClean(&script, source,len);
        len=script.length();
    }

    idx pos=dicStat.dim();
    scriptTokenize(&dicStat,ptr,len,(idx)(ptr-script.ptr()) );

    for( idx il=pos; il<dicStat.dim(); ++il) {
        executeStatement(il);
        if( curScope->retCond || curScope->contCond || exitCond)
            break;
    }
return 0;
}

#ifdef DEBUGOUT
void sLang::debugPrintf( const char * fmt, ... )
{
    if(!doDebug && !curScope->doDebug)return 0;
    for( idx i=0; i< funLevel; ++i) 
        printf("  ");

    sCallVarg(vprintf,fmt);
    return ;
}
#endif


char * sLang::getStat(sStr * dst, const sLang::Location * loc, idx clean)
{

    idx l=(loc->End-loc->Start);
    char * out=l ? dst->add(script.ptr(loc->Start),l) : dst->last();
    dst->add(_,1);
    if(out && clean)sString::cleanEnds(out,0,sString_symbolsBlank,true);
    return out;
}
const char * sLang::Scope::setVar( const char * var, const char * fmt, ... )
{
    Scope * scp; if(var[0]==':' && var[1]==':'){scp=global;var+=2;}else scp=this;
    idx * ptrOfs = scp->dicVar.set(var);if(!ptrOfs) return "";
    idx ofs=scp->data.length();
    sCallVarg(scp->data.vprintf,fmt);scp->data.add0(2);
    *ptrOfs=ofs;
    return scp->data.ptr(ofs);
}



bool sLang::executeStatement(idx il )
{
    Statement * lx=dicStat.ptr(il); 
    ExecFunction * eval;
    sStr tmp;
    char * nam=getStat(&tmp,&lx->nam);

    #ifdef DEBUGOUT
        debugPrintf("\n");
    #endif

    if( (eval = dicLang.get(nam))!=0 ){

        if( lx->body.Start==lx->body.End && *script.ptr(lx->stat.Start)!='#' ){

            if( strcmp(nam,"else")==0 ) 
                lx->body.Start=lx->nam.End;
            else 
                lx->body.Start=sMax(lx->prth.End+1,lx->nam.End);
            
            lx->body.End=lx->stat.End;
        }
        (*eval)(this,il);
    }
    else if( nam[0] && lx->body.Start!=NOSET && *script.ptr(lx->body.Start-1)=='{' )
        exec_declarefun(this,il);

    else if(lx->equ.Start!=lx->equ.End ){
        exec_assignment(this,il);
        return true;
    }
    else 
        exec_generic(this,il);


    return true;
}



idx sLang::exec_declarefun(sLang * lg, idx il)
{
    sStr tmp;
    Statement * lex=lg->dicStat.ptr(il);
    char * fun=lg->getStat(&tmp,&lex->nam,1);

    idx * numL=lg->dicFun.set(fun);
    if(numL)*numL=il; 

    #ifdef DEBUGOUT
        lg->debugPrintf("function %s ", fun);
        tmp.cut(0);fun=lg->getStat(&tmp,&lex->prth,1);
        lg->debugPrintf("(%s)\n", fun);
        lg->debugPrintf("{\n");
        
        tmp.cut(0);fun=lg->getStat(&tmp,&lex->body,1);
        sString::searchAndReplaceSymbols(fun,0,"\n "," ",0,1,1,0);
        lg->debugPrintf("%s\n", fun );
        lg->debugPrintf("}\n");
    #endif        

    return 0;
}


idx sLang::exec_assignment(sLang * lg, idx il)
{
    sStr nam,rslt;
    Statement * lex=lg->dicStat.ptr(il);
    
    lg->expressionCompute(&rslt , lg->script.ptr(lex->equ.Start), lex->stat.End-lex->equ.Start);
    lex=lg->dicStat.ptr(il);

    char * var=lg->getStat(&nam,&(lex->nam),1);
    Scope * scp;

    if( !lg->curScope->isVar(var) && lg->globalScope.isVar(var) )
        scp=&lg->globalScope;
    else scp=lg->curScope;

    scp->setVar(var,"%s",rslt.ptr());

    #ifdef DEBUGOUT
        lg->debugPrintf("%s%s = '%s' ;\n", (scp==&lg->globalScope ) ? "::" : "" , var, rslt.ptr());
    #endif

    return 0;

}

idx sLang::exec_generic(sLang * lg, idx il)
{
    Statement * lex=lg->dicStat.ptr(il);

    if ( lex->body.Start==lex->body.End) {
        lg->reslt.cut(0);
        lg->expressionCompute( &lg->reslt, lg->script.ptr(lex->stat.Start), lex->stat.End-lex->stat.Start ) ;
    }
    else {
        #ifdef DEBUGOUT
            lg->debugPrintf("{");
        #endif
        ++(lg->funLevel);
        lg->parse( lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start, true) ;
        --(lg->funLevel);
        #ifdef DEBUGOUT
            lg->debugPrintf("}\n");
        #endif
    }

    return 0;
}






idx sLang::expressionCallback(sLang * lg, sStr * ,  sCalc * xic, idx cur, idx cnt, idx )
{
    sStr nam,cot;
    char * ct=cot.printf("%s",xic->cont(cur)), * var;
    sCalc::Lexem * lit=xic->lexem(cur);
    idx * pn;
    Scope * scp;
    
    if( (pn=lg->dicFun.get(ct))!=0 ) {
        
        Statement * lx=lg->dicStat.ptr(*pn);
        Scope * scopeFun = new Scope(lg->curScope, ct ) ;
        
        ExecFunction * eval= (lx->body.End==lx->body.Start) ?  lg->dicLib.get(ct) : 0;

        #ifdef DEBUGOUT
            lg->debugPrintf("%s %s ( ", eval ? "callib"  : "calling" , ct );
        #endif

        idx funlev=lg->funLevel;lg->funLevel=0;
        cnt=0;

        idx argstart=0,argend=0,varg=0,iArg=0;
        if( cur<xic->dim()-1 && *(xic->cont(cur+1))=='(' ) {

            if((xic->lexem(cur+1)->status)&sCalc::Lexem::fSubComplete) {argstart=cur+1; argend=argstart+1;}
            else {for ( argend=argstart=cur+2; strcmp(")", xic->cont(argend))  ; ++argend );}

            char * srchpar=lg->script.ptr(lx->prth.Start);
            for ( iArg=argstart; iArg<argend  ; ++iArg){
                if( !(xic->lexem(iArg)->status&sCalc::Lexem::fQuoted) && !strcmp(",", xic->cont(iArg)) )
                    continue;

                var=0;
                if( lx->prth.Start!=lx->prth.End){
                    nam.cut(0);
                    char * nxt=sString::extractSubstring(&nam, srchpar,0, 0,"," _ ")" __,false, true );
                    var=nam.ptr();
                    sString::cleanEnds(var,0,sString_symbolsBlank,true);
                    if(strcmp(var,"..."))srchpar=nxt;
                    else { sprintf(var,"__%" DEC,cnt);if(!varg)varg=cnt;}
                    ++cnt;
                }
                
                char * val=xic->data(iArg);
                scopeFun->setVar( var ? var : "__", "%s", val ? val: "__UNDEFINED" ) ;

                #ifdef DEBUGOUT 
                    if(iArg!=argstart) lg->debugPrintf(", ");
                    lg->debugPrintf("%s = '%s' ", var , scopeFun->getVar(var) );
                #endif
            }
        }
        
        
        scopeFun->setVar( "__cnt", "%" DEC, cnt) ;
        if(varg)scopeFun->setVar( "__var", "%" DEC, varg) ;
        scopeFun->setVar( "__fun", "%s", ct) ;
            
        #ifdef DEBUGOUT 
            lg->debugPrintf(")" );
            if(!eval)lg->debugPrintf("{\n" );
            else lg->debugPrintf(";\n" );
        #endif

        lg->funLevel=funlev;

        ++(lg->funLevel);
        lg->curScope->down = scopeFun;
        lg->curScope=lg->curScope->down;
        if(eval)(*eval)(lg,*pn);
        else lg->parse( lg->script.ptr(lx->body.Start), lx->body.End-lx->body.Start, 1 ) ;
        --(lg->funLevel);
        lg->curScope=lg->curScope->up;

        if( (xic->lexem(cur+1)->status)&sCalc::Lexem::fSubComplete )--argend;
        xic->del(cur+1,argend-cur);
        xic->data(cur,0,"%s",lg->curScope->down->getVar("__ret"));

        xic->debugPrint("HHR - AFTER");

        delete scopeFun;

        #ifdef DEBUGOUT 
            if(!eval)lg->debugPrintf("}\n" );
        #endif
    }

    else if( ((scp=lg->curScope)->isVar(ct))!=0 || ((scp=(&lg->globalScope))->isVar(ct))!=0  ) {
        xic->data(cur,0,"%s",scp->getVar(ct));
    }

    else {
        return 0;
    }


    lit=xic->lexem(cur);
    lit->type|=sCalc::Lexem::fString|sCalc::Lexem::fNumber;
    lit->status|=sCalc::Lexem::fReady;

    return cur+1;
}

idx sLang::expressionCompute(sStr * out, const char * phrase, idx len )
{
    if( !len && phrase ) len = sLen(phrase);
    if( !len ) return 1;

    sCalc calc((sCalc::CallbackFuncType)expressionCallback,this);
    calc.analyse(sCalc::fCallbackAll, out, phrase, len);
    calc.collectResult(out, 1, " ", 0, true);

    return calc.errnum();
}




idx sLang::exec_debug(sLang * lg, idx il)
{
    Statement * lex=lg->dicStat.ptr(il);
    sStr tmp;
    idx local;
    
    lg->getStat(&tmp,&lex->nam,1);
    if(!strcmp(tmp.ptr(0),"debul"))local=1;else local=0;

    tmp.cut(0);lg->getStat(&tmp,&lex->body,1);
    if( ! strncmp(tmp.ptr(), "off",3) ){if(local)lg->curScope->doDebug=0;else lg->doDebug=0;}
    else if( ! strncmp(tmp.ptr(), "on",2) ){if(local)lg->curScope->doDebug=1;else lg->doDebug=1;}

    #ifdef DEBUGOUT
        lg->debugPrintf("debug %s;\n", (lg->doDebug || lg->curScope->doDebug) ? "on" : "off");
    #endif

    return 0;
}

idx sLang::exec_codebreak(sLang * lg, idx il)
{
    sStr tmp,nm;
    Statement * lex=lg->dicStat.ptr(il);

    if(lex->nam.End!=lex->body.End)
        lg->expressionCompute(&tmp, lg->script.ptr(lex->nam.End), lex->body.End-lex->nam.End);
    lex=lg->dicStat.ptr(il);

    char * ptr=tmp.ptr();
    if(ptr)lg->curScope->setVar("__ret",ptr);
    
    char * nam=lg->getStat(&nm,&lex->nam,1);
    if(!strcmp(nam,"return"))lg->curScope->retCond=1;
    else if(!strcmp(nam,"break"))lg->curScope->breakCond=1;
    else if(!strcmp(nam,"continue"))lg->curScope->contCond=1;
    else if(!strcmp(nam,"exit")) {if(ptr && *ptr)lg->msg->printf("%s",ptr); lg->exitCond=1;}
    else if(!strcmp(nam,"pause")){if(ptr && *ptr)lg->msg->printf("%s",ptr);}

    #ifdef DEBUGOUT
        lg->debugPrintf("%s %s;\n", nam, lg->curScope->getVar("__ret") );
    #endif

    return 0;
}

idx sLang::exec_if(sLang * lg, idx il)
{
    sStr tmp;
    Statement * lex=lg->dicStat.ptr(il);
    
    #ifdef DEBUGOUT 
        lg->getStat(&tmp,&lex->prth,1);
        lg->debugPrintf("if ( %s )", tmp.ptr() );
        tmp.cut(0);
    #endif

    if( lex->prth.Start!=lex->prth.End && (lg->expressionCompute(&tmp, lg->script.ptr(lex->prth.Start), lex->prth.End-lex->prth.Start) || atoi(tmp.ptr())==0 ) ){
        #ifdef DEBUGOUT 
            lg->debugPrintf(";    // false: %s\n", tmp.ptr());
        #endif
        lg->curScope->lastIf=0;
        return -1;
    }

    lex=lg->dicStat.ptr(il);
    #ifdef DEBUGOUT 
        lg->debugPrintf("    // true: %s\n", tmp.ptr());
        lg->debugPrintf("{");
    #endif

    ++(lg->funLevel);
    lg->parse( lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start, 1 ) ;
    --(lg->funLevel);

    lg->curScope->lastIf=1;

    #ifdef DEBUGOUT 
        lg->debugPrintf("}\n");
    #endif
    return 0;
}

idx sLang::exec_else(sLang * lg, idx il)
{
    sStr tmp;
    Statement * lex=lg->dicStat.ptr(il);

    #ifdef DEBUGOUT 
        lg->debugPrintf("else ");
    #endif
    
    
    #ifdef RECOMPUTE_IF_FOR_ELSE
        Statement * lexPrev= il ? lg->dicStat.ptr(il-1) : 0 ;
        if( strcmp(lg->getStat(&tmp,&lexPrev->nam,1),"if") )
            return -1;

        #ifdef DEBUGOUT 
            tmp.cut(0);lg->getStat(&tmp,&lexPrev->prth,1);
            lg->debugPrintf("else //    if(! %s ) ", tmp.ptr());
            tmp.cut(0);
        #endif

        if( lexPrev->prth.Start==lexPrev->prth.End || (lexPrev->prth.Start!=lexPrev->prth.End &&  !lg->lexicCompute(&tmp , lg->script.ptr(lexPrev->prth.Start), lexPrev->prth.End-lexPrev->prth.Start) && atoi(tmp.ptr()))  ){
            #ifdef DEBUGOUT 
                lg->debugPrintf(";    // false: %s\n", tmp.ptr());
            #endif
            return -1;
        }

        #ifdef DEBUGOUT 
            lg->debugPrintf("// true: %s\n", tmp.ptr());
        #endif
    #else 
        if( lg->curScope->lastIf  ){
            #ifdef DEBUGOUT 
                lg->debugPrintf(";    // false\n");
            #endif
            return -1;
        }
    #endif 
    #ifdef DEBUGOUT 
        lg->debugPrintf("// true\n");
        lg->debugPrintf("{");
    #endif

    ++(lg->funLevel);
    lg->parse( lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start, 1 ) ;
    --(lg->funLevel);

    #ifdef DEBUGOUT 
        lg->debugPrintf("} // end else \n");
    #endif

    return 0;
}

idx sLang::exec_loop(sLang * lg, idx il)
{
    sStr tmp;
    Statement * lex=lg->dicStat.ptr(il);
    
    #ifdef DEBUGOUT 
        lg->getStat(&tmp,&lex->prth,1);
        lg->debugPrintf("while ( %s )", tmp.ptr() );
        tmp.cut(0);
    #endif

    char * inipos=0, * nexpos=0;
    if(lex->prth.Start)inipos=sString::searchSubstring(lg->script.ptr(lex->prth.Start),0,";" __,1,")" __,0);
    if(inipos)nexpos=sString::searchSubstring(inipos+1,0,";" __,1,")" __,0);
    
    if(inipos) {
        lg->parse( lg->script.ptr(lex->prth.Start),(idx )(inipos-lg->script.ptr(lex->prth.Start)),1);
        inipos++;
    }
    else { 
        inipos = lg->script.ptr(lex->prth.Start);
        #ifdef DEBUGOUT 
            lg->debugPrintf("\n");
        #endif
    }
    if(!nexpos)
        nexpos=lg->script.ptr(lex->prth.End);

    #ifdef DEBUGOUT 
        lg->debugPrintf("{\n" );
    #endif

    ++(lg->funLevel);
    while( lex->prth.Start==lex->prth.End || (!lg->expressionCompute(&tmp, inipos, (idx)(nexpos-inipos) ) && atoi(tmp.ptr()) ))
    {
        lex=lg->dicStat.ptr(il);
        lg->parse( lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start, true ) ;
        lg->curScope->contCond=0;
        if(lg->curScope->breakCond || lg->curScope->retCond || lg->exitCond)
            break;
        
        if(nexpos!=lg->script.ptr(lex->prth.End)) {
            lg->parse( nexpos+1, (idx)(lg->script.ptr(lex->prth.End)-nexpos)-1, 1 ) ;
        }
        tmp.cut(0);
    }
    lg->curScope->breakCond=0;
    --(lg->funLevel);

    #ifdef DEBUGOUT 
        lg->debugPrintf("} // exit:  %s\n", tmp.ptr() );
    #endif
    
    return 0;
}



idx sLang::exec_shell(sLang * lg, idx il)
{
    sStr shellscript,tmp;
    Statement * lex=lg->dicStat.ptr(il);
    
    if( *lg->script.ptr(lex->body.Start-1)!='{' ){
        lg->expressionCompute(&shellscript, lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start);
        lex=lg->dicStat.ptr(il);
        sString::cleanEnds(shellscript.ptr(),shellscript.length(),sString_symbolsBlank,1);
    }else {
        shellscript.add(lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start);
        shellscript.add("\n\0",2);
    }

    char * nam=lg->getStat(&tmp,&lex->nam,1);
    idx type = (strcmp(nam,"shell")) ? 1 : 0 ;
    tmp.cut(0);
    
    #ifdef DEBUGOUT 
        lg->debugPrintf("shell\n");
        lg->debugPrintf("{\n");
        lg->debugPrintf("%s\n",shellscript.ptr() );
        lg->debugPrintf("}\n\n----- shell-start -----\n", shellscript.ptr() );
    #endif
    
    sIO tt;
    if(type){sPipe::exePipe( &tt, shellscript.ptr(), 0);tt.add(_,1);}
    
    else system(shellscript.ptr());
    lg->curScope->setVar("__ret",tt.ptr());

    #ifdef DEBUGOUT 
        lg->debugPrintf("%s",tmp.ptr());
        lg->debugPrintf("\n----- shell-end -----\n\n");
    #endif

    return 0;
}

idx sLang::exec_include(sLang * lg, idx il)
{
    sStr tmp;
    Statement * lex=lg->dicStat.ptr(il);
    
    char * flnm=tmp.add( lg->script.ptr(lex->nam.End),lex->stat.End-lex->nam.End);
    tmp.add0(1);
    sString::cleanEnds(flnm,0,"\"><",1);
    
    sFilePath olddir; olddir.curDir();
    sFilePath newdir(flnm,"%%dir");newdir.simplifyPath();
            

    #ifdef DEBUGOUT 
        lg->debugPrintf("include \"%s\" ", tmp.ptr());
    #endif
    
    sFil fil(flnm,sFil::fReadonly);
    
    if(!fil.ptr()) {
        #ifdef DEBUGOUT 
            lg->debugPrintf( "; // can't open\n " );
        #endif
        return -1;
    }
    
    #ifdef DEBUGOUT 
        lg->debugPrintf("\n---- include %s start -----\n", flnm);
    #endif
    
        if(newdir.length())sDir::chDir ( newdir.ptr() );
    ++(lg->funLevel);
    lg->parse( fil.ptr(), fil.length() ) ;
    --(lg->funLevel);
    if(newdir.length())sDir::chDir( olddir.ptr() );

    

    #ifdef DEBUGOUT 
        lg->debugPrintf("\n---- include %s end -----\n", flnm );
    #endif

    return 0;
}

idx sLang::exec_define(sLang * lg, idx il)
{
    Statement * lex=lg->dicStat.ptr(il);
    
    for ( lex->nam.Start=lex->nam.End ; strchr(sString_symbolsSpace,*lg->script.ptr(lex->nam.Start)) ; ++(lex->nam.Start) ) ;
    for ( lex->nam.End=lex->nam.Start; !strchr(sString_symbolsBlank,*lg->script.ptr(lex->nam.End)) ; ++(lex->nam.End) ) ;
    lex->body.Start=lex->nam.End;
    lex->body.End=lex->stat.End;

    return exec_declarefun(lg, il);
}





#define V(_v_nam)           (lg->curScope->getVar(_v_nam))

idx sLang::exec_eval(sLang * lg, idx )
{
    const char * txt=V("txt");
    sStr rslt; 
    if(txt)lg->expressionCompute(&rslt, txt, (idx)strlen(txt));
    lg->curScope->setVar("__ret","%s",rslt.ptr());

    return 0;
}

idx sLang::exec_parse(sLang * lg, idx )
{
    const char * txt=V("txt");
    sStr rslt; 
    if(txt){lg->parse(txt,(idx)strlen(txt),0);}

    return 0;
}

idx sLang::exec_isvar(sLang * lg, idx )
{
    lg->curScope->setVar("__ret","%d", lg->curScope->isVar(V("nam")));
    return 0;
}

idx sLang::exec_isfun(sLang * lg, idx )
{
    lg->curScope->setVar("__ret","%d", (lg->dicFun.get(V("nam"))  ? 1 : 0 ));
    return 0;
}






void sLang::buildCore( void )
{
    dicLang[ "if" ]=exec_if;
    dicLang[ "else" ]=exec_else;
    dicLang[ "while" ]=exec_loop;
    dicLang[ "for" ]=exec_loop;
    dicLang[ "shell" ]=exec_shell;
    dicLang[ "exec" ]=exec_shell;
    dicLang[ "#include" ]=exec_include;
    dicLang[ "#define" ]=exec_define;
    dicLang[ "return" ]=exec_codebreak;
    dicLang[ "break" ]=exec_codebreak;
    dicLang[ "continue" ]=exec_codebreak;
    dicLang[ "pause" ]=exec_codebreak;
    dicLang[ "exit" ]=exec_codebreak;
    dicLang[ "debug" ]=exec_debug;
    dicLang[ "debul" ]=exec_debug;
    
    const char * corefun=   "eval( txt ){}" 
                            "parse( txt ){}" 
                            "isvar ( nam ) {}"
                            "isfun ( nam ) {}";

    parse (corefun,sLen(corefun),0);
    dicLib[ "eval" ] = exec_eval;
    dicLib[ "parse" ] = exec_parse;
    dicLib[ "isvar" ] = exec_isvar;
    dicLib[ "isfun" ] = exec_isfun;

}

















