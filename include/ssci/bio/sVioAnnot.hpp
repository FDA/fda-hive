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
#ifndef sLib_sVioAnnot_hpp
#define sLib_sVioAnnot_hpp

#include <slib/utils/sRangeTree.hpp>
#include <slib/utils/sort.hpp>
#include <slib/utils/sVioDB.hpp>
#include <slib/utils/tbl.hpp>
#include <ssci/bio/bumper.hpp>
#include <math.h>


#define generalLocus_TYPE 1
#define definition_TYPE 2
#define kindOfId_TYPE 3
#define ids_TYPE 4
#define range_TYPE 5
#define giNumber_TYPE 6
#define dataName_TYPE 7

#define idxType_id 1
#define idxType_range 2
#define idxType_idType 3
#define idxType_refSrce 4

#define  SIZEBUF (1024*4)

namespace slib
{
    class sVioAnnot
    {
        private:
        struct GBdesc{
            sStr locus,start,end;
            //bool complement;
            sStr curLocus, curData, curTag, curPath, curValue ;
        };

        GBdesc gbd;
        udx locusNum;
        udx cdsNum,tagNum;
        udx matPepNum ,matPepTagNum ;
        idx header;
        sStr dataFlag;
        sStr compText;
        sStr joinText;
        bool complement;
        bool join;

        public:
            struct AnnotStruct {
                    sStr rangeID;
                    sVec<idx> rangeStart, rangeEnd;//,max;
                    idx subRangeHit,annotFileIndex;
                    sVec<idx> rangeGaps; //accumulated gaps up to i th range
//                    AnnotStruct(){sSet(this,0);}
            };

            sVioDB DB;

            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);

            callbackType myCallbackFunction;
            void * myCallbackParam;

        protected:
            idx progressReport(idx progress, idx percent);
            idx m_progressLast;
            idx m_progressUpdate;

        public:
            struct startEnd {
                    idx start,end;
                    idx group;
                    //Max will be used for search purposes. Like in the interval trees.
                    idx max;
                    startEnd(){sSet(this,0);}

            };

            struct startEndNode{
               startEnd * ranges;
               idx index,subRangesCnt;
//                       startEndNode(){sSet(this,0);}
            };

            struct ParamsRangeSorter{
               idx flags;
               sVioAnnot * vioannot;
               ParamsRangeSorter(){
                   flags=0;
                   vioannot=0;
               }
           };



             static idx sortVioAnnotFileRangesBasedOnSeqId(sVioAnnot * myAnnot);
             void FixUpMaxVirtualTree(idx * relPtr,idx bodysize,sVioDB * DB);
             idx searchInVirtualTree(idx * relPtr,idx bodysize,sVec<startEndNode> & results,idx start,idx end=0);

        private:

            struct vtreeNode{
                    startEnd * range;
                    idx ind,level;
//                    vtreeNode(){sSet(this,0);}
            };

            static const idx deBrSeq=0x022fdd63cc95386d;

            //Lookup table for the de Bruijn sequence 0x022fdd63cc95386d
            static idx deBrLUT [64];

            static idx getLSBindex1(idx & a);

            static idx getRoot(idx depth);
            static idx getParent(idx x, idx * level=0);
            static idx getLeft(idx x, idx * level=0);
            static idx getRight(idx x,idx * level=0);





        public:

           sVioAnnot(const char * InputFilename=0)
            {
                init(InputFilename);
            }

           sVioAnnot * init (const char * InputFilename=0, idx filemode=sMex::fSetZero|sMex::fBlockDoubling)
            {
                m_progressUpdate=0;
                m_progressLast=0;
                locusNum=0;
                cdsNum=0;tagNum=0;
                matPepNum = 0; matPepTagNum = 0;
                header = 0;
                dataFlag.printf(" ");
                complement = false;
                join = false;

                if(InputFilename) DB.init(InputFilename,"sVioAnnot",0,0,filemode);
                    return this;
            }
            bool isok(void)
            {
                return DB.isok("vioAnnot")? true : false;
            }
            bool isGBstructure(void);
            idx getRangeTypeIdx(void);
            idx getIdTypeIdx(void);
            idx getRltTypeIdx_Id2Rng(void);

           idx * getRangesForGivenIDAndIDType(const char * srchID, const char * idtype, idx * pRelationCnt);
           startEnd * getRangesByIndex(idx rangeIndex, idx * pDimension);

           const char * getDataNameByRangeIndex(idx rangeIndex);

           // _____/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
           //
           //         New Structure: id (1), range(2), idType(3), group(4)
           //
           //_____/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


           idx getIdIndex(const char * idToUse, const char * idTypeToUse);
           idx getIdTypeIndex(const char * idToUse);

