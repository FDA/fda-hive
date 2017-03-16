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
#include <ulib/ulib.hpp>
#include <violin/violin.hpp>
#include <ssci/math/stat/stat.hpp>


using namespace sviolin;

/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  HIVE ION GENERAL CLASS
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */


enum enumCommands{
    eWander,
    eWanderTbl,
    eLast
};

const char * listCommands=
    "wander" _
    "wantbl" _
    _;

idx sHiveIon::retrieveListOfObjects(sUsr * usr, sVec<sHiveId> * objList, sDir * fileList00, const char * objIDList, const char * typeList, const char * parList00, const char * valList00, const char * filePattern00)
{
    sVec<sHiveId> ol;
    if(!objList)objList=&ol;


    if( objList )
        sHiveId::parseRangeSet(*objList, objIDList);

    if(fileList00 ) {
        for ( idx i=0; i<objList->dim() ; ++i) {
            sUsrObj o(*usr, *objList->ptr(i));
            o.files(*fileList00, sFlag(sDir::bitFiles), filePattern00,"");
        }
    }

    // use SpecialObj::findTaxDb() to filter?
    return objList->dim();
}

/*
sDic < sDic < sVec < real > > > samplePassageReplicaDic;

sDic < idx >  rowIds;
sMex largeDicBuf;
sDic < sDic < sMex::Pos  > > valueDic;
//sString dicLabelSet00;
sStr compositePassage;
//idx limit=100;
bool collapsePassage=false;
*/

idx sHiveIon::realValStatCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist )
{
    if(!statement->label)
        return 1;

    sHiveIon::geneExpr * gE = (sHiveIon::geneExpr *) wander->callbackFuncParam;

    if( memcmp(statement->label,"dic",3)==0 || memcmp(statement->label,"Dic",3)==0 ) {
        idx recordID=*(idx*)reslist[1].body;
  //      if(recordID>limit && statement->label[0]=='D')
   //         return 1;
        //real r;sRScanf(r,reslist[4].body,reslist[4].size,10);
        char buf[1024];
        idx shift=0;

        gE->compositePassage.cut(0);
        // because we know the label for asking about the phenotype started with an 'd'
        if(statement->label[0]=='d') {
            //compositePassage.printf("%" DEC "_",statement->scope);
            gE->compositePassage.addString("a_",2); // 'a' for phenotype
            if (gE->collapsePassage){
                const char * specialSepar = strchr((const char *)reslist[3].body,'@');
                if (specialSepar) {
                    gE->compositePassage.addString((const char *)reslist[3].body,reslist[3].size-3); // something@P3
                }
                else gE->compositePassage.addString((const char *)reslist[3].body,reslist[3].size);
            }
            else gE->compositePassage.addString((const char *)reslist[3].body,reslist[3].size);
        } else {
            #ifdef WORKING_MODEL
            gE->compositePassage.printf("Expr%" DEC "-",recordID+1);
            #else
                //compositePassage.addString("\"",1);
            gE->compositePassage.addString("e_",2); // 'e' for genotype
                if(reslist[shift+3-6].size>1){
              /*      const char * specialSepar = strchr((const char *)reslist[shift+3-6].body,'@');
                    if (specialSepar) {
                        compositePassage.addString((const char *)reslist[shift+3-6].body,reslist[shift+3-6].size-3); // something@P3
                    }
                    else compositePassage.addString((const char *)reslist[shift+3-6].body,reslist[shift+3-6].size);*/
                    gE->compositePassage.addString((const char *)reslist[shift+3-6].body,reslist[shift+3-6].size);
                }
                else
                    gE->compositePassage.addString("missing",7);
            if (!gE->collapsePassage){
                gE->compositePassage.addString("@",1);
            #endif
                gE->compositePassage.addString((const char *)reslist[shift+3].body,reslist[shift+3].size);
            }
            #ifndef WORKING_MODEL
                //compositePassage.addString("\"",1);
            #endif
        }
        /* else {
            shift=-10;
            compositePassage.addString((const char *)reslist[1].body,reslist[1].size);
            compositePassage.addString("-",1);
            compositePassage.addString((const char *)reslist[shift+3].body,reslist[shift+3].size);
        }*/


        idx siz=reslist[shift+4].size; if(siz>(idx)sizeof(buf)-1)siz=sizeof(buf)-1;
        memcpy(buf,reslist[shift+4].body,siz);buf[siz]=0;

        real r;if(sscanf(buf,"%lg",&r)==1){
            gE->samplePassageReplicaDic.set(reslist[shift+2].body,reslist[shift+2].size)->set(gE->compositePassage.ptr(),gE->compositePassage.length())->vadd(1,  r );
            *gE->rowIds.set(gE->compositePassage.ptr(),gE->compositePassage.length())=1;
        } /* else {
            ::printf("########## EPRST %s ! \n",buf);
        }*/
    }
    return 1;
}

