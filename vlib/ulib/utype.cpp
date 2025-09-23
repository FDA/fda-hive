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
#error "Deprecated, switch to sUsrType2"

#include <ulib/uobj.hpp>
#include <ulib/uprop.hpp>
#include <ulib/utype.hpp>
#include <slib/utils.hpp>
#include "uperm.hpp"

#include <memory>

using namespace slib;

#define _DEBUG_OBJ_FIELD_TYPE_off

static const char * propTypeNames =
    "string"_
    "text"_
    "integer"_
    "real"_
    "bool"_
    "array"_
    "list"_
    "date"_
    "time"_
    "datetime"_
    "url"_
    "obj"_
    "password"_
    "file" __;

#define PROP_TYPE_DEFAULT eUsrObjProp_invalid

inline static const eUsrObjPropType propTypeFromName(const char *prop_type_name)
{
    idx prop_type = PROP_TYPE_DEFAULT;
    if( prop_type_name && sString::compareChoice(prop_type_name, propTypeNames, &prop_type, true, 0, true) >= 0 ) {
        return (eUsrObjPropType) prop_type;
    }
    return PROP_TYPE_DEFAULT;
}

static const char * canonicalize(sStr & buf, const char * name)
{
    idx i = 0;
    if( !name ) {
        return 0;
    }
    while( name[i] && (name[i] < 'A' || name[i] > 'Z') ) {
        i++;
    }
    if( name[i] ) {
        idx start = buf.length();
        sString::changeCase(&buf, name, 0, sString::eCaseLo);
        return buf.ptr(start);
    }

    return name;
}

class TFieldDef;
struct TTypeDef;

sDic<TTypeDef> s_types;
sDic<idx> s_type_ids;
sUsrObjRes s_all_actions;
static sVarSet s_props_template;

struct TTypeDef
{
    sStr name;
    sStr title;
    sStr description;
    sHiveId id;
    idx type_index;
    bool isAbstract;
    bool isUser;
    bool isSystem;
    bool inherited;
    sVarSet props;
    sDic<TFieldDef*> field_defs;
    bool broken_field_defs;
    sDic<idx> parents;
    sDic<idx> prop_name_row_id;
    sDic<std::auto_ptr<sUsrObj> > views;
    TActions acts;
    sStr brief;
    sStr summary;
    sVarSet props_tree_table;
    bool props_tree_is_default;
    sUsrObjPropsTree * props_tree;

    TTypeDef()
        : name(sMex::fExactSize), title(sMex::fExactSize), type_index(sNotIdx), isAbstract(false), isUser(false), isSystem(false), inherited(false), broken_field_defs(false)
    {
        views.mex()->flags |= sMex::fSetZero;
        acts.mex()->flags |= sMex::fSetZero;
        prop_name_row_id.mex()->flags |= sMex::fSetZero;
        props_tree = 0;
        props_tree_is_default = false;
    }

    ~TTypeDef();

    static const char * tolower(const char * nm)
    {
        static sStr lower(sMex::fExactSize);
        lower.cut0cut();
        if( nm && nm[0] ) {
            sString::changeCase(&lower, nm, 0, sString::eCaseLo);
        }
        return lower;
    }

    static TTypeDef * find(idx tid)
    {
        if( tid >= 0 && tid < s_types.dim() ) {
            return s_types.ptr(tid);
        }
        return 0;
    }
    static TTypeDef * find(const char * typeName)
    {
        return s_types.get(tolower(typeName));
    }
    static TTypeDef * find(const sHiveId & id)
    {
        idx * tid = s_type_ids.get(&id, sizeof(id));
        if( tid ) {
            return s_types.ptr(*tid);
        }
        return 0;
    }

    bool inherit();

    TFieldDef * ensureFieldDef(const char * propName, bool needCanonicalize, bool arrayRow);

    TFieldDef * getFieldDef(const char * propName, bool needCanonicalize)
    {
        sStr buf;
        if( needCanonicalize ) {
            propName = canonicalize(buf, propName);
        }
        TFieldDef ** ppf = field_defs.get(propName);
        return ppf ? *ppf : 0;
    }
};

class TFieldDef: public sUsrObjTypeField {
protected:
    idx _obj_type_index;
    idx _row;
    idx _defs_index;
    idx _parent;
    mutable idx _ancestor_count;
    eUsrObjPropType _type;
    sDic<bool> _children;

    bool _flattened_multi;
    bool _global_multi;
    bool _flattened_decor;
    idx _flattened_parent;
    sDic<bool> _flattened_children;
    bool _flattened;

    friend class TArrayRowDef;

    void setChild(sDic<bool> & children, idx child_defs_index, bool value)
    {
        *(children.set(&child_defs_index, sizeof(idx))) = value;
    }

    void addChild(TFieldDef * child)
    {
        setChild(_children, child->_defs_index, true);
    }

    void addFlattenedChild(TFieldDef * child)
    {
        setChild(_flattened_children, child->_defs_index, true);
    }

    void delChild(TFieldDef * child)
    {
        setChild(_children, child->_defs_index, false);
    }

    void delFlattenedChild(TFieldDef * child)
    {
        setChild(_flattened_children, child->_defs_index, false);
    }

    TFieldDef * getChild(idx i) const
    {
        const bool * phas_child = _children.ptr(i);
        if( !phas_child || !*phas_child )
            return 0;

        idx child_defs_index = *static_cast<const idx*>(_children.id(i));
        return getDef(child_defs_index);
    }

    TFieldDef * getFlattenedChild(idx i) const
    {
        const bool * phas_child = _flattened_children.ptr(i);
        if( !phas_child || !*phas_child )
            return 0;

        idx child_defs_index = *static_cast<const idx*>(_flattened_children.id(i));
        return getDef(child_defs_index);
    }

    sDic<TFieldDef*> & getDefs() const
    {
        return TTypeDef::find(_obj_type_index)->field_defs;
    }

    TFieldDef * getDef(idx defs_index) const
    {
        TFieldDef ** ppf = getDefs().ptr(defs_index);
        return ppf ? *ppf : 0;
    }

    const char * getPropsVal(const char * col_name) const
    {
        sVarSet & props = TTypeDef::find(_obj_type_index)->props;
        return props.val(_row, props.colId(col_name));
    }

    const char * getObjTypeName() const
    {
        return TTypeDef::find(_obj_type_index)->name.ptr();
    }

public:
    TFieldDef(idx obj_type_index, idx defs_index)
    {
        _row = _parent = _flattened_parent = _ancestor_count = -1;
        _obj_type_index = obj_type_index;
        _defs_index = defs_index;
        _flattened_multi = _flattened_decor = _global_multi = false;
        _flattened = false;
        _type = PROP_TYPE_DEFAULT;
    }

    virtual ~TFieldDef() {}

    virtual void init(idx row)
    {
        _row = row;
        _type = propTypeFromName(getPropsVal("type"));

        const char * parent_name = getPropsVal("parent");
        if( parent_name && *parent_name ) {
            TFieldDef * pparent = TTypeDef::find(_obj_type_index)->getFieldDef(parent_name, true);
            assert (pparent);

            _parent = pparent->_defs_index;
            pparent->setChild(pparent->_children, _defs_index, true);
        }
    }

    virtual bool isInitialized() const
    {
        return _defs_index >= 0 && _row >= 0;
    }

