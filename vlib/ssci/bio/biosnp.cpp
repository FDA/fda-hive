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
#include <ssci/math.hpp>
#include <slib/std/string.hpp>
#include <ssci/bio/bioseqsnp.hpp>
#include <limits.h>

using namespace slib;

#define QIDX(_v_iq, _v_len)    ((flags&sBioseqAlignment::fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))
#define SIDX(_v_is, _v_len)    ( (_v_is) )

const real sBioseqSNP::noiseCutoffThresholds[sBioseqSNP::noiseCutoffThresholds_NUM] = {0.5,0.75,0.85,0.9,0.95,0.99};
const real sBioseqSNP::freqProfileResolution = 0.01;
const real sBioseqSNP::histCoverResolution = 0.01;
const real sBioseqSNP::noiseProfileResolution = 0.0001;

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  SNP counting
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


idx sBioseqSNP::snpCountMatches(const char * sub, idx sublen, const char * qry, idx qrylen,sBioseqAlignment::Al * hdr, idx * m)
{
    idx flags=hdr->flags(), matches=0;

    for( idx i=0 ; i<2*hdr->lenAlign(); i+=2) {

        idx is=m[i];
        idx iq=m[i+1];

        if(is<0 || iq<0)
            continue;

        iq=QIDX( hdr->qryStart()+iq, qrylen );
        is=hdr->subStart()+is;
        char qLet=sBioseqAlignment::_seqBits(qry, iq , flags ) ;  // mapped ok
        char sLet=sBioseqAlignment::_seqBits(sub, is, 0 ) ;
        if(qLet!=sLet)
            continue;
        ++matches;

    }
    return matches;
}

idx sBioseqSNP::snpCountSingleSeq(SNPFreq * freq, ProfilePosInfo * pinf,ProfileAAInfo * ainf, idx substart, idx sublen, const char * sub, idx subbuflen,
    const char * qrybits, idx qrybuflen,const char * qua, bool quabit, sBioseqAlignment::Al * hdr, idx * m, SNPParams * SP, idx qrpt, ProfileExtraInfo * extraInf,
    idx idQry, ionRange * ionRange)
{

    if( SP->maxMissmatchPercCutoff && (100 * snpCountMatches(sub, subbuflen, qrybits, qrybuflen, hdr, m) < hdr->lenAlign() * (100 - SP->maxMissmatchPercCutoff)) )
        return 0;

    if( SP->lenPercCutoff != 0 && (hdr->lenAlign() < qrybuflen * SP->lenPercCutoff) )
        return 0;

    idx i, is, iq, cntMapped = 0;

    sDic<ATGCcount> * fentr = 0;
    if(extraInf && extraInf->EntroMap.dim() )
        fentr = extraInf->EntroMap.ptr();

    idx qLeft = hdr->qryStart() + m[1];
    idx qRight = qLeft + hdr->lenAlign();
    idx alLen = hdr->lenAlign();
    idx ipos = 0, thisCodon = 0, refCodon = 0, aaPos, prvaaPos=-2, codoncnt = 0, offsetToCodonStart = ionRange? (ionRange->forward?2:0):2;

    // retrieve the query sequence
    idx lastSubOK = 0, ch = 0, sh = 0, flags = hdr->flags();
    char let = 0;
    idx lastAlignedLetter = hdr->qryStart() + m[2 * hdr->lenAlign() - 1];
    idx qval = -1;

//    char szStart[128], szEnd[128];
//    if( ionRanges ) {
//        szEnd[0] = '0';
//        szEnd[1] = ':';
//    }
    idx iCdn = 2;
//    idx lenStart, lenEnd;
//    idx lastCDSstart = -1;
//    idx lastCDSend = -1;

    if( qua && SP->maxLowQua != 0 ) {

        idx lowqua = 0;
        for(i = 0; i < qrybuflen; ++i) {
            if( quabit ) {
                if( (qua[i / 8] & (((idx) 1) << (i % 8))) == 0 ) // quality bit is not set
                    ++lowqua; // ignore the bases with low Phred score
            } else if( (qua[i]) < 20 ) // quality bit is not set
                ++lowqua; // ignore the bases with low Phred score
        }
        if( lowqua * 100 > qrybuflen * SP->maxLowQua )
            return 0;

    }

    for(i = 0; i < 2 * hdr->lenAlign(); i += 2) {

        is = m[i];
        iq = m[i + 1];
        idx iqx = -1;
        if( is >= 0 && iq >= 0 ) {
            iqx = QIDX(hdr->qryStart() + iq, qrybuflen);
            if( qua && SP->useQuaFilter ) {
                if( quabit ) {
                    if( (qua[iqx / 8] & (((idx) 1) << (iqx % 8))) == 0 ) // quality bit is not set
                        continue; // ignore the bases with low Phred score
                } else if( (qua[iqx]) < SP->useQuaFilter ) // quality bit is not set
                    continue; // ignore the bases with low Phred score
            }
        }
        if( is < 0 ) {
            ch = 4;
            sh = 5;
            is = lastSubOK;
        }  // we remember inserts on the last mapped position
        else if( iq < 0 ) {
            sh = 4;
            ch = 5; // deletions
        } else {
            if( SP->cutEnds ) {
                if( iqx < SP->cutEnds )
                    continue; // we are cutting tails here
                else if( iqx > lastAlignedLetter - SP->cutEnds )
                    continue; // we are cutting tails here
            }
            ch = let = sBioseqAlignment::_seqBits(qrybits, iqx, hdr->flags());  // mapped ok
        }

        if( is + hdr->subStart() < substart )
            continue; // before the range requested
        else if( is + hdr->subStart() >= sublen + substart )
            break; // after the range requested

        if( (hdr->flags() & sBioseqAlignment::fAlignCircular) && is >= sublen )
            is -= sublen;

        if( freq ) {
            SNPFreq * line = (freq + (hdr->subStart() + is - substart));
            ++cntMapped;
            if( hdr->flags() & sBioseqAlignment::fAlignBackward )
                (line->atgcRev[ch]) += qrpt; // ch+=ddd/2;
            else
                (line->atgcFwd[ch]) += qrpt; //++(freq)[line+ch];

            if( qua && ch < 4 ) { //  we count the quality only for mapped bases, not inserts
                if( quabit )
                    qval = ((qua[iq / 8] & (((idx) 1) << (iq % 8))) == 0) ? 40 : 10;
                else
                    qval = qua[iq];
                qval *= qrpt;

                if( hdr->flags() & sBioseqAlignment::fAlignBackward )
                    line->setQuaRev(line->quaRev() + qval); // qua[i];
                else
                    line->setQuaFwd(line->quaFwd() + qval);  // line->qua[0]+=qval; // qua[i];
            }
        }

        // for each position remember the occurrence of the particular letter at particular position
        if( fentr && is >= 0 && iq >= 0 && ch < 4 ) {

            idx ss = is + hdr->subStart() - substart;
            idx qq = QIDX(hdr->qryStart() + iq, qrybuflen);

            ATGCcount * atgc = fentr[ss].set(&qq);
            if( hdr->flags() & sBioseqAlignment::fAlignBackward )
                (atgc->countRev)[ch] += qrpt;
            else
                (atgc->countFwd)[ch] += qrpt;
            atgc->iqx = qq;

        }

        if( pinf ) {
            ProfilePosInfo * pine = (pinf + (hdr->subStart() + is - substart));

            if( qLeft > 1 ) {
                pine->lenHistogram.cntLeft += qrpt * qLeft;
                pine->lenHistogram.maxLeft=sMax(pine->lenHistogram.maxLeft, qLeft);
            }
            if( qRight < qrybuflen - 1 ) {
                pine->lenHistogram.cntRight += qrpt * (qrybuflen - qRight);
                pine->lenHistogram.maxRight=sMax(pine->lenHistogram.maxRight, (qrybuflen - qRight));

            }

            //pine->lenAnisotropy=1111;
            //pine->lenHistogram.lenRight+= qrpt * (qRight-iq-hdr->qryStart() );
            //pine->lenHistogram.lenLeft += qrpt * (iq-m[1]);
            idx lenLeft = qrpt * (iq-m[1]);
            idx lenRight = qrpt * (qRight-iq-hdr->qryStart() );
            idx diff = ((lenLeft - lenRight) * 100) / (lenLeft + lenRight);
            diff = (diff < 0) ? (diff * -1) : diff;
            pine->lenHistogram.lenAnisotropy += (diff < 25) ? 0 : ((diff - 25) * 100) / 75;

            pine->lenHistogram.cntSeq += qrpt * qrybuflen;
            pine->lenHistogram.cntAl += qrpt * alLen;

            if( qua && ch < 4 ) {
                if( hdr->flags() & sBioseqAlignment::fAlignBackward )
                    pine->quaRevATGC[ch] += qval;
                else
                    pine->quaFwdATGC[ch] += qval;
            }
            pine->score += hdr->score();

        }
        lastSubOK = is;
        if( ainf ) {
            if(!cntMapped)
                ++cntMapped;
            ipos = hdr->subStart() + is;
            aaPos = -1;
            sh = sBioseqAlignment::_seqBits(sub, ipos, 0);
            // wander here ?
            if( ionRange ) {
                idx lastCdn = iCdn;
                aaPos = sBioseqSNP::baseFrameDecode(ch,ipos+1 ,ionRange,thisCodon, &iCdn);
                aaPos = sBioseqSNP::baseFrameDecode(sh,ipos+1 ,ionRange,refCodon);
                if(aaPos >= 0 && iCdn>=2 ) {
                    if( codoncnt>=2 ) {
                        idx aalet = sBioseq::mapCodon(thisCodon);
                        ProfileAAInfo * aine = (ainf + (ipos - offsetToCodonStart - substart)/3 ); //point to the beginning of the codon
                        aine->aa[aalet] += qrpt;
                        aine->pos = aaPos;
                        aine->ref = sBioseq::mapCodon(refCodon);
                    }
                    thisCodon = 0;
                    refCodon = 0;
                    codoncnt = 0;
                } else if (aaPos<0) {
                    iCdn = lastCdn; //if aaPos is
                } else {
                    ++codoncnt;
                }
            } else if(ipos != sNotIdx ){
                aaPos = ipos / 3;
                iCdn = ipos % 3;
                if( aaPos != prvaaPos ) {
                    codoncnt=0;
                    refCodon=0;
                    thisCodon = 0;
                    prvaaPos = aaPos;
                }
                if( ch >= 4 )
                    ch = 0;
                thisCodon |= (ch << (iCdn * 2));
                refCodon |= (sh << (iCdn * 2));
            /*    if(ipos>=117 && i < 2 * hdr->lenAlign()-4 )
                    ::printf("stop here \n");*/
                if( (++codoncnt)>= 3 && iCdn == 2 ) {
                    idx aalet = sBioseq::mapCodon(thisCodon);
                    ProfileAAInfo * aine = (ainf + (ipos - offsetToCodonStart - substart)/3 ); //point to the beginning of the codon
                    aine->aa[aalet] += qrpt;
                    aine->pos = aaPos;
                    aine->ref = sBioseq::mapCodon(refCodon);
                }
            }
        }
    }
    return cntMapped;
}




idx sBioseqSNP::snpCountPosInfo(SNPFreq * freq, ProfilePosInfo * pinf, const char * subseq, idx substart, idx sublen, SNPParams * SP, ProfileExtraInfo * extraInf )
{
    idx maxfe=0;
    for(idx ir=0; ir<sublen; ir+=1) {
        if(extraInf->EntroMap.dim()<=ir)continue;
        sDic < ATGCcount > * fe=extraInf->EntroMap.ptr(ir);
        if(fe->dim()>maxfe)
            maxfe=fe->dim();
    }

    for(idx ir=0; ir<sublen; ir+=1) {
        SNPFreq * line=(freq + ir );

        // determine consensus and reference letters
        idx consLet=0, snpLet=0, ic, v , vmax=-10000,vsmax=-10000; // refLet=sBioseq::mapATGC[(idx)s[ir-0]]; // -substart
        //idx refLet = sBioseqAlignment::_seqBits(subseq, ir+substart , 0);
        idx tot=0;
        for ( ic=0; ic<4 ; ++ic ) {
            v=line->atgcFwd[ic]+line->atgcRev[ic];
            if(vmax<v){vsmax=vmax; snpLet=consLet; vmax=v; consLet=ic;}
            else if(vsmax<v){vsmax=v;snpLet=ic;}
            tot+=v;
        }
        //idx relativeLetter= (SP->snpCompare==0 ) ? consLet : refLet ;

        //if(extraInf->EntroMap.dim()<=ir)
        if(maxfe<=0)
            continue;
        sDic < ATGCcount > * fe=extraInf->EntroMap.ptr(ir);


        real p;
        real perLetterTot[4];sSet(perLetterTot,0,sizeof(perLetterTot));
        idx allTot=0,allFwd=0,allRev=0,ik ;

        // compute the total number of atgc letters (etot) over all the read positions
        for ( idx id=0; id<fe->dim(); ++id ){ // scan all query positions and sum the atgc occurences
            ATGCcount * atgc=fe->ptr(id);
            for ( idx ik=0; ik<sDim(perLetterTot);++ik) {
                v=atgc->countFwd[ik]+atgc->countRev[ik];
                perLetterTot[ik]+=v;
                allTot+=v;
                allFwd+=atgc->countFwd[ik];
                allRev+=atgc->countRev[ik];
            }

        } // at this poitns eTot is showing the total number of ATGC in this position and etot.count[] has the same in different bins forward A,T,G,C reverse A,T,G,C
        if(allTot){
            for ( idx ik=0; ik<sDim(perLetterTot);++ik) {
                perLetterTot[ik]/=allTot;
            }
        }

        // now for every query position we scan and compute shannon entropy
        real perLetterEntr[4];sSet(perLetterEntr,0,sizeof(perLetterTot));
        real perDirectionEntr[2];sSet(perDirectionEntr,0,sizeof(perDirectionEntr));
        real allEntr=0,allFwdEntr=0,allRevEntr=0;
        for ( idx id=0; id<fe->dim(); ++id ) { // scan each query positions
            ATGCcount * atgc=fe->ptr(id);

            // entropy per letter
            idx vtot=0,vfwd=0,vrev=0;
            for (ik=0; ik<sDim(perLetterTot);++ik) {
                v=atgc->countFwd[ik]+atgc->countRev[ik];

                p = perLetterTot[ik] ? (real)(v/allTot)/(perLetterTot[ik]) : 0; // now for each letter we compute probability for this partiular query position
                p = (p>0) ? (-p*log10(p)) : 0;
                perLetterEntr[ik]+=p; // here we remember the total p*log(p) for every letter
                vtot+=v;
                vfwd+=atgc->countFwd[ik];
                vrev+=atgc->countRev[ik];
            }
            p = allTot ? (real)(vtot)/allTot : 0; // now for each letter we compute probability for this partiular query position
            p = (p>0) ? (-p*log10(p)) : 0;
            allEntr+=p;

            p = allFwd ? (real)(vfwd)/allFwd: 0; // now for each letter we compute probability for this partiular query position
            p = (p>0) ? (-p*log10(p)) : 0;
            allFwdEntr+=p;

            p = allRev ? (real)(vrev)/allRev: 0; // now for each letter we compute probability for this partiular query position
            p = (p>0) ? (-p*log10(p)) : 0;
            allRevEntr+=p;


            /*for (ik=0; ik<sDim(perLetterTot);++ik) {
                v=atgc->countFwd[ik]+atgc->countRev[ik];

                p = perLetterTot[ik] ? (real)(v)/perLetterTot[ik] : 0; // now for each letter we compute probability for this partiular query position
                p=pTot ? p/pTot : 0;
                p = (p>0) ? (-p*log10(p)) : 0;

                perLetterEntr[ik]+=p; // here we remember the total p*log(p) for every letter
            }*/
        }


        // now normalize it
        //real norm;
        idx minCoverForEntr=fe->dim();
        if(minCoverForEntr<3)minCoverForEntr=3;

        if(fe->dim()>1){
            if(allTot>minCoverForEntr) allEntr/=log10((real)maxfe);//log10((real)allTot);
            else allEntr=1;

            if(allFwd>minCoverForEntr) allFwdEntr/=log10((real)maxfe);//log10((real)allTot);
            else allFwdEntr=1;

            if(allRev>minCoverForEntr) allRevEntr/=log10((real)maxfe);//log10((real)allTot);
            else allRevEntr=1;

            for (ik=0; ik<sDim(perLetterTot);++ik){
                if(perLetterTot[ik]>minCoverForEntr)perLetterEntr[ik]/=log10((real)maxfe);//log10((real)perLetterTot[ik]);
                else perLetterEntr[ik]=allEntr;
            }
        }

        for (ik=0; ik<sDim(perLetterEntr);++ik) {
            if(perLetterTot[ik]<minCoverForEntr)perLetterEntr[ik]=allEntr;
            else if (SP->minImportantEntropy && perLetterEntr[ik]>=SP->minImportantEntropy)
                perLetterEntr[ik]=allEntr;
        }


        /*if(entrL) {
            for (ik=0; ik<sDim(perLetterTot);++ik)
                entrL[ir].atgc[ik]=perLetterEntr[ik];
            entrL[ir].tot+=allEntr;
            entrL[ir].ref=perLetterEntr[relativeLetter];
        }
        */

        //perLetterEntr[relativeLetter]
        real vv=allEntr*(SNPFreq::ENTRMAX);
        if(vv<0)vv=0;
        if(vv>(SNPFreq::ENTRMAX)) vv=(SNPFreq::ENTRMAX);
        line->setEntr((idx)(vv),0);

        vv=perLetterEntr[snpLet]*(SNPFreq::ENTRMAX);
        if(vv<0)vv=0;
        if(vv>(SNPFreq::ENTRMAX)) vv=(SNPFreq::ENTRMAX);
        line->setEntr((idx)(vv),1);


        vv=allFwdEntr*(SNPFreq::ENTRMAX);
        if(vv<0)vv=0;
        if(vv>(SNPFreq::ENTRMAX)) vv=(SNPFreq::ENTRMAX);
        line->setEntr((idx)(vv),2);

        vv=allRevEntr*(SNPFreq::ENTRMAX);
        if(vv<0)vv=0;
        if(vv>(SNPFreq::ENTRMAX)) vv=(SNPFreq::ENTRMAX);
        line->setEntr((idx)(vv),3);


                //real vv=allEntr*(SNPFreq::ENTRMAX);

        /*
        vv=perLetterEntr[snpLet]*(SNPFreq::ENTRMAX);
        if(vv<0)vv=0;
        if(vv>(SNPFreq::ENTRMAX)) vv=(SNPFreq::ENTRMAX);
        line->setSnpentr((idx)(vv));
        */

        for( idx in=0; in<4; ++in) {
            vv=perLetterEntr[in]*(SNPFreq::ENTRMAX);
            if(vv<0)vv=0;
            if(vv>(SNPFreq::ENTRMAX)) vv=(SNPFreq::ENTRMAX);
            line->setSnpentr((idx)(vv),in);
        }

    }


    return 1;

}






