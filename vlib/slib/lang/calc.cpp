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
// TODO 
// use VariableCaching 
// enable mod %
// enable ** for mantissa
//#define DEBUGOUT

#include <ctype.h>
#include <math.h>
#include <slib/std/string.hpp>
#include <slib/utils/sort.hpp>
#include <slib/std/file.hpp>
#include <slib/lang/calc.hpp>

using namespace slib;


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ data and definitions
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
enum { 
  // basics
    ePrecParenthesis=0,
    ePrecValues,
    ePrecMultiplicative,
    ePrecAdditive,
    ePrecBitwise,
    ePrecLogic,
    ePrecComparison,
    ePrecAssignment,
    ePrecQuotations,
    ePrecFunctional,
    ePrecNone,
    ePrecLast
};

const idx sCalc::_defPrecendence[]={
  // basics
    ePrecAssignment,ePrecAssignment, //":=" _ "?=" _

    // comparison operations
    ePrecComparison,ePrecComparison,// "==" _      "=" _
    ePrecComparison, //"!=" _
    ePrecComparison,ePrecComparison,// ">=" _      ">" _      
    ePrecComparison,ePrecComparison, // "<=" _      "<" _

    // logic operations
    ePrecLogic, // "&&" _  // " and " _
    ePrecLogic, // "||" _ // " or " _    
    ePrecLogic,  // "!" _  // " not " _   

    // bit operations 
    ePrecBitwise, ePrecBitwise, // "<<" _      ">>" _
    ePrecBitwise, // "~" _
    ePrecBitwise, // "&" _
    ePrecBitwise, // "|" _
    ePrecBitwise, // "^" _
    
    // math operations 
    ePrecAdditive, // "+" _
    ePrecAdditive, // "-" _
    ePrecMultiplicative, // "*" _
    ePrecMultiplicative, // "/" _
    ePrecMultiplicative, // "%" _
    
    
    // execution level operations
    ePrecParenthesis,ePrecParenthesis, // "(" _       ")" _
    //ePrecParenthesis,ePrecParenthesis, // "[" _       "]" _
    ePrecParenthesis,ePrecParenthesis, // "{" _       "}" _


    // syntaxical level 
    ePrecQuotations,ePrecQuotations, // "\"" _          "'" _
    ePrecFunctional,ePrecFunctional, // "," _           ";" _
        

    ePrecNone, // " " __;
};

const char * sCalc::_defaultTokenizingSymbols=
    // basics
    ":=" _ "?=" _

    // comparison operations
    "==" _      "=" _
    "!=" _
    ">=" _      ">" _      
    "<=" _      "<" _

    // logic operations
    "&&" _ // " and " _   
    "||" _ // " or " _    
    "!" _  // " not " _   

    // bit operations 
    "<<" _      ">>" _
    "~" _
    "&" _
    "|" _
    "^" _
    
    // math operations 
    "+" _
    "-" _
    "*" _
    "/" _
    "%" _
    
    
    // execution level operations
    "(" _       ")" _
    //"[" _       "]" _
    "{" _       "}" _


    // syntaxical level 
    "\"" _          "'" _
    "," _           ";" _
        

    " " __;

const char * sCalc::_defaultNonTokenizingSymbols=0;


