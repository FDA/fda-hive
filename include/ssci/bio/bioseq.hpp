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

#define sBIO_ATGC_SEQ_2BIT

namespace slib {

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ sBioseq class -main sequence container virtual class
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
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
            // row/sequence/spot count
            virtual idx dim(void)
            {
                return 0;
            }

            typedef idx (*callbackType)(void *param, sStr * buf, idx initid, idx initseq, idx initqua, idx seqlen);
            callbackType print_callback;
            void * print_callbackParam;
            /* Example of declaration
             *      sFilterseq::filterParams params;
                    params.complexityEntropy = 1.2;
                    params.complexityWindow = 30;
                    Sub.print_callback = sFilterseq::sequenceFilterStatic;
                    Sub.print_callbackParam = &params;
             *
             */

            typedef enum EBioMode_enum
            {
                eBioModeShort       = 0,
                eBioModeLong
            } EBioMode;

            static bool isBioModeLong (idx mode) { return mode==eBioModeLong;}
            static bool isBioModeShort (idx mode) { return mode==eBioModeShort;}
            // number of reads in a row - 1 - based, 0 - whole row!!!
            enum eReadTypes
            {
                eReadBiological,
                eReadAll,
                eReadTechnical
            };
//        virtual idx num(idx num, idx readtypes = eReadBiological){return 0;} // bio = 0 | all = 1 | tech = 2

            // len/seq/qua/id of a read in a row : (-1) all, otherwise the index
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
            } // mode
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
            /**
             * return the count of the bioseq file in long mode
             */
            idx longcount()
            {
                /**
                 *  it counts manually all the repetitions included for each row and returns the final count
                 */
                idx longdim = 0;
                for(idx i = 0; i < dim(); i++) {
                    longdim += rpt(i);
                }
                return longdim;
            }

            /**
             * start, end - are 1 - based, 0 means whole range
             * return number of rows printed
             */
            idx printFastX(sStr * outFile, bool isFastq, idx start = 0, idx end = 0, idx shift = 0, bool keepOriginalId = false, bool appendLength = false, idx iread = 0, sVec<idx> * randomIdQueries = 0, bool restoreNs = true, idx filterNs = 0);
            /**
             * row - 0-based sequence number in the set
             * start - 0-based starting position within read, negative means starting from the end
             * length - length of the subsequence of the read, 0 means whole read
             * shift - shift ID to start with
             * keepOriginalId - explicitly show Original ID
             * iread - SRA stores forward and reverse reads together, set iread=0 to skip this functionality
             * isRevCmp - print if 0 (rev), or 1 (complement) or 2 (reverse complement)
             * restoreNs - if true, it will print the N's in the sequence
             * filterNs - percentage of N's that it will allow to print, if = 40, it means 40 percent of the length
             */
//            idx printFastXRow(sFil * outFile, bool isFastq, idx row, idx start = 0, udx length = 0, idx shift = 0, bool keepOriginalId = false, bool numIdOnly = false, const char *appendID = 0, idx iread = 0, idx isRevCmp = 0, bool filterNs = 0);
            idx printFastXRow(sStr * outFile, bool isFastq, idx row, idx start = 0, udx length = 0, idx shift = 0, bool keepOriginalId = false, bool numIdOnly = false, const char *appendID = 0, idx iread = 0, idx isRevCmp = 0, bool restoreNs = true, idx filterNs = 0, bool appendRptCount = true, bool SAMCompatible = false);

        public:
            idx printFastXData(sStr * outFile, idx seqlen, const char *seqid, const char *seqs, const char *qua, idx subrpt, idx iread = 0);
            static idx atgcModel;
            sStr idstr, seqstr, quastr, outstr;

            enum eATGCModel
            {
                eATGC = 0,
                eACGT
            };
            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
            // _/
            // _/ Statics
            // _/
            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/

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

            //#ifdef sBIO_ATGC_SEQ_2BIT
#define compressATGC compressATGC_2Bit
#define uncompressATGC uncompressATGC_2Bit
            //#else
            //    #define compressATGC compressATGC_1Byte
            //    #define uncompressATGC uncompressATGC_1Byte
            //#endif

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
            //mapCodeAA should alwayd be used through mapCodon function to ensure the right genetic code
            //listAA should also remain private to avoid hacks around mapCodeAA. Use through functions that do Codon->AA or mappedCodon->AA
            static ProtAA listAA[];
            static sVec<idx> mapCodeAA;
            static unsigned char mapAAlet2ilist[256]; //! map from char code to 1 + index of ProtAA, or 0 (invalid)

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

