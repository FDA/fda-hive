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
#ifndef sLib_usrprop_h
#define sLib_usrprop_h

#include <slib/core/dic.hpp>
#include <slib/core/id.hpp>
#include <slib/core/var.hpp>
#include <slib/core/vec.hpp>
#include <slib/std/variant.hpp>
#include <slib/utils/json/printer.hpp>
#include <ulib/usr.hpp>
#include <ulib/utype2.hpp>

namespace slib {
    class sUsrObjPropsTree;

    class sUsrObjPropsNode {
    public:
        enum FindStatus {
            eFindError = -1,
            eFindStop = 0,
            eFindContinue,
            eFindPrune
        };
        typedef FindStatus (*FindConstCallback)(const sUsrObjPropsNode &node, void *param);
        typedef FindStatus (*FindCallback)(sUsrObjPropsNode &node, void *param);

    protected:
        sUsrObjPropsTree * _tree;
        idx _row;
        idx _name;
        sStr _path;

        struct Navigation {
            idx self;
            idx parent;
            idx prev_sib;
            idx next_sib;
            idx first_child;
            idx last_child;
            idx dim;
            idx depth;

            Navigation()
            {
                self = parent = prev_sib = next_sib = first_child = last_child = -1;
                dim = depth = 0;
            }
        } _nav;

        FindStatus find(const char * field_name, const sUsrObjPropsNode ** firstFound, bool backwards, FindConstCallback callback, void * callbackParam) const;
        FindStatus find(const char * field_name, sUsrObjPropsNode ** firstFound, bool backwards, FindCallback callback, void * callbackParam);

    public:
        sUsrObjPropsNode(): _tree(0), _row(-1), _name(-1), _path(sMex::fExactSize) {}
        virtual ~sUsrObjPropsNode() {}

        const sUsrTypeField * field() const;
        const char * name() const;
        idx namecmp(const char * s) const;
        const char * path() const { return _path.ptr(); }
        idx treeIndex() const { return _nav.self; }
        //! depth from root (including array row virtual nodes)
        idx depth() const { return _nav.depth; }
        sUsrTypeField::EType type() const;

        // value of a leaf node
        bool hasValue() const { return _tree && _row >= 0; }
        bool value(sVariant &var) const;
        const char * value(const char * fallback=0) const;
        idx ivalue(idx fallback=0) const;
        udx uvalue(udx fallback=0) const;
        bool boolvalue(bool fallback=false) const;
        real rvalue(real fallback=0.) const;
        bool hiveidvalue(sHiveId & val) const;

        // flat list of values in a container node
        idx values(const char * field_name, sVariant &out) const;
        const char * values00(const char * field_name, sStr &out00) const;
        idx ivalues(const char * field_name, sVec<idx> &out) const;
        idx uvalues(const char * field_name, sVec<udx> &out) const;
        idx boolvalues(const char * field_name, sVec<bool> &out) const;
        idx boolvalues(const char * field_name, sVec<idx> &out) const;
        idx rvalues(const char * field_name, sVec<real> &out) const;
        idx hiveidvalues(const char * field_name, sVec<sHiveId> &out) const;

        // structured representation of value(s) (as scalar, list, dic, whatever is more
        // appropriate) in a container node, derived from flattened type field tree
        idx structured(const char * field_name, sVariant &out, const sUsrObjPropsNode ** out_outer_list = 0) const;
        // retrieve a dictionary variant of all available structured values
        idx allStructured(sVariant &out) const;

        // json output
        /* ! \param into_object Print into an existing JSON object; do not open or close top-level braces
             \param flatten Omit intermediate decorative nodes (ones which serve only for visual grouping of fields)\
             \warning into_object can *only* be used on inner nodes that would normally print as an object of keys/values;
                                  it is an error to use into_object on leaf nodes */
        bool printJSON(sJSONPrinter & out, bool into_object = false, bool flatten = false) const;

        // navigation
        idx dim(const char * field_name=0) const;
        bool isRoot() const;

        const sUsrObjPropsNode * find(const char * field_name=0, FindConstCallback callback=0, void * callbackParam=0) const;
        const sUsrObjPropsNode * findBackwards(const char * field_name=0, FindConstCallback callback=0, void * callbackParam=0) const;
        const sUsrObjPropsNode * firstChild(const char * field_name=0) const;
        const sUsrObjPropsNode * lastChild(const char * field_name=0) const;
        const sUsrObjPropsNode * previousSibling(const char * field_name=0) const;
        const sUsrObjPropsNode * nextSibling(const char * field_name=0) const;
        const sUsrObjPropsNode * parentNode() const;
        sUsrObjPropsNode * find(const char * field_name=0, FindCallback callback=0, void * callbackParam=0);
        sUsrObjPropsNode * findBackwards(const char * field_name=0, FindCallback callback=0, void * callbackParam=0);
        inline sUsrObjPropsNode * firstChild(const char * field_name=0) { return const_cast<sUsrObjPropsNode*>(static_cast<const sUsrObjPropsNode&>(*this).firstChild(field_name)); }
        inline sUsrObjPropsNode * lastChild(const char * field_name=0) { return const_cast<sUsrObjPropsNode*>(static_cast<const sUsrObjPropsNode&>(*this).lastChild(field_name)); }
        inline sUsrObjPropsNode * previousSibling(const char * field_name=0) { return const_cast<sUsrObjPropsNode*>(static_cast<const sUsrObjPropsNode&>(*this).previousSibling(field_name)); }
        inline sUsrObjPropsNode * nextSibling(const char * field_name=0) { return const_cast<sUsrObjPropsNode*>(static_cast<const sUsrObjPropsNode&>(*this).nextSibling(field_name)); }
        inline sUsrObjPropsNode * parentNode(const char * field_name=0) { return const_cast<sUsrObjPropsNode*>(static_cast<const sUsrObjPropsNode&>(*this).parentNode()); }

