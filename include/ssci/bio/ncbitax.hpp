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
#ifndef sLib_NCBITaxTree_hpp
#define sLib_NCBITaxTree_hpp

#include <slib/core/str.hpp>
#include <slib/utils/sVioDB.hpp>

using namespace slib;

class sNCBITaxTree
{
    private:
        const char * getonelinefromBuffer(char * buf, const char * fileContent);
        void ParseOneline(idx idCnt, const char * buf, sStr * string, idx * id, ...);

        idx getParentByIndex(idx index=1);

        bool getNameByIndex(sStr * buf, idx index=1, idx whichName = 1);
        bool getAllNameByIndex(sStr * buf, idx index=1);
        bool getPathByIndex(sStr * buf, idx index=1, idx WhatToPrintFlags=ePath, const char * RankFilters=0, bool ifPrintRoot = true);

    public:
        sVioDB DB;
        enum EColumns {
            eTaxId= 0x00000001,
            eParent=0x00000002,
            eName=  0x00000004,
            eRank=  0x00000008,
            ePath=  0x00000010,
            ePathID=0x00000020,
            eCntChildren=0x00000040,
            eMatchName=0x00000080,
            eAllName=0x00000100,
            eProjectID=0x00000200,
            eDefPath=0x00000400,
            eMatchCnt=0x00000800,
            eNum=0x00001000,
            eMin=0x00002000,
            eMax=0x00004000,
            eMean=0x00008000,
            eStdDev=0x00001000,
            eConfIntval=0x00020000,
            eOutFile=0x00040000
            //eGI=0x00000400,
        };
        static const char * columnNames; // defined in ncbitax.cpp, keep in sync with EColumns!
        static const char * printColumnNames(sStr & out, idx whatToPrintflags = eTaxId);
        struct ExtraData {
            idx matchCnt;
            real num, min, max, mean, stddev, confIntval;
            ExtraData() { sSet(this, 0); }
        };
    public: 
       sNCBITaxTree(const char * InputFilename=0)
        {
            init(InputFilename);
        }

        sNCBITaxTree * init (const char * InputFilename=0)
        {
            if(InputFilename) DB.init(InputFilename,0,0,0,sMex::fReadonly);
            return this;
        }
        bool isok(void)
        {
            return DB.isok()? true : false;
        }
        idx getNodeIndex(idx taxid=1);
        idx getTaxIdCnt(void);
        idx getNodeIndexByProID(idx projectID);
        //idx getNodeIndexByGiID(idx GiID);
        //idx getNodeIndexByGiID(const char* GiID);
        bool getNameTaxid(sVec <idx> * TaxidList, const char * srch);
        //bool getNameListIndex(sVec <idx> * lontosa, const char * srch = 0);
        idx getParentIndex(idx taxid=1);
        idx getChildCnt(idx taxid=1);
        idx getParentCnt(idx taxid=1);
        idx getNodeCnt();
        idx getTaxIdByIndex(idx index = 1);
        idx getProjIDByIndex(idx index=1);
        //idx getGIByIndex(idx index=1,sStr * buf=0);
        const char * getRankByIndex(idx index=1);
        idx getRankCount(const char * rank );
        bool IfNameMatch(idx index = 1, const char * srch = 0, idx * matchNameIndex=0 );
        bool getChildrenIndexList(sVec <idx> * lontosa, idx taxid=1, const char * RankFilters=0, idx depth=1);
        bool getParentIndexList(sVec <idx> * lontosa, idx taxid=1,  idx depth=1);
        void printByFlags(sStr *buf, idx index=1, idx whatToPrintflags = eTaxId,idx whichName =1, idx matchNameIndex = 1,const char * RankFilters=0,bool ifPrintRoot = true, const ExtraData * extra=0);
        void Parsefile(const char* src, const char* nsrc, const char* inputfilename,bool combinedFile=true);
};

#endif
