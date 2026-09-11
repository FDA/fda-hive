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
#include "ncbi-meta.hpp"
#include <ulib/usr.hpp>
#include <slib/core/str.hpp>
#include <ulib/uobj.hpp>
#include <ulib/ufolder.hpp>


NCBI_Meta::NCBI_Meta(const char * biosample, const char * argosWflowID, sUsr * user)
{
    _biosampleAcc.addString(biosample);
    _argosObjID.addString(argosWflowID);
    _objUser = user;
    taxTree = 0;
}

NCBI_Meta::NCBI_Meta(const char * biosample, const char * refAcc, const char * argosWflowID, sUsr * user, bool isGenbank)
{
    if(isGenbank){
        _genbankAcc.addString(refAcc);
    } else {
        _assemblyAcc.addString(refAcc);
    }
    _biosampleAcc.addString(biosample);    
    _argosObjID.addString(argosWflowID);
    _objUser = user;
    taxTree = 0;
}

sJson * NCBI_Meta::getHiveObjJson() {return &_hiveObjJson; }
sJson * NCBI_Meta::getBiosampleJson() { return &_biosampleJson; }
sJson * NCBI_Meta::getAssemblyJson() { return &_assemblyJson; } 
sJson * NCBI_Meta::getGenbankJson() { return &_genbankJson; } 

const char * NCBI_Meta::getBiosample() const { return _biosampleAcc.ptr(); }
const char * NCBI_Meta::getAssembly() const { return _assemblyAcc.ptr(); }
const char * NCBI_Meta::getGenbank() const { return _genbankAcc.ptr(); }
const sStr & NCBI_Meta::getArgosObjID() const { return _argosObjID; }
const sUsr * NCBI_Meta::getUser() const { return _objUser; }
sTaxIon * NCBI_Meta::getTaxTree() { return taxTree; }



bool NCBI_Meta::fetchBioSampleMeta(sHIVENCBI & ncbi) { 
    if(!_biosampleAcc.ptr() || !*_biosampleAcc.ptr()){
        return false;
    }
    
    _biosampleJsonStr.cut(0);
    const char * rawJson = ncbi.getBiosampleMeta(_biosampleAcc.ptr(), &_biosampleJsonStr, 0);
    
    if(!rawJson || !*rawJson){
        return false;
    }

    if(!_biosampleJson.initMem(_biosampleJsonStr.ptr(), _biosampleJsonStr.length(), 0)) {
        return false;
    }

    return true;
}

bool NCBI_Meta::fetchAssemblyMeta(sHIVENCBI & ncbi) { 
    if(!_assemblyAcc.ptr() || !*_assemblyAcc.ptr()){
        return false;
    }

    _assemblyJsonStr.cut(0);
    const char * rawJson = ncbi.saveBiosampleData(_assemblyAcc.ptr(), &_assemblyJsonStr);

    if(!rawJson || !*rawJson){
        return false;
    }

    if(!_assemblyJson.initMem(_assemblyJsonStr.ptr(), _assemblyJsonStr.length())) {
        return false;
    }

    return true;
}

bool NCBI_Meta::fetchGenbankMeta(sHIVENCBI &ncbi){
    if(!_genbankAcc.ptr() || !*_genbankAcc.ptr()){
        return false;
    }

    _genbankJsonStr.cut(0);
    const char * rawJson = ncbi.fetchGenbankData(_genbankAcc.ptr(), &_genbankJsonStr);

    if(!rawJson || !*rawJson){
        return false;
    }

    if(!_genbankJson.initMem(_genbankJsonStr.ptr(), _genbankJsonStr.length())) {
        return false;
    }

    return true;
}

static bool hasNcbiReports(sJson &json)
{
    JSNode root(&json, "$root");
    return root["reports"].ok();
}

bool NCBI_Meta::assembly2HiveStore(const char * filePath) {
    if(_assemblyAcc.ptr() && *_assemblyAcc.ptr() && hasNcbiReports(_assemblyJson)){
        _assemblyJson.file(filePath);
        _assemblyJson.serialize();
        return true;
    }
    
    return false;
}

bool NCBI_Meta::biosample2HiveStore(const char * filePath) {

    if (_biosampleAcc.ptr() && *_biosampleAcc.ptr() && hasNcbiReports(_biosampleJson)) {
        _biosampleJson.file(filePath);
        _biosampleJson.serialize();
        return true;
    }
    
    return false;
}

bool NCBI_Meta::genbank2HiveStore(const char * filePath) {

    if (_genbankAcc.ptr() && *_genbankAcc.ptr() && hasNcbiReports(_genbankJson)) {
        _genbankJson.file(filePath);
        _genbankJson.serialize();
        return true;
    }
    return false;
}

bool NCBI_Meta::log2HiveStore(sFil &metaLog) {
    
    if (_parseLog.ptr() && _parseLog.length() > 0) {
        metaLog.printf("%s", _parseLog.ptr());
        return true;
    }
    return false;
}

