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
#ifndef sBio_vioseq_hpp
#define sBio_vioseq_hpp

#include <slib/core/str.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqtree.hpp>

namespace slib
{
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Working format Sequence Collection Class
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    class sVioseq : public sBioseq 
    {

        sFil baseFile;


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Construction/Destruction
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/

    public: // FILE based BioSeq
        enum eParseFlags {
            eParseLazy=            0x00000001,
            eParseNoId=            0x00000002,
            eParseQuaBit=        0x00000004,
            eParsedQualities=    0x00000010,
            eFossilizeReverse=  0x00000100,
            eContainsPrimer=    0x00001000,
            eLowComplexity=        0x00002000
        };

        struct Hdr {
            idx cntSeq; // count of sequences
            idx ofsSeq; // offset in the file where those sequences are
            idx ofsIdx; // offset to indexed elements in the file
            idx flags;
            idx ofsReserv[12]; // reserved for future usage
        };
        struct Rec{
            idx ofsSeq;
            idx lenSeq;
        };

        Hdr hdrR, *hdr;
        FILE * fp;
        sStr flnm;

        idx complexityWindow;
        real complexityEntropy;

        // here we cache the read content for current record
        idx lNumRec,lNumSeq,lNumQua,lNumId, maxId, maxFos;
        Rec lRec;
        sStr lSeq;
        sStr lQua;
        sStr lId;

        template < typename To> To * rd( To * ptr, idx ofs, idx siz=sizeof(To) ) {
            fseek(fp, (long)ofs ,SEEK_SET);
            fread((void*)ptr,siz,1,fp);
            return ptr;
        }

        sVioseq (const char * fl=0, bool doMap = false ) : baseFile(0){
            lNumRec=lNumSeq=lNumQua=lNumId=sNotIdx;
            fp=0;
            complexityWindow=0;
            complexityEntropy=1;
            hdr=&hdrR;
            sSet(hdr);
            init(fl, doMap);
        }
        ~sVioseq ( ){destroy();}

        sVioseq * init ( const char * fl=0, bool doMap=false) {
            if(doMap){
                baseFile.init(fl, sMex::fReadonly);
                hdr=(Hdr*)baseFile.ptr(0);
            }
            else {
                hdr=&hdrR;
                lNumRec=lNumSeq=lNumQua=lNumId=sNotIdx;
                fp=0;
                sSet(hdr);
                if(!fl)fl=flnm.ptr(0);
                else {fl=flnm.printf("%s",fl);}

                if(flnm && strcmp(flnm,"stdin")==0)fp=stdin;
                if(!fp && flnm)fp=fopen(flnm,"rb");
                if(!fp)return 0;

                rd(hdr,0);
                //maxFos=1024;lFos.resize(maxFos);
            }
            maxId=1024; lId.resize(maxId); // we support here IDs up to 4 KB
            return this;
        }
        void destroy (void) {
            if( fp && fp!=stdin ) fclose(fp);
        }

        
    public:
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ VioSeq filing operations
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
        idx parseFastaFile(const char * flnm, idx flags=eParseLazy, const char * outfile=0, const char * primers00=0);
        idx parseFastaFile2(const char * flnm, idx flags=eParseLazy, const char * outfile=0, const char * primers00=0);
        idx parseFasta(const char * inp, idx len, idx flags=0, const char * primers00=0);//sDic < idx > * ofsdic=0, idx flags=0);
        idx parseFasta2(const char * inp, idx len, idx flags=0, const char * primers00=0);//sDic < idx > * ofsdic=0, idx flags=0);
        idx parseFastQ(const char * inp, idx len, idx flags=0, const char * primers00=0);
        idx parseFastQ2(const char * inp, idx len, idx flags=0, const char * primers00=0);
        void printFasta(idx start=0, idx cnt=0);
        idx parseQualities(const char * qual, idx qualalen);
        idx primerCut(Rec & rec, idx ofsSeq, char * cpy, const char * primers00);
        sFil * base(void){ return &baseFile;}
    public:
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ VioSeq access functions
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/

        virtual idx dim(void)
        {
            return hdr ? hdr->cntSeq : 0 ;
            //return ((Hdr * )ptr(0))->cntSeq;
        }

        virtual Rec * rec( idx inum ) {
            if(fp) {
                if ( inum == lNumRec) return &lRec; else lNumRec=inum;
                return rd( &lRec, hdr->ofsIdx + inum*sizeof(Rec) );
            }else {
                return (Rec*)baseFile.ptr( hdr->ofsIdx + inum*sizeof(Rec) );
            }
        }
        virtual const char * seq(idx inum, idx ipos=0, idx ilen=0, idx iread=1) {
            Rec * r=rec(inum);
            ipos/=4;

            if(fp) {
                if ( inum == lNumSeq) return lSeq.ptr(); else lNumSeq=inum;
                if (!ilen)ilen=r->lenSeq;
                ilen=(ilen-1)/4+1;
                lSeq.resize(ilen);//lSeq.resize(r->lenSeq);
                return rd( lSeq.ptr(), r->ofsSeq + hdr->ofsSeq+ipos , ilen );//r->lenSeq);
            } else {
                return baseFile.ptr(  r->ofsSeq + hdr->ofsSeq+ipos );
            }
        }
        virtual const char * qua(idx inum, idx iread=0, idx ipos=0, idx ilen=0) {
            if(!(hdr->flags&eParsedQualities))return 0;
            Rec * r=rec(inum);
            idx lseqbuf=(r->lenSeq-1)/4+1;

            if(fp) {
                if ( inum == lNumQua) return lQua.ptr();else lNumQua=inum;
                idx qualen=(hdr->flags&sVioseq::eParseQuaBit) ? (r->lenSeq-1)/8+1 : r->lenSeq;
                lQua.resize(qualen);
                return rd( lQua.ptr(), r->ofsSeq + hdr->ofsSeq+lseqbuf , qualen );
            }
            else {
                return baseFile.ptr( r->ofsSeq + hdr->ofsSeq+lseqbuf);
            }
        }

        virtual idx len(idx inum, idx iread=1) {
            return rec(inum)->lenSeq;
        }
        virtual const char * id(idx inum, idx iread=1) {
            if(hdr->flags&eParseNoId )
                return lId.printf(0,">%"DEC,inum);
            if(fp && inum == lNumId)
                return lId.ptr();else lNumId=inum;

            Rec * r=rec(inum);
            idx qualen=(hdr->flags&eParsedQualities) ? ((hdr->flags&sVioseq::eParseQuaBit) ? (r->lenSeq-1)/8+1 : r->lenSeq) : 0 ;
            idx lseqbuf=(r->lenSeq-1)/4+1;

            if(fp) {
                return rd( lId.ptr(), r->ofsSeq + hdr->ofsSeq+lseqbuf+qualen , maxId );
            } else {
                return baseFile.ptr(  r->ofsSeq + hdr->ofsSeq+lseqbuf+qualen );
            }
        }
    };



} // namespace 

#endif // sBio_vioseq_hpp

