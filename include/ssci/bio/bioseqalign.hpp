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
#ifndef sBio_seqalign_hpp
#define sBio_seqalign_hpp

#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqhash.hpp>
#include <slib/core/iter.hpp>
#include <slib/core/heap.hpp>


#define scanHitsMethodNEW
#undef scanHitsMethodOLD
#undef printMethodComparisonHits

namespace slib
{



    class sBioseqAlignment {

    public:
        enum fAlignFlags{
            fAlignCompressed            =0x00000001,

            fAlignForward               =0x00000010,
            fAlignBackward              =0x00000020,
            fAlignGlobal                =0x00000040,
            fAlignForwardComplement     =0x00000080,
            fAlignBackwardComplement    =0x00000100,
            fAlignMaxExtendTail         =0x00000200,
            fAlignCircular              =0x00000400,
            fAlignOptimizeDiagonal      =0x00000800,
            fAlignOptimizeDoubleHash    =0x00001000,
            fAlignIdentityOnly          =0x00002000,

            fAlignKeepFirstMatch        =0x00004000,
            fAlignKeepBestFirstMatch    =0x00008000,
            fAlignKeepAllBestMatches    =0x00010000,
            fAlignKeepRandomBestMatch   =0x00020000,
            fAlignKeepUniqueMatch       =0x00040000,
            fAlignKeepUniqueBestMatch   =0x00080000,

            fAlignKeepResolveMarkovnikov=0x00100000,
            fAlignKeepResolveBalanced   =0x00200000,
            fAlignKeepResolvedHits      =0x00400000,
            fAlignKeepResolvedSymmetry  =0x00800000,
            fAlignKeepResolveUnique     =0x01000000,

            fAlignSearchRepeats         =0x02000000,
            fAlignSearchTranspositions  =0x04000000,
            fAlignReverseEngine         =0x08000000,
            fAlign32BitPositions        =0x10000000,

            fSelectedAlignment          =0x20000000,

            fAlignKeepPairedOnly        =0x40000000,
            fAlignKeepPairOnSameSubject =0x80000000,
            fAlignKeepPairDirectionality=0x10000000,
            fAlignIsPairedEndMode       =0x200000000,

            fAlignLast
        };

        class Al {
        public:
            idx ids;
            idx starts;
            idx lengths;
            idx flscore;
            inline idx idSub() {return ((idx)(ids&(0xFFFFFFFF))^0x80000000)-0x80000000;}
            inline idx idQry() {return (ids>>32)&(0xFFFFFFFF);}
            inline idx subStart() {return starts&(0xFFFFFFFF);}
            inline idx qryStart() {return (starts>>32)&(0xFFFFFFFF);}
            
            static inline idx getQueryIndex(idx * m, idx i){return m[2*i+1];}
            static inline idx getSubjectIndex(idx * m, idx i){return m[2*i];}

            inline idx getSubjectStart(idx * m) { return subStart() + getSubjectIndex(m,0); }
            inline idx getSubjectEnd(idx * m) {
                if(!dimAlign()) return subStart() + lenAlign()-1;
                idx last = m[0];
                for(idx i=0 ; i<dimAlign();++i)
                    last = ((m[i] < -1) ? last - m[i] - 1 : (m[i++]==-1?last:m[i-1]));
                return subStart()+last;
            }
            inline idx getQueryStart(idx * m) { return qryStart() + getQueryIndex(m,0); }
            inline idx getQueryEnd(idx *m)
            {
                if (!dimAlign())
                    return qryStart() + lenAlign() - 1;
                idx last = m[1];
                for (idx i = 0; i < dimAlign(); ++i)
                    last = (m[i] < -1) ? last - m[i] - 1 : (m[i++] == -1 ? last : m[i]);
                return qryStart() + last;
            }
            inline idx getSubjectEnd_uncompressed(idx * m) { return subStart()+getSubjectIndex(m,lenAlign()-1); }
            inline idx getQueryEnd_uncompressed(idx * m) { return qryStart()+getQueryIndex(m,lenAlign()-1); }

            inline idx getQueryPosition(idx i, idx qrylen) { return isReverseComplement() ? (qrylen - 1 - getQueryPositionRaw(i)) : getQueryPositionRaw(i); }

            inline idx getQueryPosition(idx *m, idx i, idx qrylen) { return isDeletion(m, i) ? getQueryIndex(m, i) : getQueryPosition(getQueryIndex(m, i), qrylen); }
            inline idx getQueryPositionRaw(idx i) { return qryStart() + i; }