    bool cycleCheck(sDic<bool> &seen)
    {
        bool * phas = seen.set(&_defs_index, sizeof(_defs_index));
        if( *phas ) {
            fprintf(stderr, "%s:%u: ERROR: object field hierarchy cycle detected: object type '%s' prop '%s'\n", __FILE__, __LINE__, getObjTypeName(), name());
            return false;
        }
        *phas = true;
        for( idx ic=0; ic<_children.dim(); ic++ ) {
            TFieldDef * child = getChild(ic);
            if( !child)
                continue;
            if( !child->cycleCheck(seen) )
                return false;
        }
        return true;
    }

    virtual bool sanityCheck()
    {
        if( !isInitialized() ) {
            fprintf(stderr, "%s:%u: ERROR: uninitialized sUsrObjTypeField: object type '%s', prop #%" DEC "\n", __FILE__, __LINE__, getObjTypeName(), _defs_index);
            return false;
        }

        if( parent() && !parent()->isInitialized() ) {
            fprintf(stderr, "%s:%u: ERROR: parent field invalid or missing: object type '%s' prop '%s'\n", __FILE__, __LINE__, getObjTypeName(), name());
            return false;
        }

        switch( type() ) {
        case eUsrObjProp_invalid:
            fprintf(stderr, "%s:%u: ERROR: invalid data type: object type '%s' prop '%s'\n", __FILE__, __LINE__, getObjTypeName(), name());
            return false;
        case eUsrObjProp_list:
        case eUsrObjProp_array:
            {
                sDic<bool> seen;
                if( !cycleCheck(seen) )
                    return false;
            }
            break;
        default:
            if( _children.dim() ) {
                fprintf(stderr, "%s:%u: ERROR: property which is neither list nor array has child properties: object type '%s' prop '%s'\n", __FILE__, __LINE__, getObjTypeName(), name());
                return false;
            }
        }

        return true;
    }

    void flatten()
    {
        if( _flattened )
            return;

        _flattened = true;
        _flattened_decor = false;
        _global_multi = isMulti() || (parent() && parent()->globalMulti());
        _flattened_multi = isMulti() && !(parent() && parent()->isArrayRow());
        _flattened_parent = _parent;
        for(idx ic=0; ic<_children.dim(); ic++) {
            if( TFieldDef * child = getChild(ic) )
                addFlattenedChild(child);
        }

        if( type() != eUsrObjProp_list && type() != eUsrObjProp_array )
            return;

        idx initial_dim_children = _flattened_children.dim();
        idx non_decor_children = 0;
        idx non_decor_multi_children = 0;

        for(idx ic=0; ic<initial_dim_children; ic++) {
            if( TFieldDef * child = getFlattenedChild(ic) ) {
                child->flatten();
            }
        }

        for(idx ic=0; ic<_flattened_children.dim(); ic++) {
            TFieldDef * child = getFlattenedChild(ic);
            if( !child )
                continue;

            if( !child->flattenedDecor() ) {
                non_decor_children++;
                if( child->flattenedMulti() )
                    non_decor_multi_children++;
            }
        }

        if( non_decor_children >= 2 && (non_decor_multi_children || isMulti()) )
            return;

        if( isArrayRow() )
            return;

#ifdef _DEBUG_OBJ_FIELD_TYPE
        fprintf(stderr, "Marking object type '%s' prop '%s' (row %" DEC ", defs_index %" DEC ") as decorative\n", getObjTypeName(), name(), _row, _defs_index);
#endif

        _flattened_decor = true;

        for(idx ic=0; ic<_flattened_children.dim(); ic++) {
            TFieldDef * child = getFlattenedChild(ic);
            if( !child )
                continue;

            if( isMulti() )
                child->_flattened_multi = true;

            if( flattenedParent() )
                flattenedParent()->addFlattenedChild(child);

            child->_flattened_parent = _flattened_parent;
            delFlattenedChild(child);
        }

        if( flattenedParent() )
            flattenedParent()->delFlattenedChild(this);

        _flattened_parent = -1;
    }

    virtual const char * name() const
    {
        return getPropsVal("name");
    }
    virtual const char * title() const
    {
        return getPropsVal("title");
    }
    virtual eUsrObjPropType type() const
    {
        return _type;
    }
    virtual const char * typeName() const
    {
        return getPropsVal("type");
    }
    virtual const TFieldDef* parent() const
    {
        return _parent >= 0 ? getDef(_parent) : 0;
    }
    TFieldDef* parent()
    {
        return const_cast<TFieldDef*>(static_cast<const TFieldDef&>(*this).parent());
    }
    virtual idx children(sVec<const sUsrObjTypeField*> &out) const
    {
        idx count = 0;
        for( idx i=0; i<_children.dim(); i++) {
            if( TFieldDef * child = getChild(i) ) {
                count++;
                out.vadd(1, child);
            }
        }
        return count;
    }
    virtual bool isKey() const
    {
        return sString::parseBool(getPropsVal("is_key_fg"));
    }
    virtual bool isReadonly() const
    {
        return sString::parseBool(getPropsVal("is_readonly_fg"));
    }
    virtual bool isWriteonce() const
    {
        const char * ro = getPropsVal("is_readonly_fg");
        return ro && atoidx(ro) == -1;
    }
    virtual bool isAutofill() const
    {
        const char * ro = getPropsVal("is_readonly_fg");
        return ro && atoidx(ro) == 2;
    }
    virtual bool isSubmitonce() const
    {
        const char * ro = getPropsVal("is_readonly_fg");
        return ro && atoidx(ro) == -2;
    }
    virtual bool isOptional() const
    {
        return sString::parseBool(getPropsVal("is_optional_fg"));
    }
    virtual bool isMulti() const
    {
        return sString::parseBool(getPropsVal("is_multi_fg"));
    }
    virtual bool isHidden() const
    {
        return sString::parseBool(getPropsVal("is_hidden_fg"));
    }
    virtual bool isSummary() const
    {
        return sString::parseBool(getPropsVal("is_summary_fg"));
    }
    virtual bool isVirtual() const
    {
        return sString::parseBool(getPropsVal("is_virtual_fg"));
    }
    virtual bool isBatch() const
    {
        return sString::parseBool(getPropsVal("is_batch_fg"));
    }
    virtual const char * brief() const
    {
        return getPropsVal("brief");
    }
    virtual real order() const
    {
        const char * order = getPropsVal("order");
        return order ? atof(order) : 0;
    }
    virtual const char * orderString() const
    {
        return getPropsVal("order");
    }
    virtual const char * defaultValue() const
    {
        return getPropsVal("default_value");
    }
    virtual const char * constraint() const
    {
        return getPropsVal("constraint");
    }
    virtual const char * constraintData() const
    {
        return getPropsVal("constraint_data");
    }
    virtual const char * constraintDescription() const
    {
        return getPropsVal("constraint_description");
    }
    virtual const char * linkUrl() const
    {
        return getPropsVal("link_url");
    }
    virtual const char * description() const
    {
        return getPropsVal("description");
    }

    virtual bool canHaveValue() const
    {
        eUsrObjPropType t = type();
        return t != eUsrObjProp_invalid && t != eUsrObjProp_list && t != eUsrObjProp_array && !flattenedDecor();
    }

    virtual bool flattenedDecor() const
    {
        return _flattened_decor;
    }

    virtual bool flattenedMulti() const
    {
        return _flattened_multi;
    }

    virtual const TFieldDef* flattenedParent() const
    {
        return _flattened_parent >= 0 ? getDef(_flattened_parent) : 0;
    }

    TFieldDef* flattenedParent()
    {
        return const_cast<TFieldDef*>(static_cast<const TFieldDef&>(*this).flattenedParent());
    }

