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
#include <dna-denovo/dna-denovoextension.hpp>

void DnaDenovoAssembly::initseqLinkage()
{
    seqOrder.mex()->flags |= sMex::fSetZero;
    seqOrder.cut(0);
    seqCount = 0;
    seqLink.mex()->flags |= sMex::fSetZero;
    seqLink.cut(0);
}

idx __sort_RepeatSorter(DnaDenovoAssembly myThis, void * arr, idx i1, idx i2)
{

    idx ipos1 = (idx) myThis.QrySrc->rpt(i1);
    idx ipos2 = (idx) myThis.QrySrc->rpt(i2);

    if( ipos1 > ipos2 )
        return 1;
    else
        return -1;
    return 0;
}

void DnaDenovoAssembly::addSeqContig(sBioseq *Qry, idx contig)
{
    char let;
    idx rpt;
    idx li = seqLink.dim();
    idx numcontigs = (seqOrder[contig].contiglen / sizeContigs) + 1;
    idx start;

    seqLink.add();

    start = seqLinkCount;
    seqLink.ptr(li)->start = start;
    seqLink.ptr(li)->end = start + (numcontigs * sizeContigs);
    seqLinkCount += (numcontigs * sizeContigs);
    seqLink.ptr(li)->prev = -1;
    seqLink.ptr(li)->next = -1;
    seqLink.ptr(li)->len = seqOrder[contig].contiglen;

    seqContig.add(sizeContigs * numcontigs);

    idx avgcov = 0;
    for(idx i = 0; i < seqOrder[contig].contiglen; i++) {
        let = seqB(Qry, contig, i, &rpt);
        seqContig[i + start] = (rpt << 10) | (let & 0x3FF);
        avgcov += rpt;
    }
    seqOrder[contig].thickness = avgcov / seqOrder[contig].contiglen;
    seqOrder[contig].link = li;

}

void DnaDenovoAssembly::addNewSeqContig(sBioseq *Qry, idx contig)
{
    idx letacgt;
    idx rpt, qua;
    idx li = seqLink.dim();
    idx numcontigs = (seqOrder[contig].contiglen / sizeContigs) + 1;
    idx start;

    seqLink.add();

    start = seqLinkCount;
    seqLink.ptr(li)->start = start;
    seqLink.ptr(li)->end = start + (numcontigs * sizeContigs);
    seqLinkCount += (numcontigs * sizeContigs);
    seqLink.ptr(li)->prev = -1;
    seqLink.ptr(li)->next = -1;
    seqLink.ptr(li)->len = seqOrder[contig].contiglen;

    seqContigContainer.add(sizeContigs * numcontigs);

    idx avgcov = 0;
    for(idx i = 0; i < seqOrder[contig].contiglen; i++) {
        letacgt = seqB(Qry, contig, i, &rpt, &qua);
        ContigRepresentation *cell = seqContigContainer.ptr(i+start);
        cell->acgt[0] = cell->acgt[1] = cell->acgt[2] = cell->acgt[3] = 0;
        cell->acgt[letacgt] = rpt;
        cell->sumcoverage = rpt;
        cell->consensus = letacgt;
        cell->quality = qua;
        avgcov += rpt;
    }
    seqOrder[contig].link = li;

}

