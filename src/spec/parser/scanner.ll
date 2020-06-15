/*
 * scanner.ll
 *
 * TODO adopted from from Binpac++, add their copyright.
 * 
 * Distributed under the MIT License (MIT).
 *
 * Copyright (c) 2015 Eric Seckler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

%{ /*** C/C++ Declarations ***/

#include <cassert>
#include <string>

#include "spec/parser/scanner.h"
#include "spec/parser/driver.h"
#include "util/util.h"

/* import the parser's token type into a local typedef */
typedef diffingo::spec::Parser::token token;
typedef diffingo::spec::Parser::token_type token_type;

/* By default yylex returns int, we use token_type. */
#define yyterminate() return token::END

/* Track locations accurately. Each time yylex is invoked,
 * the begin position is moved onto the end position. */
#define YY_USER_ACTION  yylloc->columns(yyleng);

%}

/*** Flex Declarations and Options ***/

/* enable c++ scanner class generation */
%option c++

/* change the name of the scanner class */
%option prefix="Diffingo"

/* the manual says "somewhat more optimized" */
%option batch

/* enable scanner to generate debug output. disable this for release versions. */
%option debug

/* no support for include files and unputting characters */
%option noyywrap nounput

/* track lines */
%option yylineno

/* separate state for parsing regexps and embedded code */
%x RE
%x EC

/* regexp helpers */
attribute \&[a-zA-Z_][a-zA-Z_0-9]*
blank     [ \t]
comment   [ \t]*#[^\n]*
digits    [0-9]+
hexs      [0-9a-fA-F]+
id        [a-zA-Z_][a-zA-Z_0-9-]*|[$][$]
int       [+-]?[0-9]+
double    [-+]?[0-9]+\.[0-9]+
property  %[a-zA-Z_][a-zA-Z_0-9-]*
string    \"(\\.|[^\\"])*\"


%% /*** Lexing Rules ***/


%{
	/* code to place at the beginning of yylex() */
	
    // reset location
    yylloc->step();
%}


    /* handling blanks and comments */
{blank}+              yylloc->step();
[\n]+                 yylloc->lines(yyleng);
{comment}             /* Eat. */

    /* regexp mode rules */
<RE>(\\.|[^\\\/])*    yylval->sval = yytext; return token::CREGEXP;
<RE>[/\\\n]	          return (token_type) yytext[0];

    /* embedded code rules */
<EC>"%}"              yyless(yyleng-2); yylval->sval = yytext; yy_pop_state(); return token::CPPCODE;
<EC>\n                yylloc->lines(1); yymore();
<EC>.                 yymore();

    /* keywords */
any                   return token::ANY;
as                    return token::AS;
bitfield              return token::BITFIELD;
bitset                return token::BITSET;
bool                  return token::BOOL;
bytes                 return token::BYTES;
caddr                 return token::CADDR;
cast                  return token::CAST;
const                 return token::CONST;
double                return token::DOUBLE;
else                  return token::ELSE;
enum                  return token::ENUM;
export                return token::EXPORT;
for                   return token::FOR;
find                  return token::FIND;
if                    return token::IF;
import                return token::IMPORT;
in                    return token::IN;
include               return token::INCLUDE;
instantiate           return token::INSTANTIATE;
int                   return token::INT;
int8                  return token::INT8;
int16                 return token::INT16;
int32                 return token::INT32;
int64                 return token::INT64;
list                  return token::LIST;
map                   return token::MAP;
mod                   return token::MOD;
module                return token::MODULE;
set                   return token::SET;
sink                  return token::SINK;
string                return token::STRING;
switch                return token::SWITCH;
then                  return token::THEN;
tuple                 return token::TUPLE;
type                  return token::TYPE;
uint                  return token::UINT;
uint8                 return token::UINT8;
uint16                return token::UINT16;
uint32                return token::UINT32;
uint64                return token::UINT64;
unit                  return token::UNIT;
var                   return token::VAR;
vector                return token::VECTOR;
void                  return token::VOID;

\&\&                  return token::AND;
->                    return token::ARROW;
"%{"                  yy_push_state(EC); return token::BEGINCODE;
=>                    return token::DARROW;
\.\.                  return token::DOTDOT;
"%}"                  return token::ENDCODE;
==                    return token::EQ;
\>=                   return token::GEQ;
\?\.                  return token::HASATTR;
\<=                   return token::LEQ;
-=                    return token::MINUSASSIGN;
--                    return token::MINUSMINUS;
!=                    return token::NEQ;
\|\|                  return token::OR;
\+=                   return token::PLUSASSIGN;
\+\+                  return token::PLUSPLUS;
\*\*                  return token::POW;
\<\<                  return token::SHIFTLEFT;
\>\>                  return token::SHIFTRIGHT;

    /* constants */
False                 yylval->bval = 0; return token::CBOOL;
True                  yylval->bval = 1; return token::CBOOL;
None                  return token::NONE;

{attribute}           yylval->sval = yytext; return token::ATTRIBUTE;
{property}            yylval->sval = yytext; return token::PROPERTY;

{double}              yylval->dval = strtod(yytext, 0); return token::CDOUBLE;
{int}                 yylval->ival = strtoll(yytext, 0, 10); return token::CINTEGER;
0x{hexs}              yylval->ival = strtoll(yytext + 2, 0, 16); return token::CINTEGER;

{string}              yylval->sval = util::expandEscapes(std::string(yytext, 1, strlen(yytext) - 2)); return token::CSTRING;
b{string}             yylval->sval = util::expandEscapes(std::string(yytext, 2, strlen(yytext) - 3)); return token::CBYTES;

{id}(\/{id}){1,}      yylval->sval = yytext; return token::PATH_IDENT;

{id}                  yylval->sval = yytext; return token::IDENT;
{id}(::{id}){1,}      yylval->sval = yytext; return token::SCOPED_IDENT;
\${id}                yylval->sval = yytext + 1; return token::DOLLAR_IDENT;

[][$?.,=:;<>(){}/|*/&^%!+-] return (token_type) yytext[0];

.                     driver->error("invalid character", *yylloc);


%% /*** Additional Code ***/

#ifdef yylex
#undef yylex
#endif

int DiffingoFlexLexer::yylex()
{
    assert(false); /* Shouldn't be called (use lex functions in Scanner class instead). */
    return 0;
}

void diffingo::spec::Scanner::enablePatternMode()
{
    yy_push_state(RE);
}

void diffingo::spec::Scanner::disablePatternMode()
{
    yy_pop_state();
}