sVar sCalc::varGlobal;

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ basic functions
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx sCalc::_tokenize(const char * phrase, idx phraselen)
{
    // first we digest the input to our own buffer 
    _expression.cut(0);
    sString::searchAndReplaceSymbols(&_expression, phrase,phraselen, sString_symbolsBlank, " ",0,true,true,true);
    char * ptr0=_expression.ptr(), * dig, *next, *ptrsrch , * ptr;
    
    _mex.add(__,1);
    idx i,nym;
    bool quot, sub;
    Lexem * it;
    // now scan the string 
    for(ptr=ptrsrch=ptr0 , i=0;  ptr && *ptr ;  ++i)
    {
        // for every position  ptr try finding the end of the lexem by tokenizing substrings 
        // see if this position is one of our tokenizing symbol sets
        nym=sNotIdx;
        idx pos=sString::compareChoice( ptrsrch, _lgs, &nym,(flags&fCaseSensitive) ? true : false ,0);
        dig=0;quot=false;sub=false;

        if(pos!=sNotIdx){ // if this position is a tokenizing string 
            next=ptr+pos; // put the "next" pointer to the position after tokenizer ends 
            if(*ptr==' ')ptr++; // a single space is taken as a special tokenizer to break the lexems
        }
        else { // not a tokenizing position 
            if(*ptr==' ')ptr++; // a single space is taken as a special tokenizer to break the lexems
            next=ptrsrch;  // put the "next" pointer here 
            while( next ) { // now try finding the next token 
                next=sString::searchSubstring(next,0,_lgs,1,0,flags&fCaseSensitive ? true : false );
                if(next) { // if there is a next token 
                    if(next>ptrsrch && *(next-1)=='\\') { // slashed symbol is not tokenizing 
                        strcpy(next-1,next); // skip the slash 
                    }else if( _lgsnon && sString::compareChoice(next,_lgsnon,&nym,(flags&fCaseSensitive) ? true : false ,0)!=sNotIdx){ // do not break on non tokenizing symbols
                        next+=(idx )strlen((sString::next00(_lgsnon,nym))); // skip the non tokenizing symbols 
                    }else break;
                }
                else break;
            }
            if(!next)next=ptr+strlen(ptr);

        }
                
        // filter +- signes in exponentioal numbers, put the dig into the start of the number 
        if( next>ptr0 && (*next=='-' || *next=='+') && (*(next-1)=='E' || *(next-1)=='e') ) {
            for(dig=next-2 ; dig>=ptr && isdigit((unsigned)(*dig)) ; --dig );
            if(dig!=next-2) // this minus is a part of the number
                {ptrsrch=next+1;continue;}
        }

        // run through quotations
        if( *ptr=='\"' || *ptr=='\'') {
            for(dig=ptr+1 ; *dig && *dig!=*ptr; ++dig );
            ++ptr; next=dig;quot=true;
        }
        else if( *ptr=='{' ) {
            for(dig=ptr+1 ; *dig && *dig!='}'; ++dig );
            ++ptr; next=dig;sub=true;
        }

        if(next!=ptr || quot ){ // add this lexem into our script buffer 
            
            idx len=(idx)(next -ptr);//(idx)((*next==' ' ?  next-1 : next ) -ptr);
            idx contofs=_mex.add(ptr, len+2); // terminate with double zeros
            char * dst=(char *)_mex.ptr(contofs);
            if( !quot && dst[len-1]==' ')dst[len-1]=0;
            dst[len]=0;dst[len+1]=0;
            
            it=_lxit.add(1);
            it->content=contofs;
            it->data=0;
            it->status=0;
            it->type=sub ? Lexem::fSubroutine : 0 ;
            it->precendence = nym!=sNotIdx ? _lgprec [nym] : ePrecValues ;
            if( dig && (*dig=='\"' || *dig=='\'' || *dig=='{' || *dig=='}')){
                it->status|=Lexem::fQuoted;
                sString::searchAndReplaceStrings(dst,0,"\\n" _ "\\r" _ "\\t" __,"\n" _ "\r" _ "\t" __,0,false);
            }
        }
        if( dig && (*dig=='\"' || *dig=='\'' || *dig=='{' || *dig=='}')) {
            next++;
        }

        ptr=next;
        ptrsrch=next;
    }

    return _lxit.dim();
}


idx sCalc::prep(idx cur)
{
    //if( cur==sNotIdx )return 1;
    idx res=0;
    if(_lxit.ptr(cur)->status&Lexem::fReady) 
        return cur+1 ;
    if(_whattodo&fCallbackFirst && _func)
        res=_func( _funcparam, _par, this, cur, _lxit.dim() , fCallbackFirst);

    if(!res && _whattodo&fCallGeneric)
        res=genericCallback(cur,fCallGeneric);
    if(!res && _whattodo&fCallVariables )
        res=variablesCallback(cur, fCallVariables);
    if(!res && _whattodo&fCallMath )
        res=mathCallback(cur, fCallMath, 0 );
    if(!res && _whattodo&fCallMathFunc)
        res=mathFuncCallback(cur,fCallMathFunc);
    if(!res && _whattodo&fCallText)
        res=textCallback(cur, fCallText );


    /*
    if(!res && _whattodo&fCallMath ){
        for( idx iprec=0; iprec<eMathLast; ++iprec){
            if( (res=mathCallback(cur, fCallMath, iprec) )!=0  )
                break;
        }
    }
    */

    if(!res && _whattodo&fCallLogic)
        res=logicCallback(cur,fCallLogic);
    if(!res && _whattodo&fCallbackLast && _func)
        res=_func( _funcparam, _par, this, cur, _lxit.dim() , fCallbackLast);
    return res;    
}


char * sCalc::_analyse(void)
{
    #ifdef DEBUGOUT
        if(subLevel==0) sFile::remove("debug.log");
        {
            sFil str("debug.log");
            if(subLevel)for(idx k=0; k<subLevel; ++k) str.printf("    ");str.printf("\nORIGINAL STATMENT\n");
            if(subLevel)for(idx k=0; k<subLevel; ++k) str.printf("    ");str.printf("%s\n\n",_expression.ptr());
                
        }
    #endif

    for(idx i=0, ido=0; ido!=sNotIdx; ){
        #ifdef DEBUGOUT
            debugPrint();
        #endif

        ido=sNotIdx;// chose the element with most precendence from those which are not done yet 
        for ( idx k=0; k<_lxit.dim(); ++k){if( _lxit.ptr(k)->status&Lexem::fReady ) continue;
            if( ido==sNotIdx || _lxit.ptr(k)->precendence < _lxit.ptr(ido)->precendence) ido=k;
        }if(ido==sNotIdx)break; // all done 
        
        #ifdef DEBUGOUT
            char * ct=cont(ido);
        #endif

        idx nx=prep(ido);
        if(!nx) {
            // error - something cannot be executed 
            data(ido,0,"ERR(%s)",cont(ido));
            break;
        }
        if(nx==sNotIdx)i=0;
        //else if(nx)i=nx;
        else ++i;
    }
    return data(0);
}

