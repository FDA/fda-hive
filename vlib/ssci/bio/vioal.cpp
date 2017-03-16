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
#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/utils.hpp>
#include <ssci/bio/vioalt.hpp>

#include <ctype.h>

#define _getPairedRead(_iqry) (_iqry + (1-(2*(_iqry/(_bioal_long_cnt/2))))*(_bioal_long_cnt/2))
#define _fragmentScoreCompare(_sc11, _sc12,_sc21, _sc22) \
    ( ( _sc11 + _sc12 == _sc21 + _sc22 ) ? ( (_sc11?_sc11:1) * (_sc12?_sc12:1) > (_sc21?_sc21:1) * (_sc22?_sc22:1) ) : ( _sc11 + _sc12 > _sc21 + _sc22 ) )
#define _fragmentScoreMatch(_sc11, _sc12,_sc21, _sc22) \
    ( ( _sc11 + _sc12 == _sc21 + _sc22 ) ? ( ( (_sc11 == _sc21) && (_sc12 == _sc22) ) || ( (_sc11 == _sc22) && (_sc12 == _sc21) ) ) : false )

#define _setHistogram(_hdr) \
    if(lenHistogram) {\
        idx llen=Qry->len(_hdr->idQry()); \
        idx l=llen; \
        ++(lenHistogram->set(&l,sizeof(l)))->cntSeq; \
        l=_hdr->lenAlign(); \
        ++(lenHistogram->set(&l,sizeof(l)))->cntAl; \
        l=_hdr->qryStart()+_hdr->match()[1]; \
        if(l>1) \
            ++(lenHistogram->set(&l,sizeof(l)))->cntLeft; \
        l=(l+_hdr->lenAlign()); \
        if(l<llen-1) \
            ++(lenHistogram->set(&l,sizeof(l)))->cntRight; \
    }
#define _setSubjectCoverage(_hdr, _al) \
    if(totSubjectCoverage) { \
        *totSubjectCoverage->ptrx(_al->sub) += _hdr->lenAlign() ; \
    }
#define _getHdrFromAlFact(_al) (sBioseqAlignment::Al * )fileList[_al->iFile].ptr( _al->ofsFile )

bool sVioal::filterPair(ALFact * _i1, ALFact * _i2, sBioseqAlignment::Al *_hdr1, sBioseqAlignment::Al *_hdr2, digestParams &params) {
    if( !(params.flags&sBioseqAlignment::fAlignIsPairedEndMode) ) return false;
    if( (params.flags&sBioseqAlignment::fAlignKeepPairOnSameSubject) ) {
        if( _i1->sub != _i2->sub )
            return true;
    }
    if( (params.flags&sBioseqAlignment::fAlignKeepPairDirectionality) ) {
        if ( !(_hdr1->isForward())!=!(_hdr2->isBackwardComplement() ) || !(_hdr1->isBackwardComplement())!=!(_hdr2->isForward()) ) { /*logical X0R*/
            return true;
        }
    }
    idx fragmentLength = 0, abs = 1;
    if( params.maxFragmentLength || params.minFragmentLength ) {

        if ( !(_hdr1->isForward())!=!(_hdr2->isBackwardComplement() ) || !(_hdr1->isBackwardComplement())!=!(_hdr2->isForward()) ) { /*logical X0R*/
            return true;
        } else {
            if( _hdr1->isForward() ) {                                   //  1------------               2---------------(2+lenAlign)
                fragmentLength = _i2->pos + _hdr2->lenAlign() - _i1->pos ;
            } else {                                                     //  2------------               1---------------(1+lenAlign)
                fragmentLength = _i1->pos + _hdr1->lenAlign() - _i2->pos ;
            }
        }
        if( fragmentLength < 0 && ( params.maxFragmentLength < 0 || params.minFragmentLength < 0 ) ) {  // in case  pairs are pointing outwards <---      ---->
            abs = -1;
        }
    }
    if( params.minFragmentLength && abs*fragmentLength < params.minFragmentLength) {
        return true;
    }
    if( params.maxFragmentLength && abs*fragmentLength > params.maxFragmentLength) {
        return true;
    }
    return false;
}
idx sVioal::DigestFirstStage(sVioDB * db, sBioseqAlignment::Al * hdr, sBioseqAlignment::Al * hdre, sBioseq * qry , sBioal::Stat * stat, bool selectedOnly  , bool doQueries, idx biomode, idx oneRepeatForall)
{

    // _/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Scan Alignments one by one
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/

    idx * match;
    idx iAl, idSub,idQry,  iTot;
    idx myRecordID=0;

    //idx foundAll=0,foundAllRpt=0;


    for( iAl=0, iTot=0 ; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) , ++iTot) {

        if( selectedOnly && !((hdr->flags())&(sBioseqAlignment::fSelectedAlignment) ) )
            continue;

        idx isub=hdr->idSub();
        idx rpt=oneRepeatForall ? oneRepeatForall : (qry ? qry->rpt(hdr->idQry()) : 1) ;

        stat[1+isub].found+=1;
        stat[1+isub].foundRpt+=(biomode==sBioseq::eBioModeShort)?rpt:1;
        //foundAll++;
        //foundAllRpt+=rpt;

        match=hdr->match();
        idSub=hdr->idSub();
        idQry=hdr->idQry();

        // add the Alignment Record and reserve space for three relationships
        db->AddRecord(eTypeAlignment,(const void *)hdr, sizeof(sBioseqAlignment::Al) );
        db->AddRecordRelationshipCounter(eTypeAlignment, 0, 1); // 1 here means the relationship to Match : see relationshiplistAlignment[1]
        db->AddRecordRelationshipCounter(eTypeAlignment, 0, 2); // 2 here means the relationship to Rpt : see relationshiplistAlignment[2]
//        db->AddRecordRelationshipCounter(eTypeAlignment, 0, 3); // 3 here means the relationship to Match : see relationshiplistAlignment[3]

        // add the Match Record and reserve space for a single relationships to Alignment
        myRecordID=0;

        if(db->SetRecordIndexByBody((const void *)match, eTypeMatch, &myRecordID, sizeof(idx)*hdr->dimAlign() ))
            db->AddRecord(eTypeMatch,(const void *)match, sizeof(idx)*hdr->dimAlign() );

//        db->AddRecordRelationshipCounter(eTypeMatch, 0, 1); // 1 here means the relationship to Match : see relationshiplistMatch[1]

        // add the Subject Record and reserve space for a single relationships to Alignment
        myRecordID=0;
        if(db->SetRecordIndexByBody((const void *)&(idSub), eTypeSubject, &myRecordID))
            db->AddRecord(eTypeSubject,(const void *)&(idSub), sizeof(idx));
        db->AddRecordRelationshipCounter(eTypeSubject, myRecordID, 1); // 1 here means the relationship to Match : see relationshiplistSubject[1]

        // add the Query Record and reserve space for a single relationships to Alignment
        if(doQueries){
            myRecordID=0;
            if(db->SetRecordIndexByBody((const void *)&(idQry), eTypeQry, &myRecordID))
                db->AddRecord(eTypeQry,(const void *)&(idQry), sizeof(idx));
            db->AddRecordRelationshipCounter(eTypeQry, myRecordID, 1); // 1 here means the relationship to Match : see relationshiplistQuery[1]
        }

        myRecordID=0;
        if(db->SetRecordIndexByBody((const void *)&(rpt), eTypeRpt, &myRecordID))
            db->AddRecord(eTypeRpt,(const void *)&(rpt), sizeof(idx));
        ++iAl;
    }

    return 1;
}