idx sBioseqSNP::snpCleanTable( SNPFreq * freq, const char * subseq, idx substart, idx sublen, SNPParams * SP) // subseq substart
{
    for(idx ir=0; ir<sublen; ir+=1) {
        SNPFreq * line=(freq + ir );

        idx refLet = sBioseqAlignment::_seqBits(subseq, substart+ir , 0);
        idx consLet=0 , vmax=-10000, ic, tot=0, totR=0, totF=0;
        for ( ic=0; ic<4 ; ++ic ) {
            totF+=line->atgcFwd[ic];
            totR+=line->atgcRev[ic];
            tot+=line->atgcFwd[ic]+line->atgcRev[ic];
            if(vmax<tot){ vmax=tot; consLet=ic;}
        }
        idx relativeLetter= (SP->snpCompare==0 ) ? consLet : refLet ;
        real c_noise;
        for ( ic=0; ic<4 ; ++ic ) {
            c_noise = SP->noiseCutoffs[ic][relativeLetter];
            if(!c_noise || relativeLetter==ic)continue;
            line->atgcFwd[ic]-=c_noise*totF;
            line->atgcRev[ic]-=c_noise*totR;
            if(line->atgcFwd[ic]<0)line->atgcFwd[ic]=0;
            if(line->atgcRev[ic]<0)line->atgcRev[ic]=0;
        }


        bool isok=true;
        if(!tot)
            isok=false;
        if( isok && tot < SP->minCover ){
            isok=false;
        }
        if( isok && SP->minFreqPercent ){
            idx cntMore=0;
            for ( ic=0; ic<4 ; ++ic ) {
                if(relativeLetter==ic)continue;
                if(100*(line->atgcFwd[ic]+line->atgcRev[ic])>=tot*SP->minFreqPercent){
                    if(!SP->minFreqIgnoreSNP){
                        ++cntMore;break;
                    }
                }
                else{
                    if(SP->minFreqIgnoreSNP){
                        line->atgcFwd[relativeLetter]+=line->atgcFwd[ic];
                        line->atgcRev[relativeLetter]+=line->atgcRev[ic];
                        line->atgcFwd[ic]=0;
                        line->atgcRev[ic]=0;
                    }
                }
            }
            if(!cntMore && !SP->minFreqIgnoreSNP)
                isok=false;
        }
        if( isok && SP->disbalanceFR!=0 && tot>2*SP->disbalanceFR ) {
            for ( ic=0; ic<4 ; ++ic ) {
                if(ic==relativeLetter)continue;

                if( line->atgcFwd[ic]>line->atgcRev[ic]*SP->disbalanceFR ){
                    line->atgcFwd[ic]=line->atgcRev[ic];
                }
                else if( line->atgcRev[ic]>line->atgcFwd[ic]*SP->disbalanceFR ){
                    line->atgcRev[ic]=line->atgcFwd[ic];
                }

                /*
                if( line->atgcFwd[ic]>line->atgcRev[ic]*SP->disbalanceFR ){isok=false;break;}
                else if( line->atgcRev[ic]>line->atgcFwd[ic]*SP->disbalanceFR ){isok=false;break;}
                else continue;

                if(line->atgcFwd[ic]>line->atgcRev[ic])line->atgcFwd[ic] = line->atgcRev[ic];
                else if(line->atgcRev[ic]>line->atgcFwd[ic])line->atgcRev[ic] = line->atgcFwd[ic];
                */
            }
            //adjust qualities
            idx totNR=0, totNF=0;
            for ( ic=0; ic<4 ; ++ic ) {
                totNF+=line->atgcFwd[ic];
                totNR+=line->atgcRev[ic];
            }
            if(totF!=totNF && totF)
                line->setQuaFwd((totNF*line->quaFwd())/totF);
            if(totR!=totNR && totR)
                line->setQuaRev((totNR*line->quaRev())/totR);

        }
        if(SP->entrCutoff>0){
                idx entr=line->entr(ic)*(1./(SNPFreq::ENTRMAX));
                if(entr<SP->entrCutoff)
                    isok=false;
        }

        if( !isok ){
            for ( ic=0; ic<4 ; ++ic ){
                line->atgcFwd[ic]=0;
                line->atgcRev[ic]=0;
            }
///FLAGS            line->flags|=eSNPNotEnoughCoverage;//com.printf("Not enough coverage");
        }


    }

    return 1;
}



//static
idx sBioseqSNP::snpOutTable(sStr * out, sStr * xinf, sStr * aainf, sStr * consensus, idx isub, const char * subseq,
    SNPFreq * freq, ProfilePosInfo * pinf, ProfileAAInfo * ainf, idx substart, idx sublen, SNPParams * SP , idx * multAlPosMatch, SNPminmax * MinMaxParams , sIonAnnot * iannot )
{


    bool extra_info = (pinf && xinf);
    bool aa_info = (ainf && aainf);
    idx cnt = 0, prvAinf=-1;
    char refCodon=0,refCodonCnt=0;
    if( MinMaxParams && !MinMaxParams->isPrint ) {
        MinMaxParams->reset();
    }
    //####
//    idx relativeMaxLetter = 0, relativeMinLetter = 0;
    for(idx ir=0; ir<sublen; ir+=1) {
        idx tot=0, withIndels_tot=0, totF=0, totR=0;
        SNPFreq * line=(freq + ir );
        // determine consensus and reference letters
        idx consILet=0, ic, v , vmax=-10000; // refLet=sBioseq::mapATGC[(idx)s[ir-0]];
        char consLet='A';
        idx refILet =sBioseqAlignment::_seqBits(subseq, ir+substart , 0);

        for ( ic=0; ic<4 ; ++ic ) {
            v=line->atgcFwd[ic]+line->atgcRev[ic];
            if(vmax<v){vmax=v; consILet=ic;}
            tot+=v;
            totF+=line->atgcFwd[ic];
            totR+=line->atgcRev[ic];
        }
        withIndels_tot=tot;
        for(ic=4 ; ic < 6 ; ++ic)
            withIndels_tot+=line->atgcFwd[ic]+line->atgcRev[ic];

        if( SP->supportedDeletions ) {
            totF+=line->delFwd();
            totR+=line->delRev();
            v=line->deletions();
            if(vmax<v){vmax=v; consILet=5;}
            tot+=v;
        }
        if( MinMaxParams && MinMaxParams->isPrint ) {
            tot = MinMaxParams->posCntCur->total;
            totR = MinMaxParams->posCntCur->totR;
            totF = MinMaxParams->posCntCur->totF;
            withIndels_tot = MinMaxParams->posCntCur->total_withIndels;
        }
        consLet=consILet==5?'-':(char)sBioseq::mapRevATGC[consILet];
        idx relativeLetter = (SP->snpCompare==0 ) ? consILet : refILet ;

        while( MinMaxParams && !MinMaxParams->isPrint && ( substart+ir)/MinMaxParams->bucket_size > MinMaxParams->last_valid_bucket+1 ) {
            MinMaxParams->last_valid_bucket = (substart+ir+1)/MinMaxParams->bucket_size-1;

            MinMaxParams->outCSV();
        }

        if(SP->filterZeros && (tot<SP->minCover || tot==0 ) ){
            continue;
        }

        if(consensus) consensus->printf("%c",consLet);
        // output ATGC coverage and compute forward/reverse/total coverage
        ++cnt;
        if(isub!=sNotIdx)
            out->printf("%" DEC ",",isub);
        out->printf("%" DEC ",%c,%c",(ir+1+substart),(char)sBioseq::mapRevATGC[refILet],consLet);  // -substart


        for ( ic=0; ic<6 ; ++ic ) {
            v=line->atgcFwd[ic]+line->atgcRev[ic];

            if(v)out->printf(",%" DEC,v);
            else out->add(",0",2);
        }

        if( tot ) {
            out->printf(",%" DEC, tot);
        } else {
            out->add(",0", 2);
        }
        if( totF ) {
            out->printf(",%" DEC, totF);
        } else {
            out->add(",0", 2);
        }
        if( totR ) {
            out->printf(",%" DEC, totR);
        } else {
            out->add(",0", 2);
        }

        idx qua = line->getAvQua();
        if( qua ) {
            out->printf(",%" DEC, qua);
        } else {
            out->add(",0", 2);
        }

        if( line->entr(0) < 3 ) {
            out->add(",0", 2);
        } else {
            out->printf(",%.2lg", line->entr(0) * (1. / (SNPFreq::ENTRMAX)));
        }
        if( line->entr(1) < 3 ) {
            out->add(",0", 2);
        } else {
            out->printf(",%.2lg", line->entr(1) * (1. / (SNPFreq::ENTRMAX)));
        }

        real mutMin=1,mutMax=0, vv=0;
        //frequency table relative to reference
        for ( ic=0; ic<4 ; ++ic ) {
            v=line->atgcFwd[ic]+line->atgcRev[ic];
            if( MinMaxParams && MinMaxParams->isPrint ) {
                vv = MinMaxParams->curFreq[ic];
            } else {
                //this is still fine for relativeLetter=5 (deletions) because we only report frequencies for ACGT
                vv=(ic==relativeLetter) ? 0 : ((real)v)/(tot ?tot : 1 );
            }
            if(vv<1.e-4) {
                out->add(",0",2);
            }
            else {
                out->printf(",%.2lg",vv*100);
            }
            if( vv > mutMax ) mutMax = vv;
            if( vv < mutMin ) mutMin = vv;
        }

        if(multAlPosMatch) {
            out->printf(",%" DEC,multAlPosMatch[2*ir+1]);
        }
        //
        if(SP->entrCutoff) {
            for( idx in=0; in<4; ++in) {
                if(line->entr(in)<3) {
                    // // out->printf(",0");/// zzeerroo
                    out->add(",0",2);
                }
                else {
                    out->printf(",%.2lg",line->entr(in)*(1./(SNPFreq::ENTRMAX)));
                }
            }for( idx in=0; in<4; ++in) {
                if(line->snpentr(in)<3) {
                    // // out->printf(",0");/// zzeerroo
                    out->add(",0",2);
                }
                else {
                    out->printf(",%.2lg",line->snpentr(in)*(1./(SNPFreq::ENTRMAX)));
                }
            }
        }
        if(SP->directionalityInfo && (!MinMaxParams || !MinMaxParams->isPrint) ) {
            out->printf(",%" DEC ",%" DEC ",%d,%d,%d,%d,%d,%d",(totF ? (line->quaFwd())/(totF) : 0),(totR ? (line->quaRev())/(totR) : 0),line->atgcFwd[0],line->atgcFwd[1],line->atgcFwd[2],line->atgcFwd[3],line->atgcFwd[4],line->atgcFwd[5]);
        }
        out->addString("\n");
        if( MinMaxParams && !MinMaxParams->isPrint ) {
            MinMaxParams->updateMinMax(line,ir+substart,relativeLetter,tot,withIndels_tot,totF,totR);
        }

        //Extra info (histogram)
        if( extra_info ) {
            ProfilePosInfo * pine=(pinf + ir );
            if(isub!=sNotIdx) {
                xinf->printf("%" DEC ",",isub);
            }
            xinf->printf("%" DEC ",%c", (ir + 1 + substart), (char) sBioseq::mapRevATGC[refILet]);
            if(tot)xinf->printf(",%" DEC, pine->score/tot); else xinf->add(",0",2);

            withIndels_tot = ( MinMaxParams && MinMaxParams->isPrint )?MinMaxParams->info.tot_withInDelsMin:withIndels_tot;

            xinf->printf(",%" DEC, (tot && withIndels_tot ) ? pine->lenHistogram.cntSeq / withIndels_tot : 0);
            xinf->printf(",%" DEC, (tot && withIndels_tot ) ? pine->lenHistogram.cntAl / withIndels_tot : 0);
            xinf->printf(",%" DEC, (tot && withIndels_tot ) ? pine->lenHistogram.cntLeft / withIndels_tot : 0);
            xinf->printf(",%" DEC, (tot && withIndels_tot ) ? pine->lenHistogram.cntRight / withIndels_tot : 0);
            xinf->printf(",%" DEC, pine->lenHistogram.maxLeft );
            xinf->printf(",%" DEC, pine->lenHistogram.maxRight );
            xinf->printf(",%" DEC, (tot && withIndels_tot ) ? pine->lenHistogram.lenAnisotropy / withIndels_tot : 0);


            int p_qua;
            for(idx in = 0; in < 4; ++in) {
                p_qua = pine->quaFwdATGC[in];
                if( ( !MinMaxParams || !MinMaxParams->isPrint) && line->atgcFwd[in] )
                    p_qua = p_qua /line->atgcFwd[in];
                if(MinMaxParams && !MinMaxParams->isPrint) {
                    if( MinMaxParams->outInfoMax.quaFwdATGC[in] < p_qua ) {
                        MinMaxParams->outInfoMax.quaFwdATGC[in] = p_qua ;
                    }
                    if( MinMaxParams->outInfoMin.quaFwdATGC[in] > p_qua  ) {
                        MinMaxParams->outInfoMin.quaFwdATGC[in] = p_qua ;
                    }
                }
                if( !pine->quaFwdATGC[in] || !line->atgcFwd[in] )
                    xinf->printf(",0"); /// zzeerroo
                else
                    xinf->printf(",%" DEC, (idx)(p_qua ) );
            }
            for(idx in = 0; in < 4; ++in) {
                p_qua = pine->quaRevATGC[in];
                if( (!MinMaxParams || !MinMaxParams->isPrint) && line->atgcRev[in]  )
                    p_qua = p_qua/line->atgcRev[in];
                if(MinMaxParams && !MinMaxParams->isPrint){
                    if( MinMaxParams->outInfoMax.quaRevATGC[in] < p_qua) {
                        MinMaxParams->outInfoMax.quaRevATGC[in] = p_qua;
                    }
                    if( MinMaxParams->outInfoMin.quaRevATGC[in] > p_qua ) {
                        MinMaxParams->outInfoMin.quaRevATGC[in] = p_qua;
                    }
                }
                if( !pine->quaRevATGC[in] || !line->atgcRev[in] )
                    xinf->add(",0",2); // // xinf->printf(",0"); /// zzeerroo
                else
                    xinf->printf(",%" DEC, (idx)(p_qua ) ); /// zzeerroo
            }
            xinf->addString("\n");
            if( MinMaxParams && !MinMaxParams->isPrint) {
                for(idx max = 0 ; max < 2 ; ++max){
                    sBioal::LenHistogram * lenA = 0 , * lenB = &pine->lenHistogram;
                    if(!max) {
                        lenA = &MinMaxParams->outInfoMin.lenHistogram;
                    }
                    else {
                        lenA = &MinMaxParams->outInfoMax.lenHistogram;
                    }
                    if( (max?1:-1)*(lenB->cntSeq - lenA->cntSeq) > 0) {
                        if(!max)MinMaxParams->info.tot_withInDelsMin = withIndels_tot;
                        else MinMaxParams->info.tot_withInDelsMax = withIndels_tot;
                        lenA->cntSeq = lenB->cntSeq;
                        lenA->cntAl = lenB->cntAl;
                        lenA->cntFailed = lenB->cntFailed;
                        lenA->cntLeft = lenB->cntLeft;
                        lenA->cntRight = lenB->cntRight;
                        lenA->lenAnisotropy= lenB->lenAnisotropy;
                        lenA->cntRead = lenB->cntRead;
                        lenA->maxLeft = lenB->maxLeft;
                        lenA->maxRight= lenB->maxRight;
                    }
                }
            }
        }
        if(aa_info ) {
            idx p=(ir + substart)/3;
            idx spos=(ir+substart)%3;

            if(prvAinf!=p) {
                prvAinf=p;
                refCodon=0;
                refCodonCnt=0;
            }
            refCodon|=(refILet<<(spos*2));
            ++refCodonCnt;

            if(spos==2 && refCodonCnt==3) {
                idx aatot=0;
                ProfileAAInfo * aine=(ainf + p );
                aatot = aine->coverage();
                if(aatot) {
                    for(idx ico=0 ; ico< sDim(ainf->aa); ++ico ) {
                        if(aine->aa[ico]<aatot*0.01)
                            continue;
                        idx refAA=sBioseq::mapCodon(refCodon);
                        if(refAA==ico)
                            continue;
                        if(isub!=sNotIdx) {
                            aainf->printf("%" DEC ",",isub);
                        }
                        aainf->printf("%" DEC ",%" DEC ",%" DEC ",%s,%s,%d,%.2lf\n", 1+ir, 1+(p), aatot, sBioseq::mappedCodon2AA(refAA)->let,sBioseq::mappedCodon2AA(ico)->let,aine->aa[ico], aine->aa[ico]*1./aatot);
                    }
                }
            }
        }
    }
    if( MinMaxParams && !MinMaxParams->isPrint && !((substart+sublen)%MinMaxParams->bucket_size) ) {
        MinMaxParams->last_valid_bucket = (substart+sublen)/MinMaxParams->bucket_size-1;
        MinMaxParams->outCSV();
        MinMaxParams->reset_freqs();
    }


    return cnt;

}

