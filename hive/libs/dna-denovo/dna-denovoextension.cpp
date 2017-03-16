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
    seqOrder.cut(0); // set at the beginning of the buffer
    seqCount = 0;
    seqLink.mex()->flags |= sMex::fSetZero;
    seqLink.cut(0);
}

idx __sort_RepeatSorter(DnaDenovoAssembly myThis, void * arr, idx i1, idx i2)
{
//    sDic < sBioal::LenHistogram > * lenHistogram=(sDic < sBioal::LenHistogram > *)param;
//    sDic < sBioal::LenHistogram > * lenHistogram=(sDic < sBioal::LenHistogram > *)param;

    idx ipos1 = (idx) myThis.QrySrc->rpt(i1);
    idx ipos2 = (idx) myThis.QrySrc->rpt(i2);

    if( ipos1 > ipos2 )
        return 1;
    else
        //if( ipos1< ipos2)
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

void DnaDenovoAssembly::extendSeqContig(sBioseq *Qry, idx contig, idx nextcontig, idx startpos)
{
    idx i, ipos, rpt, rpt2;
    char let;//, let2;
    idx sizeC = seqOrder[contig].contiglen;
    idx sizemer = sizeC - startpos;
    idx sizeNC = seqOrder[nextcontig].contiglen - sizemer;

    if( sizeNC < 0 ) {
        sizemer += sizeNC;
        sizeNC = 0;
    }
    seqOrder[contig].contiglen += sizeNC;
    struct seqlinkage *link = seqLink.ptr(seqOrder[contig].link); // Read the link
    idx avgcov = 0;

    // First we need to merge the first sizemer positions
    for(i = 0; i < sizemer; i++) {
        ipos = startpos + i;
        let = seqB(Qry, contig, ipos, &rpt);
        /*let2 =*/ seqB(Qry, nextcontig, i, &rpt2);
        //seqsetB (Qry, contig, ipos, rpt + rpt2);
        setContigRpt(contig, ipos, rpt + rpt2);
        avgcov += rpt2;

    }

    idx sizeAux = 0;
    // Move to the last link
    while( (sizeAux + link->len) < sizeC ) {
        sizeAux += link->len;
        link = seqLink.ptr(link->next);
    }
    idx linkpos = (link - seqLink.ptr()) + 1;
    // Now I need to check:
    // - if it is inbound, do nothing
    // - if it is outbounds, then check if we should extend it right after us, or in other location
    if( (link->len + sizeNC) < (link->end - link->start) ) {
        // We have no problem, as the contig will fit in the reserved space
        // Add the values
        for(i = 0, ipos = sizemer; i < sizeNC; i++, ipos++) {
            let = seqB(Qry, nextcontig, ipos, &rpt);
            seqContig[i + link->start + link->len] = (rpt << 10) | (let & 0x3FF);
            avgcov += rpt;
        }
        link->len += sizeNC;
    } else if( linkpos == seqLink.dim() ) {
        // This is the last one, so extend it with no problems
        idx ext = (((link->len + sizeNC) - (link->end - link->start)) / (sizeContigs)) + 1;
        seqContig.add(ext * sizeContigs);
        link->end += ext * sizeContigs;
        seqLinkCount += (ext * sizeContigs);
        for(i = 0, ipos = sizemer; i < sizeNC; i++, ipos++) {
            let = seqB(Qry, nextcontig, ipos, &rpt);
            seqContig[i + link->start + link->len] = (rpt << 10) | (let & 0x3FF);
            avgcov += rpt;
        }
        link->len += sizeNC;
    } else {
        // It is not the last one, so we need to extend it creating a new link
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
        seqContig.add(ext * sizeContigs);
        seqLinkCount += ext * sizeContigs;

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
        link->len += endpos;
    }
    seqOrder[contig].thickness = (avgcov + seqOrder[contig].thickness*seqOrder[contig].contiglen)/ seqOrder[contig].contiglen;
    return;
}

void DnaDenovoAssembly::mergeonlySeqContig(sBioseq *Qry, idx contig, idx nextcontig, idx startpos)
{
    idx i, ipos, rpt, rpt2;
    //char let, let2;
    //idx sizeC = seqOrder[contig].contiglen;
    idx sizeNC = seqOrder[nextcontig].contiglen;
    //struct seqlinkage *link = seqLink.ptr(seqOrder[contig].link); // Read the link

    idx avgcov = 0;
    // We need to merge the first sizemer positions
    for(i = 0; i < sizeNC; i++) {
        ipos = (startpos) + i;
        /*let = */ seqB(Qry, contig, ipos, &rpt);
        /*let2 = */seqB(Qry, nextcontig, i, &rpt2);
        setContigRpt(contig, ipos, rpt + rpt2);
        avgcov += rpt2;  // we are only interested in the new link
    }
    seqOrder[contig].thickness = (avgcov + seqOrder[contig].thickness*seqOrder[contig].contiglen)/ seqOrder[contig].contiglen;
}

idx DnaDenovoAssembly::getContigRpt(idx contig, idx pos)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;
    idx rpt = (seqContig[position] >> 10) & 0x3FFFFFFFFFFFFF;
    return rpt;
}

