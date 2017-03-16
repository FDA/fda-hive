/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* First part of user declarations.  */

/* Line 279 of lalr1.cc  */
#line 38 "qlang-bison.cpp"


#include "qlang-bison.hpp"

/* User implementation prologue.  */

/* Line 285 of lalr1.cc  */
#line 46 "qlang-bison.cpp"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
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


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location) YYUSE(Type)
# define YY_REDUCE_PRINT(Rule)        static_cast<void>(0)
# define YY_STACK_PRINT()             static_cast<void>(0)

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
/* Line 353 of lalr1.cc  */
#line 141 "qlang-bison.cpp"

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
              /* Fall through.  */
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
  {
  }

  sQLangBison::~sQLangBison ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  sQLangBison::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    switch (yytype)
      {
        case 15: /* "int literal" */
/* Line 423 of lalr1.cc  */
#line 85 "qlang-bison.yy"
        { debug_stream() << ((*yyvaluep).intVal); };
/* Line 423 of lalr1.cc  */
#line 218 "qlang-bison.cpp"
        break;
      case 16: /* "real literal" */
/* Line 423 of lalr1.cc  */
#line 85 "qlang-bison.yy"
        { debug_stream() << ((*yyvaluep).realVal); };
/* Line 423 of lalr1.cc  */
#line 225 "qlang-bison.cpp"
        break;
      case 17: /* "template literal substring" */
/* Line 423 of lalr1.cc  */
#line 86 "qlang-bison.yy"
        { debug_stream() << '"' << (((*yyvaluep).strVal) ? ((*yyvaluep).strVal)->ptr() : "(null)") << '"'; };
/* Line 423 of lalr1.cc  */
#line 232 "qlang-bison.cpp"
        break;
      case 18: /* "string literal" */
/* Line 423 of lalr1.cc  */
#line 86 "qlang-bison.yy"
        { debug_stream() << '"' << (((*yyvaluep).strVal) ? ((*yyvaluep).strVal)->ptr() : "(null)") << '"'; };
/* Line 423 of lalr1.cc  */
#line 239 "qlang-bison.cpp"
        break;
      case 19: /* "regex literal" */
/* Line 423 of lalr1.cc  */
#line 86 "qlang-bison.yy"
        { debug_stream() << '"' << (((*yyvaluep).strVal) ? ((*yyvaluep).strVal)->ptr() : "(null)") << '"'; };
/* Line 423 of lalr1.cc  */
#line 246 "qlang-bison.cpp"
        break;
      case 67: /* "name" */
/* Line 423 of lalr1.cc  */
#line 86 "qlang-bison.yy"
        { debug_stream() << '"' << (((*yyvaluep).strVal) ? ((*yyvaluep).strVal)->ptr() : "(null)") << '"'; };
/* Line 423 of lalr1.cc  */
#line 253 "qlang-bison.cpp"
        break;
      case 69: /* "$NUM" */
/* Line 423 of lalr1.cc  */
#line 85 "qlang-bison.yy"
        { debug_stream() << ((*yyvaluep).intVal); };
/* Line 423 of lalr1.cc  */
#line 260 "qlang-bison.cpp"
        break;
      case 70: /* "$NAME" */
/* Line 423 of lalr1.cc  */
#line 86 "qlang-bison.yy"
        { debug_stream() << '"' << (((*yyvaluep).strVal) ? ((*yyvaluep).strVal)->ptr() : "(null)") << '"'; };
/* Line 423 of lalr1.cc  */
#line 267 "qlang-bison.cpp"
        break;
      case 72: /* input */
/* Line 423 of lalr1.cc  */
#line 96 "qlang-bison.yy"
        {
    if (((*yyvaluep).block)) {
        sStr s;
        ((*yyvaluep).block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 282 "qlang-bison.cpp"
        break;
      case 73: /* statements */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 298 "qlang-bison.cpp"
        break;
      case 74: /* non_return_statements */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 314 "qlang-bison.cpp"
        break;
      case 75: /* statements_with_return */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 330 "qlang-bison.cpp"
        break;
      case 76: /* statement_block */
/* Line 423 of lalr1.cc  */
#line 96 "qlang-bison.yy"
        {
    if (((*yyvaluep).block)) {
        sStr s;
        ((*yyvaluep).block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 345 "qlang-bison.cpp"
        break;
      case 77: /* lambda_block */
/* Line 423 of lalr1.cc  */
#line 96 "qlang-bison.yy"
        {
    if (((*yyvaluep).block)) {
        sStr s;
        ((*yyvaluep).block)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 360 "qlang-bison.cpp"
        break;
      case 78: /* namelist */
/* Line 423 of lalr1.cc  */
#line 86 "qlang-bison.yy"
        { debug_stream() << '"' << (((*yyvaluep).strVal) ? ((*yyvaluep).strVal)->ptr() : "(null)") << '"'; };
/* Line 423 of lalr1.cc  */
#line 367 "qlang-bison.cpp"
        break;
      case 79: /* lambda */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 382 "qlang-bison.cpp"
        break;
      case 80: /* statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 397 "qlang-bison.cpp"
        break;
      case 81: /* non_return_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 412 "qlang-bison.cpp"
        break;
      case 82: /* if_statement_start */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 427 "qlang-bison.cpp"
        break;
      case 83: /* if_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 442 "qlang-bison.cpp"
        break;
      case 84: /* for_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 457 "qlang-bison.cpp"
        break;
      case 85: /* while_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 472 "qlang-bison.cpp"
        break;
      case 86: /* return_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 487 "qlang-bison.cpp"
        break;
      case 87: /* break_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 502 "qlang-bison.cpp"
        break;
      case 88: /* continue_statement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 517 "qlang-bison.cpp"
        break;
      case 89: /* arglist */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 533 "qlang-bison.cpp"
        break;
      case 90: /* optional_arglist */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 549 "qlang-bison.cpp"
        break;
      case 91: /* expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 564 "qlang-bison.cpp"
        break;
      case 92: /* assignment_target */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 579 "qlang-bison.cpp"
        break;
      case 93: /* assignment_source */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 594 "qlang-bison.cpp"
        break;
      case 94: /* assignment */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 609 "qlang-bison.cpp"
        break;
      case 95: /* basic_expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 624 "qlang-bison.cpp"
        break;
      case 96: /* impure_expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 639 "qlang-bison.cpp"
        break;
      case 97: /* postfix_expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 654 "qlang-bison.cpp"
        break;
      case 98: /* slice */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 669 "qlang-bison.cpp"
        break;
      case 99: /* subscript */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 684 "qlang-bison.cpp"
        break;
      case 100: /* postcrement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 699 "qlang-bison.cpp"
        break;
      case 101: /* function_call */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 714 "qlang-bison.cpp"
        break;
      case 102: /* property_call */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 729 "qlang-bison.cpp"
        break;
      case 103: /* method_call */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 744 "qlang-bison.cpp"
        break;
      case 104: /* dollar_call */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 759 "qlang-bison.cpp"
        break;
      case 105: /* unary_expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 774 "qlang-bison.cpp"
        break;
      case 106: /* precrement */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 789 "qlang-bison.cpp"
        break;
      case 107: /* cast */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 804 "qlang-bison.cpp"
        break;
      case 108: /* multiplication_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 819 "qlang-bison.cpp"
        break;
      case 109: /* addition_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 834 "qlang-bison.cpp"
        break;
      case 110: /* comparable */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 849 "qlang-bison.cpp"
        break;
      case 111: /* format_call */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 864 "qlang-bison.cpp"
        break;
      case 112: /* conjunction_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 879 "qlang-bison.cpp"
        break;
      case 113: /* disjunction_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 894 "qlang-bison.cpp"
        break;
      case 114: /* ternary_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 909 "qlang-bison.cpp"
        break;
      case 115: /* comparison_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 924 "qlang-bison.cpp"
        break;
      case 116: /* equality_priority */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 939 "qlang-bison.cpp"
        break;
      case 117: /* non_assignment_expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 954 "qlang-bison.cpp"
        break;
      case 118: /* junction */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 969 "qlang-bison.cpp"
        break;
      case 119: /* literal */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 984 "qlang-bison.cpp"
        break;
      case 120: /* scalar_literal */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 999 "qlang-bison.cpp"
        break;
      case 121: /* list_literal */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1014 "qlang-bison.cpp"
        break;
      case 122: /* kv_key */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1029 "qlang-bison.cpp"
        break;
      case 123: /* kv_pairs */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1045 "qlang-bison.cpp"
        break;
      case 124: /* dic_literal */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1060 "qlang-bison.cpp"
        break;
      case 125: /* variable */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1075 "qlang-bison.cpp"
        break;
      case 126: /* template */
/* Line 423 of lalr1.cc  */
#line 105 "qlang-bison.yy"
        {
    if (((*yyvaluep).nodes)) {
        sStr s;
        for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
            (*((*yyvaluep).nodes))[i]->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1091 "qlang-bison.cpp"
        break;
      case 127: /* template_expression */
/* Line 423 of lalr1.cc  */
#line 87 "qlang-bison.yy"
        {
    if (((*yyvaluep).node)) {
        sStr s;
        ((*yyvaluep).node)->print(s);
        debug_stream() << s.ptr();
    } else {
        debug_stream() << "(null)";
    }
};
/* Line 423 of lalr1.cc  */
#line 1106 "qlang-bison.cpp"
        break;
       default:
	  break;
      }
  }


  void
  sQLangBison::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  sQLangBison::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
        case 17: /* "template literal substring" */
/* Line 455 of lalr1.cc  */
#line 76 "qlang-bison.yy"
        { delete ((*yyvaluep).strVal); };
/* Line 455 of lalr1.cc  */
#line 1144 "qlang-bison.cpp"
        break;
      case 18: /* "string literal" */
/* Line 455 of lalr1.cc  */
#line 76 "qlang-bison.yy"
        { delete ((*yyvaluep).strVal); };
/* Line 455 of lalr1.cc  */
#line 1151 "qlang-bison.cpp"
        break;
      case 19: /* "regex literal" */
/* Line 455 of lalr1.cc  */
#line 76 "qlang-bison.yy"
        { delete ((*yyvaluep).strVal); };
/* Line 455 of lalr1.cc  */
#line 1158 "qlang-bison.cpp"
        break;
      case 67: /* "name" */
/* Line 455 of lalr1.cc  */
#line 76 "qlang-bison.yy"
        { delete ((*yyvaluep).strVal); };
/* Line 455 of lalr1.cc  */
#line 1165 "qlang-bison.cpp"
        break;
      case 70: /* "$NAME" */
/* Line 455 of lalr1.cc  */
#line 76 "qlang-bison.yy"
        { delete ((*yyvaluep).strVal); };
/* Line 455 of lalr1.cc  */
#line 1172 "qlang-bison.cpp"
        break;
      case 72: /* input */
/* Line 455 of lalr1.cc  */
#line 78 "qlang-bison.yy"
        { delete ((*yyvaluep).block); };
/* Line 455 of lalr1.cc  */
#line 1179 "qlang-bison.cpp"
        break;
      case 73: /* statements */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1190 "qlang-bison.cpp"
        break;
      case 74: /* non_return_statements */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1201 "qlang-bison.cpp"
        break;
      case 75: /* statements_with_return */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1212 "qlang-bison.cpp"
        break;
      case 76: /* statement_block */
/* Line 455 of lalr1.cc  */
#line 78 "qlang-bison.yy"
        { delete ((*yyvaluep).block); };
/* Line 455 of lalr1.cc  */
#line 1219 "qlang-bison.cpp"
        break;
      case 77: /* lambda_block */
/* Line 455 of lalr1.cc  */
#line 78 "qlang-bison.yy"
        { delete ((*yyvaluep).block); };
/* Line 455 of lalr1.cc  */
#line 1226 "qlang-bison.cpp"
        break;
      case 78: /* namelist */
/* Line 455 of lalr1.cc  */
#line 76 "qlang-bison.yy"
        { delete ((*yyvaluep).strVal); };
/* Line 455 of lalr1.cc  */
#line 1233 "qlang-bison.cpp"
        break;
      case 79: /* lambda */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1240 "qlang-bison.cpp"
        break;
      case 80: /* statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1247 "qlang-bison.cpp"
        break;
      case 81: /* non_return_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1254 "qlang-bison.cpp"
        break;
      case 82: /* if_statement_start */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1261 "qlang-bison.cpp"
        break;
      case 83: /* if_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1268 "qlang-bison.cpp"
        break;
      case 84: /* for_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1275 "qlang-bison.cpp"
        break;
      case 85: /* while_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1282 "qlang-bison.cpp"
        break;
      case 86: /* return_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1289 "qlang-bison.cpp"
        break;
      case 87: /* break_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1296 "qlang-bison.cpp"
        break;
      case 88: /* continue_statement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1303 "qlang-bison.cpp"
        break;
      case 89: /* arglist */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1314 "qlang-bison.cpp"
        break;
      case 90: /* optional_arglist */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1325 "qlang-bison.cpp"
        break;
      case 91: /* expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1332 "qlang-bison.cpp"
        break;
      case 92: /* assignment_target */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1339 "qlang-bison.cpp"
        break;
      case 93: /* assignment_source */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1346 "qlang-bison.cpp"
        break;
      case 94: /* assignment */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1353 "qlang-bison.cpp"
        break;
      case 95: /* basic_expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1360 "qlang-bison.cpp"
        break;
      case 96: /* impure_expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1367 "qlang-bison.cpp"
        break;
      case 97: /* postfix_expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1374 "qlang-bison.cpp"
        break;
      case 98: /* slice */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1381 "qlang-bison.cpp"
        break;
      case 99: /* subscript */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1388 "qlang-bison.cpp"
        break;
      case 100: /* postcrement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1395 "qlang-bison.cpp"
        break;
      case 101: /* function_call */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1402 "qlang-bison.cpp"
        break;
      case 102: /* property_call */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1409 "qlang-bison.cpp"
        break;
      case 103: /* method_call */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1416 "qlang-bison.cpp"
        break;
      case 104: /* dollar_call */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1423 "qlang-bison.cpp"
        break;
      case 105: /* unary_expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1430 "qlang-bison.cpp"
        break;
      case 106: /* precrement */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1437 "qlang-bison.cpp"
        break;
      case 107: /* cast */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1444 "qlang-bison.cpp"
        break;
      case 108: /* multiplication_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1451 "qlang-bison.cpp"
        break;
      case 109: /* addition_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1458 "qlang-bison.cpp"
        break;
      case 110: /* comparable */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1465 "qlang-bison.cpp"
        break;
      case 111: /* format_call */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1472 "qlang-bison.cpp"
        break;
      case 112: /* conjunction_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1479 "qlang-bison.cpp"
        break;
      case 113: /* disjunction_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1486 "qlang-bison.cpp"
        break;
      case 114: /* ternary_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1493 "qlang-bison.cpp"
        break;
      case 115: /* comparison_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1500 "qlang-bison.cpp"
        break;
      case 116: /* equality_priority */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1507 "qlang-bison.cpp"
        break;
      case 117: /* non_assignment_expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1514 "qlang-bison.cpp"
        break;
      case 118: /* junction */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1521 "qlang-bison.cpp"
        break;
      case 119: /* literal */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1528 "qlang-bison.cpp"
        break;
      case 120: /* scalar_literal */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1535 "qlang-bison.cpp"
        break;
      case 121: /* list_literal */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1542 "qlang-bison.cpp"
        break;
      case 122: /* kv_key */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1549 "qlang-bison.cpp"
        break;
      case 123: /* kv_pairs */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1560 "qlang-bison.cpp"
        break;
      case 124: /* dic_literal */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1567 "qlang-bison.cpp"
        break;
      case 125: /* variable */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1574 "qlang-bison.cpp"
        break;
      case 126: /* template */
/* Line 455 of lalr1.cc  */
#line 79 "qlang-bison.yy"
        {
    for (idx i=0; i<((*yyvaluep).nodes)->dim(); i++)
        delete (*((*yyvaluep).nodes))[i];
    delete ((*yyvaluep).nodes);
};
/* Line 455 of lalr1.cc  */
#line 1585 "qlang-bison.cpp"
        break;
      case 127: /* template_expression */
/* Line 455 of lalr1.cc  */
#line 77 "qlang-bison.yy"
        { delete ((*yyvaluep).node); };
/* Line 455 of lalr1.cc  */
#line 1592 "qlang-bison.cpp"
        break;

	default:
	  break;
      }
  }

  void
  sQLangBison::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
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
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    // State.
    int yyn;
    int yylen = 0;
    int yystate = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    static semantic_type yyval_default;
    semantic_type yylval = yyval_default;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


/* User initialization code.  */
/* Line 545 of lalr1.cc  */
#line 60 "qlang-bison.yy"
{
//    @$.begin.filename = @$.end.filename = parser_driver.getFilename();
}
/* Line 545 of lalr1.cc  */
#line 1692 "qlang-bison.cpp"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
        YYCDEBUG << "Reading a token: ";
        yychar = yylex (&yylval, &yylloc, parser_driver, yyscanner);
      }

    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    // Compute the default @$.
    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }

    // Perform the reduction.
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
          case 2:
/* Line 670 of lalr1.cc  */
#line 237 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, (yyloc).begin.line, (yyloc).begin.column);
        ast::Return *ret = new ast::Return((yysemantic_stack_[(2) - (1)].node), (yyloc).begin.line, (yyloc).begin.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        (yyval.block) = NULL;
    }
    break;

  case 3:
/* Line 670 of lalr1.cc  */
#line 245 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(root, (yysemantic_stack_[(2) - (1)].nodes));
        delete (yysemantic_stack_[(2) - (1)].nodes);
        parser_driver.setAstRoot(root);
        (yyval.block) = NULL;
    }
    break;

  case 4:
/* Line 670 of lalr1.cc  */
#line 253 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(root, (yysemantic_stack_[(2) - (1)].nodes));
        delete (yysemantic_stack_[(2) - (1)].nodes);
        ast::Return * ret = new ast::Return(0, (yyloc).end.line, (yyloc).end.column);
        root->addElement(ret);
        parser_driver.setAstRoot(root);
        (yyval.block) = NULL;
    }
    break;

  case 5:
/* Line 670 of lalr1.cc  */
#line 263 "qlang-bison.yy"
    {
        ast::Block *root = new ast::Block(false, true, (yyloc).begin.line, (yyloc).begin.column);
        ast::FunctionCall *cat = new ast::FunctionCall(new ast::Variable("cat", (yyloc).begin.line, (yyloc).begin.column), (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(cat, (yysemantic_stack_[(2) - (1)].nodes));
        ast::Return *ret = new ast::Return(cat, (yyloc).begin.line, (yyloc).begin.column);
        root->addElement(ret);
        delete (yysemantic_stack_[(2) - (1)].nodes);
        parser_driver.setAstRoot(root);
        (yyval.block) = NULL;
    }
    break;

  case 6:
/* Line 670 of lalr1.cc  */
#line 277 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(1) - (1)].node));
    }
    break;

  case 7:
/* Line 670 of lalr1.cc  */
#line 282 "qlang-bison.yy"
    {
        (yyval.nodes) = (yysemantic_stack_[(2) - (1)].nodes);
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(2) - (2)].node));
    }
    break;

  case 8:
/* Line 670 of lalr1.cc  */
#line 290 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(1) - (1)].node));
    }
    break;

  case 9:
