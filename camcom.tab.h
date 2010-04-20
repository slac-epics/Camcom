/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKHEXNUM = 258,
     TOKVALREQ = 259,
     TOKNONO = 260,
     TOKEQUAL = 261,
     TOKTERM = 262,
     TOKSLASH = 263,
     TOKNONOBAD = 264,
     TOKDEP = 265,
     TOKVERB1 = 266,
     TOKVERB2 = 267,
     TOKVERB3 = 268,
     TOKVERB4 = 269,
     TOKVERB5 = 270,
     TOKFOREVER = 271,
     TOKSTRING = 272,
     TOKNOVAL = 273,
     TOKNEGATED = 274,
     TOKCOMMENT = 275,
     TOKPARAM = 276
   };
#endif
#define TOKHEXNUM 258
#define TOKVALREQ 259
#define TOKNONO 260
#define TOKEQUAL 261
#define TOKTERM 262
#define TOKSLASH 263
#define TOKNONOBAD 264
#define TOKDEP 265
#define TOKVERB1 266
#define TOKVERB2 267
#define TOKVERB3 268
#define TOKVERB4 269
#define TOKVERB5 270
#define TOKFOREVER 271
#define TOKSTRING 272
#define TOKNOVAL 273
#define TOKNEGATED 274
#define TOKCOMMENT 275
#define TOKPARAM 276




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 9 "camcom.y"
typedef union YYSTYPE {
    char* str;
    int   ival;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 84 "camcom.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



