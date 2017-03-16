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
//#pragma once
#ifndef sBio_seqalign_hpp
#define sBio_seqalign_hpp

#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqhash.hpp>

namespace slib
{

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Sequence Collection Hashing class
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    //! Sequence Collection Hashing class.
    /*!
     *
     */
    class sBioseqAlignment {

    public:

        enum fAlignFlags{
            fAlignCompressed            =0x00000001, // fAlignLocal                    =0x00000000,

            fAlignForward               =0x00000010,
            fAlignBackward              =0x00000020, // vs Forward
            fAlignGlobal                =0x00000040, // vs Local
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

            fAlignKeepMarkovnikovMatch  =0x00100000, //!rich become richest

            fAlignSearchRepeats         =0x00200000,
            fAlignSearchTranspositions  =0x00400000,
            fAlignReverseEngine         =0x00800000,
            fAlign32BitPositions        =0x01000000,

            fSelectedAlignment          =0x02000000,

            fAlignKeepPairedOnly        =0x04000000,
            fAlignKeepPairOnSameSubject =0x08000000,
            fAlignKeepPairDirectionality=0x10000000,
            fAlignIsPairedEndMode       =0x20000000,

            fAlignLast
        };

        /*! This represents a single alignment */
        struct Al { // represents a single alignment
            /*! Both the subject and query IDs */
            idx ids; // idSub,idQry;
            /*! Both the subject and query start on the sequences. */
            idx starts; // subStart, qryStart; // where the alignment start on the sequences and
            /*! The size of the alignments. */
            idx lengths; // lenAlign , dimAlign // lenAlign is the size of the alignment and dimAlign is the size of the memory  (lenAlign maybe shorter especially after remappings )
            /*! Represents the flags for the structure and the score. */
            idx flscore; // flags and score

            /*! \return ID of the Subject of the alignment (the reference sequence). */
            inline idx idSub() {return ((idx)(ids&(0xFFFFFFFF))^0x80000000)-0x80000000;} //!< Returns the ID of the Subject of the alignment (the reference sequence). extend sign for negative subjects (mutual alignment)
            /*! \return ID of the Query of the alignment (the read ID). */
            inline idx idQry() {return (ids>>32)&(0xFFFFFFFF);} //!< Returns the ID of the Query of the alignment (the read ID).
            /*! \return Place value where the alignment starts on the subject (reference) sequence. */
            inline idx subStart() {return starts&(0xFFFFFFFF);} //!< Returns the value where the alignment starts on the subject (reference) sequence.
            /*! \return Place value where the alignment starts on the reference sequence. */
            inline idx qryStart() {return (starts>>32)&(0xFFFFFFFF);} //!< Returns the value where the alignment starts on the reference sequence.
            /*! \return The size of the alignment (length). */
            public:
            /*! \returns Subject starting position of alignment with un/compressed train 'm'. */
            inline idx getSubjectStart(idx * m) { return subStart() + m[0]; }
            /*! \returns Subject ending position of alignment with compressed train 'm'. */
            inline idx getSubjectEnd(idx * m) { idx last = m[0]; for(idx i=0 ; i<dimAlign();++i) last = ((m[i] < -1) ? last - m[i] - 1 : m[i++]); return subStart()+last; }
            /*! \returns Query starting position of alignment with un/compressed train 'm'. */
            inline idx getQueryStart(idx * m) { return qryStart() + m[1]; }
            /*! \returns Query endging position of alignment with compressed train 'm'. */
            inline idx getQueryEnd(idx * m) {idx last = m[1]; for(idx i=0 ; i<dimAlign();++i) last = (m[i] < -1) ? last - m[i] - 1 : m[++i]; return qryStart()+last; }
            /*! \returns Subject ending position of alignment with uncompressed train 'm'. */
            inline idx getSubjectEnd_uncompressed(idx * m) { return subStart()+m[2*lenAlign()-2]; }
            /*! \returns Query endging position of alignment with uncompressed train 'm'. */
            inline idx getQueryEnd_uncompressed(idx * m) { return qryStart()+m[2*lenAlign()-1]; }

