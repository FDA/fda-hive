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
#ifndef sLib_core_var_hpp
#define sLib_core_var_hpp

#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/id.hpp>

namespace slib
{
    class sVar: public sDic< sMex::Pos > {
    public:
        class sVarElement {
            sVar * _varContainer;
            const char * _varName;

            public:
                sVarElement(sVar * myCont, const char * myName){_varContainer=myCont;_varName=myName;};

                template < class Tobj > Tobj * operator=(const sVec < Tobj > & vec)
                {
                    return vec.dim() ? _varContainer->inp(_varName, vec.ptr(), sizeof(Tobj) * vec.dim()) : 0;
                }

                template < class Tobj > Tobj & operator=(const Tobj & data)
                {
                    return _varContainer->inp(_varName, &data, sizeof(Tobj) );

                }

                const char * operator=(const char * val)
                {
                    return (const char *)_varContainer->inp(_varName, val, sLen(val)+1);

                }
                const char * operator+=(const char * val)
                {
                    const char * prv=_varContainer->value(_varName);
                    sStr t;
                    if(prv)t.printf("%s",prv);
                    t.printf("%s",val);
                    val=t;
                    return (const char *)_varContainer->inp(_varName, val, 0);

                }
                operator const char *() const { return _varContainer->value(_varName);}
                operator idx () const { return _varContainer->ivalue(_varName); }
                const char * operator *( ) const { return _varContainer->value(_varName); }



        };
        sVar & me(){return *this;}

    public:
        sVar() {mex()->flags|=sMex::fSetZero;}
        sVar * init (const char * flnm) {sDic< sMex::Pos >::init(flnm);mex()->flags|=sMex::fSetZero;return this;}

        static void * nonconst(const void * p) {return (((char *)0)+ ((const char *)p-((const char*)0)));}

        char * inpf(const void * data, idx datasize, const char * namfmt, ... ) __attribute__((format(printf, 4, 5)))
        {
            sStr nam; sCallVarg(nam.vprintf,namfmt);
            return inp(nam.ptr(),data,datasize);
        }

        char * inp(const char * nam , const void * data, idx datasize=0, idx namln=0)
        {
            if(!data) return 0;
            idx ofs=sDic< sMex::Pos>::mex()->pos();

            if( (datasize == 0 || datasize == 4 || datasize == 5) &&
                !strncmp((const char *) data, "null", 4) &&
                (datasize == 4 || !((const char*) data)[4]) )
            {
                data = (const void *) "";
                datasize = 1;
            }

            if(datasize) {
                sDic< sMex::Pos >::mex()->add(data,datasize);
            }
            else {
                idx datastrlen=sLen((const char *)data);
                datasize=datastrlen + 2;
                sDic< sMex::Pos >::mex()->add(0, datasize);
                char* pp=(char*)sDic< sMex::Pos >::mex()->ptr(ofs);
                memmove(pp, data, datastrlen); pp[datastrlen]=0; pp[datastrlen+1]=0;

            }
            sMex::Pos * p=(sDic< sMex::Pos >::set(nam, namln));
            p->pos=ofs;
            p->size=datasize;
            return (char*)(sDic< sMex::Pos >::mex()->ptr(p->pos));
        }

        void * inptr(const char * nam , const void * blb, idx blbsize)
        {
            sMex::Pos p;p.pos=sConvPtr2Int(blb);p.size=blbsize;
            return (void*)inp(nam,&p,sizeof(p));
        }

        sVarElement operator[](const char * nam)
        {
            return sVarElement(this,nam);
        }

        idx is(const char * nam, idx lenid=0 ) const
        {
            return sDic< sMex::Pos >::find(nam, lenid);
        }

        const void * out(const char * nam, const void * defVal=0, idx * psiz=0, idx lennam=0) const
        {
            const sMex::Pos * p=sDic< sMex::Pos >::get(nam, lennam);
            if(psiz)*psiz=p ? p->size : 0 ;
            return p ? sDic< sMex::Pos >::mex()->ptr(p->pos) : defVal;
        }
        void * out(const char * nam, void * defVal=0, idx * psiz=0, idx lennam=0)
        {
            return const_cast<void *>(const_cast<const sVar&>(*this).out(nam, const_cast<const void *>(defVal), psiz, lennam));
        }

