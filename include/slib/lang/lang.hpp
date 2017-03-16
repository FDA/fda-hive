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


namespace slib
{
    class sCalc;

    class sLang
    {
        public: // basic scripting 

            struct Location {idx Start, End; };
            struct Statement { Location stat, body, prth, equ, nam ; }; // starts and ends of statement, body, parenthesis or assignment

            sStr script;
            sDic < Statement > dicStat;

            static char * scriptClean(sStr * dst, const char * src, idx len=0);
            static const char * scriptTokenize(sDic < Statement > * stats, const char * src , idx len=0, idx ofsPos=0);
    
            idx parse(const char * source, idx len , bool issubscript=false);
            void debugPrintf( const char * fmt, ... );

            char * getStat(sStr * dst, const Location * loc, idx clean=true);
    
        public: // execution 

            typedef idx (* ExecFunction)(sLang * lg, idx il);
            sDic < ExecFunction > dicLang, dicLib; // language Constructions: if , while ...  and core functions like eval(). .. 
            sDic < idx > dicFun; // dictionary of user defined functions

            sDic< sDic < idx > > dicDic; // dictionary of custom dictionaries 
            sStr dicDicData; // for the dicDic

        
            idx exitCond, doDebug;
                    
            bool executeStatement(idx il );
            idx expressionCompute(sStr * out, const char * phrase, idx len ) ; // computes the values for mathematicalexpressions 
            static idx expressionCallback(sLang * lg, sStr * out,  sCalc * xic, idx cur, idx cnt, idx whattodo);



            static idx exec_declarefun(sLang * lg, idx il);
            static idx exec_declarearr(sLang * lg, idx il);
            static idx exec_assignment(sLang * lg, idx il);
            static idx exec_generic(sLang * lg, idx il);

            static idx exec_debug(sLang * lg, idx il);
            static idx exec_codebreak(sLang * lg, idx il);
            static idx exec_if(sLang * lg, idx il);
            static idx exec_else(sLang * lg, idx il);
            static idx exec_loop(sLang * lg, idx il);
            static idx exec_shell(sLang * lg, idx il);
            static idx exec_include(sLang * lg, idx il);
            static idx exec_define(sLang * lg, idx il);

            static idx exec_eval(sLang * lg, idx il);
            static idx exec_parse(sLang * lg, idx il);
            static idx exec_isvar(sLang * lg, idx il);
            static idx exec_isfun(sLang * lg, idx il);

            void buildCore( void );
            void buildStdLib (void) ;


            static const char * stdlibDeclarations ;
            static idx eval_app_printf(sLang * lg, idx il);
            static idx eval_app_env(sLang * lg, idx il);

            static idx eval_dic_get (sLang * lg, idx il);
            static idx eval_dic_set (sLang * lg, idx il);


            static idx eval_string_len(sLang * lg, idx il);
            static idx eval_string_cat(sLang * lg, idx il);
            static idx eval_string_cmp(sLang * lg, idx il);
            static idx eval_string_cnt(sLang * lg, idx il);
            static idx eval_string_cntsymb(sLang * lg, idx il);
            static idx eval_string_skip(sLang * lg, idx il);
            static idx eval_string_extract(sLang * lg, idx il);
            static idx eval_string_compareuntil(sLang * lg, idx il);
            static idx eval_string_search(sLang * lg, idx il);
            static idx eval_string_crlf(sLang * lg, idx il);
            static idx eval_string_cstyle(sLang * lg, idx il);
            static idx eval_string_replacesymb(sLang * lg, idx il);
            static idx eval_string_replacestr(sLang * lg, idx il);
            static idx eval_string_cleanmarkup(sLang * lg, idx il);
            static idx eval_string_cleanends(sLang * lg, idx il);
            static idx eval_string_hungarian(sLang * lg, idx il);
            static idx eval_string_changecase(sLang * lg, idx il);
            static idx eval_string_printf(sLang * lg, idx il);
            static idx eval_string_unescape( sLang * lg, idx il);
            static idx eval_string_enumerate(sLang * lg, idx il);