#define addIannot(_v_type, _v_id, _v_id_len )  if(iannot){ \
        iannot->indexArr[3]=iannot->addRecord(sIonAnnot::eType,sLen((_v_type)) ,(_v_type)); \
        iannot->indexArr[4]=iannot->addRecord(sIonAnnot::eID,(_v_id_len) ,(_v_id) ); \
        iannot->addRelationVarg(0,sNotIdx,iannot->indexArr,0); \
}

#define addIannotAndDic(_v_type, _v_id, _v_id_len, _v_dic )  if(iannot){ \
        iannot->indexArr[3]=iannot->addRecord(sIonAnnot::eType,sLen((_v_type)) ,(_v_type)); \
        iannot->indexArr[4]=iannot->addRecord(sIonAnnot::eID,(_v_id_len) ,(_v_id) ); \
        _v_dic[_v_type] = iannot->indexArr[4]; \
        iannot->addRelationVarg(0,sNotIdx,iannot->indexArr,0); \
}

#define addWholeSequenceInfo(_v_zero_separated, _dummyValueToFixLater) if (iannot) {\
        for (const char * p = _v_zero_separated; *p; p=sString::next00(p)){ \
            sIPrintfFixBuf(ibuf,ilen,_dummyValueToFixLater,10,32); \
            addIannotAndDic(p,ibuf,ilen,recordIndexInfoToModify); \
        }\
}

struct contigStruct {
        idx start, end;
        idx accuCoverage;
        contigStruct(){ start=end=accuCoverage=0;};
};

idx sBioseqSNP::snpOutTable_version2( const char * subseq, SNPFreq * freq, idx substart, idx sublen,
    SNPParams * SP, sIonAnnot * iannot )
{
       static const char *ACGT_InDel[]={"A","C","G","T","Insertion","Deletion"};
       char ibuf[128]; idx ilen;
       const char * elementToPut = "total_contig_length" _ "mapped_coverage(percentage_reference)" _ "average_coverage_of_contigs" _ "total_number_of_contigs" _ "total_length_of_the_unmapped_regions" _ "unmapped_regions(percentage_reference)" _ "average_coverage_of_gaps" _ "total_number_of_gaps_found" __;

//       bool isFirstLined = true;
       idx cnt = 0;
       //#### variables for contigs, coverage information
       bool isContig = false;

       //move the window and store coverage when gap or contig ends
       ProfileStat ps; ps.initilize();
       ps.reflen=sublen;
       ps.averageContigCoverage=0;
       //ps.totalContigsNumber=1;
       ps.totalContigsNumber=0;
       ps.contigsPart=100.0;

       idx prvState=0, curState=0;
       sVec < contigStruct > contigsArray;
       idx contigCoverageAccu=0, contigLength=0;
       idx lastContigArr=0;
       //#### loop through position

       sDic < idx > recordIndexInfoToModify;
       sVec < idx > recordIndexToModifyForContigs;
       idx recordIndexContig =0;
       //idx coveragePos=0;
       for(idx ir=0; ir<sublen; ir+=1) {
           idx refLet = 0, tot=0, withIndels_tot=0, totF=0, totR=0;
           SNPFreq * line=(freq + ir );
           // determine consensus and reference letters
           idx consLet=0, ic, v , vmax=-10000; // refLet=sBioseq::mapATGC[(idx)s[ir-0]];
           refLet = sBioseqAlignment::_seqBits(subseq, ir+substart , 0);
           for ( ic=0; ic<4 ; ++ic ) {
               v=line->atgcFwd[ic]+line->atgcRev[ic];
               if(vmax<v){vmax=v; consLet=ic;}
               tot+=v;
               totF+=line->atgcFwd[ic];
               totR+=line->atgcRev[ic];
           }

           idx relativeLetter = (SP->snpCompare==0 ) ? consLet : refLet ;
           // compute contigs info for ionAnnot
           {
//               if (tot){
//                   if (!curState) {
//                       contigStruct * singleContig = contigsArray.add(1);
//                       singleContig->start = ir;
//                       isContig=true;
//                   }
//                   curState=1;
//                   prvState = curState;
//                   contigCoverageAccu +=tot;
//               }
//               else {
//                   curState=0;
//                   if (!ir) {
//                       ps.totalGapsNumber++;
//                   }
//               }
//               if (curState != prvState || ir+1==sublen) { // check gaps and contigs
//                   prvState = 0;
//                   lastContigArr = contigsArray.dim() - 1;
//                   contigsArray.ptr(lastContigArr)->end = ir-((ir+1==sublen)?0:1);
//                   contigsArray.ptr(lastContigArr)->accuCoverage = contigCoverageAccu;
//                   ps.totalGapsNumber+=((ir+1==sublen)?0:1);
//                   contigLength = (contigsArray.ptr(lastContigArr)->end - contigsArray.ptr(lastContigArr)->start +1);
//                   ps.totalContigLength += contigLength;
//                   contigLength = contigCoverageAccu =0;
//               }

               curState = 1&&tot;
               contigCoverageAccu +=tot;
               if(curState != prvState) {
                   if(curState) { //close Gap open Contig
                       if(ir)
                           ps.totalGapsNumber++;
                       contigStruct * singleContig = contigsArray.add(1);
                       singleContig->start = ir;
                       isContig=true;
                   } else { //close Contig open Gap
                       lastContigArr = contigsArray.dim() - 1;
                       contigsArray.ptr(lastContigArr)->end = ir-1;
                       contigsArray.ptr(lastContigArr)->accuCoverage = contigCoverageAccu;
                       contigLength = (contigsArray.ptr(lastContigArr)->end - contigsArray.ptr(lastContigArr)->start +1);
                       ps.totalContigLength += contigLength;
                       contigLength = contigCoverageAccu =0;
                   }
               }
               //needs to be in two steps to cover scenarios like c-c-c-c-c-g and g-g-g-g-g-c (c:coverage g:gap)
               if(ir+1==sublen) {
                   if(curState) { //close Contig
                       lastContigArr = contigsArray.dim() - 1;
                       contigsArray.ptr(lastContigArr)->end = ir;
                       contigsArray.ptr(lastContigArr)->accuCoverage = contigCoverageAccu;
                       contigLength = (contigsArray.ptr(lastContigArr)->end - contigsArray.ptr(lastContigArr)->start +1);
                       ps.totalContigLength += contigLength;
                       contigLength = contigCoverageAccu =0;
                   } else { //close Gap
                       if(ir)
                           ps.totalGapsNumber++;
                   }
               }
               prvState=curState;
           }


           withIndels_tot=tot;
           if((totF+totR)<=0 || (totF+totR)<SP->minCover){
               if (ir==0){
                   sIonAnnot::sIonPos wholeSequencePos;
                    wholeSequencePos.s32.start=0; //
                    wholeSequencePos.s32.end= sublen;
                    iannot->indexArr[1]=iannot->addRecord(sIonAnnot::ePos,sizeof(wholeSequencePos),&wholeSequencePos);
                    iannot->indexArr[2]=iannot->addRecord(sIonAnnot::eRecord,sizeof(ir),&ir);
                    idx dummyVal = ps.totalContigLength;
                      for (const char * p = elementToPut; p; p=sString::next00(p)){
                          sIPrintfFixBuf(ibuf,ilen,dummyVal,10,32);
                          addIannotAndDic(p,ibuf,ilen,recordIndexInfoToModify);
                          dummyVal++;
                      }
               }
               continue;
           }

           // output ATGC coverage and compute forward/reverse/total coverage
//           if(!isFirstLined){
               ++cnt;
//           } else{
//               isFirstLined = false;
//           }
//++coveragePos;
            if(iannot) { 
        sIonAnnot::sIonPos pos;
               pos.s32.start=ir+substart;
               pos.s32.end=pos.s32.start;
        iannot->indexArr[2]=iannot->addRecord(sIonAnnot::eRecord,sizeof(ir),&ir);
               iannot->indexArr[1]=iannot->addRecord(sIonAnnot::ePos,sizeof(pos),&pos);
            }
        for ( ic=0; ic<6 ; ++ic ) {
               v=line->atgcFwd[ic]+line->atgcRev[ic];

               if(v) {
                   sIPrintf(ibuf,ilen,v,10);
                   addIannot(ACGT_InDel[ic],ibuf,ilen);
               }
               if(v>3) withIndels_tot+=v;
           }
           if(totF) {
               sIPrintf(ibuf,ilen,totF,10);
               addIannot("Forward",ibuf,ilen);
           }
           idx qua=tot ? (line->quaFwd()+line->quaRev())/(tot) : 0;
           if(qua) {
               sIPrintf(ibuf,ilen,qua,10);
               addIannot("Quality",ibuf,ilen);
           }
           if(line->entr(0)>=3) {
               sIPrintf(ibuf,ilen,line->entr(0),10);
               addIannot("Entropy",ibuf,ilen);
           }
           if(line->entr(1)>=3) {
               sIPrintf(ibuf,ilen,line->entr(1),10);
               addIannot("SNP-Entropy",ibuf,ilen);
           }
           if (ir==0){
             //const void * body = getRecordBody(sIonAnnot::ePos,iannot->indexArr[1],&size);
               sIonAnnot::sIonPos wholeSequencePos;
              wholeSequencePos.s32.start=0; //
              wholeSequencePos.s32.end= sublen;
              iannot->indexArr[1]=iannot->addRecord(sIonAnnot::ePos,sizeof(wholeSequencePos),&wholeSequencePos);
              iannot->indexArr[2]=iannot->addRecord(sIonAnnot::eRecord,sizeof(ir),&ir);
              idx dummyVal = ps.totalContigLength;
               for (const char * p = elementToPut; p; p=sString::next00(p)){
                   sIPrintfFixBuf(ibuf,ilen,dummyVal,10,32);
                   addIannotAndDic(p,ibuf,ilen,recordIndexInfoToModify);
                   ++dummyVal;
               }
           }
           if (isContig){
              lastContigArr = contigsArray.dim() - 1;
              sIonAnnot::sIonPos contigPos;
              contigPos.s32.start=contigsArray.ptr(lastContigArr)->start; //
              contigPos.s32.end= contigPos.s32.start+1;

              iannot->indexArr[1]=iannot->addRecord(sIonAnnot::ePos,sizeof(contigPos),&contigPos);
              recordIndexContig = iannot->indexArr[1];
              recordIndexToModifyForContigs.vadd(1,recordIndexContig);

              iannot->indexArr[2]=iannot->addRecord(sIonAnnot::eRecord,sizeof(contigsArray.ptr(lastContigArr)->start),&contigsArray.ptr(lastContigArr)->start);

              //sStr contigInfo("%" DEC "",lastContigArr+1);
             // iannot->indexArr[3]=iannot->addRecord(sIonAnnot::eType,6 ,"contig");
             // iannot->indexArr[4]=iannot->addRecord(sIonAnnot::eID, contigInfo.length() ,contigInfo.ptr(0));
              //iannot->addRelationVarg(0,iannot->indexArr,0);
              sIPrintf(ibuf,ilen,lastContigArr+1,10);
              addIannot("contig",ibuf,ilen);
              isContig = false;
           }
           //
           real mutMin=1,mutMax=0, vv=0;
           //frequency table relative to reference
           for ( ic=0; ic<4 ; ++ic ) {
               v=line->atgcFwd[ic]+line->atgcRev[ic];
               vv=(ic==relativeLetter) ? 0 : ((real)v)/(tot ?tot : 1 );
               if( vv > mutMax ) mutMax = vv;
               if( vv < mutMin ) mutMin = vv;
           }

       }  // END of Loop through position

       // ##############  ingest CONTIGS info for ionAnnot

       if (iannot){
//           idx averageCoverageContig=0;//, sizeContig;
           for (idx iContig=0; iContig < contigsArray.dim(); ++iContig){     // loop through collected contigs array
                contigStruct * rg = contigsArray.ptr(iContig);
                ps.averageContigCoverage += (rg->accuCoverage * 1.0);
//                averageCoverageContig = (rg->accuCoverage * 1.0)/(rg->end - rg->start);
                // fix pos end for contigs record
                sIonAnnot::sIonPos * rangeBody = (sIonAnnot::sIonPos *)iannot->getRecordBody(sIonAnnot::ePos,recordIndexToModifyForContigs[iContig]); // ,&sizeContig
                rangeBody->s32.end = rg->end;
           }
           ps.totalGapLength = ps.reflen - ps.totalContigLength;
           ps.totalContigsNumber = contigsArray.dim();

           if(ps.totalGapsNumber && ps.totalContigLength) ps.averageGapCoverage/=ps.totalGapLength;
           if(ps.totalContigsNumber && ps.totalContigLength) ps.averageContigCoverage/=ps.totalContigLength;

           if (sublen) ps.contigsPart=100.0*ps.totalContigLength/sublen;
           ps.gapsPart=100.0-ps.contigsPart;

           // Fix value for total contig, gaps, length of contigs ....
           for (idx ik=0; ik < recordIndexInfoToModify.dim(); ++ik){
               const char * iid = (const char *) recordIndexInfoToModify.id(ik);
               idx * recordIndex = recordIndexInfoToModify.get(iid);
               idx * psize=0,ll;
               char * body = (char *)iannot->getRecordBody(sIonAnnot::eID,*recordIndex,&psize);
               if (strcmp("total_contig_length",iid)==0){
                   sIPrintf(body,ll,ps.totalContigLength,10);
               }
               else if (strcmp("mapped_coverage(percentage_reference)",iid)==0){
                   sFlPrintf(body,ll,ps.contigsPart,10,100);
               }
               else if (strcmp("average_coverage_of_contigs",iid)==0){
                   sIPrintf(body,ll,ps.averageContigCoverage,10);
               }
               else if (strcmp("total_number_of_contigs",iid)==0){
                   sIPrintf(body,ll,ps.totalContigsNumber,10);
               }
               else if (strcmp("total_length_of_the_unmapped_regions",iid)==0){
                   sIPrintf(body,ll,ps.totalGapLength,10);
               }
               else if (strcmp("unmapped_regions(percentage_reference)",iid)==0){
                   sFlPrintf(body,ll,ps.gapsPart,10,100);
               }
               else if (strcmp("average_coverage_of_gaps",iid)==0){
                   sIPrintf(body,ll,ps.averageGapCoverage,10);
               }
               else if (strcmp("total_number_of_gaps_found",iid)==0){
                   sIPrintf(body,ll,ps.totalGapsNumber,10);
               }
               *psize=sMax(ll+1,(idx)sizeof(idx)+1);
           }
       }
       // ##############
     return cnt;

}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  SNP noise functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


