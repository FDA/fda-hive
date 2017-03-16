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

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Script Pretreatment / Tokenization
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


char * sLang::scriptClean(sStr * dst, const char * source, idx len)
{
    if(!len && source)len=sLen(source); if(!len)return 0;
    idx pos=dst->pos();

    //char  * dst=(char*)vMem::New(len+4), *d=dst, ch;
    char last;
    const char * s=source, *e=source+len, *p;
    #define ADD(_v_k_)    dst->add(&(last=*(_v_k_)),1)

    if(!dst->length())ADD(" ");// *d++=' '
    else last=0;//*(dst->last()-1);
    for( ; s<e ; ++s) { // 
        char ch=*s;
        if(ch=='/') { // is it a comment 
            if(*(s+1)=='*') // block comment 
                for( s+=2 ; s<e && !(*s=='*' && *(s+1)=='/') ; ++s); 
            if(*(s+1)=='/') // one-line comment
                for(s+=2 ; s<e && *s!='\n';++s);
        }
        else if(ch=='\'' || ch=='\"' || ch=='`') { // quotes are copied without a change
            ADD(s);++s;//*d++=*s++;
            while( s<e && *s!=ch ){
                ADD(s);++s;//*d++=*s++; // scan to the next same quotation sign 
            }
            ADD(s);//*d++=*s;
            continue;
        }
        else if(ch==' ' || ch=='\t'){
            //if( *(d-1)!=' ' && *(d-1)!='\n') // no multiple spaces 
            if( last!=' ' && last!='\n') // no multiple spaces 
                ADD(" ");//*d++=' ';
            continue;
        }
        else if(ch=='\r' || ch=='\n'){
            if( last!='\n') // no multiple newlines
                ADD("\n");//*d++='\n';
            continue;
        }
        else if(ch=='\\') {
            for( p=s+1; p<e && (*p==' ' || *p=='\t' || *p=='\r' ); ++p); // scan the spaces 
            if(*p=='\n'){ // end of line immidiately after spaces ? 
                s=p;continue;
            }
        }
        //*d++=*s;
        ADD(s);

    }

    ADD(_);
    //*d=0;

    return dst->ptr(pos);
}



const char * sLang::scriptTokenize(sDic < Statement > * stats, const char * src , idx len, idx ofsPos)
{
    #define N(_v_ptr) (((idx)((_v_ptr)-src))+ofsPos)
    #define NOSET 0

    if(!len && src)len=sLen(src); if(!len)return 0;
    
    Statement * lx=0;
    idx ic;const char * ptr;
    for (ptr=src; *ptr && ptr<src+len; ++ptr ) {

        // add a new statement item 
        if(!lx && !strchr(sString_symbolsBlank,*ptr) ) {
            lx = stats->add(1);
            sSet(lx,0);
        }
        if(!lx)continue;
       
        if(lx->stat.Start==NOSET && !strchr(sString_symbolsBlank,*ptr) ){ // new statement ? set the beginning 
            lx->stat.Start=N(ptr); // beginning of the statement  
            lx->nam.Start=N(ptr); // beginning of the name
        }
        
        if(strchr(sString_symbolsBlank,*ptr) && lx->nam.Start!=NOSET && lx->nam.End==NOSET)
            lx->nam.End=N(ptr); // end of name on the first space always 

        if(*ptr=='\n' && src[lx->stat.Start]=='#' ) { // end of macro statement  // *P(lx->stat.Start)
            lx->stat.End=N(ptr); 
            if(lx->body.Start!=NOSET)lx->body.End=N(ptr); // end of body
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr); // end of name 
            lx=0; // end of this statement 
        }

        else if( *ptr == '=' && *(ptr+1)!='=' && lx->equ.Start==NOSET ) { // assignments 
            lx->equ.Start=N(ptr+1);
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            for ( ++ptr ; *ptr && ptr<src+len ; ++ptr ) { // scan to the corresponding end of the parenthesis     
                if( *ptr==';' ) break;
            }
            lx->equ.End=N(ptr);
            lx->stat.End=N(ptr); // end of statement 
            lx=0; // end of this statement 
        }
        
        else if ( *ptr == '(' && lx->prth.Start==NOSET ){ // parenthesis  
            lx->prth.Start=N(ptr+1); // beginning of parenthesis
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            for ( ic=1, ++ptr; *ptr && ptr<src+len ; ++ptr ) { // scan to the corresponding end of the parenthesis 
                if( *ptr=='(' ) ++ic;
                else if( *ptr==')' ) --ic;
                if(!ic)break;
            }
            lx->prth.End=N(ptr); //end of parenthesis 
        }

        else if ( *ptr == '{' ){ // body in curly brakets 
            lx->body.Start=N(ptr+1); // beginning of body 
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            for ( ic=1, ++ptr; *ptr && ptr<src+len; ++ptr ) { // scan to the corresponding end of the curly brakets 
                if( *ptr=='{' ) ++ic;
                else if( *ptr=='}' ) --ic;
                if(!ic)break;
            }
            lx->body.End=N(ptr); // end of body 
            lx->stat.End=N(ptr); // end of statement 
            lx=0; // end of this statement 
        }
        
        else if( *ptr==';') {
            lx->stat.End=N(ptr); // end of statement 
            if(lx->nam.End==NOSET)lx->nam.End=N(ptr);
            lx=0; // end of this statement 
        }
        if(!(*ptr) || ptr>=src+len)
            break;
    }
    
    if(lx){
        lx->stat.End=N(ptr);
        if(lx->body.End==NOSET && lx->body.Start!=NOSET)lx->body.End=N(ptr); // end of body
        if(lx->nam.End==NOSET && lx->nam.Start!=NOSET)lx->nam.End=N(ptr); // end of body
    }

    return src;
}


