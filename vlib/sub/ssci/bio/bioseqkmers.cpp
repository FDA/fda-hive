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
#include <ssci/bio/bioseqkmers.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqalign.hpp>

#include <zlib.h>

using namespace slib;


idx sBioseqKMers::callBackKmers(const char * id, idx idNumRef, idx curLength, const char * sequenceChunk, const char * qualityBits, idx totalLengthRead )
{
    sStr buf; buf.resize(curLength/4+1);
    char * dst = buf.ptr();
    sStr kmerLetters;
;
    sBioseq::compressATGC_2Bit(dst,sequenceChunk, curLength);

    idx base = m_ref.isglobal ? 0 : idNumRef * m_cntCombinations;

    if(m_ref.isglobal){
        m_big_map.cut(0);
        m_kmer_bloom.cut(0);
    }
    if(type & eKmerTable_Count) {
        m_big_map.resize((idNumRef+1) * m_cntCombinations);
    }

    if(type & eKmerTable_Bloom) {
        m_kmer_bloom.resize( m_big_map.dim()/8 +1 );
    }
    for (idx ip=0; ip < curLength - m_kmer_size +1; ++ip) {
        idx combinedLetters=0;
        for (idx im=0; im < m_kmer_size; ++im) {
            char let = sBioseqAlignment::_seqBits(buf.ptr(), ip + im,0);
            combinedLetters=(combinedLetters<<2)|let;
        }
        idx bc=base+combinedLetters;
        if(type & eKmerTable_Bloom) {
            m_kmer_bloom[(bc)>>3] |= 1<<((bc)&0x07);
        }
        if(type & eKmerTable_Count){
            if(m_big_map[bc] < (unsigned short int ) -1 ) {
                m_big_map[bc] ++;
            }
        }

        if ( kmerCallbackFunc ) { 
            ((kmerCallbackFunc))(this,callbackParam,id, idNumRef, curLength, combinedLetters, kmerLetters.ptr());
        }

    }

   

    return m_kmer_size-1;
}


idx sBioseqKMersAPI::getKmerCountPerRef(const char * readId, const char * readSeq2Bits, idx readLen, idx kmerSize, sBioseqKMers * curRef, idx idNumRef) {

    if (curRef->m_kmer_size != kmerSize) {
        return -1;
    }
    
    idx totalFound = 0;

    for (idx ip=0; ip < readLen - kmerSize +1; ++ip) {
        idx combinedLetters = 0;

        for (idx im=0; im < kmerSize; ++im) {
            char let = sBioseqAlignment::_seqBits(readSeq2Bits, ip + im,0);
            combinedLetters=(combinedLetters<<2)|let;
        }

        totalFound += curRef->kmerCount(idNumRef, combinedLetters);
        
    }

    return totalFound;
}
