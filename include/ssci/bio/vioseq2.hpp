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
#ifndef sBio_vioseq2_hpp
#define sBio_vioseq2_hpp

#include <slib/core/str.hpp>
#include <slib/utils/sVioDB.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqtree.hpp>

namespace slib {

    class sVioseq2: public sBioseq
    {

            sVioDB vioDB;
            EBioMode mode;
            idx length;
            bool quaBit;


        protected:
            bool m_isSingleFile;
            udx m_recLimit;

        public:
            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);
            callbackType m_callback;
            void * m_callbackParam;

            enum eParseFlags
            {
                eParseLazy         = 0x00000001,
                eParseNoId         = 0x00000002,
                eParseQuaBit       = 0x00000004,
                eParsedQualities   = 0x00000010,
                eParseMultiVioDB   = 0x00000020,
                eFossilizeReverse  = 0x00000100,
                eContainsPrimer    = 0x00001000,
                eLowComplexity     = 0x00002000,
                eCreateVioseqlist  = 0x00004000,
                eTreatAsFastA      = 0x00010000,
                eTreatAsFastQ      = 0x00020000,
                eTreatAsAnnotation = 0x00040000,
                eTreatAsSAM        = 0x00080000,
                eTreatAsMA         = 0x00100000
            };

            enum eRecTypes
            {
                eRecID_TYPE = 1,
                eRecREC_TYPE,
                eRecSEQ_TYPE,
                eRecQUA_TYPE
            };

            struct errParser
            {
                    idx errN;
                    const char * msg;
            };

            static errParser listErrors[];

            struct Rec
            {
                    idx lenSeq;
                    idx countSeq;
                    idx ofsSeq;
            };

            struct Infopart
            {
                    idx origID;
                    idx partID;
                    idx numID;
            };

            sVioseq2(const char * filename = 0)
                : vioDB(), mode(eBioModeShort), length(0), quaBit(false), m_isSingleFile(false), m_recLimit(0), m_callback(0), m_callbackParam(0)
            {
                init(filename);
            }
            ~sVioseq2()
            {
                destroy();
            }

            void setFlagsDB (sVioDB &db, idx flags)
            {
                idx *userspace = (idx*)db.userSpace8();
                (*userspace) = (idx)flags;
            }

            idx getFlagsDB ()
            {
                return *(idx *)vioDB.userSpace8();
            }

            idx getFlags()
            {
                return flags;
            }
            virtual bool getQuaBit (idx inum)
            {
                return quaBit;
            }

            sVioseq2 * init(const char * filename = 0)
            {
                mode = eBioModeShort;
                m_isSingleFile = false;
                m_recLimit = 0;
                if( filename )
                {
                    vioDB.init(filename, 0, 0, 0, sMex::fReadonly);
                    quaBit = ( (*(idx *) vioDB.userSpace8()) & eParseQuaBit) ? true : false;
                }
                return this;
            }

            virtual void destroy(bool justMemoryFree=false)
            {
                if (justMemoryFree){
                    vioDB.deleteAllJobs(justMemoryFree);
                }
            }

        public:
            idx parseSequenceFile(const char * outfile, const char * flnm, idx flags, idx maxChunkSize = ((idx) (8) * 1024 * 1024 * 1024), const char * primers00 = 0, idx complexityWindow = 0, real complexityEntropy = 0);
            idx parseSequenceFile(const char * outfile, const char * flnm, idx flags = eParseLazy, const char * primers00 = 0, idx complexityWindow = 0, real complexityEntropy = 0, idx sizecomb = 0, idx comb = 0, sVec<Infopart> * iPart = 0);
            idx parseFasta(sVioDB & db, sFil & baseFile, const char * inp, idx len, idx flags = 0, const char * primers00 = 0, idx complexityWindow = 0, real complexityEntropy = 0, idx sizecomb = 0, idx comb = 0, sVec<Infopart> * iPart = 0);
            idx parseFastQ(sVioDB &db, sFil & baseFile, const char * inp, idx len, idx flags = 0, const char * primers00 = 0, idx complexityWindow = 0, real complexityEntropy = 0, idx sizecomb = 0, idx comb = 0, sVec<Infopart> * iPart = 0);
            idx ParseGb(const char * fileContent, idx filesize, sVioDB &db , sFil & baseFile, sVec <Infopart> * iPart = 0);
            idx ParseSam(const char * fileContent, idx filesize, sVioDB &db, sFil & baseFile, sVec<idx> * alignOut, sDic<idx> * rgm, bool alignOnly = false, sVec<Infopart> * iPart = 0,sDic < idx > * sub=0,sDic < idx > * qry=0, idx minMatchLength = 0, bool isMinMatchPercentage = false, idx maxMissQueryPercent = 0, bool useRowInformationtoExtractQry = false,  sFil * samHeader = 0, sFil *samData = 0, sFil *samFooter = 0);
            static idx convertSAMintoAlignmentMap(const char * fileContent, idx filesize, sVec<idx> * alignOut, sDic<idx> * rgm, idx minMatchLength, bool isMinMatchPercentage, idx maxMissQueryPercent, sDic<idx> *sub = 0, sDic<idx> *qry = 0, bool useRowInformationtoExtractQry = false,  sFil * samHeader = 0, sFil *samcontent = 0, sFil *samFooter = 0);
            idx parseQualities(sVioDB & db, sFil & baseFile, const char * qual, idx qualalen, idx flags);
            static udx getPartCount(const udx fileSize, udx maxChunkSize);
            static udx getPrefixLength(const udx partCount);
            static idx fixAddRelation(sVioDB *db, sVec<sVec<Infopart> > * partList, idx *countRes, const char * partListfiles00=0, callbackType callback = 0, void * callbackParam = 0);
            void updateQualities(char *origQua, char *currQua, idx numSeq, idx lenSeq, idx rptcount = 1);
            void updateBitQualities (char *origQua, char *currQua, idx lenSeq);

