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

//#define ALGO_PRINTFS

// parses a string into a formula "C6H12ONa24"
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


// printfs the formula either nomrally C14H24O12 or isotopic C[12]13C[13]1H[1]23H[2]1O12
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
        //dst->printf(" ");
    }
}


/// the number of atoms in the formula
idx sChem::formula::countAtoms(void) 
{
    formula * frml=this;
    
    idx totCnt=0;
    for( idx i=0; i<frml->dim(); ++i) totCnt+=(*frml)[i].count;
    return totCnt;
}


// computes a mass 
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
                // pick=0;
                break;
            case eIsotopicHeaviest:
                pick=el->cntIsotops-1;
                break; 
            default :
                pick=type>el->cntIsotops-1 ? el->cntIsotops-1 : type; // we pick up the isotop number specified 
        }
        real mass=integralonly ? iso[pick].iMass : iso[pick].rMass;

        totMass+=mass*ptr(i)->count;
    }
    return totMass;
}
















/*
#ifdef ALGORITHM_DESCRIPTION
5 0 0 0 0 // we start at first cell 
4 1 0 0 0 // move a single unit of the rightmost nonzero cell to the right 
4 0 1 0 0 // continue until we can 
4 0 0 1 0 
4 0 0 0 1 // if the last cell is non -zero this is the last we can continue this way
3 2 0 0 0 // now not counting the last one we start doing the same with previous rightmost cell , the rightmost cell adds up to the destination 
3 1 1 0 0 
3 1 0 1 0
3 1 0 0 1 
3 0 2 0 0
3 0 1 1 0
3 0 1 0 1 
3 0 0 2 0 
3 0 0 1 1
3 0 0 0 2 
2 3 0 0 0
....
#endif

// generates a new isotopic formula (where different isotops of the same elements are presented as different places in formula * dst. 

idx sChem::formula::isotopicList(sChem::formula * dst, idx totIsoStart, idx totIsoEnd)
{    
    sChem::formula & frml=*this;

    idx kinds=frml.countAtoms();
    atomcount * atm=frml.add(1), * prvAtm;
    atm->element=0;atm->count=1; // add a dummy element  
    ++kinds;

    // allocate space for all atoms and for all places in formula Cs Hs Os Ns etc 
    sVec < idx > elems; idx * el=elems.add(kinds);
    sVec < idx > bases; idx * b = bases.add(frml.dim());
    idx ik,ie,id;
    for(ik=0, id=0; ik<frml.dim(); ++ik) { // remember the bases of each kind of atoms
        b[ik]=id;
        for(ie=0; ie<frml[ik].count; ++ie)el[id++]=ik;
    }

    // queueue for pending isotopes 
    sVec <idx> massDist; idx * km=massDist.add(kinds);  // here we keep the ways we can implement totIsoCnt mutations


    for(int curIsoCnt=totIsoStart; curIsoCnt<=totIsoEnd; ++curIsoCnt) {
        memset(km,0,sizeof(idx)*kinds);
        km[0]=curIsoCnt; // everything is in the first element 
        idx ie=0, last, ii  ;

        while( ie>=0 ){ // we do until we have non zero components in the first kinds-1 cells (in any cell but the last one ) 

            const char * inValid=0;// check validity 
            
            if(km[kinds-1]!=0)inValid="Xatom";
//            printf("%" DEC "  ", ++iter);for(ik=0; ik<kinds; ++ik ) printf("%s",element((frml[el[ik]].element)->symbol);printf("\n");

//            printf("%" DEC "  ", iter);
            for(ik=0; ik<kinds; ++ik ) {  // perform some checks 
                element * ee=element::ptr(frml[el[ik]].element);
                isotop * isos=element::isoTable.ptr(ee->ofsIsotops);
                
                if(!inValid && km[ik]>0) {
                    for( ii=1; ii<ee->cntIsotops; ++ii )  {
                        if(km[ik]<=isos[ii].iMass-isos[0].iMass)
                            break; 
                    }// check if the atom is too heavy and there is no such isotop for that 
                    if(ii==ee->cntIsotops)inValid="tooHeavy"; 
                }
                if(!inValid && el[ik]==el[ik+1] && km[ik]<km[ik+1])inValid="repeat";
//                printf("%" DEC "",km[ik]);
            }
//            if(inValid)printf(" %s ", inValid);printf("\n");
            
            if(!inValid) {
                prvAtm=0;
                for(ik=0; ik<kinds; ++ik ) { 
                    if(km[ik]!=0) { 
                        idx isoElementToAdd=element::makeIsotopicElement( frml[el[ik]].element , km[ik] );
                        if( prvAtm && prvAtm->element == isoElementToAdd ){
                            ++prvAtm->count;
                        }else{
                            atm=dst->add();
                            atm->element=isoElementToAdd;
                            //atm->count=km[ik];
                            atm->count=1;
                            prvAtm = atm;
                        }
                    }
                }
                atm=dst->add();
                atm->element=0;
                atm->count=0;
            }

            
            ie=kinds-1; // we start running from back to front in search of the first non zero
            //if(km[ie]!=0)last= // the last cell has a zero- remember this fact , we have to sum this thing up with others 
            for(--ie; ie>=0 && km[ie]==0; --ie) ; // find the next non zero 
            // if(el[ie]==frml.dim()-1) // if this is the last non zero element of the last kind 

            if(ie>=0) {
                last=km[kinds-1]; // the last one we always remember 
                km[kinds-1]=0;
                
                id=ie+1; // the destination is the next cell 
                if(km[ie]==1) // however if this cell is not splitting but just moving ... 
                    id=b[el[ie]+1];
                    //while(el[ie]==el[id])++id; // we make sure it moves to the cell with different element id 
                
                --km[ie]; // this cell had something - we decrement 
                ++km[id]; // this cell borrows from previus one 
                km[id]+=last; // the last one adds its content to this destination as well 
            }
        }
    }

    return dst->dim();
}
*/