            inline idx lenAlign() {return (lengths)&(0xFFFFFFFF);} //!< Returns the size of the alignment (length).
            /*! \return The size of the alignment in terms of memory. */
            inline idx dimAlign() {return (lengths>>32)&(0xFFFFFFFF);} //!< Returns the size of the alignment in terms of memory.
            /*! \return Flags associated with this alignment. */
            inline idx flags() {return (flscore&0xFFFFFFFF);}//!< Returns the flgags associated with this alignment.
            /*! \return Score associated with this alignment. */
            inline idx score() {return (flscore>>32)&(0xFFFFFFFF);}//!< Returns the score associated with this alignment.
            /*! \return true if Forward (nonComplement)*/
            inline idx isForward() {idx fl = flags(); return (fl&fAlignForward) && !(fl&fAlignForwardComplement);}
            /*! \return true if Forward Complement */
            inline idx isForwardComplement() {idx fl = flags(); return (fl&fAlignForward) && (fl&fAlignForwardComplement);}
            /*! \return true if Backward (nonComplement)*/
            inline idx isBackward() {idx fl = flags(); return (fl&fAlignBackward) && !(fl&fAlignBackwardComplement);}
            /*! \return true if Backward Complement*/
            inline idx isBackwardComplement() {idx fl = flags(); return (fl&fAlignBackward) && (fl&fAlignBackwardComplement);}

            /*! \param val is the sequence ID.*/
            inline void setIdSub(idx val) {ids=(ids&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));} //!< Set the subject (reference) sequence ID.
            /*! \param val is the sequence ID.*/
            inline void setIdQry(idx val) {ids=(ids&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);} //!< Set the query (read) sequence ID.
            /*! \param val is the reference start position with respect to this alignment. */
            inline void setSubStart(idx val) {starts=(starts&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));} //!< Set the start position of the subject (reference) sequence with respect to this alignment
            /*! \param val is the query start position with respect to this alignment.*/
            inline void setQryStart(idx val) {starts=(starts&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}  //!< Set the start position of the query (read) sequence with respect to this alignment.
            /*! \param val is the length of the alignment in bases. */
            inline void setLenAlign(idx val) {lengths=(lengths&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));}//!< Set the length of the alignment (in bases).
            /*! \param val is the size of the alignment in bites. */
            inline void setDimAlign(idx val) {lengths=(lengths&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}//!< Set the size of the alignment in memory.
            /*! \param val is bitwise container for all applicable flags. */
            inline void setFlags(idx val) {flscore=(flscore&0xFFFFFFFF00000000ull)|(val&(0xFFFFFFFF));} //!< Set the flags for the alignment.
            /*! \param val is the score for the alignment.
             */
            inline void setScore(idx val) {flscore=(flscore&0x00000000FFFFFFFFull)|((val&(0xFFFFFFFF))<<32);}//!< Set the score value for the alignment.

            inline void setFlagsOn(idx val) {flscore|=(val&(0xFFFFFFFF));}
            inline void setFlagsOff(idx val) {flscore&=((~val)&(0xFFFFFFFF));}

            /*! If the alignment exists, shifts the size of the alignment in memory by the size of the Al structure.  The function then
             * returns a pointer.
             * \return A pointer to the new Al structure memory position.
             */
            idx * match (void){ return dimAlign() ? (idx*)(sShift(this,sizeof(Al))) : zeroAlignment;} //idx * match (void){ return match(this);}
            /*! If the alignment exists and the alignment is not compressed, returns a pointer to the new memory location of Al once the structure
             * is shifted by an amount equal to the length of the alignment in nucleotides, multiplied by 2, multiplied by the integer size on the system
             * (for the full amount of memory needed to accommodate the flat).
             * \return A pointer to the new memory location of Al.
             */
            idx * lookup(void){ return (dimAlign() && !(flags()&fAlignCompressed))?(idx*)(sShift(this,sizeof(Al)+(2*lenAlign())*sizeof(idx))):zeroAlignment;}

            /*! This is amount of memory required for the (1,1,2,2,3,3, etc.) structure representing the alignment
             * and also the memory needed for the Al structure itself.
             * \return The amount of memory needed for the alignment.
             */
            idx sizeofFlat(void) { return (sizeof(Al)+dimAlign()*sizeof(idx)); } //!< Return the size of the Flat needed for the structure.

        };


