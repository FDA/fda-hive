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
#include <ctype.h>

#include <slib/std/string.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/biogencode.hpp>

using namespace slib;

unsigned char sBioseq::mapATGC[256];
unsigned char sBioseq::mapRevATGC[5];
unsigned char sBioseq::mapIUPAC[256];
unsigned char sBioseq::mapRevIUPAC[17];
struct sICodes {idx len;char * code;idx seed;} iupacCodes[256];
idx sBioseq::mapComplementATGC[5];
sVec<idx> sBioseq::mapCodeAA;
idx sBioseq::atgcModel=-1;

//sBioseq::ProtAA sBioseq::listAA[]={
//    {"Alanine","Ala","A","GCT"_"GCC"_"GCA"_"GCG"__},
//    {"Arginine","Arg","R","CGT"_"CGC"_"CGA"_"CGG"_"AGA"_"AGG"__},
//    {"Asparagine","Asn","N","AAT"_"AAC"__},
//    {"Aspartic","Asp","D","GAT"_"GAC"__},
//    {"Cysteine","Cys","C","TGT"_"TGC"__},
//    {"Glutamic","Glu","E","GAG"_"GAA"__},
//    {"Glutamine","Gln","Q","CAA"_"CAG"__},
//    {"Glycine","Gly","G","GGT"_"GGC"_"GGA"_"GGG"__},
//    {"Histidine","His","H","CAT"_"CAC"__},
//    {"Isoleucine","Ile","I","ATT"_"ATC"_"ATA"__},
//    {"Leucine","Leu","L","TTA"_"TTG"_"CTT"_"CTC"_"CTA"_"CTG"__},
//    {"Lysine","Lys","K","AAA"_"AAG"__},
//    {"Methionine","Met","M","ATG"__},
//    {"Phenylalanine","Phe","F","TTT"_"TTC"__},
//    {"Proline","Pro","P","CCT"_"CCC"_"CCA"_"CCG"__},
//    {"Serine","Ser","S","TCT"_"TCC"_"TCA"_"TCG"_"AGT"_"AGC"__},
//    {"Threonine","Thr","T","ACT"_"ACC"_"ACA"_"ACG"__},
//    {"Tryptophan","Trp","W","TGG"__},
//    {"Tyrosine","Tyr","Y","TAT"_"TAC"__},
//    {"Valine","Val","V","GTT"_"GTC"_"GTA"_"GTG"__},
//    {"STOP","Stp","*","TAA"_"TGA"_"TAG"__},
//    {0},
//};

static sStr meta_AAcodons_buf;
static const char * Asx_AACodons[sDim(sBioGenCode::AACodons[0])] = {0};
static const char * Glx_AACodons[sDim(sBioGenCode::AACodons[0])] = {0};
static const char * Any_AACodons[sDim(sBioGenCode::AACodons[0])] = {0};
static const char * Pyl_AACodons[sDim(sBioGenCode::AACodons[0])] = {
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    "UAG"__,
    0
};
static const char * Sec_AACodons[sDim(sBioGenCode::AACodons[0])] = {
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    "UGA"__,
    0
};
sBioseq::ProtAA sBioseq::listAA[]={
    {"Alanine",         "Ala","A",sBioGenCode::AACodons[0]},
    {"Arginine",        "Arg","R",sBioGenCode::AACodons[1]},
    {"Asparagine",      "Asn","N",sBioGenCode::AACodons[2]},
    {"Aspartic",        "Asp","D",sBioGenCode::AACodons[3]},
    {"Cysteine",        "Cys","C",sBioGenCode::AACodons[4]},
    {"Glutamic",        "Glu","E",sBioGenCode::AACodons[5]},
    {"Glutamine",       "Gln","Q",sBioGenCode::AACodons[6]},
    {"Glycine",         "Gly","G",sBioGenCode::AACodons[7]},
    {"Histidine",       "His","H",sBioGenCode::AACodons[8]},
    {"Isoleucine",      "Ile","I",sBioGenCode::AACodons[9]},
    {"Leucine",         "Leu","L",sBioGenCode::AACodons[10]},
    {"Lysine",          "Lys","K",sBioGenCode::AACodons[11]},
    {"Methionine",      "Met","M",sBioGenCode::AACodons[12]},
    {"Phenylalanine",   "Phe","F",sBioGenCode::AACodons[13]},
    {"Proline",         "Pro","P",sBioGenCode::AACodons[14]},
    {"Serine",          "Ser","S",sBioGenCode::AACodons[15]},
    {"Threonine",       "Thr","T",sBioGenCode::AACodons[16]},
    {"Tryptophan",      "Trp","W",sBioGenCode::AACodons[17]},
    {"Tyrosine",        "Tyr","Y",sBioGenCode::AACodons[18]},
    {"Valine",          "Val","V",sBioGenCode::AACodons[19]},
    {"STOP",            "Stp","*",sBioGenCode::AACodons[20]},
    {"Absent",          "Abs","-",sBioGenCode::AACodons[21]},
    {0},
    // AA not in the basic 20 (coded by stop codons in some contexts)
    {"Pyrrolysine",     "Pyl", "O", Pyl_AACodons},
    {"Selenocysteine",  "Sec", "U", Sec_AACodons},
    // ambiguity AA codes
    {"Aspartate or asparagine", "Asx", "B", Asx_AACodons},
    {"Glutamate or glutamine",  "Glx", "Z", Glx_AACodons},
    {"Any",                     "Any", "X", Any_AACodons},
    {0},
};

