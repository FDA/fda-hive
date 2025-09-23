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
#include <slib/std.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>

#include <slib/utils.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <violin/hiveproc.hpp>
#include <violin/alignparse.hpp>


class DnaHexagon : public sHiveProc
{
    public:
        DnaHexagon(const char * defline00,const char * srv) : sHiveProc(defline00,srv)
        {
        }
        virtual idx OnExecute(idx);

        idx getTheFlags( void )
        {
            idx reverseEngine=formIValue("reverseEngine",0);
            idx keepAllMatches=formIValue("keepAllMatches",1);
            idx flagSet=sBioseqAlignment::fAlignForward;
            if( formIValue("isglobal") ) flagSet|=sBioseqAlignment::fAlignGlobal;
            if( formIValue("isbackward", 1) ) {
                flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
                if( formIValue("isbackward", 1) == 2 ) flagSet &= ~sBioseqAlignment::fAlignForward;
            }
            if( formIValue("isoptimize", 1) ) flagSet|=sBioseqAlignment::fAlignOptimizeDiagonal;
            if( formIValue("isextendtails") ) flagSet|=sBioseqAlignment::fAlignMaxExtendTail;
            if( formIValue("isCircular") ) flagSet|=sBioseqAlignment::fAlignCircular;
            if( formIValue("doubleHash") ) flagSet|=sBioseqAlignment::fAlignOptimizeDoubleHash;
            if( formIValue("alignmentEngine",1) ==0 )flagSet|=sBioseqAlignment::fAlignIdentityOnly;

            idx resolveConflicts = formIValue("keepResolveConflicts", false);
            if( resolveConflicts ) {
                if(resolveConflicts == 1) flagSet |= sBioseqAlignment::fAlignKeepResolveMarkovnikov;
                if(resolveConflicts == 2) flagSet |= sBioseqAlignment::fAlignKeepResolveBalanced;

                idx resolveConflictsScore = formIValue("keepResolveConflicts", false);
                if( resolveConflictsScore == 1 ) flagSet |= sBioseqAlignment::fAlignKeepResolvedHits;
                if( resolveConflictsScore == 2 ) flagSet |= sBioseqAlignment::fAlignKeepResolvedSymmetry;
                if( formBoolValue("resolveConfictsUnique", false) ) flagSet |= sBioseqAlignment::fAlignKeepResolveUnique;
            }


            if( formBoolValue("keepPairedOnly", false) ) flagSet |= sBioseqAlignment::fAlignKeepPairedOnly;
            if( formBoolValue("keepPairOnSameSubject", false) ) flagSet |= sBioseqAlignment::fAlignKeepPairOnSameSubject;
            if( formBoolValue("keepPairOnOppositeStrand", false) ) flagSet |= sBioseqAlignment::fAlignKeepPairDirectionality;
            if( formIValue("fragmentLengthMin", 0) || formIValue("fragmentLengthMax", 0) ||
                (flagSet&(sBioseqAlignment::fAlignKeepPairDirectionality|sBioseqAlignment::fAlignKeepPairOnSameSubject|sBioseqAlignment::fAlignKeepPairedOnly) ) ) {
                flagSet |= sBioseqAlignment::fAlignIsPairedEndMode;
            }

            if(keepAllMatches==0)flagSet|=sBioseqAlignment::fAlignKeepFirstMatch;
            if(keepAllMatches==1)flagSet|=sBioseqAlignment::fAlignKeepBestFirstMatch;
            if(keepAllMatches==3)flagSet|=sBioseqAlignment::fAlignKeepAllBestMatches;
            if(keepAllMatches==4)flagSet|=sBioseqAlignment::fAlignKeepRandomBestMatch;
            if(keepAllMatches==5)flagSet|=sBioseqAlignment::fAlignKeepUniqueMatch;
            if(reverseEngine)flagSet|=sBioseqAlignment::fAlignReverseEngine;

            idx searchRepeatsAndTrans=formIValue("searchRepeatsAndTrans",0);
            if(searchRepeatsAndTrans==1)flagSet|=sBioseqAlignment::fAlignSearchRepeats;
            if(searchRepeatsAndTrans==2)flagSet|=sBioseqAlignment::fAlignSearchTranspositions|sBioseqAlignment::fAlignSearchRepeats;
            return flagSet;
        }