    virtual idx flattenedChildren(sVec<const sUsrObjTypeField*> &out) const
    {
        idx count = 0;
        for( idx i=0; i<_flattened_children.dim(); i++) {
            if( TFieldDef * child = getFlattenedChild(i) ) {
                count++;
                out.vadd(1, child);
            }
        }
        return count;
    }

    bool wasFlattened() const
    {
        return _flattened;
    }

    virtual bool isArrayRow() const
    {
        return false;
    }

    virtual bool globalMulti() const
    {
        return _global_multi;
    }

    virtual idx ancestorCount() const
    {
        if( _ancestor_count < 0 ) {
            _ancestor_count = parent() ? parent()->ancestorCount() + 1 : 0;
        }
        return _ancestor_count;
    }

#define TRUEFALSE(s) ((s) ? "true" : "false")

    virtual const char * printDump(sStr & out)
    {
        idx start = out.length();
        out.printf("{ type_index: %" DEC ", defs_index: %" DEC ", row: %" DEC ", parent: { index: %" DEC ", name: '%s' },\n", _obj_type_index, _defs_index, _row, _parent, _parent >= 0 ? parent()->name() : 0);
        out.printf("  name: '%s', type: %d, type_name: '%s', multi: %s, flattened_multi: %s, global_multi: %s, optional: %s\n", name(), type(), typeName(), TRUEFALSE(isMulti()), TRUEFALSE(flattenedMulti()), TRUEFALSE(globalMulti()), TRUEFALSE(isOptional()));

        idx count = 0;
        out.printf("  children: [");
        for( idx i=0; i<_children.dim(); i++ ) {
            if( TFieldDef * child = getChild(i) ) {
                out.printf("%s{index: %" DEC ", name: '%s'}", count ? ", " : "", child->_defs_index, child->name());
                count++;
            }
        }

        count = 0;
        out.printf("],\n  flattened_children: [");
        for( idx i=0; i<_flattened_children.dim(); i++ ) {
            if( TFieldDef * child = getFlattenedChild(i) ) {
                out.printf("%s{index: %" DEC ", name: '%s'}", count ? ", " : "", child->_defs_index, child->name());
                count++;
            }
        }
        out.printf("] }\n");

        return out.ptr(start);
    }
};

static const char * t_array_row_def_title = "Array row";
static const char * t_array_row_def_type_name = "list";

class TArrayRowDef : public TFieldDef {
public:
    TArrayRowDef(idx obj_type_index, idx defs_index) : TFieldDef(obj_type_index, defs_index)
    {
        _type = eUsrObjProp_list;
    }

    virtual ~TArrayRowDef() {}

    virtual void init(idx parent_index)
    {
        _parent = parent_index;

        for( idx i=0; i<parent()->_children.dim(); i++ ) {
            if( TFieldDef * child = parent()->getChild(i) ) {
                addChild(child);
                child->_parent = _defs_index;
            }
        }
        parent()->_children.empty();
        parent()->addChild(this);
    }

    virtual bool isInitialized() const
    {
        return _defs_index >= 0 && _parent >= 0;
    }

    virtual bool sanityCheck()
    {
        if( !parent() || parent()->type() != eUsrObjProp_array ) {
            fprintf(stderr, "%s:%u: ERROR: array row field requires an array parent: object type '%s' prop '%s'\n", __FILE__, __LINE__, getObjTypeName(), name());
        }

        sDic<bool> seen;
        if( !cycleCheck(seen) )
            return false;

        return true;
    }

    virtual const char * name() const
    {
        return static_cast<const char*>(getDefs().id(_defs_index));
    }
    virtual const char * title() const
    {
        return t_array_row_def_title;
    }
    virtual const char * typeName() const
    {
        return t_array_row_def_type_name;
    }
    virtual bool isKey() const
    {
        return false;
    }
    virtual bool isReadonly() const
    {
        return true;
    }
    virtual bool isWriteonce() const
    {
        return false;
    }
    virtual bool isSubmitonce() const
    {
        return false;
    }
    virtual bool isAutofill() const
    {
        return false;
    }
    virtual bool isOptional() const
    {
        return true;
    }
    virtual bool isMulti() const
    {
        return true;
    }
    virtual bool isHidden() const
    {
        return true;
    }
    virtual bool isSummary() const
    {
        return false;
    }
    virtual bool isVirtual() const
    {
        return true;
    }
    virtual bool isBatch() const
    {
        return false;
    }
    virtual const char * brief() const
    {
        return sStr::zero;
    }
    virtual real order() const
    {
        return 0;
    }
    virtual const char * orderString() const
    {
        return "0";
    }
    virtual const char * defaultValue() const
    {
        return sStr::zero;
    }
    virtual const char * constraint() const
    {
        return sStr::zero;
    }
    virtual const char * constraintData() const
    {
        return sStr::zero;
    }
    virtual const char * constraintDescription() const
    {
        return sStr::zero;
    }
    virtual const char * linkUrl() const
    {
        return sStr::zero;
    }
    virtual const char * description() const
    {
        return sStr::zero;
    }
    virtual bool isArrayRow() const
    {
        return true;
    }
};

TTypeDef::~TTypeDef()
{
    for(idx i = 0; i < field_defs.dim(); i++) {
        delete *(field_defs.ptr(i));
    }
    delete props_tree;
}

TFieldDef * TTypeDef::ensureFieldDef(const char * propName, bool needCanonicalize, bool arrayRow)
{
    sStr buf;

    if( arrayRow ) {
        buf.printf("_row_%s", propName);
        propName = buf.ptr();
    }

    if( needCanonicalize )
        propName = canonicalize(buf, propName);

    TFieldDef ** ppf = field_defs.set(propName);
    if( *ppf )
        return *ppf;

    idx defsIndex = field_defs.find(propName) - 1;

    if( arrayRow )
        *ppf = new TArrayRowDef(type_index, defsIndex);
    else
        *ppf = new TFieldDef(type_index, defsIndex);

    return *ppf;
}

