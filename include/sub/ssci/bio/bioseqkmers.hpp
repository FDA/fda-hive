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
#ifndef sBio_seqkmer_hpp
#define sBio_seqkmer_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/var.hpp>

#include <ssci/bio/bioseqraw.hpp>

#include <zlib.h>

namespace slib {


    class sBioseqKMers 
    {
            

        private:
                sVec < unsigned short int  > m_big_map;
                sVec <char> m_kmer_bloom;
                idx m_cntCombinations;
                sFilePath kmer_path;
                sFilePath bloom_path;

        public:
            enum eKmerTableType { eKmerTable_None, 
                                  eKmerTable_Count,
                                  eKmerTable_Bloom,
                                  eKmerTable_Both
            };

            eKmerTableType type;
            idx m_kmer_size;

            struct RefInfo {
                sStrT ref_path;
                bool isReadOnly;
                bool isglobal;
            };

            RefInfo m_ref;
    
            typedef idx (*kmerCallbackType)(sBioseqKMers * myThis, void * param, const char * id, idx idNum, idx curLength, idx kmerSeq, const char * kmerLetters);
            
            kmerCallbackType kmerCallbackFunc;
            void * callbackParam;

            idx callBackKmers(const char * id, idx idNumRef, idx curLength, const char * sequenceChunk,  const char * qualityBits, idx totalLengthRead );

            static idx callBackStatic(void * param, const char * id, idx idNum, idx curLength, const char * sequenceChunk,  const char * qualityBits, idx totalLengthRead )
            {
                return ((sBioseqKMers * )param)->callBackKmers(id,idNum, curLength, sequenceChunk, qualityBits, totalLengthRead);
            }

            void setCallback(void * lParam, kmerCallbackType lfunc) {
                kmerCallbackFunc=lfunc;
                callbackParam=lParam;
                
            }

            sBioseqKMers() {
            };
            
            sBioseqKMers(const char * reference_filename, idx kmer_size, bool isReadOnly=false, bool isglobal=false, eKmerTableType ltype=eKmerTable_Both)
            {
                type=ltype;
                callbackParam=0;
                kmerCallbackFunc=0;

                if(!isglobal) { 
                    kmer_path.makeName(reference_filename, "%%dir/%" DEC "-mers%s.vec",kmer_size,isglobal ? "-global" : "");
                    bloom_path.makeName(reference_filename, "%%dir/%" DEC "-mers%s.bloom",kmer_size,isglobal ? "-global" : "");
                    if (sFile::exists(kmer_path.ptr())) {isReadOnly=true;}
                    m_big_map.init(kmer_path.ptr(),sMex::fSetZero | (isReadOnly  ? sMex::fReadonly : sMex::fMapRemoveFile ));
                    m_kmer_bloom.init(bloom_path.ptr(),sMex::fSetZero | (isReadOnly  ? sMex::fReadonly : sMex::fMapRemoveFile ));
                }else { 
                    m_big_map.init(sMex::fSetZero);
                    m_kmer_bloom.init(sMex::fSetZero);
                }
                
                m_kmer_size = kmer_size;
                m_cntCombinations = 1 << (2*m_kmer_size);

                m_ref.ref_path.printf(0,"%s", reference_filename);
                m_ref.isReadOnly = isReadOnly;
                m_ref.isglobal = isglobal;

            }

            idx processRaw ( ) {
                if (m_ref.isReadOnly) {
                    return 1;
                }

                sBioseqRaw raw_parser(m_ref.ref_path.ptr(),".fq,.fastq,.fa,.fasta",true);
                raw_parser.processRaw(sBioseqKMers::callBackStatic, (void*)this);

               
                return 0;


            }

            idx kmerCount(idx ref_num, idx kmerSequence){
                idx base = 0;
                if (!m_ref.isglobal) base = ref_num * m_cntCombinations;

                if(!((m_kmer_bloom[ (base+kmerSequence)/8 ])&( 1<<( (base + kmerSequence)%8) ) ))return 0;
                return m_big_map[base+kmerSequence];
            }

            idx kmerList (sStr & outList, idx ref_num) {
                 const char * ACGT="ACGT";
                 idx base=ref_num*m_cntCombinations;
                 idx end = base +m_cntCombinations;
                 idx kmerCnt = 0;
                 if (ref_num > ((m_big_map.dim()/m_cntCombinations) -1 ) ) {
                    return -1;
                 }
                 for (idx is = base; is<end; ++is) {
                    if ( !(m_big_map[is]) ) continue;
                    idx ip = is - base;
                    for ( idx in=0 ; in<m_kmer_size; ++in) {
                        idx let=( ip>>(2*(m_kmer_size-in-1)))&(0x3);
                        char nucleotide=ACGT[let];
                        outList.printf("%c",nucleotide);
                    }
                    outList.printf(":%d\n",m_big_map[is]);
                    kmerCnt+=m_big_map[is];
                 }
                 return kmerCnt;
            }

            idx getCombinations(){ return m_cntCombinations;}
            idx getBigMapDim(){ return m_big_map.dim();}

             void destroy(){
             }

            virtual ~sBioseqKMers()
            {
                   destroy();
            }
          
        };

        class sBioseqKMersAPI : public sBioseqKMers 
        {
            public:
                sVec < sBioseqKMers> perReferenceFile;

                sBioseqKMersAPI(idx kmerSize, const char * pathList00, const char * filenametmpl)
                {
                    init(kmerSize, pathList00, filenametmpl);
                };

                void init(idx kmerSize, const char * pathList00, const char * filenametmpl = "_") 
                {
                         
                    for (const char * subPath=pathList00; subPath; subPath=sString::next00(subPath)) {
                        sFilePath gfile(subPath,"%%dir/%s", filenametmpl);

                        sBioseqKMers * pNew= perReferenceFile.add(1);
                        new (pNew) sBioseqKMers(gfile.ptr(),kmerSize,true,false,sBioseqKMers::eKmerTable_None);
                    }

                }

                idx getKmerCountPerRef(const char * readId, const char * readSeq2Bits, idx readLen, idx kmerSize, sBioseqKMers * curRef, idx idNumRef);

        };

}

#endif 

