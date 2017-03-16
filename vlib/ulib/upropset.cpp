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

#include <slib/core/tim.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>
#include <ulib/upropset.hpp>
#include <ulib/uobj.hpp>
#include <ulib/uquery.hpp>
#include "uperm.hpp"
#include <assert.h>
#include <ctype.h>

using namespace slib;

#define UPROPSET_TRACE 0

static const char * canonicalCase(sStr & buf, const char * str, idx len = 0)
{
    if( !str ) {
        return sStr::zero;
    }
    if( !len ) {
        len = sLen(str);
    }
    for(idx i=0; i<len; i++) {
        if( str[i] >= 'A' && str[i] <= 'Z' ) {
            buf.cut0cut();
            sString::changeCase(&buf, str, 0, sString::eCaseLo);
            return buf.ptr(0);
        }
    }
    return str;
}

sUsrPropSet::ObjLoc::ObjLoc()
{
    obj.is_new = false;
    obj.utype = 0;
    uobj = 0;
    is_auto_upsert = false;
}

sUsrPropSet::ObjLoc::~ObjLoc()
{
    delete uobj;
    uobj = 0;
}

sUsrPropSet::Perm::Perm()
{
    iperm = -1;
    group_id = 0; // assume 0 is invalid
    acts = 0;
    flags = 0;
    acts_enabled = false;
    state = eInOther;
}

sUsrPropSet::sUsrPropSet()
{
    _usr = 0;
    _srcbuf = 0;
    _srclen = 0;
    _cur_iobj = -sIdxMax;
    _cur_submode = eMode_other;
    _cur_icomment = -sIdxMax;
    _second_pass_mode = eSecondPass_Pretend;
    _var_set_out = 0;
    _prop_fmt_out = 0;
    _flags = 0;
}

sUsrPropSet & sUsrPropSet::reset(const sUsr & usr)
{
    _srcfil.destroy();
    _err.cut0cut();
    _file_path_buf.printf(0, ".%c", sDir::sep);

    _usr = &usr;
    _srcbuf = 0;
    _srclen = 0;
    return *this;
}

sUsrPropSet & sUsrPropSet::setSrc(const char * jsonbuf, idx len/* = 0 */)
{
    _srclen = len ? len : sLen(jsonbuf);
    _srcbuf = _srclen ? jsonbuf : 0;
    return *this;
}

sUsrPropSet & sUsrPropSet::setSrcFile(const char * jsonfilename)
{
    _srcfil.destroy();
    _srcfil.init(jsonfilename, sMex::fReadonly);
    if( _srcfil.ok() ) {
        _srcbuf = _srcfil.length() ? _srcfil.ptr(0) : 0;
        _srclen = _srcfil.length();
    } else {
        _err.cutAddString(0, "Failed to open file");
    }
    return *this;
}

sUsrPropSet & sUsrPropSet::setFileRoot(const char * path, idx len/* = 0 */)
{
    if( !len ) {
        len = sLen(path);
    }
    if( len ) {
        _file_path_buf.cutAddString(0, path, len);
        if( _file_path_buf[_file_path_buf.length() - 1] != sDir::sep ) {
            _file_path_buf.addString(&sDir::sep, 1);
        }
    } else {
        _file_path_buf.printf(0, ".%c", sDir::sep);
    }
    return *this;
}

sUsrPropSet & sUsrPropSet::disableFileRoot()
{
    _file_path_buf.cut0cut();
    return *this;
}

// Check that a path is (1) non-empty, (2) non-absolute, (3) has no .. elements, (4) exists.
// If everything looks good, print to outbuf.
static const char * printRelativeFilePath(sStr & outbuf, const char * path, idx len, sFilePath & tmp)
{
    if( !path || !len ) {
        // empty path
        return 0;
    }

    tmp.addString(path, len);
    tmp.simplifyPath();
    tmp.shrink00();

    if( !tmp.length() || !tmp[0] ) {
        // empty path after simplification
        return 0;
    }
    if( tmp[0] == '/' ) {
        // absolute path
        return 0;
    }
    for(idx i = 0; i + 1 < len; i++) {
        if( path[i] == '.' && path[i + 1] == '.' && (i == 0 || path[i - 1] == '/') && (i + 2 == len || path[i + 2] == '/') ) {
            // path equals "..", or starts with "../", or ends with "/..", or contains "/../"
            return 0;
        }
    }
    idx outbuf_start = outbuf.length();
    outbuf.addString("./", 2);
    outbuf.addString(tmp.ptr(), tmp.length());
    if( outbuf[outbuf.length() - 1] == '/' ) {
        // clean up terminal '/' to avoid breaking nextToSlash()
        outbuf.cut0cut(outbuf.length() - 1);
    }
    if( !sFile::exists(outbuf.ptr(outbuf_start)) ) {
        outbuf.cut0cut(outbuf_start);
        return 0;
    }
    return outbuf.ptr(outbuf_start);
}

static idx strncmp_exact(const char * s1, idx len1, const char * s2, idx len2)
{
    if( !len1 ) {
        len1 = sLen(s1);
    }
    if( !len2 ) {
        len2 = sLen(s2);
    }
    if( int d = strncmp(s1, s2, sMin<idx>(len1, len2)) ) {
        return d;
    }
    return len1 - len2;
}

