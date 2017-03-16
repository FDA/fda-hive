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
#ifndef sMath_textclust_h
#define sMath_textclust_h

#include <slib/core/sIO.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/net.hpp>
#include <slib/core/dic.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/math/objects/matrix.hpp>

#define PROGRESS(items,cur,max) (m_callback && !m_callback(m_callbackParam, items, ((cur) * 1.0 / (max)) * 100, 100))

namespace slib
{
    class sTextClust
    {
        private:
            enum {
                sTextTable = 0,
                sTextHiveseq
            };

            struct individual
            {
                    idx id;
                    idx cluster;
            };
            struct centroid
            {
                    idx id;
                    idx count;
                    idx offset;
                    sVec < sVec<idx> > representation;
                    sStr strRep;
//                    void (*func) (sStr * str, sVec < sVec<idx> > *rep);
            };

            sVec <centroid> Clusters;
            sVec <individual> Population;
            idx  srcType;
            sBioseq *SrcFile;
            sTxtTbl *SrcTabFile;
            bool printInfo;
            sStr *infoContainer;

            void switchPop (idx i1, idx i2);
            bool pop2cluster (idx iClust, idx iPop, idx offset);
            bool addNewCluster (idx iPop, bool isTemporary = false);
            bool addExistingCluster (idx iPop, idx iCluster);
            idx findClosestCluster (idx iPop, real tolerance, bool useMatrixDistance = false);
            idx getValue (char c);
            char getCharacter (idx i);
            real distance (const char * t1, const char * t2, idx len, real missmatches = 0);
            real matrixDistance (const char * t1, const char * t2, idx len);
            idx textComparison (idx index1, idx index2, idx len, bool useMatrix = false, bool useOffsets = false);
            idx extractRepresentation (sStr &str, idx iClust, sVec<idx> *iPositions = 0);

        protected:

            idx initial_num_population;
            idx num_population;
            idx popRows;
            idx popCols;

            idx num_clusters;
            idx num_alphabet;
            idx stringLength;
            sStr alphabet00;
            sDic <idx> alphaDic;

            idx * external_mat;
            idx external_dim;
            bool matrixMinimization;
            idx matrixMaximum;
            idx matrixMinimum;

            void init (idx pop = 0, idx clust = 0){
                if (pop > 0){
                    initial_num_population = pop;
                }
                num_population = initial_num_population;
                num_clusters = clust;
                Clusters.mex()->flags |= sMex::fSetZero;
                Clusters.cut(0);
                stringLength = sIdxMax;
                external_mat = 0;
                external_dim = 0;
                matrixMinimization = true;
                matrixMaximum = 0;
                matrixMinimum = 0;
                printInfo = false;
                infoContainer = 0;
                initPopulation (initial_num_population);
            }

            bool initCluster (idx numClusters)
            {
                num_clusters = numClusters;
                Clusters.cut(0);
                stringLength = sIdxMax;
                for (idx i = 0; i< num_population; ++i){
                    Population[i].id = i;
                    Population[i].cluster = -1;
                }
                return false;
            }

        public:
            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);
            callbackType m_callback;
            void * m_callbackParam;

            idx getString (sStr &buf, idx iPop);
            sTextClust():m_callback(0), m_callbackParam(0)
            {
                initial_num_population = 0;
                init();
            }
            sTextClust(sBioseq *sHive) : m_callback(0), m_callbackParam(0)
            {
                initial_num_population = 0;
                initPopulation(sHive);
                init();
            }

            idx clusterPopulation ()
            {
                return num_population;
            }

            void addInfo(sStr &info){
                infoContainer = &info;
//                infoContainer.add(info, sLen(info));
//                infoContainer.add0();
                printInfo = true;
            }

            void setMatrixParam (bool matrixMin, idx maximum, idx minimum){
                matrixMinimization = matrixMin;
                matrixMaximum = maximum;
                matrixMinimum = minimum;
            }

            bool setInitialPopulation (idx popSize)
            {
                initial_num_population = popSize;
                return true;
            }

            idx getCountCluster (idx id)
            {
                return Clusters.ptr(id)->count;
            }

            idx getNumCluster ()
            {
                return num_clusters;
            }

