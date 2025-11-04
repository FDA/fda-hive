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












#ifndef YY_YY_QLANG_BISON_HPP_INCLUDED
# define YY_YY_QLANG_BISON_HPP_INCLUDED
#line 20 "qlang-bison.yy"

    #include <regex.h>
    #include <slib/std/string.hpp>
    #include <qlang/parser.hpp>

    using namespace slib;
    using namespace slib::qlang;

    #ifndef yyscan_t
    typedef void* yyscan_t;
    #endif

#line 62 "qlang-bison.hpp"


# include <cstdlib>
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif
# include "location.hh"


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E)
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value)
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#ifndef YYDEBUG
# define YYDEBUG 1
#endif

namespace yy {
#line 191 "qlang-bison.hpp"




  class sQLangBison
  {
  public:
#ifndef YYSTYPE
    union semantic_type
    {
#line 48 "qlang-bison.yy"

    idx intVal;
    real realVal;
    sStr *strVal;
    ast::Node *node;
    ast::Block *block;
    sVec<ast::Node*> *nodes;

#line 213 "qlang-bison.hpp"

    };
#else
    typedef YYSTYPE semantic_type;
#endif
    typedef location location_type;

    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
    };

    struct token
    {
      enum token_kind_type
      {
        YYEMPTY = -2,
    END = 0,
    YYerror = 256,
    YYUNDEF = 257,
    BOOL_LITERAL = 258,
    INT_LITERAL = 259,
    REAL_LITERAL = 260,
    TMPL_STRING = 261,
    STRING_LITERAL = 262,
    REGEX_LITERAL = 263,
    NULL_LITERAL = 264,
    INCREMENT = 265,
    PLUS_INPLACE = 266,
    DECREMENT = 267,
    MINUS_INPLACE = 268,
    MULTIPLY_INPLACE = 269,
    DIVIDE_INPLACE = 270,
    REMAINDER_INPLACE = 271,
    EQ = 272,
    NE = 273,
    GE = 274,
    LE = 275,
    CMP = 276,
    MATCH = 277,
    NMATCH = 278,
    AND = 279,
    OR = 280,
    HAS = 281,
    IF = 282,
    ELSE = 283,
    FOR = 284,
    WHILE = 285,
    BREAK = 286,
    CONTINUE = 287,
    FUNCTION = 288,
    RETURN = 289,
    AS = 290,
    BOOL = 291,
    INT = 292,
    UINT = 293,
    INTLIST = 294,
    REAL = 295,
    STRING = 296,
    OBJ = 297,
    OBJLIST = 298,
    DATETIME = 299,
    DATE = 300,
    TIME = 301,
    NAME = 302,
    TMPL_CODE_START = 303,
    DOLLAR_NUM = 304,
    DOLLAR_NAME = 305
      };
      typedef token_kind_type yytokentype;
    };

    typedef token::yytokentype token_kind_type;