        bool getChunkRanges(idx &subStart, idx &subEnd, idx &qryStart, idx &qryEnd);
};


#define ROLL_ANNOTATIONS_START \
    bool doContinue=true; \
    sStr subid;sString::copyUntil(&subid,sub->id(curSub),0,sString_symbolsBlank); \
    for ( ;doContinue && curSubAnnotFile<annotList.dim() ; ++curSubAnnotFile, curSubAnnotRange=0, curSubAnnotRangeSection=0) { \
        idx rangeCnt, * rangeList=annotList.ptr(curSubAnnotFile)->getRangesForGivenIDAndIDType(subid, "locus", &rangeCnt); \
        if(!rangeList)continue; \
        for(; doContinue && curSubAnnotRange<rangeCnt; ++curSubAnnotRange, curSubAnnotRangeSection=0 ){ \
            if(referenceAnnotTypes){ \
                const char * annotType=annotList.ptr(curSubAnnotFile)->getDataNameByRangeIndex(rangeList[curSubAnnotRange]); \
                if( sString::compareChoice(annotType,referenceAnnotTypes,0,false,0,true)==-1 ) \
                    continue; \
            }\
            idx rangeSubsectionsCount; \
            sVioAnnot::startEnd * sen=annotList.ptr(curSubAnnotFile)->getRangesByIndex(rangeList[curSubAnnotRange], &rangeSubsectionsCount); \
            for ( ; doContinue && curSubAnnotRangeSection<rangeSubsectionsCount; ++curSubAnnotRangeSection ){
#define ROLL_ANNOTATIONS_END \
            } \
        } \
    }


extern idx cntHashLookup, cntBloomLookup, cntAlignmentSW, cntExtension, cntSuccessSW;
#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD)
    #ifdef printMethodComparisonHits
extern sStr cmpMethodsOutput;
    #endif
extern idx cntAlignmentsNEWonly, cntAlignmentsOLDonly, cntAlignmentsBOTH, cntAlignmentsNONE, cntFilteredNEWonly, cntFilteredOLDonly, cntFilteredBOTH, cntFilteredNONE;

extern bool cmpMethodsOutputISDIFF;
#endif