// First pass: elementary structural validation; detect comments; find object nodes and their _id-s
//static
void sUsrPropSet::jsonFirstPassCb(sJSONParser::ParseNode & node, sJSONParser & parser, void * param)
{
    sUsrPropSet * self = static_cast<sUsrPropSet*>(param);

    if( node.value_type == sJSONParser::eValueEndOfContainer ) {
        // not relevant on first pass
        return;
    }

    if( node.node_type == sJSONParser::eNodeTopLevel && !(node.value_type == sJSONParser::eValueObject || node.value_type == sJSONParser::eValueArray ) ) {
        parser.setValueError(node, "input must be a JSON object or array");
        return;
    }

    if( node.depth == 1 ) {
        if( node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("_comment", 8, node.key_str, node.key_str_len) == 0 ) {
            // objects-level comment!
            self->_cur_iobj = -sIdxMax;
            NodeLoc * comment_loc = self->_comment_loc.set(&node.val_pos, sizeof(node.val_pos));
            comment_loc->set(node.val_pos, node.depth, node.index);
        } else {
            if( node.value_type != sJSONParser::eValueObject ) {
                parser.setValueError(node, "input must be a JSON object (or array) whose top-level values are JSON objects");
                return;
            }
            sStr array_key_buf; // used once per second-level nocde and only if top-level node is an array
            const char * node_key_str = node.key_str;
            idx node_key_str_len = node.key_str_len;
            if( node.node_type == sJSONParser::eNodeArrayElement ) {
                array_key_buf.addNum(node.index);
                node_key_str = array_key_buf.ptr();
                node_key_str_len = array_key_buf.length();
            }
            sUsrPropSet::ObjLoc * prop_obj = self->_objs.setString(node_key_str, node_key_str_len, &self->_cur_iobj);
            prop_obj->loc.set(node.val_pos, node.depth, node.index);
            prop_obj->obj.is_new = false;
            prop_obj->obj.id.reset();
            prop_obj->obj.utype = 0;
            prop_obj->uobj = 0;
        }
    } else if( node.depth == 2 && self->_cur_iobj >= 0 ) {
        sUsrPropSet::ObjLoc * prop_obj = self->_objs.ptr(self->_cur_iobj);
        if( node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("_id", 0, node.key_str, node.key_str_len) == 0 ) {
            prop_obj->obj.id.reset();
            prop_obj->obj.is_new = false;
            if( node.value_type == sJSONParser::eValueInt ) {
                prop_obj->obj.id.setObjId(node.val.i);
            } else if( node.value_type == sJSONParser::eValueString ) {
                if( strncmp_exact("$newid()", 0, node.val.str, node.val_str_len) == 0 ) {
                    prop_obj->obj.is_new = true;
                } else if( strncmp_exact("$upsert()", 0, node.val.str, node.val_str_len) == 0 ) {
                    prop_obj->is_auto_upsert = true;
                    prop_obj->key_fields.setDic();
                } else if( strncmp_exact("$upsert_qry(", 12, node.val.str, sMin<idx>(12, node.val_str_len)) == 0 && node.val.str[node.val_str_len - 1] == ')' ) {
                    if( !self->readUpsertNode(prop_obj->obj, node, parser) ) {
                        return;
                    }
                } else {
                    idx len_parsed = prop_obj->obj.id.parse(node.val.str, node.val_str_len);
                    if( len_parsed < node.val_str_len ) {
                        prop_obj->obj.id.reset();
                    }
                }
            }
        } else if( node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("_type", 0, node.key_str, node.key_str_len) == 0 ) {
            prop_obj->obj.utype = 0;
            if( node.value_type == sJSONParser::eValueString ) {
                if( !self->ensureUTypeFor(prop_obj, node, parser) ) {
                    return;
                }
            }
        }
    }

    // unlazy-clean up key tic, so the last (editable) frame is at level node.depth - 1
    while( node.depth < self->_keys_tic.dimStack() ) {
        self->_keys_tic.pop();
    }

#if UPROPSET_TRACE
    {
        sStr key_str;
        key_str.addString(node.key_str, node.key_str_len);
        fprintf(stderr, "\nnode: depth = %"DEC", index = %"DEC", key = %s, val_pos == %"DEC"\n", node.depth, node.index, key_str.ptr(), node.val_pos);
    }
#endif

    if( node.node_type == sJSONParser::eNodeObjectElement ) {
        // lazy-initialize key tic frames up to necessary level, so the last (editable) frame is at level node.depth - 1
        while( node.depth > self->_keys_tic.dimStack() ) {
            self->_keys_tic.push();
        }
        sUsrPropSet::NodeLoc * ploc = self->_keys_tic.getTop(node.key_str, node.key_str_len);
        if( ploc ) {
            // same key was present earlier in this JSON object; this means previous occurrence of the key was a comment
            NodeLoc * comment_loc = self->_comment_loc.set(&ploc->pos, sizeof(ploc->pos));
            comment_loc->set(ploc->pos, ploc->depth, ploc->index);
#if UPROPSET_TRACE
            fprintf(stderr, "Repeated key; comment at val_pos == %"DEC", depth == %"DEC", index == %"DEC"\n", ploc->pos, ploc->depth, ploc->index);
#endif
        } else {
            ploc = self->_keys_tic.setString(node.key_str, node.key_str_len);
        }
        ploc->set(node.val_pos, node.depth, node.index);
    }

#if UPROPSET_TRACE
    {
        for(idx i = 0; i < self->_keys_tic.dimStack(); i++) {
            fprintf(stderr, "stack level %"DEC": ", i);
            for(idx j = 0; j < self->_keys_tic.dimLevel(i); j++) {
                if( j ) {
                    fprintf(stderr, ", ");
                }
                fprintf(stderr, "%s", static_cast<const char*>(self->_keys_tic.id(i, j)));
            }
            fprintf(stderr, "\n");
        }
        for(idx i = 0; i < self->_comment_loc.dim(); i++) {
            NodeLoc * comment_loc = self->_comment_loc.ptr(i);
            fprintf(stderr, "comment #%"DEC": val_pos == %"DEC", depth == %"DEC", index == %"DEC"\n", i, comment_loc->pos, comment_loc->depth, comment_loc->index);
        }
    }
#endif
}

// virtual
bool sUsrPropSet::ensureUTypeFor(sUsrPropSet::ObjLoc * prop_obj, sJSONParser::ParseNode & node, sJSONParser & parser)
{
    bool no_prefetch_types = false;
    if( node.val.str && strncmp_exact(node.val.str, node.val_str_len, "type", 4) == 0 ) {
        // special case: for type objects, fetch only type "type", no other types - otherwise,
        // that could spuriously break "make install" when installing types out of order
        no_prefetch_types = true;
    }
    prop_obj->obj.utype = sUsrType2::ensure(*_usr, node.val.str, node.val_str_len, no_prefetch_types);
    if( prop_obj->obj.utype ) {
        return true;
    } else {
        parser.setValueError(node, "unknown type name");
        return false;
    }
}

namespace {
    struct JsonSecondPassCbParam {
        sUsrPropSet * self;
        sStr cur_obj_str;
        sStr path_buf;
        sFilePath tmp_file_buf;
    };
};

