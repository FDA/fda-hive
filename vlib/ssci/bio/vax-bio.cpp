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




// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Biological relevant data parsing classes
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


idx sVaxSeq::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{

/*
    if( fieldType==0 ) { // every time we move to a new record
        if(!ensureRecordBuf())
            return sIon::eProviderDestroy;
        ++record;
    }
    if(fieldType==0 ) { // sequenceIDs
        if(seqIDCol==-1) {
            *recordBody=(const void*)"-";
            *recordSize=1;
        }
        else *recordBody=value(seqIDCol,recordSize);

    } else if(fieldType==1 ) { // every time we move to a new record
        *recordBody=&pos.s64;
        *recordSize=sizeof(pos.s64);
        return record;
    } else if(fieldType==4 ) { // every time we move to a new record
        *recordBody=sConvInt2Ptr(sNotIdx,void);
        // *recordSize=sizeof(idx);
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

        if( ((char*)(*recordBody))[0]=='0' && *recordSize==1 && fieldType==3 && skip0 ) {
            *recordBody=0;
            *recordSize=0;
            return record;
        }

    }

    if(fieldType!=1 && needsCleanup) {
        spaceAndQuoteCleanup(recordBody, recordSize );
    }

    return record;

*/
    return 0;
}


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  ion Loader for basic VAX style Annotation
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/



