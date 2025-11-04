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
#ifndef sLib_tblqryx4_utils_hpp
#define sLib_tblqryx4_utils_hpp

#include <qlib/QPrideProc.hpp>
#include <regex.h>
#include "rowscols.hpp"
#include "exec-context.hpp"

namespace slib {
    namespace tblqryx4 {
        class ParseUtils
        {
            private:
                static bool parseColsArg(sVec<idx> & out, sVariant * val, const sTabular * tbl, const OutputColumns * out_cols, ExecContext & ctx, bool allowMulti)
                {
                    if( !val )
                        return false;

                    if( val->isDic() ) {
                        bool is_optional = false;
                        if( sVariant * is_optional_val = val->getDicElt("optional") ) {
                            is_optional = is_optional_val->asBool();
                        }

                        if( sVariant * regex_val = val->getDicElt("regex") ) {
                            regex_t re;
                            bool caseSensitive = false;
                            bool negate = false;
                            if( sVariant * case_val = val->getDicElt("caseSensitive") ) {
                                caseSensitive = case_val->asBool();
                            }
                            if( sVariant * negate_val = val->getDicElt("negate") ) {
                                negate = negate_val->asBool();
                            }
                            int reflags = REG_EXTENDED | REG_NOSUB;
                            if( !caseSensitive ) {
                                reflags |= REG_ICASE;
                            }
                            if( regcomp(&re, regex_val->asString(), reflags) != 0 ) {
                                ctx.logError("Invalid regular expression '%s'\n", regex_val->asString());
                                return false;
                            }

                            idx ret = 0;
                            if( tbl ) {
                                sStr header_buf;
                                for(idx icol = -tbl->dimLeftHeader(); icol < tbl->cols(); icol++) {
                                    header_buf.cut(0);
                                    tbl->printTopHeader(header_buf, icol);
                                    int regexec_result = regexec(&re, header_buf.ptr(0), 0, 0, 0);
                                    if( (!negate && !regexec_result) || (negate && regexec_result) ) {
                                        ret++;
                                        *out.add(1) = icol;
                                    }
                                    if( ret && !allowMulti ) {
                                        break;
                                    }
                                }
                            } else if( out_cols ) {
                                for(idx icol = 0; icol < out_cols->dim(); icol++) {
                                    const char * colname = out_cols->getName(icol);
                                    int regexec_result = regexec(&re, colname, 0, 0, 0);
                                    if( (!negate && !regexec_result) || (negate && regexec_result) ) {
                                        ret++;
                                        *out.add(1) = icol;
                                    }
                                    if( ret && !allowMulti ) {
                                        break;
                                    }
                                }
                            }
                            regfree(&re);
                            return is_optional ? true : ret;
                        } else {
                            const char * name = sStr::zero;
                            if( sVariant * name_val = val->getDicElt("name") ) {
                                name = name_val->asString();
                            }

                            idx num = 0;
                            if( sVariant * num_val = val->getDicElt("num") ) {
                                num = num_val->asInt();
                            }

                            if( tbl ) {
                                idx icol = tbl->colId(name, num);
                                if( icol >= -tbl->dimLeftHeader() ) {
                                    *out.add(1) = icol;
                                    return true;
                                }
                            } else if( out_cols ) {
                                idx icol = out_cols->colIndex(name, num);
                                if( icol >= 0 ) {
                                    *out.add(1) = icol;
                                    return true;
                                }
                            }
                            return is_optional;
                        }
                    }
                    *out.add(1) = val->asInt();
                    return true;
                }

            public:
                static bool parseColsArg(sVec<idx> & out, sVariant * val, const sTabular * tbl, ExecContext & ctx, bool allowMulti)
                {
                    return parseColsArg(out, val, tbl, 0, ctx, allowMulti);
                }
                static bool parseColsArg(sVec<idx> & out, sVariant * val, const OutputColumns * out_cols, ExecContext & ctx, bool allowMulti)
                {
                    return parseColsArg(out, val, 0, out_cols, ctx, allowMulti);
                }

                static qlang::ast::Node * parseFormulaArg(const char * fmla_txt, ExecContext & ctx, const char * op)
                {
                    if( unlikely(!fmla_txt) ) {
                        ctx.logError("Missing %s", op);
                        return 0;
                    }

                    if( unlikely(!ctx.qlangParser().parse(fmla_txt, 0, qlang::Parser::fDollarValues)) ) {
                        ctx.logError("Invalid %s: %s", op, ctx.qlangParser().getErrorStr());
                        return 0;
                    }

                    return ctx.qlangParser().releaseAstRoot();
                }

                static qlang::ast::Node * parseFormulaArg(sVariant * val, ExecContext & ctx, const char * op)
                {
                    if( unlikely(!val) ) {
                        ctx.logError("Missing %s", op);
                        return 0;
                    }

                    return parseFormulaArg(val->asString(), ctx, op);
                }

                static sVariant::eType parseTypeArg(sVariant * val)
                {
                    if( unlikely(!val) )
                        return sVariant::value_NULL;
                    return sVariant::parseTypeName(val->asString());
                }

                static const char * evalStringOrFormulaArg(sStr & dst, sVariant * val, ExecContext & ctx)
                {
                    idx pos = dst.length();
                    if( val->isDic() && val->getDicElt("formula") ) {
                        const char * fmla_txt = val->getDicElt("formula")->asString();
                        if( unlikely(!ctx.qlangParser().parse(fmla_txt, 0, qlang::Parser::fDollarValues)) ) {
                            ctx.logError("Invalid formula: %s", ctx.qlangParser().getErrorStr());
                            return 0;
                        }
                        sVariant fmla_result;
                        if( unlikely(!ctx.qlangParser().getAstRoot()->eval(fmla_result, ctx.qlangCtx())) ) {
                            sStr err_buf;
                            ctx.logError("Invalid formula: %s", ctx.qlangCtx().printError(err_buf));
                        }
                        dst.addString(fmla_result.asString());
                    } else {
                        return dst.addString(val->asString());
                    }
                    return dst.ptr(pos);
                }
        };
    };
};

#endif
