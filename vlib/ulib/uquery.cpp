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
#include <slib/core/perf.hpp>
#include <slib/utils/tbl.hpp>
#include <qlang/parser.hpp>
#include <ulib/ufolder.hpp>
#include <ulib/uquery.hpp>
#include <ulib/ufile.hpp>
#include <ulib/utype2.hpp>
#include <slib/std/file.hpp>

using namespace slib;
using namespace slib::qlang;

static const char *special_props00[2] = { "id" _ "_type" __, "_id" _ "_type" __ };
static const char *default_props00[2] = { "id" __, "_id" __ };
static const char *id_key[2] = { "id", "_id" };

inline static const char * getSpecialProps00(idx flags)
{
    return flags & sUsrContext::fUnderscoreId ? special_props00[1] : special_props00[0];
}

inline static const char * getDefaultProps00(idx flags)
{
    return flags & sUsrContext::fUnderscoreId ? default_props00[1] : default_props00[0];
}

inline static const char * getIdKey(idx flags)
{
    return flags & sUsrContext::fUnderscoreId ? id_key[1] : id_key[0];
}

sUsrContext::sUsrContext()
{
    _usr = 0;
    _flags = 0;
}

void sUsrContext::init(const sUsr &usr, idx flags)
{
    _usr = &usr;
    _flags = flags;
    reset();
    registerDefaultBuiltins();

/*
    // FIXME : is this too much caching?
    sVec<udx> allObjIds;
    _usr->objs("", allObjIds);
    for (idx i=0; i<allObjIds.dim(); i++) {
        _objCache.set(allObjIds.ptr(i), sizeof(udx));
    }
*/
}

sUsrContext::~sUsrContext()
{
    for (idx i=0; i<_objCache.dim(); i++) {
        delete _objCache[i].obj;
        _objCache[i].obj = NULL;
    }
}

void sUsrContext::reset()
{
    Context::reset();
    for (idx i=0; i<_objCache.dim(); i++) {
        _objCache[i].fieldOverride.empty();
    }
}

sUsrContext::sUsrObjWrapper* sUsrContext::declareObjectId(const sHiveId & objId)
{
    if( !_usr ) {
        return NULL;
    }
    sUsrObjWrapper *wrapper = NULL;
    sHiveId canonical_id(objId);
    if (!(wrapper = _objCache.get(&canonical_id, sizeof(canonical_id)))) {
        wrapper = _objCache.set(&canonical_id, sizeof(canonical_id));
        wrapper->allowed = _usr->objGet(objId);
    }
    return wrapper;
}

void sUsrContext::declareObjectId(sVariant &objVal)
{
    sHiveId id;
    objVal.asHiveId(&id);
    declareObjectId(id);
}

eEvalStatus sUsrContext::evalUsrObjWrapper(sUsrObjWrapper **pwrapper, sVariant &objVal)
{
    if( !_usr )
        return EVAL_SYSTEM_ERROR;
    if( !objVal.isHiveId() )
        return EVAL_WANTED_OBJECT;

    sHiveId objId;
    objVal.asHiveId(&objId);
    sUsrObjWrapper *wrapper = declareObjectId(objId);
    assert(wrapper);
    if( !(wrapper->allowed) )
        return EVAL_PERMISSION_ERROR;
    if( !(wrapper->obj) )
        wrapper->obj = _usr->objFactory(objId);
    assert(wrapper->obj);
    *pwrapper = wrapper;
    return EVAL_SUCCESS;
}

bool sUsrContext::evalValidObjectId(sVariant &objVal)
{
    sUsrObjWrapper *wrapper;
    return evalUsrObjWrapper(&wrapper, objVal) == EVAL_SUCCESS;
}

eEvalStatus sUsrContext::evalUsrObj(sUsrObj **pobj, sVariant &objVal)
{
    sUsrObjWrapper *objWrapper = NULL;
    eEvalStatus status;
    if ((status = evalUsrObjWrapper(&objWrapper, objVal)) != EVAL_SUCCESS)
        return status;

    *pobj = objWrapper->obj;
    return EVAL_SUCCESS;
}

eEvalStatus sUsrContext::evalHasProperty(sVariant &outval, sVariant &objVal, const char *name)
{
    sUsrObjWrapper *objWrapper = NULL;
    eEvalStatus status;
    if ((status = evalUsrObjWrapper(&objWrapper, objVal)) != EVAL_SUCCESS)
        return status;

    bool found=false;
    for (const char *spec = getSpecialProps00(_flags); spec && *spec; spec = sString::next00(spec))
        if (!strcmp(name, spec)) {
            found = true;
            break;
        }

    if (!found)
        found = !strcmp(name, "_brief") || objWrapper->fieldOverride.get(name) || objWrapper->fieldBuiltin.get(name) || objWrapper->fieldCache.get(name) || objWrapper->obj->propGet(name);

    outval.setInt(found);
    return EVAL_SUCCESS;
}

void sUsrContext::parseField(sVariant &outval, sUsrObjWrapper *wrapper, const char *name, const sVarSet &propValSet, const sVec<idx> *rows, idx pathCol, idx valueCol)
{
    sUsrTypeField::EType type = sUsrTypeField::eString;
    bool multi = false;
    if (const sUsrTypeField * typeField = getTypeField(wrapper, name)) {
        type = typeField->type();
        multi = typeField->isGlobalMulti();
    } else if (!name || name[0] != '_') {
        // names beginning with '_' - like "_brief" - are treated specially
        outval.setNull();
        return;
    }

    idx nrows = rows ? rows->dim() : propValSet.rows;

    // special case: folder's type_count is dic
    if (dynamic_cast<sUsrFolder*>(wrapper->obj) && !strcmp(name, "type_count")) {
        outval.setDic();
        for (idx i=0; i<nrows; i++) {
            idx row = rows ? (*rows)[i] : i;
            sVariant elt;
            elt.parseInt(propValSet.val(row, valueCol));
            outval.setElt(propValSet.val(row, pathCol), elt);
        }
        return;
    // single value (or a string which needs to be interpreted as a list)
    } else if (nrows == 1 && !multi) {
        idx row = rows ? (*rows)[0] : 0;
        sUsrTypeField::parseValue(outval, type, propValSet.val(row, valueCol), sUsrTypeField::fPasswordHide);
        return;
    // explicit lists
    } else if (nrows >= 1 || multi) {
        outval.setList();
        for (idx i=0; i<nrows; i++) {
            idx row = rows ? (*rows)[i] : i;
            sVariant rowval;
            sUsrTypeField::parseValue(rowval, type, propValSet.val(row, valueCol), sUsrTypeField::fPasswordHide);
            if (rowval.isScalar())
                outval.push(rowval);
            else
                outval.append(rowval);
        }
        return;
    }

//  return EVAL_NO_SUCH_PROP;
    outval.setNull();
}

bool sUsrContext::isUsableField(const sUsrTypeField * fld)
{
    return fld && !fld->isSysInternal();
}

const sUsrTypeField * sUsrContext::getTypeField(sUsrObjWrapper * objWrapper, const char * name)
{
    const sUsrTypeField * typeField = objWrapper->obj->propGetTypeField(name);
    return isUsableField(typeField) ? typeField : 0;
}

bool sUsrContext::isPropObjectValued(sVariant &objVal, const char *name)
{
    sUsrObjWrapper *objWrapper = NULL;
    if (evalUsrObjWrapper(&objWrapper, objVal) != EVAL_SUCCESS)
        return false;

    if (objWrapper->obj->propGetValueType(name) == sUsrTypeField::eObj)
        return true;

    /* If the property was reset at runtime, it might still be object-valued */
    sVariant *overrideVal = objWrapper->fieldOverride.get(name);

    if (!overrideVal)
        overrideVal = objWrapper->fieldBuiltin.get(name);

    if (!overrideVal)
        return false;

    if (overrideVal->isHiveId()) {
        return true;
    } else if (overrideVal->isList()) {
        for (idx i=0; i<overrideVal->dim(); i++) {
            if (overrideVal->getListElt(i)->isHiveId())
                return true;
        }
    } else if (overrideVal->isDic()) {
        sVariant elt;
        for (idx i=0; i<overrideVal->dim(); i++) {
            overrideVal->getDicKeyVal(i, elt);
            if (elt.isHiveId())
                return true;
        }
    }

    return false;
}