idx sCalc::errnum(void)
{   
    if (_lxit.dim()==1 && (_lxit.ptr(0)->status&Lexem::fReady) ) 
        return 0;
    for ( idx i=0; i<_lxit.dim(); ++i) 
        if( !(_lxit.ptr(i)->status&Lexem::fReady) )
            return i+1;
    return 0;
}


// if pdata  specified - changes the content/data otherwise retrieves it  
char * sCalc::_data(idx isda, idx num, idx siz , const char * pdata, va_list ap )
{   
    if(!_lxit.dim())return 0;
    if(num==sNotIdx && !pdata) { 
        return sString::nonconst("0");
    }

    Lexem * lx=_lxit.ptr(num);
    if(!pdata) {
        idx ofs = isda ? lx->data : lx->content; 
        {return ofs ? (char *)(_mex.ptr(ofs)) : 0 ;}
    }
        
    sStr buf;
    if(lx->type&Lexem::fBinary)buf.add((const char*)pdata,siz);
    else buf.vprintf((const char*)pdata,ap);

    if(!siz) siz =sLen(buf.ptr());
    if(isda){
        lx->datasize = siz;
        lx->data=_mex.add(buf.ptr(),siz+2);
    }
    else lx->content=_mex.add(buf.ptr(),siz+2);
    char * ppp=(char *)(_mex.ptr(isda ? lx->data : lx->content));
    ppp[siz]=0;ppp[siz+1]=0;
    return ppp;
}


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ standard callbacks 
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#define mNeed1(_v_num, _v_typ)  if( !prep(_v_num) || ((_lxit[_v_num].type) && !(_lxit[_v_num].type&(_v_typ))) )return 0;
#define mNeed2(_v_num1,_v_typ1,_v_num2,_v_typ2) if( !prep(_v_num1) || !prep(_v_num2) || ((_lxit[_v_num1].type) && !(_lxit[_v_num1].type&(_v_typ1))) || ((_lxit[_v_num2].type) && !(_lxit[_v_num2].type&(_v_typ2))))return 0;

idx sCalc::subCalc( idx cur, idx end, const char * finish00 )
{
    sStr sub,res;
    idx i;
    if(end==sNotIdx)end=_lxit.dim();
    for(i=cur; i<end; ++i ) {
        char * nct=cont(i);
        if(finish00 && sString::compareChoice(nct,finish00,0,false,0,true)!=sNotIdx)break; // if end has been reached 
        sub.printf((_lxit(i)->status&Lexem::fQuoted) ? "'%s' " : "%s ",nct);
    }
    if(finish00)--end; // so we do not "compute" the condition of stopping  ) , etc
    if(!sub.length())return 0;
    
    sCalc subcalc(_func, _funcparam,_lgs,_lgsnon,_lgprec);
    subcalc.subLevel=subLevel+1;
    subcalc.setVariableSpace(variablesGlobal,varPrfxGlobal,variablesTemporary,varPrfxTemporary);
    subcalc.analyse(_whattodo,_par,sub.ptr(),0);
    //if(subcalc.errnum())return 0;

    char * ptr, * ctr; idx k;
    for(k=0; k < subcalc.dim() ; ++k ){
        ptr=subcalc.data(k);
        ctr=subcalc.cont(k);
        cont(cur+k, 0, "%s", ctr ? ctr : "");
        data(cur+k, 0, "%s", ptr ? ptr : "" );
        _lxit[cur+k].status|=subcalc._lxit[k].status;
        //    res.printf("%s ", ptr);
    }
    
    --end; del(cur+k,(end-cur)-k+1);


//    debugPrint("comming out of subCalc");
    return k;
}

idx sCalc::genericCallback(idx cur, idx )
{
    char * ct=cont(cur);
    idx lev;
    idx i;

    ///if( (!strcmp(ct,"(")) || !strcmp(ct,"{") || !strcmp(ct,"[") ){
    if( (!strcmp(ct,"(")) || !strcmp(ct,"{") ){
        sStr sub,res;

        char braceOut[2];braceOut[1]=0;
        if(ct[0]=='(' )braceOut[0]=')';
        ///else if(ct[0]=='[' )braceOut[0]=']';
        else braceOut[0]='}';
        if((idx )cur+1>=_lxit.dim()) return 0;
        for(lev=1, i=cur+1; i<(idx)_lxit.dim(); ++i ) {
            char * nct=cont(i);
            if( !strcmp(nct,braceOut))lev--;//break;
            if( !strcmp(nct,ct))lev++;
            if( !lev )break;
            //sub.printf((_lxit(i)->status&Lexem::fQuoted) ? "'%s' " : "%s ",nct);
        }
        if(lev==0){ // the number of parenthesis is ok? 
            idx cnt=subCalc(cur+1, i, 0);
            if(cnt==1){
                _lxit[cur].type=_lxit[cur+1].type;
                _lxit[cur].status|=Lexem::fSubComplete;
                cont(cur,0,"( %s )",cont(cur+1) );
                data(cur,0,"%s", data(cur+1));
                del(cur+1,2);
            } 
            else _lxit[cur+cnt+1].status|=Lexem::fReady;
        }
        else { // not equal number of opening and closing statements
            cont(cur,0,"#()",sub.length() ? sub.ptr() : "");
            return 0;
        }
            
    }
    else if( (!strcmp(ct,"?=")) ){
        prep(cur+1);
        prep(cur+2);
        prep(cur+3);
        real val=0; 
        char * ptr=data(cur+1);
        if(ptr)sscanf(ptr,"%lf",&val);
        if(val) i=cur+2;
        else i=cur+3;
        _lxit[cur].type=_lxit[i].type;
        cont(cur+1,0,"%s", cont(i) );
        data(cur+1,0,"%s", data(i) );
        del(cur+2,2);
        cont(cur,0,":=");
//        return cur+1;
    }
    else if( (!strcmp(ct,",")) ){
        data(cur,0,"," );
    }
    else return 0;
    
    _lxit[cur].status|=Lexem::fReady;
    return cur+1;
}