//static
void sUsrPropSet::jsonSecondPassCb(sJSONParser::ParseNode & node, sJSONParser & parser, void * param_)
{
    JsonSecondPassCbParam * param = static_cast<JsonSecondPassCbParam*>(param_);
    sUsrPropSet * self = param->self;
    sStr & cur_obj_str = param->cur_obj_str;

    if( self->inComment(node) ) {
#if UPROPSET_TRACE
    {
        sStr key_str;
        key_str.addString(node.key_str, node.key_str_len);
        fprintf(stderr, "\nnode: depth = %"DEC", index = %"DEC", key = %s, val_pos == %"DEC"; cur_comment = %"DEC"\n", node.depth, node.index, key_str.ptr(), node.val_pos, self->_cur_icomment);
    }
#endif
        return;
    } else {
#if UPROPSET_TRACE
    {
        sStr key_str;
        key_str.addString(node.key_str, node.key_str_len);
        fprintf(stderr, "\nnode: depth = %"DEC", index = %"DEC", key = %s, val_pos == %"DEC"; cur_comment = %"DEC"\n", node.depth, node.index, key_str.ptr(), node.val_pos, self->_cur_icomment);
    }
#endif
    }

    if( node.depth == 1 ) {
        if( node.value_type == sJSONParser::eValueEndOfContainer ) {
            // end of top-level JSON object/array - not relevant at depth 1
            return;
        }
        sStr array_key_buf; // used once per second-level nocde and only if top-level node is an array
        const char * node_key_str = node.key_str;
        idx node_key_str_len = node.key_str_len;
        if( node.node_type == sJSONParser::eNodeArrayElement ) {
            array_key_buf.addNum(node.index);
            node_key_str = array_key_buf.ptr();
            node_key_str_len = array_key_buf.length();
        }

        self->_objs.get(node_key_str, node_key_str_len, &self->_cur_iobj);
        ObjLoc * prop_obj = self->_objs.ptr(self->_cur_iobj);
        if( !self->checkCurObjSanity(node, parser) ) {
            return;
        }
        if( prop_obj->obj.is_new ) {
            cur_obj_str.cutAddString(0, node.key_str, node.key_str_len);
        } else {
            cur_obj_str.cut(0);
            prop_obj->obj.id.print(cur_obj_str);
        }

        if( self->_second_pass_mode == eSecondPass_Pretend ) {
            if( self->_prop_fmt_out && !prop_obj->obj.is_new ) {
                self->printPretendField(cur_obj_str, "_id", (const char *)0);
                self->printPretendValue(cur_obj_str, cur_obj_str.length());
            }
            self->printPretendField(cur_obj_str, "_type", (const char *)0);
            self->printPretendValue(self->getUTypeName(prop_obj));
        } else if ( self->_second_pass_mode == eSecondPass_PropSet ) {
            if( !prop_obj->obj.is_new && !self->propInit(prop_obj) ) {
                parser.setValueError(node, "Failed clearing existing object %s properties", prop_obj->obj.id.print());
                return;
            }
        }

        self->_field_stack.cut(0);
    } else if( node.depth == 2 ) {
        if( node.value_type == sJSONParser::eValueEndOfContainer ) {
            // end of JSON object value at second level (corresponding to DB object)
            if( self->_second_pass_mode == eSecondPass_AutoUpsert ) {
                self->autoUpsertFindId(node, parser, self->_objs.ptr(self->_cur_iobj));
            }
            // in non-auto-upsert mode, not relevant at depth 2
            return;
        }
        self->_cur_submode = self->readSubmode(node, parser);
    }

    if( node.depth > 1 ) {
        ObjLoc * prop_obj = self->_objs.ptr(self->_cur_iobj);
        switch( self->_cur_submode ) {
            case eMode_field: {
                if( node.value_type == sJSONParser::eValueEndOfContainer ) {
                    // not relevant for fields in sUsrPropSet (but might be relevant in subclasses)
                    self->readFieldNodeEndOfContainer(node, parser);
                    return;
                }
                if( !self->readFieldNode(node, parser) ) {
                    return;
                }

                FieldStackElt * elt = self->_field_stack.ptr(self->_field_stack.dim() - 1);

                if( node.value_type == sJSONParser::eValueArray || node.value_type == sJSONParser::eValueObject ) {
                    return;
                }
                if( !self->fldCanHaveValue(elt->fld) ) {
                    if( self->fldNeedsValidation(elt->fld) ) {
                        parser.setValueError(node, "Property %s cannot have a scalar value", self->fldName(elt->fld));
                    }
                    return;
                }
                if( !self->fldCanSetValue(elt->fld) ) {
                    // fields which must not be modified at propSet() level, e.g. "created", "modified"
                    return;
                }

                param->path_buf.cut0cut();
                const char * value_path = self->printPropPath(param->path_buf);
                if( self->_second_pass_mode == eSecondPass_Pretend ) {
                    self->printPretendField(cur_obj_str, self->fldName(elt->fld), value_path);
                }

                udx propset_success_cnt = 0;
                switch(node.value_type) {
                    case sJSONParser::eValueNull:
                    {
                        const char * value = 0;
                        udx value_len = 0;
                        // Special case: JSON null maps to NaN, which we can store as a literal string
                        // This will be converted back to null by propget for serializing to JSON
                        if( self->fldType(elt->fld) == sUsrTypeField::eReal ) {
                            value = "NaN";
                            value_len = sLen(value);
                        }

                        if( self->_second_pass_mode == eSecondPass_Pretend ) {
                            self->printPretendValue(value);
                        } else if( self->_second_pass_mode == eSecondPass_AutoUpsert ) {
                            if( self->fldIsKey(elt->fld) && !self->fldIsGlobalMulti(elt->fld) ) {
                                prop_obj->key_fields.setElt(self->fldName(elt->fld), value);
                            }
                        } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                            if( !value) {
                                value = sStr::zero;
                            }
                            propset_success_cnt = self->propSet(prop_obj, elt->fld, value_path, value, value_len);
                        }
                        break;
                    }
                    case sJSONParser::eValueBool:
                        if( self->_second_pass_mode == eSecondPass_Pretend ) {
                            if( node.val.i ) {
                                self->printPretendValue(1);
                            } else {
                                self->printPretendValue((const char *)0);
                            }
                        } else if( self->_second_pass_mode == eSecondPass_AutoUpsert ) {
                            if( self->fldIsKey(elt->fld) && !self->fldIsGlobalMulti(elt->fld) ) {
                                prop_obj->key_fields.setElt(self->fldName(elt->fld), node.val.i);
                            }
                        } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                            const char * value = node.val.i ? "1" : sStr::zero;
                            udx value_len = sLen(value);
                            propset_success_cnt = self->propSet(prop_obj, elt->fld, value_path, value, value_len);
                        }
                        break;
                    case sJSONParser::eValueInt:
                        if( self->_second_pass_mode == eSecondPass_Pretend ) {
                            self->printPretendValue(node.val.i);
                        } else if( self->_second_pass_mode == eSecondPass_AutoUpsert ) {
                            if( self->fldIsKey(elt->fld) && !self->fldIsGlobalMulti(elt->fld) ) {
                                prop_obj->key_fields.setElt(self->fldName(elt->fld), node.val.i);
                            }
                        } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                            // we can safely copy from JSON because the number format is safe (no leading zeroes etc.)
                            const char * value = node.val_raw;
                            udx value_len = node.val_raw_len;
                            propset_success_cnt = self->propSet(prop_obj, elt->fld, value_path, value, value_len);
                        }
                        break;
                    case sJSONParser::eValueReal:
                        // copy from JSON source to avoid rounding round-trip issues
                        if( self->_second_pass_mode == eSecondPass_Pretend ) {
                            self->printPretendValue(node.val_raw, node.val_raw_len);
                        } else if( self->_second_pass_mode == eSecondPass_AutoUpsert ) {
                            if( self->fldIsKey(elt->fld) && !self->fldIsGlobalMulti(elt->fld) ) {
                                prop_obj->key_fields.setElt(self->fldName(elt->fld), node.val_raw, node.val_raw_len);
                            }
                        } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                            const char * value = node.val_raw;
                            udx value_len = node.val_raw_len;
                            propset_success_cnt = self->propSet(prop_obj, elt->fld, value_path, value, value_len);
                        }
                        break;
                    case sJSONParser::eValueString:
                        if( self->_second_pass_mode == eSecondPass_Pretend ) {
                            self->printPretendValue(node.val.str, node.val_str_len);
                        } else if( self->_second_pass_mode == eSecondPass_AutoUpsert ) {
                            if( self->fldIsKey(elt->fld) && !self->fldIsGlobalMulti(elt->fld) ) {
                                prop_obj->key_fields.setElt(self->fldName(elt->fld), node.val.str, node.val_str_len);
                            }
                        } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                            const char * value = node.val.str;
                            udx value_len = node.val_str_len;
                            propset_success_cnt = self->propSet(prop_obj, elt->fld, value_path, value, value_len);
                        }
                        break;
                    default:
                        break; // should not happen
                }

                if( self->_second_pass_mode == eSecondPass_PropSet && !propset_success_cnt ) {
                    parser.setValueError(node, "Failed setting %s property", self->fldName(elt->fld));
                    return;
                }
            }
            break;
            case eMode_file: {
                if( node.value_type == sJSONParser::eValueEndOfContainer ) {
                    // not relevant for files
                    return;
                }
                if( node.depth == 2 ) {
                    if( node.value_type != sJSONParser::eValueArray ) {
                        parser.setValueError(node, "_file is expected to be an array");
                        return;
                    }
                } else {
                    if( node.value_type != sJSONParser::eValueString ) {
                        parser.setValueError(node, "_file is expected to be an array of strings");
                        return;
                    }

                    if( !self->_file_path_buf ) {
                        parser.setValueError(node, "_file loading disabled in this mode");
                        return;
                    }

                    if( self->_second_pass_mode == eSecondPass_Pretend ) {
                        self->printPretendField(cur_obj_str, "_file", node.index + 1);
                        self->printPretendValue(node.val.str, node.val_str_len);
                    } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                        param->path_buf.cutAddString(0, self->_file_path_buf);
                        param->tmp_file_buf.cut0cut();
                        if( printRelativeFilePath(param->path_buf, node.val.str, node.val_str_len, param->tmp_file_buf) ) {
                            const char * src = param->path_buf.ptr(0);
                            const char * name = sFilePath::nextToSlash(param->path_buf);
                            AddedFile * added = self->_added_files.add(1);
                            added->uobj = prop_obj->uobj;
                            added->trashed = false;
                            added->dst_file_pos = -sIdxMax;
                            self->_added_files_buf.add0();
                            added->name_pos = self->_added_files_buf.length();
                            self->_added_files_buf.addString(name);
                            self->_added_files_buf.add0();

                            idx potential_dst_pos = self->_added_files_buf.length();

                            if( prop_obj->uobj->getFilePathname(self->_added_files_buf, "%s", name) ) {
                                // back up existing file out of the way
                                if( prop_obj->uobj->trashFilePathname(self->_added_files_buf, true, "%s", name) ) {
                                    added->trashed = true;
                                } else {
                                    parser.setValueError(node, "failed to move file '%s' into trash", name);
                                    self->_added_files_buf.cut(potential_dst_pos);
                                    return;
                                }
                            }

                            self->_added_files_buf.cut(potential_dst_pos);

                            if( const char * dst = prop_obj->uobj->addFilePathname(self->_added_files_buf, false, "%s", name) ) {
                                self->_added_files_buf.add0();

                                if( sDir::exists(src) ) {

                                    if( sDir::copyDir(src, dst, true) ) {
                                        added->dst_file_pos = potential_dst_pos;
                                    } else {
                                        parser.setValueError(node, "failed to copy directory '%s' into object", name);
                                        self->_added_files_buf.cut(potential_dst_pos);
                                        return;
                                    }
                                } else if( sFile::copy(src, dst, false, true) ) {
                                    added->dst_file_pos = potential_dst_pos;
                                } else {
                                    parser.setValueError(node, "failed to copy file '%s' into object", name);
                                    self->_added_files_buf.cut(potential_dst_pos);
                                    return;
                                }
                            } else {
                                parser.setValueError(node, "failed to add '%s' filepath inside object", name);
                                self->_added_files_buf.cut(potential_dst_pos);
                                return;
                            }
                        } else {
                            parser.setValueError(node, "_file path is invalid");
                            return;
                        }
                    }
                }
            }
            break;
            case eMode_perm: {
                if( !self->readPermNode(node, parser) ) {
                    return;
                }
                if( self->cur_perm.state == Perm::eCompleted ) {
                    self->cur_perm.partybuf.addString(",,");
                    permPrettyPrint(self->cur_perm.partybuf, self->cur_perm.acts, self->cur_perm.flags);

                    if( self->_second_pass_mode == eSecondPass_Pretend ) {
                        self->printPretendField(cur_obj_str, "_perm", self->cur_perm.iperm + 1);
                        self->printPretendValue(self->cur_perm.partybuf, self->cur_perm.partybuf.length());
                    } else if( self->_second_pass_mode == eSecondPass_PropSet ) {
                        if( self->cur_perm.group_id ) {
                            if( !self->setPermission(prop_obj, node, self->cur_perm) ) {
                                parser.setKeyError(node, "failed to set permissions for object %s", prop_obj->obj.id.print());
                                return;
                            }
                        } else {
                            parser.setKeyError(node, "refusing to set permission for object %s - user group name or ID was missing or invalid", prop_obj->obj.id.print());
                            if( self->_flags & fInvalidUserGroupNonFatal ) {
                                sStr tmp_str;
                                fprintf(stderr, "Warning: %s\n", parser.printError(tmp_str));
                                parser.clearError();
                            } else {
                                return;
                            }

                        }
                    }
                }
            }
            break;
            default:
                break; // do nothing
        }
    }
}

