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
#include <violin/violin.hpp>
#include <slib/utils/json/parser.hpp>

enum enumIonCommands
{
    eIonNcbiTax,
    eIonTaxInfo,
    eIonTaxDownInfo,
    eIonTaxParent,
    eIonTaxPathogenInfo,
    eIonAnnotInfo,
    eIonAnnotIdMap,
    eIonTaxidCollapse,
    eIonTaxidByName,
    eIonCodonDB,
    eExtendCodonTable,
    eIonWander,
    eIonTaxidCollapseExt
};


idx extractIonQueryfromTable(sIonWander &iWander, sTxtTbl &tbl, const char *ionQuery, const char *defaultValue = 0, bool parseiWander = true)
{

    idx cntRows = tbl.rows();
    idx cntCols = tbl.cols();
    sVec<idx> icols;

    if (parseiWander){
        iWander.traverseCompile(ionQuery);
    }
    icols.add(iWander.parametricArguments.dim());
    idx plen;
    for(idx ipa = 0; ipa < iWander.parametricArguments.dim(); ++ipa) {
        const char *param = ((const char *) iWander.parametricArguments.id(ipa, &plen));
        ++param;
        --plen;
        idx colArg = sNotIdx;

        if( isdigit(param[0]) ) {
            colArg = atoidx(param);
        } else {
            idx clen;
            for(idx icol = 0; icol < cntCols; ++icol) {
                const char * cell = tbl.cell(-1, icol, &clen);
                if( (plen == clen) && memcmp(param, cell, plen) == 0 ) {
                    colArg = icol;
                    break;
                }
            }
        }
        icols[ipa] = colArg;
    }
    idx oldLength = sNotIdx;
    sStr cbuf;
    for(idx i = 0; i < cntRows; i++) {
        cbuf.cut(0);
        bool need_unquote = false;
        for(idx j = 0; j < icols.dim(); ++j) {
            idx plen;
            const char * pid = (const char *) iWander.parametricArguments.id(j, &plen);
            idx *toModify = (idx*) iWander.getSearchDictionaryPointer(pid, plen);

            idx cell_len = 0;
            const char * cell = tbl.cell(i, icols[j], &cell_len);
            if( cell && cell_len ) {
                if( cell[0] == '"' ) {
                    toModify[0] = cbuf.length();
                    toModify[1] = -123;
                    need_unquote = true;

                    tbl.printCell(cbuf, i, icols[j]);
                    cbuf.add0();
                } else {
                    toModify[0] = sConvPtr2Int(cell);
                    toModify[1] = cell_len;
                }
            } else {
                toModify[0] = 0;
                toModify[1] = 0;
            }
        }

        if( need_unquote ) {
            for(idx j = 0; j < icols.dim(); ++j) {
                const char * pid = (const char *) iWander.parametricArguments.id(j, &plen);
                idx *toModify = (idx*) iWander.getSearchDictionaryPointer(pid, plen);
                if( toModify[1] == -123 ) {
                    const char * unquoted_cell = cbuf.ptr(toModify[0]);
                    toModify[0] = sConvPtr2Int(unquoted_cell);
                    toModify[1] = sLen(unquoted_cell);
                }
            }
        }

        iWander.traverse();

        if( defaultValue && ((oldLength == iWander.traverseBuf.length()) || (iWander.traverseBuf.length() == 0)) ) {
            iWander.traverseBuf.addString(defaultValue);
            if( *iWander.traverseRecordSeparator == 0 ) {
                iWander.traverseBuf.add0();
            } else {
                iWander.traverseBuf.addString(iWander.traverseRecordSeparator);
            }
        }
        oldLength = iWander.traverseBuf.length();
    }

    return 0;
}

idx parseFilters(sTxtTbl &tbl, idx *colArg, const char *filterStr, const char *filterCol)
{
    idx cntCols = tbl.cols();
    if (!cntCols){
        return 0;
    }
    if (!filterStr || !filterCol){
        return 0;
    }

    idx clen;
    idx plen = sLen (filterCol);
    bool isFound = false;
    for(idx icol = 0; icol < cntCols; ++icol) {
        const char * cell = tbl.cell(-1, icol, &clen);
        if( (plen == clen) && memcmp(filterCol, cell, plen) == 0 ) {
            *colArg = icol;
            isFound = true;
            break;
        }
    }

    return isFound ? sLen(filterStr) : 0;


}

static bool findDatabaseId(sHiveId & result, const sUsr & user, idx lookfordb = 0)
{
    switch(lookfordb) {
        case 0:
            return sviolin::SpecialObj::findTaxDb(result, user, "2");
        case 1:
            return sviolin::SpecialObj::findTaxDb(result, user, "3");
        case 2:
            return sviolin::SpecialObj::find(result, user, "ionCodon", "2");
    }
    result = sHiveId::zero;
    return false;
}

namespace {
    class FilterList00 : public sStr {
        public:
            FilterList00(const char * inp)
            {
                if( inp && inp[0] == '[' ) {
                    sJSONParser parser;
                    if( parser.parse(inp) && parser.result().isList() ) {
                        for(idx i = 0; i < parser.result().dim(); i++) {
                            addString(parser.result().getListElt(i)->asString());
                            add0();
                        }
                    }
                } else {
                    addString(inp);
                }
                add0(2);
            }

            idx toColIDs(sVec<idx> & out, const sTabular & tbl, const char ** alternatives00 = 0, idx cnt_alternatives00 = 0)
            {
                idx cnt_found = 0;
                for(const char * colname = ptr(); colname && colname[0]; colname = sString::next00(colname)) {
                    idx col_id = tbl.colId(colname);
                    while(col_id < 0) {
                        for(idx ialt = 0; (col_id < 0) && (ialt < cnt_alternatives00); ialt++) {
                            const char * altcolname0 = alternatives00[ialt];
                            if( sIsExactly(colname, altcolname0) ) {
                                for(const char * altcolname = sString::next00(altcolname0); col_id < 0 && altcolname && *altcolname; altcolname = sString::next00(altcolname)) {
                                    col_id = tbl.colId(altcolname);
                                }
                            }
                        }
                    }

                    if( col_id >= 0 ) {
                        *out.add(1) = col_id;
                        cnt_found++;
                    }
                }
                return cnt_found;
            }
    };
};


