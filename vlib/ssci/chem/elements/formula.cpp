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
#include <ssci/chem/elements/elements.hpp>
#include <ctype.h>

using namespace slib;


idx sChem::formula::parse(const char * frml)
{
    char symbs[24];
    idx icnt,id,ielem,iTot=0;

    for(idx i=0; frml[i];){
        if(frml[i]>='A' && frml[i]<='Z') { 
            id=0;icnt=0;
            for(symbs[id++]=frml[i++]; frml[i] && (frml[i]<'A' || frml[i]>'Z'); ++i) {
                if(isalpha(frml[i]))symbs[id++]=frml[i];
                else if(isdigit(frml[i]))icnt=icnt*10+frml[i]-'0';
            }
            symbs[id]=0;
            ielem=element::index(symbs);
            if(!icnt)icnt=1;
            atomcount * ac=add(1);
            ac->element=ielem;
            ac->count=icnt;
            iTot+=icnt;
        }
        else break;
    }
    return iTot;
}


void sChem::formula::printfAtomList(sStr * dst, bool isotopic)
{
    sChem::formula * frml=this;
    sChem::atomcount * al=frml->ptr(0);
    for( idx imol=0,ia=0; ia<frml->dim(); ++ia, ++imol){

        for( ; ia<frml->dim() && al[ia].element!=0; ++ia) {
            element * ee=element::ptr((al[ia].element)&0xFF);
                        
            if(isotopic)dst->printf("%s[%" DEC "]%" DEC "", ee->symbol, (al[ia].element>>16) , al[ia].count); 
            else dst->printf("%s%" DEC "", ee->symbol,al[ia].count); 
        }
    }
}


idx sChem::formula::countAtoms(void) 
{
    formula * frml=this;
    
    idx totCnt=0;
    for( idx i=0; i<frml->dim(); ++i) totCnt+=(*frml)[i].count;
    return totCnt;
}


real sChem::formula::mass(idx type, bool integralonly) 
{
    real totMass=0;
    for( idx i=0; i<dim(); ++i) {
        element * el=element::ptr( (ptr(i)->element)&0xFF);
        isotop * iso=element::isoTable(el->ofsIsotops);
        idx pick=0;
        real abundance;

        switch (type) {
            case eIsotopicExact: 
                for(idx j=1; j<el->cntIsotops; ++j) if(((ptr(i)->element)>>16)==iso[j].iMass)pick=j;
                break;
            case eIsotopicAbundant:
                abundance=-REAL_MAX;
                for(idx j=1; j<el->cntIsotops; ++j) if(abundance>iso[j].abundance)pick=j;
                break;
            case eIsotopicRare:
                abundance=REAL_MAX;
                for(idx j=1; j<el->cntIsotops; ++j) if(abundance<iso[j].abundance)pick=j;
                break; 
            case eIsotopicLightest:
                break;
            case eIsotopicHeaviest:
                pick=el->cntIsotops-1;
                break; 
            default :
                pick=type>el->cntIsotops-1 ? el->cntIsotops-1 : type;
        }
        real mass=integralonly ? iso[pick].iMass : iso[pick].rMass;

        totMass+=mass*ptr(i)->count;
    }
    return totMass;
}


















