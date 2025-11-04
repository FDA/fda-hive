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
#include <slib/std.hpp>
#include <slib/utils.hpp>
#include <ulib/ulib.hpp>
#include <ssci/math/rand/rand.hpp>
#include <ssci/bio/sVioAnnot.hpp>



#include <violin/violin.hpp>
#include <xlib/dmlib.hpp>

#include <regex.h>

struct rangeStruc {
        idx start, end;
};

char * skipUntilEOL(const char * ptr, const char * lastpos)
{
    idx p = 0;
    while ( ptr[p]!='\n' && ptr+p<lastpos )
        ++p;

    if (ptr+p<lastpos) ++p;
    return (char *)(ptr +p);
}

bool labelTreatment(sStr & bufLabel,const char * bufLabelToClean, idx bufLen) {
    bufLabel.cut(0);
    char * buf = (char *)bufLabelToClean;
    if (buf[0]=='"' && buf[bufLen-1]=='"') {
        buf++; bufLen = bufLen -2;
    }

    for (idx i=0, pos=0; i< bufLen; ++i){
        while (buf[pos] != ',' && buf && i <bufLen){
            pos++; i++;
        }
        if (!pos) { buf++; continue; }
        bufLabel.addString(buf,pos);
        buf = buf +pos+1 ;
        pos =0;
    }

    return 1;
}


char * scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos)
{
    idx p = 0;
    while ( ptr[p]!='\n' && ptr+p<lastpos && ((ptr[p]=='\t')||(ptr[p]==' ')) )
        ++p;

    const char * start = ptr + p;

    while (ptr+p<lastpos && ptr[p]!='\n' && ptr[p]!=' ' && ptr[p]!='\t')
        ++p;

    ptr = ptr + p;
    if( ptr > start && strVal) {
        strVal->cut(0);
        strVal->add(start, ptr-start);
        strVal->add0();
    }
    return (char *) ptr;

}

idx genericVCFParserToViodb(const char * vcfFilePath)
{
    sFil vcfFileContent(vcfFilePath, sMex::fReadonly);
    if( !vcfFileContent )
        return 0;

    const char * lastPos = vcfFileContent.ptr() + vcfFileContent.length();

    const char * buf = vcfFileContent.ptr();

    for(idx iAl = 0; buf < lastPos; ++iAl) {
        if( buf[0] == '#' && buf[1] =='#') {
            buf = skipUntilEOL(buf, lastPos);
            continue;
        }

        sTxtTbl * tbl = new sTxtTbl();
        tbl->setBuf(buf+1, vcfFileContent.length(), 0);
        tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fTopHeader|sTblIndex::fColsep00;
        tbl->parseOptions().colsep = "\t" __;
        tbl->parse();

        for (idx ir = -1; ir<tbl->rows(); ++ir){
            for (idx ic=0; ic<tbl->cols(); ++ic){
                idx cellLength=0;
                const char * cell = tbl->cell(ir,ic,&cellLength);
                sStr c;
                c.add(cell,cellLength);
                c.add0();
                ::printf("%s,", c.ptr());
            }
            ::printf("\n");
        }



    }

    return 1;
}

idx testDictionary(){
    sDic < sVec <sStr> > mds;
    for (idx jj=0;jj<2;++jj){
        sStr key; key.printf(0,"key%" DEC "",jj);
        sVec < sStr > * spVec = mds.set(key.ptr());
        for (idx ii=0;ii<4;++ii){
            sStr * sp = spVec->add();
            sp->printf("val%" DEC "",ii);
        }
    }


    ::printf("dictionary dimension: %" DEC "\n\n", mds.dim());
    for (idx i=0; i<mds.dim(); ++i) {
        const char * k = (const char *)mds.id(i);

        sVec <sStr> * v = mds.get(k);
        for (idx j=0; j<v->dim();++j){
            ::printf("Element number i = %" DEC "\n",i);
            ::printf("by key: mds[%s]=%s\n",k,v->ptr(j)->ptr(0));
        }
        ::printf("======================================\n");
    }
    return 1;

}

idx testGenbankVioAnnotStruct(){





    return 1;
}

