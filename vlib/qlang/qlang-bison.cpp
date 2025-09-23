// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.

#line 37 "qlang-bison.cpp" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "qlang-bison.hpp"

// User implementation prologue.

#line 51 "qlang-bison.cpp" // lalr1.cc:412


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

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
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
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
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
#line 137 "qlang-bison.cpp" // lalr1.cc:479

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  sQLangBison::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
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
              // Fall through.
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


  /// Build a parser object.
  sQLangBison::sQLangBison (slib::qlang::Parser& parser_driver_yyarg, yyscan_t yyscanner_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      parser_driver (parser_driver_yyarg),
      yyscanner (yyscanner_yyarg)
  {}

  sQLangBison::~sQLangBison ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/

  inline
  sQLangBison::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // basic_symbol.
  template <typename Base>
  inline
  sQLangBison::basic_symbol<Base>::basic_symbol ()
    : value ()
  {}

  template <typename Base>
  inline
  sQLangBison::basic_symbol<Base>::basic_symbol (const basic_symbol& other)
    : Base (other)
    , value ()
    , location (other.location)
  {
    value = other.value;
  }


  template <typename Base>
  inline
  sQLangBison::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const semantic_type& v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  inline
  sQLangBison::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  inline
  sQLangBison::basic_symbol<Base>::~basic_symbol ()
  {
    clear ();
  }

  template <typename Base>
  inline
  void
  sQLangBison::basic_symbol<Base>::clear ()
  {
    Base::clear ();
  }

  template <typename Base>
  inline
  bool
  sQLangBison::basic_symbol<Base>::empty () const
  {
    return Base::type_get () == empty_symbol;
  }

  template <typename Base>
  inline
  void
  sQLangBison::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move(s);
    value = s.value;
    location = s.location;
  }

  // by_type.
  inline
  sQLangBison::by_type::by_type ()
    : type (empty_symbol)
  {}

  inline
  sQLangBison::by_type::by_type (const by_type& other)
    : type (other.type)
  {}

  inline
  sQLangBison::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  inline
  void
  sQLangBison::by_type::clear ()
  {
    type = empty_symbol;
  }

  inline
  void
  sQLangBison::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
  }

  inline
  int
  sQLangBison::by_type::type_get () const
  {
    return type;
  }


  // by_state.
  inline
  sQLangBison::by_state::by_state ()
    : state (empty_state)
  {}

  inline
  sQLangBison::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  sQLangBison::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
  sQLangBison::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
  sQLangBison::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  sQLangBison::symbol_number_type
  sQLangBison::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  inline
  sQLangBison::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  sQLangBison::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
    value = that.value;
    // that is emptied.
    that.type = empty_symbol;
  }

  inline
  sQLangBison::stack_symbol_type&
  sQLangBison::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  sQLangBison::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yysym.type_get ())
    {
            case 18: // "template literal substring"

#line 76 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.strVal); }
#line 391 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 19: // "string literal"

#line 76 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.strVal); }
#line 398 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 20: // "regex literal"

#line 76 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.strVal); }
#line 405 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 68: // "name"

#line 76 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.strVal); }
#line 412 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 71: // "$NAME"

#line 76 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.strVal); }
#line 419 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 73: // input

#line 78 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.block); }
#line 426 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 74: // statements

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 437 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 75: // non_return_statements

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 448 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 76: // statements_with_return

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 459 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 77: // statement_block

#line 78 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.block); }
#line 466 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 78: // lambda_block

#line 78 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.block); }
#line 473 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 79: // namelist

#line 76 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.strVal); }
#line 480 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 80: // lambda

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 487 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 81: // statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 494 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 82: // non_return_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 501 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 83: // if_statement_start

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 508 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 84: // if_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 515 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 85: // for_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 522 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 86: // while_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 529 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 87: // return_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 536 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 88: // break_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 543 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 89: // continue_statement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 550 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 90: // arglist

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 561 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 91: // optional_arglist

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 572 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 92: // expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 579 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 93: // assignment_target

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 586 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 94: // assignment_source

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 593 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 95: // assignment

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 600 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 96: // basic_expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 607 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 97: // impure_expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 614 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 98: // postfix_expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 621 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 99: // slice

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 628 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 100: // subscript

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 635 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 101: // postcrement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 642 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 102: // function_call

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 649 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 103: // property_call

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 656 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 104: // method_call

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 663 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 105: // dollar_call

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 670 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 106: // unary_expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 677 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 107: // precrement

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 684 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 108: // cast

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 691 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 109: // multiplication_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 698 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 110: // addition_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 705 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 111: // comparable

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 712 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 112: // format_call

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 719 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 113: // conjunction_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 726 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 114: // disjunction_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 733 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 115: // ternary_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 740 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 116: // comparison_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 747 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 117: // equality_priority

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 754 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 118: // non_assignment_expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 761 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 119: // junction

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 768 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 120: // literal

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 775 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 121: // scalar_literal

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 782 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 122: // list_literal

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 789 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 123: // kv_key

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 796 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 124: // kv_pairs

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 807 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 125: // dic_literal

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 814 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 126: // variable

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 821 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 127: // template

#line 79 "qlang-bison.yy" // lalr1.cc:614
        {
    for (idx i=0; i<(yysym.value.nodes)->dim(); i++)
        delete (*(yysym.value.nodes))[i];
    delete (yysym.value.nodes);
}
#line 832 "qlang-bison.cpp" // lalr1.cc:614
        break;

      case 128: // template_expression

#line 77 "qlang-bison.yy" // lalr1.cc:614
        { delete (yysym.value.node); }
#line 839 "qlang-bison.cpp" // lalr1.cc:614
        break;


      default:
        break;
    }
  }

