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
#ifndef sMath_clust2_h
#define sMath_clust2_h

#include <assert.h>

#ifdef _DEBUG_CLUST
#include <stdio.h>
#endif

#include <ssci/math/geom/dist.hpp>
#include <slib/core/iter.hpp>
#include <slib/core/heap.hpp>
#include <slib/core/net.hpp>
#include <slib/core/str.hpp>
#include <slib/std/string.hpp>

namespace slib
{
    class sHierarchicalClusteringTree
    {
    public:
        struct ClusteringKnotData {
            real dist;
            idx numLeaves;
            idx quality;
            idx qualityMax;

            ClusteringKnotData() { reset(); }
            void reset() {
                sSet(this, 0);
                dist = NAN;
            }
        };
        typedef sKnot<struct ClusteringKnotData> Tnode;

        enum NewickOptions {
            Newick_PRINT_DISTANCE = 1 << 0,
            Newick_PRINT_ROOT = 1 << 1,
            Newick_PRINT_LEAF_NAMES = 1 << 2,
            Newick_PRINT_INNER_NAMES = 1 << 3,
        };

        typedef idx (*NodePrintfCallback)(sStr &out, sHierarchicalClusteringTree &tree, idx x, void *param);

    protected:
        sMex _node_inout_mex;
        sVec<Tnode> _vec;

        inline bool validIndex(idx i) { return i >= 0 && i < dim(); }
        inline bool parentless(idx i)
        {
            assert (validIndex(i));
            return _vec[i].in[0] == -1;
        }
        inline bool childless(idx i)
        {
            assert (validIndex(i));
            return _vec[i].out[0] == -1 && _vec[i].out[1] == -1;
        }

        void printNewickName(sStr & out, idx i, NodePrintfCallback func, void *param)
        {
            if (func != NULL) {
                static sStr buf;
                buf.cut0cut();
                func(buf, *this, i, param);
                buf.shrink00();
                bool needEscape = !buf.length();
                for (idx ib=0; ib<buf.length(); ib++) {
                    if (!(buf[ib] >= '0' && buf[ib] <= '9') && !(buf[ib] >= 'a' && buf[ib] <= 'z') && !(buf[ib] >= 'A' && buf[ib] <= 'Z') && !(buf[ib] == ' ' && ib > 0 && ib < buf.length() - 1)) {
                        needEscape = true;
                        break;
                    }
                }
                if (needEscape) {
                    sString::escapeForJSON(out, buf.ptr());
                } else {
                    out.addString(buf.ptr());
                }
            } else {
                out.printf("%" DEC, i);
            }
        }

        virtual void printNewickNode(sStr &out, idx i, idx options, NodePrintfCallback func, void *param)
        {
            if (i < 0 || i >= dim())
                return;
            if (i < dimLeaves()) {
                if (options & Newick_PRINT_LEAF_NAMES)
                    printNewickName(out, i, func, param);
                if ((options & Newick_PRINT_DISTANCE) && !isnan(_vec[i].obj.dist))
                    out.printf(":%g", _vec[i].obj.dist);
                return;
            }
            out.printf("(");
            printNewickNode(out, _vec[i].out[0], options, func, param);
            out.printf(", ");
            printNewickNode(out, _vec[i].out[1], options, func, param);
            out.printf(")");
            bool atroot = (i == dim() - 1 && i >= dimLeaves());
            if (!atroot || (options & Newick_PRINT_ROOT)) {
                if (options & Newick_PRINT_INNER_NAMES)
                    printNewickName(out, i, func, param);
                if ((options & Newick_PRINT_DISTANCE) && !isnan(_vec[i].obj.dist))
                    out.printf(":%g", _vec[i].obj.dist);
            }
            if (atroot)
                out.printf(";");
        }

        idx getLeavesWorker(sVec<idx> & out, idx out_offset, idx i) const
        {
            if (i < 0 || i >= dim()) {
                return out_offset;
            } else if (i < dimLeaves()) {
                out[out_offset] = i;
                return out_offset + 1;
            }

            out_offset = getLeavesWorker(out, out_offset, _vec[i].out[0]);
            out_offset = getLeavesWorker(out, out_offset, _vec[i].out[1]);
            return out_offset;
        }

