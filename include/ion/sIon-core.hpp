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
#include <ion/sIon.hpp>


#define sIon_ensureContainerIon(_v_ion,_v_pRecordType, _v_container) if( !(_v_container=&((_v_ion)->typeContainers.ptr((_v_pRecordType)->typeIndexContainer)->_v_container))->ok() ) { \
    sStr flnm(sMex::fExactSize); \
    flnm.printf(0,"%s-%s-" #_v_container"%s",(_v_ion)->baseName.ptr(),(const char *)((_v_ion)->baseContainer.ptr((_v_pRecordType)->nameOfsContainer)),(_v_ion)->extension); \
    _v_container->init( flnm , (_v_ion)->openMode ); \
}

#define sIon_ensureContainer(_v_pRecordType, _v_container) sIon_ensureContainerIon(this,_v_pRecordType, _v_container)



#define sIon_ensureAssociatedContainersIon(_v_ion, _v_ac, _v_pRecordType, _v_containerLabels ) { \
        _v_ac=&((_v_ion)->typeContainers[(_v_pRecordType)->typeIndexContainer].associatedContainers); \
        if(_v_containerLabels && (_v_ac)->dim()==0 ) { \
            (_v_ac)->add(); \
            sStr flnm(sMex::fExactSize); \
            for( const char * p=_v_containerLabels, * nxt; *p !=0; p=nxt) { \
                nxt=strchr(p,'-'); \
                if(!nxt)nxt=p+sLen(p); \
                sMex * associated=(_v_ac)->get(p,nxt-p); \
                if(associated )continue; \
                associated=(_v_ac)->set(p,nxt-p); \
                flnm.printf(0,"%s-%s-#%.*s%s",(_v_ion)->baseName.ptr(),(const char *)(_v_ion)->baseContainer.ptr((_v_pRecordType)->nameOfsContainer),(int)(nxt-p),p,(_v_ion)->extension); \
                associated->init(flnm,(_v_ion)->openMode); \
                if(*nxt=='-')++nxt; \
            } \
        }\
    }



#define sIon_ensureHashContainer(_v_pRecordType, _v_container, _v_keyParam,_v_keyfunc,_v_bucketContainer) if( !(_v_container=&(typeContainers.ptr((_v_pRecordType)->typeIndexContainer)->_v_container))->ok() ) { \
    sStr flnm(sMex::fExactSize); \
    flnm.printf(0,"%s-%s-" #_v_container"%s",baseName.ptr(),(const char *)baseContainer.ptr((_v_pRecordType)->nameOfsContainer),extension ); \
    _v_container->init( flnm , openMode|openHashMode); \
    _v_container->collisionReducer=2; \
    _v_container->keyParam=(_v_keyParam); \
    _v_container->keyfunc=(_v_keyfunc); \
    _v_container->bucketContainer=(_v_bucketContainer); \
    _v_container->reversed=(_v_pRecordType)->flags&sFlag(RelationType::bRelationReverseLookup) ? true : false; \
}

#define sIon_fixRecordFromCharBuf(_v_pRecordType, _v_recordBody, _v_recordSize, _v_voidBuf) {\
        static idx ival;real rval; \
        if(_v_pRecordType->cType==sIon::eCTypeReal ) {\
            memcpy(_v_voidBuf,_v_recordBody,_v_recordSize);_v_voidBuf[_v_recordSize]=0; \
            rval=atof(_v_voidBuf); \
            _v_recordBody=(const void*) &rval; \
            _v_recordSize=sizeof(rval); \
        } \
        else if(_v_pRecordType->cType==sIon::eCTypeIdx ) { \
            sIScanf(ival , _v_recordBody,_v_recordSize , 10 ); \
            _v_recordBody=(const void*) &ival; \
            _v_recordSize=sizeof(ival); \
        } \
        else if(_v_pRecordType->cType==sIon::eCTypeIdxRange ) { \
            sIHiLoScanf(ival , _v_recordBody,_v_recordSize , 10 ); \
            _v_recordBody=(const void*) &ival; \
            _v_recordSize=sizeof(ival); \
        } \
        else { \
            char * d=_v_voidBuf; \
            _v_recordSize=sAbs((_v_recordSize)); \
            for( idx is=0; is<_v_recordSize ; ++is) { \
                char ch=*((char*)(_v_recordBody)+is); \
                if(strchr(" \r\n\t",ch)) { \
                    if(d==_v_voidBuf || *(d-1)==' ')continue; \
                    ch=' '; \
                } \
                *d=ch; \
                ++d; \
            } \
            if(d!=0 && *(d-1)==' ') \
                --d; \
            *d=0; \
            _v_recordBody=_v_voidBuf; \
            _v_recordSize=d-_v_voidBuf;\
        }\
    }


