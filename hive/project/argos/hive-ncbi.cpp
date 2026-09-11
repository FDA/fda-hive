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
#include "hive-ncbi.hpp"
#include <xlib/s_curl.hpp>

const char * sHIVENCBI::NCBI_BaseURL="https://eutils.ncbi.nlm.nih.gov/entrez/eutils";
const char * sHIVENCBI::Dataset_Base="https://api.ncbi.nlm.nih.gov/datasets";

#define CURL_LOG(_v_format, ...) if(debug) {log.printf(_v_format, __VA_ARGS__);}

#define CURL_CALLDIR(_v_format, ... ) \
    sIO retBuf; \
    sCurl c(&retBuf); \
    char * rslt=0; idx rlen=0;\
    sStr url; url.printf(_v_format,__VA_ARGS__); \
    if(!dst){dst=&m_curlBuf;m_curlBuf.cut(0);} \
    if(debug) {log.printf("CALLING %s.\n",url.ptr(0));} \
    c.Get(url); \
    if(sleepSec){if(sleepStep>0 && sleepStep%sleepGranules==0)sleep(sleepSec);++sleepStep;} \
    if((rlen=retBuf.length())) { \
        retBuf.add0(1); \
        rslt=dst->printf("%.*s",(int)rlen,retBuf.ptr()); \


#define CURL_CALLJSON(_v_json_val,_v_format, ... ) \
    sIO retBuf; \
    sCurl c(&retBuf); \
    char * rslt=0; idx rlen=0;\
    sStr url; url.printf(_v_format,__VA_ARGS__); \
    if(!dst){dst=&m_curlBuf;m_curlBuf.cut(0);} \
    if(debug) {log.printf("CALLING %s.\n",url.ptr(0));} \
    c.Get(url); \
    if(sleepSec){if(sleepStep>0 && sleepStep%sleepGranules==0)sleep(sleepSec);++sleepStep;} \
    if((rlen=retBuf.length())) { \
        retBuf.add0(1); \
        sJson retJ; \
        retJ.initMem(retBuf.ptr(0),retBuf.length()); \
        rslt=(char*)retJ.value(_v_json_val); \
        if(!rslt) \
            return 0; \
        rslt=dst->printf("%s",rslt);



#define CURL_END() \
    return rslt; \
    } \
    return 0;



const char * sHIVENCBI::eSearch(const char * db, const char * term , sStr * dst)
{
    if(!db || ! term) return 0;
    
    CURL_CALLJSON("$root.esearchresult.idlist", "%s/esearch.fcgi?db=%s&term=%s&rettype=uilist&retmode=json",NCBI_BaseURL,db,term)
::printf("################ ESEARCH\nURL=%s\nRSLT=%s\n########################\n",url.ptr(),rslt);
        return rslt;
    CURL_END();
}



const char * sHIVENCBI::eFetch(const char * db, const char * id , const char * mode, sStr * dst)
{
    if(!db || ! id) return 0;
    
    CURL_CALLDIR("%s/efetch.fcgi?db=%s&id=%s&retmode=%s",NCBI_BaseURL,db,id,mode ? mode : "json" )
::printf("################ EFETCH\nURL=%s\nRSLT=%s\n########################\n",url.ptr(),rslt);
        return rslt;
    CURL_END();
}


const char * sHIVENCBI::eLink(const char * dbFrom, const char * id, const char * dbto, const char * linkname, sStr * dst)
{
    if(!dbFrom || ! id || !dbto ) return 0;
    
    CURL_CALLJSON("$root.linksets.0.linksetdbs.0.links","%s/elink.fcgi?dbfrom=%s&id=%s&db=%s&rettype=uilist&retmode=json&linkname=%s",NCBI_BaseURL,dbFrom,id,dbto , linkname ? linkname : "")
::printf("################ ELINK\nURL=%s\nRSLT=%s\n########################\n",url.ptr(),rslt);
        return rslt;
    CURL_END();
}



const char * sHIVENCBI::id2Acc(const char * db, const char * ids, sStr * dst)
{
    if(!db || ! ids) return 0;
    CURL_CALLDIR( "%s/efetch.fcgi?db=%s&id=%s&rettype=acc&retmode=json",NCBI_BaseURL,db,ids)
::printf("################ ID2ACC\nURL=%s\nRSLT=%s\n########################\n",url.ptr(),rslt);
        sString::searchAndReplaceSymbols(dst->ptr(), 0, "\n", ",", 0, true,true,true,true,false);
        if(!dst->length())return 0;
        else {
            char * list=dst->ptr(0);
            if(list[sLen(list)-1]==',')
                list[sLen(list)-1]=0;
        }
    CURL_END();
}

const char * sHIVENCBI::eSummary(const char * db, const char * id, sStr * dst)
{
    if(!db || !id) return 0;
    CURL_CALLDIR("%s/esummary.fcgi?db=%s&id=%s&retmode=json", NCBI_BaseURL, db, id)
    ::printf("################ ESUMMARY\nURL=%s\nRSLT=%s\n########################\n", url.ptr(), rslt);
    return rslt;
    CURL_END();
}

const char * sHIVENCBI::eSummaryXML(const char * db, const char * id, sStr * dst)
{
    if(!db || !id) return 0;
    CURL_CALLDIR("%s/esummary.fcgi?db=%s&id=%s&retmode=xml", NCBI_BaseURL, db, id)
    ::printf("################ ESUMMARY XML\nURL=%s\nRSLT=%s\n############################\n", url.ptr(), rslt);
    return rslt;
    CURL_END();
}

idx sHIVENCBI::extractBioSampleAcc(const char * txt, sStr & outCsv)
{
    outCsv.cut(0);
    if(!txt) return 0;

    sDic<idx> seen;

    const char * prefixes[] = {"SAMN", "SAMEA", "SAMD"};
    const idx nPref = (idx)(sizeof(prefixes)/sizeof(prefixes[0]));

    idx count = 0;
    for(const char * p = txt; *p; ++p) {

        const char * hit = 0;
        idx hitLen = 0;

        for(idx i = 0; i < nPref; ++i) {
            const char * pref = prefixes[i];
            idx len = sLen(pref);

            if(!strncmp(p, pref, len) && p[len] >= '0' && p[len] <= '9') {
                hit = p;
                hitLen = len;
                break;
            }
        }
        if(!hit) continue;

        const char * q = hit + hitLen;
        while(*q >= '0' && *q <= '9') ++q;

        sStr one;
        one.addString(hit, q - hit);

        if(!seen.get(one.ptr())) {
            seen[one.ptr()] = 1;
            if(outCsv.length()) outCsv.addString(",");
            outCsv.addString(one.ptr());
            ++count;
        }

        p = q - 1; 
    }

    return count;
}

static bool xmlTagValue(const char *xml, const char *tag, sStr *dst)
{
    if(!xml || !tag || !dst) return false;

    dst->cut(0);

    sStr openTag, closeTag;
    openTag.printf("<%s>", tag);
    closeTag.printf("</%s>", tag);

    const char *beg = strstr(xml, openTag.ptr());
    if(!beg) return false;

    beg += openTag.length();

    const char *end = strstr(beg, closeTag.ptr());
    if(!end || end <= beg) return false;

    dst->addString(beg, end - beg);
    dst->add0(1);
    return true;
}

const char *sHIVENCBI::getLibaryInfo(const char *srr, sStr *instrumentModel, sStr *libStrategy, sStr *libSource, sStr *libSelection, sStr *libConstProtocol) {
    
    instrumentModel->cut(0);
    libStrategy->cut(0);
    libSource->cut(0);
    libSelection->cut(0);
    libConstProtocol->cut(0);

    if(!srr || !*srr) return 0;

    sStr term;
    term.printf("%s", srr);

    sStr uidBuf;
    const char *uid = eSearch("sra", term.ptr(), &uidBuf);
    if(!uid || !*uid) return 0;

    sStr xml;
    const char *xmlPtr = eFetch("sra", uid, "xml", &xml);
    if(!xmlPtr || !*xmlPtr) return 0;

    if(instrumentModel)
        xmlTagValue(xmlPtr, "INSTRUMENT_MODEL", instrumentModel);

    if(libStrategy)
        xmlTagValue(xmlPtr, "LIBRARY_STRATEGY", libStrategy);

    if(libSource)
        xmlTagValue(xmlPtr, "LIBRARY_SOURCE", libSource);

    if(libSelection)
        xmlTagValue(xmlPtr, "LIBRARY_SELECTION", libSelection);

    if(libConstProtocol)
        xmlTagValue(xmlPtr, "LIBRARY_CONSTRUCTION_PROTOCOL", libConstProtocol);

    return xmlPtr;
}

static void splitNumericIds(const char * s, sVec<sStr> & out)
{
    out.cut(0);
    if(!s) return;

    const char * p = s;
    while(*p) {
        while(*p && !(*p >= '0' && *p <= '9')) ++p;
        if(!*p) break;

        const char * start = p;
        while(*p && (*p >= '0' && *p <= '9')) ++p;

        sStr * tok = out.add(1);
        tok->cut(0);
        tok->addString(start, p - start);
    }
}


const char * sHIVENCBI::biosampleUid2Acc(const char * biosample_uids, sStr * dst)
{
    if(!dst) return 0;
    dst->cut(0);
    if(!biosample_uids || !*biosample_uids) return 0;

    sVec<sStr> uidList;
    splitNumericIds(biosample_uids, uidList);
    if(!uidList.dim()) return 0;

    sDic<idx> seenAcc;
    sStr outCsv;

    for(idx i = 0; i < uidList.dim(); ++i) {
        const char * uid = uidList[i].ptr();
        if(!uid || !*uid) continue;

        const char * js = eSummary("biosample", uid, 0);
        if(!js || !*js) continue;

        sStr jsCopy;
        jsCopy.printf("%s", js);

        sStr accCsv;
        if(extractBioSampleAcc(jsCopy.ptr(), accCsv) <= 0) continue;
        
        sStr tmp;
        tmp.printf("%s", accCsv.ptr());
        for(char * p = tmp.ptr(); p && *p; ) {
            char * start = p;
            char * comma = (char*)strchr(p, ',');
            if(comma) { *comma = 0; p = comma + 1; }
            else { p = start + sLen(start); }

            if(!*start) continue;

            if(!seenAcc.get(start)) {
                seenAcc[start] = 1;
                if(outCsv.length()) outCsv.addString(",");
                outCsv.addString(start);
            }
        }
    }

    if(!outCsv.length()) return 0;
    dst->printf("%s", outCsv.ptr());
    return dst->ptr();
}

const char * sHIVENCBI::srr2biosampleAcc(const char * srr, sStr * dst)
{
    if(!srr || !dst) return 0;
    dst->cut(0);

    sStr term;
    term.printf("%s%%5Baccn%%5D", srr);

    const char * sra_uids = eSearch("sra", term.ptr(), 0);
    if(!sra_uids || !*sra_uids) return 0;

    sStr sraUidsCopy;
    sraUidsCopy.printf("%s", sra_uids);

    const char * biosample_uids = eLink("sra", sraUidsCopy.ptr(), "biosample", "sra_biosample", 0);
    if(!biosample_uids || !*biosample_uids) return 0;

    return biosampleUid2Acc(biosample_uids, dst);
}

const char * sHIVENCBI::srr2biosampleAccXML(const char * srr, sStr * dst)
{
    if(!srr || !dst) return 0;
    dst->cut(0);

    sStr term;
    term.printf("%s%%5Baccn%%5D", srr);

    const char * sra_uids = eSearch("sra", term.ptr(), 0);
    if(!sra_uids || !*sra_uids) return 0;

    sStr sraUidsCopy;
    sraUidsCopy.printf("%s", sra_uids);

    const char * biosample_uids = eLink("sra", sraUidsCopy.ptr(), "biosample", "sra_biosample", 0);
    if(!biosample_uids || !*biosample_uids) return 0;

    sVec<sStr> uidList;
    splitNumericIds(biosample_uids, uidList);
    if(!uidList.dim()) return 0;

    sDic<idx> seenAcc;
    sStr outCsv;

    for(idx i = 0; i < uidList.dim(); ++i) {
        const char * uid = uidList[i].ptr();
        if(!uid || !*uid) continue;

        sStr xmlBuf;
        xmlBuf.cut(0);

        const char * xml = eSummaryXML("biosample", uid, &xmlBuf);
        if(!xml || !*xml) continue;

        sStr accCsv;
        if(extractBioSampleAcc(xml, accCsv) <= 0) continue;

        sStr tmp;
        tmp.printf("%s", accCsv.ptr());
        for(char * p = tmp.ptr(); p && *p; ) {
            char * start = p;
            char * comma = (char*)strchr(p, ',');
            if(comma) { *comma = 0; p = comma + 1; }
            else { p = start + sLen(start); }

            if(!*start) continue;

            if(!seenAcc.get(start)) {
                seenAcc[start] = 1;
                if(outCsv.length()) outCsv.addString(",");
                outCsv.addString(start);
            }
        }
    }

    if(!outCsv.length()) return 0;
    dst->printf("%s", outCsv.ptr());
    return dst->ptr();
}

const char * sHIVENCBI::assembly2genome(const char * assembly, sStr * dst)
{
    const char * id=eSearch("assembly", assembly);
    const char * lnk=eLink("assembly",id,"nuccore", "assembly_nuccore_refseq");
    const char * acc=id2Acc("nuccore", lnk, dst);
    return acc;
}



const char * sHIVENCBI::biosample(const char * acc, sStr * dst, sJson * json)
{
    if(!acc)return 0;
    const char * id=eSearch("biosample", acc);
    const char * src=eFetch("biosample", id,"xml");

    static sJson newJson;if(!json)json=&newJson;
    JSNode root(&newJson);

    sStr d;
    #define fetch(_v_nam,_v_start, _v_end ) {d.cut(0);sString::cleanMarkup(&d, src, 0, _v_start __ , _v_end  __ , "", 0, true, false, false);if(d.length()>3){root.link(_v_nam,d.ptr(1));}}
    #define fetchA(_v_nam,_v_start) fetch( _v_nam, _v_start "\">", "</Attribute>" )
    #define fetchV(_v_nam,_v_start) fetch( _v_nam, _v_start "=\"", "\"" )
    #define fetchS(_v_nam,_v_start) fetch( _v_nam, "<" _v_start ">", "</" _v_start ">" )

    root.link("biosample",acc);
    fetchV("bioproject","bioproject\" label");
    fetchS("title","Title");
    fetch("sample_name","Sample name\">","<");
    fetchV ("taxonomy_id","taxonomy_id");
    fetchV ("taxonomy_name","taxonomy_name");
    fetchS ("organism","OrganismName");
    fetchA("pathogen_name","pathogen scientific name");
    fetchA("pathogen_strain","pathogen strain");
    

    fetchA("strain","strain");
    fetchA("instrument","collection device");
    fetchA("infraspecific_name","isolate");
    fetchA("collected_by","collected by");
    fetchA("collection_date","collection date");
    fetchA("geo_loc_name","geographic location");
    fetchA("lattitude_and_longitude","latitude and longitud");
    fetchA("isolation_source","isolation source");
    fetchA("culture_collection","culture collection");
    fetchA("host","host");
    fetchA("host_age","host age");
    fetchA("host_description","host description");
    fetchA("host_disease","host disease");
    fetchA("host_disease_outcome","host disease outcome");
    fetchA("host_disease_stage","host disease stage");
    fetchA("host_health_state","host health state");
    fetchA("host_sex","host sex");
    fetchA("host_taxid","host taxonomy ID");

    
        
        
    
    



    if(!dst){dst=&m_curlBuf;dst->cut(0);}
    json->print("$root",dst);
    return dst->length() ? dst->ptr() : 0;
}

static bool isNumericIdList(const char *s)
{
    if(!s || !*s) return false;

    for(const char *p = s; *p; ++p) {
        if((*p >= '0' && *p <= '9') || *p == ',' || *p == ' ' || *p == '\n' || *p == '\t')
            continue;
        return false;
    }
    return true;
}

static bool looksLikeSRR(const char *s)
{
    return s &&
           s[1] == 'R' &&
           s[2] == 'R' &&
           s[3] >= '0' &&
           s[3] <= '9';
}

static bool looksLikeRunInfo(const char *txt)
{
    return txt && strncmp(txt, "Run,", 4) == 0;
}

static idx extractSRRAccsFromRunInfo(const char *txt, sStr &outCsv)
{
    outCsv.cut(0);
    if(!txt || !*txt) return 0;

    sDic<idx> seen;
    idx count = 0;

    const char *line = txt;

    while(line && *line) {
        while(*line == '\n' || *line == '\r') ++line;
        if(!*line) break;

        const char *lineEnd = strchr(line, '\n');
        if(!lineEnd) lineEnd = line + sLen(line);

        const char *comma = (const char *)memchr(line, ',', lineEnd - line);
        const char *fieldEnd = comma ? comma : lineEnd;

        sStr run;
        run.addString(line, fieldEnd - line);

        if(looksLikeSRR(run.ptr())) {
            if(!seen.get(run.ptr())) {
                seen[run.ptr()] = 1;
                if(outCsv.length()) outCsv.addString(",");
                outCsv.addString(run.ptr());
                ++count;
            }
        }

        line = (*lineEnd) ? lineEnd + 1 : lineEnd;
    }

    return count;
}

static idx extractSRRAccs(const char *txt, sStr &outCsv)
{
    outCsv.cut(0);
    if(!txt) return 0;

    sDic<idx> seen;
    idx count = 0;

    for(const char *p = txt; *p; ++p) {
        if(!looksLikeSRR(p)) continue;

        const char *q = p + 3;
        while(*q >= '0' && *q <= '9') ++q;

        sStr one;
        one.addString(p, q - p);

        if(!seen.get(one.ptr())) {
            seen[one.ptr()] = 1;
            if(outCsv.length()) outCsv.addString(",");
            outCsv.addString(one.ptr());
            ++count;
        }

        p = q - 1;
    }

    return count;
}

const char * sHIVENCBI::eFetchRunInfo(const char * sra_uids, sStr * dst)
{
    if(!sra_uids || !*sra_uids) return 0;

    CURL_CALLDIR(
        "%s/efetch.fcgi?db=sra&id=%s&rettype=runinfo&retmode=text",
        NCBI_BaseURL,
        sra_uids
    )
    ::printf("################ EFETCH RUNINFO\nURL=%s\nRSLT=%s\n########################\n",
             url.ptr(), rslt);
    return looksLikeRunInfo(rslt) ? rslt : 0;
    CURL_END();
}

const char * sHIVENCBI::sraUid2SRR(const char * sra_uids, sStr * dst)
{
    if(!dst) return 0;
    dst->cut(0);

    ::printf("sraUid2SRR INPUT sra_uids=%s\n",
             sra_uids ? sra_uids : "NULL");

    if(!sra_uids || !*sra_uids) return 0;

    sStr runInfoBuf;
    const char *runInfo = eFetchRunInfo(sra_uids, &runInfoBuf);

    ::printf("sraUid2SRR runInfo=%s\n",
             runInfo ? runInfo : "NULL");

    if(!runInfo || !*runInfo) return 0;

    sStr srrCsv;
    idx n = extractSRRAccsFromRunInfo(runInfo, srrCsv);

    ::printf("sraUid2SRR extracted count=%" DEC "\n", n);
    ::printf("sraUid2SRR srrCsv=%s\n",
             srrCsv.ptr() ? srrCsv.ptr() : "EMPTY");

    if(n <= 0) return 0;

    dst->printf("%s", srrCsv.ptr());
    return dst->ptr();
}

const char * sHIVENCBI::biosampleUid2SRR(const char * biosample_uids, sStr * dst)
{
    if(!dst) return 0;
    dst->cut(0);

    ::printf("biosampleUid2SRR INPUT biosample_uids=%s\n",
             biosample_uids ? biosample_uids : "NULL");

    if(!biosample_uids || !*biosample_uids) return 0;

    sStr sraUidBuf;
    const char *sra_uids = eLink(
        "biosample",
        biosample_uids,
        "sra",
        "biosample_sra",
        &sraUidBuf
    );

    ::printf("biosampleUid2SRR sra_uids=%s\n",
             sra_uids ? sra_uids : "NULL");

    if(!sra_uids || !*sra_uids) return 0;

    const char *srr = sraUid2SRR(sra_uids, dst);

    ::printf("biosampleUid2SRR final srr=%s\n", srr ? srr : "NULL");
    ::printf("biosampleUid2SRR dst=%s\n", dst->ptr() ? dst->ptr() : "EMPTY");

    return srr;
}

const char * sHIVENCBI::assembly2SRR(const char * assembly, sStr * dst)
{
    if(!assembly || !dst) return 0;
    dst->cut(0);

    sStr assemblyIdBuf;
    const char *assembly_ids = 0;

    if(isNumericIdList(assembly)) {
        assemblyIdBuf.printf("%s", assembly);
        assembly_ids = assemblyIdBuf.ptr();
    } else {
        assembly_ids = eSearch("assembly", assembly, &assemblyIdBuf);
    }

    if(!assembly_ids || !*assembly_ids) return 0;

    sVec<sStr> assmUidList;
    splitNumericIds(assembly_ids, assmUidList);
    if(!assmUidList.dim()) return 0;

    sDic<idx> seenSrr;

    for(idx i = 0; i < assmUidList.dim(); ++i) {
        const char *assmUid = assmUidList[i].ptr();
        if(!assmUid || !*assmUid) continue;

        sStr bioUidBuf;
        const char *bioUids = eLink(
            "assembly",
            assmUid,
            "biosample",
            "assembly_biosample",
            &bioUidBuf
        );

        if(!bioUids || !*bioUids) continue;

        sStr srrCsv;
        const char *srrs = biosampleUid2SRR(bioUids, &srrCsv);
        if(!srrs || !*srrs) continue;

        sStr srr00;
        sString::searchAndReplaceSymbols(
            &srr00,
            srrs,
            0,
            ",",
            0,
            0,
            true,
            true,
            true,
            true,
            0
        );

        for(const char *srr = srr00.ptr(0); srr && *srr; srr = sString::next00(srr)) {
            if(!srr || !*srr) continue;

            if(!seenSrr.get(srr)) {
                seenSrr[srr] = 1;

                if(dst->length())
                    dst->addString(",");

                dst->addString(srr);
            }
        }
    }

    return dst->length() ? dst->ptr() : 0;
}

const char * sHIVENCBI::nucUid2SRRFromDBLink(const char * nuc_uid, sStr * dst)
{
    if(!nuc_uid || !dst) return 0;
    dst->cut(0);

    sStr fetchBuf;
    const char *txt = eFetch("nuccore", nuc_uid, "text", &fetchBuf);
    if(!txt || !*txt) return 0;

    sStr srrCsv;
    if(extractSRRAccs(txt, srrCsv) <= 0) return 0;

    dst->printf("%s", srrCsv.ptr());
    return dst->ptr();
}

const char * sHIVENCBI::nucUid2SRRThroughBioSample(const char * nuc_uid, sStr * dst)
{
    if(!nuc_uid || !dst) return 0;
    dst->cut(0);

    sStr biosampleUidBuf;

    const char *biosample_uids = eLink(
        "nuccore",
        nuc_uid,
        "biosample",
        "nuccore_biosample",
        &biosampleUidBuf
    );

    if(!biosample_uids || !*biosample_uids) return 0;

    return biosampleUid2SRR(biosample_uids, dst);
}

const char * sHIVENCBI::nucAcc2CurrentAcc(const char * nuc_acc, sStr * dst)
{
    if(!nuc_acc || !dst) return 0;
    dst->cut(0);

    sStr term;
    term.printf("%s", nuc_acc);

    sStr uidBuf;
    const char *old_uid = eSearch("nuccore", term.ptr(), &uidBuf);
    if(!old_uid || !*old_uid) return 0;

    return id2Acc("nuccore", old_uid, dst);
}

const char * sHIVENCBI::nucseq2BiosampleAccOnce(const char * nuc_acc, sStr * dst)
{
    if(!nuc_acc || !dst) return 0;
    dst->cut(0);

    sStr nucUidBuf;
    eSearch("nuccore", nuc_acc, &nucUidBuf);
    if(!nucUidBuf.length()) return 0;

    sVec<sStr> nucUidList;
    splitNumericIds(nucUidBuf.ptr(), nucUidList);
    if(!nucUidList.dim()) return 0;

    for(idx i = 0; i < nucUidList.dim(); ++i) {
        const char *nucUid = nucUidList[i].ptr();
        if(!nucUid || !*nucUid) continue;

        sStr bioUidBuf;
        const char *bioUids = eLink("nuccore", nucUid, "biosample", "nuccore_biosample", &bioUidBuf);
        if(!bioUids || !*bioUids) continue;

        const char *biosample = biosampleUid2Acc(bioUids, dst);
        if(biosample && *biosample) return biosample;
    }

    dst->cut(0);
    return 0;
}

const char * sHIVENCBI::nucseq2BiosampleAcc(const char * nuc_acc, sStr * dst)
{
    if(!nuc_acc || !dst) return 0;

    static const unsigned retryDelay[] = { 3, 10, 30 };
    const idx retryCount = sizeof(retryDelay) / sizeof(retryDelay[0]);

    for(idx attempt = 0; attempt < retryCount; ++attempt) {
        dst->cut(0);

        const char * result = nucseq2BiosampleAccOnce(nuc_acc, dst);
        if(result && *result) return result;

        log.printf("BioSample lookup failed for %s; attempt %" DEC " of %" DEC "\n",
            nuc_acc, attempt + 1, retryCount);

        if(attempt + 1 < retryCount) sleep(retryDelay[attempt]);
    }

    dst->cut(0);
    return 0;
}

const char * sHIVENCBI::nucseq2SRROnce(const char * nuc_acc, sStr * dst)
{
    if(!nuc_acc || !dst) return 0;
    dst->cut(0);

    sStr nucUidBuf;
    eSearch("nuccore", nuc_acc, &nucUidBuf);

    if(nucUidBuf.length() == 0) return 0;

    sVec<sStr> nucUidList;
    splitNumericIds(nucUidBuf.ptr(), nucUidList);

    if(!nucUidList.dim()) return 0;

    sDic<idx> seenSrr;

    for(idx i = 0; i < nucUidList.dim(); ++i) {
        const char *nucUid = nucUidList[i].ptr();
        if(!nucUid || !*nucUid) continue;

        sStr bioUidBuf;
        const char *bioUids = eLink("nuccore", nucUid, "biosample", "nuccore_biosample", &bioUidBuf);

        if(bioUidBuf.length() == 0) continue;

        sStr srrCsv;
        const char *srrs = biosampleUid2SRR(bioUids, &srrCsv);

        if(!srrs || !*srrs) continue;

        sStr srr00;
        sString::searchAndReplaceSymbols(&srr00, srrs, 0, ",", 0, 0, true, true, true, true, 0);

        for(const char *srr = srr00.ptr(0); srr && *srr; srr = sString::next00(srr)) {
            if(!srr || !*srr) continue;

            if(!seenSrr.get(srr)) {
                seenSrr[srr] = 1;

                if(dst->length())
                    dst->addString(",");

                dst->addString(srr);
            }
        }
    }

    return dst->length() ? dst->ptr() : 0;
}

const char * sHIVENCBI::nucseq2SRR(const char * nuc_acc, sStr * dst)
{
    if(!nuc_acc || !dst) return 0;

    static const unsigned retryDelay[] = { 3, 10, 30 };
    const idx retryCount = sizeof(retryDelay) / sizeof(retryDelay[0]);

    for(idx attempt = 0; attempt < retryCount; ++attempt) {
        dst->cut(0);

        const char * result = nucseq2SRROnce(nuc_acc, dst);
        if(result && *result) return result;

        log.printf("SRR lookup failed for %s; attempt %" DEC " of %" DEC "\n",
            nuc_acc, attempt + 1, retryCount);

        if(attempt + 1 < retryCount) sleep(retryDelay[attempt]);
    }

    dst->cut(0);
    return 0;
}



const char * sHIVENCBI::saveBiosampleData(const char * assmACC, sStr * dst) {
    if(!assmACC || !*assmACC){
        return 0;
    }
    if(!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    } else {
        dst->cut(0);
    }

    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/dataset_report", Dataset_Base, assmACC)
    ::printf("################ FETCH BIOSAMPLE DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);
    
    if(!rslt || !*rslt){
        return 0;
    }

    return dst->ptr();

    CURL_END();
}

const char * sHIVENCBI::fetchGenbankData(const char * gbAcc, sStr * dst) {
    if(!gbAcc || !*gbAcc){
        return 0;
    }
    if(!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    } else {
        dst->cut(0);
    }

    CURL_CALLDIR("%s/v2alpha/virus/accession/%s/dataset_report", Dataset_Base, gbAcc)
    ::printf("################ FETCH BIOSAMPLE DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);
    
    if(!rslt || !*rslt){
        return 0;
    }

    return dst->ptr();

    CURL_END();
}


bool sHIVENCBI::getLineage(const char *taxID, sStr *strLineage, sStr *dst) {

    if (!taxID || !*taxID) {
        strLineage->printf("missing");
        return false;
    }

    CURL_CALLDIR("%s/v2alpha/taxonomy/taxon/%s/dataset_report", Dataset_Base, taxID)
    ::printf("################ FETCH LINEAGE FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);
    
    if (!rslt || !*rslt) {
        strLineage->printf("missing");
        return false;
    }

    if (!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    }

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        strLineage->printf("missing");
        return false;
    }

    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.domain.name", "Not found"));
    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.kingdom.name", "Not found"));
    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.phylum.name", "Not found"));
    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.class.name", "Not found"));
    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.order.name", "Not found"));
    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.family.name", "Not found"));
    strLineage->printf("%s; ", jsonParser.value("reports.0.taxonomy.classification.genus.name", "Not found"));
    strLineage->printf("%s", jsonParser.value("reports.0.taxonomy.classification.species.name", "Not found"));


    return true;

    CURL_END();
}


bool sHIVENCBI::flattenNcbiJson(const char * ncbiRawJson, sJson * strucJson, sStr * outJsonStr, sStr * log, const char *argos_ID) {
    strucJson->link(0, "argos_objID", argos_ID);
    if (!ncbiRawJson || !outJsonStr) return false;

    sJson src, dst;
    if (!ncbiRawJson) {
        if (log) log->printf("Empty or missing input JSON\n");
        return false;
    }
    src.initMem(ncbiRawJson, sLen(ncbiRawJson));

    sStr valBuf;
    const char * val = 0;

    #define FETCH_AND_LINK(json, path, flatKey)                    \
    do {                                                           \
        val = json.value(0, path, 0, &valBuf);                     \
        strucJson->link(0, flatKey, val ? val : "missing");        \
    } while (0)

    FETCH_AND_LINK(src, "reports.0.accession", "accession");
    FETCH_AND_LINK(src, "reports.0.paired_accession", "paired_accession");
    FETCH_AND_LINK(src, "reports.0.source_database", "source_database");

    FETCH_AND_LINK(src, "reports.0.organism.organism_name", "organism_name");
    FETCH_AND_LINK(src, "reports.0.organism.tax_id", "tax_id");

    const char * tax_id = src.value(0, "reports.0.organism.tax_id", 0, &valBuf);
    sStr lineage;
    if (tax_id) {
        getLineage(tax_id, &lineage, 0);
        strucJson->link(0, "lineage", lineage.length() ? lineage.ptr() : "missing");
    } else {
        strucJson->link(0, "lineage", "missing");
    }

    FETCH_AND_LINK(src, "reports.0.organism.infraspecific_names.strain", "strain");

    FETCH_AND_LINK(src, "reports.0.assembly_info.assembly_level", "assembly_level");
    FETCH_AND_LINK(src, "reports.0.assembly_info.assembly_status", "assembly_status");
    FETCH_AND_LINK(src, "reports.0.assembly_info.assembly_name", "assembly_name"); 
    FETCH_AND_LINK(src, "reports.0.assembly_info.assembly_type", "assembly_type");
    FETCH_AND_LINK(src, "reports.0.assembly_info.release_date", "release_date");
    FETCH_AND_LINK(src, "reports.0.assembly_info.submitter", "submitter");
    FETCH_AND_LINK(src, "reports.0.assembly_info.sequencing_tech", "sequencing_tech");
    FETCH_AND_LINK(src, "reports.0.assembly_info.assembly_method", "assembly_method");

    FETCH_AND_LINK(src, "reports.0.assembly_info.bioproject_accession", "bioproject");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.description.title", "sample_name");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.accession", "biosample");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.submission_date", "biosample_submission_date");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.collected_by", "collected_by");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.collection_date", "collection_date");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.geo_loc_name", "geo_location");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.host", "host");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.host_disease", "host_disease");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.isolate", "isolate");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.isolate_name_alias", "isolate_alias");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.isolation_source", "isolation_source");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.lat_lon", "lat_lon");
    FETCH_AND_LINK(src, "reports.0.assembly_info.biosample.strain", "biosample_strain");

    FETCH_AND_LINK(src, "reports.0.assembly_stats.total_number_of_chromosomes", "num_chromosomes");
    FETCH_AND_LINK(src, "reports.0.assembly_stats.genome_coverage", "genome_coverage");

    FETCH_AND_LINK(src, "reports.0.annotation_info.provider", "annotation_provider");
    FETCH_AND_LINK(src, "reports.0.annotation_info.stats.gene_counts.total", "num_genes");

    FETCH_AND_LINK(src, "reports.0.assembly_info.bioproject_lineage.0.bioprojects.0.parent_accessions.0", "parent_accession");


    sVec<const char *> attrKeys(sMex::fSetZero);
    
    attrKeys.add(7);
    attrKeys[0] = "host_sex";
    attrKeys[1] = "host_age";
    attrKeys[2] = "culture_collection";
    attrKeys[3] = "host_disease_outcome";
    attrKeys[4] = "host_health_state";
    attrKeys[5] = "host_description";
    attrKeys[6] = "host_disease_stage";
    
    idx i = 0;
    while (true) {
        sStr namePath, valPath, nameBuf, valueBuf;
        namePath.printf("reports.0.assembly_info.biosample.attributes.%" DEC ".name", i);
        const char *attrName = src.value(0, namePath.ptr(), 0, &nameBuf);
        if (!attrName) break;
    
        if (strcmp(attrName, "identification method") == 0) {
            valPath.printf("reports.0.assembly_info.biosample.attributes.%" DEC ".value", i);
            const char *attrValue = src.value(0, valPath.ptr(), 0, &valueBuf);
            strucJson->link(0, "id_method", attrValue ? attrValue : "missing");
        }
    
        for (idx j = 0; j < attrKeys.dim(); ++j) {
            const char *key = attrKeys[j];
            if (strcmp(attrName, key) == 0) {
                valPath.printf("reports.0.assembly_info.biosample.attributes.%" DEC ".value", i);
                const char *attrValue = src.value(0, valPath.ptr(), 0, &valueBuf);
            
                sStr flatKey;
                for (const char *p = key; *p; ++p)
                    flatKey.printf("%c", *p == ' ' ? '_' : *p);
            
                strucJson->link(0, flatKey.ptr(), attrValue ? attrValue : "missing");
                break;
            }
        }
    
        ++i;
    }


    strucJson->link(0,"bco_id", "BCO:HIVE/ALQC");
    strucJson->link(0, "schema_version", "1.0 Babajanyan");

    return true;
}