char DnaDenovoAssembly::getContigLet(idx contig, idx pos)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;
    idx let = (seqContig[position] & 0x3FF);
    return let;
}

void DnaDenovoAssembly::setContigRpt(idx contig, idx pos, idx newrpt)
{
    //idx rpt = (seqContig[pos] >> 10) & 0x3FFFFFFFFFFFFF;
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;

    idx let = (seqContig[position] & 0x3FF);
    //idx nrpt = newrpt + rpt;
    seqContig[position] = (newrpt << 10) | (let & 0x3FF);
}

void DnaDenovoAssembly::setContigLet(idx contig, idx pos, char newlet)
{
    idx link = seqOrder[contig].link;
    idx position = seqLink[link].start + pos;

    idx rpt = (seqContig[position] >> 10) & 0x3FFFFFFFFFFFFF;
    //idx let = (seqContig[pos] & 0x3FF;
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
//    prev->contigs.glue(next->contigs.ptr());

    prev->contigs.add(numitems);
    //prev->contigs.resize(numitems + base);
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
            //seqCount -= 1;
            //seqOrder[i].contigs.borrow(seqOrder[seqCount].contigs.ptr());
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
//            if (isReverse){
//                *lenpos = (currpos - pos - 1);
//            }
//            else {
//            }
//            *isrevpos = isReverse;
//            let1 = sBioseqAlignment::_seqBits(Qry->seq(seqpos), lenpos, 0);
            return true;
        }
    }
    return false;
}

char DnaDenovoAssembly::seqB(sBioseq *Qry, idx contig, idx pos, idx *rpt)
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
        //let1 = sBioseqAlignment::_seqBits(Qry->seq(seq), seqpos, flags);
        let1 = sBioseqAlignment::_seqBits(Qry->seq(seq), seqpos, flags);
        if( rpt != 0 ) {
            *rpt = Qry->rpt(seq);
        }
        //    if (*qua != 0){
        //        const char * qualities=Qry->qua(seq);
        //        *qua = qualities[seqpos];
        //    }
    } else {
        // I need to get the information from seqContig
        let1 = getContigLet(contig, pos);
        if( rpt != 0 ) {
            *rpt = getContigRpt(contig, pos);
        }
    }
    //BioseqTree::printlet(let1);
    return let1;
}

