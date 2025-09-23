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
#include <slib/utils/json/parser.hpp>
#include "json-lexer.hpp"

#include <assert.h>

using namespace slib;

const char * sJSONParser::tokenName(sJSONParser::EToken token)
{
    switch(token) {
        case eTokenError:
            return "invalid content";
        case eTokenEOF:
            return "end of file";
        case eTokenNull:
            return "null value";
        case eTokenBool:
            return "boolean value";
        case eTokenInt:
            return "integer value";
        case eTokenReal:
            return "real value";
        case eTokenString:
            return "string";
        case eTokenComma:
            return "','";
        case eTokenColon:
            return "':'";
        case eTokenOpenArray:
            return "'['";
        case eTokenCloseArray:
            return "']'";
        case eTokenOpenObject:
            return "'{'";
        case eTokenCloseObject:
            return "'}'";
    }
    return 0;
}

slib::sJSONParser::sJSONParser()
{
    _buf = 0;
    _decode_buf = 0;
    _buflen = 0;
    _cb = 0;
    _cb_param = 0;
    _no_result = _stopped = false;
    debug_lexer = debug_parser = false;
}

void slib::sJSONParser::setParseCallback(slib::sJSONParser::parseCallback cb, void * cb_param, bool no_result, sStr * decode_buf)
{
    _cb = cb;
    _cb_param = cb_param;
    _no_result = no_result;
    _decode_buf = decode_buf;
}

void sJSONParser::wrongToken(sJSONParser::EToken token, const sJSONParser::TokenValue * tokval, const char * expected)
{
    setError(tokval->start_line, tokval->start_col, _buf + tokval->start_pos, "JSON syntax error: unexpected %s; expected %s", sJSONParser::tokenName(token), expected);
}

bool sJSONParser::handleValue(sJSONParser::ParseNode & node, sJSONParser::EToken token, sJSONParser::TokenValue * tokval)
{
    sVariant new_val;
    bool no_push = _no_result;

    node.val.i = 0;
    node.val_str_len = 0;
    node.val_raw = 0;
    node.val_raw_len = 0;
    node.val_line = tokval->start_line;
    node.val_col = tokval->start_col;
    node.val_pos = tokval->start_pos;

    bool failure = false;
    sVariant * cur_accum = _levels[_levels.dim() - 1].accum;
    bool accum_pushed_value = false;

    switch(token) {
        case sJSONParser::eTokenNull:
            node.value_type = eValueNull;
            node.val.i = 0;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setNull();
            }
            break;
        case sJSONParser::eTokenBool:
            node.value_type = eValueBool;
            node.val.i = tokval->num.i;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setBool(node.val.i);
            }
            break;
        case sJSONParser::eTokenInt:
            node.value_type = eValueInt;
            node.val.i = tokval->num.i;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setInt(node.val.i);
            }
            break;
        case sJSONParser::eTokenReal:
            node.value_type = eValueReal;
            node.val.r = tokval->num.r;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setReal(node.val.r);
            }
            break;
        case sJSONParser::eTokenString:
            node.value_type = eValueString;
            if( tokval->str_len ) {
                node.val.str = tokval->str_in_decode_buf ? tokval->decode_buf.ptr(tokval->str_start) : _buf + tokval->str_start;
            } else {
                node.val.str = sStr::zero;
            }
            node.val_str_len = tokval->str_len;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setString(node.val.str, node.val_str_len);
            }
            break;
        case sJSONParser::eTokenOpenArray:
        {
            node.value_type = eValueArray;
            node.val.i = 0;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setList();
                accum_pushed_value = true;
            }
            NodeLevel * lev = _levels.add(1);
            lev->node_type = eNodeArrayElement;
            lev->index = -1;
            lev->accum = 0;
            break;
        }
        case sJSONParser::eTokenOpenObject:
        {
            node.value_type = eValueObject;
            node.val.i = 0;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            if( !_no_result ) {
                new_val.setDic();
                accum_pushed_value = true;
            }
            NodeLevel * lev = _levels.add(1);
            lev->node_type = eNodeObjectElement;
            lev->index = -1;
            lev->accum = 0;
            break;
        }
        case sJSONParser::eTokenCloseArray:
        case sJSONParser::eTokenCloseObject:
            node.value_type = eValueEndOfContainer;
            node.val.i = 0;
            node.val_str_len = 0;
            node.val_raw = _buf + tokval->start_pos;
            node.val_raw_len = tokval->raw_len;
            no_push = true;
            break;
        default:
            wrongToken(token, tokval, "a value");
            failure = true;
    }

    if( !failure ) {
        if( _cb ) {
            _cb(node, *this, _cb_param);
            failure = _error.length();
        }

        if( !no_push ) {
            assert(cur_accum);
            sVariant * pushed_value = 0;
            switch( node.node_type ) {
                case eNodeTopLevel:
                    *cur_accum = new_val;
                    pushed_value = cur_accum;
                    break;
                case eNodeArrayElement:
                    pushed_value = cur_accum->push(new_val);
                    assert(pushed_value);
                    break;
                case eNodeObjectElement:
                    pushed_value = cur_accum->setElt(node.key_str, node.key_str_len, new_val);
                    assert(pushed_value);
            }
            if( accum_pushed_value ) {
                _levels[_levels.dim() - 1].accum = pushed_value;
            }
        }
    }

    return !failure;
}

