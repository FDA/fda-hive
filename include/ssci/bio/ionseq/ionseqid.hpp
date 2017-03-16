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
#ifndef sBio_ionseqid_hpp
#define sBio_ionseqid_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>

namespace slib {
//    class StringRule
//    {
//        public:
//            enum eTypeRule
//            {
//                undefined = 0,
//                matchString,
//                rangeString
//            };
//            eTypeRule eRule;
//            idx ruleStart;
//            idx ruleLen;
//
//            StringRule(){
//                init (undefined, -1, -1);
//            }
//            void init (eTypeRule myrule, idx start, idx len)
//            {
//                eRule = myrule;
//                ruleStart = start;
//                ruleLen = len;
//            }
//            void addnewRule (eTypeRule myrule, idx start, idx len)
//            {
//                init (myrule, start, len);
//            }
//    };

    class IonSeqID
    {
            struct stringBigContainer
            {
                idx stringpos;
                idx idref;
            };

            sVec <stringBigContainer> indexBigC;
            sStr bigContainer;
            sVec <udx> idIndex;
            sStr idsCompressedTrain;
            sVec <idx> libraryIndex;
            sStr libraryContainer;

            unsigned char LastBitChar;
            unsigned char Last2BitsChar;
            udx LastBitUdx;
            idx matchFilterHead;
            idx matchFilterTail;
            idx numIds;
            enum Container {
                BigContainer = 0x01,
                CompressedContainer = 0x02
            };

//            struct indStats{
//                    idx length;
//                    idx count;
//            };

            struct IDStats{
                    sVec <idx> compBin;
                    idx count;
                    idx headLength;
                    idx middleLength;
                    idx tailLength;
            };
            IDStats stats;



            IonSeqID(const IonSeqID&);
            const IonSeqID& operator=(const IonSeqID&);

        public:
            IonSeqID (){
                init ();
            }

            ~IonSeqID(){

            }

            bool init(){
                indexBigC.cut(0);
                bigContainer.cut(0);
                idIndex.cut(0);
                idsCompressedTrain.cut(0);
                // Add the first element to point at 0
                idIndex.vadd(1, 0);
                libraryIndex.cut(0);
                libraryContainer.cut(0);
                LastBitChar = 1 << 7;
                Last2BitsChar = 3 << 6;
                LastBitUdx = (udx)1 << 63;
                matchFilterHead = 5;
                matchFilterTail = 5;
                numIds = 0;
                addStringCompressedLibrary("", 1);
                return true;
            }

            void setmatchFilters(idx head, idx tail){
                matchFilterHead = head;
                matchFilterTail = tail;
            }

            void startStats (idx numC = 0){
                stats.compBin.resize(numC);
                for (idx i = 0; i < numC; ++i){
                    stats.compBin[i] = 0;
                }
                stats.headLength = 0;
                stats.middleLength = 0;
                stats.tailLength = 0;
                stats.count = 0;
            }

            idx getNewId()
            {
                ++numIds;
                return numIds-1;
            }

            idx getDimCompressedLibrary (){
                return libraryIndex.dim();
            }

            idx addStringCompressedLibrary(const char *word, idx lenword = 0){
                if (!lenword){
                    lenword = sLen (word);
                }
                idx *index = libraryIndex.add(1);
                *index = libraryContainer.length();
                libraryContainer.add(word, lenword);
                libraryContainer.add0(1);
                return libraryIndex.dim() - 1;
            }

            const char *getStringfromCompressedLibrary (idx iword, idx pos = 0, idx *len = 0)
            {
                idx iwrd = libraryIndex[iword];
                const char *charPos = libraryContainer.ptr(iwrd);
                if (len){
                    *len = sLen (charPos + pos);
                }
                return charPos + pos;
            }

