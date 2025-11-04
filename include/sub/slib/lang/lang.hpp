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
#ifndef sLib_lang_lang_hpp
#define sLib_lang_lang_hpp

#include <slib/core/str.hpp>
#include <slib/core/sIO.hpp>
#include <slib/core/dic.hpp>
#include <slib/std/string.hpp>
#include <slib/std/regexp.hpp>
#include <slib/lang/lang_hlp.hpp>

#define DEBUGOUT

namespace slib
{
    class sCalc;

    class sLang
    {
        public:

            struct Location {idx Start, End; };
            struct Statement { Location stat, body, prth, equ, nam ; };
            enum elOperationClass { elOperationGeneric,elOperationAssignment,elOperationDeclareFunction};

            sStr script;
            sDic < Statement > dicStat;
            const char * lastRet;
            elOperationClass lastOp;

            static char * scriptClean(sStr * dst, const char * src, idx len=0);
            static const char * scriptTokenize(sDic < Statement > * stats, const char * src , idx len=0, idx ofsPos=0);

            idx parse(const char * source, idx len=0 , bool issubscript=false);
            void debugPrintf( const char * fmt, ... );

            char * getStat(sStr * dst, const Location * loc, idx clean=true);

        public:
            struct ExecFunction;
            typedef idx (* ExecFunctionCB)(sLang * lg, ExecFunction * ef, idx il);
            struct ExecFunction {
                    ExecFunctionCB func;
                    sLang * lib;
                    void * param;
            };
            sDic < ExecFunction > dicLang, dicLib;
            sDic < idx > dicFun;
            void dicLangRecord (const char * name, ExecFunctionCB func, sLang * lib=0, void * param=0 )
            {
                ExecFunction & e=dicLang[ name ];
                e.func=func;
                e.lib=lib ? lib : this;
                e.param=param;
            }
            void dicLibRecord (const char * name, ExecFunctionCB func, sLang * lib=0, void * param=0 )
            {
                ExecFunction & e=dicLib[ name ];
                e.func=func;
                e.lib=lib ? lib : this;
                e.param=param;
            }
            sDic< sDic < idx > > dicDic;
            sStr dicDicData;


            idx exitCond, doDebug;

            bool executeStatement(idx il );
            idx expressionCompute(sStr * out, const char * phrase, idx len ) ;
            static idx expressionCallback(sLang * lg, sStr * out,  sCalc * xic, idx cur, idx cnt, idx whattodo);



