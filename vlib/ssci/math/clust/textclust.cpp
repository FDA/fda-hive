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
#include <ssci/math/clust/textclust.hpp>

using namespace slib;
#define RANDREAL1 (real)rand()/RAND_MAX

idx sTextClust::extractRepresentation (sStr & str, idx iClust, sVec <idx> * iPositions)
{
    centroid *clu = Clusters.ptr(iClust);

    str.cut(0);
    idx imax, maxValue;
    idx len = clu->representation.dim();
    idx *addPtr = 0;
    if (iPositions){
        addPtr = iPositions->add(len);
    }
    for (idx i = 0; i < len; ++i){
        imax = 0;
        maxValue = 0;
        for (idx j = 0; j < num_alphabet; ++j){
            if (clu->representation[i][j] > maxValue){
                maxValue = clu->representation[i][j];
                imax = j;
            }
        }
        str.printf("%c", getCharacter(imax));
        if (addPtr){
            addPtr[i] = imax;
        }
    }
    str.add0(1);
    return len;
}

void sTextClust::switchPop (idx i1, idx i2)
{
    idx idtemp = Population[i1].id;
    idx cltemp = Population[i1].cluster;
    Population[i1].id = Population[i2].id;
    Population[i1].cluster = Population[i2].cluster;
    Population[i2].id = idtemp;
    Population[i2].id = cltemp;
}

// Get the index value of the character in the alphabet
idx sTextClust::getValue (char c)
{
    return *alphaDic.set(&c, 1);
}

// Get the character value in the alphabet based on the index
char sTextClust::getCharacter (idx i)
{
    idx sizeID;
    const char * idl = (const char *)alphaDic.id(i,&sizeID);
    return idl[0];
}

bool sTextClust::addNewCluster (idx iPop, bool isTemporary)
{
    sStr rep;
    rep.cut(0);
    idx len = getString (rep, iPop);
    stringLength = stringLength < len ? stringLength: len;
    if (!isTemporary){
        Population[iPop].cluster = num_clusters;
    }

    centroid *clu = Clusters.add(1);
    clu->id = Population[iPop].id;
    clu->count = 1;
    clu->offset = -1;
    clu->representation.mex()->flags |= sMex::fSetZero;
    clu->representation.add(len);
    for(idx ic = 0; ic < len; ++ic) {
        clu->representation.ptr(ic)->add(num_alphabet);
        clu->representation.ptr(ic)->set(0);
    }
    char let;
    clu->strRep.cut(0);
    for (idx i = 0; i < len; ++i){
        let = getValue (rep[i]);
        ++(clu->representation[i][let]);
        clu->strRep.printf("%c", getCharacter(let));
    }
//    extractRepresentation (&(clu->strRep), &(clu->representation));
//    Clusters[iClust].representation.printf("%s",rep);
    // Switch with the last one
    num_clusters++;
    return true;
}

bool sTextClust::addExistingCluster (idx iPop, idx iClust)
{
    sStr t1;
    idx len1 = getString (t1, iPop);
    idx let;

    Population.ptr(iPop)->cluster = iClust;

    // Modify the count and the representation count
    Clusters.ptr(iClust)->count += 1;
    centroid *clu = Clusters.ptr(iClust);
    idx length = len1 > clu->representation.dim() ? clu->representation.dim() : len1;
    idx maxletter;
    for (idx i = 0; i < length; ++i){
        let = getValue (t1[i]);
        ++(clu->representation[i][let]);
        // updates representation letter by letter
        // let is the new letter
        maxletter = getValue(clu->strRep[i]);
        if ((let != maxletter) && (clu->representation[i][let]) > (clu->representation[i][maxletter])){
//            char *consensus = clu->strRep;
//            consensus[i] = getCharacter(let);
            *(clu->strRep.ptr(i)) = getCharacter(let);
        }
        // clu->representation[i][]
    }
//    if ((clu->count % 200) == 0){
//        extractRepresentation (clu->strRep, iClust);
//    }
    return true;
}