/// computes distribution ... 
idx sChem::formula::isotopicDistr(sVec < sChem::formula > * dst, real cutoff, real finalcutoff) 
{    
    sChem::formula & frml=*this;

    idx kinds=frml.countAtoms();
    sVec < idx > elems; idx * el=elems.add(kinds);
    idx ik,ie,id;
    for(ik=0, id=0; ik<frml.dim(); ++ik) {for(ie=0; ie<frml[ik].count; ++ie)el[id++]=frml[ik].element;} // now prepare the list of atoms 
    
    sVec < sChem::formula > & peaks = *dst; // each item in the isotop compoisition list has associated isotopic formula 
    
    idx genFirst=0, genLast=1, ig; // the offset for the last generation indexes

    // add a dummy atom zero
    sChem::formula * pc=peaks.add();
    pc->prob=1;

    for(ie=0; ie<elems.dim(); ++ie) { // add elements one by one 
        element * ee=element::ptr(el[ie]);
        isotop * iso=element::isoTable.ptr(ee->ofsIsotops);

        idx genPos=peaks.dim(); // here we start adding for the next generation
        #ifdef ALGO_PRINTFS
            printf("---------------- Adding element #%" DEC " %s\n", ie, ee->symbol);
        #endif
        for(idx ip=genFirst; ip<genLast ; ++ip )  {
            
            
            for(idx ii=0; ii<ee->cntIsotops; ++ii) { // add the very first atoms isotopes
                idx curiso=element::makeIsotopicElement(el[ie],iso[ii].iMass);
                
                
                // copy the current formula and add the current isotop
                sChem::formula  * newPeak=peaks.add();
                sChem::formula  * srcpeak=peaks(ip); // we do it here because the previous command can move the srcpeak 

                atomcount * atmdst, * atmsrc;
                bool found=false;
                for ( idx infrml =0 ; infrml< srcpeak->dim(); ++infrml) {
                    atmdst=newPeak->add();
                    atmsrc=srcpeak->ptr(infrml); 
                    *atmdst=*atmsrc;
                    if(atmsrc->element==curiso){++atmdst->count;found=true;} // if this element exists - just add it to the existing forumla 
                }
                if(!found){atmdst=newPeak->add();atmdst->element=curiso;atmdst->count=1;}
                

                // perform convolution of probabilities 
                newPeak->prob=srcpeak->prob*iso[ii].abundance;


                // now we need to make sure current isotop is not already in the list of this generation peaks 
                
                found=false; // reuse the variable as a marker for finding a hole formula in this generation list 
                atmdst=newPeak->ptr();
                for ( ig=genPos; ig<peaks.dim()-1; ++ig) {
                    idx icnt=peaks(ig)->dim();
                    if(icnt!=newPeak->dim())continue; // different number of atoms - can not be a match 
                    
                    atmsrc=peaks(ig)->ptr();
                    idx diff=icnt; // number of difference in formulas
                    for( idx i1=0; i1<icnt && diff; ++i1 ){
                        for( idx i2=0; i2<icnt && diff; ++i2 ){
                            if( atmsrc[i1].element==atmdst[i2].element && atmsrc[i1].count==atmdst[i2].count ){--diff; continue;}
                        }
                    }// this one is not a difference
                    if(!diff){found=true;break;}
                }


                if(found ) {  // an old formula - need to add up probablities and remove the one added 
                    peaks(ig)->prob+=newPeak->prob;
                    peaks.cut(peaks.dim()-1);
                    newPeak=peaks(ig);
                }
                
            }
        }

        if(ie==elems.dim()-1) 
            cutoff=finalcutoff;

        // now we move staff around to minimize the memory usage 
        sChem::formula * pk=peaks.ptr();
        real totSum=0, totSumFiltr=0;
        idx il, id;
        peaks.del(0,genPos);
        genPos=0;
        for(il=genPos, id=0; il < peaks.dim() ; ++il ) {
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
            
            //pk[id++]=pk[il];
            totSumFiltr+=pk[il].prob;
        }
        #ifdef ALGO_PRINTFS
            printf("##### Probs = %lg %lg\n", totSum, totSumFiltr);
        #endif

        // finally adjust probabilities to  after cutoffs 
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
        // now we move staff around to minimize the memory usage 
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

