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
#ifndef sdmHoneyBee_hpp
#define sdmHoneyBee_hpp

#include <slib/std.hpp>
#include <slib/utils.hpp>
#include <ulib/utype2.hpp>
#include <ulib/uquery.hpp>
#include <qlib/QPrideProc.hpp>
#include <regex.h>
#include <unistd.h>

class sdmHoneyBee
{
    private:
        struct DicRule {
                sStr value;
                regex_t re;
                int regerr;
                bool hasBeenTreated;
                DicRule(): value(sMex::fExactSize)
                {
                    memset(&re, 0, sizeof(regex_t));
                    regerr = 0;
                    hasBeenTreated = false;
                }
        };

        struct FieldsGroup {
            const char * ancestorName;
            sDic<char> names;
            idx pushIndex;

            FieldsGroup(const sUsr & usr, const sUsrType2 * type, const char * fieldNames00);
        };

        sQPrideProc * _honey;

    public:
        sdmHoneyBee(sQPrideProc * hon = 0) { _honey = hon; }
        bool registerEngineVariable (sUsrQueryEngine & engine, sUsrTypeField::EType fieldType, const char * fieldName,  const  char * value, idx multiValueCounter );
        void createDictionaryFromConfigFile (sDic < DicRule > & mapingDictionary,
            const char * controlFileSource, idx controlFileLen, const char * formatForRule , ... );
        idx computeFunction(sStr & dest, sStr * error, const char * value, const char * formula, sUsrQueryEngine & engine);
        idx parseCell (sStr &dest, sStr *error, const char *cellData, int cellLength, const char* parseWith);
        idx realExecution(const char * cfgFileName, sVec <sStr> &csvFiles, const char * ruleForCfg);
};

#endif 