const char * sHIVENCBI::getBiosampleData(const char * assmACC, sStr * dst, sJson * json ) {
    if(!assmACC)return 0;

    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/dataset_report", Dataset_Base, assmACC)
    ::printf("################ FETCH BIOSAMPLE DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);


    
    if (!dst) {
        dst = &m_curlBuf; 
        m_curlBuf.cut(0); 
    }

    dst->cut(0);

    parseJson(rslt, dst);
    

    return dst->ptr();; 
    CURL_END();
    

}

const char * sHIVENCBI::parseJson(const char * json, sStr * dst) {
    if (!json || !dst) return 0;

    sJson jsonParser;
    if (!jsonParser.initMem(json)) {
        return 0;
    }


    
    dst->printf("Organism Name: %s\n", jsonParser.value("reports.0.organism.organism_name", "Not found"));
    dst->printf("Taxonomy ID: %s\n", jsonParser.value("reports.0.organism.tax_id", "Not found"));
    dst->printf("Bioproject: %s\n", jsonParser.value("reports.0.assembly_info.bioproject_accession", "Not found"));
    dst->printf("Biosample: %s\n", jsonParser.value("reports.0.assembly_info.biosample.accession", "Not found"));
    dst->printf("Sample Name: %s\n", jsonParser.value("reports.0.assembly_info.biosample.sample_ids.[0].value", "Not found"));
    dst->printf("Collection Date: %s\n", jsonParser.value("reports.0.assembly_info.biosample.collection_date", "Not found"));

    dst->printf("SRA ID: %s\n", jsonParser.value("reports.0.assembly_info.biosample.sample_ids.1.value", "Not found"));
    
    dst->printf("Latitude/Longitude: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='lat_lon')].value", "Not found"));
    dst->printf("Culture Collection: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='culture_collection')].value", "Not found"));
    dst->printf("Host: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host')].value", "Not found"));
    dst->printf("Host Age: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_age')].value", "Not found"));
    dst->printf("Host Description: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_description')].value", "Not found"));
    dst->printf("Host Disease: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_disease')].value", "Not found"));
    dst->printf("Host Disease Outcome: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_disease_outcome')].value", "Not found"));
    dst->printf("Host Disease Stage: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_disease_stage')].value", "Not found"));
    dst->printf("Host Health State: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_health_state')].value", "Not found"));
    dst->printf("Host Sex: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='host_sex')].value", "Not found"));
    dst->printf("Identification Method: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='id_method')].value", "Not found"));
    dst->printf("Lineage: %s\n", jsonParser.value("reports.0.assembly_info.org.parent_tax_id", "Not found"));
    dst->printf("BCO ID: %s\n", jsonParser.value("reports.0.bco_id", "Not found"));
    dst->printf("Schema Version: %s\n", jsonParser.value("reports.0.schema_version", "Not found"));
    dst->printf("Infraspecific Name: %s\n", jsonParser.value("reports.0.organism.infraspecific_names.strain", "Not found"));
    dst->printf("Instrument: %s\n", jsonParser.value("reports.0.assembly_info.sequencing_tech", "Not found"));
    dst->printf("Isolate: %s\n", jsonParser.value("reports.0.assembly_info.biosample.attributes.[?(@.name=='isolate')].value", "Not found"));
    dst->printf("Geolocation: %s\n", jsonParser.value("reports.0.assembly_info.biosample.geo_loc_name", "Not found"));
    dst->printf("Isolation Source: %s\n", jsonParser.value("reports.0.assembly_info.biosample.isolation_source", "Not found"));
    dst->printf("Strain: %s\n", jsonParser.value("reports.0.organism.infraspecific_names.strain", "Not found"));    
    dst->printf("Collected By: %s\n", jsonParser.value("reports.0.assembly_info.biosample.owner.name", "Not found"));


    return dst->ptr();

}

