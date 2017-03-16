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

#include <assert.h>
#include <ctype.h>
#include <math.h>

#include <slib/utils/sort.hpp>
#include <ulib/uprop.hpp>
#include <ulib/utype2.hpp>

#define COL_ID 0
#define COL_NAME 1
#define COL_PATH 2
#define COL_VALUE 3

using namespace slib;

static int _safe_strcasecmp(const char * s1, const char * s2)
{
    if (!s1)
        return s2 != 0;

    if (!s2)
        return -1;

    return strcasecmp(s1, s2);
}

idx sUsrObjPropsNode::namecmp(const char * s) const
{
    return _safe_strcasecmp(name(), s);
}

const sUsrTypeField * sUsrObjPropsNode::field() const
{
    if (!_tree || !name())
        return 0;

    return _tree->objType()->getField(_tree->_usr, name());
}

const char * sUsrObjPropsNode::name() const
{
    if (!_tree || _name < 0)
        return 0;

    return static_cast<const char*>(_tree->_namesPrintable.id(_name));
}

sUsrTypeField::EType sUsrObjPropsNode::type() const
{
    const sUsrTypeField * fld = field();
    if (!fld)
        return sUsrTypeField::eInvalid;
    return fld->type();
}

const char * sUsrObjPropsNode::value(const char * fallback /* = 0 */) const
{
    if (!hasValue())
        return fallback;

    const char * ret = _tree->_ptable->val(_row, COL_VALUE);
    return ret ? ret : fallback;
}

bool sUsrObjPropsNode::value(sVariant &var) const
{
    return sUsrTypeField::parseValue(var, type(), value(0));
}

idx sUsrObjPropsNode::ivalue(idx fallback /* = 0 */) const
{
    const char * svalue = value(0);
    if (!svalue)
        return fallback;

    sVariant var(svalue);
    return var.asInt();
}

udx sUsrObjPropsNode::uvalue(udx fallback /* = 0 */) const
{
    const char * svalue = value(0);
    if (!svalue)
        return fallback;

    sVariant var(svalue);
    return var.asUInt();
}

bool sUsrObjPropsNode::boolvalue(bool fallback /* = false */) const
{
    const char * svalue = value(0);
    if (!svalue)
        return fallback;

    sVariant var(svalue);
    return var.asBool();
}

real sUsrObjPropsNode::rvalue(real fallback /* = 0 */) const
{
    const char * svalue = value(0);
    if (!svalue)
        return fallback;

    sVariant var(svalue);
    return var.asReal();
}

bool sUsrObjPropsNode::hiveidvalue(sHiveId & val) const
{
    val.parse(value(0));
    return val;
}

static sUsrObjPropsNode::FindStatus values00getter(const sUsrObjPropsNode& node, void * param)
{
    sStr * out00 = static_cast<sStr*>(param);
    const char * value = node.value();
    if (!value)
        return sUsrObjPropsNode::eFindError;

    out00->add(value);
    return sUsrObjPropsNode::eFindContinue;
}

const char * sUsrObjPropsNode::values00(const char * field_name, sStr &out00) const
{
    idx len = out00.length();
    if (!find(field_name, values00getter, &out00))
        out00.cut(len);
    if (out00.ptr())
        out00.add0(2);
    return out00.ptr(len);
}

static sUsrObjPropsNode::FindStatus valuesGetter(const sUsrObjPropsNode& node, void * param)
{
    sVariant * out = static_cast<sVariant*>(param);
    sVariant var;
    if (!node.value(var))
        return sUsrObjPropsNode::eFindError;

    if (var.isScalar())
        out->push(var);
    else
        out->append(var);

    return sUsrObjPropsNode::eFindContinue;
}

idx sUsrObjPropsNode::values(const char * field_name, sVariant &out) const
{
    out.setList();
    if (!find(field_name, valuesGetter, &out))
        out.setList();
    return out.dim();
}

