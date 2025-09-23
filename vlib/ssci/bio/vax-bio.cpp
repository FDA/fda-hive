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
#include <ssci/bio/vax-bio.hpp>
#include <ion/sIon-core.hpp>






idx sVaxSeq::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{

    return 0;
}





idx sVaxAnnot::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{
    if (_isVCF && cleanHeaderForVCF){
        while (true) {
            if (recNext[0]=='\n') recNext++;
            if (recNext[0]=='#') {
                while(recNext<srcEnd && *recNext!='\n' )
                    ++recNext;
            }
            else break;
        }

        cleanHeaderForVCF = false;
    }

    if(fieldType==0 && internalFieldIterator ==0) {
        if(!ensureRecordBuf())
            return sIon::eProviderDestroy;

        if(startCol==-1 && endCol==-1) {
            pos.s32.start=0;
            pos.s32.end=0xFFFFFFFF;
        } else {
            pos.s32.start=iValue(startCol);
            pos.s32.end=iValue(endCol);
        }
        ++record;
    }
    if(fieldType==0 ) {
        if(seqIDCol==-1) {
            *recordBody=(const void*)"-";
            *recordSize=1;
        }
        else *recordBody=value(seqIDCol,recordSize);
    } else if(fieldType==1 ) {
        *recordBody=&pos.s64;
        *recordSize=sizeof(pos.s64);
        return record;
    } else if(fieldType==4 ) {
        *recordBody=sConvInt2Ptr(sNotIdx,void);
        return record;
    }else {

        if(internalColumnMap==0 ) {
            for( idx col; (col=iteratorCol(internalFieldIterator))<hdrTable.dim() ; internalFieldIterator=col+1){
                if( col!=startCol && col!=endCol && col!=seqIDCol)
                    break;
            }
        }

        internalFieldIterator=nextInternal(internalFieldIterator,fieldType==2,recordBody,recordSize);

        if(fieldType==2) {
            if(nameSubstitutionDictionary && *recordSize >0) {
                idx pNumIndex=-1;
                const char * * foundReplacement = nameSubstitutionDictionary->get(*recordBody,*recordSize,&pNumIndex);
                if (pNumIndex!=-1){
                    *recordBody=*foundReplacement;
                    *recordSize=sLen(*recordBody);
                }
            }
        }

        if(internalFieldIterator==sNotIdx) {
            internalFieldIterator=0;
            if(! (*recordBody))
                return record;
        }

        if(*recordBody) {
            if( skip0 && ((char*)(*recordBody))[0]=='0' && *recordSize==1 && fieldType==3 ) {
                *recordBody=0;
                *recordSize=0;
                return record;
            }
        }

    }

    if(fieldType!=1 && needsCleanup) {
        spaceAndQuoteCleanup(recordBody, recordSize );
    }

    return record;

}






idx sVaxAnnot::rangeSorter(sIon * ion, void * param, sIon::RecordResult * targets1, sIon::RecordResult * targets2)
{
    idx res=0;sIon_bodyCmp(res,targets1,targets2);

    if(res)
        return res;

    return ( *((int*)targets1[1].body)-*((int*)targets2[1].body) );
}