void DnaDenovoAssembly::extendSeqContig(sBioseq *Qry, idx contig, idx nextcontig, idx startpos)
{
    idx i, ipos, rpt, rpt2, qua;
    char let;
    idx sizeC = seqOrder[contig].contiglen;
    idx sizemer = sizeC - startpos;
    idx sizeNC = seqOrder[nextcontig].contiglen - sizemer;

    if( sizeNC < 0 ) {
        sizemer += sizeNC;
        sizeNC = 0;
    }
    seqOrder[contig].contiglen += sizeNC;
    struct seqlinkage *link = seqLink.ptr(seqOrder[contig].link);
    idx avgcov = 0;

    for(i = 0; i < sizemer; i++) {
        ipos = startpos + i;
        if (useNewContigContainer){
            ContigRepresentation *r2 = getNewContigLet(nextcontig, i);
            if (r2){
                rpt2 = r2->sumcoverage;
                addNewContigLet(contig, ipos, r2);
            }
            else {
                idx letacgt = seqB(Qry, nextcontig, i, &rpt2, &qua);
                ContigRepresentation *cell = getNewContigLet(contig, ipos);
                cell->acgt[letacgt] += rpt2;
                cell->sumcoverage += rpt2;
                cell->quality += qua;
                cell->updateConsensus();
            }
        }
        else {
            seqB(Qry, contig, ipos, &rpt);
            seqB(Qry, nextcontig, i, &rpt2, &qua);
            setContigRpt(contig, ipos, rpt + rpt2);
        }
        avgcov += rpt2;
    }

    idx sizeAux = 0;
    while( (sizeAux + link->len) < sizeC ) {
        sizeAux += link->len;
        link = seqLink.ptr(link->next);
    }
    idx linkpos = (link - seqLink.ptr()) + 1;
    if( (link->len + sizeNC) < (link->end - link->start) ) {
        if (useNewContigContainer){
            for(i = 0, ipos = sizemer; i < sizeNC; i++, ipos++) {
                idx letacgt = seqB(Qry, nextcontig, ipos, &rpt, &qua);
                ContigRepresentation *cell = seqContigContainer.ptr(i + link->start + link->len);
                cell->acgt[0] = cell->acgt[1] = cell->acgt[2] = cell->acgt[3] = 0;
                cell->acgt[letacgt] = rpt;
                cell->sumcoverage = rpt;
                cell->consensus = letacgt;
                cell->quality = qua;
                avgcov += rpt;
            }

        }
        else {
            for(i = 0, ipos = sizemer; i < sizeNC; i++, ipos++) {
                let = seqB(Qry, nextcontig, ipos, &rpt);
                seqContig[i + link->start + link->len] = (rpt << 10) | (let & 0x3FF);
                avgcov += rpt;
            }
        }
        link->len += sizeNC;
    } else if( linkpos == seqLink.dim() ) {
        idx ext = (((link->len + sizeNC) - (link->end - link->start)) / (sizeContigs)) + 1;
        link->end += ext * sizeContigs;
        seqLinkCount += (ext * sizeContigs);
        if (useNewContigContainer){
            seqContigContainer.add(ext * sizeContigs);
            for(i = 0, ipos = sizemer; i < sizeNC; i++, ipos++) {
                idx letacgt = seqB(Qry, nextcontig, ipos, &rpt, &qua);
                ContigRepresentation *cell = seqContigContainer.ptr(i + link->start + link->len);
                cell->acgt[0] = cell->acgt[1] = cell->acgt[2] = cell->acgt[3] = 0;
                cell->acgt[letacgt] = rpt;
                cell->sumcoverage = rpt;
                cell->consensus = letacgt;
                cell->quality = qua;
                avgcov += rpt;
            }
        }
        else {
            seqContig.add(ext * sizeContigs);
            for(i = 0, ipos = sizemer; i < sizeNC; i++, ipos++) {
                let = seqB(Qry, nextcontig, ipos, &rpt);
                seqContig[i + link->start + link->len] = (rpt << 10) | (let & 0x3FF);
                avgcov += rpt;
            }
        }
        link->len += sizeNC;
    } else {
        idx li = seqLink.dim();
        idx endpos = (link->end - link->start) - link->len;
        idx ext = (((link->len + sizeNC) - (link->end - link->start)) / sizeContigs) + 1;
        seqLink.add();
        seqLink.ptr(li)->start = seqLinkCount;
        seqLink.ptr(li)->len = ext;
        seqLink.ptr(li)->end = seqLinkCount + ext * sizeContigs;
        seqLink.ptr(li)->prev = linkpos;
        link->next = li;
        seqLink.ptr(li)->next = -1;
        seqLinkCount += ext * sizeContigs;

        if (useNewContigContainer){
            seqContigContainer.add(ext * sizeContigs);
            for(i = 0, ipos = sizemer; i < endpos; i++, ipos++) {
                idx letacgt = seqB(Qry, nextcontig, ipos, &rpt, &qua);
                ContigRepresentation *cell = seqContigContainer.ptr(i + link->start + link->len);
                cell->acgt[0] = cell->acgt[1] = cell->acgt[2] = cell->acgt[3] = 0;
                cell->acgt[letacgt] = rpt;
                cell->sumcoverage = rpt;
                cell->consensus = letacgt;
                cell->quality = qua;
                avgcov += rpt;
            }
            idx offset = seqLink.ptr(li)->start;
            for(; i < sizeNC; i++, ipos++) {
                idx letacgt = seqB(Qry, nextcontig, ipos, &rpt, &qua);
                ContigRepresentation *cell = seqContigContainer.ptr(i + offset);
                cell->acgt[0] = cell->acgt[1] = cell->acgt[2] = cell->acgt[3] = 0;
                cell->acgt[letacgt] = rpt;
                cell->sumcoverage = rpt;
                cell->consensus = letacgt;
                cell->quality = qua;
                avgcov += rpt;
            }
        }
        else {
            seqContig.add(ext * sizeContigs);
            for(i = 0, ipos = sizemer; i < endpos; i++, ipos++) {
                let = seqB(Qry, nextcontig, ipos, &rpt);
                seqContig[i + link->start + link->len] = (rpt << 10) | (let & 0x3FF);
                avgcov += rpt;
            }
            idx offset = seqLink.ptr(li)->start;
            for(; i < sizeNC; i++, ipos++) {
                let = seqB(Qry, nextcontig, ipos, &rpt);
                seqContig[i + offset] = (rpt << 10) | (let & 0x3FF);
                avgcov += rpt;
            }
        }
        link->len += endpos;
    }
    seqOrder[contig].thickness = (avgcov + seqOrder[contig].thickness*seqOrder[contig].contiglen)/ seqOrder[contig].contiglen;
    return;
}