#define DECLARE_VALUES_VEC_GETTER(method_name, store_type, value_getter) \
static sUsrObjPropsNode::FindStatus method_name ## _ ## store_type ## _ ## value_getter ## Getter (const sUsrObjPropsNode& node, void * param) \
{ \
    sVec<store_type> * vec = static_cast<sVec<store_type>*>(param); \
    sVariant var; \
    if (!node.value(var)) \
        return sUsrObjPropsNode::eFindError; \
    *(vec->add(1)) = var.value_getter(); \
    return sUsrObjPropsNode::eFindContinue; \
} \
idx sUsrObjPropsNode::method_name(const char * field_name, sVec<store_type> &out) const \
{ \
    idx len = out.dim(); \
    if (!find(field_name, method_name ## _ ## store_type ## _ ## value_getter ## Getter, &out)) \
        out.cut(len); \
    return out.dim() - len; \
}

DECLARE_VALUES_VEC_GETTER(ivalues, idx, asInt)
DECLARE_VALUES_VEC_GETTER(uvalues, udx, asUInt)
DECLARE_VALUES_VEC_GETTER(boolvalues, bool, asBool)
DECLARE_VALUES_VEC_GETTER(boolvalues, idx, asBool)
DECLARE_VALUES_VEC_GETTER(rvalues, real, asReal)

static sUsrObjPropsNode::FindStatus hiveidvalues_getter(const sUsrObjPropsNode& node, void * param)
{
    sVec<sHiveId> * vec = static_cast<sVec<sHiveId>*>(param);
    sVariant var;
    if (!node.value(var))
        return sUsrObjPropsNode::eFindError;
    var.asHiveId(vec->add(1));
    return sUsrObjPropsNode::eFindContinue;
}

idx sUsrObjPropsNode::hiveidvalues(const char * field_name, sVec<sHiveId> &out) const
{
    idx len = out.dim();
    if (!find(field_name, hiveidvalues_getter, &out))
        out.cut(len);
    return out.dim() - len;
}

class StructuredStack {
protected:
    struct StackElt {
        const sUsrObjPropsNode * prop;
        bool is_dic;

        StackElt() { sSet(this); }
    };

    sVec<StackElt> _stack;
    idx _stack_dim;
    idx _stack_done_dim;
    sVariant _val_root;
    const sUsrObjPropsNode * _outer_list;

    void makeValueNode(sVariant * value_node, idx i)
    {
        if (_stack[i].is_dic)
            value_node->setDic();
        else
            value_node->setList();
    }

public:
    StructuredStack()
    {
        _stack_dim = _stack_done_dim = 0;
        _outer_list = 0;
    }

    const char * printDump(sStr & out)
    {
        idx start = out.length();
        out.printf("{ dim: %" DEC ", done: %" DEC ", root: %s, stack: [", _stack_dim, _stack_done_dim, _val_root.asString());
        if (_stack_dim) {
            out.printf("\n");
            for (idx i=0; i<_stack_dim; i++) {
                out.printf("    { name: '%s', path: '%s', dic: %s }%s\n", _stack[i].prop->name(), _stack[i].prop->path(), _stack[i].is_dic ? "true" : "false", i+1 < _stack_dim ? "," : "");
            }
        }
        out.printf("] }\n");
        return out.ptr(start);
    }

    void push(const sUsrObjPropsNode * prop, bool is_list, bool is_dic)
    {
        assert (prop);
        if( !_outer_list && is_list ) {
            _outer_list = prop;
        }

        // multivalued nodes need to be merged with previous siblings of the same
        // type into one list
        if (_stack_dim == _stack_done_dim &&
            _stack.dim() > _stack_done_dim &&
            is_list &&
            !_stack[_stack_dim].is_dic &&
            prop->field() == _stack[_stack_dim].prop->field())
        {
            _stack_dim++;
            _stack_done_dim = _stack_dim;

            is_list = false;
            if (!is_dic)
                return;
        }

        _stack.resize(++_stack_dim);
        _stack[_stack_dim-1].prop = prop;
        if (is_list && is_dic) {
            _stack[_stack_dim-1].is_dic = false;
            _stack.resize(++_stack_dim);
            _stack[_stack_dim-1].prop = prop;
            _stack[_stack_dim-1].is_dic = true;
        } else {
            _stack[_stack_dim-1].is_dic = is_dic;
        }
    }

    void pop()
    {
        if (isEmpty())
            return;

        const sUsrObjPropsNode * prop = peek();
        while (_stack_dim && _stack[_stack_dim-1].prop->field() == prop->field()) {
            _stack_dim--;
        }

        _stack_done_dim = sMin<idx>(_stack_done_dim, _stack_dim);
    }

    inline bool isEmpty()
    {
        return !_stack_dim;
    }

    const sUsrObjPropsNode * peek()
    {
        if (isEmpty())
            return 0;

        return _stack[_stack_dim-1].prop;
    }

    const sUsrObjPropsNode * outerList()
    {
        return _outer_list;
    }

    void addValue(const sUsrObjPropsNode * prop, sVariant &val)
    {
        if (isEmpty()) {
            _val_root = val;
            return;
        }

        sVariant * value_node = &_val_root;
        if (!_stack_done_dim) {
            makeValueNode(value_node, 0);
            _stack_done_dim = 1;
        }

        for (idx i=1; i<_stack_dim; i++) {
            const char * name = _stack[i].prop->name();
            assert(name);

            if (i < _stack_done_dim) {
                if (value_node->isDic()) {
                    value_node = value_node->getDicElt(name);
                } else {
                    assert(value_node->isList());
                    value_node = value_node->getListElt(value_node->dim() - 1);
                }
            } else {
                if (value_node->isDic()) {
                    value_node = value_node->setElt(name, (idx)0);
                } else {
                    assert(value_node->isList());
                    value_node = value_node->push((idx)0);
                }
                makeValueNode(value_node, i);
            }
        }

        _stack_done_dim = _stack_dim;

        if (value_node->isDic()) {
            value_node->setElt(prop->name(), val);
        } else {
            assert(value_node->isList());
            value_node->push(val);
        }
    }

    sVariant& getValueRoot()
    {
        return _val_root;
    }
};

static idx structuredRecurse(const sUsrObjPropsNode * prop, const char * field_name, StructuredStack & stack)
{
    idx total = 0;
    bool need_pop = false;
    const sUsrTypeField * field = prop->field();
    bool prop_matches_name = !_safe_strcasecmp(field_name, prop->name());
    if (field && !field->isFlattenedDecor()) {
        sUsrTypeField::EType type = field->type();
        bool is_list, is_dic;
        if( type == sUsrTypeField::eArray) {
            is_list = field->isMulti();
            const sUsrTypeField * array_row = field->getChild(0);
            is_dic = array_row && array_row->isArrayRow() && array_row->isFlattenedDecor();
        } else {
            is_list = field->isMulti();
            is_dic = type == sUsrTypeField::eList;
        }

        if (is_list || is_dic) {
            stack.push(prop, is_list, is_dic);
            need_pop = true;
        }

        if (prop->hasValue() && (!field_name || prop_matches_name)) {
            sVariant val;
            prop->value(val);
            stack.addValue(prop, val);
            total++;
        }
    }

    for (const sUsrObjPropsNode * child = prop->firstChild(); child; child = child->nextSibling()) {
        idx count = structuredRecurse(child, prop_matches_name ? 0 : field_name, stack);
        if (count < 0)
            return -1;
        total += count;
    }

    if (need_pop)
        stack.pop();

    return total;
}

idx sUsrObjPropsNode::structured(const char * field_name, sVariant &out, const sUsrObjPropsNode ** out_outer_list/* = 0*/) const
{
    StructuredStack stack;

    idx count = structuredRecurse(this, field_name, stack);
    out = stack.getValueRoot();
    if( out_outer_list ) {
        *out_outer_list = stack.outerList();
    }
    return count;
}

static sUsrObjPropsNode::FindStatus interestingFieldsGetter(const sUsrObjPropsNode& node, void * param)
{
    sDic<const sUsrTypeField *> *interesting_fields = static_cast<sDic<const sUsrTypeField *>*>(param);

    const sUsrTypeField * fld = node.field();
    if (fld && !fld->isFlattenedDecor() && !fld->flattenedParent())
        *(interesting_fields->set(node.name())) = fld->isArrayRow() ? fld->parent() : fld;

    return sUsrObjPropsNode::eFindContinue;
}

idx sUsrObjPropsNode::allStructured(sVariant &out) const
{
    sDic<const sUsrTypeField *> interesting_fields;
    find(0, interestingFieldsGetter, &interesting_fields);
    idx total = 0;
    out.setDic();
    for (idx i=0; i<interesting_fields.dim(); i++) {
        const char * search_name = static_cast<const char *>(interesting_fields.id(i));
        const char * print_name = (*(interesting_fields.ptr(i)))->name();

        sVariant val;
        idx count = structured(search_name, val);
        if (count > 0) {
            out.setElt(print_name, val);
            total += count;
        }
    }
    return total;
}

// if a path has been used already, append '.' and an incrementing counter (because JSON requires unique keys in an object)
static const char * ensureUniquifiedPath(sDic<idx> & path2cnt, const char * path)
{
    if( !path ) {
        path = "";
    }

    if( idx * pcnt = path2cnt.get(path) ) {
        sStr buf;
        buf.printf("%s.%" DEC, path, *pcnt);
        (*pcnt)++;

        idx new_path_num = 0;
        (*path2cnt.setString(buf.ptr(), 0, &new_path_num))++;
        return static_cast<const char *>(path2cnt.id(new_path_num));
    } else {
        *path2cnt.setString(path) = 1;
        return path;
    }
}

bool sUsrObjPropsNode::printJSON(sJSONPrinter & out, bool into_object/*  = false*/, bool flatten/*  = false*/) const
{
    // simple case : leaf node = json scalar value
    if( hasValue() ) {
        if( into_object ) {
            // we cannot print a scalar without a key into the middle of a JSON object!
            fprintf(stderr, "sUsrObjPropsNode::printJSON() with into_object==true does not make sense for leaf nodes\n");
            return false;
        }
        sVariant val;
        bool ret = value(val);
        out.addValue(val);
        return ret;
    }

    // complex case : inner node = json object, with names as first-level keys and paths as second-level

    sMex lst_mex;
    sDic< sLst<const sUsrObjPropsNode*> > by_name;

    if( !into_object ) {
        out.startObject();
    }

    for(const sUsrObjPropsNode * child = firstChild(); child; child = child->nextSibling()) {
        sLst<const sUsrObjPropsNode*> * lst = by_name.set(child->name());
        if( !lst->_mex ) {
            lst->init(&lst_mex);
        }
        *lst->add(1) = child;
    }

    bool ret = true;

    sDic<idx> path2cnt;

    for(idx iname=0; iname<by_name.dim(); iname++) {
        sLst<const sUsrObjPropsNode*> * lst = by_name.ptr(iname);
        const char * child_name = static_cast<const char *>(by_name.id(iname));
        const sUsrTypeField * child_field = _tree->objType()->getField(_tree->_usr, child_name);
        bool is_array = child_field && child_field->isArrayRow();
        bool is_multi = !is_array && child_field && child_field->isMulti();
        bool is_broken_multi = !is_array && !is_multi && lst->dim() > 1; // multiple nodes with different paths for a field that must be non-multi. Merge these manually.
        bool is_broken_multi_scalar = false;
        if( is_broken_multi ) {
            for(idx ic=0; ic<lst->dim(); ic++) {
                const sUsrObjPropsNode * child = (*lst)[ic];
                if( child->hasValue() ) {
                    is_broken_multi_scalar = true; // multiple nodes with different paths for a *scalar* field that must be non-multi. This is really bad, but try to do something sane when printing
                }
            }
        }
        bool is_flattened_decorative_child = flatten && child_field && child_field->isFlattenedDecor();
        bool added_child_name_key = false;
        bool started_child_object = false;

        if( !is_flattened_decorative_child ) {
            if( !is_array ) {
                out.addKey(child_name);
                added_child_name_key = true;
            }

            if( is_multi || is_broken_multi ) {
                out.startObject();
                started_child_object = true;
            }
        }

        for(idx ic=0; ic<lst->dim(); ic++) {
            const sUsrObjPropsNode * child = (*lst)[ic];
            if( is_multi || is_broken_multi_scalar ) {
                out.addKey(ensureUniquifiedPath(path2cnt, child->path()));
                if( !child->printJSON(out, false, flatten) ) {
                    ret = false;
                }
            } else if( is_array ) {
                if( !is_flattened_decorative_child ) {
                    out.addKey(ensureUniquifiedPath(path2cnt, child->path()));
                    out.startObject();
                }
                for (const sUsrObjPropsNode * grandchild = child->firstChild(); grandchild; grandchild = grandchild->nextSibling()) {
                    const sUsrTypeField * grandchild_field = grandchild->field();
                    bool is_flattened_decorative_grandchild = flatten && grandchild_field && grandchild_field->isFlattenedDecor();
                    if( !is_flattened_decorative_grandchild ) {
                        out.addKey(grandchild->name());
                    }
                    if( !grandchild->printJSON(out, is_flattened_decorative_grandchild, flatten) ) {
                        ret = false;
                    }
                }
                if( !is_flattened_decorative_child ) {
                    out.endObject();
                }
            } else {
                if( !child->printJSON(out, started_child_object || !added_child_name_key, flatten) ) {
                    ret = false;
                }
            }
        }

        if( !is_flattened_decorative_child ) {
            if( is_multi || is_broken_multi ) {
                out.endObject();
            }
        }
    }

    if( !into_object ) {
        out.endObject();
    }

    return ret;
}

idx sUsrObjPropsNode::dim(const char * field_name) const
{
    if (!field_name)
        return _nav.dim;

    idx count=0;
    for (const sUsrObjPropsNode * child = firstChild(); child; child = child->nextSibling()) {
        if (!_safe_strcasecmp(child->name(), field_name))
            count++;
    }
    return count;
}

bool sUsrObjPropsNode::isRoot() const
{
    return this == _tree;
}

sUsrObjPropsNode::FindStatus sUsrObjPropsNode::find(const char * field_name, const sUsrObjPropsNode ** firstFound, bool backwards, FindConstCallback callback, void * callbackParam) const
{
    sUsrObjPropsNode::FindStatus status = eFindContinue;

    if (!field_name || !namecmp(field_name)) {
        if (callback)
            status = callback(*this, callbackParam);

        if (status == eFindError)
            return eFindError;

        if (!*firstFound)
            *firstFound = this;
    }

    if (backwards) {
        for (const sUsrObjPropsNode * child = lastChild(); child && status == eFindContinue; child = child->previousSibling())
            status = child->find(field_name, firstFound, backwards, callback, callbackParam);
    } else {
        for (const sUsrObjPropsNode * child = firstChild(); child && status == eFindContinue; child = child->nextSibling())
            status = child->find(field_name, firstFound, backwards, callback, callbackParam);
    }

    if (status == eFindPrune)
        return eFindContinue;

    return status;
}

sUsrObjPropsNode::FindStatus sUsrObjPropsNode::find(const char * field_name, sUsrObjPropsNode ** firstFound, bool backwards, FindCallback callback, void * callbackParam)
{
    return const_cast<const sUsrObjPropsNode&>(*this).find(field_name, const_cast<const sUsrObjPropsNode**>(firstFound), backwards, (FindConstCallback)callback, callbackParam);
}

const sUsrObjPropsNode * sUsrObjPropsNode::find(const char * field_name /* = 0 */, FindConstCallback callback /* = 0 */, void * callbackParam /* = 0 */) const
{
    const sUsrObjPropsNode * ret = 0;
    if (find(field_name, &ret, false, callback, callbackParam) == eFindError)
        return 0;
    return ret;
}

sUsrObjPropsNode * sUsrObjPropsNode::find(const char * field_name /* = 0 */, FindCallback callback /* = 0 */, void * callbackParam /* = 0 */)
{
    sUsrObjPropsNode * ret = 0;
    if (find(field_name, &ret, false, callback, callbackParam) == eFindError)
        return 0;
    return ret;
}

const sUsrObjPropsNode * sUsrObjPropsNode::findBackwards(const char * field_name /* = 0 */, FindConstCallback callback /* = 0 */, void * callbackParam /* = 0 */) const
{
    const sUsrObjPropsNode * ret = 0;
    if (find(field_name, &ret, true, callback, callbackParam) == eFindError)
        return 0;
    return ret;
}

sUsrObjPropsNode * sUsrObjPropsNode::findBackwards(const char * field_name /* = 0 */, FindCallback callback /* = 0 */, void * callbackParam /* = 0 */)
{
    sUsrObjPropsNode * ret = 0;
    if (find(field_name, &ret, true, callback, callbackParam) == eFindError)
        return 0;
    return ret;
}

#define TREE_ELT(i) ( _tree ? ( (i) < 0 ? 0 : _tree->_nodes.ptr(i) ) : 0 )
#define TREE_ELT_OR_ROOT(i) ( _tree ? ( (i) < 0 ? _tree : _tree->_nodes.ptr(i) ) : 0 )

const sUsrObjPropsNode * sUsrObjPropsNode::firstChild(const char * field_name /* = 0 */) const
{
    for (const sUsrObjPropsNode * child = TREE_ELT(_nav.first_child); child; child = child->nextSibling()) {
        if (!field_name || !child->namecmp(field_name))
            return child;
    }
    return 0;
}

const sUsrObjPropsNode * sUsrObjPropsNode::lastChild(const char * field_name /* = 0 */) const
{
    for (const sUsrObjPropsNode * child = TREE_ELT(_nav.last_child); child; child = child->previousSibling()) {
        if (!field_name || !child->namecmp(field_name))
            return child;
    }
    return 0;
}

const sUsrObjPropsNode * sUsrObjPropsNode::previousSibling(const char * field_name /* = 0 */) const
{
    for (const sUsrObjPropsNode * sibling = TREE_ELT(_nav.prev_sib); sibling; sibling = TREE_ELT(sibling->_nav.prev_sib)) {
        if (!field_name || !sibling->namecmp(field_name))
            return sibling;
    }

    return 0;
}

const sUsrObjPropsNode * sUsrObjPropsNode::nextSibling(const char * field_name /* = 0 */) const
{
    for (const sUsrObjPropsNode * sibling = TREE_ELT(_nav.next_sib); sibling; sibling = TREE_ELT(sibling->_nav.next_sib)) {
        if (!field_name || !sibling->namecmp(field_name))
            return sibling;
    }

    return 0;
}

const sUsrObjPropsNode * sUsrObjPropsNode::parentNode() const
{
    return TREE_ELT(_nav.parent);
}

const char * sUsrObjPropsNode::findValue(const char * field_name, const char * fallback /*=0*/) const
{
    const sUsrObjPropsNode * node = find(field_name);
    return node ? node->value(fallback) : fallback;
}

bool sUsrObjPropsNode::findValue(const char * field_name, sVariant & val) const
{
    const sUsrObjPropsNode * node = find(field_name);
    return node ? node->value(val) : false;
}

idx sUsrObjPropsNode::findIValue(const char * field_name, idx fallback /*=0*/) const
{
    const sUsrObjPropsNode * node = find(field_name);
    return node ? node->ivalue(fallback) : fallback;
}

udx sUsrObjPropsNode::findUValue(const char * field_name, udx fallback /*=0*/) const
{
    const sUsrObjPropsNode * node = find(field_name);
    return node ? node->uvalue(fallback) : fallback;
}

bool sUsrObjPropsNode::findBoolValue(const char * field_name, bool fallback /*=false*/) const
{
    const sUsrObjPropsNode * node = find(field_name);
    return node ? node->boolvalue(fallback) : fallback;
}

real sUsrObjPropsNode::findRValue(const char * field_name, real fallback /*=0.*/) const
{
    const sUsrObjPropsNode * node = find(field_name);
    return node ? node->rvalue(fallback) : fallback;
}

bool sUsrObjPropsNode::findHiveIdValue(const char * field_name, sHiveId & val) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (!node) {
        val.reset();
        return false;
    }
    return node->hiveidvalue(val);
}

const char * sUsrObjPropsNode::findValueOrDefault(const char * field_name) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->value();

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld)
        return fld->defaultValue();

    return 0;
}

