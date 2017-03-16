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
#include <slib/core.hpp>
#include <slib/std/file.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ctype.h>

using namespace slib;


enum eMode{
    eIdLine,
    eSeq, 
    eId2,
    eQual,
    eStop
};

idx sVioseq::parseFastaFile(const char * flnm, idx flags, const char * outfile, const char * primers00)
{
    // see if the fab file already exists 
    //sFilePath fabnm(flnm,"%%pathx%s",extFasta);

    if(outfile){
        baseFile.init(outfile,sMex::fReadonly);
        if((flags&eParseLazy) && baseFile.length())return dim();// we are ok
        baseFile.destroy();
    }

    baseFile.mex()->flags&=~(sMex::fReadonly);
    if(outfile)baseFile.init(outfile);
    baseFile.flags|=sMex::fBlockDoubling; // allocate this buffer with large chunks since these sequences can be large

    sDic < idx > ofs; 
        
    // try opening the original sequence file and parse it 
    baseFile.cut(0);        
    sFil inf(0); // mapATGC the input file
    if(strcmp(flnm,"stdin")==0)inf.readIO(stdin);
    else inf.init(flnm,sMex::fReadonly);

    if(!inf.length())return 0;

    idx res=0;

    if( strstr(flnm,".fastq") || strstr(flnm,".fq")  )
            res= parseFastQ(inf.ptr(), inf.length(),flags, primers00);
    else {//if( strstr(flnm,".fna") || strstr(flnm,".fa")  )
        //res= parseFasta(inf.ptr(), inf.length(), flags);

        sFilePath quanm(flnm,"%%pathx%s",".qual");
        idx lenFastQ=0;
        {
            sFil qua(quanm);
            lenFastQ=qua.length();
            if(lenFastQ) {
                res= parseFasta(inf.ptr(), inf.length(), flags|eParsedQualities , primers00);
                if(!primers00 && complexityWindow==0 ) // if primers are defined ,or complexity level is defined  ... some sequences will not be taken in
                    res=parseQualities(qua.ptr(), qua.length());
            }
            else     res= parseFasta(inf.ptr(), inf.length(), flags, primers00);
        }
        if(lenFastQ==0)
            sFile::remove(quanm);

    }

    return res;

}
idx sVioseq::parseFasta(const char * fasta, idx fastalen, idx flags,const char * primers00)//sDic < idx > * dicofs , idx flags)
{
    idx ret=0;

    //baseFile.flags|=sMex::fBlockDoubling; // allocate this buffer with large chunks since these sequences can be large
    idx ofsSeq=sizeof(sVioseq::Hdr);
    baseFile.add(0,ofsSeq); // create a fab file header

    sVec < Rec > vofs (sMex::fBlockDoubling);//, nm.makeName(inp,"%%pathx.idx" ) );
    idx idofs=0, seqofs=-1, idnext=-1;
    
    const char * c0=fasta, *c=c0, *e=c+fastalen, *p="\n";
    idx mode=eSeq, pmode=-1;//, num=-1;

    Rec rec;

    while( c0<e  && (*c0=='\n' || *c0=='\r') )++c0; // skip first empty lines
    c=c0;

    //idx bm=dynamicMatch( Mat.ptr(), seq, seq, start, lenfos, start,lenfos , maxDiag, history, costs );
    //bm=dynamicMatch( Mat.ptr(), seq, seq, start, lenfos, start,lenfos , maxDiag, history, costs , lenseq );

    for ( idx iN=0 ; c<e; p=c, ++c  ) 
    { 
        if(*p=='\n' || *p=='\r' || c==e-1 ) { // first choice is for windows and unix, the second is for mac, the third is end of file
            if( mode==eIdLine ) {mode = eSeq; seqofs=c-c0;}
            else if( mode==eSeq && *c=='>') {mode=eIdLine; idnext=c-c0;}
            else if( *c=='#' || c==e-1 ) {idnext=c-c0; mode=eStop;}
            
            if( pmode==eSeq && mode!=eSeq ){ // generate record 
                const char * id=c0+idofs;
                const char * seq=c0+seqofs;
                const char * nxt=c0+idnext;

                // count the length of the ID line 
                // and add it to the destination 
                idx il, iddiclen=0;
                for ( il=0; id+il<nxt && id[il]!='\n' &&  id[il]!='\r' ; ++il){
                    if(!iddiclen && il>2 && (id[il]==' ' || id[il]=='\t'))iddiclen=il; // also mark where the first word of the dictionary ends
                } if(!iddiclen)iddiclen=il; // if it is a single word ... it is what is used for dictionarizing
                

                // now squeeze the sequence
                rec.ofsSeq=baseFile.length()-ofsSeq;
                while ( mapATGC[(idx)(*seq)]==0xFF && seq<nxt )++seq; // new-code
                char * cpy=baseFile.add(0, ( nxt-seq )/4+1 ); // for two bit representation
                rec.lenSeq=sBioseq::compressATGC(cpy,seq,nxt-seq); // TODO : cut the bufflen extension length to req.lenseq
                baseFile.cut(ofsSeq+rec.ofsSeq+(rec.lenSeq-1)/4+1); // new-code


                // complexity filtering
                bool isok=(complexityWindow!=0) ? complexityFilter( cpy, rec.lenSeq ,complexityWindow, complexityEntropy ) : true ;

                idx primershift=0;
                if( isok && primers00 ) {
                    primershift=primerCut(rec,ofsSeq,cpy,primers00); // cut the primer from the sequence
                    if(primershift==sNotIdx )
                        isok=false; // sequence is to be discarded completely
                }


                idx lqua=0;
                if( isok && rec.lenSeq  && flags&eParsedQualities) { // reserve the space for qualities
                    if( (flags&eParseQuaBit) ){
                        lqua=(rec.lenSeq-1)/8+1;
                        char * pqu=baseFile.add(0,lqua);memset(pqu,0,lqua);
                    }
                    else {
                        lqua=rec.lenSeq;
                        baseFile.add(0,rec.lenSeq);
                    }
                }

                if(isok==false) {
                    baseFile.cut(rec.ofsSeq+ofsSeq);
                } else {
                    baseFile.cut(ofsSeq+rec.ofsSeq+(rec.lenSeq-1)/4+1 + lqua);
                    if( (flags&eParseNoId) ==0 ) 
                        baseFile.add(id,il);baseFile.add0();
                    *vofs.add()=rec;
                }

                idofs=idnext;

                ++iN;
            }

            pmode=mode;
        }

    }

    // make sure alignment is right 
    baseFile.flags|=sMex::fAlignInteger;
    //baseFile.add(0,sizeof(idx));
    baseFile.add(sMex::_zero,sizeof(idx));


    sVioseq::Hdr * hd=(sVioseq::Hdr *)baseFile.ptr();
    hd->ofsIdx=baseFile.length();
    hd->cntSeq=vofs.dim();
    hd->ofsSeq=ofsSeq;
    //hd->ofsFos=fossilcount>1 ? ofsSeq : 0 ;
    //hd->ofsQua=0; // not yet qualities there
    hd->flags=flags;
    hdrR=*hd;
    baseFile.add((const char * )vofs.ptr(), vofs.dim()*sizeof(Rec) );
    //hdr=*hd;
    return ret;
}


