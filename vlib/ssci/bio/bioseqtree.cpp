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

//    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    // _/
//    // _/ Methods of the class BioseqTree
//    // _/
//    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//  addSequence:  
//      It adds a new sequence to the tree, adding two nodes as maximum per sequence.
//      
//   Input :
//          It requires the index, length and repetitions of the sequence
//      that is candidate to be inserted.
//   Output: It returns a value of:  
//              -1 - if the candidate sequence is new in the tree.
//           or id - the candidate sequence is not new, and 'id' is the index of the 
//                   other sequence exactly as the candidate.
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

//    const char * seq1 = qryTree->seq(seqnum);
//    const char * seq2 = qryTree->seq(father->seqoriginalID);  // pointer to the sequence in qryTree
    //Rec *origRec1 = vofs->ptr(seqnum);

    //char *seq1 = qryTree->ptr(ofsSeq + seqnum);
    const char *seq1 = seq(ofsSeq + seqnum);

    //Rec *origRec2 = vofs->ptr(father->seqoriginalID);
    const char *seq2;
        
    if (father->seq_end != -1)
        seq2 = seq(ofsSeq + father->seqIndex);
    else
        seq2 = 0;
        //seq2 = qryTree->ptr(ofsSeq + father->seqIndex);

    // Walk for every position of the sequence and look for similarities in the tree
    for(pos = 0; pos<qrylen; ++pos) {

        if (reversecomplement == 0){ bitpos = pos;}
        else { bitpos = (qrylen - 1) - pos; }

        let1 = sBioseqAlignment::_seqBits(seq1, bitpos, flags); // retrieve 2-bit presentation of a particular base (A,C,G,T)  for the current query position

//        printlet(let1);
        // We check if we have reach the last element of the node.
        if (father->seq_end <= pos){ 
            // We have reached the last element, so we need to check if 
            // there is another node after this node.
            if (father->offsets[(idx)let1] == -1){
                // There is no other node, so we need to add a new node.
                // Case 1. A new son (son) is added to the previous node (father):
                //                     (1)           (1)            (1)
                //     (1)       ->     |     OR    | |     ->     | | |
                //                     (2)        (2)(3)         (2)(3)(4)
                temp = father - iseq.ptr();
                son = iseq.add(1);
                father = iseq.ptr() + temp;
                createNode(son, qrylen, seqnum, auxID, revcomp);
                numSeq++;
                son->rptcount += seqrpt;
                if (father->seq_end != -1)  // Just to avoid moving root index from -1
                    father->seq_end = pos;
                father->offsets[(idx)let1] = son - iseq.ptr();
                lastNode = son - iseq.ptr();
                return -1;
            }
            // We have not reached the last element of the tree, so we move 
            // father to the next position in the tree.
            father = father->offsets[(idx)let1] + iseq.ptr();
            
            seq2 = seq(ofsSeq + father->seqIndex);
            //seq2 = qryTree->ptr(ofsSeq + father->seqIndex);
            //seq2 = qryTree->seq(father->seqoriginalID);  // pointer to the sequence in qryTree
            continue;
        }

        //  We read the next element of the sequence in the tree to compare with candidate
        if (father->revcomp == 0){ bitpos = pos; flags2 = 0;}
        else { bitpos = (len(ofsSeq + father->seqIndex) - 1) - pos; flags2 = 0x00000120;}

        let2 = sBioseqAlignment::_seqBits(seq2, bitpos, flags2); // retrieve 2-bit presentation of a particular base (A,T,G,C)  for the current query position

        //printlet(let2);
        // We check that both elements are different
        if (let1 != let2){   
            // We found a difference, so we will break the sequence in 2 
            // Case 2. A new node (son1) and an internal node (son) are added:
            //             |                |
            //             |               (3)
            //            (1)     ->     |   |
            //                          (1) (2)
            temp = father - iseq.ptr();
            son = iseq.add(1);
            father = iseq.ptr() + temp;
            // First node is created with all father's properties.
            createNode(son, father->seq_end, father->seqIndex, father->seqOriginalID, father->revcomp);
            son->rptcount = father->rptcount;
            // Move the offsets to the new node and clear the actual ones
            for (idx icount = 0; icount < 4; ++icount){
                son->offsets[icount] = father->offsets[icount];
                father->offsets[icount] = -1;
            }
            // Update the values for the old node (father)
            father->rptcount = 0;
            father->seq_end = pos;
            father->offsets[(idx)let2] = son - iseq.ptr();
            temp = father - iseq.ptr();
            son1 = iseq.add(1);
            father = iseq.ptr() + temp;
            // Second Node is created with the candidate sequence.
            createNode(son1, qrylen, seqnum, auxID, revcomp);
            numSeq++;
            son1->rptcount += seqrpt;;
            father->offsets[(idx)let1] = son1 - iseq.ptr();
            lastNode = son1 - iseq.ptr();
            return -1;
        }
    }
    //  If we reached this point, it means that the candidate sequence shares a node with 
    // another sequence in the tree.  But we can have two possibilities:
    //    1. candidate sequence is a subsequence of original sequence
    //    2. they both are the same sequence.
    if (father->seq_end != pos){
        // the new sequence is a subsequence of the original one and we will create a 
        // new node.
        // Case 3. A new son (son) is added to the previous node (father):
        //                                                  (1)
        //                    (1)            (1)    ->       |
        //     (1)      ->     |      OR    / |             (4)
        //                    (2)         (2)(3)           / | 
        //                                               (2)(3)  
        // First we need to read the next element of father to move the offset
        // into the right direction
        seq2 = seq(ofsSeq + father->seqIndex);
        //seq2 = qryTree->ptr(ofsSeq + father->seqIndex);
        //seq2 = qryTree->seq(father->seqoriginalID);  // pointer to the sequence in qryTree

        if (father->revcomp == 0){ bitpos = pos; flags2 = 0;}
        else { bitpos = (len(ofsSeq + father->seqIndex) - 1) - pos; flags2 = 0x00000120;}

        let2 = sBioseqAlignment::_seqBits(seq2, bitpos, flags2); // retrieve 2-bit presentation of a particular base (A,T,G,C)  for the current query position
        temp = father - iseq.ptr();
        son = iseq.add(1);
        father = iseq.ptr() + temp;
        // We create the node with all the father's properties
        createNode(son, father->seq_end, father->seqIndex, father->seqOriginalID, father->revcomp);
        son->rptcount = father->rptcount;
        for (idx icount = 0; icount < 4; ++icount){
            son->offsets[icount] = father->offsets[icount];
            father->offsets[icount] = -1;
        }
        // We modified the father's offset, count and sequence ID
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
        //  If count == 0, it means that father's sequence does not finish at this node
        // so the candidate node is a subsequence of the original one.
        //  So we don't modify the tree, only increase the counter and add the
        // candidate sequence ID to the node.
        father->seqIndex = seqnum;
        father->revcomp = revcomp;
        father->seqOriginalID = auxID;
        father->rptcount += seqrpt;
        lastNode = father - iseq.ptr();
        numSeq++;
        return -1;
    }
    //  Candidate sequence is a repeated sequence, so we increase the count on this
    // node and return the value of the original sequence.
    father->rptcount +=seqrpt;
    lastNode = father - iseq.ptr();
    return (father->seqOriginalID);
}