struct StructurizerParam
{
    sVariant * outVal;
    bool scalar;

    StructurizerParam(sVariant & outVal_, bool scalar_)
    {
        outVal = &outVal_;
        scalar = scalar_;
        if (!scalar)
            outVal->setList();
    }
};

static sUsrObjPropsNode::FindStatus structurizer(const sUsrObjPropsNode &node, void *param_)
{
    StructurizerParam * param = static_cast<StructurizerParam*>(param_);

    sVariant val;
    const sUsrObjPropsNode * outer_list = 0;
    node.structured(0, val, &outer_list);
    if( param->scalar ) {
        *(param->outVal) = val;
    } else {
        if( outer_list && node.namecmp(outer_list->name()) == 0 ) {
            // val is a list of values of node's field; append them all to outVal
            param->outVal->append(val);
        } else {
            // val is either a single value of node's field, or value(s) of a descendent of node's field; push to outVal
            param->outVal->push(val);
        }
    }

    return param->scalar ? sUsrObjPropsNode::eFindStop : sUsrObjPropsNode::eFindContinue;
}

eEvalStatus sUsrContext::evalGetProperty(sVariant &outval, sVariant &objVal, const char *name)
{
    if (objVal.isDic()) {
        if (!objVal.getElt(name, outval))
            outval.setNull();
        return EVAL_SUCCESS;
    }

    sUsrObjWrapper *objWrapper = NULL;
    eEvalStatus status;
    if ((status = evalUsrObjWrapper(&objWrapper, objVal)) != EVAL_SUCCESS) {
        outval.setNull();
        return EVAL_SUCCESS; /*status*/
    }

    // special values
    if (!strcmp(name, getIdKey(_flags))) {
        sHiveId id;
        objVal.asHiveId(&id);
        outval.setHiveId(id);
        return EVAL_SUCCESS;
    }

    sVariant *cachedVal = objWrapper->fieldOverride.get(name);

    if (!cachedVal)
        cachedVal = objWrapper->fieldBuiltin.get(name);

    if (!cachedVal)
        cachedVal = objWrapper->fieldCache.get(name);

    if (!cachedVal) {
        // _brief: special case, requires sUsrObj::props()
        if (!strcmp(name, "_brief")) {
            cacheObjectFields(objVal, "_brief" __);
            cachedVal = objWrapper->fieldCache.get(name);
            assert(cachedVal);
        } else {
            if (const sUsrTypeField * fld = getTypeField(objWrapper, name)) {
                if (fld->canHaveValue()) {
                    sVarSet propValSet;
                    objWrapper->obj->propGet(name, propValSet);
                    assert (propValSet.cols == 2 || propValSet.rows == 0);
                    parseField(outval, objWrapper, name, propValSet);
                } else {
                    StructurizerParam param(outval, !fld->isGlobalMulti());
                    objWrapper->obj->propsTree()->find(name, structurizer, &param);
                }
            }
        }
    }

    if (cachedVal)
        outval = *cachedVal;

    return EVAL_SUCCESS;
}

eEvalStatus sUsrContext::evalSetProperty(sVariant &objVal, const char *name, sVariant &val)
{
    if (objVal.isDic()) {
        return objVal.setElt(name, val) ? EVAL_SUCCESS : EVAL_OTHER_ERROR;
    }

    sUsrObjWrapper *objWrapper = NULL;
    eEvalStatus status;
    if ((status = evalUsrObjWrapper(&objWrapper, objVal)) != EVAL_SUCCESS)
        return status;

    for (const char *spec = getSpecialProps00(_flags); spec && *spec; spec = sString::next00(spec))
        if (!strcmp(name, spec))
            return EVAL_READ_ONLY_ERROR;

    *(objWrapper->fieldOverride.set(name)) = val;

    return EVAL_SUCCESS;
}

bool sUsrContext::registerBuiltinProperty(sVariant &objVal, const char * name, sVariant & value)
{
    sUsrObjWrapper *objWrapper = NULL;
    if (evalUsrObjWrapper(&objWrapper, objVal) != EVAL_SUCCESS)
        return false;

    *(objWrapper->fieldBuiltin.set(name)) = value;
    return true;
}

bool sUsrContext::registerBuiltinProperties(sVariant &objVal, const sUsrObjPropsTree &tree)
{
    sUsrObjWrapper *objWrapper = NULL;
    if (evalUsrObjWrapper(&objWrapper, objVal) != EVAL_SUCCESS)
        return false;

    sVariant valDic;
    if (tree.allStructured(valDic) <= 0)
        return false;

    for (idx i=0; i<valDic.dim(); i++) {
        sVariant propval;
        const char * name = valDic.getDicKeyVal(i, propval);
        *(objWrapper->fieldBuiltin.set(name)) = propval;
    }
    return true;
}

bool sUsrContext::registerBuiltinValue(const char * name, const sUsrObjPropsTree &tree)
{
    sVariant val;
    if (tree.allStructured(val) < 0)
        return false;

    return registerBuiltinValue(name, val);
}

eEvalStatus sUsrContext::evalProps(sVariant &outval, sVariant &objVal, const char * filter00)
{
    sUsrObjWrapper *objWrapper = NULL;
    eEvalStatus status;
    if ((status = evalUsrObjWrapper(&objWrapper, objVal)) != EVAL_SUCCESS)
        return status;

    outval.setList();

    sDic<bool> propsDic;

    // special properties
    for (const char *spec = getSpecialProps00(_flags); spec && *spec; spec = sString::next00(spec)) {
        outval.push(spec);
        propsDic.set(spec);
    }

    for (const char *f = filter00; f && *f; f = sString::next00(f)) {
        if (!propsDic.get(f)) {
            outval.push(f);
            propsDic.set(f);
        }
    }

    // builtin and cached properties
    for (idx i=0; i<objWrapper->fieldOverride.dim(); i++) {
        const char * name = static_cast<const char *>(objWrapper->fieldOverride.id(i));
        if (!propsDic.get(name)) {
            outval.push(name);
            propsDic.set(name);
        }
    }

    for (idx i=0; i<objWrapper->fieldBuiltin.dim(); i++) {
        const char * name = static_cast<const char *>(objWrapper->fieldBuiltin.id(i));
        if (!propsDic.get(name)) {
            outval.push(name);
            propsDic.set(name);
        }
    }

    // sUsrObj properties
    idx nfields = objWrapper->obj->getType() ? objWrapper->obj->getType()->dimFields(*_usr) : 0;
    for (idx i=0; i<nfields; i++) {
        const sUsrTypeField * field = objWrapper->obj->getType()->getField(*_usr, i);
        assert (field->name());
        if( !isUsableField(field) ) {
            continue;
        }
        if (!propsDic.get(field->name())) {
            propsDic.set(field->name());
            if (!field->canHaveValue())
                continue;
            outval.push(field->name());
        }
    }

    return EVAL_SUCCESS;
}

eEvalStatus sUsrContext::evalProps(sVariant &outval, sVariant &objVal, sVariant *options)
{
    sStr filter00;
    if (options) {
        if (options->isList()) {
            for (idx i=0; i<options->dim(); i++) {
                const char *name = options->getListElt(i)->asString();
                filter00.add(name);
            }
        } else {
            const char *name = options->asString();
            filter00.add(name);
        }
        filter00.add0();
    }

    return evalProps(outval, objVal, options ? filter00.ptr() : NULL);
}

enum EFieldMatcherMethod {
    eFieldEquals,
    eFieldSubstring,
    eFieldRegex
};