bool sUsrObjPropsNode::findValueOrDefault(const char * field_name, sVariant & val) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->value(val);

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld)
        return fld->parseValue(val, fld->defaultValue());

    val.setNull();
    return false;
}

idx sUsrObjPropsNode::findIValueOrDefault(const char * field_name) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->ivalue();

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld) {
        sVariant var;
        var.parseInt(fld->defaultValue());
        return var.asInt();
    }

    return 0;
}

udx sUsrObjPropsNode::findUValueOrDefault(const char * field_name) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->uvalue();

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld) {
        sVariant var;
        var.parseUInt(fld->defaultValue());
        return var.asUInt();
    }

    return 0;
}

bool sUsrObjPropsNode::findBoolValueOrDefault(const char * field_name) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->boolvalue();

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld) {
        sVariant var;
        var.parseBool(fld->defaultValue());
        return var.asBool();
    }

    return 0;
}

real sUsrObjPropsNode::findRValueOrDefault(const char * field_name) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->rvalue();

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld) {
        sVariant var;
        var.parseReal(fld->defaultValue());
        return var.asReal();
    }

    return 0;
}

bool sUsrObjPropsNode::findHiveIdValueOrDefault(const char * field_name, sHiveId & val) const
{
    const sUsrObjPropsNode * node = find(field_name);
    if (node && node->hasValue())
        return node->hiveidvalue(val);

    const sUsrTypeField * fld = _tree->objType()->getField(_tree->_usr, field_name);
    if (fld) {
        val.parse(fld->defaultValue());
        return val;
    }

    val.reset();
    return false;
}

