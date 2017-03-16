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
#ifndef sMath_clust_h
#define sMath_clust_h

#include <slib/core/sIO.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/net.hpp>
#include <slib/core/dic.hpp>

#include <ssci/math/objects/matrix.hpp>


namespace slib
{
    enum sClust_DISTANCE{
        sClust_DISTANCE_EUCLIDIAN=0,
        sClust_DISTANCE_CITIBLOCK,
        sClust_DISTANCE_MINIMAX,
        sClust_DISTANCE_MINKOVSKI,
        //sClust_DISTANCE_QUADRATIC,
        sClust_DISTANCE_CANBERRA,
        sClust_DISTANCE_PEARSON,
        sClust_DISTANCE_PEARSONUNCENTERED,
        sClust_DISTANCE_PEARSONSQUARED,
        sClust_DISTANCE_SPEARMAN,
        //sClust_DISTANCE_COSINE,
        sClust_DISTANCE_GIVEN
    };

    enum sClust_LINKAGE{
        sClust_LINKAGE_MIN=0,
        sClust_LINKAGE_MAX,
        sClust_LINKAGE_AVERAGE_UNWEIGHTED,
        sClust_LINKAGE_AVERAGE_WEIGHTED,
        sClust_LINKAGE_CENTROID,
        sClust_LINKAGE_MEDIAN,
        sClust_LINKAGE_WARD,

        sClust_LINKAGE_NEIGHBORJOINING,

    };

    #define sClust_DOWN -1
    #define sClust_NEXT -2
    #define sClust_UP   -3



    class sClust : public sVec< sKnot <real> >
    {

    public:
        //static vLix lix;
        idx baseDim;

        typedef sVec< idx >  Queue;
        struct NodeLoc{ real cy,sy,fy,sx,fx;idx par;} ;

    public:
        static void debugPrintArr(idx dim, real * array, idx * map);
        static real * computeDistanceMatrix(idx transpose , idx method, idx rowcnt, idx colcnt, real * actmat, real * array );
    public:
        static void clusterMethod(const char * format, idx * pDstMethod, idx * pClsMethod);

    public:
        typedef idx (*NodePrintfCallback)(sStr * out, sClust * tree, idx inode, void * par);

        sClust(): sVec< sKnot<real> >()
        {
        }

        void clusterLinkage(idx method, idx dim , real * array );
        void clusterHierarchically(idx transpose, idx distmethod, idx clustmethod, idx rows, idx cols, real * actmat );
        void flattenHierarchy(Queue * res, idx direction);
        void printNewick(sStr * out, NodePrintfCallback func, void * par, idx dodist);
        void printListed(sStr * out, NodePrintfCallback func, void * par, idx dodist);
        void positionTree(NodeLoc * loc,NodeLoc * limits);

        //vClr * bitmap( idx cx, idx cy ) ;

        ///void clusterKMeans(idx doBinary, idx iter, real convMax, idx cntclust, idx rows, idx cols, real * actmat );
    };
}

#endif // sMath_clust_h





