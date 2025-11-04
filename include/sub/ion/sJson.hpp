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
#ifndef sLib_sJson_hpp
#define sLib_sJson_hpp

#include "sIon-birel.hpp"

namespace slib {

    class sJson: public sIonBirel
    {

        public:
            sIO * out, myOut, myErr;
            sStr vbuf;
            const char * destinationIon;
            const char * ret(idx * plen=0){if(plen)*plen=out->length();return out->ptr(0);};
            void file(const char * nm) { out->init(nm);}

        public:
            sJson (const char * baseName=0, idx lopenMode=0, sIO * lout=0, sIO * lerrIO=0)
                : sIonBirel(baseName, lopenMode, lout ? lout : & myOut, lerrIO) {
                out=lout ? lout : & myOut;
                errIO=lerrIO ? lerrIO : & myErr;
                levelOut=0;
                identOut="    ";
                levelOutCnt.init(sMex::fSetZero);
                newLineOut="\n";
                spaceOut=" ";
                quoteVar="\"";
                quoteVal="\"";
                commaOut=",";
                colonOut=":";
                arrOut="[]";
                objOut="{}";
                destinationIon=0;
                return;
            }
            ~sJson(){
                cln();
            }
            sJson * init(const char * baseName, idx lopenMode)
            {
                return (sJson*)sIonBirel::init(baseName,lopenMode);
            }

            sJson * initMem(const char * src,idx len=0, const char * dstFile=0)
            {
                if(!src)return 0;
                sIonBirel::init(dstFile ,sMex::fMapRemoveFile);
                sJax vs(0,src,len ? len : sLen (src));
                sIonBirel::parse(&vs);
                if( vs.errCode || vs.errIO.length() ) {
                        vs.errorReport(out);
                        return 0;
                    }
                return this;
            }

            sJson * initFile(const char * flnm, const char * destination=0, bool lazy=true)
            {
                destinationIon=destination;
                sStr dst; dst.printf("%s.ion",destination);

                if(!lazy || !sFile::exists(dst) || sFile::time(flnm)>sFile::time(dst) ) {
                    sIonBirel::init(destination,sMex::fMapRemoveFile);
                    sJax vs(sFlax::fUseMMap,flnm);
                    sIonBirel::parse(&vs);
                    if( vs.errCode || vs.errIO.length() ) {
                        vs.errorReport(out);
                        return 0;
                    }
                }
                else
                    sIonBirel::init(destination,0);

                return this;
            }
            typedef sIonBirel::LinkType Link;
            typedef sIon::Bucket Iterator;


            struct Node {

                sJson * ion;
                LinkType lnk;
                Bucket bucket;
                RecordResult rr[4];
                idx isRecord,cntRecord,iType,vType;
                enum eType { eUndefined, eObj, eArr, eVal};
                bool isok;

                Node (sJson * lion=0, const char * lpath=0) {ion=lion;isRecord=0;cntRecord=-1;isok=false;lnk.ixes.sub=0;lnk.ixes.atr=0;lnk.ixes.val=0;if(lpath)path(lpath);iType=eUndefined;}

                Node * path(const char * lpath=0, idx sizePath=0 , idx flags=fDictionarize|fFuncEval){
                    RecordResult rv[4];
                    idx start=ion->resolve(lpath,sizePath ? sizePath : sLen(lpath) ,flags,rv);
                    lnk.ixes.sub=rv[0].index;lnk.ixes.atr=rv[1].index;lnk.ixes.val=rv[2].index;
                    if(start>=0)isok=true;
                    isRecord=0;
                    iType=eUndefined;
                    vType=eUndefined;
                    return this;
                }


                Node * find(const char * atr, const char * val, idx atrlen=0, idx vallen=0){
                    ((sIonBirel *) ion)->look(&lnk, sIonBirel::Link_INDEX,lnk.ixes.val,atr,atrlen ? atrlen : (atr ? sLen(atr) : 0),val,vallen ? vallen :(val ? sLen(val) : 0 ),&bucket,&cntRecord);
                    if(bucket.found())isok=true;


                    return this;
                }
                idx dim(const char * atr=0) {
                    if(!this->ok())return 0;
                    if(cntRecord>=0)return cntRecord;
                    Node tempNode(ion);
                    ((sIonBirel *) ion)->look(&tempNode.lnk, sIonBirel::Link_INDEX,lnk.ixes.val,atr,atr ? sLen(atr) : 0,0,0,&tempNode.bucket,&cntRecord);
                    return cntRecord;
                }