    public:
        sHierarchicalClusteringTree(idx dimLeaves=0): _vec(sMex::fExactSize) { reset(dimLeaves); }
        virtual ~sHierarchicalClusteringTree() {}
        void reset(idx dimLeaves)
        {
            idx oldSize = dim();
            _vec.resize((dimLeaves << 1) - 1);
            for (idx i=0; i<dimLeaves; i++) {
                if (i >= oldSize) {
                    _vec[i].init(&_node_inout_mex);
                }
                _vec[i].obj.reset();
                _vec[i].obj.numLeaves = 1;
                _vec[i].in.resize(1);
                _vec[i].in[0] = -1;
                if (_vec[i].out.dim())
                    for (idx j=0; i<_vec[i].out.dim(); j++)
                        _vec[i].out[j] = -1;
            }
            for (idx i=dimLeaves; i<_vec.dim(); i++) {
                if (i >= oldSize) {
                    _vec[i].init(&_node_inout_mex);
                }
                _vec[i].obj.reset();
                _vec[i].obj.numLeaves = 0;
                _vec[i].in.resize(1);
                _vec[i].in[0] = -1;
                _vec[i].out.resize(2);
                _vec[i].out[0] = _vec[i].out[1] = -1;
            }
        }
        inline idx dimLeaves() const { return _vec.dim() ? (_vec.dim()+1) >> 1 : 0; }
        inline idx dim() const { return _vec.dim(); }
        void makeParent(idx parent, idx child0, real dist0, idx child1, real dist1)
        {
            assert (childless(parent));
            assert (parent >= dimLeaves());
            assert (parentless(child0));
            assert (parentless(child1));
            assert (parent != child0 && parent != child0 && child0 != child1);

            _vec[parent].out[0] = child0;
            _vec[parent].out[1] = child1;
            _vec[parent].obj.numLeaves = _vec[child0].obj.numLeaves + _vec[child1].obj.numLeaves;
            _vec[child0].in[0] = parent;
            _vec[child0].obj.dist = dist0;
            _vec[child1].in[0] = parent;
            _vec[child1].obj.dist = dist1;
        }

        inline const sVec<Tnode>& getNodeVec() const { return _vec; }
        inline sVec<Tnode>& getNodeVec() { return _vec; }

        inline const Tnode& operator[](idx i) const { return _vec[i]; }
        inline Tnode& operator[](idx i) { return _vec[i]; }

        inline const Tnode& getRoot() const { return _vec[_vec.dim() - 1]; }
        inline Tnode& getRoot() { return _vec[_vec.dim() - 1]; }

        inline virtual void printNewick(sStr &out, int options = Newick_PRINT_DISTANCE|Newick_PRINT_LEAF_NAMES, NodePrintfCallback func = NULL, void *param = NULL)
        {
            printNewickNode(out, dim()-1, options, func, param);
        }

        idx getLeaves(sVec<idx> & out) const
        {
            idx out_start = out.dim();
            out.add(dimLeaves());
            getLeavesWorker(out, out_start, _vec.dim() - 1);
            return dimLeaves();
        }
    };

    class sHierarchicalClustering: public sDistMatrix
    {
    public:
        typedef idx (*NodePrintfCallback)(sStr &out, sHierarchicalClustering &clust, idx x, void *param);

    protected:
        sHierarchicalClusteringTree _tree;
        sVec<idx> _matrix2tree;

        virtual void init()
        {
            _tree.reset(sDistMatrix::_npoints);
            _matrix2tree.resize(_tree.dim());
            for (idx i=0; i<sDistMatrix::_npoints; i++)
                _matrix2tree[i] = i;
        }

        virtual bool peekNext(idx *x, idx *nearesty) = 0;
        virtual void mergeNext(real *distx, real *disty) = 0;

        virtual bool buildTree(progressCb prog_cb = 0, void * prog_param = 0)
        {
            for(idx i=sDistMatrix::_npoints; i<_tree.dim(); i++) {
                if( prog_cb ) {
                    prog_cb(prog_param, i, sNotIdx, _tree.dim() - sDistMatrix::_npoints);
                }
                idx x = -1, y = -1;
                peekNext(&x, &y);
#ifdef _DEBUG_CLUST
                printf("Merging clusters %" DEC " and %" DEC "\n", x, y);
                dump();
#endif

                assert(x >= 0); assert(x < sDistMatrix::_npoints);
                assert(y >= 0); assert(y < sDistMatrix::_npoints);
                real d = sDistMatrix::dist(x, y);

                idx xi = _matrix2tree[x];
                idx yi = _matrix2tree[y];

                real distx = d/2, disty = d/2;
                mergeNext(&distx, &disty);
                _matrix2tree[x] = i;

                _tree.makeParent(i, xi, distx, yi, disty);
            }
#ifdef _DEBUG_CLUST
            printf("Built tree\n");
            dump();
#endif
            return true;
        }

