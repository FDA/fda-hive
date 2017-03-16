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

#include <math.h>
#include <ssci/math/objects/matrix.hpp>
#include <ssci/math/algebra/algebra.hpp>
#include <slib/std/string.hpp>
#include <slib/utils/sort.hpp>
#include <slib/utils/txt.hpp>

using namespace slib;

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ Matrix outpts
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

inline void titleOutput(sStr * out, const char * title,idx plen, bool forCSV)
{
    if( !title ) {
        title = "";
    }

    if( forCSV ) {
        sString::escapeForCSV(*out, title,plen);
    } else {
        out->printf("%.*s", (int)plen,title);
    }
}

void sMatrix::matOutput(sStr * out, void * opr, real * , idx row, idx col, bool forCSV)//param pVal
{
    MatrixDicHeaders * op= (MatrixDicHeaders * )opr;
    idx plen;
    const char * p;

    if(row==-1 && col==-1) out->printf("/");
    else if(col==-1){
        if( op && op->rowset)row=(*(op->rowset))[row];
        if( op && op->rows ) {
            p=(const char*)(op->rows->id(row,&plen));
            titleOutput(out, p, plen, forCSV);
        } else {
            out->printf("row_%"DEC"", row+1 );
        }
    }
    else if(row==-1){
        if( op && op->colset)col=(*(op->colset))[col];
        if( op && op->cols ) {
            p=(const char*)(op->cols->id(col,&plen));
            titleOutput(out, (const char*)(op->cols->id(col)), plen, forCSV);
        } else {
            out->printf("col_%"DEC"", col+1);
        }
    }

    return ;
}
/*

void sMatrix::matOutput(sStr * out, void * , real * , idx row, idx col)//param pVal
{
    if(row==-1 && col==-1) out->printf("/");
    else if(col==-1){out->printf("row_%"DEC"",row);}
    else if(row==-1){out->printf("col_%"DEC"",col);}
    //else out->printf("%lf",*pVal);
    return ;
}
*/

void sMatrix::out(sStr * ot, void * param, bool transpose, bool header, const char * fmt, MatrixOutput ho, idx colstart, idx colend, idx rowstart, idx rowend) const
{
    if(!colend || colend>cols()) colend=cols();
    if(!rowend || rowend>rows()) rowend=rows();
    if(colstart<0) colstart=0;
    if(rowstart<0) rowstart=0;


    if(header && !ho) ho=(MatrixOutput)matOutput;

    if(!transpose) {
        if(header) {
            ho(ot, param, 0, -1, -1, true);
            for( idx ic=colstart; ic<colend; ++ic) {
                ot->printf(",");
                ho(ot, param, 0, -1, ic, true);
            }
            ot->printf("\n");
        }

        for(idx ir=rowstart; ir< rowend; ++ir ) {
            if(header) ho(ot, param, 0, ir, -1, true);

            for(idx ic=colstart; ic< colend; ++ic ) {
                if(ic || header)ot->printf(",");
                //ho(ot,param,ptr(ir,ic),ir,ic);
                ot->printf(fmt,val(ir,ic));
            }
            ot->printf("\n");
        }
    }else {
        if(header) {
            ho(ot, param, 0, -1, -1, true);
            for( idx ir=rowstart; ir<rowend; ++ir) {
                ot->printf(",");
                ho(ot, param, 0, ir, -1, true);
            }
            ot->printf("\n");
        }

        for(idx ic=colstart; ic< colend; ++ic ) {
            if(header)ho(ot, param, 0, -1, ic, true);

            for( idx ir=rowstart; ir<rowend; ++ir){
                if(ic || header)ot->printf(",");
                //ho(ot,param,ptr(ir,ic),ir,ic);
                ot->printf(fmt,val(ir,ic));
            }
            ot->printf("\n");
        }
    }
    // output the transpose of activity matrix
}
void sMatrix::out(const char * flnm, void * param, bool transpose, bool header, const char * fmt, MatrixOutput ho, idx colstart, idx colend, idx rowstart, idx rowend) const
{
    sFil fl(flnm);fl.cut(0);
    out(&fl,param, transpose, header, fmt, ho, colstart, colend,rowstart, rowend);
}