bool getFieldMatcherOp(EFieldMatcherMethod * method, const char * s)
{
    if( !s ) {
        return false;
    }
    if( strcmp(s, "equals") == 0 ) {
        *method = eFieldEquals;
        return true;
    }
    if( strcmp(s, "substring") == 0 ) {
        *method = eFieldSubstring;
        return true;
    }
    if( strcmp(s, "regex") == 0 || strcmp(s, "regexp") == 0 ) {
        *method = eFieldRegex;
        return true;
    }
    return false;
}

static bool printMatcherSql(sStr & matcherSql, sVariant & fielddesc, sUsrTypeField::EType fieldtype, sSql & db)
{
    EFieldMatcherMethod method = eFieldEquals;
    sVariant * fieldval = &fielddesc;
    if( fielddesc.isDic() ) {
        if( sVariant * opval = fielddesc.getDicElt("method") ) {
            if( !getFieldMatcherOp(&method, opval->asString()) ) {
                return false;
            }
        }
        fieldval = fielddesc.getDicElt("value");
        if( !fieldval ) {
            return false;
        }
    }
    switch(method) {
    case eFieldSubstring:
        matcherSql.addString("f.value LIKE ");
        db.protectSubstringLike(matcherSql, fieldval->asString());
        break;
    case eFieldRegex:
        matcherSql.addString("f.value REGEXP ");
        db.protectValue(matcherSql, fieldval->asString());
        break;
    case eFieldEquals:
        switch (fieldtype) {
        case sUsrTypeField::eString:
        case sUsrTypeField::eUrl:
        case sUsrTypeField::eText:
        case sUsrTypeField::eFile:
            matcherSql.printf("f.value = ");
            db.protectValue(matcherSql, fieldval->asString());
            break;
        case sUsrTypeField::eInteger:
        case sUsrTypeField::eDate:
        case sUsrTypeField::eTime:
        case sUsrTypeField::eDateTime:
            matcherSql.printf("f.value = %" DEC, fieldval->asInt());
            break;
        case sUsrTypeField::eReal:
            matcherSql.printf("f.value = %g", fieldval->asReal());
            break;
        case sUsrTypeField::eBool:
            if (fieldval->asBool()) {
                matcherSql.addString("f.value <> 0");
            } else {
                matcherSql.addString("f.value = 0");
            }
            break;
        case sUsrTypeField::eObj:
        {
            matcherSql.addString("f.value = ");
            sHiveId tmp_id;
            fieldval->asHiveId(&tmp_id);
            if( tmp_id.domainId() || tmp_id.ionId() ) {
                db.protectValue(matcherSql, tmp_id.print());
            } else {
                tmp_id.print(matcherSql);
            }
            break;
        }
        default:
            // This property cannot have a value for the given type
            matcherSql.addString("FALSE");
            break;
        }
    }
    return true;
}

void sUsrContext::getAllObjsOfType(sVariant &outval, const char *typeName, sVariant *propFilter, sUsrObjRes * res/*=0*/, sVariant * res_props/*=0*/)
{
    if( !_usr ) {
        outval.setNull();
        return;
    }

    if (!typeName || !typeName[0] || strcmp(typeName, "*") == 0)
        typeName = 0;

    sStr propFilterSql;
    if (propFilter && propFilter->isDic() && propFilter->dim()) {
        sVec<const sUsrType2*> relevantTypes;
        sStr matcherSql;
        sUsrType2::find(*_usr, &relevantTypes, typeName);

        for (idx i = 0; i < propFilter->dim(); i++) {
            sVariant fielddesc;
            const char * fieldname = propFilter->getDicKeyVal(i, fielddesc);
            for (idx it=0; it<relevantTypes.dim(); it++) {
                matcherSql.cut0cut();
                if( fielddesc.isList() ) {
                    matcherSql.addString("(");
                    bool have_matchers = false;
                    for(idx im=0; im<fielddesc.dim(); im++) {
                        idx prev_len = matcherSql.length();
                        if( !printMatcherSql(matcherSql, *fielddesc.getListElt(im), relevantTypes[it]->getFieldType(*_usr, fieldname), _usr->db()) ) {
                            // TODO: return error
                            outval.setNull();
                            return;
                        }
                        if( prev_len < matcherSql.length() ) {
                            matcherSql.addString(" OR ");
                            have_matchers = true;
                        }
                    }
                    if( have_matchers ) {
                        // remove last " OR "
                        matcherSql.cut(matcherSql.length() - 4);
                        matcherSql.addString(")");
                    } else {
                        matcherSql.cut0cut();
                    }
                } else {
                    if( !printMatcherSql(matcherSql, fielddesc, relevantTypes[it]->getFieldType(*_usr, fieldname), _usr->db()) ) {
                        // TODO: return error
                        outval.setNull();
                        return;
                    }
                }

                if( matcherSql.length() ) {
                    if (propFilterSql.length())
                        propFilterSql.printf(" OR ");
                    propFilterSql.printf("(f.name = '%s' AND %s)", fieldname, matcherSql.ptr());
                }
            }
        }
    }

    sUsrObjRes local_res;
    if( !res ) {
        res = &local_res;
    }
    sStr prop_name_csv;
    if( res_props && res_props->dim() ) {
        for(idx i = 0; i < res_props->dim(); i++) {
            if( i ) {
                prop_name_csv.addString(",");
            }
            prop_name_csv.addString(res_props->getListElt(i)->asString());
        }
    }
    _usr->objsLowLevel(typeName, 0, propFilterSql, prop_name_csv.length() ? prop_name_csv.ptr() : 0, false, 0, 0, res);
    outval.setList();
    for(sUsrObjRes::IdIter it = res->first(); res->has(it); res->next(it)) {
        const sHiveId * oid = res->id(it);
        sVariant v;
        v.setHiveId(*oid);
        outval.push(v);
        declareObjectId(*oid);
    }
}

void sUsrContext::declareObjlist(sVariant &objlist)
{
    if( !_usr ) {
        return;
    }
    sVec<sHiveId> objIds, objIdsOut;
    for(idx i = 0; i < objlist.dim(); i++) {
        objlist.getListElt(i)->asHiveId(objIds.add(1));
    }
    _usr->objs(objIds.ptr(), objIds.dim(), objIdsOut);
    for(idx i = 0; i < objIds.dim(); i++) {
        declareObjectId(objIds[i]);
    }
}

void sUsrContext::cacheObjectFields(sVariant &objVal, const char *fieldNames00)
{
    sUsrObjWrapper *objWrapper = NULL;
    if (evalUsrObjWrapper(&objWrapper, objVal) != EVAL_SUCCESS)
        return;

    sVarSet propsSet;
    sDic<sVec<idx> > propRows;
    objWrapper->obj->propBulk(propsSet, NULL, fieldNames00);
    assert(propsSet.cols >= 4 || propsSet.rows == 0);

    for (idx i=0; i<propsSet.rows; i++) {
        const char *name = propsSet.val(i, 1);
        assert (name);

        sVec<idx> *rows = propRows.set(name);
        rows->vadd(1, i);
    }

    for (idx i=0; i<propRows.dim(); i++) {
        const char *name = static_cast<const char*>(propRows.id(i));
        assert(name);

        // load missing sUsrObj properties into fieldCache
        sVariant *cachedVal = objWrapper->fieldCache.get(name);
        if (!cachedVal) {
            cachedVal = objWrapper->fieldCache.set(name);
            parseField(*cachedVal, objWrapper, name, propsSet, propRows.get(name), 2, 3);
        }

    }

    // If we missed any properties, make sure to cache this fact
    for (const char * name = fieldNames00; name && *name; name = sString::next00(name)) {
        if (!propRows.get(name) && !objWrapper->fieldCache.get(name))
            objWrapper->fieldCache.set(name)->setNull();
    }
}

void sUsrContext::uncacheObjectFields(sVariant &objVal)
{
    sUsrObjWrapper *objWrapper = NULL;
    if (evalUsrObjWrapper(&objWrapper, objVal) != EVAL_SUCCESS)
        return;

    objWrapper->fieldCache.empty();
}