        template <class Tclust_>
        struct NodePrintfCallbackWrapper
        {
            Tclust_ &clust;
            typename Tclust_::NodePrintfCallback func;
            void *param;
            NodePrintfCallbackWrapper<Tclust_>(Tclust_ &c, typename Tclust_::NodePrintfCallback f, void *p): clust(c), func(f), param(p) {}
        };

        static idx wrapNodePrintfCallback(sStr &out, sDistMatrix &matrix, idx x, void *param)
        {
            NodePrintfCallbackWrapper<sHierarchicalClustering> *wrapper = static_cast<NodePrintfCallbackWrapper<sHierarchicalClustering>*>(param);
            return wrapper->func(out, wrapper->clust, x, wrapper->param);
        }

        static idx wrapTreeNodePrintfCallback(sStr &out, sHierarchicalClusteringTree &tree, idx x, void *param)
        {
            NodePrintfCallbackWrapper<sHierarchicalClustering> *wrapper = static_cast<NodePrintfCallbackWrapper<sHierarchicalClustering>*>(param);
            return wrapper->func(out, wrapper->clust, x, wrapper->param);
        }

    public:
        template<class Tdata, class Titer>
        inline bool resetDistance(const Titer *points, idx npoints, const sDist<Tdata,Titer> &distObj, progressCb prog_cb = 0, void * prog_param = 0)
        {
            return sDistMatrix::reset(points, npoints, distObj, prog_cb, prog_param);
        }

        inline virtual bool resetDistance(idx npoints, DistCallback d, void * param, progressCb prog_cb = 0, void * prog_param = 0)
        {
            return sDistMatrix::reset(npoints, d, param, prog_cb, prog_param);
        }

        inline virtual bool resetDistance(const sDistMatrix &rhs)
        {
            return sDistMatrix::reset(rhs);
        }

        inline virtual bool recluster(progressCb prog_cb = 0, void * prog_param = 0)
        {
            this->init();
            return this->buildTree(prog_cb, prog_param);
        }

        sHierarchicalClustering(const sDistMatrix &rhs): sDistMatrix(rhs), _matrix2tree(sMex::fExactSize) {}
        sHierarchicalClustering(): sDistMatrix(0), _matrix2tree(sMex::fExactSize) {}

        inline virtual const sHierarchicalClusteringTree& getTree() const {return _tree;}
        inline virtual sHierarchicalClusteringTree& getTree() {return _tree;}

        inline virtual void printNewick(sStr &out, int options = sHierarchicalClusteringTree::Newick_PRINT_DISTANCE|sHierarchicalClusteringTree::Newick_PRINT_LEAF_NAMES, NodePrintfCallback func = NULL, void *param = NULL)
        {
            if (func) {
                NodePrintfCallbackWrapper<sHierarchicalClustering> wrapper(*this, func, param);
                _tree.printNewick(out, options, wrapTreeNodePrintfCallback, &wrapper);
            } else
                _tree.printNewick(out, options);
        }

        inline virtual void printMatrixCSV(sStr &out, NodePrintfCallback func = NULL, void *param = NULL)
        {
            if (func) {
                NodePrintfCallbackWrapper<sHierarchicalClustering> wrapper(*this, func, param);
                sDistMatrix::printMatrixCSV(out, wrapNodePrintfCallback, &wrapper);
            } else
                sDistMatrix::printMatrixCSV(out);
        }

#ifdef _DEBUG_CLUST
        virtual void dump()
        {
            printf("Matrix:\n");
            for (idx x=0; x<_npoints; x++) {
                printf("%3" DEC ": ", x);
                for (idx y=0; y<_npoints; y++)
                    printf("%7.4g ", dist(x, y));
                printf("\n");
            }
            printf("Tree raw:\n");
            for (idx i=0; i<_npoints; i++) {
                printf("%" DEC ":(d:%g; i:%" DEC ") ", i, _tree[i].obj.dist, _tree[i].in[0]);
            }
            for (idx i=_npoints; i<_tree.dim(); i++) {
                printf("%" DEC ":(d:%g; i:%" DEC ", o:%" DEC ",%" DEC ") ", i, _tree[i].obj.dist, _tree[i].in[0], _tree[i].out[0], _tree[i].out[1]);
            }
            printf("\n");
            printf("Newick:\n");
            sStr s;
            _tree.printNewick(s);
            printf("%s\n", s.ptr());
        }
#endif
    };