void sMatrix::outSingleEvecSrt(sStr * out, real * evals, idx col, sDic < idx > * ids, sStr * outshort ) const// cntpeaks
{
    idx cls=cols();
    sVec < idx > ind;ind.resize(cls);
    sortOrderByCol(ind, col);

    real allEval=0;for( idx ic=0; ic<cls; ++ic)allEval+=evals[ic];

    real prctl=100.*evals[col]/allEval;
    out->printf("%"DEC"-vec,%lf,(%.2lf),",col+1,evals[col],prctl);
    if(outshort)
        outshort->cut(0);
    //if(prctl<1)topPeaks=0;
    real sum=0,sum2=0;
    for( idx ir=0,ip=0; ir<cls; ++ir) {
        real vv=val(ind[ir],col);
        sum+=vv;sum2+=vv*vv;
        if(vv*vv<0.01)continue;
        //real * pmass=(real *)peaks.id(ind[ir]);
        const char * pid=ids ? (const char * ) ids->id(ind[ir]) : 0;
        if(ip) {
            out->printf(" + ");
            if(outshort)outshort->printf(" ");
        }
        if(pid) out->printf("%.2lf(%s)", vv*vv*100.,pid);
        else out->printf("%.2lf(#%"DEC")", vv*vv*100.,ind[ir]);

        if(outshort) {
            if(pid) outshort->printf("%.lf-%s", vv*vv*100.,pid);
            else outshort->printf("%.lf-#%"DEC"", vv*vv*100.,ind[ir]);
        }

        ++ip;
    }
    //out->printf("   [ %lf %lf ]",sum, sum2);
    out->printf("\n");

}



/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ extraction routines
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void sMatrix::copy(sMatrix & newmat, bool transpose) const
{
    if(transpose)newmat.resize(cols(),rows());
    else newmat.resize(rows(),cols());

    for ( idx ir=0; ir<rows(); ++ir) {
        for(idx ic=0; ic<cols(); ++ic) {
            if(transpose) newmat.val(ic,ir)=val(ir,ic);
            else newmat.val(ir,ic)=val(ir,ic);
        }
    }
}

void sMatrix::extractRowset(sMatrix & newmat, sVec < idx> & rowset) const
{
    idx orirows=newmat.rows();
    newmat.resize(orirows+rowset.dim(),cols());

    idx ireal=0;
    for ( idx ir=0; ir<rowset.dim(); ++ir) {
        idx iir=rowset[ir];
        if(iir<0)
            continue;

        for(idx ic=0; ic<cols(); ++ic) { // for every column
            newmat[orirows+ireal][ic]=val(iir,ic);
        }
        ++ireal;
    }
    if(ireal!=rowset.dim())
        newmat.resize(orirows+ireal,cols());
}

void sMatrix::extractRowset(sMatrix & newmat, sVec < sVec < idx > > & rowsetset) const
{
    for ( idx il=0; il<rowsetset.dim(); ++il) {extractRowset(newmat,rowsetset[il]);}
}

void sMatrix::extractColset(sMatrix & newmat, sVec < idx> & colset) const
{
    newmat.resize(rows(),colset.dim());

    for ( idx ic=0; ic<colset.dim(); ++ic) {
        for(idx ir=0; ir<rows(); ++ir) { // for every column
            idx iic=colset[ic];
            newmat[ir][ic]=val(ir,iic);
        }
    }

}

void sMatrix::extractColset(sMatrix & newmat, sVec < sVec < idx > > & colsetset) const
{
    for ( idx il=0; il<colsetset.dim(); ++il) {extractRowset(newmat,colsetset[il]);}
}