idx sVioseq::parseFastQ(const char * fasta, idx fastalen, idx flags, const char * primers00)
{
    idx ret=0;

    idx ofsSeq=sizeof(sVioseq::Hdr);
    baseFile.add(0,ofsSeq); // create a fab file header

    sVec < Rec > vofs (sMex::fBlockDoubling);
    idx idofs=0, seqofs=-1, idnext=-1 , idsecofs=-1, qualofs=-1;

    const char * c0=fasta, *c=c0, *e=c+fastalen, *p="\n";
    idx mode=eQual, pmode=-1;

    Rec rec;
    
    while(c0<e  && (*c0=='\n' || *c0=='\r') )++c0; // skip first empty lines
    c=c0;

    for ( idx iN=0 ; c<e; p=c, ++c  )
    {
        if(*p=='\n' || *p=='\r' || c==e-1 ) { // first choice is for windows and unix, the second is for mac, the third is end of file
            if( mode==eIdLine ) {mode = eSeq; seqofs=c-c0;}
            else if( mode==eSeq && *c=='+') {mode=eId2; idsecofs=c-c0;}
            else if( mode==eId2 ) {mode=eQual; qualofs=c-c0;}
            else if( mode==eQual ) {mode=eIdLine; idnext=c-c0;}
            else if( c==e-1 ) {idnext=c-c0; mode=eStop;}

            if( pmode==eQual && mode!=eQual ){ // generate record
                const char * id=c0+idofs;
                const char * seq=c0+seqofs;
                const char * qua=c0+qualofs;
                const char * idsec=c0+idsecofs;
                const char * nxt=c0+idnext;

                // count the length of the ID line
                // and add it to the destination
                idx il, iddiclen=0;
                for ( il=0; id+il<nxt && id[il]!='\n' &&  id[il]!='\r' ; ++il){
                    if(!iddiclen && il>2 && (id[il]==' ' || id[il]=='\t'))iddiclen=il; // also mark where the first word of the dictionary ends
                } if(!iddiclen)iddiclen=il; // if it is a single word ... it is what is used for dictionarizing


                // now squeeze the sequence
                rec.ofsSeq=baseFile.length()-ofsSeq;
                idx qshift=0;
                while ( mapATGC[(idx)(*seq)]==0xFF && seq<idsec ){++seq;++qshift;}
                char * cpy=baseFile.add(0, ( idsec-seq )/4+1 ); // for two bit representation
                rec.lenSeq=sBioseq::compressATGC(cpy,seq,idsec-seq);
                baseFile.cut(ofsSeq+rec.ofsSeq+(rec.lenSeq-1)/4+1);

                // complexity filtering
                bool isok=(complexityWindow!=0) ? complexityFilter( cpy, rec.lenSeq ,complexityWindow, complexityEntropy ) : true ;

                idx primershift=0;
                if( isok && primers00 ) {
                    primershift=primerCut(rec,ofsSeq,cpy,primers00); // cut the primer from the sequence
                    if(primershift==sNotIdx )
                        isok=false; // sequence is to be discarded completely
                    else if(primershift!=0)
                        isok=true;
                }


                // now squeeze the qualities
                if( isok && rec.lenSeq ) {
                    qua+=qshift+primershift;
                    if( (flags&eParseQuaBit) ){
                        idx lqua=(rec.lenSeq-1)/8+1;
                        char * pqu=baseFile.add(0,lqua);memset(pqu,0,lqua);
                        for ( idx iq=0; iq<rec.lenSeq; ++iq)
                            if(qua[iq]>53)pqu[iq/8]|=((idx)1)<<(iq%8);
                    }
                    else {///char * pqu=baseFile.add(qua,rec.lenSeq);
                        char * pqu=baseFile.add(0,rec.lenSeq);
                        for ( idx iq=0; iq<rec.lenSeq; ++iq)
                            pqu[iq]=qua[iq]-33;
                    }
                }
                
                if(isok==false ) {
                    baseFile.cut(rec.ofsSeq+ofsSeq);
                } else {
                    if( (flags&eParseNoId) ==0 ) {
                        baseFile.add(id,il);baseFile.add0();
                    }
                    *vofs.add()=rec;
                }
                idofs=idnext;
                ++iN;
            }

            pmode=mode;
        }

    }

    // make sure alignment is right
    baseFile.flags|=sMex::fAlignInteger;
    baseFile.add(sMex::_zero,sizeof(idx));


    sVioseq::Hdr * hd=(sVioseq::Hdr *)baseFile.ptr();
    hd->ofsIdx=baseFile.length();
    hd->cntSeq=vofs.dim();
    hd->ofsSeq=ofsSeq;

    hd->flags=flags|eParsedQualities;
    hdrR=*hd;
    baseFile.add((const char * )vofs.ptr(), vofs.dim()*sizeof(Rec) );
    
    //hdr=*hd;
    return ret;
}

