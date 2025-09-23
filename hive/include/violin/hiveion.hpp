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
#ifndef sHiveIonhpp
#define sHiveIon_hpp

#include <slib/std.hpp>
#include <ulib/usr.hpp>
#include <ion/sIon.hpp>
#include <violin/hiveseq.hpp>
#include <violin/hiveal.hpp>

namespace sviolin
{
    class sHiveIon
    {
        protected:
            sUsr * myUser;


        public:
            idx ionCnt;
            sDic < sIonWander> wanderList;
            sStr pathList00;

        public:

            sHiveIon(sUsr * user, const char * objList=0, const char * iontype=0, const char * file=0 ) { init(user, objList,  iontype, file ); }


            sHiveIon * init(sUsr * user, const char * objList=0, const char * iontype=0, const char * filenameWithoutExtension00=0 );
            sIonWander * addIonWander(const char * wandername, const char * iql, idx iqllen);
            sIonWander * addIonWander(const char * wandername, const char * iql, ... ){
                sStr buf;
                sCallVarg(buf.vprintf,iql);
                return addIonWander(wandername,buf.ptr(),buf.length());
            }

            void setSeparators(const char * sepField, const char * sepRecord, const char * wandername=0)
            {
                for (idx i=0 ;i < wanderList.dim() ; ++i ) {
                    if(wandername && strcmp(wandername,(const char * ) wanderList.id(i))!=0 )
                        continue;

                    if(sepField)wanderList[i].traverseFieldSeparator=sepField;
                    if(sepRecord)wanderList[i].traverseRecordSeparator=sepRecord;
                }
            }

            struct geneExpr {
               sDic < sDic < sVec < real > > > samplePassageReplicaDic;

               sDic < idx >  rowIds;
               sMex largeDicBuf;
               sDic < sDic < sMex::Pos  > > valueDic;
               sStr compositePassage;
               bool collapsePassage;
               bool exportStDev;
               geneExpr () {
                 collapsePassage=false;
                 exportStDev=false;
                 compositePassage.cut(0);
               };
           };
            idx retrieveListOfObjects(sUsr * usr, sVec<sHiveId> * objList, sDir * fileList00, const char * objIDList, const char * typeList, const char * parList00, const char * valList00, const char * filePattern00);
            static idx realValStatCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist );
            static idx anyValStatCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist );
            void dicDicVecPrint(sIO * buf, sDic < sDic < sVec < real > > > & sPrDic, idx characteristics, idx printMeasurementAsHeader=0 , idx doSort=false, void * params=0);

            idx Cmd(sIO * out, const char * cmd, sVar * pForm);

            sIonWander & operator [](const char * nam)
            {
                return wanderList[nam];
            }
            sIonWander * operator()(const char * nam)
            {
                return wanderList.get((const void*)nam,0,0);
            }

            static idx loadIonFile(sUsr * user,sVec<sHiveId> & objList,sIonWander & genericWander, const char * ionFileTmplt) {
                if (!objList.dim()) {
                    return 0;
                }
                sStr tmpPath;
                for ( idx i=0; i<objList.dim() ; ++i) {
                   tmpPath.cut(0);
                   sUsrObj o(*user, objList[i]);

                   o.getFilePathname(tmpPath,ionFileTmplt);
                   if (tmpPath.length()){
                       const char * ipath = strrchr(tmpPath.ptr(),'.');
                       if (ipath){
                           tmpPath.cut0cut(ipath-tmpPath.ptr(), 2);
                           genericWander.attachIons( tmpPath.ptr(), sMex::fReadonly,1);
                       }
                   }

                }

                return genericWander.ionList.dim();
            }

            void getPathList00(sStr & path00) {
                path00.add(pathList00.ptr(),pathList00.length());
            }

    };

    class sHiveIonSeq: public sHiveIon {

        public:
            sHiveIonSeq(sUsr * user, const char * objList=0, const char * iontype=0,  const char * file=0)
                :sHiveIon(user, objList, iontype,  file)
            {

            }
            struct infoParams {
                    idx cntStart;
                    idx cnt;
                    idx curIndex;
                    bool printDots;
                    infoParams() {cntStart=curIndex=0;cnt=20;printDots=true;};
            };

            static idx locateSeqId(const char * seqId, idx * seqLen) {
                if (!seqId) {
                    return -1;
                }

                idx idStartPos=0;
                (*seqLen)=sLen(seqId);
                const char * space = strpbrk(seqId," ");
                if (space) {
                    *seqLen=space-seqId;
                }
                if (seqId[0]=='>') {
                    (*seqLen)=(*seqLen)-1;
                    idStartPos+=1;
                }
                return idStartPos;
            }
            idx annotMap(sIO * io, sBioseq * sub, sDic <sStr > * dic,const char * seqidFrom00, idx countResultMax=sIdxMax, idx startResult=0, idx contSequencesMax=sIdxMax , idx outPutWithHeader=0, sStr * header=0);
            static idx annotMap(void * hiveIonPointer ,sIO * io, sBioseq * sub, sDic <sStr > * dic,const char * seqidFrom00, idx countResultMax=sIdxMax, idx startResult=0, idx contSequencesMax=sIdxMax ,idx outPutWithHeader=0, sStr * header=0)
            {
                return ((sHiveIonSeq * )hiveIonPointer)->annotMap(io, sub, dic,seqidFrom00, countResultMax, startResult, contSequencesMax,outPutWithHeader, header );
            }
            static idx traverserCallback (sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist);
            idx annotMapPosition(sStr *output, sDic < sStr > * dic, const char * seqidFrom00,idx pos_start=0, idx pos_end=0, idx countResultMax=sIdxMax, idx startResult=0, idx header=1, sBioal * seqMultipleAlign=0);
            idx standardTraverse(sStr & output, sDic < sStr > * dic, idx countResultMax=20, idx startResult=0, bool printDots=true, idx wanderIndex=-1);


    };
}


#endif 



