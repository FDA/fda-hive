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
#include <ssci/bio/ionseq/suffixTree.hpp>

using namespace slib;

void printRange (sStr *out, const char *str, idx start, idx end)
{
    out->add(str+start, end - start);
}

void sNode::serialize(sStr *out, sDic <idx> *transMap, sVec <sEdge> *map, sVec <idx> *words, sStr *wordContainer, sVec <sNode>*nodeArray)
{
    out->add("(", 1);
//    idx idlen;
    idx id;
    for (idx i = 1; i <= 26; ++i){
        id = SuffixTree::getHashKey(_id, i);
//        idx * key = (idx *) (transMap->id(id, &idlen));
        idx *key = transMap->get(&id, sizeof(idx));
        if (!key){ // the last one
            continue;
        }
        sEdge *e = map->ptr(*key);
        out->add("{", 1);
        idx iword = *words->ptr(e->wordIndex());
        const char *str = wordContainer->ptr(iword);
        out->add(str+e->startIndex(), e->endIndex()-e->startIndex());
        out->add("}", 1);
        idx next = e->to();
        nodeArray->ptr(next)->serialize(out, transMap, map, words, wordContainer, nodeArray);
    }
    out->addString(")", 1);
}

idx SuffixTree::canonize(sNode *s, idx k, idx p, idx iword, idx *toReturn)
{
    if (p < k){
        *toReturn = k;
        return s->id();
    }
    const char *str = getLetterFromWord(iword);
    sEdge *leavEdge = findEdge(s->id(), str[k]);
    while (leavEdge && ((leavEdge->endIndex() - leavEdge->startIndex()) <= p - k)){
        k += leavEdge->endIndex() - leavEdge->startIndex() + 1;
        idx stmp = leavEdge->to();
        s = getXNode(stmp);
        if ( k <= p){
            leavEdge = findEdge(s->id(), str[k]);
        }
    }
    *toReturn = k;
    return s->id();
}

idx SuffixTree::testAndSplit(sNode *s, idx k, idx p, char t, idx iword, bool *toReturn)
{
    if (k <= p){
        const char *str = getLetterFromWord(iword);
        idx hkey = getHashKey(s->id(), str[k]);
        sEdge *leavEdge = findEdge(s->id(), str[k]);
        idx leavPos = leavEdge-edgeArray.ptr();
        if (leavEdge->wordIndex() != iword){
            str = getLetterFromWord(leavEdge->wordIndex());
        }
        if (leavEdge && (t == str[leavEdge->startIndex() + p - k + 1])){
            *toReturn = true;
            return s->id();
        }
        else {
            // Create a new edge
            sEdge *newEdge = edgeArray.add(1);
            leavEdge = edgeArray.ptr(leavPos);
            newEdge->init(s, leavEdge->startIndex(), leavEdge->startIndex() + p - k, leavEdge->wordIndex());
            // and a new node
            sNode *r = addNewNode(newEdge);
            newEdge->to(r);
            // register the key in dictionary
            registerKey(hkey);

            // Create another edge
            newEdge = edgeArray.add(1);
            leavEdge = edgeArray.ptr(leavPos);
            newEdge->init(r, leavEdge->startIndex() + p - k + 1, leavEdge->endIndex(), leavEdge->wordIndex());
            newEdge->to(leavEdge->to());
            idx hashkey = getHashKey(r->id(), str[leavEdge->startIndex() + p - k + 1]);
            registerKey(hashkey);
            *toReturn = false;
            return r->id();
        }
    }
    idx hkey = getHashKey(s->id(), t);
    idx key = getKey(hkey);
    *toReturn = (key == -1) ? false : true;
    return s->id();
}