void DnaDenovoAssembly::mergeonlySeqContig(sBioseq *Qry, idx contig, idx nextcontig, idx startpos)
{
    idx i, ipos, rpt, qua;
    idx sizeNC = seqOrder[nextcontig].contiglen;
    idx avgcov = 0;
    for(i = 0; i < sizeNC; i++) {
        ipos = (startpos) + i;
        if (useNewContigContainer){
            ContigRepresentation *r2 = getNewContigLet(nextcontig, i);
            if( r2 ) {
                rpt = r2->sumcoverage;
                addNewContigLet(contig, ipos, r2);
            } else {
                idx letacgt = seqB(Qry, nextcontig, i, &rpt, &qua);
                ContigRepresentation *cell = getNewContigLet(contig, ipos);
                cell->acgt[letacgt] += rpt;
                cell->sumcoverage += rpt;
                cell->quality += qua;
                cell->updateConsensus();
            }
        }
        else {
            idx rpt1;
            seqB(Qry, contig, ipos, &rpt1);
            seqB(Qry, nextcontig, i, &rpt);
            setContigRpt(contig, ipos, rpt1 + rpt);
        }
        avgcov += rpt;
    }
    seqOrder[contig].thickness = (avgcov + seqOrder[contig].thickness*seqOrder[contig].contiglen)/ seqOrder[contig].contiglen;
}

idx DnaDenovoAssembly::getContigRpt(idx contig, idx pos)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;
    idx rpt = 0;
    if (useNewContigContainer){
        rpt = seqContigContainer.ptr(position)->sumcoverage;
    }
    else {
        rpt = (seqContig[position] >> 10) & 0x3FFFFFFFFFFFFF;
    }
    return rpt;
}

DnaDenovoAssembly::ContigRepresentation * DnaDenovoAssembly::getNewContigLet(idx contig, idx pos)
{
    idx link = seqOrder[contig].link;
    if (link < 0){
        return 0;
    }
    idx position = seqLink[link].start + pos;
    return seqContigContainer.ptr(position);

}

char DnaDenovoAssembly::getContigLet(idx contig, idx pos)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;
    idx let = 0;
    if (useNewContigContainer){
        let = seqContigContainer.ptr(position)->consensus;
    }
    else {
        let = (seqContig[position] & 0x3FF);
    }
    return let;
}

void DnaDenovoAssembly::setContigRpt(idx contig, idx pos, idx newrpt)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;

    if (useNewContigContainer){
        seqContigContainer[position].sumcoverage = newrpt;
    }
    else {
        idx let = (seqContig[position] & 0x3FF);
        seqContig[position] = (newrpt << 10) | (let & 0x3FF);
    }

}

void DnaDenovoAssembly::addNewContigLet(idx contig, idx pos, ContigRepresentation *r2)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;

    ContigRepresentation *cell = seqContigContainer.ptr(position);
    cell->acgt[0] += r2->acgt[0];
    cell->acgt[1] += r2->acgt[1];
    cell->acgt[2] += r2->acgt[2];
    cell->acgt[3] += r2->acgt[3];
    cell->sumcoverage += r2->sumcoverage;
    cell->updateConsensus ();
    cell->quality += r2->quality;


}

void DnaDenovoAssembly::setContigLet(idx contig, idx pos, char newlet)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;

    idx rpt = (seqContig[position] >> 10) & 0x3FFFFFFFFFFFFF;
    seqContig[position] = (rpt << 10) | (newlet & 0x3FF);
}

void DnaDenovoAssembly::addnewSequence(idx pos, idx len, idx rpt, idx strNode, idx revNode, idx seqRev, idx validation)
{
    seqOrder.add();
    seqOrder[seqCount].contigs.mex()->flags |= sMex::fSetZero;
    seqOrder[seqCount].contigs.cut(0);
    seqOrder[seqCount].contigs.add();
    seqOrder[seqCount].contigs[0].seqpos = pos;
    seqOrder[seqCount].contigs[0].seqlen = len;
    seqOrder[seqCount].contigs[0].seqrevpos = seqRev;
    seqOrder[seqCount].contigs[0].isReverse = false;
    seqOrder[seqCount].contiglen = len;
    seqOrder[seqCount].isValid = validation;
    seqOrder[seqCount].nodeStraight = strNode;
    seqOrder[seqCount].nodeReverse = revNode;
    seqOrder[seqCount].thickness = rpt;
    seqOrder[seqCount].link = -1;
    seqCount++;
    return;
}

void DnaDenovoAssembly::mergeSeq(idx prevpos, idx nextpos, idx sizemer, BioseqTree * tree, idx thisNode)
{
    struct longSeq *prev = &(seqOrder[prevpos]);
    struct longSeq *next = &(seqOrder[nextpos]);
    idx base = prev->contigs.dim();
    idx numitems = next->contigs.dim();
    bool isRev = tree->getRevComp(thisNode);

    prev->contigs[0].isReverse = false;
    tree->setCount(prev->nodeReverse, 0);

    if( numitems == 1 ) {
        next->contigs[0].isReverse = isRev;
        tree->setCount(next->nodeStraight, 0);
        tree->setCount(next->nodeReverse, 0);
    }
    prev->contiglen += (next->contiglen - sizemer);

    prev->contigs.add(numitems);
    prev->contigs[base - 1].seqlen = prev->contigs[base - 1].seqlen - sizemer;
    for(idx i = 0; i < numitems; i++) {
        prev->contigs[base + i] = next->contigs[i];
    }
    prev->thickness = ((prev->thickness * prev->contiglen) + (next->thickness * next->contiglen)) / (prev->contiglen + next->contiglen);
    next->isValid = 0;
}