struct startEnd {
        idx start, end, group, max;
        bool exactStart, exactEnd, oneBaseBetween, oneSiteBetween, forward;
        sStr buf;
        startEnd (){
            start = end = group = max = 0;
            buf.cut(0);
            forward = exactStart= exactEnd = true;
            oneBaseBetween = false;
            oneSiteBetween = false;
        }
};


idx strchrFreq (const char * input, const char * srch, idx & frq){
    frq =0;
    idx p =0, start =0;
    idx lastpos = sLen(input);
    idx lenSrch = sLen(srch);
    char * myString = (char *)input;
    while (*myString && p < lastpos){
        if (strncmp(myString,srch,lenSrch)==0){
            frq += 1;
            if (frq == 1) start = p;
        }
        p++;
        myString = myString + 1;
    }
    return start;
}

char * extractContent (const char * input, sStr & dst, const char * startMarkup, const char * endMarkup){
    idx lenStart = sLen(startMarkup);
    idx lenString = sLen(input);
    idx lenEnd = sLen(endMarkup);
    const char * start;

    if (strncmp(startMarkup,input,lenStart)==0){
        start = input + lenStart;
        const char * moveToEnd = input + (lenString - lenEnd);
        if (strncmp(moveToEnd,endMarkup,lenEnd)==0){
                dst.cut(0);
                dst.add(start,lenString-lenStart-lenEnd);
                dst.add0();
        }
        else {
            dst.printf(0,"%s",start);
        }
    }
    else {
        start = input;
        dst.printf(0,"%s",input);
    }
    return (char *)start;
}

void extractLocation(const char * location, startEnd & locationExtracted, bool forward){
    idx freq = 0;
    bool oneExactBase = false, oneBaseBetween = false, isRange = false;
    locationExtracted.forward = forward;
    const char * sep;
    strchrFreq(location,"^", freq);

    if (freq==1) {
        oneExactBase = true;
        sep = "^";
        locationExtracted.oneSiteBetween = true;
        locationExtracted.exactEnd = locationExtracted.exactStart = false;
    }
    idx ss = strchrFreq(location,":", freq);
    if (freq==1) {
        sString::copyUntil(&locationExtracted.buf,location,ss,":");
        location = location + ss +1;
        sep = "..";
    }
    strchrFreq(location,".", freq);
    if (freq==1) {
        oneBaseBetween = true;
        sep = ".";
        locationExtracted.oneBaseBetween = true;
        locationExtracted.exactEnd = locationExtracted.exactStart = false;
    }
    if (freq ==2) {
        isRange = true;
        sep = "..";
    }
    if (oneExactBase == false && oneBaseBetween == false && isRange == false) {
        idx position;
        sscanf(location, "%" DEC "", &position);
        locationExtracted.start = locationExtracted.end = locationExtracted.max =position;

    }
    else {
        sStr buf;
        sString::searchAndReplaceSymbols(&buf,location,0,sep,0,0,true,true,false,true);
        idx cnt = sString::cnt00(buf);
        idx start, end =0;
        if (cnt==2) {
            char * startRaw = sString::next00(buf,0);
            char * endRaw = sString::next00(buf,1);
            if (strncmp("<",startRaw,1)==0) {
                startRaw = startRaw+1;
                locationExtracted.exactStart = false;
            }
            if (strncmp(">",endRaw,1)==0) {
                endRaw = endRaw +1;
                locationExtracted.exactEnd = false;
            }
            sscanf(startRaw, "%" DEC "", &start);
            sscanf(endRaw, "%" DEC "", &end);
            locationExtracted.start = start;
            locationExtracted.end = locationExtracted.max =end;
        }
    }
}


void parseJoin(const char * textRaw, sVec < startEnd > & startEndOut){
    sStr joinTagRemoved;
    extractContent(textRaw,joinTagRemoved , "join(", ")");

    sStr textRawSplit;
    sString::searchAndReplaceSymbols(&textRawSplit,joinTagRemoved.ptr(0),0,",",0,0,true,true,false,true);
    for (const char * cmp = textRawSplit; cmp; cmp = sString::next00(cmp)){
        bool isComplement = (strncmp("complement",cmp,10)==0) ? true : false;
        startEnd * myStartEnd = startEndOut.add();
        if (isComplement){
            sStr text; text.cut(0);
            sString::cleanMarkup(&text,cmp,0,"complement(" _,")" _,0,0,true,false,false);
            extractLocation(text.ptr(1),*myStartEnd,false);
        }
        else  {
            extractLocation(cmp,*myStartEnd,true);
        }
    }
}


