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
#include <ulib/uusage.hpp>
#include <slib/utils/json/parser.hpp>
#include "tblqryX4_cmd.hpp"
#include "tblqryX4_cmd_addmissingrows.hpp"
#include "../../vlib/ulib/uperm.hpp"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        struct CmdLoadSNPprofile: public tblqryx4::Command
        {
            idx loader_handle;
            sHiveId obj_id;
            sUsrObj * obj;
            bool thumb;
            bool auto_add_missing_rows;
            sVariant add_missing_rows_arg;
            idx sub;

            CmdAddMissingRows cmd_add_missing_rows;

            virtual const char * getName() { return "load-SNPprofile"; }

            CmdLoadSNPprofile(ExecContext & ctx) :
                tblqryx4::Command(ctx),
                cmd_add_missing_rows(ctx)
            {
                loader_handle = -1;
                obj = 0;
                thumb = false;
                auto_add_missing_rows = false;
                sub = 0;
            }

            virtual ~CmdLoadSNPprofile()
            {
                delete obj;
            }

            virtual bool init(const char * op_name, sVariant * tqs_arg);
            virtual bool compute(sTabular * in_table);
        };

        Command * cmdLoadSNPprofileFactory(ExecContext & ctx) { return new CmdLoadSNPprofile(ctx); }
    }
}

using namespace slib;
using namespace slib::tblqryx4;

static bool findCsvStartLen(idx * pos_out, idx * len_out, sFil & fil, idx isub)
{
    const char * buf_start = sBioseqSNP::binarySearchReference(fil.ptr(), fil.last(), isub, false);
    const char * buf_end = sBioseqSNP::binarySearchReference(fil.ptr(), fil.last(), isub, true);
    if( buf_start >= fil.ptr() && buf_end >= fil.ptr() && buf_start < buf_end ) {
        *pos_out = buf_start - fil.ptr();
        *len_out = buf_end - buf_start;
        return true;
    } else {
        *pos_out = -sIdxMax;
        *len_out = 0;
        return false;
    }
}