bool sTextClust::mergeClusters (idx iClustOrig, idx iClustDest)
{
    idx len = stringLength;

    centroid *cluOrig = Clusters.ptr(iClustOrig);
    centroid *cluDest = Clusters.ptr(iClustDest);

    for (idx i = 0; i < len; ++i){
        for (idx j = 0; j < num_alphabet; ++j){
            if (cluOrig->representation[i][j] > 0){
                (cluDest->representation[i][j]) += cluOrig->representation[i][j];
            }
        }
    }
    return true;

}

idx sTextClust::getString (sStr &buf, idx iPop)
{
    idx len1 = 0;
    if ( srcType == sTextHiveseq){
        const char *seq1 = SrcFile->seq(iPop);
        len1 = SrcFile->len(iPop);
        buf.resize(len1);
        sBioseq::uncompressATGC_2Bit(buf.ptr(), seq1, 0, len1);
    }
    else if (srcType == sTextTable){
        SrcTabFile->printCell (buf, iPop, 0, 0);
        len1 = buf.length();
    }
    return len1;
}

idx sTextClust::findClosestCluster (idx iPop, real tolerance, bool useMatrixDistance)
{

    sStr t1, t2;
    idx len1 = getString (t1, iPop);
    idx id, len2;
    real dist;
    real min_distance = sIdxMax;
    idx iminDist = -1;

    for (idx i = 0; i < num_clusters; ++i){
        t2.cut(0);
        if (Clusters.ptr(i)->strRep.length() != 0){
            // useConsensus
            t2.printf("%s", Clusters.ptr(i)->strRep.ptr());
            len2 = Clusters.ptr(i)->representation.dim();
        }
        else {
            id = Clusters.ptr(i)->id;
            len2 = getString(t2, id);
        }
        if (useMatrixDistance){
            dist = matrixDistance (t1, t2, (len1 < len2) ? len1 : len2);
        }
        else{
            dist = distance (t1, t2, (len1 < len2) ? len1 : len2, tolerance);
        }
        if ((dist < tolerance) && (dist < min_distance)){
            // Return index of the cluster
            min_distance = dist;
            iminDist = i;
        }
    }
    // Could not find any cluster
    return iminDist;
}

bool sTextClust::pop2cluster (idx iClust, idx iPop, idx offset)
{
    Clusters[iClust].id = Population[iPop].id;
    Clusters[iClust].count = 1;
    Clusters[iClust].offset = offset;
    sStr rep;
    rep.cut(0);
//    idx len = getString (&rep, iPop);

//    Clusters[iClust].representation.printf("%s",rep);
    // Switch with the last one
    return true;
}

bool sTextClust::exportCluster(idx *pmat, bool useMatrix, bool useOffsets)
{
    idx im = 0;

    for (idx i = 0; i < num_clusters; ++i){
        for (idx j = 0; j < num_clusters; ++j){
            pmat[im] = textComparison(i, j, 0, useMatrix, useOffsets);
//            if (pmat[im] == 0 && i != j){
//                pmat[im] = 50;
//            }
            im++;
        }
    }
    return true;
}

real sTextClust::distance (const char * t1, const char * t2, idx len, real tolerance)
{
    if (len == 0){
        return -1;
    }
    real t_diff = 0;
    real tol = (tolerance * len) / 100.0;
    for(idx i = 0; i < len; ++i) {
        t_diff += (t1[i] == t2[i]) ? 0 : 1;
        if ((tol != 0) && (t_diff > tol)){
            break;
        }
    }
    t_diff = (t_diff * 100.0) / len;
   return t_diff;
}

real sTextClust::matrixDistance (const char * t1, const char * t2, idx len)
{
    if (!external_mat){
        return 0;
    }
    real t_diff = 0;
    for(idx i = 0; i < len; ++i) {
        t_diff += (idx) external_mat[t1[i] * external_dim + t2[i]];
    }
    t_diff /= len;
//    if (!matrixMinimization){
//        idx value, normalization;
//        for(idx i = 0; i < len; ++i) {
//            value = (idx) external_mat[t1[i] * external_dim + t2[i]];
//            normalization = (real)((value - matrixMinimum) * 100.0) / (matrixMaximum - matrixMinimum);
//            t_diff += normalization;
//        }
//        // t_diff is a value between 0 and 100*len
//        t_diff = 100 - (real)(t_diff* 100.0) / (len*100.0);
//    }
    return t_diff;
}