void parseComplement(const char * textRaw, sVec < startEnd > & startEndOut){
    sStr complementTagRemoved;
    extractContent(textRaw, complementTagRemoved,"complement(",")");

    bool isJoin = (strncmp("join",complementTagRemoved,4)==0) ? true : false;
    if (isJoin){
        sVec < startEnd > joinSection;
        parseJoin(complementTagRemoved,startEndOut);
    }
    else {
        sStr textRawSplit;
        sString::searchAndReplaceSymbols(&textRawSplit,complementTagRemoved.ptr(0),0,",",0,0,true,true,false,true);
        for (const char * cmp = textRawSplit; cmp; cmp = sString::next00(cmp)){
            startEnd * myStartEnd = startEndOut.add();
            extractLocation(cmp,*myStartEnd,true);
        }

    }



}


void parseToDictionary(const char * src, sDic < sVec <sStr> > & myDict){
    sStr objId;
    idx len = sString::copyUntil(&objId,src,sLen(src),"[");
    if (!len){ return;}
    idx fnd = myDict.find(objId.ptr());
    sVec <sStr> * idType = 0;
    if (len && !fnd){
        idType = myDict.set(objId.ptr());
    }
    const char * sub = sString::searchStruc(src,sLen(src),"[" __,"]" __,0,0);
    if (!sub) {return;}
    sStr t;
    sString::copyUntil(&t,sub,sLen(sub),"]");
    sStr tt;
    sString::searchAndReplaceSymbols(&tt,t.ptr(),0,"|" _,0,0,true,true,true,true);
    for (const char * pptr = tt; pptr; pptr = sString::next00(pptr)){
        idType->add()->printf("%s",pptr);
    }
}

#define posMatrix( _v_i, _v_j, _v_len, _v_mat) (*(_v_mat->ptr( (((_v_i)) * ((_v_len))) + ((_v_j)) )))
idx fuzzyStringCompareDynamat(const char * string1, idx str1Len, const char * string2, idx str2Len, sVec <idx> * matrix )
{

    static idx conventionalPenaltyScore =-1, conventionalMisMatch =-1, conventionalMatchScore = 1;
    matrix->resize(str1Len*str2Len);
    idx * m=matrix->ptr(0);

    idx highestScore = -sIdxMax;
    idx cur_score;
    idx levelp = 0;
    idx prevlevel = 0;

    for (idx i=0; i< str1Len; ++i) {

        for (idx j=0; j< str2Len; ++j) {

            cur_score = ( (i==0 || j==0 ) ? 0 : m[prevlevel+j-1] ) + ((string1[i]==string2[j]) ? conventionalMatchScore : conventionalMisMatch);
            if(j>0) {
                idx leftScore = m[levelp+j-1] + conventionalPenaltyScore;
                if(cur_score<leftScore)
                    cur_score=leftScore;
            }
            if(i>0) {
                idx topScore = m[prevlevel+j] + conventionalPenaltyScore;
                if(cur_score<topScore)
                    cur_score=topScore;
            }

            m[levelp + j]= cur_score;
            if(highestScore<cur_score)
                highestScore=cur_score;
        }
        prevlevel=levelp;
        levelp+=str2Len;
    }
    return highestScore;
}

#define TESTFOLDER "/hive/vol_xfs/store1/231/099/3099231/"