void DnaDenovoAssembly::cleanSeqs(void)
{
    idx z = seqCount - 1;
    for(idx i = 0; (i < seqCount) && (i <= z); i++) {
        if( !seqOrder[i].isValid ) {
            for(; (!seqOrder[z].isValid); z--, seqOrder.del(--seqCount))
                ;
            seqOrder[i].contigs.cut(0);
            seqOrder[i].contigs.resize(seqOrder[z].contigs.dim());
            for(idx j = 0; j < seqOrder[z].contigs.dim(); j++) {
                seqOrder[i].contigs[j] = seqOrder[z].contigs[j];
            }
            seqOrder[i].contiglen = seqOrder[z].contiglen;
            seqOrder[i].isValid = 1;
            seqOrder.del(--seqCount);
            z--;
        }
    }
    return;
}

bool DnaDenovoAssembly::getValues(idx contig, idx pos, idx *seqpos, idx *lenpos, bool *isrevpos)
{
    idx currpos = 0;
    struct longSeq *currcon = &(seqOrder[contig]);

    for(idx i = 0; i < currcon->contigs.dim(); i++) {
        currpos += currcon->contigs[i].seqlen;
        if( pos < currpos ) {
            *seqpos = currcon->contigs[i].seqpos;
            *isrevpos = currcon->contigs[i].isReverse;
            *lenpos = pos - (currpos - currcon->contigs[i].seqlen);
            return true;
        }
    }
    return false;
}

char DnaDenovoAssembly::seqB(sBioseq *Qry, idx contig, idx pos, idx *rpt, idx *qua)
{
    idx seq, seqpos;
    bool isRev;
    char let1;
    idx flags = 0;
    if( seqOrder[contig].link == -1 ) {
        getValues(contig, pos, &seq, &seqpos, &isRev);
        if( isRev ) {
            flags = 0x00000120;
            seqpos = Qry->len(seq) - seqpos - 1;
        }
        let1 = sBioseqAlignment::_seqBits(Qry->seq(seq), seqpos, flags);
        if( rpt) {
            *rpt = Qry->rpt(seq);
        }
        if( qua) {
            const char * qualities = Qry->qua(seq);
            *qua = qualities[seqpos];
        }
    } else if (useNewContigContainer){
        ContigRepresentation *rep = getNewContigLet(contig, pos);
        let1 = rep->consensus;
        if( rpt) {
            *rpt = rep->sumcoverage;
        }
        if (qua){
            *qua = rep->quality;
        }
    }
    else {
        let1 = getContigLet(contig, pos);
        if( rpt != 0 ) {
            *rpt = getContigRpt(contig, pos);
        }
    }
    return let1;
}


idx DnaDenovoAssembly::printResult(sFil * outFile, sBioseq *vio, idx limit, bool printFromQuery, const char *seqid)
{
    if (limit == 0){
        limit = outLengthFilter;
    }
    if (printFromQuery == false){
        printFromQuery = firstStageOnly;
    }
    finalseqCount = 0;
    for(idx i = 0; i < seqCount; i++) {
        if( seqOrder[i].isValid && (seqOrder[i].contiglen > limit) ) {
            printSequenceID(outFile, i, vio, printFromQuery, seqid);
            ++finalseqCount;
        }
    }
    return finalseqCount;
}

void DnaDenovoAssembly::printSequenceID(sFil * outFile, idx position, sBioseq *vio, bool printFromQuery, const char *seqid)
{
    sStr tempout;
    if( printFromQuery == true ) {
        tempout.printf("\n>%s: %" DEC " (%" DEC ")  len=%" DEC " sumcov=%.2lf\n", seqid ? seqid : "Sequence" , position, seqOrder[position].isValid, seqOrder[position].contiglen, seqOrder[position].thickness);
        for(idx p = 0; p < seqOrder[position].contiglen; p++)
            BioseqTree::printlet(&tempout, seqB(vio, position, p));
        tempout.printf("\n");
    } else {
        idx seqlen = seqOrder[position].contiglen;
        char let;
        real avgcov = 0;
        for(idx i = 0; i < seqlen; i++) {
            avgcov += getContigRpt(position, i);
        }
        avgcov = avgcov / seqlen;
        tempout.printf(">%s: %lld (%lld)  len=%lld sumcov=%.2lf\n", seqid ? seqid : "Sequence", position, seqOrder[position].isValid, seqlen, avgcov);
        for(idx i = 0; i < seqlen; i++) {
            let = getContigLet(position, i);
            BioseqTree::printlet(&tempout, let);
        }
        tempout.printf("\n");
    }
    if( outFile ) {
        outFile->printf("%s", tempout.ptr());
    } else {
        ::printf("%s", tempout.ptr());
    }

}

idx DnaDenovoAssembly::seqsLength(idx position)
{
    idx total = 0;
    idx num = seqOrder[position].contigs.dim();
    for(idx i = 0; i < num; i++)
        total += seqOrder[position].contigs[i].seqlen;
    return total - (num - 1 * 16);
}