                Node * operator ()(const char * atr, Node * el) {
                    if(!this->ok())return this;
                    el->ion=ion;

                    const char * patr=atr, * nxt=0;
                    idx ixesval=lnk.ixes.val;
                    do {
                        nxt=strchr(patr,'.');
                        idx len=nxt ? nxt-patr : sLen(patr);
                        if(patr[0]=='.' && patr[1]=='.' ) { ((sIonBirel *) ion)->look(&el->lnk, 0,0,0,0,sIonBirel::Link_INDEX,this->lnk.ixes.sub,&el->bucket); len+=1;}
                        else ((sIonBirel *) ion)->look(&el->lnk, sIonBirel::Link_INDEX,ixesval,patr,len,0,0,&el->bucket);
                        if(!el->bucket.found())break;
                        patr+=len;if(!*patr)break;else ++patr;
                        ixesval=el->lnk.ixes.val;
                    }while(*patr);

                    el->isok=el->bucket.found();
                    return el;
                }


                Node & next(bool next=1)
                {
                    Node * el=new Node();ion->nodeList.vadd(1,el);
                    if(!this->ok())return * el;
                    el->ion=ion;
                    if(next==0 ){
                        ((sIonBirel *) ion)->look(&el->lnk, sIonBirel::Link_INDEX,lnk.ixes.val,0,0,0,0,&bucket, &cntRecord);
                    }else {
                        cntRecord=1;
                        ((sIonBirel *) ion)->next(&el->lnk, &bucket );
                    }
                    el->isok=bucket.found();
                    el->bucket=bucket;
                    return * el;
                }
                Node & child(void) { return next(0);}

                Node * operator ()(idx i, Node * el) {
                    if(!this->ok())return this;
                    idx l;char atr[32];sIPrintf(atr,l,i,10);
                    return (*this)(atr, el);
                }

                Node & operator [](const char * atr) {
                    if(!this->ok())return *this;
                    Node * tempNode=new Node();ion->nodeList.vadd(1,tempNode);
                    (*this)(atr,tempNode);
                    return * tempNode;
                }

                Node & operator [](idx i) {
                    if(!this->ok())return *this;
                    idx l;char atr[32];sIPrintf(atr,l,i,10);
                    return (*this)[(const char*)atr];
                }


                Node & del(const char * atr) {
                    ((sIonBirel * )ion)->del(sIonBirel_INDEX(lnk.ixes.val), sIonBirel_STRING(atr), sIonBirel_NULL, 0,sIdxMax);
                    return *this;
                }

                Node & link(const char * atr, const char * val, bool doDel=true) {
                    Node * tempNode=new Node();ion->nodeList.vadd(1,tempNode);
                    if(!val)return *tempNode;
                    {
                        if(iType==eUndefined)
                            ensureVal();
                        if(iType==eObj || doDel) {
                            ((sIonBirel * )ion)->del(sIonBirel_INDEX(lnk.ixes.val), sIonBirel_STRING(atr), sIonBirel_NULL, 0,sIdxMax);
                        }
                    }
                    if(iType==eArr && (!atr || (atr[0]=='#' && atr[1]==0))) {
                        char bb[64]; idx len;
                        if(cntRecord<0)cntRecord=0;
                        sIPrintf(bb,len,cntRecord,10);
                        ((sIonBirel * )ion)->link(&(tempNode->lnk), sIonBirel_INDEX(lnk.ixes.val),bb,len, sIonBirel_STRING(val));
                        ++cntRecord;
                    }
                    else
                        ((sIonBirel * )ion)->link(&(tempNode->lnk), sIonBirel_INDEX(lnk.ixes.val), sIonBirel_STRING(atr), sIonBirel_STRING(val));

                    tempNode->ion=ion;
                    tempNode->iType=eVal;
                    return * tempNode;
                }

                Node & link(const char * atr, const char * val, idx vallen) {
                    Node * tempNode=new Node();ion->nodeList.vadd(1,tempNode);
                    {
                        if(iType==eUndefined)
                            ensureVal();
                        if(iType==eObj) {
                            ((sIonBirel * )ion)->del(sIonBirel_INDEX(lnk.ixes.val), sIonBirel_STRING(atr), sIonBirel_NULL, 0,sIdxMax);
                        }
                    }
                    ((sIonBirel * )ion)->link(&(tempNode->lnk), sIonBirel_INDEX(lnk.ixes.val), sIonBirel_STRING(atr), val, vallen ? vallen : sLen (val) );

                    tempNode->ion=ion;
                    tempNode->iType=eVal;
                    return * tempNode;
                }
                
                Node & remove(const char * atr) {
                    Node * tempNode=new Node();ion->nodeList.vadd(1,tempNode);
                    {
                        if(iType==eUndefined)
                            ensureVal();
                        if(iType==eObj) {
                            ((sIonBirel * )ion)->del(sIonBirel_INDEX(lnk.ixes.val), sIonBirel_STRING(atr), sIonBirel_NULL, 0,sIdxMax);
                        }
                    }
                    
                    tempNode->ion=ion;
                    tempNode->iType=eVal;
                    return * tempNode;
                }

        

