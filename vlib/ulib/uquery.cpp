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
#include <qlib/QPrideBase.hpp>
#include <qlib/QPrideProc.hpp>
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
    if( !_usr ) {
        return EVAL_SYSTEM_ERROR;
    }
    if( !objVal.isHiveId() ) {
        return EVAL_WANTED_OBJECT;
    }
    sHiveId objId;
    objVal.asHiveId(&objId);
    sUsrObjWrapper *wrapper = declareObjectId(objId);
    assert(wrapper);
    if( !(wrapper->allowed) ) {
        return EVAL_PERMISSION_ERROR;
    }
    if( !(wrapper->obj) ) {
        wrapper->obj = _usr->objFactory(objId);
    }
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
    if( (status = evalUsrObjWrapper(&objWrapper, objVal)) != EVAL_SUCCESS ) {
        return status;
    }
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
        found = !strcmp(name, "_brief") || objWrapper->fieldOverride.get(name) || objWrapper->fieldBuiltin.get(name) || objWrapper->fieldCache.get(name) || objWrapper->obj->propGet(name, 0, allowSysInternal());

    outval.setBool(found);
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
        outval.setNull();
        return;
    }

    idx nrows = rows ? rows->dim() : propValSet.rows;

    if (dynamic_cast<sUsrFolder*>(wrapper->obj) && !strcmp(name, "type_count")) {
        outval.setDic();
        for (idx i=0; i<nrows; i++) {
            idx row = rows ? (*rows)[i] : i;
            sVariant elt;
            elt.parseInt(propValSet.val(row, valueCol));
            outval.setElt(propValSet.val(row, pathCol), elt);
        }
        return;
    } else if (nrows == 1 && !multi) {
        idx row = rows ? (*rows)[0] : 0;
        sUsrTypeField::parseValue(outval, type, propValSet.val(row, valueCol), sUsrTypeField::fPasswordHide);
        return;
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

    outval.setNull();
}

bool sUsrContext::isUsableField(const sUsrTypeField * fld)
{
    return fld && (allowSysInternal() || !fld->isSysInternal());
}

const sUsrTypeField * sUsrContext::getTypeField(sUsrObjWrapper * objWrapper, const char * name)
{
    const sUsrTypeField * typeField = objWrapper->obj->propGetTypeField(name);
    return isUsableField(typeField) ? typeField : 0;
}

bool sUsrContext::isPropObjectValued(sVariant &objVal, const char *name, bool require_strong_reference)
{
    sUsrObjWrapper *objWrapper = NULL;
    if (evalUsrObjWrapper(&objWrapper, objVal) != EVAL_SUCCESS)
        return false;

    if( require_strong_reference ) {
        const sUsrTypeField * fld = getTypeField(objWrapper, name);
        if( fld && fld->isWeakReference() ) {
            return false;
        }
    }

    if (objWrapper->obj->propGetValueType(name) == sUsrTypeField::eObj)
        return true;

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
            param->outVal->append(val);
        } else {
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
        return EVAL_SUCCESS;
    }

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
        if (!strcmp(name, "_brief")) {
            cacheObjectFields(objVal, "_brief" __);
            cachedVal = objWrapper->fieldCache.get(name);
            assert(cachedVal);
        } else {
            if (const sUsrTypeField * fld = getTypeField(objWrapper, name)) {
                if (fld->canHaveValue()) {
                    sVarSet propValSet;
                    objWrapper->obj->propGet(name, propValSet, false, allowSysInternal());
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
            matcherSql.addString("FALSE");
            break;
        }
    }
    return true;
}

void sUsrContext::getAllObjsOfType(sVariant &outval, const char *typeName, sVariant *propFilter, sUsrObjRes * res, sVariant * res_props)
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
                            outval.setNull();
                            return;
                        }
                        if( prev_len < matcherSql.length() ) {
                            matcherSql.addString(" OR ");
                            have_matchers = true;
                        }
                    }
                    if( have_matchers ) {
                        matcherSql.cut(matcherSql.length() - 4);
                        matcherSql.addString(")");
                    } else {
                        matcherSql.cut0cut();
                    }
                } else {
                    if( !printMatcherSql(matcherSql, fielddesc, relevantTypes[it]->getFieldType(*_usr, fieldname), _usr->db()) ) {
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
    _usr->objsLowLevel(typeName, 0, propFilterSql, prop_name_csv.length() ? prop_name_csv.ptr() : 0, false, 0, 0, res, 0, allowSysInternal());
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
    objWrapper->obj->propBulk(propsSet, NULL, fieldNames00, allowSysInternal());
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

        sVariant *cachedVal = objWrapper->fieldCache.get(name);
        if (!cachedVal) {
            cachedVal = objWrapper->fieldCache.set(name);
            parseField(*cachedVal, objWrapper, name, propsSet, propRows.get(name), 2, 3);
        }

    }

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
            result.setBool(false);
            return true;
        }

        result.setBool(obj->isTypeOf(args[0].asString()));
        return true;
    }
};