const char * sCalc::varValue(const char * ct, const char * val) 
{
    sVar * varRem=0; const char * varPrfx=0;

    if( ct[0]==':' && ct[1]!=':' && variablesTemporary){varRem=variablesTemporary; varPrfx=varPrfxTemporary;}
    else if(variablesGlobal) {varRem=variablesGlobal; varPrfx=varPrfxGlobal;}
    if(!varRem)return 0;

    if( ct[0]==':' && ct[1]==':') varPrfx="";
    if(!varPrfx)varPrfx="";

    if(!val)
        val=(char *)(varRem->outf("%s%s", varPrfx ,ct)); // get the variable 
    else 
        varRem->inpf( val ,0 , "%s%s", varPrfx , ct ); // temporary variable 
    return val;
}

idx sCalc::variablesCallback(idx cur, idx )
{
    char * ct=cont(cur);
    const char * pp;
    //sVar * varRem=0; const char * varPrfx=0;

    if( cur+2<dim() && (!strcmp(cont(cur+1),":=")) ) { // if this is an alignment within expression ... execute it 
        prep(cur+2);// prepare the lexem next to equal sign 
        pp =data(cur+2); 
        if(!pp)pp=cont(cur+2); // we still sometimes maintain the cont - for assigning the rvalue ranges which are not computeable at this point 
        if(!pp)pp="";

        varValue(ct, pp);

        _lxit[cur].type= _lxit[cur+2].type;
        _lxit[cur].status|=Lexem::fReady; // mark as done 
        data(cur,0,pp);
        cont(cur,0,cont(cur+2));

        del(cur+2);del(cur+1);
        //del(cur);
        
        return cur+1; // this way we force to reevaluate the lexems: there might have been some which now are computable 

    } else if( !isdigit(ct[0]) ) { // is this a global or local variable inquiry (not a assingment ) ? 

        pp=varValue(ct); 

        if(pp){ 
            data(cur,0,"%s",pp);
            _lxit[cur].type=Lexem::fNumber|Lexem::fString;
            _lxit[cur].status|=Lexem::fReady;
            return cur+1;
        }
    }

    return 0;
}




const char * sCalc::symbText="equal" _ "equal" _ "inequal" _ "hasnt" _ "has" _ "concat" _ "after" _ "before" _ "repeat" _ "permute" _ "subrand" __;
                             // 0     1    2     3      4       5
