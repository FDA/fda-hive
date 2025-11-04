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
#ifndef cloneTrees_hpp
#define cloneTrees_hpp

#include <slib/utils.hpp>
#include <ssci/math/clust/clust2.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>

#define DEBUGGING_KILLING
#define DEBUGGING_MERGING

#define RAND1 (real)rand()/RAND_MAX

class sAlTree {

    public:
        struct cloneNode {
            idx posStart;
            idx posEnd;
            idx iAl;
            sVec<idx> treeList;
        };
        struct treeStat {
            idx id, start, end, dim,startAl, endAl;
        };
        struct alNodes {
            idx Al;
            idx rpt;
        };
        sVec<cloneNode> readDic;
        sVec<cloneNode> readsList;
        sVec<treeStat> treesList;
        idx cur_treeId;
        idx _size,_space;
        real _overlap;
        real _forceRefreshPortion;

    private:
        idx _startAl,_endAl;
        idx _start,_end;
        idx _fStart,_fEnd;
        idx _oStart, _oEnd;
        idx _iStart, _iEnd;
        idx * _alList;
        idx _step, _width, _curWidth, _readSize;
        real _readOffset;
        sHiveal * _hiveal;
        bool _isFixedMode;
        bool _isRandom;

        void _setEndAl(idx endAl){
            _endAl = endAl;
        }
        void _setStartAl(idx startAl){
            _startAl = startAl;
        }

        bool _isNodeInOverlap(cloneNode * node) {
            return _isInOverlap(node->posStart, node->posEnd);
        }

        bool _isNodeInFrame (cloneNode * node) {
            return _isInFrame(node->posStart, node->posEnd);
        }

        bool _isNodeValid(cloneNode * node) {
            return _isValid(node->posStart, node->posEnd);
        }

        bool _isInOverlap (idx st, idx end) {

            bool res = ( st <= _oStart ) && ( end >= _oEnd );
            return res;
        }

        bool _isInFrame (idx st, idx end) {
            idx t_start = (_iStart>=0)?_iStart:0;
            idx t_end = _iEnd ;
            bool res = ( end > t_start ) && (st <= t_start);
            if ( !res ) {
                res = ( st < t_end ) && ( end >= t_end );
            }
            return res;
        }

        bool _isValid (idx st, idx end) {
            if( !_overlap ) {
                return _isInFrame(st,end);
            }
            else {
                return _isInFrame(st,end) && _isInOverlap(st,end);
            }
        }


        void _setWidth (idx width) {
            _width = width;
        }

        void _setCord ( idx start ) {
            _fStart = start;
            _start = start;
            _fEnd = start + _width;
            _end = _fEnd;

        }

        void _setStep (idx step ) {
            _step = step;
        }

        void _setReadSize (idx readSize) {
            _readSize = readSize;
        }


        idx _getOverlapStart () {
            idx t_width = _getOverlapSize ();
            return _start + (t_width - t_width*_overlap)/2;
        }
        idx _getOverlapSize () {
            idx width = _width;
            if( _width > _curWidth && _curWidth )
                width = _curWidth;
            return width * _overlap;
        }
        idx _getOverlapEnd () {
            return _getOverlapStart() + _getOverlapSize();
        }
        idx _getAl (idx iAl) {
            return  _alList?_alList[iAl]:iAl;
        }


        void _setOverlap (real overlap) {
            if( overlap > 1 ) {
                overlap = 1;
            }
            else if ( overlap < 0 ) {
                overlap = 0;
            }
            _overlap = overlap;
        }
        void _setOverlapCord () {
            if( !_overlap ) {
                _oStart = _start;
                _oEnd = _end;
            }
            else {
                _oStart = _getOverlapStart();
                _oEnd = _getOverlapEnd();
            }
        }

        void _setInputOffset () {
            if( !_overlap ) {
                _iStart = _start;
                _iEnd = _end;
            }
            else {
                idx dif = _readSize - _getOverlapSize();
                if( dif < 0 ) dif = 0;
                _iStart = _oStart - dif;
                _iEnd = _start+1;
            }
        }


        idx _generateRandVector(idx iStart, idx iEnd,idx cnt, sVec<idx> & out);
        idx _getPosEnd(idx iAl);
        idx _getPosStart(idx iAl);
        idx  _getNextAlInds( bool reStart = false );
        idx _refreshPoolToPos();
        void _updateStartsOnAl();
        void _updateEndsOnAl();
        void _moveFrameInds();
        void _addTree() ;
        void _dic2VecIDs(sVec<alNodes> & qryIds) ;


    public:

        sAlTree ( idx size,sHiveal * hiveal, idx * alList=0){
            _size = size;
            _space = _size;
            _alList = alList;
            _hiveal = hiveal;
            init();
        }
        void init() {
            _startAl = 0;_endAl = 0;
            _start = -1;_end = -1;
            _oStart = -1;_oEnd = -1;
            _iStart = -1;_iEnd = -1;
            _fStart = -1; _fEnd = -1;
            _isFixedMode = false;
            _isRandom = false;
            readDic.empty();
            treesList.empty();
            readsList.empty();
            cur_treeId=0;
            _space =  _size ;
            _step = 0;
            _overlap = 0;
            _width = 0;
            _curWidth = 0;
            _forceRefreshPortion = 0.5;
        }
        void setFixedMode (idx start, idx width, idx step, real overlap, idx readSize) {
            _isFixedMode = true;
            _setWidth( width );
            _setStep( step );
            _setOverlap ( overlap );
            _setReadSize ( readSize );

            _setCord ( start-step );
            _setOverlapCord();
            _setInputOffset ();
        }

        idx getStart() {
            return _start;
        }
        idx getEnd() {
            return _end;
        }

        idx appendToReadsList(cloneNode * node);

        idx fillRange(sVec<alNodes> & qryIds, idx * startAl = 0 , idx * endAl = 0 );

        void printTreeListJSON( sStr & buf);

        bool isFull() {
            return !_space;
        }
        bool isRandom() {
            return _isRandom;
        }
        bool isFixedMode() {
            return _isFixedMode;
        }

        static idx createNJTree(sVec<sAlTree::alNodes> & mappedAl,sBioal * bioal, sStr &buf, idx frameStart, idx frameEnd, idx treeSize = 0, bool fixedTreeSize = false);
        static idx calculatePairwiseDistance(sBioal * bioal, idx * ai1, idx * ai2, idx frameStart, idx frameEnd, bool countNonOverlappingRegions = true);
        static idx nodePrintfCallback(sStr &out, sHierarchicalClustering &clust, idx x, void *param)
        {
            sStr * idList = static_cast<sStr*>(param);
            if (x >= 0 && x < sString::cnt00( idList->ptr() ))
                out.printf("%s", sString::next00(idList->ptr(),x) );
            return 0;
        }
};
#endif 