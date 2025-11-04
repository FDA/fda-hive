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

#include <sys/stat.h>
#include <ion/vax.hpp>


using namespace slib;


idx sFlax::ensureRecordBuf(const char * seprec)
{

    if(filePointer || fileHandle){
        recBuf.cut(0);
        if(fileHandle) {
            recLen=recBuf.readIO( fileHandle, seprec );
        } else if(filePointer) {
            recLen=recBuf.readIO( (FILE*)filePointer, seprec );
        }
        if(!recLen) {
            if(callbackProgress)callbackProgress(this,0,recStart-srcStart,srcEnd-srcStart,gLog);
            return 0;
        }
        recStart=(const char * ) recBuf.ptr();
        recNext=recStart+recLen+1;
        if(callbackProgress)callbackProgress(this,0,recStart-srcStart,srcEnd-srcStart,gLog);
    }
    else {
        if(recNext>=srcEnd) {
            if(callbackProgress)callbackProgress(this,0,recStart-srcStart,srcEnd-srcStart,gLog);
            return 0;
        }

        idx rofs=recNext-srcStart;
        if(reMapChunk && rofs-mapOfs>=reMapChunk){
            idx len=srcEnd-srcStart;
            mapOfs+=reMapChunk;
            idx sizemap=2*reMapChunk;if(sizemap>len-mapOfs)sizemap=len-mapOfs;
            mapFile.remap( 0,0,false, mapOfs, sizemap);
            srcStart=mapFile.ptr(0)-mapOfs;
            srcEnd=srcStart+len;
            recNext=srcStart+rofs;
        }
    
        recStart=recNext;
        if(seprec) {
            rofs=0;
            while( recStart<srcEnd ) {
                while(recNext<srcEnd){
                    const char * s=seprec;
                    while( *s && *s!=*recNext )
                        {++s;++rofs;}
                    if((*s))break;
                    if (!rofs) break;
                    ++recNext;
                }
                if(recStart<recNext) {
                    ++recNext;
                    break;
                }
                else{
                    if (!rofs) {
                        ++recNext; recStart=recNext;
                    }
                    else
                        continue;
                }
            }
        } else {
            recNext=srcEnd;
        }
        if(callbackProgress)callbackProgress(this,0,recStart-srcStart,srcEnd-srcStart,gLog);
        if(recStart>=srcEnd) {
            return 0;
        }

        recLen=recNext-recStart;
    }

    return recLen;
}

void sFlax::spaceAndQuoteCleanup (const void ** recordBody, idx * recordSize , sMex * clnEscapeBuf)
{
    idx i;
    for(i=0 ; i<(*recordSize) && strchr(sString_symbolsBlank,((char*)(*recordBody))[i]) ; ++i)
        {}
    if(i) {
        *recordBody=(void*)sShift((*recordBody),i);
        *recordSize-=i;
    }
    for(; (*recordSize)>0 && strchr(sString_symbolsBlank,((char*)(*recordBody))[(*recordSize)-1]) ; --(*recordSize))
        {}

    char ch= (*recordSize) ?  ((char*)(*recordBody))[0] :0 ;
    if( ( ch=='\'' || ch=='\"')  && (   ((char*)(*recordBody))[(*recordSize)-1]==ch    ) ){
        *recordBody=(void*)sShift((*recordBody),1);
        (*recordSize)-=2;
    }

    if(clnEscapeBuf) {
        idx ofs=clnEscapeBuf->add((const char*)0,(*recordSize)+1);
        char * dst0=(char*)clnEscapeBuf->ptr(ofs), *dst=dst0;
        const char * src=(char * )(*recordBody);
        for ( idx i=0; i< (*recordSize) ; ++i , ++dst) {
            ch=src[i];
            if( i>0 && src[i-1]=='\\') {
                --dst;
                if(ch=='n')ch=10;
                else if(ch=='r')ch=13;
                else if(ch=='t')ch=9;
                else if(ch=='b')ch=7;
            }
            *dst=ch;
        }
        *recordSize=dst-dst0;
        *recordBody=dst0;
    }
}



#define scanTillChar(_v_p, _v_ch )  { \
    idx incomment=0; \
    while( strchr((_v_ch),*(_v_p)) && (_v_p)<recNext) { \
        if(*(_v_p)=='\n'){ ++lineCount; lineStart=nextLine;nextLine=_v_p+1-srcStart;} \
        if(*(_v_p)=='/' && *((_v_p)+1)=='*'){ ++incomment; ++(_v_p);continue; } \
        if(incomment && *(_v_p)=='*' && *((_v_p)+1)=='/' ){ --incomment; ++(_v_p);continue; } \
        ++(_v_p);} \
    } \
    if( (_v_p)>=recNext) break;

