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
#include "tblqryX4_cmd.hpp"

using namespace slib;
using namespace slib::qlang;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class CmdGeneric: public tblqryx4::Command
        {
            private:
                sStr _name_buf;
                sVariant * _tqs_arg;
                struct OpDesc {
                    const char * op_name; //! tool name (must match "op" in TQS)
                    const char * cmdline00; //! 0-delimited, 00-terminated list of the initial part of the tool command line
                    bool computes_out_table; //! true if this tool computes an output table
                };
                static const OpDesc _ops[];
                idx _iop;

            public:
                CmdGeneric(ExecContext & ctx) : Command(ctx)
                {
                    _iop = -1;
                    _tqs_arg = 0;
                }

                const char * getName()
                {
                    if( _iop >= 0 ) {
                        if( !_name_buf.length() ) {
                            _name_buf.printf("generic command %s", _ops[_iop].op_name);
                        }
                        return _name_buf.ptr();
                    } else {
                        return "unknown generic command";
                    }
                }
                bool computesOutTable() { return _iop >= 0 ? _ops[_iop].computes_out_table : true; }

                bool init(const char * op_name, sVariant * tqs_arg);
                bool compute(sTabular * in_table);
        };

        Command * cmdGenericFactory(ExecContext & ctx) { return new CmdGeneric(ctx); }
    };
};

// To add a new generic command: describe here and add an entry to TBLQRYX4_EXT_FACTORIES in tblqryX4_cmdlist.hpp
const CmdGeneric::OpDesc CmdGeneric::_ops[] = {
    { "generic-cmd-test", "python-run.os" SLIB_PLATFORM _"2.7" _ "tblqry-generic-cmd-test.py" __, true },
};

bool CmdGeneric::init(const char * op_name, sVariant * tqs_arg)
{
    _name_buf.cutAddString(0, op_name);
    for(idx i = 0; i < sDim(_ops); i++) {
        if( op_name && !strcmp(op_name, _ops[i].op_name) ) {
            _iop = i;
            break;
        }
    }
    if( _iop < 0 ) {
        return false;
    }

    _tqs_arg = tqs_arg;
    return true;
}

bool CmdGeneric::compute(sTabular * in_table)
{
    sStr cmdline00;
    cmdline00.add(_ops[_iop].cmdline00, sString::size00(_ops[_iop].cmdline00));
    cmdline00.shrink00();
    cmdline00.add0();

    cmdline00.add("-i");
    const char * in_path = _ctx.qproc().reqAddFile(cmdline00, "req-generic-cmd-%" DEC "-in.csv", _ctx.curCmdIndex());
    sFil in_fil(in_path);
    if( !in_path || !in_path[0] || !in_fil.ok() ) {
        _ctx.logError("Failed to create file for input table for %s", getName());
        return false;
    }
    _ctx.logDebug("Serializing input table to %s", in_path);
    in_table->printCSV(in_fil);
    in_fil.destroy();

    cmdline00.shrink00();
    cmdline00.add0();
    idx out_path_pos = -1;
    if( computesOutTable() ) {
        cmdline00.add("-o");
        const char * out_path = _ctx.qproc().reqAddFile(cmdline00, "req-generic-cmd-%" DEC "-out.csv", _ctx.curCmdIndex());
        if( !out_path || !out_path[0] ) {
            _ctx.logError("Failed to create path for output table for %s", getName());
            return false;
        }
        out_path_pos = out_path - cmdline00.ptr();
        out_path = 0;
        cmdline00.shrink00();
        cmdline00.add0();
    }

    cmdline00.printf("--req=%" DEC, _ctx.reqID());
    cmdline00.add0();

    for(idx i = 0; i < _tqs_arg->dim(); i++) {
        sVariant val;
        const char * key = _tqs_arg->getDicKeyVal(i, val);
        cmdline00.printf("--%s=", key);
        if( val.isScalar() ) {
            val.print(cmdline00, sVariant::eDefault);
        } else if( val.isList() ) {
            for(idx j = 0; j < val.dim(); j++) {
                if( j ) {
                    cmdline00.addString(",");
                }
                val.getListElt(j)->print(cmdline00, sVariant::eJSON);
            }
        } else {
            val.print(cmdline00, sVariant::eJSON);
        }
        cmdline00.add0();
    }

    cmdline00.add0(2);

    sStr tmpbuf;
    for(const char * s = cmdline00.ptr(); s && *s; s = sString::next00(s)) {
        sString::escapeForShell(tmpbuf, s);
        tmpbuf.addString(" ");
    }
    _ctx.logDebug("Executing command line: %s", tmpbuf.ptr());

    idx retcode = sPS::execute00(cmdline00);
    if( retcode != 0 ) {
        _ctx.logError("%s failed with error code %" DEC, getName(), retcode);
        return false;
    }

    if( computesOutTable() ) {
        const char * out_path = cmdline00.ptr(out_path_pos);
        sTxtTbl * out_table = new sTxtTbl();
        out_table->setFile(out_path);
        if( !out_table->parse() ) {
            _ctx.logError("%s output table is not in CSV format", getName());
            return false;
        }
        setOutTable(out_table);
    }

    return true;
}
