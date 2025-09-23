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
#ifndef sBio_seq_hpp
#define sBio_seq_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/rc.hpp>

#define sBIO_ATGC_SEQ_2BIT

namespace slib {

    class sBioseq
    {

        public:
            sBioseq()
            {
                print_callback = 0;
                print_callbackParam = 0;
            }
            virtual ~sBioseq()
            {
            }
            virtual idx dim(void)
            {
                return 0;
            }

            typedef idx (*callbackType)(void *param, sStr * buf, idx initid, idx initseq, idx initqua, idx seqlen);
            callbackType print_callback;
            void * print_callbackParam;

            typedef enum EBioMode_enum
            {
                eBioModeShort       = 0,
                eBioModeLong
            } EBioMode;

            static bool isBioModeLong (idx mode) { return mode==eBioModeLong;}
            static bool isBioModeShort (idx mode) { return mode==eBioModeShort;}
            enum eReadTypes
            {
                eReadBiological,
                eReadAll,
                eReadTechnical
            };

            typedef enum ESequenceOrientation_enum
            {
                eSeqForward             = 0,
                eSeqForwardComplement   = 2,
                eSeqReverse             = 1,
                eSeqReverseComplement   = 3
            } ESeqDirection;


            virtual idx len(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual idx rpt(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual idx sim(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual const char * seq(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual const char * id(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual const char * qua(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual idx startlen(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual idx endlen(idx num, idx iread = 0)
            {
                return 0;
            }
            virtual idx long2short(idx inum, idx iread = 0)
            {
                return 0;
            }
            virtual idx short2long(idx inum, idx iread = 0)
            {
                return 0;
            }
            virtual void setmode(EBioMode)
            {
                return;
            }
            virtual EBioMode getmode()
            {
                return eBioModeShort;
            }
            virtual void setDimVioseqlist(idx tot)
            {
                return;
            }
            virtual idx getDimVioseqlist(idx tot)
            {
                return 0;
            }
            virtual void destroy(bool justMemoryFree = false)
            {
            }
            virtual bool getQuaBit(idx inum)
            {
                return false;
            }
            virtual idx getlongCount()
            {
                return 0;
            }
            virtual idx getshortCount()
            {
                return 0;
            }

            virtual bool isFastA (){
                return false;
            }

            virtual idx countFastQs(){
                return 0;
            }

            inline static char Qua(const char * qua, idx i, bool quabit)
            {
                if( quabit )
                    return (qua[i / 8] & (((idx) 1) << (i % 8))) ? 0 : 40;
                return (qua[i]);
            }
            idx longcount()
            {
                idx longdim = 0;
                for(idx i = 0; i < dim(); i++) {
                    longdim += rpt(i);
                }
                return longdim;
            }

            idx printFastX(sStr * outFile, bool isFastq, idx start = 0, idx end = 0, idx shift = 0, bool keepOriginalId = false, bool appendLength = false, idx iread = 0, sVec<idx> * randomIdQueries = 0, bool restoreNs = true, idx filterNs = 0);
            idx printFastXRow(sStr * outFile, bool isFastq, idx row, idx start = 0, udx length = 0, idx shift = 0, bool keepOriginalId = false, bool numIdOnly = false, const char *appendID = 0, idx iread = 0, ESeqDirection isRevCmp = eSeqForward, bool restoreNs = true, idx filterNs = 0, bool appendRptCount = true, bool SAMCompatible = false, bool printRepeats = false);
            idx printSAMRow(sStr & out, idx row, idx iread, idx flag, bool keepOriginalId, bool printQualities);


            idx seqstr_2Bit(sStr * buf, idx num, idx start = 0, idx lenseq = 0, idx isrev = 0, idx iscomp = 0);
        public:
            idx printFastXData(sStr * outFile, idx seqlen, const char *seqid, const char *seqs, const char *qua, idx subrpt, idx iread = 0);
            idx printSequence(sStr *outFile, idx row, idx start = 0, idx length = 0, ESeqDirection isRevCmp = eSeqForward, bool restoreNs = true);
            sRC printQualities(sStr & outBuf, idx row, idx iread, bool isRev);

            idx countNs(idx num, idx start = 0, idx seqlen = 0){
                const char * seqqua1 = qua(num);
                if (!seqqua1){
                    return 0;
                }
                bool quaBit = getQuaBit(num);
                if (!seqlen){
                    seqlen = len(num);
                }
                idx NCount = 0;
                for(idx i = start; i < start + seqlen; ++i) {
                    if( Qua(seqqua1, i, quaBit) == 0 ) {
                        NCount++;
                    }
                }
                return NCount;
            }
            static idx atgcModel;
            sStr idstr, seqstr, quastr, outstr;

            enum eATGCModel
            {
                eATGC = 0,
                eACGT
            };

            static void initModule(idx initmodel);

            static idx compressATGC_2Bit(sStr * buf, const char * seq, idx len, idx isextensiblebuf = true, sStr * Nbuf = 0);
            static idx compressATGC_2Bit(char * b, const char * seq, idx len, char *Nbuf = 0)
            {
                return compressATGC_2Bit((sStr *) b, seq, len, false, (sStr*) Nbuf);
            }
            static idx uncompressATGC_2Bit(sStr * buf, const char * seq, idx start, idx len, idx isextensiblebuf = true, idx isbrk = 0, idx isrev = 0, idx iscomp = 0);
            static idx uncompressATGC_2Bit(char * b, const char * seq, idx start, idx len, idx isbrk = 0, idx isrev = 0, idx iscomp = 0)
            {
                return uncompressATGC_2Bit((sStr *) b, seq, start, len, false, isbrk, isrev, iscomp);
            }

            static idx compressATGC_1Byte(sStr * buf, const char * seq, idx len, idx isextensiblebuf = true);
            static idx compressATGC_1Byte(char * b, const char * seq, idx len)
            {
                return compressATGC_1Byte((sStr *) b, seq, len, false);
            }
            static idx uncompressATGC_1Byte(sStr * buf, const char * seq, idx start, idx len, idx isextensiblebuf = true, idx isbrk = 0);
            static idx uncompressATGC_1Byte(char * b, const char * seq, idx start, idx len, idx isbrk = 0)
            {
                return uncompressATGC_1Byte((sStr *) b, seq, start, len, false, isbrk);
            }

            static idx compressIUPAC(sStr * buf, const char * seq, idx len, idx isextensiblebuf = true);
            static idx uncompressIUPAC(sStr * buf, const char * seq, idx start, idx len, idx isextensiblebuf = true, idx isbrk = 0, idx isrev = 0);

            static const char * pHash(idx hash, idx len);
            static bool matchIUPAC(char atgc, char iupac);

#define compressATGC compressATGC_2Bit
#define uncompressATGC uncompressATGC_2Bit

            static unsigned char mapATGC[256];
            static unsigned char mapRevATGC[5];
            static unsigned char mapIUPAC[256];
            static unsigned char mapRevIUPAC[17];
            static idx mapComplementATGC[5];

            typedef enum EBioAAcode_enum {
                eBioAAsingle = 0,
                eBioAAtriple,
                eBioAAfull
            } EBioAAcode;


            struct ProtAA
            {
                    const char * nm, *let3, *let, **codon00;
                    const char * print(EBioAAcode code = eBioAAsingle) {
                        switch(code){
                            case eBioAAsingle:
                                return let;
                                break;
                            case eBioAAtriple:
                                return let3;
                            case eBioAAfull:
                                return nm;
                            default:
                                return let;
                        }
                    }

            };
            static ProtAA * AAFindByLet3(const char * three );
            static ProtAA * AAFindByLet(char let);

        private:
            static ProtAA listAA[];
            static sVec<idx> mapCodeAA;
            static unsigned char mapAAlet2ilist[256];

        public:
            static ProtAA * getListAA(){
                return listAA;
            }
            static idx mapCodon(char compressedTriplet, idx geneticCode = 0)
            {
                return (idx)compressedTriplet < mapCodeAA.dim() ? mapCodeAA[(geneticCode * 64) + (idx)compressedTriplet] : 0;
            }
            static ProtAA * mappedCodon2AA(idx mappedCodon)
            {
                return listAA + mappedCodon;
            }
            static ProtAA * codon2AA(char compressedTriplet, idx geneticCode = 0)
            {
                return (idx)compressedTriplet < mapCodeAA.dim() ? listAA + mapCodon( compressedTriplet,geneticCode) : 0;
            }
            static ProtAA * codon2AA(const char * triplet, idx geneticCode = 0)
            {
                char byt = 0;
                compressATGC(&byt, triplet, 3);
                return codon2AA(byt, geneticCode);
            }
            static void usemask(const char * letters)
            {
                for(const char* seq = letters; *seq; ++seq)
                    mapATGC[(idx) (*seq)] = 0xFF;
            }

            static idx proteinDecode(char * prot, const char * dna, idx len, bool nozero = false);
            static idx proteinVariantsFromSNP(char * pro, char * dna, idx dlen, idx pos, const char * ale, char * * variants = 0, idx * pcntVariants = 0);


            class ProteinNmerIter {
                public:
                    enum EState {
                        eOK,
                        eEOF,
                        eError
                    };
                private:
                    const char * _prot;
                    const char * _nmer;
                    idx _pos;
                    idx _line;
                    idx _prot_len;
                    idx _nmer_len;
                    sStr _buf;
                    sVec<idx> _prot_pos;
                    EState _state;

                public:
                    void init(idx nmer_len, const char * prot, idx prot_len = 0);
                    ProteinNmerIter(idx nmer_len = 0, const char * prot = 0, idx prot_len = 0) { init(nmer_len, prot, prot_len); }
                    const char * operator*() const { return _buf.ptr(); }
                    operator bool() const { return _state == eOK; }
                    EState getState() const { return _state; }
                    idx getPos() const { return _prot_pos[0]; }
                    idx getLastLine() const { return _line; }
                    idx getLastLetterPos() const { return _pos; }
                    ProteinNmerIter & operator++();
            };

            enum EBlastMatrix {
                eBlosumDefault = 0,
                eBlosum45,
                eBlosum50,
                eBlosum62,
                eBlosum80,
                eBlosum90,
                ePam30,
                ePam70,
                ePam250
            };
            struct BlastMatrix {
                const char * letters;
                idx nletters;
                const idx * matrix;

                BlastMatrix() { sSet(this); }

                idx getScore(idx irow, idx icol) const {
                    if( irow > icol ) {
                        sSwap(irow, icol);
                    }
                    idx index = irow * nletters + icol - ((irow * (irow + 1)) >> 1);
                    return letters[index];
                }

                bool makeLinearizedMap(sDic<idx> & out_map) const;
            };
            static const BlastMatrix * getBlastMatrix(EBlastMatrix m = eBlosumDefault);
    };

    class sBioseqSet: public sBioseq
    {
        public:

            sBioseqSet()
            {
                totDim = 0;
                totDimVioseqlist = 0;
                needsReindex = false;
            }

            void attach(sBioseq * bioseq, idx seqNum = 0, idx seqCnt = sIdxMax, idx partialRangeStart = 0, idx partialRangeLen = sIdxMax);
            void reindex(void);
            sBioseq * ref(idx * inum, idx *seqCnt=0, idx mode=-1);
            idx refsList(idx inum);

            virtual void setmode(EBioMode mode)
            {
                if( biosR.dim() > 0 ) {
                    biosR[0]->setmode(mode);
                }
                return;
            }

            virtual void destroy(bool justMemoryFree = false)
            {
            }

            virtual void empty()
            {
                biosR.empty();
                refs.empty();
                seqInd.empty();
                totDim = 0;
                totDimVioseqlist = 0;
            }

            virtual idx dim(void)
            {
                if( totDimVioseqlist > 0 ) {
                    return totDimVioseqlist;
                }
                return totDim;
            }

            virtual void setDimVioseqlist(idx tot)
            {
                totDimVioseqlist = tot;
            }

            virtual idx getDimVioseqlist()
            {
                return totDimVioseqlist;
            }

            virtual EBioMode getmode()
            {
                return biosR[0]->getmode();
            }

            virtual idx num(idx num, idx readtypes = eReadBiological)
            {
                return readtypes < eReadTechnical ? 1 : 0;
            }

            virtual idx len(idx inum, idx iread = 0)
            {
                idx s;
                if( totDimVioseqlist > 0 ) {
                    idx newnum = biosR[0]->long2short(inum);
                    EBioMode newtemp = getmode();
                    setmode(eBioModeShort);
                    s = ref(&newnum)->len(newnum);
                    setmode(newtemp);
                } else {
                    s = ref(&inum)->len(inum, iread);
                }
                return s;
            }

            virtual const char * seq(idx inum, idx iread = 0)
            {
                const char *s;
                if( totDimVioseqlist > 0 ) {
                    idx newnum = biosR[0]->long2short(inum);
                    EBioMode newtemp = getmode();
                    setmode(eBioModeShort);
                    s = ref(&newnum)->seq(newnum);
                    setmode(newtemp);
                } else {
                    s = ref(&inum)->seq(inum, iread);
                }
                return s;
            }
            virtual const char * qua(idx inum, idx iread = 0)
            {
                const char *s;
                if( totDimVioseqlist > 0 ) {
                    idx newnum = biosR[0]->long2short(inum);
                    EBioMode newtemp = getmode();
                    setmode(eBioModeShort);
                    s = ref(&newnum)->qua(newnum);
                    setmode(newtemp);
                } else {
                    s = ref(&inum)->qua(inum, iread);
                }
                return s;
            }
            virtual const char * id(idx inum, idx iread = 0)
            {
                const char *s;
                if( totDimVioseqlist > 0 ) {
                    s = biosR[0]->id(inum);
                } else if( totDimVioseqlist == -2 ) {
                    idx newnum = ref(&inum)->short2long(inum);
                    setmode(eBioModeLong);
                    s = biosR[0]->id(newnum);
                    setmode(eBioModeShort);
                } else {
                    s = ref(&inum)->id(inum, iread);
                }
                return s;
            }
            virtual idx rpt(idx inum, idx iread = 0)
            {
                idx s;
                if( totDimVioseqlist > 0 ) {
                    s = 1;
                } else {
                    s = ref(&inum)->rpt(inum, iread);
                }
                return s;
            }
            virtual idx sim(idx inum, idx iread = 0)
            {
                idx s;
                if( totDimVioseqlist > 0 ) {
                    s = 0;
                } else {
                    s = ref(&inum)->sim(inum, iread);
                }
                return s;
            }
            virtual idx long2short(idx inum, idx iread = 0)
            {
                idx s;
                idx sum = 0;
                if( totDimVioseqlist > 0 ) {
                    return biosR[0]->long2short(inum);
                } else {
                    sBioseq *r = ref(&inum,0,1);
                    s = r->long2short(inum, iread);
                    if (refs.dim() > 1){
                        RefSeq * rs = refs.ptr(0);
                        for(idx ir=1; (ir<refs.dim()) && (biosR[rs->bioNum] != r); ++ir ) {
                            sum +=  biosR[rs->bioNum]->getshortCount();
                            rs=refs.ptr(ir);
                        }
                    }
                }
                return s + sum;
            }
            virtual bool getQuaBit(idx inum)
            {
                bool s;
                if( totDimVioseqlist > 0 ) {
                    idx newnum = biosR[0]->long2short(inum);
                    EBioMode newtemp = getmode();
                    setmode(eBioModeShort);
                    s = ref(&newnum)->getQuaBit(newnum);
                    setmode(newtemp);
                } else {
                    s = ref(&inum)->getQuaBit(inum);
                }
                return s;
            }

            virtual idx getlongCount()
            {
                RefSeq * rs;
                idx tDim = 0;
                for(idx ir = 0; ir < refs.dim(); ++ir) {
                    rs = refs.ptr(ir);
                    tDim += biosR[rs->bioNum]->getlongCount();
                }
                return tDim;
            }

            virtual idx getshortCount()
            {
                RefSeq * rs;
                idx tDim = 0;
                for(idx ir = 0; ir < refs.dim(); ++ir) {
                    rs = refs.ptr(ir);
                    tDim += biosR[rs->bioNum]->getshortCount();
                }
                return tDim;
            }
            virtual idx short2long(idx inum, idx iread = 0)
            {
                sBioseq *r = ref(&inum,0,0);
                idx s = r->short2long(inum, iread), sum = 0;
                if (refs.dim() > 1){
                    RefSeq * rs = refs.ptr(0);
                    for(idx ir=1; (ir<refs.dim()) && (biosR[rs->bioNum] != r); ++ir ) {
                        sum +=  biosR[rs->bioNum]->getlongCount();
                        rs=refs.ptr(ir);
                    }
                }
                return s + sum;

            }
            virtual idx startlen(idx num, idx iread = 0)
            {
                RefSeq *rs = refs.ptr(refsList(num));
                if( rs ) {
                    return rs->partialRangeStart;
                }
                return 0;
            }
            virtual idx endlen(idx num, idx iread = 0)
            {
                RefSeq *rs = refs.ptr(refsList(num));
                if( rs ) {
                    return rs->partialRangeEnd;
                }
                return 0;
            }

            idx dimRefs(void)
            {
                return refs.dim();
            }

            bool isFastA (){
                for(idx i = 0; i < dim(); ++i) {
                    const char * seqqua = qua(i);
                    if( (seqqua && *seqqua) && (getQuaBit(i) == false) ) {
                        return false;
                    }
                }
                return true;
            }

            idx countFastQs(){
                idx cnt = 0;
                for(idx i = 0; i < dim(); ++i) {
                    const char * seqqua = qua(i);
                    if( (seqqua && *seqqua) && (getQuaBit(i) == false) ) {
                        cnt++;
                    }
                }
                return cnt;
            }

        protected:

        private:

            struct RefSeq
            {
                    idx bioNum;
                    idx seqNum;
                    idx seqCnt;
                    idx longseqCnt;
                    idx shortseqCnt;
                    idx partialRangeStart;
                    idx partialRangeEnd;
            };

            sVec<sBioseq *> biosR;
            sVec<RefSeq> refs;

            sVec<idx> seqInd;
            idx totDim;
            idx totDimVioseqlist;
            bool needsReindex;
    };

}

#endif 