bool CmdLoadSNPprofile::init(const char * op_name, sVariant * arg)
{
    if( arg ) {
        if( sVariant * obj_val = arg->getDicElt("obj") ) {
            obj_val->asHiveId(&obj_id);
        }
        if( !obj_id ) {
            _ctx.logError("loadSNPprofile command: missing or invalid obj argument");
            return false;
        }
        obj = _ctx.qproc().user->objFactory(obj_id, 0, ePermCanRead | ePermCanDownload);
        if( !obj || !obj->Id() ) {
            _ctx.logError("loadSNPprofile command: object %s cannot be loaded", obj_id.print());
            return false;
        }
        if( sVariant * ref_val = arg->getDicElt("sub") ) {
            sub = ref_val->asInt();
        }
        if( sub <= 0 ) {
            _ctx.logError("loadSNPprofile command: missing or invalid sub argument");
            return false;
        }

        if( sVariant * thumb_val = arg->getDicElt("thumb") ) {
            thumb = thumb_val->asBool();
        }
        if( sVariant * add_missing_rows_val = arg->getDicElt("autoAddMissingRows") ) {
            auto_add_missing_rows = add_missing_rows_val->asBool();
        }
    } else {
        _ctx.logError("loadSNPprofile command: missing argument");
        return false;
    }

    sStr csv_path, thumb_path, idx_suffix;
    const char * tbl_name = 0;
    sFil csv_file;
    idx csv_pos = -sIdxMax, csv_len = 0;
    bool is_thumb = false;
    if( obj->getFilePathname(csv_path, "SNPprofile.csv") ) {
        // heptagon profile
        if( thumb ) {
            sStr thumb_path;
            if( obj->getFilePathname(thumb_path, "SNPthumb.csv") ) {
                csv_file.destroy();
                csv_file.init(thumb_path.ptr(), sMex::fReadonly);
                if( csv_file.ok() && csv_file.length() && findCsvStartLen(&csv_pos, &csv_len, csv_file, sub) ) {
                    tbl_name = "SNPthumb.csv";
                    idx_suffix.printf(0, "iSub-%"DEC, sub);
                    is_thumb = true;
                }
            }
        }
        if( csv_pos < 0 ) {
            csv_file.destroy();
            csv_file.init(csv_path.ptr(), sMex::fReadonly);
            if( csv_file.ok() && csv_file.length() && findCsvStartLen(&csv_pos, &csv_len, csv_file, sub) ) {
                tbl_name = "SNPprofile.csv";
                idx_suffix.printf(0, "iSub-%"DEC, sub);
            }
        }
    } else if( obj->getFilePathname(csv_path, "SNPprofile-%"DEC".csv", sub ) ) {
        // check if it's an old profiler with per-subject files
        tbl_name = sFilePath::nextToSlash(csv_path.ptr());
        csv_file.destroy();
        csv_file.init(csv_path.ptr(), sMex::fReadonly);
        if( csv_file.ok() && csv_file.length() ) {
            csv_pos = 0;
            csv_len = csv_file.length();
            tbl_name = sFilePath::nextToSlash(csv_path.ptr());
        }
    } else {
        _ctx.logError("loadSNPprofile command: object %s doesn't look like a profiler or heptagon object", obj_id.print());
        return false;
    }

    if( csv_pos < 0 ) {
        _ctx.logError("loadSNPprofile command: object %s doesn't have snp table for subject %"DEC, obj_id.print(), sub);
        return false;
    }

    if( auto_add_missing_rows && !is_thumb ) {
        sStr json_buf(
            "{ \
                \"abscissa\": { \
                    \"col\": { \"name\": \"Position\" }, \
                    \"dense\": false, \
                    \"minValue\": 1, \
                    \"maxValue\": { \"formula\": \"input_obj.refSeqLen(%"DEC")\" } \
                }, \
                \"add\": [ \
                    { \"col\": { \"name\": \"Reference\" }, \"value\": %"DEC" }, \
                    { \"col\": { \"name\": \"Letter\" }, \"value\": { \"formula\": \"input_obj.refSeqLetter(%"DEC",cur_abscissa_val)\" } }, \
                    { \"col\": { \"name\": \"Consensus\" }, \"value\": \"A\" }, \
                    { \"col\": \"*\", \"value\": 0 } \
                ] \
            }", sub, sub, sub);
        sJSONParser json_parser;
        json_parser.parse(json_buf.ptr(), json_buf.length());
        add_missing_rows_arg = json_parser.result();
#ifdef _DEBUG
        json_buf.add0();
        fprintf(stderr, "Inserting addMissingRows sub-command with TQS arg %s\n", add_missing_rows_arg.print(json_buf, sVariant::eJSON));
#endif
        if( !cmd_add_missing_rows.init(op_name, &add_missing_rows_arg) ) {
            return false;
        }
    }

    loader_handle = _ctx.allocateLoaderHandle();
    _ctx.requestLoadObjTable(loader_handle, obj_id, tbl_name, idx_suffix.ptr(), false, ",", 0, 0, sIdxMax, csv_pos, csv_pos ? 0 : -1, csv_len);

    return true;
}

bool CmdLoadSNPprofile::compute(sTabular * tbl)
{
    setOutTable(_ctx.releaseLoadedTable(loader_handle));
    bool ret = true;
    if( getOutTable() && !add_missing_rows_arg.isNull() ) {
        _ctx.logDebug("Running addMissingRows sub-command");
        _ctx.qlangCtx().setTable(getOutTable());
        ret = cmd_add_missing_rows.compute(getOutTable());
        cmd_add_missing_rows.postcompute();
        if( sTabular * out_add_missing = cmd_add_missing_rows.getOutTable() ) {
            setOutTable(out_add_missing);
            cmd_add_missing_rows.setOutTable(0);
        }
        _ctx.qlangCtx().setTable(getOutTable());
    }
    return ret;
}