#define scanTillNotChar(_v_p, _v_ch , _v_doquote) idx inparentesis=0, incomment=0; for( char inquote=0; (_v_p)<=recNext ; ++(_v_p)) { \
        if( *(_v_p)=='\"'){ if(_v_doquote && !inquote) {inquote=*(_v_p);continue;} else if(inquote==*(_v_p) && *((_v_p)-1)!='\\'){inquote=0;continue;} } \
        if(*(_v_p)=='/' && *((_v_p)+1)=='*'){ ++incomment; ++(_v_p);continue; } \
        if(incomment && *(_v_p)=='*' && *((_v_p)+1)=='/' ){ --incomment; ++(_v_p);continue; } \
        if(!inquote && !inparentesis && !incomment && strchr((_v_ch), *(_v_p) )  ) {break;} \
        if(*(_v_p)=='\n'){ ++lineCount; lineStart=nextLine;nextLine=_v_p+1-srcStart;} \
        else if(!inquote && *(_v_p)=='(' && !inparentesis) {++inparentesis;continue; } \
        else if(!inquote && *(_v_p)==')' && inparentesis) {--inparentesis;continue; } \
    }if( (_v_p)>=recNext) break;

#define reportErr(_v_p,_v_errcode) {errPos=(_v_p)-srcStart;errCode=(_v_errcode);if(stopOnErrors)return 0;}


#define treatComment(_v_p) while((_v_p)[0]=='/' && (_v_p)[1]=='/') { \
    scanTillNotChar((_v_p),sString_symbolsEndline,false); \
    if(*(_v_p)=='\n')++lineCount; \
    ++(_v_p); \
    scanTillChar((_v_p),sString_symbolsBlank); \
}

#define treatBlockComment(_v_p) while((_v_p)[0]=='/' && (_v_p)[1]=='*') { \
    for( (_v_p)+=2; (_v_p<recNext) && ((_v_p)[0-2]!='*' || (_v_p)[0-1]!='/' ) ; ++(_v_p)) { \
        if(*(_v_p)=='\n')++lineCount; \
    } \
    ++(_v_p); \
    scanTillChar((_v_p),sString_symbolsBlank); \
}


static const char * sJax_flagsExpected[]={
    " \'variable\'",
    " \'comma\'",
    " \'semicolon\'",
    " \'square braket open\'",
    " \'square braket close\'",
    " \'curly braket open\'",
    " \'curly braket close\'",
    " \'value\'",
    " #include",
};

enum bFlagsExpected {
    bVariableExpected=0,
    bCommaExpected,
    bSemicolonExpected,
    bBraketOpenExpected,
    bBraketCloseExpected,
    bCurlyBraketOpenExpected,
    bCurlyBraketCloseExpected,
    bValueExpected,
    bIncludeExpected
};


idx sJax::include(const char * p)
{
    sFil vs(p,sMex::fReadonly);

    idx r=1;
    if(vs.ok() ) {
        idx p_varCounter=varCounter;
        idx p_lineCount=lineCount;
        idx p_errPos=errPos;
        idx p_lineStart=lineStart;
        idx p_nextLine=nextLine;
        idx p_flNmPos=flNmPos;
        const char * p_errCode=errCode;



        const char * p_recStart=recStart;
        const char * p_recNext=recNext;
        idx p_recLen=recLen;
        const char * p_srcStart=srcStart;
        const char * p_srcEnd=srcEnd;


        srcStart=vs.ptr(0);
        srcEnd=srcStart+vs.length()+1;
        recNext=srcStart;


        r=parse();


        varCounter=p_varCounter;
        lineCount=p_lineCount;
        errPos=p_errPos;
        flNmPos=p_flNmPos;
        lineStart=p_lineStart;
        nextLine=p_nextLine;
        errCode=p_errCode;
        recStart=p_recStart;
        recNext=p_recNext;
        recLen=p_recLen;
        srcStart=p_srcStart;
        srcEnd=p_srcEnd;

    }

    return r;
}