bool TTypeDef::inherit()
{
    static sDic<idx> chain;

    while( !inherited ) {
        idx * key = chain.get(&type_index, sizeof(type_index));
        if( key && *key != 0) {
            fprintf(stderr, "%s:%u: ERROR: type '%s' has loop in inheritance/use\n", __FILE__, __LINE__, name.ptr());
            break;
        }
        key = chain.set(&type_index, sizeof(type_index));
        if( !key ) {
            break;
        }
        *key = 1;

        static const idx colName = props.colId("name");
        static const idx colBrief = props.colId("brief");
        static const idx colSummary= props.colId("is_summary_fg");
        static const idx colMulti = props.colId("is_multi_fg");
        static const idx colVirtual = props.colId("is_virtual_fg");
        static const idx colHidden= props.colId("is_hidden_fg");
        static const idx colReadonly = props.colId("is_readonly_fg");
        static const idx colType = props.colId("type");
        static const idx colTitle = props.colId("title");
        static const idx colDesc = props.colId("description");
        static const idx colParent = props.colId("parent");
        static const idx colConstData = props.colId("constraint_data");
        static const idx colRole = props.colId("role");
        static const idx colKey = props.colId("is_key_fg");

        idx i = 0;
        for(i = 0; i < props.rows; ++i) {
            if( sIs("type2", props.val(i, colType)) ) {
                const char * use = props.val(i, colConstData);
                if( !use ) {
                    break;
                }
                TTypeDef * t = TTypeDef::find(use);
                if( !t || !t->inherit() ) {
                    break;
                }
                sStr buf;
                sStr prefix("$(%s_", props.val(i, colName));
                prefix.add0(2);
                for(idx r = 0; r < t->props.rows; ++r) {
                    if( t->props.val(r, colName)[0] != '_' ) {
                        props.addRow();
                        for(idx c = 0; c < t->props.cols; ++c) {
                            const char * p = t->props.val(r, c);
                            if( c == colTitle || c == colRole || c == colKey || c == colType ) {
                            } else if( c == colName ) {
                                p = buf.printf(0, "%s%s", prefix.ptr(2), p);
                            } else if( c == colParent ) {
                                if( p && *p ) {
                                    p = buf.printf(0, "%s%s", prefix.ptr(2),p);
                                } else {
                                    p = buf.printf(0, "%s", prefix.ptr(2));
                                    buf.cut0cut(-1);
                                }
                            } else if( p && *p ) {
                                idx pos = buf.length();
                                sString::searchAndReplaceStrings(&buf, p, 0, "$(" __, prefix, 0, true);
                                p = buf.ptr(pos);
                            }
                            props.addCol(p);
                        }
                    }
                }
                buf.printf(0, "%s", &(props.val(i, colType)[5]));
                props.updateVal(i, colType, buf);
                props.updateVal(i, colConstData, "");
            }
        }
        const idx pq = parents.dim();
        for(i = (i == props.rows) ? 1 : (pq + 1); i < pq; ++i) {
            idx parIdx = *(parents.ptr(i));
            TTypeDef * par = TTypeDef::find(parIdx);
            if( par ) {
                if( !par->inherit() ) {
                    break;
                }
                idx p = 0;
                for(; p < par->parents.dim(); ++p ) {
                    if( par->type_index == type_index ) {
                    }
                    idx * pidx = parents.setString((const char*)par->parents.id(p));
                    if( !pidx ) {
                        break;
                    }
                    *pidx = *par->parents.ptr(p);
                }
                if( p != par->parents.dim() ) {
                    break;
                }
                for(idx r = 0; r < par->props.rows; ++r) {
                    if( par->props.val(r, colName)[0] != '_' ) {
                        props.addRow();
                        for(idx c = 0; c < par->props.cols; ++c) {
                            props.addCol(par->props.val(r, c));
                        }
                    }
                }
                if( par->isUser ) {
                    isUser = true;
                }
                if( par->isSystem ) {
                    isSystem = true;
                }
            } else {
                break;
            }
        }
        if( i == pq ) {
            struct
            {
                    const char * name, * title, * type;
                    idx isMulti;
            } builtins[] = {
                { "_type", "Type", "string", false },
                { "_parent", "Parent type", "obj", true },
                { "_brief", "Summary", "string", false },
                { "_dir", "Location", "string", true },
                { "_perm", "Permissions", "string", true }
            };
            sStr tmp_overlapping;
            for(idx b = 0; b < sDim(builtins); ++b) {
                props.addRow();
                for(idx c = 0; c < s_props_template.cols; ++c) {
                    if( c == colName ) {
                        props.addCol(builtins[b].name);
                    } else if( c == colTitle ) {
                        props.addCol(builtins[b].title);
                    } else if( c == colType ) {
                        props.addCol(builtins[b].type);
                    } else if( c == colBrief || c == colDesc || c == colSummary || c == colParent ) {
                        props.addCol((const char*) 0);
                    } else if( c == colMulti ) {
                        props.addCol(builtins[b].isMulti);
                    } else if( c == colVirtual || c == colHidden || c == colReadonly ) {
                        props.addCol((idx) 1);
                    } else {
                        tmp_overlapping.printf(0, "%s", props.val(0, c));
                        props.addCol(tmp_overlapping);
                    }

                }
            }
            idx r = 0;
            for(r = 0; r < props.rows; ++r) {
                const char * prop_name = props.val(r, colName);
                idx* u = prop_name_row_id.set(prop_name);
                if( u ) {
                    if( *u ) {
                        if( *u >= props.rows ||
                            strcasecmp(props.val(r, colType), props.val(*u, colType)) != 0 ||
                            strcasecmp(props.val(r, colParent), props.val(*u, colParent)) != 0 ||
                            props.boolval(r, colMulti) != props.boolval(*u, colMulti) ) {
                            *u = (*u * 100000) + r;
                            continue;
                        }
                    }
                    *u = *u ? *u : r;
                }
                const char * v = props.val(r, colBrief);
                if( v && v[0] ) {
                    brief.printf("%s", prop_name);
                    brief.add0();
                }
                if( props.uval(r, colSummary) ) {
                    summary.printf("%s", prop_name);
                    summary.add0();
                }
                v = props.val(r, colParent);
                if( v && v[0] ) {
                    ensureFieldDef(v, true, false);
                }
                ensureFieldDef(prop_name, true, false)->init(r);
            }
            idx ifd_array_row_start = field_defs.dim();
            for(idx ifd = 0; ifd < ifd_array_row_start; ifd++) {
                TFieldDef * pf = *(field_defs.ptr(ifd));
                if( pf->type() == eUsrObjProp_array ) {
                    ensureFieldDef(pf->name(), true, true)->init(ifd);
                }
            }
            for(idx ifd = 0; ifd < field_defs.dim(); ifd++) {
                TFieldDef * pf = *(field_defs.ptr(ifd));
                if( !pf->sanityCheck() ) {
#ifdef _DEBUG_OBJ_FIELD_TYPE
                    sStr buf;
                    pf->printDump(buf);
::fprintf(stderr, "%s:%u %s", __FILE__, __LINE__, buf.ptr());
#endif
                    broken_field_defs = true;
                    break;
                }
                if( !pf->parent() && !pf->wasFlattened() ) {
                    pf->flatten();
                }
#ifdef _DEBUG_OBJ_FIELD_TYPE
                sStr buf;
                pf->printDump(buf);
                fprintf(stderr, "%s", buf.ptr());
#endif
            }
            inherited = r == props.rows;
        }
        key = chain.get(&type_index, sizeof(type_index));
        if( key ) {
            *key = 0;
        }
        break;
    }
    return inherited;
}

