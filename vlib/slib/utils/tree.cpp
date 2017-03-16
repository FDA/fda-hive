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
#include <slib/utils/tree.hpp>
#include <ssci/math/clust/clust2.hpp>


using namespace slib;

#if 0
struct ForCallBack
{
        sTabular *tbl;
        sVec <idx> *values;
        sVec <idx> *uIDs;
        idx horizontal;
};

static idx myCallBack(sStr &out, sHierarchicalClustering &clust, idx x, void *param_)
{
    struct ForCallBack * param = static_cast<ForCallBack*>(param_);
    if (x >= param->values->dim())
        return 0;

    if (param->horizontal)
    {
        if(!param->uIDs || !param->uIDs->dim())
            param->tbl->printCell(out, (*(param->values))[x], 0);
        else {
            for( idx iu=0; iu<param->uIDs->dim(); ++iu  ) {
                if(iu)out.printf(" ");
                param->tbl->printCell(out, (*(param->values))[x], (*(param->uIDs))[iu]);
            }

        }
    }
    else
    {
        param->tbl->printCell(out, -1, (*(param->values))[x]);
    }

    return 1;
}
#endif

struct PrintNewickCallbackParam {
    sTabular * tbl;
    sVec <idx> * objToUse;
    sVec<idx> * uIDs;
    bool horizontal;
};

static idx printNewickCallback(sStr &out, sHierarchicalClustering &clust, idx x, void *param_)
{
    struct PrintNewickCallbackParam * param = static_cast<PrintNewickCallbackParam*>(param_);
    if( param->horizontal ) {
        idx ir = param->objToUse && param->objToUse->dim() ? (*param->objToUse)[x] : x;
        if( param->uIDs && param->uIDs->dim() ) {
            for(idx i = 0; i < param->uIDs->dim(); i++) {
                idx ic = (*param->uIDs)[i];
                if( i ) {
                    out.addString(" ");
                }
                param->tbl->printCell(out, ir, ic);
            }
        } else {
            out.addNum(ir);
        }
    } else {
        idx ic = param->objToUse && param->objToUse->dim() ? (*param->objToUse)[x] : x;
        if( param->uIDs && param->uIDs->dim() ) {
            for(idx i = 0; i < param->uIDs->dim(); i++) {
                idx ir = (*param->uIDs)[i];
                if( i ) {
                    out.addString(" ");
                }
                param->tbl->printCell(out, ir, ic);
            }
        } else {
            out.addNum(ic);
        }
    }

    return 1;
}

idx reorderIterate (sHierarchicalClusteringTree & tree, idx curPos, sVec <real> *averages, sVec <idx> newOrder, sVec <idx> colsToUse, sTabular * tbl)
{
    if (curPos < 0) {
        return 0;
    }

    sHierarchicalClusteringTree::Tnode & node = tree.getNodeVec()[curPos];

    if (curPos < tree.dimLeaves())
    {
        idx row = newOrder[curPos];
        real total = 0;

        for (idx i = 0; i < colsToUse.dim(); i++)
        {
            idx curCol = colsToUse[i];

            total += tbl->rval(row, curCol);
        }

        (*averages)[curPos] = total/colsToUse.dim();

        return 0;
    }

    reorderIterate (tree, node.out[0], averages, newOrder, colsToUse, tbl);
    reorderIterate (tree, node.out[1], averages, newOrder, colsToUse, tbl);

    if ((*averages)[node.out[0]] > (*averages) [node.out[1]])
    {
        sSwapI(node.out[0], node.out[1]);
    }

    (*averages)[curPos] = ((*averages)[node.out[0]] + (*averages) [node.out[1]])/2;

    return 0;
}