idx SuffixTree::update (sNode *s, idx k, idx i, idx iword, idx *toReturn)
{
    idx oldrPos = rootNode;
    const char *str = getLetterFromWord(iword, 0);
    idx lenstr = sLen (str);

    bool retTestResult;
    idx sPos = testAndSplit (s, k, i-1, str[i], iword, &retTestResult);
    sNode *testResult = getXNode(sPos);
    idx retVal;

    while (!retTestResult){
        sEdge *leavEdge = edgeArray.add(1);
        leavEdge->init(testResult->id(), -1, i, lenstr-1, iword);
        // Add a node
        sNode *node = addNewNode(leavEdge);
        leavEdge->to(node);

        idx hkey = getHashKey(testResult->id(), str[i]);
        registerKey(hkey);

        if (oldrPos != rootNode ){
            sNode *oldr = getXNode(oldrPos);
            oldr->setLink(testResult->id());
            oldr->setLeaf(false);
        }
        oldrPos = testResult->id();
        idx stmp = canonize (getXNode(s->getLink()), k, i-1, iword, &retVal);
        s = getXNode(stmp);
        k = retVal;
        sPos = testAndSplit(s, k, i - 1, str[i], iword, &retTestResult);
        testResult = getXNode(sPos);
    }
    if (oldrPos != rootNode){
        sNode *oldr = getXNode(oldrPos);
        oldr->setLink(s->id());
        oldr->setLeaf(false);
    }
    *toReturn = k;
    return s->id();

}

bool SuffixTree::insert(const char *str, idx len)
{
    if (!len){
        len = sLen (str);
    }

    addNewWord(str, len);

    idx iword = words.dim() - 1;

    for (idx iter = 0; iter < len; ++iter){
        idx key = getHashKey(getExpNode()->id(), str[iter]);
        idx myedge = getKey(key);
        if (myedge != -1){
            continue;
        }
        setNewEdge(getExpNode(), getRootNode(), iter, iter, key, iword);
    }

    sNode *s = getRootNode();
    idx k = 0;
    idx startiWord = 0;
//    idx tmp = 0;
//    idx stWord = 0;
    if (iword > 0){
        idx tmp = walkTree(str, 0, 0, &startiWord);
        s = getXNode(tmp);
        k = startiWord;
    }
    sStr out;
    out.cut(0);
    printInfo(&out);
    ::printf("\niter: 0");
    ::printf("\n%s", out.ptr());
    for (idx iter = startiWord; iter < len; ++iter){
        idx upNum, canNum;
        idx tmp = update (s, k, iter, iword, &upNum);
        sNode *updateRes = getXNode(tmp);
        tmp = canonize(updateRes, upNum, iter, iword, &canNum);
        s = getXNode(tmp);
        k = canNum;
        out.cut(0);
        printInfo(&out);
        ::printf("\niter: %"DEC, iter+1);
        ::printf("\n%s", out.ptr());
    }

    return true;

}

void SuffixTree::serialize(sStr *out)
{
    sNode *root = getRootNode();

    root->serialize(out, &edgeList, &edgeArray, &words, &wordsContainer, &nodeArray);
    out->add("\n");

}

//sNode * sNode::walkTree(const char *word, idx wordlen, idx pntr, sDic <idx> *transMap, sVec <idx> *words, idx *toReturn)
idx SuffixTree::walkTree(const char *word, idx nodeId, idx pntr, idx *toReturn)
{
    sNode *node = getXNode(nodeId);
    sEdge *e = findEdge(node->id(), word[pntr]);
    if (!e){
        *toReturn = pntr;
        return node->id();
    }
    const char *tmp = getLetterFromWord(e->wordIndex(), 0);
    idx j = pntr;
    bool flag = false;
    idx cnt = 0;
    for (idx i = e->startIndex(); i<=e->endIndex(); ++i, ++cnt){
        if (tmp[i] != word[j]){
            flag = true;
            break;
        }
        ++j;
    }
    if (flag){
        *toReturn = j;
        if (j > e->endIndex()){
            return e->to();
        }
        return node->id();
    }

    return walkTree (word, e->to(), pntr + cnt, toReturn);

}

idx SuffixTree::longSubstring(idx nodeId, idx cnt)
{
    if (!cnt){
        cnt = words.dim();
    }
    sNode *node = getXNode(nodeId);
    char initChar = 0;
    idx currNode = 0;
    for (idx i = 0; i < sizeof(char); ++i){
        // For all the characters
        char character = initChar + i;
        sEdge *e = findEdge (nodeId, character);
        if (!e){
            continue;
        }
        const char *tmp = getLetterFromWord(e->wordIndex(), 0);
        // Move to the end of the edge
        currNode = e->to();



    }
}

