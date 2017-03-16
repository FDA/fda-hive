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
#ifndef sLib_qlang_ast_h
#define sLib_qlang_ast_h

namespace slib {
    namespace qlang {
        //! Query language syntax tree namespace
        namespace ast {
            //! Base class for query language's abstract syntax tree nodes
            class Node {
            public:
                enum eType {
                    // Warning: keep in sync with NodeNames in ast.cpp
                    node_ERROR = 0,
                    node_NULL_LITERAL,
                    node_INT_LITERAL,
                    node_REAL_LITERAL,
                    node_STRING_LITERAL,
                    node_LIST_LITERAL,
                    node_DIC_LITERAL,
                    node_VARIANT_LITERAL,
                    node_VARIABLE,
                    node_JUNCTION,
                    node_CAST,
                    node_OP_ASSIGN,
                    node_OP_INCREMENT,
                    node_OP_DECREMENT,
                    node_OP_PLUS,
                    node_OP_MINUS,
                    node_OP_MULTIPLY,
                    node_OP_DIVIDE,
                    node_OP_REMAINDER,
                    node_OP_PLUS_INPLACE,
                    node_OP_MINUS_INPLACE,
                    node_OP_MULTIPLY_INPLACE,
                    node_OP_DIVIDE_INPLACE,
                    node_OP_REMAINDER_INPLACE,
                    node_OP_U_PLUS,
                    node_OP_U_MINUS,
                    node_OP_SUBSCRIPT,
                    node_OP_SLICE,
                    node_OP_EQ,
                    node_OP_NE,
                    node_OP_GT,
                    node_OP_GE,
                    node_OP_LT,
                    node_OP_LE,
                    node_OP_CMP,
                    node_OP_HAS,
                    node_OP_MATCH,
                    node_OP_NMATCH,
                    node_OP_AND,
                    node_OP_OR,
                    node_OP_NOT,
                    node_OP_TERNARY,
                    node_PROPERTY,
                    node_FUNCTION_CALL,
                    node_FORMAT_CALL,
                    node_METHOD_CALL,
                    node_DOLLAR_CALL,
                    node_IF,
                    node_FOR,
                    node_WHILE,
                    node_BLOCK,
                    node_UNBREAKABLE_BLOCK,
                    node_LAMBDA,
                    node_RETURN,
                    node_BREAK,
                    node_CONTINUE
                };
            protected:
                eType _type;
                SourceLocation _loc;

            public:
                Node(idx line=-1, idx col=-1): _type(node_ERROR), _loc(line, col) {}
                virtual ~Node() {}
                void setLocation(idx line, idx col) { _loc.set(line, col); }
                const SourceLocation& getLocation() const { return _loc; }
                inline virtual eType getType() const { return _type; }
                virtual const char* getTypeName() const;
                virtual void print(sStr &s) const;
                virtual bool run(Context &ctx) const;
                virtual bool eval(sVariant &result, Context &ctx) const;

                virtual bool isDollarCall(const char ** name, idx * num) const { return false; }
            };

            //! Abstract syntax tree node that contains one other node
            class Unary : public Node {
            protected:
                Node *_arg;
            public:
                Unary(Node *node, idx line=-1, idx col=-1): Node(line, col), _arg(node) {}
                void setArg(Node *arg) { delete _arg; _arg = arg; }
                Node* getArg() { return _arg; }
                virtual ~Unary();
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node that contains two other nodes
            class Binary : public Node {
            protected:
                Node *_lhs, *_rhs;
            public:
                Binary(Node *lhs, Node *rhs, idx line=-1, idx col=-1): Node(line, col), _lhs(lhs), _rhs(rhs) {}
                Node* getLhs() { return _lhs; }
                Node* getRhs() { return _rhs; }
                virtual ~Binary();
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node that contains a list of other nodes
            class Nary : public Node {
            protected:
                sVec<Node*> _elts;
            public:
                Nary(idx line=-1, idx col=-1): Node(line,col) {}
                virtual ~Nary();
                virtual void addElement(Node *node);
                virtual idx dim() const { return _elts.dim(); }
                virtual Node* getElement(idx i) { return _elts[i]; }
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for a scalar literal
            class ScalarLiteral : public Node {
            protected:
                sVariant _val;
            public:
                ScalarLiteral(idx line=-1, idx col=-1): Node(line,col) {}
                virtual ~ScalarLiteral() {}
                virtual sVariant& getValue() { return _val; }
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for a null literal
            class NullLiteral : public ScalarLiteral {
            public:
                NullLiteral(idx line=-1, idx col=-1): ScalarLiteral(line, col) { _val.setNull(); }
            };