idx sCalc::textCallback(idx cur, idx )
{
    char * ct=cont(cur);
    idx res=0;
        
    //_/_/_/_/_/_/_/_/_/
    //_/
    //_/ quoted strings
    //_/
    enum { eDoubleEqual=0, eEqual, eNotEqual, eHasNot, eHas,ePlus, eAfter, eBefore, eRepeat, ePermute, eSubRandom};

        
    if(_lxit(cur)->status&Lexem::fQuoted){
        data(cur,0, "%s",ct);
        _lxit[cur].status|=Lexem::fReady;
        _lxit[cur].type=Lexem::fString;
        return cur+1;
    }
    
    // choices
    idx num=sNotIdx;
    //const char * lchoices="==" _ "=" _ "!=" _ "hasnt" _ "has" _ "+" __;
                         //  0      1      2       3        4      5
    // find what to do 
    idx fndLen=sString::compareChoice(ct,symbText,&num,1,0);
    if( fndLen==sNotIdx )
        return 0;

    //_/_/_/_/_/_/_/_/_/
    //_/
    //_/ binary operations 
    //_/
    if(num<=eBefore) {
        if(cur==0)return 0;
        mNeed2(cur-1, Lexem::fString|Lexem::fNumber, cur+1, Lexem::fString|Lexem::fNumber);
        char * p1=data(cur-1), * p2=data(cur+1);
        char * posS=p1, * posE=p1+strlen(p1);

        switch( num ) {
            case eDoubleEqual:case eEqual:  res=strcmp(p1, p2) ? 0 : 1 ; break; // = ==
            case eNotEqual: res=strcmp(p1, p2) ? 1 : 0 ; break; // !=
            case eHasNot: res=strstr(p1,p2) ? 0 : 1 ;break; // hasnt
            case eHas: res=strstr(p1,p2) ? 1 : 0 ;break; // has 
            case eAfter: posS=strstr(p1,p2) ; if(posS) posS+=strlen(p2); else posS=posE; break; // after 
            case eBefore: posE=strstr(p1,p2)  ;if(!posE) posE=posS;break; // has         
            case ePlus: cont(cur,0,"");break; // has         
            
            default: break;
        }
        
        cont(cur-1,0,"%s %s %s",cont(cur-1),cont(cur),cont(cur+1)); //concatenate the content 
        del(cur+1);del(cur);--cur; // delete the last two 
        if(num<=eHas) {
            data(cur,0,"%" DEC,res); // set the result 
            _lxit[cur].type= Lexem::fLogical; // the result type 
            _lxit[cur].status|=Lexem::fReady; // mark as done 
            return cur+1;
        }
        /* // don't remember why did I block this '+', perhaps was interfering with math + 
        else if(num==7){
            data(cur,0,"%s%s",p1,p2); // set the result 
            _lxit[cur].type= Lexem::fString; // the result type 
        }*/
        else if(num<=ePlus) { // +
            data(cur,0,"%s%s", p1,p2);
            return cur+1;
        }
        else if(num<=eBefore) { // after, before 
            data(cur,(idx )(posE-posS),"%s",posS); // set the result 
            _lxit[cur].type= Lexem::fString; // the result type 
            _lxit[cur].status|=Lexem::fReady; // mark as done 
            return cur+1;
        }
    }

    //_/_/_/_/_/_/_/_/_/
    //_/
    //_/ unary operations 
    //_/
    mNeed1(cur+1,Lexem::fString|Lexem::fNumber);
    idx funval=(idx)atoi(ct+fndLen);if(!funval)funval=1;
    sStr out, tmp;
    char * p =data(cur+1);
    if(num==eRepeat){ 
        // requres number after them, read that  
        for(idx ir=0; ir< funval; ++ir)  // if( !strcmp(",", cont(ia) ))continue;
            out.printf("%s%s", ir ? " " : "" , p);
    }else if(num==ePermute || num==eSubRandom){ 
        // prepare the list of pointers to items 
        sVec< char * > items;
        for( char * pc=p; pc ; pc=sString::skipWords(pc,sLen(pc),1,sString_symbolsBlank) ) // prepare the value 
            *items.add()=pc;
        idx rcnt=items.dim();
        //char dsttmp[vFile_TEXTLINE_MAXLEN];
        if(num==ePermute)funval=rcnt;
        for(idx ir=0; ir<funval; ++ir) {
            double rnum=1.*rand()/RAND_MAX;
            idx io=(idx)(rcnt*rnum);if(io>=rcnt)io=rcnt-1;
            char * pc=items[io];
            if(num==ePermute){items.del(io);--rcnt;}
            sString::copyUntil(&tmp,pc,0,sString_symbolsBlank);
            out.printf("%s%s", ir ? " " : "" , tmp.ptr());
            tmp.cut(0);
        }
    }
    data(cur,out.length(),"%s",out.ptr());
    out.cut(0);out.printf("%s %s",cont(cur),cont(cur+1));
    cont(cur,0,"%s",out.ptr()); //concatenate the content 
    del(cur+1); // delete the last two 
        
    _lxit[cur].type= Lexem::fString|Lexem::fNumber; // the result type 
    _lxit[cur].status|=Lexem::fReady; // mark as done 
    return cur+1;
}

const char * sCalc::symbLogic="&&" _ "and" _ "||" _ "or" _ "!" _ "not" __;
                             // 0     1      2        3     4      5

idx sCalc::logicCallback(idx cur, idx )
{
    char * ct=cont(cur);
    idx res=0;
    idx num=sNotIdx;
//    const char * lchoices="&&" _ "and" _ "||" _ "or" _ "!" _ "not" __;
                                // 0     1      2     3     4    5
    
    // find what operation it is 
        if( sString::compareChoice(ct,symbLogic,&num,1,0)==(idx)(-1) )
                return 0;

    //_/_/_/_/_/_/_/_/_/
    //_/
    //_/ unary operations 
    //_/
    if(num==4 || num==5)  { // ! not
        mNeed1(cur+1,Lexem::fLogical);
        char * p=data(cur+1);
        res= ( !p || atoi( p )!=0 ) ? 0 : 1;
        cont(cur,0,"%s %s",cont(cur),cont(cur+1));
        del(cur+1);
        data(cur,0,"%" DEC,res); // set the result 
        _lxit[cur].status|=Lexem::fReady;
        _lxit[cur].type=Lexem::fLogical;
        return cur+1;
    }

    //_/_/_/_/_/_/_/_/_/
    //_/
    //_/ unary operations 
    //_/
    if(cur==0)return 0;
    mNeed2(cur-1, Lexem::fLogical, cur+1, Lexem::fLogical);
    char * p1=data(cur-1), *  p2=data(cur+1);
    idx l1 = p1 ? atoi( p1 ) : 0  , l2 = p2 ? atoi( p2 ) : 0;


    switch(num){
        case 0: case 1: res= (l1 && l2) ? 1 : 0; break;
        case 2: case 3: res= (l1 || l2) ? 1 : 0; break;
        default: break;
    }

    // set the lower item and remove two following
    cont(cur-1,0,"%s %s %s",cont(cur-1),cont(cur),cont(cur+1)); //concatenate the content 
    del(cur+1);del(cur);--cur; // delete the last two 
    data(cur,0,"%" DEC,res); // set the result 
    _lxit[cur].status|=Lexem::fReady; // mark as done 
    _lxit[cur].type=Lexem::fLogical; // the result type 
    return cur+1;
}