bool NCBI_Meta::populateHiveObj(sJson * metaJson, sUsrFolder &dst) { 
    
    sUsrObj metadata(*_objUser,  "argos_ncbi_Metadata");
    sJson ncbiMetadata;
    sHiveId metaId = metadata.Id();
    _objUser->objJson(metaId.objId(), &ncbiMetadata);

    JSNode root=JSNode(metaJson);
    root.link("_id", ncbiMetadata.value("_id"));
    root.link("_type", ncbiMetadata.value("_type"));
    root.link("created", ncbiMetadata.value("created"));
    metaJson->serialize();
    sStr jsonStr;
    idx jsLen = 0;
    const char *jsonCont = metaJson->ret(&jsLen);
    jsonStr.addString(jsonCont, jsLen);
    
    _objUser->propSetJson(0, jsonStr.ptr());
    dst.attach(metadata);
    
    return false; 
}

static bool isAllDigits(const char *s, idx n)
{
    if(!s || n <= 0) return false;
    for(idx i=0; i<n; ++i) {
        char c = s[i];
        if(c < '0' || c > '9') return false;
    }
    return true;
}

static void getAncestors(const char *path, sStr &leaf, sStr &parent, sStr &grand)
{
    leaf.cut(0); parent.cut(0); grand.cut(0);
    if(!path || !*path) return;

    const char *p = path;
    const char *segStart = p;

    const char *lastPtr[3] = {0,0,0};
    idx lastLen[3] = {0,0,0};

    for(; ; ++p) {
        char c = *p;
        if(c == '.' || c == 0) {
            idx segLen = (idx)(p - segStart);
            if(segLen > 0 && !isAllDigits(segStart, segLen)) {
                lastPtr[0] = lastPtr[1]; lastLen[0] = lastLen[1];
                lastPtr[1] = lastPtr[2]; lastLen[1] = lastLen[2];
                lastPtr[2] = segStart;         lastLen[2] = segLen;
            }
            if(c == 0) break;
            segStart = p + 1;
        }
    }

    if(lastPtr[2]) leaf.addString(lastPtr[2], lastLen[2]);
    if(lastPtr[1]) parent.addString(lastPtr[1], lastLen[1]);
    if(lastPtr[0]) grand.addString(lastPtr[0], lastLen[0]);
}

void NCBI_Meta::initNormalizedOut(sJson &out) const {}

static void strictNorm(const char *in, sStr &out)
{
    out.cut(0);

    if (!in || !*in) {
        out.add0(1);
        return;
    }

    std::string tmp;
    std::string_view view(in);
    tmp.reserve(view.size());

    bool prevSep = true;

    for (unsigned char uc : view) {
        char c = static_cast<char>(uc);

        bool isDigit = (c >= '0' && c <= '9');
        bool isUpper = (c >= 'A' && c <= 'Z');
        bool isLower = (c >= 'a' && c <= 'z');
        bool noSymbol = isDigit || isUpper || isLower;

        if (!noSymbol) {
            
            if (!prevSep) {
                tmp.push_back('_');
                prevSep = true;
            }
            continue;
        }

        if (isUpper) {
            c = static_cast<char>(c - 'A' + 'a');
        }

        tmp.push_back(c);
        prevSep = false;
    }

    if (!tmp.empty() && tmp.back() == '_') {
        tmp.pop_back();
    }

    if (tmp.empty()) {
        out.add0(1);
        return;
    }

    out.addString(tmp.c_str());
}

static const char *aliasFallback(const char *field, endpoint ep)
{
    if(ep == eGenbank) {
        if(strcmp(field, "geo_location") == 0) return "location_geographic_location";
        if(strcmp(field, "bioproject") == 0) return "bioprojects";
        if(strcmp(field, "host") == 0) return "host_organism_name";
        if(strcmp(field, "isolate") == 0) return "isolate_name";
        if(strcmp(field, "submitter") == 0) return "submitter_affiliation";
        if(strcmp(field, "isolation_source") == 0) return "isolate_source";
        if(strcmp(field, "num_genes") == 0) return "gene_count";
        return 0;
    }

    if(ep == eBioSample) {
        if(strcmp(field, "biosample") == 0) return "accession";
        if(strcmp(field, "biosample_submission_date") == 0) return "submission_date";
        if(strcmp(field, "bioproject") == 0) return "bioprojects_accession";
        if(strcmp(field, "geo_location") == 0) return "geo_loc_name";
        return 0;
    }

    if(ep == eAssembly) {
        if(strcmp(field, "bioproject") == 0) return "bioprojects_accession";
        if(strcmp(field, "biosample") == 0) return "biosample_accession";
        if(strcmp(field, "num_chromosomes") == 0) return "total_number_of_chromosomes";
        if(strcmp(field, "annotation_provider") == 0) return "annotation_info_provider";
        if(strcmp(field, "num_genes") == 0) return "gene_counts_total";
        if(strcmp(field, "geo_location") == 0) return "geo_loc_name";
        return 0;
    }

    return 0;
}

static const char * getBestVal(sDic<sStr> &dic, const char *key)
{
    if(!key || !*key) return 0;

    sStr *p = dic.get(key);
    if(!p) return 0;

    const char *val = p->ptr();
    if(!val || !*val) return 0;

    return val;
}

