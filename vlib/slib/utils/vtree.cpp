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

#include <slib/utils/vtree.hpp>

using namespace slib;


const idx sVTree::deBrLUT[] = {
            0,  1,  2, 53,  3,  7, 54, 27,
            4, 38, 41,  8, 34, 55, 48, 28,
           62,  5, 39, 46, 44, 42, 22,  9,
           24, 35, 59, 56, 49, 18, 29, 11,
           63, 52,  6, 26, 37, 40, 33, 47,
           61, 45, 43, 21, 23, 58, 17, 10,
           51, 25, 36, 32, 60, 20, 57, 16,
           50, 31, 19, 15, 30, 14, 13, 12,
        };
const idx sVTree::deBrLUT_32[]={
             0,  1, 28,  2, 29, 14, 24,  3,
            30, 22, 20, 15, 25, 17,  4,  8,
            31, 27, 13, 23, 21, 19, 16,  7,
            26, 12, 18, 6, 11, 5, 10, 9
        };

idx sVTree::fixMax(void * params) {
    idx current = 0, parent = 0,current_max = 0,parent_max = 0, cnt = 0;
    rangeNode c_node, p_node;
    for(idx i=1; i<_virualSize; i+=2){
        idx level=0;
        current = i;
        parent = current;
        while( level<_depth ) {
            parent = _getParent(parent,level);

            if(parent>_size){ ++level;continue;}

            c_node.max = 1;p_node.max = 1;
            _getNode( _rngPtr, _maxPtr, current, &c_node, params );
            _getNode( _rngPtr, _maxPtr, parent, &p_node, params);
            current_max = c_node.max; parent_max = p_node.max;
            if(!current_max)current_max = current;
            if(!parent_max)parent_max = parent;

            c_node.end = (void *)1;p_node.end = (void *)1;
            c_node.max = 0;p_node.max = 0;
            _getNode( _rngPtr, _maxPtr,current_max, &c_node, params );
            _getNode( _rngPtr, _maxPtr, parent_max, &p_node, params);

            if( _compare(c_node.end, p_node.end, params, sVTree::eNC_EE)>0 ) {
                _setMax(_rngPtr, _maxPtr, parent, current_max, params);
                ++cnt;
            }
            current=parent;
            ++level;
        }
    }
    return cnt;
};
idx sVTree::search(void * start, void * end, sVec<idx> & results, idx cnt, sVec<vTreeNode> * state, void * params)
{
    sVec<vTreeNode> stack, * pstack;
    vTreeNode curr;
    idx level = 0, max_ind = 0;
    if(state) {
        pstack = state;
    } else {
        pstack = &stack;
    }
    if(!pstack->dim()) {
        curr.range.start=(void *)1,curr.range.end=(void *)1,curr.range.max=1;
        curr.setInd(_getRoot(_depth));
        curr.setLevel(_depth+1);
        _getNode( _rngPtr, _maxPtr, curr.getInd(), &curr.range, params );
        *pstack->add() = curr;
    }
    if(!cnt) cnt = sIdxMax;

    if(!end)end=start;

    idx new_cnt = cnt + results.dim();
PERF_START("Loop inside Tree");
    while( pstack->dim() && results.dim() < new_cnt ){
        curr=*pstack->ptr(pstack->dim()-1);
        pstack->cut(pstack->dim()-1);

        if(curr.getInd()>_size){
            if(curr.getLevel()>0){
                vTreeNode next;
                level = curr.getLevel();
                next.setInd(_getLeft(curr.getInd(),level ));
                next.setLevel(curr.getLevel()-1);
                *pstack->add() = next;
            }
            continue;
        }
        if(!curr.range.start || !curr.range.end) {
            curr.range.start=(void *)1,curr.range.end=(void *)1;
            _getNode( _rngPtr, _maxPtr, curr.getInd(), &curr.range, params );
        }

        if( inVirtualRange(start,end,curr.range.start,curr.range.end) ){
            results.vadd(1,curr.getInd() );
        }

        if(curr.getLevel()>1){
            vTreeNode nextL;
            level = curr.getLevel();
            nextL.setInd( _getLeft( curr.getInd(), level ));
            nextL.setLevel(curr.getLevel()-1);
            if(nextL.getInd()<_size){
                nextL.range.max=1;
                _getNode( _rngPtr, _maxPtr, nextL.getInd(), &nextL.range, params );
                max_ind = nextL.range.max;
                if(!max_ind) max_ind = nextL.getInd();
                nextL.range.start=0;nextL.range.end=(void *)1;nextL.range.max=0;
                _getNode( _rngPtr, _maxPtr, max_ind, &nextL.range, params );
                if( _compare(start, nextL.range.end, params, sVTree::eNC_SE)<=0 ){
                    nextL.range.end=0;
                    *pstack->add() = nextL;
                }
            } else{
                *pstack->add() = nextL;
            }
            vTreeNode nextR;
            level = curr.getLevel();
            nextR.setInd( _getRight( curr.getInd(), level ));
            nextR.setLevel(curr.getLevel()-1);
            if(nextR.getInd()<_size){
                nextR.range.max=1;
                _getNode( _rngPtr, _maxPtr, nextR.getInd(), &nextR.range, params );
                max_ind = nextR.range.max;
                if(!max_ind) max_ind = nextR.getInd();
                nextR.range.max=0;nextR.range.start=0;nextR.range.end=(void *)1;
                _getNode( _rngPtr, _maxPtr, max_ind, &nextR.range, params );
                if( _compare(end, curr.range.start, params, sVTree::eNC_ES)>=0 && _compare(start, nextR.range.end, params, sVTree::eNC_SE)<=0 ){
                    nextR.range.end=0;
                    *pstack->add() = nextR;
                }
            } else if ( curr.getInd() < _size && _compare(end, curr.range.start, params, sVTree::eNC_ES)>=0 ){
                *pstack->add() = nextR;
            }
        }
    }
PERF_END();
    return results.dim();
}