idx rememberReorder (sHierarchicalClusteringTree & tree, idx curPosInTree, idx curPosInVec, sVec <idx> * newOrder)
{
    if (curPosInTree < 0 || curPosInTree >= newOrder->dim()) {
        return curPosInVec;
    }

    sHierarchicalClusteringTree::Tnode & node = tree.getNodeVec()[curPosInTree];

    if (curPosInTree < tree.dimLeaves()) {
        // leaf node
        (*newOrder)[curPosInVec] = curPosInTree;
        return curPosInVec + 1;
    }

    // inner node
    curPosInVec = rememberReorder(tree, curPosInVec, node.out[0], newOrder);
    curPosInVec = rememberReorder(tree, curPosInVec, node.out[1], newOrder);

    return curPosInVec;
}

namespace {
    struct DistCallbackParam {
        sTabular * tbl;
        idx * rowSet;
        idx * colSet;
        idx rowCnt;
        idx colCnt;
        bool horizontal;
        sTree::DistanceMethods distMethod;
    };
};

static real distCallback(idx x, idx y, void * param_)
{
    const DistCallbackParam * param = static_cast<DistCallbackParam*>(param_);

    sDistAccum<real, sBufferIter<real> > * dist = 0;
    sEuclideanDistAccum<real, sBufferIter<real> > euclidean_dist;
    sManhattanDistAccum<real, sBufferIter<real> > manhattan_dist;
    sMaximumDistAccum<real, sBufferIter<real> > maximum_dist;
    sCosineDistAccum<real, sBufferIter<real> > cosine_dist;
    switch( param->distMethod ) {
        case sTree::EUCLIDEAN:
            dist = &euclidean_dist;
            break;
        case sTree::MANHATTAN:
            dist = &manhattan_dist;
            break;
        case sTree::MAXIMUM:
            dist = &maximum_dist;
            break;
        case sTree::COSINE:
            dist = &cosine_dist;
            break;
    }

    idx vec_dim = param->horizontal ? param->colCnt : param->rowCnt;

    idx ir1 = 0, ir2 = 0, ic1 = 0, ic2 = 0;
    if( param->horizontal ) {
        if( param->rowSet ) {
            ir1 = param->rowSet[x];
            ir2 = param->rowSet[y];
        } else {
            ir1 = x;
            ir2 = y;
        }
    } else {
        if( param->colSet ) {
            ic1 = param->colSet[x];
            ic2 = param->colSet[y];
        } else {
            ic1 = x;
            ic2 = y;
        }
    }

    for(idx i = 0; i < vec_dim; i++) {
        if( param->horizontal ) {
            if( param->colSet ) {
                ic1 = ic2 = param->colSet[i];
            } else {
                ic1 = ic2 = i;
            }
        } else {
            if( param->rowSet ) {
                ir1 = ir2 = param->rowSet[i];
            } else {
                ir1 = ir2 = i;
            }
        }
        dist->accum(param->tbl->rdiff(ir1, ic1, ir2, ic2), 0);
    }

    return dist->result();
}

#define BENCH_TREE 1

#ifdef BENCH_TREE
#define print_bench(fmt, ...) fprintf(stderr, "%s:%u %.2g sec (%.2g CPU) " fmt "\n", __FILE__, __LINE__, wall_clock.clock(0, 0, true), cpu_clock.clock(), ##__VA_ARGS__)
#else
#define print_bench(fmt, ...)
#endif

