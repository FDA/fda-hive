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
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioal.hpp>


const char * sBioAlBlast::readVal ( char * src,const char * find, const char * fmt, void * pVal )
{

    const char* fnd=strstr(src,find) ;
    if(!fnd)return 0;

    fnd+=sLen(find);
    fnd+= strspn(fnd, ":="sString_symbolsSpace);
    sscanf(fnd,fmt,pVal);
    return fnd;
}

const char * sBioAlBlast::getNextLine(sStr * Buf, const char * currentPtr, idx sizeLeftover)
{
    Buf->cut(0);
    idx nextPtr=sString::copyUntil(Buf, currentPtr, sizeLeftover,"\r\n");

    currentPtr += nextPtr;
    // make sure that nextPtr is not pointing to \r\n but pointing to beginning of next line
    while( ((*currentPtr=='\r')||(*currentPtr=='\n')) && (currentPtr<(currentPtr+sizeLeftover)) )
        ++currentPtr;
    return currentPtr;
}

idx sBioAlBlast::SSSParseAlignment(sIO * log, char * fileContent, idx filesize, sVec < idx > * alignOut,  idx scoreFilter, idx minMatchLength, idx maxMissQueryPercent, sDic < idx > * rgm, sDic < idx > * sub,sDic < idx > * qry, idx blastOutMode, sDic <idx> *unalignedQry)
{
    //sString::searchAndReplaceSymbols(fileContent, filesize,"\r\n",0,0,true,true,false,false);
    //char * buf=fileContent;
    const char * ptr=fileContent;
    char * buf;
    // temporary variables
    sStr t;
    idx num,cntFound=0;

    idx qryLen=0, qryId=0,subId=0, alScore=0;
    sStr Buf;


    for (idx iAl=0, iLine=0; *ptr && (0 < (fileContent+filesize-ptr)) ; ++iAl , ++iLine){

        //buf=sString::next00(buf); if(!buf)break;
        const char * ptr0=ptr;
        ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
        buf=Buf.ptr();

        if(iLine==0 && blastOutMode==eBlastStandardOut && strstr(buf,"blat"))
            blastOutMode=eBlastBlatOut ;

        if( strstr (buf,"Query=") ) {

            //Determine the query identifier
            t.cut(0);sString::cleanEnds(&t,buf+6, 0,sString_symbolsBlank,true);
            idx qryIdStart = (idx)(ptr0-fileContent);   // remember query Id Start

            t.cut(t.length()-2);
            // walk through strings until we find the query length
            while(*ptr &&
                (
                    (blastOutMode == eBlastStandardOut && !readVal (buf,"Length", "%"DEC, &qryLen))
                    ||
                    (blastOutMode == eBlastProteinOut && !readVal (buf,"Length", "%"DEC, &qryLen))
                    ||
                    (blastOutMode == eBlastBlatOut && !readVal (buf,"(", "%"DEC, &qryLen))
                )
                ){
//                buf=sString::next00(buf);
                ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                buf=Buf.ptr();
                t.printf("%s",buf);  // Append the next line to the query Id
            }
            t.cut(t.length() - strlen(buf));  // Remove Length line from t.ptr()
            t.add0();

            if(qry){
                idx * pq=qry->set(t,0,&qryId);
                if(*pq==0) {*pq=qryIdStart;}
            }else {
                sscanf(t,"%"DEC,&qryId);
            }
        }

        if(buf[0]=='>') {
            t.cut(0);sString::cleanEnds(&t,buf, 0,">"sString_symbolsBlank,true);
            idx subIDtemp = 0;
            if(sub){
//                *sub->set(t,0,&subIDtemp)=(buf-fileContent);
                *sub->set(t,0,&subIDtemp)=(idx)(ptr0-fileContent);
            }else {
                sscanf(t,"%"DEC,&subIDtemp);
            }
            idx * idSubptr = 0;
            sStr tid;
            sString::searchAndReplaceSymbols(&tid,t.ptr(),0," ","\0",true,true,false,true);
            if (rgm && rgm->dim() != 0){   // There is a dictionary to look for
                idSubptr = rgm->get(tid.ptr());  // So go and get something
            } //for old .vioseq format bioseq->id(i) returns id with leading '>'
            if (!idSubptr && rgm) {
                sStr tt(">%s",tid.ptr());
                idSubptr = rgm->get(tt.ptr());
            }

            if (!idSubptr){  // There is nothing in the dictionary, or return value is 0
                char *p = strstr(tid.ptr(), "HIVESEQID=");
                if (p){  // If there is a HIVESEQID=, get it in idSubptr
                    sscanf(p+10,"%"DEC"", idSubptr);
                }
                else // There is no HIVESEQ
                    idSubptr = &subIDtemp;
            }
            else if (!idSubptr)
                idSubptr = &subIDtemp;

            subId = *idSubptr;
        }
        else if (unalignedQry && (strncmp("***** No hits found *****", buf, 25) == 0)) {
            idx * unqry = unalignedQry->get(&qryId, sizeof(idx));
            if( !unqry ) {
                unqry = unalignedQry->set(&qryId, sizeof(idx));
                *unqry = 0;
            };
            (*unqry) = (*unqry) + 1;

        }
        /*else if (strncmp("ref|",buf,4)==0) {
            t.cut(0);sString::cleanEnds(&t,buf, 0,">"sString_symbolsBlank,true);

            sStr s;
            idx slength = 0;
            for(char * p = t.ptr(); p < t.ptr() + t.length(); ++p) {
                if( s.length() == 0 && *p == '|' ) {
                    s.printf("%s", p+1);
                    ++slength;
                } else if( s.length() > 0 && *p == '|' ) {
                    s.cut(slength-1);
                    s.add0();
                    continue;
                } else if( slength > 0 )
                    ++slength;
            }

            sStr idSubStr;
            idSubStr.printf("%s", s.ptr());
            idx * idSubptr;
            idSubptr = rgm->get(idSubStr.ptr());
            if (idSubptr) subId = *idSubptr;
            else {
                continue;
            }
        }*/

        if(strstr (buf,"Score =") ) {
            sVec<idx> QS,QE,SS,SE;//record every query and subject num;
            sStr ALSEQ1,ALSEQ2;
            sVec<idx> S1, S2;
            idx alMissMatches=0;
            char alLenIdent;
            idx alLength0 = 0;
            char alStrand=0;

            // read the score and the Strand information
            while(*ptr && !readVal (buf,"Score", "%"DEC, &alScore) ){
//                buf=sString::next00(buf);
                ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                buf=Buf.ptr();
            }
            while(*ptr && !readVal (buf,"Identities =", "%c", &alLenIdent) ){
                ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                buf=Buf.ptr();
            }
            {
                const char* fnd=strstr(buf,"/") ;
                if(fnd){
                    sscanf(fnd+1,"%"DEC,&alLength0);
                }
            }

            if (blastOutMode != eBlastProteinOut){
                sString::searchAndReplaceSymbols(buf,0,sString_symbolsBlank,"",0,true,true,false,true);
                while(*ptr && !readVal (buf,"Strand=Plus/", "%c", &alStrand) ){
                    //                buf=sString::next00(buf);
                    ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                    buf=Buf.ptr();
                    sString::searchAndReplaceSymbols(buf,0,sString_symbolsBlank,"",0,true,true,false,true);
                }
            }
            else {
                alStrand = 'P';
            }

            // now loop over alignment itself
//            ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
//            buf=Buf.ptr();

            // !*ptr
            while ( *buf ) {
                char *ptr2;
                const char *ptr_prev = ptr;
                ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                if(!*ptr)
                    break;
                buf=Buf.ptr();

                if(buf[0]=='>' || strstr(buf,"Query=") || strstr(buf,"Score") )
                    {// buf-=2;
                    //ptr -= (strlen(buf) + 1);
                    idx dif = ptr - ptr_prev;
                    ptr -= dif;
                    break; // this is the next Query or subject alignment information
                    }

                // read the query line
                if( !readVal(buf,"Query","%"DEC,&num) )
                    continue;


                QS.vadd(1,num);
                ptr2=sString::skipWords(buf,0,2);
                if(ptr2){ALSEQ1.shrink00(0,1);sString::copyUntil(&ALSEQ1,ptr2,0,sString_symbolsBlank);}
                ptr2=sString::skipWords(buf,0,3);
                QE.vadd(1,(idx)atol(ptr2));

//                buf=sString::next00(buf);
                ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                buf=Buf.ptr();

                //read the sbject line
                while( *ptr && !readVal(buf,"Sbjct","%"DEC,&num) ){
//                    buf=sString::next00(buf);
                    ptr = getNextLine(&Buf,ptr,fileContent+filesize-ptr);
                    buf=Buf.ptr();
                }
                SS.vadd(1,num);
                ptr2=sString::skipWords(buf,0,2);
                if(ptr2){ALSEQ2.shrink00(0,1);sString::copyUntil(&ALSEQ2,ptr2,0,sString_symbolsBlank);}
                ptr2=sString::skipWords(buf,0,3);
                SE.vadd(1,(idx)atol(ptr2));


            }

            // if this alignment does not pass filters, skip this
            if (scoreFilter && alScore<scoreFilter)
                continue;
            idx alLength = (alLength0 == 0) ? (abs(QE[QE.dim()-1]-QS[0])+1) : alLength0;
            if ( alLength < minMatchLength )
                  continue;

            char * alseq1=ALSEQ1.ptr(), * alseq2=ALSEQ2.ptr();
            idx count1, count2;//count '-' in query and subject
            idx h,l, q=0;


            // now parse the query and subject lines
            for( l=0 ; l<QS.dim() ; ++l ) { // for every single alignment lineset
                idx seclen=sLen(alseq1);
                S1.add(seclen);
                S2.add(seclen );

                for( h=0,count1=0,count2=0 ; h<seclen ; h++ ) //for every single line
                {

                    if(alStrand=='P'){
                        if(alseq1[h]!='-')
                        {
                            if(toupper(alseq1[h])==toupper(alseq2[h])){
                                S1[q]=QS[l]+h-count1;
                                S2[q]=SS[l]+h-count2;
                            } else if(alseq2[h]=='-') {
                                S2[q]=-1;
                                S1[q]=QS[l]+h-count2;
                                count2++;
                            } else {
                                S1[q]=QS[l]+h-count1;
                                S2[q]=SS[l]+h-count2;
                                alMissMatches++;
                            }
                        }
                        else if(alseq1[h]=='-'){
                            S1[q]=-1;
                            count1++;
                            S2[q]=SS[l]+h-count2;
                        }
                    }
                    else if(alStrand=='M') {
                        if(alseq1[h]!='-'){
                            if(toupper(alseq1[h])==toupper(alseq2[h])) {
                                S1[q]=QS[l]+h-count1;
                                S2[q]=SS[l]-h+count2;
                            } else if(alseq2[h]=='-') {
                                S2[q]=-1;
                                S1[q]=QS[l]+h-count1;
                                count2++;
                            } else  {
                                S1[q]=QS[l]+h-count1;
                                S2[q]=SS[l]-h+count2;
                                alMissMatches++;
                            }
                        }
                        else if(alseq1[h]=='-'){
                            S1[q]=-1;
                            count1++;
                            S2[q]=SS[l]-h+count2;
                        }
                    }
                    q++;
                }
                alseq1=sString::next00(alseq1);
                alseq2=sString::next00(alseq2);
            }

            // skip if did not pass the minimal accuracy criteria
            if ( (100*(alMissMatches&0xFFFF))/alLength > maxMissQueryPercent)
                continue;

            idx curOfs=alignOut->dim();
            sBioseqAlignment::Al * hdr=(sBioseqAlignment::Al *)alignOut->add(sizeof(sBioseqAlignment::Al)/sizeof(idx));
            hdr->setFlags(alStrand=='P' ? sBioseqAlignment::fAlignForward : (sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement));
            if (hdr->flags()&sBioseqAlignment::fAlignBackward) {
                hdr->setSubStart(SE[l-1]);//hdr->subEnd=SS[0];
                hdr->setQryStart(qryLen-QE[l-1]);//hdr->qryEnd=qryLen-QS[0];
            }
            else {
                hdr->setSubStart(SS[0]); //hdr->subEnd=SE[l-1];
                hdr->setQryStart(QS[0]); //hdr->qryEnd=QE[l-1];
            }
            hdr->setIdSub(subId);
            hdr->setIdQry(qryId);
            //hdr->idSrc=0;
            hdr->setScore(alScore);
            hdr->setLenAlign(alLength);
            hdr->setDimAlign(q*2);
//                hdr->missMatches=((alMissMatches&0xFFFF));// | ((matchcounters[2]&0xFFFF)<<16) | ((matchcounters[3]&0xFFFF)<<32);
            //store in the output buffer
            idx * pointerToWrite=alignOut->add(2*q);
            hdr=(sBioseqAlignment::Al *)alignOut->ptr(curOfs);
            for(idx r=0;r<q;r++)    {
                idx rfrom =  (hdr->flags()&sBioseqAlignment::fAlignBackward) ? (q-r-1) : r ;

                pointerToWrite[r*2+0]=S2[rfrom];
                pointerToWrite[r*2+1]=S1[rfrom];
                if(pointerToWrite[r*2+0]!=-1){
                    pointerToWrite[r*2+0]-=hdr->subStart();
                }
                if(pointerToWrite[r*2+1]!=-1) {
                    if((hdr->flags())&sBioseqAlignment::fAlignBackward) pointerToWrite[r*2+1] = qryLen - pointerToWrite[r*2+1];
                    pointerToWrite[2*r+1]-=hdr->qryStart();
                }
            }
            hdr->setSubStart(hdr->subStart()-1);
            //--hdr->subEnd;
            if(!((hdr->flags())&sBioseqAlignment::fAlignBackward)){
                hdr->setQryStart(hdr->qryStart()-1);
                //--hdr->qryEnd;
            }


            hdr->setDimAlign(sBioseqAlignment::compressAlignment(hdr, pointerToWrite, pointerToWrite ));
            alignOut->cut(curOfs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+hdr->dimAlign());
            ++cntFound;
        }

    }
    return cntFound;
}