            char * scanNumUntilEOL(const char * ptr, idx * pVal, const char * lastpos);
            char * skipBlanks(const char * ptr, const char * lastpos);
            char * skipUntilEOL(const char * ptr, const char * lastpos);
            char * scanUntilLetterOrSpace(const char * ptr, idx * pVal, const char * lastpos);
            char * scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos, bool allowSpaces = true);
            void skipMismatchesAtTheLeftEnd(const char * ptr, const char * lastpos, idx * skipPositions=0);
            void skipMismatchesAtTheRightEnd(const char * ptr, const char * lastpos, idx * skipPositions=0);
            char * cigar_parser(const char * ptr, const char * lastpos, sVec<idx> * alignOut, idx * lenalign, idx * qryStart, idx PositionsToSkip=0, idx *matches = 0);
            static const char * extractGBSequence(const char * srcFile, sStr & sequenceOut);
            static idx sequential_parse_callback(void * param, idx countDone, idx percentDone, idx percentMax);
        public:

            void setmode(EBioMode mod)
            {
                mode = mod;
            }

            EBioMode getmode()
            {
                return mode;
            }

            void setLimit(udx limit)
            {
                m_recLimit = limit;
            }
            udx getLimit(void)
            {
                return m_recLimit;
            }

            void setSingleFile(bool isSingleFile)
            {
                m_isSingleFile = isSingleFile;
            }

            virtual idx dim(void)
            {
                if( mode == eBioModeLong ) {
                    length = vioDB.GetRecordCnt(eRecID_TYPE);
                } else {
                    length = vioDB.GetRecordCnt(eRecREC_TYPE);
                }
                return length;
            }


            virtual idx len(idx num, idx iread = 0)
            {
                idx sizetype, relcnt, reltype;
                if( mode == eBioModeLong ) {
                    idx irel = *vioDB.GetRelationPtr(eRecID_TYPE, num + 1, 1, &relcnt, &reltype);
                    if( irel == 0 ) {
                        return 0;
                    }
                    Rec * rec = (sVioseq2::Rec *) vioDB.Getbody(reltype, irel, &sizetype);
                    return rec ? rec->lenSeq : 0;
                } else {
                    Rec * rec = (sVioseq2::Rec *) vioDB.Getbody(eRecREC_TYPE, num + 1, &sizetype);
                    return rec ? rec->lenSeq : 0;
                }
            }

            virtual idx rpt(idx num, idx iread = 0)
            {
                if( mode == eBioModeLong ) {
                    return 1;
                } else {
                    idx sizetype;
                    Rec * rec = (sVioseq2::Rec *) vioDB.Getbody(eRecREC_TYPE, num + 1, &sizetype);
                    return rec ? (rec->countSeq & 0xFFFFFFFF) : 0;
                }
            }

            virtual idx sim(idx num, idx iread = 0)
            {
                if( mode == eBioModeLong ) {
                    return 0;
                } else {
                    idx sizetype;
                    Rec * rec = (sVioseq2::Rec *) vioDB.Getbody(eRecREC_TYPE, num + 1, &sizetype);
                    return rec ? ((rec->countSeq >> 32) & 0xFFFFFFFF) : 0;
                }
            }

            virtual const char * seq(idx num, idx iread = 0)
            {
                idx sizetype, irel, relcnt, reltype;
                const char *seq;
                if( mode == eBioModeLong ) {
                    irel = *vioDB.GetRelationPtr(eRecID_TYPE, num + 1, 1, &relcnt, &reltype);
                    if( irel == 0 ) {
                        return 0;
                    }
                    irel = *vioDB.GetRelationPtr(reltype, irel, 2, &relcnt, &reltype);
                    seq = (const char *) vioDB.Getbody(reltype, irel, &sizetype);
                    return seq;
                } else {
                    irel = *vioDB.GetRelationPtr(eRecREC_TYPE, num + 1, 2, &relcnt, &reltype);
                    if( irel == 0 ) {
                        return 0;
                    }
                    seq = (const char *) vioDB.Getbody(reltype, irel, &sizetype);
                    return seq;
                }
            }

            virtual const char * id(idx num, idx iread = 0)
            {
                idx sizetype, irel, relcnt, reltype;
                const char * id;
                if( mode  == eBioModeLong ) {
                    id = (const char *) vioDB.Getbody(eRecID_TYPE, num + 1, &sizetype);
                    return id;
                } else {
                    irel = *vioDB.GetRelationPtr(eRecREC_TYPE, num + 1, 1, &relcnt, &reltype);
                    if( irel <= 0 ) {
                        return 0;
                    }
                    id = (const char *) vioDB.Getbody(reltype, irel, &sizetype);
                    return id;
                }
            }

            virtual const char * qua(idx num, idx iread = 0)
            {
                idx sizetype, irel, relcnt, reltype;
                const char * qua;
                if( mode == eBioModeLong ) {
                    idx * prel = vioDB.GetRelationPtr(eRecID_TYPE, num + 1, 1, &relcnt, &reltype);
                    if( !prel || !relcnt ) {
                        return 0;
                    }
                    irel = *prel;
                    idx *ire2 = vioDB.GetRelationPtr(reltype, irel, 3, &relcnt, &reltype);
                    if( !ire2 || !relcnt ) {
                        return 0;
                    }
                    qua = (const char *) vioDB.Getbody(reltype, *ire2, &sizetype);
                    return qua;
                } else {
                    idx * prel = vioDB.GetRelationPtr(eRecREC_TYPE, num + 1, 3, &relcnt, &reltype);
                    if( !prel || !relcnt ) {
                        return 0;
                    }
                    irel = *prel;
                    qua = (const char *) vioDB.Getbody(reltype, irel, &sizetype);
                    return qua;
                }
            }

            virtual idx long2short(idx num, idx iread = 0)
            {
                idx irel, relcnt, reltype;
                irel = *vioDB.GetRelationPtr(eRecID_TYPE, num + 1, 1, &relcnt, &reltype);
                if (irel == 0) { return 0;}
                if (irel <= 0){
                    return (irel*-1)-1;
                }
                return irel - 1;
            }
            virtual idx short2long(idx num, idx iread = 0)
            {
                idx irel, relcnt, reltype;
                irel = *vioDB.GetRelationPtr(eRecREC_TYPE, num + 1, 1, &relcnt, &reltype);
                if (irel == 0) { return 0;}
                if (irel <= 0){
                    return (irel*-1)-1;
                }
                return irel - 1;
            }
            virtual idx getlongCount()
            {
                return vioDB.GetRecordCnt(eRecID_TYPE);
            }
            virtual idx getshortCount()
            {
                return vioDB.GetRecordCnt(eRecREC_TYPE);
            }

        private:
            BioseqTree *tree;
            idx uniqueCount, removeCount;
            sVec < idx > ids, ods, idsN;
            sVec < Rec > vofs;
            idx idNum, iNN;
            idx sizecomb, isParallelversion, comb;
            idx flags;
            idx complexityWindow, complexityEntropy;
            bool sVioDBQualities;

            void initVariables (sFil &baseFile, idx ofs = 0, idx sizeComb = 0, idx c = 0, idx flag = 0, idx compWindow = 0, idx compEntropy = 0){
                tree = new BioseqTree(&baseFile, ofs);
                ids.mex()->flags |= sMex::fSetZero;
                ids.cut(0);
                ods.mex()->flags |= sMex::fSetZero;
                ods.cut(0);
                idsN.mex()->flags |= sMex::fSetZero;
                idsN.cut(0);
                vofs.mex()->flags |= sMex::fBlockDoubling;
                vofs.cut(0);
                uniqueCount = 0;
                removeCount = 0;
                idNum = 0;
                iNN = 0;
                sizecomb = sizeComb;
                isParallelversion = (sizecomb != 0) ? 1 : 0;
                comb = c;
                flags = flag;
                complexityWindow = compWindow;
                complexityEntropy = compEntropy;
                sVioDBQualities = true;
            }

            void freeVariables (){
                delete tree;
                ids.destroy();
                ods.destroy();
                idsN.destroy();
                vofs.destroy();
            }
            bool addNewSequence(sFil & baseFile, sVioDB &db, const char *id, idx idlen, const char *seq, const char *qua, idx seqlen);
            idx writeSortVioDB(sFil & baseFile, sVioDB &db, sVec <Infopart> * iPart, bool isQuaBit = false);
            idx extractRptCount (const char *id, idx *idlen);
            inline void addRecID(sVioDB & db, const void * id, idx id_len)
            {
                static sMex buf;
                if( (id_len % sizeof(idx)) == 0) {
                    buf.cut(0);
                    buf.add(id, id_len);
                    buf.add("", 1);
                    id = buf.ptr();
                    id_len = buf.pos();
                }
                db.AddRecord(eRecID_TYPE, id, id_len);
                db.AddRecordRelationshipCounter(eRecID_TYPE, 0, 1);
            }
    };

}

#endif 