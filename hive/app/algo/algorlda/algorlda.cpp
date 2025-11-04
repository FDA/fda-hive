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
#include <slib/core/str.hpp>
#include <slib/utils/txt.hpp>
#include <slib/utils/sort.hpp>
#include <slib/std/app.hpp>
#include <ssci/math/objects/matrix.hpp>
#include <ssci/math/clust/clust.hpp>
#include <ulib/usr.hpp>
#include <ulib/ufile.hpp>
#include <qlib/QPrideProc.hpp>
#include <ssci/math/rand/rand.hpp>
#include <ssci/math/stat/stat.hpp>

#include <ssci/math/func/func.hpp>

using namespace slib;

class FileWithFallback {
protected:
    const char * _buf;
    idx _len;
    sFil _fil;
public:
    FileWithFallback(sUsr * usr, const char * fileIdStr, const char * fallbackBuf=0, idx fallbackLen=0) {
        sHiveId fileId(fileIdStr);
        sUsrFile o(fileId, usr);
        if (o.Id()) {
            _buf = 0;
            _len = 0;
            sStr path;
            o.getFile(path);
            _fil.init(path.ptr(), sMex::fReadonly);
        } else {
            _buf = fallbackBuf;
            _len = fallbackLen ? fallbackLen : sLen(_buf);
        }
    }
    const char * ptr(idx offset=0) {return _buf ? _buf + offset : _fil.ptr(offset);}
    idx length() {return _buf ? _len : _fil.length();}
};


class AlgRLDA:public sQPrideProc
{
    public:
        AlgRLDA(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
        bool getRowsByClassifiers(sDic< sDic < sVec <idx> > > & grpset, sDic< sDic < sVec <idx> > > & checkset, sStr & grp_composite_name, sDic< sVec < idx > > *& grpclass, sStr & check_composite_name, sDic< sVec < idx > > *& checkclass);
        sDic< sVec <idx> > composite_grps, composite_checks;
};

idx lindex2(idx k, idx n)
{

    k-=n;
    idx i = n - 2 - floor(sqrt(-8*k + 4*n*(n-1)-7)/2.0 - 0.5);
    idx j = k + i + 1 - n*(n-1)/2 + (n-i)*((n-i)-1)/2;
    ++i;
    return ( i | ( j <<32) );

}

real myQuadraticFunction(sMatrix * orimat , idx irow, idx icol,sRlda::FDAStruc * pF, void * vars, sStr * out )
{
    idx ij=lindex2(icol, orimat->cols());
    idx i=ij&0xFFFFFFFF;
    idx j=ij>>32;

    if(out) { if(i!=j)out->printf("x%" DEC "*x%" DEC "",i,j); else out->printf("x%" DEC "^2",i); return 0;}
    return orimat->val(irow,i)*orimat->val(irow,j);
}


real myEnzymaticFunction(sMatrix * orimat , idx irow, idx icol,sRlda::FDAStruc * pF, void * vars, sStr * out )
{
    idx i=icol/pF->universe;
    idx p=icol%pF->universe;

    if(out) { out->printf("x%" DEC "/(x%" DEC "+%" DEC ")",i,i,p); return 0;}

    real v=orimat->val(irow,i);

    return v ? v/(v+p) : 0;
}

sRlda::FDAStruc funcSet[]={
    {
        "linear",
        1,-1,
        0,0,
        0,
        0,
        0,0,0
    },{
        "quadratic",
        1,-2.5,
        myQuadraticFunction,0,
        0,
        0,
        0,0,0
    }, {
        "enzymatic",
        10,-1,
        myEnzymaticFunction,0,
        0,
        0,
        0,0,0
    }
};

bool AlgRLDA::getRowsByClassifiers(sDic< sDic < sVec <idx> > > & grpset, sDic< sDic < sVec <idx> > > & checkset, sStr & grp_composite_name, sDic< sVec < idx > > *& grpclass, sStr & check_composite_name, sDic< sVec < idx > > *& checkclass) {
    sVec<idx> classifierIndices;
    formIValues("classifier", &classifierIndices);
    if( classifierIndices.dim() <= 0 || classifierIndices.dim() > grpset.dim() ) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "Invalid number of classifier");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return false;
    }
    composite_grps.empty();
    composite_checks.empty();
    grpclass = &composite_grps;
    checkclass = &composite_checks;
    for(idx i = 0; i < classifierIndices.dim(); ++i) {
        idx classifierIndex = classifierIndices[i];
        if( classifierIndex < 0 || classifierIndex >= grpset.dim() || (checkset.dim() && classifierIndex >= checkset.dim()) ) {
            reqSetInfo(reqId, eQPInfoLevel_Error, "Invalid classifier column; check column number and categorization table");
            reqSetStatus(reqId, eQPReqStatus_ProgError);
            return false;
        }
        if( classifierIndices.dim() == 1) {
            grpclass = grpset.ptr(classifierIndex);
            checkclass = checkset.ptr(classifierIndex);
            return true;
        }
    }
    sVec<idx> indices(sMex::fSetZero); indices.add(classifierIndices.dim());
    sDic<idx> union_rows;
    sStr composite_group_name;
    idx id_len;
    bool traversed_all_combinations = false;
    while ( !traversed_all_combinations ) {
        union_rows.empty();
        composite_group_name.cut0cut();
        for(idx i = 0 ; i < classifierIndices.dim() ; ++i ) {
            idx classifierIndex = classifierIndices[i];
            sDic< sVec< idx> > * cur_grp = grpset.ptr(classifierIndex);
            sVec< idx> * cur_rows= cur_grp->ptr(indices[i]);
            for(idx j = 0; j < cur_rows->dim(); ++j) {
                idx * cur_row_cnt = union_rows.get(cur_rows->ptr(j), sizeof(idx));
                if( !cur_row_cnt ) {
                    *union_rows.set(cur_rows->ptr(j), sizeof(idx)) = 1;
                } else {
                    (*cur_row_cnt)++;
                }
            }
            const char * cur_id = static_cast<const char *>(cur_grp->id(indices[i],&id_len));
            composite_group_name.add("|",1);
            composite_group_name.add(cur_id,id_len);
        }
        idx i = 0;
        while( i < indices.dim() ) {
            sDic< sVec< idx> > * cur_grp = grpset.ptr(classifierIndices[i]);
            if( indices[i]+1 < cur_grp->dim() ) {
                indices[i]++;
                break;
            } else if ( i+1 < indices.dim() ){
                indices[i]=0;
            } else {
                traversed_all_combinations = true;
                break;
            }
            ++i;
        }
        composite_group_name.add0(2);
        for(idx i = 0; i < union_rows.dim(); ++i) {
            if(*union_rows.ptr(i)==classifierIndices.dim()) {
                composite_grps.set(composite_group_name.ptr(1))->vadd(1,*(idx*)union_rows.id(i));
            }
        }
    }

    return true;
}

