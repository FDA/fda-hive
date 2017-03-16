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
#include <slib/std/string.hpp>
#include <qlang/interpreter.hpp>
#include <qlang/ast.hpp>
#include <assert.h>

using namespace slib;
using namespace slib::qlang;
using namespace slib::qlang::ast;

// keep in sync with qlang::ast::Node::eType
static const char *NodeNames[] = {
    "uninitialized syntax node",
    "null literal",
    "int literal",
    "real literal",
    "string literal",
    "list literal",
    "dic literal",
    "variant literal",
    "variable name",
    "junction expression",
    "'as' typecast",
    "'=' operation",
    "'++' operation",
    "'--' operation",
    "'+' operation",
    "'-' operation",
    "'*' operation",
    "'/' operation",
    "'%' operation",
    "'+=' operation",
    "'-=' operation",
    "'*=' operation",
    "'/=' operation",
    "'%=' operation",
    "unary '+' operation",
    "unary '-' operation",
    "'[]' expression",
    "'[:]' operation",
    "'==' operation",
    "'!=' operation",
    "'>' operation",
    "'<=' operation",
    "'<' operation",
    "'<=' operation",
    "'<=>' operation",
    "'has' operation",
    "'=~' operation",
    "'!~' operation",
    "'&&' operation",
    "'||' operation",
    "'!' operation",
    "'?:' ternary conditional",
    "property access",
    "function call",
    "format call",
    "method call",
    "$ call",
    "'if' statement",
    "'for' statement",
    "'while' statement",
    "statement block",
    "unbreakable block",
    "function declaration",
    "'return' statement",
    "'break' statement",
    "'continue' statement",
    NULL
};

const char* Node::getTypeName() const
{
    return NodeNames[(int)_type];
}

void Node::print(sStr &s) const
{
    if (_loc.print(s))
        s.printf(" : ");
    s.printf("%s @ %p\n", getTypeName(), this);
}

enum checkCtxModeFlags {
    flag_POP_SCOPE = 1,
    flag_IGNORE_RETURN = 1 << 1,
    flag_IGNORE_BREAK_CONTINUE = 1 << 2,
};

// returns -1 on error, 0 on break, 1 on normal
static idx checkCtxMode(Node *node, Context &ctx, idx flags=0)
{
    switch (ctx.getMode()) {

    case Context::MODE_NORMAL:
        break;

    case Context::MODE_RETURN:
        if (flags & flag_IGNORE_RETURN)
            return 1;

        if (flags & flag_POP_SCOPE)
            ctx.popScope();
        return 0;

    case Context::MODE_BREAK:
    case Context::MODE_CONTINUE:
        if (flags & flag_IGNORE_BREAK_CONTINUE)
            return 1;

        if (!ctx.isScopeInLoop()) {
            ctx.setError(node->getLocation(), EVAL_SYNTAX_ERROR, "cannot break/continue: no enclosing loop scope");
            return -1;
        }
        if (flags & flag_POP_SCOPE)
            ctx.popScope();
        return 0;

    case Context::MODE_ERROR:
        return -1;
    }
    return 1;
}

#define CHECK_RUN(node, flags) \
do { \
    if (!(node)->run(ctx)) \
        return false; \
    idx c = checkCtxMode(node, ctx, flags); \
    if (c <= 0) \
        return !c; \
} while(0)

#define CHECK_EVAL(node, value, flags) \
do { \
    if (!(node)->eval(value, ctx)) \
        return false; \
    idx c = checkCtxMode(node, ctx, flags); \
    if (c <= 0) \
        return !c; \
} while(0)

bool Node::run(Context &ctx) const
{
    sVariant dummy;
    return eval(dummy, ctx);
}

bool Node::eval(sVariant &result, Context &ctx) const
{
    ctx.setError(getLocation(), EVAL_NOT_IMPLEMENTED, "Cannot evaluate %s", getTypeName());
    return false;
}

Unary::~Unary()
{
    delete _arg;
}

void Unary::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tArgument: %s @ %p\n", _arg ? _arg->getTypeName() : NULL, _arg);
    if (_arg)
        _arg->print(s);
}

Binary::~Binary()
{
    delete _lhs;
    delete _rhs;
}

void Binary::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tArguments: %s @ %p; %s @ %p\n", _lhs ? _lhs->getTypeName() : NULL, _lhs, _rhs ? _rhs->getTypeName() : NULL, _rhs);
    if (_lhs)
        _lhs->print(s);
    if (_rhs)
        _rhs->print(s);
}