    class sSingleLinkClustering: public sHierarchicalClustering
    {
    public:
        sSingleLinkClustering(const sDistMatrix &distMatrix): sHierarchicalClustering(distMatrix) {init(); this->buildTree();}
        sSingleLinkClustering() { _min = -1; }

    protected:
        sVec<idx> _nearest;
        idx _min;

        void init()
        {
            sHierarchicalClustering::init();

            _nearest.resize(_npoints);
            _min = -1;
            real mind = REAL_MAX;

            for (idx x=0; x<_npoints; x++) {
                _nearest[x] = -1;
                real minxd = REAL_MAX;
                if (!sDistMatrix::valid(x))
                    continue;
                for (idx y=0; y<_npoints; y++) {
                    if (x == y || !valid(y))
                        continue;
                    real d = dist(x,y);
                    if (d < minxd) {
                        _nearest[x] = y;
                        minxd = d;
                    }
    #ifdef _DEBUG_CLUST
                    printf("x=%" DEC " (nearest=%" DEC "), y=%" DEC ", min_nearest=%" DEC ", mind=%g, minxd=%g, d=%g\n", x, _nearest[x], y, _min, mind, minxd, d);
    #endif
                }
                if (minxd < mind) {
                    _min = x;
                    mind = minxd;
                }
            }
        }

        bool peekNext(idx *x, idx *nearesty)
        {
            if (_min < 0)
                return false;
            if (x != NULL)
                *x = _min;
            if (nearesty != NULL)
                *nearesty = _nearest[_min];
            return true;
        }

        void mergeNext(real *distx, real *disty)
        {
            idx x = -1, y = -1;
            peekNext(&x, &y);

            assert(valid(x));
            assert(valid(y));

            real d = dist(x,y);
            if (distx)
                *distx = d/2;
            if (disty)
                *disty = d/2;

            real mind = REAL_MAX;
            real minxd = REAL_MAX;
            _nearest[x] = _nearest[y] = _min = -1;

            for (idx z=0; z<_npoints; z++) {
                if (!valid(z) || z == x || z == y)
                    continue;

                real zxd = sMin<real>(dist(z,x), dist(z,y));
                setDist(z,x,zxd);
                if (_nearest[z] == y)
                    _nearest[z] = x;

                if (zxd < minxd) {
                    minxd = zxd;
                    _nearest[x] = z;
                }

                real minzd = dist(z, _nearest[z]);
                if (minzd < mind) {
                    mind = minzd;
                    _min = z;
                }
            }
            setInvalid(y);
        }

    public:
    #ifdef _DEBUG_CLUST
        void dump()
        {
            sHierarchicalClustering::dump();
            printf("Nearest:\n");
            for (idx x=0; x<_npoints; x++) {
                real d = _nearest[x] >= 0 ? dist(x, _nearest[x]) : NAN;
                printf("%" DEC "->%" DEC " (%g) ", x, _nearest[x], d);
            }
            printf("\nBest: %" DEC "\n\n", _min);
        }
    #endif

    };

    class sCompleteLinkClustering: public sHierarchicalClustering
    {
    public:
        typedef sMinHeap<real, sDistMatrixRowIter> Theap;

        sCompleteLinkClustering(const sDistMatrix &distMatrix): sHierarchicalClustering(distMatrix) {init(); this->buildTree();}
        sCompleteLinkClustering() { _min = -1; }

    protected:
        sVec<Theap> _nearest;
        idx _min;