idx DnaCGI::CmdIon(idx cmd)
{

    sIonWander iWander;
    sStr statement;
    sStr myPath;

    sStr ionP, error_log;
    const char *ver = pForm->value("version",0);
    if( !sviolin::SpecialObj::findTaxDbIonPath(ionP, *user, ver, 0, &error_log) ) {
        error(error_log.ptr());
        outHtml();
        return 1;
    }

    iWander.addIon(0)->ion->init(ionP.ptr(), sMex::fReadonly);

    sIonWander genericWander;
    switch(cmd) {

        case eIonWander :{

               sVec<sHiveId> ionObjs;
               sHiveId::parseRangeSet(ionObjs, pForm->value("objs"));

               const char * ionFile = pForm->value("ionfile","ion.ion");
               if (!sHiveIon::loadIonFile(user,ionObjs,genericWander, ionFile)) {
                   error("Could not find the objects having given name");
                   outHtml();
                   return 1;
               }
               sDic < sMex::Pos  > bigD;
               idx qrylen;const char * query=pForm->value("query",0,&qrylen);

               if (!qrylen) {
                   error("No query to run");
                   outHtml();
                   return 1;
               }
               genericWander.traverseCompile(query, qrylen, this, true);
               genericWander.maxNumberResults=pForm->ivalue("maxCnt",200);
               genericWander.setSepar(pForm->value("sepField"),pForm->value("sepRecord"));
               genericWander.debug=pForm->ivalue("debug");
               genericWander.retrieveParametricWander(pForm,0);

               genericWander.bigDicCumulator=&bigD;
               genericWander.traverse();
               genericWander.traverseViewBigDic2D();

               idx hdr = pForm->ivalue("hdr",1);
               idx repos = 0;
               if (bigD.dim() && !hdr) {
                   for (idx ip=0; ip < genericWander.traverseBuf.length(); ++ip) {
                       if (genericWander.traverseBuf.ptr(ip)[0]=='\n') {
                           repos=ip;
                           break;
                       }
                   }
               }


               dataForm.printf("%s", (genericWander.traverseBuf.length() == 0) ? "no result": genericWander.traverseBuf.ptr(repos));
               outHtml();
               return 1;
        }
        case eIonNcbiTax: {
            sHiveId screenId(pForm->value("screenId"));
            const char * screenType = pForm->value("screenType", 0);
            const char * screenResult = pForm->value("screenResult", 0);
            bool showPercentage = pForm->ivalue("percentage", 0) == 1 ? true : false;
            idx accession = pForm->ivalue("accession", 0);

            sTxtTbl tbl;

            sStr err, objname;
            bool success = loadScreeningFile(&tbl, screenId, screenType, screenResult, &err, &objname);

            if( !success ) {
                error("%s", err.ptr(0));
                outHtml();
                return 1;
            }

            const char *a = 0;
            iWander.traverseRecordSeparator = (const char *) &a;
            iWander.traverseFieldSeparator = ":";
            sStr ionQuery;
            ionQuery.addString(
                "\
                o=find.taxid_name(taxid=='$taxid', tag='scientific name'); \
                b=find.taxid_parent(taxid==o.taxid); \
                d=count.taxid_parent(parent==b.parent); \
                e=find.taxid_name(taxid==b.parent, tag='scientific name'); \
                f=find.taxid_parent(taxid==e.taxid);\
                print(e.name,e.taxid,f.rank,d.#,'//'); \
                jump!.b(e.taxid,1); ");
            extractIonQueryfromTable(iWander, tbl, ionQuery.ptr(), "NONE");

            sStr col2;
            col2.addString(iWander.traverseBuf, iWander.traverseBuf.length());

            iWander.resetCompileBuf();
            iWander.traverseRecordSeparator = (const char *) &a;
            iWander.traverseFieldSeparator = ",";
            sStr ionQuery2;
            ionQuery2.addString("\
                o=find.taxid_name(taxid=='$taxid', tag='scientific name'); \
                a=find.taxid_parent(taxid==o.taxid); \
                printCSV(o.name,a.rank);");

            extractIonQueryfromTable(iWander, tbl, ionQuery2.ptr(), "no_match,no_rank");
            sStr col1;
            col1.addString(iWander.traverseBuf, iWander.traverseBuf.length());

            sStr preOut, cbuf, dst, tstr;
            sStr tablename;
            real percRatio;
            if( accession == 1 ) {
                if( showPercentage ) {
                    preOut.printf("accession,taxid,matchname,rank,path,matchCnt,matchCnt_pct,min,min_pct,max,max_pct,mean,mean_pct,stddev,intval\n");
                } else {
                    preOut.printf("accession,taxid,matchname,rank,path,matchCnt,matchCnt_pct,min,max,mean,stddev,intval\n");
                }
                tablename.addString("dnaAccessionBasedResult.csv");
            } else {
                if( showPercentage ) {
                    preOut.printf("taxid,matchname,rank,path,matchCnt,matchCnt_pct,min,min_pct,max,max_pct,mean,mean_pct,stddev,intval\n");
                } else {
                    preOut.printf("taxid,matchname,rank,path,matchCnt,matchCnt_pct,min,max,mean,stddev,intval\n");
                }
                tablename.addString("dnaTaxidBasedResult.csv");
            }
            char * co1 = col1.ptr(0);
            char * co2 = col2.ptr(0);
            idx shiftCol = 0;
            for(idx i = 0; i < tbl.rows(); i++) {
                idx icols = 0;
                if( accession == 1 ) {
                    cbuf.cut(0);
                    tbl.printCell(cbuf, i, icols++);
                    if( strncmp(cbuf.ptr(), "-1", 2) == 0 ) {
                        cbuf.printf(0, "\"n/a\"");
                    }
                    preOut.printf("%s,", cbuf.ptr());

                    sVariant mean, perc;
                    tbl.val(mean, i, 7, false);
                    tbl.val(perc, i, 4, false);
                    percRatio = perc.asReal() / mean.asReal();
                    icols++;
                    shiftCol = 2;
                } else {
                    sVariant mean, perc;
                    tbl.val(mean, i, 5, false);
                    tbl.val(perc, i, 2, false);
                    percRatio = perc.asReal() / mean.asReal();
                    shiftCol = 0;
                }
                cbuf.cut(0);
                tbl.printCell(cbuf, i, icols++);
                bool unalignedReads = false;
                if( strncmp(cbuf.ptr(), "-1", 2) == 0 ) {
                    preOut.printf("n/a,unaligned_reads,no_rank,");
                    unalignedReads = true;
                } else {
                    preOut.printf("%s,%s,", cbuf.ptr(), co1);
                }
                if( co2 && (strncmp(co2, "NONE", 4) == 0) ) {
                    preOut.printf("\"/%s:1:no_rank:0/\",", unalignedReads ? "unaligned" : "no_match");
                } else {
                    dst.cut(0);
                    tstr.cut(0);
                    sString::searchandInvertStrings(&tstr, co2, 0, "://" __, "?" __);
                    sString::searchAndReplaceStrings(&dst, tstr.ptr(0), tstr.length(), "/" __, "-" __, 0, false);
                    tstr.cut(0);
                    sString::searchAndReplaceStrings(&tstr, dst.ptr(0), dst.length(), "root" _ "?:" __, "all" _ "/" __, 0, false);
                    *(tstr.ptr(0)) = '/';
                    preOut.printf("\"%s/\",", tstr.ptr(0));
                }

                for(; icols < tbl.cols(); ++icols) {
                    if( icols == (7 + shiftCol) ) {
                        if( pForm->uvalue("downloadCSVFile", 0) ) {
                            preOut.printf("%0.1f+/-%0.2f", tbl.rval(i, icols - 2), tbl.rval(i, icols));
                        } else {
                            preOut.printf("%0.1f\302\261%0.2f", tbl.rval(i, icols - 2), tbl.rval(i, icols));
                        }
                    } else {
                        cbuf.cut(0);
                        if( showPercentage && icols > (2 + shiftCol) && icols <= (5 + shiftCol) ) {
                            sVariant value;
                            tbl.val(value, i, icols, false);
                            cbuf.printf("%s,%0.2f", value.asString(), value.asReal() * percRatio);
                        } else {
                            tbl.printCell(cbuf, i, icols);
                        }
                        preOut.addString(cbuf.ptr(), cbuf.length());
                        preOut.add(",", 1);
                    }
                }
                preOut.cut(preOut.length() - 1);
                preOut.add("\n", 1);
                co1 = sString::next00(co1);
                co2 = sString::next00(co2);
            }
            outBin(preOut.ptr(), preOut.length(), 0, true, "%s-%" DEC "-%s", objname.ptr(0), screenId.objId(), tablename.ptr());
            return 1;
        }
        case eIonTaxInfo: {
            const char * taxidInput = pForm->value("taxid", 0);

            if( strncmp(taxidInput, "NO_INFO", 7) == 0 ) {
                dataForm.printf("id,name,path,value\n");
                dataForm.printf("1,taxid,,0\n");
                dataForm.printf("1,parentid,,0\n");
                dataForm.printf("1,rank,,no rank\n");
                dataForm.printf("1,taxName,0,no name\n");
                dataForm.printf("1,path,,no path\n");
                outHtml();
                return 1;
            }
            sStr path, altnames, info;

            sStr ionQuery1;
            ionQuery1.printf(
                "\
                    o=find.taxid_name(taxid=='%s', tag='scientific name'); \
                    b=find.taxid_parent(taxid==o.taxid); \
                    d=count.taxid_parent(parent==b.parent); \
                    e=find.taxid_name(taxid==b.parent, tag='scientific name'); \
                    print(e.name,e.taxid, d.#,'//'); \
                    jump!.b(e.taxid,1); ",
                taxidInput);
            iWander.traverseFieldSeparator = ":";
            iWander.traverseCompile(ionQuery1);
            iWander.traverse();
            path.addString(iWander.traverseBuf, iWander.traverseBuf.length());
            if( path.length() == 0 ) {
                path.addString("/no path");
            } else {
                sStr pdst;
                sString::searchandInvertStrings(&pdst, path.ptr(0), 0, "://", "/");
                path.cut(1);
                sString::searchAndReplaceStrings(&path, pdst.ptr(0), pdst.length(), "root" _ "/:" __, "all" _ "/" __, 0, false);
                *(path.ptr(0)) = '/';
            }
            iWander.resetCompileBuf();

            sStr ionQuery2;
            ionQuery2.printf("\
                    o=find.taxid_name(taxid=='%s');\
                    printCSV(o.name);", taxidInput);
            const char *a = 0;
            iWander.traverseRecordSeparator = (const char *) &a;

            iWander.traverseCompile(ionQuery2);
            iWander.traverse();
            altnames.addString(iWander.traverseBuf, iWander.traverseBuf.length());
            altnames.add0(2);
            iWander.resetCompileBuf();

            sStr ionQuery3;
            ionQuery3.printf("\
                    o=find.taxid_name(taxid=='%s',tag=='scientific name');\
                    a=find.taxid_parent(taxid==o.taxid);\
                    printCSV(o.taxid,a.parent,a.rank,o.name);", taxidInput);
            iWander.traverseCompile(ionQuery3);
            iWander.traverse();
            sString::searchAndReplaceStrings(&info, iWander.traverseBuf.ptr(0), iWander.traverseBuf.length(), "," __, 0, 0, false);

            idx irow = 1;
            dataForm.printf("id,name,path,value\n");
            dataForm.printf("%" DEC ",taxid,,%s\n", irow, info.ptr(0));
            const char *parentid = sString::next00(info);
            dataForm.printf("%" DEC ",parentid,,%s\n", irow, parentid);
            const char *rank = sString::next00(parentid);
            dataForm.printf("%" DEC ",rank,,%s\n", irow, rank);
            idx iname = 0;
            for(const char *p = altnames; p; p = sString::next00(p)) {
                dataForm.printf("%" DEC ",taxName,%" DEC ",%s\n", irow, iname++, p);
            }
            dataForm.printf("%" DEC ",path,,\"%s/\"\n", irow, path.ptr(0));
            outHtml();
            return 1;
        }
        case eIonTaxDownInfo: {
            const char *taxidInput = pForm->value("taxid", 0);
            idx level = pForm->ivalue("level",1);
            idx cnt = pForm->ivalue("cnt",1000);
            idx start = pForm->ivalue("start",0);
            if( taxidInput && strncmp(taxidInput, "NO_INFO", 7) == 0 ) {
                return 1;
            }

            sStr preOut;
            sStr outInfo;

            preOut.printf("taxid,parent,rank,name,numChild\n");
            sTaxIon taxIon(ionP.ptr());
            preOut.printf("%s\n",taxIon.getTaxIdInfo(taxidInput));
            taxIon.getTaxTreeChildrenInfo(taxidInput, 0, &outInfo);
            idx curlevel = 1;
            idx nextround = outInfo.length();
            idx printCount = 0;
            idx startCount = 0;

            for(char *tc = outInfo; tc && (printCount < cnt); tc = sString::next00(tc)) {
                char *ptr = tc;
                while( *ptr != ',' ) {++ptr;}
                idx ptrlen = ptr - tc;
                ++ptr;
                char *parptr = ptr;
                while( *parptr != ',' ) {++parptr;}
                idx parptrlen = parptr - ptr;

                if ( (ptrlen == parptrlen) && (strncmp(tc, ptr, ptrlen) == 0)){
                    continue;
                }
                idx pos = tc - outInfo.ptr(0);

                if (pos >= nextround){
                    nextround = outInfo.length();
                    if (curlevel >= level){
                        break;
                    }
                    ++curlevel;
                }

                taxIon.getTaxTreeChildrenInfo(tc, ptrlen, &outInfo);

                tc = outInfo.ptr(pos);
                ++startCount;
                if (startCount > start){
                    preOut.printf("%s\n", tc);
                    ++printCount;
                }
            }

            dataForm.addString(preOut.ptr(), preOut.length());
            outHtml();
            return 1;
        }
        case eIonTaxParent: {
            const char * taxidForm = pForm->value("taxid", 0);
            idx parent = pForm->ivalue("parent", 1);
            const char * name = pForm->value("name", 0);
            bool findParent = pForm->boolvalue("commonParent", 0);

            sStr path;
            sTaxIon taxIon(ionP.ptr());
            sStr taxidList;
            taxidList.cut(0);
            if (taxidForm){
                sString::searchAndReplaceSymbols(&taxidList, taxidForm, 0, ",", 0, 0, true, true, true, true);
            }
            if (findParent){
                const char * taxidstring = taxidList.ptr(0);
                idx taxid = 0;
                sscanf(taxidstring, "%" DEC, &taxid);
                if( taxid == 0 ) {
                    break;
                }
                taxIon.filterbyParent(1, taxid, &path, true);
                taxidstring = sString::next00(taxidstring);
                const char *parentPath = path.ptr(0);
                idx taxParent = 1;
                sscanf(parentPath, "%" DEC, &taxParent);

                sStr tmpPath;
                for(; taxidstring && (taxParent != 1); taxidstring = sString::next00(taxidstring)) {
                    tmpPath.cut(0);
                    idx taxId = atoidx((const char *)taxidstring);
                    bool success = taxIon.filterbyParent(taxParent, taxId, &tmpPath, true);
                    tmpPath.add0();
                    if (!success){
                        parentPath = sString::next00(parentPath);
                        for(; parentPath; parentPath = sString::next00(parentPath)) {
                            idx result = sString::compareChoice(parentPath, tmpPath.ptr(0), 0, false, 0, true, 0);
                            if (result != -1){
                                sscanf(parentPath, "%" DEC, &taxParent);
                                break;
                            }
                        }
                    }
                }
                parent = taxParent;
            }
            dataForm.addString("taxid,parent,rank,name,numChild\n");
            if(taxidList.length()){
                bool printFirstRow = true;
                for(const char * taxidstring = taxidList.ptr(0); taxidstring; taxidstring = sString::next00(taxidstring)) {
                    idx taxid = 0;
                    sscanf(taxidstring, "%" DEC, &taxid);
                    if (taxid == 0){
                        continue;
                    }
                    path.cut(0);
                    taxIon.filterbyParent(parent, taxid, &path);
                    path.add0();
                    idx numRows = sString::cnt00(path.ptr());
                    idx offset = printFirstRow ? 0 : 1;
                    for(idx irow = numRows-(1+offset); irow >= 0; --irow) {
                        const char *ptr = sString::next00(path.ptr(0), irow);
                        dataForm.addString(ptr);
                        dataForm.add("\n", 1);
                        printFirstRow = false;
                    }
                }
            }
            else if (name){
                idx limit = pForm->ivalue("limit", 20);
                sStr ionResults00;
                ionResults00.cut(0);
                taxIon.getTaxIdsByName(name, 0, &ionResults00);
                char *r00 = ionResults00.ptr(0);
                sString::searchAndReplaceSymbols(r00, 0, "\n", 0, 0, true, true, true, true);
                idx prevTaxid = 0;
                idx cnt = 0;
                bool printFirstRow = true;
                for(const char *tc = ionResults00.ptr(0); tc && cnt < limit; tc = sString::next00(tc)) {
                    idx localTaxid = atoidx(tc);
                    if (prevTaxid != localTaxid){
                        path.cut(0);
                        bool success = taxIon.filterbyParent(parent, localTaxid, &path);
                        if (success){
                            idx numRows = sString::cnt00(path.ptr());
                            idx offset = printFirstRow ? 0 : 1;
                            for(idx irow = numRows-(1+offset); irow >= 0; --irow) {
                                const char *ptr = sString::next00(path.ptr(0), irow);
                                dataForm.addString(ptr);
                                dataForm.add("\n", 1);
                                printFirstRow = false;
                            }
                            ++cnt;
                        }
                        prevTaxid = localTaxid;
                    }
                }
            }
            dataForm.add0cut();
            outHtml();
            return 1;
        }
        case eIonTaxPathogenInfo: {
            sHiveId screenId(pForm->value("screenId"));
            const char * screenType = pForm->value("screenType", 0);
            const char * screenResult = pForm->value("screenResult", 0);

            sTxtTbl tbl;

            sStr err, objname;
            bool success = loadScreeningFile(&tbl, screenId, screenType, screenResult, &err, &objname);

            if( !success ) {
                error("%s", err.ptr(0));
                outHtml();
                return 1;
            }

            const char *a = 0;
            iWander.traverseRecordSeparator = (const char *) &a;
            iWander.traverseFieldSeparator = ",";
            sStr ionQuery;
            ionQuery.addString(
                "\
            a=find.taxid_name(taxid=='$taxid', tag='scientific name');\
            o=find.taxid_pathogen(taxid==a.taxid); \
            printCSV(o.taxid,a.name,o.pathogen,o.pathogenic,o.severity,o.transmission_food, o.transmission_air, o.transmission_blood, o.human_host); \
            ");

            extractIonQueryfromTable(iWander, tbl, ionQuery.ptr(), "NONE");

            sStr col, buf;
            sStr preOut, tablename;
            col.addString(iWander.traverseBuf, iWander.traverseBuf.length());

            preOut.printf("taxid,name,pathogen,pathogenic,severity,transmission_food,transmission_air,transmission_blood,human_host\n");
            char * co1 = col.ptr(0);
            for(idx i = 0; i < tbl.rows(); i++) {
                buf.cut(0);
                if( co1 && (strncmp(co1, "NONE", 4) == 0) ) {
                    tbl.printCell(buf, i, 0);
                    preOut.printf("%s,,,,,,,,", buf.ptr());
                }
                else {
                    preOut.printf("%s", co1);
                }
                preOut.add("\n", 1);
                co1 = sString::next00(co1);
            }

            tablename.addString("dnaTaxPathogenTable.csv");
            outBin(preOut.ptr(), preOut.length(), 0, true, "%s-%" DEC "-%s", objname.ptr(0), screenId.objId(), tablename.ptr());
            return 1;
        }
        case eIonAnnotInfo: {
            statement.printf("a=foreach(9606,2); b=find.taxid_parent(taxid==a.); print(b.taxid,b.parent,b.rank); ");
        }
            break;
        case eIonAnnotIdMap: {
            statement.printf("a=foreach(9606,2); b=find.taxid_parent(taxid==a.); print(b.taxid,b.parent,b.rank); ");
        }
            break;
        case eIonTaxidCollapse: {
            sStr localBuf;
            localBuf.cut(0);

            sVec <sHiveId> subIDs;
            sHiveId::parseRangeSet(subIDs, pForm->value("objId"));

            if (subIDs.dim() == 0){
                const char * svcType = pForm->value("svcType", "svc-refseq-processor");
                if (!svcType){
                    error("can't find objId's to get the source files in ionTaxidCollapse");
                    outHtml();
                    return 1;
                }
                sUsrObjRes subIDList;
                user->objs2(svcType, subIDList, 0, 0, 0, 0, false, 0, 0);

                subIDs.add(subIDList.dim());
                idx i = 0;
                for(sUsrObjRes::IdIter it = subIDList.first(); subIDList.has(it); subIDList.next(it)) {
                    subIDs[i++] = *subIDList.id(it);
                }

                if (subIDs.dim() == 0){
                    error("can't find objId's based on svctype");
                    outHtml();
                    return 1;
                }

            }
            const char * sourceFile = pForm->value("fileSource", 0);
            const char * outFile = pForm->value("outputFile", 0);
            const char * accessionnumber = pForm->value("accessionnumber", 0);
            const char * taxidInput = pForm->value("taxid", 0);
            bool searchTree = pForm->boolvalue("searchDeep", false);

            sTaxIon taxIon(ionP.ptr());
            if (!taxidInput && accessionnumber){
                taxidInput = taxIon.getTaxIdsByAccession(accessionnumber);
            }

            sStr ionQuery;
            if (searchTree == false){
                ionQuery.printf(0,"\
                    o=find.taxid_name(taxid=='$Taxid', tag='scientific name');\
                    f=print(o.taxid);");
            }
            else {
                ionQuery.printf(0,"\
                    o=find.taxid_name(taxid=='$Taxid', tag='scientific name'); \
                    e=jump.f(o.taxid, '%s'); \
                    b=find.taxid_parent(taxid==o.taxid); \
                    e=find.taxid_name(taxid==b.parent, tag='scientific name'); \
                    jump.f(e.taxid, '%s'); \
                    jump!.b(e.taxid,1); \
                    f=print(e.taxid);", taxidInput, taxidInput);
            }

            sDic<idx> codonDict;
            idx plen = 3;
            idx collapseTaxids = 0;
            bool firstTime = true;
            idx geneCount = 0;
            for (idx iter = 0; iter < subIDs.dim(); ++iter){
                sTxtTbl tbl;
                sStr err, objname;

                bool success = loadScreeningFile(&tbl, subIDs[iter], sourceFile, outFile ? outFile : 0, &err, &objname, true);

                if( !success ) {
                    continue;
                    error("%s", err.ptr(0));
                    outHtml();
                    return 1;
                }
                const char *a = 0;
                iWander.traverseRecordSeparator = (const char *) &a;
                iWander.traverseFieldSeparator = ",";
                iWander.traverseBuf.cut(0);

                extractIonQueryfromTable(iWander, tbl, ionQuery.ptr(), "NONE", firstTime);

                firstTime = false;
                sStr col, buf;
                sStr tablename;
                col.addString(iWander.traverseBuf, iWander.traverseBuf.length());

                if (codonDict.dim() == 0){
                    idx clen = 0;
                    for(idx icol = 0; icol < tbl.cols(); ++icol) {
                        const char * hdr = tbl.cell(-1, icol, &clen);
                        if( plen == clen ) {
                            idx isvalid = true;
                            const char *codon = hdr;
                            unsigned char let;
                            for(idx ilet = 0; ilet < plen; ++codon, ++ilet) {
                                let = sBioseq::mapATGC[(idx) (*codon)];
                                if( let < 0 || let > 3 ) {
                                    isvalid = false;
                                }
                            }
                            if( isvalid ) {
                                idx *cnt = codonDict.set(hdr, plen);
                                *cnt = 0;
                            }
                        }
                    }
                }

                char * co1 = col.ptr(0);
                idx lencolumn = 0;
                idx lentaxid = sLen (taxidInput);

                FilterList00 filterIn(pForm->value("filterIn", 0));
                FilterList00 filterInCol(pForm->value("filterInColName", 0));
                FilterList00 filterOut(pForm->value("filterOut", 0));
                FilterList00 filterOutCol(pForm->value("filterOutColName", 0));
                sStr csv_buf;
                const char *alternative_colnames00[2] = { "Organelle" _ "Organellar" __, "Species" _ "Species_taxid" __};

                sVec<idx> icolInFilters, icolOutFilters;
                for(const char * colname = filterInCol.ptr(); colname && colname[0]; colname = sString::next00(colname)) {
                      filterInCol.toColIDs(icolInFilters, tbl, alternative_colnames00, 2);
                }
                for(const char * colname = filterOutCol.ptr(); colname && colname[0]; colname = sString::next00(colname)) {
                      filterOutCol.toColIDs(icolOutFilters, tbl, alternative_colnames00, 2);
                }


                idx cdsColumn = tbl.colId("# CDS", 5);

                for(idx i = 0; i < tbl.rows(); i++) {
                    buf.cut(0);
                    lencolumn = sLen(co1);
                    if( co1 && (lencolumn == lentaxid) && (strncmp(co1, taxidInput, lencolumn) == 0) ) {
                        bool isValid = true;
                        const char * filt = 0;
                        idx ifilt = 0;
                        for(ifilt = 0, filt = filterIn.ptr(); ifilt < icolInFilters.dim(); ifilt++, filt = filt && filt[0] ? sString::next00(filt) : 0) {
                            csv_buf.cut0cut();
                            tbl.printCell(csv_buf, i, icolInFilters[ifilt]);
                            if( strcmp(filt ? filt : sStr::zero, csv_buf.ptr()) == 0 ) {
                                isValid = true;
                            } else {
                                isValid = false;
                                break;
                            }
                        }
                        if( isValid ) {
                            for(ifilt = 0, filt = filterOut.ptr(); ifilt < icolOutFilters.dim(); ifilt++, filt = filt && filt[0] ? sString::next00(filt) : 0) {
                                csv_buf.cut0cut();
                                tbl.printCell(csv_buf, i, icolOutFilters[ifilt]);
                                if( strcmp(filt ? filt : sStr::zero, csv_buf.ptr()) == 0 ) {
                                    isValid = false;
                                    break;
                                } else {
                                    isValid = true;
                                }
                            }
                        }
                        if (isValid){
                            ++collapseTaxids;
                            if (cdsColumn != -sIdxMax){
                                geneCount += tbl.ival(i, cdsColumn);
                            }
                            for(idx icol = 0; icol < tbl.cols(); ++icol) {
                                csv_buf.cut0cut();
                                tbl.printCell(csv_buf, -1, icol);
                                idx * isCodon = codonDict.get(csv_buf.ptr(), csv_buf.length());
                                if( isCodon ) {
                                    idx resultCount = tbl.ival(i, icol);
                                    (*isCodon) = (*isCodon) + resultCount;
                                }
                            }
                        }
                    }
                    else if( co1 && (strncmp(co1, "NONE", 4) == 0) ) {
                    }
                    co1 = sString::next00(co1);
                }
            }
            idx totCnt = 0;
            idx gcCnt = 0;
            idx gc[3] = {
                0,
                0,
                0 };

            for(idx i = 0; i < codonDict.dim(); ++i) {
                idx codoncnt = *codonDict.ptr(i);
                const char * key = (const char *) (codonDict.id(i));
                totCnt += codoncnt;
                const char *codon = key;
                unsigned char let;
                idx containsGC = 0;
                for(idx ilet = 0; ilet < plen; ++codon, ++ilet) {
                    let = sBioseq::mapATGC[(idx) (*codon)];
                    if( let == 1 || let == 2 ) {
                        gc[ilet] += codoncnt;
                        ++containsGC;
                    }
                }
                gcCnt += (codoncnt * containsGC);
            }
            dataForm.printf("id,value\n");
            dataForm.printf("taxid,%s\n", taxidInput);
            dataForm.printf("collapse,%" DEC "\n", collapseTaxids);
            dataForm.printf("\"#codon\",%" DEC "\n", totCnt);
            dataForm.printf("\"#CDS\",%" DEC "\n", geneCount);
            dataForm.printf("\"GC%%\",%0.2lf\n", totCnt ? (real)(gcCnt*100)/(3*totCnt) : 0);
            dataForm.printf("\"GC1%%\",%0.2lf\n", totCnt ? (real)(gc[0]*100)/(totCnt) : 0);
            dataForm.printf("\"GC2%%\",%0.2lf\n", totCnt ? (real)(gc[1]*100)/(totCnt) : 0);
            dataForm.printf("\"GC3%%\",%0.2lf\n", totCnt ? (real)(gc[2]*100)/(totCnt) : 0);

            idx idlen;
            for(idx i = 0; i < codonDict.dim(); ++i) {
                const char *id = (const char *) (codonDict.id(i, &idlen));
                dataForm.printf("%.*s,%" DEC "\n", (int)idlen, id, *codonDict.ptr(i));
            }

            outHtml();
            return 1;
        }

        case eIonTaxidByName:{

            const char * name = pForm->value("name", 0);
            idx parent = pForm->ivalue("parent", 0);
            idx limit = pForm->ivalue("limit", 20);

            sTaxIon taxIon(ionP.ptr());
            sStr ionResults00;
            ionResults00.cut(0);

            dataForm.printf("id,value\n");
            if (!parent){
                taxIon.getTaxIdsByName(name, limit, &ionResults00);
                for(const char *tc = ionResults00.ptr(0); tc; tc = sString::next00(tc)) {
                    dataForm.printf("%s\n", tc);
                }
            }
            else {
                taxIon.getTaxIdsByName(name, 0, &ionResults00);
                sStr aux1;
                idx cnt = 0;
                char *r00 = ionResults00.ptr(0);
                sString::searchAndReplaceSymbols(r00, 0, "\n", 0, 0, true, true, true, true);
                idx prevTaxid = -1;
                for(const char *tc = ionResults00.ptr(0); tc && cnt < limit; tc = sString::next00(tc)) {
                    aux1.cut(0);
                    sString::copyUntil(&aux1, tc, 0, ",");
                    idx taxId = atoidx((const char *)aux1.ptr());
                    if (prevTaxid == taxId){
                        continue;
                    }
                    if (taxIon.filterbyParent(parent, taxId)){
                        dataForm.printf("%s\n", tc);
                        ++cnt;
                    }
                    prevTaxid = taxId;
                }
            }

            outHtml();
            return 1;

        }
        case eIonCodonDB:{

            sHiveId codID(pForm->value("id"));
            if( !codID && !findDatabaseId(codID, *user, 2) ) {
                error("codon Database is missing");
                outHtml();
                return 1;
            }
            sUsrObj * codonObj = user->objFactory(codID);
            if( !codonObj || !codonObj->Id() ) {
                error("codon database ID is invalid");
                outHtml();
                delete codonObj;
                return 1;
            }
            sStr ioncodonPath;
            codonObj->getFilePathname(ioncodonPath, pForm->value("ionCodon", "ionCodon.ion"));
            if( !ioncodonPath ) {
                error("Incorrect codon ionDB files");
                outHtml();
                delete codonObj;
                return 1;
            }
            if( ioncodonPath.length() > 4 && sIsExactly(ioncodonPath.ptr(ioncodonPath.length() - 4), ".ion") ) {
                ioncodonPath.cut0cut(ioncodonPath.length() - 4);
            }
            sIonWander cWander;

            cWander.addIon(0)->ion->init(ioncodonPath.ptr(), sMex::fReadonly);

            const char *type = pForm->value("type", 0);
            const char *name = pForm->value("name", 0);
            const char *taxid = pForm->value("taxid", 0);
            const char *assembly = pForm->value("assembly", 0);
            bool boolRegex = pForm->boolvalue("regex", 1);
            const char *regex = boolRegex ? "regex:" : "";



            if (!type || !(name || taxid || assembly)){
                error("can't find query to search");
                outHtml();
                delete codonObj;
                return 1;
            }

            const char *column = taxid ? "taxid" : (name ? "name" : "assembly");
            const char *value = taxid ? taxid : name ? name : assembly;

            sStr t1val, tval, cval;
            sString::cleanEnds(&cval, column, 0, "\"" sString_symbolsBlank, true);
            column = cval.ptr(0);
            sString::cleanEnds(&tval, value, 0, "\"" sString_symbolsBlank, true);
            sString::searchAndReplaceStrings(&t1val, tval, tval.length(), "\'" __, "\\'" __, 0, true);
            value = t1val.ptr(0);

            if (boolRegex){
                regex_t preg;
                if( regcomp(&preg, value, REG_EXTENDED) == 0 ) {
                    regfree(&preg);
                } else {
                    error("Invalid regular expression syntax in query\n");
                    outHtml();
                    delete codonObj;
                    return 1;
                }
            }

            cWander.resetCompileBuf();
            sStr ionQuery;
            ionQuery.printf(0,
                "\
                a=search.row(name=%s,value='%s%s'); \
                b=find.row(#R=a.#R,name='%s');\
                c=find.row(#R=b.#R,name='taxid');\
                d=find.row(#R=b.#R,name='assembly');\
                e=find.row(#R=b.#R,name='name');\
                printCSV(c.value,d.value,e.value); ",
                column, regex, value, type);
            cWander.traverseFieldSeparator = ",";
            cWander.traverseCompile(ionQuery);
            cWander.traverse();

            dataForm.printf("taxid,assembly,name\n");
            dataForm.printf("%s", (cWander.traverseBuf.length() == 0) ? "0,0,0": cWander.traverseBuf.ptr());
            outHtml();
            delete codonObj;
            return 1;
        }
        case eExtendCodonTable: {

            sVec <sHiveId> subIDs;
            sHiveId::parseRangeSet(subIDs, pForm->value("objId"));
            const char * srcName = pForm->value("srcName");

            if (subIDs.dim() == 0){
                error("can't find objId");
                outHtml();
                return 1;
            }
            sUsrFile sf(subIDs[0], user);
            sStr outfilePathname;
            if( sf.Id() ) {
                sf.makeFilePathname(outfilePathname, "codonDB.csv");
            }

            sTxtTbl tbl;
            sStr err;
            bool success = loadScreeningFile(&tbl, subIDs[0], srcName, 0, &err, 0, false);
            if( !success ) {
                error("%s", err.ptr(0));
                outHtml();
                return 1;
            }

            idx cntRows = tbl.rows();
            idx cntCols = tbl.cols();
            enum enumColumns
            {
                cTaxid,
                cDivision,
                cType,
                cAssembly,
                cName,
                cLast = cName
            };
            idx plen;
            idx icols[cLast + 1];
            for(idx ipa = 0; ipa < sDim(icols); ipa++) {
                icols[ipa] = -sIdxMax;
            }

            sStr colname;
            for(idx icol = 0; icol < cntCols; ++icol) {
                colname.cut0cut();
                tbl.printCell(colname, -1, icol);
                if( sIsExactly(colname.ptr(), "taxid") ) {
                    icols[cTaxid] = icol;
                } else if( sIsExactly(colname.ptr(), "division") ) {
                    icols[cDivision] = icol;
                } else if( sIsExactly(colname.ptr(), "type") || sIsExactly(colname.ptr(), "organellar") || sIsExactly(colname.ptr(), "organelle") ) {
                    icols[cType] = icol;
                } else if( sIsExactly(colname.ptr(), "assembly") ) {
                    icols[cAssembly] = icol;
                } else if( sIsExactly(colname.ptr(), "name") || sIsExactly(colname.ptr(), "species") ) {
                    icols[cName] = icol;
                }
            }

            enum enumType
            {
                genomic = 0x1,
                mitochondrion = 0x2,
                chloroplast = 0x4,
                plastid = 0x8,
                leucoplast = 0x10,
                chromoplast = 0x20
            };
            typedef struct {
                enumType eType;
                const char *nametype;
                const char letterType;
            } CodonKnownTypes;

            CodonKnownTypes c_types[] = {
                {
                    genomic,
                    "genomic" __,
                    'G'
                },
                {
                    mitochondrion,
                    "mitochondrion" __,
                    'M'
                },
                {
                    chloroplast,
                    "chloroplast" __,
                    'C'
                },
                {
                    plastid,
                    "plastid",
                    'P'
                },
                {
                    leucoplast,
                    "leucoplast" __,
                    'L'
                },
                {
                    chromoplast,
                    "chromoplast" __,
                    'O'
                }
            };

            idx numtypes = 0;
            while(c_types[++numtypes].nametype);


            enum enumDivision
            {
                genbank = 0x1,
                refseq = 0x2
            };
            idx divisions[2] = {genbank, refseq};
            const char *listDivision = "genbank" _ "refseq" __;

            sDic <idx> parents;

            sStr cbuf, tbuf, dbuf;
            sStr taxCurrent, taxParent;
            sStr auxout;
            auxout.cut(0);
            sTaxIon taxIon(ionP.ptr());

            idx taxId;
            sStr buf1;
            sStr buf2;
            for (idx irow = 0; irow < cntRows; ++irow){
                taxCurrent.cut(0);
                tbl.printCell(taxCurrent, irow, icols[cTaxid]);
                const char * stringtype = tbl.printCell(tbuf, irow, icols[cType]);
                idx type1 = 0;
                idx typeID;
                for(typeID = 0; typeID < numtypes; ++typeID) {
                    if (strcmp(c_types[typeID].nametype, stringtype) == 0){
                        type1 = c_types[typeID].eType;
                        break;
                    }
                }
                const char * stringdivision = tbl.printCell(dbuf, irow, icols[cDivision]);
                idx aux2;
                sString::compareChoice(stringdivision, listDivision, &aux2, false, 0, true, 0);
                udx typedivision = (divisions[aux2] == genbank) ?(udx)type1 << 32 : (udx) type1 && 0xFFFFFFFF;


                if ((divisions[aux2] == refseq)){
                    buf1.cut(0);
                    buf2.cut(0);
                    tbl.printCell(buf1, irow, icols[cAssembly]);
                    tbl.printCell(buf2, irow, icols[cName]);
                    for(idx typeID = 0; typeID < numtypes; ++typeID) {
                        auxout.add(",", 1);
                    }
                    for(idx typeID = 0; typeID < numtypes; ++typeID) {
                        if ((c_types[typeID].eType & type1)){
                            auxout.add("1", 1);
                        }
                        auxout.add(",", 1);
                    }
                    auxout.add0cut();
                    auxout.printf("%.*s,%s,%s\n", (int)taxCurrent.length(), taxCurrent.ptr(), buf2.ptr(), buf1.ptr());
                }
                do {

                    taxId = atoidx((const char *)taxCurrent.ptr());
                    idx *value = parents.get(taxCurrent, taxCurrent.length());
                    if (!value){
                        value = parents.set(taxCurrent, taxCurrent.length());
                        *value = typedivision;
                    }
                    else {
                        udx locValue = (divisions[aux2] == genbank) ? (udx)*value&0xFFFFFFFF00000000 : (udx)*value&0xFFFFFFFF;
                        udx result = locValue | typedivision ;
                        if (result == locValue){
                            taxId = 1;
                        }
                        else {
                            *value = (divisions[aux2] == genbank) ? (result)|(*value&0xFFFFFFFF) : (*value&0xFFFFFFFF00000000)|(result&0xFFFFFFFF);
                        }
                    }
                    taxParent.cut(0);
                    taxIon.getParentTaxIds(taxCurrent.ptr(0), &taxParent);
                    taxCurrent.cut(0);
                    taxCurrent.addString(taxParent.ptr(0), taxParent.length());
                }
                while (taxId > 1);
            }

            sStr ionQuery;
            ionQuery.printf(0, "\
                    o=find.taxid_name(taxid=='$Taxid', tag='scientific name');\
                    f=printCSV(o.name);");
            const char *a = 0;
            iWander.traverseRecordSeparator = (const char *) &a;
            iWander.traverseFieldSeparator = 0;
            iWander.traverseCompile(ionQuery);

            const char * pid = (const char *) iWander.parametricArguments.id(0, &plen);
            idx *toModify = (idx*) iWander.getSearchDictionaryPointer(pid, plen);

            idx keylen;
            sFil outTable(outfilePathname);
            outTable.empty();
            if (!outTable.ok()){
                error("can't write in the output filename");
                outHtml();
                return 1;
            }
            for(idx idiv = 0; idiv < 2; ++idiv) {
                char letDivision = idiv == 0 ? 'G' : 'R';
                for(idx typeID = 0; typeID < numtypes; ++typeID) {
                    outTable.add("t",1);
                    outTable.add(&letDivision,1);
                    outTable.add(&c_types[typeID].letterType, 1);
                    outTable.add(",",1);
                }
            }
            outTable.addString("taxid,name,assembly\n");
            for(idx i = 0; i < parents.dim(); ++i) {
                udx val = *parents.ptr(i);
                const char *key = (const char *) (parents.id(i, &keylen));

                idx typeGenbank = val >> 32;

                idx typeRefseq = val & 0xFFFFFFFF;


                toModify[0] = sConvPtr2Int(key);
                toModify[1] = keylen;

                iWander.traverseBuf.cut(0);
                iWander.traverse();
                iWander.traverseBuf.shrink00();

                if (iWander.traverseBuf.length()){
                    for(idx typeID = 0; typeID < numtypes; ++typeID) {
                        if ((c_types[typeID].eType & typeGenbank)){
                            outTable.add("1", 1);
                        }
                        outTable.add(",", 1);
                    }
                    for(idx typeID = 0; typeID < numtypes; ++typeID) {
                        if ((c_types[typeID].eType & typeRefseq)){
                            outTable.add("1", 1);
                        }
                        outTable.add(",", 1);
                    }
                    outTable.add(key, keylen);
                    outTable.add(",",1);
                    outTable.add(iWander.traverseBuf.ptr(0), iWander.traverseBuf.length());
                    outTable.add(",0\n",3);
                }
            }
            outTable.add(auxout.ptr(),auxout.length());
            outTable.add0cut();
            return 1;
        }
        case eIonTaxidCollapseExt: {
            sStr localBuf;
            localBuf.cut(0);

            sVec <sHiveId> subIDs;
            sHiveId::parseRangeSet(subIDs, pForm->value("objId"));

            if (subIDs.dim() == 0){
                const char * svcType = pForm->value("svcType", "svc-refseq-processor");
                if (!svcType){
                    error("can't find objId's to get the source files in ionTaxidCollapse");
                    outHtml();
                    return 1;
                }
                sUsrObjRes subIDList;
                user->objs2(svcType, subIDList, 0, 0, 0, 0, false, 0, 0);

                subIDs.add(subIDList.dim());
                idx i = 0;
                for(sUsrObjRes::IdIter it = subIDList.first(); subIDList.has(it); subIDList.next(it)) {
                    subIDs[i++] = *subIDList.id(it);
                }

                if (subIDs.dim() == 0){
                    error("can't find objId's based on svctype");
                    outHtml();
                    return 1;
                }

            }
            const char * sourceFile = pForm->value("fileSource", 0);
            const char * outFile = pForm->value("outputFile", 0);
            const char * accessionnumber = pForm->value("accessionnumber", 0);
            const char * taxidInput = pForm->value("taxid", 0);
            bool searchTree = pForm->boolvalue("searchDeep", false);

            const char * sourceIndex = strstr(sourceFile, "genbank") ? "genbank_index.tsv" : "Refseq_index.tsv";

            sTaxIon taxIon(ionP.ptr());
            if (!taxidInput && accessionnumber){
                taxidInput = taxIon.getTaxIdsByAccession(accessionnumber);
            }

            sStr ionQuery;
            if (searchTree == false){
                ionQuery.printf(0,"\
                    o=find.taxid_name(taxid=='$Taxid', tag='scientific name');\
                    f=print(o.taxid);");
            }
            else {
                ionQuery.printf(0,"\
                    o=find.taxid_name(taxid=='$Taxid', tag='scientific name'); \
                    e=jump.f(o.taxid, '%s'); \
                    b=find.taxid_parent(taxid==o.taxid); \
                    e=find.taxid_name(taxid==b.parent, tag='scientific name'); \
                    jump.f(e.taxid, '%s'); \
                    jump!.b(e.taxid,1); \
                    f=print(e.taxid);", taxidInput, taxidInput);
            }

            sDic<idx> codonDict;

            idx plen = 0;
            if (strstr(sourceFile, "Dinuc")){
                plen = 2;
            }
            else if (strstr(sourceFile, "species")){
                plen = 3;
            }
            else if (strstr(sourceFile, "Bicod")){
                plen = 6;
            }

            idx collapseTaxids = 0;
            bool firstTime = true;
            idx geneCount = 0;
            for (idx iter = 0; iter < subIDs.dim(); ++iter){
                sTxtTbl tbl, tblindex;
                sStr err, objname;
                tbl.parseOptions().absrowCnt = 2;
                bool success = loadScreeningFile(&tbl, subIDs[iter], sourceFile, outFile ? outFile : 0, &err, &objname, true);
                success &= loadScreeningFile(&tblindex, subIDs[iter], sourceIndex, outFile ? outFile : 0, &err, &objname, true);

                if( !success ) {
                    continue;
                    error("%s", err.ptr(0));
                    outHtml();
                    return 1;
                }
                const char *a = 0;
                iWander.traverseRecordSeparator = (const char *) &a;
                iWander.traverseFieldSeparator = ",";
                iWander.traverseBuf.cut(0);

                extractIonQueryfromTable(iWander, tblindex, ionQuery.ptr(), "NONE", firstTime);

                firstTime = false;
                sStr col, buf;
                sStr tablename;
                col.addString(iWander.traverseBuf, iWander.traverseBuf.length());

                if (codonDict.dim() == 0){
                    idx clen = plen;
                    for(idx icol = 0; icol < tbl.cols(); ++icol) {
                        const char * hdr = tbl.cell(-1, icol, &clen);
                        if( plen == clen ) {
                            idx isvalid = true;
                            const char *codon = hdr;
                            unsigned char let;
                            for(idx ilet = 0; ilet < plen; ++codon, ++ilet) {
                                let = sBioseq::mapATGC[(idx) (*codon)];
                                if( let < 0 || let > 3 ) {
                                    isvalid = false;
                                }
                            }
                            if( isvalid ) {
                                idx *cnt = codonDict.set(hdr, plen);
                                *cnt = 0;
                            }
                        }
                    }
                }

                char * co1 = col.ptr(0);
                idx lencolumn = 0;
                idx lentaxid = sLen (taxidInput);

                FilterList00 filterIn(pForm->value("filterIn", 0));
                FilterList00 filterInCol(pForm->value("filterInColName", 0));
                FilterList00 filterOut(pForm->value("filterOut", 0));
                FilterList00 filterOutCol(pForm->value("filterOutColName", 0));
                sStr csv_buf;
                const char *alternative_colnames00[2] = { "Organelle" _ "Organellar" __, "Species" _ "Species_taxid" __};

                sVec<idx> icolInFilters, icolOutFilters;
                for(const char * colname = filterInCol.ptr(); colname && colname[0]; colname = sString::next00(colname)) {
                      filterInCol.toColIDs(icolInFilters, tbl, alternative_colnames00, 2);
                }
                for(const char * colname = filterOutCol.ptr(); colname && colname[0]; colname = sString::next00(colname)) {
                      filterOutCol.toColIDs(icolOutFilters, tbl, alternative_colnames00, 2);
                }


                idx cdsColumn = tbl.colId("# CDS", 5);

                sTxtTbl temptbl;
                temptbl.setBuf(tbl.getFile(0));
                temptbl.parseOptions() = tbl.parseOptions();
                temptbl.parseOptions().absrowCnt = 1;

                for(idx i = 0; i < tblindex.rows(); i++) {
                    buf.cut(0);
                    lencolumn = sLen(co1);
                    if( co1 && (lencolumn == lentaxid) && (strncmp(co1, taxidInput, lencolumn) == 0) ) {
                        bool isValid = true;
                        const char * filt = 0;
                        idx ifilt = 0;
                        for(ifilt = 0, filt = filterIn.ptr(); ifilt < icolInFilters.dim(); ifilt++, filt = filt && filt[0] ? sString::next00(filt) : 0) {
                            csv_buf.cut0cut();
                            tblindex.printCell(csv_buf, i, icolInFilters[ifilt]);
                            if( strcmp(filt ? filt : sStr::zero, csv_buf.ptr()) == 0 ) {
                                isValid = true;
                            } else {
                                isValid = false;
                                break;
                            }
                        }
                        if( isValid ) {
                            for(ifilt = 0, filt = filterOut.ptr(); ifilt < icolOutFilters.dim(); ifilt++, filt = filt && filt[0] ? sString::next00(filt) : 0) {
                                csv_buf.cut0cut();
                                tblindex.printCell(csv_buf, i, icolOutFilters[ifilt]);
                                if( strcmp(filt ? filt : sStr::zero, csv_buf.ptr()) == 0 ) {
                                    isValid = false;
                                    break;
                                } else {
                                    isValid = true;
                                }
                            }
                        }
                        if (isValid){
                            ++collapseTaxids;

                            idx locColumn = 0;
                            if (strstr(sourceFile, "species")){
                                locColumn = tblindex.colId("codon",5);
                            }
                            else if (strstr(sourceFile, "Junc")){
                                locColumn = tblindex.colId("juncdinuc",9);
                            }
                            else if (strstr(sourceFile, "Dinuc")){
                                locColumn = tblindex.colId("dinuc",5);
                            }
                            else if (strstr(sourceFile, "Bicod")){
                                locColumn = tblindex.colId("bicod",5);
                            }

                            idx offset = tblindex.ival(i, locColumn);
                            temptbl.parseOptions().initialOffset = offset;
                            temptbl.parse();

                            if (cdsColumn != -sIdxMax){
                                geneCount += temptbl.ival(-1, cdsColumn);
                            }
                            for(idx icol = 0; icol < tbl.cols(); ++icol) {
                                csv_buf.cut0cut();
                                tbl.printCell(csv_buf, -1, icol);
                                idx * isCodon = codonDict.get(csv_buf.ptr(), csv_buf.length());
                                if( isCodon ) {
                                    idx resultCount = temptbl.ival(-1, icol);
                                    (*isCodon) = (*isCodon) + resultCount;
                                }
                            }
                        }
                    }
                    else if( co1 && (strncmp(co1, "NONE", 4) == 0) ) {
                    }
                    co1 = sString::next00(co1);
                }
            }
            idx totCnt = 0;
            idx gcCnt = 0;
            idx gc[plen];
            for (idx i = 0; i < plen; i++){
                gc[i] = 0;
            }

            for(idx i = 0; i < codonDict.dim(); ++i) {
                idx codoncnt = *codonDict.ptr(i);
                const char * key = (const char *) (codonDict.id(i));
                totCnt += codoncnt;
                const char *codon = key;
                unsigned char let;
                idx containsGC = 0;
                for(idx ilet = 0; ilet < plen; ++codon, ++ilet) {
                    let = sBioseq::mapATGC[(idx) (*codon)];
                    if( let == 1 || let == 2 ) {
                        gc[ilet] += codoncnt;
                        ++containsGC;
                    }
                }
                gcCnt += (codoncnt * containsGC);
            }
            dataForm.printf("id,value\n");
            dataForm.printf("taxid,%s\n", taxidInput);
            dataForm.printf("collapse,%" DEC "\n", collapseTaxids);
            dataForm.printf("\"#codon\",%" DEC "\n", totCnt);
            dataForm.printf("\"#CDS\",%" DEC "\n", geneCount);
            dataForm.printf("\"GC%%\",%0.2lf\n", totCnt ? (real)(gcCnt*100)/(plen*totCnt) : 0);
            for (idx i = 0; i < plen; i++){
                dataForm.printf("\"GC%d%%\",%0.2lf\n", (int)i+1, totCnt ? (real)(gc[i]*100)/(totCnt) : 0);
            }

            idx idlen;
            for(idx i = 0; i < codonDict.dim(); ++i) {
                const char *id = (const char *) (codonDict.id(i, &idlen));
                dataForm.printf("%.*s,%" DEC "\n", (int)idlen, id, *codonDict.ptr(i));
            }

            outHtml();
            return 1;
        }

            break;
        default: {
        }
            break;
    }

    iWander.traverseCompile(statement.ptr());
    iWander.traverse();

    return 1;
}