static const char * toValueString(sVariant &val, sUsrTypeField::EType type, sVariant &tmp)
{
    const char * s = 0;
    switch (type) {
    case sUsrTypeField::eString:
    case sUsrTypeField::eUrl:
    case sUsrTypeField::eText:
    case sUsrTypeField::ePassword:
        s = val.asString();
        break;
    case sUsrTypeField::eBool:
        tmp.setInt(val.asBool());
        s = tmp.asString();
        break;
    case sUsrTypeField::eInteger:
        if (val.isInt()) {
            s = val.asString();
        } else {
            tmp.setInt(val.asInt());
            s = tmp.asString();
        }
        break;
    case sUsrTypeField::eDate:
        if (val.isDate()) {
            s = val.asString();
        } else {
            tmp.setDate(val);
            s = tmp.asString();
        }
        break;
    case sUsrTypeField::eTime:
        if (val.isTime()) {
            s = val.asString();
        } else {
            tmp.setTime(val);
            s = tmp.asString();
        }
        break;
    case sUsrTypeField::eDateTime:
        if (val.isDateTime()) {
            s = val.asString();
        } else {
            tmp.setDateTime(val);
            s = tmp.asString();
        }
        break;
    case sUsrTypeField::eReal:
        if (val.isNumeric()) {
            s = val.asString();
        } else {
            tmp.setReal(val.asReal());
            s = tmp.asString();
        }
        break;
    case sUsrTypeField::eObj:
        tmp.setHiveId(val);
        s = tmp.asString();
        break;
    default:
        // Don't allow to set values of list, array, or invalid
        return 0;
    }
    return s;
}

static bool isValueType(const char * val, sUsrTypeField::EType type)
{
    if (!val)
        return false;

    sHiveId id;
    struct tm tm;
    idx len = -1;
    char * endptr = 0;
    switch (type) {
    case sUsrTypeField::eString:
    case sUsrTypeField::eUrl:
    case sUsrTypeField::eText:
    case sUsrTypeField::ePassword:
        // any string works
        return true;
    case sUsrTypeField::eBool:
        return sString::matchesBool(val);
    case sUsrTypeField::eInteger:
        strtoidx(val, &endptr, 10);
        break;
    case sUsrTypeField::eDate:
    case sUsrTypeField::eDateTime:
        if (sString::parseDateTime(&tm, val, &len) != -sIdxMax) {
            endptr = const_cast<char*>(val + len);
        } else {
            return false;
        }
        break;
    case sUsrTypeField::eTime:
        if (sString::parseTime(val, &len) != -sIdxMax) {
            endptr = const_cast<char*>(val + len);
        } else {
            strtoidx(val, &endptr, 10);
        }
        break;
    case sUsrTypeField::eReal:
        strtod(val, &endptr);
        break;
    case sUsrTypeField::eObj:
        len = id.parse(val);
        endptr = const_cast<char*>(val + len);
        break;
    default:
        return false;
    }

    if (!endptr)
        return false;

    while (*endptr) {
        if (!isspace(*endptr++))
            return false;
    }

    return true;
}

bool sUsrObjPropsNode::set(sVariant &val)
{
    const sUsrTypeField * fld = field();
    if( !hasValue() || !fld || !fld->canHaveValue() )
        return false;

    sVariant tmp;
    const char * s = toValueString(val, type(), tmp);
    if (!s)
        return false;

    return _tree->_ptable->updateVal(_row, COL_VALUE, s);
}

bool sUsrObjPropsNode::set(const char * val /* = 0 */, idx len /* = 0 */)
{
    const sUsrTypeField * fld = field();
    if( !hasValue() || !fld || !fld->canHaveValue() )
        return false;

    return _tree->_ptable->updateVal(_row, COL_VALUE, val, len);
}

bool sUsrObjPropsNode::set(const sHiveId & val)
{
    const sUsrTypeField * fld = field();
    if( fld ) {
        sStr tmp;
        switch(fld->type()) {
            case sUsrTypeField::eInteger:
            case sUsrTypeField::eReal:
            case sUsrTypeField::eBool:
                return uset(val.objId());
            default:
                val.print(tmp);
                return set(tmp, tmp.length());
        }
    }
    return false;
}

bool sUsrObjPropsNode::iset(idx val)
{
    sVariant tmp(val);
    return set(tmp);
}

bool sUsrObjPropsNode::uset(udx val)
{
    sVariant tmp;
    tmp.setUInt(val);
    return set(tmp);
}

bool sUsrObjPropsNode::rset(real val)
{
    sVariant tmp(val);
    return set(tmp);
}

const sUsrObjPropsNode * sUsrObjPropsNode::push(const char * field_name, sVariant &val)
{
    if (!_tree || !field_name)
        return 0;

    sUsrObjPropsTree * tree = _tree; // after pushHelper, our node's allocation may move, so _tree may be invalid
    return tree->getNodeByIndex(tree->pushHelper(_nav.self, field_name, val));
}

const sUsrObjPropsNode * sUsrObjPropsNode::push(const char * field_name, const char * val)
{
    if (!_tree || !field_name)
        return 0;

    sUsrObjPropsTree * tree = _tree; // after pushHelper, our node's allocation may move, so _tree may be invalid
    return tree->getNodeByIndex(tree->pushHelper(_nav.self, field_name, val));
}