           idx * getNumberOfRangesByIdTypeAndId(const char * idTypeToUse, const char * idToUse, idx & cntRanges);
           idx getNberOfIdsByRangeIndex(idx rangeIndex);
           idx * getNumberOfRangesByIdType(const char * idTypeToUse, idx &cntRanges);

           startEnd * getRangeJointsByRangeIndex(idx rangeIndex, idx * cntRangeJoints);
           //void getIdTypeByRangeIndexAndIdIndex(idx rangeIndex,idx idIndex, sDic < sVec <sStr> > & myIdAndIdType);
           void getIdTypeByRangeIndexAndIdIndex(idx rangeIndex, idx idIndex, const char ** idPtr, idx * pIdSize, const char ** idTypePtr, idx * pIdTypeSize);

           idx * getIdByIdType(const char * idTypeToUse, idx * cntId);
           void getIdByIdIndex(sStr & myId, idx idIndex);

           idx getAllIdTypes(sStr & buf, sVec<sMex::Pos> * bufposes = 0);
           void getAllIdsbyIdType(const char * idTypeToUse, sDic < sStr > & idVec);

           idx printInformationBasedOnIdAndIdType(const char * idTypeToSearch, const char * idToSearch, sVec < sStr > & whatToPrint, sStr & outPut, idx & nbOfLinePrinted, idx start, idx end, idx cnt, bool combineIdIdType = false);
           idx printInformationBasedOnIdTypeList(const char * sourceFileName,const char * refSeqID, sVec < sStr > & idTypeFilterList, sStr & outPut, idx & nbOfLinePrinted, idx pos_start, idx pos_end,idx page_start, idx page_end, idx cnt);


           // ------------- /***********/ --------------------------

          idx ParseGBfile(const char* inputGBfilename, const char * vioFileName,bool combineFile=true);
          void addRecordRelationShipCounterForSeqID(sVioDB * myDB, sStr & myLine, sStr & gi);
          void composeIdAndIdType(const char * myId,const char * myIdType, sStr & ididType);
          idx getTotalRecord();

          static idx GetAnnotRangeList( const char * idToUse, const char * idtype00, idx position, sVec < sVioAnnot::AnnotStruct  > * results,  sVec < sVioAnnot > * annotList,const char * dataName="CDS");

          static idx VioAnnotRangeComparator(void * parameters, void * RangeA, void * RangeB, void * arr,idx i1,idx i2);

          void specificTreatmentFunction ( const char * tagName, const char * value, bool isContinued, sStr & lineExp );
          bool extractedInfo(char * mystring, int FloorNum[],int & currentFloor ,int & FoundLineNonBlankPos, int & quoteCount, bool & continueationMode, int & line_num, int & find_result,sStr & lineLine);
          static void cleanIdFromProfiler(const char * idToClean, sStr & idOut);

          static idx runBumperEngine(const char * contentInCSVFormat,idx sourceLength,sStr & tableOut, idx _referenceStart, idx _referenceEnd, idx _width, idx _resolution, idx _annotationDensity, idx _maxLayers);


          /*!
          ePrintIDpos                   : position(s) or range(s) of query
          ePrintSeqID                   : e.g. chromosome
          ePrintAnnotRange              : print annotation range (start-end)
          ePrintAnnotID                 : print annotID
          ePrintAnnotRangeInOneColumn   : e.g. 1234-152 or 12
          ePrintSingleHitRow            : will merge all ranges (hits) of a searched ragned (e.g if we searched for position 11 and we got 3 ranges back,
                                          with each one corresponding to more than one annotation then we get one row for all these fields
          ePrintSingleAnnotRangeRow     : will merge all ids and id types of a single range
          */
          enum ePrintSearchParams {
              ePrintIDpos=                      0x00000001,
              ePrintSeqID=                      0x00000002,
              ePrintAnnotRange=                 0x00000004,
              ePrintAnnotID=                    0x00000080,
              ePrintAnnotRangeInOneColumn=      0x00000010,
              ePrintSingleHitRow=               0x00000020,
              ePrintSingleAnnotRangeRow=        0x00000040
          };
          struct searchOutputParams {
              const char * column_delim;
              const char * row_delim;
              sStr * outBuf;
              idx rowParams;
              void * miscParams;
              searchOutputParams(const char * c_del, const char * r_del){
                  column_delim = c_del ? c_del :",";
                  row_delim = r_del ? r_del :"\n";
                  outBuf = 0;
                  rowParams = ePrintIDpos | ePrintSeqID | ePrintAnnotRange | ePrintAnnotID;
                  miscParams = 0;
              }
          };