idx sBioseqSNP::snpCountNoise(SNPFreq * freq, const char * subseq, idx substart, idx sublen,  sDic < sVec < idx > > * noiseProfile, real noiseProfileResolution, real noiseProfileMax, sVec < HistogHistog > * histogramCoverage, idx minCoverage /*=0*/ )
{
    noiseProfile->mex()->flags|=sMex::fSetZero;

    char noiseType[32];

    // artificially enforce the order the pairs will appear in the list
    if(noiseProfile->dim()!=16) {
        for ( idx ic1=0; ic1<4 ; ++ic1 ) {
            for ( idx ic2=0; ic2<4 ; ++ic2) {
                if(ic1==ic2)
                    continue;
                sprintf(noiseType, "%c-%c",(char)sBioseq::mapRevATGC[ic1],(char)sBioseq::mapRevATGC[ic2]);
                noiseProfile->set(noiseType);
            }
        }
    }

    // now compute the noise of each kind
    for(idx ir=0; ir<sublen; ir+=1) {
        SNPFreq * line=(freq + ir );

        // determine consensus and referennce letters
        idx ic, v , vmax=-10000; // refLet=sBioseq::mapATGC[(idx)s[ir-0]]; // -substart
        idx refLet = sBioseqAlignment::_seqBits(subseq, ir+substart , 0), tot=0;
        for ( ic=0; ic<4 ; ++ic ) {
            v=line->atgcFwd[ic]+line->atgcRev[ic];
            if(vmax<v){vmax=v;}
            tot+=v;
        }
        if(!tot)continue;
        if (tot < minCoverage) continue;
        idx relativeLetter= refLet; // (SP->snpCompare==0 ) ? consLet : refLet ;


        for ( ic=0; ic<4 ; ++ic ) {
            if( relativeLetter==ic)continue;
            v=line->atgcFwd[ic]+line->atgcRev[ic];
            real vv=((real)v)/tot;

            if(vv>=noiseProfileMax)continue;
            if(!vv)continue;
            idx ibin=(idx)((vv+0.1*noiseProfileResolution)/noiseProfileResolution);
            char noiseType[32]; sprintf(noiseType, "%c-%c",(char)sBioseq::mapRevATGC[relativeLetter],(char)sBioseq::mapRevATGC[ic]);
            sVec <idx > * nt=noiseProfile->set(noiseType);
            nt->flagOn(sMex::fSetZero);
            nt->resize(ibin+1);
            (*nt)[ibin]++;

            if(histogramCoverage){
                histogramCoverage->resize(ibin+1);
                sBioseqSNP::HistogHistog * hist=histogramCoverage->ptr(ibin);
                if (hist) {
                    char histType[32]; sprintf(histType, "%" DEC,v);
                    hist->countForCoverage[ic][histType]++;
                }
            }
        }

    }

    return 1;
}

idx sBioseqSNP::snpOutHistog(sVec <HistogHistog> & histogramCoverage, real step, sFil & histProf,idx iSub, bool printHeader /*= true*/)
{
    if (!(step>0) || !(histProf.ok()) )
        return 0;
    if( printHeader ) {
        if( iSub ) {
            histProf.printf("Reference,");
        }
        histProf.printf("SNPfrequency,letter,histogram\n"); //empty header
    }
    for(idx i = 0; i < histogramCoverage.dim(); ++i) {
        real f = i * step;
        sBioseqSNP::HistogHistog * hist = histogramCoverage.ptr(i);

        for(idx ia = 0; ia < 4; ++ia) {
            sDic<idx> & dic = hist->countForCoverage[ia];
            if( dic.dim() ) {
                if( iSub )
                    histProf.printf("%" DEC ",", iSub);
                histProf.printf("%g,%c,\"", f, sBioseq::mapRevATGC[ia]);
                for(idx j = 0; j < dic.dim(); ++j) {
                    const char * k = (const char *) dic.id(j);
                    idx val = dic[j];
                    if( j ) {
                        histProf.printf(",");
                    }
                    // assume k is string representing an integer (position)
                    histProf.printf("%s,%" DEC, k, val);
                }
                histProf.printf("\"\n");
            }
        }
    }
    return 1;
}

idx sBioseqSNP::snpOutNoise(sStr * out, sStr * noiseCuts, real noiseProfileResolution, sDic < sVec < idx > > * noiseProfile, real Ctof, sDic<sVec<real> > * noiseIntegrals, idx iSub, bool printHeaders /*= true*/)
{
    sVec<idx> noiseSum(sMex::fSetZero);

    snpSumNoisePrint(out,iSub,noiseProfileResolution, *noiseProfile, noiseSum, printHeaders);

    if( noiseCuts ) {
        const real *ctof = noiseCutoffThresholds;
        idx lenct=noiseCutoffThresholds_NUM;
        // so we can demand computing a single particular value from caller function
        if(Ctof){ctof=&Ctof;lenct=1;}
        sDic<sVec<real> > NoiseIntegrals;if(!noiseIntegrals)noiseIntegrals=&NoiseIntegrals;

        snpComputeIntegrals(*noiseProfile,noiseSum,*noiseIntegrals,noiseProfileResolution,ctof, lenct);

        snpOutIntegrals(*noiseCuts,*noiseIntegrals,iSub,printHeaders);
    }
    return 1;
}

idx sBioseqSNP::snpOutIntegrals(sStr & out, sDic<sVec<real> > & integrals, idx iSub, bool printHeaders) {
    idx prevLen = out.length();
    if(printHeaders) {
        if( iSub ) out.addString("Reference,");
        out.addString("Threshold");
        for(idx ic=0;ic<4 ;++ic) {
            for(idx iv=0;iv<4;++iv) {
                if(ic==iv)
                    continue;
                out.printf( ",%c-%c",(char)sBioseq::mapRevATGC[ic],(char)sBioseq::mapRevATGC[iv]);
            }
        }
        out.addString("\n");
    }
    for(idx il=0;il<integrals.dim();++il)
    {
        if(iSub) out.printf("%" DEC ",",iSub);

        sVec<real> * intgrl = integrals.ptr(il);
        out.printf("%s",(const char *)integrals.id(il));
        for ( idx ins=0; ins<4 ; ++ins) {
            for(idx ine=0; ine<4 ; ++ine){
                if(ins==ine)
                    continue;
                out.printf(",%.4lf",(*intgrl)[4*ins + ine]);
            }
        }
        out.addString("\n");
    }
    return out.length()-prevLen;
}

idx sBioseqSNP::snpSumNoisePrint(sStr * out, idx iSub, real noiseProfileResolution, sDic < sVec < idx > > & noiseProfile, sVec<idx> &noiseSum, bool printHeaders)
{
    idx maxBin=0;
    noiseSum.cut(0);noiseSum.setflag(sMex::fSetZero|sMex::fExactSize);noiseSum.add(16);
    // now compute the maximum length of the noise histogram and output the headers at the same time
    if(out && iSub && printHeaders) out->addString("Reference,");
    if(out && printHeaders)out->addString("Frequency");
    for ( idx ins=0; ins<4 ; ++ins) {
        for ( idx ine=0; ine<4 ; ++ine) {

            if(ins==ine)
                continue;

            char noiseType[32];sprintf(noiseType,"%c-%c",(char)sBioseq::mapRevATGC[ins],(char)sBioseq::mapRevATGC[ine]);
            sVec<idx> * noiseVec=noiseProfile.get(noiseType);
            if(noiseVec){
                if(out && printHeaders)out->printf(",%s",noiseType);
                if(maxBin<noiseVec->dim())
                   maxBin=noiseVec->dim();
            }
        }
    }
    if(out && printHeaders)out->addString("\n");

    // now print the values for the noise histogram
    for( idx ib=1; ib<maxBin; ++ib ) {
        if(out && iSub) out->printf("%" DEC ",",iSub);
        if(out)out->printf("%lg",noiseProfileResolution*ib);
        for ( idx ins=0; ins<4 ; ++ins) {
            for(idx ine=0; ine<4 ; ++ine){
                if(ins==ine)
                    continue;
                char noiseType[32];sprintf(noiseType,"%c-%c",(char)sBioseq::mapRevATGC[ins],(char)sBioseq::mapRevATGC[ine]);
                sVec<idx> * noiseVec=noiseProfile.get(noiseType);
                if(!noiseVec) {
                    if(out)out->printf(",0");
                    continue;
                }

                idx v= (ib>=noiseVec->dim()) ? 0 : *(noiseVec->ptr(ib));
                if(out)out->printf(",%" DEC,v);

                // also accumulate the totals on each letter combination
                noiseSum[4*ins + ine]+=v;
            }
        }
        if(out)out->addString("\n");
    }
    return maxBin;
}

//static
idx sBioseqSNP::snpComputeIntegrals( sDic < sVec < idx > > & noiseProfile, sVec<idx> & noiseSum, sDic<sVec<real> > & integrals, real noiseProfileResolution, const real * Ctof , idx Ctof_size )
{
    idx cnt=0;

    for ( idx ins=0; ins<4 ; ++ins) {
        for(idx ine=0; ine<4 ; ++ine){
            if(ins==ine)
                continue;
            char noiseType[32];sprintf(noiseType,"%c-%c",(char)sBioseq::mapRevATGC[ins],(char)sBioseq::mapRevATGC[ine]);
            sVec<idx> * noiseVec=noiseProfile.get(noiseType);

            if(!noiseVec)
                continue;

            real accumulated=0;

            for(idx ir=0;ir<noiseVec->dim();++ir){
                accumulated+=(*noiseVec->ptr(ir));
                for(idx ii=0;ii<Ctof_size;++ii){
                    char intgrl[32]; sprintf(intgrl, "%.4lf",Ctof[ii]);
                    sVec<real> * nsThrsld = integrals.set(intgrl);
                    if( !nsThrsld->dim() ) {
                        nsThrsld->setflag(sMex::fSetZero);
                        nsThrsld->add(noiseSum.dim());
                    }
                    if(accumulated>(real)(Ctof[ii]*noiseSum[4*ins + ine]) && (*nsThrsld)[4*ins + ine]==0){
                        ++cnt;
                        (*nsThrsld)[4*ins + ine]=noiseProfileResolution*ir;
                    }
                }
            }
        }
    }
    return cnt;
}

idx sBioseqSNP::aminoacidDecode( const char * seq, idx position, ionRange * ranges,char * AA, idx * threeCodeOffset) {
    idx discontinuous=0;

    idx aaPos=position,iA=0,bApos=0,iAA=0,iAAEnd=0,diffP=0,diffN=0,back=0;

    if(!ranges){
        bApos=aaPos%3;
        iAA=position-bApos;
        iAAEnd=iAA+3;

        for(; iAA<iAAEnd; ++iAA, ++iA){
            AA[0]|= (sBioseqAlignment::_seqBits(seq, iAA , 0)<<(2*iA));
        }
        if(threeCodeOffset)
            *threeCodeOffset=bApos;
        return (aaPos/3);
    }
    aaPos=aaPos-(ranges->start-1); // first range
    bApos=aaPos%3;
    idx subRangeHit=0, rnaLength = ranges->end - ranges->start, transcriptLength = rnaLength;
    sVec < idx > rangeGaps;

    if(ranges->connectedRanges.dim() >1){
        // find the subRange
        for(idx sR=0; sR < ranges->connectedRanges.dim(); ++sR){ // try to get the range where the position is in within start and end
            if( ( position <= (ranges->connectedRanges[sR].start-1) )? (( (ranges->connectedRanges[sR].start-1) <= position)?1:0) : ((position <= (ranges->connectedRanges[sR].end-1) )?1:0)){
                subRangeHit=sR; // when found, exit thressthe loop
//                break;
            }
        }
        //if annotation is in joins then place your self in the frame
        if(subRangeHit){
            // find the gaps number
            rangeGaps.add(ranges->connectedRanges.dim());
            rangeGaps[0] = 0;
            for(idx sR=1; sR< ranges->connectedRanges.dim(); ++sR){
                rangeGaps[sR] = rangeGaps[sR-1] + ( ranges->connectedRanges[sR].start - ranges->connectedRanges[sR-1].end );
            }

            aaPos=aaPos-rangeGaps[subRangeHit];
            bApos=aaPos%3;
        }
        transcriptLength -= rangeGaps[ranges->connectedRanges.dim()-1];

        diffP=position -  (ranges->connectedRanges[subRangeHit].start-1); // *ranges->rangeStart.ptr(ranges->subRangeHit);
        if(diffP>=0 && diffP<2 && bApos>diffP){
            discontinuous=1;
        }
        // diffN= *ranges->rangeStart.ptr(ranges->subRangeHit)-position;
        diffN= (ranges->connectedRanges[subRangeHit].end-1) - position; //*ranges->rangeStart.ptr(ranges->subRangeHit)-position;
        if(diffN>=0 && diffN<2 && bApos<diffN){
            discontinuous=2;
        }
    }

    switch (discontinuous){
        case 0:
            iAA=position-bApos;
            iAAEnd=iAA+3;
            if(!ranges)return (char)(-1);
            for(; iAA<iAAEnd; ++iAA, ++iA){
                AA[0]|= (sBioseqAlignment::_seqBits(seq, iAA , 0)<<(2*iA));
            }

            break;
        case 1:
            if(subRangeHit)
                return (char)(-1);
            back = bApos-diffP;//,ipos=0,nst=0,iA;
            iAA = (ranges->connectedRanges[subRangeHit-1].end-1) - back+1; // ranges->rangeEnd[ranges->subRangeHit-1]-back+1;
            for(;iA<back; ++iAA){
                AA[0]|= (sBioseqAlignment::_seqBits(seq, iAA , 0)<<(2*(iA++)));
            }
            iAA = position - (ranges->connectedRanges[subRangeHit].start-1); // ranges->rangeStart[ranges->subRangeHit]
            for(;iA<3;++iAA){
                AA[0]|= (sBioseqAlignment::_seqBits(seq, iAA , 0)<<(2*(iA++)));
            }
            break;
        case 2:
            if( subRangeHit >= ranges->connectedRanges.dim() )
                return -1;

            back = diffN-bApos;//,ipos=0,nst=0;
            iAA = position-bApos;

            for( ; iAA < (ranges->connectedRanges[subRangeHit].end-1) ; ++iAA){
                AA[0]|= (sBioseqAlignment::_seqBits(seq, iAA , 0)<<(2*(iA++)));
            }
            iAA = ranges->connectedRanges[subRangeHit+1].start;//ranges->rangeStart[ranges->subRangeHit+1];
            for(;iA<3;++iAA){
                AA[0]|= (sBioseqAlignment::_seqBits(seq, iAA , 0)<<(2*(iA++)));
            }
            break;
    }
    if( !ranges->forward ) {
        char t_aa = AA[0];
        AA[0] = 0;
        AA[0] |= sBioseq::mapComplementATGC[(t_aa&3)]<<4;
        AA[0] |= sBioseq::mapComplementATGC[(t_aa&(3<<2))>>2]<<2;
        AA[0] |= sBioseq::mapComplementATGC[(t_aa&(3<<4))>>4];

        bApos = 2 - bApos;
        aaPos = transcriptLength - aaPos;
    }

    if(threeCodeOffset)*threeCodeOffset =bApos;// AA & (~(0x3 << (2 * bApos)));
    return (aaPos/3);
}

