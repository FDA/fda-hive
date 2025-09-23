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

#include <slib/std/string.hpp>
#include <ctype.h>

using namespace slib;

char sString::encode64Char( unsigned char binary )
{
    static const idx  A_to_Z_offset = (idx)( 'A' );
    static const idx  a_to_z_offset = (idx)( 'a' ) - 26;
    static const idx  zero_to_nine_offset = (idx)( '0' ) - 52;
    static const char  plus =  '+' ;
    static const char  slash =  '/' ;
    static const char  equal =  '=' ;

    char  c;
    if ( binary > 61 ) {
        switch ( binary )
        {
        case 62:c = plus;break;
        case 63:c = slash;break;
        case 64:c = equal;break;
        default:c = equal;
        }
    } else if ( binary > 51 ) {
        c = (char)( binary + zero_to_nine_offset );
    } else if ( binary > 25 ) {
        c = (char)( binary + a_to_z_offset );
    } else {
        c = (char)( binary + A_to_Z_offset );
    }

    return  c;
}




#define pushdirect(_v_car)  {char cenc=(char)(_v_car);dst->add(&cenc ,sizeof(cenc));}
#define pushencode(_v_car)  {char cenc=sString::encode64Char( (unsigned char)( _v_car) );dst->add(&cenc ,sizeof(cenc));}

idx sString::encodeBase64( sMex * dst, const char * src, idx len, const bool RFC2045_compliant )
{
    if ( !len ) len=(idx)strlen(src);
    idx rlen=dst->pos();
      idx mexflags = dst->flags;
      dst->flags &= ~(sMex::fAlignInteger|sMex::fAlignParagraph|sMex::fAlignPage);

    int  byte = 0;
    idx  c, cc = 0;
    for ( idx i=0 ;  i<len ;  ++i ) {
        c = src[i];
        switch ( byte )
        {
        case 0:
            cc = ( ( c & 0xFC ) << 22 );
            cc |= ( ( c & 0x03 ) << 20 );
            byte++;
            break;
        case 1:
            cc |= ( ( c & 0xF0 ) << 12 );
            cc |= ( ( c & 0x0F ) << 10 );
            byte++;
            break;
        case 2:
            cc |= ( ( c & 0xC0 ) << 2 );
            cc |= ( c & 0x3F );
            pushencode( ( cc & 0xFF000000 ) >> 24  );
            pushencode( ( cc & 0x00FF0000 ) >> 16  );
            pushencode( ( cc & 0x0000FF00 ) >>  8  );
            pushencode( ( cc & 0x000000FF ) );
            byte = 0;
            break;
        default:
                 dst->flags=mexflags;
            return 0;
        }
    }

    switch ( byte )
    {
    case 1:
        pushencode( ( cc & 0xFF000000 ) >> 24 );
        pushencode( ( cc & 0x00FF0000 ) >> 16 );

        if ( RFC2045_compliant ) {
            pushdirect('=');
            pushdirect('=');
        }
        break;
    case 2:
        pushencode( ( cc & 0xFF000000 ) >> 24 );
        pushencode( ( cc & 0x00FF0000 ) >> 16 );
        pushencode( ( cc & 0x0000FF00 ) >>  8 );
        if ( RFC2045_compliant ) {
            pushdirect( '=' );
        }
        break;
    case 0:
        break;
    default:
        dst->flags=mexflags;
        return 0;
    }

    dst->flags=mexflags;
    return  dst->pos()-rlen;
}