Nary::~Nary()
{
    for (idx i=0; i<_elts.dim(); i++)
        delete _elts[i];
}

void Nary::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tArguments (%"DEC"):", _elts.dim());
    for (idx i=0; i<_elts.dim(); i++) {
        s.printf(" %s @ %p;", _elts[i]->getTypeName(), _elts[i]);
    }
    s.printf("\n");
    for (idx i=0; i<_elts.dim(); i++) {
        _elts[i]->print(s);
    }
}

void Nary::addElement(Node *node)
{
    _elts.vadd(1, node);
}

bool ScalarLiteral::eval(sVariant &result, Context &ctx) const
{
    result = _val;
    return true;
}

void ScalarLiteral::print(sStr &s) const
{
    if (_loc.print(s))
        s.printf(" : ");
    s.printf("%s value ", getTypeName());
    _val.print(s);
    s.printf(" @ %p\n", this);
}

IntLiteral::IntLiteral(idx i, idx line, idx col): ScalarLiteral(line, col)
{
    this->_type = node_INT_LITERAL;
    this->_val.setInt(i);
}

RealLiteral::RealLiteral(real r, idx line, idx col): ScalarLiteral(line, col)
{
    this->_type = node_REAL_LITERAL;
    this->_val.setReal(r);
}

StringLiteral::StringLiteral(const char *s, idx line, idx col): ScalarLiteral(line, col)
{
    this->_type = node_STRING_LITERAL;
    this->_val.setString(s);
}

bool ListLiteral::eval(sVariant &result, Context &ctx) const
{
    result.setList();
    for (idx i=0; i<this->_elts.dim(); i++) {
        sVariant val;
        CHECK_EVAL(this->_elts[i], val, 0);
        result.push(val);
    }
    return true;
}

bool DicLiteral::eval(sVariant &result, Context &ctx) const
{
    result.setDic();
    for (idx i=0; i<this->_elts.dim(); i+=2) {
        sVariant key, val;
        CHECK_EVAL(this->_elts[i], key, 0);
        CHECK_EVAL(this->_elts[i+1], val, 0);
        result.setElt(key.asString(), val);
    }
    return true;
}

VariantLiteral::VariantLiteral(sVariant & val, idx line, idx col): ScalarLiteral(line, col)
{
    this->_type = node_VARIANT_LITERAL;
    this->_val = val;
}

bool BoolCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setInt(val.asBool());
    return true;
}

bool IntCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setInt(val.asInt());
    return true;
}

bool UIntCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setUInt(val.asUInt());
    return true;
}

bool IntlistCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val, elt;
    CHECK_EVAL(this->_arg, val, 0);
    result.setList();
    // cast lists per-element
    // cast string scalars as comma-, semicolon-, or whitespace-separated lists
    // cast non-string scalars as one-element lists
    if (val.isList()) {
        for (idx i=0; i<val.dim(); i++) {
            val.getElt(i, elt);
            if (!elt.isInt())
                elt.setInt(elt.asInt());
            result.push(elt);
        }
    } else if (val.isString()) {
        result.parseIntList(val.asString());
    } else if (val.isInt()) {
        elt = val;
        result.push(val);
    } else {
        elt.setInt(val.asInt());
        result.push(elt);
    }
    return true;
}

bool ObjlistCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val, elt;
    CHECK_EVAL(this->_arg, val, 0);
    result.setList();
    // cast lists per-element
    // cast string scalars as comma-, semicolon-, or whitespace-separated lists
    // cast non-string scalars as one-element lists
    if (val.isList()) {
        for (idx i=0; i<val.dim(); i++) {
            val.getElt(i, elt);
            if (!elt.isHiveId())
                elt.parseHiveId(elt.asString());
            result.push(elt);
        }
    } else if (val.isString()) {
        result.parseHiveIdList(val.asString());
    } else if (val.isHiveId()) {
        result.push(val);
    } else if (val.isNumeric()) {
        elt.setHiveId(0, val.asUInt(), 0);
        result.push(elt);
    } else {
        elt.setHiveId(0, 0, 0);
        result.push(elt);
    }
    ctx.declareObjlist(result);
    return true;
}

bool RealCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setReal(val.asReal());
    return true;
}

bool StringCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setString(val.asString());
    return true;
}