        void init()
        {
            sHierarchicalClustering::init();

            _nearest.resize(_npoints);
            _min = -1;
            real mind = REAL_MAX;

            for (idx x=0; x<_npoints; x++) {
                _nearest[x].reset(rowIter(x));
                _nearest[x].remove(x);
                real d = _nearest[x].peekValue();
                if (d < mind) {
                    _min = x;
                    mind = d;
                }
            }
        }
        bool peekNext(idx *x, idx *nearesty)
        {
            if (_min < 0)
                return false;
            if (x != NULL)
                *x = _min;
            if (nearesty != NULL)
                *nearesty = _nearest[_min].peekIndex();
            return true;
        }

        virtual inline real mergedDist(idx x, idx y, idx z)
        {
            return sMax<real>(dist(z, x), dist(z, y));
        }

        virtual void mergeNext(real *distx, real *disty)
        {
            idx x = -1, y = -1;
            peekNext(&x, &y);

            assert(valid(x));
            assert(valid(y));

            real d = dist(x,y);
            if (distx)
                *distx = d/2;
            if (disty)
                *disty = d/2;

            _nearest[x].pop();
            real mind = REAL_MAX;
            _min = -1;

            for (idx z=0; z<_npoints; z++) {
                if (!valid(z) || z == x || z == y)
                    continue;

                real d = mergedDist(x, y, z);
                setDist(z, x, d);
                _nearest[x].adjust(z, d);
                _nearest[z].adjust(x, d);
                _nearest[z].remove(y);

                real minzd = _nearest[z].peekValue();

                if (minzd < mind) {
                    mind = minzd;
                    _min = z;
                }
            }

            real minxd = _nearest[x].peekValue();
            if (minxd < mind) {
                mind = minxd;
                _min = x;
            }

            setInvalid(y);
            _nearest[y].clear();
        }

    public:
    #ifdef _DEBUG_CLUST
        void dump()
        {
            sHierarchicalClustering::dump();
    #ifdef _DEBUG_HEAP
            for (idx x=0; x<_npoints; x++) {
                printf("Nearest heap for %" DEC ": ", x);
                _nearest[x].dump();
                printf("\n");
            }
    #endif
            printf("Best: %" DEC "\n\n\n", _min);
        }
    #endif

    };

    class sNeighborJoining: public sHierarchicalClustering
    {
    public:
        sNeighborJoining(const sDistMatrix &distMatrix, bool allowNegativeLength=false): sHierarchicalClustering(distMatrix), _divergence(sMex::fExactSize)
        {
            _allowNegativeLength = allowNegativeLength;
            init();
            this->buildTree();
        }
        sNeighborJoining(bool allowNegativeLength=false)
        {
            _allowNegativeLength = allowNegativeLength;
            _needMakeDivergence = _needMakeMin = true;
            _minx = _miny = -1;
            _nvalid = 0;
        }

    protected:
        sVec<real> _divergence;
        idx _minx, _miny, _nvalid;
        bool _needMakeDivergence, _needMakeMin;
        bool _allowNegativeLength;

        void makeDivergenceNValid()
        {
            _nvalid = 0;
            _needMakeDivergence = false;
            for (idx x=0; x<_npoints; x++) {
                if (!valid(x))
                    continue;
                _nvalid++;
                real sum = 0;
                for (idx y=0; y<_npoints; y++) {
                    if (!valid(y))
                        continue;
                    sum += dist(x, y);
                }
                _divergence[x] = sum;
            }
        }

        inline real getWeight(idx x, idx y)
        {
            return (_nvalid - 2) * dist(x, y) - _divergence[x] - _divergence[y];
        }

        virtual void makeMin()
        {
            real minWeight = REAL_MAX;
            _minx = _miny = -1;
            _needMakeMin = false;

            if (_nvalid < 3) {
                for (idx x=0; x<_npoints; x++) {
                    if (!valid(x))
                        continue;
                    if (_minx < 0)
                        _minx = x;
                    _miny = x;
                }
                return;
            }

            for (idx x=0; x<_npoints; x++) {
                if (!valid(x))
                    continue;
                for (idx y=0; y<x; y++) {
                    if (!valid(y))
                        continue;
                    real weight = getWeight(x, y);
#ifdef _DEBUG_CLUST
                    printf("Weight of clusters %" DEC " and %" DEC " is %g\n", x, y, weight);
#endif
                    if (weight < minWeight) {
                        minWeight = weight;
                        _minx = x;
                        _miny = y;
                    }
                }
            }
        }

        virtual void init()
        {
            sHierarchicalClustering::init();
            _divergence.resize(_npoints);
            _needMakeDivergence = _needMakeMin = true;
            _minx = _miny = -1;
            _nvalid = 0;
        }