const sUsrObjPropsNode * sUsrObjPropsNode::push(const char * field_name, const sHiveId & val)
{
    if (!_tree || !field_name)
        return 0;

    sStr tmp;
    val.print(tmp);
    sUsrObjPropsTree * tree = _tree; // after pushHelper, our node's allocation may move, so _tree may be invalid
    return tree->getNodeByIndex(tree->pushHelper(_nav.self, field_name, tmp.ptr()));
}

const sUsrObjPropsNode * sUsrObjPropsNode::ipush(const char * field_name, idx ival)
{
    if (!_tree || !field_name)
        return 0;

    sVariant tmp(ival);
    sUsrObjPropsTree * tree = _tree; // after pushHelper, our node's allocation may move, so _tree may be invalid
    return tree->getNodeByIndex(tree->pushHelper(_nav.self, field_name, tmp));
}

const sUsrObjPropsNode * sUsrObjPropsNode::upush(const char * field_name, udx uval)
{
    if (!_tree || !field_name)
        return 0;

    sVariant tmp;
    tmp.setUInt(uval);
    sUsrObjPropsTree * tree = _tree; // after pushHelper, our node's allocation may move, so _tree may be invalid
    return tree->getNodeByIndex(tree->pushHelper(_nav.self, field_name, tmp));
}

const sUsrObjPropsNode * sUsrObjPropsNode::rpush(const char * field_name, real rval)
{
    if (!_tree || !field_name)
        return 0;

    sVariant tmp(rval);
    sUsrObjPropsTree * tree = _tree; // after pushHelper, our node's allocation may move, so _tree may be invalid
    return tree->getNodeByIndex(tree->pushHelper(_nav.self, field_name, tmp));
}

#ifdef _DEBUG

#define INDEX_NAME_PATH(i) \
i, TREE_ELT(i) ? TREE_ELT(i)->name() : 0, TREE_ELT(i) ? TREE_ELT(i)->path() : 0

#define FMT_INDEX_NAME_PATH "{ index: %" DEC ", name: \"%s\", path: \"%s\" }"

const char * sUsrObjPropsNode::printDump(sStr & out, bool onlySelf) const
{
    idx start = out.length();

    out.printf("{ index: %" DEC ", row: %" DEC ", name: \"%s\", path: \"%s\", value: \"%s\"\n", _nav.self, _row, name(), path(), value());
    out.printf("  parent: " FMT_INDEX_NAME_PATH ",\n  prev_sib: " FMT_INDEX_NAME_PATH ",\n  next_sib: " FMT_INDEX_NAME_PATH ",\n  first_child: " FMT_INDEX_NAME_PATH ",\n  last_child: " FMT_INDEX_NAME_PATH ",\n  dim: %" DEC ",\n  depth: %" DEC " }\n", INDEX_NAME_PATH(_nav.parent), INDEX_NAME_PATH(_nav.prev_sib), INDEX_NAME_PATH(_nav.next_sib), INDEX_NAME_PATH(_nav.first_child), INDEX_NAME_PATH(_nav.last_child), _nav.dim, _nav.depth);

    if (!onlySelf) {
        for (const sUsrObjPropsNode * child = firstChild(); child; child = child->nextSibling())
            child->printDump(out, false);
    }

    return out.ptr(start);
}

#endif

void sUsrObjPropsTree::init(const sUsrType2 * obj_type, const sHiveId * obj_type_id)
{
    _tree = this;
    _row = -1;
    _type_id.reset();
    if( !obj_type && obj_type_id ) {
        obj_type = sUsrType2::ensure(_usr, *obj_type_id);
    }
    if( obj_type ) {
        _type_id = obj_type->id();
    }
    _table.setColId(COL_ID, "id");
    _table.setColId(COL_NAME, "name");
    _table.setColId(COL_PATH, "group");
    _table.setColId(COL_VALUE, "value");
    _ptable = NULL;
}

bool sUsrObjPropsTree::initialized() const
{
    return _tree && objType();
}

const char * sUsrObjPropsTree::objTypeName() const
{
    const sUsrType2 * utype = objType();
    return utype ? utype->name() : sStr::zero;
}

const sUsrType2 * sUsrObjPropsTree::objType() const
{
    return sUsrType2::ensure(_usr, _type_id);
}

void sUsrObjPropsTree::empty(bool empty_table)
{
    if (empty_table) {
        _table.empty();
        _table.setColId(COL_ID, "id");
        _table.setColId(COL_NAME, "name");
        _table.setColId(COL_PATH, "group");
        _table.setColId(COL_VALUE, "value");
    }
    _nodes.empty();
    _pathMap.empty();

    _nav.first_child = _nav.last_child = -1;
    _nav.dim = 0;
}

sUsrObjPropsTree::sUsrObjPropsTree(const sUsr& usr, const sUsrType2 * obj_type): _usr(usr)
{
    init(obj_type, 0);
    _ptable = &_table;
}

sUsrObjPropsTree::sUsrObjPropsTree(const sUsr& usr, const char * obj_type_name): _usr(usr)
{
    init(sUsrType2::ensure(_usr, obj_type_name), 0);
    _ptable = &_table;
}

sUsrObjPropsTree::sUsrObjPropsTree(const sUsr& usr, const sUsrType2 * obj_type, sVarSet & table): _usr(usr)
{
    init(obj_type, 0);
    useTable(table);
}

sUsrObjPropsTree::sUsrObjPropsTree(const sUsr& usr, const char * obj_type_name, sVarSet & table): _usr(usr)
{
    init(sUsrType2::ensure(_usr, obj_type_name), 0);
    useTable(table);
}

sUsrObjPropsTree::sUsrObjPropsTree(const sUsr& usr, const sUsrType2 * obj_type, const sVar & form): _usr(usr)
{
    init(obj_type, 0);
    useForm(form);
}

sUsrObjPropsTree::sUsrObjPropsTree(const sUsr& usr, const char * obj_type_name, const sVar & form): _usr(usr)
{
    init(sUsrType2::ensure(_usr, obj_type_name), 0);
    useForm(form);
}

sUsrObjPropsTree::~sUsrObjPropsTree()
{
}

bool sUsrObjPropsTree::addPathMapEntry(const char * field_name, const char * path, idx index)
{
    if (!field_name || !path)
        return false;

    sStr field_canonical;
    sString::changeCase(&field_canonical, field_name, 0, sString::eCaseLo);

    sDic<idx> * nameMap = _pathMap.get(path);
    if (!nameMap)
        nameMap = _pathMap.set(path);

    idx * pindex = nameMap->get(field_name);
    if (pindex && *pindex < 0)
        return false;

    if (!pindex)
        pindex = nameMap->set(field_canonical.ptr());

    *pindex = index;
    return true;
}

idx sUsrObjPropsTree::getPathMapEntry(const char * field_name, const char * path) const
{
    if (!field_name || !path)
        return -1;

    sStr field_canonical;
    sString::changeCase(&field_canonical, field_name, 0, sString::eCaseLo);

    const sDic<idx> * nameMap = _pathMap.get(path);
    if (!nameMap)
        return -1;

    const idx * pindex = nameMap->get(field_canonical.ptr());
    if (!pindex)
        return -1;

    return *pindex;
}

sUsrObjPropsNode * sUsrObjPropsTree::addNode(const char * field_name, const char * path)
{
    idx index = _nodes.dim();
    if (!addPathMapEntry(field_name, path, index))
        return 0;

    sUsrObjPropsNode * pnode = _nodes.add(1);
    pnode->_tree = this;
    _namesPrintable.set(field_name, 0, &(pnode->_name));
    pnode->_path.printf("%s", path);
    pnode->_nav.self = index;
    const sUsrTypeField * fld = pnode->field();
    if (fld)
        pnode->_nav.depth = fld->ancestorCount() + 1;
    return pnode;
}

