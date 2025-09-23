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
#ifndef sLib_vioalt_hpp
#define sLib_vioalt_hpp

#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>

#include <ssci/bio/bioseqalign.hpp>
#include <ssci/bio/bioal.hpp>
#include <ssci/math/rand/rand.hpp>

#include <regex.h>

class sVioal : public sBioal
{

    public:
        struct digestParams {
                idx flags, minFragmentLength, maxFragmentLength, countHiveAlPieces, seed;
                bool combineFiles;

                inline bool isKeepAll() {
                    return (flags&sBioseqAlignment::fAlignIsPairedEndMode) || sBioseqAlignment::doSelectMatches(flags);
                }
                digestParams() {
                    sSet(this,0);
                }
        };

    private:
        sVioDB vioDB;
        enum eVioaltTypes {
            eTypeAlignment=1,
            eTypeMatch,
            eTypeSubject,
            eTypeQry,
            eTypeStat,
            eTypeRpt,
            eTypeSAMContent
        };

        struct ALFact {
            int pos;
            short int iFile;
            int sub;
            int rpt;
            idx ofsFile;
            idx idQry;
            idx ofsSam;
        };
        sVec < ALFact > AlFactors, * alFactors;
        idx DigestPairedEnds( sVec<ALFact> &als, sVec<sFil> & fileList, digestParams &params, sDic < sBioal::LenHistogram > * lenHistogram = 0, sVec< idx > * totSubjectCoverage = 0);
        inline bool filterPair(sVioal::ALFact * _i1, sVioal::ALFact * _i2, sBioseqAlignment::Al *_hdr1, sBioseqAlignment::Al *_hdr2, digestParams &params);

        inline  sBioseqAlignment::Al * _getHdrFromAlFact(ALFact * _al, sVec<sFil> & fileList) {return (sBioseqAlignment::Al *)fileList[_al->iFile].ptr( _al->ofsFile );};
        inline void _countForConflicts(ALFact * al1, sVec< sFil> & fileList, idx * pSubHits, idx sublen, digestParams &params);
        inline void _countForConflicts(sBioseqAlignment::Al * hdr, idx * pSubHits, idx sublen, digestParams &params);
        inline void _scaleConflictScores(sVec<idx> & pSubHits, digestParams & params);
        inline idx _getConflictScore( idx * pSubHits, idx l_sub, digestParams &params);

        sBioseqAlignment::Al * getConflictResolvedHDR(sBioseqAlBundleAscIter & it, sVec<idx> &subHits, digestParams &params);

    public:

        sVioal (const char * InputFilename=0, sBioseq * sub=0, sBioseq * qry=0, idx mode=sMex::fReadonly, idx bioMode=0): sBioal()
        {
            myCallbackFunction=0;
            myCallbackParam=0;
            alFactors=&AlFactors;

            init(InputFilename, sub, qry, mode , bioMode);
        }
        virtual ~sVioal ()
        {
        }


        sVioal* init (const char * InputFilename=0, sBioseq * sub=0, sBioseq * qry=0, idx mode=sMex::fReadonly, idx bioMode=0)
        {
            if(InputFilename) vioDB.init(InputFilename,"vioalt",0,0,mode);
            Sub=sub;
            Qry=qry;
            if(!(mode&sMex::fReadonly)){
                setMode(Qry->getmode(), Sub->getmode());
            }
            return this;
        }

        typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
        callbackType myCallbackFunction;
        void * myCallbackParam;


        virtual bool isok(void)
        {
            return vioDB.isok("vioalt")? true : false;
        }

        virtual idx dimAl(void){
            return vioDB.GetRecordCnt(eTypeAlignment);
        }

