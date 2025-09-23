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
#ifndef sLib_upropset_h
#define sLib_upropset_h

#include <slib/utils/tic.hpp>
#include <slib/utils/json/parser.hpp>
#include <ulib/usr.hpp>
#include <ulib/utype2.hpp>

namespace slib {
    class sUsrObj;
    class sUsrPropSet {
        public:
            sUsrPropSet();
            sUsrPropSet(const sUsr & usr)
            {
                reset(usr);
            }
            virtual ~sUsrPropSet() {}

            sUsrPropSet & reset(const sUsr & usr);
            sUsrPropSet & setSrc(const char * jsonbuf, idx len = 0);
            sUsrPropSet & setSrcFile(const char * jsonfilename);
            sUsrPropSet & setFileRoot(const char * path, idx len = 0);
            sUsrPropSet & disableFileRoot();

            struct Obj {
                sHiveId id;
                udx in_domain_id;
                const sUsrType2 * utype;
                bool is_new;
                bool overwrite;
            };

            bool run(sDic<Obj> * modified_objs = 0, udx flags = 0);
            bool pretend(sVarSet & out, sDic<Obj> * modified_objs = 0, udx flags = 0);
            bool pretendPropFmt(sStr & out, sDic<Obj> * modified_objs = 0, udx flags = 0);
            const char * getErr() const
            {
                return _err.length() ? _err.ptr() : 0;
            }
            const char * getWarn00()
            {
                if( _warn00 ) {
                    _warn00.add0(3);
                    return _warn00.ptr();
                }
                return 0;
            }

            enum ERunFlags {
                fInvalidUserGroupNonFatal = 1,
                fOverwriteExistingSameType = 2,
                fOverwriteExistingDiffType = 4,
            };

        protected:
            const sUsr * _usr;
            sStr _err;
            sStr _warn00;
            udx _flags;
            sFil _srcfil;
            const char * _srcbuf;
            idx _srclen;

            sStr _buf;
            sStr _file_path_buf;

            sStr _added_files_buf;
            struct AddedFile {
                idx name_pos;
                idx dst_file_pos;
                bool trashed;
                sUsrObj * uobj;
            };
            sVec<AddedFile> _added_files;

            enum {
                eSecondPass_Pretend,
                eSecondPass_AutoUpsert,
                eSecondPass_PropSet
            } _second_pass_mode;
            sVarSet * _var_set_out;
            sStr * _prop_fmt_out;

            struct NodeLoc {
                idx pos;
                idx depth;
                idx index;
                void set(idx p, idx d, idx i)
                {
                    pos = p;
                    depth = d;
                    index = i;
                }
            };
            struct ObjLoc {
                NodeLoc loc;
                Obj obj;
                sVariant key_fields;

                sUsrObj * uobj;
                bool is_auto_upsert;

                ObjLoc();
                ~ObjLoc();
            };

            struct FieldStackElt {
                const sUsrTypeField * fld;
                idx depth;
                idx imulti;
                idx multi_depth;
                idx irow;
                idx row_depth;
                bool is_flattened;
            };

            enum ESubObjMode {
                eMode_other,
                eMode_id,
                eMode_type,
                eMode_field,
                eMode_file,
                eMode_perm
            };
            sDic<ObjLoc> _objs;
            idx _cur_iobj;
            ESubObjMode _cur_submode;
            sDic<idx> _propnames;
            sTic<NodeLoc> _keys_tic;
            sDic<NodeLoc> _comment_loc;
            idx _cur_icomment;
            sVec<FieldStackElt> _field_stack;

            struct Perm {
                sStr partybuf;
                idx iperm;
                udx group_id;
                udx acts;
                udx flags;
                bool acts_enabled;
                enum {
                    eInOther,
                    eInAct,
                    eInInfect,
                    eInUpfect,
                    eInFlags,
                    eCompleted
                } state;
                Perm();
            } cur_perm;

            bool inComment(sJSONParser::ParseNode & node);
            ESubObjMode readSubmode(sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual bool checkCurObjSanity(sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual void readFieldNodeEndOfContainer(sJSONParser::ParseNode & node, sJSONParser & parser) { }
            virtual bool readFieldNode(sJSONParser::ParseNode & node, sJSONParser & parser);
            bool readPermNode(sJSONParser::ParseNode & node, sJSONParser & parser);
            bool readUpsertNode(Obj & out_obj, sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual const char * printPropPath(sStr & out);
            virtual const char * printIntermediateFlattenedPath(sStr & out, const sUsrTypeField * fld, const sUsrTypeField * ancestor) const;
            bool autoUpsertFindId(sJSONParser::ParseNode & node, sJSONParser & parser, ObjLoc * prop_obj);

            bool pretendInternal(sVarSet * var_set_out, sStr * prop_fmt_out, sDic<Obj> * modified_objs = 0, udx flags = 0);
            static void jsonFirstPassCb(sJSONParser::ParseNode & node, sJSONParser & parser, void * param);
            static void jsonSecondPassCb(sJSONParser::ParseNode & node, sJSONParser & parser, void * param);
            void printPretendField(const char * obj_name, const char * fld_name, const char * path);
            void printPretendField(const char * obj_name, const char * fld_name, idx path);
            void printPretendValue(const char * value, idx len=0);
            void printPretendValue(idx value);

            const sUsrTypeField * fldFlattenedNonArrayRowParent(const sUsrTypeField * fld) const;

            virtual bool ensureUTypeFor(ObjLoc * prop_obj, sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual udx getUpdateLevel();
            virtual bool updateStart();
            virtual bool hadDeadlocked();
            virtual bool updateAbandon();
            virtual bool updateComplete();
            virtual bool propInit(ObjLoc * prop_obj);
            virtual udx propSet(ObjLoc * prop_obj, const sUsrTypeField * fld, const char * path, const char * value, udx value_len);
            virtual bool setPermission(ObjLoc * prop_obj, sJSONParser::ParseNode & node, Perm & perm);
            virtual bool fldNeedsValidation(const sUsrTypeField * fld) const { return true; }
    };
};

#endif