idx sVaxAnnotGB::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{
    #define isThisField(  _v_keyword , _v_sz) ( recNext<srcEnd-(lenWord=(sLen(_v_keyword)-(v_sz=_v_sz))) && strncmp(recNext,_v_keyword,lenWord+(_v_sz))==0 )

    if(recNext>=srcEnd)
        return sIon::eProviderDestroy;

    idx lenWord = 0, it, v_sz=0;
    const char * featureWord="FEATURES";
    const char * listOfFeatures[]={ "     source","     gene","     CDS","     mat_peptide","     misc_feature"};
    const char * listOfTypes[]={ "                     /organism=","                     /organelle=","                     /gene=","                     /codon_start=","                     /note=","                     /product=","                     /protein_id=","                     /db_xref=","                     /locus_tag=","                     /transl_table="};
    const char * listOfTypesFromHeader[] ={"VERSION     ","ACCESSION   "};

    bool needToLookForFeature = false;
    if( ofsetsOfFieldTypes[fieldType]==sNotIdx ){
        for ( ;recNext<srcEnd; ++recNext) {

            if (pleaseAddFeature || continuousRanges || pleaseAddStrand){
                ofsetsOfFieldTypes[fieldType]=1;
                break;
            }
            if (isThisField("//",0)){
                skipToNextLocus = true;
            }
            if (skipToNextLocus){
                ofsetsOfFieldTypes[2]=recNext-srcStart;
                ofsetsOfFieldTypes[3]=ofsetsOfFieldTypes[2]+2;
                break;
            }

            if( isThisField("\nLOCUS",1) || isThisField("LOCUS",0) ) {
                ofsetsOfFieldTypes[0]=recNext-srcStart+lenWord+v_sz;
                ofsetsOfFieldTypes[1]=sNotIdx;
                ofsetsOfFieldTypes[2]=sNotIdx;
                ofsetsOfFieldTypes[3]=sNotIdx;
                fromHeader=false;
            }
            if(ofsetsOfFieldTypes[fieldType]!=sNotIdx )
                break;

            if(!needToLookForFeature){
                for ( it=0; it<sDim(listOfTypes) ; ++it ) {
                    if( isThisField(listOfTypes[it],21) ) {
                       ofsetsOfFieldTypes[2]=recNext-srcStart+21;
                       ofsetsOfFieldTypes[3]=ofsetsOfFieldTypes[2]+lenWord;
                       break;
                    }
                }
            }
            for ( it=0; it<sDim(listOfTypesFromHeader) ; ++it ) {
                idx lenType=sLen(listOfTypesFromHeader[it]);
                if( strncmp(recNext,listOfTypesFromHeader[it],lenType)==0){
                    ofsetsOfFieldTypes[2]=recNext-srcStart+lenType;
                    ofsetsOfFieldTypes[3]=ofsetsOfFieldTypes[2];
                    ofsetsOfFieldTypes[1]=ofsetsOfFieldTypes[0] + 7;
                    fromHeader=true;
                    headerMatchNumber=it;
                    break;
                }
            }
            if(ofsetsOfFieldTypes[fieldType]!=sNotIdx )
                break;

            bool foundFt = false;
            for ( it=0; it<sDim(listOfFeatures) ; ++it ) {
                if( isThisField(listOfFeatures[it],5) ) {
                   ofsetsOfFieldTypes[3]=recNext-srcStart+5;
                   ofsetsOfFieldTypes[2]=featureWord - srcStart;
                   ofsetsOfFieldTypes[1]=ofsetsOfFieldTypes[3]+lenWord;
                   fromFeature = true;
                   foundFt = true;
                   break;
                }
            }

            if ( !foundFt &&strncmp("     ",recNext,5)==0 && recNext[6]!=' ' ){
                ofsetsOfFieldTypes[1]=sNotIdx;
                ofsetsOfFieldTypes[2]=sNotIdx;
                ofsetsOfFieldTypes[3]=sNotIdx;
                fromFeature = false;
                needToLookForFeature=true;
            }

            if(ofsetsOfFieldTypes[fieldType]!=sNotIdx ){
                break;
            }

            while(recNext<srcEnd && *recNext!='\n' )
                ++recNext;

        }

        if (!pleaseAddFeature && !continuousRanges && !pleaseAddStrand){
            if(ofsetsOfFieldTypes[fieldType]==sNotIdx)
                return sIon::eProviderDestroy;


            while(recNext<srcEnd && *recNext!='\n' ) {
                ++recNext;
            }
            ++record;
        }
    }


    const char * body=ofsetsOfFieldTypes[fieldType]+srcStart;
    while(body<srcEnd && strchr(sString_symbolsBlank,*body) ){
        ++body;
    }



    if(fieldType==0 ) {
        *recordSize=0;
        while(body+(*recordSize)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize])==0 ){
            ++(*recordSize);
        }
        *recordBody=body;
        const char * sequenceLength = body + (*recordSize);
        idx a = 0;
        while(sequenceLength<srcEnd && strchr(sString_symbolsBlank,*sequenceLength) ){
            ++sequenceLength;
        }
        while(sequenceLength<srcEnd && strchr(sString_symbolsBlank,sequenceLength[a])==0 ){
            ++(a);
        }
        defaultLength = atoi(sequenceLength);
        while (body && body<srcEnd && strncmp(body,listOfTypesFromHeader[0], sLen(listOfTypesFromHeader[0]))!=0 ) {
            ++body;
        }
        body=body+sLen(listOfTypesFromHeader[0]);
        while(body<srcEnd && strchr(sString_symbolsBlank,*body) ){
            ++body;
        }
        *recordSize=0;
        while(body+(*recordSize)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize])==0 ){
            ++(*recordSize);
        }
        *recordBody=body;
        return record;
    }
    else if(fieldType==1 ) {
        if (fromHeader){
            pos.s32.start=1;
            pos.s32.end=defaultLength;
            *recordBody=&pos.s32;
            *recordSize=sizeof(pos.s32);
            return record;
        }
        scanUntilNextLine(reserveBuf,body,recordSize,srcEnd);
        sVec < startEnd > myRangeVec;
        bool isSet = false;
        bool checkJoin = (strncmp("join",reserveBuf.ptr(0),4)==0) ? true : false;
        bool checkOrder = (strncmp("order",reserveBuf.ptr(0),5)==0) ? true : false;
        forward=true;
        if (checkJoin || checkOrder){
            if (checkOrder){
                parseJoinOrOrder(reserveBuf.ptr(0), myRangeVec,"order(");
            }
            else parseJoinOrOrder(reserveBuf.ptr(0), myRangeVec);
            if (myRangeVec.dim()) {
                pos.s32.start= myRangeVec[0].start;
                pos.s32.end= myRangeVec[myRangeVec.dim()-1].end;
                *recordBody=&pos.s32;
                *recordSize=sizeof(pos.s32);
                isSet = true;
                forward = myRangeVec[0].forward;
            }
        }
        else {
            bool checkComplement = (strncmp("complement",reserveBuf.ptr(0),10)==0) ? true : false;
            if (checkComplement){
                parseComplement(reserveBuf.ptr(0), myRangeVec);
                if (myRangeVec.dim()) {
                    pos.s32.start= myRangeVec[0].start;
                    pos.s32.end= myRangeVec[myRangeVec.dim()-1].end;
                    *recordBody=&pos.s32;
                    *recordSize=sizeof(pos.s32);
                    isSet = true;
                    forward=false;
                }
            }
            else {
                char * nxt;
                if(*body=='>' || *body=='<')++body;
                pos.s32.start=strtoidx(body,&nxt,10);
                if (strncmp(nxt,"..",2)==0){
                    body=nxt+2;
                    if(*body=='>' || *body=='<' )++body;
                    pos.s32.end=strtoidx(body,&nxt,10);
                }
                else pos.s32.end = pos.s32.start;
                *recordBody=&pos.s32;
                *recordSize=sizeof(pos.s32);
                isSet = true;
            }
        }
        if (myRangeVec.dim() > 1 && (recordNum != previousRecordNum)){
            rangeListForJoinAndComplement.cut(0);
            for (idx i=0; i<myRangeVec.dim(); ++i){
                if (i) rangeListForJoinAndComplement.printf(";");
                rangeListForJoinAndComplement.printf("%" DEC "-%" DEC "", myRangeVec.ptr(i)->start, myRangeVec.ptr(i)->end);
            }
            continuousRanges = true;
        }
        if (!isSet) {
            pos.s32.start=0;
            pos.s32.end=0xFFFFFFFF;
            *recordBody=&pos.s32;
            *recordSize=sizeof(pos.s32);
        }
        return record;
    } else if(fieldType==2 ) {

        if (fromFeature){
            *recordBody=0;
        } else if (pleaseAddFeature){
            *recordBody=featureWord;
            *recordSize=sLen(featureWord);
            previousRecordNum=recordNum;
            recordNum++;
        } else if (pleaseAddStrand){
            *recordBody=strandWord;
            *recordSize=sLen(strandWord);
        } else if(fromHeader){
            idx lenT=0;
            while(lenT<sLen(listOfTypesFromHeader[headerMatchNumber]) && strchr(sString_symbolsBlank,listOfTypesFromHeader[headerMatchNumber][lenT])==0 ){
                ++(lenT);
            }
            *recordBody=listOfTypesFromHeader[headerMatchNumber];
            *recordSize=lenT;
            previousRecordNum=recordNum;
        } else if (continuousRanges) {
            *recordBody="listOfConnectedRanges";
            *recordSize=21;
        } else if (skipToNextLocus){
            *recordBody=0;
        } else {
            *recordBody=body+1;
            *recordSize=lenWord-2;
        }
        ofsetsOfFieldTypes[2]=sNotIdx;
        return record;
    } else if(fieldType==3 ) {
        *recordSize=0;
        if (fromFeature){
            while(body+(*recordSize)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize])==0 ){
                ++(*recordSize);
            }
            *recordBody=0;
            curFeatureToAdd.cut(0);
            curFeatureToAdd.addString(body,*recordSize);
            pleaseAddFeature=true;
            fromFeature=false;
            pleaseAddStrand=true;
            ofsetsOfFieldTypes[2]=sNotIdx;
        } else if (pleaseAddFeature){
            *recordBody = curFeatureToAdd.ptr(0);
            *recordSize = curFeatureToAdd.length();
            pleaseAddFeature=0;
        } else if (pleaseAddStrand){
            *recordBody = forward ? "+" : "-";
            *recordSize = 1;
            pleaseAddStrand=0;
        } else if (continuousRanges){
            *recordBody = rangeListForJoinAndComplement.ptr(0);
            *recordSize = rangeListForJoinAndComplement.length();
            previousRecordNum=recordNum;
            continuousRanges=0;
        } else if (fromHeader){
            scanUntilNextLine(reserveBuf,body,recordSize,srcEnd);
            *recordBody = reserveBuf.ptr(0);
            *recordSize = reserveBuf.length();
        } else if (skipToNextLocus){
            *recordBody=0;
        }
        else {
            scanUntilNextLine(reserveBuf,body,recordSize,srcEnd);
            *recordBody = reserveBuf.ptr(0);
            *recordSize = reserveBuf.length();

            if(fieldType==3 && needsCleanup) {
                spaceAndQuoteCleanup(recordBody, recordSize );
            }
        }
        return record;
    } else if(fieldType==4 ) {
        if (skipToNextLocus){
            *recordBody=0;
            skipToNextLocus=false;
            ofsetsOfFieldTypes[0]=sNotIdx;
        }
        else {
            *recordBody = &recordNum;
            *recordSize = sizeof(recordNum);
        }
        if (fromHeader){
            ofsetsOfFieldTypes[1]=sNotIdx;
            fromHeader=false;
        }
        return record;
    }

    return record;

}