const char * sHIVENCBI::assm2biosample(const char * assmACC, sStr * dst, const char * json, sStr * seqLen) {

    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/dataset_report", Dataset_Base, assmACC)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);
    
    if (!dst) {
        dst = &m_curlBuf; 
        m_curlBuf.cut(0); 
    }

    dst->cut(0);

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return 0;
    }

    dst->printf("%s", jsonParser.value("reports.0.assembly_info.biosample.accession", "Not found"));
    seqLen->printf("%s", jsonParser.value("reports.0.assembly_stats.total_sequence_length"));
    jsonParser.value("reports.0.assembly_info.biosample.accession", nullptr);

    if (strstr(dst->ptr(), "Not found") ) {
        return nullptr;
    }


    return dst->ptr();;
    CURL_END();
}

const char * sHIVENCBI::getRefLen(const char * ref, sStr * dst) {

    CURL_CALLDIR("%s/v2alpha/virus/accession/%s/dataset_report", Dataset_Base, ref)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    if (!dst) {
        dst = &m_curlBuf; 
        m_curlBuf.cut(0); 
    }

    dst->cut(0);

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return 0;
    }

    dst->printf("%s", jsonParser.value("reports.0.length", "Not found"));

    if (strstr(dst->ptr(), "Not found") ) {
        return nullptr;
    }

    return dst->ptr();;
    CURL_END();
}