#define sIon_fixHasherIndex(_v_relationHasherIndexType, _v_pRelationTargetsInHash) \
    { \
    if(_v_relationHasherIndexType>=0) { \
        idx encounters=0,i,realRelationHasher=0; \
        for( i=0; encounters<_v_relationHasherIndexType+1 ; ++i){ \
            if(_v_pRelationTargetsInHash[i]==sNotIdx){ \
                ++encounters; \
                ++i; \
                realRelationHasher=i; \
                if(_v_pRelationTargetsInHash[i]==sNotIdx) \
                    break; \
            } \
        } \
        _v_relationHasherIndexType=realRelationHasher; \
    }else _v_relationHasherIndexType=-_v_relationHasherIndexType; \
    }


#define sIon_outTextBody(_v_buf, _v_pRecordCType, _v_pRecordBody, _v_sz, _v_internal_sep ) { \
    if((_v_pRecordCType)==sIon::eCTypeIdx) { \
            (_v_buf)->printf("%" DEC,*(idx*)(_v_pRecordBody)); \
        } \
        else if((_v_pRecordCType)==sIon::eCTypeIndexOnly) { \
            (_v_buf)->printf("%" DEC,*(idx*)(_v_pRecordBody)); \
        } \
        else if((_v_pRecordCType)==sIon::eCTypeReal) { \
            (_v_buf)->printf("%lf",*(real*)(_v_pRecordBody)); \
        }else if((_v_pRecordCType)==sIon::eCTypeIdxRange) { \
            (_v_buf)->printf("%" DEC "%c%" DEC "", (idx)((int *)(_v_pRecordBody))[1], (_v_internal_sep) , (idx)((int *)(_v_pRecordBody))[0] ); \
        }else if((_v_pRecordCType)==sIon::eCTypeString) { \
            if( *(char*)(_v_pRecordBody)!=0 ) {\
                idx il=0; \
                for(;il<(_v_sz) && ((char *)(_v_pRecordBody))[il]!=0;)++il; \
                if(il)(_v_buf)->add((const char *)(_v_pRecordBody),il); \
            }\
        }else { \
            (_v_buf)->add("0x",2); \
            char a[2]; \
            for( idx ic=0; ic<(_v_sz); ++ic) { \
                unsigned char ch=((const unsigned char*)(_v_pRecordBody))[ic]; \
                a[0]=sMex::_hexCode[(ch>>4)];a[1]=sMex::_hexCode[ch&0xF]; \
                (_v_buf)->add(a,2); \
            } \
        } \
    }

#define sIon_outTextBodyInternal(_v_buf, _v_pRecordCType, _v_pRecordBody, _v_sz ) { \
    if((_v_pRecordCType)==sIon::eCTypeIdx) { \
            (_v_buf)->printf("%" DEC,*(idx*)(_v_pRecordBody)); \
        } \
        else if((_v_pRecordCType)==sIon::eCTypeReal) { \
            (_v_buf)->printf("%lf",*(real*)(_v_pRecordBody)); \
        }else if((_v_pRecordCType)==sIon::eCTypeIdxRange) { \
            (_v_buf)->printf("%" DEC ":%" DEC "", (idx)((int *)(_v_pRecordBody))[1], (idx)((int *)(_v_pRecordBody))[0] ); \
        }else if((_v_pRecordCType)==sIon::eCTypeString) { \
            if( *(char*)(_v_pRecordBody)!=0 ) {\
                idx il=0; \
                for(;il<(_v_sz) && ((char *)(_v_pRecordBody))[il]!=0;)++il; \
                if(il) { \
                    (_v_buf)->add("\"",1); \
                    (_v_buf)->add((const char *)(_v_pRecordBody),il); \
                    (_v_buf)->add("\"",1); \
                } \
            }\
        }else { \
            (_v_buf)->add("0x",2); \
            char a[2]; \
            for( idx ic=0; ic<(_v_sz); ++ic) { \
                unsigned char ch=((const unsigned char*)(_v_pRecordBody))[ic]; \
                a[0]=sMex::_hexCode[(ch>>4)];a[1]=sMex::_hexCode[ch&0xF]; \
                (_v_buf)->add(a,2); \
            } \
        } \
    }