        sBioseqHash bioHash;
        sVec < char > MatSW; // working buffer for coefficients of smith watermann
        sVec < char > MatBR; // for backtracking
        sVec < idx > SubScanQPos;
        idx cntScanQPos;
        sVec < idx > ExtensionGapBuff;
        char * QHitBitmask;
        char * SHitBitmask;
        idx extGapMidPos;
        sVec < idx > SimilarityBuf;
        sVec <idx> localBitMatch;


        idx costMatch,costGapOpen,costGapNext,costMismatch,costMismatchNext;
        idx computeDiagonalWidth;
        idx compactSWMatrix;
        real maxMissQueryPercent;
        idx minMatchLen, maxExtensionGaps;
        idx scoreFilter, hashStp,considerGoodSubalignments;
        idx allowShorterEnds;
        static idx zeroAlignment[2];
        idx looseExtenderMismatchesPercent;
        idx looseExtenderMinimumLengthPercent;
        idx maxHitsPerRead;
        idx maxSeedSearchQueryPos;
        idx selfSubjectPosJumpInNonPerfectAlignment,selfQueryPosJumpInNonPerfectAlignment;
        idx selfSimilairityBufferSize;
        idx minFragmentLength;
        idx maxFragmentLength;

        public:

        sBioseqAlignment (const char * fl=0, idx lenunit=11, idx lenalphabet=4) :
            bioHash(lenunit,lenalphabet) ,
            MatSW(sMex::fSetZero),
            MatBR(sMex::fSetZero),
            SubScanQPos(sMex::fSetZero),
            SimilarityBuf(sMex::fSetZero)
            {

                QHitBitmask=0;SHitBitmask=0;
                extGapMidPos=1024;
                ExtensionGapBuff.resizeM(extGapMidPos*2);
                extGapMidPos-=4;
                hashStp=1;
                selfSubjectPosJumpInNonPerfectAlignment=0;
                cntScanQPos=0;
            }

            sBioseqAlignment * init ( idx lenunit, idx lenalphabet) {
                bioHash.hdr.lenUnit=lenunit;
                bioHash.hdr.lenAlpha=lenalphabet;
                return this;
            }

        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Computational Functions
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/

    public:

        inline static char _seqBits(const char * seq, idx is, idx flags)
        {
            idx ibyte=is/4;
            idx ishift=(is%4)*2; // determine the byte number and the shift count
            idx val=(idx)((seq[ibyte]>>ishift)&0x3) ; // this particular base introduces this two bits
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
                res=( res<<2 ) | (idx)((seq[ibyte]>>ishift)&0x3) ; // this particular base introduces this two bits
            }
            return (udx)res;
        }

        udx read32letters (idx position, const idx * binarysequence, idx len, idx * endOfChar)
        {
            if(position % 32 == 0 && (len - position + 1) > 31)
                    return binarysequence[position/32];

            // In all other cases:

            udx k = binarysequence[position/32];
            udx l = binarysequence[(position/32) + 1];
            udx shift_k = (position % 32) * 2;

            k >>= shift_k; // Shift the k to the right by the position number (counting from the beginning of the memory block)
            l <<= (64 - shift_k); // shift the l to the left by the position number - 32

            k = k | l;

            if((len - position + 1) < 32){ // In case at the end of the sequence
                // Need to return some from the beginning of the sequence
                // This if statement doesn't take into account if there are less than 32 characters in the genome

                // Set the end of character
                //*endOfChar = (len - position + 1);
                *endOfChar = (len - position);

                //idx additionalShift = (64 - ((len - position) * 2));

                //udx mask = 0xFFFFFFFFFFFFFFFFUL;
                //udx mask = ~0;
                //mask >>= additionalShift;
                //mask >>= 5;
                //k &= mask;

                l = binarysequence [0]; // Start at the beginning again.
                //l <<= (((32 - (position % 32)) * 2) + 1);
                //mask = 0xFFFFFFFFFFFFFFFF;

                //mask <<= (len - position + 1) * 2;
                //if ((len - position + 1) * 2 != 64) mask = ~mask;
                //k &= mask;

                l <<= (len - position + 1) * 2;
                udx mask = 0xFFFFFFFFFFFFFFFFUL;
                mask <<= ((len - position + 1) * 2) + 2;
                l &= mask;

                k = k | l;
            }
            //printBWTstream (&k, 32);

            return k;
        }
        idx alignSeq( sVec < idx > * allHits ,sBioseq * Subs, const char * qry, idx qrylen, idx idSub, idx idQry, idx flagset, idx qrysim, idx * subfos=0, idx * qryfos=0);
        idx alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags, idx subbuflen, idx  qrybuflen, idx startFloatingDiagonal=0, idx * pLastQryEnd=0,  idx * pLastSubEnd=0);
        bool align2bioseqDefaultParameters (idx *flags);

        enum eReadAlignmentFlags{eAlRelativeToMultiple=0x00000001, eAlKeepGaps=0x00000002, eAlKeepSelfAlignment=0x00000004};
        static idx readMultipleAlignment(sVec < idx > * alSub,const char * src,idx len, idx flags=0, idx * alLen=0 , bool withIdLines=0, sBioseq * qry = 0);
        static idx getLUTMultipleAlignment(sVec < Al * > * alSub,sVec< sVec<idx> > & lutAlSub);