//static
idx sBioseqSNP::baseFrameDecode( idx & base, idx pos, ionRange * ranges,idx & codon, idx * codonOffset)
{
    idx codonPos = 0, aaPos = 0;

    if(!ranges){
        codonPos=pos%3;
        aaPos = pos/3;
    } else if ( !sOverlap(pos,pos,ranges->start,ranges->end) ) {
        aaPos = -1;
    } else {
        idx exonHit=-1, transcriptLength  = 1 + ranges->end - ranges->start;

        if(ranges->connectedRanges.dim() >1){
            // find the subRange
            transcriptLength = 0;
            for(idx sR=0; sR < ranges->connectedRanges.dim(); ++sR){ // try to get the range where the position is in within start and end
                if( sOverlap(pos,pos,ranges->connectedRanges[sR].start,ranges->connectedRanges[sR].end) ){
                    exonHit=sR;
                    codonPos = ((pos - ranges->connectedRanges[exonHit].start) + transcriptLength);
                    aaPos = codonPos/3;
                    codonPos = codonPos%3;
                }
                transcriptLength += ranges->connectedRanges[sR].end + 1 - ranges->connectedRanges[sR].start;
            }

            if(exonHit<0) aaPos = -1;
        } else {
            codonPos = (pos - ranges->start) % 3;
            aaPos = (pos-ranges->start)/3;
        }
        if(!ranges->forward && aaPos >= 0 ) {
            aaPos = transcriptLength/3 - aaPos;
            codonPos = 2 - codonPos;
        }
    }
    if (aaPos>=0) {
        codon |= (base)<<(2*codonPos);
        if(codonOffset) {
            if(codonPos != (1+*codonOffset)%3 )
                aaPos=-1;
            *codonOffset = codonPos;
        }
    }

    return aaPos;
}

idx sBioseqSNP::protSeqGeneration(const char * subseq, const char * subName,idx sublen,sStr * seqOut){

       return 1;
}
void sBioseqSNP::SNPRecord::printCSV(sStr &out, idx snpCompare) const
{
    // Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T
    out.printf("%" UDEC ",%c,%c,", position, letter, consensus);
    for (idx i=0; i<4; i++)
        out.printf("%" DEC ",", atgc[i]);
    for (idx i=0; i<2; i++)
        out.printf("%" DEC ",", indel[i]);
    out.printf("%" DEC ",%" DEC ",%" DEC ",%" DEC ",", countFwd+countRev, countFwd, countRev, qua);
    out.printf((entrScaled < 0.01) ? "0," : "%.2lg", entrScaled);
    out.printf((snpentrScaled < 0.01) ? "0," : "%.2lg", snpentrScaled);
    for (idx i=0; i<4; i++) {
        real f = freq(i, snpCompare);
        if (f < 1.e-4)
            out.printf("0");
        else
            out.printf("%.2lg", f*100);

        if (i < 3)
            out.printf(",");
    }
}

void sBioseqSNP::SNPRecord::printCSV_withSubject(sStr &out, idx snpCompare) const
{
    // Reference,Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T
    out.printf("%" DEC "",iSub);
    out.printf("%" UDEC ",%c,%c,", position, letter, consensus);
    for (idx i=0; i<4; i++)
        out.printf("%" DEC ",", atgc[i]);
    for (idx i=0; i<2; i++)
        out.printf("%" DEC ",", indel[i]);
    out.printf("%" DEC ",%" DEC ",%" DEC ",%" DEC ",", countFwd+countRev, countFwd, countRev, qua);
    out.printf((entrScaled < 0.01) ? "0," : "%.2lg", entrScaled);
    out.printf((snpentrScaled < 0.01) ? "0," : "%.2lg", snpentrScaled);
    for (idx i=0; i<4; i++) {
        real f = freq(i, snpCompare);
        if (f < 1.e-4)
            out.printf("0");
        else
            out.printf("%.2lg", f*100);

        if (i < 3)
            out.printf(",");
    }
}

idx sBioseqSNP::printCSV(sBioseqSNP::SNPRecord * snpRecord, sBioseqSNP::ParamsProfileIterator * params, idx iNum)
{
    if( !(snpRecord->countFwd+snpRecord->countRev) ){
        snpRecord->consensus = '-';
        snpRecord->letter = ((sStr *)params->userPointer)->ptr(snpRecord->position-1)[0];
    }
    //userIndex carries the snpCompare flag that defines what frequency will be calculated on (consensus or reference letter)
    snpRecord->printCSV(*params->str,params->userIndex);
    params->str->printf("\n");
    return 1;
}

//bool sBioseqSNP::SNPRecord::parseCSV(const char *buf, const char *bufend)
//{
//    if (!buf)
//        return false;
//
//    idx dummy;
//
//    // Position,Letter,Consensus,
//    // Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,
//    // Count Total,Count Forward,Count Reverse,
//    // Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T
//    return (sString::bufscanf(buf, bufend, "%" UDEC ",%c,%c,"
//        "%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ","
//        "%" DEC ",%" DEC ",%" DEC ","
//        "%" DEC ",%lf,%lf,%*f,%*f,%*f,%*f",
//        &position, &letter, &consensus,
//        atgc, atgc+1, atgc+2, atgc+3, indel, indel+1,
//        &dummy, &countFwd, &countRev,
//        &qua, &entrScaled, &snpentrScaled) == 15);
//}

idx translationTable[256];



idx sizeofSet[]={
    sizeof(sBioseqSNP::SNPRecord::iSub),0,
    sizeof(sBioseqSNP::SNPRecord::position),0,
    sizeof(sBioseqSNP::SNPRecord::letter),2,
    sizeof(idx)-sizeof(sBioseqSNP::SNPRecord::letter),2,
    sizeof(sBioseqSNP::SNPRecord::atgc[0]),0,
    sizeof(sBioseqSNP::SNPRecord::atgc[1]),0,
    sizeof(sBioseqSNP::SNPRecord::atgc[2]),0,
    sizeof(sBioseqSNP::SNPRecord::atgc[3]),0,
    sizeof(sBioseqSNP::SNPRecord::indel[0]),0,
    sizeof(sBioseqSNP::SNPRecord::indel[1]),0,
    0,-1,
    sizeof(sBioseqSNP::SNPRecord::countFwd),0,
    sizeof(sBioseqSNP::SNPRecord::countRev),0,
    sizeof(sBioseqSNP::SNPRecord::qua),0,
    sizeof(sBioseqSNP::SNPRecord::entrScaled),1,
    sizeof(sBioseqSNP::SNPRecord::snpentrScaled),1
};

bool sBioseqSNP::SNPRecord::parseCSV(const char *buf, const char *bufend, idx icolMax, bool withSub)
{
    if (!buf)
        return false;
    if(translationTable[0]==0){
        translationTable[0]=255;
        translationTable['0']=0;
        translationTable['1']=1;
        translationTable['2']=2;
        translationTable['3']=3;
        translationTable['4']=4;
        translationTable['5']=5;
        translationTable['6']=6;
        translationTable['7']=7;
        translationTable['8']=8;
        translationTable['9']=9;
        translationTable['A']='A';
        translationTable['C']='C';
        translationTable['G']='G';
        translationTable['T']='T';
    }

    if( !icolMax ) {
        icolMax=sDim(sizeofSet)/2-1;
    }
    memset((void*)this,0,sizeof(*this));


    idx * curValPtr=(idx*)&this->iSub;
    idx icol=0;
    if(!withSub) {
        curValPtr = (idx*)&this->position;
        icol=1;
    }
    idx divideBy=1;


    for ( ; buf<bufend ; ++buf ){
        if(* buf == ',' ) {
            if(icol+1>=icolMax) //if(icol>=sDim(sizeofSet)/2-1)
                break;
            if(divideBy>1) { // for real numbers
                *((real*)curValPtr)/=divideBy;
            }
            curValPtr=(idx*)sShift(curValPtr, (sizeofSet[icol*2]) ) ;
            if(divideBy)divideBy=0;
            ++icol;
            continue;
        }
        else if(sizeofSet[icol*2+1]==-1) // skipped columns
            continue;
        else if(* buf == '.' ) {
            divideBy=1;
            continue;
        }
        if(sizeofSet[icol*2+1]==0) {
            *curValPtr=*curValPtr *10 +  translationTable[(idx)(*buf)];
        }
        else if( sizeofSet[icol*2+1]==1 ) {
            *((real*)curValPtr)=*((real*)curValPtr) *10 +  translationTable[(idx)(*buf)];
            if(divideBy)divideBy*=10;
        }
        else {
            *((char*)curValPtr)=translationTable[(idx)(*buf)];
        }
    }
    if(divideBy>1) { // for real numbers
        *((real*)curValPtr)/=divideBy;
    }
    return true;
}

bool sBioseqSNP::SNPRecord::parseCSV_withSubject(const char *buf, const char *bufend, idx icolMax)
{
    if (!buf)
        return false;
    if(translationTable[0]==0){
        translationTable[0]=255;
        translationTable['0']=0;
        translationTable['1']=1;
        translationTable['2']=2;
        translationTable['3']=3;
        translationTable['4']=4;
        translationTable['5']=5;
        translationTable['6']=6;
        translationTable['7']=7;
        translationTable['8']=8;
        translationTable['9']=9;
        translationTable['A']='A';
        translationTable['C']='C';
        translationTable['G']='G';
        translationTable['T']='T';
    }

    if( !icolMax ) {
        icolMax=sDim(sizeofSet)/2-1;
    }
    memset((void*)this,0,sizeof(*this));

    idx * curValPtr=(idx*)&(this->iSub);
    idx divideBy=1;


    for ( idx icol=0; buf<bufend ; ++buf ){
        if(* buf == ',' ) {
            if(icol+1>=icolMax) //if(icol>=sDim(sizeofSet)/2-1)
                break;
            if(divideBy>1) { // for real numbers
                *((real*)curValPtr)/=divideBy;
            }
            curValPtr=(idx*)sShift(curValPtr, (sizeofSet[icol*2]) ) ;
            if(divideBy)divideBy=0;
            ++icol;
            continue;
        }
        else if(sizeofSet[icol*2+1]==-1) // skipped columns
            continue;
        else if(* buf == '.' ) {
            divideBy=1;
            continue;
        }
        if(sizeofSet[icol*2+1]==0) {
            *curValPtr=*curValPtr *10 +  translationTable[(idx)(*buf)];
        }
        else if( sizeofSet[icol*2+1]==1 ) {
            *((real*)curValPtr)=*((real*)curValPtr) *10 +  translationTable[(idx)(*buf)];
            if(divideBy)divideBy*=10;
        }
        else {
            *((char*)curValPtr)=translationTable[(idx)(*buf)];
        }
    }
    if(divideBy>1) { // for real numbers
        *((real*)curValPtr)/=divideBy;
    }
    return true;
}
/*
bool sBioseqSNP::SNPRecord::parseCSV_withSubject(const char *buf, const char *bufend)
{
    if (!buf)
        return false;

    idx dummy;

    // Reference,Position,Letter,Consensus,
    // Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,
    // Count Total,Count Forward,Count Reverse,
    // Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T
    return (sString::bufscanf(buf, bufend, "%u,%u,%c,%c,"
        "%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ","
        "%" DEC ",%" DEC ",%" DEC ","
        "%" DEC ",%lf,%lf,%*f,%*f,%*f,%*f",
        &iSub,&position, &letter, &consensus,
        atgc, atgc+1, atgc+2, atgc+3, indel, indel+1,
        &dummy, &countFwd, &countRev,
        &qua, &entrScaled, &snpentrScaled) == 16);
}
*/
static const char* skipToNextLine(const char *buf, const char *bufend)
{
    while (buf < bufend && *buf && *buf != '\r' && *buf != '\n')
        buf++;
    while (buf < bufend && (*buf == '\r' || *buf == '\n'))
        buf++;
    return buf;
}


static const char* goToPreviousLine(const char *buf, const char *bufstart)
{
    if(buf <= bufstart)return 0;
    while (buf > bufstart && *buf && *buf != '\r' && *buf != '\n')
        buf--;
    while (buf > bufstart && (*buf == '\r' || *buf == '\n'))
        buf--;
    while (buf > bufstart && *buf && *buf != '\r' && *buf != '\n')
        buf--;
    return (*buf == '\r' || *buf == '\n')?++buf:buf;
}

static const char* goToBeginingofLine(const char *buf, const char *bufstart)
{
    while (buf > bufstart && (*buf == '\r' || *buf == '\n'))
        buf--;
    while (buf > bufstart && *buf && *buf != '\r' && *buf != '\n')
        buf--;
    return (*buf == '\r' || *buf == '\n')?++buf:buf;
}
const char * sBioseqSNP::SNPRecordPrevious(const char *buf, SNPRecord *rec, const char *bufstart)
{
    //assert (rec);

    const char * bufend = buf;
    if (!buf)
        return NULL;

    // Skip header if it exists
    if (*buf < '0' || *buf > '9')
        buf = goToPreviousLine(buf, bufstart);

    if (buf < bufstart || !*buf)
        return NULL;

    if (!rec->parseCSV(buf, bufend))
        return NULL;

    return goToPreviousLine(buf, bufstart);
}

const char * sBioseqSNP::SNPConcatenatedRecordPrevious(const char *buf, SNPRecord *rec, const char *bufstart,idx iSub, idx icolMax)
{
    //assert (rec);

    const char * bufend = buf;
    if (!buf)
        return NULL;

    // Skip header if it exists
    if (*buf < '0' || *buf > '9')
        buf = goToPreviousLine(buf, bufstart);

    if (buf < bufstart || !*buf)
        return NULL;

    if (!rec->parseCSV_withSubject(buf, bufend, icolMax) || (iSub && rec->iSub!=iSub))
        return NULL;

    return goToPreviousLine(buf, bufstart);
}

