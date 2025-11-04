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

#include <ion/sIon.hpp>
#include <ssci/bio/tax-ion.hpp>
#include <violin/hiveqlang.hpp>
#include <violin/hivespecobj.hpp>

using namespace slib;

qlang::sHiveContext::sHiveContext()
{
    _tax_ion = 0;
    registerDefaultBuiltins();
}

qlang::sHiveContext::sHiveContext(const sUsr & usr, idx flags) : qlang::sUsrContext(usr, flags)
{
    _tax_ion = 0;
    registerDefaultBuiltins();
}

qlang::sHiveContext::~sHiveContext()
{
    delete _tax_ion;
    _tax_ion = 0;
}

void qlang::sHiveContext::init(const sUsr &usr, idx flags)
{
    reset();
    qlang::sUsrContext::init(usr, flags);
    sHiveContext::registerDefaultBuiltins();
}

void qlang::sHiveContext::reset()
{
    delete _tax_ion;
    _tax_ion = 0;
    sUsrContext::reset();
}

bool qlang::sHiveContext::ensureTaxIon()
{
    if( _tax_ion ) {
        return true;
    }

    sStr tax_ion_path, error_log;
    if( !sviolin::SpecialObj::findTaxDbIonPath(tax_ion_path, *_usr, 0, 0, &error_log) ) {
        setError(EVAL_OTHER_ERROR, "%s", error_log.ptr());
        return false;
    }

    _tax_ion = new sTaxIon(tax_ion_path);
    return true;
}


class sHiveContext_tax_id_info: public qlang::BuiltinFunction
{
    public:
        sHiveContext_tax_id_info()
        {
            _name.printf("builtin tax_id_info() function");
        }

        bool call(sVariant &result, qlang::Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            qlang::sHiveContext & ctx = static_cast<qlang::sHiveContext &>(qlctx);

            if( !ctx.ensureTaxIon() || !checkNArgs(ctx, nargs, 1, 1) ) {
                return false;
            }
            sStr tax_id_buf("%" UDEC, args->asUInt());
            const char * tax_result = ctx.getTaxIon()->getTaxIdInfo(tax_id_buf.ptr(), tax_id_buf.length());
            idx tax_result_len = sLen(tax_result);
            if( !tax_result || !tax_result_len ) {
                result.setNull();
                return true;
            }
            sTxtTbl result_parser;
            result_parser.parseOptions().flags = 0;
            result_parser.setBuf(tax_result, tax_result_len);
            if( !result_parser.parse() || result_parser.cols() < 5 ) {
                ctx.setError(qlang::EVAL_SYSTEM_ERROR, "Taxonomy database result in unexpected format");
                return false;
            }
            result.setDic();
            result.setElt("tax_id", result_parser.uval(0, 0));
            result.setElt("parent_tax_id", result_parser.uval(0, 1));
            sVariant rank_value;
            result_parser.val(rank_value, 0, 2);
            result.setElt("rank", rank_value);
            sVariant name_value;
            result_parser.val(name_value, 0, 3);
            result.setElt("name", name_value);
            result.setElt("num_children", result_parser.ival(0, 4));
            return true;
        }
};


class sHiveContext_tax_ids_by_name: public qlang::BuiltinFunction
{
    public:
        sHiveContext_tax_ids_by_name()
        {
            _name.printf("builtin tax_ids_by_name() function");
        }

        bool call(sVariant &result, qlang::Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            qlang::sHiveContext & ctx = static_cast<qlang::sHiveContext &>(qlctx);

            if( !ctx.ensureTaxIon() || !checkNArgs(ctx, nargs, 1, 2) ) {
                return false;
            }

            idx limit = 0;
            if( nargs > 1 ) {
                limit = args[1].asInt();
            }

            const char * tax_result = ctx.getTaxIon()->getTaxIdsByName(args->asString(), limit);
            idx tax_result_len = sLen(tax_result);
            sTxtTbl result_parser;
            result_parser.parseOptions().flags = 0;
            result_parser.setBuf(tax_result);
            if( (tax_result_len && !result_parser.parse()) || (result_parser.rows() && result_parser.cols() < 2) ) {
                ctx.setError(qlang::EVAL_SYSTEM_ERROR, "Taxonomy database result in unexpected format");
                return 0;
            }

            result.setList();
            for(idx ir = 0; ir < result_parser.rows(); ir++) {
                sVariant row;
                row.setDic();
                row.setElt("tax_id", result_parser.uval(ir, 0));
                sVariant name_val;
                result_parser.val(name_val, ir, 1);
                row.setElt("name", name_val);
                result.push(row);
            }

            return true;
        }
};

#define REGISTER_BUILTIN_FUNC(name) \
static sHiveContext_ ## name builtin_ ## name; \
registerBuiltinFunction(#name, builtin_ ## name)


void qlang::sHiveContext::registerDefaultBuiltins()
{
    REGISTER_BUILTIN_FUNC(tax_id_info);
    REGISTER_BUILTIN_FUNC(tax_ids_by_name);
}

qlang::sHiveEngine::sHiveEngine(const sUsr & usr, idx ctx_flags)
{
    _usr = &usr;
    _ctx = new qlang::sHiveContext(usr, ctx_flags);
}

void qlang::sHiveEngine::init(const sUsr &usr, idx ctx_flags)
{
    delete _ctx;
    _usr = &usr;
    _ctx = new qlang::sHiveContext(usr, ctx_flags);
}