const char * sHIVENCBI::assm2sra(const char * assmACC, sStr * dst, const char * json) {

    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/dataset_report", Dataset_Base, assmACC)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);
    
    if (!dst) {
        dst = &m_curlBuf; 
        m_curlBuf.cut(0); 
    }

    dst->cut(0);

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return 0;
    }

    dst->printf("%s", jsonParser.value("reports.0.assembly_info.biosample.sample_ids[0].value", "Not found"));

    jsonParser.value("reports.0.assembly_info.biosample.sample_ids[0].value", nullptr);

    if (strstr(dst->ptr(), "Not found") ) {
        return nullptr;
    }


    return dst->ptr();;
    CURL_END();
}

const char * sHIVENCBI::biosample2assm(const char * bsAcc, sStr * dst, const char * json) {

    CURL_CALLDIR("%s/v2alpha/genome/biosample/%s/dataset_report", Dataset_Base, bsAcc)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);
    
    if (!dst) {
        dst = &m_curlBuf; 
        m_curlBuf.cut(0); 
    }

    dst->cut(0);

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return 0;
    }

    dst->printf("Assembly: %s\n", jsonParser.value("reports.0.accession", "Not found"));

    return dst->ptr();;
    CURL_END();
}

const char * sHIVENCBI::getBiosampleMeta(const char * bsAcc, sStr * dst, const char * json) {
    if(!bsAcc || !*bsAcc){
        return 0;
    }
    if(!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    } else {
        dst->cut(0);
    }

    CURL_CALLDIR("%s/v2alpha/biosample/accession/%s/biosample_report", Dataset_Base, bsAcc)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    if(!rslt || !*rslt){
        return 0;
    }

    return dst->ptr();

    CURL_END();
}

