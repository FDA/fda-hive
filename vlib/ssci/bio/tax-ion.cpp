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

#include <ion/sIon.hpp>
#include <ssci/bio/tax-ion.hpp>
#include <regex.h>

using namespace slib;

const char * sTaxIon::initPrecompile(void)
{
    sStr trbuf;
    trbuf.printf("\
                o=find.accession_taxid(accession=='to_modify') ; \
                print(o.taxid); ");
    tsTIBG.traverseFieldSeparator = ",";
    tsTIBG.traverseRecordSeparator = "";
    const char * err = tsTIBG.traverseCompile(trbuf.ptr(), trbuf.length());
    if( err ) {
        return err;
    }
    argTIBG = (idx*) tsTIBG.getSearchArgumentPointer(0);

    trbuf.printf(0, "\
        a=find.taxid_name(taxid=='to_modify', tag='scientific name'); \
        b=find.taxid_parent(taxid==a.taxid); \
        c=find.taxid_parent(parent==b.taxid);\
        d=find.taxid_name(taxid==c.taxid, tag='scientific name'); \
        e=count.taxid_parent(parent==c.taxid); \
        printCSV(c.taxid,b.taxid,c.rank,d.name,e.#);  ");
    tsTree.traverseFieldSeparator=",";
    tsTree.traverseRecordSeparator="";
    const char * err2=tsTree.traverseCompile(trbuf.ptr() ,trbuf.length());
    if (err2){
        return err2;
    }
    argTTree=(idx*)tsTree.getSearchArgumentPointer( 0);

    trbuf.printf (0, "\
        o=count.taxid_parent(rank=='to_modify'); \
        print(o.#);");
    ts2.traverseRecordSeparator="";
    const char * err3=ts2.traverseCompile(trbuf.ptr() ,trbuf.length());
    if (err3){
        return err3;
    }
    argT2=(idx*)ts2.getSearchArgumentPointer(0);

    trbuf.printf (0, "\
        o=find.taxid_parent(taxid=='to_modify'); \
        print(o.parent,o.rank);");
    ts3.traverseFieldSeparator=",";
    ts3.traverseRecordSeparator="";
    const char * err4=ts3.traverseCompile(trbuf.ptr() ,trbuf.length());
    if (err4){
        return err4;
    }
    argT3=(idx*)ts3.getSearchArgumentPointer(0);

    trbuf.printf (0, "\
        o=find.taxid_name(taxid=='to_modify',tag=='scientific name');\
        a=find.taxid_parent(taxid==o.taxid); \
        b=count.taxid_parent(parent==o.taxid);\
        printCSV(o.taxid,a.parent,a.rank,o.name,b.#);\
        ");
    ts4.traverseFieldSeparator=",";
    ts4.traverseRecordSeparator="";
    const char * err5=ts4.traverseCompile(trbuf.ptr() ,trbuf.length());
    if (err5){
        return err5;
    }
    argT4=(idx*)ts4.getSearchArgumentPointer(0);

    return 0;

}

const char *sTaxIon::precompileGItoAcc(void)
{
    sStr trbuf;
    trbuf.printf(0, "\
        o=find.gi_accession(gi=='to_modify'); \
                print(o.accession);");
    ts1.traverseFieldSeparator = ",";
    ts1.traverseRecordSeparator = "";
    const char * err1 = ts1.traverseCompile(trbuf.ptr(), trbuf.length());
    if( err1 ) {
        return err1;
    }
    argT1 = (idx*) ts1.getSearchArgumentPointer(0);
    return 0;
}

idx sTaxIon::getTaxCount (const char * rank, idx ranklen)
{
    idx len = ranklen;
    if (!len){
        len = sLen (rank);
    }
    idx pos = ts2.traverseBuf.length();
    argT2[0]=sConvPtr2Int(rank);
    argT2[1]=len;
    ts2.traverse();

    taxbuf.cut(0);
    if (ts2.traverseBuf.length()-pos > 0){
        taxbuf.addString(ts2.traverseBuf.ptr(pos), ts2.traverseBuf.length()-pos);
        return atoidx(taxbuf.ptr(0));
    }
    return 0;
}

idx sTaxIon::getRecordCount (idx recordTypeIndex)
{
    return ION->getRecordCount(recordTypeIndex);
}

