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
#include <ion/sIon-hc.hpp>



void sIonHC::attachTypeIterators( sIO * out)
{
    sIonWander * w;
    bool lazy=false;
    w=addWander("_inherit", out);w->traverseCompile("b=find.rel(atr=\"_type\",val=\"type\");a=find.rel(sub=b.sub,atr=\"name\",val=\"$val\");hc=find.rel(sub=a.sub);", 0, 0, true,lazy);
    w=addWander("?type-type", out);w->traverseCompile("t=find.rel(atr=\"_type\",val=\"type\");n=find.rel(sub=t.sub,atr=\"name\",val=\"$type\");hc=find.rel(val=n.sub);", 0, 0, true,lazy);
    w=addWander("?type", out);w->traverseCompile("t=find.rel(atr=\"_type\",val=\"type\");hc=find.rel(val=t.sub);", 0, 0, true,lazy);
}


const char * sIonHC::typeExpansionIQL=
    "t=find.rel(atr=\"_type\",val=\"type\");"
    "n=find.rel(sub=t.sub,atr=\"name\");"
    "o=jump.f(n.val,-l.1);"
    "i=find.rel(sub=n.sub,atr=\"_inherit\");"
    "a=jump!.tn(i.val,\"^#\");"
    "w=find.rel(sub=i.val);"
    "tn=find.rel(atr=\"name\",val=w.val);"
    "jump.o(1,1);";

idx sIonHC::getInheritedTypes( const char * typenames, idx lentp, sIO * out )
{
    if(!lentp){lentp=sLen(typenames);}
    iql.cut(0);
    iql.printf("l=foreach(%.*s);"
        "%s"
        "f=print(-n.val);"
        ,(int)lentp,typenames,typeExpansionIQL);
    sIonWander * w=addWander("?");
    w->traverseCompile(iql.ptr(),iql.length(),errIO,true);
    w->pTraverseBuf=out;
    return w->traverse();
}




idx sIonHC::realObjectIteratorCallback(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout )
{

    if(inorout!=1)
        return sIonBirel::eRactionContinue;
    if( level->reslist->atrLen != 3)
        return  sIonBirel::eRactionContinue;
    const char * a=level->reslist->atr;
    if(a[0]!='_' || a[1]!='i' || a[2]!='d')
        return sIonBirel::eRactionContinue;

    sDic <char > * dic=(sDic <char > * ) param;
    char * b=dic->set(level->reslist->val,level->reslist->valLen);
    if(*b)
        return sIonBirel::eRactionReturn;
    else *b=1;
    return
        sIonBirel::eRactionContinue;
}


void sIonHC::attachObjectListTraverseQuery( sIO * out)
{
    bool lazy=false;
    sIonWander * w;
    w=addWander("?obj-type-id",out);w->traverseCompile("t=find.rel(atr=\"_type\",val=$type);i=find.rel(sub=t.sub,atr=\"_id\",val=$id);hc=find.rel(val=i.sub);", 0, out, true,lazy);
    w=addWander("?obj-type",out);w->traverseCompile("t=find.rel(atr=\"_type\",val=$type);i=find.rel(sub=t.sub,atr=\"_id\");hc=find.rel(val=t.sub);", 0, out, true,lazy);
    w=addWander("?obj-id",out);w->traverseCompile("i=find.rel(atr=\"_id\",val=$id);hc=find.rel(val=i.sub);", 0, out, true,lazy);
    w=addWander("?obj",out);w->traverseCompile("i=find.rel(atr=\"_id\");hc=find.rel(val=i.sub);", 0, out, true,lazy);

}