idx sJax::parse(void)
{
    static const char * varChars="$_#@`\'\"";
    if(!expected)
        expected=sFlag(bVariableExpected)|sFlag(bCurlyBraketOpenExpected);
    lineStart=0;
    varCounter=0;
    nextLine=lineStart;
    static const char  * autoVar="$=";

    do{
        if ( !sFlax::ensureRecordBuf())return 1;

        const char * p=recStart;

        while(p<recNext || p==autoVar) {

            if(p!=autoVar) {
                scanTillChar(p,sString_symbolsBlank);

                treatComment(p);
                treatBlockComment(p);
            }


            if(expected&sFlag(bSemicolonExpected) && *p==';' ){
                expected=0;
                if(scope->dim()==0)
                    expected=sFlag(bVariableExpected)|sFlag(bIncludeExpected);
                ++p;
                continue;
            }

            else if( *p=='#' ){
                ++p;
                bool ok=false;
                if(strncmp(p,"include",7)==0 || strncmp(p,"define",6)==0){
                    char frType=fr->type;
                    p+= (*p=='i') ? 7 : 6;
                    if(*p==':' || strchr(sString_symbolsSpace,*p)!=0){
                        scanTillChar(p,":" sString_symbolsBlank);
                        const char * src=p;
                        scanTillNotChar(p,";]}," sString_symbolsBlank,true);
                        if(p>src) {
                            idx pos=flName.length(); bool inquote=false;
                            if(*src=='\'' || *src=='\"' ) {
                                if(*(p-1)==*src){
                                    ++src;
                                    --p;
                                    inquote=true;
                                }
                            }
                            flName.add(0,flName.length()+(p-src)+2);
                            flNmPos=pos+1;
                            flName.cut(pos+1);
                            flName.makeNameAt(sIdxMax,flName.ptr(0),"%%dir/%.*s",(int)(p-src),src);
                            include(flName.ptr(pos+1));
                            flName.cut(pos);

                            if(errIO.length()) {
                                if(stopOnErrors)return 0;
                            }
                            ok=true;
                            if(inquote)++p;
                        }
                    }
                    scanTillNotChar(p,",;" sString_symbolsEndline,true);
                    if(frType=='[' && *p==',' ) {
                        expected=sFlag(bValueExpected);
                    } else {
                        expected=sFlag(bVariableExpected);
                    }
                    expected|=sFlag(bIncludeExpected)|sFlag(bCurlyBraketOpenExpected)|sFlag(bBraketOpenExpected);
                    if(frType=='{') expected|=sFlag(bCurlyBraketCloseExpected);
                    else if(frType=='[') {expected|=sFlag(bBraketCloseExpected);}
                }
                if(!ok)
                    {reportErr(p,"unexpected directive");}
                ++p;
                continue;
            }
            else if(expected&sFlag(bCommaExpected) && *p==','  ) {
                expected=sFlag(bBraketOpenExpected)|sFlag(bCurlyBraketOpenExpected);
                if(fr && fr->type=='[')expected|=sFlag(bValueExpected);
                else expected|=sFlag(bVariableExpected)|sFlag(bIncludeExpected);
                ++p;
                continue;
            }
            else if( (expected&sFlag(bCurlyBraketCloseExpected) && *p=='}') || (expected&sFlag(bBraketCloseExpected) && *p==']')  ){

                if(fr) {
                    fr->valSize=p-fr->valOfs+1;
                }
                scope->cut(scope->dim()-1);
                fr=scope->dim() ? scope->ptr(scope->dim()-1) : 0 ;
                prv=scope->dim()>1 ? scope->ptr(scope->dim()-2) : 0 ;


                expected=0;
                if(scope->dim()==0)
                    expected|=sFlag(bSemicolonExpected);
                else if(scope->dim()){
                    if(fr){
                        if(fr->type=='{') expected|=sFlag(bCurlyBraketCloseExpected);
                        else if(fr->type=='[') expected|=sFlag(bBraketCloseExpected);
                    }
                    expected|=sFlag(bCommaExpected);
                }
                ++p;
                continue;
            }

            if(fr && fr->type=='[' ){
                if(fr)++fr->rowNum;
                fr=scope->add();
                prv=scope->dim()>1 ? scope->ptr(scope->dim()-2) : 0 ;
                fr->varOfs=prv->valOfs;
                fr->varSize=1;
                fr->valSize=0;
                fr->varNum=0;
                fr->rowNum=0;
                fr->lineNumber=lineCount;
                fr->type='+';
                fr->depth=scope->dim();
                fr->op=':';
                fr->userData=-1;
                analyzeVariable(fr,prv);
            }


            if( expected&sFlag(bCurlyBraketOpenExpected) && *p=='{' ){
                if(!fr){
                    p=autoVar;
                    expected=sFlag(bVariableExpected);
                    continue;
                }
                expected=sFlag(bVariableExpected)|sFlag(bCurlyBraketCloseExpected);

                fr->valOfs=p;
                fr->type=*p;++p;
                fr->varNum=++varCounter;
                analyzeValue(fr,prv);
                if(errIO.length())
                    { reportErr(p,errIO.ptr());}
            }
            else if( expected&sFlag(bBraketOpenExpected) &&  *p=='[' ){
                if(!fr){
                    p=autoVar;
                    expected=sFlag(bVariableExpected)|sFlag(bBraketCloseExpected);
                    continue;
                }
                expected=sFlag(bValueExpected)|sFlag(bCurlyBraketOpenExpected)|sFlag(bBraketOpenExpected)|sFlag(bBraketCloseExpected);
                fr->valOfs=p;
                fr->type=*p;++p;
                fr->varNum=++varCounter;

                analyzeValue(fr,prv);
                if(errIO.length())
                    { reportErr(p,errIO.ptr());}
            }
            else if( expected&sFlag(bVariableExpected) && (strchr(varChars,*p) || (*p>='a' && *p<='z') || (*p>='A' && *p<='Z') || (*p>='0' && *p<='9') ) ) {

                if(fr)++fr->rowNum;
                fr=scope->add();
                prv=scope->dim()>1 ? scope->ptr(scope->dim()-2) : 0 ;

                fr->varOfs=p;
                if(p==autoVar) ++p;
                else {scanTillNotChar(p,":="".{}[],;\"\'" sString_symbolsBlank,true);}
                fr->varSize=p-fr->varOfs;
                fr->valSize=0;
                fr->varNum=0;
                fr->rowNum=0;
                fr->lineNumber=lineCount;
                fr->userData=-1;
                fr->depth=scope->dim();

                if(!strchr(":=",*p))
                    { scanTillChar(p,sString_symbolsBlank);}
                if(!strchr(":=",*p))
                    reportErr(p,"not an assignment");
                fr->type=*p;
                fr->op=*p;++p;
                analyzeVariable(fr,prv);
                expected=sFlag(bValueExpected)|sFlag(bCurlyBraketOpenExpected)|sFlag(bBraketOpenExpected);
                if(p==autoVar+2) {
                    p=recStart;
                    lineCount=0;
                }
            }
            else if(expected&sFlag(bValueExpected)) {

                scanTillChar(p,sString_symbolsBlank);
                treatComment(p);
                fr->valOfs=p;
                fr->varNum=++varCounter;

                scanTillNotChar(p,",;]}" sString_symbolsBlank,true);

                fr->valSize=p-fr->valOfs;
                if(fr->type!='{' && fr->type!='[')
                    analyzeValue(fr,prv);

                if(errIO.length())
                    { reportErr(p,errIO.ptr());}

                scope->cut(scope->dim()-1);
                fr=scope->dim() ? scope->ptr(scope->dim()-1) : 0 ;
                prv=scope->dim()>1 ? scope->ptr(scope->dim()-2) : 0 ;

                expected=0;
                if(scope->dim()==0)
                    expected|=sFlag(bSemicolonExpected);
                else if(scope->dim()){
                    if(fr){
                        if(fr->type=='{') expected|=sFlag(bCurlyBraketCloseExpected);
                        else if(fr->type=='[') expected|=sFlag(bBraketCloseExpected);
                    }
                    expected|=sFlag(bCommaExpected);
                }

            }
            else if(p!=recNext) {
                {
                    errIO.add("Expected",8);
                    for( idx ie=0; ie<sDim(sJax_flagsExpected); ++ie ) {
                        if(expected&sFlag(ie)) {
                            errIO.add(sJax_flagsExpected[ie],sLen(sJax_flagsExpected[ie]));

                        }
                    }
                    errIO.add("\nFound \'",8);
                    errIO.add(p,32);
                    errIO.add("...\'\0",5);
                    reportErr(p,"unexpected syntax");

                }
            }
        }


    }while(true);
    return 1;
}