#if YYDEBUG
  template <typename Base>
  void
  sQLangBison::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
    {
            case 15: // "bool literal"

#line 85 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << (yysym.value.intVal); }
#line 870 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 16: // "int literal"

#line 85 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << (yysym.value.intVal); }
#line 877 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 17: // "real literal"

#line 85 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << (yysym.value.realVal); }
#line 884 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 18: // "template literal substring"

#line 86 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 891 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 19: // "string literal"

#line 86 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 898 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 20: // "regex literal"

#line 86 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 905 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 68: // "name"

#line 86 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 912 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 70: // "$NUM"

#line 85 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << (yysym.value.intVal); }
#line 919 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 71: // "$NAME"

#line 86 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 926 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 73: // input

#line 96 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.block)) {
        sStr s;
        (yysym.value.block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 941 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 74: // statements

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 957 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 75: // non_return_statements

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 973 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 76: // statements_with_return

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 989 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 77: // statement_block

#line 96 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.block)) {
        sStr s;
        (yysym.value.block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1004 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 78: // lambda_block

#line 96 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.block)) {
        sStr s;
        (yysym.value.block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1019 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 79: // namelist

#line 86 "qlang-bison.yy" // lalr1.cc:636
        { debug_stream() << '"' << ((yysym.value.strVal) ? (yysym.value.strVal)->ptr() : "(null)") << '"'; }
#line 1026 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 80: // lambda

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1041 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 81: // statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1056 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 82: // non_return_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1071 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 83: // if_statement_start

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1086 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 84: // if_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1101 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 85: // for_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1116 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 86: // while_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1131 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 87: // return_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1146 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 88: // break_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1161 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 89: // continue_statement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1176 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 90: // arglist

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 1192 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 91: // optional_arglist

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 1208 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 92: // expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1223 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 93: // assignment_target

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1238 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 94: // assignment_source

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1253 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 95: // assignment

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1268 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 96: // basic_expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1283 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 97: // impure_expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1298 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 98: // postfix_expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1313 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 99: // slice

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1328 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 100: // subscript

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1343 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 101: // postcrement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1358 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 102: // function_call

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1373 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 103: // property_call

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1388 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 104: // method_call

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1403 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 105: // dollar_call

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1418 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 106: // unary_expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1433 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 107: // precrement

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1448 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 108: // cast

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1463 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 109: // multiplication_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1478 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 110: // addition_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1493 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 111: // comparable

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1508 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 112: // format_call

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1523 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 113: // conjunction_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1538 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 114: // disjunction_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1553 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 115: // ternary_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1568 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 116: // comparison_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1583 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 117: // equality_priority

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1598 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 118: // non_assignment_expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1613 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 119: // junction

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1628 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 120: // literal

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1643 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 121: // scalar_literal

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1658 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 122: // list_literal

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1673 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 123: // kv_key

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1688 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 124: // kv_pairs

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 1704 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 125: // dic_literal

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1719 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 126: // variable

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1734 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 127: // template

#line 105 "qlang-bison.yy" // lalr1.cc:636
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
#line 1750 "qlang-bison.cpp" // lalr1.cc:636
        break;

      case 128: // template_expression

#line 87 "qlang-bison.yy" // lalr1.cc:636
        {
    if ((yysym.value.node)) {
        sStr s;
        (yysym.value.node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
}
#line 1765 "qlang-bison.cpp" // lalr1.cc:636
        break;


      default:
        break;
    }
    yyo << ')';
  }
#endif

  inline
  void
  sQLangBison::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  sQLangBison::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  sQLangBison::yypop_ (unsigned int n)
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
#endif // YYDEBUG

  inline sQLangBison::state_type
  sQLangBison::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  sQLangBison::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  sQLangBison::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  sQLangBison::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    // User initialization code.
    #line 60 "qlang-bison.yy" // lalr1.cc:741
{
//    @$.begin.filename = @$.end.filename = parser_driver.getFilename();
}

#line 1883 "qlang-bison.cpp" // lalr1.cc:741

    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location, parser_driver, yyscanner));
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:
#line 238 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ast::Return *ret = new ast::Return((yystack_[1].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 1999 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 3:
#line 246 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(root, (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 2011 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 4:
#line 254 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::Block *root = new ast::Block(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(root, (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        ast::Return * ret = new ast::Return(0, yylhs.location.end.line, yylhs.location.end.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        (yylhs.value.block) = NULL;
    }
#line 2025 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 5:
#line 264 "qlang-bison.yy" // lalr1.cc:859
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
#line 2040 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 6:
#line 278 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2049 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 7:
#line 283 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2058 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 8:
#line 291 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2067 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 9:
#line 296 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2076 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 10:
#line 304 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2085 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 11:
#line 309 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2094 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 12:
#line 317 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.block) = new ast::Block(true, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS((yylhs.value.block), (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
    }