idx sIonHC::iterateObjects( const char * typenames, idx typenameslen, const char * ids, idx idslen, const char * baseline)
{
    if(!typenames || !(*typenames))typenames=",";
    if(!ids || !(*ids))ids=",";
    if(!typenameslen)typenameslen=sLen(typenames);
    if(!idslen)idslen=sLen(ids);

    sDic  < char > iddic;iddic.flagOn(sMex::fSetZero);

    if((outFlags&sFlag(fOutJson)))
        outFlags|=sFlag(fOutJsonNaked);

    const char * wanderid=0;
    idx res=0,cntr=0;
    sIonWander * w;
    for(const char * tp=typenames, *tpe=tp+typenameslen, *tpnext; tp<tpe && *tp; ++tpnext, tp=tpnext ) {

        for(tpnext=tp; tpnext<tpe && *tpnext!=0 && *tpnext!=',' && *tpnext!='\n'; ++tpnext);

        idx tpAlen=tpnext-tp;
        const char * tpA=typeAliasDic.dim() ? typeAliasDic.value(tp,0,&tpAlen,tpnext-tp) : tp, * tpAE=tpe;
        if(tpnext>tp){
            if(*tp=='+') {
                tbuf.cut(0);
                tpAlen=tpnext-tp-1;
                getInheritedTypes(tp+1, tpAlen, &tbuf );
                if(tbuf.length()){tpA=typeAliasDic.inp(tp,tbuf.ptr(0),tbuf.length(),tpnext-tp);tpAlen=tbuf.length();}
                else  { tpA=typeAliasDic.inp(tp,"+",1,tpnext-tp);tpAlen=1; }
                tbuf.cut(0);
            }
            if(!tpA || *tpA=='+') {tpA=tp+1;tpAE=tpnext;}
            else {tpAE=tpA+tpAlen; }
        }

        for( const char * tpI=tpA, * tpAnext; tpI<tpAE; ++tpAnext, tpI=tpAnext ) {
            for(tpAnext=tpI; tpAnext<tpAE && *tpAnext!=0 && *tpAnext!=',' && *tpAnext!='\n'; ++tpAnext);

            for(const char * id=ids, * ide=id+idslen, * idnext; id<ide && *id; ++idnext, id=idnext ) {

                for(idnext=id; *idnext && *idnext!=',' && *idnext!='\n'; ++idnext);

                if(strcmp(baseline,"?obj")==0) {
                    if(tpAnext>tpA){
                        if(idnext>id){wanderid="?obj-type-id";}
                        else {wanderid="?obj-type";}
                    }else  {
                        if(idnext>id){wanderid="?obj-id";}
                        else {wanderid="?obj";}
                    }
                } else if(strcmp(baseline,"?group")==0) {
                    if(idnext>id){wanderid="?group-id";}
                    else {wanderid="?group";}
                }
                else if(strcmp(baseline,"?type")==0) {
                    if(tpAnext>tpA){
                        wanderid="?type-type";
                    }else  {
                        wanderid="?type";
                    }
                } else wanderid=baseline;

                w=wander(wanderid);
                if(tpAnext>tpA)
                    w->setSearchTemplateVariable("$type",5,tpI,tpAnext-tpI);
                if(idnext>id)
                    w->setSearchTemplateVariable("$id",3,id,idnext-id);
                if(cntr==0 && (outFlags&sFlag(fOutJsonNaked)) ) {
                    w->pTraverseBuf->add("{\n" ,2);
                    w->pTraverseBuf->callback(w->pTraverseBuf->ptr(0));
                }
                firstTraverseQueryID=wanderid;
                res+=iterateNodes( Link_BodyByIndex, sConvPtr2Int(Link_IONBIREL_ROOT), objectIteratorCallback, (void * )&iddic);
                ++cntr;
            }
        }
    }

    if(res && (outFlags&sFlag(fOutJsonNaked)) ) {
        w->pTraverseBuf->add("\n}" ,2);
        w->pTraverseBuf->callback(w->pTraverseBuf->ptr(0));
    }
    return res;
}