idx sJax::analyzeJsonValue(JsonFrame * fr,JsonFrame * prv)
{
    static sMex jax_buf;

    const void * body;
    idx sz;

    jax_buf.cut(0);
    if(prv) {
        body=prv->varOfs;
        sz=prv->varSize;
        if( sz!=1 || *(const char*)body!='$' ) {
            body=(const void *)jsonNodes.id(prv->userData,&sz);
            jax_buf.add(body,sz);
            jax_buf.add(".",1);
        }
    }

    body=fr->varOfs;
    sz=fr->varSize;
    if( sz==1 || *(const char*)body=='$' )
        return 0;
    sFlax::spaceAndQuoteCleanup (&body, &sz, &jax_buf );
    JsonNode * js=jsonNodes.set(body,sz,&(fr->userData));
    js->json=this;
    js->fr=fr;

    jax_buf.cut(0);
    sFlax::spaceAndQuoteCleanup ((const void **)&fr->valOfs, &fr->valSize, &jax_buf );

    return 1;
}






idx sVax::ensureRecordBuf(void)
{
    if( (recordCnt && recordCur>=recordStart+recordCnt) ) {
        return 0;
    }


    do{
        const char * seprec=varBuf.ptr(separRecord);
        if ( !sFlax::ensureRecordBuf(isInHeader ? "\n": seprec))return 0;


        idx ll = sLen(vaxMarker);
        if( (!(flags&fDoNotSupportVaxHeader)) &&
                ( strncmp( recStart,vaxMarker,sMin ( recLen-1, ll  ))==0 ) &&
                (isInHeader || (flags&fSupportMultipleVaxHeader)) ) {
            vaxHeaderParse(recStart+ll,recLen-ll);
            isInHeader=1;
            continue;
        }


        if(isInHeader>=1 ) {
            if( !(flags&fDoNotSupportTableHeader) ) {
                tableHeaderParse(recStart,recLen);
            }else {
                isInHeader=0;
                if(flags&fStopAfterHeaders) {
                    flags&=(~fStopAfterHeaders);
                }
                break;
            }
            isInHeader=0;
            if(flags&fStopAfterHeaders) {
                flags&=(~fStopAfterHeaders);
                break;
            }
            continue;
        }

        if( commentMarker ) {
            if(sString::compareChoice(recStart,commentMarker,0,true,0,false, recNext-recStart-1)!=-1)
                continue;
        }

        if( (recordStart && recordCur<recordStart) ) {
            ++recordCur;
            continue;
        }
        break;

    } while( true ) ;


    if(!(flags&fDoNotPreparseCellOffsets)) {

        const char * p=recStart, * sf=varBuf.ptr(separField);
        hdrTable[(idx)0].ofs=p;
        idx ic,lenInOneCell=-1;
        char inquote=0;
        for(ic=1; p<recNext ; ++p) {
            lenInOneCell++;

            if(*p=='\'' || *p=='\"'  ){
                if ( !(flags&fDoNotUseQuoteProtection) ) {
                    if(!inquote) {
                        if(lenInOneCell==0)
                              {inquote=*p;continue;}
                    }
                    else if(inquote==*p && *(p-1)!='\\'){inquote=0;continue;}
                }
            }
            if(!inquote && strchr(sf, *p )  ) {
                if(ic<hdrTable.dim()){
                    hdrTable[ic].ofs=p+1;
                }
                ++ic;
                lenInOneCell=-1;
            }
        }
        while(ic<hdrTable.dim())
            {hdrTable[ic].ofs=p;++ic;}
        for ( ic=0; ic<hdrTable.dim()-1; ++ic ) {
            FieldInfo * hf=hdrTable.ptr(ic);
            FieldInfo * hf1=hdrTable.ptr(ic+1);
            hf->size=(hf1)->ofs-(const char *)(hf->ofs) - 1 ;
            if( (hf->ofs[0]=='\'' || hf->ofs[0]=='\"') && hf->ofs[hf->size-1]==hf->ofs[0] ) {
                ++hf->ofs;
                hf->size-=2;
            }
        }
    }

    ++recordCur;

    return recLen;
}