class sUsrQueryBuiltin_alloftype : public BuiltinFunction {
public:
    sUsrQueryBuiltin_alloftype() { _name.printf("builtin alloftype() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicNone(ctx, topic) || !checkNArgs(ctx, nargs, 1, 2))
            return false;


        static_cast<sUsrContext&>(ctx).getAllObjsOfType(result, args[0].asString(), nargs>1 ? args+1 : 0);

        return true;
    }
};


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
            if( !prop || !prop[0] || prop[0] == '_' ) {
                continue;
            }
            if (!static_cast<sUsrContext&>(ctx).isPropObjectValued(obj_val, prop, true))
                continue;

            props00.printf("%s", prop);
            props00.add0();
        }
        props00.add0(2);

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

        for (const char *spec = getDefaultProps00(static_cast<sUsrContext&>(ctx).getFlags()); spec && *spec; spec = sString::next00(spec))
            *(columns.set(spec)) = true;

        if (haveColList) {
            for (idx j=0; j<args[0].dim(); j++) {
                const char *col = args[0].getListElt(j)->asString();
                if (!strcmp("_summary", col))
                    haveSummary = true;
                else
                    *(columns.set(col)) = true;
            }
        }

        if (haveSummary || !haveColList) {
            sVariant summaryOpt;
            summaryOpt.setString("_summary");

            for (idx i=start; i<end; i++) {
                sVariant elt, eltkeys;
                topic->getElt(i, elt);

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


        sStr cacheFieldNames00;
        sVariantTblData * tbld = new sVariantTblData(sMax<idx>(1, columns.dim()), 0);

        for (idx j=0; j<columns.dim(); j++) {
            const char *col = static_cast<const char *>(columns.id(j));
            tbld->getTable().setVal(-1, j, col);

            cacheFieldNames00.printf("%s", col);
            cacheFieldNames00.add0();
        }
        if (haveSummary) {
            cacheFieldNames00.printf("_summary");
            cacheFieldNames00.add0();
        }
        cacheFieldNames00.add0(2);

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

        if (wantInfo) {
            idx ir = tbld->getTable().rows();
            tbld->getTable().setVal(ir, 0, "info");
            tbld->getTable().setVal(ir, 1, start);
            tbld->getTable().setVal(ir, 2, topic->dim());
        }

        result.setData(*tbld);


        return true;
    }
};


class sUsrQueryBuiltin_validobj : public BuiltinFunction {
public:
    sUsrQueryBuiltin_validobj() { _name.printf("builtin validobj() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic && (!topic->isHiveId() || !ctx.evalValidObjectId(*topic))) {
            result.setBool(false);
            return true;
        }

        for (idx i=0; i<nargs; i++) {
            if (!args[i].isHiveId() || !ctx.evalValidObjectId(args[i])) {
                result.setBool(false);
                return true;
            }
        }

        result.setBool(true);
        return true;
    }
};