            inline idx getQueryPositionRaw(idx *m, idx i) { return getQueryPositionRaw(getQueryIndex(m, i)); }

            inline idx getSubjectPosition(idx i) { return subStart() + i; }

            inline idx getSubjectPosition(idx *m, idx i) { return isInsertion(m, i) ? getSubjectIndex(m, i) : getSubjectPosition(getSubjectIndex(m, i)); }

            inline char getQueryChar(idx *m, idx i, idx qrylen, const char *seq, const char * qua = 0, bool isquabit = 0) { return isDeletion(m, i) ? '-' : getQueryCharByPosition(getQueryPosition(m, i, qrylen), seq, qua, isquabit); }

            inline char getQueryChar(idx i, idx qrylen, const char *seq, const char  * qua = 0, bool isquabit = 0) { return getQueryCharByPosition(getQueryPosition(i, qrylen), seq, qua, isquabit); }

            inline char getQueryCharByPosition(idx i, const char *seq, const char * qua = 0, bool isquabit = 0) { return sBioseq::mapRevATGC[getQueryLetterByPosition(i, seq, qua, isquabit)]; }

            inline idx getQueryLetter(idx *m, idx i, idx qrylen, const char *seq, const char * qua = 0, bool isquabit = 0) { return isDeletion(m, i) ? getQueryIndex(m, i) : getQueryLetterByPosition(getQueryPosition(m, i, qrylen), seq, qua, isquabit); }

            inline idx getQueryLetter(idx i, idx qrylen, const char *seq, const char * qua = 0, bool isquabit = 0) { return getQueryLetterByPosition(getQueryPosition(i, qrylen), seq, qua, isquabit); }

            inline idx getQueryLetterByPosition(idx pos, const char *seq, const char * qua = 0, bool isquabit = 0) { return (qua && sBioseq::Qua(qua, pos, isquabit) == 0) ? 4 : (idx)sBioseqAlignment::_seqBits(seq, pos, flags()); }
            
            inline char getSubjectChar(idx *m, idx i, const char *seq) { return isInsertion(m, i) ? '-' : getSubjectCharByPosition(getSubjectPosition(m, i), seq); }
            
            inline char getSubjectChar(idx i, const char *seq) { return getSubjectCharByPosition(getSubjectPosition(i), seq); }
            inline char getSubjectCharByPosition(idx pos, const char *seq) { return sBioseq::mapRevATGC[getSubjectLetterByPosition(pos,seq)]; }

            inline idx getSubjectLetter(idx *m, idx i, const char *seq) { return isInsertion(m, i) ? getSubjectIndex(m, i) : getSubjectLetterByPosition(getSubjectPosition(m, i), seq); }
            
            inline idx getSubjectLetter(idx i, const char *seq) { return getSubjectLetterByPosition(getSubjectPosition(i), seq); }
            
            inline idx getSubjectLetterByPosition(idx pos, const char *seq) { return (idx)sBioseqAlignment::_seqBits(seq, pos); }

            idx countMatches(idx *m, idx qrylen, const char *qry, const char *sub, const char * qua = 0, bool isquabit = 0)
            {
                idx matches = 0;

                for (idx i = 0; i < lenAlign(); i++)
                {
                    idx qLet = getQueryLetter(m, i, qrylen, qry, qua, isquabit);
                    idx sLet = getSubjectLetter(m, i, sub);
                    if (qLet == sLet && qLet >= 0)
                        ++matches;
                }
                return matches;
            }

            real percentIdentity(idx *m, idx qrylen, const char *qry, const char *sub, const char * qua = 0, bool isquabit = 0) {
                return 100*(real)countMatches(m,qrylen,qry,sub,qua,isquabit)/lenAlign();
            }