static bool isBadValue(const char *v)
{
    if (!v || !*v) return true;

    if(strcmp(v, "-") == 0) return true;
    if(strcmp(v, "unknown") == 0) return true;
    if(strcmp(v, "Unknown") == 0) return true;
    if(strcmp(v, "not provided") == 0) return true;
    if(strcmp(v, "Not Provided") == 0) return true;
    if(strcmp(v, "missing") == 0) return true;
    if(strcmp(v, "Missing") == 0) return true;
    if(strcmp(v, "Not Collected") == 0) return true;
    if(strcmp(v, "not collected") == 0) return true;
    if(strcmp(v, "not applicable") == 0) return true;
    if(strcmp(v, "Not Applicable") == 0) return true;
    if(strcmp(v, "...") == 0) return true;

    return false;
}

void NCBI_Meta::populateJson(sJson &out, sDic<sStr> &bestByKey, const char * const *schemaKeys, endpoint ep, sVec<sStr> *missingFields = 0)
{
    JSNode root(&out, "$root");
    sStr normKey;
    bool filled = false;
    sStr linBuf, taxId;
    for(idx i = 0; schemaKeys[i]; ++i) {

        
        
        filled = false;
        const char *field = schemaKeys[i];

        normKey.cut(0);
        strictNorm(field, normKey);

        sStr *val = bestByKey.get(normKey.ptr());

        if(strcmp(schemaKeys[i], "tax_id") == 0) {
            taxId.addString(val->ptr());
        }

        if(val && val->ptr() && *val->ptr() && !isBadValue(val->ptr())) {
            root.link(field, val->ptr());   
            filled = true;
            continue;
        }

        const char *alias = aliasFallback(field, ep);
        if(alias) {
            normKey.cut(0);
            strictNorm(alias, normKey);
            val = bestByKey.get(normKey.ptr());
            if(val && val->ptr() && *val->ptr() && !isBadValue(val->ptr())) {
                root.link(field, val->ptr());   
                filled = true;
            }
        }

        if (!filled && missingFields) {
            sStr *missing = missingFields->add(1);
            missing->cut(0);
            missing->addString(field);
        }
    }

        linBuf.cut(0);
        taxTree = this->getTaxTree();
        if(taxTree) {
            idx numTaxId = strtoidx(taxId, nullptr, 10);
            const char * lin = taxTree->getLineageByTaxon(numTaxId, &linBuf);
            root.link("lineage", lin);
        }
}

static const char *findSchemaValue(const char *field, sDic<sStr> &bestByKey, endpoint ep)
{
    if(!field) return 0;

    sStr normKey;
    strictNorm(field, normKey);

    sStr *val = bestByKey.get(normKey.ptr());
    if(val && val->ptr() && *val->ptr()) {
        return val->ptr();
    }

    const char *alias = aliasFallback(field, ep);
    if(alias) {
        strictNorm(alias, normKey);
        val = bestByKey.get(normKey.ptr());
        if(val && val->ptr() && *val->ptr()) {
            return val->ptr();
        }
    }

    return 0;
}

static void fillGaps(sJson &out, const sVec<sStr> &missingFields, sDic<sStr> &bioBestByKey)
{
    JSNode root(&out, "$root");

    for(idx i = 0; i < missingFields.dim(); ++i) {
        const char *field = missingFields[i].ptr();
        if(!field || !*field) {
            continue;
        }

        const char *val = findSchemaValue(field, bioBestByKey, eBioSample);
        if(val && *val) {
            root.link(field, val);
        }
    }
}







    



    
        
    

static bool containsDelimitedToken(const char *hay, const char *needle, char delim=';')
{
    if (!hay || !needle || !*needle) return false;

    idx nlen = sLen(needle);
    const char *p = hay;

    while (*p) {
        const char *start = p;

        while (*p && *p != delim) ++p;
        idx len = p - start;

        if (len == nlen && strncmp(start, needle, nlen) == 0)
            return true;

        if (*p == delim) ++p;
    }

    return false;
}





static bool contains(const char *hay, const char *needle)
{
    if (!hay || !needle || !*needle) return false;
    return strstr(hay, needle) != 0;
}

static bool endsWith(const char *s, const char *suffix)
{
    if (!s || !suffix) return false;
    size_t sl = strlen(s);
    size_t tl = strlen(suffix);
    if (tl > sl) return false;
    return memcmp(s + (sl - tl), suffix, tl) == 0;
}

static bool replaceSuffix(sStr &s, const char *oldSuffix, const char *newSuffix)
{
    const char *sp = s.ptr();
    if (!sp || !oldSuffix || !newSuffix) return false;

    size_t sl = strlen(sp);
    size_t ol = strlen(oldSuffix);
    if (ol > sl) return false;

    if (memcmp(sp + (sl - ol), oldSuffix, ol) != 0) return false;

    s.cut((idx)(sl - ol));
    s.addString(newSuffix);
    return true;
}

