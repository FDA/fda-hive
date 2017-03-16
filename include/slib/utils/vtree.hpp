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
#ifndef  sLib_utils_vtree_hpp
#define  sLib_utils_vtree_hpp

#include <slib/core.hpp>
#include <math.h>

#define inVirtualRange(a1, a2, b1, b2) ( (_compare(a1,b1,params,sVTree::eNC_SS)<=0) ? ( (_compare(b1,a2,params,sVTree::eNC_SE)<=0)?1:0) : ((_compare(a1,b2,params,sVTree::eNC_SE)<=0)?1:0)  )


class sVTree
{
    public:

        enum eNodeComparisonType {
            eNC_SS=0x00,       //start start
            eNC_SE=0x01,       //start end
            eNC_ES=0x10,       //end   start
            eNC_EE=0x11,       //end   end
            eNC_Max
        };

        //deBruijn Sequence and LUT is used to find the LSB of the index ('ind') with ONE operation
        //LSB in the virtual tree indicates the level of the tree
        static const idx deBrLUT_32[];

        static const idx deBrSeq_32 = 0x077cb531;

        static const idx deBrLUT[];

        static const idx deBrSeq = 0x022fdd63cc95386d;

        inline static idx getLSBindex1(idx & a){
            return deBrLUT[ (a&-a)*deBrSeq >> 58];
        };

        struct rangeNode {
                void * start, * end; idx max;
                rangeNode(){
                    sSet(this,0);
                }
        };

        struct vTreeNode {
                rangeNode range;
                idx _ind_level;
                vTreeNode(){
                    sSet(this,0);
                }
                idx getInd(){
                    return (_ind_level&0xFFFFFFFFFFFF);
                }
                idx getLevel(){
                    return (_ind_level>>48)&(0xFFFF);
                }
                void setInd(idx v){
                    _ind_level=(_ind_level&0xFFFFFFFF00000000ull)|(v&(0xFFFFFFFFFFFF));
                }
                void setLevel(idx v){
                    _ind_level=(_ind_level&0x0000FFFFFFFFFFFFull)|((v&(0xFFFF))<<48);
                }
        };


        typedef idx (* sCallbackVTreeComparator)( void * i1, void * i2, void * params, idx type );
        typedef bool (* sCallbackVTreeGetNode)( void * rngPtr, void * maxPtr, idx ind, rangeNode * node, void * params );
        typedef void (* sCallbackVTreeSetMax)( void * rngPtr, void * maxPtr, idx ind, idx max_ind, void * params );

        struct vTreeCallbacksStruct {
                sCallbackVTreeGetNode getNode;
                sCallbackVTreeComparator compare;
                sCallbackVTreeSetMax setMax;
        };

        static idx idxComparator( void * i1, void * i2, void * params ) {
            return (*(idx *)i1) - (*(idx *)i2);
        }

        static bool getIdxNode( void * rngPtr, void * maxPtr, idx ind, rangeNode * node, void * params ) {
            rangeNode * rnglist = (rangeNode *)rngPtr;
            --ind;
            if(node->start) node->start = &rnglist[ind].start;
            if(node->end) node->end = &rnglist[ind].end;
            if(node->max) {
//                idx ind_max = 0;
                if(maxPtr) {
                    idx * maxlist = (idx *)maxPtr;
                    node->max = maxlist[ind];
                } else {
                    node->max = rnglist[ind].max;//maxlist[ind];
                }
            }
            return true;
        }

        static void setIdxMax( void * rngPtr, void * maxPtr, idx ind, idx max_ind, void * params ) {
            --ind;
            if( maxPtr ) {
                idx * maxlist = (idx *)maxPtr;
                maxlist[ind] = max_ind;
            } else {
                rangeNode * rnglist = (rangeNode *)rngPtr;
                rnglist[ind].max = max_ind;
            }
        }

        sVTree( void * t_rngPtr, idx t_rngCnt, void * t_maxPtr = 0, vTreeCallbacksStruct * callbacks = 0)
        {
            init( t_rngPtr, t_rngCnt, t_maxPtr, callbacks);
        }

        void init( void * t_rngPtr, idx t_rngCnt, void * t_maxPtr, vTreeCallbacksStruct * callbacks )
        {
            _rngPtr = t_rngPtr;
            _maxPtr = t_maxPtr;
            _size = t_rngCnt;
            _getNode = (callbacks&&callbacks->getNode)?callbacks->getNode:(sCallbackVTreeGetNode)getIdxNode;
            _compare = (callbacks&&callbacks->compare)?callbacks->compare:(sCallbackVTreeComparator)idxComparator;
            _setMax = (callbacks&&callbacks->setMax)?callbacks->setMax:(sCallbackVTreeSetMax)setIdxMax;

            _depth=floor(log10(_size)/log10(2));
            _virualSize=pow(2,_depth+1)-1;
        }

    private:
        sCallbackVTreeGetNode _getNode; sCallbackVTreeComparator _compare; sCallbackVTreeSetMax _setMax;
        idx _size,_depth,_virualSize;
        void * _rngPtr, *_maxPtr;

        idx _getRoot(idx depth){
            idx root=0;
            return root|(1<<depth);
        };
        idx _getParent(idx x, idx &level){
            x&=~(3<<level);
            x|=2<<level;
            return x;
        };
        idx _getLeft(idx x, idx &level){
            if(level<=1)return 0;
            x&=~(1<<(level-1));
            x|=(1<<(level-2));
            return x;
        };
        idx _getRight(idx x, idx &level){
            if(level<=1)return 0;
            x|=(1<<(level-2));
            return x;
        };

    public:
        idx search( void * start, void * end, sVec<idx> & results, idx cnt = 0, sVec<vTreeNode> * state = 0 , void * params = 0 );
        idx fixMax(void * params);

};


#endif