const char * sBioseqSNP::SNPRecordNext(const char *buf, SNPRecord *rec, const char *bufend)
{
    //assert (rec);

    if (!buf)
        return NULL;

    if (!bufend)
        bufend = buf + strlen(buf);

    // Skip header if it exists
    if (*buf < '0' || *buf > '9')
        buf = skipToNextLine(buf, bufend);

    if (buf >= bufend || !*buf)
        return NULL;

    if (!rec->parseCSV(buf, bufend))
        return NULL;

    return skipToNextLine(buf, bufend);
}


const char * sBioseqSNP::SNPConcatenatedRecordNext(const char *buf, SNPRecord *rec, const char *bufend,idx iSub, idx icolMax)
{
    //assert (rec);

    if (!buf)
        return NULL;

    if (!bufend)
        bufend = buf + strlen(buf);

    // Skip header if it exists
    if (*buf < '0' || *buf > '9')
        buf = skipToNextLine(buf, bufend);

    if (buf >= bufend || !*buf)
        return NULL;

    if (! rec->parseCSV_withSubject(buf, bufend, icolMax) || (iSub && rec->iSub!=iSub))
        return NULL;

    return skipToNextLine(buf, bufend);
}

//static
const char * sBioseqSNP::binarySearchReference(const char * buf, const char * bufend, idx iSub, bool wantEnd/* = false */)
{
    //const char * lineStart = profile->ptr();
    //const char * lineEnd = profile->ptr()+profile->length();
    SNPRecord rec;
    const char * lineMin=buf; //goToBeginingofLine(buf, bufend);// lineStart;
    // Skip header if it exists
    if (*lineMin < '0' || *lineMin > '9')
        lineMin = skipToNextLine(buf, bufend);

    const char * lineBest = 0;
    rec.parseCSV_withSubject(lineMin, bufend, 1 );
    if( rec.iSub == iSub ) {
        if( wantEnd ) {
            lineBest = lineMin;
        } else {
            return lineMin;
        }
    } else if( rec.iSub > iSub ) {
        return 0;
    }

    const char * lineMax=goToBeginingofLine(bufend-1, lineMin);
    rec.parseCSV_withSubject(lineMax, bufend, 1 );
    if( rec.iSub == iSub ) {
        if( wantEnd ) {
            return bufend;
        } else {
            lineBest = lineMax;
        }
    } else if( rec.iSub<iSub ) {
        return 0;
    }

    while(lineMax>=lineMin && lineMin){
        const char * lineMid=lineMin+((lineMax-lineMin)/2);
        lineMid = goToBeginingofLine(lineMid,lineMin);
        const char * next_line = SNPConcatenatedRecordNext(lineMid, &rec, bufend,0,1);
        if(rec.iSub < iSub) {
            lineMin=next_line;
        } else if (rec.iSub > iSub) {
            lineMax=goToPreviousLine(lineMid, buf);
        } else {
            // we are in the correct iSub
            lineBest = lineMid;
            if( !wantEnd && lineMin != lineMid ) {
                lineMax = goToPreviousLine(lineMid, buf);
            } else if( wantEnd && lineMax != lineMid ) {
                lineMin = skipToNextLine(lineMid, bufend);
            } else {
                const char * lineMidEnd = SNPConcatenatedRecordNext(lineMid, &rec, bufend, iSub); // read all values now
                return wantEnd ? lineMidEnd : lineMid;
            }
        }

    }

    return wantEnd ? SNPConcatenatedRecordNext(lineBest, &rec, bufend, iSub) : lineBest;
}

//static
const char * sBioseqSNP::binarySearchReferenceNoise(const char * buf, const char * bufend, idx iSub,bool wantEnd /*= false */)
{
    const char * lineMin=buf;
    // Skip header if it exists
    if (*lineMin < '0' || *lineMin > '9')
        lineMin = skipToNextLine(buf, bufend);

    const char * lineBest = 0;
    idx curSub;
    noiseProfileSubFromConcatenatedCSV(lineMin,bufend,0,&curSub);
    if( curSub == iSub ) {
        if( wantEnd ) {
            lineBest = lineMin;
        } else {
            return lineMin;
        }
    } else if( curSub > iSub ) {
        return 0;
    }

    const char * lineMax=goToBeginingofLine(bufend-1, lineMin);
    noiseProfileSubFromConcatenatedCSV(lineMax,bufend,0,&curSub);
    if( curSub == iSub ) {
        if( wantEnd ) {
            return bufend;
        } else {
            lineBest = lineMax;
        }
    } else if( curSub<iSub ) {
        return 0;
    }

    while(lineMax>=lineMin && lineMin){
        const char * lineMid=lineMin+((lineMax-lineMin)/2);
        lineMid = goToBeginingofLine(lineMid,lineMin);
        const char * next_line = noiseProfileSubFromConcatenatedCSV(lineMid,bufend,0,&curSub);
        if(curSub < iSub) {
            lineMin=next_line;
        } else if (curSub > iSub) {
            lineMax=goToPreviousLine(lineMid, buf);
        } else {
            // we are in the correct iSub
            lineBest = lineMid;
            if( !wantEnd && lineMin != lineMid ) {
                lineMax = goToPreviousLine(lineMid, buf);
            } else if( wantEnd && lineMax != lineMid ) {
                lineMin = skipToNextLine(lineMid, bufend);
            } else {
                const char * lineMidEnd = noiseProfileSubFromConcatenatedCSV(lineMid,bufend,0,&curSub);
                if( curSub != iSub ) lineMidEnd = 0;
                return wantEnd ? lineMidEnd : lineMid;
            }
        }

    }
    if( !wantEnd ) return lineBest;
    lineBest = noiseProfileSubFromConcatenatedCSV(lineBest,bufend,0,&curSub);
    if( curSub != iSub ) lineBest = 0;
    return lineBest;
}

// ionWanderCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults)
idx sBioseqSNP::traverserCallback (sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist){

    if(!statement->label)
        return 1;

    sVec < ionRange > * rangeVec = (sVec <ionRange> *) wander->callbackFuncParam;

    if( memcmp(statement->label,"restriction",10)==0) {
        //seqID|pos|record|type|id
        sStr seqid; seqid.addString((const char *)reslist[1].body,reslist[1].size);
        idx record = *((idx*) (reslist[3].body));

        ionRange * a = rangeVec->add();
        a->start = (idx) (((int *)(reslist[2].body))) [1];
        a->end = (idx) (((int *)(reslist[2].body))) [0];
        a->recordIndex = record;
    }
    if (memcmp(statement->label,"askJoin",6)==0) {
        idx record = *((idx*) (reslist[3].body));
        sStr connectedRanges; connectedRanges.addString((const char *)reslist[5].body,reslist[5].size);
        for (idx iv=0; iv<rangeVec->dim(); ++iv){
            if (rangeVec->ptr(iv)->recordIndex == record){
                sStr splitByZero; sString::searchAndReplaceStrings(&splitByZero,connectedRanges,0,";",0,0,false);
                for (const char * p = splitByZero.ptr(0); p; p = sString::next00(p)){
                    char * dash;
                    startEndAnnot * a = rangeVec->ptr(iv)->connectedRanges.add();
                    a->start = strtoidx(p,&dash,10); p=dash+1;
                    a->end = strtoidx(p,&dash,10);
                }
                break;
            }
        }
    }
    idx checkCase = (memcmp(statement->label,"proteinId",8)==0) ? 1 : ( (memcmp(statement->label,"askStrand",9)==0) ? 2 : 0);
    if (checkCase) {
        idx record = *((idx*) (reslist[3].body));
        sStr elementBody; elementBody.addString((const char *)reslist[5].body,reslist[5].size);
        for (idx iv=0; iv<rangeVec->dim(); ++iv){
            if (rangeVec->ptr(iv)->recordIndex == record && checkCase==1){
                rangeVec->ptr(iv)->proteinId.printf(0,elementBody.ptr(0));
                break;
            }
            if (rangeVec->ptr(iv)->recordIndex == record && checkCase==2){
                rangeVec->ptr(iv)->forward = (strncmp(elementBody.ptr(0),"-",1)==0) ? false : true ;
                break;
            }
        }
    }
    return 1;
}

//static
bool sBioseqSNP::createAnnotationIonRangeQuery( sStr & qry, const char * seqID, const char * record ) {
    if( seqID && record ) {
        return qry.printf("restriction=find.annot(seqID=%s,record=%s,id=CDS);"
            "proteinId=find.annot(seqID=restriction.seqID,record=restriction.record,type=protein_id);"
            "askStrand=find.annot(seqID=restriction.seqID,record=restriction.record,type=strand);"
            "askJoin=find.annot(seqID=restriction.seqID,record=restriction.record,type=listOfConnectedRanges);", seqID, record);
    } else {
        return false;
    }
}

//static
bool sBioseqSNP::createAnnotationSearchIonRangeQueries(sStr & qry1 , sStr & qry2 , const char * seqID, const char * start/*="$start"*/, const char * end/*="$end"*/) {
    if( !seqID ) {
        return false;
    }
    qry1.printf(0,"seq=foreach('%s');a=find.annot(#range=possort-max,seq.1,%s,seq.1,%s);", seqID, start, end );
    createAnnotationIonRangeQuery(qry1,"a.seqID","a.record");

    qry2.cut0cut();
    const char * nxt, *seqid=seqID;

    idx sizeSeqId = 0, i = 0;
    sStr seqQry,seqLbls;
    const char * seqLbl = 0;
    for( const char * p=seqid; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){ // scan until pipe | separated types and ids are there

        const char * curType=p;
        nxt=strpbrk(p,"| "); // find the separator
        if(!nxt || *nxt==' ')
            break;

        const char * curId=nxt+1;
        nxt=strpbrk(nxt+1," |"); // find the separator
        if(!nxt) // if not more ... put it to thee end of the id line
            nxt=seqid+sizeSeqId;/// nxt=seqid+id1Id[1];
        if(*nxt==' ')
            break;
        seqLbls.addSeparator(",");
        seqLbl = seqLbls.last();
        seqLbls.printf("seq%" DEC,++i);
        qry2.printf("%s=find.annot(id='%s', type='%s');unique.1(%s.record);",seqLbl, curId,curType,seqLbl);
    }
    qry2.printf("seq=foreach(%s);a=find.annot(#range=possort-max,seq.1,%s,seq.1,%s);", seqLbls.ptr(1), start, end);
    createAnnotationIonRangeQuery(qry2,"a.seqID","a.record");
    return true;
}

//static
idx sBioseqSNP::launchIonAnnot(sIonWander * iWander, sIonWander * iWanderComplex, sVec < ionRange > * rangeVec, idx position, const char * sequenceIdFromProfiler){

    char szStart[128],szEnd[128];
    szEnd[0]='0'; szEnd[1]=':';
    idx lenStart,lenEnd;

    iWander->callbackFunc =  sBioseqSNP::traverserCallback;
    iWander->callbackFuncParam = rangeVec;

    sIPrintf(szStart,lenStart,position,10);
    memcpy(szStart+lenStart,":0",3);
    lenStart+=2;

    sIPrintf(szEnd+2,lenEnd,(position+1),10);
    lenEnd+=2;

    // case 1: use the whole "sequence identifier from profiler" as the sequence id
    iWander->setSearchTemplateVariable("$start",6,szStart,lenStart); // format should be => posStart:0
    iWander->setSearchTemplateVariable("$end",4,szEnd,lenEnd);       //                  => 0:posEnd
    iWander->resetResultBuf();
    iWander->traverse();

    // when case 1 failed, the "sequence identifier from profiler" might be in the NCBI format
    // => split by "|" like: gi|367460291|gb|CP003231.1|
    if (!rangeVec || !rangeVec->dim()){
        iWanderComplex->callbackFunc =  sBioseqSNP::traverserCallback;
        iWanderComplex->callbackFuncParam = rangeVec;

        const char * nxt, *seqid=sequenceIdFromProfiler;


        iWanderComplex->setSearchTemplateVariable("$start",6,szStart,lenStart); // format should be => posStart:0
        iWanderComplex->setSearchTemplateVariable("$end",4,szEnd,lenEnd);       //                  => 0:posEnd

        idx sizeSeqId =0;
        for( const char * p=seqid; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){ // scan until pipe | separated types and ids are there
            const char * curType=p;
            nxt=strpbrk(p,"| "); // find the separator
            if(!nxt || *nxt==' ')
                break;

            const char * curId=nxt+1;
            nxt=strpbrk(nxt+1," |"); // find the separator
            if(!nxt) // if not more ... put it to thee end of the id line
                nxt=seqid+sizeSeqId;/// nxt=seqid+id1Id[1];
            if(*nxt==' ')
                break;

            iWanderComplex->setSearchTemplateVariable("$id", 3, curId, nxt-curId);
            iWanderComplex->setSearchTemplateVariable("$type", 5, curType, curId-1-p);
            iWanderComplex->resetResultBuf();
            iWanderComplex->traverse();

            if (rangeVec && rangeVec->dim()){
                break;
            }
        }

    }

    return rangeVec->dim();
}


//static
idx * sBioseqSNP::tryAlternativeWay (sIonWander * myWander, const char * orignal_id, idx * recDim) {

    const char * nxt;
    nxt = orignal_id+sLen(orignal_id);
    idx sizeSeqId=nxt-orignal_id;
    for( const char * p=orignal_id; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){ // scan until pipe | separated types and ids are there
               nxt=strpbrk(p,"| "); // find the separator
               if(!nxt || *nxt==' ')
                   break;

               const char * curId=nxt+1;
               nxt=strpbrk(nxt+1," "); // find the separator
               if(!nxt) // if not more ... put it to thee end of the id line
                   nxt=orignal_id+sizeSeqId;/// nxt=seqid+id1Id[1];
//               if(*nxt==' ')
//                   break;

               myWander->setSearchTemplateVariable("$seqID1",7,curId, nxt-curId);
               myWander->setSearchTemplateVariable("$seqID2",7,curId, nxt-curId);
               myWander->resetResultBuf();
               myWander->traverse();

               if (myWander->traverseBuf.length()){
                   if(recDim)*recDim=myWander->traverseBuf.length()/sizeof(idx);
                   return (idx * )myWander->traverseBuf.ptr(0);
               }
               const char * dot = strpbrk(curId,".");
               if (dot){
                   myWander->setSearchTemplateVariable("$seqID1",7, curId, dot-curId);
                   myWander->setSearchTemplateVariable("$seqID2",7, curId, dot-curId);

                   myWander->resetResultBuf();
                   myWander->traverse();
                   if (myWander->traverseBuf.length()){
                       return (idx * )myWander->traverseBuf.ptr(0);
                  }
              }
           }

    return 0 ;
}