idx sIonHC::iterateObjectsIQL( const char * typenames,  idx typenameslen, const char * ids, idx idslen)
{
    if(typenames && *typenames ) {
        bool typeexpand=false;
        if(typenames[0]=='+') {
            ++typenames;
            typeexpand=true;
        }

        if(typeexpand) tbuf.printf(0,
            "l=foreach(%.*s);"
            "%s"
            "f=find.rel(atr=\"_type\",val=-n.val);"
            ,(int)typenameslen,typenames,typeExpansionIQL);
        else tbuf.printf(0,
            "n=foreach(%.*s);"
            "f=find.rel(atr=\"_type\",val=-n.1);"
            ,(int)typenameslen,typenames);


        if(ids && *ids) tbuf.printf(
            "ii=foreach(%.*s);"
            "id=find.rel(sub=f.sub,atr=\"_id\",val=ii.1);"
            "hc=find.rel(val=id.sub);"
            ,(int)idslen,ids);
        else tbuf.printf(
            "id=find.rel(sub=f.sub,atr=\"_id\");"
            "hc=find.rel(val=id.sub);"
            );

    }else {
        if(ids && *ids) tbuf.printf(
            "ii=foreach(%.*s);"
            "id=find.rel(atr=\"_id\",val=ii.1);"
            "hc=find.rel(val=id.sub);"
            ,(int)idslen,ids);
        else tbuf.printf(
            "id=find.rel(atr=\"_id\");"
            "hc=find.rel(val=id.sub);"
            );
    }

    sIonWander *w=addWander("__");
    w->resetCompileBuf();
    w->traverseCompile(tbuf.ptr(),tbuf.length(),errIO,true);
    return iterateNodes( Link_BodyByIndex, sConvPtr2Int(Link_IONBIREL_ROOT));
}




idx sIonHC::userIteratorCallback(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout )
{
    if( inorout==1 && level->reslist->atrLen == 5 &&
        memcmp(level->reslist->atr,"_type",5)==0 && memcmp(level->reslist->val,"hc_user", 7)!=0 )
        return sIonBirel::eRactionReturn;
    return
        realObjectIteratorCallback(param, level, birel, wander, inorout );
}

void sIonHC::attachGroupListListTraverseQuery (sIO * out)
{

    sIonWander * w;
    bool lazy=false;
    w=addWander("?group",out);w->traverseCompile(
        "g=find.rel(atr=\"_type\",val=\"$type\");"
        "hc=find.rel(sub=g.sub);"
        , 0, out, true,lazy);
    w=addWander("?group-id",out);w->traverseCompile(
        "t=find.rel(atr=\"_type\",val=\"$type\");"
        "m=find.rel(sub=t.sub,atr=\"members\");"
        "c=find.rel(sub=m.val,val=\"$id\");"
        "i=find.rel(sub=t.sub,atr=\"_id\");"
        "hc=find.rel(val=i.sub);"
        , 0, out, true,lazy);

}


void sIonHC::attachUserIterators( sIO * out)
{
    sIonWander * w;
    bool lazy=false;
    w=addWander("members", out);w->traverseCompile("a=find.rel(atr=\"_id\",val=\"$val\");hc=find.rel(sub=a.sub);", 0, 0, true,lazy);
}




idx sIonHC::addRule( const char * act, const char * party, const char * obj, const char * infparty, const char * infobj )
{
    sIonBirel::LinkType rul;
    const char * keyP[5]={act,party,obj,infparty, infobj};
    const char * key[5]={"#act","#party","#obj","_infect.#party", "_infect.dodo.#obj"};
    sStr b;

    link(&rul, sIonBirel_STRING("$root.rules"), sIonBirel_AUTOVAL, sIonBirel_AUTOVAL );
    idx cnt=0;
    for ( idx i=0; i<sDim(key); ++i ) {
        if(!keyP[i])continue;
        sString::searchAndReplaceSymbols(&b,keyP[i],0,",",0,0,true,true,true,true);

        set( rul.ixes.val, key[i], b.ptr(0), 0, 0);

        b.cut(0);
    }
    return cnt;
}