bool ObjCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    if (val.isHiveId()) {
        result = val;
    } else if (val.isNumeric()) {
        result.setHiveId(0, val.asUInt(), 0);
    } else if (val.isString()) {
        result.parseHiveId(val.asString());
    } else {
        result.setHiveId(0, 0, 0);
    }
    return true;
}

bool DateTimeCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setDateTime(val);
    return true;
}

bool DateCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setDate(val);
    return true;
}

bool TimeCast::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(this->_arg, val, 0);
    result.setTime(val);
    return true;
}

bool Variable::eval(sVariant &result, Context &ctx) const
{
    if (!ctx.getVarValue(_var.ptr(), result)) {
        ctx.setError(getLocation(), EVAL_VARIABLE_ERROR, "undefined variable '%s'", _var.ptr());
        return false;
    }
    return true;
}

void Variable::print(sStr &s) const
{
    if (_loc.print(s))
        s.printf(" : ");
    s.printf("%s \"%s\" @ %p\n", getTypeName(), _var.ptr(), this);
}

bool Assign::assign(Node *lhs, sVariant &val, Context &ctx) const
{
    assert(lhs);

    eEvalStatus code;
    Property *prop_node = NULL;
    Node *topic_node = NULL;
    const char *name = NULL;

    // We can assign to variables, properties, or list subscripts
    switch (lhs->getType()) {
    case node_VARIABLE:
        name = dynamic_cast<Variable*>(lhs)->getName();
        if (!ctx.setVarValue(name, val)) {
            ctx.setError(getLocation(), EVAL_READ_ONLY_ERROR, "assignment to %s failed", name);
            return false;
        }
        return true;

    case node_PROPERTY:
        prop_node = dynamic_cast<Property*>(lhs);
        // Is the property node applied to an object?
        topic_node = prop_node->getTopic();
        if (topic_node) {
            sVariant topic_val;
            CHECK_EVAL(topic_node, topic_val, 0);
            code = ctx.evalSetProperty(topic_val, prop_node->getName(), val);
        } else {
            code = ctx.evalSetProperty(prop_node->getName(), val);
        }
        if (code != EVAL_SUCCESS) {
            ctx.setError(getLocation(), code, "assignment to a property failed");
            return false;
        }
        return true;

    case node_OP_SUBSCRIPT:
        {
            Subscript *sub_node = dynamic_cast<Subscript*>(lhs);
            Node *list_node = sub_node->getLhs();
            Node *index_node = sub_node->getRhs();
            sVariant list_val, index_val;
            CHECK_EVAL(list_node, list_val, 0);
            CHECK_EVAL(index_node, index_val, 0);
            code = ctx.evalSetSubscript(list_val, index_val, val);
            if (code != EVAL_SUCCESS) {
                ctx.setError(getLocation(), code, "assignment to an element failed");
                return false;
            }
        }
        return true;

    default: // fall through
        break;
    }

    ctx.setError(getLocation(), EVAL_NOT_IMPLEMENTED, "cannot assign to a %s", lhs->getTypeName());
    return false;
}

bool Assign::eval(sVariant &result, Context &ctx) const
{
    CHECK_EVAL(_rhs, result, 0);
    return assign(_lhs, result, ctx);
}

bool Precrement::eval(sVariant &result, Context &ctx) const
{
    CHECK_EVAL(_lhs, result, 0);

    eEvalStatus code = _type == node_OP_INCREMENT ? ctx.evalIncrement(result) : ctx.evalDecrement(result);
    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }

    return assign(_lhs, result, ctx);
}

bool Postcrement::eval(sVariant &result, Context &ctx) const
{
    CHECK_EVAL(_lhs, result, 0);

    sVariant crement_val = result;
    eEvalStatus code = _type == node_OP_INCREMENT ? ctx.evalIncrement(crement_val) : ctx.evalDecrement(crement_val);
    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }

    return assign(_lhs, crement_val, ctx);
}

Arithmetic::Arithmetic(Node *lhs, char op, Node *rhs, idx line, idx col) : Binary(lhs, rhs, line, col)
{
    switch(op) {
    case '+':
        _type = node_OP_PLUS;
        break;
    case '-':
        _type = node_OP_MINUS;
        break;
    case '*':
        _type = node_OP_MULTIPLY;
        break;
    case '/':
        _type = node_OP_DIVIDE;
        break;
    case '%':
        _type = node_OP_REMAINDER;
        break;
    default:
        assert(0);
        break;
    }
}