idx sVax::parse ( idx count )
{

    for ( idx i=0; i<count ; ++i )  {
        ensureRecordBuf();
    }
    return count;
}

const char * sVax::headerKeywords="$number_records" _ "$field_separator" _ "$record_separator" _ "$field_list" __;
const char * sVax::headerAttributes="default" _ "type" _ "field" _ "unique" _ "separator" __;
const char * sVax::valueTypes="string" _ "integer" _ "real" _ "boolean" __;

sVax * sVax::vaxHeaderParse(const char * ptr,idx len)
{

    if(!len)return this;


    for(idx l=0;l<len && strchr(sString_symbolsSpace,*ptr);++l)++ptr;

    idx num, l=sString::compareChoice(ptr,headerKeywords,&num,false,0,false,sMin(len,(idx)64)), ic, ll;
    const char * content = (l!=sNotIdx) ? sString::skipWords(ptr+l,0,0,"=") : 0, * p,*nxt;
    idx pos=varBuf.length();
    char * val,*v,*eq;

    if(content) {

        len-=content-ptr+1;

        sString::cleanEnds(&varBuf,content,len,"\r\n" ,true,0);
        val=varBuf.ptr(pos);
        sString::searchAndReplaceStrings(val,0,"\\n" _ "\\r" _ "\\t" _ "\\s" __,"\n" _ "\r" _ "\t" _ " " __,0,false);



        switch ( num) {
            case eNumRecords:
                maxNumRecords=atoidx(val);
                break;
            case eRecordSeparator:
                sString::cleanEnds(val,0,"\'\"" ,true,0);
                separRecord=pos;
                break;
            case eFieldSeparator:
                sString::cleanEnds(val,len,"\'\"",true,0);
                separField=pos;
                break;
            case eFieldList:
                sString::searchAndReplaceSymbols(val,0,sString_symbolsBlank,0,0,true,true,true,false);
                for ( ic=0, p=val; *p; p=nxt ) {
                    l=sLen(p);
                    nxt=p+l+1;
                    if( (*p=='\'' || *p=='\"') && *(p+l-1)==*p )
                        {++p; l-=2;}
                    hdrTable.set(p,l)->iCol=ic;
                }
                break;
            default:
                break;
        }

    } else {
        sString::searchAndReplaceSymbols(&varBuf,ptr,len,sString_symbolsBlank," ",0,true,true,true,false);
        val=varBuf.ptr(pos);
        sString::searchAndReplaceStrings(val,0," =" _ "= " __,"=" __,0,false);
        sString::searchAndReplaceStrings(val,0," =" _ "= " __,"=" __,0,false);

        sString::searchAndReplaceSymbols(val,0,sString_symbolsBlank,0,0,true,true,true,false);


        FieldInfo * fi=0;
        for ( ic=0, v=val; *v; v+=l+1 , ++ic) {
            ll=l=sLen(v);

            eq=0;
            bool isquote=false;
            for(idx i=0; i<l; ++i)  {
                if(*(v+i)=='='){eq=v+i+1;*(v+i)=0;ll-=i+1; break;}
            }
            if( eq && (*eq=='\'' || *eq=='\"') && *(eq+ll-1)==*eq )
                {++eq;ll-=2;isquote=true;}

            if( ic==0 ){
                fi=hdrTable.get(v,l);
                if(fi)continue;
                callbackHeaderStruct * cs=callbackList.get(v);
                idx vlen=sLen(v);
                if(cs && cs->func){ cs->func(this,cs->param,ptr+vlen,len-vlen); }
                break;
            }

            idx il=sString::compareChoice(v,headerAttributes,&num,false,0,false);
            if(il==-1)break;
            switch(num) {
                case eAttrType:
                    sString::compareChoice(eq,valueTypes,&(fi->valueType),false,0,true);
                    break;
                case eAttrFieldNum:
                    fi->iCol=atoidx(eq);
                    break;
                case eAttrUnique:
                    sString::compareChoice(eq,"false" _ "true" _ "0" _ "1" __,&(fi->valueUnique),false,0,true);
                    fi->valueUnique%=2;
                    break;
                case eAttrKeyValSeparator:
                    sString::cleanEnds(val,len,"\'\"",true,0);
                    fi->keyValSeparator=pos;
                    break;
                case eAttrDefault:
                    if(isquote) {
                        fi->defValueType=FieldInfo::eDefValueDirect;
                        fi->defValuePos=pos+(eq-val);
                        fi->defValueSize=ll;
                    }else if(*eq=='+' || *eq=='-') {
                        fi->defValueType=FieldInfo::eDefValueIncrement;
                        fi->defValuePos=atoidx(eq);
                    }else if(!strcmp(eq,"previous") ) {
                        fi->defValueType=FieldInfo::eDefValuePrevious;
                    }
                    else {
                        fi->defValueType=FieldInfo::eDefValueReference;
                        fi->defValuePos=hdrTable.find(eq,ll);
                        --fi->defValuePos;
                    }
                    break;
                default:
                break;
            }
        }
        if(fi)
            fi->isValid=1;

    }


    return this;
}