const char * sHIVENCBI::biosample2SRA(const char *biosample, sStr *dst, const char *json) {
    CURL_CALLDIR("%s/v2alpha/genome/biosample/%s/dataset_report", Dataset_Base, biosample)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    if (!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    }
    dst->cut(0);

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return nullptr;
    }

    idx i = 0;               
    bool firstSRR = true;    

    while (true) {
        sStr dbKey, valueKey;
        dbKey.printf("reports.0.assembly_info.biosample.sample_ids.%lld.db", i);
        valueKey.printf("reports.0.assembly_info.biosample.sample_ids.%lld.value", i);

        const char *exists = jsonParser.value(valueKey.ptr(), nullptr);
        if (!exists) {
            break; 
        }

        const char *db = jsonParser.value(dbKey.ptr(), nullptr);
        if (!db) {
            ++i; 
            continue;
        }

        if (strcmp(db, "SRA") == 0) {
            const char *sraValue = jsonParser.value(valueKey.ptr(), nullptr);
            if (sraValue && *sraValue) {
                
                sStr srrBuffer;
                const char *srr = srs2srr(sraValue, &srrBuffer);
                if (srr && *srr) {
                    if (!firstSRR) {
                        dst->printf(","); 
                    }
                    dst->printf("%s", srr);
                    firstSRR = false; 
                }
            }
        }

        ++i; 
    }

    if (firstSRR) {
        dst->cut(0);
        return nullptr;
    }

    return dst->ptr(); 
    CURL_END();
}