            //! Abstract syntax tree node for an integer literal
            class IntLiteral : public ScalarLiteral {
            public:
                IntLiteral(idx i, idx line=-1, idx col=-1);
            };

            //! Abstract syntax tree node for a real literal
            class RealLiteral : public ScalarLiteral {
            public:
                RealLiteral(real r, idx line=-1, idx col=-1);
            };

            //! Abstract syntax tree node for a string literal
            class StringLiteral : public ScalarLiteral {
            public:
                StringLiteral(const char *s, idx line=-1, idx col=-1);
            };

            //! Abstract syntax tree node for a list literal: [1,2,"abc"]
            class ListLiteral : public Nary {
            public:
                ListLiteral(idx line=-1, idx col=-1): Nary(line, col) { _type = node_LIST_LITERAL; }
                virtual ~ListLiteral() {}
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for a dictionary literal: {"a":1, "b":2}
            class DicLiteral : public Nary {
            public:
                DicLiteral(idx line=-1, idx col=-1): Nary(line, col) { _type = node_DIC_LITERAL; }
                virtual ~DicLiteral() {}
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for a real literal
            class VariantLiteral : public ScalarLiteral {
            public:
                VariantLiteral(sVariant &val, idx line=-1, idx col=-1);
            };

            //! Abstract syntax tree node for a junction: 1|2|3
            class Junction : public ListLiteral {
            public:
                Junction(idx line=-1, idx col=-1): ListLiteral(line,col) { _type = node_JUNCTION; }
            };