bool Arithmetic::eval(sVariant &result, Context &ctx) const
{
    sVariant lval, rval;
    eEvalStatus code = EVAL_OTHER_ERROR;

    CHECK_EVAL(_lhs, lval, 0);
    CHECK_EVAL(_rhs, rval, 0);

    switch(this->_type) {
    case node_OP_PLUS:
    case node_OP_PLUS_INPLACE:
        code = ctx.evalAdd(result, lval, rval);
        break;
    case node_OP_MINUS:
    case node_OP_MINUS_INPLACE:
        code = ctx.evalSubtract(result, lval, rval);
        break;
    case node_OP_MULTIPLY:
    case node_OP_MULTIPLY_INPLACE:
        code = ctx.evalMultiply(result, lval, rval);
        break;
    case node_OP_DIVIDE:
    case node_OP_DIVIDE_INPLACE:
        code = ctx.evalDivide(result, lval, rval);
        break;
    case node_OP_REMAINDER:
    case node_OP_REMAINDER_INPLACE:
        code = ctx.evalRemainder(result, lval, rval);
        break;
    default:
        assert(0);
        break;
    }

    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }
    return true;
}

ArithmeticInplace::ArithmeticInplace(Node *lhs, char op, Node *rhs, idx line, idx col): Arithmetic(lhs, op, rhs, line, col), assigner(0, 0, line, col)
{
    switch(op) {
    case '+':
        _type = node_OP_PLUS_INPLACE;
        break;
    case '-':
        _type = node_OP_MINUS_INPLACE;
        break;
    case '*':
        _type = node_OP_MULTIPLY_INPLACE;
        break;
    case '/':
        _type = node_OP_DIVIDE_INPLACE;
        break;
    case '%':
        _type = node_OP_REMAINDER_INPLACE;
        break;
    default:
        assert(0);
        break;
    }
}

bool ArithmeticInplace::eval(sVariant &result, Context &ctx) const
{
    return Arithmetic::eval(result, ctx) && assigner.assign(_lhs, result, ctx);
}

UnaryPlusMinus::UnaryPlusMinus(char op, Node *node, idx line, idx col): Unary(node, line, col)
{
    switch(op) {
    case '+':
        _type = node_OP_U_PLUS;
        break;
    case '-':
        _type = node_OP_U_MINUS;
        break;
    default:
        assert(0);
        break;
    }
}

bool UnaryPlusMinus::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(_arg, val, 0);

    eEvalStatus code;

    if (this->_type == node_OP_U_PLUS)
        code = ctx.evalUnaryPlus(val);
    else
        code = ctx.evalUnaryMinus(val);

    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }
    result = val;
    return true;
}

bool Subscript::eval(sVariant &result, Context &ctx) const
{
    sVariant lval, index;
    CHECK_EVAL(_lhs, lval, 0);
    CHECK_EVAL(_rhs, index, 0);

    eEvalStatus code = ctx.evalGetSubscript(result, lval, index);
    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }
    return true;
}

Slice::~Slice()
{
    delete _lhs;
    delete _rhs1;
    delete _rhs2;
}

bool Slice::eval(sVariant &result, Context &ctx) const
{
    sVariant lval, index1, index2;
    CHECK_EVAL(_lhs, lval, 0);
    CHECK_EVAL(_rhs1, index1, 0);
    CHECK_EVAL(_rhs2, index2, 0);

    eEvalStatus code = ctx.evalGetSlice(result, lval, index1, index2);
    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }
    return true;
}

void Slice::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tArguments: %s @ %p [ %s @ %p : %s @ %p ]\n", _lhs->getTypeName(), _lhs, _rhs1->getTypeName(), _rhs1, _rhs2->getTypeName(), _rhs2);
    _lhs->print(s);
    _rhs1->print(s);
    _rhs2->print(s);
}

Equality::Equality(Node *lhs, char op, Node *rhs, idx line, idx col): Binary(lhs, rhs, line, col)
{
    switch(op) {
    case '=':
        _type = node_OP_EQ;
        break;
    case '!':
        _type = node_OP_NE;
        break;
    default:
        assert(0);
        break;
    }
}

