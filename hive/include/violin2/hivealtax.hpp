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
#ifndef sHivealtax_hpp
#define sHivealtax_hpp

#include <ssci/bio.hpp>
#include <ssci/bio/tax-ion.hpp>
#include <violin/hiveidmap.hpp>
#include <violin/hivetaxtree.hpp>
#include <violin/hiveseq.hpp>
#include <violin/hiveal.hpp>

#define NUM_RANKS 8

#define PROGRESS(items,cur,max) (m_callback && !m_callback(m_callbackParam, items, ((cur) * 1.0 / (max)) * 100, 100))

namespace sviolin
{
    class sHivealtax
    {
        public:
            struct stats
            {
                    idx min;
                    idx max;
                    real localnum;
                    idx num;
                    real sum;
                    real sumsq;
                    idx family_taxid_pos;
                    idx leaf_taxid_pos;
            };
        private:
            struct RanksAndWeights
            {
                    const char * rankName;
                    real weight;
                    idx totalPopulation;
            };
            sUsr * user;
            RanksAndWeights rankList[NUM_RANKS];

            sHiveIdMap *NT;
            idx iterationNumber, iteration;
            sDic<stats> censusStats;
            sDic<stats> accCentericStats;
            sStr taxidContainer;

            real calculateShannonEntropy (sDic <idx> *taxCnt, idx rankdim, idx totRankWeight = 1 , sStr * outReport = 0);
            real ShannonFunction(sDic<idx> * taxCnt, idx totalPopulation);
            idx RankedTaxCountFunction(sDic<idx> & dstTaxCnt, sDic<idx> * srcTaxCnt, const char * requiredRank);

            void addStats(stats *st, idx lnum = 0)
            {
                if (lnum){
                    st->localnum = lnum;
                }
                if (st->localnum > 0){
                    st->sum += st->localnum;
                    st->sumsq += (st->localnum * st->localnum);
                    if( st->num == 0 ) {
                        st->min = st->localnum;
                        st->max = st->localnum;
                    } else {
                        if( st->min > st->localnum ) {
                            st->min = st->localnum;
                        }
                        if( st->max < st->localnum ) {
                            st->max = st->localnum;
                        }
                    }
                    st->num += 1;
                    st->localnum = 0;
                }

            }
            real Stats_stddev(stats * st)
            {
                if( st->num <= 1 )
                    return 0;
                return sqrt((st->sumsq - (st->sum * st->sum / st->num)) / (st->num - 1));
            }
            real Stats_mean(stats * st)
            {
                if( st->num == 0 )
                    return 0;
                return (st->sum / st->num);
            }

        public:
            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);
            callbackType m_callback;
            void * m_callbackParam;

            sTaxIon * taxion;
            void addStats (idx option, const char *key, idx keylen, idx cnt)
            {
                stats *auxcensusStat = 0;
                if (!keylen){
                    keylen = sLen(key);
                }
                if (option == 0){
                    auxcensusStat = censusStats.set(key, keylen);
                }
                else if (option == 1){
                    auxcensusStat = accCentericStats.set(key, keylen);
                }
                if (auxcensusStat){
                    addStats (auxcensusStat, cnt);
                }
            }

            stats * setStats(idx option, const char *key)
            {
                if (option == 0){
                    return censusStats.set(key);
                }
                else if (option == 1){
                    return accCentericStats.set(key);
                }
                return 0;
            }

            sHivealtax(sTaxIon*myTree = 0, idx totIter = 0, idx currIter = 0)
            {

                init(0, 0, 0, totIter, currIter);
                taxion=myTree;
            }

            sHivealtax(sUsr * usr, sNCBITaxTree *myTree = 0, sHiveIdMap * idMap = 0, idx totIter = 0, idx currIter = 0)
            {
                init(usr, myTree , idMap , totIter , currIter );
            }

            sHivealtax * init(sUsr * usr, sNCBITaxTree *myTree = 0, sHiveIdMap * idMap = 0, idx totIter = 0, idx currIter = 0)
            {
                user = usr;

                const char * RankNames00 = "leaf" _
                "species" _
                "genus" _
                "family" _
                "order" _
                "class" _
                "phylum" _
                "kingdom" __;

                const char * rankName = RankNames00;
                for (idx i = 0; i < NUM_RANKS; i++, rankName = sString::next00(rankName)) {
                    rankList[i].rankName = rankName;
                    rankList[i].weight = 1;
                    rankList[i].totalPopulation = 0;
                }
                iterationNumber = totIter;
                iteration = currIter;
                censusStats.cut(0);
                accCentericStats.cut(0);
                taxidContainer.cut(0);
                m_callback = 0;
                m_callbackParam = 0;

                NT = idMap;
                if (user){
                    if (myTree == 0){
                    sStr myTaxTreePath;
                    sTaxTreeMap findObjId(*user);
                    findObjId.getFileName(myTaxTreePath);
                    }
                    if (idMap == 0){
                        NT = new sHiveIdMap(*user, "giTaxVioDB");
                    }
                }
                return this;
            }

            ~sHivealtax()
            {
            }
            void setProgressReport (callbackType ct, void * ctp)
            {
                m_callback = ct;
                m_callbackParam = ctp;
            }

