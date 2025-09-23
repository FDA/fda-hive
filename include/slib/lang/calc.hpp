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
#ifndef sLib_utils_calc_hpp
#define sLib_utils_calc_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/var.hpp>

namespace slib
{
    class sCalc
    {

        public:
        struct Lexem {
            idx content;
            idx data;
            idx datasize;
            idx status; 
            enum fStatus {fReady=0x0000001,fQuoted=0x00000002,fSubComplete=0x00000004};
            idx type,precendence;
            enum Type {
                fInteger    =0x00000001,
                fReal       =0x00000002,
                fBool       =0x00000004,
                fString     =0x00000008,
                fOther      =0x00000010,
                fSubroutine =0x00000020,
                fBinary        =0x00000040,

                fNumber     =0x00000003,
                fLogical    =0x0000007

            };
        } ;

        sVec < Lexem > _lxit;
        sStr _expression;
        sMex _mex;
        Lexem * lexem(idx cur) { return (cur>=0 && cur<_lxit.dim()) ? _lxit.ptr(cur) : 0;}
        
        idx flags; 
        enum ModeFlags{
            fCaseSensitive  =0x00000001,
            fUseCaching     =0x00000002,
        };

        
        
        typedef idx (* CallbackFuncType)(void * funcpar, void * par,  sCalc * lex, idx cur, idx cnt, idx whattodo);
        enum eCallbackType {
            fCallbackFirst  =0x00000001,
            fCallGeneric    =0x00000002, 
            fCallVariables  =0x00000004, 
            fCallText       =0x00000008,
            fCallLogic      =0x00000010,
            fCallMathFunc   =0x00000020,
            fCallMath        =0x00000040,
            fCallbackLast   =0x00000080,
        
            fCallbackAll    =fCallbackFirst|fCallGeneric|fCallVariables|fCallText|fCallLogic|fCallMathFunc|fCallbackLast|fCallMath,
            
        };
        enum MathPrecendence {
            eMathNumerical=0,
            eMathBitwise,
            eMathMultiplicative,
            eMathAdditive,
            eMathComparative,
            eMathLast,
        };

    public:
        static const char * _defaultTokenizingSymbols;
        static const char * _defaultNonTokenizingSymbols;
        static const idx  _defPrecendence[];
        static const char * symbMath;
        static const char * symbText;
        static const char * symbLogic;
        static const char * symbMathFunc;

    private:

        idx _whattodo;
        const char * _lgs, * _lgsnon;
        const idx * _lgprec;

        CallbackFuncType _func;
        void * _funcparam, * _par ;

        char * _analyse();
        static sVar  varGlobal;
        sVar varTemporary;

        idx subLevel;
    public:
        sVar * variablesTemporary, * variablesGlobal;
        const char * varPrfxTemporary, * varPrfxGlobal;

    public:

        sCalc(CallbackFuncType func=0, void * funcparam=0, const char * lgs=0 , const char * lgsnon=0, const idx * lprecendence=0)
        {
            _lgs=lgs ? lgs : _defaultTokenizingSymbols;
            _lgsnon=lgsnon ? lgsnon : _defaultNonTokenizingSymbols;
            _lgprec=lprecendence ? lprecendence  : _defPrecendence ;
            _func=func;
            _funcparam=funcparam;
            variablesTemporary=&varTemporary; variablesGlobal=&varGlobal;
            varPrfxTemporary=0;varPrfxGlobal=0;
            subLevel=0;
        }
        ~sCalc()
        {
            _lxit.empty();
            _mex.empty();
        }

        idx _tokenize(const char * phrase, idx phraselen);

        char * analyse(idx whattodo, void * param, const char * phrase, idx phraselen)
        {
            _par=param;
            _whattodo=whattodo;
            if(phrase){
                _mex.empty(); 
                _lxit.empty();
                _tokenize(phrase, phraselen);
            }
            return _analyse();
        }
        void setVariableSpace( sVar * lvariablesGlobal, const char * lvarPrfxGlobal, sVar * lvariablesTemporary, const char * lvarPrfxTemporary)
        {
            variablesGlobal=lvariablesGlobal ? lvariablesGlobal  : &varGlobal;
            varPrfxGlobal=lvarPrfxGlobal;
            variablesTemporary=lvariablesTemporary ? lvariablesTemporary : &varTemporary;
            varPrfxTemporary=lvarPrfxTemporary;
        }
        const char * varValue(const char * ct, const char * val=0);
        

        char * _data(idx isda, idx num, idx siz , const char * pdata, va_list ap );

        char * cont(idx num, idx siz=0, const char * pdata=0, ... )
            { va_list ap;va_start(ap,pdata);char * res=_data(0,num,siz,pdata,ap);va_end(ap); return res;}
        char * data(idx num, idx siz=0, const char * pdata=0, ... )
            { va_list ap;va_start(ap,pdata);char * res=_data(1,num,siz,pdata,ap);va_end(ap); return res;}
        


        idx textCallback(idx cur,idx whattodo);
        idx mathCallback(idx cur, idx whattodo, idx stage);
        idx mathFuncCallback(idx cur, idx whattodo);
        idx logicCallback(idx cur, idx whattodo);
        idx genericCallback(idx cur, idx whattodo);
        idx variablesCallback(idx cur, idx whattodo);

        idx subCalc( idx cur, idx end, const char * finish00=0);

        
        idx prep(idx cur);
        
        void del (idx pos, idx cnt=1)
            {return _lxit.del(pos,cnt);};

        char * collectResult (sStr * out, idx autoSpace, const char * spcSymb, idx * spcCounter, bool isMissInfo);


        idx errnum(void);
        char * result(void)
            {return data(0);}
        idx dim(void)
            {return _lxit.dim();}



        char * debugPrint(const char * t=0);

    };

}

#endif 