        bool peekNext(idx *x, idx *nearesty)
        {
            if (_needMakeDivergence)
                makeDivergenceNValid();
            if (_needMakeMin)
                makeMin();

            if (_minx < 0 || _miny < 0)
                return false;
            if (x != NULL)
                *x = _minx;
            if (nearesty != NULL)
                *nearesty = _miny;
            return true;
        }

        virtual void mergeNext(real *distx, real *disty)
        {
            if (_needMakeDivergence)
                makeDivergenceNValid();
            if (_needMakeMin)
                makeMin();

            assert(valid(_minx));
            assert(valid(_miny));

            real d = dist(_minx, _miny);

            real dx = d/2 + (_nvalid > 2 ? (_divergence[_minx] - _divergence[_miny])/(2*(_nvalid - 2)) : 0);

            if (_allowNegativeLength) {
                if (distx)
                    *distx = dx;
                if (disty)
                    *disty = d - dx;
            } else {
                real dclamped = sMax<real>(0, d);
                if (distx) {
                    *distx = sMin<real>(sMax<real>(0, dx), dclamped);
                    assert(*distx >= 0);
                }
                if (disty) {
                    *disty = sMin<real>(sMax<real>(0, d - dx), dclamped);
                    assert(*disty >= 0);
                }
            }

            real divx = 0;
            for (idx z=0; z<_npoints; z++) {
                if (!valid(z) || z == _minx || z == _miny)
                    continue;
                real old_distzx = dist(z,_minx);
                real old_distzy = dist(z,_miny);
                real new_distzx = (old_distzx + old_distzy - d)/2;
                divx += new_distzx;
                _divergence[z] += new_distzx - old_distzx - old_distzy;
                setDist(z, _minx, new_distzx);
            }
            setInvalid(_miny);
            _nvalid--;
            _divergence[_minx] = divx;
            _needMakeDivergence = false;
            _needMakeMin = true;
        }
    };

    class sFastNeighborJoining: public sNeighborJoining
    {
    public:
        sFastNeighborJoining(const sDistMatrix &distMatrix, bool allowNegativeLength=false): sNeighborJoining(distMatrix, allowNegativeLength), _visible(sMex::fExactSize) {}
        sFastNeighborJoining(bool allowNegativeLength=false): sNeighborJoining(allowNegativeLength) {}

    protected:
        sVec<idx> _visible;

        idx getLightest(idx x)
        {
            real minWeight = REAL_MAX;
            idx miny = -1;
            for (idx y=0; y<_npoints; y++) {
                if (y == x || !valid(y))
                    continue;
                real weight = getWeight(x, y);
                if (weight < minWeight) {
                    minWeight = weight;
                    miny = y;
                }
            }
            return miny;
        }

        virtual void init()
        {
            sNeighborJoining::init();
            if (_needMakeDivergence)
                makeDivergenceNValid();

            _visible.resize(_npoints);
            for (idx x=0; x<_npoints; x++)
                _visible[x] = valid(x) ? getLightest(x) : -1;
        }

        void makeMin()
        {
            real minWeight = REAL_MAX;
            _minx = _miny = -1;
            _needMakeMin = false;

            if (_nvalid < 3) {
                for (idx x=0; x<_npoints; x++) {
                    if (!valid(x))
                        continue;
                    if (_minx < 0)
                        _minx = x;
                    _miny = x;
                }
                return;
            }

            for (idx x=0; x<_npoints; x++) {
                idx y = _visible[x];
                if (y < 0 || !valid(x) || !valid(y))
                    continue;

                real weight = getWeight(x, y);
#ifdef _DEBUG_CLUST
                printf("Weight of clusters %" DEC " and %" DEC " is %g\n", x, y, weight);
#endif
                if (weight < minWeight) {
                    minWeight = weight;
                    _minx = x;
                    _miny = y;
                }
            }
        }

        void mergeNext(real *distx, real *disty)
        {
            sNeighborJoining::mergeNext(distx, disty);

            for (idx x=0; x<_npoints; x++) {
                if (x == _miny || _visible[x] == _minx || _visible[x] == _miny)
                    _visible[x] = -1;
            }

            _visible[_minx] = getLightest(_minx);
        }
    };
};

#endif