// cuts the primers and returns the position of the shift of leftover sequence from the beginning or -1 if sequence has to be discarded completely
idx sVioseq::primerCut(Rec & rec, idx ofsSeq,  char * cpy, const char * primers00)
{
    //idx maxMissMatches=1;

    //bool isok=true;
    for( const char * pr=primers00 ; pr; pr=sString::next00(pr)){
        bool reverse= (pr[0]=='R') ? true : false ;
        bool keepIfEnd= (pr[1]=='K') ? true : false ;
        bool keepIfMid= (pr[2]=='K') ? true : false ;
        idx maxMissMatches= (pr[3]-'0')*10+(pr[4]-'0');
        pr+=6;
        idx prlen=sLen(pr), ispos, ippos=0, ik=0;
        if(!reverse) {
            for ( ispos=rec.lenSeq-1 ; ispos>=0 ; --ispos) {
                idx missMatches=0;

                for ( ik=0, ippos=prlen-1; ispos-ik>=0 && ippos>=0; --ippos, ++ik ) {
                    char let=sBioseq::mapRevATGC[((cpy[ ((ispos-ik)/4) ]>> (((ispos-ik)%4)*2) )&0x3)];
                    if(pr[ippos]=='.')continue;
                    if(let!=toupper(pr[ippos]) )
                        ++missMatches;
                    if(missMatches>maxMissMatches)
                        break;
                }

                if( ippos<=0 || ispos-ik<0)
                    break;
            }
            if(ispos<0) continue; // no matches
            if( ippos<0 || islower( pr[ippos] ) ){ // enough characters matched from primer
                //if(*pr=='f')return sNotIdx;
                bool doKeep=false;
                if( ispos-ik<0 && keepIfEnd ) doKeep=true;// primer matched to the end : cut the sequence or keep it ?
                if( ispos-ik>=0 && keepIfMid ) doKeep=true;// // primer matched in the middle : remove the sequence

                if(doKeep) { // if(ispos-ik<0){ // primer matched to the end : cut the sequence
                    idx movpos=prlen-1-ippos +(ispos-ik+1);
                    if(movpos>=rec.lenSeq)movpos=rec.lenSeq-1;
                    sStr tt;uncompressATGC(&tt,cpy, 0, rec.lenSeq);
                    rec.lenSeq=compressATGC(cpy,tt.ptr(movpos), rec.lenSeq-movpos );
                    baseFile.cut(ofsSeq+rec.ofsSeq+(rec.lenSeq-1)/4+1);
                    return movpos;
                }
                else // primer matched in the middle : remove the sequence
                    return sNotIdx;
            }

        }else {
            for ( ispos=0 ; ispos<rec.lenSeq ; ++ispos) {
                idx missMatches=0;
                for ( ik=0, ippos=0; ippos<prlen && ispos+ik<rec.lenSeq; ++ippos , ++ik) {
                    char let=sBioseq::mapRevATGC[((cpy[ ((ispos+ik)/4) ]>> (((ispos+ik)%4)*2) )&0x3)];
                    if(pr[ippos]=='.')continue;
                    if(let!=toupper(pr[ippos]) )
                        ++missMatches;
                    if(missMatches>maxMissMatches)
                        break;
                }
                if( ippos>=prlen || ispos+ik==rec.lenSeq)
                    break;
            }
            if(ispos>=rec.lenSeq) continue; // no matches
            if( ippos==prlen || islower( pr[ippos] ) ){ // enough characters matched from primer
                //if(*pr=='r')    return sNotIdx;
                bool doKeep=false;
                if( ispos+ik>=rec.lenSeq && keepIfEnd ) doKeep=true;// primer matched to the end : cut the sequence or keep it ?
                if( ispos+ik<rec.lenSeq && keepIfMid ) doKeep=true;// // primer matched in the middle : remove the sequence

                if(doKeep){// if(ispos+ik>=rec.lenSeq){ // primer matched to the end : cut the sequence
                    rec.lenSeq=ispos;//rec.lenSeq-=ippos;
                    if(rec.lenSeq<=0)rec.lenSeq=1;
                    baseFile.cut(ofsSeq+rec.ofsSeq+(rec.lenSeq-1)/4+1);
                    return 0;
                }
                else // primer matched in the middle : remove the sequence
                    return sNotIdx;
            }
        }

    }
    return 0;
}