idx sHiveIon::anyValStatCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist )
{
    if(!statement->label)
        return 1;

    sHiveIon::geneExpr * gE = (sHiveIon::geneExpr *) wander->callbackFuncParam;

    if( memcmp(statement->label,"dic",3)==0 ) {
        sMex::Pos * pos= gE->valueDic.set(reslist[3].body,reslist[3].size)->set(reslist[2].body,reslist[2].size);
        pos->pos=gE->largeDicBuf.add(reslist[1].body,reslist[1].size);
        pos->size=reslist[1].size;
    }
    return 1;
}

enum eCharacteristicToPrint{eCharacteristicMean=0,eCharacteristicMax};


void sHiveIon::dicDicVecPrint(sIO * buf, sDic < sDic < sVec < real > > > & sPrDic, idx characteristics, idx printMeasurementAsHeader, idx doSort, void * params )
{
    idx sSize =0;
    idx idlen;

    sHiveIon::geneExpr * gE = (sHiveIon::geneExpr *) params;

    #define PREPAREHEADER(_v_dic) for (idx ir=0; ir<_v_dic.dim(); ++ir){ \
                const void * r=_v_dic.id(ir,&idlen); \
                buf->addString(",",1); \
                buf->addString((const char*)r,idlen);\
                }\
            buf->addString("\n",1);

    #define CHARACTERISTIC(_characteristics_) \
                    real val=0, stdev; \
                    switch (_characteristics_) { \
                        case eCharacteristicMean:  { \
                            val = sStat::mean(arrVal); \
                            stdev = sStat::stDev(arrVal);\
                        } break; \
                        default: \
                            break;  \
                    } \
                    if (gE->exportStDev){ \
                        buf->printf(",%.3lf +/- %.3lf",val, stdev);\
                    } \
                    else buf->printf(",%.3lf",val);

    sVec < idx > ind;
    if (printMeasurementAsHeader){
        // Preparing header
        // Sample,P3,P5,C3,C5.....
        buf->addString("Sample");
        PREPAREHEADER(gE->rowIds);
        if(doSort) {
            ind.resize(sPrDic.dim());
            sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sSort::sort_stringsDicID,&sPrDic,sPrDic.dim(), sPrDic.ptr(),ind.ptr());
        }
        // Printing the content
        // Sample1,valueOfP3...
        for (idx is =0; is < sPrDic.dim(); ++is){
            idx iS=ind.dim() ? ind[is] : is ;

            const char * sampleName = (const char *)sPrDic.id(iS,&sSize);
            buf->addString(sampleName,sSize);

            for (idx ir=0; ir<gE->rowIds.dim(); ++ir){
                const void * r=gE->rowIds.id(ir,&idlen);
                sVec < real > * arrVal = sPrDic.ptr(iS)->get(r,idlen);
                if(!arrVal){
                    buf->printf(",");
                    continue;
                }
                CHARACTERISTIC(characteristics);

            }
            buf->printf("\n");
        }
    }
    else {
        //Preparing the header
        // passageNumber,sample1,sample2,...\n
        if (gE->collapsePassage){
            buf->addString("DataType,Measurements");
        }
        else buf->addString("DataType,Measurements");//buf->addString("DataType,Measurements,Passage");
        //else buf->addString("DataType,Measurements");
        PREPAREHEADER(sPrDic);
        if(doSort) {
            ind.resize(gE->rowIds.dim());
            sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sSort::sort_stringsDicID,&gE->rowIds,gE->rowIds.dim(), gE->rowIds.ptr(),ind.ptr());
        }
        // Printing the content
        for (idx ir=0; ir<gE->rowIds.dim(); ++ir){
            idx iR=ind.dim() ? ind[ir] : ir ;
            const char * r=(const char*)gE->rowIds.id(iR,&idlen);
            const char * typeSep=strchr(r,'_');
            if (typeSep){
                buf->addString((const char*)r,typeSep-r);
                buf->addString(",",1);
            }
            const char * separated=strchr(r,'@');
            //const char * separated=strchr(r,'#');
            if(separated) {
                //buf->addString((const char*)r,separated-r);
                buf->addString(typeSep+1,separated-typeSep-1);
                if (!gE->collapsePassage){
                    //buf->add(",",1);
                    //buf->addString(separated+1,r+idlen-separated-1);
                    buf->addString("_",1);
                    buf->addString(separated+1,typeSep+idlen-separated-1-1);
                }
            }else{
                buf->addString((const char*)r,idlen);
                //buf->addString(typeSep+1,idlen-1-1);
                if (!gE->collapsePassage){
                    buf->add(",",1);
                }
            }


            for (idx is=0; is < sPrDic.dim(); ++is){
                sVec < real > * arrVal = sPrDic.ptr(is)->get(r,idlen);
                if(!arrVal || arrVal->dim()==0){
                    buf->printf(",");
                    continue;
                }
                CHARACTERISTIC(characteristics);
            }
            buf->printf("\n");
        }
    }
}