static void storeBest(sDic<sStr> &bestByKey, const char *normKey, const char *val)
{
    if (!normKey || !*normKey) return;
    if (!val) return;
    if(isBadValue(val)) return;

    sStr &slot = bestByKey[normKey];
    const char *old = slot.ptr();

    if(strcmp(normKey, "bioproject") == 0 || strcmp(normKey, "bioprojects") == 0) {

        if (!old || !*old) {
            slot.cut(0);
            slot.addString(val);
            return;
        }

        if (!containsDelimitedToken(old, val)) {
            slot.addString(";");
            slot.addString(val);
        }
        return;
    }

    if (!old || !*old) {
        slot.cut(0);
        slot.addString(val);
        return;
    }

    if (isBadValue(old) && !isBadValue(val)) {
        slot.cut(0);
        slot.addString(val);
    }
}

static void virusPrecedenceCollision(sDic<sStr> &bestByKey, sDic<sStr> &bioBestByKey, sVec<sStr> &missingFields, sStr *mergeLog = 0)
{
    sStr normField;

    for(idx i = 0; i < missingFields.dim(); ++i) {

        const char *rawField = missingFields[i].ptr();
        if(!rawField || !*rawField) continue;

        normField.cut(0);
        strictNorm(rawField, normField);

        const char *virusVal = getBestVal(bestByKey, normField.ptr());
        const char *bioVal   = getBestVal(bioBestByKey, normField.ptr());

        if(isBadValue(virusVal) && !isBadValue(bioVal)) {
            storeBest(bestByKey, normField.ptr(), bioVal);

            if(mergeLog) {
                mergeLog->printf(
                    "GAP_FILL field='%s' norm='%s' chosen='biosample' value='%s'\n",
                    rawField,
                    normField.ptr(),
                    bioVal
                );
            }
        } else {
            if(mergeLog) {
                mergeLog->printf(
                    "NO_GAP_FILL field='%s' norm='%s' virus='%s' biosample='%s'\n",
                    rawField,
                    normField.ptr(),
                    virusVal ? virusVal : "",
                    bioVal   ? bioVal   : ""
                );
            }
        }
    }

    for(idx i = 0; i < bioBestByKey.dim(); ++i) {

        const char *key = (const char *) bioBestByKey.id(i);
        if(!key || !*key) continue;

        const char *virusVal = getBestVal(bestByKey, key);
        const char *bioVal   = getBestVal(bioBestByKey, key);

        if(isBadValue(bioVal) || isBadValue(virusVal)) continue;

        if(strcmp(virusVal, bioVal) == 0) {
            if(mergeLog) {
                mergeLog->printf(
                    "MATCH key='%s' value='%s'\n",
                    key,
                    virusVal
                );
            }
        } else {
            if(mergeLog) {
                mergeLog->printf(
                    "CONFLICT key='%s' chosen='virus' virus='%s' biosample='%s'\n",
                    key,
                    virusVal,
                    bioVal
                );
            }
        }
    }
}

bool NCBI_Meta::Walker::pushKey(const char *key){
    
    if(!key){
        return false;
    }

    stack.push();
    sStr &top = stack[stack.dim()-1];
    top.addString(key);

    if(path.length()) {
        path.addSeparator(".",1);
    }

    path.addString(key);

    return true;
}

bool NCBI_Meta::Walker::pushIndex(idx index){
    if(index < 0) {
        return false;
    }

    stack.push();
    sStr strIdx;
    strIdx.printf("%lld", index);

    sStr &iTop = stack[stack.dim()-1];
    iTop.addString(strIdx.ptr());
    
    if(path.length()) {
        path.addSeparator(".",1);
    }
    
    path.addString(strIdx.ptr());

    return true;
}

bool NCBI_Meta::Walker::pop() {
    if(!stack.dim()) {
        return false;
    }

    stack.pop();

    path.cut(0);
    for (idx i = 0; i < stack.dim(); ++i) {
        if(i){
            path.addSeparator(".", 1);
        }
        path.addString(stack[i].ptr());
    }

    return true;
}

static bool inPath(const char *path, const char *tok)
{
    if(!path || !tok || !*tok) return false;

    sStr a, b, c;
    a.printf(".%s.", tok);   
    b.printf(".%s", tok);    
    c.printf("%s.", tok);

    if(strcmp(path, tok) == 0) return true;
    if(strstr(path, a.ptr())) return true;
    if(strstr(path, b.ptr())) return true;
    if(strstr(path, c.ptr()) == path) return true;

    return false;
}

static bool isVirusLineage(const char *path)
{
    if(!path) return false;

    if(inPath(path, "lineage")) return true;

    if(strstr(path, "submitter.") && inPath(path, "names")) return true;

    return false;
}

static bool isSafeVirusKey(const char *path, const char *leaf, const char *parent, const char *grand)
{
    if(!path || !leaf || !*leaf) return false;

    if(isVirusLineage(path)) return false;

    if(strcmp(leaf, "organism_name") == 0) {
        return parent && strcmp(parent, "virus") == 0;
    }

    if(strcmp(leaf, "tax_id") == 0) {
        return parent && strcmp(parent, "virus") == 0;
    }

    if(strcmp(leaf, "name") == 0) {
        return false;
    }

    if(strcmp(leaf, "collection_date") == 0) {
        return parent && strcmp(parent, "isolate") == 0;
    }

    if(strcmp(leaf, "geographic_location") == 0) {
        return parent && strcmp(parent, "location") == 0;
    }

    if(strcmp(leaf, "geographic_region") == 0) {
        return parent && strcmp(parent, "location") == 0;
    }

    if(strcmp(leaf, "accession") == 0) return true;
    if(strcmp(leaf, "source_database") == 0) return true;
    if(strcmp(leaf, "release_date") == 0) return true;
    if(strcmp(leaf, "update_date") == 0) return true;
    if(strcmp(leaf, "biosample") == 0) return true;
    if(strcmp(leaf, "length") == 0) return true;
    if(strcmp(leaf, "completeness") == 0) return true;
    if(strcmp(leaf, "protein_count") == 0) return true;

    return true;
}