#line 2104 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 13:
#line 326 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.block) = new ast::Block(false, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ast::Return *r = new ast::Return((yystack_[1].value.node), yystack_[2].location.begin.line, yystack_[2].location.begin.column);
        (yylhs.value.block)->addElement(r);
    }
#line 2114 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 14:
#line 332 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.block) = new ast::Block(false, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS((yylhs.value.block), (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
    }
#line 2124 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 15:
#line 338 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.block) = new ast::Block(false, false, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS((yylhs.value.block), (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        ast::Return * r = new ast::Return(0, yylhs.location.end.line, yylhs.location.end.column);
        (yylhs.value.block)->addElement(r);
    }
#line 2136 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 16:
#line 349 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.strVal) = (yystack_[0].value.strVal);
        (yylhs.value.strVal)->add0();
    }
#line 2145 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 17:
#line 354 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.strVal) = (yystack_[2].value.strVal);
        (yylhs.value.strVal)->add((yystack_[0].value.strVal)->ptr());
        delete (yystack_[0].value.strVal);
    }
#line 2155 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 18:
#line 363 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Lambda(NULL, (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2163 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 19:
#line 367 "qlang-bison.yy" // lalr1.cc:859
    {
        (yystack_[2].value.strVal)->add0();
        (yylhs.value.node) = new ast::Lambda((yystack_[2].value.strVal)->ptr(), (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[2].value.strVal);
    }
#line 2173 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 27:
#line 386 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = (yystack_[1].value.node);
    }
#line 2181 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 28:
#line 393 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::If((yystack_[2].value.node), (yystack_[0].value.block), NULL, yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2189 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 29:
#line 397 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = (yystack_[6].value.node);
        ast::If *chain = new ast::If((yystack_[2].value.node), (yystack_[0].value.block), NULL, yystack_[4].location.begin.line, yystack_[4].location.begin.column);
        dynamic_cast<ast::If*>((yylhs.value.node))->setLastElse(chain);
    }
#line 2199 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 31:
#line 407 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = (yystack_[2].value.node);
        dynamic_cast<ast::If*>((yylhs.value.node))->setLastElse((yystack_[0].value.block));
    }
#line 2208 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 32:
#line 415 "qlang-bison.yy" // lalr1.cc:859
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
#line 2225 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 33:
#line 431 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::While((yystack_[2].value.node), (yystack_[0].value.block), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2233 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 34:
#line 438 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Return(0, yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2241 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 35:
#line 442 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Return((yystack_[1].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2249 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 36:
#line 449 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Break(yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2257 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 37:
#line 456 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Continue(yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2265 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 38:
#line 463 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2274 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 39:
#line 468 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = (yystack_[2].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 2283 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 40:
#line 476 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
    }