//char DnaDenovoAssemblyProc::seqsetRpt(sBioseq *Qry, idx contig, idx pos, idx rpt)
//{
//    idx seq, seqpos;
//    bool isRev;
//    char let1;
//    idx flags = 0;
//    if (seqOrder[contig].link == -1){
//        getValues (contig, pos, &seq, &seqpos, &isRev);
//        if (isRev){
//            flags = 0x00000120;
//        }
//        let1 = sBioseqAlignment::_seqBits(Qry->seq(seq), seqpos, flags);
////    *rpt = Qry->rpt(seq);
////    if (*qua != 0){
////        const char * qualities=Qry->qua(seq);
////        *qua = qualities[seqpos];
////    }
//    }
//    else{
//        ;
//
//    }
//    return let1;
//
//}

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
//        idx pos, len, rev, isrev;
//        idx num = seqOrder[position].contigs.dim();
//        idx nodeStr, nodeRev;
//        for(idx i = 0; i < num; i++) {
//            pos = seqOrder[position].contigs[i].seqpos;
//            len = seqOrder[position].contigs[i].seqlen;
//            rev = seqOrder[position].contigs[i].seqrevpos;
//            nodeStr = seqOrder[position].nodeStraight;
//            nodeRev = seqOrder[position].nodeReverse;
//            isrev = (idx) seqOrder[position].contigs[i].isReverse;
//            tempout.printf("( %lld - %lld ), rev = %lld, isRev = %lld  ", pos, len, rev, isrev);
//            tempout.printf("%s pos = %lld rpt = %lld \n", vio->id(pos), pos, vio->rpt(pos));
//            tempout.printf("nodeStr = %lld,  nodeReverse = %lld \n", nodeStr, nodeRev);
//            sStr decompressedACGT;
//            sBioseq::uncompressATGC(&decompressedACGT, vio->seq(pos), 0, vio->len(pos), true, 0, isrev);
//            tempout.printf("%s\n", decompressedACGT.ptr());
//        }
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
//        ::printf("\n+\n");
//        if (1){
//            idx rpt;
//            for (idx i = 0; i < seqlen; i++) {
//                rpt = getContigRpt(position, i);
//                ::printf("%lld ", rpt);
//            }
//        }
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
//    idx num = seqOrder[seqpos].contigs.dim();
    sStr Seq;
    Seq.cut(0);
    Seq.resize(sizemer);
    char * sequence = (char *) Seq;

//    ::printf("\n The sequence to scan in the tree is: ");
    for(idx i = 0; i < sizemer; i++) {
        sequence[i] = seqB(Qry, contigpos, start + i);
//        BioseqTree::printlet(sequence[i]);
    }
//    ::printf("\n");

    idx node = tree->getSubNode(sequence, sizemer);
    return node;
//    idx seqpos1, seq;
//    bool isrev;
//    if (getValues (contigpos, pos, &seq, &seqpos1, &isrev)){
//        idx node = tree->getSubNode(seq, seqpos1, sizemer);
//        return (node);
//    }
//    return -1;
}

idx DnaDenovoAssembly::getSubNodeContig(BioseqTree *tree, idx contigpos, idx start, idx sizemer)
{
    sStr Seq;
    Seq.cut(0);
    Seq.resize(sizemer);
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
//    const char *let1;
    char * s = (char *) sequence;
    sequence->resize(seqOrder[seq].contiglen);
    s = sequence->ptr(0);
    for(idx i = 0; i < seqOrder[seq].contiglen; i++) {
        let = seqB(Qry, seq, i);
//        let1 = &let;
        s[i] = let;
    }
//    ::printf("print seq\n %s\n", sequence->ptr(0));
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
    //  We start from -12 and +/- 5 if it's a match or mismatch
    opt1[0] = -5;
    opt2[0] = -12;
    opt3[0] = -12;
    //            idx score = value + ext -2;
    for(idx i = 1; i < ext; i++) {
        opt1[i] = compareBit(qlet[i], slet[i], value); // case 1:    |_|.
        opt2[i] = compareBit(qlet[i - 1], slet[i], value); // case 2:  \_\.
        opt3[i] = compareBit(qlet[i], slet[i - 1], value); // case 3:  /_/.
        //                score--;
    }

    opt1[ext] = 0, opt2[ext] = 0, opt3[ext] = 0;
    for(idx i = 0; i < ext; i++) {
        opt1[ext] += opt1[i];
        opt2[ext] += opt2[i];
        opt3[ext] += opt3[i];
    }

    if( opt1[ext] >= opt2[ext] && opt1[ext] >= opt3[ext] ) {
        // It is a mismatch (case 1)
        return 1;
    } else if( opt2[ext] == opt3[ext] ) {
        // We need to go further down to investigate the sequence
        return 4;
    } else if( opt2[ext] >= opt3[ext] ) {
        // It is an insert (case 2)
        return 2;
    } else {
        // It is a deletion (case 3)
        return 3;
    }
}