            static idx exec_declarefun(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_declarearr(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_assignment(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_generic(sLang * lg, ExecFunction * ef, idx il);

            static idx exec_debug(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_comment(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_codebreak(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_if(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_else(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_loop(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_shell(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_include(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_define(sLang * lg, ExecFunction * ef, idx il);

            static idx exec_eval(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_foreach(sLang * lg, sLang::ExecFunction * ef, idx il);
            static idx exec_parse(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_isvar(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_isfun(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_set(sLang * lg, ExecFunction * ef, idx il);
            static idx exec_setvar(sLang * lg, ExecFunction * ef, idx il);

            void buildCore( void );
            void buildStdLib (void) ;


            static const char * stdlibDeclarations ;
            static idx eval_err(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_app_printf(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_app_env(sLang * lg, ExecFunction * ef, idx il);

            static idx eval_dic_get (sLang * lg, ExecFunction * ef, idx il);
            static idx eval_dic_set (sLang * lg, ExecFunction * ef, idx il);


            static idx eval_string_len(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cat(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cmp(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cnt(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cntsymb(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_skip(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_extract(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_compareuntil(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_search(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_crlf(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cstyle(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_replacesymb(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_replacestr(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cleanmarkup(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_cleanends(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_hungarian(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_changecase(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_glue(sLang * lg, sLang::ExecFunction * ef, idx il);
            static idx eval_string_printf(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_unescape( sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_enumerate(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_string_xml2json(sLang * lg, ExecFunction * ef, idx il);


            static idx eval_file_exists(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_open(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_gets(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_read(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_write(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_close(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_len(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_getpos(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_setpos(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_remove(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_rename(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_content(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_printf(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_find(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_makename(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_timestamp(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_makedir(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_rmdir(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_file_curdir(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_time_time(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_time_date(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_time_sleep(sLang * lg, sLang::ExecFunction * ef, idx il);

            static idx eval_array_collect(sLang * lg, ExecFunction * ef, idx il);

            static idx eval_online_httpget(sLang * lg, ExecFunction * ef, idx il);
            static idx eval_online_httpbulk(sLang * lg, ExecFunction * ef, idx il);


        public:

            class Scope {
                public:
                    sStr data;
                    sDic < idx >  dicVar;
                    Scope *  up, *  down, *  global;
                    idx breakCond,retCond, contCond;
                    idx lastIf;
                    idx doDebug;
                    idx lastSerialize;

                    Scope ( Scope * lup=0 , char * lnam=0 )
                    {
                        breakCond=0;retCond=0;contCond=0;doDebug=0;lastIf=1;
                        up=lup;down=0;global=(lup && lup->global) ? lup->global : this;
                        if( lnam ) data.printf("%s", lnam);
                        data.add("\0\0", 2);
                        lastSerialize=0;
                    }

                    const char * getVar( const char * var , idx len=0, const char * defVal="", bool qualifyName=true)
                    {
                        if(!len)len=sLen(var);
                        sStr vv;
                        if(qualifyName) {
                            var=qualifyVarName( &vv, var , len);len=sLen(var);
                        }
                        Scope * scp; if(var[0]==':' && var[1]==':'){scp=global;var+=2;len=sLen(var);}else scp=this;
                        idx * ptrOfs=scp->dicVar.get(var, len);if(!ptrOfs)return defVal;
                        return scp->data.ptr(*ptrOfs);
                    }

                    int isVar( const char * var )
                    {
                        Scope * scp; if(var[0]==':' && var[1]==':'){scp=global;var+=2;}else scp=this;
                        return scp->dicVar.get(var) ? 1 : 0 ;
                    }
                    void delVar( const char * var);
                    char * qualifyVarName( sStr * vv, const char * var , idx varLen=0, bool respectQuotes=true);
                    const char * setVar( const char * var, const char * fmt, ... );
                    const char * setVar( const char * var, idx lenvar, const char * fmt, ... );
                    const char * setVar( const char * var, idx lenvar, const char * value, idx lenval);

                    ~Scope(){
                        data.empty();
                        dicVar.empty();
                    }
                    const char * serialize (sStr * buf,const char * equ="=",const char * sep=";\n" , const char * quote=",", idx start=0, const char * tmplt=0,const char * prfx=0, const char * copyTo=0);
                    const char * copyvars (const char * copyTo,const char * tmplt=0,idx start=0) {return serialize(0,"=",";\n" , ",", start, tmplt,0, copyTo);}


                };
            Scope * curScope, globalScope;
            idx funLevel;


            idx charint(const char * nam, Scope * scp=0){ const char * par=(scp ? scp : curScope)->getVar(nam); return par ? atoi(par) : 0 ;}
            real charreal(const char * nam, Scope * scp=0){ const char * par=(scp ? scp : curScope)->getVar(nam); real val=0;if(par) sscanf(par,"%lf",&val);return val;}
            idx charcase(void){const char * vcase=curScope->getVar("case"); if(!strcmp(vcase,"upper") )return sString::eCaseHi;if(!strcmp(vcase,"lower") )return sString::eCaseLo;return atoi(vcase);}
            idx charbool(const char * nam, Scope * scp=0) {
                const char * val=(scp ? scp : curScope)->getVar(nam);
                idx sz=sLen(val);
                if( sz==5 && strcmp(val,"false")==0)return false;
                else if( sz==4 && strcmp(val,"true")==0)return true;
                return (bool) atoidx(val);
            }

            const char * serialize (sStr * buf,const char * equ="=",const char * sep=";" , const char * quote=",", idx start=-1, const char * tmplt=0,const char * prfx=0, const char * copyTo=0, Scope * scope=0){
                if(!scope)scope=&globalScope;
                if(start==-2) start=lastLibVar;
                if(start==-1) start=scope->lastSerialize;
                scope->lastSerialize=scope->dicVar.dim();
                return scope->serialize(buf,equ,sep,quote, start,tmplt,prfx, copyTo);
            }
            const char * copyvars (const char * copyTo,const char * tmplt=0,idx start=0, Scope * scope=0) {return serialize(0,"=",";\n" , ",", start, tmplt,0, copyTo, scope);}

    public:
        sIO * msgIO;
        sIO * errIO;
        bool constructionMode;
        sStr reslt;
        idx lastLibVar;


        const char * ret(idx * psize=0)
        {
            if(psize)*psize=sLen(lastRet);
            return reslt.ptr(0);
        }
        idx iret(void)
        {
            idx ival=0;
            sIScanf(ival,lastRet,sLen(lastRet),10);
            return ival;
        }
        real rret(void)
        {
            real rval=0;
            sRScanf(rval,lastRet,sLen(lastRet),10);
            return rval;
        }
        bool bret(void)
        {
            if(lastOp==elOperationAssignment)return true;
            idx len=sLen(lastRet);
            if(len==4 && strcmp(lastRet,"true")==0)return true;
            else if(len==5 && strcmp(lastRet,"false")==0)return false;
            else if(lastRet[0]=='0' && len==1) return (bool)iret();
            return true;
        }

        enum eFlags{
            eBuildCore        =0x00000001,
            eBuildStdLib    =0x00000002
        };
        sLang(sIO * lmsg=0, sIO * lerr=0, idx flags=eBuildCore|eBuildStdLib)
        {
            msgIO=lmsg;
            errIO=lerr;

            funLevel=0;
            doDebug=0;
            curScope=&globalScope;
            exitCond=0;
            constructionMode=true;
            if(flags&eBuildCore)
                buildCore();
            if(flags&eBuildStdLib)
                buildStdLib();
            constructionMode=false;
            lastLibVar=globalScope.dicVar.dim();
            lastRet=0;
        }
    };

}

#endif