    typedef token_kind_type token_type;

    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 72,
        S_YYEMPTY = -2,
        S_YYEOF = 0,
        S_YYerror = 1,
        S_YYUNDEF = 2,
        S_3_ = 3,
        S_4_ = 4,
        S_5_ = 5,
        S_6_ = 6,
        S_7_ = 7,
        S_8_ = 8,
        S_9_ = 9,
        S_10_ = 10,
        S_11_ = 11,
        S_12_ = 12,
        S_13_ = 13,
        S_14_ = 14,
        S_BOOL_LITERAL = 15,
        S_INT_LITERAL = 16,
        S_REAL_LITERAL = 17,
        S_TMPL_STRING = 18,
        S_STRING_LITERAL = 19,
        S_REGEX_LITERAL = 20,
        S_NULL_LITERAL = 21,
        S_22_ = 22,
        S_23_ = 23,
        S_INCREMENT = 24,
        S_PLUS_INPLACE = 25,
        S_26_ = 26,
        S_DECREMENT = 27,
        S_MINUS_INPLACE = 28,
        S_29_ = 29,
        S_MULTIPLY_INPLACE = 30,
        S_31_ = 31,
        S_DIVIDE_INPLACE = 32,
        S_33_ = 33,
        S_REMAINDER_INPLACE = 34,
        S_EQ = 35,
        S_NE = 36,
        S_37_ = 37,
        S_38_ = 38,
        S_GE = 39,
        S_LE = 40,
        S_CMP = 41,
        S_MATCH = 42,
        S_NMATCH = 43,
        S_AND = 44,
        S_OR = 45,
        S_46_ = 46,
        S_HAS = 47,
        S_IF = 48,
        S_ELSE = 49,
        S_FOR = 50,
        S_WHILE = 51,
        S_BREAK = 52,
        S_CONTINUE = 53,
        S_FUNCTION = 54,
        S_RETURN = 55,
        S_AS = 56,
        S_BOOL = 57,
        S_INT = 58,
        S_UINT = 59,
        S_INTLIST = 60,
        S_REAL = 61,
        S_STRING = 62,
        S_OBJ = 63,
        S_OBJLIST = 64,
        S_DATETIME = 65,
        S_DATE = 66,
        S_TIME = 67,
        S_NAME = 68,
        S_TMPL_CODE_START = 69,
        S_DOLLAR_NUM = 70,
        S_DOLLAR_NAME = 71,
        S_YYACCEPT = 72,
        S_input = 73,
        S_statements = 74,
        S_non_return_statements = 75,
        S_statements_with_return = 76,
        S_statement_block = 77,
        S_lambda_block = 78,
        S_namelist = 79,
        S_lambda = 80,
        S_statement = 81,
        S_non_return_statement = 82,
        S_if_statement_start = 83,
        S_if_statement = 84,
        S_for_statement = 85,
        S_while_statement = 86,
        S_return_statement = 87,
        S_break_statement = 88,
        S_continue_statement = 89,
        S_arglist = 90,
        S_optional_arglist = 91,
        S_expression = 92,
        S_assignment_target = 93,
        S_assignment_source = 94,
        S_assignment = 95,
        S_basic_expression = 96,
        S_impure_expression = 97,
        S_postfix_expression = 98,
        S_slice = 99,
        S_subscript = 100,
        S_postcrement = 101,
        S_function_call = 102,
        S_property_call = 103,
        S_method_call = 104,
        S_dollar_call = 105,
        S_unary_expression = 106,
        S_precrement = 107,
        S_cast = 108,
        S_multiplication_priority = 109,
        S_addition_priority = 110,
        S_comparable = 111,
        S_format_call = 112,
        S_conjunction_priority = 113,
        S_disjunction_priority = 114,
        S_ternary_priority = 115,
        S_comparison_priority = 116,
        S_equality_priority = 117,
        S_non_assignment_expression = 118,
        S_junction = 119,
        S_literal = 120,
        S_scalar_literal = 121,
        S_list_literal = 122,
        S_kv_key = 123,
        S_kv_pairs = 124,
        S_dic_literal = 125,
        S_variable = 126,
        S_template = 127,
        S_template_expression = 128
      };
    };

    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    template <typename Base>
    struct basic_symbol : Base
    {
      typedef Base super_type;

      basic_symbol ()
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value (std::move (that.value))
        , location (std::move (that.location))
      {}
#endif

      basic_symbol (const basic_symbol& that);
      basic_symbol (typename Base::kind_type t,
                    YY_MOVE_REF (location_type) l);

      basic_symbol (typename Base::kind_type t,
                    YY_RVREF (semantic_type) v,
                    YY_RVREF (location_type) l);

      ~basic_symbol ()
      {
        clear ();
      }

      void clear ()
      {
        Base::clear ();
      }

      std::string name () const YY_NOEXCEPT
      {
        return sQLangBison::symbol_name (this->kind ());
      }

      symbol_kind_type type_get () const YY_NOEXCEPT;

      bool empty () const YY_NOEXCEPT;

      void move (basic_symbol& s);

      semantic_type value;

      location_type location;

    private:
#if YY_CPLUSPLUS < 201103L
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    struct by_kind
    {
      by_kind ();

#if 201103L <= YY_CPLUSPLUS
      by_kind (by_kind&& that);
#endif

      by_kind (const by_kind& that);

      typedef token_kind_type kind_type;

      by_kind (kind_type t);

      void clear ();

      void move (by_kind& that);

      symbol_kind_type kind () const YY_NOEXCEPT;

      symbol_kind_type type_get () const YY_NOEXCEPT;

      symbol_kind_type kind_;
    };

    typedef by_kind by_type;

    struct symbol_type : basic_symbol<by_kind>
    {};

    sQLangBison (slib::qlang::Parser& parser_driver_yyarg, yyscan_t yyscanner_yyarg);
    virtual ~sQLangBison ();

#if 201103L <= YY_CPLUSPLUS
    sQLangBison (const sQLangBison&) = delete;
    sQLangBison& operator= (const sQLangBison&) = delete;
#endif

    int operator() ();

    virtual int parse ();

#if YYDEBUG
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    void set_debug_stream (std::ostream &);

    typedef int debug_level_type;
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    void set_debug_level (debug_level_type l);
#endif

    virtual void error (const location_type& loc, const std::string& msg);

    void error (const syntax_error& err);

    static std::string symbol_name (symbol_kind_type yysymbol);



    class context
    {
    public:
      context (const sQLangBison& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const { return yyla_; }
      symbol_kind_type token () const { return yyla_.kind (); }
      const location_type& location () const { return yyla_.location; }

      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const sQLangBison& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    sQLangBison (const sQLangBison&);
    sQLangBison& operator= (const sQLangBison&);
#endif


    typedef short state_type;

    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    virtual std::string yysyntax_error_ (const context& yyctx) const;
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    static bool yy_pact_value_is_default_ (int yyvalue);

    static bool yy_table_value_is_error_ (int yyvalue);

    static const short yypact_ninf_;
    static const short yytable_ninf_;

    static symbol_kind_type yytranslate_ (int t);

    static std::string yytnamerr_ (const char *yystr);

    static const char* const yytname_[];


    static const short yypact_[];

    static const unsigned char yydefact_[];

    static const short yypgoto_[];

    static const short yydefgoto_[];

    static const short yytable_[];

    static const short yycheck_[];

    static const unsigned char yystos_[];

    static const unsigned char yyr1_[];

    static const signed char yyr2_[];


#if YYDEBUG
    static const short yyrline_[];
    virtual void yy_reduce_print_ (int r) const;
    virtual void yy_stack_print_ () const;

    int yydebug_;
    std::ostream* yycdebug_;

    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    struct by_state
    {
      by_state () YY_NOEXCEPT;

      typedef state_type kind_type;

      by_state (kind_type s) YY_NOEXCEPT;

      by_state (const by_state& that) YY_NOEXCEPT;

      void clear () YY_NOEXCEPT;

      void move (by_state& that);

      symbol_kind_type kind () const YY_NOEXCEPT;

      enum { empty_state = 0 };

      state_type state;
    };

    struct stack_symbol_type : basic_symbol<by_state>
    {
      typedef basic_symbol<by_state> super_type;
      stack_symbol_type ();
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      stack_symbol_type& operator= (stack_symbol_type& that);

      stack_symbol_type& operator= (const stack_symbol_type& that);
#endif
    };

    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t index_type;

      stack (size_type n = 200)
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      stack (const stack&) = delete;
      stack& operator= (const stack&) = delete;
#endif

      const T&
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      T&
      operator[] (index_type i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
      }

      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      class slice
      {
      public:
        slice (const stack& stack, index_type range)
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      stack (const stack&);
      stack& operator= (const stack&);
#endif
      S seq_;
    };


    typedef stack<stack_symbol_type> stack_type;

    stack_type yystack_;

    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    void yypop_ (int n = 1);

    enum
    {
      yylast_ = 974,
      yynnts_ = 57,
      yyfinal_ = 123
    };


    slib::qlang::Parser& parser_driver;
    yyscan_t yyscanner;

  };


}
#line 965 "qlang-bison.hpp"


#line 97 "qlang-bison.yy"

    #define YY_DECL \
        int \
        yylex (yy::sQLangBison::semantic_type* yylval_param, \
               yy::sQLangBison::location_type* yylloc,       \
               slib::qlang::Parser& parser_driver, \
               yyscan_t yyscanner)

    YY_DECL;

    #define ADD_ELEMENTS(where, pelts) \
        do { \
            for (idx i=0; i<(pelts)->dim(); i++) { \
                (where)->addElement((*(pelts))[i]); \
                (*(pelts))[i] = NULL; \
            } \
        } while(0)

#line 990 "qlang-bison.hpp"


#endif 