                Node & linkf(const char * atr, const char * val, ... )
                {
                    sStrT buf;
                    sCallVarg(buf.vprintf,val);
                    return link(atr,buf.ptr());
                }

                Node & link(const char * atr, idx num)
                {
                    sStrT buf;buf.printf("%" DEC, num) ;
                    return link(atr,buf.ptr());
                }

                Node & link(const char * atr, real num)
                {
                    sStrT buf;buf.printf("%lf" , num) ;
                    return link(atr,buf.ptr());
                }

                Node & link(const char * atr, bool num)
                {
                    return link(atr,num ? "true" : "false");
                }

                Node & linkpercent(const char * atr, real num)
                {
                    sStrT buf;buf.printf("%.2lf%%", num) ;
                    return link(atr,buf.ptr());
                }
                Node & linkarr(const char * atr)
                {
                    Node &n=link(atr,"#");
                    n.iType=eArr;
                    return n;
                }

                Node & linkobj(const char * atr)
                {
                    Node &n=link(atr,"@");
                    n.iType=eObj;
                    return n;
                }

                void ensureVal(void)
                {
                    if(!bucket.found()) {iType=eVal; isRecord=1;return ;}
                    if(isRecord)return ;
                    idx cnt=1;
                    if(lnk.ixes.val==0){iType=eObj; return; }
                    ion->getRelationVals(&bucket, rr, &cnt, false);
                    iType=eVal;
                    if(((char*)(rr[0].body))[0]=='_') {
                        if(((char*)(rr[0].body))[1]=='#')iType=eArr;
                        else if(((char*)(rr[0].body))[1]=='@')iType=eObj;
                    }
                    vType=eVal;
                    if(((char*)(rr[2].body))[0]=='_') {
                        if(((char*)(rr[2].body))[1]=='#')vType=eArr;
                        else if(((char*)(rr[2].body))[1]=='@')vType=eObj;
                    }
                    isRecord=1;
                }

                idx subI()
                {
                    return lnk.ixes.sub;
                }
                idx atrI()
                {
                    return lnk.ixes.atr;
                }
                idx valI()
                {
                    return lnk.ixes.val;
                }

                const char * sub (idx * plen=0){
                    if(!bucket.found())return 0;
                    ensureVal();
                    if(plen)*plen=rr[0].size;
                    return (const char * ) rr[0].body;
                }
                const char * atr (idx * plen=0){
                    if(!bucket.found())return 0;
                    ensureVal();
                    if(plen)*plen=rr[1].size;
                    return (const char * ) rr[1].body;
                }
                const char * val (idx * plen=0){
                    if(!bucket.found())return 0;
                    ensureVal();
                    if(plen)*plen=rr[2].size;
                    return (const char * ) rr[2].body;
                }

                operator const char * ()
                {
                    idx l;const char * p=val(&l);if(!p)return 0;
                    idx pos=ion->jBuf.length();
                    ion->jBuf.add(p,l);ion->jBuf.add0();
                    return ion->jBuf.ptr(pos);
                }
                operator idx ()
                {
                    idx l;const char * p=val(&l);if(!p)return 0;
                    idx v;sIScanf(v,p,l,10);
                    return v;
                }
                operator bool ()
                {
                    idx l;const char * p=val(&l);if(!p)return 0;
                    if(l==4 && strcmp(p,"true"))return true;
                    else if(l==5 && strcmp(p,"false"))return true;
                    idx i;sIScanf(i,p,l,10);
                    return (bool)i;
                }
                operator real()
                {
                    idx l;const char * p=val(&l);if(!p)return 0;
                    idx r;sRScanf(r,p,l,10);
                    return r;
                }

                idx unixTime( const char * fmt="%Y-%m-%dT%H:%M:%S%z")
                {
                    idx l;const char * p=val(&l);if(!p)return 0;
                    struct tm tm;sSet(&tm,0,sizeof(tm));
                    char dt[256];strncpy(dt,p,l);strptime( dt, fmt, &tm);
                    idx tim = mktime(&tm);
                    return tim > 0 ? tim : 0 ;
                }

                bool ok(void){return isok;}

                idx copy(Node & from, const char * exclList=0) {
                    if(!from.isok) return 0;
                    return from.ion->sIonBirel::copy(from.lnk.ixes.val,(sIonBirel * )ion,lnk.ixes.val,exclList);
                }
                idx copy(sJson & js, const char * exclList=0) {
                    return ((sIonBirel*)&js)->copy(0,(sIonBirel * )ion,lnk.ixes.val,exclList);
                }