#define sIon_bodyStartsWith(_v_r, _t_1, _t_2 ) { \
        const char * b1=(const char*)((_t_1)->body); \
        const char * b2=(const char*)((_t_2)->body); \
        for ( idx i=0, m=sMin(((_t_1)->size), ((_t_2)->size) ); i<m; ++i ) { \
            _v_r=b1[i]-b2[i]; \
            if(_v_r)break; \
        } \
    }

#define sIon_bodyCmp(_v_r, _t_1, _t_2 ) { \
        const char * b1=(const char*)((_t_1)->body); \
        const char * b2=(const char*)((_t_2)->body); \
        for ( idx i = 0, m=sMin(((_t_1)->size), ((_t_2)->size) ); i<m; ++i ) { \
            _v_r=b1[i]-b2[i]; \
            if(_v_r)break; \
        } \
        if ( !_v_r ) \
            _v_r = (_t_1)->size - (_t_2)->size; \
    }

#define sIon_numericCmp(_v_r, _t_1, _t_2 , _v_base ) { \
        idx n1=0,n2=0; \
        for ( idx i=0; i< (_t_1)->size && isdigit(*(const char*)((_t_1)->body)) ; ++i ) { \
            n1=n1*(_v_base)+(((const char*)((_t_1)->body))[i]-'0'); } \
        for ( idx i=0; i< (_t_2)->size && isdigit(*(const char*)((_t_2)->body)); ++i ) { \
            n2=n2*(_v_base)+(((const char*)((_t_2)->body))[i]-'0'); } \
       _v_r=n1-n2; \
    }


#define sIon_numericCmpHex(_v_r, _t_1, _t_2 ) { \
        idx n1=0,n2=0; \
        const char * b=(const char*)((_t_1)->body); \
        char ch; \
        for ( idx i=0; i< (_t_1)->size && (ch=b[i]-'0')>=0 && ch<=9 ; ++i ) { \
            n1=(n1<<4)+ch; } \
        b=(const char*)((_t_2)->body); \
        for ( idx i=0; i< (_t_2)->size && (ch=b[i]-'0')>=0 && ch<=9 ; ++i ) { \
            n2=(n2<<4)+ch; } \
       _v_r=n1-n2; \
    }

#define sIon_numerizeRecord(_v_r, _t_1 , _v_base ) { \
        _v_r=0; \
        char ch; \
        idx hi=0; \
        for ( idx i=0; i< (_t_1)->size && (isdigit(   (ch=(((const char*)((_t_1)->body))[i]))    ) || ch==':' ) ; ++i ) { \
            if(ch==':') {hi=(_v_r)<<32; (_v_r)=0; continue;} \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
        } \
        _v_r|=hi; \
    }


#define sIon_numerizeRecordReal(_v_r, _t_1 , _v_base ) { \
        _v_r=0; \
        real vorder=0; \
        char ch; \
        for ( idx i=0; i< (_t_1)->size && (isdigit(   (ch=(((const char*)((_t_1)->body))[i]))    )  || ch=='.')  ; ++i ) { \
            if(ch=='.'){vorder=1; continue;} \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
            if(vorder)vorder*=(_v_base); \
        } \
        if(vorder)_v_r/=vorder; \
    }

#define sIon_numerizeInBufBasedOnType(_v_ctype,_vv_t,_v_base,_v_buf)  {\
    (_vv_t)->cType=(_v_ctype); \
    if((_v_ctype)==sIon::eCTypeReal ) { \
        real r; sIon_numerizeRecordReal( r , (_vv_t) , (_v_base) ); \
        (_vv_t)->size=sizeof(r); \
        idx ofs=(_v_buf)->add(&r,sizeof(r)) ; \
        (_vv_t)->body=sConvInt2Ptr( ofs, void ); \
    }else if((_v_ctype)==sIon::eCTypeIdx || (_v_ctype)==sIon::eCTypeIdxRange ) { \
        idx r; sIon_numerizeRecord( r , (_vv_t) , (_v_base) ); \
        (_vv_t)->size=sizeof(r); \
        idx ofs=(_v_buf)->add(&r,sizeof(r)) ; \
        (_vv_t)->body=sConvInt2Ptr( ofs, void ); \
    } \
}