idx SuffixTree::longestString(idx nodeId, idx *iword, idx *start, idx *end)
{
    char initChar = 0;
    idx curNode = nodeId;
    sVec <nodeVal> stack;
    idx maxCnt = 0;
    idx currentCnt = 0;

    stack.mex()->flags|=sMex::fSetZero;
    stack.cut(0);    // set at the beginning of the buffer

    //if (curr != -1){
//    nodeStack(&stack, curNode);
    sNode *node = getXNode(nodeId);
    if (node->isleaf()){
        stack.vadd(1, nodeId);
    }
    else {
        addNodestoStack(&stack, nodeId, 0);
    }
    idx len = stack.dim();
    idx edgePos;
    idx nodelen;
    idx bestEdge;
    while (len != 0){
        nodeVal *curStack = stack.ptr(--len);
        edgePos = curStack->node;
        sEdge *e = edgeArray.ptr(edgePos);
        idx nodelen = e->endIndex() - e->startIndex() + 1;
        currentCnt = (nodelen + curStack->val);
        stack.del(len);

        if (currentCnt >= maxCnt){
            bestEdge = edgePos;
            maxCnt = currentCnt;
        }
        node = getXNode(e->to());
        if (!node->isleaf()){
            addNodestoStack(&stack, e->to(), currentCnt);
        }
        len = stack.dim();
    }

    sEdge *e = edgeArray.ptr(bestEdge);
    *iword = e->wordIndex();
    *start = e->endIndex() - maxCnt + 1;
    *end = e->endIndex();
    return 1;
}
//    idx hkey = SuffixTree::getHashKey(_id, word[pntr]);
//    idx *key = transMap->get(&hkey, 1);
//    if (!key){
//        *toReturn = pntr;
//        return _id;
//    }
//    sEdge *e = map->ptr(*key);
//
//    idx windx = e->wordIndex();
//    idx iwrd = *words->ptr(windx);
//    const char *str = wordContainer->ptr(iwrd);
//
//    idx j = pntr;
//    bool flag = false;
//    for(idx i = e->startIndex(); i <=e->endIndex(); ++i ){
//        if (str[i] != word[j]){
//            flag = true;
//            break;
//        }
//        ++j;
//    }
//    if (flag){
//        *toReturn = j;
//        if (j > pntr) {
//            return e->to();
//        }
//        return _id;
//    }
//    idx eto = e->to();
//    sNode *node = nodeArray->ptr(eto);
//    return node->walkTree(word, wordlen, pntr+sLen(str), transMap, map, nodeArray, words, wordContainer, toReturn);
//}