const char * sHIVENCBI::assm2genome(const char * assembly, sStr * dst, sStr * outRefseqAssembly) {
    
    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/sequence_reports", Dataset_Base, assembly)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    if (!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    }
    dst->cut(0);

    if (outRefseqAssembly) {
        outRefseqAssembly->cut(0);
    }

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return nullptr;
    }

    int i = 0;

    while (true) {
        sStr reportKey;
        reportKey.printf("reports.%d", i);

        const char *exists = jsonParser.value(reportKey.ptr(), nullptr);
        if (!exists) {
            break;
        }

        sStr pairedKey, refseqKey, genbankKey, refseqAssemblyKey;
        pairedKey.printf("%s.paired_accession", reportKey.ptr());
        refseqKey.printf("%s.refseq_accession", reportKey.ptr());
        genbankKey.printf("%s.genbank_accession", reportKey.ptr());
        refseqAssemblyKey.printf("%s.refseq_assembly_accession", reportKey.ptr());

        const char *pairedAcc = jsonParser.value(pairedKey.ptr(), nullptr);
        const char *refseqAcc = jsonParser.value(refseqKey.ptr(), nullptr);
        const char *genbankAcc = jsonParser.value(genbankKey.ptr(), nullptr);
        const char *refseqAssembly = jsonParser.value(refseqAssemblyKey.ptr(), nullptr);

        const char *chosenGenomeAcc = nullptr;

        if (pairedAcc && strncmp(pairedAcc, "GCF_", 4) == 0) {
            chosenGenomeAcc = pairedAcc;

            if (outRefseqAssembly && refseqAssembly && *refseqAssembly) {
                outRefseqAssembly->printf("%s", refseqAssembly);
            }

            dst->printf("%s", chosenGenomeAcc);
            return dst->ptr();
        }

        if (refseqAcc && *refseqAcc) {
            chosenGenomeAcc = refseqAcc;

            if (outRefseqAssembly && refseqAssembly && *refseqAssembly) {
                outRefseqAssembly->printf("%s", refseqAssembly);
            }

            dst->printf("%s", chosenGenomeAcc);
            return dst->ptr();
        }

        if (genbankAcc && *genbankAcc) {
            dst->printf("%s", genbankAcc);
            return dst->ptr();
        }

        ++i;
    }

    dst->cut(0);
    return nullptr;

    CURL_END();
}