idx sVioal::DigestFragmentStage(ALFact * als, ALFact * ale, sBioal::Stat *  stat, sBioseq * qry, idx biomode, idx oneRepeatForall)
{
    idx isub = 0, rpt = 0;
    for( ALFact * al = als; al <ale ; ++al) {
        rpt=oneRepeatForall ? oneRepeatForall : ( qry ? qry->rpt(al->idQry&(0xFFFFFFFFFFFFFull)) : 1) ;
        isub = al->sub;
        ++stat[isub].found;
        stat[isub].foundRpt+=(biomode==sBioseq::eBioModeShort)?rpt:1;
    }
    return 1;
}


idx sVioal::DigestSecondStage(sVioDB * db, sBioseq * qry, idx * pAl, sBioseqAlignment::Al * hdr,sBioseqAlignment::Al * hdre , bool selectedOnly, bool doQueries, idx oneRepeatForall)
{

    // _/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Scan Alignments one by one
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/

    idx  *match, idSub, idQry;
    for(  ; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) ) {
        if( selectedOnly && !((hdr->flags())&(sBioseqAlignment::fSelectedAlignment) ) )
            continue;


        match=hdr->match();
        idSub=hdr->idSub();
        idQry=hdr->idQry();

        idx rpt=oneRepeatForall ? oneRepeatForall : ( qry ? qry->rpt(hdr->idQry()) : 1) ;

        idx mySubID=db->GetRecordIndexByBody((const void *)&(idSub), eTypeSubject);
        idx myQryID=db->GetRecordIndexByBody((const void *)&(idQry), eTypeQry);
        idx myMatchID=db->GetRecordIndexByBody((const void *)match, eTypeMatch,  sizeof(idx)*hdr->dimAlign() );
        idx myRptID=db->GetRecordIndexByBody((const void *)&rpt, eTypeRpt,  sizeof(idx) );

        db->AddRelation(eTypeAlignment, 1, (*pAl)+1, myMatchID);

        db->AddRelation(eTypeAlignment, 2, (*pAl)+1, myRptID);

        db->AddRelation(eTypeSubject, 1, mySubID,  (*pAl)+1 );


        if(doQueries)
            db->AddRelation(eTypeQry, 1, myQryID,  (*pAl)+1 );

        ++(*pAl);
    }

    return 1;
}


idx sVioal::DigestInit(sVioDB * db , const char * outputfilename)
{

    // _/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Create the Data format headers
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/

    db->init(outputfilename,"vioalt",6,3);//constructor: 6 types(Alignment, Matches, Subjects, Queries, Stat, Repeats) 3 is the maximum relationships


    idx relationlistAlignment[3]={2,6,0}; //{2,3,4}; // to Matches (, Queries and Subjects), Repeats
    idx relationlistMatch[1]={1}; // to the Alignment
    idx relationlistSubject[1]={1}; // to the Alignment
    idx relationlistQuery[1]={1}; // to the Alignment
    idx relationlistStat[1]={1}; // to itself ?
    idx relationlistRpts[1]={6}; // to itself ?


    db->AddType(sVioDB::eOther,3,relationlistAlignment,"align", 1); // Alignments are Al structures
    db->AddType(sVioDB::eOther,1,relationlistMatch,"match", 2); // Match trains
    db->AddType(sVioDB::eInt,1,relationlistSubject,"subject", 3); // subject Ids
    db->AddType(sVioDB::eInt,1,relationlistQuery,"query", 4); // query Ids
    db->AddType(sVioDB::eOther,1,relationlistStat,"stat", 5); // query Ids
    db->AddType(sVioDB::eInt,1,relationlistRpts,"repeat", 6); // query Ids
    return 1;

}

