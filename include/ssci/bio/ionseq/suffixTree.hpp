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
#ifndef sBio_suffixTree_hpp
#define sBio_suffixTree_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>

namespace slib
{
    class sEdge;
    class sNode{
        private:
            bool isLeaf;
            bool isRoot;
            bool isExplicit;
            idx link;
            idx edge;
            idx _id;
//            sNode (const sNode &);
            const sNode & operator= (const sNode &);

        public:
            sNode(bool root = false){
                isRoot = root;
            }
            sNode(idx edgePos){
                edge = edgePos;
                isLeaf = true;
                isRoot = false;
            }
            sNode (bool r, bool e, bool l){
                isRoot = r;
                isExplicit = e;
                isLeaf = l;
            }
            ~sNode()
            {
            }

//            void init (sEdge *e){
//                isRoot = false;
//                isLeaf = true;
//                edge = e
//            }

            void init (idx edgePos){
                init (false, false, true);
                edge = edgePos;
            }

            void init(bool r, bool e, bool l){
                isRoot = r;
                isExplicit = e;
                isLeaf = l;
            }

            void setid(idx id){
                _id = id;
            }

            bool isroot() const{
                return isRoot;
            }

            bool isleaf() const{
                return isLeaf;
            }

            void setLink(idx l){
                link = l;
            }

            idx getLink(){
                return link;
            }

            void setLeaf(bool val){
                isLeaf = val;
            }
            idx id(){
                return _id;
            }
            void serialize(sStr *out, sDic <idx> *transMap, sVec <sEdge> *map, sVec <idx> *words, sStr *wordContainer, sVec <sNode>*nodeArray);


//        public:
//            idx startNode;
//            idx endNode;
//            idx startStringPos;
//            idx endStringPos;
//
//
//            sStr *stringContainer;
//        public:
//            void init(idx stNode = -1, idx edNode = 0, idx startStrPos = 0, idx endStrPos = 0){
//               startNode = stNode;
//               endNode = edNode;
//               startStringPos = startStrPos;
//               endStringPos = endStrPos;
//            }
//
//            // New edge is created, node is not assigned
//            TreeEdge(){
//                init (-1, 0, 0, 0);
//            }
//            TreeEdge(idx start, idx end, idx first, idx last){
//                init (start, end, first, last);
//            }
//
//            // Destructor
//            ~TreeEdge(){
//
//            }

    };

    class sEdge
    {
        private:
            idx _fromNode;
            idx _toNode;
            idx _startIndex;
            idx _endIndex;
            idx _wordIndex;

            sEdge(const sEdge &);
            const sEdge& operator=(const sEdge &);
        public:
            sEdge(){
                init(-1, -1, -1, -1, -1);
            }
            sEdge (sNode *from, idx start, idx end, idx wIndex){
                init(from->id(), -1, start,end, wIndex);
                from->setLeaf(false);
            }
            sEdge(sNode *from, sNode *to, idx start, idx end){
                init(from->id(), to->id(), start, end, 1);
                _fromNode = from->id();
            }
            ~sEdge(){
            }

            void init (sNode *from, sNode *to, idx start, idx end, idx wIndex){
                init(from->id(), to->id(), start, end, wIndex);
                from->setLeaf(false);
            }

            void init (sNode *from, idx start, idx end, idx wIndex){
                init(from->id(), -1, start, end, wIndex);
                from->setLeaf(false);
            }

            void init (idx from, idx to, idx start, idx end, idx wIndex){
                _fromNode = from;
                _toNode = to;
                _startIndex = start;
                _endIndex = end;
                _wordIndex = wIndex;
            }

            void to(sNode *to){
                _toNode = to->id();
            }

            void to(idx to){
                _toNode = to;
            }

            idx from(){
                return _fromNode;
            }

            idx to (){
                return _toNode;
            }
            idx wordIndex(){
                return _wordIndex;
            }

            idx startIndex(){
                return _startIndex;
            }

            idx endIndex(){
                return _endIndex;
            }

    };
//    class LastPosTree
//    {
//        public:
//            idx rootNode;   // Root of suffix tree
//            idx startIndex; // start of the string
//            idx endIndex;   // end of the string
//
//            LastPosTree(){
//                rootNode=0;
//                startIndex = -1;
//                endIndex = -1;
//            }
//            LastPosTree (idx root, idx start, idx end){
//                rootNode = root;
//                startIndex = start;
//                endIndex = end;
//            }
//
//            bool endTree(){
//                return startIndex > endIndex;
//            }
//
//            bool endSubTree(){
//                return endIndex >= startIndex;
//            }
//
//            void print(sStr *inputString){
//                ::printf("rootNode:%" DEC " startIndex:%" DEC " %c", rootNode, startIndex, inputString->ptr(startIndex)[0]);
//            }
//    };

    class SuffixTree
    {
            struct nodeVal
            {
                    idx node;
                    idx val;
            };
        private:

            idx rootNode;
            idx expNode;
            sVec <sNode> nodeArray; // Nodes container
            sVec <sEdge> edgeArray; // Container of sEdges
            sDic<idx> edgeList; // dictionary of edgeArray info
            idx countNodes;
            sStr wordsContainer;
            sVec <idx> words;

            SuffixTree(const SuffixTree&);
            const SuffixTree& operator=(const SuffixTree&);

        public:
            SuffixTree (){
                // Add a new node
                countNodes = 0;
                edgeArray.cut(0);
                nodeArray.cut(0);
                wordsContainer.cut(0);
                words.cut(0);
                sNode *node = nodeArray.add(1);
                node->init(true, false, false);
                node->setid(nodeArray.dim() - 1);
                rootNode = node->id();

                sNode *exp = nodeArray.add(1);
                exp->init(false, true, false);
                exp->setid(nodeArray.dim() - 1);
                node->setLink(exp->id());
                expNode = exp->id();
            }

            ~SuffixTree(){

            }

            void printInfo(sStr *out){
                static sStr o;
                if (!out){
                    o.cut(0);
                    out = &o;
                }
                idx count = 0;
                out->addString("key\tStartNode\tEndNode\t\tSuffixLink\t\tFirstIndex\tLastIndex\tString\n");
                idx idlen;
                for (idx i = 0; i < edgeList.dim(); ++i){
                    idx *val = edgeList.ptr(i); // iterate over all values
                    if (val && *val >= 0){
                        sEdge *e = edgeArray.ptr(*val);
                        idx *ival = static_cast<idx *>(edgeList.id(i, &idlen));
                        const char *str = getLetterFromWord(e->wordIndex(), 0);

                        out->printf("%" DEC "\t%" DEC "\t\t%" DEC, *ival, e->from(), e->to());
                        out->printf("\t\t%" DEC "\t\t\t%" DEC "\t\t%" DEC "\t\t", nodeArray[e->from()].getLink(), e->startIndex(), e->endIndex());
                        ++count;
                        out->add(str + e->startIndex(), e->endIndex() - e->startIndex() + 1);
                        out->addString("\n", 1);
                    }
                }
                out->printf("Total edges: %" DEC "\n", count);
                out->addString("nodeID\tisRoot\tisLeaf\tlink\n");
                for (idx i = 0; i < nodeArray.dim(); ++i){
                    sNode *node = nodeArray.ptr(i);
                    out->printf("%" DEC "\t%s\t%s\t%" DEC, node->id(), node->isroot() ? "true" : "false", node->isleaf() ? "true" : "false", node->getLink());
                    out->addString("\n", 1);
                }
                out->printf("Total nodes: %" DEC "\n", nodeArray.dim());
                if(o.length()){
                    ::printf("%s", o.ptr());
                }
            }
            bool insert(const char *str, idx len = 0);
            idx update (sNode *, idx, idx, idx);
            idx canonize (sNode *s, idx k, idx p, idx iword, idx *toReturn);
            idx split(sNode *, idx, idx, char, idx);
            void serialize(sStr *out);
            idx testAndSplit(sNode *s, idx k, idx p, char t, idx iword, bool *toReturn);
            idx update (sNode *s, idx k, idx i, idx iword, idx *toReturn);

            static idx numFromLetter (char ch)
            {
                if (ch >= 97 && ch <= 122){
                    ch -= 32;
                }
                return ch - 65 + 1;
            }

            static idx getHashKey(idx num, idx character)
            {
//                idx hashkey = (node + ((character) << 59)); // very simple hashkey
                idx hashkey = num * 100 + character;
                return hashkey;
            }

            static idx getHashKey(idx num, char character)
            {
//                idx hashkey = (node + ((character) << 59)); // very simple hashkey
                idx hashkey = num * 100 + numFromLetter(character);
                return hashkey;
            }

            sNode *getRootNode (){
                return nodeArray.ptr(rootNode);
            }

            sNode *getExpNode (){
                return nodeArray.ptr(expNode);
            }

            sNode *getXNode(idx num){
                if (num < nodeArray.dim()){
                    return nodeArray.ptr(num);
                }
                return 0;
            }

            sEdge *setNewEdge(sNode *exp, sNode*root, idx start, idx end, idx hkey, idx iword){
                sEdge *ne = edgeArray.add(1);
                ne->init(exp, root, start, end, iword);
                // register the key in dictionary
                registerKey(hkey);
                return ne;
            }

            bool addNewWord(const char *wo, idx lenword){
                words.vadd(1, wordsContainer.length());
                wordsContainer.add(wo, lenword);
                wordsContainer.add0(1);
                return true;
            }

            const char * getLetterFromWord (idx iword, idx pos = 0)
            {
                idx iwrd = words[iword];
                const char *charPos = wordsContainer.ptr(iwrd);
                return charPos + pos;
            }