idx DnaHexagon::OnExecute(idx req)
{
    sStr errS;
    const int progressIntervalBases = 1000000;

    idx maxNumberQuery=formIValue("maxNumberQuery");
    idx seed=formIValue("seed",11);

    idx isDoubleHash = formIValue("doubleHash") ? true : false;
    if(seed<0) {seed=-seed; isDoubleHash=true;}
    idx reverseEngine=formIValue("reverseEngine",0);


    idx maxHashBin=formIValue("maxHashBin",50); if(!maxHashBin)maxHashBin=50;
    idx complexityWindow=formIValue("complexityWindow",0);if(complexityWindow<0)complexityWindow=seed;
    real complexityEntropy=formRValue("complexityEntropy",0);
    idx complexityRefWindow=formIValue("complexityRefWindow",0);if(complexityRefWindow<0)complexityRefWindow=seed;
    real complexityRefEntropy=formRValue("complexityRefEntropy",0);
    idx acceptNNNQuaTrheshold=formRValue("acceptNNNQuaTrheshold",1);
    real maximumPercentLowQualityAllowed=formRValue("maximumPercentLowQualityAllowed",0);
    idx doubleStagePerfect=formIValue("doubleStagePerfect",1);
    idx quabit=0;

    idx maxSubjectBasesToCompile=formIValue("maxSubjectBasesToCompile",0); if(!maxSubjectBasesToCompile)maxSubjectBasesToCompile=1024*1024*128;

    idx hashExpectation=maxSubjectBasesToCompile/(((idx)1)<<(2*seed));
    if(!hashExpectation)hashExpectation=1;
    hashExpectation=(maxHashBin<0) ? hashExpectation*(-maxHashBin) : maxHashBin ;


    idx flagSet=getTheFlags( );
    if( isDoubleHash ) flagSet|=sBioseqAlignment::fAlignOptimizeDoubleHash;
    sVec <sVioAnnot> annotList;



    const char * referenceAnnotTypes=formValue("referenceAnnotTypes");
    sStr refA;
    if(referenceAnnotTypes) {
        sString::searchAndReplaceSymbols(&refA,referenceAnnotTypes,0,";\n",0,0,true,true,true,true);
        if(sString::cnt00(refA))
            referenceAnnotTypes=refA.ptr();
    }
    const char * referenceAnnot=formValue("referenceAnnot");
    if(referenceAnnot){
        sVec <sHiveId> annotIds;
        sHiveId::parseRangeSet(annotIds, referenceAnnot);
        sHiveannot::InitAnnotList(user,annotList,&annotIds);
    }

    idx qStart,qEnd,sStart,sEnd;
    idx cntFound=0;

    sVec < idx > alignmentMap;

    sBioseq * sub, * qry;


    sStr subject;
    QPSvcDnaHexagon::getSubject00(objs[0],subject);
    sHiveseq Sub(user, subject, sBioseq::eBioModeShort, false, false, &errS);
    if( Sub.dim() == 0 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "dna-hexagon: Reference '%s' sequences are missing or corrupted%s%s", subject.length() ? subject.ptr() : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    errS.cut0cut();


    sStr query;
    QPSvcDnaHexagon::getQuery00(objs[0],query);
    sHiveseq Qry(sQPride::user, query, sBioseq::eBioModeShort, false, false, &errS);
    if(Qry.dim()==0) {
        reqSetInfo(req, eQPInfoLevel_Error, "dna-hexagon: Query '%s' sequences are missing or corrupted%s%s", query.length() ? query.ptr() : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    errS.cut0cut();
    sub=&Sub;
    qry=&Qry;

    bool doBreak = false;

    {
        sBioseqAlignment masterAl;
        masterAl.MatSW.addM(4*1024*1024);
        masterAl.MatBR.addM(4*1024*1024);

        masterAl.costMatchFirst=formIValue("costMatchFirst",5 ) ;
        masterAl.costMatchSecond=formIValue("costMatchSecond",5);
        masterAl.costMatch=formIValue("costMatch",5);
        masterAl.costMismatch=formIValue("costMismatch",-4);
        masterAl.costMismatchNext=formIValue("costMismatchNext",-6);
        masterAl.costGapOpen=formIValue("costGapOpen",-12);
        masterAl.costGapNext=formIValue("costGapNext",-4);
        masterAl.computeDiagonalWidth=formIValue("computeDiagonalWidth",0);
        masterAl.maxMissQueryPercent=formRValue("maxMissQueryPercent",15);
        masterAl.minMatchLen=formIValue("minMatchLen",75);
        masterAl.isMinMatchPercentage=(bool)formIValue("minMatchUnit",0);
        masterAl.considerGoodSubalignments=formIValue("considerGoodSubalignments",1);

        masterAl.scoreFilter=formIValue("scoreFilter",0);
        masterAl.trimLowScoreEnds=formIValue("trimLowScoreEndsWindow",0);
        if(masterAl.trimLowScoreEnds<0)
            masterAl.trimLowScoreEnds = seed;
        masterAl.trimLowScoreEndsMaxMM=formIValue("trimLowScoreEndsMaxMismatches",0);
        if(masterAl.trimLowScoreEndsMaxMM<0)
            masterAl.trimLowScoreEndsMaxMM = masterAl.maxMissQueryPercent;
        masterAl.trimLowScoreEndsMaxMM = 100 - masterAl.trimLowScoreEndsMaxMM;
        masterAl.allowShorterEnds=formIValue("allowShorterEnds",0);
        if(!masterAl.allowShorterEnds)masterAl.allowShorterEnds=masterAl.minMatchLen;
        masterAl.maxExtensionGaps=formIValue("maxExtensionGaps",0);
        masterAl.hashStp=formIValue("hashStp",reverseEngine ? seed : 1 );
        masterAl.bioHash.hashStp=formIValue("hashCompileStp",1 );
        masterAl.looseExtenderMismatchesPercent=formIValue("looseExtenderMismatchesPercent",25);
        masterAl.looseExtenderMinimumLengthPercent=formIValue("looseExtenderMinimumLengthPercent",66);

        masterAl.maxHitsPerRead=formIValue("maxHitsPerRead",50);
        masterAl.maxSeedSearchQueryPos=formIValue("maxSeedSearchQueryPos",0);
        masterAl.ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment=formIValue("selfSubjectPosJumpInNonPerfectAlignment",1);
        masterAl.ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment=formIValue("selfQueryPosJumpInNonPerfectAlignment",1);


        masterAl.bioHash.init(seed,4ll);
        masterAl.hashHits.init(seed);
        idx isBloom = (seed <= 16) ? sBioseqHash::eCompileBloom  : 0 ;
        if(!isBloom)masterAl.bioHash.collisionReducer=4;


        sub = reverseEngine ? &Qry : &Sub;
        qry = reverseEngine ? &Sub : &Qry;

        idx slice = (qry->dim() - 1) / reqSliceCnt + 1;

        qStart = reverseEngine ? 0 : slice * reqSliceId;
        qEnd = reverseEngine ? Sub.dim() : qStart + slice;
        if( qEnd>qry->dim() || qEnd <= 0){
            qEnd = qry->dim();
        }
        sStart = reverseEngine ? slice * reqSliceId : 0;
        sEnd = reverseEngine ? sStart + slice : Sub.dim();
        if( sEnd > sub->dim() || sEnd <= 0){
            sEnd = sub->dim();
        }


        sVec < char > QhitBM(sMex::fSetZero),ShitBM(sMex::fSetZero);
        masterAl.QHitBitmask=QhitBM.addM((qEnd-qStart)/8+1);
        masterAl.SHitBitmask=ShitBM.addM((sEnd-sStart)/8+1);



        sVec < idx > randomQueries;
        if(maxNumberQuery>0){
            randomQueries.add(maxNumberQuery);
            for( idx im=0; im<maxNumberQuery; ++im){
                randomQueries[im]=qStart+1.*(qEnd-qStart-1)*rand()*rand()/RAND_MAX/RAND_MAX;
            }

        }else {
            maxNumberQuery=-maxNumberQuery;
        }







        idx curSub;
        idx curSubAnnotFile=0, curSubAnnotRange=0, curSubAnnotPos=0, curSubAnnotRangeSection=0;

        idx subTotalLen=0;
        for( curSub=sStart ; curSub<sEnd ; ++curSub , curSubAnnotFile=0) {
            if(annotList.dim()){
                ROLL_ANNOTATIONS_START
                    subTotalLen+=sen[curSubAnnotRangeSection].end-sen[curSubAnnotRangeSection].start+1;
                ROLL_ANNOTATIONS_END
            }else {
                subTotalLen+=sub->len(curSub);
            }
        }
        curSubAnnotFile=0, curSubAnnotRange=0, curSubAnnotPos=0, curSubAnnotRangeSection=0;
        idx stDim=0;
        idx curSubPos=0;
        curSub=0;

        idx curWork=0;
        idx totWork=(qEnd-qStart)*subTotalLen;
        idx cntSimple=0,cntComplex=0,cntLowQua=0,cntFast=0,cntVeryFast=0,cntSlow=0,cntNotFound=0;
        idx subjectIn=0;
        idx chunkSub=0;

        for( idx subTotalDone=0; !doBreak && subTotalDone<subTotalLen ; subTotalDone+=subjectIn) {
            subjectIn=0;
            idx curSubSave=curSub, curSubPosSave=curSubPos;
            idx curSubAnnotFileSave=curSubAnnotFile, curSubAnnotRangeSave=curSubAnnotRange, curSubAnnotPosSave=curSubAnnotPos, curSubAnnotRangeSectionSave=curSubAnnotRangeSection;


    PERF_START("KMERE_SEEDING");

            for ( ; curSub<sEnd  ; ++curSub, curSubPos=0, curSubAnnotFile=0) {

                if(annotList.dim()){
                    ROLL_ANNOTATIONS_START;
                        idx howMuchToPutIn=sen[curSubAnnotRangeSection].end-sen[curSubAnnotRangeSection].start+1;
                        if(subjectIn+howMuchToPutIn>maxSubjectBasesToCompile){
                            howMuchToPutIn=maxSubjectBasesToCompile-subjectIn;
                        }
                        subjectIn+=howMuchToPutIn;
                        if( subjectIn>=maxSubjectBasesToCompile) {
                            doContinue=false;
                        }
                    ROLL_ANNOTATIONS_END;

                }
                else {
                    idx howMuchToPutIn=sub->len(curSub)-curSubPos;
                    curSubPos+=howMuchToPutIn;
                    if(subjectIn+howMuchToPutIn>maxSubjectBasesToCompile){
                        howMuchToPutIn=maxSubjectBasesToCompile-subjectIn;
                    }
                    subjectIn+=howMuchToPutIn;
                    if( subjectIn>=maxSubjectBasesToCompile)
                        break;
                }
            }
            ++stDim;

            masterAl.bioHash.reset();
            ++chunkSub;


            curSub=curSubSave;
            curSubPos=curSubPosSave;
            curSubAnnotFile=curSubAnnotFileSave;
            curSubAnnotRange=curSubAnnotRangeSave;
            curSubAnnotPos=curSubAnnotPosSave;
            curSubAnnotRangeSection=curSubAnnotRangeSectionSave;

            subjectIn=0;

            logOut(eQPLogType_Debug,"Compiling %" DEC " seed dictionary # %" DEC " to accumulate %" DEC " MegaBases \n",seed, chunkSub, maxSubjectBasesToCompile/1024/1024);

            for ( ; curSub<sEnd ; ++curSub, curSubPos=0, curSubAnnotFile=0) {

                if(annotList.dim()){
                    ROLL_ANNOTATIONS_START
                        idx howMuchToPutIn=sen[curSubAnnotRangeSection].end-sen[curSubAnnotRangeSection].start+1;
                        if(subjectIn+howMuchToPutIn>maxSubjectBasesToCompile){
                            howMuchToPutIn=maxSubjectBasesToCompile-subjectIn;
                        }
                        subjectIn+=howMuchToPutIn;
                        if( subjectIn>=maxSubjectBasesToCompile) {
                            doContinue=false;
                        }

                        idx posToStartHash=sen[curSubAnnotRangeSection].start;
                        if(posToStartHash<0)posToStartHash=0;
                        idx cntCompiled=masterAl.bioHash.compile(curSub, sub->seq(curSub), sub->len(curSub), sBioseqHash::eCompileDic|isBloom, hashExpectation,  posToStartHash, howMuchToPutIn , complexityRefWindow, complexityRefEntropy, acceptNNNQuaTrheshold ? sub->qua(curSub) :0 , true, acceptNNNQuaTrheshold );
                        logOut(eQPLogType_Debug,"\t reference %" DEC " [%" DEC "- %" DEC "] ... %" DEC " out of %" DEC " in! %s\n",curSub, posToStartHash, posToStartHash+howMuchToPutIn , cntCompiled , howMuchToPutIn ,sub->id(curSub));
                    ROLL_ANNOTATIONS_END
                }
                else {
                    idx howMuchToPutIn=sub->len(curSub)-curSubPos;
                    if(subjectIn+howMuchToPutIn>maxSubjectBasesToCompile){
                        howMuchToPutIn=maxSubjectBasesToCompile-subjectIn;
                    }

                    subjectIn+=howMuchToPutIn;
                    idx posToStartHash=curSubPos-seed;
                    if(posToStartHash<0)posToStartHash=0;

                    idx cntCompiled=masterAl.bioHash.compile(curSub, sub->seq(curSub), sub->len(curSub), sBioseqHash::eCompileDic|isBloom, hashExpectation,  posToStartHash, howMuchToPutIn , complexityRefWindow, complexityRefEntropy, acceptNNNQuaTrheshold ? sub->qua(curSub) :0 , true, acceptNNNQuaTrheshold );
                    logOut(eQPLogType_Debug,"\t reference %" DEC " [%" DEC "- %" DEC "] ... %" DEC " out of %" DEC " in! %s\n",curSub, posToStartHash, posToStartHash+howMuchToPutIn , cntCompiled , howMuchToPutIn ,sub->id(curSub));

                    curSubPos+=howMuchToPutIn;

                    if( subjectIn>=maxSubjectBasesToCompile)
                        break;
                }
            }

    PERF_END();
            logOut(eQPLogType_Debug,"Aligning %" DEC " query sequences to %" DEC " subject Mega-Bases in the range %" DEC "-%" DEC " out of %" DEC " Mega-Bases\n"  , qEnd-qStart, subjectIn/1024/1024 , (subTotalDone)/1024/1024, (subTotalDone+subjectIn)/1024/1024, subTotalLen/1024/1024);

            idx qriesHit=0;
            real minMatchLenPerc = formRValue("minMatchLen",75)/100;
            idx totalQryLen = 0, progressMod = 0;
            for(idx iqry=qStart; !doBreak && iqry<qEnd ; ++iqry ) {
                idx iq=(randomQueries.dim ()) ? randomQueries[iqry-qStart] : iqry;
                idx iqx=iq-qStart;


                if( ((flagSet & sBioseqAlignment::fAlignKeepFirstMatch) && (flagSet & sBioseqAlignment::fAlignReverseEngine) == 0) && masterAl.QHitBitmask && (masterAl.QHitBitmask[iqx / 8] & (((idx) 1) << (iqx % 8))) ) {
                    ++qriesHit;
                    continue;
                }

                idx found=0;


                idx qlen=qry->len(iq);
                const char * seq=qry->seq(iq);
                if(masterAl.isMinMatchPercentage) {
                    masterAl.minMatchLen = minMatchLenPerc * qlen;
                    if(!formIValue("allowShorterEnds",0))masterAl.allowShorterEnds=masterAl.minMatchLen;
                }

    PERF_START("CPLX");
                bool isok=(complexityWindow!=0) ? (sFilterseq::complexityFilter( seq, qlen, complexityWindow, complexityEntropy )==0 ? true:false ): true ;
    PERF_END();

    PERF_START("QUAL");

                if(isok && maximumPercentLowQualityAllowed ){
                    const char * qua=qry->qua(iq);
                    if(qua) {


                        idx lowqua=0;
                        for( idx i=0 ; i<qlen; ++i) {
                            if( quabit ) {
                                if( (qua[i/8]&(((idx)1)<<(i%8))) ==0 )
                                ++lowqua;
                            }else if( (qua[i]) < 20 )
                                ++lowqua;
                        }
                        if( lowqua*100 > qlen*maximumPercentLowQualityAllowed )
                            isok=false;
                    }
                }

                if(masterAl.minMatchLen>qlen) {
                    isok=false;
                }
    PERF_END();



    PERF_START("ALIGNMENTS");
                if(isok) {
                    idx qsim=qry->sim(iq);
                    ++cntComplex;
                    PERF_START("ALIGNMENTS-FAST");
                    if(doubleStagePerfect) {

                        idx rem_looseExtenderMismatchesPercent=masterAl.looseExtenderMismatchesPercent;
                        idx rem_looseExtenderMinimumLengthPercent=masterAl.looseExtenderMinimumLengthPercent;

                        idx rem_flagset=flagSet;
                        idx rem_maxExtensionGaps=masterAl.maxExtensionGaps;
                        idx rem_minMatchLen=masterAl.minMatchLen;
                        idx rem_isMinMatchPercentage=masterAl.isMinMatchPercentage;
                        idx rem_ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment=masterAl.ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment;
                        idx rem_ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment=masterAl.ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment;


                        flagSet|=(sBioseqAlignment::fAlignOptimizeDoubleHash|sBioseqAlignment::fAlignIdentityOnly);
                        masterAl.looseExtenderMismatchesPercent=0;
                        masterAl.maxExtensionGaps=0;
                        masterAl.minMatchLen=qlen;
                        masterAl.isMinMatchPercentage=false;
                        masterAl.looseExtenderMinimumLengthPercent=100;
                        masterAl.ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment=0;
                        masterAl.ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment=1;


                        PERF_START("ALIGNMENTS-VERY-FAST");
                        found=masterAl.alignSeq(&alignmentMap, sub, seq, qlen, sNotIdx, iq-qStart ,  flagSet, qsim);
                        PERF_END();

                        if(found)
                            ++cntVeryFast;



                        else if(doubleStagePerfect>1)  {
                            flagSet|=(sBioseqAlignment::fAlignSearchRepeats);
                            masterAl.ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment=0;
                            masterAl.ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment=1;
                            PERF_START("ALIGNMENTS-JUST-FAST");
                            found=masterAl.alignSeq(&alignmentMap, sub, seq, qlen, sNotIdx, iq-qStart ,  flagSet, qsim);
                            PERF_END();
                            if(found)
                                ++cntFast;
                        }



                        masterAl.looseExtenderMismatchesPercent=rem_looseExtenderMismatchesPercent;
                        flagSet=rem_flagset;
                        masterAl.maxExtensionGaps=rem_maxExtensionGaps;
                        masterAl.minMatchLen=rem_minMatchLen;
                        masterAl.isMinMatchPercentage=rem_isMinMatchPercentage;
                        masterAl.looseExtenderMinimumLengthPercent=rem_looseExtenderMinimumLengthPercent;
                        masterAl.ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment=rem_ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment;
                        masterAl.ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment=rem_ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment;

                    }
                    PERF_END();

                    if(!found) {
                        PERF_START("ALIGNMENTS-SLOW");
                        found=masterAl.alignSeq(&alignmentMap, sub, seq, qlen, sNotIdx, iq-qStart ,  flagSet, qsim);
                        if(found)
                            ++cntSlow;
                        PERF_END();
                    }

                    if(!found)
                        ++cntNotFound;

#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD) && defined(printMethodComparisonHits)
                    if(cmpMethodsOutputISDIFF) {
                        ::printf("%s",cmpMethodsOutput.ptr());
                    }
                    cmpMethodsOutputISDIFF = false;
                    cmpMethodsOutput.cut0cut();
#endif

                }
                else if(subTotalDone==0){
                    ++cntSimple;
                }
                PERF_END();


                if(found) {
                    ++qriesHit;
                    cntFound+=qry->rpt(iq)*found;
                }

                curWork=((subTotalDone)*(qEnd-qStart)+(subjectIn)*(iq-qStart));
                totalQryLen+=qlen;
                if ( totalQryLen > progressMod * progressIntervalBases ){
                    progressMod = ((totalQryLen-1)/progressIntervalBases)+1;
                    if(!reqProgress(cntFound, curWork, totWork) ){
                        subTotalDone=subTotalLen;
                        doBreak = true;
                        break;
                        }
                    PERF_PRINT();
                    logOut(eQPLogType_Debug,"\n\tnotfound %" DEC ",foundfast %" DEC ",foundveryfast %" DEC ",foundslow %" DEC "\n\tcomplex %" DEC " simple %" DEC "\n\tlowqaul %" DEC "\n\tlookups: hash %" DEC " bloom %" DEC " extension %" DEC " SW %" DEC " SW-success %" DEC " \n\n"  , cntNotFound,  cntFast, cntVeryFast, cntSlow, cntComplex, cntSimple, cntLowQua,cntHashLookup,cntBloomLookup,cntExtension,cntAlignmentSW,cntSuccessSW);

                }
                if(maxNumberQuery!=0 && iqry>qStart+maxNumberQuery)
                    break;

            }
            logOut(eQPLogType_Debug,"\n\tnotfound %" DEC ",foundfast %" DEC ",foundveryfast %" DEC ",foundslow %" DEC "\n\tcomplex %" DEC " simple %" DEC "\n\tlowqaul %" DEC "\n\tlookups: hash %" DEC " bloom%" DEC " extension %" DEC " SW %" DEC " SW-success %" DEC " \n\n"  , cntNotFound,  cntFast, cntVeryFast, cntSlow, cntComplex, cntSimple, cntLowQua, cntHashLookup,cntBloomLookup,cntExtension,cntAlignmentSW,cntSuccessSW);

#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD)
            ::printf("\n\n\n"
                "COMPARE METHODS:\n"
                "\tFiltering stage"
                "\t\tBOTH %" DEC ",NEW %" DEC ",OLD %" DEC ",NONE %" DEC ""
                "\n\tAlignments "
                "\t\tBOTH %" DEC ",NEW %" DEC ",OLD %" DEC ",NONE %" DEC "\n"  , cntFilteredBOTH, cntFilteredNEWonly, cntFilteredOLDonly, cntFilteredNONE, cntAlignmentsBOTH, cntAlignmentsNEWonly, cntAlignmentsOLDonly, cntAlignmentsNONE);
#endif
            if(qriesHit>=qEnd-qStart )
                break;


        }

        curWork=totWork;
        reqProgress(cntFound, curWork, totWork);

        if(reverseEngine){
            sSwapI(qStart, sStart);
            sSwapI(qEnd, sEnd);
            sBioseq * t=sub;sub=qry;qry=t;
        }


        logOut(eQPLogType_Debug,"Analyzing results\n");

        AlignMapParser alignParser(*this, Sub, Qry);
        idx countAls = 0;
        sStr errmsg;
        if (sRC rc = alignParser.writeAls(alignmentMap, qStart, flagSet, countAls, errmsg)) {
            reqSetInfo(req, eQPInfoLevel_Error, "Failed to save alignment result part: %s", rc.print());
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

    }

    if( doBreak ) {
        return 0;
    }

    if( isLastInMasterGroupWithLock() ) {
        reqSetStatus(req, eQPReqStatus_Running);
        logOut(eQPLogType_Debug,"Submitting collector req\n");
        idx reqCollector = reqSubmit("dna-hexagon-collector");

        reqSetData(reqCollector, "hexagonMasterId", "%lld", masterId ? masterId : grpId);

        grpAssignReqID(reqCollector, masterId ? masterId : grpId, reqSliceCnt);
        reqSetAction(reqCollector, eQPReqAction_Run);
    }

    reqSetProgress(req, cntFound, 100);
    reqSetStatus(req, eQPReqStatus_Done);


    PERF_PRINT();

    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv);

    DnaHexagon backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-hexagon",argv[0]));
    return (int)backend.run(argc,argv);
}