//  printTree:  
//      It prints the nodes of the tree, and counts the number of sequences in it
//    based on the node container 'iseq'.
//   Input : It does not require any input.
//   Output: A printout of the nodes, number of nodes and sequences to the screen.
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
    //#ifdef _DEBUG
          ::printf( "\n Number of nodes in the Tree = %lld \n", numNodes);
          ::printf( "\n Number of sequences in the Tree = %lld \n", numUnique);
    //#endif
    
}

//  inOrderTree:  
//      It prints the unique sequences of the tree sorted, using recursion
//   Input : It requires the index of the node to be printed (usually the root).
//   Output: A printout of the sorted sequences to the screen.
void BioseqTree::inOrderTree(idx root)
{
    NodeTree *current;
    idx numOffsets = 4;
   
    current = root + iseq.ptr();
    if (current->rptcount != 0){
        // If the node has sequences, print it
        printNode (current->seqIndex);
    }
    for (idx i = 0; i < numOffsets; i++){
        // Look into the offsets
        if (current->offsets[i] != -1){
            inOrderTree (current->offsets[i]);
        }
    }
    return ;
}

//  inOrderTree:  
//      It prints the unique sequences of the tree sorted, using a queue
//   Input : It requires the index of the node to be printed (usually the root).
//   Output: A printout of the sorted sequences to the screen.
void BioseqTree::inOrderTree2(idx root, sVec <idx> * inSort)
{
    NodeTree *current;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;

    stack.mex()->flags|=sMex::fSetZero; 
    stack.cut(0);    // set at the beginning of the buffer

    curr = root;
    stack.vadd(1, curr);
    //if (curr != -1){ 
    len = stack.dim();
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        // Visit the actual node
        current = curr + iseq.ptr();
        if (current->rptcount != 0){
            inSort->vadd(1, current->seqOriginalID);
            //printNode (current->seqIndex);
        }

        for (idx i = numOffsets-1; 0 <= i; i--){
            // Look into the offsets
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    return;
}

//  inOrderTree3:
//      It prints the unique sequences of the tree sorted, using a queue
//   Input : It requires the index of the node to be printed (usually the root).
//   Output: A printout of the sorted sequences to the screen.
void BioseqTree::inOrderTree3(idx root, sVec <idx> * inSort, sVec <idx> * diffSort)
{
    NodeTree *current;
    sVec <idx> stack, queue;
    idx curr, len, offset, offlen;
    idx square = -1;
    idx numOffsets = 4;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);    // set at the beginning of the buffer

    queue.mex()->flags|=sMex::fSetZero;
    queue.cut(0);    // set at the beginning of the buffer

    curr = root;
    stack.vadd(1, curr);
    //if (curr != -1){
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
            // Look into the offsets
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
                offset ++;
            }
        }
        if (offset == 0){  // This node has no offsets
            //empty square;
            diffSort->vadd(1, square);
            offlen = queue.dim();
            if (offlen > 0){
                square = queue[--offlen];
                queue.del(offlen);
            }
        }
        else{ // Node has at least an offset
            // push (offset - 1) numbers into the queue
            for (idx i = 0; i < offset - 1; i++)
                queue.vadd (1, current->seq_end);

            // Visit the actual node
            if (current->rptcount != 0){
                diffSort->vadd(1, square);
                square = current->seq_end;
            }
        }
        len = stack.dim();
    }
    return;
}

