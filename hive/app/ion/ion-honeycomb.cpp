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
#include <ulib/ulib.hpp>

sIO errIO(0, (sIO::callbackFun)::printf);
sIO out(0, (sIO::callbackFun)::printf);
sIonHC IONBirel(0,0,&out,&errIO), * RION=(sIonHC *)&IONBirel;



#define  isAtr(_v_atr) (sLen(_v_atr)==level->reslist->atrLen && strncasecmp(level->reslist->atr,_v_atr,level->reslist->atrLen )==0)
#define  isSub(_v_sub) (sLen(_v_sub)==level->reslist->subLen && strncasecmp(level->reslist->sub,_v_sub,level->reslist->subLen )==0)

idx myHCIterator(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout )
{

    return sIonBirel::eRactionContinue;
}


idx myTypeIterator(void * param, sIonBirel::LevelInfo * level, sIonBirel * birel, sIonWander * wander, idx inorout )
{

    return sIonBirel::eRactionContinue;
}




idx __on_hc(sIonTools * iap, const char * cmd, const char * args, const char * ,sVar * pForm)
{
    if(iap->gLazy)
        return 0;
    RION->debug=iap->debug;

    if(sIs(cmd,"-hcRead") ) {
        RION->init(args,sMex::fReadonly);
    }else if(sIs(cmd,"-hcCreate") ) {
        RION->init(pForm->value("ionbirelname","sion"),sMex::fMapRemoveFile);
    }else if(sIs(cmd,"-hcWrite") ) {
        RION->init(args,0);
    }else if(sIs(cmd,"-hcRoot") ) {
        RION->setRoot(args);
    } else if(sIs(cmd,"-hcOut") ) {
        RION->outFlags=0;
        sString::xscanf(args,sIonBirel::ionScanOutFlagsFormat,&RION->outFlags);
    } else if(sIs(cmd,"-hcSearch") ) {
        RION->setSearch(sIonBirel::eSearch,args);
    } else if(sIs(cmd,"-hcFields") ) {
        RION->setSearch(sIonBirel::eFields,args);
    } else if(sIs(cmd,"-hcInto") ) {
        RION->setSearch(sIonBirel::eInto,args);
    } else if(sIs(cmd,"-hcStart") ) {
        RION->iterStartRange=pForm->ivalue("start",0);
    } else if(sIs(cmd,"-hcCnt") ) {
        RION->iterCntRangeRoot=pForm->ivalue("cnt",0);
    } else if(sIs(cmd,"-hcCntD") ) {
        RION->iterCntRangeDeep=pForm->ivalue("cnt",0);
    } else if(sIs(cmd,"-hcDepth") ) {
        RION->iterMaxDepth=pForm->ivalue("depth",0);
    } else if(sIs(cmd,"-hcParse") ) {

        for( const char * p=args; p; p=sString::next00(p))
        {
            sJax vs(sFlax::fUseMMap,p);

            RION->parse(&vs);

            if( vs.errCode || vs.errIO.length() ) {
                vs.errorReport(&out);
                return 1;
            }

            p=sString::next00(p);
        }
    }
    else if(sIs(cmd,"-hcLink")) {

        sIonBirel::LinkType lnk;
        const char * sub=pForm->value("sub");
        const char * atr=pForm->value("atr");
        const char * val=pForm->value("val");
        idx sizeSub=sLen(sub);
        idx sizeAtr=sLen(atr);
        idx sizeVal=sLen(val);


        RION->link(&lnk, sub, sizeSub, atr, sizeAtr, val, sizeVal);

    } else if( sIs(cmd,"-hcQry") ) {

        RION->iterateNodes(pForm->value("sub","$root"));

    } else if(sIs(cmd,"-hcType")) {
        RION->attachTypeIterators(&out);

        const char * typenm=pForm->value("types");


        #ifndef OPTION_DIC
            RION->addDomainIterator("_field", 0, 0, 0, pForm->ivalue("startDepth",2),pForm->ivalue("endDepth",10));
            RION->iterateObjects( typenm, 0,0,0, "?type");
        #else
            sDic <  sIonHC::BirelResult > attributes;
            RION->nodePathSeparator=".";
            sIonHC::DomainCollect cache;cache.dic=&attributes;
            RION->addDomainIterator("_field", 0, RION->domainElementIterator, (void*)&cache, pForm->ivalue("startDepth",2),pForm->ivalue("endDepth",2));
            RION->iterateObjects( typenm, 0,0,0, "?type");

            for( idx i=0,idlen; i<attributes.dim(); i++ ) {
                const char * id=(const char*)attributes.id(i,&idlen);
                sIonBirel::BirelResult * reslist=attributes.ptr(i);
                ::printf("%.*s=%.*s\n",(int)idlen,id,(int)reslist->valLen,reslist->val);
            }
        #endif

    } else if(sIs(cmd,"-hcType+")) {
        const char * typenm=pForm->value("types");
        RION->getInheritedTypes( typenm, 0, &out) ;
    } else if(sIs(cmd,"-hcObj")) {
        RION->attachObjectListTraverseQuery(&out);
        RION->iterateObjects( pForm->value("types") , 0, pForm->value("ids") , 0 );
    } else if(sIs(cmd,"-hcUser")) {
        RION->attachObjectListTraverseQuery(&out);
        RION->iterateObjects( "+hc_group" , 0, pForm->value("ids") , 0 );
    } else if(sIs(cmd,"-hcGroup")) {
        RION->attachGroupListListTraverseQuery (&out);
        RION->iterateObjects( "+hc_group" , 0, pForm->value("ids"), 0, "?group" );
    } else if(sIs(cmd,"-hcRule")) {
        RION->addRule( pForm->value("act"), pForm->value("party"), pForm->value("obj"), pForm->value("infparty"), pForm->value("infobj") );

    }

    return 0;
}