                const char * print(sStr * buf)
                {
                    ((sIonBirel*)ion)->wander("")->resetResultBuf();
                    ((sIonBirel * )ion)->iterateNodesL("", sIonBirel::Link_INDEX,subI(),(sIonBirel::callbackIterator)0,(void*)0);
                    sIO * p=((sIonBirel*)ion)->wander("")->pTraverseBuf;
                    const char * res=p->ptr(0);
                    idx l=p->length();
                    if(buf)buf->add(res,l);
                    return buf ? buf->ptr(0) : p->ptr(0);
                }

            };
            Node & node(const char * lpath, idx sizepath=0, idx flags=fDictionarize|fFuncEval){
                Node * tempNode=new Node(); nodeList.vadd(1,tempNode);
                tempNode->ion=this;
                return *tempNode->path(lpath,sizepath,flags);
            }

            Node & find(const char * atr, const char * val, idx atrlen=0, idx vallen=0){
                Node * tempNode=new Node(); nodeList.vadd(1,tempNode);
                tempNode->ion=this;
                return *tempNode->find(atr,val,atrlen,vallen);
            }

            bool find(const char * sub, const char * atr, const char * val, idx sublen=0, idx atrlen=0, idx vallen=0) {
                Link lnk;
                Bucket bucket;
                idx cntRecord=0;
                return ((sIonBirel *) this)->look(&lnk, sub,sublen ? sublen : (sub ? sLen(sub) : 0 ),atr,atrlen ? atrlen : (atr ? sLen(atr) : 0),val,vallen ? vallen :(val ? sLen(val) : 0 ),&bucket,&cntRecord) ? true : false;
            }
            Node * pathConstruct(const char * path, bool isreadonly=0, Node * prvnode=0);
            Node * pathAuto(const char * path, ... ) {
                sStr p;if(path) sCallVarg(p.vprintf,path);
                Node * node=pathConstruct(p.ptr(0)); return node ? &((*node)[".."]) : 0;
            }

            struct Node;
            sVec < Node * > nodeList;
            sStr jBuf;
            void cln(void) {
                for( idx i=0; i<nodeList.dim(); ++i ){
                    delete nodeList[i];
                }
                nodeList.destroy();jBuf.destroy();
            }

            idx copy(sJson::Node * from, const char * atrsrc, sJson * dst, sJson::Node * to, const char * atrdst=0,bool createDstnode=false, const char * exclList=0) {
                return sIonBirel::copy(from->lnk.ixes.sub,atrsrc,0,(sIonBirel * )dst,to ? to->lnk.ixes.sub : 0 , atrdst ? atrdst : atrsrc, 0,createDstnode,exclList);
            }
            idx copy(sJson::Node & from, const char * atrsrc, sJson * dst, sJson::Node * to, const char * atrdst=0,bool createDstnode=false, const char * exclList=0) {
                return sIonBirel::copy(from.lnk.ixes.sub,atrsrc,0,(sIonBirel * )dst,to ? to->lnk.ixes.sub : 0 , atrdst ? atrdst : atrsrc, 0,createDstnode,exclList);
            }
            struct Tripple: public sIon::RecordResult {

                const char * sub (idx index=0){return (const char * ) ((sIon::RecordResult * )this)[0+(3*index)].body;}
                const char * atr (idx index=0){return (const char * ) ((sIon::RecordResult * )this)[1+(3*index)].body;}
                const char * val (idx index=0){return (const char * ) ((sIon::RecordResult * )this)[2+(3*index)].body;}

                idx cnt(){return atoidx((const char * )((sIon::RecordResult * )this)[-1].body);}
                idx subLen(idx index=0){return ((sIon::RecordResult * )this)[0+(3*index)].size;}
                idx atrLen(idx index=0){return ((sIon::RecordResult * )this)[1+(3*index)].size;}
                idx valLen(idx index=0){return ((sIon::RecordResult * )this)[2+(3*index)].size;}

                idx subIdx (idx index=0){return ((sIon::RecordResult * )this)[0+(3*index)].index;}
                idx atrIdx (idx index=0){return ((sIon::RecordResult * )this)[1+(3*index)].index;}
                idx valIdx (idx index=0){return ((sIon::RecordResult * )this)[2+(3*index)].index;}

                RecordResult * s (idx index=0){return this+(0+(3*index));}
                RecordResult * a (idx index=0){return this+(1+(3*index));}
                RecordResult * v (idx index=0){return this+(2+(3*index));}