void sVaxAnnotGB::scanUntilNextLine (sStr & dest, const char * body, idx * recordSize, const char * srcEnd){

    dest.cut(0);
    while (body+(*recordSize)<srcEnd){
        while(body+(*recordSize)<srcEnd && strchr(sString_symbolsEndline,body[*recordSize])==0 ){
            ++(*recordSize);
        }
        idx i =1;
        while((body+(*recordSize) + i)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize+i]) ){
            ++i;
        }
        dest.addString(body,*recordSize);
        if (i<10) {
            return;
        }

        if (strchr("/",body[*recordSize+i]) ) {
            return;
        }
        body=sShift(body,*recordSize + i);
        *recordSize=0;
    }
    return;
}

idx sVaxAnnotGB::strchrFreq (const char * input, const char * srch, idx & frq){
    frq =0;
    idx p =0, start =0;
    idx lastpos = sLen(input);
    idx lenSrch = sLen(srch);
    char * myString = (char *)input;
    while (*myString && p < lastpos){
        if (strncmp(myString,srch,lenSrch)==0){
            frq += 1;
            if (frq == 1) start = p;
        }
        p++;
        myString = myString + 1;
    }
    return start;
}

char * sVaxAnnotGB::extractContent (const char * input, sStr & dst, const char * startMarkup, const char * endMarkup){
    idx lenStart = sLen(startMarkup);
    idx lenString = sLen(input);
    idx lenEnd = sLen(endMarkup);
    const char * start;

    if (strncmp(startMarkup,input,lenStart)==0){
        start = input + lenStart;
        const char * moveToEnd = input + (lenString - lenEnd);
        if (strncmp(moveToEnd,endMarkup,lenEnd)==0){
                dst.cut(0);
                dst.add(start,lenString-lenStart-lenEnd);
                dst.add0();
        }
        else {
            dst.printf(0,"%s",start);
        }
    }
    else {
        start = input;
        dst.printf(0,"%s",input);
    }
    return (char *)start;
}

