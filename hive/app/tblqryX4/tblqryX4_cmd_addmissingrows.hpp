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
#ifndef sLib_tblqryx4_cmd_addmissingrows_h
#define sLib_tblqryx4_cmd_addmissingrows_h

#include "tblqryX4_cmd.hpp"
#include "utils.hpp"

namespace slib {
    namespace tblqryx4 {
        class sAddedRowsTabular;

        class CmdAddMissingRows: public tblqryx4::Command
        {
            protected:
                struct ParsedParam
                {
                    sVariant val;
                    ast::Node * fmla;

                    ParsedParam() { fmla = 0; }
                    ~ParsedParam() { delete fmla; }
                };

                sVariant * _abscissa_col_arg;
                ParsedParam _abscissa_max_gap;
                ParsedParam _abscissa_min_value;
                ParsedParam _abscissa_max_value;
                bool _abscissa_dense;
                bool _abscissa_descending;

                sVec<sVariant *> _add_cols_args;
                sVec<ParsedParam> _add_values;
                ParsedParam _add_default_value;

                idx _cur_abscissa_val;

                bool parseParam(ParsedParam & param, const char * name, sVariant * arg);
                bool evalParam(ParsedParam & param, const char * name);

            public:
                CmdAddMissingRows(ExecContext & ctx)
                : tblqryx4::Command(ctx), _add_cols_args(sMex::fSetZero)
                {
                    _abscissa_col_arg = 0;
                    _abscissa_dense = false;
                    _abscissa_descending = false;
                    _cur_abscissa_val = -sIdxMax;
                }
                const char * getName() { return "addMissingRows"; }
                bool wrapsInTable() { return true; }
                bool init(const char * op_name, sVariant * tqs_arg);
                bool compute(sTabular * in_table);
                void postcompute()
                {
                    _ctx.qlangCtx().removeBuiltin("cur_abscissa_val");
                }

                void addMissingRow(sAddedRowsTabular * out, idx abscissa_value, const sVec<idx> & varying2iadd, idx non_abscissa_varying_cols, sVariant & tmp_val);
                void addMissingRowSpan(sAddedRowsTabular * out, idx abscissa_first, bool include_first, idx abscissa_last, bool include_last, idx abscissa_max_gap, const sVec<idx> & varying2iadd, idx non_abscissa_varying_cols, sVariant & tmp_val);
        };
    };
};

#endif