idx sVaxAnnot::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{
    if (_isVCF && cleanHeaderForVCF){
        // skip lines start with ##
        // use line starts with # as header
        while (true) {
            if (recNext[0]=='\n') recNext++;
            if (recNext[0]=='#') {
                while(recNext<srcEnd && *recNext!='\n' ) // position to the next line
                    ++recNext;
            }
            else break;
        }

        cleanHeaderForVCF = false;
    }

    if(fieldType==0 && internalFieldIterator ==0) { // every time we move to a new record
        if(!ensureRecordBuf())
            return sIon::eProviderDestroy;
        if(startCol==-1 && endCol==-1) {
            pos.s32.start=0;
            pos.s32.end=0xFFFFFFFF; // 0
        } else {
            pos.s32.start=iValue(startCol);
            pos.s32.end=iValue(endCol);
        }
        ++record;
    }
    if(fieldType==0 ) { // sequenceIDs
        if(seqIDCol==-1) {
            *recordBody=(const void*)"-";
            *recordSize=1;
        }
        else *recordBody=value(seqIDCol,recordSize);
    } else if(fieldType==1 ) { // every time we move to a new record
        *recordBody=&pos.s64;
        *recordSize=sizeof(pos.s64);
        return record;
    } else if(fieldType==4 ) { // every time we move to a new record
        *recordBody=sConvInt2Ptr(sNotIdx,void);
        // *recordSize=sizeof(idx);
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




// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Annotation sorter and virtual tree on ranges builder
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


idx sVaxAnnot::rangeSorter(sIon * ion, void * param, sIon::RecordResult * targets1, sIon::RecordResult * targets2)
{
    idx res=0;sIon_bodyCmp(res,targets1,targets2);

    if(res)
        return res;

    return ( *((int*)targets1[1].body)-*((int*)targets2[1].body) );
}



// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  ion Loader for GenBank file
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx sVaxAnnotGB::ionProviderCallback(idx record, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{
    #define isThisField(  _v_keyword , _v_sz) ( recNext<srcEnd-(lenWord=(sLen(_v_keyword)-(v_sz=_v_sz))) && strncmp(recNext,_v_keyword,lenWord+(_v_sz))==0 )

    if(recNext>=srcEnd)
        return sIon::eProviderDestroy;

    idx lenWord, it, v_sz=0;
    const char * featureWord="FEATURES";
    const char * listOfFeatures[]={ "     source","     gene","     CDS","     mat_peptide","     misc_feature"};
    const char * listOfTypes[]={ "                     /organism=","                     /organelle=","                     /gene=","                     /codon_start=","                     /note=","                     /product=","                     /protein_id=","                     /db_xref=","                     /locus_tag=","                     /transl_table="};
    const char * listOfTypesFromHeader[] ={"VERSION     ","ACCESSION   "};
    //const char * locusSplitter = "//";

    // recNext is the moving pointer through the end of the file
    bool needToLookForFeature = false;
    if( ofsetsOfFieldTypes[fieldType]==sNotIdx ){
        for ( ;recNext<srcEnd; ++recNext) {

            // from Vahan: Lam the word CDS may occur also in a comment section
            //while(recNext<srcEnd && strchr(sString_symbolsBlank,*recNext) ) // position to the next non space chacter
            //    ++recNext;
            if (pleaseAddFeature || continuousRanges || pleaseAddStrand){
                ofsetsOfFieldTypes[fieldType]=1; // fake position in order to satisfy the condition after break
                break;
            }
            if (isThisField("//",0)){ // This symbol is the separator between two LOCUS
                skipToNextLocus = true;
            }
            if (skipToNextLocus){
                ofsetsOfFieldTypes[2]=recNext-srcStart;
                ofsetsOfFieldTypes[3]=ofsetsOfFieldTypes[2]+2;
                break;
            }

            if( isThisField("\nLOCUS",1) || isThisField("LOCUS",0) ) { // matching the recNext with the word LOCUS
                ofsetsOfFieldTypes[0]=recNext-srcStart+lenWord+v_sz;
                ofsetsOfFieldTypes[1]=sNotIdx; // locus invalidates field off all previous findinigs
                ofsetsOfFieldTypes[2]=sNotIdx;
                ofsetsOfFieldTypes[3]=sNotIdx;
                fromHeader=false;
            }
            if(ofsetsOfFieldTypes[fieldType]!=sNotIdx )
                break;

            // look if this is one of the known types: db_xref, locus_tag, protein_id ...
            if(!needToLookForFeature){
                for ( it=0; it<sDim(listOfTypes) ; ++it ) {
                    if( isThisField(listOfTypes[it],21) ) {
                       ofsetsOfFieldTypes[2]=recNext-srcStart+21;
                       ofsetsOfFieldTypes[3]=ofsetsOfFieldTypes[2]+lenWord; // for types next word is the id
                       break;
                    }
                }
            }
            // the types from header:
            // => the ranges (positions) takes the whole length of the sequence: 0 to -1
            for ( it=0; it<sDim(listOfTypesFromHeader) ; ++it ) {
                idx lenType=sLen(listOfTypesFromHeader[it]);
                if( strncmp(recNext,listOfTypesFromHeader[it],lenType)==0){
                    ofsetsOfFieldTypes[2]=recNext-srcStart+lenType;
                    ofsetsOfFieldTypes[3]=ofsetsOfFieldTypes[2]; // for types next word is the id
                    ofsetsOfFieldTypes[1]=ofsetsOfFieldTypes[0] + 7; // LOCUS.......
                    fromHeader=true;
                    headerMatchNumber=it;
                    break;
                }
            }
            if(ofsetsOfFieldTypes[fieldType]!=sNotIdx )
                break;

            // look if this is one of the FEATURES ids: source, gene, misc_feature, mat_peptide ...
            bool foundFt = false;
            for ( it=0; it<sDim(listOfFeatures) ; ++it ) {
                if( isThisField(listOfFeatures[it],5) ) {
                   ofsetsOfFieldTypes[3]=recNext-srcStart+5;
                   ofsetsOfFieldTypes[2]=featureWord - srcStart;
                   ofsetsOfFieldTypes[1]=ofsetsOfFieldTypes[3]+lenWord; // for non-slashed ones next word is the position
                   fromFeature = true;
                   foundFt = true;
                   break;
                }
            }

            if ( !foundFt &&strncmp("     ",recNext,5)==0 && recNext[6]!=' ' ){
                ofsetsOfFieldTypes[1]=sNotIdx; // locus invalidates field off all previous findinigs
                ofsetsOfFieldTypes[2]=sNotIdx;
                ofsetsOfFieldTypes[3]=sNotIdx;
                fromFeature = false;
                needToLookForFeature=true;
            }

            if(ofsetsOfFieldTypes[fieldType]!=sNotIdx ){
                break; // when found, escape the loop in order to add recordBody, recordSize
            }

            while(recNext<srcEnd && *recNext!='\n' ) // position to the next line
                ++recNext;

        }

        if (!pleaseAddFeature && !continuousRanges && !pleaseAddStrand){
            if(ofsetsOfFieldTypes[fieldType]==sNotIdx)
                return sIon::eProviderDestroy;


            while(recNext<srcEnd && *recNext!='\n' ) {// position to the next line
                ++recNext;
            }
            ++record;
        }
    }


    const char * body=ofsetsOfFieldTypes[fieldType]+srcStart;
    while(body<srcEnd && strchr(sString_symbolsBlank,*body) ){ // skipping blank space
        ++body;
    }



    // clean your body pointer into actual data
    if(fieldType==0 ) { // sequenceIDs
        *recordSize=0;
        while(body+(*recordSize)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize])==0 ){ // read until blank space
            ++(*recordSize);
        }
        *recordBody=body;
        const char * sequenceLength = body + (*recordSize);
        idx a = 0;
        while(sequenceLength<srcEnd && strchr(sString_symbolsBlank,*sequenceLength) ){ // skipping blank space
            ++sequenceLength;
        }
        while(sequenceLength<srcEnd && strchr(sString_symbolsBlank,sequenceLength[a])==0 ){ // read until blank space
            ++(a);
        }
        defaultLength = atoi(sequenceLength);
        return record;
    }
    else if(fieldType==1 ) {
        // provide the position
        if (fromHeader){
            pos.s32.start=1;
            //pos.s32.end=0xFFFFFFFF; //0
            pos.s32.end=defaultLength; //0
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
                    body=nxt+2; // start..end
                    if(*body=='>' || *body=='<' )++body;
                    pos.s32.end=strtoidx(body,&nxt,10);
                }
                else pos.s32.end = pos.s32.start;
                *recordBody=&pos.s32;
                *recordSize=sizeof(pos.s32);
                isSet = true;
            }
        }
        if (myRangeVec.dim() > 1 && (recordNum != previousRecordNum)){ // try to keep the list of ranges
            rangeListForJoinAndComplement.cut(0);
            for (idx i=0; i<myRangeVec.dim(); ++i){
                if (i) rangeListForJoinAndComplement.printf(";");
                rangeListForJoinAndComplement.printf("%"DEC"-%"DEC"", myRangeVec.ptr(i)->start, myRangeVec.ptr(i)->end);
            }
            continuousRanges = true;
        }
        if (!isSet) {
            pos.s32.start=0;
            pos.s32.end=0xFFFFFFFF; //0
            *recordBody=&pos.s32;
            *recordSize=sizeof(pos.s32);
        }
        return record;
    } else if(fieldType==2 ) {
        // type

        if (fromFeature){ // dont put the record and relation yet, because the range is still from the previous one
            *recordBody=0;
        } else if (pleaseAddFeature){
            *recordBody=featureWord;
            *recordSize=sLen(featureWord);
            previousRecordNum=recordNum;
            recordNum++;
        } else if (pleaseAddStrand){
            *recordBody=strandWord;
            *recordSize=sLen(strandWord);
        } else if(fromHeader){ // for now, we try to extract the gi for the whole locus which is in the header
            if (headerMatchNumber==1){
                *recordBody="accession";
                *recordSize=9;
            }
            else {
                *recordBody="gi";
                *recordSize=2;
            }
            previousRecordNum=recordNum;
            //recordNum++;
        } else if (continuousRanges) { // add the Special TYPE name for connectedRanges
            *recordBody="listOfConnectedRanges";
            *recordSize=21;
        } else if (skipToNextLocus){ // when the end of one LOCUS is reached, dont put anything
            *recordBody=0;
        } else {
            *recordBody=body+1;
            *recordSize=lenWord-2;
        }
        ofsetsOfFieldTypes[2]=sNotIdx; // ids invalidate themselves not be be reused next time
        return record;
    } else if(fieldType==3 ) {
        *recordSize=0;
        if (fromFeature){ // try to get the Feature Id, and put into a buffer which is going to be used in the next loop
            while(body+(*recordSize)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize])==0 ){ // read until blank space
                ++(*recordSize);
            }
            *recordBody=0;
            curFeatureToAdd.cut(0);
            curFeatureToAdd.addString(body,*recordSize); // keep current Feature to the buffer
            pleaseAddFeature=true;
            fromFeature=false;
            pleaseAddStrand=true;
            ofsetsOfFieldTypes[2]=sNotIdx;
        } else if (pleaseAddFeature){ // actually put the Feature Id
            *recordBody = curFeatureToAdd.ptr(0);
            *recordSize = curFeatureToAdd.length();
            pleaseAddFeature=0;
        } else if (pleaseAddStrand){
            *recordBody = forward ? "+" : "-";
            *recordSize = 1;
            pleaseAddStrand=0;
        } else if (continuousRanges){ // actually put the Feature Id
            *recordBody = rangeListForJoinAndComplement.ptr(0);
            *recordSize = rangeListForJoinAndComplement.length();
            previousRecordNum=recordNum;
            continuousRanges=0;
        } else if (fromHeader){ // for now, just put the Gi number
            char * foundGi=sString::searchSubstring(body,0,"GI:",1,"\n"_"\r\n"__,true); // sString::searchSubstring( const char * src, idx lenSrc, const char * find00,idx occurence, const char * stopFind00,bool isCaseInSensitive) // ,idx lenSrc
            if (foundGi && foundGi[0] != '\n' && foundGi[0] !='\r'){
                body=body+(foundGi-body)+3;
            }
            scanUntilNextLine(reserveBuf,body,recordSize,srcEnd);
            *recordBody = reserveBuf.ptr(0);
            *recordSize = reserveBuf.length();
        } else if (skipToNextLocus){
            *recordBody=0; // because the end of one LOCUS is reached, dont put anything
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
            *recordBody=0; // because the end of one LOCUS is reached, dont put anything
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



// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Miscellaneous utility functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void sVaxAnnotGB::scanUntilNextLine (sStr & dest, const char * body, idx * recordSize, const char * srcEnd){

    dest.cut(0);
    while (body+(*recordSize)<srcEnd){
        while(body+(*recordSize)<srcEnd && strchr(sString_symbolsEndline,body[*recordSize])==0 ){ // read until end of Line
            ++(*recordSize);
        }
        idx i =1;
        while((body+(*recordSize) + i)<srcEnd && strchr(sString_symbolsBlank,body[*recordSize+i]) ){ // skip blank space
            ++i;
        }
        dest.addString(body,*recordSize);
        if (i<10) {  // it is not the continuous of the previous line
            return;
        }

        if (strchr("/",body[*recordSize+i]) ) { // it is not the continuous of the previous line
            return;
        }
        body=sShift(body,*recordSize + i);
        *recordSize=0;
    }
    return;
}
// range treatment for Genbank

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