sVax * sVax::tableHeaderParse(const char * ptr,idx len)
{
    if(!len)len=sLen(ptr);
    const char * end=ptr+len, * nxt;
    callbackHeaderStruct * cs=callbackList.get("$");
    const char * sepfld=varBuf.ptr(separField);
    const char * seprec=varBuf.ptr(separRecord);
    for(idx iCol=0; ptr && ptr<end ;++iCol) {

        {
            nxt=ptr;
            char inquote=0;
            for(; nxt<end  ; ++nxt){
                if (flags&fSupportQuotes) {
                    if( *nxt=='\"' || *nxt=='\'' ) {
                        if(!inquote)
                            {inquote=*nxt; continue;}
                        if( inquote==*nxt )
                            {inquote=0;continue;}
                    }
                    if(inquote)
                        continue;
                }
                if( strchr(sepfld,*nxt) )
                    break;
                if( strchr( seprec,*nxt) )
                    break;
            }

            if(nxt!=end)
                ++nxt;
        }




        idx clen=nxt-ptr;
        if(nxt<end)
            --clen;

        while(strchr(sString_symbolsSpace,*ptr) || *ptr=='\r'){++ptr;--clen;}
        while( clen > 1 && (strchr(sString_symbolsSpace, ptr[clen - 1]) || ptr[clen - 1] == '\r') ) {--clen;}
        if(!clen)
        break;

        FieldInfo * fi=hdrTable.get(ptr,clen);
        if(!fi || !fi->iCol)
            hdrTable.set(ptr,clen)->iCol=iCol;
        if(cs && cs->func){ cs->func(this,cs->param,ptr,clen); }

        if(!nxt)break;
        ptr=nxt;
    }
    hdrTable.add(1);
    return this;
}