idx sVioal::Digest(const char* outputfilename, bool combineFile, sBioseqAlignment::Al * rawAlignment, idx size , sBioal::Stat * statFailed, bool selectedOnly  , bool doQueries, idx biomode)
{

    // _/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Create the Data format headers
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/

    sVioDB db;
    DigestInit(&db , outputfilename);

/*
    sVioDB db(outputfilename,"vioalt",5,3);//constructor: 4 types(Alignment, Matches, Subjects, Queries) 3 is the maximum relationships


    idx relationlistAlignment[1]={2}; //{2,3,4}; // to Matches, Queries and Subjects
    idx relationlistMatch[1]={1}; // to the Alignment
    idx relationlistSubject[1]={1}; // to the Alignment
    idx relationlistQuery[1]={1}; // to the Alignment
    idx relationlistStat[1]={1}; // to itself ?

    db.AddType(sVioDB::eOther,3,relationlistAlignment,"align", 1); // Alignments are Al structures
    db.AddType(sVioDB::eOther,1,relationlistMatch,"match", 2); // Match trains
    db.AddType(sVioDB::eInt,1,relationlistSubject,"subject", 3); // subject Ids
    db.AddType(sVioDB::eInt,1,relationlistQuery,"query", 4); // query Ids
    db.AddType(sVioDB::eOther,1,relationlistStat,"stat", 5); // query Ids
*/


    // _/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Scan Alignments one by one
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/
    sVec < Stat > stat(sMex::fSetZero); stat.add(1+Sub->dim());

    sBioseqAlignment::Al * hdr=rawAlignment, * hdre=sShift(rawAlignment,size*sizeof(idx));
    DigestFirstStage(&db, hdr, hdre, Qry, stat, selectedOnly  , doQueries, biomode);

    /*
    sBioseqAlignment::Al * hdr, * hdre=sShift(rawAlignment,size*sizeof(idx));;
    idx * match;
    idx iAl, idSub,idQry,  iTot;
    idx myRecordID=0;

    //idx foundAll=0,foundAllRpt=0;

    for( hdr=rawAlignment, iAl=0, iTot=0 ; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) , ++iTot) {

        if( selectedOnly && !((hdr->flags())&(sBioseqAlignment::fSelectedAlignment) ) )
            continue;

        idx isub=hdr->idSub();
        idx rpt=Qry->rpt(hdr->idQry());

        stat[1+isub].found+=1;
        stat[1+isub].foundRpt+=(biomode==0)?rpt:1;
        //foundAll++;
        //foundAllRpt+=rpt;

        match=hdr->match();
        idSub=hdr->idSub();
        idQry=hdr->idQry();

        // add the Alignment Record and reserve space for three relationships
        db.AddRecord(eTypeAlignment,(const void *)hdr, sizeof(sBioseqAlignment::Al) );
        db.AddRecordRelationshipCounter(eTypeAlignment, 0, 1); // 1 here means the relationship to Match : see relationshiplistAlignment[1]
//      db.AddRecordRelationshipCounter(eTypeAlignment, 0, 2); // 2 here means the relationship to Match : see relationshiplistAlignment[2]
//      db.AddRecordRelationshipCounter(eTypeAlignment, 0, 3); // 3 here means the relationship to Match : see relationshiplistAlignment[3]

        // add the Match Record and reserve space for a single relationships to Alignment
        myRecordID=0;

        if(db.SetRecordIndexByBody((const void *)match, eTypeMatch, &myRecordID, sizeof(idx)*hdr->dimAlign() ))
            db.AddRecord(eTypeMatch,(const void *)match, sizeof(idx)*hdr->dimAlign() );

//      db.AddRecordRelationshipCounter(eTypeMatch, 0, 1); // 1 here means the relationship to Match : see relationshiplistMatch[1]

        // add the Subject Record and reserve space for a single relationships to Alignment
        myRecordID=0;
        if(db.SetRecordIndexByBody((const void *)&(idSub), eTypeSubject, &myRecordID))
            db.AddRecord(eTypeSubject,(const void *)&(idSub), sizeof(idx));
        db.AddRecordRelationshipCounter(eTypeSubject, myRecordID, 1); // 1 here means the relationship to Match : see relationshiplistSubject[1]

        // add the Query Record and reserve space for a single relationships to Alignment
        if(doQueries){
            myRecordID=0;
            if(db.SetRecordIndexByBody((const void *)&(idQry), eTypeQry, &myRecordID))
                db.AddRecord(eTypeQry,(const void *)&(idQry), sizeof(idx));
            db.AddRecordRelationshipCounter(eTypeQry, myRecordID, 1); // 1 here means the relationship to Match : see relationshiplistQuery[1]
        }
        ++iAl;
    }
     */
    if(statFailed){
        //statFailed=this->Qry.dim()-foundAll;
        stat[0]=*statFailed;
    }

    db.AddRecord(eTypeStat,(const void *)stat, sizeof(Stat)*stat.dim());
    db.AddRecordRelationshipCounter(eTypeStat, 1, 1); // 1 here means the relationship to Match : see relationshiplistQuery[1]

    db.AllocRelation();
    /*
    idx iAl;
    for( hdr=rawAlignment, iAl=0 ; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) ) {
        if( selectedOnly && !((hdr->flags())&(sBioseqAlignment::fSelectedAlignment) ) )
            continue;


        match=hdr->match();
        idSub=hdr->idSub();
        idQry=hdr->idQry();


        idx mySubID=db.GetRecordIndexByBody((const void *)&(idSub), eTypeSubject);
        idx myQryID=db.GetRecordIndexByBody((const void *)&(idQry), eTypeQry);
        idx myMatchID=db.GetRecordIndexByBody((const void *)match, eTypeMatch,  sizeof(idx)*hdr->dimAlign() );


        db.AddRelation(eTypeAlignment, 1, iAl+1, myMatchID);
        //db.AddRelation(eTypeAlignment, 2, iAl+1, mySubID);
        //db.AddRelation(eTypeAlignment, 3, iAl+1, myQryID);

        // db.AddRelation(eTypeMatch, eTypeAlignment, iAl+1, iAl+1, 1);

        db.AddRelation(eTypeSubject, 1, mySubID, iAl+1 );

        if(doQueries)
            db.AddRelation(eTypeQry, 1, myQryID, iAl+1 );

        ++iAl;
    }
     */
    idx iAlCnt=0;
    DigestSecondStage(&db, Qry, &iAlCnt, hdr,hdre , selectedOnly  , doQueries);

    db.AddRelation(eTypeStat, 1, 1, 1); // 1 here means the relationship to Match : see relationshiplistQuery[1]

    db.Finalize(combineFile);
    return 1;
}







// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Vioalt information
// _/
// _/_/_/_/_/_/_/_/_/_/_/


struct linkList {
        sVec<idx> list;
        void addNode(idx value, idx index = -1) {
//            short int l = 0;
//            idx selfInd = list.dim();
//
//            if ( value <= 0x3fffffff ) {
//                l = 0;
//            } else {
//                l = 1;
//            }
//
            idx * n = list.add();
//            sConvPtr(n,idx) = 0;
            *n = 0;
            if( index >= 0 ) {
                setLink( index, list.dim()-1 );
            }
            setValue( list.dim()-1, value);
        }

        void appendNode(idx value, idx index = -1) {
            idx * n = list.add();
            *n = 0;
            if( index >= 0 ) {
                index = getLastNodeIndex(index);
                setLink(index, list.dim()-1);
            }
            setValue( list.dim()-1, value);
        }

        inline idx getNextNode(idx i) {
            return list[getLink(i)];
        }

        inline bool isLastNode(idx i) {
            return getLink(i)==0;
        }

        inline void setValue(idx i, idx v) {
            list[i] |= v&0xFFFFFFFFull;
        }
        inline void setLink(idx i, idx l = 0) {
            list[i] |= ((idx)(l-i)<<32);
        }

        inline idx getLink(idx i) {
            idx linkI = (idx)((list[i]&(0xFFFFFFFFull<<32))>>32);
            return linkI? linkI + i:0;
        }

        inline idx getValue(idx i) {
            return list[i]&0xFFFFFFFFull;
        }
        idx getLastNodeIndex(idx index) {
            while( !isLastNode(index) ) {
                index = getLink( index );
            }
            return index;
        }

};