            sEdge *findEdge (idx id, idx let){
                idx hkey = getHashKey(id, let);
                idx retkey = getKey(hkey);
                if (retkey == -1){
                    return 0;
                }
                sEdge *e = edgeArray.ptr(retkey);
                return e;
            }

            sEdge *findEdge (idx id, char let){
                idx hkey = getHashKey(id, let);
                idx retkey = getKey(hkey);
                if (retkey == -1){
                    return 0;
                }
                sEdge *e = edgeArray.ptr(retkey);
                return e;
            }

            bool registerKey (idx id, idx val = -1)
            {
                if (val == -1){
                    val = edgeArray.dim() - 1;
                }
                *edgeList.set(&id, sizeof(idx)) = val;
                return true;
            }

            idx getKey (idx key)
            {
                idx *myedge = edgeList.get(&key, sizeof(idx));
                return (myedge ? *myedge : -1);
            }

            sNode *addNewNode (sEdge *e){
                sNode *r = nodeArray.add(1);
                r->init(e - edgeArray.ptr(0));
                r->setid(nodeArray.dim() - 1);
                return r;
            }

            idx walkTree(const char *word, idx nodeId, idx pntr, idx *toReturn = 0);
            idx longSubstring(idx nodeId, idx cnt = 0);
            idx longestString(idx nodeId, idx *iword, idx *start, idx *end);

            idx nodeStack (sVec<nodeVal> *stack, idx nodeId, idx val = 0){
                sNode *node = getXNode(nodeId);
                if (node->isleaf() || node->isroot()){
                    stack->vadd(1, nodeId);
                    return 1;
                }
                return addNodestoStack(stack, nodeId, val);
            }

            idx addNodestoStack(sVec<nodeVal> * stack, idx nodeId, idx val){
                char zeroChar = 0;
                idx prevCnt = stack->dim();
                for (idx i = 0; i < 128; ++i){
                    // For all the characters
                    if (i >= 97 && i <= 122){
                        continue;
                    }
                    char character = zeroChar + i;
                    sEdge *e = findEdge (nodeId, character);
                    if (!e){
                        continue;
                    }
                    nodeVal *newnode = stack->add(1);
                    newnode->node = e - edgeArray.ptr(0);
                    newnode->val = val;
                }
                return stack->dim() - prevCnt;
            }

//            sVec <TreeEdge> edgeArray;
//            sDic <idx> edgeList;
//            idx lastEdge;
//            sStr inputString;
//            idx inputLength;
//            idx numNodes;
////            LastPosTree *lastPos;
//
//            void insertEdge (TreeEdge *edge){
//                idx key = getHashKey(edge->startNode, inputString[edge->startStringPos]);
//                idx edgepos = edge - edgeArray.ptr();
//                idx * myedge = edgeList.get(&key, 1);
//                if (myedge){
//                    *myedge = edgepos;
//                }
//                else {
//                    *edgeList.set(&key, 1) = edgepos;
//                }
//            }
//
//            void removeEdge (TreeEdge *edge){
//                idx key = getHashKey(edge->startNode, inputString[edge->startStringPos]);
//
//                idx * myedge = edgeList.get(&key, 1);
//                if (myedge){
//                    *myedge = -1;
//                }
//            }
//            void movetoClosestParent(LastPosTree *lastPos);
//            idx splitEdge(LastPosTree &e, idx edgepos);
//
//
//        public:
//            SuffixTree(const char *s, idx slen = 0){
//                numNodes = 0;
//                inputString.printf(0,"%s", s);
//
//                if(slen){
//                    inputLength = slen;
//                }
//                else {
//                    inputLength = inputString.length();
//                }
//                nodeArray.resize(inputLength);
//            }
//
//            void createTree(LastPosTree &tree, idx lastIndex);
//
//            void insert(const char *str, idx len = 0);
//            void printAllEdges(sStr *out);
//            bool search(const char *searchString, idx stringlen);
//
//            idx getHashKey (idx node, idx character){
//                idx hashkey = (node + ((character) << 59)); // very simple hashkey
//                return hashkey;
//            }
//
//            idx findLocalEdge (sDic <idx> *dic, idx node, char asciiChar){
//                idx key = getHashKey(node, asciiChar);
//                idx * myedge = dic->get(&key, 1);
//                if (myedge){
//                    return *myedge;
//                }
//                return -1;
//            }
//
//            TreeEdge * findEdge(idx startNode, char character){
//                idx edgePos = findLocalEdge (&edgeList, startNode, character);
//                TreeEdge *e = 0;
//                if (edgePos < 0){
//                    return 0;
////                    e = edgeArray.add(1);
////                    e->init(-1, edgeArray.dim(), -1, -1);
//                }
//                else {
//                    e = edgeArray.ptr(edgePos);
//                }
//                return e;
//            }

    };

}
#endif // sMath_suffixTree_hpp