#define REGISTER_BUILTIN_FUNC(name) \
static sUsrQueryBuiltin_ ## name builtin_ ## name; \
registerBuiltinFunction(#name, builtin_ ## name)


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

class sUsrQueryBuiltin_filepath: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_filepath()
        {
            _name.printf("builtin filepath() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !topic ) {
                if( !checkNArgs(ctx, nargs, 1, sIdxMax) ) {
                    return false;
                }
                topic = args;
                args++;
                nargs--;
            }

            sUsrObj * obj = NULL;
            eEvalStatus status;
            if( (status = static_cast<sUsrContext&>(ctx).evalUsrObj(&obj, *topic)) != EVAL_SUCCESS ) {
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

class sUsrQueryBuiltin_setenv: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_setenv()
        {
            _name.printf("builtin setenv() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !checkNArgs(ctx, nargs, 0, 3) ) {
                return false;
            }
            const char * env = getArgAsString(0, args, nargs, nargs > 0 ? 0 : "mico2");
            const char * sub = getArgAsString(1, args, nargs, "base");
            const bool deact = getArgAsBool(2, args, nargs, false);
            result.setString("");
            if( deact ) {
                result.append("conda deactivate || true;\n");
            }
            if( env ) {
                result.append(". \"${QPRIDE_BIN}/%s-os${os}/bin/activate\" \"%s\";\n", env, sub);
            }
            return true;
        }
};

class sUsrQueryBuiltin_bash_common: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_bash_common()
        {
            _name.printf("builtin bash_common() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            result.setString(". ${QPRIDE_BIN}/qlang.sh.os${os}\n");
            return true;
        }
};

class sUsrQueryBuiltin_getWorkDir: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_getWorkDir()
        {
            _name.printf("builtin getWorkDir() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !checkNArgs(ctx, nargs, 0, 0) ) {
                return false;
            }
            sUsrInternalContext * ictx = dynamic_cast<sUsrInternalContext*>(&ctx);
            sStr path;
            if( ictx && ictx->getQPride() ) {
                if(! ictx->getQPride()->getWorkDir(path) ) {
                    return false;
                }
            } else {
                return false;
            }
            result.setString(path.ptr(), path.length());
            return true;
        }
};


class sUsrQueryBuiltin_sliceid: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_sliceid()
        {
            _name.printf("builtin sliceid() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !checkNArgs(ctx, nargs, 0, 0) ) {
                return false;
            }
            sUsrInternalContext * ictx = dynamic_cast<sUsrInternalContext*>(&ctx);
            sQPrideProc * qpp = 0;
            if ( ictx && ictx->getQPride() ) {
                qpp = dynamic_cast<sQPrideProc*>(ictx->getQPride());
            }
            if ( qpp ) {
                result.setInt(qpp->reqSliceId);
            } else {
                result.setInt(0);
            }
            return true;
        }
};

class sUsrQueryBuiltin_slicecount: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_slicecount()
        {
            _name.printf("builtin slicecount() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !checkNArgs(ctx, nargs, 0, 0) ) {
                return false;
            }
            sUsrInternalContext * ictx = dynamic_cast<sUsrInternalContext*>(&ctx);
            sQPrideProc * qpp = 0;
            if ( ictx && ictx->getQPride() ) {
                qpp = dynamic_cast<sQPrideProc*>(ictx->getQPride());
            }
            if ( qpp ) {
                result.setInt(qpp->reqSliceCnt);
            } else {
                result.setInt(1);
            }
            return true;
        }
};

class sUsrQueryBuiltin_slicesize: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_slicesize()
        {
            _name.printf("builtin slicesize() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !checkNArgs(ctx, nargs, 0, 0) ) {
                return false;
            }

            sUsrInternalContext * ictx = dynamic_cast<sUsrInternalContext*>(&ctx);
            sQPrideProc * qpp = 0;
            if ( ictx && ictx->getQPride() ) {
                qpp = dynamic_cast<sQPrideProc*>(ictx->getQPride());
            }

            if ( qpp ) {
                sUsrObj * obj = 0;
                if (qpp->objs.dim() > 0) {
                    obj = qpp->objs.ptr(0);
                }
                result.setString(qpp->getSplitSize(obj));
            } else {
                result.setNull();
            }
            return true;
        }
};

class sUsrQueryBuiltin_configGet: public BuiltinFunction
{
    public:
        sUsrQueryBuiltin_configGet()
        {
            _name.printf("builtin configGet() function");
        }
        virtual bool call(sVariant & result, Context & ctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            if( !checkNArgs(ctx, nargs, 1, 2) ) {
                return false;
            }
            const char * name = getArgAsString(0, args, nargs, nullptr);
            const char * dflt = getArgAsString(1, args, nargs, nullptr);

            sUsrInternalContext * ictx = dynamic_cast<sUsrInternalContext*>(&ctx);
            sQPrideBase * qpp = (ictx && ictx->getQPride()) ? ictx->getQPride() : nullptr;
            if ( qpp && sLen(name) ) {
                sStr buf;
                qpp->cfgStr(&buf, 0, name, dflt);
                result.setString(buf.ptr());
            } else {
                result.setString(dflt);
            }
            return true;
        }
};

