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
#include <qlib/QPrideProc.hpp>

#include <slib/utils.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>


class DnaHexagon : public sQPrideProc
{
    public:
        DnaHexagon(const char * defline00,const char * srv) : sQPrideProc(defline00,srv)
        {
        }
        virtual idx OnExecute(idx);
//        virtual idx OnCollect(idx);

        idx getTheFlags( void )
        {
            //idx isDoubleHash = formIValue("doubleHash") ? true : false;
            idx reverseEngine=formIValue("reverseEngine",0);
            idx keepAllMatches=formIValue("keepAllMatches",1);
            idx flagSet=sBioseqAlignment::fAlignForward;
            if( formIValue("isglobal") ) flagSet|=sBioseqAlignment::fAlignGlobal;//else flagSet|=sBioseqAlignment::fAlignLocal;
            if( formIValue("isbackward", 1) ) {
                flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
                if( formIValue("isbackward", 1) == 2 ) flagSet &= ~sBioseqAlignment::fAlignForward;
            }
            if( formIValue("isoptimize", 1) ) flagSet|=sBioseqAlignment::fAlignOptimizeDiagonal;
            if( formIValue("isextendtails") ) flagSet|=sBioseqAlignment::fAlignMaxExtendTail;
            if( formIValue("isCircular") ) flagSet|=sBioseqAlignment::fAlignCircular;
            if( formIValue("doubleHash") ) flagSet|=sBioseqAlignment::fAlignOptimizeDoubleHash;
            if( formIValue("alignmentEngine",1) ==0 )flagSet|=sBioseqAlignment::fAlignIdentityOnly; //keep SW as default

            if( formBoolValue("keepMarkovnikovMatches", false) ) flagSet |= sBioseqAlignment::fAlignKeepMarkovnikovMatch;

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

idx DnaHexagon::OnExecute(idx req)
{
    sStr errS;

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ initialize the parameters
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    idx maxNumberQuery=formIValue("maxNumberQuery");
    idx seed=formIValue("seed",11);

    idx isDoubleHash = formIValue("doubleHash") ? true : false;
    if(seed<0) {seed=-seed; isDoubleHash=true;}
    idx reverseEngine=formIValue("reverseEngine",0);
    //idx subjectChunkOverlap=formIValue("subjectChunkOverlap",100);
//    idx saveSeedHash=formIValue("saveSeedHash",0);

    //idx keepAllMatches=formIValue("keepAllMatches",1);

    idx slice=formIValue("slice",sHiveseq::defaultSliceSizeI);
    //idx reportZeroHits=formIValue("reportZeroHits");
    idx maxHashBin=formIValue("maxHashBin",50); if(!maxHashBin)maxHashBin=50;
    idx complexityWindow=formIValue("complexityWindow",0);if(complexityWindow<0)complexityWindow=seed;
    real complexityEntropy=formRValue("complexityEntropy",0);
    //idx acceptNNNRead=formRValue("acceptNNNRead",1);
    idx complexityRefWindow=formIValue("complexityRefWindow",0);if(complexityRefWindow<0)complexityRefWindow=seed;
    real complexityRefEntropy=formRValue("complexityRefEntropy",0);
    idx acceptNNNQuaTrheshold=formRValue("acceptNNNQuaTrheshold",1);
    real maximumPercentLowQualityAllowed=formRValue("maximumPercentLowQualityAllowed",0);
    idx doubleStagePerfect=formIValue("doubleStagePerfect",1);
    idx quabit=0;
    //doubleStagePerfect=2;

    idx maxSubjectBasesToCompile=formIValue("maxSubjectBasesToCompile",0); if(!maxSubjectBasesToCompile)maxSubjectBasesToCompile=1024*1024*128;

    idx hashExpectation=maxSubjectBasesToCompile/(((idx)1)<<(2*seed));//sub->dim()*sub->len(0)/(((idx)1)<<(2*seed));
    if(!hashExpectation)hashExpectation=1;
    hashExpectation=(maxHashBin<0) ? hashExpectation*(-maxHashBin) : maxHashBin ;


    idx flagSet=getTheFlags( );
    /*idx flagSet=sBioseqAlignment::fAlignForward;
    if( formIValue("isglobal") ) flagSet|=sBioseqAlignment::fAlignGlobal;//else flagSet|=sBioseqAlignment::fAlignLocal;
    if( formIValue("isbackward", 1) ) flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
    if( formIValue("isoptimize", 1) ) flagSet|=sBioseqAlignment::fAlignOptimizeDiagonal;
    if( formIValue("isextendtails") ) flagSet|=sBioseqAlignment::fAlignMaxExtendTail;
    if( formIValue("isCircular") ) flagSet|=sBioseqAlignment::fAlignCircular;

    if( formIValue("alignmentEngine") ==0 )flagSet|=sBioseqAlignment::fAlignIdentityOnly;
     */
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
/*
    if(keepAllMatches==0)flagSet|=sBioseqAlignment::fAlignKeepFirstMatch;
    if(keepAllMatches==1)flagSet|=sBioseqAlignment::fAlignKeepBestFirstMatch;
    if(keepAllMatches==3)flagSet|=sBioseqAlignment::fAlignKeepAllBestMatches;
    if(keepAllMatches==4)flagSet|=sBioseqAlignment::fAlignKeepRandomBestMatch;
    if(reverseEngine)flagSet|=sBioseqAlignment::fAlignReverseEngine;
*/
    //idx searchRepeatsAndTrans=formIValue("searchRepeatsAndTrans",0);
/*    if(searchRepeatsAndTrans==1)flagSet|=sBioseqAlignment::fAlignSearchRepeats;
    if(searchRepeatsAndTrans==2)flagSet|=sBioseqAlignment::fAlignSearchTranspositions|sBioseqAlignment::fAlignSearchRepeats;
*/

    idx qStart,qEnd,sStart,sEnd;
    idx cntFound=0;

    sVec < idx > alignmentMap;//(0,pathT.ptr()); // flat formatted list of sBioseqAlignment::Alignments for query mappings to Amplicons

    sBioseq * sub, * qry;

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ load the subject and query sequences
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    const char * subject = formValue("subject");
    sHiveseq Sub(user, subject, sBioseq::eBioModeShort, false, false, &errS);
    //Sub.reindex();
    if( Sub.dim() == 0 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Reference '%s' sequences are missing or corrupted%s%s", subject ? subject : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    errS.cut0cut();

    /*
    sStr singleSubPath;
    sVec<idx> rgSub;sString::scanRangeSet(subject,0,&rgSub,0,0,0);
    if(rgSub.dim()==1) {
        sUsrFile obj(rgSub[0],user); // todo : get the best way of getting paths
//        obj.getPath(&singleSubPath);
    }
    */

    // load the subject and query sequences
    const char * query=formValue("query");
    sHiveseq Qry(sQPride::user, query, sBioseq::eBioModeShort, false, false, &errS);//Qry.reindex();
    if(Qry.dim()==0) {
        reqSetInfo(req, eQPInfoLevel_Error, "Query '%s' sequences are missing or corrupted%s%s", query ? query : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0; // error
    }
    errS.cut0cut();
    sub=&Sub;
    qry=&Qry;

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare the Alignment Engine
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    bool doBreak = false;

    {
        sBioseqAlignment masterAl; // alignment classes for master and for amplicons
        masterAl.MatSW.addM(4*1024*1024);// reserve few megs at the beginning
        masterAl.MatBR.addM(4*1024*1024);

        masterAl.costMatch=formIValue("costMatch",5);
        masterAl.costMismatch=formIValue("costMismatch",-4);
        masterAl.costMismatchNext=formIValue("costMismatchNext",-6);
        masterAl.costGapOpen=formIValue("costGapOpen",-12);
        masterAl.costGapNext=formIValue("costGapNext",-4);
        masterAl.computeDiagonalWidth=formIValue("computeDiagonalWidth",0);
        masterAl.maxMissQueryPercent=formRValue("maxMissQueryPercent",15);
        masterAl.minMatchLen=formIValue("minMatchLen",75);masterAl.localBitMatch.resize(1+( (masterAl.minMatchLen<=0?1:masterAl.minMatchLen))/(sizeof(idx)*8));
        masterAl.considerGoodSubalignments=formIValue("considerGoodSubalignments",1);

        masterAl.scoreFilter=formIValue("scoreFilter",0);
        masterAl.considerGoodSubalignments=formIValue("considerGoodSubalignments",1);
        masterAl.allowShorterEnds=formIValue("allowShorterEnds",0);if(!masterAl.allowShorterEnds)masterAl.allowShorterEnds=masterAl.minMatchLen;
        masterAl.maxExtensionGaps=formIValue("maxExtensionGaps",0);
        //masterAl.maxExtensionGaps=1;
        masterAl.hashStp=formIValue("hashStp",reverseEngine ? seed : 1 );
        masterAl.bioHash.hashStp=formIValue("hashCompileStp",1 );
    //masterAl.hashStp=1;
    //masterAl.bioHash.hashStp=1;
        masterAl.looseExtenderMismatchesPercent=formIValue("looseExtenderMismatchesPercent",25);
        masterAl.looseExtenderMinimumLengthPercent=formIValue("looseExtenderMinimumLengthPercent",66);
    //masterAl.looseExtenderMismatchesPercent=15;
    //masterAl.looseExtenderMinimumLengthPercent=75;

        masterAl.maxHitsPerRead=formIValue("maxHitsPerRead",50);
        masterAl.compactSWMatrix=formIValue("compactSWMatrix",1);
        masterAl.maxSeedSearchQueryPos=formIValue("maxSeedSearchQueryPos",512);
        //masterAl.selfSubjectPosJumpInNonPerfectAlignment=formIValue("selfSubjectPosJumpInNonPerfectAlignment",searchRepeatsAndTrans ? 0 : 1);
        masterAl.selfSubjectPosJumpInNonPerfectAlignment=formIValue("selfSubjectPosJumpInNonPerfectAlignment",1);
        masterAl.selfQueryPosJumpInNonPerfectAlignment=formIValue("selfQueryPosJumpInNonPerfectAlignment",1);


        //masterAl.callbackFunc=(sBioseqAlignment::callbackReport)reportProgress;
        //masterAl.callbackParam=this;
        masterAl.bioHash.init(seed,4ll);
        idx isBloom = (seed <= 16) ? sBioseqHash::eCompileBloom  : 0 ; // ( ((udx)1<<(2*seed))/8/1024/1024/1024 <= 2 )  ? sBioseqHash::eCompileBloom  : 0 ;// the size neede for bloom in GB
        if(!isBloom)masterAl.bioHash.collisionReducer=4;


        // determine the thread specific parameters for this slice of the total request
        sub = reverseEngine ? &Qry : &Sub;
        qry = reverseEngine ? &Sub : &Qry;
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

        /*masterAl.selfSimilairityBufferSize=100;
        if(masterAl.selfSimilairityBufferSize) {
            masterAl.SimilarityBuf.addM( Sub.dim() * masterAl.selfSimilairityBufferSize);
        }*/


        sVec < idx > randomQueries;
        if(maxNumberQuery>0){
            randomQueries.add(maxNumberQuery);
            for( idx im=0; im<maxNumberQuery; ++im){
                randomQueries[im]=qStart+1.*(qEnd-qStart-1)*rand()*rand()/RAND_MAX/RAND_MAX;
            }

        }else {
            maxNumberQuery=-maxNumberQuery;
        }





        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ align queries and subjects
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


        //sVec<idx> tmpFailed;tmpFailed.add(qEnd-qStart);tmpFailed.set(0);
        // compute cumulative size of subject sequences
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
        idx stDim=0;//sEnd-sStart;
        idx curSubPos=0;
        curSub=0;

        idx curWork=0;
        idx totWork=(qEnd-qStart)*subTotalLen;
        idx cntSimple=0,cntComplex=0,cntLowQua=0,cntFast=0,cntVeryFast=0,cntSlow=0,cntNotFound=0;
        idx subjectIn=0;
        idx chunkSub=0;

//goto dodo;
        //for( idx is=sStart ; is<sEnd ; is=curS, ++stDim) {
        for( idx subTotalDone=0; !doBreak && subTotalDone<subTotalLen ; subTotalDone+=subjectIn) {
            subjectIn=0;
            idx curSubSave=curSub, curSubPosSave=curSubPos;
            idx curSubAnnotFileSave=curSubAnnotFile, curSubAnnotRangeSave=curSubAnnotRange, curSubAnnotPosSave=curSubAnnotPos, curSubAnnotRangeSectionSave=curSubAnnotRangeSection;

            //logOut(eQPLogType_Debug,"Processing next subject range from %" DEC " position %" DEC " \n"  , curSubSave, curSubPos);

    PERF_START("KMERE_SEEDING");
            // estimate how many bases from current sequence are are getting into this bunch

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
    //if(chunkSub==4)break;
            ++stDim;

            masterAl.bioHash.reset();
            ++chunkSub;

            //logOut(eQPLogType_Debug,"... to subject range %" DEC " position %" DEC " \n"  , curSub, curSubPos);

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

                        idx posToStartHash=sen[curSubAnnotRangeSection].start;//-seed;
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
            for(idx iqry=qStart; !doBreak && iqry<qEnd ; ++iqry ) { //curQ=iq;
                idx iq=(randomQueries.dim ()) ? randomQueries[iqry-qStart] : iqry;
                idx iqx=iq-qStart;

                /*const char * dbgid=qry->id(iq);
                if(strstr(dbgid,"12 pos=556911 len=50 REV ori=vargi|330443590")==dbgid) {
                    ::printf(":------------------------%s-------------%" DEC "    %" DEC "--\n",dbgid,req,iq);
                   // exit(0);
                }*/
                //continue;


                if( ((flagSet & sBioseqAlignment::fAlignKeepFirstMatch) && (flagSet & sBioseqAlignment::fAlignReverseEngine) == 0) && masterAl.QHitBitmask && (masterAl.QHitBitmask[iqx / 8] & (((idx) 1) << (iqx % 8))) ) {
                    ++qriesHit;
                    continue;
                }

                idx found=0;


                idx qlen=qry->len(iq);
                const char * seq=qry->seq(iq);

    PERF_START("CPLX");
                bool isok=(complexityWindow!=0) ? (sFilterseq::complexityFilter( seq, qlen, complexityWindow, complexityEntropy )==0 ? true:false ): true ;
    PERF_END();

    PERF_START("QUAL");

                if(isok && maximumPercentLowQualityAllowed ){
                    const char * qua=qry->qua(iq);
                    if(qua) {

    //                    isok=(complexityWindow!=0) ? sFilterseq::complexityFilter( qry->seq(iq), qry->len(iq), complexityWindow, complexityEntropy ) : true ;

                        idx lowqua=0;
                        for( idx i=0 ; i<qlen; ++i) {
                            if( quabit ) {
                                if( (qua[i/8]&(((idx)1)<<(i%8))) ==0 ) // quality bit is not set
                                ++lowqua; // ignore the bases with low Phred score
                            }else if( (qua[i]) < 20 ) // quality bit is not set
                                ++lowqua; // ignore the bases with low Phred score
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
                //if( iq==5559 )
                //    ::printf("POTENTIAL PROBLEM %" DEC "\n",iq);
                //idx ofsAA=alignmentMap.dim();
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
                        idx rem_selfSubjectPosJumpInNonPerfectAlignment=masterAl.selfSubjectPosJumpInNonPerfectAlignment;
                        idx rem_selfQueryPosJumpInNonPerfectAlignment=masterAl.selfQueryPosJumpInNonPerfectAlignment;
                        //idx rem_maxHitsPerRead=masterAl.maxHitsPerRead;


                        flagSet|=(sBioseqAlignment::fAlignOptimizeDoubleHash|sBioseqAlignment::fAlignIdentityOnly);
                        //flagSet|=(sBioseqAlignment::fAlignOptimizeDoubleHash|sBioseqAlignment::fAlignSearchRepeats);
                        masterAl.looseExtenderMismatchesPercent=0;
                        masterAl.maxExtensionGaps=0;
                        masterAl.minMatchLen=qlen;
                        masterAl.looseExtenderMinimumLengthPercent=100;
                        masterAl.selfSubjectPosJumpInNonPerfectAlignment=0;
                        masterAl.selfQueryPosJumpInNonPerfectAlignment=1;

                        //masterAl.maxHitsPerRead=rem_maxHitsPerRead;

                        PERF_START("ALIGNMENTS-VERY-FAST");
                        found=masterAl.alignSeq(&alignmentMap, sub, seq, qlen, sNotIdx, iq-qStart ,  flagSet, qsim);//, sFos, sBioseqHash::fos(qFos,iq) ); // , reverseEngine
                        PERF_END();

                        if(found)
                            ++cntVeryFast;



                        else if(doubleStagePerfect>1)  {
                            flagSet|=(sBioseqAlignment::fAlignSearchRepeats);
                            masterAl.selfSubjectPosJumpInNonPerfectAlignment=0;
                            masterAl.selfQueryPosJumpInNonPerfectAlignment=1;
                            PERF_START("ALIGNMENTS-JUST-FAST");
                            found=masterAl.alignSeq(&alignmentMap, sub, seq, qlen, sNotIdx, iq-qStart ,  flagSet, qsim);//, sFos, sBioseqHash::fos(qFos,iq) ); // , reverseEngine
                            PERF_END();
                            if(found)
                                ++cntFast;
                        }


                        //if(found && (alignmentMap[ofsAA+6]!=-50 || alignmentMap[ofsAA+5]!=0 || alignmentMap[ofsAA+4]!=0 ) ){
                        //    ::printf("::::::::::::::::::::: idQry=%" DEC " %s\n",iq, qry->id(iq) );
                        //    exit(0);
                        //}

                        masterAl.looseExtenderMismatchesPercent=rem_looseExtenderMismatchesPercent;
                        flagSet=rem_flagset;
                        masterAl.maxExtensionGaps=rem_maxExtensionGaps;
                        masterAl.minMatchLen=rem_minMatchLen;
                        masterAl.looseExtenderMinimumLengthPercent=rem_looseExtenderMinimumLengthPercent;
                        masterAl.selfSubjectPosJumpInNonPerfectAlignment=rem_selfSubjectPosJumpInNonPerfectAlignment;
                        masterAl.selfQueryPosJumpInNonPerfectAlignment=rem_selfQueryPosJumpInNonPerfectAlignment;

                        //masterAl.maxHitsPerRead=rem_maxHitsPerRead;
                    }
                    PERF_END();

                    if(!found) {
                        PERF_START("ALIGNMENTS-SLOW");
                        found=masterAl.alignSeq(&alignmentMap, sub, seq, qlen, sNotIdx, iq-qStart ,  flagSet, qsim);//, sFos, sBioseqHash::fos(qFos,iq) ); // , reverseEngine
                        if(found)
                            ++cntSlow;
                        PERF_END();
                    }

                    if(!found)
                        ++cntNotFound;

                }
                else if(subTotalDone==0){
                    ++cntSimple;
                }
                PERF_END();


                if(found) {
                    ++qriesHit;
                    cntFound+=qry->rpt(iq)*found;
                    //if(alignmentMap[ofsAA+6]!=-100)
                    //    ::printf("Oo! %" DEC "\n",iq);
                }

                //curWork=(is*(qEnd-qStart)+(curS-is)*(iq-qStart));
                curWork=((subTotalDone)*(qEnd-qStart)+(subjectIn)*(iq-qStart));

                if ( (iqry%10000)==0 ){
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
                    //{subTotalDone=subTotalLen;break;}

            }
            logOut(eQPLogType_Debug,"\n\tnotfound %" DEC ",foundfast %" DEC ",foundveryfast %" DEC ",foundslow %" DEC "\n\tcomplex %" DEC " simple %" DEC "\n\tlowqaul %" DEC "\n\tlookups: hash %" DEC " bloom%" DEC " extension %" DEC " SW %" DEC " SW-success %" DEC " \n\n"  , cntNotFound,  cntFast, cntVeryFast, cntSlow, cntComplex, cntSimple, cntLowQua, cntHashLookup,cntBloomLookup,cntExtension,cntAlignmentSW,cntSuccessSW);

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


        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ Analyze results
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
        logOut(eQPLogType_Debug,"Analyzing results\n");
        const char * const altPath = "file://alignment-slice.vioalt";
        if( alignmentMap.dim() ) {
            sStr pathT;
            sFil ff;
            if( !reqSetData(req, altPath, 0, 0) || !reqDataPath(req, &altPath[7], &pathT) || !ff.init(pathT.ptr()) || !ff.ok() ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Failed to save alignment result part");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            sBioseqAlignment::filterChosenAlignments(&alignmentMap, qStart, flagSet, &ff);
        } else {
            // FIXME: replace line below with reqDelData() when it's added
            reqSetData(req, &altPath[7], 0, 0); // magic combination of arguments to delete file and wipe from db; 7 matters
        }
    }
//dodo:
    /*
PERF_START("ANALYSIS");
    if (isLastInGroup()){
        logOut(eQPLogType_Info,"Concatenating results\n");

        progress100Start=50;
        progress100Count=50;

        sStr srcAlignmentsT;grpDataPaths(grpId, "alignment-slice.vioalt", &srcAlignmentsT, vars.value("serviceName"));

        const char * resultFileTemplate =  formValue("resultFileTemplate", 0);
        if(!resultFileTemplate)resultFileTemplate = "";
        sStr resultFileName("%salignment.hiveal",resultFileTemplate);

        sStr dstAlignmentsT;sQPrideProc::reqAddFile(dstAlignmentsT, resultFileName.ptr());
        sVioal vioAltAAA(0,sub,qry);
        vioAltAAA.myCallbackFunction=sQPrideProc::reqProgressStatic;
        vioAltAAA.myCallbackParam=this;


        sDic < sBioal::LenHistogram > lenHistogram;
        sStr coverT;sQPrideProc::reqAddFile(coverT, "coverage_dict");
        sVec< idx > subCoverage(coverT.ptr());
//        subCoverage.init(coverT.ptr());

        vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT,srcAlignmentsT, 4000000, false, flagSet ,0,&lenHistogram,&subCoverage);
        if(lenHistogram.dim()){
            resultFileName.printf(0,"%shistogram.csv",resultFileTemplate);
            dstAlignmentsT.cut(0);sQPrideProc::reqAddFile(dstAlignmentsT, resultFileName.ptr()); // "histogram.csv"
            sFile::remove(dstAlignmentsT);
            sFil hist(dstAlignmentsT);
            if(hist.ok())
                sBioal::printAlignmentHistogram(&hist, &lenHistogram );
        }
//        if(subCoverage.dim()){
//            dstAlignmentsT.cut(0);sQPrideProc::reqAddFile(dstAlignmentsT, "summary.csv");
//            sFile::remove(dstAlignmentsT);
//            sFil cover(dstAlignmentsT);
//            if(cover.ok())
//                sBioal::printAlignmentCoverage(&cover, &subCoverage );
//        }

    }

PERF_END();
*/
    if( doBreak ) {
        return 0;
    }
    if( isLastInMasterGroup() ) {
        idx reqCollector = reqSubmit("dna-hexagon-collector");
        //grpAssignReqID(reqCollector, grpId, reqSliceCnt) ;
        grpAssignReqID(reqCollector, grpId, reqSliceCnt);
        reqSetAction(reqCollector, eQPReqAction_Run);
    }
    reqSetProgress(req, cntFound, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    PERF_PRINT();

    return 0;
}
/*
idx DnaHexagon::OnExecute(idx req)
{
    idx reqCollector=reqSubmit( "dna-hexagon-collector");
    grpAssignReqID(reqCollector, grpId, reqSliceCnt) ;
    reqSetAction(reqCollector,eQPReqAction_Run);
}*/

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaHexagon backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-hexagon",argv[0]));
    return (int)backend.run(argc,argv);
}