idx sTextClust::textComparison (idx index1, idx index2, idx len, bool useMatrix, bool useOffsets)
{
    idx ix1 = Clusters.ptr(index1)->id;
    idx ix2 = Clusters.ptr(index2)->id;
    sStr t1, t2;
    idx len1 = getString (t1, ix1);
    idx len2 = getString (t2, ix2);
    idx length = (len1 < len2) ? len1 : len2;
    if (len){
        // try to use the len given
        length = (len < length) ? len : length;
    }
    idx t_diff = 0;
    if (useOffsets){
        // We need to calculate the part in which both clusters intersect
        // two intervals [ofs1, ofs1+len1] and [ofs2, ofs2+len2]
        idx ofs1 = Clusters.ptr(index1)->offset;
        idx ofs2 = Clusters.ptr(index2)->offset;
        idx start, newlength;
        const char *newt1, *newt2;
        if (ofs1 != -1 && ofs2 != -1){
            if (ofs1 == -1){
                ofs1 = 0;
            }
            if (ofs2 == -1){
                ofs2 = 0;
            }
            // The intersection of two intervals [s1, s2] and [t1, t2] is empty if and only if:
            // t2 < s1 or s2 < t1
            if ((ofs2 + len2 < ofs1) || (ofs1 + len1 < ofs2)) {
                return 100;
            }
            // The intersection of [s1,s2] and [t1, t2] is:
            // [max(s1, t1), min(s2,t2)]
            start = sMax (ofs1, ofs2);
            newlength = sMin(ofs1 + len1, ofs2 + len2) - start;
            newt1 = t1.ptr(start - ofs1);
            newt2 = t2.ptr(start - ofs2);
            t_diff = distance (newt1, newt2, newlength);
            return t_diff;
        }
    }
    // returns the differences of t1 and t2 based on the matrix (_mat),
    if( useMatrix ) {
        t_diff = matrixDistance (t1, t2, length);
    }
    else {
        t_diff = distance (t1, t2, length);
    }
    return t_diff;
}

const char * sTextClust::stringConsensus (sStr &out, idx iClust)
{
    if (iClust > num_clusters){
        return 0;
    }
//    getString (&out, Clusters.ptr(iClust)->id);
    extractRepresentation(out, iClust);

    return out.ptr();

}

bool sTextClust::reportPopulationInfo (sStr &out)
{
    out.cut(0);
    //out.printf("# There are: %"DEC" clusters in total \n", num_clusters);
    //out.printf("# Population size: %"DEC" \n", num_population);
    out.printf("row,cluster,id%s\n", printInfo ? ",info": 0);
    sStr t1;

    for (idx i = 0; i< num_population; ++i){
        t1.cut(0);
        getString(t1, i);
        out.printf ("%"DEC, i+1);
        out.printf(",%"DEC",%s", Population.ptr(i)->cluster, t1.ptr());
        if (printInfo && infoContainer){
            out.printf (",%s", sString::next00(infoContainer->ptr(0), i));
        }
        out.addString("\n", 1);

    }
    return true;

}

bool sTextClust::reportClusterInfo (sStr &out)
{
    out.printf("# There are: %"DEC" clusters \n", num_clusters);
    out.printf("population index, count, info \n");
    sStr aux;
    for (idx i = 0; i< num_clusters; ++i){
        aux.cut(0);
        out.printf ("%"DEC", %"DEC", %s\n", i, Clusters.ptr(i)->count, stringConsensus(aux, i));
    }
    return true;
}

bool sTextClust::reportClusterDistanceMatrix (sStr &out, idx *pmat)
{
    out.printf("# There are: %"DEC" clusters \n ,", num_clusters);
    for(idx i = 0; i < num_clusters; ++i) {
        out.printf("%"DEC",", i+1 );
    }
    *out.ptr(out.length()-1) = '\n';
    idx im = 0;
    for(idx i = 0; i < num_clusters; ++i) {
        out.printf("%"DEC",", i+1 );
        for(idx j = 0; j < num_clusters; ++j) {
            out.printf("%"DEC",", pmat[im++]);
        }
        *out.ptr(out.length()-1) = '\n';
    }
    return true;
}