bool sUsrQueryBuiltinFunction::call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
{
    result.setNull();
    sRC rc;
    sUsrQueryBuiltinImpl * impl = getImpl(ctx, *this);
    if( impl ) {
        rc = impl->call(result, ctx, args, nargs);
        delete impl;
    } else {
        rc = RC(sRC::eRunning, sRC::eQuery, sRC::eFunction, sRC::eZero);
    }
    if( rc.isSet() ) {
        ctx.setError(EVAL_OTHER_ERROR, "%s", rc.print());
    }
    return rc == sRC::zero;
}


void sUsrQueryBuiltinBase_files::registerVariables(Context & ctx)
{
    ctx.registerBuiltinIdxConst("fg_none", eFP_None);
    ctx.registerBuiltinIdxConst("fg_obj_name", eFP_ObjName);
    ctx.registerBuiltinIdxConst("fg_obj_id", eFP_ObjID);
    ctx.registerBuiltinIdxConst("fg_keep_original_read_ids", eFP_KeepOriginalIDs);
    ctx.registerBuiltinIdxConst("fg_concatenate_objects", eFX_ConcatenateObjects);
    ctx.registerBuiltinIdxConst("fg_pair_in_one", eUO_PairInOne);
    ctx.registerBuiltinIdxConst("fg_pair_split", eUO_PairSplit);
}

sRC sUsrQueryBuiltinBase_files::call(sVariant &result, Context &ctx, sVariant *args, const idx& nargs)
{
    result.setNull();
    sRC rc = parseArgs(ctx, args, nargs);
    if( !rc.isSet() ) {
        sStr outPathList;
        rc = iterateObjects(ctx, outPathList);
        if( !rc.isSet() && outPathList ) {
            result.setSprintf("%s", outPathList.ptr());
        }
    }
    return rc;
}

sRC sUsrQueryBuiltinBase_files::parseArgs(Context &ctx, sVariant *args, const idx& nargs)
{
    if( nargs > maxArgs() ) {
        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eParameter, sRC::eExcessive);
    }
    if( args && nargs > 0 ) {
        _func.getArgAsHiveIds(0, args, nargs, _ids);
        _nameTemplate = _func.getArgAsString(1, args, nargs);
        _flags = _func.getArgAsUInt(2, args, nargs, eFP_ObjID);
        _separator = _func.getArgAsString(3, args, nargs, " ");
        return sRC::zero;
    }
    ctx.setError(EVAL_BAD_ARGS_NUMBER, "%s function expects object(s) as first argument", _func.getName());
    return RC(sRC::eEvaluating, sRC::eCallback, sRC::eArguments, sRC::eNotProperlyPassed);
}