            //! Abstract syntax tree node for casting to a boolean: "true" as bool
            class BoolCast : public Unary {
            public:
                BoolCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to an integer: [1,2,3] as int
            class IntCast : public Unary {
            public:
                IntCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to an unsigned integer
            class UIntCast : public Unary {
            public:
                UIntCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to an integer list: "1,2,3" as intlist
            class IntlistCast : public Unary {
            public:
                IntlistCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to a real
            class RealCast : public Unary {
            public:
                RealCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to a string
            class StringCast : public Unary {
            public:
                StringCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to an object ID
            class ObjCast : public Unary {
            public:
                ObjCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to a list of object IDs
            class ObjlistCast : public Unary {
            public:
                ObjlistCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to date-time
            class DateTimeCast : public Unary {
            public:
                DateTimeCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to date
            class DateCast : public Unary {
            public:
                DateCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for casting to time of day
            class TimeCast : public Unary {
            public:
                TimeCast(Node *arg, idx line=-1, idx col=-1): Unary(arg,line,col) { _type = node_CAST; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for accessing a variable or builtin by name
            class Variable : public Node {
            protected:
                sStr _var;
            public:
                Variable(const char *var, idx line=-1, idx col=-1): Node(line,col), _var(var) { _type = node_VARIABLE; }
                virtual bool eval(sVariant &result, Context &ctx) const;
                const char* getName() const { return _var.ptr(); }
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for an assignment expression
            class Assign : public Binary {
            public:
                Assign(Node *lhs, Node *rhs, idx line=-1, idx col=-1): Binary(lhs, rhs, line, col) { _type = node_OP_ASSIGN; }
                bool assign(Node *lhs, sVariant &val, Context &ctx) const;
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for ++x or --x
            class Precrement : public Assign {
            public:
                Precrement(Node *node, char op, idx line=-1, idx col=-1): Assign(node,NULL,line,col) { _type = (op == '+') ? node_OP_INCREMENT : node_OP_DECREMENT; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for x++ or x--
            class Postcrement : public Assign {
            public:
                Postcrement(Node *node, char op, idx line=-1, idx col=-1): Assign(node,NULL,line,col) { _type = (op == '+') ? node_OP_INCREMENT : node_OP_DECREMENT; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for a basic arithmetic expression
            class Arithmetic : public Binary {
            public:
                Arithmetic(Node *lhs, char op, Node *rhs, idx line=-1, idx col=-1);
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for an in-place arithmetic expression
            class ArithmeticInplace : public Arithmetic {
            protected:
                Assign assigner;
            public:
                ArithmeticInplace(Node *lhs, char op, Node *rhs, idx line=-1, idx col=-1);
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for a unary plus or minus, e.g. -x
            class UnaryPlusMinus : public Unary {
            public:
                UnaryPlusMinus(char op, Node *node, idx line=-1, idx col=-1);
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for subscripting a string, list, dictionary, or object, e.g. list[42];
            class Subscript : public Binary {
            public:
                Subscript(Node *lhs, Node *index, idx line=-1, idx col=-1): Binary(lhs, index, line, col) { _type = node_OP_SUBSCRIPT; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for taking a list slice, e.g. list[0:10]
            class Slice : public Node {
            protected:
                Node *_lhs, *_rhs1, *_rhs2;
            public:
                Slice(Node *list, Node *index1, Node *index2, idx line=-1, idx col=-1): Node(line,col), _lhs(list), _rhs1(index1), _rhs2(index2) { _type = node_OP_SLICE; }
                virtual ~Slice();
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for (in)equality expressions
            class Equality : public Binary {
            public:
                Equality(Node *lhs, char op, Node *rhs, idx line=-1, idx col=-1);
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for comparison expressions
            class Comparison : public Binary {
            public:
                Comparison(Node *lhs, const char *op, Node *rhs, idx line=-1, idx col=-1);
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for the "has" operator, e.g. [1,2,3] has 2
            class Has : public Binary {
            public:
                Has(Node *lhs, Node *rhs, idx line=-1, idx col=-1): Binary(lhs, rhs, line, col) { _type = node_OP_HAS; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for regular expression match and no-match operators
            class Match : public Unary {
            protected:
                regex_t _re;
                mutable sVec<regmatch_t> _pmatch;
            public:
                Match(Node *lhs, char op, regex_t *re, const char *re_string, idx line=-1, idx col=-1);
                virtual ~Match();
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for binary logic expressions (&&, ||)
            class BinaryLogic : public Binary {
            public:
                BinaryLogic(Node *lhs, char op, Node *rhs, idx line=-1, idx col=-1): Binary(lhs, rhs, line, col) { _type = (op == '&') ? node_OP_AND : node_OP_OR; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for the ! logical not operator
            class Not : public Unary {
            public:
                Not(Node *node, idx line=-1, idx col=-1): Unary(node, line, col) { _type = node_OP_NOT; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for the ?: ternary conditional
            class TernaryConditional : public Node {
            protected:
                Node *_condition, *_ifnode, *_elsenode;
            public:
                TernaryConditional(Node * condition, Node * ifnode, Node * elsenode, idx line=-1, idx col=-1): Node(line, col), _condition(condition), _ifnode(ifnode), _elsenode(elsenode) { _type = node_OP_TERNARY; }
                virtual ~TernaryConditional();
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for accessing an object's property by name
            class Property : public Node {
            protected:
                Node *_topic;
                sStr _name;
            public:
                Property(Node *topic, const char *name, idx line=-1, idx col=-1): Node(line, col), _topic(topic), _name(name) { _type = node_PROPERTY; }
                virtual ~Property();
                virtual void setTopic(Node* topic) { delete _topic; _topic = topic; }
                virtual Node* getTopic() const { return _topic; }
                void setName(const char *name) { _name.cut(0); _name.printf(name); }
                const char* getName() const { return _name.ptr(); }
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree for calling a function without a topic: max(1,2,3)
            class FunctionCall : public Nary {
            protected:
                Node* _verb;
                bool eval(sVariant &result, Context &ctx, Node *topic, const char *callableType) const;

            public:
                FunctionCall(Node *verb, idx line=-1, idx col=-1): Nary(line, col), _verb(verb) { _type = node_FUNCTION_CALL; }
                virtual ~FunctionCall();
                virtual Node* getVerb() const { return _verb; }
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree for calling a function on a topic: objectList.filter({.prop == 42})
            class MethodCall : public FunctionCall {
            protected:
                Node *_topic;
            public:
                MethodCall(Node *topic, Node *verb, idx line=-1, idx col=-1);
                virtual ~MethodCall();
                virtual void setTopic(Node* topic) { delete _topic; _topic = topic; }
                virtual Node* getTopic() const { return _topic; }
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree for calling a function on a topic using the "as" syntax: objectList as csv()
            class FormatCall : public MethodCall {
            public:
                FormatCall(Node *topic, Node *verb, idx line=-1, idx col=-1): MethodCall(topic, verb, line, col) { _type = node_FORMAT_CALL; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for $NUM or $NAME call
            class DollarCall : public Node {
            protected:
                idx _num;
                sStr _name;
            public:
                DollarCall(idx line=-1, idx col=-1): Node(line, col) { _type = node_DOLLAR_CALL; }
                void setNum(idx i) { _num = i; }
                void setName(const char *s) { _name.printf(0, "%s", s); }
                void borrowNameFrom(sStr &s) { _name.borrow(s.mex()); }
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;

                virtual bool isDollarCall(const char ** name, idx * num) const;
            };

            //! Abstract syntax tree node for a block of statements
            class Block : public Nary {
            protected:
                bool _addsScope;
                bool _isMain;
            public:
                Block(bool addsScope=true, bool isMain=false, idx line=-1, idx col=-1): Nary(line, col), _addsScope(addsScope), _isMain(isMain) { _type = node_BLOCK; }
                virtual bool eval(sVariant &result, Context &ctx) const;
                void setAddsScope(bool a=true) { _addsScope = a; }
                bool getAddsScope() { return _addsScope; }
                virtual bool isDollarCall(const char ** name, idx * num) const { return _elts.dim() == 1 ? _elts[0]->isDollarCall(name, num) : false; }
            };

            //! A block that catches break/continue/return statements
            class UnbreakableBlock : public Block {
            public:
                UnbreakableBlock(bool addsScope=true, bool isMain=false, idx line=-1, idx col=-1): Block(addsScope, isMain, line, col) { _type = node_UNBREAKABLE_BLOCK; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for a lambda expression or function declared in query text
            class Lambda : public Node, public Callable {
            protected:
                sVec<sStr> _arglist;
                Block *_block;
                sStr _name;
            public:
                Lambda(const char *arglist00, Block *block, idx line=-1, idx col=-1);
                virtual ~Lambda();
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual const char* getName() const { return _name.ptr(); }
                virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for an if expression
            class If : public Node {
            protected:
                Node* _condition;
                Node* _ifBlock;
                Node* _elseBlock;
            public:
                If(Node* condition, Node* ifBlock, Node *elseBlock, idx line=-1, idx col=-1): Node(line, col), _condition(condition), _ifBlock(ifBlock), _elseBlock(elseBlock) { _type = node_IF; }
                virtual ~If();
                // sets the last else-block in a chain of If-s
                virtual void setLastElse(Node *node);
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for a for loop
            class For : public Node {
            protected:
                Node* _init;
                Node* _condition;
                Node* _step;
                Block* _block;
            public:
                For(Node* init, Node* condition, Node* step, Block *block, idx line=-1, idx col=-1);
                virtual ~For();
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual void print(sStr &s) const;
            };

            //! Abstract syntax tree node for a while loop
            class While : public For {
            public:
                While(Node* condition, Block *block, idx line=-1, idx col=-1): For(NULL, condition, NULL, block, line, col) { _type = node_WHILE; }
            };

            //! Abstract syntax tree node for a return statement
            class Return : public Unary {
            public:
                Return(Node *arg, idx line=-1, idx col=-1): Unary(arg, line, col) { _type = node_RETURN; }
                virtual bool eval(sVariant &result, Context &ctx) const;
                virtual bool isDollarCall(const char ** name, idx * num) const { return _arg ? _arg->isDollarCall(name, num) : false; }
            };

            //! Abstract syntax tree node for a break statement
            class Break : public Node {
            public:
                Break(idx line=-1, idx col=-1): Node(line,col) { _type = node_BREAK; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };

            //! Abstract syntax tree node for a continue statement
            class Continue : public Node {
            public:
                Continue(idx line=-1, idx col=-1): Node(line,col) { _type = node_BREAK; }
                virtual bool eval(sVariant &result, Context &ctx) const;
            };
        };
    };
};

#endif
