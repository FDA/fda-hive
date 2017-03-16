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
    //! JSON-based HIVE object creator/updater
    class sUsrPropSet {
        public:
            sUsrPropSet();
            sUsrPropSet(const sUsr & usr)
            {
                reset(usr);
            }
            virtual ~sUsrPropSet() {}

            sUsrPropSet & reset(const sUsr & usr);
            //! set JSON input buffer
            sUsrPropSet & setSrc(const char * jsonbuf, idx len = 0);
            //! set filename for JSON input
            sUsrPropSet & setSrcFile(const char * jsonfilename);
            //! set root directory relative to which _file values will be loaded; default is current directory, ./
            sUsrPropSet & setFileRoot(const char * path, idx len = 0);
            //! disable loading _file values into objects
            sUsrPropSet & disableFileRoot();

            //! description of object added or updated by propset
            struct Obj {
                sHiveId id; //!< id of new or updated object
                // sUsrPropSet implementation note: all access to utype must be through virtual functions (to allow pseudo-type override)
                const sUsrType2 * utype; //!< type of new or updated object
                bool is_new; //!< true if object was created (not existing updated object)
            };

            //! run propset, write values into the database
            /*! \param[out] modified_objs map from JSON object key to sUsrPropSet::Obj describing newly added or updated object
             *  \param flags bitwise-or of ERunFlags (0 by default)
             *  \returns true on success */
            bool run(sDic<Obj> * modified_objs = 0, udx flags = 0);
            //! pretend to propset, validate the JSON input, export values in 4-column tabular form
            /*! \param[out] out 4-column table into which to write props on success. Columns: id, name, path, value
             *  \param[out] modified_objs map from JSON object key to sUsrPropSet::Obj describing newly added or updated object
             *              (note that "newly added" objects will have Obj::id zero, but Obj::is_new == true)
             *  \param flags bitwise-or of ERunFlags (0 by default)
             *  \returns true on success */
            bool pretend(sVarSet & out, sDic<Obj> * modified_objs = 0, udx flags = 0);
            //! pretend to propset, validate the JSON input, export values in prop format
            /*! \param[out] out buffer into which to write traditional prop-format text on success.
             *  \param[out] modified_objs map from JSON object key to sUsrPropSet::Obj describing newly added or updated object
             *              (note that "newly added" objects will have Obj::id zero, but Obj::is_new == true)
             *  \param flags bitwise-or of ERunFlags (0 by default)
             *  \returns true on success */
            bool pretendPropFmt(sStr & out, sDic<Obj> * modified_objs = 0, udx flags = 0);
            const char * getErr() const
            {
                return _err.length() ? _err.ptr() : 0;
            }

            enum ERunFlags {
                fInvalidUserGroupNonFatal = 1 //!< an invalid user group ID/name is not a fatal error
            };

        protected:
            const sUsr * _usr;
            sStr _err;
            udx _flags;
            sFil _srcfil;
            const char * _srcbuf;
            idx _srclen;

            sStr _buf;
            sStr _file_path_buf;

            // for undoing added object files in case of failure
            sStr _added_files_buf;
            struct AddedFile {
                idx name_pos; // index into _added_files_buf, or negative for unused
                idx dst_file_pos; // index into _added_files_buf, or negative for unused
                bool trashed; // true if dst file already existed and was therefore trashed by trashFilePathname()
                sUsrObj * uobj; // not owned/allocated by this struct!
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
                sVariant key_fields; //!< used in is_auto_upsert case

                sUsrObj * uobj;
                bool is_auto_upsert; //!< true if object is being upserted by globally single-valued is_key_fg property values & type

                ObjLoc();
                ~ObjLoc();
            };

            typedef void TypeField;
            struct FieldStackElt {
                const TypeField * fld;
                idx depth; // json depth at which the field was specified
                idx imulti; // index of instance for multi-value field, or -sIdxMax if not used
                idx multi_depth; // json depth at which multi-value field's instance was specified
                idx irow; // index of row for array field, or -sIdxMax if not use
                idx row_depth; // json depth at which array field's row was specified
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
            sDic<ObjLoc> _objs; // object nodes (top-level children of json input)
            idx _cur_iobj;
            ESubObjMode _cur_submode;
            sDic<idx> _propnames;
            sTic<NodeLoc> _keys_tic; // stack of dicts of current json object keys during first pass, where stack level == node.depth
            sDic<NodeLoc> _comment_loc; // map from node.val_pos to json nodes containing comments, created by first pass
            idx _cur_icomment; // index into _comment_loc for current comment node; or -sIdxMax if not in a comment
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
            sDic<udx> _group_ids; // cached map of lower-cased user group paths to group IDs

            bool inComment(sJSONParser::ParseNode & node);
            ESubObjMode readSubmode(sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual bool checkCurObjSanity(sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual void readFieldNodeEndOfContainer(sJSONParser::ParseNode & node, sJSONParser & parser) { }
            virtual bool readFieldNode(sJSONParser::ParseNode & node, sJSONParser & parser);
            udx getGroupId(const char * grp_name);
            bool readPermNode(sJSONParser::ParseNode & node, sJSONParser & parser);
            bool readUpsertNode(Obj & out_obj, sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual const char * printPropPath(sStr & out);
            virtual const char * printIntermediateFlattenedPath(sStr & out, const TypeField * fld, const TypeField * ancestor) const;
            bool autoUpsertFindId(sJSONParser::ParseNode & node, sJSONParser & parser, ObjLoc * prop_obj);

            bool pretendInternal(sVarSet * var_set_out, sStr * prop_fmt_out, sDic<Obj> * modified_objs = 0, udx flags = 0);
            static void jsonFirstPassCb(sJSONParser::ParseNode & node, sJSONParser & parser, void * param);
            static void jsonSecondPassCb(sJSONParser::ParseNode & node, sJSONParser & parser, void * param);
            void printPretendField(const char * obj_name, const char * fld_name, const char * path);
            void printPretendField(const char * obj_name, const char * fld_name, idx path);
            void printPretendValue(const char * value, idx len=0);
            void printPretendValue(idx value);

            // overrideable to customize deriving prop_obj->obj.utype from JSON value
            virtual bool ensureUTypeFor(ObjLoc * prop_obj, sJSONParser::ParseNode & node, sJSONParser & parser);
            virtual const char * getUTypeName(ObjLoc * prop_obj) const
            {
                return prop_obj->obj.utype ? prop_obj->obj.utype->name() : "(null)";
            }
            virtual const sHiveId & getUTypeId(ObjLoc * prop_obj) const
            {
                return prop_obj->obj.utype ? prop_obj->obj.utype->id() : sHiveId::zero;
            }
            virtual udx getUpdateLevel();
            virtual bool updateStart();
            virtual bool hadDeadlocked();
            virtual bool updateAbandon();
            virtual bool updateComplete();
            virtual bool propInit(ObjLoc * prop_obj);
            virtual udx propSet(ObjLoc * prop_obj, const sUsrPropSet::TypeField * fld, const char * path, const char * value, udx value_len);
            virtual bool setPermission(ObjLoc * prop_obj, sJSONParser::ParseNode & node, Perm & perm);
            virtual const char * fldName(const TypeField * fld) const;
            virtual sUsrTypeField::EType fldType(const TypeField * fld) const;
            virtual bool fldNeedsValidation(const TypeField * fld) const { return true; }
            virtual bool fldCanHaveValue(const TypeField * fld) const;
            virtual bool fldCanSetValue(const TypeField * fld) const;
            virtual bool fldIsArrayRow(const TypeField * fld) const;
            virtual bool fldIsKey(const TypeField * fld) const;
            virtual bool fldIsMulti(const TypeField * fld) const;
            virtual bool fldIsGlobalMulti(const TypeField * fld) const;
            virtual const TypeField * fldGet(ObjLoc * prop_obj, const char * name, idx name_len) const;
            virtual const TypeField * fldParent(const TypeField * fld) const;
            virtual const TypeField * fldFlattenedNonArrayRowParent(const TypeField * fld) const;
    };
};

#endif