const char * sCalc::symbMath="~" _ "+" _ "-" _ "*" _ "/" _ "%" _ "==" _ "=" _ "!=" _ ">=" _ ">" _ "<=" _ "<" _ "<<" _ ">>" _ "&" _ "|" _ "^" __;
                            // 0     1     2     3     4     5     6     7     8      9      10    11     12    13     14     15    16    17

idx sCalc::mathCallback(idx cur, idx , idx stage) // whattodo
{
    char * ct=cont(cur);
    double res=0;
    idx num=sNotIdx;
    idx wrk=0;
    stage =eMathLast;

//    const char * lchoices="~" _ "+" _ "-" _ "*" _ "/" _ "%" _ "==" _ "=" _ "!=" _ ">=" _ ">" _ "<=" _ "<" _ "<<" _ ">>" _ "&" _ "|" _ "^" __;
                                // 0     1     2     3     4     5     6     7     8      9      10    11     12    13     14     15    16    17

    // quoted ... something we do deal with math 
    if ( _lxit[cur].status&Lexem::fQuoted ) {
        return 0;
    }    
    
    // find what operation it is 
    if( sString::compareChoice(ct,symbMath,&num,1,0)==sNotIdx ) {
        if(ct[0]=='0' && (ct[1]=='x' || ct[1]=='X') ){ // see if this is a hexadecimal ?  && strchr("0123456789ABCDEFabcdef",ct[2]) 
            idx hexVal=0;wrk=sscanf(ct+2,"%" HEX,&hexVal);
            res=(double)hexVal;
        }
        else {
            idx is; // check if this is a number 
            for (is=0; ct[is] ; ++is)if( !strchr("0123456789eE-+.",ct[is])) break;
            if(ct[is] || (wrk=sscanf(ct,"%lf",&res))==0) // not a number , can not be scanned or has no digit characters
                return 0;
        }
    }

    // Es sirum em Valerain 
    //_/_/_/_/_/_/_/_/_/
    //_/
    //_/ unary operations and numbers 
    //_/
    if(stage>=eMathNumerical){
        bool isunary=false;
        if(num==0) isunary=true;
        char * pct;
        if(num==1 || num==2 ) {
            if(cur!=0) {
                pct=cont(cur-1);
                //if(pct[1]==0 && (pct[0]=='(' || pct[0]=='[' ||pct[0]==',') )
                if(pct[1]==0 && (pct[0]=='(' || pct[0]==',' ) )
                    isunary=true; 
            }else 
                isunary=true; // together with unary - + 
        }

        if(isunary || num==sNotIdx ){
            if(num==0 || num==1 || num==2 ) { // unary negation
                mNeed1(cur+1, Lexem::fNumber);
                char * p=data(cur+1) ; 
                switch(num){
                    case 0: res= ~ (p ? atoi( p ) : 0 );break;
                    case 1: res= res;break;
                    case 2: res= -res;break;
                }
                cont(cur,0,"%s %s",cont(cur),cont(cur+1));
                del(cur+1);
            }
            
            if(cur==1){ // check for unary + or - sign 
                pct=cont(cur-1);
                if( pct[1]==0 && (pct[0]=='-' || pct[0]=='+') ){
                    if(pct[0]=='-')res=-res;
                    cont(cur-1,0,"%s%s",pct,cont(cur));
                    del(cur);cur--;
                }
                
            }
            data(cur,0,"%lg",res);
            _lxit[cur].status|=Lexem::fReady;
            _lxit[cur].type=Lexem::fNumber;
            return cur+1;
        }
    }

    //_/_/_/_/_/_/_/_/_/
    //_/
        //_/ binary operations 
    //_/
    idx type=Lexem::fLogical;

    if(cur==0)return 0;
    mNeed2(cur-1, Lexem::fNumber, cur+1, Lexem::fNumber);
    
    // arythmetics operations
    //double r1=atoi( data(cur-1) ), r2=atoi( data(cur+1) );
    const char * p1=cur>0 ? data(cur-1) : "0";
    const char * p2=data(cur+1);
    real r1=p1 ? atof( p1 ) : 0, r2=p2 ? atof( p2 ) : 0 ;
    
    if(stage>=eMathMultiplicative){
        switch ( num ) {
            case 3: res= r1*r2; wrk=1;break;
            case 4: res= r1/r2; wrk=1;break; 
            case 5: res= (real)(((idx)r1)%((idx)r2)); wrk=1;break; 
            default: break;
        }
    }
    
    if(stage>=eMathAdditive){
        switch ( num ) {
            case 1: res= r1+r2; wrk=1;break;
            case 2: res= r1-r2; wrk=1;break;
            default: break;
        }
    }

    // comparison 
    if(stage>=eMathComparative){
        switch ( num ) {
            case  6:case 7: res= (r1==r2) ? 1 : 0 ; wrk=1;break;
            case  8: res= (r1!=r2) ? 1 : 0 ; wrk=1;break;
            case  9: res= (r1>=r2) ? 1 : 0 ; wrk=1;break;
            case 10: res= (r1>r2) ? 1 : 0 ; wrk=1;break;
            case 11: res= (r1<=r2) ? 1 : 0 ; wrk=1;break;
            case 12: res= (r1<r2) ? 1 : 0 ; wrk=1;break;
            default: type=Lexem::fNumber; break; // if it does not produce logical result it produces a number 
        }
    }

    // bit operations 
    idx i1=(idx)r1, i2=(idx)r2;
    if(stage>=eMathBitwise) { 
        switch ( num ) {
            case 13: res= (double)(((idx64)i1)<<i2); wrk=1;break;
            case 14: res= (real)(i1>>i2); wrk=1;break;
            case 15: res= (real)(i1&i2); wrk=1;break;
            case 16: res= (real)(i1|i2); wrk=1;break;
            case 17: res= (real)(i1^i2); wrk=1;break;
            default: break;
        }
    }
    if(wrk==0)
        return 0;

    // set the lower item and remove two following
    cont(cur-1,0,"%s %s %s",cont(cur-1),cont(cur),cont(cur+1)); //concatenate the content 
    del(cur+1);del(cur);--cur; // delete the last two 
    if(floor(res)==res)data(cur,0,"%i",(idx)res); // set the result 
    else data(cur,0,"%lf",res); // set the result 
    _lxit[cur].status|=Lexem::fReady; // mark as done 
    _lxit[cur].type=type; // the result type 
    return cur+1;
}