const char * sHIVENCBI::srs2srr(const char * srsId, sStr * dst) {
    if (!srsId || !dst) return nullptr;

    sStr idSearchResult;
    const char *internalId = eSearch("sra", srsId, &idSearchResult);
    if (!internalId || !*internalId) {
        ::printf("No internal ID found for SRS/ERS/DRS: %s\n", srsId);
        return nullptr;
    }

    sStr fetchResult;
    const char *metadata = eFetch("sra", internalId, "xml", &fetchResult);
    if (!metadata || !*metadata) {
        ::printf("Failed to fetch metadata for internal ID: %s\n", internalId);
        return nullptr;
    }

    const char *buf = fetchResult.ptr();
    const char *bufEnd = buf + fetchResult.length();

    dst->cut(0);

    const char *p = buf;
    while (p && p < bufEnd) {
        const char *runTagStart = strstr(p, "<RUN");
        if (!runTagStart || runTagStart >= bufEnd) break;

        const char *tagEnd = strchr(runTagStart, '>');
        if (!tagEnd) break;

        const char *accessionAttr = strstr(runTagStart, "accession=\"");
        if (accessionAttr && accessionAttr < tagEnd) {
            accessionAttr += 11;
            const char *accessionEnd = strchr(accessionAttr, '"');
            if (accessionEnd && accessionEnd <= tagEnd) {
                idx accLen = accessionEnd - accessionAttr;
                if (accLen > 0) {
                    bool already = false;
                    const char *q = dst->ptr();
                    const char *dstEnd = q + dst->length();
                    while (q < dstEnd) {
                        const char *nextComma = (const char*)memchr(q, ',', dstEnd - q);
                        const char *tokEnd = nextComma ? nextComma : dstEnd;
                        idx tokLen = tokEnd - q;
                        if (tokLen == accLen && strncmp(q, accessionAttr, (size_t)accLen) == 0) {
                            already = true;
                            break;
                        }
                        if (!nextComma) break;
                        q = nextComma + 1;
                    }

                    if (!already) {
                        if (dst->length() > 0) dst->add(",", 1);
                        dst->add(accessionAttr, accLen);
                    }
                }
            }
        }

        p = tagEnd + 1;
    }

    if (!dst->length()) {
        ::printf("No <RUN accession=\"...\"> entries found for sample: %s\n", srsId);
        return nullptr;
    }

    dst->add0cut();

    return dst->ptr();
}


