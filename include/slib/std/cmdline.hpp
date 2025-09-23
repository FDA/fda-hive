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
#ifndef sLib_std_cmdline_hpp
#define sLib_std_cmdline_hpp

#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/var.hpp>

namespace slib { 
     
    class sCmdLine : public sDic <idx>
    {
        
    public:

        sCmdLine ()
            {isError=0;exeFunCallBack=0;}
        void parse(const char * cmdLine, idx len=0,const char * separ=0);

        void init(idx argc, const char ** argv,const char ** envp=0);
        void initCgi(const char * src)
            {if(src)parse(src, 0, "&=" );}
        void initV( const char * fmt, va_list marker )
            {sStr str;str.vprintf( fmt, marker );parse(str.ptr(),str.length());}
        void init(const char * fmt, ... ) __attribute__((format(printf, 2, 3)))
            {sCallVarg(initV,fmt);}
        void empty(void)
            {sDic<idx>::empty();}
        
        const char * get(idx argnum, idx * pLen=0) const
            {return static_cast<const char*>(sDic<idx>::id( *ptr(argnum), pLen));}
        char * get(idx argnum, idx * pLen=0) { return static_cast<char*>(sDic<idx>::id(*ptr(argnum), pLen)); }

        bool is(const char * name) const
        {
            idx iFnd=sDic<idx>::find(name)-1;
            return (iFnd!=sNotIdx) ? true : false;
        }
        const char * next(const char * name) const
        {
            idx iFnd=sDic<idx>::find(name)-1;
            return (iFnd>=0 && iFnd<dim()-1) ? get(iFnd+1) : 0;
        }
        template < class Tobj > Tobj * scanf(const char * name, const char * frmt, Tobj * valPtr) const
        {
            const char * val=next(name);
            if(!val)return 0;
            if(!sscanf(val,frmt,valPtr))
                return 0;
            return valPtr;
        }
        const char * operator [](idx index) const {return get(index);}
        char * operator[](idx index) { return get(index); }

        typedef idx (* exeFunType)(void * param,const char * cmd ,const  char * arg,const  char * equCmd, sVar * vars);    
        enum argType{
            argNone=0,
            argOneByOne=1,
            argAllSpacedList,
            argAllZeroList,
            argUnknown
        };
        struct exeCommand{
            void * param;
            exeFunType cmdFun;
            idx kind;
            const char * cmd ;
            const char * descr;
        } ;
        exeFunType exeFunCallBack;
        idx isError;
        idx exeFunCaller(exeCommand * cmdexe, const char * cmd ,const  char * arg,const  char * equCmd, sVar * vars);
        idx exec(exeCommand * cmds, sVar * externalVars, sStr * applog=0, sStr * dbgLog=0, char * onecmd=0);

        #define sCmdLine_DBG "debug: > "
        #define sCmdLine_EXE "executing: > "
        #define sCmdLine_MAC "macro: > "
        #define sCmdLine_WRN "warning: [%s] > "
        #define sCmdLine_ERR "error: [%s] > "
        #define sCmdLine_ACT "solution: > "

    };



}
#endif 


