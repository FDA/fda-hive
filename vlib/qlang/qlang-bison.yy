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
/*
* To generate the parser code, run "bison qlang-bison.yy"
*/

%skeleton "lalr1.cc"
%defines "qlang-bison.hpp"
%output "qlang-bison.cpp"
%define parser_class_name "sQLangBison"

%code requires {
    #include <regex.h>
    #include <slib/std/string.hpp>
    #include <qlang/parser.hpp>

    using namespace slib;
    using namespace slib::qlang;

    #ifndef yyscan_t
    typedef void* yyscan_t;
    #endif
}

%parse-param { slib::qlang::Parser& parser_driver }
%parse-param { yyscan_t yyscanner }

%lex-param { slib::qlang::Parser& parser_driver }
%lex-param { yyscan_t yyscanner }

%locations
%initial-action
{
//    @$.begin.filename = @$.end.filename = parser_driver.getFilename();
}

%debug
%error-verbose

%union {
    idx intVal;
    real realVal;
    sStr *strVal;
    ast::Node *node;
    ast::Block *block;
    sVec<ast::Node*> *nodes;
}

%destructor { delete $$; } <strVal>
%destructor { delete $$; } <node>
%destructor { delete $$; } <block>
%destructor {
    for (idx i=0; i<$$->dim(); i++)
        delete (*$$)[i];
    delete $$;
} <nodes>