/*
void SuffixTree::printAllEdges(sStr *out){
    idx count = 0;

    out->addString("StartNode\tEndNode\t\tSuffixLink\t\tFirstIndex\tLastIndex\tString\n");
    idx idlen;
    for (idx i = 0; i < edgeList.dim(); ++i){
//        const char * key = static_cast<const char *>(edgeList.id(i, &idlen)); // iterate over all keys - you must know the key type!
        idx *val = edgeList.ptr(i); // iterate over all values
        if (val && *val >= 0){
            TreeEdge *e = edgeArray.ptr(*val);

            out->printf("%"DEC"\t\t%"DEC, e->startNode, e->endNode);
            out->printf("\t\t%"DEC"\t%"DEC"\t\t%"DEC"\t\t", nodeArray[e->endNode], e->startStringPos, e->endStringPos);
            ++count;
            idx head = (inputLength > e->endStringPos) ? e->endStringPos : inputLength;
            out->add(inputString.ptr(e->startStringPos), head+1);
            out->addString("\n", 1);
        }
    }
    out->printf("Total edges: %"DEC, count);
}

void SuffixTree::movetoClosestParent(LastPosTree *lastPos){

    if (lastPos->endTree()){
        // We have reached the end
    }
    else {
        TreeEdge *e = findEdge(lastPos->rootNode, inputString.ptr(lastPos->startIndex)[0]);

        if (e->startNode == -1){
            lastPos->print(&inputString);
//            ::printf("rootNode:%"DEC" startIndex:%"DEC" %c", lastPos->rootNode, lastPos->startIndex, inputString[lastPos->startIndex]);
        }

        idx labelLength = e->endStringPos - e->startStringPos;

        while (labelLength <= (lastPos->endIndex - lastPos->startIndex)){
            lastPos->startIndex = labelLength + 1;
            lastPos->rootNode = e->endNode;
            if (lastPos->startIndex <= lastPos->endIndex){
                e = findEdge(e->endNode, inputString.ptr(lastPos->startIndex)[0]);
                if (e->startNode == -1){
                    lastPos->print(&inputString);
                }
            }
            labelLength = e->endStringPos - e->startStringPos;
        }
    }
}

idx SuffixTree::splitEdge(LastPosTree &s, idx edgepos)
{
    // Remove the edge
    TreeEdge *newEdge = edgeArray.add(1);
    TreeEdge *e = edgeArray.ptr(edgepos);

    newEdge->init(s.rootNode, edgeArray.dim(), e->startStringPos, e->startStringPos+s.endIndex - s.startIndex);
    ++numNodes;

    insertEdge(newEdge);
    nodeArray[newEdge->endNode] = s.rootNode;
    e->startStringPos += s.endIndex - s.startIndex + 1;
    e->startNode = newEdge->endNode;
    insertEdge(e);

    return newEdge->endNode;

}

void SuffixTree::createTree(LastPosTree &tree, idx lastIndex)
{
    idx parentNode;

    idx prevParentNode = -1;
//    idx edgePos;
    while (true){
//        TreeEdge e;
        parentNode = tree.rootNode;
//        char inputString[lastIndex];

        if (tree.endTree()){
            TreeEdge *e = findEdge(tree.rootNode, inputString.ptr(lastIndex)[0]);

            if (e && e->startNode != -1){
                break;
            }
        }
        else {
            TreeEdge *e = findEdge(tree.rootNode, inputString.ptr(tree.startIndex)[0]);

            idx diff = tree.endIndex - tree.startIndex;
            if (inputString.ptr(e->startStringPos + diff + 1) == inputString.ptr(lastIndex)){
                // match
                break;
            }

            parentNode = splitEdge(tree, e-edgeArray.ptr());
        }

        TreeEdge *newEdge = edgeArray.add(1);
        newEdge->init(parentNode, edgeArray.dim(), lastIndex, inputLength);
        ++numNodes;
        insertEdge(newEdge);

        if (prevParentNode > 0){
            nodeArray[prevParentNode] = parentNode;
        }
        prevParentNode = parentNode;

        // Move to next position
        if (tree.rootNode == 0){
            ++tree.startIndex;
        }
        else {
            tree.rootNode = nodeArray[tree.rootNode];
        }
        movetoClosestParent(&tree);
    }

    nodeArray[prevParentNode] = parentNode;
    ++tree.endIndex;
    movetoClosestParent(&tree);
}

bool SuffixTree::search(const char *searchString, idx stringlen)
{

    TreeEdge *e  = findEdge(0, searchString[0]);

    idx iter = 0;
    idx i = -1;

    if (e->startNode != -1){
        while (i < stringlen){
            ::printf("Search:\tEdge:%"DEC" %"DEC" : %c %c I: %"DEC"\n", e->startNode, e->endNode, inputString.ptr(e->startStringPos)[0], inputString.ptr(e->endStringPos)[0], i);

            iter = 0;

            while (e->endStringPos >= e->startStringPos + iter){
                ::printf("Search:\tmatching %c %c at index: %"DEC"\n", inputString.ptr(e->startStringPos + iter)[0], searchString[i+iter+1], e->startStringPos+iter);

                if (inputString.ptr(e->startStringPos + iter)[0] == searchString[i+iter+1]){
                    ++iter;
                    if (i + iter + 1 >= stringlen){
                        ::printf("Search:\tWe have a match ending at %"DEC"\n", e->startStringPos + iter - 1);
                        return true;
                    }
                }
                else {
                    ::printf("Search:\tMatch not found, matched only up to index: %"DEC"\n", i + iter);
                    return false;
                }
            }

            iter = (e->endStringPos - e->startStringPos + 1);

            e = findEdge(e->endNode, searchString[i+iter+1]);

            if (e->startNode == -1){
                ::printf("Search:\tMatch not found, matched only up to: %"DEC" %c\n", i + iter, searchString[i+iter+1]);
                return false;
            }

            i += iter;
        }
    }

    ::printf("Search:\tMatched %"DEC" %s", iter, searchString);
    return true;

}
*/