bool Equality::eval(sVariant &result, Context &ctx) const
{
    sVariant lval, rval;
    Junction *junc_node = dynamic_cast<Junction*>(_rhs);
    CHECK_EVAL(_lhs, lval, 0);

    // for equality operation, rhs can be a junction
    if (junc_node) {
        // x == a|b means "x == a || x == b"
        // x != a|b means "x != a && x != b"
        for (idx i=0; i<junc_node->dim(); i++) {
            CHECK_EVAL(junc_node->getElement(i), rval, 0);
            if (ctx.evalEquality(lval, rval)) {
                result.setInt(_type == node_OP_EQ);
                return true;
            }
        }
        result.setInt(_type != node_OP_EQ);
    } else {
        CHECK_EVAL(_rhs, rval, 0);
        bool match = ctx.evalEquality(lval, rval);
        result.setInt(_type == node_OP_EQ ? match : !match);
    }
    return true;
}

Comparison::Comparison(Node *lhs, const char *op, Node *rhs, idx line, idx col): Binary(lhs, rhs, line, col)
{
    switch(op[0]) {
    case '<':
        _type = op[1] ? op[2] ? node_OP_CMP : node_OP_LE : node_OP_LT;
        break;
    case '>':
        _type = op[1] ? node_OP_GE : node_OP_GT;
        break;
    default:
        assert(0);
        break;
    }
}

bool Comparison::eval(sVariant &result, Context &ctx) const
{
    sVariant lval, rval;
    CHECK_EVAL(_lhs, lval, 0);
    CHECK_EVAL(_rhs, rval, 0);

    switch (_type) {
    case node_OP_LT:
        result.setInt(ctx.evalLess(lval, rval));
        break;
    case node_OP_LE:
        result.setInt(ctx.evalLessOrEqual(lval, rval));
        break;
    case node_OP_GT:
        result.setInt(ctx.evalGreater(lval, rval));
        break;
    case node_OP_GE:
        result.setInt(ctx.evalGreaterOrEqual(lval, rval));
        break;
    case node_OP_CMP:
        if (ctx.evalEquality(lval, rval))
            result.setInt(0);
        else
            result.setInt(ctx.evalLess(lval, rval) ? -1 : 1 );
        break;
    default:
        assert(0);
        break;
    }
    return true;
}

bool Has::eval(sVariant &result, Context &ctx) const
{
    sVariant lval;
    sVec<sVariant> rvals;
    idx dim = 1;
    CHECK_EVAL(_lhs, lval, 0);
    Junction *junc_node = dynamic_cast<Junction*>(_rhs);

    // for has operation, rhs can be a junction
    if (junc_node) {
        dim = junc_node->dim();
        rvals.resize(dim);
        for (idx i=0; i<dim; i++)
            CHECK_EVAL(junc_node->getElement(i), rvals[i], 0);
    } else {
        rvals.resize(1);
        CHECK_EVAL(_rhs, rvals[0], 0);
    }

    eEvalStatus code = ctx.evalHas(result, lval, rvals.ptr(), dim);
    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }
    return true;
}

Match::Match(Node *lhs, char op, regex_t *re, const char * re_string, idx line, idx col): Unary(lhs, line, col), _pmatch(sMex::fExactSize)
{
    memcpy(&_re, re, sizeof(regex_t));
    _type = (op == '=') ? node_OP_MATCH : node_OP_NMATCH;
    if (re_string) {
        idx nmatch = 0;
        // estimate number of substitutions
        for (idx i=0; re_string[i]; i++) {
            if (re_string[i] == '(' && (i == 0 || re_string[i-1] != '\\')) {
                nmatch++;
            }
        }
        if (nmatch) {
            _pmatch.resize(nmatch + 1);
        }
    }
}

Match::~Match()
{
    regfree(&_re);
}

bool Match::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(_arg, val, 0);

    eEvalStatus code = _type == node_OP_MATCH ? ctx.evalMatch(result, val, &_re, _pmatch.dim(), _pmatch.ptr()) : ctx.evalNMatch(result, val, &_re);
    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "%s failed", getTypeName());
        return false;
    }
    return true;
}

bool BinaryLogic::eval(sVariant &result, Context &ctx) const
{
    sVariant lval, rval;
    CHECK_EVAL(_lhs, lval, 0);

    // short-circuit semantics: don't evaluate rhs unless needed
    switch(this->_type) {
    case node_OP_AND:
        if (lval.asBool()) {
            CHECK_EVAL(_rhs, rval, 0);
            result.setInt(rval.asBool());
        } else
            result.setInt(0);
        break;
    case node_OP_OR:
        if (lval.asBool())
            result.setInt(1);
        else {
            CHECK_EVAL(_rhs, rval, 0);
            result.setInt(rval.asBool());
        }
        break;
    default:
        assert(0);
        break;
    }

    return true;
}