idx AlgRLDA::OnExecute(idx req)
{
    idx len;
    const char * p;
    sStr d;
    objs[0].propGet00("funcType", &d);
    sRlda Rlda;

    Rlda.callbackProgress=reqProgressStatic;
    Rlda.callbackParam=(void*)this;

    idx funcCollection=0;
    for(const char * p=d; p; p=sString::next00(p)) {
        for(idx i=0; i<sDim(funcSet); ++i) {
            if(strcmp(funcSet[i].name,p)==0) {
                *(Rlda.FDAfuncList.set(p))=funcSet[i];
                break;
            }
        }
        ++funcCollection;
    }
    if(funcCollection==1 && strcmp(funcSet[0].name,"linear")==0)
        funcCollection=0;


    sStr buf01,buf02, buf03;
    const char * filterRows=formValues00("filterRows",&buf01);
    const char * includeCats=formValues00("includeCats",&buf03);
    const char * filterColumns=formValues00("filterColumns",&buf02);
    idx dataMode=formIValue("dataMode",0);
    real binThreshold=formRValue("binThreshold",1.);
    idx nonZeroMin=formIValue("nonZeroMin",0);
    idx zeroMax=formIValue("zeroMax",-1);
    bool isTransposed=formBoolValue("matrixTransposed");
    bool readNumAsNum=formBoolValue("readNumsAsNums");
    bool minimalOutput=formBoolValue("minimalOut",0);
    const char * sMissingVal=formValue("missing");
    bool aveMissingVal=(sMissingVal && strcmp(sMissingVal,"ave")==0) ? true : false ;
    real rMissingVal=formRValue("missing",0);






    logOut(eQPLogType_Info,"Loading the matrix");
    sDic <idx> OriColIDs, OriRowIDs, * rowIDs=&OriRowIDs, * colIDs=&OriColIDs, SubColIDs ;
    sDic < idx > CheckColIDs,CheckRowIDs;
    sStr flbuf;
    sVec < sMex::Pos> colIds, rowIds;
    sMatrix OriMat, * Mat=&OriMat;
    sMatrix CheckMat;
    {
        sVec < real > mataves;
        sVec < idx > matTmp;


        FileWithFallback srcMat(user, formValue("matrixFile"), formValue("matrix"));


        FileWithFallback checkMat(user, formValue("checkMatrixFile"), formValue("checkMatrix"));
        CheckMat.parseCsv(&flbuf, checkMat.ptr(), checkMat.length(),
                    &colIds, &rowIds, 0,
                    dataMode,
                    binThreshold,
                    readNumAsNum,
                    isTransposed,
                    true,
                    filterRows,
                    filterColumns,
                    nonZeroMin,zeroMax,true,aveMissingVal ? REAL_MAX : rMissingVal);

        if(aveMissingVal && CheckMat.rows() ){
            mataves.resize(CheckMat.cols()+1);
            matTmp.resize(CheckMat.cols()+1);

            CheckMat.computeRowStat(mataves.ptr(),0,0,0, matTmp.ptr());
        }

        for ( idx i=1; i<rowIds.dim(); ++i ){
            const char * ppp=flbuf.ptr(rowIds[i].pos);
            *CheckRowIDs.set(ppp,rowIds[i].size)=i;
        }

        for ( idx i=1; i<colIds.dim(); ++i ){
            const char * ppp=flbuf.ptr(colIds[i].pos);
            *CheckColIDs.set(ppp,colIds[i].size)=i;
        }

        flbuf.cut(0);
        colIds.cut(0);rowIds.cut(0);

        Mat->parseCsv(&flbuf, srcMat.ptr(), srcMat.length(),
            &colIds, &rowIds, 0,
            dataMode,
            binThreshold,
            readNumAsNum,
            isTransposed,
            true,
            filterRows,
            filterColumns,
            nonZeroMin, zeroMax, aveMissingVal ? REAL_MAX : rMissingVal);
        if( false) {
            reqSetInfo(req, eQPInfoLevel_Error, "Matrix of measurables is empty or not in CSV format");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

        if(aveMissingVal ){
            mataves.resize(Mat->cols()+1);
            matTmp.resize(Mat->cols()+1);

            Mat->computeRowStat(mataves.ptr(),0,0,0, matTmp.ptr());
        }

        sStr tt;

        for ( idx i=1; i<rowIds.dim(); ++i ){
            const char * ppp=flbuf.ptr(rowIds[i].pos);
            idx * d=rowIDs->get(ppp,rowIds[i].size);
            if(d) {
                reqSetInfo(req,eQPInfoLevel_Warning, "Duplicate row %.*s identifiers found in matrix rows %" DEC " and %" DEC " ",(int)rowIds[i].size,ppp,*d,i);
                tt.printf(0,"%.*s ;dup%" DEC ,(int)rowIds[i].size,ppp,i);
                *rowIDs->set(tt.ptr(0), tt.length()) = i - 1;
            }
            else
                *rowIDs->set(ppp, rowIds[i].size) = i - 1;
        }

        tt.cut(0);
        for ( idx i=1; i<colIds.dim(); ++i ){
            const char * ppp=flbuf.ptr(colIds[i].pos);
            idx * d=colIDs->get(ppp,colIds[i].size);
            idx sz=colIds[i].size;
            if(d) {
                reqSetInfo(req,eQPInfoLevel_Warning, "Duplicate column %.*s identifiers found in matrix rows %" DEC " and %" DEC " ",(int)colIds[i].size,ppp,*d,i);
                tt.printf(0,"%.*s ;dup%" DEC,(int)colIds[i].size, ppp, i);
                ppp=tt.ptr(0);
                sz=tt.length();
            }
            *colIDs->set(ppp, sz) = i - 1;
        }



    }

    idx cntSample=formIValue("cntSample",10);
    idx iterMax=formIValue("iterMax",2*cntSample*Mat->cols());
    Rlda.bootStrapCounter=formIValue("bootStrapCounter",1000);
    Rlda.bootStrapFraction=formRValue("bootStrapFraction",0.9);
    Rlda.PCAMode=formBoolValue("PCAMode",false);
    Rlda.scaleMode = formIValue("scaleMode",2);

    const char * ssMax= 0, * ssMin= 0;
    if( formBoolIValue("continous_cls",0) ) {
        ssMax= formValue("continousPhenotypeMax",0);
        ssMin= formValue("continousPhenotypeMin",0);
    }
    real continousPhenotypeMax=ssMax ? (strcmp(ssMax,"auto")==0 ? -REAL_MAX : (strcmp(ssMax,"order")==0 ? -sIdxMax :  formRValue("continousPhenotypeMax",0))) : 0;
    real continousPhenotypeMin=ssMin ? (strcmp(ssMin,"auto")==0 ?  REAL_MAX : (strcmp(ssMin,"order")==0 ? sIdxMax  : formRValue("continousPhenotypeMin",0)) ) : 0;

    logOut(eQPLogType_Info,"Load categorization schema");
    sDic < sDic < sVec <idx> >  > grpset;
    sDic < sDic < sVec <idx> >  > checkset;
    sDic< sVec <idx> > emptyset;
    {
        sStr catbuf;
        FileWithFallback srcCat(user, formValue("categsFile"), formValue("categs"));
        sText::categoryListParseCsv(&catbuf, srcCat.ptr(), srcCat.length(), &grpset , rowIDs,0, false, true, includeCats );
        if( !grpset.dim()) {
            reqSetInfo(req, eQPInfoLevel_Error, "Categorization table is empty or not in CSV format");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

        idx grpset_min_cats = -1, grpset_max_cats = 0;
        for( idx i=0; i<grpset.dim(); i++ ) {
            idx ncats = grpset.ptr(i)->dim();
            grpset_max_cats = sMax<idx>(grpset_max_cats, ncats);
            grpset_min_cats = grpset_min_cats < 0 ? ncats : sMin<idx>(grpset_min_cats, ncats);
        }

        catbuf.cut(0);
        FileWithFallback checkCat(user, formValue("checkCategsFile"), formValue("checkCategs"));
        sText::categoryListParseCsv(&catbuf, checkCat.ptr(), checkCat.length(), &checkset , &CheckRowIDs,0, false, true);

    }





    idx classifierIndex=formIValue("classifier",0);
    sDic < sVec <idx> > * colset=grpset.ptr(classifierIndex);
    sDic < sVec <idx> > * check_colset=checkset.ptr(classifierIndex);


    sDic < sVec < idx > > continuousDic;


    if(continousPhenotypeMax!=continousPhenotypeMin) {
        idx contDim=0;
        bool autoMax=(continousPhenotypeMax==-REAL_MAX) ? true : false ;
        bool autoMin=(continousPhenotypeMin==REAL_MAX) ? true : false ;

        continuousDic.set("__up");
        continuousDic.set("__down");

        for (idx ir=0;ir<colset->dim(); ++ir) {
            contDim+=colset->ptr(ir)->dim();
        }
        sVec < idx > * vec1=continuousDic.ptr(0);vec1->add(contDim);
        sVec < idx > * vec2=continuousDic.ptr(1);vec2->add(contDim);
        Rlda.contRealVals.resize(contDim);

        for (idx ir=0,len;ir<colset->dim(); ++ir) {
            sVec < idx > * src=colset->ptr(ir);
            const char * p =(const char*)colset->id(ir,&len);
            real v; sRScanf(v,p,len,10);
            for (idx ic=0;ic<src->dim(); ++ic ) {
                idx iidx=*src->ptr(ic);
                Rlda.contRealVals[iidx]=v;
                *vec1->ptr(iidx)=iidx;
            }
            if(autoMin && (continousPhenotypeMin>v))
                continousPhenotypeMin=v;
            if(autoMax && (continousPhenotypeMax<v))
                continousPhenotypeMax=v;

        }
        sSort::sort(contDim,Rlda.contRealVals.ptr(0),vec1->ptr(0));
        for(idx ic=0; ic<contDim; ++ic) {
            *vec2->ptr(ic)=*vec1->ptr(contDim-1-ic);
        }
        colset=&continuousDic;
    }


    logOut(eQPLogType_Info,"Outputing sources \n");
    sStr dstFilePath;
    sMatrix::MatrixDicHeaders hdrs;

    if(!minimalOutput){
        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "Source.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Parsed source matrix to %s\n", dstFilePath.ptr());
            hdrs.cols=colIDs;hdrs.rows=rowIDs;
            sFil ot(dstFilePath);
            Mat->out(&ot, &hdrs, false, true, "%.3lg",0,0, 0, 0, 0);
        }
        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "SourceTransposed.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Parsed source matrix transpose to %s\n", dstFilePath.ptr());
            hdrs.cols=colIDs;hdrs.rows=rowIDs;
            sFil ot(dstFilePath);
            Mat->out(&ot, &hdrs, true, true, "%.3lg",0,0, 0, 0, 0);
        }

        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "PreCorrelation.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Output correlation matrix to %s\n", dstFilePath.ptr());

            sFil ot(dstFilePath);


            idx maxCols=Mat->cols()>1500 ? 1500 : Mat->cols() ;
            maxCols=Mat->cols();
            ot.addString("/,");
            sString::escapeForCSV(ot, (const char*)grpset.id(classifierIndex));
            for(idx ic=0;ic<maxCols; ++ic) {
                p=(const char*)colIDs->id(ic,&len);
                ot.addString(",");
                if( len ) {
                    sString::escapeForCSV(ot, p, len);
                }
            }
            ot.printf("\n");

            for(idx it=0;it<colset->dim(); ++it) {
                idx clen;
                const char * cat=(const char*)colset->id(it,&clen);
                idx dd=colset->ptr(it)->dim();
                if(continuousDic.dim()){
                    dd= (it==0 ? dd/2 : (dd-dd/2) );
                }
                for(idx ir=0;ir<colset->ptr(it)->dim(); ++ir) {
                    idx iR=*colset->ptr(it)->ptr(ir);
                    p=(const char*)rowIDs->id(iR,&len);
                    if( len ) {
                        sString::escapeForCSV(ot, p, len);
                    }
                    ot.addString(",");
                    if( clen ) {
                        sString::escapeForCSV(ot, cat, clen);
                    }
                    for(idx ic=0;ic<maxCols; ++ic) {
                        ot.printf(",%.3lg",Mat->val(iR,ic));
                    }
                    ot.printf("\n");
                }
            }

        }
    }

    logOut(eQPLogType_Info,"Preconfiguring kernel functions\n");

    logOut(eQPLogType_Info,"Computing RLDA\n");
    idx MacroMicroManagement=formRValue("MacroMicroManagement",1);
    real shannonThreshold=formRValue("shannonThreshold",0.001);
    const char * path=0, * report = 0, * repold = 0;
    sStr p0,p1, p2;


    sVec < idx > reqs;grp2Req(grpId, &reqs) ;
    idx topChoiceForFinal=formIValue("topChoiceForFinal",700);

    if(Mat->cols()>500 || funcCollection || (continousPhenotypeMax!=continousPhenotypeMin) ) {


        if(reqSliceCnt>1) {
            reqSetData(req,"file://samplingLdaTransforValCumulatorOrder",0,0);
            path=reqDataPath(req, "samplingLdaTransforValCumulatorORder", &p0);
            sFile::remove(path);
            Rlda.samplingLdaTransforValCumulatorOrder.init(path,sMex::fExactSize);

            p0.cut(0);
            reqSetData(req,"file://samplingLdaTransforOccurence",0,0);
            path=reqDataPath(req, "samplingLdaTransforOccurence", &p0);
            sFile::remove(path);
            Rlda.samplingLdaTransforOccurence.init(path,sMex::fExactSize);



            report=p1.printf(0,"%s.report1",path);p1.add0(1);
            repold=p1.printf("%s.report2",path);

        }


        Rlda.prepareComputeExtraLarge(*Mat,colset->dim());




        for( idx a=0; a<MacroMicroManagement; ++a) {

            if(reqSliceCnt>1) {

                if(sFile::exists(report)){
                    path=repold;
                    repold=report;
                    report=path;
                    sFile::remove(report);
                    path=p0.ptr(0);
                }

                sFile::copy(path,report);
                const char * slink=p2.printf(0,"%s.report",path);
                sFile::remove(slink);
                sFile::symlink(report,slink);
                sFile::remove(repold);
                logOut(eQPLogType_Info,"symlinking %s to %s and removing older %s\n",slink, report, repold);

                idx sz=Rlda.samplingLdaTransforOccurence.dim();
                sVec < idx > totalSamplingLdaTransforOccurence(sMex::fSetZero), curS; totalSamplingLdaTransforOccurence.resize(sz);
                idx * p, *s=totalSamplingLdaTransforOccurence.ptr(0);


                for(idx ir=0 ; ir<reqs.dim() ; ++ir) {


                    if(req==reqs[ir]){
                        p=Rlda.samplingLdaTransforOccurence.ptr(0);
                        logOut(eQPLogType_Info,"incorporating %" DEC " self\n",reqs[ir]);

                    } else {

                        p2.cut(0);reqDataPath(reqs[ir], "samplingLdaTransforOccurence", &p2);
                        if(!p2.length())
                            continue;
                        p2.add(".report");
                        logOut(eQPLogType_Info,"incorporating %" DEC " %s\n",reqs[ir],p2.ptr(0));
                        curS.init(p2,sMex::fReadonly);
                        if(!curS.dim())
                            continue;
                        p=curS.ptr();
                    }


                    for( idx idi=0; idi<sz; ++idi) {
                        s[idi]+=p[idi];
                    }
                    if(curS.ok()) {
                        curS.destroy();
                    }
                }

                real sumPLogP=0,totSamp=s[sz-1];
                for( idx idi =0; idi <sz-2 ; ++idi ) {
                    idx occ=s[idi];if(!occ)continue;
                    real p=occ/totSamp;
                    sumPLogP+=-p*(::log(p));
                }

                real curShannon=sumPLogP/::log(Rlda.samplingSetDimAvailable );
                logOut(eQPLogType_Info,"Local shannon vs global Shannon entropy %lf/%lf\n",Rlda.curShannon,curShannon);


            }




            printf("MacroIteration #%" DEC "\n",a);
            idx res=Rlda.computeExtraLarge(*Mat,  colset,
                cntSample,
                reqSliceCnt > 1 ? 0 : topChoiceForFinal,
                iterMax/MacroMicroManagement,
                formIValue("iterMin",100),
                shannonThreshold,
                formIValue("goodShannonMaxThreshold",5),
                formBoolValue("useEValScaling"),
                0.,
                0.,
                0,
                continousPhenotypeMin,
                continousPhenotypeMax
                );


            if(res)
                break;
        }


        Mat=&Rlda.SubMatrix;
        SubColIDs.empty();
        sStr d;
        for( idx it=0; it<Rlda.samplingSet.dim(); ++it) {
            const char * dd=0;
            d.cut(0);
            idx icol=Rlda.samplingSet[it];

            if( Rlda.cntFDAList) {

                idx icls;for( icls=1; icls<Rlda.cntFDAList && icol>=Rlda.pFDAfuncList[icls].universeBase ; ++icls ) {}--icls;
                sRlda::FDAStruc * pF=Rlda.pFDAfuncList+ icls;
                if(pF->funcCall){
                    pF->funcCall(&OriMat, sNotIdx, icol-Rlda.pFDAfuncList[icls].universeBase,pF, 0, &d);
                    dd =d.ptr(0);
                }

            }
            idx len;
            if(!dd ) {
                dd =(const char * )OriColIDs.id(Rlda.samplingSet[it],&len);
            }

            *SubColIDs.set(dd, len)=icol;
        }
        colIDs=&SubColIDs;
    }
    else {

        Rlda.compute(*Mat, *colset);
    }


    if(reqSliceCnt>1 && !isLastInMasterGroup()) {
        reqSetStatus(req, eQPReqStatus_Done);
        reqProgress(0, 100, 100);
        return 0;
    }

    if(reqSliceCnt>1) {
        real * p, *s=Rlda.samplingLdaTransforValCumulatorOrder.ptr(0);
        idx sz=Rlda.samplingLdaTransforValCumulatorOrder.dim();
        sVec < real > curS;
        for(idx ir=0 ; ir<reqs.dim() ; ++ir) {

            if(req==reqs[ir]){
                logOut(eQPLogType_Info,"including sampling of %" DEC " self\n",reqs[ir]);
                continue;
            }
            p2.cut(0);reqDataPath(reqs[ir], "samplingLdaTransforValCumulatorOrder", &p2);
            if(!p2.length())
                continue;

            logOut(eQPLogType_Info,"including sampling of %" DEC " %s\n",reqs[ir],p2.ptr(0));
            curS.init(p2,sMex::fReadonly);
            if(!curS.dim())
                continue;
            p=curS.ptr();

            for( idx idi=0; idi<sz; ++idi) {
                s[idi]+=p[idi];
            }
            if(curS.ok()) {
                curS.destroy();
            }

        }


        printf("Final Iteration\n");
        Rlda.finalCompute(*Mat,  colset,cntSample,topChoiceForFinal);
    }

    if(continuousDic.dim()){
        idx first=colset->ptr(0)->dim()/2;
        for(idx ic=0; ic<colset->dim(); ++ic){
            colset->ptr(ic)->cut(ic==0 ? first : colset->ptr(ic)->dim()-first);
        }
    }

    logOut(eQPLogType_Info,"Outputing results \n");

    dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "Vectors.csv");sFile::remove(dstFilePath);
    {
        logOut(eQPLogType_Info,"Eigen-vectors to %s\n", dstFilePath.ptr());
        hdrs.cols=0;hdrs.rows=colIDs;
        sFil ot(dstFilePath);
        Rlda.ldaTransformVecs.out(&ot, &hdrs, false, true);
    }

    dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "Values.csv");sFile::remove(dstFilePath);
    {
        logOut(eQPLogType_Info,"Eigen-values to %s\n", dstFilePath.ptr());
        hdrs.cols=0;hdrs.rows=colIDs;
        sFil ot(dstFilePath);
        ot.printf("Column,Eigenvalue\n");
        for(idx i=0; i<Mat->cols(); ++i ) {
            ot.printf("vector-%" DEC ",%.3lg\n",i+1,Rlda.ldaTransformVals[i]);
        }
    }

    dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "Sampling.csv");sFile::remove(dstFilePath);
    {
        logOut(eQPLogType_Info,"Sampling %s\n", dstFilePath.ptr());
        sFil ot(dstFilePath);
        ot.printf("Gene,Num,Sampling,Order\n");

        for(idx i=0; i<Rlda.samplingSetSize; ++i ) {
            const char * ppp=flbuf.ptr(colIds[i+1].pos);
            colIDs->get(ppp,colIds[i+1].size);
            idx sz=colIds[i+1].size;
            ot.printf("%" DEC " ,%" DEC ",%.*s,%lf\n",i+1,Rlda.samplingLdaTransforOccurence[i],(int)sz,ppp,Rlda.samplingLdaTransforValCumulatorOrder[i]);
        }
    }




    sMatrix translatedCoordinates, check_translatedCoordinates;
    translatedCoordinates.multiplyMatrixes(*Mat,Rlda.ldaTransformVecs);

    bool computeStat = formBoolValue("computeStat", true);
    real *pVals = 0, *tVals = 0, *pValsClass = 0, *tValsClass = 0;
    sVec<real> testV(sMex::fExactSize);
    sVec<sStat::tagSta> stats(sMex::fExactSize);
    const idx pairwiseStatDim = (colset->dim() * (colset->dim() - 1)) / 2;
    if( computeStat ) {
        pVals = testV.add((OriMat.cols() + translatedCoordinates.cols()) * pairwiseStatDim * 2);
        tVals = pVals + OriMat.cols() * pairwiseStatDim;
        stats.resize(colset->dim() * OriMat.cols());
        sStat::statTestCols(OriMat.ptr(0, 0), OriMat.rows(), OriMat.cols(), colset, pVals, tVals, true, stats);

        pValsClass = tVals + OriMat.cols() * pairwiseStatDim;
        tValsClass = pValsClass + translatedCoordinates.cols() * pairwiseStatDim;
        sStat::statTestCols(translatedCoordinates.ptr(0, 0), translatedCoordinates.rows(), translatedCoordinates.cols(), colset, pValsClass, tValsClass, true);
    }



    idx scaleVectors=formIValue("scaleVectorsByValue",0);

    {
        for( idx iprt=0; iprt<2; ++iprt) {
            bool printAllContributors = (iprt == 0) ? false : true;

            dstFilePath.cut(0);
            sQPrideProc::reqAddFile(dstFilePath, "ContributorGraph%s.csv", printAllContributors ? "All" : "");
            sFile::remove(dstFilePath);
            sFil ot(dstFilePath);
            dstFilePath.cut(0);
            sQPrideProc::reqAddFile(dstFilePath, "ExtractGenes%s.csv", printAllContributors ? "All" : "");
            sFile::remove(dstFilePath);
            sFil otr(dstFilePath);
            dstFilePath.cut(0);
            sQPrideProc::reqAddFile(dstFilePath, "GeneSet%s.rnk", printAllContributors ? "All" : "");
            sFile::remove(dstFilePath);
            sFil gset(dstFilePath);

            ot.printf("Column");
            otr.printf("Column");
            gset.printf("#genes\tcontributions\n");

            if (!printAllContributors){

                for (idx y = 0; y < colset->dim()-1; y++) {
                    ot.printf (",Contribution Coefficient %" DEC, y);
                    otr.printf (",Contribution Coefficient %" DEC, y);
                    if(computeStat) {
                        for(idx ipr=0; ipr<pairwiseStatDim; ++ipr ) {
                            ot.printf(" [%lg/%3lg]",1-pValsClass[y*pairwiseStatDim+ipr],tValsClass[y*pairwiseStatDim+ipr]);
                            otr.printf(" [%lg/%3lg]",1-pValsClass[y*pairwiseStatDim+ipr],tValsClass[y*pairwiseStatDim+ipr]);
                        }
                    }
                }
                ot.printf(",Cumulative Contribution Coefficient");
            }

            if(computeStat) {
                const char * pi;
                ot.printf(",PValue,Student T-Value\n");
                otr.printf(",PValue");
                for(idx ic = 0; ic < colset->dim(); ++ic) {
                    pi = (const char*) colset->id(ic, &len);
                    otr.printf(",\"cnt-%.*s\",\"ave-%.*s\",\"std-%.*s\"", (int) len, pi, (int) len, pi, (int) len, pi);
                }
                for(idx ic = 0; ic < colset->dim(); ++ic) {
                    sVec<idx> * pcol = colset->ptr(ic);
                    for(idx id = 0; id < pcol->dim(); ++id) {
                        const char * pi = (const char*) rowIDs->id(*pcol->ptr(id), &len);
                        otr.printf(",%" DEC "-", colset->ptr(ic)->dim());
                        sString::escapeForCSV(otr, pi, len);

                    }
                }
                otr.printf("\n");
            }

            sVec <real> firstCol;firstCol.add(Rlda.ldaTransformVecs.rows());
            for (idx x = 0; x < Rlda.ldaTransformVecs.rows(); x++) {
                firstCol[x]=Rlda.ldaTransformVecs[x][0];
            }

            sVec <idx> index;
            if(!printAllContributors) {
                index.add(firstCol.dim());
                sSort::sortabs(firstCol.dim(), firstCol.ptr(), index.ptr());
            }

            for (idx x =0; x < (printAllContributors ? colIds.dim()-1 : index.dim() ); ++x ) {
                idx curRow = printAllContributors ? x : index[index.dim()-1-x];

                const char * pi;
                idx  dex=0;
                if (!printAllContributors){
                    pi=(const char*)colIDs->id(curRow,&len);
                    dex=*colIDs->ptr(curRow);
                }
                else {
                    pi = flbuf.ptr(colIds[curRow+1].pos);
                    len = colIds[curRow+1].size;
                    dex = x;
                }

                if( len ) {
                    sString::escapeForCSV(ot, pi, len);
                    sString::escapeForCSV(otr, pi, len);
                    sStr t;
                    sString::searchAndReplaceSymbols(&t,pi,len," ;",0,0,true,true,true,true,true);
                    sString::escapeForCSV(gset, t.ptr(), 0);
                }
                if (!printAllContributors){
                    real ctot=0;
                    for (idx y = 0; y < colset->dim()-1; y++) {
                        real val=Rlda.ldaTransformVecs[curRow][y];
                        if( y && scaleVectors  )
                            val*=Rlda.ldaTransformVals[y]/Rlda.ldaTransformVals[0];
                        ot.printf (",%.4lg", val);
                        otr.printf (",%.4lg", val);
                        gset.printf ("\t%.4lg", val);
                        ctot+=val*val;
                    }
                    ot.printf(",%.3lg",sqrt(ctot));
                }
                if(computeStat) {
                    ot.printf(",");
                    otr.printf(",");
                    for(idx ipr=0; ipr<pairwiseStatDim; ++ipr ) {
                        if(ipr){
                            ot.printf("|");
                            otr.printf("|");
                        }
                        ot.printf("%lg",1-pVals[dex*pairwiseStatDim+ipr]);
                        otr.printf("%lg",1-pVals[dex*pairwiseStatDim+ipr]);
                    }
                    ot.printf(",");
                    for(idx ipr=0; ipr<pairwiseStatDim; ++ipr ) {
                        if(ipr)ot.printf("|");
                        ot.printf("%3lg",tVals[dex*pairwiseStatDim+ipr]);
                    }
                    for(idx ic = 0; ic < colset->dim(); ++ic) {
                        sStat::tagSta *st = stats.ptr(dex * colset->dim() + ic);
                        otr.printf(",%" DEC ",%.3lg,%.3lg",st->n, st->ave ,st->ss);
                    }

                }
                for(idx ic=0; ic<colset->dim();++ic ){
                    sVec <idx> * pcol=colset->ptr(ic);
                    for(idx id=0; id<pcol->dim();++id ){
                        idx sid=*pcol->ptr(id);
                        otr.printf(",%.3lg",OriMat.val(sid,dex));
                    }
                }
                ot.printf("\n");
                otr.printf("\n");
                gset.printf("\n");

            }
        }
    }




    sVec < real > TotStat;
    {
        TotStat.add(Mat->cols()*2);
        Mat->computeRowStat(TotStat.ptr(0),0,0);
        Mat->shiftRows(TotStat.ptr(0));
    }


    if(!minimalOutput){

        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "Translated.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Output translated matrix to %s\n", dstFilePath.ptr());

            hdrs.cols=0;
            hdrs.rows=rowIDs;
            sFil ot(dstFilePath);
            translatedCoordinates.out(&ot, &hdrs, false, true, "%lg",0,0, 0, 0, 0);
        }

        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "TranslatedTranspose.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Output translated matrix to %s\n", dstFilePath.ptr());

            hdrs.cols=0;
            hdrs.rows=rowIDs;
            sFil ot(dstFilePath);
            translatedCoordinates.out(&ot, &hdrs, true, true, "%lg",0,0, 0, 0, 0);
        }

        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "PostCorrelation.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Output correlation matrix to %s\n", dstFilePath.ptr());

            sFil ot(dstFilePath);

            idx maxCols=Mat->cols()>5 ? 5 : Mat->cols() ;
            ot.addString("/,");
            sString::escapeForCSV(ot, (const char*)grpset.id(classifierIndex));
            for(idx ic=0;ic<maxCols; ++ic) {
                ot.printf(",col_%" DEC,ic+1);
            }
            ot.printf("\n");

            for(idx it=0;it<colset->dim(); ++it) {
                idx clen;
                const char * cat=(const char*)colset->id(it,&clen);

                for(idx ir=0;ir<colset->ptr(it)->dim(); ++ir) {
                    idx iR=*colset->ptr(it)->ptr(ir);
                    const char * pi=(const char*)rowIDs->id(iR,&len);

                    if( len ) {
                        sString::escapeForCSV(ot, pi, len);
                    }
                    ot.addString(",");
                    if( clen ) {
                        sString::escapeForCSV(ot, cat, clen);
                    }
                    for(idx ic=0;ic<maxCols; ++ic) {
                        ot.printf(",%.3lg",translatedCoordinates.val(iR,ic));
                    }
                    ot.printf("\n");
                }
            }

        }
    }

    if(CheckMat.rows()) {

        Rlda.finalCompute(CheckMat,  colset,cntSample,topChoiceForFinal, true);
        sMatrix * cMat=&Rlda.SubMatrix;

        check_translatedCoordinates.multiplyMatrixes(*cMat,Rlda.ldaTransformVecs);

        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "CheckTranslated.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Output translated matrix to %s\n", dstFilePath.ptr());

            hdrs.cols=0;
            hdrs.rows=&CheckRowIDs;
            sFil ot(dstFilePath);
            check_translatedCoordinates.out(&ot, &hdrs, false, true, "%.3lg",0,0, 0, 0, 0);
        }


        dstFilePath.cut(0);sQPrideProc::reqAddFile(dstFilePath, "CheckPostCorrelation.csv");sFile::remove(dstFilePath);
        {
            logOut(eQPLogType_Info,"Output check correlation matrix to %s\n", dstFilePath.ptr());


            sFil ot(dstFilePath);
            idx maxCols=cMat->cols()>5 ? 5 : cMat->cols() ;
            ot.addString("/,");
            sString::escapeForCSV(ot, (const char*)checkset.id(classifierIndex));
            for(idx ic=0;ic<maxCols; ++ic) {
                ot.printf(",col_%" DEC,ic+1);
            }
            ot.printf("\n");


            for(idx it=0;it<check_colset->dim(); ++it) {
                idx clen;
                const char * cat=(const char*)check_colset->id(it,&clen);

                for(idx ir=0;ir<check_colset->ptr(it)->dim(); ++ir) {
                    idx iR=*check_colset->ptr(it)->ptr(ir);
                    const char * pi=(const char*)CheckRowIDs.id(iR,&len);

                    if( len ) {
                        sString::escapeForCSV(ot, pi, len);
                    }
                    ot.addString(",");
                    if( clen ) {
                        sString::escapeForCSV(ot, cat, clen);
                    }
                    for(idx ic=0;ic<maxCols; ++ic) {
                        ot.printf(",%.3lg",check_translatedCoordinates.val(iR,ic));
                    }
                    ot.printf("\n");
                }
            }

        }




    }



    reqSetStatus(req, eQPReqStatus_Done);
    reqProgress(0, 100, 100);

return 0;
}


int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc,argv);

    AlgRLDA backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"algorlda",argv[0]));
    return (int)backend.run(argc,argv);
}










