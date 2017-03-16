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
#include <ssci/bio/sviodbseq.hpp>
#include <ssci/bio/vioseq2.hpp>

idx sVioDBseq::getLen (idx i)
{
    udx sizetype;
    udx irel, relcnt, reltype;
    const char *id;//, *seq;
    sVioseq2::Rec * rec;

    if (i < 0 || i >= length)
        return -1;
    if (mode){
        id = (const char *) db->Getbody (ID_TYPE, i+1, &sizetype);
        irel = *db->GetRelationPtr(ID_TYPE, i+1, 1, &relcnt, &reltype);

        rec = (sVioseq2::Rec *) db->Getbody (reltype, irel, &sizetype);
        return rec->lenSeq;
    }
    else {
        rec = (sVioseq2::Rec *) db->Getbody (REC_TYPE, i+1, &sizetype);
        return rec->lenSeq;
    }
}

idx sVioDBseq::getCount(idx i)
{
    udx sizetype;
    udx irel, relcnt, reltype;
    const char *id;
    sVioseq2::Rec * rec;

    if (i < 0 || i >= length)
        return -1;
    if (mode){
        id = (const char *) db->Getbody (ID_TYPE, i+1, &sizetype);
        irel = *db->GetRelationPtr(ID_TYPE, i+1, 1, &relcnt, &reltype);
        rec = (sVioseq2::Rec *) db->Getbody (reltype, irel, &sizetype);
        return rec->countSeq;
    }
    else {
        rec = (sVioseq2::Rec *) db->Getbody (REC_TYPE, i+1, &sizetype);
        return rec->countSeq;
    }
}


const char *sVioDBseq::getSeq(idx i)
{
    udx sizetype;
    udx irel, relcnt, reltype;
    const char *id, *seq;
    sVioseq2::Rec * rec;

    if (i < 0 || i >= length)
        return 0;
    if (mode){
        id = (const char *) db->Getbody (ID_TYPE, i+1, &sizetype);
        irel = *db->GetRelationPtr(ID_TYPE, i+1, 1, &relcnt, &reltype);
        rec = (sVioseq2::Rec *) db->Getbody (reltype, irel, &sizetype);
        irel = *db->GetRelationPtr(reltype, irel, 2, &relcnt, &reltype);
        seq = (const char *) db->Getbody (reltype, irel, &sizetype);
        return seq;
    }
    else {
        rec = (sVioseq2::Rec *) db->Getbody (REC_TYPE, i+1, &sizetype);
        //irel = *db->GetRelationPtr(2, i+1, 1, &relcnt, &reltype);
        //id = (const char *) db->Getbody (reltype, irel, &sizetype);
        irel = *db->GetRelationPtr(2, i+1, 2, &relcnt, &reltype);
        seq= (const char *) db->Getbody (reltype, irel, &sizetype);
        return seq;
    }

}

const char *sVioDBseq::getQua(idx i)
{
    udx sizetype;
    udx irel, relcnt, reltype;
    const char *id, *qua;
    sVioseq2::Rec * rec;

    if (i < 0 || i >= length)
        return 0;
    if (mode){
        id = (const char *) db->Getbody (ID_TYPE, i+1, &sizetype);
        irel = *db->GetRelationPtr(ID_TYPE, i+1, 1, &relcnt, &reltype);
        rec = (sVioseq2::Rec *) db->Getbody (reltype, irel, &sizetype);
        irel = *db->GetRelationPtr(reltype, irel, 3, &relcnt, &reltype);
        qua = (const char *) db->Getbody (reltype, irel, &sizetype);
        return qua;
    }
    else {
        rec = (sVioseq2::Rec *) db->Getbody (REC_TYPE, i+1, &sizetype);
        //irel = *db->GetRelationPtr(2, i+1, 1, &relcnt, &reltype);
        //id = (const char *) db->Getbody (reltype, irel, &sizetype);
        irel = *db->GetRelationPtr(2, i+1, 3, &relcnt, &reltype);
        qua = (const char *) db->Getbody (reltype, irel, &sizetype);
        return qua;
    }

}

const char *sVioDBseq::getID(idx i)
{
    udx sizetype;
    udx irel, relcnt, reltype;
    const char *id;//, *seq;
    sVioseq2::Rec * rec;

    if (i < 0 || i >= length)
        return 0;
    if (mode){
        id = (const char *) db->Getbody (ID_TYPE, i+1, &sizetype);
        return id;
    }
    else {
        rec = (sVioseq2::Rec *) db->Getbody (REC_TYPE, i+1, &sizetype);
        irel = *db->GetRelationPtr(2, i+1, 1, &relcnt, &reltype);
        id = (const char *) db->Getbody (reltype, irel, &sizetype);
        return id;
    }

}