// 467
// 340..565
// <345..500
// 1..>888
// 102.110
// 123^124
// J00194.1:100..202
void sVaxAnnotGB::extractLocation(const char * location, startEnd & locationExtracted, bool forward){
    idx freq = 0;
    bool oneExactBase = false, oneBaseBetween = false, isRange = false;//, isExternal = false;
    locationExtracted.forward = forward;
    const char * sep;
    strchrFreq(location,"^", freq);

    if (freq==1) { // 123^124
        oneExactBase = true;
        sep = "^";
        locationExtracted.oneSiteBetween = true;
        locationExtracted.exactEnd = locationExtracted.exactStart = false;
    }
    idx ss = strchrFreq(location,":", freq);
    if (freq==1) { // J00194.1:100..202
        //isExternal = true;
        sString::copyUntil(&locationExtracted.buf,location,ss,":");
        location = location + ss +1;
        sep = "..";
    }
    strchrFreq(location,".", freq);
    if (freq==1) { // 102.110
        oneBaseBetween = true;
        sep = ".";
        locationExtracted.oneBaseBetween = true;
        locationExtracted.exactEnd = locationExtracted.exactStart = false;
    }
    if (freq ==2) { // 340..565 or <345..500 or 1..>888
        isRange = true;
        sep = "..";
    }
    if (oneExactBase == false && oneBaseBetween == false && isRange == false) {
        idx position;
        sscanf(location, "%"DEC"", &position);
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
            if (strncmp("<",startRaw,1)==0) {
                startRaw = startRaw+1;
                locationExtracted.exactStart = false;
            }
            if (strncmp(">",endRaw,1)==0) {
                endRaw = endRaw +1;
                locationExtracted.exactEnd = false;
            }
            sscanf(startRaw, "%"DEC"", &start);
            sscanf(endRaw, "%"DEC"", &end);
            locationExtracted.start = start;
            locationExtracted.end = locationExtracted.max =end;
        }
    }
}

