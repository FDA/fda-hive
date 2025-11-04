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

#include <ssci/bio/bioAnnot.hpp>


const char * sIonAnnotBase::initPrecompile(void)
{
    sStr trbuf;
    trbuf.printf(0, "a=foreach.type(\"\");b=unique.1(a.1);print(b.1);");

    const char * err = ts1.traverseCompile(trbuf.ptr(), trbuf.length());
    if( err ) {
        return err;
    }
    trbuf.printf(0, "\
                 a=find.annot(#range=possort-max,seqID=$seqID1,$start,seqID=$seqID2,$end); \
                 unique.1(a.record);print(a.record); \
                 print(a.1); ");

    const char *a = 0;
    ts2.traverseRecordSeparator = (const char *) &a;
    err = ts2.traverseCompile(trbuf.ptr(), trbuf.length());
    if( err ) {
        return err;
    }

    trbuf.printf(0, "\
        a=find.annot(type=gene); \
        unique.1(a.id); \
        print(a.id); ");
    err = ts4.traverseCompile(trbuf.ptr(), trbuf.length());
    if( err ) {
        return err;
    }

    trbuf.printf(0, "\
        a=find.annot(#range=possort-max,seqID=$seqID1,$start,seqID=$seqID2,$end); \
        unique.1(a.record); \
        b = find.annot(seqID=a.seqID,record=a.record,type=\"gene\"); \
        unique.1(b.id); \
        print (b.pos,b.id); ");
    err = ts5.traverseCompile(trbuf.ptr(), trbuf.length());
    if( err ) {
        return err;
    }
    return 0;

}

idx sIonAnnotBase::getNumType(sStr *lbuf, const char * separator)
{
    const char *a = 0;
    ts1.traverseRecordSeparator = separator ? separator : (const char *) &a;
    ts1.resetResultBuf();
    ts1.traverseBuf.cut(0);
    ts1.traverse();
    if( ts1.traverseBuf.length() ) {
        if( lbuf ) {
            lbuf->addString(ts1.traverseBuf.ptr(), ts1.traverseBuf.length());
        }
        idx num = ts1.cntResults;
        return num;
    }
    return 0;
}

bool sIonAnnotBase::setGeneType (const char * geneType, idx geneid_len)
{
    ts4.resetCompileBuf();
    rBuf.printf(0, "\
        a=find.annot(type=%s); \
        unique.1(a.id); \
        print(a.id); ", geneType);
    const char *err = ts4.traverseCompile(rBuf.ptr(), rBuf.length());
    if( err ) {
        errIO.printf("%s",err);
        return true;
    }

    ts5.resetCompileBuf();
    rBuf.printf(0, "\
        a=find.annot(#range=possort-max,seqID=$seqID1,$start,seqID=$seqID2,$end); \
        unique.1(a.record); \
        b = find.annot(seqID=a.seqID,record=a.record,type=\"%s\"); \
        unique.1(b.id); \
        print (b.pos,b.id); ", geneType);
    err = ts5.traverseCompile(rBuf.ptr(), rBuf.length());
    if( err ) {
        errIO.printf("%s",err);
        return true;
    }
    return false;

}

idx sIonAnnotBase::getNumRecords(const char * seqid, idx seqlen, idx startpos, idx endpos)
{
    if( !seqlen ) {
        seqlen = sLen(seqid);
    }
    ts2.setSearchTemplateVariable("$seqID1", 7, seqid, seqlen);
    ts2.setSearchTemplateVariable("$seqID2", 7, seqid, seqlen);
    rBuf.cut(0);
    rBuf.addNum(startpos);
    rBuf.addString(":0");
    ts2.setSearchTemplateVariable("$start", 6, rBuf.ptr(0), rBuf.length());
    rBuf.add0(1);
    idx initpos = rBuf.length();
    const char *end = rBuf.addNum(startpos);
    rBuf.addString(":0");
    ts2.setSearchTemplateVariable("$end", 4, end, rBuf.length() - initpos);

    ts2.traverseBuf.cut(0);
    ts2.resetResultBuf();
    ts2.traverse();
    if( ts2.traverseBuf.length() ) {
        idx num = sString::cnt00(ts2.traverseBuf.ptr());
        return num;
    }
    return 0;
}