enum enumIonBioCommands
{
    eIonGenBankAnnotPosMap,
    eIonAnnotTypes,
    eIonAnnotInfoAll
};

idx lookForAlternativeSeqID(const char * seqID, sHiveIonSeq & hiWander, const char * wandername, idx cnt, idx cntStart, sStr & output, idx wanderIndex=-1) {

    sIonWander * iWander = &hiWander.wanderList[wandername];
    const char * nxt;
    for( const char * p=seqID; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
        nxt=strpbrk(p,"| ");
        if(!nxt || *nxt==' ')
            break;

        const char * curId=nxt+1;
        nxt=strpbrk(nxt+1," |");
        if(!nxt)
            nxt=seqID+sLen(seqID);
        if(*nxt==' ')
            break;
        iWander->setSearchTemplateVariable("$seqid",6, curId, nxt-curId);
        hiWander.standardTraverse(output, 0, cnt, cntStart,false,wanderIndex);
        if (output.length()) {
            return 0;
        }
        else {
            const char * dot = strpbrk(curId,".");
           if (dot){
               iWander->setSearchTemplateVariable("$seqid",6, curId, dot-curId);
               hiWander.standardTraverse(output, 0, cnt, cntStart,false,wanderIndex);
               if (output.length()) {
                   return 0;
               }
               if (iWander->cntResults) {
                   return 0;
               }
           }
        }
    }

    return 0;
}

