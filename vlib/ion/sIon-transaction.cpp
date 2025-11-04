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
#include <sys/stat.h>
#include <ion/sIon-core.hpp>
#include <slib/std.hpp>


idx sIonTransaction::openTransaction(idx lTransactionId, const char * baseName, const char * extension)
{
    sVec < idx > * transactionFile=0;
    if(_ion) {
        transactionFile=&(_ion->transactionFile);
        if(!baseName)baseName=_ion->baseName.ptr();
        if(!extension)extension=_ion->extension;
    } else {
        transactionFile=&(_wander->transactionFile);
    }

    if( transactionFile->ok() ) {
        Header * hdr=(Header * )transactionFile->ptr();
        if( transactionFile->dim()>=(idx)sizeof(Header)
        && hdr->id==lTransactionId
        && (hdr->flags&fTransactionLockedForReading) ) {
            hdr->flags|=fTransactionLockedForWriting;
            return lTransactionId;
        }
        else {
            hdr->flags&=~fTransactionLockedForWriting;
            if(lTransactionId==hdr->id)
                lTransactionId=0;
            transactionFile->destroy();
        }
    }


    sStr flnm(sMex::fExactSize);
    if( lTransactionId ) {
        flnm.printf(0,"%s-#transaction-%" DEC "%str",baseName,lTransactionId,extension);
        transactionFile->init(flnm,sMex::fBlockCompact);
        Header * hdr=(Header * )transactionFile->ptr();
        if( transactionFile->dim()>=(idx)sizeof(Header) ) {
            if( hdr->flags&fTransactionLockedForReading ){
                lTransactionId=0;
                transactionFile->destroy();
            } else {
                if(hdr->flags&fTransactionLockedForWriting) {
                    lTransactionId=0;
                    transactionFile->destroy();
                }
                else {
                    hdr->flags|=fTransactionLockedForWriting;
                    return lTransactionId;
                }

            }
        }else {
            transactionFile->cut(0);
            Header * hdr=(Header * )transactionFile->add(sizeof(Header)/sizeof(idx));
            if(hdr->flags&fTransactionLockedForWriting) {
                lTransactionId=0;
                transactionFile->destroy();
            }
            else {
                hdr->flags|=fTransactionLockedForWriting;
                hdr->id=lTransactionId;
                return lTransactionId;
            }

        }
    }

    struct stat b;
    do{
        lTransactionId=( ((idx)rand()&0xFFFFFFFF)<<32) |  (getpid() && 0xFFFFFFFF);
        flnm.printf(0,"%s-#transaction-%" DEC "%str",baseName,lTransactionId,extension);
    }while(stat(flnm,&b)==0);
    transactionFile->init(flnm,sMex::fBlockCompact);

    transactionFile->cut(0);
    Header * hdr=(Header * )transactionFile->add(sizeof(Header)/sizeof(idx));
    hdr->flags|=fTransactionLockedForWriting;
    hdr->id=lTransactionId;
    return lTransactionId;
}






idx sIonTransaction::commitTransaction(void)
{
    sVec < idx > * transactionFile= (_ion) ? &(_ion->transactionFile) : &(_wander->transactionFile);

    if(transactionFile->ok()) {
        Header * hdr=(Header * )transactionFile->ptr();
        if( transactionFile->dim()>=(idx)(sizeof(Header)/sizeof(idx))){
            hdr->flags&=~fTransactionLockedForWriting;

            hdr->checksum=sAlgo::hax_hashfun(transactionFile->ptr(sizeof(Header)/sizeof(idx)), (sizeof(idx)*transactionFile->dim()-sizeof(Header)), 58, 0 );
            transactionFile->destroy();

        }
        return 1;
    }
    return 0;
}

idx sIonTransaction::waitTransaction(idx lTransactionId, const char * baseName, const char * extension,idx waitInMiliSec, idx maxIters)
{

    sStr flnm(sMex::fExactSize);
    flnm.printf(0,"%s-#transaction-%" DEC "%str",baseName,lTransactionId,extension);
    struct stat b;
    for( idx iter=0; iter<maxIters; ++iter ) {
        if(stat(flnm,&b)!=0)
            return 1;
        sleepMS(waitInMiliSec);
    }
    return 0;
}

idx sIonTransaction::loadTransactions(sIO * log, idx lTransactionId, const char * baseName, const char * extension)
{
    sStr flnm(sMex::fExactSize);
    flnm.printf(0,"%s-#transaction-%" DEC "%str",baseName,lTransactionId,extension);

    return loadTransactions(log, flnm.ptr());

}

idx sIonTransaction::loadTransactions(sIO * log, const char * dirini, const char * wildcard, idx waitInMiliSec, idx maxIters)
{
    if(maxIters<=0)
        maxIters=1;

    idx cnt=0;
    for(idx iter=0; iter<maxIters; ++iter ) {
        sDir d;d.list(sFlag(sDir::bitFiles),dirini,wildcard,0);
        for(const char * f=d.ptr(); f && *f; f=sString::next00(f)){
            cnt+=loadTransactions(log, f);

        }
        if(maxIters!=sNotIdx)
            ++iter;

        sleepMS(waitInMiliSec);
    }
    return cnt;
}