            idx addTempCluster (idx iCluster)
            {
                Clusters.cut(num_clusters);
                addNewCluster (Clusters.ptr(iCluster)->id, true);
                num_clusters -= 1;
                mergeClusters (iCluster, num_clusters);
                return num_clusters;

            }

            bool resetCluster(idx clustSize = 0)
            {
//                if (!popSize && srcType == sTextHiveseq){
//                    popSize = SrcFile->dim();
//                }
//                else if (srcType == sTextTable){
////  Missing the size from the table
//                    // popSize = ?;
//                }
                initCluster (clustSize);
                return true;
            }
            idx runCluster (real tolerance, bool useMatrixDistance = false)
            {
                // Add the first element to the cluster
                addNewCluster (0);

                // Add the rest elements to the cluster

                idx center = 0;
                for (idx i = 1; i < num_population; ++i){
             //        rand = (RANDREAL1*num_population);
                    if (PROGRESS(i, i, num_population)){
                        return -1;
                    }

                    center = findClosestCluster (i, tolerance, useMatrixDistance);
                    if (center < 0){
                        addNewCluster (i);
                    }
                    else {
                        addExistingCluster (i, center);
                    }

                 }
                 return 0;
             }

            idx loadClusterwithExistingPopulation (void)
            {
                // Add the first element to the cluster
                for (idx i = 0; i < num_population; ++i){
                    addNewCluster (i);
                }
                return num_population;
            }

            void addClusterOffset(idx iClust, idx offset){
                Clusters[iClust].offset = offset;
            }

//            bool loadAlphabet(const char * abc){
//                alphabet00.cut(0);
//                alphabet00.printf ("%s",abc);
//                alphaDic.parse00(alphabet00 , alphabet00.length());
//                return true;
//            }
//
            bool loadMatrix (idx *mat, idx dim)
            {
                external_mat = mat;
                external_dim = dim;
                return true;
            }

            bool loadAlphabet(sTxtTbl * table)
            {
                num_alphabet = table->rows();
                alphabet00.cut(0);
                for(idx i = 0; i < num_alphabet; ++i) {
                    table->printCell(alphabet00, i, -1, 0);
                    alphabet00.add0(1);
                }
                alphaDic.parse00(alphabet00 , alphabet00.length());
                return true;
            }

            bool loadAlphabet(const char *alph, idx alphlen = 0)
            {
                if (!alphlen){
                    alphlen = sLen (alph);
                }
                num_alphabet = alphlen;
                alphabet00.cut(0);
                for (idx i=0; i < alphlen; ++i){
                    alphabet00.printf("%c", alph[i]);
                    alphabet00.add0(1);
                }
                alphaDic.parse00(alphabet00 , alphabet00.length());
                return true;
            }

            void initPopulation(idx pop){
                num_population = pop;
                Population.mex()->flags |= sMex::fSetZero;
                Population.cut(0);
                Population.resize(num_population);
                for (idx i = 0; i< num_population; ++i){
                    Population[i].id = i;
                    Population[i].cluster = -1;
                }
            }

            bool initPopulation(sBioseq *sHive){
                this->srcType = sTextHiveseq;
                SrcFile = sHive;
                this->initial_num_population = SrcFile->dim();
                initPopulation (this->initial_num_population);
                return true;
            }

            bool initPopulation(sTxtTbl * sTab)
            {
                srcType = sTextTable;
                SrcTabFile = sTab;
                initial_num_population = SrcTabFile->dim();
                initPopulation (this->initial_num_population);
                return true;

            }

            bool exportCluster (idx *matrix, bool useMatrix = false, bool useOffsets = false);
            bool reportPopulationInfo (sStr &out);
            bool reportClusterInfo (sStr &out);
            bool reportClusterDistanceMatrix (sStr &out, idx *pmat);
            bool reportHeatmapInfoGen (sStr &out);
            bool reportAnnotationMap (sStr &out, bool printHeader = true);
            bool reportHeatmapInfoDetail (sStr &out);
            const char *stringConsensus (sStr &out, idx iClust);
            bool mergeClusters (idx iClustOrig, idx iClustDest);
            idx getMinNumberCols();
    };
}
#endif // sMath_textclust_h