idx sVioal::DigestPairedEnds( sVec<ALFact> &als, sVec<sFil> & fileList, sVioal::digestParams &params, sDic < sBioal::LenHistogram > * lenHistogram, sVec< idx > * totSubjectCoverage ) {
    idx _bioal_long_cnt = Qry->getlongCount();
    sVec< idx > reads_short2long(sMex::fSetZero|sMex::fExactSize);
    reads_short2long.resize(_bioal_long_cnt);

    idx curInd = 0, shortI = 0;

    sVec< idx > reads_ind_short2long(sMex::fSetZero|sMex::fExactSize);
    sVec< idx > reads_cnt_short2long(sMex::fSetZero|sMex::fExactSize);
    reads_ind_short2long.resize(Qry->dim());
    reads_cnt_short2long.resize(Qry->dim());
    idx cnt = 0;
    for(idx i = 0 ; i < Qry->dim() ; ++i ){
        reads_ind_short2long[i]=cnt;
        cnt+=Qry->rpt(i);
        if((i%100000==0) &&  myCallbackFunction){
            if( !myCallbackFunction(myCallbackParam,i,5*i/Qry->dim(),100) )
                return 0;
        }
    }
    for( idx i = 0 ; i < _bioal_long_cnt ; ++i ) {
        shortI = Qry->long2short(i);
        curInd = reads_ind_short2long[shortI] + reads_cnt_short2long[shortI]++;
        reads_short2long[curInd] = i;
        if((i%100000==0) && myCallbackFunction){
            if( !myCallbackFunction(myCallbackParam,i,5+5*i/_bioal_long_cnt,100) )
                return 0;
        }
    }

    reads_cnt_short2long.destroy();

    sDic< idx > long2al;
    linkList lst;
    idx iLongQry = 0, iqry = 0, rptcnt = 0;
    idx * endAl1, * endAl2, * ind1, ind_d = -1;

    for( idx i = 0 ; i < als.dim() ; ++i ) {
        if((i%100000==0) && myCallbackFunction){
            if( !myCallbackFunction(myCallbackParam,i,10+5*i/als.dim(),100) )
                return 0;
        }
        ALFact * al = als.ptr(i);
        al->rpt = 0;
        iqry = al->idQry&(0xFFFFFFFFFFFFFull);
        rptcnt = ((iqry+1 >= reads_ind_short2long.dim())?reads_short2long.dim():reads_ind_short2long[iqry+1]) - reads_ind_short2long[iqry];
        for(idx iq = 0 ; iq < rptcnt ; ++iq ){
            iLongQry = reads_short2long[reads_ind_short2long[iqry] + iq];
            ind1 = long2al.get(&iLongQry,sizeof(iLongQry));
            if( !ind1 ) {
                ind1 = &ind_d;
                *long2al.set(&iLongQry,sizeof(iLongQry)) = lst.list.dim();
            }
            lst.appendNode(i,*ind1);
//            endAl1 = long2al.set(&iLongQry,sizeof(iLongQry));
//            *endAl1->add() = i;
        }
        reads_short2long[iqry];
    }
    reads_ind_short2long.destroy();
    reads_short2long.destroy();

    ALFact * al1 = 0, *al2 = 0;
    sBioseqAlignment::Al * hdr1 = 0, * hdr2 = 0;
    sVec<idx> filteredPairedAls;
    real best1Score = 0, best2Score = 0, end1Score = 0, end2Score = 0;
    idx bestCnt =0 ;
    sVec<idx> subHits(sMex::fSetZero|sMex::fExactSize); idx * pSubHits = 0;
    if( (params.flags&sBioseqAlignment::fAlignKeepMarkovnikovMatch) ) {
        subHits.resize(Sub->dim());
        pSubHits = subHits.ptr();
    }
    idx tot = 0, leftCnt = 0, rightCnt = 0;
    bool onesideonly = false;
    for( idx i1 = 0 ; i1 < _bioal_long_cnt/2 ; ++i1 ) {
        if((i1%10000==0) && myCallbackFunction){
            if( !myCallbackFunction(myCallbackParam,i1,15+5*i1/(_bioal_long_cnt/2),100) )
                return 0;
        }
        idx i2 = _getPairedRead (i1);

        endAl1 = long2al.get(&i1,sizeof(i1));
        endAl2 = long2al.get(&i2,sizeof(i2));

        filteredPairedAls.cut(0);
        best1Score = best2Score = bestCnt =0;

        onesideonly = false;
        if( !( endAl1 && endAl2 ) ) {
            if(params.flags&sBioseqAlignment::fAlignKeepPairedOnly)
                continue;
            if( !endAl1 && endAl2 ) {
                endAl1 = endAl2;
                onesideonly = true;
            } else if( endAl1 && !endAl2 ) {
                onesideonly = true;
            } else {
                continue;
            }
        }
        bool isA1Last = false;
        idx ia1 = *endAl1;
        while( !isA1Last ) {
            al1 = als.ptr(lst.getValue(ia1));
            bool isA2Last = false;
            idx ia2 = onesideonly? 0: *endAl2;
            while ( !isA2Last ) {
                al2 = onesideonly? 0: als.ptr(lst.getValue(ia2));
                hdr1 = _getHdrFromAlFact(al1);
                hdr2 = al2 ? _getHdrFromAlFact(al2):0;
                if( onesideonly || !filterPair(al1, al2, hdr1, hdr2,params) ) {
                    end1Score = 0; end2Score = 0;
                    *filteredPairedAls.add() = lst.getValue(ia1);
                    *filteredPairedAls.add() = onesideonly?-1:lst.getValue(ia2);
                    end1Score = hdr1?hdr1->score():0;
                    end2Score = hdr2?hdr2->score():0;
                    if( _fragmentScoreCompare(end1Score,end2Score,best1Score,best2Score)) {
                        best1Score = end1Score;
                        best2Score = end2Score;
                        bestCnt = 1;
                    } else if ( _fragmentScoreMatch(end1Score,end2Score,best1Score,best2Score) ) {
                        ++bestCnt;
                    }
                    *filteredPairedAls.add() = end1Score;
                    *filteredPairedAls.add() = end2Score;
                }

                if( onesideonly )
                    break;
                isA2Last = lst.isLastNode(ia2);
                ia2 = lst.getLink(ia2);

            }
            isA1Last = lst.isLastNode( ia1 );
            ia1 = lst.getLink(ia1);
        }

        if( !filteredPairedAls.dim() ||
            ((params.flags & sBioseqAlignment::fAlignKeepUniqueMatch) && filteredPairedAls.dim() > 3) ||
            ( (params.flags & sBioseqAlignment::fAlignKeepUniqueBestMatch) && bestCnt > 1 ))
            continue;

        idx bestInd = ((idx)(sRand::ran0(&params.seed)*bestCnt))%bestCnt; bestCnt=0;bool doBreak = false; leftCnt = 0; rightCnt = 0;
        for( idx ip = 0 ; ip < filteredPairedAls.dim() ; ip += 4 ) {
            if(  (params.flags & sBioseqAlignment::fAlignKeepBestFirstMatch) &&  _fragmentScoreMatch(filteredPairedAls[ip+2],filteredPairedAls[ip+3],best1Score,best2Score) ) {
                doBreak = true;
            } else if ( (params.flags & sBioseqAlignment::fAlignKeepAllBestMatches) && !_fragmentScoreMatch(filteredPairedAls[ip+2],filteredPairedAls[ip+3],best1Score,best2Score) ) {
                continue;
            } else if ( (params.flags & sBioseqAlignment::fAlignKeepRandomBestMatch) && ( !_fragmentScoreMatch(filteredPairedAls[ip+2],filteredPairedAls[ip+3],best1Score,best2Score) || ( bestInd != bestCnt++) ) ) {
                continue;
            }
            al1 = als.ptr(filteredPairedAls[ip]);
            al2 = filteredPairedAls[ip+1]>=0?als.ptr(filteredPairedAls[ip+1]):0;

            ++al1->rpt;
            ++leftCnt;
            if( pSubHits ) {
                ++pSubHits[al1->sub];
            }
            if( al2 ) {
                ++al2->rpt;
                if( pSubHits ) {
                    ++pSubHits[al2->sub];
                }
                ++rightCnt;
                //HI bit signifies tha the read is part of aligned fragment;
                al1->idQry |= (idx)1<<((8*sizeof(idx))-1);
                al2->idQry |= (idx)1<<((8*sizeof(idx))-1);
            }
            if( doBreak )
                break;
        }
        if(leftCnt)++tot;
        if(rightCnt)++tot;

    }
    if( pSubHits || lenHistogram || totSubjectCoverage ) {
        idx bestSub = -1, is1 = 0, len = 0;
        bool isHit = false;

        for( idx i1 = 0 ; i1 < _bioal_long_cnt ; ++i1 ) {
            endAl1 = long2al.get(&i1,sizeof(i1));
            is1 = Qry->long2short(i1);
            len = Qry->len(is1);
            if(!endAl1) {
                if( lenHistogram ) {
                    ++(lenHistogram->set(&len,sizeof(len)))->cntFailed;
                }
                continue;
            }
            isHit = false;
            if( pSubHits ) {
                bestSub = -1;
                bool isA1Last = false;
                idx ia1 = *endAl1;
                while ( !isA1Last ) {
                    al1 = als.ptr(lst.getValue(ia1));
                    if ( al1->rpt ) {
                        if( bestSub < 0 || pSubHits[bestSub] < pSubHits[al1->sub]) {
                            bestSub = al1->sub;
                        }
                    }
                    isA1Last = lst.isLastNode(ia1);
                    ia1 = lst.getLink(ia1);
                }
                isA1Last = false;
                ia1 = *endAl1;
                while ( !isA1Last ) {
                    al1 = als.ptr(lst.getValue(ia1));
                    if ( al1->rpt && bestSub != al1->sub) {
                        --al1->rpt;
                    }
                    isA1Last = lst.isLastNode(ia1);
                    ia1 = lst.getLink(ia1);
                }
            }
            bool isA1Last = false;
            idx ia1 = *endAl1;
            while ( !isA1Last ) {
                al1 = als.ptr(lst.getValue(ia1));
                hdr1 = (sBioseqAlignment::Al * )fileList[al1->iFile].ptr( al1->ofsFile );
                if ( al1->rpt ) {
                    isHit = true;
                    _setHistogram(hdr1);
                    _setSubjectCoverage(hdr1, al1);
                }

                isA1Last = lst.isLastNode(ia1);
                ia1 = lst.getLink(ia1);
            }
            if( isHit ) {
                if( lenHistogram )
                    ++(lenHistogram->set(&len,sizeof(len)))->cntRead;
            } else {
                if( lenHistogram )
                    ++(lenHistogram->set(&len,sizeof(len)))->cntFailed;
            }
        }
    }
    return cnt-tot;
}