// virtual
udx sUsrPropSet::getUpdateLevel()
{
    return _usr->getUpdateLevel();
}

// virtual
bool sUsrPropSet::updateStart()
{
    return _usr->updateStart();
}

// virtual
bool sUsrPropSet::hadDeadlocked()
{
    return _usr->hadDeadlocked();
}

// virtual
bool sUsrPropSet::updateAbandon()
{
    return _usr->updateAbandon();
}

// virtual
bool sUsrPropSet::updateComplete()
{
    return _usr->updateComplete();
}

// virtual
bool sUsrPropSet::propInit(ObjLoc * prop_obj)
{
    return prop_obj->uobj->propInit();
}

// virtual
udx sUsrPropSet::propSet(sUsrPropSet::ObjLoc * prop_obj, const sUsrPropSet::TypeField * fld, const char * path, const char * value, udx value_len)
{
    return prop_obj->uobj->propSet(fldName(fld), &path, &value, 1, true, 0, &value_len);
}

// virtual
bool sUsrPropSet::setPermission(sUsrPropSet::ObjLoc * prop_obj, sJSONParser::ParseNode & node, sUsrPropSet::Perm & perm)
{
    return _usr->setPermission(perm.group_id, prop_obj->obj.id, perm.acts, perm.flags);
}

// virtual
const char * sUsrPropSet::fldName(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->name();
}

// virtual
sUsrTypeField::EType sUsrPropSet::fldType(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->type();
}

// virtual
bool sUsrPropSet::fldCanHaveValue(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->canHaveValue();
}

// virtual
bool sUsrPropSet::fldCanSetValue(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->canSetValue();
}

// virtual
bool sUsrPropSet::fldIsArrayRow(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->isArrayRow();
}

// virtual
bool sUsrPropSet::fldIsKey(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->isKey();
}

// virtual
bool sUsrPropSet::fldIsMulti(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->isMulti();
}

// virtual
bool sUsrPropSet::fldIsGlobalMulti(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->isGlobalMulti();
}

// virtual
const sUsrPropSet::TypeField * sUsrPropSet::fldGet(sUsrPropSet::ObjLoc * prop_obj, const char * name, idx name_len) const
{
    return prop_obj->obj.utype ? prop_obj->obj.utype->getField(*_usr, name, name_len) : 0;
}

//virtual
const sUsrPropSet::TypeField * sUsrPropSet::fldParent(const sUsrPropSet::TypeField * fld) const
{
    return static_cast<const sUsrTypeField*>(fld)->parent();
}

//virtual
const sUsrPropSet::TypeField * sUsrPropSet::fldFlattenedNonArrayRowParent(const sUsrPropSet::TypeField * fld) const
{
    const sUsrTypeField * par = fld ? static_cast<const sUsrTypeField*>(fld)->flattenedParent() : 0;
    if( par && par->isArrayRow() ) {
        par = par->parent();
        // See sUsrType2::isFlattenedDecor() : if array row is non-decorative,
        // then the array itself must be non-decorative. Verify for sanity.
        assert(par && par->type() == sUsrTypeField::eArray && !par->isFlattenedDecor());
    }
    return par;
}

// check if the current node is inside a comment; update _cur_icomment appropriately (will be set to >= 0 if in a comment); return true if inside a comment
bool sUsrPropSet::inComment(sJSONParser::ParseNode & node)
{
    if( _cur_icomment >= 0 && (node.depth < _comment_loc[_cur_icomment].depth || (node.depth == _comment_loc[_cur_icomment].depth && node.index > _comment_loc[_cur_icomment].index) ) ) {
        // we have either moved closed to json root, or to a sibling json value - either way, we've left the comment
        _cur_icomment = -sIdxMax;
    }

    idx potential_icomment = -sIdxMax;
    // if we were not previously in a comment (or just left a comment), check whether we are now in one
    if( _cur_icomment < 0 && _comment_loc.get(&node.val_pos, sizeof(node.val_pos), &potential_icomment) ) {
        _cur_icomment = potential_icomment;
    }

    return (_cur_icomment >= 0);
}

sUsrPropSet::ESubObjMode sUsrPropSet::readSubmode(sJSONParser::ParseNode & node, sJSONParser & parser)
{
    if( node.key_str ) {
        if( node.key_str[0] != '_' ) {
            return eMode_field;
        }
        if( strncmp_exact("_id", 3, node.key_str, node.key_str_len) == 0 ) {
            return eMode_id;
        }
        if( strncmp_exact("_type", 5, node.key_str, node.key_str_len) == 0 ) {
            return eMode_type;
        }
        if( strncmp_exact("_file", 5, node.key_str, node.key_str_len) == 0 ) {
            return eMode_file;
        }
        if( strncmp_exact("_perm", 5, node.key_str, node.key_str_len) == 0 ) {
            return eMode_perm;
        }
    }
    return eMode_other;
}