/* builtin functions */

/*! \page qlang_uquery_builtin_objtype objtype()
Returns the type object of a valid object, or null otherwise.

\code
    (12345 as obj).objtype(); // returns object 12345's type object if the user can see it
    objtype(0); // returns null
\endcode

\see \ref qlang_builtin_isobj, \ref qlang_uquery_builtin_objoftype, \ref qlang_uquery_builtin_validobj */
class sUsrQueryBuiltin_objtype : public BuiltinFunction {
public:
    sUsrQueryBuiltin_objtype() { _name.printf("builtin objtype() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic) {
            if (!checkNArgs(ctx, nargs, 0, 0))
                return false;
        } else {
            if (!checkNArgs(ctx, nargs, 1, 1))
                return false;
            topic = args;
            nargs--;
        }

        sUsrObj *obj = NULL;
        eEvalStatus status;
        if ((status = static_cast<sUsrContext&>(ctx).evalUsrObj(&obj, *topic)) != EVAL_SUCCESS) {
            result.setNull();
            return true;
        }

        if( obj->getType() ) {
            result.setHiveId(obj->getType()->id());
        } else {
            result.setNull();
        }
        return true;
    }
};

/*! \page qlang_uquery_builtin_objoftype objoftype()
Returns true if object's type name matches a query (comma-separated list of regexps, optionally followed by '+' for recurse or prefixed by '!' for negate), false otherwise

\code
    objoftype((obj)12345, "svc-align"); // returns true if object 12345's type name contains "svc-align" substring
    (12345 as obj).objoftype("^svc-align$+"); // returns true if object 12345's type is svc-align or descends from it
    (12345 as obj).objoftype("^svc-align-hexagon$+,!^svc-align-hexagon$"); // returns true if object 12345's type descends from svc-align-hexagon but is not svc-align-hexagon itself
\endcode

\see \ref qlang_uquery_builtin_objtype, \ref qlang_builtin_isobj, \ref qlang_uquery_builtin_validobj */
class sUsrQueryBuiltin_objoftype : public BuiltinFunction {
public:
    sUsrQueryBuiltin_objoftype() { _name.printf("builtin objoftype() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic) {
            if (!checkNArgs(ctx, nargs, 1, 1))
                return false;
        } else {
            if (!checkNArgs(ctx, nargs, 2, 2))
                return false;
            topic = args;
            args++;
            nargs--;
        }

        sUsrObj *obj = NULL;
        eEvalStatus status;
        if ((status = static_cast<sUsrContext&>(ctx).evalUsrObj(&obj, *topic)) != EVAL_SUCCESS) {
            result.setInt(0);
            return true;
        }

        result.setInt(obj->isTypeOf(args[0].asString()));
        return true;
    }
};

/*! \page qlang_uquery_builtin_alloftype alloftype()
Retrieve all visible objects matching a type name.

The first argument is a modified regexp for matching type names:
\code
    alloftype("*");       // returns all visible objects of all types
    alloftype("file");    // returns all visible objects whose type name matches
                          // regexp /file/ (so u-file, svc-profiler etc.)
    alloftype("u-file+"); // returns all visible objects whose type name either
                          // matches regexp /u-file/ or is a descendent of one
                          // of such types (so u-file, image, nuc-read etc.)
\endcode

The second, optional, argument is a dictionary of key-value pairs, such
that only objects with at least one field having the given value will be
retrieved:
\code
    alloftype("*", {name: "foo", size: 0}); // returns objects o of any type
                                            // having either o.name == "foo" or
                                            // o.size == 0 (or both)
\endcode

The values can themselves be dictionaries with two elements:

- 'method' - comparison method ('equals', 'substring', 'regex');
- 'value' - actual value to check using the comparison method.

Therefore:
\code
    // returns objects o of any type for which o.bar contains "wombat" substring
    alloftype("*", {bar: { value: "wombat", method: "substring" } });
\endcode

Or the values can be lists, for a disjunction query:

 \code
    // returns objects o of any type for which o.bar equals "wombat" or "wallaby"
    alloftype("*", {bar: ["wombat", "wallaby"] });
\endcode

\see \ref qlang_uquery_builtin_allusedby, \ref qlang_uquery_builtin_allthatuse */

class sUsrQueryBuiltin_alloftype : public BuiltinFunction {
public:
    sUsrQueryBuiltin_alloftype() { _name.printf("builtin alloftype() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicNone(ctx, topic) || !checkNArgs(ctx, nargs, 1, 2))
            return false;

//        PERF_START("sUsrQueryBuiltin_alloftype");

        static_cast<sUsrContext&>(ctx).getAllObjsOfType(result, args[0].asString(), nargs>1 ? args+1 : 0);
//        PERF_END();

        return true;
    }
};

/*! \page qlang_uquery_builtin_allusedby allusedby()
Retrieve all object IDs which are the value of a field of a specified object (or list of objects), possibly recursively.

Fields that are not of type obj are ignored.

If a field's type is an object ID, but the field's value is not a valid
object ID, a dictionary with an \a id field is added to the output list instead.

\code
    allusedby(12345 as obj); // retrieves list of object-type field values of object 12345
    ((obj) 12345).allusedy(); // equivalent to above

    // optional parameter dictionary: whether to recurse (false by default), to what depth (infinite by default),
    // whether to include topic argument in result (false by default)
    allusedby([12345, 12346] as objlist, { recurse: true, depth : 10, with_topic : true });
\endcode
\see \ref qlang_uquery_builtin_allthatuse, \ref qlang_uquery_builtin_alloftype */

class sUsrQueryBuiltin_allusedby : public BuiltinFunction {
public:
    sUsrQueryBuiltin_allusedby() { _name.printf("builtin allusedby() function"); }
    struct Status {
        bool valid;
        idx depth;
        Status()
        {
            valid = false;
            depth = sIdxMax;
        }
        void set(bool v, idx d)
        {
            valid = v;
            depth = sMin<idx>(d, depth);
        }
    };

    void filterAndRecurse(sVariant & obj_val, sUsrContext& ctx, idx depth, const idx max_depth, sDic<Status> & seen) const
    {
        sHiveId id;
        obj_val.asHiveId(&id);
        if (!id) {
            return;
        }

        if (ctx.evalValidObjectId(obj_val)) {
            recurse(obj_val, ctx, depth, max_depth, seen);
            seen.set(&id, sizeof(id))->set(true, depth);
        } else {
            seen.set(&id, sizeof(id))->set(false, depth);
        }
    }

    void recurse(sVariant & obj_val, sUsrContext& ctx, idx depth, const idx max_depth, sDic<Status> & seen) const
    {
        if (depth > max_depth) {
            return;
        }

        sHiveId id;
        obj_val.asHiveId(&id);
        if (!id || seen.get(&id, sizeof(id))) {
            return;
        }

        seen.set(&id, sizeof(id))->set(true, depth);

        // Don't error out on invalid objects by default
        if (ctx.evalValidObjectId(obj_val)) {
            seen.set(&id, sizeof(id))->set(true, depth);
        } else {
            seen.set(&id, sizeof(id))->set(false, depth);
            return;
        }

        sVariant props_val;
        ctx.evalProps(props_val, obj_val);
        sStr props00;
        for (idx i=0; i<props_val.dim(); i++) {
            const char * prop = props_val.getListElt(i)->asString();
            // ignore virtual and builtin propreties
            if( !prop || !prop[0] || prop[0] == '_' ) {
                continue;
            }
            if (!static_cast<sUsrContext&>(ctx).isPropObjectValued(obj_val, prop))
                continue;

            props00.printf("%s", prop);
            props00.add0();
        }
        props00.add0(2);

        // If we didn't find any object-valued properties, exit immediately
        if (!props00[0]) {
            return;
        }

        ctx.cacheObjectFields(obj_val, props00);
        for (const char * prop = props00; prop && *prop; prop = sString::next00(prop)) {
            sVariant val;
            if (ctx.evalGetProperty(val, obj_val, prop) != EVAL_SUCCESS)
                continue;

            if (val.isHiveId()) {
                filterAndRecurse(val, ctx, depth + 1, max_depth, seen);
            } else if (val.isList()) {
                for (idx j=0; j<val.dim(); j++) {
                    sVariant elt;
                    val.getElt(j, elt);
                    filterAndRecurse(elt, ctx, depth + 1, max_depth, seen);
                }
            } else if (val.isDic()) {
                for (idx j=0; j<val.dim(); j++) {
                    sVariant elt;
                    val.getDicKeyVal(j, elt);
                    filterAndRecurse(elt, ctx, depth + 1, max_depth, seen);
                }
            }
        }
    }

    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            if (!checkNArgs(ctx, nargs, 1, 2))
                return false;
            topic = args;
            nargs--;
            args++;
        }

