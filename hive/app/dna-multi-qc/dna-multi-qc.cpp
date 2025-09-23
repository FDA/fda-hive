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
#include <qlib/QPrideProc.hpp>
#include <math.h>
#include <violin/violin.hpp>
#include <qpsvc/dna-qc.hpp>

class DnaMQCProc : public sQPrideProc
{
public:
    DnaMQCProc(const char *defline00, const char *srv)
        : sQPrideProc(defline00, srv), m_progressLast(0), m_progressUpdate(0)
    {}

    virtual idx OnExecute(idx);
    bool dnaMqcProc(idx req, sHiveId &objId, sStr &errmsg);

private:
    idx progressReport(idx req, idx progress, idx percent);
    idx m_progressLast;
    idx m_progressUpdate;
};

idx DnaMQCProc::progressReport(idx req, idx progress, idx percent)
{
    idx rc = 1;
    time_t t0 = time(0);
    if(percent > m_progressUpdate || (t0 - m_progressLast) > 60) {
        m_progressUpdate = percent;
        m_progressLast = t0;
        rc = reqProgress(progress, percent, 100);
    }
    return rc;
}

#define NARRAY 4

class fileRec
{
public:
    idx nameKey;
    idx bases;
    idx counts[NARRAY];
    real proportions[NARRAY];
    real qualities[NARRAY];
    real meanQual;
    idx highOutCount;
    idx lowOutCount;
    idx totOutCount;
    idx maxReadLength;
    idx numReads;
    idx minReadLength;
    real avgReadLength;

    fileRec()
        : nameKey(0), bases(0), meanQual(0), highOutCount(0), lowOutCount(0), totOutCount(0), maxReadLength(0), numReads(0), minReadLength(0), avgReadLength(0)
    {
        for(idx i = 0; i < NARRAY; ++i) {
            counts[i] = 0;
            proportions[i] = 0;
            qualities[i] = 0;
        }
    }
};

class posRec
{
public:
    idx bases;
    idx reads;
    idx counts[NARRAY];
    real qualities[NARRAY];
    idx pos;
    real meanQual;
    real stDev;
    real max;
    real min;
    sVec< std::pair<std::pair<idx, idx>, real> > fp;

    posRec()
        : bases(0), reads(0), pos(0), meanQual(0), stDev(0), max(-999.9), min(999.9)
    {
        for(idx i = 0; i < NARRAY; ++i) {
            counts[i] = 0;
            qualities[i] = 0;
        }
    }
};

