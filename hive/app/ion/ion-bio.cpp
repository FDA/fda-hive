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
#include "ion-tools.hpp"
#include <ssci/bio/vax-bio.hpp>
#include <slib/utils/tbl.hpp>







static idx sVaxGTF_internalColumnMap[]={1,2,5,6,7,8,sNotIdx};
static idx sVaxVCF_internalColumnMap[]={2,3,4,7,sNotIdx};
static idx sVaxSNP_internalColumnMap[]={4,5,6,7,8,9,11,13,sNotIdx};
extern sStr vaxNameInclusionList;
extern sStr vaxNameExclusionList;
extern sStr vaxValueExclusionList;
extern sDic < char> nameInclDic;
static sVec <idx > sSNP_columnMap;
extern sIon * ION;
extern sIO dbg;

idx __on_ion(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);

idx __on_bio(sIonTools * iap, const char * cmd, const char * args, const char * ,sVar * pForm)
{
    if(iap->gLazy)
        return 0;

    if(sIs(cmd,"-seqParse")) {
    }
    else if(sIs(cmd,"-gtfLazyParse")){
        const char * gtfInfile=pForm->value("gtfFile");
        sStr localbuf ("%s.ion", gtfInfile);
        bool parseGTF = true;
        if (sFile::exists(localbuf.ptr(0))){
            if (sFile::time(gtfInfile) < sFile::time(localbuf.ptr())){
                parseGTF = false;
            }
         }
        if (parseGTF){
            pForm->inp("ionname", gtfInfile);
            __on_ion(iap, "-ionCreate", args, 0, pForm);
            gtfInfile = pForm->value("ionname");
            __on_bio(iap, "-gtfParse", gtfInfile, 0, pForm);
        }
    }
    else if(sIs(cmd,"-gtfParse") || sIs(cmd,"-gffParse")) {

        sVaxAnnot vax(ION,0,3,4,sSNP_columnMap.dim() ? sSNP_columnMap.ptr() : (idx*)sVaxGTF_internalColumnMap);
        vax.recordStart=iap->gStart;
        vax.recordCnt=iap->gCnt;
        vax.commentMarker="###" __;
        if(vaxNameInclusionList.length())vax.inclusionNames=vaxNameInclusionList.ptr();
        if(vaxValueExclusionList.length())vax.exclusionValues=vaxValueExclusionList.ptr();



        vax.setSepar("\t",(const char*)0,(const char *)0, sNotIdx );
        vax.tableHeaderParse("seqID\tsource\tfeature\tstart\tend\tsignal\tstrand\tframe\tattribute",0);
        vax.setSepar(0,0,sIs(cmd,"-gffParse") ? "=;" : " ;",8);
        vax.needsCleanup=true;
        for( const char * p=args; p; p=sString::next00(p))
        {
            vax.init((iap->stream ? sVax::fUseFStream : sVax::fUseMMap)|sVax::fDoNotSupportTableHeader,p);
            ION->providerLoad(sVax::ionProviderCallback,(void*)&vax,0,iap->gStart,iap->gUseSynonym ? sFlag(sIon::fValueFormatting) : 0);
        }

    }
    else if( sIs(cmd,"-oboParse") ){
        const char * oboFile=pForm->value("obofile");

        const char * controlFile=
            "record,#R,#,ordered\n"
            "record,name,string,string\n"
            "record,value,string,string\n"
            "record,tbl,string,string\n"
            "relation,row,#R|name|value|tbl,#R,#R|name,#R|value,name|value,name,value,tbl|name,tbl|#R,tbl|#R|name,tbl|name|value,#R|name|value\n";


        ION->constructRecordAndRelationTypes(controlFile, sLen(controlFile));


        idx iRow=0;
        sFil fil(oboFile);

        enum {tbl_R=0,tbl_name,tbl_value,tbl_tbl};

        oboFile=sFilePath::nextToSlash(oboFile);
        idx arr[4];
        arr[0]=-1;
        arr[3]=ION->addRecord(tbl_tbl,sLen(oboFile),oboFile);


        for(const char * ptr=fil.ptr(), * end=fil.last(), *next=ptr; ptr<end; ptr=next) {


            while(*next!='\n' && next<end)++next;
            ++next;
            if(next-ptr<=1)
                continue;

            if((next-ptr)>=6 && strncmp(ptr,"[Term]",6)==0){
                arr[0]=ION->addRecord(tbl_R,sizeof(iRow),&iRow);
                ++iRow;
                continue;
            }
            if(arr[0]<0)
                continue;

            const char * colon=ptr;
            while(*colon!=':' && colon<next)++colon;
            if(colon>=next)
                continue;

            arr[1]=ION->addRecord(tbl_name,colon-ptr,ptr);
            for(++colon; strchr(sString_symbolsSpace, *colon)!=0 ; ++colon);
            idx len=next-colon-1;
            for(; strchr(sString_symbolsSpace, colon[len])!=0 ; --len);
            arr[2]=ION->addRecord(tbl_value,len,colon);

            ION->addRelationVarg(0,sNotIdx,arr,0);

        }


    }
    else if(sIs(cmd,"-vcfParseSpecial")) {

        sVaxAnnot vax(ION,0,1,1,sSNP_columnMap.dim() ? sSNP_columnMap.ptr() : 0);
        vax.vaxMarker="##";
        vax.recordStart=iap->gStart;
        vax.recordCnt=iap->gCnt;
        vax.callbackProgress=sVax::progressReport;


        if(vaxNameInclusionList.length())vax.inclusionNames=vaxNameInclusionList.ptr();
        if(vaxNameExclusionList.length())vax.exclusionNames=vaxNameExclusionList.ptr();
        if(vaxValueExclusionList.length())vax.exclusionValues=vaxValueExclusionList.ptr();
        if(nameInclDic.dim())vax.nameInclusionDic=&nameInclDic;

        vax.setSepar("\t",(const char*)0,(const char *)0, sNotIdx );
        vax.needsCleanup=true;
        for( const char * p=args; p; p=sString::next00(p))
        {
            vax.init(iap->stream ? sVax::fUseFStream : sVax::fUseMMap,p);
            vax.gLog=&dbg;
            ION->providerLoad(sVax::ionProviderCallback,(void*)&vax,0,iap->gStart,sFlag(sIon::fDoNotLoadIncompleteRelation));
        }

    } else if(sIs(cmd,"-vcfParse")) {

        sVaxAnnot vax(ION,0,1,1,sSNP_columnMap.dim() ? sSNP_columnMap.ptr() : (idx*)sVaxVCF_internalColumnMap);
        vax.recordStart=iap->gStart;
        vax.recordCnt=iap->gCnt;
        if(vaxNameInclusionList.length())vax.inclusionNames=vaxNameInclusionList.ptr();
        if(vaxValueExclusionList.length())vax.exclusionValues=vaxValueExclusionList.ptr();

        vax.setSepar("\t",(const char*)0,(const char *)0, sNotIdx );
        vax.tableHeaderParse("CHROM\tPOS\trsID\tref\talt\tQUAL\tFILTER\tINFO",0);
        vax.setSepar(0,0,"=;",7);
        vax.needsCleanup=true;
        for( const char * p=args; p; p=sString::next00(p))
        {
            vax.init(sVax::fUseMMap|sVax::fDoNotSupportTableHeader,p);
            ION->providerLoad(sVax::ionProviderCallback,(void*)&vax,0,iap->gStart,sFlag(sIon::fDoNotLoadIncompleteRelation));
        }

    } else if(sIs(cmd,"-gbParse")) {

        sVaxAnnotGB vax(ION);
        vax.recordStart=iap->gStart;
        vax.recordCnt=iap->gCnt;
        for( const char * p=args; p; p=sString::next00(p))
        {
            vax.init(sVax::fUseMMap|sVax::fDoNotSupportTableHeader,p);
            ION->providerLoad(sVax::ionProviderCallback,(void*)&vax,0,iap->gStart);
        }
        cmd="-annotSort";
    }

    else if(sIs(cmd,"-annotParse")) {
        const char * annotFile=pForm->value("annotFile");
        idx seqIDCol=pForm->ivalue("seqID",0);
        idx startCol=pForm->ivalue("start",-1);
        idx endCol=pForm->ivalue("end",-1);
        const char * separCol=pForm->value("separ");
        if(separCol) {
            if(!strcmp(separCol,"tab")) separCol="\t";
        }else separCol=",";


        sVaxAnnot vax(ION,seqIDCol, startCol,endCol,sSNP_columnMap.dim() ? sSNP_columnMap.ptr() : 0 );
        if(vaxNameInclusionList.length())vax.inclusionNames=vaxNameInclusionList.ptr();
        if(vaxValueExclusionList.length())vax.exclusionValues=vaxValueExclusionList.ptr();
        vax.setSepar(separCol,(const char*)0,(const char *)0, sNotIdx );


        vax.recordStart=iap->gStart;
        vax.recordCnt=iap->gCnt;
        vax.needsCleanup=true;
        vax.init(sVax::fUseMMap,annotFile);
        ION->providerLoad(sVax::ionProviderCallback,(void*)&vax,0,iap->gStart);
    }

    else if(sIs(cmd,"-snpParse")) {
        const char * annotFile=pForm->value("snpFile");

        sVaxAnnot vax(ION,0,1,1,(idx*)sVaxSNP_internalColumnMap);
        vax.recordStart=iap->gStart;
        vax.recordCnt=iap->gCnt;

        vax.init(sVax::fUseMMap,annotFile);
        ION->providerLoad(sVax::ionProviderCallback,(void*)&vax,0,iap->gStart);
    }

    if(sIs(cmd,"-annotColumnMap")) {
        sString::scanRangeSet(pForm->value("columns"),0,&sSNP_columnMap,0,0,0);

    }
    else if(sIs(cmd,"-annotSort")) {
        sVec<idx> recordTypesUsed;
        ION->sortRelations("annot","seqID pos", pForm->value("sortFile", "possort"), 0, 0,&recordTypesUsed, sizeof(int),0 );

        sVec < sIon::RecordResult > rr;rr.resize(2*recordTypesUsed.dim());
        idx cnt=recordTypesUsed.dim() ;
        for( idx ir=0; ir<cnt; ++ir) {
            rr[ir].typeIndex=recordTypesUsed[ir];
            rr[ir+cnt].typeIndex=recordTypesUsed[ir];
        }
        ION->buildVTree("annot", pForm->value("sortFile", "possort"), pForm->value("vTreeName","max"), rr.ptr(0), rr.ptr(cnt ),cnt );
    }
    else if(sIs(cmd,"-parseConventionalExpression")) {

            const char * experiment = pForm->value("experiment","experiment");
            const char * expressionFile=pForm->value("expressionFile",0);
            const char * ionFile=pForm->value("ionFile");
            sIonExpression ia(ionFile,sMex::fMapRemoveFile);
            ia.parseConventionalExpression(expressionFile,experiment);
    }
    else if(sIs(cmd,"-parseExpression")) {
        const char * cutSym=pForm->value("cutSym","-_");


        #define PARSECOL(_v_col, _v_default) const char * _v_col##S; idx _v_col##Hdr=false; \
            sVec < idx > _v_col##List ; \
            _v_col##S=pForm->value(#_v_col,_v_default ); \
            if(_v_col##S && *(_v_col##S)=='#') { \
                if(memcmp(_v_col##S+1,"hdr",3)==0)_v_col##Hdr=true; \
                sString::scanRangeSet((_v_col##S)+1+((_v_col##Hdr) ? 3 : 0 ),0,&(_v_col##List) ,0,0,0); \
            }else if(_v_col##S) \
                _v_col##List.vadd(1,sNotIdx);

        #define STARTLOOP(_v_col) for(idx ic=0; ic<tbl.cols() && ic<_v_col##List.dim(); ++ic) { \
                idx iC=_v_col##List[ic]; \
                if(iC>=tbl.cols()) break; \
                const char * hdr=0;\
                const char * cel=0;\
                const char * _v_col; \
                idx size##_v_col; \
                if(iC==sNotIdx ) { \
                    _v_col=_v_col##S; \
                    size##_v_col=sLen(_v_col##S); \
                } else { \
                    hdr=tbl.cell(-1,iC,&sizeHdr); \
                    cel=tbl.cell(ir,iC,&sizeCel);\
                    if(cutSym) { \
                        idx isym; \
                        for(isym=0; isym<sizeHdr && strchr(cutSym,hdr[isym])==0; ++isym); \
                        sizeHdr=isym; \
                    } \
                    if(_v_col##Hdr) { _v_col=hdr; size##_v_col=sizeHdr; val=cel; sizeVal=sizeCel;} \
                    else  { _v_col=cel; size##_v_col=sizeCel;} \
                } \


        #define ENDLOOP() }

        PARSECOL(id,0);
        PARSECOL(passage,"#hdr1-10000");
        PARSECOL(sample,"#0");
        PARSECOL(experiment,"experiment");

        const char * expressionFile=pForm->value("expressionFile",0);
        sFil exprFl(expressionFile,sMex::fReadonly);
        if(!exprFl.ok())
           return 0;

        sTxtTbl tbl;
        tbl.setBuf(exprFl.ptr(),exprFl.length());
        tbl.parse();

        const char * ionFile=pForm->value("ionFile");
        sIonExpression ia(ionFile);
        idx sizeHdr,sizeCel,sizeVal = 0, arr[100];


        for(idx ir=0; ir<tbl.rows(); ++ir) {
            idx rowIndexInIon=ia.addRecord(sIonExpression::eRow,sizeof(ir),&ir);

            const char * val="";

            STARTLOOP(id)

                if(!id || !hdr)continue;

                arr[0]=ia.addRecord(sIonExpression::eID,sizeid,id);
                arr[1]=ia.addRecord(sIonExpression::eType,sizeHdr,hdr);
                arr[2]=rowIndexInIon;
                ia.addRelationVarg(sIonExpression::eRelID,sNotIdx,arr,0);

            ENDLOOP()


            STARTLOOP(experiment)

                if(!experiment)continue;

                arr[0]=rowIndexInIon;
                arr[1]=ia.addRecord(sIonExpression::eExperiment,sizeexperiment,experiment);
                ia.addRelationVarg(sIonExpression::eRelExperiment,sNotIdx,arr,0);

            ENDLOOP()


            val="";
            STARTLOOP(sample)
                if(!sample)continue;

                STARTLOOP(passage)
                    if(!passage)continue;


                    arr[0]=rowIndexInIon;
                    arr[1]=ia.addRecord(sIonExpression::eSample,sizesample,sample);
                    arr[2]=ia.addRecord(sIonExpression::ePassage,sizepassage,passage);
                    arr[3]=ia.addRecord(sIonExpression::eValue,sizeVal,val);
                    arr[4]=ia.addRecord(sIonExpression::ePvalue,1,&sMex::_zero);
                    ia.addRelationVarg(sIonExpression::eRelVal,sNotIdx,arr,0);

                ENDLOOP()
            ENDLOOP()
        }

    }

    else if(sIs(cmd,"-parseExpressionOmnibus")) {

        const char * experiment=pForm->value("experiment","");



        const char * expressionFile=pForm->value("expressionFile",0);
        sFil exprFl(expressionFile,sMex::fReadonly);
        if(!exprFl.ok())
           return 0;

        sTxtTbl tbl;
        tbl.setBuf(exprFl.ptr(),exprFl.length());
        tbl.parse();

        const char * ionFile=pForm->value("ionFile");
        sIonExpression ia(ionFile,sMex::fMapRemoveFile);


        idx idColumns=9,sizeVal,sizeHdr,arr[100], is;


        for(idx ir=0; ir<tbl.rows(); ++ir) {
            idx rowIndexInIon=ia.addRecord(sIonExpression::eRow,sizeof(ir),&ir);

            for(idx ic=0; ic<tbl.cols(); ++ic) {
                const char * hdr=tbl.cell(-1,ic,&sizeHdr);
                const char * val=tbl.cell(ir,ic,&sizeVal);

                if(ic<idColumns) {
                    const char * type=hdr;
                    const char * id=val;
                    arr[0]=ia.addRecord(sIonExpression::eID,sizeVal,id);
                    arr[1]=ia.addRecord(sIonExpression::eType,sizeHdr,type);
                    arr[2]=rowIndexInIon;
                    ia.addRelationVarg(sIonExpression::eRelID,sNotIdx,arr,0);
                }
                else {
                    const char * sample=hdr+1;
                    const char * passage=0;
                    for(is=1; is<sizeHdr; ++is ) {
                        if(hdr[is]=='_') {
                            if(passage==0)
                                passage=hdr+is+1;
                            else break;
                        }
                    }
                    idx sizeSample=passage-sample-1;
                    idx sizePassage=hdr+is-passage;

                    arr[0]=rowIndexInIon;
                    arr[1]=ia.addRecord(sIonExpression::eSample,sizeSample,sample);
                    arr[2]=ia.addRecord(sIonExpression::ePassage,sizePassage,passage);
                    arr[3]=ia.addRecord(sIonExpression::eValue,sizeVal,val);
                    arr[4]=ia.addRecord(sIonExpression::ePvalue,1,&sMex::_zero);
                    ia.addRelationVarg(sIonExpression::eRelVal,sNotIdx,arr,0);

                }

            }

            arr[0]=rowIndexInIon;
            arr[1]=ia.addRecord(sIonExpression::eExperiment,sLen(experiment),experiment);
            ia.addRelationVarg(sIonExpression::eRelExperiment,sNotIdx,arr,0);

        }

    }

    return 0;
}
