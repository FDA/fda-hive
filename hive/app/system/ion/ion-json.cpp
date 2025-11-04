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


#include "ion-tools.hpp"

sIO JerrIO(0, (sIO::callbackFun)::printf);
sIO Jout;
sJson JJson(0,0,&Jout,&JerrIO), * JJS=&JJson;


struct TableCells {
    idx value;
    idx size;
};

sDic < TableCells > biTbl;


struct MyScope {
    sStr currentPath;
    idx currentDepth;
    idx currentRow[1024];
};

idx jsonNodeIterator(void * param, sJson * js, sJson::Tripple * cur, sIon::Bucket * cb)
{
    MyScope * myscope=(MyScope *)param;
    sStr b;

    idx oldDepth=myscope->currentDepth;


    if(((char*)cur[2].body)[0]=='_')
        sIScanf((myscope->currentRow[myscope->currentDepth]),(char*)(cur[1].body),cur[1].size,10);

    myscope->currentPath.printf("%s%.*s[%" DEC "]",myscope->currentPath.length()? "." : "" , (int)cur[1].size,(const char*)cur[1].body,myscope->currentRow[myscope->currentDepth]);
    idx len=myscope->currentPath.length();

    myscope->currentDepth++;

    ::printf("BEFORE->%.*s.%.*s=%.*s\n",(int)cur[0].size,(const char *)cur[0].body,(int)cur[1].size,(const char *)cur[1].body,(int)cur[2].size,(const char *)cur[2].body);


    if(((char*)cur[2].body)[0]=='_')
        js->enumChildren("_new",cur->valIdx(), jsonNodeIterator, param);

    myscope->currentPath.cut(len);*myscope->currentPath.ptr(len)=0;
    myscope->currentDepth=oldDepth;



    return 1;
}

MyScope myscope;
idx __on_json(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm)
{


    if(sIs(cmd,"-jsonFlatten")){
        JJS->initFile(pForm->value("schema"), pForm->value("destination"), true);


        sIonWander w;
        JJS->enumChildren("cursorname","$root", jsonNodeIterator, (void*)&myscope);

    }

    return 0;
}