bool sUsrPropSet::checkCurObjSanity(sJSONParser::ParseNode & node, sJSONParser & parser)
{
    ObjLoc * prop_obj = _objs.ptr(_cur_iobj);
    if( !prop_obj->obj.utype ) {
        parser.setValueError(node, "missing or invalid _type for object");
        return false;
    }
    if( _second_pass_mode != eSecondPass_AutoUpsert ) {
        if( !prop_obj->obj.id && !prop_obj->obj.is_new ) {
            parser.setValueError(node, "missing or invalid _id for object");
            return false;
        }
        if( !prop_obj->uobj ) {
            if( _second_pass_mode == eSecondPass_PropSet && prop_obj->obj.is_new && !prop_obj->obj.id ) {
                if( sRC rc = _usr->objCreate(prop_obj->obj.id, getUTypeName(prop_obj)) ) {
                    prop_obj->obj.id.reset();
                    parser.setValueError(node, "failed %s", rc.print());
                    return false;
                }
            }
            if( _second_pass_mode == eSecondPass_PropSet || !prop_obj->obj.is_new ) {
                prop_obj->uobj = _usr->objFactory(prop_obj->obj.id, &getUTypeId(prop_obj), ePermCanWrite);

                bool uobj_can_be_null = false;

                if( !prop_obj->uobj && prop_obj->obj.id.domainId() && prop_obj->obj.id.objId() ) {
                    // for objects with non-0 domainID, if the object does not exist, we try to create it
                    // TODO: switch to make this behavior controllable
                    if( _second_pass_mode == eSecondPass_Pretend ) {
                        // pretend we can create such an object!
                        uobj_can_be_null = true;
                    } else {
                        sHiveId expected_id = prop_obj->obj.id;
                        if( sRC rc = _usr->objCreate(prop_obj->obj.id, getUTypeName(prop_obj), prop_obj->obj.id.domainId(), prop_obj->obj.id.objId()) ) {
                            parser.setValueError(node, "failed %s", rc.print());
                            return false;
                        }
                        prop_obj->uobj = _usr->objFactory(prop_obj->obj.id, &getUTypeId(prop_obj), ePermCanWrite);
                        if( prop_obj->obj.id != expected_id ) {
                            // should not happen unless objCreate() logic failure
                            sStr err_buf;
                            parser.setValueError(node, "failed to create object with id %s (got %s instead)", expected_id.print(), prop_obj->obj.id.print(err_buf));
                            return false;
                        }
                    }
                }

                if( !uobj_can_be_null && !prop_obj->uobj ) {
                    parser.setValueError(node, "object with id %s either does not exist, cannot be modified, or is of the wrong object type (expected type is %s)", prop_obj->obj.id.print(), getUTypeName(prop_obj));
                    return false;
                }
            }
        }
    }
    return true;
}

bool sUsrPropSet::readFieldNode(sJSONParser::ParseNode & node, sJSONParser & parser)
{
    // pop field_stack as necessary to match depth
    for(idx is = _field_stack.dim() - 1; is >= 0; is--) {
        if( node.depth <= _field_stack[is].depth ) {
            _field_stack.cut(is);
        } else if( node.depth == _field_stack[is].multi_depth ) {
            _field_stack[is].multi_depth = _field_stack[is].row_depth = -sIdxMax;
            _field_stack[is].imulti = _field_stack[is].irow = -sIdxMax;
        } else if( node.depth == _field_stack[is].row_depth ) {
            _field_stack[is].row_depth = -sIdxMax;
            _field_stack[is].irow = -sIdxMax;
        } else {
            break;
        }
    }

    ObjLoc * prop_obj = _objs.ptr(_cur_iobj);

    idx node_array_index = -sIdxMax;
    if( node.node_type == sJSONParser::eNodeArrayElement ) {
        node_array_index = node.index;
    } else if( node.node_type == sJSONParser::eNodeObjectElement && isdigit(node.key_str[0]) ) {
        // TODO - operation order @ syntax in key
        char * end = 0;
        const char * haystack = node.key_str;
        // guaranteed to be safe because key terminates with '"' in original JSON buffer
        while( haystack ) {
            node_array_index = strtoidx(haystack, &end, 10);
            if( end && end[0] == '.' ) {
                // Backwards compatibility hack: in old cmd=propget&mode=json output format,
                // JSON key is the full dot-delimeted path to this element. We want the last part.
                haystack = end + 1;
                continue;
            } else if( !end || end - node.key_str != node.key_str_len ) {
                // not an integer in this segment
                node_array_index = -sIdxMax;
                haystack = 0;
            } else {
                // integer found and nothing more to search
                haystack = 0;
            }
        }
    }

    if( node_array_index >= 0 ) {
        if( !_field_stack.dim() ) {
            parser.setKeyError(node, "unknown field for type %s", getUTypeName(prop_obj));
            return false;
        }
        FieldStackElt * elt = _field_stack.ptr(_field_stack.dim() - 1);
        if( fldIsMulti(elt->fld) && elt->imulti < 0 ) {
            if( elt->imulti >= 0 ) {
                parser.setKeyError(node, "unexpected multi-value inside multi-value");
                return false;
            }
            elt->imulti = node_array_index;
            elt->multi_depth = node.depth;
        } else if( fldType(elt->fld) == sUsrTypeField::eArray ) {
            if( elt->irow >= 0 ) {
                parser.setValueError(node, "unexpected array row inside array row");
                return false;
            }
            elt->irow = node_array_index;
            elt->row_depth = node.depth;
        } else {
            parser.setKeyError(node, "unexpected multi-value for single-valued field '%s'", fldName(elt->fld));
            return false;
        }
    } else {
        // new field!
        const TypeField * fld = (node.node_type == sJSONParser::eNodeObjectElement) ? fldGet(prop_obj, node.key_str, node.key_str_len) : 0;
        if( !fld ) {
            parser.setKeyError(node, "unknown field for type %s", getUTypeName(prop_obj));
            return false;
        }
        bool is_flattened = false;
        if( fldNeedsValidation(fld) ) {
            // parent field must be either parent or flattened-parent of the new field
            const TypeField * parent_fld = _field_stack.dim() ? _field_stack[_field_stack.dim() - 1].fld : 0;
            const TypeField * expected_parent_fld = fldParent(fld);
            if( expected_parent_fld && fldIsArrayRow(expected_parent_fld) ) {
                expected_parent_fld = fldParent(expected_parent_fld);
            }
            if( expected_parent_fld == parent_fld ) {
                is_flattened = false;
            } else if( fldFlattenedNonArrayRowParent(fld) == parent_fld ) {
                is_flattened = true;
            } else {
                const TypeField * expected_flattened_parent_fld = fldFlattenedNonArrayRowParent(fld);
                if( expected_flattened_parent_fld == expected_parent_fld ) {
                    expected_flattened_parent_fld = 0;
                }
                parser.setKeyError(node, "invalid or missing parent field; expected %s%s%s", expected_parent_fld ? fldName(expected_parent_fld) : "no parent", expected_flattened_parent_fld ? " or " : "", expected_flattened_parent_fld ? fldName(expected_flattened_parent_fld) : "");
                return false;
            }
        }
        FieldStackElt * elt = _field_stack.add(1);
        elt->fld = fld;
        elt->depth = node.depth;
        elt->is_flattened = is_flattened;
        elt->imulti = elt->irow = -sIdxMax;
        elt->multi_depth = elt->row_depth = -sIdxMax;
    }

#if 0
    fprintf(stderr, "fld,depth,imulti,multi_depth,irow,row_depth,is_flattened\n");
    for(idx i = 0; i < _field_stack.dim(); i++) {
        FieldStackElt * elt = _field_stack.ptr(i);
        fprintf(stderr, "%s,%"DEC",%"DEC",%"DEC",%"DEC",%"DEC",%s\n", elt->fld->name(), elt->depth, elt->imulti, elt->multi_depth, elt->irow, elt->row_depth, elt->is_flattened ? "true" : "false");
    }
    fprintf(stderr, "\n");
#endif

    return true;
}

udx sUsrPropSet::getGroupId(const char * grp_name)
{
    // TODO - add a way to clear the _group_ids cache?

    sStr case_buf;
    if( !_group_ids.dim() ) {
        sVec<sStr> table;
        _usr->listGrp(&table, 0, 0, 0, true, true);
        for(idx i = 0; i < table.dim(); i++) {
            const char * user_name = table[i].ptr();
            const char * group_path = sString::next00(user_name);
            const char * group_id_str = sString::next00(group_path);
            udx group_id = group_id_str ? atoudx(group_id_str) : 0;
            if( group_id && group_path ) {
                case_buf.cut(0);
                *_group_ids.set(canonicalCase(case_buf, group_path)) = group_id;
            }
        }
    }

    case_buf.cut(0);
    grp_name = canonicalCase(case_buf, grp_name);
    if( const udx * pgroup_id = _group_ids.get(grp_name) ) {
        return *pgroup_id;
    }
    return 0;
}