%printer { debug_stream() << $$; } <*>
%printer { debug_stream() << '"' << ($$ ? $$->ptr() : "(null)") << '"'; } <strVal>
%printer {
    if ($$) {
        sStr s;
        $$->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
} <node>
%printer {
    if ($$) {
        sStr s;
        $$->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
} <block>
%printer {
    if ($$) {
        sStr s;
        for (idx i=0; i<$$->dim(); i++)
            (*$$)[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
} <nodes>

%code provides {
    // Tell Flex the lexer prototype
    #define YY_DECL \
        int \
        yylex (yy::sQLangBison::semantic_type* yylval_param, \
               yy::sQLangBison::location_type* yylloc,       \
               slib::qlang::Parser& parser_driver, \
               yyscan_t yyscanner)

    // Define yylex() for the parser
    YY_DECL;

    #define ADD_ELEMENTS(where, pelts) \
        do { \
            for (idx i=0; i<(pelts)->dim(); i++) { \
                (where)->addElement((*(pelts))[i]); \
                (*(pelts))[i] = NULL; \
            } \
        } while(0)
}

%token  END         0               "end of program"

/* basic syntactic elements */
%token  '('
%token  ')'
%token  '['
%token  ']'
%token  '{'
%token  '}'
%token  '.'
%token  ','
%token  ':'
%token  ';'
%token  '|'
%token  '?'

/* scalar literals */
%token  <intVal>    INT_LITERAL     "int literal"
%token  <realVal>   REAL_LITERAL    "real literal"
%token  <strVal>    TMPL_STRING     "template literal substring"
%token  <strVal>    STRING_LITERAL  "string literal"
%token  <strVal>    REGEX_LITERAL   "regex literal"
%token              NULL_LITERAL    "null literal"

/* operators */
%token              '='
/* arithmetic */
%token              '+'
%token              INCREMENT       "'++'"
%token              PLUS_INPLACE    "'+='"
%token              '-'
%token              DECREMENT       "'--'"
%token              MINUS_INPLACE   "'-='"
%token              '*'
%token              MULTIPLY_INPLACE "'*='"
%token              '/'
%token              DIVIDE_INPLACE  "'/='"
%token              '%'
%token              REMAINDER_INPLACE "'%='"
/* comparison */
%token              EQ              "'=='"
%token              NE              "'!='"
%token              '>'
%token              '<'
%token              GE              "'>='"
%token              LE              "'<='"
%token              CMP             "'<=>'"
%token              MATCH           "'=~'"
%token              NMATCH          "'!~'"
/* boolean */
%token              AND             "'&&'"
%token              OR              "'||'"
%token              '!'
/* list access */
%token              HAS             "'has'"

/* keywords */
%token              IF              "'if'"
%token              ELSE            "'else'"
%token              FOR             "'for'"
%token              WHILE           "'while'"
%token              BREAK           "'break'"
%token              CONTINUE        "'continue'"
%token              FUNCTION        "'function'"
%token              RETURN          "'return'"
%token              AS              "'as'"
%token              BOOL            "'bool'"
%token              INT             "'int'"
%token              UINT            "'uint'"
%token              INTLIST         "'intlist'"
%token              REAL            "'real'"
%token              STRING          "'string'"
%token              OBJ             "'obj'"
%token              OBJLIST         "'objlist'"
%token              DATETIME        "'datetime'"
%token              DATE            "'date'"
%token              TIME            "'time'"

/* all else */
%token  <strVal>    NAME            "name"
%token              TMPL_CODE_START "$("
%token  <intVal>    DOLLAR_NUM      "$NUM"
%token  <strVal>    DOLLAR_NAME     "$NAME"

%type <block> input statement_block lambda_block
%type <node> statement return_statement non_return_statement
%type <node> if_statement_start if_statement for_statement while_statement break_statement continue_statement
%type <node> variable lambda
%type <node> expression assignment assignment_source assignment_target non_assignment_expression
%type <node> basic_expression impure_expression postfix_expression unary_expression multiplication_priority addition_priority comparable conjunction_priority disjunction_priority ternary_priority comparison_priority equality_priority
%type <node> precrement postcrement slice subscript
%type <node> cast function_call method_call property_call format_call dollar_call
%type <node> literal scalar_literal dic_literal list_literal junction
%type <node> template_expression kv_key
%type <nodes> statements non_return_statements statements_with_return arglist optional_arglist kv_pairs template
%type <strVal> namelist

%%
input
    : non_assignment_expression END
    {
        ast::Block *root = new ast::Block(false, true, @$.begin.line, @$.begin.column);
        ast::Return *ret = new ast::Return($1, @$.begin.line, @$.begin.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        $$ = NULL;
    }
    | statements_with_return END
    {
        ast::Block *root = new ast::Block(false, true, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS(root, $1);
        delete $1;
        parser_driver.setAstRoot(root);
        $$ = NULL;
    }
    | non_return_statements END
    {
        ast::Block *root = new ast::Block(false, true, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS(root, $1);
        delete $1;
        ast::Return * ret = new ast::Return(0, @$.end.line, @$.end.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        $$ = NULL;
    }
    | template END
    {
        ast::Block *root = new ast::Block(false, true, @$.begin.line, @$.begin.column);
        ast::FunctionCall *cat = new ast::FunctionCall(new ast::Variable("cat", @$.begin.line, @$.begin.column), @$.begin.line, @$.begin.column);
        ADD_ELEMENTS(cat, $1);
        ast::Return *ret = new ast::Return(cat, @$.begin.line, @$.begin.column);
        root->addElement(ret);
        delete $1;
        parser_driver.setAstRoot(root);
        $$ = NULL;
    }
    ;

statements
    : statement
    {
        $$ = new sVec<ast::Node*>;
        $$->vadd(1, $1);
    }
    | statements statement
    {
        $$ = $1;
        $$->vadd(1, $2);
    }
    ;

non_return_statements
    : non_return_statement
    {
        $$ = new sVec<ast::Node*>;
        $$->vadd(1, $1);
    }
    | non_return_statements non_return_statement
    {
        $$ = $1;
        $$->vadd(1, $2);
    }
    ;

statements_with_return
    : return_statement
    {
        $$ = new sVec<ast::Node*>;
        $$->vadd(1, $1);
    }
    | non_return_statements return_statement
    {
        $$ = $1;
        $$->vadd(1, $2);
    }
    ;

statement_block
    : '{' statements '}'
    {
        $$ = new ast::Block(true, false, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS($$, $2);
        delete $2;
    }
    ;

lambda_block
    : '{' expression '}'
    {
        $$ = new ast::Block(false, false, @$.begin.line, @$.begin.column);
        ast::Return *r = new ast::Return($2, @1.begin.line, @1.begin.column);
        $$->addElement(r);
    }
    | '{' statements_with_return '}'
    {
        $$ = new ast::Block(false, false, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS($$, $2);
        delete $2;
    }
    | '{' non_return_statements '}'
    {
        $$ = new ast::Block(false, false, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS($$, $2);
        delete $2;
        ast::Return * r = new ast::Return(0, @$.end.line, @$.end.column);
        $$->addElement(r);
    }
    ;

namelist
    : NAME
    {
        $$ = $1;
        $$->add0();
    }
    | namelist ',' NAME
    {
        $$ = $1;
        $$->add($3->ptr());
        delete $3;
    }
    ;

lambda
    : lambda_block
    {
        $$ = new ast::Lambda(NULL, $1, @$.begin.line, @$.begin.column);
    }
    | FUNCTION '(' namelist ')' lambda_block
    {
        $3->add0();
        $$ = new ast::Lambda($3->ptr(), $5, @$.begin.line, @$.begin.column);
        delete $3;
    }
    ;

statement
    : non_return_statement
    | return_statement
    ;

non_return_statement
    : if_statement
    | for_statement
    | while_statement
    | break_statement
    | continue_statement
    | impure_expression ';'
    {
        $$ = $1;
    }
    ;

if_statement_start
    : IF '(' expression ')' statement_block
    {
        $$ = new ast::If($3, $5, NULL, @$.begin.line, @$.begin.column);
    }
    | if_statement_start ELSE IF '(' expression ')' statement_block
    {
        $$ = $1;
        ast::If *chain = new ast::If($5, $7, NULL, @3.begin.line, @3.begin.column);
        dynamic_cast<ast::If*>($$)->setLastElse(chain);
    }
    ;

if_statement
    : if_statement_start
    | if_statement_start ELSE statement_block
    {
        $$ = $1;
        dynamic_cast<ast::If*>($$)->setLastElse($3);
    }
    ;

for_statement
    : FOR '(' optional_arglist ';' optional_arglist ';' optional_arglist ')' statement_block
    {
        ast::Block *init = new ast::Block(false, false, @3.begin.line, @3.begin.column);
        ADD_ELEMENTS(init, $3);
        ast::Block *cond = new ast::Block(false, false, @5.begin.line, @5.begin.column);
        ADD_ELEMENTS(cond, $5);
        ast::Block *step = new ast::Block(false, false, @7.begin.line, @7.begin.column);
        ADD_ELEMENTS(step, $7);
        delete $3;
        delete $5;
        delete $7;
        $$ = new ast::For(init, cond, step, $9, @$.begin.line, @$.begin.column);
    }
    ;

while_statement
    : WHILE '(' expression ')' statement_block
    {
        $$ = new ast::While($3, $5, @$.begin.line, @$.begin.column);
    }
    ;

return_statement
    : RETURN ';'
    {
        $$ = new ast::Return(0, @$.begin.line, @$.begin.column);
    }
    | RETURN non_assignment_expression ';'
    {
        $$ = new ast::Return($2, @$.begin.line, @$.begin.column);
    }
    ;

break_statement
    : BREAK ';'
    {
        $$ = new ast::Break(@$.begin.line, @$.begin.column);
    }
    ;

continue_statement
    : CONTINUE ';'
    {
        $$ = new ast::Continue(@$.begin.line, @$.begin.column);
    }
    ;

arglist
    : expression
    {
        $$ = new sVec<ast::Node*>;
        $$->vadd(1, $1);
    }
    | arglist ',' expression
    {
        $$ = $1;
        $$->vadd(1, $3);
    }
    ;

optional_arglist
    :
    {
        $$ = new sVec<ast::Node*>;
    }
    | arglist
    ;

expression
    : non_assignment_expression
    | assignment
    ;

assignment_target
    : variable
    | property_call
    | postfix_expression '[' expression ']'
    {
        $$ = new ast::Subscript($1, $3, @2.begin.line, @2.begin.column);
    }
    ;

assignment_source
    : assignment
    | non_assignment_expression
    ;

assignment
    : assignment_target '=' assignment_source
    {
        $$ = new ast::Assign($1, $3, @2.begin.line, @2.begin.column);
    }
    | assignment_target PLUS_INPLACE assignment_source
    {
        $$ = new ast::ArithmeticInplace($1, '+', $3, @2.begin.line, @2.begin.column);
    }
    | assignment_target MINUS_INPLACE assignment_source
    {
        $$ = new ast::ArithmeticInplace($1, '-', $3, @2.begin.line, @2.begin.column);
    }
    | assignment_target MULTIPLY_INPLACE assignment_source
    {
        $$ = new ast::ArithmeticInplace($1, '*', $3, @2.begin.line, @2.begin.column);
    }
    | assignment_target DIVIDE_INPLACE assignment_source
    {
        $$ = new ast::ArithmeticInplace($1, '/', $3, @2.begin.line, @2.begin.column);
    }
    | assignment_target REMAINDER_INPLACE assignment_source
    {
        $$ = new ast::ArithmeticInplace($1, '%', $3, @2.begin.line, @2.begin.column);
    }
    ;

basic_expression
    : literal
    | variable
    | lambda
    | '(' expression ')'
    {
        $$ = $2;
    }
    | dollar_call
    ;

impure_expression
    : postcrement
    | precrement
    | assignment
    | function_call
    | method_call
    ;

postfix_expression
    : basic_expression
    | slice
    | subscript
    | postcrement
    | property_call
    | method_call
    | function_call
    ;

slice
    : postfix_expression '[' expression ':' expression ']'
    {
        $$ = new ast::Slice($1, $3, $5, @2.begin.line, @2.begin.column);
    }
    ;

subscript
    : postfix_expression '[' expression ']'
    {
        $$ = new ast::Subscript($1, $3, @2.begin.line, @2.begin.column);
    }
    ;

postcrement
    : postfix_expression INCREMENT
    {
        $$ = new ast::Postcrement($1, '+', @2.begin.line, @2.begin.column);
    }
    | postfix_expression DECREMENT
    {
        $$ = new ast::Postcrement($1, '-', @2.begin.line, @2.begin.column);
    }
    ;

function_call
    : basic_expression '(' optional_arglist ')'
    {
        ast::FunctionCall *fcall = new ast::FunctionCall($1, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS(fcall, $3);
        $$ = fcall;
        delete $3;
    }
    ;

property_call
    : '.' NAME
    {
        $$ = new ast::Property(NULL, $2->ptr(), @2.begin.line, @2.begin.column);
        delete $2;
    }
    | postfix_expression '.' NAME
    {
        $$ = new ast::Property($1, $3->ptr(), @2.begin.line, @2.begin.column);
        delete $3;
    }
    ;

method_call
    : '.' basic_expression '(' optional_arglist ')'
    {
        ast::MethodCall *mcall = new ast::MethodCall(NULL, $2, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS(mcall, $4);
        $$ = mcall;
        delete $4;
    }
    | postfix_expression '.' basic_expression '(' optional_arglist ')'
    {
        ast::MethodCall *mcall = new ast::MethodCall($1, $3, @2.begin.line, @2.begin.column);
        ADD_ELEMENTS(mcall, $5);
        $$ = mcall;
        delete $5;
    }
    ;

dollar_call
    : DOLLAR_NUM
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error(@$, "$NUM calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall(@$.begin.line, @$.begin.column);
        dcall->setNum($1);
        $$ = dcall;
    }
    | DOLLAR_NAME
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error(@$, "$NAME calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall(@$.begin.line, @$.begin.column);
        dcall->borrowNameFrom(*$1);
        delete $1;
        $$ = dcall;
    }
    ;

unary_expression
    : postfix_expression
    | precrement
    | cast
    | '-' unary_expression
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>($2);
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->getValue() *= (idx)(-1);
            scalar->setLocation(@$.begin.line, @$.begin.column);
            $$ = scalar;
        } else {
            $$ = new ast::UnaryPlusMinus('-', $2, @$.begin.line, @$.begin.column);
        }
    }
    | '+' unary_expression
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>($2);
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->setLocation(@$.begin.line, @$.begin.column);
            $$ = scalar;
        } else {
            $$ = new ast::UnaryPlusMinus('+', $2, @$.begin.line, @$.begin.column);
        }
    }
    | '!' unary_expression
    {
        $$ = new ast::Not($2, @$.begin.line, @$.begin.column);
    }
    ;

precrement
    : INCREMENT unary_expression
    {
        $$ = new ast::Precrement($2, '+', @$.begin.line, @$.begin.column);
    }
    | DECREMENT unary_expression
    {
        $$ = new ast::Precrement($2, '-', @$.begin.line, @$.begin.column);
    }
    ;

cast
    : '(' BOOL ')' basic_expression
    {
        $$ = new ast::BoolCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' INT ')' basic_expression
    {
        $$ = new ast::IntCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' UINT ')' basic_expression
    {
        $$ = new ast::UIntCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' INTLIST ')' basic_expression
    {
        $$ = new ast::IntlistCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' REAL ')' basic_expression
    {
        $$ = new ast::RealCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' STRING ')' basic_expression
    {
        $$ = new ast::StringCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' OBJ ')' basic_expression
    {
        $$ = new ast::ObjCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' OBJLIST ')' basic_expression
    {
        $$ = new ast::ObjlistCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' DATETIME ')' basic_expression
    {
        $$ = new ast::DateTimeCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' DATE ')' basic_expression
    {
        $$ = new ast::DateCast($4, @$.begin.line, @$.begin.column);
    }
    | '(' TIME ')' basic_expression
    {
        $$ = new ast::TimeCast($4, @$.begin.line, @$.begin.column);
    }
    ;

multiplication_priority
    : unary_expression
    | multiplication_priority '*' unary_expression
    {
        $$ = new ast::Arithmetic($1, '*', $3, @2.begin.line, @2.begin.column);
    }
    | multiplication_priority '/' unary_expression
    {
        $$ = new ast::Arithmetic($1, '/', $3, @2.begin.line, @2.begin.column);
    }
    | multiplication_priority '%' unary_expression
    {
        $$ = new ast::Arithmetic($1, '%', $3, @2.begin.line, @2.begin.column);
    }
    ;

addition_priority
    : multiplication_priority
    | addition_priority '+' multiplication_priority
    {
        $$ = new ast::Arithmetic($1, '+', $3, @2.begin.line, @2.begin.column);
    }
    | addition_priority '-' multiplication_priority
    {
        $$ = new ast::Arithmetic($1, '-', $3, @2.begin.line, @2.begin.column);
    }
    ;

comparable
    : addition_priority
    | unary_expression HAS unary_expression
    {
        $$ = new ast::Has($1, $3, @2.begin.line, @2.begin.column);
    }
    | unary_expression HAS junction
    {
        $$ = new ast::Has($1, $3, @2.begin.line, @2.begin.column);
    }
    | unary_expression MATCH REGEX_LITERAL
    {
        regex_t re;
        const char * flag_string = sString::next00($3->ptr());
        int flags = REG_EXTENDED;
        if (flag_string && strchr(flag_string, 'i')) {
            flags |= REG_ICASE;
        }
        if (regcomp(&re, $3->ptr(), flags)) {
            error(@3, "Invalid regular expression");
            delete $3;
            YYERROR;
        }
        $$ = new ast::Match($1, '=', &re, $3->ptr(), @2.begin.line, @2.begin.column);
        delete $3;
    }
    | unary_expression NMATCH REGEX_LITERAL
    {
        regex_t re;
        const char * flag_string = sString::next00($3->ptr());
        int flags = REG_EXTENDED;
        if (flag_string && strchr(flag_string, 'i')) {
            flags |= REG_ICASE;
        }
        if (regcomp(&re, $3->ptr(), flags)) {
            error(@3, "Invalid regular expression");
            delete $3;
            YYERROR;
        }
        $$ = new ast::Match($1, '!', &re, $3->ptr(), @2.begin.line, @2.begin.column);
        delete $3;
    }
    | addition_priority AS BOOL
    {
        $$ = new ast::BoolCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS INT
    {
        $$ = new ast::IntCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS UINT
    {
        $$ = new ast::UIntCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS INTLIST
    {
        $$ = new ast::IntlistCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS REAL
    {
        $$ = new ast::RealCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS STRING
    {
        $$ = new ast::StringCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS OBJ
    {
        $$ = new ast::ObjCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS OBJLIST
    {
        $$ = new ast::ObjlistCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS DATETIME
    {
        $$ = new ast::DateTimeCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS DATE
    {
        $$ = new ast::DateCast($1, @2.begin.line, @2.begin.column);
    }
    | addition_priority AS TIME
    {
        $$ = new ast::TimeCast($1, @2.begin.line, @2.begin.column);
    }
    | format_call
    ;

format_call
    : addition_priority AS basic_expression '(' optional_arglist ')'
    {
        ast::FormatCall *fcall = new ast::FormatCall($1, $3, @3.begin.line, @3.begin.column);
        ADD_ELEMENTS(fcall, $5);
        $$ = fcall;
        delete $5;
    }
    ;

conjunction_priority
    : equality_priority
    | conjunction_priority AND equality_priority
    {
        $$ = new ast::BinaryLogic($1, '&', $3, @2.begin.line, @2.begin.column);
    }
    ;

disjunction_priority
    : conjunction_priority
    | disjunction_priority OR conjunction_priority
    {
        $$ = new ast::BinaryLogic($1, '|', $3, @2.begin.line, @2.begin.column);
    }
    ;

ternary_priority
    : disjunction_priority
    | disjunction_priority '?' disjunction_priority ':' ternary_priority
    {
        $$ = new ast::TernaryConditional($1, $3, $5, @2.begin.line, @2.begin.column);
    }
    ;

comparison_priority
    : comparable
    | comparison_priority '<' comparable
    {
        $$ = new ast::Comparison($1, "<", $3, @2.begin.line, @2.begin.column);
    }
    | comparison_priority '>' comparable
    {
        $$ = new ast::Comparison($1, ">", $3, @2.begin.line, @2.begin.column);
    }
    | comparison_priority LE comparable
    {
        $$ = new ast::Comparison($1, "<=", $3, @2.begin.line, @2.begin.column);
    }
    | comparison_priority GE comparable
    {
        $$ = new ast::Comparison($1, ">=", $3, @2.begin.line, @2.begin.column);
    }
    | comparison_priority CMP comparable
    {
        $$ = new ast::Comparison($1, "<=>", $3, @2.begin.line, @2.begin.column);
    }
    ;

equality_priority
    : comparison_priority
    | equality_priority EQ comparison_priority
    {
        $$ = new ast::Equality($1, '=', $3, @2.begin.line, @2.begin.column);
    }
    | equality_priority EQ junction
    {
        $$ = new ast::Equality($1, '=', $3, @2.begin.line, @2.begin.column);
    }
    | equality_priority NE comparison_priority
    {
        $$ = new ast::Equality($1, '!', $3, @2.begin.line, @2.begin.column);
    }
    | equality_priority NE junction
    {
        $$ = new ast::Equality($1, '!', $3, @2.begin.line, @2.begin.column);
    }
    ;

non_assignment_expression
    : ternary_priority
    ;

junction
    : unary_expression '|' unary_expression
    {
        ast::Junction *junc = new ast::Junction(@$.begin.line, @$.begin.column);
        junc->addElement($1);
        junc->addElement($3);
        $$ = junc;
    }
    | junction '|' unary_expression
    {
        $$ = $1;
        dynamic_cast<ast::Junction*>($$)->addElement($3);
    }
    ;

literal
    : scalar_literal
    | list_literal
    | dic_literal
    ;

scalar_literal
    : INT_LITERAL
    {
        $$ = new ast::IntLiteral($1, @$.begin.line, @$.begin.column);
    }
    | REAL_LITERAL
    {
        $$ = new ast::RealLiteral($1, @$.begin.line, @$.begin.column);
    }
    | STRING_LITERAL
    {
        $$ = new ast::StringLiteral($1->ptr(), @$.begin.line, @$.begin.column);
        delete $1;
    }
    | NULL_LITERAL
    {
        $$ = new ast::NullLiteral(@$.begin.line, @$.begin.column);
    }
    ;

list_literal
    : '[' optional_arglist ']'
    {
        ast::ListLiteral *llit = new ast::ListLiteral(@$.begin.line, @$.begin.column);
        ADD_ELEMENTS(llit, $2);
        $$ = llit;
        delete $2;
    }
    ;

kv_key
    : NAME
    {
        $$ = new ast::StringLiteral($1->ptr(), @$.begin.line, @$.begin.column);
        delete $1;
    }
    | STRING_LITERAL
    {
        $$ = new ast::StringLiteral($1->ptr(), @$.begin.line, @$.begin.column);
        delete $1;
    }
    ;

kv_pairs
    : kv_key ':' expression
    {
        $$ = new sVec<ast::Node*>;
        $$->vadd(2, $1, $3);
    }
    | kv_pairs ',' kv_key ':' expression
    {
        $$ = $1;
        $$->vadd(2, $3, $5);
    }
    ;

dic_literal
    : '{' kv_pairs '}'
    {
        ast::DicLiteral *dlit = new ast::DicLiteral(@$.begin.line, @$.begin.column);
        ADD_ELEMENTS(dlit, $2);
        $$ = dlit;
        delete $2;
    }
    ;

variable
    : NAME
    {
        $$ = new ast::Variable($1->ptr(), @$.begin.line, @$.begin.column);
        delete $1;
    }
    ;

template
    : template_expression
    {
        $$ = new sVec<ast::Node*>;
        $$->vadd(1, $1);
    }
    | template template_expression
    {
        $$ = $1;
        $$->vadd(1, $2);
    }
    ;

template_expression
    : TMPL_STRING
    {
        $$ = new ast::StringLiteral($1->ptr(), @$.begin.line, @$.begin.column);
        delete $1;
    }
    | TMPL_CODE_START expression ')'
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, @$.begin.line, @$.begin.column);
        block->addElement($2);
        $$ = block;
        parser_driver.yyPopStateUntilTemplate(); // switch scanner state back to template
    }
    | TMPL_CODE_START statements_with_return ')'
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, @$.begin.line, @$.begin.column);
        ADD_ELEMENTS(block, $2);
        delete $2;
        $$ = block;
        parser_driver.yyPopStateUntilTemplate(); // switch scanner state back to template
    }


%%
