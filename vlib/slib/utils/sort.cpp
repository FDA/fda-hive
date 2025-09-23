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
#include <slib/utils/sort.hpp>
#include <slib/std.hpp>

using namespace slib;

idx sSort::sort_stringComparator(void * param, void * arr, idx i1, idx oper, idx i2 )
{
    const char * * parr=(const char * *)arr;
    idx res=strcmp(parr[i1],parr[i2]);
    if(oper==eSort_EQ && res==0)
        return true;
    else if(oper==eSort_GT && res>0)
        return true;
    else if(oper==eSort_GE && res>=0)
        return true;
    else if(oper==eSort_LT && res<0)
        return true;
    else if(oper==eSort_LE && res<=0)
        return true;
    return false;

}

idx sSort::sort_stringNaturalComparator(void * param, void * arr, idx i1, idx oper, idx i2)
{
    const char * * parr = (const char * *) arr;
    idx res = sString::cmpNatural(parr[i1], parr[i2], true);
    if( oper == eSort_EQ && res == 0 )
        return true;
    else if( oper == eSort_GT && res > 0 )
        return true;
    else if( oper == eSort_GE && res >= 0 )
        return true;
    else if( oper == eSort_LT && res < 0 )
        return true;
    else if( oper == eSort_LE && res <= 0 )
        return true;
    return false;
}

idx sSort::sort_stringNaturalCaseComparator(void * param, void * arr, idx i1, idx oper, idx i2)
{
    const char * * parr = (const char * *) arr;
    idx res = sString::cmpNatural(parr[i1], parr[i2], false);
    if( oper == eSort_EQ && res == 0 )
        return true;
    else if( oper == eSort_GT && res > 0 )
        return true;
    else if( oper == eSort_GE && res >= 0 )
        return true;
    else if( oper == eSort_LT && res < 0 )
        return true;
    else if( oper == eSort_LE && res <= 0 )
        return true;
    return false;
}

idx sSort::sort_idxDicComparator(void * param, void * arr, idx i1, idx oper, idx i2 )
{
    sDic <Lenstats> * lenstat = (sDic <Lenstats> *) arr;

    idx first=*(idx *)(lenstat->id(i1));
    idx second=*(idx *)(lenstat->id(i2));

    idx res = first - second;

    if(oper==eSort_EQ && res==0)
        return true;
    else if(oper==eSort_GT && res>0)
        return true;
    else if(oper==eSort_GE && res>=0)
        return true;
    else if(oper==eSort_LT && res<0)
        return true;
    else if(oper==eSort_LE && res<=0)
        return true;
    return false;
}

idx sSort::sort_idxDic(void * param, void * arr, idx i1, idx oper, idx i2 )
{
    sDic <idx> * idCnt = (sDic <idx> *) arr;

    idx first = *idCnt->ptr(i1);
    idx second = *idCnt->ptr(i2);

    idx res = first - second;

    if(oper==eSort_EQ && res==0)
        return true;
    else if(oper==eSort_GT && res>0)
        return true;
    else if(oper==eSort_GE && res>=0)
        return true;
    else if(oper==eSort_LT && res<0)
        return true;
    else if(oper==eSort_LE && res<=0)
        return true;
    return false;
}


void sSort::sortStringsInd(idx n, const char * * arr, idx * ind)
{
    sSort::sortCallback((sCallbackSorter)(sort_stringComparator), 0 , n, arr, ind);
    return ;

}

idx sSort::sort_stringsDicID(void * param, void * arr, idx i1, idx i2 )
{
    sDic < idx > * dic=(sDic<idx> *)param;
    return strcmp((const char * ) dic->id(1), (const char * ) dic->id(i2));
}