sBioseq::ProtAA * sBioseq::AAFindByLet(char let)
{
    static bool have_let2iprotAA = false;
    static char let2iprotAA[256] = { 0 };

    if( !have_let2iprotAA ) {
        for(idx iia=0; iia < sDim(listAA); ++iia ) {
            if( listAA[iia].let ) {
                char protlet = listAA[iia].let[0];
                let2iprotAA[(unsigned char)protlet] = let2iprotAA[(unsigned char)tolower(protlet)] = iia + 1;
            }
        }
        have_let2iprotAA = true;
    }

    idx iia = (idx)(let2iprotAA[(unsigned char)let]) - 1;
    return iia >= 0 ? listAA + iia : 0;
}

sBioseq::ProtAA * sBioseq::AAFindByLet3(const char * three)
{
    for(idx iia = 0; iia < sDim(listAA); ++iia) {
        sBioseq::ProtAA * p = sBioseq::listAA + iia;
        if( p->let3 && toupper(three[0]) == p->let3[0] && tolower(three[1]) == p->let3[1] && tolower(three[2]) == p->let3[2] ) {
            return p;
        }
    }
    return 0;
}

void sBioseq::initModule(idx modelnumber)
{

    if(modelnumber!=atgcModel) {  // mapATGC[(idx)'t']!=1
        for(idx i = 0; i < sDim(mapATGC); ++i) {
            mapATGC[i] = (unsigned char) 0xFF;
            iupacCodes[i] = (sICodes){0, 0, 0};
        }
        for(idx i = 0; i < sDim(mapIUPAC); ++i) {
            mapIUPAC[i] = (unsigned char) 0xFF;
        }
        if(modelnumber==eATGC){
            mapATGC[(idx)'a']=0;mapATGC[(idx)'t']=1;mapATGC[(idx)'g']=2;mapATGC[(idx)'c']=3;  mapATGC[(idx)'u']=mapATGC[(idx)'t'];
            mapATGC[(idx)'A']=0;mapATGC[(idx)'T']=1;mapATGC[(idx)'G']=2;mapATGC[(idx)'C']=3;  mapATGC[(idx)'U']=mapATGC[(idx)'T'];
            mapRevATGC[0]='A';
            mapRevATGC[1]='T';
            mapRevATGC[2]='G';
            mapRevATGC[3]='C';
            mapComplementATGC[0]=1;
            mapComplementATGC[1]=0;
            mapComplementATGC[2]=3;
            mapComplementATGC[3]=2;
        }
        else if(modelnumber==eACGT){
            mapATGC[(idx)'a']=0;mapATGC[(idx)'c']=1;mapATGC[(idx)'g']=2;mapATGC[(idx)'t']=3;  mapATGC[(idx)'u']=mapATGC[(idx)'t'];
            mapATGC[(idx)'A']=0;mapATGC[(idx)'C']=1;mapATGC[(idx)'G']=2;mapATGC[(idx)'T']=3;  mapATGC[(idx)'U']=mapATGC[(idx)'T'];
            mapRevATGC[0]='A';
            mapRevATGC[1]='C';
            mapRevATGC[2]='G';
            mapRevATGC[3]='T';
            mapComplementATGC[0]=3;
            mapComplementATGC[1]=2;
            mapComplementATGC[2]=1;
            mapComplementATGC[3]=0;
        }
        mapATGC[(idx)'x']=4;mapATGC[(idx)'n']=4;
        mapATGC[(idx)'X']=4;mapATGC[(idx)'N']=4;
        mapRevATGC[4]='N';
        mapComplementATGC[4]=4;

        mapATGC[(idx)'R']=4;mapATGC[(idx)'r']=4; mapATGC[(idx)'Y']=4;mapATGC[(idx)'y']=4; mapATGC[(idx)'S']=4;mapATGC[(idx)'s']=4;
        mapATGC[(idx)'W']=4;mapATGC[(idx)'w']=4; mapATGC[(idx)'K']=4;mapATGC[(idx)'k']=4; mapATGC[(idx)'M']=4;mapATGC[(idx)'m']=4;
        mapATGC[(idx)'B']=4;mapATGC[(idx)'b']=4; mapATGC[(idx)'D']=4;mapATGC[(idx)'d']=4; mapATGC[(idx)'H']=4;mapATGC[(idx)'h']=4;
        mapATGC[(idx)'V']=4;mapATGC[(idx)'v']=4; mapATGC[(idx)'.']=4;//mapATGC[(idx)'-']=4;

        // iupacCodes['A'] etc. needed for matchIUPAC
        iupacCodes[(idx)'A']=iupacCodes[(idx)'a']=(sICodes){1,(char *)"A",0};
        mapIUPAC[(idx)'A']=mapIUPAC[(idx)'a']=0; mapRevIUPAC[0]='A';
        iupacCodes[(idx)'T']=iupacCodes[(idx)'t']=(sICodes){1,(char *)"T",0};
        mapIUPAC[(idx)'T']=mapIUPAC[(idx)'t']=1; mapRevIUPAC[1]='T';
        iupacCodes[(idx)'G']=iupacCodes[(idx)'g']=(sICodes){1,(char *)"G",0};
        mapIUPAC[(idx)'G']=mapIUPAC[(idx)'g']=2; mapRevIUPAC[2]='G';
        iupacCodes[(idx)'C']=iupacCodes[(idx)'c']=(sICodes){1,(char *)"C",0};
        mapIUPAC[(idx)'C']=mapIUPAC[(idx)'c']=3; mapRevIUPAC[3]='C';
        iupacCodes[(idx)'R']=iupacCodes[(idx)'r']=(sICodes){2,(char *)"AG",0};
        mapIUPAC[(idx)'R']=mapIUPAC[(idx)'r']=4; mapRevIUPAC[4]='R';
        iupacCodes[(idx)'Y']=iupacCodes[(idx)'y']=(sICodes){2,(char *)"CT",0};
        mapIUPAC[(idx)'Y']=mapIUPAC[(idx)'y']=5; mapRevIUPAC[5]='Y';
        iupacCodes[(idx)'S']=iupacCodes[(idx)'s']=(sICodes){2,(char *)"CG",0};
        mapIUPAC[(idx)'S']=mapIUPAC[(idx)'s']=6; mapRevIUPAC[6]='S';
        iupacCodes[(idx)'W']=iupacCodes[(idx)'w']=(sICodes){2,(char *)"AT",0};
        mapIUPAC[(idx)'W']=mapIUPAC[(idx)'w']=7; mapRevIUPAC[7]='W';
        iupacCodes[(idx)'K']=iupacCodes[(idx)'k']=(sICodes){2,(char *)"GT",0};
        mapIUPAC[(idx)'K']=mapIUPAC[(idx)'k']=8; mapRevIUPAC[8]='K';
        iupacCodes[(idx)'M']=iupacCodes[(idx)'m']=(sICodes){2,(char *)"AC",0};
        mapIUPAC[(idx)'M']=mapIUPAC[(idx)'m']=9; mapRevIUPAC[9]='M';
        iupacCodes[(idx)'B']=iupacCodes[(idx)'b']=(sICodes){3,(char *)"CGT",0};
        mapIUPAC[(idx)'B']=mapIUPAC[(idx)'b']=10; mapRevIUPAC[10]='B';
        iupacCodes[(idx)'D']=iupacCodes[(idx)'d']=(sICodes){3,(char *)"AGT",0};
        mapIUPAC[(idx)'D']=mapIUPAC[(idx)'d']=11; mapRevIUPAC[11]='D';
        iupacCodes[(idx)'H']=iupacCodes[(idx)'h']=(sICodes){3,(char *)"ACT",0};
        mapIUPAC[(idx)'H']=mapIUPAC[(idx)'h']=12; mapRevIUPAC[12]='H';
        iupacCodes[(idx)'V']=iupacCodes[(idx)'v']=(sICodes){3,(char *)"ACG",0};
        mapIUPAC[(idx)'V']=mapIUPAC[(idx)'v']=13; mapRevIUPAC[13]='V';
        iupacCodes[(idx)'N']=iupacCodes[(idx)'n']=(sICodes){4,(char *)"ACGT",0};
        mapIUPAC[(idx)'N']=mapIUPAC[(idx)'n']=14; mapRevIUPAC[14]='N';
        iupacCodes[(idx)'.']=iupacCodes[(idx)'-']=(sICodes){4,(char *)"ACGT",0};
        mapIUPAC[(idx)'.']=mapIUPAC[(idx)'-']=15; mapRevIUPAC[15]='-';
        mapRevIUPAC[16]=0;


        // make a codon map to AA
        for(idx j=0; listAA[0].codon00[j];++j){
            for( idx i=0; listAA[i].nm; ++i) {
                for ( const char * p=listAA[i].codon00[j]; p ; p=sString::next00(p) ){
                    char byt=0;
                    if(sLen(p))compressATGC_2Bit (&byt, p, sLen(p) );
                    else continue;
//                    if((j*64+byt)>=sDim(mapCodeAA) )
//                        continue; // should never happen
                    *mapCodeAA.ptrx(j*64+(idx)byt)=i;
                }
            }
        }

        // create meta AA codons
        sVec<idx> asx_offsets(sMex::fExactSize), glx_offsets(sMex::fExactSize), any_offsets(sMex::fExactSize);
        asx_offsets.resize(sDim(sBioGenCode::AACodons[0]));
        glx_offsets.resize(sDim(sBioGenCode::AACodons[0]));
        any_offsets.resize(sDim(sBioGenCode::AACodons[0]));
        for(idx i = 0; i < sDim(sBioGenCode::AACodons[0]); i++) {
            asx_offsets[i] = meta_AAcodons_buf.length();
            // D = aspartate, N = asparagine
            for(const char * codon = AAFindByLet('D')->codon00[i]; codon && *codon; codon = sString::next00(codon) ) {
                meta_AAcodons_buf.add(codon);
            }
            for(const char * codon = AAFindByLet('N')->codon00[i]; codon && *codon; codon = sString::next00(codon) ) {
                meta_AAcodons_buf.add(codon);
            }
            meta_AAcodons_buf.add0(2);

            glx_offsets[i] = meta_AAcodons_buf.length();
            // E = glutamate, Q = glutamine
            for(const char * codon = AAFindByLet('E')->codon00[i]; codon && *codon; codon = sString::next00(codon) ) {
                meta_AAcodons_buf.add(codon);
            }
            for(const char * codon = AAFindByLet('Q')->codon00[i]; codon && *codon; codon = sString::next00(codon) ) {
                meta_AAcodons_buf.add(codon);
            }
            meta_AAcodons_buf.add0(2);

            any_offsets[i] = meta_AAcodons_buf.length();
            // iterate over all non-meta AA codes (which terminate at the row with NULL letter)
            for(idx j = 0; listAA[j].let; j++) {
                if( isalpha(listAA[j].let[0]) ) {
                    for(const char * codon = listAA[j].codon00[i]; codon && *codon; codon = sString::next00(codon) ) {
                        meta_AAcodons_buf.add(codon);
                    }
                }
            }
            meta_AAcodons_buf.add0(2);
        }
        for(idx i = 0; i < sDim(sBioGenCode::AACodons[0]) - 1; i++) {
            Asx_AACodons[i] = meta_AAcodons_buf[asx_offsets[i]] ? meta_AAcodons_buf.ptr(asx_offsets[i]) : 0;
            Glx_AACodons[i] = meta_AAcodons_buf[glx_offsets[i]] ? meta_AAcodons_buf.ptr(glx_offsets[i]) : 0;
            Any_AACodons[i] = meta_AAcodons_buf[any_offsets[i]] ? meta_AAcodons_buf.ptr(any_offsets[i]) : 0;
        }
    }

}