idx generateHeatMap(sStr & pathToTable, sTime * wall_clock = 0, sTime * cpu_clock = 0) {
        sTxtTbl * tbl = new sTxtTbl();
        tbl->setFile(pathToTable);
        tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fTopHeader|sTblIndex::fLeftHeader|sTblIndex::fColsep00;
        if( !tbl->parse() ) {
           ::printf("Failed to parse the table");
           return 1;
        }

        sVec <idx> colSetTree; colSetTree.add(tbl->cols());
        for (idx i=0; i < tbl->cols();++i) { colSetTree[i]=i;}

        sVec <idx> rowSetTree; rowSetTree.add(tbl->rows());
        for (idx i=0; i < tbl->rows();++i) { rowSetTree[i]=i;}

        sTree::DistanceMethods method = sTree::EUCLIDEAN;
        sTree::neighborJoiningMethods njM = sTree::FAST;


        sStr hTreePath("%s_horizontal.tre",TESTFOLDER);
        sFile::remove(hTreePath);
        sFil horizontalTree(hTreePath);

        sVec <idx> actualRowOrder;

        if( wall_clock ) {
            wall_clock->clock(0, 0, true);
        }
        if( cpu_clock ) {
            cpu_clock->clock();
        }
        sTree::generateTree(horizontalTree, &colSetTree, &rowSetTree,tbl,&actualRowOrder,0,0,method,njM);
        return 0;


        return 0;
}

sTime generateTable(sStr & path, idx ncols, idx nrows){

    sTime tmCount; tmCount.time();
    sFile::remove(path);
    sFil myFile(path);

    for (idx ir=0; ir < nrows + 1; ++ir) {
        for (idx ic=0; ic<ncols + 1; ++ic) {
            if (ic) myFile.printf(",");
            if (!ir) {
                myFile.printf("col_%" DEC "",ic);
                continue;
            }
            if (!ic) myFile.printf("row_%" DEC "",ir);
            else {
                myFile.printf("%.4lf",sRand::random1());
            }
        }
        myFile.printf("\n");
    }

    return tmCount;
}

idx collapseTable(sStr & fn){
        sTxtTbl * tbl = new sTxtTbl();
        tbl->setFile(fn);
        tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fColsep00;
        if( !tbl->parse() ) {
           ::printf("Failed to parse the table");
           return 1;
        }

        idx rowCnt = tbl->rows();


        sDic < sDic < sDic < sDic < idx > > >  > aa;
        sDic < sDic < sDic < idx > > > diseaseDic;
        for (idx ir=0; ir<rowCnt; ++ir)
        {
            sStr seq; tbl->printCell(seq,ir,0);
            sStr pos; tbl->printCell(pos,ir,1);
            sStr start; tbl->printCell(start,ir,4);
            sStr end; tbl->printCell(end, ir,5);
            sStr anotId; tbl->printCell(anotId, ir,3);
            sStr type; tbl->printCell(type,ir,6);
            sStr id; tbl->printCell(id,ir,7);
            sStr range; range.printf(0,"%s,%s",start.ptr(),end.ptr());

             if (strncmp(anotId.ptr(),"annotation 2",12)==0 && strncmp(type.ptr(),"Name",4)==0)
            {
              idx *iid = aa.set(seq.ptr())->set(id.ptr())->set(range.ptr())->set(pos.ptr());
              *iid=1;
            }
            else if (strncmp(anotId.ptr(),"annotation 1",12)==0 && strncmp(type.ptr(),"Disease Ontology",15)==0)
            {
                idx * iid = diseaseDic.set(seq.ptr())->set(pos.ptr())->set(id.ptr());
                *iid=1;
            }
        }

        sFil out("/home/lam/test/funcElementRangeSummary.csv");
        out.printf("seqID,DNAFunc,start,end,disease,cntDisease\n");

        sFil outF("/home/lam/test/funcElementWithoutRangeSummary.csv");
        outF.printf("seqID,DNAFunc,disease,cntDisease\n");
        for (idx ii=0; ii < aa.dim(); ++ii)
        {
            idx seqLen =0; idx posLen=0;
            const char * seq = (const char *)aa.id(ii,&seqLen);
            for (idx jj=0; jj < aa.ptr(ii)->dim(); ++jj)
            {
                const char * func = (const char *)aa.ptr(ii)->id(jj,&posLen);
                sDic < idx > funcDis;
                for (idx zz=0; zz < aa.ptr(ii)->ptr(jj)->dim(); ++zz)
                {
                    const char * range = (const char *)aa.ptr(ii)->ptr(jj)->id(zz,&posLen);
                    sDic < idx > d;
                    for (idx kk=0; kk< aa.ptr(ii)->ptr(jj)->ptr(zz)->dim(); ++kk)
                    {
                        const char * pos = (const char *)aa.ptr(ii)->ptr(jj)->ptr(zz)->id(kk,&posLen);
                        sDic <idx> * disease= diseaseDic.get(seq)->get(pos);
                        for (idx ll=0; ll<disease->dim();++ll)
                        {
                            sStr dd;
                            dd.printf("%s",(const char *)disease->id(ll));
                            idx * iid = d.set(dd.ptr());
                            *iid=1;
                            idx * iif = funcDis.set(dd.ptr());
                            *iif=1;
                        }
                    }
                    sStr ds;
                    for (idx qq=0; qq<d.dim(); ++qq)
                    {
                        ds.printf("%s;",(const char *)d.id(qq));
                    }
                    out.printf("%s,%s,%s,%s,%" DEC "\n",seq, func, range,ds.ptr(0),d.dim());
                }
                sStr fs;
                for (idx qq=0; qq<funcDis.dim(); ++qq)
                {
                    fs.printf("%s;",(const char *)funcDis.id(qq));
                }
                outF.printf("%s,%s,%s,%" DEC "\n",seq,func,fs.ptr(),funcDis.dim());
            }
        }


        return 0;
}