bool sUsrPropSet::readPermNode(sJSONParser::ParseNode & node, sJSONParser & parser)
{
    if( node.depth == 2 ) {
        if( node.value_type != sJSONParser::eValueArray ) {
            parser.setValueError(node, "array of permission objects expected");
            return false;
        }
        cur_perm.state = Perm::eInOther;
    } else if( node.depth == 3 ) {
        if( node.value_type != sJSONParser::eValueObject && node.value_type != sJSONParser::eValueEndOfContainer ) {
            parser.setValueError(node, "permission object expected");
            return false;
        }
        // initialize cur_perm
        cur_perm.iperm = node.index;
        cur_perm.partybuf.cut0cut();
        cur_perm.group_id = 0;
        cur_perm.acts = 0;
        cur_perm.acts_enabled = true;
        cur_perm.flags = 0;
        cur_perm.state = Perm::eInOther;
    } else if( node.depth == 4 && node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("party", 5, node.key_str, node.key_str_len) == 0 ) {
        if( node.value_type == sJSONParser::eValueString ) {
            cur_perm.partybuf.cutAddString(0, node.val.str, node.val_str_len);
            cur_perm.group_id = getGroupId(cur_perm.partybuf);
            if( !cur_perm.group_id ) {
                parser.setValueError(node, "user group '%s' was not found", cur_perm.partybuf.ptr());
                if( _flags & fInvalidUserGroupNonFatal ) {
                    sStr tmp_str;
                    fprintf(stderr, "Warning: %s", parser.printError(tmp_str));
                    parser.clearError();
                } else {
                    return false;
                }
            }
        } else if( node.value_type == sJSONParser::eValueInt ) {
            // numeric group ID
            cur_perm.partybuf.cut0cut();
            cur_perm.partybuf.addNum(node.val.i);
            if( node.val.i > 0 ) {
                cur_perm.group_id = node.val.i;
            } else {
                parser.setValueError(node, "user group ID %"DEC" is invalid", node.val.i);
                if( _flags & fInvalidUserGroupNonFatal ) {
                    sStr tmp_str;
                    fprintf(stderr, "Warning: %s", parser.printError(tmp_str));
                    parser.clearError();
                } else {
                    return false;
                }
            }
        } else {
            parser.setValueError(node, "string or integer value with party (user group) name expected");
            return false;
        }
    } else if( node.depth == 4 && node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("act", 3, node.key_str, node.key_str_len) == 0 ) {
        if( node.value_type == sJSONParser::eValueObject ) {
            cur_perm.state = Perm::eInAct;
        } else {
            parser.setValueError(node, "expected JSON object mapping permission to 0/1 or true/false");
            return false;
        }
    } else if( node.depth == 5 && node.node_type == sJSONParser::eNodeObjectElement && cur_perm.state == Perm::eInAct ) {
        if( node.value_type == sJSONParser::eValueEndOfContainer ) {
            cur_perm.state = Perm::eInOther;
        } else {
            if( node.value_type != sJSONParser::eValueBool && node.value_type != sJSONParser::eValueInt && node.val.i != 0 && node.val.i != 1 ) {
                parser.setValueError(node, "expected 0/1 or true/false");
                return false;
            }
            bool acts_enabled = node.val.i;
            if( cur_perm.flags && acts_enabled != cur_perm.acts_enabled ) {
                parser.setValueError(node, "due to technical limitations of current permission model, all actions must be either disabled or enabled");
                return false;
            }
            cur_perm.acts_enabled = acts_enabled;
            if( !acts_enabled ) {
                cur_perm.flags |= eFlagRestrictive;
            }

            udx act = permPermParse(node.key_str, node.key_str_len);
            if( !act ) {
                parser.setKeyError(node, "unknown permission name");
                return false;
            }

            cur_perm.acts |= act;
        }
    } else if( node.depth == 4 && node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("_infect", 7, node.key_str, node.key_str_len) == 0 ) {
        if( node.value_type == sJSONParser::eValueObject ) {
            cur_perm.state = Perm::eInInfect;
            cur_perm.flags |= eFlagInheritDown;
        } else {
            parser.setValueError(node, "expected { \"party\": [ \"member\" ] }");
            return false;
        }
    } else if( node.depth == 4 && node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("_upfect", 7, node.key_str, node.key_str_len) == 0 ) {
        if( node.value_type == sJSONParser::eValueObject ) {
            cur_perm.state = Perm::eInUpfect;
            cur_perm.flags |= eFlagInheritUp;
        } else {
            parser.setValueError(node, "expected { \"party\": [ \"member\" ] }");
            return false;
        }
    } else if( cur_perm.state == Perm::eInInfect || cur_perm.state == Perm::eInUpfect ) {
        if( node.depth == 5 && node.value_type == sJSONParser::eValueEndOfContainer ) {
            cur_perm.state = Perm::eInOther;
            return true;
        } else if( node.depth == 5 && node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("party", 5, node.key_str, node.key_str_len) == 0 && node.value_type == sJSONParser::eValueArray ) {
            return true;
        } else if( node.depth == 6 && node.node_type == sJSONParser::eNodeArrayElement && node.value_type == sJSONParser::eValueString && strncmp_exact("member", 6, node.val.str, node.val_str_len) == 0 ) {
            return true;
        } else if( node.depth == 6 && node.node_type == sJSONParser::eNodeArrayElement && node.value_type == sJSONParser::eValueEndOfContainer ) {
            return true;
        } else {
            parser.setValueError(node, "due to technical limitations of current permission model, _infect/_upfect must map to the literal { \"party\": [ \"member\" ] } JSON object value");
            return false;
        }
    } else if( node.depth == 4 && node.node_type == sJSONParser::eNodeObjectElement && strncmp_exact("flags", 5, node.key_str, node.key_str_len) == 0 ) {
        if( node.value_type == sJSONParser::eValueArray ) {
            cur_perm.state = Perm::eInFlags;
        } else {
            parser.setValueError(node, "expected array of flags");
            return false;
        }
    } else if( node.depth == 5 && node.node_type == sJSONParser::eNodeArrayElement && cur_perm.state == Perm::eInFlags ) {
        if( node.value_type == sJSONParser::eValueEndOfContainer ) {
            cur_perm.state = Perm::eInOther;
        } else if( node.value_type == sJSONParser::eValueString ) {
            udx flags = permFlagParse(node.key_str, node.key_str_len);

            if( !flags ) {
                parser.setValueError(node, "unknown or invalid flag");
                return false;
            }
            if( flags & eFlagRestrictive ) {
                parser.setValueError(node, "deny flag is expected to be set via \"act\" JSON construct");
                return false;
            }
            if( flags & eFlagInheritDown ) {
                parser.setValueError(node, "deny flag is expected to be set via \"_infect\" JSON construct");
                return false;
            }
            if( flags & eFlagInheritUp ) {
                parser.setValueError(node, "deny flag is expected to be set via \"_upfect\" JSON construct");
                return false;
            }

            cur_perm.flags |= flags;
        } else {
            parser.setValueError(node, "string with flag name expected");
            return false;
        }
    } else if( node.depth == 4 && node.value_type == sJSONParser::eValueEndOfContainer ) {
        if( !cur_perm.partybuf.length() ) {
            parser.setValueError(node, "party (user group) was not specified");
            return false;
        }
        if( !cur_perm.acts ) {
            parser.setValueError(node, "act (list of permissions) was not specified");
            return false;
        }
        cur_perm.state = Perm::eCompleted;
    } else {
        parser.setKeyError(node, "invalid JSON _perm structure syntax");
        return false;
    }

    return true;
}