bool slib::sJSONParser::parse(const char *buf, idx len, idx * plen_parsed, idx flags)
{
    _result.setNull();
    _error.cut(0);
    _levels.cut(0);
    _levels.resize(1);
    _levels[0].node_type = eNodeTopLevel;
    _levels[0].index = 0;
    _levels[0].accum = _no_result ? 0 : &_result;
    _stopped = false;

    _buf = buf;
    _buflen = len ? len : sLen(_buf);

    sStr local_decode_buf, & decode_buf = _decode_buf ? *_decode_buf : local_decode_buf;

    sJSONParser::Lexer lexer(this, _buf, _buflen);
    sJSONParser::TokenValue tokval(decode_buf), tokval2(decode_buf);

    ParseNode node;
    sSet(&node, 0);
    node.node_type = eNodeTopLevel;

    if( !_buflen ) {
        if( flags & fParseEmpty ) {
            return true;
        } else {
            _error.cutAddString(0, "JSON syntax error: unexpected empty input");
            return false;
        }
    }

    bool failure = false;
    _levels[0].index = -1;

    do {
        sJSONParser::EToken token = lexer.lex(tokval);
        if( token == sJSONParser::eTokenError ) {
            failure = true;
            break;
        }

        const idx nlevels = _levels.dim();
        node.depth = nlevels - 1;
        node.node_type = _levels[nlevels - 1].node_type;
        node.index = _levels[nlevels - 1].index;
        node.key_str = 0;
        node.key_str_len = 0;
        node.key_raw = 0;
        node.key_raw_len = 0;
        node.key_line = node.key_col = node.key_pos = -1;

        switch(node.node_type) {
            case eNodeTopLevel:
                if( _levels[0].index == -1 ) {
                    node.index = _levels[0].index = 0;
                    if( token == sJSONParser::eTokenEOF ) {
                        if( flags & fParseEmpty ) {
                            _levels.cut(0);
                        } else {
                            _error.cutAddString(0, "JSON syntax error: unexpected empty input");
                            failure = true;
                        }
                    } else if( token == eTokenError || token == eTokenCloseArray || token == eTokenCloseObject ) {
                        wrongToken(token, &tokval, "JSON value");
                        failure = true;
                    } else {
                        failure = !handleValue(node, token, &tokval);
                    }
                } else if( token == sJSONParser::eTokenEOF ) {
                    _levels.cut(0);
                } else {
                    wrongToken(token, &tokval, "end of input");
                    failure = true;
                }
                break;
            case eNodeArrayElement:
                if( token == eTokenCloseArray ) {
                    failure = !handleValue(node, token, &tokval);
                    if( !failure ) {
                        _levels.cut(nlevels - 1);
                    }
                } else {
                    if( node.index < 0 ) {
                        node.index = _levels[nlevels - 1].index = 0;
                    } else if( token == eTokenComma ) {
                        token = lexer.lex(tokval);
                        if( token == eTokenError ) {
                            failure = true;
                        }
                    } else {
                        wrongToken(token, &tokval, "',' or ']'");
                        failure = true;
                    }

                    if( !failure ) {
                        failure = !handleValue(node, token, &tokval);
                    }

                    if( !failure ) {
                        _levels[nlevels - 1].index++;
                    }
                }
                break;
            case eNodeObjectElement:
                if( token == eTokenCloseObject ) {
                    failure = !handleValue(node, token, &tokval);
                    if( !failure ) {
                        _levels.cut(nlevels - 1);
                    }
                } else {
                    if( node.index < 0 ) {
                        node.index = _levels[nlevels - 1].index = 0;
                    } else if( token == eTokenComma ) {
                        token = lexer.lex(tokval);
                        if( token == eTokenError ) {
                            failure = true;
                        }
                    } else {
                        wrongToken(token, &tokval, "',' or '}'");
                        failure = true;
                    }

                    if( !failure ) {
                        if( token == eTokenString ) {
                            token = lexer.lex(tokval2);
                            if( token == eTokenColon ) {
                                token = lexer.lex(tokval2);

                                if( tokval.str_len ) {
                                    node.key_str = tokval.str_in_decode_buf ? tokval.decode_buf.ptr(tokval.str_start) : _buf + tokval.str_start;
                                } else {
                                    node.key_str = sStr::zero;
                                }
                                node.key_str_len = tokval.str_len;
                                node.key_raw = _buf + tokval.start_pos;
                                node.key_raw_len = tokval.raw_len;

                                node.key_line = tokval.start_line;
                                node.key_col = tokval.start_col;
                                node.key_pos = tokval.start_pos;

                                failure = !handleValue(node, token, &tokval2);
                            } else {
                                wrongToken(token, &tokval2, "':'");
                                failure = true;
                            }
                        } else {
                            wrongToken(token, &tokval, "key string");
                            failure = true;
                        }
                    }

                    if( !failure ) {
                        _levels[nlevels - 1].index++;
                    }
                }
                break;
        }
    } while( _levels.dim() && !failure && !_stopped && (_levels.dim() > 1 || !(flags & fParsePrefix)) );

    if( failure ) {
        if( plen_parsed ) {
            *plen_parsed = 0;
        }
        _result.setNull();
        return false;
    } else {
        if( plen_parsed ) {
            *plen_parsed = lexer.getPos();
        }
        return true;
    }
}

