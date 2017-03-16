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

#endif // sDnaQC_hpp

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
        idx numTable, numUniqTable;     //number of selected, unique selected
//        idx numStop[7];                   //numStopCodons
//        unsigned char StopBase[7][4 * 3];
//        sVec <unsigned char> stopBase;
        sVec <sVec<unsigned char> > stopBase;
//        sVec <idx> numStop;
//        static idx TbGroup[];                //store the table group info
        unsigned char * SBpt;           //pointer to StopBase

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


        bool stopCodon(char b1, char b2, char b3, idx id) //is it stop codon of idth table
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





/*! Structure that we use to calculate Positional Statistics like:
 * min, max, Std. Dev. and Average
 */
struct QCstats
{
        idx cACGT[4];   //! array that stores the count of A,C,G,T's
        idx cqACGT[4];  //! array that stores the quality count of A,C,G,T's
        idx min[4];     //! array that stores the minimum value
        idx max[4];     //! array that stores the maximum value
        real num[4];    //! array that stores the number of A,C,G,T's only if they have a quality value associated
        real sum[4];    //! array that stores the quality sum of A,C,G,T's only if they have a quality value associated
        real sumsq[4];  //! array that stores the sum square of A,C,G,T's only if they have a quality value associated
};

/*! Structure that we use to calculate Lengthwise Statistics of the Reads
*/
struct Lenstats
{
        idx num;    //! the length value
        idx sum;    //! the count of length value
};

//! Function to Calculate the Average
real Stats_mean(QCstats * st, idx i)
{
    if (st->num[i] == 0)
        return 0;
    return (st->sum[i] / st->num[i]);
}

//! Function to Calculate the Std. Dev.
real Stats_stddev(QCstats * st, idx i)
{
    if (st->num[i] <= 1)
        return 0;
    return sqrt((st->sumsq[i] - (st->sum[i] * st->sum[i] / st->num[i])) / (st->num[i] - 1));
}


//! Function that process each sample to register into the Statistics Library (structure)
void Stats_sample(QCstats *st, idx s, idx q, idx rpt)
{
    st->cACGT[s] += rpt;

    if( q != -1 ) {
        st->cqACGT[s] += (q * rpt);
        st->sum[s] += (q * rpt);    //! It could be a redundant variable that we can remove
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

//! Function to Report Progress taking into consideration the time and percentage
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