const char * sTaxIon::getAccByGi(const char * ginum, idx gilen )
{
    if (!gilen){
        gilen = sLen(ginum);
    }
    if (strncmp(ginum,"-1",gilen) == 0){
        return "-1";
    }

    idx pos = ts1.traverseBuf.length();
    argT1[0]=sConvPtr2Int(ginum);
    argT1[1]=gilen;
    ts1.traverse();

    taxbuf.cut(0);
    if (ts1.traverseBuf.length()-pos > 0){
        taxbuf.addString(ts1.traverseBuf.ptr(pos), ts1.traverseBuf.length()-pos);
        return taxbuf.ptr(0);
    }
    return 0;

}

const char * sTaxIon::getTaxIdsByAccession(const char * gilist )
{
    idx pos = tsTIBG.traverseBuf.length();
    argTIBG[0]=sConvPtr2Int(gilist);
    argTIBG[1]=sLen(gilist);
    tsTIBG.traverse();

    taxbuf.cut(0);
    if (tsTIBG.traverseBuf.length()-pos > 0){
        taxbuf.addString(tsTIBG.traverseBuf.ptr(pos), tsTIBG.traverseBuf.length()-pos);
        return taxbuf.ptr(0);
    }
    return 0;
}

const char * sTaxIon::getTaxTreeChildrenInfo(const char * taxlist, idx taxlen, sStr *buf )
{
    idx pos = tsTree.traverseBuf.length();
    if (!taxlen){
        taxlen = sLen(taxlist);
    }
    argTTree[0]=sConvPtr2Int(taxlist);
    argTTree[1]=taxlen;
    tsTree.traverse();

    const char *retstring = 0;
    if (tsTree.traverseBuf.length()-pos > 0){
        if (buf){
            retstring = buf->addString(tsTree.traverseBuf.ptr(pos), tsTree.traverseBuf.length()-pos);
        }
        else {
            taxbuf.cut(0);
            retstring = taxbuf.addString(tsTree.traverseBuf.ptr(pos), tsTree.traverseBuf.length()-pos);
        }
    }
    return retstring;
}

const char * sTaxIon::getParentTaxIds(const char * taxlist, sStr *taxParent, sStr *taxRank )
{
    ts3.traverseBuf.cut(0);
    argT3[0]=sConvPtr2Int(taxlist);
    argT3[1]=sLen(taxlist);
    ts3.traverse();

    taxbuf.cut(0);
    if (ts3.traverseBuf.length() > 0){
        sString::searchAndReplaceSymbols(&taxbuf, ts3.traverseBuf.ptr(0), ts3.traverseBuf.length(), ",", 0, 0, true, true, true, true);

        const char * result00 = taxbuf.ptr(0);
        taxParent->addString(result00);
        if (taxRank){
            taxRank->addString(sString::next00(result00));
        }
    }
    return taxbuf.ptr(0);
}

bool sTaxIon::filterbyParent (idx taxPar, idx taxid, sStr *path, bool taxidOnly, const char *fieldSeparator, const char * recordSeparator)
{
    sStr taxParent, taxCurrent;
    idx taxId;
    taxCurrent.cut(0);
    taxCurrent.addNum(taxid);
    do{
        taxParent.cut(0);
        getParentTaxIds(taxCurrent, &taxParent);
        taxId = atoidx((const char *)taxCurrent.ptr());
        if (path){
            if (taxidOnly){
                path->add(taxCurrent.ptr(), taxCurrent.length());
                path->add0();
            }
            else {
                const char *info = getTaxIdInfo(taxCurrent.ptr(), taxCurrent.length(), fieldSeparator);
                path->addString(info);
                if(recordSeparator)
                    path->addString(recordSeparator);
                else
                    path->add0();
            }
        }
        if (taxId == taxPar){
            return true;
        }
        taxCurrent.cut(0);
        taxCurrent.addString(taxParent.ptr(), taxParent.length());
    }while (taxId > 1);

    return false;
}

idx sTaxIon::filterByRank(sStr *dstTax, const char * srcTax, const char * requiredRank)
{
    idx taxId;
    if(strcmp(requiredRank, "leaf") == 0){
        dstTax->addString(srcTax);
        taxId = atoidx((const char *)srcTax);
        if (taxId < 0){
            dstTax->printf(0,"NO_INFO");
            taxId = 0;
        }
        return taxId;
    }
    sStr taxParent, taxRank, taxCurrent;
    taxCurrent.addString(srcTax);
    do{
        taxParent.cut(0);
        taxRank.cut(0);
        getParentTaxIds(taxCurrent, &taxParent, &taxRank);
        taxId = atoidx((const char *)taxCurrent.ptr());
        if (taxParent.length() && (strcmp (taxRank.ptr(), requiredRank) == 0)){
            dstTax->addString(taxCurrent);
            return taxId;
        }
        taxCurrent.printf(0,"%s", taxParent.ptr());
    }while (taxId > 1);

    dstTax->printf(0,"NO_INFO");
    return 0;
}