idx sTree::generateTree (sStr & out,sVec < idx > * columnsToUse, sVec <idx > * rowsToUse, sTabular * tbl, sVec <idx> * newOrder, idx horizontal, sVec < idx > * uIDs , DistanceMethods distMethod, neighborJoiningMethods jMethod)
{
#ifdef BENCH_TREE
    sTime cpu_clock, wall_clock;
    wall_clock.clock(0, 0, true);
    cpu_clock.clock();
#endif

    sVec <idx> * objToUse = horizontal ? rowsToUse : columnsToUse;

    //idx colCnt=(columnsToUse && columnsToUse->dim()) ? columnsToUse->dim() : tbl->cols();//! the total number of columns to go through

    DistCallbackParam dist_cb_param;
    dist_cb_param.tbl = tbl;
    dist_cb_param.rowSet = rowsToUse && rowsToUse->dim() ? rowsToUse->ptr() : 0;
    dist_cb_param.colSet = columnsToUse && columnsToUse->dim() ? columnsToUse->ptr() : 0;
    dist_cb_param.rowCnt = rowsToUse ? rowsToUse->dim() : tbl->rows();
    dist_cb_param.colCnt = columnsToUse ? columnsToUse->dim() : tbl->cols();
    dist_cb_param.horizontal = horizontal;
    dist_cb_param.distMethod = distMethod;

    idx cnt = horizontal ? dist_cb_param.rowCnt : dist_cb_param.colCnt; // count of objToUse

    print_bench("to setup %" DEC " items (%s)", cnt, horizontal ? "horizontal" : "vertical");

    sHierarchicalClustering * nj;
    if( jMethod==FAST ) {
        nj = new sFastNeighborJoining();
    } else {
        nj = new sNeighborJoining ();
    }

    nj->resetDistance(cnt, distCallback, &dist_cb_param);
    print_bench("to make dist matrix");
    nj->recluster();
    print_bench("to cluster using %s", jMethod==FAST ? "FNJ" : "NJ");

#ifdef DEBUG_TREE
    fprintf(stderr, "Distance matrix:\n");
    sStr distbuf;
    nj.printMatrixCSV(distbuf);
    fprintf(stderr, "%s\n", distbuf.ptr());

    sFil heatmapCSV("distmatrix.csv");

    nj.printMatrixCSV(heatmapCSV);

#endif

    sVec<idx> leafOrder;
    nj->getTree().getLeaves(leafOrder);

    print_bench("to get leaves");

    newOrder->resize(leafOrder.dim());
    for (idx i=0; i < leafOrder.dim(); i++)
    {
        idx newPos=leafOrder[i];
        (*newOrder)[i]=(*objToUse)[newPos];
    }
    print_bench("to make new leaf order");

#if 0
    sVec <real> perRowAverage;
    if (horizontal)
    {
        sHierarchicalClusteringTree & tree = nj->getTree();

        perRowAverage.resize(tree.dim());
        //sHierarchicalClusteringTree root = tree.getNodeVec();
        reorderIterate (tree, &tree.getRoot() - tree.getNodeVec().ptr(), &perRowAverage, *newOrder, *columnsToUse, tbl);

        //rememberReorder (tree,  &tree.getRoot() - tree.getNodeVec().ptr(), 0, newOrder);

        sVec<idx> leafOrder;
        tree.getLeaves(leafOrder);

        //newOrder->resize(leafOrder.dim());

        for (idx i=0; i < leafOrder.dim(); i++)
        {
            idx newPos=leafOrder[i];
            (*newOrder)[i]=(*objToUse)[newPos];
        }
    }
    print_bench("to make averages");


    ForCallBack obj;

    obj.tbl = tbl;
    obj.values = newOrder;
    obj.uIDs=uIDs;
    obj.horizontal = horizontal;

    nj->printNewick(out,sHierarchicalClusteringTree::Newick_PRINT_DISTANCE|sHierarchicalClusteringTree::Newick_PRINT_LEAF_NAMES,myCallBack,&obj);
#endif

    PrintNewickCallbackParam print_newick_callback_param;
    print_newick_callback_param.tbl = tbl;
    print_newick_callback_param.objToUse = objToUse;
    print_newick_callback_param.uIDs = uIDs;
    print_newick_callback_param.horizontal = horizontal;

    nj->printNewick(out, sHierarchicalClusteringTree::Newick_PRINT_DISTANCE|sHierarchicalClusteringTree::Newick_PRINT_LEAF_NAMES, printNewickCallback, &print_newick_callback_param);

    print_bench("to print Newick");

    return 0;
}