idx sBioseq::compressATGC_2Bit (sStr * buf, const char * seq, idx len, idx isextensiblebuf, sStr *Nbuf)
{
    idx pos=isextensiblebuf ? buf->length() : 0 ;
    idx Npos= (isextensiblebuf) ? Nbuf->length() : 0 ;

    char * b=(char *)buf;
    //char * Nb= (Nbuf) ? (isextensiblebuf ? (char *)Nbuf->ptr(0) : (char *)Nbuf ) : 0;
    char * Nb= (Nbuf) ? (char *)Nbuf->ptr(0)  : 0;
    const char * nxt=seq+len;
    unsigned char let;
    idx is, ib=0, im;
    idx Nib=0, Nim;
    idx seed = 0;

//    iupacCodes['N'].seed=iupacCodes['n'].seed=0;  // Reset seed only for letter 'N'
    idx newseed = 0;

    for ( is=0; seq<nxt  ;  ++seq) { // && *seq!='#'
        let = mapATGC[(idx)(*seq)];
        if(let == 0xFF) continue;

        ib=is/4, im=(is%4)*2;
        Nib = is/8, Nim = is % 8;
        newseed += (let*(is+1)) & 0x7fffffff;
        if(let==4) {
            if (Nbuf && (Nbuf->length() == 0)){
                Nbuf->add(0,(len-1) / 8 + 1);
//                Nbuf->resize();
                Nb = (char *)Nbuf->ptr(Npos);
            }
            if (seed == 0){
                seed = (unsigned)newseed & 0x7fffffff;
            }
            sICodes *iupac = &iupacCodes[(idx)*seq];
            if (iupac->code != 0){
                let = mapATGC[(idx)iupac->code[(idx)(seed % iupac->len)]];
//                iupac->seed = (iupac->seed + 1) % iupac->len;
            }
            else {
                let = seed % 4;
            }
//            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            seed = ((seed * 279470273UL) % 4294967291UL) & 0x7fffffff;
            if (Nbuf != 0){
                Nb[Nib]|=(0x01<<Nim);
            }
        }



        if( isextensiblebuf ){
            buf->resize(pos+ib+1); // make sure buffer is big enough
            b=buf->ptr(pos);

            if (Nbuf){
                Nbuf->resize(Npos + Nib +1);
                Nb = Nbuf->ptr(Npos);
            }
        }

//        if (Nbuf != 0){
//            if (Nim == 0)
//                Nb[Nib] = 0;
//            if (let == 4)
//                Nb[Nib]|=(0x01<<Nim);
//        }

        if(im==0){
            b[ib]=0; // zero the if the first residue goes into our mask
        }
        b[ib]|=(let<<im);
        ++is;
    }
    return is;
}