void sUsrObjPropsTree::linkNodeParentSiblings(idx index, idx parent_index)
{
    sUsrObjPropsNode * pparent = TREE_ELT_OR_ROOT(parent_index);
    _nodes[index]._nav.parent = parent_index;
    if (pparent->_nav.first_child < 0) {
        pparent->_nav.first_child = pparent->_nav.last_child = index;
    } else {
        idx prev_sib = pparent->_nav.last_child;
        pparent->_nav.last_child = index;
        _nodes[prev_sib]._nav.next_sib = index;
        _nodes[index]._nav.prev_sib = prev_sib;
    }
    assert(_nodes[index]._nav.depth == pparent->_nav.depth + 1);
    pparent->_nav.dim++;
}

idx countExpectedPathElts(const sUsrTypeField * fld)
{
    idx ret = 0;
    for (; fld; fld = fld->parent()) {
        if (!fld->isArrayRow())
            ret++;
    }
    return ret;
}

bool sUsrObjPropsTree::linkNode(idx cur_index)
{
    if (cur_index < 0 || cur_index >= _nodes.dim())
        return false;

    sStr path;
    path.printf("%s", _nodes[cur_index].path());
    char * path_end = path.ptr(path.length() - 1);

    const sUsrTypeField * cur_fld = _nodes[cur_index].field();

    // generate inner nodes as needed ...
    while (const sUsrTypeField * parent_fld = cur_fld->parent()) {
        // array row pseudo-nodes use the same path as their children
        if (!parent_fld->isArrayRow()) {
            while (path_end > path.ptr() && *path_end >= '0' && *path_end <= '9') {
                path_end--;
            }
            // defend against weird malformed paths
            if (path_end > path.ptr()) {
                *path_end-- = 0;
            } else {
                if (_nodes[cur_index].field()->isGlobalMulti()) {
                    // warn developers that there can be multiple instances of this node, so a valid path is expected
                    // TODO: proper logging API
                    fprintf(stderr, "%s:%u: ERROR: sUsrObjPropsNode of type '%s' value '%s' has a malformed or missing path '%s'; a path of at least %" DEC " elements was expected\n", __FILE__, __LINE__, _nodes[cur_index].name(), _nodes[cur_index].value(), _nodes[cur_index].path(), countExpectedPathElts(_nodes[cur_index].field()));
                }
                path_end = path.cut0cut();
            }
        }

        const char * parent_name = parent_fld->name();
        idx parent_index = getPathMapEntry(parent_name, path);
        if (parent_index < 0) {
            assert(_nodes[cur_index]._nav.parent < 0); 
            sUsrObjPropsNode * pnode = addNode(parent_name, path.ptr());
            if (!pnode)
                return false;
            parent_index = pnode->_nav.self;
        }

        if (_nodes[cur_index]._nav.parent < 0)
            linkNodeParentSiblings(cur_index, parent_index);

        cur_index = parent_index;
        cur_fld = parent_fld;
    }

    // ... and then link the top-level node to top-level siblings, if it wasn't linked already
    if (_nodes[cur_index]._nav.prev_sib == -1 && _nodes[cur_index]._nav.next_sib == -1 && _nav.first_child != cur_index)
        linkNodeParentSiblings(cur_index, -1);

    return true;
}

inline static idx splitPath(sVec<idx> &elts, const char * path)
{
    return sString::sscanfAnyVec<idx>(&elts, path, 0, 0, 0, sIdxMax, "%" DEC, ".");
}

namespace {
    struct SortPathsCallbackParam {
        sVarSet * ptable;
        const sUsrType2 * utype;
        const sUsr * user;
    };
};

static idx sortPathsCallback(void *param, void *arr_param, idx i1, idx i2)
{
    sVarSet *ptable = static_cast<SortPathsCallbackParam*>(param)->ptable;
    const sUsrType2 * utype = static_cast<SortPathsCallbackParam*>(param)->utype;
    const sUsr * user = static_cast<SortPathsCallbackParam*>(param)->user;
    idx *arr = static_cast<idx*>(arr_param);

    sVec<idx> split_path1, split_path2;
    const char * path1 = ptable->val(arr[i1], COL_PATH);
    const char * path2 = ptable->val(arr[i2], COL_PATH);
    const char * name1 = ptable->val(arr[i1], COL_NAME);
    const char * name2 = ptable->val(arr[i2], COL_NAME);
    splitPath(split_path1, path1);
    splitPath(split_path2, path2);

    idx i=0;
    idx dim1 = split_path1.dim();
    idx dim2 = split_path2.dim();
    while (i<dim1 && i<dim2 && split_path1[i] == split_path2[i])
        i++;

    // use field order and then names to break ties
    idx ret = 0;
    if (i == dim1 && i == dim2) {
        if( utype ) {
            const sUsrTypeField * f1 = utype->getField(*user, name1);
            const sUsrTypeField * f2 = utype->getField(*user, name2);
            if( f1 && f2 ) {
                if( f1->order() < f2->order() ) {
                    ret = -1;
                } else if( f1->order() > f2->order() ) {
                    ret = 1;
                }
            }
        }
        if( !ret ) {
            ret = _safe_strcasecmp(name1, name2);
        }
    } else if (i == dim1 && i < dim2) {
        ret = -1;
    } else if (i < dim1 && i == dim2) {
        ret = 1;
    } else {
        ret = split_path1[i] - split_path2[i];
    }

#if 0
    if (i == dim1 && i == dim2) {
        if (ret == 0)
            fprintf(stderr, "%s == %s\n", name1, name2);
        else
            fprintf(stderr, "%s < %s\n", ret<0 ? name1 : name2, ret<0 ? name2 : name1);
    } else {
        if (ret == 0)
            fprintf(stderr, "'%s' == '%s'\n", path1, path2);
        else
            fprintf(stderr, "'%s' < '%s'\n", ret<0 ? path1 : path2, ret<0 ? path2 : path1);
    }
#endif

    return ret;
}