sRC sUsrQueryBuiltinBase_files::iterateObjects(Context & ctx, sStr & outPathList)
{
    sUsrInternalContext * ictx = dynamic_cast<sUsrInternalContext*>(&ctx);
    sUsr * user = ictx ? ictx->getUsr() : 0;
    sRC res;
    if( user ) {
        const BuiltinFunction * wdf = ctx.getBuiltin("getWorkDir");
        if( wdf && wdf->call(_wDir, ctx, 0, 0, 0) && !_wDir.isNull() ) {
            sDic<bool> visited;
            for(idx i = 0; !res.isSet() && i < _ids.dim(); ++i) {
                if( !visited.find(&_ids[i], sizeof(_ids[0])) ) {
                    if( !_ids[i] ) {
                        continue;
                    }
                    sUsrObj * curObject = user->objFactory(_ids[i]);
                    if( curObject ) {
                        *(visited.set(&_ids[i], sizeof(_ids[0]))) = true;
                        sStr files00;
                        res = files(*ictx, *curObject, files00);
                        for(const char * src = files00; src && src[0]; src = sString::next00(src)) {
                            sStr fpath;
                            curObject->getFilePathname(fpath, "%s", src);
#if _DEBUG
                            fprintf(stderr, "Processing path %s '%s'\n", curObject->IdStr(), fpath.ptr());
#endif
                            sStr fileName;
                            res = make_name(*ictx, *curObject, fpath.ptr(), fileName);
                            if( !res.isSet() ) {
                                sStr full;
                                if( sDir::uniqueName(full, "%s%s", _wDir.asString(), fileName.ptr()) ) {
                                    res = make_file(*ictx, *curObject, fpath.ptr(), full.ptr());
                                    if( !res.isSet() ) {
                                        fileName.cut0cut();
                                        if( sDir::cleanUpName(full.ptr(), fileName, true) ) {
                                            if( _separator && outPathList.length() ) {
                                                outPathList.printf("%s", _separator);
                                            }
                                            outPathList.printf("%s", fileName.ptr(_wDir.dim()));
#if _DEBUG
                                            fprintf(stderr, "Added path %s '%s'\n", curObject->IdStr(), fileName.ptr(_wDir.dim()));
#endif
                                        } else {
                                            res = RC(sRC::eCleaningUp, sRC::eFile, sRC::eOperation, sRC::eFailed);
                                        }
                                    }
                                }
                            }
                            if( res.val.parts.bad_entity == sRC::eFile && res.val.parts.state == sRC::eIgnored ) {
#if _DEBUG
                                fprintf(stderr, "Ignored file %s '%s'\n", curObject->IdStr(), fpath.ptr());
#endif
                                res = sRC::zero;
                            }
                        }
                        if( res.val.parts.bad_entity == sRC::eFile && res.val.parts.state == sRC::eIgnored ) {
#if _DEBUG
                            fprintf(stderr, "Ignored object %s\n", curObject->IdStr());
#endif
                            res = sRC::zero;
                        }
                        delete curObject;
                    } else {
                        res = RC(sRC::eAccessing, sRC::eObject, sRC::eOperation, sRC::eFailed);
                    }
                } else {
                    res = RC(sRC::eEvaluating, sRC::eCallback, sRC::eObject, sRC::eNotFound);
                }
            }
        } else {
            ctx.setError(EVAL_VARIABLE_ERROR, "Working directory not set");
            res = RC(sRC::eEvaluating, sRC::eCallback, sRC::eVariable, sRC::eUndefined);
        }
    } else {
        res = RC(sRC::eEvaluating, sRC::eCallback, sRC::eUser, sRC::eInvalid);
    }
    return res;
}

sRC sUsrQueryBuiltinBase_files::make_name(sUsrInternalContext & ctx, sUsrObj & obj, const char * const src, sStr & dst)
{
    sUsrFile *ufile = dynamic_cast<sUsrFile*>(&obj);
    sFilePath orig(src, "%%flnmx");
    if( ufile == 0 && orig.length() > 1 && orig[0] == '_' && orig[1] == '.' ) {
        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
    }
    const char * path = 0;
    if( _flags & eFP_ObjName ) {
        path = obj.propGet("name");
    }
    if( !path || !path[0] ) {
        path = _nameTemplate;
    }
    if( !path || !path[0] ) {
        path = src;
    }
    sFilePath nm(path, "%%flnmx");
    nm.shrink00();
    if( (_flags & eFP_ObjID) || !nm ) {
        dst.printf("o%s%s", obj.Id().print(), nm ? "_" : "");
    }
    if( nm ) {
        dst.printf("%s", nm.ptr());
    }
    return add_extension(ctx, src, dst);
}

class sUsrQueryBuiltin_files: public sUsrQueryBuiltinFunction
{
    public:
        sUsrQueryBuiltin_files()
            : sUsrQueryBuiltinFunction("files")
        {
        }
        virtual ~sUsrQueryBuiltin_files()
        {
        }
        virtual sUsrQueryBuiltinImpl * getImpl(Context & ctx, const BuiltinFunction &func) const
        {
            return new files_impl(func);
        }

    private:

        class files_impl: public sUsrQueryBuiltinBase_files
        {
                typedef sUsrQueryBuiltinBase_files TParent;
                sStr _masks;

            public:
                files_impl(const BuiltinFunction & func)
                    : TParent(func)
                {
                }
                virtual ~files_impl()
                {
                }
                virtual idx maxArgs()
                {
                    return 5;
                }

                sRC parseArgs(qlang::Context &ctx, sVariant *args, const idx &nargs)
                {
                    const char *masks = _func.getArgAsString(4, args, nargs, "*");
                    sString::searchAndReplaceSymbols(&_masks, masks, 0, ";", 0, 0, true, true, true, true);
                    return TParent::parseArgs(ctx, args, nargs);
                }