            real analyzeResults(sFil * outTaxCnt, sDic<idx> *taxCnt, sFil * outSha = 0);
            idx CurateResult(sDic<idx> *taxCnt, sHiveseq *Sub, sHiveal *hiveal, sDic<idx> *giCnt, const char *taxDepth = "leaf", sTxtTbl *tbl = 0, idx colNumber = 0, bool useAlignments = false);
            idx reportResults(sFil * outTaxCnt, sDic<idx> *taxCnt, sFil * outGiCnt = 0, sDic<idx> *accCnt = 0);

            idx setTaxidContainer (const char* str, idx len = 0){
                if (!len){
                    len = sLen(str);
                }
                if (strncmp(str, "-1", 2) == 0){
                    len = 2;
                }
                idx ret = taxidContainer.length();
                taxidContainer.add(str,len);
                taxidContainer.add0();
                return ret;
            }

            const char * getTaxidContainer (idx pos){
                return taxidContainer.ptr(pos);
            }
    };

    class sHiveTaxAcc
    {
        public:
        sVar  tax2acc;
        sDic < sBioseq::FlatFASTXPos > acc2nt;
        idx fpNT, fpOut;
        sDic < idx > excl, incl;

        sHiveTaxAcc(){}

        sHiveTaxAcc(sUsr * luser, const char * objType, const char * findVar, const char * findVal, const char * outFile)
        {
            sUsrObjRes obj_res;
            if( luser->objs2(objType, obj_res,(udx*)0, findVar, findVal)) {
                init( luser, obj_res.id(obj_res.first())->print() , outFile );
            }

        }
        sHiveTaxAcc(sUsr * usr, const char * hiveObjId, const char * outFile)
        {
            init(usr, hiveObjId, outFile);
        }

        sHiveTaxAcc * init(sUsr * usr, const char * hiveObjId, const char * outFile)
        {
            fpNT=0, fpOut=0;
            if(!hiveObjId)return this;
            sUsrObj obj(*usr, sHiveId(hiveObjId) );
            sStr buf;obj.getFilePathname(buf,"_.fa.dic_taxid2acc");tax2acc.init(buf);
            buf.cut(0);obj.getFilePathname(buf,"_.fa.dic_index");acc2nt.init(buf);
            buf.cut(0);obj.getFilePathname(buf,"_.fa");
            fpNT=open(buf,O_RDONLY);
            if(!ok())return 0;
            sFile::remove(outFile);fpOut=open(outFile,O_CREAT|O_RDWR,S_IRWXU|S_IRWXG|S_IROTH);
            return this;
        }
        bool ok(void)
        {
            if(!fpNT || !tax2acc.dim() || !acc2nt.dim())
                return 0;
            return true;
        }


        struct TAX2ACC {
            char Acc[128];
            idx len;
            sBioseq::FlatFASTXPos  * inNT;
        };

        idx listAccByTaxId(const char * taxid, sVec < TAX2ACC > *  t2a, idx maxAccPerSpecies=0, bool empty=true)
        {
            if(incl.dim() && incl.get(taxid)==0)return 0;
            if(excl.dim() && excl.get(taxid)) return 0;

            if(empty)t2a->cut(0);
            const char * accList=tax2acc.value(taxid);
            idx cnt=0;
            for ( const char * acc=accList; acc!=0; ) {
                const char * nxt =strchr(acc,',');
                idx len=nxt ? nxt-acc : sLen(acc);
                sBioseq::FlatFASTXPos * inNT = acc2nt.get(acc,len );

                if(nxt)acc=nxt+1;else acc=nxt;
                if(!inNT)continue;
                if(acc) {
                    TAX2ACC* ta=t2a->add(1);
                    memcpy(ta->Acc,acc,len); ta->Acc[len]=0;
                    ta->inNT=inNT;
                    ta->len=inNT->endSeq-inNT->startSeq;
                }
                ++cnt;
                if(maxAccPerSpecies && cnt>maxAccPerSpecies)
                    break;
            }
            return t2a->dim();
        }

        void copySequences( TAX2ACC * t2a, idx fOut=0 )
        {
            sBioseq::FlatFASTXPos  * inNT=t2a->inNT;
            if(!inNT)return;
            if(!fOut)fOut=fpOut;
            idx len, copied=0;
            char buf[4*1024];
            lseek(fpNT,inNT->startId,SEEK_SET);
            while( (len = read(fpNT, buf, (copied+sDim(buf)>(inNT->endSeq-inNT->startId)) ? ((inNT->endSeq-inNT->startId)-copied) :  sDim(buf) )) != 0 ) {
                idx written = 0;
                while( len - written > 0 ) {
                    written += write(fOut, buf + written, len - written);
                }
                copied+=len;
            }
        }

        enum eExlcIncl{eExclude,eInclude};
        void parseTaxIds(const char * flnm, bool excl_incl=eExclude, char separator='\n')
        {
            sFil fl(flnm,sMex::fReadonly);
            (excl_incl==eExclude ? &excl : &incl)->parseDic(fl.ptr(),fl.length(),separator);
        }
        void parseTaxIds(const char * src, idx len, bool excl_incl=eExclude, char separator='\n')
        {
            if(!len){len=sLen(src);}
            (excl_incl==eExclude ? &excl : &incl)->parseDic(src,len, separator);
        }

        ~sHiveTaxAcc()
        {
            if(fpNT){close(fpNT);}
            if(fpOut){close(fpOut);}
        }
    };
}
#endif