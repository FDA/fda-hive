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
            sVec <sNode> nodeArray;
            sVec <sEdge> edgeArray;
            sDic<idx> edgeList;
            idx countNodes;
            sStr wordsContainer;
            sVec <idx> words;

            SuffixTree(const SuffixTree&);
            const SuffixTree& operator=(const SuffixTree&);

        public:
            SuffixTree (){
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
                    idx *val = edgeList.ptr(i);
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
                idx hashkey = num * 100 + character;
                return hashkey;
            }

            static idx getHashKey(idx num, char character)
            {
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


    };

}
#endif 