static void loadType(sSql& sql, bool prefetch, sStr * names00, sStr * parents00, const sHiveId & qid)
{
    const sHiveId * id = 0;
    sDic<idx> loaded;
    if( qid ) {
        TTypeDef * def = TTypeDef::find(qid);
        if( !def ) {
            id = &qid;
        }
    }
    sStr lst, more;
    do {
        if( !id ) {
            if( names00 ) {
                names00->add0(2);
                lst.cut(0);
                for(const char * p = names00->ptr(); p; p = sString::next00(p)) {
                    if( p[0] && !TTypeDef::find(p) ) {
                        lst.printf(",%s", p);
                    }
                }
            } else if( parents00 ) {
                parents00->add0(2);
                lst.cut(0);
                for(const char * p = parents00->ptr(); p; p = sString::next00(p)) {
                    if( p[0] && !TTypeDef::find(p) ) {
                        lst.printf(",%s", p);
                    }
                }
            }
        }
        more.cut(0);
        if( lst || id || prefetch ) {
            const bool use_type_domain_id = sString::parseBool(getenv("TYPE_DOMAIN_ID"));
            std::auto_ptr<sSql::sqlProc> p(sql.Proc(use_type_domain_id ? "sp_type_get_v4" : "sp_type_get_v3" ));
            if( use_type_domain_id ) {
                p->Add(id ? id->domainId() : 0);
            }
            p->Add(id ? id->objId() : 0).Add((names00 && lst) ? lst.ptr(1) : ((char *)0)).Add((parents00 && lst) ? lst.ptr(1) : ((char *)0)).Add(prefetch);
            prefetch = false;
            id = 0;
            if( p->resultOpen() && sql.resultNext() ) {
                static const idx colDom = sql.resultColId("domainID");
                static const idx colId = sql.resultColId("type_id");
                static const idx colName = sql.resultColId("name");
                static const idx colParent = sql.resultColId("parent");
                static const idx colAbstract = sql.resultColId("is_virtual_fg");
                static const idx colTitle = sql.resultColId("title");
                static const idx colDescription = sql.resultColId("description");
                while( sql.resultNextRow() ) {
                    idx typeIndex = sNotIdx;
                    TTypeDef * t = s_types.setString(TTypeDef::tolower(sql.resultValue(colName)), 0, &typeIndex);
                    if( !t ) {
                        return;
                    }
                    t->broken_field_defs = true;
                    sStr par00("%s", sql.resultValue(colName));
                    par00.add0();
                    sString::searchAndReplaceSymbols(&par00, sql.resultValue(colParent), 0, ",", 0, 0, true, true, true, true);
                    par00.add0(2);
                    for(const char * p = par00; p; p = sString::next00(p) ) {
                        if( p[0] ) {
                            idx * pidx = t->parents.setString(TTypeDef::tolower(p));
                            if( !pidx ) {
                                return;
                            }
                            TTypeDef * def = TTypeDef::find(p);
                            if( def ) {
                                *pidx = def->type_index;
                            } else {
                                *pidx = sNotIdx;
                                more.printf("%s", p);
                                more.add0(1);
                            }
                        }
                    }
                    const idx type_id = sql.resultUValue(colId);
                    idx * tid = loaded.set(&type_id, sizeof(type_id));
                    if( !tid ) {
                        return;
                    }
                    *tid = typeIndex;
                    t->type_index = typeIndex;
                    if( use_type_domain_id ) {
                        t->id.setDomainId(sql.resultUValue(colDom));
                    } else {
                        static const udx systype_domain_id = sHiveId::encodeDomainId("type");
                        t->id.setDomainId(systype_domain_id);
                    }
                    t->id.setObjId(type_id);
                    idx * revIdx = s_type_ids.set(&t->id, sizeof(t->id));
                    if( !revIdx ) {
                        return;
                    }
                    *revIdx = typeIndex;
                    t->isAbstract = sql.resultUValue(colAbstract) != 0;
                    t->name.mex()->flags |= sMex::fExactSize;
                    t->name.cutAddString(0, sql.resultValue(colName));
                    t->title.mex()->flags |= sMex::fExactSize;
                    t->title.cutAddString(0, sql.resultValue(colTitle));
                    t->description.mex()->flags |= sMex::fExactSize;
                    t->description.cutAddString(0, sql.resultValue(colDescription));
                    if( t->name.ptr() ) {
                        t->isUser = !strcasecmp(t->name.ptr(), "base_user_type");
                        t->isSystem = !strcasecmp(t->name.ptr(), "base_system_type");
                    }
                    sVariant::internString(t->name);
                }
                while( sql.resultNext() ) {
                    static const idx id_col = sql.resultColId("type_id");
                    if( !s_props_template.colName(0) ) {
                        for(idx c = 0, k = 0; c < sql.resultColDim(); ++c) {
                            if( c != id_col ) {
                                const char * cnm = sql.resultColName(c);
                                s_props_template.setColId(k++, cnm);
                                s_props_template.addCol("");
                                sVariant::internString(cnm);
                            }
                        }
                    }
                    static const idx name_col = sql.resultColId("name");
                    static const idx prop_type_col = sql.resultColId("type");
                    static const idx default_val_col = sql.resultColId("default_value");
                    static const idx constraint_col = sql.resultColId("constraint");
                    static const idx constraint_data_col = sql.resultColId("constraint_data");
                    TTypeDef * t = 0;
                    while( sql.resultNextRow() ) {
                        const udx type_id = sql.resultUValue(id_col);
                        if( !t || (t->id.objId() != type_id) ) {
                            idx * typeIndex = loaded.get(&type_id, sizeof(type_id));
                            if( !typeIndex) {
                                return;
                            }
                            t = TTypeDef::find(*typeIndex);
                            if( !t ) {
                                return;
                            }
                        }
                        const char * constraint = sql.resultValue(constraint_col);
                        const char * constraint_data = sql.resultValue(constraint_data_col);
                        t->props.addRow();
                        for(idx c = 0; c < sql.resultColDim(); ++c) {
                            const char * val = sql.resultValue(c);
                            if( c == id_col ) {
                                continue;
                            } else if( c == name_col || c == default_val_col ) {
                                sVariant::internString(val);
                            } else if( c == prop_type_col ) {
                                sVariant::internString(val);
                                if( sIs("type2", val) && constraint_data && constraint_data[0] && !TTypeDef::find(constraint_data) ) {
                                    more.printf("%s", constraint_data);
                                    more.add0(1);
                                }
                            }
                            t->props.addCol(val);
                        }
                        if( (sIs(constraint, "choice") || sIs(constraint, "choice+")) && constraint_data ) {
                            sStr buf("%s", constraint_data);
                            idx bufpos = 0;
                            while(true) {
                                char * desc = strstr(buf.ptr(bufpos), "///");
                                char * next = strchr(buf.ptr(bufpos), '|');
                                if( next ) {
                                    *next = 0;
                                }
                                if( desc && (!next || desc < next) ) {
                                    *desc = 0;
                                }
                                sVariant::internString(buf.ptr(bufpos));
                                if( next ) {
                                    bufpos = next + 1 - buf.ptr();
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            names00 = &more;
            parents00 = 0;
        }
    } while(more);
    for(idx i = 0; i < loaded.dim(); ++i) {
        TTypeDef * t = TTypeDef::find(*(loaded.ptr(i)));
        for(idx c = 0; t && c < s_props_template.cols; ++c) {
            t->props.setColId(c, s_props_template.colName(c));
        }
        for(idx j = 0; t && j < t->parents.dim(); ++j) {
            idx * pidx = t->parents.ptr(j);
            if( *pidx == sNotIdx ) {
                TTypeDef * def = TTypeDef::find((const char*)t->parents.id(j));
                if( !def ) {
                    return;
                }
                *pidx = def->type_index;
            }
        }
        t->broken_field_defs = false;
    }
    for(idx i = 0; i < loaded.dim(); ++i) {
        TTypeDef * t = TTypeDef::find(*(loaded.ptr(i)));
        if( !t || !t->inherit() ) {
            return;
        }
    }
}

static
void loadAction(const sUsr& usr, idx typeIndex)
{
    if( s_all_actions.dim() == 0 ) {
        usr.objs2("action", s_all_actions);
    }
    TTypeDef * def = TTypeDef::find(typeIndex);

    if( def && def->acts.dim() == 0 ) {
        for(idx t = 0; t < def->parents.dim(); ++t) {
            for(sUsrObjRes::IdIter it = s_all_actions.first(); s_all_actions.has(it); s_all_actions.next(it)) {
                sUsrObjRes::TObjProp * props = s_all_actions.get(it);
                const sUsrObjRes::TPropTbl * tbl = s_all_actions.get(*props, "type_name");
                const char * tpnm = s_all_actions.getValue(tbl);
                const idx plen = sLen(tpnm);
                const bool isVirtual = plen && (tpnm[plen - 1] == '+');
                if( strncasecmp(tpnm, (const char *) def->parents.id(t), isVirtual ? plen - 1 : plen) == 0 && (t == 0 || isVirtual)) {
                    const sUsrObjRes::TPropTbl * tbl1 = s_all_actions.get(*props, "name");
                    const char * actName = s_all_actions.getValue(tbl1);
                    if( actName && actName[0] ) {
                        if( !def->acts.get(actName) ) {
                            sVarSet* na = def->acts.set(actName);
                            if( na ) {
                                sUsrObjRes::IdIter it_l = s_all_actions.first();
                                for(idx l = 0; l < props->dim(); ++l) {
                                    const char * pnm = (const char*) props->id(l);
                                    const sUsrObjRes::TPropTbl * tbl1 = s_all_actions.get(*props, pnm);
                                    while( tbl1 ) {
                                        na->addRow().addCol(s_all_actions.id(it_l)->objId()).addCol(pnm).addCol(s_all_actions.getPath(tbl1)).addCol(s_all_actions.getValue(tbl1));
                                        tbl1 = s_all_actions.getNext(tbl1);
                                    }
                                    s_all_actions.next(it_l);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static sUsrObj* loadView(const sUsr& usr, idx typeIndex, const char* view_name)
{
    TTypeDef* def = TTypeDef::find(typeIndex);

    std::auto_ptr<sUsrObj>* avw = view_name ? def->views.get(view_name) : 0;
    if( !avw && def->views.dim() == 0 ) {
        sUsrObjRes vec;
        usr.objs2("view", vec);
        def = TTypeDef::find(typeIndex);
        for(sUsrObjRes::IdIter it = vec.first(); vec.has(it); vec.next(it)) {
            sUsrObj* vw = new sUsrObj(usr, *(vec.id(it)));
            if( vw && vw->Id() ) {
                std::auto_ptr<sUsrObj>* nvw = def->views.set(view_name);
                if( nvw ) {
                    nvw->reset(vw);
                    if( view_name ) {
                        const char* nm = vw->propGet("name");
                        if( nm && strcmp(nm, view_name) == 0 ) {
                            avw = nvw;
                        }
                    }
                }
            } else {
                delete vw;
            }
        }
    }
    return avw ? avw->get() : 0;
}

class CViewField
{
    public:
        CViewField(const char* name, const char* dflt, bool readonly, real order, const char* parent)
            : name("%s", name), dflt("%s", dflt), readonly(readonly), order(order), parent("%s", parent ? parent : "")
        {
        }
        sStr name;
        sStr dflt;
        bool readonly;
        real order;
        sStr parent;
};

typedef sDic<std::auto_ptr<CViewField> > CViewFieldList;

static idx field_sorter(void * param, void * arr, idx i1, idx oper, idx i2)
{
    CViewFieldList* vs = (CViewFieldList*) arr;
    CViewField* f1 = vs->ptr(i1)->get(), *f2 = vs->ptr(i2)->get();

    real res = strcasecmp(f2->parent.ptr(), f1->parent.ptr());
    if( res == 0 ) {
        res = f1->order - f2->order;
    }
    if( oper == sSort::eSort_EQ && res == 0 ) {
        return true;
    } else if( oper == sSort::eSort_GT && res > 0 ) {
        return true;
    } else if( oper == sSort::eSort_GE && res >= 0 ) {
        return true;
    } else if( oper == sSort::eSort_LT && res < 0 ) {
        return true;
    } else if( oper == sSort::eSort_LE && res <= 0 ) {
        return true;
    }
    return false;
}

const char* sUsrObjType::name() const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        if( def ) {
            return def->name.ptr();
        }
    }
    return 0;
}

const char* sUsrObjType::Title() const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->title.ptr() : 0;
    }
    return 0;
}

const sHiveId & sUsrObjType::id() const
{
    static sHiveId dummy;
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->id : dummy;
    }
    return dummy;
}

idx sUsrObjType::init(const sUsr & usr, const char * name, const sHiveId & id) const
{
    static bool prefetch = true;
    if( prefetch ) {
        loadType(usr.db(), prefetch, 0, 0, sHiveId());
        prefetch = false;
    }
    if( m_def == sNotIdx ) {
        sUsrObjType* x = const_cast<sUsrObjType*>(this);
        sStr nm("%s", name ? name : "");
        loadType(usr.db(), false, (name && name[0]) ? &nm : 0, 0, id);
        if( name ) {
            TTypeDef * def = TTypeDef::find(name);
            if( def ) {
                x->m_def = def->type_index;
            }
        } else if( id ) {
            TTypeDef * def = TTypeDef::find(id);
            if( def ) {
                x->m_def = def->type_index;
            }
        }
    }
    return m_def;
}

bool sUsrObjType::isOk() const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->id.operator bool() : false;
    }
    return false;
}

bool sUsrObjType::isAbstract() const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->isAbstract : false;
    }
    return false;
}

bool sUsrObjType::isUser() const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->isUser : false;
    }
    return false;
}

bool sUsrObjType::isSystem() const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->isSystem : false;
    }
    return false;
}

udx sUsrObjType::props(const sUsr & usr, sVarSet& list, const char* view_name, const char* filter00) const
{
    if( m_def != sNotIdx ) {
        const idx start_col = 0;
        TTypeDef * def = TTypeDef::find(m_def);
        sStr pFltr;
        for(const char * f = filter00; f; f = sString::next00(f)) {
            if( strcasecmp(f, "_brief") == 0 ) {
                if( def->brief ) {
                    pFltr.add(def->brief.ptr(), def->brief.length());
                }
            } else if( strcasecmp(f, "_summary") == 0 ) {
                if( def->summary ) {
                    pFltr.add(def->summary.ptr(), def->summary.length());
                }
            } else if( strcasecmp(f, "_all") == 0 ) {
                pFltr.cut(0);
                break;
            } else {
                pFltr.printf("%s", f);
                pFltr.add0();
            }
        }
        if( pFltr ) {
            pFltr.add0(2);
        }

        if( list.rows == 0 ) {
            list.setColId(0, "type_id");
            for(idx i = start_col; i < def->props.cols; ++i) {
                list.setColId(i - start_col + 1, def->props.colName(i));
            }
        }
        if( view_name ) {
            sVec<sUsrObj*> views;
            if( view_name[0] ) {
                *(views.ptrx(0)) = loadView(usr, m_def, view_name);
            } else {
                for(idx i = 0; i < def->views.dim(); ++i) {
                    sUsrObj* vw = def->views[i].get();
                    if( vw->propGetBool("is_default") ) {
                        *(views.ptrx(views.dim())) = vw;
                    }
                }
            }
            CViewFieldList vf;
            vf.mex()->flags |= sMex::fSetZero;
            for(idx k = 0; k < views.dim(); ++k) {
                sVarSet ff[4];
                views[k]->propGet("field_name", ff[0]);
                views[k]->propGet("field_readonly", ff[2]);
                if( k == 0 ) {
                    views[k]->propGet("field_default", ff[1]);
                    views[k]->propGet("field_order", ff[3]);
                }
                for(idx i = 0; i < ff[0].rows; ++i) {
                    const char* nm = ff[0].val(i, 0);
                    if( !pFltr ) {
                        if( nm[0] == '_') {
                            continue;
                        }
                    } else if( sString::compareChoice(nm, pFltr, 0, true, 1, true) == -1 ) {
                        continue;
                    }
                    std::auto_ptr<CViewField>* avf = vf.set(nm);
                    if( avf ) {
                        const char* p = ff[2].val(i, 0);
                        bool ro = p && p[0];
                        if( avf->get() ) {
                            avf->get()->readonly &= ro;
                        } else {
                            avf->reset(new CViewField(nm, ff[1].val(i, 0), ro, ff[3].uval(i, 0), 0));
                        }
                    }
                }
            }
            idx* ind = new idx[vf.dim()];
            sSort::sortCallback<CViewFieldList>(field_sorter, 0, vf.dim(), &vf, ind);
            static const idx col_ro = def->props.colId("is_readonly_fg");
            static const idx col_dflt = def->props.colId("default");
            for(idx i = 0; i < vf.dim(); ++i) {
                list.addRow();
                CViewField* f = vf.ptr(ind[i])->get();
                idx * prop_row = def->prop_name_row_id.get(f->name.ptr());
                if( prop_row ) {
                    for(idx c = start_col; c < def->props.cols; ++c) {
                        list.addCol(def->name);
                        if( c == col_ro ) {
                            list.addCol(f->readonly ? "1" : "0");
                        } else if( c == col_dflt ) {
                            list.addCol(f->dflt.ptr());
                        } else {
                            list.addCol(def->props.val(*prop_row, c));
                        }
                    }
                }
            }
            delete[] ind;
        } else {
            CViewFieldList vf;
            vf.mex()->flags |= sMex::fSetZero;

            static const idx col_nm = def->props.colId("name");
            static const idx col_ord = def->props.colId("order");
            static const idx col_prnt = def->props.colId("parent");
            for(idx r = 0; r < def->props.rows; ++r) {
                const char* nm = def->props.val(r, col_nm);
                if( !pFltr ) {
                    if( nm[0] == '_') {
                        continue;
                    }
                } else if( sString::compareChoice(nm, pFltr, 0, true, 1, true) == -1 ) {
                    continue;
                }
                std::auto_ptr<CViewField>* avf = vf.set(nm);
                if( avf ) {
                    real order = def->props.rval(r, col_ord);
                    order = order ? order : REAL_MAX;
                    avf->reset(new CViewField(nm, _, true, order, def->props.val(r, col_prnt)));
                }
            }
            idx* ind = new idx[vf.dim()];
            sSort::sortCallback<CViewFieldList>(field_sorter, 0, vf.dim(), &vf, ind);
            for(idx i = 0; i < vf.dim(); ++i) {
                CViewField* f = vf.ptr(ind[i])->get();
                idx * r = def->prop_name_row_id.get(f->name);
                if( r ) {
                    list.addRow().addCol(def->name);
                    for(idx c = start_col; c < def->props.cols; ++c) {
                        list.addCol(def->props.val(*r, c));
                    }
                }
            }
            delete[] ind;
        }
    }
    return list.rows;
}

struct TypeTypeField
{
    idx icol;
    idx name_offset;
};

const sUsrObjPropsTree* sUsrObjType::propsTree(const sUsr & usr, const char* view_name, const char * filter00) const
{
    const sUsrObjPropsTree * tree = 0;
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        if( def ) {
            bool want_default = (view_name == 0 && filter00 == 0);
            if( !def->props_tree || !def->props_tree_is_default || !want_default) {
                def->props_tree_table.empty();
                sVarSet props_table;
                props(usr, props_table, view_name, filter00);
                def->props_tree_is_default = want_default;
                sStr id_str, path_str, name_buf;
                def->id.print(id_str);
                sVec<TypeTypeField> type_type_fields;

                sUsrObjType type_type(usr, "type");
                for(idx ic=0; ic<props_table.cols; ic++) {
                    idx name_offset = name_buf.length();
                    name_buf.addString("field_");
                    name_buf.addString(props_table.colName(ic));
                    name_buf.add0();
                    if( type_type.prop(name_buf.ptr(name_offset)) ) {
                        TypeTypeField * t = type_type_fields.add(1);
                        t->icol = ic;
                        t->name_offset = name_offset;
                    }
                }

                def->props_tree_table.addRow().addCol(id_str.ptr()).addCol("name").addCol(type_type.prop("name")->orderString()).addCol(def->name.ptr());
                def->props_tree_table.addRow().addCol(id_str.ptr()).addCol("title").addCol(type_type.prop("title")->orderString()).addCol(def->title.ptr());
                def->props_tree_table.addRow().addCol(id_str.ptr()).addCol("description").addCol(type_type.prop("description")->orderString()).addCol(def->description.ptr());
                for(idx ip=1; ip<def->parents.dim(); ip++) {
                    idx * piparent_def = def->parents.ptr(ip);
                    if( piparent_def ) {
                        TTypeDef * parent_def = TTypeDef::find(*piparent_def);
                        if( parent_def ) {
                            path_str.printf(0, "%s.%" DEC, type_type.prop("parent")->orderString(), ip);
                            def->props_tree_table.addRow().addCol(id_str.ptr()).addCol("parent").addCol(path_str.ptr()).addCol(parent_def->id.print());
                        }
                    }
                }
                for(idx ir=0; ir<props_table.rows; ir++) {
                    for(idx it=0; it<type_type_fields.dim(); it++) {
                        path_str.printf(0, "%s.%" DEC, type_type.prop("fields")->orderString(), ir);
                        def->props_tree_table.addRow().addCol(id_str.ptr()).addCol(name_buf.ptr(type_type_fields[it].name_offset)).addCol(path_str.ptr()).addCol(props_table.val(ir, type_type_fields[it].icol));
                    }
                }

                if( !def->props_tree ) {
                    def->props_tree = new sUsrObjPropsTree(usr, "type");
                }
                if( def->props_tree ) {
                    def->props_tree->useTable(def->props_tree_table);
                }
            }
            if( def->props_tree && def->props_tree->initialized() ) {
                tree = def->props_tree;
            }
        }
    }
    return tree;
}


inline static void addStringOrNull(sJSONPrinter & printer, const char * val)
{
    if( val && *val ) {
        printer.addValue(val);
    } else {
        printer.addNull();
    }
}

static void fieldJSON(const sUsrObjTypeField* fld, sJSONPrinter & printer, sVariant & tmp_val)
{
    const char * name = fld->name();
    if( !name || !*name || name[0] == '_' ) {
        return;
    }
    printer.addKey(name);
    printer.startObject();
    printer.addKey("type");
    printer.addValue(fld->typeName());
    printer.addKey("default_value");
    tmp_val.setNull();
    sUsrTypeField::parseValue(tmp_val, (sUsrTypeField::EType)fld->type(), fld->defaultValue());
    printer.addValue(tmp_val);
    printer.addKey("title");
    addStringOrNull(printer, fld->title());
    printer.addKey("description");
    addStringOrNull(printer, fld->description());
    printer.addKey("order");
    const char * order_string = fld->orderString();
    if( order_string && *order_string ) {
        printer.addValue(fld->order(), fld->orderString());
    } else {
        printer.addValue(0);
    }
    printer.addKey("is_key");
    printer.addValue(fld->isKey());
    printer.addKey("is_readonly");
    printer.addValue(fld->isReadonly());
    printer.addKey("is_writeonce");
    printer.addValue(fld->isWriteonce());
    printer.addKey("is_submitonce");
    printer.addValue(fld->isSubmitonce());
    printer.addKey("is_autofill");
    printer.addValue(fld->isAutofill());
    printer.addKey("is_multi");
    printer.addValue(fld->isMulti());
    printer.addKey("is_hidden");
    printer.addValue(fld->isHidden());
    printer.addKey("is_summary");
    printer.addValue(fld->isSummary());
    printer.addKey("is_virtual");
    printer.addValue(fld->isVirtual());
    printer.addKey("is_batch");
    printer.addValue(fld->isBatch());
    printer.addKey("brief");
    addStringOrNull(printer, fld->brief());
    printer.addKey("constraint");
    addStringOrNull(printer, fld->constraint());
    printer.addKey("constraint_data");
    addStringOrNull(printer, fld->constraintData());
    printer.addKey("constraint_description");
    addStringOrNull(printer, fld->constraintDescription());
    printer.addKey("link_url");
    addStringOrNull(printer, fld->linkUrl());
    sVec<const sUsrObjTypeField*> children;
    fld->children(children);
    idx ifirst_child = 0;
    if( children.dim() && children[0]->isArrayRow() ) {
        ifirst_child = 1;
        children[0]->children(children);
    }
    if( children.dim() > ifirst_child ) {
        printer.addKey("_children");
        printer.startObject();
        for(idx i = ifirst_child; i < children.dim(); i++) {
            fieldJSON(children[i], printer, tmp_val);
        }
        printer.endObject();
    }
    printer.endObject();
}

bool sUsrObjType::propsJSON(const sUsr & usr, sJSONPrinter & printer, bool into_object) const
{
    bool ret = false;
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        if( def ) {
            if( !into_object ) {
                printer.startObject();
            }
            printer.addKey("_id");
            printer.addValue(def->id);
            printer.addKey("name");
            printer.addValue(def->name.ptr());
            printer.addKey("title");
            printer.addValue(def->title.ptr());
            printer.addKey("description");
            printer.addValue(def->description.ptr());
            if( def->isUser ) {
                printer.addKey("is_user");
                printer.addValue(true);
            }
            if( def->isSystem ) {
                printer.addKey("is_system");
                printer.addValue(true);
            }
            printer.addKey("_attributes");
            printer.startObject();
            sVariant tmp_val;
            for(idx i=0; i<def->field_defs.dim(); i++) {
                const sUsrObjTypeField* fld = *def->field_defs.ptr(i);
                if( !fld->parent() ) {
                    fieldJSON(fld, printer, tmp_val);
                }
            }
            printer.endObject();
            if( !into_object ) {
                printer.endObject();
            }
            ret = true;
        }
    }
    return ret;
}