            inline idx lenAlign() {return (lengths)&(0xFFFFFFFF);}
            inline idx dimAlign() {return (lengths>>32)&(0xFFFFFFFF);}
            inline idx flags() {return (flscore&0xFFFFFFFF);}
            inline idx score() {return (flscore>>32)&(0xFFFFFFFF);}
            inline idx isCompressed() {return flags()&fAlignCompressed;}
            inline idx isForward() {idx fl = flags(); return (fl&fAlignForward) && !(fl&fAlignForwardComplement);}
            inline idx isForwardComplement() {idx fl = flags(); return (fl&fAlignForward) && (fl&fAlignForwardComplement);}
            inline idx isReverse() {idx fl = flags(); return (fl&fAlignBackward) && !(fl&fAlignBackwardComplement);}
            inline idx isReverseComplement() {idx fl = flags(); return (fl&fAlignBackward) && (fl&fAlignBackwardComplement);}
            inline idx isComplement() {idx fl = flags(); return (fl&fAlignForwardComplement) || (fl&fAlignBackwardComplement);}
            inline idx hasGaps() { return dimAlign() && (isCompressed() ? (dimAlign() != 3) : dimAlign() != (lenAlign() * 2) ); }
            inline bool isPerfect(idx readlen) {return !hasGaps() && lenAlign()==readlen;}
            inline bool hasOverhang(idx * m, idx readlen) {return hasLeftOverhang(m) || hasRightOverhang(m,readlen);}
            inline bool hasLeftOverhang(idx * m) {return m[1]>0;}
            inline bool hasRightOverhang(idx * m, idx readlen) {return getQueryEnd(m) < readlen;}

            inline sBioseq::ESeqDirection getDirectionality() {idx fl = flags(); if (fl&fAlignBackward) {return (fl&fAlignBackwardComplement)?sBioseq::eSeqReverseComplement:sBioseq::eSeqReverse;} else {return (fl&fAlignForwardComplement)?sBioseq::eSeqForwardComplement:sBioseq::eSeqForward;}}

            inline void setIdSub(idx val) {ids=(ids&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));}
            inline void setIdQry(idx val) {ids=(ids&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}
            inline void setSubStart(idx val) {starts=(starts&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));}
            inline void setQryStart(idx val) {starts=(starts&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}
            inline void setLenAlign(idx val) {lengths=(lengths&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));}
            inline void setDimAlign(idx val) {lengths=(lengths&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}
            inline void setFlags(idx val) {flscore=(flscore&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));}
            inline void setScore(idx val) {flscore=(flscore&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}

            inline void setFlagsOn(idx val) {flscore|=(val&(0xFFFFFFFF));}
            inline void setFlagsOff(idx val) {flscore&=((~val)&(0xFFFFFFFF));}

            idx * match (void){ return dimAlign() ? (idx*)(sShift(this,sizeof(Al))) : zeroAlignment;}
            idx * lookup(void){ return (dimAlign() && !(flags()&fAlignCompressed))?(idx*)(sShift(this,sizeof(Al)+(2*lenAlign())*sizeof(idx))):zeroAlignment;}

            idx sizeofFlat(void) { return (sizeof(Al)+dimAlign()*sizeof(idx)); }

            inline static bool isInsertion(idx * m, idx i){return getSubjectIndex(m,i)<0;};
            
            inline static bool isDeletion(idx * m, idx i){return getQueryIndex(m,i)<0;};

            inline static bool isGap(idx * m, idx i){return isDeletion(m,i) || isInsertion(m,i);};

        };


        sBioseqHash bioHash;
        sBioHashHits hashHits;
        sVec < char > MatSW;
        sVec < char > MatBR;
        sVec < idx > SubScanQPos;
        sVec < idx > qryHashHits;
        idx cntScanQPos;
        sVec < idx > ExtensionGapBuff;
        char * QHitBitmask;
        char * SHitBitmask;
        idx extGapMidPos;
        sVec < idx > SimilarityBuf;
        sVec <idx> localBitMatch;


        idx costMatch,costGapOpen,costGapNext,costMismatch,costMismatchNext;
        idx costMatchFirst,costMatchSecond;
        idx computeDiagonalWidth;
        real maxMissQueryPercent;
        idx minMatchLen, maxExtensionGaps;
        bool isMinMatchPercentage;
        idx trimLowScoreEnds, trimLowScoreEndsMaxMM;
        idx scoreFilter, hashStp,considerGoodSubalignments;
        idx allowShorterEnds;
        static idx zeroAlignment[2];
        static idx resolveConflicts;
        idx looseExtenderMismatchesPercent;
        idx looseExtenderMinimumLengthPercent;
        idx maxHitsPerRead;
        idx maxSeedSearchQueryPos;
        idx ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment,ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment;
        idx selfSimilairityBufferSize;
        idx minFragmentLength;
        idx maxFragmentLength;
        
        public:

        sBioseqAlignment (const char * fl=0, idx lenunit=11, idx lenalphabet=4) :
            bioHash(lenunit,lenalphabet),
            hashHits(lenunit),
            MatSW(sMex::fSetZero),
            MatBR(sMex::fSetZero),
            SubScanQPos(sMex::fSetZero),
            SimilarityBuf(sMex::fSetZero),
            localBitMatch(sMex::fSetZero)
            {
                minMatchLen=0;
                QHitBitmask=0;SHitBitmask=0;
                extGapMidPos=1024;
                ExtensionGapBuff.resizeM(extGapMidPos*2);
                extGapMidPos-=4;
                hashStp=1;
                cntScanQPos=0;
                isMinMatchPercentage = false;
                selfSimilairityBufferSize = minFragmentLength = maxFragmentLength = ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment = ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment = 0;
                maxMissQueryPercent = maxSeedSearchQueryPos = looseExtenderMinimumLengthPercent = looseExtenderMismatchesPercent = allowShorterEnds = trimLowScoreEnds = trimLowScoreEndsMaxMM = scoreFilter = maxExtensionGaps = 0;
                costMatch = costMatchFirst = costMatchSecond = 5;
                computeDiagonalWidth = 0;
                costGapOpen  = -12;
                costGapNext = costMismatch = -4;
                costMismatchNext = -6;
                considerGoodSubalignments = 1;
                maxHitsPerRead = 50;
            }

            sBioseqAlignment * init ( idx lenunit, idx lenalphabet) {
                bioHash.hdr.lenUnit=lenunit;
                bioHash.hdr.lenAlpha=lenalphabet;
                return this;
            }


    public:

        inline static char _seqBits(const char * seq, idx is, idx flags = 0)
        {
            idx ibyte=is/4;
            idx ishift=(is%4)*2;
            idx val=(idx)((seq[ibyte]>>ishift)&0x3) ;
            if( ( (flags&fAlignForwardComplement) &&(flags&fAlignForward) ) ||
                ( (flags&fAlignBackwardComplement) &&(flags&fAlignBackward) )
                ) return (char)sBioseq::mapComplementATGC[val];
            return (char)val;
        }
        inline static udx _seqBytes(const char * seq, idx is, idx bytes)
        {
            udx res=0;
            for( idx i=0 ; i<bytes; ++i) {
                idx ibyte=(is+i)/4;
                idx ishift=((is+i)%4)*2;
                res=( res<<2 ) | (idx)((seq[ibyte]>>ishift)&0x3) ;
            }
            return (udx)res;
        }

        udx read32letters (idx position, const idx * binarysequence, idx len, idx * endOfChar)
        {
            if(position % 32 == 0 && (len - position + 1) > 31)
                    return binarysequence[position/32];


            udx k = binarysequence[position/32];
            udx l = binarysequence[(position/32) + 1];
            udx shift_k = (position % 32) * 2;

            k >>= shift_k;
            l <<= (64 - shift_k);

            k = k | l;

            if((len - position + 1) < 32){

                *endOfChar = (len - position);



                l = binarysequence [0];


                l <<= (len - position + 1) * 2;
                udx mask = 0xFFFFFFFFFFFFFFFFUL;
                mask <<= ((len - position + 1) * 2) + 2;
                l &= mask;

                k = k | l;
            }

            return k;
        }
        idx alignSeq( sVec < idx > * allHits ,sBioseq * Subs, const char * qry, idx qrylen, idx idSub, idx idQry, idx flagset, idx qrysim, idx * subfos=0, idx * qryfos=0);
        idx alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags, idx subbuflen, idx  qrybuflen, idx startFloatingDiagonal=0, idx * pLastQryEnd=0,  idx * pLastSubEnd=0);
        Al * alignSWProfile(sVec < idx > & al, const char ** sub, idx ** sub_m, idx substart, idx sublen, idx subcnt, const char ** qry, idx ** qry_m, idx qrybuflen, idx qrystart, idx qrylen, idx qrycnt, idx flags);
        Al * backTracking(sVec < idx > * allHits, idx substart, idx sublen, idx qrystart, idx qrylen, idx SWMatrixWidth, idx floatSZ, idx maxS, idx maxQ, idx maxAll, idx flags);
        bool align2bioseqDefaultParameters (idx *flags);

        enum eReadAlignmentFlags{eAlRelativeToMultiple=0x00000001, eAlKeepGaps=0x00000002, eAlKeepSelfAlignment=0x00000004};
        static idx readMultipleAlignment(sVec < idx > * alSub,const char * src,idx len, idx flags=0, idx * alLen=0 , bool withIdLines=0, sBioseq * qry = 0);
        static idx getLUTMultipleAlignment(sVec < Al * > * alSub,sVec< sVec<idx> > & lutAlSub);