idx sString::decodeBase64( sMex * dst, const char * src, idx len )
{
    if ( !len ) len=(idx)strlen(src);
    idx rlen=dst->pos();
      idx mexflags = dst->flags;
      dst->flags &= ~(sMex::fAlignInteger|sMex::fAlignParagraph|sMex::fAlignPage);

    static const int  A_to_Z_offset = (int)( 'A' ) * -1;
    static const int  a_to_z_offset = 26 - (int)( 'a' );
    static const int  zero_to_nine_offset = 52 - (int)( '0' );

    int  byte = 0;
    idx  c, cc = 0;
    for ( idx i=0; i<len;  ++i) {
        if ( isspace( src[i]) )
            continue;
        c = (int)src[i];


        switch( c )
        {
        case '+':
            c = 62;
            break;
        case '/':
            c = 63;
            break;
        case '=':
        case '-':
        case 0:
            c = 64;
            break;
        default:
            if ( 'A' <= c && c <= 'Z' ) {
                c += A_to_Z_offset;
            } else if ( 'a' <= c && c <= 'z' ) {
                c += a_to_z_offset;
            } else if ( '0' <= c && c <= '9' ) {
                c += zero_to_nine_offset;
            } else {
                c = 64;
            }
        }

        if ( c == 64 )
            break;

        switch ( byte )
        {
        case 0:
            cc = ( ( c & 0x3F ) << 18 );
            byte++;
            break;
        case 1:
            cc |= ( ( c & 0x3F ) << 12 );
            byte++;
            break;
        case 2:
            cc |= ( ( c & 0x3F ) << 6 );
            byte++;
            break;
        case 3:
            cc |= ( c & 0x3F );

            pushdirect( ( cc & 0x00FF0000 ) >> 16 );
            pushdirect( ( cc & 0x0000FF00 ) >> 8 );
            pushdirect( ( cc & 0x000000FF ) );

            byte = 0;
            break;
        default:
            dst->flags=mexflags;
            return 0;
        }
    }
    switch ( byte )
    {
    case 1:
        pushdirect( ( cc & 0x00FF0000 ) >> 16 );
        break;
    case 2:
        pushdirect( ( cc & 0x00FF0000 ) >> 16 );
        {
            char  last_char = (char)( ( cc & 0x0000FF00 ) >>  8 );
            if ( 0 != last_char ) {
                pushdirect( last_char );
            }
        }
        break;
    case 3:
        pushdirect( ( cc & 0x00FF0000 ) >> 16 );
        pushdirect( ( cc & 0x0000FF00 ) >>  8 );
        {
            char  last_char = (char)( cc & 0x000000FF );
            if ( 0 != last_char ) {
                pushdirect( last_char );
            }
        }
        break;
    case 0:
        break;
    default:
        dst->flags=mexflags;
        return 0;
    }

    dst->flags=mexflags;
    return  dst->pos()-rlen;
}


idx sString::IPDigest(const char * ptr)
{
    idx cgiIP=0, byte;
    int shift=0;
    for ( const char * p=ptr-1; p; p=strchr(p,'.')){
        byte=0;++p;
        if(!sscanf(p,"%" DEC,&byte))break;
        byte&=0xFF;byte<<=shift;
        cgiIP|= byte;
        shift+=sizeof(char)*8;
    }
    return cgiIP;
}

idx sString::fuzzyStringCompareDynamat(const char * string1, idx str1Len, const char * string2, idx str2Len, sVec <idx> * matrix )
{
    static idx conventionalPenaltyScore =-1, conventionalMisMatch =-1, conventionalMatchScore = 1;
    matrix->resize(str1Len*str2Len);
    idx * m=matrix->ptr(0);

    idx highestScore = -sIdxMax;
    idx cur_score;
    idx levelp = 0;
    idx prevlevel = 0;

    for (idx i=0; i< str1Len; ++i) {

        for (idx j=0; j< str2Len; ++j) {

            cur_score = ( (i==0 || j==0 ) ? 0 : m[prevlevel+j-1] ) + ((string1[i]==string2[j]) ? conventionalMatchScore : conventionalMisMatch);
            if(j>0) {
                idx leftScore = m[levelp+j-1] + conventionalPenaltyScore;
                if(cur_score<leftScore)
                    cur_score=leftScore;
            }
            if(i>0) {
                idx topScore = m[prevlevel+j] + conventionalPenaltyScore;
                if(cur_score<topScore)
                    cur_score=topScore;
            }

            m[levelp + j]= cur_score;
            if(highestScore<cur_score)
                highestScore=cur_score;
        }
        prevlevel=levelp;
        levelp+=str2Len;
    }
    return highestScore;
}