idx sBioseqSNP::snpCalls(sFil * prof, const char * subseq, const char * subName,idx sub_start, idx sub_end,idx start,idx sublen, idx cnt, sStr * snpOut,ProfileSNPcallParams * SPC, idx resolution, idx rsID)
{
//    idx modeRequested=SPC->consensusAAMode;
    SPC->consensusAAMode=0;

    if (resolution && resolution >0)SPC->minmax=true;
    if(SPC->minmax){
        SPC->minRngFreq=REAL_MAX;
        SPC->maxRngFreq=-REAL_MAX;
    }
    sStr minOut,maxOut;
    idx iVis=0,outP=1;
    if(cnt<=0)cnt=sIdxMax;

    char consLet3[4],refLet3[4]; consLet3[3]=0,refLet3[3]=0;
    sStr cleanSubName;
    //TODO: @Lam, Do we need this function? we don't seem to use the cleanSubName
    sVioAnnot::cleanIdFromProfiler(subName,cleanSubName);

    /*sVec < sVioAnnot::AnnotStruct > results;*/
    sStr tmpOut, baseMut, aaMut;
    real vv=0;
    idx rangeS=0,rangeE=0;
    char refLchar,mutLchar;
    idx intervall; intervall = resolution && resolution>0 ? (sub_end-sub_start)/resolution > 0 ? (sub_end-sub_start)/resolution : 1 : 0;

    SNPRecord Line, CLine,* line, * codonLine;line=&Line,codonLine=&CLine;idx lastValid=-1,t_lastValid=-1;

    const char * SNPline=prof->ptr(),* endSNP=prof->ptr()+prof->length();
    const char * t_SNPline;
    SNPline = sString::skipWords(SNPline,0,1,sString_symbolsEndline);
    if(!SNPline)return 0;
    if(SPC->iSub){
        //SNPline = binarySearchReference(prof,SNPline,goToPreviousLine(prof->ptr()+prof->length()-1,SNPline),SPC->iSub);
        SNPline = binarySearchReference(SNPline,prof->ptr()+prof->length(),SPC->iSub);
        if(!SNPline)return 0;
    }

    idx cntMuts=0, t_ir=0;

    idx elCnt = 0;
    idx cumulLetForCodon=0, howManyGoodCoverageForThisCodon=0;
    real consFreq=0.;

    sVec < ionRange > ionRangeVec;
    for(idx ir=sub_start; ir<sub_end; ir+=1) {

        if((ir%3)==0) {
            cntMuts=0;
            cumulLetForCodon=0;
            consFreq=0;
            howManyGoodCoverageForThisCodon=0;
        }


        t_ir=ir;
        t_SNPline=SNPline;
        if( SPC->codonScale && SPC->isORF && (ir%3==0 || ir==sub_start ) ){
            for( ; ir < t_ir+3 && ir<sub_end ; ++ir){
                if(ir-t_lastValid==1){
                    if(!SPC->iSub) t_SNPline=sBioseqSNP::SNPRecordNext(t_SNPline, codonLine,endSNP);
                    else t_SNPline=sBioseqSNP::SNPConcatenatedRecordNext(t_SNPline, codonLine,endSNP,SPC->iSub);
                    t_lastValid=codonLine->position-1;
                }
                if(ir!=t_lastValid)continue;

                char refLet = codonLine->letter;
                idx tot=codonLine->coverage();
                for (idx ic=0; ic<4 ; ++ic ) {
                    if( ic==sBioseq::mapATGC[(idx)refLet] ) continue;
                    idx v1=codonLine->atgc[ic];
                    vv=((real)v1)/tot;
                    if( vv && vv>=SPC->snpCallCutoff ){
                        cntMuts++;
                    }
                }
            }
        }
        ir=t_ir;

        if(ir-lastValid==1){
            if(!SPC->iSub) SNPline=sBioseqSNP::SNPRecordNext(SNPline, line,endSNP);
            else SNPline=sBioseqSNP::SNPConcatenatedRecordNext(SNPline, line,endSNP,SPC->iSub);
            lastValid=line->position-1;
        }

        //cumulLetForCodon<<=2;
        //cumulLetForCodon|=sBioseq::mapATGC[line->consensus];
        cumulLetForCodon |= sBioseq::mapATGC[(idx)(line->consensus)] << (2*((ir%3)));
        consLet3[ir%3]=line->consensus;
        refLet3[ir%3]=line->letter;

        if(ir<sub_start)continue;

        if (intervall > 0){
            if (elCnt == intervall){
                if (minOut.length()>0 && maxOut.length()>0){
                    sStr minSepar; sStr maxSepar; idx minPosi; idx maxPosi;
                    sString::searchAndReplaceSymbols(&minSepar,minOut,0,",",0,0,true,true,true,true);
                    sString::searchAndReplaceSymbols(&maxSepar,maxOut,0,",",0,0,true,true,true,true);
                    minPosi = atol(sString::next00(minSepar,1));
                    maxPosi = atol(sString::next00(maxSepar,1));
                    if (minPosi<maxPosi)
                        snpOut->printf("%s%s",minOut.ptr(),maxOut.ptr());
                    if (minPosi>maxPosi)
                        snpOut->printf("%s%s",maxOut.ptr(),minOut.ptr());
                    if (strcmp(minOut.ptr(0),maxOut.ptr(0))==0)
                        snpOut->printf("%s",minOut.ptr());
                }
                else {
                    if (!SPC->isORF)
                        snpOut->printf("\"%s\",%" DEC ",NA,NA,0,0,0,0\n",SPC->hideSubjectName?"-":subName,ir+1);
                    if (SPC->isORF)
                        snpOut->printf("\"%s\",%" DEC ",NA,NA,0,0,0,0,0,0,0,NA,NA\n",SPC->hideSubjectName?"-":subName,ir+1);
                }

                SPC->minRngFreq=REAL_MAX;
                SPC->maxRngFreq=-REAL_MAX;
                minOut.cut0cut();
                maxOut.cut0cut();
                elCnt =0;
            }
            elCnt +=1;
        }

        if(ir!=lastValid)continue;
        idx ic, v ,tot=0; // refLet=sBioseq::mapATGC[(idx)s[ir-0]]; // -substart
        char refLet = sBioseqAlignment::_seqBits(subseq, ir , 0);
        tot=line->coverage();
        if(!tot)continue;
        else ++howManyGoodCoverageForThisCodon;

        idx relativeLetter= (idx)refLet; // (SP->snpCompare==0 ) ? consLet : refLet ;

        idx statusPositionAnot=1,rangeCnt=0;//,rangeSt=1,rangeCnt=0,rangeEnd=sublen;
        //results.cut(0);
        sStr rsIdFound("-");
       /* if(SPC->isORF && SPC->annotList) {
            statusPositionAnot=sVioAnnot::GetAnnotRangeList(cleanSubName.ptr(), 0, ir, &results,SPC->annotList);
        }*/
        // ion wander
        if(SPC->isORF && SPC->iWander) {
            ionRangeVec.empty();
            statusPositionAnot = sBioseqSNP::launchIonAnnot(SPC->iWander, SPC->iWanderComplex, &ionRangeVec, ir, subName);
            if (!statusPositionAnot) statusPositionAnot=1;
        }
        //
        refLchar=(char)sBioseq::mapRevATGC[(idx)refLet];


        if( SPC->consensusAAMode ){
            for ( ic=0; ic<4 ; ++ic ) {
                if( relativeLetter==ic)continue;

                v=line->atgc[ic];//+line->atgcRev[ic];
                vv=((real)v)/tot;
                if(line->consensus==(char)sBioseq::mapRevATGC[ic])
                    consFreq=sMax(consFreq,vv);
            }
        }

        for ( ic=0; ic<6 ; ++ic ) {
            if( relativeLetter==ic)continue;
            if( ic<4 ) {
                v=line->atgc[ic];//+line->atgcRev[ic];
                vv=((real)v)/tot;
                mutLchar=(char)sBioseq::mapRevATGC[ic];
            } else {
                v = line->indel[ic - 4];
                vv = ((real) v) / tot;
                if(ic==4) {
                    mutLchar = '+'; //insert
                } else {
                    mutLchar = '-'; //deletion
                }
            }

            if(cntMuts)vv=vv/cntMuts;

            bool amIPrinting=false;
            if(!SPC->consensusAAMode && ic!=refLet && vv && vv>=SPC->snpCallCutoff){
                amIPrinting=true;
            }if( SPC->consensusAAMode ){
                amIPrinting=true;
            }


            if(amIPrinting) {
                if( ic < 4 ) {
                    baseMut.printf(0,"%c",mutLchar);
                } else if (ic ==4) {
                    baseMut.printf(0,"%c%c",mutLchar,refLchar);
                } else {
                    baseMut.printf(0,"%c",mutLchar);
                }

                if(!SPC->isORF){
                    tmpOut.printf(0,"\"%s\",%" DEC ",%c,%s,%.4lf,%" DEC ",%.2lf,%" DEC "",SPC->hideSubjectName?"-":subName,ir+1,refLchar,baseMut.ptr(),vv,tot,line->entrScaled,sublen);

                    if (rsID && SPC->annotList){
                       sVec < sVioAnnot::AnnotStruct > resultsForRsID;
                       idx rangeCntForRsID = resultsForRsID.dim();
                       if(!rangeCntForRsID)rangeCntForRsID=1;

                       for(idx iL=0;iL<rangeCntForRsID;++iL){
                           if( resultsForRsID.dim() ) {
                               rsIdFound.printf(0,"%s", resultsForRsID.ptr(iL)->rangeID.ptr());
                           }
                       }
                       tmpOut.printf(",%s", rsIdFound.ptr());
                    }
                    tmpOut.printf("\n");
                } else {
                    if(!statusPositionAnot)outP=0;
                    else {
                        tmpOut.cut(0);
                        rangeCnt=ionRangeVec.dim();
                        if(!rangeCnt) rangeCnt=1;
                        for(idx iL=0;iL<rangeCnt;++iL){
                            rangeS = rangeE = 0;
                            ionRange * annotRange = ionRangeVec.ptr(iL);
                            const char * translationSource;

                            if (annotRange){
                                rangeS = annotRange->start;
                                rangeE = annotRange->end;
                                translationSource="from annotation file";
                            }
                            else
                                translationSource="subject is interpreted as ORF";
                            if(SPC->consensusAAMode) {
                                tmpOut.printf("\"%s\",%" DEC ",%s,%s,%.4lf,%" DEC ",%.2lf,%" DEC "",SPC->hideSubjectName?"-":subName,ir+1,refLet3,consLet3,consFreq,tot,line->entrScaled,sublen);
                            } else {
                                tmpOut.printf("\"%s\",%" DEC ",%c,%s,%.4lf,%" DEC ",%.2lf,%" DEC "",SPC->hideSubjectName?"-":subName,ir+1,refLchar,baseMut.ptr(),vv,tot,line->entrScaled,sublen);
                            }

                            if (rsID){
                                tmpOut.printf(",%s", rsIdFound.ptr());
                            }
//
                            idx threeCodeOffset=0;
                            char refAA=0;
                            idx mutPosition = 0;
                            mutPosition = sBioseqSNP::aminoacidDecode(subseq,ir,annotRange,&refAA,&threeCodeOffset);

                            char mutAA = refAA & (~(0x3 << (2 * threeCodeOffset)));
                            if(ic<4) {
                                mutAA |= (ic << (2*threeCodeOffset));
                            }
                            if( SPC->consensusAAMode ){
                                if(howManyGoodCoverageForThisCodon==3 && strcmp(consLet3,refLet3)!=0 )
                                    mutAA=cumulLetForCodon;
                                else{
                                    outP = 0;
                                    break;
                                }
                            }
                            if( sBioseq::mapCodon(refAA) == sBioseq::mapCodon(mutAA) && ic<4 && SPC->nsSNVs )
                                outP = 0;
                            else {
                                outP = 1;
                                if (ionRangeVec.dim()){
                                    tmpOut.printf(",%s",ionRangeVec[0].proteinId.ptr(0));
                                } else tmpOut.printf(",-");

                                const char * aa1=sBioseq::codon2AA(refAA)->print(SPC->AAcode);
                                const char * aa2=(ic>=4)?"fs":sBioseq::codon2AA(mutAA)->print(SPC->AAcode);
                                tmpOut.printf(",%" DEC ",%" DEC ",%" DEC ",%s,%s",rangeS,rangeE, mutPosition+1, aa1, aa2);
                                tmpOut.printf(",%s",translationSource);
                                tmpOut.printf("\n");

                            }
                            if(SPC->consensusAAMode )
                                break;
                        }
                    }

                }

                if( outP ) {
                    if( SPC->minmax ) {
                        if( vv > SPC->maxRngFreq ) {
                            SPC->maxRngFreq = vv;
                            maxOut.printf(0, "%s", tmpOut.ptr());
                        }
                        if( vv < SPC->minRngFreq ) {
                            SPC->minRngFreq = vv;
                            minOut.printf(0, "%s", tmpOut.ptr());
                        }
                    } else {
                        if( tmpOut.ptr() && iVis >= start )
                            snpOut->printf("%s", tmpOut.ptr());
                        ++iVis;
                    }
                }
                if(SPC->consensusAAMode )
                    break;

            }
            if( iVis >= start + cnt )
                break;
        }
        if(SPC->outF){
            fwrite(snpOut->ptr(),snpOut->length(),1,SPC->outF);
            snpOut->cut(0);
        }
        if( iVis >= start + cnt )
            break;

    }
    if (SPC->minmax && minOut.length()>0 && maxOut.length()>0) {
        sStr minSepar; sStr maxSepar; idx minPosi; idx maxPosi;
        sString::searchAndReplaceSymbols(&minSepar,minOut,0,",",0,0,true,true,true,true);
        sString::searchAndReplaceSymbols(&maxSepar,maxOut,0,",",0,0,true,true,true,true);
        minPosi = atol(sString::next00(minSepar,1));
        maxPosi = atol(sString::next00(maxSepar,1));
        if (minPosi<maxPosi)
            snpOut->printf("%s%s",minOut.ptr(),maxOut.ptr());
        if (minPosi>maxPosi)
            snpOut->printf("%s%s",maxOut.ptr(),minOut.ptr());
        if (strcmp(minOut.ptr(0),maxOut.ptr(0))==0)
            snpOut->printf("%s",minOut.ptr());
    }
    if(SPC->outF){
        fwrite(snpOut->ptr(),snpOut->length(),1,SPC->outF);
        snpOut->cut(0);
    }
    return 1;
}

idx sBioseqSNP::iterateProfile(sFil * profile,idx subLen,idx * piVis, idx start, idx cnt, sBioseqSNP::typeCallbackIteratorFunction callbackFunc,ParamsProfileIterator * callbackParam)
{
    idx iFound=0,myVis=0,iprofStart=0,iposEnd=subLen,res=0, buflenBefore;

    if(!piVis)piVis=&myVis;
    if(!cnt)cnt=sIdxMax;

    bool primaryFilters=(callbackParam->isCoverageThrs)? true : false;
    bool secondaryFilters=(callbackParam->regp)? true : false;
    if(!secondaryFilters && cnt!=sIdxMax)++cnt;


    idx ia=0,lastValid=-1;//, profdiff=0;
    const char * SNPline=profile->ptr(),*endSNP=profile->ptr()+profile->length(), *tmp_SNPline;
    SNPline = sString::skipWords(SNPline,0,1,sString_symbolsEndline);
    if(!SNPline)return 0;
    if(callbackParam->iSub){
        //SNPline = binarySearchReference(profile,SNPline,goToPreviousLine(profile->ptr()+profile->length()-1,SNPline),callbackParam->iSub);
        SNPline = binarySearchReference(SNPline,profile->ptr()+profile->length(),callbackParam->iSub);
        if(!SNPline)return 0;
    }
    SNPRecord rec,trec;
    if(callbackParam->pageRevDir)ia-=start;

//    sSNPRecordIter profIter(profile);
//    SNPRecordNext(SNPline, &trec,endSNP);
//    lastValid=trec.position-2;      //-1 because it is one-based and -1 because we want it to point to the previous position

    for (; ia<iposEnd; ++ia) {
        if( ia-lastValid==1 ){
            if(callbackParam->iSub) tmp_SNPline=SNPConcatenatedRecordNext(SNPline, &rec,endSNP,callbackParam->iSub);
            else tmp_SNPline=SNPRecordNext(SNPline, &rec,endSNP);
            lastValid=rec.position-1;
            if(ia == lastValid) {
                SNPline = tmp_SNPline;
            }
            else {
                lastValid = rec.position - 2;
                sSet(&rec,0);rec.position=ia+1;
            }
        }
        else {sSet(&rec,0);rec.position=ia+1;}
        if(ia<iprofStart+start)continue;

        // filters which do not need an output to be generated
        bool isok=true;
        buflenBefore=callbackParam->str ? callbackParam->str->length() : 0 ; // we might need this to roll back additions to the buffer

        if(primaryFilters){

            if(rec.coverage()<=callbackParam->coverageThrs)
                isok=false;
        }

        if( isok && secondaryFilters ) { // call the function only if it has passed preliminary filters and needs a secondary filter
            res=callbackFunc(&rec,callbackParam,ia-iprofStart);
        }

        // secondary filters which needed an output content to filter with
        if(secondaryFilters && isok) {
            //MORE FILTER TO BE ADDED
        }

        // now consider if we need to really output this or not
        if( isok ) {
            if(piVis){
                ++(*piVis);
                if(*piVis<= (callbackParam->pageRevDir?start:0)) {
                    isok=false; // after this point isok is only used as a marker for cutting the str back
                }
                if(*piVis>=(callbackParam->pageRevDir?start +cnt:cnt)) {
                    //iFound=sNotIdx;
                    break;//return sNotIdx;
                }
            }
        }

        if( isok && !secondaryFilters ) // this function has not yet been called if secondary filters were not defined
            res=callbackFunc(&rec,callbackParam, ia-iprofStart);
        else { // cut out the last additions, those have not passed the filter
            if(callbackParam->str && !isok)
                callbackParam->str->cut(buflenBefore);
        }
        if(isok && !res && piVis)--(*piVis);
        if(res){
            ++iFound;
            if(callbackParam->outF){
                fwrite(callbackParam->str->ptr(),callbackParam->str->length(),1,callbackParam->outF);
                callbackParam->str->cut(0);
            }
        }

        if(res==sNotIdx)
            break;

    }

    return iFound;
}