        virtual sBioseqAlignment::Al * getAl(idx iAlIndex){
            return (sBioseqAlignment::Al * )vioDB.Getbody (eTypeAlignment, iAlIndex+1, 0);
        }
        virtual idx * getMatch (idx iAlIndex) {
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeAlignment, (idx)iAlIndex+1, 1, 0,0 );
            if(!pMatch)return sBioseqAlignment::zeroAlignment;
            return (idx*)vioDB.Getbody (eTypeMatch, *pMatch, 0);
        }

        virtual idx getRpt (idx iAlIndex) {
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeAlignment, (idx)iAlIndex+1, 2, 0,0 );
            if(pMatch) {
                idx* ret = (idx*)vioDB.Getbody (eTypeRpt, *pMatch, 0);
                if(ret)
                    return *ret; 
            }
            return 0;
        }
        virtual idx dimSub(){
            return vioDB.GetRecordCnt(eTypeSubject);
        }

        virtual idx listSubAlIndex(idx idSub, idx * relCount){
            idx iRec=vioDB.GetRecordIndexByBody((const void *)&(idSub), eTypeSubject);
            idx * res=vioDB.GetRelationPtr(eTypeSubject, iRec, 1, (idx*)relCount,0 );
            return res ? *res : 0 ;
        }

        virtual idx dimStat() {
            return vioDB.GetRecordCnt(eTypeStat);
        }
        virtual Stat * getStat(idx iStat=0, idx iSub=0, idx * size=0) {
            Stat * pI=(Stat *)vioDB.Getbody (eTypeStat, iStat+1, size);
            if(size)*size/=sizeof(Stat);
            return pI+iSub;
        }

        virtual const char * getSAMContent(idx iAlIndex, idx * samContentSize = 0) {
            if (vioDB.GetTypeCnt() >= 7 && vioDB.GetRecordCnt(eTypeSAMContent) > 0) {
                idx * pSAMContent = vioDB.GetRelationPtr(eTypeAlignment, iAlIndex+1, 4, 0, 0);
                if (pSAMContent) {
                    return static_cast<char *> (vioDB.Getbody (eTypeSAMContent, *pSAMContent, samContentSize) );
                }
            }
            return NULL;
        }

        static void setMode(sVioDB &db, sBioseq::EBioMode qrymode, sBioseq::EBioMode submode){

            idx resmode = eBioModeShortBoth;
            if(qrymode==sBioseq::eBioModeLong && submode==sBioseq::eBioModeLong)
                resmode = eBioModeLongBoth;
            else if(qrymode==sBioseq::eBioModeLong && submode==sBioseq::eBioModeShort)
                resmode = eBioModeLongQry;
            else if(qrymode==sBioseq::eBioModeShort && submode==sBioseq::eBioModeLong)
                resmode = eBioModeLongSub;

            char userspace = *(db.userSpace8());
            userspace &= ~(0x3);
            userspace |= (char)resmode;
            *(db.userSpace8()) = userspace;
        }

        virtual void setMode(sBioseq::EBioMode qrymode,sBioseq:: EBioMode submode){
            sVioal::setMode(vioDB, qrymode, submode);
        }

        virtual sBioseq::EBioMode getSubMode(void)
        {
            char userspace = *(vioDB.userSpace8());
            userspace &= 0x03;
            return ((userspace == eBioModeLongBoth) || (userspace == eBioModeLongSub)) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;
        }
        virtual sBioseq::EBioMode getQryMode(void)
        {
            char userspace = *(vioDB.userSpace8());
            userspace &= 0x03;
            return ((userspace ==eBioModeLongBoth) || (userspace == eBioModeLongQry)) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;
        }

        virtual bool isPairedEnd(void) {
            idx size;
            getStat(0,0,&size);
            return size == 2*Sub->dim()+1;
        }



        static idx DigestInit(sVioDB * db , const char * outputfilename);
        static idx DigestFirstStage(sVioDB * db, sBioseqAlignment::Al * hdr, sBioseqAlignment::Al * hdre, sBioseq * qry, sBioal::Stat *  stat, bool selectedOnly  , bool doQueries, idx biomode, idx oneRepeatForall=0);
        static idx DigestFragmentStage(ALFact * hdr, ALFact * hdre, sBioal::Stat *  stat, sBioseq * qry, idx biomode, idx oneRepeatForall=0);
        static idx DigestSecondStage(sVioDB * db, sBioseq * qry, idx * pAl, sBioseqAlignment::Al * hdr,sBioseqAlignment::Al * hdre , bool selectedOnly  , bool doQueries, idx oneRepeatForall=0);



        idx Digest(const char* outputfilename, bool combineFile, sBioseqAlignment::Al * rawAlignment, idx size , Stat * statFailed, bool selectedOnly=false , bool doQueries=false, idx biomode=0);
        idx concatAlignmentFiles( const char * dstfile, const char * filenames , bool ifGlueTheFile=false, bool removeOriginals=false, callbackType * func = 0 ){
            return vioDB.concatFiles( dstfile, filenames ,"vioalt", ifGlueTheFile,removeOriginals);
        }

    public:

        void SortOnSubject(){
            ParamsAlignmentSorter Param;
            Param.flags=alSortByPosStart;
            Param.bioal=this;
            vioDB.viodDBSorter( eTypeSubject, 1, BioseqAlignmentComparator , &Param );
        }
        void SortAll(idx * ind,ParamsAlignmentSorter &param){
            vioDB.viodDBSorter( eTypeAlignment, ind, BioseqAlignmentComparator, &param );
        }


        static idx __sort_totalAlignmentSorter_onSubject(void * param, void * arr , idx i1, idx i2);
        static idx __sort_totalAlignmentSorter_onQuery(void * param, void * arr , idx i1, idx i2);
        static idx __sort_totalAlignmentSorter_onPosition(void * param, void * arr , idx i1, idx i2);
        idx DigestCombineAlignmentsRaw(const char* outputfilename, const char * filenames00, digestParams &params, sDic < sBioal::LenHistogram > * lenHistogram=0, sVec < idx > * totSubjectCoverage=0, idx sortFlags = 0, const char * samFilenames00 = 0) ;

}; 

#endif