idx sVax::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{


    if(fieldType==0) {
        record=ensureRecordBuf();
        if(!record)
            return -2;
    }

    idx fieldTypeNum;
    if( !hdrTable.find(fieldTypeName,&fieldTypeNum))
        return record;
    idx ic=hdrTable[fieldTypeNum].iCol;
    if(ic!=sNotIdx)
        fieldTypeNum=ic;


    const char * p=hdrTable[fieldTypeNum].ofs;
    idx sz=hdrTable[fieldTypeNum].size;
    idx quoted =  ( ( p[0]=='\"' || p[0]=='\'') && (p[sz-1]==p[0] ) ) ? 1 : 0 ;

    *recordBody=(const void * ) (p+quoted);
    *recordSize=- ( hdrTable[fieldTypeNum].size -2*quoted);

    return record;

}


idx sVax::ionProviderCallbackTable(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{

    if(fieldType==0 && internalFieldIterator ==0) {
        record=ensureRecordBuf();if(!record)return -2;
    }

    if(fieldType==0 && fieldTypeName[0]=='#' && fieldTypeName[1]=='R' && fieldTypeName[2]==0 ){
        *recordBody=&recordCur;
        *recordSize=sizeof(recordCur);
        return record;
    }
    internalFieldIterator=nextInternal(internalFieldIterator,fieldType==1,recordBody,recordSize);
    if(internalFieldIterator==sNotIdx)
        internalFieldIterator=0;


    spaceAndQuoteCleanup(recordBody, recordSize );

    return record;

}

idx sVax::nextInternal(idx iter, bool isName, const void * * recordBody, idx * recordSize)
{
    idx iatr=iter>>32;
    iter&=0xFFFFFFFF;
    idx icol=iter,isLastInColMap=false;
    if(internalColumnMap){
        if(internalColumnMap[icol+1]==sNotIdx)
            isLastInColMap=true;
        icol=internalColumnMap[icol];
    }
    if(icol==sNotIdx || icol>=hdrTable.dim()){ return sNotIdx; }


    FieldInfo * fi=hdrTable.ptr(icol);

    if(fi->keyValSeparator!=0 || separAttribs!=0 ) {
        const char * secondarySeparators=fi->keyValSeparator ? varBuf.ptr(fi->keyValSeparator) : varBuf.ptr(separAttribs) ;
        const char * rec = fi->ofs+iatr, * rend=fi->ofs+fi->size;
        if(rend==recNext)
            --rend;
        while( rec<rend && strchr(secondarySeparators, *rec ) )
            ++rec;
        if(rec==rend  ) {
            return nextInternal(iter+1, isName, recordBody, recordSize);
        }
        do {
            *recordBody=rec;
            *recordSize=0;
            char inquote=0;

            while( rec<rend ) {
                if(*rec=='\\' || *rec=='\"'){
                    if(!inquote) {inquote=*rec;}
                    else if(inquote==*rec){inquote=0;}
                }
                if(!inquote && strchr(secondarySeparators, *rec ) )
                    break;
                 ++rec; (*recordSize)++;
            }
            if( isName && (inclusionNames || exclusionNames) ) {
                bool flt=false;
                if( (inclusionNames && sString::compareChoice((char*)(*recordBody),inclusionNames,0,true,0,false, *recordSize)==-1)  ) {
                    flt=true;
                }
                else if(  (exclusionNames && sString::compareChoice((char*)(*recordBody),exclusionNames,0,true,0,false, *recordSize)!=-1) ) {
                    flt=true;
                }
                if(flt){
                    if( rec<rend && *rec==secondarySeparators[0] ) {
                        for( ++rec; rec<rend ; ) {
                            if(*rec=='\\' || *rec=='\"'){
                                if(!inquote) {inquote=*rec;}
                                else if(inquote==*rec){inquote=0;}
                            }
                            if(!inquote && strchr(secondarySeparators, *rec ) )
                                break;
                             ++rec;
                        }
                    }
                    while( rec<rend && strchr(secondarySeparators, *rec ) )
                        ++rec;
                    if(rec<rend) {
                        continue;
                    }else {
                        iatr=-1;
                        *recordBody=0;
                        *recordSize=0;
                    }
                }
            }
            break;
        }while(true);

        if(iatr==0 && rec==rend && isName ){
            *recordBody=(const void * ) hdrTable.id(icol,recordSize);
            if(isName  ) {
                bool flt=false;
                if( (inclusionNames && sString::compareChoice((char*)(*recordBody),inclusionNames,0,true,0,false, *recordSize)==-1)  ) {
                    flt=true;
                }
                else if(nameInclusionDic && nameInclusionDic->get((char*)(*recordBody),*recordSize)==0 ) {
                    flt=true;
                }
                else if( (exclusionNames && sString::compareChoice((char*)(*recordBody),exclusionNames,0,true,0,false, *recordSize)!=-1) ) {
                    flt=true;
                }
                if(flt) {
                    return nextInternal(iter+1, isName, recordBody, recordSize);
                }
            }
            else
                return ((iter)|(iatr<<32));
        }

        while( rec<rend && strchr(secondarySeparators, *rec ) )
            ++rec;
        iatr=rec-fi->ofs;


        if(rec==rend ) {
            iatr=0;
            if(rec==recNext-1)
                return sNotIdx;
            else
                ++iter;
        }
        return ((iter)|(iatr<<32));
    }

    {
        if(isName){
            while (icol<hdrTable.dim()) {
                *recordBody=(const void * ) hdrTable.id(icol,recordSize);
                bool flt=false;
                if(isName){
                    if((inclusionNames && sString::compareChoice((char*)(*recordBody),inclusionNames,0,true,0,false, *recordSize)==-1)  ) {
                        flt=true;
                    }
                    else if(nameInclusionDic && nameInclusionDic->get((char*)(*recordBody),*recordSize)==0 ) {
                        flt=true;
                    }
                    else if((exclusionNames && sString::compareChoice((char*)(*recordBody),exclusionNames,0,true,0,false, *recordSize)!=-1) ) {
                        flt=true;
                    }
                    if(flt ) {
                        iter++;
                        iatr=iter>>32;
                        icol=iter&0xFFFFFFFF;

                    }else break;
                }
            }
            if(icol>=hdrTable.dim()){ *recordBody=0; *recordSize=0;return sNotIdx; }





        } else {
            *recordBody=(const void * ) fi->ofs;
            *recordSize=fi->size;
            if(fi->ofs+fi->size == recNext && *(recordSize) )
                --(*recordSize);

            if( exclusionValues ) {
                if(*recordSize<0 || sString::compareChoice((char*)(*recordBody),exclusionValues,0,true,0,false, *recordSize)!=-1) {
                    *recordBody=0;
                    *recordSize=0;
                }
            }
            if(icol>=hdrTable.dim()-2)
                return sNotIdx;
            else if(isLastInColMap)
                return sNotIdx;
            else
                ++iter;
        }

        iatr=0;
    }

    return ((iter)|(iatr<<32));
}