//  getSubNode:
//      It gets the node that is based on the sequence
//   Input : It requires:
//             - the index to the candidate sequence
//             - the start position of the candidate sequence
//             - length of the candidate sequence
//   Output: It returns a value of:
//              -1 - if there is no other sequence in the tree
//           or id - the value of the node in which the sequence ends
idx BioseqTree::getSubNode(const char * sequence, idx seqlen, bool isCompressed, bool exactMatch)
{
    NodeTree *father;
    char let1, let2;
    idx pos, bitpos;
    idx flags;
    father = iseq.ptr();

//    const char *seq1 = seq(ofsSeq + seqnum);

    const char *seq2 = 0;

    if (father->seq_end != -1)
        seq2 = seq(ofsSeq + father->seqIndex);

    // Walk for every position of the sequence and look for similarities in the tree
    for(pos = 0; pos<seqlen; ++pos) {

        if (isCompressed){
            let1 = sBioseqAlignment::_seqBits(sequence, pos, 0);
        }
        else {
            let1 = sequence[pos];
        }
        // We check if we have reach the last element of the node.
        if (father->seq_end <= pos){
            // We have reached the last element, so we need to check if
            // there is another node after this node.
            if (father->offsets[(idx)let1] == -1){
                // There is no other node
                return -1;
            }
            // We have not reached the last element of the tree, so we move
            // father to the next position in the tree.
            father = father->offsets[(idx)let1] + iseq.ptr();
            seq2 = seq(ofsSeq + father->seqIndex);
            continue;
        }
        //  We read the next element of the sequence in the tree to compare with candidate
        if (father->revcomp == 0){ bitpos = pos; flags = 0;}
        else { bitpos = (len(ofsSeq + father->seqIndex) - 1) - pos; flags = 0x00000120;}

        let2 = sBioseqAlignment::_seqBits(seq2, bitpos, flags);

        // We check that both elements are different
        if (let1 != let2){
            // We found a difference, so there is no similar sequence in the tree
            return -1;
        }
    }

    if (exactMatch && ( (father->seq_end != pos) || (father->rptcount == 0)) ){
            return -1;
    }
    return father - iseq.ptr();
}