#line 2291 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 46:
#line 491 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Subscript((yystack_[3].value.node), (yystack_[1].value.node), yystack_[2].location.begin.line, yystack_[2].location.begin.column);
    }
#line 2299 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 49:
#line 503 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Assign((yystack_[2].value.node), (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2307 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 50:
#line 507 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '+', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2315 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 51:
#line 511 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '-', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2323 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 52:
#line 515 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '*', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2331 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 53:
#line 519 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '/', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2339 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 54:
#line 523 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ArithmeticInplace((yystack_[2].value.node), '%', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2347 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 58:
#line 533 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = (yystack_[1].value.node);
    }
#line 2355 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 72:
#line 559 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Slice((yystack_[5].value.node), (yystack_[3].value.node), (yystack_[1].value.node), yystack_[4].location.begin.line, yystack_[4].location.begin.column);
    }
#line 2363 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 73:
#line 566 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Subscript((yystack_[3].value.node), (yystack_[1].value.node), yystack_[2].location.begin.line, yystack_[2].location.begin.column);
    }
#line 2371 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 74:
#line 573 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Postcrement((yystack_[1].value.node), '+', yystack_[0].location.begin.line, yystack_[0].location.begin.column);
    }
#line 2379 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 75:
#line 577 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Postcrement((yystack_[1].value.node), '-', yystack_[0].location.begin.line, yystack_[0].location.begin.column);
    }
#line 2387 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 76:
#line 584 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::FunctionCall *fcall = new ast::FunctionCall((yystack_[3].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(fcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = fcall;
        delete (yystack_[1].value.nodes);
    }
#line 2398 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 77:
#line 594 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Property(NULL, (yystack_[0].value.strVal)->ptr(), yystack_[0].location.begin.line, yystack_[0].location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2407 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 78:
#line 599 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Property((yystack_[2].value.node), (yystack_[0].value.strVal)->ptr(), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2416 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 79:
#line 607 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::MethodCall *mcall = new ast::MethodCall(NULL, (yystack_[3].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(mcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = mcall;
        delete (yystack_[1].value.nodes);
    }
#line 2427 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 80:
#line 614 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::MethodCall *mcall = new ast::MethodCall((yystack_[5].value.node), (yystack_[3].value.node), yystack_[4].location.begin.line, yystack_[4].location.begin.column);
        ADD_ELEMENTS(mcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = mcall;
        delete (yystack_[1].value.nodes);
    }
#line 2438 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 81:
#line 624 "qlang-bison.yy" // lalr1.cc:859
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error(yylhs.location, "$NUM calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall(yylhs.location.begin.line, yylhs.location.begin.column);
        dcall->setNum((yystack_[0].value.intVal));
        (yylhs.value.node) = dcall;
    }
#line 2453 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 82:
#line 635 "qlang-bison.yy" // lalr1.cc:859
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
#line 2469 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 86:
#line 653 "qlang-bison.yy" // lalr1.cc:859
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
#line 2484 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 87:
#line 664 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>((yystack_[0].value.node));
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->setLocation(yylhs.location.begin.line, yylhs.location.begin.column);
            (yylhs.value.node) = scalar;
        } else {
            (yylhs.value.node) = new ast::UnaryPlusMinus('+', (yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
        }
    }
#line 2498 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 88:
#line 674 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Not((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2506 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 89:
#line 681 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Precrement((yystack_[0].value.node), '+', yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2514 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 90:
#line 685 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Precrement((yystack_[0].value.node), '-', yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2522 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 91:
#line 692 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::BoolCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2530 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 92:
#line 696 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::IntCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2538 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 93:
#line 700 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::UIntCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2546 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 94:
#line 704 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::IntlistCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2554 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 95:
#line 708 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::RealCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2562 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 96:
#line 712 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::StringCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2570 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 97:
#line 716 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ObjCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2578 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 98:
#line 720 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ObjlistCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2586 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 99:
#line 724 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::DateTimeCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2594 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 100:
#line 728 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::DateCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2602 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 101:
#line 732 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::TimeCast((yystack_[0].value.node), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2610 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 103:
#line 740 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '*', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2618 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 104:
#line 744 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '/', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2626 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 105:
#line 748 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '%', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2634 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 107:
#line 756 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '+', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2642 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 108:
#line 760 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Arithmetic((yystack_[2].value.node), '-', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2650 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 110:
#line 768 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Has((yystack_[2].value.node), (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2658 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 111:
#line 772 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Has((yystack_[2].value.node), (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2666 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 112:
#line 776 "qlang-bison.yy" // lalr1.cc:859
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
#line 2686 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 113:
#line 792 "qlang-bison.yy" // lalr1.cc:859
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
#line 2706 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 114:
#line 808 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::BoolCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2714 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 115:
#line 812 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::IntCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2722 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 116:
#line 816 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::UIntCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2730 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 117:
#line 820 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::IntlistCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2738 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 118:
#line 824 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::RealCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2746 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 119:
#line 828 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::StringCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2754 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 120:
#line 832 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ObjCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2762 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 121:
#line 836 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::ObjlistCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2770 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 122:
#line 840 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::DateTimeCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2778 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 123:
#line 844 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::DateCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2786 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 124:
#line 848 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::TimeCast((yystack_[2].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2794 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 126:
#line 856 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::FormatCall *fcall = new ast::FormatCall((yystack_[5].value.node), (yystack_[3].value.node), yystack_[3].location.begin.line, yystack_[3].location.begin.column);
        ADD_ELEMENTS(fcall, (yystack_[1].value.nodes));
        (yylhs.value.node) = fcall;
        delete (yystack_[1].value.nodes);
    }
