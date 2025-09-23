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
#include <ssci/bio/bioseqtree.hpp>

using namespace slib;

idx BioseqTree::addSequence(idx seqnum, idx seqlen, idx seqrpt, idx reversecomplement, idx auxID)
{
    
    NodeTree *father, *son, *son1;
    char let1, let2;
    idx pos, temp;
    idx flags, bitpos, flags2;
    bool revcomp = (reversecomplement) ? 1 : 0;
    if (auxID == -1){ auxID = numSeq;}
    if (reversecomplement == 0){ flags = 0;  }
    else{ flags = 0x00000120; }

    father = iseq.ptr();
    idx qrylen = seqlen;


    const char *seq1 = seq(ofsSeq + seqnum);

    const char *seq2;
        
    if (father->seq_end != -1)
        seq2 = seq(ofsSeq + father->seqIndex);
    else
        seq2 = 0;

    for(pos = 0; pos<qrylen; ++pos) {

        if (reversecomplement == 0){ bitpos = pos;}
        else { bitpos = (qrylen - 1) - pos; }

        let1 = sBioseqAlignment::_seqBits(seq1, bitpos, flags);

        if (father->seq_end <= pos){ 
            if (father->offsets[(idx)let1] == -1){
                temp = father - iseq.ptr();
                son = iseq.add(1);
                father = iseq.ptr() + temp;
                createNode(son, qrylen, seqnum, auxID, revcomp);
                numSeq++;
                son->rptcount += seqrpt;
                if (father->seq_end != -1)
                    father->seq_end = pos;
                father->offsets[(idx)let1] = son - iseq.ptr();
                lastNode = son - iseq.ptr();
                return -1;
            }
            father = father->offsets[(idx)let1] + iseq.ptr();
            
            seq2 = seq(ofsSeq + father->seqIndex);
            continue;
        }

        if (father->revcomp == 0){ bitpos = pos; flags2 = 0;}
        else { bitpos = (len(ofsSeq + father->seqIndex) - 1) - pos; flags2 = 0x00000120;}

        let2 = sBioseqAlignment::_seqBits(seq2, bitpos, flags2);

        if (let1 != let2){   
            temp = father - iseq.ptr();
            son = iseq.add(1);
            father = iseq.ptr() + temp;
            createNode(son, father->seq_end, father->seqIndex, father->seqOriginalID, father->revcomp);
            son->rptcount = father->rptcount;
            for (idx icount = 0; icount < 4; ++icount){
                son->offsets[icount] = father->offsets[icount];
                father->offsets[icount] = -1;
            }
            father->rptcount = 0;
            father->seq_end = pos;
            father->offsets[(idx)let2] = son - iseq.ptr();
            temp = father - iseq.ptr();
            son1 = iseq.add(1);
            father = iseq.ptr() + temp;
            createNode(son1, qrylen, seqnum, auxID, revcomp);
            numSeq++;
            son1->rptcount += seqrpt;;
            father->offsets[(idx)let1] = son1 - iseq.ptr();
            lastNode = son1 - iseq.ptr();
            return -1;
        }
    }
    if (father->seq_end != pos){
        seq2 = seq(ofsSeq + father->seqIndex);

        if (father->revcomp == 0){ bitpos = pos; flags2 = 0;}
        else { bitpos = (len(ofsSeq + father->seqIndex) - 1) - pos; flags2 = 0x00000120;}

        let2 = sBioseqAlignment::_seqBits(seq2, bitpos, flags2);
        temp = father - iseq.ptr();
        son = iseq.add(1);
        father = iseq.ptr() + temp;
        createNode(son, father->seq_end, father->seqIndex, father->seqOriginalID, father->revcomp);
        son->rptcount = father->rptcount;
        for (idx icount = 0; icount < 4; ++icount){
            son->offsets[icount] = father->offsets[icount];
            father->offsets[icount] = -1;
        }
        father->rptcount = 1;
        father->seqIndex = seqnum;
        father->seqOriginalID = auxID;
        father->seq_end = pos;
        father->revcomp = revcomp;
        father->offsets[(idx)let2] = son - iseq.ptr();
        lastNode = father - iseq.ptr();
        numSeq++;
        return -1;
    }
    if (father->rptcount == 0){
        father->seqIndex = seqnum;
        father->revcomp = revcomp;
        father->seqOriginalID = auxID;
        father->rptcount += seqrpt;
        lastNode = father - iseq.ptr();
        numSeq++;
        return -1;
    }
    father->rptcount +=seqrpt;
    lastNode = father - iseq.ptr();
    return (father->seqOriginalID);
}

void BioseqTree::printTree(bool generalInfo)
{
    idx numNodes = iseq.dim();
    NodeTree *leaf;
    idx numUnique = 0;

    if (generalInfo){
        for (idx i =0; i < numNodes; i++){
            leaf = &iseq[i];
            numUnique += leaf ->rptcount;
        }
    }
    else {
        for(idx i = 0; i < numNodes; i++) {
            leaf = &iseq[i];
            numUnique += leaf->rptcount;
            printNode(leaf, leaf - iseq.ptr());
        }
    }
          ::printf( "\n Number of nodes in the Tree = %lld \n", numNodes);
          ::printf( "\n Number of sequences in the Tree = %lld \n", numUnique);
    
}

