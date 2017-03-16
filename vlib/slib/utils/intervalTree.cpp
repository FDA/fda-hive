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

#include <slib/utils/intervalTree.hpp>

using namespace slib;

//idx sIntervalTree::deBrLUT[64]={
//    0,  1,  2, 53,  3,  7, 54, 27,
//    4, 38, 41,  8, 34, 55, 48, 28,
//   62,  5, 39, 46, 44, 42, 22,  9,
//   24, 35, 59, 56, 49, 18, 29, 11,
//   63, 52,  6, 26, 37, 40, 33, 47,
//   61, 45, 43, 21, 23, 58, 17, 10,
//   51, 25, 36, 32, 60, 20, 57, 16,
//   50, 31, 19, 15, 30, 14, 13, 12,
//};
//
//idx sIntervalTree::getLSBindex1(idx & a){
//    return deBrLUT[ (a&-a)*deBrSeq ];
//}
//
//idx sIntervalTree::getRoot(idx depth){
//    idx root=0;
//    return root|(1<<depth);
//}
//
//idx sIntervalTree::getParent(idx x, idx * level){
//    if(!level){
//        *level=getLSBindex1(x);
//    }
//    x&=~(3<<*level);
//    x|=2<<*level;
//    return x;
//}
//
//idx sIntervalTree::getLeft(idx x, idx * level){
//    if(!level){
//        *level=getLSBindex1(x);
//    }
//    if(*level<=1)return 0;
//    x&=~(1<<(*level-1));
//    x|=(1<<(*level-2));
//    return x;
//}
//
//idx sIntervalTree::getRight(idx x,idx * level){
//    if(!level){
//        *level=getLSBindex1(x);
//    }
//    if(*level<=1)return 0;
//    x|=(1<<(*level-2));
//    return x;
//}
//
//idx sIntervalTree::searchInVirtualTree(idx * relPtr,idx bodysize,sVec<startEndNode> &results,idx start,idx end, sVec<idx> * rangeIndexes)
//{
//    if(start<0)start=0;
//    if(!end || end<start)end=start;
//    idx depth=floor(log10(bodysize)/log10(2)),rangeBodySize=0;
////                idx virualSize=pow(2,depth);
//    vtreeNode curr;curr.ind=getRoot(depth);curr.level=depth+1;curr.range=0;
//    if(rangeIndexes){
//        rangeIndexes->cut(0);
//    }
//    sVec<vtreeNode> stack;
//    stack.vadd(1,curr);
//    while(stack.dim()){
//        curr=*stack.ptr(stack.dim()-1);
//        stack.cut(stack.dim()-1);
//
//        //check if we are out of bounds of the real tree
//        if(curr.ind>bodysize){
//            if(curr.level>0){
//                vtreeNode next;
//                next.ind=getLeft(curr.ind,&curr.level);
//                next.level=curr.level-1;
//                stack.vadd(1,next);
//            }
//            continue;
//        }
//        if(!curr.range) curr.range = 0; // ( startEnd*)DB.Getbody(range_TYPE,relPtr[curr.ind-1]?relPtr[curr.ind-1]:1,&rangeBodySize);
//
//        idx subRangesCnt= (rangeBodySize/ sizeof(startEnd));
//        if(sOverlap(start,end,curr.range[0].start,curr.range[subRangesCnt-1].end)){
//            startEndNode node;
//            node.ranges=curr.range;
//            node.index=relPtr[curr.ind-1];
//            node.subRangesCnt= subRangesCnt;
//            results.vadd(1,node);
//
//        }
//
//        if(curr.level>1){
//            vtreeNode nextL;
//            nextL.ind=getLeft(curr.ind,&curr.level);
//            nextL.level=curr.level-1;
//            if(nextL.ind<bodysize){
//                nextL.range = getRange(relPtr[nextL.ind-1]);
//                 //   nextL.range =  ( startEnd*)DB.Getbody(range_TYPE,relPtr[nextL.ind-1],0);
//                if(start<=nextL.range->max){
//                    stack.vadd(1,nextL);
//                }
//            }
//            else
//                stack.vadd(1,nextL);
//
//            vtreeNode nextR;
//            nextR.ind=getRight(curr.ind,&curr.level);
//            nextR.level=curr.level-1;
//            if(nextR.ind<bodysize){
//                nextR.range = getRange(nextR.ind-1);
//                if(start<=nextR.range->max && end>=nextR.range->start){
//                    stack.vadd(1,nextR);
//                }
//            }
//        }
//
//    }
//    return results.dim();
//}