idx sVioal::__sort_totalAlignmentSorter_onSubject(void * param, void * arr , idx i1, idx i2)
{
    ALFact * a1=((ALFact*)arr+i1);
    ALFact * a2=((ALFact*)arr+i2);

    if( a1->sub > a2->sub )
        return 1;
    else if( a1->sub < a2->sub )
        return -1;
    else if( a1->pos > a2->pos )
        return 1;
    else if( a1->pos < a2->pos )
        return -1;
    else if( a1->iFile > a2->iFile )
        return 1;
    else if( a1->iFile< a2->iFile )
        return -1;
    else if( (((udx)a1->idQry)>>(52)) > (((udx)a2->idQry)>>(52)) )
        return 1;
    else if( (((udx)a1->idQry)>>(52)) < (((udx)a2->idQry)>>(52)) )
        return -1;
    else if( i1 > i2)
        return 1;
    else if( i1 < i2)
        return -1;
    return 0;
}


idx sVioal::__sort_totalAlignmentSorter_onPosition(void * param, void * arr , idx i1, idx i2)
{
    ALFact * a1=((ALFact*)arr+i1);
    ALFact * a2=((ALFact*)arr+i2);

    if( a1->pos > a2->pos )
        return 1;
    else if( a1->pos < a2->pos )
        return -1;
    else if( a1->sub > a2->sub )
        return 1;
    else if( a1->sub < a2->sub )
        return -1;
    else if( a1->iFile > a2->iFile )
        return 1;
    else if( a1->iFile< a2->iFile )
        return -1;
    else if( (((udx)a1->idQry)>>(52)) > (((udx)a2->idQry)>>(52)) )
            return 1;
    else if( (((udx)a1->idQry)>>(52)) < (((udx)a2->idQry)>>(52)) )
        return -1;
    else if( i1 > i2)
        return 1;
    else if( i1 < i2)
        return -1;
    return 0;
}

idx sVioal::__sort_totalAlignmentSorter_onQuery(void * param, void * arr , idx i1, idx i2)
{
    ALFact * a1=((ALFact*)arr+i1);
    ALFact * a2=((ALFact*)arr+i2);

    if( ((a1->idQry)&(0xFFFFFFFFFFFFFull)) > ((a2->idQry)&(0xFFFFFFFFFFFFFull)) )
        return 1;
    else if( ((a1->idQry)&(0xFFFFFFFFFFFFFull)) < ((a2->idQry)&(0xFFFFFFFFFFFFFull)) )
        return -1;
    else if( a1->pos > a2->pos )
        return 1;
    else if( a1->pos < a2->pos )
        return -1;
    else if( a1->iFile > a2->iFile )
        return 1;
    else if( a1->iFile< a2->iFile )
        return -1;
    else if( i1 > i2)
        return 1;
    else if( i1 < i2)
        return -1;
    return 0;
}




