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

#include <ssci/math.hpp>

#include "tblqryX4_cmd.hpp"

#define PRFX "pca-"
#define OUTFILE "pca.csv"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class PcaCommand : public Command
        {
            private:
                sVec<idx> rowSet;
                sVec<idx> colSet;
                sVec<idx> categories;
                bool is_categories_col_default;

            public:
                PcaCommand(ExecContext & ctx) : Command(ctx)
                {
                    is_categories_col_default = false;
                }

                const char * getName() { return "pca"; }
                bool computesOutTable() { return false; }
                bool neeedsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdPcaFactory(ExecContext & ctx) { return new PcaCommand(ctx); }
    };
};

bool variantToIVec(sVec<idx> & outvec, const sVariant * val)
{
    if( val->isScalar() ) {
        *outvec.add(1) = val->asInt();
        return true;
    } else if( val->isList() ) {
        idx pos = outvec.dim();
        idx dim = val->dim();
        outvec.add(dim);
        for(idx i = 0; i < dim; i++) {
            outvec[pos + i] = val->getListElt(i)->asInt();
        }
        return true;
    }
    return false;
}

bool PcaCommand::init(const char * op_name, sVariant * arg)
{
    if( sVariant * colSetVal = arg->getDicElt("colSet") ) {
        if( !variantToIVec(colSet, colSetVal) ) {
            _ctx.logError("%s command: invalid colSet argument", op_name);
            return false;
        }
    }
    if( sVariant * rowSetVal = arg->getDicElt("rowSet") ) {
        if( !variantToIVec(rowSet, rowSetVal) ) {
            _ctx.logError("%s command: invalid rowSet argument", op_name);
            return false;
        }
    }
    if( sVariant * categoriesVal = arg->getDicElt("categories") ) {
        if( !variantToIVec(categories, categoriesVal) ) {
            _ctx.logError("%s command: invalid categories argument", op_name);
            return false;
        }
    } else {
        is_categories_col_default = true;
    }

    return true;
}

bool PcaCommand::compute(sTabular * tbl)
{
    if( is_categories_col_default && !categories.dim() ) {
        if( tbl->dimLeftHeader() ) {
            *categories.vadd(1) = -1;
        } else if( tbl->cols() ){
            *categories.vadd(1) = 0;
        }

        _ctx.logDebug("Assuming categories = [%" DEC "]", categories[0]);
    }

    idx mat_rows = rowSet.dim() ? rowSet.dim() : tbl->rows();
    idx mat_cols = colSet.dim() ? colSet.dim() : tbl->cols();

    if( !colSet.dim() && categories.dim() && tbl->cols() ) {
        for(idx icol = 0; icol < tbl->cols(); icol++) {
            bool is_category = false;
            for(idx j = 0; j < categories.dim(); j++) {
                if( icol == categories[j] ) {
                    is_category = true;
                    break;
                }
            }
            if( !is_category ) {
                *colSet.add(1) = icol;
            }
        }

        mat_cols = colSet.dim();

        sStr buf;
        sString::printfIVec(&buf, &colSet);
        buf.add0cut();
        _ctx.logDebug("Assuming colSet = [%s]", buf.ptr(0));
    }

    sMatrix mat;
    sMatrix evecs;
    sVec<real> evals;

    if( mat_rows && mat_cols ) {
        mat.parseTabular(tbl, &rowSet, &colSet, 0, 0, 0, 0, 0, true);
        evecs.resize(mat.cols(), mat.cols());
        evals.resize(mat.cols());
        sAlgebra::matrix::pca(mat.ptr(0, 0), mat.rows(), mat.cols(), evals.ptr(), evecs.ptr(0, 0));
    }

    sStr remapFilePath;
    if( _ctx.qproc().reqAddFile(remapFilePath, "pca-remap.csv") ) {
       _ctx.logDebug("Writing %s", remapFilePath.ptr());
       sFil remapFile(remapFilePath);
       sStr buf;
       idx out_icol = 0;

       if( mat_cols + categories.dim() ) {
           for(idx icatcol = 0; icatcol < categories.dim(); icatcol++, out_icol++) {
               if( out_icol ) {
                   remapFile.addString(",");
               }
               buf.cut0cut();
               tbl->printTopHeader(buf, categories[icatcol]);
               sString::escapeForCSV(remapFile, buf.ptr(), buf.length());
           }
           if( colSet.dim() ) {
               for(idx iicol = 0; iicol < colSet.dim() && iicol < mat.cols(); iicol++, out_icol++) {
                   if( out_icol ) {
                       remapFile.addString(",");
                   }
                   buf.cut0cut();
                   tbl->printTopHeader(buf, colSet[iicol]);
                   sString::escapeForCSV(remapFile, buf.ptr(), buf.length());
               }
           } else {
               for(idx icol = 0; icol < tbl->cols(); icol++, out_icol++) {
                   if( out_icol ) {
                       remapFile.addString(",");
                   }
                   buf.cut0cut();
                   tbl->printTopHeader(buf, icol);
                   sString::escapeForCSV(remapFile, buf.ptr(), buf.length());
               }
           }

           for(idx mat_irow = 0; mat_irow < mat.rows(); mat_irow++) {
               remapFile.addString("\n");

               out_icol = 0;
               idx tbl_irow = rowSet.dim() ? rowSet[mat_irow] : mat_irow;
               for(idx icatcol = 0; icatcol < categories.dim(); icatcol++, out_icol++) {
                   if( out_icol ) {
                       remapFile.addString(",");
                   }
                   buf.cut0cut();
                   tbl->printCell(buf, tbl_irow, categories[icatcol]);
                   sString::escapeForCSV(remapFile, buf.ptr(), buf.length());
               }
               for(idx mat_icol = 0; mat_icol < mat.cols(); mat_icol++, out_icol++) {
                   if( out_icol ) {
                       remapFile.addString(",");
                   }
                   remapFile.printf("%g", mat.val(mat_irow, mat_icol));
               }
           }
       }
    }

    sStr evecFilePath;
    if( _ctx.qproc().reqAddFile(evecFilePath, "pca-eigenvectors.csv") ) {
        _ctx.logDebug("Writing %s", evecFilePath.ptr());
        sFil evecFile(evecFilePath);
        if( mat_rows && mat_cols ) {
            evecs.out(&evecFile);
        }
    }

    sStr evalFilePath;
    if( _ctx.qproc().reqAddFile(evalFilePath, "pca-eigenvalues.csv") ) {
        _ctx.logDebug("Writing %s", evalFilePath.ptr());
        sFil evalFile(evalFilePath);
        for(idx i = 0; i < evals.dim(); i++) {
            if( i ) {
                evalFile.addString("\n");
            }
            evalFile.printf("%g", evals[i]);
        }
    }

    return true;
}
