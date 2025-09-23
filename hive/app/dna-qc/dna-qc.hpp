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
#ifndef sDnaQC_hpp
#define sDnaQC_hpp

#include <qlib/QPrideProc.hpp>
#include <slib/utils.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <ssci/bio/biogencode.hpp>

using namespace slib;

#endif 
class DnaQCProc: public sQPrideProc
{
    public:
        DnaQCProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv), m_progressLast(0), m_progressUpdate(0)
        {
        }

        virtual idx OnExecute(idx);
        bool dnaqcProc(idx req, sHiveId &objId, sStr &errmsg);

        static idx TbGroup[];

        const char *stopCODONS;
        idx numTable, numUniqTable;
        sVec <sVec<unsigned char> > stopBase;
        unsigned char * SBpt;

    private:
        idx progressReport(idx req, idx progress, idx percent);
        idx m_progressLast;
        idx m_progressUpdate;

        idx minLength;
        idx maxLength;
        idx sumLength;
        idx numLength;


        void initLengthStats(){
            minLength = sIdxMax;
            maxLength = 0;
            sumLength = 0;
            numLength = 0;
        }

        void addlengthStat(idx len, idx rpt = 1)
        {
            numLength += rpt;
            sumLength += (len * rpt);
            if( len < minLength ) {
                minLength = len;
            }
            if (len > maxLength){
                maxLength = len;
            }
        }


        bool stopCodon(char b1, char b2, char b3, idx id)
        {
            for(idx i = 0; i < stopBase[id].dim(); i+=3) {
                if( b1 == stopBase[id][i] && b2 == stopBase[id][i+1] && b3 == stopBase[id][i+2] ) {
                    return true;
                }
            }
            return false;
        }

        char getBase(idx charpos, idx basepos, const char * str)
        {
            return ((*(str + charpos)) >> (basepos * 2) & 0x03);
        }

        void initStopCodon();
        void getCodonQC(sVec<idx> &countHas, const char* str, idx seqLen, idx rpt);

};





struct DnaQCstats
{
        DnaQCstats()
        {
            sSet(this);
        }

        idx cACGT[5];
        idx cqACGT[5];
        idx min[5];
        idx max[5];
        real num[5];
        real sum[5];
        real sumsq[5];
};

struct Lenstats
{
        idx num;
        idx sum;
};

real Stats_mean(DnaQCstats * st, idx i)
{
    if (st->num[i] == 0)
        return 0;
    return (st->sum[i] / st->num[i]);
}

real Stats_stddev(DnaQCstats * st, idx i)
{
    if (st->num[i] <= 1)
        return 0;
    return sqrt((st->sumsq[i] - (st->sum[i] * st->sum[i] / st->num[i])) / (st->num[i] - 1));
}


void Stats_sample(DnaQCstats *st, idx s, idx q, idx rpt)
{
    st->cACGT[s] += rpt;

    if( q != -1 ) {
        st->cqACGT[s] += (q * rpt);
        st->sum[s] += (q * rpt);
        st->sumsq[s] += ((q * q) * rpt);

        if( st->num[s] == 0 ) {
            st->min[s] = q;
            st->max[s] = q;
        } else {
            if( st->min[s] > q ){
                st->min[s] = q;
            }
            if( st->max[s] < q ){
                st->max[s] = q;
            }
        }
        st->num[s] += rpt;
    }
}

idx DnaQCProc::progressReport(idx req, idx progress, idx percent)
{
    idx rc = 1;
    time_t t0 = time(0);
    if( percent > m_progressUpdate || (t0 - m_progressLast) > 60 ) {
        m_progressUpdate = percent;
        m_progressLast = t0;
        rc = reqProgress(progress, percent, 100);
    }
    return rc;
}