        sVec<sHiveId> ids;
        topic->asHiveIds(ids);

        idx max_depth = 0;
        bool with_topic = false;
        if( nargs && args->isDic() ) {
            if( args->getDicElt("recurse") && args->getDicElt("recurse")->asBool() ) {
                max_depth = args->getDicElt("depth") ? args->getDicElt("depth")->asInt() : sIdxMax;
            }
            with_topic = args->getDicElt("with_topic") && args->getDicElt("with_topic")->asBool();
        }

        sDic<Status> seenObjIds;
        for(idx i=0; i<ids.dim(); i++) {
            sVariant obj_val;
            obj_val.setHiveId(ids[i]);
            recurse(obj_val, static_cast<sUsrContext&>(ctx), 0, max_depth, seenObjIds);
        }

        result.setList();

        for(idx i=0; i<seenObjIds.dim(); i++) {
            sVariant obj_val;
            obj_val.setHiveId(*static_cast<const sHiveId*>(seenObjIds.id(i)));

            if( with_topic || seenObjIds.ptr(i)->depth ) {
                result.push(obj_val);
            }

            static_cast<sUsrContext&>(ctx).uncacheObjectFields(obj_val);
        }

        return true;
    }
};

/*! \page qlang_uquery_builtin_allthatuse allthatuse()
Retrieve all objects having a field whose value is a given object ID.

Fields that are not of type obj are ignored.

\code
    allthatuse(12345 as obj);
    ((obj) 12345).allthatuse(); // equivalent to above
\endcode
\see \ref qlang_uquery_builtin_allusedby, \ref qlang_uquery_builtin_alloftype */