static void buildBestIndex(const sVec<sStr> &paths, const sVec<sStr> &vals, sDic<sStr> &bestByKey, endpoint ep)
{
    sStr leaf, parent, grand;
    sStr k0, k1, k2, normKey;

    for(idx i = 0; i < paths.dim(); ++i) {
        const char *p = paths[i].ptr();
        const char *v = vals[i].ptr();
        if(!p || !v || !*v) continue;

        getAncestors(p, leaf, parent, grand);
        if(!leaf.length()) continue;

        bool allowK0 = true;
        if(ep == eGenbank) {
            allowK0 = isSafeVirusKey(p, leaf.ptr(), parent.ptr(), grand.ptr());
        }

        if(ep == eBioSample && strcmp(p, "reports.0.accession") == 0) {
            allowK0 = false;
        }

        if(allowK0) {
            k0.cut(0);
            k0.addString(leaf.ptr());
            normKey.cut(0);
            strictNorm(k0.ptr(), normKey);
            storeBest(bestByKey, normKey.ptr(), v);
        }

        if(parent.length()) {
            k1.cut(0);
            k1.addString(parent.ptr());
            k1.addString(".");
            k1.addString(leaf.ptr());

            normKey.cut(0);
            strictNorm(k1.ptr(), normKey);
            storeBest(bestByKey, normKey.ptr(), v);
        }

        if(grand.length() && parent.length()) {
            k2.cut(0);
            k2.addString(grand.ptr());
            k2.addString(".");
            k2.addString(parent.ptr());
            k2.addString(".");
            k2.addString(leaf.ptr());

            normKey.cut(0);
            strictNorm(k2.ptr(), normKey);
            storeBest(bestByKey, normKey.ptr(), v);
        }

        if(ep == eBioSample && strcmp(p, "reports.0.accession") == 0) {
            normKey.cut(0);
            strictNorm("biosample", normKey);
            storeBest(bestByKey, normKey.ptr(), v);
        }
    }

    for(idx i = 0; i < paths.dim(); ++i) {
        const char *p = paths[i].ptr();
        const char *nameVal = vals[i].ptr();
        if(!p || !nameVal) continue;

        if(!endsWith(p, ".name")) continue;
        if(!contains(p, "attributes.")) continue;

        sStr sib;
        sib.cut(0);
        sib.addString(p);
        if(!replaceSuffix(sib, ".name", ".value")) continue;

        const char *attrVal = 0;
        for(idx j = 0; j < paths.dim(); ++j) {
            if(strcmp(paths[j].ptr(), sib.ptr()) == 0) {
                attrVal = vals[j].ptr();
                break;
            }
        }
        if(!attrVal || !*attrVal) continue;

        normKey.cut(0);
        strictNorm(nameVal, normKey);
        storeBest(bestByKey, normKey.ptr(), attrVal);
    }
}