          bool printRangeSetSearch( sVec<idx> &start, sVec<idx> &end, idx indexID, searchOutputParams & params);
          bool printSingleRangeSearch(startEndNode & range,searchOutputParams & params, const char * seq = 0);
          bool printSingleAnnotation (const char * id, const char * id_type, searchOutputParams & params, idx start = 0, idx end = 0, const char * seqID = 0 );
          bool printRangeSetOnSeqIDSearch( sVec<idx> &start, sVec<idx> &end, const char * seqID, searchOutputParams & params) {
              if(!seqID || isGBstructure() ){
                  return false;
              }
              idx idIndex = getIdIndex(seqID,"seqID");
              if( !idIndex ) {
                  return false;
              }

              return printRangeSetSearch(start,end,idIndex,params);
          };

    };



    template <class Tobj=idx,class Kobj=idx> class sRangeTreeList{
            typedef sRangeTreeList<Tobj,Kobj> RangeTreeList;
        const char * sRangeTreeFileMarker;
        idx HeaderSpace;
        idx pointerForSizeValidation;
        idx pointerToOffsets;
    private :
        sVec<idx> _vec;
        sVec<idx> _offsets;
        udx _LastReferenceI;

    public:
        struct NodeRangeTree{
            Tobj start,end,max,RangeI;    //start,end, Tind:range index, Rind,
            idx left,right,parent;
            NodeRangeTree(){sSet(this,0);}
        };
    public: // constructor/destructor

        typedef sRangeTree<Tobj,Kobj> sRangetree;
//        sRangeTree( idx lflags=sMex::fBlockDoubling, const char * flnm=0 )  : _vec( lflags) {if(flnm) _vec.init(flnm);}
        sRangeTreeList(const char * flnm=0, bool doMap = false )  {
            _LastReferenceI=-1;
            pointerToOffsets=7;
            pointerForSizeValidation=6;
            HeaderSpace=8;
            if(doMap){
                _vec.init(flnm,sMex::fReadonly);
                if(*_vec.ptr(pointerToOffsets)>0 && *_vec.ptr(pointerToOffsets)<_vec.dim()){
                    idx * offs=_offsets.add(_vec.dim()-*_vec.ptr(pointerToOffsets));
                    memcpy(offs,_vec.ptr(*_vec.ptr(pointerToOffsets)), _offsets.dim()*sizeof(idx)  );

                }
                _LastReferenceI=-1;
            }
            else{
                if(flnm){_vec.init(flnm);}
                init();
            }
        }

        RangeTreeList * init(){

            sRangeTreeFileMarker="v1.0";

            _vec.add(HeaderSpace);
            char * space=(char *)(_vec.ptr(1));
            strcpy(space,sRangeTreeFileMarker);

            *_vec.ptr(pointerForSizeValidation)=sizeof(Tobj);

            return this;
        }


        idx insert(sVec<sVioAnnot::AnnotStruct> & rangeVec,idx ReferenceI){
            if(ReferenceI>=_offsets.dim()){
                _offsets.vadd(ReferenceI-_offsets.dim()+1,0);
//                _offsets.resize(ReferenceI+1);
            }
            idx * curroffs=_offsets.ptr(ReferenceI);
            sRangetree rTree(sRangetree::eAVLmode);
            for(idx iR=0;iR<rangeVec.dim();++iR){
                sVioAnnot::AnnotStruct * range=rangeVec.ptr(iR);
                rTree.insert(range->rangeStart,range->rangeEnd,iR);
            }
            idx sizeNode=sizeof(sRangenode<Tobj,Kobj>);
            idx sizeTree=rTree._vec.dim()*sizeNode;
            idx * newTree=(idx*)_vec.add(1+(sizeTree/sizeof(idx)));
            *curroffs=((char *)newTree-(char *)_vec.ptr())+((char *)rTree.root-(char *)rTree._vStart);

            memcpy(newTree,rTree._vec.ptr(),sizeTree);
            return 0;
        }

        idx search(sVec<Kobj> &results,idx position,idx ReferenceI,Kobj * value=0){
            if(!_offsets.dim()){
                return 0;
            }
            idx offset=*_offsets.ptr(ReferenceI);
            char * rootP=(char *)_vec.ptr()+ offset;
            sRangetree rTree(rootP);
            return rTree.search(results,position,position,value);
        }
        idx insert(idx start,idx end,idx RangeI,idx ReferenceI){
            if(ReferenceI>=_offsets.dim()){
                _offsets.resize(ReferenceI+1);
            }
            else{

            }
            return 0;
        }

        idx search(idx start,idx end,idx RangeI,idx ReferenceI){
            return 0;
        }
        idx Finalize(){
            *_vec.ptr(pointerToOffsets)=_vec.dim();
            _vec.glue(&_offsets);
            return 111232341234;
        }
    };
}
#endif
