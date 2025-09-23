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
#ifndef sLib_core_net_h
#define sLib_core_net_h

#include <slib/core/dic.hpp>

namespace slib
{

    template < class Tobj > class sKnot
    {
    public:
        Tobj obj;
        sLst <idx> in;
        sLst <idx> out;

        sKnot * init(sMex * mex) {
            in.init(mex);
            out.init(mex);
            return this;
        }
        Tobj & operator *( ){return obj;}

        sKnot() {  }
        sKnot(Tobj & o) { obj=o; }
        virtual ~sKnot() { }
    };



    struct sNetObj {
        idx size;
        idx ref;
        bool (* eventCallback)(idx code, void * usr, idx size);
        sLst <idx> links[2];
        sNetObj(){ }
        virtual ~sNetObj(){}
        virtual void destroy(void){}
    };
    class sNetText: public sNetObj
    {
        public:
            char * src;
    };

    class sNet : public sDic< idx >
    {
        public:
        enum {fMemObject=0x01};
        idx netFlags;
            sNet(idx lflags=0):sDic<idx>(){
                lastRef=-1;
                objStack.init(sMex::fBlockCompact);
                serialObj=0;
                netFlags=lflags;
                }

        ~sNet()
        {
            del(0,dim());
        }

        private:
            idx lastRef,serialObj;
            sVec < idx > objStack;

        public:


        enum eObjLinkType{
            eLinkNone=-2,
            eLinkLastParent=-1,
            eLinkParent2Child=0x00000001,
            eLinkChild2Parent=0x00000002,
            eLinkIn=0x00000004,
            eLinkCross=eLinkChild2Parent|eLinkParent2Child
        };
        protected:
        sNetObj * doAdd( void * usrbuf, idx usrsize=0)
        {
            sMex * mex=sDic< idx >::mex();
            idx ofs;

            sNetObj * p;
            if(netFlags&fMemObject){
                p=(sNetObj*)sNew(usrsize);
                if(p && usrbuf)memmove((void*)p,(void*)usrbuf,usrsize);
                ofs=mex->add(&p,sizeof(p));
            }else {
                ofs=mex->add(usrbuf,usrsize);
                p=(sNetObj * )mex->ptr(ofs);
            }
            idx * pofs=sDic< idx >::add();
            *pofs=ofs;

            p->size=usrsize;
            lastRef=p->ref=dim()-1;
            ++serialObj;

            return p;
        }
        sNetObj * doLink( sNetObj * p, idx linkType=eLinkCross, idx parRef=eLinkLastParent)
        {
            if(parRef==eLinkNone || linkType==eLinkNone )
                return p;

            if(parRef==eLinkLastParent){
                idx cnt=objStack.dim();
                if(cnt==0){
                    if(linkType&eLinkIn)objIn(lastRef);
                    return p;
                    }
                else parRef=objStack[cnt-1];
            }

            if(linkType&eLinkParent2Child)objLinks(eLinkParent2Child,parRef).vadd(1,lastRef);
            if(linkType&eLinkChild2Parent)objLinks(eLinkChild2Parent,lastRef).vadd(1,parRef);
            if(linkType&eLinkIn)objIn(lastRef);
            return p;
        }


        void del( idx posDel, idx cntDel=1)
        {
            for(idx i=posDel  ; i < cntDel ; ++i) {
                sNetObj *p = (sNetObj*)(sDic<idx>::mex()->ptr(*ptr(i)));
                if(netFlags&fMemObject)p=*((sNetObj **)p);
                p->destroy();
                if(netFlags&fMemObject)sDel(p);
            }
            sDic<idx>::del(posDel,cntDel);
        }

    public:

        template < class Tusr > Tusr & objAdd(Tusr * usr,idx linkType=eLinkCross, idx parRef=eLinkLastParent) {
            Tusr  * p=(Tusr *)doAdd( (void * )usr, sizeof(Tusr));
            if(!usr){new (p) Tusr; }
            doLink(p, linkType, parRef);
            return (Tusr &)*p;
        }
        sNetText * objAdd(const char * usr,idx usrsize, idx linkType=eLinkCross, idx parRef=eLinkLastParent) {
            if( !usrsize && usr)usrsize=sLen(usr);
            sNetText * p=(sNetText *)doAdd( (void * )0, usrsize+1+sizeof(sNetText));
            if(usr && usrsize)strncpy(p->src,usr, usrsize);
            doLink(p, linkType, parRef);
            return (sNetText * )p;
        }

        idx refIdx(idx ref) { return ref;}
        void objIn(idx ref=-1) {
            if(ref==-1)ref=lastRef;
            ref=refIdx(ref);
            objStack.vadd(1,ref);
        }
        void objOut(idx howmany=1) {
            idx cnt=objStack.dim()-howmany;if(cnt<0)cnt=0;
            objStack.cut(cnt);
        }

        sLst <idx> & objLinks(idx ewhat, idx ref){
            ref=refIdx(ref);
            return usrPtr(ref)->links[ewhat-1];
        }

        sNetObj * usrPtr(idx ref=-1){
            if(ref==-1)ref=lastRef;
            ref=refIdx(ref);
            idx * pofs=ptr(ref);
            if(!pofs)return 0;
            sNetObj *p = (sNetObj*)(sDic<idx>::mex()->ptr(*pofs));
            if(netFlags&fMemObject){ return *((sNetObj **)p);}
            return p;
        }

        sNetObj * usrPtr(const char * id){
            idx * pofs=get(id);
            if(!pofs)return 0;
            sNetObj *p = (sNetObj*)(sDic<idx>::mex()->ptr(*pofs));
            if(netFlags&fMemObject){ return *((sNetObj **)p);}
            return p;
        }

        void destroyLinked(idx ewhat,idx ref){
            ref=refIdx(ref);
            sLst <idx> & ll=objLinks(ewhat, ref);
            for( idx i=ll.dim()-1; i>=0 ; --i)
                del(refIdx(i));
        }

    };





}

#endif 