idx sChem::formula::isotopicDistr(sVec < sChem::formula > * dst, real cutoff, real finalcutoff) 
{    
    sChem::formula & frml=*this;

    idx kinds=frml.countAtoms();
    sVec < idx > elems; idx * el=elems.add(kinds);
    idx ik,ie,id;
    for(ik=0, id=0; ik<frml.dim(); ++ik) {for(ie=0; ie<frml[ik].count; ++ie)el[id++]=frml[ik].element;}
    
    sVec < sChem::formula > & peaks = *dst;
    
    idx genFirst=0, genLast=1, ig;

    sChem::formula * pc=peaks.add();
    pc->prob=1;

    for(ie=0; ie<elems.dim(); ++ie) {
        element * ee=element::ptr(el[ie]);
        isotop * iso=element::isoTable.ptr(ee->ofsIsotops);

        idx genPos=peaks.dim();
        #ifdef ALGO_PRINTFS
            printf("---------------- Adding element #%" DEC " %s\n", ie, ee->symbol);
        #endif
        for(idx ip=genFirst; ip<genLast ; ++ip )  {
            
            
            for(idx ii=0; ii<ee->cntIsotops; ++ii) {
                idx curiso=element::makeIsotopicElement(el[ie],iso[ii].iMass);
                
                
                sChem::formula  * newPeak=peaks.add();
                sChem::formula  * srcpeak=peaks(ip);

                atomcount * atmdst, * atmsrc;
                bool found=false;
                for ( idx infrml =0 ; infrml< srcpeak->dim(); ++infrml) {
                    atmdst=newPeak->add();
                    atmsrc=srcpeak->ptr(infrml); 
                    *atmdst=*atmsrc;
                    if(atmsrc->element==curiso){++atmdst->count;found=true;}
                }
                if(!found){atmdst=newPeak->add();atmdst->element=curiso;atmdst->count=1;}
                

                newPeak->prob=srcpeak->prob*iso[ii].abundance;


                
                found=false;
                atmdst=newPeak->ptr();
                for ( ig=genPos; ig<peaks.dim()-1; ++ig) {
                    idx icnt=peaks(ig)->dim();
                    if(icnt!=newPeak->dim())continue;
                    
                    atmsrc=peaks(ig)->ptr();
                    idx diff=icnt;
                    for( idx i1=0; i1<icnt && diff; ++i1 ){
                        for( idx i2=0; i2<icnt && diff; ++i2 ){
                            if( atmsrc[i1].element==atmdst[i2].element && atmsrc[i1].count==atmdst[i2].count ){--diff; continue;}
                        }
                    }
                    if(!diff){found=true;break;}
                }


                if(found ) {
                    peaks(ig)->prob+=newPeak->prob;
                    peaks.cut(peaks.dim()-1);
                    newPeak=peaks(ig);
                }
                
            }
        }

        if(ie==elems.dim()-1) 
            cutoff=finalcutoff;

        sChem::formula * pk=peaks.ptr();
        real totSum=0, totSumFiltr=0;
        idx il;
        peaks.del(0,genPos);
        genPos=0;
        for(il=genPos; il < peaks.dim() ; ++il ) {
            totSum+=pk[il].prob;
            #ifdef ALGO_PRINTFS
                printf("#  %" DEC "\t",il);
                sChemAtomCount * oo=peaks(il)->ptr();
                for( idx kk=0; kk< peaks(il)->dim() ; ++kk){
                    sChemElement * eell=sChemPeriodicTable::element((oo[kk].element)&0xFF);
                    printf("%s[%" DEC "]%" DEC "", eell->symbol, (oo[kk].element>>16) , oo[kk].count); 
                }
                printf(" %lg", peaks(il)->prob); 
            #endif
            if(cutoff!=0 && pk[il].prob<cutoff) { 
                #ifdef ALGO_PRINTFS
                    printf("       cutoff filtering\n");
                #endif
                peaks.del(il,1);
                --il;
                continue;
            }
            #ifdef ALGO_PRINTFS
                printf("\n"); 
            #endif
            
            totSumFiltr+=pk[il].prob;
        }
        #ifdef ALGO_PRINTFS
            printf("##### Probs = %lg %lg\n", totSum, totSumFiltr);
        #endif

        for(il=0; il < peaks.dim() ; ++il ) {
            pk[il].prob/=totSumFiltr;
        }

        genFirst=0;
        genLast=peaks.dim();

    }

    return dst->dim();
}






void sChem::formula::printfIsotopicDistribution(sStr * dst,sVec < sChem::formula > * peaks, idx start, idx finish)
{

    if(finish==-1)finish=peaks->dim();
    real totSum=0;
    for(idx il=start; il < finish ; ++il ) {
        totSum+=peaks->ptr(il)->prob;
        dst->printf("#  %" DEC "\t",il);
        sChem::atomcount * oo=peaks->ptr(il)->ptr();
        for( idx kk=0; kk< peaks->ptr(il)->dim() ; ++kk){
            element * eell=element::ptr((oo[kk].element)&0xFF);
            dst->printf("%s[%" DEC "]%" DEC "", eell->symbol, (oo[kk].element>>16) , oo[kk].count); 
        }
        dst->printf(" %lg\n", peaks->ptr(il)->prob); 
    }
    dst->printf("##### %" DEC " elements %lg\n",peaks->dim(),totSum);

}