bool Not::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(_arg, val, 0);

    result.setInt(!val.asBool());
    return true;
}

TernaryConditional::~TernaryConditional()
{
    delete _condition;
    delete _ifnode;
    delete _elsenode;
}

bool TernaryConditional::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    CHECK_EVAL(_condition, val, 0);
    if (val.asBool()) {
        CHECK_EVAL(_ifnode, result, 0);
    } else {
        CHECK_EVAL(_elsenode, result, 0);
    }
    return true;
}

void TernaryConditional::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tCondition: %s @ %p; if-value: %s @ %p; else-value: %s @ %p\n", _condition->getTypeName(), _condition, _ifnode->getTypeName(), _ifnode, _elsenode->getTypeName(), _elsenode);
    _condition->print(s);
    _ifnode->print(s);
    _elsenode->print(s);
}

Property::~Property()
{
    delete _topic;
}

bool Property::eval(sVariant &result, Context &ctx) const
{
    eEvalStatus code;
    if (_topic) {
        sVariant topic_val;
        CHECK_EVAL(_topic, topic_val, 0);
        code = ctx.evalGetProperty(result, topic_val, _name.ptr());
    } else {
        code = ctx.evalGetProperty(result, _name.ptr());
    }

    if (code != EVAL_SUCCESS) {
        ctx.setError(getLocation(), code, "accessing '%s' property failed", _name.ptr());
        return false;
    }
    return true;
}

void Property::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tName == \"%s\"; topic %s @ %s\n", _name.ptr(), _topic ? _topic->getTypeName() : NULL, getName());
    if (_topic)
        _topic->print(s);
}

FunctionCall::~FunctionCall()
{
    delete _verb;
}

bool FunctionCall::eval(sVariant &result, Context &ctx, Node *topic, const char *callableType) const
{
    sVariant verbVal, topicVal;
    sVec<sVariant> argVals;
    argVals.resize(_elts.dim());

    if (topic)
        CHECK_EVAL(topic, topicVal, 0);

    CHECK_EVAL(_verb, verbVal, 0);
    for (idx i=0; i<_elts.dim(); i++)
        CHECK_EVAL(_elts[i], argVals[i], 0);

    CallableWrapper *cw = dynamic_cast<CallableWrapper*>(verbVal.asData());
    if (!cw) {
        ctx.setError(getLocation(), EVAL_TYPE_ERROR, "a %s cannot  be called like a %s", verbVal.getTypeName(), callableType);
        return false;
    }

    return cw->call(result, ctx, topic ? &topicVal : NULL, argVals.ptr(), argVals.dim());
}

bool FunctionCall::eval(sVariant &result, Context &ctx) const
{
    return eval(result, ctx, NULL, "function");
}

void FunctionCall::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tVerb == %s @ %p; arguments (%"DEC"): ", _verb->getTypeName(), _verb, _elts.dim());
     for (idx i=0; i<_elts.dim(); i++) {
        s.printf(" %s @ %p;", _elts[i]->getTypeName(), _elts[i]);
    }
    s.printf("\n");
    _verb->print(s);
    for (idx i=0; i<_elts.dim(); i++) {
        _elts[i]->print(s);
    }
}

MethodCall::MethodCall(Node *topic, Node *verb, idx line, idx col): FunctionCall(verb, line, col), _topic(topic)
{
    _type = node_METHOD_CALL;
    // Empty topic means "this"
    if (!_topic)
        _topic = new Variable("this", line, col);
}

MethodCall::~MethodCall()
{
    delete _topic;
}

bool MethodCall::eval(sVariant &result, Context &ctx) const
{
    return FunctionCall::eval(result, ctx, _topic, "method");
}

void MethodCall::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tVerb == %s @ %p; topic == %s @ %p; arguments (%"DEC"): ", _verb->getTypeName(), _verb, _topic ? _topic->getTypeName() : NULL, _topic, _elts.dim());
     for (idx i=0; i<_elts.dim(); i++) {
        s.printf(" %s @ %p;", _elts[i]->getTypeName(), _elts[i]);
    }
    s.printf("\n");
    _verb->print(s);
    for (idx i=0; i<_elts.dim(); i++) {
        _elts[i]->print(s);
    }
}