        inline static bool doSelectMatches(idx flagSet) {
            return ( !(flagSet&sBioseqAlignment::fAlignIsPairedEndMode) && ( (flagSet&sBioseqAlignment::fAlignKeepBestFirstMatch) ||
                        (flagSet&sBioseqAlignment::fAlignKeepRandomBestMatch) ||
                        (flagSet&sBioseqAlignment::fAlignKeepAllBestMatches) ||
                        (flagSet&sBioseqAlignment::fAlignKeepUniqueMatch) ) ) ;
        }

        static idx remapAlignment(Al * from, Al * to, idx * mfrom0=0, idx * mto0=0,idx maxlen=0,idx readonly=0,sVec<idx> * lookup=0 ) ;

        static idx truncateAlignment(Al * hdr, idx maskBefore, idx maskAfter, idx issuborqry) ;
        static idx remapQueryPosition(Al * hrd, idx * to0, idx pos, idx valid = 0 );
        static idx remapSubjectPosition(Al * hrd, idx * to0, idx pos, idx valid = 0 );
        

        static idx selectBestAlignments(Al * alignmentMap,idx alignmentMapSize, idx qryOrSub=0, idx flags=0);
        static idx compressAlignment(Al * hdr, idx * m, idx * mdst );
        static idx * uncompressAlignment(Al *hdr, idx *m, idx *mdst = 0, bool reverse = false);
        static idx reverseAlignment(Al * hdr, idx * m);

        static idx filterChosenAlignments(sVec< idx> * alignmentMap,idx idQryShift,idx flagSet, sStr * dst);