void sVaxAnnotGB::extractLocation(const char * location, startEnd & locationExtracted, bool forward){
    idx freq = 0;
    bool oneExactBase = false, oneBaseBetween = false, isRange = false;
    locationExtracted.forward = forward;
    const char * sep;
    strchrFreq(location,"^", freq);

    if (freq==1) {
        oneExactBase = true;
        sep = "^";
        locationExtracted.oneSiteBetween = true;
        locationExtracted.exactEnd = locationExtracted.exactStart = false;
    }
    idx ss = strchrFreq(location,":", freq);
    if (freq==1) {
        sString::copyUntil(&locationExtracted.buf,location,ss,":");
        location = location + ss +1;
        sep = "..";
    }
    strchrFreq(location,".", freq);
    if (freq==1) {
        oneBaseBetween = true;
        sep = ".";
        locationExtracted.oneBaseBetween = true;
        locationExtracted.exactEnd = locationExtracted.exactStart = false;
    }
    if (freq ==2) {
        isRange = true;
        sep = "..";
    }
    if (oneExactBase == false && oneBaseBetween == false && isRange == false) {
        idx position;
        sscanf(location, "%" DEC "", &position);
        locationExtracted.start = locationExtracted.end = locationExtracted.max =position;

    }
    else {
        sStr buf;
        sString::searchAndReplaceSymbols(&buf,location,0,sep,0,0,true,true,false,true);
        idx cnt = sString::cnt00(buf);
        idx start, end =0;
        if (cnt==2) {
            char * startRaw = sString::next00(buf,0);
            char * endRaw = sString::next00(buf,1);
            if ((strncmp("<",startRaw,1)==0) || (strncmp(">",startRaw,1)==0)) {
                startRaw = startRaw+1;
                locationExtracted.exactStart = false;
            }
            if ((strncmp(">",endRaw,1)==0) || (strncmp("<",endRaw,1)==0)) {
                endRaw = endRaw +1;
                locationExtracted.exactEnd = false;
            }
            sscanf(startRaw, "%" DEC "", &start);
            sscanf(endRaw, "%" DEC "", &end);
            locationExtracted.start = start;
            locationExtracted.end = locationExtracted.max =end;
        }
    }
}