bool NCBI_Meta::parseBioSampleMeta() { 

    sJson *js = getBiosampleJson();
    if(!js || !js->ok()) return false;
    idx rep0 = 0;
    JSNode root(js, "$root");
    JSNode reportsArr = root["reports"];
    JSNode report0 = reportsArr[rep0];
    if(!report0.ok()) return false;

    Walker walker;
    walker.outPaths.cut(0);
    walker.outVals.cut(0);
    walker.path.cut(0);
    walker.walk(report0);

    sDic<sStr> bestByKey;
    buildBestIndex(walker.outPaths, walker.outVals, bestByKey, eBioSample);

    sJson out;
    JSNode outRoot(&_hiveObjJson, "$root");

    outRoot.link("argos_objID", _argosObjID.ptr());

    static const char *schemaKeys[] = {
        "accession","paired_accession","source_database","organism_name","lineage","tax_id",
        "strain","assembly_level","assembly_status","assembly_name","assembly_type",
        "release_date","submitter","sequencing_tech","assembly_method",
        "bioproject","biosample","biosample_submission_date",
        "collected_by","collection_date","geo_location",
        "host","host_disease","isolate","isolate_alias","isolation_source",
        "lat_lon","biosample_strain","host_sex","host_age","culture_collection",
        "host_disease_outcome","host_health_state","host_description","host_disease_stage",
        "num_chromosomes","genome_coverage","annotation_provider","num_genes",
        "parent_accession","bco_id","schema_version","id_method","sample_name","sci_name",
        "type_label","infraspecific_name","title","type_display_text",
        0
    };

    populateJson(_hiveObjJson, bestByKey, schemaKeys, eBioSample);


    return true; 
}
bool NCBI_Meta::parseGenbankMeta() {
    sStr metaLog; 
    sJson *js = getGenbankJson();
    if(!js || !js->ok()) return false;
    idx rep0 = 0;
    JSNode root(js, "$root");
    JSNode reportsArr = root["reports"];
    JSNode report0 = reportsArr[rep0];
    if(!report0.ok()) return false;

    Walker walker;
    walker.outPaths.cut(0);
    walker.outVals.cut(0);
    walker.path.cut(0);
    walker.walk(report0);

    sDic<sStr> bestByKey;
    buildBestIndex(walker.outPaths, walker.outVals, bestByKey, eGenbank);

    JSNode outRoot(&_hiveObjJson, "$root");

    outRoot.link("argos_objID", _argosObjID.ptr());

    static const char *schemaKeys[] = {
        "accession","paired_accession","source_database","organism_name","lineage","tax_id",
        "strain","assembly_level","assembly_status","assembly_name","assembly_type",
        "release_date","submitter","sequencing_tech","assembly_method",
        "bioproject","biosample","biosample_submission_date",
        "collected_by","collection_date","geo_location",
        "host","host_disease","isolate","isolate_alias","isolation_source",
        "lat_lon","biosample_strain","host_sex","host_age","culture_collection",
        "host_disease_outcome","host_health_state","host_description","host_disease_stage",
        "num_chromosomes","genome_coverage","annotation_provider","num_genes",
        "parent_accession","bco_id","schema_version","id_method","sample_name","sci_name",
        "type_label","infraspecific_name","title","type_display_text",
        0
    };

    sVec<sStr> missingFields;
    sDic<sStr> bioBestByKey;

    populateJson(_hiveObjJson, bestByKey, schemaKeys, eGenbank, &missingFields);

    sJson *bioJs = getBiosampleJson();
    if(bioJs && bioJs->ok() && missingFields.dim() > 0) {
        JSNode bioRoot(bioJs, "$root");
        JSNode bioReports = bioRoot["reports"];
        JSNode bioReport0 = bioReports[rep0];

        if(bioReport0.ok()) {
            Walker bioWalker;
            bioWalker.outPaths.cut(0);
            bioWalker.outVals.cut(0);
            bioWalker.path.cut(0);
            bioWalker.walk(bioReport0);

            buildBestIndex(bioWalker.outPaths, bioWalker.outVals, bioBestByKey, eBioSample);

            fillGaps(_hiveObjJson, missingFields, bioBestByKey);
        }
    }

    virusPrecedenceCollision(bestByKey, bioBestByKey, missingFields, &metaLog);

    _parseLog.printf(metaLog);

    return true;
}

bool NCBI_Meta::parseAssemblyMeta() { 

    sJson *js = getAssemblyJson();
    if(!js || !js->ok()) return false;
    idx rep0 = 0;
    JSNode root(js, "$root");
    JSNode reportsArr = root["reports"];
    JSNode report0 = reportsArr[rep0];
    if(!report0.ok()) return false;

    Walker walker;
    walker.outPaths.cut(0);
    walker.outVals.cut(0);
    walker.path.cut(0);
    walker.walk(report0);

    sDic<sStr> bestByKey;
    buildBestIndex(walker.outPaths, walker.outVals, bestByKey, eAssembly);

    sJson out;
    JSNode outRoot(&_hiveObjJson, "$root");
    
    outRoot.link("argos_objID", _argosObjID.ptr());

    static const char *schemaKeys[] = {
        "accession","paired_accession","source_database","organism_name","lineage","tax_id",
        "strain","assembly_level","assembly_status","assembly_name","assembly_type",
        "release_date","submitter","sequencing_tech","assembly_method",
        "bioproject","biosample","biosample_submission_date",
        "collected_by","collection_date","geo_location",
        "host","host_disease","isolate","isolate_alias","isolation_source",
        "lat_lon","biosample_strain","host_sex","host_age","culture_collection",
        "host_disease_outcome","host_health_state","host_description","host_disease_stage",
        "num_chromosomes","genome_coverage","annotation_provider","num_genes",
        "parent_accession","bco_id","schema_version","id_method","sample_name","sci_name",
        "type_label","infraspecific_name","title","type_display_text",
        0
    };

    populateJson(_hiveObjJson, bestByKey, schemaKeys, eAssembly);


    return true;
} 