idx DnaDenovoAssembly::alignExtension(sBioseq *Ioseq, idx seq1, idx seq2, idx pos)
{
    sStr Qry, Sub;
    idx maxExtensionGaps = 300000;
    if (!missmatchesPercent){
        missmatchesPercent = 90; // mismatches value out of 100
    }
    idx ext = 3;

    getSequence(&Qry, Ioseq, seq1);
    getSequence(&Sub, Ioseq, seq2);

    const char * qry = Qry;
    idx qrylen = seqOrder[seq1].contiglen;
    const char * sub = Sub;
    idx sublen = seqOrder[seq2].contiglen;

    idx isub = ext - 1, iqry = pos + isub;
    idx diffs = 0, gaps = 0, matches = 0;

//    ::printf("\n seq to compare: seqid1 = %lld, len = %lld\n", seq1, qrylen);
//    for (idx i = 0; i < qrylen; i++)
//        BioseqTree::printlet (qry[i]);
//
//    ::printf("\n to compare with : seqid2 = %lld, len = %lld \n", seq2, sublen);
//    for (idx i = 0; i < sublen; i++)
//        BioseqTree::printlet(sub[i]);
//    ::printf("\n");

//    idx scoreResult;
    idx cbit;

    while( gaps <= (maxExtensionGaps + 1) && diffs * 100 <= missmatchesPercent * (matches + diffs) ) {
        if( isub >= (sublen - ext + 1) ) {
            // sub is a subsequence of qry, we need to discard sub
            return 1;
            break;
        }
        if( iqry >= (qrylen - ext + 1) ) {
            // Both sequences need to merge, cause the second one is an extension of the first one
            return 0;
            break;
        }

        cbit = compareBit(qry[iqry], sub[isub], 1);

        if( cbit == 1 ) {
            // It is a match
            matches++;
            gaps = 0;
        } else {
            ++diffs;


//            return -1;
            // We need to decide what to do, so we put a scoring system.
//            scoreResult = scoring(qry, sub, iqry, isub, ext);
//            if( scoreResult == 1 ) {
//                // It is a mismatch (case 1)
//                diffs++;
//                gaps = 0;
//                ext = 3;
//            } else if( scoreResult == 2 ) {
//                // It is an insert (case 2)
//                iqry -= 1;
//                gaps++;
//                ext = 3;
//            } else if( scoreResult == 3 ) {
//                // It is a deletion (case 3)
//                isub -= 1;
//                gaps++;
//                ext = 3;
//            } else {
//                // We need further data to decide what to do (Case 4)
//                if( ext > 10 ) {
//                    // For weird cases, just consider it an insertion
//                    iqry -= 1;
//                    gaps++;
//                    ext = 3;
//                }
//                isub -= 1;
//                iqry -= 1;
//                ext += 2;
//            }

        }
//        if (isub<=0 || iqry<=0) break;
        isub += 1;
        iqry += 1;
    }

    return -1;
}