const char * sCalc::symbMathFunc="lg10" _ "lg2" _ "ln" _ "abs" _
    "plus" _ "minus" _ "neg" _ "mult" _ "div" _ "inv" _ "pow" _ "exp" _ "log" _
    "cos" _ "acos" _ "sin" _ "asin" _ "tan" _ "atan" _ "floor" _ "ceiling" _
    "sum" _ "min" _ "max" _ "sigma" _ "RMS" _ "mean" _
        "irand" _ "rand" _
    __;

idx sCalc::mathFuncCallback(idx cur, idx )
{
    char * ct=cont(cur);
    idx num=sNotIdx;

    enum {
        fLg10=0,fLg2,fLn,fAbs,
        fPlus,fMinus,fNeg,fMult,fDiv,fInv,fPow,fExp,fLog,
        fCos,fACos,fSin,fASin,fTan,fATan,fFloor,fCeiling,
        fSum,fMin,fMax,fSigma,fRMS,fMean,
            fIRandom,fRandom,
        fNone
    };

    // find the name of the function
    idx fndLen=sString::compareChoice(ct,symbMathFunc,&num,1,fLg10);
    if( fndLen==sNotIdx || cur>=dim()  || *cont(cur+1)!='(' ) // functions must be followed by parenthesis 
        return 0;

        // some functions requre number after them, read that  
    real funval=1;sscanf(ct+fndLen,"%lf",&funval);
//    mNeed1(cur+1, Lexem::fNumber);
    
    // prepare all arguments 
    idx argstart=0,argend=0,cntarg=0;
    // scan arguments in the function call and in function declaration
    if((_lxit[cur+1].status)&Lexem::fSubComplete)
        {argstart=cur+1; argend=argstart+1;}
    else {
        for ( argend=argstart=cur+2; strcmp(")", cont(argend))  ;){ // scan lexic statement to the next parenthesis 
            if( !strcmp(",", cont(argend)) ) { ++argend;continue; }// skip commas
            argend=prep(argend);// prepare the arguments value 
        }
    }
    sStr out;
    idx ia,io;
    real outval=0,val=0;
        
    // numeric accumulator based functions 
    if(num>=fSum && num<=fMean) {
        real outval=0,val=0;
        idx ia;
        if(num==fMin)outval=REAL_MAX;
        else if(num==fMax)outval=-REAL_MAX;
        else outval=0;

        for ( ia=argstart; ia<argend  ; ++ia){if( !strcmp(",", cont(ia) ))continue;
            for( char * pc=data(ia); pc ; pc=sString::skipWords(pc,sLen(pc),1,sString_symbolsBlank) ){ // prepare the value 
                sscanf(pc,"%lf",&val);++cntarg;

                switch ( num ) {
                    case fSigma:                        
                    case fSum:
                    case fMean:
                    case fRMS: 
                        outval+=(num==fRMS ? val*val : val);break;
                    case fMin:if(outval>val)outval=val;break;
                    case fMax:if(outval<val)outval=val;break;
                        
                    default: break;
                }   
            }
        }
        if(num!=fSum && num!=fMin && num!=fMax)
            outval/=cntarg;
        if(num==fSigma) {
            real mean=outval;
            for ( outval=0, ia=argstart; ia<argend  ; ++ia){if( !strcmp(",", cont(ia) ))continue;
                for( char * pc=data(ia); pc ; pc=sString::skipWords(pc,sLen(pc),1,sString_symbolsBlank) ){ // prepare the value 
                    sscanf(pc,"%lf",&val);
                    outval+= (val-mean)*(val-mean);
                }
            }
            outval/=cntarg;
        }
        if(num==fSigma || num==fRMS)
            outval=sqrt(outval);
        out.printf("%lf",outval);

    }else if(num>=fIRandom && num<=fRandom){ 
        real rmin=REAL_MAX, rmax=-REAL_MAX;
        for ( ia=argstart; ia<argend  ; ++ia){if( !strcmp(",", cont(ia) ))continue;
            for( char * pc=data(ia); pc ; pc=sString::skipWords(pc,sLen(pc),1,sString_symbolsBlank) ){ // prepare the value 
                sscanf(pc,"%lf",&val);
                rmin=sMin(rmin,val);
                rmax=sMax(rmax,val);
            }
        }
        for(idx ir=0; ir< funval; ++ir) {
                outval=(rmax-rmin+(num==fIRandom ? 1 : 0 ))*rand()/RAND_MAX+rmin;
                //if(num==fIRandom)out.printf("%s"DEC, ir ? " " : "" , (idx)(outval>rmax  ? rmax : outval));
                if(num==fIRandom)out.printf("%s%" DEC, ir ? " " : "" , (idx)(outval>rmax  ? rmax : outval));
                else out.printf("%s""%lf", ir ? " " : "" , outval);

        }
    }
    else { // if(num<fMean) // math functions
        for (  io=0,ia=argstart; ia<argend  ; ++ia){if( !strcmp(",", cont(ia) ))continue;
                                
            for( char * pc=data(ia); pc ; pc=sString::skipWords(pc,sLen(pc),1,sString_symbolsBlank) ){ // prepare the value 
                sscanf(pc,"%lf",&val) ;
                real excoef=1.;
                switch ( num ) {
                    case fLg10:excoef*=(real)(log(2.)/log(10.));
                    case fLg2:excoef*=(real)(1./log(2.));
                    case fLn: outval = log( val )*excoef ;break;
                                    case fAbs: outval = fabs( val ) ;break;
                    case fPlus: outval = val + funval ;break;
                    case fMinus:outval = val - funval ;break;
                    case fNeg:outval = funval - val ;break;
                    case fMult:outval = val * funval ;break;
                    case fDiv:outval = val / funval ;break;
                    case fInv:outval = funval / val;break;
                    case fPow:outval = pow(val,funval);break;
                    case fExp:outval = pow(funval,val);break;
                    case fLog:outval = (real)(1./log(funval))*log(val);break;
                    case fCos:outval = cos(val);break;
                    case fACos:outval = acos(val);break;
                    case fSin:outval = sin(val);break;
                    case fASin:outval = asin(val);break;
                    case fTan:outval = tan(val);break;
                    case fATan:outval = atan(val);break;
                                        case fCeiling:outval = sMin(val, funval);break;
                                        case fFloor:outval = sMax(val, funval );break;
                    default: break;
                }
                ++io;
                out.printf("%s""%lf", io ? " " : "" , outval);
            }
        }

    }

    // replace function by its value 
    if( (_lxit[cur+1].status)&Lexem::fSubComplete )--argend; // complete substatements do not have closing parenthesis 

    data(cur,out.length(),"%s",out.ptr());
    out.cut(0);for(ia=cur+1; ia<=argend; ++ia) out.printf("%s ",cont(ia));
    //if( (_lxit[cur+1].status)&Lexem::fSubComplete )cont(cur,0,"%s%s",ct,out.ptr()); //concatenate the content 
    //else 
    cont(cur,0,"%s%s",ct,out.ptr()); //concatenate the content 
    //for(ia=argend; ia>cur; --ia) del(ia);
    del(cur+1,argend-cur);
    
    // set the lower item and remove two following
    _lxit[cur].status|=Lexem::fReady; // mark as done 
    _lxit[cur].type=Lexem::fNumber; // the result type 
    return cur+1;
}


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ debug staff 
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/