void sMatrix::sortOrderByCol(idx * indexes, idx col) const
{
    sVec <real > sorder; sorder.add(cols());
    for(idx ir=0; ir< cols(); ++ir ) sorder[ir]=-sAbs(val(ir,col));
    sSort::sort(cols(), sorder.ptr(), indexes);
}


void sMatrix::squareRoot(sMatrix & result) const
{
    idx ccols=cols();
    sMatrix copy; copy.resize(ccols,ccols);
    for(idx i1=0; i1<ccols; ++i1) for(idx i2=0; i2<ccols; ++i2)copy(i1,i2)=val(i1,i2);

    sVec <real> eVals;eVals.resize(ccols);
    sMatrix eVecs;eVecs.resize(ccols,ccols);
    copy.diagJacoby(eVals,&eVecs);
    sMatrix diag;diag.resize(ccols,ccols);diag.set(0);
    for(idx i=0; i<ccols-1; ++i) {
        real val=eVals[i];val=sAbs(val);
        if(val<1e-10)val=0;
        diag(i,i)=sqrt(val); // the very last one should be zeroo anyway
    }
    sMatrix stp1; stp1.multiplyMatrixes(diag,eVecs,true);
    result.multiplyMatrixes(eVecs, stp1);
    ///sMatrix stp3; stp3.multiplyMatrixes(stp2,stp2,false);
}