void slib::sJSONParser::setError(idx line, idx col, const char * raw, const char * msg_fmt, ...)
{
    va_list ap;
    va_start(ap, msg_fmt);
    vsetError(line, col, raw, msg_fmt, ap);
    va_end(ap);
}

void sJSONParser::setKeyError(ParseNode & node, const char * msg_fmt, ...)
{
    va_list ap;
    va_start(ap, msg_fmt);
    vsetError(node.key_line, node.key_col, node.key_raw, msg_fmt, ap);
    va_end(ap);
}

void sJSONParser::setValueError(ParseNode & node, const char * msg_fmt, ...)
{
    va_list ap;
    va_start(ap, msg_fmt);
    vsetError(node.val_line, node.val_col, node.val_raw, msg_fmt, ap);
    va_end(ap);
}

void slib::sJSONParser::vsetError(idx line, idx col, const char * raw, const char * msg_fmt, va_list ap)
{
    _error.cut0cut();
    if( line >= 0 ) {
        if( col >= 0 ) {
            _error.printf("line %" DEC " col %" DEC ": ", line + 1, col + 1);
        } else {
            _error.printf("line %" DEC ": ", line);
        }
    } else if( col >= 0 ) {
        _error.printf("col %" DEC ": ", col);
    }
    _error.vprintf(msg_fmt, ap);
    if( !_buf || !_buflen || !raw || !*raw ) {
        return;
    }

    static const idx max_len_affected = 80;
    static const idx max_len_left = max_len_affected / 2 - 1;


    idx len_left = 0;
    for(; raw - len_left > _buf && len_left < max_len_affected - 1 && raw[-len_left - 1] && raw[-len_left - 1] != '\r' && raw[-len_left - 1] != '\n'; len_left++);
    idx len_right = 0;
    for(; raw + len_right < _buf + _buflen && len_right < max_len_affected - 1 && raw[len_right + 1] && raw[len_right + 1] != '\r' && raw[len_right + 1] != '\n'; len_right++);

    if( len_left + len_right + 1 > max_len_affected ) {
        if( len_left <= max_len_left ) {
            len_right = max_len_affected - len_left;
        } else if( len_right <= max_len_affected - max_len_left ) {
            len_left = max_len_left;
        } else {
            len_left = max_len_left;
            len_right = max_len_affected - max_len_left;
        }
    }

    _error.addString("\n\n", 2);
    _error.addString(raw - len_left, len_left + len_right + 1);
    _error.addString("\n", 1);
    for(idx i = 0; i < len_left; i++) {
        _error.addString(" ", 1);
    }
    _error.addString("^\n", 2);
}

void sJSONParser::clearError()
{
    _error.cut0cut();
}

void sJSONParser::stopParser()
{
    _stopped = true;
}

const char * slib::sJSONParser::printError(sStr &s)
{
    idx len = s.length();
    if (_error.length()) {
        s.printf("%s", _error.ptr());
    } else {
        s.add0cut();
    }
    return s.ptr(len);
}