bool searchSeqId(const char * idSrc, idx idLen, const char * ncbiLikeId, idx ncbiLen) {

    if (!idLen) idLen = sLen(idSrc);
    if (!ncbiLen) ncbiLen = sLen(ncbiLikeId);

    const char * nxt=strchr(ncbiLikeId, ' ');
    if(nxt) {
        ncbiLen = nxt - ncbiLikeId;
    }
    idx curP=0;
    for( const char * p=ncbiLikeId; (curP<ncbiLen) && p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
        nxt=strpbrk(p,"|");

        if(!nxt || *nxt==' ')
            break;

        const char * curId=nxt+1;
        nxt=strpbrk(nxt+1," |");

        if (strncmp(idSrc,curId,idLen)==0) {
            return true;
        }
        curP = nxt-ncbiLikeId;
    }
    return false;
}



int main(int argc, const char *argv[])
{


    return 0;
    sStr fn("/home/lam/test/mapperResults.csv");

    collapseTable(fn);


    return 0;
    sStr p("%s_perf_20.csv",TESTFOLDER);
    sFil perf(p);
    perf.printf("rows,cols,wall time (s),cpu clock (s)\n");
    static const idx colNum[] = { 1000, 2000, 3000, 4000, 5000, 6000 };
    static const idx rowNum[] = { 1, 5, 10, 20 };

    sStr fp;
    sTime wall_clock, cpu_clock;
    for (idx i = 0; i < sDim(rowNum); i++) {
        for (idx j=0; j < sDim(colNum); ++j) {
            perf.printf("%" DEC ",%" DEC "",rowNum[i],colNum[j]);
            fp.printf(0,"%ssample_%" DEC "-rows_%" DEC "-cols.csv",TESTFOLDER,rowNum[i],colNum[j]);
            ::printf("\n Generating Table %" DEC "-rows_%" DEC "-cols\n",rowNum[i],colNum[j]);
            wall_clock.clock(0, 0, true);
            cpu_clock.clock();
            generateTable(fp,colNum[j],rowNum[i]);
            ::printf(" ==> table generation within %.2g seconds (%.2g CPU) \n", wall_clock.clock(0, 0, true), cpu_clock.clock());
            ::printf(" Computing HeatMap \n");
            generateHeatMap(fp, &wall_clock, &cpu_clock);
            real wall_clock_sec = wall_clock.clock(0, 0, true);
            real cpu_clock_sec = cpu_clock.clock();
            perf.printf(",%.2g,%.2g\n", wall_clock_sec, cpu_clock_sec);
            ::printf(" ==> heatMap generation within %.2g seconds (%.2g CPU) \n", wall_clock_sec, cpu_clock_sec);
        }
    }




    return 0;
































    return 0;
}