bool sUsrPropSet::readUpsertNode(sUsrPropSet::Obj & out_obj, sJSONParser::ParseNode & node, sJSONParser & parser)
{
    // $upsert_qry(<query language expression>)
    const char * qry = node.val.str + 12; // strlen("$upsert_qry(")
    idx qry_len = node.val_str_len - 13; // strlen("$upsert_qry(") + strlen(")")

    qlang::sUsrEngine query_engine(*_usr);
    sStr error_buf;
    if( !query_engine.parse(qry, qry_len, &error_buf) ) {
        parser.setValueError(node, "failed to parse $upsert_qry()");
        return false;
    }
    sVariant query_result;
    if( !query_engine.eval(query_result, &error_buf) ) {
        parser.setValueError(node, "failed to run $upsert_qry()");
        return false;
    }
    if( query_result.isScalar() ) {
        if( query_result.isHiveId() ) {
            query_result.asHiveId(&out_obj.id);
            return true;
        } else if( query_result.isNull() || (query_result.isInt() && !query_result.asBool()) ) {
            out_obj.id.reset();
            out_obj.is_new = true;
            return true;
        }
    } else if( query_result.isList() ) {
        if( query_result.dim() == 1 ) {
            query_result.getListElt(0)->asHiveId(&out_obj.id);
            return true;
        } else if( query_result.dim() == 0 ) {
            out_obj.id.reset();
            out_obj.is_new = true;
            return true;
        }
    }

    parser.setValueError(node, "$upsert_qry() returned unexpected or non-unique results");
    return false;
}

// virtual
const char * sUsrPropSet::printIntermediateFlattenedPath(sStr & out, const sUsrPropSet::TypeField * fld, const sUsrPropSet::TypeField * ancestor) const
{
    idx start = out.length();
    for( const TypeField * parent = fldParent(fld); parent && (!ancestor || parent != ancestor); parent = fldParent(parent) ) {
        if( !fldIsArrayRow(parent) ) {
            out.addString("1.", 2);
        }
    }
    return out.ptr(start);
}

// path element format:
// arbitrary integer (default record viewer behavior: field order) for scalar fields without array parent;
// unique integers (per-field) for multi-valued fields;
// array row index for elements of array rows
const char * sUsrPropSet::printPropPath(sStr & out)
{
    idx start = out.length();
    for(idx is = 0; is < _field_stack.dim(); is++) {
        if( _field_stack[is].imulti >= 0 ) {
            if( _field_stack[is].is_flattened ) {
                printIntermediateFlattenedPath(out,  _field_stack[is].fld, is ? _field_stack[is - 1].fld : 0);
            }
            out.addNum(_field_stack[is].imulti);
            out.addString(".", 1);
        } else if( is && _field_stack[is - 1].irow >= 0 ) {
            out.addNum(_field_stack[is - 1].irow);
            out.addString(".", 1);
            if( _field_stack[is].is_flattened ) {
                printIntermediateFlattenedPath(out,  _field_stack[is].fld, _field_stack[is - 1].fld);
            }
        } else {
            if( _field_stack[is].is_flattened ) {
                printIntermediateFlattenedPath(out,  _field_stack[is].fld, is ? _field_stack[is - 1].fld : 0);
            }
            out.addString("1.", 2);
        }
    }
    // remove last '.'
    out.cut0cut(out.length() - 1);
    return out.ptr(start);
}

bool sUsrPropSet::pretend(sVarSet & out, sDic<sUsrPropSet::Obj> * modified_objs/* = 0 */, udx flags/* = 0 */)
{
    return pretendInternal(&out, 0, modified_objs, flags);
}

bool sUsrPropSet::pretendPropFmt(sStr & out, sDic<sUsrPropSet::Obj> * modified_objs/* = 0 */, udx flags/* = 0 */)
{
    return pretendInternal(0, &out, modified_objs, flags);
}

bool sUsrPropSet::run(sDic<sUsrPropSet::Obj> * modified_objs/* = 0*/, udx flags/* = 0 */)
{
    if( _err ) {
        return false;
    }

    _flags = flags;
    _buf.cut(0);
    _objs.cut(0);
    _cur_iobj = -sIdxMax;
    _cur_icomment = -sIdxMax;
    _propnames.empty();
    _keys_tic.empty();
    _comment_loc.cut(0);
    _added_files_buf.cut0cut();
    _added_files.cut(0);
    JsonSecondPassCbParam run_param;

    _second_pass_mode = eSecondPass_PropSet;
    _var_set_out = 0;
    _prop_fmt_out = 0;

    sJSONParser parser;

    parser.setParseCallback(jsonFirstPassCb, this, true, &_buf);
    if( !parser.parse(_srcbuf, _srclen, 0, 0) ) {
        parser.printError(_err);
        _second_pass_mode = eSecondPass_Pretend;
        return false;
    }

    run_param.self = this;
    parser.setParseCallback(jsonSecondPassCb, &run_param, true, &_buf);

    // if any objects in the json are being upserted by key prop & type, we need another pass
    // to check if their object ids exist in the database
    for(idx i = 0; i < _objs.dim(); i++) {
        if( _objs[i].is_auto_upsert ) {
            _second_pass_mode = eSecondPass_AutoUpsert;
            break;
        }
    }
    if( _second_pass_mode == eSecondPass_AutoUpsert ) {
        _cur_iobj = -sIdxMax;
        _cur_icomment = -sIdxMax;
        run_param.self = this;
        if( !parser.parse(_srcbuf, _srclen, 0, 0) ) {
            parser.printError(_err);
            _second_pass_mode = eSecondPass_Pretend;
            return false;
        }
        _second_pass_mode = eSecondPass_PropSet;
    }

    for(idx itry = 0; itry < sSql::max_deadlock_retries; itry++) {
        _cur_iobj = -sIdxMax;
        _cur_icomment = -sIdxMax;
        bool is_our_transaction = !getUpdateLevel();
        if( !updateStart() ) {
            if( _usr->db().HasFailed() ) {
                _err.printf(0, "DB error %"UDEC" : %s", _usr->db().Get_errno(), _usr->db().Get_error().ptr());
            } else {
                _err.printf(0, "Unknown DB error");
            }
            return false;
        }
        if( !parser.parse(_srcbuf, _srclen, 0, 0) ) {
            // roll back newly copied files (from both modified and new objects)
            sStr tmp_buf;
            for(idx i = 0; i < _added_files.dim(); i++) {
                if( _added_files[i].dst_file_pos >= 0 ) {
                    const char * dst_file = _added_files_buf.ptr(_added_files[i].dst_file_pos);
                    if( sDir::exists(dst_file) ) {
                        sDir::removeDir(dst_file);
                    } else {
                        sFile::remove(dst_file);
                    }
                }
                if( _added_files[i].name_pos >= 0 && _added_files[i].trashed && _added_files[i].uobj ) {
                    tmp_buf.cut(0);
                    _added_files[i].uobj->restoreFilePathname(tmp_buf, false, "%s", _added_files_buf.ptr(_added_files[i].name_pos));
                }
            }
            // purge newly created objects and their storage directories
            for(idx i = 0; i < _objs.dim(); i++) {
                if( _objs[i].obj.is_new && _objs[i].uobj && _objs[i].uobj->Id() ) {
                    _objs[i].uobj->purge();
                }
            }

            // DB deadlock detected, and our own g_user->updateStart() call had
            // started the current DB transaction. Save this fact before updateAbandon().
            bool is_our_deadlock = hadDeadlocked() && is_our_transaction;

            // roll back DB
            _usr->updateAbandon();

            _buf.cut(0);
            _objs.cut(0);
            _propnames.empty();
            _keys_tic.empty();
            _comment_loc.cut(0);
            _added_files_buf.cut0cut();
            _added_files.cut(0);

            if( is_our_deadlock ) {
                // DB deadlock was detected, and our own g_user->updateStart() call had
                // started the current DB transaction. After cleanup, wait a bit and retry.
                _err.cut0cut();
#if UPROPSET_TRACE
                fprintf(stderr, "%s:%u - restarting deadlocked transaction, attempt %"DEC"/%"DEC"\n", __FILE__, __LINE__, itry + 1, sSql::max_deadlock_retries);
#endif
                sTime::randomSleep(sSql::max_deadlock_wait_usec);
                continue;
            } else {
                parser.printError(_err);
                _second_pass_mode = eSecondPass_Pretend;
                return false;
            }
        }
        updateComplete();
        break;
    }
    _second_pass_mode = eSecondPass_Pretend;

    if( modified_objs ) {
        for(idx i = 0; i < _objs.dim(); i++) {
            idx key_len = 0;
            const char * key = static_cast<const char *>(_objs.id(i, &key_len));
            *modified_objs->set(key, key_len) = _objs[i].obj;
        }
    }

    return true;
}