bool sUsrObjPropsTree::parseTable(const sVarSet & table, idx first_row, sVec<idx> * rows_pushable, const char * filter)
{
    // does the table already include type information?
    const char * parsed_type_name = "";
    if (!objType()) {
        for (idx ir=first_row; ir<table.rows; ir++) {
            if (strcmp("_type", table.val(ir, COL_NAME)) == 0) {
                _type_id.reset();
                parsed_type_name = table.val(ir, COL_VALUE);
                if( const sUsrType2 * utype = sUsrType2::ensure(_usr, parsed_type_name) ) {
                    _type_id = utype->id();
                }
                break;
            }
        }
    }

    if (!objType()) {
        fprintf(stderr, "%s:%u: ERROR: object type '%s' not accessible for user '%" UDEC "'\n", __FILE__, __LINE__, parsed_type_name, _usr.Id());
        return false;
    }

    sDic<sDic<idx> > path_map;

    sDic<bool> rows_pushable_dic;
    if (rows_pushable) {
        for (idx i=0; i<rows_pushable->dim(); i++) {
            *(rows_pushable_dic.set(rows_pushable->ptr(i), sizeof(idx))) = true;
        }
    }

    // sort rows by path to ensure correct sibling node order in tree
    sVec<idx> rows_sorted;
    sStr first_id;
    for (idx ir=first_row; ir<table.rows; ir++) {
        if (filter) {
            if (strcmp(filter, table.val(ir, COL_ID)) != 0)
                continue;
        } else {
            first_id.printf("%s", table.val(ir, COL_ID));
            filter = first_id.ptr();
        }

        idx row = ir;
        // if we are not parsing _ptable, append the row to _ptable
        if (&table != _ptable) {
            row = _ptable->rows;
            _ptable->addRow();
            for (idx ic=0; ic<4; ic++)
                _ptable->addCol(table.val(ir, ic));
        }

        rows_sorted.vadd(1, row);
    }
    SortPathsCallbackParam sort_paths_callback_param = { _ptable, objType(), &_usr };
    sSort::sortSimpleCallback<idx>(sortPathsCallback, &sort_paths_callback_param, rows_sorted.dim(), rows_sorted.ptr());

    // load leaf nodes from table rows
    for (idx i=0; i<rows_sorted.dim(); i++) {
        idx row = rows_sorted[i];

        if (rows_pushable_dic.get(&row, sizeof(idx)))
            continue;

        const char * node_name = _ptable->val(row, COL_NAME);
        const char * node_path = _ptable->val(row, COL_PATH);

        // do not add nodes with invalid/deprecated types
        if (!objType()->getField(_usr, node_name)) {
#ifdef _DEBUG
            // TODO: replace with proper log API
            fprintf(stderr, "%s:%u: WARNING: failed to insert node of field type '%s' value '%s' - no field type '%s' in object type '%s'\n", __FILE__, __LINE__, node_name, _ptable->val(row, COL_VALUE), node_name, objTypeName());
#endif
            continue;
        }

        sUsrObjPropsNode * pnode = addNode(node_name, node_path);
        if (pnode) {
            pnode->_row = row;
        } else {
            idx node_index = getPathMapEntry(node_name, node_path);
            _nodes[node_index]._row = row;
        }
    }

    // then try to add those pushable rows which can be added trivially
    sVec<idx> rows_push_recalcitrant;
    for (idx i=0; rows_pushable && i<rows_pushable->dim(); i++) {
        idx row = *(rows_pushable->ptr(i));
        const char * node_name = _ptable->val(row, COL_NAME);
        const char * node_path = _ptable->val(row, COL_PATH);

        idx node_index = _nodes.dim();
        if (!addPathMapEntry(node_name, node_path, node_index)) {
            rows_push_recalcitrant.vadd(1, row);
            continue;
        }

        sUsrObjPropsNode * pnode = addNode(node_name, node_path);
        if (pnode) {
            pnode->_row = row;
        } else {
            rows_push_recalcitrant.vadd(1, row);
        }
    }

    // generate inner nodes as needed and link together
    idx leaves = _nodes.dim();
    for (idx ir=0; ir<leaves; ir++) {
        if (!linkNode(ir)) {
            empty(true);
            return false;
        }
    }

    // individually push the recalcitrant pushable rows
    for (idx i=0; i<rows_push_recalcitrant.dim(); i++) {
        idx row = rows_push_recalcitrant[i];
        const char * node_name = _ptable->val(row, COL_NAME);
        const char * node_val = _ptable->val(row, COL_VALUE);
        if (pushHelper(-1, node_name, node_val) < 0) {
            empty(true);
            return false;
        }
    }

    return true;
}

struct pushFinderData {
    sVec<const sUsrTypeField *> target_ancestry; // ancestors of target field (target field is last)
    const sUsrObjPropsNode * best_node;
    idx best_depth;
    idx target_depth;

    pushFinderData(sUsrObjPropsNode * start_node, const sUsrTypeField * target_field)
    {
        best_node = start_node;
        best_depth = 0;
        const sUsrTypeField * fld = target_field;
        target_depth = 1 + target_field->ancestorCount();
        target_ancestry.resize(target_depth);
        for (idx i = target_depth - 1; i >= 0; i--) {
            assert (fld);
            target_ancestry[i] = fld;
            fld = fld->parent();
        }
    }

    const sUsrTypeField * ancestorAtDepth(idx i) const
    {
        // top-level non-root nodes have depth 1; top-level ancestor has index 0
        if (i < 1 || i > target_depth)
            return 0;
        return target_ancestry[i - 1];
    }
};

static sUsrObjPropsNode::FindStatus pushFinder(const sUsrObjPropsNode& node, void * param)
{
    pushFinderData * pdata = static_cast<pushFinderData*>(param);

    if (node.depth() >= pdata->target_depth) {
        // node is as deep as target - time to move sideways
        return sUsrObjPropsNode::eFindPrune;
    }

    if (node.depth() <= pdata->best_depth) {
        return sUsrObjPropsNode::eFindContinue;
    }

    if (node.field() == pdata->ancestorAtDepth(node.depth())) {
        // if this node already has a child of type field_stack[i-1],
        // can we add another one?
        const sUsrTypeField * child_fld = pdata->ancestorAtDepth(node.depth() + 1);
        const sUsrObjPropsNode * cur_child = node.firstChild(child_fld->name());
        if (!cur_child || (child_fld->isMulti() && !node.field()->isArrayRow())) {
            pdata->best_node = &node;
            pdata->best_depth = node.depth();
            if (node.depth() + 1 == pdata->target_depth) {
                // can't get any better than this!
                return sUsrObjPropsNode::eFindStop;
            }
        }
    }

    return sUsrObjPropsNode::eFindContinue;
}

idx sUsrObjPropsTree::pushHelper(idx node_index, const char * field_name, sVariant &val)
{
    sVariant tmp;
    const sUsrTypeField * target_field = objType()->getField(_usr, field_name);
    if (!target_field)
        return -sIdxMax;

    const char * s = toValueString(val, target_field->type(), tmp);
    if (!s)
        return -sIdxMax;

    return pushHelper(node_index, field_name, s);
}

idx sUsrObjPropsTree::pushHelper(idx node_index, const char * field_name, const char * value)
{
    assert(objType());

    const sUsrTypeField * target_field = objType()->getField(_usr, field_name);
    if (!target_field) {
#ifdef _DEBUG
        // TODO: replace with proper log API
        fprintf(stderr, "%s:%u: WARNING: failed to insert node of field type '%s' value '%s' - no field type '%s' in object type '%s'\n", __FILE__, __LINE__, field_name, value, field_name, objTypeName());
#endif
        return -sIdxMax;
    }
    sUsrObjPropsNode * pnode = TREE_ELT_OR_ROOT(node_index);
    if (target_field == pnode->field() || (value && !target_field->canHaveValue()))
        return -sIdxMax;

    // ensure that we are not repeating a singleton field
    if (!target_field->isGlobalMulti() && find(field_name))
        return -sIdxMax;

    if (!value && target_field->canHaveValue())
        value = target_field->defaultValue();

    pushFinderData data(pnode, target_field);

    // ensure that starting node's field is a strict ancestor of the target field
    if (data.ancestorAtDepth(pnode->depth()) != pnode->field() || target_field == pnode->field())
        return -sIdxMax;

    // find the best node where to push
    pnode->findBackwards(0, pushFinder, &data);
    pnode = TREE_ELT_OR_ROOT(data.best_node->_nav.self);

    // create new nodes
    sStr prefix;
    findIdString(prefix);
    sStr node_path("%s", pnode->path() ? pnode->path() : "");

    for (idx i = data.best_depth + 1; i <= data.target_depth; i++) {
        idx parent = pnode->_nav.self;
        idx sib = pnode->_nav.last_child;
        // If we are at the root and adding a multivalue field, the relevant
        // sibling is the previous node of the same field type
        if (parent < 0 && sib >= 0) {
            sib = -1;
            if (data.ancestorAtDepth(i)->isMulti()) {
                for (idx j=pnode->_nav.last_child; j >= 0; j=TREE_ELT(j)->_nav.prev_sib) {
                    if (TREE_ELT(j)->field() == data.ancestorAtDepth(i)) {
                        sib = j;
                        break;
                    }
                }
            }
        }
        const sUsrTypeField * parent_fld = pnode->field();
        bool parent_is_array_row = parent_fld && parent_fld->isArrayRow();

        // If we are adding a top-level node, use its field's order as
        // path by default. (Unless this is a multivalue field, and there
        // are previous siblings of the same field type.) Otherwise, use
        // 1 + sibling's path. The exception is array row pseudo-fields,
        // which use same path as their children.
        idx node_path_seg = parent<0 ? sMax<idx>(1, floor(data.ancestorAtDepth(i)->order())) : 1;
        if ((parent_fld || parent < 0) && sib >= 0 && !parent_is_array_row) {
            const char * sib_path = TREE_ELT(sib)->path();
            if (!sib_path)
                sib_path = sStr::zero;
            const char * sib_path_seg_str = strrchr(sib_path, '.');
            if (sib_path_seg_str) {
                sib_path_seg_str++;
            } else {
                sib_path_seg_str = sib_path;
            }

            node_path_seg = 1 + atoidx(sib_path_seg_str);
        }

        if (!parent_is_array_row) {
            if (node_path.length())
                node_path.printf(".");

            node_path.printf("%" DEC, node_path_seg);
        }

        pnode = addNode(data.ancestorAtDepth(i)->name(), node_path.ptr());
        linkNodeParentSiblings(pnode->_nav.self, parent);
    }

    if (target_field->canHaveValue()) {
        pnode->_row = _ptable->rows;
        _ptable->addRow().addCol(prefix.ptr()).addCol(pnode->name()).addCol(pnode->path()).addCol(value);
    }
    return pnode->_nav.self;
}