class sUsrQueryBuiltin_allthatuse : public BuiltinFunction {
protected:
    sUsrQueryBuiltin_allusedby usesobjs;

public:
    sUsrQueryBuiltin_allthatuse() { _name.printf("builtin allthatuse() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic && !checkTopicObjectId(ctx, topic))
            return false;
        if (!topic) {
            if (!checkNArgs(ctx, nargs, 1, 1))
                return false;
            topic = args;
        }

        result.setList();
        sVariant obj;
        obj.setHiveId(*topic);
        // Don't error out on invalid objects by default
        if (!ctx.evalValidObjectId(obj))
            return true;

        sVariant allObjs;
        static_cast<sUsrContext&>(ctx).getAllObjsOfType(allObjs, 0, 0);

        for (idx i=0; i<allObjs.dim(); i++) {
            sVariant otherObj;
            allObjs.getElt(i, otherObj);
            sVariant otherObjUses;
            if (usesobjs.call(otherObjUses, ctx, &otherObj, NULL, 0) != EVAL_SUCCESS)
                continue;

            bool found = false;
            for (idx j=0; j<otherObjUses.dim(); j++) {
                sHiveId eltId;
                otherObjUses.getListElt(j)->asHiveId(&eltId);
                if (eltId == *obj.asHiveId()) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            result.push(otherObj);
        }

        return true;
    }
};

/*! \page qlang_uquery_builtin_csv csv()
Print a list of object IDs or dictionaries as a CSV table, with columns being
the union of the lists of the objects' fields or the dictionaries' keys.

Note that an \a id column is always printed (and is always first).

\code
    ("12345,12346" as objlist).csv(); // prints the following:
\endcode
<pre>
id,_type,name,size
12345,foo,bar,99
12346,foo,"hello, world",10
</pre>

An optional first argument specified which columns to print. Adding the
special value "_brief" to this list will print the object's dynamic
brief description. Adding the special value "_summary" will add brief-enabled
fields to the list of enabled columns.

\code
    ("12345,12346" as objlist).csv(["name","missing column"]); // prints the following:
\endcode
<pre>
id,name,missing column
12345,bar,
12346,"hello, world",
</pre>

An optional second argument is a parameter dictionary:
- \a start: at which row to start printing (0 by default)
- \a cnt: how many rows to print (all of them by default)
- \a info: whether to append a special last row with information about the
table's geometry: "info" in the leftmost column, then the \a start parameter,
and then the total number of rows that would have been printed if \a start
and \a cnt parameters were not specified.

\code
    ("12345,12346,12346,12346" as objlist).csv(["_brief", "name", "size"], {start: 1, cnt: 1, info: true}); // prints the following:
\endcode
<pre>
id,_brief,name,size
12346,"<b>Foo Object</b> 'hello, world' of size 10","hello, world",10
info,1,4,
</pre> */

class sUsrQueryBuiltin_csv : public BuiltinFunction {
public:
    sUsrQueryBuiltin_csv() { _name.printf("builtin csv() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicList(ctx, topic) || !checkNArgs(ctx, nargs, 0, 2))
            return false;

        bool haveColList = false;
        if (nargs) {
            if (!args[0].isList() && !args[0].isNull()) {
                ctx.setError(EVAL_WANTED_LIST, "%s expects a list of column names in the first argument", getName());
                return false;
            }
            haveColList = args[0].isList();
        }

        idx start = 0;
        idx cnt = -1;
        bool wantInfo = false;
        if (nargs >= 2) {
            if (!args[1].isDic()) {
                ctx.setError(EVAL_WANTED_DIC, "%s expects a dictionary as the second argument", getName());
                return false;
            }

            sVariant val;
            if (args[1].getElt("start", val))
                start = sMax<idx>(0, val.asInt());
            if (args[1].getElt("cnt", val))
                cnt = sMax<idx>(0, val.asInt());
            if (args[1].getElt("info", val))
                wantInfo = val.asBool();
        }
        idx end = cnt < 0 ? topic->dim() : sMin<idx>(topic->dim(), start+cnt);

        eEvalStatus status;
        sDic<bool> columns;
        bool haveSummary = false;

        // always add id to column list
        for (const char *spec = getDefaultProps00(static_cast<sUsrContext&>(ctx).getFlags()); spec && *spec; spec = sString::next00(spec))
            *(columns.set(spec)) = true;

        // if we are given a column list, append it
        if (haveColList) {
            for (idx j=0; j<args[0].dim(); j++) {
                // "_summary" column is special: we do not print it, but use it as a flag to load other columns
                const char *col = args[0].getListElt(j)->asString();
                if (!strcmp("_summary", col))
                    haveSummary = true;
                else
                    *(columns.set(col)) = true;
            }
        }

        // if we are not given a column list, append any column present in at least one row
        // if we were given a "_summary" flag, append any summary columns present in at least one row
        if (haveSummary || !haveColList) {
            sVariant summaryOpt;
            summaryOpt.setString("_summary");

            for (idx i=start; i<end; i++) {
                sVariant elt, eltkeys;
                topic->getElt(i, elt);

                // do we skip invalid objects?
                if (elt.isHiveId() && !ctx.evalValidObjectId(elt))
                    continue;

                if (elt.isHiveId()) {
                    if ((status = ctx.evalProps(eltkeys, elt, haveSummary ? &summaryOpt : NULL)) != EVAL_SUCCESS) {
                        ctx.setError(status, "%s failed to retrieve keys for row #%" DEC, getName(), i+1);
                        return false;
                    }
                } else {
                    if ((status = ctx.evalKeys(eltkeys, elt)) != EVAL_SUCCESS) {
                        ctx.setError(status, "%s failed to retrieve keys for row #%" DEC, getName(), i+1);
                        return false;
                    }
                }
                for (idx j=0; j<eltkeys.dim(); j++)
                    *(columns.set(eltkeys.getListElt(j)->asString())) = true;
            }
        }

//        PERF_START("sUsrQueryBuiltin_csv");

        // print header
        sStr cacheFieldNames00;
        sVariantTblData * tbld = new sVariantTblData(sMax<idx>(1, columns.dim()), 0);

        for (idx j=0; j<columns.dim(); j++) {
            const char *col = static_cast<const char *>(columns.id(j));
            // j - 1 because _id column should be the left header
            tbld->getTable().setVal(-1, j, col);

            cacheFieldNames00.printf(col);
            cacheFieldNames00.add0();
        }
        if (haveSummary) {
            cacheFieldNames00.printf("_summary");
            cacheFieldNames00.add0();
        }
        cacheFieldNames00.add0(2);

        // print table body
        for (idx i=start; i<end; i++) {
            sVariant elt;
            topic->getElt(i, elt);

            bool valid_obj = false;
            if (elt.isHiveId() && ctx.evalValidObjectId(elt) ) {
                static_cast<sUsrContext&>(ctx).cacheObjectFields(elt, cacheFieldNames00);
                valid_obj = true;
            }

            for (idx j=0; j<columns.dim(); j++) {
                const char *col = static_cast<const char *>(columns.id(j));
                sVariant cell;
                bool gotCell = false;

                if (elt.isHiveId()) {
                    if( valid_obj ) {
                        gotCell = (ctx.evalGetProperty(cell, elt, col) == EVAL_SUCCESS);
                    } else if( strcmp(col, getIdKey(static_cast<sUsrContext&>(ctx).getFlags())) == 0 ) {
                        // for invalid objects, just insert the "id" / "_id" column
                        cell.setHiveId(elt);
                        gotCell = true;
                    }
                } else {
                    gotCell = elt.getElt(col, cell);
                }

                if (gotCell) {
                    tbld->getTable().setVal(i-start, j, cell);
                }
            }

            if (elt.isHiveId())
                static_cast<sUsrContext&>(ctx).uncacheObjectFields(elt);
        }

        // same format as tableView uses
        if (wantInfo) {
            idx ir = tbld->getTable().rows();
            tbld->getTable().setVal(ir, 0, "info");
            tbld->getTable().setVal(ir, 1, start);
            tbld->getTable().setVal(ir, 2, topic->dim());
        }

        result.setData(*tbld);
        // do not delete tbld - that will be handled by result

//        PERF_END();

        return true;
    }
};

/*! \page qlang_uquery_builtin_validobj validobj()
Check whether a value is an object ID corresponding to a valid object visible
to the current user.

\code
    (12345 as obj).validobj(); // can the current user see object 12345?
    (null as obj).validobj(); // returns 0
\endcode

\see \ref qlang_builtin_isobj, \ref qlang_uquery_builtin_objtype, \ref qlang_uquery_builtin_objoftype */

class sUsrQueryBuiltin_validobj : public BuiltinFunction {
public:
    sUsrQueryBuiltin_validobj() { _name.printf("builtin validobj() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic && (!topic->isHiveId() || !ctx.evalValidObjectId(*topic))) {
            result.setInt(0);
            return true;
        }

        for (idx i=0; i<nargs; i++) {
            if (!args[i].isHiveId() || !ctx.evalValidObjectId(args[i])) {
                result.setInt(0);
                return true;
            }
        }

        result.setInt(1);
        return true;
    }
};

#define REGISTER_BUILTIN_FUNC(name) \
static sUsrQueryBuiltin_ ## name builtin_ ## name; \
registerBuiltinFunction(#name, builtin_ ## name)

/*! \page qlang_uquery_builtins Additional sUsrContext builtin functions

These are available in slib::qlang::sUsrContext and its child classes.
They are split out to allow basic query language functionality to be
used without linking to ulib.

- \subpage qlang_uquery_builtin_alloftype
- \subpage qlang_uquery_builtin_allusedby
- \subpage qlang_uquery_builtin_allthatuse
- \subpage qlang_uquery_builtin_csv
- \subpage qlang_uquery_builtin_objtype
- \subpage qlang_uquery_builtin_objoftype
- \subpage qlang_uquery_builtin_validobj

*/

void sUsrContext::registerDefaultBuiltins()
{
    REGISTER_BUILTIN_FUNC(alloftype);
    REGISTER_BUILTIN_FUNC(allusedby);
    REGISTER_BUILTIN_FUNC(allthatuse);
    REGISTER_BUILTIN_FUNC(csv);
    REGISTER_BUILTIN_FUNC(objtype);
    REGISTER_BUILTIN_FUNC(objoftype);
    REGISTER_BUILTIN_FUNC(validobj);
}

/* sUsrInternalContext */

class ArgStringComposer
{
protected:
    sStr _result, _enc;

public:
    ArgStringComposer() {}

    ArgStringComposer& addStr(const char * s)
    {
        if (s)
            _enc.printf("%s", s);

        return *this;
    }

    ArgStringComposer& addVal(sVariant * v)
    {
        if (!v)
            return *this;

        v->print(_enc, sVariant::eUnquoted);
        return *this;
    }

    ArgStringComposer& addLiteral(const char * arg)
    {
        encode();
        if (arg)
            _result.printf("%s", arg);

        return *this;
    }

    ArgStringComposer& encode()
    {
        if (!_enc.length())
            return *this;

        sString::escapeForShell(_result, _enc, _enc.length());

        _enc.cut(0);
        return *this;
    }

    ArgStringComposer& delimit()
    {
        encode();
        if (_result.length())
            _result.printf(" ");
        return *this;
    }

    const char * ptr()
    {
        encode();
        return _result.ptr();
    }
};

static bool isFalseOpt(sVariant * v)
{
    return v->isNull() || (v->isInt() && !v->asBool());
}

static const char * getStringOpt(sVariant * dicVar, const char * name, const char * ifunset, const char * iffalse)
{
    sVariant * v = dicVar ? dicVar->getDicElt(name) : 0;
    if (!v)
        return ifunset;
    else if (isFalseOpt(v))
        return iffalse;
    return v->asString();
}

static void getListOpt(sVariant & out, sVariant * dicVar, const char * name, const char * ifunset)
{
    sVariant * v = dicVar ? dicVar->getDicElt(name) : 0;
    if (!v) {
        out.setList();
        if (ifunset) 
            out.push(ifunset);
    } else if (isFalseOpt(v)) {
        out.setNull();
    } else if (v->isList()) {
        out = *v;
    } else {
        out.setList();
        out.push(*v);
    }
}

class sUsrQueryBuiltin_argstring : public BuiltinFunction {
public:
    sUsrQueryBuiltin_argstring() { _name.printf("builtin argstring() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            if (!checkNArgs(ctx, nargs, 1, sIdxMax))
                return false;
            topic = args;
            args++;
            nargs--;
        }

        ArgStringComposer comp;

        const char * prefix = getStringOpt(nargs?args:0, "prefix", "--", "");
        const char * assign = getStringOpt(nargs?args:0, "assign", "=", " ");
        const char * join = getStringOpt(nargs?args:0, "listjoin", ",", 0);

        sVariant skip;
        getListOpt(skip, nargs?args:0, "skipval", "");

        if (topic->isHiveId()) {
            if (!ctx.evalValidObjectId(*topic))
                return true;

            sVariant props;
            ctx.evalProps(props, *topic);
            for (idx i=0; i<props.dim(); i++) {
                const char * arg = props.getListElt(i)->asString();
                sVariant val;
                ctx.evalGetProperty(val, *topic, arg);
                composeArg(comp, prefix, arg, assign, &val, join, skip);
            }
        } else if (topic->isDic()) {
            for (idx i=0; i<topic->dim(); i++) {
                sVariant val;
                const char * arg = topic->getDicKeyVal(i, val);
                composeArg(comp, prefix, arg, assign, &val, join, skip);
            }
        } else if (topic->isList()) {
            for (idx i=0; i<topic->dim(); i++)
                comp.delimit().addLiteral(prefix).addVal(topic->getListElt(i));
        } else {
            comp.addLiteral(prefix).addVal(topic);
        }

        result.setString(comp.ptr());

        return true;
    }

    bool wantAssign(sVariant * val, sVariant & skip) const
    {
        if (!val || val->isNull() || val->isData())
            return false;

        if (!(val->isScalar() || val->dim()))
            return false;

        if (!skip.isList())
            return true;

        for (idx i=0; i<skip.dim(); i++) {
            if (*val == *(skip.getListElt(i)))
                return false;
        }

        return true;
    }

    void composeArg(ArgStringComposer & comp, const char * prefix, const char * arg, const char * assign, sVariant * val, const char * join, sVariant & skip) const
    {
        if (val && val->isList()) {
            idx eltsprinted = 0;
            for (idx ival=0; ival<val->dim(); ival++) {
                sVariant * elt = val->getListElt(ival);
                if (wantAssign(elt, skip)) {
                    if (join && eltsprinted) {
                        comp.addStr(join).addVal(elt);
                    } else {
                        comp.delimit().addLiteral(prefix).addStr(arg).addLiteral(assign).addVal(elt);
                    }
                    eltsprinted++;
                }
            }
        } else {
            comp.delimit().addLiteral(prefix).addStr(arg);
            if (wantAssign(val, skip))
                comp.addLiteral(assign).addVal(val);
        }
    }
};

/*! \page qlang_uquery_internal_builtin_filepath filepath()
Find storage file path for a file belonging to an object

\code
    (12345 as obj).filepath(); // if 12345 is u-file - path to the main file
    filepath((obj)12346); // equivalent to above
    (12346 as obj).filepath(); // if 12346 is not u-file - returns null
    (12346 as obj).filepath(''); // storage directory
    (12346 as obj).filepath('hello.csv'); // path to hello.csv if object 12346 has it
    (12346 as obj).filepath('bye.txt'); // null if object 12346 doesn't have bye.txt
\endcode */
class sUsrQueryBuiltin_filepath : public BuiltinFunction {
public:
    sUsrQueryBuiltin_filepath() { _name.printf("builtin filepath() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            if (!checkNArgs(ctx, nargs, 1, sIdxMax))
                return false;
            topic = args;
            args++;
            nargs--;
        }

        sUsrObj *obj = NULL;
        eEvalStatus status;
        if ((status = static_cast<sUsrContext&>(ctx).evalUsrObj(&obj, *topic)) != EVAL_SUCCESS) {
            sStr s;
            topic->print(s);
            ctx.setError(status, "%s failed to retrieve object for %s", getName(), s.ptr());
            return false;
        }

        result.setNull();

        sStr pathbuf;
        if( nargs ) {
            sStr key;
            for(idx i = 0; i < nargs; i++) {
                key.printf("%s", args[i].asString());
                key.add0();
            }
            key.add0(2);
            if( obj->getFilePathname00(pathbuf, key.ptr()) ) {
                result.setString(pathbuf.ptr());
            }
        } else if( sUsrFile * ufile = dynamic_cast<sUsrFile*>(obj) ) {
            if( ufile->getFile(pathbuf) ) {
                result.setString(pathbuf.ptr());
            }
        }
        return true;
    }
};

/*! \page qlang_uquery_internal_builtins Additional sUsrInternalContext builtin functions

These are available in slib::qlang::sUsrInternalContext; they should be used only
in query language code that has been safely constructed by a backend, not when
evaluating query text that had been submitted by the user.

- \subpage qlang_uquery_internal_builtin_argstring
- \subpage qlang_uquery_internal_builtin_filepath

*/

void sUsrInternalContext::registerDefaultBuiltins()
{
    REGISTER_BUILTIN_FUNC(argstring);
    REGISTER_BUILTIN_FUNC(filepath);
}

bool sUsrInternalContext::isUsableField(const sUsrTypeField * fld)
{
    return true;
}

/* sUsrEngine */

sUsrEngine::sUsrEngine(): Engine(true)
{
    _usr = 0;
    _ctx = 0;
}

sUsrEngine::sUsrEngine(const sUsr &usr, idx ctx_flags): Engine(true)
{
    _usr = &usr;
    _ctx = new sUsrContext(usr, ctx_flags);
}

void sUsrEngine::init(const sUsr &usr, idx ctx_flags)
{
    delete _ctx;
    _usr = &usr;
    _ctx = new sUsrContext(usr, ctx_flags);
}

bool sUsrEngine::registerBuiltinValuePropertiesTable(const char * name, const char * typeName, sVarSet & propsTable)
{
    if( !_usr ) {
        return false;
    }
    sUsrObjPropsTree tree(*_usr, typeName);
    if (!tree.useTable(propsTable))
        return false;
    return registerBuiltinValue(name, tree);
}

bool sUsrEngine::registerBuiltinValuePropertiesForm(const char * name, const char * typeName, const sVar & form)
{
    if( !_usr ) {
        return false;
    }
    sUsrObjPropsTree tree(*_usr, typeName);
    if (!tree.useForm(form))
        return false;
    return registerBuiltinValue(name, tree);
}

bool sUsrEngine::registerBuiltinPropertiesTable(const char * typeName, const sHiveId & objId, sVarSet & propsTable)
{
    if( !_usr ) {
        return false;
    }
    sUsrObjPropsTree tree(*_usr, typeName);
    if (!tree.useTable(propsTable))
        return false;
    return registerBuiltinProperties(objId, tree);
}

bool sUsrEngine::registerBuiltinPropertiesForm(const char * typeName, const sHiveId & objId, const sVar & form)
{
    if( !_usr ) {
        return false;
    }
    sUsrObjPropsTree tree(*_usr, typeName);
    if (!tree.useForm(form))
        return false;
    return registerBuiltinProperties(objId, tree);
}

/* sUsrInternalEngine */

sUsrInternalEngine::sUsrInternalEngine(sUsr &usr, idx ctx_flags)
{
    _usr = &usr;
    _ctx = new sUsrInternalContext(usr, ctx_flags);
}

void sUsrInternalEngine::init(sUsr &usr, idx ctx_flags)
{
    delete _ctx;
    _usr = &usr;
    _ctx = new sUsrInternalContext(usr, ctx_flags);
}







bool sUsrInternalContext::dispatcher_callback(sVariant &result, const qlang::BuiltinFunction &funcObj, qlang::Context &ctx, sVariant *topic, sVariant *args, idx nargs, void *param)
{
    //sQPrideGenericLauncher * qp = (sQPrideGenericLauncher *) param;
    //const char * workDir()=qp->launcherDir.ptr();
    sUsr * usr = static_cast<sUsrInternalContext&>(ctx).getUsr(); //qp->user;
    const char * workDir = static_cast<sUsrInternalContext&>(ctx).workDir();

    bool ret = true;
    const char * myFuncName = funcObj.getName();
    /*
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    _/
    _/ retrieval of arguments in a batch
    _/
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    */
    if( strcmp(myFuncName, "get_batch_args") == 0 ) {
        if( !topic || (!topic->isDic() && !topic->isHiveId()) ) {
            ctx.setError(qlang::EVAL_SYNTAX_ERROR, "%s: expected dic or object topic", funcObj.getName());
            return false;
        }
        sVariant keys;
        if( ctx.evalKeys(keys, *topic) != qlang::EVAL_SUCCESS )
            return false;
        sStr res;
        for(idx i = 0; i < keys.dim(); i++) {
            if( i )
                res.printf(" ");
            sVariant * key = keys.getListElt(i);
            sVariant val;
            ctx.evalGetSubscript(val, *topic, *key);
            res.printf("-%s ", keys.getListElt(i)->asString());
            if( sLen(val.asString()) )
                val.print(res, sVariant::eCSV);
            else
                res.printf("\"\"");
        }
        result.setString(res);
    }
    /*
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    _/
    _/ retrieval of a session ID of the logged in user
    _/
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    */
    else if( strcmp(myFuncName, "sessionID") == 0 ) {
        sStr res, sid, buf;
        usr->batch("generic-launcher");
        usr->encodeSID(sid, buf);
        res.printf("\"%s\"", buf.ptr());
        result.setString(res);
    }
    /*
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    _/
    _/ retrieval of the working directory
    _/
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    */
    /*
    else if( strcmp(myFuncName, "workDir()") == 0 ) {
        sStr res;
        res.printf("%s%s", workDir(), args[0].asString());
        result.setString(res);
    }
    */
    /*
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    _/
    _/ retrieval of the special object path: symlinking all internal object files
    _/
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    */

    else if( (strcmp(myFuncName, "spObjPath") == 0) ) {

        sHiveId latestId;
        args[0].asHiveId(&latestId);
        const char *nameSpObj = nargs > 1 ? args[1].asString() : 0;
        idx cnt = 0;
        std::auto_ptr<sUsrObj> obj(usr->objFactory(latestId));
        if( obj->Id() ) {
            // make directory
            sStr subDirForObj;
            if( nameSpObj ) {
                subDirForObj.printf("%s%s/", workDir, nameSpObj);
            } else {
                subDirForObj.printf("%s%" DEC "/", workDir, latestId.objId());
            }
            sDir::makeDir(subDirForObj);

            //  sStr subDirForObj("%s%" DEC,workDir.ptr(),objids[i]);
            sStr fileNames;
            obj->propGet00("file", &fileNames);
            for(const char * ptr = fileNames.ptr(0); ptr && *ptr; ptr = sString::next00(ptr)) {
                sStr linkPath;
                linkPath.printf("%s%s", subDirForObj.ptr(), ptr);
                sStr link;
                sStr link1;
                obj->getFilePathname00(link, ptr);
                obj->getFilePathname(link1, ptr);

                sFile::symlink(link.ptr(), linkPath.ptr());
                cnt++;
            }
            result.setString(subDirForObj.ptr());
        }

    }
    /*
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    _/
    _/ retrieval of any other object path: works through symlinking the actual path
    _/
    _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    */
    else if( (strcmp(myFuncName, "anyObjPath") == 0) ) {
        sHiveId latestId;
        args[0].asHiveId(&latestId);
        const char *namefile = nargs >= 2 && !args[2].isNull() ? args[2].asString() : NULL;

        const char *noprint = nargs >= 3 && !args[3].isNull() ? args[3].asString() : NULL;
        const char *validation = nargs >= 4 && !args[4].isNull() ? args[4].asString() : NULL;
        regex_t validation_re;
        if( validation && regcomp(&validation_re, validation, REG_EXTENDED | REG_NOSUB) != 0 ) {
            ctx.setError(qlang::EVAL_SYNTAX_ERROR, "Invalid validation regular expression: '%s'", validation);
            return false;
        }
        bool printResult = true;
        if( noprint && !strcmp(noprint, "noPrint") ) {
            printResult = false;
        }
        std::auto_ptr<sUsrObj> obj(usr->objFactory(latestId));        //find the actual file pointer
        if( !obj.get() ) {
            ctx.setError(qlang::EVAL_TYPE_ERROR, "Invalid object ID '%s'", latestId.print());
            ret = false;
        } else {
            sStr path;
            sStr ext(".");
            sStr actualName;
            obj->propGet("file", &path);
            obj->propGet("ext", &ext);
            obj->propGet("name", &actualName);


            if( !path ) {
                //path.printf("%s",extObj);//extension is the only thing required for creating link
                path.printf("%s", ext.ptr());
            }
            if( obj->Id() ) {
                // make directory
                sStr subDirForObj;
                if( namefile ) {
                    subDirForObj.printf("%s", workDir);
                } else {
                    subDirForObj.printf("%s%" DEC "/", workDir, latestId.objId());
                    sDir::makeDir(subDirForObj);
                }

                const char *linkName;

                sStr linkPath, link;
                linkName = nargs > 1 ? args[0].asString() : "_";
                if( actualName ) {
                    linkPath.printf("%s%s", subDirForObj.ptr(), actualName.ptr());
                } else if( namefile ) {
                    linkPath.printf("%s%s", subDirForObj.ptr(), namefile);
                } else {
                    linkPath.printf("%s%s", subDirForObj.ptr(), linkName);
                }

                if( sUsrFile * fileobj = dynamic_cast<sUsrFile*>(obj.get()) ) {
                    fileobj->getFile(link);
                } else {
                    obj->getFilePathname00(link, ext);
                }

                sFile::symlink(link.ptr(), linkPath.ptr());
                if( validation && regexec(&validation_re, linkPath.ptr(), 0, 0, 0) != 0 ) {
                    ctx.setError(qlang::EVAL_OTHER_ERROR, "Path '%s' failed validation", linkPath.ptr());
                    ret = false;
                } else if( printResult ) {
                    result.setString(linkPath.ptr());
                } else {
                    result.setString("");
                }
            }
        }
        if( validation )
            regfree(&validation_re);

    }

    else if( strncmp(myFuncName, "path", 4) == 0 || strncmp(myFuncName, "dirPath", 4) == 0 ) {
       idx prvLen = 0;
       sStr path;
       const char * separator = nargs >= 3 ? args[2].asString() : " ";

        sStr t1;
        sString::searchAndReplaceStrings(&t1, args[0].asString(), 0, "obj" __, " " __, 0, true);
        sStr t;
        sString::searchAndReplaceSymbols(&t, t1.ptr(), 0, "[]", 0, 0, true, true, true, true);
        sVec<sHiveId> objids;
        sHiveId::parseRangeSet(objids, t.ptr());

        sStr srcdir,subDirForObj,list;

        if(nargs>1)sString::searchAndReplaceSymbols(&list, args[1].asString() , 0, " ,;\n", 0, 0, true, true, true, true);
        else list.add0(2);

        sStr FileListBuf;
        sFilePath link,linkPath;
        for(idx i = 0; i < objids.dim(); ++i) {
            sUsrObj obj(*(usr), objids[i]);

            subDirForObj.printf(0,"%s%s", workDir, objids[i].print()); // ^0xAAAAAAAA
            //obj.getPath(srcDir, obj.Id()) ;


            if( strcmp(myFuncName, "dirPath") == 0 ) {
                sFile::symlink(srcdir.ptr(), subDirForObj.ptr());
                continue;
            }


            sDir::makeDir(subDirForObj);
            sDir FileList00;
            for(const char * p = list.ptr(0); p; p = sString::next00(p)) {

                const char * fileList00 = 0;


                FileListBuf.cut(0);
                if( !*p ) {
                    if( ((sUsrFile *) &obj)->getFile(FileListBuf) ) {
                        FileListBuf.add0(2);
                        fileList00 = FileListBuf.ptr();
                    }
                } else {
                    obj.files(FileList00, sFlag(sDir::bitFiles), p, "");
                    fileList00 = FileList00.ptr();
                }
                if( !fileList00 )
                    continue;

                for(const char * pp = fileList00; pp; pp = sString::next00(pp)) {

                    link.cut(0);
                    linkPath.cut(0);

                    if( path.length() != prvLen )
                        path.printf("%s", separator);

                    linkPath.makeName(pp, "%s/%%flnm", subDirForObj.ptr());
                    link.printf("%s", pp);

                    sFile::symlink(link.ptr(), linkPath.ptr());
                    prvLen = path.length();
                    path.printf("%s", linkPath.ptr());

                }
            }
        }
        result.setSprintf("%s", path.ptr());
    }

    return ret;
}