bool FormatCall::eval(sVariant &result, Context &ctx) const
{
    return FunctionCall::eval(result, ctx, _topic, "format");
}

bool Block::eval(sVariant &result, Context &ctx) const
{
    bool ret = true;

    // a block introduces a new scope, evaluates its expressions
    // one after another, and returns the last one evaluated
    if (_addsScope)
        ctx.pushScope();

    for (idx i=0; i<_elts.dim(); i++) {
        if (!_elts[i]->eval(result, ctx)) {
            ret = false;
            goto CLEANUP;
        }

        idx c = checkCtxMode(_elts[i], ctx, _addsScope ? flag_POP_SCOPE : 0);
        if (c <= 0) {
            ret = !c;
            goto CLEANUP;
        }
    }

    if (_addsScope)
        ctx.popScope();

  CLEANUP:
    if (_isMain) {
        ctx.clearBreakContinue();
        ctx.clearReturn(false);
    }

    return ret;
}

bool DollarCall::eval(sVariant &result, Context &ctx) const
{
    bool ret = _name.ptr() ? ctx.evalGetDollarNameValue(result, _name.ptr()) : ctx.evalGetDollarNumValue(result, _num);

    if (!ret) {
        sStr e;
        if (_name.ptr())
            e.printf("${%s}", _name.ptr());
        else
            e.printf("$%"DEC, _num);

        ctx.setError(getLocation(), EVAL_VARIABLE_ERROR, "undefined expression %s", e.ptr());
        return false;
    }

    return ret;
}

void DollarCall::print(sStr &s) const
{
    if (_loc.print(s))
        s.printf(" : ");

    if (_name.ptr())
        s.printf("${%s} call @ %p\n", _name.ptr(), this);
    else
        s.printf("$%"DEC" call @ %p\n", _num, this);
}

bool DollarCall::isDollarCall(const char ** name, idx * num) const
{
    if (_name.ptr()) {
        *name = _name.ptr();
        *num = 0;
    } else {
        *name = 0;
        *num = _num;
    }
    return true;
}

bool UnbreakableBlock::eval(sVariant &result, Context &ctx) const
{
    bool ret = Block::eval(result, ctx);
    if (ctx.getMode() == Context::MODE_RETURN) {
        result = ctx.getReturnValue();
        ctx.clearReturn();
    }
    ctx.clearBreakContinue();
    return ret;
}

Lambda::Lambda(const char *arglist00, Block *block, idx line, idx col): Node(line, col)
{
    _block = block;
    _block->setAddsScope(false); // make sure we don't delete local variables before returning them

    const char *argname;
    for (argname = arglist00; argname && *argname; argname = sString::next00(argname)) {
        _arglist.add();
        _arglist[_arglist.dim()-1].printf("%s", argname);
    }

    _name.printf("function %p", this);
}

Lambda::~Lambda()
{
    delete _block;
}

// This returns a callable value; use call() to actually call execute the lambda
bool Lambda::eval(sVariant &result, Context &ctx) const
{
    CallableWrapper callable(this, ctx.getScope());
    result.setData(callable);
    return true;
}

bool Lambda::call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
{
    // assume that context sanity has been checked by the caller...
    if (nargs != _arglist.dim()) {
        ctx.setError(getLocation(), EVAL_BAD_ARGS_NUMBER, "expected %"DEC" arguments, not %"DEC, _arglist.dim(), nargs);
        return false;
    }

    ctx.pushLambda();
    if (topic && !ctx.addVarValue("this", *topic)) {
        ctx.setError(getLocation(), EVAL_READ_ONLY_ERROR, "'this' is read-only and cannot be modified");
        return false;
    }
    for (idx i=0; i<nargs; i++) {
        if (!ctx.setVarValue(_arglist[i].ptr(), args[i])) {
            ctx.setError(getLocation(), EVAL_READ_ONLY_ERROR, "%s is read-only and cannot be modified", _arglist[i].ptr());
            return false;
        }
    }

    sVariant blockResult;
    CHECK_EVAL(_block, blockResult, flag_POP_SCOPE|flag_IGNORE_RETURN);
    // did we explicitly call return?
    if (ctx.getMode() == Context::MODE_RETURN) {
        result = ctx.getReturnValue();
        ctx.clearReturn();
    } else
        result = blockResult;

    ctx.popScope();
    return true;
}

