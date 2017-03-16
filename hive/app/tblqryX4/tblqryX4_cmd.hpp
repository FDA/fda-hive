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
#ifndef sLib_tblqryx4_cmd_h
#define sLib_tblqryx4_cmd_h

#include <qlib/QPrideProc.hpp>
#include "exec-context.hpp"
#include "sort.hpp"

namespace slib {
    namespace tblqryx4 {
        class Command {
            public:
                Command(ExecContext & ctx) : _ctx(ctx)
                {
                    _out_table = 0;
                }

                static Command * extFactory(const char * ext_name, ExecContext & ctx);

                virtual ~Command() { delete _out_table; }

                virtual const char * getName() = 0;

                // override these in child classes to change operation behavior
                //! does this command have compute() that is non-trivial and needs to be reported in overall request progress?
                virtual bool hasProgress() { return true; }
                //! does this command produce an output table in compute()?
                virtual bool computesOutTable() { return true; }
                //! does compute() require an input table where strings are reinterpreted as numbers?
                virtual bool needsInTableReinterpret() { return false; }
                //! does _out_table wrap in_table, so that in_table cannot be simply deleted after compute()?
                virtual bool wrapsInTable() { return false; }

                //! define in child class: parse and check parameters from tqs operation and argument, return true on success
                virtual bool init(const char * op_name, sVariant * tqs_arg) = 0;
                //! define in child class: run the computation, return true on success
                virtual bool compute(sTabular * in_table) = 0;
                //! define in child class if needed: clean up after compute(), whether or not it succeeded
                virtual void postcompute() {}

                sTabular * getOutTable() { return _out_table; }
                void setOutTable(sTabular * t) { _out_table = t; }

            protected:
                ExecContext & _ctx;
                sTabular * _out_table;
        };

        class GraphCommand : public Command
        {
            protected:
                idx colorMethod;
                sTree::DistanceMethods buildMethod;
                sVec <idx> colSetImg;
                sVec <idx> colSetTree;
                sVec <idx> rowSet;
                sVec <idx> categories;
                sVec <idx> uid;
                idx dataMode;
                idx readNumsAsNums;
                idx mapSize;
                RowSorter sorter;

            public:
                GraphCommand(ExecContext & ctx) : Command(ctx)
                {
                    buildMethod = sTree::EUCLIDEAN;
                    colorMethod = dataMode = readNumsAsNums = mapSize = 0;
                }
                virtual ~GraphCommand() {}
        };
    };
};

#endif