idx sHiveIon::Cmd(sIO * out, const char * cmd, sVar * pForm)
{

    geneExpr * cur_geneExp = new geneExpr();

    idx cmdnum=-1;
    if(cmd)
        sString::compareChoice( cmd, listCommands,&cmdnum,false, 0,true);
    if(cmdnum==-1){return 0;}// {sQPrideCGIProc::Cmd(cmd );}

    idx measurementAsHdr = pForm->ivalue("measurementAsHdr",0); // flag to print the measurements as header, instead of samples
    const char * postTreat=pForm->value("resTreat");
    cur_geneExp->collapsePassage = pForm->boolvalue("collapse_passage");

    // initialize the wander object which is running the statements
    sIonWander wander;
    wander.resetCompileBuf();
    wander.resetResultBuf();

    if(postTreat && !strcmp(postTreat,"rvs")){
        wander.callbackFunc=realValStatCallback;
        wander.callbackFuncParam = cur_geneExp;
    }
    else if(postTreat && !strcmp(postTreat,"dic")){
        wander.callbackFunc=anyValStatCallback;
        wander.callbackFuncParam = cur_geneExp;
    }
    else
        wander.pTraverseBuf=out; // this is where we output
        //wander.pTraverseBuf=static_cast < sIO * >(this); // this is where we output

    wander.setSepar(pForm->value("sepField"),pForm->value("sepRecord"));

    if (pForm->value("stdev",0)) {
        cur_geneExp->exportStDev = true;
    }

    // retrieve the query statements to work w`ith and compile those through wander objects
    //sFilePath path;
    sFil qryF;
    const char * query=pForm->value("qry");
    if(!query) {
        sDir qryList;
        retrieveListOfObjects(myUser, 0, &qryList, pForm->value("qryid") , pForm->value("qrytype"), pForm->value("qrypar"), pForm->value("qryval"), pForm->value("qryfile"));
        qryF.init(qryList.ptr(),sMex::fReadonly);
        query=qryF.ptr();
        //path.makeName(qryList.ptr(),"%%dir/%s",ionfile);
        //ionList00._entryBuf.printf("%s",path.ptr());ionList00._entryBuf.add0(1);
    }

    // retrieve the list of ions to work with and initialize those through wander objects

    sDir ionList00;
    const char * ionfile=pForm->value("ionfile","ion.ion");
    retrieveListOfObjects(myUser, 0, &ionList00, pForm->value("ionid") , pForm->value("iontype"), pForm->value("ionpar"), pForm->value("ionval"), ionfile);
    // get the list of files
    for( char * f=(char*)ionList00.ptr(); f && *f; f=sString::next00(f)) {
        char * k=strrchr(f,'.');
        if(k)*k=0;
        //if(strcmp(f,ionList00.ptr())==0)
        //    continue;
        wander.addIon(0)->ion->init(f,sMex::fReadonly);
        if(k)*k='.';
    }
    cur_geneExp->rowIds.empty();
   // samplePassageReplicaDic.empty();

    sIO errb;
    wander.traverseCompile(query, sLen(query), &errb, true);

    if(pForm->ivalue("debug"))
        wander.debug=1;

    idx cnt=pForm->ivalue("cnt",200);


    // commands
    switch(cmdnum) {
        case eWander:{ // execute actual traverse script and produce outcome
            //outHtml();
            wander.resetResultBuf();
            wander.retrieveParametricWander(pForm,0);
            wander.traverse();
        } break;
        case eWanderTbl:{
            sDir tblList00;
            retrieveListOfObjects(myUser, 0, &tblList00, pForm->value("tblid") , pForm->value("tbltype"), pForm->value("tblpar"), pForm->value("tblval"), pForm->value("tblfile","_.csv"));

            // retrieve the list of tables to work with and initialize those through wander objects
            #ifdef USETXTTBL
                sTxtTbl txttbl;
                txttbl.setFile(tblList00);
                txttbl.parse();
                wander.traverseTable(0,&txttbl);
            #else
                sTbl tbl;
                sFil tblSrc(tblList00.ptr(),sMex::fReadonly);
                tbl.parse(tblSrc.ptr(),tblSrc.length());
                wander.traverseTable(&tbl,0);
            #endif


        }break;
        default: {

        } break;
    }


    if(cur_geneExp->samplePassageReplicaDic.dim()) {
        idx doSort=pForm->ivalue("sort",0);
        dicDicVecPrint(out,cur_geneExp->samplePassageReplicaDic,eCharacteristicMean,measurementAsHdr,doSort,cur_geneExp);
    }


    if(postTreat && !strcmp(postTreat,"dic")){
        for(idx ic=0; ic<cur_geneExp->valueDic.ptr(0)->dim() ; ++ic ) {
            if(ic)out->add(",",1);
            idx len;
            const char * iiid=(const char * ) cur_geneExp->valueDic.ptr(0)->id(ic,&len);
            out->add(iiid,len);
        }
        out->printf("\n");
        for(idx ir=0; ir<cur_geneExp->valueDic.dim() ; ++ir ) {
            //out->printf("%s",(const char*) valueDic.id(ir));
            for(idx ic=0; ic<cur_geneExp->valueDic.ptr(ir)->dim() ; ++ic ) {
                sMex::Pos * pos=cur_geneExp->valueDic.ptr(ir)->ptr(ic);
                if(ic)out->add(",",1);
                out->add((const char * ) cur_geneExp->largeDicBuf.ptr(pos->pos),pos->size);
            }
            out->printf("\n");
            if(ir>=cnt) break;
        }
    }

    delete cur_geneExp;
    return 1;

}

