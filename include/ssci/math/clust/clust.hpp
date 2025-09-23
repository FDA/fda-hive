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
        sClust_DISTANCE_CANBERRA,
        sClust_DISTANCE_PEARSON,
        sClust_DISTANCE_PEARSONUNCENTERED,
        sClust_DISTANCE_PEARSONSQUARED,
        sClust_DISTANCE_SPEARMAN,
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


    };

    class sRlda {
    public:
        sVec< real> ldaTransformVals;
        sMatrix ldaTransformVecs;
        real regulAlphaStart,regulAlphaEnd;
        idx bootStrapCounter;
        real bootStrapFraction;
        real curShannon;
        idx (*callbackProgress) (void * param, idx items, idx progress, idx progressMax);
        void * callbackParam;
        bool PCAMode;

        sRlda(){
            regulAlphaStart=0.01;
            regulAlphaEnd=0.0001;
            pFDAfuncList=0;
            missingValuesTreatment=0;
            bootStrapCounter=1;
            bootStrapFraction=0.5;
            curShannon=0;
            PCAMode=0;
            callbackProgress=0;
            callbackParam=0;
        }



        static idx compute(sMatrix & ori , sDic < sVec < idx >  > & grpset, sVec< real> * ldaTransformVals, sMatrix * ldaTransformVecs, real regulAlpha, idx scaleType=1);
        idx compute(sMatrix & ori , sDic < sVec < idx >  > & grpset, idx scaleType=1){return compute(ori , grpset, &ldaTransformVals, &ldaTransformVecs, regulAlphaStart,scaleType);}
        sMatrix * remap(sMatrix & rsltMat, sMatrix & ori, sMatrix  * vecs=0) {
            if(!vecs)vecs=&ldaTransformVecs;
            rsltMat.resize(ori.rows(),ori.cols());
            sAlgebra::matrix::pcaReMap(rsltMat.ptr(0,0),ori.ptr(0,0), ori.cols(), ori.rows(), vecs->ptr(0,0));
            return &rsltMat;
        }
        void finalCompute(sMatrix & OriMat,  sDic < sVec < idx > > & rowsToUseDic, idx cntSample, idx topChoiceForFinal, bool onlyExtract=false);

        void empty(void) {
            ldaTransformVals.empty();
            ldaTransformVecs.empty();
        }
        idx bootstrap(sMatrix & mat, idx maxIter, idx squeezeSize, real importantLDA, idx randSeed, sDic < sVec < idx > > & catSet, sDic < sVec < idx > > & checkSet, sIO * gLog, sStr * flda, sStr * cat , sStr * rslfCSV, sDic <idx > * rids, sDic < idx > * cids);



        sMatrix SubMatrix,samplingLdaTransforVecCumulator;
        sVec< real> distributionPerRow,distributionPerCol;
        sVec < real > samplingLdaTransforValCumulator,totalVals,samplingLdaTransforValCumulatorOrder;
        sVec < idx > samplingLdaTransforOccurence;
        sVec < idx > samplingSet;

        enum eSamplingStategy{
            eSamplingUniform=0,
            bSamplingMonteCarlo=0x01,
            bSamplingRadioactive=0x02,
        };
        idx samplingStrategy;

        class FDAStruc;
        typedef real (*functionSpaceFunction) (sMatrix * orimat , idx irow, idx icol,FDAStruc * param, void * vars, sStr * out);
        typedef void (*functionSpaceInitalizer) (FDAStruc * fp, idx icol, void * variables);
        struct FDAStruc {
            const char * name;
            idx universe;
            real universeScaling;
            functionSpaceFunction funcCall;
            functionSpaceInitalizer initCall;
            idx varSize;
            idx funcClass;
            void * funcParam;


            idx classVarOfs,universeBase;

        };

        sDic < FDAStruc > FDAfuncList;
        FDAStruc * pFDAfuncList;
        idx cntFDAList,samplingSetSize,samplingSetDimAvailable;
        idx totalItersRun;
        sMex FDAvariablePool;
        idx missingValuesTreatment;
        idx prepareComputeExtraLarge(sMatrix & OriMat, idx rowsToUseDicDim, idx cntNonZerosMinMax=0);
        idx computeExtraLarge(sMatrix & OriMat,  sDic < sVec < idx > > & rowsToUseDic, idx cntSample, idx topChoiceForFinal, idx iterMax, idx iterMin,real shannonThreshold, idx goodShannonMaxThreshold, bool useEValScaling, real KTemperature=0, real radioactiveDecay=0, idx maxMissFire=0);
    };
}

#endif 