idx DnaDenovoAssembly::getSubNode(sBioseq *Qry, BioseqTree *tree, idx contigpos, idx start, idx sizemer)
{
    char Seq[sizemer];
    char * sequence = (char *) Seq;

    for(idx i = 0; i < sizemer; i++) {
        sequence[i] = seqB(Qry, contigpos, start + i);
    }

    idx node = tree->getSubNode(sequence, sizemer);
    return node;
}

idx DnaDenovoAssembly::getSubNodeContig(BioseqTree *tree, idx contigpos, idx start, idx sizemer)
{
    char Seq[sizemer];
    char * sequence = (char *) Seq;

    for(idx i = 0; i < sizemer; i++) {
        sequence[i] = getContigLet(contigpos, start + i);
    }
    idx node = tree->getSubNode(sequence, sizemer);
    return node;
}

void DnaDenovoAssembly::getSequence(sStr *sequence, sBioseq *Qry, idx seq)
{
    char let;
    char * s = (char *) sequence;
    sequence->resize(seqOrder[seq].contiglen);
    s = sequence->ptr(0);
    for(idx i = 0; i < seqOrder[seq].contiglen; i++) {
        let = seqB(Qry, seq, i);
        s[i] = let;
    }
}

idx DnaDenovoAssembly::compareBit(char let1, char let2, idx value)
{
    if( let1 == let2 )
        return value;
    else
        return value * -1;
}

idx DnaDenovoAssembly::scoring(const char * qry, const char * sub, idx iqry, idx isub, idx ext)
{
    char qlet[ext], slet[ext];
    idx opt1[ext + 1], opt2[ext + 1], opt3[ext + 1];
    idx value = 5;

    for(idx i = 0; i < ext; i++) {
        qlet[i] = qry[iqry + i];
        slet[i] = sub[isub + i];
    }
    opt1[0] = -5;
    opt2[0] = -12;
    opt3[0] = -12;
    for(idx i = 1; i < ext; i++) {
        opt1[i] = compareBit(qlet[i], slet[i], value);
        opt2[i] = compareBit(qlet[i - 1], slet[i], value);
        opt3[i] = compareBit(qlet[i], slet[i - 1], value);
    }

    opt1[ext] = 0, opt2[ext] = 0, opt3[ext] = 0;
    for(idx i = 0; i < ext; i++) {
        opt1[ext] += opt1[i];
        opt2[ext] += opt2[i];
        opt3[ext] += opt3[i];
    }

    if( opt1[ext] >= opt2[ext] && opt1[ext] >= opt3[ext] ) {
        return 1;
    } else if( opt2[ext] == opt3[ext] ) {
        return 4;
    } else if( opt2[ext] >= opt3[ext] ) {
        return 2;
    } else {
        return 3;
    }
}


idx DnaDenovoAssembly::alignExtension(sBioseq *Ioseq, idx seq1, idx seq2, idx pos)
{
    static sStr Qry, Sub;
    idx maxExtensionGaps = 300000;
    if (!missmatchesPercent){
        missmatchesPercent = 90;
    }
    idx ext = 3;

    Qry.cut(0);
    Sub.cut(0);
    getSequence(&Qry, Ioseq, seq1);
    getSequence(&Sub, Ioseq, seq2);

    const char * qry = Qry;
    idx qrylen = seqOrder[seq1].contiglen;
    const char * sub = Sub;
    idx sublen = seqOrder[seq2].contiglen;

    idx isub = ext - 1, iqry = pos + isub;
    idx diffs = 0, gaps = 0, matches = 0;


    idx cbit;

    while( (gaps <= (maxExtensionGaps + 1)) && ((matches * 100) >= missmatchesPercent * (matches + diffs)) ) {
        if( isub >= (sublen - ext + 1) ) {
            return 1;
            break;
        }
        if( iqry >= (qrylen - ext + 1) ) {
            return 0;
            break;
        }

        cbit = compareBit(qry[iqry], sub[isub], 1);

        if( cbit == 1 ) {
            matches++;
            gaps = 0;
        } else {
            ++diffs;



        }
        isub += 1;
        iqry += 1;
    }

    return -1;
}

idx DnaDenovoAssembly::dnaDenovoExtension(sBioseq &Qry, BioseqTree &tree, idx sizem, idx sizef, idx rptf, bool firstStage)
{
    sizemer = sizem;
    lenfilter = sizef;
    rptfilter = rptf;
    firstStageOnly = firstStage;
    return dnaDenovoExtension (Qry, tree);

}

