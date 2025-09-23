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
#pragma once
#ifndef sBio_BioseqTree_hpp
#define sBio_BioseqTree_hpp

#include <slib/core/vec.hpp>
#include <ssci/bio/bioseqalign.hpp>

using namespace slib;

class BioseqTree
{

    protected:
        struct NodeTree
        {

                idx offsets[4];
                idx seqOriginalID;
                idx seqIndex;
                idx rptcount;
                idx seq_end;
                bool revcomp;

        };
        sVec < NodeTree > iseq;

        idx ofsSeq;
        idx numSeq;
        idx lastNode;
        typedef char * (*sequenceProviderFunc)(idx ofs);

    public:
        sequenceProviderFunc seqFunc;
        sBioseq * bioseqPtr;
        sMex * mexPtr;
        char * basicMemPtr;

    public:

        void createNode(NodeTree *node, idx end, idx index, idx seq, bool rev)
        {
            node->seqOriginalID = seq;
            node->seqIndex = index;
            node->seq_end = end;
            node->offsets[0] = -1, node->offsets[1] = -1, node->offsets[2] = -1, node->offsets[3] = -1;
            node->rptcount = 0;
            node->revcomp = rev;
        }
        ;

        static void printNode(NodeTree *node, idx pos)
        {
            ::printf("\n \t pos = %lld ", pos);
            ::printf("\n \t OrigId = %lld, seqId = %lld, rptcount = %lld ", node->seqOriginalID, node->seqIndex, node->rptcount);
            ::printf("\n \t\t offsets = %lld %lld %lld %lld", node->offsets[0], node->offsets[1], node->offsets[2], node->offsets[3]);
            ::printf("\n \t\t end = %lld  rev = %lld\n", node->seq_end, (idx)node->revcomp);
        }

        idx getSeqOriginalID (idx nodepos)
        {
            NodeTree *node = iseq.ptr(nodepos);
            return node->seqOriginalID;
        }

        idx getSeqIndex (idx nodepos)
        {
            NodeTree *node = iseq.ptr(nodepos);
            return node->seqIndex;
        }

        idx getCount (idx nodepos)
        {
            NodeTree *node = iseq.ptr(nodepos);
            return (node->rptcount);
        }

        bool getRevComp (idx nodepos)
        {
            NodeTree *node = iseq.ptr(nodepos);
            return (node->revcomp);
        }

        idx getLastNode ()
        {
            return lastNode;
        }

        void setCount (idx nodepos, idx newcount)
        {
            NodeTree *node = iseq.ptr(nodepos);
            node->rptcount = newcount;
        }

        static void printlet(sStr *out, idx ACGT)
        {

            if( ACGT == 0 )
                out->printf("A");
            else if( ACGT == 1 )
                out->printf("C");
            else if( ACGT == 2 )
                out->printf("G");
            else
                out->printf("T");

            return;
        }

        BioseqTree()
        {
        }
        void init(idx ofs)
        {
            iseq.mex()->flags |= sMex::fSetZero;
            iseq.cut(0);
            NodeTree *rot = iseq.add(1);
            createNode(rot, -1, 0, 0, false);
            numSeq = 0;
            ofsSeq = ofs;
            seqFunc = 0;
            bioseqPtr = 0;
            basicMemPtr = 0;
            mexPtr = 0;
            lastNode = -1;
        }

        BioseqTree(sMex *ptr, idx ofs)
        {
            this->init(ofs);
            this->mexPtr = ptr;
        }

        BioseqTree(sBioseq *ptr, idx ofs)
        {
            this->init(ofs);
            this->bioseqPtr = ptr;
        }

        BioseqTree(char *ptr, idx ofs)
        {
            this->init(ofs);
            this->basicMemPtr = ptr;
        }

        idx addSequence(idx seqnum, idx seqlen, idx seqrpt = 1, idx reversecomplement = 0, idx auxID = -1);

        void printTree(bool generalInfo = false);

        static void printNode(idx pos)
        {
            ::printf("\n Unique sequence pos = %lld \n", pos);
        }

        void inOrderTree(idx root);
        void inOrderTree2(idx root, sVec<idx> * inSort);
        void inOrderTree3(idx root, sVec<idx> * inSort, sVec<idx> * diffSort);
        idx getSubNode(const char *sequence, idx seqlen, bool isCompressed = false, bool exactMatch = false);
        idx getLongestSeq(idx *node, idx rptcount, idx tabu);
        idx getNodesSeq (sVec<idx> *seqs, idx node, idx rptcount, idx tabu);
        idx fixNodeTree(idx node, idx seqnum, idx seqlen, bool isRev);

        const char * seq(idx ofs)
        {
            if( seqFunc )
                return seqFunc(ofs);
            if( bioseqPtr )
                return (char *) bioseqPtr->seq(ofs);
            else if( mexPtr )
                return (char*) mexPtr->ptr(ofs);
            else if( basicMemPtr )
                return basicMemPtr + ofs;
            else
                return "we are doomed";
        }

        idx len (idx ofs)
        {
            if (seqFunc)
                return 0;
            if (bioseqPtr)
                return (idx) bioseqPtr->len(ofs);
            else if (mexPtr)
                return 0;
            else if (basicMemPtr)
                return 0;
            else
                return 0;
        }

};

#endif 