                idx subscan(const char * fmt, ... ) {
                    idx res;
                    sCallVargResPara(res,sString::xscanf,sub(),fmt);
                    return res;
                }
                idx atrscan(const char * fmt, ... ) {
                    idx res;
                    sCallVargResPara(res,sString::xscanf,atr(),fmt);
                    return res;
                }
                idx valscan(const char * fmt, ... ) {
                    idx res;
                    sCallVargResPara(res,sString::xscanf,val(),fmt);
                    return res;
                }
            };

            struct TrippleSet {
                Tripple  ds;
                Tripple  da;
                Tripple  dv;

                const char * sub (idx index=0){return (const char * ) ds.body;}
                const char * atr (idx index=0){return (const char * ) da.body;}
                const char * val (idx index=0){return (const char * ) dv.body;}

                idx lsub (idx index=0){return ds.size;}
                idx latr(idx index=0){return da.size;}
                idx lval(idx index=0){return dv.size;}

                idx isub (idx index=0){return ds.index;}
                idx iatr(idx index=0){return da.index;}
                idx ival (idx index=0){return dv.index;}
            };

            typedef idx (*jsCallbackSimpleFunc)(void * param, sJson * ion, Tripple * curResults, sIon::Bucket * cb);
            idx enumChildren(const char * cursor, const char * node, jsCallbackSimpleFunc iteratorFunc, void * param)
            {
                return iterateLevel(cursor, node,sLen(node), (sJson::callbackSimpleFunc) iteratorFunc, param);
            }
            idx enumChildren(const char * cursor, idx node, jsCallbackSimpleFunc iteratorFunc, void * param)
            {
                return iterateLevel(cursor, sIonBirel::Link_INDEX,node, (sJson::callbackSimpleFunc) iteratorFunc, param);
            }
            idx enumChildren( const char * node, jsCallbackSimpleFunc iteratorFunc, void * param)
            {
                return iterateLevel("_new", node,sLen(node), (sJson::callbackSimpleFunc) iteratorFunc, param);
            }
            idx enumChildren(idx node, jsCallbackSimpleFunc iteratorFunc, void * param)
            {
                return iterateLevel("_new", sIonBirel::Link_INDEX,node, (sJson::callbackSimpleFunc) iteratorFunc, param);
            }


            static idx tabulateFunc(void * param, sJson * js, sJson::Tripple * cur, sIon::Bucket * cb)
            {
                sVec < sJson::TrippleSet > * tbl=(sVec < sJson::TrippleSet > *)param;

                TrippleSet * t=tbl->add(1);
                t->ds=cur[0];
                t->da=cur[1];
                t->dv=cur[2];
                return 1;
            }

            idx tabulateChildren ( const char * cursor, idx node , sVec < TrippleSet > * tbl)
            {
                return iterateLevel(cursor, sIonBirel::Link_INDEX,node, (sJson::callbackSimpleFunc) sJson::tabulateFunc, (void*)tbl);
            }
            idx tabulateChildren ( const char * cursor, const char * node , sVec < TrippleSet > * tbl)
            {
                return iterateLevel(cursor, node,sLen(node), (sJson::callbackSimpleFunc) sJson::tabulateFunc, (void*)tbl);
            }

            const char * print(const char * node="$root", sStr * b=0, bool add0=true)
            {
                iterateNodes("",node);
                sIO * p=wander("")->pTraverseBuf;
                const char * res=p->ptr(0);
                idx l=p->length();
                if(b){b->add(res,l);if(add0)b->add0();wander("")->pTraverseBuf->cut(0);wander("")->resetResultBuf();}
                return b ? b->ptr(0) : p->ptr(0);

            }

            idx iteratePathVal(pathCallbackIterator func, void * param)
            {
                sIonBirel::pathCallback=func;
                sIonBirel::pathCallbackParam=param;
                iterateNodesL(wander(""), Link_BodyByIndex, 0 ,0,0);
                return 0;
            }

            Link lastLink;
            Iterator lastIterator;
            Tripple lastTripple[4];