idx sVioseq::parseQualities(const char * qual, idx qualalen) //, sDic < idx > * ofs
{
    idx ret=0;
        
    //idx ofsQua=baseFile.length();

    idx idofs=0, seqofs=-1, idnext=-1;
    const char * c0=qual, *c=c0, *e=c+qualalen, *p="\n";
    idx mode=eSeq, pmode=-1;//, num=-1;
    sVioseq::Hdr * hd=(sVioseq::Hdr *)baseFile.ptr();

    for ( idx iN=0 ; c<e; p=c, ++c  ) 
    { 
        if(*p=='\n' || *p=='\r' || c==e-1 ) { // first choice is for windows and unix, the second is for mac, the third is end of file
            if( mode==eIdLine ) {mode = eSeq; seqofs=c-c0;}
            else if( mode==eSeq && *c=='>') {mode=eIdLine; idnext=c-c0;}
            else if( *c=='#' || c==e-1 ) {idnext=c-c0; mode=eStop;}
            
            if( pmode==eSeq && mode!=eSeq ){ // generate record 
                const char * id=qual+idofs;
                const char * seq=qual+seqofs;
                const char * nxt=qual+idnext;

                // count the length of the ID line 
                // and add it to the destination 
                idx il, iddiclen=0;
                for ( il=0; id+il<nxt && id[il]!='\n' &&  id[il]!='\r' ; ++il){
                    if(!iddiclen && (id[il]==' ' || id[il]=='\t'))iddiclen=il; // also mark where the first word of the dictionary ends
                }
                if(!iddiclen)iddiclen=il; // if it is a single word ... it is what is used for dictionarizing 

                idx iofs=iN;
                Rec * rr=rec( iofs ); 

                idx lseqbuf=(rr->lenSeq-1)/4+1;
                char * pqu=baseFile.ptr( rr->ofsSeq + hd->ofsSeq+lseqbuf); // the space has been reserved here before

                // now squeeze the sequence
                //rr->ofsQua=baseFile.length()-ofsQua;
                //idx lqua=( (flags&eParseQuaBit) ) ? (rr->lenSeq-1)/8+1 : rr->lenSeq;
                //char * pqu=baseFile.add(0,lqua);
                //if( (flags&eParseQuaBit) ) memset(pqu,0,lqua);
                const char * pp;
                idx iq;
                //rr=rec( iofs ); // we redo it after reallocation few lines above
                for(pp=seq, iq=0 ; pp && pp<nxt && iq<rr->lenSeq; pp=sString::skipWords(pp,nxt-pp,0),  ++iq) {
                    idx qu=0;
                    for(;*pp>='0' && pp<nxt && *pp<='9';++pp)
                        qu=qu*10+((*pp)-'0');
                    if( (hd->flags&eParseQuaBit) ) {if(qu>20)pqu[iq/8]|=((idx)1)<<(iq%8);}
                    else pqu[iq]=(char)qu;
                }
                
                idofs=idnext;
                //num=-1;
                ++iN;
            }

            pmode=mode;
        }

    }

    // make sure alignment is right 
    baseFile.flags|=sMex::fAlignInteger;
    baseFile.add(0,sizeof(idx));


    return ret;
}








void sVioseq::printFasta(idx start, idx cnt)
{
    if(!cnt)cnt=dim();
    for(idx i=start; i<cnt ; ++i ) { 
        Rec &rec=*this->rec(i);
        const char * seq=this->seq(i);
        const char * id=this->id(i);
        
        sStr buf;
        sBioseq::uncompressATGC (&buf, seq, 0, rec.lenSeq );
        printf("%s\n",id);
        printf("%s\n\n",buf.ptr());

    }
}