#line 2805 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 128:
#line 867 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::BinaryLogic((yystack_[2].value.node), '&', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2813 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 130:
#line 875 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::BinaryLogic((yystack_[2].value.node), '|', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2821 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 132:
#line 883 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::TernaryConditional((yystack_[4].value.node), (yystack_[2].value.node), (yystack_[0].value.node), yystack_[3].location.begin.line, yystack_[3].location.begin.column);
    }
#line 2829 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 134:
#line 891 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), "<", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2837 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 135:
#line 895 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), ">", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2845 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 136:
#line 899 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), "<=", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2853 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 137:
#line 903 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), ">=", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2861 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 138:
#line 907 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Comparison((yystack_[2].value.node), "<=>", (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2869 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 140:
#line 915 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '=', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2877 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 141:
#line 919 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '=', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2885 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 142:
#line 923 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '!', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2893 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 143:
#line 927 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Equality((yystack_[2].value.node), '!', (yystack_[0].value.node), yystack_[1].location.begin.line, yystack_[1].location.begin.column);
    }
#line 2901 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 145:
#line 938 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::Junction *junc = new ast::Junction(yylhs.location.begin.line, yylhs.location.begin.column);
        junc->addElement((yystack_[2].value.node));
        junc->addElement((yystack_[0].value.node));
        (yylhs.value.node) = junc;
    }
#line 2912 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 146:
#line 945 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = (yystack_[2].value.node);
        dynamic_cast<ast::Junction*>((yylhs.value.node))->addElement((yystack_[0].value.node));
    }
#line 2921 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 150:
#line 959 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::BoolLiteral((yystack_[0].value.intVal), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2929 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 151:
#line 963 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::IntLiteral((yystack_[0].value.intVal), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2937 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 152:
#line 967 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::RealLiteral((yystack_[0].value.realVal), yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2945 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 153:
#line 971 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2954 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 154:
#line 976 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::NullLiteral(yylhs.location.begin.line, yylhs.location.begin.column);
    }
#line 2962 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 155:
#line 983 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::ListLiteral *llit = new ast::ListLiteral(yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(llit, (yystack_[1].value.nodes));
        (yylhs.value.node) = llit;
        delete (yystack_[1].value.nodes);
    }
#line 2973 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 156:
#line 993 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2982 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 157:
#line 998 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 2991 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 158:
#line 1006 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(2, (yystack_[2].value.node), (yystack_[0].value.node));
    }
#line 3000 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 159:
#line 1011 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = (yystack_[4].value.nodes);
        (yylhs.value.nodes)->vadd(2, (yystack_[2].value.node), (yystack_[0].value.node));
    }
#line 3009 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 160:
#line 1019 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::DicLiteral *dlit = new ast::DicLiteral(yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(dlit, (yystack_[1].value.nodes));
        (yylhs.value.node) = dlit;
        delete (yystack_[1].value.nodes);
    }
#line 3020 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 161:
#line 1029 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::Variable((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3029 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 162:
#line 1037 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = new sVec<ast::Node*>;
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 3038 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 163:
#line 1042 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.nodes) = (yystack_[1].value.nodes);
        (yylhs.value.nodes)->vadd(1, (yystack_[0].value.node));
    }
