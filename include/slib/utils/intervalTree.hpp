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
#ifndef sLib_utils_intervalTree_hpp
#define sLib_utils_intervalTree_hpp

#include <slib/core/vec.hpp>
#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>
#include <slib/utils/sort.hpp>
#include <slib/utils/sRangeTree.hpp>
#include <math.h>

namespace slib {

    template<class Tobj = idx, class Kobj = idx> class sRangeIntervalTreeList
    {
        public:
            idx pointerForSizeValidation;
            typedef sRangeTree<Tobj, Kobj> sRangetree;
            struct NodeRange
            {
                Tobj start, end;    //start,end, Tind:range index, Rind,
                NodeRange()
                {
                    sSet(this, 0);
                }
            };
            struct setRanges
            {
                idx id;
                sVec<NodeRange> nodes;
                setRanges(){id=0;}
            };

            idx getDim ()
            {
                return _vec.dim();
            }

            idx getVecDim(idx nodeNum)
            {
                if (_vec.dim() > nodeNum){
                    return _vec.ptr(nodeNum)->nodes.dim();
                }
                return 0;
            }

            idx getVecRangeSize(idx nodeNum)
            {
                if( _vec.dim() > nodeNum ) {

                    return (_vec.ptr(nodeNum)->nodes.ptr(0)->end - _vec.ptr(nodeNum)->nodes.ptr(0)->start);
                }
                return 0;
            }

            // Number of Nodes in the Tree
            idx getTreeDim()
            {
                return _rTree._vec.dim();
            }

            void giveMeEverything(sStr *buf )
            {
                buf->printf ("Number of Range Nodes: %" DEC "\n", getDim());
                buf->printf ("Number of Nodes in the Tree:%" DEC "\n", getTreeDim());

                sDic<sSort::Lenstats> lenStat;

                lenStat.mex()->flags |= sMex::fSetZero;

                for (idx i = 0; i < _vec.dim(); i ++){
//                    sStr s_vecSize;
//                    s_vecSize.printf("%" DEC,getVecDim(i));
//                    lenStat[s_vecSize.ptr()]++;
                    idx seqlen = getVecDim(i);
                    sSort::Lenstats * auxlenStat = lenStat.set(&seqlen, sizeof(seqlen));
                    auxlenStat->num += 1;
                    auxlenStat->sum = getVecRangeSize(i);
                }

                sVec<idx> ind;
                ind.cut(0);
                ind.add(lenStat.dim());
                sSort::sortCallback(sSort::sort_idxDicComparator, 0, lenStat.dim(), &lenStat, ind.ptr());

                buf->printf("Number of Nodes,count\n");
                for(idx l = 0; l < lenStat.dim(); ++l) {
                    idx aux = *(idx *)lenStat.id(ind[l]);
                    sSort::Lenstats *auxlen = lenStat.ptr(ind[l]);
                    buf->printf("%" DEC ",%" DEC ",%" DEC "\n", aux, auxlen->num, auxlen->sum);
                }
            }

        private:
            sVec< setRanges > _vec;
            sRangetree _rTree;


            idx addExistingRange(idx start, idx end, idx iptr)
            {
                if( iptr >= _vec.dim() ) {
                    return 0;
                }
                setRanges * t_set = _vec.ptr(iptr);
                NodeRange * t_node = t_set->nodes.add();
                t_node->start = start;
                t_node->end = end;
                return t_set->nodes.dim();
            }
            idx addNewRange(idx start, idx end, idx id)
            {
                setRanges * t_set = _vec.add();
                t_set->id = id;
                NodeRange * t_node = t_set->nodes.add();
                t_node->start = start;
                t_node->end = end;
                return _vec.dim();
            }

            idx addExistingRangeSorted(idx start, idx end, idx iptr){
                if(iptr >= _vec.dim()){
                    return 0;
                }
                setRanges * t_set = _vec.ptr(iptr);
                NodeRange * t_node = 0;
                bool insertion = false;
                for (idx i = 0; i < t_set->nodes.dim(); ++i){
                    if ( start < (t_set->nodes.ptr(i)->start)){
                        t_node = t_set->nodes.insert(i,1);
                        insertion = true;
                        break;
                    }
                }
                if (!insertion){
                    t_node = t_set->nodes.add();
                }
                t_node->start = start;
                t_node->end = end;
                return t_set->nodes.dim();
            }

            idx addNewRangeSorted(idx start, idx end, idx id){
                setRanges * t_set = _vec.add();
                t_set->id = id;
//                NodeRange * t_node = t_set->nodes.add();
                NodeRange * t_node = 0;
                bool insertion = false;
                for (idx i = 0; i < t_set->nodes.dim(); ++i){
                    if ( start < (t_set->nodes.ptr(i)->start)){
                        t_node = t_set->nodes.insert(i,1);
                        insertion = true;
                        break;
                    }
                }
                if (!insertion){
                    t_node = t_set->nodes.add();
                }
                t_node->start = start;
                t_node->end = end;
                return _vec.dim();
            }

            idx _insert(idx start, idx end, idx * id = 0 )
            {
                idx range_id = _vec.dim();
                if( id ) {
                    range_id = *id;
                    if( !addExistingRange(start,end,range_id) ){
                        return -1;
                    }
                }
                else{
                    addNewRange(start,end,range_id);
                }
                _rTree.insert(start,end,range_id);
                return range_id;
            }

        public:

            void init (idx tree_mode = 0)
            {
                _vec.mex()->flags |= sMex::fSetZero;
                _vec.cut(0);
                _rTree.init();
                _rTree.setMode(tree_mode);

            }

            sRangeIntervalTreeList()
            {
                init (0);
            }

            sRangeIntervalTreeList(idx tree_mode)//const char * flnm = 0, bool doMap = false)
            {
                _vec.mex()->flags |= sMex::fSetZero;
                _rTree.setMode(tree_mode);
            }

            idx insert(idx start1, idx end1, idx start2, idx end2)
            {
                sVec<idx> search_res1,search_res2;
                bool range1_exists = _rTree.exactSearch(search_res1,start1,end1);
                bool range2_exists = _rTree.exactSearch(search_res2,start2,end2);
                idx range_id = 0;
                if( !range1_exists && !range2_exists ) {
                    range_id = _insert(start1,end1);
                    range_id = _insert(start2,end2,&range_id);
                }
                else if ( range1_exists ){
                    if (range2_exists){
                        return 0;
                    }
                    range_id = _insert(start2,end2, search_res1.ptr(0));
                }
                else if( range2_exists ) {
                    range_id = _insert(start1,end1, search_res2.ptr(0));
                }
                else {
                    // bug (or not?)
                }
                return range_id;
            }

            idx search(idx position1, idx position2)
            {
                sVec<idx> search_res1,search_res2;
                bool range1_exists = _rTree.search(search_res1,position1);

                bool range2_exists = _rTree.search(search_res2,position2);
                idx jump = 0;
                idx range_id = 0;
                if( range1_exists && range2_exists ) {
                    range_id = _insert(position1);
                    range_id = _insert(position2,&range_id);
                }

                return range_id;
            }

            idx searchInRangeSet (idx iptr, idx start, idx position1, idx position2)
            {
                if(iptr >= _vec.dim()){
                    return 0;
                }
                setRanges * t_set = _vec.ptr(iptr);
                idx ioffset = position1 - start;
                for (idx i = 0; i < t_set->nodes.dim(); ++i){
                    NodeRange * t_node = t_set->nodes.ptr(i);

                    if ( (t_node->start != start)  && (t_node->start + ioffset == position2 ) ){
                        return (t_node->end - position2);
                    }
                }
                return -1;
            }

            idx searchInSortedRangeSet (idx iptr, idx start, idx position1, idx position2)
            {
                if(iptr >= _vec.dim()){
                    return 0;
                }
                setRanges * t_set = _vec.ptr(iptr);
                idx ioffset = position1 - start;
                idx low = 0;
                idx middle = 0;
                idx high = t_set->nodes.dim();
                while (low < high){
                    middle = (low + high) / 2;
                    NodeRange * t_node = t_set->nodes.ptr(middle);

                    if ( (t_node->start != start)  && (t_node->start + ioffset == position2 ) ){
                        return (t_node->end - position2);
                    }
                    if(t_node->start < (t_node->start + ioffset < position2 )){
                        high = middle - 1;
                    }
                    else {
                        low = middle + 1;
                    }
                }
                return -1;
            }

            idx search2(idx position1, idx position2)
            {
                typedef sRangenode<Tobj, Kobj> sRangeNode;
                sVec <sRangeNode *> stack;

                sRangeNode *curr = _rTree.getRoot();

                if (curr == 0){
                    // the tree is empty
                    return 0;
                }
                stack.vadd(1, curr);
                bool isPresent;
                while (stack.dim()){
                    curr = *stack.ptr(stack.dim() - 1);
                    isPresent = _rTree.searchOneNode(stack, position1, position1, 0);
                    if (isPresent){
                        // I found something and I need to check in my sVec
                        idx offset = searchInRangeSet (curr->value, curr->start, position1, position2);
                        // If I found something, return the value
                        if (offset >= 0){
                            return offset;
                        }
                    }
                }
                return 0;
           }

            idx search(idx position, idx RangeI, idx ReferenceI)
            {
                return _rTree.search(position);
            }

    };

}

#endif