//idx sBioseq::compressATGC_2Bit (sStr * buf, const char * seq, idx len, idx isextensiblebuf)
//{
//    idx pos=isextensiblebuf ? buf->length() : 0 ;
//
//    char * b=(char *)buf;
//    const char * nxt=seq+len;
//    unsigned char let;
//    idx is, ib=0, im;
//    idx seed = 0;
//
//    for ( is=0; seq<nxt  ;  ++seq) { // && *seq!='#'
//        // if( *seq=='\n' || *seq=='\r' || *seq=='\t' || *seq==' ' )continue;
//        if(strchr("1234567890/"sString_symbolsBlank,*seq))continue;
//
//        let = mapATGC[(idx)(idx)(*seq)];
//        if(let == 0xFF) continue;
//
//        ib=is/4, im=(is%4)*2;
//
//        if( isextensiblebuf ){
//            buf->resize(pos+ib+1); // make sure buffer is big enough
//            b=buf->ptr(pos);
//
//        }
//
//        if (let == 4)
//            let = seed % 4;
//        seed += let;
//
//        if(im==0) b[ib]=0; // zero the if the first residue goes into our mask
//        //if(mapATGC[(idx)(idx)(*seq)]!=0xFF) {
//            b[ib]|=(let<<im);
//        //}
//        ++is;
//    }
//    return is;
//}