                sRC files(sUsrInternalContext &ctx, sUsrObj &obj, sStr &list00)
                {
                    sUsrFile *ufile = dynamic_cast<sUsrFile*>(&obj);
                    sDir list;
                    for(const char *m = _masks.ptr(); m; m = sString::next00(m)) {
                        sStr fl("%s%s", ufile ? "_." : "", m);
#if _DEBUG
                        fprintf(stderr, "%s::files %s '%s'\n", _func.getName(), obj.IdStr(), fl.ptr());
#endif
                        obj.files(list, sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs), fl.ptr());
                    }
                    list00.add(list.ptr(), list.length());
                    list00.add0(2);
                    return sRC::zero;
                }

                sRC make_name(sUsrInternalContext &ctx, sUsrObj &obj, const char *const src, sStr &dst)
                {
                    sUsrFile *ufile = dynamic_cast<sUsrFile*>(&obj);
                    if( !ufile && strcmp(_masks, "*") == 0 ) {
                        _flags &= ~eFP_ObjID;
                        sStr opath("%s%s", _wDir.asString(), obj.IdStr());
                        if( sDir::makeDir(opath) ) {
                            dst.printf("%s/", obj.IdStr());
                        } else {
                            return RC(sRC::eCreating, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    return TParent::make_name(ctx, obj, src, dst);
                }

                sRC add_extension(sUsrInternalContext &ctx, const char *src, sStr &dst)
                {
                    sFilePath ext(src, "%%ext");
                    sFilePath nm(src, "%%flnm");
                    if( sIs("_.", nm) ) {
                        ext.printf(0, "%s", nm.ptr(2));
                    }
                    ext.shrink00();
                    dst.printf("%s%s", ext ? "." : "", ext.ptr());
                    return sRC::zero;
                }
                sRC make_file(sUsrInternalContext &ctx, sUsrObj &obj, const char *src, const char *dst)
                {
                    if( !sFile::symlink(src, dst) ) {
#if _DEBUG
                        fprintf(stderr, "Failed to link for %s '%s' to '%s'\n", obj.IdStr(), src, dst);
#endif
                        return RC(sRC::eCreating, sRC::eSymLink, sRC::eOperation, sRC::eFailed);
                    }
                    return sRC::zero;
                }
        };
};

void sUsrInternalContext::registerDefaultBuiltins()
{
    registerBuiltinUdxConst("DEBUG_MODE"
#if _DEBUG
    , 1);
#else
    , 0);
#endif
    sUsrQueryBuiltinBase_files::registerVariables(*this);
    REGISTER_BUILTIN_FUNC(argstring);
    REGISTER_BUILTIN_FUNC(filepath);
    REGISTER_BUILTIN_FUNC(setenv);
    REGISTER_BUILTIN_FUNC(bash_common);
    REGISTER_BUILTIN_FUNC(files);
    REGISTER_BUILTIN_FUNC(getWorkDir);
    REGISTER_BUILTIN_FUNC(sliceid);
    REGISTER_BUILTIN_FUNC(slicecount);
    REGISTER_BUILTIN_FUNC(slicesize);
    REGISTER_BUILTIN_FUNC(configGet);
}


sUsrEngine::sUsrEngine()
    : TParent(true)
{
    _usr = 0;
    _ctx = 0;
}

sUsrEngine::sUsrEngine(const sUsr & usr, idx ctx_flags)
    : TParent(true)
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
    if (!tree.useForm(form)) {
        return false;
    }
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

bool sUsrEngine::eval(const char * query, const idx query_len, sVariant & result, sStr * errorMsg)
{
    const bool retval = parse(query, query_len, errorMsg) && TParent::eval(result, errorMsg);
    reset();
    return retval;
}

bool sUsrEngine::evalTemplate(const char * tmpl, const idx tmpl_len, sVariant & result, sStr * errorMsg)
{
    const bool retval = parseTemplate(tmpl, tmpl_len, errorMsg) && TParent::eval(result, errorMsg);
    reset();
    return retval;
}


sUsrInternalEngine::sUsrInternalEngine(sQPrideBase * qp, sUsr &usr, idx ctx_flags)
{
    _usr = &usr;
    _ctx = new sUsrInternalContext(qp, usr, ctx_flags);
}

void sUsrInternalEngine::init(sQPrideBase * qp, sUsr &usr, idx ctx_flags)
{
    delete _ctx;
    _usr = &usr;
    _ctx = new sUsrInternalContext(qp, usr, ctx_flags);
}