void NCBI_Meta::Walker::walk(JSNode &node) {

    if (!node.isok) {
        return;
    }
    idx vlen = 0;
    sStr key;


    node.ensureVal();
    switch(node.vType) {

        case JSNode::eObj: {
            JSNode rec = node.child();

            while(rec.ok()) {
                idx klen = 0;
                const char *kptr = rec.atr(&klen);
                if(!kptr || klen <= 0) {
                    break;
                }
                
                key.cutAddString(0, kptr, klen);

                if(!pushKey(key.ptr())) {
                    rec = rec.next(true);
                    continue;
                }

                JSNode childNode = node[(const char*)key.ptr()];

                if(childNode.ok()) {
                    childNode.ensureVal();

                    if((idx)childNode.vType == JSNode::eVal) {
                        vlen = 0;
                        const char *vptr = childNode.val(&vlen);
                        if(vptr && vlen > 0) {
                            sStr *pp = outPaths.add(1);
                            pp->cut(0);
                            pp->addString(path.ptr());

                            sStr *vv = outVals.add(1);
                            vv->cut(0);
                            vv->addString(vptr, vlen);
                        }
                    } else {
                        walk(childNode);
                    }
                }
                pop();
                rec = rec.next(true);
            }
            return;
        }
        case JSNode::eArr: {

            idx n = node.dim();

            for(idx i = 0; i < n; ++i) {
                if(!pushIndex(i)) {
                    continue;
                }
                JSNode el = node[i];
                if(el.ok()) {
                    el.ensureVal();

                    if((idx)el.vType == JSNode::eVal) {
                        vlen = 0;
                        const char *vptr = el.val(&vlen);
                        if(vptr && vlen > 0) {

                            sStr *pp = outPaths.add(1);
                            pp->cut(0);
                            pp->addString(path.ptr());

                            sStr *vv = outVals.add(1);
                            vv->cut(0);
                            vv->addString(vptr, vlen);
                        }
                    } else {
                        walk(el);
                    }
                }
                pop();
            }
            return;
        }
        
        case JSNode::eVal: {
            
            vlen = 0;
            const char *vptr = node.val(&vlen);
            
            if(vptr && vlen > 0) {
               sStr *pp = outPaths.add(1);
                pp->cutAddString(0, path.ptr(), path.length());

                sStr *vv = outVals.add(1);
                vv->cutAddString(0, vptr, vlen);
            }
            return;
        }
        case JSNode::eUndefined:
        default:
            return;
    }
}

void NCBI_Meta::populateTempAssmJson(endpoint ep, const char *assmName, const char *tempAssmAcc, sJson &out, sDic<sStr> &bestByKey, const char * const *schemaKeys) {
    JSNode root(&out, "$root");
    sStr normKey;
    sStr linBuf, taxId;
    for(idx i = 0; schemaKeys[i]; ++i) {

        
        const char *field = schemaKeys[i];

        normKey.cut(0);
        strictNorm(field, normKey);

        sStr *val = bestByKey.get(normKey.ptr());

        if(strcmp(schemaKeys[i], "tax_id") == 0) {
            taxId.addString(val->ptr());
        }

        if(strcmp(field, "accession") == 0) {root.link(field, tempAssmAcc); continue;}
        else if (strcmp(field, "bioproject") == 0) {root.link(field, "PRJNA231221"); continue;}
        else if (strcmp(field, "assembly_level") == 0) {root.link(field, "contig"); continue;}
        else if (strcmp(field, "assembly_method") == 0) {root.link(field, "PMID: 28977510"); continue;}
        else if (strcmp(field, "assembly_method_version") == 0) {root.link(field, "1.0 Babajanyan"); continue;}
        else if (strcmp(field, "assembly_type") == 0) {root.link(field, "haploid"); continue;}
        else if (strcmp(field, "assembly_name") == 0) {root.link(field, assmName); continue;}

        if(val && val->ptr() && *val->ptr() && !isBadValue(val->ptr())) {
            root.link(field, val->ptr());   
            continue;
        }

        const char *alias = aliasFallback(field, ep);
        if(alias) {
            normKey.cut(0);
            strictNorm(alias, normKey);
            val = bestByKey.get(normKey.ptr());
            if(val && val->ptr() && *val->ptr() && !isBadValue(val->ptr())) {
                root.link(field, val->ptr());   
            }
        }
    }

    linBuf.cut(0);
    taxTree = this->getTaxTree();
    if(taxTree) {
        idx numTaxId = strtoidx(taxId, nullptr, 10);
        const char * lin = taxTree->getLineageByTaxon(numTaxId, &linBuf);
        root.link("lineage", lin);
    }
}

bool NCBI_Meta::tempAssmMeta(const char *tempAssmMeta, const char *assmName) {
    sJson *js = getBiosampleJson();
    if(!js || !js->ok()) return false;
    idx rep0 = 0;
    JSNode root(js, "$root");
    JSNode reportsArr = root["reports"];
    JSNode report0 = reportsArr[rep0];
    if(!report0.ok()) return false;

    Walker walker;
    walker.outPaths.cut(0);
    walker.outVals.cut(0);
    walker.path.cut(0);
    walker.walk(report0);

    sDic<sStr> bestByKey;
    buildBestIndex(walker.outPaths, walker.outVals, bestByKey, eBioSample);

    sJson out;
    JSNode outRoot(&_hiveObjJson, "$root");

    outRoot.link("argos_objID", _argosObjID.ptr());

    static const char *schemaKeys[] = {
        "accession","paired_accession","source_database","organism_name","lineage","tax_id",
        "strain","assembly_level","assembly_status","assembly_name","assembly_type",
        "release_date","submitter","sequencing_tech","assembly_method",
        "bioproject","biosample","biosample_submission_date",
        "collected_by","collection_date","geo_location",
        "host","host_disease","isolate","isolate_alias","isolation_source",
        "lat_lon","biosample_strain","host_sex","host_age","culture_collection",
        "host_disease_outcome","host_health_state","host_description","host_disease_stage",
        "num_chromosomes","genome_coverage","annotation_provider","num_genes",
        "parent_accession","bco_id","schema_version","id_method","sample_name","sci_name",
        "type_label","infraspecific_name","title","type_display_text",
        0
    };

    populateTempAssmJson(eBioSample,assmName, tempAssmMeta, _hiveObjJson, bestByKey, schemaKeys);


    return false;
}