idx sBioseq::uncompressATGC_2Bit(sStr * buf, const char * seq, idx start, idx len, idx isextensiblebuf, idx isbrk, idx isrev, idx iscomp)
{
    idx pos = 0;
    char * b = (char *) buf;
    if( isextensiblebuf ) {
        // make sure buffer is big enough
        pos = buf->length();
        buf->resize(pos + (isbrk ? len + ((len - 1) / isbrk) : len) + 1 /* for \0 at the end */);
        b = buf->ptr(pos);
        if( !b ) {
            return start;
        }
        buf->cut(-1); // to hide \0 at the end
    }
    idx is, ia = 0;
    if( !isrev ) {
        for(is = start; is < start + len; ++is) {
            idx ib = is / 4;
            idx im = (is % 4) * 2;
            if( isbrk && is > start && ((is - start) % isbrk) == 0 ) {
                b[is - start + ia++] = '\n';
            }
            if( !iscomp ) {
                b[is - start + ia] = mapRevATGC[((seq[ib] >> im) & 0x3)];
            } else {
                b[is - start + ia] = mapRevATGC[mapComplementATGC[((seq[ib] >> im) & 0x3)]];
            }
        }
    } else {
        for(is = start + len - 1; is >= start; --is) {
            idx ib = is / 4;
            idx im = (is % 4) * 2;
            if( isbrk && is < (start + len - 1) && ((len - is - 1 - start) % isbrk) == 0 ) {
                b[len - is - 1 + start + ia++] = '\n';
            }
            if( !iscomp ) {
                b[len - is - 1 + start + ia] = mapRevATGC[((seq[ib] >> im) & 0x3)];
            } else {
                b[len - is - 1 + start + ia] = mapRevATGC[mapComplementATGC[((seq[ib] >> im) & 0x3)]];
            }
        }
        is = start + len;
    }
    b[is - start + ia] = 0; // add \0 at the end
    return is;
}

idx sBioseq::compressATGC_1Byte (sStr * buf, const char * seq, idx len, idx isextensiblebuf)
{
    idx pos=isextensiblebuf ? buf->length() : 0 ;
    char * b=(char *)buf;
    const char * nxt=seq+len;
    idx is;//, ib=0, im;
    for ( is=0; seq<nxt  ;  ++seq) { // && *seq!='#'
        // if( *seq=='\n' || *seq=='\r' || *seq=='\t' || *seq==' ' )continue;
        if(strchr("1234567890/"sString_symbolsBlank,*seq))continue;

        //ib=is/4, im=(is%4)*2;

        if( isextensiblebuf ){
            buf->resize(pos+is+1); // make sure buffer is big enough
            b=buf->ptr(pos);
        }

        if(mapATGC[(idx)(idx)(*seq)]!=0xFF) {
            b[is]=mapATGC[(idx)(idx)(*seq)];
        }
        ++is;
    }
    return is;
}

idx sBioseq::compressIUPAC (sStr * buf, const char * seq, idx len, idx isextensiblebuf)
{
    idx pos=isextensiblebuf ? buf->length() : 0 ;
    char * b=(char *)buf;
    const char * nxt=seq+len;
    unsigned char let;
    idx is, ib=0, im;
    bool compress = true;
    for ( is=0; seq<nxt  ;  ++seq) { // && *seq!='#'
        // if( *seq=='\n' || *seq=='\r' || *seq=='\t' || *seq==' ' )continue;
        if(strchr("1234567890/"sString_symbolsBlank,*seq))continue;

        ib=is/2, im=(is%2)*4;
        if(im==0){
            b[ib]=0; // zero the if the first residue goes into our mask
        }
        if( isextensiblebuf ){
            buf->resize(pos+is+1); // make sure buffer is big enough
            b=buf->ptr(pos);
        }

        if(mapIUPAC[(idx)(idx)(*seq)]!=0xFF) {
            if (compress){
                let = mapIUPAC[(idx)(idx)(*seq)];
                b[ib]|=(let<<im);
            }
            else {
                b[is] = mapIUPAC[(idx)(idx)(*seq)];
            }

        }
        ++is;
    }
    return is;
}