        static idx prepareMultipleAlignmentSrc(sStr * tempStr, const char * src, idx len,  bool  withIdlines);
    };

    class sBioseqAlIter: public sIter<sBioseqAlignment::Al * const, sBioseqAlIter>
    {
        protected:
            sBioseqAlignment::Al *_alstart, *_alend, * _alrec;
            idx _i;
            bool _valid;

            inline bool isEof() const { return _alstart == 0 || _alrec == 0 || _alrec >= _alend; }

            void nextRec() {
                if( unlikely(isEof()) ) {
                    _valid = false;
                } else {
                    _valid = true;
                    _alrec = sShift(_alrec, _alrec->sizeofFlat() );
                    if( unlikely(isEof()) ) {
                        _valid = false;
                    }
                }
            }

            void init(const char * buf, const char * bufend)
            {
                _alrec = _alstart = static_cast<sBioseqAlignment::Al *>((void*)buf);
                _alend = static_cast<sBioseqAlignment::Al *>((void*)(!buf ? NULL : bufend ? bufend : buf + strlen(buf)));
                _i = 0;
                _valid = false;
                if( !isEof() ) {
                    _valid = true;
                }
            }


        public:
            inline void requestData_impl() {}
            inline void releaseData_impl() {}
            inline bool readyData_impl() const { return true; }
            inline bool validData_impl() const { return _valid; }
            inline idx pos_impl() const { return _i; }
            inline idx segmentPos_impl() const { return 0; }

            inline void reset( const sBioseqAlIter &rhs ) {
                _alrec = rhs._alrec;
                _i = rhs._i;
                _valid = rhs._valid;
            }

            sBioseqAlIter(const char *buf = NULL, const char *bufend = NULL) { init(buf, bufend); }
            sBioseqAlIter(const sFil *f) { init(f->ptr(), f->last()); }
            sBioseqAlIter(const sFil &f) { init(f.ptr(), f.last()); }

            sBioseqAlIter(const sBioseqAlIter &rhs) : _alstart(rhs._alstart), _alend(rhs._alend) { reset(rhs); }


            inline sBioseqAlIter* clone_impl() const { return new sBioseqAlIter(*this); }
            inline sBioseqAlIter& operator++()
            {
                ++_i;
                nextRec();
                return *this;
            }

            inline sBioseqAlignment::Al * const dereference_impl() const { return _alrec; }
    };

    struct isAlLessThanOrEqual {
        inline bool operator()( sBioseqAlignment::Al * x, sBioseqAlignment::Al * y ) const {
            return x->idQry() == y->idQry() ? (x->idSub() <= y->idSub()) : (x->idQry() <= y->idQry());
        }
    };

    template<class Tcmp = isAlLessThanOrEqual>
    class sBioseqAlBundleIter : public sIter<sBioseqAlignment::Al * const, sBioseqAlBundleIter<Tcmp> >
    {
        protected:
            sVec<sBioseqAlIter> _iters;
            sBioseqAlignment::Al * _val;
            idx _i, _minIndex;
            Tcmp tcmp;
            sHeap<sBioseqAlignment::Al *, Tcmp> _heap;

            void init( const sBioseqAlBundleIter &rhs )
            {
                if( rhs._iters.dim() ) {
                    _iters.resize( rhs._iters.dim() );
                    for( idx i = 0; i < rhs._iters.dim() ; ++i) {
                        _iters[i] = rhs._iters[i];
                    }
                }

                _val = rhs._val;
                _i = rhs._i;
                _minIndex = rhs._minIndex;
                _heap.setCmp(&tcmp);
                _heap.reset( rhs._heap );
            }

            void init(const sBioseqAlIter * iters, idx iters_cnt) {
                _i = _minIndex = 0;
                _heap.setCmp(&tcmp);

                if(iters_cnt) {
                    _iters.resize(iters_cnt);
                    for( idx i = 0; i < iters_cnt ; ++i) {
                        _iters[i] = iters[i];
                        _heap.push(static_cast<sBioseqAlignment::Al *>(*iters[i]));
                    }
                    _minIndex = _heap.peekIndex();
                }
                getVal();
            }

            inline void getVal()
            {
                if( _heap.dim() ) {
                    _val = *_iters[_minIndex];
                }
            }

            inline void incrementMin()
            {
                if( _iters[_minIndex].valid() && _heap.dim() ) {
                    if( (++_iters[_minIndex]).valid() ) {
                        _heap.adjust(_minIndex,static_cast<sBioseqAlignment::Al *>(*_iters[_minIndex]));
                    } else {
                        _heap.remove(_minIndex);
                    }
                }
                if( _heap.dim() ) {
                    _minIndex = _heap.peekIndex();
                }
            }

        public:

            sBioseqAlBundleIter(const sBioseqAlIter * iters = 0, idx iters_cnt = 0) : _heap(NULL) { init(iters,iters_cnt); }
            sBioseqAlBundleIter(const sVec<sBioseqAlIter> & iters) : _heap(NULL) { init(iters.ptr(),iters.dim()); }
            sBioseqAlBundleIter(const sBioseqAlBundleIter &rhs) : _heap(NULL) { init(rhs); }

            sBioseqAlBundleIter& operator=(const sBioseqAlBundleIter &rhs) { init(rhs); return *this; }

            inline void reset( const sBioseqAlBundleIter &rhs ) {
                if( rhs._iters.dim() ) {
                    for( idx i = 0; i < rhs._iters.dim() ; ++i) {
                        _iters[i].reset( rhs._iters[i] );
                    }
                }

                _val = rhs._val;
                _i = rhs._i;
                _minIndex = rhs._minIndex;
                _heap.reset( rhs._heap );
            }

            inline idx cnt() { return _i; }
            inline void requestData_impl() {}
            inline void releaseData_impl() {}
            inline bool readyData_impl() const { return true; }
            inline bool validData_impl() const { return _heap.dim(); }
            inline sBioseqAlBundleIter& increment_impl()
            {
                incrementMin();
                getVal();
                ++_i;
                return *this;
            }

            inline sBioseqAlignment::Al * const dereference_impl() const { return _val; }


            static idx getIters(sFil & f, sVec<sBioseqAlIter> & iters) { return getIters(&f, iters);}
            static idx getIters(sFil * f, sVec<sBioseqAlIter> & iters) {
                sBioseqAlIter it(f);
                if( !it.valid() )
                    return 0;

                sBioseqAlignment::Al *prv = *it, *hdr = *it, *f_hdr = *it, *s_hdr = 0;
                idx ii=0;
                for(; it.valid(); ++it) {
                    ++ii;
                    hdr = *it;
                    if( prv->idQry() > hdr->idQry() ) {
                        if( !s_hdr ) {
                            s_hdr = f_hdr;
                        }
                        *iters.add() = sBioseqAlIter((const char *)s_hdr,(const char *)hdr);
                        s_hdr = hdr;
                    }
                    prv = hdr;
                }
                if( s_hdr != hdr ) {
                    if( !s_hdr )
                        s_hdr = f_hdr;
                    *iters.add() = sBioseqAlIter((const char *) s_hdr, f->last());
                }

                return iters.dim();
            }
    };

    typedef sBioseqAlBundleIter<> sBioseqAlBundleAscIter;

}

#endif 