void sMatrix::statisticsRowset( DistrRowSetStruc * ds, sDic < sVec < idx > > & rowsToUseDic ) const
{


    ds->dPerColStdDev=0;
    ds->dPerColMean=0;
    ds->dPerColStdDevCls=0;
    ds->dPerColMeanCls=0;
    ds->dPerRowStdDev=0;
    ds->dPerRowMean=0;
    ds->dPerRowArm=0;
    ds->dPerColArm=0;

    if(ds->distributionPerCol ) {
        ds->distributionPerCol->resize(2*cols() *( 1 + rowsToUseDic.dim()) + cols()*( 1 + rowsToUseDic.dim())*2 ); // need space for distribution STDEV and means for each measurement 1 for all rows and for each class and more for arms and delat-arms
        ds->distributionPerCol->set(0);
        ds->dPerColStdDev=ds->distributionPerCol->ptr(0);
        ds->dPerColStdDevCls=ds->dPerColStdDev+cols();
        ds->dPerColMean=ds->distributionPerCol->ptr(ds->distributionPerCol->dim()/2);
        ds->dPerColMeanCls=ds->dPerColMean+cols();
        ds->dPerColArm=ds->dPerColMeanCls+cols()*rowsToUseDic.dim();
    }

    if( ds->distributionPerRow ) {
        ds->distributionPerRow->resize(2*( rows()) + 2*rows() ); // need space for distribution STDEV and means per row and one more for arms and delta arms
        ds->distributionPerRow->set(0);
        ds->dPerRowStdDev=ds->distributionPerRow->ptr(0);
        ds->dPerRowMean=ds->distributionPerRow->ptr(ds->distributionPerRow->dim()/2);
        ds->dPerRowArm=ds->dPerRowMean+rows();
    }

    // if we are checking the distributions
    if(ds->dPerColMean || ds->dPerRowMean) {

        //accumulate the values
        for(idx iCls=0; iCls<rowsToUseDic.dim(); ++iCls) {
            idx cntR=rowsToUseDic.ptr(iCls)->dim();
            idx * ptrR=rowsToUseDic.ptr(iCls)->ptr(0);
            for(idx iR=0; iR<cntR; ++iR) {
                idx ir=ptrR[iR];
                for(idx ic=0; ic<cols(); ++ic) {
                    real v=val(ir,ic);
                    if(ds->dPerColMean) {
                        ds->dPerColMean[ic]+=v;
                        ds->dPerColMeanCls[iCls*cols()+ic]+=v;
                    }
                    if(ds->dPerRowMean)
                        ds->dPerRowMean[ir]+=v;

                }
            }
        }

        // compute the means
        idx totCnt=0;
        if(ds->dPerColMean) {
            for(idx iCls=0; iCls<rowsToUseDic.dim(); ++iCls) {totCnt+=rowsToUseDic.ptr(iCls)->dim();}
            for(idx ic=0; ic<cols(); ++ic) {
                ds->dPerColMean[ic]/=totCnt;
                for(idx iCls=0; iCls<rowsToUseDic.dim(); ++iCls) {
                    ds->dPerColMeanCls[iCls*cols()+ic]/=rowsToUseDic.ptr(iCls)->dim();
                }
            }
        }

        if(ds->dPerRowMean) {
            for(idx ic=0; ic<cols(); ++ic) {
                ds->dPerRowMean[ic]/=cols();
            }
        }

        real delta=0;

        //accumulate the variance
        for(idx iCls=0; iCls<rowsToUseDic.dim(); ++iCls) {
            idx cntR=rowsToUseDic.ptr(iCls)->dim();
            idx * ptrR=rowsToUseDic.ptr(iCls)->ptr(0);
            for(idx iR=0; iR<cntR; ++iR) {
                idx ir=ptrR[iR];
                for(idx ic=0; ic<cols(); ++ic) {
                    real v=val(ir,ic),d;
                    if(ds->dPerColMean) {
                        d=v-ds->dPerColMean[ic];
                        ds->dPerColStdDev[ic]+=d*d;
                        ds->dPerColArm[2*ic]+=v*d;
                        ds->dPerColArm[2*ic+1]+=v*delta;

                        d=v-ds->dPerColMeanCls[iCls*cols()+ic];
                        ds->dPerColStdDevCls[iCls*cols()+ic]+=d*d;
                        ds->dPerColArm[iCls*cols()+2*ic]+=v*d;
                        ds->dPerColArm[iCls*cols()+2*ic+1]+=v*delta;
                    }
                    if(ds->dPerRowMean)
                        d=v-ds->dPerRowMean[ir];
                        ds->dPerRowStdDev[ir]+=d*d;
                        ds->dPerRowArm[2*ir]+=v*d;
                        ds->dPerRowArm[2*ir+1]+=v*delta;
                }
            }
        }

        // compute the stdDev values
        if(ds->dPerColMean) {
            for(idx ic=0; ic<cols(); ++ic) {
                ds->dPerColStdDev[ic]=sqrt(ds->dPerColStdDev[ic]/totCnt);

                for(idx iCls=0; iCls<rowsToUseDic.dim(); ++iCls) {
                    ds->dPerColStdDevCls[iCls*cols()+ic]=sqrt(ds->dPerColStdDevCls[iCls*cols()+ic]/rowsToUseDic.ptr(iCls)->dim());
                }
            }
        }

        if(ds->dPerRowMean) {
            for(idx ic=0; ic<cols(); ++ic) {
                ds->dPerRowStdDev[ic]=sqrt(ds->dPerRowStdDev[ic]/cols());
            }
        }
    }


}
/*

idx sMatrix::parseMAFasta(sStr * flbuf, const char* src, idx len, sVec < sMex::Pos> * colIds, sVec < sMex::Pos> * rowIds)
{
    sVax v(idx flagSet, src, len );

    while( v.ensureRecordBuf()) {

    }

    return (ir+1)*(ic+1);
}

*/