char * sCalc::collectResult (sStr * out, idx autoSpace, const char * spcSymb, idx * spcCounter, bool isMissInfo)
{
    char * ptr; 
    for(idx i=0; i < dim() ; ++i ){
        if( (ptr=data(i))!=0 || (isMissInfo && (ptr=cont(i))!=0)) {
            const char * spc = "";
            if( autoSpace<0  )
                spc=i ? spcSymb : "" ;
            else if( autoSpace>0 ) {
                 spc=(!spcCounter || (*spcCounter)==0) ? "" : ((spcCounter && !((*spcCounter)%autoSpace) ) ? "\n" : spcSymb);
            }
            

            if(lexem(i)->type&Lexem::fBinary)out->add( ptr,lexem(i)->datasize );
            else out->printf("%s%s", spc , ptr);
            if(spcCounter)++(*spcCounter);
        }
    }
    return out->ptr();
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ debug staff 
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

char * sCalc::debugPrint(const char * t)
{
    //sStr str(0,0,0,(sStrCallback)printf,stdout);
    sFil str("debug.log");
    str.printf("\n\n");
    if(t)str.printf("%s\n",t);
    for(idx i=0; i < _lxit.dim() ; ++i){
        Lexem * it=_lxit.ptr(i);
        if(subLevel)for(idx k=0; k<subLevel; ++k) str.printf("    ");
        str.printf("%" DEC ") %2" HEX " %s '%s' '%s'\n", i, it->type, it->status&Lexem::fReady ? "done" : "todo" ,(char*)_mex.ptr(it->content), it->data ? (char*)_mex.ptr(it->data) : "");
    }
    return str.ptr();
}