/* Line 670 of lalr1.cc  */
#line 295 "qlang-bison.yy"
    {
        (yyval.nodes) = (yysemantic_stack_[(2) - (1)].nodes);
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(2) - (2)].node));
    }
    break;

  case 10:
/* Line 670 of lalr1.cc  */
#line 303 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(1) - (1)].node));
    }
    break;

  case 11:
/* Line 670 of lalr1.cc  */
#line 308 "qlang-bison.yy"
    {
        (yyval.nodes) = (yysemantic_stack_[(2) - (1)].nodes);
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(2) - (2)].node));
    }
    break;

  case 12:
/* Line 670 of lalr1.cc  */
#line 316 "qlang-bison.yy"
    {
        (yyval.block) = new ast::Block(true, false, (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS((yyval.block), (yysemantic_stack_[(3) - (2)].nodes));
        delete (yysemantic_stack_[(3) - (2)].nodes);
    }
    break;

  case 13:
/* Line 670 of lalr1.cc  */
#line 325 "qlang-bison.yy"
    {
        (yyval.block) = new ast::Block(false, false, (yyloc).begin.line, (yyloc).begin.column);
        ast::Return *r = new ast::Return((yysemantic_stack_[(3) - (2)].node), (yylocation_stack_[(3) - (1)]).begin.line, (yylocation_stack_[(3) - (1)]).begin.column);
        (yyval.block)->addElement(r);
    }
    break;

  case 14:
/* Line 670 of lalr1.cc  */
#line 331 "qlang-bison.yy"
    {
        (yyval.block) = new ast::Block(false, false, (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS((yyval.block), (yysemantic_stack_[(3) - (2)].nodes));
        delete (yysemantic_stack_[(3) - (2)].nodes);
    }
    break;

  case 15:
/* Line 670 of lalr1.cc  */
#line 337 "qlang-bison.yy"
    {
        (yyval.block) = new ast::Block(false, false, (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS((yyval.block), (yysemantic_stack_[(3) - (2)].nodes));
        delete (yysemantic_stack_[(3) - (2)].nodes);
        ast::Return * r = new ast::Return(0, (yyloc).end.line, (yyloc).end.column);
        (yyval.block)->addElement(r);
    }
    break;

  case 16:
/* Line 670 of lalr1.cc  */
#line 348 "qlang-bison.yy"
    {
        (yyval.strVal) = (yysemantic_stack_[(1) - (1)].strVal);
        (yyval.strVal)->add0();
    }
    break;

  case 17:
/* Line 670 of lalr1.cc  */
#line 353 "qlang-bison.yy"
    {
        (yyval.strVal) = (yysemantic_stack_[(3) - (1)].strVal);
        (yyval.strVal)->add((yysemantic_stack_[(3) - (3)].strVal)->ptr());
        delete (yysemantic_stack_[(3) - (3)].strVal);
    }
    break;

  case 18:
/* Line 670 of lalr1.cc  */
#line 362 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Lambda(NULL, (yysemantic_stack_[(1) - (1)].block), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 19:
/* Line 670 of lalr1.cc  */
#line 366 "qlang-bison.yy"
    {
        (yysemantic_stack_[(5) - (3)].strVal)->add0();
        (yyval.node) = new ast::Lambda((yysemantic_stack_[(5) - (3)].strVal)->ptr(), (yysemantic_stack_[(5) - (5)].block), (yyloc).begin.line, (yyloc).begin.column);
        delete (yysemantic_stack_[(5) - (3)].strVal);
    }
    break;

  case 27:
/* Line 670 of lalr1.cc  */
#line 385 "qlang-bison.yy"
    {
        (yyval.node) = (yysemantic_stack_[(2) - (1)].node);
    }
    break;

  case 28:
/* Line 670 of lalr1.cc  */
#line 392 "qlang-bison.yy"
    {
        (yyval.node) = new ast::If((yysemantic_stack_[(5) - (3)].node), (yysemantic_stack_[(5) - (5)].block), NULL, (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 29:
/* Line 670 of lalr1.cc  */
#line 396 "qlang-bison.yy"
    {
        (yyval.node) = (yysemantic_stack_[(7) - (1)].node);
        ast::If *chain = new ast::If((yysemantic_stack_[(7) - (5)].node), (yysemantic_stack_[(7) - (7)].block), NULL, (yylocation_stack_[(7) - (3)]).begin.line, (yylocation_stack_[(7) - (3)]).begin.column);
        dynamic_cast<ast::If*>((yyval.node))->setLastElse(chain);
    }
    break;

  case 31:
/* Line 670 of lalr1.cc  */
#line 406 "qlang-bison.yy"
    {
        (yyval.node) = (yysemantic_stack_[(3) - (1)].node);
        dynamic_cast<ast::If*>((yyval.node))->setLastElse((yysemantic_stack_[(3) - (3)].block));
    }
    break;

  case 32:
/* Line 670 of lalr1.cc  */
#line 414 "qlang-bison.yy"
    {
        ast::Block *init = new ast::Block(false, false, (yylocation_stack_[(9) - (3)]).begin.line, (yylocation_stack_[(9) - (3)]).begin.column);
        ADD_ELEMENTS(init, (yysemantic_stack_[(9) - (3)].nodes));
        ast::Block *cond = new ast::Block(false, false, (yylocation_stack_[(9) - (5)]).begin.line, (yylocation_stack_[(9) - (5)]).begin.column);
        ADD_ELEMENTS(cond, (yysemantic_stack_[(9) - (5)].nodes));
        ast::Block *step = new ast::Block(false, false, (yylocation_stack_[(9) - (7)]).begin.line, (yylocation_stack_[(9) - (7)]).begin.column);
        ADD_ELEMENTS(step, (yysemantic_stack_[(9) - (7)].nodes));
        delete (yysemantic_stack_[(9) - (3)].nodes);
        delete (yysemantic_stack_[(9) - (5)].nodes);
        delete (yysemantic_stack_[(9) - (7)].nodes);
        (yyval.node) = new ast::For(init, cond, step, (yysemantic_stack_[(9) - (9)].block), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 33:
/* Line 670 of lalr1.cc  */
#line 430 "qlang-bison.yy"
    {
        (yyval.node) = new ast::While((yysemantic_stack_[(5) - (3)].node), (yysemantic_stack_[(5) - (5)].block), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 34:
/* Line 670 of lalr1.cc  */
#line 437 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Return(0, (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 35:
/* Line 670 of lalr1.cc  */
#line 441 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Return((yysemantic_stack_[(3) - (2)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 36:
/* Line 670 of lalr1.cc  */
#line 448 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Break((yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 37:
/* Line 670 of lalr1.cc  */
#line 455 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Continue((yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 38:
/* Line 670 of lalr1.cc  */
#line 462 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(1) - (1)].node));
    }
    break;

  case 39:
/* Line 670 of lalr1.cc  */
#line 467 "qlang-bison.yy"
    {
        (yyval.nodes) = (yysemantic_stack_[(3) - (1)].nodes);
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(3) - (3)].node));
    }
    break;

  case 40:
/* Line 670 of lalr1.cc  */
#line 475 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
    }
    break;

  case 46:
/* Line 670 of lalr1.cc  */
#line 490 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Subscript((yysemantic_stack_[(4) - (1)].node), (yysemantic_stack_[(4) - (3)].node), (yylocation_stack_[(4) - (2)]).begin.line, (yylocation_stack_[(4) - (2)]).begin.column);
    }
    break;

  case 49:
/* Line 670 of lalr1.cc  */
#line 502 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Assign((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 50:
/* Line 670 of lalr1.cc  */
#line 506 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ArithmeticInplace((yysemantic_stack_[(3) - (1)].node), '+', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 51:
/* Line 670 of lalr1.cc  */
#line 510 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ArithmeticInplace((yysemantic_stack_[(3) - (1)].node), '-', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 52:
/* Line 670 of lalr1.cc  */
#line 514 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ArithmeticInplace((yysemantic_stack_[(3) - (1)].node), '*', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 53:
/* Line 670 of lalr1.cc  */
#line 518 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ArithmeticInplace((yysemantic_stack_[(3) - (1)].node), '/', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 54:
/* Line 670 of lalr1.cc  */
#line 522 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ArithmeticInplace((yysemantic_stack_[(3) - (1)].node), '%', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 58:
/* Line 670 of lalr1.cc  */
#line 532 "qlang-bison.yy"
    {
        (yyval.node) = (yysemantic_stack_[(3) - (2)].node);
    }
    break;

  case 72:
/* Line 670 of lalr1.cc  */
#line 558 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Slice((yysemantic_stack_[(6) - (1)].node), (yysemantic_stack_[(6) - (3)].node), (yysemantic_stack_[(6) - (5)].node), (yylocation_stack_[(6) - (2)]).begin.line, (yylocation_stack_[(6) - (2)]).begin.column);
    }
    break;

  case 73:
/* Line 670 of lalr1.cc  */
#line 565 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Subscript((yysemantic_stack_[(4) - (1)].node), (yysemantic_stack_[(4) - (3)].node), (yylocation_stack_[(4) - (2)]).begin.line, (yylocation_stack_[(4) - (2)]).begin.column);
    }
    break;

  case 74:
/* Line 670 of lalr1.cc  */
#line 572 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Postcrement((yysemantic_stack_[(2) - (1)].node), '+', (yylocation_stack_[(2) - (2)]).begin.line, (yylocation_stack_[(2) - (2)]).begin.column);
    }
    break;

  case 75:
/* Line 670 of lalr1.cc  */
#line 576 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Postcrement((yysemantic_stack_[(2) - (1)].node), '-', (yylocation_stack_[(2) - (2)]).begin.line, (yylocation_stack_[(2) - (2)]).begin.column);
    }
    break;

  case 76:
/* Line 670 of lalr1.cc  */
#line 583 "qlang-bison.yy"
    {
        ast::FunctionCall *fcall = new ast::FunctionCall((yysemantic_stack_[(4) - (1)].node), (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(fcall, (yysemantic_stack_[(4) - (3)].nodes));
        (yyval.node) = fcall;
        delete (yysemantic_stack_[(4) - (3)].nodes);
    }
    break;

  case 77:
/* Line 670 of lalr1.cc  */
#line 593 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Property(NULL, (yysemantic_stack_[(2) - (2)].strVal)->ptr(), (yylocation_stack_[(2) - (2)]).begin.line, (yylocation_stack_[(2) - (2)]).begin.column);
        delete (yysemantic_stack_[(2) - (2)].strVal);
    }
    break;

  case 78:
/* Line 670 of lalr1.cc  */
#line 598 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Property((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].strVal)->ptr(), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
        delete (yysemantic_stack_[(3) - (3)].strVal);
    }
    break;

  case 79:
/* Line 670 of lalr1.cc  */
#line 606 "qlang-bison.yy"
    {
        ast::MethodCall *mcall = new ast::MethodCall(NULL, (yysemantic_stack_[(5) - (2)].node), (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(mcall, (yysemantic_stack_[(5) - (4)].nodes));
        (yyval.node) = mcall;
        delete (yysemantic_stack_[(5) - (4)].nodes);
    }
    break;

  case 80:
/* Line 670 of lalr1.cc  */
#line 613 "qlang-bison.yy"
    {
        ast::MethodCall *mcall = new ast::MethodCall((yysemantic_stack_[(6) - (1)].node), (yysemantic_stack_[(6) - (3)].node), (yylocation_stack_[(6) - (2)]).begin.line, (yylocation_stack_[(6) - (2)]).begin.column);
        ADD_ELEMENTS(mcall, (yysemantic_stack_[(6) - (5)].nodes));
        (yyval.node) = mcall;
        delete (yysemantic_stack_[(6) - (5)].nodes);
    }
    break;

  case 81:
/* Line 670 of lalr1.cc  */
#line 623 "qlang-bison.yy"
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error((yyloc), "$NUM calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall((yyloc).begin.line, (yyloc).begin.column);
        dcall->setNum((yysemantic_stack_[(1) - (1)].intVal));
        (yyval.node) = dcall;
    }
    break;

  case 82:
/* Line 670 of lalr1.cc  */
#line 634 "qlang-bison.yy"
    {
        if (!(parser_driver.getFlags() & slib::qlang::Parser::fDollarValues)) {
            error((yyloc), "$NAME calls not allowed in this parser mode");
            YYERROR;
        }

        ast::DollarCall *dcall = new ast::DollarCall((yyloc).begin.line, (yyloc).begin.column);
        dcall->borrowNameFrom(*(yysemantic_stack_[(1) - (1)].strVal));
        delete (yysemantic_stack_[(1) - (1)].strVal);
        (yyval.node) = dcall;
    }
    break;

  case 86:
/* Line 670 of lalr1.cc  */
#line 652 "qlang-bison.yy"
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>((yysemantic_stack_[(2) - (2)].node));
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->getValue() *= (idx)(-1);
            scalar->setLocation((yyloc).begin.line, (yyloc).begin.column);
            (yyval.node) = scalar;
        } else {
            (yyval.node) = new ast::UnaryPlusMinus('-', (yysemantic_stack_[(2) - (2)].node), (yyloc).begin.line, (yyloc).begin.column);
        }
    }
    break;

  case 87:
/* Line 670 of lalr1.cc  */
#line 663 "qlang-bison.yy"
    {
        ast::ScalarLiteral* scalar = dynamic_cast<ast::ScalarLiteral*>((yysemantic_stack_[(2) - (2)].node));
        if (scalar && scalar->getValue().isNumeric()) {
            scalar->setLocation((yyloc).begin.line, (yyloc).begin.column);
            (yyval.node) = scalar;
        } else {
            (yyval.node) = new ast::UnaryPlusMinus('+', (yysemantic_stack_[(2) - (2)].node), (yyloc).begin.line, (yyloc).begin.column);
        }
    }
    break;

  case 88:
/* Line 670 of lalr1.cc  */
#line 673 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Not((yysemantic_stack_[(2) - (2)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 89:
/* Line 670 of lalr1.cc  */
#line 680 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Precrement((yysemantic_stack_[(2) - (2)].node), '+', (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 90:
/* Line 670 of lalr1.cc  */
#line 684 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Precrement((yysemantic_stack_[(2) - (2)].node), '-', (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 91:
/* Line 670 of lalr1.cc  */
#line 691 "qlang-bison.yy"
    {
        (yyval.node) = new ast::BoolCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 92:
/* Line 670 of lalr1.cc  */
#line 695 "qlang-bison.yy"
    {
        (yyval.node) = new ast::IntCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 93:
/* Line 670 of lalr1.cc  */
#line 699 "qlang-bison.yy"
    {
        (yyval.node) = new ast::UIntCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 94:
/* Line 670 of lalr1.cc  */
#line 703 "qlang-bison.yy"
    {
        (yyval.node) = new ast::IntlistCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 95:
/* Line 670 of lalr1.cc  */
#line 707 "qlang-bison.yy"
    {
        (yyval.node) = new ast::RealCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 96:
/* Line 670 of lalr1.cc  */
#line 711 "qlang-bison.yy"
    {
        (yyval.node) = new ast::StringCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 97:
/* Line 670 of lalr1.cc  */
#line 715 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ObjCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 98:
/* Line 670 of lalr1.cc  */
#line 719 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ObjlistCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 99:
/* Line 670 of lalr1.cc  */
#line 723 "qlang-bison.yy"
    {
        (yyval.node) = new ast::DateTimeCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 100:
/* Line 670 of lalr1.cc  */
#line 727 "qlang-bison.yy"
    {
        (yyval.node) = new ast::DateCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 101:
/* Line 670 of lalr1.cc  */
#line 731 "qlang-bison.yy"
    {
        (yyval.node) = new ast::TimeCast((yysemantic_stack_[(4) - (4)].node), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 103:
/* Line 670 of lalr1.cc  */
#line 739 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Arithmetic((yysemantic_stack_[(3) - (1)].node), '*', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 104:
/* Line 670 of lalr1.cc  */
#line 743 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Arithmetic((yysemantic_stack_[(3) - (1)].node), '/', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 105:
/* Line 670 of lalr1.cc  */
#line 747 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Arithmetic((yysemantic_stack_[(3) - (1)].node), '%', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 107:
/* Line 670 of lalr1.cc  */
#line 755 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Arithmetic((yysemantic_stack_[(3) - (1)].node), '+', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 108:
/* Line 670 of lalr1.cc  */
#line 759 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Arithmetic((yysemantic_stack_[(3) - (1)].node), '-', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 110:
/* Line 670 of lalr1.cc  */
#line 767 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Has((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 111:
/* Line 670 of lalr1.cc  */
#line 771 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Has((yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 112:
/* Line 670 of lalr1.cc  */
#line 775 "qlang-bison.yy"
    {
        regex_t re;
        const char * flag_string = sString::next00((yysemantic_stack_[(3) - (3)].strVal)->ptr());
        int flags = REG_EXTENDED;
        if (flag_string && strchr(flag_string, 'i')) {
            flags |= REG_ICASE;
        }
        if (regcomp(&re, (yysemantic_stack_[(3) - (3)].strVal)->ptr(), flags)) {
            error((yylocation_stack_[(3) - (3)]), "Invalid regular expression");
            delete (yysemantic_stack_[(3) - (3)].strVal);
            YYERROR;
        }
        (yyval.node) = new ast::Match((yysemantic_stack_[(3) - (1)].node), '=', &re, (yysemantic_stack_[(3) - (3)].strVal)->ptr(), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
        delete (yysemantic_stack_[(3) - (3)].strVal);
    }
    break;

  case 113:
/* Line 670 of lalr1.cc  */
#line 791 "qlang-bison.yy"
    {
        regex_t re;
        const char * flag_string = sString::next00((yysemantic_stack_[(3) - (3)].strVal)->ptr());
        int flags = REG_EXTENDED;
        if (flag_string && strchr(flag_string, 'i')) {
            flags |= REG_ICASE;
        }
        if (regcomp(&re, (yysemantic_stack_[(3) - (3)].strVal)->ptr(), flags)) {
            error((yylocation_stack_[(3) - (3)]), "Invalid regular expression");
            delete (yysemantic_stack_[(3) - (3)].strVal);
            YYERROR;
        }
        (yyval.node) = new ast::Match((yysemantic_stack_[(3) - (1)].node), '!', &re, (yysemantic_stack_[(3) - (3)].strVal)->ptr(), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
        delete (yysemantic_stack_[(3) - (3)].strVal);
    }
    break;

  case 114:
/* Line 670 of lalr1.cc  */
#line 807 "qlang-bison.yy"
    {
        (yyval.node) = new ast::BoolCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 115:
/* Line 670 of lalr1.cc  */
#line 811 "qlang-bison.yy"
    {
        (yyval.node) = new ast::IntCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 116:
/* Line 670 of lalr1.cc  */
#line 815 "qlang-bison.yy"
    {
        (yyval.node) = new ast::UIntCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 117:
/* Line 670 of lalr1.cc  */
#line 819 "qlang-bison.yy"
    {
        (yyval.node) = new ast::IntlistCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 118:
/* Line 670 of lalr1.cc  */
#line 823 "qlang-bison.yy"
    {
        (yyval.node) = new ast::RealCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 119:
/* Line 670 of lalr1.cc  */
#line 827 "qlang-bison.yy"
    {
        (yyval.node) = new ast::StringCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 120:
/* Line 670 of lalr1.cc  */
#line 831 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ObjCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 121:
/* Line 670 of lalr1.cc  */
#line 835 "qlang-bison.yy"
    {
        (yyval.node) = new ast::ObjlistCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 122:
/* Line 670 of lalr1.cc  */
#line 839 "qlang-bison.yy"
    {
        (yyval.node) = new ast::DateTimeCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 123:
/* Line 670 of lalr1.cc  */
#line 843 "qlang-bison.yy"
    {
        (yyval.node) = new ast::DateCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 124:
/* Line 670 of lalr1.cc  */
#line 847 "qlang-bison.yy"
    {
        (yyval.node) = new ast::TimeCast((yysemantic_stack_[(3) - (1)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 126:
/* Line 670 of lalr1.cc  */
#line 855 "qlang-bison.yy"
    {
        ast::FormatCall *fcall = new ast::FormatCall((yysemantic_stack_[(6) - (1)].node), (yysemantic_stack_[(6) - (3)].node), (yylocation_stack_[(6) - (3)]).begin.line, (yylocation_stack_[(6) - (3)]).begin.column);
        ADD_ELEMENTS(fcall, (yysemantic_stack_[(6) - (5)].nodes));
        (yyval.node) = fcall;
        delete (yysemantic_stack_[(6) - (5)].nodes);
    }
    break;

  case 128:
/* Line 670 of lalr1.cc  */
#line 866 "qlang-bison.yy"
    {
        (yyval.node) = new ast::BinaryLogic((yysemantic_stack_[(3) - (1)].node), '&', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 130:
/* Line 670 of lalr1.cc  */
#line 874 "qlang-bison.yy"
    {
        (yyval.node) = new ast::BinaryLogic((yysemantic_stack_[(3) - (1)].node), '|', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 132:
/* Line 670 of lalr1.cc  */
#line 882 "qlang-bison.yy"
    {
        (yyval.node) = new ast::TernaryConditional((yysemantic_stack_[(5) - (1)].node), (yysemantic_stack_[(5) - (3)].node), (yysemantic_stack_[(5) - (5)].node), (yylocation_stack_[(5) - (2)]).begin.line, (yylocation_stack_[(5) - (2)]).begin.column);
    }
    break;

  case 134:
/* Line 670 of lalr1.cc  */
#line 890 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Comparison((yysemantic_stack_[(3) - (1)].node), "<", (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 135:
/* Line 670 of lalr1.cc  */
#line 894 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Comparison((yysemantic_stack_[(3) - (1)].node), ">", (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 136:
/* Line 670 of lalr1.cc  */
#line 898 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Comparison((yysemantic_stack_[(3) - (1)].node), "<=", (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 137:
/* Line 670 of lalr1.cc  */
#line 902 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Comparison((yysemantic_stack_[(3) - (1)].node), ">=", (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 138:
/* Line 670 of lalr1.cc  */
#line 906 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Comparison((yysemantic_stack_[(3) - (1)].node), "<=>", (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 140:
/* Line 670 of lalr1.cc  */
#line 914 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Equality((yysemantic_stack_[(3) - (1)].node), '=', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 141:
/* Line 670 of lalr1.cc  */
#line 918 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Equality((yysemantic_stack_[(3) - (1)].node), '=', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 142:
/* Line 670 of lalr1.cc  */
#line 922 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Equality((yysemantic_stack_[(3) - (1)].node), '!', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 143:
/* Line 670 of lalr1.cc  */
#line 926 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Equality((yysemantic_stack_[(3) - (1)].node), '!', (yysemantic_stack_[(3) - (3)].node), (yylocation_stack_[(3) - (2)]).begin.line, (yylocation_stack_[(3) - (2)]).begin.column);
    }
    break;

  case 145:
/* Line 670 of lalr1.cc  */
#line 937 "qlang-bison.yy"
    {
        ast::Junction *junc = new ast::Junction((yyloc).begin.line, (yyloc).begin.column);
        junc->addElement((yysemantic_stack_[(3) - (1)].node));
        junc->addElement((yysemantic_stack_[(3) - (3)].node));
        (yyval.node) = junc;
    }
    break;

  case 146:
/* Line 670 of lalr1.cc  */
#line 944 "qlang-bison.yy"
    {
        (yyval.node) = (yysemantic_stack_[(3) - (1)].node);
        dynamic_cast<ast::Junction*>((yyval.node))->addElement((yysemantic_stack_[(3) - (3)].node));
    }
    break;

  case 150:
/* Line 670 of lalr1.cc  */
#line 958 "qlang-bison.yy"
    {
        (yyval.node) = new ast::IntLiteral((yysemantic_stack_[(1) - (1)].intVal), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 151:
/* Line 670 of lalr1.cc  */
#line 962 "qlang-bison.yy"
    {
        (yyval.node) = new ast::RealLiteral((yysemantic_stack_[(1) - (1)].realVal), (yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 152:
/* Line 670 of lalr1.cc  */
#line 966 "qlang-bison.yy"
    {
        (yyval.node) = new ast::StringLiteral((yysemantic_stack_[(1) - (1)].strVal)->ptr(), (yyloc).begin.line, (yyloc).begin.column);
        delete (yysemantic_stack_[(1) - (1)].strVal);
    }
    break;

  case 153:
/* Line 670 of lalr1.cc  */
#line 971 "qlang-bison.yy"
    {
        (yyval.node) = new ast::NullLiteral((yyloc).begin.line, (yyloc).begin.column);
    }
    break;

  case 154:
/* Line 670 of lalr1.cc  */
#line 978 "qlang-bison.yy"
    {
        ast::ListLiteral *llit = new ast::ListLiteral((yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(llit, (yysemantic_stack_[(3) - (2)].nodes));
        (yyval.node) = llit;
        delete (yysemantic_stack_[(3) - (2)].nodes);
    }
    break;

  case 155:
/* Line 670 of lalr1.cc  */
#line 988 "qlang-bison.yy"
    {
        (yyval.node) = new ast::StringLiteral((yysemantic_stack_[(1) - (1)].strVal)->ptr(), (yyloc).begin.line, (yyloc).begin.column);
        delete (yysemantic_stack_[(1) - (1)].strVal);
    }
    break;

  case 156:
/* Line 670 of lalr1.cc  */
#line 993 "qlang-bison.yy"
    {
        (yyval.node) = new ast::StringLiteral((yysemantic_stack_[(1) - (1)].strVal)->ptr(), (yyloc).begin.line, (yyloc).begin.column);
        delete (yysemantic_stack_[(1) - (1)].strVal);
    }
    break;

  case 157:
/* Line 670 of lalr1.cc  */
#line 1001 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
        (yyval.nodes)->vadd(2, (yysemantic_stack_[(3) - (1)].node), (yysemantic_stack_[(3) - (3)].node));
    }
    break;

  case 158:
/* Line 670 of lalr1.cc  */
#line 1006 "qlang-bison.yy"
    {
        (yyval.nodes) = (yysemantic_stack_[(5) - (1)].nodes);
        (yyval.nodes)->vadd(2, (yysemantic_stack_[(5) - (3)].node), (yysemantic_stack_[(5) - (5)].node));
    }
    break;

  case 159:
/* Line 670 of lalr1.cc  */
#line 1014 "qlang-bison.yy"
    {
        ast::DicLiteral *dlit = new ast::DicLiteral((yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(dlit, (yysemantic_stack_[(3) - (2)].nodes));
        (yyval.node) = dlit;
        delete (yysemantic_stack_[(3) - (2)].nodes);
    }
    break;

  case 160:
/* Line 670 of lalr1.cc  */
#line 1024 "qlang-bison.yy"
    {
        (yyval.node) = new ast::Variable((yysemantic_stack_[(1) - (1)].strVal)->ptr(), (yyloc).begin.line, (yyloc).begin.column);
        delete (yysemantic_stack_[(1) - (1)].strVal);
    }
    break;

  case 161:
/* Line 670 of lalr1.cc  */
#line 1032 "qlang-bison.yy"
    {
        (yyval.nodes) = new sVec<ast::Node*>;
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(1) - (1)].node));
    }
    break;

  case 162:
/* Line 670 of lalr1.cc  */
#line 1037 "qlang-bison.yy"
    {
        (yyval.nodes) = (yysemantic_stack_[(2) - (1)].nodes);
        (yyval.nodes)->vadd(1, (yysemantic_stack_[(2) - (2)].node));
    }
    break;

  case 163:
/* Line 670 of lalr1.cc  */
#line 1045 "qlang-bison.yy"
    {
        (yyval.node) = new ast::StringLiteral((yysemantic_stack_[(1) - (1)].strVal)->ptr(), (yyloc).begin.line, (yyloc).begin.column);
        delete (yysemantic_stack_[(1) - (1)].strVal);
    }
    break;

  case 164:
/* Line 670 of lalr1.cc  */
#line 1050 "qlang-bison.yy"
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, (yyloc).begin.line, (yyloc).begin.column);
        block->addElement((yysemantic_stack_[(3) - (2)].node));
        (yyval.node) = block;
        parser_driver.yyPopStateUntilTemplate(); // switch scanner state back to template
    }
    break;

  case 165:
/* Line 670 of lalr1.cc  */
#line 1057 "qlang-bison.yy"
    {
        ast::UnbreakableBlock *block = new ast::UnbreakableBlock(false, true, (yyloc).begin.line, (yyloc).begin.column);
        ADD_ELEMENTS(block, (yysemantic_stack_[(3) - (2)].nodes));
        delete (yysemantic_stack_[(3) - (2)].nodes);
        (yyval.node) = block;
        parser_driver.yyPopStateUntilTemplate(); // switch scanner state back to template
    }
    break;


/* Line 670 of lalr1.cc  */
#line 2896 "qlang-bison.cpp"
      default:
        break;
      }

    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */
        if (yychar <= yyeof_)
          {
            /* Return failure if at end of input.  */
            if (yychar == yyeof_)
              YYABORT;
          }
        else
          {
            yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
            yychar = yyempty_;
          }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
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

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
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

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	  YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystate_stack_.height ())
      {
        yydestruct_ ("Cleanup: popping",
                     yystos_[yystate_stack_[0]],
                     &yysemantic_stack_[0],
                     &yylocation_stack_[0]);
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
        if (yychar != yyempty_)
          {
            /* Make sure we have latest lookahead translation.  See
               comments at user semantic actions for why this is
               necessary.  */
            yytoken = yytranslate_ (yychar);
            yydestruct_ (YY_NULL, yytoken, &yylval, &yylloc);
          }

        while (1 < yystate_stack_.height ())
          {
            yydestruct_ (YY_NULL,
                         yystos_[yystate_stack_[0]],
                         &yysemantic_stack_[0],
                         &yylocation_stack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  // Generate an error message.
  std::string
  sQLangBison::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
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
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
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
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
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

    char const* yyformat = YY_NULL;
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


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const short int sQLangBison::yypact_ninf_ = -257;
  const short int
  sQLangBison::yypact_[] =
  {
       498,   472,   896,   554,   361,  -257,  -257,  -257,  -257,  -257,
     896,   896,   896,   896,   896,     8,    11,    22,    49,    67,
      25,   870,  -257,   610,  -257,  -257,    71,   397,    82,  -257,
    -257,  -257,    37,  -257,  -257,  -257,  -257,  -257,  -257,   128,
    -257,    85,    93,    58,  -257,  -257,   103,   106,   195,   109,
    -257,   -26,   115,  -257,    94,    13,  -257,  -257,   104,    16,
    -257,    59,    41,   151,  -257,  -257,  -257,  -257,   374,     4,
    -257,   149,   152,   154,   156,   160,   162,   164,   170,   171,
     173,   174,   175,  -257,  -257,  -257,  -257,  -257,  -257,   153,
     176,  -257,   169,   172,   702,   178,   179,   190,   177,   138,
     896,   198,   202,  -257,    64,  -257,  -257,  -257,  -257,  -257,
    -257,   896,   896,   896,  -257,  -257,   126,  -257,   194,   814,
     208,   209,  -257,  -257,  -257,  -257,    58,  -257,  -257,    -2,
     896,   896,   896,   896,   896,   896,   896,  -257,   896,   578,
    -257,  -257,   210,   212,   896,   896,   896,   896,   896,   896,
     634,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,  -257,  -257,  -257,   666,   666,   666,   666,   666,   666,
     666,   666,   666,   666,   666,  -257,   896,  -257,  -257,  -257,
    -257,   896,  -257,   -11,   896,   896,   213,   215,   217,  -257,
      70,  -257,  -257,  -257,   814,   211,  -257,  -257,  -257,  -257,
    -257,  -257,  -257,  -257,  -257,   228,    53,   198,   230,  -257,
    -257,   221,   223,  -257,  -257,  -257,  -257,    94,    94,  -257,
    -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,
     235,    41,     7,   104,  -257,  -257,  -257,  -257,  -257,     6,
      59,   223,    59,   223,  -257,  -257,  -257,  -257,  -257,  -257,
    -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,   246,
     242,    72,   252,   896,   252,   253,   199,   758,  -257,  -257,
    -257,   896,  -257,   432,   896,   896,   896,   896,   896,   896,
     896,  -257,  -257,  -257,   249,  -257,   610,  -257,  -257,  -257,
    -257,   261,   262,   265,  -257,  -257,   266,  -257,  -257,   896,
     252,  -257,  -257,  -257,   268,  -257,   252,  -257
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  sQLangBison::yydefact_[] =
  {
         0,     0,    40,     0,     0,   150,   151,   163,   152,   153,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   160,     0,    81,    82,     0,     0,     0,    18,
      57,     8,    30,    22,    23,    24,    10,    25,    26,     0,
      62,    65,     0,    83,    66,    67,    68,    71,    69,    70,
      59,   102,    84,    85,   106,   109,   133,   125,   129,   131,
     144,   139,   127,     0,    55,   147,   148,   149,    56,     0,
     161,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    43,    68,    71,    70,    84,    42,    41,
       0,    38,   152,   160,     0,     0,     0,    43,     0,     0,
       0,    77,     0,    56,    83,    69,    87,    89,    86,    90,
      88,     0,    40,     0,    36,    37,     0,    34,     0,     0,
       0,     0,     1,     4,     9,    11,     0,    61,     3,     0,
       0,     0,     0,     0,     0,     0,    40,    27,     0,     0,
      74,    75,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     5,   162,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    58,     0,   154,    15,    14,
      13,     0,   159,     0,    40,     0,     0,     0,     0,    16,
       0,    35,   165,   164,     0,     0,    31,    49,    47,    48,
      50,    51,    52,    53,    54,     0,     0,    78,     0,   112,
     113,   110,   111,   103,   104,   105,   102,   107,   108,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
       0,   128,     0,   130,   135,   134,   137,   136,   138,   102,
     140,   141,   142,   143,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,    39,   157,   156,   155,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     6,    20,
      21,     0,    76,    73,     0,    40,     0,     0,    40,     0,
       0,    79,    73,    28,     0,    33,     0,    19,    17,    12,
       7,     0,     0,     0,   145,   146,     0,   132,   158,    40,
       0,    72,    80,   126,     0,    29,     0,    32
  };

  /* YYPGOTO[NTERM-NUM].  */
  const short int
  sQLangBison::yypgoto_[] =
  {
      -257,  -257,  -257,    17,    39,  -256,    14,  -257,  -257,    18,
      -5,  -257,  -257,  -257,  -257,    -3,  -257,  -257,  -257,   146,
      54,  -257,   189,     9,   219,  -257,    96,  -257,  -257,    10,
      26,   197,    31,  -257,   159,    43,  -257,  -107,  -257,   181,
    -257,   120,   129,     5,   -67,   132,    65,   -58,  -257,  -257,
    -257,   105,  -257,  -257,     0,  -257,   218
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const short int
  sQLangBison::yydefgoto_[] =
  {
        -1,    26,   267,    94,    95,   196,    29,   190,    30,   268,
      31,    32,    33,    34,    35,    36,    37,    38,    89,    90,
      91,    39,   197,    83,    41,    42,    43,    44,    45,    84,
      85,    48,    86,    50,    51,    87,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    88,   212,    64,    65,
      66,    98,    99,    67,   103,    69,    70
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const short int sQLangBison::yytable_ninf_ = -161;
  const short int
  sQLangBison::yytable_[] =
  {
        68,    68,    68,    68,   162,   194,   283,   257,   285,    40,
      46,   111,    97,    46,   112,   142,   143,    27,   279,   276,
     144,     7,   124,    68,   125,   113,    47,    68,   116,    47,
     152,    49,    97,    46,    49,   148,    40,    46,   149,    28,
     119,   217,   218,    52,   305,   195,    52,   142,   143,    47,
     307,   153,   144,    47,    49,    82,   258,    96,    49,   273,
     153,   114,   120,   138,   274,    63,    52,   139,   150,   185,
     127,   122,    23,   139,   265,   159,   160,   121,   282,   115,
     266,   140,   128,   274,   141,   129,   118,   140,   136,   124,
     141,   125,   240,   242,    68,   154,   155,   156,   157,   158,
      68,   241,   243,    40,    46,   137,   104,   104,   104,   104,
     104,    68,    68,    68,   124,   -60,   125,   104,   -63,    68,
      47,   -64,   145,   126,   146,    49,   147,   -61,    40,    46,
      68,    68,    68,    68,    68,    68,    68,   127,    68,   198,
     198,   198,   198,   198,   198,    47,   182,   151,   183,   130,
      49,   161,   131,   164,    82,   132,   165,   133,   166,   134,
     167,   135,   127,   176,   168,   186,   169,   188,   170,   106,
     107,   108,   109,   110,   171,   172,    68,   173,   174,   175,
    -156,    68,   177,  -155,    68,    68,   179,   180,   181,   269,
     126,   270,   206,   189,    68,   199,   199,   199,   199,   199,
     199,  -160,   -62,    40,    46,   184,   191,   105,   105,   105,
     105,   105,   192,   193,   271,   126,   -45,   262,   105,   -45,
      47,   264,   -45,   102,   -45,    49,   -45,   263,   -45,   209,
     255,   210,   272,   275,   276,   256,   277,   127,   278,   261,
     104,   104,   104,   104,   104,   104,   281,   104,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   280,   187,   194,
     286,   299,   269,    68,   270,   300,   288,    68,   301,   302,
     303,    68,   306,   233,    68,    68,    40,    46,    68,   287,
      68,   232,   205,   231,   297,   290,    68,   163,   259,     0,
     126,     0,     0,    47,     0,    97,    46,     0,    49,    68,
       0,     0,     0,   211,   213,   214,   215,   216,   216,     0,
     127,     0,    47,     0,     0,     0,     0,    49,   239,   239,
     200,   201,   202,   203,   204,   291,     0,     0,   292,    52,
     260,     0,     0,     0,   298,   234,   235,   236,   237,   238,
      96,   105,   105,   105,   105,   105,   105,     0,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   208,     0,
       0,     0,     0,   126,   100,     0,     2,     0,     3,   230,
       0,     0,   104,   104,     0,   104,     5,     6,     0,     8,
       0,     9,     0,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,     0,   -44,     0,   123,   -44,     0,
     100,   -44,     2,   -44,     3,   -44,     4,   -44,     0,   284,
       0,     0,     5,     6,    20,     8,     0,     9,     0,     0,
      11,   293,     0,    13,   296,     0,     0,     0,   101,     0,
      24,    25,     0,     0,     0,   294,   295,     0,     0,     0,
       0,     0,     0,     0,    15,   304,    16,    17,    18,    19,
      20,    21,     0,   -46,     0,     0,   -46,     0,     0,   -46,
       0,   -46,     0,   -46,    22,   -46,    24,    25,     0,     0,
       0,     0,     0,   105,   105,     1,   105,     2,     0,     3,
       0,     4,     0,     0,     0,     0,     0,     5,     6,     0,
       8,     0,     9,     0,    10,    11,     0,    12,    13,     0,
       0,     1,     0,     2,     0,     3,     0,     4,     0,     0,
       0,     0,     0,     5,     6,     7,     8,    14,     9,     0,
      10,    11,     0,    12,    13,    20,     0,     0,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    22,
       0,    24,    25,    14,     0,    15,     0,    16,    17,    18,
      19,    20,    21,     0,     0,     0,     0,     1,     0,     2,
       0,     3,     0,     4,     0,    22,    23,    24,    25,     5,
       6,     0,    92,     0,     9,     0,    10,    11,     0,    12,
      13,   100,     0,     2,     0,     3,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     0,     8,     0,     9,    14,
       0,    15,     0,    16,    17,    18,    19,    20,    21,     0,
       0,     0,     0,     1,     0,     2,     0,     3,     0,     4,
       0,    93,     0,    24,    25,     5,     6,     0,     8,     0,
       9,    20,    10,    11,     0,    12,    13,   100,     0,     2,
       0,     3,     0,     0,     0,   207,     0,    24,    25,     5,
       6,     0,     8,     0,     9,    14,     0,    15,     0,    16,
      17,    18,    19,    20,    21,     0,     0,     0,     0,   100,
       0,     2,     0,     3,     0,     0,     0,    22,     0,    24,
      25,     5,     6,     0,     8,     0,     9,    20,     0,     0,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,    22,     0,    24,    25,   100,     0,     2,     0,     3,
     178,     4,     0,     0,     0,     0,     0,     5,     6,    20,
       8,     0,     9,     0,     0,    11,     0,     0,    13,     0,
       0,     0,     0,    22,     0,    24,    25,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
       0,    16,    17,    18,    19,    20,    21,     0,     0,     0,
       0,   100,     0,     2,     0,     3,   289,     4,     0,    22,
       0,    24,    25,     5,     6,     0,     8,     0,     9,     0,
       0,    11,     0,     0,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,     0,    16,    17,    18,
      19,    20,    21,     0,     0,     0,     0,   100,     0,     2,
       0,     3,     0,     4,     0,    22,     0,    24,    25,     5,
       6,     0,     8,     0,     9,     0,     0,    11,     0,     0,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,     0,    16,    17,    18,    19,    20,    21,     0,
       0,     0,     0,     1,     0,     2,     0,     3,     0,     4,
       0,    22,   117,    24,    25,     5,     6,     0,     8,     0,
       9,     0,    10,    11,     0,    12,    13,     0,     0,     1,
       0,     2,     0,     3,     0,     4,     0,     0,     0,     0,
       0,     5,     6,     0,     8,    14,     9,     0,    10,    11,
       0,    12,    13,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,    24,
      25,    14,     0,     0,     0,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,    24,    25
  };

  /* YYCHECK.  */
  const short int
  sQLangBison::yycheck_[] =
  {
         0,     1,     2,     3,     0,     7,   262,    18,   264,     0,
       0,     3,     3,     3,     3,    41,    42,     0,    11,    13,
      46,    17,    27,    23,    27,     3,     0,    27,     3,     3,
      14,     0,    23,    23,     3,    22,    27,    27,    25,     0,
      23,   148,   149,     0,   300,    47,     3,    41,    42,    23,
     306,    44,    46,    27,    23,     1,    67,     3,    27,     6,
      44,    12,    23,     5,    11,     0,    23,     9,    55,     5,
      27,     0,    68,     9,     4,    34,    35,    23,     6,    12,
      10,    23,     0,    11,    26,    48,    21,    23,     3,    94,
      26,    94,   159,   160,    94,    36,    37,    38,    39,    40,
     100,   159,   160,    94,    94,    12,    10,    11,    12,    13,
      14,   111,   112,   113,   119,    12,   119,    21,    12,   119,
      94,    12,    28,    27,    30,    94,    32,    12,   119,   119,
     130,   131,   132,   133,   134,   135,   136,    94,   138,   130,
     131,   132,   133,   134,   135,   119,     8,    43,    10,    21,
     119,     0,    24,     4,   100,    27,     4,    29,     4,    31,
       4,    33,   119,    10,     4,   111,     4,   113,     4,    10,
      11,    12,    13,    14,     4,     4,   176,     4,     4,     4,
      11,   181,     6,    11,   184,   185,     8,     8,    11,   194,
      94,   194,   138,    67,   194,   130,   131,   132,   133,   134,
     135,     3,    12,   194,   194,     3,    12,    10,    11,    12,
      13,    14,     4,     4,     3,   119,    21,     4,    21,    24,
     194,     4,    27,     4,    29,   194,    31,    12,    33,    19,
     176,    19,     4,     3,    13,   181,    13,   194,     3,   185,
     144,   145,   146,   147,   148,   149,     4,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,    11,   112,     7,
       7,    12,   267,   263,   267,     4,    67,   267,     6,     4,
       4,   271,     4,   153,   274,   275,   267,   267,   278,   265,
     280,   152,   136,   151,   279,   267,   286,    69,   183,    -1,
     194,    -1,    -1,   267,    -1,   286,   286,    -1,   267,   299,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     267,    -1,   286,    -1,    -1,    -1,    -1,   286,   159,   160,
     131,   132,   133,   134,   135,   271,    -1,    -1,   274,   286,
     184,    -1,    -1,    -1,   280,   154,   155,   156,   157,   158,
     286,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   139,    -1,
      -1,    -1,    -1,   267,     3,    -1,     5,    -1,     7,   150,
      -1,    -1,   276,   277,    -1,   279,    15,    16,    -1,    18,
      -1,    20,    -1,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,    21,    -1,     0,    24,    -1,
       3,    27,     5,    29,     7,    31,     9,    33,    -1,   263,
      -1,    -1,    15,    16,    53,    18,    -1,    20,    -1,    -1,
      23,   275,    -1,    26,   278,    -1,    -1,    -1,    67,    -1,
      69,    70,    -1,    -1,    -1,   276,   277,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,   299,    49,    50,    51,    52,
      53,    54,    -1,    21,    -1,    -1,    24,    -1,    -1,    27,
      -1,    29,    -1,    31,    67,    33,    69,    70,    -1,    -1,
      -1,    -1,    -1,   276,   277,     3,   279,     5,    -1,     7,
      -1,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,
      18,    -1,    20,    -1,    22,    23,    -1,    25,    26,    -1,
      -1,     3,    -1,     5,    -1,     7,    -1,     9,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    45,    20,    -1,
      22,    23,    -1,    25,    26,    53,    -1,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    45,    -1,    47,    -1,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,     3,    -1,     5,
      -1,     7,    -1,     9,    -1,    67,    68,    69,    70,    15,
      16,    -1,    18,    -1,    20,    -1,    22,    23,    -1,    25,
      26,     3,    -1,     5,    -1,     7,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    18,    -1,    20,    45,
      -1,    47,    -1,    49,    50,    51,    52,    53,    54,    -1,
      -1,    -1,    -1,     3,    -1,     5,    -1,     7,    -1,     9,
      -1,    67,    -1,    69,    70,    15,    16,    -1,    18,    -1,
      20,    53,    22,    23,    -1,    25,    26,     3,    -1,     5,
      -1,     7,    -1,    -1,    -1,    67,    -1,    69,    70,    15,
      16,    -1,    18,    -1,    20,    45,    -1,    47,    -1,    49,
      50,    51,    52,    53,    54,    -1,    -1,    -1,    -1,     3,
      -1,     5,    -1,     7,    -1,    -1,    -1,    67,    -1,    69,
      70,    15,    16,    -1,    18,    -1,    20,    53,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,     3,    -1,     5,    -1,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,    53,
      18,    -1,    20,    -1,    -1,    23,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    67,    -1,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    49,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      -1,     3,    -1,     5,    -1,     7,     8,     9,    -1,    67,
      -1,    69,    70,    15,    16,    -1,    18,    -1,    20,    -1,
      -1,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,     3,    -1,     5,
      -1,     7,    -1,     9,    -1,    67,    -1,    69,    70,    15,
      16,    -1,    18,    -1,    20,    -1,    -1,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    -1,    49,    50,    51,    52,    53,    54,    -1,
      -1,    -1,    -1,     3,    -1,     5,    -1,     7,    -1,     9,
      -1,    67,    12,    69,    70,    15,    16,    -1,    18,    -1,
      20,    -1,    22,    23,    -1,    25,    26,    -1,    -1,     3,
      -1,     5,    -1,     7,    -1,     9,    -1,    -1,    -1,    -1,
      -1,    15,    16,    -1,    18,    45,    20,    -1,    22,    23,
      -1,    25,    26,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    69,
      70,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    69,    70
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  sQLangBison::yystos_[] =
  {
         0,     3,     5,     7,     9,    15,    16,    17,    18,    20,
      22,    23,    25,    26,    45,    47,    49,    50,    51,    52,
      53,    54,    67,    68,    69,    70,    72,    74,    75,    77,
      79,    81,    82,    83,    84,    85,    86,    87,    88,    92,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   119,   120,   121,   124,   125,   126,
     127,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    91,    94,   100,   101,   103,   106,   117,    89,
      90,    91,    18,    67,    74,    75,    91,    94,   122,   123,
       3,    67,    95,   125,    97,   102,   105,   105,   105,   105,
     105,     3,     3,     3,    12,    12,     3,    12,   117,    74,
      75,    91,     0,     0,    81,    86,    97,   106,     0,    48,
      21,    24,    27,    29,    31,    33,     3,    12,     5,     9,
      23,    26,    41,    42,    46,    28,    30,    32,    22,    25,
      55,    43,    14,    44,    36,    37,    38,    39,    40,    34,
      35,     0,     0,   127,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,    10,     6,     8,     8,
       8,    11,     8,    10,     3,     5,    91,    90,    91,    67,
      78,    12,     4,     4,     7,    47,    76,    93,    94,   117,
      93,    93,    93,    93,    93,    90,    91,    67,    95,    19,
      19,   105,   118,   105,   105,   105,   105,   108,   108,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      95,   116,   113,   112,   110,   110,   110,   110,   110,   105,
     115,   118,   115,   118,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    91,    91,    18,    67,   122,
      90,    91,     4,    12,     4,     4,    10,    73,    80,    81,
      86,     3,     4,     6,    11,     3,    13,    13,     3,    11,
      11,     4,     6,    76,    90,    76,     7,    77,    67,     8,
      80,    91,    91,    90,   105,   105,    90,   114,    91,    12,
       4,     6,     4,     4,    90,    76,     4,    76
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  sQLangBison::yytoken_number_[] =
  {
         0,   256,   257,    40,    41,    91,    93,   123,   125,    46,
      44,    58,    59,   124,    63,   258,   259,   260,   261,   262,
     263,    61,    43,   264,   265,    45,   266,   267,    42,   268,
      47,   269,    37,   270,   271,   272,    62,    60,   273,   274,
     275,   276,   277,   278,   279,    33,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  sQLangBison::yyr1_[] =
  {
         0,    71,    72,    72,    72,    72,    73,    73,    74,    74,
      75,    75,    76,    77,    77,    77,    78,    78,    79,    79,
      80,    80,    81,    81,    81,    81,    81,    81,    82,    82,
      83,    83,    84,    85,    86,    86,    87,    88,    89,    89,
      90,    90,    91,    91,    92,    92,    92,    93,    93,    94,
      94,    94,    94,    94,    94,    95,    95,    95,    95,    95,
      96,    96,    96,    96,    96,    97,    97,    97,    97,    97,
      97,    97,    98,    99,   100,   100,   101,   102,   102,   103,
     103,   104,   104,   105,   105,   105,   105,   105,   105,   106,
     106,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   108,   108,   108,   108,   109,   109,   109,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   111,   112,   112,   113,
     113,   114,   114,   115,   115,   115,   115,   115,   115,   116,
     116,   116,   116,   116,   117,   118,   118,   119,   119,   119,
     120,   120,   120,   120,   121,   122,   122,   123,   123,   124,
     125,   126,   126,   127,   127,   127
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
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
       1,     1,     1,     1,     3,     1,     1,     3,     5,     3,
       1,     1,     2,     1,     3,     3
  };


  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const sQLangBison::yytname_[] =
  {
    "\"end of program\"", "error", "$undefined", "'('", "')'", "'['", "']'",
  "'{'", "'}'", "'.'", "','", "':'", "';'", "'|'", "'?'",
  "\"int literal\"", "\"real literal\"", "\"template literal substring\"",
  "\"string literal\"", "\"regex literal\"", "\"null literal\"", "'='",
  "'+'", "\"'++'\"", "\"'+='\"", "'-'", "\"'--'\"", "\"'-='\"", "'*'",
  "\"'*='\"", "'/'", "\"'/='\"", "'%'", "\"'%='\"", "\"'=='\"", "\"'!='\"",
  "'>'", "'<'", "\"'>='\"", "\"'<='\"", "\"'<=>'\"", "\"'=~'\"",
  "\"'!~'\"", "\"'&&'\"", "\"'||'\"", "'!'", "\"'has'\"", "\"'if'\"",
  "\"'else'\"", "\"'for'\"", "\"'while'\"", "\"'break'\"",
  "\"'continue'\"", "\"'function'\"", "\"'return'\"", "\"'as'\"",
  "\"'bool'\"", "\"'int'\"", "\"'uint'\"", "\"'intlist'\"", "\"'real'\"",
  "\"'string'\"", "\"'obj'\"", "\"'objlist'\"", "\"'datetime'\"",
  "\"'date'\"", "\"'time'\"", "\"name\"", "\"$(\"", "\"$NUM\"",
  "\"$NAME\"", "$accept", "input", "statements", "non_return_statements",
  "statements_with_return", "statement_block", "lambda_block", "namelist",
  "lambda", "statement", "non_return_statement", "if_statement_start",
  "if_statement", "for_statement", "while_statement", "return_statement",
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
  "template", "template_expression", YY_NULL
  };

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const sQLangBison::rhs_number_type
  sQLangBison::yyrhs_[] =
  {
        72,     0,    -1,   117,     0,    -1,    75,     0,    -1,    74,
       0,    -1,   126,     0,    -1,    80,    -1,    73,    80,    -1,
      81,    -1,    74,    81,    -1,    86,    -1,    74,    86,    -1,
       7,    73,     8,    -1,     7,    91,     8,    -1,     7,    75,
       8,    -1,     7,    74,     8,    -1,    67,    -1,    78,    10,
      67,    -1,    77,    -1,    53,     3,    78,     4,    77,    -1,
      81,    -1,    86,    -1,    83,    -1,    84,    -1,    85,    -1,
      87,    -1,    88,    -1,    96,    12,    -1,    47,     3,    91,
       4,    76,    -1,    82,    48,    47,     3,    91,     4,    76,
      -1,    82,    -1,    82,    48,    76,    -1,    49,     3,    90,
      12,    90,    12,    90,     4,    76,    -1,    50,     3,    91,
       4,    76,    -1,    54,    12,    -1,    54,   117,    12,    -1,
      51,    12,    -1,    52,    12,    -1,    91,    -1,    89,    10,
      91,    -1,    -1,    89,    -1,   117,    -1,    94,    -1,   125,
      -1,   102,    -1,    97,     5,    91,     6,    -1,    94,    -1,
     117,    -1,    92,    21,    93,    -1,    92,    24,    93,    -1,
      92,    27,    93,    -1,    92,    29,    93,    -1,    92,    31,
      93,    -1,    92,    33,    93,    -1,   119,    -1,   125,    -1,
      79,    -1,     3,    91,     4,    -1,   104,    -1,   100,    -1,
     106,    -1,    94,    -1,   101,    -1,   103,    -1,    95,    -1,
      98,    -1,    99,    -1,   100,    -1,   102,    -1,   103,    -1,
     101,    -1,    97,     5,    91,    11,    91,     6,    -1,    97,
       5,    91,     6,    -1,    97,    23,    -1,    97,    26,    -1,
      95,     3,    90,     4,    -1,     9,    67,    -1,    97,     9,
      67,    -1,     9,    95,     3,    90,     4,    -1,    97,     9,
      95,     3,    90,     4,    -1,    69,    -1,    70,    -1,    97,
      -1,   106,    -1,   107,    -1,    25,   105,    -1,    22,   105,
      -1,    45,   105,    -1,    23,   105,    -1,    26,   105,    -1,
       3,    56,     4,    95,    -1,     3,    57,     4,    95,    -1,
       3,    58,     4,    95,    -1,     3,    59,     4,    95,    -1,
       3,    60,     4,    95,    -1,     3,    61,     4,    95,    -1,
       3,    62,     4,    95,    -1,     3,    63,     4,    95,    -1,
       3,    64,     4,    95,    -1,     3,    65,     4,    95,    -1,
       3,    66,     4,    95,    -1,   105,    -1,   108,    28,   105,
      -1,   108,    30,   105,    -1,   108,    32,   105,    -1,   108,
      -1,   109,    22,   108,    -1,   109,    25,   108,    -1,   109,
      -1,   105,    46,   105,    -1,   105,    46,   118,    -1,   105,
      41,    19,    -1,   105,    42,    19,    -1,   109,    55,    56,
      -1,   109,    55,    57,    -1,   109,    55,    58,    -1,   109,
      55,    59,    -1,   109,    55,    60,    -1,   109,    55,    61,
      -1,   109,    55,    62,    -1,   109,    55,    63,    -1,   109,
      55,    64,    -1,   109,    55,    65,    -1,   109,    55,    66,
      -1,   111,    -1,   109,    55,    95,     3,    90,     4,    -1,
     116,    -1,   112,    43,   116,    -1,   112,    -1,   113,    44,
     112,    -1,   113,    -1,   113,    14,   113,    11,   114,    -1,
     110,    -1,   115,    37,   110,    -1,   115,    36,   110,    -1,
     115,    39,   110,    -1,   115,    38,   110,    -1,   115,    40,
     110,    -1,   115,    -1,   116,    34,   115,    -1,   116,    34,
     118,    -1,   116,    35,   115,    -1,   116,    35,   118,    -1,
     114,    -1,   105,    13,   105,    -1,   118,    13,   105,    -1,
     120,    -1,   121,    -1,   124,    -1,    15,    -1,    16,    -1,
      18,    -1,    20,    -1,     5,    90,     6,    -1,    67,    -1,
      18,    -1,   122,    11,    91,    -1,   123,    10,   122,    11,
      91,    -1,     7,   123,     8,    -1,    67,    -1,   127,    -1,
     126,   127,    -1,    17,    -1,    68,    91,     4,    -1,    68,
      75,     4,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  sQLangBison::yyprhs_[] =
  {
         0,     0,     3,     6,     9,    12,    15,    17,    20,    22,
      25,    27,    30,    34,    38,    42,    46,    48,    52,    54,
      60,    62,    64,    66,    68,    70,    72,    74,    77,    83,
      91,    93,    97,   107,   113,   116,   120,   123,   126,   128,
     132,   133,   135,   137,   139,   141,   143,   148,   150,   152,
     156,   160,   164,   168,   172,   176,   178,   180,   182,   186,
     188,   190,   192,   194,   196,   198,   200,   202,   204,   206,
     208,   210,   212,   219,   224,   227,   230,   235,   238,   242,
     248,   255,   257,   259,   261,   263,   265,   268,   271,   274,
     277,   280,   285,   290,   295,   300,   305,   310,   315,   320,
     325,   330,   335,   337,   341,   345,   349,   351,   355,   359,
     361,   365,   369,   373,   377,   381,   385,   389,   393,   397,
     401,   405,   409,   413,   417,   421,   423,   430,   432,   436,
     438,   442,   444,   450,   452,   456,   460,   464,   468,   472,
     474,   478,   482,   486,   490,   492,   496,   500,   502,   504,
     506,   508,   510,   512,   514,   518,   520,   522,   526,   532,
     536,   538,   540,   543,   545,   549
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  sQLangBison::yyrline_[] =
  {
         0,   236,   236,   244,   252,   262,   276,   281,   289,   294,
     302,   307,   315,   324,   330,   336,   347,   352,   361,   365,
     374,   375,   379,   380,   381,   382,   383,   384,   391,   395,
     404,   405,   413,   429,   436,   440,   447,   454,   461,   466,
     475,   478,   482,   483,   487,   488,   489,   496,   497,   501,
     505,   509,   513,   517,   521,   528,   529,   530,   531,   535,
     539,   540,   541,   542,   543,   547,   548,   549,   550,   551,
     552,   553,   557,   564,   571,   575,   582,   592,   597,   605,
     612,   622,   633,   648,   649,   650,   651,   662,   672,   679,
     683,   690,   694,   698,   702,   706,   710,   714,   718,   722,
     726,   730,   737,   738,   742,   746,   753,   754,   758,   765,
     766,   770,   774,   790,   806,   810,   814,   818,   822,   826,
     830,   834,   838,   842,   846,   850,   854,   864,   865,   872,
     873,   880,   881,   888,   889,   893,   897,   901,   905,   912,
     913,   917,   921,   925,   932,   936,   943,   951,   952,   953,
     957,   961,   965,   970,   977,   987,   992,  1000,  1005,  1013,
    1023,  1031,  1036,  1044,  1049,  1056
  };

  // Print the state stack on the debug stream.
  void
  sQLangBison::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  sQLangBison::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
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
       2,     2,     2,    45,     2,     2,     2,    32,     2,     2,
       3,     4,    28,    22,    10,    25,     9,    30,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    11,    12,
      37,    21,    36,    14,     2,     2,     2,     2,     2,     2,
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
      17,    18,    19,    20,    23,    24,    26,    27,    29,    31,
      33,    34,    35,    38,    39,    40,    41,    42,    43,    44,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int sQLangBison::yyeof_ = 0;
  const int sQLangBison::yylast_ = 966;
  const int sQLangBison::yynnts_ = 57;
  const int sQLangBison::yyempty_ = -2;
  const int sQLangBison::yyfinal_ = 122;
  const int sQLangBison::yyterror_ = 1;
  const int sQLangBison::yyerrcode_ = 256;
  const int sQLangBison::yyntokens_ = 71;

  const unsigned int sQLangBison::yyuser_token_number_max_ = 304;
  const sQLangBison::token_number_type sQLangBison::yyundef_token_ = 2;


} // yy
/* Line 1141 of lalr1.cc  */
#line 3842 "qlang-bison.cpp"
/* Line 1142 of lalr1.cc  */
#line 1066 "qlang-bison.yy"