idx sLang::parse(const char * source, idx len , bool issubscript)
{
    const char * ptr;
    
    if(issubscript)  // check if this source is already in the script ... 
        ptr=source;
    else { 
        ptr=scriptClean(&script, source,len); // get the treated input 
        len=script.length();
    }

    idx pos=dicStat.dim();
    scriptTokenize(&dicStat,ptr,len,(idx)(ptr-script.ptr()) );

    // scan over all the statements
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
    //#define P(_v_ofs) (lg->script.ptr(_v_ofs))

    idx l=(loc->End-loc->Start);
    char * out=l ? dst->add(script.ptr(loc->Start),l) : dst->last();
    dst->add(_,1);
    if(out && clean)sString::cleanEnds(out,0,sString_symbolsBlank,true); // clean spaces at the end
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

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Fundamental code execution functions 
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


bool sLang::executeStatement(idx il )
{
    Statement * lx=dicStat.ptr(il); 
    ExecFunction * eval;
    sStr tmp;
    char * nam=getStat(&tmp,&lx->nam);

    #ifdef DEBUGOUT
        debugPrintf("\n");
    #endif

    if( (eval = dicLang.get(nam))!=0 ){  // look if this is a language construction 

        if( lx->body.Start==lx->body.End && *script.ptr(lx->stat.Start)!='#' ){ // has no body and is not a macro 

            if( strcmp(nam,"else")==0 ) 
                lx->body.Start=lx->nam.End; // it is after the parenthesis
            else 
                lx->body.Start=sMax(lx->prth.End+1,lx->nam.End); // it is after the parenthesis
            
            lx->body.End=lx->stat.End; // and to the statement end
        }
        (*eval)(this,il); // call the language construction handling function
    }
    else if( nam[0] && lx->body.Start!=NOSET && *script.ptr(lx->body.Start-1)=='{' ) // real function body declaration 
        exec_declarefun(this,il);

    else if(lx->equ.Start!=lx->equ.End ){ // has an equal assigment
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

    idx * numL=lg->dicFun.set(fun); // get the existing or create a new entry 
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
    
    // compute the right part 
    lg->expressionCompute(&rslt , lg->script.ptr(lex->equ.Start), lex->stat.End-lex->equ.Start);
    lex=lg->dicStat.ptr(il); // lexicCompute has a potential of reallocating the statments dictionary , so we have to reget the pointer

    // set the variable 
    char * var=lg->getStat(&nam,&(lex->nam),1);
    Scope * scp;

    if( !lg->curScope->isVar(var) && lg->globalScope.isVar(var) ) // not local but already in global 
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

    // parse the whole statement 
    if ( lex->body.Start==lex->body.End) {
        lg->reslt.cut(0);
        lg->expressionCompute( &lg->reslt, lg->script.ptr(lex->stat.Start), lex->stat.End-lex->stat.Start ) ;
    }
    else {
        #ifdef DEBUGOUT
            lg->debugPrintf("{"); // DEBUG PRINTOUT
        #endif
        ++(lg->funLevel);
        lg->parse( lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start, true) ;
        --(lg->funLevel);
        #ifdef DEBUGOUT
            lg->debugPrintf("}\n"); // DEBUG PRINTOUT
        #endif
    }

    return 0;
}





// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Expression evaluation 
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx sLang::expressionCallback(sLang * lg, sStr * ,  sCalc * xic, idx cur, idx cnt, idx )
{
    sStr nam,cot;
    char * ct=cot.printf("%s",xic->cont(cur)), * var;
    sCalc::Lexem * lit=xic->lexem(cur);
    idx * pn;
    Scope * scp;
    
    // check if this is a user defined or a library function name 
    if( (pn=lg->dicFun.get(ct))!=0 ) {
        
        // prepare the function and the scope 
        Statement * lx=lg->dicStat.ptr(*pn); // get the corresponding function statement 
        Scope * scopeFun = new Scope(lg->curScope, ct ) ; // create a scope 
        
        ExecFunction * eval= (lx->body.End==lx->body.Start) ?  lg->dicLib.get(ct) : 0; // see if this is standard library function (bodyless definition)
        //var=nam.add(0,lx->prth.End-lx->prth.Start+1+6) ; // this buffer should be enough to hold all the arguments names (__var and __cnt too ) 

        #ifdef DEBUGOUT
            lg->debugPrintf("%s %s ( ", eval ? "callib"  : "calling" , ct );
        #endif

        idx funlev=lg->funLevel;lg->funLevel=0; // remember this so we can restore at the end 
        cnt=0;

        // evaluate arguments and set as localScope variables 
        idx argstart=0,argend=0,varg=0,iArg=0;
        if( cur<xic->dim()-1 && *(xic->cont(cur+1))=='(' ) { // the next one is parenthesis ? evaluate it 

            // scan arguments in the function call and in function declaration
            if((xic->lexem(cur+1)->status)&sCalc::Lexem::fSubComplete) {argstart=cur+1; argend=argstart+1;}
            else {for ( argend=argstart=cur+2; strcmp(")", xic->cont(argend))  ; ++argend );} // scan lexic statement to the next parenthesis 

            char * srchpar=lg->script.ptr(lx->prth.Start); // this pointer runs inside the function call declaration parenthesis and picks one by one the arguments
            for ( iArg=argstart; iArg<argend  ; ++iArg){ // scan lexic statement to the next parenthesis 
                //char * curarg=xic->cont(iArg);
                if( !(xic->lexem(iArg)->status&sCalc::Lexem::fQuoted) && !strcmp(",", xic->cont(iArg)) ) // skip commas
                    continue;

                // get the formal name of the argument in the function declaration 
                var=0;
                if( lx->prth.Start!=lx->prth.End){ // this function does have a argument list declaration 
                    nam.cut(0);
                    char * nxt=sString::extractSubstring(&nam, srchpar,0, 0,"," _ ")" __,false, true );
                    var=nam.ptr();
                    sString::cleanEnds(var,0,sString_symbolsBlank,true);
                    if(strcmp(var,"..."))srchpar=nxt; // not a variable number of arguments
                    else { sprintf(var,"__%" DEC,cnt);if(!varg)varg=cnt;} // remember the ordinal of the first variable argument 
                    ++cnt; // increment the number of arguments
                }
                // if(*var)continue; // and only those arguments which have formal parameter
                
                // prepare the arguments value 
                //for( idx t=1,cmparg=argend; xic->lexem(cmparg)->status&sCalc::Lexem::fQuoted || (strcmp(",", curarg )  && strcmp(")", curarg)) ; curarg=xic->cont(cmparg) ) { // skip commas
                //    if( t && (t=xic->prep(cmparg))!=0)cmparg=t;
                //    else ++cmparg;// TODO: if prep returns a zero : somthing uncomputeable: message about the error 
                //}
                char * val=xic->data(iArg);
                scopeFun->setVar( var ? var : "__", "%s", val ? val: "__UNDEFINED" ) ;  // remember as local variable 

                #ifdef DEBUGOUT 
                    if(iArg!=argstart) lg->debugPrintf(", ");
                    lg->debugPrintf("%s = '%s' ", var , scopeFun->getVar(var) );
                #endif
            }
        }
        
        
        // set the total number of arguments and the ordinal of first hidden variable as hidden variable called __cnt __var
        scopeFun->setVar( "__cnt", "%" DEC, cnt) ;  // remember the total number of arguments 
        if(varg)scopeFun->setVar( "__var", "%" DEC, varg) ;  // remember the ordinal of first variable argument
        scopeFun->setVar( "__fun", "%s", ct) ;  // remember the total number of arguments 
            
        #ifdef DEBUGOUT 
            lg->debugPrintf(")" );
            if(!eval)lg->debugPrintf("{\n" );
            else lg->debugPrintf(";\n" );
        #endif

        lg->funLevel=funlev;

        // change the scope and execute the functions body        
        ++(lg->funLevel);
        lg->curScope->down = scopeFun;
        lg->curScope=lg->curScope->down;
        if(eval)(*eval)(lg,*pn);// built-in library support
        else lg->parse( lg->script.ptr(lx->body.Start), lx->body.End-lx->body.Start, 1 ) ;
        --(lg->funLevel);
        lg->curScope=lg->curScope->up; // restore the scope 

        // replace function by its value 
        //if(cur!=iArg)for(idx i=iArg; i>cur; --i) xic->del(i);
        if( (xic->lexem(cur+1)->status)&sCalc::Lexem::fSubComplete )--argend; // complete substatements do not have closing parenthesis 
        xic->del(cur+1,argend-cur);
        xic->data(cur,0,"%s",lg->curScope->down->getVar("__ret"));

        xic->debugPrint("HHR - AFTER");

        delete scopeFun;

        #ifdef DEBUGOUT 
            if(!eval)lg->debugPrintf("}\n" );
        #endif
    }

    // check if this is a variable name , but not for those which were function names already
    else if( ((scp=lg->curScope)->isVar(ct))!=0 || ((scp=(&lg->globalScope))->isVar(ct))!=0  ) {
        xic->data(cur,0,"%s",scp->getVar(ct));
    }

    else {
        return 0;
    }


    lit=xic->lexem(cur); // reget li because evaluations on the way hve chanced the pointer
    lit->type|=sCalc::Lexem::fString|sCalc::Lexem::fNumber;
    lit->status|=sCalc::Lexem::fReady;

    return cur+1;
}

idx sLang::expressionCompute(sStr * out, const char * phrase, idx len )
{
    if(!len && phrase) len=sLen(phrase) ; if(!len)return 1;

    sCalc calc((sCalc::CallbackFuncType)expressionCallback,this);
    calc.analyse(sCalc::fCallbackAll, out, phrase, len);
    calc.collectResult(out, 1, " ", 0, true);

    return calc.errnum();
}


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Language Synthaxis implementations
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


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
    //else if(!strcmp(nam,"pause")){if(ptr && *ptr)lg->msg->printf("%s",ptr);getch();}

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

    // check the condition
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
        Statement * lexPrev= il ? lg->dicStat.ptr(il-1) : 0 ; // we need to check the previous statement 
        if( strcmp(lg->getStat(&tmp,&lexPrev->nam,1),"if") ) // check if the previous statement was an if 
            return -1;

        #ifdef DEBUGOUT 
            tmp.cut(0);lg->getStat(&tmp,&lexPrev->prth,1);
            lg->debugPrintf("else //    if(! %s ) ", tmp.ptr());
            tmp.cut(0);
        #endif

        // check the condition on the prevous 'if' and execute current body
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
    #endif //RECOMPUTE_IF_FOR_ELSE

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
    if(lex->prth.Start)inipos=sString::searchSubstring(lg->script.ptr(lex->prth.Start),0,";" __,1,")" __,0); // look for the first ';' semicolon 
    if(inipos)nexpos=sString::searchSubstring(inipos+1,0,";" __,1,")" __,0); // look for the second ';' semicolon 
    
    if(inipos) {
        lg->parse( lg->script.ptr(lex->prth.Start),(idx )(inipos-lg->script.ptr(lex->prth.Start)),1); // execute the initial condition
        inipos++;
    }
    else { 
        inipos = lg->script.ptr(lex->prth.Start); // parenthesis starts at the beginning 
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
        // parse the body 
        lg->parse( lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start, true ) ;
        lg->curScope->contCond=0;
        if(lg->curScope->breakCond || lg->curScope->retCond || lg->exitCond)
            break;
        
        if(nexpos!=lg->script.ptr(lex->prth.End)) { // there is a loop iterator statement 
            lg->parse( nexpos+1, (idx)(lg->script.ptr(lex->prth.End)-nexpos)-1, 1 ) ; // parse the iterator statement 
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
    
    if( *lg->script.ptr(lex->body.Start-1)!='{' ){  // not a real body - a one liner 
        lg->expressionCompute(&shellscript, lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start);
        lex=lg->dicStat.ptr(il);
        sString::cleanEnds(shellscript.ptr(),shellscript.length(),sString_symbolsBlank,1);
    }else { // whole body should be executed , treat it as it is 
        shellscript.add(lg->script.ptr(lex->body.Start), lex->body.End-lex->body.Start);
        shellscript.add("\n\0",2);
    }

    // determine the execution style : shell versus exec 
    char * nam=lg->getStat(&tmp,&lex->nam,1);
    idx type = (strcmp(nam,"shell")) ? 1 : 0 ;
    tmp.cut(0);
    
    #ifdef DEBUGOUT 
        lg->debugPrintf("shell\n");
        lg->debugPrintf("{\n");
        lg->debugPrintf("%s\n",shellscript.ptr() );
        lg->debugPrintf("}\n\n----- shell-start -----\n", shellscript.ptr() );
    #endif
    
    // execution
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
    
    /*
    char olddir[vFile_PATH_MAXLEN],newdir[vFile_PATH_MAXLEN];
        sFile::getCurDir(olddir,sizeof(olddir)-1); // getcwd
    sFile::makeName(newdir, flnm, "%%" DEC "ir" );
        sFile::simplifyPath(newdir, newdir);
    */
    sFilePath olddir; olddir.curDir();
    sFilePath newdir(flnm,"%%dir");newdir.simplifyPath();
            

    #ifdef DEBUGOUT 
        lg->debugPrintf("include \"%s\" ", tmp.ptr());
    #endif
    
    // open the include file 
    sFil fil(flnm,sFil::fReadonly);
    
    if(!fil.ptr()) {
        #ifdef DEBUGOUT 
            lg->debugPrintf( "; // can't open\n " ); // cant open the makefile 
        #endif
        return -1;
    }
    
    #ifdef DEBUGOUT 
        lg->debugPrintf("\n---- include %s start -----\n", flnm);
    #endif
    
    // parse the include file 
        if(newdir.length())sDir::chDir ( newdir.ptr() ); // remember the current directory
    ++(lg->funLevel);
    lg->parse( fil.ptr(), fil.length() ) ;
    --(lg->funLevel);
    if(newdir.length())sDir::chDir( olddir.ptr() ); // restore the directory

    

    #ifdef DEBUGOUT 
        lg->debugPrintf("\n---- include %s end -----\n", flnm );
    #endif

    return 0;
}

idx sLang::exec_define(sLang * lg, idx il)
{
    Statement * lex=lg->dicStat.ptr(il);
    
    // redefine the startucture of defline 
    for ( lex->nam.Start=lex->nam.End ; strchr(sString_symbolsSpace,*lg->script.ptr(lex->nam.Start)) ; ++(lex->nam.Start) ) ; // find the first non space after #define 
    for ( lex->nam.End=lex->nam.Start; !strchr(sString_symbolsBlank,*lg->script.ptr(lex->nam.End)) ; ++(lex->nam.End) ) ;// find the next space as the end of the name 
    lex->body.Start=lex->nam.End;
    lex->body.End=lex->stat.End;

    return exec_declarefun(lg, il);
}




// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Core functions definition
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#define V(_v_nam)           (lg->curScope->getVar(_v_nam))

//"eval( txt ){}" 
idx sLang::exec_eval(sLang * lg, idx )
{
    const char * txt=V("txt");
    sStr rslt; 
    if(txt)lg->expressionCompute(&rslt, txt, (idx)strlen(txt));
    lg->curScope->setVar("__ret","%s",rslt.ptr());

    return 0;
}

//"parse( txt ){}" 
idx sLang::exec_parse(sLang * lg, idx )
{
    const char * txt=V("txt");
    sStr rslt; 
    if(txt){lg->parse(txt,(idx)strlen(txt),0);}

    return 0;
}

// "isvar ( nam ) {}"
idx sLang::exec_isvar(sLang * lg, idx )
{
    lg->curScope->setVar("__ret","%d", lg->curScope->isVar(V("nam")));
    return 0;
}

// "isfun ( nam ) {}"
idx sLang::exec_isfun(sLang * lg, idx )
{
    lg->curScope->setVar("__ret","%d", (lg->dicFun.get(V("nam"))  ? 1 : 0 ));
    return 0;
}




// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Declaration of the Core functionality
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


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








// debug  on|off - turns the debug mode on generally 
// debul on|off - turns the debug mode on locally

// return expression - returns from the scope with the expression value in __ret
// break - breaks loops 
// continue - jumps to the beginning of a loop
// exit expression - prints expression result and quits
// pause expression - prints the expression result and waits for a keyboard input

// if (expression ) { body } - body is executed by calling parse if the expression value is not 0
// else {}

// exec command_line - executes shell with piped execution method after computing a command_line as an expression  
// shell command_line  - executes shell using system call
// exec { commands  }- executes shell with piped execution method as is ... no expression computation for commands 
// shell { commands } - executes shell using system call
// the exec returns the stdout of computed program as __ret


// #include <filename> or "filename" - execute commands from the other file 
// #define macro ... - defined macros just as a one liner function 


// eval (expression)  - computes an expression 
// parse (expression) - executes an expression
// isvar (varname ) - determines if the variable is defined in current scope 
// isfun (funcname) - determines if a function is defined in current scope 