const char * sIonAnnotBase::prepareQuerywithTypes(const char * types00, idx format)
{

    rBuf.printf(0, "\
                     a=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end); \
                     unique.1(a.record); \
                     check_off;itraverse(\"this\",\"this\");");

    idx ib = 1;
    for(const char *p = types00; p; p = sString::next00(p), ++ib) {
        rBuf.printf("b%" DEC "=find.annot(seqID=a.seqID,record=a.record,type=\"%s\");", ib, p);
    }

    rBuf.printf("printCSV(a.pos");
    if( format ) {
        for(idx i = 1; i < ib; ++i) {
            rBuf.printf(",b%" DEC ".type,\"?\",b%" DEC ".id", i, i);
        }
    } else {
        for(idx i = 1; i < ib; ++i) {
            rBuf.printf(",b%" DEC ".id", i);
        }
    }

    rBuf.printf(");");

    ts3.resetCompileBuf();

    ts3.traverseRecordSeparator = "|";
    ts3.traverseFieldSeparator = ";";
    ts3.internalSeparator = '-';


    const char * err = ts3.traverseCompile(rBuf.ptr(), rBuf.length());
    return err;
}

const char * sIonAnnotBase::getRecordInfo(const char * seqid, idx seqlen, idx startpos, idx endpos, bool escapeCSV)
{
    ts3.setSearchTemplateVariable("$seqID1", 7, seqid, seqlen);
    ts3.setSearchTemplateVariable("$seqID2", 7, seqid, seqlen);
    rBuf.cut(0);
    rBuf.addNum(startpos);
    rBuf.addString(":0");
    ts3.setSearchTemplateVariable("$start", 6, rBuf.ptr(0), rBuf.length());
    rBuf.add0(1);
    idx initpos = rBuf.length();
    const char * end = rBuf.add("0:", 2);
    rBuf.addNum(endpos);
    ts3.setSearchTemplateVariable("$end", 4, end, rBuf.length() - initpos);

    ts3.traverseBuf.cut(0);
    ts3.resetResultBuf();
    ts3.traverse();

    if( ts3.traverseBuf.length() ) {
        localBuf.cut(0);
        sString::searchAndReplaceStringsPaired(&localBuf, ts3.traverseBuf.ptr(0), ts3.traverseBuf.length(), ";?;" _ ";;?;" __, ":" _ "" __, 0, true, true);
        if( escapeCSV ) {
            rBuf.cut(0);
            if (localBuf.length()>=32000) {
                localBuf.cutAddString(32000,"...",3);
            }
            sString::escapeForCSV(rBuf, localBuf.ptr(0), localBuf.length());
            return rBuf.ptr();
        }
        return localBuf.ptr();
    }

    return 0;

}

idx sIonAnnotBase::getAllGenes(sStr *lbuf, const char * separator)
{
    const char *a = 0;
    if( !separator ) {
        ts4.traverseRecordSeparator = (const char *) &a;
    }
    ts4.resetResultBuf();
    ts4.traverseBuf.cut(0);
    ts4.traverse();
    if( ts4.traverseBuf.length() ) {
        if( lbuf ) {
            lbuf->addString(ts4.traverseBuf.ptr(), ts4.traverseBuf.length());
        }
        idx num = ts4.cntResults;
        return num;
    }
    return 0;
}

const char * sIonAnnotBase::getGeneInfo(const char * seqid, idx seqlen, idx startpos, idx endpos)
{
    ts5.setSearchTemplateVariable("$seqID1", 7, seqid, seqlen);
    ts5.setSearchTemplateVariable("$seqID2", 7, seqid, seqlen);
    rBuf.cut(0);
    rBuf.addNum(startpos);
    rBuf.addString(":0");
    ts5.setSearchTemplateVariable("$start", 6, rBuf.ptr(0), rBuf.length());
    rBuf.add0(1);
    idx initpos = rBuf.length();
    const char * end = rBuf.add("0:", 2);
    rBuf.addNum(endpos);
    ts5.setSearchTemplateVariable("$end", 4, end, rBuf.length() - initpos);

    ts5.traverseBuf.cut(0);
    ts5.resetResultBuf();

    const char *a = 0;
    localBuf.cut(0);
    ts5.pTraverseBuf = &localBuf;
    ts5.traverseRecordSeparator = (const char *) &a;
    ts5.traverseFieldSeparator = ",";

    ts5.traverse();
    if (localBuf.length() == 0){
        return 0;
    }
    localBuf.add0(1);
    return localBuf.ptr();
}