//        /*! \returns true if all bits for alignment filtering are 0. */
//        static bool keepAllMatches(idx flag) { return flag&(sBioseqAlignment::fAlignKeepFirstMatch|sBioseqAlignment::fAlignKeepBestFirstMatch|
//            sBioseqAlignment::fAlignKeepUniqueBestMatch|sBioseqAlignment::fAlignKeepAllBestMatches|
//            sBioseqAlignment::fAlignKeepRandomBestMatch|sBioseqAlignment::fAlignKeepUniqueMatch); }//!< Returns the true if no alignment filtering bit flag is set.

        /*! \returns true if bits for alignment filtering are set. */
        inline static bool doSelectMatches(idx flagSet) {
            return ( !(flagSet&sBioseqAlignment::fAlignIsPairedEndMode) && ( (flagSet&sBioseqAlignment::fAlignKeepBestFirstMatch) ||
                        (flagSet&sBioseqAlignment::fAlignKeepRandomBestMatch) ||
                        (flagSet&sBioseqAlignment::fAlignKeepAllBestMatches) ||
                        (flagSet&sBioseqAlignment::fAlignKeepUniqueMatch) ) ) ;
        }
        static idx remapAlignment(Al * from, Al * to, idx * mfrom0=0, idx * mto0=0,idx maxlen=0,idx readonly=0,sVec<idx> * lookup=0 ) ;
        static idx truncateAlignment(Al * hdr, idx maskBefore, idx maskAfter, idx issuborqry) ;
        /*! Returns the subject position of the query position provided by "pos" given an
         * alignment "hdr" and the train "to0".
         * If "valid" = 0 In/Dels are returned as -1
         * If "valid" = 1 last valid position is return instead of In/Del
         * If "valid" = 2 next valid position is return instead of In/Del! */
        static idx remapQueryPosition(Al * hrd, idx * to0, idx pos, idx valid = 0 );
        /*! Returns the query position of the subject position provided by "pos" given an
         * alignment "hdr" and the train "to0".
         * If "valid" = 0 In/Dels are returned as -1
         * If "valid" = 1 last valid position is return instead of In/Del
         * If "valid" = 2 next valid position is return instead of In/Del! */
        static idx remapSubjectPosition(Al * hrd, idx * to0, idx pos, idx valid = 0 );
        
        static idx selectBestAlignments(Al * alignmentMap,idx alignmentMapSize, idx qryOrSub=0, idx flags=0);
        static idx compressAlignment(Al * hdr, idx * m, idx * mdst );
        static idx uncompressAlignment(Al * hdr, idx * m, idx * mdst, bool reverse=false  );
        static idx reverseAlignment(Al * hdr, idx * m);

        static idx filterChosenAlignments(sVec< idx> * alignmentMap,idx idQryShift,idx flagSet, sStr * dst);

        static idx prepareMultipleAlignmentSrc(sStr * tempStr, const char * src, idx len,  bool  withIdlines);
    };

} // namespace 

#endif // sBio_seqalign_hpp