#line 3047 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 164:
#line 1050 "qlang-bison.yy" // lalr1.cc:859
    {
        (yylhs.value.node) = new ast::StringLiteral((yystack_[0].value.strVal)->ptr(), yylhs.location.begin.line, yylhs.location.begin.column);
        delete (yystack_[0].value.strVal);
    }
#line 3056 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 165:
#line 1055 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        block->addElement((yystack_[1].value.node));
        (yylhs.value.node) = block;
        parser_driver.yyPopStateUntilTemplate(); // switch scanner state back to template
    }
#line 3067 "qlang-bison.cpp" // lalr1.cc:859
    break;

  case 166:
#line 1062 "qlang-bison.yy" // lalr1.cc:859
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, yylhs.location.begin.line, yylhs.location.begin.column);
        ADD_ELEMENTS(block, (yystack_[1].value.nodes));
        delete (yystack_[1].value.nodes);
        (yylhs.value.node) = block;
        parser_driver.yyPopStateUntilTemplate(); // switch scanner state back to template
    }
#line 3079 "qlang-bison.cpp" // lalr1.cc:859
    break;


#line 3083 "qlang-bison.cpp" // lalr1.cc:859
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  sQLangBison::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  sQLangBison::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short int sQLangBison::yypact_ninf_ = -225;

  const short int sQLangBison::yytable_ninf_ = -162;

  const short int
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

  const short int
  sQLangBison::yypgoto_[] =
  {
    -225,  -225,  -225,    25,    44,  -224,     8,  -225,  -225,    12,
     -17,  -225,  -225,  -225,  -225,    -5,  -225,  -225,  -225,   -92,
      57,  -225,    77,     9,   255,  -225,   203,  -225,  -225,    10,
      26,   233,    27,  -225,     5,    43,  -225,  -118,  -225,   133,
    -225,   130,   132,     6,   -64,   141,    66,   -52,  -225,  -225,
    -225,   117,  -225,  -225,     0,  -225,   239
  };

  const short int
  sQLangBison::yydefgoto_[] =
  {
      -1,    27,   268,    95,    96,   197,    30,   191,    31,   269,
      32,    33,    34,    35,    36,    37,    38,    39,    90,    91,
      92,    40,   198,    84,    42,    43,    44,    45,    46,    85,
      86,    49,    87,    51,    52,    88,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    89,   213,    65,    66,
      67,    99,   100,    68,   104,    70,    71
  };

  const short int
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

  const short int
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

  const unsigned char
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



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const sQLangBison::yytname_[] =
  {
  "\"end of program\"", "error", "$undefined", "'('", "')'", "'['", "']'",
  "'{'", "'}'", "'.'", "','", "':'", "';'", "'|'", "'?'",
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

#if YYDEBUG
  const unsigned short int
  sQLangBison::yyrline_[] =
  {
       0,   237,   237,   245,   253,   263,   277,   282,   290,   295,
     303,   308,   316,   325,   331,   337,   348,   353,   362,   366,
     375,   376,   380,   381,   382,   383,   384,   385,   392,   396,
     405,   406,   414,   430,   437,   441,   448,   455,   462,   467,
     476,   479,   483,   484,   488,   489,   490,   497,   498,   502,
     506,   510,   514,   518,   522,   529,   530,   531,   532,   536,
     540,   541,   542,   543,   544,   548,   549,   550,   551,   552,
     553,   554,   558,   565,   572,   576,   583,   593,   598,   606,
     613,   623,   634,   649,   650,   651,   652,   663,   673,   680,
     684,   691,   695,   699,   703,   707,   711,   715,   719,   723,
     727,   731,   738,   739,   743,   747,   754,   755,   759,   766,
     767,   771,   775,   791,   807,   811,   815,   819,   823,   827,
     831,   835,   839,   843,   847,   851,   855,   865,   866,   873,
     874,   881,   882,   889,   890,   894,   898,   902,   906,   913,
     914,   918,   922,   926,   933,   937,   944,   952,   953,   954,
     958,   962,   966,   970,   975,   982,   992,   997,  1005,  1010,
    1018,  1028,  1036,  1041,  1049,  1054,  1061
  };

  // Print the state stack on the debug stream.
  void
  sQLangBison::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  sQLangBison::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  // Symbol number corresponding to token number t.
  inline
  sQLangBison::token_number_type
  sQLangBison::yytranslate_ (int t)
  {
    static
    const token_number_type
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
    const unsigned int user_token_number_max_ = 305;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} // yy
#line 3866 "qlang-bison.cpp" // lalr1.cc:1167
#line 1071 "qlang-bison.yy" // lalr1.cc:1168