idx sMatrix::parseCsv(sStr * flbuf, const char* src, idx len, sVec < sMex::Pos> * colIds, sVec < sMex::Pos> * rowIds,
        const char * ignoresym, idx dicMode, real binThreshold, bool readNumsAsNums, bool transpose/*=false*/, bool supportquote/*=false*/,
        const char * filterRows00, const char * filterCols00
        , idx nonzeroMin, idx zeroMax
        )
{

    sVec < sText::SearchStruc > fltRows;
    if(filterRows00) sText::compileSearchStrings(filterRows00, &fltRows);
    sVec < sText::SearchStruc > fltCols;
    if(filterCols00) sText::compileSearchStrings(filterCols00, &fltCols);
    if(zeroMax==-1)zeroMax=0x7FFFFFFFFFFFFFFFll;


    //if(!src || !len)return 0;
    if(!src )return 0;
    sString::searchAndReplaceSymbols(flbuf,src, len, sString_symbolsEndline,",,",0,true,true,true);
    flbuf->add0(4);
    //sString::searchAndReplaceSymbols(flbuf->ptr(),flbuf->length(), ",;",0,0,true,false,true);
    sString::searchAndReplaceSymbols(flbuf->ptr(),flbuf->length(), ",",0,0,true,false,true);
    const char* fp0=flbuf->ptr();
    sVec< sDic <idx> > adic;
    idx ival;

    const char * prow, * pcol;
    idx irow=0, icol=0, ic=0, ir=0, slen;
    idx irow_src=0, icol_src=0, imaxcol_src=0, icol_digested, irow_digested;

    sVec < idx > colsToRemove;
    sVec < idx > rowsToRemove;

    for ( irow_src=0, prow=fp0; prow; prow=sString::next00(prow) , ++irow_src) {
        for ( icol_src=0, pcol=prow; pcol; pcol=sString::next00(pcol) , ++icol_src) {
            idx pl=sLen(pcol);
            idx quot= ( *pcol == '\"' && pcol[pl-1]==*pcol) ? 1 : 0 ;


            if(irow_src==0 && fltCols.dim() ) {
                if( sText::matchSearchToString(pcol+quot, pl-quot*2 , fltCols.ptr(), fltCols.dim() , 1) )
                    *colsToRemove.add()=icol_src;//-(rowIds ? 1 : 0 );
            }
            if(fltRows.dim() && icol_src==0) {
                if( sText::matchSearchToString(pcol+quot, pl-quot*2 , fltRows.ptr(), fltRows.dim() , 1) ) {
                    *rowsToRemove.add()=irow_src;//-(colIds ? 1 : 0 );
                    //printf("::%.*s david david davinci",10,pcol);
                }
            }
            prow=pcol+pl+1;
            if(icol_src>imaxcol_src)imaxcol_src=icol_src;
        }
        if(icol_src>imaxcol_src)imaxcol_src=icol_src;
    }
    icol_src=imaxcol_src;

    irow_src-=rowsToRemove.dim();
    icol_src-=colsToRemove.dim();

    if( transpose ) {
        irow = icol_src;
        icol = irow_src;
    } else {
        irow = irow_src;
        icol = icol_src;
    }

    colIds->add(icol);
    rowIds->add(irow);
    if(colIds && irow)--irow;
    if(rowIds && icol)--icol;
    resize(irow, icol);

    idx iRowRemoveFinder=0;

    for ( irow_src=0, irow_digested=0,  prow=fp0; prow; prow=sString::next00(prow) , ++irow_src) {

        // scan if this is one of our removed rows
        while ( iRowRemoveFinder < rowsToRemove.dim() && rowsToRemove[iRowRemoveFinder]<irow_src ) {
            ++iRowRemoveFinder;
        }
        if ( iRowRemoveFinder < rowsToRemove.dim() && rowsToRemove[iRowRemoveFinder]==irow_src) {
            for ( pcol=prow; pcol; pcol=sString::next00(pcol) )
                prow=pcol+sLen(pcol)+1;
            continue;
        }

        idx iColRemoveFinder=0;
        // Vahan: lastImple
        idx cntNonZeros=0;
        bool counting=false;
        for ( icol_src=0, icol_digested=0, pcol=prow; pcol; pcol=sString::next00(pcol) , ++icol_src) {

            // scan if this is one of our removed columns
            while ( iColRemoveFinder < colsToRemove.dim() && colsToRemove[iColRemoveFinder]<icol_src ) {
                ++iColRemoveFinder;
            }
            if ( iColRemoveFinder < colsToRemove.dim() && colsToRemove[iColRemoveFinder]==icol_src) {
                prow=pcol+sLen(pcol)+1;
                continue;
            }

        /*    if( transpose ) {
                irow = icol_src;
                icol = irow_src;
            } else {
                irow = irow_src;
                icol = icol_src;
            }*/
            if( transpose ) {
                irow = icol_digested;
                icol = irow_digested;
            } else {
                irow = irow_digested;
                icol = icol_digested;
            }

            ic=icol;
            ir=irow;
            //slen=sLen(pcol)+1;
            slen=sLen(pcol);
            if(supportquote) {
                char quot=*pcol;
                if( (quot=='\"' || quot=='\'') && pcol[slen-1]==quot) {
                    ++pcol;slen-=2;
                }
            }
            if(colIds){
                if( irow==0 ) {
                    (*colIds)[icol].pos=(idx)(pcol-fp0);(*colIds)[icol].size=slen;
                }
                --ic;
            }
            if(rowIds){
                if( icol==0 ) {
                    (*rowIds)[irow].pos=(idx)(pcol-fp0);(*rowIds)[irow].size=slen;
                }
                --ir;
            }

            if(dicMode!=0)
                adic.resize(icol+1);

            if(ir>=0 && ic>=0) {
                if(ignoresym)
                    while(strchr(ignoresym,*pcol))++pcol;

                bool couldReadAsNumber=true;
                if(!sscanf(pcol,"%lf",ptr(ir,ic)))
                    couldReadAsNumber=false;

                if(!readNumsAsNums || !couldReadAsNumber){

                    if (dicMode==2 || (dicMode==1 && ir==1)){
                        adic[icol].set(pcol,0,&ival);
                        val(ir,ic)=(real)(ival);
                    }
                    else if (dicMode==1){
                        if( adic[icol].find(pcol,0))
                            ival=0;
                        else ival=1;
                        val(ir,ic)=(real)(ival);
                    }else {// if(dicMode==0){
                        //if(!sscanf(pcol,"%lf",ptr(ir,ic)))
                        if( !couldReadAsNumber )
                            val(ir,ic)=0;
                    }
                }
                // Vahan: lastImple
                //if(nonZeroMin<zeroMax)
                counting=true;
                //if(val(ir,ic)!=0)
                //    ++cntNonZeros;
            }

            if(counting) {
                if(val(ir,ic)!=0)
                    ++cntNonZeros;

                if(dicMode==3 )
                {
                    if(val(ir,ic)<binThreshold)
                        val(ir,ic)=0;
                    else
                        val(ir,ic)=1;

                }

            }

            prow=pcol+sLen(pcol)+1;
            ++icol_digested;
        }

        // Vahan: lastImple
        if(counting && (cntNonZeros<nonzeroMin || (icol-cntNonZeros)>zeroMax) )
            --irow_digested;
        ++irow_digested;
    }

    // Vahan: lastImple
    resize(irow_digested, icol);

    return (ir+1)*(ic+1);
}