idx DnaDenovoAssembly::dnaDenovoExtension(sBioseq &Qry, BioseqTree &tree)
{
    idx unique = 0;
    idx uniqueCount = 0;
    idx removeCount = 0;
    registerBioseq(&Qry);

    sVec<idx> idxArr, resArr;
    resArr.add(Qry.dim());
    {
        sVec<idx> idxArr;
        idxArr.add(Qry.dim());
        for(idx pos = 0; pos < Qry.dim(); ++pos) {
            idxArr[pos] = Qry.rpt(pos) * -1;
        }
        sSort::sort(Qry.dim(), idxArr.ptr(), resArr.ptr(0));
    }

    initseqLinkage();
    qp->reqProgress(1, 5, 100);

    idx pos;
    for(idx countpos = 0; countpos < Qry.dim(); ++countpos) {
        pos = resArr[countpos];
        idx nodeStr;
        idx nodeRev;
        idx seqRev;
        idx seqN;
        if( Qry.len(pos) > lenfilter && Qry.rpt(pos) > rptfilter ) {
            idx len1 = Qry.len(pos);
            idx rpt1 = Qry.rpt(pos);
            unique = tree.addSequence(pos, len1, rpt1, 0, countpos);
            nodeStr = tree.getLastNode();
            if( unique == -1 ) {
                uniqueCount++;
            } else {
                seqN = tree.getSeqOriginalID(nodeStr);
                seqOrder[seqN].nodeReverse = nodeStr;
                seqOrder[seqN].contigs[0].seqrevpos = pos;
                removeCount++;
                addnewSequence(pos, Qry.len(pos), Qry.rpt(pos), nodeStr, 0, seqN, 0);
                continue;
            }
            unique = tree.addSequence(pos, len1, rpt1, 1, countpos);
            nodeRev = tree.getLastNode();
            if( (unique == -1) || (nodeRev == nodeStr)) {
                ;
            } else {
                removeCount++;
                sStr s1, s2;
                const char *seq1 = Qry.seq(pos);
                idx len1 = Qry.len(pos);
                idx seqN = tree.getSeqIndex(nodeRev);
                const char *seq2 = Qry.seq(seqN);
                idx len2 = Qry.len(seqN);
                sBioseq::uncompressATGC(&s1, seq1, 0, len1, true, 0, 1, 1);
                sBioseq::uncompressATGC(&s2, seq2, 0, len2, true);
                ::printf("%s\n%s", s1.ptr(), s2.ptr());
            }
            seqRev = tree.getSeqOriginalID(nodeRev);
            addnewSequence(pos, Qry.len(pos), Qry.rpt(pos), nodeStr, nodeRev, seqRev);

        } else {
            continue;
        }
    }





    idx straight, reverse;

    for(idx i = 0; i < seqCount; i++) {
        if( seqOrder[i].isValid ) {
            straight = seqOrder[i].nodeStraight;
            reverse = seqOrder[i].nodeReverse;
            if (straight != reverse){
                seqOrder[i].nodeStraight = tree.fixNodeTree(straight, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, false);
                seqOrder[i].nodeReverse = tree.fixNodeTree(reverse, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, true);
            }

            if( seqOrder[i].nodeStraight == -1 || seqOrder[i].nodeReverse == -1 ) {
                ::printf("Houston we have a problem\n");
                seqOrder[i].nodeStraight = tree.fixNodeTree(straight, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, false);
                seqOrder[i].nodeReverse = tree.fixNodeTree(reverse, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, true);
            }
        }
    }


    ::printf("\n\n*********** First Stage Begins ***************\n\n");
    idx prevpos = 0;
    idx seqpos, seqlen;
    bool isEnd;
    idx elimValid = 0;
    for(idx i = 0; i < uniqueCount; i++) {
        if( seqOrder[i].isValid ) {
            prevpos = i;
            isEnd = true;
            while( isEnd ) {
                seqpos = seqOrder[prevpos].contigs[0].seqpos;
                seqlen = seqOrder[i].contiglen;

                idx nodepos = getSubNode(&Qry, &tree, i, seqlen - sizemer, sizemer);
                if( nodepos != -1 ) {
                    idx nextpos = tree.getLongestSeq(&nodepos, 0, seqpos);
                    if( (nextpos != -1) && (nextpos != i) ) {

#ifdef _DEBUG
#endif

                        mergeSeq(i, nextpos, sizemer, &tree, nodepos);
#ifdef _DEBUG
#endif

                        prevpos = nextpos;
                        ++elimValid;

#ifdef _DEBUG
#endif
                        continue;
                    }

                }
                isEnd = false;
            }
        }
    }



    qp->reqProgress(15, 15, 100);
    if (firstStageOnly == true){
        return 0;
    }

    qp->reqSetInfo(qp->reqId, qp->eQPInfoLevel_Info, "Contig Extension Algorithm: second stage with %" DEC " sequences", uniqueCount);

    idx nodepos, nodecount, nextseq, flag;
    sVec<idx> nodes;

    idx elimseq = 0;
    ::printf("\n\n*********** Second Stage Begins ***************\n\n");
    idx nextSequence;
    for(idx it = 0; it < 1; it++) {
        initContigs();
        for(idx i = 0; i < uniqueCount; i++) {
            qp->reqProgress(i + 1, i + 1, uniqueCount);

            if( seqOrder[i].isValid ) {
                addSeqContig(&Qry, i);

                for(idx j = 0; j < seqOrder[i].contiglen; j++) {
                    nodepos = getSubNodeContig(&tree, i, j, sizemer);
                    if( nodepos == -1 ) {
                        continue;
                    }

                    nodes.cut(0);
                    nodecount = tree.getNodesSeq(&nodes, nodepos, 0, i);
                    if( nodecount == -1 ) {
                        continue;
                    }
                    nextSequence = -1;
                    for(idx k = 0; k < nodecount; k++){
                        nextseq = tree.getSeqOriginalID((idx) nodes[k]);

                        flag = alignExtension(&Qry, i, nextseq, j);
                        if( flag == 1 ) {
                            mergeonlySeqContig(&Qry, i, nextseq, j);
                            tree.setCount(seqOrder[i].nodeReverse, 0);
                            tree.setCount(seqOrder[nextseq].nodeStraight, 0);
                            tree.setCount(seqOrder[nextseq].nodeReverse, 0);
                            seqOrder[nextseq].isValid = 0;
                            elimseq++;
                        } else if( flag == 0 ) {
                            if (nextSequence == -1){
                                nextSequence = nextseq;
                            }
                            else if( seqOrder[nextseq].thickness > seqOrder[nextSequence].thickness ) {
                                nextSequence = nextseq;
                            }
                        }
                    }
                    if( nextSequence != -1 ) {
                        extendSeqContig(&Qry, i, nextSequence, j);
                        tree.setCount(seqOrder[i].nodeReverse, 0);
                        tree.setCount(seqOrder[nextSequence].nodeStraight, 0);
                        tree.setCount(seqOrder[nextSequence].nodeReverse, 0);
                        seqOrder[nextSequence].isValid = 0;
                        elimseq++;
                    } else {
                        ;
                    }
                }
            }
        }
    }

    return 0;
}