const char * sUsrObjPropsTree::findIdString(sStr & buf) const
{
    buf.cut(0);
    for (idx i=0; i<_nodes.dim(); i++) {
        if (_nodes[i]._row >= 0) {
            buf.printf("%s", _ptable->val(_nodes[i]._row, COL_ID));
            break;
        }
    }

    if (!buf.length())
        buf.printf("%s", objType()->name());

    buf.add0();

    return buf.ptr();
}

bool sUsrObjPropsTree::useTable(sVarSet & table, const char * filter)
{
    empty(&table != &_table);
    _ptable = &table;
    return parseTable(table, 0, 0, filter);
}

bool sUsrObjPropsTree::addTable(const sVarSet & table, const char * filter)
{
    return parseTable(table, 0, 0, filter);
}

bool sUsrObjPropsTree::useForm(const sVar & form, const char * filter)
{
    _table.empty();
    _ptable = &_table;
    return addForm(form, filter);
}

bool sUsrObjPropsTree::addForm(const sVar & form, const char * filter)
{
    sVec<idx> push_rows;
    idx first_row=_table.rows;
    idx ir = first_row;

    sStr first_prefix, prefix, name, path;

    for (idx i=0; i<form.dim(); i++) {
        prefix.cut(0);
        name.cut(0);
        path.cut(0);

        const char * key = static_cast<const char*>(form.id(i));
        idx key_len = sLen(key);
        static const char * key_start = "prop.";
        static const idx key_start_len = sLen(key_start);

        // format: prop.VALID_HIVE_ID.PROP_NAME[.PATH] or prop.SERVICE_PREFIX.PROP_NAME[.PATH]
        if (key_len < key_start_len || strncmp(key, key_start, key_start_len) != 0)
            continue;

        sHiveId tmpId;
        idx prefix_len = tmpId.parse(key + key_start_len);
        if (prefix_len) {
            prefix.add(key + key_start_len, prefix_len);
            prefix.add0();
            name.add0(key_len + 1 - key_start_len - prefix_len);
            path.add0(key_len + 1 - key_start_len - prefix_len);
            if (sscanf(key + key_start_len + prefix_len, ".%[^.].%s", name.ptr(), path.ptr()) < 1) {
                continue;
            }
        } else {
            prefix.add0(key_len + 1 - key_start_len);
            name.add0(key_len + 1 - key_start_len);
            path.add0(key_len + 1 - key_start_len);
            if (sscanf(key + key_start_len, "%[a-zA-Z0-9_-].%[^.].%s", prefix.ptr(), name.ptr(), path.ptr()) < 2) {
                continue;
            }
        }

        if (filter) {
            if (strcmp(filter, prefix.ptr()) != 0)
                continue;
        } else {
            first_prefix.printf("%s", prefix.ptr());
            filter = first_prefix.ptr();
        }

        idx name_len = sLen(name.ptr());
        if (name_len && name[name_len-1] == '+') {
            name[name_len-1] = 0;
            name.cut(name_len-1);
            push_rows.vadd(1, ir);
        }

        _table.addRow().addCol(prefix.ptr()).addCol(name.ptr()).addCol(path.ptr()).addCol(form.value(key));
        ir++;
    }

    return parseTable(_table, first_row, &push_rows, filter);
}

const sUsrObjPropsNode * sUsrObjPropsTree::getNodeByIndex(idx i) const
{
    if (i < -1 || i >= _nodes.dim())
        return 0;

    return TREE_ELT_OR_ROOT(i);
}

#define VALIDATION_FAIL(name, path, msg) \
do { \
    res = false; \
    if (log) { \
        log->printf("err.%s.%s", prefix.ptr(), (name)); \
        if (path) \
            log->printf(".%s", (path) ? (path) : ""); /* ?: syntax silences silly gcc warning */ \
        log->printf("=\"%s\"\n", msg); \
    } else { \
        goto RET; \
    } \
} while(0);

bool sUsrObjPropsTree::valid(sStr * log) const
{
    bool res = true;

    sStr prefix;
    findIdString(prefix);
    sDic<sVec<idx> > singleton_map;

    for (idx i=0; i<_nodes.dim(); i++) {
        if (!_nodes[i].field()) {
            VALIDATION_FAIL(_nodes[i].name(), _nodes[i].path(), "object type does not have field with this name");
            continue;
        }
        if (_nodes[i].hasValue()) {
            if (!_nodes[i].field()->canHaveValue())
                VALIDATION_FAIL(_nodes[i].name(), _nodes[i].path(), "this node must not have a value");
            if (!isValueType(_nodes[i].value(), _nodes[i].type()))
                VALIDATION_FAIL(_nodes[i].name(), _nodes[i].path(), "value fails to parse into expected data type");
        }
        if (!_nodes[i].field()->isGlobalMulti()) {
            singleton_map.set(_nodes[i].field()->name())->vadd(1, i);
        }
    }

    for (idx i=0; i<singleton_map.dim(); i++) {
        if (singleton_map[i].dim() <= 1)
            continue;

        for (idx j=0; j<singleton_map[i].dim(); j++) {
            VALIDATION_FAIL(_nodes[singleton_map[i][j]].name(), _nodes[singleton_map[i][j]].path(), "this field cannot have multiple values in one object");
        }
    }

  RET:
    return res;
}

bool sUsrObjPropsTree::complete(sStr * log) const
{
    bool res = true;

    sStr prefix;
    findIdString(prefix);

    idx nfields = objType()->dimFields(_usr);
    for (idx i=0; i<nfields; i++) {
        const sUsrTypeField * fld = objType()->getField(_usr, i);
        if (fld->isOptional())
            continue;

        // Ignore system fields like _type, _parent
        if (fld->name()[0] == '_')
            continue;

        bool found = false;
        for (idx j=0; j<_nodes.dim(); j++) {
            if (_nodes[j].field() == fld) {
                found = true;
                break;
            }
        }
        if (!found) {
            VALIDATION_FAIL(fld->name(), 0, "missing required value");
        }
    }
  RET:
    return res;
}

#ifdef _DEBUG
const char * sUsrObjPropsTree::printDump(sStr & out, bool onlySelf) const
{
    idx start = out.length();

    for (idx i=0; i<_nodes.dim(); i++)
        _nodes[i].printDump(out, true);

    return out.ptr(start);
}
#endif
