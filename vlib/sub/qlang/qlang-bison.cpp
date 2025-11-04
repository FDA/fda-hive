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













#include "qlang-bison.hpp"




#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h>
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


#if YYDEBUG

# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else 
# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif 
#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 136 "qlang-bison.cpp"

  sQLangBison::sQLangBison (slib::qlang::Parser& parser_driver_yyarg, yyscan_t yyscanner_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      parser_driver (parser_driver_yyarg),
      yyscanner (yyscanner_yyarg)
  {}

  sQLangBison::~sQLangBison ()
  {}

  sQLangBison::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}


  template <typename Base>
  sQLangBison::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  template <typename Base>
  sQLangBison::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  sQLangBison::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}

  template <typename Base>
  sQLangBison::symbol_kind_type
  sQLangBison::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  sQLangBison::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  sQLangBison::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  sQLangBison::by_kind::by_kind ()
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  sQLangBison::by_kind::by_kind (by_kind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  sQLangBison::by_kind::by_kind (const by_kind& that)
    : kind_ (that.kind_)
  {}

  sQLangBison::by_kind::by_kind (token_kind_type t)
    : kind_ (yytranslate_ (t))
  {}

  void
  sQLangBison::by_kind::clear ()
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  sQLangBison::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  sQLangBison::symbol_kind_type
  sQLangBison::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  sQLangBison::symbol_kind_type
  sQLangBison::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  sQLangBison::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  sQLangBison::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  sQLangBison::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  sQLangBison::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  sQLangBison::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  sQLangBison::symbol_kind_type
  sQLangBison::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  sQLangBison::stack_symbol_type::stack_symbol_type ()
  {}

  sQLangBison::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    that.state = empty_state;
#endif
  }

  sQLangBison::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  sQLangBison::stack_symbol_type&
  sQLangBison::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  sQLangBison::stack_symbol_type&
  sQLangBison::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  sQLangBison::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    switch (yysym.kind ())
    {
      case symbol_kind::S_TMPL_STRING:
#line 57 "qlang-bison.yy"
                    { delete (yysym.value.strVal); }
#line 343 "qlang-bison.cpp"
        break;

      case symbol_kind::S_STRING_LITERAL:
#line 57 "qlang-bison.yy"
                    { delete (yysym.value.strVal); }
#line 349 "qlang-bison.cpp"
        break;

      case symbol_kind::S_REGEX_LITERAL:
#line 57 "qlang-bison.yy"
                    { delete (yysym.value.strVal); }
#line 355 "qlang-bison.cpp"
        break;

      case symbol_kind::S_NAME:
#line 57 "qlang-bison.yy"
                    { delete (yysym.value.strVal); }
#line 361 "qlang-bison.cpp"
        break;

      case symbol_kind::S_DOLLAR_NAME:
#line 57 "qlang-bison.yy"
                    { delete (yysym.value.strVal); }
#line 367 "qlang-bison.cpp"
        break;

      case symbol_kind::S_input:
#line 59 "qlang-bison.yy"
                    { delete (yysym.value.block); }
#line 373 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statements:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 383 "qlang-bison.cpp"
        break;

      case symbol_kind::S_non_return_statements:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 393 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statements_with_return:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 403 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statement_block:
#line 59 "qlang-bison.yy"
                    { delete (yysym.value.block); }
#line 409 "qlang-bison.cpp"
        break;

      case symbol_kind::S_lambda_block:
#line 59 "qlang-bison.yy"
                    { delete (yysym.value.block); }
#line 415 "qlang-bison.cpp"
        break;

      case symbol_kind::S_namelist:
#line 57 "qlang-bison.yy"
                    { delete (yysym.value.strVal); }
#line 421 "qlang-bison.cpp"
        break;

      case symbol_kind::S_lambda:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 427 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 433 "qlang-bison.cpp"
        break;

      case symbol_kind::S_non_return_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 439 "qlang-bison.cpp"
        break;

      case symbol_kind::S_if_statement_start:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 445 "qlang-bison.cpp"
        break;

      case symbol_kind::S_if_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 451 "qlang-bison.cpp"
        break;

      case symbol_kind::S_for_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 457 "qlang-bison.cpp"
        break;

      case symbol_kind::S_while_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 463 "qlang-bison.cpp"
        break;

      case symbol_kind::S_return_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 469 "qlang-bison.cpp"
        break;

      case symbol_kind::S_break_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 475 "qlang-bison.cpp"
        break;

      case symbol_kind::S_continue_statement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 481 "qlang-bison.cpp"
        break;

      case symbol_kind::S_arglist:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 491 "qlang-bison.cpp"
        break;

      case symbol_kind::S_optional_arglist:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 501 "qlang-bison.cpp"
        break;

      case symbol_kind::S_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 507 "qlang-bison.cpp"
        break;

      case symbol_kind::S_assignment_target:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 513 "qlang-bison.cpp"
        break;

      case symbol_kind::S_assignment_source:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 519 "qlang-bison.cpp"
        break;

      case symbol_kind::S_assignment:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 525 "qlang-bison.cpp"
        break;

      case symbol_kind::S_basic_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 531 "qlang-bison.cpp"
        break;

      case symbol_kind::S_impure_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 537 "qlang-bison.cpp"
        break;

      case symbol_kind::S_postfix_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 543 "qlang-bison.cpp"
        break;

      case symbol_kind::S_slice:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 549 "qlang-bison.cpp"
        break;

      case symbol_kind::S_subscript:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 555 "qlang-bison.cpp"
        break;

      case symbol_kind::S_postcrement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 561 "qlang-bison.cpp"
        break;

      case symbol_kind::S_function_call:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 567 "qlang-bison.cpp"
        break;

      case symbol_kind::S_property_call:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 573 "qlang-bison.cpp"
        break;

      case symbol_kind::S_method_call:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 579 "qlang-bison.cpp"
        break;

      case symbol_kind::S_dollar_call:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 585 "qlang-bison.cpp"
        break;

      case symbol_kind::S_unary_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 591 "qlang-bison.cpp"
        break;

      case symbol_kind::S_precrement:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 597 "qlang-bison.cpp"
        break;

      case symbol_kind::S_cast:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 603 "qlang-bison.cpp"
        break;

      case symbol_kind::S_multiplication_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 609 "qlang-bison.cpp"
        break;

      case symbol_kind::S_addition_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 615 "qlang-bison.cpp"
        break;

      case symbol_kind::S_comparable:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 621 "qlang-bison.cpp"
        break;

      case symbol_kind::S_format_call:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 627 "qlang-bison.cpp"
        break;

      case symbol_kind::S_conjunction_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 633 "qlang-bison.cpp"
        break;

      case symbol_kind::S_disjunction_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 639 "qlang-bison.cpp"
        break;

      case symbol_kind::S_ternary_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 645 "qlang-bison.cpp"
        break;

      case symbol_kind::S_comparison_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 651 "qlang-bison.cpp"
        break;

      case symbol_kind::S_equality_priority:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 657 "qlang-bison.cpp"
        break;

      case symbol_kind::S_non_assignment_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 663 "qlang-bison.cpp"
        break;

      case symbol_kind::S_junction:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 669 "qlang-bison.cpp"
        break;

      case symbol_kind::S_literal:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 675 "qlang-bison.cpp"
        break;

      case symbol_kind::S_scalar_literal:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 681 "qlang-bison.cpp"
        break;

      case symbol_kind::S_list_literal:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 687 "qlang-bison.cpp"
        break;

      case symbol_kind::S_kv_key:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 693 "qlang-bison.cpp"
        break;

      case symbol_kind::S_kv_pairs:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 703 "qlang-bison.cpp"
        break;

      case symbol_kind::S_dic_literal:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 709 "qlang-bison.cpp"
        break;

      case symbol_kind::S_variable:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 715 "qlang-bison.cpp"
        break;

      case symbol_kind::S_template:
#line 60 "qlang-bison.yy"
                    {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 725 "qlang-bison.cpp"
        break;

      case symbol_kind::S_template_expression:
#line 58 "qlang-bison.yy"
                    { delete (yysym.value.node); }
#line 731 "qlang-bison.cpp"
        break;

      default:
        break;
    }
  }