idx sIonTransaction::loadTransactions(sIO * log, const char * flnm)
{
    sVec <idx> Fl(flnm,sMex::fExactSize|sMex::fNoRealloc);
    if(!Fl.ok() || Fl.dim()<(idx)(sizeof(Header)/sizeof(idx)))
        return 0;
    if(log)
        log->printf("loading %s",flnm);

    Header * hdr=(Header * )Fl.ptr();
    if(hdr->flags&sIonTransaction::fTransactionLockedForWriting)
        return 0;

    idx * fl=(idx*)sShift(Fl.ptr(0),sizeof(Header));
    idx flsize=sizeof(idx)*Fl.dim()-sizeof(Header) ;
    idx checksum=sAlgo::hax_hashfun(fl, flsize, 58, 0 );
    if(hdr->checksum!=checksum)
        return 0;
    hdr->flags|=sIonTransaction::fTransactionLockedForReading;

    sIO iout;
    if(_wander) {
        sStr Flnmout("%s.iout",flnm);
        iout.init(Flnmout.ptr());
        _wander->pTraverseBuf=&iout;
    }

    idx cnt=_wander ? _wander->loadTransactions(fl, flsize/sizeof(idx) ) : _ion->loadTransactions(fl,flsize/sizeof(idx) )  ;
    if(_wander && _wander->pTraverseBuf == &iout){
        _wander->pTraverseBuf= nullptr;
    }

    if(log) {
        if(cnt)
            log->printf(" %" DEC "\n",cnt);
        else
            log->printf(" skipped\n");
    }
    if(cnt)
        Fl.empty();
    Fl.destroy();
    if(cnt)
     sFile::remove(flnm);
    return cnt;
}

idx sIon::loadTransactions(const idx * fl, idx fldim, sIO * out,sIO * errB)
{

    sDic < idx > resultSet;
    sVec < idx > relationSet;

    idx ofs=0, cnt=0;
    for ( ; ofs<fldim; ++cnt) {

        idx * pResult=resultSet.set(&ofs,sizeof(ofs));


        if(fl[ofs]==sIonTransaction::eTransactionAddRecord){++ofs;
            idx recordTypeIndex=fl[ofs];++ofs;
            idx recordSize=fl[ofs];++ofs;
            const void * recordBody=(const void *)&(fl[ofs]);
            idx b= (recordSize/8); if(recordSize%8)b+=1;
            *pResult=addRecord(recordTypeIndex,recordSize,recordBody);
            ofs+=b;
        } else if(fl[ofs]==sIonTransaction::eTransactionAddRelation) {++ofs;


            idx relationTypeIndex=fl[ofs];++ofs;
            idx insertIndex=fl[ofs];++ofs;
            idx cntRelations=fl[ofs];++ofs;
            relationSet.cut(0);
            for(idx ic=0; ic<cntRelations;++ic) {
                idx r=fl[ofs];++ofs;
                if(r<0) {
                    r=-r;
                    r-=4;
                    r=resultSet[&r];
                }
                *relationSet.add()=r;
            }

            addRelationVarg(relationTypeIndex, insertIndex, relationSet.ptr(0),0);
        } else if(fl[ofs]==sIonTransaction::eTransactionDeleteRelation){++ofs;
            idx recordTypeIndex=fl[ofs];++ofs;
            idx recordSize=fl[ofs];++ofs;
            const void * recordBody=(const void *)&(fl[ofs]);
            idx b= (recordSize/8); if(recordSize%8)b+=1;
            *pResult=addRecord(recordTypeIndex,recordSize,recordBody);
            ofs+=b;
        }
        else
            ++ofs;
    }

    return cnt;
}


idx sIonWander::loadTransactions(const idx * fl, idx fldim, sIO * out, sIO * errb)
{

    if(out)
        pTraverseBuf=out;
    idx ofs=0, cnt=0;
    for ( ; ofs<fldim; ++cnt) {
        if(errb)
            errb->cut(0);

        if(fl[ofs]==sIonTransaction::eTransactionTraverseCompile){++ofs;
            idx len=fl[ofs];++ofs;
            idx acceptInvalid=fl[ofs];++ofs;
            const char * rules=(const char *)&(fl[ofs]);
            idx b= (len/8); if(len%8)b+=1;
            traverseCompile(rules,len, errb, acceptInvalid ? true : false ) ;
            ofs+=b;

        } else if(fl[ofs]==sIonTransaction::eTransactionTraverse){++ofs;
            idx start=fl[ofs];++ofs;
            traverse(start ) ;
        }

    }
    return cnt;
}