            static idx eval_file_exists(sLang * lg, idx il);
            static idx eval_file_open(sLang * lg, idx il);
            static idx eval_file_gets(sLang * lg, idx il);
            static idx eval_file_read(sLang * lg, idx il);
            static idx eval_file_write(sLang * lg, idx il);
            static idx eval_file_close(sLang * lg, idx il);
            static idx eval_file_len(sLang * lg, idx il);
            static idx eval_file_getpos(sLang * lg, idx il);
            static idx eval_file_setpos(sLang * lg, idx il);
            static idx eval_file_remove(sLang * lg, idx il);
            static idx eval_file_rename(sLang * lg, idx il);
            static idx eval_file_content(sLang * lg, idx il);
            static idx eval_file_printf(sLang * lg, idx il);
            static idx eval_file_find(sLang * lg, idx il);
            static idx eval_file_makename(sLang * lg, idx il);
            static idx eval_file_timestamp(sLang * lg, idx il);
            static idx eval_file_makedir(sLang * lg, idx il);
            static idx eval_file_rmdir(sLang * lg, idx il);
            static idx eval_file_curdir(sLang * lg, idx il);

            static idx eval_online_httpget(sLang * lg, idx il);
            static idx eval_online_httpbulk(sLang * lg, idx il);


        public: // functional scope 

            class Scope { // this structure defines functional scope 
                public:
                    sStr data;//idx namOfs; 
                    sDic < idx >  dicVar; // Variables definition
                    Scope *  up, *  down, *  global;
                    idx breakCond,retCond, contCond;
                    idx lastIf; // the outcome of the last if in this scope statement 
                    idx doDebug;

                    Scope ( Scope * lup=0 , char * lnam=0 ) 
                    {
                        breakCond=0;retCond=0;contCond=0;doDebug=0;lastIf=1;
                        up=lup;down=0;global=(lup && lup->global) ? lup->global : this;
                        //namOfs=0;//namOfs=data.length();
                        if(lnam)data.printf("%s",lnam);data.add("\0\0",2); // add double zero terminated name 
                    }

                    const char * getVar( const char * var )
                    {
                        Scope * scp; if(var[0]==':' && var[1]==':'){scp=global;var+=2;}else scp=this;
                        idx * ptrOfs=scp->dicVar.get(var);if(!ptrOfs)return "";
                        return scp->data.ptr(*ptrOfs);
                    }

                    int isVar( const char * var )
                    {
                        Scope * scp; if(var[0]==':' && var[1]==':'){scp=global;var+=2;}else scp=this;
                        return scp->dicVar.get(var) ? 1 : 0 ; 
                    }
                    const char * setVar( const char * var, const char * fmt, ... );

                    ~Scope(){
                        data.empty();//idx namOfs; 
                        dicVar.empty(); // Variables definition
                    }
                };
            Scope * curScope, globalScope; 
            idx funLevel;

            idx charint(const char * nam, Scope * scp=0){ const char * par=(scp ? scp : curScope)->getVar(nam); return par ? atoi(par) : 0 ;}
            real charreal(const char * nam, Scope * scp=0){ const char * par=(scp ? scp : curScope)->getVar(nam); real val=0;if(par) sscanf(par,"%lf",&val);return val;}
            idx charcase(void){const char * vcase=curScope->getVar("case"); if(!strcmp(vcase,"upper") )return sString::eCaseHi;if(!strcmp(vcase,"lower") )return sString::eCaseLo;return atoi(vcase);}
    
    public: // construction 
        sIO * msg;
        bool constructionMode;
        sStr reslt;

        enum eFlags{
            eBuildCore        =0x00000001,
            eBuildStdLib    =0x00000002
        };
        sLang(sIO * lmsg=0, idx flags=eBuildCore|eBuildStdLib) 
        {
            msg=lmsg;
            
            //inLib=0;
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
        }
    };

}

#endif





// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Documentation
// _/
// /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

/*
        variable assignment    -   varname = value ; 

        function call          -   funcname ( parameters ) ; 
        function  definition   -   funcname ( parameters ) { body }
        language constructions -   if ( statement ) { body }
                                   while ( statement ) { body }
        canonicalized as           name ( parameters ) { body } ;
        with any of parts 
        possibly missing
*/