idx sVioal::DigestCombineAlignmentsRaw(const char* outputfilename, const char * filenames, digestParams &params, sDic < sBioal::LenHistogram > * lenHistogram, sVec < idx > * totSubjectCoverage, idx sortFlags /*= 0*/)
{
    if( !outputfilename )
        return 0;
    sStr filenames00;
    sString::searchAndReplaceSymbols(&filenames00,filenames,0,"," sString_symbolsBlank,(const char *)0,0,true,true,true,true);
    idx iFile=0 ,iAl , iTot;


    // prepare the list of all alignments in all files for sorting
    if(!alFactors)alFactors=&AlFactors;
    sVec < sFil  > fileList;
    idx cntFiles=sString::cnt00(filenames00);

    sVec < char > HitBits(sMex::fSetZero);HitBits.resize(Qry->dim()/8+1);
    char * hitBits=HitBits.ptr();;

    /*
    sStr concatFileNameForManyRequests;
    if(cntFiles>100) {
        concatFileNameForManyRequests.printf("%s.concatenated.vioalt",filenames00);
        sFile::remove(concatFileNameForManyRequests);
        for( const char * flnm=filenames00; flnm; flnm=sString::next00(flnm)) {
            sFile::copy(concatFileNameForManyRequests,flnm,true);
        }

    }*/
    idx l;
    //udx sMinAl=0xFFFFFFFFFFFFFFFFull, sMaxAl=0;
    udx outOfPlace=0, prvPos=-1;
    idx irand=64;

    sVec<idx> subHits(sMex::fSetZero|sMex::fExactSize);

    if( !(params.flags&sBioseqAlignment::fAlignIsPairedEndMode) && (params.flags&sBioseqAlignment::fAlignKeepMarkovnikovMatch) ) {
        subHits.resize( Sub->dim() );
        bool filteredMatches = sBioseqAlignment::doSelectMatches(params.flags);
        for( const char * flnm=filenames00; flnm; flnm=sString::next00(flnm) ) {
            sFil * fl=fileList.add();
            fl->init(flnm,sMex::fReadonly);
            if(!fl->ok()){
                fileList.cut(fileList.dim()-1);
                continue;
            }

            sBioseqAlignment::Al * hdr=(sBioseqAlignment::Al * )fl->ptr(), * hdre=sShift(hdr,fl->length());
            for(  ; hdr<hdre ; hdr=sShift(hdr,hdr->sizeofFlat()) ) {
                if( filteredMatches && !(hdr->flags()&sBioseqAlignment::fSelectedAlignment) ) {
                    continue;
                }
                subHits[hdr->idSub()] += Qry->rpt( hdr->idQry() );
            }
        }

    }
    idx fileDim = fileList.dim();
    for( const char * flnm=filenames00; flnm; flnm=sString::next00(flnm) , ++iFile)
    {
        sFil * fl= 0;
        if( fileDim ){
            if(iFile>=fileDim)
                break;
            fl = fileList.ptr(iFile);
        } else {
            fl = fileList.add();
            fl->init(flnm,sMex::fReadonly);
        }
        if( !fl->ok() || !fl->length() ){
            --iFile;
            fileList.cut(fileList.dim()-1);
            continue;
        }

        sBioseqAlignment::Al * hdr=(sBioseqAlignment::Al * )fl->ptr(), * hdre=sShift(hdr,fl->length());
        sBioseqAlignment::Al * hdrQry = hdr;
        idx bestSub = hdr->idSub();
        for( iAl=0, iTot=0 ; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) , ++iTot) {
            idx q=hdr->idQry();

            if( !(params.flags&sBioseqAlignment::fAlignIsPairedEndMode) && (params.flags&sBioseqAlignment::fAlignKeepMarkovnikovMatch) ) {
                if( hdrQry <= hdr )  {
                    bestSub = hdr->idSub();
                    hdrQry = sShift(hdr,hdr->sizeofFlat());
                    while ( hdrQry < hdre && hdrQry->idQry() == q ) {
                        if( subHits[bestSub] < subHits[ hdrQry->idSub() ] ) {
                            bestSub = hdrQry->idSub();
                        }
                        hdrQry=sShift(hdrQry,hdrQry->sizeofFlat());
                    }
                }
                if ( hdr->idSub() != bestSub ) {
                    continue;
                }
            }

            if( !(params.flags&sBioseqAlignment::fAlignIsPairedEndMode) && (params.flags & sBioseqAlignment::fAlignKeepBestFirstMatch) && !((hdr->flags()) & (sBioseqAlignment::fSelectedAlignment)) ) {
                continue;
            }

            if( !(params.flags&sBioseqAlignment::fAlignIsPairedEndMode) && (params.flags & sBioseqAlignment::fAlignKeepFirstMatch) && (hitBits[q / 8] & (((char) 1) << (q % 8))) ) {
                continue;
            }

            idx * m=hdr->match();

            ALFact * al = alFactors->add();
            al->pos=hdr->subStart()+m[0];
            al->sub=hdr->idSub();
            al->iFile=iFile;
            al->ofsFile=((const char *)hdr)-((const char * )fl->ptr());
            al->rpt =  Qry->rpt(hdr->idQry());
            udx pos=(((udx)(al->sub))<<32)|(((udx)al->pos));
            al->idQry=(q|(irand<<52));
            irand=(irand+1367)&0x7FF;
            // this one has an alignment
            //if(pos<sMinAl)sMinAl=pos;
            //if(pos>sMaxAl)sMaxAl=pos;
            if(pos<prvPos)
                ++outOfPlace;
            prvPos=pos;


            if(lenHistogram){

                idx llen=Qry->len(hdr->idQry());
                l=llen;
                (lenHistogram->set(&l,sizeof(l)))->cntSeq+=al->rpt;
                l=hdr->lenAlign();
                (lenHistogram->set(&l,sizeof(l)))->cntAl+=al->rpt;
                l=hdr->qryStart()+m[1];
                if(l>1)
                    (lenHistogram->set(&l,sizeof(l)))->cntLeft+=al->rpt;

                l=(l+hdr->lenAlign());
                if(l<llen-1)
                    (lenHistogram->set(&l,sizeof(l)))->cntRight+=al->rpt;

            }
            if(totSubjectCoverage) {
                *totSubjectCoverage->ptrx(al->sub) += (al->rpt * hdr->lenAlign()) ;
            }

            hitBits[q/8]|=((char)1<<(q%8));

            //al->idQry=hdr->idQry();
        }
        //fl->destroy();

        if((iAl%10000==0) &&  myCallbackFunction){
            if( !myCallbackFunction(myCallbackParam,iFile,50*iFile/cntFiles,100) )
                return 0;
        }

    }

    //if(sMinAl!=sMaxAl) {
    sSort::sCallbackSorterSimple __sort_totalAlignmentSorter;

    switch(sortFlags){
        case alSortBySubID:
            __sort_totalAlignmentSorter = (sSort::sCallbackSorterSimple)__sort_totalAlignmentSorter_onSubject;
            break;
        case alSortByQryID:
            __sort_totalAlignmentSorter = (sSort::sCallbackSorterSimple)__sort_totalAlignmentSorter_onQuery;
            break;
        case alSortByPosStart:
            __sort_totalAlignmentSorter = (sSort::sCallbackSorterSimple)__sort_totalAlignmentSorter_onPosition;
            break;
        default:
            __sort_totalAlignmentSorter = (sSort::sCallbackSorterSimple)__sort_totalAlignmentSorter_onSubject;
            break;
    }

    sSort::sortSimpleCallback<ALFact>(__sort_totalAlignmentSorter, 0, alFactors->dim(), alFactors->ptr());

    //}

    idx failed=0, failedRpt=0;

    if( params.flags&sBioseqAlignment::fAlignIsPairedEndMode ) {
        failed = failedRpt = DigestPairedEnds( *alFactors, fileList, params, lenHistogram, totSubjectCoverage);
    } else {
        sVec < idx > rptsOnly;
        for( idx iq=0; iq<Qry->dim() ; ++iq) {

            idx rpt=Qry->rpt(iq);
            l=Qry->len(iq);
            if( lenHistogram ) {
                (lenHistogram->set(&l,sizeof(l)))->cntRead+=rpt;
            }


            if( (iq%10000==0) && myCallbackFunction){
                if( !myCallbackFunction(myCallbackParam,iq,2*iq+1,Qry->dim()))
                    return 0;
            }

            if( hitBits[iq/8] & (((char )1)<<(iq%8)) )
                continue;
            ++failed;
            failedRpt+=rpt;
            if ( lenHistogram ) {
                (lenHistogram->set(&l,sizeof(l)))->cntFailed+=rpt;
            }
        }
    }


    sFile::remove(outputfilename);
    sFil hiveAlBuf(outputfilename);
    sBioseqAlignment::Al * hdr ;

    Qry->destroy(true);

    for( idx posCur=0,iChunk=0; posCur<alFactors->dim() ; posCur+=params.countHiveAlPieces , ++iChunk) {


        sFilePath path(outputfilename,"%%pathx.%" DEC ".vioal",iChunk);sFile::remove(path.ptr(0));
        sVioDB db(0);
        DigestInit(&db,path);

        sVec < Stat > stat(sMex::fSetZero); stat.add(1+((params.flags&sBioseqAlignment::fAlignIsPairedEndMode)?(2*Sub->dim()):Sub->dim()));


        idx ip, countToDo=params.countHiveAlPieces;
        if(alFactors->dim()-(posCur+countToDo)<=params.countHiveAlPieces/4) {// the leftover chunk is less than fourth of the chunk - join those
            countToDo=alFactors->dim()-posCur;
            params.countHiveAlPieces=countToDo;
        }

            for( ip=0; ip<countToDo && posCur+ip<alFactors->dim(); ++ip ){
                if( (params.flags&sBioseqAlignment::fAlignIsPairedEndMode) && !alFactors->ptr(posCur+ip)->rpt ) {
                    continue;
                }
                hdr=(sBioseqAlignment::Al * )fileList[alFactors->ptr(posCur+ip)->iFile].ptr( alFactors->ptr(posCur+ip)->ofsFile );
                DigestFirstStage(&db, hdr, hdr+1, 0, stat, false , false, 0 ,alFactors->ptr(posCur+ip)->rpt);
                if( params.flags&sBioseqAlignment::fAlignIsPairedEndMode )
                    DigestFragmentStage( alFactors->ptr(posCur+ip), alFactors->ptr(posCur+ip)+1,stat.ptr(Sub->dim()+1), 0, 0,alFactors->ptr(posCur+ip)->rpt);
            }
        if( params.flags&sBioseqAlignment::fAlignIsPairedEndMode ) {
            for(idx iss = 0 ; iss < Sub->dim() ; ++iss) {
                stat[Sub->dim()+1+iss].found/=2;
                stat[Sub->dim()+1+iss].foundRpt/=2;
            }
        }
        if(iChunk==0 ){ // && statFailed){
            //stat[0]=*statFailed;
            stat[0].found=failed;
            stat[0].foundRpt=failedRpt;
        }

        db.AddRecord(eTypeStat,(const void *)stat, sizeof(Stat)*stat.dim());
        db.AddRecordRelationshipCounter(eTypeStat, 1, 1); // 1 here means the relationship to Match : see relationshiplistQuery[1]

        db.AllocRelation();


        idx iAlCnt=0, ipCnt = 0;
        for( ip=0; ip<countToDo  && posCur+ip<alFactors->dim(); ++ip ){
            if( (params.flags&sBioseqAlignment::fAlignIsPairedEndMode) && !alFactors->ptr(posCur+ip)->rpt ) {
                continue;
            }
            ++ipCnt;
            hdr=(sBioseqAlignment::Al * )fileList[alFactors->ptr(posCur+ip)->iFile].ptr( alFactors->ptr(posCur+ip)->ofsFile );
            DigestSecondStage(&db, Qry, &iAlCnt, hdr,hdr+1 , false  , false, alFactors->ptr(posCur+ip)->rpt);
        }

        db.AddRelation(eTypeStat, 1, 1, 1); // 1 here means the relationship to Match : see relationshiplistQuery[1]
        setMode(db, Qry->getmode(), Sub->getmode());
        db.Finalize(params.combineFiles);
        //sVioal::setMode(&db,mode);

        path.makeName(outputfilename,"%%flnmx.%" DEC ".vioal",iChunk);
        hiveAlBuf.printf("file://%s,%" DEC ",%" DEC "\n",path.ptr(0),(idx)0,ipCnt);

        if(myCallbackFunction){
            if( !myCallbackFunction(myCallbackParam,iChunk,50+50*(posCur+countToDo)/alFactors->dim(), 100))
                return 0;
        }
    }

    return 1;
}