            const char * getLetterFromBigContainer (idx iword, idx pos = 0, idx *idpos = 0, bool wStats = false)
            {
                idx iwrd = indexBigC[iword].stringpos;
                const char *charPos = bigContainer.ptr(iwrd);
                if (idpos){
                    *idpos = sLen (charPos + pos); //indexBigC[iword].idref;
                }
                return charPos + pos;
            }

            idx lcommon(const char *s, idx strlen1, const char *t, idx strlen2);
            idx endsWith(const char *str, idx strlen, const char *suffix, idx suffixlen);
            bool addID(const char *id, idx lenid = 0);
            idx searchCompressed(const char *s, idx len, bool tail = false, idx *match = 0);
            idx searchBigContainer (const char *s, idx lenString, idx *lenmatch = 0);
            idx compressString (udx startLibrary, const char *s, idx slen, idx start, idx end, udx endLibrary);
            idx addBigContainer(idx idNum, const char *word, idx lenword = 0);
            const char *uncompressString (sStr *buf, const char *s, idx *len=0, bool wStats = false);

            bool setIndex(idx idnum, udx contNum, Container cont){
                udx val = (cont == BigContainer) ? (contNum | LastBitUdx) : (contNum) & ~(LastBitUdx);
//                udx val = (-contNum ^ (cont==BigContainer ? 1 : 0)) & (LastBitUdx);
                idnum += 1; // Zero position is reserved
                if( idnum < idIndex.dim() ) {
                    // Already existing
                    *idIndex.ptr(idnum) = val;
                } else {
                    // Add a new one
                    idIndex.vadd(1, val);
                }

                return true;
            }

            const char *getString (sStr *buf, idx inum, idx *idlen, bool wStats = false)
            {
                udx regid = idIndex[inum+1];
                udx bit = regid  & LastBitUdx;
                udx num = (regid & ~(LastBitUdx));
                const char *retid = 0;
                if (bit){
                    // Goes to the Big Container
                    retid = getLetterFromBigContainer(num, 0, idlen, wStats);
                    buf->addString(retid, *idlen);
                }
                else {
                    // Goes to the Compress Container
                    const char *s = idsCompressedTrain.ptr(num*4);
                    retid = uncompressString (buf, s, idlen, wStats);
//                    retid = getStringfromLibrary(num, 0, idlen);
                }
                return retid;
            }

            idx getStatInfo(sStr *buf)
            {
                idx stringsize = 0;
                stringsize += idIndex.dim() * sizeof(udx);
                stringsize += indexBigC.dim() * sizeof(stringBigContainer);
                stringsize += libraryIndex.dim() * sizeof(idx);
                stringsize += bigContainer.length();
                stringsize += idsCompressedTrain.length();
                stringsize += libraryContainer.length();
                if (buf){
                    buf->addString("We have:\n");
                    buf->printf("big dictionary elements: %" DEC " using %" DEC " bytes, and \n", indexBigC.dim(), bigContainer.length());
                    buf->printf("compressed dictionary elements: %" DEC "\n", libraryIndex.dim());
                    buf->printf("space in compressed library: %" DEC "\n", idsCompressedTrain.length());
                    buf->printf("avg per id compressed library: %" DEC "\n", idsCompressedTrain.length() / idIndex.dim());
                    buf->addString("\n");
                    if (stats.count) {
                        // Get internal stats
                        buf->addString("Compressed Library:\n");
                        buf->printf("  Head avg: %" DEC "\n", stats.headLength / stats.count);
                        buf->printf("  Middle avg: %" DEC "\n", stats.middleLength / stats.count);
                        buf->printf("  Tail avg: %" DEC "\n", stats.tailLength / stats.count);
                        for (idx ib = 0; ib < stats.compBin.dim(); ++ib){
                            buf->printf("      %" DEC " - %s\n", stats.compBin[ib], getStringfromCompressedLibrary(ib));
                        }
                    }
                }
                return stringsize;
            }

    };
}
#endif // sBio_ionseqid_hpp