idx sBioseqSNP::snpDetectGaps(ProfileStat * ps, sVec < ProfileGap > * spg, sFil * prof, idx sublen, idx gapWindowSize, idx gapThreshold,idx minGapLength,idx iSub )
{
    sSet(ps,0,sizeof(*ps));
    ps->reflen=sublen;
    ps->averageContigCoverage=0;// it was =sublen; why? it has to be zero.
    ps->totalContigsNumber=1;
    ps->contigsPart=100.0;

    sVec < idx > Window;idx * window=Window.resize(gapWindowSize);Window.set(0);
    idx windowAll=0;

    //move the window and store coverage when gap or contig ends

    idx prvState=-1, curState=0;//gap;
    idx stateChangePos=0, icel, cumulWin=0;
    ProfileGap * wpg, *wpprev=0 ;
    spg->mex()->flags|=sMex::fSetZero;

    SNPRecord Line,* line;line=&Line;idx lastValid=-1;
    const char * SNPline=prof->ptr(),* endSNP=prof->ptr()+prof->length(), * tmp_SNPline = 0;
    SNPline = sString::skipWords(SNPline,0,1,sString_symbolsEndline);
    if(!SNPline)return 0;
    if(iSub){
        //SNPline = binarySearchReference(prof,SNPline,goToPreviousLine(prof->ptr()+prof->length()-1,SNPline),iSub);
        SNPline = binarySearchReference(SNPline,prof->ptr()+prof->length(),iSub);
        if(!SNPline)return 0;
    }
    SNPline-=1;

    for (idx i=0; i<sublen; ++i) {
        if(i-lastValid==1){
            if(iSub) tmp_SNPline=sBioseqSNP::SNPConcatenatedRecordNext(SNPline, line,endSNP,iSub);
            else tmp_SNPline=sBioseqSNP::SNPRecordNext(SNPline, line,endSNP);
            lastValid=line->position-1; //from one to zero based
            if(i == lastValid) {
                SNPline = tmp_SNPline;
            }
            else {
                lastValid = line->position - 2;
                sSet(line,0);line->position=i+1;
            }
        }
        else sSet(line,0);
//        if(i!=lastValid)continue;

        icel=i%gapWindowSize;

        if(i>=gapWindowSize)
            windowAll-=window[icel];
        window[icel]=line->coverage();
        windowAll+=window[icel];
        cumulWin+=window[icel];

        curState = (windowAll<=gapWindowSize*gapThreshold) ? 0 : 1;

        if(!i || i>=gapWindowSize) {
            if(curState!=prvState){
                wpg = spg->add(1);
                stateChangePos=i;
                if(spg->dim()>1)wpprev=spg->ptr(spg->dim()-2); // wpprev is in the dynamically allocated buffer , we have
                if(wpprev) {
                    wpprev->end=stateChangePos-1;
                    wpprev->averageCoverage = cumulWin - window[icel];
                }
                wpg->start = stateChangePos;

                cumulWin = window[icel];
                wpg->hasCoverage=curState;
                wpprev=wpg;
            }
        }
        prvState=curState;
    }
    if(wpprev) {
        wpprev->end=sublen-1;
        wpprev->averageCoverage = cumulWin;
    }
    if(spg->dim())
        ps->totalContigsNumber=0;
    //ProfileGap * wpgDst=spg->ptr(0);
    for(idx i=0 ;i<spg->dim(); ++i) {
        wpg=spg->ptr(i);

        wpg->length=(wpg->end-wpg->start)+1;
        wpg->hasCoverage = ( wpg->averageCoverage > (sMax(wpg->length,gapWindowSize)*gapThreshold ) ) ;

        if(wpg->length<minGapLength){
            continue;
        }

        if( !wpg->hasCoverage ) {
            //if(wpg->start>=gapWindowSize)wpg->start-=gapWindowSize-1;
            ps->averageGapCoverage+=wpg->averageCoverage;
        }else {
            //if (i!=spg->dim()-1) wpg->end-=gapWindowSize-1;
            ps->averageContigCoverage+=wpg->averageCoverage;
        }


        wpg->averageCoverage/=wpg->length;

        if( !wpg->hasCoverage ) {
            ps->totalGapLength+=wpg->length;
            ++ps->totalGapsNumber;
        }else {
            ps->totalContigLength+=wpg->length;
             ++ps->totalContigsNumber;
        }

    }

    for(idx i=0 ;i<spg->dim(); ++i) {
        wpg=spg->ptr(i);
        if ((wpg->hasCoverage)&&(wpg->averageCoverage<1)) wpg->averageCoverage=1;
    }


    if(ps->totalGapsNumber && ps->totalGapLength)ps->averageGapCoverage/=ps->totalGapLength;
    if(ps->totalContigsNumber && ps->totalContigLength)ps->averageContigCoverage/=ps->totalContigLength;
    if (sublen) ps->contigsPart=100.0*ps->totalContigLength/sublen;
    ps->gapsPart=100.0-ps->contigsPart;


    return(0);
}

idx sBioseqSNP::snpOutConsensus(SNPRecord  * rec,ParamsProfileIterator * params,idx iNum){
    idx tot = rec->coverage() ;
    bool pass = true;
    idx consInd = 0;
    if(rec->consensus!='-') {
        consInd = (idx)sBioseq::mapATGC[(idx)rec->consensus];
    }
    else consInd = 4;
    if( !tot  || (params->consensusThrs && rec->freqRaw(consInd) < params->consensusThrs) ) pass=false;
    if( pass ){
        if( params->iter_flags==sBioseqSNP::ePIsplitOnGaps && !params->userIndex ) {
            params->str->printf("%s start=%" DEC "\n",(const char *)params->userPointer, rec->position);
        }
        params->str->printf("%c",rec->consensus);
        ++params->userIndex;
    }
    else {
        switch (params->iter_flags) {
            case sBioseqSNP::ePIskipGaps:
                return 0;
//                break;
            case sBioseqSNP::ePIreplaceGaps:
                ++params->userIndex;
                params->str->printf("%c",(char)sBioseq::mapRevATGC[(idx)sBioseqAlignment::_seqBits((const char *)params->userPointer, rec->position-1, 0)]);
                break;
            case sBioseqSNP::ePIsplitOnGaps:
                if( params->userIndex%params->wrap ) {
                    params->str->printf("\n");
                }
                params->userIndex=0;
                return 1;
            default:
                ++params->userIndex;
                params->str->printf("+");
                break;
        }
    }

    if(params->wrap && params->userIndex && ((params->userIndex)%(params->wrap)==0))
        params->str->printf("\n");

    return 1;
}


idx sBioseqSNP::noiseProfileFromCSV(const char *buf, const char *bufend, sDic< sVec<idx> > * noiseProfile, real noiseProfileResolution)
{
    idx iSub = 0;
    if (!buf)
        return iSub;
    if (!bufend)
        bufend = buf + strlen(buf);
    idx noise[6][6], ibin;
    real fr;
    while(buf<bufend) {
        const char * bufNext = skipToNextLine(buf, bufend);
        noiseProfileFreqFromConcatenatedCSV(buf,bufend,bufNext,&iSub,&fr,noise);

        ibin=(idx)((fr+0.1*noiseProfileResolution)/noiseProfileResolution);
        for ( idx ins=0; ins<4 ; ++ins) {
            for ( idx ine=0; ine<4 ; ++ine) {
                if(ins==ine)
                    continue;
                char noiseType[32];sprintf(noiseType,"%c-%c",(char)sBioseq::mapRevATGC[ins],(char)sBioseq::mapRevATGC[ine]);
                sVec<idx> * noiseVec=noiseProfile->set(noiseType);
                noiseVec->flagOn(sMex::fSetZero);
                noiseVec->resize(ibin+1);
                (*noiseVec)[ibin]+=noise[ins][ine];
            }
        }
        buf=bufNext;
    }
    return iSub;
}


idx sBioseqSNP::integralFromProfileCSV(const char *buf, const char *bufend, sDic<sVec<real> > & integrals, real noiseProfileResolution, const real * Ctof /*= noiseCutoffThresholds */, idx Ctof_size /*= noiseCutoffThresholds_NUM*/)
{
    sDic< sVec<idx> > noiseProfile;
    if(!noiseProfileFromCSV(buf,bufend,&noiseProfile,noiseProfileResolution)) return 0;
    sVec<idx> noiseSum;
    if(!snpSumNoise( noiseProfile, noiseSum, noiseProfileResolution )) return 0;

    return snpComputeIntegrals( noiseProfile, noiseSum, integrals, noiseProfileResolution, Ctof, Ctof_size );
}

const char *  sBioseqSNP::noiseProfileFreqFromConcatenatedCSV(const char * buf, const char * bufend, const char * bufNext, idx * iSub, real * fr, idx noise[6][6]) {
    if(!buf || !iSub || !fr) return 0;

    if( !bufNext ) {
        bufNext = skipToNextLine(buf, bufend);
    }

    memset(noise, 0, sizeof(*noise));
    if( ! (sString::bufscanf(buf, bufNext, "%" DEC ",%lf,%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC, iSub, fr,
            &(noise[0][1]), &(noise[0][2]), &(noise[0][3]),
            &(noise[1][0]), &(noise[1][2]), &(noise[1][3]),
            &(noise[2][0]), &(noise[2][1]), &(noise[2][3]),
            &(noise[3][0]), &(noise[3][1]), &(noise[3][2])) == 14) )
        return 0;
    return bufNext;
}

const char * sBioseqSNP::noiseProfileSubFromConcatenatedCSV(const char * buf, const char * bufend, const char * bufNext, idx * iSub) {
    if(!buf || !iSub) return 0;

    if( !bufNext ) {
        bufNext = skipToNextLine(buf, bufend);
    }
    if( sString::bufscanf(buf, bufNext, "%" DEC ",%*s", iSub) != 1 )
        return 0;
    return bufNext;
}

idx sBioseqSNP::histogramProfileFromCSV(const char *buf, const char *bufend, sVec<HistogHistog> * histProfile, real noiseProfileResolution)
{
    if (!buf)
        return -1;
    if (!bufend)
        bufend = buf + strlen(buf);

    idx iSub, v, cnt, ibin;
    char letter;
    char histC[1024];
    char * hist, * histNext, * histEnd;
    real fr;
    while(buf < bufend) {
        sSet(histC);
        const char * bufNext = skipToNextLine(buf, bufend);
        sString::bufscanf(buf, bufNext, "%" DEC ",%lf,%c,\"%s\"", &iSub, &fr, &letter, histC);

        ibin=(idx)(fr/noiseProfileResolution);
        histProfile->resize(ibin+1);
        sBioseqSNP::HistogHistog * hh=histProfile->ptr(ibin);
        hh->countForCoverage[0].flagOn(sMex::fSetZero);hh->countForCoverage[1].flagOn(sMex::fSetZero);hh->countForCoverage[2].flagOn(sMex::fSetZero);hh->countForCoverage[3].flagOn(sMex::fSetZero);

        hist = &histC[0];
        histEnd = hist + sLen(hist);
        while(hist && hist < histEnd) {
            histNext = sString::skipWords(hist,0,2,",");
            sString::bufscanf(hist, histNext, "%" DEC ",%" DEC, &v, &cnt);
            char histType[32]; sprintf(histType, "%" DEC,v);
            hh->countForCoverage[sBioseq::mapATGC[(idx)letter]][histType]+=cnt;
            hist = histNext;
        }
        buf=bufNext;
    }
    return iSub;
}

bool sBioseqSNP::snpNoiseCutoffsFromIntegralCSV(const char *buf, const char *bufend, real noiseCutoffs[6][6], idx thresholdIdx)
{
    if (!buf)
        return false;
    if (!bufend)
        bufend = buf + strlen(buf);
    for (idx i=0; i<thresholdIdx+1; i++) {
        buf = skipToNextLine(buf, bufend);
        if (buf >= bufend || !*buf)
            return false;
    }
    memset(noiseCutoffs, 0, sizeof(*noiseCutoffs));
    return (sString::bufscanf(buf, bufend, "%*f,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
        &(noiseCutoffs[0][1]), &(noiseCutoffs[0][2]), &(noiseCutoffs[0][3]),
        &(noiseCutoffs[1][0]), &(noiseCutoffs[1][2]), &(noiseCutoffs[1][3]),
        &(noiseCutoffs[2][0]), &(noiseCutoffs[2][1]), &(noiseCutoffs[2][3]),
        &(noiseCutoffs[3][0]), &(noiseCutoffs[3][1]), &(noiseCutoffs[3][2])) == 12);
}

const char * sBioseqSNP::snpConcatenatedNoiseCutoffsFromIntegralCSV(const char *buf, const char *bufend, real noiseCutoffs[6][6], idx iSub, idx thresholdIdx, idx * piSub)
{
    if (!buf)
        return 0;
    if (!bufend)
        bufend = buf + strlen(buf);
    idx cur_sub = 0;
    // skip header
    if( buf < bufend && !isdigit(buf[0]) ) {
        buf = skipToNextLine(buf, bufend);
    }
    if( iSub ) {
        while( buf < bufend && buf[0] ) {
            cur_sub = atoidx(buf);
            if( cur_sub >= iSub ) {
                break;
            }
            buf = skipToNextLine(buf, bufend);
        }
    }

    if( buf >= bufend || !buf[0] || (iSub && cur_sub != iSub) ) {
        return 0;
    }

    cur_sub = atoidx(buf);

    for(idx i=0; i<thresholdIdx; i++) {
        buf = skipToNextLine(buf, bufend);
        if( buf >= bufend || !buf[0] ) {
            return 0;
        }
    }

    if( atoidx(buf) != cur_sub ) {
        return 0;
    }

    idx num_read = sString::bufscanf(buf, bufend, "%*d,%*f,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &(noiseCutoffs[0][1]), &(noiseCutoffs[0][2]), &(noiseCutoffs[0][3]),
            &(noiseCutoffs[1][0]), &(noiseCutoffs[1][2]), &(noiseCutoffs[1][3]),
            &(noiseCutoffs[2][0]), &(noiseCutoffs[2][1]), &(noiseCutoffs[2][3]),
            &(noiseCutoffs[3][0]), &(noiseCutoffs[3][1]), &(noiseCutoffs[3][2]));

    if( piSub ) {
        *piSub = cur_sub;
    }

    if( num_read == 12 ) {
        return skipToNextLine(buf, bufend);
    } else {
        return 0;
    }
}