idx DnaCGI::CmdIonBio(idx cmd)
{

    sStr blobName("result.csv");
    sStr outPutContent;

    sStr seqID;
    formValue("seqID", &seqID, 0);
    idx iSub = formIValue("mySubID", -1) -1;
    idx fromComputation = formIValue("fromComputation", 1);
    idx sequenceLength = 0;
    std::unique_ptr < sUsrObj > al;
    if( !seqID && fromComputation ) {
        if( !objs.dim() ) {
            return 1;
        }
        if (iSub==-2) {
            outPutContent.printf("no sequence identifier's specified");
            outBin(outPutContent.ptr(), outPutContent.length(), 0, true, blobName.ptr(0));
            return 1;
        }
        sUsrObj& obj = objs[0];
        sVec < sHiveId > parent_proc_ids;
        obj.propGetHiveIds("parent_proc_ids", parent_proc_ids);
        al.reset(user->objFactory(parent_proc_ids.dim() ? parent_proc_ids[0] : sHiveId::zero));
        if( !al.get() || !al->Id() ) {
            return 1;
        }
        sStr parentAlignmentPath;
        al->getFilePathname00(parentAlignmentPath, "alignment.hiveal" _ "alignment.vioal" __);
        if( !parentAlignmentPath ) {
            return 1;
        }
        sHiveal hiveal(user, parentAlignmentPath);

        sHiveseq Sub(user, al->propGet00("subject", 0, ";"), hiveal.getSubMode());
        seqID.printf(0, "%s", Sub.id(iSub));
        sequenceLength = Sub.len(iSub);
    }
    if( !seqID.length() && fromComputation ) {
        outPutContent.printf("no sequence identifier's specified");
        outBin(outPutContent.ptr(), outPutContent.length(), 0, true, blobName.ptr(0));
        return 1;
    }
    idx cntStart = pForm->ivalue("start", 0);
    idx cnt = pForm->ivalue("cnt", 20);
    if( cnt == -1 )
        cnt = sIdxMax;

    idx pos_start = pForm->ivalue("pos_start", 1);
    idx pos_end = pForm->ivalue("pos_end", 0);
    if (pos_start==-1){
        pos_end=sIdxMax;
    }
    if (!pos_end && sequenceLength) pos_end = sequenceLength;
    if( !pos_end )
        pos_end = pos_start + 10000;

    const char * ionObjIDs = pForm->value("ionObjs", 0);
    if (!ionObjIDs) {
        ionObjIDs = pForm->value("ids",0);
    }
    const char * ionType = pForm->value("ionType", "u-ionAnnot");
    const char * fileNameTemplate = pForm->value("file", "ion");

    const char * feature = pForm->value("features", "");
    sStr featureSplitByZero;
    sStr featureStatement("foreach");
    if (sLen(feature) > 0){
        sString::searchAndReplaceSymbols(&featureSplitByZero, feature, 0, ",", 0, 0, true, true, true, true);

        featureStatement.addString("(", 1);
        idx icomma = 0;
        for(const char *p = featureSplitByZero.ptr(0); p; p = sString::next00(p)) {
            if( icomma )
                featureStatement.addString(",", 1);
            featureStatement.addString("\"",1);
            featureStatement.addString(p, sLen(p));
            featureStatement.addString("\"",1);
            ++icomma;
        }

        featureStatement.addString(")", 1);
    }
    else  featureStatement.addString(".type(\"\")");

    switch(cmd) {
        case eIonAnnotInfoAll: {
            outPutContent.printf("running ion annot info all");
        }
            break;
        case eIonGenBankAnnotPosMap: {
            if (!ionObjIDs || !sLen(ionObjIDs)){
                outPutContent.printf("no object ids specified !!!\n");
                outBin(outPutContent.ptr(), outPutContent.length(), 0, true, blobName.ptr(0));
                return 1;
            }

            sHiveId mutualAlObjID(formValue("multipleAlObjID"));
            sUsrFile mutualAlignmentObj(mutualAlObjID, sQPride::user);
            sBioal * multipleAl=0;

            sStr mutualAlignmentPath;
            if( mutualAlignmentObj.Id() ) {
                mutualAlignmentObj.getFilePathname00(mutualAlignmentPath,"alignment.vioalt" _ "alignment.vioal" _ "alignment.hiveal" __);
            }
            bool isMutAl = mutualAlignmentPath.ok();
            sHiveal mutAl(user, mutualAlignmentPath);

            const sBioseq::EBioMode mutualAlMode = sBioseq::eBioModeLong;
            sHiveseq Sub_mut(sQPride::user, isMutAl ? al->propGet00("subject", 0, ";") : 0 ,mutualAlMode);

            if( isMutAl ) {
                multipleAl=&mutAl;

                multipleAl->Qry = &Sub_mut;
                sBioseqAlignment::Al * hdr;
                sVec<idx> uncompM(sMex::fSetZero);
                idx * matchTrain = 0, iMutS = 0;
                for( iMutS = 0 ; iMutS < mutAl.dimAl(); ++iMutS ) {
                    hdr = mutAl.getAl(iMutS);
                    if( hdr->idQry() == iSub ) {
                        matchTrain = mutAl.getMatch(iMutS);
                        uncompM.resize(hdr->lenAlign()*2);
                        sBioseqAlignment::uncompressAlignment(hdr,matchTrain,uncompM);
                        break;
                    }
                }
                if( iMutS < mutAl.dimAl() ) {
                    pos_start = sBioseqAlignment::remapSubjectPosition( hdr,uncompM.ptr(),pos_start ) ;
                    if (pos_end < sequenceLength){
                        pos_end = sBioseqAlignment::remapSubjectPosition(hdr,uncompM.ptr(),pos_end ) ;
                    }
                }
            }



            sHiveIonSeq hi(user, ionObjIDs, ionType, fileNameTemplate);

            sStr ionQry1;
            ionQry1.printf(0, "seq=foreach($id);a=find.annot(#range=possort-max,seq.1,%" DEC ":%" DEC ",seq.1,%" DEC ":%" DEC ");", pos_start, pos_start, pos_end, pos_end);
            ionQry1.printf("unique.1(a.record);f=%s;c=find.annot(seqID=a.seqID,record=a.record,id=f.1);", featureStatement.ptr(0));
            ionQry1.printf("printCSV(c.seqID,c.pos,c.type,c.id);");

            if (pos_start==-1) {
                ionQry1.printf(0,"seq=foreach($id);a=find.annot(seqID=seq.1);printCSV(a.seqID,a.pos,a.type,a.id)");
            }
            hi.addIonWander("seqPositionLookUp", ionQry1.ptr(0));

            sStr ionQry2;
            ionQry2.printf(0, "seq=foreach($id);a=find.annot(#range=possort-max,seq.1,%" DEC ":%" DEC ",seq.1,%" DEC ":%" DEC ");", pos_start, pos_start, pos_end, pos_end);
            ionQry2.printf("unique.1(a.record);f=%s;c=find.annot(seqID=a.seqID,record=a.record,type=f.1);", featureStatement.ptr(0));
            ionQry2.printf("printCSV(c.seqID,c.pos,c.type,c.id);");
            hi.addIonWander("seqPositionLookUp_1", ionQry2.ptr(0));

            sStr ionQry3;
            ionQry3.printf(0,"a=find.annot(id='$id', type='$type'); unique.1(a.seqID);dict(a.seqID,1);");

            hi.addIonWander("idTypePositionLookUp", ionQry3.ptr(0));

            sDic < sStr > dic;
            idx tmpLen=0;
            idx seqIdStartPos=sHiveIonSeq::locateSeqId(seqID.ptr(),&tmpLen);
            seqID.cut0cut(seqIdStartPos+tmpLen,2);
            hi.annotMapPosition(&outPutContent, &dic, seqID.ptr(seqIdStartPos), pos_start, pos_end, cnt, cntStart,0,multipleAl);

        }
            break;
        case eIonAnnotTypes: {
            sHiveIonSeq hi(user, ionObjIDs, ionType, fileNameTemplate);

            sStr ionQry1;

            const char * refGenomeList = pForm->value("refGenomeList", 0);
            sHiveseq ref(user, refGenomeList);

            const char * recordTypes = pForm->value("recordTypes", 0);

            const char * searchId = pForm->value("search",0);
            const char * searchType = pForm->value("searchType",0);

            outPutContent.cut(0);
            if (ref.dim())
            {

                sStr wandername, tmpOut;
                idx curIndex = 0, tmpStart =0;
                sStr mySeqID, accNum;

                ionQry1.printf(0, "relation=find.annot(seqID=$seqid);p=print(relation.seqID,relation.pos,relation.type,relation.id);");
                if (searchId && searchType)
                {
                    ionQry1.printf(0,"a=search.annot(id='regex:%s',type=%s);relation=find.annot(seqID=$seqid,type=a.type,id=a.id);p=print(relation.seqID,relation.pos,relation.type,relation.id);",searchId,searchType);
                }
                else if (searchId)
                {
                    ionQry1.printf(0,"a=search.annot(id='regex:%s');relation=find.annot(seqID=$seqid,record=a.record,id=a.id);p=print(relation.seqID,relation.pos,relation.type,relation.id);",searchId);
                }
                else if (searchType)
                {
                    ionQry1.printf(0,"a=search.annot(type=%s);relation=find.annot(seqID=$seqid,record=a.record,type=a.type);p=print(relation.seqID,relation.pos,relation.type,relation.id);",searchType);
                }
                wandername.printf(0,"lookUp");
                sIonWander * hiWander = hi.addIonWander(wandername.ptr(), ionQry1.ptr(0));

                for ( idx iSub=0 ; iSub<ref.dim() ; ++iSub ) {
                    tmpStart = cntStart - curIndex;
                    const char * mySeqID=ref.id(iSub);
                    tmpOut.cut(0);

                    sString::extractNCBIInfoSeqID(0, &accNum,mySeqID, sLen(mySeqID));
                    accNum.shrink00();

                    hiWander->resetResultBuf();
                    hiWander->setSearchTemplateVariable("$seqid",6, accNum.ptr(0), accNum.length());

                    if (tmpStart<=0) tmpStart=0;

                    hi.standardTraverse(tmpOut, 0, cnt, tmpStart,false);

                    if (!tmpOut.length()){
                        lookForAlternativeSeqID(mySeqID,hi, wandername.ptr(), cnt, tmpStart, tmpOut);
                    }
                    curIndex += hiWander->cntResults;
                    if (tmpOut.length()) {
                        outPutContent.addString(tmpOut.ptr(), tmpOut.length());
                        cnt -=  (hiWander->cntResults  - tmpStart);
                    }

                }
            }
            else
            {
                if (recordTypes && strcmp(recordTypes,"seqID")==0){
                    ionQry1.printf(0, "seq=foreach.seqID('');printLimit=print(seq.1);");
                } else if (recordTypes &&  strcmp(recordTypes,"relation")==0){
                    ionQry1.printf(0, "seq=foreach.seqID('');relation=find.annot(seqID=seq.1);p=print(relation.seqID,relation.pos,relation.type,relation.id);");
                    if (searchId && searchType)
                    {
                        ionQry1.printf(0,"seq=foreach.seqID('');a=search.annot(id=%s,type=%s);relation=find.annot(seqID=seq.1,type=a.type,id=a.id);p=print(relation.seqID,relation.pos,relation.type,relation.id);",searchId,searchType);
                    }
                    else if (searchId)
                    {
                        ionQry1.printf(0,"seq=foreach.seqID('');a=search.annot(id=%s);relation=find.annot(seqID=seq.1,record=a.record,id=a.id);p=print(relation.seqID,relation.pos,relation.type,relation.id);",searchId);
                    }
                    else if (searchType)
                    {
                        ionQry1.printf(0,"seq=foreach.seqID('');a=search.annot(type=%s);relation=find.annot(seqID=seq.1,record=a.record,type=a.type);p=print(relation.seqID,relation.pos,relation.type,relation.id);",searchType);
                    }
                } else {
                    ionQry1.printf(0, "type=foreach.type('');printLimit=print(type.1);");
                }
                hi.addIonWander("lookUp", ionQry1.ptr(0));

                hi.standardTraverse(outPutContent, 0, cnt, cntStart);
            }
        }
            break;
        default:
            break;
    }

    outBin(outPutContent.ptr(), outPutContent.length(), 0, true, blobName.ptr(0));
    return 1;
}
