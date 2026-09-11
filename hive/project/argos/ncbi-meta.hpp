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

#include "hive-ncbi.hpp"
#include <ulib/usr.hpp>
#include <slib/core/str.hpp>
#include <ion/sJson.hpp>
#include <slib/std/string.hpp>
#include <violin/violin.hpp>

using namespace slib;

enum endpoint {
        eAssembly,
        eGenbank,
        eBioSample,
        eTempAssm
};


class NCBI_Meta
{
public:
    NCBI_Meta(const char * biosample, const char * argosWflowID, sUsr * user);
    NCBI_Meta(const char * biosample, const char * refAcc, const char * argosWflowID, sUsr * user, bool isGenbank);

    sJson * getBiosampleJson();
    sJson * getAssemblyJson();
    sJson * getGenbankJson();
    sJson * getHiveObjJson();
    const char * getBiosample() const;
    const char * getAssembly() const;
    const char * getGenbank() const;
    const sStr & getArgosObjID() const;
    const sUsr * getUser() const;
    sTaxIon * getTaxTree();


    bool fetchBioSampleMeta(sHIVENCBI & ncbi);
    bool fetchAssemblyMeta(sHIVENCBI & ncbi);
    bool fetchGenbankMeta(sHIVENCBI & ncbi);
    bool assembly2HiveStore(const char * filePath);
    bool biosample2HiveStore(const char * filePath);
    bool genbank2HiveStore(const char * filePath);
    bool log2HiveStore(sFil &metaLog);
    void populateJson(sJson &out, sDic<sStr> &bestByKey, const char * const *schemaKeys, endpoint ep, sVec<sStr> *missingFields);

    bool parseBioSampleMeta();
    bool parseAssemblyMeta();
    bool parseGenbankMeta();
    bool parseBioSampleMetaTest();
    bool parseAssemblyMetaTest();
    bool parseGenbankMetaTest();
    void initNormalizedOut(sJson &out) const;
    bool setFields(const char *field, const char *val, bool isAssemblySource);
    bool tempAssmMeta(const char *tempAssmAcc, const char *assmName);
    void populateTempAssmJson(endpoint ep, const char *assmName, const char *tempAssmAcc, sJson &out, sDic<sStr> &bestByKey, const char * const *schemaKeys);

    bool populateHiveObj(sJson * metaJson, sUsrFolder &dst);
    sTaxIon *taxTree;

private:
    sJson _hiveObjJson;
    sJson _biosampleJson;
    sJson _assemblyJson;
    sJson _genbankJson;
    sStr _biosampleAcc;
    sStr _assemblyAcc;
    sStr _genbankAcc;
    sUsr * _objUser;
    sStr _argosObjID;
    sStr _biosampleJsonStr;
    sStr _assemblyJsonStr;
    sStr _genbankJsonStr;
    sStr _parseLog;
    

    struct Walker {
        sStr path;
        sStack<sStr> stack;
        sVec<sStr> outPaths;
        sVec<sStr> outVals;
        bool pushKey(const char *key);
        bool pushIndex(idx index);
        bool pop();
        void walk(JSNode &node);
    };
};