const TActions* sUsrObjType::actions(const sUsr& usr) const
{
    if( m_def != sNotIdx ) {
        loadAction(usr, m_def);
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? &(def->acts) : 0;
    }
    return 0;
}

eUsrObjPropType sUsrObjType::propType(const char* prop_name) const
{
    if( prop_name && *prop_name && m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        TFieldDef * pf = def ? def->getFieldDef(prop_name, true) : 0;
        if( pf ) {
            return pf->type();
        }
    }
    return PROP_TYPE_DEFAULT;
}

const sUsrObjTypeField* sUsrObjType::prop(const char* prop_name) const
{
    if( prop_name && *prop_name && m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        return def ? def->getFieldDef(prop_name, true) : 0;
    }
    return 0;
}

udx sUsrObjType::props(sVec<const sUsrObjTypeField*> &list) const
{
    if( m_def != sNotIdx ) {
        TTypeDef * def = TTypeDef::find(m_def);
        for(idx i = 0; def && i < def->field_defs.dim(); i++) {
            list.vadd(1, *(def->field_defs.ptr(i)));
        }
        return def ? def->field_defs.dim() : 0;
    }
    return 0;
}

struct GetTreeWorker
{
    enum {
        eColTypeId = 0,
        eColParent,
        eColIsVirtualFg,
        eColName,
        eColTitle,
        eColDescription
    };