/*
 * dicMode= 0 is
 * dicMode= 1 is for strings. If equal to the first cell, 0, otherwise 1
 * dicMode= 2 is for strings. If equal to the first cell, 0, otherwise i, i++
 */
idx sMatrix::parseTabular(sTabular * tbl, sVec< idx > * rowSet, sVec< idx > * colSet, sDic <idx > * colIds, sDic <idx> * rowIds, const char * ignoresym, idx dicMode, real binThreshold, bool readNumsAsNums, sVec< idx > * revertColSet, idx forceRowID)
{
    if(rowSet && !rowSet->dim())rowSet=0;
    //if(!src || !len)return 0;
    sVec< sDic <idx> > adic;
    idx ival;


    //const char * b;
    idx irow=0, icol=0;

    //rowIds->add(tbl->rows());


    idx cntCols = (colSet && colSet->dim()) ? colSet->dim() : tbl->cols(), excludedCols=0;
    idx cntRows = (rowSet && rowSet->dim()) ? rowSet->dim() : tbl->rows();
    resize(cntRows,cntCols);
    //colIds->add(cntCols);

    idx iCol;
    sStr cellResult;
    for ( idx icol=0, irc,serial=0; icol<cntCols; ++serial) {
        iCol= (colSet && colSet->dim()) ? (*colSet)[serial] : serial;
        if( revertColSet ) {
            for ( irc=0; irc<revertColSet->dim() && iCol!=(*revertColSet)[irc]; ++irc){} // try finding it in a revert colset
            if(irc<revertColSet->dim() ) { // found in exclusion list
                ++excludedCols;
                continue;
            }
        }

        if(colIds ){
            //do printCell instead of cell here

            cellResult.cut(0);tbl->printTopHeader(cellResult, iCol);
            if (cellResult && ::strcmp(cellResult.ptr(), ""))
                *colIds->set(cellResult.ptr())=icol;
            //else ::printf("\nERRRRRRRRRRRR %"DEC" %"DEC" \n",icol,iCol);
        }
        ++icol;
    }
    if(excludedCols){
        resize(cntRows,cntCols-excludedCols);
    }

    //idx cntCols = (colSet && colSet->dim()) ? colSet->dim() : tbl->cols();

    for ( irow=0; irow<cntRows; ++irow) {
        idx iRow=rowSet ? (*rowSet)[irow] : irow;

        if(rowIds ) {
            cellResult.cut(0);tbl->printCell(cellResult, iRow,forceRowID );
            if (cellResult && ::strcmp(cellResult.ptr(), ""))
                *rowIds->set(cellResult.ptr())=irow;
        }


        //idx cntCols = (colSet && colSet->dim()) ? colSet->dim() : tbl->cols();

        for ( idx icol=0, irc, serial=0; icol<cntCols; ++serial) {
            iCol= (colSet && colSet->dim()) ? (*colSet)[serial] : serial;
            if( revertColSet ) {
                for ( irc=0; irc<revertColSet->dim() && iCol!=(*revertColSet)[irc]; ++irc){} // try finding it in a revert colset
                if(irc<revertColSet->dim() ) { // found in exclusion list
                    ++excludedCols;
                    continue;
                }
            }


            adic.resize(icol+1);

            cellResult.cut(0);tbl->printCell(cellResult, iRow, iCol);

            idx startPos=0;
            if(ignoresym)
                while(strchr(ignoresym,*(cellResult.ptr(startPos))))++startPos;

            bool couldReadAsNumber=true;
            if(!sscanf(cellResult.ptr(startPos),"%lf",ptr(iRow,icol)))
                couldReadAsNumber=false;

            if (readNumsAsNums && couldReadAsNumber)
            {
                val(irow,icol)=atof(cellResult.ptr(startPos));
            }

            if(!readNumsAsNums || !couldReadAsNumber){

                if (dicMode==2 || (dicMode==1 && irow==0)){
                    adic[icol].set(cellResult.ptr(startPos),0,&ival);
                    val(irow,icol)=(real)(ival);
                }
                else if (dicMode==1){
                    if( adic[icol].find(cellResult.ptr(startPos),0))
                        ival=0;
                    else ival=1;
                    val(irow,icol)=(real)(ival);
                }else {
                    if( !couldReadAsNumber )
                        val(irow,icol)=0;
                }
            }

            if(dicMode==3 )
            {
                if(val(irow,icol)<binThreshold)
                    val(irow,icol)=0;
                else
                    val(irow,icol)=1
                    ;

            }

            ++icol;
        }
    }

    return (irow)*(icol);
}