#if YYDEBUG
  template <typename Base>
  void
  sQLangBison::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        switch (yykind)
    {
      case symbol_kind::S_BOOL_LITERAL:
#line 66 "qlang-bison.yy"
                 { debug_stream() << (yysym.value.intVal); }
#line 759 "qlang-bison.cpp"
        break;

      case symbol_kind::S_INT_LITERAL:
#line 66 "qlang-bison.yy"
                 { debug_stream() << (yysym.value.intVal); }
#line 765 "qlang-bison.cpp"
        break;

      case symbol_kind::S_REAL_LITERAL:
#line 66 "qlang-bison.yy"
                 { debug_stream() << (yysym.value.realVal); }
#line 771 "qlang-bison.cpp"
        break;

      case symbol_kind::S_TMPL_STRING:
#line 67 "qlang-bison.yy"
                 { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 777 "qlang-bison.cpp"
        break;

      case symbol_kind::S_STRING_LITERAL:
#line 67 "qlang-bison.yy"
                 { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 783 "qlang-bison.cpp"
        break;

      case symbol_kind::S_REGEX_LITERAL:
#line 67 "qlang-bison.yy"
                 { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 789 "qlang-bison.cpp"
        break;

      case symbol_kind::S_NAME:
#line 67 "qlang-bison.yy"
                 { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 795 "qlang-bison.cpp"
        break;

      case symbol_kind::S_DOLLAR_NUM:
#line 66 "qlang-bison.yy"
                 { debug_stream() << (yysym.value.intVal); }
#line 801 "qlang-bison.cpp"
        break;

      case symbol_kind::S_DOLLAR_NAME:
#line 67 "qlang-bison.yy"
                 { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 807 "qlang-bison.cpp"
        break;

      case symbol_kind::S_input:
#line 77 "qlang-bison.yy"
                 {
    if ((yysym.value.block)) {
        sStr s;
        (yysym.value.block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 821 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statements:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 836 "qlang-bison.cpp"
        break;

      case symbol_kind::S_non_return_statements:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 851 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statements_with_return:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 866 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statement_block:
#line 77 "qlang-bison.yy"
                 {
    if ((yysym.value.block)) {
        sStr s;
        (yysym.value.block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 880 "qlang-bison.cpp"
        break;

      case symbol_kind::S_lambda_block:
#line 77 "qlang-bison.yy"
                 {
    if ((yysym.value.block)) {
        sStr s;
        (yysym.value.block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 894 "qlang-bison.cpp"
        break;

      case symbol_kind::S_namelist:
#line 67 "qlang-bison.yy"
                 { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 900 "qlang-bison.cpp"
        break;

      case symbol_kind::S_lambda:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 914 "qlang-bison.cpp"
        break;

      case symbol_kind::S_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 928 "qlang-bison.cpp"
        break;

      case symbol_kind::S_non_return_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 942 "qlang-bison.cpp"
        break;

      case symbol_kind::S_if_statement_start:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 956 "qlang-bison.cpp"
        break;

      case symbol_kind::S_if_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 970 "qlang-bison.cpp"
        break;

      case symbol_kind::S_for_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 984 "qlang-bison.cpp"
        break;

      case symbol_kind::S_while_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 998 "qlang-bison.cpp"
        break;

      case symbol_kind::S_return_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1012 "qlang-bison.cpp"
        break;

      case symbol_kind::S_break_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1026 "qlang-bison.cpp"
        break;

      case symbol_kind::S_continue_statement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1040 "qlang-bison.cpp"
        break;

      case symbol_kind::S_arglist:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1055 "qlang-bison.cpp"
        break;

      case symbol_kind::S_optional_arglist:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1070 "qlang-bison.cpp"
        break;

      case symbol_kind::S_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1084 "qlang-bison.cpp"
        break;

      case symbol_kind::S_assignment_target:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1098 "qlang-bison.cpp"
        break;

      case symbol_kind::S_assignment_source:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1112 "qlang-bison.cpp"
        break;

      case symbol_kind::S_assignment:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1126 "qlang-bison.cpp"
        break;

      case symbol_kind::S_basic_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1140 "qlang-bison.cpp"
        break;

      case symbol_kind::S_impure_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1154 "qlang-bison.cpp"
        break;

      case symbol_kind::S_postfix_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1168 "qlang-bison.cpp"
        break;

      case symbol_kind::S_slice:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1182 "qlang-bison.cpp"
        break;

      case symbol_kind::S_subscript:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1196 "qlang-bison.cpp"
        break;

      case symbol_kind::S_postcrement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1210 "qlang-bison.cpp"
        break;

      case symbol_kind::S_function_call:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1224 "qlang-bison.cpp"
        break;

      case symbol_kind::S_property_call:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1238 "qlang-bison.cpp"
        break;

      case symbol_kind::S_method_call:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1252 "qlang-bison.cpp"
        break;

      case symbol_kind::S_dollar_call:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1266 "qlang-bison.cpp"
        break;

      case symbol_kind::S_unary_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1280 "qlang-bison.cpp"
        break;

      case symbol_kind::S_precrement:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1294 "qlang-bison.cpp"
        break;

      case symbol_kind::S_cast:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1308 "qlang-bison.cpp"
        break;

      case symbol_kind::S_multiplication_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1322 "qlang-bison.cpp"
        break;

      case symbol_kind::S_addition_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1336 "qlang-bison.cpp"
        break;

      case symbol_kind::S_comparable:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1350 "qlang-bison.cpp"
        break;

      case symbol_kind::S_format_call:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1364 "qlang-bison.cpp"
        break;

      case symbol_kind::S_conjunction_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1378 "qlang-bison.cpp"
        break;

      case symbol_kind::S_disjunction_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1392 "qlang-bison.cpp"
        break;

      case symbol_kind::S_ternary_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1406 "qlang-bison.cpp"
        break;

      case symbol_kind::S_comparison_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1420 "qlang-bison.cpp"
        break;

      case symbol_kind::S_equality_priority:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1434 "qlang-bison.cpp"
        break;

      case symbol_kind::S_non_assignment_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1448 "qlang-bison.cpp"
        break;

      case symbol_kind::S_junction:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1462 "qlang-bison.cpp"
        break;

      case symbol_kind::S_literal:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1476 "qlang-bison.cpp"
        break;

      case symbol_kind::S_scalar_literal:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1490 "qlang-bison.cpp"
        break;

      case symbol_kind::S_list_literal:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1504 "qlang-bison.cpp"
        break;

      case symbol_kind::S_kv_key:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1518 "qlang-bison.cpp"
        break;

      case symbol_kind::S_kv_pairs:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1533 "qlang-bison.cpp"
        break;

      case symbol_kind::S_dic_literal:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1547 "qlang-bison.cpp"
        break;

      case symbol_kind::S_variable:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1561 "qlang-bison.cpp"
        break;

      case symbol_kind::S_template:
#line 86 "qlang-bison.yy"
                 {
    if ((yysym.value.nodes)) {
        sStr s;
        for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
            (*(yysym.value.nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1576 "qlang-bison.cpp"
        break;

      case symbol_kind::S_template_expression:
#line 68 "qlang-bison.yy"
                 {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1590 "qlang-bison.cpp"
        break;

      default:
        break;
    }
        yyo << ')';
      }
  }
#endif

  void
  sQLangBison::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  sQLangBison::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  sQLangBison::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  sQLangBison::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  sQLangBison::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  sQLangBison::debug_level_type
  sQLangBison::debug_level () const
  {
    return yydebug_;
  }

  void
  sQLangBison::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif 
  sQLangBison::state_type
  sQLangBison::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  sQLangBison::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  sQLangBison::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  sQLangBison::operator() ()
  {
    return parse ();
  }

  int
  sQLangBison::parse ()
  {
    int yyn;
    int yylen = 0;

    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    symbol_type yyla;

    stack_symbol_type yyerror_range[3];

    int yyresult;

#if YY_EXCEPTIONS
    try
#endif       {
    YYCDEBUG << "Starting parse\n";


#line 41 "qlang-bison.yy"
{
}

#line 1714 "qlang-bison.cpp"


    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  yybackup:
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif           {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location, parser_driver, yyscanner));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif       }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    if (yyerrstatus_)
      --yyerrstatus_;

    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif         {
          switch (yyn)
            {
  case 2:
#line 219 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ast::Return *ret = new ast::Return((yystack_[1].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 1858 "qlang-bison.cpp"
    break;

  case 3:
#line 227 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(root, (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 1870 "qlang-bison.cpp"
    break;

  case 4:
#line 235 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(root, (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        ast::Return * ret = new ast::Return(0, yylhs.location.end.line, yylhs.location.end.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 1884 "qlang-bison.cpp"
    break;

  case 5:
#line 245 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ast::FunctionCall *cat = new ast::FunctionCall(new ast::Variable("cat", yylhs.location.begin.line, yylhs.location.begin.column), yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(cat, (yystack_[1].value.nodes));
        ast::Return *ret = new ast::Return(cat, yylhs.location.begin.line, yylhs.location.begin.column);
        root->addElement(ret);
        delete (yystack_[1].value.nodes);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 1899 "qlang-bison.cpp"
    break;

  case 6:
#line 259 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 1908 "qlang-bison.cpp"
    break;

  case 7:
#line 264 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 1917 "qlang-bison.cpp"
    break;

  case 8:
#line 272 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 1926 "qlang-bison.cpp"
    break;

  case 9:
#line 277 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 1935 "qlang-bison.cpp"
    break;

  case 10:
#line 285 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 1944 "qlang-bison.cpp"
    break;

  case 11:
#line 290 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 1953 "qlang-bison.cpp"
    break;

  case 12:
#line 298 "qlang-bison.yy"
    {
        (yylhs.value.block) = new ast::Block(true, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS((yylhs.value.block), (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
    }
#line 1963 "qlang-bison.cpp"
    break;

  case 13:
#line 307 "qlang-bison.yy"
    {
        (yylhs.value.block) = new ast::Block(false, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ast::Return *r = new ast::Return((yystack_[1].value.node), yystack_[2].location.begin.line, yystack_[2].location.begin.column);
        (yylhs.value.block)->addElement(r);
    }
#line 1973 "qlang-bison.cpp"
    break;

  case 14:
#line 313 "qlang-bison.yy"
    {
        (yylhs.value.block) = new ast::Block(false, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS((yylhs.value.block), (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
    }
#line 1983 "qlang-bison.cpp"
    break;

  case 15:
#line 319 "qlang-bison.yy"
    {
        (yylhs.value.block) = new ast::Block(false, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS((yylhs.value.block), (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        ast::Return * r = new ast::Return(0, yylhs.location.end.line, yylhs.location.end.column);
        (yylhs.value.block)->addElement(r);
    }
#line 1995 "qlang-bison.cpp"
    break;

  case 16:
#line 330 "qlang-bison.yy"
    {
        (yylhs.value.strVal) = (yystack_[0].value.strVal);
        (yylhs.value.strVal)->add0();
    }
#line 2004 "qlang-bison.cpp"
    break;

  case 17:
#line 335 "qlang-bison.yy"
    {
        (yylhs.value.strVal) = (yystack_[2].value.strVal);
        (yylhs.value.strVal)->add((yystack_[0].value.strVal)->ptr());
        delete (yystack_[0].value.strVal);
    }
#line 2014 "qlang-bison.cpp"
    break;

  case 18:
#line 344 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Lambda(NULL, (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2022 "qlang-bison.cpp"
    break;

  case 19:
#line 348 "qlang-bison.yy"
    {
        (yystack_[2].value.strVal)->add0();
        (yylhs.value.node) = new ast::Lambda((yystack_[2].value.strVal)->ptr(), (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[2].value.strVal);
    }
#line 2032 "qlang-bison.cpp"
    break;

  case 20:
#line 356 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2038 "qlang-bison.cpp"
    break;

  case 21:
#line 357 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2044 "qlang-bison.cpp"
    break;

  case 22:
#line 361 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2050 "qlang-bison.cpp"
    break;

  case 23:
#line 362 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2056 "qlang-bison.cpp"
    break;

  case 24:
#line 363 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2062 "qlang-bison.cpp"
    break;

  case 25:
#line 364 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2068 "qlang-bison.cpp"
    break;

  case 26:
#line 365 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2074 "qlang-bison.cpp"
    break;

  case 27:
#line 367 "qlang-bison.yy"
    {
        (yylhs.value.node) = (yystack_[1].value.node);
    }
#line 2082 "qlang-bison.cpp"
    break;

  case 28:
#line 374 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::If((yystack_[2].value.node), (yystack_[0].value.block), NULL, yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2090 "qlang-bison.cpp"
    break;

  case 29:
#line 378 "qlang-bison.yy"
    {
        (yylhs.value.node) = (yystack_[6].value.node);
        ast::If *chain = new ast::If((yystack_[2].value.node), (yystack_[0].value.block), NULL, yystack_[4].location.begin.line, yystack_[4].location.begin.column);
        dynamic_cast<ast::If*>((yylhs.value.node))->setLastElse(chain);
    }
#line 2100 "qlang-bison.cpp"
    break;

  case 30:
#line 386 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2106 "qlang-bison.cpp"
    break;

  case 31:
#line 388 "qlang-bison.yy"
    {
        (yylhs.value.node) = (yystack_[2].value.node);
        dynamic_cast<ast::If*>((yylhs.value.node))->setLastElse((yystack_[0].value.block));
    }
#line 2115 "qlang-bison.cpp"
    break;

  case 32:
#line 396 "qlang-bison.yy"
    {
        ast::Block *init = new ast::Block(false, false, yystack_[6].location.begin.line, yystack_[6].location.begin.column);
        ADD_ELEMENTS(init, (yystack_[6].value.nodes));
        ast::Block *cond = new ast::Block(false, false, yystack_[4].location.begin.line, yystack_[4].location.begin.column);
        ADD_ELEMENTS(cond, (yystack_[4].value.nodes));
        ast::Block *step = new ast::Block(false, false, yystack_[2].location.begin.line, yystack_[2].location.begin.column);
        ADD_ELEMENTS(step, (yystack_[2].value.nodes));
        delete (yystack_[6].value.nodes);
        delete (yystack_[4].value.nodes);
        delete (yystack_[2].value.nodes);
        (yylhs.value.node) = new ast::For(init, cond, step, (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2132 "qlang-bison.cpp"
    break;

  case 33:
#line 412 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::While((yystack_[2].value.node), (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2140 "qlang-bison.cpp"
    break;

  case 34:
#line 419 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Return(0, yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2148 "qlang-bison.cpp"
    break;

  case 35:
#line 423 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Return((yystack_[1].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2156 "qlang-bison.cpp"
    break;

  case 36:
#line 430 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Break(yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2164 "qlang-bison.cpp"
    break;

  case 37:
#line 437 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Continue(yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2172 "qlang-bison.cpp"
    break;

  case 38:
#line 444 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2181 "qlang-bison.cpp"
    break;

  case 39:
#line 449 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = (yystack_[2].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2190 "qlang-bison.cpp"
    break;

  case 40:
#line 457 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
    }
#line 2198 "qlang-bison.cpp"
    break;

  case 41:
#line 460 "qlang-bison.yy"
      { (yylhs.value.nodes) = (yystack_[0].value.nodes); }
#line 2204 "qlang-bison.cpp"
    break;

  case 42:
#line 464 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2210 "qlang-bison.cpp"
    break;

  case 43:
#line 465 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2216 "qlang-bison.cpp"
    break;

  case 44:
#line 469 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2222 "qlang-bison.cpp"
    break;

  case 45:
#line 470 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2228 "qlang-bison.cpp"
    break;

  case 46:
#line 472 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Subscript((yystack_[3].value.node), (yystack_[1].value.node), yystack_[2].location.begin.line, yystack_[2].location.begin.column);
    }
#line 2236 "qlang-bison.cpp"
    break;

  case 47:
#line 478 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2242 "qlang-bison.cpp"
    break;

  case 48:
#line 479 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2248 "qlang-bison.cpp"
    break;

  case 49:
#line 484 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Assign((yystack_[2].value.node), (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2256 "qlang-bison.cpp"
    break;

  case 50:
#line 488 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '+', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2264 "qlang-bison.cpp"
    break;

  case 51:
#line 492 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '-', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2272 "qlang-bison.cpp"
    break;

  case 52:
#line 496 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '*', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2280 "qlang-bison.cpp"
    break;

  case 53:
#line 500 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '/', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2288 "qlang-bison.cpp"
    break;

  case 54:
#line 504 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '%', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2296 "qlang-bison.cpp"
    break;

  case 55:
#line 510 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2302 "qlang-bison.cpp"
    break;

  case 56:
#line 511 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2308 "qlang-bison.cpp"
    break;

  case 57:
#line 512 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2314 "qlang-bison.cpp"
    break;

  case 58:
#line 514 "qlang-bison.yy"
    {
        (yylhs.value.node) = (yystack_[1].value.node);
    }
#line 2322 "qlang-bison.cpp"
    break;

  case 59:
#line 517 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2328 "qlang-bison.cpp"
    break;

  case 60:
#line 521 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2334 "qlang-bison.cpp"
    break;

  case 61:
#line 522 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2340 "qlang-bison.cpp"
    break;

  case 62:
#line 523 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2346 "qlang-bison.cpp"
    break;

  case 63:
#line 524 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2352 "qlang-bison.cpp"
    break;

  case 64:
#line 525 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2358 "qlang-bison.cpp"
    break;

  case 65:
#line 529 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2364 "qlang-bison.cpp"
    break;

  case 66:
#line 530 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2370 "qlang-bison.cpp"
    break;

  case 67:
#line 531 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2376 "qlang-bison.cpp"
    break;

  case 68:
#line 532 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2382 "qlang-bison.cpp"
    break;

  case 69:
#line 533 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2388 "qlang-bison.cpp"
    break;

  case 70:
#line 534 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2394 "qlang-bison.cpp"
    break;

  case 71:
#line 535 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2400 "qlang-bison.cpp"
    break;

  case 72:
#line 540 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Slice((yystack_[5].value.node), (yystack_[3].value.node), (yystack_[1].value.node), yystack_[4].location.begin.line, yystack_[4].location.begin.column);
    }
#line 2408 "qlang-bison.cpp"
    break;

  case 73:
#line 547 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Subscript((yystack_[3].value.node), (yystack_[1].value.node), yystack_[2].location.begin.line, yystack_[2].location.begin.column);
    }
#line 2416 "qlang-bison.cpp"
    break;

  case 74:
#line 554 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Postcrement((yystack_[1].value.node), '+', yystack_[0].location.begin.line, yystack_[0].location.begin.column);
    }
#line 2424 "qlang-bison.cpp"
    break;

  case 75:
#line 558 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Postcrement((yystack_[1].value.node), '-', yystack_[0].location.begin.line, yystack_[0].location.begin.column);
    }
#line 2432 "qlang-bison.cpp"
    break;

  case 76:
#line 565 "qlang-bison.yy"
    {
        ast::FunctionCall *fcall = new ast::FunctionCall((yystack_[3].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(fcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = fcall;
        delete (yystack_[1].value.nodes);
    }
#line 2443 "qlang-bison.cpp"
    break;

  case 77:
#line 575 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Property(NULL, (yystack_[0].value.strVal)->ptr(), yystack_[0].location.begin.line, yystack_[0].location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2452 "qlang-bison.cpp"
    break;

  case 78:
#line 580 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Property((yystack_[2].value.node), (yystack_[0].value.strVal)->ptr(), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2461 "qlang-bison.cpp"
    break;

  case 79:
#line 588 "qlang-bison.yy"
    {
        ast::MethodCall *mcall = new ast::MethodCall(NULL, (yystack_[3].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(mcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = mcall;
        delete (yystack_[1].value.nodes);
    }
#line 2472 "qlang-bison.cpp"
    break;

  case 80:
#line 595 "qlang-bison.yy"
    {
        ast::MethodCall *mcall = new ast::MethodCall((yystack_[5].value.node), (yystack_[3].value.node), yystack_[4].location.begin.line, yystack_[4].location.begin.column);
        ADD_ELEMENTS(mcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = mcall;
        delete (yystack_[1].value.nodes);
    }
#line 2483 "qlang-bison.cpp"
    break;

  case 81:
#line 605 "qlang-bison.yy"
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error(yylhs.location, "$NUM calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall(yylhs.location.begin.line, yylhs.location.begin.column);
        dcall->setNum((yystack_[0].value.intVal));
        (yylhs.value.node) = dcall;
    }
#line 2498 "qlang-bison.cpp"
    break;

  case 82:
#line 616 "qlang-bison.yy"
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error(yylhs.location, "$NAME calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall(yylhs.location.begin.line, yylhs.location.begin.column);
        dcall->borrowNameFrom(*(yystack_[0].value.strVal));
        delete (yystack_[0].value.strVal);
        (yylhs.value.node) = dcall;
    }
#line 2514 "qlang-bison.cpp"
    break;

  case 83:
#line 630 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2520 "qlang-bison.cpp"
    break;

  case 84:
#line 631 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2526 "qlang-bison.cpp"
    break;

  case 85:
#line 632 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2532 "qlang-bison.cpp"
    break;

  case 86:
#line 634 "qlang-bison.yy"
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>((yystack_[0].value.node));
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->getValue() *= (idx)(-1);
            scalar->setLocation(yylhs.location.begin.line, yylhs.location.begin.column);
            (yylhs.value.node) = scalar;
        } else {
            (yylhs.value.node) = new ast::UnaryPlusMinus('-', (yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        }
    }
#line 2547 "qlang-bison.cpp"
    break;

  case 87:
#line 645 "qlang-bison.yy"
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>((yystack_[0].value.node));
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->setLocation(yylhs.location.begin.line, yylhs.location.begin.column);
            (yylhs.value.node) = scalar;
        } else {
            (yylhs.value.node) = new ast::UnaryPlusMinus('+', (yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        }
    }
#line 2561 "qlang-bison.cpp"
    break;

  case 88:
#line 655 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Not((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2569 "qlang-bison.cpp"
    break;

  case 89:
#line 662 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Precrement((yystack_[0].value.node), '+', yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2577 "qlang-bison.cpp"
    break;

  case 90:
#line 666 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Precrement((yystack_[0].value.node), '-', yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2585 "qlang-bison.cpp"
    break;

  case 91:
#line 673 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::BoolCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2593 "qlang-bison.cpp"
    break;

  case 92:
#line 677 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::IntCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2601 "qlang-bison.cpp"
    break;

  case 93:
#line 681 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::UIntCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2609 "qlang-bison.cpp"
    break;

  case 94:
#line 685 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::IntlistCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2617 "qlang-bison.cpp"
    break;

  case 95:
#line 689 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::RealCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2625 "qlang-bison.cpp"
    break;

  case 96:
#line 693 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::StringCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2633 "qlang-bison.cpp"
    break;

  case 97:
#line 697 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ObjCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2641 "qlang-bison.cpp"
    break;

  case 98:
#line 701 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ObjlistCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2649 "qlang-bison.cpp"
    break;

  case 99:
#line 705 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::DateTimeCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2657 "qlang-bison.cpp"
    break;

  case 100:
#line 709 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::DateCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2665 "qlang-bison.cpp"
    break;

  case 101:
#line 713 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::TimeCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2673 "qlang-bison.cpp"
    break;

  case 102:
#line 719 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2679 "qlang-bison.cpp"
    break;

  case 103:
#line 721 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '*', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2687 "qlang-bison.cpp"
    break;

  case 104:
#line 725 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '/', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2695 "qlang-bison.cpp"
    break;

  case 105:
#line 729 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '%', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2703 "qlang-bison.cpp"
    break;

  case 106:
#line 735 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2709 "qlang-bison.cpp"
    break;

  case 107:
#line 737 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '+', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2717 "qlang-bison.cpp"
    break;

  case 108:
#line 741 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '-', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2725 "qlang-bison.cpp"
    break;

  case 109:
#line 747 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2731 "qlang-bison.cpp"
    break;

  case 110:
#line 749 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Has((yystack_[2].value.node), (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2739 "qlang-bison.cpp"
    break;

  case 111:
#line 753 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Has((yystack_[2].value.node), (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2747 "qlang-bison.cpp"
    break;

  case 112:
#line 757 "qlang-bison.yy"
    {
        regex_t re;
        const char * flag_string = sString::next00((yystack_[0].value.strVal)->ptr());
        int flags = REG_EXTENDED;
        if (flag_string && strchr(flag_string, 'i')) {
            flags |= REG_ICASE;
        }
        if (regcomp(&re, (yystack_[0].value.strVal)->ptr(), flags)) {
            error(yystack_[0].location, "Invalid regular expression");
            delete (yystack_[0].value.strVal);
            YYERROR;
        }
        (yylhs.value.node) = new ast::Match((yystack_[2].value.node), '=', &re, (yystack_[0].value.strVal)->ptr(), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2767 "qlang-bison.cpp"
    break;

  case 113:
#line 773 "qlang-bison.yy"
    {
        regex_t re;
        const char * flag_string = sString::next00((yystack_[0].value.strVal)->ptr());
        int flags = REG_EXTENDED;
        if (flag_string && strchr(flag_string, 'i')) {
            flags |= REG_ICASE;
        }
        if (regcomp(&re, (yystack_[0].value.strVal)->ptr(), flags)) {
            error(yystack_[0].location, "Invalid regular expression");
            delete (yystack_[0].value.strVal);
            YYERROR;
        }
        (yylhs.value.node) = new ast::Match((yystack_[2].value.node), '!', &re, (yystack_[0].value.strVal)->ptr(), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2787 "qlang-bison.cpp"
    break;

  case 114:
#line 789 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::BoolCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2795 "qlang-bison.cpp"
    break;

  case 115:
#line 793 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::IntCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2803 "qlang-bison.cpp"
    break;

  case 116:
#line 797 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::UIntCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2811 "qlang-bison.cpp"
    break;

  case 117:
#line 801 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::IntlistCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2819 "qlang-bison.cpp"
    break;

  case 118:
#line 805 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::RealCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2827 "qlang-bison.cpp"
    break;

  case 119:
#line 809 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::StringCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2835 "qlang-bison.cpp"
    break;

  case 120:
#line 813 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ObjCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2843 "qlang-bison.cpp"
    break;

  case 121:
#line 817 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::ObjlistCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2851 "qlang-bison.cpp"
    break;

  case 122:
#line 821 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::DateTimeCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2859 "qlang-bison.cpp"
    break;

  case 123:
#line 825 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::DateCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2867 "qlang-bison.cpp"
    break;

  case 124:
#line 829 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::TimeCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2875 "qlang-bison.cpp"
    break;

  case 125:
#line 832 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2881 "qlang-bison.cpp"
    break;

  case 126:
#line 837 "qlang-bison.yy"
    {
        ast::FormatCall *fcall = new ast::FormatCall((yystack_[5].value.node), (yystack_[3].value.node), yystack_[3].location.begin.line, yystack_[3].location.begin.column);
        ADD_ELEMENTS(fcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = fcall;
        delete (yystack_[1].value.nodes);
    }
#line 2892 "qlang-bison.cpp"
    break;

  case 127:
#line 846 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2898 "qlang-bison.cpp"
    break;

  case 128:
#line 848 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::BinaryLogic((yystack_[2].value.node), '&', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2906 "qlang-bison.cpp"
    break;

  case 129:
#line 854 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2912 "qlang-bison.cpp"
    break;

  case 130:
#line 856 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::BinaryLogic((yystack_[2].value.node), '|', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2920 "qlang-bison.cpp"
    break;

  case 131:
#line 862 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2926 "qlang-bison.cpp"
    break;

  case 132:
#line 864 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::TernaryConditional((yystack_[4].value.node), (yystack_[2].value.node), (yystack_[0].value.node), yystack_[3].location.begin.line, yystack_[3].location.begin.column);
    }
#line 2934 "qlang-bison.cpp"
    break;

  case 133:
#line 870 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2940 "qlang-bison.cpp"
    break;

  case 134:
#line 872 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), "<", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2948 "qlang-bison.cpp"
    break;

  case 135:
#line 876 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), ">", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2956 "qlang-bison.cpp"
    break;

  case 136:
#line 880 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), "<=", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2964 "qlang-bison.cpp"
    break;

  case 137:
#line 884 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), ">=", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2972 "qlang-bison.cpp"
    break;

  case 138:
#line 888 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), "<=>", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2980 "qlang-bison.cpp"
    break;

  case 139:
#line 894 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 2986 "qlang-bison.cpp"
    break;

  case 140:
#line 896 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '=', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2994 "qlang-bison.cpp"
    break;

  case 141:
#line 900 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '=', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 3002 "qlang-bison.cpp"
    break;

  case 142:
#line 904 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '!', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 3010 "qlang-bison.cpp"
    break;

  case 143:
#line 908 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '!', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 3018 "qlang-bison.cpp"
    break;

  case 144:
#line 914 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 3024 "qlang-bison.cpp"
    break;

  case 145:
#line 919 "qlang-bison.yy"
    {
        ast::Junction *junc = new ast::Junction(yylhs.location.begin.line, yylhs.location.begin.column);
        junc->addElement((yystack_[2].value.node));
        junc->addElement((yystack_[0].value.node));
        (yylhs.value.node) = junc;
    }
#line 3035 "qlang-bison.cpp"
    break;

  case 146:
#line 926 "qlang-bison.yy"
    {
        (yylhs.value.node) = (yystack_[2].value.node);
        dynamic_cast<ast::Junction*>((yylhs.value.node))->addElement((yystack_[0].value.node));
    }
#line 3044 "qlang-bison.cpp"
    break;

  case 147:
#line 933 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 3050 "qlang-bison.cpp"
    break;

  case 148:
#line 934 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 3056 "qlang-bison.cpp"
    break;

  case 149:
#line 935 "qlang-bison.yy"
      { (yylhs.value.node) = (yystack_[0].value.node); }
#line 3062 "qlang-bison.cpp"
    break;

  case 150:
#line 940 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::BoolLiteral((yystack_[0].value.intVal), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 3070 "qlang-bison.cpp"
    break;

  case 151:
#line 944 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::IntLiteral((yystack_[0].value.intVal), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 3078 "qlang-bison.cpp"
    break;

  case 152:
#line 948 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::RealLiteral((yystack_[0].value.realVal), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 3086 "qlang-bison.cpp"
    break;

  case 153:
#line 952 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3095 "qlang-bison.cpp"
    break;

  case 154:
#line 957 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::NullLiteral(yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 3103 "qlang-bison.cpp"
    break;

  case 155:
#line 964 "qlang-bison.yy"
    {
        ast::ListLiteral *llit = new ast::ListLiteral(yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(llit, (yystack_[1].value.nodes));
        (yylhs.value.node) = llit;
        delete (yystack_[1].value.nodes);
    }
#line 3114 "qlang-bison.cpp"
    break;

  case 156:
#line 974 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3123 "qlang-bison.cpp"
    break;

  case 157:
#line 979 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3132 "qlang-bison.cpp"
    break;

  case 158:
#line 987 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(2, (yystack_[2].value.node), (yystack_[0].value.node));
    }
#line 3141 "qlang-bison.cpp"
    break;

  case 159:
#line 992 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = (yystack_[4].value.nodes);
        (yylhs.value.nodes)->vadd(2, (yystack_[2].value.node), (yystack_[0].value.node));
    }
#line 3150 "qlang-bison.cpp"
    break;

  case 160:
#line 1000 "qlang-bison.yy"
    {
        ast::DicLiteral *dlit = new ast::DicLiteral(yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(dlit, (yystack_[1].value.nodes));
        (yylhs.value.node) = dlit;
        delete (yystack_[1].value.nodes);
    }
#line 3161 "qlang-bison.cpp"
    break;

  case 161:
#line 1010 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::Variable((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3170 "qlang-bison.cpp"
    break;

  case 162:
#line 1018 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 3179 "qlang-bison.cpp"
    break;

  case 163:
#line 1023 "qlang-bison.yy"
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 3188 "qlang-bison.cpp"
    break;

  case 164:
#line 1031 "qlang-bison.yy"
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3197 "qlang-bison.cpp"
    break;

  case 165:
#line 1036 "qlang-bison.yy"
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        block->addElement((yystack_[1].value.node));
        (yylhs.value.node) = block;
        parser_driver.yyPopStateUntilTemplate();
    }
#line 3208 "qlang-bison.cpp"
    break;

  case 166:
#line 1043 "qlang-bison.yy"
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(block, (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        (yylhs.value.node) = block;
        parser_driver.yyPopStateUntilTemplate();
    }
#line 3220 "qlang-bison.cpp"
    break;


#line 3224 "qlang-bison.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif       YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  yyerrlab:
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {

        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    goto yyerrlab1;


  yyerrorlab:
    if (false)
      YYERROR;

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  yyerrlab1:
    yyerrstatus_ = 3;
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif   }

  void
  sQLangBison::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  std::string
  sQLangBison::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  sQLangBison::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  sQLangBison::context::context (const sQLangBison& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  sQLangBison::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    int yycount = 0;

    int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        int yyxbegin = yyn < 0 ? -yyn : 0;
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }



  int
  sQLangBison::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  std::string
  sQLangBison::yysyntax_error_ (const context& yyctx) const
  {
    enum { YYARGS_MAX = 5 };
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default:
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short sQLangBison::yypact_ninf_ = -225;

  const short sQLangBison::yytable_ninf_ = -162;

  const short
  sQLangBison::yypact_[] =
  {
     535,   509,   903,   592,   300,  -225,  -225,  -225,  -225,  -225,
    -225,   903,   903,   903,   903,   903,    37,    45,    49,    58,
      82,    95,   877,  -225,   649,  -225,  -225,   100,   434,   110,
    -225,  -225,  -225,    67,  -225,  -225,  -225,  -225,  -225,  -225,
     198,  -225,   114,   106,    48,  -225,  -225,   107,   116,   228,
     136,  -225,    64,   137,  -225,    56,    36,  -225,  -225,   112,
      -9,  -225,    86,   -21,   157,  -225,  -225,  -225,  -225,   303,
       4,  -225,   155,   156,   158,   160,   163,   164,   166,   169,
     170,   171,   172,   175,  -225,  -225,  -225,  -225,  -225,  -225,
     151,   174,  -225,   177,   178,   706,   173,   183,   180,   182,
      53,   903,   191,   200,  -225,    75,  -225,  -225,  -225,  -225,
    -225,  -225,   903,   903,   903,  -225,  -225,   115,  -225,   194,
     820,   215,   220,  -225,  -225,  -225,  -225,    48,  -225,  -225,
      -1,   903,   903,   903,   903,   903,   903,   903,  -225,   903,
     394,  -225,  -225,   187,   207,   903,   903,   903,   903,   903,
     903,   620,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,  -225,  -225,  -225,   721,   721,   721,   721,   721,
     721,   721,   721,   721,   721,   721,  -225,   903,  -225,  -225,
    -225,  -225,   903,  -225,   -12,   903,   903,   225,   221,   231,
    -225,    72,  -225,  -225,  -225,   820,   234,  -225,  -225,  -225,
    -225,  -225,  -225,  -225,  -225,  -225,   232,    68,   191,   237,
    -225,  -225,   229,   236,  -225,  -225,  -225,  -225,    56,    56,
    -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,
    -225,   238,   -21,    -3,   112,  -225,  -225,  -225,  -225,  -225,
      22,    86,   236,    86,   236,  -225,  -225,  -225,  -225,  -225,
    -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,
     241,   250,    80,   254,   903,   254,   258,   189,   763,  -225,
    -225,  -225,   903,  -225,   410,   903,   903,   903,   903,   903,
     903,   903,  -225,  -225,  -225,   257,  -225,   649,  -225,  -225,
    -225,  -225,   262,   261,   266,  -225,  -225,   267,  -225,  -225,
     903,   254,  -225,  -225,  -225,   269,  -225,   254,  -225
  };

  const unsigned char
  sQLangBison::yydefact_[] =
  {
       0,     0,    40,     0,     0,   150,   151,   152,   164,   153,
     154,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   161,     0,    81,    82,     0,     0,     0,
      18,    57,     8,    30,    22,    23,    24,    10,    25,    26,
       0,    62,    65,     0,    83,    66,    67,    68,    71,    69,
      70,    59,   102,    84,    85,   106,   109,   133,   125,   129,
     131,   144,   139,   127,     0,    55,   147,   148,   149,    56,
       0,   162,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    43,    68,    71,    70,    84,    42,
      41,     0,    38,   153,   161,     0,     0,     0,    43,     0,
       0,     0,    77,     0,    56,    83,    69,    87,    89,    86,
      90,    88,     0,    40,     0,    36,    37,     0,    34,     0,
       0,     0,     0,     1,     4,     9,    11,     0,    61,     3,
       0,     0,     0,     0,     0,     0,     0,    40,    27,     0,
       0,    74,    75,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     2,     5,   163,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    58,     0,   155,    15,
      14,    13,     0,   160,     0,    40,     0,     0,     0,     0,
      16,     0,    35,   166,   165,     0,     0,    31,    49,    47,
      48,    50,    51,    52,    53,    54,     0,     0,    78,     0,
     112,   113,   110,   111,   103,   104,   105,   102,   107,   108,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,     0,   128,     0,   130,   135,   134,   137,   136,   138,
     102,   140,   141,   142,   143,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,    39,   158,   157,   156,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     6,
      20,    21,     0,    76,    73,     0,    40,     0,     0,    40,
       0,     0,    79,    73,    28,     0,    33,     0,    19,    17,
      12,     7,     0,     0,     0,   145,   146,     0,   132,   159,
      40,     0,    72,    80,   126,     0,    29,     0,    32
  };

  const short
  sQLangBison::yypgoto_[] =
  {
    -225,  -225,  -225,    25,    44,  -224,     8,  -225,  -225,    12,
     -17,  -225,  -225,  -225,  -225,    -5,  -225,  -225,  -225,   -92,
      57,  -225,    77,     9,   255,  -225,   203,  -225,  -225,    10,
      26,   233,    27,  -225,     5,    43,  -225,  -118,  -225,   133,
    -225,   130,   132,     6,   -64,   141,    66,   -52,  -225,  -225,
    -225,   117,  -225,  -225,     0,  -225,   239
  };

  const short
  sQLangBison::yydefgoto_[] =
  {
      -1,    27,   268,    95,    96,   197,    30,   191,    31,   269,
      32,    33,    34,    35,    36,    37,    38,    39,    90,    91,
      92,    40,   198,    84,    42,    43,    44,    45,    46,    85,
      86,    49,    87,    51,    52,    88,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    89,   213,    65,    66,
      67,    99,   100,    68,   104,    70,    71
  };

  const short
  sQLangBison::yytable_[] =
  {
      69,    69,    69,    69,   163,   153,   195,   258,   280,    41,
      47,   125,    98,    47,   160,   161,   107,   108,   109,   110,
     111,   188,     8,   126,    69,    28,    48,    50,    69,    48,
      50,   218,   219,    98,    47,   277,   154,    41,    47,   284,
     112,   286,   154,    53,    29,   206,    53,   196,   113,   120,
      48,    50,   114,   139,    48,    50,   259,   140,    83,   149,
      97,   183,   150,   184,   143,   144,    64,    53,   121,   145,
     115,   128,   141,    24,   274,   142,   266,   306,   125,   275,
     186,   122,   267,   308,   140,   146,   283,   147,   119,   148,
     126,   275,   151,   261,   116,    69,   241,   243,   117,   141,
     123,    69,   142,   125,    41,    47,   143,   144,   242,   244,
     129,   145,    69,    69,    69,   126,   130,   137,   138,   -60,
      69,    48,    50,   155,   156,   157,   158,   159,   -63,    41,
      47,    69,    69,    69,    69,    69,    69,    69,   128,    69,
     199,   199,   199,   199,   199,   199,    48,    50,   -64,   -61,
     212,   214,   215,   216,   217,   217,   152,   162,    83,   165,
     166,   177,   167,   128,   168,   240,   240,   169,   170,   187,
     171,   189,   285,   172,   173,   174,   175,    69,   270,   176,
     178,   180,    69,   190,   294,    69,    69,   297,  -157,  -156,
     271,   181,   -62,   182,  -161,    69,   207,   200,   200,   200,
     200,   200,   200,   185,    41,    47,   192,   210,   305,   201,
     202,   203,   204,   205,   105,   105,   105,   105,   105,   193,
     131,    48,    50,   132,   194,   105,   133,   211,   134,   263,
     135,   127,   136,   264,   256,   265,   273,   272,   128,   257,
     276,   279,   277,   262,   106,   106,   106,   106,   106,   278,
     -45,   270,   281,   -45,   282,   106,   -45,   289,   -45,   103,
     -45,   195,   -45,   271,    69,   287,   301,   302,    69,   300,
     303,   304,    69,   307,   288,    69,    69,    41,    47,    69,
     291,    69,   295,   296,   234,   233,   298,    69,   235,   236,
     237,   238,   239,   232,    48,    50,    98,    47,   127,     0,
      69,   260,     0,   101,     0,     2,     0,     3,     0,   164,
       0,   128,     0,    48,    50,     5,     6,     7,     0,     9,
       0,    10,     0,   127,     0,   -44,     0,     0,   -44,   292,
      53,   -44,   293,   -44,     0,   -44,     0,   -44,   299,     0,
       0,     0,     0,     0,    97,     0,     0,     0,   105,   105,
     105,   105,   105,   105,    21,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,     0,     0,     0,   102,     0,
      25,    26,     0,     0,     0,     0,     0,     0,   106,   106,
     106,   106,   106,   106,     0,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   209,     0,   101,   127,     2,
       0,     3,     0,     0,     0,     0,   231,     0,     0,     5,
       6,     7,     0,     9,     0,    10,     0,     0,     0,     0,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,     0,   -46,     0,   124,   -46,     0,   101,   -46,     2,
     -46,     3,   -46,     4,   -46,     0,     0,     0,    21,     5,
       6,     7,     0,     9,     0,    10,     0,     0,    12,     0,
       0,    14,   208,     0,    25,    26,     0,     0,     0,     0,
       0,   127,     0,     0,     0,     0,     0,     0,     0,     0,
     105,   105,    16,   105,    17,    18,    19,    20,    21,    22,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,     0,    25,    26,     0,     0,     0,     0,
     106,   106,     1,   106,     2,     0,     3,     0,     4,     0,
       0,     0,     0,     0,     5,     6,     7,     0,     9,     0,
      10,     0,    11,    12,     0,    13,    14,     0,     1,     0,
       2,     0,     3,     0,     4,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,    15,    10,     0,    11,    12,
       0,    13,    14,    21,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    23,     0,    25,
      26,    15,     0,    16,     0,    17,    18,    19,    20,    21,
      22,     0,     0,     0,     0,     1,     0,     2,     0,     3,
       0,     4,     0,    23,    24,    25,    26,     5,     6,     7,
       0,    93,     0,    10,     0,    11,    12,     0,    13,    14,
       0,     0,     0,   101,     0,     2,     0,     3,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,    15,     9,
      16,    10,    17,    18,    19,    20,    21,    22,     0,     0,
       0,     0,     1,     0,     2,     0,     3,     0,     4,     0,
      94,     0,    25,    26,     5,     6,     7,     0,     9,     0,
      10,     0,    11,    12,    21,    13,    14,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,    23,     0,
      25,    26,     0,     0,     0,    15,     0,    16,     0,    17,
      18,    19,    20,    21,    22,     0,     0,     0,     0,   101,
       0,     2,     0,     3,   179,     4,     0,    23,     0,    25,
      26,     5,     6,     7,   101,     9,     2,    10,     3,     0,
      12,     0,     0,    14,     0,     0,     5,     6,     7,     0,
       9,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,     0,     0,     0,     0,   101,     0,     2,     0,
       3,   290,     4,     0,    23,    21,    25,    26,     5,     6,
       7,     0,     9,     0,    10,     0,     0,    12,     0,    23,
      14,    25,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,     0,
       0,     0,     0,   101,     0,     2,     0,     3,     0,     4,
       0,    23,     0,    25,    26,     5,     6,     7,     0,     9,
       0,    10,     0,     0,    12,     0,     0,    14,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,     0,     0,     0,     0,
       1,     0,     2,     0,     3,     0,     4,     0,    23,   118,
      25,    26,     5,     6,     7,     0,     9,     0,    10,     0,
      11,    12,     0,    13,    14,     0,     1,     0,     2,     0,
       3,     0,     4,     0,     0,     0,     0,     0,     5,     6,
       7,     0,     9,    15,    10,     0,    11,    12,     0,    13,
      14,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,     0,    25,    26,    15,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    23,     0,    25,    26
  };

  const short
  sQLangBison::yycheck_[] =
  {
       0,     1,     2,     3,     0,    14,     7,    19,    11,     0,
       0,    28,     3,     3,    35,    36,    11,    12,    13,    14,
      15,   113,    18,    28,    24,     0,     0,     0,    28,     3,
       3,   149,   150,    24,    24,    13,    45,    28,    28,   263,
       3,   265,    45,     0,     0,   137,     3,    48,     3,    24,
      24,    24,     3,     5,    28,    28,    68,     9,     1,    23,
       3,     8,    26,    10,    42,    43,     0,    24,    24,    47,
      12,    28,    24,    69,     6,    27,     4,   301,    95,    11,
       5,    24,    10,   307,     9,    29,     6,    31,    22,    33,
      95,    11,    56,   185,    12,    95,   160,   161,     3,    24,
       0,   101,    27,   120,    95,    95,    42,    43,   160,   161,
       0,    47,   112,   113,   114,   120,    49,     3,    12,    12,
     120,    95,    95,    37,    38,    39,    40,    41,    12,   120,
     120,   131,   132,   133,   134,   135,   136,   137,    95,   139,
     131,   132,   133,   134,   135,   136,   120,   120,    12,    12,
     145,   146,   147,   148,   149,   150,    44,     0,   101,     4,
       4,    10,     4,   120,     4,   160,   161,     4,     4,   112,
       4,   114,   264,     4,     4,     4,     4,   177,   195,     4,
       6,     8,   182,    68,   276,   185,   186,   279,    11,    11,
     195,     8,    12,    11,     3,   195,   139,   131,   132,   133,
     134,   135,   136,     3,   195,   195,    12,    20,   300,   132,
     133,   134,   135,   136,    11,    12,    13,    14,    15,     4,
      22,   195,   195,    25,     4,    22,    28,    20,    30,     4,
      32,    28,    34,    12,   177,     4,     4,     3,   195,   182,
       3,     3,    13,   186,    11,    12,    13,    14,    15,    13,
      22,   268,    11,    25,     4,    22,    28,    68,    30,     4,
      32,     7,    34,   268,   264,     7,     4,     6,   268,    12,
       4,     4,   272,     4,   266,   275,   276,   268,   268,   279,
     268,   281,   277,   278,   154,   153,   280,   287,   155,   156,
     157,   158,   159,   152,   268,   268,   287,   287,    95,    -1,
     300,   184,    -1,     3,    -1,     5,    -1,     7,    -1,    70,
      -1,   268,    -1,   287,   287,    15,    16,    17,    -1,    19,
      -1,    21,    -1,   120,    -1,    22,    -1,    -1,    25,   272,
     287,    28,   275,    30,    -1,    32,    -1,    34,   281,    -1,
      -1,    -1,    -1,    -1,   287,    -1,    -1,    -1,   145,   146,
     147,   148,   149,   150,    54,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,    -1,    -1,    -1,    68,    -1,
      70,    71,    -1,    -1,    -1,    -1,    -1,    -1,   145,   146,
     147,   148,   149,   150,    -1,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   140,    -1,     3,   195,     5,
      -1,     7,    -1,    -1,    -1,    -1,   151,    -1,    -1,    15,
      16,    17,    -1,    19,    -1,    21,    -1,    -1,    -1,    -1,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,    22,    -1,     0,    25,    -1,     3,    28,     5,
      30,     7,    32,     9,    34,    -1,    -1,    -1,    54,    15,
      16,    17,    -1,    19,    -1,    21,    -1,    -1,    24,    -1,
      -1,    27,    68,    -1,    70,    71,    -1,    -1,    -1,    -1,
      -1,   268,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     277,   278,    48,   280,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    70,    71,    -1,    -1,    -1,    -1,
     277,   278,     3,   280,     5,    -1,     7,    -1,     9,    -1,
      -1,    -1,    -1,    -1,    15,    16,    17,    -1,    19,    -1,
      21,    -1,    23,    24,    -1,    26,    27,    -1,     3,    -1,
       5,    -1,     7,    -1,     9,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    18,    19,    46,    21,    -1,    23,    24,
      -1,    26,    27,    54,    -1,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    46,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,     3,    -1,     5,    -1,     7,
      -1,     9,    -1,    68,    69,    70,    71,    15,    16,    17,
      -1,    19,    -1,    21,    -1,    23,    24,    -1,    26,    27,
      -1,    -1,    -1,     3,    -1,     5,    -1,     7,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    46,    19,
      48,    21,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,     3,    -1,     5,    -1,     7,    -1,     9,    -1,
      68,    -1,    70,    71,    15,    16,    17,    -1,    19,    -1,
      21,    -1,    23,    24,    54,    26,    27,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    -1,    -1,    -1,    46,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,     3,
      -1,     5,    -1,     7,     8,     9,    -1,    68,    -1,    70,
      71,    15,    16,    17,     3,    19,     5,    21,     7,    -1,
      24,    -1,    -1,    27,    -1,    -1,    15,    16,    17,    -1,
      19,    -1,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,     3,    -1,     5,    -1,
       7,     8,     9,    -1,    68,    54,    70,    71,    15,    16,
      17,    -1,    19,    -1,    21,    -1,    -1,    24,    -1,    68,
      27,    70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,     3,    -1,     5,    -1,     7,    -1,     9,
      -1,    68,    -1,    70,    71,    15,    16,    17,    -1,    19,
      -1,    21,    -1,    -1,    24,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
       3,    -1,     5,    -1,     7,    -1,     9,    -1,    68,    12,
      70,    71,    15,    16,    17,    -1,    19,    -1,    21,    -1,
      23,    24,    -1,    26,    27,    -1,     3,    -1,     5,    -1,
       7,    -1,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    -1,    19,    46,    21,    -1,    23,    24,    -1,    26,
      27,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    70,    71,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    70,    71
  };

  const unsigned char
  sQLangBison::yystos_[] =
  {
       0,     3,     5,     7,     9,    15,    16,    17,    18,    19,
      21,    23,    24,    26,    27,    46,    48,    50,    51,    52,
      53,    54,    55,    68,    69,    70,    71,    73,    75,    76,
      78,    80,    82,    83,    84,    85,    86,    87,    88,    89,
      93,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   120,   121,   122,   125,   126,
     127,   128,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    92,    95,   101,   102,   104,   107,   118,
      90,    91,    92,    19,    68,    75,    76,    92,    95,   123,
     124,     3,    68,    96,   126,    98,   103,   106,   106,   106,
     106,   106,     3,     3,     3,    12,    12,     3,    12,   118,
      75,    76,    92,     0,     0,    82,    87,    98,   107,     0,
      49,    22,    25,    28,    30,    32,    34,     3,    12,     5,
       9,    24,    27,    42,    43,    47,    29,    31,    33,    23,
      26,    56,    44,    14,    45,    37,    38,    39,    40,    41,
      35,    36,     0,     0,   128,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,    10,     6,     8,
       8,     8,    11,     8,    10,     3,     5,    92,    91,    92,
      68,    79,    12,     4,     4,     7,    48,    77,    94,    95,
     118,    94,    94,    94,    94,    94,    91,    92,    68,    96,
      20,    20,   106,   119,   106,   106,   106,   106,   109,   109,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    96,   117,   114,   113,   111,   111,   111,   111,   111,
     106,   116,   119,   116,   119,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    92,    92,    19,    68,
     123,    91,    92,     4,    12,     4,     4,    10,    74,    81,
      82,    87,     3,     4,     6,    11,     3,    13,    13,     3,
      11,    11,     4,     6,    77,    91,    77,     7,    78,    68,
       8,    81,    92,    92,    91,   106,   106,    91,   115,    92,
      12,     4,     6,     4,     4,    91,    77,     4,    77
  };

  const unsigned char
  sQLangBison::yyr1_[] =
  {
       0,    72,    73,    73,    73,    73,    74,    74,    75,    75,
      76,    76,    77,    78,    78,    78,    79,    79,    80,    80,
      81,    81,    82,    82,    82,    82,    82,    82,    83,    83,
      84,    84,    85,    86,    87,    87,    88,    89,    90,    90,
      91,    91,    92,    92,    93,    93,    93,    94,    94,    95,
      95,    95,    95,    95,    95,    96,    96,    96,    96,    96,
      97,    97,    97,    97,    97,    98,    98,    98,    98,    98,
      98,    98,    99,   100,   101,   101,   102,   103,   103,   104,
     104,   105,   105,   106,   106,   106,   106,   106,   106,   107,
     107,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   109,   109,   109,   109,   110,   110,   110,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   112,   113,   113,   114,
     114,   115,   115,   116,   116,   116,   116,   116,   116,   117,
     117,   117,   117,   117,   118,   119,   119,   120,   120,   120,
     121,   121,   121,   121,   121,   122,   123,   123,   124,   124,
     125,   126,   127,   127,   128,   128,   128
  };

  const signed char
  sQLangBison::yyr2_[] =
  {
       0,     2,     2,     2,     2,     2,     1,     2,     1,     2,
       1,     2,     3,     3,     3,     3,     1,     3,     1,     5,
       1,     1,     1,     1,     1,     1,     1,     2,     5,     7,
       1,     3,     9,     5,     2,     3,     2,     2,     1,     3,
       0,     1,     1,     1,     1,     1,     4,     1,     1,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     6,     4,     2,     2,     4,     2,     3,     5,
       6,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     6,     1,     3,     1,
       3,     1,     5,     1,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     3,     5,
       3,     1,     1,     2,     1,     3,     3
  };


#if YYDEBUG || 1
  const char*
  const sQLangBison::yytname_[] =
  {
  "\"end of program\"", "error", "\"invalid token\"", "'('", "')'", "'['",
  "']'", "'{'", "'}'", "'.'", "','", "':'", "';'", "'|'", "'?'",
  "\"bool literal\"", "\"int literal\"", "\"real literal\"",
  "\"template literal substring\"", "\"string literal\"",
  "\"regex literal\"", "\"null literal\"", "'='", "'+'", "\"'++'\"",
  "\"'+='\"", "'-'", "\"'--'\"", "\"'-='\"", "'*'", "\"'*='\"", "'/'",
  "\"'/='\"", "'%'", "\"'%='\"", "\"'=='\"", "\"'!='\"", "'>'", "'<'",
  "\"'>='\"", "\"'<='\"", "\"'<=>'\"", "\"'=~'\"", "\"'!~'\"", "\"'&&'\"",
  "\"'||'\"", "'!'", "\"'has'\"", "\"'if'\"", "\"'else'\"", "\"'for'\"",
  "\"'while'\"", "\"'break'\"", "\"'continue'\"", "\"'function'\"",
  "\"'return'\"", "\"'as'\"", "\"'bool'\"", "\"'int'\"", "\"'uint'\"",
  "\"'intlist'\"", "\"'real'\"", "\"'string'\"", "\"'obj'\"",
  "\"'objlist'\"", "\"'datetime'\"", "\"'date'\"", "\"'time'\"",
  "\"name\"", "\"$(\"", "\"$NUM\"", "\"$NAME\"", "$accept", "input",
  "statements", "non_return_statements", "statements_with_return",
  "statement_block", "lambda_block", "namelist", "lambda", "statement",
  "non_return_statement", "if_statement_start", "if_statement",
  "for_statement", "while_statement", "return_statement",
  "break_statement", "continue_statement", "arglist", "optional_arglist",
  "expression", "assignment_target", "assignment_source", "assignment",
  "basic_expression", "impure_expression", "postfix_expression", "slice",
  "subscript", "postcrement", "function_call", "property_call",
  "method_call", "dollar_call", "unary_expression", "precrement", "cast",
  "multiplication_priority", "addition_priority", "comparable",
  "format_call", "conjunction_priority", "disjunction_priority",
  "ternary_priority", "comparison_priority", "equality_priority",
  "non_assignment_expression", "junction", "literal", "scalar_literal",
  "list_literal", "kv_key", "kv_pairs", "dic_literal", "variable",
  "template", "template_expression", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  sQLangBison::yyrline_[] =
  {
       0,   218,   218,   226,   234,   244,   258,   263,   271,   276,
     284,   289,   297,   306,   312,   318,   329,   334,   343,   347,
     356,   357,   361,   362,   363,   364,   365,   366,   373,   377,
     386,   387,   395,   411,   418,   422,   429,   436,   443,   448,
     457,   460,   464,   465,   469,   470,   471,   478,   479,   483,
     487,   491,   495,   499,   503,   510,   511,   512,   513,   517,
     521,   522,   523,   524,   525,   529,   530,   531,   532,   533,
     534,   535,   539,   546,   553,   557,   564,   574,   579,   587,
     594,   604,   615,   630,   631,   632,   633,   644,   654,   661,
     665,   672,   676,   680,   684,   688,   692,   696,   700,   704,
     708,   712,   719,   720,   724,   728,   735,   736,   740,   747,
     748,   752,   756,   772,   788,   792,   796,   800,   804,   808,
     812,   816,   820,   824,   828,   832,   836,   846,   847,   854,
     855,   862,   863,   870,   871,   875,   879,   883,   887,   894,
     895,   899,   903,   907,   914,   918,   925,   933,   934,   935,
     939,   943,   947,   951,   956,   963,   973,   978,   986,   991,
     999,  1009,  1017,  1022,  1030,  1035,  1042
  };

  void
  sQLangBison::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  sQLangBison::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif 
  sQLangBison::symbol_kind_type
  sQLangBison::yytranslate_ (int t)
  {
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    46,     2,     2,     2,    33,     2,     2,
       3,     4,    29,    23,    10,    26,     9,    31,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    11,    12,
      38,    22,    37,    14,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     5,     2,     6,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     7,    13,     8,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,    15,    16,
      17,    18,    19,    20,    21,    24,    25,    27,    28,    30,
      32,    34,    35,    36,    39,    40,    41,    42,    43,    44,
      45,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71
    };
    const int code_max = 305;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return YY_CAST (symbol_kind_type, translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

}
#line 4100 "qlang-bison.cpp"

#line 1052 "qlang-bison.yy"