idx sBioseq::uncompressATGC_1Byte(sStr * buf, const char * seq, idx start, idx len , idx isextensiblebuf, idx isbrk)
{

    idx pos=isextensiblebuf ? buf->length() : 0 ;
    char * b=(char *)buf;
    idx is,ia=0;//, ib, im;
    for ( is=start; is<start+len; ++is ) {

        if( isextensiblebuf ){
            buf->resize(pos+is-start+1+ia); // make sure buffer is big enough
            b=buf->ptr(pos);
        }

        b[is-start+ia]=mapRevATGC[(idx)seq[is]];//mapRevATGC[ ((seq[ib]>>im)&0x3) ];

        if(isbrk && isextensiblebuf && is>start && ((is-start)%isbrk)==0 ){
            buf->add("\n");
            ++ia;
        }
    }

    if(isextensiblebuf){
        buf->resize(pos+is-start+1+ia); // make sure buffer is big enough
        b=buf->ptr(pos);
    }
    b[is-start+ia]=0;
    return is;
}

idx sBioseq::uncompressIUPAC(sStr * buf, const char * seq, idx start, idx len , idx isextensiblebuf, idx isbrk, idx isrev)
{

    idx pos=isextensiblebuf ? buf->length() : 0 ;
    char * b=(char *)buf;
    idx is,ia=0, ib, im;
    bool compress = true;

    if (!isrev){
        for ( is=start; is<start+len; ++is ) {
            ib=is/2; im=(is%2)*4;

            if( isextensiblebuf ){
                buf->resize(pos+is-start+1+ia); // make sure buffer is big enough
                b=buf->ptr(pos);
            }

            if (compress){
                b[is-start+ia]=mapRevIUPAC[ ((seq[ib]>>im)&0xF) ];
            }
            else {
                b[is-start+ia] = mapRevIUPAC[(idx)(seq[is])];
            }

            if(isbrk && isextensiblebuf && is>start && ((is-start)%isbrk)==0 ){
                buf->add("\n");
                ++ia;
            }
        }
    }

    if(isrev){
        for ( is=start+len-1; is>=start; --is ) {

            ib=is/2; im=(is%2)*4;
            if( isextensiblebuf ){
                buf->resize(pos+len-is-1-start+1+ia); // make sure buffer is big enough
                b=buf->ptr(pos);
            }

            if (compress){
                b[len-is-1-start+ia]=mapRevIUPAC[ ((seq[ib]>>im)&0x3) ];
            }
            else {
                b[len-is-1-start+ia] = mapRevIUPAC[(idx)(seq[is])];
            }
            if(isbrk && isextensiblebuf && len-is>start && ((len-is-1-start)%isbrk)==0 ){
                buf->add("\n");
                ++ia;
            }
        }
        is=len-is-1;
    }

    if(isextensiblebuf){
        buf->resize(pos+is-start+1+ia); // make sure buffer is big enough
        b=buf->ptr(pos);
    }
    b[is-start+ia]=0;
    return is;
}

/*idx sBioseq::compressATGCAllele(const char * seq)
{

    idx alleles=0;
    for( idx is=0; seq[is]; ++is) {
        if(mapATGC[(idx)seq[is]]>sizeof(idx)*8-1)continue;
        alleles|=1<<mapATGC[(idx)seq[is]];
    }
    return alleles;
}
idx sBioseq::uncompressATGCAllele(char * seq0, idx allele)
{
    char * seq=seq0;
    for( idx i=0; i<sizeof(idx)*8 ; ++i ) {
        if( ((allele>>i)&0x1) ==0 ) continue;

        if(seq!=seq0) *seq++='/';
        *seq=mapRevATGC[i];
        ++seq;
    }
    *seq=0;

    return allele;
}
*/


idx sBioseq::proteinDecode( char * prot, const char * dna, idx len, bool nozero)
{
    ProtAA * pr;
    idx i=0;
    for( i=0; i*3<len; ++i) {
        pr=codon2AA(dna+3*i);
        prot[i]=pr->let[0];
    }
    if(!nozero) prot[i]=0;
    return i;
}



idx sBioseq::proteinVariantsFromSNP(char * pro, char * dna, idx dlen, idx pos, const char * ale , char * * variants , idx * pcntVariants)
{
    char * pro0=pro;
    idx cntVar=0;
    idx contpos=pos+1, lentail=dlen-pos; // the position if the substitution, the position to move from and the length to move
    for( idx i=0 ; ale[i]!=0; ++i) {
        idx ia;
        for (ia=0; ale[ia+i] && ale[i+ia]!='/' && ale[i+ia]!='-'; ++ia ) ; // determine the length of the insert/snp/delete
        if(pos+ia != contpos )
            memmove(dna+pos+ia,dna+contpos,lentail+1); // make place for it
        for( idx ii=0; ii<ia; ++ii) dna[pos+ii]=ale[i+ii];
        contpos=pos+ia; // next time we move the tail from here
        while(ale[i+ia] && ale[i+ia]!='/') ++ia; // determine the length of the insert/snp/delete
        i+=ia;

        if(variants)variants[cntVar++]=pro;
        pro=pro+1+sBioseq::proteinDecode( pro, dna, dlen+ia-1 );
        //printf("%s\n",pro);
        if(!ale[i])break;
    }
    if(pcntVariants)*pcntVariants=cntVar;
    return pro-pro0;
}