// join(12..78,134..202)
// join(complement(4918..5163),complement(2691..4571))
// join(1..100,J00194.1:100..202)

void sVaxAnnotGB::parseJoinOrOrder(const char * textRaw, sVec < startEnd > & startEndOut, const char * toCompare /* join( */){ // toCompare either: "join(" or "order("
    sStr joinTagRemoved;
    //extractContent(textRaw,joinTagRemoved , "join(", ")");
    extractContent(textRaw,joinTagRemoved , toCompare, ")");

    sStr textRawSplit;
    sString::searchAndReplaceSymbols(&textRawSplit,joinTagRemoved.ptr(0),0,",",0,0,true,true,false,true);
    for (const char * cmp = textRawSplit; cmp; cmp = sString::next00(cmp)){
        bool isComplement = (strncmp("complement",cmp,10)==0) ? true : false;
        startEnd * myStartEnd = startEndOut.add();
        if (isComplement){
            myStartEnd->forward=false;
            sStr text; text.cut(0);
            sString::cleanMarkup(&text,cmp,0,"complement("_,")"_,0,0,true,false,false);
            extractLocation(text.ptr(1),*myStartEnd,false);
        }
        else  {
            extractLocation(cmp,*myStartEnd,true);
        }
    }
}
// complement(34..126)
// complement(join(2691..4571,4918..5163))


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
        }

    }

}