const char * sTaxIon::getTaxIdsByRankandAcc(const char * acclist, const char * rank )
{
    sStr trbuf;
    trbuf.printf(0,"\
        k=foreach(%s); \
        o=find.accession_taxid(accession==k.) ; \
        b=taxid_parent(taxid==o.taxid); \
        jump!.b(b.rank, '%s'); \
        print(b.taxid); ",
        acclist,  rank);
    ts.traverseFieldSeparator=",";
    ts.traverseRecordSeparator="";

    const char * err=ts.traverseCompile(trbuf.ptr() ,trbuf.length());
    if (err){
        return err;
    }
    ts.traverseBuf.cut0cut(0);
    ts.traverse();

    return ts.traverseBuf.ptr(0);
}

const char * sTaxIon::getTaxIdsByName (const char *name, idx limit, sStr *taxResults00)
{
    if (!limit){
        limit=sIdxMax;
    }
    sStr trbuf;
    trbuf.printf (0, "\
        o=search.taxid_name(name='regex:%s')[:%" DEC "]; \
        print(o.taxid,o.name);", name, limit);
    ts.traverseFieldSeparator=",";
    ts.traverseRecordSeparator="";

    ts.resetCompileBuf();
    const char * err=ts.traverseCompile(trbuf.ptr() ,trbuf.length());
    if (err){
        return err;
    }
    ts.traverseBuf.cut0cut(0);
    ts.traverse();

    if (taxResults00 && ts.traverseBuf.length()){
        taxResults00->add(ts.traverseBuf.ptr(0), ts.traverseBuf.length());
        taxResults00->add0cut(2);
    }
    return ts.traverseBuf.ptr(0);

}

const char *sTaxIon::extractTaxIDfromSeqID(sStr *taxid, sStr *ginum, sStr *accnum, const char *seqid, idx seqlen, const char *defaultStr)
{
    lbuf.cut(0);
    bool isValid = sString::extractNCBIInfoSeqID(ginum, &lbuf, seqid, seqlen);
    const char *tid = 0;
    bool isAccnum = (lbuf.length()) ? true : false;

    if( isValid ) {
        if( isAccnum ) {
            tid = taxid->addString(getTaxIdsByAccession(lbuf.ptr()));
        }
    }
    if( !(tid && *tid) && defaultStr ) {
        if (ginum){
            ginum->addString(defaultStr);
        }
        if (!isAccnum){
            lbuf.addString(defaultStr);
        }
        tid = taxid->addString(defaultStr);
    }

    if (accnum){
        accnum->add(lbuf.ptr(), lbuf.length());
    }
    return tid;
}

const char *sTaxIon::getTaxIdInfo (const char *taxid, idx taxidlen, const char *fieldSeparator )
{
    if (!taxidlen){
        taxidlen = sLen(taxid);
    }
    ts4.traverseFieldSeparator = fieldSeparator;
    ts4.traverseBuf.cut0cut(0);
    argT4[0]=sConvPtr2Int(taxid);
    argT4[1]=taxidlen;
    ts4.traverse();

    return ts4.traverseBuf.ptr(0);
}

idx sTaxIon::dnaScreeningGItoAcc(sStr &out, const char *src)
{
    sTxtTbl srcTable;
    srcTable.setFile(src, sMex::fReadonly);

    srcTable.parseOptions().dimLeftHeader = 0;
    srcTable.parse();

    idx cols = srcTable.cols();
    out.cut(0);
    srcTable.printCSV(out,0,1,0,cols,true);

    sStr gi, accession;
    idx cntRows = 0;
    for(idx ir = 0; ir < srcTable.rows(); ++ir) {
        gi.cut(0);
        srcTable.printCell(gi,ir,0,0);
        if (!gi.length()){
            continue;
        }
        const char *accnum= getAccByGi(gi.ptr(),gi.length());
        if (!accnum){
            continue;
        }
        out.printf("%s,",accnum);
        srcTable.printCSV(out,ir,2,1,cols,false);
        ++cntRows;
    }
    return cntRows;
}