void BioseqTree::inOrderTree(idx root)
{
    NodeTree *current;
    idx numOffsets = 4;
   
    current = root + iseq.ptr();
    if (current->rptcount != 0){
        printNode (current->seqIndex);
    }
    for (idx i = 0; i < numOffsets; i++){
        if (current->offsets[i] != -1){
            inOrderTree (current->offsets[i]);
        }
    }
    return ;
}

void BioseqTree::inOrderTree2(idx root, sVec <idx> * inSort)
{
    NodeTree *current;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;

    stack.mex()->flags|=sMex::fSetZero; 
    stack.cut(0);

    curr = root;
    stack.vadd(1, curr);
    len = stack.dim();
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        current = curr + iseq.ptr();
        if (current->rptcount != 0){
            inSort->vadd(1, current->seqOriginalID);
        }

        for (idx i = numOffsets-1; 0 <= i; i--){
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    return;
}

void BioseqTree::inOrderTree3(idx root, sVec <idx> * inSort, sVec <idx> * diffSort)
{
    NodeTree *current;
    sVec <idx> stack, queue;
    idx curr, len, offset, offlen;
    idx square = -1;
    idx numOffsets = 4;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);

    queue.mex()->flags|=sMex::fSetZero;
    queue.cut(0);

    curr = root;
    stack.vadd(1, curr);
    len = stack.dim();
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        current = curr + iseq.ptr();
        offset = 0;
        if (current->rptcount != 0){
            inSort->vadd(1, current->seqOriginalID);
        }
        for (idx i = numOffsets-1; 0 <= i; i--){
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
                offset ++;
            }
        }
        if (offset == 0){
            diffSort->vadd(1, square);
            offlen = queue.dim();
            if (offlen > 0){
                square = queue[--offlen];
                queue.del(offlen);
            }
        }
        else{
            for (idx i = 0; i < offset - 1; i++)
                queue.vadd (1, current->seq_end);

            if (current->rptcount != 0){
                diffSort->vadd(1, square);
                square = current->seq_end;
            }
        }
        len = stack.dim();
    }
    return;
}

idx BioseqTree::getSubNode(const char * sequence, idx seqlen, bool isCompressed, bool exactMatch)
{
    NodeTree *father;
    char let1, let2;
    idx pos, bitpos;
    idx flags;
    father = iseq.ptr();


    const char *seq2 = 0;

    if (father->seq_end != -1)
        seq2 = seq(ofsSeq + father->seqIndex);

    for(pos = 0; pos<seqlen; ++pos) {

        if (isCompressed){
            let1 = sBioseqAlignment::_seqBits(sequence, pos);
        }
        else {
            let1 = sequence[pos];
        }
        if (father->seq_end <= pos){
            if (father->offsets[(idx)let1] == -1){
                return -1;
            }
            father = father->offsets[(idx)let1] + iseq.ptr();
            seq2 = seq(ofsSeq + father->seqIndex);
            continue;
        }
        if (father->revcomp == 0){ bitpos = pos; flags = 0;}
        else { bitpos = (len(ofsSeq + father->seqIndex) - 1) - pos; flags = 0x00000120;}

        let2 = sBioseqAlignment::_seqBits(seq2, bitpos, flags);

        if (let1 != let2){
            return -1;
        }
    }

    if (exactMatch && ( (father->seq_end != pos) || (father->rptcount == 0)) ){
            return -1;
    }
    return father - iseq.ptr();
}

idx BioseqTree::getLongestSeq(idx *node, idx rptcount, idx tabu)
{
    NodeTree *current, *best;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;
    idx maxCount = 0;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);

    curr = *node;
    stack.vadd(1, curr);
    len = stack.dim();
    best = 0;
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        current = curr + iseq.ptr();
        if (current->rptcount != 0){
            if ((current->rptcount > maxCount) && (tabu != current->seqIndex)){
                maxCount = current->rptcount;
                best = current;
            }
        }

        for (idx i = numOffsets-1; 0 <= i; i--){
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    if (best == 0)
        return -1;
    best->rptcount = rptcount;
    *node = best - iseq.ptr();
    return best->seqOriginalID;
}

idx BioseqTree::getNodesSeq (sVec<idx> *seqs, idx node, idx rptcount, idx tabu)
{
    NodeTree *current;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);

    curr = node;
    stack.vadd(1, curr);
    len = stack.dim();
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        current = iseq.ptr(curr);
        if (current->rptcount != 0){
            if (tabu != current->seqOriginalID){
                seqs->vadd(1, curr);
            }
        }

        for (idx i = numOffsets-1; 0 <= i; i--){
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    idx count = seqs->dim();
    if (count == 0)
        return -1;
    return count;
}

idx BioseqTree::fixNodeTree(idx node, idx seqnum, idx seqlen, bool isRev)
{
    NodeTree *current;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;
    idx temp = 0;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);
    curr = node;
    stack.vadd(1, curr);
    len = stack.dim();
    current = iseq.ptr(curr);
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        current = iseq.ptr(curr);
        if (current->rptcount !=0){
            if (current->seqIndex == seqnum){
                if (current->revcomp == isRev && current->seq_end == seqlen){
                    return curr;
                }
                else {
                    ++temp;
                }
            }
        }
        for (idx i = numOffsets-1; 0 <= i; i--){
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    return -1;
}