char printchar(idx ACGT)
{

    if( ACGT == 0 )
        return ('A');
    else if( ACGT == 1 )
        return ('C');
    else if( ACGT == 2 )
        return ('G');
    else
        return ('T');
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
    //    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //  // _/ sort sequences based on repeat count
    //  // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

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

    //    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //  // _/ loop over sequences and
    //  // _/ filter them according
    //  // _/ to size and repetitions
    //  // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    initseqLinkage();
    qp->reqProgress(1, 5, 100);

    idx pos;
//    rptfilter = 5;
    for(idx countpos = 0; countpos < Qry.dim(); ++countpos) {
        pos = resArr[countpos];
        idx nodeStr;
        idx nodeRev;
        idx seqRev;
        idx seqN;
        if( Qry.len(pos) > lenfilter && Qry.rpt(pos) > rptfilter ) {
            // We add the sequence to the tree
            idx len1 = Qry.len(pos);
            idx rpt1 = Qry.rpt(pos);
            unique = tree.addSequence(pos, len1, rpt1, 0, countpos);
            nodeStr = tree.getLastNode();
            //  Add to the vector
            if( unique == -1 ) {
                uniqueCount++;
            } else {
                // If we get here, it means that this sequence is the reverse
                // complement of another sequence in the file
                // so we don't add a new contig, but only modify their values.
//                    idx seqN = tree.getSeqIndex(nodeStr);
                seqN = tree.getSeqOriginalID(nodeStr);
//                    ::printf("node: %lld nodeStr = %lld ", nodeStr, seqN);
                seqOrder[seqN].nodeReverse = nodeStr;
                seqOrder[seqN].contigs[0].seqrevpos = pos;
                removeCount++;
                // Add a new sequence to keep it, but make it invalid
                addnewSequence(pos, Qry.len(pos), Qry.rpt(pos), nodeStr, 0, seqN, 0);
                continue;
            }
            // We add the reverse complement sequence to the tree
            //            ::printf("\n reverse: ");
            unique = tree.addSequence(pos, len1, rpt1, 1, countpos);
            nodeRev = tree.getLastNode();
            if( (unique == -1) || (nodeRev == nodeStr)) {
                ;
            } else {
                // If we get here, then something is wrong, cause the orig seq
                // has to be the reverse complement of the same sequence.
                removeCount++;
                // Test if they are the same
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
//                seqRev = tree.getSeqIndex(nodeRev);
            //            ::printf("\n\n");
            //            //  Add to the vector
            addnewSequence(pos, Qry.len(pos), Qry.rpt(pos), nodeStr, nodeRev, seqRev);
            //            addnewSequence(pos, Qry.len(pos), 1);

        } else {
            continue;
        }
    }
    // Print nodes
    //    tree.printTree();

    // We need to create long sequences from the ones that are in the tree

    //    seqOrder.resize(uniqueCount);
    //
    ////    seqOrder[seqCount].seqpos = 0;
    ////    seqOrder[seqCount].seqlen = 0;
    //
    //    //const char *seq = Qry.seq(16);
    //
    //    const char *smallseq = Qry.seq(16, lenseq - sizemer, sizemer);
    //    //char let;

    //    sVec <idx> diff, inSort;
    //    sVec <idx> similarities;
    //    similarities.cut(0);
    //    diff.cut(0);
    //    inSort.cut(0);
    //    tree.inOrderTree3(0, &inSort, &diff);
    //
    //    idx rpt, sim, rptsim;
    //
    //    for (idx pos = 0; pos < Qry.dim(); pos++){
    //        rpt = Qry.rpt(pos);
    //        if (pos != (Qry.dim() - 1) )
    //            sim = diff[pos+1];
    //        else sim = 0;
    //        if (sim == -1)
    //            sim = 0;
    //        rptsim = (rpt << 32) | sim;
    //        similarities.vadd(1, rptsim);
    //    }
    //

    //    for (idx pos = 0; pos < Qry.dim(); pos++){
    //        ::printf("%s pos = %lld rpt = %lld sim = %lld \n",Qry.id(pos), pos, Qry.rpt(pos), Qry.sim(pos));
    //        sStr decompressedACTG;
    //        sBioseq::uncompressATGC(&decompressedACTG, Qry.seq(pos), 0, Qry.len(pos));
    //        ::printf ( "%s\n", decompressedACTG.ptr() );
    //        if (strstr(decompressedReference, decompressedACTG) == NULL && strstr(decompressedReferenceRev, decompressedACTG) == NULL )
    //            ::printf ( "Does not exist seq id = %lld \n", pos);
    //    }

    idx straight, reverse;

    for(idx i = 0; i < seqCount; i++) {
        //        printSequenceID(i, &Qry);
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
            //            printSequenceID(i, &Qry);
        }
    }

    //    tree.printTree();

    //    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //  // _/ First Stage
    //  // _/ build contigs
    //  // _/ glue heads and tails
    //  // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    ::printf("\n\n*********** First Stage Begins ***************\n\n");
    idx prevpos = 0;
    idx seqpos, seqlen;
    //    bool isReverse;
    bool isEnd;
    idx elimValid = 0;
    for(idx i = 0; i < uniqueCount; i++) {
        if( seqOrder[i].isValid ) {
            //            prevpos = 7;
            prevpos = i;
            isEnd = true;
            while( isEnd ) {
                seqpos = seqOrder[prevpos].contigs[0].seqpos;
                //                seqlen = seqOrder[prevpos].contigs[0].seqlen;
                seqlen = seqOrder[i].contiglen;
                //                isReverse = seqOrder[prevpos].contigs[0].isReverse;
                // I need to add the reverse complement

//                idx nodepos = getSubNode(&Qry, &tree, seqpos, seqlen - sizemer, sizemer);
                idx nodepos = getSubNode(&Qry, &tree, i, seqlen - sizemer, sizemer);
                //                ::printf ( "\n Node = %lld in position %lld of %lld-mer\n", nodepos, prevpos, sizemer);
                if( nodepos != -1 ) {
                    //                    ::printf ( "\n This is node: %lld\n", nodepos ) ;
                    idx nextpos = tree.getLongestSeq(&nodepos, 0, seqpos);
                    if( (nextpos != -1) && (nextpos != i) ) {

#ifdef _DEBUG
                        //idx noderev;
//                        ::printf ( "\n *** The Next position of %lld in %lld-mer is: %lld\n", prevpos, sizemer, nextpos );
//                        ::printf("\n Previous position pos = %lld will merge with nextpos = %lld, cause node = %lld \n", prevpos, nextpos, nodepos);
//                        printSequenceID(0, i, &Qry, true);
//                        printSequenceID(0, nextpos, &Qry, true);
#endif

                        // Merge both positions and remove one
                        mergeSeq(i, nextpos, sizemer, &tree, nodepos);
#ifdef _DEBUG
//                        ::printf("\n After merging\n");
//                        printSequenceID(0, i, &Qry, true);
#endif

                        //tree.setCount(noderev, 0);
                        // Remove the rptcount from the node in nextpos
                        //i--;
                        prevpos = nextpos;
                        ++elimValid;

#ifdef _DEBUG
//                        ::printf("\n Validating\n");
//                        sStr decompACTG, decompressedReference, decompressedReferenceRev;
//                        //                        getSequence (&decompACTG, &Qry, i);
//                        //                        const char * ref = decompACTG;
//                        decompACTG.mex()->flags |= sMex::fSetZero;
//                        decompACTG.cut(0);
//                        decompACTG.resize(seqOrder[i].contiglen);
//                        char *d = (char *) decompACTG;
//                        d = decompACTG.ptr(0);
//                        idx p;
//                        for(p = 0; p < seqOrder[i].contiglen; p++) {
//                            d[p] = printchar(seqB(&Qry, i, p));
//                        }
//                        d[p] = '\0';
//                        ::printf("\n");
//
//                        //                        for (idx p = 0; p < seqOrder[i].contiglen; p++)
//                        //                            BioseqTree::printlet (ref[p]);
//                        //                        ::printf ( "\n");
//                        //::printf("%s\n", decompressedReference.ptr());
//
//                        const char * foundPos = strstr(decompressedReference.ptr(), decompACTG.ptr());
//                        if( foundPos ) {
//                            for(idx iii = 0; iii < foundPos - decompressedReference.ptr(); ++iii)
//                                ::printf(" ");
//                            ::printf("%s", decompACTG.ptr());
//                        } else
//                            ::printf("\ncouldn't find this forward \n%s\n", decompACTG.ptr());
//
//                        const char * foundPosRev = strstr(decompressedReferenceRev.ptr(), decompACTG.ptr());
//                        if( foundPosRev ) {
//                            for(idx iii = 0; iii < foundPosRev - decompressedReferenceRev.ptr(); ++iii)
//                                ::printf(" ");
//                            ::printf("%s", decompACTG.ptr());
//                        } else
//                            ::printf("\ncouldn't find this reverse \n%s\n", decompACTG.ptr());
//
//                        if( strstr(decompressedReference, decompACTG) == NULL && strstr(decompressedReferenceRev, decompACTG) == NULL )
//                            ::printf("Does not exist seq id = %lld, len = %lld \n", i, seqOrder[i].contiglen);
#endif
                        continue;
                    }

                }
                isEnd = false;
            }
        }
    }


    ////    tree.printTree();
    //
    //    ::printf("\n\n*********** First Stage Results ***************\n\n");
    //
    //    for (idx i=0; i < uniqueCount; i++)
    //    {
    //        if (seqOrder[i].isValid){
    //            printSequenceID(i, &Qry);
    //        }
    //
    //    }
    //

    //    cleanSeqs();
    qp->reqProgress(15, 15, 100);
    if (firstStageOnly == true){
        return 0;
    }

    qp->reqSetInfo(qp->reqId, qp->eQPInfoLevel_Info, "Contig Extension Algorithm: second stage with %" DEC " sequences", uniqueCount);

    idx nodepos, nodecount, nextseq, flag;
    sVec<idx> nodes;

    idx elimseq = 0;
    //    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //  // _/ Second Stage
    //  // _/ build contigs
    //  // _/ glue bodies and heads
    //  // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    ::printf("\n\n*********** Second Stage Begins ***************\n\n");
    initContigs();
    idx maxthickness;
    for(idx it = 0; it < 2; it++) {
        for(idx i = 0; i < uniqueCount; i++) {
            qp->reqProgress(i + 1, i + 1, uniqueCount);

            if( seqOrder[i].isValid ) {
                //  Put seqOrder[i] contig into an sVec that we will use instead
                addSeqContig(&Qry, i);
                //            printSequenceID(i, &Qry);

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
                    for(idx k = 0; k < nodecount; k++){ // Until the end - 1
//                        nextseq = tree.getSeqIndex((idx)nodes[k]);
                        nextseq = tree.getSeqOriginalID((idx) nodes[k]);

                        flag = alignExtension(&Qry, i, nextseq, j);
                        //                    ::printf("node = %lld, nodeStraight = %lld, nodeReverse = %lld\n", nodes[k], seqOrder[nextseq].nodeStraight, seqOrder[nextseq].nodeReverse);
                        //                    ::printf("elimseq = %lld out of %lld\n", elimseq, uniqueCount);
                        if( flag == 1 ) {
                            // Ignore the sequence, cause it is a subsequence of the tree
                            // But we need to add repetition count first
                            mergeonlySeqContig(&Qry, i, nextseq, j);
                            //                        ::printf("Pos = %lld, We should ignore seq = %lld cause it is a subsequence of %lld\n", j, nextseq, i);
                            // Remove the sequence from the tree, putting rptcount to zero.
                            tree.setCount(seqOrder[i].nodeReverse, 0);
                            tree.setCount(seqOrder[nextseq].nodeStraight, 0);
                            tree.setCount(seqOrder[nextseq].nodeReverse, 0);
                            seqOrder[nextseq].isValid = 0;
                            //                        printSequenceID(i, &Qry);
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
                        // Is a continuation of the sequence
                        //                        ::printf("Pos = %lld, We should extend prevpos = %lld with nextpos = %lld \n", j, i, nextseq);
                        ////                        mergeSeq (i, nextseq, k, &tree, nodes[k]);
                        //                        printSequenceID(i, &Qry);
                        //                        printSequenceID(nextseq, &Qry);
                        extendSeqContig(&Qry, i, maxthickness, j);
                        tree.setCount(seqOrder[i].nodeReverse, 0);
                        tree.setCount(seqOrder[maxthickness].nodeStraight, 0);
                        tree.setCount(seqOrder[maxthickness].nodeReverse, 0);
                        seqOrder[maxthickness].isValid = 0;
                        //                        printSequenceID(i, &Qry);
                        elimseq++;
                    } else {
                        // Both sequences are different, so do nothing
                        // ::printf("Pos = %lld, prevpos = %lld and nextpos = %lld are different\n", j, i, nextseq);
                        ;
                    }
                }
            }
        }
    }

    return 0;
}