//  getLongestSeq:
//      It gets the longest sequence from the node number it receives
//   Input : It requires:
//             - the number of the node
//   Output: It returns a value of:
//              -1 - if there is no other sequence in the tree
//           or id - the id of the sequence with the maximum count
idx BioseqTree::getLongestSeq(idx *node, idx rptcount, idx tabu)
{
    NodeTree *current, *best;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;
    idx maxCount = 0;
    //idx maxSeqID;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);    // set at the beginning of the buffer

    curr = *node;
    stack.vadd(1, curr);
    //if (curr != -1){
    len = stack.dim();
    best = 0;
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        // Visit the actual node
        current = curr + iseq.ptr();
        if (current->rptcount != 0){
            //inSort->vadd(1, current->seqOriginalID);
            //printNode (current->seqIndex);
            if ((current->rptcount > maxCount) && (tabu != current->seqIndex)){
                maxCount = current->rptcount;
                //maxSeqID = current->seqOriginalID;
                best = current;
            }
        }

        for (idx i = numOffsets-1; 0 <= i; i--){
            // Look into the offsets
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    if (best == 0)
        return -1;
    best->rptcount = rptcount;
    //return maxSeqID;
    *node = best - iseq.ptr();
    return best->seqOriginalID;
}

idx BioseqTree::getNodesSeq (sVec<idx> *seqs, idx node, idx rptcount, idx tabu)
{
    NodeTree *current;
    sVec <idx> stack;
    idx curr, len;
    idx numOffsets = 4;
    //idx maxSeqID;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);    // set at the beginning of the buffer

    curr = node;
    stack.vadd(1, curr);
    //if (curr != -1){
    len = stack.dim();
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        // Visit the actual node
//        current = curr + iseq.ptr();
        current = iseq.ptr(curr);
        if (current->rptcount != 0){
            //inSort->vadd(1, current->seqOriginalID);
            //printNode (current->seqIndex);
            if (tabu != current->seqOriginalID){
                seqs->vadd(1, curr);
            }
        }

        for (idx i = numOffsets-1; 0 <= i; i--){
            // Look into the offsets
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
    stack.cut(0);    // set at the beginning of the buffer
    curr = node;
    stack.vadd(1, curr);
    len = stack.dim();
    current = iseq.ptr(curr);
    while (len != 0){
        curr = stack[--len];
        stack.del(len);
        // Visit the actual node
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
        // Look into the offsets
        for (idx i = numOffsets-1; 0 <= i; i--){
            if (current->offsets[i] != -1){
                stack.vadd(1, current->offsets[i]);
            }
        }
        len = stack.dim();
    }
    return -1;
}

//idx BioseqTree::fixNodeTree(idx node, idx seqnum, bool isRev)
//{
//    NodeTree *current, *next;
//    bool flag = true;
//    idx curr, nxt;
//    idx numOffsets = 4;
//    curr = node;
//
//
//    current = iseq.ptr(curr);
//    while (true){
//        flag = true;
//        for (idx i = 0; i < numOffsets; i++){
//            if (current->offsets[i] != -1){
//                nxt = current->offsets[i];
//                next = iseq.ptr(nxt);
//                if (next->seqIndex == seqnum && next->revcomp == isRev){
//                    flag = false;
//                    curr = nxt;
//                    current = next;
//                    break;
//                }
//            }
//        }
//        if (flag){
//            return curr;
//        }
//    }
//
//    return curr;
//
//}