const char * sHIVENCBI::getReferences(const char * assembly, sStr * dst, bool *isRefseq) {
    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/sequence_reports", Dataset_Base, assembly)
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    if (!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    }
    dst->cut(0);

    if (isRefseq) {
        *isRefseq = false;
    }

    sJson jsonParser;
    if (!jsonParser.initMem(rslt)) {
        return nullptr;
    }

    bool hasRefseq = false, hasGenbank = false;
    bool firstAccession = true;

    idx i = 0;
    while (true) {
        sStr refseqKey, genbankKey;
        refseqKey.printf("reports.%lld.refseq_accession", i);
        genbankKey.printf("reports.%lld.genbank_accession", i);

        const char *refseq = jsonParser.value(refseqKey.ptr(), nullptr);
        const char *genbank = jsonParser.value(genbankKey.ptr(), nullptr);

        if (!refseq && !genbank) {
            break;
        }

        if (refseq && *refseq) {
            if (!firstAccession) dst->printf(";");
            dst->printf("%s", refseq);
            hasRefseq = true;
            firstAccession = false;
        }

        if (!hasRefseq && genbank && *genbank) {
            if (!firstAccession) dst->printf(";");
            dst->printf("%s", genbank);
            hasGenbank = true;
            firstAccession = false;
        }

        ++i;
    }

    if (hasRefseq && isRefseq) {
        *isRefseq = true;
    } else if (!hasRefseq && hasGenbank && isRefseq) {
        *isRefseq = false;
    }
    if (firstAccession) {
        dst->cut(0);
        return nullptr;
    }

    return dst->ptr();
    CURL_END();
}

const char * sHIVENCBI::getRefSeqAssemblyAcc(const char * assembly, sStr * dst) {
    CURL_CALLDIR("%s/v2alpha/genome/accession/%s/dataset_report", Dataset_Base, assembly);
    ::printf("################ FETCH DATA FROM NCBI\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    if (!dst) {
        dst = &m_curlBuf;
    }
    dst->cut(0);

    if (!rslt || !*rslt) {
        ::printf("ERROR: Empty or null response from NCBI.\n");
        return nullptr;
    }

    sJson jsonParser;
    bool memCheck = jsonParser.initMem(rslt, sLen(rslt));
    if (!memCheck) {
        ::printf("ERROR: Failed to parse JSON from rslt.\n");
        return nullptr;
    }

    const char *paired = jsonParser.value("reports.0.paired_accession", nullptr);
    if (paired && *paired && strncmp(paired, "GCF_", 4) == 0) {
        dst->printf("%s", paired);
        return dst->ptr();
    }

    ::printf("INFO: No GCF_ RefSeq paired_accession found. Falling back.\n");
    return nullptr;
    CURL_END();
}


bool sHIVENCBI::getUniProtData(const char *speciesName, sJson *strucJson) {
    if (!speciesName || !strucJson) return false;

    sStr searchBuf, proteomeIDBuf, genomeBuf;

    const char *jsonStr = searchUniProt(speciesName, &searchBuf);
    const char *proteomeID = getUniProtProteomeID(jsonStr, &proteomeIDBuf, speciesName);


    if (!proteomeID) return false;

    const char *genomeAcc = 0;
    if (proteomeID) {
        genomeAcc = mapUniProtID(proteomeID, &genomeBuf);
    }

    strucJson->link(0, "representative_genome_org", speciesName);
    strucJson->link(0, "representative_proteome_uniprot_acc", proteomeID ? proteomeID : "missing");
    strucJson->link(0, "representative_genome_acc", genomeAcc ? genomeAcc : "missing");

    return true;
}





const char * sHIVENCBI::searchUniProt(const char *speciesName, sStr *dst) {
    if (!speciesName) return 0;

    sStr encodedName;
    for (const char *p = speciesName; *p; p++) {
        switch (*p) {
            case ' ':
                encodedName.printf("%%20");
                break;
            case '"':
                encodedName.printf("%%22");
                break;
            case ':':
                encodedName.printf("%%3A");
                break;
            case '/':
                encodedName.printf("%%2F");
                break;
            case '?':
                encodedName.printf("%%3F");
                break;
            case '&':
                encodedName.printf("%%26");
                break;
            case '=':
                encodedName.printf("%%3D");
                break;
            case '+':
                encodedName.printf("%%2B");
                break;
            case ',':
                encodedName.printf("%%2C");
                break;
            default:
                encodedName.printf("%c", *p);
                break;
        }
    }

    sStr queryStr;
    queryStr.printf("taxonomy_name%%3A%%22%s%%22", encodedName.ptr());

    CURL_CALLDIR("https://rest.uniprot.org/proteomes/search?query=%s&format=json", queryStr.ptr());
    ::printf("################ FETCH DATA FROM UNIPROT\nURL=%s\nRSLT=%s\n########################\n\n\n", url.ptr(), rslt);

    rslt = retBuf.ptr();
    if (!dst) {
        dst = &m_curlBuf;
        m_curlBuf.cut(0);
    }
    dst->printf("%s", rslt);
    return dst->ptr();

    CURL_END();
}


const char * sHIVENCBI::getUniProtProteomeID(const char *uniprotJson, sStr *buf, const char *speciesName) {
    if (!uniprotJson) return 0;
    sJson src;
    src.initMem(uniprotJson, sLen(uniprotJson));
    sStr valBuf;

    for (idx i = 0;; ++i) {
        sStr typePath, idPath, panPath;
        typePath.printf("results.%" DEC ".proteomeType", i);
        idPath.printf("results.%" DEC ".id", i);
        panPath.printf("results.%" DEC ".panproteome", i);

        const char *ptype = src.value(0, typePath, 0, &valBuf);
        const char *pid = src.value(0, idPath, 0, &valBuf);
        const char *pan = src.value(0, panPath, 0, &valBuf);

        if (!ptype && !pid && !pan) break;

        if (ptype && strcmp(ptype, "Representative") == 0 && pid) {
            return buf->printf("%s", pid);
        }
        if (pan) {
            return buf->printf("%s", pan);
        }
    }

    return 0;
}

const char * sHIVENCBI::mapUniProtID(const char *proteomeID, sStr *dst) 
{
    if (!proteomeID) return 0;

    CURL_CALLDIR("https://rest.uniprot.org/proteomes/%s?format=json", proteomeID);
    rslt = retBuf.ptr();

    sJson json;
    json.initMem(rslt, sLen(rslt));

    sStr valBuf;
    const char *genomeAcc = json.value(0, "genomeAssembly.assemblyId", 0, &valBuf);
    if (genomeAcc && genomeAcc[0]) {
        if (dst) {
            dst->cut(0);
            dst->printf("%s", genomeAcc);
            return dst->ptr();
        } else {
            return genomeAcc;
        }
    }

    for (idx i = 0;; ++i) {
        sStr dbPath, idPath;
        dbPath.printf("components.%" DEC ".proteomeCrossReferences.0.database", i);
        idPath.printf("components.%" DEC ".proteomeCrossReferences.0.id", i);
        const char *db = json.value(0, dbPath, 0, &valBuf);
        const char *gid = json.value(0, idPath, 0, &valBuf);
        if (!db && !gid) break;
        if (db && strcmp(db, "GenomeAccession") == 0 && gid) {
            if (dst) {
                dst->cut(0);
                dst->printf("%s", gid);
                return dst->ptr();
            } else {
                return gid;
            }
        }
    }

    return 0;
    CURL_END();
}




    


