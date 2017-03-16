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
#include <ssci/bio.hpp>
#include <ulib/ulib.hpp>
#include <violin/violin.hpp>

#include <ssci/bio/sVioAnnot.hpp>

using namespace sviolin;

void sHiveannot::InitAnnotList(sUsr * user, sVec<sVioAnnot> & annotList, sVec<sHiveId> * annotIDListToUse, bool getAll)
{
    sVec<sHiveId> annotIDList;

    //if( !annotIDListToUse || !annotIDListToUse->dim()) {
    if( getAll) {
        annotIDListToUse->empty();
        sUsrObjRes res;
        user->objs2("u-annot", res);
        for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
            *(annotIDList.add(1)) = *res.id(it);
        }
        annotIDListToUse = &annotIDList;
    }
    for(idx iV = 0; iV < annotIDListToUse->dim(); ++iV) {
        if( !*annotIDListToUse->ptr(iV) )continue;
        sUsrObj obj(*user, *annotIDListToUse->ptr(iV));
        sStr path;
        obj.getFilePathname00(path, ".vioannot" __);
        if (!path.length()) continue;
        sVioAnnot * a = annotList.add(1);
        if( a ) {
            a->init(path, sMex::fReadonly);
        }
    }
}

void sHiveannot::getAnnotListFromIdAndIdType(sUsr * user, const char * idTypeToUse, const char * idToUse, sVec < sVioAnnot > * annotListOut, sStr * tableOut){
    sUsrObjRes annotIDList;
    user->objs2("u-annot", annotIDList);

    for(sUsrObjRes::IdIter it = annotIDList.first(); annotIDList.has(it); annotIDList.next(it)) {
        sUsrObj obj(*user, *annotIDList.id(it));
        sStr path;
        obj.getFilePathname00(path, ".vioannot" __);
        if (!path.length()) continue;
        sVioAnnot a;
        a.init(path, sMex::fReadonly);
        if (a.isGBstructure()){
#ifdef _DEBUG
            ::printf("====> %s,%s,%" DEC "\n",obj.Id().print(), obj.propGet("name"), sFile::time(path));
#endif
        } else {
            idx cntRanges=0;
            idx * indexRangePtr =0;
            indexRangePtr = a.getNumberOfRangesByIdTypeAndId(idTypeToUse,idToUse, cntRanges);
            if (cntRanges && indexRangePtr){
                if (annotListOut){
                    //sVioAnnot * toAdd = annotListOut->add();
                    //toAdd = &a;
                }
                if (tableOut){ // objAnnot.Id().print(line);
                    tableOut->printf("%s,%s,%" DEC "\n",obj.Id().print(), obj.propGet("name"), sFile::time(path)); // ,%s,%" DEC "\n", objAnnot.propGet("name"), sFile::time(anotPath));
                }
            }
        }
    }
}

idx sHiveannot::outInfo (sStr & output, const char * inputRangeTable, sVec< sVioAnnot > & anotList){

    sFil crossRange(inputRangeTable,sMex::fReadonly); // OPEN THE FILE
    const char * filebody = crossRange.ptr();

    output.printf("Reference,Overlap_start,Overlap_end,FileName,Range_start,Range_end,Annotation_type,Annotation_id\n");

    sTxtTbl * tbl = new sTxtTbl(); // Using sTxtTbl to parse the csv table
    tbl->setBuf(filebody, crossRange.length(), 0);
    tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fTopHeader|sTblIndex::fColsep00;
    tbl->parseOptions().colsep = "," __;
    tbl->parse();
    tbl->parseOptions().colsep = 0; // clean up dangling pointers

    idx tblRowLen = tbl->rows();
    for (idx irow = 0; irow < tblRowLen; irow++) { // START of loop over the rows of the crossing ranges file
        sStr reference, startAsString, endAsString;
        tbl->printCell(reference,irow,0);
        tbl->printCell(startAsString,irow,1);
        tbl->printCell(endAsString,irow,2);

        idx start=0, end=0;
        sscanf(startAsString.ptr(),"%" DEC "",&start); // convert sStr to idx
        sscanf(endAsString.ptr(),"%" DEC "",&end);

        for (idx iannot=0; iannot<anotList.dim(); ++iannot){
            sVioAnnot * ia = anotList.ptr(iannot);

            idx idIndex = ia->getIdIndex(reference.ptr(),"seqID"); // get the index for the seqID
            idx typeIdIdx = ia->getIdTypeIdx(); // get the type id index
            idx rangeTypeIdx = ia->getRangeTypeIdx();

            idx relationCnt= 0, relationTypeIndex = 0;
            idx * indexPtrRange= ia->DB.GetRelationPtr(typeIdIdx, idIndex, 1,&relationCnt,&relationTypeIndex); // get the pointer to the vector of list range index

            if (!relationCnt)
                continue;
            idx rCount;
            bool atLeastFoundOne = false;


            idx resultSize=0;
            sVec <sVioAnnot::startEndNode> resStruct;
  PERF_START("Search Virtual Tree");
            resultSize = ia->searchInVirtualTree(indexPtrRange,relationCnt,resStruct,start,end); // use virtualtree funtion to localize what range the start and end are, and return the number of ranges
  PERF_END();
/*
#ifdef _DEBUG
                if (irow%10000 == 0){::printf(" row %" DEC " / %" DEC "\n ==> relationCnt = %" DEC "/%" DEC " ==> %" DEC "-%" DEC "", irow, tblRowLen,relationCnt,start,end);}
#endif
*/
             if (!resultSize) {
                continue;
             }
             for (idx iRange=0; iRange <resStruct.dim(); ++iRange){
                 idx cntIDsForRange=ia->getNberOfIdsByRangeIndex(resStruct[iRange].index); // get the number of IDs for the range

                 for( idx i = 0; i < cntIDsForRange; ++i)  {
                     const char * idPtr,*idTypePtr;
                     ia->getIdTypeByRangeIndexAndIdIndex(resStruct[iRange].index, i, &idPtr, 0, &idTypePtr, 0);
                     if (strcmp(idTypePtr,"seqID")==0) continue;
                     output.printf("\"%s\",%" DEC ",%" DEC ",annotation %" DEC ",%" DEC ",%" DEC ",\"%s\",\"%s\"\n",reference.ptr(0),start,end,iannot+1,resStruct[iRange].ranges->start,resStruct[iRange].ranges->end,idTypePtr,idPtr);
                 }

             }


        }

    }
    return 1;
}