//        static bool complexityFilter( const char * seq, idx len ,idx complexityWindow, real complexityEntropy );
//        static idx primersFilter(const char *seq, idx seqlen, const char *primer, idx primlen, idx flags);

            //! Iterates over nmers of a protein FASTA file; ignores sequence ID lines
            class ProteinNmerIter {
                public:
                    enum EState {
                        eOK, //!< nmer at current position has been read
                        eEOF, //!< end of file
                        eError //!< invalid file format
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
                    //! get current nmer, normalized to upper-case
                    const char * operator*() const { return _buf.ptr(); }
                    operator bool() const { return _state == eOK; }
                    EState getState() const { return _state; }
                    //! get current position in protein FASTA, corresponding to start of the current nmer
                    idx getPos() const { return _prot_pos[0]; }
                    //! get last line which was read (0-based, possibly in middle of nmer if error happened)
                    idx getLastLine() const { return _line; }
                    //! get last position which was read (0-based, possibly in middle of nmer if error happened)
                    idx getLastLetterPos() const { return _pos; }
                    //! read next nmer
                    ProteinNmerIter & operator++();
            };

            enum EBlastMatrix {
                eBlosumDefault = 0, //!< = eBlosum62
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

                //! get the score at specified row/column
                idx getScore(idx irow, idx icol) const {
                    if( irow > icol ) {
                        sSwap(irow, icol);
                    }
                    idx index = irow * nletters + icol - ((irow * (irow + 1)) >> 1);
                    return letters[index];
                }

                //! generate a map from letter to number such that AA which are likely to substitute get assigned numbers which are clustered together
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
            void reindex(void); // re-indexing function
            sBioseq * ref(idx * inum, idx *seqCnt=0, idx mode=-1); // returns the sBioseq * object and its offset
            idx refsList(idx inum); // returns the RefSeq structure associated to an offset

            virtual void setmode(EBioMode mode)
            {
                if( biosR.dim() > 0 ) {
                    biosR[0]->setmode(mode);
                }
//            ref (&inum)->setmode(inum);
//            totDim = ref(&inum)->dim();
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

            // row/sequence/spot count
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
//            return ref(&inum)->len(inum,iread);
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
//            return ref(&inum)->seq(inum,iread,ipos,ilen);
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
//            return ref(&inum)->qua(inum, iread);
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
//            return ref(&inum)->id(inum, iread);
                const char *s;
                if( totDimVioseqlist > 0 ) {
                    s = biosR[0]->id(inum);
                } else if( totDimVioseqlist == -2 ) {
                    idx newnum = ref(&inum)->short2long(inum);
                    setmode(eBioModeLong);
                    s = biosR[0]->id(newnum);
                    setmode(eBioModeShort);
//                EBioMode newtemp = biosR[0]->getmode();
//                biosR[0]->setmode(eBioModeLong);
//                biosR[0]->setmode(newtemp);
                } else {
                    s = ref(&inum)->id(inum, iread);
                }
                return s;
            }
            virtual idx rpt(idx inum, idx iread = 0)
            {
//            return ref(&inum)->rpt(inum, iread);
                idx s;
                if( totDimVioseqlist > 0 ) {
                    s = 1;
//                idx newnum=biosR[0]->long2short(inum);
//                EBioMode newtemp = getmode();
//                setmode(eBioModeShort);
//                s = ref(&newnum)->rpt(newnum);
//                setmode(newtemp);
                } else {
                    s = ref(&inum)->rpt(inum, iread);
                }
                return s;
            }
            virtual idx sim(idx inum, idx iread = 0)
            {
//            return ref(&inum)->sim(inum, iread);
                idx s;
                if( totDimVioseqlist > 0 ) {
                    s = 0;
//                idx newnum=biosR[0]->long2short(inum);
//                EBioMode newtemp = getmode();
//                setmode(eBioModeShort);
//                s = ref(&newnum)->sim(newnum);
//                setmode(newtemp);
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

            // each record defines a multiple or a single sequences in the container
            struct RefSeq
            {
                    idx bioNum; // the serial number of sBioseq in the biosP this sequence originates from
                    idx seqNum; // the first sequence being pushed
                    idx seqCnt; // how many is being pushed in current mode
                    idx longseqCnt; // how many is being pushed in long mode
                    idx shortseqCnt; // how many is being pushed in short mode
                    idx partialRangeStart; // start of the range within a sequence
                    idx partialRangeEnd; // end of the range within a sequence
            };

            sVec<sBioseq *> biosR; // managed sBioseq objects;
            sVec<RefSeq> refs; // list of references from this bioseq

            sVec<idx> seqInd;
            idx totDim; // total dimension
            idx totDimVioseqlist;
            bool needsReindex; // flag signifying the need to reindex the sBioseqSet
    };

} // namespace

#endif // sBio_seq_hpp