void sBioseq::ProteinNmerIter::init(idx nmer_len, const char * prot, idx prot_len)
{
    _prot = prot;
    _nmer = 0;
    _pos = 0;
    _line = 0;
    _prot_len = prot_len;
    if( _prot && !_prot_len ) {
        _prot_len = sIdxMax;
    }
    _nmer_len = nmer_len;

    _buf.init(sMex::fExactSize);
    _buf.resize(nmer_len + 1);
    _buf.set(0);
    _buf.cut0cut();

    _prot_pos.init(sMex::fExactSize);
    _prot_pos.resize(nmer_len);
    _prot_pos.set(0);
    _prot_pos.cut(0);

    _state = _prot && _prot_len ? eOK : eEOF;
    operator++();
}

sBioseq::ProteinNmerIter & sBioseq::ProteinNmerIter::operator++()
{
    if( _state == eOK ) {
        // scroll bufer 1 letter down
        if( idx blen = _buf.length() ) {
            memmove(_buf.ptr(), _buf.ptr() + 1, blen - 1);
            memmove(_prot_pos.ptr(), _prot_pos.ptr() + 1, sizeof(idx) * (blen - 1));
            _buf.cut0cut(blen - 1);
            _prot_pos.cut(blen - 1);
        }

        while( _buf.length() < _nmer_len ) {
            // scroll past any newlines
            while( _pos < _prot_len && (_prot[_pos] == '\r' || _prot[_pos] == '\n') ) {
                if( _prot[_pos] == '\r' || _pos == 0 || _prot[_pos - 1] != '\r' ) {
                    // increment line count on \r or \r\n or \n not preceded by \r
                    _line++;
                }
                _pos++;
            }
            // on ID line, terminate current nmer and skip to start of next protein sequence
            while( _pos < _prot_len && _prot[_pos] == '>' ) {
                _buf.cut0cut();
                _pos++;
                while( _pos < _prot_len && _prot[_pos] != '\r' && _prot[_pos] != '\n' ) {
                    _pos++;
                }
                while( _pos < _prot_len && (_prot[_pos] == '\r' || _prot[_pos] == '\n') ) {
                    if( _prot[_pos] == '\r' || _pos == 0 || _prot[_pos - 1] != '\r' ) {
                        // increment line count on \r or \r\n or \n not preceded by \r
                        _line++;
                    }
                    _pos++;
                }
            }

            // always record protein position (even in case of error, useful for tracking which letter failed)
            *_prot_pos.add(1) = _pos;

            ProtAA * p = 0;
            if( _pos < _prot_len && (p = AAFindByLet(_prot[_pos])) ) {
                _buf.addString(p->let, 1);
                _pos++;
            } else {
                if( _pos < _prot_len && _prot[_pos] ) {
                    // unknown AA letter!
                    _state = eError;
                } else {
                    _state = eEOF;
                }
                _buf.cut0cut();
                break;
            }
        }
    }

    return *this;
}

const char * sBioseq::pHash(idx hash, idx len)
{
    static  char buf[64];

    sBioseq::uncompressATGC(buf,(const char *)&hash,0, len);
    return buf;
}

bool sBioseq::matchIUPAC(char atgc, char iupac)
{
    const char * iupac_code = iupacCodes[(unsigned char)iupac].code;

    if (!iupac_code)
        return false;

    switch(atgc) {
    case 'a':
    case 'A':
        return strchr(iupac_code, 'A');
    case 't':
    case 'T':
        return strchr(iupac_code, 'T');
    case 'g':
    case 'G':
        return strchr(iupac_code, 'G');
    case 'c':
    case 'C':
        return strchr(iupac_code, 'C');
    case 'n':
    case 'N':
        return strchr(iupac_code, 'N');//*iupac_code;
    }

    return false;
}

idx sBioseq::printFastX(sStr * outFile, bool isFastq, idx start /* = 0 */, idx end /* = 0 */,  idx shift /* = 0 */, bool keepOriginalId /* = false */,  bool appendLength /* = false*/, idx iread /* = 0 */, sVec <idx> * randomIdQueries /*=0*/, bool restoreNs /* = true */, idx filterNs/* = 0*/)
{
    idx q = 0;

    if (end > dim()){
        end = dim();
    }
    if (randomIdQueries && randomIdQueries->dim()>0) {
        for(idx k = 0; k < randomIdQueries->dim(); ++k)  {
            idx i = randomIdQueries->ptr()[k];
            q += printFastXRow(outFile, isFastq, i, 0, 0, shift, keepOriginalId, appendLength, 0, iread, 0, restoreNs, filterNs);
        }
    }
    else {
        for(idx i = start; i < end; ++i) {
            q += printFastXRow(outFile, isFastq, i, 0, 0, shift, keepOriginalId, appendLength, 0, iread, 0, restoreNs, filterNs);
        }
    }

    return q;
}

