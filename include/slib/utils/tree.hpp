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
#pragma once
#ifndef sLib_utils_tree_hpp
#define sLib_utils_tree_hpp
#include <slib/std.hpp>
#include <slib/core/iter.hpp>
#include <slib/utils/tbl.hpp>
#include <ssci/math/geom/dist.hpp>
#include <ssci/math/clust/clust2.hpp>
#include <slib/utils/sort.hpp>

class sTree {
    public:

        class MyRow : public sIter<real, MyRow>
        {
            protected:
                idx irow;
                idx icoluse; // index into columnsToUse
                bool vertical;

            public:
                const sVec<idx> * columnsToUse;
                const sTabular * tbl;

                MyRow(idx r=0, bool v=0): irow(r), icoluse(0), vertical(v) {columnsToUse=0;tbl=0;}
                MyRow(const MyRow & rhs): irow(rhs.irow), icoluse(rhs.icoluse), vertical(rhs.vertical), columnsToUse(rhs.columnsToUse), tbl(rhs.tbl) {}
                MyRow* clone_impl() const { return new MyRow(*this); }
                bool validData_impl() const { return tbl && icoluse < (columnsToUse && columnsToUse->dim() ? columnsToUse->dim() : vertical ? tbl->rows() : tbl->cols()); }
                idx pos_impl() const { return icoluse; }
                MyRow& increment_impl() { icoluse++; return *this; }
                real dereference_impl() const
                {
                    if (vertical)
                        return tbl->rval(columnsToUse && columnsToUse->dim() ? *(columnsToUse->ptr(icoluse)) : icoluse, irow);
                    else
                        return tbl->rval(irow, columnsToUse && columnsToUse->dim() ? *(columnsToUse->ptr(icoluse)) : icoluse);
                }
                MyRow& operator=(const MyRow& rhs)
                {
                    irow = rhs.irow;
                    icoluse = rhs.icoluse;
                    vertical = rhs.vertical;
                    columnsToUse = rhs.columnsToUse;
                    tbl = rhs.tbl;
                    return *this;
                }
        };
        typedef MyRow Titer;

        enum DistanceMethods
        {
            EUCLIDEAN, MANHATTAN, MAXIMUM, COSINE
        };
        enum neighborJoiningMethods
        {
            FAST, REGULAR
        };

        static idx generateTree (sStr & out,sVec < idx > * columnsToUse, sVec <idx > * rowsToUse, sTabular * tbl, sVec <idx> * newOrder, idx horizontal=1, sVec < idx > * uIDs=0, DistanceMethods distMethod=EUCLIDEAN, neighborJoiningMethods jMethod=REGULAR);


};

#endif // sLib_utils_tree_hpp