bool NCBI_Meta::parseAssemblyMetaTest() { 
    sFil testJs("asmMetaTest1.json");
    sJson test;
    sStr source = testJs.ptr();
    test.initMem(source, source.length());
    JSNode root(&test, "$root");


    idx rep0 = 0;
    JSNode reportsArr = root["reports"];
    JSNode report0 = reportsArr[rep0];
    if(!report0.ok()) return false;

    Walker walker;
    walker.outPaths.cut(0);
    walker.outVals.cut(0);
    walker.path.cut(0);
    walker.walk(report0);

    sDic<sStr> bestByKey;
    buildBestIndex(walker.outPaths, walker.outVals, bestByKey, eAssembly);

    sJson out;
    JSNode outRoot(&_hiveObjJson, "$root");
    
    outRoot.link("argos_objID", _argosObjID.ptr());

    static const char *schemaKeys[] = {
        "accession","paired_accession","source_database","organism_name","lineage","tax_id",
        "strain","assembly_level","assembly_status","assembly_name","assembly_type",
        "release_date","submitter","sequencing_tech","assembly_method",
        "bioproject","biosample","biosample_submission_date",
        "collected_by","collection_date","geo_location",
        "host","host_disease","isolate","isolate_alias","isolation_source",
        "lat_lon","biosample_strain","host_sex","host_age","culture_collection",
        "host_disease_outcome","host_health_state","host_description","host_disease_stage",
        "num_chromosomes","genome_coverage","annotation_provider","num_genes",
        "parent_accession","bco_id","schema_version","id_method","sample_name","sci_name",
        "type_label","infraspecific_name","title","type_display_text",
        0
    };

    populateJson(_hiveObjJson, bestByKey, schemaKeys, eAssembly);

    sStr outTxt;
    _hiveObjJson.serialize();
    outTxt.printf("%s", _hiveObjJson.ret());

    return true;
} 

bool NCBI_Meta::parseGenbankMetaTest() {

    sStr metaLog; 
    sFil testJs("genbankMetaTest3.json");
    sJson test;
    sStr source = testJs.ptr();
    test.initMem(source, source.length());
    JSNode root(&test, "$root");
    idx rep0 = 0;
    JSNode reportsArr = root["reports"];
    JSNode report0 = reportsArr[rep0];
    if(!report0.ok()) return false;

    Walker walker;
    walker.outPaths.cut(0);
    walker.outVals.cut(0);
    walker.path.cut(0);
    walker.walk(report0);

    sDic<sStr> bestByKey;
    buildBestIndex(walker.outPaths, walker.outVals, bestByKey, eGenbank);

    JSNode outRoot(&_hiveObjJson, "$root");

    outRoot.link("argos_objID", _argosObjID.ptr());

    static const char *schemaKeys[] = {
        "accession","paired_accession","source_database","organism_name","lineage","tax_id",
        "strain","assembly_level","assembly_status","assembly_name","assembly_type",
        "release_date","submitter","sequencing_tech","assembly_method",
        "bioproject","biosample","biosample_submission_date",
        "collected_by","collection_date","geo_location",
        "host","host_disease","isolate","isolate_alias","isolation_source",
        "lat_lon","biosample_strain","host_sex","host_age","culture_collection",
        "host_disease_outcome","host_health_state","host_description","host_disease_stage",
        "num_chromosomes","genome_coverage","annotation_provider","num_genes",
        "parent_accession","bco_id","schema_version","id_method","sample_name","sci_name",
        "type_label","infraspecific_name","title","type_display_text",
        0
    };

    sVec<sStr> missingFields;
    sDic<sStr> bioBestByKey;

    populateJson(_hiveObjJson, bestByKey, schemaKeys, eGenbank, &missingFields);

    sFil testJsBio("biosampleMetaTest3.json");
    sJson testBio;
    sStr sourceBio = testJsBio.ptr();
    testBio.initMem(sourceBio, sourceBio.length());
    JSNode bioRoot(&testBio, "$root");
    if( testBio.ok() && missingFields.dim() > 0) {
        JSNode bioRoot(&testBio, "$root");
        JSNode bioReports = bioRoot["reports"];
        JSNode bioReport0 = bioReports[rep0];

        if(bioReport0.ok()) {
            Walker bioWalker;
            bioWalker.outPaths.cut(0);
            bioWalker.outVals.cut(0);
            bioWalker.path.cut(0);
            bioWalker.walk(bioReport0);

            buildBestIndex(bioWalker.outPaths, bioWalker.outVals, bioBestByKey, eBioSample);

            fillGaps(_hiveObjJson, missingFields, bioBestByKey);
        }
    }

    virusPrecedenceCollision(bestByKey, bioBestByKey, missingFields, &metaLog);

    _parseLog.printf(metaLog);
    sStr outTxt;
    _hiveObjJson.serialize();
    outTxt.printf("%s", _hiveObjJson.ret());
    outTxt.add0();

    return true;
}
