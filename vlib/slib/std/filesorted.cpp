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
#include <slib/std/file.hpp>

using namespace slib;

sFileSorted * sFileSorted::init( const char * flnm, idx lbackuplength)
{
    backuplength=lbackuplength;
    sStr Buf; char * buf=Buf.resize(backuplength);

    // open the file
    fl=fopen(flnm,"rb");if(!fl)return 0;

    // read the first gi
    fscanf(fl,"%" DEC, &firstGI);

    // position to the end and get the size 
    fseek(fl,0,SEEK_END);
    size=ftell(fl);
    
    // position to somewhere before the end 
    if(size<backuplength)backuplength=size;
    fseek(fl,(long)(size-backuplength), SEEK_SET);
    
    
    
    fgets(buf, (int)backuplength, fl) ; // skip a string 
    
    idx pos;
    while ( !feof(fl) ){ // read until it is readeable 
        pos=ftell(fl);
        if( fscanf(fl,"%" DEC, &lastGI) >0)lastPos=pos;
        fgets(buf, (int)backuplength, fl ) ;
    }
    return this;
}

idx sFileSorted::search( idx gi)
{
    sStr Buf; char * buf=Buf.resize(backuplength);

    if(gi<firstGI)
        return (idx )(-1);
    else if(gi==firstGI)
        {fseek(fl,0,SEEK_SET);return 0;}
    else if(gi==lastGI)
        {fseek(fl,(long)lastPos,SEEK_SET);return lastPos;}
    else if(gi>lastGI)
        return (idx )(-1);

    idx i,giCur=(idx )(-1);
    idx pos=sNotIdx, posS=0;
    idx posE=size;
    idx giS=firstGI;
    idx giE=lastGI;
    idx guesspos;

    for ( i=0; posS!=posE ; ++i ){
        //guesspos= (idx) (posS+1.*( gi - giS)*(posE-posS)/ (giE-giS) );
        guesspos= (idx) (1.*posS+posE)/2;
        fseek(fl,(long)guesspos, SEEK_SET);
        fgets(buf,sizeof(buf),fl);

        pos=ftell(fl);
        if( fscanf(fl,"%" DEC, &giCur) <1 )
            break;
        if(gi == giCur) 
            {fseek(fl,(long)pos,SEEK_SET);return pos;}
        else if(gi<giCur) 
            {if(posE==pos) break;posE=pos;giE=giCur;}
        else if(gi>giCur) 
            {if(posS==pos) break;posS=pos;giS=giCur;}
    }

    fseek(fl,(long)posS, SEEK_SET);
    for( giCur=giS; giCur<gi ; ){ // read until it is readeable 
        pos=ftell(fl);
        if( fscanf(fl,"%" DEC, &giCur )<1) break;
        fgets(buf,sizeof(buf),fl);
        if(giCur==gi)
            {fseek(fl,(long)pos,SEEK_SET);return pos;}
    }
    
    return -1;
}


idx sFileSorted::search( const char * acc,const char * separ)
{
    sStr Buf; char * buf=Buf.resize(backuplength);
    sStr Acc; char * accCur=Acc.resize(backuplength);
    
    idx i;
    idx pos=sNotIdx, posS=0;
    idx posE=size;
    //idx giS=firstGI;
    //idx giE=lastGI;
    idx guesspos;
    idx res=-1;
    char * p;

    for ( i=0; posS!=posE ; ++i ){
        //guesspos= (idx) (posS+1.*( gi - giS)*(posE-posS)/ (giE-giS) );
        guesspos= (idx) ((1.*posS+posE)/2);
        fseek(fl,(long)guesspos, SEEK_SET);
        fgets(buf,sizeof(buf),fl);

        pos=ftell(fl);
        fgets(accCur,sizeof(accCur)-1, fl);
        p=strpbrk(accCur,separ); if(p)*p=0;
        res=strcasecmp(acc,accCur);
        
        if(!res) 
            {fseek(fl,(long)pos,SEEK_SET);return pos;}
        else if(res<0) 
            {if(posE==pos) break;posE=pos;}
        else //if(res>0) 
            {if(posS==pos) break;posS=pos;}
    }

    fseek(fl,(long)posS, SEEK_SET);
    for( ; res<0  ; ){ // read until it is readeable 
        pos=ftell(fl);
        fgets(accCur,sizeof(accCur)-1, fl);
        p=strpbrk(accCur,separ); if(p)*p=0;
        res=strcasecmp(accCur,acc);
        if(!res)
            {fseek(fl,(long)pos,SEEK_SET);return pos;}
    }
    
    return (idx)(-1);
}



idx sFileSorted::searchReverse( idx pos, idx gi)
{
    idx giCur=gi;
    sStr Buf; char * buf=Buf.resize(backuplength);
        
    if(pos==0)return pos;
    // backup until the gi is less than the one we look for
    while( gi==giCur ){
        // position
        if(pos>backuplength)pos-=backuplength;else pos=0;
        fseek(fl,(long)pos,SEEK_SET);
        fgets(buf,sizeof(buf),fl);
        pos=ftell(fl);
        
        // read 
        fscanf (fl, "%" DEC, &giCur);  
        fgets(buf,sizeof(buf),fl);
        // compare
        if( giCur>gi)
            return (idx)(-1);
    }
    //fseek(fl,pos,SEEK_SET);
    for( ; giCur<gi ; ){ // read until we get the first line with the gi we want
        pos=ftell(fl);
        if( fscanf(fl,"%" DEC, &giCur )<1) break;
        fgets(buf,sizeof(buf),fl);
        if(giCur==gi)
            {fseek(fl,(long)pos,SEEK_SET);return pos;}
    }
    
    return (idx )(-1);
}