        const void * outf(const char * namfmt, ... ) const __attribute__((format(printf, 2, 3)))
        {
            sStr nam; sCallVarg(nam.vprintf,namfmt);
            return out(nam.ptr(),0);
        }
        char * value(const char * name,const void * defVal=0, idx * psiz=0, idx lennam=0) const
        {
            return (char *)(out(name, defVal, psiz, lennam));
        }
        idx ivalue(const char * name,idx defVal=0) const
        {
            idx vval=defVal;
            const char * val=value(name);
            if(val)sscanf(val,"%" DEC,&vval);
            return vval;
        }
        udx uvalue(const char* name, udx defVal = 0) const
        {
            const char* val = value(name);
            if(val) {
                udx vval=defVal;
                sscanf(val, "%" UDEC, &vval);
                return vval;
            }
            return defVal;
        }
        real rvalue(const char * name,real defVal=0) const
        {
            real vval=defVal;
            const char * val=value(name);
            if(val)sscanf(val,"%lf",&vval);
            return vval;
        }
        bool boolvalue(const char *name, bool defVal=false) const
        {
            const char * val=value(name);
            return val ? parseBool(val) : defVal;
        }

        const char * ptrvalue(const char * name, idx * pblobsize=0,const void * defVal=0) const
        {
            sMex::Pos *p=(sMex::Pos* )value(name,defVal);
            if(!p){if(pblobsize)*pblobsize=0;return 0;}
            if(pblobsize)*pblobsize=p->size;
            return sConvInt2Ptr(p->pos, const char);
        }

        template < class Tobj > Tobj * scanf(const char * name, const char * frmt, Tobj * valPtr) const
        {
            const char * val=value(name);if(!val)return 0;
            if(!sscanf(val,frmt,valPtr))return 0;
            return valPtr;
        }


        public:
        idx serialIn(const char * list00, idx lenmax = 0)
        {
            sStr ttt;
            if( !lenmax ) {
                lenmax = sIdxMax;
            }
            for(const char * p = list00; p && p < (list00 + lenmax); p = p + sLen(p) + 1) {
                const char * psize = p + sLen(p) + 1;
                idx size = 0;
                sscanf(psize, "%" DEC, &size);
                const char * val0 = psize + sLen(psize) + 1, *val = val0;
                ttt.cut(0);
                for(idx i = 0; val < list00 + lenmax && i < size; ++val, ++i) {
                    unsigned char v = (*val >= 'A' ? *val - 'A' + 10 : *val - '0') * 16;
                    v += *++val >= 'A' ? *val - 'A' + 10 : *val - '0';
                    ttt.add((char *)&v, sizeof(v));
                }
                if( size ) {
                    inp(p, ttt.ptr(0), ttt.length());
                }
                if( val0 >= list00 + lenmax ) {
                    break;
                }
                p = val0;
            }
            return dim();
        }

        idx serialOut(sStr * buf, const char * sep = 0, bool nocode = false) const
        {
            for(idx i = 0; i < dim(); ++i) {
                const char * nm = (const char *) id(i);
                buf->printf("%s", nm);
                buf->addSeparator(sep);
                const sMex::Pos * p = ptr(i);
                buf->printf("%" DEC, p->size);
                buf->addSeparator(sep);
                const char * val = (const char *) sDic<sMex::Pos>::mex()->ptr(p->pos);
                if( nocode ) {
                    buf->add(val, p->size);
                    buf->add0();
                    buf->cut(-1);
                } else {
                    for(idx k = 0; k < p->size; ++k) {
                        buf->printf("%02X", (unsigned char)val[k]);
                    }
                }
                buf->addSeparator(sep);
            }
            buf->add0(2);
            return buf->length();
        }

    };
}
#endif