idx DnaMQCProc::OnExecute(idx req)
{
    if( !objs.dim() ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Service cannot be used without an object");
        return 1;
    }
    sVec<sHiveId> qryIds;
    formHiveIdValues("query_objID", qryIds);
    if( !qryIds.dim() ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Missing input nucletotide reads objects");
        return 1;
    }

    sStr errmsg, qryBlb, lockBuf;
    fileRec fData[qryIds.dim()];
    sVec<posRec> posData;
    sStr names00;
    idx totCount = 0;
    real totQual = 0;

    for(idx i = 0; i < qryIds.dim(); ++i) {
        sUsrObj * qry = user->objFactory(*qryIds.ptr(i));
        if( !qry ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Input object %s not found or access denied", qryIds.ptr(i)->print());
            return 1;
        }
        sStr pathtomyfile;
        if( !qry->getFilePathname(pathtomyfile, "_.qc2.sumPositionTable.csv") ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Input object %s QC position table not found", qryIds.ptr(i)->print());
            delete qry;
            return 1;
        }
        sTxtTbl tbl;
        tbl.setFile(pathtomyfile);
        tbl.parseOptions().colsep = ",";
        if( !tbl.parse() || !tbl.rows() ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Input object %s QC position table is corrupt or empty", qryIds.ptr(i)->print());
            delete qry;
            return 1;
        }
        sVec<idx> currFilPos;
        currFilPos.resize(tbl.rows());
        if( currFilPos.dim() != tbl.rows() ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Memory allocation failed: %i", __LINE__);
            delete qry;
            return 1;
        }
        fData[i].nameKey = names00.length();
        qry->propGet("name", &names00);
        for(idx r = 0; r < tbl.rows(); ++r) {
            if( r >= posData.dim() ) {
                posData.add();
                posData[r].pos = tbl.ival(r, 0);
            }
            real fpQual = 0;
            idx fpCount = 0;
            for(idx b = 0; b < NARRAY; ++b) {
                idx tempCount = tbl.ival(r, (4 + (4 * b)));
                real sumQual = tbl.rval(r, 2 + (4 * b)) * tempCount;
                totCount += tempCount;
                fpCount += tempCount;
                fData[i].counts[b] += tempCount;
                posData[r].counts[b] += tempCount;
                fData[i].bases += tempCount;
                posData[r].bases += tempCount;
                fpQual += sumQual;
                fData[i].qualities[b] += sumQual;
                posData[r].qualities[b] += sumQual;
                totQual += sumQual;
            }
            if(r == 0) {
                fData[i].numReads = fData[i].bases;
            }
            std::pair<std::pair<idx, idx>, real> * p = posData[r].fp.add();
            if( !p ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Memory allocation failed: %i", __LINE__);
                delete qry;
                return 1;
            }
            p->first.first = i;
            p->first.second = fData[i].nameKey;
            p->second = fpQual / fpCount;

            posData[r].reads = posData[r].reads + fpCount;
            currFilPos[r] = fpCount;
            if(r > 0) {
                posData[r - 1].reads = posData[r - 1].reads - fpCount;
                currFilPos[r - 1] -= fpCount;
            }
        }
        for(idx b = 0; b < NARRAY; ++b) {
            fData[i].proportions[b] = fData[i].counts[b] / real(fData[i].bases);
            fData[i].qualities[b] = fData[i].qualities[b] / real(fData[i].counts[b]);
            fData[i].meanQual += fData[i].qualities[b] * fData[i].proportions[b];
        }
        if( !progressReport(req, i, i * 60 / qryIds.dim()) ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Killed by the user");
            delete qry;
            return 1;
        }
        fData[i].maxReadLength = currFilPos.dim();
        for (idx cfp = currFilPos.dim() - 1; cfp >= 0; --cfp) {
            fData[i].avgReadLength += (currFilPos[cfp] * (cfp + 1));
            if( currFilPos[cfp] > 0 ) {
                fData[i].minReadLength = cfp + 1;
            }
        }
        fData[i].avgReadLength = fData[i].avgReadLength / fData[i].numReads;
        delete qry;
    }
    names00.add0(2);
    totQual = totQual / totCount;

    for(idx j = 0; j < posData.dim(); ++j) {
        real tempQual = 0;
        for(idx k = 0; k < posData[j].fp.dim(); ++k) {
            tempQual += posData[j].fp[k].second;
            if( posData[j].fp[k].second > posData[j].max ) {
                posData[j].max = posData[j].fp[k].second;
            }
            if( posData[j].fp[k].second < posData[j].min ) {
                posData[j].min = posData[j].fp[k].second;
            }
        }
        posData[j].meanQual = tempQual / posData[j].fp.dim();
    }
    real totStDev = 0;
    for(idx s = 0; s < qryIds.dim(); ++s) {
        totStDev += (totQual - fData[s].meanQual) * (totQual - fData[s].meanQual);
    }
    totStDev = totStDev / qryIds.dim();
    totStDev = sqrt(totStDev);
    for(idx i = 0; i < posData.dim(); ++i) {
        real tempStDev = 0;
        for(idx j = 0; j < posData[i].fp.dim(); ++j) {
            tempStDev += (posData[i].fp[j].second - posData[i].meanQual) * (posData[i].fp[j].second - posData[i].meanQual);
        }
        posData[i].stDev = tempStDev / posData[i].fp.dim();
        posData[i].stDev = sqrt(posData[i].stDev);
    }
    if( !progressReport(req, qryIds.dim(), 75) ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Killed by the user");
        return 1;
    }
    {{
        sStr outfile;
        objs[0].addFilePathname(outfile, true, "mqc.positionTable.csv");
        sFil out;
        if( outfile ) {
            out.init(outfile.ptr(0));
        }
        if( !outfile || !out.ok() ) {
            reqSetInfo(req, eQPInfoLevel_Warning, "Failed to write mqc.positionTable.csv table");
            return 1;
        }
        out.addString("pos,mean-2stdev,mean,mean+2stdev,numFiles,max,min,highOutliers,lowOutliers,#reads\n");
        for(idx i = 0; i < posData.dim(); ++i) {
            out.printf("%" DEC ",%0.4lf,%0.4lf,%0.4lf,%" DEC ",%0.4lf,%0.4lf,\"",
                    posData[i].pos,
                    posData[i].meanQual - 2 * posData[i].stDev,
                    posData[i].meanQual,
                    posData[i].meanQual + 2 * posData[i].stDev,
                    posData[i].fp.dim(), posData[i].max, posData[i].min);
            bool firstOutlier = true;
            for(idx j = 0; j < posData[i].fp.dim(); ++j) {
                if( posData[i].fp[j].second > posData[i].meanQual + (2 * posData[i].stDev) ) {
                    fData[posData[i].fp[j].first.first].highOutCount++;
                    fData[posData[i].fp[j].first.first].totOutCount++;
                    if(firstOutlier) {
                        out.printf("%s", names00.ptr(posData[i].fp[j].first.second));
                        firstOutlier = false;
                    } else {
                        out.printf(",%s", names00.ptr(posData[i].fp[j].first.second));
                    }
                }
            }
            out.printf("\",\"");
            firstOutlier = true;
            for(idx j = 0; j < posData[i].fp.dim(); ++j) {
                if( posData[i].fp[j].second < posData[i].meanQual - (2 * posData[i].stDev) ) {
                    fData[posData[i].fp[j].first.first].lowOutCount++;
                    fData[posData[i].fp[j].first.first].totOutCount++;
                    if(firstOutlier) {
                        out.printf("%s", names00.ptr(posData[i].fp[j].first.second));
                        firstOutlier = false;
                    } else {
                        out.printf(",%s", names00.ptr(posData[i].fp[j].first.second));
                    }
                }
            }
            out.printf("\",%" DEC "\n", posData[i].reads);
        }
    }}
    {{
        sStr outfile;
        objs[0].addFilePathname(outfile, true, "mqc.fileTable.csv");
        sFil out;
        if( outfile ) {
            out.init(outfile.ptr(0));
        }
        if( !outfile || !out.ok() ) {
            reqSetInfo(req, eQPInfoLevel_Warning, "Failed to write mqc.fileTable.csv table");
            return 1;
        }
        out.addString("FileName,A_fr,qA,C_fr,qC,G_fr,qG,T_fr,qT,Avg File Qual,highOutlierCount,lowOutlierCount,"
                      "Position Outlier Count,Num Reads,min Read Length,avg Read Length,max Read Length,gc Content\n");
        for(idx i = 0; i < qryIds.dim(); ++i) {
            out.printf("%s,"
                       "%0.4lf,%0.4lf,"
                       "%0.4lf,%0.4lf,"
                       "%0.4lf,%0.4lf,"
                       "%0.4lf,%0.4lf,"
                       "%0.4lf,%" DEC ",%" DEC ",%" DEC ","
                       "%" DEC ",%" DEC ",%0.4lf,%" DEC ","
                       "%0.4lf\n",
                       names00.ptr(fData[i].nameKey),
                       fData[i].proportions[0], fData[i].qualities[0],
                       fData[i].proportions[1], fData[i].qualities[1],
                       fData[i].proportions[2], fData[i].qualities[2],
                       fData[i].proportions[3], fData[i].qualities[3],
                       fData[i].meanQual, fData[i].highOutCount, fData[i].lowOutCount, fData[i].totOutCount,
                       fData[i].numReads, fData[i].minReadLength, fData[i].avgReadLength, fData[i].maxReadLength,
                       (fData[i].proportions[1] + fData[i].proportions[2]));
        }
    }}
    reqProgress(0, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}

int main(int argc, const char *argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc, argv);
    DnaMQCProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-multi-qc", argv[0]));
    return (int)backend.run(argc, argv);
}