bool sTextClust::reportHeatmapInfoDetail (sStr &out)
{
    // print Header
    out.printf("let");
    for (idx i = 0; i < num_alphabet; ++i){
        out.printf(",%c", getCharacter(i));
    }
    out.printf("\n");
    centroid *clu;
    real val;
    // print Rows
    for (idx i = 1; i < 2; ++i){
        clu = Clusters.ptr(i);
//        out.printf("cluster_%"DEC, i);
        for (idx j = 0; j < stringLength; ++j){
            out.printf("pos_%"DEC, j);
            for (idx k = 0; k < num_alphabet; ++k){
                val = (clu->representation[j][k] != 0) ? ((real)clu->representation[j][k] / (real)clu->count) : 0;
                if (val < 0.001){
                    out.printf(",0");
                }
                else {
                    out.printf(",%.3lf", val);
                }
            }
            out.printf("\n");
        }
        out.printf("\n");
    }

    return true;
}

bool sTextClust::reportHeatmapInfoGen (sStr &out)
{
    // print Header
    out.printf("cluster");
    for (idx i = 0; i < stringLength; ++i){
        out.printf(",pos_%"DEC, i);
    }

    out.printf("\n");
    sVec <idx> tpos;
    centroid *clu;
    real val;
    sStr buf;
    // print Rows
    sStr aux;
    for (idx i = 0; i < num_clusters; ++i){
        aux.cut(0);
        clu = Clusters.ptr(i);
        stringConsensus(aux, i);
        out.printf("\"%"DEC",%s (%"DEC")\"", i, aux.ptr(), getCountCluster(i));

//        out.printf("%"DEC, i);
//        out.printf("cluster_%"DEC, i);
        tpos.cut(0);
        extractRepresentation (buf, i, &tpos);
        for (idx j = 0; j < stringLength; ++j){
            val = (clu->representation[j][tpos[j]] != 0) ? ((real)clu->representation[j][tpos[j]] / (real)clu->count) : 0;
            if (val < 0.001){
                out.printf(",0");
            }
            else {
                out.printf(",%.3lf", val);
            }
        }
        out.printf("\n");
    }

    return true;
}

bool sTextClust::reportAnnotationMap (sStr &out, bool printHeader)
{
    sStr outAux;
    // print Header
    if (printHeader){
        out.printf("ref,start,end,idType-id\n");
    }
    sStr aux;
    for (idx i = 0; i < num_clusters; ++i){
        aux.cut(0);
        centroid *clu = Clusters.ptr(i);
        idx repLen = getString(aux, i);

//        if (printInfo && infoContainer){
//            out.printf ("\"%s\"", );
//        }
        outAux.printf(0,"%"DEC",%"DEC",%"DEC",\"id:%s;seq:%.*s\"\n", i+1, clu->offset+1, clu->offset+1 + repLen, sString::next00(infoContainer->ptr(0), i), (int)repLen, aux.ptr(0));
        out.add(outAux, outAux.length());
//        sString::escapeForCSV(out, outAux, outAux.length());
//        out.printf(",%"DEC":%.*s\n", i+1, );
    }
    return true;
}
// Expensive function that should be avoided
idx sTextClust::getMinNumberCols()
{
    idx minCols = sIdxMax;
    if ( srcType == sTextHiveseq){
        idx len1;
        for (idx i = 0; i < num_population; ++i){
            len1 = SrcFile->len(i);
            if (len1 < minCols){
                minCols = len1;
            }
        }
    }
    else if (srcType == sTextTable){
        idx len1;
        sStr buf;
        for (idx i = 0; i < num_population; ++i){
            buf.cut(0);
            SrcTabFile->printCell (buf, i, 0, 0);
            len1 = buf.length();
            if (len1 < minCols){
                minCols = len1;
            }
        }
    }
    return minCols;
}