    sVarSet sql_tbl;
    sVec<idx> roots;
    sDic<idx> names;
    sMex tree_mex;
    sVec< sKnot<idx> > tree;

    GetTreeWorker(sSql & db)
    {
        sStr sql("SELECT type_id, parent, is_virtual_fg, name, title, description FROM UPType");
        db.getTable(sql, &sql_tbl);
    }

    idx addBranch(sVarSet& out_tbl, idx irow, sStr & path)
    {
        path.addString("/");
        path.addString(sql_tbl.val(irow, eColName));

        out_tbl.addRow().printCol("type.%s", sql_tbl.val(irow, eColTypeId)).addCol(sql_tbl.val(irow, eColName)).addCol(path.ptr()).addCol(sql_tbl.val(irow, eColTitle)).addCol(sql_tbl.val(irow, eColDescription)).addCol(sql_tbl.val(irow, eColIsVirtualFg));

        idx path_len = path.length();
        idx ret = 1;
        for( idx i=0; i<tree[irow].out.dim(); i++ ) {
            path.cut0cut(path_len);
            ret += addBranch(out_tbl, tree[irow].out[i], path);
        }
        return ret;
    }

    idx run(sVarSet& out_tbl, const char* root_name)
    {
        if( !sql_tbl.rows ) {
            return 0;
        }

        idx all_roots = false;
        if( !sLen(root_name) ) {
            root_name = "base";
        } else if( strcmp(root_name, "*") == 0 ) {
            all_roots = true;
        }

        tree.resize(sql_tbl.rows);

        for( idx ir=0; ir<sql_tbl.rows; ir++ ) {
            tree[ir].init(&tree_mex);

            const char * name = sql_tbl.val(ir, eColName);
            if( !name || ((name[0] < 'a' || name[0] > 'z') && (name[0] < 'A' || name[0] > 'Z')) ) {
                continue;
            }   
            if( (all_roots && !sLen(sql_tbl.val(ir, eColParent))) || (!all_roots && strcmp(root_name, name) == 0) ) {
                *roots.add(1) = ir;
            }
            *names.setString(name) = ir;
        }

        for( idx ir=0; ir<sql_tbl.rows; ir++ ) {
            const char * name = sql_tbl.val(ir, eColName);
            if( !name || !names.get(name) ) {
                continue;
            }
            const char * parent = sql_tbl.val(ir, eColParent);
            idx * piparent = parent ? names.get(parent) : 0;
            if( piparent ) {
                *tree[ir].in.add(1) = *piparent;
                *tree[*piparent].out.add(1) = ir;
            }
        }

        if( out_tbl.cols < 6 ) {
            idx icol = 0;
            out_tbl.setColId(icol++, "id");
            out_tbl.setColId(icol++, "name");
            out_tbl.setColId(icol++, "path");
            out_tbl.setColId(icol++, "title");
            out_tbl.setColId(icol++, "description");
            out_tbl.setColId(icol++, "is_virtual_fg");
        }

        idx ret = 0;
        sStr path;
        for( idx i=0; i<roots.dim(); i++ ) {
            path.cut0cut();
            ret += addBranch(out_tbl, roots[i], path);
        }
        return ret;
    }
};

idx sUsrObjType::getTree(const sUsr& usr, sVarSet& out_tbl, const char* root_name)
{
    GetTreeWorker w(usr.db());
    return w.run(out_tbl, root_name);
}

void sUsrType::propBulk(const char * filter00, sVarSet& list) const
{
    sStr props;
    for(const char * f = filter00; f; f = sString::next00(f)) {
        props.printf(",%s", f);
    }
    std::auto_ptr<sSql::sqlProc> p(m_usr.db().Proc("sp_type_prop_list"));
    p->Add(m_id.domainId()).Add(m_id.objId()).Add(props ? props.ptr(1) : ((char*)0)).Add(false);
    p->getTable(&list);
}

udx sUsrType::propGet(const char* prop, sVarSet& res, bool sort) const
{
    std::auto_ptr<sSql::sqlProc> p(m_usr.db().Proc("sp_type_prop_list"));
    p->Add(m_id.domainId()).Add(m_id.objId()).Add(prop).Add(true);
    p->getTable(&res);
    return res.rows;
}