void sVaxAnnotGB::parseJoinOrOrder(const char * textRaw, sVec < startEnd > & startEndOut, const char * toCompare){
    sStr joinTagRemoved;
    extractContent(textRaw,joinTagRemoved , toCompare, ")");

    sStr textRawSplit;
    sString::searchAndReplaceSymbols(&textRawSplit,joinTagRemoved.ptr(0),0,",",0,0,true,true,false,true);
    for (const char * cmp = textRawSplit; cmp; cmp = sString::next00(cmp)){
        bool isComplement = (strncmp("complement",cmp,10)==0) ? true : false;
        startEnd * myStartEnd = startEndOut.add();
        if (isComplement){
            myStartEnd->forward=false;
            sStr text; text.cut(0);
            sString::cleanMarkup(&text,cmp,0,"complement(" _,")" _,0,0,true,false,false);
            extractLocation(text.ptr(1),*myStartEnd,false);
        }
        else  {
            extractLocation(cmp,*myStartEnd,true);
        }
    }
}


void sVaxAnnotGB::parseComplement(const char * textRaw, sVec < startEnd > & startEndOut){
    sStr complementTagRemoved;
    extractContent(textRaw, complementTagRemoved,"complement(",")");

    bool isJoin = (strncmp("join",complementTagRemoved,4)==0) ? true : false;
    bool isOrder = (strncmp("order",complementTagRemoved,5)==0) ? true : false;
    if (isJoin){
        sVec < startEnd > joinSection;
        parseJoinOrOrder(complementTagRemoved,startEndOut);
    }
    else if (isOrder){
        sVec < startEnd > joinSection;
        parseJoinOrOrder(complementTagRemoved,startEndOut,"order(");
    }
    else {
        sStr textRawSplit;
        sString::searchAndReplaceSymbols(&textRawSplit,complementTagRemoved.ptr(0),0,",",0,0,true,true,false,true);
        for (const char * cmp = textRawSplit; cmp; cmp = sString::next00(cmp)){
            startEnd * myStartEnd = startEndOut.add();
            extractLocation(cmp,*myStartEnd,true);
            myStartEnd->forward = false;
        }

    }

}