idx sBioseq::printFastXRow(sStr * outFile, bool isFastq, idx row, idx start /* = 0 */, udx length /* = 0 */, idx shift /* = 0 */, bool keepOriginalId /* = false */, bool appendLength /* = false*/, const char *appendID /* = 0 */, idx iread /* = 0 */, idx isRevCmp /* = 0 */, bool restoreNs /* = true */, idx filterNs/* = 0*/, bool appendRptCount /*= true */, bool SAMCompatible /* = 0*/)
{
    sStr *out;
    if (outFile){
        out = outFile;
    }
    else {
        outstr.cut(0);
        out = &outstr;
    }
    const idx initpos = out->length();

    if( row < 0 || row > dim() ) {
        return 0;
    }
    idx seqlen = len(row, iread);
    if( start < 0 ) {
        start = seqlen + start;
    }
    if( start > seqlen ) {
        return 0;
    }
    idx end = length ? start + length : seqlen;
    if( end > seqlen ) {
        end = seqlen;
    }
    seqlen = end - start;
    char initId = isFastq ? '@' : '>';
    out->add(&initId, 1);//printf("%s", isFastq ? "@" : ">");//!checking is fastq or FASTA
    bool quaBit = getQuaBit(row);//! check the quality of FASTQ sequence , it will return zero if given is FASTA

    const char * seqid = id(row, iread);
    if( seqid[0] == '>' || seqid[0] == '@' ) {
        ++seqid;
    }
    idx subrpt = 1;
    if (appendRptCount && getmode() == 0){
        // short mode, rpt count is printed
        subrpt = rpt(row, iread);
    }
    if( keepOriginalId == false) {
        // short mode, a number is going to be printed
        out->addNum(row + shift);
    } else {
        if (SAMCompatible) {
            sString::searchAndReplaceSymbols(out, seqid, 0, " ", "_", 0, true, true, false, true);
            out->shrink00();
        } else {
            out->addString(seqid);
        }
    }

    if (appendLength){
        out->add(" len=", 5);
        out->addNum(seqlen);
    }
    bool isRev = false;
    bool isComp = false;
    if (isRevCmp == 1){
        out->add(" rev.", 5);
        isRev = true;
    }
    else if (isRevCmp == 2){
        out->add(" comp.", 6);
        isComp = true;
    }
    else if (isRevCmp == 3){
        out->add(" rev.comp.", 10);
        isRev = true;
        isComp = true;
    }
    if (appendID){
        out->addString(appendID);
    }

    if( subrpt > 1 ) {
        out->add(" H#=", 4);
        out->addNum(subrpt);
    }

    out->add("\n", 1);
    idx initseq = out->length() - initpos;
    const char * seqs = seq(row, iread);
    idx seqposition = out->length();
    uncompressATGC(out, seqs, start, seqlen, true, 0, isRev, isComp);
    // restore N based on quality 0
    const char * seqqua = qua(row, iread);

    if( seqqua && restoreNs) {
        idx NCount=0;// count all Ns in one read if it is bigger than filterNs reject the sequence
        char * seqpos = out->ptr(seqposition);
        for(idx i = start, pos = 0; i < start + seqlen; ++i, ++pos) {
            if( Qua(seqqua, i, quaBit) == 0 ) {
                seqpos[pos] = 'N';
                NCount++;
            }
        }
        if( filterNs && (NCount > (seqlen * filterNs / 100)) ) {
            out->cut0cut(initpos);
            return 0;
        }
    }

    idx initqua = 0;

    if( isFastq ) {
        out->add("\n+\n", 3);
        initqua = out->length() - initpos;
        char *outbuf = out->add(0, seqlen);
        idx ipos = 0;
        if(seqqua && !quaBit) {
            for(idx i = start; i < (start+seqlen); ++i, ++ipos) {
                outbuf[ipos] = seqqua[i] + 33;
            }
        } else {
            // fake qualities
            for(idx i = start; i < (start+seqlen); ++i, ++ipos) {
                outbuf[ipos] = 30 + 33;
            }
        }
    }

    if (print_callback){
        // Use the callback function
        idx isValid = print_callback(print_callbackParam, out, initpos, initseq, initqua, seqlen);
        if (!isValid){
            out->cut0cut(initpos);
            return 0;
        }
    }
    out->addString("\n", 1);
    if( !outFile ) {
        ::printf("%s", outstr.ptr());
    }
    return 1;
}

idx sBioseq::printFastXData(sStr * outFile, idx seqlen, const char *seqid, const char *seq, const char *seqqua, idx subrpt, idx iread /* = 0 */)
{
    sStr *out;
    if (outFile){
        out = outFile;
    }
    else {
        outstr.cut(0);
        out = &outstr;
    }
    out->add(seqid);
    if( subrpt > 1 ) {
        out->add(" H#=", 4);
        out->addNum(subrpt);
    }
    out->add("\n");
    out->add(seq, seqlen);
    out->add("\n");
//    outstr.printf("\n%.*s\n", (int)seqlen, seq);
    if( seqqua ) {
        out->add("+\n", 2);
        out->add(seqqua, seqlen);
        out->add("\n", 1);
    }
    out->add0cut();
    if( !outFile ) {
        ::printf("%s", outstr.ptr());
    }
    return 1;
}