        const char * findValue(const char * field_name, const char * fallback=0) const;
        bool findValue(const char * field_name, sVariant & val) const;
        idx findIValue(const char * field_name, idx fallback=0) const;
        udx findUValue(const char * field_name, udx fallback=0) const;
        bool findBoolValue(const char * field_name, bool fallback=false) const;
        real findRValue(const char * field_name, real fallback=0.) const;
        bool findHiveIdValue(const char * field_name, sHiveId & val) const;

        const char * findValueOrDefault(const char * field_name) const;
        bool findValueOrDefault(const char * field_name, sVariant & val) const;
        idx findIValueOrDefault(const char * field_name) const;
        udx findUValueOrDefault(const char * field_name) const;
        bool findBoolValueOrDefault(const char * field_name) const;
        real findRValueOrDefault(const char * field_name) const;
        bool findHiveIdValueOrDefault(const char * field_name, sHiveId & val) const;

        // modify data
        bool set(sVariant &val);
        bool set(const char * val=0, idx len=0);
        bool set(const sHiveId & val);
        bool iset(idx val);
        bool uset(udx val);
        bool rset(real val);

        // append to containers. Creates intermediate container nodes automatically if needed.
        const sUsrObjPropsNode * push(const char * field_name, sVariant &val);
        const sUsrObjPropsNode * push(const char * field_name, const char * val=0);
        const sUsrObjPropsNode * push(const char * field_name, const sHiveId & val);
        const sUsrObjPropsNode * ipush(const char * field_name, idx ival);
        const sUsrObjPropsNode * upush(const char * field_name, udx uval);
        const sUsrObjPropsNode * rpush(const char * field_name, real rval);

#ifdef _DEBUG
        virtual const char * printDump(sStr & out, bool onlySelf=false) const;
#endif

        friend class sUsrObjPropsTree;
    };

    class sUsrObjPropsTree: public sUsrObjPropsNode {
    protected:
        const sUsr& _usr;
        sHiveId _type_id; // guaranteed to be zero or a valid type id
        sVarSet _table;
        sVarSet * _ptable;
        sVec<sUsrObjPropsNode> _nodes;
        sDic<char> _namesPrintable;
        sDic<sDic<idx> > _pathMap; // path => name => node index

        void init(const sUsrType2 * obj_type, const sHiveId * obj_type_id);

        bool addPathMapEntry(const char * field_name, const char * path, idx index);
        idx getPathMapEntry(const char * field_name, const char * path) const;
        sUsrObjPropsNode * addNode(const char * field_name, const char * path);
        void linkNodeParentSiblings(idx index, idx parent_index);
        bool linkNode(idx index);

        bool parseTable(const sVarSet & table, idx first_row, sVec<idx> * rows_push, const char * filter);
        idx pushHelper(idx node_index, const char * field_name, sVariant &val);
        idx pushHelper(idx node_index, const char * field_name, const char * value);
        const char * findIdString(sStr & buf) const;

    public:
        sUsrObjPropsTree(const sUsr& usr, const sUsrType2 * obj_type);
        sUsrObjPropsTree(const sUsr& usr, const char * obj_type_name);
        sUsrObjPropsTree(const sUsr& usr, const sUsrType2 * obj_type, sVarSet & table);
        sUsrObjPropsTree(const sUsr& usr, const char * obj_type_name, sVarSet & table);
        sUsrObjPropsTree(const sUsr& usr, const sUsrType2 * obj_type, const sVar & form);
        sUsrObjPropsTree(const sUsr& usr, const char * obj_type_name, const sVar & form);

        bool initialized() const;
        const char * objTypeName() const;
        const sUsrType2 * objType() const;

        void empty(bool empty_table);
        virtual ~sUsrObjPropsTree();

        sVarSet& getTable() { return *_ptable; }
        const sVarSet& getTable() const { return *_ptable; }
        bool useTable(sVarSet & table, const char * filter=0);
        bool addTable(const sVarSet & table, const char * filter=0);
        bool useForm(const sVar & form, const char * filter=0);
        bool addForm(const sVar & form, const char * filter=0);

        const sUsrObjPropsNode * getNodeByIndex(idx i) const;
        inline sUsrObjPropsNode * getNodeByIndex(idx i) { return const_cast<sUsrObjPropsNode*>(static_cast<const sUsrObjPropsTree&>(*this).getNodeByIndex(i)); }
        idx dimTree() const { return _nodes.dim(); }

#if 0 // TODO
        // set a value with a specific path
        bool setValueAt(const char * path, const char * field_name, const sVariant &val);
        bool setValueAt(const char * path, const char * field_name, const char * val=0);
#endif

        bool valid(sStr * log = 0) const;
        bool complete(sStr * log = 0) const;

#ifdef _DEBUG
        virtual const char * printDump(sStr & out, bool onlySelf=false) const;
#endif

        friend class sUsrObjPropsNode;
    };
};

#endif
