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
#include <ssci/bio/sVioAnnot.hpp>
#include <violin/hiveannot.hpp>
#include <violin/hiveutils.hpp>


using namespace sviolin;

void sHiveannot::InitAnnotList(sUsr * user, sVec<sVioAnnot> & annotList, sVec<sHiveId> * annotIDListToUse, bool getAll)
{
    sVec<sHiveId> annotIDList;

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
                }
                if (tableOut){
                    tableOut->printf("%s,%s,%" DEC "\n",obj.Id().print(), obj.propGet("name"), sFile::time(path));
                }
            }
        }
    }
}

idx sHiveannot::outInfo (sStr & output, const char * inputRangeTable, sVec< sVioAnnot > & anotList){

    sFil crossRange(inputRangeTable,sMex::fReadonly);
    const char * filebody = crossRange.ptr();

    output.printf("Reference,Overlap_start,Overlap_end,FileName,Range_start,Range_end,Annotation_type,Annotation_id\n");

    sTxtTbl * tbl = new sTxtTbl();
    tbl->setBuf(filebody, crossRange.length(), 0);
    tbl->parseOptions().flags = sTblIndex::fSaveRowEnds|sTblIndex::fTopHeader|sTblIndex::fColsep00;
    tbl->parseOptions().colsep = "," __;
    tbl->parse();
    tbl->parseOptions().colsep = 0;

    idx tblRowLen = tbl->rows();
    for (idx irow = 0; irow < tblRowLen; irow++) {
        sStr reference, startAsString, endAsString;
        tbl->printCell(reference,irow,0);
        tbl->printCell(startAsString,irow,1);
        tbl->printCell(endAsString,irow,2);

        idx start=0, end=0;
        sscanf(startAsString.ptr(),"%" DEC "",&start);
        sscanf(endAsString.ptr(),"%" DEC "",&end);

        for (idx iannot=0; iannot<anotList.dim(); ++iannot){
            sVioAnnot * ia = anotList.ptr(iannot);

            idx idIndex = ia->getIdIndex(reference.ptr(),"seqID");
            idx typeIdIdx = ia->getIdTypeIdx();
                ia->getRangeTypeIdx();

            idx relationCnt= 0, relationTypeIndex = 0;
            idx * indexPtrRange= ia->DB.GetRelationPtr(typeIdIdx, idIndex, 1,&relationCnt,&relationTypeIndex);

            if (!relationCnt)
                continue;


            idx resultSize=0;
            sVec <sVioAnnot::startEndNode> resStruct;
  PERF_START("Search Virtual Tree");
            resultSize = ia->searchInVirtualTree(indexPtrRange,relationCnt,resStruct,start,end);
  PERF_END();
             if (!resultSize) {
                continue;
             }
             for (idx iRange=0; iRange <resStruct.dim(); ++iRange){
                 idx cntIDsForRange=ia->getNberOfIdsByRangeIndex(resStruct[iRange].index);

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

sHiveannot::sHiveannot(sUsr * user, const char * annotation_source, const char * annotation_enrichment, sVec < sHiveId > * infoSrcList, sStr * srcNames, sDic < idx > * columnIds)
{
    hionAnnot.init(user,annotation_source);
    if(annotation_source) {
        hionAnnot.addIonWander("mapGenePos","a=find.annot(#range=possort-max,$seqID1,$start,$seqID1,$end);b=find.annot(record=a.record);unique.1(b.id);print(b.pos,b.type,b.id);");
    }
    if(infoSrcList)
        sHiveUtils::tblReadInfoSources(user,infoSrcList,&infoSet,srcNames,columnIds);

    if(annotation_enrichment) {
        hionAnnot.addIonWander("mapAddPos","a=find.annot(#range=possort-max,$seqID1,$start,$seqID1,$end);b=find.annot(record=a.record);unique.1(b.id);print(b.pos,b.type,b.id);");
        sHiveUtils::TBLINFO * ti=infoSet.add();
        ti->type=sHiveUtils::TBLINFO::eANNOT;
    }
}

#define scanTillChar(_v_p, _v_ch , _v_len, _v_doquote)  { \
    idx incomment=0, dslash=0; \
    { \
        while( strchr((_v_ch),*(_v_p)) && (_v_p)<recNext) { \
            if(*(_v_p)=='\n'){ ++lineCount; lineStart=nextLine;nextLine=_v_p+1-srcStart;} \
            ++(_v_p);} \
        } \
    } \
    if( (_v_p)>=recNext) break;


idx sHiveannot::mapPosToGeneInfo(sVar * var , const char * refid, idx idlen,  idx start ,idx end, idx varStart, idx varEnd)
{
    sIonWander * wander=hionAnnot.wanderList.get("mapGenePos");
    sIonWander * wanderAdd=hionAnnot.wanderList.get("mapAddPos");
    sStrT buf;
    const char * chr=0; idx chrlen=0;
    if(strncmp(refid,"chromosome:GRCh",15)==0 ) {
        chr=strchr(refid+15,':')+1;
        const char * p=strchr(chr,':');
        if(p)chrlen=p-chr;
    }
    Chromosome.cut(0);Chromosome.add(chr ? chr : refid, chr ? chrlen : idlen); Chromosome.add0();
    gene_name.cut(0);gene_name.add0(1);gene_name.cut(0);
    sStr ss,ee;
    if(wander){
        wander->setSearchTemplateVariable("$seqID1", 7, chr ? chr : refid, chr ? chrlen : idlen);
        wander->setSearchTemplateVariable("$start", 6, ss.printf(0,"%" DEC ":%" DEC ,start, start) ,0 );
        wander->setSearchTemplateVariable("$end", 4, ee.printf(0,"%" DEC ":%" DEC, end, end) , 0 );
        wander->resetResultBuf();
        wander->traverse();
        if(wander && wander->traverseBuf.length()){
            const char * gn=strstr(wander->traverseBuf.ptr(),"gene_name,");
            if(gn)sString::copyUntil(&gene_name,gn+10,0,"\n");
            sHiveUtils::tblOutInfoSources(var, gene_name,&infoSet);

        }
    }
    if(wanderAdd) {
        wanderAdd->setSearchTemplateVariable("$seqID1", 7, chr ? chr : refid, chr ? chrlen : idlen);
        wanderAdd->setSearchTemplateVariable("$start", 6, ss.printf(0,"%" DEC ":%" DEC ,varStart, varStart) ,0 );
        wanderAdd->setSearchTemplateVariable("$end", 4, ee.printf(0,"%" DEC ":%" DEC, varEnd, varEnd) , 0 );
        wanderAdd->resetResultBuf();
        wanderAdd->traverse();
        idx len, fl,vl,sl,el;
        sStrT t,b;
        if(wanderAdd && (len=wanderAdd->traverseBuf.length())){
            char * p=wanderAdd->traverseBuf.ptr(), * last=p+len;
            while( p<last ){
                char * s=p;
                while(p<last && *p!='\n'){if(*p==',')*p=0;++p;}if(*p=='\n')*p=0;
                char * e=s+(sl=sLen(s))+1;char * f=e+(el=sLen(e))+1;char * v=f+(fl=sLen(f))+1;
                var->inp(f, v, (vl=sLen(v)), fl);
                t.printf(0,"%.*s-POS",(int)fl,f);
                b.printf(0,"%.*s-%.*s",(int)sl,s,(int)el,e);
                var->inp(t.ptr(), b.ptr(), b.length(),t.length());

                if(p==last-1)break;
                ++p;
            }
        }
    }

    return var->dim();
}

const char * sHiveannot::value(const char * var, sStr * dst, bool cleanBuf)
{

    sIonWander * wander=hionAnnot.wanderList.get("mapGenePos");
    if( !wander || wander->traverseBuf.length()==0)return "";
    char varname[1024];varname[0]=',';
    strcpy(varname+1,var);strcat(varname+1,",");

    const char * gn=strstr(wander->traverseBuf.ptr(),varname);
    if(!gn)return "";
    if(!dst){dst=&lBuf;cleanBuf=true;}
    if(cleanBuf){dst->cut(0);}
    sString::copyUntil(dst,gn+sLen(varname),0,"\n");
    return dst->ptr(0);
}