idx DnaDenovoAssembly::dnaContigExtension(sBioseq &Qry, BioseqTree &tree, idx sizem, idx sizef, idx rptf, bool firstStage, sStr *err)
{
    sizemer = sizem;
    lenfilter = sizef;
    rptfilter = rptf;
    firstStageOnly = firstStage;

    idx unique = 0;
    idx uniqueCount = 0;
    idx removeCount = 0;

    registerBioseq(&Qry);
    idx qrydim = Qry.dim();

    sVec<idx> idxArr, resArr, indxArr;
    resArr.add(qrydim);
    {
        sVec<idx> idxArr;
        idxArr.add(qrydim);
        for(idx pos = 0; pos < qrydim; ++pos) {
            idxArr[pos] = Qry.rpt(pos) * -1;
        }
        sSort::sort(qrydim, idxArr.ptr(), resArr.ptr(0));
    }


    initseqLinkage();
    qp->reqProgress(1, 5, 100);

    idx pos;
    for(idx countpos = 0; countpos < qrydim; ++countpos) {
        pos = resArr[countpos];
        idx nodeStr;
        idx nodeRev;
        idx seqRev;
        idx seqN;
        idx len1 = Qry.len(pos);
        idx rpt1 = Qry.rpt(pos);
        if( (len1 > lenfilter) && (rpt1 > rptfilter) ) {
            unique = tree.addSequence(pos, len1, rpt1, 0, countpos);
            nodeStr = tree.getLastNode();
            if( unique == -1 ) {
                uniqueCount++;
            } else {
                seqN = tree.getSeqOriginalID(nodeStr);
                seqOrder[seqN].nodeReverse = nodeStr;
                seqOrder[seqN].contigs[0].seqrevpos = pos;
                seqOrder[seqN].thickness += rpt1;
                removeCount++;
                addnewSequence(pos, len1, rpt1, nodeStr, 0, seqN, 0);
                continue;
            }
            unique = tree.addSequence(pos, len1, rpt1, 1, countpos);
            nodeRev = tree.getLastNode();
            if( (unique == -1) || (nodeRev == nodeStr)) {
                ;
            } else {
                removeCount++;
                sStr s1, s2;
                const char *seq1 = Qry.seq(pos);
                idx len1 = Qry.len(pos);
                idx seqN = tree.getSeqIndex(nodeRev);
                const char *seq2 = Qry.seq(seqN);
                idx len2 = Qry.len(seqN);
                sBioseq::uncompressATGC(&s1, seq1, 0, len1, true, 0, 1, 1);
                sBioseq::uncompressATGC(&s2, seq2, 0, len2, true);
                if (err){
                    err->printf("Error\n%s\n%s", s1.ptr(), s2.ptr());
                }
                return false;
            }
            seqRev = tree.getSeqOriginalID(nodeRev);
            addnewSequence(pos, len1, rpt1, nodeStr, nodeRev, seqRev);

        }
    }

    idx straight, reverse;

    for(idx i = 0; i < seqCount; i++) {
        if( seqOrder[i].isValid ) {
            straight = seqOrder[i].nodeStraight;
            reverse = seqOrder[i].nodeReverse;
            if (straight != reverse){
                seqOrder[i].nodeStraight = tree.fixNodeTree(straight, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, false);
                seqOrder[i].nodeReverse = tree.fixNodeTree(reverse, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, true);
            }

            if( seqOrder[i].nodeStraight == -1 || seqOrder[i].nodeReverse == -1 ) {
                if (err){
                    err->printf("Houston we have a problem\n");
                }
                seqOrder[i].nodeStraight = tree.fixNodeTree(straight, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, false);
                seqOrder[i].nodeReverse = tree.fixNodeTree(reverse, seqOrder[i].contigs[0].seqpos, seqOrder[i].contigs[0].seqlen, true);
                return false;
            }
        }
    }
    resArr.resize(uniqueCount);
    indxArr.resize(uniqueCount);
    {
        sVec<idx> idxArr;
        idxArr.resize(uniqueCount);
        idx count = 0;
        for(idx pos = 0; pos < seqCount; ++pos) {
            if (seqOrder[pos].isValid){
                idxArr[count] = seqOrder[pos].thickness * -1;
                indxArr[count++] = pos;
            }
        }
        sSort::sort(uniqueCount, idxArr.ptr(), resArr.ptr(0));
    }


    ::printf("\n\n*********** First Stage Begins ***************\n\n");
    idx prevpos = 0;
    idx seqpos, seqlen;
    bool isEnd;
    idx elimValid = 0;
    for(idx ipos = 0; ipos < 0; ++ipos) {
        idx i = indxArr[resArr[ipos]];
        if( seqOrder[i].isValid ) {
            prevpos = i;
            isEnd = true;
            while( isEnd ) {
                seqpos = seqOrder[prevpos].contigs[0].seqpos;
                seqlen = seqOrder[i].contiglen;

                idx nodepos = getSubNode(&Qry, &tree, i, seqlen - sizemer, sizemer);
                if( nodepos != -1 ) {
                    idx nextpos = tree.getLongestSeq(&nodepos, 0, seqpos);
                    if( (nextpos != -1) && (nextpos != i) ) {

#ifdef _DEBUG
#endif

                        mergeSeq(i, nextpos, sizemer, &tree, nodepos);
#ifdef _DEBUG
#endif

                        prevpos = nextpos;
                        ++elimValid;

#ifdef _DEBUG
#endif
                        continue;
                    }

                }
                isEnd = false;
            }
        }
    }


        ::printf("\n\n*********** First Stage Results ***************\n\n");

        idx validCount = 0;
        for (idx i=0; i < uniqueCount; i++)
        {
            if (seqOrder[i].isValid){
                ++validCount;
            }
        }
        ::printf("We have %" DEC " valid sequences after First Stage\n", validCount);


    qp->reqProgress(15, 15, 100);
    if (firstStageOnly == true){
        return 0;
    }

    qp->reqSetInfo(qp->reqId, qp->eQPInfoLevel_Info, "Contig Extension Algorithm: second stage with %" DEC " sequences", uniqueCount);

    idx nodepos, nodecount, nextseq, flag;
    sVec<idx> nodes;

    idx elimseq = 0;
    ::printf("\n\n*********** Second Stage Begins ***************\n\n");
    initNewContigs();
    idx maxthickness;
    for(idx it = 0; it < 1; it++) {
        for(idx ipos = 0; ipos < uniqueCount; ipos++) {
            idx i = indxArr[resArr[ipos]];
            qp->reqProgress(i + 1, i + 1, uniqueCount);

            if( seqOrder[i].isValid ) {
                addNewSeqContig(&Qry, i);

                for(idx j = 0; j < seqOrder[i].contiglen; j++) {
                    nodepos = getSubNodeContig(&tree, i, j, sizemer);
                    if( nodepos == -1 ) {
                        continue;
                    }

                    nodes.cut(0);
                    nodecount = tree.getNodesSeq(&nodes, nodepos, 0, i);
                    if( nodecount == -1 ) {
                        continue;
                    }
                    maxthickness = -1;
                    for(idx k = 0; k < nodecount; k++){
                        nextseq = tree.getSeqOriginalID((idx) nodes[k]);

                        flag = alignExtension(&Qry, i, nextseq, j);
                        if( flag == 1 ) {
                            mergeonlySeqContig(&Qry, i, nextseq, j);
                            tree.setCount(seqOrder[i].nodeReverse, 0);
                            tree.setCount(seqOrder[nextseq].nodeStraight, 0);
                            tree.setCount(seqOrder[nextseq].nodeReverse, 0);
                            seqOrder[nextseq].isValid = 0;
                            elimseq++;
                        } else if( flag == 0 ) {
                            if (maxthickness == -1){
                                maxthickness = nextseq;
                            }
                            else if( seqOrder[nextseq].thickness > seqOrder[maxthickness].thickness ) {
                                maxthickness = nextseq;
                            }
                        }
                    }
                    if( maxthickness != -1 ) {
                        extendSeqContig(&Qry, i, maxthickness, j);
                        tree.setCount(seqOrder[i].nodeReverse, 0);
                        tree.setCount(seqOrder[maxthickness].nodeStraight, 0);
                        tree.setCount(seqOrder[maxthickness].nodeReverse, 0);
                        seqOrder[maxthickness].isValid = 0;

                        elimseq++;
                    } else {
                        ;
                    }
                }
            }
        }
        idx uniqueTotal = 0;
        for(idx ipos = 0; ipos < uniqueCount; ipos++) {
            idx i = resArr[ipos];
            if( seqOrder[i].isValid ) {
                ++uniqueTotal;
            }
        }
        ::printf("After round: %" DEC ", there are %" DEC " unique sequences to be merged\n", it+1, uniqueTotal);
    }

    return 0;
}