void Lambda::print(sStr &s) const
{
    Node::print(s);
    if (_name)
        s.printf("\tName == \"%s\"\n", getName());
    if (_arglist.dim()) {
        s.printf("\tParameters (%"DEC"): ", _arglist.dim());
        for (idx i=0; i<_arglist.dim(); i++)
            s.printf("%s%s", i ? ", " : "", _arglist[i].ptr());
        s.printf("\n");
    }
    s.printf("\tBlock : %s @ %p\n", _block->getTypeName(), _block);
    _block->print(s);
}

If::~If()
{
    delete _condition;
    delete _ifBlock;
    delete _elseBlock;
}

void If::setLastElse(Node *node)
{
    If *cur = this;
    while (If *next = dynamic_cast<If*>(cur->_elseBlock))
        cur = next;

    delete cur->_elseBlock;
    cur->_elseBlock = node;
}

bool If::eval(sVariant &result, Context &ctx) const
{
    sVariant val;

    ctx.pushScope();
    CHECK_EVAL(_condition, val, flag_POP_SCOPE);

    if (val.asBool())
        CHECK_EVAL(_ifBlock, result, flag_POP_SCOPE);
    else if (_elseBlock)
        CHECK_EVAL(_elseBlock, result, flag_POP_SCOPE);

    ctx.popScope();
    return true;
}

void If::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tCondition %s @ %p; if-block %s @ %p; else-block %s @ %p\n", _condition ? _condition->getTypeName() : NULL, _condition, _ifBlock ? _ifBlock->getTypeName() : NULL, _ifBlock, _elseBlock ? _elseBlock->getTypeName() : NULL, _elseBlock);
    if (_condition)
        _condition->print(s);
    if (_ifBlock)
        _ifBlock->print(s);
    if (_elseBlock)
        _elseBlock->print(s);
}

For::For(Node* init, Node* condition, Node* step, Block *block, idx line, idx col): Node(line,col)
{
    _init = init;
    if (dynamic_cast<Block*>(_init))
        dynamic_cast<Block*>(_init)->setAddsScope(false);

    _condition = condition;
    if (dynamic_cast<Block*>(_condition))
        dynamic_cast<Block*>(_condition)->setAddsScope(false);

    _step = step;
    if (dynamic_cast<Block*>(_step))
        dynamic_cast<Block*>(_step)->setAddsScope(false);

    _block = block;
    this->_type = node_FOR;
}

For::~For()
{
    delete _init;
    delete _condition;
    delete _step;
    delete _block;
}

bool For::eval(sVariant &result, Context &ctx) const
{
    ctx.pushLoop();

    if (_init)
        CHECK_RUN(_init, flag_POP_SCOPE);

    while(1) {
        sVariant val((idx)1);

        if (_condition)
            CHECK_EVAL(_condition, val, flag_POP_SCOPE);

        if (!val.asBool())
            break;

        if (_block)
            CHECK_RUN(_block, flag_POP_SCOPE|flag_IGNORE_BREAK_CONTINUE);

        switch (ctx.getMode()) {
        case Context::MODE_BREAK:
            ctx.clearBreakContinue();
            ctx.popScope();
            return true;

        case Context::MODE_CONTINUE:
            ctx.clearBreakContinue();
            break;
        }

        if (_step)
            CHECK_RUN(_step, flag_POP_SCOPE);
    }

    ctx.popScope();
    return true;
}

void For::print(sStr &s) const
{
    Node::print(s);
    s.printf("\tInit %s @ %p; condition %s @ %p; step %s @ %p; block %s @ %p\n", _init ? _init->getTypeName() : NULL, _init, _condition ? _condition->getTypeName() : NULL, _condition, _step ? _step->getTypeName() : NULL, _step, _block ? _block->getTypeName() : NULL, _block);
    if (_init)
        _init->print(s);
    if (_condition)
        _condition->print(s);
    if (_step)
        _step->print(s);
    if (_block)
        _block->print(s);
}

bool Return::eval(sVariant &result, Context &ctx) const
{
    if (!_arg) {
        result.setNull();
        return true;
    }

    CHECK_EVAL(_arg, result, 0);
    ctx.setReturn(result);
    return true;
}

bool Break::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    ctx.setBreak();
    return true;
}

bool Continue::eval(sVariant &result, Context &ctx) const
{
    sVariant val;
    ctx.setContinue();
    return true;
}