            Tripple * look(sIon::RecordResult * sub, sIon::RecordResult * atr,  Tripple * rr=0, bool moveToNext=false, idx * pRelCnt=0, Iterator * iter=0, Link * lnk=0)
            {
                if(!lnk)lnk=&lastLink;if(!iter)iter=&lastIterator;if(!rr)rr=lastTripple;
                Link* res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(sub->index), sIonBirel_INDEX(atr->index), sIonBirel_NULL , (sIon::Bucket* ) iter);
                if(res && rr)return (Tripple*)sIonBirel::getRelationVals(iter, (sIon::RecordResult*)rr, pRelCnt, moveToNext);
                return (Tripple * ) res;
            }
            Tripple * look(idx subi, const char * atr,  Tripple * rr=0, bool moveToNext=false, idx * pRelCnt=0, Iterator * iter=0, Link * lnk=0)
            {
                if(!lnk)lnk=&lastLink;if(!iter)iter=&lastIterator;if(!rr)rr=lastTripple;
                Link* res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(subi), sIonBirel_STRING(atr) , sIonBirel_NULL , (sIon::Bucket* ) iter);
                if(res && rr)return (Tripple*)sIonBirel::getRelationVals(iter, (sIon::RecordResult*)rr, pRelCnt, moveToNext);
                return (Tripple * ) res;
            }

            Tripple * look(sIon::RecordResult * sub, sIon::RecordResult * atr, sIon::RecordResult * val , Tripple * rr=0, bool moveToNext=false, idx * pRelCnt=0, Iterator * iter=0, Link * lnk=0)
            {
                if(!lnk)lnk=&lastLink;if(!iter)iter=&lastIterator;if(!rr)rr=lastTripple;
                Link* res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(sub->index), sIonBirel_INDEX(atr->index), sIonBirel_INDEX(val->index) , (sIon::Bucket* ) iter);
                if(res && rr)return (Tripple*)sIonBirel::getRelationVals(iter, (sIon::RecordResult*)rr, pRelCnt, moveToNext);
                return (Tripple * ) res;
            }
            Tripple * look(idx subi, const char * atr,  const char * val , Tripple * rr=0, bool moveToNext=false, idx * pRelCnt=0, Iterator * iter=0, Link * lnk=0)
            {
                if(!lnk){lnk=&lastLink;}if(!iter){iter=&lastIterator;}if(!rr){rr=lastTripple;}
                Link* res;
                if(atr)res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(subi), sIonBirel_STRING(atr) , sIonBirel_STRING(val)  , (sIon::Bucket* ) iter);
                else res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(subi), sIonBirel_NULL , sIonBirel_STRING(val)  , (sIon::Bucket* ) iter);
                if(res && rr)return (Tripple*)sIonBirel::getRelationVals(iter, (sIon::RecordResult*)rr, pRelCnt, moveToNext);
                return (Tripple * ) res;
            }

            Tripple * look(idx subi, const char * atr, sVec < Tripple > * rv, bool moveToNext=false, idx * pRelCnt=0, Iterator * iter=0, Link * lnk=0)
            {
                if(!lnk)lnk=&lastLink;if(!iter)iter=&lastIterator;
                Link * res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(subi), sIonBirel_STRING(atr) , sIonBirel_NULL , (sIon::Bucket* ) iter);
                if(rv)return (Tripple*)sIonBirel::getRelationVals(iter, (sVec <sIon::RecordResult> *)rv, pRelCnt, moveToNext);
                return (Tripple * ) res;
            }

            idx scanf(idx sub, const char * atr, const char * fmt, ... ) {
                Tripple * r=look(sub, atr);
                if(!r)return 0;
                idx res;
                sCallVargResPara(res,sString::xvscanf,r->val(),fmt);
                return res;
            }

            const char * printf(idx sub, const char * atr, const char * fmt, ... ) {
                sStr s;
                const char * res;
                sCallVargResPara(res,sString::xvprintf,&s,fmt);
                link(sub,atr,s.ptr(0));
                return res;
            }

            const char * printf(sStr * str, idx sub, const char * atr, const char * fmt, ... ) {
                const char * res;
                sCallVargResPara(res,sString::xvprintf,str,fmt);
                link(sub,atr,str->ptr(0));
                return res;
            }

            Tripple * look(idx subi, const char * atr, const char * val,  sVec < Tripple > * rv, bool moveToNext=false, idx * pRelCnt=0, Iterator * iter=0, Link * lnk=0)
            {
                if(!lnk)lnk=&lastLink;if(!iter)iter=&lastIterator;
                Link * res;
                if(atr)res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(subi), sIonBirel_STRING(atr) , sIonBirel_STRING(val) , (sIon::Bucket* ) iter);
                else res=(Link*)sIonBirel::look((sIonBirel::LinkType *)lnk, sIonBirel_INDEX(subi), sIonBirel_NULL , sIonBirel_STRING(val) , (sIon::Bucket* ) iter);
                if(rv)return (Tripple*)sIonBirel::getRelationVals(iter, (sVec <sIon::RecordResult> *)rv, pRelCnt, moveToNext);
                return (Tripple * ) res;
            }

            Tripple * get(const char * path, idx pathLen=0, Tripple * rr=0, idx startFrom=sNotIdx)
            {
                if(!rr)rr=lastTripple;
                idx res=resolve( path, pathLen,(fDictionarize|fFuncEval),(sIon::RecordResult*)rr, startFrom);
                if(res!=sNotIdx)return rr;
                return 0;
            }

            const char * value(const char * path, const char * def=0, sStr * b=0, idx pathLen=0 , const char * separ=",")
            {
                return value(0, path, def, b, pathLen, separ);
            }
            const char * value(idx start, const char * path, const char * def=0, sStr * b=0, idx pathLen=0 , const char * separ=",")
            {
                Tripple * t=get(path, pathLen, 0, start);
                if(!t)return def;
                if(!b)b=&vbuf;
                idx pos=b->length();
                if(t->valLen()>=2 && t->val()[0]=='_' && t->val()[1]=='#'  ) {
                    Bucket bucket;

                    sIonBirel::LinkType lnk;
                    sIonBirel::look(&lnk,sIonBirel_INDEX(t->valIdx()), sIonBirel_NULL, sIonBirel_NULL, &bucket);
                    sIon::RecordResult rr[3];
                    for(idx i=0; bucket.found(); ++i) {
                        sIonBirel::getRelationVals(&bucket, rr, 0, true);
                        if(b->length()>pos && separ)b->add(separ,1);
                        b->add((const char*)rr[2].body,rr[2].size);
                    }
                }
                else {
                    b->add(t->val(),t->valLen());
                }
                b->add0(1);
                return b->ptr(pos);
            }

            idx ivalue(const char * path, idx def=0, idx pathLen=0 )
            {
                Tripple * t=get(path, pathLen);
                if(!t)return def;
                return atoidx(t->val());
            }

            idx ivalue(idx start, const char * path, idx def=0, idx pathLen=0 )
            {
                Tripple * t=get(path, pathLen, 0, start);
                return atoidx(t->val());
            }

            idx del(idx start, const char * atr)
            {
                sIonBirel::del(sIonBirel_INDEX(start), sIonBirel_STRING(atr), sIonBirel_NULL, 0,sIdxMax);
                return 1;
            }

            idx link(idx start, const char * atr, const char * val)
            {
                if(!val) return 0;
                sIonBirel::LinkType lnk;
                sIonBirel::del(sIonBirel_INDEX(start), sIonBirel_STRING(atr), sIonBirel_NULL, 0,sIdxMax);
                sIonBirel::link(&lnk, sIonBirel_INDEX(start), sIonBirel_STRING(atr), sIonBirel_STRING(val));
                return lnk.ixes.val;
            }

            idx linkf(idx start, const char * atr, const char * val, ... )
            {
                sStr buf;
                sCallVarg(buf.vprintf,val);
                return link(start,atr,buf.ptr());
            }

            idx linkidx(idx start, const char * atr, idx num)
            {
                sStr buf("%" DEC, num) ;
                return link(start,atr,buf.ptr());
            }

            idx linkreal(idx start, const char * atr, real num)
            {
                sStr buf("%lf", num) ;
                return link(start,atr,buf.ptr());
            }

            idx linkpercent(idx start, const char * atr, real num)
            {
                sStr buf("%.2lf%%", num) ;
                return link(start,atr,buf.ptr());
            }

            idx linkbool(idx start, const char * atr, bool num)
            {
                return link(start,atr,num ? "true" : "false");
            }

            idx linkarr(idx start, const char * atr)
            {
                return link(start,atr,"#");
            }

            idx linkobj(idx start, const char * atr)
            {
                return link(start,atr,"@");
            }

           const char * serialize(const char * path="$root"){
               iterateNodes("",path);
               return ret();
           }

            idx levelOut;

            const char * identOut;
            const char * newLineOut;
            const char * spaceOut;
            const char * quoteVar;
            const char * quoteVal;
            const char * commaOut;
            const char * colonOut;
            const char * arrOut;
            const char * objOut;

            sVec < idx > levelOutCnt;
            sVec < const char * > levelTypeArr;
            void outReset(void) {
                levelOutCnt.set(0);
            }
            void outObj(const char * var, bool arr=false) {
                const char * levelType=arr ? arrOut : objOut;

                outVar(var);

                out->add(levelType,1);
                ++levelOut;
                *levelOutCnt(levelOut)=0;
                *levelTypeArr(levelOut)=levelType;
            }

            void outObj(bool arr=false) {
                const char * levelType=arr ? arrOut : objOut;
                outNext();
                out->add(levelType,1);
                (*levelOutCnt(levelOut))++;
                ++levelOut;
                *levelOutCnt(levelOut)=0;
                *levelTypeArr(levelOut)=levelType;
            }

            void outArr(const char * var) {
                outObj(var,true);

            }
            void outRet()
            {
                if(levelOut==0)return ;
                const char * levelType=*levelTypeArr(levelOut);
                --levelOut;
                outNext(true);

                out->add(levelType+1,1);

            }
            void outNext(bool noComma=false)
            {
                idx * pcnt=levelOutCnt(levelOut);
                if(*pcnt && !noComma)
                    out->add(commaOut,1);
                if( (levelOut!=0 || *pcnt) && newLineOut)out->add(newLineOut,1);
                if(identOut){
                    for( idx i=0;i <levelOut; ++i)
                        out->add(identOut,sLen(identOut));
                }

            }
            void outVar(const char * var)
            {
                outNext();
                out->add(quoteVar,1);
                out->add(var,sLen(var));
                out->add(quoteVar,1);
                if(spaceOut){out->add(spaceOut,1);}
                out->add(colonOut,1);
                if(spaceOut){out->add(spaceOut,1);}
                (*levelOutCnt(levelOut))++;
            }
            void outVar(const char * var,const char * val)
            {
                outVar(var);
                bool isBoolValue=false;

                idx sz=sLen(val);
                if( sz==5 && strcmp(val,"false")==0)isBoolValue=true;
                else if( sz==4 && strcmp(val,"true")==0)isBoolValue=true;

                if(!isBoolValue)out->add(quoteVal,1);
                escape(out,val);
                if(!isBoolValue)out->add(quoteVal,1);

            }

            void escape(sIO * o, const char * body,  idx sz=0)
            {
                if(!sz)sz=sLen(body);
                const char * escapableCharacters = "\\\"\n\r\b\t";
                char chch[3];
                chch[0] = '\\';
                for (idx ik = 0, j; ik < sz;) {
                    for (j = ik; j < sz; ++j) {
                        if( body[j]=='\\' && body[j+1]=='u' )
                            continue;
                        if (strchr(escapableCharacters, body[j]) != 0)
                            break;
                    }
                    if (j > ik)
                        o->add(body + ik, j - ik);
                    if (j < sz) {
                        if(body[j]=='\n')chch[1]='n';
                        else if(body[j]=='\r')chch[1]='r';
                        else if(body[j]=='\b')chch[1]='b';
                        else if(body[j]=='\t')chch[1]='t';
                        else chch[1] = body[j];
                        if(body[0]!=0)
                            o->add(chch, 2);
                    }
                    ik = j + 1;
                }

            }

            void outVal(const char * val)
            {
                outNext();
                out->add(quoteVal,1);
                escape(out,val);
                out->add(quoteVal,1);
            }

            static const char * cleanIndexesFromPath(const char * path, sStr * dstB=0)
            {
                static sStr buf;
                if(!dstB){buf.cut(0);dstB=&buf;}
                idx l=sLen(path);
                char * dst=dstB->add(0,l); const char * p = path;
                for ( idx i=0; i<l; ++i,++p) {
                    if( *p=='.' && isdigit(*(p+1)) ) {
                        for(++p; *p && *p!='.' ; ++p);
                        --p;
                        continue;
                    }
                    (*dst)=*p; ++dst;
                }
                *dst=0;
                dstB->cut(dst-dstB->ptr(0));
                return dstB->ptr(0);
            }


            class CSVFlattener {
                private:


                    struct  VAL {
                        idx rows, columns;
                        idx startRow;
                        const char * val;
                        VAL(void) {sSet(this,0);}

                    };
                    struct CLN {
                        const char * val;
                        idx vLen;
                        idx ofsPrefix;
                    };
                    sDic< idx > varDic;
                    sDic< CLN > clnDic;
                    sDic< VAL > valDic;
                    sStr Path;
                    sStr Prefix;
                    idx ofsPrefix;

                    sVec < idx > colOrder;
                    VAL * precomputeGeometry(sJson::Node * parentNode);



                public:
                    idx maxRows, maxCols, iBlock;
                    void streamJsonIn(sJson::Node * parentNode, const char * startPath=0, const char * prefix=0);
                    const char * printCSV(sStr * out, const char * removePrefix =0);
                    CSVFlattener(void){iBlock=0;ofsPrefix=-1;maxRows=0;maxCols=0;}
            };
    };

    typedef sJson::Node JSNode;
    class sJsonFile: public sJson {
        public:
        sJsonFile(const char * filename, const char * baseName=0, idx lopenMode=0, sIO * lout=0, sIO * lerrIO=0)
            :sJson(baseName, lopenMode, lout, lerrIO)
        {
            sStr path;
            sFil fl(filename,sMex::fReadonly);initMem(fl,fl.length(),baseName);
        }

    };
};

    #define  sIonBirel_INDEX(_v_i)  sIonBirel::Link_INDEX,(_v_i)
    #define  sIonBirel_STRING(_v_s)  (_v_s), sLen(_v_s)
    #define  sIonBirel_AUTOVAL  0, sIonBirel::Link_AUTOVAL
    #define  sIonBirel_AUTOARR  0, sIonBirel::Link_AUTOARR
    #define  sIonBirel_NULL 0,0

#endif