bool sUsrPropSet::pretendInternal(sVarSet * var_set_out, sStr * prop_fmt_out, sDic<sUsrPropSet::Obj> * modified_objs, udx flags/* = 0*/)
{
    if( _err ) {
        return false;
    }

    if( var_set_out && var_set_out->rows == 0 ) {
        var_set_out->setColId(0, "id");
        var_set_out->setColId(1, "name");
        var_set_out->setColId(2, "path");
        var_set_out->setColId(3, "value");
    }

    _flags = flags;
    _buf.cut(0);
    _objs.cut(0);
    _cur_iobj = -sIdxMax;
    _cur_icomment = -sIdxMax;
    _propnames.empty();
    _keys_tic.empty();
    _comment_loc.cut(0);
    _added_files_buf.cut0cut();
    _added_files.cut(0);
    JsonSecondPassCbParam pretend_param;

    bool ret = false;
    _second_pass_mode = eSecondPass_Pretend;
    _var_set_out = var_set_out;
    _prop_fmt_out = prop_fmt_out;

    sJSONParser parser;
    parser.setParseCallback(jsonFirstPassCb, this, true, &_buf);
    if( !parser.parse(_srcbuf, _srclen, 0, 0) ) {
        parser.printError(_err);
        goto CLEANUP;
    }

    pretend_param.self = this;
    parser.setParseCallback(jsonSecondPassCb, &pretend_param, true, &_buf);

    // if any objects in the json are being upserted by key prop & type, we need another pass
    // to check if their object ids exist in the database
    for(idx i = 0; i < _objs.dim(); i++) {
        if( _objs[i].is_auto_upsert ) {
            _second_pass_mode = eSecondPass_AutoUpsert;
            break;
        }
    }
    if( _second_pass_mode == eSecondPass_AutoUpsert ) {
        _cur_iobj = -sIdxMax;
        _cur_icomment = -sIdxMax;
        if( !parser.parse(_srcbuf, _srclen, 0, 0) ) {
            parser.printError(_err);
            _second_pass_mode = eSecondPass_Pretend;
            goto CLEANUP;
        }
        _second_pass_mode = eSecondPass_Pretend;
    }

    _cur_iobj = -sIdxMax;
    _cur_icomment = -sIdxMax;
    if( !parser.parse(_srcbuf, _srclen, 0, 0) ) {
        parser.printError(_err);
        goto CLEANUP;
    }

    if( modified_objs ) {
        for(idx i = 0; i < _objs.dim(); i++) {
            idx key_len = 0;
            const char * key = static_cast<const char *>(_objs.id(i, &key_len));
            *modified_objs->set(key, key_len) = _objs[i].obj;
        }
    }

    ret = true;

  CLEANUP:
    _second_pass_mode = eSecondPass_Pretend;
    _var_set_out = 0;
    _prop_fmt_out = 0;
    return ret;
}

bool sUsrPropSet::autoUpsertFindId(sJSONParser::ParseNode & node, sJSONParser & parser, sUsrPropSet::ObjLoc * prop_obj)
{
    sStr type_regex_buf, query_buf, error_buf;
    sVariant query_result, parsed_value, expected_value;
    qlang::sUsrEngine query_engine(*_usr);

    // alloftype("my_type", {"key_prop1": "val1", "key_prop2": 12345})

    type_regex_buf.addString("^");
    type_regex_buf.addString(getUTypeName(prop_obj));
    type_regex_buf.addString("$");

    sUsrObjRes res;
    sVariant * prop_filter = prop_obj->key_fields.isDic() && prop_obj->key_fields.dim() ? &prop_obj->key_fields : 0;
    sVariant res_props;
    res_props.setList();
    sVec<const sUsrTypeField *> key_fields;
    for(idx ifld = 0; ifld < prop_obj->obj.utype->dimFields(*_usr); ifld++) {
        const sUsrTypeField * fld = prop_obj->obj.utype->getField(*_usr, ifld);
        if( fld->isKey() && !fld->isGlobalMulti() ) {
            res_props.push(fld->name());
            *key_fields.add(1) = fld;
        }
    }
    query_engine.getContext().getAllObjsOfType(query_result, type_regex_buf, prop_filter, &res, &res_props);

    if( query_result.isNull() ) {
        parser.setValueError(node, "failed automatic query for $upsert()");
        return false;
    }

    sVec<sHiveId> matching_ids;
    // getAllObjsOfType matches by *disjunction* of prop_filter; we need conjunction, and to check empty fields
    for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
        const sHiveId * id = res.id(it);
        const sUsrObjRes::TObjProp * obj = res.get(it);
        if( id && obj ) {
            bool matches = true;
            for(idx ifld = 0; ifld < key_fields.dim(); ifld++) {
                const char * value = 0;
                if( const sUsrObjRes::TPropTbl * tbl = res.get(*obj, key_fields[ifld]->name()) ) {
                    value = res.getValue(tbl);
                }
                if( !value ) {
                    value = sStr::zero;
                }

                parsed_value.setNull();
                key_fields[ifld]->parseValue(parsed_value, value, 0);
                expected_value.setNull();
                if( sVariant * v = prop_filter ? prop_filter->getDicElt(key_fields[ifld]->name()) : 0 ) {
                    expected_value = *v;
                }
                if( parsed_value != expected_value ) {
                    matches = false;
                    break;
                }
            }
            if( matches ) {
                *matching_ids.add(1) = *id;
            }
        }
    }

    if( matching_ids.dim() == 0 ) {
        // no matching objects found - upsert() is an insert
        prop_obj->obj.is_new = true;
    } else if( matching_ids.dim() == 1 ) {
        // exactly 1 matching object found - upsert() is an update
        prop_obj->obj.is_new = false;
        prop_obj->obj.id = matching_ids[0];
    } else {
        // multiple matching objects found - uniqueness constraint violated
        sStr err("automatic query for $upsert() found multiple existing objects (");
        sHiveId::printVec(err, matching_ids, ", ");
        err.addString(") with the same matching value(s) for key fields; cannot update automatically");
        parser.setValueError(node, err.ptr());
        return false;
    }

    return true;
}

void sUsrPropSet::printPretendField(const char * obj_name, const char * fld_name, const char * path)
{
    if( _var_set_out ) {
        _var_set_out->addRow().addCol(obj_name).addCol(fld_name).addCol(path);
    } else if( _prop_fmt_out ) {
        _prop_fmt_out->addString("\nprop.");
        _prop_fmt_out->addString(obj_name);
        _prop_fmt_out->addString(".");
        _prop_fmt_out->addString(fld_name);
        if( path && path[0] ) {
            _prop_fmt_out->addString(".", 1);
            _prop_fmt_out->addString(path);
        }
        _prop_fmt_out->addString("=", 1);
    }
}

void sUsrPropSet::printPretendField(const char * obj_name, const char * fld_name, idx path)
{
    if( _var_set_out ) {
        _var_set_out->addRow().addCol(obj_name).addCol(fld_name).addCol(path);
    } else if( _prop_fmt_out ) {
        _prop_fmt_out->addString("\nprop.");
        _prop_fmt_out->addString(obj_name);
        _prop_fmt_out->addString(".");
        _prop_fmt_out->addString(fld_name);
        _prop_fmt_out->addString(".", 1);
        _prop_fmt_out->addNum(path);
        _prop_fmt_out->addString("=", 1);
    }
}

void sUsrPropSet::printPretendValue(const char * value, idx len)
{
    if( _var_set_out ) {
        _var_set_out->addCol(value, len);
    } else if( _prop_fmt_out ) {
        _prop_fmt_out->addString(value, len);
    }
}

void sUsrPropSet::printPretendValue(idx value)
{
    if( _var_set_out ) {
        _var_set_out->addCol(value);
    } else if( _prop_fmt_out ) {
        _prop_fmt_out->addNum(value);
    }
}
